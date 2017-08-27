#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"
#include "http_chunk.h"

#include "plugin.h"

#include "mod_magnet_cache.h"
#include "response.h"
#include "stat_cache.h"
#include "status_counter.h"
#include "etag.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>

#include <lua.h>
#include <lauxlib.h>

#define LUA_RIDX_LIGHTTPD_SERVER     "lighty.srv"
#define LUA_RIDX_LIGHTTPD_CONNECTION "lighty.con"

#define MAGNET_CONFIG_RAW_URL       "magnet.attract-raw-url-to"
#define MAGNET_CONFIG_PHYSICAL_PATH "magnet.attract-physical-path-to"
#define MAGNET_RESTART_REQUEST      99

/* plugin config for all request/connections */

static jmp_buf exceptionjmp;

typedef struct {
	array *url_raw;
	array *physical_path;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	script_cache *cache;

	buffer *encode_buf;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

/* init the plugin data */
INIT_FUNC(mod_magnet_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	p->cache = script_cache_init();
	p->encode_buf = buffer_init();

	return p;
}

/* detroy the plugin data */
FREE_FUNC(mod_magnet_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;

		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (NULL == s) continue;

			array_free(s->url_raw);
			array_free(s->physical_path);

			free(s);
		}
		free(p->config_storage);
	}

	script_cache_free(p->cache);
	buffer_free(p->encode_buf);

	free(p);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_magnet_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ MAGNET_CONFIG_RAW_URL,       NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ MAGNET_CONFIG_PHYSICAL_PATH, NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 1 */
		{ NULL,                           NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->url_raw  = array_init();
		s->physical_path = array_init();

		cv[0].destination = s->url_raw;
		cv[1].destination = s->physical_path;

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}
	}

	return HANDLER_GO_ON;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_magnet_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(url_raw);
	PATCH(physical_path);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN(MAGNET_CONFIG_RAW_URL))) {
				PATCH(url_raw);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN(MAGNET_CONFIG_PHYSICAL_PATH))) {
				PATCH(physical_path);
			}
		}
	}

	return 0;
}
#undef PATCH

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
/* lua5.1 backward compat definition */
static void lua_pushglobaltable(lua_State *L) { /* (-0, +1, -) */
	lua_pushvalue(L, LUA_GLOBALSINDEX);
}
#endif

static void magnet_setfenv_mainfn(lua_State *L, int funcIndex) { /* (-1, 0, -) */
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 502
	/* set "_ENV" upvalue, which should be the first upvalue of a "main" lua
	 * function if it uses any global names
	 */

	const char* first_upvalue_name = lua_getupvalue(L, funcIndex, 1);
	if (NULL == first_upvalue_name) return; /* doesn't have any upvalues */
	lua_pop(L, 1); /* only need the name of the upvalue, not the value */

	if (0 != strcmp(first_upvalue_name, "_ENV")) return;

	if (NULL == lua_setupvalue(L, funcIndex, 1)) {
		/* pop value if lua_setupvalue didn't set the (not existing) upvalue */
		lua_pop(L, 1);
	}
#else
	lua_setfenv(L, funcIndex);
#endif
}

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
/* lua 5.2 already supports __pairs */

/* See http://lua-users.org/wiki/GeneralizedPairsAndIpairs for implementation details.
 * Override the default pairs() function to allow us to use a __pairs metakey
 */
static int magnet_pairs(lua_State *L) {
	luaL_checkany(L, 1); /* "self" */

	if (luaL_getmetafield(L, 1, "__pairs")) {
		/* call __pairs(self) */
		lua_pushvalue(L, 1);
		lua_call(L, 1, 3);
	} else {
		/* call <original-pairs-method>(self) */
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_pushvalue(L, 1);
		lua_call(L, 1, 3);
	}
	return 3;
}
#endif

