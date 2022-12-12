/* Interface for HTTP functions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <string.h>

#include "ssl.h"
#include "http.h"
#include "error.h"

int http_construct(http_t *client)
{
	ASSERT(client);

	DO(tcp_construct(&client->tcp));

	memset((char *)client + sizeof(client->tcp), 0, sizeof(*client) - sizeof(client->tcp));
	client->initialized = 0;

	return 0;
}

int http_destruct(http_t *client, int num)
{
	int i = 0, rv = 0;

	while (i < num)
		rv = tcp_destruct(&client[i++].tcp);

	return rv;
}

static int local_set_params(http_t *client)
{
	int timeout = 0;

	http_get_remote_timeout(client, &timeout);
	if (timeout == 0)
		http_set_remote_timeout(client, HTTP_DEFAULT_TIMEOUT);

	return 0;
}

int http_init(http_t *client, char *msg)
{
	int rc = 0;

	do {
		TRY(local_set_params(client));
		TRY(ssl_open(client, msg));
	}
	while (0);

	if (rc) {
		http_exit(client);
		return rc;
	}

	client->initialized = 1;

	return 0;
}

int http_exit(http_t *client)
{
	ASSERT(client);

	if (!client->initialized)
		return 0;

	client->initialized = 0;
	return ssl_close(client);
}

static void http_response_parse(http_trans_t *trans)
{
	char *body;
	char *rsp = trans->rsp_body = trans->rsp;
	int status = trans->status = 0;
	const char sep[] = "\r\n\r\n";

	memset(trans->status_desc, 0, sizeof(trans->status_desc));

	if (rsp != NULL && (body = strstr(rsp, sep)) != NULL) {
		body += strlen(sep);
		trans->rsp_body = body;
	}

	/*
	 * %*c         : HTTP/1.0, 1.1 etc, discard read value
	 * %4d         : HTTP status code, e.g. 200
	 * %255[^\r\n] : HTTP status text, e.g. OK -- Reads max 255 bytes, including \0, not \r or \n
	 */
	if (sscanf(trans->rsp, "HTTP/1.%*c %4d %255[^\r\n]", &status, trans->status_desc) == 2)
		trans->status = status;
}

int http_transaction(http_t *client, http_trans_t *trans)
{
	int rc = 0;

	ASSERT(client);
	ASSERT(trans);

	if (!client->initialized)
		return RC_HTTP_OBJECT_NOT_INITIALIZED;

	trans->rsp_len = 0;
	do {
		TRY(ssl_send(client, trans->req, trans->req_len));
		TRY(ssl_recv(client, trans->rsp, trans->max_rsp_len, &trans->rsp_len));
	}
	while (0);

	trans->rsp[trans->rsp_len] = 0;
	http_response_parse(trans);

	return rc;
}

int http_status_valid(int status)
{
	if (status == 200)
		return 0;

	if (status == 401 || status == 403)
		return RC_DDNS_RSP_AUTH_FAIL;

	if (status >= 500 && status < 600)
		return RC_DDNS_RSP_RETRY_LATER;

	return RC_DDNS_RSP_NOTOK;
}

int http_set_port(http_t *client, int port)
{
	ASSERT(client);
	return tcp_set_port(&client->tcp, port);
}

int http_get_port(http_t *client, int *port)
{
	ASSERT(client);
	return tcp_get_port(&client->tcp, port);
}


int http_set_remote_name(http_t *client, const char *name)
{
	ASSERT(client);
	return tcp_set_remote_name(&client->tcp, name);
}

int http_get_remote_name(http_t *client, const char **name)
{
	ASSERT(client);
	return tcp_get_remote_name(&client->tcp, name);
}

int http_set_remote_timeout(http_t *client, int timeout)
{
	ASSERT(client);
	return tcp_set_remote_timeout(&client->tcp, timeout);
}

int http_get_remote_timeout(http_t *client, int *timeout)
{
	ASSERT(client);
	return tcp_get_remote_timeout(&client->tcp, timeout);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
