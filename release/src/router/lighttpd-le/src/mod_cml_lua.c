#include "first.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "mod_cml_funcs.h"
#include "mod_cml.h"

#include "log.h"
#include "stat_cache.h"

#define HASHLEN 16
typedef unsigned char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];

static int lua_to_c_get_string(lua_State *L, const char *varname, buffer *b) {
	int curelem = lua_gettop(L);
	int result;

	lua_getglobal(L, varname);

	if (lua_isstring(L, curelem)) {
		buffer_copy_string(b, lua_tostring(L, curelem));
		result = 0;
	} else {
		result = -1;
	}

	lua_pop(L, 1);
	force_assert(curelem == lua_gettop(L));
	return result;
}

static int lua_to_c_is_table(lua_State *L, const char *varname) {
	int curelem = lua_gettop(L);
	int result;

	lua_getglobal(L, varname);

	result = lua_istable(L, curelem) ? 1 : 0;

	lua_pop(L, 1);
	force_assert(curelem == lua_gettop(L));
	return result;
}

static int c_to_lua_push(lua_State *L, int tbl, const char *key, size_t key_len, const char *val, size_t val_len) {
	lua_pushlstring(L, key, key_len);
	lua_pushlstring(L, val, val_len);
	lua_settable(L, tbl);

	return 0;
}

static int cache_export_get_params(lua_State *L, int tbl, buffer *qrystr) {
	size_t is_key = 1;
	size_t i, len;
	char *key = NULL, *val = NULL;

	key = qrystr->ptr;

	/* we need the \0 */
	len = buffer_string_length(qrystr);
	for (i = 0; i <= len; i++) {
		switch(qrystr->ptr[i]) {
		case '=':
			if (is_key) {
				val = qrystr->ptr + i + 1;

				qrystr->ptr[i] = '\0';

				is_key = 0;
			}

			break;
		case '&':
		case '\0': /* fin symbol */
			if (!is_key) {
				/* we need at least a = since the last & */

				/* terminate the value */
				qrystr->ptr[i] = '\0';

				c_to_lua_push(L, tbl,
					key, strlen(key),
					val, strlen(val));
			}

			key = qrystr->ptr + i + 1;
			val = NULL;
			is_key = 1;
			break;
		}
	}

	return 0;
}

