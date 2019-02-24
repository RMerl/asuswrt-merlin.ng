/*
 * Copyright (C) 2011-2017 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <utils/debug.h>
#include <library.h>
#include <collections/array.h>
#include <networking/host.h>
#include <utils/identification.h>
#include <attributes/attributes.h>

#include "pool_attributes.h"
#include "pool_usage.h"

/**
 * global database handle
 */
database_t *db;

/**
 * --start/--end addresses of various subcommands
 */
host_t *start = NULL, *end = NULL;

/**
 * whether --add should --replace an existing pool
 */
bool replace_pool = FALSE;

/**
 * forward declarations
 */
static void del(char *name);
static void do_args(int argc, char *argv[]);

/**
 * Create or replace a pool by name
 */
static u_int create_pool(char *name, chunk_t start, chunk_t end, u_int timeout)
{
	enumerator_t *e;
	int pool;
	bool exists;

	e = db->query(db, "SELECT id FROM pools WHERE name = ?",
			DB_TEXT, name, DB_UINT);
	exists = e && e->enumerate(e, &pool);
	DESTROY_IF(e);

	if (exists)
	{
		if (!replace_pool)
		{
			fprintf(stderr, "pool '%s' exists.\n", name);
			exit(EXIT_FAILURE);
		}
		del(name);
	}
	if (db->execute(db, &pool,
			"INSERT INTO pools (name, start, end, timeout) VALUES (?, ?, ?, ?)",
			DB_TEXT, name, DB_BLOB, start, DB_BLOB, end,
			DB_UINT, timeout) != 1)
	{
		fprintf(stderr, "creating pool failed.\n");
		exit(EXIT_FAILURE);
	}

	return pool;
}

/**
 * instead of a pool handle a DNS or NBNS attribute
 */
static bool is_attribute(char *name)
{
	return strcaseeq(name, "dns") || strcaseeq(name, "nbns") ||
		   strcaseeq(name, "wins");
}

/**
 * calculate the size of a pool using start and end address chunk
 */
static u_int get_pool_size(chunk_t start, chunk_t end)
{
	u_int *start_ptr, *end_ptr;

	if (start.len < sizeof(u_int) || end.len < sizeof(u_int))
	{
		return 0;
	}
	start_ptr = (u_int*)(start.ptr + start.len - sizeof(u_int));
	end_ptr = (u_int*)(end.ptr + end.len - sizeof(u_int));
	return ntohl(*end_ptr) -  ntohl(*start_ptr) + 1;
}

/**
 * ipsec pool --status - show pool overview
 */
