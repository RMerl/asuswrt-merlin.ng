/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup database_factory database_factory
 * @{ @ingroup database
 */

#ifndef DATABASE_FACTORY_H_
#define DATABASE_FACTORY_H_

typedef struct database_factory_t database_factory_t;

#include <database/database.h>

/**
 * Generic database construction function.
 *
 * @param uri			implementation specific connection URI
 */
typedef database_t*(*database_constructor_t)(char *uri);

/**
 * Create instances of database connections using registered constructors.
 */
struct database_factory_t {

	/**
	 * Create a database connection instance.
	 *
	 * @param uri		implementation specific connection URI
	 * @return			database_t instance, NULL if not supported/failed
	 */
	database_t* (*create)(database_factory_t *this, char *uri);

	/**
	 * Register a database constructor.
	 *
	 * @param create	database constructor to register
	 */
	void (*add_database)(database_factory_t *this, database_constructor_t create);

	/**
	 * Unregister a previously registered database constructor.
	 *
	 * @param create	database constructor to unregister
	 */
	void (*remove_database)(database_factory_t *this, database_constructor_t create);

	/**
	 * Destroy a database_factory instance.
	 */
	void (*destroy)(database_factory_t *this);
};

/**
 * Create a database_factory instance.
 */
database_factory_t *database_factory_create();

#endif /** DATABASE_FACTORY_H_ @}*/
