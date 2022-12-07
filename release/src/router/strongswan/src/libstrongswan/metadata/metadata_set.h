/*
 * Copyright (C) 2021 Tobias Brunner
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
 * @defgroup metadata_set metadata_set
 * @{ @ingroup metadata
 */

#ifndef METADATA_SET_H_
#define METADATA_SET_H_

#include "metadata.h"

/**
 * Set of metadata objects, indexed via string.
 */
typedef struct metadata_set_t metadata_set_t;

/**
 * Create a metadata_set_t instance.
 */
metadata_set_t *metadata_set_create();

/**
 * Add a metadata object with the given key to the set, replacing any previous
 * object with the same key.
 *
 * @param set			set to add metadata to
 * @param key			key under which to store metadata (cloned)
 * @param data			metadata object (adopted), NULL to remove/destroy
 *						existing object
 */
void metadata_set_put(metadata_set_t *set, const char *key, metadata_t *data);

/**
 * Retrieve the metadata object with the given key.
 *
 * @param set			set to query
 * @param key			key of the metadata object
 * @return				metadata object, NULL if not found
 */
metadata_t *metadata_set_get(metadata_set_t *set, const char *key);

/**
 * Clone a complete metadata set.
 *
 * @param set			set to clone
 * @return				cloned set
 */
metadata_set_t *metadata_set_clone(metadata_set_t *set);

/**
 * Destroy a metadata set, destroying all contained metadata objects.
 *
 * @param set			set to destroy
 */
void metadata_set_destroy(metadata_set_t *set);

#endif /** METADATA_SET_H_ @}*/
