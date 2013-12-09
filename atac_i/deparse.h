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
/*
* $Id: deparse.h,v 3.7 2013/12/08 23:46:10 tom Exp $
* @Log: deparse.h,v @
* Revision 3.5  1995/12/13 01:20:14  tom
* handle CLASSTYPE_INLINE
*
* Revision 3.4  94/06/01  09:01:46  saul
* fix for ANSI f(...)
* 
* Revision 3.3  94/04/04  10:12:18  jrh
* Add Release Copyright
* 
* Revision 3.2  94/03/21  08:19:11  saul
* MVS support __offsetof as builtin (not handled by cpp)
* 
* Revision 3.1  93/11/19  12:14:08  saul
* MVS support for _Packed
* 
* Revision 3.0  92/11/06  07:45:33  saul
* propagate to version 3.0
* 
* Revision 2.5  92/03/17  14:22:20  saul
* copyright
* 
* Revision 2.4  91/10/23  13:21:15  saul
* Handle "*const volatile".
* 
* Revision 2.3  91/10/23  12:33:10  saul
* Allow empty declaration list as in "int;"
* 
* Revision 2.2  91/06/13  13:24:33  saul
* ansi grammar
* put RCS header in last entry.
* 
* Revision 2.1  91/06/13  12:38:59  saul
* Propagate to version 2.0
* 
* Revision 1.3  91/06/12  21:13:13  saul
* remove extra #endif
* 
* Revision 1.2  91/06/12  21:10:44  saul
* change rcs header (not a standard .h file)
* 
* Revision 1.1  91/06/12  20:25:37  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#ifndef deparse_H
#define deparse_H

	{GEN_MODULE,	0,	"@L\n@@"},
	{GEN_MODULE_ITEM,	DCL_ITEM,	"@N\n"},
	{GEN_MODULE_ITEM,	FUNCTION_ITEM,	"@N\n"},
	{GEN_FUNCTION,	FUNC_TFPC,	"@N\n@S\n@S\n@S"},
	{GEN_FUNCTION,	FUNC_TFC,	"@N\n@S\n@S"},
	{GEN_FUNCTION,	FUNC_FPC,	"@N\n@S\n@S"},
	{GEN_FUNCTION,	FUNC_FC,	"@N\n@S"},
	{GEN_FUNC_SPEC,	FUNC_STARS_SPEC,	"@N@S"},
	{GEN_FUNC_SPEC,	FUNC_SPEC_NFCALL,	"@N()"},
	{GEN_FUNC_SPEC,	FUNC_SPEC_ARRAY_EXPR,	"@N[@S]"},
	{GEN_FUNC_SPEC,	FUNC_SPEC_ARRAY,	"@N[]"},
	{GEN_FUNC_SPEC,	FUNC_SPEC_INHERIT,	"(@N)"},
	{GEN_FUNC_SPEC,	FUNC_FCALL_NAMES,	"@N(@S)"},
	{GEN_FUNC_SPEC,	FUNC_FCALL,	"@N()"},
	{GEN_FUNC_SPEC,	FUNC_FCALL_ANSI,	"@N(@S)"},
	{GEN_FUNC_SPEC,	FUNC_FCALL_ANSI_E,	"@N(@S, ...)"},
	{GEN_FUNC_SPEC,	FUNC_SPEC_ANSI,	"@N(@S)"},
	{GEN_FUNC_SPEC,	FUNC_SPEC_ANSI_E,	"@N(@S, ...)"},
	{GEN_FUNC_SPEC,	FUNC_FCALL_E_ANSI,	"@N(...)"},
	{GEN_FUNC_SPEC,	FUNC_SPEC_E_ANSI,	"@N(...)"},
	{GEN_CLASSTYPES,0,		"@L @@"},
	{GEN_CLASSTYPE, CLASSTYPE_INT,		"int"},
	{GEN_CLASSTYPE,	CLASSTYPE_CHAR,		"char"},
	{GEN_CLASSTYPE,	CLASSTYPE_FLOAT,	"float"},
	{GEN_CLASSTYPE,	CLASSTYPE_DOUBLE,	"double"},
	{GEN_CLASSTYPE,	CLASSTYPE_LONG,		"long"},
	{GEN_CLASSTYPE,	CLASSTYPE_SHORT,	"short"},
	{GEN_CLASSTYPE,	CLASSTYPE_UNSIGNED,	"unsigned"},
	{GEN_CLASSTYPE,	CLASSTYPE_SIGNED,	"signed"},
	{GEN_CLASSTYPE,	CLASSTYPE_VOID,		"void"},
	{GEN_CLASSTYPE,	CLASSTYPE_CONST,	"const"},
	{GEN_CLASSTYPE,	CLASSTYPE_VOLATILE,	"volatile"},
	{GEN_CLASSTYPE,	CLASSTYPE_AUTO,		"auto"},
	{GEN_CLASSTYPE,	CLASSTYPE_REGISTER,	"register"},
	{GEN_CLASSTYPE,	CLASSTYPE_STATIC,	"static"},
	{GEN_CLASSTYPE,	CLASSTYPE_EXTERN,	"extern"},
	{GEN_CLASSTYPE,	CLASSTYPE_TYPEDEF,	"typedef"},
	{GEN_CLASSTYPE,	CLASSTYPE_TNAME,	"@N"},
	{GEN_CLASSTYPE,	CLASSTYPE_STRUCT_D,	"@N"},
	{GEN_CLASSTYPE,	CLASSTYPE_ENUM_D,	"@N"},
	{GEN_CLASSTYPE,	CLASSTYPE_STRUCT_R,	"@N"},
	{GEN_CLASSTYPE,	CLASSTYPE_ENUM_R,	"@N"},
	{GEN_CLASSTYPE,	CLASSTYPE_INLINE,	"inline"},
	{GEN_PARAM_DCLS,	0,	"@L\n@@"},
	{GEN_PARAM_DCL,	0,	"@N @S;"},
	{GEN_PARAM_DEFS,	0,	"@L,@@"},
	{GEN_STARS,	0,	"@L@@"},
	{GEN_STAR,	STAR_NORMAL,	"*"},
	{GEN_STAR,	STAR_QUALS,	"*@N "},
	{GEN_STMT_LIST,	0,	"@L\n@@"},
	{GEN_ENUM_DCL,	sENUM_NOTAG,	"enum @P@A {@N}"},
	{GEN_ENUM_DCL,	sENUM_TAG,	"enum @N {@S}"},
	{GEN_ENUM_REF,	0,	"enum @N"},
	{GEN_MOE_LIST,	0,	"@L,@@"},
	{GEN_MOE,	MOE_VAL,	"@N = @S"},
	{GEN_MOE,	MOE_NOVAL,	"@N"},
	{GEN_STRUCT_DCL,	DCL_STRUCT_TAG,	"struct @N {@+\n@S@-\n}"},
	{GEN_STRUCT_DCL,	DCL_STRUCT_NOTAG,	"struct @P@A {@+\n@N@-\n}"},
	{GEN_STRUCT_DCL,	DCL_UNION_TAG,	"union @N {@+\n@S@-\n}"},
	{GEN_STRUCT_DCL,	DCL_UNION_NOTAG,	"union @P@A {@+\n@N@-\n}"},
#ifdef MVS
	{GEN_STRUCT_DCL,	DCL_PSTRUCT_TAG,	"_Packed struct @N {@+\n@S@-\n}"},
	{GEN_STRUCT_DCL,	DCL_PSTRUCT_NOTAG,	"_Packed struct @P@A {@+\n@N@-\n}"},
	{GEN_STRUCT_DCL,	DCL_PUNION_TAG,	"_Packed union @N {@+\n@S@-\n}"},
	{GEN_STRUCT_DCL,	DCL_PUNION_NOTAG,	"_Packed union @P@A {@+\n@N@-\n}"},
#endif /* MVS */
	{GEN_STRUCT_REF,	REF_STRUCT,	"struct @N"},
	{GEN_STRUCT_REF,	REF_UNION,	"union @N"},
