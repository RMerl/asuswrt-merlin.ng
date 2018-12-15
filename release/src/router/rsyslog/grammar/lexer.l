 /* Lex file for rsyslog config format v2 (RainerScript).
  * Please note: this file introduces the new config format, but maintains
  * backward compatibility. In order to do so, the grammar is not 100% clean,
  * but IMHO still sufficiently easy both to understand for programmers
  * maitaining the code as well as users writing the config file. Users are,
  * of course, encouraged to use new constructs only. But it needs to be noted
  * that some of the legacy constructs (specifically the in-front-of-action
  * PRI filter) are very hard to beat in ease of use, at least for simpler
  * cases. So while we hope that cfsysline support can be dropped some time in
  * the future, we will probably keep these useful constructs.
  *
  * Copyright 2011-2014 Rainer Gerhards and Adiscon GmbH.
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

%top{
#ifndef __clang_analyzer__ /* this is not really our code */
#include "config.h"
#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#endif
}

%{
#include <libestr.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "rsyslog.h"
#include "srUtils.h"
#include "parserif.h"

/* TODO: move this to a better modules, refactor -- rgerhards, 2018-01-22 */
static char *
read_file(const char *const filename)
{
	char *content = NULL;
	int fd = -1;
	struct stat sb;
	ssize_t nread;
	assert(filename != NULL);

	if((fd = open((const char*) filename, O_RDONLY)) == -1) {
		goto done;
	}

	if(fstat(fd, &sb) == -1) {
		goto done;
	}

	content = malloc(sb.st_size+1);
	if(content == NULL) {
		goto done;
	}

	nread = read(fd, content, sb.st_size);
	content[nread] = '\0';
	if(nread != (ssize_t) sb.st_size) {
		free(content);
		content = NULL;
		goto done;
	}

done:
	if(fd != -1) {
		close(fd);
	}
	return content;
}

static es_str_t* ATTR_NONNULL(1)
expand_backticks_echo(const char *param)
{
	assert(param != NULL);
	assert(strncmp(param, "echo ", sizeof("echo ")-1) == 0);
	char envvar[512];
	int i_envvar = 0;
	int in_env = 0;
	es_str_t *estr;

	param += sizeof("echo ")-1;
	if((estr = es_newStr(strlen(param))) == NULL) {
		goto done;
	}

	while(*param) {
		if(in_env) {
			if(isspace(*param) || *param == '/') {
				envvar[i_envvar] = '\0';
				const char *envval = getenv(envvar);
				es_addBuf(&estr, envval, strlen(envval));
				es_addChar(&estr, *param); /* curr char part of output! */
				i_envvar = 0;
				in_env = 0;
			} else if(i_envvar > sizeof(envvar) - 1) {
				parser_errmsg("environment variable too long, begins with %s", envvar);
				goto done;
			} else {
				envvar[i_envvar++] = *param;
			}
		} else if (*param == '$') {
			in_env = 1;
		} else {
			es_addChar(&estr, *param);
		}
		++param;
	}

	/* final check, we may be in env var name (very probable!) */
	if(in_env) {
		envvar[i_envvar] = '\0';
		const char *envval = getenv(envvar);
		es_addBuf(&estr, envval, strlen(envval));
	}

done:	return estr;
}


static es_str_t* ATTR_NONNULL(1)
expand_backticks(char *const param)
{
	es_str_t *estr;
	assert(param != NULL);

	if(strncmp(param, "echo ", sizeof("echo ")-1) == 0) {
		estr = expand_backticks_echo(param);
	} else if(strncmp(param, "cat ", sizeof("cat ")-1) == 0) {
		const char *val = read_file(param+4);
		estr = es_newStrFromCStr(val, strlen(val));
		free((void*) val);
	} else {
		parser_errmsg("invalid backtick parameter `%s` - replaced by "
			"empty string (\"\")", param);
		estr = es_newStr(1);
	}

	return estr;
}
%}

