#ifndef _RESPONSE_H_
#define _RESPONSE_H_
#include "first.h"

#include "server.h"

#include <time.h>

int http_response_parse(server *srv, connection *con);
int http_response_write_header(server *srv, connection *con);

int response_header_insert(server *srv, connection *con, const char *key, size_t keylen, const char *value, size_t vallen);
int response_header_overwrite(server *srv, connection *con, const char *key, size_t keylen, const char *value, size_t vallen);
int response_header_append(server *srv, connection *con, const char *key, size_t keylen, const char *value, size_t vallen);

typedef struct http_cgi_opts_t {
  int authorizer;
  int break_scriptfilename_for_php;
  buffer *docroot;
  buffer *strip_request_uri;
} http_cgi_opts;

typedef int (*http_cgi_header_append_cb)(void *vdata, const char *k, size_t klen, const char *v, size_t vlen);
int http_cgi_headers(server *srv, connection *con, http_cgi_opts *opts, http_cgi_header_append_cb cb, void *vdata);
#ifdef USE_OPENSSL
void http_cgi_ssl_env(server *srv, connection *con);
#endif

handler_t http_response_prepare(server *srv, connection *con);
int http_response_redirect_to_directory(server *srv, connection *con);
int http_response_handle_cachable(server *srv, connection *con, buffer * mtime);
void http_response_send_file (server *srv, connection *con, buffer *path);
void http_response_xsendfile (server *srv, connection *con, buffer *path, const array *xdocroot);
void http_response_backend_done (server *srv, connection *con);
void http_response_backend_error (server *srv, connection *con);

buffer * strftime_cache_get(server *srv, time_t last_mod);
#endif