#ifdef MVS
	{GEN_STRUCT_REF,	REF_PSTRUCT,	"_Packed struct @N"},
	{GEN_STRUCT_REF,	REF_PUNION,	"_Packed union @N"},
#endif /* MVS */
	{GEN_MEM_LIST,	0,	"@L\n@@"},
	{GEN_MEMBER,	0,	"@N @S;"},
	{GEN_MEM_DCLS,	0,	"@L,@@"},
	{GEN_MEM_DCL,	MEM_DCL_BIT,	"@N:@S"},
	{GEN_MEM_DCL,	MEM_DCL,	"@N"},
	{GEN_MEM_DCL,	MEM_BIT,	":@N"},
	{GEN_NAMES,	0,	"@L,@@"},
	{GEN_INIT_DCL,	INIT_DCL_SPEC,	"@N @S;"},
	{GEN_INIT_DCL,	INIT_DCL_NOSPEC,	"@N;"},
	{GEN_INIT_DCL,	INIT_DCL_EMPTY,	";"},
	{GEN_INDATA_DCLS,	0,	"@L\n@@"},
	{GEN_INDATA_DCL,	INIT_DCL_SPEC,	"@N @S;"},
	{GEN_INDATA_DCL,	INIT_DCL_NOSPEC,	"@N;"},
	{GEN_INDATA_DCL,	INIT_DCL_EMPTY,	";"},
	{GEN_DATA_SPECS,	0,	"@L,@@"},
	{GEN_DATA_SPEC,	DATA_SPEC_INIT,	"@N @S"},
	{GEN_DATA_SPEC,	DATA_SPEC,	"@N"},
	{GEN_DATA_ITEMS,	0,	"@L,@@"},
	{GEN_DATA_ITEM,	FUNC_STARS_SPEC,	"@N@S"},
	{GEN_DATA_ITEM,	FUNC_SPEC_NFCALL,	"@N()"},
	{GEN_DATA_ITEM,	FUNC_SPEC_ARRAY_EXPR,	"@N[@S]"},
	{GEN_DATA_ITEM,	FUNC_SPEC_ARRAY,	"@N[]"},
	{GEN_DATA_ITEM,	FUNC_SPEC_INHERIT,	"(@N)"},
	{GEN_DATA_ITEM,	FUNC_FCALL_NAMES,	"@N(@S)"},
	{GEN_DATA_ITEM,	FUNC_FCALL,	"@N()"},
	{GEN_DATA_ITEM,	FUNC_FCALL_ANSI,	"@N(@S)"},
	{GEN_DATA_ITEM,	FUNC_FCALL_ANSI_E,	"@N(@S, ...)"},
	{GEN_DATA_ITEM,	FUNC_SPEC_ANSI,	"@N(@S)"},
	{GEN_DATA_ITEM,	FUNC_SPEC_ANSI_E,	"@N(@S, ...)"},
	{GEN_DATA_ITEM,	FUNC_FCALL_E_ANSI,	"@N(...)"},
	{GEN_DATA_ITEM,	FUNC_SPEC_E_ANSI,	"@N(...)"},
	{GEN_DATA_ITEM,	DATA_NAME,	"@N"},
	{GEN_INIT_LIST,	0,	"@L,@@"},
	{GEN_INIT_ITEM,	INIT_EXPR,	"@N"},
	{GEN_INIT_ITEM,	INIT_LIST,	"@+\n{@N}@-"},
	{GEN_INITIALIZER,	INITIALIZER_EXPR,	"= @N"},
	{GEN_INITIALIZER,	INITIALIZER_LIST,	"= {@N}"},
	{GEN_COMPSTMT,	COMPSTMT_DCL_STMTS,	"{@+\n@N\n\n@S@-\n}"},
	{GEN_COMPSTMT,	COMPSTMT_STMTS,	"{@+\n@N@-\n}"},
	{GEN_COMPSTMT,	COMPSTMT_DCL,	"{@+\n@N@-\n}"},
	{GEN_COMPSTMT,	COMPSTMT_EMPTY,	"{\n}"},
	{GEN_STMT,	STMT_EXPR,	"@N;"},
	{GEN_STMT,	STMT_EMPTY,	";"},
	{GEN_STMT,	STMT_COMPSTMT,	"@N"},
	{GEN_STMT,	STMT_IF_ELSE,	"if(@N) @S\nelse @S"},
	{GEN_STMT,	STMT_IF,	"if(@N) @S"},
	{GEN_STMT,	STMT_WHILE,	"while(@N) @S"},
	{GEN_STMT,	STMT_DO,	"do @N while(@S);"},
	{GEN_STMT,	STMT_FOR_EEES,	"for(@N;@S;@S) @S"},
	{GEN_STMT,	STMT_FOR_EEE_,	"for(@N;@S;@S) ;"},
	{GEN_STMT,	STMT_FOR_EE_S,	"for (@N;@S;) @S"},
	{GEN_STMT,	STMT_FOR_EE__,	"for (@N;@S;);"},
	{GEN_STMT,	STMT_FOR_E_ES,	"for (@N;;@S) @S"},
	{GEN_STMT,	STMT_FOR_E_E_,	"for (@N;;@S);"},
	{GEN_STMT,	STMT_FOR_E__S,	"for (@N;;) @S"},
	{GEN_STMT,	STMT_FOR_E___,	"for (@N;;);"},
	{GEN_STMT,	STMT_FOR__EES,	"for (;@N;@S) @S"},
	{GEN_STMT,	STMT_FOR__EE_,	"for (;@N;@S);"},
	{GEN_STMT,	STMT_FOR__E_S,	"for (;@N;) @S"},
	{GEN_STMT,	STMT_FOR__E__,	"for (;@N;);"},
	{GEN_STMT,	STMT_FOR___ES,	"for (;;@N) @S"},
	{GEN_STMT,	STMT_FOR___E_,	"for (;;@N);"},
	{GEN_STMT,	STMT_FOR____S,	"for (;;) @N"},
	{GEN_STMT,	STMT_DUMMYSW,	"switch(@N) {@+\n@S\n@S@-\n}"},
	{GEN_STMT,	STMT_SWITCH,	"switch(@N)\n@S"},
	{GEN_STMT,	STMT_BREAK,	"break;"},
	{GEN_STMT,	STMT_CONTINUE,	"continue;"},
	{GEN_STMT,	STMT_RETURN_EXPR,	"return @N;"},
	{GEN_STMT,	STMT_RETURN,	"return;"},
	{GEN_STMT,	STMT_GOTO,	"goto @N;"},
	{GEN_STMT,	STMT_LABEL,	"@-@N:@+\n@S"},
	{GEN_STMT,	STMT_CASE,	"@-case @N:@+\n@S"},
	{GEN_STMT,	STMT_DEFAULT,	"@-default:@+\n@N"},
	{GEN_EXP_LIST,	0,	"@L,@@"},
	{GEN_EXPR,	EXPR_QCOLON,	"@N ? @S:@S"},
	{GEN_EXPR,	EXPR_COMMA,	"@N,@S"},
	{GEN_EXPR,	EXPR_BINOP,	"@N @S @S"},
	{GEN_EXPR,	EXPR_UNOP,	"@N@S"},
	{GEN_EXPR,	EXPR_INCOP,	"@N@S"},
	{GEN_EXPR,	EXPR_LSTAR,	"@N@S"},
	{GEN_EXPR,	EXPR_LARRAY,	"@N[@S]"},
	{GEN_EXPR,	EXPR_LARROW,	"@N->@S"},
	{GEN_EXPR,	EXPR_LDOT,	"@N.@S"},
	{GEN_EXPR,	EXPR_LFCALL1,	"@N(@S)"},
	{GEN_EXPR,	EXPR_LFCALL0,	"@N()"},
	{GEN_EXPR,	EXPR_CAST,	"(@N)@S"},
	{GEN_EXPR,	EXPR_SIZEOF,	"sizeof @N"},
	{GEN_EXPR,	EXPR_SIZEOF_TYPE,	"sizeof(@N)"},
	{GEN_EXPR,	EXPR_LNAME,	"@N"},
	{GEN_EXPR,	EXPR_ICON,	"@N"},
	{GEN_EXPR,	EXPR_FCON,	"@N"},
	{GEN_EXPR,	EXPR_STRING,	"@N"},
	{GEN_EXPR,	EXPR_INHERIT,	"(@N)"},
