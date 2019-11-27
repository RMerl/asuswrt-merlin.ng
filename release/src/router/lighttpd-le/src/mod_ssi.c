#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#include "response.h"

#include "mod_ssi.h"

#include "inet_ntop_cache.h"

#include "sys-socket.h"

#include <sys/types.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

#ifdef HAVE_FORK
# include <sys/wait.h>
#endif

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#include "etag.h"

static handler_ctx * handler_ctx_init(plugin_data *p) {
	handler_ctx *hctx = calloc(1, sizeof(*hctx));
	force_assert(hctx);
	hctx->timefmt = p->timefmt;
	hctx->stat_fn = p->stat_fn;
	hctx->ssi_vars = p->ssi_vars;
	hctx->ssi_cgi_env = p->ssi_cgi_env;
	memcpy(&hctx->conf, &p->conf, sizeof(plugin_config));
	return hctx;
}

static void handler_ctx_free(handler_ctx *hctx) {
	free(hctx);
}

/* The newest modified time of included files for include statement */
static volatile time_t include_file_last_mtime = 0;

/* init the plugin data */
INIT_FUNC(mod_ssi_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	p->timefmt = buffer_init();
	p->stat_fn = buffer_init();

	p->ssi_vars = array_init();
	p->ssi_cgi_env = array_init();

	return p;
}

/* detroy the plugin data */
FREE_FUNC(mod_ssi_free) {
	plugin_data *p = p_d;
	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (NULL == s) continue;

			array_free(s->ssi_extension);
			buffer_free(s->content_type);

			free(s);
		}
		free(p->config_storage);
	}

	array_free(p->ssi_vars);
	array_free(p->ssi_cgi_env);
	buffer_free(p->timefmt);
	buffer_free(p->stat_fn);

	free(p);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_ssi_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "ssi.extension",              NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ "ssi.content-type",           NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },      /* 1 */
		{ "ssi.conditional-requests",   NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },     /* 2 */
		{ "ssi.exec",                   NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },     /* 3 */
		{ "ssi.recursion-max",          NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },     /* 4 */
		{ NULL,                         NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->ssi_extension  = array_init();
		s->content_type = buffer_init();
		s->conditional_requests = 0;
		s->ssi_exec = 1;
		s->ssi_recursion_max = 0;

		cv[0].destination = s->ssi_extension;
		cv[1].destination = s->content_type;
		cv[2].destination = &(s->conditional_requests);
		cv[3].destination = &(s->ssi_exec);
		cv[4].destination = &(s->ssi_recursion_max);

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}
	}

	return HANDLER_GO_ON;
}


static int ssi_env_add(void *venv, const char *key, size_t klen, const char *val, size_t vlen) {
	array *env = venv;
	data_string *ds;

	/* array_set_key_value() w/o extra lookup to see if key already exists */
	if (NULL == (ds = (data_string *)array_get_unused_element(env, TYPE_STRING))) {
		ds = data_string_init();
	}
	buffer_copy_string_len(ds->key,   key, klen);
	buffer_copy_string_len(ds->value, val, vlen);

	array_insert_unique(env, (data_unset *)ds);

	return 0;
}

static int build_ssi_cgi_vars(server *srv, connection *con, handler_ctx *p) {
	http_cgi_opts opts = { 0, 0, NULL, NULL };
	/* temporarily remove Authorization from request headers
	 * so that Authorization does not end up in SSI environment */
	data_string *ds_auth = (data_string *)array_get_element(con->request.headers, "Authorization");
	buffer *b_auth = NULL;
	if (ds_auth) {
		b_auth = ds_auth->value;
		ds_auth->value = NULL;
	}

	array_reset(p->ssi_cgi_env);

	if (0 != http_cgi_headers(srv, con, &opts, ssi_env_add, p->ssi_cgi_env)) {
		con->http_status = 400;
		return -1;
	}

	if (ds_auth) {
		ds_auth->value = b_auth;
	}

	return 0;
}

static int mod_ssi_process_file(server *srv, connection *con, handler_ctx *p, struct stat *st);

