#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"
#include "response.h"
#include "connections.h"

#include "plugin.h"

#include "stat_cache.h"

#include "sys-mmap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <dirent.h>

#if defined(HAVE_LIBXML_H) && defined(HAVE_SQLITE3_H)
#define USE_PROPPATCH
#include <libxml/tree.h>
#include <libxml/parser.h>

#include <sqlite3.h>
#endif

#if defined(HAVE_LIBXML_H) && defined(HAVE_SQLITE3_H) && defined(HAVE_UUID_UUID_H)
#define USE_LOCKS
#include <uuid/uuid.h>
#endif

/**
 * this is a webdav for a lighttpd plugin
 *
 * at least a very basic one.
 * - for now it is read-only and we only support PROPFIND
 *
 */

#define WEBDAV_FILE_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
#define WEBDAV_DIR_MODE  S_IRWXU | S_IRWXG | S_IRWXO

/* plugin config for all request/connections */

typedef struct {
	unsigned short enabled;
	unsigned short is_readonly;
	unsigned short log_xml;

	buffer *sqlite_db_name;
#ifdef USE_PROPPATCH
	sqlite3 *sql;
	sqlite3_stmt *stmt_update_prop;
	sqlite3_stmt *stmt_delete_prop;
	sqlite3_stmt *stmt_select_prop;
	sqlite3_stmt *stmt_select_propnames;

	sqlite3_stmt *stmt_delete_uri;
	sqlite3_stmt *stmt_move_uri;
	sqlite3_stmt *stmt_copy_uri;

	sqlite3_stmt *stmt_remove_lock;
	sqlite3_stmt *stmt_create_lock;
	sqlite3_stmt *stmt_read_lock;
	sqlite3_stmt *stmt_read_lock_by_uri;
	sqlite3_stmt *stmt_refresh_lock;
#endif
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	buffer *tmp_buf;
	request_uri uri;
	physical physical;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

typedef struct {
	plugin_config conf;
} handler_ctx;

/* init the plugin data */
INIT_FUNC(mod_webdav_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	p->tmp_buf = buffer_init();

	p->uri.scheme = buffer_init();
	p->uri.path_raw = buffer_init();
	p->uri.path = buffer_init();
	p->uri.authority = buffer_init();

	p->physical.path = buffer_init();
	p->physical.rel_path = buffer_init();
	p->physical.doc_root = buffer_init();
	p->physical.basedir = buffer_init();

	return p;
}

/* detroy the plugin data */
FREE_FUNC(mod_webdav_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (NULL == s) continue;

			buffer_free(s->sqlite_db_name);
#ifdef USE_PROPPATCH
			if (s->sql) {
				sqlite3_finalize(s->stmt_delete_prop);
				sqlite3_finalize(s->stmt_delete_uri);
				sqlite3_finalize(s->stmt_copy_uri);
				sqlite3_finalize(s->stmt_move_uri);
				sqlite3_finalize(s->stmt_update_prop);
				sqlite3_finalize(s->stmt_select_prop);
				sqlite3_finalize(s->stmt_select_propnames);

				sqlite3_finalize(s->stmt_read_lock);
				sqlite3_finalize(s->stmt_read_lock_by_uri);
				sqlite3_finalize(s->stmt_create_lock);
				sqlite3_finalize(s->stmt_remove_lock);
				sqlite3_finalize(s->stmt_refresh_lock);
				sqlite3_close(s->sql);
			}
#endif
			free(s);
		}
		free(p->config_storage);
	}

	buffer_free(p->uri.scheme);
	buffer_free(p->uri.path_raw);
	buffer_free(p->uri.path);
	buffer_free(p->uri.authority);

	buffer_free(p->physical.path);
	buffer_free(p->physical.rel_path);
	buffer_free(p->physical.doc_root);
	buffer_free(p->physical.basedir);

	buffer_free(p->tmp_buf);

	free(p);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_webdav_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "webdav.activate",            NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ "webdav.is-readonly",         NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },       /* 1 */
		{ "webdav.sqlite-db-name",      NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION },       /* 2 */
		{ "webdav.log-xml",             NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },       /* 3 */
		{ NULL,                         NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->sqlite_db_name = buffer_init();

		cv[0].destination = &(s->enabled);
		cv[1].destination = &(s->is_readonly);
		cv[2].destination = s->sqlite_db_name;
		cv[3].destination = &(s->log_xml);

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}

		if (!buffer_string_is_empty(s->sqlite_db_name)) {
#ifdef USE_PROPPATCH
			const char *next_stmt;
			char *err;

			if (SQLITE_OK != sqlite3_open(s->sqlite_db_name->ptr, &(s->sql))) {
				log_error_write(srv, __FILE__, __LINE__, "sbs", "sqlite3_open failed for",
						s->sqlite_db_name,
						sqlite3_errmsg(s->sql));
				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_exec(s->sql,
					"CREATE TABLE IF NOT EXISTS properties ("
					"  resource TEXT NOT NULL,"
					"  prop TEXT NOT NULL,"
					"  ns TEXT NOT NULL,"
					"  value TEXT NOT NULL,"
					"  PRIMARY KEY(resource, prop, ns))",
					NULL, NULL, &err)) {

				if (0 != strcmp(err, "table properties already exists")) {
					log_error_write(srv, __FILE__, __LINE__, "ss", "can't open transaction:", err);
					sqlite3_free(err);

					return HANDLER_ERROR;
				}
				sqlite3_free(err);
			}

			if (SQLITE_OK != sqlite3_exec(s->sql,
					"CREATE TABLE IF NOT EXISTS locks ("
					"  locktoken TEXT NOT NULL,"
					"  resource TEXT NOT NULL,"
					"  lockscope TEXT NOT NULL,"
					"  locktype TEXT NOT NULL,"
					"  owner TEXT NOT NULL,"
					"  depth INT NOT NULL,"
					"  timeout TIMESTAMP NOT NULL,"
					"  PRIMARY KEY(locktoken))",
					NULL, NULL, &err)) {

				if (0 != strcmp(err, "table locks already exists")) {
					log_error_write(srv, __FILE__, __LINE__, "ss", "can't open transaction:", err);
					sqlite3_free(err);

					return HANDLER_ERROR;
				}
				sqlite3_free(err);
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("SELECT value FROM properties WHERE resource = ? AND prop = ? AND ns = ?"),
				&(s->stmt_select_prop), &next_stmt)) {
				/* prepare failed */

				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed:", sqlite3_errmsg(s->sql));
				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("SELECT ns, prop FROM properties WHERE resource = ?"),
				&(s->stmt_select_propnames), &next_stmt)) {
				/* prepare failed */

				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed:", sqlite3_errmsg(s->sql));
				return HANDLER_ERROR;
			}


			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("REPLACE INTO properties (resource, prop, ns, value) VALUES (?, ?, ?, ?)"),
				&(s->stmt_update_prop), &next_stmt)) {
				/* prepare failed */

				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed:", sqlite3_errmsg(s->sql));
				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("DELETE FROM properties WHERE resource = ? AND prop = ? AND ns = ?"),
				&(s->stmt_delete_prop), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("DELETE FROM properties WHERE resource = ?"),
				&(s->stmt_delete_uri), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("INSERT INTO properties SELECT ?, prop, ns, value FROM properties WHERE resource = ?"),
				&(s->stmt_copy_uri), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("UPDATE OR REPLACE properties SET resource = ? WHERE resource = ?"),
				&(s->stmt_move_uri), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			/* LOCKS */

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("INSERT INTO locks (locktoken, resource, lockscope, locktype, owner, depth, timeout) VALUES (?,?,?,?,?,?, CURRENT_TIME + 600)"),
				&(s->stmt_create_lock), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("DELETE FROM locks WHERE locktoken = ?"),
				&(s->stmt_remove_lock), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("SELECT locktoken, resource, lockscope, locktype, owner, depth, timeout-CURRENT_TIME FROM locks WHERE locktoken = ?"),
				&(s->stmt_read_lock), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("SELECT locktoken, resource, lockscope, locktype, owner, depth, timeout-CURRENT_TIME FROM locks WHERE resource = ?"),
				&(s->stmt_read_lock_by_uri), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}

			if (SQLITE_OK != sqlite3_prepare(s->sql,
				CONST_STR_LEN("UPDATE locks SET timeout = CURRENT_TIME + 600 WHERE locktoken = ?"),
				&(s->stmt_refresh_lock), &next_stmt)) {
				/* prepare failed */
				log_error_write(srv, __FILE__, __LINE__, "ss", "sqlite3_prepare failed", sqlite3_errmsg(s->sql));

				return HANDLER_ERROR;
			}


#else
			log_error_write(srv, __FILE__, __LINE__, "s", "Sorry, no sqlite3 and libxml2 support include, compile with --with-webdav-props");
			return HANDLER_ERROR;
#endif
		}
	}

	return HANDLER_GO_ON;
}

#define PATCH_OPTION(x) \
	p->conf.x = s->x;
static int mod_webdav_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH_OPTION(enabled);
	PATCH_OPTION(is_readonly);
	PATCH_OPTION(log_xml);

#ifdef USE_PROPPATCH
	PATCH_OPTION(sql);
	PATCH_OPTION(stmt_update_prop);
	PATCH_OPTION(stmt_delete_prop);
	PATCH_OPTION(stmt_select_prop);
	PATCH_OPTION(stmt_select_propnames);

	PATCH_OPTION(stmt_delete_uri);
	PATCH_OPTION(stmt_move_uri);
	PATCH_OPTION(stmt_copy_uri);

	PATCH_OPTION(stmt_remove_lock);
	PATCH_OPTION(stmt_refresh_lock);
	PATCH_OPTION(stmt_create_lock);
	PATCH_OPTION(stmt_read_lock);
	PATCH_OPTION(stmt_read_lock_by_uri);
#endif
	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("webdav.activate"))) {
				PATCH_OPTION(enabled);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("webdav.is-readonly"))) {
				PATCH_OPTION(is_readonly);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("webdav.log-xml"))) {
				PATCH_OPTION(log_xml);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("webdav.sqlite-db-name"))) {
#ifdef USE_PROPPATCH
				PATCH_OPTION(sql);
				PATCH_OPTION(stmt_update_prop);
				PATCH_OPTION(stmt_delete_prop);
				PATCH_OPTION(stmt_select_prop);
				PATCH_OPTION(stmt_select_propnames);

				PATCH_OPTION(stmt_delete_uri);
				PATCH_OPTION(stmt_move_uri);
				PATCH_OPTION(stmt_copy_uri);

				PATCH_OPTION(stmt_remove_lock);
				PATCH_OPTION(stmt_refresh_lock);
				PATCH_OPTION(stmt_create_lock);
				PATCH_OPTION(stmt_read_lock);
				PATCH_OPTION(stmt_read_lock_by_uri);
#endif
			}
		}
	}

	return 0;
}

