%{

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "ssfilter.h"

typedef struct ssfilter * ssfilter_t;

#define YYSTYPE ssfilter_t

static struct ssfilter * alloc_node(int type, void *pred)
{
	struct ssfilter *n;

	if (!ssfilter_is_supported(type)) {
		fprintf(stderr, "It looks like such filter is not supported! Too old kernel?\n");
		exit(-1);
	}

	n = malloc(sizeof(*n));
	if (n == NULL)
		abort();
	n->type = type;
	n->pred = pred;
	n->post = NULL;
	return n;
}

static char		**yy_argv;
static int		yy_argc;
static FILE		*yy_fp;
static ssfilter_t	*yy_ret;
static int tok_type = -1;

static int yylex(void);

static void yyerror(char *s)
{
	fprintf(stderr, "ss: bison bellows (while parsing filter): \"%s!\"", s);
}

%}

%token HOSTCOND DCOND SCOND DPORT SPORT LEQ GEQ NEQ AUTOBOUND DEVCOND DEVNAME MARKMASK FWMARK CGROUPCOND CGROUPPATH
%left '|'
%left '&'
%nonassoc '!'

%%
applet: exprlist
        {
                *yy_ret = $1;
                $$ = $1;
        }
        | null
        ;

null:   /* NOTHING */ { $$ = NULL; }
        ;

exprlist: expr
        | exprlist '|' expr
        {
                $$ = alloc_node(SSF_OR, $1);
                $$->post = $3;
        }
        | exprlist '&' expr
        {
                $$ = alloc_node(SSF_AND, $1);
                $$->post = $3;
        }
        | exprlist expr
        {
                $$ = alloc_node(SSF_AND, $1);
                $$->post = $2;
        }
        ;

eq:	'='
	| /* nothing */
	;

expr:	'(' exprlist ')'
	{
		$$ = $2;
	}
	| '!' expr
	{
		$$ = alloc_node(SSF_NOT, $2);
	}
	| DCOND eq HOSTCOND
        {
		$$ = alloc_node(SSF_DCOND, $3);
        }
        | SCOND eq HOSTCOND
        {
		$$ = alloc_node(SSF_SCOND, $3);
        }
        | DPORT GEQ HOSTCOND
        {
                $$ = alloc_node(SSF_D_GE, $3);
        }
        | DPORT LEQ HOSTCOND
        {
                $$ = alloc_node(SSF_D_LE, $3);
        }
        | DPORT '>' HOSTCOND
        {
                $$ = alloc_node(SSF_NOT, alloc_node(SSF_D_LE, $3));
        }
        | DPORT '<' HOSTCOND
        {
                $$ = alloc_node(SSF_NOT, alloc_node(SSF_D_GE, $3));
        }
        | DPORT eq HOSTCOND
        {
		$$ = alloc_node(SSF_DCOND, $3);
        }
        | DPORT NEQ HOSTCOND
        {
		$$ = alloc_node(SSF_NOT, alloc_node(SSF_DCOND, $3));
        }

        | SPORT GEQ HOSTCOND
        {
                $$ = alloc_node(SSF_S_GE, $3);
        }
        | SPORT LEQ HOSTCOND
        {
                $$ = alloc_node(SSF_S_LE, $3);
        }
        | SPORT '>' HOSTCOND
        {
                $$ = alloc_node(SSF_NOT, alloc_node(SSF_S_LE, $3));
        }
        | SPORT '<' HOSTCOND
        {
                $$ = alloc_node(SSF_NOT, alloc_node(SSF_S_GE, $3));
        }
        | SPORT eq HOSTCOND
        {
		$$ = alloc_node(SSF_SCOND, $3);
        }
        | SPORT NEQ HOSTCOND
        {
		$$ = alloc_node(SSF_NOT, alloc_node(SSF_SCOND, $3));
        }
        | DEVNAME eq DEVCOND
        {
		$$ = alloc_node(SSF_DEVCOND, $3);
        }
        | DEVNAME NEQ DEVCOND
        {
		$$ = alloc_node(SSF_NOT, alloc_node(SSF_DEVCOND, $3));
        }
        | FWMARK eq MARKMASK
        {
                $$ = alloc_node(SSF_MARKMASK, $3);
        }
        | FWMARK NEQ MARKMASK
        {
                $$ = alloc_node(SSF_NOT, alloc_node(SSF_MARKMASK, $3));
        }
        | CGROUPPATH eq CGROUPCOND
        {
                $$ = alloc_node(SSF_CGROUPCOND, $3);
        }
        | CGROUPPATH NEQ CGROUPCOND
        {
                $$ = alloc_node(SSF_NOT, alloc_node(SSF_CGROUPCOND, $3));
        }
        | AUTOBOUND
        {
                $$ = alloc_node(SSF_S_AUTO, NULL);
        }
;
%%

static char *get_token_from_line(char **ptr)
{
	char *tok, *cp = *ptr;

	while (*cp == ' ' || *cp == '\t') cp++;

	if (*cp == 0) {
		*ptr = cp;
		return NULL;
	}

	tok = cp;

	while (*cp != 0 && *cp != ' ' && *cp != '\t') {
		/* Backslash escapes everything. */
		if (*cp == '\\') {
			char *tp;
			for (tp = cp; tp != tok; tp--)
				*tp = *(tp-1);
			cp++;
			tok++;
			if (*cp == 0)
				break;
		}
		cp++;
	}
	if (*cp)
		*cp++ = 0;
	*ptr = cp;
	return tok;
}

