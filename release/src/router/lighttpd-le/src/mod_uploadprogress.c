#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#include "response.h"
#include "stat_cache.h"

#include <stdlib.h>
#include <string.h>

/**
 * this is a uploadprogress for a lighttpd plugin
 *
 */

typedef struct {
	buffer     *con_id;
	connection *con;
} connection_map_entry;

typedef struct {
	connection_map_entry **ptr;

	size_t used;
	size_t size;
} connection_map;

/* plugin config for all request/connections */

typedef struct {
	buffer *progress_url;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	connection_map *con_map;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

/**
 *
 * connection maps
 *
 */

/* init the plugin data */
static connection_map *connection_map_init() {
	connection_map *cm;

	cm = calloc(1, sizeof(*cm));

	return cm;
}

static void connection_map_free(connection_map *cm) {
	size_t i;
	for (i = 0; i < cm->size; i++) {
		connection_map_entry *cme = cm->ptr[i];

		if (!cme) break;

		if (cme->con_id) {
			buffer_free(cme->con_id);
		}
		free(cme);
	}

	free(cm);
}

static int connection_map_insert(connection_map *cm, connection *con, buffer *con_id) {
	connection_map_entry *cme;
	size_t i;

	if (cm->size == 0) {
		cm->size = 16;
		cm->ptr = malloc(cm->size * sizeof(*(cm->ptr)));
		for (i = 0; i < cm->size; i++) {
			cm->ptr[i] = NULL;
		}
	} else if (cm->used == cm->size) {
		cm->size += 16;
		cm->ptr = realloc(cm->ptr, cm->size * sizeof(*(cm->ptr)));
		for (i = cm->used; i < cm->size; i++) {
			cm->ptr[i] = NULL;
		}
	}

	if (cm->ptr[cm->used]) {
		/* is already alloced, just reuse it */
		cme = cm->ptr[cm->used];
	} else {
		cme = malloc(sizeof(*cme));
		cme->con_id = buffer_init();
	}
	buffer_copy_buffer(cme->con_id, con_id);
	cme->con = con;

	cm->ptr[cm->used++] = cme;

	return 0;
}

static connection *connection_map_get_connection(connection_map *cm, buffer *con_id) {
	size_t i;

	for (i = 0; i < cm->used; i++) {
		connection_map_entry *cme = cm->ptr[i];

		if (buffer_is_equal(cme->con_id, con_id)) {
			/* found connection */

			return cme->con;
		}
	}
	return NULL;
}

static int connection_map_remove_connection(connection_map *cm, connection *con) {
	size_t i;

	for (i = 0; i < cm->used; i++) {
		connection_map_entry *cme = cm->ptr[i];

		if (cme->con == con) {
			/* found connection */

			buffer_reset(cme->con_id);
			cme->con = NULL;

			cm->used--;

			/* swap positions with the last entry */
			if (cm->used) {
				cm->ptr[i] = cm->ptr[cm->used];
				cm->ptr[cm->used] = cme;
			}

			return 1;
		}
	}

	return 0;
}

/* init the plugin data */
INIT_FUNC(mod_uploadprogress_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	p->con_map = connection_map_init();

	return p;
}

/* detroy the plugin data */
FREE_FUNC(mod_uploadprogress_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (NULL == s) continue;

			buffer_free(s->progress_url);

			free(s);
		}
		free(p->config_storage);
	}

	connection_map_free(p->con_map);

	free(p);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_uploadprogress_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "upload-progress.progress-url", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ NULL,                         NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->progress_url    = buffer_init();

		cv[0].destination = s->progress_url;

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}
	}

	return HANDLER_GO_ON;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_uploadprogress_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(progress_url);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("upload-progress.progress-url"))) {
				PATCH(progress_url);
			}
		}
	}

	return 0;
}
#undef PATCH

/**
 *
 * the idea:
 *
 * for the first request we check if it is a post-request
 *
 * if no, move out, don't care about them
 *
 * if yes, take the connection structure and register it locally
 * in the progress-struct together with an session-id (md5 ... )
 *
 * if the connections closes, cleanup the entry in the progress-struct
 *
 * a second request can now get the info about the size of the upload,
 * the received bytes
 *
 */

