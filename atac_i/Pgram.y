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
%{
#ifdef MVS
 #pragma csect (CODE, "pgram$")
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#include <string.h>
#endif /* MVS */

static char Pgram_y[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/Pgram.y,v 3.11 1997/11/03 19:06:56 tom Exp $";
/*
* $Log: Pgram.y,v $
* Revision 3.11  1997/11/03 19:06:56  tom
* move <stdio.h> down below the include for config.h so we have const
* consistently
*
* Revision 3.10  1997/05/10 22:17:14  tom
* absorb srcpos.h into error.h
*
* Revision 3.9  1996/11/13 01:26:26  tom
* undo redefinition of literal-tokens (e.g., ';' vs TOK_SEMICOLON).
* change def of AUTO to avoid conflict with bison's ENDFILE.
* add forward-ref prototypes.
*
* Revision 3.8  1996/01/11 17:47:36  tom
* add _something_ to handle ASM case
*
* Revision 3.7  1995/12/27 01:35:34  tom
* handle INLINE, ASM states.  Also declare ATTRIBUTE state.
*
* Revision 3.6  94/06/01  09:02:58  saul
* fix for ANSI f(...) 
* 
* Revision 3.5  94/04/04  10:11:33  jrh
* Add Release Copyright
* 
* Revision 3.4  94/03/21  08:17:21  saul
* MVS support __offsetof as builtin (not handled by cpp)
* 
* Revision 3.3  93/11/19  12:15:20  saul
* MVS support for _Packed
* 
* Revision 3.2  93/08/04  15:43:17  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  08:56:07  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:44:44  saul
* propagate to version 3.0
* 
* Revision 2.10  92/11/04  15:55:49  saul
* removed access to freed memory in STMT_FOR.  Fixed struct {;};
* 
* Revision 2.9  92/11/02  15:45:21  saul
* changed CHAR to CHAR_KW to make room for CHAR() in portable.h
* 
* Revision 2.8  92/10/30  09:47:24  saul
* include portable.h
* 
* Revision 2.7  92/09/16  07:35:06  saul
* New scan interface.  Get rid of unused keywords.
* 
* Revision 2.6  92/04/07  07:36:57  saul
* added unique prefix stuff
* 
* Revision 2.5  92/03/17  14:22:06  saul
* copyright
* 
* Revision 2.4  91/10/23  13:20:25  saul
* Handle "*const volatile".
* 
* Revision 2.3  91/10/23  12:34:13  saul
* Allow empty declaration list as in "int;"
* 
* Revision 2.2  91/06/13  13:12:15  saul
* Changes for ansi.
* 
* Revision 2.1  91/06/13  12:38:51  saul
* Propagate to version 2.0
* 
* Revision 1.3  91/06/12  21:01:23  saul
* remove percent } from log
* 
* Revision 1.2  91/06/12  20:57:02  saul
* Move rcs id inside %{ %\}
* 
* Revision 1.1  91/06/12  20:25:32  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include "portable.h"
#include <stdio.h>
#include "error.h"
#include "scan.h"
#include "tnode.h"
#include "tree.h"

/* forward declarations */
extern int yyparse P_(( void ));
static void insertTypeNames P_(( TNODE *node ));
extern int parse P_(( FILE *srcfile, TNODE **tree, char **uprefix ));

static TNODE *tree_root;

static SRCPOS nosrcpos[2] = {{-1,0,0}, {-1,0,0}};
%}

%union {
	TOKENVALUE	token;
	TNODE		*tnode;
}

/*
* Keywords.
*/
%term <token> BREAK		259
%term <token> CASE		260
%term <token> CHAR_KW		261
%term <token> CONST		262
%term <token> CONTINUE		263
%term <token> DEFAULT		264
%term <token> DO		265
%term <token> DOUBLE		266
%term <token> ELSE		267
%term <token> ENUM		268
%term <token> EXTERN		269
%term <token> FLOAT		270
%term <token> FOR		271
%term <token> GOTO		272
%term <token> IF		273
%term <token> INT		275
%term <token> LONG		276
%term <token> REGISTER		277
%term <token> RETURN		278
%term <token> SHORT		279
%term <token> SIGNED		280
%term <token> SIZEOF		281
%term <token> STATIC		282
%term <token> STRUCT		283
%term <token> SWITCH		284
%term <token> TYPEDEF		285
%term <token> UNION		287
%term <token> UNSIGNED		288
%term <token> VOID		289
%term <token> VOLATILE		290
%term <token> WHILE		291
%term <token> TOK_PACKED	292		/* for MVS */
%term <token> OFFSET		293

%term <token> ASM               294
%term <token> INLINE            295
%term <token> ATTRIBUTE         296
%term <token> AUTO		297

%term <token> TOK_LPAREN	501
%term <token> TOK_RPAREN	502
%term <token> TOK_LSQUARE	503
%term <token> TOK_RSQUARE	504
%term <token> TOK_LCURLY	505
%term <token> TOK_RCURLY	506
%term <token> TOK_COMMA		507
%term <token> TOK_EQUALS	508
%term <token> TOK_QMARK		509
%term <token> TOK_COLON		510
%term <token> TOK_VERTICAL	511
%term <token> TOK_CARROT	512
%term <token> TOK_AMPER		513
%term <token> TOK_GREATER	514
%term <token> TOK_LESSER	515
%term <token> TOK_PLUS		516
%term <token> TOK_DASH		517
%term <token> TOK_STAR		518
%term <token> TOK_PERCENT	519
%term <token> TOK_SLASH		520
%term <token> TOK_EXCLAIM	521
%term <token> TOK_TILDE		522
%term <token> TOK_PERIOD	523
%term <token> TOK_SEMICOLON	524

/*
* Multi-char. tokens and token groups
*/

%term <token> ELLIPSIS		300
%term <token> ANDAND		303
%term <token> OROR		304
%term <token> PLUSPLUS		309
%term <token> MINUSMINUS	310
%term <token> STREF		311
%term <token> PLUS_EQ		312
%term <token> MINUS_EQ		313
%term <token> MUL_EQ		314
%term <token> DIV_EQ		315
%term <token> MOD_EQ		316
%term <token> LS_EQ		317
%term <token> RS_EQ		318
%term <token> AND_EQ		319
%term <token> OR_EQ		320
%term <token> ER_EQ		321
%term <token> EQUAL		322
%term <token> BANG_EQUAL	323
%term <token> GT_EQ		324
%term <token> LT_EQ		325
%term <token> LSHIFT		326
%term <token> RSHIFT		327

/*
* Other tokens
*/
%term <token> NAME		400
%term <token> T_NAME	401
%term <token> STRING		402
%term <token> ICON		403
%term <token> FCON		404
%term ENDFILE			0

%left CHAR_KW DOUBLE ENUM FLOAT INT LONG SHORT STRUCT UNION UNSIGNED VOID TOK_SEMICOLON
%right T_NAME
%right ELSE
%left TOK_COMMA
%right PLUS_EQ MINUS_EQ MUL_EQ DIV_EQ MOD_EQ LS_EQ RS_EQ AND_EQ OR_EQ ER_EQ TOK_EQUALS
%right TOK_QMARK TOK_COLON
%left OROR
%left ANDAND
%left TOK_VERTICAL
%left TOK_CARROT
%left TOK_AMPER
%left EQUAL BANG_EQUAL
%left GT_EQ LT_EQ TOK_GREATER TOK_LESSER
%left LSHIFT RSHIFT
%left TOK_PLUS TOK_DASH
%left TOK_STAR TOK_PERCENT TOK_SLASH
%right TOK_EXCLAIM TOK_TILDE PLUSPLUS MINUSMINUS SIZEOF OFFSET
%left TOK_LSQUARE TOK_LPAREN TOK_PERIOD STREF

%type <tnode> ansi_param
%type <tnode> ansi_params
%type <tnode> cast_type
%type <tnode> classtype
%type <tnode> classtypes
%type <tnode> complex_term
%type <tnode> compstmt
%type <tnode> d_i_term
%type <tnode> data_item
%type <tnode> data_spec
%type <tnode> data_specs
%type <tnode> enum_dcl
%type <tnode> enum_ref
%type <tnode> exp_list
%type <tnode> expr
%type <tnode> func_spec
%type <tnode> function
%type <tnode> indata_dcl
%type <tnode> indata_dcls
%type <tnode> init_dcl
%type <tnode> init_item
%type <tnode> init_list
%type <tnode> initializer
%type <tnode> mem_dcl
%type <tnode> mem_dcls
%type <tnode> mem_list
%type <tnode> member
%type <tnode> module
%type <tnode> module_item
%type <tnode> moe
%type <tnode> moe_list
%type <tnode> name
%type <tnode> names
%type <tnode> nfunc_spec
%type <tnode> null_dcl
%type <tnode> param_dcl
%type <tnode> param_dcls
%type <tnode> parameter_defs
%type <tnode> qualifier
%type <tnode> qualifiers
%type <tnode> stmt
%type <tnode> stmt_list
%type <tnode> strings
%type <tnode> struct_dcl
%type <tnode> struct_ref
%type <tnode> t_name
%type <tnode> term
%type <tnode> tname_or_name
%type <tnode> mname

%type <tnode> star
%type <tnode> asop
%type <tnode> relop
%type <tnode> equop

%start module
%%
module:
			module module_item
			{
				$$ = $1;
				if ($2) tlist_add($$, $2);
			}
		|	/* empty */
			{
				$$ = tmknode(GEN_MODULE, 0, 0, 0);
				tree_root = $$;
			}
		;
module_item:
			function
			{
				$$ = tmknode(GEN_MODULE_ITEM, FUNCTION_ITEM,
					$1, 0);
			}
		|	init_dcl
			{
				if ($1) $$ = tmknode(GEN_MODULE_ITEM, DCL_ITEM,
					$1, 0);
				else $$ = NULL;
			}
		;
function:
			classtypes func_spec param_dcls compstmt
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					parse_error($1->srcpos,
						"typedef function definition");
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_FUNCTION, FUNC_TFPC, $1, $2);
				tlist_add($$, $3);
				tlist_add($$, $4);
			}
		|	classtypes func_spec compstmt
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					parse_error($1->srcpos,
						"typedef function definition");
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_FUNCTION, FUNC_TFC, $1, $2);
				tlist_add($$, $3);
			}
		|	func_spec param_dcls compstmt
			{
				$$ = tmknode(GEN_FUNCTION, FUNC_FPC, $1, $2);
				tlist_add($$, $3);
			}
		|	func_spec compstmt
			{
				$$ = tmknode(GEN_FUNCTION, FUNC_FC, $1, $2);
			}
		;
