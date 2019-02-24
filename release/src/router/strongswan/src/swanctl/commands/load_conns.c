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
#include "load_conns.h"

/**
 * Check if we should handle a key as a list of comma separated values
 */
static bool is_list_key(char *key)
{
	char *keys[] = {
		"local_addrs",
		"remote_addrs",
		"proposals",
		"esp_proposals",
		"ah_proposals",
		"local_ts",
		"remote_ts",
		"vips",
		"pools",
		"groups",
		"cert_policy",
	};
	int i;

	for (i = 0; i < countof(keys); i++)
	{
		if (strcaseeq(keys[i], key))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Check if we should handle a key as a list of comma separated files
 */
static bool is_file_list_key(char *key)
{
	char *keys[] = {
		"certs",
		"cacerts",
		"pubkeys"
	};
	int i;

	for (i = 0; i < countof(keys); i++)
	{
		if (strcaseeq(keys[i], key))
		{
			return TRUE;
		}
	}
	return FALSE;
}

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
 * Add a vici list of blobs from a comma separated file list
 */
static bool add_file_list_key(vici_req_t *req, char *key, char *value)
{
	enumerator_t *enumerator;
	chunk_t *map, blob;
	char *token, buf[PATH_MAX];
	bool ret = TRUE;

	vici_begin_list(req, key);
	enumerator = enumerator_create_token(value, ",", " ");
	while (enumerator->enumerate(enumerator, &token))
	{
		if (strcasepfx(token, "0x") || strcasepfx(token, "0s"))
		{
			blob = chunk_from_str(token + 2);
			blob = strcasepfx(token, "0x") ? chunk_from_hex(blob, NULL)
										   : chunk_from_base64(blob, NULL);
			vici_add_list_item(req, blob.ptr, blob.len);
			chunk_free(&blob);
		}
		else
		{
			if (!path_absolute(token))
			{
				if (streq(key, "certs"))
				{
					snprintf(buf, sizeof(buf), "%s%s%s%s%s", swanctl_dir,
							 DIRECTORY_SEPARATOR, SWANCTL_X509DIR,
							 DIRECTORY_SEPARATOR, token);
					token = buf;
				}
				else if (streq(key, "cacerts"))
				{
					snprintf(buf, sizeof(buf), "%s%s%s%s%s", swanctl_dir,
							 DIRECTORY_SEPARATOR, SWANCTL_X509CADIR,
							 DIRECTORY_SEPARATOR, token);
					token = buf;
				}
				else if (streq(key, "pubkeys"))
				{
					snprintf(buf, sizeof(buf), "%s%s%s%s%s", swanctl_dir,
							 DIRECTORY_SEPARATOR, SWANCTL_PUBKEYDIR,
							 DIRECTORY_SEPARATOR, token);
					token = buf;
				}
			}
			map = chunk_map(token, FALSE);
			if (map)
			{
				vici_add_list_item(req, map->ptr, map->len);
				chunk_unmap(map);
			}
			else
			{
				fprintf(stderr, "loading %s certificate '%s' failed: %s\n",
						key, token, strerror(errno));
				ret = FALSE;
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	vici_end_list(req);

	return ret;
}

/**
 * Translate setting key/values from a section into vici key-values/lists
 */
static bool add_key_values(vici_req_t *req, settings_t *cfg, char *section)
{
	enumerator_t *enumerator;
	char *key, *value;
	bool ret = TRUE;

	enumerator = cfg->create_key_value_enumerator(cfg, section);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		if (is_list_key(key))
		{
			add_list_key(req, key, value);
		}
		else if (is_file_list_key(key))
		{
			ret = add_file_list_key(req, key, value);
		}
		else
		{
			vici_add_key_valuef(req, key, "%s", value);
		}
		if (!ret)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	return ret;
}

/**
 * Translate a settings section to a vici section
 */
static bool add_sections(vici_req_t *req, settings_t *cfg, char *section)
{
	enumerator_t *enumerator;
	char *name, buf[256];
	bool ret = TRUE;

	enumerator = cfg->create_section_enumerator(cfg, section);
	while (enumerator->enumerate(enumerator, &name))
	{
		vici_begin_section(req, name);
		snprintf(buf, sizeof(buf), "%s.%s", section, name);
		ret = add_key_values(req, cfg, buf);
		if (!ret)
		{
			break;
		}
		ret = add_sections(req, cfg, buf);
		if (!ret)
		{
			break;
		}
		vici_end_section(req);
	}
	enumerator->destroy(enumerator);

	return ret;
}

/**
 * Load an IKE_SA config with CHILD_SA configs from a section
 */
static bool load_conn(vici_conn_t *conn, settings_t *cfg,
					  char *section, command_format_options_t format)
{
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;
	char buf[BUF_LEN];

	snprintf(buf, sizeof(buf), "%s.%s", "connections", section);

	req = vici_begin("load-conn");

	vici_begin_section(req, section);
	if (!add_key_values(req, cfg, buf) ||
		!add_sections(req, cfg, buf))
	{
		vici_free_req(req);
		return FALSE;
	}
	vici_end_section(req);

	res = vici_submit(req, conn);
	if (!res)
	{
		fprintf(stderr, "load-conn request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "load-conn reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "loading connection '%s' failed: %s\n",
				section, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	else
	{
		printf("loaded connection '%s'\n", section);
	}
	vici_free_res(res);
	return ret;
}

CALLBACK(list_conn, int,
	linked_list_t *list, vici_res_t *res, char *name, void *value, int len)
{
	if (streq(name, "conns"))
	{
		char *str;

		if (asprintf(&str, "%.*s", len, value) != -1)
		{
			list->insert_last(list, str);
		}
	}
	return 0;
}

/**
 * Create a list of currently loaded connections
 */
static linked_list_t* list_conns(vici_conn_t *conn,
								 command_format_options_t format)
{
	linked_list_t *list;
	vici_res_t *res;

	list = linked_list_create();

	res = vici_submit(vici_begin("get-conns"), conn);
	if (res)
	{
		if (format & COMMAND_FORMAT_RAW)
		{
			vici_dump(res, "get-conns reply", format & COMMAND_FORMAT_PRETTY,
					  stdout);
		}
		vici_parse_cb(res, NULL, NULL, list_conn, list);
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
 * Unload a connection by name
 */
static bool unload_conn(vici_conn_t *conn, char *name,
					    command_format_options_t format)
{
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;

	req = vici_begin("unload-conn");
	vici_add_key_valuef(req, "name", "%s", name);
	res = vici_submit(req, conn);
	if (!res)
	{
		fprintf(stderr, "unload-conn request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "unload-conn reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "unloading connection '%s' failed: %s\n",
				name, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	vici_free_res(res);
	return ret;
}

/**
 * See header.
 */
int load_conns_cfg(vici_conn_t *conn, command_format_options_t format,
				   settings_t *cfg)
{
	u_int found = 0, loaded = 0, unloaded = 0;
	char *section;
	enumerator_t *enumerator;
	linked_list_t *conns;

	conns = list_conns(conn, format);

	enumerator = cfg->create_section_enumerator(cfg, "connections");
	while (enumerator->enumerate(enumerator, &section))
	{
		remove_from_list(conns, section);
		found++;
		if (load_conn(conn, cfg, section, format))
		{
			loaded++;
		}
	}
	enumerator->destroy(enumerator);

	/* unload all connection in daemon, but not in file */
	while (conns->remove_first(conns, (void**)&section) == SUCCESS)
	{
		if (unload_conn(conn, section, format))
		{
			unloaded++;
		}
		free(section);
	}
	conns->destroy(conns);

	if (format & COMMAND_FORMAT_RAW)
	{
		return 0;
	}
	if (found == 0)
	{
		fprintf(stderr, "no connections found, %u unloaded\n", unloaded);
		return 0;
	}
	if (loaded == found)
	{
		printf("successfully loaded %u connections, %u unloaded\n",
			   loaded, unloaded);
		return 0;
	}
	fprintf(stderr, "loaded %u of %u connections, %u failed to load, "
			"%u unloaded\n", loaded, found, found - loaded, unloaded);
	return EINVAL;
}

static int load_conns(vici_conn_t *conn)
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
				return command_usage("invalid --load-conns option");
		}
		break;
	}

	cfg = load_swanctl_conf(file);
	if (!cfg)
	{
		return EINVAL;
	}

	ret = load_conns_cfg(conn, format, cfg);

	cfg->destroy(cfg);

	return ret;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		load_conns, 'c', "load-conns", "(re-)load connection configuration",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
			{"file",		'f', 1, "custom path to swanctl.conf"},
		}
	});
}
