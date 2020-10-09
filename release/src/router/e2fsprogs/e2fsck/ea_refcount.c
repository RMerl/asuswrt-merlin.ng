/*
 * ea_refcount.c
 *
 * Copyright (C) 2001 Theodore Ts'o.  This file may be
 * redistributed under the terms of the GNU Public License.
 */

#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>

#ifdef TEST_PROGRAM
#undef ENABLE_NLS
#endif
#include "e2fsck.h"

/*
 * The strategy we use for keeping track of EA refcounts is as
 * follows.  We keep a sorted array of first EA blocks and its
 * reference counts.  Once the refcount has dropped to zero, it is
 * removed from the array to save memory space.  Once the EA block is
 * checked, its bit is set in the block_ea_map bitmap.
 */
struct ea_refcount_el {
	/* ea_key could either be an inode number or block number. */
	ea_key_t	ea_key;
	ea_value_t	ea_value;
};

struct ea_refcount {
	size_t		count;
	size_t		size;
	size_t		cursor;
	struct ea_refcount_el	*list;
};

void ea_refcount_free(ext2_refcount_t refcount)
{
	if (!refcount)
		return;

	if (refcount->list)
		ext2fs_free_mem(&refcount->list);
	ext2fs_free_mem(&refcount);
}

errcode_t ea_refcount_create(size_t size, ext2_refcount_t *ret)
{
	ext2_refcount_t	refcount;
	errcode_t	retval;
	size_t		bytes;

	retval = ext2fs_get_memzero(sizeof(struct ea_refcount), &refcount);
	if (retval)
		return retval;

	if (!size)
		size = 500;
	refcount->size = size;
	bytes = size * sizeof(struct ea_refcount_el);
#ifdef DEBUG
	printf("Refcount allocated %zu entries, %zu bytes.\n",
	       refcount->size, bytes);
#endif
	retval = ext2fs_get_memzero(bytes, &refcount->list);
	if (retval)
		goto errout;

	refcount->count = 0;
	refcount->cursor = 0;

	*ret = refcount;
	return 0;

errout:
	ea_refcount_free(refcount);
	return(retval);
}

/*
 * collapse_refcount() --- go through the refcount array, and get rid
 * of any count == zero entries
 */
static void refcount_collapse(ext2_refcount_t refcount)
{
	unsigned int	i, j;
	struct ea_refcount_el	*list;

	list = refcount->list;
	for (i = 0, j = 0; i < refcount->count; i++) {
		if (list[i].ea_value) {
			if (i != j)
				list[j] = list[i];
			j++;
		}
	}
#if defined(DEBUG) || defined(TEST_PROGRAM)
	printf("Refcount_collapse: size was %zu, now %d\n",
	       refcount->count, j);
#endif
	refcount->count = j;
}


/*
 * insert_refcount_el() --- Insert a new entry into the sorted list at a
 * 	specified position.
 */
static struct ea_refcount_el *insert_refcount_el(ext2_refcount_t refcount,
						 ea_key_t ea_key, int pos)
{
	struct ea_refcount_el 	*el;
	errcode_t		retval;
	size_t			new_size = 0;
	int			num;

	if (refcount->count >= refcount->size) {
		new_size = refcount->size + 100;
#ifdef DEBUG
		printf("Reallocating refcount %d entries...\n", new_size);
#endif
		retval = ext2fs_resize_mem((size_t) refcount->size *
					   sizeof(struct ea_refcount_el),
					   (size_t) new_size *
					   sizeof(struct ea_refcount_el),
					   &refcount->list);
		if (retval)
			return 0;
		refcount->size = new_size;
	}
	num = (int) refcount->count - pos;
	if (num < 0)
		return 0;	/* should never happen */
	if (num) {
		memmove(&refcount->list[pos+1], &refcount->list[pos],
			sizeof(struct ea_refcount_el) * num);
	}
	refcount->count++;
	el = &refcount->list[pos];
	el->ea_key = ea_key;
	el->ea_value = 0;
	return el;
}


/*
 * get_refcount_el() --- given an block number, try to find refcount
 * 	information in the sorted list.  If the create flag is set,
 * 	and we can't find an entry, create one in the sorted list.
 */