static int process_ssi_stmt(server *srv, connection *con, handler_ctx *p, const char **l, size_t n, struct stat *st) {

	/**
	 * <!--#element attribute=value attribute=value ... -->
	 *
	 * config       DONE
	 *   errmsg     -- missing
	 *   sizefmt    DONE
	 *   timefmt    DONE
	 * echo         DONE
	 *   var        DONE
	 *   encoding   -- missing
	 * exec         DONE
	 *   cgi        -- never
	 *   cmd        DONE
	 * fsize        DONE
	 *   file       DONE
	 *   virtual    DONE
	 * flastmod     DONE
	 *   file       DONE
	 *   virtual    DONE
	 * include      DONE
	 *   file       DONE
	 *   virtual    DONE
	 * printenv     DONE
	 * set          DONE
	 *   var        DONE
	 *   value      DONE
	 *
	 * if           DONE
	 * elif         DONE
	 * else         DONE
	 * endif        DONE
	 *
	 *
	 * expressions
	 * AND, OR      DONE
	 * comp         DONE
	 * ${...}       -- missing
	 * $...         DONE
	 * '...'        DONE
	 * ( ... )      DONE
	 *
	 *
	 *
	 * ** all DONE **
	 * DATE_GMT
	 *   The current date in Greenwich Mean Time.
	 * DATE_LOCAL
	 *   The current date in the local time zone.
	 * DOCUMENT_NAME
	 *   The filename (excluding directories) of the document requested by the user.
	 * DOCUMENT_URI
	 *   The (%-decoded) URL path of the document requested by the user. Note that in the case of nested include files, this is not then URL for the current document.
	 * LAST_MODIFIED
	 *   The last modification date of the document requested by the user.
	 * USER_NAME
	 *   Contains the owner of the file which included it.
	 *
	 */

	size_t i, ssicmd = 0;
	char buf[255];
	buffer *b = NULL;

	static const struct {
		const char *var;
		enum { SSI_UNSET, SSI_ECHO, SSI_FSIZE, SSI_INCLUDE, SSI_FLASTMOD,
				SSI_CONFIG, SSI_PRINTENV, SSI_SET, SSI_IF, SSI_ELIF,
				SSI_ELSE, SSI_ENDIF, SSI_EXEC, SSI_COMMENT } type;
	} ssicmds[] = {
		{ "echo",     SSI_ECHO },
		{ "include",  SSI_INCLUDE },
		{ "flastmod", SSI_FLASTMOD },
		{ "fsize",    SSI_FSIZE },
		{ "config",   SSI_CONFIG },
		{ "printenv", SSI_PRINTENV },
		{ "set",      SSI_SET },
		{ "if",       SSI_IF },
		{ "elif",     SSI_ELIF },
		{ "endif",    SSI_ENDIF },
		{ "else",     SSI_ELSE },
		{ "exec",     SSI_EXEC },
		{ "comment",  SSI_COMMENT },

		{ NULL, SSI_UNSET }
	};

	for (i = 0; ssicmds[i].var; i++) {
		if (0 == strcmp(l[1], ssicmds[i].var)) {
			ssicmd = ssicmds[i].type;
			break;
		}
	}

	switch(ssicmd) {
	case SSI_ECHO: {
		/* echo */
		int var = 0;
		/* int enc = 0; */
		const char *var_val = NULL;

		static const struct {
			const char *var;
			enum {
				SSI_ECHO_UNSET,
				SSI_ECHO_DATE_GMT,
				SSI_ECHO_DATE_LOCAL,
				SSI_ECHO_DOCUMENT_NAME,
				SSI_ECHO_DOCUMENT_URI,
				SSI_ECHO_LAST_MODIFIED,
				SSI_ECHO_USER_NAME,
				SSI_ECHO_SCRIPT_URI,
				SSI_ECHO_SCRIPT_URL,
			} type;
		} echovars[] = {
			{ "DATE_GMT",      SSI_ECHO_DATE_GMT },
			{ "DATE_LOCAL",    SSI_ECHO_DATE_LOCAL },
			{ "DOCUMENT_NAME", SSI_ECHO_DOCUMENT_NAME },
			{ "DOCUMENT_URI",  SSI_ECHO_DOCUMENT_URI },
			{ "LAST_MODIFIED", SSI_ECHO_LAST_MODIFIED },
			{ "USER_NAME",     SSI_ECHO_USER_NAME },
			{ "SCRIPT_URI",    SSI_ECHO_SCRIPT_URI },
			{ "SCRIPT_URL",    SSI_ECHO_SCRIPT_URL },

			{ NULL, SSI_ECHO_UNSET }
		};

/*
		static const struct {
			const char *var;
			enum { SSI_ENC_UNSET, SSI_ENC_URL, SSI_ENC_NONE, SSI_ENC_ENTITY } type;
		} encvars[] = {
			{ "url",          SSI_ENC_URL },
			{ "none",         SSI_ENC_NONE },
			{ "entity",       SSI_ENC_ENTITY },

			{ NULL, SSI_ENC_UNSET }
		};
*/

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "var")) {
				int j;

				var_val = l[i+1];

				for (j = 0; echovars[j].var; j++) {
					if (0 == strcmp(l[i+1], echovars[j].var)) {
						var = echovars[j].type;
						break;
					}
				}
			} else if (0 == strcmp(l[i], "encoding")) {
/*
				int j;

				for (j = 0; encvars[j].var; j++) {
					if (0 == strcmp(l[i+1], encvars[j].var)) {
						enc = encvars[j].type;
						break;
					}
				}
*/
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"ssi: unknown attribute for ",
						l[1], l[i]);
			}
		}

		if (p->if_is_false) break;

		if (!var_val) {
			log_error_write(srv, __FILE__, __LINE__, "sss",
					"ssi: ",
					l[1], "var is missing");
			break;
		}

		switch(var) {
		case SSI_ECHO_USER_NAME: {
			struct passwd *pw;

			b = buffer_init();
#ifdef HAVE_PWD_H
			if (NULL == (pw = getpwuid(st->st_uid))) {
				buffer_copy_int(b, st->st_uid);
			} else {
				buffer_copy_string(b, pw->pw_name);
			}
#else
			buffer_copy_int(b, st->st_uid);
#endif
			chunkqueue_append_buffer(con->write_queue, b);
			buffer_free(b);
			break;
		}
		case SSI_ECHO_LAST_MODIFIED: {
			time_t t = st->st_mtime;

			if (0 == strftime(buf, sizeof(buf), p->timefmt->ptr, localtime(&t))) {
				chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("(none)"));
			} else {
				chunkqueue_append_mem(con->write_queue, buf, strlen(buf));
			}
			break;
		}
		case SSI_ECHO_DATE_LOCAL: {
			time_t t = time(NULL);

			if (0 == strftime(buf, sizeof(buf), p->timefmt->ptr, localtime(&t))) {
				chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("(none)"));
			} else {
				chunkqueue_append_mem(con->write_queue, buf, strlen(buf));
			}
			break;
		}
		case SSI_ECHO_DATE_GMT: {
			time_t t = time(NULL);

			if (0 == strftime(buf, sizeof(buf), p->timefmt->ptr, gmtime(&t))) {
				chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("(none)"));
			} else {
				chunkqueue_append_mem(con->write_queue, buf, strlen(buf));
			}
			break;
		}
		case SSI_ECHO_DOCUMENT_NAME: {
			char *sl;

			if (NULL == (sl = strrchr(con->physical.path->ptr, '/'))) {
				chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->physical.path));
			} else {
				chunkqueue_append_mem(con->write_queue, sl + 1, strlen(sl + 1));
			}
			break;
		}
		case SSI_ECHO_DOCUMENT_URI: {
			chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->uri.path));
			break;
		}
		case SSI_ECHO_SCRIPT_URI: {
			if (!buffer_string_is_empty(con->uri.scheme) && !buffer_string_is_empty(con->uri.authority)) {
				chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->uri.scheme));
				chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("://"));
				chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->uri.authority));
				chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->request.uri));
				if (!buffer_string_is_empty(con->uri.query)) {
					chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("?"));
					chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->uri.query));
				}
			}
			break;
		}
		case SSI_ECHO_SCRIPT_URL: {
			chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->request.uri));
			if (!buffer_string_is_empty(con->uri.query)) {
				chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("?"));
				chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(con->uri.query));
			}
			break;
		}
		default: {
			data_string *ds;
			/* check if it is a cgi-var or a ssi-var */

			if (NULL != (ds = (data_string *)array_get_element(p->ssi_cgi_env, var_val)) ||
			    NULL != (ds = (data_string *)array_get_element(p->ssi_vars, var_val))) {
				chunkqueue_append_mem(con->write_queue, CONST_BUF_LEN(ds->value));
			} else {
				chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("(none)"));
			}

			break;
		}
		}
		break;
	}
	case SSI_INCLUDE:
	case SSI_FLASTMOD:
	case SSI_FSIZE: {
		const char * file_path = NULL, *virt_path = NULL;
		struct stat stb;
		char *sl;

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "file")) {
				file_path = l[i+1];
			} else if (0 == strcmp(l[i], "virtual")) {
				virt_path = l[i+1];
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"ssi: unknown attribute for ",
						l[1], l[i]);
			}
		}

		if (!file_path && !virt_path) {
			log_error_write(srv, __FILE__, __LINE__, "sss",
					"ssi: ",
					l[1], "file or virtual are missing");
			break;
		}

		if (file_path && virt_path) {
			log_error_write(srv, __FILE__, __LINE__, "sss",
					"ssi: ",
					l[1], "only one of file and virtual is allowed here");
			break;
		}


		if (p->if_is_false) break;

		if (file_path) {
			/* current doc-root */
			if (NULL == (sl = strrchr(con->physical.path->ptr, '/'))) {
				buffer_copy_string_len(p->stat_fn, CONST_STR_LEN("/"));
			} else {
				buffer_copy_string_len(p->stat_fn, con->physical.path->ptr, sl - con->physical.path->ptr + 1);
			}

			buffer_copy_string(srv->tmp_buf, file_path);
			buffer_urldecode_path(srv->tmp_buf);
			buffer_path_simplify(srv->tmp_buf, srv->tmp_buf);
			buffer_append_string_buffer(p->stat_fn, srv->tmp_buf);
		} else {
			/* virtual */
			size_t remain;

			if (virt_path[0] == '/') {
				buffer_copy_string(p->stat_fn, virt_path);
			} else {
				/* there is always a / */
				sl = strrchr(con->uri.path->ptr, '/');

				buffer_copy_string_len(p->stat_fn, con->uri.path->ptr, sl - con->uri.path->ptr + 1);
				buffer_append_string(p->stat_fn, virt_path);
			}

			buffer_urldecode_path(p->stat_fn);
			buffer_path_simplify(srv->tmp_buf, p->stat_fn);

			/* we have an uri */

			/* Destination physical path (similar to code in mod_webdav.c)
			 * src con->physical.path might have been remapped with mod_alias, mod_userdir.
			 *   (but neither modifies con->physical.rel_path)
			 * Find matching prefix to support relative paths to current physical path.
			 * Aliasing of paths underneath current con->physical.basedir might not work.
			 * Likewise, mod_rewrite URL rewriting might thwart this comparison.
			 * Use mod_redirect instead of mod_alias to remap paths *under* this basedir.
			 * Use mod_redirect instead of mod_rewrite on *any* parts of path to basedir.
			 * (Related, use mod_auth to protect this basedir, but avoid attempting to
			 *  use mod_auth on paths underneath this basedir, as target path is not
			 *  validated with mod_auth)
			 */

			/* find matching URI prefix
			 * check if remaining con->physical.rel_path matches suffix
			 *   of con->physical.basedir so that we can use it to
			 *   remap Destination physical path */
			{
				const char *sep, *sep2;
				sep = con->uri.path->ptr;
				sep2 = srv->tmp_buf->ptr;
				for (i = 0; sep[i] && sep[i] == sep2[i]; ++i) ;
				while (i != 0 && sep[--i] != '/') ; /* find matching directory path */
			}
			if (con->conf.force_lowercase_filenames) {
				buffer_to_lower(srv->tmp_buf);
			}
			remain = buffer_string_length(con->uri.path) - i;
			if (!con->conf.force_lowercase_filenames
			    ? buffer_is_equal_right_len(con->physical.path, con->physical.rel_path, remain)
			    :(buffer_string_length(con->physical.path) >= remain
			      && 0 == strncasecmp(con->physical.path->ptr+buffer_string_length(con->physical.path)-remain, con->physical.rel_path->ptr+i, remain))) {
				buffer_copy_string_len(p->stat_fn, con->physical.path->ptr, buffer_string_length(con->physical.path)-remain);
				buffer_append_string_len(p->stat_fn, srv->tmp_buf->ptr+i, buffer_string_length(srv->tmp_buf)-i);
			} else {
				/* unable to perform physical path remap here;
				 * assume doc_root/rel_path and no remapping */
				buffer_copy_buffer(p->stat_fn, con->physical.doc_root);
				buffer_append_string_buffer(p->stat_fn, srv->tmp_buf);
			}
		}

		if (0 == stat(p->stat_fn->ptr, &stb)) {
			time_t t = stb.st_mtime;

			switch (ssicmd) {
			case SSI_FSIZE:
				b = buffer_init();
				if (p->sizefmt) {
					int j = 0;
					const char *abr[] = { " B", " kB", " MB", " GB", " TB", NULL };

					off_t s = stb.st_size;

					for (j = 0; s > 1024 && abr[j+1]; s /= 1024, j++);

					buffer_copy_int(b, s);
					buffer_append_string(b, abr[j]);
				} else {
					buffer_copy_int(b, stb.st_size);
				}
				chunkqueue_append_buffer(con->write_queue, b);
				buffer_free(b);
				break;
			case SSI_FLASTMOD:
				if (0 == strftime(buf, sizeof(buf), p->timefmt->ptr, localtime(&t))) {
					chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("(none)"));
				} else {
					chunkqueue_append_mem(con->write_queue, buf, strlen(buf));
				}
				break;
			case SSI_INCLUDE:
				/* Keep the newest mtime of included files */
				if (stb.st_mtime > include_file_last_mtime)
					include_file_last_mtime = stb.st_mtime;

				if (file_path || 0 == p->conf.ssi_recursion_max) {
					/* don't process if #include file="..." is used */
					chunkqueue_append_file(con->write_queue, p->stat_fn, 0, stb.st_size);
				} else {
					buffer *upsave, *ppsave, *prpsave;

					/* only allow predefined recursion depth */
					if (p->ssi_recursion_depth >= p->conf.ssi_recursion_max) {
						chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("(error: include directives recurse deeper than pre-defined ssi.recursion-max)"));
						break;
					}

					/* prevents simple infinite loop */
					if (buffer_is_equal(con->physical.path, p->stat_fn)) {
						chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("(error: include directives create an infinite loop)"));
						break;
					}

					/* save and restore con->physical.path, con->physical.rel_path, and con->uri.path around include
					 *
					 * srv->tmp_buf contains url-decoded, path-simplified, and lowercased (if con->conf.force_lowercase) uri path of target.
					 * con->uri.path and con->physical.rel_path are set to the same since we only operate on filenames here,
					 * not full re-run of all modules for subrequest */
					upsave = con->uri.path;
					ppsave = con->physical.path;
					prpsave = con->physical.rel_path;

					con->physical.path = p->stat_fn;
					p->stat_fn = buffer_init();

					con->uri.path = con->physical.rel_path = buffer_init_buffer(srv->tmp_buf);

					/*(ignore return value; muddle along as best we can if error occurs)*/
					++p->ssi_recursion_depth;
					mod_ssi_process_file(srv, con, p, &stb);
					--p->ssi_recursion_depth;

					buffer_free(con->uri.path);
					con->uri.path = upsave;
					con->physical.rel_path = prpsave;

					buffer_free(p->stat_fn);
					p->stat_fn = con->physical.path;
					con->physical.path = ppsave;
				}

				break;
			}
		} else {
			log_error_write(srv, __FILE__, __LINE__, "sbs",
					"ssi: stating failed ",
					p->stat_fn, strerror(errno));
		}
		break;
	}
	case SSI_SET: {
		const char *key = NULL, *val = NULL;
		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "var")) {
				key = l[i+1];
			} else if (0 == strcmp(l[i], "value")) {
				val = l[i+1];
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"ssi: unknown attribute for ",
						l[1], l[i]);
			}
		}

		if (p->if_is_false) break;

		if (key && val) {
			data_string *ds;

			if (NULL == (ds = (data_string *)array_get_unused_element(p->ssi_vars, TYPE_STRING))) {
				ds = data_string_init();
			}
			buffer_copy_string(ds->key,   key);
			buffer_copy_string(ds->value, val);

			array_insert_unique(p->ssi_vars, (data_unset *)ds);
		} else if (key || val) {
			log_error_write(srv, __FILE__, __LINE__, "sSSss",
					"ssi: var and value have to be set in <!--#set", l[1], "=", l[2], "-->");
		} else {
			log_error_write(srv, __FILE__, __LINE__, "s",
					"ssi: var and value have to be set in <!--#set var=... value=... -->");
		}
		break;
	}
	case SSI_CONFIG:
		if (p->if_is_false) break;

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "timefmt")) {
				buffer_copy_string(p->timefmt, l[i+1]);
			} else if (0 == strcmp(l[i], "sizefmt")) {
				if (0 == strcmp(l[i+1], "abbrev")) {
					p->sizefmt = 1;
				} else if (0 == strcmp(l[i+1], "bytes")) {
					p->sizefmt = 0;
				} else {
					log_error_write(srv, __FILE__, __LINE__, "sssss",
							"ssi: unknown value for attribute '",
							l[i],
							"' for ",
							l[1], l[i+1]);
				}
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"ssi: unknown attribute for ",
						l[1], l[i]);
			}
		}
		break;
	case SSI_PRINTENV:
		if (p->if_is_false) break;

		b = buffer_init();
		for (i = 0; i < p->ssi_vars->used; i++) {
			data_string *ds = (data_string *)p->ssi_vars->data[p->ssi_vars->sorted[i]];

			buffer_append_string_buffer(b, ds->key);
			buffer_append_string_len(b, CONST_STR_LEN("="));
			buffer_append_string_encoded(b, CONST_BUF_LEN(ds->value), ENCODING_MINIMAL_XML);
			buffer_append_string_len(b, CONST_STR_LEN("\n"));
		}
		for (i = 0; i < p->ssi_cgi_env->used; i++) {
			data_string *ds = (data_string *)p->ssi_cgi_env->data[p->ssi_cgi_env->sorted[i]];

			buffer_append_string_buffer(b, ds->key);
			buffer_append_string_len(b, CONST_STR_LEN("="));
			buffer_append_string_encoded(b, CONST_BUF_LEN(ds->value), ENCODING_MINIMAL_XML);
			buffer_append_string_len(b, CONST_STR_LEN("\n"));
		}
		chunkqueue_append_buffer(con->write_queue, b);
		buffer_free(b);

		break;
	case SSI_EXEC: {
		const char *cmd = NULL;
		pid_t pid;
		int from_exec_fds[2];

		if (!p->conf.ssi_exec) { /* <!--#exec ... --> disabled by config */
			break;
		}

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "cmd")) {
				cmd = l[i+1];
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"ssi: unknown attribute for ",
						l[1], l[i]);
			}
		}

		if (p->if_is_false) break;

		/* create a return pipe and send output to the html-page
		 *
		 * as exec is assumed evil it is implemented synchronously
		 */

		if (!cmd) break;
