/*
 * Copyright (C) 2007-2011 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup linked_list linked_list
 * @{ @ingroup collections
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

typedef struct linked_list_t linked_list_t;

#include <collections/enumerator.h>

/**
 * Method to match elements in a linked list (used in find_* functions)
 *
 * @param item			current list item
 * @param ...			user supplied data (only pointers, at most 5)
 * @return
 *						- TRUE, if the item matched
 *						- FALSE, otherwise
 */
typedef bool (*linked_list_match_t)(void *item, ...);

/**
 * Method to be invoked on elements in a linked list (used in invoke_* functions)
 *
 * @param item			current list item
 * @param ...			user supplied data (only pointers, at most 5)
 */
typedef void (*linked_list_invoke_t)(void *item, ...);

/**
 * Class implementing a double linked list.
 *
 * General purpose linked list. This list is not synchronized.
 */
struct linked_list_t {

	/**
	 * Gets the count of items in the list.
	 *
	 * @return			number of items in list
	 */
	int (*get_count) (linked_list_t *this);

	/**
	 * Create an enumerator over the list.
	 *
	 * @note The enumerator's position is invalid before the first call
	 * to enumerate().
	 *
	 * @return			enumerator over list items
	 */
	enumerator_t* (*create_enumerator)(linked_list_t *this);

	/**
	 * Resets the enumerator's current position to the beginning of the list.
	 *
	 * @param enumerator	enumerator to reset
	 */
	void (*reset_enumerator)(linked_list_t *this, enumerator_t *enumerator);

	/**
	 * Inserts a new item at the beginning of the list.
	 *
	 * @param item		item value to insert in list
	 */
	void (*insert_first) (linked_list_t *this, void *item);

	/**
	 * Removes the first item in the list and returns its value.
	 *
	 * @param item		returned value of first item, or NULL
	 * @return			SUCCESS, or NOT_FOUND if list is empty
	 */
	status_t (*remove_first) (linked_list_t *this, void **item);

	/**
	 * Inserts a new item before the item the enumerator currently points to.
	 *
	 * If this method is called before starting the enumeration the item is
	 * inserted first. If it is called after all items have been enumerated
	 * the item is inserted last. This is helpful when inserting items into
	 * a sorted list.
	 *
	 * @note The position of the enumerator is not changed.
	 *
	 * @param enumerator	enumerator with position
	 * @param item			item value to insert in list
	 */
	void (*insert_before)(linked_list_t *this, enumerator_t *enumerator,
						  void *item);

	/**
	 * Remove an item from the list where the enumerator points to.
	 *
	 * @param enumerator enumerator with position
	 */
	void (*remove_at)(linked_list_t *this, enumerator_t *enumerator);

	/**
	 * Remove items from the list matching the given item.
	 *
	 * If a compare function is given, it is called for each item, with the
	 * first parameter being the current list item and the second parameter
	 * being the supplied item. Return TRUE from the compare function to remove
	 * the item, return FALSE to keep it in the list.
	 *
	 * If compare is NULL, comparison is done by pointers.
	 *
	 * @param item		item to remove/pass to comparator
	 * @param compare	compare function, or NULL
	 * @return			number of removed items
	 */
	int (*remove)(linked_list_t *this, void *item, bool (*compare)(void*,void*));

	/**
	 * Returns the value of the first list item without removing it.
	 *
	 * @param item		returned value of first item
	 * @return			SUCCESS, NOT_FOUND if list is empty
	 */
	status_t (*get_first) (linked_list_t *this, void **item);

	/**
	 * Inserts a new item at the end of the list.
	 *
	 * @param item		value to insert into list
	 */
	void (*insert_last) (linked_list_t *this, void *item);

	/**
	 * Removes the last item in the list and returns its value.
	 *
	 * @param item		returned value of last item, or NULL
	 * @return			SUCCESS, NOT_FOUND if list is empty
	 */
	status_t (*remove_last) (linked_list_t *this, void **item);

