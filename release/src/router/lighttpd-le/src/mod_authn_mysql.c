#include "first.h"

/* mod_authn_mysql
 * 
 * KNOWN LIMITATIONS:
 * - no mechanism provided to configure SSL connection to a remote MySQL db
 *
 * FUTURE POTENTIAL PERFORMANCE ENHANCEMENTS:
 * - database response is not cached
 *   TODO: db response caching (for limited time) to reduce load on db
 *     (only cache successful logins to prevent cache bloat?)
 *     (or limit number of entries (size) of cache)
 *     (maybe have negative cache (limited size) of names not found in database)
 * - database query is synchronous and blocks waiting for response
 *   TODO: https://mariadb.com/kb/en/mariadb/using-the-non-blocking-library/
 * - opens and closes connection to MySQL db for each request (inefficient)
 *   (fixed) one-element cache for persistent connection open to last used db
 *   TODO: db connection pool (if asynchronous requests)
 */

#include <mysql.h>

#include "server.h"
#include "http_auth.h"
#include "log.h"
#include "md5.h"
#include "plugin.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

typedef struct {
    MYSQL  *mysql_conn;
    buffer *mysql_conn_host;
    buffer *mysql_conn_user;
    buffer *mysql_conn_pass;
    buffer *mysql_conn_db;
    int mysql_conn_port;
    int auth_mysql_port;
    buffer *auth_mysql_host;
    buffer *auth_mysql_user;
    buffer *auth_mysql_pass;
    buffer *auth_mysql_db;
    buffer *auth_mysql_socket;
    buffer *auth_mysql_users_table;
    buffer *auth_mysql_col_user;
    buffer *auth_mysql_col_pass;
    buffer *auth_mysql_col_realm;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config **config_storage;
    plugin_config conf;
} plugin_data;

static void mod_authn_mysql_sock_close(plugin_config *pconf) {
    if (NULL != pconf->mysql_conn) {
        mysql_close(pconf->mysql_conn);
        pconf->mysql_conn = NULL;
    }
}

static MYSQL * mod_authn_mysql_sock_connect(server *srv, plugin_config *pconf) {
    if (NULL != pconf->mysql_conn) {
        /* reuse open db connection if same ptrs to host user pass db port */
        if (   pconf->mysql_conn_host == pconf->auth_mysql_host
            && pconf->mysql_conn_user == pconf->auth_mysql_user
            && pconf->mysql_conn_pass == pconf->auth_mysql_pass
            && pconf->mysql_conn_db   == pconf->auth_mysql_db
            && pconf->mysql_conn_port == pconf->auth_mysql_port) {
            return pconf->mysql_conn;
        }
        mod_authn_mysql_sock_close(pconf);
    }

    /* !! mysql_init() is not thread safe !! (see MySQL doc) */
    pconf->mysql_conn = mysql_init(NULL);
    if (mysql_real_connect(pconf->mysql_conn,
                           pconf->auth_mysql_host->ptr,
                           pconf->auth_mysql_user->ptr,
                           pconf->auth_mysql_pass->ptr,
                           pconf->auth_mysql_db->ptr,
                           pconf->auth_mysql_port,
                           !buffer_string_is_empty(pconf->auth_mysql_socket)
                             ? pconf->auth_mysql_socket->ptr
                             : NULL,
                           CLIENT_IGNORE_SIGPIPE)) {
        /* (copy ptrs to config data (has lifetime until server shutdown)) */
        pconf->mysql_conn_host = pconf->auth_mysql_host;
        pconf->mysql_conn_user = pconf->auth_mysql_user;
        pconf->mysql_conn_pass = pconf->auth_mysql_pass;
        pconf->mysql_conn_db   = pconf->auth_mysql_db;
        pconf->mysql_conn_port = pconf->auth_mysql_port;
        return pconf->mysql_conn;
    }
    else {
        /*(note: any of these params might be buffers with b->ptr == NULL)*/
        log_error_write(srv, __FILE__, __LINE__, "sbsb"/*sb*/"sbss",
                        "opening connection to mysql:", pconf->auth_mysql_host,
                        "user:", pconf->auth_mysql_user,
                        /*"pass:", pconf->auth_mysql_pass,*//*(omit from logs)*/
                        "db:",   pconf->auth_mysql_db,
                        "failed:", mysql_error(pconf->mysql_conn));
        mod_authn_mysql_sock_close(pconf);
        return NULL;
    }
}

