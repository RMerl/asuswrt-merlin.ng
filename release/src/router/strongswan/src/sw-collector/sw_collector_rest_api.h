/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup sw_collector_rest_api_t sw_collector_rest_api
 * @{ @ingroup sw_collector
 */

#ifndef SW_COLLECTOR_REST_API_H_
#define SW_COLLECTOR_REST_API_H_

#include "sw_collector_db.h"

typedef struct sw_collector_rest_api_t sw_collector_rest_api_t;

/**
 * Software collector REST API object
 */
struct sw_collector_rest_api_t {

	/**
	 * List of locally stored software identifiers that are not registered
	 * in a central collector database
	 *
	 * @param type			Query type (ALL, INSTALLED, REMOVED)
	 * @return				Enumerator
	 */
	enumerator_t* (*create_sw_enumerator)(sw_collector_rest_api_t *this,
										  sw_collector_db_query_t type);

	/**
	 * Destroy sw_collector_rest_api_t object
	 */
	void (*destroy)(sw_collector_rest_api_t *this);

};

/**
 * Create an sw_collector_rest_api_t instance
 *
 * @param db				Software collector database to be used
 */
sw_collector_rest_api_t* sw_collector_rest_api_create(sw_collector_db_t *db);

#endif /** SW_COLLECTOR_REST_API_H_ @}*/
