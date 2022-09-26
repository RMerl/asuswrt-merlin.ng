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