static MYSQL * mod_authn_mysql_sock_acquire(server *srv, plugin_config *pconf) {
    return mod_authn_mysql_sock_connect(srv, pconf);
}

static void mod_authn_mysql_sock_release(server *srv, plugin_config *pconf) {
    UNUSED(srv);
    UNUSED(pconf);
    /*(empty; leave db connection open)*/
    /* Note: mod_authn_mysql_result() calls mod_authn_mysql_sock_error()
     *       on error, so take that into account if making changes here.
     *       Must check if (NULL == pconf->mysql_conn) */
}

static void mod_authn_mysql_sock_error(server *srv, plugin_config *pconf) {
    UNUSED(srv);
    mod_authn_mysql_sock_close(pconf);
}

static handler_t mod_authn_mysql_basic(server *srv, connection *con, void *p_d, const http_auth_require_t *require, const buffer *username, const char *pw);
static handler_t mod_authn_mysql_digest(server *srv, connection *con, void *p_d, const char *username, const char *realm, unsigned char HA1[16]);

INIT_FUNC(mod_authn_mysql_init) {
    static http_auth_backend_t http_auth_backend_mysql =
      { "mysql", mod_authn_mysql_basic, mod_authn_mysql_digest, NULL };
    plugin_data *p = calloc(1, sizeof(*p));

    /* register http_auth_backend_mysql */
    http_auth_backend_mysql.p_d = p;
    http_auth_backend_set(&http_auth_backend_mysql);

    return p;
}

FREE_FUNC(mod_authn_mysql_free) {
    plugin_data *p = p_d;

    UNUSED(srv);

    if (!p) return HANDLER_GO_ON;

    if (p->config_storage) {
        size_t i;
        for (i = 0; i < srv->config_context->used; i++) {
            plugin_config *s = p->config_storage[i];

            if (NULL == s) continue;

            buffer_free(s->auth_mysql_host);
            buffer_free(s->auth_mysql_user);
            buffer_free(s->auth_mysql_pass);
            buffer_free(s->auth_mysql_db);
            buffer_free(s->auth_mysql_socket);
            buffer_free(s->auth_mysql_users_table);
            buffer_free(s->auth_mysql_col_user);
            buffer_free(s->auth_mysql_col_pass);
            buffer_free(s->auth_mysql_col_realm);

            if (s->mysql_conn) mod_authn_mysql_sock_close(s);

            free(s);
        }
        free(p->config_storage);
    }
    mod_authn_mysql_sock_close(&p->conf);

    free(p);

    return HANDLER_GO_ON;
}

