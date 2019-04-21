/* 
   neon-specific test utils
   Copyright (C) 2001-2009, Joe Orton <joe@manyfish.co.uk>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef UTILS_H
#define UTILS_H 1

#include "ne_request.h"

#include "child.h"

#define ONREQ(x) do { int _ret = (x); if (_ret) { t_context("line %d: HTTP error:\n%s", __LINE__, ne_get_error(sess)); return FAIL; } } while (0);

int single_serve_string(ne_socket *s, void *userdata);

struct many_serve_args {
    int count;
    const char *str;
};

/* Serves args->str response args->count times down a single
 * connection. */
int many_serve_string(ne_socket *s, void *userdata);

/* Run a request using URI on the session. */
int any_request(ne_session *sess, const char *uri);

/* Run a request using URI on the session; fail on a non-2xx response.
 */
int any_2xx_request(ne_session *sess, const char *uri);

/* As above but with a request body. */
int any_2xx_request_body(ne_session *sess, const char *uri);

/* makes *session, spawns server which will run 'fn(userdata,
 * socket)'.  sets error context if returns non-zero, i.e use like:
 * CALL(make_session(...)); */
int make_session(ne_session **sess, server_fn fn, void *userdata);

/* Server which sleeps for 10 seconds then closes the socket. */
int sleepy_server(ne_socket *sock, void *userdata);

struct string {
    char *data;
    size_t len;
};

struct double_serve_args {
    struct string first, second;
};

/* Serve a struct string. */
int serve_sstring(ne_socket *sock, void *ud);

/* Discards an HTTP request, serves response ->first, discards another
 * HTTP request, then serves response ->second. */
int double_serve_sstring(ne_socket *s, void *userdata);

/* Serve a struct string slowly. */
int serve_sstring_slowly(ne_socket *sock, void *ud);

struct infinite {
    const char *header, *repeat;
};

/* Pass a "struct infinite *" as userdata, this function sends
 * ->header and then loops sending ->repeat forever. */
int serve_infinite(ne_socket *sock, void *ud);

/* SOCKS server stuff. */
struct socks_server {
    enum ne_sock_sversion version;
    enum socks_failure {
        fail_none = 0,
        fail_init_vers,
        fail_init_close,
        fail_init_trunc,
        fail_no_auth,
        fail_bogus_auth, 
        fail_auth_close, 
        fail_auth_denied 
    } failure;
    unsigned int expect_port;
    ne_inet_addr *expect_addr;
    const char *expect_fqdn;
    const char *username;
    const char *password;
    int say_hello;
    server_fn server;
    void *userdata;
};

int socks_server(ne_socket *sock, void *userdata);

int full_write(ne_socket *sock, const char *data, size_t len);
    
/* Create a session with server process running fn(userdata).  Sets
 * test suite error on failure; initializes *sess with a new session
 * on success.  Uses an unspecified hostname/port for the server. */
int session_server(ne_session **sess, server_fn fn, void *userdata);

/* Create a session with server process running fn(userdata).  Sets
 * test suite error on failure; initializes *sess with a new session
 * on success.  Uses an unspecified hostname/port for the server;
 * session is created as if using origin 'host:fakeport' via HTTP
 * proxy to spawned server.  */
int proxied_session_server(ne_session **sess, const char *scheme,
                           const char *host, unsigned int fakeport,
                           server_fn fn, void *userdata);

/* As per proxied_session_server, but uses a "fake" (direct) TCP proxy
 * rather than an HTTP proxy. */
int fakeproxied_session_server(ne_session **sess, const char *scheme,
                               const char *host, unsigned int fakeport,
                               server_fn fn, void *userdata);

/* Read contents of file 'filename' into buffer 'buf'. */
int file_to_buffer(const char *filename, ne_buffer *buf);
   
#endif /* UTILS_H */
