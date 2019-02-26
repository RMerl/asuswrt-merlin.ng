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

#include "sw_collector_rest_api.h"

#include <rest/rest.h>
#include <utils/debug.h>

typedef struct private_sw_collector_rest_api_t private_sw_collector_rest_api_t;

/**
 * Private data of an sw_collector_rest_api_t object.
 */
struct private_sw_collector_rest_api_t {

	/**
	 * Public members of sw_collector_rest_api_state_t
	 */
	sw_collector_rest_api_t public;

	/**
	 * Software collector database
	 */
	sw_collector_db_t *db;

	/**
	 * REST API of central collector database
	 */
	rest_t *rest_api;

};

/**
 * Put all locally retrieved software identifiers into a json object
 */
static json_object* create_rest_request(private_sw_collector_rest_api_t *this,
										sw_collector_db_query_t type)
{
	json_object *jrequest, *jarray, *jstring;
	char *name, *package, *version;
	uint32_t sw_id, i;
	enumerator_t *e;

	jrequest = json_object_new_object();
	jarray = json_object_new_array();
	json_object_object_add(jrequest, "data", jarray);

	e = this->db->create_sw_enumerator(this->db, type, NULL);
	if (!e)
	{
		return NULL;
	}
	while (e->enumerate(e, &sw_id, &name, &package, &version, &i))
	{
		jstring = json_object_new_string(name);
		json_object_array_add(jarray, jstring);
	}
	e->destroy(e);

	return jrequest;
}

typedef struct {
	/** public enumerator interface */
	enumerator_t public;
	/** enumerated json array */
	json_object *jarray;
	/** current index +1, initialized at 0 */
	int idx;
} json_array_enumerator_t;

METHOD(enumerator_t, enumerate, bool,
	json_array_enumerator_t *this, va_list args)
{
	json_object *jvalue;
	char **out;

	VA_ARGS_VGET(args, out);

	if (this->idx >= json_object_array_length(this->jarray))
	{
		return FALSE;
	}

	jvalue = json_object_array_get_idx(this->jarray, this->idx++);
	if (json_object_get_type(jvalue) != json_type_string)
	{
		DBG1(DBG_IMC, "json_string element expected in json_array");
		return FALSE;
	}
	*out = (char*)json_object_get_string(jvalue);

	return TRUE;
}

METHOD(enumerator_t, enumerator_destroy, void,
	json_array_enumerator_t *this)
{
	json_object_put(this->jarray);
	free(this);	
}

METHOD(sw_collector_rest_api_t, create_sw_enumerator, enumerator_t*,
	private_sw_collector_rest_api_t *this, sw_collector_db_query_t type)
{
	json_array_enumerator_t *enumerator;
	json_object *jrequest, *jresponse;
	char cmd[BUF_LEN];
	status_t status;

	jrequest = create_rest_request(this, type);
	if (!jrequest)
	{
		return NULL;
	}
	snprintf(cmd, BUF_LEN, "sessions/0/swid-measurement/");

	status = this->rest_api->post(this->rest_api, cmd, jrequest, &jresponse);
	json_object_put(jrequest);

	switch (status)
	{
		case SUCCESS:
		case NOT_FOUND:
			jresponse = json_object_new_array();
			break;
		case NEED_MORE:
			if (json_object_get_type(jresponse) != json_type_array)
			{
				DBG1(DBG_IMC, "REST response was not a json_array");
				json_object_put(jresponse);
				return NULL;
			}
			break;
		case FAILED:
		default:
			return NULL;
	}

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = _enumerator_destroy,
		},
		.jarray = jresponse,
	);

	return &enumerator->public;
}

METHOD(sw_collector_rest_api_t, destroy, void,
	private_sw_collector_rest_api_t *this)
{
	this->rest_api->destroy(this->rest_api);
	free(this);
}

/**
 * Described in header.
 */
sw_collector_rest_api_t *sw_collector_rest_api_create(sw_collector_db_t *db)
{
	private_sw_collector_rest_api_t *this;
	int timeout;
	char *uri;

	uri = lib->settings->get_str(lib->settings, "%s.rest_api.uri", NULL,
								 lib->ns);
	timeout = lib->settings->get_int(lib->settings, "%s.rest_api.timeout", 120,
								 lib->ns);
	if (!uri)
	{
		DBG1(DBG_IMC, "REST URI to central collector database not set");
		return NULL;
	}

	INIT(this,
		.public = {
			.create_sw_enumerator = _create_sw_enumerator,
			.destroy = _destroy,
		},
		.db = db,
		.rest_api = rest_create(uri, timeout),
	);

	return &this->public;
}
