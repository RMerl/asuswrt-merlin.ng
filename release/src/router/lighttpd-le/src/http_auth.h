#ifndef _HTTP_AUTH_H_
#define _HTTP_AUTH_H_
#include "first.h"

#include "base.h"

struct http_auth_scheme_t;
struct http_auth_require_t;
struct http_auth_backend_t;

typedef struct http_auth_require_t {
    const struct http_auth_scheme_t *scheme;
    buffer *realm;
    int valid_user;
    array *user;
    array *group;
    array *host;
} http_auth_require_t;

http_auth_require_t * http_auth_require_init (void);
void http_auth_require_free (http_auth_require_t *require);
int http_auth_match_rules (const http_auth_require_t *require, const char *user, const char *group, const char *host);

typedef struct http_auth_backend_t {
    const char *name;
    handler_t(*basic)(server *srv, connection *con, void *p_d, const http_auth_require_t *require, const buffer *username, const char *pw);
    handler_t(*digest)(server *srv, connection *con, void *p_d, const char *username, const char *realm, unsigned char HA1[16]);
    void *p_d;
} http_auth_backend_t;

typedef struct http_auth_scheme_t {
    const char *name;
    handler_t(*checkfn)(server *srv, connection *con, void *p_d, const struct http_auth_require_t *require, const struct http_auth_backend_t *backend);
    /*(backend is arg only because auth.backend is separate config directive)*/
    void *p_d;
} http_auth_scheme_t;

const http_auth_scheme_t * http_auth_scheme_get (const buffer *name);
void http_auth_scheme_set (const http_auth_scheme_t *scheme);
const http_auth_backend_t * http_auth_backend_get (const buffer *name);
void http_auth_backend_set (const http_auth_backend_t *backend);

void http_auth_setenv(array *env, const char *username, size_t ulen, const char *auth_type, size_t alen);

int http_auth_md5_hex2bin (const char *md5hex, size_t len, unsigned char md5bin[16]);

#endif
