/****************************************************************
*Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)
*
*Permission to use, copy, modify, and distribute this material
*for any purpose and without fee is hereby granted, provided
*that the above copyright notice and this permission notice
*appear in all copies, and that the name of Bellcore not be
*used in advertising or publicity pertaining to this
*material without the specific, prior written permission
*of an authorized representative of Bellcore.  BELLCORE
*MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
*OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
*WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
****************************************************************/
static char mvs_libget_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/mvs_libget.c,v 1.2 1994/04/04 10:22:12 jrh Exp $";
/*
*$Log: mvs_libget.c,v $
*Revision 1.2  1994/04/04 10:22:12  jrh
*FROM_KEYS
*
*Revision 1.2  94/04/04  10:22:12  jrh
*Add Release Copyright
*
*Revision 1.1  93/08/04  15:39:15  ewk
*Initial revision
*
*-----------------------------------------------end of log
*/
/*
 *  libGet - MVS read include members
 *
 *    function: this module reads an object deck from the DD OBJIN.
 *              it examines each ESD line to look for external
 *              definitions and external references.  For each external
 *              definition, it keeps track of it in a hash table
 *              and for each external reference, it attempts to
 *              resolve it by looking in an object library for
 *              that module.
 *
 *              In addition, control cards of the form:
 *                  LIBRARY DDNAME(MEMBER)  or
 *                  INCLUDE DDNAME(MEMBER)
 *              are also accepted.  For the LIBRARY case, the
 *              program keeps the member is a hash table, and if
 *              there is an external reference to it, it will
 *              use the designated library to find it, instead
 *              of SYSLIB.  For the INCLUDE case, the program
 *              will immediately read the member from the given
 *              DDNAME and process each line, then go back to
 *              the former input stream
 *
 *    return codes: 0 - OK
 *                  8 - global variable not found
 *                 12 - internal error
 *
 *
 */

/*
  Copyright 1990 Bell Communications Research, Inc.
*/

#include <mvapts.h>

MODULEID(%M%,%I%/%D%/%T%)

COPYRIGHT(%YR%)

#include <stdefs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vsdmcpds"
#include "vsdmmh"
#include "malloc"

#define MAXEXT 20000               /* max num of external references */
#define MAXMEM 10000               /* max number of read object mems */
#define MAXPDSR 5000               /* max number of read PDS mems */
#define NAMELEN 8                  /* length of member name */
#define OBJLEN 80                  /* length of line from obj file */
#define PDSRLEN 16                 /* length of ttr+file data */
#define MAXFILE 128                /* max num of include/library dd's*/
#define MAXLIB  1024               /* max num of library cards */
#define ISINCLUDE 2                /* tags include file */
#define ISGLOBAL 1                 /* tags global reference */
#define ISFUNC   0                 /* tags external function reference*/
#define NOGLOBCHECK 0              /* dont check for global var error */
#define GLOBCHECK 1                /* check for global var error */
#define NOHASHCHECK 0              /* dont check in hash table for var*/
#define HASHCHECK 1                /* check in hash table for var*/
#define ERRORMSG if (1<=Errlevel)  /* used for error level msgs */
#define INFOMSG if (2<=Errlevel)   /* used for error level msgs */
#define TRACEMSG if (3<=Errlevel)  /* used for error level msgs */

static int Errlevel = 1;           /* error message level to print out*/
static ENTRY Item;                 /* hash table Item entered */
static FILE *Objout;               /* output composite object */

static struct                  /* ptr to space for ddname - vsdmpds*/
{                              /* data                                */
    char ddname[NAMELEN+1];
    struct pds_struct *pdsinfo;
} *Ddspace;
static int Ddnum = 0;          /* counter of dd names */

static HTABLE *Files;          /* DD's opened hash table */

unsigned vsdmpds();
#pragma linkage(vsdmpds,OS)

struct esd                       /* layout of ESD object card */
{
    char fill1;                  /* always x'02' */
    char type[3];                /* object card type */
    char fill2[12];
    struct
    {
        char name[NAMELEN];      /* external reference name */
        char key;                /* type external reference */
        char fill3[4];
        unsigned length : 24;    /* length of reference */
    } entry[3];
    char fill4[16];
};

struct bldl                      /* layout of BLDL dir entry */
{
    char modname[8];             /* module name */
    union {
        unsigned ttr_concat;    /* TTR and lib concat */
        struct {
            unsigned ttr : 24;
            unsigned concat : 8;
        } t_c;
    } ttr_union;
    char fill1;
    unsigned alias_ind : 1;      /* 1 if alias, 0 if not */
    unsigned fill2 : 2;
    unsigned length : 5;         /* no. of halfwords of user data */
    char fill3[24];
    char aliasname[8];           /* name of module aliased to */
};



/*
 * this internal function trims trailing blanks from a character
 * string.  From MVA/PTS.
 */