func_spec:	
			star func_spec
			{
				if ($2->species == FUNC_STARS_SPEC) {
					tlist_ladd($2->down->over, $1);
					$$ = $2;
				} else {
					$$ = tmknode(GEN_FUNC_SPEC,
						FUNC_STARS_SPEC,
						tmknode(GEN_STARS, 0, $1, 0),
						$2);
				}
			}
		|	complex_term
			{
				$$ = $1;
			}
		;
complex_term:		
			complex_term TOK_LPAREN TOK_RPAREN
			{
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_SPEC_NFCALL,
					$1, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	complex_term TOK_LSQUARE expr TOK_RSQUARE
			{
				$$ = tmknode(GEN_FUNC_SPEC,
					FUNC_SPEC_ARRAY_EXPR, $1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	complex_term TOK_LSQUARE TOK_RSQUARE
			{
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_SPEC_ARRAY,
					$1, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	TOK_LPAREN func_spec TOK_RPAREN
			{
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_SPEC_INHERIT,
					$2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		|	name TOK_LPAREN names TOK_RPAREN 
			{
				$1->genus = GEN_FNAME;	/* fix leaf type */
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_FCALL_NAMES,
					$1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	name TOK_LPAREN TOK_RPAREN 
			{
				$1->genus = GEN_FNAME;	/* fix leaf type */
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_FCALL, $1, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	name TOK_LPAREN ansi_params TOK_RPAREN 
			{
				$1->genus = GEN_FNAME;	/* fix leaf type */
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_FCALL_ANSI,
					$1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	name TOK_LPAREN ansi_params TOK_COMMA ELLIPSIS TOK_RPAREN 
			{
				$1->genus = GEN_FNAME;	/* fix leaf type */
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_FCALL_ANSI_E,
					$1, $3);
				tsrc_pos($$, NULL, $6.srcpos);
			}
		|	complex_term TOK_LPAREN ansi_params TOK_RPAREN
			{
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_SPEC_ANSI,
					$1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	complex_term TOK_LPAREN ansi_params TOK_COMMA ELLIPSIS TOK_RPAREN
			{
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_SPEC_ANSI_E,
					$1, $3);
				tsrc_pos($$, NULL, $6.srcpos);
			}
		|	name TOK_LPAREN ELLIPSIS TOK_RPAREN 
			{
				$1->genus = GEN_FNAME;	/* fix leaf type */
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_FCALL_E_ANSI,
					$1, 0);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	complex_term TOK_LPAREN ELLIPSIS TOK_RPAREN
			{
				$$ = tmknode(GEN_FUNC_SPEC, FUNC_SPEC_E_ANSI,
					$1, 0);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		;
ansi_params:
			ansi_param
			{
				$$ = tmknode(GEN_ANSI_PARAMS, 0, $1, 0);
			}
		|	ansi_params TOK_COMMA ansi_param
			{
				$$ = tlist_add($1, $3);
			}
		;
ansi_param:
			classtypes data_item
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					parse_error($1->srcpos,
						"typedef parameters");
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_ANSI_PARAM, ANSI_DATA_ITEM,
					$1, $2);
			}
		|	classtypes null_dcl
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					parse_error($1->srcpos,
						"typedef parameters");
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_ANSI_PARAM, ANSI_NULL_DCL,
					$1, $2);
			}
		;
classtypes:
			classtype
			{
				$$ = tmknode(GEN_CLASSTYPES, 0, $1, 0);
				if ($1->species == CLASSTYPE_TYPEDEF)
					$$->species = CLASSTYPES_TYPEDEF;
			}
		|	classtypes classtype
			{
				$$ = tlist_add($1, $2);
				if ($2->species == CLASSTYPE_TYPEDEF)
					$$->species = CLASSTYPES_TYPEDEF;
			}
		;
classtype:
			INT
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_INT,
					$1.srcpos, 0);
			}
		|	CHAR_KW
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_CHAR,
					$1.srcpos, 0);
			}
		|	FLOAT
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_FLOAT,
					$1.srcpos, 0);
			}
		|	DOUBLE
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_DOUBLE,
					$1.srcpos, 0);
			}
		|	LONG
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_LONG,
					$1.srcpos, 0);
			}
		|	SHORT
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_SHORT,
					$1.srcpos, 0);
			}
		|	UNSIGNED
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_UNSIGNED,
					$1.srcpos, 0);
			}
		|	SIGNED
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_SIGNED,
					$1.srcpos, 0);
			}
		|	VOID
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_VOID,
					$1.srcpos, 0);
			}
		|	CONST
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_CONST,
					$1.srcpos, 0);
			}
		|	VOLATILE
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_VOLATILE,
					$1.srcpos, 0);
			}
		|	AUTO
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_AUTO,
					$1.srcpos, 0);
			}
		|	REGISTER
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_REGISTER,
					$1.srcpos, 0);
			}
		|	STATIC
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_STATIC,
					$1.srcpos, 0);
			}
		|	INLINE
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_INLINE,
					$1.srcpos, 0);
			}
		|	EXTERN
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_EXTERN,
					$1.srcpos, 0);
			}
		|	TYPEDEF
			{
				$$ = tmkleaf(GEN_CLASSTYPE, CLASSTYPE_TYPEDEF,
					$1.srcpos, 0);
			}
		|	t_name
			{
				$$ = tmknode(GEN_CLASSTYPE, CLASSTYPE_TNAME,
					$1, 0);
			}
		|	struct_dcl
			{
				$$ = tmknode(GEN_CLASSTYPE, CLASSTYPE_STRUCT_D,
					$1, 0);
			}
		|	enum_dcl
			{
				$$ = tmknode(GEN_CLASSTYPE, CLASSTYPE_ENUM_D,
					$1, 0);
			}
		|	struct_ref
			{
				$$ = tmknode(GEN_CLASSTYPE, CLASSTYPE_STRUCT_R,
					$1, 0);
			}
		|	enum_ref
			{
				$$ = tmknode(GEN_CLASSTYPE, CLASSTYPE_ENUM_R,
					$1, 0);
			}
		;