#ifdef MVS
	{GEN_EXPR,	EXPR_OFFSET,	"__offsetof(@N,@N)"},
#endif
	{GEN_FUNC_LP,	FUNC_NAME_LP,	"@N"},
	{GEN_FUNC_LP,	FUNC_EXPR_LP,	"@N"},
	{GEN_CAST_TYPE,	CAST_TYPE,	"@N @S"},
	{GEN_CAST_TYPE,	CAST_TYPE_NULL,	"@N"},
	{GEN_NULL_DCL,	NULL_N_FUNC,	"@N()"},
	{GEN_NULL_DCL,	NULL_INHERIT,	"()"},
	{GEN_NULL_DCL,	NULL_STAR_N,	"@N@S"},
	{GEN_NULL_DCL,	NULL_STAR,	"@N"},
	{GEN_NULL_DCL,	NULL_N,	"@N"},
	{GEN_NULL_DCL,	NULL_N_SUB_E,	"@N[@S]"},
	{GEN_NULL_DCL,	NULL_N_SUB,	"@N[]"},
	{GEN_NULL_DCL,	NULL_SUB_E,	"[@N]"},
	{GEN_NULL_DCL,	NULL_SUB,	"[]"},
	{GEN_NULL_DCL,	NULL_INHERIT_N,	"(@N)"},
	{GEN_NULL_DCL,	NULL_ANSI,	"@N(@S)"},
	{GEN_NULL_DCL,	NULL_ANSI_E,	"@N(@S, ...)"},
	{GEN_NULL_DCL,	NULL_E_ANSI,	"@N(...)"},
	{GEN_BINOP,	BINOP_PLUS,	"+"},
	{GEN_BINOP,	BINOP_MINUS,	"-"},
	{GEN_BINOP,	BINOP_MUL,	"*"},
	{GEN_BINOP,	BINOP_DIV,	"/"},
	{GEN_BINOP,	BINOP_MOD,	"%"},
	{GEN_BINOP,	BINOP_LS,	"<<"},
	{GEN_BINOP,	BINOP_RS,	">>"},
	{GEN_BINOP,	BINOP_AND,	"&"},
	{GEN_BINOP,	BINOP_OR,	"|"},
	{GEN_BINOP,	BINOP_ER,	"^"},
	{GEN_BINOP,	BINOP_ASGN,	"="},
	{GEN_BINOP,	BINOP_APLUS,	"+="},
	{GEN_BINOP,	BINOP_AMINUS,	"-="},
	{GEN_BINOP,	BINOP_AMUL,	"*="},
	{GEN_BINOP,	BINOP_ADIV,	"/="},
	{GEN_BINOP,	BINOP_AMOD,	"%="},
	{GEN_BINOP,	BINOP_ALS,	"<<="},
	{GEN_BINOP,	BINOP_ARS,	">>="},
	{GEN_BINOP,	BINOP_AAND,	"&="},
	{GEN_BINOP,	BINOP_AOR,	"|="},
	{GEN_BINOP,	BINOP_AER,	"^="},
	{GEN_BINOP,	BINOP_ANDAND,	"&&"},
	{GEN_BINOP,	BINOP_OROR,	"||"},
	{GEN_BINOP,	BINOP_EQ,	"=="},
	{GEN_BINOP,	BINOP_GTEQ,	">="},
	{GEN_BINOP,	BINOP_LTEQ,	"<="},
	{GEN_BINOP,	BINOP_NEQ,	"!="},
	{GEN_BINOP,	BINOP_LT,	"<"},
	{GEN_BINOP,	BINOP_GT,	">"},
	{GEN_INCOP,	INCOP_INC,	"++"},
	{GEN_INCOP,	INCOP_DEC,	"--"},
	{GEN_UNOP,	UNOP_AND,	"&"},
	{GEN_UNOP,	UNOP_MINUS,	"-"},
	{GEN_UNOP,	UNOP_INC,	"++"},
	{GEN_UNOP,	UNOP_DEC,	"--"},
	{GEN_UNOP,	UNOP_NOT,	"!"},
	{GEN_UNOP,	UNOP_COMPL,	"~"},
	{GEN_ANSI_PARAMS,	0,	"@L, @@"},
	{GEN_ANSI_PARAM,	ANSI_DATA_ITEM, "@N @S"},
	{GEN_ANSI_PARAM,	ANSI_NULL_DCL, "@N @S"},
	{GEN_QUALS,	0,	"@L @@"},
	{GEN_QUAL,	QUAL_CONST,	"const"},
	{GEN_QUAL,	QUAL_VOLATILE,	"volatile"},
	{GEN_STRINGS,	0,	"@L@+\n@@@-"},
	{GENUS_COUNT,	0,	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/deparse.h,v 3.7 2013/12/08 23:46:10 tom Exp $"},

#endif /* deparse_H */
