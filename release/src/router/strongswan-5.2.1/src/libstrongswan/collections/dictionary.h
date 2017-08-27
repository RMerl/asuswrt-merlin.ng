/*
 * Copyright (C) 2014 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup dictionary dictionary
 * @{ @ingroup collections
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <collections/enumerator.h>

typedef struct dictionary_t dictionary_t;

/**
 * Interface for read-only dictionaries.
 */
struct dictionary_t {

	/**
	 * Create an enumerator over the key/value pairs in the dictionary.
	 *
	 * @return			enumerator over (const void *key, void *value)
	 */
	enumerator_t *(*create_enumerator)(dictionary_t *this);

	/**
	 * Returns the value with the given key, if the dictionary contains such an
	 * entry, otherwise NULL is returned.
	 *
	 * @param key		the key of the requested value
	 * @return			the value, NULL if not found
	 */
	void *(*get)(dictionary_t *this, const void *key);

	/**
	 * Destroys a dictionary object.
	 */
	void (*destroy)(dictionary_t *this);
};

#endif /** DICTIONARY_H_ @}*/
