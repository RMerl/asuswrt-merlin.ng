/*
 * Copyright (C) 2007-2008 Martin Willi
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
 * @defgroup sqlite_database sqlite_database
 * @{ @ingroup sqlite_p
 */

#ifndef SQLITE_DATABASE_H_
#define SQLITE_DATABASE_H_

#include <database/database.h>

typedef struct sqlite_database_t sqlite_database_t;

/**
 * sqlite databse_t implementation.
 */
struct sqlite_database_t {

	/**
	 * Implements database_t
	 */
	database_t db;
};

/**
 * Create a sqlite_database instance.
 *
 * @param uri			connection URI, sqlite:///path/to/file.db
 */
sqlite_database_t *sqlite_database_create(char *uri);

#endif /** SQLITE_DATABASE_H_ @}*/