int yylex(void)
{
	static char argbuf[1024];
	static char *tokptr = argbuf;
	static int argc;
	char *curtok;

	do {
		while (*tokptr == 0) {
			tokptr = NULL;
			if (argc < yy_argc) {
				tokptr = yy_argv[argc];
				argc++;
			} else if (yy_fp) {
				while (tokptr == NULL) {
					size_t len;

					if (fgets(argbuf, sizeof(argbuf), yy_fp) == NULL)
						return 0;

					len = strnlen(argbuf, sizeof(argbuf));
					if (len == 0) {
						fprintf(stderr, "Invalid line\n");
						exit(-1);
					}

					if (len >= sizeof(argbuf) - 1) {
						fprintf(stderr, "Too long line in filter\n");
						exit(-1);
					}
					if (argbuf[len - 1] == '\n')
						argbuf[len-1] = 0;
					if (argbuf[0] == '#' || argbuf[0] == '0')
						continue;
					tokptr = argbuf;
				}
			} else {
				return 0;
			}
		}
	} while ((curtok = get_token_from_line(&tokptr)) == NULL);

	if (strcmp(curtok, "!") == 0 ||
	    strcmp(curtok, "not") == 0)
		return '!';
	if (strcmp(curtok, "&") == 0 ||
	    strcmp(curtok, "&&") == 0 ||
	    strcmp(curtok, "and") == 0)
		return '&';
	if (strcmp(curtok, "|") == 0 ||
	    strcmp(curtok, "||") == 0 ||
	    strcmp(curtok, "or") == 0)
		return '|';
	if (strcmp(curtok, "(") == 0)
		return '(';
	if (strcmp(curtok, ")") == 0)
		return ')';
	if (strcmp(curtok, "dst") == 0) {
		tok_type = DCOND;
		return DCOND;
	}
	if (strcmp(curtok, "src") == 0) {
                tok_type = SCOND;
		return SCOND;
        }
	if (strcmp(curtok, "dport") == 0) {
		tok_type = DPORT;
		return DPORT;
	}
	if (strcmp(curtok, "sport") == 0) {
		tok_type = SPORT;
		return SPORT;
	}
	if (strcmp(curtok, "dev") == 0) {
		tok_type = DEVNAME;
		return DEVNAME;
	}
	if (strcmp(curtok, "fwmark") == 0) {
		tok_type = FWMARK;
		return FWMARK;
	}
	if (strcmp(curtok, "cgroup") == 0) {
		tok_type = CGROUPPATH;
		return CGROUPPATH;
	}
	if (strcmp(curtok, ">=") == 0 ||
	    strcmp(curtok, "ge") == 0 ||
	    strcmp(curtok, "geq") == 0)
		return GEQ;
	if (strcmp(curtok, "<=") == 0 ||
	    strcmp(curtok, "le") == 0 ||
	    strcmp(curtok, "leq") == 0)
		return LEQ;
	if (strcmp(curtok, "!=") == 0 ||
	    strcmp(curtok, "ne") == 0 ||
	    strcmp(curtok, "neq") == 0)
		return NEQ;
	if (strcmp(curtok, "=") == 0 ||
	    strcmp(curtok, "==") == 0 ||
	    strcmp(curtok, "eq") == 0)
		return '=';
	if (strcmp(curtok, ">") == 0 ||
	    strcmp(curtok, "gt") == 0)
		return '>';
	if (strcmp(curtok, "<") == 0 ||
	    strcmp(curtok, "lt") == 0)
		return '<';
	if (strcmp(curtok, "autobound") == 0) {
		tok_type = AUTOBOUND;
		return AUTOBOUND;
	}
	if (tok_type == DEVNAME) {
		yylval = (void*)parse_devcond(curtok);
		if (yylval == NULL) {
			fprintf(stderr, "Cannot parse device.\n");
			exit(1);
		}
		return DEVCOND;
	}
	if (tok_type == FWMARK) {
		yylval = (void*)parse_markmask(curtok);
		if (yylval == NULL) {
			fprintf(stderr, "Cannot parse mark %s.\n", curtok);
			exit(1);
		}
		return MARKMASK;
	}
	if (tok_type == CGROUPPATH) {
		yylval = (void*)parse_cgroupcond(curtok);
		if (yylval == NULL) {
			fprintf(stderr, "Cannot parse cgroup %s.\n", curtok);
			exit(1);
		}
		return CGROUPCOND;
	}
	yylval = (void*)parse_hostcond(curtok, tok_type == SPORT || tok_type == DPORT);
	if (yylval == NULL) {
		fprintf(stderr, "Cannot parse dst/src address.\n");
		exit(1);
	}
	return HOSTCOND;
}

int ssfilter_parse(struct ssfilter **f, int argc, char **argv, FILE *fp)
{
	yy_argc = argc;
	yy_argv = argv;
	yy_fp   = fp;
	yy_ret  = f;

	if (yyparse()) {
		fprintf(stderr, " Sorry.\n");
		return -1;
	}
	return 0;
}
