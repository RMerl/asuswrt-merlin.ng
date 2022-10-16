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
 * @defgroup metadata metadata
 * @ingroup libstrongswan
 *
 * @defgroup metadata_t metadata
 * @{ @ingroup metadata
 */

#ifndef METADATA_H_
#define METADATA_H_

#include <utils/utils.h>

typedef struct metadata_t metadata_t;

/**
 * Pre-defined metadata type for int values. Note that while the constructor and
 * equals() work with integer values/types smaller than int (such values get
 * promoted to int when passed via ...), get() does not and expects a pointer to
 * an int variable.
 */
#define METADATA_TYPE_INT "int"

/**
 * Pre-defined metadata type for uint64_t values. Make sure to pass only
 * uint64_t values/variables also to the constructor and equals() (or cast them
 * appropriately there).
 */
#define METADATA_TYPE_UINT64 "uint64"

/**
 * Metadata object to allow storing arbitrary values in an encapsulated
 * object.
 */
struct metadata_t {

	/**
	 * Return the type of the metadata object.
	 *
	 * @return		string type of the metadata object
	 */
	const char *(*get_type)(metadata_t *this);

	/**
	 * Clone this metadata object.
	 *
	 * @return		a cloned instance
	 */
	metadata_t *(*clone)(metadata_t *this);

	/**
	 * Compare this to another value (or values, depending on the type).
	 *
	 * @param ...	value(s) (raw, not metadata_t) to compare this to
	 * @return		TRUE if value is equal to metadata value
	 */
	bool (*equals)(metadata_t *this, ...);

	/**
	 * Retrieve the values via variadic argument(s).
	 *
	 * @param ...	pointer(s) to obtain metadata value(s)
	 */
	void (*get)(metadata_t *this, ...);

	/**
	 * Destroy this metadata object.
	 */
	void (*destroy)(metadata_t *this);
};

/**
 * Constructor type for metadata objects.
 *
 * @param type		type of the metadata object, allows using the same
 *					constructor for different types
 * @param args		type-specific arguments
 * @return			metadata object, NULL on failure
 */
typedef metadata_t *(*metadata_create_t)(const char *type, va_list args);

#endif /** METADATA_H_ @}*/
