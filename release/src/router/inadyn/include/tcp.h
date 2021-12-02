/* Interface for TCP functions
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
 * Boston, MA  02110-1301, USA.
 */

#ifndef INADYN_TCP_H_
#define INADYN_TCP_H_

#include "os.h"
#include "error.h"

#define TCP_DEFAULT_TIMEOUT		5000	/* msec */
#define TCP_SOCKET_MAX_PORT		65535
#define TCP_DEFAULT_READ_CHUNK_SIZE	100

typedef enum {
	NO_PROXY = 0,
	PROXY_SOCKS4,
	PROXY_SOCKS4A,
	PROXY_SOCKS5,
	PROXY_SOCKS5_HOSTNAME,
	PROXY_HTTP_CONNECT, /* SSL only. */
} tcp_proxy_type_t;

typedef struct {
	int                 initialized;

	int                 socket;
	const char         *remote_host;

	unsigned short      port;
	int                 timeout;

	tcp_proxy_type_t    proxy_type;
	const char         *proxy_host;
	unsigned short      proxy_port;
} tcp_sock_t;

int tcp_construct          (tcp_sock_t *tcp);
int tcp_destruct           (tcp_sock_t *tcp);

int tcp_init               (tcp_sock_t *tcp, char *msg);
int tcp_exit               (tcp_sock_t *tcp);

int tcp_send               (tcp_sock_t *tcp, const char *buf, int len);
int tcp_recv               (tcp_sock_t *tcp,       char *buf, int len, int *recv_len);

int tcp_set_port           (tcp_sock_t *tcp, int  port);
int tcp_get_port           (tcp_sock_t *tcp, int *port);

int tcp_set_remote_name    (tcp_sock_t *tcp, const char  *name);
int tcp_get_remote_name    (tcp_sock_t *tcp, const char **name);

int tcp_set_remote_timeout (tcp_sock_t *tcp, int  timeout);
int tcp_get_remote_timeout (tcp_sock_t *tcp, int *timeout);

#endif /* INADYN_TCP_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
