/* Interface for HTTP functions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2020  Joachim Nilsson <troglobit@gmail.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef INADYN_HTTP_H_
#define INADYN_HTTP_H_

#include "config.h"

#if defined(CONFIG_OPENSSL)
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <openssl/err.h>
#elif defined(CONFIG_GNUTLS)
#include <gnutls/gnutls.h>
#endif

#include "error.h"
#include "os.h"
#include "tcp.h"

#define HTTP_DEFAULT_TIMEOUT	10000	/* msec */
#define	HTTP_DEFAULT_PORT	80
#define	HTTPS_DEFAULT_PORT	443

typedef struct {
	tcp_sock_t tcp;

	int        ssl_enabled;
#ifdef ENABLE_SSL
#ifdef CONFIG_OPENSSL
	SSL       *ssl;
	SSL_CTX   *ssl_ctx;
#else
	gnutls_session_t ssl;
#endif
#endif

	int        initialized;
} http_t;

typedef struct {
	char *req;
	int   req_len;

	char *rsp;
	int   rsp_len;
	int   max_rsp_len;

	char *rsp_body;

	int   status;
	char  status_desc[256];
} http_trans_t;

int http_construct          (http_t *client);
int http_destruct           (http_t *client, int num);

int http_init               (http_t *client, char *msg);
int http_exit               (http_t *client);

int http_transaction        (http_t *client, http_trans_t *trans);
int http_status_valid       (int status);

int http_set_port           (http_t *client, int  porg);
int http_get_port           (http_t *client, int *port);

int http_set_remote_name    (http_t *client, const char  *name);
int http_get_remote_name    (http_t *client, const char **name);

int http_set_remote_timeout (http_t *client, int  timeout);
int http_get_remote_timeout (http_t *client, int *timeout);

#endif /* INADYN_HTTP_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
