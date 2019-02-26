/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup storage storage
 * @{ @ingroup manager
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <collections/enumerator.h>


typedef struct storage_t storage_t;

/**
 * Persistent database storage.
 */
struct storage_t {

	/**
	 * Try to log in using specified credentials.
	 *
	 * @param username			username
	 * @param password			plaintext password
	 * @return					user ID if login good, 0 otherwise
	 */
	int (*login)(storage_t *this, char *username, char *password);

	/**
	 * Create an enumerator over the gateways.
	 *
	 * enumerate() arguments: int id, char *name, int port, char *address
	 * If port is 0, address is a Unix socket address.
	 *
	 * @param user				user Id
	 * @return					enumerator
	 */
	enumerator_t* (*create_gateway_enumerator)(storage_t *this, int user);

	/**
	 * Destroy a storage instance.
	 */
	void (*destroy)(storage_t *this);
};

/**
 * Create a storage instance.
 *
 * @param uri		database connection URI
 */
storage_t *storage_create(char *uri);

#endif /** STORAGE_H_ @}*/
