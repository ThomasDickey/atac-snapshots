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
#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

/* INCLUDED FILES */

#include <stdio.h>              /* standard input/output lib    */
#include <string.h>
#include <ctype.h>              /* standard library routines    */

#include "portable.h"
#include "atacysis.h"           /* ATAC post run-time stuff     */

static char const tab_disp_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/tab_disp.c,v 3.7 1996/11/13 01:35:57 tom Exp $";
/*
* $Log: tab_disp.c,v $
* Revision 3.7  1996/11/13 01:35:57  tom
* ifdef'd unused code
*
* Revision 3.6  1995/12/28 15:23:06  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.5  94/04/04  10:26:26  jrh
*Add Release Copyright
*
*Revision 3.4  93/11/02  11:50:35  saul
*Same as revision 3.2
*
*Revision 3.2  93/08/04  15:58:59  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/03/30  15:35:09  saul
*Fix warning on sun for unsigned short constant of -5 (TD_CHECKED).
*
*Revision 3.0  92/11/06  07:47:41  saul
*propagate to version 3.0
*
*Revision 2.4  92/11/02  11:43:47  saul
*remove unused variables
*
*Revision 2.3  92/10/30  09:55:46  saul
*include portable.h
*
*Revision 2.2  92/09/08  09:01:43  saul
*New coverage vector data structure.
*
*Revision 2.1  92/09/08  08:58:27  saul
*Purdue tabular display.
*
*-----------------------------------------------end of log
*/
/* CS 490T
 * Group III -- TIGGER        
 *
 * M.S. Manley (manley@iies.ecn.purdue.edu)
 *
 * 17 Feb 1992  -- MSM -- creation  
 * 
 * $Log: tab_disp.c,v $
 * Revision 3.7  1996/11/13 01:35:57  tom
 * ifdef'd unused code
 *
 * Revision 3.6  1995/12/28 15:23:06  tom
 * adjust headers, prototyped for autoconfig
 *
*Revision 3.5  94/04/04  10:26:26  jrh
*Add Release Copyright
*
*Revision 3.4  93/11/02  11:50:35  saul
*Same as revision 3.2
*
*Revision 3.2  93/08/04  15:58:59  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
*Revision 3.1  93/03/30  15:35:09  saul
*Fix warning on sun for unsigned short constant of -5 (TD_CHECKED).
*
*Revision 3.0  92/11/06  07:47:41  saul
*propagate to version 3.0
*
*Revision 2.4  92/11/02  11:43:47  saul
*remove unused variables
*
*Revision 2.3  92/10/30  09:55:46  saul
*include portable.h
*
*Revision 2.2  92/09/08  09:01:43  saul
*New coverage vector data structure.
*
 * Revision 2.3  92/05/28  20:15:21  cs490sb
 * MSM -- corrected output display for empty criteria
 * 
 * Revision 3.2  92/04/30  14:30:21  cs490sa
 * y
 * 
 * Revision 3.1  92/04/19  23:09:13  cs490sa
 * 
 * Coverage Tracking
 * 
 * 
 * Revision 2.1  92/04/07  13:36:06  cs490sa
 * 
 * 
 * Created for version p1
 * 
 * 
 * Revision 1.17  92/04/02  19:57:29  cs490sb
 * MSM -- removed global display functions for ATAC version P1
 * 
 * Revision 1.16  92/03/25  00:10:57  cs490sb
 * MSM -- added safecheck to decision coverage routine
 * 
 * Revision 1.15  92/03/20  12:08:25  cs490sb
 * MSM -- fixed decision display
 * BLW -- fixed header display flag
 * 
 * Revision 1.14  92/03/16  01:09:31  cs490sb
 * MSM -- added global_cov recognition
 * 
 * Revision 1.13  92/03/14  16:13:14  cs490sb
 * BLW--Added the tabular display of undefined functions.
 * 
 * Revision 1.12  92/03/12  01:49:28  cs490sb
 * MSM -- Added header() function to print centered table titles
 * MSM -- First version of tab_disp(): does not do global c- or p-uses
 * 
 * Revision 1.11  92/03/10  13:03:25  cs490sb
 * MSM -- fixed minor format bug in disp_upuse()
 * 
 * Revision 1.10  92/03/09  22:29:52  cs490sb
 * MSM -- added log to comments
 * 
 */

