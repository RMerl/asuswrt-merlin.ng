/*
 * region.c --- code which manages allocations within a region.
 *
 * Copyright (C) 2001 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

#ifdef TEST_PROGRAM
#undef ENABLE_NLS
#endif
#include "e2fsck.h"

struct region_el {
	region_addr_t	start;
	region_addr_t	end;
	struct region_el *next;
};

struct region_struct {
	region_addr_t	min;
	region_addr_t	max;
	struct region_el *allocated;
	struct region_el *last;
};

region_t region_create(region_addr_t min, region_addr_t max)
{
	region_t	region;
	errcode_t	retval;

	retval = ext2fs_get_memzero(sizeof(struct region_struct), &region);
	if (retval)
		return NULL;

	region->min = min;
	region->max = max;
	region->last = NULL;
	return region;
}

void region_free(region_t region)
{
	struct region_el	*r, *next;

	for (r = region->allocated; r; r = next) {
		next = r->next;
		ext2fs_free_mem(&r);
	}
	memset(region, 0, sizeof(struct region_struct));
	ext2fs_free_mem(&region);
}

int region_allocate(region_t region, region_addr_t start, int n)
{
	struct region_el	*r, *new_region, *prev, *next;
	region_addr_t end;
	errcode_t retval;

	end = start+n;
	if ((start < region->min) || (end > region->max))
		return -1;
	if (n == 0)
		return 1;

	if (region->last && region->last->end == start &&
	    !region->last->next) {
		region->last->end = end;
		return 0;
	}
	if (region->last && start > region->last->end &&
	    !region->last->next) {
		r = NULL;
		prev = region->last;
		goto append_to_list;
	}

	/*
	 * Search through the linked list.  If we find that it
	 * conflicts with something that's already allocated, return
	 * 1; if we can find an existing region which we can grow, do
	 * so.  Otherwise, stop when we find the appropriate place
	 * insert a new region element into the linked list.
	 */
	for (r = region->allocated, prev=NULL; r; prev = r, r = r->next) {
		if (((start >= r->start) && (start < r->end)) ||
		    ((end > r->start) && (end <= r->end)) ||
		    ((start <= r->start) && (end >= r->end)))
			return 1;
		if (end == r->start) {
			r->start = start;
			return 0;
		}
		if (start == r->end) {
			if ((next = r->next)) {
				if (end > next->start)
					return 1;
				if (end == next->start) {
					r->end = next->end;
					r->next = next->next;
					ext2fs_free_mem(&next);
					if (!r->next)
						region->last = r;
					return 0;
				}
			}
			r->end = end;
			return 0;
		}
		if (start < r->start)
			break;
	}
	/*
	 * Insert a new region element structure into the linked list
	 */
append_to_list:
	retval = ext2fs_get_mem(sizeof(struct region_el), &new_region);
	if (retval)
		return -1;
	new_region->start = start;
	new_region->end = start + n;
	new_region->next = r;
	if (!new_region->next)
		region->last = new_region;
	if (prev)
		prev->next = new_region;
	else
		region->allocated = new_region;
	return 0;
}

#ifdef TEST_PROGRAM
#include <stdio.h>

#define BCODE_END	0
#define BCODE_CREATE	1
#define BCODE_FREE	2
#define BCODE_ALLOCATE	3
#define BCODE_PRINT	4

int bcode_program[] = {
	BCODE_CREATE, 1, 1001,
	BCODE_PRINT,
	BCODE_ALLOCATE, 10, 10,
	BCODE_ALLOCATE, 30, 10,
	BCODE_PRINT,
	BCODE_ALLOCATE, 1, 15,
	BCODE_ALLOCATE, 15, 8,
	BCODE_ALLOCATE, 1, 20,
	BCODE_ALLOCATE, 1, 8,
	BCODE_PRINT,
	BCODE_ALLOCATE, 40, 10,
	BCODE_PRINT,
	BCODE_ALLOCATE, 22, 5,
	BCODE_PRINT,
	BCODE_ALLOCATE, 27, 3,
	BCODE_PRINT,
	BCODE_ALLOCATE, 20, 2,
	BCODE_PRINT,
	BCODE_ALLOCATE, 49, 1,
	BCODE_ALLOCATE, 50, 5,
	BCODE_ALLOCATE, 9, 2,
	BCODE_ALLOCATE, 9, 1,
	BCODE_PRINT,
	BCODE_FREE,
	BCODE_END
};

void region_print(region_t region, FILE *f)
{
	struct region_el	*r;
	int	i = 0;

	fprintf(f, "Printing region (min=%llu. max=%llu)\n\t", region->min,
		region->max);
	for (r = region->allocated; r; r = r->next) {
		fprintf(f, "(%llu, %llu)  ", r->start, r->end);
		if (++i >= 8)
			fprintf(f, "\n\t");
	}
	fprintf(f, "\n");
}

int main(int argc, char **argv)
{
	region_t	r = NULL;
	int		pc = 0, ret;
	region_addr_t	start, end;


	while (1) {
		switch (bcode_program[pc++]) {
		case BCODE_END:
			exit(0);
		case BCODE_CREATE:
			start = bcode_program[pc++];
			end = bcode_program[pc++];
			printf("Creating region with args(%llu, %llu)\n",
			       start, end);
			r = region_create(start, end);
			if (!r) {
				fprintf(stderr, "Couldn't create region.\n");
				exit(1);
			}
			break;
		case BCODE_ALLOCATE:
			start = bcode_program[pc++];
			end = bcode_program[pc++];
			ret = region_allocate(r, start, end);
			printf("Region_allocate(%llu, %llu) returns %d\n",
			       start, end, ret);
			break;
		case BCODE_PRINT:
			region_print(r, stdout);
			break;
		}
	}
	if (r)
		region_free(r);
}

#endif /* TEST_PROGRAM */
