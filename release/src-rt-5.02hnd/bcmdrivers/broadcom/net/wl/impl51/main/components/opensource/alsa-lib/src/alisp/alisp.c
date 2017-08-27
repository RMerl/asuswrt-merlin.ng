/*
 *  ALSA lisp implementation
 *  Copyright (c) 2003 by Jaroslav Kysela <perex@perex.cz>
 *
 *  Based on work of Sandro Sigala (slisp-1.2)
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <assert.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <err.h>

#define alisp_seq_iterator alisp_object

#include "local.h"
#include "alisp.h"
#include "alisp_local.h"

struct alisp_object alsa_lisp_nil;
struct alisp_object alsa_lisp_t;

/* parser prototypes */
static struct alisp_object * parse_object(struct alisp_instance *instance, int havetoken);
static void princ_cons(snd_output_t *out, struct alisp_object * p);
static void princ_object(snd_output_t *out, struct alisp_object * p);
static struct alisp_object * eval(struct alisp_instance *instance, struct alisp_object * p);

/* functions */
static struct alisp_object *F_eval(struct alisp_instance *instance, struct alisp_object *);
static struct alisp_object *F_progn(struct alisp_instance *instance, struct alisp_object *);
static struct alisp_object *F_funcall(struct alisp_instance *instance, struct alisp_object *);

/* others */
static int alisp_include_file(struct alisp_instance *instance, const char *filename);

/*
 *  object handling
 */

static int get_string_hash(const char *s)
{
	int val = 0;
	if (s == NULL)
		return val;
	while (*s)
		val += *s++;
	return val & ALISP_OBJ_PAIR_HASH_MASK;
}

static void nomem(void)
{
	SNDERR("alisp: no enough memory");
}

static void lisp_verbose(struct alisp_instance *instance, const char *fmt, ...)
{
	va_list ap;

	if (!instance->verbose)
		return;
	va_start(ap, fmt);
	snd_output_printf(instance->vout, "alisp: ");
	snd_output_vprintf(instance->vout, fmt, ap);
	snd_output_putc(instance->vout, '\n');
	va_end(ap);
}

static void lisp_error(struct alisp_instance *instance, const char *fmt, ...)
{
	va_list ap;

	if (!instance->warning)
		return;
	va_start(ap, fmt);
	snd_output_printf(instance->eout, "alisp error: ");
	snd_output_vprintf(instance->eout, fmt, ap);
	snd_output_putc(instance->eout, '\n');
	va_end(ap);
}

static void lisp_warn(struct alisp_instance *instance, const char *fmt, ...)
{
	va_list ap;

	if (!instance->warning)
		return;
	va_start(ap, fmt);
	snd_output_printf(instance->wout, "alisp warning: ");
	snd_output_vprintf(instance->wout, fmt, ap);
	snd_output_putc(instance->wout, '\n');
	va_end(ap);
}

static void lisp_debug(struct alisp_instance *instance, const char *fmt, ...)
{
	va_list ap;

	if (!instance->debug)
		return;
	va_start(ap, fmt);
	snd_output_printf(instance->dout, "alisp debug: ");
	snd_output_vprintf(instance->dout, fmt, ap);
	snd_output_putc(instance->dout, '\n');
	va_end(ap);
}

static struct alisp_object * new_object(struct alisp_instance *instance, int type)
{
	struct alisp_object * p;

	if (list_empty(&instance->free_objs_list)) {
		p = (struct alisp_object *)malloc(sizeof(struct alisp_object));
		if (p == NULL) {
			nomem();
			return NULL;
		}
		lisp_debug(instance, "allocating cons %p", p);
	} else {
		p = (struct alisp_object *)instance->free_objs_list.next;
		list_del(&p->list);
		instance->free_objs--;
		lisp_debug(instance, "recycling cons %p", p);
	}

	instance->used_objs++;

	alisp_set_type(p, type);
	alisp_set_refs(p, 1);
	if (type == ALISP_OBJ_CONS) {
		p->value.c.car = &alsa_lisp_nil;
		p->value.c.cdr = &alsa_lisp_nil;
		list_add(&p->list, &instance->used_objs_list[0][ALISP_OBJ_CONS]);
	}

	if (instance->used_objs + instance->free_objs > instance->max_objs)
		instance->max_objs = instance->used_objs + instance->free_objs;

	return p;
}

static void free_object(struct alisp_object * p)
{
	switch (alisp_get_type(p)) {
	case ALISP_OBJ_STRING:
	case ALISP_OBJ_IDENTIFIER:
		free(p->value.s);
		alisp_set_type(p, ALISP_OBJ_INTEGER);
		break;
	default:
		break;
	}
}

static void delete_object(struct alisp_instance *instance, struct alisp_object * p)
{
	if (p == NULL || p == &alsa_lisp_nil || p == &alsa_lisp_t)
		return;
	if (alisp_compare_type(p, ALISP_OBJ_NIL) ||
	    alisp_compare_type(p, ALISP_OBJ_T))
		return;
	assert(alisp_get_refs(p) > 0);
	lisp_debug(instance, "delete cons %p (type = %i, refs = %i) (s = '%s')", p, alisp_get_type(p), alisp_get_refs(p),
			alisp_compare_type(p, ALISP_OBJ_STRING) ||
			alisp_compare_type(p, ALISP_OBJ_IDENTIFIER) ? p->value.s : "???");
	if (alisp_dec_refs(p))
		return;
	list_del(&p->list);
	instance->used_objs--;
	free_object(p);
	if (instance->free_objs >= ALISP_FREE_OBJ_POOL) {
		lisp_debug(instance, "freed cons %p", p);
		free(p);
		return;
	}
	lisp_debug(instance, "moved cons %p to free list", p);
	list_add(&p->list, &instance->free_objs_list);
	instance->free_objs++;
}

static void delete_tree(struct alisp_instance *instance, struct alisp_object * p)
{
	if (p == NULL)
		return;
	if (alisp_compare_type(p, ALISP_OBJ_CONS)) {
		delete_tree(instance, p->value.c.car);
		delete_tree(instance, p->value.c.cdr);
	}
	delete_object(instance, p);
}

static struct alisp_object * incref_object(struct alisp_instance *instance ATTRIBUTE_UNUSED, struct alisp_object * p)
{
	if (p == NULL || p == &alsa_lisp_nil || p == &alsa_lisp_t)
		return p;
	if (alisp_get_refs(p) == ALISP_MAX_REFS) {
		assert(0);
		fprintf(stderr, "OOPS: alsa lisp: incref fatal error\n");
		exit(EXIT_FAILURE);
	}
	alisp_inc_refs(p);
	return p;
}

static struct alisp_object * incref_tree(struct alisp_instance *instance, struct alisp_object * p)
{
	if (p == NULL)
		return NULL;
	if (alisp_compare_type(p, ALISP_OBJ_CONS)) {
		incref_tree(instance, p->value.c.car);
		incref_tree(instance, p->value.c.cdr);
	}
	return incref_object(instance, p);
}

/* Function not used yet. Leave it commented out until we actually use it to
 * avoid compiler complaints */

static void free_objects(struct alisp_instance *instance)
{
	struct list_head *pos, *pos1;
	struct alisp_object * p;
	struct alisp_object_pair * pair;
	int i, j;

	for (i = 0; i < ALISP_OBJ_PAIR_HASH_SIZE; i++) {
		list_for_each_safe(pos, pos1, &instance->setobjs_list[i]) {
			pair = list_entry(pos, struct alisp_object_pair, list);
			lisp_debug(instance, "freeing pair: '%s' -> %p", pair->name, pair->value);
			delete_tree(instance, pair->value);
			free((void *)pair->name);
			free(pair);
		}
	}
	for (i = 0; i < ALISP_OBJ_PAIR_HASH_SIZE; i++)
		for (j = 0; j <= ALISP_OBJ_LAST_SEARCH; j++) {
			list_for_each_safe(pos, pos1, &instance->used_objs_list[i][j]) {
				p = list_entry(pos, struct alisp_object, list);
				lisp_warn(instance, "object %p is still referenced %i times!", p, alisp_get_refs(p));
				if (alisp_get_refs(p) > 0)
					alisp_set_refs(p, 1);
				delete_object(instance, p);
			}
		}
	list_for_each_safe(pos, pos1, &instance->free_objs_list) {
		p = list_entry(pos, struct alisp_object, list);
		list_del(&p->list);
		free(p);
		lisp_debug(instance, "freed (all) cons %p", p);
	}
}

static struct alisp_object * search_object_identifier(struct alisp_instance *instance, const char *s)
{
	struct list_head * pos;
	struct alisp_object * p;

	list_for_each(pos, &instance->used_objs_list[get_string_hash(s)][ALISP_OBJ_IDENTIFIER]) {
		p = list_entry(pos, struct alisp_object, list);
		if (alisp_get_refs(p) > ALISP_MAX_REFS_LIMIT)
			continue;
		if (!strcmp(p->value.s, s))
			return incref_object(instance, p);
	}

	return NULL;
}

static struct alisp_object * search_object_string(struct alisp_instance *instance, const char *s)
{
	struct list_head * pos;
	struct alisp_object * p;

	list_for_each(pos, &instance->used_objs_list[get_string_hash(s)][ALISP_OBJ_STRING]) {
		p = list_entry(pos, struct alisp_object, list);
		if (!strcmp(p->value.s, s)) {
			if (alisp_get_refs(p) > ALISP_MAX_REFS_LIMIT)
				continue;
			return incref_object(instance, p);
		}
	}

	return NULL;
}

static struct alisp_object * search_object_integer(struct alisp_instance *instance, long in)
{
	struct list_head * pos;
	struct alisp_object * p;

	list_for_each(pos, &instance->used_objs_list[in & ALISP_OBJ_PAIR_HASH_MASK][ALISP_OBJ_INTEGER]) {
		p = list_entry(pos, struct alisp_object, list);
		if (p->value.i == in) {
			if (alisp_get_refs(p) > ALISP_MAX_REFS_LIMIT)
				continue;
			return incref_object(instance, p);
		}
	}

	return NULL;
}

static struct alisp_object * search_object_float(struct alisp_instance *instance, double in)
{
	struct list_head * pos;
	struct alisp_object * p;

	list_for_each(pos, &instance->used_objs_list[(long)in & ALISP_OBJ_PAIR_HASH_MASK][ALISP_OBJ_FLOAT]) {
		p = list_entry(pos, struct alisp_object, list);
		if (p->value.i == in) {
			if (alisp_get_refs(p) > ALISP_MAX_REFS_LIMIT)
				continue;
			return incref_object(instance, p);
		}
	}

	return NULL;
}

static struct alisp_object * search_object_pointer(struct alisp_instance *instance, const void *ptr)
{
	struct list_head * pos;
	struct alisp_object * p;

	list_for_each(pos, &instance->used_objs_list[(long)ptr & ALISP_OBJ_PAIR_HASH_MASK][ALISP_OBJ_POINTER]) {
		p = list_entry(pos, struct alisp_object, list);
		if (p->value.ptr == ptr) {
			if (alisp_get_refs(p) > ALISP_MAX_REFS_LIMIT)
				continue;
			return incref_object(instance, p);
		}
	}

	return NULL;
}

static struct alisp_object * new_integer(struct alisp_instance *instance, long value)
{
	struct alisp_object * obj;
	
	obj = search_object_integer(instance, value);
	if (obj != NULL)
		return obj;
	obj = new_object(instance, ALISP_OBJ_INTEGER);
	if (obj) {
		list_add(&obj->list, &instance->used_objs_list[value & ALISP_OBJ_PAIR_HASH_MASK][ALISP_OBJ_INTEGER]);
		obj->value.i = value;
	}
	return obj;
}

static struct alisp_object * new_float(struct alisp_instance *instance, double value)
{
	struct alisp_object * obj;
	
	obj = search_object_float(instance, value);
	if (obj != NULL)
		return obj;
	obj = new_object(instance, ALISP_OBJ_FLOAT);
	if (obj) {
		list_add(&obj->list, &instance->used_objs_list[(long)value & ALISP_OBJ_PAIR_HASH_MASK][ALISP_OBJ_FLOAT]);
		obj->value.f = value;
	}
	return obj;
}

