#ifndef _MOD_CACHE_H_
#define _MOD_CACHE_H_
#include "first.h"

#include "buffer.h"
#include "server.h"
#include "response.h"

#include "plugin.h"

#if defined(USE_MEMCACHED)
#include <libmemcached/memcached.h>
#endif

#define plugin_data mod_cache_plugin_data

typedef struct {
	buffer *ext;

	array  *mc_hosts;
	buffer *mc_namespace;
#if defined(USE_MEMCACHED)
	memcached_st *memc;
#endif
	buffer *power_magnet;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	buffer *basedir;
	buffer *baseurl;

	buffer *trigger_handler;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

int cache_parse_lua(server *srv, connection *con, plugin_data *p, buffer *fn);

#endif