static void status(void)
{
	enumerator_t *ns, *pool, *lease;
	host_t *server;
	chunk_t value;
	bool found = FALSE;

	/* enumerate IPv4 DNS servers */
	ns = db->query(db, "SELECT value FROM attributes WHERE type = ?",
				   DB_INT, INTERNAL_IP4_DNS, DB_BLOB);
	if (ns)
	{
		while (ns->enumerate(ns, &value))
		{
			if (!found)
			{
				printf("dns servers:");
				found = TRUE;
			}
			server = host_create_from_chunk(AF_INET, value, 0);
			if (server)
			{
				printf(" %H", server);
				server->destroy(server);
			}
		}
		ns->destroy(ns);
	}

	/* enumerate IPv6 DNS servers */
	ns = db->query(db, "SELECT value FROM attributes WHERE type = ?",
				   DB_INT, INTERNAL_IP6_DNS, DB_BLOB);
	if (ns)
	{
		while (ns->enumerate(ns, &value))
		{
			if (!found)
			{
				printf("dns servers:");
				found = TRUE;
			}
			server = host_create_from_chunk(AF_INET6, value, 0);
			if (server)
			{
				printf(" %H", server);
				server->destroy(server);
			}
		}
		ns->destroy(ns);
	}
	if (found)
	{
		printf("\n");
	}
	else
	{
		printf("no dns servers found.\n");
	}
	found = FALSE;

	/* enumerate IPv4 NBNS servers */
	ns = db->query(db, "SELECT value FROM attributes WHERE type = ?",
				   DB_INT, INTERNAL_IP4_NBNS, DB_BLOB);
	if (ns)
	{
		while (ns->enumerate(ns, &value))
		{
			if (!found)
			{
				printf("nbns servers:");
				found = TRUE;
			}
			server = host_create_from_chunk(AF_INET, value, 0);
			if (server)
			{
				printf(" %H", server);
				server->destroy(server);
			}
		}
		ns->destroy(ns);
	}

	/* enumerate IPv6 NBNS servers */
	ns = db->query(db, "SELECT value FROM attributes WHERE type = ?",
				   DB_INT, INTERNAL_IP6_NBNS, DB_BLOB);
	if (ns)
	{
		while (ns->enumerate(ns, &value))
		{
			if (!found)
			{
				printf("nbns servers:");
				found = TRUE;
			}
			server = host_create_from_chunk(AF_INET6, value, 0);
			if (server)
			{
				printf(" %H", server);
				server->destroy(server);
			}
		}
		ns->destroy(ns);
	}
	if (found)
	{
		printf("\n");
	}
	else
	{
		printf("no nbns servers found.\n");
	}
	found = FALSE;

	pool = db->query(db, "SELECT id, name, start, end, timeout FROM pools",
					 DB_INT, DB_TEXT, DB_BLOB, DB_BLOB, DB_UINT);
	if (pool)
	{
		char *name;
		chunk_t start_chunk, end_chunk;
		host_t *start, *end;
		u_int id, timeout, online = 0, used = 0, size = 0;

		while (pool->enumerate(pool, &id, &name,
							   &start_chunk, &end_chunk, &timeout))
		{
			if (!found)
			{
				printf("%8s %15s %15s %8s %6s %11s %11s\n", "name", "start",
					   "end", "timeout", "size", "online", "usage");
				found = TRUE;
			}

			start = host_create_from_chunk(AF_UNSPEC, start_chunk, 0);
			end = host_create_from_chunk(AF_UNSPEC, end_chunk, 0);
			if (start->is_anyaddr(start) && end->is_anyaddr(end))
			{
				printf("%8s %15s %15s ", name, "n/a", "n/a");
			}
			else
			{
				printf("%8s %15H %15H ", name, start, end);
			}
			if (timeout)
			{
				if (timeout >= 60 * 300)
				{
					printf("%7dh ", timeout/3600);
				}
				else if (timeout >= 300)
				{
					printf("%7dm ", timeout/60);
				}
				else
				{
					printf("%7ds ", timeout);
				}
			}
			else
			{
				printf("%8s ", "static");
			}
			/* get total number of hosts in the pool */
			lease = db->query(db, "SELECT COUNT(*) FROM addresses "
							  "WHERE pool = ?", DB_UINT, id, DB_INT);
			if (lease)
			{
				lease->enumerate(lease, &size);
				lease->destroy(lease);
			}
			if (!size)
			{	/* empty pool */
				printf("%6d %11s %11s ", 0, "n/a", "n/a");
				goto next_pool;
			}
			printf("%6d ", size);
			/* get number of online hosts */
			lease = db->query(db, "SELECT COUNT(*) FROM addresses "
							  "WHERE pool = ? AND released = 0",
							  DB_UINT, id, DB_INT);
			if (lease)
			{
				lease->enumerate(lease, &online);
				lease->destroy(lease);
			}
			printf("%5d (%2d%%) ", online, online*100/size);
			/* get number of online or valid leases */
			lease = db->query(db, "SELECT COUNT(*) FROM addresses "
							  "WHERE addresses.pool = ? "
							  "AND ((? AND acquired != 0) "
							  "     OR released = 0 OR released > ?) ",
							  DB_UINT, id, DB_UINT, !timeout,
							  DB_UINT, time(NULL) - timeout, DB_UINT);
			if (lease)
			{
				lease->enumerate(lease, &used);
				lease->destroy(lease);
			}
			printf("%5d (%2d%%) ", used, used*100/size);

next_pool:
			printf("\n");
			DESTROY_IF(start);
			DESTROY_IF(end);
		}
		pool->destroy(pool);
	}
	if (!found)
	{
		printf("no pools found.\n");
	}
}

/**
 * ipsec pool --add - add a new pool
 */