SETDEFAULTS_FUNC(mod_authn_mysql_set_defaults) {
    plugin_data *p = p_d;
    size_t i;
    config_values_t cv[] = {
        { "auth.backend.mysql.host",        NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.user",        NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.pass",        NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.db",          NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.port",        NULL, T_CONFIG_INT,    T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.socket",      NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.users_table", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.col_user",    NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.col_pass",    NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { "auth.backend.mysql.col_realm",   NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
        { NULL,                             NULL, T_CONFIG_UNSET,  T_CONFIG_SCOPE_UNSET }
    };

    p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

    for (i = 0; i < srv->config_context->used; i++) {
        data_config const* config = (data_config const*)srv->config_context->data[i];
        plugin_config *s;

        s = calloc(1, sizeof(plugin_config));

        s->mysql_conn                             = NULL;
        s->auth_mysql_host                        = buffer_init();
        s->auth_mysql_user                        = buffer_init();
        s->auth_mysql_pass                        = buffer_init();
        s->auth_mysql_db                          = buffer_init();
        s->auth_mysql_socket                      = buffer_init();
        s->auth_mysql_users_table                 = buffer_init();
        s->auth_mysql_col_user                    = buffer_init();
        s->auth_mysql_col_pass                    = buffer_init();
        s->auth_mysql_col_realm                   = buffer_init();

        cv[0].destination = s->auth_mysql_host;
        cv[1].destination = s->auth_mysql_user;
        cv[2].destination = s->auth_mysql_pass;
        cv[3].destination = s->auth_mysql_db;
        cv[4].destination = &s->auth_mysql_port;
        cv[5].destination = s->auth_mysql_socket;
        cv[6].destination = s->auth_mysql_users_table;
        cv[7].destination = s->auth_mysql_col_user;
        cv[8].destination = s->auth_mysql_col_pass;
        cv[9].destination = s->auth_mysql_col_realm;

        p->config_storage[i] = s;

        if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
            return HANDLER_ERROR;
        }

        if (!buffer_is_empty(s->auth_mysql_col_user)
            && buffer_string_is_empty(s->auth_mysql_col_user)) {
            log_error_write(srv, __FILE__, __LINE__, "s",
                            "auth.backend.mysql.col_user must not be blank");
            return HANDLER_ERROR;
        }
        if (!buffer_is_empty(s->auth_mysql_col_pass)
            && buffer_string_is_empty(s->auth_mysql_col_pass)) {
            log_error_write(srv, __FILE__, __LINE__, "s",
                            "auth.backend.mysql.col_pass must not be blank");
            return HANDLER_ERROR;
        }
        if (!buffer_is_empty(s->auth_mysql_col_realm)
            && buffer_string_is_empty(s->auth_mysql_col_realm)) {
            log_error_write(srv, __FILE__, __LINE__, "s",
                            "auth.backend.mysql.col_realm must not be blank");
            return HANDLER_ERROR;
        }
    }

    if (p->config_storage[0]) { /*(always true)*/
        plugin_config *s = p->config_storage[0];
        if (buffer_is_empty(s->auth_mysql_col_user)) {
            s->auth_mysql_col_user = buffer_init_string("user");
        }
        if (buffer_is_empty(s->auth_mysql_col_pass)) {
            s->auth_mysql_col_pass = buffer_init_string("password");
        }
        if (buffer_is_empty(s->auth_mysql_col_realm)) {
            s->auth_mysql_col_realm = buffer_init_string("realm");
        }
    }

    return HANDLER_GO_ON;
}

#define PATCH(x) \
    p->conf.x = s->x;
static int mod_authn_mysql_patch_connection(server *srv, connection *con, plugin_data *p) {
    size_t i, j;
    plugin_config *s = p->config_storage[0];

    PATCH(auth_mysql_host);
    PATCH(auth_mysql_user);
    PATCH(auth_mysql_pass);
    PATCH(auth_mysql_db);
    PATCH(auth_mysql_port);
    PATCH(auth_mysql_socket);
    PATCH(auth_mysql_users_table);
    PATCH(auth_mysql_col_user);
    PATCH(auth_mysql_col_pass);
    PATCH(auth_mysql_col_realm);

    /* skip the first, the global context */
    for (i = 1; i < srv->config_context->used; i++) {
        data_config *dc = (data_config *)srv->config_context->data[i];
        s = p->config_storage[i];

        /* condition didn't match */
        if (!config_check_cond(srv, con, dc)) continue;

        /* merge config */
        for (j = 0; j < dc->value->used; j++) {
            data_unset *du = dc->value->data[j];

            if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.host"))) {
                PATCH(auth_mysql_host);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.user"))) {
                PATCH(auth_mysql_user);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.pass"))) {
                PATCH(auth_mysql_pass);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.db"))) {
                PATCH(auth_mysql_db);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.port"))) {
                PATCH(auth_mysql_port);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.socket"))) {
                PATCH(auth_mysql_socket);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.users_table"))) {
                PATCH(auth_mysql_users_table);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.col_user"))) {
                PATCH(auth_mysql_col_user);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.col_pass"))) {
                PATCH(auth_mysql_col_pass);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.mysql.col_realm"))) {
                PATCH(auth_mysql_col_realm);
            }
        }
    }

    return 0;
}
#undef PATCH