#ifdef HAVE_FORK
		if (pipe(from_exec_fds)) {
			log_error_write(srv, __FILE__, __LINE__, "ss",
					"pipe failed: ", strerror(errno));
			return -1;
		}

		/* fork, execve */
		switch (pid = fork()) {
		case 0: {
			/* move stdout to from_rrdtool_fd[1] */
			close(STDOUT_FILENO);
			dup2(from_exec_fds[1], STDOUT_FILENO);
			close(from_exec_fds[1]);
			/* not needed */
			close(from_exec_fds[0]);

			/* close stdin */
			close(STDIN_FILENO);

			execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);

			log_error_write(srv, __FILE__, __LINE__, "sss", "spawing exec failed:", strerror(errno), cmd);

			/* */
			SEGFAULT();
			break;
		}
		case -1:
			/* error */
			log_error_write(srv, __FILE__, __LINE__, "ss", "fork failed:", strerror(errno));
			break;
		default: {
			/* father */
			int status;
			ssize_t r;
			int was_interrupted = 0;

			close(from_exec_fds[1]);

			/* wait for the client to end */

			/*
			 * OpenBSD and Solaris send a EINTR on SIGCHILD even if we ignore it
			 */
			do {
				if (-1 == waitpid(pid, &status, 0)) {
					if (errno == EINTR) {
						was_interrupted++;
					} else {
						was_interrupted = 0;
						log_error_write(srv, __FILE__, __LINE__, "ss", "waitpid failed:", strerror(errno));
					}
				} else if (WIFEXITED(status)) {
					int toread;
					/* read everything from client and paste it into the output */
					was_interrupted = 0;

					while(1) {
						if (ioctl(from_exec_fds[0], FIONREAD, &toread)) {
							log_error_write(srv, __FILE__, __LINE__, "s",
								"unexpected end-of-file (perhaps the ssi-exec process died)");
							return -1;
						}

						if (toread > 0) {
							char *mem;
							size_t mem_len;

							chunkqueue_get_memory(con->write_queue, &mem, &mem_len, 0, toread);
							r = read(from_exec_fds[0], mem, mem_len);
							chunkqueue_use_memory(con->write_queue, r > 0 ? r : 0);

							if (r < 0) break; /* read failed */
						} else {
							break;
						}
					}
				} else {
					was_interrupted = 0;
					log_error_write(srv, __FILE__, __LINE__, "s", "process exited abnormally");
				}
			} while (was_interrupted > 0 && was_interrupted < 4); /* if waitpid() gets interrupted, retry, but max 4 times */

			close(from_exec_fds[0]);

			break;
		}
		}