%option noyywrap nodefault case-insensitive yylineno
 /*%option noyywrap nodefault case-insensitive */

/* avoid compiler warning: `yyunput' defined but not used */
%option nounput noinput


%x INOBJ
	/* INOBJ is selected if we are inside an object (name/value pairs!) */
%x COMMENT
	/* COMMENT is "the usual trick" to handle C-style comments */
%x INCL
	/* INCL is in $IncludeConfig processing (skip to include file) */
%x LINENO
	/* LINENO: support for setting the linenumber */
%x INCALL
	/* INCALL: support for the call statement */
%x IN_PROCEDURE_CALL
	/* IN_PROCEDURE_CALL: support for the call statement */	
%x EXPR
	/* EXPR is a bit ugly, but we need it to support pre v6-syntax. The problem
	 * is that cfsysline statement start with $..., the same like variables in
	 * an expression. However, cfsysline statements can never appear inside an
	 * expression. So we create a specific expr mode, which is turned on after 
	 * we lexed a keyword that needs to be followed by an expression (using 
	 * knowledge from the upper layer...). In expr mode, we strictly do
	 * expression-based parsing. Expr mode is stopped when we reach a token
	 * that can not be part of an expression (currently only "then"). As I
	 * wrote this ugly, but the price needed to pay in order to remain
	 * compatible to the previous format.
	 */
%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libestr.h>
#include "rainerscript.h"
#include "parserif.h"
#include "grammar.h"
static int preCommentState;	/* save for lex state before a comment */

struct bufstack {
	struct bufstack *prev;
	YY_BUFFER_STATE bs;
	int lineno;
	char *fn;
	es_str_t *estr;
} *currbs = NULL;

char *cnfcurrfn;			/* name of currently processed file */

int popfile(void);
int cnfSetLexFile(char *fname);

extern int yydebug;

/* somehow, I need these prototype even though the headers are 
 * included. I guess that's some autotools magic I don't understand...
 */
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) \
	&& !defined(__DragonflyBSD__) && !defined(_AIX)
int fileno(FILE *stream);
#endif


%}

%%

 /* keywords */
"if"				{ BEGIN EXPR; return IF; }
"foreach"			{ BEGIN EXPR; return FOREACH; }
"reload_lookup_table"		{ BEGIN IN_PROCEDURE_CALL; return RELOAD_LOOKUP_TABLE_PROCEDURE; }
<IN_PROCEDURE_CALL>"("		{ return yytext[0]; }
<IN_PROCEDURE_CALL>\'([^'\\]|\\['"\\$bntr]|\\x[0-9a-f][0-9a-f]|\\[0-7][0-7][0-7])*\'	 {
				   yytext[yyleng-1] = '\0';
				   unescapeStr((uchar*)yytext+1, yyleng-2);
				   yylval.estr = es_newStrFromBuf(yytext+1, strlen(yytext)-1);
				   return STRING; }
<IN_PROCEDURE_CALL>\"([^"\\$]|\\["'\\$bntr]|\\x[0-9a-f][0-9a-f]|\\[0-7][0-7][0-7])*\" {
				   yytext[yyleng-1] = '\0';
				   unescapeStr((uchar*)yytext+1, yyleng-2);
				   yylval.estr = es_newStrFromBuf(yytext+1, strlen(yytext)-1);
				   return STRING; }
<IN_PROCEDURE_CALL>"," 		{ return yytext[0]; }
<IN_PROCEDURE_CALL>")" 		{ BEGIN INITIAL; return yytext[0]; }
<IN_PROCEDURE_CALL>[ \t\n]* 		{  }
<IN_PROCEDURE_CALL>. 		{ parser_errmsg("invalid character '%s' in expression "
					        "- is there an invalid escape sequence somewhere?",
						yytext); }
