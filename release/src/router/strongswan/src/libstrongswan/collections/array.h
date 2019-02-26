/*
 * Copyright (C) 2014 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup array array
 * @{ @ingroup collections
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include <collections/enumerator.h>

/**
 * Variable sized array with fixed size elements.
 *
 * An array is a primitive object with associated functions to avoid the
 * overhead of an object with methods. It is efficient in memory usage, but
 * less efficient than a linked list in manipulating elements.
 */
typedef struct array_t array_t;

typedef enum array_idx_t array_idx_t;

/**
 * Special array index values for insert/remove.
 */
enum array_idx_t {
	ARRAY_HEAD = 0,
	ARRAY_TAIL = -1,
};

/**
 * Callback function invoked for each array element.
 *
 * Data is a pointer to the array element. If this is a pointer based array,
 * (esize is zero), data is the pointer itself.
 *
 * @param data			pointer to array data, or the pointer itself
 * @param idx			array index
 * @param user			user data passed with callback
 */
typedef void (*array_callback_t)(void *data, int idx, void *user);

/**
 * Create a array instance.
 *
 * Elements get tight packed to each other. If any alignment is required, pass
 * appropriate padding to each element. The reserved space does not affect
 * array_count(), but just preallocates buffer space.
 *
 * @param esize			element size for this array, use 0 for a pointer array
 * @param reserve		number of items to allocate space for
 * @return				array instance
 */
array_t *array_create(u_int esize, uint8_t reserve);

/**
 * Get the number of elements currently in the array.
 *
 * @return				number of elements
 */
int array_count(array_t *array);

/**
 * Compress an array, remove unused head/tail space.
 *
 * @param array			array to compress, or NULL
 */
void array_compress(array_t *array);

/**
 * Create an enumerator over an array.
 *
 * The enumerater enumerates directly over the array element (pass a pointer to
 * element types), unless the array is pointer based. If zero is passed as
 * element size during construction, the enumerator enumerates over the
 * dereferenced pointer values.
 *
 * @param array			array to create enumerator for, or NULL
 * @return				enumerator, over elements or pointers
 */
enumerator_t* array_create_enumerator(array_t *array);

/**
 * Remove an element at enumerator position.
 *
 * @warning For **value based** arrays don't use the pointer returned by
 * enumerate() anymore after calling this function.  For performance reasons
 * that pointer will point to internal data structures that get modified when
 * this function is called.
 *
 * @param array			array to remove element in
 * @param enumerator	enumerator position, from array_create_enumerator()
 */
void array_remove_at(array_t *array, enumerator_t *enumerator);

/**
 * Insert an element to an array.
 *
 * If the array is pointer based (esize = 0), the pointer itself is appended.
 * Otherwise the element gets copied from the pointer.
 * The idx must be either within array_count() or one above to append the item.
 * Passing -1 has the same effect as passing array_count(), i.e. appends the
 * item. It is always valid to pass idx 0 to prepend the item.
 *
 * @param array			array to append element to
 * @param idx			index to insert item at
 * @param data			pointer to array element to copy
 */
void array_insert(array_t *array, int idx, void *data);

/**
 * Create an pointer based array if it does not exist, insert pointer.
 *
 * This is a convenience function for insert a pointer and implicitly
 * create a pointer based array if array is NULL. Array is set the the newly
 * created array, if any.
 *
 * @param array			pointer to array reference, potentially NULL
 * @param idx			index to insert item at
 * @param ptr			pointer to append
 */
void array_insert_create(array_t **array, int idx, void *ptr);

/**
 * Create a value based array if it does not exist, insert value.
 *
 * This is a convenience function to insert a value and implicitly
 * create a value based array if array is NULL. Array is set the the newly
 * created array, if any.
 *
 * @param array			pointer to array reference, potentially NULL
 * @param esize			element size of this array
 * @param idx			index to insert item at
 * @param val			pointer to value to insert
 */
void array_insert_create_value(array_t **array, u_int esize,
							   int idx, void *val);

/**
 * Insert all items from an enumerator to an array.
 *
 * @param array			array to add items to
 * @param idx			index to insert each item with
 * @param enumerator	enumerator over void*, gets destroyed
 */
void array_insert_enumerator(array_t *array, int idx, enumerator_t *enumerator);

/**
 * Get an element from the array.
 *
 * If data is given, the element is copied to that position.
 *
 * @param array			array to get element from, or NULL
 * @param idx			index of the item to get
 * @param data			data to copy element to, or NULL
 * @return				TRUE if idx valid and item returned
 */
bool array_get(array_t *array, int idx, void *data);

/**
 * Remove an element from the array.
 *
 * If data is given, the element is copied to that position.
 *
 * @param array			array to remove element from, or NULL
 * @param idx			index of the item to remove
 * @param data			data to copy element to, or NULL
 * @return				TRUE if idx existed and item removed
 */
bool array_remove(array_t *array, int idx, void *data);

/**
 * Sort the array.
 *
 * The comparison function must return an integer less than, equal to, or
 * greater than zero if the first argument is considered to be respectively less
 * than, equal to, or greater than the second.  If two elements compare as
 * equal, their order in the sorted array is undefined.
 *
 * The comparison function receives pointers to the array elements (esize != 0)
 * or the actual pointers (esize = 0). The third argument is the user data
 * supplied to this function.
 *
 * @param array			array to sort, or NULL
 * @param cmp			comparison function
 * @param user			user data to pass to comparison function
 */
void array_sort(array_t *array, int (*cmp)(const void*,const void*,void*),
				void *user);

/**
 * Binary search of a sorted array.
 *
 * The array should be sorted in ascending order according to the given
 * comparison function.
 *
 * The comparison function must return an integer less than, equal to, or
 * greater than zero if the first argument (the key) is considered to be
 * respectively less than, equal to, or greater than the second.
 *
 * If there are multiple elements that match the key it is not specified which
 * element is returned.
 *
 * The comparison function receives the key object and a pointer to an array
 * element (esize != 0) or an actual pointer (esize = 0).
 *
 * @param array			array to search, or NULL
 * @param key			key to search for
 * @param cmp			comparison function
 * @param data			data to copy element to, or NULL
 * @return				index of the element if found, -1 if not
 */
int array_bsearch(array_t *array, const void *key,
				  int (*cmp)(const void*,const void*), void *data);

/**
 * Invoke a callback for all array members.
 *
 * @param array			array to traverse, or NULL
 * @param cb			callback function to invoke each element with
 * @param user			user data to pass to callback
 */
void array_invoke(array_t *array, array_callback_t cb, void *user);

/**
 * Invoke a method of each element defined with offset.
 *
 * @param array			array to traverse, or NULL
 * @param offset		offset of element method, use offsetof()
 */
void array_invoke_offset(array_t *array, size_t offset);

/**
 * Destroy an array.
 *
 * @param array			array to destroy, or NULL
 */
void array_destroy(array_t *array);

/**
 * Destroy an array, call a function to clean up all elements.
 *
 * @param array			array to destroy, or NULL
 * @param cb			callback function to free element data
 * @param user			user data to pass to callback
 */
void array_destroy_function(array_t *array, array_callback_t cb, void *user);

/**
 * Destroy an array, call element method defined with offset.
 *
 * @param array			array to destroy, or NULL
 * @param offset		offset of element method, use offsetof()
 */
void array_destroy_offset(array_t *array, size_t offset);


/**
 * Required on some platforms to initialize thread local value to implement
 * array_sort().
 */
void arrays_init();

/**
 * Destroys the thread local value if required.
 */
void arrays_deinit();

#endif /** ARRAY_H_ @}*/
