/*
 * Copyright (C) 2013 Andreas Steffen
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

#define _GNU_SOURCE /* for asprintf() */

#include "tnc_ifmap_http.h"

#include <utils/debug.h>
#include <utils/lexparser.h>

#include <stdio.h>

typedef struct private_tnc_ifmap_http_t private_tnc_ifmap_http_t;

/**
 * Private data of an tnc_ifmap_http_t object.
 */
struct private_tnc_ifmap_http_t {

	/**
	 * Public tnc_ifmap_http_t interface.
	 */
	tnc_ifmap_http_t public;

	/**
	 * HTTPS Server URI with https:// prefix removed
	 */
	char *uri;

	/**
	 * Optional base64-encoded username:password for HTTP Basic Authentication
	 */
	chunk_t user_pass;

	/**
	 * HTTP chunked mode
	 */
	bool chunked;

};

METHOD(tnc_ifmap_http_t, build, status_t,
	private_tnc_ifmap_http_t *this, chunk_t *in, chunk_t *out)
{
	char *host, *path, *request, auth[128];
	int len;

	/* Duplicate host[/path] string since we are going to manipulate it */
	len = strlen(this->uri) + 2;
	host = malloc(len);
	memset(host, '\0', len);
	strcpy(host, this->uri);

	/* Extract appended path or set to root */
	path = strchr(host, '/');
	if (!path)
	{
		path = host + len - 2;
		*path = '/';
	}

	/* Use Basic Authentication? */
	if (this->user_pass.len)
	{
		snprintf(auth, sizeof(auth), "Authorization: Basic %.*s\r\n",
				(int)this->user_pass.len, this->user_pass.ptr);
	}
	else
	{
		*auth = '\0';
	}

	/* Write HTTP POST request, TODO break up into chunks */
	len = asprintf(&request,
			"POST %s HTTP/1.1\r\n"
			"Host: %.*s\r\n"
			"%s"
			"Content-Type: application/soap+xml;charset=utf-8\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			"%.*s", path, (int)(path-host), host, auth, (int)in->len,
			(int)in->len, in->ptr);
	free(host);

	if (len == -1)
	{
		return FAILED;
	}
	*out = chunk_create(request, len);
	DBG3(DBG_TLS, "sending HTTP POST request %B", out);

	return SUCCESS;
}

static bool process_header(chunk_t *in, bool *chunked, u_int *content_len)
{
	chunk_t line, version, parameter;
	int code;
	u_int len;

	/* Process HTTP protocol version */
	if (!fetchline(in, &line) || !extract_token(&version, ' ', &line) ||
		!match("HTTP/1.1", &version) || sscanf(line.ptr, "%d", &code) != 1)
	{
		DBG1(DBG_TNC, "malformed http response header");
		return FALSE;
	}
	if (code != 200)
	{
		DBG1(DBG_TNC, "http response returns error code %d", code);
		return FALSE;
	}

	*content_len = 0;
	*chunked = FALSE;

	/* Process HTTP header line by line until the HTTP body is reached */
	while (fetchline(in, &line))
	{
		if (line.len == 0)
		{
			break;
		}
		if (extract_token(&parameter, ':', &line) && eat_whitespace(&line))
		{
			if (match("Content-Length", &parameter))
			{
				if (sscanf(line.ptr, "%u", &len) == 1)
	 			{
					*content_len = len;
				}
			}
			else if (match("Transfer-Encoding", &parameter) &&
					 match("chunked", &line))
			{
				*chunked = TRUE;
			}
		}
	}

	return TRUE;
}

METHOD(tnc_ifmap_http_t, process, status_t,
	private_tnc_ifmap_http_t *this, chunk_t *in, chunk_t *out)
{
	u_int len = 0;
	chunk_t line, out_chunk;

	DBG3(DBG_TLS, "receiving HTTP response %B", in);

	if (!this->chunked)
	{
		if (!process_header(in, &this->chunked, &len))
		{
			return FAILED;
		}
	}

	while (in->len)
	{
		if (this->chunked)
		{
			if (!fetchline(in, &line) || sscanf(line.ptr, "%x", &len) != 1)
			{
				return FAILED;
			}
			DBG3(DBG_TLS, "received HTTP response is chunked (%u bytes)", len);

			/* Received last chunk? */
			if (len == 0)
			{
				return SUCCESS;
			}
		}

		/* Check size of of remaining HTTP body */
		if (len > in->len)
		{
			DBG1(DBG_TNC, "insufficient data in HTTP body");
			return FAILED;
		}

		if (this->chunked)
		{
			out_chunk = *in;
			out_chunk.len = len;
			*out = chunk_cat("mc", *out, out_chunk);
			*in = chunk_skip(*in, len);
			if (!fetchline(in, &line) || line.len > 0)
			{
				return FAILED;
			}
		}
		else
		{
			if (len)
			{
				in->len = len;
			}
			*out = chunk_clone(*in);
			return SUCCESS;
		}
	}
	return NEED_MORE;
}

METHOD(tnc_ifmap_http_t, destroy, void,
	private_tnc_ifmap_http_t *this)
{
	free(this);
}

/**
 * See header
 */
tnc_ifmap_http_t *tnc_ifmap_http_create(char *uri, chunk_t user_pass)
{
	private_tnc_ifmap_http_t *this;

	INIT(this,
		.public = {
			.build = _build,
			.process = _process,
			.destroy = _destroy,
		},
		.uri = uri,
		.user_pass = user_pass,
	);

	return &this->public;
}