param_dcls:
			param_dcl
			{
				$$ = tmknode(GEN_PARAM_DCLS, 0, $1, 0);
			}
		|	param_dcls param_dcl
			{
				$$ = tlist_add($1, $2);
			}
		|	param_dcls TOK_SEMICOLON
			{
				/* extra ';' not allowed before 1st param_dcl */
				$$ = $1;
				tsrc_pos($$, NULL, $2.srcpos);
			}
		;
param_dcl:
			classtypes parameter_defs TOK_SEMICOLON
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					parse_error($1->srcpos,
						"typedef parameters");
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_PARAM_DCL, 0, $1, $2);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		;
parameter_defs:
			data_item
			{
				$$ = tmknode(GEN_PARAM_DEFS, 0, $1, 0);
			}
		|	parameter_defs TOK_COMMA data_item 
			{
				$$ = tlist_add($1, $3);
			}
		;
stmt_list:
			stmt_list stmt
			{
				$$ = tlist_add($1, $2);
			}
		|	stmt
			{
				$$ = tmknode(GEN_STMT_LIST, 0, $1, 0);
			}
		;
enum_dcl:
			ENUM TOK_LCURLY moe_list TOK_RCURLY
			{
				$$ = tmknode(GEN_ENUM_DCL, sENUM_NOTAG, $3, 0);
				tsrc_pos($$, $1.srcpos, $4.srcpos);
			}
		|	ENUM tname_or_name TOK_LCURLY moe_list TOK_RCURLY
			{
				$$ = tmknode(GEN_ENUM_DCL, sENUM_TAG, $2, $4);
				tsrc_pos($$, $1.srcpos, $5.srcpos);
			}
		|	ENUM TOK_LCURLY moe_list TOK_COMMA TOK_RCURLY
			{
				$$ = tmknode(GEN_ENUM_DCL, sENUM_NOTAG, $3, 0);
				tsrc_pos($$, $1.srcpos, $5.srcpos);
			}
		|	ENUM tname_or_name TOK_LCURLY moe_list TOK_COMMA TOK_RCURLY
			{
				$$ = tmknode(GEN_ENUM_DCL, sENUM_TAG, $2, $4);
				tsrc_pos($$, $1.srcpos, $6.srcpos);
			}
		;
