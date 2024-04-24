/*
 * Copyright (C) 2013-2022 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include "est_tls.h"

#include <utils/debug.h>
#include <utils/lexparser.h>
#include <tls_socket.h>

static const char *operations[] = {
	"cacerts",
	"simpleenroll",
	"simplereenroll",
	"fullcmc",
	"serverkeygen",
	"csrattrs"
};

static const char *request_types[] = {
	"",
	"application/pkcs10",
	"application/pkcs10",
	"application/pkcs7-mime",
	"application/pkcs10",
	""
};

typedef struct private_est_tls_t private_est_tls_t;

/**
 * Private data of an est_tls_t object.
 */
struct private_est_tls_t {

	/**
	 * Public est_tls_t interface.
	 */
	est_tls_t public;

	/**
	 * EST Server (IP address and port)
	 */
	host_t *host;

	/**
	 * File descriptor for secure TCP socket
	 */
	int fd;

	/**
	 * TLS socket
	 */
	tls_socket_t *tls;

	/**
	 * Host string of the form <hostname:port> used for http requests
	 */
	char *http_host;

	/**
	 * Path string used for http requests
	 */
	char *http_path;

	/**
	 * Label string used for http requests
	 */
	char *http_label;

	/**
	 * Optional base64-encoded <username:password> for http basic authentication
	 */
	chunk_t user_pass;
};

static chunk_t build_http_request(private_est_tls_t *this, est_op_t op, chunk_t in)
{
	char  *http_header, http_auth[256];
	chunk_t request = chunk_empty, data;
	int len;

	/* Use Basic Authentication? */
	if (this->user_pass.len > 0)
	{
		snprintf(http_auth, sizeof(http_auth), "Authorization: Basic %.*s\r\n",
				 (int)this->user_pass.len, this->user_pass.ptr);
	}
	else
	{
		*http_auth = '\0';
	}

	if (strlen(request_types[op]) > 0)  /* create HTTP POST request */
	{
		data = chunk_to_base64(in, NULL);

		len = asprintf(&http_header,
				"POST %s/.well-known/est%s%s HTTP/1.1\r\n"
				"Host: %s\r\n"
				"%s"
				"Content-Type: %s\r\n"
				"Content-Length: %d\r\n"
				"\r\n",
				this->http_path, this->http_label, operations[op], this->http_host, http_auth,
				request_types[op], (int)data.len);
		if (len > 0)
		{
			request = chunk_cat("mm", chunk_create(http_header, len), data);
		}
		else
		{
			chunk_free(&data);
		}
	}
	else                                /* create HTTP GET request */
	{
		len = asprintf(&http_header,
				"GET %s/.well-known/est%s%s HTTP/1.1\r\n"
				"Host: %s\r\n"
				"%s"
				"\r\n",
				this->http_path, this->http_label, operations[op], this->http_host, http_auth);
		if (len > 0)
		{
			request = chunk_create(http_header, len);
		}
	}
	return request;
}

static bool parse_http_header(chunk_t *in,  u_int *http_code, u_int *content_len,
							  u_int *retry_after)
{
	chunk_t line, version, parameter;
	u_int len;

	/*initialize output parameters */
	*http_code = 0;
	*content_len = 0;

	if (retry_after)
	{
		*retry_after = 0;
	}

	/* Process HTTP protocol version and HTTP status code */
	if (!fetchline(in, &line) || !extract_token(&version, ' ', &line) ||
		!match("HTTP/1.1", &version) || sscanf(line.ptr, "%d", http_code) != 1)
	{
		DBG1(DBG_APP, "malformed http response header");
		return FALSE;
	}

	/* Process HTTP header line by line until the HTTP body is reached */
	while (fetchline(in, &line))
	{
		if (line.len == 0)
		{
			break;
		}
		if (extract_token(&parameter, ':', &line) && eat_whitespace(&line))
		{
			if (matchcase("Content-Length", &parameter))
			{
				if (sscanf(line.ptr, "%u", &len) == 1)
			{
					*content_len = len;
				}
			}
			else if (matchcase("Retry-After", &parameter))
			{
				if (sscanf(line.ptr, "%u", &len) == 1 && retry_after)
				{
					*retry_after = len;
				}
			}
		}
	}

	return (*http_code < 300);
}

