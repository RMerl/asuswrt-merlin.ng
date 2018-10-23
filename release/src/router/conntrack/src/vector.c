/*
 * (C) 2006-2008 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "vector.h"

#include <stdlib.h>
#include <string.h>

struct vector {
	char *data;
	unsigned int cur_elems;
	unsigned int max_elems;
	size_t size;
};

#define DEFAULT_VECTOR_MEMBERS	8
#define DEFAULT_VECTOR_GROWTH	8

struct vector *vector_create(size_t size)
{
	struct vector *v;

	v = calloc(sizeof(struct vector), 1);
	if (v == NULL)
		return NULL;

	v->size = size;
	v->cur_elems = 0;
	v->max_elems = DEFAULT_VECTOR_MEMBERS;

	v->data = calloc(size * DEFAULT_VECTOR_MEMBERS, 1);
	if (v->data == NULL) {
		free(v);
		return NULL;
	}

	return v;
}

void vector_destroy(struct vector *v)
{
	free(v->data);
	free(v);
}

int vector_add(struct vector *v, void *data)
{
	if (v->cur_elems >= v->max_elems) {
		v->max_elems += DEFAULT_VECTOR_GROWTH;
		v->data = realloc(v->data, v->max_elems * v->size);
		if (v->data == NULL) {
			v->max_elems -= DEFAULT_VECTOR_GROWTH;
			return -1;
		}
	}
	memcpy(v->data + (v->size * v->cur_elems), data, v->size);
	v->cur_elems++;
	return 0;
}

int vector_iterate(struct vector *v,
		   const void *data,
		   int (*fcn)(const void *a, const void *b))
{
	unsigned int i;

	for (i=0; i<v->cur_elems; i++) {
		char *ptr = v->data + (v->size * i);
		if (fcn(ptr, data))
			return 1;
	}
	return 0;
}