/* Define a function that will iterate over an array* (in upval 1) using current position (upval 2) */
static int magnet_array_next(lua_State *L) {
	data_unset *du;
	data_string *ds;
	data_integer *di;

	size_t pos = lua_tointeger(L, lua_upvalueindex(1));
	array *a = lua_touserdata(L, lua_upvalueindex(2));

	lua_settop(L, 0);

	if (pos >= a->used) return 0;
	if (NULL != (du = a->data[pos])) {
		lua_pushlstring(L, CONST_BUF_LEN(du->key));
		switch (du->type) {
			case TYPE_STRING:
				ds = (data_string *)du;
				if (!buffer_is_empty(ds->value)) {
					lua_pushlstring(L, CONST_BUF_LEN(ds->value));
				} else {
					lua_pushnil(L);
				}
				break;
			case TYPE_INTEGER:
				di = (data_integer *)du;
				lua_pushinteger(L, di->value);
				break;
			default:
				lua_pushnil(L);
				break;
		}

		/* Update our positional upval to reflect our new current position */
		pos++;
		lua_pushinteger(L, pos);
		lua_replace(L, lua_upvalueindex(1));

		/* Returning 2 items on the stack (key, value) */
		return 2;
	}
	return 0;
}

/* Create the closure necessary to iterate over the array *a with the above function */
static int magnet_array_pairs(lua_State *L, array *a) {
	lua_pushinteger(L, 0); /* Push our current pos (the start) into upval 1 */
	lua_pushlightuserdata(L, a); /* Push our array *a into upval 2 */
	lua_pushcclosure(L, magnet_array_next, 2); /* Push our new closure with 2 upvals */
	return 1;
}

static server* magnet_get_server(lua_State *L) {
	server *srv;

	lua_getfield(L, LUA_REGISTRYINDEX, LUA_RIDX_LIGHTTPD_SERVER);
	srv = lua_touserdata(L, -1);
	lua_pop(L, 1);

	return srv;
}

static connection* magnet_get_connection(lua_State *L) {
	connection *con;

	lua_getfield(L, LUA_REGISTRYINDEX, LUA_RIDX_LIGHTTPD_CONNECTION);
	con = lua_touserdata(L, -1);
	lua_pop(L, 1);

	return con;
}

typedef struct {
	const char *ptr;
	size_t len;
} const_buffer;

static const_buffer magnet_checkconstbuffer(lua_State *L, int index) {
	const_buffer cb;
	cb.ptr = luaL_checklstring(L, index, &cb.len);
	return cb;
}

static buffer* magnet_checkbuffer(lua_State *L, int index) {
	const_buffer cb = magnet_checkconstbuffer(L, index);
	buffer *b = buffer_init();
	buffer_copy_string_len(b, cb.ptr, cb.len);
	return b;
}

static int magnet_print(lua_State *L) {
	buffer *b = magnet_checkbuffer(L, 1);

	log_error_write(magnet_get_server(L), __FILE__, __LINE__, "sB",
		"(lua-print)",
		b);

	buffer_free(b);

	return 0;
}

static int magnet_stat(lua_State *L) {
	buffer *sb = magnet_checkbuffer(L, 1);
	server *srv = magnet_get_server(L);
	connection *con = magnet_get_connection(L);
	stat_cache_entry *sce = NULL;
	handler_t res;

	res = stat_cache_get_entry(srv, con, sb, &sce);
	buffer_free(sb);

	if (HANDLER_GO_ON != res) {
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L); // return value

	lua_pushboolean(L, S_ISREG(sce->st.st_mode));
	lua_setfield(L, -2, "is_file");

	lua_pushboolean(L, S_ISDIR(sce->st.st_mode));
	lua_setfield(L, -2, "is_dir");

	lua_pushboolean(L, S_ISCHR(sce->st.st_mode));
	lua_setfield(L, -2, "is_char");

	lua_pushboolean(L, S_ISBLK(sce->st.st_mode));
	lua_setfield(L, -2, "is_block");

	lua_pushboolean(L, S_ISSOCK(sce->st.st_mode));
	lua_setfield(L, -2, "is_socket");

	lua_pushboolean(L, S_ISLNK(sce->st.st_mode));
	lua_setfield(L, -2, "is_link");

	lua_pushboolean(L, S_ISFIFO(sce->st.st_mode));
	lua_setfield(L, -2, "is_fifo");

	lua_pushinteger(L, sce->st.st_mtime);
	lua_setfield(L, -2, "st_mtime");

	lua_pushinteger(L, sce->st.st_ctime);
	lua_setfield(L, -2, "st_ctime");

	lua_pushinteger(L, sce->st.st_atime);
	lua_setfield(L, -2, "st_atime");

	lua_pushinteger(L, sce->st.st_uid);
	lua_setfield(L, -2, "st_uid");

	lua_pushinteger(L, sce->st.st_gid);
	lua_setfield(L, -2, "st_gid");

	lua_pushinteger(L, sce->st.st_size);
	lua_setfield(L, -2, "st_size");

	lua_pushinteger(L, sce->st.st_ino);
	lua_setfield(L, -2, "st_ino");

	if (!buffer_string_is_empty(sce->etag)) {
		/* we have to mutate the etag */
		buffer *b = buffer_init();
		etag_mutate(b, sce->etag);

		lua_pushlstring(L, CONST_BUF_LEN(b));
		buffer_free(b);
	} else {
		lua_pushnil(L);
	}
	lua_setfield(L, -2, "etag");

	if (!buffer_string_is_empty(sce->content_type)) {
		lua_pushlstring(L, CONST_BUF_LEN(sce->content_type));
	} else {
		lua_pushnil(L);
	}
	lua_setfield(L, -2, "content-type");

	return 1;
}


