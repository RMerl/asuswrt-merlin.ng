/* Interface for optional HTTPS functions
 *
 * Copyright (C) 2014-2021  Joachim Wiberg <troglobit@gmail.com>
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

#ifndef INADYN_SSL_H_
#define INADYN_SSL_H_

#include "config.h"
#include "http.h"
#include "tcp.h"

/* User can supply local CA PEM bundle in .conf file */
extern char *ca_trust_file;

/* Cert validation is enabled by default, user can disable in .conf file */
extern int secure_ssl;
extern int broken_rtc;

#ifdef ENABLE_SSL
int     ssl_init(void);
void    ssl_exit(void);

int     ssl_open(http_t *client, char *msg);
int     ssl_close(http_t *client);

int     ssl_send(http_t *client, const char *buf, int     len);
int     ssl_recv(http_t *client,       char *buf, int buf_len, int *recv_len);

#else
#define ssl_init()  0
#define ssl_exit()

#define ssl_open(client, msg)                    tcp_init(&client->tcp, msg)
#define ssl_close(client)                        tcp_exit(&client->tcp)

#define ssl_send(client, buf, len)               tcp_send(&client->tcp, buf, len)
#define ssl_recv(client, buf, buf_len, recv_len) tcp_recv(&client->tcp, buf, buf_len, recv_len)

#endif /* ENABLE_SSL */
#endif /* INADYN_SSL_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