int cache_parse_lua(server *srv, connection *con, plugin_data *p, buffer *fn) {
	lua_State *L;
	int ret = -1;
	buffer *b;

	b = buffer_init();
	/* push the lua file to the interpreter and see what happends */
	L = luaL_newstate();
	luaL_openlibs(L);

	/* register functions */
	lua_register(L, "md5", f_crypto_md5);
	lua_register(L, "file_mtime", f_file_mtime);
	lua_register(L, "file_isreg", f_file_isreg);
	lua_register(L, "file_isdir", f_file_isreg);
	lua_register(L, "dir_files", f_dir_files);

#ifdef USE_MEMCACHED
	lua_pushlightuserdata(L, p->conf.memc);
	lua_pushcclosure(L, f_memcache_get_long, 1);
	lua_setglobal(L, "memcache_get_long");

	lua_pushlightuserdata(L, p->conf.memc);
	lua_pushcclosure(L, f_memcache_get_string, 1);
	lua_setglobal(L, "memcache_get_string");

	lua_pushlightuserdata(L, p->conf.memc);
	lua_pushcclosure(L, f_memcache_exists, 1);
	lua_setglobal(L, "memcache_exists");
#endif

	/* register CGI environment */
	lua_newtable(L);
	{
		int header_tbl = lua_gettop(L);

		c_to_lua_push(L, header_tbl, CONST_STR_LEN("REQUEST_URI"), CONST_BUF_LEN(con->request.orig_uri));
		c_to_lua_push(L, header_tbl, CONST_STR_LEN("SCRIPT_NAME"), CONST_BUF_LEN(con->uri.path));
		c_to_lua_push(L, header_tbl, CONST_STR_LEN("SCRIPT_FILENAME"), CONST_BUF_LEN(con->physical.path));
		c_to_lua_push(L, header_tbl, CONST_STR_LEN("DOCUMENT_ROOT"), CONST_BUF_LEN(con->physical.basedir));
		if (!buffer_string_is_empty(con->request.pathinfo)) {
			c_to_lua_push(L, header_tbl, CONST_STR_LEN("PATH_INFO"), CONST_BUF_LEN(con->request.pathinfo));
		}

		c_to_lua_push(L, header_tbl, CONST_STR_LEN("CWD"), CONST_BUF_LEN(p->basedir));
		c_to_lua_push(L, header_tbl, CONST_STR_LEN("BASEURL"), CONST_BUF_LEN(p->baseurl));
	}
	lua_setglobal(L, "request");

	/* register GET parameter */
	lua_newtable(L);
	{
		int get_tbl = lua_gettop(L);

		buffer_copy_buffer(b, con->uri.query);
		cache_export_get_params(L, get_tbl, b);
		buffer_reset(b);
	}
	lua_setglobal(L, "get");

	/* 2 default constants */
	lua_pushinteger(L, 0);
	lua_setglobal(L, "CACHE_HIT");

	lua_pushinteger(L, 1);
	lua_setglobal(L, "CACHE_MISS");

	/* load lua program */
	ret = luaL_loadfile(L, fn->ptr);
	if (0 != ret) {
		log_error_write(srv, __FILE__, __LINE__, "sbsS",
			"failed loading cml_lua script",
			fn,
			":",
			lua_tostring(L, -1));
		goto error;
	}

	if (lua_pcall(L, 0, 1, 0)) {
		log_error_write(srv, __FILE__, __LINE__, "sbsS",
			"failed running cml_lua script",
			fn,
			":",
			lua_tostring(L, -1));
		goto error;
	}

	/* get return value */
	ret = (int)lua_tointeger(L, -1);
	lua_pop(L, 1);

	/* fetch the data from lua */
	lua_to_c_get_string(L, "trigger_handler", p->trigger_handler);

	if (0 == lua_to_c_get_string(L, "output_contenttype", b)) {
		response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(b));
	}

	if (ret == 0) {
		/* up to now it is a cache-hit, check if all files exist */

		int curelem;
		time_t mtime = 0;

		if (!lua_to_c_is_table(L, "output_include")) {
			log_error_write(srv, __FILE__, __LINE__, "s",
				"output_include is missing or not a table");
			ret = -1;

			goto error;
		}

		lua_getglobal(L, "output_include");
		curelem = lua_gettop(L);

		/* HOW-TO build a etag ?
		 * as we don't just have one file we have to take the stat()
		 * from all base files, merge them and build the etag from
		 * it later.
		 *
		 * The mtime of the content is the mtime of the freshest base file
		 *
		 * */

		lua_pushnil(L);  /* first key */
		while (lua_next(L, curelem) != 0) {
			/* key' is at index -2 and value' at index -1 */

			if (lua_isstring(L, -1)) {
				const char *s = lua_tostring(L, -1);
				struct stat st;
				int fd;

				/* the file is relative, make it absolute */
				if (s[0] != '/') {
					buffer_copy_buffer(b, p->basedir);
					buffer_append_string(b, lua_tostring(L, -1));
				} else {
					buffer_copy_string(b, lua_tostring(L, -1));
				}

				fd = stat_cache_open_rdonly_fstat(srv, con, b, &st);
				if (fd < 0) {
					/* stat failed */

					switch(errno) {
					case ENOENT:
						/* a file is missing, call the handler to generate it */
						if (!buffer_string_is_empty(p->trigger_handler)) {
							ret = 1; /* cache-miss */

							log_error_write(srv, __FILE__, __LINE__, "s",
									"a file is missing, calling handler");

							break;
						} else {
							/* handler not set -> 500 */
							ret = -1;

							log_error_write(srv, __FILE__, __LINE__, "s",
									"a file missing and no handler set");

							break;
						}
						break;
					default:
						break;
					}
				} else {
					chunkqueue_append_file_fd(con->write_queue, b, fd, 0, st.st_size);
					if (st.st_mtime > mtime) mtime = st.st_mtime;
				}
			} else {
				/* not a string */
				ret = -1;
				log_error_write(srv, __FILE__, __LINE__, "s",
						"not a string");
				break;
			}

			lua_pop(L, 1);  /* removes value'; keeps key' for next iteration */
		}

		lua_settop(L, curelem - 1);

		if (ret == 0) {
			data_string *ds;
			char timebuf[sizeof("Sat, 23 Jul 2005 21:20:01 GMT")];

			con->file_finished = 1;

			ds = (data_string *)array_get_element(con->response.headers, "Last-Modified");
			if (0 == mtime) mtime = time(NULL); /* default last-modified to now */

			/* no Last-Modified specified */
			if (NULL == ds) {

				strftime(timebuf, sizeof(timebuf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&mtime));

				response_header_overwrite(srv, con, CONST_STR_LEN("Last-Modified"), timebuf, sizeof(timebuf) - 1);
				ds = (data_string *)array_get_element(con->response.headers, "Last-Modified");
				force_assert(NULL != ds);
			}

			if (HANDLER_FINISHED == http_response_handle_cachable(srv, con, ds->value)) {
				/* ok, the client already has our content,
				 * no need to send it again */

				chunkqueue_reset(con->write_queue);
				ret = 0; /* cache-hit */
			}
		} else {
			chunkqueue_reset(con->write_queue);
		}
	}

	if (ret == 1 && !buffer_string_is_empty(p->trigger_handler)) {
		/* cache-miss */
		buffer_copy_buffer(con->uri.path, p->baseurl);
		buffer_append_string_buffer(con->uri.path, p->trigger_handler);

		buffer_copy_buffer(con->physical.path, p->basedir);
		buffer_append_string_buffer(con->physical.path, p->trigger_handler);

		chunkqueue_reset(con->write_queue);
	}

error:
	lua_close(L);

	buffer_free(b);

	return ret /* cache-error */;
}
