/*
 * Copyright (C) 2009 Martin Willi
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

#include "cred_encoding.h"

#include <stdint.h>

#include <collections/linked_list.h>
#include <collections/hashtable.h>
#include <threading/rwlock.h>

typedef struct private_cred_encoding_t private_cred_encoding_t;

/**
 * Private data of an cred_encoding_t object.
 */
struct private_cred_encoding_t {

	/**
	 * Public cred_encoding_t interface.
	 */
	cred_encoding_t public;

	/**
	 * cached encodings, a table for each encoding_type_t, containing chunk_t*
	 */
	hashtable_t *cache[CRED_ENCODING_MAX];

	/**
	 * Registered encoding functions, cred_encoder_t
	 */
	linked_list_t *encoders;

	/**
	 * lock to access cache/encoders
	 */
	rwlock_t *lock;
};

/**
 * See header.
 */
bool cred_encoding_args(va_list args, ...)
{
	va_list parts, copy;
	bool failed = FALSE;

	va_start(parts, args);

	while (!failed)
	{
		cred_encoding_part_t current, target;
		chunk_t *out, data;

		/* get the part we are looking for */
		target = va_arg(parts, cred_encoding_part_t);
		if (target == CRED_PART_END)
		{
			break;
		}
		out = va_arg(parts, chunk_t*);

		va_copy(copy, args);
		while (!failed)
		{
			current = va_arg(copy, cred_encoding_part_t);
			if (current == CRED_PART_END)
			{
				failed = TRUE;
				break;
			}
			data = va_arg(copy, chunk_t);
			if (current == target)
			{
				*out = data;
				break;
			}
		}
		va_end(copy);
	}
	va_end(parts);
	return !failed;
}

METHOD(cred_encoding_t, get_cache, bool,
	private_cred_encoding_t *this, cred_encoding_type_t type, void *cache,
	chunk_t *encoding)
{
	chunk_t *chunk;

	if (type >= CRED_ENCODING_MAX || (int)type < 0)
	{
		return FALSE;
	}
	this->lock->read_lock(this->lock);
	chunk = this->cache[type]->get(this->cache[type], cache);
	if (chunk)
	{
		*encoding = *chunk;
	}
	this->lock->unlock(this->lock);
	return !!chunk;
}

/**
 * Implementation of cred_encoding_t.encode
 */
static bool encode(private_cred_encoding_t *this, cred_encoding_type_t type,
				   void *cache, chunk_t *encoding, ...)
{
	enumerator_t *enumerator;
	va_list args, copy;
	cred_encoder_t encode;
	bool success = FALSE;
	chunk_t *chunk;

	if (type >= CRED_ENCODING_MAX || (int)type < 0)
	{
		return FALSE;
	}
	this->lock->read_lock(this->lock);
	if (cache)
	{
		chunk = this->cache[type]->get(this->cache[type], cache);
		if (chunk)
		{
			*encoding = *chunk;
			this->lock->unlock(this->lock);
			return TRUE;
		}
	}
	va_start(args, encoding);
	enumerator = this->encoders->create_enumerator(this->encoders);
	while (enumerator->enumerate(enumerator, &encode))
	{
		va_copy(copy, args);
		success = encode(type, encoding, copy);
		va_end(copy);
		if (success)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	va_end(args);

	if (success && cache)
	{
		chunk = malloc_thing(chunk_t);
		*chunk = *encoding;
		this->lock->write_lock(this->lock);
		chunk = this->cache[type]->put(this->cache[type], cache, chunk);
		this->lock->unlock(this->lock);
		if (chunk)
		{
			free(chunk->ptr);
			free(chunk);
		}
	}
	return success;
}

METHOD(cred_encoding_t, cache, void,
	private_cred_encoding_t *this, cred_encoding_type_t type, void *cache,
	chunk_t encoding)
{
	chunk_t *chunk;

	if (type >= CRED_ENCODING_MAX || (int)type < 0)
	{
		return free(encoding.ptr);
	}
	chunk = malloc_thing(chunk_t);
	*chunk = encoding;
	this->lock->write_lock(this->lock);
	chunk = this->cache[type]->put(this->cache[type], cache, chunk);
	this->lock->unlock(this->lock);
	/* free an encoding already associated to the cache */
	if (chunk)
	{
		free(chunk->ptr);
		free(chunk);
	}
}

METHOD(cred_encoding_t, clear_cache, void,
	private_cred_encoding_t *this, void *cache)
{
	cred_encoding_type_t type;
	chunk_t *chunk;

	this->lock->write_lock(this->lock);
	for (type = 0; type < CRED_ENCODING_MAX; type++)
	{
		chunk = this->cache[type]->remove(this->cache[type], cache);
		if (chunk)
		{
			chunk_free(chunk);
			free(chunk);
		}
	}
	this->lock->unlock(this->lock);
}

METHOD(cred_encoding_t, add_encoder, void,
	private_cred_encoding_t *this, cred_encoder_t encoder)
{
	this->lock->write_lock(this->lock);
	this->encoders->insert_last(this->encoders, encoder);
	this->lock->unlock(this->lock);
}

METHOD(cred_encoding_t, remove_encoder, void,
	private_cred_encoding_t *this, cred_encoder_t encoder)
{
	this->lock->write_lock(this->lock);
	this->encoders->remove(this->encoders, encoder, NULL);
	this->lock->unlock(this->lock);
}

METHOD(cred_encoding_t, destroy, void,
	private_cred_encoding_t *this)
{
	cred_encoding_type_t type;

	for (type = 0; type < CRED_ENCODING_MAX; type++)
	{
		/* We explicitly do not free remaining encodings. All creds should
		 * have gone now, and they are responsible for cleaning out their
		 * cache entries. Not flushing here allows the leak detective to
		 * complain if a credential did not flush cached encodings. */
		this->cache[type]->destroy(this->cache[type]);
	}
	this->encoders->destroy(this->encoders);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
cred_encoding_t *cred_encoding_create()
{
	private_cred_encoding_t *this;
	cred_encoding_type_t type;

	INIT(this,
		.public = {
			.encode = (bool(*)(cred_encoding_t*, cred_encoding_type_t type, void *cache, chunk_t *encoding, ...))encode,
			.get_cache = _get_cache,
			.cache = _cache,
			.clear_cache = _clear_cache,
			.add_encoder = _add_encoder,
			.remove_encoder = _remove_encoder,
			.destroy = _destroy,
		},
		.encoders = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	for (type = 0; type < CRED_ENCODING_MAX; type++)
	{
		this->cache[type] = hashtable_create(hashtable_hash_ptr,
											 hashtable_equals_ptr, 8);
	}

	return &this->public;
}

