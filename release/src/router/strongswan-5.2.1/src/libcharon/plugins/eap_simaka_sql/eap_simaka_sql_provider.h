/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup eap_simaka_sql_provider eap_simaka_sql_provider
 * @{ @ingroup eap_simaka_sql
 */

#ifndef EAP_SIMAKA_SQL_PROVIDER_H_
#define EAP_SIMAKA_SQL_PROVIDER_H_

#include <database/database.h>
#include <simaka_provider.h>

typedef struct eap_simaka_sql_provider_t eap_simaka_sql_provider_t;

/**
 * SIM provider implementation using a triplet/quintuplet database backend.
 */
struct eap_simaka_sql_provider_t {

	/**
	 * Implements simaka_provider_t interface
	 */
	simaka_provider_t provider;

	/**
	 * Destroy a eap_simaka_sql_provider_t.
	 */
	void (*destroy)(eap_simaka_sql_provider_t *this);
};

/**
 * Create a eap_simaka_sql_provider instance.
 *
 * @param db			triplet/quintuplet database
 * @param remove_used	TRUE to remove used triplets/quintuplets from db
 */
eap_simaka_sql_provider_t *eap_simaka_sql_provider_create(database_t *db,
														  bool remove_used);

#endif /** EAP_SIMAKA_SQL_PROVIDER_H_ @}*/