<EXPR>"("			{ BEGIN EXPR; return yytext[0]; }
<EXPR>"then"			{ BEGIN INITIAL; return THEN; }
<EXPR>"do"			{ BEGIN INITIAL; return DO; }
<EXPR>";"			{ BEGIN INITIAL; return ';'; }
<EXPR>"or"			{ return OR; }
<EXPR>"and"			{ return AND; }
<EXPR>"not"			{ return NOT; }
<EXPR>"=" |
<EXPR>"," |
<EXPR>"*" |
<EXPR>"/" |
<EXPR>"%" |
<EXPR>"+" |
<EXPR>"&" |
<EXPR>"-" |
<EXPR>"[" |
<EXPR>"]" |
<EXPR>")"			{ return yytext[0]; }
<EXPR>"=="			{ return CMP_EQ; }
<EXPR>"<="			{ return CMP_LE; }
<EXPR>">="			{ return CMP_GE; }
<EXPR>"!=" |
<EXPR>"<>"			{ return CMP_NE; }
<EXPR>"<"			{ return CMP_LT; }
<EXPR>">"			{ return CMP_GT; }
<EXPR>"contains"		{ return CMP_CONTAINS; }
<EXPR>"in"		        { return ITERATOR_ASSIGNMENT; }
<EXPR>"contains_i"		{ return CMP_CONTAINSI; }
<EXPR>"startswith"		{ return CMP_STARTSWITH; }
<EXPR>"startswith_i"		{ return CMP_STARTSWITHI; }
<EXPR>0[0-7]+ |			/* octal number */
<EXPR>0x[0-9a-f]+ |		/* hex number, following rule is dec; strtoll handles all! */
<EXPR>([1-9][0-9]*|0)		{ yylval.n = strtoll(yytext, NULL, 0); return NUMBER; }
<EXPR>\$[$!./]{0,1}[@a-z_]*[!@a-z0-9\-_\.\[\]]*	{ yylval.s = strdup(yytext+1); return VAR; }
<EXPR>\'([^'\\]|\\['"\\$bntr]|\\x[0-9a-f][0-9a-f]|\\[0-7][0-7][0-7])*\'	 {
				   yytext[yyleng-1] = '\0';
				   unescapeStr((uchar*)yytext+1, yyleng-2);
				   yylval.estr = es_newStrFromBuf(yytext+1, strlen(yytext)-1);
				   return STRING; }
<EXPR>`([^`\\]|\\['`"\\bntr]|\\x[0-9a-f][0-9a-f]|\\[0-7][0-7][0-7])*`	 {
				   yytext[yyleng-1] = '\0';
				   unescapeStr((uchar*)yytext+1, yyleng-2);
				   yylval.estr = expand_backticks(yytext+1);
				   return STRING; }
<EXPR>\"([^"\\$]|\\["'\\$bntr]|\\x[0-9a-f][0-9a-f]|\\[0-7][0-7][0-7])*\" {
				   yytext[yyleng-1] = '\0';
				   unescapeStr((uchar*)yytext+1, yyleng-2);
				   yylval.estr = es_newStrFromBuf(yytext+1, strlen(yytext)-1);
				   return STRING; }
<EXPR>[ \t\n]
<EXPR>[a-z][a-z0-9_]*		{ yylval.estr = es_newStrFromCStr(yytext, yyleng);
				  return FUNC; }
<EXPR>.				{ parser_errmsg("invalid character '%s' in expression "
					        "- is there an invalid escape sequence somewhere?",
						yytext); }
<INCALL>[ \t\n]
<INCALL>.			{ parser_errmsg("invalid character '%s' in 'call' statement"
					        "- is there an invalid escape sequence somewhere?",
						yytext); }
<INCALL>[a-zA-Z][a-zA-Z0-9\-_\.]*	{ yylval.estr = es_newStrFromCStr(yytext, yyleng);
				  BEGIN INITIAL;
				  return NAME; }
