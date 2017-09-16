/*
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if 0
static char sccsid[] = "@(#)rpc_scan.c 1.11 89/02/22 (C) 1987 SMI";
#endif

/*
 * rpc_scan.c, Scanner for the RPC protocol compiler 
 * Copyright (C) 1987, Sun Microsystems, Inc. 
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "rpc_scan.h"
#include "rpc_parse.h"
#include "rpc_util.h"

static void	unget_token(token *tokp);
static void	findstrconst(char **str, char **val);
static void	findchrconst(char **str, char **val);
static void	findconst(char **str, char **val);
static void	findkind(char **mark, token *tokp);
static int	cppline(char *line);
static int	directive(char *line);
static void	printdirective(char *line);
static void	docppline(char *line, int *lineno, char **fname);

#define startcomment(where) (where[0] == '/' && where[1] == '*')
#define endcomment(where) (where[-1] == '*' && where[0] == '/')

static int pushed = 0;	/* is a token pushed */
static token lasttok;	/* last token, if pushed */

/*
 * scan expecting 1 given token 
 */
void
scan(tok_kind expect, token *tokp)
{
	get_token(tokp);
	if (tokp->kind != expect) {
		expected1(expect);
	}
}

/*
 * scan expecting any of the 2 given tokens 
 */
void
scan2(tok_kind expect1, tok_kind expect2, token *tokp)
{
	get_token(tokp);
	if (tokp->kind != expect1 && tokp->kind != expect2) {
		expected2(expect1, expect2);
	}
}

/*
 * scan expecting any of the 3 given token 
 */
void
scan3(tok_kind expect1, tok_kind expect2, tok_kind expect3, token *tokp)
{
	get_token(tokp);
	if (tokp->kind != expect1 && tokp->kind != expect2
	    && tokp->kind != expect3) {
		expected3(expect1, expect2, expect3);
	}
}

/*
 * scan expecting a constant, possibly symbolic 
 */
void
scan_num(token *tokp)
{
	get_token(tokp);
	switch (tokp->kind) {
	case TOK_IDENT:
		break;
	default:
		error("constant or identifier expected");
	}
}

/*
 * Peek at the next token 
 */
void
peek(token *tokp)
{
	get_token(tokp);
	unget_token(tokp);
}

/*
 * Peek at the next token and scan it if it matches what you expect 
 */
int
peekscan(tok_kind expect, token *tokp)
{
	peek(tokp);
	if (tokp->kind == expect) {
		get_token(tokp);
		return (1);
	}
	return (0);
}

/*
 * Get the next token, printing out any directive that are encountered. 
 */
void
get_token(token *tokp)
{
	int commenting;

	if (pushed) {
		pushed = 0;
		*tokp = lasttok;
		return;
	}
	commenting = 0;
	for (;;) {
		if (*where == 0) {
			for (;;) {
				if (!fgets(curline, MAXLINESIZE, fin)) {
					tokp->kind = TOK_EOF;
					*where = 0;
					return;
				}
				linenum++;
				if (commenting) {
					break;
				} else if (cppline(curline)) {
					docppline(curline, &linenum, 
						  &infilename);
				} else if (directive(curline)) {
					printdirective(curline);
				} else {
					break;
				}
			}
			where = curline;
		} else if (isspace(*where)) {
			while (isspace(*where)) {
				where++;	/* eat */
			}
		} else if (commenting) {
			for (where++; *where; where++) {
				if (endcomment(where)) {
					where++;
					commenting--;
					break;
				}
			}
		} else if (startcomment(where)) {
			where += 2;
			commenting++;
		} else {
			break;
		}
	}

	/*
	 * 'where' is not whitespace, comment or directive Must be a token! 
	 */
	switch (*where) {
	case ':':
		tokp->kind = TOK_COLON;
		where++;
		break;
	case ';':
		tokp->kind = TOK_SEMICOLON;
		where++;
		break;
	case ',':
		tokp->kind = TOK_COMMA;
		where++;
		break;
	case '=':
		tokp->kind = TOK_EQUAL;
		where++;
		break;
	case '*':
		tokp->kind = TOK_STAR;
		where++;
		break;
	case '[':
		tokp->kind = TOK_LBRACKET;
		where++;
		break;
	case ']':
		tokp->kind = TOK_RBRACKET;
		where++;
		break;
	case '{':
		tokp->kind = TOK_LBRACE;
		where++;
		break;
	case '}':
		tokp->kind = TOK_RBRACE;
		where++;
		break;
	case '(':
		tokp->kind = TOK_LPAREN;
		where++;
		break;
	case ')':
		tokp->kind = TOK_RPAREN;
		where++;
		break;
	case '<':
		tokp->kind = TOK_LANGLE;
		where++;
		break;
	case '>':
		tokp->kind = TOK_RANGLE;
		where++;
		break;

	case '"':
		tokp->kind = TOK_STRCONST;
		findstrconst(&where, &tokp->str);
		break;
	case '\'':
		tokp->kind = TOK_CHARCONST;
		findchrconst(&where, &tokp->str);
		break;

	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		tokp->kind = TOK_IDENT;
		findconst(&where, &tokp->str);
		break;

	default:
		if (!(isalpha(*where) || *where == '_')) {
			char buf[100];
			char *p;

			s_print(buf, "illegal character in file: ");
			p = buf + strlen(buf);
			if (isprint(*where)) {
				s_print(p, "%c", *where);
			} else {
				s_print(p, "%d", *where);
			}
			error(buf);
		}
		findkind(&where, tokp);
		break;
	}
}

