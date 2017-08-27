#ifndef LRAD_HEAP_H
#define LRAD_HEAP_H

/*
 * heap.h	Structures and prototypes for binary heaps.
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
 * Copyright 2007 Alan DeKok
 */

RCSIDH(heap_h, "$Id$")

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*fr_heap_cmp_t)(void const *, void const *);

typedef struct fr_heap_t fr_heap_t;
fr_heap_t *fr_heap_create(fr_heap_cmp_t cmp, size_t offset);
void fr_heap_delete(fr_heap_t *hp);

int fr_heap_insert(fr_heap_t *hp, void *data);
int fr_heap_extract(fr_heap_t *hp, void *data);
void *fr_heap_peek(fr_heap_t *hp);
int fr_heap_num_elements(fr_heap_t *hp);

#ifdef __cplusplus
}
#endif

#endif /* LRAD_HEAP_H */