#else

		return -1;
#endif

		break;
	}
	case SSI_IF: {
		const char *expr = NULL;

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "expr")) {
				expr = l[i+1];
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"ssi: unknown attribute for ",
						l[1], l[i]);
			}
		}

		if (!expr) {
			log_error_write(srv, __FILE__, __LINE__, "sss",
					"ssi: ",
					l[1], "expr missing");
			break;
		}

		if ((!p->if_is_false) &&
		    ((p->if_is_false_level == 0) ||
		     (p->if_level < p->if_is_false_level))) {
			switch (ssi_eval_expr(srv, con, p, expr)) {
			case -1:
			case 0:
				p->if_is_false = 1;
				p->if_is_false_level = p->if_level;
				break;
			case 1:
				p->if_is_false = 0;
				break;
			}
		}

		p->if_level++;

		break;
	}
	case SSI_ELSE:
		p->if_level--;

		if (p->if_is_false) {
			if ((p->if_level == p->if_is_false_level) &&
			    (p->if_is_false_endif == 0)) {
				p->if_is_false = 0;
			}
		} else {
			p->if_is_false = 1;

			p->if_is_false_level = p->if_level;
		}
		p->if_level++;

		break;
	case SSI_ELIF: {
		const char *expr = NULL;
		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "expr")) {
				expr = l[i+1];
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"ssi: unknown attribute for ",
						l[1], l[i]);
			}
		}

		if (!expr) {
			log_error_write(srv, __FILE__, __LINE__, "sss",
					"ssi: ",
					l[1], "expr missing");
			break;
		}

		p->if_level--;

		if (p->if_level == p->if_is_false_level) {
			if ((p->if_is_false) &&
			    (p->if_is_false_endif == 0)) {
				switch (ssi_eval_expr(srv, con, p, expr)) {
				case -1:
				case 0:
					p->if_is_false = 1;
					p->if_is_false_level = p->if_level;
					break;
				case 1:
					p->if_is_false = 0;
					break;
				}
			} else {
				p->if_is_false = 1;
				p->if_is_false_level = p->if_level;
				p->if_is_false_endif = 1;
			}
		}

		p->if_level++;

		break;
	}
	case SSI_ENDIF:
		p->if_level--;

		if (p->if_level == p->if_is_false_level) {
			p->if_is_false = 0;
			p->if_is_false_endif = 0;
		}

		break;
	case SSI_COMMENT:
		break;
	default:
		log_error_write(srv, __FILE__, __LINE__, "ss",
				"ssi: unknown ssi-command:",
				l[1]);
		break;
	}

	return 0;

}

