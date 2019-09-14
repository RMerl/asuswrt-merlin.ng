/* Bison file for rsyslog config format v2 (RainerScript).
 * Please note: this file introduces the new config format, but maintains
 * backward compatibility. In order to do so, the grammar is not 100% clean,
 * but IMHO still sufficiently easy both to understand for programmers
 * maitaining the code as well as users writing the config file. Users are,
 * of course, encouraged to use new constructs only. But it needs to be noted
 * that some of the legacy constructs (specifically the in-front-of-action
 * PRI filter) are very hard to beat in ease of use, at least for simpler
 * cases.
 *
 * Copyright 2011-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
%{
#define IN_GRAMMAR_Y /* tell parserif.h not to redefine things! */

#include "config.h"
#include <stdio.h>
#include <libestr.h>
#include "rainerscript.h"
#include "parserif.h"
#define YYDEBUG 1
extern int yylineno;
extern char *yytext;

/* keep compile rule clean of errors */
extern int yylex(void);
extern int yyerror(const char*);
%}

%union {
	char *s;
	long long n;
	es_str_t *estr;
	enum cnfobjType objType;
	struct cnfobj *obj;
	struct cnfstmt *stmt;
	struct nvlst *nvlst;
	struct objlst *objlst;
	struct cnfexpr *expr;
	struct cnfarray *arr;
	struct cnffunc *func;
	struct cnffparamlst *fparams;
	struct cnfitr *itr;
}

%token <estr> NAME
%token <estr> FUNC
%token <objType> BEGINOBJ
%token ENDOBJ
%token BEGIN_INCLUDE
%token BEGIN_ACTION
%token BEGIN_PROPERTY
%token BEGIN_CONSTANT
%token BEGIN_TPL
%token BEGIN_RULESET
%token STOP
%token SET
%token RESET
%token UNSET
%token CONTINUE
%token <cnfstmt> CALL
%token <cnfstmt> CALL_INDIRECT
%token <s> LEGACY_ACTION
%token <s> LEGACY_RULESET
%token <s> PRIFILT
%token <s> PROPFILT
%token <s> BSD_TAG_SELECTOR
%token <s> BSD_HOST_SELECTOR
%token <s> RELOAD_LOOKUP_TABLE_PROCEDURE
%token IF
%token THEN
%token ELSE
%token FOREACH
%token ITERATOR_ASSIGNMENT
%token DO
%token OR
%token AND
%token NOT
%token <s> VAR
%token <estr> STRING
%token <n> NUMBER
%token CMP_EQ
%token CMP_NE
%token CMP_LE
%token CMP_GE
%token CMP_LT
%token CMP_GT
%token CMP_CONTAINS
%token CMP_CONTAINSI
%token CMP_STARTSWITH
%token CMP_STARTSWITHI

%type <nvlst> nv nvlst value
%type <obj> obj property constant
%type <objlst> propconst
%type <expr> expr
%type <stmt> stmt s_act actlst block script
%type <itr> iterator_decl
%type <fparams> fparams
%type <arr> array arrayelt

%left AND OR
%left CMP_EQ CMP_NE CMP_LE CMP_GE CMP_LT CMP_GT CMP_CONTAINS CMP_CONTAINSI CMP_STARTSWITH CMP_STARTSWITHI
%left '+' '-' '&'
%left '*' '/' '%'
%nonassoc UMINUS NOT

%expect 1 /* dangling else */
/* If more erors show up, Use "bison -v grammar.y" if more conflicts arise and
 * check grammar.output for were exactly these conflicts exits.
 */
%%
/* note: we use left recursion below, because that saves stack space AND
 * offers the right sequence so that we can submit the top-layer objects
 * one by one.
 */
conf:	/* empty (to end recursion) */
	| conf obj			{ cnfDoObj($2); }
	| conf stmt			{ cnfDoScript($2); }
	| conf LEGACY_RULESET		{ cnfDoCfsysline($2); }
	| conf BSD_TAG_SELECTOR		{ cnfDoBSDTag($2); }
	| conf BSD_HOST_SELECTOR	{ cnfDoBSDHost($2); }
