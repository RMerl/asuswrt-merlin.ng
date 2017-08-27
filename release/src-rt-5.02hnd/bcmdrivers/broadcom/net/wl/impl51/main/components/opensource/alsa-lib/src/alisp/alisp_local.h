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

#include "list.h"

enum alisp_tokens {
	ALISP_IDENTIFIER,
	ALISP_INTEGER,
	ALISP_FLOAT,
	ALISP_FLOATE,
	ALISP_STRING
};

enum alisp_objects {
	ALISP_OBJ_INTEGER,
	ALISP_OBJ_FLOAT,
	ALISP_OBJ_IDENTIFIER,
	ALISP_OBJ_STRING,
	ALISP_OBJ_POINTER,
	ALISP_OBJ_CONS,
	ALISP_OBJ_LAST_SEARCH = ALISP_OBJ_CONS,
	ALISP_OBJ_NIL,
	ALISP_OBJ_T,
};

struct alisp_object;

#define ALISP_TYPE_MASK	0xf0000000
#define ALISP_TYPE_SHIFT 28
#define ALISP_REFS_MASK 0x0fffffff
#define ALISP_REFS_SHIFT 0
#define ALISP_MAX_REFS (ALISP_REFS_MASK>>ALISP_REFS_SHIFT)
#define ALISP_MAX_REFS_LIMIT ((ALISP_MAX_REFS + 1) / 2)

struct alisp_object {
	struct list_head list;
	unsigned int	type_refs;	/* type and count of references */
	union {
		char	*s;
		long	i;
		double	f;
		const void *ptr;
		struct {
			struct alisp_object *car;
			struct alisp_object *cdr;
		} c;
	} value;
};

static inline enum alisp_objects alisp_get_type(struct alisp_object *p)
{
	return (p->type_refs >> ALISP_TYPE_SHIFT);
}

static inline void alisp_set_type(struct alisp_object *p, enum alisp_objects type)
{
	p->type_refs &= ~ALISP_TYPE_MASK;
	p->type_refs |= (unsigned int)type << ALISP_TYPE_SHIFT;
}

static inline int alisp_compare_type(struct alisp_object *p, enum alisp_objects type)
{
	return ((unsigned int)type << ALISP_TYPE_SHIFT) ==
	       (p->type_refs & ALISP_TYPE_MASK);
}

static inline void alisp_set_refs(struct alisp_object *p, unsigned int refs)
{
	p->type_refs &= ~ALISP_REFS_MASK;
	p->type_refs |= refs & ALISP_REFS_MASK;
}

static inline unsigned int alisp_get_refs(struct alisp_object *p)
{
	return p->type_refs & ALISP_REFS_MASK;
}

static inline unsigned int alisp_inc_refs(struct alisp_object *p)
{
	unsigned r = alisp_get_refs(p) + 1;
	alisp_set_refs(p, r);
	return r;
}

static inline unsigned int alisp_dec_refs(struct alisp_object *p)
{
	unsigned r = alisp_get_refs(p) - 1;
	alisp_set_refs(p, r);
	return r;
}

struct alisp_object_pair {
	struct list_head list;
	const char *name;
 	struct alisp_object *value;
};

#define ALISP_LEX_BUF_MAX	16
#define ALISP_OBJ_PAIR_HASH_SHIFT 4
#define ALISP_OBJ_PAIR_HASH_SIZE (1<<ALISP_OBJ_PAIR_HASH_SHIFT)
#define ALISP_OBJ_PAIR_HASH_MASK (ALISP_OBJ_PAIR_HASH_SIZE-1)
#define ALISP_FREE_OBJ_POOL	512	/* free objects above this pool */

struct alisp_instance {
	int verbose: 1,
	    warning: 1,
	    debug: 1;
	/* i/o */
	snd_input_t *in;
	snd_output_t *out;
	snd_output_t *eout;	/* error output */
	snd_output_t *vout;	/* verbose output */
	snd_output_t *wout;	/* warning output */
	snd_output_t *dout;	/* debug output */
	/* lexer */
	int charno;
	int lineno;
	int lex_buf[ALISP_LEX_BUF_MAX];
	int *lex_bufp;
	char *token_buffer;
	int token_buffer_max;
	int thistoken;
	/* object allocator / storage */
	long free_objs;
	long used_objs;
	long max_objs;
	struct list_head free_objs_list;
	struct list_head used_objs_list[ALISP_OBJ_PAIR_HASH_SIZE][ALISP_OBJ_LAST_SEARCH + 1];
	/* set object */
	struct list_head setobjs_list[ALISP_OBJ_PAIR_HASH_SIZE];
};
