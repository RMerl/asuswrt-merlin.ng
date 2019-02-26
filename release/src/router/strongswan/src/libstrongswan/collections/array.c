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

#define _GNU_SOURCE /* for qsort_r() */
#include <stdlib.h>

#include "array.h"

#ifndef HAVE_QSORT_R
#include <threading/thread_value.h>
#endif

/**
 * Data is an allocated block, with potentially unused head and tail:
 *
 *   "esize" each (or sizeof(void*) if esize = 0)
 *  /-\ /-\ /-\ /-\ /-\ /-\
 *
 * +---------------+-------------------------------+---------------+
 * | h | e | a | d | e | l | e | m | e | n | t | s | t | a | i | l |
 * +---------------+-------------------------------+---------------+
 *
 * \--------------/ \-----------------------------/ \-------------/
 *      unused                    used                   unused
 *      "head"                   "count"                 "tail"
 *
 */
struct array_t {
	/** number of elements currently in array (not counting head/tail) */
	uint32_t count;
	/** size of each element, 0 for a pointer based array */
	uint16_t esize;
	/** allocated but unused elements at array front */
	uint8_t head;
	/** allocated but unused elements at array end */
	uint8_t tail;
	/** array elements */
	void *data;
};

#ifndef HAVE_QSORT_R
	/* store data to replicate qsort_r in thread local storage */
	static thread_value_t *sort_data;
#endif

/** maximum number of unused head/tail elements before cleanup */
#define ARRAY_MAX_UNUSED 32

/**
 * Get the actual size of a number of elements
 */
static size_t get_size(array_t *array, uint32_t num)
{
	if (array->esize)
	{
		return array->esize * num;
	}
	return sizeof(void*) * num;
}

/**
 * Increase allocated but unused tail room to at least "room"
 */
static void make_tail_room(array_t *array, uint8_t room)
{
	if (array->tail < room)
	{
		array->data = realloc(array->data,
						get_size(array, array->count + array->head + room));
		array->tail = room;
	}
}

/**
 * Increase allocated but unused head room to at least "room"
 */
static void make_head_room(array_t *array, uint8_t room)
{
	if (array->head < room)
	{
		uint8_t increase = room - array->head;

		array->data = realloc(array->data,
						get_size(array, array->count + array->tail + room));
		memmove(array->data + get_size(array, increase), array->data,
				get_size(array, array->count + array->tail + array->head));
		array->head = room;
	}
}

/**
 * Make space for an item at index using tail room
 */
static void insert_tail(array_t *array, int idx)
{
	make_tail_room(array, 1);
	/* move up all elements after idx by one */
	memmove(array->data + get_size(array, array->head + idx + 1),
			array->data + get_size(array, array->head + idx),
			get_size(array, array->count - idx));

	array->tail--;
	array->count++;
}

/**
 * Make space for an item at index using head room
 */
static void insert_head(array_t *array, int idx)
{
	make_head_room(array, 1);
	/* move down all elements before idx by one */
	memmove(array->data + get_size(array, array->head - 1),
			array->data + get_size(array, array->head),
			get_size(array, idx));

	array->head--;
	array->count++;
}

/**
 * Remove an item, increase tail
 */
static void remove_tail(array_t *array, int idx)
{
	/* move all items after idx one down */
	memmove(array->data + get_size(array, idx + array->head),
			array->data + get_size(array, idx + array->head + 1),
			get_size(array, array->count - 1 - idx));
	array->count--;
	array->tail++;
}

/**
 * Remove an item, increase head
 */
static void remove_head(array_t *array, int idx)
{
	/* move all items before idx one up */
	memmove(array->data + get_size(array, array->head + 1),
			array->data + get_size(array, array->head), get_size(array, idx));
	array->count--;
	array->head++;
}

array_t *array_create(u_int esize, uint8_t reserve)
{
	array_t *array;

	INIT(array,
		.esize = esize,
		.tail = reserve,
	);
	if (array->tail)
	{
		array->data = malloc(get_size(array, array->tail));
	}
	return array;
}

int array_count(array_t *array)
{
	if (array)
	{
		return array->count;
	}
	return 0;
}

