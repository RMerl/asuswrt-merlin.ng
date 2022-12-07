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

/**
 * @defgroup metadata_factory metadata_factory
 * @{ @ingroup metadata
 */

#ifndef METADATA_FACTORY_H_
#define METADATA_FACTORY_H_

#include "metadata.h"

typedef struct metadata_factory_t metadata_factory_t;

/**
 * Create a factory for dealing with metadata objects.
 */
struct metadata_factory_t {

	/**
	 * Create a metadata object of the given type.
	 *
	 * @param type		type of the desired metadata object
	 * @param ...		data to wrap into the metadata object
	 * @return			metadata handler on success, NULL otherwise
	 */
	metadata_t *(*create)(metadata_factory_t *this, const char *type, ...);

	/**
	 * Register a constructor for a given type.
	 *
	 * @param type		type to register
	 * @param create	metadata constructor
	 */
	void (*register_type)(metadata_factory_t *this, const char *type,
						  metadata_create_t create);

	/**
	 * Destroy a metadata_factory_t instance.
	 */
	void (*destroy)(metadata_factory_t *this);
};

/**
 * Create a metadata_factory_t instance.
 */
metadata_factory_t *metadata_factory_create();

#endif /** METADATA_FACTORY_H_ @}*/
