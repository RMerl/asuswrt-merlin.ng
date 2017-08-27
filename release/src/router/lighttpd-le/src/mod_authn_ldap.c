#include "first.h"

#define USE_LDAP
#include <ldap.h>

#include "server.h"
#include "http_auth.h"
#include "log.h"
#include "plugin.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

typedef struct {
    LDAP *ldap;

    buffer *auth_ldap_hostname;
    buffer *auth_ldap_basedn;
    buffer *auth_ldap_binddn;
    buffer *auth_ldap_bindpw;
    buffer *auth_ldap_filter;
    buffer *auth_ldap_cafile;
    unsigned short auth_ldap_starttls;
    unsigned short auth_ldap_allow_empty_pw;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config **config_storage;
    plugin_config conf, *anon_conf; /* this is only used as long as no handler_ctx is setup */

    buffer *ldap_filter;
} plugin_data;

static handler_t mod_authn_ldap_basic(server *srv, connection *con, void *p_d, const http_auth_require_t *require, const buffer *username, const char *pw);

INIT_FUNC(mod_authn_ldap_init) {
    static http_auth_backend_t http_auth_backend_ldap =
      { "ldap", mod_authn_ldap_basic, NULL, NULL };
    plugin_data *p = calloc(1, sizeof(*p));
    p->ldap_filter = buffer_init();

    /* register http_auth_backend_ldap */
    http_auth_backend_ldap.p_d = p;
    http_auth_backend_set(&http_auth_backend_ldap);

    return p;
}

FREE_FUNC(mod_authn_ldap_free) {
    plugin_data *p = p_d;

    UNUSED(srv);

    if (!p) return HANDLER_GO_ON;

    buffer_free(p->ldap_filter);

    if (p->config_storage) {
        size_t i;
        for (i = 0; i < srv->config_context->used; i++) {
            plugin_config *s = p->config_storage[i];

            if (NULL == s) continue;

            buffer_free(s->auth_ldap_hostname);
            buffer_free(s->auth_ldap_basedn);
            buffer_free(s->auth_ldap_binddn);
            buffer_free(s->auth_ldap_bindpw);
            buffer_free(s->auth_ldap_filter);
            buffer_free(s->auth_ldap_cafile);

            if (NULL != s->ldap) ldap_unbind_ext_s(s->ldap, NULL, NULL);
            free(s);
        }
        free(p->config_storage);
    }

    free(p);

    return HANDLER_GO_ON;
}