/* DEFINITIONS */

#define MAX_STRING 80           /* maximum field string length  */

#define CUSE_TITLE  "\nUncovered Local C-Uses -- File: %s Function: %s" 
#define PUSE_TITLE  "\nUncovered Local P-Uses -- File: %s Function: %s" 
#define BLOCK_TITLE "\nUncovered Blocks -- File: %s Function: %s" 
#define DECIS_TITLE "\nUncovered Decisions -- File: %s Function: %s" 
#define UCUSE_TITLE "\nUndefined C-Uses -- File: %s Function: %s"
#define UPUSE_TITLE "\nUndefined P-Uses -- File: %s Function: %s"

#define CUSE_HEAD1 \
"Var        Def  Segment                       Use  Segment\n"
#define CUSE_HEAD2 \
"---------- ---- ----------------------------- ---- ---------------------------\n"

#define PUSE_HEAD1 \
"Var        Def  Segment           Use  Segment           To   Segment\n"
#define PUSE_HEAD2 \
"---------- ---- ----------------- ---- ----------------- ---- ----------------\n"

#define BLOCK_HEAD1 "Start End  Segment\n"
#define BLOCK_HEAD2 \
"----- ---- -------------------------------------------------------------------\n"

#define DECIS_HEAD1 "Uncov Line Segment\n"
#define DECIS_HEAD2 \
"----- ---- -------------------------------------------------------------------\n"

#define UCUSE_HEAD1 \
"Var        Def  Use  Segment\n"
#define UCUSE_HEAD2 \
"---------- ---- ---- --------------------------------------------------------\n"

#define UPUSE_HEAD1 \
"Var        Def  Use  Segment                   To   Segment\n"
#define UPUSE_HEAD2 \
"---------- ---- ---- ------------------------- ---- -------------------------\n"

/* TYPE DEFINITIONS */

/* FUNCTION PROTOTYPES (DECLARATIONS) */

static void disp_cuse        /* display uncovered local c-uses       */
	P_((int first, char *file, char *func, char *var, T_BLK *def, T_BLK *use));
static void disp_puse        /* display uncovered local p-uses       */
	P_((int first, char *file, char *func, char *var, T_BLK *def, T_BLK *use, T_BLK *to));
static void disp_block       /* display uncovered blocks             */
	P_((int first, char *file, char *func, T_BLK *b1));
static void disp_decis       /* display uncovered decisions          */
	P_((int first, char *file, char *func, T_BLK *use, int cov));
#ifdef UNDEFINED_USES /* { */
static void disp_ucuse       /* display undefined local c-uses       */
	P_((int first, char *file, char *func, char *var, T_BLK *use));
static void disp_upuse       /* display undefined local p-uses       */
	P_((int first, char *file, char *func, char *var, T_BLK *use, T_BLK *to));
#endif /* UNDEFINED_USES } */
static void get_context      /* get a source file context            */
	P_((char *filename, T_BLK *block, char *str, int length));
static void right_pad        /* right-pad a string with spaces       */
	P_((char *to, char *from, size_t max));
static void header           /* print a centered header              */
	P_((char *title, char *file, char *func));

/* FUNCTIONS */

/* FUNCTION: void tab_disp(td_flag, global_cov, mod, n_mod)
 * int       td_flag -- indicator for criteria to display
 * int       global_cov -- indicator of global coverage selected
 * T_MODULE *modules -- pointer to the atac coverage structure
 * int       n_mod   -- the number of modules in the n_mod structure
 *
 * Function implements tabular disply of coverage data.
 */