static int mod_ssi_parse_ssi_stmt_value(const char * const s, const int len) {
	int n;
	const int c = (s[0] == '"' ? '"' : s[0] == '\'' ? '\'' : 0);
	if (0 != c) {
		for (n = 1; n < len; ++n) {
			if (s[n] == c) return n+1;
			if (s[n] == '\\') {
				if (n+1 == len) return 0; /* invalid */
				++n;
			}
		}
		return 0; /* invalid */
	} else {
		for (n = 0; n < len; ++n) {
			if (isspace(s[n])) return n;
			if (s[n] == '\\') {
				if (n+1 == len) return 0; /* invalid */
				++n;
			}
		}
		return n;
	}
}

static int mod_ssi_parse_ssi_stmt_offlen(int o[10], const char * const s, const int len) {

	/**
	 * <!--#element attribute=value attribute=value ... -->
	 */

	/* s must begin "<!--#" and must end with "-->" */
	int n = 5;
	o[0] = n;
	for (; light_isalpha(s[n]); ++n) ; /*(n = 5 to begin after "<!--#")*/
	o[1] = n - o[0];
	if (0 == o[1]) return -1; /* empty token */

	if (n+3 == len) return 2; /* token only; no params */
	if (!isspace(s[n])) return -1;
	do { ++n; } while (isspace(s[n])); /* string ends "-->", so n < len */
	if (n+3 == len) return 2; /* token only; no params */

	o[2] = n;
	for (; light_isalpha(s[n]); ++n) ;
	o[3] = n - o[2];
	if (0 == o[3] || s[n++] != '=') return -1;

	o[4] = n;
	o[5] = mod_ssi_parse_ssi_stmt_value(s+n, len-n-3);
	if (0 == o[5]) return -1; /* empty or invalid token */
	n += o[5];

	if (n+3 == len) return 6; /* token and one param */
	if (!isspace(s[n])) return -1;
	do { ++n; } while (isspace(s[n])); /* string ends "-->", so n < len */
	if (n+3 == len) return 6; /* token and one param */

	o[6] = n;
	for (; light_isalpha(s[n]); ++n) ;
	o[7] = n - o[6];
	if (0 == o[7] || s[n++] != '=') return -1;

	o[8] = n;
	o[9] = mod_ssi_parse_ssi_stmt_value(s+n, len-n-3);
	if (0 == o[9]) return -1; /* empty or invalid token */
	n += o[9];

	if (n+3 == len) return 10; /* token and two params */
	if (!isspace(s[n])) return -1;
	do { ++n; } while (isspace(s[n])); /* string ends "-->", so n < len */
	if (n+3 == len) return 10; /* token and two params */
	return -1;
}

