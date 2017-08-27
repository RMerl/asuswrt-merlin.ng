/*
 * Copyright (C) 2007-2011 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
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

#include <stdlib.h>
#include <stdarg.h>

#include "linked_list.h"

typedef struct element_t element_t;

/**
 * This element holds a pointer to the value it represents.
 */
struct element_t {

	/**
	 * Value of a list item.
	 */
	void *value;

	/**
	 * Previous list element.
	 *
	 * NULL if first element in list.
	 */
	element_t *previous;

	/**
	 * Next list element.
	 *
	 * NULL if last element in list.
	 */
	element_t *next;
};

/**
 * Creates an empty linked list object.
 */
element_t *element_create(void *value)
{
	element_t *this;
	INIT(this,
		.value = value,
	);
	return this;
}


typedef struct private_linked_list_t private_linked_list_t;

/**
 * Private data of a linked_list_t object.
 *
 */
struct private_linked_list_t {
	/**
	 * Public part of linked list.
	 */
	linked_list_t public;

	/**
	 * Number of items in the list.
	 */
	int count;

	/**
	 * First element in list.
	 * NULL if no elements in list.
	 */
	element_t *first;

	/**
	 * Last element in list.
	 * NULL if no elements in list.
	 */
	element_t *last;
};

typedef struct private_enumerator_t private_enumerator_t;

/**
 * linked lists enumerator implementation
 */
struct private_enumerator_t {

	/**
	 * implements enumerator interface
	 */
	enumerator_t enumerator;

	/**
	 * associated linked list
	 */
	private_linked_list_t *list;

	/**
	 * current item
	 */
	element_t *current;

	/**
	 * enumerator has enumerated all items
	 */
	bool finished;
};

METHOD(enumerator_t, enumerate, bool,
	private_enumerator_t *this, void **item)
{
	if (this->finished)
	{
		return FALSE;
	}
	if (!this->current)
	{
		this->current = this->list->first;
	}
	else
	{
		this->current = this->current->next;
	}
	if (!this->current)
	{
		this->finished = TRUE;
		return FALSE;
	}
	if (item)
	{
		*item = this->current->value;
	}
	return TRUE;
}

METHOD(linked_list_t, create_enumerator, enumerator_t*,
	private_linked_list_t *this)
{
	private_enumerator_t *enumerator;

	INIT(enumerator,
		.enumerator = {
			.enumerate = (void*)_enumerate,
			.destroy = (void*)free,
		},
		.list = this,
	);

	return &enumerator->enumerator;
}

METHOD(linked_list_t, reset_enumerator, void,
	private_linked_list_t *this, private_enumerator_t *enumerator)
{
	enumerator->current = NULL;
	enumerator->finished = FALSE;
}

METHOD(linked_list_t, get_count, int,
	private_linked_list_t *this)
{
	return this->count;
}

METHOD(linked_list_t, insert_first, void,
	private_linked_list_t *this, void *item)
{
	element_t *element;

	element = element_create(item);
	if (this->count == 0)
	{
		/* first entry in list */
		this->first = element;
		this->last = element;
	}
	else
	{
		element->next = this->first;
		this->first->previous = element;
		this->first = element;
	}
	this->count++;
}

/**
 * unlink an element form the list, returns following element
 */
static element_t* remove_element(private_linked_list_t *this,
								 element_t *element)
{
	element_t *next, *previous;

	next = element->next;
	previous = element->previous;
	free(element);
	if (next)
	{
		next->previous = previous;
	}
	else
	{
		this->last = previous;
	}
	if (previous)
	{
		previous->next = next;
	}
	else
	{
		this->first = next;
	}
	if (--this->count == 0)
	{
		this->first = NULL;
		this->last = NULL;
	}
	return next;
}

METHOD(linked_list_t, get_first, status_t,
	private_linked_list_t *this, void **item)
{
	if (this->count == 0)
	{
		return NOT_FOUND;
	}
	*item = this->first->value;
	return SUCCESS;
}

METHOD(linked_list_t, remove_first, status_t,
	private_linked_list_t *this, void **item)
{
	if (get_first(this, item) == SUCCESS)
	{
		remove_element(this, this->first);
		return SUCCESS;
	}
	return NOT_FOUND;
}

METHOD(linked_list_t, insert_last, void,
	private_linked_list_t *this, void *item)
{
	element_t *element;

	element = element_create(item);
	if (this->count == 0)
	{
		/* first entry in list */
		this->first = element;
		this->last = element;
	}
	else
	{
		element->previous = this->last;
		this->last->next = element;
		this->last = element;
	}
	this->count++;
}

METHOD(linked_list_t, insert_before, void,
	private_linked_list_t *this, private_enumerator_t *enumerator,
	void *item)
{
	element_t *current, *element;

	current = enumerator->current;
	if (!current)
	{
		if (enumerator->finished)
		{
			this->public.insert_last(&this->public, item);
		}
		else
		{
			this->public.insert_first(&this->public, item);
		}
		return;
	}
	element = element_create(item);
	if (current->previous)
	{
		current->previous->next = element;
		element->previous = current->previous;
		current->previous = element;
		element->next = current;
	}
	else
	{
		current->previous = element;
		element->next = current;
		this->first = element;
	}
	this->count++;
}

