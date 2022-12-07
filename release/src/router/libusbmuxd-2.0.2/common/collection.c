/*
 * collection.c
 *
 * Copyright (C) 2009 Hector Martin <hector@marcansoft.com>
 * Copyright (C) 2009 Nikias Bassen <nikias@gmx.li>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "collection.h"

#undef NDEBUG // we need to make sure we still get assertions because we can't handle memory allocation errors
#include <assert.h>

#define INIT_NULL(addr, count) { unsigned int i_ = 0; for (i_ = 0; i_ < count; i_++) ((void**)addr)[i_] = NULL; }

#define CAPACITY_STEP 8

void collection_init(struct collection *col)
{
	col->list = malloc(sizeof(void *) * CAPACITY_STEP);
	assert(col->list);
	INIT_NULL(col->list, CAPACITY_STEP);
	col->capacity = CAPACITY_STEP;
}

void collection_free(struct collection *col)
{
	free(col->list);
	col->list = NULL;
	col->capacity = 0;
}

void collection_add(struct collection *col, void *element)
{
	int i;
	for(i=0; i<col->capacity; i++) {
		if(!col->list[i]) {
			col->list[i] = element;
			return;
		}
	}
	void **newlist = realloc(col->list, sizeof(void*) * (col->capacity + CAPACITY_STEP));
	assert(newlist);
	col->list = newlist;
	INIT_NULL(&col->list[col->capacity], CAPACITY_STEP);
	col->list[col->capacity] = element;
	col->capacity += CAPACITY_STEP;
}

int collection_remove(struct collection *col, void *element)
{
	int i;
	for(i=0; i<col->capacity; i++) {
		if(col->list[i] == element) {
			col->list[i] = NULL;
			return 0;
		}
	}
	fprintf(stderr, "%s: WARNING: element %p not present in collection %p (cap %d)", __func__, element, col, col->capacity);
	return -1;
}

int collection_count(struct collection *col)
{
	int i, cnt = 0;
	for(i=0; i<col->capacity; i++) {
		if(col->list[i])
			cnt++;
	}
	return cnt;
}
