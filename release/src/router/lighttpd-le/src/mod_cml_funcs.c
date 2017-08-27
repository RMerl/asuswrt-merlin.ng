#include "first.h"

#include <sys/stat.h>
#include <time.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>

#include <lauxlib.h>

#include "mod_cml_funcs.h"
#include "mod_cml.h"

#include "buffer.h"
#include "server.h"
#include "log.h"
#include "plugin.h"
#include "response.h"

#include "md5.h"

#define HASHLEN 16
typedef unsigned char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];

int f_crypto_md5(lua_State *L) {
	li_MD5_CTX Md5Ctx;
	HASH HA1;
	char hex[33];
	int n = lua_gettop(L);
	size_t s_len;
	const char *s;

	if (n != 1) {
		lua_pushstring(L, "md5: expected one argument");
		lua_error(L);
	}

	if (!lua_isstring(L, 1)) {
		lua_pushstring(L, "md5: argument has to be a string");
		lua_error(L);
	}

	s = lua_tolstring(L, 1, &s_len);

	li_MD5_Init(&Md5Ctx);
	li_MD5_Update(&Md5Ctx, (unsigned char *) s, (unsigned int) s_len);
	li_MD5_Final(HA1, &Md5Ctx);

	li_tohex(hex, sizeof(hex), (const char*) HA1, 16);

	lua_pushstring(L, hex);

	return 1;
}


int f_file_mtime(lua_State *L) {
	struct stat st;
	int n = lua_gettop(L);

	if (n != 1) {
		lua_pushstring(L, "file_mtime: expected one argument");
		lua_error(L);
	}

	if (!lua_isstring(L, 1)) {
		lua_pushstring(L, "file_mtime: argument has to be a string");
		lua_error(L);
	}

	if (-1 == stat(lua_tostring(L, 1), &st)) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, st.st_mtime);

	return 1;
}

static int f_dir_files_iter(lua_State *L) {
	DIR *d;
	struct dirent *de;

	d = lua_touserdata(L, lua_upvalueindex(1));

	if (NULL == (de = readdir(d))) {
		/* EOF */
		closedir(d);

		return 0;
	} else {
		lua_pushstring(L, de->d_name);
		return 1;
	}
}

int f_dir_files(lua_State *L) {
	DIR *d;
	int n = lua_gettop(L);

	if (n != 1) {
		lua_pushstring(L, "dir_files: expected one argument");
		lua_error(L);
	}

	if (!lua_isstring(L, 1)) {
		lua_pushstring(L, "dir_files: argument has to be a string");
		lua_error(L);
	}

	/* check if there is a valid DIR handle on the stack */
	if (NULL == (d = opendir(lua_tostring(L, 1)))) {
		lua_pushnil(L);
		return 1;
	}

	/* push d into userdata */
	lua_pushlightuserdata(L, d);
	lua_pushcclosure(L, f_dir_files_iter, 1);

	return 1;
}

int f_file_isreg(lua_State *L) {
	struct stat st;
	int n = lua_gettop(L);

	if (n != 1) {
		lua_pushstring(L, "file_isreg: expected one argument");
		lua_error(L);
	}

	if (!lua_isstring(L, 1)) {
		lua_pushstring(L, "file_isreg: argument has to be a string");
		lua_error(L);
	}

	if (-1 == stat(lua_tostring(L, 1), &st)) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, S_ISREG(st.st_mode));

	return 1;
}

int f_file_isdir(lua_State *L) {
	struct stat st;
	int n = lua_gettop(L);

	if (n != 1) {
		lua_pushstring(L, "file_isreg: expected one argument");
		lua_error(L);
	}

	if (!lua_isstring(L, 1)) {
		lua_pushstring(L, "file_isreg: argument has to be a string");
		lua_error(L);
	}

	if (-1 == stat(lua_tostring(L, 1), &st)) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, S_ISDIR(st.st_mode));

	return 1;
}



#ifdef USE_MEMCACHED
int f_memcache_exists(lua_State *L) {
	size_t key_len;
	const char *key;
	memcached_st *memc;

	if (!lua_islightuserdata(L, lua_upvalueindex(1))) {
		lua_pushstring(L, "where is my userdata ?");
		lua_error(L);
	}

	memc = lua_touserdata(L, lua_upvalueindex(1));

	if (1 != lua_gettop(L)) {
		lua_pushstring(L, "expected one argument");
		lua_error(L);
	}

	key = luaL_checklstring(L, 1, &key_len);
	lua_pushboolean(L, (MEMCACHED_SUCCESS == memcached_exist(memc, key, key_len)));
	return 1;
}

int f_memcache_get_string(lua_State *L) {
	size_t key_len, value_len;
	char *value;
	const char *key;
	memcached_st *memc;

	if (!lua_islightuserdata(L, lua_upvalueindex(1))) {
		lua_pushstring(L, "where is my userdata ?");
		lua_error(L);
	}

	memc = lua_touserdata(L, lua_upvalueindex(1));

	if (1 != lua_gettop(L)) {
		lua_pushstring(L, "expected one argument");
		lua_error(L);
	}

	key = luaL_checklstring(L, 1, &key_len);
	if (NULL == (value = memcached_get(memc, key, key_len, &value_len, NULL, NULL))) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushlstring(L, value, value_len);

	free(value);

	return 1;
}

int f_memcache_get_long(lua_State *L) {
	size_t key_len, value_len;
	char *value;
	const char *key;
	memcached_st *memc;
	char *endptr;
	long v;

	if (!lua_islightuserdata(L, lua_upvalueindex(1))) {
		lua_pushstring(L, "where is my userdata ?");
		lua_error(L);
	}

	memc = lua_touserdata(L, lua_upvalueindex(1));

	if (1 != lua_gettop(L)) {
		lua_pushstring(L, "expected one argument");
		lua_error(L);
	}

	key = luaL_checklstring(L, 1, &key_len);
	if (NULL == (value = memcached_get(memc, key, key_len, &value_len, NULL, NULL))) {
		lua_pushnil(L);
		return 1;
	}

	errno = 0;
	v = strtol(value, &endptr, 10);
	if (0 == errno && *endptr == '\0') {
		lua_pushinteger(L, v);
	} else {
		lua_pushnil(L);
	}

	free(value);

	return 1;
}
#endif
