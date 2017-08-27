#ifndef FR_PARSER_H
#define FR_PARSER_H

/*
 * parser.h	Structures and prototypes for parsing
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2013 Alan DeKok <aland@freeradius.org>
 */

RCSIDH(parser_h, "$Id$");

#include <freeradius-devel/map.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RADIUSD_H
/*
 *	Also defined in radiusd.h for radius_evalute_cond()
 */
typedef struct fr_cond_t fr_cond_t;
#endif

typedef enum cond_op_t {
	COND_NONE = 0,
	COND_AND = '&',
	COND_OR = '|'
} cond_op_t;


typedef enum fr_cond_type_t {
	COND_TYPE_INVALID = 0,
	COND_TYPE_TRUE,
	COND_TYPE_FALSE,
	COND_TYPE_EXISTS,
	COND_TYPE_MAP,
	COND_TYPE_CHILD
} fr_cond_type_t;


/*
 *	Allow for the following structures:
 *
 *	FOO			no OP, RHS is NULL
 *	FOO OP BAR
 *	(COND)			no LHS/RHS, child is COND, child OP is true
 *	(!(COND))		no LHS/RHS, child is COND, child OP is NOT
 *	(COND1 OP COND2)	no LHS/RHS, next is COND2, next OP is OP
 */
struct fr_cond_t {
	fr_cond_type_t	type;

	union {
		value_pair_map_t *map;
		value_pair_tmpl_t *vpt;
		fr_cond_t  	*child;
	} data;

	int		regex_i;
	int		negate;
	int		pass2_fixup;

	DICT_ATTR const *cast;

	cond_op_t	next_op;
	fr_cond_t	*next;
};


#define PASS2_FIXUP_NONE  (0)
#define PASS2_FIXUP_ATTR  (1)
#define PASS2_FIXUP_TYPE  (2)
#define PASS2_PAIRCOMPARE (3)

/*
 *	One pass over the conditions means that all references must
 *	exist at parse time.
 *
 *	Two pass means "soft fail", that some invalid references are
 *	left for pass 2.
 */
#define FR_COND_ONE_PASS (0)
#define FR_COND_TWO_PASS (1)

ssize_t fr_condition_tokenize(TALLOC_CTX *ctx, CONF_ITEM *ci, char const *start, fr_cond_t **head, char const **error, int flag);
size_t fr_cond_sprint(char *buffer, size_t bufsize, fr_cond_t const *c);

bool fr_condition_walk(fr_cond_t *head, bool (*callback)(void *, fr_cond_t *), void *ctx);

/*
 *	In xlat.c for now
 */
typedef struct xlat_exp xlat_exp_t;
ssize_t xlat_tokenize(TALLOC_CTX *ctx, char *fmt, xlat_exp_t **head,
		      char const **error);
size_t xlat_sprint(char *buffer, size_t bufsize, xlat_exp_t const *node);

#ifdef __cplusplus
}
#endif

#endif /* FR_PARSER_H */