static int mod_authn_mysql_password_cmp(const char *userpw, unsigned long userpwlen, const char *reqpw) {
  #if defined(HAVE_CRYPT_R) || defined(HAVE_CRYPT)
    if (userpwlen >= 3 && userpw[0] == '$' && userpw[2] == '$') {
        /* md5 crypt()
         * request by Nicola Tiling <nti@w4w.net> */
        const char *saltb = userpw+3;
        const char *salte = strchr(saltb, '$');
        char salt[32];
        size_t slen = (NULL != salte) ? (size_t)(salte - saltb) : sizeof(salt);

        if (slen < sizeof(salt)) {
            char *crypted;
          #if defined(HAVE_CRYPT_R)
            struct crypt_data crypt_tmp_data;
           #ifdef _AIX
            memset(&crypt_tmp_data, 0, sizeof(crypt_tmp_data));
           #else
            crypt_tmp_data.initialized = 0;
           #endif
          #endif
            memcpy(salt, saltb, slen);
            salt[slen] = '\0';

          #if defined(HAVE_CRYPT_R)
            crypted = crypt_r(reqpw, salt, &crypt_tmp_data);
          #else
            crypted = crypt(reqpw, salt);
          #endif
            if (NULL != crypted) {
                return strcmp(userpw, crypted);
            }
        }
    }
    else
  #endif
    if (32 == userpwlen) {
        /* plain md5 */
        li_MD5_CTX Md5Ctx;
        unsigned char HA1[16];
        unsigned char md5pw[16];

        li_MD5_Init(&Md5Ctx);
        li_MD5_Update(&Md5Ctx, (unsigned char *)reqpw, strlen(reqpw));
        li_MD5_Final(HA1, &Md5Ctx);

        /*(compare 16-byte MD5 binary instead of converting to hex strings
         * in order to then have to do case-insensitive hex str comparison)*/
        return (0 == http_auth_md5_hex2bin(userpw, 32 /*(userpwlen)*/, md5pw))
          ? memcmp(HA1, md5pw, sizeof(md5pw))
          : -1;
    }

    return -1;
}

static int mod_authn_mysql_result(server *srv, plugin_data *p, const char *pw, unsigned char HA1[16]) {
    MYSQL_RES *result = mysql_store_result(p->conf.mysql_conn);
    int rc = -1;
    my_ulonglong num_rows;

    if (NULL == result) {
        /*(future: might log mysql_error() string)*/
      #if 0
        log_error_write(srv, __FILE__, __LINE__, "ss", "mysql_store_result:",
                        mysql_error(p->conf.mysql_conn));
      #endif
        mod_authn_mysql_sock_error(srv, &p->conf);
        return -1;
    }

    num_rows = mysql_num_rows(result);
    if (1 == num_rows) {
        MYSQL_ROW row = mysql_fetch_row(result);
        unsigned long *lengths = mysql_fetch_lengths(result);
        if (NULL == lengths) {
            /*(error; should not happen)*/
        }
        else if (pw) {  /* used with HTTP Basic auth */
            rc = mod_authn_mysql_password_cmp(row[0], lengths[0], pw);
        }
        else {          /* used with HTTP Digest auth */
            rc = http_auth_md5_hex2bin(row[0], lengths[0], HA1);
        }
    }
    else if (0 == num_rows) {
        /* user,realm not found */
    }
    else {
        /* (multiple rows returned, which should not happen) */
        /* (future: might log if multiple rows returned; unexpected result) */
    }
    mysql_free_result(result);
    return rc;
}

