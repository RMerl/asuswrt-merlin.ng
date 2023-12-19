/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef _CHILLI_MODULE_H
#define _CHILLI_MODULE_H

struct chilli_module {
  void *lib;

  /*
   *   Modules shall return an integer code. 
   *   We shall use the lower 8 bits as a main code,
   *   the rest of the integer available for handler
   *   specific flags. 
   */

# define CHILLI_MOD_OK        0
# define CHILLI_MOD_ERROR    -1
# define CHILLI_MOD_CONTINUE  1
# define CHILLI_MOD_BREAK     2

  int (* initialize)      (char *, char isReload);
  int (* net_select)      (select_ctx *sctx);

# define CHILLI_MOD_REDIR_SKIP_RADIUS (1 << 8)
  int (* redir_login)     (struct redir_t *, 
			   struct redir_conn_t *,
			   struct redir_socket_t *);

  int (* dhcp_connect)    (struct app_conn_t *, 
			   struct dhcp_conn_t *);

  int (* dhcp_disconnect) (struct app_conn_t *, 
			   struct dhcp_conn_t *);

  int (* session_start)   (struct app_conn_t *);
  int (* session_update)  (struct app_conn_t *);
  int (* session_stop)    (struct app_conn_t *);

#define CHILLI_CMDSOCK_ERR -1
#define CHILLI_CMDSOCK_OK   0
  int (* cmdsock_handler) 
  (struct cmdsock_request *req, bstring s, int sock);

  /* lower level handers */
#define CHILLI_DNS_ERR   -1
#define CHILLI_DNS_DROP   0
#define CHILLI_DNS_OK     1
#define CHILLI_DNS_MOD    2
#define CHILLI_DNS_NAK    3
  int (* dns_handler) 
  (struct app_conn_t *, struct dhcp_conn_t *, uint8_t *, size_t *, int);

#define CHILLI_RADIUS_ERR  -1
#define CHILLI_RADIUS_OK    0
  int (* radius_handler)  
  (struct radius_t *radius, struct app_conn_t *conn,
   struct radius_packet_t *req, struct radius_packet_t *resp);

  /* lower level handers */
  size_t (* dhcp_handler) 
  (int, struct app_conn_t *, struct dhcp_conn_t *, 
   uint8_t *, size_t, uint8_t *, size_t);

  int (* destroy)         (char isReload);
};

#define chilli_mod_state(x) ((x)&0xff)

int chilli_module_load(void **ctx, char *name);
int chilli_module_unload(void *ctx);

#define CHILLI_MODULE_INIT 0

#endif