static struct alisp_object * new_string(struct alisp_instance *instance, const char *str)
{
	struct alisp_object * obj;
	
	obj = search_object_string(instance, str);
	if (obj != NULL)
		return obj;
	obj = new_object(instance, ALISP_OBJ_STRING);
	if (obj)
		list_add(&obj->list, &instance->used_objs_list[get_string_hash(str)][ALISP_OBJ_STRING]);
	if (obj && (obj->value.s = strdup(str)) == NULL) {
		delete_object(instance, obj);
		nomem();
		return NULL;
	}
	return obj;
}

static struct alisp_object * new_identifier(struct alisp_instance *instance, const char *id)
{
	struct alisp_object * obj;
	
	obj = search_object_identifier(instance, id);
	if (obj != NULL)
		return obj;
	obj = new_object(instance, ALISP_OBJ_IDENTIFIER);
	if (obj)
		list_add(&obj->list, &instance->used_objs_list[get_string_hash(id)][ALISP_OBJ_IDENTIFIER]);
	if (obj && (obj->value.s = strdup(id)) == NULL) {
		delete_object(instance, obj);
		nomem();
		return NULL;
	}
	return obj;
}

static struct alisp_object * new_pointer(struct alisp_instance *instance, const void *ptr)
{
	struct alisp_object * obj;
	
	obj = search_object_pointer(instance, ptr);
	if (obj != NULL)
		return obj;
	obj = new_object(instance, ALISP_OBJ_POINTER);
	if (obj) {
		list_add(&obj->list, &instance->used_objs_list[(long)ptr & ALISP_OBJ_PAIR_HASH_MASK][ALISP_OBJ_POINTER]);
		obj->value.ptr = ptr;
	}
	return obj;
}

static struct alisp_object * new_cons_pointer(struct alisp_instance * instance, const char *ptr_id, void *ptr)
{
	struct alisp_object * lexpr;

	if (ptr == NULL)
		return &alsa_lisp_nil;
	lexpr = new_object(instance, ALISP_OBJ_CONS);
	if (lexpr == NULL)
		return NULL;
	lexpr->value.c.car = new_string(instance, ptr_id);
	if (lexpr->value.c.car == NULL)
		goto __end;
	lexpr->value.c.cdr = new_pointer(instance, ptr);
	if (lexpr->value.c.cdr == NULL) {
		delete_object(instance, lexpr->value.c.car);
	      __end:
		delete_object(instance, lexpr);
		return NULL;
	}
	return lexpr;
}

void alsa_lisp_init_objects(void) __attribute__ ((constructor));

void alsa_lisp_init_objects(void)
{
	memset(&alsa_lisp_nil, 0, sizeof(alsa_lisp_nil));
	alisp_set_type(&alsa_lisp_nil, ALISP_OBJ_NIL);
	INIT_LIST_HEAD(&alsa_lisp_nil.list);
	memset(&alsa_lisp_t, 0, sizeof(alsa_lisp_t));
	alisp_set_type(&alsa_lisp_t, ALISP_OBJ_T);
	INIT_LIST_HEAD(&alsa_lisp_t.list);
}

/*
 * lexer
 */ 

static int xgetc(struct alisp_instance *instance)
{
	instance->charno++;
	if (instance->lex_bufp > instance->lex_buf)
		return *--(instance->lex_bufp);
	return snd_input_getc(instance->in);
}

static inline void xungetc(struct alisp_instance *instance, int c)
{
	*(instance->lex_bufp)++ = c;
	instance->charno--;
}

static int init_lex(struct alisp_instance *instance)
{
	instance->charno = instance->lineno = 1;
	instance->token_buffer_max = 10;
	if ((instance->token_buffer = (char *)malloc(instance->token_buffer_max)) == NULL) {
		nomem();
		return -ENOMEM;
	}
	instance->lex_bufp = instance->lex_buf;
	return 0;
}

static void done_lex(struct alisp_instance *instance)
{
	free(instance->token_buffer);
}

static char * extend_buf(struct alisp_instance *instance, char *p)
{
	int off = p - instance->token_buffer;

	instance->token_buffer_max += 10;
	instance->token_buffer = (char *)realloc(instance->token_buffer, instance->token_buffer_max);
	if (instance->token_buffer == NULL) {
		nomem();
		return NULL;
	}

	return instance->token_buffer + off;
}

static int gettoken(struct alisp_instance *instance)
{
	char *p;
	int c;

	for (;;) {
		c = xgetc(instance);
		switch (c) {
		case '\n':
			++instance->lineno;
			break;

		case ' ': case '\f': case '\t': case '\v': case '\r':
			break;

		case ';':
			/* Comment: ";".*"\n" */
			while ((c = xgetc(instance)) != '\n' && c != EOF)
				;
			if (c != EOF)
				++instance->lineno;
			break;

		case '?':
			/* Character: "?". */
			c = xgetc(instance);
			sprintf(instance->token_buffer, "%d", c);
			return instance->thistoken = ALISP_INTEGER;

		case '-':
			/* Minus sign: "-". */
			c = xgetc(instance);
			if (!isdigit(c)) {
				xungetc(instance, c);
				c = '-';
				goto got_id;
			}
			xungetc(instance, c);
			c = '-';
			/* FALLTRHU */

		case '0':
		case '1': case '2': case '3':
		case '4': case '5': case '6':
		case '7': case '8': case '9':
			/* Integer: [0-9]+ */
			p = instance->token_buffer;
			instance->thistoken = ALISP_INTEGER;
			do {
			      __ok:
				if (p - instance->token_buffer >= instance->token_buffer_max - 1) {
					p = extend_buf(instance, p);
					if (p == NULL)
						return instance->thistoken = EOF;
				}
				*p++ = c;
				c = xgetc(instance);
				if (c == '.' && instance->thistoken == ALISP_INTEGER) {
					c = xgetc(instance);
					xungetc(instance, c);
					if (isdigit(c)) {
						instance->thistoken = ALISP_FLOAT;
						c = '.';
						goto __ok;
					} else {
						c = '.';
					}
				} else if (c == 'e' && instance->thistoken == ALISP_FLOAT) {
					c = xgetc(instance);
					if (isdigit(c)) {
						instance->thistoken = ALISP_FLOATE;
						goto __ok;
					}
				}
			} while (isdigit(c));
			xungetc(instance, c);
			*p = '\0';
			return instance->thistoken;

		got_id:
		case '!': case '_': case '+': case '*': case '/': case '%':
		case '<': case '>': case '=': case '&':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
			/* Identifier: [!-/+*%<>=&a-zA-Z_][-/+*%<>=&a-zA-Z_0-9]* */
			p = instance->token_buffer;
			do {
				if (p - instance->token_buffer >= instance->token_buffer_max - 1) {
					p = extend_buf(instance, p);
					if (p == NULL)
						return instance->thistoken = EOF;
				}
				*p++ = c;
				c = xgetc(instance);
			} while (isalnum(c) || strchr("!_-+*/%<>=&", c) != NULL);
			xungetc(instance, c);
			*p = '\0';
			return instance->thistoken = ALISP_IDENTIFIER;

		case '"':
			/* String: "\""([^"]|"\\".)*"\"" */
			p = instance->token_buffer;
			while ((c = xgetc(instance)) != '"' && c != EOF) {
				if (p - instance->token_buffer >= instance->token_buffer_max - 1) {
					p = extend_buf(instance, p);
					if (p == NULL)
						return instance->thistoken = EOF;
				}
				if (c == '\\') {
					c = xgetc(instance);
					switch (c) {
					case '\n': ++instance->lineno; break;
					case 'a': *p++ = '\a'; break;
					case 'b': *p++ = '\b'; break;
					case 'f': *p++ = '\f'; break;
					case 'n': *p++ = '\n'; break;
					case 'r': *p++ = '\r'; break;
					case 't': *p++ = '\t'; break;
					case 'v': *p++ = '\v'; break;
					default: *p++ = c;
					}
				} else {
					if (c == '\n')
						++instance->lineno;
					*p++ = c;
				}
			}
			*p = '\0';
			return instance->thistoken = ALISP_STRING;

		default:
			return instance->thistoken = c;
		}
	}
}

/*
 *  parser
 */

static struct alisp_object * parse_form(struct alisp_instance *instance)
{
	int thistoken;
	struct alisp_object * p, * first = NULL, * prev = NULL;

	while ((thistoken = gettoken(instance)) != ')' && thistoken != EOF) {
		/*
		 * Parse a dotted pair notation.
		 */
		if (thistoken == '.') {
			gettoken(instance);
			if (prev == NULL) {
				lisp_error(instance, "unexpected '.'");
			      __err:
				delete_tree(instance, first);
				return NULL;
			}
			prev->value.c.cdr = parse_object(instance, 1);
			if (prev->value.c.cdr == NULL)
				goto __err;
			if ((thistoken = gettoken(instance)) != ')') {
				lisp_error(instance, "expected ')'");
				goto __err;
			}
			break;
		}

		p = new_object(instance, ALISP_OBJ_CONS);
		if (p == NULL)
			goto __err;

		if (first == NULL)
			first = p;
		if (prev != NULL)
			prev->value.c.cdr = p;

		p->value.c.car = parse_object(instance, 1);
		if (p->value.c.car == NULL)
			goto __err;

		prev = p;
	}

	if (first == NULL)
		return &alsa_lisp_nil;
	else
		return first;
}

static struct alisp_object * quote_object(struct alisp_instance *instance, struct alisp_object * obj)
{
	struct alisp_object * p;

	if (obj == NULL)
		goto __end1;

	p = new_object(instance, ALISP_OBJ_CONS);
	if (p == NULL)
		goto __end1;

	p->value.c.car = new_identifier(instance, "quote");
	if (p->value.c.car == NULL)
		goto __end;
	p->value.c.cdr = new_object(instance, ALISP_OBJ_CONS);
	if (p->value.c.cdr == NULL) {
		delete_object(instance, p->value.c.car);
	      __end:
		delete_object(instance, p);
	      __end1:
		delete_tree(instance, obj);
		return NULL;
	}

	p->value.c.cdr->value.c.car = obj;
	return p;
}

static inline struct alisp_object * parse_quote(struct alisp_instance *instance)
{
	return quote_object(instance, parse_object(instance, 0));
}

static struct alisp_object * parse_object(struct alisp_instance *instance, int havetoken)
{
	int thistoken;
	struct alisp_object * p = NULL;

	if (!havetoken)
		thistoken = gettoken(instance);
	else
		thistoken = instance->thistoken;

	switch (thistoken) {
	case EOF:
		break;
	case '(':
		p = parse_form(instance);
		break;
	case '\'':
		p = parse_quote(instance);
		break;
	case ALISP_IDENTIFIER:
		if (!strcmp(instance->token_buffer, "t"))
			p = &alsa_lisp_t;
		else if (!strcmp(instance->token_buffer, "nil"))
			p = &alsa_lisp_nil;
		else {
			p = new_identifier(instance, instance->token_buffer);
		}
		break;
	case ALISP_INTEGER: {
		p = new_integer(instance, atol(instance->token_buffer));
		break;
	}
	case ALISP_FLOAT:
	case ALISP_FLOATE: {
		p = new_float(instance, atof(instance->token_buffer));
		break;
	}
	case ALISP_STRING:
		p = new_string(instance, instance->token_buffer);
		break;
	default:
		lisp_warn(instance, "%d:%d: unexpected character `%c'", instance->lineno, instance->charno, thistoken);
		break;
	}

	return p;
}

/*
 *  object manipulation
 */

static struct alisp_object_pair * set_object_direct(struct alisp_instance *instance, struct alisp_object * name, struct alisp_object * value)
{
	struct alisp_object_pair *p;
	const char *id;

	id = name->value.s;
	p = (struct alisp_object_pair *)malloc(sizeof(struct alisp_object_pair));
	if (p == NULL) {
		nomem();
		return NULL;
	}
	p->name = strdup(id);
	if (p->name == NULL) {
		delete_tree(instance, value);
		free(p);
		return NULL;
	}
	list_add(&p->list, &instance->setobjs_list[get_string_hash(id)]);
	p->value = value;
	return p;
}

