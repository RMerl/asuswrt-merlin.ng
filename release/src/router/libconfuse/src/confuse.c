/*
 * Copyright (c) 2002-2017  Martin Hedenfalk <martin@bzero.se>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifndef _WIN32
# include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <ctype.h>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
# ifndef S_ISREG
#  define S_ISREG(mode) ((mode) & S_IFREG)
# endif
#endif

#include "compat.h"
#include "confuse.h"

#define is_set(f, x) (((f) & (x)) == (f))

#if defined(ENABLE_NLS) && defined(HAVE_GETTEXT)
# include <locale.h>
# include <libintl.h>
# define _(str) dgettext(PACKAGE, str)
#else
# define _(str) str
#endif
#define N_(str) str

const char confuse_version[] = PACKAGE_VERSION;
const char confuse_copyright[] = PACKAGE_STRING " by Martin Hedenfalk <martin@bzero.se>";
const char confuse_author[] = "Martin Hedenfalk <martin@bzero.se>";

char *cfg_yylval = 0;

extern int  cfg_yylex(cfg_t *cfg);
extern void cfg_yylex_destroy(void);
extern int  cfg_lexer_include(cfg_t *cfg, const char *fname);
extern void cfg_scan_fp_begin(FILE *fp);
extern void cfg_scan_fp_end(void);

static int cfg_parse_internal(cfg_t *cfg, int level, int force_state, cfg_opt_t *force_opt);
static void cfg_free_opt_array(cfg_opt_t *opts);

#define STATE_CONTINUE 0
#define STATE_EOF -1
#define STATE_ERROR 1

#ifndef HAVE_FMEMOPEN
extern FILE *fmemopen(void *buf, size_t size, const char *type);
#endif

#ifndef HAVE_STRDUP
/*
 * Copyright (c) 1988, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
static char *strdup(const char *s)
{
	size_t siz;
	char *copy;

	siz = strlen(str) + 1;
	copy = malloc(siz);
	if (!copy)
		return NULL;

	memcpy(copy, str, siz);

	return copy;
}
#endif

#ifndef HAVE_STRNDUP
static char *strndup(const char *s, size_t n)
{
	char *r;

	r = malloc(n + 1);
	if (!r)
		return NULL;

	strncpy(r, s, n);
	r[n] = 0;

	return r;
}
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2)
{
	assert(s1);
	assert(s2);

	while (*s1) {
		int c1 = tolower(*(const unsigned char *)s1);
		int c2 = tolower(*(const unsigned char *)s2);

		if (c1 < c2)
			return -1;
		if (c1 > c2)
			return +1;

		++s1;
		++s2;
	}

	if (*s2 != 0)
		return -1;

	return 0;
}
#endif

DLLIMPORT cfg_opt_t *cfg_getopt(cfg_t *cfg, const char *name)
{
	unsigned int i;
	cfg_t *sec = cfg;

	if (!cfg || !cfg->name || !name) {
		errno = EINVAL;
		return NULL;
	}

	while (name && *name) {
		char *secname;
		size_t len = strcspn(name, "|");

		if (name[len] == 0 /*len == strlen(name) */ )
			/* no more subsections */
			break;

		if (len) {
			secname = strndup(name, len);
			if (!secname)
				return NULL;

			sec = cfg_getsec(sec, secname);
			if (!sec) {
				if (!is_set(CFGF_IGNORE_UNKNOWN, cfg->flags))
					cfg_error(cfg, _("no such option '%s'"), secname);
				free(secname);
				return NULL;
			}
			free(secname);
		}
		name += len;
		name += strspn(name, "|");
	}

	for (i = 0; sec->opts[i].name; i++) {
		if (is_set(CFGF_NOCASE, sec->flags)) {
			if (strcasecmp(sec->opts[i].name, name) == 0)
				return &sec->opts[i];
		} else {
			if (strcmp(sec->opts[i].name, name) == 0)
				return &sec->opts[i];
		}
	}

	if (!is_set(CFGF_IGNORE_UNKNOWN, cfg->flags))
		cfg_error(cfg, _("no such option '%s'"), name);

	return NULL;
}

DLLIMPORT const char *cfg_title(cfg_t *cfg)
{
	if (cfg)
		return cfg->title;
	return NULL;
}

DLLIMPORT const char *cfg_name(cfg_t *cfg)
{
	if (cfg)
		return cfg->name;
	return NULL;
}

DLLIMPORT const char *cfg_opt_name(cfg_opt_t *opt)
{
	if (opt)
		return opt->name;
	return NULL;
}

DLLIMPORT unsigned int cfg_opt_size(cfg_opt_t *opt)
{
	if (opt)
		return opt->nvalues;
	return 0;
}

DLLIMPORT unsigned int cfg_size(cfg_t *cfg, const char *name)
{
	return cfg_opt_size(cfg_getopt(cfg, name));
}

DLLIMPORT char *cfg_opt_getcomment(cfg_opt_t *opt)
{
	if (opt)
		return opt->comment;

	return NULL;
}

DLLIMPORT char *cfg_getcomment(cfg_t *cfg, const char *name)
{
	return cfg_opt_getcomment(cfg_getopt(cfg, name));
}

DLLIMPORT signed long cfg_opt_getnint(cfg_opt_t *opt, unsigned int index)
{
	if (!opt || opt->type != CFGT_INT) {
		errno = EINVAL;
		return 0;
	}

	if (opt->values && index < opt->nvalues)
		return opt->values[index]->number;
	if (opt->simple_value.number)
		return *opt->simple_value.number;

	return 0;
}

DLLIMPORT signed long cfg_getnint(cfg_t *cfg, const char *name, unsigned int index)
{
	return cfg_opt_getnint(cfg_getopt(cfg, name), index);
}

DLLIMPORT signed long cfg_getint(cfg_t *cfg, const char *name)
{
	return cfg_getnint(cfg, name, 0);
}

DLLIMPORT double cfg_opt_getnfloat(cfg_opt_t *opt, unsigned int index)
{
	if (!opt || opt->type != CFGT_FLOAT) {
		errno = EINVAL;
		return 0;
	}

	if (opt->values && index < opt->nvalues)
		return opt->values[index]->fpnumber;
	if (opt->simple_value.fpnumber)
		return *opt->simple_value.fpnumber;

	return 0;
}

DLLIMPORT double cfg_getnfloat(cfg_t *cfg, const char *name, unsigned int index)
{
	return cfg_opt_getnfloat(cfg_getopt(cfg, name), index);
}

DLLIMPORT double cfg_getfloat(cfg_t *cfg, const char *name)
{
	return cfg_getnfloat(cfg, name, 0);
}

DLLIMPORT cfg_bool_t cfg_opt_getnbool(cfg_opt_t *opt, unsigned int index)
{
	if (!opt || opt->type != CFGT_BOOL) {
		errno = EINVAL;
		return cfg_false;
	}

	if (opt->values && index < opt->nvalues)
		return opt->values[index]->boolean;
	if (opt->simple_value.boolean)
		return *opt->simple_value.boolean;

	return cfg_false;
}

DLLIMPORT cfg_bool_t cfg_getnbool(cfg_t *cfg, const char *name, unsigned int index)
{
	return cfg_opt_getnbool(cfg_getopt(cfg, name), index);
}

DLLIMPORT cfg_bool_t cfg_getbool(cfg_t *cfg, const char *name)
{
	return cfg_getnbool(cfg, name, 0);
}

DLLIMPORT char *cfg_opt_getnstr(cfg_opt_t *opt, unsigned int index)
{
	if (!opt || opt->type != CFGT_STR) {
		errno = EINVAL;
		return NULL;
	}

	if (opt->values && index < opt->nvalues)
		return opt->values[index]->string;
	if (opt->simple_value.string)
		return *opt->simple_value.string;

	return NULL;
}