static
char *vsmvtrmt(in_string,out_string)
char *in_string;
char *out_string;
{
    char *p_out;
    p_out = out_string;

    while (*p_out++ = *in_string++);

    /* get before the the '\0' for p_out subtract 2 */

    for (p_out -= 2; *p_out == ' ' && p_out >= out_string; p_out--);
    *(++p_out) = '\0';

    return(out_string);
}
/*
 * add_file: open dd of given name, and add its entry to the dd name
 *           hash table
 */
static
void add_file(char *file)
{
    struct pds_struct *libdata;   /* work pointer to vsdmpds struct */

    TRACEMSG
        fprintf(stdout, "Opening %s\n", file);
    /*
     * allocate a new pds structure and initialize
     */
    if ((libdata = g_malloc(sizeof(struct pds_struct))) == NULL)
    {
        ERRORMSG
        {
            fprintf(stderr, "Cannot allocate space to libdata\n");
            fprintf(stdout, "Cannot allocate space to libdata\n");
        }
        exit(12);
    }
    memset(libdata, '\0', sizeof(struct pds_struct));
    memset(libdata->member, ' ', sizeof(libdata->member));
    /*
     * set up open parms in structure - open command and dd name
     */
    memcpy(libdata->command, "OPEN    ",
        sizeof(libdata->command));
    memset(libdata->ddname, ' ', sizeof(libdata->ddname));
    memcpy(libdata->ddname, file, strlen(file));
    /*
     * open the dd - if unable, then exit the program
     */
    if (vsdmpds(libdata) != 0)
    {
        ERRORMSG
        {
            fprintf(stderr, "CANNOT OPEN %s\n", file);
            fprintf(stdout, "CANNOT OPEN %s\n", file);
        }
        free(libdata);
        exit(12);
    }
    /*
     * put pointer and dd name into hash table space
     */
    Ddspace[Ddnum].pdsinfo = libdata;
    strcpy(Ddspace[Ddnum].ddname, file);
    /*
     * set up hash entry item and enter into hash table
     */
    Item.key = Ddspace[Ddnum].ddname;
    Item.data = Ddspace[Ddnum].pdsinfo;
    if (m_hsearch(Files, Item, ENTER) == NULL)
    {
        ERRORMSG
        {
            fprintf(stderr, "CANNOT ENTER %s INTO HASH\n",file);
            fprintf(stdout, "CANNOT ENTER %s INTO HASH\n",file);
        }
        free(libdata);
        exit(16);
    }
    Ddnum++;
}

/*
 * proc_line: process a line of input
 *
 *     parms: cardin - pointer to line of input
 *            inblk  - pointer to external ref queue
 *
 *    return: 1 - if line should be output by caller
 *            0 - if line should not be output
 *
 *  function: this routine does several things:
 *             1) check if the col 73-80 tag has already been seen
 *             2) if it is an ESD card, parse and account for
 *                external definitions and external references
 *             3) if it is a LIBRARY card, put name into libname
 *                hash table
 *             4) if it is an INCLUDE card, immediately call subget
 *                to read in file
 */
static
int proc_line(char *cardin, char *cardout)
{
    memcpy(cardout, cardin, OBJLEN);
    cardout[OBJLEN] = '\0';
    vsmvtrmt(cardout, cardout);
    cardout[strlen(cardout)+1] = '\0';
    cardout[strlen(cardout)] = '\n';
    return(1);
}

/*
 * subget : read in a member from a PDS and process each line
 *
 * parms  : file - name of DD to read in
 *          inname - member to read (blank padded to 8 chars)
 *          buffer - address of buffer pointer for include mem
 */
