#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_
#include "first.h"

#include "array.h"
#include "buffer.h"
#include "server.h"

typedef struct {
	server *srv;
	int     ok;
	array  *all_configs;
	vector_config_weak configs_stack; /* to parse nested block */
	data_config *current; /* current started with { */
	buffer *basedir;
} config_t;

void *configparserAlloc(void *(*mallocProc)(size_t));
void configparserFree(void *p, void (*freeProc)(void*));
void configparser(void *yyp, int yymajor, buffer *yyminor, config_t *ctx);
int config_parse_file(server *srv, config_t *context, const char *fn);
int config_parse_cmd(server *srv, config_t *context, const char *cmd);
data_unset *configparser_merge_data(data_unset *op1, const data_unset *op2);

int config_setup_connection(server *srv, connection *con);
int config_patch_connection(server *srv, connection *con);

void config_cond_cache_reset(server *srv, connection *con);
void config_cond_cache_reset_item(server *srv, connection *con, comp_key_t item);

#endif
