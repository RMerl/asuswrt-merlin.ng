/*
 * Copyright (C) 2011-2012 Reto Guadagnini
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

/**
* @defgroup resolver_manager resolver_manager
* @{ @ingroup resolver
*/

#ifndef RESOLVER_MANAGER_H_
#define RESOLVER_MANAGER_H_

typedef struct resolver_manager_t resolver_manager_t;

#include <resolver/resolver.h>

/**
 * The resolver_manager manages the resolver implementations and
 * creates instances of them.
 *
 * A resolver plugin is registered by providing its constructor function
 * to the manager. The manager creates instances of the resolver plugin
 * using the registered constructor function.
 */
struct resolver_manager_t {

	/**
	 * Register a resolver implementation.
	 *
	 * @param constructor	resolver constructor function
	 */
	void (*add_resolver)(resolver_manager_t *this,
						 resolver_constructor_t constructor);

	/**
	 * Unregister a previously registered resolver implementation.
	 *
	 * @param constructor	resolver constructor function to unregister
	 */
	void (*remove_resolver)(resolver_manager_t *this,
							resolver_constructor_t constructor);

	/**
	 * Get a new resolver instance.
	 *
	 * @return 				resolver instance.
	 */
	resolver_t* (*create)(resolver_manager_t *this);

	/**
	 * Destroy a resolver_manager instance.
	 */
	void (*destroy)(resolver_manager_t *this);
};

/**
 * Create a resolver_manager instance.
 */
resolver_manager_t *resolver_manager_create();

#endif /** RESOLVER_MANAGER_H_ @}*/
