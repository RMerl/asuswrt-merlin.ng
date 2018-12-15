/**
*******************************************************************************
* @file fjson_object_iterator.c
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2016 Adiscon GmbH
* Rainer Gerhards <rgerhards@adiscon.com>
*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the MIT license. See COPYING for details.
*
* @brief  json-c forces clients to use its private data
*         structures for JSON Object iteration.  This API
*         implementation corrects that by abstracting the
*         private json-c details.
*
*******************************************************************************
*/

#include "config.h"
#include <stddef.h>
#include "json.h"
#include "json_object_private.h"
#include "json_object_iterator.h"
#include "debug.h"

/**
 * How It Works
 *
 * For each JSON Object, json-c maintains a linked list of zero
 * or more lh_entry (link-hash entry) structures inside the
 * Object's link-hash table (lh_table).
 *
 * Each lh_entry structure on the JSON Object's linked list
 * represents a single name/value pair.  The "next" field of the
 * last lh_entry in the list is set to NULL, which terminates
 * the list.
 *
 * We represent a valid iterator that refers to an actual
 * name/value pair via a pointer to the pair's lh_entry
 * structure set as the iterator's opaque_ field.
 *
 * We follow json-c's current pair list representation by
 * representing a valid "end" iterator (one that refers past the
 * last pair) with a NULL value in the iterator's opaque_ field.
 *
 * A JSON Object without any pairs in it will have the "head"
 * field of its lh_table structure set to NULL.  For such an
 * object, fjson_object_iter_begin will return an iterator with
 * the opaque_ field set to NULL, which is equivalent to the
 * "end" iterator.
 *
 * When iterating, we simply update the iterator's opaque_ field
 * to point to the next lh_entry structure in the linked list.
 * opaque_ will become NULL once we iterate past the last pair
 * in the list, which makes the iterator equivalent to the "end"
 * iterator.
 */


/**
 * ****************************************************************************
 */
struct fjson_object_iterator
fjson_object_iter_begin(struct fjson_object *const __restrict__ obj)
{
	struct fjson_object_iterator iter = {
		.objs_remain = 0,
		.curr_idx = 0,
		.pg = NULL
		};

	if(obj->o_type == fjson_type_object) {
		iter.objs_remain = obj->o.c_obj.nelem;
		if(iter.objs_remain > 0) {
			iter.curr_idx = 0;
			iter.pg = &obj->o.c_obj.pg;
			/* check if first slot is empty, if so, advance */
			if(iter.pg->children[0].k == NULL) {
				++iter.objs_remain; /* correct _iter_next decrement */
				fjson_object_iter_next(&iter);
			}
		}
	}
	return iter;
}

/**
 * ****************************************************************************
 */
struct fjson_object_iterator
fjson_object_iter_end(const struct fjson_object __attribute__((unused)) *obj)
{
	struct fjson_object_iterator iter = {
		.objs_remain = 0,
		.curr_idx = 0,
		.pg = NULL
		};
	return iter;
}

/**
 * ****************************************************************************
 */
void
fjson_object_iter_next(struct fjson_object_iterator *const __restrict__ iter)
{
	JASSERT(NULL != iter);

	if(iter->objs_remain > 0) {
		--iter->objs_remain;
		if(iter->objs_remain > 0) {
			++iter->curr_idx;
			if(iter->curr_idx == FJSON_OBJECT_CHLD_PG_SIZE) {
				iter->pg = iter->pg->next;
				iter->curr_idx = 0;
			}
			/* check empty slots; TODO: recurse or iterate? */
			if(iter->pg->children[iter->curr_idx].k == NULL) {
				++iter->objs_remain; /* correct */
				fjson_object_iter_next(iter);
			}
		}
	}
}


/**
 * ****************************************************************************
 */
const char*
fjson_object_iter_peek_name(const struct fjson_object_iterator *const __restrict__ iter)
{
	JASSERT(NULL != iter);
	return iter->pg->children[iter->curr_idx].k;
}


/**
 * ****************************************************************************
 */
struct fjson_object*
fjson_object_iter_peek_value(const struct fjson_object_iterator *const __restrict__ iter)
{
	JASSERT(NULL != iter);
	return iter->pg->children[iter->curr_idx].v;
}

/**
 * ****************************************************************************
 */
struct _fjson_child*
_fjson_object_iter_peek_child(const struct fjson_object_iterator *const __restrict__ iter)
{
	JASSERT(NULL != iter);
	return (struct _fjson_child*) &(iter->pg->children[iter->curr_idx]);
}


/**
 * ****************************************************************************
 */
fjson_bool
fjson_object_iter_equal(const struct fjson_object_iterator* iter1,
	const struct fjson_object_iterator* iter2)
{
	int is_eq;
	JASSERT(NULL != iter1);
	JASSERT(NULL != iter2);

	if (iter1->objs_remain == iter2->objs_remain) {
		if (iter1->objs_remain == 0) {
			is_eq = 1;
		} else {
			if ( (iter1->curr_idx == iter2->curr_idx) &&
			     (iter1->pg == iter2->pg)                ) {
				is_eq = 1;
			} else {
				is_eq = 0;
			}
		}
	} else {
		is_eq= 0;
	}

	return is_eq;
}


/**
 * ****************************************************************************
 */
struct fjson_object_iterator
fjson_object_iter_init_default(void)
{
	struct fjson_object_iterator iter;

	/**
	* @note Make this an invalid value, such that
	*       accidental access to it would likely be trapped by the
	*       hardware as an invalid address.
	*/
	iter.pg = NULL;
	iter.curr_idx = 0;
	iter.objs_remain = 1;

	return iter;
}
