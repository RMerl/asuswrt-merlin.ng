/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2007 Andreas Steffen
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

#include <curl/curl.h>

#include <library.h>
#include <utils/debug.h>

#include "curl_fetcher.h"

#define CONNECT_TIMEOUT 10

typedef struct private_curl_fetcher_t private_curl_fetcher_t;

/**
 * private data of a curl_fetcher_t object.
 */
struct private_curl_fetcher_t {
	/**
	 * Public data
	 */
	curl_fetcher_t public;

	/**
	 * CURL handle
	 */
	CURL* curl;

	/**
	 * Optional HTTP headers
	 */
	struct curl_slist *headers;

	/**
	 * Callback function
	 */
	fetcher_callback_t cb;

	/**
	 * Variable that receives the response code
	 */
	u_int *result;

	/**
	 * Timeout for a transfer
	 */
	long timeout;

	/**
	 * Maximum number of redirects to follow
	 */
	long redir;
};

/**
 * Data to pass to curl callback
 */
typedef struct {
	fetcher_callback_t cb;
	void *user;
} cb_data_t;

/**
 * Curl callback function, invokes fetcher_callback_t function
 */
static size_t curl_cb(void *ptr, size_t size, size_t nmemb, cb_data_t *data)
{
	size_t realsize = size * nmemb;

	if (data->cb(data->user, chunk_create(ptr, realsize)))
	{
		return realsize;
	}
	return 0;
}

METHOD(fetcher_t, fetch, status_t,
	private_curl_fetcher_t *this, char *uri, void *userdata)
{
	char error[CURL_ERROR_SIZE], *enc_uri, *p1, *p2;
	CURLcode curl_status;
	status_t status;
	long result = 0;
	cb_data_t data = {
		.cb = this->cb,
		.user = userdata,
	};

	if (this->cb == fetcher_default_callback)
	{
		*(chunk_t*)userdata = chunk_empty;
	}

	/* the URI has to be URL-encoded, we only replace spaces as replacing other
	 * characters (e.g. '/' or ':') would render the URI invalid */
	enc_uri = strreplace(uri, " ", "%20");

	if (curl_easy_setopt(this->curl, CURLOPT_URL, enc_uri) != CURLE_OK)
	{	/* URL type not supported by curl */
		status = NOT_SUPPORTED;
		goto out;
	}
	curl_easy_setopt(this->curl, CURLOPT_ERRORBUFFER, error);
	curl_easy_setopt(this->curl, CURLOPT_FAILONERROR, FALSE);
	curl_easy_setopt(this->curl, CURLOPT_NOSIGNAL, TRUE);
	if (this->timeout)
	{
		curl_easy_setopt(this->curl, CURLOPT_TIMEOUT, this->timeout);
	}
	curl_easy_setopt(this->curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
	curl_easy_setopt(this->curl, CURLOPT_FOLLOWLOCATION, TRUE);
	curl_easy_setopt(this->curl, CURLOPT_MAXREDIRS, this->redir);
	curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, (void*)curl_cb);
	curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &data);
	if (this->headers)
	{
		curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, this->headers);
	}

	/* if the URI contains a username[:password] prefix then mask it */
	p1 = strstr(uri, "://");
	p2 = strchr(uri, '@');
	if (p1 && p2)
	{
		DBG2(DBG_LIB, "  sending request to '%.*sxxxx%s'...", p1+3-uri, uri, p2);
	}
	else
	{
		DBG2(DBG_LIB, "  sending request to '%s'...", uri);
	}
	curl_status = curl_easy_perform(this->curl);
	switch (curl_status)
	{
		case CURLE_UNSUPPORTED_PROTOCOL:
			status = NOT_SUPPORTED;
			break;
		case CURLE_OK:
			curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE,
							  &result);
			if (this->result)
			{
				*this->result = result;
			}
			status = (result < 400) ? SUCCESS : FAILED;
			break;
		default:
			DBG1(DBG_LIB, "libcurl request failed [%d]: %s", curl_status,
				 error);
			status = FAILED;
			break;
	}

out:
	if (enc_uri != uri)
	{
		free(enc_uri);
	}
	return status;
}

METHOD(fetcher_t, set_option, bool,
	private_curl_fetcher_t *this, fetcher_option_t option, ...)
{
	bool supported = TRUE;
	va_list args;

	va_start(args, option);
	switch (option)
	{
		case FETCH_REQUEST_DATA:
		{
			chunk_t data = va_arg(args, chunk_t);

			curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, (char*)data.ptr);
			curl_easy_setopt(this->curl, CURLOPT_POSTFIELDSIZE, data.len);
			break;
		}
		case FETCH_REQUEST_TYPE:
		{
			char header[BUF_LEN];
			char *request_type = va_arg(args, char*);

			snprintf(header, BUF_LEN, "Content-Type: %s", request_type);
			this->headers = curl_slist_append(this->headers, header);
			break;
		}
		case FETCH_REQUEST_HEADER:
		{
			char *header = va_arg(args, char*);

			this->headers = curl_slist_append(this->headers, header);
			break;
		}
		case FETCH_HTTP_VERSION_1_0:
		{
			curl_easy_setopt(this->curl, CURLOPT_HTTP_VERSION,
							 CURL_HTTP_VERSION_1_0);
			break;
		}
		case FETCH_TIMEOUT:
		{
			this->timeout = va_arg(args, u_int);
			break;
		}
		case FETCH_CALLBACK:
		{
			this->cb = va_arg(args, fetcher_callback_t);
			break;
		}
		case FETCH_RESPONSE_CODE:
		{
			this->result = va_arg(args, u_int*);
			break;
		}
		case FETCH_SOURCEIP:
		{
			char buf[64];

			snprintf(buf, sizeof(buf), "%H", va_arg(args, host_t*));
			supported = curl_easy_setopt(this->curl, CURLOPT_INTERFACE,
										 buf) == CURLE_OK;
			break;
		}
		default:
			supported = FALSE;
			break;
	}
	va_end(args);
	return supported;
}

METHOD(fetcher_t, destroy, void,
	private_curl_fetcher_t *this)
{
	curl_slist_free_all(this->headers);
	curl_easy_cleanup(this->curl);
	free(this);
}

/*
 * Described in header.
 */
curl_fetcher_t *curl_fetcher_create()
{
	private_curl_fetcher_t *this;

	INIT(this,
		.public = {
			.interface = {
				.fetch = _fetch,
				.set_option = _set_option,
				.destroy = _destroy,
			},
		},
		.curl = curl_easy_init(),
		.cb = fetcher_default_callback,
		.redir = lib->settings->get_int(lib->settings, "%s.plugins.curl.redir",
										-1, lib->ns),
	);

	if (!this->curl)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}
