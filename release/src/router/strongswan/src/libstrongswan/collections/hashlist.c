/*
 * Copyright (C) 2008-2020 Tobias Brunner
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

#include "hashtable.h"
#include "hashtable_profiler.h"

#include <utils/chunk.h>
#include <utils/debug.h>
#ifdef HASHTABLE_PROFILER
#include <utils/backtrace.h>
#endif

/** The minimum size of the hash table (MUST be a power of 2) */
#define MIN_SIZE 8
/** The maximum size of the hash table (MUST be a power of 2) */
#define MAX_SIZE (1 << 30)

/** Maximum load factor before the hash table is resized */
#define LOAD_FACTOR 0.75f

/** Provided by hashtable_t */
u_int hashtable_get_nearest_powerof2(u_int n);

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

	/**
	 * Next pair in an overflow list.
	 */
	pair_t *next;
};

typedef struct private_hashlist_t private_hashlist_t;

/**
 * Private data of a hashlist_t object.
 */
struct private_hashlist_t {

	/**
	 * Public interface.
	 */
	hashlist_t public;

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
	 * The actual table.
	 */
	pair_t **table;

	/**
	 * The hashing function.
	 */
	hashtable_hash_t hash;

	/**
	 * The equality function.
	 */
	hashtable_equals_t equals;

	/**
	 * Alternative comparison function.
	 */
	hashtable_cmp_t cmp;

	/**
	 * Profiling information
	 */
	hashtable_profile_t profile;
};

typedef struct private_enumerator_t private_enumerator_t;

/**
 * Hash table enumerator implementation
 */
struct private_enumerator_t {

	/**
	 * Implements enumerator interface.
	 */
	enumerator_t enumerator;

	/**
	 * Associated hash table.
	 */
	private_hashlist_t *table;

	/**
	 * Current row index.
	 */
	u_int row;

	/**
	 * Number of remaining items to enumerate.
	 */
	u_int count;

	/**
	 * Current pair.
	 */
	pair_t *current;

	/**
	 * Previous pair (used by remove_at).
	 */
	pair_t *prev;
};

/**
 * Init hash table parameters
 */
static void init_hashtable(private_hashlist_t *this, u_int size)
{
	size = max(MIN_SIZE, min(size, MAX_SIZE));
	this->size = hashtable_get_nearest_powerof2(size);
	this->mask = this->size - 1;
	profile_size(&this->profile, this->size);

	this->table = calloc(this->size, sizeof(pair_t*));
}

/**
 * Insert an item into a bucket.
 */
static inline void insert_pair(private_hashlist_t *this, pair_t *to_insert,
							   pair_t *prev)
{
	u_int row;

	if (prev)
	{
		to_insert->next = prev->next;
		prev->next = to_insert;
	}
	else
	{
		row = to_insert->hash & this->mask;
		to_insert->next = this->table[row];
		this->table[row] = to_insert;
	}
}

/**
 * Double the size of the hash table and rehash all the elements.
 */
static void rehash(private_hashlist_t *this)
{
	pair_t **old_table, *to_move, *pair, *next;
	u_int row, old_size;

	if (this->size >= MAX_SIZE)
	{
		return;
	}

	old_size = this->size;
	old_table = this->table;

	init_hashtable(this, old_size << 1);

	for (row = 0; row < old_size; row++)
	{
		to_move = old_table[row];
		while (to_move)
		{
			pair_t *prev = NULL;

			pair = this->table[to_move->hash & this->mask];
			while (pair)
			{
				if (this->cmp && this->cmp(to_move->key, pair->key) < 0)
				{
					break;
				}
				prev = pair;
				pair = pair->next;
			}
			next = to_move->next;
			insert_pair(this, to_move, prev);
			to_move = next;
		}
	}
	free(old_table);
}

/**
 * Find the pair with the given key, optionally returning the hash and previous
 * (or last) pair in the bucket.
 */
static inline pair_t *find_key(private_hashlist_t *this, const void *key,
							   hashtable_equals_t equals, u_int *out_hash,
							   pair_t **out_prev)
{
	pair_t *pair, *prev = NULL;
	bool use_callback = equals != NULL;
	u_int hash;

	if (!this->count && !out_hash)
	{	/* no need to calculate the hash if not requested */
		return NULL;
	}

	equals = equals ?: this->equals;
	hash = this->hash(key);
	if (out_hash)
	{
		*out_hash = hash;
	}

	lookup_start();

	pair = this->table[hash & this->mask];
	while (pair)
	{
		lookup_probing();
		/* when keys are sorted, we compare all items so we can abort earlier
		 * even if the hash does not match, but only as long as we don't
		 * have a callback */
		if (!use_callback && this->cmp)
		{
			int cmp = this->cmp(key, pair->key);
			if (cmp == 0)
			{
				break;
			}
			else if (cmp < 0)
			{	/* no need to continue as the key we search is smaller */
				pair = NULL;
				break;
			}
		}
		else if (hash == pair->hash && equals(key, pair->key))
		{
			break;
		}
		prev = pair;
		pair = pair->next;
	}
	if (out_prev)
	{
		*out_prev = prev;
	}
	if (pair)
	{
		lookup_success(&this->profile);
	}
	else
	{
		lookup_failure(&this->profile);
	}
	return pair;
}

