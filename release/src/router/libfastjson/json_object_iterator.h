/**
*******************************************************************************
* @file fjson_object_iterator.h
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the MIT license. See COPYING for details.
*
* @brief  json-c forces clients to use its private data
*         structures for JSON Object iteration.  This API
*         corrects that by abstracting the private json-c
*         details.
*
* API attributes: <br>
*   * Thread-safe: NO<br>
*   * Reentrant: NO
*
*******************************************************************************
*/


#ifndef FJ_JSON_OBJECT_ITERATOR_H
#define FJ_JSON_OBJECT_ITERATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Forward declaration for the opaque iterator information.
 */
struct fjson_object_iter_info_;

/**
 * The opaque iterator that references a name/value pair within
 * a JSON Object instance or the "end" iterator value.
 */
struct fjson_object_iterator {
	int objs_remain;
	int curr_idx;
	const struct _fjson_child_pg *pg;
};


/**
 * forward declaration of json-c's JSON value instance structure
 */
struct fjson_object;


/**
 * Initializes an iterator structure to a "default" value that
 * is convenient for initializing an iterator variable to a
 * default state (e.g., initialization list in a class'
 * constructor).
 *
 * @code
 * struct fjson_object_iterator iter = fjson_object_iter_init_default();
 * MyClass() : iter_(fjson_object_iter_init_default())
 * @endcode
 *
 * @note The initialized value doesn't reference any specific
 *       pair, is considered an invalid iterator, and MUST NOT
 *       be passed to any json-c API that expects a valid
 *       iterator.
 *
 * @note User and internal code MUST NOT make any assumptions
 *       about and dependencies on the value of the "default"
 *       iterator value.
 *
 * @return fjson_object_iterator
 */
struct fjson_object_iterator
fjson_object_iter_init_default(void);

/** Retrieves an iterator to the first pair of the JSON Object.
 *
 * @warning 	Any modification of the underlying pair invalidates all
 * 		iterators to that pair.
 *
 * @param obj	JSON Object instance (MUST be of type fjson_object)
 *
 * @return fjson_object_iterator If the JSON Object has at
 *              least one pair, on return, the iterator refers
 *              to the first pair. If the JSON Object doesn't
 *              have any pairs, the returned iterator is
 *              equivalent to the "end" iterator for the same
 *              JSON Object instance.
 *
 * @code
 * struct fjson_object_iterator it;
 * struct fjson_object_iterator itEnd;
 * struct fjson_object* obj;
 *
 * obj = fjson_tokener_parse("{'first':'george', 'age':100}");
 * it = fjson_object_iter_begin(obj);
 * itEnd = fjson_object_iter_end(obj);
 *
 * while (!fjson_object_iter_equal(&it, &itEnd)) {
 *     printf("%s\n",
 *            fjson_object_iter_peek_name(&it));
 *     fjson_object_iter_next(&it);
 * }
 *
 * @endcode
 */
struct fjson_object_iterator
fjson_object_iter_begin(struct fjson_object* obj);

/** Retrieves the iterator that represents the position beyond the
 *  last pair of the given JSON Object instance.
 *
 *  @warning Do NOT write code that assumes that the "end"
 *        iterator value is NULL, even if it is so in a
 *        particular instance of the implementation.
 *
 *  @note The reason we do not (and MUST NOT) provide
 *        "fjson_object_iter_is_end(fjson_object_iterator* iter)"
 *        type of API is because it would limit the underlying
 *        representation of name/value containment (or force us
 *        to add additional, otherwise unnecessary, fields to
 *        the iterator structure). The "end" iterator and the
 *        equality test method, on the other hand, permit us to
 *        cleanly abstract pretty much any reasonable underlying
 *        representation without burdening the iterator
 *        structure with unnecessary data.
 *
 *  @note For performance reasons, memorize the "end" iterator prior
 *        to any loop.
 *
 * @param obj JSON Object instance (MUST be of type fjson_object)
 *
 * @return fjson_object_iterator On return, the iterator refers
 *              to the "end" of the Object instance's pairs
 *              (i.e., NOT the last pair, but "beyond the last
 *              pair" value)
 */
struct fjson_object_iterator
fjson_object_iter_end(const struct fjson_object* obj);