"&"				{ return '&'; }
"{"				{ return '{'; }
"}"				{ return '}'; }
"stop"				{ return STOP; }
"else"				{ return ELSE; }
"call"				{ BEGIN INCALL; return CALL; }
"call_indirect"			{ BEGIN EXPR; return CALL_INDIRECT; }
"set"				{ BEGIN EXPR; return SET; }
"reset"				{ BEGIN EXPR; return RESET; }
"unset"				{ BEGIN EXPR; return UNSET; }
"continue"			{ return CONTINUE; }
 /* line number support because the "preprocessor" combines lines and so needs
  * to tell us the real source line.
  */
"preprocfilelinenumber("	{ BEGIN LINENO; }
<LINENO>[0-9]+			{ yylineno = atoi(yytext) - 1; }
<LINENO>")"			{ BEGIN INITIAL; }
<LINENO>.|\n
 /* $IncludeConfig must be detected as part of CFSYSLINE, because this is
  * always the longest match :-(
  */
<INCL>.|\n
<INCL>[^ \t\n]+			{ if(cnfDoInclude(yytext, 0) != 0)
					yyterminate();
				  BEGIN INITIAL; }
"main_queue"[ \n\t]*"("		{ yylval.objType = CNFOBJ_MAINQ;
				  BEGIN INOBJ; return BEGINOBJ; }
"timezone"[ \n\t]*"("		{ yylval.objType = CNFOBJ_TIMEZONE;
				  BEGIN INOBJ; return BEGINOBJ; }
"parser"[ \n\t]*"("		{ yylval.objType = CNFOBJ_PARSER;
				  BEGIN INOBJ; return BEGINOBJ; }
"global"[ \n\t]*"("		{ yylval.objType = CNFOBJ_GLOBAL;
				  BEGIN INOBJ; return BEGINOBJ; }
"template"[ \n\t]*"("		{ yylval.objType = CNFOBJ_TPL;
				  BEGIN INOBJ; return BEGIN_TPL; }
"ruleset"[ \n\t]*"("		{ yylval.objType = CNFOBJ_RULESET;
				  BEGIN INOBJ; return BEGIN_RULESET; }
"property"[ \n\t]*"("		{ yylval.objType = CNFOBJ_PROPERTY;
				  BEGIN INOBJ; return BEGIN_PROPERTY; }
"constant"[ \n\t]*"("		{ yylval.objType = CNFOBJ_CONSTANT;
				  BEGIN INOBJ; return BEGIN_CONSTANT; }
"input"[ \n\t]*"("		{ yylval.objType = CNFOBJ_INPUT;
				  BEGIN INOBJ; return BEGINOBJ; }
"module"[ \n\t]*"("		{ yylval.objType = CNFOBJ_MODULE;
				  BEGIN INOBJ; return BEGINOBJ; }
"lookup_table"[ \n\t]*"("	{ yylval.objType = CNFOBJ_LOOKUP_TABLE;
				  BEGIN INOBJ; return BEGINOBJ; }
"dyn_stats"[ \n\t]*"("		{ yylval.objType = CNFOBJ_DYN_STATS;
				  BEGIN INOBJ; return BEGINOBJ; }
"include"[ \n\t]*"("		{ BEGIN INOBJ; return BEGIN_INCLUDE; }
"action"[ \n\t]*"("		{ BEGIN INOBJ; return BEGIN_ACTION; }
^[ \t]*:\$?[a-z\-]+[ ]*,[ ]*!?[a-z]+[ ]*,[ ]*\"(\\\"|[^\"])*\"	{
				  yylval.s = strdup(rmLeadingSpace(yytext));
				  dbgprintf("lexer: propfilt is '%s'\n", yylval.s);
				  return PROPFILT;
				  }
