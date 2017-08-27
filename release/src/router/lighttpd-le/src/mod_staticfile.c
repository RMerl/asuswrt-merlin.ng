#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#include "stat_cache.h"
#include "etag.h"
#include "http_chunk.h"
#include "response.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * this is a staticfile for a lighttpd plugin
 *
 */



/* plugin config for all request/connections */

typedef struct {
	array *exclude_ext;
	unsigned short etags_used;
	unsigned short disable_pathinfo;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

/* init the plugin data */
INIT_FUNC(mod_staticfile_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	return p;
}

/* detroy the plugin data */
FREE_FUNC(mod_staticfile_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (NULL == s) continue;

			array_free(s->exclude_ext);

			free(s);
		}
		free(p->config_storage);
	}

	free(p);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_staticfile_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "static-file.exclude-extensions", NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ "static-file.etags",    NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 1 */
		{ "static-file.disable-pathinfo", NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 2 */
		{ NULL,                         NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->exclude_ext    = array_init();
		s->etags_used     = 1;
		s->disable_pathinfo = 0;

		cv[0].destination = s->exclude_ext;
		cv[1].destination = &(s->etags_used);
		cv[2].destination = &(s->disable_pathinfo);

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}
	}

	return HANDLER_GO_ON;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_staticfile_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(exclude_ext);
	PATCH(etags_used);
	PATCH(disable_pathinfo);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("static-file.exclude-extensions"))) {
				PATCH(exclude_ext);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("static-file.etags"))) {
				PATCH(etags_used);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("static-file.disable-pathinfo"))) {
				PATCH(disable_pathinfo);
			}
		}
	}

	return 0;
}
#undef PATCH

URIHANDLER_FUNC(mod_staticfile_subrequest) {
	plugin_data *p = p_d;
	size_t k;
	data_string *ds;

	/* someone else has done a decision for us */
	if (con->http_status != 0) return HANDLER_GO_ON;
	if (buffer_is_empty(con->uri.path)) return HANDLER_GO_ON;
	if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;

	/* someone else has handled this request */
	if (con->mode != DIRECT) return HANDLER_GO_ON;

	/* we only handle GET, POST and HEAD */
	switch(con->request.http_method) {
	case HTTP_METHOD_GET:
	case HTTP_METHOD_POST:
	case HTTP_METHOD_HEAD:
		break;
	default:
		return HANDLER_GO_ON;
	}

	mod_staticfile_patch_connection(srv, con, p);

	if (p->conf.disable_pathinfo && !buffer_string_is_empty(con->request.pathinfo)) {
		if (con->conf.log_request_handling) {
			log_error_write(srv, __FILE__, __LINE__,  "s",  "-- NOT handling file as static file, pathinfo forbidden");
		}
		return HANDLER_GO_ON;
	}

	/* ignore certain extensions */
	for (k = 0; k < p->conf.exclude_ext->used; k++) {
		ds = (data_string *)p->conf.exclude_ext->data[k];

		if (buffer_is_empty(ds->value)) continue;

		if (buffer_is_equal_right_len(con->physical.path, ds->value, buffer_string_length(ds->value))) {
			if (con->conf.log_request_handling) {
				log_error_write(srv, __FILE__, __LINE__,  "s",  "-- NOT handling file as static file, extension forbidden");
			}
			return HANDLER_GO_ON;
		}
	}


	if (con->conf.log_request_handling) {
		log_error_write(srv, __FILE__, __LINE__,  "s",  "-- handling file as static file");
	}

	if (!p->conf.etags_used) con->etag_flags = 0;
	http_response_send_file(srv, con, con->physical.path);

	return HANDLER_FINISHED;
}

/* this function is called at dlopen() time and inits the callbacks */

int mod_staticfile_plugin_init(plugin *p);
int mod_staticfile_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("staticfile");

	p->init        = mod_staticfile_init;
	p->handle_subrequest_start = mod_staticfile_subrequest;
	p->set_defaults  = mod_staticfile_set_defaults;
	p->cleanup     = mod_staticfile_free;

	p->data        = NULL;

	return 0;
}