static void add(char *name, host_t *start, host_t *end, u_int timeout)
{
	chunk_t start_addr, end_addr, cur_addr;
	u_int id, count;

	start_addr = start->get_address(start);
	end_addr = end->get_address(end);
	cur_addr = chunk_clonea(start_addr);
	count = get_pool_size(start_addr, end_addr);

	if (start_addr.len != end_addr.len ||
		memcmp(start_addr.ptr, end_addr.ptr, start_addr.len) > 0)
	{
		fprintf(stderr, "invalid start/end pair specified.\n");
		exit(EXIT_FAILURE);
	}
	id = create_pool(name, start_addr, end_addr, timeout);
	printf("allocating %d addresses... ", count);
	fflush(stdout);
	db->transaction(db, FALSE);
	while (TRUE)
	{
		db->execute(db, NULL,
			"INSERT INTO addresses (pool, address, identity, acquired, released) "
			"VALUES (?, ?, ?, ?, ?)",
			DB_UINT, id, DB_BLOB, cur_addr,	DB_UINT, 0, DB_UINT, 0, DB_UINT, 1);
		if (chunk_equals(cur_addr, end_addr))
		{
			break;
		}
		chunk_increment(cur_addr);
	}
	db->commit(db);
	printf("done.\n");
}

static bool add_address(u_int pool_id, char *address_str, int *family)
{
	host_t *address;
	int user_id = 0;

	char *pos_eq = strchr(address_str, '=');
	if (pos_eq != NULL)
	{
		identification_t *id = identification_create_from_string(pos_eq + 1);
		user_id = get_identity(id);
		id->destroy(id);

		if (user_id == 0)
		{
			return FALSE;
		}
		*pos_eq = '\0';
	}

	address = host_create_from_string(address_str, 0);
	if (address == NULL)
	{
		fprintf(stderr, "invalid address '%s'.\n", address_str);
		return FALSE;
	}
	if (family && *family != AF_UNSPEC &&
		*family != address->get_family(address))
	{
		fprintf(stderr, "invalid address family '%s'.\n", address_str);
		address->destroy(address);
		return FALSE;
	}

	if (db->execute(db, NULL,
			"INSERT INTO addresses "
			"(pool, address, identity, acquired, released) "
			"VALUES (?, ?, ?, ?, ?)",
			DB_UINT, pool_id, DB_BLOB, address->get_address(address),
			DB_UINT, user_id, DB_UINT, 0, DB_UINT, 1) != 1)
	{
		fprintf(stderr, "inserting address '%s' failed.\n", address_str);
		address->destroy(address);
		return FALSE;
	}
	if (family)
	{
		*family = address->get_family(address);
	}
	address->destroy(address);

	return TRUE;
}

static void add_addresses(char *pool, char *path, u_int timeout)
{
	u_int pool_id, count = 0;
	int family = AF_UNSPEC;
	char address_str[512];
	host_t *addr;
	FILE *file;

	db->transaction(db, FALSE);

	addr = host_create_from_string("%any", 0);
	pool_id = create_pool(pool, addr->get_address(addr),
						  addr->get_address(addr), timeout);
	addr->destroy(addr);

	file = (strcmp(path, "-") == 0 ? stdin : fopen(path, "r"));
	if (file == NULL)
	{
		fprintf(stderr, "opening '%s' failed: %s\n", path, strerror(errno));
		exit(-1);
	}

	printf("starting allocation... ");
	fflush(stdout);

	while (fgets(address_str, sizeof(address_str), file))
	{
		size_t addr_len = strlen(address_str);
		char *last_chr = address_str + addr_len - 1;
		if (*last_chr == '\n')
		{
			if (addr_len == 1)
			{	/* end of input */
				break;
			}
			*last_chr = '\0';
		}
		if (add_address(pool_id, address_str, &family) == FALSE)
		{
			if (file != stdin)
			{
				fclose(file);
			}
			exit(EXIT_FAILURE);
		}
		++count;
	}

	if (file != stdin)
	{
		fclose(file);
	}

	if (family == AF_INET6)
	{	/* update address family if necessary */
		addr = host_create_from_string("%any6", 0);
		if (db->execute(db, NULL,
					"UPDATE pools SET start = ?, end = ? WHERE id = ?",
					DB_BLOB, addr->get_address(addr),
					DB_BLOB, addr->get_address(addr), DB_UINT, pool_id) <= 0)
		{
			addr->destroy(addr);
			fprintf(stderr, "updating pool address family failed.\n");
			exit(EXIT_FAILURE);
		}
		addr->destroy(addr);
	}

	db->commit(db);

	printf("%d addresses done.\n", count);
}

/**
 * ipsec pool --del - delete a pool
 */