URIHANDLER_FUNC(mod_webdav_uri_handler) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (buffer_is_empty(con->uri.path)) return HANDLER_GO_ON;

	mod_webdav_patch_connection(srv, con, p);

	if (!p->conf.enabled) return HANDLER_GO_ON;

	switch (con->request.http_method) {
	case HTTP_METHOD_OPTIONS:
		/* we fake a little bit but it makes MS W2k happy and it let's us mount the volume */
		response_header_overwrite(srv, con, CONST_STR_LEN("DAV"), CONST_STR_LEN("1,2"));
		response_header_overwrite(srv, con, CONST_STR_LEN("MS-Author-Via"), CONST_STR_LEN("DAV"));

		if (p->conf.is_readonly) {
			response_header_insert(srv, con, CONST_STR_LEN("Allow"), CONST_STR_LEN("PROPFIND"));
		} else {
			response_header_insert(srv, con, CONST_STR_LEN("Allow"), CONST_STR_LEN("PROPFIND, DELETE, MKCOL, PUT, MOVE, COPY, PROPPATCH, LOCK, UNLOCK"));
		}
		break;
	default:
		break;
	}

	/* not found */
	return HANDLER_GO_ON;
}
static int webdav_gen_prop_tag(server *srv, connection *con,
		char *prop_name,
		char *prop_ns,
		char *value,
		buffer *b) {

	UNUSED(srv);
	UNUSED(con);

	if (value) {
		buffer_append_string_len(b,CONST_STR_LEN("<"));
		buffer_append_string(b, prop_name);
		buffer_append_string_len(b, CONST_STR_LEN(" xmlns=\""));
		buffer_append_string(b, prop_ns);
		buffer_append_string_len(b, CONST_STR_LEN("\">"));

		buffer_append_string(b, value);

		buffer_append_string_len(b,CONST_STR_LEN("</"));
		buffer_append_string(b, prop_name);
		buffer_append_string_len(b, CONST_STR_LEN(">"));
	} else {
		buffer_append_string_len(b,CONST_STR_LEN("<"));
		buffer_append_string(b, prop_name);
		buffer_append_string_len(b, CONST_STR_LEN(" xmlns=\""));
		buffer_append_string(b, prop_ns);
		buffer_append_string_len(b, CONST_STR_LEN("\"/>"));
	}

	return 0;
}


static int webdav_gen_response_status_tag(server *srv, connection *con, physical *dst, int status, buffer *b) {
	UNUSED(srv);

	buffer_append_string_len(b,CONST_STR_LEN("<D:response xmlns:ns0=\"urn:uuid:c2f41010-65b3-11d1-a29f-00aa00c14882/\">\n"));

	buffer_append_string_len(b,CONST_STR_LEN("<D:href>\n"));
	buffer_append_string_buffer(b, dst->rel_path);
	buffer_append_string_len(b,CONST_STR_LEN("</D:href>\n"));
	buffer_append_string_len(b,CONST_STR_LEN("<D:status>\n"));

	if (con->request.http_version == HTTP_VERSION_1_1) {
		buffer_copy_string_len(b, CONST_STR_LEN("HTTP/1.1 "));
	} else {
		buffer_copy_string_len(b, CONST_STR_LEN("HTTP/1.0 "));
	}
	buffer_append_int(b, status);
	buffer_append_string_len(b, CONST_STR_LEN(" "));
	buffer_append_string(b, get_http_status_name(status));

	buffer_append_string_len(b,CONST_STR_LEN("</D:status>\n"));
	buffer_append_string_len(b,CONST_STR_LEN("</D:response>\n"));

	return 0;
}

static int webdav_delete_file(server *srv, connection *con, handler_ctx *hctx, physical *dst, buffer *b) {
	int status = 0;

	/* try to unlink it */
	if (-1 == unlink(dst->path->ptr)) {
		switch(errno) {
		case EACCES:
		case EPERM:
			/* 403 */
			status = 403;
			break;
		default:
			status = 501;
			break;
		}
		webdav_gen_response_status_tag(srv, con, dst, status, b);
	} else {
#ifdef USE_PROPPATCH
		sqlite3_stmt *stmt = hctx->conf.stmt_delete_uri;

		if (!stmt) {
			status = 403;
			webdav_gen_response_status_tag(srv, con, dst, status, b);
		} else {
			sqlite3_reset(stmt);

			/* bind the values to the insert */

			sqlite3_bind_text(stmt, 1,
				CONST_BUF_LEN(dst->rel_path),
				SQLITE_TRANSIENT);

			if (SQLITE_DONE != sqlite3_step(stmt)) {
				/* */
			}
		}
#else
		UNUSED(hctx);
#endif
	}

	return (status != 0);
}

static int webdav_delete_dir(server *srv, connection *con, handler_ctx *hctx, physical *dst, buffer *b) {
	DIR *dir;
	int have_multi_status = 0;
	physical d;

	d.path = buffer_init();
	d.rel_path = buffer_init();

	if (NULL != (dir = opendir(dst->path->ptr))) {
		struct dirent *de;

		while(NULL != (de = readdir(dir))) {
			struct stat st;

			if ((de->d_name[0] == '.' && de->d_name[1] == '\0')  ||
			  (de->d_name[0] == '.' && de->d_name[1] == '.' && de->d_name[2] == '\0')) {
				continue;
				/* ignore the parent dir */
			}

			buffer_copy_buffer(d.path, dst->path);
			buffer_append_slash(d.path);
			buffer_append_string(d.path, de->d_name);

			buffer_copy_buffer(d.rel_path, dst->rel_path);
			buffer_append_slash(d.rel_path);
			buffer_append_string(d.rel_path, de->d_name);

			/* stat and unlink afterwards */
			if (-1 == stat(d.path->ptr, &st)) {
				/* don't about it yet, rmdir will fail too */
			} else if (S_ISDIR(st.st_mode)) {
				have_multi_status = webdav_delete_dir(srv, con, hctx, &d, b);

				/* try to unlink it */
				if (-1 == rmdir(d.path->ptr)) {
					int status;
					switch(errno) {
					case EACCES:
					case EPERM:
						/* 403 */
						status = 403;
						break;
					default:
						status = 501;
						break;
					}
					have_multi_status = 1;

					webdav_gen_response_status_tag(srv, con, &d, status, b);
				} else {
#ifdef USE_PROPPATCH
					sqlite3_stmt *stmt = hctx->conf.stmt_delete_uri;

					if (stmt) {
						sqlite3_reset(stmt);

						/* bind the values to the insert */

						sqlite3_bind_text(stmt, 1,
							CONST_BUF_LEN(d.rel_path),
							SQLITE_TRANSIENT);

						if (SQLITE_DONE != sqlite3_step(stmt)) {
							/* */
						}
					}
#endif
				}
			} else {
				have_multi_status = webdav_delete_file(srv, con, hctx, &d, b);
			}
		}
		closedir(dir);

		buffer_free(d.path);
		buffer_free(d.rel_path);
	}

	return have_multi_status;
}

/* don't want to block when open()ing a fifo */
#if defined(O_NONBLOCK)
# define FIFO_NONBLOCK O_NONBLOCK
#else
# define FIFO_NONBLOCK 0
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

static int webdav_copy_file(server *srv, connection *con, handler_ctx *hctx, physical *src, physical *dst, int overwrite) {
	char *data;
	ssize_t rd, wr, offset;
	int status = 0, ifd, ofd;
	UNUSED(srv);
	UNUSED(con);

	if (-1 == (ifd = open(src->path->ptr, O_RDONLY | O_BINARY | FIFO_NONBLOCK))) {
		return 403;
	}

	if (-1 == (ofd = open(dst->path->ptr, O_WRONLY|O_TRUNC|O_CREAT|(overwrite ? 0 : O_EXCL), WEBDAV_FILE_MODE))) {
		/* opening the destination failed for some reason */
		switch(errno) {
		case EEXIST:
			status = 412;
			break;
		case EISDIR:
			status = 409;
			break;
		case ENOENT:
			/* at least one part in the middle wasn't existing */
			status = 409;
			break;
		default:
			status = 403;
			break;
		}
		close(ifd);
		return status;
	}

	data = malloc(131072);
	force_assert(data);

	while (0 < (rd = read(ifd, data, 131072))) {
		offset = 0;
		do {
			wr = write(ofd, data+offset, (size_t)(rd-offset));
		} while (wr >= 0 ? (offset += wr) != rd : (errno == EINTR));
		if (-1 == wr) {
			status = (errno == ENOSPC) ? 507 : 403;
			break;
		}

	}
	if (0 != rd && 0 == status) status = 403;

	free(data);
	close(ifd);
	if (0 != close(ofd)) {
		if (0 == status) status = (errno == ENOSPC) ? 507 : 403;
		log_error_write(srv, __FILE__, __LINE__, "sbss",
				"close ", dst->path, "failed: ", strerror(errno));
	}

#ifdef USE_PROPPATCH
	if (0 == status) {
		/* copy worked fine, copy connected properties */
		sqlite3_stmt *stmt = hctx->conf.stmt_copy_uri;

		if (stmt) {
			sqlite3_reset(stmt);

			/* bind the values to the insert */
			sqlite3_bind_text(stmt, 1,
				CONST_BUF_LEN(dst->rel_path),
				SQLITE_TRANSIENT);

			sqlite3_bind_text(stmt, 2,
				CONST_BUF_LEN(src->rel_path),
				SQLITE_TRANSIENT);

			if (SQLITE_DONE != sqlite3_step(stmt)) {
				/* */
			}
		}
	}
#else
	UNUSED(hctx);
#endif
	return status;
}

static int webdav_copy_dir(server *srv, connection *con, handler_ctx *hctx, physical *src, physical *dst, int overwrite) {
	DIR *srcdir;
	int status = 0;

	if (NULL != (srcdir = opendir(src->path->ptr))) {
		struct dirent *de;
		physical s, d;

		s.path = buffer_init();
		s.rel_path = buffer_init();

		d.path = buffer_init();
		d.rel_path = buffer_init();

		while (NULL != (de = readdir(srcdir))) {
			struct stat st;

			if ((de->d_name[0] == '.' && de->d_name[1] == '\0')
				|| (de->d_name[0] == '.' && de->d_name[1] == '.' && de->d_name[2] == '\0')) {
				continue;
			}

			buffer_copy_buffer(s.path, src->path);
			buffer_append_slash(s.path);
			buffer_append_string(s.path, de->d_name);

			buffer_copy_buffer(d.path, dst->path);
			buffer_append_slash(d.path);
			buffer_append_string(d.path, de->d_name);

			buffer_copy_buffer(s.rel_path, src->rel_path);
			buffer_append_slash(s.rel_path);
			buffer_append_string(s.rel_path, de->d_name);

			buffer_copy_buffer(d.rel_path, dst->rel_path);
			buffer_append_slash(d.rel_path);
			buffer_append_string(d.rel_path, de->d_name);

			if (-1 == stat(s.path->ptr, &st)) {
				/* why ? */
			} else if (S_ISDIR(st.st_mode)) {
				/* a directory */
				if (-1 == mkdir(d.path->ptr, WEBDAV_DIR_MODE) &&
				    errno != EEXIST) {
					/* WTH ? */
				} else {
#ifdef USE_PROPPATCH
					sqlite3_stmt *stmt = hctx->conf.stmt_copy_uri;

					if (0 != (status = webdav_copy_dir(srv, con, hctx, &s, &d, overwrite))) {
						break;
					}
					/* directory is copied, copy the properties too */

					if (stmt) {
						sqlite3_reset(stmt);

						/* bind the values to the insert */
						sqlite3_bind_text(stmt, 1,
							CONST_BUF_LEN(dst->rel_path),
							SQLITE_TRANSIENT);

						sqlite3_bind_text(stmt, 2,
							CONST_BUF_LEN(src->rel_path),
							SQLITE_TRANSIENT);

						if (SQLITE_DONE != sqlite3_step(stmt)) {
							/* */
						}
					}
#endif
				}
			} else if (S_ISREG(st.st_mode)) {
				/* a plain file */
				if (0 != (status = webdav_copy_file(srv, con, hctx, &s, &d, overwrite))) {
					break;
				}
			}
		}

		buffer_free(s.path);
		buffer_free(s.rel_path);
		buffer_free(d.path);
		buffer_free(d.rel_path);

		closedir(srcdir);
	}

	return status;
}