SETDEFAULTS_FUNC(mod_authn_ldap_set_defaults) {
    plugin_data *p = p_d;
    size_t i;
config_values_t cv[] = {
        { "auth.backend.ldap.hostname",     NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION }, /* 0 */
        { "auth.backend.ldap.base-dn",      NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION }, /* 1 */
        { "auth.backend.ldap.filter",       NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION }, /* 2 */
        { "auth.backend.ldap.ca-file",      NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION }, /* 3 */
        { "auth.backend.ldap.starttls",     NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 4 */
        { "auth.backend.ldap.bind-dn",      NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION }, /* 5 */
        { "auth.backend.ldap.bind-pw",      NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION }, /* 6 */
        { "auth.backend.ldap.allow-empty-pw",     NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 7 */
        { NULL,                             NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
    };

    p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

    for (i = 0; i < srv->config_context->used; i++) {
        data_config const* config = (data_config const*)srv->config_context->data[i];
        plugin_config *s;

        s = calloc(1, sizeof(plugin_config));

        s->auth_ldap_hostname = buffer_init();
        s->auth_ldap_basedn = buffer_init();
        s->auth_ldap_binddn = buffer_init();
        s->auth_ldap_bindpw = buffer_init();
        s->auth_ldap_filter = buffer_init();
        s->auth_ldap_cafile = buffer_init();
        s->auth_ldap_starttls = 0;
        s->ldap = NULL;

        cv[0].destination = s->auth_ldap_hostname;
        cv[1].destination = s->auth_ldap_basedn;
        cv[2].destination = s->auth_ldap_filter;
        cv[3].destination = s->auth_ldap_cafile;
        cv[4].destination = &(s->auth_ldap_starttls);
        cv[5].destination = s->auth_ldap_binddn;
        cv[6].destination = s->auth_ldap_bindpw;
        cv[7].destination = &(s->auth_ldap_allow_empty_pw);

        p->config_storage[i] = s;

        if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
            return HANDLER_ERROR;
        }

        if (!buffer_string_is_empty(s->auth_ldap_filter)) {
            if (*s->auth_ldap_filter->ptr != ','
                && NULL == strchr(s->auth_ldap_filter->ptr, '$')) {
                log_error_write(srv, __FILE__, __LINE__, "s", "ldap: auth.backend.ldap.filter is missing a replace-operator '$'");

                return HANDLER_ERROR;
            }
        }
    }

    return HANDLER_GO_ON;
}

#define PATCH(x) \
    p->conf.x = s->x;
static int mod_authn_ldap_patch_connection(server *srv, connection *con, plugin_data *p) {
    size_t i, j;
    plugin_config *s = p->config_storage[0];

    PATCH(auth_ldap_hostname);
    PATCH(auth_ldap_basedn);
    PATCH(auth_ldap_binddn);
    PATCH(auth_ldap_bindpw);
    PATCH(auth_ldap_filter);
    PATCH(auth_ldap_cafile);
    PATCH(auth_ldap_starttls);
    PATCH(auth_ldap_allow_empty_pw);
    p->anon_conf = s;

    /* skip the first, the global context */
    for (i = 1; i < srv->config_context->used; i++) {
        data_config *dc = (data_config *)srv->config_context->data[i];
        s = p->config_storage[i];

        /* condition didn't match */
        if (!config_check_cond(srv, con, dc)) continue;

        /* merge config */
        for (j = 0; j < dc->value->used; j++) {
            data_unset *du = dc->value->data[j];

            if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.hostname"))) {
                PATCH(auth_ldap_hostname);
                p->anon_conf = s;
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.base-dn"))) {
                PATCH(auth_ldap_basedn);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.filter"))) {
                PATCH(auth_ldap_filter);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.ca-file"))) {
                PATCH(auth_ldap_cafile);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.starttls"))) {
                PATCH(auth_ldap_starttls);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.bind-dn"))) {
                PATCH(auth_ldap_binddn);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.bind-pw"))) {
                PATCH(auth_ldap_bindpw);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("auth.backend.ldap.allow-empty-pw"))) {
                PATCH(auth_ldap_allow_empty_pw);
            }
        }
    }

    return 0;
}
#undef PATCH

static void mod_authn_ldap_err(server *srv, const char *file, unsigned long line, const char *fn, int err)
{
    log_error_write(srv,file,line,"sSss","ldap:",fn,":",ldap_err2string(err));
}

static void mod_authn_ldap_opt_err(server *srv, const char *file, unsigned long line, const char *fn, LDAP *ld)
{
    int err;
    ldap_get_option(ld, LDAP_OPT_ERROR_NUMBER, &err);
    mod_authn_ldap_err(srv, file, line, fn, err);
}

static LDAP * mod_authn_ldap_host_init(server *srv, plugin_config *s) {
    LDAP *ld;
    int ret;

    if (buffer_string_is_empty(s->auth_ldap_hostname)) return NULL;

    ld = ldap_init(s->auth_ldap_hostname->ptr, LDAP_PORT);
    if (NULL == ld) {
        log_error_write(srv, __FILE__, __LINE__, "sss", "ldap:", "ldap_init():",
                        strerror(errno));
        return NULL;
    }

    ret = LDAP_VERSION3;
    ret = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ret);
    if (LDAP_OPT_SUCCESS != ret) {
        mod_authn_ldap_err(srv, __FILE__, __LINE__, "ldap_set_options()", ret);
        ldap_memfree(ld);
        return NULL;
    }

    if (s->auth_ldap_starttls) {
        /* if no CA file is given, it is ok, as we will use encryption
         * if the server requires a CAfile it will tell us */
        if (!buffer_string_is_empty(s->auth_ldap_cafile)) {
            ret = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE,
                                  s->auth_ldap_cafile->ptr);
            if (LDAP_OPT_SUCCESS != ret) {
                mod_authn_ldap_err(srv, __FILE__, __LINE__,
                                   "ldap_set_option(LDAP_OPT_X_TLS_CACERTFILE)",
                                   ret);
                ldap_memfree(ld);
                return NULL;
            }
        }

        ret = ldap_start_tls_s(ld, NULL,  NULL);
        if (LDAP_OPT_SUCCESS != ret) {
            mod_authn_ldap_err(srv,__FILE__,__LINE__,"ldap_start_tls_s()",ret);
            ldap_memfree(ld);
            return NULL;
        }
    }

    return ld;
}

