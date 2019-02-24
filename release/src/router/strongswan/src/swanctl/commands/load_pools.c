/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include "command.h"
#include "swanctl.h"
#include "load_pools.h"

/**
 * Add a vici list from a comma separated string value
 */
static void add_list_key(vici_req_t *req, char *key, char *value)
{
	enumerator_t *enumerator;
	char *token;

	vici_begin_list(req, key);
	enumerator = enumerator_create_token(value, ",", " ");
	while (enumerator->enumerate(enumerator, &token))
	{
		vici_add_list_itemf(req, "%s", token);
	}
	enumerator->destroy(enumerator);
	vici_end_list(req);
}

/**
 * Translate setting key/values from a section enumerator into vici
 * key-values/lists. Destroys the enumerator.
 */
static void add_key_values(vici_req_t *req, enumerator_t *enumerator)
{
	char *key, *value;

	while (enumerator->enumerate(enumerator, &key, &value))
	{
		/* pool subnet is encoded as key/value, all other attributes as list */
		if (streq(key, "addrs"))
		{
			vici_add_key_valuef(req, key, "%s", value);
		}
		else
		{
			add_list_key(req, key, value);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Load a pool configuration
 */
static bool load_pool(vici_conn_t *conn, settings_t *cfg,
					  char *section, command_format_options_t format)
{
	enumerator_t *enumerator;
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;

	req = vici_begin("load-pool");

	vici_begin_section(req, section);
	enumerator = cfg->create_key_value_enumerator(cfg, "pools.%s", section);
	add_key_values(req, enumerator);
	vici_end_section(req);

	res = vici_submit(req, conn);
	if (!res)
	{
		fprintf(stderr, "load-pool request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "load-pool reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "loading pool '%s' failed: %s\n",
				section, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	else
	{
		printf("loaded pool '%s'\n", section);
	}
	vici_free_res(res);
	return ret;
}

CALLBACK(list_pool, int,
	linked_list_t *list, vici_res_t *res, char *name)
{
	list->insert_last(list, strdup(name));
	return 0;
}

/**
 * Create a list of currently loaded pools
 */
static linked_list_t* list_pools(vici_conn_t *conn,
								 command_format_options_t format)
{
	linked_list_t *list;
	vici_res_t *res;

	list = linked_list_create();

	res = vici_submit(vici_begin("get-pools"), conn);
	if (res)
	{
		if (format & COMMAND_FORMAT_RAW)
		{
			vici_dump(res, "get-pools reply", format & COMMAND_FORMAT_PRETTY,
					  stdout);
		}
		vici_parse_cb(res, list_pool, NULL, NULL, list);
		vici_free_res(res);
	}
	return list;
}

/**
 * Remove and free a string from a list
 */
static void remove_from_list(linked_list_t *list, char *str)
{
	enumerator_t *enumerator;
	char *current;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (streq(current, str))
		{
			list->remove_at(list, enumerator);
			free(current);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Unload a pool by name
 */
static bool unload_pool(vici_conn_t *conn, char *name,
						command_format_options_t format)
{
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;

	req = vici_begin("unload-pool");
	vici_add_key_valuef(req, "name", "%s", name);
	res = vici_submit(req, conn);
	if (!res)
	{
		fprintf(stderr, "unload-pool request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "unload-pool reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "unloading pool '%s' failed: %s\n",
				name, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	vici_free_res(res);
	return ret;
}

/**
 * See header.
 */
int load_pools_cfg(vici_conn_t *conn, command_format_options_t format,
				   settings_t *cfg)
{
	u_int found = 0, loaded = 0, unloaded = 0;
	char *section;
	enumerator_t *enumerator;
	linked_list_t *pools;

	pools = list_pools(conn, format);

	enumerator = cfg->create_section_enumerator(cfg, "pools");
	while (enumerator->enumerate(enumerator, &section))
	{
		remove_from_list(pools, section);
		found++;
		if (load_pool(conn, cfg, section, format))
		{
			loaded++;
		}
	}
	enumerator->destroy(enumerator);

	/* unload all pools in daemon, but not in file */
	while (pools->remove_first(pools, (void**)&section) == SUCCESS)
	{
		if (unload_pool(conn, section, format))
		{
			unloaded++;
		}
		free(section);
	}
	pools->destroy(pools);

	if (format & COMMAND_FORMAT_RAW)
	{
		return 0;
	}
	if (found == 0)
	{
		fprintf(stderr, "no pools found, %u unloaded\n", unloaded);
		return 0;
	}
	if (loaded == found)
	{
		printf("successfully loaded %u pools, %u unloaded\n",
			   loaded, unloaded);
		return 0;
	}
	fprintf(stderr, "loaded %u of %u pools, %u failed to load, "
			"%u unloaded\n", loaded, found, found - loaded, unloaded);
	return EINVAL;
}

static int load_pools(vici_conn_t *conn)
{
	command_format_options_t format = COMMAND_FORMAT_NONE;
	settings_t *cfg;
	char *arg, *file = NULL;
	int ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'P':
				format |= COMMAND_FORMAT_PRETTY;
				/* fall through to raw */
			case 'r':
				format |= COMMAND_FORMAT_RAW;
				continue;
			case 'f':
				file = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --load-pools option");
		}
		break;
	}

	cfg = load_swanctl_conf(file);
	if (!cfg)
	{
		return EINVAL;
	}

	ret = load_pools_cfg(conn, format, cfg);

	cfg->destroy(cfg);

	return ret;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		load_pools, 'a', "load-pools", "(re-)load pool configuration",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
			{"file",		'f', 1, "custom path to swanctl.conf"},
		}
	});
}
