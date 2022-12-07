%{
/*
 * Copyright (C) 2014-2018 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include <library.h>
#include <collections/array.h>
#include <settings/settings_types.h>
#include <utils/parser_helper.h>

#include "settings_parser.h"

#define YYDEBUG 1

/**
 * Defined by the lexer
 */
int settings_parser_lex(YYSTYPE *lvalp, void *scanner);
int settings_parser_lex_init_extra(parser_helper_t *extra, void *scanner);
int settings_parser_lex_destroy(void *scanner);
int settings_parser_set_in(FILE *in, void *scanner);
void settings_parser_set_debug(int debug, void *scanner);
char *settings_parser_get_text(void *scanner);
int settings_parser_get_leng(void *scanner);
int settings_parser_get_lineno(void *scanner);
/* Custom functions in lexer */
bool settings_parser_open_next_file(parser_helper_t *ctx);
bool settings_parser_load_string(parser_helper_t *ctx, const char *content);

/**
 * Forward declarations
 */
static void settings_parser_error(parser_helper_t *ctx, const char *s);
static section_t *push_section(parser_helper_t *ctx, char *name);
static section_t *pop_section(parser_helper_t *ctx);
static void add_section(parser_helper_t *ctx, section_t *section);
static void add_setting(parser_helper_t *ctx, kv_t *kv);
static void add_references(parser_helper_t *ctx, array_t *references);

/**
 * Make sure to call lexer with the proper context
 */
#undef yylex
static int yylex(SETTINGS_PARSER_STYPE *yylval, parser_helper_t *ctx)
{
	return settings_parser_lex(yylval, ctx->scanner);
}

%}
%debug

/* generate verbose error messages */
%define parse.error verbose
/* generate a reentrant parser */
%define api.pure
/* prefix function/variable declarations */
%define api.prefix {settings_parser_}
/* make sure flex uses the right definition */
%code provides
{
	#define YY_DECL \
		int settings_parser_lex(SETTINGS_PARSER_STYPE *yylval, void *yyscanner)
	YY_DECL;
}

/* interact properly with the reentrant lexer */
%lex-param {parser_helper_t *ctx}
%parse-param {parser_helper_t *ctx}

/* types for terminal symbols... (can't use the typedef'd types) */
%union {
	char *s;
	struct section_t *sec;
	struct kv_t *kv;
	array_t *refs;
}
%token <s> NAME STRING
%token DOT "."
%token COMMA ","
%token COLON ":"
%token NEWLINE STRING_ERROR

/* ...and other symbols */
%type <s> value valuepart
%type <sec> section_start section
%type <kv> setting
%type <refs> references

/* properly destroy string tokens that are strdup()ed on error */
%destructor { free($$); } NAME STRING value valuepart
/* properly destroy parse results on error */
%destructor { pop_section(ctx); settings_section_destroy($$, NULL); } section_start section
%destructor { settings_kv_destroy($$, NULL); } setting
%destructor { array_destroy_function($$, (void*)free, NULL); } references

/* there are two shift/reduce conflicts because of the "NAME = NAME" and
 * "NAME {" ambiguity, and the "NAME =" rule) */
%expect 2

%%

/**
 * strongswan.conf grammar rules
 */
statements:
	/* empty */
	| statements NEWLINE
	| statements statement
	;

statement:
	section
	{
		add_section(ctx, $section);
	}
	| setting
	{
		add_setting(ctx, $setting);
	}
	;

section:
	section_start statements '}'
	{
		pop_section(ctx);
		$$ = $section_start;
	}
	;

section_start:
	NAME '{'
	{
		$$ = push_section(ctx, $NAME);
	}
	|
	NAME ":" references '{'
	{
		$$ = push_section(ctx, $NAME);
		add_references(ctx, $references);
		array_destroy($references);
	}
	;

references:
	NAME
	{
		$$ = array_create(0, 0);
		array_insert($$, ARRAY_TAIL, $1);
	}
	| references "," NAME
	{
		array_insert($1, ARRAY_TAIL, $3);
		$$ = $1;
	}
	;

setting:
	NAME '=' value
	{
		$$ = settings_kv_create($NAME, $value);
	}
	|
	NAME '='
	{
		$$ = settings_kv_create($NAME, NULL);
	}
	;

value:
	valuepart
	| value valuepart
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

valuepart:
	NAME
	| STRING
	;

%%

/**
 * Referenced by the generated parser
 */
