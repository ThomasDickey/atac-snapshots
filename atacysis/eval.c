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
static char eval_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/eval.c,v 3.4 1994/04/04 10:25:14 jrh Exp $";
/*
*-----------------------------------------------$Log: eval.c,v $
*-----------------------------------------------Revision 3.4  1994/04/04 10:25:14  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.4  94/04/04  10:25:14  jrh
*Add Release Copyright
*
*Revision 3.3  93/08/09  12:36:15  saul
*bug in -n feature
*
*Revision 3.2  1993/08/04  15:53:54  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
*Revision 3.1  93/06/30  15:15:21  saul
*atac -N experimental feature
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include <ctype.h>
#include "portable.h"
#include "atacysis.h"

/* forward declarations */
static int evalConj();
static int evalDisj();
static int evalTerm();
static int evalNames();
static void doConj();
static void doDisj();
static void doNot();
static void readTrace();

typedef int	*value;

struct tr {
    char	**traces;
    int		n_traces;
    T_MODULE	*mod;
    int		n_static;
    int		covCount;
    int		options;
    int		threshold;
    int		weaker;
};

static void
readTrace(tr, names, covPtr)
struct tr	*tr;
char		*names;
value		*covPtr;
{
    struct cfile	*cf;
    int			nSelect;
    T_TESTLIST		*selectList;
    int			i;
    value		cov;

    nSelect = 0;	/* No tests seen so far. */

    /*
    * Read trace data and create selectList.
    */
    cf = (struct cfile *)cf_openIn(tr->traces[0]);
    if (cf == NULL) {
	fprintf(stderr, "can't open %s\n", tr->traces[0]);
	exit(1);
    }
    selectList = NULL;
    trace_data(cf, tr->traces[0], tr->mod, tr->n_static, tr->covCount,
	       tr->options,  names, &selectList, &nSelect);
    cf_close(cf);
    if (nSelect == 1) {
	covThreshold(selectList[0].cov, tr->covCount, tr->threshold);
	if (tr->weaker) {
	    covWeaker(tr->mod, tr->n_static, selectList[0].cov,
		      tr->options);
	}
	*covPtr = selectList[0].cov;
	free(selectList);
    } else {
	if (nSelect > 1) {
	    fprintf(stderr, "%d: Huh? nSelect should be 1\n", nSelect);
	    exit(1);
	}
	if (selectList != NULL) {
	    fprintf(stderr, "Warning: selectList not NULL\n");
	    selectList = NULL;
	}
	cov = (value)malloc(tr->covCount * sizeof *cov);
	if (cov == NULL) {
	    fprintf(stderr, "Can't malloc\n");
	    exit(1);
	}
	for (i = 0; i < tr->covCount; ++i) cov[i] = 0;
	*covPtr = cov;
    }

    return;
}

static void
doNot(cov, covCount)
int	*cov;
int	covCount;
{
    int j;

    for (j = 0; j < covCount; ++j) {
	if (cov[j] != -1) {
	    cov[j] = !cov[j];
	}
    }
}

static void
doDisj(cov1, cov2, covCount)
int	*cov1;
int	*cov2;
int	covCount;
{
    int j;

    for (j = 0; j < covCount; ++j) {
	if (cov1[j] == 1 && cov2[j] == 0) {
	    cov1[j] = 0;
	}
    }
}

static void
doConj(cov1, cov2, covCount)
int	*cov1;
int	*cov2;
int	covCount;
{
    int j;

    for (j = 0; j < covCount; ++j) {
	if (cov1[j] == 0 && cov2[j] == 1) {
	    cov1[j] = 1;
	}
    }
}

static int
evalNames(s, tr, lenPtr, covPtr)
char		*s;
struct tr	*tr;
int		*lenPtr;
value		*covPtr;
{
    int		len;
    int		namesStart;
    char	tmpC;

    len = 0;

    while (isspace(s[len])) ++len;

    namesStart = len;
    while(!isspace(s[len])
	  && s[len] != '\0'
	  && s[len] != '('
	  && s[len] != ')'
	  && s[len] != '!'
	  && s[len] != '&'
	  && s[len] != '+')
    {
	if (s[len] == '[' && s[len+1] == '!')
	    ++len; /* In this context '!' is name matching, not set comp. */
	++len;
    }

    if (len == namesStart) {
	fprintf(stderr, "missing names\n");
	return -1;
    }
    tmpC = s[len];
    s[len] = '\0';
    readTrace(tr, s + namesStart, covPtr);
    s[len] = tmpC;

    *lenPtr = len;

    return 0;
}