include:  BEGIN_INCLUDE nvlst ENDOBJ	{ includeProcessCnf($2); }
obj:	  BEGINOBJ nvlst ENDOBJ 	{ $$ = cnfobjNew($1, $2); }
        | BEGIN_TPL nvlst ENDOBJ	{ $$ = cnfobjNew(CNFOBJ_TPL, $2); }
        | BEGIN_TPL nvlst ENDOBJ '{' propconst '}'
					{ $$ = cnfobjNew(CNFOBJ_TPL, $2);
					  $$->subobjs = $5;
					}
        | BEGIN_RULESET nvlst ENDOBJ '{' script '}'
					{ $$ = cnfobjNew(CNFOBJ_RULESET, $2);
					  $$->script = $5;
					}
        | BEGIN_RULESET nvlst ENDOBJ '{' '}'
					{ $$ = cnfobjNew(CNFOBJ_RULESET, $2);
					  $$->script = NULL;
					}
propconst:				{ $$ = NULL; }
	| propconst property		{ $$ = objlstAdd($1, $2); }
	| propconst constant		{ $$ = objlstAdd($1, $2); }
property: BEGIN_PROPERTY nvlst ENDOBJ	{ $$ = cnfobjNew(CNFOBJ_PROPERTY, $2); }
constant: BEGIN_CONSTANT nvlst ENDOBJ	{ $$ = cnfobjNew(CNFOBJ_CONSTANT, $2); }
nvlst:					{ $$ = NULL; }
	| nvlst nv 			{ $2->next = $1; $$ = $2; }
nv:	NAME '=' value 			{ $$ = nvlstSetName($3, $1); }
value:	  STRING			{ $$ = nvlstNewStr($1); }
	| array				{ $$ = nvlstNewArray($1); }
script:	  stmt				{ $$ = $1; }
	| script stmt			{ $$ = scriptAddStmt($1, $2); }
stmt:	  actlst			{ $$ = $1; }
	| IF expr THEN block 		{ $$ = cnfstmtNew(S_IF);
					  $$->d.s_if.expr = $2;
					  $$->d.s_if.t_then = $4;
					  $$->d.s_if.t_else = NULL; }
	| IF expr THEN block ELSE block	{ $$ = cnfstmtNew(S_IF);
					  $$->d.s_if.expr = $2;
					  $$->d.s_if.t_then = $4;
					  $$->d.s_if.t_else = $6; }
	| FOREACH iterator_decl DO block { $$ = cnfstmtNew(S_FOREACH);
					  $$->d.s_foreach.iter = $2;
					  $$->d.s_foreach.body = $4;}
	| RESET VAR '=' expr ';'	{ $$ = cnfstmtNewSet($2, $4, 1); }
	| SET VAR '=' expr ';'		{ $$ = cnfstmtNewSet($2, $4, 0); }
	| UNSET VAR ';'			{ $$ = cnfstmtNewUnset($2); }
	| PRIFILT block			{ $$ = cnfstmtNewPRIFILT($1, $2); }
	| PROPFILT block		{ $$ = cnfstmtNewPROPFILT($1, $2); }
	| RELOAD_LOOKUP_TABLE_PROCEDURE '(' fparams ')' { $$ = cnfstmtNewReloadLookupTable($3);}
	| include			{ $$ = NULL; }
	| BEGINOBJ			{ $$ = NULL; parser_errmsg("declarative object '%s' not permitted in action block [stmt]", yytext);}
block:    stmt				{ $$ = $1; }
	| '{' script '}'		{ $$ = $2; }
actlst:	  s_act				{ $$ = $1; }
	| actlst '&' s_act 		{ $$ = scriptAddStmt($1, $3); }
