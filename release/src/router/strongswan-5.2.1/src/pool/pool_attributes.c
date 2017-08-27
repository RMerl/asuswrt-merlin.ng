/*
 * Copyright (C) 2009-2010 Andreas Steffen
 * Hochschule fuer Technik Rapperswil
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
#include <string.h>

#include <library.h>
#include <networking/host.h>

#include "pool_attributes.h"
#include "pool_usage.h"

/**
 * global database handle
 */
extern database_t *db;

#define UNITY_NETWORK_LEN	14

ENUM(value_type_names, VALUE_HEX, VALUE_SUBNET,
	"hex",
	"string",
	"addr",
	"subnet"
);

typedef struct attr_info_t attr_info_t;

struct attr_info_t {
	char*                           keyword;
	value_type_t                    value_type;
	configuration_attribute_type_t  type;
	configuration_attribute_type_t  type_ip6;
};

static const attr_info_t attr_info[] = {
	{ "internal_ip4_netmask", VALUE_ADDR,   INTERNAL_IP4_NETMASK, 0 },
	{ "internal_ip6_netmask", VALUE_ADDR,   INTERNAL_IP6_NETMASK, 0 },
	{ "netmask",              VALUE_ADDR,   INTERNAL_IP4_NETMASK,
											INTERNAL_IP6_NETMASK    },
	{ "internal_ip4_dns",     VALUE_ADDR,   INTERNAL_IP4_DNS,     0 },
	{ "internal_ip6_dns",     VALUE_ADDR,   INTERNAL_IP6_DNS,     0 },
	{ "dns",                  VALUE_ADDR,   INTERNAL_IP4_DNS,
											INTERNAL_IP6_DNS        },
	{ "internal_ip4_nbns",    VALUE_ADDR,   INTERNAL_IP4_NBNS,    0 },
	{ "internal_ip6_nbns",    VALUE_ADDR,   INTERNAL_IP6_NBNS,    0 },
	{ "nbns",                 VALUE_ADDR,   INTERNAL_IP4_NBNS,
											INTERNAL_IP6_NBNS       },
	{ "wins",                 VALUE_ADDR,   INTERNAL_IP4_NBNS,
											INTERNAL_IP6_NBNS       },
	{ "internal_ip4_dhcp",    VALUE_ADDR,   INTERNAL_IP4_DHCP,    0 },
	{ "internal_ip6_dhcp",    VALUE_ADDR,   INTERNAL_IP6_DHCP,    0 },
	{ "dhcp",                 VALUE_ADDR,   INTERNAL_IP4_DHCP,
											INTERNAL_IP6_DHCP       },
	{ "internal_ip4_server",  VALUE_ADDR,   INTERNAL_IP4_SERVER,  0 },
	{ "internal_ip6_server",  VALUE_ADDR,   INTERNAL_IP6_SERVER,  0 },
	{ "server",               VALUE_ADDR,   INTERNAL_IP4_SERVER,
											INTERNAL_IP6_SERVER     },
	{ "application_version",  VALUE_STRING, APPLICATION_VERSION,  0 },
	{ "version",              VALUE_STRING, APPLICATION_VERSION,  0 },
	{ "unity_banner",         VALUE_STRING, UNITY_BANNER,         0 },
	{ "banner",               VALUE_STRING, UNITY_BANNER,         0 },
	{ "unity_def_domain",     VALUE_STRING, UNITY_DEF_DOMAIN,     0 },
	{ "unity_splitdns_name",  VALUE_STRING, UNITY_SPLITDNS_NAME,  0 },
	{ "unity_split_include",  VALUE_SUBNET, UNITY_SPLIT_INCLUDE,  0 },
	{ "unity_split_exclude",  VALUE_SUBNET, UNITY_LOCAL_LAN,      0 },
	{ "unity_local_lan",      VALUE_SUBNET, UNITY_LOCAL_LAN,      0 },
};

/**
 * Determine the type of the attribute and its value
 */
