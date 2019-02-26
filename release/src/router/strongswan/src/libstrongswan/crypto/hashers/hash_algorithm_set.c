/*
 * Copyright (C) 2015 Tobias Brunner
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

#include "hash_algorithm_set.h"

#include <collections/array.h>

typedef struct private_hash_algorithm_set_t private_hash_algorithm_set_t;

struct private_hash_algorithm_set_t {

	/**
	 * Public interface
	 */
	hash_algorithm_set_t public;

	/**
	 * Algorithms contained in the set
	 */
	array_t *algorithms;
};

/**
 * Sort hash algorithms
 */
static int hash_sort(const void *a, const void *b, void *user)
{
	const hash_algorithm_t *ha = a, *hb = b;
	return *ha - *hb;
}

/**
 * Find a hash algorithm
 */
static int hash_find(const void *a, const void *b)
{
	return hash_sort(a, b, NULL);
}

METHOD(hash_algorithm_set_t, contains, bool,
	private_hash_algorithm_set_t *this, hash_algorithm_t hash)
{
	return array_bsearch(this->algorithms, &hash, hash_find, NULL) != -1;
}

METHOD(hash_algorithm_set_t, add, void,
	private_hash_algorithm_set_t *this, hash_algorithm_t hash)
{
	if (!contains(this, hash))
	{
		array_insert(this->algorithms, ARRAY_TAIL, &hash);
		array_sort(this->algorithms, hash_sort, NULL);
	}
}

METHOD(hash_algorithm_set_t, count, int,
	private_hash_algorithm_set_t *this)
{
	return array_count(this->algorithms);
}

CALLBACK(hash_filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	hash_algorithm_t *algo, *out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, &algo))
	{
		*out = *algo;
		return TRUE;
	}
	return FALSE;
}

METHOD(hash_algorithm_set_t, create_enumerator, enumerator_t*,
	private_hash_algorithm_set_t *this)
{
	return enumerator_create_filter(array_create_enumerator(this->algorithms),
									hash_filter, NULL, NULL);
}

METHOD(hash_algorithm_set_t, destroy, void,
	private_hash_algorithm_set_t *this)
{
	array_destroy(this->algorithms);
	free(this);
}

/**
 * Described in header
 */
hash_algorithm_set_t *hash_algorithm_set_create()
{
	private_hash_algorithm_set_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.contains = _contains,
			.count = _count,
			.create_enumerator = _create_enumerator,
			.destroy = _destroy,
		},
		.algorithms = array_create(sizeof(hash_algorithm_t), 0),
	);

	return &this->public;
}