METHOD(hashtable_t, put, void*,
	private_hashlist_t *this, const void *key, void *value)
{
	void *old_value = NULL;
	pair_t *pair, *prev = NULL;
	u_int hash;

	if (this->count >= this->size * LOAD_FACTOR)
	{
		rehash(this);
	}

	pair = find_key(this, key, NULL, &hash, &prev);
	if (pair)
	{
		old_value = pair->value;
		pair->value = value;
		pair->key = key;
	}
	else
	{
		INIT(pair,
			.key = key,
			.value = value,
			.hash = hash,
		);
		insert_pair(this, pair, prev);
		this->count++;
		profile_count(&this->profile, this->count);
	}
	return old_value;
}


METHOD(hashtable_t, get, void*,
	private_hashlist_t *this, const void *key)
{
	pair_t *pair = find_key(this, key, NULL, NULL, NULL);
	return pair ? pair->value : NULL;
}

METHOD(hashlist_t, get_match, void*,
	private_hashlist_t *this, const void *key, hashtable_equals_t match)
{
	pair_t *pair = find_key(this, key, match, NULL, NULL);
	return pair ? pair->value : NULL;
}

METHOD(hashtable_t, remove_, void*,
	private_hashlist_t *this, const void *key)
{
	void *value = NULL;
	pair_t *pair, *prev = NULL;

	pair = find_key(this, key, NULL, NULL, &prev);
	if (pair)
	{
		if (prev)
		{
			prev->next = pair->next;
		}
		else
		{
			this->table[pair->hash & this->mask] = pair->next;
		}
		value = pair->value;
		free(pair);
		this->count--;
	}
	return value;
}

METHOD(hashtable_t, remove_at, void,
	private_hashlist_t *this, private_enumerator_t *enumerator)
{
	if (enumerator->table == this && enumerator->current)
	{
		pair_t *current = enumerator->current;
		if (enumerator->prev)
		{
			enumerator->prev->next = current->next;
		}
		else
		{
			this->table[enumerator->row] = current->next;
		}
		enumerator->current = enumerator->prev;
		free(current);
		this->count--;
	}
}

METHOD(hashtable_t, get_count, u_int,
	private_hashlist_t *this)
{
	return this->count;
}

METHOD(enumerator_t, enumerate, bool,
	private_enumerator_t *this, va_list args)
{
	const void **key;
	void **value;

	VA_ARGS_VGET(args, key, value);

	while (this->count && this->row < this->table->size)
	{
		this->prev = this->current;
		if (this->current)
		{
			this->current = this->current->next;
		}
		else
		{
			this->current = this->table->table[this->row];
		}
		if (this->current)
		{
			if (key)
			{
				*key = this->current->key;
			}
			if (value)
			{
				*value = this->current->value;
			}
			this->count--;
			return TRUE;
		}
		this->row++;
	}
	return FALSE;
}

METHOD(hashtable_t, create_enumerator, enumerator_t*,
	private_hashlist_t *this)
{
	private_enumerator_t *enumerator;

	INIT(enumerator,
		.enumerator = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = (void*)free,
		},
		.table = this,
		.count = this->count,
	);

	return &enumerator->enumerator;
}

static void destroy_internal(private_hashlist_t *this,
							 void (*fn)(void*,const void*))
{
	pair_t *pair, *next;
	u_int row;

	profiler_cleanup(&this->profile, this->count, this->size);

	for (row = 0; row < this->size; row++)
	{
		pair = this->table[row];
		while (pair)
		{
			if (fn)
			{
				fn(pair->value, pair->key);
			}
			next = pair->next;
			free(pair);
			pair = next;
		}
	}
	free(this->table);
	free(this);
}

METHOD2(hashlist_t, hashtable_t, destroy, void,
	private_hashlist_t *this)
{
	destroy_internal(this, NULL);
}

METHOD(hashtable_t, destroy_function, void,
	private_hashlist_t *this, void (*fn)(void*,const void*))
{
	destroy_internal(this, fn);
}

/**
 * Create a hash list
 */
static private_hashlist_t *hashlist_create_internal(hashtable_hash_t hash,
													u_int size)
{
	private_hashlist_t *this;

	INIT(this,
		.public = {
			.ht = {
				.put = _put,
				.get = _get,
				.remove = _remove_,
				.remove_at = (void*)_remove_at,
				.get_count = _get_count,
				.create_enumerator = _create_enumerator,
				.destroy = _destroy,
				.destroy_function = _destroy_function,
			},
			.get_match = _get_match,
			.destroy = _destroy,
		},
		.hash = hash,
	);

	init_hashtable(this, size);

	profiler_init(&this->profile, 3);

	return this;
}

/*
 * Described in header
 */
hashlist_t *hashlist_create(hashtable_hash_t hash, hashtable_equals_t equals,
							u_int size)
{
	private_hashlist_t *this = hashlist_create_internal(hash, size);

	this->equals = equals;

	return &this->public;
}

/*
 * Described in header
 */
hashlist_t *hashlist_create_sorted(hashtable_hash_t hash,
								   hashtable_cmp_t cmp, u_int size)
{
	private_hashlist_t *this = hashlist_create_internal(hash, size);

	this->cmp = cmp;

	return &this->public;
}