static bool parse_attributes(char *name, char *value, value_type_t *value_type,
							 configuration_attribute_type_t *type,
							 configuration_attribute_type_t *type_ip6,
							 chunk_t *blob)
{
	host_t *addr = NULL, *mask = NULL;
	chunk_t addr_chunk, mask_chunk, blob_next;
	char *text = "", *pos_addr, *pos_mask, *pos_next, *endptr;
	int i;

	switch (*value_type)
	{
		case VALUE_STRING:
			*blob = chunk_create(value, strlen(value));
			*blob = chunk_clone(*blob);
			break;
		case VALUE_HEX:
			*blob = chunk_from_hex(chunk_create(value, strlen(value)), NULL);
			break;
		case VALUE_ADDR:
			addr = host_create_from_string(value, 0);
			if (addr == NULL)
			{
				fprintf(stderr, "invalid IP address: '%s'.\n", value);
				return FALSE;
			}
			addr_chunk = addr->get_address(addr);
			*blob = chunk_clone(addr_chunk);
			break;
		case VALUE_SUBNET:
			*blob = chunk_empty;
			pos_next = value;

			do
			{
				pos_addr = pos_next;
				pos_next = strchr(pos_next, ',');
				if (pos_next)
				{
					*pos_next = '\0';
					pos_next += 1;
				}
				pos_mask = strchr(pos_addr, '/');
				if (pos_mask == NULL)
				{
					fprintf(stderr, "invalid IPv4 subnet: '%s'.\n", pos_addr);
					free(blob->ptr);
					return FALSE;
				}
				*pos_mask = '\0';
				pos_mask += 1;
				addr = host_create_from_string(pos_addr, 0);
				mask = host_create_from_string(pos_mask, 0);
				if (addr == NULL || addr->get_family(addr) != AF_INET ||
					mask == NULL || mask->get_family(addr) != AF_INET)
				{
					fprintf(stderr, "invalid IPv4 subnet: '%s/%s'.\n",
									pos_addr, pos_mask);
					DESTROY_IF(addr);
					DESTROY_IF(mask);
					free(blob->ptr);
					return FALSE;
				}
				addr_chunk = addr->get_address(addr);
				mask_chunk = mask->get_address(mask);
				blob_next = chunk_alloc(blob->len + UNITY_NETWORK_LEN);
				memcpy(blob_next.ptr, blob->ptr, blob->len);
				pos_addr = blob_next.ptr + blob->len;
				memset(pos_addr, 0x00, UNITY_NETWORK_LEN);
				memcpy(pos_addr,     addr_chunk.ptr, 4);
				memcpy(pos_addr + 4, mask_chunk.ptr, 4);
				addr->destroy(addr);
				addr = NULL;
				mask->destroy(mask);
				chunk_free(blob);
				*blob = blob_next;
			}
			while (pos_next);
			break;
		case VALUE_NONE:
			*blob = chunk_empty;
			break;
	}

	/* init the attribute type */
	*type     = 0;
	*type_ip6 = 0;

	for (i = 0; i < countof(attr_info); i++)
	{
		if (strcaseeq(name, attr_info[i].keyword))
		{
			*type      = attr_info[i].type;
			*type_ip6  = attr_info[i].type_ip6;

			if (*value_type == VALUE_NONE)
			{
				*value_type = attr_info[i].value_type;
				return TRUE;
			}

			if (*value_type != attr_info[i].value_type &&
				*value_type != VALUE_HEX)
			{
				switch (attr_info[i].value_type)
				{
					case VALUE_STRING:
						text = "a string";
						break;
					case VALUE_HEX:
						text = "a hex";
						break;
					case VALUE_ADDR:
						text = "an IP address";
						break;
					case VALUE_SUBNET:
						text = "a subnet";
						break;
					case VALUE_NONE:
						text = "no";
						break;
				}
				fprintf(stderr, "the %s attribute requires %s value.\n",
								 name, text);
				DESTROY_IF(addr);
				free(blob->ptr);
				return FALSE;
			}

			if (*value_type == VALUE_ADDR)
			{
				*type = (addr->get_family(addr) == AF_INET) ?
							attr_info[i].type : attr_info[i].type_ip6;
				addr->destroy(addr);
			}
			else if (*value_type == VALUE_HEX)
			{
				*value_type = attr_info[i].value_type;

				if (*value_type == VALUE_ADDR)
				{
					if (blob->len == 16)
					{
						*type = attr_info[i].type_ip6;
					}
					else if (blob->len != 4)
					{
						fprintf(stderr, "the %s attribute requires "
										"a valid IP address.\n", name);
						free(blob->ptr);
						return FALSE;
					}
				}
			}
			return TRUE;
		}
	}

	/* clean up */
	DESTROY_IF(addr);

	/* is the attribute type numeric? */
	*type = strtol(name, &endptr, 10);

	if (*endptr != '\0')
	{
		fprintf(stderr, "the %s attribute is not recognized.\n", name);
		free(blob->ptr);
		return FALSE;
	}
	if (*type < 1 || *type > 32767)
	{
		fprintf(stderr, "the attribute type must lie in the range 1..32767.\n");
		free(blob->ptr);
		return FALSE;
	}
	if (*value_type == VALUE_NONE)
	{
		*value_type = VALUE_HEX;
	}
	return TRUE;
}