#ifdef USE_LOCKS
static void webdav_activelock(buffer *b,
		const buffer *locktoken, const char *lockscope, const char *locktype, int depth, int timeout) {
	buffer_append_string_len(b, CONST_STR_LEN("<D:activelock>\n"));

	buffer_append_string_len(b, CONST_STR_LEN("<D:lockscope>"));
	buffer_append_string_len(b, CONST_STR_LEN("<D:"));
	buffer_append_string(b, lockscope);
	buffer_append_string_len(b, CONST_STR_LEN("/>"));
	buffer_append_string_len(b, CONST_STR_LEN("</D:lockscope>\n"));

	buffer_append_string_len(b, CONST_STR_LEN("<D:locktype>"));
	buffer_append_string_len(b, CONST_STR_LEN("<D:"));
	buffer_append_string(b, locktype);
	buffer_append_string_len(b, CONST_STR_LEN("/>"));
	buffer_append_string_len(b, CONST_STR_LEN("</D:locktype>\n"));

	buffer_append_string_len(b, CONST_STR_LEN("<D:depth>"));
	buffer_append_string(b, depth == 0 ? "0" : "infinity");
	buffer_append_string_len(b, CONST_STR_LEN("</D:depth>\n"));

	buffer_append_string_len(b, CONST_STR_LEN("<D:timeout>"));
	buffer_append_string_len(b, CONST_STR_LEN("Second-"));
	buffer_append_int(b, timeout);
	buffer_append_string_len(b, CONST_STR_LEN("</D:timeout>\n"));

	buffer_append_string_len(b, CONST_STR_LEN("<D:owner>"));
	buffer_append_string_len(b, CONST_STR_LEN("</D:owner>\n"));

	buffer_append_string_len(b, CONST_STR_LEN("<D:locktoken>"));
	buffer_append_string_len(b, CONST_STR_LEN("<D:href>"));
	buffer_append_string_buffer(b, locktoken);
	buffer_append_string_len(b, CONST_STR_LEN("</D:href>"));
	buffer_append_string_len(b, CONST_STR_LEN("</D:locktoken>\n"));

	buffer_append_string_len(b, CONST_STR_LEN("</D:activelock>\n"));
}

static void webdav_get_live_property_lockdiscovery(server *srv, connection *con, handler_ctx *hctx, physical *dst, buffer *b) {

	sqlite3_stmt *stmt = hctx->conf.stmt_read_lock_by_uri;
	if (!stmt) { /*(should not happen)*/
		buffer_append_string_len(b, CONST_STR_LEN("<D:lockdiscovery>\n</D:lockdiscovery>\n"));
		return;
	}
	UNUSED(srv);
	UNUSED(con);

	/* SELECT locktoken, resource, lockscope, locktype, owner, depth, timeout
	 *   FROM locks
	 *  WHERE resource = ? */

	sqlite3_reset(stmt);

	sqlite3_bind_text(stmt, 1,
		CONST_BUF_LEN(dst->rel_path),
		SQLITE_TRANSIENT);

	buffer_append_string_len(b, CONST_STR_LEN("<D:lockdiscovery>\n"));
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		const char *lockscope = (const char *)sqlite3_column_text(stmt, 2);
		const char *locktype  = (const char *)sqlite3_column_text(stmt, 3);
		const int depth       =               sqlite3_column_int(stmt, 5);
		const int timeout     =               sqlite3_column_int(stmt, 6);
		buffer locktoken = { NULL, 0, 0 };
		locktoken.ptr         =       (char *)sqlite3_column_text(stmt, 0);
		locktoken.used        =               sqlite3_column_bytes(stmt, 0);
		if (locktoken.used) ++locktoken.used;
		locktoken.size = locktoken.used;

		if (timeout > 0) {
			webdav_activelock(b, &locktoken, lockscope, locktype, depth, timeout);
		}
	}
	buffer_append_string_len(b, CONST_STR_LEN("</D:lockdiscovery>\n"));
}
#endif

static int webdav_get_live_property(server *srv, connection *con, handler_ctx *hctx, physical *dst, char *prop_name, buffer *b) {
	stat_cache_entry *sce = NULL;
	int found = 0;

	UNUSED(hctx);

	if (HANDLER_ERROR != (stat_cache_get_entry(srv, con, dst->path, &sce))) {
		char ctime_buf[] = "2005-08-18T07:27:16Z";
		char mtime_buf[] = "Thu, 18 Aug 2005 07:27:16 GMT";
		size_t k;

		if (0 == strcmp(prop_name, "resourcetype")) {
			if (S_ISDIR(sce->st.st_mode)) {
				buffer_append_string_len(b, CONST_STR_LEN("<D:resourcetype><D:collection/></D:resourcetype>"));
			} else {
				buffer_append_string_len(b, CONST_STR_LEN("<D:resourcetype/>"));
			}
			found = 1;
		} else if (0 == strcmp(prop_name, "getcontenttype")) {
			if (S_ISDIR(sce->st.st_mode)) {
				buffer_append_string_len(b, CONST_STR_LEN("<D:getcontenttype>httpd/unix-directory</D:getcontenttype>"));
				found = 1;
			} else if(S_ISREG(sce->st.st_mode)) {
				for (k = 0; k < con->conf.mimetypes->used; k++) {
					data_string *ds = (data_string *)con->conf.mimetypes->data[k];

					if (buffer_is_empty(ds->key)) continue;

					if (buffer_is_equal_right_len(dst->path, ds->key, buffer_string_length(ds->key))) {
						buffer_append_string_len(b,CONST_STR_LEN("<D:getcontenttype>"));
						buffer_append_string_buffer(b, ds->value);
						buffer_append_string_len(b, CONST_STR_LEN("</D:getcontenttype>"));
						found = 1;

						break;
					}
				}
			}
		} else if (0 == strcmp(prop_name, "creationdate")) {
			buffer_append_string_len(b, CONST_STR_LEN("<D:creationdate ns0:dt=\"dateTime.tz\">"));
			strftime(ctime_buf, sizeof(ctime_buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&(sce->st.st_ctime)));
			buffer_append_string(b, ctime_buf);
			buffer_append_string_len(b, CONST_STR_LEN("</D:creationdate>"));
			found = 1;
		} else if (0 == strcmp(prop_name, "getlastmodified")) {
			buffer_append_string_len(b,CONST_STR_LEN("<D:getlastmodified ns0:dt=\"dateTime.rfc1123\">"));
			strftime(mtime_buf, sizeof(mtime_buf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&(sce->st.st_mtime)));
			buffer_append_string(b, mtime_buf);
			buffer_append_string_len(b, CONST_STR_LEN("</D:getlastmodified>"));
			found = 1;
		} else if (0 == strcmp(prop_name, "getcontentlength")) {
			buffer_append_string_len(b,CONST_STR_LEN("<D:getcontentlength>"));
			buffer_append_int(b, sce->st.st_size);
			buffer_append_string_len(b, CONST_STR_LEN("</D:getcontentlength>"));
			found = 1;
		} else if (0 == strcmp(prop_name, "getcontentlanguage")) {
			buffer_append_string_len(b,CONST_STR_LEN("<D:getcontentlanguage>"));
			buffer_append_string_len(b, CONST_STR_LEN("en"));
			buffer_append_string_len(b, CONST_STR_LEN("</D:getcontentlanguage>"));
			found = 1;
		} else if (0 == strcmp(prop_name, "getetag")) {
			etag_create(con->physical.etag, &sce->st, con->etag_flags);
			buffer_append_string_len(b, CONST_STR_LEN("<D:getetag>"));
			buffer_append_string_buffer(b, con->physical.etag);
			buffer_append_string_len(b, CONST_STR_LEN("</D:getetag>"));
			buffer_reset(con->physical.etag);
			found = 1;
	      #ifdef USE_LOCKS
		} else if (0 == strcmp(prop_name, "lockdiscovery")) {
			webdav_get_live_property_lockdiscovery(srv, con, hctx, dst, b);
			found = 1;
		} else if (0 == strcmp(prop_name, "supportedlock")) {
			buffer_append_string_len(b,CONST_STR_LEN("<D:supportedlock>"));
			buffer_append_string_len(b,CONST_STR_LEN("<D:lockentry>"));
			buffer_append_string_len(b,CONST_STR_LEN("<D:lockscope><D:exclusive/></D:lockscope>"));
			buffer_append_string_len(b,CONST_STR_LEN("<D:locktype><D:write/></D:locktype>"));
			buffer_append_string_len(b,CONST_STR_LEN("</D:lockentry>"));
			buffer_append_string_len(b, CONST_STR_LEN("</D:supportedlock>"));
			found = 1;
	      #endif
		}
	}

	return found ? 0 : -1;
}

static int webdav_get_property(server *srv, connection *con, handler_ctx *hctx, physical *dst, char *prop_name, char *prop_ns, buffer *b) {
	if (0 == strcmp(prop_ns, "DAV:")) {
		/* a local 'live' property */
		return webdav_get_live_property(srv, con, hctx, dst, prop_name, b);
	} else {
		int found = 0;
#ifdef USE_PROPPATCH
		sqlite3_stmt *stmt = hctx->conf.stmt_select_prop;

		if (stmt) {
			/* perhaps it is in sqlite3 */
			sqlite3_reset(stmt);

			/* bind the values to the insert */

			sqlite3_bind_text(stmt, 1,
				CONST_BUF_LEN(dst->rel_path),
				SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2,
				prop_name,
				strlen(prop_name),
				SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3,
				prop_ns,
				strlen(prop_ns),
				SQLITE_TRANSIENT);

			/* it is the PK */
			while (SQLITE_ROW == sqlite3_step(stmt)) {
				/* there is a row for us, we only expect a single col 'value' */
				webdav_gen_prop_tag(srv, con, prop_name, prop_ns, (char *)sqlite3_column_text(stmt, 0), b);
				found = 1;
			}
		}
#endif
		return found ? 0 : -1;
	}

	/* not found */
	return -1;
}

typedef struct {
	char *ns;
	char *prop;
} webdav_property;

static webdav_property live_properties[] = {
	{ "DAV:", "creationdate" },
	/*{ "DAV:", "displayname" },*//*(not implemented)*/
	{ "DAV:", "getcontentlanguage" },
	{ "DAV:", "getcontentlength" },
	{ "DAV:", "getcontenttype" },
	{ "DAV:", "getetag" },
	{ "DAV:", "getlastmodified" },
	{ "DAV:", "resourcetype" },
	/*{ "DAV:", "source" },*//*(not implemented)*/
      #ifdef USE_LOCKS
	{ "DAV:", "lockdiscovery" },
	{ "DAV:", "supportedlock" },
      #endif

	{ NULL, NULL }
};

