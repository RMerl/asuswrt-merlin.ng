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

#ifdef USE_JSON

#define _GNU_SOURCE
#include <stdio.h>

#include "rest.h"

typedef struct private_rest_t private_rest_t;

/**
 * Private data of an rest_t object.
 */
struct private_rest_t {

	/**
	 * Public members of rest_t
	 */
	rest_t public;

	/**
	 * URI of REST API
	 */
	char *uri;

	/**
	 * Timeout of REST API connection
	 */
	u_int timeout;

};

METHOD(rest_t, get, status_t,
	private_rest_t *this, char *command, json_object **jresponse)
{
	struct json_tokener *tokener;
	chunk_t response = chunk_empty;
	status_t status;
	char *uri;

	if (asprintf(&uri, "%s%s",this->uri, command) < 0)
	{
		return FAILED;
	}

	status = lib->fetcher->fetch(lib->fetcher, uri, &response,
				FETCH_TIMEOUT, this->timeout,
				FETCH_END);
	free(uri);

	if (status == SUCCESS && jresponse)
	{
		/* Parse HTTP response into a JSON object */
		tokener = json_tokener_new();
		*jresponse = json_tokener_parse_ex(tokener, response.ptr, response.len);
		json_tokener_free(tokener);
	}
	free(response.ptr);

	return status;
}

#define HTTP_STATUS_CODE_NOT_FOUND				404
#define HTTP_STATUS_CODE_PRECONDITION_FAILED	412

METHOD(rest_t, post, status_t,
	private_rest_t *this, char *command, json_object *jrequest,
	json_object **jresponse)
{
	struct json_tokener *tokener;
	chunk_t data, response = chunk_empty;
	status_t status;
	char *uri;
	int code;

	if (asprintf(&uri, "%s%s",this->uri, command) < 0)
	{
		return FAILED;
	}
	data = chunk_from_str((char*)json_object_to_json_string(jrequest));

	status = lib->fetcher->fetch(lib->fetcher, uri, &response,
				FETCH_TIMEOUT, this->timeout,
				FETCH_REQUEST_DATA, data,
				FETCH_REQUEST_TYPE, "application/json; charset=utf-8",
				FETCH_REQUEST_HEADER, "Accept: application/json",
				FETCH_REQUEST_HEADER, "Expect:",
				FETCH_RESPONSE_CODE, &code,
				FETCH_END);
	free(uri);

	if (status != SUCCESS)
	{
		switch (code)
		{
			case HTTP_STATUS_CODE_NOT_FOUND:
				status = NOT_FOUND;
				break;
			case HTTP_STATUS_CODE_PRECONDITION_FAILED:
				if (!response.ptr)
				{
					return FAILED;
				}
				if (jresponse)
				{
					/* Parse HTTP response into a JSON object */
					tokener = json_tokener_new();
					*jresponse = json_tokener_parse_ex(tokener, response.ptr,
																response.len);
					json_tokener_free(tokener);
				}
				status = NEED_MORE;
				break;
			default:
				DBG2(DBG_IMV, "REST http request failed with status code: %d",
							   code);
				status = FAILED;
				break;
		}
	}
	free(response.ptr);

	return status;
}

METHOD(rest_t, destroy, void,
	private_rest_t *this)
{
	free(this->uri);
	free(this);
}

/**
 * Described in header.
 */
rest_t *rest_create(char *uri, u_int timeout)
{
	private_rest_t *this;

	INIT(this,
		.public = {
			.get = _get,
			.post = _post,
			.destroy = _destroy,
		},
		.uri = strdup(uri),
		.timeout = timeout,
	);

	return &this->public;
}

#endif /* USE_JSON */