static struct ea_refcount_el *get_refcount_el(ext2_refcount_t refcount,
					      ea_key_t ea_key, int create)
{
	int	low, high, mid;

	if (!refcount || !refcount->list)
		return 0;
retry:
	low = 0;
	high = (int) refcount->count-1;
	if (create && ((refcount->count == 0) ||
		       (ea_key > refcount->list[high].ea_key))) {
		if (refcount->count >= refcount->size)
			refcount_collapse(refcount);

		return insert_refcount_el(refcount, ea_key,
					  (unsigned) refcount->count);
	}
	if (refcount->count == 0)
		return 0;

	if (refcount->cursor >= refcount->count)
		refcount->cursor = 0;
	if (ea_key == refcount->list[refcount->cursor].ea_key)
		return &refcount->list[refcount->cursor++];
#ifdef DEBUG
	printf("Non-cursor get_refcount_el: %u\n", ea_key);
#endif
	while (low <= high) {
		mid = (low+high)/2;
		if (ea_key == refcount->list[mid].ea_key) {
			refcount->cursor = mid+1;
			return &refcount->list[mid];
		}
		if (ea_key < refcount->list[mid].ea_key)
			high = mid-1;
		else
			low = mid+1;
	}
	/*
	 * If we need to create a new entry, it should be right at
	 * low (where high will be left at low-1).
	 */
	if (create) {
		if (refcount->count >= refcount->size) {
			refcount_collapse(refcount);
			if (refcount->count < refcount->size)
				goto retry;
		}
		return insert_refcount_el(refcount, ea_key, low);
	}
	return 0;
}

errcode_t ea_refcount_fetch(ext2_refcount_t refcount, ea_key_t ea_key,
			    ea_value_t *ret)
{
	struct ea_refcount_el	*el;

	el = get_refcount_el(refcount, ea_key, 0);
	if (!el) {
		*ret = 0;
		return 0;
	}
	*ret = el->ea_value;
	return 0;
}

errcode_t ea_refcount_increment(ext2_refcount_t refcount, ea_key_t ea_key,
				ea_value_t *ret)
{
	struct ea_refcount_el	*el;

	el = get_refcount_el(refcount, ea_key, 1);
	if (!el)
		return EXT2_ET_NO_MEMORY;
	el->ea_value++;

	if (ret)
		*ret = el->ea_value;
	return 0;
}

errcode_t ea_refcount_decrement(ext2_refcount_t refcount, ea_key_t ea_key,
				ea_value_t *ret)
{
	struct ea_refcount_el	*el;

	el = get_refcount_el(refcount, ea_key, 0);
	if (!el || el->ea_value == 0)
		return EXT2_ET_INVALID_ARGUMENT;

	el->ea_value--;

	if (ret)
		*ret = el->ea_value;
	return 0;
}

errcode_t ea_refcount_store(ext2_refcount_t refcount, ea_key_t ea_key,
			    ea_value_t ea_value)
{
	struct ea_refcount_el	*el;

	/*
	 * Get the refcount element
	 */
	el = get_refcount_el(refcount, ea_key, ea_value ? 1 : 0);
	if (!el)
		return ea_value ? EXT2_ET_NO_MEMORY : 0;
	el->ea_value = ea_value;
	return 0;
}

size_t ext2fs_get_refcount_size(ext2_refcount_t refcount)
{
	if (!refcount)
		return 0;

	return refcount->size;
}

void ea_refcount_intr_begin(ext2_refcount_t refcount)
{
	refcount->cursor = 0;
}

ea_key_t ea_refcount_intr_next(ext2_refcount_t refcount,
				ea_value_t *ret)
{
	struct ea_refcount_el	*list;

	while (1) {
		if (refcount->cursor >= refcount->count)
			return 0;
		list = refcount->list;
		if (list[refcount->cursor].ea_value) {
			if (ret)
				*ret = list[refcount->cursor].ea_value;
			return list[refcount->cursor++].ea_key;
		}
		refcount->cursor++;
	}
}


#ifdef TEST_PROGRAM

errcode_t ea_refcount_validate(ext2_refcount_t refcount, FILE *out)
{
	errcode_t	ret = 0;
	int		i;
	const char *bad = "bad refcount";

	if (refcount->count > refcount->size) {
		fprintf(out, "%s: count > size\n", bad);
		return EXT2_ET_INVALID_ARGUMENT;
	}
	for (i=1; i < refcount->count; i++) {
		if (refcount->list[i-1].ea_key >= refcount->list[i].ea_key) {
			fprintf(out,
				"%s: list[%d].ea_key=%llu, list[%d].ea_key=%llu\n",
				bad, i-1, refcount->list[i-1].ea_key, i,
				refcount->list[i].ea_key);
			ret = EXT2_ET_INVALID_ARGUMENT;
		}
	}
	return ret;
}