/** Returns an iterator to the next pair, if any
 *
 * @warning	Any modification of the underlying pair
 *       	invalidates all iterators to that pair.
 *
 * @param iter [IN/OUT] Pointer to iterator that references a
 *         name/value pair; MUST be a valid, non-end iterator.
 *         WARNING: bad things will happen if invalid or "end"
 *         iterator is passed. Upon return will contain the
 *         reference to the next pair if there is one; if there
 *         are no more pairs, will contain the "end" iterator
 *         value, which may be compared against the return value
 *         of fjson_object_iter_end() for the same JSON Object
 *         instance.
 */
void
fjson_object_iter_next(struct fjson_object_iterator* iter);


/** Returns a const pointer to the name of the pair referenced
 *  by the given iterator.
 *
 * @param iter pointer to iterator that references a name/value
 *             pair; MUST be a valid, non-end iterator.
 *
 * @warning	bad things will happen if an invalid or
 *             	"end" iterator is passed.
 *
 * @return const char* Pointer to the name of the referenced
 *         name/value pair.  The name memory belongs to the
 *         name/value pair, will be freed when the pair is
 *         deleted or modified, and MUST NOT be modified or
 *         freed by the user.
 */
const char*
fjson_object_iter_peek_name(const struct fjson_object_iterator* iter);


/** Returns a pointer to the json-c instance representing the
 *  value of the referenced name/value pair, without altering
 *  the instance's reference count.
 *
 * @param iter 	pointer to iterator that references a name/value
 *             	pair; MUST be a valid, non-end iterator.
 *
 * @warning	bad things will happen if invalid or
 *             "end" iterator is passed.
 *
 * @return struct fjson_object* Pointer to the json-c value
 *         instance of the referenced name/value pair;  the
 *         value's reference count is not changed by this
 *         function: if you plan to hold on to this json-c node,
 *         take a look at fjson_object_get() and
 *         fjson_object_put(). IMPORTANT: json-c API represents
 *         the JSON Null value as a NULL fjson_object instance
 *         pointer.
 */
struct fjson_object*
fjson_object_iter_peek_value(const struct fjson_object_iterator* iter);


/** Tests two iterators for equality.  Typically used to test
 *  for end of iteration by comparing an iterator to the
 *  corresponding "end" iterator (that was derived from the same
 *  JSON Object instance).
 *
 *  @note The reason we do not (and MUST NOT) provide
 *        "fjson_object_iter_is_end(fjson_object_iterator* iter)"
 *        type of API is because it would limit the underlying
 *        representation of name/value containment (or force us
 *        to add additional, otherwise unnecessary, fields to
 *        the iterator structure). The equality test method, on
 *        the other hand, permits us to cleanly abstract pretty
 *        much any reasonable underlying representation.
 *
 * @param iter1 Pointer to first valid, non-NULL iterator
 * @param iter2 POinter to second valid, non-NULL iterator
 *
 * @warning	if a NULL iterator pointer or an uninitialized
 *       	or invalid iterator, or iterators derived from
 *       	different JSON Object instances are passed, bad things
 *       	will happen!
 *
 * @return fjson_bool non-zero if iterators are equal (i.e., both
 *         reference the same name/value pair or are both at
 *         "end"); zero if they are not equal.
 */
fjson_bool
fjson_object_iter_equal(const struct fjson_object_iterator* iter1,
			const struct fjson_object_iterator* iter2);

/* some private functions -- TODO: move to their own header */
struct _fjson_child*
_fjson_object_iter_peek_child(const struct fjson_object_iterator *const __restrict__ iter);


#ifndef FJSON_NATIVE_API_ONLY
#define json_object_iter_info_ fjson_object_iter_info_
#define json_object_iterator fjson_object_iterator
#define json_object_iter_init_default fjson_object_iter_init_default
#define json_object_iter_begin fjson_object_iter_begin
#define json_object_iter_end fjson_object_iter_end
#define json_object_iter_next fjson_object_iter_next
#define json_object_iter_peek_name fjson_object_iter_peek_name
#define json_object_iter_peek_value fjson_object_iter_peek_value
#define json_object_iter_equal fjson_object_iter_equal
#endif

#ifdef __cplusplus
}
#endif


#endif /* FJSON_OBJECT_ITERATOR_H */
