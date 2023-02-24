/*
 * Copyright (C) 2021 Tobias Brunner
 * Copyright (C) 2021 Thomas Egerer
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

#include "metadata_factory.h"
#include "metadata_int.h"

#include <library.h>
#include <collections/hashtable.h>

typedef struct private_metadata_factory_t private_metadata_factory_t;

/**
 * Private data
 */
struct private_metadata_factory_t {

	/**
	 * Public interface
	 */
	metadata_factory_t public;

	/**
	 * Registered metadata types (entry_t)
	 */
	hashtable_t *types;
};

/**
 * Registered constructor data
 */
typedef struct {
	/** Type name */
	char *type;
	/** Constructor */
	metadata_create_t create;
} entry_t;

/**
 * Destroy an entry
 */
static void destroy_entry(entry_t *this)
{
	if (this)
	{
		free(this->type);
		free(this);
	}
}

METHOD(metadata_factory_t, create, metadata_t*,
	private_metadata_factory_t *this, const char *type, ...)
{
	metadata_t *metadata = NULL;
	entry_t *entry;
	va_list args;

	entry = this->types->get(this->types, type);
	if (entry)
	{
		va_start(args, type);
		metadata = entry->create(type, args);
		va_end(args);
	}
	return metadata;
}

METHOD(metadata_factory_t, register_type, void,
	private_metadata_factory_t *this, const char *type,
	metadata_create_t create)
{
	entry_t *entry;

	INIT(entry,
		.type = strdup(type),
		.create = create,
	);

	destroy_entry(this->types->put(this->types, entry->type, entry));
}

METHOD(metadata_factory_t, destroy, void,
	private_metadata_factory_t *this)
{
	this->types->destroy_function(this->types, (void*)destroy_entry);
	free(this);
}

metadata_factory_t *metadata_factory_create()
{
	private_metadata_factory_t *this;

	INIT(this,
		.public = {
			.create = _create,
			.register_type = _register_type,
			.destroy = _destroy,
		},
		.types = hashtable_create(hashtable_hash_str, hashtable_equals_str, 0),
	);

	/* register pre-defined types */
	register_type(this, METADATA_TYPE_INT, metadata_create_int);
	register_type(this, METADATA_TYPE_UINT64, metadata_create_int);

	return &this->public;
}
