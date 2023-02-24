/*
 * Copyright (C) 2008-2020 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "hashtable.h"
#include "hashtable_profiler.h"

#include <utils/chunk.h>
#include <utils/debug.h>

/** The minimum size of the hash table (MUST be a power of 2) */
#define MIN_SIZE 8
/** The maximum size of the hash table (MUST be a power of 2) */
#define MAX_SIZE (1 << 30)

/** Determine the capacity/maximum load of the table (higher values cause
 * more collisions, lower values increase the memory overhead) */
#define CAPACITY(size) (size / 3 * 2)
/** Factor for the new table size based on the number of items when resizing,
 * with the above load factor this results in doubling the size when growing */
#define RESIZE_FACTOR 3

/**
 * A note about these parameters:
 *
 * The maximum number of items that can be stored in this implementation
 * is MAX_COUNT = CAPACITY(MAX_SIZE).
 * Since we use u_int throughout, MAX_COUNT * RESIZE_FACTOR must not overflow
 * this type.
 */
#if (UINT_MAX / RESIZE_FACTOR < CAPACITY(MAX_SIZE))
	#error Hahstable parameters invalid!
#endif

typedef struct pair_t pair_t;

/**
 * This pair holds a pointer to the key and value it represents.
 */
struct pair_t {

	/**
	 * Key of a hash table item.
	 */
	const void *key;

	/**
	 * Value of a hash table item.
	 */
	void *value;

	/**
	 * Cached hash (used in case of a resize).
	 */
	u_int hash;
};

typedef struct private_hashtable_t private_hashtable_t;

/**
 * Private data of a hashtable_t object.
 *
 */
struct private_hashtable_t {

	/**
	 * Public part of hash table.
	 */
	hashtable_t public;

	/**
	 * The number of items in the hash table.
	 */
	u_int count;

	/**
	 * The current size of the hash table (always a power of 2).
	 */
	u_int size;

	/**
	 * The current mask to calculate the row index (size - 1).
	 */
	u_int mask;

	/**
	 * All items in the order they were inserted (removed items are marked by
	 * setting the key to NULL until resized).
	 */
	pair_t *items;

	/**
	 * Number of available slots in the array above and the table in general,
	 * is set to CAPACITY(size) when the hash table is initialized.
	 */
	u_int capacity;

	/**
	 * Number of used slots in the array above.
	 */
	u_int items_count;

	/**
	 * Hash table with indices into the array above.  The type depends on the
	 * current capacity.
	 */
	void *table;

	/**
	 * The hashing function.
	 */
	hashtable_hash_t hash;

	/**
	 * The equality function.
	 */
	hashtable_equals_t equals;

	/**
	 * Profiling data
	 */
	hashtable_profile_t profile;
};

typedef struct private_enumerator_t private_enumerator_t;

/**
 * Hash table enumerator implementation
 */
struct private_enumerator_t {

	/**
	 * Implements enumerator interface
	 */
	enumerator_t enumerator;

	/**
	 * Associated hash table
	 */
	private_hashtable_t *table;

	/**
	 * Current index
	 */
	u_int index;
};

/*
 * Described in header
 */
u_int hashtable_hash_ptr(const void *key)
{
	return chunk_hash(chunk_from_thing(key));
}

/*
 * Described in header
 */
u_int hashtable_hash_str(const void *key)
{
	return chunk_hash(chunk_from_str((char*)key));
}

/*
 * Described in header
 */
bool hashtable_equals_ptr(const void *key, const void *other_key)
{
	return key == other_key;
}

/*
 * Described in header
 */
bool hashtable_equals_str(const void *key, const void *other_key)
{
	return streq(key, other_key);
}

/**
 * Returns the index stored in the given bucket. If the bucket is empty,
 * 0 is returned.
 */
static inline u_int get_index(private_hashtable_t *this, u_int row)
{
	if (this->capacity <= 0xff)
	{
		return ((uint8_t*)this->table)[row];
	}
	else if (this->capacity <= 0xffff)
	{
		return ((uint16_t*)this->table)[row];
	}
	return ((u_int*)this->table)[row];
}

/**
 * Set the index stored in the given bucket. Set to 0 to clear a bucket.
 */