void tab_disp(td_flag, global_cov, modules, n_mod, covVector)
int       td_flag;
int       global_cov;
T_MODULE *modules;
int       n_mod;
int	*covVector;
{
  int	     i;
  int	     j;
  T_MODULE   *mod;
  T_FUNC     *func;
  T_BLK      *blk;
  T_CUSE     *cuse;
  T_PUSE     *puse;
  T_PUSE     *puse2;
  T_PUSE     *dec_t, *dec_f;
#ifdef UNDEFINED_USES /* { */
  U_PUSE     *u_puse;
  U_CUSE     *u_cuse;
#endif /* UNDEFINED_USES } */

  int         first, printed;
  short       dec_use;
  int         dec_result;

  if ((td_flag & OPTION_CUSE) && !global_cov) 
   {
     first = TRUE;
     printed = FALSE;
     for (mod = modules; mod < modules+n_mod; mod++)
      for (func = mod->func; func < mod->func+mod->n_func; func++, first = TRUE)
       {
         if (func->ignore) continue;
         if (func->pos.start.file != func->pos.end.file) continue;
         for (i = 0; i < (int)func->n_cuse; ++i) {
	     cuse = func->cuse + i;
	     if (covVector[func->cUseCovStart + i] == 0) {
		 disp_cuse(first, 
		       mod->file[func->pos.start.file].filename,
                       func->fname,
                       func->var[cuse->varno].vname,
                       &func->blk[cuse->blk1],
                       &func->blk[cuse->blk2]);
		 first = FALSE;
		 printed = TRUE;
	     }
	 }
       }
     if (printed == FALSE)
      printf(" --- No Uncovered C-Uses ---\n");
   }
  else if ((td_flag & OPTION_CUSE) && global_cov)
   {
     printf("Current version does not support c-use display with global");
     printf("variable coverage.\n");
   }
  
  if ((td_flag & OPTION_PUSE) && !global_cov)
   {
     first = TRUE;
     printed = FALSE;
     for (mod = modules; mod < modules+n_mod; mod++)
      for (func = mod->func; func < mod->func+mod->n_func; func++, first = TRUE)
       {
         if (func->ignore) continue;
         if (func->pos.start.file != func->pos.end.file) continue;
         for (i = 0; i < (int)func->n_puse; ++i) {
	     puse = func->puse + i;
	     if (covVector[func->pUseCovStart + i] == 0
		 && (puse->varno != (unsigned short)func->decis_var))
	     {
		 disp_puse(first,
                       mod->file[func->pos.start.file].filename,
                       func->fname,
                       func->var[puse->varno].vname,
                       &func->blk[puse->blk1],
                       &func->blk[puse->blk2],
                       &func->blk[puse->blk3]);
		 first = FALSE;
		 printed = TRUE;
	     }
	 }
       }
     if (printed == FALSE)
      printf("--- No Uncovered P-Uses ---\n");
   }
  else if ((td_flag & OPTION_PUSE) && global_cov)
   {
     printf("Current version does not support p-use display with global");
     printf("variable coverage.\n");
   }

  if (td_flag & OPTION_BLOCK)
   {
     first = TRUE;
     printed = FALSE;
     for (mod = modules; mod < modules+n_mod; mod++)
      for (func = mod->func; func < mod->func+mod->n_func; func++, first = TRUE)
       {
         if (func->ignore) continue;
         if (func->pos.start.file != func->pos.end.file) continue;
	 for (i = 0; i < (int)func->n_blk; ++i) {
	     blk = func->blk + i;
	     if (covVector[func->blkCovStart + i] == 0) {
		 disp_block(first,
                       mod->file[func->pos.start.file].filename,
                       func->fname,
                       blk);
		 first = FALSE;
		 printed = TRUE;
	     }
	 }
       }
     if (printed == FALSE)
      printf("--- No Uncovered Blocks ---\n");
   }

  if (td_flag & OPTION_DECIS)
   {
     first = TRUE;
     printed = FALSE;
     for (mod = modules; mod < modules+n_mod; mod++)
      for (func = mod->func; func < mod->func+mod->n_func; func++, first = TRUE)
       {
         if (func->ignore) continue;
         if (func->pos.start.file != func->pos.end.file) continue;
         for (i = 0; i < (int)func->n_puse; ++i) {
	     puse = func->puse + i;
	     if ((puse->varno == (unsigned short)func->decis_var) &&
		 (puse->blk1 != (unsigned short)TD_CHECKED))
	     {
		 dec_use = puse->blk2;
		 dec_t = puse;
		 dec_t->blk1 = (unsigned short)TD_CHECKED;
		 dec_f = NULL;
		 for (j = i; j < (int)func->n_puse; ++j) {
		     puse2 = func->puse + j;
		     if ((puse2->varno == (unsigned short)func->decis_var) &&
			 (puse2->blk2 == (unsigned short)dec_use) &&
			 (puse2 != puse))
		     {
			 dec_f = puse2;
			 dec_f->blk1 = (unsigned short)TD_CHECKED;
			 break;
		     }
		 }
		 dec_result = TD_NEITHER;
		 if (covVector[func->pUseCovStart + i] == 0)
		     dec_result += TD_TRUE;
		 if (dec_f != NULL)
		   if (covVector[func->pUseCovStart + j] == 0)
		     dec_result += TD_FALSE;
		 if (dec_result != TD_NEITHER)
		 {
		     disp_decis(first,
			   mod->file[func->pos.start.file].filename,
			   func->fname,
                           &func->blk[dec_use],
                           dec_result);
		     first = FALSE;
		     printed = TRUE;
		 }
	     }
	 }
       }
    if (printed == FALSE)
     printf("--- No Uncovered Decisions---\n");
   }

#ifdef UNDEFINED_USES /* { */
   if (td_flag & TD_UNDEF)
     /* Programmed by that fuzzy guy: Erik Gerdes */
	for (mod = modules; mod < modules + n_mod; ++mod) 
                for (func = mod->func; func < mod->func + mod->n_func; 
                     ++func, first = TRUE) 
                {
                        if (func->ignore) continue;
			for (u_puse = func->u_puse, first = TRUE;
			     	u_puse != NULL; 
		             	u_puse = u_puse->next, first = FALSE)
		 	   disp_upuse(first,
				      mod->file[func->pos.start.file].filename,
				      func->fname,
				      func->var[u_puse->varno].vname, 
				      &(func->blk[u_puse->use_blk]),
				      &(func->blk[u_puse->to_blk]));
                        if ((u_puse == NULL) && (first == TRUE))
                          printf("--- No Undefined P-Uses ---\n");
			for (u_cuse = func->u_cuse, first = TRUE;
				   u_cuse != NULL; 
			  	   u_cuse = u_cuse->next, first = FALSE) 
		 	   disp_ucuse(first,
			  	      mod->file[func->pos.start.file].filename,
				      func->fname,
				      func->var[u_cuse->varno].vname, 
				      &(func->blk[u_cuse->use_blk]));
                        if ((u_cuse == NULL) && (first == TRUE))
                          printf("--- No Undefined C-Uses ---\n");

		}
#endif /* UNDEFINED_USES } */
	
}