static void mod_ssi_parse_ssi_stmt(server *srv, connection *con, handler_ctx *p, char *s, int len, struct stat *st) {

	/**
	 * <!--#element attribute=value attribute=value ... -->
	 */

	int o[10];
	int m;
	const int n = mod_ssi_parse_ssi_stmt_offlen(o, s, len);
	char *l[6] = { s, NULL, NULL, NULL, NULL, NULL };
	if (-1 == n) {
		/* ignore <!--#comment ... --> */
		if (len >= 16
		    && 0 == memcmp(s+5, "comment", sizeof("comment")-1)
		    && (s[12] == ' ' || s[12] == '\t'))
			return;
		/* XXX: perhaps emit error comment instead of invalid <!--#...--> code to client */
		chunkqueue_append_mem(con->write_queue, s, len); /* append stmt as-is */
		return;
	}

      #if 0
	/* dup s and then modify s */
	/*(l[0] is no longer used; was previously used in only one place for error reporting)*/
	l[0] = malloc((size_t)(len+1));
	memcpy(l[0], s, (size_t)len);
	(l[0])[len] = '\0';
      #endif

	/* modify s in-place to split string into arg tokens */
	for (m = 0; m < n; m += 2) {
		char *ptr = s+o[m];
		switch (*ptr) {
		case '"':
		case '\'': (++ptr)[o[m+1]-2] = '\0'; break;
		default:       ptr[o[m+1]] = '\0';   break;
		}
		l[1+(m>>1)] = ptr;
		if (m == 4 || m == 8) {
			/* XXX: removing '\\' escapes from param value would be
			 * the right thing to do, but would potentially change
			 * current behavior, e.g. <!--#exec cmd=... --> */
		}
	}

	process_ssi_stmt(srv, con, p, (const char **)l, 1+(n>>1), st);

      #if 0
	free(l[0]);
      #endif
}

