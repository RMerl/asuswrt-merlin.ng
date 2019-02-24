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
 * @defgroup attr_sql_provider attr_sql_provider
 * @{ @ingroup attr_sql
 */

#ifndef ATTR_SQL_PROVIDER_H_
#define ATTR_SQL_PROVIDER_H_

#include <attributes/attribute_provider.h>
#include <database/database.h>

typedef struct attr_sql_provider_t attr_sql_provider_t;

/**
 * SQL database based IKEv2 cfg attribute provider.
 */
struct attr_sql_provider_t {

	/**
	 * Implements attribute provider interface
	 */
	attribute_provider_t provider;

	/**
	 * Destroy a attr_sql_provider instance.
	 */
	void (*destroy)(attr_sql_provider_t *this);
};

/**
 * Create a attr_sql_provider instance.
 */
attr_sql_provider_t *attr_sql_provider_create(database_t *db);

#endif /** ATTR_SQL_PROVIDER_H_ @}*/