static int magnet_atpanic(lua_State *L) {
	buffer *b = magnet_checkbuffer(L, 1);

	log_error_write(magnet_get_server(L), __FILE__, __LINE__, "sB",
		"(lua-atpanic)",
		b);

	buffer_free(b);

	longjmp(exceptionjmp, 1);
}

static int magnet_reqhdr_get(lua_State *L) {
	connection *con = magnet_get_connection(L);
	data_string *ds;

	/* __index: param 1 is the (empty) table the value was not found in */
	const char *key = luaL_checkstring(L, 2);

	if (NULL != (ds = (data_string *)array_get_element(con->request.headers, key))) {
		if (!buffer_is_empty(ds->value)) {
			lua_pushlstring(L, CONST_BUF_LEN(ds->value));
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int magnet_reqhdr_pairs(lua_State *L) {
	connection *con = magnet_get_connection(L);

	return magnet_array_pairs(L, con->request.headers);
}

static int magnet_status_get(lua_State *L) {
	data_integer *di;
	server *srv = magnet_get_server(L);

	/* __index: param 1 is the (empty) table the value was not found in */
	const_buffer key = magnet_checkconstbuffer(L, 2);

	di = status_counter_get_counter(srv, key.ptr, key.len);

	lua_pushinteger(L, (lua_Integer)di->value);

	return 1;
}

static int magnet_status_set(lua_State *L) {
	server *srv = magnet_get_server(L);

	/* __newindex: param 1 is the (empty) table the value is supposed to be set in */
	const_buffer key = magnet_checkconstbuffer(L, 2);
	int counter = (int) luaL_checkinteger(L, 3);

	status_counter_set(srv, key.ptr, key.len, counter);

	return 0;
}

static int magnet_status_pairs(lua_State *L) {
	server *srv = magnet_get_server(L);

	return magnet_array_pairs(L, srv->status);
}

typedef struct {
	const char *name;
	enum {
		MAGNET_ENV_UNSET,

		MAGNET_ENV_PHYICAL_PATH,
		MAGNET_ENV_PHYICAL_REL_PATH,
		MAGNET_ENV_PHYICAL_DOC_ROOT,
		MAGNET_ENV_PHYICAL_BASEDIR,

		MAGNET_ENV_URI_PATH,
		MAGNET_ENV_URI_PATH_RAW,
		MAGNET_ENV_URI_SCHEME,
		MAGNET_ENV_URI_AUTHORITY,
		MAGNET_ENV_URI_QUERY,

		MAGNET_ENV_REQUEST_METHOD,
		MAGNET_ENV_REQUEST_URI,
		MAGNET_ENV_REQUEST_ORIG_URI,
		MAGNET_ENV_REQUEST_PATH_INFO,
		MAGNET_ENV_REQUEST_REMOTE_IP,
		MAGNET_ENV_REQUEST_PROTOCOL
	} type;
} magnet_env_t;

static const magnet_env_t magnet_env[] = {
	{ "physical.path", MAGNET_ENV_PHYICAL_PATH },
	{ "physical.rel-path", MAGNET_ENV_PHYICAL_REL_PATH },
	{ "physical.doc-root", MAGNET_ENV_PHYICAL_DOC_ROOT },
	{ "physical.basedir", MAGNET_ENV_PHYICAL_BASEDIR },

	{ "uri.path", MAGNET_ENV_URI_PATH },
	{ "uri.path-raw", MAGNET_ENV_URI_PATH_RAW },
	{ "uri.scheme", MAGNET_ENV_URI_SCHEME },
	{ "uri.authority", MAGNET_ENV_URI_AUTHORITY },
	{ "uri.query", MAGNET_ENV_URI_QUERY },

	{ "request.method", MAGNET_ENV_REQUEST_METHOD },
	{ "request.uri", MAGNET_ENV_REQUEST_URI },
	{ "request.orig-uri", MAGNET_ENV_REQUEST_ORIG_URI },
	{ "request.path-info", MAGNET_ENV_REQUEST_PATH_INFO },
	{ "request.remote-ip", MAGNET_ENV_REQUEST_REMOTE_IP },
	{ "request.protocol", MAGNET_ENV_REQUEST_PROTOCOL },

	{ NULL, MAGNET_ENV_UNSET }
};

static buffer *magnet_env_get_buffer_by_id(server *srv, connection *con, int id) {
	buffer *dest = NULL;

	UNUSED(srv);

	/**
	 * map all internal variables to lua
	 *
	 */

	switch (id) {
	case MAGNET_ENV_PHYICAL_PATH: dest = con->physical.path; break;
	case MAGNET_ENV_PHYICAL_REL_PATH: dest = con->physical.rel_path; break;
	case MAGNET_ENV_PHYICAL_DOC_ROOT: dest = con->physical.doc_root; break;
	case MAGNET_ENV_PHYICAL_BASEDIR: dest = con->physical.basedir; break;

	case MAGNET_ENV_URI_PATH: dest = con->uri.path; break;
	case MAGNET_ENV_URI_PATH_RAW: dest = con->uri.path_raw; break;
	case MAGNET_ENV_URI_SCHEME: dest = con->uri.scheme; break;
	case MAGNET_ENV_URI_AUTHORITY: dest = con->uri.authority; break;
	case MAGNET_ENV_URI_QUERY: dest = con->uri.query; break;

	case MAGNET_ENV_REQUEST_METHOD:
		buffer_copy_string(srv->tmp_buf, get_http_method_name(con->request.http_method));
		dest = srv->tmp_buf;
		break;
	case MAGNET_ENV_REQUEST_URI:      dest = con->request.uri; break;
	case MAGNET_ENV_REQUEST_ORIG_URI: dest = con->request.orig_uri; break;
	case MAGNET_ENV_REQUEST_PATH_INFO: dest = con->request.pathinfo; break;
	case MAGNET_ENV_REQUEST_REMOTE_IP: dest = con->dst_addr_buf; break;
	case MAGNET_ENV_REQUEST_PROTOCOL:
		buffer_copy_string(srv->tmp_buf, get_http_version_name(con->request.http_version));
		dest = srv->tmp_buf;
		break;

	case MAGNET_ENV_UNSET: break;
	}

	return dest;
}

static buffer *magnet_env_get_buffer(server *srv, connection *con, const char *key) {
	size_t i;

	for (i = 0; magnet_env[i].name; i++) {
		if (0 == strcmp(key, magnet_env[i].name)) break;
	}

	return magnet_env_get_buffer_by_id(srv, con, magnet_env[i].type);
}

static int magnet_env_get(lua_State *L) {
	server *srv = magnet_get_server(L);
	connection *con = magnet_get_connection(L);

	/* __index: param 1 is the (empty) table the value was not found in */
	const char *key = luaL_checkstring(L, 2);
	buffer *dest = NULL;

	dest = magnet_env_get_buffer(srv, con, key);

	if (!buffer_is_empty(dest)) {
		lua_pushlstring(L, CONST_BUF_LEN(dest));
	} else {
		lua_pushnil(L);
	}

	return 1;
}

static int magnet_env_set(lua_State *L) {
	server *srv = magnet_get_server(L);
	connection *con = magnet_get_connection(L);

	/* __newindex: param 1 is the (empty) table the value is supposed to be set in */
	const char *key = luaL_checkstring(L, 2);
	buffer *dest = NULL;

	luaL_checkany(L, 3); /* nil or a string */

	if (NULL != (dest = magnet_env_get_buffer(srv, con, key))) {
		if (lua_isnil(L, 3)) {
			buffer_reset(dest);
		} else {
			const_buffer val = magnet_checkconstbuffer(L, 3);
			buffer_copy_string_len(dest, val.ptr, val.len);
		}
	} else {
		/* couldn't save */

		return luaL_error(L, "couldn't store '%s' in lighty.env[]", key);
	}

	return 0;
}

static int magnet_env_next(lua_State *L) {
	server *srv = magnet_get_server(L);
	connection *con = magnet_get_connection(L);
	const int pos = lua_tointeger(L, lua_upvalueindex(1));

	buffer *dest;

	/* ignore previous key: use upvalue for current pos */
	lua_settop(L, 0);

	if (NULL == magnet_env[pos].name) return 0; /* end of list */
	/* Update our positional upval to reflect our new current position */
	lua_pushinteger(L, pos + 1);
	lua_replace(L, lua_upvalueindex(1));

	/* key to return */
	lua_pushstring(L, magnet_env[pos].name);

	/* get value */
	dest = magnet_env_get_buffer_by_id(srv, con, magnet_env[pos].type);
	if (!buffer_is_empty(dest)) {
		lua_pushlstring(L, CONST_BUF_LEN(dest));
	} else {
		lua_pushnil(L);
	}

	/* return 2 items on the stack (key, value) */
	return 2;
}

static int magnet_env_pairs(lua_State *L) {
	lua_pushinteger(L, 0); /* Push our current pos (the start) into upval 1 */
	lua_pushcclosure(L, magnet_env_next, 1); /* Push our new closure with 1 upvals */
	return 1;
}

static int magnet_cgi_get(lua_State *L) {
	connection *con = magnet_get_connection(L);
	data_string *ds;

	/* __index: param 1 is the (empty) table the value was not found in */
	const char *key = luaL_checkstring(L, 2);

	ds = (data_string *)array_get_element(con->environment, key);
	if (NULL != ds && !buffer_is_empty(ds->value))
		lua_pushlstring(L, CONST_BUF_LEN(ds->value));
	else
		lua_pushnil(L);

	return 1;
}

static int magnet_cgi_set(lua_State *L) {
	connection *con = magnet_get_connection(L);

	/* __newindex: param 1 is the (empty) table the value is supposed to be set in */
	const_buffer key = magnet_checkconstbuffer(L, 2);
	const_buffer val = magnet_checkconstbuffer(L, 3);

	array_set_key_value(con->environment, key.ptr, key.len, val.ptr, val.len);

	return 0;
}

static int magnet_cgi_pairs(lua_State *L) {
	connection *con = magnet_get_connection(L);

	return magnet_array_pairs(L, con->environment);
}


static int magnet_copy_response_header(server *srv, connection *con, lua_State *L, int lighty_table_ndx) {
	force_assert(lua_istable(L, lighty_table_ndx));

	lua_getfield(L, lighty_table_ndx, "header"); /* lighty.header */
	if (lua_istable(L, -1)) {
		/* header is found, and is a table */

		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_isstring(L, -1) && lua_isstring(L, -2)) {
				const_buffer key = magnet_checkconstbuffer(L, -2);
				const_buffer val = magnet_checkconstbuffer(L, -1);

				response_header_overwrite(srv, con, key.ptr, key.len, val.ptr, val.len);
			}

			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1); /* pop lighty.header */

	return 0;
}

/**
 * walk through the content array
 *
 * content = { "<pre>", { file = "/content" } , "</pre>" }
 *
 * header["Content-Type"] = "text/html"
 *
 * return 200
 */
static int magnet_attach_content(server *srv, connection *con, lua_State *L, int lighty_table_ndx) {
	force_assert(lua_istable(L, lighty_table_ndx));

	lua_getfield(L, lighty_table_ndx, "content"); /* lighty.content */
	if (lua_istable(L, -1)) {
		int i;
		/* content is found, and is a table */

		for (i = 1; ; i++) {
			lua_rawgeti(L, -1, i);

			/* -1 is the value and should be the value ... aka a table */
			if (lua_isstring(L, -1)) {
				const_buffer data = magnet_checkconstbuffer(L, -1);

				chunkqueue_append_mem(con->write_queue, data.ptr, data.len);
			} else if (lua_istable(L, -1)) {
				lua_getfield(L, -1, "filename");
				lua_getfield(L, -2, "length"); /* (0-based) end of range (not actually "length") */
				lua_getfield(L, -3, "offset"); /* (0-based) start of range */

				if (lua_isstring(L, -3)) { /* filename has to be a string */
					buffer *fn = magnet_checkbuffer(L, -3);
					off_t off = (off_t) luaL_optinteger(L, -1, 0);
					off_t len = (off_t) luaL_optinteger(L, -2, -1); /*(-1 to http_chunk_append_file_range() uses file size minus offset)*/
					if (off < 0) {
						buffer_free(fn);
						return luaL_error(L, "offset for '%s' is negative", lua_tostring(L, -3));
					}

					if (len >= off) {
						len -= off;
					} else if (-1 != len) {
						buffer_free(fn);
						return luaL_error(L, "offset > length for '%s'", lua_tostring(L, -3));
					}

					if (0 != len && 0 != http_chunk_append_file_range(srv, con, fn, off, len)) {
						buffer_free(fn);
						return luaL_error(L, "error opening file content '%s' at offset %lld", lua_tostring(L, -3), (long long)off);
					}

					buffer_free(fn);
				} else {
					return luaL_error(L, "content[%d] is a table and requires the field \"filename\"", i);
				}

				lua_pop(L, 3);
			} else if (lua_isnil(L, -1)) {
				/* end of list */

				lua_pop(L, 1);

				break;
			} else {
				return luaL_error(L, "content[%d] is neither a string nor a table: ", i);
			}

			lua_pop(L, 1); /* pop the content[...] entry value */
		}
	} else {
		return luaL_error(L, "lighty.content has to be a table");
	}
	lua_pop(L, 1); /* pop lighty.content */

	return 0;
}

static int traceback(lua_State *L) {
	if (!lua_isstring(L, 1))  /* 'message' not a string? */
		return 1;  /* keep it intact */
	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */
	return 1;
}

/* push traceback function before calling lua_pcall after narg arguments
 * have been pushed (inserts it before the arguments). returns index for
 * traceback function ("msgh" in lua_pcall)
 */
static int push_traceback(lua_State *L, int narg) {
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, traceback);
	lua_insert(L, base);
	return base;
}

static handler_t magnet_attract(server *srv, connection *con, plugin_data *p, buffer *name) {
	lua_State *L;
	int lua_return_value;
	const int func_ndx = 1;
	const int lighty_table_ndx = 2;

	/* get the script-context */
	L = script_cache_get_script(srv, con, p->cache, name);

	if (lua_isstring(L, -1)) {
		log_error_write(srv, __FILE__, __LINE__,
				"sbss",
				"loading script",
				name,
				"failed:",
				lua_tostring(L, -1));

		lua_pop(L, 1);

		force_assert(lua_gettop(L) == 0); /* only the error should have been on the stack */

		con->http_status = 500;
		con->mode = DIRECT;

		return HANDLER_FINISHED;
	}

	force_assert(lua_gettop(L) == 1);
	force_assert(lua_isfunction(L, func_ndx));

	lua_pushlightuserdata(L, srv);
	lua_setfield(L, LUA_REGISTRYINDEX, LUA_RIDX_LIGHTTPD_SERVER);

	lua_pushlightuserdata(L, con);
	lua_setfield(L, LUA_REGISTRYINDEX, LUA_RIDX_LIGHTTPD_CONNECTION);

	lua_atpanic(L, magnet_atpanic);

	/**
	 * we want to create empty environment for our script
	 *
	 * setmetatable({}, {__index = _G})
	 *
	 * if a function symbol is not defined in our env, __index will lookup
	 * in the global env.
	 *
	 * all variables created in the script-env will be thrown
	 * away at the end of the script run.
	 */
	lua_newtable(L); /* my empty environment aka {}              (sp += 1) */

	/* we have to overwrite the print function */
	lua_pushcfunction(L, magnet_print);                       /* (sp += 1) */
	lua_setfield(L, -2, "print"); /* -1 is the env we want to set(sp -= 1) */

	/**
	 * lighty.request[] (ro) has the HTTP-request headers
	 * lighty.env[] (rw) has various url/physical file paths and
	 *                   request meta data; might contain nil values
	 * lighty.req_env[] (ro) has the cgi environment
	 * lighty.status[] (ro) has the status counters
	 * lighty.content[] (rw) is a table of string/file
	 * lighty.header[] (rw) is a array to set response headers
	 */

	lua_newtable(L); /* lighty.*                                 (sp += 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the request-table     (sp += 1) */
	lua_pushcfunction(L, magnet_reqhdr_get);                  /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_reqhdr_pairs);                /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to request     (sp -= 1) */
	lua_setfield(L, -2, "request"); /* content = {}              (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the env-table         (sp += 1) */
	lua_pushcfunction(L, magnet_env_get);                     /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_env_set);                     /* (sp += 1) */
	lua_setfield(L, -2, "__newindex");                        /* (sp -= 1) */
	lua_pushcfunction(L, magnet_env_pairs);                   /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to env         (sp -= 1) */
	lua_setfield(L, -2, "env"); /* content = {}                  (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the req_env-table     (sp += 1) */
	lua_pushcfunction(L, magnet_cgi_get);                     /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_cgi_set);                     /* (sp += 1) */
	lua_setfield(L, -2, "__newindex");                        /* (sp -= 1) */
	lua_pushcfunction(L, magnet_cgi_pairs);                   /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to req_env     (sp -= 1) */
	lua_setfield(L, -2, "req_env"); /* content = {}              (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the status-table      (sp += 1) */
	lua_pushcfunction(L, magnet_status_get);                  /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_status_set);                  /* (sp += 1) */
	lua_setfield(L, -2, "__newindex");                        /* (sp -= 1) */
	lua_pushcfunction(L, magnet_status_pairs);                /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to statzs      (sp -= 1) */
	lua_setfield(L, -2, "status"); /* content = {}               (sp -= 1) */

	/* add empty 'content' and 'header' tables */
	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_setfield(L, -2, "content"); /* content = {}              (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_setfield(L, -2, "header"); /* header = {}                (sp -= 1) */

	lua_pushinteger(L, MAGNET_RESTART_REQUEST);
	lua_setfield(L, -2, "RESTART_REQUEST");

	lua_pushcfunction(L, magnet_stat);                        /* (sp += 1) */
	lua_setfield(L, -2, "stat"); /* -1 is the env we want to set (sp -= 1) */

	/* insert lighty table at index 2 */
	lua_pushvalue(L, -1);
	lua_insert(L, lighty_table_ndx);

	lua_setfield(L, -2, "lighty"); /* lighty.*                   (sp -= 1) */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
	/* override the default pairs() function to our __pairs capable version;
	 * not needed for lua 5.2+
	 */
	lua_getglobal(L, "pairs"); /* push original pairs()          (sp += 1) */
	lua_pushcclosure(L, magnet_pairs, 1);
	lua_setfield(L, -2, "pairs");                             /* (sp -= 1) */
