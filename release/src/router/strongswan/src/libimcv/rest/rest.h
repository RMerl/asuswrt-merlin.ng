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
 * @defgroup imv_swima imv_swima
 * @ingroup libimcv_plugins
 *
 * @defgroup rest_t rest
 * @{ @ingroup imv_swima
 */

#ifndef REST_H_
#define REST_H_

#ifdef USE_JSON

#include <library.h>
#include <json.h>

typedef struct rest_t rest_t;

/**
 * Public REST interface
 */
struct rest_t {

	/**
	 * Send an HTTP GET request returning a JSON object
	 *
	 * @param jresp		JSON object in HTTP
	 * @return			Status (SUCCESS or FAILED)
	 */
	status_t (*get)(rest_t *this, char *command, json_object **jresp);

	/**
	 * Send an HTTP POST request including a JSON object
	 *
	 * @param jreq		JSON object in HTTP request
	 * @param jresp		JSON object in HTTP response if NEED_MORE
	 * @return			Status (SUCCESS, NEED_MORE or FAILED)
	 */
	status_t (*post)(rest_t *this, char *command, json_object *jreq,
					 json_object **jresp);

	/**
	 * Destroy rest_t object
	 */
	void (*destroy)(rest_t *this);

};

/**
 * Create an rest_t instance
 *
 * @param uri			REST URI (http://username:password@hostname[:port]/api/)
 * @param timeout		Timeout of the REST connection
 */
rest_t* rest_create(char *uri, u_int timeout);

#endif /* USE_JSON */

#endif /** REST_H_ @}*/
