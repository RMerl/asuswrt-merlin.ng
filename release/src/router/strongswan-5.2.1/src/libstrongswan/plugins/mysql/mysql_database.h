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
 * @defgroup mysql_database mysql_database
 * @{ @ingroup mysql_p
 */

#ifndef MYSQL_DATABASE_H_
#define MYSQL_DATABASE_H_

#include <library.h>
#include <database/database.h>

typedef struct mysql_database_t mysql_database_t;

/**
 * MySQL databse_t implementation.
 */
struct mysql_database_t {

	/**
	 * Implements database_t
	 */
	database_t db;
};

/**
 * Create a mysql_database instance.
 *
 * @param uri			connection URI, mysql://user:pass@example.com:port/database
 */
mysql_database_t *mysql_database_create(char *uri);

/**
 * MySQL client library initialization function
 *
 * @return 		FALSE if initialization failed
 */
bool mysql_database_init();

/**
 * Mysql client library cleanup function
 */
void mysql_database_deinit();

#endif /** MYSQL_DATABASE_H_ @}*/