typedef struct {
	webdav_property **ptr;

	size_t used;
	size_t size;
} webdav_properties;

static int webdav_get_props(server *srv, connection *con, handler_ctx *hctx, physical *dst, webdav_properties *props, buffer *b_200, buffer *b_404) {
	size_t i;

	if (props && props->used) {
		for (i = 0; i < props->used; i++) {
			webdav_property *prop;

			prop = props->ptr[i];

			if (0 != webdav_get_property(srv, con, hctx,
				dst, prop->prop, prop->ns, b_200)) {
				webdav_gen_prop_tag(srv, con, prop->prop, prop->ns, NULL, b_404);
			}
		}
	} else {
		for (i = 0; live_properties[i].prop; i++) {
			/* a local 'live' property */
			webdav_get_live_property(srv, con, hctx, dst, live_properties[i].prop, b_200);
		}
	}

	return 0;
}

#ifdef USE_PROPPATCH
static int webdav_parse_chunkqueue(server *srv, connection *con, handler_ctx *hctx, chunkqueue *cq, xmlDoc **ret_xml) {
	xmlParserCtxtPtr ctxt;
	xmlDoc *xml;
	int res;
	int err;

	chunk *c;

	UNUSED(con);

	/* read the chunks in to the XML document */
	ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);

	for (c = cq->first; cq->bytes_out != cq->bytes_in; c = cq->first) {
		size_t weWant = cq->bytes_out - cq->bytes_in;
		size_t weHave;
		int mapped;
		void *data;

		switch(c->type) {
		case FILE_CHUNK:
			weHave = c->file.length - c->offset;

			if (weHave > weWant) weHave = weWant;

			/* xml chunks are always memory, mmap() is our friend */
			mapped = (c->file.mmap.start != MAP_FAILED);
			if (mapped) {
				data = c->file.mmap.start + c->offset;
			} else {
				if (-1 == c->file.fd &&  /* open the file if not already open */
				    -1 == (c->file.fd = open(c->file.name->ptr, O_RDONLY))) {
					log_error_write(srv, __FILE__, __LINE__, "ss", "open failed: ", strerror(errno));

					return -1;
				}

				if (MAP_FAILED != (c->file.mmap.start = mmap(0, c->file.length, PROT_READ, MAP_PRIVATE, c->file.fd, 0))) {
					/* chunk_reset() or chunk_free() will cleanup for us */
					c->file.mmap.length = c->file.length;
					data = c->file.mmap.start + c->offset;
					mapped = 1;
				} else {
					ssize_t rd;
					if (weHave > 65536) weHave = 65536;
					data = malloc(weHave);
					force_assert(data);
					if (-1 == lseek(c->file.fd, c->file.start + c->offset, SEEK_SET)
					    || 0 > (rd = read(c->file.fd, data, weHave))) {
						log_error_write(srv, __FILE__, __LINE__, "ssbd", "lseek/read failed: ",
								strerror(errno), c->file.name, c->file.fd);
						free(data);
						return -1;
					}
					weHave = (size_t)rd;
				}
			}

			if (XML_ERR_OK != (err = xmlParseChunk(ctxt, data, weHave, 0))) {
				log_error_write(srv, __FILE__, __LINE__, "sodd", "xmlParseChunk failed at:", cq->bytes_out, weHave, err);
			}

			chunkqueue_mark_written(cq, weHave);

			if (!mapped) free(data);
			break;
		case MEM_CHUNK:
			/* append to the buffer */
			weHave = buffer_string_length(c->mem) - c->offset;

			if (weHave > weWant) weHave = weWant;

			if (hctx->conf.log_xml) {
				log_error_write(srv, __FILE__, __LINE__, "ss", "XML-request-body:", c->mem->ptr + c->offset);
			}

			if (XML_ERR_OK != (err = xmlParseChunk(ctxt, c->mem->ptr + c->offset, weHave, 0))) {
				log_error_write(srv, __FILE__, __LINE__, "sodd", "xmlParseChunk failed at:", cq->bytes_out, weHave, err);
			}

			chunkqueue_mark_written(cq, weHave);

			break;
		}
	}

	switch ((err = xmlParseChunk(ctxt, 0, 0, 1))) {
	case XML_ERR_DOCUMENT_END:
	case XML_ERR_OK:
		break;
	default:
		log_error_write(srv, __FILE__, __LINE__, "sd", "xmlParseChunk failed at final packet:", err);
		break;
	}

	xml = ctxt->myDoc;
	res = ctxt->wellFormed;
	xmlFreeParserCtxt(ctxt);

	if (res == 0) {
		xmlFreeDoc(xml);
	} else {
		*ret_xml = xml;
	}

	return res;
}
#endif

#ifdef USE_LOCKS
static int webdav_lockdiscovery(server *srv, connection *con,
		buffer *locktoken, const char *lockscope, const char *locktype, int depth) {

	buffer *b = buffer_init();

	response_header_overwrite(srv, con, CONST_STR_LEN("Lock-Token"), CONST_BUF_LEN(locktoken));

	response_header_overwrite(srv, con,
		CONST_STR_LEN("Content-Type"),
		CONST_STR_LEN("text/xml; charset=\"utf-8\""));

	buffer_copy_string_len(b, CONST_STR_LEN("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));

	buffer_append_string_len(b,CONST_STR_LEN("<D:prop xmlns:D=\"DAV:\" xmlns:ns0=\"urn:uuid:c2f41010-65b3-11d1-a29f-00aa00c14882/\">\n"));
	buffer_append_string_len(b,CONST_STR_LEN("<D:lockdiscovery>\n"));
	webdav_activelock(b, locktoken, lockscope, locktype, depth, 600);
	buffer_append_string_len(b,CONST_STR_LEN("</D:lockdiscovery>\n"));
	buffer_append_string_len(b,CONST_STR_LEN("</D:prop>\n"));

	chunkqueue_append_buffer(con->write_queue, b);
	buffer_free(b);

	return 0;
}
#endif

/**
 * check if resource is having the right locks to access to resource
 *
 *
 *
 */
static int webdav_has_lock(server *srv, connection *con, handler_ctx *hctx, buffer *uri) {
	int has_lock = 1;

#ifdef USE_LOCKS
	data_string *ds;
	UNUSED(srv);

	/**
	 * This implementation is more fake than real
	 * we need a parser for the If: header to really handle the full scope
	 *
	 * X-Litmus: locks: 11 (owner_modify)
	 * If: <http://127.0.0.1:1025/dav/litmus/lockme> (<opaquelocktoken:2165478d-0611-49c4-be92-e790d68a38f1>)
	 * - a tagged check:
	 *   if http://127.0.0.1:1025/dav/litmus/lockme is locked with
	 *   opaquelocktoken:2165478d-0611-49c4-be92-e790d68a38f1, go on
	 *
	 * X-Litmus: locks: 16 (fail_cond_put)
	 * If: (<DAV:no-lock> ["-1622396671"])
	 * - untagged:
	 *   go on if the resource has the etag [...] and the lock
	 */
	if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "If"))) {
		/* Ooh, ooh. A if tag, now the fun begins.
		 *
		 * this can only work with a real parser
		 **/
	} else {
		/* we didn't provided a lock-token -> */
		/* if the resource is locked -> 423 */

		sqlite3_stmt *stmt = hctx->conf.stmt_read_lock_by_uri;

		sqlite3_reset(stmt);

		sqlite3_bind_text(stmt, 1,
			CONST_BUF_LEN(uri),
			SQLITE_TRANSIENT);

		while (SQLITE_ROW == sqlite3_step(stmt)) {
			has_lock = 0;
		}
	}
#else
	UNUSED(srv);
	UNUSED(con);
	UNUSED(hctx);
	UNUSED(uri);
#endif

	return has_lock;
}