/* FUNCTION: void disp_cuse(first, file, func, var, def, use)
 * int    first -- non-zero signifies "print table header"
 * char  *file  -- filename of the source text file
 * char  *func  -- function name for this c-use
 * char  *var   -- variable name for this c-use
 * T_BLK *def   -- pointer to def-block
 * T_BLK *use   -- pointer to use-block
 *
 * The tabular display routine for local c-uses.
 */

static void
disp_cuse(first, file, func, var, def, use)
int    first;
char  *file;
char  *func;
char  *var;
T_BLK *def;
T_BLK *use;
{
  char dstr[MAX_STRING+1];

  if (first)
   {
     header(CUSE_TITLE, file, func);
     printf(CUSE_HEAD1);
     printf(CUSE_HEAD2);
   }
  
  right_pad(dstr, var, 10);
  printf("%s ", dstr);
  if (def != NULL)
   {
     printf("%04d ", def->pos.start.line);
     get_context(file, def, dstr, 29);
     printf("%s ", dstr);
   }
  else
   printf("?                                  ");
  if (use != NULL)
   {
     printf("%04d ", use->pos.start.line);
     get_context(file, use, dstr, 27);
     printf("%s\n", dstr);
   }
  else
   printf("?\n");
}

/* FUNCTION: void disp_puse(first, file, func, var, def, use, to)
 * int    first -- non-zero signifies "print table header"
 * char  *file  -- filename of the source text file
 * char  *func  -- function name for this p-use
 * char  *var   -- variable name for this p-use
 * T_BLK *def   -- pointer to def-block
 * T_BLK *use   -- pointer to use-block
 * T_BLK *to    -- pointer to to-block
 *
 * The tabular display routine for local p-uses.
 */

