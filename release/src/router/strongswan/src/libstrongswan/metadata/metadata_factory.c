/*
 * Copyright (C) 2021 Tobias Brunner, codelabs GmbH
 * Copyright (C) 2021 Thomas Egerer, secunet AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
