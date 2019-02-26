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
 * @defgroup medcli_creds_i medcli_creds
 * @{ @ingroup medcli
 */

#ifndef MEDCLI_CREDS_H_
#define MEDCLI_CREDS_H_

#include <credentials/credential_set.h>
#include <database/database.h>

typedef struct medcli_creds_t medcli_creds_t;

/**
 * Mediation client credentials database.
 */
struct medcli_creds_t {

	/**
	 * Implements credential_set_t interface
	 */
	credential_set_t set;

	/**
	 * Destroy the credentials database.
	 */
	void (*destroy)(medcli_creds_t *this);
};

/**
 * Create the medcli credential set.
 *
 * @param database		underlying database
 * @return				credential set implementation on that database
 */
medcli_creds_t *medcli_creds_create(database_t *database);

#endif /** MEDCLI_CREDS_H_ @}*/