void array_compress(array_t *array)
{
	if (array)
	{
		uint32_t tail;

		tail = array->tail;
		if (array->head)
		{
			memmove(array->data, array->data + get_size(array, array->head),
					get_size(array, array->count + array->tail));
			tail += array->head;
			array->head = 0;
		}
		if (tail)
		{
			array->data = realloc(array->data, get_size(array, array->count));
			array->tail = 0;
		}
	}
}

typedef struct {
	/** public enumerator interface */
	enumerator_t public;
	/** enumerated array */
	array_t *array;
	/** current index +1, initialized at 0 */
	int idx;
} array_enumerator_t;

METHOD(enumerator_t, enumerate, bool,
	array_enumerator_t *this, va_list args)
{
	void *pos, **out;

	VA_ARGS_VGET(args, out);

	if (this->idx >= this->array->count)
	{
		return FALSE;
	}

	pos = this->array->data +
		  get_size(this->array, this->idx + this->array->head);
	if (this->array->esize)
	{
		/* for element based arrays we return a pointer to the element */
		*out = pos;
	}
	else
	{
		/* for pointer based arrays we return the pointer directly */
		*out = *(void**)pos;
	}
	this->idx++;
	return TRUE;
}

enumerator_t* array_create_enumerator(array_t *array)
{
	array_enumerator_t *enumerator;

	if (!array)
	{
		return enumerator_create_empty();
	}

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = (void*)free,
		},
		.array = array,
	);
	return &enumerator->public;
}

void array_remove_at(array_t *array, enumerator_t *public)
{
	array_enumerator_t *enumerator = (array_enumerator_t*)public;

	if (enumerator->idx)
	{
		array_remove(array, --enumerator->idx, NULL);
	}
}

void array_insert_create(array_t **array, int idx, void *ptr)
{
	if (*array == NULL)
	{
		*array = array_create(0, 0);
	}
	array_insert(*array, idx, ptr);
}

void array_insert_create_value(array_t **array, u_int esize,
							   int idx, void *val)
{
	if (*array == NULL)
	{
		*array = array_create(esize, 0);
	}
	array_insert(*array, idx, val);
}

void array_insert_enumerator(array_t *array, int idx, enumerator_t *enumerator)
{
	void *ptr;

	while (enumerator->enumerate(enumerator, &ptr))
	{
		array_insert(array, idx, ptr);
	}
	enumerator->destroy(enumerator);
}

void array_insert(array_t *array, int idx, void *data)
{
	if (idx < 0 || idx <= array_count(array))
	{
		void *pos;

		if (idx < 0)
		{
			idx = array_count(array);
		}

		if (array->head && !array->tail)
		{
			insert_head(array, idx);
		}
		else if (array->tail && !array->head)
		{
			insert_tail(array, idx);
		}
		else if (idx > array_count(array) / 2)
		{
			insert_tail(array, idx);
		}
		else
		{
			insert_head(array, idx);
		}

		pos = array->data + get_size(array, array->head + idx);
		if (array->esize)
		{
			memcpy(pos, data, get_size(array, 1));
		}
		else
		{
			/* pointer based array, copy pointer value */
			*(void**)pos = data;
		}
	}
}

bool array_get(array_t *array, int idx, void *data)
{
	if (!array)
	{
		return FALSE;
	}
	if (idx >= 0 && idx >= array_count(array))
	{
		return FALSE;
	}
	if (idx < 0)
	{
		if (array_count(array) == 0)
		{
			return FALSE;
		}
		idx = array_count(array) - 1;
	}
	if (data)
	{
		memcpy(data, array->data + get_size(array, array->head + idx),
			   get_size(array, 1));
	}
	return TRUE;
}

bool array_remove(array_t *array, int idx, void *data)
{
	if (!array_get(array, idx, data))
	{
		return FALSE;
	}
	if (idx < 0)
	{
		idx = array_count(array) - 1;
	}
	if (idx > array_count(array) / 2)
	{
		remove_tail(array, idx);
	}
	else
	{
		remove_head(array, idx);
	}
	if (array->head + array->tail > ARRAY_MAX_UNUSED)
	{
		array_compress(array);
	}
	return TRUE;
}

