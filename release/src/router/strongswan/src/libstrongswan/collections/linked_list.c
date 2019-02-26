/*
 * Copyright (C) 2007-2018 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * HSR Hochschule fuer Technik Rapperswil
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

/*
 * Described in header
 */
bool linked_list_match_str(void *item, va_list args)
{
	char *a = item, *b;

	VA_ARGS_VGET(args, b);
	return streq(a, b);
}

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
	enumerator_t public;

	/**
	 * associated linked list
	 */
	private_linked_list_t *list;

	/**
	 * current item
	 */
	element_t *current;
};

/**
 * Enumerate the current item
 */
static bool do_enumerate(private_enumerator_t *this, va_list args)
{
	void **item;

	VA_ARGS_VGET(args, item);

	if (!this->current)
	{
		return FALSE;
	}
	if (item)
	{
		*item = this->current->value;
	}
	return TRUE;
}

METHOD(enumerator_t, enumerate_next, bool,
	private_enumerator_t *this, va_list args)
{
	if (this->current)
	{
		this->current = this->current->next;
	}
	return do_enumerate(this, args);
}

METHOD(enumerator_t, enumerate_current, bool,
	private_enumerator_t *this, va_list args)
{
	this->public.venumerate = _enumerate_next;
	return do_enumerate(this, args);
}

METHOD(linked_list_t, create_enumerator, enumerator_t*,
	private_linked_list_t *this)
{
	private_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_current,
			.destroy = (void*)free,
		},
		.list = this,
		.current = this->first,
	);

	return &enumerator->public;
}

METHOD(linked_list_t, reset_enumerator, void,
	private_linked_list_t *this, private_enumerator_t *enumerator)
{
	enumerator->current = this->first;
	enumerator->public.venumerate = _enumerate_current;
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
		insert_last(this, item);
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
		enumerator->current = current->next;
		/* the enumerator already points to the next item */
		enumerator->public.venumerate = _enumerate_current;
		remove_element(this, current);
	}
}

METHOD(linked_list_t, find_first, bool,
	private_linked_list_t *this, linked_list_match_t match, void **item, ...)
{
	element_t *current = this->first;
	va_list args;
	bool matched = FALSE;

	if (!match && !item)
	{
		return FALSE;
	}

	while (current)
	{
		if (match)
		{
			va_start(args, item);
			matched = match(current->value, args);
			va_end(args);
		}
		else
		{
			matched = current->value == *item;
		}
		if (matched)
		{
			if (item != NULL)
			{
				*item = current->value;
			}
			return TRUE;
		}
		current = current->next;
	}
	return FALSE;
}

METHOD(linked_list_t, invoke_offset, void,
	private_linked_list_t *this, size_t offset)
{
	element_t *current = this->first;
	void (**method)(void*);

	while (current)
	{
		method = current->value + offset;
		(*method)(current->value);
		current = current->next;
	}
}

METHOD(linked_list_t, invoke_function, void,
	private_linked_list_t *this, linked_list_invoke_t fn, ...)
{
	element_t *current = this->first;
	va_list args;

	while (current)
	{
		va_start(args, fn);
		fn(current->value, args);
		va_end(args);
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

METHOD(linked_list_t, equals_offset, bool,
	private_linked_list_t *this, linked_list_t *other_pub, size_t offset)
{
	private_linked_list_t *other = (private_linked_list_t*)other_pub;
	element_t *cur_t, *cur_o;

	if (this->count != other->count)
	{
		return FALSE;
	}
	cur_t = this->first;
	cur_o = other->first;
	while (cur_t && cur_o)
	{
		bool (**method)(void*,void*) = cur_t->value + offset;
		if (!(*method)(cur_t->value, cur_o->value))
		{
			return FALSE;
		}
		cur_t = cur_t->next;
		cur_o = cur_o->next;
	}
	return TRUE;
}

METHOD(linked_list_t, equals_function, bool,
	private_linked_list_t *this, linked_list_t *other_pub,
	bool (*fn)(void*,void*))
{
	private_linked_list_t *other = (private_linked_list_t*)other_pub;
	element_t *cur_t, *cur_o;

	if (this->count != other->count)
	{
		return FALSE;
	}
	cur_t = this->first;
	cur_o = other->first;
	while (cur_t && cur_o)
	{
		if (!fn(cur_t->value, cur_o->value))
		{
			return FALSE;
		}
		cur_t = cur_t->next;
		cur_o = cur_o->next;
	}
	return TRUE;
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
			.find_first = _find_first,
			.insert_first = _insert_first,
			.insert_last = _insert_last,
			.insert_before = (void*)_insert_before,
			.remove_first = _remove_first,
			.remove_last = _remove_last,
			.remove = _remove_,
			.remove_at = (void*)_remove_at,
			.invoke_offset = _invoke_offset,
			.invoke_function = _invoke_function,
			.clone_offset = _clone_offset,
			.equals_offset = _equals_offset,
			.equals_function = _equals_function,
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