static void
unget_token(token *tokp)
{
	lasttok = *tokp;
	pushed = 1;
}

static void
findstrconst(char **str, char **val)
{
	char *p;
	int size;

	p = *str;
	do {
		*p++;
	} while (*p && *p != '"');
	if (*p == 0) {
		error("unterminated string constant");
	}
	p++;
	size = p - *str;
	*val = alloc(size + 1);
	(void) strncpy(*val, *str, size);
	(*val)[size] = 0;
	*str = p;
}

static void
findchrconst(char **str, char **val)
{
	char *p;
	int size;

	p = *str;
	do {
		*p++;
	} while (*p && *p != '\'');
	if (*p == 0) {
		error("unterminated string constant");
	}
	p++;
	size = p - *str;
	if (size != 3) {
		error("empty char string");
	}
	*val = alloc(size + 1);
	(void) strncpy(*val, *str, size);
	(*val)[size] = 0;
	*str = p;
}

static void
findconst(char **str, char **val)
{
	char *p;
	int size;

	p = *str;
	if (*p == '0' && *(p + 1) == 'x') {
		p++;
		do {
			p++;
		} while (isxdigit(*p));
	} else {
		do {
			p++;
		} while (isdigit(*p));
	}
	size = p - *str;
	*val = alloc(size + 1);
	(void) strncpy(*val, *str, size);
	(*val)[size] = 0;
	*str = p;
}

static token symbols[] = {
			  {TOK_CONST, "const"},
			  {TOK_UNION, "union"},
			  {TOK_SWITCH, "switch"},
			  {TOK_CASE, "case"},
			  {TOK_DEFAULT, "default"},
			  {TOK_STRUCT, "struct"},
			  {TOK_TYPEDEF, "typedef"},
			  {TOK_ENUM, "enum"},
			  {TOK_OPAQUE, "opaque"},
			  {TOK_BOOL, "bool"},
			  {TOK_VOID, "void"},
			  {TOK_CHAR, "char"},
			  {TOK_INT, "int"},
			  {TOK_UNSIGNED, "unsigned"},
			  {TOK_SHORT, "short"},
			  {TOK_INT32, "int32"},
			  {TOK_FLOAT, "float"},
			  {TOK_DOUBLE, "double"},
			  {TOK_STRING, "string"},
			  {TOK_PROGRAM, "program"},
			  {TOK_VERSION, "version"},
			  {TOK_EOF, "??????"},
};

static void
findkind(char **mark, token *tokp)
{
	int len;
	token *s;
	char *str;

	str = *mark;
	for (s = symbols; s->kind != TOK_EOF; s++) {
		len = strlen(s->str);
		if (strncmp(str, s->str, len) == 0) {
			if (!isalnum(str[len]) && str[len] != '_') {
				tokp->kind = s->kind;
				tokp->str = s->str;
				*mark = str + len;
				return;
			}
		}
	}
	tokp->kind = TOK_IDENT;
	for (len = 0; isalnum(str[len]) || str[len] == '_'; len++);
	tokp->str = alloc(len + 1);
	(void) strncpy(tokp->str, str, len);
	tokp->str[len] = 0;
	*mark = str + len;
}

static int
cppline(char *line)
{
	return (line == curline && *line == '#');
}

static int
directive(char *line)
{
	return (line == curline && *line == '%');
}

static void
printdirective(char *line)
{
	f_print(fout, "%s", line + 1);
}

static void
docppline(char *line, int *lineno, char **fname)
{
	char *file;
	int num;
	char *p;

	line++;
	while (isspace(*line)) {
		line++;
	}
	num = atoi(line);
	while (isdigit(*line)) {
		line++;
	}
	while (isspace(*line)) {
		line++;
	}
	if (*line != '"') {
		error("preprocessor error");
	}
	line++;
	p = file = alloc(strlen(line) + 1);
	while (*line && *line != '"') {
		*p++ = *line++;
	}
	if (*line == 0) {
		error("preprocessor error");
	}
	*p = 0;
	if (*file == 0) {
		*fname = NULL;
		free(file);
	} else {
		*fname = file;
	}
	*lineno = num - 1;
}