SUBREQUEST_FUNC(mod_webdav_subrequest_handler_huge) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];
	buffer *b;
	DIR *dir;
	data_string *ds;
	int depth = -1; /* (Depth: infinity) */
	struct stat st;
	buffer *prop_200;
	buffer *prop_404;
	webdav_properties *req_props;
	stat_cache_entry *sce = NULL;

	UNUSED(srv);

	if (NULL == hctx) return HANDLER_GO_ON;
	if (!hctx->conf.enabled) return HANDLER_GO_ON;
	/* physical path is setup */
	if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;

	/* PROPFIND need them */
	if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "Depth")) && 1 == buffer_string_length(ds->value)) {
		if ('0' == *ds->value->ptr) {
			depth = 0;
		} else if ('1' == *ds->value->ptr) {
			depth = 1;
		}
	} /* else treat as Depth: infinity */

	switch (con->request.http_method) {
	case HTTP_METHOD_PROPFIND:
		/* they want to know the properties of the directory */
		req_props = NULL;

		/* is there a content-body ? */

		switch (stat_cache_get_entry(srv, con, con->physical.path, &sce)) {
		case HANDLER_ERROR:
			if (errno == ENOENT) {
				con->http_status = 404;
				return HANDLER_FINISHED;
			}
			break;
		default:
			break;
		}

		if (S_ISDIR(sce->st.st_mode) && con->physical.path->ptr[buffer_string_length(con->physical.path)-1] != '/') {
			http_response_redirect_to_directory(srv, con);
			return HANDLER_FINISHED;
		}

#ifdef USE_PROPPATCH
		/* any special requests or just allprop ? */
		if (con->request.content_length) {
			xmlDocPtr xml;

			if (con->state == CON_STATE_READ_POST) {
				handler_t r = connection_handle_read_post_state(srv, con);
				if (r != HANDLER_GO_ON) return r;
			}

			if (1 == webdav_parse_chunkqueue(srv, con, hctx, con->request_content_queue, &xml)) {
				xmlNode *rootnode = xmlDocGetRootElement(xml);

				force_assert(rootnode);

				if (0 == xmlStrcmp(rootnode->name, BAD_CAST "propfind")) {
					xmlNode *cmd;

					req_props = calloc(1, sizeof(*req_props));

					for (cmd = rootnode->children; cmd; cmd = cmd->next) {

						if (0 == xmlStrcmp(cmd->name, BAD_CAST "prop")) {
							/* get prop by name */
							xmlNode *prop;

							for (prop = cmd->children; prop; prop = prop->next) {
								if (prop->type == XML_TEXT_NODE) continue; /* ignore WS */

								if (prop->ns &&
								  (0 == xmlStrcmp(prop->ns->href, BAD_CAST "")) &&
								    (0 != xmlStrcmp(prop->ns->prefix, BAD_CAST ""))) {
									size_t i;
									log_error_write(srv, __FILE__, __LINE__, "ss",
											"no name space for:",
											prop->name);

									xmlFreeDoc(xml);

									for (i = 0; i < req_props->used; i++) {
										free(req_props->ptr[i]->ns);
										free(req_props->ptr[i]->prop);
										free(req_props->ptr[i]);
									}
									free(req_props->ptr);
									free(req_props);

									con->http_status = 400;
									return HANDLER_FINISHED;
								}

								/* add property to requested list */
								if (req_props->size == 0) {
									req_props->size = 16;
									req_props->ptr = malloc(sizeof(*(req_props->ptr)) * req_props->size);
								} else if (req_props->used == req_props->size) {
									req_props->size += 16;
									req_props->ptr = realloc(req_props->ptr, sizeof(*(req_props->ptr)) * req_props->size);
								}

								req_props->ptr[req_props->used] = malloc(sizeof(webdav_property));
								req_props->ptr[req_props->used]->ns = (char *)xmlStrdup(prop->ns ? prop->ns->href : (xmlChar *)"");
								req_props->ptr[req_props->used]->prop = (char *)xmlStrdup(prop->name);
								req_props->used++;
							}
						} else if (0 == xmlStrcmp(cmd->name, BAD_CAST "propname")) {
							sqlite3_stmt *stmt = p->conf.stmt_select_propnames;

							if (stmt) {
								/* get all property names (EMPTY) */
								sqlite3_reset(stmt);
								/* bind the values to the insert */

								sqlite3_bind_text(stmt, 1,
									CONST_BUF_LEN(con->uri.path),
									SQLITE_TRANSIENT);

								if (SQLITE_DONE != sqlite3_step(stmt)) {
								}
							}
						} else if (0 == xmlStrcmp(cmd->name, BAD_CAST "allprop")) {
							/* get all properties (EMPTY) */
						}
					}
				}

				xmlFreeDoc(xml);
			} else {
				con->http_status = 400;
				return HANDLER_FINISHED;
			}
		}
#endif
		con->http_status = 207;

		response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/xml; charset=\"utf-8\""));

		b = buffer_init();

		buffer_copy_string_len(b, CONST_STR_LEN("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));

		buffer_append_string_len(b,CONST_STR_LEN("<D:multistatus xmlns:D=\"DAV:\" xmlns:ns0=\"urn:uuid:c2f41010-65b3-11d1-a29f-00aa00c14882/\">\n"));

		/* allprop */

		prop_200 = buffer_init();
		prop_404 = buffer_init();

		{
			/* Depth: 0  or  Depth: 1 */
			webdav_get_props(srv, con, hctx, &(con->physical), req_props, prop_200, prop_404);

			buffer_append_string_len(b,CONST_STR_LEN("<D:response>\n"));
			buffer_append_string_len(b,CONST_STR_LEN("<D:href>"));
			buffer_append_string_buffer(b, con->uri.scheme);
			buffer_append_string_len(b,CONST_STR_LEN("://"));
			buffer_append_string_buffer(b, con->uri.authority);
			buffer_append_string_encoded(b, CONST_BUF_LEN(con->uri.path), ENCODING_REL_URI);
			buffer_append_string_len(b,CONST_STR_LEN("</D:href>\n"));

			if (!buffer_string_is_empty(prop_200)) {
				buffer_append_string_len(b,CONST_STR_LEN("<D:propstat>\n"));
				buffer_append_string_len(b,CONST_STR_LEN("<D:prop>\n"));

				buffer_append_string_buffer(b, prop_200);

				buffer_append_string_len(b,CONST_STR_LEN("</D:prop>\n"));

				buffer_append_string_len(b,CONST_STR_LEN("<D:status>HTTP/1.1 200 OK</D:status>\n"));

				buffer_append_string_len(b,CONST_STR_LEN("</D:propstat>\n"));
			}
			if (!buffer_string_is_empty(prop_404)) {
				buffer_append_string_len(b,CONST_STR_LEN("<D:propstat>\n"));
				buffer_append_string_len(b,CONST_STR_LEN("<D:prop>\n"));

				buffer_append_string_buffer(b, prop_404);

				buffer_append_string_len(b,CONST_STR_LEN("</D:prop>\n"));

				buffer_append_string_len(b,CONST_STR_LEN("<D:status>HTTP/1.1 404 Not Found</D:status>\n"));

				buffer_append_string_len(b,CONST_STR_LEN("</D:propstat>\n"));
			}

			buffer_append_string_len(b,CONST_STR_LEN("</D:response>\n"));
		}

		if (depth == 1) {

			if (NULL != (dir = opendir(con->physical.path->ptr))) {
				struct dirent *de;
				physical d;
				physical *dst = &(con->physical);

				d.path = buffer_init();
				d.rel_path = buffer_init();

				while(NULL != (de = readdir(dir))) {
					if (de->d_name[0] == '.' && (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0'))) {
						continue;
						/* ignore the parent and target dir */
					}

					buffer_copy_buffer(d.path, dst->path);
					buffer_append_slash(d.path);

					buffer_copy_buffer(d.rel_path, dst->rel_path);
					buffer_append_slash(d.rel_path);

					buffer_append_string(d.path, de->d_name);
					buffer_append_string(d.rel_path, de->d_name);

					buffer_reset(prop_200);
					buffer_reset(prop_404);

					webdav_get_props(srv, con, hctx, &d, req_props, prop_200, prop_404);

					buffer_append_string_len(b,CONST_STR_LEN("<D:response>\n"));
					buffer_append_string_len(b,CONST_STR_LEN("<D:href>"));
					buffer_append_string_buffer(b, con->uri.scheme);
					buffer_append_string_len(b,CONST_STR_LEN("://"));
					buffer_append_string_buffer(b, con->uri.authority);
					buffer_append_string_encoded(b, CONST_BUF_LEN(d.rel_path), ENCODING_REL_URI);
					if (0 == stat(d.path->ptr, &st) && S_ISDIR(st.st_mode)) {
						/* Append a '/' on subdirectories */
						buffer_append_string_len(b,CONST_STR_LEN("/"));
					}
					buffer_append_string_len(b,CONST_STR_LEN("</D:href>\n"));

					if (!buffer_string_is_empty(prop_200)) {
						buffer_append_string_len(b,CONST_STR_LEN("<D:propstat>\n"));
						buffer_append_string_len(b,CONST_STR_LEN("<D:prop>\n"));

						buffer_append_string_buffer(b, prop_200);

						buffer_append_string_len(b,CONST_STR_LEN("</D:prop>\n"));

						buffer_append_string_len(b,CONST_STR_LEN("<D:status>HTTP/1.1 200 OK</D:status>\n"));

						buffer_append_string_len(b,CONST_STR_LEN("</D:propstat>\n"));
					}
					if (!buffer_string_is_empty(prop_404)) {
						buffer_append_string_len(b,CONST_STR_LEN("<D:propstat>\n"));
						buffer_append_string_len(b,CONST_STR_LEN("<D:prop>\n"));

						buffer_append_string_buffer(b, prop_404);

						buffer_append_string_len(b,CONST_STR_LEN("</D:prop>\n"));

						buffer_append_string_len(b,CONST_STR_LEN("<D:status>HTTP/1.1 404 Not Found</D:status>\n"));

						buffer_append_string_len(b,CONST_STR_LEN("</D:propstat>\n"));
					}

					buffer_append_string_len(b,CONST_STR_LEN("</D:response>\n"));
				}
				closedir(dir);
				buffer_free(d.path);
				buffer_free(d.rel_path);
			}

		}

		if (req_props) {
			size_t i;
			for (i = 0; i < req_props->used; i++) {
				free(req_props->ptr[i]->ns);
				free(req_props->ptr[i]->prop);
				free(req_props->ptr[i]);
			}
			free(req_props->ptr);
			free(req_props);
		}

		buffer_free(prop_200);
		buffer_free(prop_404);

		buffer_append_string_len(b,CONST_STR_LEN("</D:multistatus>\n"));

		if (p->conf.log_xml) {
			log_error_write(srv, __FILE__, __LINE__, "sb", "XML-response-body:", b);
		}

		chunkqueue_append_buffer(con->write_queue, b);
		buffer_free(b);

		con->file_finished = 1;

		return HANDLER_FINISHED;
	case HTTP_METHOD_MKCOL:
		if (p->conf.is_readonly) {
			con->http_status = 403;
			return HANDLER_FINISHED;
		}

		if (con->request.content_length != 0) {
			/* we don't support MKCOL with a body */
			con->http_status = 415;

			return HANDLER_FINISHED;
		}

		/* let's create the directory */

		if (-1 == mkdir(con->physical.path->ptr, WEBDAV_DIR_MODE)) {
			switch(errno) {
			case EPERM:
				con->http_status = 403;
				break;
			case ENOENT:
			case ENOTDIR:
				con->http_status = 409;
				break;
			case EEXIST:
			default:
				con->http_status = 405; /* not allowed */
				break;
			}
		} else {
			con->http_status = 201;
			con->file_finished = 1;
		}

		return HANDLER_FINISHED;
	case HTTP_METHOD_DELETE:
		if (p->conf.is_readonly) {
			con->http_status = 403;
			return HANDLER_FINISHED;
		}

		/* does the client have a lock for this connection ? */
		if (!webdav_has_lock(srv, con, hctx, con->uri.path)) {
			con->http_status = 423;
			return HANDLER_FINISHED;
		}

		/* stat and unlink afterwards */
		if (-1 == stat(con->physical.path->ptr, &st)) {
			/* don't about it yet, unlink will fail too */
			switch(errno) {
			case ENOENT:
				 con->http_status = 404;
				 break;
			default:
				 con->http_status = 403;
				 break;
			}
		} else if (S_ISDIR(st.st_mode)) {
			buffer *multi_status_resp;

			if (con->physical.path->ptr[buffer_string_length(con->physical.path)-1] != '/') {
				http_response_redirect_to_directory(srv, con);
				return HANDLER_FINISHED;
			}

			multi_status_resp = buffer_init();

			if (webdav_delete_dir(srv, con, hctx, &(con->physical), multi_status_resp)) {
				/* we got an error somewhere in between, build a 207 */
				response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/xml; charset=\"utf-8\""));

				b = buffer_init();

				buffer_copy_string_len(b, CONST_STR_LEN("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));

				buffer_append_string_len(b,CONST_STR_LEN("<D:multistatus xmlns:D=\"DAV:\">\n"));

				buffer_append_string_buffer(b, multi_status_resp);

				buffer_append_string_len(b,CONST_STR_LEN("</D:multistatus>\n"));

				if (p->conf.log_xml) {
					log_error_write(srv, __FILE__, __LINE__, "sb", "XML-response-body:", b);
				}

				chunkqueue_append_buffer(con->write_queue, b);
				buffer_free(b);

				con->http_status = 207;
				con->file_finished = 1;
			} else {
				/* everything went fine, remove the directory */

				if (-1 == rmdir(con->physical.path->ptr)) {
					switch(errno) {
					case EPERM:
						con->http_status = 403;
						break;
					case ENOENT:
						con->http_status = 404;
						break;
					default:
						con->http_status = 501;
						break;
					}
				} else {
					con->http_status = 204;
				}
			}

			buffer_free(multi_status_resp);
		} else if (-1 == unlink(con->physical.path->ptr)) {
			switch(errno) {
			case EPERM:
				con->http_status = 403;
				break;
			case ENOENT:
				con->http_status = 404;
				break;
			default:
				con->http_status = 501;
				break;
			}
		} else {
			con->http_status = 204;
		}
		return HANDLER_FINISHED;
	case HTTP_METHOD_PUT: {
		int fd;
		chunkqueue *cq = con->request_content_queue;
		chunk *c;
		data_string *ds_range;

		if (p->conf.is_readonly) {
			con->http_status = 403;
			return HANDLER_FINISHED;
		}

		/* is a exclusive lock set on the source */
		/* (check for lock once before potentially reading large input) */
		if (0 == cq->bytes_in && !webdav_has_lock(srv, con, hctx, con->uri.path)) {
			con->http_status = 423;
			return HANDLER_FINISHED;
		}

		if (con->state == CON_STATE_READ_POST) {
			handler_t r = connection_handle_read_post_state(srv, con);
			if (r != HANDLER_GO_ON) return r;
		}

		/* RFC2616 Section 9.6 PUT requires us to send 501 on all Content-* we don't support
		 * - most important Content-Range
		 *
		 *
		 * Example: Content-Range: bytes 100-1037/1038 */

		if (NULL != (ds_range = (data_string *)array_get_element(con->request.headers, "Content-Range"))) {
			const char *num = ds_range->value->ptr;
			off_t offset;
			char *err = NULL;

			if (0 != strncmp(num, "bytes ", 6)) {
				con->http_status = 501; /* not implemented */

				return HANDLER_FINISHED;
			}

			/* we only support <num>- ... */

			num += 6;

			/* skip WS */
			while (*num == ' ' || *num == '\t') num++;

			if (*num == '\0') {
				con->http_status = 501; /* not implemented */

				return HANDLER_FINISHED;
			}

			offset = strtoll(num, &err, 10);

			if (*err != '-' || offset < 0) {
				con->http_status = 501; /* not implemented */

				return HANDLER_FINISHED;
			}

			if (-1 == (fd = open(con->physical.path->ptr, O_WRONLY, WEBDAV_FILE_MODE))) {
				switch (errno) {
				case ENOENT:
					con->http_status = 404; /* not found */
					break;
				default:
					con->http_status = 403; /* not found */
					break;
				}
				return HANDLER_FINISHED;
			}

			if (-1 == lseek(fd, offset, SEEK_SET)) {
				con->http_status = 501; /* not implemented */

				close(fd);

				return HANDLER_FINISHED;
			}
			con->http_status = 200; /* modified */
		} else {
			/* take what we have in the request-body and write it to a file */

			/* if the file doesn't exist, create it */
			if (-1 == (fd = open(con->physical.path->ptr, O_WRONLY|O_TRUNC, WEBDAV_FILE_MODE))) {
				if (errno != ENOENT ||
				    -1 == (fd = open(con->physical.path->ptr, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, WEBDAV_FILE_MODE))) {
					/* we can't open the file */
					con->http_status = 403;

					return HANDLER_FINISHED;
				} else {
					con->http_status = 201; /* created */
				}
			} else {
				con->http_status = 200; /* modified */
			}
		}

		con->file_finished = 1;

		for (c = cq->first; c; c = cq->first) {
			int r = 0;
			int mapped;
			void *data;
			size_t dlen;

			/* copy all chunks */
			switch(c->type) {
			case FILE_CHUNK:

				mapped = (c->file.mmap.start != MAP_FAILED);
				dlen = c->file.length - c->offset;
				if (mapped) {
					data = c->file.mmap.start + c->offset;
				} else {
					if (-1 == c->file.fd &&  /* open the file if not already open */
					    -1 == (c->file.fd = open(c->file.name->ptr, O_RDONLY))) {
						log_error_write(srv, __FILE__, __LINE__, "ss", "open failed: ", strerror(errno));
						close(fd);
						return HANDLER_ERROR;
					}

					if (MAP_FAILED != (c->file.mmap.start = mmap(NULL, c->file.length, PROT_READ, MAP_PRIVATE, c->file.fd, 0))) {
						/* chunk_reset() or chunk_free() will cleanup for us */
						c->file.mmap.length = c->file.length;
						data = c->file.mmap.start + c->offset;
						mapped = 1;
					} else {
						ssize_t rd;
						if (dlen > 65536) dlen = 65536;
						data = malloc(dlen);
						force_assert(data);
						if (-1 == lseek(c->file.fd, c->file.start + c->offset, SEEK_SET)
						    || 0 > (rd = read(c->file.fd, data, dlen))) {
							log_error_write(srv, __FILE__, __LINE__, "ssbd", "lseek/read failed: ",
									strerror(errno), c->file.name, c->file.fd);
							free(data);
							close(fd);
							return HANDLER_ERROR;
						}
						dlen = (size_t)rd;
					}

				}

				if ((r = write(fd, data, dlen)) < 0) {
					switch(errno) {
					case ENOSPC:
						con->http_status = 507;

						break;
					default:
						con->http_status = 403;
						break;
					}
				}

				if (!mapped) free(data);
				break;
			case MEM_CHUNK:
				if ((r = write(fd, c->mem->ptr + c->offset, buffer_string_length(c->mem) - c->offset)) < 0) {
					switch(errno) {
					case ENOSPC:
						con->http_status = 507;

						break;
					default:
						con->http_status = 403;
						break;
					}
				}
				break;
			}

			if (r > 0) {
				chunkqueue_mark_written(cq, r);
			} else {
				break;
			}
		}
		if (0 != close(fd)) {
			log_error_write(srv, __FILE__, __LINE__, "sbss",
					"close ", con->physical.path, "failed: ", strerror(errno));
			return HANDLER_ERROR;
		}

		return HANDLER_FINISHED;
	}
	case HTTP_METHOD_MOVE:
	case HTTP_METHOD_COPY: {
		buffer *destination = NULL;
		char *sep, *sep2, *start;
		int overwrite = 1;

		if (p->conf.is_readonly) {
			con->http_status = 403;
			return HANDLER_FINISHED;
		}

		/* is a exclusive lock set on the source */
		if (con->request.http_method == HTTP_METHOD_MOVE) {
			if (!webdav_has_lock(srv, con, hctx, con->uri.path)) {
				con->http_status = 423;
				return HANDLER_FINISHED;
			}
		}

		if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "Destination"))) {
			destination = ds->value;
		} else {
			con->http_status = 400;
			return HANDLER_FINISHED;
		}

		if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "Overwrite"))) {
			if (buffer_string_length(ds->value) != 1 ||
			    (ds->value->ptr[0] != 'F' &&
			     ds->value->ptr[0] != 'T') )  {
				con->http_status = 400;
				return HANDLER_FINISHED;
			}
			overwrite = (ds->value->ptr[0] == 'F' ? 0 : 1);
		}
		/* let's parse the Destination
		 *
		 * http://127.0.0.1:1025/dav/litmus/copydest
		 *
		 * - host has to be the same as the Host: header we got
		 * - we have to stay inside the document root
		 * - the query string is thrown away
		 *  */

		buffer_reset(p->uri.scheme);
		buffer_reset(p->uri.path_raw);
		buffer_reset(p->uri.authority);

		start = destination->ptr;

		if (NULL == (sep = strstr(start, "://"))) {
			con->http_status = 400;
			return HANDLER_FINISHED;
		}
		buffer_copy_string_len(p->uri.scheme, start, sep - start);

		start = sep + 3;

		if (NULL == (sep = strchr(start, '/'))) {
			con->http_status = 400;
			return HANDLER_FINISHED;
		}
		if (NULL != (sep2 = memchr(start, '@', sep - start))) {
			/* skip login information */
			start = sep2 + 1;
		}
		buffer_copy_string_len(p->uri.authority, start, sep - start);

		start = sep + 1;

		if (NULL == (sep = strchr(start, '?'))) {
			/* no query string, good */
			buffer_copy_string(p->uri.path_raw, start);
		} else {
			buffer_copy_string_len(p->uri.path_raw, start, sep - start);
		}

		if (!buffer_is_equal(p->uri.authority, con->uri.authority)) {
			/* not the same host */
			con->http_status = 502;
			return HANDLER_FINISHED;
		}

		buffer_copy_buffer(p->tmp_buf, p->uri.path_raw);
		buffer_urldecode_path(p->tmp_buf);
		buffer_path_simplify(p->uri.path, p->tmp_buf);

		/* we now have a URI which is clean. transform it into a physical path */
		buffer_copy_buffer(p->physical.doc_root, con->physical.doc_root);
		buffer_copy_buffer(p->physical.rel_path, p->uri.path);

		if (con->conf.force_lowercase_filenames) {
			buffer_to_lower(p->physical.rel_path);
		}

		/* Destination physical path
		 * src con->physical.path might have been remapped with mod_alias.
		 *   (but mod_alias does not modify con->physical.rel_path)
		 * Find matching prefix to support use of mod_alias to remap webdav root.
		 * Aliasing of paths underneath the webdav root might not work.
		 * Likewise, mod_rewrite URL rewriting might thwart this comparison.
		 * Use mod_redirect instead of mod_alias to remap paths *under* webdav root.
		 * Use mod_redirect instead of mod_rewrite on *any* parts of path to webdav.
		 * (Related, use mod_auth to protect webdav root, but avoid attempting to
		 *  use mod_auth on paths underneath webdav root, as Destination is not
		 *  validated with mod_auth)
		 *
		 * tl;dr: webdav paths and webdav properties are managed by mod_webdav,
		 *        so do not modify paths externally or else undefined behavior
		 *        or corruption may occur
		 */
		{
			/* find matching URI prefix
			 * check if remaining con->physical.rel_path matches suffix
			 *   of con->physical.basedir so that we can use it to
			 *   remap Destination physical path */
			size_t i, remain;
			sep = con->uri.path->ptr;
			sep2 = p->uri.path->ptr;
			for (i = 0; sep[i] && sep[i] == sep2[i]; ++i) ;
			if (sep[i] == '\0' && (sep2[i] == '\0' || sep2[i] == '/' || (i > 0 && sep[i-1] == '/'))) {
				/* src and dst URI match or dst is nested inside src; invalid COPY or MOVE */
				con->http_status = 403;
				return HANDLER_FINISHED;
			}
			while (i != 0 && sep[--i] != '/') ; /* find matching directory path */
			remain = buffer_string_length(con->uri.path) - i;
			if (!con->conf.force_lowercase_filenames
			    ? buffer_is_equal_right_len(con->physical.path, con->physical.rel_path, remain)
			    :(buffer_string_length(con->physical.path) >= remain
			      && 0 == strncasecmp(con->physical.path->ptr+buffer_string_length(con->physical.path)-remain, con->physical.rel_path->ptr+i, remain))) {
				/* (at this point, p->physical.rel_path is identical to (or lowercased version of) p->uri.path) */
				buffer_copy_string_len(p->physical.path, con->physical.path->ptr, buffer_string_length(con->physical.path)-remain);
				buffer_append_string_len(p->physical.path, p->physical.rel_path->ptr+i, buffer_string_length(p->physical.rel_path)-i);

				buffer_copy_buffer(p->physical.basedir, con->physical.basedir);
				buffer_append_slash(p->physical.basedir);
			} else {
				/* unable to perform physical path remap here;
				 * assume doc_root/rel_path and no remapping */
				buffer_copy_buffer(p->physical.path, p->physical.doc_root);
				buffer_append_slash(p->physical.path);
				buffer_copy_buffer(p->physical.basedir, p->physical.path);

				/* don't add a second / */
				if (p->physical.rel_path->ptr[0] == '/') {
					buffer_append_string_len(p->physical.path, p->physical.rel_path->ptr + 1, buffer_string_length(p->physical.rel_path) - 1);
				} else {
					buffer_append_string_buffer(p->physical.path, p->physical.rel_path);
				}
			}
		}

		/* let's see if the source is a directory
		 * if yes, we fail with 501 */

		if (-1 == stat(con->physical.path->ptr, &st)) {
			/* don't about it yet, unlink will fail too */
			switch(errno) {
			case ENOENT:
				 con->http_status = 404;
				 break;
			default:
				 con->http_status = 403;
				 break;
			}
		} else if (S_ISDIR(st.st_mode)) {
			int r;
			int created = 0;
			/* src is a directory */

			if (con->physical.path->ptr[buffer_string_length(con->physical.path)-1] != '/') {
				http_response_redirect_to_directory(srv, con);
				return HANDLER_FINISHED;
			}

			if (-1 == stat(p->physical.path->ptr, &st)) {
				if (-1 == mkdir(p->physical.path->ptr, WEBDAV_DIR_MODE)) {
					con->http_status = 403;
					return HANDLER_FINISHED;
				}
				created = 1;
			} else if (!S_ISDIR(st.st_mode)) {
				if (overwrite == 0) {
					/* copying into a non-dir ? */
					con->http_status = 409;
					return HANDLER_FINISHED;
				} else {
					unlink(p->physical.path->ptr);
					if (-1 == mkdir(p->physical.path->ptr, WEBDAV_DIR_MODE)) {
						con->http_status = 403;
						return HANDLER_FINISHED;
					}
					created = 1;
				}
			}

			/* copy the content of src to dest */
			if (0 != (r = webdav_copy_dir(srv, con, hctx, &(con->physical), &(p->physical), overwrite))) {
				con->http_status = r;
				return HANDLER_FINISHED;
			}
			if (con->request.http_method == HTTP_METHOD_MOVE) {
				b = buffer_init();
				webdav_delete_dir(srv, con, hctx, &(con->physical), b); /* content */
				buffer_free(b);

				rmdir(con->physical.path->ptr);
			}
			con->http_status = created ? 201 : 204;
			con->file_finished = 1;
		} else {
			/* it is just a file, good */
			int r;
			int destdir = 0;

			/* does the client have a lock for this connection ? */
			if (!webdav_has_lock(srv, con, hctx, p->uri.path)) {
				con->http_status = 423;
				return HANDLER_FINISHED;
			}

			/* destination exists */
			if (0 == (r = stat(p->physical.path->ptr, &st))) {
				if (S_ISDIR(st.st_mode)) {
					/* file to dir/
					 * append basename to physical path */
					destdir = 1;

					if (NULL != (sep = strrchr(con->physical.path->ptr, '/'))) {
						buffer_append_string(p->physical.path, sep);
						r = stat(p->physical.path->ptr, &st);
					}
				}
			}

			if (-1 == r) {
				con->http_status = destdir ? 204 : 201; /* we will create a new one */
				con->file_finished = 1;

				switch(errno) {
				case ENOTDIR:
					con->http_status = 409;
					return HANDLER_FINISHED;
				}
			} else if (overwrite == 0) {
				/* destination exists, but overwrite is not set */
				con->http_status = 412;
				return HANDLER_FINISHED;
			} else {
				con->http_status = 204; /* resource already existed */
			}

			if (con->request.http_method == HTTP_METHOD_MOVE) {
				/* try a rename */

				if (0 == rename(con->physical.path->ptr, p->physical.path->ptr)) {
#ifdef USE_PROPPATCH
					sqlite3_stmt *stmt;

					stmt = p->conf.stmt_move_uri;
					if (stmt) {

						sqlite3_reset(stmt);

						/* bind the values to the insert */
						sqlite3_bind_text(stmt, 1,
							CONST_BUF_LEN(p->uri.path),
							SQLITE_TRANSIENT);

						sqlite3_bind_text(stmt, 2,
							CONST_BUF_LEN(con->uri.path),
							SQLITE_TRANSIENT);

						if (SQLITE_DONE != sqlite3_step(stmt)) {
							log_error_write(srv, __FILE__, __LINE__, "ss", "sql-move failed:", sqlite3_errmsg(p->conf.sql));
						}
					}
#endif
					return HANDLER_FINISHED;
				}

				/* rename failed, fall back to COPY + DELETE */
			}

			if (0 != (r = webdav_copy_file(srv, con, hctx, &(con->physical), &(p->physical), overwrite))) {
				con->http_status = r;

				return HANDLER_FINISHED;
			}

			if (con->request.http_method == HTTP_METHOD_MOVE) {
				b = buffer_init();
				webdav_delete_file(srv, con, hctx, &(con->physical), b);
				buffer_free(b);
			}
		}

		return HANDLER_FINISHED;
	}
	case HTTP_METHOD_PROPPATCH:
		if (p->conf.is_readonly) {
			con->http_status = 403;
			return HANDLER_FINISHED;
		}

		if (!webdav_has_lock(srv, con, hctx, con->uri.path)) {
			con->http_status = 423;
			return HANDLER_FINISHED;
		}

		/* check if destination exists */
		if (-1 == stat(con->physical.path->ptr, &st)) {
			switch(errno) {
			case ENOENT:
				con->http_status = 404;
				break;
			}
		}

		if (S_ISDIR(st.st_mode) && con->physical.path->ptr[buffer_string_length(con->physical.path)-1] != '/') {
			http_response_redirect_to_directory(srv, con);
			return HANDLER_FINISHED;
		}