METHOD(est_tls_t, request, bool,
	private_est_tls_t *this, est_op_t op, chunk_t in, chunk_t *out,
	u_int *http_code, u_int *retry_after)
{
	chunk_t http = chunk_empty, data = chunk_empty, response;
	u_int content_len;
	char buf[1024];
	int i, len;

	/* initialize output variables */
	*out = chunk_empty;
	*http_code = 0;

	if (retry_after)
	{
		*retry_after = 0;
	}

	http = build_http_request(this, op, in);

	if (http.len == 0)
	{
		return FALSE;
	}
	DBG2(DBG_APP, "http request: %B", &http);

	/* send https request */
	if (this->tls->write(this->tls, http.ptr, http.len) != http.len)
	{
		DBG1(DBG_APP, "TLS socket write failed");
		chunk_free(&http);
		return FALSE;
	}
	chunk_free(&http);

	/* receive first part of https response */
	len = this->tls->read(this->tls, buf, sizeof(buf), TRUE);
	if (len <= 0)
	{
		DBG1(DBG_APP, "TLS socket first read failed");
		return FALSE;
	}
	response = chunk_create(buf, len);
	DBG2(DBG_APP, "http response: %B", &response);

	if (!parse_http_header(&response, http_code, &content_len, retry_after))
	{
		return FALSE;
	}
	if (*http_code == EST_HTTP_CODE_OK)
	{
		if (content_len == 0)
		{
			DBG1(DBG_APP, "no content-length defined in http header");
			return FALSE;
		}
		if (response.len > content_len)
		{
			DBG1(DBG_APP, "http body is larger than content-length");
			return FALSE;
		}

		data = chunk_alloc(content_len);
		memcpy(data.ptr, response.ptr, response.len);

		if (data.len > response.len)
		{
			/* read remaining part of https response */
			len = this->tls->read(this->tls, data.ptr + response.len,
								  data.len - response.len, TRUE);
			if (len < data.len - response.len)
			{
				DBG1(DBG_APP, "TLS socket second read failed");
				chunk_free(&data);
				return FALSE;
			}
		}

		for (i = 0, len = 0; i < data.len; i++)
		{
			if (!isspace(data.ptr[i]))
			{
				data.ptr[len++] = data.ptr[i];
			}
		}
		data.len = len;

		*out = chunk_from_base64(data, NULL);
		chunk_free(&data);
	}
	return TRUE;
}

METHOD(est_tls_t, destroy, void,
	private_est_tls_t *this)
{
	DESTROY_IF(this->tls);
	DESTROY_IF(this->host);
	if (this->fd != -1)
	{
		close(this->fd);
	}
	chunk_clear(&this->user_pass);
	free(this->http_host);
	free(this->http_label);
	free(this->http_path);
	free(this);
}

static bool est_tls_init(private_est_tls_t *this, char *uri, char *label,
						 certificate_t *client_cert)
{
	identification_t *client_id = NULL, *server_id = NULL;
	char *host_str, *port_str, *path_str;
	int port = 443;
	bool success = FALSE;

	/* check for "https://" prefix and remove it */
	if (strlen(uri) < 8 || !strncaseeq(uri, "https://", 8))
	{
		DBG1(DBG_APP, "'%s' is not an https URI", uri);
		return FALSE;
	}
	uri += 8;

	/* any trailing path or command? */
	path_str = strchr(uri, '/');

	this->http_path =
		strdup( (path_str == NULL || path_str[1] == '\0') ? "" : path_str );

	if (path_str)
	{
		/* NUL-terminate host_str */
		*path_str = '\0';
	}

	/* ensure sure label starts and ends with '/' character */
	if (!label || !label[0] ||
		asprintf(&this->http_label, "%s%s%s",
				 label[0] == '/' ? "" : "/",
				 label,
				 label[strlen(label) - 1] == '/' ? "" : "/") < 0)
	{
		this->http_label = strdup("/");
	}

	/* duplicate <hostname:port> string since we are going to manipulate it */
	host_str = strdup(uri);

	/* another duplicate for http requests */
	this->http_host = strdup(host_str);

	/* extract hostname and port from URI */
	port_str = strchr(host_str, ':');

	if (port_str)
	{
		/* NUL-terminate hostname */
		*port_str++ = '\0';

		/* extract port */
		if (sscanf(port_str, "%d", &port) != 1)
		{
			DBG1(DBG_APP, "parsing server port %s failed", port_str);
			goto end;
		}
	}

	/* open TCP socket and connect to EST server */
	this->host = host_create_from_dns(host_str, 0, port);
	if (!this->host)
	{
		DBG1(DBG_APP, "resolving hostname %s failed", host_str);
		goto end;
	}

	this->fd = socket(this->host->get_family(this->host), SOCK_STREAM, 0);
	if (this->fd == -1)
	{
		DBG1(DBG_APP, "opening socket failed: %s", strerror(errno));
		goto end;
	}

	if (connect(this->fd, this->host->get_sockaddr(this->host),
						 *this->host->get_sockaddr_len(this->host)) == -1)
	{
		DBG1(DBG_APP, "connecting to %#H failed: %s",
					   this->host, strerror(errno));
		goto end;
	}

	if (client_cert)
	{
		client_id = client_cert->get_subject(client_cert);
	}
	server_id = identification_create_from_string(host_str);

	/* open TLS socket */
	this->tls = tls_socket_create(FALSE, server_id, client_id, this->fd,
								  NULL, TLS_UNSPEC, TLS_UNSPEC, 0);
	server_id->destroy(server_id);
	if (!this->tls)
	{
		DBG1(DBG_APP, "creating TLS socket failed");
		goto end;
	}
	success = TRUE;

end:
	free(host_str);

	return success;
}

/**
 * See header
 */
est_tls_t *est_tls_create(char *uri, char *label, certificate_t *client_cert, char *user_pass)
{
	private_est_tls_t *this;

	INIT(this,
		.public = {
			.request = _request,
			.destroy = _destroy,
		},
	);

	if (user_pass)
	{
		this->user_pass = chunk_to_base64(chunk_from_str(user_pass), NULL);
	}

	if (!est_tls_init(this, uri, label, client_cert))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