static int mod_authn_ldap_bind(server *srv, LDAP *ld, const char *dn, const char *pw) {
  #if 0
    struct berval creds;
    int ret;

    if (NULL != pw) {
        *((const char **)&creds.bv_val) = pw; /*(cast away const)*/
        creds.bv_len = strlen(pw);
    } else {
        creds.bv_val = NULL;
        creds.bv_len = 0;
    }

    /* RFE: add functionality: LDAP_SASL_EXTERNAL (or GSS-SPNEGO, etc.) */

    ret = ldap_sasl_bind_s(ld,dn,LDAP_SASL_SIMPLE,&creds,NULL,NULL,NULL);
    if (ret != LDAP_SUCCESS) {
        mod_authn_ldap_err(srv, __FILE__, __LINE__, "ldap_sasl_bind_s()", ret);
    }
  #else
    int ret = ldap_simple_bind_s(ld, dn, pw);
    if (ret != LDAP_SUCCESS) {
        mod_authn_ldap_err(srv, __FILE__, __LINE__, "ldap_simple_bind_s()",ret);
    }
  #endif

    return ret;
}

static LDAPMessage * mod_authn_ldap_search(server *srv, plugin_config *s, char *base, char *filter) {
    LDAPMessage *lm = NULL;
    char *attrs[] = { LDAP_NO_ATTRS, NULL };
    int ret;

    /*
     * 1. connect anonymously (if not already connected)
     *    (ldap connection is kept open unless connection-level error occurs)
     * 2. issue search using filter
     */

    if (s->ldap != NULL) {
        ret = ldap_search_ext_s(s->ldap, base, LDAP_SCOPE_SUBTREE, filter,
                                attrs, 0, NULL, NULL, NULL, 0, &lm);
        if (LDAP_SUCCESS == ret) {
            return lm;
        } else if (LDAP_SERVER_DOWN != ret) {
            /* try again (or initial request);
             * ldap lib sometimes fails for the first call but reconnects */
            ret = ldap_search_ext_s(s->ldap, base, LDAP_SCOPE_SUBTREE, filter,
                                    attrs, 0, NULL, NULL, NULL, 0, &lm);
            if (LDAP_SUCCESS == ret) {
                return lm;
            }
        }

        ldap_unbind_ext_s(s->ldap, NULL, NULL);
    }

    s->ldap = mod_authn_ldap_host_init(srv, s);
    if (NULL == s->ldap) {
        return NULL;
    }

    ret = !buffer_string_is_empty(s->auth_ldap_binddn)
      ? mod_authn_ldap_bind(srv, s->ldap,
                            s->auth_ldap_binddn->ptr,
                            s->auth_ldap_bindpw->ptr)
      : mod_authn_ldap_bind(srv, s->ldap, NULL, NULL);
    if (LDAP_SUCCESS != ret) {
        ldap_memfree(s->ldap);
        s->ldap = NULL;
        return NULL;
    }

    ret = ldap_search_ext_s(s->ldap, base, LDAP_SCOPE_SUBTREE, filter,
                            attrs, 0, NULL, NULL, NULL, 0, &lm);
    if (LDAP_SUCCESS != ret) {
        log_error_write(srv, __FILE__, __LINE__, "sSss",
                        "ldap:", ldap_err2string(ret), "; filter:", filter);
        ldap_unbind_ext_s(s->ldap, NULL, NULL);
        s->ldap = NULL;
        return NULL;
    }

    return lm;
}

static char * mod_authn_ldap_get_dn(server *srv, plugin_config *s, char *base, char *filter) {
    LDAP *ld;
    LDAPMessage *lm, *first;
    char *dn;
    int count;

    lm = mod_authn_ldap_search(srv, s, base, filter);
    if (NULL == lm) {
        return NULL;
    }

    ld = s->ldap; /*(must be after mod_authn_ldap_search(); might reconnect)*/

    count = ldap_count_entries(ld, lm);
    if (0 == count) { /*(no entires found)*/
        ldap_msgfree(lm);
        return NULL;
    } else if (count > 1) {
        log_error_write(srv, __FILE__, __LINE__, "sss",
                        "ldap:", "more than one record returned.  "
                        "you might have to refine the filter:", filter);
    }

    if (NULL == (first = ldap_first_entry(ld, lm))) {
        mod_authn_ldap_opt_err(srv,__FILE__,__LINE__,"ldap_first_entry()",ld);
        ldap_msgfree(lm);
        return NULL;
    }

    if (NULL == (dn = ldap_get_dn(ld, first))) {
        mod_authn_ldap_opt_err(srv,__FILE__,__LINE__,"ldap_get_dn()",ld);
        ldap_msgfree(lm);
        return NULL;
    }

    ldap_msgfree(lm);
    return dn;
}