static inline void set_index(private_hashtable_t *this, u_int row, u_int index)
{
	if (this->capacity <= 0xff)
	{
		((uint8_t*)this->table)[row] = index;
	}
	else if (this->capacity <= 0xffff)
	{
		((uint16_t*)this->table)[row] = index;
	}
	else
	{
		((u_int*)this->table)[row] = index;
	}
}

/**
 * This function returns the next-highest power of two for the given number.
 * The algorithm works by setting all bits on the right-hand side of the most
 * significant 1 to 1 and then increments the whole number so it rolls over
 * to the nearest power of two. Note: returns 0 for n == 0
 *
 * Also used by hashlist_t.
 */
u_int hashtable_get_nearest_powerof2(u_int n)
{
	u_int i;

	--n;
	for (i = 1; i < sizeof(u_int) * 8; i <<= 1)
	{
		n |= n >> i;
	}
	return ++n;
}

/**
 * Init hash table to the given size
 */
static void init_hashtable(private_hashtable_t *this, u_int size)
{
	u_int index_size = sizeof(u_int);

	this->size = max(MIN_SIZE, min(size, MAX_SIZE));
	this->size = hashtable_get_nearest_powerof2(this->size);
	this->mask = this->size - 1;
	profile_size(&this->profile, this->size);

	this->capacity = CAPACITY(this->size);
	this->items = calloc(this->capacity, sizeof(pair_t));
	this->items_count = 0;

	if (this->capacity <= 0xff)
	{
		index_size = sizeof(uint8_t);
	}
	else if (this->capacity <= 0xffff)
	{
		index_size = sizeof(uint16_t);
	}
	this->table = calloc(this->size, index_size);
}

/**
 * Calculate the next bucket using quadratic probing (the sequence is h(k) + 1,
 * h(k) + 3, h(k) + 6, h(k) + 10, ...).
 */
static inline u_int get_next(private_hashtable_t *this, u_int row, u_int *p)
{
	*p += 1;
	return (row + *p) & this->mask;
}

/**
 * Find the pair with the given key, optionally returns the hash and first empty
 * or previously used row if the key is not found.
 */
static inline pair_t *find_key(private_hashtable_t *this, const void *key,
								u_int *out_hash, u_int *out_row)
{
	pair_t *pair;
	u_int hash, row, p = 0, removed = 0, index;
	bool found_removed = FALSE;

	if (!this->count && !out_hash && !out_row)
	{
		return NULL;
	}

	lookup_start();

	hash = this->hash(key);
	row = hash & this->mask;
	index = get_index(this, row);
	while (index)
	{
		lookup_probing();
		pair = &this->items[index-1];

		if (!pair->key)
		{
			if (!found_removed && out_row)
			{
				removed = row;
				found_removed = TRUE;
			}
		}
		else if (pair->hash == hash && this->equals(key, pair->key))
		{
			lookup_success(&this->profile);
			return pair;
		}
		row = get_next(this, row, &p);
		index = get_index(this, row);
	}
	if (out_hash)
	{
		*out_hash = hash;
	}
	if (out_row)
	{
		*out_row = found_removed ? removed : row;
	}
	lookup_failure(&this->profile);
	return NULL;
}

/**
 * Helper to insert a new item into the table and items array,
 * returns its new index into the latter.
 */
static inline u_int insert_item(private_hashtable_t *this, u_int row)
{
	u_int index = this->items_count++;

	/* we use 0 to mark unused buckets, so increase the index */
	set_index(this, row, index + 1);
	return index;
}

/**
 * Resize the hash table to the given size and rehash all the elements,
 * size may be smaller or even the same (e.g. if it's necessary to clear
 * previously used buckets).
 */
static bool rehash(private_hashtable_t *this, u_int size)
{
	pair_t *old_items, *pair;
	u_int old_count, i, p, row, index;

	if (size > MAX_SIZE)
	{
		return FALSE;
	}

	old_items = this->items;
	old_count = this->items_count;
	free(this->table);
	init_hashtable(this, size);

	/* no need to do anything if the table is empty and we are just cleaning
	 * up previously used items */
	if (this->count)
	{
		for (i = 0; i < old_count; i++)
		{
			pair = &old_items[i];

			if (pair->key)
			{
				row = pair->hash & this->mask;
				index = get_index(this, row);
				for (p = 0; index;)
				{
					row = get_next(this, row, &p);
					index = get_index(this, row);
				}
				index = insert_item(this, row);
				this->items[index] = *pair;
			}
		}
	}
	free(old_items);
	return TRUE;
}