/**
 * Lookup/insert an attribute pool by name
 */
static u_int get_attr_pool(char *name)
{
	enumerator_t *e;
	u_int row = 0;

	/* look for an existing attribute pool in the table */
	e = db->query(db, "SELECT id FROM attribute_pools WHERE name = ?",
				  DB_TEXT, name, DB_UINT);
	if (e && e->enumerate(e, &row))
	{
		e->destroy(e);
		return row;
	}
	DESTROY_IF(e);
	/* not found, insert new one */
	if (db->execute(db, &row, "INSERT INTO attribute_pools (name) VALUES (?)",
					DB_TEXT, name) != 1)
	{
		fprintf(stderr, "creating attribute pool '%s' failed.\n", name);
		return 0;
	}
	return row;
}

/**
 * Lookup/insert an identity
 */
u_int get_identity(identification_t *id)
{
	enumerator_t *e;
	u_int row;

	/* look for peer identity in the identities table */
	e = db->query(db, "SELECT id FROM identities WHERE type = ? AND data = ?",
			DB_INT, id->get_type(id), DB_BLOB, id->get_encoding(id), DB_UINT);
	if (e && e->enumerate(e, &row))
	{
		e->destroy(e);
		return row;
	}
	DESTROY_IF(e);
	/* not found, insert new one */
	if (db->execute(db, &row, "INSERT INTO identities (type,data) VALUES (?,?)",
				  DB_INT, id->get_type(id), DB_BLOB, id->get_encoding(id)) != 1)
	{
		fprintf(stderr, "creating id '%Y' failed.\n", id);
		return 0;
	}
	return row;
}

/**
 * ipsec pool --addattr <type> - add attribute entry
 */
void add_attr(char *name, char *pool, char *identity,
			  char *value, value_type_t value_type)
{
	configuration_attribute_type_t type, type_ip6;
	u_int pool_id = 0, identity_id = 0;
	char id_pool_str[128] = "";
	chunk_t blob;
	bool success;

	if (pool)
	{
		pool_id = get_attr_pool(pool);
		if (pool_id == 0)
		{
			exit(EXIT_FAILURE);
		}

		if (identity)
		{
			identification_t *id;

			id = identification_create_from_string(identity);
			identity_id = get_identity(id);
			id->destroy(id);
			if (identity_id == 0)
			{
				exit(EXIT_FAILURE);
			}
			snprintf(id_pool_str, sizeof(id_pool_str),
					 " for '%s' in pool '%s'", identity, pool);
		}
		else
		{
			snprintf(id_pool_str, sizeof(id_pool_str), " in pool '%s'", pool);
		}
	}

	if (value_type == VALUE_NONE)
	{
		fprintf(stderr, "the value of the %s attribute is missing.\n", name);
		usage();
	}
	if (!parse_attributes(name, value, &value_type, &type, &type_ip6, &blob))
	{
		exit(EXIT_FAILURE);
	}

	success = db->execute(db, NULL,
				"INSERT INTO attributes (identity, pool, type, value) "
				"VALUES (?, ?, ?, ?)", DB_UINT, identity_id, DB_UINT, pool_id,
				DB_INT, type, DB_BLOB, blob) == 1;
	free(blob.ptr);

	if (success)
	{
		printf("added %s attribute (%N)%s.\n", name,
			   configuration_attribute_type_names, type, id_pool_str);
	}
	else
	{
		fprintf(stderr, "adding %s attribute (%N)%s failed.\n", name,
						configuration_attribute_type_names, type, id_pool_str);
	}
}

/**
 * ipsec pool --delattr <type> - delete attribute entry
 */