enum_ref:
			ENUM tname_or_name 
			{
				$$ = tmknode(GEN_ENUM_REF, 0, $2, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		;
moe_list:
			moe
			{
				$$ = tmknode(GEN_MOE_LIST, 0, $1, 0);
			}
		|	moe_list TOK_COMMA moe
			{
				$$ = tlist_add($1, $3);
			}
		;
moe:
			name
			{
				$$ = tmknode(GEN_MOE, MOE_NOVAL, $1, 0);
			}
		|	name TOK_EQUALS expr
			{
				$$ = tmknode(GEN_MOE, MOE_VAL, $1, $3);
			}
		;
struct_dcl:
			STRUCT tname_or_name TOK_LCURLY mem_list TOK_RCURLY
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_STRUCT_TAG,
					$2, $4);
				tsrc_pos($$, $1.srcpos, $5.srcpos);
			}
		|	UNION tname_or_name TOK_LCURLY mem_list TOK_RCURLY
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_UNION_TAG,
					$2, $4);
				tsrc_pos($$, $1.srcpos, $5.srcpos);
			}
		|	STRUCT TOK_LCURLY mem_list TOK_RCURLY
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_STRUCT_NOTAG,
					$3, 0);
				tsrc_pos($$, $1.srcpos, $4.srcpos);
			}
		|	UNION TOK_LCURLY mem_list TOK_RCURLY
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_UNION_NOTAG,
					$3, 0);
				tsrc_pos($$, $1.srcpos, $4.srcpos);
			}
		|	TOK_PACKED STRUCT tname_or_name TOK_LCURLY mem_list TOK_RCURLY /*MVS*/
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_PSTRUCT_TAG,
					$3, $5);
				tsrc_pos($$, $1.srcpos, $6.srcpos);
			}
		|	TOK_PACKED UNION tname_or_name TOK_LCURLY mem_list TOK_RCURLY /*MVS*/
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_PUNION_TAG,
					$3, $5);
				tsrc_pos($$, $1.srcpos, $6.srcpos);
			}
		|	TOK_PACKED STRUCT TOK_LCURLY mem_list TOK_RCURLY	/*MVS*/
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_PSTRUCT_NOTAG,
					$4, 0);
				tsrc_pos($$, $1.srcpos, $5.srcpos);
			}
		|	TOK_PACKED UNION TOK_LCURLY mem_list TOK_RCURLY	/*MVS*/
			{
				$$ = tmknode(GEN_STRUCT_DCL, DCL_PUNION_NOTAG,
					$4, 0);
				tsrc_pos($$, $1.srcpos, $5.srcpos);
			}
		;