#ifdef USE_PROPPATCH
		if (con->request.content_length) {
			xmlDocPtr xml;

			if (con->state == CON_STATE_READ_POST) {
				handler_t r = connection_handle_read_post_state(srv, con);
				if (r != HANDLER_GO_ON) return r;
			}

			if (1 == webdav_parse_chunkqueue(srv, con, hctx, con->request_content_queue, &xml)) {
				xmlNode *rootnode = xmlDocGetRootElement(xml);

				if (0 == xmlStrcmp(rootnode->name, BAD_CAST "propertyupdate")) {
					xmlNode *cmd;
					char *err = NULL;
					int empty_ns = 0; /* send 400 on a empty namespace attribute */

					/* start response */

					if (SQLITE_OK != sqlite3_exec(p->conf.sql, "BEGIN TRANSACTION", NULL, NULL, &err)) {
						log_error_write(srv, __FILE__, __LINE__, "ss", "can't open transaction:", err);
						sqlite3_free(err);

						goto propmatch_cleanup;
					}

					/* a UPDATE request, we know 'set' and 'remove' */
					for (cmd = rootnode->children; cmd; cmd = cmd->next) {
						xmlNode *props;
						/* either set or remove */

						if ((0 == xmlStrcmp(cmd->name, BAD_CAST "set")) ||
						    (0 == xmlStrcmp(cmd->name, BAD_CAST "remove"))) {

							sqlite3_stmt *stmt;

							stmt = (0 == xmlStrcmp(cmd->name, BAD_CAST "remove")) ?
								p->conf.stmt_delete_prop : p->conf.stmt_update_prop;

							for (props = cmd->children; props; props = props->next) {
								if (0 == xmlStrcmp(props->name, BAD_CAST "prop")) {
									xmlNode *prop;
									char *propval = NULL;
									int r;

									prop = props->children;

									if (prop->ns &&
									    (0 == xmlStrcmp(prop->ns->href, BAD_CAST "")) &&
									    (0 != xmlStrcmp(prop->ns->prefix, BAD_CAST ""))) {
										log_error_write(srv, __FILE__, __LINE__, "ss",
												"no name space for:",
												prop->name);

										empty_ns = 1;
										break;
									}

									sqlite3_reset(stmt);

									/* bind the values to the insert */

									sqlite3_bind_text(stmt, 1,
										CONST_BUF_LEN(con->uri.path),
										SQLITE_TRANSIENT);
									sqlite3_bind_text(stmt, 2,
										(char *)prop->name,
										strlen((char *)prop->name),
										SQLITE_TRANSIENT);
									if (prop->ns) {
										sqlite3_bind_text(stmt, 3,
											(char *)prop->ns->href,
											strlen((char *)prop->ns->href),
											SQLITE_TRANSIENT);
									} else {
										sqlite3_bind_text(stmt, 3,
											"",
											0,
											SQLITE_TRANSIENT);
									}
									if (stmt == p->conf.stmt_update_prop) {
										propval = prop->children
										  ? (char *)xmlNodeListGetString(xml, prop->children, 0)
										  : NULL;

										sqlite3_bind_text(stmt, 4,
											propval ? propval : "",
											propval ? strlen(propval) : 0,
											SQLITE_TRANSIENT);
									}

									if (SQLITE_DONE != (r = sqlite3_step(stmt))) {
										log_error_write(srv, __FILE__, __LINE__, "ss",
												"sql-set failed:", sqlite3_errmsg(p->conf.sql));
									}

									if (propval) xmlFree(propval);
								}
							}
							if (empty_ns) break;
						}
					}

					if (empty_ns) {
						if (SQLITE_OK != sqlite3_exec(p->conf.sql, "ROLLBACK", NULL, NULL, &err)) {
							log_error_write(srv, __FILE__, __LINE__, "ss", "can't rollback transaction:", err);
							sqlite3_free(err);

							goto propmatch_cleanup;
						}

						con->http_status = 400;
					} else {
						if (SQLITE_OK != sqlite3_exec(p->conf.sql, "COMMIT", NULL, NULL, &err)) {
							log_error_write(srv, __FILE__, __LINE__, "ss", "can't commit transaction:", err);
							sqlite3_free(err);

							goto propmatch_cleanup;
						}
						con->http_status = 200;
					}
					con->file_finished = 1;

					xmlFreeDoc(xml);
					return HANDLER_FINISHED;
				}

propmatch_cleanup:

				xmlFreeDoc(xml);
			} else {
				con->http_status = 400;
				return HANDLER_FINISHED;
			}
		}