void del_attr(char *name, char *pool, char *identity,
			  char *value, value_type_t value_type)
{
	configuration_attribute_type_t type, type_ip6, type_db;
	u_int pool_id = 0, identity_id = 0;
	char id_pool_str[128] = "";
	chunk_t blob, blob_db;
	u_int id;
	enumerator_t *query;
	bool found = FALSE;

	if (pool)
	{
		pool_id = get_attr_pool(pool);
		if (pool_id == 0)
		{
			exit(EXIT_FAILURE);
		}

		if (identity)
		{
			identification_t *id;

			id = identification_create_from_string(identity);
			identity_id = get_identity(id);
			id->destroy(id);
			if (identity_id == 0)
			{
				exit(EXIT_FAILURE);
			}
			snprintf(id_pool_str, sizeof(id_pool_str),
					 " for '%s' in pool '%s'", identity, pool);
		}
		else
		{
			snprintf(id_pool_str, sizeof(id_pool_str), " in pool '%s'", pool);
		}
	}

	if (!parse_attributes(name, value, &value_type, &type, &type_ip6, &blob))
	{
		exit(EXIT_FAILURE);
	}

	if (blob.len > 0)
	{
		query = db->query(db,
					"SELECT id, type, value FROM attributes "
					"WHERE identity = ? AND pool = ? AND type = ? AND value = ?",
					DB_UINT, identity_id, DB_UINT, pool_id, DB_INT, type,
					DB_BLOB, blob, DB_UINT, DB_INT, DB_BLOB);
	}
	else if (type_ip6 == 0)
	{
		query = db->query(db,
					"SELECT id, type, value FROM attributes "
					"WHERE identity = ? AND pool = ? AND type = ?",
					DB_UINT, identity_id, DB_UINT, pool_id, DB_INT, type,
					DB_UINT, DB_INT, DB_BLOB);
	}
	else
	{
		query = db->query(db,
					"SELECT id, type, value FROM attributes "
					"WHERE identity = ? AND pool = ? AND (type = ? OR type = ?)",
					DB_UINT, identity_id, DB_UINT, pool_id, DB_INT, type,
					DB_INT, type_ip6, DB_UINT, DB_INT, DB_BLOB);
	}

	if (!query)
	{
		fprintf(stderr, "deleting '%s' attribute (%N)%s failed.\n",
				name, configuration_attribute_type_names, type, id_pool_str);
		free(blob.ptr);
		exit(EXIT_FAILURE);
	}

	while (query->enumerate(query, &id, &type_db, &blob_db))
	{
		host_t *server = NULL;

		found = TRUE;

		if (value_type == VALUE_ADDR)
		{
			int family = (type_db == type_ip6) ? AF_INET6 : AF_INET;

			server = host_create_from_chunk(family, blob_db, 0);
		}

		if (db->execute(db, NULL,
					"DELETE FROM attributes WHERE id = ?",
					 DB_UINT, id) != 1)
		{
			if (server)
			{
				fprintf(stderr, "deleting %s server %H%s failed\n",
						name, server, id_pool_str);
				server->destroy(server);
			}
			else if (value_type == VALUE_STRING)
			{
				fprintf(stderr, "deleting %s attribute (%N) with value '%.*s'%s failed.\n",
								name, configuration_attribute_type_names, type,
								(int)blob_db.len, blob_db.ptr, id_pool_str);
			}

			else
			{
				fprintf(stderr, "deleting %s attribute (%N) with value %#B%s failed.\n",
								name, configuration_attribute_type_names, type,
								&blob_db, id_pool_str);
			}
			query->destroy(query);
			free(blob.ptr);
			exit(EXIT_FAILURE);
		}
		if (server)
		{
			printf("deleted %s server %H%s\n", name, server, id_pool_str);
			server->destroy(server);
		}
		else if (value_type == VALUE_STRING)
		{
			printf("deleted %s attribute (%N) with value '%.*s'%s.\n",
				   name, configuration_attribute_type_names, type,
				   (int)blob_db.len, blob_db.ptr, id_pool_str);
		}
		else
		{
			printf("deleted %s attribute (%N) with value %#B%s.\n",
				   name, configuration_attribute_type_names, type,
				   &blob_db, id_pool_str);
		}
	}
	query->destroy(query);

	if (!found)
	{
		if (blob.len == 0)
		{
			if (type_ip6 == 0)
			{
				fprintf(stderr, "no %s attribute (%N) was found%s.\n", name,
						configuration_attribute_type_names, type, id_pool_str);
			}
			else
			{
				fprintf(stderr, "no %s attribute%s was found.\n",
						name, id_pool_str);
			}
		}
		else
		{
			if (value_type == VALUE_ADDR)
			{
				host_t *server = host_create_from_chunk(AF_UNSPEC, blob, 0);

				fprintf(stderr, "the %s server %H%s was not found.\n", name,
								 server, id_pool_str);
				server->destroy(server);
			}
			else
			{
				fprintf(stderr, "the %s attribute (%N) with value '%.*s'%s "
								"was not found.\n", name,
								 configuration_attribute_type_names, type,
								 (int)blob.len, blob.ptr, id_pool_str);
			}
		}
	}
	free(blob.ptr);
}

