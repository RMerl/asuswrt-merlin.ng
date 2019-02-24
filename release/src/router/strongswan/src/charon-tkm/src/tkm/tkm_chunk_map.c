/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <collections/hashtable.h>
#include <threading/rwlock.h>
#include <utils/chunk.h>
#include <utils/debug.h>

#include "tkm_chunk_map.h"

typedef struct private_tkm_chunk_map_t private_tkm_chunk_map_t;

/**
 * Private data of tkm chunk map.
 */
struct private_tkm_chunk_map_t {

	/**
	 * public functions
	 */
	tkm_chunk_map_t public;

	/**
	 * Hashtable to store mappings.
	 */
	hashtable_t *mappings;

	/**
	 * rwlock for table.
	 */
	rwlock_t *lock;

};

/**
 * Entry for hashtables
 */
typedef struct {
	/** Key chunk */
	chunk_t key;
	/** Entry value */
	uint64_t value;
} entry_t;

/**
 * Destroy a hashtable entry
 */
static void entry_destroy(entry_t *this)
{
	chunk_free(&this->key);
	free(this);
}

METHOD(tkm_chunk_map_t, insert, void,
	private_tkm_chunk_map_t * const this, const chunk_t * const data,
	const uint64_t id)
{
	entry_t *entry;
	INIT(entry,
		.key = chunk_clone(*data),
		.value = id
	);

	this->lock->write_lock(this->lock);
	entry = this->mappings->put(this->mappings, (void*)&entry->key, entry);
	this->lock->unlock(this->lock);

	if (entry)
	{
		entry_destroy(entry);
	}
}

METHOD(tkm_chunk_map_t, get_id, uint64_t,
	private_tkm_chunk_map_t * const this, chunk_t *data)
{
	entry_t *entry;
	this->lock->read_lock(this->lock);
	entry = this->mappings->get(this->mappings, data);
	this->lock->unlock(this->lock);

	if (!entry)
	{
		return 0;
	}

	return entry->value;
}

METHOD(tkm_chunk_map_t, remove_, bool,
	private_tkm_chunk_map_t * const this, chunk_t *data)
{
	entry_t *entry;

	this->lock->write_lock(this->lock);
	entry = this->mappings->remove(this->mappings, data);
	this->lock->unlock(this->lock);

	if (entry)
	{
		entry_destroy(entry);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

METHOD(tkm_chunk_map_t, destroy, void,
	private_tkm_chunk_map_t *this)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->mappings->create_enumerator(this->mappings);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		entry_destroy(entry);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	this->mappings->destroy(this->mappings);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * Hashtable hash function.
 */
static u_int hash(chunk_t *key)
{
	return chunk_hash(*key);
}

/*
 * see header file
 */
tkm_chunk_map_t *tkm_chunk_map_create()
{
	private_tkm_chunk_map_t *this;

	INIT(this,
		.public = {
			.insert = _insert,
			.get_id = _get_id,
			.remove = _remove_,
			.destroy = _destroy,
		},
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.mappings = hashtable_create((hashtable_hash_t)hash,
									 (hashtable_equals_t)chunk_equals_ptr, 32),
	);

	return &this->public;
}