#endif
		con->http_status = 501;
		return HANDLER_FINISHED;
	case HTTP_METHOD_LOCK:
		/**
		 * a mac wants to write
		 *
		 * LOCK /dav/expire.txt HTTP/1.1\r\n
		 * User-Agent: WebDAVFS/1.3 (01308000) Darwin/8.1.0 (Power Macintosh)\r\n
		 * Accept: * / *\r\n
		 * Depth: 0\r\n
		 * Timeout: Second-600\r\n
		 * Content-Type: text/xml; charset=\"utf-8\"\r\n
		 * Content-Length: 229\r\n
		 * Connection: keep-alive\r\n
		 * Host: 192.168.178.23:1025\r\n
		 * \r\n
		 * <?xml version=\"1.0\" encoding=\"utf-8\"?>\n
		 * <D:lockinfo xmlns:D=\"DAV:\">\n
		 *  <D:lockscope><D:exclusive/></D:lockscope>\n
		 *  <D:locktype><D:write/></D:locktype>\n
		 *  <D:owner>\n
		 *   <D:href>http://www.apple.com/webdav_fs/</D:href>\n
		 *  </D:owner>\n
		 * </D:lockinfo>\n
		 */

		if (depth != 0 && depth != -1) {
			con->http_status = 400;

			return HANDLER_FINISHED;
		}