#define BCODE_END	0
#define BCODE_CREATE	1
#define BCODE_FREE	2
#define BCODE_STORE	3
#define BCODE_INCR	4
#define BCODE_DECR	5
#define BCODE_FETCH	6
#define BCODE_VALIDATE	7
#define BCODE_LIST	8
#define BCODE_COLLAPSE 9

int bcode_program[] = {
	BCODE_CREATE, 5,
	BCODE_STORE, 3, 3,
	BCODE_STORE, 4, 4,
	BCODE_STORE, 1, 1,
	BCODE_STORE, 8, 8,
	BCODE_STORE, 2, 2,
	BCODE_STORE, 4, 0,
	BCODE_STORE, 2, 0,
	BCODE_STORE, 6, 6,
	BCODE_VALIDATE,
	BCODE_STORE, 4, 4,
	BCODE_STORE, 2, 2,
	BCODE_FETCH, 1,
	BCODE_FETCH, 2,
	BCODE_INCR, 3,
	BCODE_INCR, 3,
	BCODE_DECR, 4,
	BCODE_STORE, 4, 4,
	BCODE_VALIDATE,
	BCODE_STORE, 20, 20,
	BCODE_STORE, 40, 40,
	BCODE_STORE, 30, 30,
	BCODE_STORE, 10, 10,
	BCODE_DECR, 30,
	BCODE_FETCH, 30,
	BCODE_DECR, 2,
	BCODE_DECR, 2,
	BCODE_COLLAPSE,
	BCODE_LIST,
	BCODE_VALIDATE,
	BCODE_FREE,
	BCODE_END
};

int main(int argc, char **argv)
{
	int	i = 0;
	ext2_refcount_t refcount;
	size_t		size;
	ea_key_t	ea_key;
	ea_value_t	arg;
	errcode_t	retval;

	while (1) {
		switch (bcode_program[i++]) {
		case BCODE_END:
			exit(0);
		case BCODE_CREATE:
			size = bcode_program[i++];
			retval = ea_refcount_create(size, &refcount);
			if (retval) {
				com_err("ea_refcount_create", retval,
					"while creating size %zu", size);
				exit(1);
			} else
				printf("Creating refcount with size %zu\n",
				       size);
			break;
		case BCODE_FREE:
			ea_refcount_free(refcount);
			refcount = 0;
			printf("Freeing refcount\n");
			break;
		case BCODE_STORE:
			ea_key = (size_t) bcode_program[i++];
			arg = bcode_program[i++];
			printf("Storing ea_key %llu with value %llu\n", ea_key,
			       arg);
			retval = ea_refcount_store(refcount, ea_key, arg);
			if (retval)
				com_err("ea_refcount_store", retval,
					"while storing ea_key %llu", ea_key);
			break;
		case BCODE_FETCH:
			ea_key = (size_t) bcode_program[i++];
			retval = ea_refcount_fetch(refcount, ea_key, &arg);
			if (retval)
				com_err("ea_refcount_fetch", retval,
					"while fetching ea_key %llu", ea_key);
			else
				printf("bcode_fetch(%llu) returns %llu\n",
				       ea_key, arg);
			break;
		case BCODE_INCR:
			ea_key = (size_t) bcode_program[i++];
			retval = ea_refcount_increment(refcount, ea_key, &arg);
			if (retval)
				com_err("ea_refcount_increment", retval,
					"while incrementing ea_key %llu",
					ea_key);
			else
				printf("bcode_increment(%llu) returns %llu\n",
				       ea_key, arg);
			break;
		case BCODE_DECR:
			ea_key = (size_t) bcode_program[i++];
			retval = ea_refcount_decrement(refcount, ea_key, &arg);
			if (retval)
				com_err("ea_refcount_decrement", retval,
					"while decrementing ea_key %llu",
					ea_key);
			else
				printf("bcode_decrement(%llu) returns %llu\n",
				       ea_key, arg);
			break;
		case BCODE_VALIDATE:
			retval = ea_refcount_validate(refcount, stderr);
			if (retval)
				com_err("ea_refcount_validate", retval,
					"while validating");
			else
				printf("Refcount validation OK.\n");
			break;
		case BCODE_LIST:
			ea_refcount_intr_begin(refcount);
			while (1) {
				ea_key = ea_refcount_intr_next(refcount, &arg);
				if (!ea_key)
					break;
				printf("\tea_key=%llu, count=%llu\n", ea_key,
				       arg);
			}
			break;
		case BCODE_COLLAPSE:
			refcount_collapse(refcount);
			break;
		}

	}
}

#endif
