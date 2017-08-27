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
 * @defgroup sql_attribute sql_attribute
 * @{ @ingroup attr_sql
 */

#ifndef SQL_ATTRIBUTE_H_
#define SQL_ATTRIBUTE_H_

#include <attributes/attribute_provider.h>
#include <database/database.h>

typedef struct sql_attribute_t sql_attribute_t;

/**
 * SQL database based IKEv2 cfg attribute provider.
 */
struct sql_attribute_t {

	/**
	 * Implements attribute provider interface
	 */
	attribute_provider_t provider;

	/**
	 * Destroy a sql_attribute instance.
	 */
	void (*destroy)(sql_attribute_t *this);
};

/**
 * Create a sql_attribute instance.
 */
sql_attribute_t *sql_attribute_create(database_t *db);

#endif /** SQL_ATTRIBUTE_H_ @}*/