METHOD(linked_list_t, get_last, status_t,
	private_linked_list_t *this, void **item)
{
	if (this->count == 0)
	{
		return NOT_FOUND;
	}
	*item = this->last->value;
	return SUCCESS;
}

METHOD(linked_list_t, remove_last, status_t,
	private_linked_list_t *this, void **item)
{
	if (get_last(this, item) == SUCCESS)
	{
		remove_element(this, this->last);
		return SUCCESS;
	}
	return NOT_FOUND;
}

METHOD(linked_list_t, remove_, int,
	private_linked_list_t *this, void *item, bool (*compare)(void*,void*))
{
	element_t *current = this->first;
	int removed = 0;

	while (current)
	{
		if ((compare && compare(current->value, item)) ||
			(!compare && current->value == item))
		{
			removed++;
			current = remove_element(this, current);
		}
		else
		{
			current = current->next;
		}
	}
	return removed;
}

METHOD(linked_list_t, remove_at, void,
	   private_linked_list_t *this, private_enumerator_t *enumerator)
{
	element_t *current;

	if (enumerator->current)
	{
		current = enumerator->current;
		enumerator->current = current->previous;
		remove_element(this, current);
	}
}

METHOD(linked_list_t, find_first, status_t,
	private_linked_list_t *this, linked_list_match_t match,
	void **item, void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;

	while (current)
	{
		if ((match && match(current->value, d1, d2, d3, d4, d5)) ||
			(!match && item && current->value == *item))
		{
			if (item != NULL)
			{
				*item = current->value;
			}
			return SUCCESS;
		}
		current = current->next;
	}
	return NOT_FOUND;
}

METHOD(linked_list_t, invoke_offset, void,
	private_linked_list_t *this, size_t offset,
	void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;
	linked_list_invoke_t *method;

	while (current)
	{
		method = current->value + offset;
		(*method)(current->value, d1, d2, d3, d4, d5);
		current = current->next;
	}
}

METHOD(linked_list_t, invoke_function, void,
	private_linked_list_t *this, linked_list_invoke_t fn,
	void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;

	while (current)
	{
		fn(current->value, d1, d2, d3, d4, d5);
		current = current->next;
	}
}

METHOD(linked_list_t, clone_offset, linked_list_t*,
	private_linked_list_t *this, size_t offset)
{
	element_t *current = this->first;
	linked_list_t *clone;

	clone = linked_list_create();
	while (current)
	{
		void* (**method)(void*) = current->value + offset;
		clone->insert_last(clone, (*method)(current->value));
		current = current->next;
	}

	return clone;
}

METHOD(linked_list_t, destroy, void,
	private_linked_list_t *this)
{
	void *value;

	/* Remove all list items before destroying list */
	while (remove_first(this, &value) == SUCCESS)
	{
		/* values are not destroyed so memory leaks are possible
		 * if list is not empty when deleting */
	}
	free(this);
}

METHOD(linked_list_t, destroy_offset, void,
	private_linked_list_t *this, size_t offset)
{
	element_t *current = this->first, *next;

	while (current)
	{
		void (**method)(void*) = current->value + offset;
		(*method)(current->value);
		next = current->next;
		free(current);
		current = next;
	}
	free(this);
}

METHOD(linked_list_t, destroy_function, void,
	private_linked_list_t *this, void (*fn)(void*))
{
	element_t *current = this->first, *next;

	while (current)
	{
		fn(current->value);
		next = current->next;
		free(current);
		current = next;
	}
	free(this);
}

/*
 * Described in header.
 */
linked_list_t *linked_list_create()
{
	private_linked_list_t *this;

	INIT(this,
		.public = {
			.get_count = _get_count,
			.create_enumerator = _create_enumerator,
			.reset_enumerator = (void*)_reset_enumerator,
			.get_first = _get_first,
			.get_last = _get_last,
			.find_first = (void*)_find_first,
			.insert_first = _insert_first,
			.insert_last = _insert_last,
			.insert_before = (void*)_insert_before,
			.remove_first = _remove_first,
			.remove_last = _remove_last,
			.remove = _remove_,
			.remove_at = (void*)_remove_at,
			.invoke_offset = (void*)_invoke_offset,
			.invoke_function = (void*)_invoke_function,
			.clone_offset = _clone_offset,
			.destroy = _destroy,
			.destroy_offset = _destroy_offset,
			.destroy_function = _destroy_function,
		},
	);

	return &this->public;
}

/*
 * See header.
 */
linked_list_t *linked_list_create_from_enumerator(enumerator_t *enumerator)
{
	linked_list_t *list;
	void *item;

	list = linked_list_create();

	while (enumerator->enumerate(enumerator, &item))
	{
		list->insert_last(list, item);
	}
	enumerator->destroy(enumerator);

	return list;
}

/*
 * See header.
 */
linked_list_t *linked_list_create_with_items(void *item, ...)
{
	linked_list_t *list;
	va_list args;

	list = linked_list_create();

	va_start(args, item);
	while (item)
	{
		list->insert_last(list, item);
		item = va_arg(args, void*);
	}
	va_end(args);

	return list;
}