static void del(char *name)
{
	enumerator_t *query;
	u_int id;
	bool found = FALSE;

	query = db->query(db, "SELECT id FROM pools WHERE name = ?",
					  DB_TEXT, name, DB_UINT);
	if (!query)
	{
		fprintf(stderr, "deleting pool failed.\n");
		exit(EXIT_FAILURE);
	}
	while (query->enumerate(query, &id))
	{
		found = TRUE;
		if (db->execute(db, NULL,
				"DELETE FROM leases WHERE address IN ("
				" SELECT id FROM addresses WHERE pool = ?)", DB_UINT, id) < 0 ||
			db->execute(db, NULL,
				"DELETE FROM addresses WHERE pool = ?", DB_UINT, id) < 0 ||
			db->execute(db, NULL,
				"DELETE FROM pools WHERE id = ?", DB_UINT, id) < 0)
		{
			fprintf(stderr, "deleting pool failed.\n");
			query->destroy(query);
			exit(EXIT_FAILURE);
		}
	}
	query->destroy(query);
	if (!found)
	{
		fprintf(stderr, "pool '%s' not found.\n", name);
		exit(EXIT_FAILURE);
	}
}

/**
 * ipsec pool --resize - resize a pool
 */
static void resize(char *name, host_t *end)
{
	enumerator_t *query;
	chunk_t old_addr, new_addr, cur_addr;
	u_int id, count;
	host_t *old_end;

	new_addr = end->get_address(end);

	query = db->query(db, "SELECT id, end FROM pools WHERE name = ?",
					  DB_TEXT, name, DB_UINT, DB_BLOB);
	if (!query || !query->enumerate(query, &id, &old_addr))
	{
		DESTROY_IF(query);
		fprintf(stderr, "resizing pool failed.\n");
		exit(EXIT_FAILURE);
	}
	if (old_addr.len != new_addr.len ||
		memcmp(new_addr.ptr, old_addr.ptr, old_addr.len) < 0)
	{
		fprintf(stderr, "shrinking of pools not supported.\n");
		query->destroy(query);
		exit(EXIT_FAILURE);
	}
	cur_addr = chunk_clonea(old_addr);
	count = get_pool_size(old_addr, new_addr) - 1;
	query->destroy(query);

	/* Check whether pool is resizable */
	old_end = host_create_from_chunk(AF_UNSPEC, old_addr, 0);
	if (old_end && old_end->is_anyaddr(old_end))
	{
		fprintf(stderr, "pool is not resizable.\n");
		old_end->destroy(old_end);
		exit(EXIT_FAILURE);
	}
	DESTROY_IF(old_end);

	db->transaction(db, FALSE);
	if (db->execute(db, NULL,
			"UPDATE pools SET end = ? WHERE name = ?",
			DB_BLOB, new_addr, DB_TEXT, name) <= 0)
	{
		fprintf(stderr, "pool '%s' not found.\n", name);
		exit(EXIT_FAILURE);
	}

	printf("allocating %d new addresses... ", count);
	fflush(stdout);
	while (count-- > 0)
	{
		chunk_increment(cur_addr);
		db->execute(db, NULL,
			"INSERT INTO addresses (pool, address, identity, acquired, released) "
			"VALUES (?, ?, ?, ?, ?)",
			DB_UINT, id, DB_BLOB, cur_addr,	DB_UINT, 0, DB_UINT, 0, DB_UINT, 1);
	}
	db->commit(db);
	printf("done.\n");

}

/**
 * create the lease query using the filter string
 */