static int mod_ssi_stmt_len(const char *s, const int len) {
	/* s must begin "<!--#" */
	int n, sq = 0, dq = 0, bs = 0;
	for (n = 5; n < len; ++n) { /*(n = 5 to begin after "<!--#")*/
		switch (s[n]) {
		default:
			break;
		case '-':
			if (!sq && !dq && n+2 < len && s[n+1] == '-' && s[n+2] == '>') return n+3; /* found end of stmt */
			break;
		case '"':
			if (!sq && (!dq || !bs)) dq = !dq;
			break;
		case '\'':
			if (!dq && (!sq || !bs)) sq = !sq;
			break;
		case '\\':
			if (sq || dq) bs = !bs;
			break;
		}
	}
	return 0; /* incomplete directive "<!--#...-->" */
}

static void mod_ssi_read_fd(server *srv, connection *con, handler_ctx *p, struct stat *st, int fd) {
	ssize_t rd;
	size_t offset, pretag;
	size_t bufsz = 8192;
	char *buf = malloc(bufsz); /* allocate to reduce chance of stack exhaustion upon deep recursion */
	force_assert(buf);

	offset = 0;
	pretag = 0;
	while (0 < (rd = read(fd, buf+offset, bufsz-offset))) {
		char *s;
		size_t prelen = 0, len;
		offset += (size_t)rd;
		for (; (s = memchr(buf+prelen, '<', offset-prelen)); ++prelen) {
			prelen = s - buf;
			if (prelen + 5 <= offset) { /*("<!--#" is 5 chars)*/
				if (0 != memcmp(s+1, CONST_STR_LEN("!--#"))) continue; /* loop to loop for next '<' */

				if (prelen - pretag && !p->if_is_false) {
					chunkqueue_append_mem(con->write_queue, buf+pretag, prelen-pretag);
				}

				len = mod_ssi_stmt_len(buf+prelen, offset-prelen);
				if (len) { /* num of chars to be consumed */
					mod_ssi_parse_ssi_stmt(srv, con, p, buf+prelen, len, st);
					prelen += (len - 1); /* offset to '>' at end of SSI directive; incremented at top of loop */
					pretag = prelen + 1;
					if (pretag == offset) {
						offset = pretag = 0;
						break;
					}
				} else if (0 == prelen && offset == bufsz) { /*(full buf)*/
					/* SSI statement is way too long
					 * NOTE: skipping this buf will expose *the rest* of this SSI statement */
					chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("<!-- [an error occurred: directive too long] "));
					/* check if buf ends with "-" or "--" which might be part of "-->"
					 * (buf contains at least 5 chars for "<!--#") */
					if (buf[offset-2] == '-' && buf[offset-1] == '-') {
						chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("--"));
					} else if (buf[offset-1] == '-') {
						chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("-"));
					}
					offset = pretag = 0;
					break;
				} else { /* incomplete directive "<!--#...-->" */
					memmove(buf, buf+prelen, (offset -= prelen));
					pretag = 0;
					break;
				}
			} else if (prelen + 1 == offset || 0 == memcmp(s+1, "!--", offset - prelen - 1)) {
				if (prelen - pretag && !p->if_is_false) {
					chunkqueue_append_mem(con->write_queue, buf+pretag, prelen-pretag);
				}
				memcpy(buf, buf+prelen, (offset -= prelen));
				pretag = 0;
				break;
			}
			/* loop to look for next '<' */
		}
		if (offset == bufsz) {
			if (!p->if_is_false) {
				chunkqueue_append_mem(con->write_queue, buf+pretag, offset-pretag);
			}
			offset = pretag = 0;
		}
	}

	if (0 != rd) {
		log_error_write(srv, __FILE__, __LINE__,  "SsB", "read(): ", strerror(errno), con->physical.path);
	}

	if (offset - pretag) {
		/* copy remaining data in buf */
		if (!p->if_is_false) {
			chunkqueue_append_mem(con->write_queue, buf+pretag, offset-pretag);
		}
	}

	free(buf);
}


/* don't want to block when open()ing a fifo */
#if defined(O_NONBLOCK)
# define FIFO_NONBLOCK O_NONBLOCK
#else
# define FIFO_NONBLOCK 0
#endif