static void
disp_puse(first, file, func, var, def, use, to)
int    first;
char  *file;
char  *func;
char  *var;
T_BLK *def;
T_BLK *use;
T_BLK *to;
{
  char dstr[MAX_STRING+1];

  if (first)
   {
     header(PUSE_TITLE, file, func);
     printf(PUSE_HEAD1);
     printf(PUSE_HEAD2);
   }

  right_pad(dstr, var, 10);
  printf("%s ", dstr);
  if (def != NULL)
   {
     printf("%04d ", def->pos.start.line);
     get_context(file, def, dstr, 17);
     printf("%s ", dstr);
   }
  else
   printf("?                      ");
  if (use != NULL)
   {
     printf("%04d ", use->pos.start.line);
     get_context(file, use, dstr, 17);
     printf("%s ", dstr);
   }
  else
   printf("?                      ");
  if (to != NULL)
   {
     printf("%04d ", to->pos.start.line);
     get_context(file, to, dstr, 16);
     printf("%s\n", dstr);
   }
  else
   printf("?\n");
}

/* FUNCTION: void disp_block(first, file, func, b1)
 * int    first -- non-zero signifies "print table header"
 * char  *file  -- filename of the source text file
 * char  *func  -- function name for this block
 * T_BLK *b1    -- pointer to use-block
 *
 * The tabular display routine for blocks.
 */

static void
disp_block(first, file, func, b1)
int    first;
char  *file;
char  *func;
T_BLK *b1;
{
  char dstr[MAX_STRING+1];

  if (first)
   {
     header(BLOCK_TITLE, file, func);
     printf(BLOCK_HEAD1);
     printf(BLOCK_HEAD2);
   }

  if (b1 != NULL)
   {
     printf("%04d  %04d ", b1->pos.start.line, b1->pos.end.line);
     get_context(file, b1, dstr, 67); 
     printf("%s\n", dstr);
   }
  else
   printf("?     ?\n");
}

/* FUNCTION: void disp_decis(first, file, func, use, cov)
 * int    first -- non-zero signifies "print table header"
 * char  *file  -- filename of the source text file
 * char  *func  -- function name for this decis
 * T_BLK *use   -- pointer to use-block
 * int    cov   -- coverage -- TD_TRUE, TD_FALSE, or TD_BOTH
 *
 * The tabular display routine for decisions.   
 */

static void
disp_decis(first, file, func, use, cov)
int    first;
char  *file;
char  *func;
T_BLK *use;
int    cov;
{
  char dstr[MAX_STRING+1];

  if (first)
   {
     header(DECIS_TITLE, file, func);
     printf(DECIS_HEAD1);
     printf(DECIS_HEAD2);
   }

  switch(cov)
   {
     case TD_TRUE : printf("TRUE  ");
                    break;
     case TD_FALSE: printf("FALSE ");
                    break;
     case TD_BOTH : printf("BOTH  ");
                    break;
     default      : printf("      ");
                    break;
   }
  if (use != NULL)
   {
     printf("%04d ", use->pos.start.line);
     get_context(file, use, dstr, 67);
     printf("%s\n", dstr);
   }
  else
   printf("?\n");
}

/* FUNCTION: void disp_ucuse(first, file, func, var, use)
 * int    first -- non-zero signifies "print table header"
 * char  *file  -- filename of the source text file
 * char  *func  -- function name for this c-use
 * char  *var   -- variable name for this c-use
 * T_BLK *use   -- pointer to use-block
 *
 * The tabular display routine for undefined c-uses.
 */

#ifdef UNDEFINED_USES /* { */
static void
disp_ucuse(first, file, func, var, use)
int    first;
char  *file;
char  *func;
char  *var;
T_BLK *use;
{
  char dstr[MAX_STRING+1];

  if (first)
   {
     header(UCUSE_TITLE, file, func);
     printf(UCUSE_HEAD1);
     printf(UCUSE_HEAD2);
   }
  
  right_pad(dstr, var, 10);
  printf("%s ?    ", dstr);
  if (use != NULL)
   {
     printf("%04d ", use->pos.start.line);
     get_context(file, use, dstr, 56);
     printf("%s\n", dstr);
   }
  else
   printf("?\n");
}

/* FUNCTION: void disp_upuse(first, file, func, var, use, to)
 * int    first -- non-zero signifies "print table header"
 * char  *file  -- filename of the source text file
 * char  *func  -- function name for this c-use
 * char  *var   -- variable name for this c-use
 * T_BLK *use   -- pointer to use-block
 * T_BLK *to    -- pointer to to-block
 *
 * The tabular display routine for local p-uses.
 */