static int check_set_object(struct alisp_instance * instance, struct alisp_object * name)
{
	if (name == &alsa_lisp_nil) {
		lisp_warn(instance, "setting the value of a nil object");
		return 0;
	}
	if (name == &alsa_lisp_t) {
		lisp_warn(instance, "setting the value of a t object");
		return 0;
	}
	if (!alisp_compare_type(name, ALISP_OBJ_IDENTIFIER) &&
	    !alisp_compare_type(name, ALISP_OBJ_STRING)) {
		lisp_warn(instance, "setting the value of an object with non-indentifier");
		return 0;
	}
	return 1;
}

static struct alisp_object_pair * set_object(struct alisp_instance *instance, struct alisp_object * name, struct alisp_object * value)
{
	struct list_head *pos;
	struct alisp_object_pair *p;
	const char *id;

	if (name == NULL || value == NULL)
		return NULL;

	id = name->value.s;

	list_for_each(pos, &instance->setobjs_list[get_string_hash(id)]) {
		p = list_entry(pos, struct alisp_object_pair, list);
		if (!strcmp(p->name, id)) {
			delete_tree(instance, p->value);
			p->value = value;
			return p;
		}
	}

	p = (struct alisp_object_pair *)malloc(sizeof(struct alisp_object_pair));
	if (p == NULL) {
		nomem();
		return NULL;
	}
	p->name = strdup(id);
	if (p->name == NULL) {
		delete_tree(instance, value);
		free(p);
		return NULL;
	}
	list_add(&p->list, &instance->setobjs_list[get_string_hash(id)]);
	p->value = value;
	return p;
}

static struct alisp_object * unset_object(struct alisp_instance *instance, struct alisp_object * name)
{
	struct list_head *pos;
	struct alisp_object *res;
	struct alisp_object_pair *p;
	const char *id;
	
	if (!alisp_compare_type(name, ALISP_OBJ_IDENTIFIER) &&
	    !alisp_compare_type(name, ALISP_OBJ_STRING)) {
	    	lisp_warn(instance, "unset object with a non-indentifier");
		return &alsa_lisp_nil;
	}
	id = name->value.s;

	list_for_each(pos, &instance->setobjs_list[get_string_hash(id)]) {
		p = list_entry(pos, struct alisp_object_pair, list);
		if (!strcmp(p->name, id)) {
			list_del(&p->list);
			res = p->value;
			free((void *)p->name);
			free(p);
			return res;
		}
	}
	
	return &alsa_lisp_nil;
}

static struct alisp_object * get_object1(struct alisp_instance *instance, const char *id)
{
	struct alisp_object_pair *p;
	struct list_head *pos;

	list_for_each(pos, &instance->setobjs_list[get_string_hash(id)]) {
		p = list_entry(pos, struct alisp_object_pair, list);
		if (!strcmp(p->name, id))
			return p->value;
	}

	return &alsa_lisp_nil;
}

static struct alisp_object * get_object(struct alisp_instance *instance, struct alisp_object * name)
{
	if (!alisp_compare_type(name, ALISP_OBJ_IDENTIFIER) &&
	    !alisp_compare_type(name, ALISP_OBJ_STRING)) {
	    	delete_tree(instance, name);
		return &alsa_lisp_nil;
	}
	return get_object1(instance, name->value.s);
}

static struct alisp_object * replace_object(struct alisp_instance *instance, struct alisp_object * name, struct alisp_object * onew)
{
	struct alisp_object_pair *p;
	struct alisp_object *r;
	struct list_head *pos;
	const char *id;

	if (!alisp_compare_type(name, ALISP_OBJ_IDENTIFIER) &&
	    !alisp_compare_type(name, ALISP_OBJ_STRING)) {
	    	delete_tree(instance, name);
		return &alsa_lisp_nil;
	}
	id = name->value.s;
	list_for_each(pos, &instance->setobjs_list[get_string_hash(id)]) {
		p = list_entry(pos, struct alisp_object_pair, list);
		if (!strcmp(p->name, id)) {
			r = p->value;
			p->value = onew;
			return r;
		}
	}

	return NULL;
}

static void dump_objects(struct alisp_instance *instance, const char *fname)
{
	struct alisp_object_pair *p;
	snd_output_t *out;
	struct list_head *pos;
	int i, err;

	if (!strcmp(fname, "-"))
		err = snd_output_stdio_attach(&out, stdout, 0);
	else
		err = snd_output_stdio_open(&out, fname, "w+");
	if (err < 0) {
		SNDERR("alisp: cannot open file '%s' for writting (%s)", fname, snd_strerror(errno));
		return;
	}

	for (i = 0; i < ALISP_OBJ_PAIR_HASH_SIZE; i++) {
		list_for_each(pos, &instance->setobjs_list[i]) {
			p = list_entry(pos, struct alisp_object_pair, list);
			if (alisp_compare_type(p->value, ALISP_OBJ_CONS) &&
			    alisp_compare_type(p->value->value.c.car, ALISP_OBJ_IDENTIFIER) &&
			    !strcmp(p->value->value.c.car->value.s, "lambda")) {
			    	snd_output_printf(out, "(defun %s ", p->name);
			    	princ_cons(out, p->value->value.c.cdr);
			    	snd_output_printf(out, ")\n");
			    	continue;
			}
			snd_output_printf(out, "(setq %s '", p->name);
 			princ_object(out, p->value);
			snd_output_printf(out, ")\n");
		}
	}
	snd_output_close(out);
}

static const char *obj_type_str(struct alisp_object * p)
{
	switch (alisp_get_type(p)) {
	case ALISP_OBJ_NIL: return "nil";
	case ALISP_OBJ_T: return "t";
	case ALISP_OBJ_INTEGER: return "integer";
	case ALISP_OBJ_FLOAT: return "float";
	case ALISP_OBJ_IDENTIFIER: return "identifier";
	case ALISP_OBJ_STRING: return "string";
	case ALISP_OBJ_POINTER: return "pointer";
	case ALISP_OBJ_CONS: return "cons";
	default: assert(0);
	}
}

static void print_obj_lists(struct alisp_instance *instance, snd_output_t *out)
{
	struct list_head *pos;
	struct alisp_object * p;
	int i, j;

	snd_output_printf(out, "** used objects\n");
	for (i = 0; i < ALISP_OBJ_PAIR_HASH_SIZE; i++)
		for (j = 0; j <= ALISP_OBJ_LAST_SEARCH; j++)
			list_for_each(pos, &instance->used_objs_list[i][j]) {
				p = list_entry(pos, struct alisp_object, list);
				snd_output_printf(out, "**   %p (%s) (", p, obj_type_str(p));
				if (!alisp_compare_type(p, ALISP_OBJ_CONS))
					princ_object(out, p);
				else
					snd_output_printf(out, "cons");
				snd_output_printf(out, ") refs=%i\n", alisp_get_refs(p));
			}
	snd_output_printf(out, "** free objects\n");
	list_for_each(pos, &instance->free_objs_list) {
		p = list_entry(pos, struct alisp_object, list);
		snd_output_printf(out, "**   %p\n", p);
	}
}

static void dump_obj_lists(struct alisp_instance *instance, const char *fname)
{
	snd_output_t *out;
	int err;

	if (!strcmp(fname, "-"))
		err = snd_output_stdio_attach(&out, stdout, 0);
	else
		err = snd_output_stdio_open(&out, fname, "w+");
	if (err < 0) {
		SNDERR("alisp: cannot open file '%s' for writting (%s)", fname, snd_strerror(errno));
		return;
	}

	print_obj_lists(instance, out);

	snd_output_close(out);
}

/*
 *  functions
 */

static int count_list(struct alisp_object * p)
{
	int i = 0;

	while (p != &alsa_lisp_nil && alisp_compare_type(p, ALISP_OBJ_CONS)) {
		p = p->value.c.cdr;
		++i;
	}

	return i;
}

static inline struct alisp_object * car(struct alisp_object * p)
{
	if (alisp_compare_type(p, ALISP_OBJ_CONS))
		return p->value.c.car;

	return &alsa_lisp_nil;
}

static inline struct alisp_object * cdr(struct alisp_object * p)
{
	if (alisp_compare_type(p, ALISP_OBJ_CONS))
		return p->value.c.cdr;

	return &alsa_lisp_nil;
}

/*
 * Syntax: (car expr)
 */
static struct alisp_object * F_car(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object *p1 = car(args), *p2;
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	p1 = eval(instance, p1);
	delete_tree(instance, cdr(p1));
	p2 = car(p1);
	delete_object(instance, p1);
	return p2;
}

/*
 * Syntax: (cdr expr)
 */
static struct alisp_object * F_cdr(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object *p1 = car(args), *p2;
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	p1 = eval(instance, p1);
	delete_tree(instance, car(p1));
	p2 = cdr(p1);
	delete_object(instance, p1);
	return p2;
}

/*
 * Syntax: (+ expr...)
 */
static struct alisp_object * F_add(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1, * n;
	long v = 0;
	double f = 0;
	int type = ALISP_OBJ_INTEGER;

	p1 = eval(instance, car(p));
	for (;;) {
		if (alisp_compare_type(p1, ALISP_OBJ_INTEGER)) {
			if (type == ALISP_OBJ_FLOAT)
				f += p1->value.i;
			else
				v += p1->value.i;
		} else if (alisp_compare_type(p1, ALISP_OBJ_FLOAT)) {
			f += p1->value.f + v;
			v = 0;
			type = ALISP_OBJ_FLOAT;
		} else {
			lisp_warn(instance, "sum with a non integer or float operand");
		}
		delete_tree(instance, p1);
		p = cdr(n = p);
		delete_object(instance, n);
		if (p == &alsa_lisp_nil)
			break;
		p1 = eval(instance, car(p));
	}
	if (type == ALISP_OBJ_INTEGER) {
		return new_integer(instance, v);
	} else {
		return new_float(instance, f);
	}
}

/*
 * Syntax: (concat expr...)
 */
static struct alisp_object * F_concat(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1, * n;
	char *str = NULL, *str1;
	
	p1 = eval(instance, car(p));
	for (;;) {
		if (alisp_compare_type(p1, ALISP_OBJ_STRING)) {
			str1 = realloc(str, (str ? strlen(str) : 0) + strlen(p1->value.s) + 1);
			if (str1 == NULL) {
				nomem();
				free(str);
				return NULL;
			}
			if (str == NULL)
				strcpy(str1, p1->value.s);
			else
				strcat(str1, p1->value.s);
			str = str1;
		} else {
			lisp_warn(instance, "concat with a non string or identifier operand");
		}
		delete_tree(instance, p1);
		p = cdr(n = p);
		delete_object(instance, n);
		if (p == &alsa_lisp_nil)
			break;
		p1 = eval(instance, car(p));
	}
	if (str) {
		p = new_string(instance, str);
		free(str);
	} else {
		p = &alsa_lisp_nil;
	}
	return p;
}

/*
 * Syntax: (- expr...)
 */
static struct alisp_object * F_sub(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1, * n;
	long v = 0;
	double f = 0;
	int type = ALISP_OBJ_INTEGER;

	do {
		p1 = eval(instance, car(p));
		if (alisp_compare_type(p1, ALISP_OBJ_INTEGER)) {
			if (p == args && cdr(p) != &alsa_lisp_nil) {
				v = p1->value.i;
			} else {
				if (type == ALISP_OBJ_FLOAT)
					f -= p1->value.i;
				else
					v -= p1->value.i;
			}
		} else if (alisp_compare_type(p1, ALISP_OBJ_FLOAT)) {
			if (type == ALISP_OBJ_INTEGER) {
				f = v;
				type = ALISP_OBJ_FLOAT;
			}
			if (p == args && cdr(p) != &alsa_lisp_nil)
				f = p1->value.f;
			else {
				f -= p1->value.f;
			}
		} else
			lisp_warn(instance, "difference with a non integer or float operand");
		delete_tree(instance, p1);
		n = cdr(p);
		delete_object(instance, p);
		p = n;
	} while (p != &alsa_lisp_nil);

	if (type == ALISP_OBJ_INTEGER) {
		return new_integer(instance, v);
	} else {
		return new_float(instance, f);
	}
}

