/*
 * Copyright (C) 2021 Tobias Brunner, codelabs GmbH
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