static handler_t mod_authn_mysql_query(server *srv, connection *con, void *p_d, const char *username, const char *realm, const char *pw, unsigned char HA1[16]) {
    plugin_data *p = (plugin_data *)p_d;
    int rc = -1;

    mod_authn_mysql_patch_connection(srv, con, p);

    if (buffer_string_is_empty(p->conf.auth_mysql_users_table)) {
        /*(auth.backend.mysql.host, auth.backend.mysql.db might be NULL; do not log)*/
        log_error_write(srv, __FILE__, __LINE__, "sb",
                        "auth config missing auth.backend.mysql.users_table for uri:",
                        con->request.uri);
        return HANDLER_ERROR;
    }

    do {
        size_t unamelen = strlen(username);
        size_t urealmlen = strlen(realm);
        char q[1024], uname[512], urealm[512];
        unsigned long mrc;

        if (unamelen > sizeof(uname)/2-1)
            return HANDLER_ERROR;
        if (urealmlen > sizeof(urealm)/2-1)
            return HANDLER_ERROR;

        if (!mod_authn_mysql_sock_acquire(srv, &p->conf)) {
            return HANDLER_ERROR;
        }

      #if 0
        mrc = mysql_real_escape_string_quote(p->conf.mysql_conn,uname,username,
                                             (unsigned long)unamelen, '\'');
        if ((unsigned long)~0 == mrc) break;

        mrc = mysql_real_escape_string_quote(p->conf.mysql_conn,urealm,realm,
                                             (unsigned long)urealmlen, '\'');
        if ((unsigned long)~0 == mrc) break;
      #else
        mrc = mysql_real_escape_string(p->conf.mysql_conn, uname,
                                       username, (unsigned long)unamelen);
        if ((unsigned long)~0 == mrc) break;

        mrc = mysql_real_escape_string(p->conf.mysql_conn, urealm,
                                       realm, (unsigned long)urealmlen);
        if ((unsigned long)~0 == mrc) break;
      #endif

        rc = snprintf(q, sizeof(q),
                      "SELECT %s FROM %s WHERE %s='%s' AND %s='%s'",
                      p->conf.auth_mysql_col_pass->ptr,
                      p->conf.auth_mysql_users_table->ptr,
                      p->conf.auth_mysql_col_user->ptr,
                      uname,
                      p->conf.auth_mysql_col_realm->ptr,
                      urealm);

        if (rc >= (int)sizeof(q)) {
            rc = -1;
            break;
        }

        /* for now we stay synchronous */
        if (0 != mysql_query(p->conf.mysql_conn, q)) {
            /* reconnect to db and retry once if query error occurs */
            mod_authn_mysql_sock_error(srv, &p->conf);
            if (!mod_authn_mysql_sock_acquire(srv, &p->conf)) {
                rc = -1;
                break;
            }
            if (0 != mysql_query(p->conf.mysql_conn, q)) {
                /*(note: any of these params might be bufs w/ b->ptr == NULL)*/
                log_error_write(srv, __FILE__, __LINE__, "sbsb"/*sb*/"sbssss",
                                "mysql_query host:", p->conf.auth_mysql_host,
                                "user:", p->conf.auth_mysql_user,
                                /*(omit pass from logs)*/
                                /*"pass:", p->conf.auth_mysql_pass,*/
                                "db:",   p->conf.auth_mysql_db,
                                "query:", q,
                                "failed:", mysql_error(p->conf.mysql_conn));
                rc = -1;
                break;
            }
        }

        rc = mod_authn_mysql_result(srv, p, pw, HA1);

    } while (0);

    mod_authn_mysql_sock_release(srv, &p->conf);

    return (0 == rc) ? HANDLER_GO_ON : HANDLER_ERROR;
}

static handler_t mod_authn_mysql_basic(server *srv, connection *con, void *p_d, const http_auth_require_t *require, const buffer *username, const char *pw) {
    /*(HA1 is not written since pw passed should not be NULL;
     * avoid passing NULL since subroutine expects unsigned char HA1[16] arg)*/
    static unsigned char HA1[16];
    char *realm = require->realm->ptr;
    handler_t rc =mod_authn_mysql_query(srv,con,p_d,username->ptr,realm,pw,HA1);
    if (HANDLER_GO_ON != rc) return rc;
    return http_auth_match_rules(require, username->ptr, NULL, NULL)
      ? HANDLER_GO_ON  /* access granted */
      : HANDLER_ERROR;
}

static handler_t mod_authn_mysql_digest(server *srv, connection *con, void *p_d, const char *username, const char *realm, unsigned char HA1[16]) {
    return mod_authn_mysql_query(srv,con,p_d,username,realm,NULL,HA1);
}

int mod_authn_mysql_plugin_init(plugin *p);
int mod_authn_mysql_plugin_init(plugin *p) {
    p->version     = LIGHTTPD_VERSION_ID;
    p->name        = buffer_init_string("authn_mysql");
    p->init        = mod_authn_mysql_init;
    p->set_defaults= mod_authn_mysql_set_defaults;
    p->cleanup     = mod_authn_mysql_free;

    p->data        = NULL;

    return 0;
}
