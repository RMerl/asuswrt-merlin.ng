#ifndef _MOD_SSI_H_
#define _MOD_SSI_H_
#include "first.h"

#include "base.h"
#include "buffer.h"
#include "array.h"

#include "plugin.h"

/* plugin config for all request/connections */

typedef struct {
	array *ssi_extension;
	buffer *content_type;
	unsigned short conditional_requests;
	unsigned short ssi_exec;
	unsigned short ssi_recursion_max;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	buffer *timefmt;

	buffer *stat_fn;

	array *ssi_vars;
	array *ssi_cgi_env;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

typedef struct {
	buffer *timefmt;
	int sizefmt;

	buffer *stat_fn;

	array *ssi_vars;
	array *ssi_cgi_env;

	int if_level, if_is_false_level, if_is_false, if_is_false_endif;
	unsigned short ssi_recursion_depth;

	plugin_config conf;
} handler_ctx;

int ssi_eval_expr(server *srv, connection *con, handler_ctx *p, const char *expr);

#endif
