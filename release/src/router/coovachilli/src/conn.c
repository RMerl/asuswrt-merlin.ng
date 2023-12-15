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

#include "chilli.h"
#include "debug.h"

#if defined(ENABLE_CHILLIPROXY) || defined(ENABLE_CHILLIRADSEC) || defined(ENABLE_CHILLIREDIR)

int conn_sock(struct conn_t *conn, struct in_addr *addr, int port) {
  struct sockaddr_in server;
  int sock;

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = addr->s_addr;
  
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) > 0) {

#if(_debug_)
    /*log_dbg("SETTING non-blocking");*/
#endif

    if (ndelay_on(sock) < 0) {
      log_err(errno, "could not set non-blocking");
    }

    if (safe_connect(sock, 
		     (struct sockaddr *) &server, 
		     sizeof(server)) < 0) {
      if (errno != EINPROGRESS) {
	log_err(errno, "could not connect to %s:%d", 
		inet_ntoa(server.sin_addr), port);
	close(sock);
	return -1;
      }
    }
  }

  conn->sock = sock;

  return 0;
}

int conn_setup(struct conn_t *conn, char *hostname, 
	       int port, bstring bwrite, bstring bread) {
  struct hostent *host;

  conn->write_pos = 0;
  conn->write_buf = bwrite;
  conn->read_pos = 0;
  conn->read_buf = bread;

  if (!(host = gethostbyname(hostname)) || !host->h_addr_list[0]) {
    log_err(0, "Could not resolve IP address of uamserver: %s! [%s]", 
	    hostname, strerror(errno));
    return -1;
  }

  return conn_sock(conn, (struct in_addr *)host->h_addr_list[0], port);
}

int conn_fd(struct conn_t *conn,
	    fd_set *r, fd_set *w, fd_set *e, int *m) {
  if (conn->sock) {
    FD_SET(conn->sock, r);
    if (conn->write_pos < conn->write_buf->slen) {
      FD_SET(conn->sock, w);
    }
    FD_SET(conn->sock, e);
    if (conn->sock > (*m)) {
      (*m) = conn->sock;
    }
  }
  return 0;
}

int conn_select_fd(struct conn_t *conn, select_ctx *sctx) {
  int evts = SELECT_READ;
  if (!conn->sock) return -1;
  if (conn->write_buf &&
      conn->write_pos < conn->write_buf->slen) 
    evts |= SELECT_WRITE;
  net_select_modfd(sctx, conn->sock, evts);
  return net_select_fd(sctx, conn->sock, evts);
}

void conn_finish(struct conn_t *conn) {
  if (conn->done_handler) {
    conn->done_handler(conn, conn->done_handler_ctx);
  } else {
    conn_close(conn);
  }
}

int conn_update_write(struct conn_t *conn) {
#if(_debug_)
  log_dbg("socket writeable!");
#endif
  
  if (conn->write_pos == 0) {
    int err;
    socklen_t errlen = sizeof(err);
    if (getsockopt(conn->sock, SOL_SOCKET, SO_ERROR, 
		   &err, &errlen) || (err != 0)) {
      log_err(errno, "not connected");
      conn_finish(conn);
      return -1;
    } else {
#if(_debug_)
      /*log_dbg("RESETTING non-blocking");*/
#endif
      /*if (ndelay_off(conn->sock) < 0) {
	log_err(errno, "could not un-set non-blocking");
	}*/
    }
  }
  
  if (conn->write_pos < conn->write_buf->slen) {
    int ret = net_write(conn->sock, 
			conn->write_buf->data + conn->write_pos,
			conn->write_buf->slen - conn->write_pos);
    if (ret > 0) {
      /*log_dbg("write: %d bytes", ret);*/
      conn->write_pos += ret;
    } else if (ret < 0 || errno != EWOULDBLOCK) {
#if(_debug_)
      log_dbg("socket closed!");
#endif
      conn_finish(conn);
      return -1;
    }
  } 
  
  return 0;
}

int conn_select_update(struct conn_t *conn, select_ctx *sctx) {
  if (conn->sock) {
    switch (net_select_read_fd(sctx, conn->sock)) {
    case -1:
      log_dbg("exception");
      conn_finish(conn);
      return -1;
      
    case 1:
      if (conn->read_handler)
	conn->read_handler(conn, conn->read_handler_ctx);
      break;
    }
    
    if (net_select_write_fd(sctx, conn->sock)==1)
      conn_update_write(conn);
  }
  
  return 0;
}

int conn_update(struct conn_t *conn, fd_set *r, fd_set *w, fd_set *e) {

  if (conn->sock) {
    if (FD_ISSET(conn->sock, r)) {
      if (conn->read_handler) {
	conn->read_handler(conn, conn->read_handler_ctx);
      }
    }

    if (FD_ISSET(conn->sock, w)) {
      conn_update_write(conn);
    }

    if (FD_ISSET(conn->sock, e)) {
#if(_debug_)
      log_dbg("socket exception!");
#endif
      conn_finish(conn);
    }
  }

  return 0;
}

static int 
_conn_bstring_readhandler(struct conn_t *conn, void *ctx) {
  bstring data = (bstring)ctx;
  int ret;
  ballocmin(data, data->slen + 128);

  ret = safe_read(conn->sock, 
		  data->data + data->slen,
		  data->mlen - data->slen);

  if (ret > 0) {
#if(_debug_)
    log_dbg("bstring_read: %d bytes", ret);
#endif
    data->slen += ret;
  } else {
#if(_debug_)
    log_dbg("socket closed!");
    log_dbg("<== [%s]", data->data);
#endif
    conn_finish(conn);
  }

  return ret;
}

void conn_bstring_readhandler(struct conn_t *conn, bstring data) {
  conn->read_handler = _conn_bstring_readhandler;
  conn->read_handler_ctx = data;
}

void conn_set_readhandler(struct conn_t *conn, conn_handler handler, void *ctx) {
  conn->read_handler = handler;
  conn->read_handler_ctx = ctx;
}

void conn_set_donehandler(struct conn_t *conn, conn_handler handler, void *ctx) {
  conn->done_handler = handler;
  conn->done_handler_ctx = ctx;
}

int conn_close(struct conn_t *conn) {
  if (conn->sock) 
    close(conn->sock);
#ifdef HAVE_SSL
  if (conn->sslcon) {
    openssl_shutdown(conn->sslcon, 2);
    openssl_free(conn->sslcon);
    conn->sslcon = 0;
  }
#endif
  conn->sock = 0;
  return 0;
}

#endif