DLLIMPORT char *cfg_getnstr(cfg_t *cfg, const char *name, unsigned int index)
{
	return cfg_opt_getnstr(cfg_getopt(cfg, name), index);
}

DLLIMPORT char *cfg_getstr(cfg_t *cfg, const char *name)
{
	return cfg_getnstr(cfg, name, 0);
}

DLLIMPORT void *cfg_opt_getnptr(cfg_opt_t *opt, unsigned int index)
{
	if (!opt || opt->type != CFGT_PTR) {
		errno = EINVAL;
		return NULL;
	}

	if (opt->values && index < opt->nvalues)
		return opt->values[index]->ptr;
	if (opt->simple_value.ptr)
		return *opt->simple_value.ptr;

	return NULL;
}

DLLIMPORT void *cfg_getnptr(cfg_t *cfg, const char *name, unsigned int index)
{
	return cfg_opt_getnptr(cfg_getopt(cfg, name), index);
}

DLLIMPORT void *cfg_getptr(cfg_t *cfg, const char *name)
{
	return cfg_getnptr(cfg, name, 0);
}

DLLIMPORT cfg_t *cfg_opt_getnsec(cfg_opt_t *opt, unsigned int index)
{
	if (!opt || opt->type != CFGT_SEC) {
		errno = EINVAL;
		return NULL;
	}

	if (opt->values && index < opt->nvalues)
		return opt->values[index]->section;

	errno = ENOENT;
	return NULL;
}

DLLIMPORT cfg_t *cfg_getnsec(cfg_t *cfg, const char *name, unsigned int index)
{
	return cfg_opt_getnsec(cfg_getopt(cfg, name), index);
}

DLLIMPORT cfg_t *cfg_opt_gettsec(cfg_opt_t *opt, const char *title)
{
	unsigned int i, n;

	if (!opt || !title) {
		errno = EINVAL;
		return NULL;
	}

	if (!is_set(CFGF_TITLE, opt->flags)) {
		errno = EINVAL;
		return NULL;
	}

	n = cfg_opt_size(opt);
	for (i = 0; i < n; i++) {
		cfg_t *sec = cfg_opt_getnsec(opt, i);

		if (!sec || !sec->title)
			return NULL;

		if (is_set(CFGF_NOCASE, opt->flags)) {
			if (strcasecmp(title, sec->title) == 0)
				return sec;
		} else {
			if (strcmp(title, sec->title) == 0)
				return sec;
		}
	}

	errno = ENOENT;
	return NULL;
}

DLLIMPORT cfg_t *cfg_gettsec(cfg_t *cfg, const char *name, const char *title)
{
	return cfg_opt_gettsec(cfg_getopt(cfg, name), title);
}

DLLIMPORT cfg_t *cfg_getsec(cfg_t *cfg, const char *name)
{
	return cfg_getnsec(cfg, name, 0);
}

static cfg_value_t *cfg_addval(cfg_opt_t *opt)
{
	void *ptr;

	ptr = realloc(opt->values, (opt->nvalues + 1) * sizeof(cfg_value_t *));
	if (!ptr)
		return NULL;

	opt->values = ptr;
	opt->values[opt->nvalues] = calloc(1, sizeof(cfg_value_t));
	if (!opt->values[opt->nvalues])
		return NULL;

	return opt->values[opt->nvalues++];
}

DLLIMPORT int cfg_numopts(cfg_opt_t *opts)
{
	int n;

	for (n = 0; opts[n].name; n++)
		/* do nothing */ ;
	return n;
}

static cfg_opt_t *cfg_dupopt_array(cfg_opt_t *opts)
{
	int i;
	cfg_opt_t *dupopts;
	int n = cfg_numopts(opts);

	dupopts = calloc(n + 1, sizeof(cfg_opt_t));
	if (!dupopts)
		return NULL;

	memcpy(dupopts, opts, n * sizeof(cfg_opt_t));

	for (i = 0; i < n; i++) {
		dupopts[i].name = strdup(opts[i].name);
		if (!dupopts[i].name)
			goto err;

		if (opts[i].type == CFGT_SEC && opts[i].subopts) {
			dupopts[i].subopts = cfg_dupopt_array(opts[i].subopts);
			if (!dupopts[i].subopts)
				goto err;
		}

		if (is_set(CFGF_LIST, opts[i].flags) || opts[i].type == CFGT_FUNC) {
			dupopts[i].def.parsed = opts[i].def.parsed ? strdup(opts[i].def.parsed) : NULL;
			if (opts[i].def.parsed && !dupopts[i].def.parsed)
				goto err;
		}
		else if (opts[i].type == CFGT_STR) {
			dupopts[i].def.string = opts[i].def.string ? strdup(opts[i].def.string) : NULL;
			if (opts[i].def.string && !dupopts[i].def.string)
				goto err;
		}
	}

	return dupopts;
err:
	cfg_free_opt_array(dupopts);
	return NULL;
}

DLLIMPORT int cfg_parse_boolean(const char *s)
{
	if (!s) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	if (strcasecmp(s, "true") == 0 || strcasecmp(s, "on") == 0 || strcasecmp(s, "yes") == 0)
		return 1;
	if (strcasecmp(s, "false") == 0 || strcasecmp(s, "off") == 0 || strcasecmp(s, "no") == 0)
		return 0;

	return CFG_FAIL;
}

static void cfg_init_defaults(cfg_t *cfg)
{
	int i;

	for (i = 0; cfg->opts[i].name; i++) {
		int j;

		for (j = 0; j < i; ++j) {
			if (is_set(CFGF_NOCASE, cfg->opts[i].flags | cfg->opts[j].flags)) {
				if (strcasecmp(cfg->opts[i].name, cfg->opts[j].name))
					continue;
			} else {
				if (strcmp(cfg->opts[i].name, cfg->opts[j].name))
					continue;
			}
			/*
			 * There are two definitions of the same option name.
			 * What to do? It's a programming error and not an end
			 * user input error. Lets print a message and abort...
			 */
			cfg_error(cfg, _("duplicate option '%s' not allowed"),
				cfg->opts[i].name);
			break;
		}

		/* libConfuse doesn't handle default values for "simple" options */
		if (cfg->opts[i].simple_value.ptr || is_set(CFGF_NODEFAULT, cfg->opts[i].flags))
			continue;

		if (cfg->opts[i].type != CFGT_SEC) {
			cfg->opts[i].flags |= CFGF_DEFINIT;

			if (is_set(CFGF_LIST, cfg->opts[i].flags) || cfg->opts[i].def.parsed) {
				int xstate, ret = 0;
				char *buf;
				FILE *fp;

				/* If it's a list, but no default value was given,
				 * keep the option uninitialized.
				 */
				buf = cfg->opts[i].def.parsed;
				if (!buf || !buf[0])
					continue;

				/* setup scanning from the string specified for the
				 * "default" value, force the correct state and option
				 */

				if (is_set(CFGF_LIST, cfg->opts[i].flags))
					/* lists must be surrounded by {braces} */
					xstate = 3;
				else if (cfg->opts[i].type == CFGT_FUNC)
					xstate = 0;
				else
					xstate = 2;

				fp = fmemopen(buf, strlen(buf), "r");
				if (!fp) {
					/*
					 * fmemopen() on older GLIBC versions do not accept zero
					 * length buffers for some reason.  This is a workaround.
					 */
					if (strlen(buf) > 0)
						ret = STATE_ERROR;
				} else {
					cfg_scan_fp_begin(fp);

					do {
						ret = cfg_parse_internal(cfg, 1, xstate, &cfg->opts[i]);
						xstate = -1;
					} while (ret == STATE_CONTINUE);

					cfg_scan_fp_end();
					fclose(fp);
				}

				if (ret == STATE_ERROR) {
					/*
					 * If there was an error parsing the default string,
					 * the initialization of the default value could be
					 * inconsistent or empty. What to do? It's a
					 * programming error and not an end user input
					 * error. Lets print a message and abort...
					 */
					fprintf(stderr, "Parse error in default value '%s'"
						" for option '%s'\n", cfg->opts[i].def.parsed, cfg->opts[i].name);
					fprintf(stderr, "Check your initialization macros and the" " libConfuse documentation\n");
					abort();
				}
			} else {
				switch (cfg->opts[i].type) {
				case CFGT_INT:
					cfg_opt_setnint(&cfg->opts[i], cfg->opts[i].def.number, 0);
					break;

				case CFGT_FLOAT:
					cfg_opt_setnfloat(&cfg->opts[i], cfg->opts[i].def.fpnumber, 0);
					break;

				case CFGT_BOOL:
					cfg_opt_setnbool(&cfg->opts[i], cfg->opts[i].def.boolean, 0);
					break;

				case CFGT_STR:
					cfg_opt_setnstr(&cfg->opts[i], cfg->opts[i].def.string, 0);
					break;

				case CFGT_FUNC:
				case CFGT_PTR:
					break;

				default:
					cfg_error(cfg, "internal error in cfg_init_defaults(%s)", cfg->opts[i].name);
					break;
				}
			}

			/* The default value should only be returned if no value
			 * is given in the configuration file, so we set the RESET
			 * flag here. When/If cfg_setopt() is called, the value(s)
			 * will be freed and the flag unset.
			 */
			cfg->opts[i].flags |= CFGF_RESET;
		} else if (!is_set(CFGF_MULTI, cfg->opts[i].flags)) {
			cfg_setopt(cfg, &cfg->opts[i], 0);
			cfg->opts[i].flags |= CFGF_DEFINIT;
		}
	}
}