#endif

	lua_newtable(L); /* the meta-table for the new env           (sp += 1) */
	lua_pushglobaltable(L);                                   /* (sp += 1) */
	lua_setfield(L, -2, "__index"); /* { __index = _G }          (sp -= 1) */
	lua_setmetatable(L, -2); /* setmetatable({}, {__index = _G}) (sp -= 1) */

	magnet_setfenv_mainfn(L, 1);                              /* (sp -= 1) */

	/* pcall will destroy the func value, duplicate it */     /* (sp += 1) */
	lua_pushvalue(L, func_ndx);
	{
		int errfunc = push_traceback(L, 0);
		int ret = lua_pcall(L, 0, 1, errfunc);
		lua_remove(L, errfunc);

		/* reset environment */
		lua_pushglobaltable(L);                               /* (sp += 1) */
		magnet_setfenv_mainfn(L, 1);                          /* (sp -= 1) */

		if (0 != ret) {
			log_error_write(srv, __FILE__, __LINE__,
				"ss",
				"lua_pcall():",
				lua_tostring(L, -1));
			lua_pop(L, 2); /* remove the error-msg and the lighty table at index 2 */

			force_assert(lua_gettop(L) == 1); /* only the function should be on the stack */

			con->http_status = 500;
			con->mode = DIRECT;

			return HANDLER_FINISHED;
		}
	}

	/* we should have the function, the lighty table and the return value on the stack */
	force_assert(lua_gettop(L) == 3);

	lua_return_value = (int) luaL_optinteger(L, -1, -1);
	lua_pop(L, 1); /* pop return value */

	magnet_copy_response_header(srv, con, L, lighty_table_ndx);

	{
		handler_t result = HANDLER_GO_ON;

		if (lua_return_value > 99) {
			con->http_status = lua_return_value;
			con->file_finished = 1;

			/* try { ...*/
			if (0 == setjmp(exceptionjmp)) {
				magnet_attach_content(srv, con, L, lighty_table_ndx);
				if (!chunkqueue_is_empty(con->write_queue)) {
					con->mode = p->id;
				}
			} else {
				lua_settop(L, 2); /* remove all but function and lighty table */
				/* } catch () { */
				con->http_status = 500;
				con->mode = DIRECT;
			}

			result = HANDLER_FINISHED;
		} else if (MAGNET_RESTART_REQUEST == lua_return_value) {
			result = HANDLER_COMEBACK;
		}

		lua_pop(L, 1); /* pop the lighty table */
		force_assert(lua_gettop(L) == 1); /* only the function should remain on the stack */

		return result;
	}
}