static int mod_ssi_process_file(server *srv, connection *con, handler_ctx *p, struct stat *st) {
	int fd = open(con->physical.path->ptr, O_RDONLY | FIFO_NONBLOCK);
	if (-1 == fd) {
		log_error_write(srv, __FILE__, __LINE__,  "SsB", "open(): ",
				strerror(errno), con->physical.path);
		return -1;
	}

	if (0 != fstat(fd, st)) {
		log_error_write(srv, __FILE__, __LINE__,  "SsB", "fstat(): ",
				strerror(errno), con->physical.path);
		close(fd);
		return -1;
	}

	mod_ssi_read_fd(srv, con, p, st, fd);

	close(fd);
	return 0;
}


static int mod_ssi_handle_request(server *srv, connection *con, handler_ctx *p) {
	struct stat st;

	/* get a stream to the file */

	array_reset(p->ssi_vars);
	array_reset(p->ssi_cgi_env);
	buffer_copy_string_len(p->timefmt, CONST_STR_LEN("%a, %d %b %Y %H:%M:%S %Z"));
	build_ssi_cgi_vars(srv, con, p);

	/* Reset the modified time of included files */
	include_file_last_mtime = 0;

	mod_ssi_process_file(srv, con, p, &st);

	con->file_started  = 1;
	con->file_finished = 1;

	if (buffer_string_is_empty(p->conf.content_type)) {
		response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
	} else {
		response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(p->conf.content_type));
	}

	if (p->conf.conditional_requests) {
		/* Generate "ETag" & "Last-Modified" headers */
		buffer *mtime = NULL;

		/* use most recently modified include file for ETag and Last-Modified */
		if (st.st_mtime < include_file_last_mtime)
			st.st_mtime = include_file_last_mtime;

		etag_create(con->physical.etag, &st, con->etag_flags);
		response_header_overwrite(srv, con, CONST_STR_LEN("ETag"), CONST_BUF_LEN(con->physical.etag));

		mtime = strftime_cache_get(srv, st.st_mtime);
		response_header_overwrite(srv, con, CONST_STR_LEN("Last-Modified"), CONST_BUF_LEN(mtime));

		if (HANDLER_FINISHED == http_response_handle_cachable(srv, con, mtime)) {
			/* ok, the client already has our content,
			 * no need to send it again */

			chunkqueue_reset(con->write_queue);
		}
	}

	/* Reset the modified time of included files */
	include_file_last_mtime = 0;

	/* reset physical.path */
	buffer_reset(con->physical.path);

	return 0;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_ssi_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(ssi_extension);
	PATCH(content_type);
	PATCH(conditional_requests);
	PATCH(ssi_exec);
	PATCH(ssi_recursion_max);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssi.extension"))) {
				PATCH(ssi_extension);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssi.content-type"))) {
				PATCH(content_type);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssi.conditional-requests"))) {
				PATCH(conditional_requests);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssi.exec"))) {
				PATCH(ssi_exec);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssi.recursion-max"))) {
				PATCH(ssi_recursion_max);
			}
		}
	}

	return 0;
}
#undef PATCH

URIHANDLER_FUNC(mod_ssi_physical_path) {
	plugin_data *p = p_d;
	size_t k;

	if (con->mode != DIRECT) return HANDLER_GO_ON;

	if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;

	mod_ssi_patch_connection(srv, con, p);

	for (k = 0; k < p->conf.ssi_extension->used; k++) {
		data_string *ds = (data_string *)p->conf.ssi_extension->data[k];

		if (buffer_is_empty(ds->value)) continue;

		if (buffer_is_equal_right_len(con->physical.path, ds->value, buffer_string_length(ds->value))) {
			con->plugin_ctx[p->id] = handler_ctx_init(p);
			con->mode = p->id;
			break;
		}
	}

	return HANDLER_GO_ON;
}

SUBREQUEST_FUNC(mod_ssi_handle_subrequest) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];
	if (NULL == hctx) return HANDLER_GO_ON;
	if (con->mode != p->id) return HANDLER_GO_ON; /* not my job */
	/*
	 * NOTE: if mod_ssi modified to use fdevents, HANDLER_WAIT_FOR_EVENT,
	 * instead of blocking to completion, then hctx->timefmt, hctx->ssi_vars,
	 * and hctx->ssi_cgi_env should be allocated and cleaned up per request.
	 */

			/* handle ssi-request */

			if (mod_ssi_handle_request(srv, con, hctx)) {
				/* on error */
				con->http_status = 500;
				con->mode = DIRECT;
			}

			return HANDLER_FINISHED;
}

static handler_t mod_ssi_connection_reset(server *srv, connection *con, void *p_d) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];
	if (hctx) {
		handler_ctx_free(hctx);
		con->plugin_ctx[p->id] = NULL;
	}

	UNUSED(srv);
	return HANDLER_GO_ON;
}

/* this function is called at dlopen() time and inits the callbacks */

int mod_ssi_plugin_init(plugin *p);
int mod_ssi_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("ssi");

	p->init        = mod_ssi_init;
	p->handle_subrequest_start = mod_ssi_physical_path;
	p->handle_subrequest       = mod_ssi_handle_subrequest;
	p->connection_reset        = mod_ssi_connection_reset;
	p->set_defaults  = mod_ssi_set_defaults;
	p->cleanup     = mod_ssi_free;

	p->data        = NULL;

	return 0;
}