/*
 * Syntax: (* expr...)
 */
static struct alisp_object * F_mul(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1, * n;
	long v = 1;
	double f = 1;
	int type = ALISP_OBJ_INTEGER;

	do {
		p1 = eval(instance, car(p));
		if (alisp_compare_type(p1, ALISP_OBJ_INTEGER)) {
			if (type == ALISP_OBJ_FLOAT)
				f *= p1->value.i;
			else
				v *= p1->value.i;
		} else if (alisp_compare_type(p1, ALISP_OBJ_FLOAT)) {
			f *= p1->value.f * v; v = 1;
			type = ALISP_OBJ_FLOAT;
		} else {
			lisp_warn(instance, "product with a non integer or float operand");
		}
		delete_tree(instance, p1);
		n = cdr(p);
		delete_object(instance, p);
		p = n;
	} while (p != &alsa_lisp_nil);

	if (type == ALISP_OBJ_INTEGER) {
		return new_integer(instance, v);
	} else {
		return new_float(instance, f);
	}
}

/*
 * Syntax: (/ expr...)
 */
static struct alisp_object * F_div(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1, * n;
	long v = 0;
	double f = 0;
	int type = ALISP_OBJ_INTEGER;

	do {
		p1 = eval(instance, car(p));
		if (alisp_compare_type(p1, ALISP_OBJ_INTEGER)) {
			if (p == args && cdr(p) != &alsa_lisp_nil) {
				v = p1->value.i;
			} else {
				if (p1->value.i == 0) {
					lisp_warn(instance, "division by zero");
					v = 0;
					f = 0;
					break;
				} else {
					if (type == ALISP_OBJ_FLOAT)
						f /= p1->value.i;
					else
						v /= p1->value.i;
				}
			}
		} else if (alisp_compare_type(p1, ALISP_OBJ_FLOAT)) {
			if (type == ALISP_OBJ_INTEGER) {
				f = v;
				type = ALISP_OBJ_FLOAT;
			}
			if (p == args && cdr(p) != &alsa_lisp_nil) {
				f = p1->value.f;
			} else {
				if (p1->value.f == 0) {
					lisp_warn(instance, "division by zero");
					f = 0;
					break;
				} else {
					f /= p1->value.i;
				}
			}
		} else
			lisp_warn(instance, "quotient with a non integer or float operand");
		delete_tree(instance, p1);
		n = cdr(p);
		delete_object(instance, p);
		p = n;
	} while (p != &alsa_lisp_nil);

	if (type == ALISP_OBJ_INTEGER) {
		return new_integer(instance, v);
	} else {
		return new_float(instance, f);
	}
}

/*
 * Syntax: (% expr1 expr2)
 */
