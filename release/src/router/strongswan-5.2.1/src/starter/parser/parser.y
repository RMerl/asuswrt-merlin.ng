%{
/*
 * Copyright (C) 2013-2014 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#define _GNU_SOURCE /* for asprintf() */
#include <stdio.h>

#include <utils/parser_helper.h>
#include <settings/settings_types.h>
#include <parser/conf_parser.h>

#include "parser.h"

#define YYDEBUG 1

/**
 * Defined by the lexer
 */
int conf_parser_lex(YYSTYPE *lvalp, void *scanner);
int conf_parser_lex_init_extra(parser_helper_t *extra, void *scanner);
int conf_parser_lex_destroy(void *scanner);
int conf_parser_set_in(FILE *in, void *scanner);
void conf_parser_set_debug(int debug, void *scanner);
char *conf_parser_get_text(void *scanner);
int conf_parser_get_leng(void *scanner);
int conf_parser_get_lineno(void *scanner);
/* Custom functions in lexer */
bool conf_parser_open_next_file(parser_helper_t *ctx);

/**
 * Forward declaration
 */
static void conf_parser_error(parser_helper_t *ctx, const char *s);

/**
 * Make sure to call lexer with the proper context
 */
#undef yylex
static int yylex(YYSTYPE *lvalp, parser_helper_t *ctx)
{
	return conf_parser_lex(lvalp, ctx->scanner);
}

%}
%debug

/* generate verbose error messages */
%error-verbose
/* generate a reentrant parser */
%define api.pure
/* prefix function/variable declarations */
%name-prefix "conf_parser_"

/* interact properly with the reentrant lexer */
%lex-param {parser_helper_t *ctx}
%parse-param {parser_helper_t *ctx}

/* types for terminal symbols... */
%union {
	char *s;
	conf_parser_section_t t;
}
%token <s> STRING
%token EQ SPACES NEWLINE CONFIG_SETUP CONN CA

/* ...and other symbols */
%type <t> section_type
%type <s> section_name value

/* make the equal sign left associative */
%left EQ

/* properly destroy STRING tokens, which are strdup()ed, on errors */
%destructor { free($$); } STRING section_name value

/* there are two shift/reduce conflicts because we allow empty lines (and lines
 * with spaces) within settings and anywhere else (i.e. in the beginning) */
//%expect 2

%%

/**
 * ipsec.conf grammar rules
 */
statements:
	/* empty */
	| statements NEWLINE
	| statements statement
	;

statement:
	section
	| SPACES setting
	;

section:
	section_type section_name
	{
		if ($1 != CONF_PARSER_CONFIG_SETUP && (!$2 || !strlen($2)))
		{
			PARSER_DBG1(ctx, "section name missing");
			free($2);
			YYERROR;
		}
		conf_parser_t *parser = (conf_parser_t*)ctx->context;
		parser->add_section(parser, $1, $2);
	}
	;

section_type:
	CONFIG_SETUP
	{
		$$ = CONF_PARSER_CONFIG_SETUP;
	}
	|
	CONN
	{
		$$ = CONF_PARSER_CONN;
	}
	|
	CA
	{
		$$ = CONF_PARSER_CA;
	}
	;

section_name:
	/* empty */
	{
		$$ = NULL;
	}
	| STRING
	{
		$$ = $1;
	}
	;

setting:
	/* empty */
	|
	STRING EQ value
	{
		if (!strlen($1))
		{
			PARSER_DBG1(ctx, "setting name can't be empty");
			free($1);
			free($3);
			YYERROR;
		}
		conf_parser_t *parser = (conf_parser_t*)ctx->context;
		parser->add_setting(parser, $1, $value);
	}
	|
	STRING EQ
	{
		if (!strlen($1))
		{
			PARSER_DBG1(ctx, "setting name can't be empty");
			free($1);
			YYERROR;
		}
		conf_parser_t *parser = (conf_parser_t*)ctx->context;
		parser->add_setting(parser, $1, NULL);
	}
	|
	STRING
	{
		PARSER_DBG1(ctx, "missing value for setting '%s'", $1);
		free($1);
		YYERROR;
	}
	;

value:
	STRING
	| value STRING
	{	/* just put a single space between them, use strings for more */
		if (asprintf(&$$, "%s %s", $1, $2) < 0)
		{
			free($1);
			free($2);
			YYERROR;
		}
		free($1);
		free($2);
	}
	;

%%

/**
 * Referenced by the generated parser
 */
static void conf_parser_error(parser_helper_t *ctx, const char *s)
{
	char *text = conf_parser_get_text(ctx->scanner);
	int len = conf_parser_get_leng(ctx->scanner);

	if (len && text[len-1] == '\n')
	{	/* cut off newline at the end to avoid muti-line log messages */
		len--;
	}
	PARSER_DBG1(ctx, "%s [%.*s]", s, (int)len, text);
}

/**
 * Parse the given file
 */
bool conf_parser_parse_file(conf_parser_t *this, char *name)
{
	parser_helper_t *helper;
	bool success = FALSE;

	helper = parser_helper_create(this);
	helper->get_lineno = conf_parser_get_lineno;
	if (conf_parser_lex_init_extra(helper, &helper->scanner) != 0)
	{
		helper->destroy(helper);
		return FALSE;
	}
	helper->file_include(helper, name);
	if (!conf_parser_open_next_file(helper))
	{
		DBG1(DBG_CFG, "failed to open config file '%s'", name);
	}
	else
	{
		if (getenv("DEBUG_CONF_PARSER"))
		{
			yydebug = 1;
			conf_parser_set_debug(1, helper->scanner);
		}
		success = yyparse(helper) == 0;
		if (!success)
		{
			DBG1(DBG_CFG, "invalid config file '%s'", name);
		}
	}
	conf_parser_lex_destroy(helper->scanner);
	helper->destroy(helper);
	return success;
}
