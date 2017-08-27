#include "first.h"

#include "http_auth.h"

#include <string.h>


static http_auth_scheme_t http_auth_schemes[8];

const http_auth_scheme_t * http_auth_scheme_get (const buffer *name)
{
    int i = 0;
    while (NULL != http_auth_schemes[i].name
           && 0 != strcmp(http_auth_schemes[i].name, name->ptr)) {
        ++i;
    }
    return (NULL != http_auth_schemes[i].name) ? http_auth_schemes+i : NULL;
}

void http_auth_scheme_set (const http_auth_scheme_t *scheme)
{
    unsigned int i = 0;
    while (NULL != http_auth_schemes[i].name) ++i;
    /*(must resize http_auth_schemes[] if too many different auth schemes)*/
    force_assert(i<(sizeof(http_auth_schemes)/sizeof(http_auth_scheme_t))-1);
    memcpy(http_auth_schemes+i, scheme, sizeof(http_auth_scheme_t));
}


static http_auth_backend_t http_auth_backends[8];

const http_auth_backend_t * http_auth_backend_get (const buffer *name)
{
    int i = 0;
    while (NULL != http_auth_backends[i].name
           && 0 != strcmp(http_auth_backends[i].name, name->ptr)) {
        ++i;
    }
    return (NULL != http_auth_backends[i].name) ? http_auth_backends+i : NULL;
}

void http_auth_backend_set (const http_auth_backend_t *backend)
{
    unsigned int i = 0;
    while (NULL != http_auth_backends[i].name) ++i;
    /*(must resize http_auth_backends[] if too many different auth backends)*/
    force_assert(i<(sizeof(http_auth_backends)/sizeof(http_auth_backend_t))-1);
    memcpy(http_auth_backends+i, backend, sizeof(http_auth_backend_t));
}

http_auth_require_t * http_auth_require_init (void)
{
    http_auth_require_t *require = calloc(1, sizeof(http_auth_require_t));
    force_assert(NULL != require);

    require->realm = buffer_init();
    require->valid_user = 0;
    require->user = array_init();
    require->group = array_init();
    require->host = array_init();

    return require;
}

void http_auth_require_free (http_auth_require_t * const require)
{
    buffer_free(require->realm);
    array_free(require->user);
    array_free(require->group);
    array_free(require->host);
    free(require);
}

/* (case-sensitive version of array.c:array_get_index(),
 *  and common case expects small num of allowed tokens,
 *  so it is reasonably performant to simply walk the array) */
static int http_auth_array_contains (const array * const a, const char * const k, const size_t klen)
{
    for (size_t i = 0, used = a->used; i < used; ++i) {
        if (buffer_is_equal_string(a->data[i]->key, k, klen)) {
            return 1;
        }
    }
    return 0;
}

int http_auth_match_rules (const http_auth_require_t * const require, const char * const user, const char * const group, const char * const host)
{
    if (NULL != user
        && (require->valid_user
            || http_auth_array_contains(require->user, user, strlen(user)))) {
        return 1; /* match */
    }

    if (NULL != group
        && http_auth_array_contains(require->group, group, strlen(group))) {
        return 1; /* match */
    }

    if (NULL != host
        && http_auth_array_contains(require->host, host, strlen(host))) {
        return 1; /* match */
    }

    return 0; /* no match */
}

void http_auth_setenv(array *env, const char *username, size_t ulen, const char *auth_type, size_t alen) {
    data_string *ds;

    /* REMOTE_USER */

    if (NULL == (ds = (data_string *)array_get_element(env, "REMOTE_USER"))) {
        if (NULL == (ds = (data_string *)array_get_unused_element(env, TYPE_STRING))) {
            ds = data_string_init();
        }
        buffer_copy_string_len(ds->key, CONST_STR_LEN("REMOTE_USER"));
        array_insert_unique(env, (data_unset *)ds);
    }
    buffer_copy_string_len(ds->value, username, ulen);

    /* AUTH_TYPE */

    if (NULL == (ds = (data_string *)array_get_element(env, "AUTH_TYPE"))) {
        if (NULL == (ds = (data_string *)array_get_unused_element(env, TYPE_STRING))) {
            ds = data_string_init();
        }
        buffer_copy_string_len(ds->key, CONST_STR_LEN("AUTH_TYPE"));
        array_insert_unique(env, (data_unset *)ds);
    }
    buffer_copy_string_len(ds->value, auth_type, alen);
}

int http_auth_md5_hex2bin (const char *md5hex, size_t len, unsigned char md5bin[16])
{
    /* validate and transform 32-byte MD5 hex string to 16-byte binary MD5 */
    if (32 != len) return -1; /*(Note: char *md5hex must be a 32-char string)*/
    for (int i = 0; i < 32; i+=2) {
        int hi = md5hex[i];
        int lo = md5hex[i+1];
        if ('0' <= hi && hi <= '9')                    hi -= '0';
        else if ((hi |= 0x20), 'a' <= hi && hi <= 'f') hi += -'a' + 10;
        else                                           return -1;
        if ('0' <= lo && lo <= '9')                    lo -= '0';
        else if ((lo |= 0x20), 'a' <= lo && lo <= 'f') lo += -'a' + 10;
        else                                           return -1;
        md5bin[(i >> 1)] = (unsigned char)((hi << 4) | lo);
    }
    return 0;
}

#if 0
int http_auth_md5_hex2lc (char *md5hex)
{
    /* validate and transform 32-byte MD5 hex string to lowercase */
    int i;
    for (i = 0; md5hex[i]; ++i) {
        int c = md5hex[i];
        if ('0' <= c && c <= '9')                   continue;
        else if ((c |= 0x20), 'a' <= c && c <= 'f') md5hex[i] = c;
        else                                        return -1;
    }
    return (32 == i) ? 0 : -1; /*(Note: char *md5hex must be a 32-char string)*/
}
#endif