/* s_act are actions and action-like statements */
s_act:	  BEGIN_ACTION nvlst ENDOBJ	{ $$ = cnfstmtNewAct($2); }
	| LEGACY_ACTION			{ $$ = cnfstmtNewLegaAct($1); }
	| STOP				{ $$ = cnfstmtNew(S_STOP); }
	| CALL NAME			{ $$ = cnfstmtNewCall($2); }
	| CALL_INDIRECT expr ';'	{ $$ = cnfstmtNew(S_CALL_INDIRECT);
					  $$->d.s_call_ind.expr = $2;
					}
	| CONTINUE			{ $$ = cnfstmtNewContinue(); }
expr:	  expr AND expr			{ $$ = cnfexprNew(AND, $1, $3); }
	| expr OR expr			{ $$ = cnfexprNew(OR, $1, $3); }
	| NOT expr			{ $$ = cnfexprNew(NOT, NULL, $2); }
	| expr CMP_EQ expr		{ $$ = cnfexprNew(CMP_EQ, $1, $3); }
	| expr CMP_NE expr		{ $$ = cnfexprNew(CMP_NE, $1, $3); }
	| expr CMP_LE expr		{ $$ = cnfexprNew(CMP_LE, $1, $3); }
	| expr CMP_GE expr		{ $$ = cnfexprNew(CMP_GE, $1, $3); }
	| expr CMP_LT expr		{ $$ = cnfexprNew(CMP_LT, $1, $3); }
	| expr CMP_GT expr		{ $$ = cnfexprNew(CMP_GT, $1, $3); }
	| expr CMP_CONTAINS expr	{ $$ = cnfexprNew(CMP_CONTAINS, $1, $3); }
	| expr CMP_CONTAINSI expr	{ $$ = cnfexprNew(CMP_CONTAINSI, $1, $3); }
	| expr CMP_STARTSWITH expr	{ $$ = cnfexprNew(CMP_STARTSWITH, $1, $3); }
	| expr CMP_STARTSWITHI expr	{ $$ = cnfexprNew(CMP_STARTSWITHI, $1, $3); }
	| expr '&' expr			{ $$ = cnfexprNew('&', $1, $3); }
	| expr '+' expr			{ $$ = cnfexprNew('+', $1, $3); }
	| expr '-' expr			{ $$ = cnfexprNew('-', $1, $3); }
	| expr '*' expr			{ $$ = cnfexprNew('*', $1, $3); }
	| expr '/' expr			{ $$ = cnfexprNew('/', $1, $3); }
	| expr '%' expr			{ $$ = cnfexprNew('%', $1, $3); }
	| '(' expr ')'			{ $$ = $2; }
	| '-' expr %prec UMINUS		{ $$ = cnfexprNew('M', NULL, $2); }
	| FUNC '(' ')'			{ $$ = (struct cnfexpr*) cnffuncNew($1, NULL); }
	| FUNC '(' fparams ')'		{ $$ = (struct cnfexpr*) cnffuncNew($1, $3); }
	| NUMBER			{ $$ = (struct cnfexpr*) cnfnumvalNew($1); }
	| STRING			{ $$ = (struct cnfexpr*) cnfstringvalNew($1); }
	| VAR				{ $$ = (struct cnfexpr*) cnfvarNew($1); }
	| array				{ $$ = (struct cnfexpr*) $1; }
fparams:  expr				{ $$ = cnffparamlstNew($1, NULL); }
	| expr ',' fparams		{ $$ = cnffparamlstNew($1, $3); }
array:	 '[' arrayelt ']'		{ $$ = $2; }
iterator_decl:  '(' VAR ITERATOR_ASSIGNMENT expr ')'	{ $$ = cnfNewIterator($2, $4); }
arrayelt: STRING			{ $$ = cnfarrayNew($1); }
	| arrayelt ',' STRING		{ $$ = cnfarrayAdd($1, $3); }

%%
/*
int yyerror(char *s)
{
	printf("parse failure on or before line %d: %s\n", yylineno, s);
	return 0;
}
*/