static enumerator_t *create_lease_query(char *filter, array_t **to_free)
{
	enumerator_t *query;
	chunk_t id_chunk = chunk_empty, addr_chunk = chunk_empty;
	id_type_t id_type = 0;
	u_int tstamp = 0;
	bool online = FALSE, valid = FALSE, expired = FALSE;
	char *value, *pos, *pool = NULL;
	enum {
		FIL_POOL = 0,
		FIL_ID,
		FIL_ADDR,
		FIL_TSTAMP,
		FIL_STATE,
	};
	char *const token[] = {
		[FIL_POOL] = "pool",
		[FIL_ID] = "id",
		[FIL_ADDR] = "addr",
		[FIL_TSTAMP] = "tstamp",
		[FIL_STATE] = "status",
		NULL
	};

	/* if the filter string contains a distinguished name as a ID, we replace
	 * ", " by "/ " in order to not confuse the getsubopt parser */
	pos = filter;
	while ((pos = strchr(pos, ',')))
	{
		if (pos[1] == ' ')
		{
			pos[0] = '/';
		}
		pos++;
	}

	while (filter && *filter != '\0')
	{
		switch (getsubopt(&filter, token, &value))
		{
			case FIL_POOL:
				if (value)
				{
					pool = value;
				}
				break;
			case FIL_ID:
				if (value)
				{
					identification_t *id;

					id = identification_create_from_string(value);
					id_type = id->get_type(id);
					id_chunk = chunk_clone(id->get_encoding(id));
					array_insert_create(to_free, ARRAY_TAIL, id_chunk.ptr);
					id->destroy(id);
				}
				break;
			case FIL_ADDR:
				if (value)
				{
					host_t *addr;

					addr = host_create_from_string(value, 0);
					if (!addr)
					{
						fprintf(stderr, "invalid 'addr' in filter string.\n");
						exit(EXIT_FAILURE);
					}
					addr_chunk = chunk_clone(addr->get_address(addr));
					array_insert_create(to_free, ARRAY_TAIL, addr_chunk.ptr);
					addr->destroy(addr);
				}
				break;
			case FIL_TSTAMP:
				if (value)
				{
					tstamp = atoi(value);
				}
				if (tstamp == 0)
				{
					online = TRUE;
				}
				break;
			case FIL_STATE:
				if (value)
				{
					if (streq(value, "online"))
					{
						online = TRUE;
					}
					else if (streq(value, "valid"))
					{
						valid = TRUE;
					}
					else if (streq(value, "expired"))
					{
						expired = TRUE;
					}
					else
					{
						fprintf(stderr, "invalid 'state' in filter string.\n");
						exit(EXIT_FAILURE);
					}
				}
				break;
			default:
				fprintf(stderr, "invalid filter string.\n");
				exit(EXIT_FAILURE);
		}
	}
	query = db->query(db,
				"SELECT name, addresses.address, identities.type, "
				"identities.data, leases.acquired, leases.released, timeout "
				"FROM leases JOIN addresses ON leases.address = addresses.id "
				"JOIN pools ON addresses.pool = pools.id "
				"JOIN identities ON leases.identity = identities.id "
				"WHERE (? OR name = ?) "
				"AND (? OR (identities.type = ? AND identities.data = ?)) "
				"AND (? OR addresses.address = ?) "
				"AND (? OR (? >= leases.acquired AND (? <= leases.released))) "
				"AND (? OR leases.released > ? - timeout) "
				"AND (? OR leases.released < ? - timeout) "
				"AND ? "
				"UNION "
				"SELECT name, address, identities.type, identities.data, "
				"acquired, released, timeout FROM addresses "
				"JOIN pools ON addresses.pool = pools.id "
				"JOIN identities ON addresses.identity = identities.id "
				"WHERE ? AND released = 0 "
				"AND (? OR name = ?) "
				"AND (? OR (identities.type = ? AND identities.data = ?)) "
				"AND (? OR address = ?)",
				DB_INT, pool == NULL, DB_TEXT, pool,
				DB_INT, !id_chunk.ptr,
					DB_INT, id_type,
					DB_BLOB, id_chunk,
				DB_INT, !addr_chunk.ptr,
					DB_BLOB, addr_chunk,
				DB_INT, tstamp == 0, DB_UINT, tstamp, DB_UINT, tstamp,
				DB_INT, !valid, DB_INT, time(NULL),
				DB_INT, !expired, DB_INT, time(NULL),
				DB_INT, !online,
				/* union */
				DB_INT, !(valid || expired),
				DB_INT, pool == NULL, DB_TEXT, pool,
				DB_INT, !id_chunk.ptr,
					DB_INT, id_type,
					DB_BLOB, id_chunk,
				DB_INT, !addr_chunk.ptr,
					DB_BLOB, addr_chunk,
				/* res */
				DB_TEXT, DB_BLOB, DB_INT, DB_BLOB, DB_UINT, DB_UINT, DB_UINT);
	return query;
}

/**
 * ipsec pool --leases - show lease information of a pool
 */