#ifdef USE_LOCKS
		if (con->request.content_length) {
			xmlDocPtr xml;
			buffer *hdr_if = NULL;
			int created = 0;

			if (con->state == CON_STATE_READ_POST) {
				handler_t r = connection_handle_read_post_state(srv, con);
				if (r != HANDLER_GO_ON) return r;
			}

			if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "If"))) {
				hdr_if = ds->value;
			}

			if (0 != stat(con->physical.path->ptr, &st)) {
				if (errno == ENOENT) {
					int fd = open(con->physical.path->ptr, O_WRONLY|O_CREAT|O_APPEND|O_BINARY|FIFO_NONBLOCK, WEBDAV_FILE_MODE);
					if (fd >= 0) {
						close(fd);
						created = 1;
					} else {
						log_error_write(srv, __FILE__, __LINE__, "sBss",
								"create file", con->physical.path, ":", strerror(errno));
						con->http_status = 403; /* Forbidden */

						return HANDLER_FINISHED;
					}
				}
			} else if (hdr_if == NULL && depth == -1) {
				/* we don't support Depth: Infinity on directories */
				if (S_ISDIR(st.st_mode)) {
					con->http_status = 409; /* Conflict */

					return HANDLER_FINISHED;
				}
			}

			if (1 == webdav_parse_chunkqueue(srv, con, hctx, con->request_content_queue, &xml)) {
				xmlNode *rootnode = xmlDocGetRootElement(xml);

				force_assert(rootnode);

				if (0 == xmlStrcmp(rootnode->name, BAD_CAST "lockinfo")) {
					xmlNode *lockinfo;
					const xmlChar *lockscope = NULL, *locktype = NULL; /* TODO: compiler says unused: *owner = NULL; */

					for (lockinfo = rootnode->children; lockinfo; lockinfo = lockinfo->next) {
						if (0 == xmlStrcmp(lockinfo->name, BAD_CAST "lockscope")) {
							xmlNode *value;
							for (value = lockinfo->children; value; value = value->next) {
								if ((0 == xmlStrcmp(value->name, BAD_CAST "exclusive")) ||
								    (0 == xmlStrcmp(value->name, BAD_CAST "shared"))) {
									lockscope = value->name;
								} else {
									con->http_status = 400;

									xmlFreeDoc(xml);
									return HANDLER_FINISHED;
								}
							}
						} else if (0 == xmlStrcmp(lockinfo->name, BAD_CAST "locktype")) {
							xmlNode *value;
							for (value = lockinfo->children; value; value = value->next) {
								if ((0 == xmlStrcmp(value->name, BAD_CAST "write"))) {
									locktype = value->name;
								} else {
									con->http_status = 400;

									xmlFreeDoc(xml);
									return HANDLER_FINISHED;
								}
							}

						} else if (0 == xmlStrcmp(lockinfo->name, BAD_CAST "owner")) {
						}
					}

					if (lockscope && locktype) {
						sqlite3_stmt *stmt = p->conf.stmt_read_lock_by_uri;

						/* is this resourse already locked ? */

						/* SELECT locktoken, resource, lockscope, locktype, owner, depth, timeout
						 *   FROM locks
						 *  WHERE resource = ? */

						if (stmt) {

							sqlite3_reset(stmt);

							sqlite3_bind_text(stmt, 1,
								CONST_BUF_LEN(p->uri.path),
								SQLITE_TRANSIENT);

							/* it is the PK */
							while (SQLITE_ROW == sqlite3_step(stmt)) {
								/* we found a lock
								 * 1. is it compatible ?
								 * 2. is it ours */
								char *sql_lockscope = (char *)sqlite3_column_text(stmt, 2);

								if (strcmp(sql_lockscope, "exclusive")) {
									con->http_status = 423;
								} else if (0 == xmlStrcmp(lockscope, BAD_CAST "exclusive")) {
									/* resourse is locked with a shared lock
									 * client wants exclusive */
									con->http_status = 423;
								}
							}
							if (con->http_status == 423) {
								xmlFreeDoc(xml);
								return HANDLER_FINISHED;
							}
						}

						stmt = p->conf.stmt_create_lock;
						if (stmt) {
							/* create a lock-token */
							uuid_t id;
							char uuid[37] /* 36 + \0 */;

							uuid_generate(id);
							uuid_unparse(id, uuid);

							buffer_copy_string_len(p->tmp_buf, CONST_STR_LEN("opaquelocktoken:"));
							buffer_append_string(p->tmp_buf, uuid);

							/* "CREATE TABLE locks ("
							 * "  locktoken TEXT NOT NULL,"
							 * "  resource TEXT NOT NULL,"
							 * "  lockscope TEXT NOT NULL,"
							 * "  locktype TEXT NOT NULL,"
							 * "  owner TEXT NOT NULL,"
							 * "  depth INT NOT NULL,"
							 */

							sqlite3_reset(stmt);

							sqlite3_bind_text(stmt, 1,
									CONST_BUF_LEN(p->tmp_buf),
									SQLITE_TRANSIENT);

							sqlite3_bind_text(stmt, 2,
									CONST_BUF_LEN(con->uri.path),
									SQLITE_TRANSIENT);

							sqlite3_bind_text(stmt, 3,
									(const char *)lockscope,
									xmlStrlen(lockscope),
									SQLITE_TRANSIENT);

							sqlite3_bind_text(stmt, 4,
									(const char *)locktype,
									xmlStrlen(locktype),
									SQLITE_TRANSIENT);

							/* owner */
							sqlite3_bind_text(stmt, 5,
									"",
									0,
									SQLITE_TRANSIENT);

							/* depth */
							sqlite3_bind_int(stmt, 6,
									depth);


							if (SQLITE_DONE != sqlite3_step(stmt)) {
								log_error_write(srv, __FILE__, __LINE__, "ss",
										"create lock:", sqlite3_errmsg(p->conf.sql));
							}

							/* looks like we survived */
							webdav_lockdiscovery(srv, con, p->tmp_buf, (const char *)lockscope, (const char *)locktype, depth);

							con->http_status = created ? 201 : 200;
							con->file_finished = 1;
						}
					}
				}

				xmlFreeDoc(xml);
				return HANDLER_FINISHED;
			} else {
				con->http_status = 400;
				return HANDLER_FINISHED;
			}
		} else {

			if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "If"))) {
				buffer *locktoken = ds->value;
				sqlite3_stmt *stmt = p->conf.stmt_refresh_lock;

				/* remove the < > around the token */
				if (buffer_string_length(locktoken) < 5) {
					con->http_status = 400;

					return HANDLER_FINISHED;
				}

				buffer_copy_string_len(p->tmp_buf, locktoken->ptr + 2, buffer_string_length(locktoken) - 4);

				sqlite3_reset(stmt);

				sqlite3_bind_text(stmt, 1,
					CONST_BUF_LEN(p->tmp_buf),
					SQLITE_TRANSIENT);

				if (SQLITE_DONE != sqlite3_step(stmt)) {
					log_error_write(srv, __FILE__, __LINE__, "ss",
						"refresh lock:", sqlite3_errmsg(p->conf.sql));
				}

				webdav_lockdiscovery(srv, con, p->tmp_buf, "exclusive", "write", 0);

				con->http_status = 200;
				con->file_finished = 1;
				return HANDLER_FINISHED;
			} else {
				/* we need a lock-token to refresh */
				con->http_status = 400;

				return HANDLER_FINISHED;
			}
		}
		break;
#else
		con->http_status = 501;
		return HANDLER_FINISHED;
#endif
	case HTTP_METHOD_UNLOCK:
#ifdef USE_LOCKS
		if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "Lock-Token"))) {
			buffer *locktoken = ds->value;
			sqlite3_stmt *stmt = p->conf.stmt_remove_lock;

			/* remove the < > around the token */
			if (buffer_string_length(locktoken) < 3) {
				con->http_status = 400;

				return HANDLER_FINISHED;
			}

			/**
			 * FIXME:
			 *
			 * if the resourse is locked:
			 * - by us: unlock
			 * - by someone else: 401
			 * if the resource is not locked:
			 * - 412
			 *  */

			buffer_copy_string_len(p->tmp_buf, locktoken->ptr + 1, buffer_string_length(locktoken) - 2);

			sqlite3_reset(stmt);

			sqlite3_bind_text(stmt, 1,
				CONST_BUF_LEN(p->tmp_buf),
				SQLITE_TRANSIENT);

			if (SQLITE_DONE != sqlite3_step(stmt)) {
				log_error_write(srv, __FILE__, __LINE__, "ss",
					"remove lock:", sqlite3_errmsg(p->conf.sql));
			}

			if (0 == sqlite3_changes(p->conf.sql)) {
				con->http_status = 401;
			} else {
				con->http_status = 204;
			}
			return HANDLER_FINISHED;
		} else {
			/* we need a lock-token to unlock */
			con->http_status = 400;

			return HANDLER_FINISHED;
		}
		break;
#else
		con->http_status = 501;
		return HANDLER_FINISHED;
#endif
	default:
		break;
	}

	/* not found */
	return HANDLER_GO_ON;
}


SUBREQUEST_FUNC(mod_webdav_subrequest_handler) {
	handler_t r;
	plugin_data *p = p_d;
	if (con->mode != p->id) return HANDLER_GO_ON;

	r = mod_webdav_subrequest_handler_huge(srv, con, p_d);
	if (con->http_status >= 400) con->mode = DIRECT;
	return r;
}


PHYSICALPATH_FUNC(mod_webdav_physical_handler) {
	plugin_data *p = p_d;
	if (!p->conf.enabled) return HANDLER_GO_ON;

	/* physical path is setup */
	if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;

	UNUSED(srv);

	switch (con->request.http_method) {
	case HTTP_METHOD_PROPFIND:
	case HTTP_METHOD_PROPPATCH:
	case HTTP_METHOD_PUT:
	case HTTP_METHOD_COPY:
	case HTTP_METHOD_MOVE:
	case HTTP_METHOD_MKCOL:
	case HTTP_METHOD_DELETE:
	case HTTP_METHOD_LOCK:
	case HTTP_METHOD_UNLOCK: {
		handler_ctx *hctx = calloc(1, sizeof(*hctx));
		memcpy(&hctx->conf, &p->conf, sizeof(plugin_config));
		con->plugin_ctx[p->id] = hctx;
		con->conf.stream_request_body = 0;
		con->mode = p->id;
		break;
	}
	default:
		break;
	}

	return HANDLER_GO_ON;
}

static handler_t mod_webdav_connection_reset(server *srv, connection *con, void *p_d) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];
	if (hctx) {
		free(hctx);
		con->plugin_ctx[p->id] = NULL;
	}

	UNUSED(srv);
	return HANDLER_GO_ON;
}


/* this function is called at dlopen() time and inits the callbacks */

int mod_webdav_plugin_init(plugin *p);
int mod_webdav_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("webdav");

	p->init        = mod_webdav_init;
	p->handle_uri_clean  = mod_webdav_uri_handler;
	p->handle_physical   = mod_webdav_physical_handler;
	p->handle_subrequest = mod_webdav_subrequest_handler;
	p->connection_reset  = mod_webdav_connection_reset;
	p->set_defaults  = mod_webdav_set_defaults;
	p->cleanup     = mod_webdav_free;

	p->data        = NULL;

	return 0;
}
