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

#include "soup_fetcher.h"

#include <libsoup/soup.h>

#include <library.h>
#include <utils/debug.h>

#define DEFAULT_TIMEOUT 10

typedef struct private_soup_fetcher_t private_soup_fetcher_t;

/**
 * private data of a soup_fetcher_t object.
 */
struct private_soup_fetcher_t {

	/**
	 * Public data
	 */
	soup_fetcher_t public;

	/**
	 * HTTP request method
	 */
	const char *method;

	/**
	 * Request content type
	 */
	char *type;

	/**
	 * Request data
	 */
	chunk_t data;

	/**
	 * Request timeout
	 */
	u_int timeout;

	/**
	 * HTTP request version
	 */
	SoupHTTPVersion version;

	/**
	 * Fetcher callback function
	 */
	fetcher_callback_t cb;

	/**
	 * Response status
	 */
	u_int *result;
};

/**
 * Data to pass to soup callback
 */
typedef struct {
	fetcher_callback_t cb;
	void *user;
	SoupSession *session;
} cb_data_t;

/**
 * Soup callback invoking our callback
 */
static void soup_cb(SoupMessage *message, SoupBuffer *chunk, cb_data_t *data)
{
	if (!data->cb(data->user, chunk_create((u_char*)chunk->data, chunk->length)))
	{
		soup_session_cancel_message(data->session, message,
									SOUP_STATUS_CANCELLED);
	}
}

METHOD(fetcher_t, fetch, status_t,
	private_soup_fetcher_t *this, char *uri, void *userdata)
{
	SoupMessage *message;
	status_t status = FAILED;
	cb_data_t data = {
		.cb = this->cb,
		.user = userdata,
	};

	message = soup_message_new(this->method, uri);
	if (!message)
	{
		return NOT_SUPPORTED;
	}
	if (this->cb == fetcher_default_callback)
	{
		*(chunk_t*)userdata = chunk_empty;
	}
	if (this->type)
	{
		soup_message_set_request(message, this->type, SOUP_MEMORY_STATIC,
								 this->data.ptr, this->data.len);
	}
	soup_message_set_http_version(message, this->version);
	soup_message_body_set_accumulate(message->response_body, FALSE);
	g_signal_connect(message, "got-chunk", G_CALLBACK(soup_cb), &data);
	data.session = soup_session_sync_new();
	g_object_set(G_OBJECT(data.session),
				 SOUP_SESSION_TIMEOUT, (guint)this->timeout, NULL);

	DBG2(DBG_LIB, "sending http request to '%s'...", uri);
	soup_session_send_message(data.session, message);
	if (this->result)
	{
		*this->result = message->status_code;
	}
	if (SOUP_STATUS_IS_SUCCESSFUL(message->status_code))
	{
		status = SUCCESS;
	}
	else if (!this->result)
	{	/* only log an error if the code is not returned */
		DBG1(DBG_LIB, "HTTP request failed: %s", message->reason_phrase);
	}
	g_object_unref(G_OBJECT(message));
	g_object_unref(G_OBJECT(data.session));
	return status;
}

METHOD(fetcher_t, set_option, bool,
	private_soup_fetcher_t *this, fetcher_option_t option, ...)
{
	bool supported = TRUE;
	va_list args;

	va_start(args, option);
	switch (option)
	{
		case FETCH_REQUEST_DATA:
			this->method = SOUP_METHOD_POST;
			this->data = va_arg(args, chunk_t);
			break;
		case FETCH_REQUEST_TYPE:
			this->type = va_arg(args, char*);
			break;
		case FETCH_HTTP_VERSION_1_0:
			this->version = SOUP_HTTP_1_0;
			break;
		case FETCH_TIMEOUT:
			this->timeout = va_arg(args, u_int);
			break;
		case FETCH_CALLBACK:
			this->cb = va_arg(args, fetcher_callback_t);
			break;
		case FETCH_RESPONSE_CODE:
			this->result = va_arg(args, u_int*);
			break;
		default:
			supported = FALSE;
			break;
	}
	va_end(args);
	return supported;
}

METHOD(fetcher_t, destroy, void,
	private_soup_fetcher_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
soup_fetcher_t *soup_fetcher_create()
{
	private_soup_fetcher_t *this;

	INIT(this,
		.public = {
			.interface = {
				.fetch = _fetch,
				.set_option = _set_option,
				.destroy = _destroy,
			},
		},
		.method = SOUP_METHOD_GET,
		.version = SOUP_HTTP_1_1,
		.timeout = DEFAULT_TIMEOUT,
		.cb = fetcher_default_callback,
	);

	return &this->public;
}