/**
 * ipsec pool --statusattr - show all attribute entries
 */
void status_attr(bool hexout)
{
	configuration_attribute_type_t type;
	value_type_t value_type;
	chunk_t value, addr_chunk, mask_chunk, identity_chunk;
	identification_t *identity;
	enumerator_t *enumerator;
	host_t *addr, *mask;
	char type_name[30];
	bool first = TRUE;
	int i, identity_type;
	char *pool_name;

	/* enumerate over all attributes */
	enumerator = db->query(db,
					"SELECT attributes.type, attribute_pools.name, "
					"identities.type, identities.data, attributes.value "
					"FROM attributes "
					"LEFT OUTER JOIN identities "
					"ON attributes.identity = identities.id "
					"LEFT OUTER JOIN attribute_pools "
					"ON attributes.pool = attribute_pools.id "
					"ORDER BY attributes.type, attribute_pools.name, "
					"identities.type, identities.data, attributes.value",
					DB_INT, DB_TEXT, DB_INT, DB_BLOB, DB_BLOB);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, &type,&pool_name,
									 &identity_type, &identity_chunk, &value))
		{
			if (first)
			{
				printf(" type  description           pool       "
					   " identity              value\n");
				first = FALSE;
			}
			snprintf(type_name, sizeof(type_name), "%N",
					 configuration_attribute_type_names, type);
			if (type_name[0] == '(')
			{
				type_name[0] = '\0';
			}
			printf("%5d  %-20s ",type, type_name);

			printf(" %-10s ", (pool_name ? pool_name : ""));

			if (identity_type)
			{
				identity = identification_create_from_encoding(identity_type, identity_chunk);
				printf(" %-20.20Y ", identity);
				identity->destroy(identity);
			}
			else
			{
				printf("                      ");
			}

			value_type = VALUE_HEX;
			if (!hexout)
			{
				for (i = 0; i < countof(attr_info); i++)
				{
					if (type == attr_info[i].type)
					{
						value_type = attr_info[i].value_type;
						break;
					}
				}
			}
			switch (value_type)
			{
				case VALUE_ADDR:
					addr = host_create_from_chunk(AF_UNSPEC, value, 0);
					if (addr)
					{
						printf(" %H\n", addr);
						addr->destroy(addr);
					}
					else
					{
						/* value cannot be represented as an IP address */
						printf(" %#B\n", &value);
					}
					break;
				case VALUE_SUBNET:
					if (value.len % UNITY_NETWORK_LEN == 0)
					{
						for (i = 0; i < value.len / UNITY_NETWORK_LEN; i++)
						{
							addr_chunk = chunk_create(value.ptr + i*UNITY_NETWORK_LEN, 4);
							addr = host_create_from_chunk(AF_INET, addr_chunk, 0);
							mask_chunk = chunk_create(addr_chunk.ptr + 4, 4);
							mask = host_create_from_chunk(AF_INET, mask_chunk, 0);
							printf("%s%H/%H", (i > 0) ? "," : " ", addr, mask);
							addr->destroy(addr);
							mask->destroy(mask);
						}
						printf("\n");
					}
					else
					{
						/* value cannot be represented as a list of subnets */
						printf(" %#B\n", &value);
					}
					break;
				case VALUE_STRING:
					printf("\"%.*s\"\n", (int)value.len, value.ptr);
					break;
				case VALUE_HEX:
				default:
					printf(" %#B\n", &value);
			}
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * ipsec pool --showattr - show all supported attribute keywords
 */
void show_attr(void)
{
	int i;

	for (i = 0; i < countof(attr_info); i++)
	{
		char value_name[10];


		snprintf(value_name, sizeof(value_name), "%N",
			value_type_names, attr_info[i].value_type);

		printf("%-20s  --%-6s  (%N",
				attr_info[i].keyword, value_name,
				configuration_attribute_type_names, attr_info[i].type);

		if (attr_info[i].type_ip6)
		{
			printf(", %N)\n",
				configuration_attribute_type_names, attr_info[i].type_ip6);
		}
		else
		{
			printf(")\n");
		}
	}
}
