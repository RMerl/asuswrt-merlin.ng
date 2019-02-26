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
 * @defgroup medsrv_config_i medsrv_config
 * @{ @ingroup medsrv_p
 */

#ifndef MEDSRV_CONFIG_H_
#define MEDSRV_CONFIG_H_

#include <config/backend.h>
#include <database/database.h>

typedef struct medsrv_config_t medsrv_config_t;

/**
 * Mediation server configuration backend.
 */
struct medsrv_config_t {

	/**
	 * Implements backend_t interface
	 */
	backend_t backend;

	/**
	 * Destroy the backend.
	 */
	void (*destroy)(medsrv_config_t *this);
};

/**
 * Create a medsrv_config backend instance.
 *
 * @param db		underlying database
 * @return			backend instance
 */
medsrv_config_t *medsrv_config_create(database_t *db);

#endif /** MEDSRV_CONFIG_H_ @}*/
