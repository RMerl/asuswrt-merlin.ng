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