static handler_t mod_authn_ldap_basic(server *srv, connection *con, void *p_d, const http_auth_require_t *require, const buffer *username, const char *pw) {
    plugin_data *p = (plugin_data *)p_d;
    LDAP *ld;
    char *dn;
    buffer *template;

    mod_authn_ldap_patch_connection(srv, con, p);

    if (pw[0] == '\0' && !p->conf.auth_ldap_allow_empty_pw)
        return HANDLER_ERROR;

    /* check username
     *
     * we have to protect againt username which modifies our filter in
     * an unpleasant way
     */

    for (size_t i = 0, len = buffer_string_length(username); i < len; i++) {
        char c = username->ptr[i];

        if (!isalpha(c) &&
            !isdigit(c) &&
            (c != ' ') &&
            (c != '@') &&
            (c != '-') &&
            (c != '_') &&
            (c != '.') ) {

            log_error_write(srv, __FILE__, __LINE__, "sbd",
                            "ldap: invalid character (- _.@a-zA-Z0-9 allowed) "
                            "in username:", username, i);

            con->http_status = 400; /* Bad Request */
            con->mode = DIRECT;
            return HANDLER_FINISHED;
        }
    }

    template = p->conf.auth_ldap_filter;
    if (buffer_string_is_empty(template)) {
        return HANDLER_ERROR;
    }

    /* build filter to get DN for uid = username */
    buffer_string_set_length(p->ldap_filter, 0);
    if (*template->ptr == ',') {
        /* special-case filter template beginning with ',' to be explicit DN */
        buffer_append_string_len(p->ldap_filter, CONST_STR_LEN("uid="));
        buffer_append_string_buffer(p->ldap_filter, username);
        buffer_append_string_buffer(p->ldap_filter, template);
        dn = p->ldap_filter->ptr;
    } else {
        for (char *b = template->ptr, *d; *b; b = d+1) {
            if (NULL != (d = strchr(b, '$'))) {
                buffer_append_string_len(p->ldap_filter, b, (size_t)(d - b));
                buffer_append_string_buffer(p->ldap_filter, username);
            } else {
                d = template->ptr + buffer_string_length(template);
                buffer_append_string_len(p->ldap_filter, b, (size_t)(d - b));
                break;
            }
        }

        /* ldap_search for DN (synchronous; blocking) */
        dn = mod_authn_ldap_get_dn(srv, p->anon_conf,
                                   p->conf.auth_ldap_basedn->ptr,
                                   p->ldap_filter->ptr);
        if (NULL == dn) {
            return HANDLER_ERROR;
        }
    }

    /* auth against LDAP server (synchronous; blocking) */

    ld = mod_authn_ldap_host_init(srv, &p->conf);
    if (NULL == ld) {
        if (dn != p->ldap_filter->ptr) ldap_memfree(dn);
        return HANDLER_ERROR;
    }

    if (LDAP_SUCCESS != mod_authn_ldap_bind(srv, ld, dn, pw)) {
        ldap_memfree(ld);
        if (dn != p->ldap_filter->ptr) ldap_memfree(dn);
        return HANDLER_ERROR;
    }

    ldap_unbind_ext_s(ld, NULL, NULL); /* disconnect */
    if (dn != p->ldap_filter->ptr) ldap_memfree(dn);
    return http_auth_match_rules(require, username->ptr, NULL, NULL)
      ? HANDLER_GO_ON  /* access granted */
      : HANDLER_ERROR;
}

int mod_authn_ldap_plugin_init(plugin *p);
int mod_authn_ldap_plugin_init(plugin *p) {
    p->version     = LIGHTTPD_VERSION_ID;
    p->name        = buffer_init_string("authn_ldap");
    p->init        = mod_authn_ldap_init;
    p->set_defaults = mod_authn_ldap_set_defaults;
    p->cleanup     = mod_authn_ldap_free;

    p->data        = NULL;

    return 0;
}