struct_ref:
			STRUCT tname_or_name 
			{
				$$ = tmknode(GEN_STRUCT_REF, REF_STRUCT,
					$2, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	UNION tname_or_name 
			{
				$$ = tmknode(GEN_STRUCT_REF, REF_UNION, $2, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|
			TOK_PACKED STRUCT tname_or_name 	/*MVS*/
			{
				$$ = tmknode(GEN_STRUCT_REF, REF_PSTRUCT,
					$3, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	TOK_PACKED UNION tname_or_name 		/*MVS*/
			{
				$$ = tmknode(GEN_STRUCT_REF, REF_PUNION, $3, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		;
mem_list:
			member
			{
				$$ = tmknode(GEN_MEM_LIST, 0, $1, 0);
			}
		|	mem_list member 
			{	
			        if ($2 != NULL) {
				    $$ = tlist_add($1, $2);
				} else {
				    $$ = $1;
				}
			}
		;
member:
			classtypes mem_dcls TOK_SEMICOLON
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					parse_error($1->srcpos,
						"typedef struct/union member");
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_MEMBER, 0, $1, $2);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	TOK_SEMICOLON
			{
			        $$ = NULL; 
			}
/*
		|	mem_dcls TOK_SEMICOLON
			{
				/ *
				* This rule is not in the internal grammar
				* so we have to fake it.
				* /
				$$ = tmknode(GEN_MEMBER, 0,
				  tmknode(GEN_CLASSTYPES, CLASSTYPES_NORMAL,
				    tmkleaf(GEN_CLASSTYPE, CLASSTYPE_INT,
					nosrcpos, 0),
				  0),
				$1);
				tsrc_pos($$, NULL, $2.srcpos);
			}
*/
		;
mem_dcls:
			mem_dcl
			{
				$$ = tmknode(GEN_MEM_DCLS, 0, $1, 0);
			}
		|	mem_dcls TOK_COMMA mem_dcl
			{
				$$ = tlist_add($1, $3);
			}
		;
mem_dcl:
			data_item
			{
				$$ = tmknode(GEN_MEM_DCL, MEM_DCL, $1, 0);
			}
		|	data_item TOK_COLON expr
			{
				$$ = tmknode(GEN_MEM_DCL, MEM_DCL_BIT, $1, $3);
			}
		|	TOK_COLON expr
			{
				$$ = tmknode(GEN_MEM_DCL, MEM_BIT, $2, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		;
names:
			name
			{
				$$ = tmknode(GEN_NAMES, 0, $1, 0);
			}
		|	names  TOK_COMMA  name 
			{
				$$ = tlist_add($1, $3);
			}
		;
init_dcl:		
			classtypes data_specs TOK_SEMICOLON
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					insertTypeNames($2);
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_INIT_DCL, INIT_DCL_SPEC,
					$1, $2);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	classtypes TOK_SEMICOLON
			{
				if ($1->species == CLASSTYPES_TYPEDEF)
					$1->species = CLASSTYPES_NORMAL;
				$$ = tmknode(GEN_INIT_DCL, INIT_DCL_NOSPEC,
					$1, 0);
				tsrc_pos($$, NULL, $2.srcpos);
			}
		|	data_specs TOK_SEMICOLON
			{
				/*
				* This rule is not in the internal grammar
				* so we have to fake it.
				*/
				$$ = tmknode(GEN_INIT_DCL, INIT_DCL_SPEC,
				  tmknode(GEN_CLASSTYPES, CLASSTYPES_NORMAL,
				    tmkleaf(GEN_CLASSTYPE, CLASSTYPE_INT,
					nosrcpos, 0),
				  0),
				$1);
				tsrc_pos($$, NULL, $2.srcpos);
			}
		|	TOK_SEMICOLON
			{
				/* Never generate corresponding species. */
				$$ = NULL;
			}
		;
indata_dcls:		
			indata_dcls indata_dcl
			{
				$$ = tlist_add($1, $2);
			}
		|	indata_dcl
			{
				$$ = tmknode(GEN_INDATA_DCLS, 0, $1, 0);
			}
		;
indata_dcl:
			classtypes data_specs TOK_SEMICOLON
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					insertTypeNames($2);
					$1->species = CLASSTYPES_NORMAL;
				}
				$$ = tmknode(GEN_INDATA_DCL, INIT_DCL_SPEC,
					$1, $2);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	classtypes TOK_SEMICOLON
			{
				if ($1->species == CLASSTYPES_TYPEDEF)
					$1->species = CLASSTYPES_NORMAL;
				$$ = tmknode(GEN_INDATA_DCL, INIT_DCL_NOSPEC,
					$1, 0);
				tsrc_pos($$, NULL, $2.srcpos);
			}
		|	TOK_SEMICOLON indata_dcl				%prec TOK_SEMICOLON
			{
				$$ = $2;
				tsrc_pos($$, $1.srcpos, NULL);
			}
		;
data_specs:	
			data_spec
			{
				$$ = tmknode(GEN_DATA_SPECS, 0, $1, 0);
			}
		|	data_specs TOK_COMMA data_spec
			{
				$$ = tlist_add($1, $3);
			}
		;
data_spec:
			data_item initializer
			{
				$$ = tmknode(GEN_DATA_SPEC, DATA_SPEC_INIT,
					$1, $2);
			}
		|	data_item 
			{
				$$ = tmknode(GEN_DATA_SPEC, DATA_SPEC, $1, 0);
			}
		;
data_item:	
			func_spec
			{
				/*
				* Change all FUNC_SPEC to DATA_ITEM in tree from
				* $1.  Assume at most one FUNC_SPEC child at
				* each node and FUNC_SPEC always a child of
				* FUNC_SPEC.
				*/
				TNODE *p;
				TNODE *first;

				first = $1;
				p = first;
				while (p) {
					if (p->genus == GEN_FUNC_SPEC) {
						p->genus = GEN_DATA_ITEM;
						first = p->down;
						p = first;
					} else {
						p = p->over;
						if (p == first) break;
					}
				}
				$$ = $1;
			}
		|	nfunc_spec
			{
				$$ = $1;
			}
		;
nfunc_spec:
			star nfunc_spec  		%prec TOK_STAR
			{
				if ($2->species == FUNC_STARS_SPEC) {
					tlist_ladd($2->down->over, $1);
					$$ = $2;
				} else {
					$$ = tmknode(GEN_DATA_ITEM,
						FUNC_STARS_SPEC,
						tmknode(GEN_STARS, 0, $1, 0),
						$2);
				}
			}
		|	d_i_term
			{
				$$ = $1;
			}
		|	name
			{
				$$ = tmknode(GEN_DATA_ITEM, DATA_NAME, $1, 0);
			}
		;
d_i_term:		
			d_i_term TOK_LPAREN TOK_RPAREN
			{
				$$ = tmknode(GEN_DATA_ITEM,
					FUNC_SPEC_NFCALL, $1, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	nfunc_spec TOK_LSQUARE expr TOK_RSQUARE
			{
				$$ = tmknode(GEN_DATA_ITEM,
					FUNC_SPEC_ARRAY_EXPR, $1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	nfunc_spec TOK_LSQUARE TOK_RSQUARE
			{
				$$ = tmknode(GEN_DATA_ITEM,
					FUNC_SPEC_ARRAY, $1, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	TOK_LPAREN nfunc_spec TOK_RPAREN
			{
				$$ = tmknode(GEN_DATA_ITEM,
					FUNC_SPEC_INHERIT, $2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		|	d_i_term TOK_LPAREN ansi_params TOK_RPAREN
			{
				$$ = tmknode(GEN_DATA_ITEM,
					FUNC_SPEC_ANSI, $1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	d_i_term TOK_LPAREN ansi_params TOK_COMMA ELLIPSIS TOK_RPAREN
			{
				$$ = tmknode(GEN_DATA_ITEM,
					FUNC_SPEC_ANSI_E, $1, $3);
				tsrc_pos($$, NULL, $6.srcpos);
			}
		|	d_i_term TOK_LPAREN ELLIPSIS TOK_RPAREN
			{
				$$ = tmknode(GEN_DATA_ITEM,
					FUNC_SPEC_E_ANSI, $1, 0);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		;
init_list:
			init_item
			{
				$$ = tmknode(GEN_INIT_LIST, 0, $1, 0);
			}
		|	init_list TOK_COMMA init_item
			{
				$$ = tlist_add($1, $3);
			}
		;
init_item:
			expr				%prec TOK_COMMA
			{
				$$ = tmknode(GEN_INIT_ITEM, INIT_EXPR,
					$1, 0);
			}
		|	TOK_LCURLY init_list TOK_RCURLY
			{
				$$ = tmknode(GEN_INIT_ITEM, INIT_LIST,
					$2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		|	TOK_LCURLY init_list TOK_COMMA TOK_RCURLY
			{
				$$ = tmknode(GEN_INIT_ITEM, INIT_LIST,
					$2, 0);
				tsrc_pos($$, $1.srcpos, $4.srcpos);
			}
		;
initializer:
			TOK_EQUALS expr
			{
				$$ = tmknode(GEN_INITIALIZER, INITIALIZER_EXPR,
					$2, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	TOK_EQUALS TOK_LCURLY init_list TOK_RCURLY
			{
				$$ = tmknode(GEN_INITIALIZER, INITIALIZER_LIST,
					$3, 0);
				tsrc_pos($$, $1.srcpos, $4.srcpos);
			}
		|	TOK_EQUALS TOK_LCURLY init_list TOK_COMMA TOK_RCURLY
			{
				/* Ignore extra comma! */
				$$ = tmknode(GEN_INITIALIZER, INITIALIZER_LIST,
					$3, 0);
				tsrc_pos($$, $1.srcpos, $5.srcpos);
			}
		;
compstmt:	   
			TOK_LCURLY indata_dcls stmt_list TOK_RCURLY
			{
				$$ = tmknode(GEN_COMPSTMT, COMPSTMT_DCL_STMTS,
					$2, $3);
				tsrc_pos($$, $1.srcpos, $4.srcpos);
			}
		|	TOK_LCURLY stmt_list TOK_RCURLY
			{
				$$ = tmknode(GEN_COMPSTMT, COMPSTMT_STMTS,
					$2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		|	TOK_LCURLY indata_dcls TOK_RCURLY
			{
				$$ = tmknode(GEN_COMPSTMT, COMPSTMT_DCL,
					$2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		|	TOK_LCURLY TOK_RCURLY
			{
				$$ = tmknode(GEN_COMPSTMT, COMPSTMT_EMPTY,
					0, 0);
				tsrc_pos($$, $1.srcpos, $2.srcpos);
			}
		;
stmt:
			ASM TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_EMPTY, 0, 0);
				tsrc_pos($$, $1.srcpos, $1.srcpos);
			}
		|
			expr TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_EXPR, $1, 0);
				tsrc_pos($$, NULL, $2.srcpos);
			}
		|	TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_EMPTY, 0, 0);
				tsrc_pos($$, $1.srcpos, $1.srcpos);
			}
		|	compstmt
			{
				$$ = tmknode(GEN_STMT, STMT_COMPSTMT, $1, 0);
			}
		|	IF TOK_LPAREN expr TOK_RPAREN stmt ELSE stmt
			{
				$$ = tmknode(GEN_STMT, STMT_IF_ELSE, $3, $5);
				tlist_add($$, $7);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	IF TOK_LPAREN expr TOK_RPAREN stmt			%prec ELSE
			/*
			* Right associative precedence needed to parse
			* "if(e)if(e)s;else s;" correctly.  Otherwise
			* "else" could be taken as part of first "if".
			* saul 8/22/89 
			*/
			{
				$$ = tmknode(GEN_STMT, STMT_IF, $3, $5);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	WHILE TOK_LPAREN expr TOK_RPAREN stmt
			{
				$$ = tmknode(GEN_STMT, STMT_WHILE, $3, $5);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	DO stmt WHILE  TOK_LPAREN  expr  TOK_RPAREN   TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_DO, $2, $5);
				tsrc_pos($$, $1.srcpos, $7.srcpos);
			}
		|	FOR TOK_LPAREN expr TOK_SEMICOLON expr TOK_SEMICOLON expr TOK_RPAREN stmt
			{
				if ($9->species == STMT_EMPTY) {
					$$ = tmknode(GEN_STMT, STMT_FOR_EEE_,
						$3, $5);
					tlist_add($$, $7);
					tsrc_pos($$, $1.srcpos, $9->srcpos);
					tfreenode($9);
				} else {
					$$ = tmknode(GEN_STMT, STMT_FOR_EEES,
						$3, $5);
					tlist_add($$, $7);
					tlist_add($$, $9);
					tsrc_pos($$, $1.srcpos, $9->srcpos);
				}
			}
		|	FOR TOK_LPAREN expr TOK_SEMICOLON expr TOK_SEMICOLON TOK_RPAREN stmt
			{
				if ($8->species == STMT_EMPTY) {
					$$ = tmknode(GEN_STMT, STMT_FOR_EE__,
						$3, $5);
					tsrc_pos($$, $1.srcpos, $8->srcpos);
					tfreenode($8);
				} else {
					$$ = tmknode(GEN_STMT, STMT_FOR_EE_S,
						$3, $5);
					tlist_add($$, $8);
					tsrc_pos($$, $1.srcpos, $8->srcpos);
				}
			}
		|	FOR TOK_LPAREN expr TOK_SEMICOLON TOK_SEMICOLON expr TOK_RPAREN stmt
			{
				if ($8->species == STMT_EMPTY) {
					$$ = tmknode(GEN_STMT, STMT_FOR_E_E_,
						$3, $6);
					tsrc_pos($$, $1.srcpos, $8->srcpos);
					tfreenode($8);
				} else {
					$$ = tmknode(GEN_STMT, STMT_FOR_E_ES,
						$3, $6);
					tlist_add($$, $8);
					tsrc_pos($$, $1.srcpos, $8->srcpos);
				}
			}
		|	FOR TOK_LPAREN expr TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN stmt
			{
				if ($7->species == STMT_EMPTY) {
					$$ = tmknode(GEN_STMT, STMT_FOR_E___,
						$3, 0);
					tsrc_pos($$, $1.srcpos, $7->srcpos);
					tfreenode($7);
				} else {
				        $$ = tmknode(GEN_STMT, STMT_FOR_E__S,
						     $3, $7);
				        tsrc_pos($$, $1.srcpos, $7->srcpos);
				}
			}
		|	FOR TOK_LPAREN TOK_SEMICOLON expr TOK_SEMICOLON expr TOK_RPAREN stmt
			{
				if ($8->species == STMT_EMPTY) {
					$$ = tmknode(GEN_STMT, STMT_FOR__EE_,
						$4, $6);
					tsrc_pos($$, $1.srcpos, $8->srcpos);
					tfreenode($8);
				} else {
					$$ = tmknode(GEN_STMT, STMT_FOR__EES,
						$4, $6);
					tlist_add($$, $8);
					tsrc_pos($$, $1.srcpos, $8->srcpos);
				}
			}
		|	FOR TOK_LPAREN TOK_SEMICOLON expr TOK_SEMICOLON TOK_RPAREN stmt
			{
				if ($7->species == STMT_EMPTY) {
					$$ = tmknode(GEN_STMT, STMT_FOR__E__,
						$4, 0);
					tsrc_pos($$, $1.srcpos, $7->srcpos);
					tfreenode($7);
				} else {
				        $$ = tmknode(GEN_STMT, STMT_FOR__E_S,
						$4, $7);
				        tsrc_pos($$, $1.srcpos, $7->srcpos);
				}
			}
		|	FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON expr TOK_RPAREN stmt
			{
				if ($7->species == STMT_EMPTY) {
					$$ = tmknode(GEN_STMT, STMT_FOR___E_,
						$5, 0);
					tsrc_pos($$, $1.srcpos, $7->srcpos);
					tfreenode($7);
				} else {
				        $$ = tmknode(GEN_STMT, STMT_FOR___ES,
						$5, $7);
					tsrc_pos($$, $1.srcpos, $7->srcpos);
				}
			}
		|	FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN stmt
			{
				$$ = tmknode(GEN_STMT, STMT_FOR____S, $6, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	SWITCH TOK_LPAREN expr TOK_RPAREN stmt
			{
				$$ = tmknode(GEN_STMT, STMT_SWITCH, $3, $5);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	CASE expr TOK_COLON stmt
			{
				$$ = tmknode(GEN_STMT, STMT_CASE, $2, $4);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	DEFAULT TOK_COLON stmt
			{
				$$ = tmknode(GEN_STMT, STMT_DEFAULT, $3, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	BREAK  TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_BREAK, 0, 0);
				tsrc_pos($$, $1.srcpos, $2.srcpos);
			}
		|	CONTINUE  TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_CONTINUE, 0, 0);
				tsrc_pos($$, $1.srcpos, $2.srcpos);
			}
		|	RETURN expr  TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_RETURN_EXPR,
					$2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		|	RETURN  TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_RETURN, 0, 0);
				tsrc_pos($$, $1.srcpos, $2.srcpos);
			}
		|	GOTO tname_or_name TOK_SEMICOLON
			{
				$$ = tmknode(GEN_STMT, STMT_GOTO, $2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		|	tname_or_name TOK_COLON stmt
			{
				$$ = tmknode(GEN_STMT, STMT_LABEL, $1, $3);
			}
		;
/*	EXPRESSIONS	*/
exp_list:
			expr				%prec TOK_COMMA
			{
				$$ = tmknode(GEN_EXP_LIST, 0, $1, 0);
			}
		|	exp_list TOK_COMMA expr
			{
				$$ = tlist_add($1, $3);
			}
		;
expr:
			expr relop expr			%prec TOK_GREATER
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1, $2);
				tlist_add($$, $3);
			}
		|
			expr equop expr			%prec EQUAL
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1, $2);
				tlist_add($$, $3);
			}
		|	expr TOK_COMMA expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_COMMA, $1, $3);
			}
		|	expr TOK_PLUS expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_PLUS,
						$2.srcpos, 0));
				tlist_add($$, $3);
			}
		|	expr TOK_DASH expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_MINUS,
						$2.srcpos, 0));
				tlist_add($$, $3);
			}
		|	expr TOK_STAR expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_MUL, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr TOK_SLASH expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_DIV, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr TOK_PERCENT expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_MOD, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr LSHIFT expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_LS, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr RSHIFT expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_RS, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr TOK_VERTICAL expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_OR, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr TOK_CARROT expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_ER, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr TOK_AMPER expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_AND, $2.srcpos,
						0));
				tlist_add($$, $3);
			}
		|	expr ANDAND expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_ANDAND,
						$2.srcpos, 0));
				tlist_add($$, $3);
			}
		|	expr OROR expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_OROR,
						$2.srcpos, 0));
				tlist_add($$, $3);
			}
		|	expr TOK_QMARK expr TOK_COLON expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_QCOLON, $1, $3);
				tlist_add($$, $5);
			}
		|	expr asop expr			%prec	TOK_EQUALS
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1, $2);
				tlist_add($$, $3);
			}
		|	expr TOK_EQUALS expr
			{
				$$ = tmknode(GEN_EXPR, EXPR_BINOP, $1,
					tmkleaf(GEN_BINOP, BINOP_ASGN,
						$2.srcpos, 0));
				tlist_add($$, $3);
			}
		|	term
			{
				$$ = $1;
			}
		;