DLLIMPORT cfg_value_t *cfg_setopt(cfg_t *cfg, cfg_opt_t *opt, const char *value)
{
	cfg_value_t *val = NULL;
	int b;
	const char *s;
	double f;
	long int i;
	void *p;
	char *endptr;

	if (!cfg || !opt) {
		errno = EINVAL;
		return NULL;
	}

	if (opt->simple_value.ptr) {
		if (opt->type == CFGT_SEC) {
			errno = EINVAL;
			return NULL;
		}
		val = (cfg_value_t *)opt->simple_value.ptr;
	} else {
		if (is_set(CFGF_RESET, opt->flags)) {
			cfg_free_value(opt);
			opt->flags &= ~CFGF_RESET;
		}

		if (opt->nvalues == 0 || is_set(CFGF_MULTI, opt->flags) || is_set(CFGF_LIST, opt->flags)) {
			val = NULL;

			if (opt->type == CFGT_SEC && is_set(CFGF_TITLE, opt->flags)) {
				unsigned int i;

				/* XXX: Check if there already is a section with the same title. */

				/*
				 * Check there are either no sections at
				 * all, or a non-NULL section title.
				 */
				if (opt->nvalues != 0 && !value) {
					errno = EINVAL;
					return NULL;
				}

				for (i = 0; i < opt->nvalues && val == NULL; i++) {
					cfg_t *sec = opt->values[i]->section;

					if (is_set(CFGF_NOCASE, cfg->flags)) {
						if (strcasecmp(value, sec->title) == 0)
							val = opt->values[i];
					} else {
						if (strcmp(value, sec->title) == 0)
							val = opt->values[i];
					}
				}

				if (val && is_set(CFGF_NO_TITLE_DUPES, opt->flags)) {
					cfg_error(cfg, _("found duplicate title '%s'"), value);
					return NULL;
				}
			}

			if (!val) {
				val = cfg_addval(opt);
				if (!val)
					return NULL;
			}
		} else {
			val = opt->values[0];
		}
	}

	switch (opt->type) {
	case CFGT_INT:
		if (opt->parsecb) {
			if ((*opt->parsecb) (cfg, opt, value, &i) != 0)
				return NULL;
		} else {
			if (!value) {
				errno = EINVAL;
				return NULL;
			}
			i = strtol(value, &endptr, 0);
			if (*endptr != '\0') {
				cfg_error(cfg, _("invalid integer value for option '%s'"), opt->name);
				return NULL;
			}
			if (errno == ERANGE) {
				cfg_error(cfg, _("integer value for option '%s' is out of range"), opt->name);
				return NULL;
			}
		}
		val->number = i;
		break;

	case CFGT_FLOAT:
		if (opt->parsecb) {
			if ((*opt->parsecb) (cfg, opt, value, &f) != 0)
				return NULL;
		} else {
			if (!value) {
				errno = EINVAL;
				return NULL;
			}
			f = strtod(value, &endptr);
			if (*endptr != '\0') {
				cfg_error(cfg, _("invalid floating point value for option '%s'"), opt->name);
				return NULL;
			}
			if (errno == ERANGE) {
				cfg_error(cfg, _("floating point value for option '%s' is out of range"), opt->name);
				return NULL;
			}
		}
		val->fpnumber = f;
		break;

	case CFGT_STR:
		if (opt->parsecb) {
			s = NULL;
			if ((*opt->parsecb) (cfg, opt, value, &s) != 0)
				return NULL;
		} else {
			s = value;
		}

		if (!s) {
			errno = EINVAL;
			return NULL;
		}

		free(val->string);
		val->string = strdup(s);
		if (!val->string)
			return NULL;
		break;

	case CFGT_SEC:
		if (is_set(CFGF_MULTI, opt->flags) || val->section == 0) {
			if (val->section) {
				val->section->path = NULL; /* Global search path */
				cfg_free(val->section);
			}
			val->section = calloc(1, sizeof(cfg_t));
			if (!val->section)
				return NULL;

			val->section->name = strdup(opt->name);
			if (!val->section->name) {
				free(val->section);
				return NULL;
			}

			val->section->flags = cfg->flags;
			val->section->filename = cfg->filename ? strdup(cfg->filename) : NULL;
			if (cfg->filename && !val->section->filename) {
				free(val->section->name);
				free(val->section);
				return NULL;
			}

			val->section->line = cfg->line;
			val->section->errfunc = cfg->errfunc;
			val->section->title = value ? strdup(value) : NULL;
			if (value && !val->section->title) {
				free(val->section->filename);
				free(val->section->name);
				free(val->section);
				return NULL;
			}

			val->section->opts = cfg_dupopt_array(opt->subopts);
			if (!val->section->opts) {
				if (val->section->title)
					free(val->section->title);
				if (val->section->filename)
					free(val->section->filename);
				free(val->section->name);
				free(val->section);
				return NULL;
			}
		}
		if (!is_set(CFGF_DEFINIT, opt->flags))
			cfg_init_defaults(val->section);
		break;

	case CFGT_BOOL:
		if (opt->parsecb) {
			if ((*opt->parsecb) (cfg, opt, value, &b) != 0)
				return NULL;
		} else {
			b = cfg_parse_boolean(value);
			if (b == -1) {
				cfg_error(cfg, _("invalid boolean value for option '%s'"), opt->name);
				return NULL;
			}
		}
		val->boolean = (cfg_bool_t)b;
		break;

	case CFGT_PTR:
		if (!opt->parsecb) {
			errno = EINVAL;
			return NULL;
		}

		if ((*opt->parsecb) (cfg, opt, value, &p) != 0)
			return NULL;
		val->ptr = p;
		break;

	default:
		cfg_error(cfg, "internal error in cfg_setopt(%s, %s)", opt->name, (value) ? (value) : "NULL");
		return NULL;
	}

	return val;
}