^[ \t]*[\*a-z][\*a-z]*[0-7]*[\.,][,!=;\.\*a-z0-7]+ { yylval.s = strdup(rmLeadingSpace(yytext)); return PRIFILT; }
"~" |
"*" |
\-\/[^*][^\n]* |
\/[^*][^\n]* |
:[a-z0-9]+:[^\n]* |
[\|\.\-\@\^?~>][^\n]+ |
[a-z0-9_][a-z0-9_\-\+,;]*	{ yylval.s = yytext; return LEGACY_ACTION; }
<INOBJ>")"			{ BEGIN INITIAL; return ENDOBJ; }
<INOBJ>[a-z][a-z0-9_\.]*	{ yylval.estr = es_newStrFromCStr(yytext, yyleng);
				  return NAME; }
<INOBJ>"," |
<INOBJ>"[" |
<INOBJ>"]" |
<INOBJ>"="			{ return(yytext[0]); }
<INOBJ>\"([^"\\]|\\['"?\\abfnrtv]|\\[0-7]{1,3})*\" {
				   yytext[yyleng-1] = '\0';
				   unescapeStr((uchar*)yytext+1, yyleng-2);
				   yylval.estr = es_newStrFromBuf(yytext+1, strlen(yytext)-1);
				   return STRING; }
<INOBJ>`([^`\\]|\\['`?\\abfnrtv]|\\[0-7]{1,3})*` {
				   yytext[yyleng-1] = '\0';
				   unescapeStr((uchar*)yytext+1, yyleng-2);
				   yylval.estr = expand_backticks(yytext+1);
				   return STRING; }
				  /*yylval.estr = es_newStrFromBuf(yytext+1, yyleng-2);
				  return VALUE; }*/
"/*"				{ preCommentState = YY_START; BEGIN COMMENT; }
<INOBJ>"/*"			{ preCommentState = YY_START; BEGIN COMMENT; }
<EXPR>"/*"			{ preCommentState = YY_START; BEGIN COMMENT; }
<COMMENT>"*/"			{ BEGIN preCommentState; }
<COMMENT>([^*]|\n)+|.
<INOBJ>#.*$	/* skip comments in input */
<INOBJ>[ \n\t]
<INOBJ>.			{ parser_errmsg("invalid character '%s' in object definition "
					        "- is there an invalid escape sequence somewhere?",
						yytext); }
\$[a-z]+.*$			{ /* see comment on $IncludeConfig above */
				  if(!strncasecmp(yytext, "$includeconfig ", 14)) {
					yyless((yy_size_t)14);
				  	BEGIN INCL;
				  } else if(!strncasecmp(yytext, "$ruleset ", 9)) {
					yylval.s = strdup(yytext);
					return LEGACY_RULESET;
				  } else {
					  cnfDoCfsysline(strdup(yytext)); 
				  }
				}
![^ \t\n]+[ \t]*$		{ yylval.s = strdup(yytext); return BSD_TAG_SELECTOR; }
[+-]\*[ \t\n]*#.*$		{ yylval.s = strdup(yytext); return BSD_HOST_SELECTOR; }
[+-]\*[ \t\n]*$			{ yylval.s = strdup(yytext); return BSD_HOST_SELECTOR; }
^[ \t]*[+-][a-z0-9.:-]+[ \t]*$	{ yylval.s = strdup(yytext); return BSD_HOST_SELECTOR; }
\#.*\n	/* skip comments in input */
[\n\t ]	/* drop whitespace */
.				{ parser_errmsg("invalid character '%s' "
					        "- is there an invalid escape sequence somewhere?",
						yytext); }
<<EOF>>				{ if(popfile() != 0) yyterminate(); }

%%
int
cnfParseBuffer(char *buf, unsigned lenBuf)
{
	struct bufstack *bs;
	int r = 0;
	yydebug = 1;
	BEGIN INITIAL;
	/* maintain stack */
	if((bs = malloc(sizeof(struct bufstack))) == NULL) {
		r = 1;
		goto done;
	}

	if(currbs != NULL)
		currbs->lineno = yylineno;
	bs->prev = currbs;
	bs->fn = strdup("*buffer*");
	bs->bs = yy_scan_buffer(buf, lenBuf);
	bs->estr = NULL;
	currbs = bs;
	cnfcurrfn = bs->fn;
	yylineno = 1;
done:	return r;
}