METHOD(hashtable_t, put, void*,
	private_hashtable_t *this, const void *key, void *value)
{
	void *old_value = NULL;
	pair_t *pair;
	u_int index, hash = 0, row = 0;

	if (this->items_count >= this->capacity &&
		!rehash(this, this->count * RESIZE_FACTOR))
	{
		DBG1(DBG_LIB, "!!! FAILED TO RESIZE HASHTABLE TO %u !!!",
			 this->count * RESIZE_FACTOR);
		return NULL;
	}
	pair = find_key(this, key, &hash, &row);
	if (pair)
	{
		old_value = pair->value;
		pair->value = value;
		pair->key = key;
		return old_value;
	}
	index = insert_item(this, row);
	this->items[index] = (pair_t){
		.hash = hash,
		.key = key,
		.value = value,
	};
	this->count++;
	profile_count(&this->profile, this->count);
	return NULL;
}

METHOD(hashtable_t, get, void*,
	private_hashtable_t *this, const void *key)
{
	pair_t *pair = find_key(this, key, NULL, NULL);
	return pair ? pair->value : NULL;
}

/**
 * Remove the given item from the table, returns the currently stored value.
 */
static void *remove_internal(private_hashtable_t *this, pair_t *pair)
{
	void *value = NULL;

	if (pair)
	{	/* this does not decrease the item count as we keep the previously
		 * used items until the table is rehashed/resized */
		value = pair->value;
		pair->key = NULL;
		this->count--;
	}
	return value;
}

METHOD(hashtable_t, remove_, void*,
	private_hashtable_t *this, const void *key)
{
	pair_t *pair = find_key(this, key, NULL, NULL);
	return remove_internal(this, pair);
}

METHOD(hashtable_t, remove_at, void,
	private_hashtable_t *this, private_enumerator_t *enumerator)
{
	if (enumerator->table == this && enumerator->index)
	{	/* the index is already advanced by one */
		u_int index = enumerator->index - 1;
		remove_internal(this, &this->items[index]);
	}
}

METHOD(hashtable_t, get_count, u_int,
	private_hashtable_t *this)
{
	return this->count;
}

METHOD(enumerator_t, enumerate, bool,
	private_enumerator_t *this, va_list args)
{
	const void **key;
	void **value;
	pair_t *pair;

	VA_ARGS_VGET(args, key, value);

	while (this->index < this->table->items_count)
	{
		pair = &this->table->items[this->index++];
		if (pair->key)
		{
			if (key)
			{
				*key = pair->key;
			}
			if (value)
			{
				*value = pair->value;
			}
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(hashtable_t, create_enumerator, enumerator_t*,
	private_hashtable_t *this)
{
	private_enumerator_t *enumerator;

	INIT(enumerator,
		.enumerator = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = (void*)free,
		},
		.table = this,
	);
	return &enumerator->enumerator;
}

static void destroy_internal(private_hashtable_t *this,
							 void (*fn)(void*,const void*))
{
	pair_t *pair;
	u_int i;

	profiler_cleanup(&this->profile, this->count, this->size);

	if (fn)
	{
		for (i = 0; i < this->items_count; i++)
		{
			pair = &this->items[i];
			if (pair->key)
			{
				fn(pair->value, pair->key);
			}
		}
	}
	free(this->items);
	free(this->table);
	free(this);
}

METHOD(hashtable_t, destroy, void,
	private_hashtable_t *this)
{
	destroy_internal(this, NULL);
}

METHOD(hashtable_t, destroy_function, void,
	private_hashtable_t *this, void (*fn)(void*,const void*))
{
	destroy_internal(this, fn);
}

/*
 * Described in header.
 */
hashtable_t *hashtable_create(hashtable_hash_t hash, hashtable_equals_t equals,
							  u_int size)
{
	private_hashtable_t *this;

	INIT(this,
		.public = {
			.put = _put,
			.get = _get,
			.remove = _remove_,
			.remove_at = (void*)_remove_at,
			.get_count = _get_count,
			.create_enumerator = _create_enumerator,
			.destroy = _destroy,
			.destroy_function = _destroy_function,
		},
		.hash = hash,
		.equals = equals,
	);

	init_hashtable(this, size);

	profiler_init(&this->profile, 2);

	return &this->public;
}