URIHANDLER_FUNC(mod_uploadprogress_uri_handler) {
	plugin_data *p = p_d;
	size_t i, len;
	data_string *ds;
	buffer *b;
	connection *post_con = NULL;

	UNUSED(srv);

	if (buffer_string_is_empty(con->uri.path)) return HANDLER_GO_ON;
	switch(con->request.http_method) {
	case HTTP_METHOD_GET:
	case HTTP_METHOD_POST: break;
	default:               return HANDLER_GO_ON;
	}

	mod_uploadprogress_patch_connection(srv, con, p);
	if (buffer_string_is_empty(p->conf.progress_url)) return HANDLER_GO_ON;

	if (con->request.http_method == HTTP_METHOD_GET) {
		if (!buffer_is_equal(con->uri.path, p->conf.progress_url)) {
			return HANDLER_GO_ON;
		}
	}

		/* the request has to contain a 32byte ID */

		if (NULL == (ds = (data_string *)array_get_element(con->request.headers, "X-Progress-ID"))) {
			if (!buffer_string_is_empty(con->uri.query)) {
				/* perhaps the POST request is using the querystring to pass the X-Progress-ID */
				b = con->uri.query;
			} else {
				return HANDLER_GO_ON;
			}
		} else {
			b = ds->value;
		}

		len = buffer_string_length(b);
		if (len != 32) {
			log_error_write(srv, __FILE__, __LINE__, "sd",
					"len of progress-id != 32:", len);
			return HANDLER_GO_ON;
		}

		for (i = 0; i < len; i++) {
			char c = b->ptr[i];

			if (!light_isxdigit(c)) {
				log_error_write(srv, __FILE__, __LINE__, "sb",
						"non-xdigit in progress-id:", b);
				return HANDLER_GO_ON;
			}
		}

	/* check if this is a POST request */
	switch(con->request.http_method) {
	case HTTP_METHOD_POST:

		connection_map_insert(p->con_map, con, b);

		return HANDLER_GO_ON;
	case HTTP_METHOD_GET:
		buffer_reset(con->physical.path);

		con->file_started = 1;
		con->file_finished = 1;

		con->http_status = 200;
		con->mode = DIRECT;

		/* get the connection */
		if (NULL == (post_con = connection_map_get_connection(p->con_map, b))) {
			log_error_write(srv, __FILE__, __LINE__, "sb",
					"ID no known:", b);

			chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("not in progress"));

			return HANDLER_FINISHED;
		}

		response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/xml"));

		/* just an attempt the force the IE/proxies to NOT cache the request ... doesn't help :( */
		response_header_overwrite(srv, con, CONST_STR_LEN("Pragma"), CONST_STR_LEN("no-cache"));
		response_header_overwrite(srv, con, CONST_STR_LEN("Expires"), CONST_STR_LEN("Thu, 19 Nov 1981 08:52:00 GMT"));
		response_header_overwrite(srv, con, CONST_STR_LEN("Cache-Control"), CONST_STR_LEN("no-store, no-cache, must-revalidate, post-check=0, pre-check=0"));

		b = buffer_init();

		/* prepare XML */
		buffer_copy_string_len(b, CONST_STR_LEN(
			"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>"
			"<upload>"
			"<size>"));
		buffer_append_int(b, post_con->request.content_length);
		buffer_append_string_len(b, CONST_STR_LEN(
			"</size>"
			"<received>"));
		buffer_append_int(b, post_con->request_content_queue->bytes_in);
		buffer_append_string_len(b, CONST_STR_LEN(
			"</received>"
			"</upload>"));

#if 0
		log_error_write(srv, __FILE__, __LINE__, "sb", "...", b);
#endif

		chunkqueue_append_buffer(con->write_queue, b);
		buffer_free(b);

		return HANDLER_FINISHED;
	default:
		break;
	}

	return HANDLER_GO_ON;
}

REQUESTDONE_FUNC(mod_uploadprogress_request_done) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (con->request.http_method != HTTP_METHOD_POST) return HANDLER_GO_ON;
	if (buffer_string_is_empty(con->uri.path)) return HANDLER_GO_ON;

	if (connection_map_remove_connection(p->con_map, con)) {
		/* removed */
	}

	return HANDLER_GO_ON;
}

/* this function is called at dlopen() time and inits the callbacks */

int mod_uploadprogress_plugin_init(plugin *p);
int mod_uploadprogress_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("uploadprogress");

	p->init        = mod_uploadprogress_init;
	p->handle_uri_clean  = mod_uploadprogress_uri_handler;
	p->handle_request_done  = mod_uploadprogress_request_done;
	p->handle_connection_close  = mod_uploadprogress_request_done;
	p->set_defaults  = mod_uploadprogress_set_defaults;
	p->cleanup     = mod_uploadprogress_free;

	p->data        = NULL;

	return 0;
}