/* add config file or text the the stack of config objects to be
 * processed.
 * cnfobjname is either the file name or "text" if generated from
 * text ("text" can also be replaced by something more intelligent
 * by the caller.
 * The provided string is freed.
 */
int ATTR_NONNULL()
cnfAddConfigBuffer(es_str_t *const str, const char *const cnfobj_name)
{
	struct bufstack *bs;
	int r = 0;
	assert(str != NULL);
	assert(cnfobj_name != NULL);

	if((bs = malloc(sizeof(struct bufstack))) == NULL) {
		r = 1;
		goto done;
	}

	if(currbs != NULL)
		currbs->lineno = yylineno;
	bs->prev = currbs;
	bs->fn = strdup(cnfobj_name);
	yy_size_t lll = es_strlen(str);
	bs->bs = yy_scan_buffer((char*)es_getBufAddr(str), lll);
	bs->estr = str; /* needed so we can free it later */
	currbs = bs;
	cnfcurrfn = bs->fn;
	yylineno = 1;
	dbgprintf("config parser: pushed config fragment on top of stack: %s\n", cnfobj_name);

done:
	if(r != 0) {
		es_deleteStr(str);
	}
	return r;
}


/* set a new buffers. Returns 0 on success, 1 on error, 2 on file not exists.
 * note: in case of error, errno must be kept valid!
 */
int
cnfSetLexFile(char *fname)
{
	es_str_t *str = NULL;
	struct bufstack *bs;
	FILE *fp;
	int r = 0;

	/* check for invalid recursive include */
	for(bs = currbs ; bs != NULL ; bs = bs->prev) {
		if(!strcmp(fname, bs->fn)) {
			parser_errmsg("trying to include file '%s', "
				"which is already included - ignored", fname);
			r = 1;
			goto done;
		}
	}

	if(fname == NULL) {
		fp = stdin;
	} else {
		if((fp = fopen(fname, "r")) == NULL) {
			r = 2;
			goto done;
		}
	}
	readConfFile(fp, &str);
	if(fp != stdin)
		fclose(fp);
	
	r = cnfAddConfigBuffer(str, ((fname == NULL) ? "stdin" : fname));

done:
	return r;
}


/* returns 0 on success, something else otherwise */
int
popfile(void)
{
	struct bufstack *bs = currbs;

	if(bs == NULL)
		return 1;
	
	/* delete current entry. But we must not free the file name if
	 * this is the top-level file, because then it may still be used
	 * in error messages for other processing steps.
	 * TODO: change this to another method which stores the file
	 * name inside the config objects. In the longer term, this is
	 * necessary, as otherwise we may provide wrong file name information
	 * at the end of include files as well. -- rgerhards, 2011-07-22
	 */
	dbgprintf("config parser: reached end of file %s\n", bs->fn);
	yy_delete_buffer(bs->bs);
	if(bs->prev != NULL)
		free(bs->fn);
	free(bs->estr);

	/* switch back to previous */
	currbs = bs->prev;
	free(bs);

	if(currbs == NULL) {
		dbgprintf("config parser: parsing completed\n");
		return 1; /* all processed */
	}
	
	yy_switch_to_buffer(currbs->bs);
	yylineno = currbs->lineno;
	cnfcurrfn = currbs->fn;
	dbgprintf("config parser: resume parsing of file %s at line %d\n",
		  cnfcurrfn, yylineno);
	return 0;
}

void
tellLexEndParsing(void)
{
	free(cnfcurrfn);
	cnfcurrfn= NULL;
}
#endif // #ifndef __clang_analyzer__