term:
			term PLUSPLUS
			{
				$$ = tmknode(GEN_EXPR, EXPR_INCOP, $1,
					tmkleaf(GEN_INCOP, INCOP_INC, $2.srcpos,
						0));
			}
		|	term MINUSMINUS
			{
				$$ = tmknode(GEN_EXPR, EXPR_INCOP, $1,
					tmkleaf(GEN_INCOP, INCOP_DEC, $2.srcpos,
						0));
			}
		|	PLUSPLUS term
			{
				$$ = tmknode(GEN_EXPR, EXPR_UNOP,
					tmkleaf(GEN_UNOP, UNOP_INC, $1.srcpos,
						0),
					$2);
			}
		|	MINUSMINUS term
			{
				$$ = tmknode(GEN_EXPR, EXPR_UNOP,
					tmkleaf(GEN_UNOP, UNOP_DEC, $1.srcpos,
						0),
					$2);
			}
		|	TOK_AMPER term				%prec TOK_EXCLAIM
			{
				$$ = tmknode(GEN_EXPR, EXPR_UNOP,
					tmkleaf(GEN_UNOP, UNOP_AND, $1.srcpos,
						0),
					$2);
			}
		|	TOK_TILDE term 
			{
				$$ = tmknode(GEN_EXPR, EXPR_UNOP,
					tmkleaf(GEN_UNOP, UNOP_COMPL, $1.srcpos,
						0),
					$2);
			}
		|	TOK_EXCLAIM term 
			{
				$$ = tmknode(GEN_EXPR, EXPR_UNOP,
					tmkleaf(GEN_UNOP, UNOP_NOT, $1.srcpos,
						0),
					$2);
			}
		|	TOK_DASH term				%prec TOK_EXCLAIM
			{
				$$ = tmknode(GEN_EXPR, EXPR_UNOP,
					tmkleaf(GEN_UNOP, UNOP_MINUS, $1.srcpos,
						0),
					$2);
			}
		|	TOK_PLUS term				%prec TOK_EXCLAIM
			{
				/*
				* This rule is not in the internal grammar
				* so we have to fake it.
				*/
				$$ = $2;
			}
		|	SIZEOF term
			{
				$$ = tmknode(GEN_EXPR, EXPR_SIZEOF, $2, 0);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	TOK_LPAREN cast_type TOK_RPAREN term			%prec SIZEOF
			{
				$$ = tmknode(GEN_EXPR, EXPR_CAST, $2, $4);
				tsrc_pos($$, $1.srcpos, NULL);
			}
		|	SIZEOF TOK_LPAREN cast_type TOK_RPAREN		%prec SIZEOF
			{
				$$ = tmknode(GEN_EXPR, EXPR_SIZEOF_TYPE,
					$3, 0);
				tsrc_pos($$, $1.srcpos, $4.srcpos);
			}
		|	OFFSET TOK_LPAREN tname_or_name TOK_COMMA mname TOK_RPAREN
			{
				$$ = tmknode(GEN_EXPR, EXPR_OFFSET, $3, $5);
				tsrc_pos($$, $1.srcpos, $6.srcpos);
			}
		|	star term				%prec TOK_EXCLAIM
			{
				if ($2->species == EXPR_LSTAR) {
					tlist_ladd($2->down->over, $1);
					$$ = $2;
				} else {
					$$ = tmknode(GEN_EXPR, EXPR_LSTAR,
						tmknode(GEN_STARS, 0, $1, 0),
						$2);
				}
			}
		|	term TOK_LSQUARE expr TOK_RSQUARE
			{
				$$ = tmknode(GEN_EXPR, EXPR_LARRAY, $1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	term STREF tname_or_name
			{
				$$ = tmknode(GEN_EXPR, EXPR_LARROW, $1, $3);
			}
		|	term TOK_PERIOD tname_or_name
			{
				$$ = tmknode(GEN_EXPR, EXPR_LDOT, $1, $3);
			}
		|	name
			{
				$$ = tmknode(GEN_EXPR, EXPR_LNAME, $1, 0);
			}
		|	term TOK_LPAREN exp_list  TOK_RPAREN
			{
				TNODE *p;

				if ($1->species == EXPR_LNAME) {
					$1->genus = GEN_FUNC_LP;
					$1->species = FUNC_NAME_LP;
					$1->down->genus = GEN_FNAME;
					p = $1;
				} else {
					p = tmknode(GEN_FUNC_LP, FUNC_EXPR_LP,
						$1, 0);
				}
				$$ = tmknode(GEN_EXPR, EXPR_LFCALL1, p, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	term TOK_LPAREN TOK_RPAREN
			{
				TNODE *p;

				if ($1->species == EXPR_LNAME) {
					$1->genus = GEN_FUNC_LP;
					$1->species = FUNC_NAME_LP;
					$1->down->genus = GEN_FNAME;
					p = $1;
				} else {
					p = tmknode(GEN_FUNC_LP, FUNC_EXPR_LP,
						$1, 0);
				}
				$$ = tmknode(GEN_EXPR, EXPR_LFCALL0, p, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	ICON
			{
				$$ = tmknode(GEN_EXPR, EXPR_ICON, 
					tmkleaf(GEN_ICON, 0,
						$1.srcpos, $1.text),
					0);
			}
		|	FCON
			{
				$$ = tmknode(GEN_EXPR, EXPR_FCON,
					tmkleaf(GEN_FCON, 0,
						$1.srcpos, $1.text),
					0);
			}
		|	strings
			{
				$$ = tmknode(GEN_EXPR, EXPR_STRING, $1, 0);
			}
		|	TOK_LPAREN expr TOK_RPAREN
			{
				$$ = tmknode(GEN_EXPR, EXPR_INHERIT, $2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		;
strings:
			strings STRING
			{
				$$ = tlist_add($1, 
					tmkleaf(GEN_STRING, 0,
						$2.srcpos, $2.text));
			}
		|	STRING
			{
				$$ = tmknode(GEN_STRINGS, 0, 
					tmkleaf(GEN_STRING, 0,
						$1.srcpos, $1.text),
					0);
			}
		;
cast_type:
			classtypes null_dcl
			{
				if ($1->species == CLASSTYPES_TYPEDEF) {
					parse_error($1->srcpos,
						"typedef in cast");
					$1->species = CLASSTYPES_NORMAL;
				}
				if ($2)
					$$ = tmknode(GEN_CAST_TYPE, CAST_TYPE,
						$1, $2);
				else 
					$$ = tmknode(GEN_CAST_TYPE,
						CAST_TYPE_NULL, $1, NULL);
			}
		;
null_dcl:
			/* empty */			%prec T_NAME
			{
				$$ = NULL;
			}
		|	null_dcl TOK_LPAREN TOK_RPAREN
			{
				$$ = tmknode(GEN_NULL_DCL, NULL_N_FUNC, $1, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	null_dcl TOK_LPAREN ansi_params TOK_RPAREN
			{
				$$ = tmknode(GEN_NULL_DCL, NULL_ANSI, $1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	null_dcl TOK_LPAREN ansi_params TOK_COMMA ELLIPSIS TOK_RPAREN
			{
				$$ = tmknode(GEN_NULL_DCL, NULL_ANSI_E, $1, $3);
				tsrc_pos($$, NULL, $6.srcpos);
			}
		|	null_dcl TOK_LPAREN ELLIPSIS TOK_RPAREN
			{
				$$ = tmknode(GEN_NULL_DCL, NULL_E_ANSI, $1, 0);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	star null_dcl			%prec TOK_STAR
			{
				if ($2 == NULL)
					$$ = tmknode(GEN_NULL_DCL, NULL_STAR,
						$1, 0);
				else 
					$$ = tmknode(GEN_NULL_DCL, NULL_STAR_N,
						tmknode(GEN_STARS, 0, $1, 0),
						$2);
			}
		|	null_dcl TOK_LSQUARE TOK_RSQUARE
			{
				if ($1 == NULL)
					$$ = tmknode(GEN_NULL_DCL, NULL_SUB,
						0, 0);
				else 
					$$ = tmknode(GEN_NULL_DCL, NULL_N_SUB,
						$1, 0);
				tsrc_pos($$, NULL, $3.srcpos);
			}
		|	null_dcl TOK_LSQUARE expr TOK_RSQUARE
			{
				if ($1 == NULL)
					$$ = tmknode(GEN_NULL_DCL, NULL_SUB_E,
						$3, 0);
				else 
					$$ = tmknode(GEN_NULL_DCL,NULL_N_SUB_E,
						$1, $3);
				tsrc_pos($$, NULL, $4.srcpos);
			}
		|	TOK_LPAREN null_dcl TOK_RPAREN
			{
				if ($2 == NULL)
					$$ = tmknode(GEN_NULL_DCL,
						NULL_INHERIT, 0, 0);
				else 
					$$ = tmknode(GEN_NULL_DCL,
						NULL_INHERIT_N, $2, 0);
				tsrc_pos($$, $1.srcpos, $3.srcpos);
			}
		;
mname:
			name
		|	mname TOK_PERIOD name
		;

tname_or_name:						/* saul 8/22/89 */
			name
		|
			t_name
		;
t_name:
			T_NAME
			{
				$$ = tmkleaf(GEN_TNAME, 0,
					$1.srcpos, $1.text);
			}
		;
name:
			NAME
			{
				$$ = tmkleaf(GEN_NAME, 0, $1.srcpos, $1.text);
			}
		;
star:			
			TOK_STAR
			{
				$$ = tmkleaf(GEN_STAR, STAR_NORMAL, $1.srcpos, 0);
			}
		|
			TOK_STAR qualifiers
			{
				$$ = tmknode(GEN_STAR, STAR_QUALS, $2, 0);
			}
		;
qualifiers:
			qualifier
			{
				$$ = tmknode(GEN_QUALS, 0, $1, 0);
			}
		|	qualifiers qualifier
			{
				$$ = tlist_add($1, $2);
			}
		;
qualifier:
			CONST
			{
				$$ = tmkleaf(GEN_QUAL, QUAL_CONST, $1.srcpos, 0);
			}
		|
			VOLATILE
			{
				$$ = tmkleaf(GEN_QUAL, QUAL_VOLATILE, $1.srcpos, 0);
			}
		;
asop:			PLUS_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_APLUS,
					$1.srcpos, 0);
			}
		|
			MINUS_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_AMINUS,
					$1.srcpos, 0);
			}
		|
			MUL_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_AMUL,
					$1.srcpos, 0);
			}
		|
			DIV_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_ADIV,
					$1.srcpos, 0);
			}
		|
			MOD_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_AMOD,
					$1.srcpos, 0);
			}
		|
			LS_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_ALS,
					$1.srcpos, 0);
			}
		|
			RS_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_ARS,
					$1.srcpos, 0);
			}
		|
			AND_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_AAND,
					$1.srcpos, 0);
			}
		|
			OR_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_AOR,
					$1.srcpos, 0);
			}
		|
			ER_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_AER,
					$1.srcpos, 0);
			}
		;