DLLIMPORT int cfg_opt_setmulti(cfg_t *cfg, cfg_opt_t *opt, unsigned int nvalues, char **values)
{
	cfg_opt_t old;
	unsigned int i;

	if (!opt || !nvalues) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	old = *opt;
	opt->nvalues = 0;
	opt->values = 0;

	for (i = 0; i < nvalues; i++) {
		if (cfg_setopt(cfg, opt, values[i]))
			continue;

		/* ouch, revert */
		cfg_free_value(opt);
		opt->nvalues = old.nvalues;
		opt->values = old.values;

		return CFG_FAIL;
	}

	cfg_free_value(&old);

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_setmulti(cfg_t *cfg, const char *name, unsigned int nvalues, char **values)
{
	cfg_opt_t *opt;

	if (!cfg || !name || !values) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	opt = cfg_getopt(cfg, name);
	if (!opt) {
		errno = ENOENT;
		return CFG_FAIL;
	}

	return cfg_opt_setmulti(cfg, opt, nvalues, values);
}

/* searchpath */

struct cfg_searchpath_t {
	char *dir;	        /**< directory to search */
	cfg_searchpath_t *next; /**< next in list */
};

/* prepend a new cfg_searchpath_t to the linked list */

DLLIMPORT int cfg_add_searchpath(cfg_t *cfg, const char *dir)
{
	cfg_searchpath_t *p;
	char *d;

	if (!cfg || !dir) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	d = cfg_tilde_expand(dir);
	if (!d)
		return CFG_FAIL;

	p = malloc(sizeof(cfg_searchpath_t));
	if (!p) {
		free(d);
		return CFG_FAIL;
	}

	p->next   = cfg->path;
	p->dir    = d;
	cfg->path = p;

	return CFG_SUCCESS;
}

DLLIMPORT cfg_errfunc_t cfg_set_error_function(cfg_t *cfg, cfg_errfunc_t errfunc)
{
	cfg_errfunc_t old;

	if (!cfg) {
		errno = EINVAL;
		return NULL;
	}

	old = cfg->errfunc;
	cfg->errfunc = errfunc;

	return old;
}

DLLIMPORT void cfg_error(cfg_t *cfg, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	if (cfg && cfg->errfunc)
		(*cfg->errfunc) (cfg, fmt, ap);
	else {
		if (cfg && cfg->filename && cfg->line)
			fprintf(stderr, "%s:%d: ", cfg->filename, cfg->line);
		else if (cfg && cfg->filename)
			fprintf(stderr, "%s: ", cfg->filename);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
	}

	va_end(ap);
}

static int call_function(cfg_t *cfg, cfg_opt_t *opt, cfg_opt_t *funcopt)
{
	int ret;
	const char **argv;
	unsigned int i;

	if (!cfg || !opt ||!funcopt) {
		errno = EINVAL;
		return CFG_FAIL;
	}
		
	/*
	 * create am argv string vector and call the registered function
	 */
	argv = calloc(funcopt->nvalues, sizeof(char *));
	if (!argv)
		return CFG_FAIL;

	for (i = 0; i < funcopt->nvalues; i++)
		argv[i] = funcopt->values[i]->string;

	ret = (*opt->func) (cfg, opt, funcopt->nvalues, argv);
	cfg_free_value(funcopt);
	free(argv);

	return ret;
}

static void cfg_handle_deprecated(cfg_t *cfg, cfg_opt_t *opt)
{
	if (is_set(CFGF_DROP, opt->flags)) {
		cfg_error(cfg, _("dropping deprecated configuration option '%s'"), opt->name);
		cfg_free_value(opt);
	} else {
		cfg_error(cfg, _("found deprecated option '%s', please update configuration file."), opt->name);
	}
}

static int cfg_parse_internal(cfg_t *cfg, int level, int force_state, cfg_opt_t *force_opt)
{
	int state = 0;
	char *comment = NULL;
	char *opttitle = NULL;
	cfg_opt_t *opt = NULL;
	cfg_value_t *val = NULL;
	cfg_opt_t funcopt = CFG_STR(0, 0, 0);
	int ignore = 0;		/* ignore until this token, traverse parser w/o error */
	int num_values = 0;	/* number of values found for a list option */
	int rc;

	if (force_state != -1)
		state = force_state;
	if (force_opt)
		opt = force_opt;

	while (1) {
		int tok = cfg_yylex(cfg);

		if (tok == 0) {
			/* lexer.l should have called cfg_error() */
			goto error;
		}

		if (tok == EOF) {
			if (state != 0) {
				cfg_error(cfg, _("premature end of file"));
				goto error;
			}

			if (opt && is_set(CFGF_DEPRECATED, opt->flags))
				cfg_handle_deprecated(cfg, opt);

			if (comment)
				free(comment);

			return STATE_EOF;
		}

		switch (state) {
		case 0:	/* expecting an option name */
			if (opt && is_set(CFGF_DEPRECATED, opt->flags))
				cfg_handle_deprecated(cfg, opt);

			switch (tok) {
			case '}':
				if (level == 0) {
					cfg_error(cfg, _("unexpected closing brace"));
					goto error;
				}
				if (comment)
					free(comment);

				return STATE_EOF;

			case CFGT_STR:
				break;

			case CFGT_COMMENT:
				if (!is_set(CFGF_COMMENTS, cfg->flags))
					continue;

				if (comment)
					free(comment);
				comment = strdup(cfg_yylval);
				continue;

			default:
				cfg_error(cfg, _("unexpected token '%s'"), cfg_yylval);
				goto error;
			}

			opt = cfg_getopt(cfg, cfg_yylval);
			if (!opt) {
				if (is_set(CFGF_IGNORE_UNKNOWN, cfg->flags)) {
					state = 10;
					break;
				}

				goto error;
			}

			if (opt->type == CFGT_SEC) {
				if (is_set(CFGF_TITLE, opt->flags))
					state = 6;
				else
					state = 5;
			} else if (opt->type == CFGT_FUNC) {
				state = 7;
			} else {
				state = 1;
			}
			break;

		case 1:	/* expecting an equal sign or plus-equal sign */
			if (!opt)
				goto error;

			if (tok == '+') {
				if (!is_set(CFGF_LIST, opt->flags)) {
					cfg_error(cfg, _("attempt to append to non-list option '%s'"), opt->name);
					goto error;
				}
				/* Even if the reset flag was set by
				 * cfg_init_defaults, appending to the defaults
				 * should be ok.
				 */
				opt->flags &= ~CFGF_RESET;
			} else if (tok == '=') {
				/* set the (temporary) reset flag to clear the old
				 * values, since we obviously didn't want to append */
				opt->flags |= CFGF_RESET;
			} else {
				cfg_error(cfg, _("missing equal sign after option '%s'"), opt->name);
				goto error;
			}

			if (is_set(CFGF_LIST, opt->flags)) {
				state = 3;
				num_values = 0;
			} else {
				state = 2;
			}
			break;

		case 2:	/* expecting an option value */
			if (tok == '}' && opt && is_set(CFGF_LIST, opt->flags)) {
				state = 0;
				if (num_values == 0 && is_set(CFGF_RESET, opt->flags))
					/* Reset flags was set, and the empty list was
					 * specified. Free all old values. */
					cfg_free_value(opt);
				break;
			}

			if (tok != CFGT_STR) {
				cfg_error(cfg, _("unexpected token '%s'"), cfg_yylval);
				goto error;
			}

			if (cfg_setopt(cfg, opt, cfg_yylval) == 0)
				goto error;

			if (opt && opt->validcb && (*opt->validcb) (cfg, opt) != 0)
				goto error;

			/* Inherit last read comment */
			cfg_opt_setcomment(opt, comment);
			comment = NULL;

			if (opt && is_set(CFGF_LIST, opt->flags)) {
				++num_values;
				state = 4;
			} else {
				state = 0;
			}
			break;

		case 3:	/* expecting an opening brace for a list option */
			if (tok != '{') {
				if (tok != CFGT_STR) {
					cfg_error(cfg, _("unexpected token '%s'"), cfg_yylval);
					goto error;
				}

				if (cfg_setopt(cfg, opt, cfg_yylval) == 0)
					goto error;
				if (opt && opt->validcb && (*opt->validcb) (cfg, opt) != 0)
					goto error;
				++num_values;
				state = 0;
			} else {
				state = 2;
			}
			break;

		case 4:	/* expecting a separator for a list option, or closing (list) brace */
			if (tok == ',') {
				state = 2;
			} else if (tok == '}') {
				state = 0;
				if (opt && opt->validcb && (*opt->validcb) (cfg, opt) != 0)
					goto error;
			} else {
				cfg_error(cfg, _("unexpected token '%s'"), cfg_yylval);
				goto error;
			}
			break;

		case 5:	/* expecting an opening brace for a section */
			if (tok != '{') {
				cfg_error(cfg, _("missing opening brace for section '%s'"), opt ? opt->name : "");
				goto error;
			}

			val = cfg_setopt(cfg, opt, opttitle);
			if (!val)
				goto error;

			if (opttitle)
				free(opttitle);
			opttitle = NULL;

			val->section->path = cfg->path; /* Remember global search path */
			val->section->line = cfg->line;
			val->section->errfunc = cfg->errfunc;
			rc = cfg_parse_internal(val->section, level + 1, -1, 0);
			if (rc != STATE_EOF)
				goto error;

			cfg->line = val->section->line;
			if (opt && opt->validcb && (*opt->validcb) (cfg, opt) != 0)
				goto error;
			state = 0;
			break;

		case 6:	/* expecting a title for a section */
			if (tok != CFGT_STR) {
				cfg_error(cfg, _("missing title for section '%s'"), opt ? opt->name : "");
				goto error;
			} else {
				opttitle = strdup(cfg_yylval);
				if (!opttitle)
					goto error;
			}
			state = 5;
			break;

		case 7:	/* expecting an opening parenthesis for a function */
			if (tok != '(') {
				cfg_error(cfg, _("missing parenthesis for function '%s'"), opt ? opt->name : "");
				goto error;
			}
			state = 8;
			break;

		case 8:	/* expecting a function parameter or a closing paren */
			if (tok == ')') {
				if (call_function(cfg, opt, &funcopt))
					goto error;
				state = 0;
			} else if (tok == CFGT_STR) {
				val = cfg_addval(&funcopt);
				if (!val)
					goto error;

				val->string = strdup(cfg_yylval);
				if (!val->string)
					goto error;

				state = 9;
			} else {
				cfg_error(cfg, _("syntax error in call of function '%s'"), opt ? opt->name : "");
				goto error;
			}
			break;

		case 9:	/* expecting a comma in a function or a closing paren */
			if (tok == ')') {
				if (call_function(cfg, opt, &funcopt))
					goto error;
				state = 0;
			} else if (tok == ',') {
				state = 8;
			} else {
				cfg_error(cfg, _("syntax error in call of function '%s'"), opt ? opt->name : "");
				goto error;
			}
			break;

		case 10: /* unknown option, mini-discard parser states: 10-15 */
			if (comment) {
				free(comment);
				comment = NULL;
			}

			if (tok == '+') {
				ignore = '=';
				state = 13; /* Append to list, should be followed by '=' */
			} else if (tok == '=') {
				ignore = 0;
				state = 14; /* Assignment, regular handling */
			} else if (tok == '(') {
				ignore = ')';
				state = 13; /* Function, ignore until end of param list */
			} else if (tok == '{') {
				state = 12; /* Section, ignore all until closing brace */
			} else if (tok == CFGT_STR) {
				state = 11; /* No '=' ... must be a titled section */
			} else if (tok == '}' && force_state == 10) {
				if (comment)
					free(comment);

				return STATE_CONTINUE;
			}
			break;

		case 11: /* unknown option, expecting start of title section */
			if (tok != '{') {
				cfg_error(cfg, _("unexpected token '%s'"), cfg_yylval);
				goto error;
			}
			state = 12;
			break;

		case 12: /* unknown option, recursively ignore entire sub-section */
			rc = cfg_parse_internal(cfg, level + 1, 10, NULL);
			if (rc != STATE_CONTINUE)
				goto error;
			ignore = '}';
			state = 13;
			break;

		case 13: /* unknown option, consume tokens silently until end of func/list */
			if (tok != ignore)
				break;

			if (ignore == '=') {
				ignore = 0;
				state = 14;
				break;
			}

			/* Are we done with recursive ignore of sub-section? */
			if (force_state == 10) {
				if (comment)
					free(comment);

				return STATE_CONTINUE;
			}

			ignore = 0;
			state = 0;
			break;

		case 14: /* unknown option, assuming value or start of list */
			if (tok == '{') {
				ignore = '}';
				state = 13;
				break;
			}

			if (tok != CFGT_STR) {
				cfg_error(cfg, _("unexpected token '%s'"), cfg_yylval);
				goto error;
			}

			ignore = 0;
			if (force_state == 10)
				state = 15;
			else
				state = 0;
			break;

		case 15: /* unknown option, dummy read of next parameter in sub-section */
			state = 10;
			break;

		default:
			cfg_error(cfg, _("Internal error in cfg_parse_internal(), unknown state %d"), state);
			goto error;
		}
	}

	if (comment)
		free(comment);

	return STATE_EOF;

error:
	if (opttitle)
		free(opttitle);
	if (comment)
		free(comment);

	return STATE_ERROR;
}

DLLIMPORT int cfg_parse_fp(cfg_t *cfg, FILE *fp)
{
	int ret;

	if (!cfg || !fp) {
		errno = EINVAL;
		return CFG_PARSE_ERROR;
	}

	if (!cfg->filename)
		cfg->filename = strdup("FILE");
	if (!cfg->filename)
		return CFG_PARSE_ERROR;

	cfg->line = 1;
	cfg_scan_fp_begin(fp);
	ret = cfg_parse_internal(cfg, 0, -1, NULL);
	cfg_scan_fp_end();
	if (ret == STATE_ERROR)
		return CFG_PARSE_ERROR;

	return CFG_SUCCESS;
}

static char *cfg_make_fullpath(const char *dir, const char *file)
{
	int np;
	char *path;
	size_t len;

	if (!dir || !file) {
		errno = EINVAL;
		return NULL;
	}

	len = strlen(dir) + strlen(file) + 2;
	path = malloc(len);
	if (!path)
		return NULL;

	np = snprintf(path, len, "%s/%s", dir, file);

	/*
	 * np is the number of characters that would have
	 * been printed if there was enough room in path.
	 * if np >= n then the snprintf() was truncated
	 * (which must be a bug).
	 */
	assert(np < (int)len);

	return path;
}

DLLIMPORT char *cfg_searchpath(cfg_searchpath_t *p, const char *file)
{
	char *fullpath;
#ifdef HAVE_SYS_STAT_H
	struct stat st;
	int err;
#endif

	if (!p || !file) {
		errno = EINVAL;
		return NULL;
	}

	if ((fullpath = cfg_searchpath(p->next, file)) != NULL)
		return fullpath;

	if ((fullpath = cfg_make_fullpath(p->dir, file)) == NULL)
		return NULL;

#ifdef HAVE_SYS_STAT_H
	err = stat((const char *)fullpath, &st);
	if ((!err) && S_ISREG(st.st_mode))
		return fullpath;
#else
	/* needs an alternative check here for win32 */
#endif

	free(fullpath);
	return NULL;
}

DLLIMPORT int cfg_parse(cfg_t *cfg, const char *filename)
{
	int ret;
	char *fn;
	FILE *fp;

	if (!cfg || !filename) {
		errno = EINVAL;
		return CFG_FILE_ERROR;
	}

	if (cfg->path)
		fn = cfg_searchpath(cfg->path, filename);
	else
		fn = cfg_tilde_expand(filename);
	if (!fn)
		return CFG_FILE_ERROR;

	free(cfg->filename);
	cfg->filename = fn;

	fp = fopen(cfg->filename, "r");
	if (!fp)
		return CFG_FILE_ERROR;

	ret = cfg_parse_fp(cfg, fp);
	fclose(fp);

	return ret;
}

DLLIMPORT int cfg_parse_buf(cfg_t *cfg, const char *buf)
{
	int ret;
	char *fn;
	FILE *fp;

	if (!cfg) {
		errno = EINVAL;
		return CFG_PARSE_ERROR;
	}

	if (!buf)
		return CFG_SUCCESS;

	fn = strdup("[buf]");
	if (!fn)
		return CFG_PARSE_ERROR;

	free(cfg->filename);
	cfg->filename = fn;

	fp = fmemopen((void *)buf, strlen(buf), "r");
	if (!fp) {
		/*
		 * fmemopen() on older GLIBC versions do not accept zero
		 * length buffers for some reason.  This is a workaround.
		 */
		if (strlen(buf) > 0)
			return CFG_FILE_ERROR;

		return CFG_SUCCESS;
	}

	ret = cfg_parse_fp(cfg, fp);
	fclose(fp);

	return ret;
}

DLLIMPORT cfg_t *cfg_init(cfg_opt_t *opts, cfg_flag_t flags)
{
	cfg_t *cfg;

	cfg = calloc(1, sizeof(cfg_t));
	if (!cfg)
		return NULL;

	cfg->name = strdup("root");
	if (!cfg->name) {
		free(cfg);
		return NULL;
	}

	cfg->opts = cfg_dupopt_array(opts);
	if (!cfg->opts) {
		free(cfg->name);
		free(cfg);
		return NULL;
	}

	cfg->flags = flags;
	cfg->filename = 0;
	cfg->line = 0;
	cfg->errfunc = 0;

#if defined(ENABLE_NLS) && defined(HAVE_GETTEXT)
	bindtextdomain(PACKAGE, LOCALEDIR);
#endif

	cfg_init_defaults(cfg);

	return cfg;
}

DLLIMPORT char *cfg_tilde_expand(const char *filename)
{
	char *expanded = 0;

#ifndef _WIN32
	/* Do tilde expansion */
	if (filename[0] == '~') {
		struct passwd *passwd = 0;
		const char *file = 0;

		if (filename[1] == '/' || filename[1] == 0) {
			/* ~ or ~/path */
			passwd = getpwuid(geteuid());
			file = filename + 1;
		} else {
			/* ~user or ~user/path */
			char *user;

			file = strchr(filename, '/');
			if (file == 0)
				file = filename + strlen(filename);

			user = malloc(file - filename);
			if (!user)
				return NULL;

			strncpy(user, filename + 1, file - filename - 1);
			passwd = getpwnam(user);
			free(user);
		}

		if (passwd) {
			expanded = malloc(strlen(passwd->pw_dir) + strlen(file) + 1);
			if (!expanded)
				return NULL;

			strcpy(expanded, passwd->pw_dir);
			strcat(expanded, file);
		}
	}
#endif
	if (!expanded)
		expanded = strdup(filename);

	return expanded;
}

DLLIMPORT int cfg_free_value(cfg_opt_t *opt)
{
	if (!opt) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	if (opt->comment && !is_set(CFGF_RESET, opt->flags)) {
		free(opt->comment);
		opt->comment = NULL;
	}

	if (opt->values) {
		unsigned int i;

		for (i = 0; i < opt->nvalues; i++) {
			if (opt->type == CFGT_STR) {
				free((void *)opt->values[i]->string);
			} else if (opt->type == CFGT_SEC) {
				opt->values[i]->section->path = NULL; /* Global search path */
				cfg_free(opt->values[i]->section);
			} else if (opt->type == CFGT_PTR && opt->freecb && opt->values[i]->ptr) {
				(opt->freecb) (opt->values[i]->ptr);
			}
			free(opt->values[i]);
		}
		free(opt->values);
	}

	opt->values  = NULL;
	opt->nvalues = 0;

	return CFG_SUCCESS;
}

static void cfg_free_opt_array(cfg_opt_t *opts)
{
	int i;

	for (i = 0; opts[i].name; ++i) {
		free((void *)opts[i].name);
		if (opts[i].comment)
			free(opts[i].comment);
		if (opts[i].def.parsed)
			free(opts[i].def.parsed);
		if (opts[i].def.string)
			free((void *)opts[i].def.string);
		if (opts[i].subopts)
			cfg_free_opt_array(opts[i].subopts);
	}
	free(opts);
}

static int cfg_free_searchpath(cfg_searchpath_t *p)
{
	if (p) {
		cfg_free_searchpath(p->next);
		free(p->dir);
		free(p);
	}

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_free(cfg_t *cfg)
{
	int i;
	int isroot = 0;

	if (!cfg) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	if (cfg->comment)
		free(cfg->comment);

	for (i = 0; cfg->opts[i].name; ++i)
		cfg_free_value(&cfg->opts[i]);

	cfg_free_opt_array(cfg->opts);
	cfg_free_searchpath(cfg->path);

	if (cfg->name) {
		isroot = !strcmp(cfg->name, "root");
		free(cfg->name);
	}
	if (cfg->title)
		free(cfg->title);
	if (cfg->filename)
		free(cfg->filename);

	free(cfg);
	if (isroot)
		cfg_yylex_destroy();

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_include(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv)
{
	(void)opt;		/* Unused in this predefined include FUNC */

	if (!cfg || !argv) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	if (argc != 1) {
		cfg_error(cfg, _("wrong number of arguments to cfg_include()"));
		return 1;
	}

	return cfg_lexer_include(cfg, argv[0]);
}

static cfg_value_t *cfg_opt_getval(cfg_opt_t *opt, unsigned int index)
{
	cfg_value_t *val = 0;

	if (index != 0 && !is_set(CFGF_LIST, opt->flags) && !is_set(CFGF_MULTI, opt->flags)) {
		errno = EINVAL;
		return NULL;
	}

	if (opt->simple_value.ptr)
		val = (cfg_value_t *)opt->simple_value.ptr;
	else {
		if (is_set(CFGF_RESET, opt->flags)) {
			cfg_free_value(opt);
			opt->flags &= ~CFGF_RESET;
		}

		if (index >= opt->nvalues)
			val = cfg_addval(opt);
		else
			val = opt->values[index];
	}

	return val;
}

DLLIMPORT int cfg_opt_setcomment(cfg_opt_t *opt, char *comment)
{
	char *oldcomment, *newcomment;

	if (!opt || !comment) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	oldcomment = opt->comment;
	newcomment = strdup(comment);
	if (!newcomment)
		return CFG_FAIL;

	if (oldcomment)
		free(oldcomment);
	opt->comment = newcomment;
	opt->flags  |= CFGF_COMMENTS;

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_setcomment(cfg_t *cfg, const char *name, char *comment)
{
	return cfg_opt_setcomment(cfg_getopt(cfg, name), comment);
}

DLLIMPORT int cfg_opt_setnint(cfg_opt_t *opt, long int value, unsigned int index)
{
	cfg_value_t *val;

	if (!opt || opt->type != CFGT_INT) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	val = cfg_opt_getval(opt, index);
	if (!val)
		return CFG_FAIL;

	val->number = value;

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_setnint(cfg_t *cfg, const char *name, long int value, unsigned int index)
{
	cfg_opt_t *opt;

	opt = cfg_getopt(cfg, name);
	if (opt && opt->validcb2 && (*opt->validcb2)(cfg, opt, (void *)&value) != 0)
		return CFG_FAIL;

	return cfg_opt_setnint(opt, value, index);
}

DLLIMPORT int cfg_setint(cfg_t *cfg, const char *name, long int value)
{
	return cfg_setnint(cfg, name, value, 0);
}

DLLIMPORT int cfg_opt_setnfloat(cfg_opt_t *opt, double value, unsigned int index)
{
	cfg_value_t *val;

	if (!opt || opt->type != CFGT_FLOAT) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	val = cfg_opt_getval(opt, index);
	if (!val)
		return CFG_FAIL;

	val->fpnumber = value;

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_setnfloat(cfg_t *cfg, const char *name, double value, unsigned int index)
{
	cfg_opt_t *opt;

	opt = cfg_getopt(cfg, name);
	if (opt && opt->validcb2 && (*opt->validcb2)(cfg, opt, (void *)&value) != 0)
		return CFG_FAIL;

	return cfg_opt_setnfloat(opt, value, index);
}

DLLIMPORT int cfg_setfloat(cfg_t *cfg, const char *name, double value)
{
	return cfg_setnfloat(cfg, name, value, 0);
}

DLLIMPORT int cfg_opt_setnbool(cfg_opt_t *opt, cfg_bool_t value, unsigned int index)
{
	cfg_value_t *val;

	if (!opt || opt->type != CFGT_BOOL) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	val = cfg_opt_getval(opt, index);
	if (!val)
		return CFG_FAIL;

	val->boolean = value;

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_setnbool(cfg_t *cfg, const char *name, cfg_bool_t value, unsigned int index)
{
	return cfg_opt_setnbool(cfg_getopt(cfg, name), value, index);
}

DLLIMPORT int cfg_setbool(cfg_t *cfg, const char *name, cfg_bool_t value)
{
	return cfg_setnbool(cfg, name, value, 0);
}

DLLIMPORT int cfg_opt_setnstr(cfg_opt_t *opt, const char *value, unsigned int index)
{
	char *newstr, *oldstr = NULL;
	cfg_value_t *val;

	if (!opt || opt->type != CFGT_STR) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	val = cfg_opt_getval(opt, index);
	if (!val)
		return CFG_FAIL;

	if (val->string)
		oldstr = val->string;

	if (value) {
		newstr = strdup(value);
		if (!newstr)
			return CFG_FAIL;
		val->string = newstr;
	} else {
		val->string = NULL;
	}

	if (oldstr)
		free(oldstr);

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_setnstr(cfg_t *cfg, const char *name, const char *value, unsigned int index)
{
	cfg_opt_t *opt;

	opt = cfg_getopt(cfg, name);
	if (opt && opt->validcb2 && (*opt->validcb2)(cfg, opt, (void *)value) != 0)
		return CFG_FAIL;

	return cfg_opt_setnstr(opt, value, index);
}

DLLIMPORT int cfg_setstr(cfg_t *cfg, const char *name, const char *value)
{
	return cfg_setnstr(cfg, name, value, 0);
}

static int cfg_addlist_internal(cfg_opt_t *opt, unsigned int nvalues, va_list ap)
{
	int result = CFG_FAIL;
	unsigned int i;

	for (i = 0; i < nvalues; i++) {
		switch (opt->type) {
		case CFGT_INT:
			result = cfg_opt_setnint(opt, va_arg(ap, int), opt->nvalues);
			break;

		case CFGT_FLOAT:
			result = cfg_opt_setnfloat(opt, va_arg(ap, double), opt->nvalues);
			break;

		case CFGT_BOOL:
			result = cfg_opt_setnbool(opt, va_arg(ap, cfg_bool_t), opt->nvalues);
			break;

		case CFGT_STR:
			result = cfg_opt_setnstr(opt, va_arg(ap, char *), opt->nvalues);
			break;

		case CFGT_FUNC:
		case CFGT_SEC:
		default:
			result = CFG_SUCCESS;
			break;
		}
	}

	return result;
}

DLLIMPORT int cfg_setlist(cfg_t *cfg, const char *name, unsigned int nvalues, ...)
{
	va_list ap;
	cfg_opt_t *opt = cfg_getopt(cfg, name);

	if (!opt || !is_set(CFGF_LIST, opt->flags)) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	cfg_free_value(opt);
	va_start(ap, nvalues);
	cfg_addlist_internal(opt, nvalues, ap);
	va_end(ap);

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_addlist(cfg_t *cfg, const char *name, unsigned int nvalues, ...)
{
	va_list ap;
	cfg_opt_t *opt = cfg_getopt(cfg, name);

	if (!opt || !is_set(CFGF_LIST, opt->flags)) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	va_start(ap, nvalues);
	cfg_addlist_internal(opt, nvalues, ap);
	va_end(ap);

	return CFG_SUCCESS;
}

DLLIMPORT cfg_t *cfg_addtsec(cfg_t *cfg, const char *name, const char *title)
{
	cfg_opt_t *opt;
	cfg_value_t *val;

	if (cfg_gettsec(cfg, name, title))
		return NULL;

	opt = cfg_getopt(cfg, name);
	if (!opt) {
		cfg_error(cfg, _("no such option '%s'"), name);
		return NULL;
	}
	val = cfg_setopt(cfg, opt, title);
	if (!val)
		return NULL;

	val->section->path = cfg->path; /* Remember global search path. */
	val->section->line = 1;
	val->section->errfunc = cfg->errfunc;

	return val->section;
}

DLLIMPORT int cfg_opt_rmnsec(cfg_opt_t *opt, unsigned int index)
{
	unsigned int n;
	cfg_value_t *val;

	if (!opt || opt->type != CFGT_SEC) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	n = cfg_opt_size(opt);
	if (index >= n)
		return CFG_FAIL;

	val = cfg_opt_getval(opt, index);
	if (!val)
		return CFG_FAIL;

	if (index + 1 != n) {
		/* not removing last, move the tail */
		memmove(&opt->values[index], &opt->values[index + 1], sizeof(opt->values[index]) * (n - index - 1));
	}
	--opt->nvalues;

	cfg_free(val->section);
	free(val);

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_rmnsec(cfg_t *cfg, const char *name, unsigned int index)
{
	return cfg_opt_rmnsec(cfg_getopt(cfg, name), index);
}

DLLIMPORT int cfg_rmsec(cfg_t *cfg, const char *name)
{
	return cfg_rmnsec(cfg, name, 0);
}

DLLIMPORT int cfg_opt_rmtsec(cfg_opt_t *opt, const char *title)
{
	unsigned int i, n;

	if (!opt || !title) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	if (!is_set(CFGF_TITLE, opt->flags))
		return CFG_FAIL;

	n = cfg_opt_size(opt);
	for (i = 0; i < n; i++) {
		cfg_t *sec = cfg_opt_getnsec(opt, i);

		if (!sec || !sec->title)
			return CFG_FAIL;

		if (is_set(CFGF_NOCASE, opt->flags)) {
			if (strcasecmp(title, sec->title) == 0)
				break;
		} else {
			if (strcmp(title, sec->title) == 0)
				break;
		}
	}
	if (i == n)
		return CFG_FAIL;

	return cfg_opt_rmnsec(opt, i);
}

DLLIMPORT int cfg_rmtsec(cfg_t *cfg, const char *name, const char *title)
{
	return cfg_opt_rmtsec(cfg_getopt(cfg, name), title);
}

DLLIMPORT int cfg_opt_nprint_var(cfg_opt_t *opt, unsigned int index, FILE *fp)
{
	const char *str;

	if (!opt || !fp) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	switch (opt->type) {
	case CFGT_INT:
		fprintf(fp, "%ld", cfg_opt_getnint(opt, index));
		break;

	case CFGT_FLOAT:
		fprintf(fp, "%f", cfg_opt_getnfloat(opt, index));
		break;

	case CFGT_STR:
		str = cfg_opt_getnstr(opt, index);
		fprintf(fp, "\"");
		while (str && *str) {
			if (*str == '"')
				fprintf(fp, "\\\"");
			else if (*str == '\\')
				fprintf(fp, "\\\\");
			else
				fprintf(fp, "%c", *str);
			str++;
		}
		fprintf(fp, "\"");
		break;

	case CFGT_BOOL:
		fprintf(fp, "%s", cfg_opt_getnbool(opt, index) ? "true" : "false");
		break;

	case CFGT_NONE:
	case CFGT_SEC:
	case CFGT_FUNC:
	case CFGT_PTR:
	case CFGT_COMMENT:
		break;
	}

	return CFG_SUCCESS;
}

static void cfg_indent(FILE *fp, int indent)
{
	while (indent--)
		fprintf(fp, "  ");
}

DLLIMPORT int cfg_opt_print_indent(cfg_opt_t *opt, FILE *fp, int indent)
{
	if (!opt || !fp) {
		errno = EINVAL;
		return CFG_FAIL;
	}

	if (is_set(CFGF_COMMENTS, opt->flags) && opt->comment) {
		cfg_indent(fp, indent);
		fprintf(fp, "/* %s */\n", opt->comment);
	}

	if (opt->type == CFGT_SEC) {
		cfg_t *sec;
		unsigned int i;

		for (i = 0; i < cfg_opt_size(opt); i++) {
			sec = cfg_opt_getnsec(opt, i);
			cfg_indent(fp, indent);
			if (is_set(CFGF_TITLE, opt->flags))
				fprintf(fp, "%s \"%s\" {\n", opt->name, cfg_title(sec));
			else
				fprintf(fp, "%s {\n", opt->name);
			cfg_print_indent(sec, fp, indent + 1);
			cfg_indent(fp, indent);
			fprintf(fp, "}\n");
		}
	} else if (opt->type != CFGT_FUNC && opt->type != CFGT_NONE) {
		if (is_set(CFGF_LIST, opt->flags)) {
			cfg_indent(fp, indent);
			fprintf(fp, "%s = {", opt->name);

			if (opt->nvalues) {
				unsigned int i;

				if (opt->pf)
					opt->pf(opt, 0, fp);
				else
					cfg_opt_nprint_var(opt, 0, fp);
				for (i = 1; i < opt->nvalues; i++) {
					fprintf(fp, ", ");
					if (opt->pf)
						opt->pf(opt, i, fp);
					else
						cfg_opt_nprint_var(opt, i, fp);
				}
			}

			fprintf(fp, "}");
		} else {
			cfg_indent(fp, indent);
			/* comment out the option if is not set */
			if (opt->simple_value.ptr) {
				if (opt->type == CFGT_STR && *opt->simple_value.string == 0)
					fprintf(fp, "# ");
			} else {
				if (cfg_opt_size(opt) == 0 || (opt->type == CFGT_STR && (opt->values[0]->string == 0 ||
											 opt->values[0]->string[0] == 0)))
					fprintf(fp, "# ");
			}
			fprintf(fp, "%s=", opt->name);
			if (opt->pf)
				opt->pf(opt, 0, fp);
			else
				cfg_opt_nprint_var(opt, 0, fp);
		}

		fprintf(fp, "\n");
	} else if (opt->pf) {
		cfg_indent(fp, indent);
		opt->pf(opt, 0, fp);
		fprintf(fp, "\n");
	}

	return CFG_SUCCESS;
}

DLLIMPORT int cfg_opt_print(cfg_opt_t *opt, FILE *fp)
{
	return cfg_opt_print_indent(opt, fp, 0);
}

DLLIMPORT int cfg_print_indent(cfg_t *cfg, FILE *fp, int indent)
{
	int i, result = CFG_SUCCESS;

	for (i = 0; cfg->opts[i].name; i++)
		result += cfg_opt_print_indent(&cfg->opts[i], fp, indent);

	return result;
}

DLLIMPORT int cfg_print(cfg_t *cfg, FILE *fp)
{
	return cfg_print_indent(cfg, fp, 0);
}

DLLIMPORT cfg_print_func_t cfg_opt_set_print_func(cfg_opt_t *opt, cfg_print_func_t pf)
{
	cfg_print_func_t oldpf;

	if (!opt) {
		errno = EINVAL;
		return NULL;
	}

	oldpf = opt->pf;
	opt->pf = pf;

	return oldpf;
}

DLLIMPORT cfg_print_func_t cfg_set_print_func(cfg_t *cfg, const char *name, cfg_print_func_t pf)
{
	return cfg_opt_set_print_func(cfg_getopt(cfg, name), pf);
}

static cfg_opt_t *cfg_getopt_array(cfg_opt_t *rootopts, int cfg_flags, const char *name)
{
	unsigned int i;
	cfg_opt_t *opts = rootopts;

	if (!rootopts || !name) {
		errno = EINVAL;
		return NULL;
	}

	while (name && *name) {
		cfg_t *seccfg;
		char *secname;
		size_t len = strcspn(name, "|");

		if (name[len] == 0 /*len == strlen(name) */ )
			/* no more subsections */
			break;

		if (len) {
			cfg_opt_t *secopt;

			secname = strndup(name, len);
			if (!secname)
				return NULL;

			secopt = cfg_getopt_array(opts, cfg_flags, secname);
			free(secname);
			if (!secopt) {
				/*fprintf(stderr, "section not found\n"); */
				return NULL;
			}
			if (secopt->type != CFGT_SEC) {
				/*fprintf(stderr, "not a section!\n"); */
				return NULL;
			}

			if (!is_set(CFGF_MULTI, secopt->flags) && (seccfg = cfg_opt_getnsec(secopt, 0)) != 0)
				opts = seccfg->opts;
			else
				opts = secopt->subopts;

			if (!opts) {
				/*fprintf(stderr, "section have no subopts!?\n"); */
				return NULL;
			}
		}
		name += len;
		name += strspn(name, "|");
	}

	for (i = 0; opts[i].name; i++) {
		if (is_set(CFGF_NOCASE, cfg_flags)) {
			if (strcasecmp(opts[i].name, name) == 0)
				return &opts[i];
		} else {
			if (strcmp(opts[i].name, name) == 0)
				return &opts[i];
		}
	}

	return NULL;
}

DLLIMPORT cfg_validate_callback_t cfg_set_validate_func(cfg_t *cfg, const char *name, cfg_validate_callback_t vf)
{
	cfg_opt_t *opt;
	cfg_validate_callback_t oldvf;

	opt = cfg_getopt_array(cfg->opts, cfg->flags, name);
	if (!opt)
		return NULL;

	oldvf = opt->validcb;
	opt->validcb = vf;

	return oldvf;
}

DLLIMPORT cfg_validate_callback2_t cfg_set_validate_func2(cfg_t *cfg, const char *name, cfg_validate_callback2_t vf)
{
	cfg_opt_t *opt;
	cfg_validate_callback2_t oldvf;

	opt = cfg_getopt_array(cfg->opts, cfg->flags, name);
	if (!opt)
		return NULL;

	oldvf = opt->validcb2;
	opt->validcb2 = vf;

	return oldvf;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
