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
#ifndef tree_H
#define tree_H
static const char tree_h[] = "$Id: tree.h,v 3.7 2013/12/08 22:02:11 tom Exp $";
/*
* @Log: tree.h,v @
* Revision 3.6  1996/11/12 11:05:42  tom
* change ident to 'const' to quiet gcc
*
* Revision 3.5  1995/12/27 23:04:00  tom
* declare CLASSTYPE_INLINE
*
* Revision 3.4  94/06/01  09:02:25  saul
* fix for ANSI f(...) 
* 
* Revision 3.3  94/04/04  10:15:17  jrh
* Add Release Copyright
* 
* Revision 3.2  94/03/21  08:22:12  saul
* MVS support __offsetof as builtin (not handled by cpp)
* 
* Revision 3.1  93/11/19  12:15:05  saul
* MVS support for _Packed
* 
* Revision 3.0  92/11/06  07:45:45  saul
* propagate to version 3.0
* 
* Revision 2.6  92/04/27  11:18:36  saul
* Beefed up comments.
* 
* Revision 2.5  92/03/17  14:23:10  saul
* copyright
* 
* Revision 2.4  91/10/23  13:19:11  saul
* Handle "*const volitile"
* 
* Revision 2.3  91/10/23  12:33:37  saul
* Allow empty declaration list as in "int;"
* 
* Revision 2.2  91/06/13  13:02:19  saul
* Changes for ansi grammar.
* Incorporate internal grammar (gram) description.
* 
* Revision 2.1  91/06/13  12:39:29  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:55  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
/*
* This file defines the genus and species values that are used to classify
* parse tree nodes.  Each genus may have one or more species.  The species for
* a genus are defined under the genus definition numbered starting from 0.  If
* there is only one species for a genus, the species is number 0 and the
* definition is ommited.  The comments indicate the parser rule represented by
* each species.
*
* Species numbers are not unique, so we have to be carefull to assign or
* compare genus and species together.  It would have been nicer if
* genus/species were encoded as a single value.  A GENUS() macro could have
* been used to get the genus part.  To do this now would require changes to
* this file as well as Pgram.y where the tree nodes are built, deparse.c where
* genus and species are used as indices, and everywhere that genus is
* referenced.  This is complicated by the fact that GEN_INDATA_DCL and
* GEN_DATA_ITEM share species with GEN_INIT_DCL and GEN_FUNC_SPEC.
*/
#define GEN_MODULE	0			/* MODULE_ITEM*					*/
#define GEN_MODULE_ITEM	1
#define		DCL_ITEM		0	/* INIT_DCL					*/
#define		FUNCTION_ITEM		1	/* FUNCTION					*/
#define GEN_FUNCTION	2
#define		FUNC_TFPC		0	/* CLASSTYPES FUNC_SPEC PARAM_DCLS COMPSTMT	*/
#define		FUNC_TFC		1	/* CLASSTYPES FUNC_SPEC COMPSTMT		*/
#define		FUNC_FPC		2	/* FUNC_SPEC PARAM_DCLS COMPSTMT		*/
#define		FUNC_FC			3	/* FUNC_SPEC COMPSTMT				*/
#define GEN_FUNC_SPEC	3
#define		FUNC_STARS_SPEC		0	/* STARS FUNC_SPEC				*/
#define		FUNC_SPEC_NFCALL	1	/* FUNC_SPEC ( )				*/
#define		FUNC_SPEC_ARRAY_EXPR	2	/* FUNC_SPEC [ EXPR ]				*/
#define		FUNC_SPEC_ARRAY		3	/* FUNC_SPEC [ ]				*/
#define		FUNC_SPEC_INHERIT	4	/* ( FUNC_SPEC )				*/
#define		FUNC_FCALL_NAMES	5	/* FNAME ( NAMES )				*/
#define		FUNC_FCALL		6	/* FNAME ( )					*/
#define		FUNC_FCALL_ANSI		7	/* FNAME ( ANSI_PARAMS )			*/
#define		FUNC_FCALL_ANSI_E	8	/* FNAME ( ANSI_PARAMS , ... )			*/
#define		FUNC_SPEC_ANSI		9	/* FUNC_SPEC ( ANSI_PARAMS )			*/
#define		FUNC_SPEC_ANSI_E	10	/* FUNC_SPEC ( ANSI_PARAMS , ... )		*/
#define		FUNC_FCALL_E_ANSI	11	/* FNAME ( ... )				*/
#define		FUNC_SPEC_E_ANSI	12	/* FUNC_SPEC ( ... )				*/
#define	GEN_CLASSTYPES	4			/* CLASSTYPE*					*/
#define		CLASSTYPES_NORMAL	0
#define		CLASSTYPES_TYPEDEF	1
#define GEN_CLASSTYPE	5
#define		CLASSTYPE_INT		0	/* int						*/
#define		CLASSTYPE_CHAR		1	/* char						*/
#define		CLASSTYPE_FLOAT		2	/* float					*/
#define		CLASSTYPE_DOUBLE	3	/* double					*/
#define		CLASSTYPE_LONG		4	/* long						*/
#define		CLASSTYPE_SHORT		5	/* short					*/
#define		CLASSTYPE_UNSIGNED	6	/* unsigned					*/
#define		CLASSTYPE_SIGNED	7	/* signed					*/
#define		CLASSTYPE_VOID		8	/* void						*/
#define		CLASSTYPE_CONST		9	/* const					*/
#define		CLASSTYPE_VOLATILE	10	/* volatile					*/
#define		CLASSTYPE_AUTO		11	/* auto						*/
#define		CLASSTYPE_REGISTER	12	/* register					*/
#define		CLASSTYPE_STATIC	13	/* static					*/
#define		CLASSTYPE_EXTERN	14	/* extern					*/
#define		CLASSTYPE_TYPEDEF	15	/* typedef					*/
#define		CLASSTYPE_TNAME		16	/* TNAME					*/
#define		CLASSTYPE_STRUCT_D	17	/* STRUCT_DCL					*/
#define		CLASSTYPE_ENUM_D	18	/* ENUM_DCL					*/
#define		CLASSTYPE_STRUCT_R	19	/* STRUCT_REF					*/
#define		CLASSTYPE_ENUM_R	20	/* ENUM_REF					*/
#define		CLASSTYPE_INLINE	21	/* inline					*/
#define GEN_PARAM_DCLS	6			/* PARAM_DCL*					*/
#define GEN_PARAM_DCL	7			/* CLASSTYPES PARAM_DEFS ;			*/
#define GEN_PARAM_DEFS	8			/* DATA_ITEM*					*/
#define GEN_STARS	9			/* STAR*					*/
#define GEN_STAR	10
#define		STAR_NORMAL	0		/* *						*/
#define		STAR_QUALS	1		/* * QUALS					*/
#define GEN_STMT_LIST	11			/* STMT*					*/
#define GEN_ENUM_DCL	12
#define		sENUM_NOTAG		0	/* enum { MOE_LIST }				*/
#define		sENUM_TAG		1	/* enum NAME { MOE_LIST }			*/
#define GEN_ENUM_REF	13			/* enum NAME					*/
#define GEN_MOE_LIST	14			/* MOE*						*/
#define GEN_MOE		15
#define		MOE_VAL		0		/* NAME = EXPR					*/
#define		MOE_NOVAL	1		/* NAME						*/
#define GEN_STRUCT_DCL	16
#define		DCL_STRUCT_TAG		0	/* struct NAME { MEM_LIST }			*/
#define		DCL_STRUCT_NOTAG	1	/* struct { MEM_LIST }				*/
#define		DCL_UNION_TAG		2	/* union NAME { MEM_LIST }			*/
#define		DCL_UNION_NOTAG		3	/* union { MEM_LIST }				*/
#define		DCL_PSTRUCT_TAG		4	/* _Packed struct NAME { MEM_LIST }	(MVS)	*/
#define		DCL_PSTRUCT_NOTAG	5	/* _Packed struct { MEM_LIST }		(MVS)	*/
#define		DCL_PUNION_TAG		6	/* _Packed union NAME { MEM_LIST }	(MVS)	*/
#define		DCL_PUNION_NOTAG	7	/* _Packed union { MEM_LIST }		(MVS)	*/
#define GEN_STRUCT_REF	17
#define		REF_STRUCT	0		/* struct NAME					*/
#define		REF_UNION	1		/* union NAME					*/
#define		REF_PSTRUCT	2		/* _Packed struct NAME			(MVS)	*/
#define		REF_PUNION	3		/* _Packed union NAME			(MVS)	*/
#define GEN_MEM_LIST	18			/* MEMBER*					*/
#define GEN_MEMBER	19			/* CLASSTYPES MEM_DCLS ;			*/
#define GEN_MEM_DCLS	20			/* MEM_DCL*					*/
#define GEN_MEM_DCL	21
#define		MEM_DCL_BIT	0		/* DATA_ITEM : EXPR				*/
#define		MEM_DCL		1		/* DATA_ITEM					*/
#define		MEM_BIT		2		/* : EXPR					*/
#define GEN_NAMES	22			/* NAME*					*/
#define GEN_INIT_DCL	23
#define		INIT_DCL_SPEC		0	/* CLASSTYPES DATA_SPECS ;			*/
#define		INIT_DCL_NOSPEC		1	/* CLASSTYPES ;					*/
#define		INIT_DCL_EMPTY		2	/* ;						*/
#define GEN_INDATA_DCLS	24			/* INDATA_DCL*					*/
#define GEN_INDATA_DCL	25
/*		INIT_DCL_SPEC		0	   CLASSTYPES DATA_SPECS ;			*/
#define GEN_DATA_SPECS	26			/* DATA_SPEC*					*/
#define GEN_DATA_SPEC	27
#define		DATA_SPEC_INIT		0	/* DATA_ITEM INITIALIZER			*/
#define		DATA_SPEC		1	/* DATA_ITEM 					*/
#define GEN_DATA_ITEMS	28			/* DATA_ITEM*					*/
#define GEN_DATA_ITEM	29
/*		FUNC_STARS_SPEC		0	   STARS DATA_ITEM				*/
/*		FUNC_SPEC_NFCALL	1	   DATA_ITEM ( )				*/
/*		FUNC_SPEC_ARRAY_EXPR	2	   DATA_ITEM [ EXPR ]				*/
/*		FUNC_SPEC_ARRAY		3	   DATA_ITEM [ ] 				*/
/*		FUNC_SPEC_INHERIT	4	   ( DATA_ITEM )				*/
/*		FUNC_FCALL_NAMES	5	   FNAME ( NAMES )				*/
/*		FUNC_FCALL		6	   FNAME ( )					*/
/*		FUNC_FCALL_ANSI		7	   FNAME ( ANSI_PARAMS )			*/
/*		FUNC_FCALL_ANSI_E	8	   FNAME ( ANSI_PARAMS , ... )			*/
/* 		FUNC_SPEC_ANSI		9	   FUNC_SPEC ( ANSI_PARAMS )			*/
/* 		FUNC_SPEC_ANSI_E	10	   FUNC_SPEC ( ANSI_PARAMS , ... )		*/
/*		FUNC_FCALL_E_ANSI	11	   FNAME ( ... )				*/
/* 		FUNC_SPEC_E_ANSI	12	   FUNC_SPEC ( ... )				*/
#define		DATA_NAME		13	/* NAME						*/
#define GEN_INIT_LIST	30			/* INIT_ITEM*					*/
#define GEN_INIT_ITEM	31
#define		INIT_EXPR		0	/* EXPR						*/
#define		INIT_LIST		1	/* { INIT_LIST }				*/
#define GEN_INITIALIZER	32
#define		INITIALIZER_EXPR	0	/* = EXPR					*/
#define		INITIALIZER_LIST	1	/* = { INIT_LIST }				*/
/*		INITIALIZER_LIST	1	** = { INIT_LIST , }				*/
#define GEN_COMPSTMT	33
#define		COMPSTMT_DCL_STMTS	0	/* { INDATA_DCLS STMT_LIST }			*/
#define		COMPSTMT_STMTS		1	/* { STMT_LIST }				*/
#define		COMPSTMT_DCL		2	/* { INDATA_DCLS }				*/
#define		COMPSTMT_EMPTY		3	/* { }						*/
#define GEN_STMT	34
#define		STMT_EXPR	0		/* EXPR ;					*/
#define		STMT_EMPTY	1		/* ;						*/
#define		STMT_COMPSTMT	2		/* COMPSTMT 					*/
#define		STMT_IF_ELSE	3		/* if ( EXPR ) STMT else STMT			*/
#define		STMT_IF		4		/* if ( EXPR ) STMT				*/
#define		STMT_WHILE	5		/* while ( EXPR ) STMT				*/
#define		STMT_DO		6		/* do STMT  while ( EXPR ) ;			*/
#define		STMT_FOR_EEES	7		/* for ( EXPR ; EXPR ; EXPR ) STMT		*/
#define		STMT_FOR_EEE_	8		/* for ( EXPR ; EXPR ; EXPR ) ;			*/
#define		STMT_FOR_EE_S	9		/* for ( EXPR ; EXPR ; ) STMT			*/
#define		STMT_FOR_EE__	10		/* for ( EXPR ; EXPR ; ) ;			*/
#define		STMT_FOR_E_ES	11		/* for ( EXPR ; ; EXPR ) STMT			*/
#define		STMT_FOR_E_E_	12		/* for ( EXPR ; ; EXPR ) ;			*/
#define		STMT_FOR_E__S	13		/* for ( EXPR ; ; ) STMT 			*/
#define		STMT_FOR_E___	14		/* for ( EXPR ; ; ) ;				*/
#define		STMT_FOR__EES	15		/* for ( ; EXPR ; EXPR ) STMT			*/
#define		STMT_FOR__EE_	16		/* for ( ; EXPR ; EXPR ) ;			*/
#define		STMT_FOR__E_S	17		/* for ( ; EXPR ; ) STMT 			*/
#define		STMT_FOR__E__	18		/* for ( ; EXPR ; ) ;				*/
#define		STMT_FOR___ES	19		/* for ( ; ; EXPR ) STMT			*/
#define		STMT_FOR___E_	20		/* for ( ; ; EXPR ) ;				*/
#define		STMT_FOR____S	21		/* for ( ; ; ) STMT				*/
#define		STMT_DUMMYSW	22		/* switch ( EXPR ) { CASE_LIST DEFAULT }	*/
#define		STMT_SWITCH	23		/* switch ( EXPR ) STMT				*/
#define		STMT_BREAK	24		/* break ;					*/
#define		STMT_CONTINUE	25		/* continue ;					*/
#define		STMT_RETURN_EXPR	26	/* return EXPR ;				*/
#define		STMT_RETURN	27		/* return ;					*/
#define		STMT_GOTO	28		/* goto NAME ;					*/
#define		STMT_LABEL	29		/* NAME : STMT					*/
#define		STMT_CASE	30		/* case EXPR : STMT				*/
#define		STMT_DEFAULT	31		/* default : STMT				*/
#define GEN_EXP_LIST	35			/* EXPR*					*/
#define GEN_EXPR	36
#define		EXPR_QCOLON	0		/* EXPR ? EXPR : EXPR				*/
#define		EXPR_COMMA	1		/* EXPR , EXPR					*/
#define		EXPR_BINOP	2		/* EXPR BINOP EXPR				*/
#define		EXPR_UNOP	3		/* UNOP EXPR					*/
#define		EXPR_INCOP	4		/* EXPR INCOP					*/
#define		EXPR_LSTAR	5		/* STARS EXPR					*/
#define		EXPR_LARRAY	6		/* EXPR [ EXPR ]				*/
#define		EXPR_LARROW	7		/* EXPR -> NAME					*/
#define		EXPR_LDOT	8		/* EXPR . NAME					*/
#define		EXPR_LFCALL1	9		/* FUNC_LP ( EXP_LIST )				*/
#define		EXPR_LFCALL0	10		/* FUNC_LP ( )					*/
#define		EXPR_CAST	11		/* ( CAST_TYPE ) EXPR				*/
#define		EXPR_SIZEOF	12		/* sizeof EXPR					*/
#define		EXPR_SIZEOF_TYPE	13	/* sizeof ( CAST_TYPE )				*/
#define		EXPR_LNAME	14		/* NAME						*/
#define		EXPR_ICON	15		/* ICON						*/
#define		EXPR_FCON	16		/* FCON						*/
#define		EXPR_STRING	17		/* STRINGS					*/
#define		EXPR_INHERIT	18		/* ( EXPR )					*/
#define 	EXPR_OFFSET	19		/* __offset(expr, expr)				*/ /* MVS */
#define GEN_FUNC_LP	37
#define		FUNC_NAME_LP	0		/* FNAME					*/
#define		FUNC_EXPR_LP	1		/* EXPR						*/
#define GEN_CAST_TYPE	38
#define		CAST_TYPE	0		/* CLASSTYPES NULL_DCL				*/
#define		CAST_TYPE_NULL	1		/* CLASSTYPES 					*/
#define GEN_NULL_DCL	39
#define		NULL_N_FUNC	0		/* NULL_DCL ( )					*/
#define		NULL_INHERIT	1		/* ( )						*/
#define		NULL_STAR_N	2		/* STAR NULL_DCL				*/
#define		NULL_STAR	3		/* STAR						*/
#define		NULL_N		4		/* NULL_DCL					*/
#define		NULL_N_SUB_E	5		/* NULL_DCL [ EXPR ]				*/
#define		NULL_N_SUB	6		/* NULL_DCL [ ]					*/
#define		NULL_SUB_E	7		/* [ EXPR ]					*/
#define		NULL_SUB	8		/* [ ]						*/
#define		NULL_INHERIT_N	9		/* ( NULL_DCL )					*/
#define		NULL_ANSI	10		/* NULL_DCL ( ansi_params )			*/
#define		NULL_ANSI_E	11		/* NULL_DCL ( ansi_params ... )			*/
#define		NULL_E_ANSI	12		/* NULL_DCL ( ... )				*/
#define GEN_BINOP	40
#define		BINOP_PLUS	0		/* +						*/
#define		BINOP_MINUS	1		/* -						*/
#define		BINOP_MUL	2		/* *						*/
#define		BINOP_DIV	3		/* /						*/
#define		BINOP_MOD	4		/* %						*/
#define		BINOP_LS	5		/* <<						*/
#define		BINOP_RS	6		/* >>						*/
#define		BINOP_AND	7		/* &						*/
#define		BINOP_OR	8		/* |						*/
#define		BINOP_ER	9		/* ^						*/
#define		BINOP_ASGN	10		/* =						*/
#define		BINOP_APLUS	11		/* +=						*/
#define		BINOP_AMINUS	12		/* -=						*/
#define		BINOP_AMUL	13		/* *=						*/
#define		BINOP_ADIV	14		/* /=						*/
#define		BINOP_AMOD	15		/* %=						*/
#define		BINOP_ALS	16		/* <<=						*/
#define		BINOP_ARS	17		/* >>=						*/
#define		BINOP_AAND	18		/* &=						*/
#define		BINOP_AOR	19		/* |=						*/
#define		BINOP_AER	20		/* ^=						*/
#define		BINOP_ANDAND	21		/* &&						*/
#define		BINOP_OROR	22		/* ||						*/
#define		BINOP_EQ	23		/* ==						*/
#define		BINOP_GTEQ	24		/* >=						*/
#define		BINOP_LTEQ	25		/* <=						*/
#define		BINOP_NEQ	26		/* !=						*/
#define		BINOP_LT	27		/* <						*/
#define		BINOP_GT	28		/* >						*/
#define GEN_INCOP	41
#define		INCOP_INC	0		/* ++						*/
#define		INCOP_DEC	1		/* --						*/
#define GEN_UNOP	42
#define		UNOP_AND	0		/* &						*/
#define		UNOP_MINUS	1		/* -						*/
#define		UNOP_INC	2		/* ++						*/
#define		UNOP_DEC	3		/* --						*/
#define		UNOP_NOT	4		/* !						*/
#define		UNOP_COMPL	5		/* ~						*/
#define GEN_ANSI_PARAMS	43			/* ANSI_PARAM*					*/
#define GEN_ANSI_PARAM	44
#define		ANSI_DATA_ITEM	0		/* CLASSTYPES DATA_ITEM				*/
#define		ANSI_NULL_DCL	1		/* CLASSTYPES NULL_DCL				*/
#define GEN_QUALS	45			/* QUAL*					*/
#define GEN_QUAL	46
#define		QUAL_CONST	0		/* const					*/
#define		QUAL_VOLATILE	1		/* volatile					*/
#define GEN_STRINGS	47			/* STRING* 					*/
#define GEN_FCON	48			/* float literal 				*/
#define GEN_ICON	49			/* integer literal 				*/
#define GEN_STRING	50			/* quoted string 				*/
#define GEN_NAME	51			/* symbol name 					*/
#define GEN_TNAME	52			/* symbol name in typedef table			*/
#define GEN_FNAME	53			/* function symbol name				*/

#define GENUS_COUNT	54	/* Must be larger than largest GEN_. */
#endif /* tree_H */