relop:			GT_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_GTEQ,
					$1.srcpos, 0);
			}
		|
			LT_EQ
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_LTEQ,
					$1.srcpos, 0);
			}
		|
			TOK_GREATER
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_GT, $1.srcpos, 0);
			}
		|
			TOK_LESSER
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_LT, $1.srcpos, 0);
			}
		;
equop:			EQUAL
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_EQ, $1.srcpos, 0);
			}
		|
			BANG_EQUAL
			{
				$$ = tmkleaf(GEN_BINOP, BINOP_NEQ,
					$1.srcpos, 0);
			}
		;
%%
#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory\n"))

int
parse(srcfile, tree, uprefix)
FILE	*srcfile;
TNODE	**tree;
char	**uprefix;
{
	int	status;

	scan_init(srcfile);

	status = yyparse();

	scan_end(uprefix);

	*tree = tree_root;

	return status;
}

static void
insertTypeNames(node)
TNODE	*node;
{
	TNODE	*p;
	TNODE	*item;

	if (node->genus != GEN_DATA_SPECS) {
		internal_error(node->srcpos, "Unexpected genus: %d",
			node->genus);
	}

	for (p = CHILD0(node); p != NULL; p = TNEXT(p)) {
		item = CHILD0(p);
		while (item->genus == GEN_DATA_ITEM) {
			if (item->species == FUNC_STARS_SPEC)
				item = CHILD1(item);
			else item = CHILD0(item);
		}
		scan_setType(item->text);
	}
}