static struct alisp_object * F_mod(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * p3;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (alisp_compare_type(p1, ALISP_OBJ_INTEGER) &&
	    alisp_compare_type(p2, ALISP_OBJ_INTEGER)) {
		if (p2->value.i == 0) {
			lisp_warn(instance, "module by zero");
			p3 = new_integer(instance, 0);
		} else {
			p3 = new_integer(instance, p1->value.i % p2->value.i);
		}
	} else if ((alisp_compare_type(p1, ALISP_OBJ_INTEGER) || 
	            alisp_compare_type(p1, ALISP_OBJ_FLOAT)) &&
		   (alisp_compare_type(p2, ALISP_OBJ_INTEGER) ||
		    alisp_compare_type(p2, ALISP_OBJ_FLOAT))) {
		double f1, f2;
		f1 = alisp_compare_type(p1, ALISP_OBJ_INTEGER) ? p1->value.i : p1->value.f;
		f2 = alisp_compare_type(p2, ALISP_OBJ_INTEGER) ? p2->value.i : p2->value.f;
		f1 = fmod(f1, f2);
		if (f1 == EDOM) {
			lisp_warn(instance, "module by zero");
			p3 = new_float(instance, 0);
		} else {
			p3 = new_float(instance, f1);
		}
	} else {
		lisp_warn(instance, "module with a non integer or float operand");
		delete_tree(instance, p1);
		delete_tree(instance, p2);
		return &alsa_lisp_nil;
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return p3;
}

/*
 * Syntax: (< expr1 expr2)
 */
static struct alisp_object * F_lt(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (alisp_compare_type(p1, ALISP_OBJ_INTEGER) &&
	    alisp_compare_type(p2, ALISP_OBJ_INTEGER)) {
		if (p1->value.i < p2->value.i) {
		      __true:
			delete_tree(instance, p1);
			delete_tree(instance, p2);
			return &alsa_lisp_t;
		}
	} else if ((alisp_compare_type(p1, ALISP_OBJ_INTEGER) ||
	            alisp_compare_type(p1, ALISP_OBJ_FLOAT)) &&
		   (alisp_compare_type(p2, ALISP_OBJ_INTEGER) ||
		    alisp_compare_type(p2, ALISP_OBJ_FLOAT))) {
		double f1, f2;
		f1 = alisp_compare_type(p1, ALISP_OBJ_INTEGER) ? p1->value.i : p1->value.f;
		f2 = alisp_compare_type(p2, ALISP_OBJ_INTEGER) ? p2->value.i : p2->value.f;
		if (f1 < f2)
			goto __true;
	} else {
		lisp_warn(instance, "comparison with a non integer or float operand");
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (> expr1 expr2)
 */
static struct alisp_object * F_gt(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (alisp_compare_type(p1, ALISP_OBJ_INTEGER) &&
	    alisp_compare_type(p2, ALISP_OBJ_INTEGER)) {
		if (p1->value.i > p2->value.i) {
		      __true:
			delete_tree(instance, p1);
			delete_tree(instance, p2);
			return &alsa_lisp_t;
		}
	} else if ((alisp_compare_type(p1, ALISP_OBJ_INTEGER) ||
	            alisp_compare_type(p1, ALISP_OBJ_FLOAT)) &&
		   (alisp_compare_type(p2, ALISP_OBJ_INTEGER) ||
		    alisp_compare_type(p2, ALISP_OBJ_FLOAT))) {
		double f1, f2;
		f1 = alisp_compare_type(p1, ALISP_OBJ_INTEGER) ? p1->value.i : p1->value.f;
		f2 = alisp_compare_type(p2, ALISP_OBJ_INTEGER) ? p2->value.i : p2->value.f;
		if (f1 > f2)
			goto __true;
	} else {
		lisp_warn(instance, "comparison with a non integer or float operand");
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (<= expr1 expr2)
 */
static struct alisp_object * F_le(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (alisp_compare_type(p1, ALISP_OBJ_INTEGER) &&
	    alisp_compare_type(p2, ALISP_OBJ_INTEGER)) {
		if (p1->value.i <= p2->value.i) {
		      __true:
			delete_tree(instance, p1);
			delete_tree(instance, p2);
			return &alsa_lisp_t;
		}
	} else if ((alisp_compare_type(p1, ALISP_OBJ_INTEGER) ||
	            alisp_compare_type(p1, ALISP_OBJ_FLOAT)) &&
		   (alisp_compare_type(p2, ALISP_OBJ_INTEGER) ||
		    alisp_compare_type(p2, ALISP_OBJ_FLOAT))) {
		double f1, f2;
		f1 = alisp_compare_type(p1, ALISP_OBJ_INTEGER) ? p1->value.i : p1->value.f;
		f2 = alisp_compare_type(p2, ALISP_OBJ_INTEGER) ? p2->value.i : p2->value.f;
		if (f1 <= f2)
			goto __true;
	} else {
		lisp_warn(instance, "comparison with a non integer or float operand");
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (>= expr1 expr2)
 */
static struct alisp_object * F_ge(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (alisp_compare_type(p1, ALISP_OBJ_INTEGER) &&
	    alisp_compare_type(p2, ALISP_OBJ_INTEGER)) {
		if (p1->value.i >= p2->value.i) {
		      __true:
			delete_tree(instance, p1);
			delete_tree(instance, p2);
			return &alsa_lisp_t;
		}
	} else if ((alisp_compare_type(p1, ALISP_OBJ_INTEGER) ||
	            alisp_compare_type(p1, ALISP_OBJ_FLOAT)) &&
		   (alisp_compare_type(p2, ALISP_OBJ_INTEGER) ||
		    alisp_compare_type(p2, ALISP_OBJ_FLOAT))) {
		double f1, f2;
		f1 = alisp_compare_type(p1, ALISP_OBJ_INTEGER) ? p1->value.i : p1->value.f;
		f2 = alisp_compare_type(p2, ALISP_OBJ_INTEGER) ? p2->value.i : p2->value.f;
		if (f1 >= f2)
			goto __true;
	} else {
		lisp_warn(instance, "comparison with a non integer or float operand");
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (= expr1 expr2)
 */
static struct alisp_object * F_numeq(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (alisp_compare_type(p1, ALISP_OBJ_INTEGER) &&
	    alisp_compare_type(p2, ALISP_OBJ_INTEGER)) {
		if (p1->value.i == p2->value.i) {
		      __true:
			delete_tree(instance, p1);
			delete_tree(instance, p2);
			return &alsa_lisp_t;
		}
	} else if ((alisp_compare_type(p1, ALISP_OBJ_INTEGER) ||
	            alisp_compare_type(p1, ALISP_OBJ_FLOAT)) &&
		   (alisp_compare_type(p2, ALISP_OBJ_INTEGER) ||
		    alisp_compare_type(p2, ALISP_OBJ_FLOAT))) {
		double f1, f2;
		f1 = alisp_compare_type(p1, ALISP_OBJ_INTEGER) ? p1->value.i : p1->value.f;
		f2 = alisp_compare_type(p2, ALISP_OBJ_INTEGER) ? p2->value.i : p2->value.f;
		if (f1 == f2)
			goto __true;
	} else {
		lisp_warn(instance, "comparison with a non integer or float operand");
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (!= expr1 expr2)
 */
static struct alisp_object * F_numneq(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p;
	
	p = F_numeq(instance, args);
	if (p == &alsa_lisp_nil)
		return &alsa_lisp_t;
	return &alsa_lisp_nil;
}

/*
 * Syntax: (exfun name)
 * Test, if a function exists
 */
static struct alisp_object * F_exfun(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	p2 = get_object(instance, p1);
	if (p2 == &alsa_lisp_nil) {
		delete_tree(instance, p1);
		return &alsa_lisp_nil;
	}
	p2 = car(p2);
	if (alisp_compare_type(p2, ALISP_OBJ_IDENTIFIER) &&
	    !strcmp(p2->value.s, "lambda")) {
		delete_tree(instance, p1);
		return &alsa_lisp_t;
	}
	delete_tree(instance, p1);
	return &alsa_lisp_nil;
}

static void princ_string(snd_output_t *out, char *s)
{
	char *p;

	snd_output_putc(out, '"');
	for (p = s; *p != '\0'; ++p)
		switch (*p) {
		case '\a': snd_output_putc(out, '\\'); snd_output_putc(out, 'a'); break;
		case '\b': snd_output_putc(out, '\\'); snd_output_putc(out, 'b'); break;
		case '\f': snd_output_putc(out, '\\'); snd_output_putc(out, 'f'); break;
		case '\n': snd_output_putc(out, '\\'); snd_output_putc(out, 'n'); break;
		case '\r': snd_output_putc(out, '\\'); snd_output_putc(out, 'r'); break;
		case '\t': snd_output_putc(out, '\\'); snd_output_putc(out, 't'); break;
		case '\v': snd_output_putc(out, '\\'); snd_output_putc(out, 'v'); break;
		case '"': snd_output_putc(out, '\\'); snd_output_putc(out, '"'); break;
		default: snd_output_putc(out, *p);
		}
	snd_output_putc(out, '"');
}

static void princ_cons(snd_output_t *out, struct alisp_object * p)
{
	do {
		princ_object(out, p->value.c.car);
		p = p->value.c.cdr;
		if (p != &alsa_lisp_nil) {
			snd_output_putc(out, ' ');
			if (!alisp_compare_type(p, ALISP_OBJ_CONS)) {
				snd_output_printf(out, ". ");
				princ_object(out, p);
			}
		}
	} while (p != &alsa_lisp_nil && alisp_compare_type(p, ALISP_OBJ_CONS));
}

static void princ_object(snd_output_t *out, struct alisp_object * p)
{
	switch (alisp_get_type(p)) {
	case ALISP_OBJ_NIL:
		snd_output_printf(out, "nil");
		break;
	case ALISP_OBJ_T:
		snd_output_putc(out, 't');
		break;
	case ALISP_OBJ_IDENTIFIER:
		snd_output_printf(out, "%s", p->value.s);
		break;
	case ALISP_OBJ_STRING:
		princ_string(out, p->value.s);
		break;
	case ALISP_OBJ_INTEGER:
		snd_output_printf(out, "%ld", p->value.i);
		break;
	case ALISP_OBJ_FLOAT:
		snd_output_printf(out, "%f", p->value.f);
		break;
	case ALISP_OBJ_POINTER:
		snd_output_printf(out, "<%p>", p->value.ptr);
		break;
	case ALISP_OBJ_CONS:
		snd_output_putc(out, '(');
		princ_cons(out, p);
		snd_output_putc(out, ')');
	}
}

/*
 * Syntax: (princ expr...)
 */
static struct alisp_object * F_princ(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1 = NULL, * n;

	do {
		if (p1)
			delete_tree(instance, p1);
		p1 = eval(instance, car(p));
		if (alisp_compare_type(p1, ALISP_OBJ_STRING))
			snd_output_printf(instance->out, p1->value.s);
		else
			princ_object(instance->out, p1);
		n = cdr(p);
		delete_object(instance, p);
		p = n;
	} while (p != &alsa_lisp_nil);

	return p1;
}

/*
 * Syntax: (atom expr)
 */
static struct alisp_object * F_atom(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p;

	p = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (p == NULL)
		return NULL;

	switch (alisp_get_type(p)) {
	case ALISP_OBJ_T:
	case ALISP_OBJ_NIL:
	case ALISP_OBJ_INTEGER:
	case ALISP_OBJ_FLOAT:
	case ALISP_OBJ_STRING:
	case ALISP_OBJ_IDENTIFIER:
	case ALISP_OBJ_POINTER:
		delete_tree(instance, p);
		return &alsa_lisp_t;
	default:
		break;
	}

	delete_tree(instance, p);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (cons expr1 expr2)
 */
static struct alisp_object * F_cons(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p;

	p = new_object(instance, ALISP_OBJ_CONS);
	if (p) {
		p->value.c.car = eval(instance, car(args));
		p->value.c.cdr = eval(instance, car(cdr(args)));
		delete_tree(instance, cdr(cdr(args)));
		delete_object(instance, cdr(args));
		delete_object(instance, args);
	} else {
		delete_tree(instance, args);
	}

	return p;
}

/*
 * Syntax: (list expr1...)
 */
static struct alisp_object * F_list(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * first = NULL, * prev = NULL, * p1;

	if (p == &alsa_lisp_nil)
		return &alsa_lisp_nil;

	do {
		p1 = new_object(instance, ALISP_OBJ_CONS);
		if (p1 == NULL) {
			delete_tree(instance, p);
			delete_tree(instance, first);
			return NULL;
		}
		p1->value.c.car = eval(instance, car(p));
		if (p1->value.c.car == NULL) {
			delete_tree(instance, first);
			delete_tree(instance, cdr(p));
			delete_object(instance, p);
			return NULL;
		}
		if (first == NULL)
			first = p1;
		if (prev != NULL)
			prev->value.c.cdr = p1;
		prev = p1;
		p = cdr(p1 = p);
		delete_object(instance, p1);
	} while (p != &alsa_lisp_nil);

	return first;
}

static inline int eq(struct alisp_object * p1, struct alisp_object * p2)
{
	return p1 == p2;
}

static int equal(struct alisp_object * p1, struct alisp_object * p2)
{
	int type1, type2;

	if (eq(p1, p2))
		return 1;

	type1 = alisp_get_type(p1);
	type2 = alisp_get_type(p2);

	if (type1 == ALISP_OBJ_CONS || type2 == ALISP_OBJ_CONS)
		return 0;

	if (type1 == type2) {
		switch (type1) {
		case ALISP_OBJ_STRING:
			return !strcmp(p1->value.s, p2->value.s);
		case ALISP_OBJ_INTEGER:
			return p1->value.i == p2->value.i;
		case ALISP_OBJ_FLOAT:
			return p1->value.i == p2->value.i;
		}
	}

	return 0;
}

/*
 * Syntax: (eq expr1 expr2)
 */
static struct alisp_object * F_eq(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (eq(p1, p2)) {
		delete_tree(instance, p1);
		delete_tree(instance, p2);
		return &alsa_lisp_t;
	}
	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (equal expr1 expr2)
 */
static struct alisp_object * F_equal(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (equal(p1, p2)) {
		delete_tree(instance, p1);
		delete_tree(instance, p2);
		return &alsa_lisp_t;
	}
	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (quote expr)
 */
static struct alisp_object * F_quote(struct alisp_instance *instance ATTRIBUTE_UNUSED, struct alisp_object * args)
{
	struct alisp_object *p = car(args);
	
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	return p;
}

/*
 * Syntax: (and expr...)
 */
static struct alisp_object * F_and(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1 = NULL, * n;

	do {
		if (p1)
			delete_tree(instance, p1);
		p1 = eval(instance, car(p));
		if (p1 == &alsa_lisp_nil) {
			delete_tree(instance, p1);
			delete_tree(instance, cdr(p));
			delete_object(instance, p);
			return &alsa_lisp_nil;
		}
		p = cdr(n = p);
		delete_object(instance, n);
	} while (p != &alsa_lisp_nil);

	return p1;
}

/*
 * Syntax: (or expr...)
 */
static struct alisp_object * F_or(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1 = NULL, * n;

	do {
		if (p1)
			delete_tree(instance, p1);
		p1 = eval(instance, car(p));
		if (p1 != &alsa_lisp_nil) {
			delete_tree(instance, cdr(p));
			delete_object(instance, p);
			return p1;
		}
		p = cdr(n = p);
		delete_object(instance, n);
	} while (p != &alsa_lisp_nil);

	return &alsa_lisp_nil;
}

/*
 * Syntax: (not expr)
 * Syntax: (null expr)
 */
static struct alisp_object * F_not(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = eval(instance, car(args));

	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (p != &alsa_lisp_nil) {
		delete_tree(instance, p);
		return &alsa_lisp_nil;
	}

	delete_tree(instance, p);
	return &alsa_lisp_t;
}

/*
 * Syntax: (cond (expr1 [expr2])...)
 */
static struct alisp_object * F_cond(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1, * p2, * p3;

	do {
		p1 = car(p);
		if ((p2 = eval(instance, car(p1))) != &alsa_lisp_nil) {
			p3 = cdr(p1);
			delete_object(instance, p1);
			delete_tree(instance, cdr(p));
			delete_object(instance, p);
			if (p3 != &alsa_lisp_nil) {
				delete_tree(instance, p2);
				return F_progn(instance, p3);
			} else {
				delete_tree(instance, p3);
				return p2;
			}
		} else {
			delete_tree(instance, p2);
			delete_tree(instance, cdr(p1));
			delete_object(instance, p1);
		}
		p = cdr(p2 = p);
		delete_object(instance, p2);
	} while (p != &alsa_lisp_nil);

	return &alsa_lisp_nil;
}

/*
 * Syntax: (if expr then-expr else-expr...)
 */
static struct alisp_object * F_if(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * p3;

	p1 = car(args);
	p2 = car(cdr(args));
	p3 = cdr(cdr(args));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	p1 = eval(instance, p1);
	if (p1 != &alsa_lisp_nil) {
		delete_tree(instance, p1);
		delete_tree(instance, p3);
		return eval(instance, p2);
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return F_progn(instance, p3);
}

/*
 * Syntax: (when expr then-expr...)
 */
static struct alisp_object * F_when(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = car(args);
	p2 = cdr(args);
	delete_object(instance, args);
	if ((p1 = eval(instance, p1)) != &alsa_lisp_nil) {
		delete_tree(instance, p1);
		return F_progn(instance, p2);
	} else {
		delete_tree(instance, p1);
		delete_tree(instance, p2);
	}

	return &alsa_lisp_nil;
}

/*
 * Syntax: (unless expr else-expr...)
 */
static struct alisp_object * F_unless(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2;

	p1 = car(args);
	p2 = cdr(args);
	delete_object(instance, args);
	if ((p1 = eval(instance, p1)) == &alsa_lisp_nil) {
		return F_progn(instance, p2);
	} else {
		delete_tree(instance, p1);
		delete_tree(instance, p2);
	}

	return &alsa_lisp_nil;
}

/*
 * Syntax: (while expr exprs...)
 */
static struct alisp_object * F_while(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * p3;

	p1 = car(args);
	p2 = cdr(args);

	delete_object(instance, args);
	while (1) {
		incref_tree(instance, p1);
		if ((p3 = eval(instance, p1)) == &alsa_lisp_nil)
			break;
		delete_tree(instance, p3);
		incref_tree(instance, p2);
		delete_tree(instance, F_progn(instance, p2));
	}

	delete_tree(instance, p1);
	delete_tree(instance, p2);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (progn expr...)
 */
static struct alisp_object * F_progn(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1 = NULL, * n;

	do {
		if (p1)
			delete_tree(instance, p1);
		p1 = eval(instance, car(p));
		n = cdr(p);
		delete_object(instance, p);
		p = n;
	} while (p != &alsa_lisp_nil);

	return p1;
}

/*
 * Syntax: (prog1 expr...)
 */
static struct alisp_object * F_prog1(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * first = NULL, * p1;

	do {
		p1 = eval(instance, car(p));
		if (first == NULL)
			first = p1;
		else
			delete_tree(instance, p1);
		p1 = cdr(p);
		delete_object(instance, p);
		p = p1;
	} while (p != &alsa_lisp_nil);

	if (first == NULL)
		first = &alsa_lisp_nil;

	return first;
}

/*
 * Syntax: (prog2 expr...)
 */
static struct alisp_object * F_prog2(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * second = NULL, * p1;
	int i = 0;

	do {
		++i;
		p1 = eval(instance, car(p));
		if (i == 2)
			second = p1;
		else
			delete_tree(instance, p1);
		p1 = cdr(p);
		delete_object(instance, p);
		p = p1;
	} while (p != &alsa_lisp_nil);

	if (second == NULL)
		second = &alsa_lisp_nil;

	return second;
}

/*
 * Syntax: (set name value)
 */
static struct alisp_object * F_set(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1 = eval(instance, car(args)),
			    * p2 = eval(instance, car(cdr(args)));

	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);
	if (!check_set_object(instance, p1)) {
		delete_tree(instance, p2);
		p2 = &alsa_lisp_nil;
	} else {
		if (set_object(instance, p1, p2) == NULL) {
			delete_tree(instance, p1);
			delete_tree(instance, p2);
			return NULL;
		}
	}
	delete_tree(instance, p1);
	return incref_tree(instance, p2);
}

/*
 * Syntax: (unset name)
 */
static struct alisp_object * F_unset(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1 = eval(instance, car(args));

	delete_tree(instance, unset_object(instance, p1));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	return p1;
}

/*
 * Syntax: (setq name value...)
 * Syntax: (setf name value...)
 * `name' is not evalled
 */
static struct alisp_object * F_setq(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1, * p2 = NULL, *n;

	do {
		p1 = car(p);
		p2 = eval(instance, car(cdr(p)));
		n = cdr(cdr(p));
		delete_object(instance, cdr(p));
		delete_object(instance, p);
		if (!check_set_object(instance, p1)) {
			delete_tree(instance, p2);
			p2 = &alsa_lisp_nil;
		} else {
			if (set_object(instance, p1, p2) == NULL) {
				delete_tree(instance, p1);
				delete_tree(instance, p2);
				return NULL;
			}
		}
		delete_tree(instance, p1);
		p = n;
	} while (p != &alsa_lisp_nil);

	return incref_tree(instance, p2);
}

/*
 * Syntax: (unsetq name...)
 * Syntax: (unsetf name...)
 * `name' is not evalled
 */
static struct alisp_object * F_unsetq(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1 = NULL, * n;

	do {
		if (p1)
			delete_tree(instance, p1);
		p1 = unset_object(instance, car(p));
		delete_tree(instance, car(p));
		p = cdr(n = p);
		delete_object(instance, n);
	} while (p != &alsa_lisp_nil);

	return p1;
}

/*
 * Syntax: (defun name arglist expr...)
 * `name' is not evalled
 * `arglist' is not evalled
 */
static struct alisp_object * F_defun(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1 = car(args),
			    * p2 = car(cdr(args)),
			    * p3 = cdr(cdr(args));
	struct alisp_object * lexpr;

	lexpr = new_object(instance, ALISP_OBJ_CONS);
	if (lexpr) {
		lexpr->value.c.car = new_identifier(instance, "lambda");
		if (lexpr->value.c.car == NULL) {
			delete_object(instance, lexpr);
			delete_tree(instance, args);
			return NULL;
		}
		if ((lexpr->value.c.cdr = new_object(instance, ALISP_OBJ_CONS)) == NULL) {
			delete_object(instance, lexpr->value.c.car);
			delete_object(instance, lexpr);
			delete_tree(instance, args);
			return NULL;
		}
		lexpr->value.c.cdr->value.c.car = p2;
		lexpr->value.c.cdr->value.c.cdr = p3;
		delete_object(instance, cdr(args));
		delete_object(instance, args);
		if (set_object(instance, p1, lexpr) == NULL) {
			delete_tree(instance, p1);
			delete_tree(instance, lexpr);
			return NULL;
		}
		delete_tree(instance, p1);
	} else {
		delete_tree(instance, args);
	}
	return &alsa_lisp_nil;
}

static struct alisp_object * eval_func(struct alisp_instance *instance, struct alisp_object * p, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * p3, * p4;
	struct alisp_object ** eval_objs, ** save_objs;
	int i;

	p1 = car(p);
	if (alisp_compare_type(p1, ALISP_OBJ_IDENTIFIER) &&
	    !strcmp(p1->value.s, "lambda")) {
		p2 = car(cdr(p));
		p3 = args;

		if ((i = count_list(p2)) != count_list(p3)) {
			lisp_warn(instance, "wrong number of parameters");
			goto _delete;
		}

		eval_objs = malloc(2 * i * sizeof(struct alisp_object *));
		if (eval_objs == NULL) {
			nomem();
			goto _delete;
		}
		save_objs = eval_objs + i;
		
		/*
		 * Save the new variable values.
		 */
		i = 0;
		while (p3 != &alsa_lisp_nil) {
			eval_objs[i++] = eval(instance, car(p3));
			p3 = cdr(p4 = p3);
			delete_object(instance, p4);
		}

		/*
		 * Save the old variable values and set the new ones.
		 */
		i = 0;
		while (p2 != &alsa_lisp_nil) {
			p3 = car(p2);
			save_objs[i] = replace_object(instance, p3, eval_objs[i]);
			if (save_objs[i] == NULL &&
			    set_object_direct(instance, p3, eval_objs[i]) == NULL) {
			    	p4 = NULL;
				goto _end;
			}
			p2 = cdr(p2);
			++i;
		}

		p4 = F_progn(instance, cdr(incref_tree(instance, p3 = cdr(p))));

		/*
		 * Restore the old variable values.
		 */
		p2 = car(p3);
		delete_object(instance, p3);
		i = 0;
		while (p2 != &alsa_lisp_nil) {
			p3 = car(p2);
			if (save_objs[i] == NULL) {
				p3 = unset_object(instance, p3);
			} else {
				p3 = replace_object(instance, p3, save_objs[i]);
			}
			i++;
			delete_tree(instance, p3);
			delete_tree(instance, car(p2));
			p2 = cdr(p3 = p2);
			delete_object(instance, p3);
		}

               _end:
		free(eval_objs);

		return p4;
	} else {
	       _delete:
		delete_tree(instance, args);
	}
	return &alsa_lisp_nil;
}

struct alisp_object * F_gc(struct alisp_instance *instance ATTRIBUTE_UNUSED, struct alisp_object * args ATTRIBUTE_UNUSED)
{
	/* improved: no more traditional gc */
	return &alsa_lisp_t;
}

/*
 * Syntax: (path what)
 * what is string ('data')
 */
struct alisp_object * F_path(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (!alisp_compare_type(p1, ALISP_OBJ_STRING)) {
		delete_tree(instance, p1);
		return &alsa_lisp_nil;
	}
	if (!strcmp(p1->value.s, "data")) {
		delete_tree(instance, p1);
		return new_string(instance, ALSA_CONFIG_DIR);
	}
	delete_tree(instance, p1);
	return &alsa_lisp_nil;
}

/*
 * Syntax: (include filename...)
 */
struct alisp_object * F_include(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1;
	int res = -ENOENT;

	do {
		p1 = eval(instance, car(p));
		if (alisp_compare_type(p1, ALISP_OBJ_STRING))
			res = alisp_include_file(instance, p1->value.s);
		delete_tree(instance, p1);
		p = cdr(p1 = p);
		delete_object(instance, p1);
	} while (p != &alsa_lisp_nil);

	return new_integer(instance, res);
}

/*
 * Syntax: (string-to-integer value)
 * 'value' can be integer or float type
 */
struct alisp_object * F_string_to_integer(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = eval(instance, car(args)), * p1;

	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (alisp_compare_type(p, ALISP_OBJ_INTEGER))
		return p;
	if (alisp_compare_type(p, ALISP_OBJ_FLOAT)) {
		p1 = new_integer(instance, floor(p->value.f));
	} else {
		lisp_warn(instance, "expected an integer or float for integer conversion");
		p1 = &alsa_lisp_nil;
	}
	delete_tree(instance, p);
	return p1;
}

/*
 * Syntax: (string-to-float value)
 * 'value' can be integer or float type
 */
struct alisp_object * F_string_to_float(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = eval(instance, car(args)), * p1;

	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (alisp_compare_type(p, ALISP_OBJ_FLOAT))
		return p;
	if (alisp_compare_type(p, ALISP_OBJ_INTEGER)) {
		p1 = new_float(instance, p->value.i);
	} else {
		lisp_warn(instance, "expected an integer or float for integer conversion");
		p1 = &alsa_lisp_nil;
	}
	delete_tree(instance, p);
	return p1;
}

static int append_to_string(char **s, int *len, char *from, int size)
{
	if (*len == 0) {
		*s = malloc(*len = size + 1);
		if (*s == NULL) {
			nomem();
			return -ENOMEM;
		}
		memcpy(*s, from, size);
	} else {
		*len += size;
		*s = realloc(*s, *len);
		if (*s == NULL) {
			nomem();
			return -ENOMEM;
		}
		memcpy(*s + strlen(*s), from, size);
	}
	(*s)[*len - 1] = '\0';
	return 0;
}

static int format_parse_char(struct alisp_instance *instance, char **s, int *len, struct alisp_object *p)
{
	char b;

	if (!alisp_compare_type(p, ALISP_OBJ_INTEGER)) {
		lisp_warn(instance, "format: expected integer\n");
		return 0;
	}
	b = p->value.i;
	return append_to_string(s, len, &b, 1);
}

static int format_parse_integer(struct alisp_instance *instance, char **s, int *len, struct alisp_object *p)
{
	int res;
	char *s1;

	if (!alisp_compare_type(p, ALISP_OBJ_INTEGER) &&
	    !alisp_compare_type(p, ALISP_OBJ_FLOAT)) {
		lisp_warn(instance, "format: expected integer or float\n");
		return 0;
	}
	s1 = malloc(64);
	if (s1 == NULL) {
		nomem();
		return -ENOMEM;
	}
	sprintf(s1, "%li", alisp_compare_type(p, ALISP_OBJ_FLOAT) ? (long)floor(p->value.f) : p->value.i);
	res = append_to_string(s, len, s1, strlen(s1));
	free(s1);
	return res;
}

static int format_parse_float(struct alisp_instance *instance, char **s, int *len, struct alisp_object *p)
{
	int res;
	char *s1;

	if (!alisp_compare_type(p, ALISP_OBJ_INTEGER) &&
	    !alisp_compare_type(p, ALISP_OBJ_FLOAT)) {
		lisp_warn(instance, "format: expected integer or float\n");
		return 0;
	}
	s1 = malloc(64);
	if (s1 == NULL) {
		nomem();
		return -ENOMEM;
	}
	sprintf(s1, "%f", alisp_compare_type(p, ALISP_OBJ_FLOAT) ? p->value.f : (double)p->value.i);
	res = append_to_string(s, len, s1, strlen(s1));
	free(s1);
	return res;
}

static int format_parse_string(struct alisp_instance *instance, char **s, int *len, struct alisp_object *p)
{
	if (!alisp_compare_type(p, ALISP_OBJ_STRING)) {
		lisp_warn(instance, "format: expected string\n");
		return 0;
	}
	return append_to_string(s, len, p->value.s, strlen(p->value.s));
}

/*
 * Syntax: (format format value...)
 * 'format' is C-like format string
 */
struct alisp_object * F_format(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = eval(instance, car(args)), * p1 = cdr(args), * n;
	char *s, *s1, *s2;
	int len;

	delete_object(instance, args);
	if (!alisp_compare_type(p, ALISP_OBJ_STRING)) {
		delete_tree(instance, p1);
		delete_tree(instance, p);
		lisp_warn(instance, "format: expected an format string");
		return &alsa_lisp_nil;
	}
	s = p->value.s;
	s1 = NULL;
	len = 0;
	n = eval(instance, car(p1));
	do {
		while (1) {
			s2 = s;
			while (*s2 && *s2 != '%')
				s2++;
			if (s2 != s) {
				if (append_to_string(&s1, &len, s, s2 - s) < 0) {
				      __error:
					delete_tree(instance, n);
					delete_tree(instance, cdr(p1));
					delete_object(instance, p1);
					delete_tree(instance, p);
					return NULL;
				}
			}
			if (*s2 == '%')
				s2++;
			switch (*s2) {
			case '%':
				if (append_to_string(&s1, &len, s2, 1) < 0)
					goto __error;
				s = s2 + 1;
				break;
			case 'c':
				if (format_parse_char(instance, &s1, &len, n) < 0)
					goto __error;
				s = s2 + 1;
				goto __next;
			case 'd':
			case 'i':
				if (format_parse_integer(instance, &s1, &len, n) < 0)
					goto __error;
				s = s2 + 1;
				goto __next;
			case 'f':
				if (format_parse_float(instance, &s1, &len, n) < 0)
					goto __error;
				s = s2 + 1;
				goto __next;
			case 's':
				if (format_parse_string(instance, &s1, &len, n) < 0)
					goto __error;
				s = s2 + 1;
				goto __next;
			case '\0':
				goto __end;
			default:
				lisp_warn(instance, "unknown format char '%c'", *s2);
				s = s2 + 1;
				goto __next;
			}
		}
	      __next:
		delete_tree(instance, n);
		p1 = cdr(n = p1);
		delete_object(instance, n);
		n = eval(instance, car(p1));
	} while (*s);
      __end:
	delete_tree(instance, n);
	delete_tree(instance, cdr(p1));
	delete_object(instance, p1);
	delete_tree(instance, p);
	if (len > 0) {
		p1 = new_string(instance, s1);
		free(s1);
	} else {
		p1 = &alsa_lisp_nil;
	}
	return p1;
}

/*
 * Syntax: (compare-strings str1 start1 end1 str2 start2 end2 /opt-case-insensitive)
 * 'str1' is first compared string
 * 'start1' is first char (0..)
 * 'end1' is last char (0..)
 * 'str2' is second compared string
 * 'start2' is first char (0..)
 * 'end2' is last char (0..)
 * /opt-case-insensitive true - case insensitive match
 */
struct alisp_object * F_compare_strings(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1 = args, * n, * p[7];
	char *s1, *s2;
	int start1, end1, start2, end2;
	
	for (start1 = 0; start1 < 7; start1++) {
		p[start1] = eval(instance, car(p1));
		p1 = cdr(n = p1);
		delete_object(instance, n);
	}
	delete_tree(instance, p1);
	if (alisp_compare_type(p[0], ALISP_OBJ_STRING)) {
		lisp_warn(instance, "compare-strings: first argument must be string\n");
		p1 = &alsa_lisp_nil;
		goto __err;
	}
	if (alisp_compare_type(p[1], ALISP_OBJ_INTEGER)) {
		lisp_warn(instance, "compare-strings: second argument must be integer\n");
		p1 = &alsa_lisp_nil;
		goto __err;
	}
	if (alisp_compare_type(p[2], ALISP_OBJ_INTEGER)) {
		lisp_warn(instance, "compare-strings: third argument must be integer\n");
		p1 = &alsa_lisp_nil;
		goto __err;
	}
	if (alisp_compare_type(p[3], ALISP_OBJ_STRING)) {
		lisp_warn(instance, "compare-strings: fifth argument must be string\n");
		p1 = &alsa_lisp_nil;
		goto __err;
	}
	if (!alisp_compare_type(p[4], ALISP_OBJ_NIL) &&
	    !alisp_compare_type(p[4], ALISP_OBJ_INTEGER)) {
		lisp_warn(instance, "compare-strings: fourth argument must be integer\n");
		p1 = &alsa_lisp_nil;
		goto __err;
	}
	if (!alisp_compare_type(p[5], ALISP_OBJ_NIL) &&
	    !alisp_compare_type(p[5], ALISP_OBJ_INTEGER)) {
		lisp_warn(instance, "compare-strings: sixth argument must be integer\n");
		p1 = &alsa_lisp_nil;
		goto __err;
	}
	s1 = p[0]->value.s;
	start1 = p[1]->value.i;
	end1 = p[2]->value.i;
	s2 = p[3]->value.s;
	start2 = alisp_compare_type(p[4], ALISP_OBJ_NIL) ? 0 : p[4]->value.i;
	end2 = alisp_compare_type(p[5], ALISP_OBJ_NIL) ? start2 + (end1 - start1) : p[5]->value.i;
	if (start1 < 0 || start2 < 0 || end1 < 0 || end2 < 0 ||
	    start1 >= (int)strlen(s1) || start2 >= (int)strlen(s2) ||
	    (end1 - start1) != (end2 - start2)) {
	    	p1 = &alsa_lisp_nil;
	    	goto __err;
	}
	if (p[6] != &alsa_lisp_nil) {
		while (start1 < end1) {
			if (s1[start1] == '\0' ||
			    s2[start2] == '\0' ||
			    tolower(s1[start1]) != tolower(s2[start2])) {
				p1 = &alsa_lisp_nil;
				goto __err;
			}
			start1++;
			start2++;
		}
	} else {
		while (start1 < end1) {
			if (s1[start1] == '\0' ||
			    s2[start2] == '\0' ||
			    s1[start1] != s2[start2]) {
				p1 = &alsa_lisp_nil;
				goto __err;
			}
			start1++;
			start2++;
		}
	}
	p1 = &alsa_lisp_t;
	
      __err:
      	for (start1 = 0; start1 < 7; start1++)
      		delete_tree(instance, p[start1]);
      	return p1;	
}

/*
 *  Syntax: (assoc key alist)
 */
struct alisp_object * F_assoc(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * n;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	do {
		if (eq(p1, car(car(p2)))) {
			n = car(p2);
			delete_tree(instance, p1);
			delete_tree(instance, cdr(p2));
			delete_object(instance, p2);
			return n;
		}
		delete_tree(instance, car(p2));
		p2 = cdr(n = p2);
		delete_object(instance, n);
	} while (p2 != &alsa_lisp_nil);

	delete_tree(instance, p1);
	return &alsa_lisp_nil;	
}

/*
 *  Syntax: (rassoc value alist)
 */
struct alisp_object * F_rassoc(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, *p2, * n;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	do {
		if (eq(p1, cdr(car(p2)))) {
			n = car(p2);
			delete_tree(instance, p1);
			delete_tree(instance, cdr(p2));
			delete_object(instance, p2);
			return n;
		}
		delete_tree(instance, car(p2));
		p2 = cdr(n = p2);
		delete_object(instance, n);
	} while (p2 != &alsa_lisp_nil);

	delete_tree(instance, p1);
	return &alsa_lisp_nil;	
}

/*
 *  Syntax: (assq key alist)
 */
struct alisp_object * F_assq(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * n;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	do {
		if (equal(p1, car(car(p2)))) {
			n = car(p2);
			delete_tree(instance, p1);
			delete_tree(instance, cdr(p2));
			delete_object(instance, p2);
			return n;
		}
		delete_tree(instance, car(p2));
		p2 = cdr(n = p2);
		delete_object(instance, n);
	} while (p2 != &alsa_lisp_nil);

	delete_tree(instance, p1);
	return &alsa_lisp_nil;	
}

/*
 *  Syntax: (nth index alist)
 */
struct alisp_object * F_nth(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * n;
	long idx;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	if (!alisp_compare_type(p1, ALISP_OBJ_INTEGER)) {
		delete_tree(instance, p1);
		delete_tree(instance, p2);
		return &alsa_lisp_nil;
	}
	if (!alisp_compare_type(p2, ALISP_OBJ_CONS)) {
		delete_object(instance, p1);
		delete_tree(instance, p2);
		return &alsa_lisp_nil;
	}
	idx = p1->value.i;
	delete_object(instance, p1);
	while (idx-- > 0) {
		delete_tree(instance, car(p2));
		p2 = cdr(n = p2);
		delete_object(instance, n);
	}
	n = car(p2);
	delete_tree(instance, cdr(p2));
	delete_object(instance, p2);
	return n;
}

/*
 *  Syntax: (rassq value alist)
 */
struct alisp_object * F_rassq(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, * p2, * n;

	p1 = eval(instance, car(args));
	p2 = eval(instance, car(cdr(args)));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);

	do {
		if (equal(p1, cdr(car(p2)))) {
			n = car(p2);
			delete_tree(instance, p1);
			delete_tree(instance, cdr(p2));
			delete_object(instance, p2);
			return n;
		}
		delete_tree(instance, car(p2));
		p2 = cdr(n = p2);
		delete_object(instance, n);
	} while (p2 != &alsa_lisp_nil);

	delete_tree(instance, p1);
	return &alsa_lisp_nil;	
}

static struct alisp_object * F_dump_memory(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = car(args);

	if (p != &alsa_lisp_nil && cdr(args) == &alsa_lisp_nil &&
	    alisp_compare_type(p, ALISP_OBJ_STRING)) {
		if (strlen(p->value.s) > 0) {
			dump_objects(instance, p->value.s);
			delete_tree(instance, args);
			return &alsa_lisp_t;
		} else
			lisp_warn(instance, "expected filename");
	} else
		lisp_warn(instance, "wrong number of parameters (expected string)");

	delete_tree(instance, args);
	return &alsa_lisp_nil;
}

static struct alisp_object * F_stat_memory(struct alisp_instance *instance, struct alisp_object * args)
{
	snd_output_printf(instance->out, "*** Memory stats\n");
	snd_output_printf(instance->out, "  used_objs = %li, free_objs = %li, max_objs = %li, obj_size = %i (total bytes = %li, max bytes = %li)\n",
			  instance->used_objs,
			  instance->free_objs,
			  instance->max_objs,
			  (int)sizeof(struct alisp_object),
			  (long)((instance->used_objs + instance->free_objs) * sizeof(struct alisp_object)),
			  (long)(instance->max_objs * sizeof(struct alisp_object)));
	delete_tree(instance, args);
	return &alsa_lisp_nil;
}

static struct alisp_object * F_check_memory(struct alisp_instance *instance, struct alisp_object * args)
{
	delete_tree(instance, args);
	if (instance->used_objs > 0) {
		fprintf(stderr, "!!!alsa lisp - check memory failed!!!\n");
		F_stat_memory(instance, &alsa_lisp_nil);
		exit(EXIT_FAILURE);
	}
	return &alsa_lisp_t;
}

static struct alisp_object * F_dump_objects(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = car(args);

	if (p != &alsa_lisp_nil && cdr(args) == &alsa_lisp_nil &&
	    alisp_compare_type(p, ALISP_OBJ_STRING)) {
		if (strlen(p->value.s) > 0) {
			dump_obj_lists(instance, p->value.s);
			delete_tree(instance, args);
			return &alsa_lisp_t;
		} else
			lisp_warn(instance, "expected filename");
	} else
		lisp_warn(instance, "wrong number of parameters (expected string)");

	delete_tree(instance, args);
	return &alsa_lisp_nil;
}

struct intrinsic {
	const char *name;
	struct alisp_object * (*func)(struct alisp_instance *instance, struct alisp_object * args);
};

static const struct intrinsic intrinsics[] = {
	{ "!=", F_numneq },
	{ "%", F_mod },
	{ "&check-memory", F_check_memory },
	{ "&dump-memory", F_dump_memory },
	{ "&dump-objects", F_dump_objects },
	{ "&stat-memory", F_stat_memory },
	{ "*", F_mul },
	{ "+", F_add },
	{ "-", F_sub },
	{ "/", F_div },
	{ "<", F_lt },
	{ "<=", F_le },
	{ "=", F_numeq },
	{ ">", F_gt },
	{ ">=", F_ge },
	{ "and", F_and },
	{ "assoc", F_assoc },
	{ "assq", F_assq },
	{ "atom", F_atom },
	{ "car", F_car },
	{ "cdr", F_cdr },
	{ "compare-strings", F_compare_strings },
	{ "concat", F_concat },
	{ "cond", F_cond },
	{ "cons", F_cons },
	{ "defun", F_defun },
	{ "eq", F_eq },
	{ "equal", F_equal },
	{ "eval", F_eval },
	{ "exfun", F_exfun },
	{ "format", F_format },
	{ "funcall", F_funcall },
	{ "garbage-collect", F_gc },
	{ "gc", F_gc },
	{ "if", F_if },
	{ "include", F_include },
	{ "list", F_list },
	{ "not", F_not },
	{ "nth", F_nth },
	{ "null", F_not },
	{ "or", F_or },
	{ "path", F_path },
	{ "princ", F_princ },
	{ "prog1", F_prog1 },
	{ "prog2", F_prog2 },
	{ "progn", F_progn },
	{ "quote", F_quote },
	{ "rassoc", F_rassoc },
	{ "rassq", F_rassq },
	{ "set", F_set },
	{ "setf", F_setq },
	{ "setq", F_setq },
	{ "string-equal", F_equal },
	{ "string-to-float", F_string_to_float },
	{ "string-to-integer", F_string_to_integer },
	{ "string-to-number", F_string_to_float },
	{ "string=", F_equal },
	{ "unless", F_unless },
	{ "unset", F_unset },
	{ "unsetf", F_unsetq },
	{ "unsetq", F_unsetq },
	{ "when", F_when },
	{ "while", F_while },
};

#include "alisp_snd.c"

static int compar(const void *p1, const void *p2)
{
	return strcmp(((struct intrinsic *)p1)->name,
		      ((struct intrinsic *)p2)->name);
}

static inline struct alisp_object * eval_cons1(struct alisp_instance *instance, struct alisp_object * p1, struct alisp_object * p2)
{
	struct alisp_object * p3;
	struct intrinsic key, *item;

	key.name = p1->value.s;

	if ((item = bsearch(&key, intrinsics,
			    sizeof intrinsics / sizeof intrinsics[0],
			    sizeof intrinsics[0], compar)) != NULL) {
		delete_object(instance, p1);
		return item->func(instance, p2);
	}

	if ((item = bsearch(&key, snd_intrinsics,
			    sizeof snd_intrinsics / sizeof snd_intrinsics[0],
			    sizeof snd_intrinsics[0], compar)) != NULL) {
		delete_object(instance, p1);
		return item->func(instance, p2);
	}

	if ((p3 = get_object(instance, p1)) != &alsa_lisp_nil) {
		delete_object(instance, p1);
		return eval_func(instance, p3, p2);
	} else {
		lisp_warn(instance, "function `%s' is undefined", p1->value.s);
		delete_object(instance, p1);
		delete_tree(instance, p2);
	}

	return &alsa_lisp_nil;
}

/*
 * Syntax: (funcall function args...)
 */
static struct alisp_object * F_funcall(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = eval(instance, car(args)), * p1;

	if (!alisp_compare_type(p, ALISP_OBJ_IDENTIFIER) &&
	    !alisp_compare_type(p, ALISP_OBJ_STRING)) {
		lisp_warn(instance, "expected an function name");
		delete_tree(instance, p);
		delete_tree(instance, cdr(args));
		delete_object(instance, args);
		return &alsa_lisp_nil;
	}
	p1 = cdr(args);
	delete_object(instance, args);
	return eval_cons1(instance, p, p1);
}

static inline struct alisp_object * eval_cons(struct alisp_instance *instance, struct alisp_object * p)
{
	struct alisp_object * p1 = car(p), * p2;

	if (p1 != &alsa_lisp_nil && alisp_compare_type(p1, ALISP_OBJ_IDENTIFIER)) {
		if (!strcmp(p1->value.s, "lambda"))
			return p;

		p2 = cdr(p);
		delete_object(instance, p);
		return eval_cons1(instance, p1, p2);
	} else {
		delete_tree(instance, p);
	}

	return &alsa_lisp_nil;
}

static struct alisp_object * eval(struct alisp_instance *instance, struct alisp_object * p)
{
	switch (alisp_get_type(p)) {
	case ALISP_OBJ_IDENTIFIER: {
		struct alisp_object *r = incref_tree(instance, get_object(instance, p));
		delete_object(instance, p);
		return r;
	}
	case ALISP_OBJ_INTEGER:
	case ALISP_OBJ_FLOAT:
	case ALISP_OBJ_STRING:
	case ALISP_OBJ_POINTER:
		return p;
	case ALISP_OBJ_CONS:
		return eval_cons(instance, p);
	default:
		break;
	}

	return p;
}

static struct alisp_object * F_eval(struct alisp_instance *instance, struct alisp_object * args)
{
	return eval(instance, eval(instance, car(args)));
}

/*
 *  main routine
 */

static int alisp_include_file(struct alisp_instance *instance, const char *filename)
{
	snd_input_t *old_in;
	struct alisp_object *p, *p1;
	char *name;
	int retval = 0, err;

	err = snd_user_file(filename, &name);
	if (err < 0)
		return err;
	old_in = instance->in;
	err = snd_input_stdio_open(&instance->in, name, "r");
	if (err < 0) {
		retval = err;
		goto _err;
	}
	if (instance->verbose)
		lisp_verbose(instance, "** include filename '%s'", name);

	for (;;) {
		if ((p = parse_object(instance, 0)) == NULL)
			break;
		if (instance->verbose) {
			lisp_verbose(instance, "** code");
			princ_object(instance->vout, p);
			snd_output_putc(instance->vout, '\n');
		}
		p1 = eval(instance, p);
		if (p1 == NULL) {
			retval = -ENOMEM;
			break;
		}
		if (instance->verbose) {
			lisp_verbose(instance, "** result");
			princ_object(instance->vout, p1);
			snd_output_putc(instance->vout, '\n');
		}
		delete_tree(instance, p1);
		if (instance->debug) {
			lisp_debug(instance, "** objects after operation");
			print_obj_lists(instance, instance->dout);
		}
	}	

	snd_input_close(instance->in);
       _err:
	free(name);
	instance->in = old_in;
	return retval;
}
 
int alsa_lisp(struct alisp_cfg *cfg, struct alisp_instance **_instance)
{
	struct alisp_instance *instance;
	struct alisp_object *p, *p1;
	int i, j, retval = 0;
	
	instance = (struct alisp_instance *)malloc(sizeof(struct alisp_instance));
	if (instance == NULL) {
		nomem();
		return -ENOMEM;
	}
	memset(instance, 0, sizeof(struct alisp_instance));
	instance->verbose = cfg->verbose && cfg->vout;
	instance->warning = cfg->warning && cfg->wout;
	instance->debug = cfg->debug && cfg->dout;
	instance->in = cfg->in;
	instance->out = cfg->out;
	instance->vout = cfg->vout;
	instance->eout = cfg->eout;
	instance->wout = cfg->wout;
	instance->dout = cfg->dout;
	INIT_LIST_HEAD(&instance->free_objs_list);
	for (i = 0; i < ALISP_OBJ_PAIR_HASH_SIZE; i++) {
		for (j = 0; j <= ALISP_OBJ_LAST_SEARCH; j++)
			INIT_LIST_HEAD(&instance->used_objs_list[i][j]);
		INIT_LIST_HEAD(&instance->setobjs_list[i]);
	}
	
	init_lex(instance);

	for (;;) {
		if ((p = parse_object(instance, 0)) == NULL)
			break;
		if (instance->verbose) {
			lisp_verbose(instance, "** code");
			princ_object(instance->vout, p);
			snd_output_putc(instance->vout, '\n');
		}
		p1 = eval(instance, p);
		if (p1 == NULL) {
			retval = -ENOMEM;
			break;
		}
		if (instance->verbose) {
			lisp_verbose(instance, "** result");
			princ_object(instance->vout, p1);
			snd_output_putc(instance->vout, '\n');
		}
		delete_tree(instance, p1);
		if (instance->debug) {
			lisp_debug(instance, "** objects after operation");
			print_obj_lists(instance, instance->dout);
		}
	}

	if (_instance)
		*_instance = instance;
	else
		alsa_lisp_free(instance); 
	
	return 0;
}

void alsa_lisp_free(struct alisp_instance *instance)
{
	if (instance == NULL)
		return;
	done_lex(instance);
	free_objects(instance);
	free(instance);
}

struct alisp_cfg *alsa_lisp_default_cfg(snd_input_t *input)
{
	snd_output_t *output, *eoutput;
	struct alisp_cfg *cfg;
	int err;
	
	err = snd_output_stdio_attach(&output, stdout, 0);
	if (err < 0)
		return NULL;
	err = snd_output_stdio_attach(&eoutput, stderr, 0);
	if (err < 0) {
		snd_output_close(output);
		return NULL;
	}
	cfg = calloc(1, sizeof(struct alisp_cfg));
	if (cfg == NULL) {
		snd_output_close(eoutput);
		snd_output_close(output);
		return NULL;
	}
	cfg->out = output;
	cfg->wout = eoutput;
	cfg->eout = eoutput;
	cfg->dout = eoutput;
	cfg->in = input;
	return cfg;
}

void alsa_lisp_default_cfg_free(struct alisp_cfg *cfg)
{
	snd_input_close(cfg->in);
	snd_output_close(cfg->out);
	snd_output_close(cfg->dout);
	free(cfg);
}

int alsa_lisp_function(struct alisp_instance *instance, struct alisp_seq_iterator **result,
		       const char *id, const char *args, ...)
{
	int err = 0;
	struct alisp_object *aargs = NULL, *obj, *res;

	if (args && *args != 'n') {
		va_list ap;
		struct alisp_object *p;
		p = NULL;
		va_start(ap, args);
		while (*args) {
			if (*args++ != '%') {
				err = -EINVAL;
				break;
			}
			if (*args == '\0') {
				err = -EINVAL;
				break;
			}
			obj = NULL;
			err = 0;
			switch (*args++) {
			case 's':
				obj = new_string(instance, va_arg(ap, char *));
				break;
			case 'i':
				obj = new_integer(instance, va_arg(ap, int));
				break;
			case 'l':
				obj = new_integer(instance, va_arg(ap, long));
				break;
			case 'f':
			case 'd':
				obj = new_integer(instance, va_arg(ap, double));
				break;
			case 'p': {
				char _ptrid[24];
				char *ptrid = _ptrid;
				while (*args && *args != '%')
					*ptrid++ = *args++;
				*ptrid = 0;
				if (ptrid == _ptrid) {
					err = -EINVAL;
					break;
				}
				obj = new_cons_pointer(instance, _ptrid, va_arg(ap, void *));
				obj = quote_object(instance, obj);
				break;
			}
			default:
				err = -EINVAL;
				break;
			}
			if (err < 0)
				goto __args_end;
			if (obj == NULL) {
				err = -ENOMEM;
				goto __args_end;
			}
			if (p == NULL) {
				p = aargs = new_object(instance, ALISP_OBJ_CONS);
			} else {
				p->value.c.cdr = new_object(instance, ALISP_OBJ_CONS);
				p = p->value.c.cdr;
			}
			if (p == NULL) {
				err = -ENOMEM;
				goto __args_end;
			}
			p->value.c.car = obj;
		}
	      __args_end:
		va_end(ap);
		if (err < 0)
			return err;
	}

	err = -ENOENT;
	if (aargs == NULL)
		aargs = &alsa_lisp_nil;
	if ((obj = get_object1(instance, id)) != &alsa_lisp_nil) {
		res = eval_func(instance, obj, aargs);
		err = 0;
	} else {
		struct intrinsic key, *item;
		key.name = id;
		if ((item = bsearch(&key, intrinsics,
				    sizeof intrinsics / sizeof intrinsics[0],
				    sizeof intrinsics[0], compar)) != NULL) {
			res = item->func(instance, aargs);
			err = 0;
		} else if ((item = bsearch(&key, snd_intrinsics,
				         sizeof snd_intrinsics / sizeof snd_intrinsics[0],
					 sizeof snd_intrinsics[0], compar)) != NULL) {
			res = item->func(instance, aargs);
			err = 0;
		} else {
			res = &alsa_lisp_nil;
		}
	}
	if (res == NULL)
		err = -ENOMEM;
	if (err == 0 && result) {
		*result = res;
	} else {
		delete_tree(instance, res);
	}

	return 0;
}

void alsa_lisp_result_free(struct alisp_instance *instance,
			   struct alisp_seq_iterator *result)
{
	delete_tree(instance, result);
}

int alsa_lisp_seq_first(struct alisp_instance *instance, const char *id,
			struct alisp_seq_iterator **seq)
{
	struct alisp_object * p1;

	p1 = get_object1(instance, id);
	if (p1 == NULL)
		return -ENOMEM;
	*seq = p1;
	return 0;
}

int alsa_lisp_seq_next(struct alisp_seq_iterator **seq)
{
	struct alisp_object * p1 = *seq;

	p1 = cdr(p1);
	if (p1 == &alsa_lisp_nil)
		return -ENOENT;
	*seq = p1;
	return 0;
}

int alsa_lisp_seq_count(struct alisp_seq_iterator *seq)
{
	int count = 0;
	
	while (seq != &alsa_lisp_nil) {
		count++;
		seq = cdr(seq);
	}
	return count;
}

int alsa_lisp_seq_integer(struct alisp_seq_iterator *seq, long *val)
{
	if (alisp_compare_type(seq, ALISP_OBJ_CONS))
		seq = seq->value.c.cdr;
	if (alisp_compare_type(seq, ALISP_OBJ_INTEGER))
		*val = seq->value.i;
	else
		return -EINVAL;
	return 0;
}

int alsa_lisp_seq_pointer(struct alisp_seq_iterator *seq, const char *ptr_id, void **ptr)
{
	struct alisp_object * p2;
	
	if (alisp_compare_type(seq, ALISP_OBJ_CONS) &&
	    alisp_compare_type(seq->value.c.car, ALISP_OBJ_CONS))
		seq = seq->value.c.car;
	if (alisp_compare_type(seq, ALISP_OBJ_CONS)) {
		p2 = seq->value.c.car;
		if (!alisp_compare_type(p2, ALISP_OBJ_STRING))
			return -EINVAL;
		if (strcmp(p2->value.s, ptr_id))
			return -EINVAL;
		p2 = seq->value.c.cdr;
		if (!alisp_compare_type(p2, ALISP_OBJ_POINTER))
			return -EINVAL;
		*ptr = (void *)seq->value.ptr;
	} else
		return -EINVAL;
	return 0;
}