static void settings_parser_error(parser_helper_t *ctx, const char *s)
{
	PARSER_DBG1(ctx, "%s", s);
}

/**
 * Create a section and push it to the stack (the name is adopted), returns
 * the created section
 */
static section_t *push_section(parser_helper_t *ctx, char *name)
{
	array_t *sections = (array_t*)ctx->context;
	section_t *section;

	section = settings_section_create(name);
	array_insert(sections, ARRAY_TAIL, section);
	return section;
}

/**
 * Removes the top section of the stack and returns it
 */
static section_t *pop_section(parser_helper_t *ctx)
{
	array_t *sections = (array_t*)ctx->context;
	section_t *section;

	array_remove(sections, ARRAY_TAIL, &section);
	return section;
}

/**
 * Adds the given section to the section on top of the stack
 */
static void add_section(parser_helper_t *ctx, section_t *section)
{
	array_t *sections = (array_t*)ctx->context;
	section_t *parent;

	array_get(sections, ARRAY_TAIL, &parent);
	settings_section_add(parent, section, NULL);
}

/**
 * Adds the given key/value pair to the section on top of the stack
 */
static void add_setting(parser_helper_t *ctx, kv_t *kv)
{
	array_t *sections = (array_t*)ctx->context;
	section_t *section;

	array_get(sections, ARRAY_TAIL, &section);
	settings_kv_add(section, kv, NULL);
}

/**
 * Adds the given references to the section on top of the stack
 */
static void add_references(parser_helper_t *ctx, array_t *references)
{
	array_t *sections = (array_t*)ctx->context;
	section_t *section;
	enumerator_t *refs;
	char *ref;

	array_get(sections, ARRAY_TAIL, &section);

	refs = array_create_enumerator(references);
	while (refs->enumerate(refs, &ref))
	{
		settings_reference_add(section, ref, FALSE);
		array_remove_at(references, refs);
	}
	refs->destroy(refs);
}

/**
 * Parse the given file and add all sections and key/value pairs to the
 * given section.
 */
bool settings_parser_parse_file(section_t *root, char *name)
{
	parser_helper_t *helper;
	array_t *sections = NULL;
	bool success = FALSE;

	array_insert_create(&sections, ARRAY_TAIL, root);
	helper = parser_helper_create(sections);
	helper->get_lineno = settings_parser_get_lineno;
	if (settings_parser_lex_init_extra(helper, &helper->scanner) != 0)
	{
		helper->destroy(helper);
		array_destroy(sections);
		return FALSE;
	}
	helper->file_include(helper, name);
	if (!settings_parser_open_next_file(helper))
	{
		if (lib->conf && streq(name, lib->conf))
		{
			DBG2(DBG_CFG, "failed to open config file '%s'", name);
		}
		else
		{
			DBG1(DBG_CFG, "failed to open config file '%s'", name);
		}
	}
	else
	{
		if (getenv("DEBUG_SETTINGS_PARSER"))
		{
			yydebug = 1;
			settings_parser_set_debug(1, helper->scanner);
		}
		success = yyparse(helper) == 0;
		if (!success)
		{
			DBG1(DBG_CFG, "invalid config file '%s'", name);
		}
	}
	array_destroy(sections);
	settings_parser_lex_destroy(helper->scanner);
	helper->destroy(helper);
	return success;
}

/**
 * Parse the given string and add all sections and key/value pairs to the
 * given section.
 */
bool settings_parser_parse_string(section_t *root, char *settings)
{
	parser_helper_t *helper;
	array_t *sections = NULL;
	bool success = FALSE;

	array_insert_create(&sections, ARRAY_TAIL, root);
	helper = parser_helper_create(sections);
	helper->get_lineno = settings_parser_get_lineno;
	if (settings_parser_lex_init_extra(helper, &helper->scanner) != 0)
	{
		helper->destroy(helper);
		array_destroy(sections);
		return FALSE;
	}
	settings_parser_load_string(helper, settings);
	if (getenv("DEBUG_SETTINGS_PARSER"))
	{
		yydebug = 1;
		settings_parser_set_debug(1, helper->scanner);
	}
	success = yyparse(helper) == 0;
	if (!success)
	{
		DBG1(DBG_CFG, "failed to parse settings '%s'", settings);
	}
	array_destroy(sections);
	settings_parser_lex_destroy(helper->scanner);
	helper->destroy(helper);
	return success;
}