static void
disp_upuse(first, file, func, var, use, to)
int    first;
char  *file;
char  *func;
char  *var;
T_BLK *use;
T_BLK *to;
{
  char dstr[MAX_STRING+1];

  if (first)
   {
     header(UPUSE_TITLE, file, func);
     printf(UPUSE_HEAD1);
     printf(UPUSE_HEAD2);
   }

  right_pad(dstr, var, 10);
  printf("%s ?    ", dstr);
  if (use != NULL)
   {
     printf("%04d ", use->pos.start.line);
     get_context(file, use, dstr, 25);
     printf("%s ", dstr);
   }
  else
   printf("?                              ");
  if (to != NULL)
   {
     printf("%04d ", to->pos.start.line);
     get_context(file, to, dstr, 24);
     printf("%s\n", dstr);
   }
  else
   printf("?\n");
}
#endif /* UNDEFINED_USES } */

/* FUNCTION: void get_context(filename, block, str, length)
 * char  *filename -- filename of the text source file 
 * T_BLK *block    -- pointer to block structure, gives file positions
 * char  *str      -- pointer to string of at least length+1 characters
 * int    length   -- maximum length of the string to return
 *
 * Function reads from source file given in filename, starting at the
 * begin row and column given in block, and stopping at the end row and
 * column given in block, or length characters, whichever is shorter.
 * Text is copied into str, and right padded with blanks.  str[length] is
 * set to NULL.
 */

static void
get_context(filename, block, str, length)
char  *filename;
T_BLK *block;
char  *str;
int    length;
{
  FILE *filep;                          /* the file to scan             */
  int   i, j;                           /* counter                      */
  int   c;                              /* character to read            */
  int   pos;                            /* position in str              */
  int   inspace;			/* whitspace elimination flag   */
  
  for (i = 0; i <length; i++)           /* fill string with blanks      */
   str[i] = ' ';
  str[length] = '\0';

  filep = fopen(filename, "r");         /* open the source file         */
  if (filep == NULL)
    return;

  i = 1;
  while (i < (int)block->pos.start.line) /* read down to the start line  */
   {
     while (((c = fgetc(filep)) != EOF) && (c != '\n'));
     if (c == EOF)
       {
         j = fclose(filep);
         return;
       }
     i++;
   }

  j = 1;                                /* read to pos.start.col            */
  while (j < block->pos.start.col)
   {
     c = fgetc(filep);
     if (c != EOF) j++;
     else 
      {
        j = fclose(filep);
        return;
      }
   }

  pos = 0;
  inspace = 0;
  do                                    /* get context from file        */
   {
     c = fgetc(filep);
     if (c == EOF)
      {
        j = fclose(filep);
        return;
      }
     j++;
     if (c == '\n')
      {
        i++;
        j = 1;
      }
     if (isspace(c))
        inspace = 1;
     else
      {
        if (inspace == 1)
         {
           inspace = 0;
           str[pos++] = ' ';
         }
        if (pos != length)
          str[pos++] = c; 
      }
     if ((i == block->pos.end.line) && (j == block->pos.end.col))
      {
        c = fgetc(filep);
        if ((c != EOF) && (pos != length))
         str[pos++] = c;
        pos = length;
      }
   }
  while (pos != length);

  str[length+1] = '\0';
  j = fclose(filep);
  return;
}

/* FUNCTION: void right_pad(to, from, max)
 * char *to -- destination string
 * char *from -- source string
 * size_t   max  -- maximum string length
 *
 * Function copies max characters from from to to, padding extra spaces.
 */

static void
right_pad(to, from, max)
char *to;
char *from;
size_t   max;
{
  int j;

  strncpy(to, from, max);
  if (strlen(from) < max)
   {
     for (j = strlen(to); j < max; j++)
      to[j] = ' ';
     to[max] = '\0';
   }
  else
   to[max] = '\0';
}

static void
header(title, file, func)
char	*title;
char    *file;
char    *func;
{
	int	spaces;
	int	i;
	char	*p;
        char    buf1[MAX_STRING];
        char    buf2[MAX_STRING];

        sprintf(buf1, title, file, func); 
	spaces = (78 - strlen(buf1)) / 2;
	p = buf2;
	for (i = 0; i < spaces; ++i) *p++ = ' ';
	sprintf(p, "%s", buf1);
	p = buf2 + strlen(buf2);
	for (i = 0; i < spaces; ++i) *p++ = ' ';
	*p = '\0';
        printf("%s\n", buf2);
}