static void leases(char *filter, bool utc)
{
	enumerator_t *query;
	array_t *to_free = NULL;
	chunk_t address_chunk, identity_chunk;
	int identity_type;
	char *name;
	u_int db_acquired, db_released, db_timeout;
	time_t acquired, released, timeout;
	host_t *address;
	identification_t *identity;
	bool found = FALSE;

	query = create_lease_query(filter, &to_free);
	if (!query)
	{
		fprintf(stderr, "querying leases failed.\n");
		exit(EXIT_FAILURE);
	}
	while (query->enumerate(query, &name, &address_chunk, &identity_type,
							&identity_chunk, &db_acquired, &db_released, &db_timeout))
	{
		if (!found)
		{
			int len = utc ? 25 : 21;

			found = TRUE;
			printf("%-8s %-15s %-7s  %-*s %-*s %s\n",
				   "name", "address", "status", len, "start", len, "end", "identity");
		}
		address = host_create_from_chunk(AF_UNSPEC, address_chunk, 0);
		identity = identification_create_from_encoding(identity_type, identity_chunk);

		/* u_int is not always equal to time_t */
		acquired = (time_t)db_acquired;
		released = (time_t)db_released;
		timeout  = (time_t)db_timeout;

		printf("%-8s %-15H ", name, address);
		if (released == 0)
		{
			printf("%-7s ", "online");
		}
		else if (timeout == 0)
		{
			printf("%-7s ", "static");
		}
		else if (released >= time(NULL) - timeout)
		{
			printf("%-7s ", "valid");
		}
		else
		{
			printf("%-7s ", "expired");
		}

		printf(" %T  ", &acquired, utc);
		if (released)
		{
			printf("%T  ", &released, utc);
		}
		else
		{
			printf("                      ");
			if (utc)
			{
				printf("    ");
			}
		}
		printf("%Y\n", identity);
		DESTROY_IF(address);
		identity->destroy(identity);
	}
	query->destroy(query);
	if (to_free)
	{
		array_destroy_function(to_free, (void*)free, NULL);
	}
	if (!found)
	{
		fprintf(stderr, "no matching leases found.\n");
		exit(EXIT_FAILURE);
	}
}

/**
 * ipsec pool --purge - delete expired leases
 */
static void purge(char *name)
{
	int purged = 0;

	purged = db->execute(db, NULL,
				"DELETE FROM leases WHERE address IN ("
				" SELECT id FROM addresses WHERE pool IN ("
				"  SELECT id FROM pools WHERE name = ?))",
				DB_TEXT, name);
	if (purged < 0)
	{
		fprintf(stderr, "purging pool '%s' failed.\n", name);
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "purged %d leases in pool '%s'.\n", purged, name);
}

#define ARGV_SIZE 32

static void argv_add(char **argv, int argc, char *value)
{
	if (argc >= ARGV_SIZE)
	{
		fprintf(stderr, "too many arguments: %s\n", value);
		exit(EXIT_FAILURE);
	}
	argv[argc] = value;
}

/**
 * ipsec pool --batch - read commands from a file
 */
static void batch(char *argv0, char *name)
{
	char command[512];

	FILE *file = strncmp(name, "-", 1) == 0 ? stdin : fopen(name, "r");
	if (file == NULL)
	{
		fprintf(stderr, "opening '%s' failed: %s\n", name, strerror(errno));
		exit(EXIT_FAILURE);
	}

	db->transaction(db, FALSE);
	while (fgets(command, sizeof(command), file))
	{
		char *argv[ARGV_SIZE], *start;
		int i, argc = 0;
		size_t cmd_len = strlen(command);

		/* ignore empty lines */
		if (cmd_len == 1 && *(command + cmd_len - 1) == '\n')
		{
			continue;
		}

		/* parse command into argv */
		start = command;
		argv_add(argv, argc++, argv0);
		for (i = 0; i < cmd_len; ++i)
		{
			if (command[i] == ' ' || command[i] == '\n')
			{
				if (command + i == start)
				{
					/* ignore leading whitespace */
					++start;
					continue;
				}
				command[i] = '\0';
				argv_add(argv, argc++, start);
				start = command + i + 1;
			}
		}
		if (strlen(start) > 0)
		{
			argv_add(argv, argc++, start);
		}
		argv_add(argv, argc, NULL);

		do_args(argc, argv);
	}
	db->commit(db);

	if (file != stdin)
	{
		fclose(file);
	}
}

