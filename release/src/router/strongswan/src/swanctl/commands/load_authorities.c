/*
 * Copyright (C) 2015 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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
#include "load_authorities.h"

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
 * Add a vici certificate blob value given by its file patch
 */
static bool add_file_key_value(vici_req_t *req, char *key, char *value)
{
	chunk_t *map;
	char *path, buf[PATH_MAX];

	if (path_absolute(value))
	{
		path = value;
	}
	else
	{
		path = buf;
		snprintf(path, PATH_MAX, "%s%s%s%s%s", swanctl_dir,
				 DIRECTORY_SEPARATOR, SWANCTL_X509CADIR,
				 DIRECTORY_SEPARATOR, value);
	}
	map = chunk_map(path, FALSE);

	if (map)
	{
		vici_add_key_value(req, key, map->ptr, map->len);
		chunk_unmap(map);
		return TRUE;
	}
	else
	{
		fprintf(stderr, "loading ca certificate '%s' failed: %s\n",
				path, strerror(errno));
		return FALSE;
	}
}

/**
 * Translate sletting key/values from a section enumerator into vici
 * key-values/lists. Destroys the enumerator.
 */
static bool add_key_values(vici_req_t *req, enumerator_t *enumerator)
{
	char *key, *value;
	bool ret = TRUE;

	while (enumerator->enumerate(enumerator, &key, &value))
	{
		if (streq(key, "cacert"))
		{
			ret = add_file_key_value(req, key, value);
		}
		else if (streq(key, "crl_uris") ||
				 streq(key, "ocsp_uris"))
		{
			add_list_key(req, key, value);
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
 * Load an authority configuration
 */
static bool load_authority(vici_conn_t *conn, settings_t *cfg,
						   char *section, command_format_options_t format)
{
	enumerator_t *enumerator;
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;

	req = vici_begin("load-authority");

	vici_begin_section(req, section);
	enumerator = cfg->create_key_value_enumerator(cfg, "authorities.%s",
												  section);
	if (!add_key_values(req, enumerator))
	{
		vici_free_req(req);
		return FALSE;
	}
	vici_end_section(req);

	res = vici_submit(req, conn);
	if (!res)
	{
		fprintf(stderr, "load-authority request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "load-authority reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "loading authority '%s' failed: %s\n",
				section, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	else
	{
		printf("loaded authority '%s'\n", section);
	}
	vici_free_res(res);
	return ret;
}

CALLBACK(list_authority, int,
	linked_list_t *list, vici_res_t *res, char *name, void *value, int len)
{
	if (streq(name, "authorities"))
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
 * Create a list of currently loaded authorities
 */
static linked_list_t* list_authorities(vici_conn_t *conn,
									   command_format_options_t format)
{
	linked_list_t *list;
	vici_res_t *res;

	list = linked_list_create();

	res = vici_submit(vici_begin("get-authorities"), conn);
	if (res)
	{
		if (format & COMMAND_FORMAT_RAW)
		{
			vici_dump(res, "get-authorities reply", format & COMMAND_FORMAT_PRETTY,
					  stdout);
		}
		vici_parse_cb(res, NULL, NULL, list_authority, list);
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
 * Unload a authority by name
 */
static bool unload_authority(vici_conn_t *conn, char *name,
							 command_format_options_t format)
{
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;

	req = vici_begin("unload-authority");
	vici_add_key_valuef(req, "name", "%s", name);
	res = vici_submit(req, conn);
	if (!res)
	{
		fprintf(stderr, "unload-authority request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "unload-authority reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "unloading authority '%s' failed: %s\n",
				name, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	vici_free_res(res);
	return ret;
}

/**
 * See header.
 */
int load_authorities_cfg(vici_conn_t *conn, command_format_options_t format,
						 settings_t *cfg)
{
	u_int found = 0, loaded = 0, unloaded = 0;
	char *section;
	enumerator_t *enumerator;
	linked_list_t *authorities;

	authorities = list_authorities(conn, format);

	enumerator = cfg->create_section_enumerator(cfg, "authorities");
	while (enumerator->enumerate(enumerator, &section))
	{
		remove_from_list(authorities, section);
		found++;
		if (load_authority(conn, cfg, section, format))
		{
			loaded++;
		}
	}
	enumerator->destroy(enumerator);

	/* unload all authorities in daemon, but not in file */
	while (authorities->remove_first(authorities, (void**)&section) == SUCCESS)
	{
		if (unload_authority(conn, section, format))
		{
			unloaded++;
		}
		free(section);
	}
	authorities->destroy(authorities);

	if (format & COMMAND_FORMAT_RAW)
	{
		return 0;
	}
	if (found == 0)
	{
		fprintf(stderr, "no authorities found, %u unloaded\n", unloaded);
		return 0;
	}
	if (loaded == found)
	{
		printf("successfully loaded %u authorities, %u unloaded\n",
			   loaded, unloaded);
		return 0;
	}
	fprintf(stderr, "loaded %u of %u authorities, %u failed to load, "
			"%u unloaded\n", loaded, found, found - loaded, unloaded);
	return EINVAL;
}

static int load_authorities(vici_conn_t *conn)
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
				return command_usage("invalid --load-authorities option");
		}
		break;
	}

	cfg = load_swanctl_conf(file);
	if (!cfg)
	{
		return EINVAL;
	}

	ret = load_authorities_cfg(conn, format, cfg);

	cfg->destroy(cfg);

	return ret;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		load_authorities, 'b',
		"load-authorities", "(re-)load authority configuration",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
			{"file",		'f', 1, "custom path to swanctl.conf"},
		}
	});
}