static
int  subget(char *file, char *inname, char **buffer)
{
    char nullname[NAMELEN+1];   /* null terminated version
                                   of inname */
    struct pds_struct *pdsptr;  /* pointer to vsdmpds struct */
    ENTRY *ritem;               /* pointer to hash table Item */
    struct bldl *bldlptr;       /* pointer to BLDL record */
    int totalsize = 0;

    TRACEMSG
        fprintf(stdout,
            "Subget called file=%s, inname=%-8.8s\n",
            file, inname);
    /*
     * look to see if file has been opened - it should have been
     */
    Item.key = file;
    if ((ritem=m_hsearch(Files, Item, FIND)) == NULL)
    {
        ERRORMSG
        {
            fprintf(stderr,"File: %s not in hash table\n", file);
            fprintf(stdout,"File: %s not in hash table\n", file);
        }
        exit(12);
    }
    pdsptr = ritem->data;

    /*
     * look for member in designated PDS
     */
    memcpy(pdsptr->command, "FIND    ", sizeof(pdsptr->command));
    memcpy(pdsptr->member , inname, sizeof(pdsptr->ddname));
    TRACEMSG
        fprintf(stdout,"FINDING %-8.8s in library %-8.8s\n",
            inname, pdsptr->ddname);
    if (vsdmpds(pdsptr) != 0)
    {
        INFOMSG
        {
            fprintf(stderr,
                "MEMBER  %-8.8s NOT FOUND\n", inname);
            fprintf(stdout,
                "MEMBER  %-8.8s NOT FOUND\n", inname);
        }
        return(0);
    }
    /*
     * check member that has been found
     */
    bldlptr = (struct bldl *) pdsptr->record_addr;
    TRACEMSG
        {
            fprintf(stdout,
                "Member found, TTR: %x, CONCAT: %u, alias = %u,\
 user data amt: %u\n", bldlptr->ttr_union.t_c.ttr,
                    bldlptr->ttr_union.t_c.concat,
                    bldlptr->alias_ind, bldlptr->length);
            if (bldlptr->alias_ind && bldlptr->length == 16)
                fprintf(stdout,
                    "  Is alias gen by vsdmala, orig member: %-8.8s\n",
                    bldlptr->aliasname);
        }
    /*
     * member has been found, read through it  and process each line
     */
    *buffer = NULL;
    TRACEMSG
        fprintf(stdout,"READING %-8.8s\n", inname);
    memcpy(pdsptr->command, "READ    ", sizeof(pdsptr->command));
    while(vsdmpds(pdsptr) == 0)
    {
        char newline[256];

        if(proc_line((char *) pdsptr->record_addr, newline))
        {
            int insize;
            insize = strlen(newline);
            if ((*buffer=realloc(*buffer, totalsize+insize+1)) == NULL)
            {
                ERRORMSG
                {
                    fprintf(stderr,
                        "Cannot allocate space for mem buffer\n");
                    fprintf(stdout,
                        "Cannot allocate space for mem buffer\n");
                }
                exit(12);
            }
            strcpy((*buffer)+totalsize, newline);
            totalsize += insize;
        }
    }
    return(totalsize);
}

char *libGet(char *memName, char *includePath, int *includeLen,
             char **fname)
{
    int chararg;              /* getopt switch variable */
    ENTRY *ritem;             /* entry from DD name hash table */
    int i;                    /* index */
    int begin;                /* beginning of include name */
    int end;                  /* end of include name */
    int buflen;               /* length of returned include mem */
    char inmem[16];           /* include member name */
    char myinclude[256];      /* work area for include path */
    char *token;              /* token from strtok */
    char *allocptr;           /* variable to put include buff ptr */

    /*
     * initialize hash tables
     */
    if(Ddspace == NULL &&
        (Ddspace=malloc(MAXFILE*(sizeof(*Ddspace)))) == NULL)
    {
        ERRORMSG
        {
            fprintf(stderr, "Cannot allocate Ddspace\n");
            fprintf(stdout, "Cannot allocate Ddspace\n");
        }
        exit(12);
    }
    if(Files == NULL && (Files=m_hcreate(MAXFILE)) == NULL)
    {
        ERRORMSG
        {
            fprintf(stderr, "Cannot create Files Hash Table\n");
            fprintf(stdout, "Cannot create Files Hash Table\n");
        }
        exit(12);
    }

    mallopt(M_WHERE, M_BELOW);

    begin = 0;
    for (i=strlen(memName)-1; i>=0; i--)
    {
        if (memName[i] == '/')
        {
            begin = i+1;
            break;
        }
    }
    end = 0;
    for (i=strlen(memName)-1; i>=0 && i>=begin; i--)
    {
        if (memName[i] == '.')
        {
            end = i-1;
            break;
        }
        if (memName[i] == '"' || memName[i] == '>')
        {
            end = i-1;
        }
    }
    if (end == 0)
       end = strlen(memName) - 1;
    memset(inmem, ' ', 8);
    for (i=begin; i<=end; i++)
    {
        if (islower(memName[i]))
            inmem[i-begin] = toupper(memName[i]);
        else if (memName[i] == '_')
            inmem[i-begin] = '@';
        else
            inmem[i-begin] = memName[i];
    }
    /*
     * open up dd names
     */

    strcpy(myinclude, includePath);
    token = strtok(myinclude, ",");
    do
    {
        char ddname[16];

        strncpy(ddname, token, 8);
        ddname[8] = '\0';
        vsmvtrmt(ddname, ddname);
        for (i=0; i<strlen(ddname); i++)
            ddname[i] = toupper(ddname[i]);
        Item.key = ddname;
        if (m_hsearch(Files, Item, FIND) == NULL)
            add_file(ddname);
        if (buflen=subget(ddname, inmem, &allocptr))
        {
            if (fname != NULL)
            {
                if ((*fname=malloc(64)) == NULL)
                {
                    ERRORMSG
                    {
                        fprintf(stderr,
                            "Cannot allocate space for name buffer\n");
                        fprintf(stdout,
                            "Cannot allocate space for name buffer\n");
                    }
                    exit(12);
                }
                sprintf(*fname, "DD:%s(%-8.8s)", ddname, inmem);
            }

            if (includeLen != NULL)
            {
                *includeLen = buflen;
            }
            return(allocptr);
        }
    }
    while (token = strtok(NULL, ","));
    return(NULL);
}