static handler_t magnet_attract_array(server *srv, connection *con, plugin_data *p, array *files) {
	size_t i;
	handler_t ret = HANDLER_GO_ON;

	/* no filename set */
	if (files->used == 0) return HANDLER_GO_ON;

      #ifdef USE_OPENSSL
	if (con->ssl) http_cgi_ssl_env(srv, con);
      #endif

	/**
	 * execute all files and jump out on the first !HANDLER_GO_ON
	 */
	for (i = 0; i < files->used && ret == HANDLER_GO_ON; i++) {
		data_string *ds = (data_string *)files->data[i];

		if (buffer_string_is_empty(ds->value)) continue;

		ret = magnet_attract(srv, con, p, ds->value);
	}

	if (con->error_handler_saved_status) {
		/* retrieve (possibly modified) REDIRECT_STATUS and store as number */
		unsigned long x;
		data_string * const ds = (data_string *)array_get_element(con->environment, "REDIRECT_STATUS");
		if (ds && (x = strtoul(ds->value->ptr, NULL, 10)) < 1000)
			/*(simplified validity check x < 1000)*/
			con->error_handler_saved_status =
			  con->error_handler_saved_status > 0 ? (int)x : -(int)x;
	}

	return ret;
}

URIHANDLER_FUNC(mod_magnet_uri_handler) {
	plugin_data *p = p_d;

	mod_magnet_patch_connection(srv, con, p);

	return magnet_attract_array(srv, con, p, p->conf.url_raw);
}

URIHANDLER_FUNC(mod_magnet_physical) {
	plugin_data *p = p_d;

	mod_magnet_patch_connection(srv, con, p);

	return magnet_attract_array(srv, con, p, p->conf.physical_path);
}


/* this function is called at dlopen() time and inits the callbacks */

int mod_magnet_plugin_init(plugin *p);
int mod_magnet_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("magnet");

	p->init        = mod_magnet_init;
	p->handle_uri_clean  = mod_magnet_uri_handler;
	p->handle_physical   = mod_magnet_physical;
	p->set_defaults  = mod_magnet_set_defaults;
	p->cleanup     = mod_magnet_free;

	p->data        = NULL;

	return 0;
}
