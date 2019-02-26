/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup sql_config_i sql_config
 * @{ @ingroup sql
 */

#ifndef SQL_CONFIG_H_
#define SQL_CONFIG_H_

#include <config/backend.h>
#include <database/database.h>

typedef struct sql_config_t sql_config_t;

/**
 * SQL database configuration backend.
 */
struct sql_config_t {

	/**
	 * Implements backend_t interface
	 */
	backend_t backend;

	/**
	 * Destry the backend.
	 */
	void (*destroy)(sql_config_t *this);
};

/**
 * Create a sql_config backend instance.
 *
 * @param db		underlying database
 * @return			backend instance
 */
sql_config_t *sql_config_create(database_t *db);

#endif /** SQL_CONFIG_H_ @}*/