static int
evalTerm(s, tr, lenPtr, covPtr)
char		*s;
struct tr	*tr;
int		*lenPtr;
value		*covPtr;
{
    int		len;
    int		skip;

    len = 0;

    while (isspace(s[len])) ++len;

    switch (s[len])
    {
    case '\0':
	fprintf(stderr, "empty subexpression\n");
	return -1;
    case '(':
	++len;
	if (evalConj(s + len, tr, &skip, covPtr) == -1)
	    return -1;
	len += skip;
	while (isspace(s[len])) ++len;
	if (s[len] != ')') {
	    fprintf(stderr, "unballanced parenthesis\n");
	    free(*covPtr);
	    return -1;
	}
	++len;
	*lenPtr = len;
	break;
    case '!':
	++len;
	if (evalTerm(s + len, tr, &skip, covPtr) == -1)
	    return -1;
	len += skip;
	doNot(*covPtr, tr->covCount);
	*lenPtr = len;
	break;
    default:
	if (evalNames(s + len, tr, &skip, covPtr) == -1)
	    return -1;
	len += skip;
	*lenPtr = len;
	break;
    }
    return 0;
}

static int
evalDisj(s, tr, lenPtr, covPtr)
char		*s;
struct tr	*tr;
int		*lenPtr;
value		*covPtr;
{
    int		len;
    int		skip;
    value	cov1;
    value	cov2;

    len = 0;

    if (evalTerm(s + len, tr, &skip, &cov1) == -1)
	return -1;
    len += skip;

    while (isspace(s[len])) ++len;

    while (s[len] == '&') {
	++len;
	if (evalTerm(s + len, tr, &skip, &cov2) == -1) {
	    free(cov1);
	    return -1;
	}
	len += skip;
	doDisj(cov1, cov2, tr->covCount);
	free(cov2);
	while (isspace(s[len])) ++len;
    }

    *covPtr = cov1;
    *lenPtr = len;
    return 0;
}

static int
evalConj(s, tr, lenPtr, covPtr)
char		*s;
struct tr	*tr;
int		*lenPtr;
value		*covPtr;
{
    int		len;
    int		skip;
    value	cov1;
    value	cov2;

    len = 0;

    if (evalDisj(s + len, tr, &skip, &cov1) == -1)
	return -1;
    len += skip;

    while (isspace(s[len])) ++len;

    while (s[len] == '+') {
	++len;
	if (evalDisj(s + len, tr, &skip, &cov2) == -1) {
	    free(cov1);
	    return -1;
	}
	len += skip;
	doConj(cov1, cov2, tr->covCount);
	free(cov2);
	while (isspace(s[len])) ++len;
    }

    *covPtr = cov1;
    *lenPtr = len;
    return 0;
}

int
evalExpr(s, traces, n_traces, mod, n_static, covCount, options, selectListPtr,
	 nSelectPtr, threshold, weaker)
char		*s;
char		**traces;
int		n_traces;
T_MODULE	*mod;
int		n_static;
int		covCount;
int		options;
T_TESTLIST	**selectListPtr;   	/* return selectList */
int		*nSelectPtr;
int		threshold;
int		weaker;
{
    int		skip;
    struct tr	tr;
    value	cov;
    T_TESTLIST	*selectList;

    tr.traces = traces;
    tr.n_traces = n_traces;
    tr.mod = mod;
    tr.n_static = n_static;
    tr.covCount = covCount;
    tr.options = options;
    tr.threshold = threshold;
    tr.weaker = weaker;

    if (evalConj(s, &tr, &skip, &cov) == -1)
	return -1;

    if (s[skip] != '\0') {
	fprintf(stderr, "unexpected stuff at end: %s\n", s + skip);
	return -1;
    }

    selectList = (T_TESTLIST *)malloc(sizeof *selectList);
    if (selectList == NULL) {
	fprintf(stderr, "Can't malloc\n");
	exit(1);
    }

    selectList->cov = cov;
    selectList->cost = 1;
    selectList->name[0] = '\0';

    *selectListPtr = selectList;
    *nSelectPtr = 1;

    return 0;
}
