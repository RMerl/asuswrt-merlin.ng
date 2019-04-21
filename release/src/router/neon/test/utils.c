/* 
   Utility functions for HTTP client tests
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

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for sleep() */
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <fcntl.h>

#include "ne_session.h"

#include "child.h"
#include "tests.h"
#include "utils.h"

static int serve_response(ne_socket *s, const char *response)
{
    CALL(discard_request(s));
    CALL(discard_body(s));
    ONN("failed to send response", SEND_STRING(s, response));
    return OK;
}    

int single_serve_string(ne_socket *s, void *userdata)
{
    const char *str = userdata;
    return serve_response(s, str);
}

int double_serve_sstring(ne_socket *s, void *userdata)
{
    struct double_serve_args *args = userdata;
    struct string *str;

    CALL(discard_request(s));
    CALL(discard_body(s));
    
    str = &args->first;
    NE_DEBUG(NE_DBG_SOCKET, "Serving string: [[[%.*s]]]\n",
	     (int)str->len, str->data);
    ONN("write failed", ne_sock_fullwrite(s, str->data, str->len));

    CALL(discard_request(s));
    CALL(discard_body(s));

    str = &args->second;
    NE_DEBUG(NE_DBG_SOCKET, "Serving string: [[[%.*s]]]\n",
	     (int)str->len, str->data);
    ONN("write failed", ne_sock_fullwrite(s, str->data, str->len));

    return OK;
}

int sleepy_server(ne_socket *sock, void *userdata)
{
    sleep(10);
    return 0;
}

int many_serve_string(ne_socket *s, void *userdata)
{
    int n;
    struct many_serve_args *args = userdata;
    
    for (n = 0; n < args->count; n++) {
	NE_DEBUG(NE_DBG_HTTP, "Serving response %d\n", n);
	CALL(serve_response(s, args->str));
    }

    return OK;
}

int any_request(ne_session *sess, const char *uri)
{
    ne_request *req = ne_request_create(sess, "GET", uri);
    int ret = ne_request_dispatch(req);
    ne_request_destroy(req);
    return ret;
}

int any_2xx_request(ne_session *sess, const char *uri)
{
    ne_request *req = ne_request_create(sess, "GET", uri);
    int ret = ne_request_dispatch(req);
    int klass = ne_get_status(req)->klass;
    ne_request_destroy(req);
    ONV(ret != NE_OK || klass != 2,
	("request failed: %s", ne_get_error(sess)));
    return ret;
}

int any_2xx_request_body(ne_session *sess, const char *uri)
{
    ne_request *req = ne_request_create(sess, "GET", uri);
#define BSIZE 5000
    char *body = memset(ne_malloc(BSIZE), 'A', BSIZE);
    int ret;
    ne_set_request_body_buffer(req, body, BSIZE);
    ret = ne_request_dispatch(req);
    ne_free(body);
    ONV(ret != NE_OK || ne_get_status(req)->klass != 2,
	("request failed: %s", ne_get_error(sess)));
    ne_request_destroy(req);
    return ret;
}

int serve_sstring(ne_socket *sock, void *ud)
{
    struct string *str = ud;

    NE_DEBUG(NE_DBG_SOCKET, "Serving string: [[[%.*s]]]\n",
	     (int)str->len, str->data);

    ONN("write failed", ne_sock_fullwrite(sock, str->data, str->len));
    
    return 0;
}

int serve_sstring_slowly(ne_socket *sock, void *ud)
{
    struct string *str = ud;
    size_t n;

    NE_DEBUG(NE_DBG_SOCKET, "Slowly serving string: [[[%.*s]]]\n",
	     (int)str->len, str->data);
    
    for (n = 0; n < str->len; n++) {
	ONN("write failed", ne_sock_fullwrite(sock, &str->data[n], 1));
	minisleep();
    }
    
    return 0;
}

int serve_infinite(ne_socket *sock, void *ud)
{
    struct infinite *i = ud;

    CALL(discard_request(sock));

    SEND_STRING(sock, i->header);

    while (server_send(sock, i->repeat, strlen(i->repeat)) == 0)
        /* nullop */;
    
    return OK;
}

int full_write(ne_socket *sock, const char *data, size_t len)
{
    int ret = ne_sock_fullwrite(sock, data, len);
    NE_DEBUG(NE_DBG_SOCKET, "wrote: [%.*s]\n", (int)len, data);
    ONV(ret, ("write failed (%d): %s", ret, ne_sock_error(sock)));
    return OK;
}

int session_server(ne_session **sess, server_fn fn, void *userdata)
{
    unsigned int port;
    
    CALL(new_spawn_server(1, fn, userdata, &port));
    
    *sess = ne_session_create("http", "127.0.0.1", port);

    return OK;
}

int proxied_session_server(ne_session **sess, const char *scheme,
                           const char *host, unsigned int fakeport,
                           server_fn fn, void *userdata)
{
    unsigned int port;
    
    CALL(new_spawn_server(1, fn, userdata, &port));
    
    *sess = ne_session_create(scheme, host, fakeport);

    NE_DEBUG(NE_DBG_HTTP, "test: Using proxied session to port %u.\n", port);

    ne_session_proxy(*sess, "127.0.0.1", port);

    return OK;
}

static void fakesess_destroy(void *userdata)
{
    ne_inet_addr *addr = userdata;

    ne_iaddr_free(addr);
}

int fakeproxied_session_server(ne_session **sess, const char *scheme,
                               const char *host, unsigned int fakeport,
                               server_fn fn, void *userdata)
{
    unsigned int port;
    ne_inet_addr *addr;
    const ne_inet_addr *alist[1];
    
    CALL(new_spawn_server2(1, fn, userdata, &addr, &port));
    
    alist[0] = addr;

    *sess = ne_session_create(scheme, host, fakeport);

    ne_set_addrlist2(*sess, port, alist, 1);

    ne_hook_destroy_session(*sess, fakesess_destroy, addr);

    return OK;
}

int make_session(ne_session **sess, server_fn fn, void *ud)
{
    return session_server(sess, fn, ud);
}

int file_to_buffer(const char *filename, ne_buffer *buf)
{
    char buffer[BUFSIZ];
    int fd;
    ssize_t n;
    
    fd = open(filename, O_RDONLY);
    ONV(fd < 0, ("could not open file %s", filename));

    while ((n = read(fd, buffer, BUFSIZ)) > 0) {
	ne_buffer_append(buf, buffer, n);
    }

    close(fd);
    
    return 0;
}