/**
 * atexit handler to close db on shutdown
 */
static void cleanup(void)
{
	db->destroy(db);
	DESTROY_IF(start);
	DESTROY_IF(end);
}

static void do_args(int argc, char *argv[])
{
	char *name = "", *value = "", *filter = "";
	char *pool = NULL, *identity = NULL, *addresses = NULL;
	value_type_t value_type = VALUE_NONE;
	time_t timeout = 0;
	bool utc = FALSE, hexout = FALSE;

	enum {
		OP_UNDEF,
		OP_USAGE,
		OP_STATUS,
		OP_STATUS_ATTR,
		OP_ADD,
		OP_ADD_ATTR,
		OP_DEL,
		OP_DEL_ATTR,
		OP_SHOW_ATTR,
		OP_RESIZE,
		OP_LEASES,
		OP_PURGE,
		OP_BATCH
	} operation = OP_UNDEF;

	/* reinit getopt state */
	optind = 0;

	while (TRUE)
	{
		int c;

		struct option long_opts[] = {
			{ "help", no_argument, NULL, 'h' },

			{ "utc", no_argument, NULL, 'u' },
			{ "status", no_argument, NULL, 'w' },
			{ "add", required_argument, NULL, 'a' },
			{ "replace", required_argument, NULL, 'c' },
			{ "del", required_argument, NULL, 'd' },
			{ "resize", required_argument, NULL, 'r' },
			{ "leases", no_argument, NULL, 'l' },
			{ "purge", required_argument, NULL, 'p' },
			{ "statusattr", no_argument, NULL, '1' },
			{ "addattr", required_argument, NULL, '2' },
			{ "delattr", required_argument, NULL, '3' },
			{ "showattr", no_argument, NULL, '4' },
			{ "batch", required_argument, NULL, 'b' },

			{ "start", required_argument, NULL, 's' },
			{ "end", required_argument, NULL, 'e' },
			{ "addresses", required_argument, NULL, 'y' },
			{ "timeout", required_argument, NULL, 't' },
			{ "filter", required_argument, NULL, 'f' },
			{ "addr", required_argument, NULL, 'v' },
			{ "mask", required_argument, NULL, 'v' },
			{ "server", required_argument, NULL, 'v' },
			{ "subnet", required_argument, NULL, 'n' },
			{ "string", required_argument, NULL, 'g' },
			{ "hex", required_argument, NULL, 'x' },
			{ "hexout", no_argument, NULL, '5' },
			{ "pool", required_argument, NULL, '6' },
			{ "identity", required_argument, NULL, '7' },
			{ 0,0,0,0 }
		};

		c = getopt_long(argc, argv, "", long_opts, NULL);
		switch (c)
		{
			case EOF:
				break;
			case 'h':
				operation = OP_USAGE;
				break;
			case 'w':
				operation = OP_STATUS;
				break;
			case '1':
				operation = OP_STATUS_ATTR;
				break;
			case 'u':
				utc = TRUE;
				continue;
			case 'c':
				replace_pool = TRUE;
				/* fallthrough */
			case 'a':
				name = optarg;
				operation = is_attribute(name) ? OP_ADD_ATTR : OP_ADD;
				if (replace_pool && operation == OP_ADD_ATTR)
				{
					fprintf(stderr, "invalid pool name: "
									"reserved for '%s' attribute.\n", optarg);
					usage();
					exit(EXIT_FAILURE);
				}
				continue;
			case '2':
				name = optarg;
				operation = OP_ADD_ATTR;
				continue;
			case 'd':
				name = optarg;
				operation = is_attribute(name) ? OP_DEL_ATTR : OP_DEL;
				continue;
			case '3':
				name = optarg;
				operation = OP_DEL_ATTR;
				continue;
			case '4':
				operation = OP_SHOW_ATTR;
				continue;
			case 'r':
				name = optarg;
				operation = OP_RESIZE;
				continue;
			case 'l':
				operation = OP_LEASES;
				continue;
			case 'p':
				name = optarg;
				operation = OP_PURGE;
				continue;
			case 'b':
				name = optarg;
				if (operation == OP_BATCH)
				{
					fprintf(stderr, "--batch commands can not be nested\n");
					exit(EXIT_FAILURE);
				}
				operation = OP_BATCH;
				continue;
			case 's':
				DESTROY_IF(start);
				start = host_create_from_string(optarg, 0);
				if (start == NULL)
				{
					fprintf(stderr, "invalid start address: '%s'.\n", optarg);
					usage();
					exit(EXIT_FAILURE);
				}
				continue;
			case 'e':
				DESTROY_IF(end);
				end = host_create_from_string(optarg, 0);
				if (end == NULL)
				{
					fprintf(stderr, "invalid end address: '%s'.\n", optarg);
					usage();
					exit(EXIT_FAILURE);
				}
				continue;
			case 't':
				if (!timespan_from_string(optarg, "h", &timeout))
				{
					fprintf(stderr, "invalid timeout '%s'.\n", optarg);
					usage();
					exit(EXIT_FAILURE);
				}
				continue;
			case 'f':
				filter = optarg;
				continue;
			case 'y':
				addresses = optarg;
				continue;
			case 'g':
				value_type = VALUE_STRING;
				value = optarg;
				continue;
			case 'n':
				value_type = VALUE_SUBNET;
				value = optarg;
				continue;
			case 'v':
				value_type = VALUE_ADDR;
				value = optarg;
				continue;
			case 'x':
				value_type = VALUE_HEX;
				value = optarg;
				continue;
			case '5':
				hexout = TRUE;
				continue;
			case '6':
				pool = optarg;
				continue;
			case '7':
				identity = optarg;
				continue;
			default:
				usage();
				exit(EXIT_FAILURE);
		}
		break;
	}

	switch (operation)
	{
		case OP_USAGE:
			usage();
			break;
		case OP_STATUS:
			status();
			break;
		case OP_STATUS_ATTR:
			status_attr(hexout);
			break;
		case OP_ADD:
			if (addresses != NULL)
			{
				add_addresses(name, addresses, timeout);
			}
			else if (start != NULL && end != NULL)
			{
				add(name, start, end, timeout);
			}
			else
			{
				fprintf(stderr, "missing arguments.\n");
				usage();
				exit(EXIT_FAILURE);
			}
			break;
		case OP_ADD_ATTR:
			if (value_type == VALUE_NONE)
			{
				fprintf(stderr, "missing arguments.\n");
				usage();
				exit(EXIT_FAILURE);
			}
			if (identity && !pool)
			{
				fprintf(stderr, "--identity option can't be used without --pool.\n");
				usage();
				exit(EXIT_FAILURE);
			}
			add_attr(name, pool, identity, value, value_type);
			break;
		case OP_DEL:
			del(name);
			break;
		case OP_DEL_ATTR:
			if (identity && !pool)
			{
				fprintf(stderr, "--identity option can't be used without --pool.\n");
				usage();
				exit(EXIT_FAILURE);
			}
			del_attr(name, pool, identity, value, value_type);
			break;
		case OP_SHOW_ATTR:
			show_attr();
			break;
		case OP_RESIZE:
			if (end == NULL)
			{
				fprintf(stderr, "missing arguments.\n");
				usage();
				exit(EXIT_FAILURE);
			}
			resize(name, end);
			break;
		case OP_LEASES:
			leases(filter, utc);
			break;
		case OP_PURGE:
			purge(name);
			break;
		case OP_BATCH:
			if (name == NULL)
			{
				fprintf(stderr, "missing arguments.\n");
				usage();
				exit(EXIT_FAILURE);
			}
			batch(argv[0], name);
			break;
		default:
			usage();
			exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	char *uri;

	atexit(library_deinit);

	/* initialize library */
	if (!library_init(NULL, "pool"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "pool", argv[0]))
	{
		fprintf(stderr, "integrity check of pool failed\n");
		exit(SS_RC_DAEMON_INTEGRITY);
	}
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "pool.load", PLUGINS)))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	/* TODO: make database URI or setting key configurable via command line */
	uri = lib->settings->get_str(lib->settings,
			"pool.database",
			lib->settings->get_str(lib->settings,
				"charon.plugins.attr-sql.database",
				lib->settings->get_str(lib->settings,
					"libhydra.plugins.attr-sql.database", NULL)));
	if (!uri)
	{
		fprintf(stderr, "database URI pool.database not set.\n");
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	db = lib->db->create(lib->db, uri);
	if (!db)
	{
		fprintf(stderr, "opening database failed.\n");
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	atexit(cleanup);

	do_args(argc, argv);

	exit(EXIT_SUCCESS);
}