typedef struct {
	/** the array */
	array_t *array;
	/** comparison function */
	int (*cmp)(const void*,const void*,void*);
	/** optional user arg */
	void *arg;
} sort_data_t;

#ifdef HAVE_QSORT_R_GNU
static int compare_elements(const void *a, const void *b, void *arg)
#elif defined(HAVE_QSORT_R_BSD)
static int compare_elements(void *arg, const void *a, const void *b)
#else /* !HAVE_QSORT_R */
static int compare_elements(const void *a, const void *b)
#endif
{
#ifdef HAVE_QSORT_R
	sort_data_t *data = (sort_data_t*)arg;
#else
	sort_data_t *data = sort_data->get(sort_data);
#endif

	if (data->array->esize)
	{
		return data->cmp(a, b, data->arg);
	}
	return data->cmp(*(void**)a, *(void**)b, data->arg);
}

void array_sort(array_t *array, int (*cmp)(const void*,const void*,void*),
				void *user)
{
	if (array)
	{
		sort_data_t data = {
			.array = array,
			.cmp = cmp,
			.arg = user,
		};
		void *start;

		start = array->data + get_size(array, array->head);

#ifdef HAVE_QSORT_R_GNU
		qsort_r(start, array->count, get_size(array, 1), compare_elements,
				&data);
#elif defined(HAVE_QSORT_R_BSD)
		qsort_r(start, array->count, get_size(array, 1), &data,
				compare_elements);
#else /* !HAVE_QSORT_R */
		sort_data->set(sort_data, &data);
		qsort(start, array->count, get_size(array, 1), compare_elements);
#endif
	}
}

typedef struct {
	/** the array */
	array_t *array;
	/** the key */
	const void *key;
	/** comparison function */
	int (*cmp)(const void*,const void*);
} bsearch_data_t;

static int search_elements(const void *a, const void *b)
{
	bsearch_data_t *data = (bsearch_data_t*)a;

	if (data->array->esize)
	{
		return data->cmp(data->key, b);
	}
	return data->cmp(data->key, *(void**)b);
}

int array_bsearch(array_t *array, const void *key,
				  int (*cmp)(const void*,const void*), void *out)
{
	int idx = -1;

	if (array)
	{
		bsearch_data_t data = {
			.array = array,
			.key = key,
			.cmp = cmp,
		};
		void *start, *item;

		start = array->data + get_size(array, array->head);

		item = bsearch(&data, start, array->count, get_size(array, 1),
					   search_elements);
		if (item)
		{
			if (out)
			{
				memcpy(out, item, get_size(array, 1));
			}
			idx = (item - start) / get_size(array, 1);
		}
	}
	return idx;
}

void array_invoke(array_t *array, array_callback_t cb, void *user)
{
	if (array)
	{
		void *obj;
		int i;

		for (i = array->head; i < array->count + array->head; i++)
		{
			obj = array->data + get_size(array, i);
			if (!array->esize)
			{
				/* dereference if we store store pointers */
				obj = *(void**)obj;
			}
			cb(obj, i - array->head, user);
		}
	}
}

void array_invoke_offset(array_t *array, size_t offset)
{
	if (array)
	{
		void (*method)(void *data);
		void *obj;
		int i;

		for (i = array->head; i < array->count + array->head; i++)
		{
			obj = array->data + get_size(array, i);
			if (!array->esize)
			{
				/* dereference if we store store pointers */
				obj = *(void**)obj;
			}
			method = *(void**)(obj + offset);
			method(obj);
		}
	}
}

void array_destroy(array_t *array)
{
	if (array)
	{
		free(array->data);
		free(array);
	}
}

void array_destroy_function(array_t *array, array_callback_t cb, void *user)
{
	array_invoke(array, cb, user);
	array_destroy(array);
}

void array_destroy_offset(array_t *array, size_t offset)
{
	array_invoke_offset(array, offset);
	array_destroy(array);
}

void arrays_init()
{
#ifndef HAVE_QSORT_R
	sort_data =  thread_value_create(NULL);
#endif
}

void arrays_deinit()
{
#ifndef HAVE_QSORT_R
	sort_data->destroy(sort_data);
#endif
}