	/**
	 * Returns the value of the last list item without removing it.
	 *
	 * @param item		returned value of last item
	 * @return			SUCCESS, NOT_FOUND if list is empty
	 */
	status_t (*get_last) (linked_list_t *this, void **item);

	/**
	 * Find the first matching element in the list.
	 *
	 * The first object passed to the match function is the current list item,
	 * followed by the user supplied data.
	 * If the supplied function returns TRUE this function returns SUCCESS, and
	 * the current object is returned in the third parameter, otherwise,
	 * the next item is checked.
	 *
	 * If match is NULL, *item and the current object are compared.
	 *
	 * @warning Only use pointers as user supplied data.
	 *
	 * @param match			comparison function to call on each object, or NULL
	 * @param item			the list item, if found
	 * @param ...			user data to supply to match function (limited to 5 arguments)
	 * @return				SUCCESS if found, NOT_FOUND otherwise
	 */
	status_t (*find_first) (linked_list_t *this, linked_list_match_t match,
							void **item, ...);

	/**
	 * Invoke a method on all of the contained objects.
	 *
	 * If a linked list contains objects with function pointers,
	 * invoke() can call a method on each of the objects. The
	 * method is specified by an offset of the function pointer,
	 * which can be evalutated at compile time using the offsetof
	 * macro, e.g.: list->invoke(list, offsetof(object_t, method));
	 *
	 * @warning Only use pointers as user supplied data.
	 *
	 * @param offset	offset of the method to invoke on objects
	 * @param ...		user data to supply to called function (limited to 5 arguments)
	 */
	void (*invoke_offset) (linked_list_t *this, size_t offset, ...);

	/**
	 * Invoke a function on all of the contained objects.
	 *
	 * @warning Only use pointers as user supplied data.
	 *
	 * @param function	offset of the method to invoke on objects
	 * @param ...		user data to supply to called function (limited to 5 arguments)
	 */
	void (*invoke_function) (linked_list_t *this, linked_list_invoke_t function, ...);

	/**
	 * Clones a list and its objects using the objects' clone method.
	 *
	 * @param offset	offset ot the objects clone function
	 * @return			cloned list
	 */
	linked_list_t *(*clone_offset) (linked_list_t *this, size_t offset);

	/**
	 * Destroys a linked_list object.
	 */
	void (*destroy) (linked_list_t *this);

	/**
	 * Destroys a list and its objects using the destructor.
	 *
	 * If a linked list and the contained objects should be destroyed, use
	 * destroy_offset. The supplied offset specifies the destructor to
	 * call on each object. The offset may be calculated using the offsetof
	 * macro, e.g.: list->destroy_offset(list, offsetof(object_t, destroy));
	 *
	 * @param offset	offset of the objects destructor
	 */
	void (*destroy_offset) (linked_list_t *this, size_t offset);

	/**
	 * Destroys a list and its contents using a a cleanup function.
	 *
	 * If a linked list and its contents should get destroyed using a specific
	 * cleanup function, use destroy_function. This is useful when the
	 * list contains malloc()-ed blocks which should get freed,
	 * e.g.: list->destroy_function(list, free);
	 *
	 * @param function	function to call on each object
	 */
	void (*destroy_function) (linked_list_t *this, void (*)(void*));
};

/**
 * Creates an empty linked list object.
 *
 * @return		linked_list_t object.
 */
linked_list_t *linked_list_create(void);

/**
 * Creates a linked list from an enumerator.
 *
 * @param enumerator	enumerator over void*, gets destroyed
 * @return				linked_list_t object, containing enumerated values
 */
linked_list_t *linked_list_create_from_enumerator(enumerator_t *enumerator);

/**
 * Creates a linked list from a NULL terminated vararg list of items.
 *
 * @param first			first item
 * @param ...			subsequent items, terminated by NULL
 * @return				linked_list_t object, containing passed items
 */
linked_list_t *linked_list_create_with_items(void *first, ...);

#endif /** LINKED_LIST_H_ @}*/
