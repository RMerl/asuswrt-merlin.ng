/*
 * lib/route/pktloc.c     Packet Location Aliasing
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup tc
 * @defgroup pktloc Packet Location Aliasing
 * Packet Location Aliasing
 *
 * The packet location aliasing interface eases the use of offset definitions
 * inside packets by allowing them to be referenced by name. Known positions
 * of protocol fields are stored in a configuration file and associated with
 * a name for later reference. The configuration file is distributed with the
 * library and provides a well defined set of definitions for most common
 * protocol fields.
 *
 * @section pktloc_examples Examples
 * @par Example 1.1 Looking up a packet location
 * @code
 * struct rtnl_pktloc *loc;
 *
 * rtnl_pktloc_lookup("ip.src", &loc);
 * @endcode
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/pktloc.h>

#include "pktloc_syntax.h"
#include "pktloc_grammar.h"

/** @cond SKIP */
#define PKTLOC_NAME_HT_SIZ 256

static struct nl_list_head pktloc_name_ht[PKTLOC_NAME_HT_SIZ];

/* djb2 */
static unsigned int pktloc_hash(const char *str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash % PKTLOC_NAME_HT_SIZ;
}

static int __pktloc_lookup(const char *name, struct rtnl_pktloc **result)
{
	struct rtnl_pktloc *loc;
	int hash;

	hash = pktloc_hash(name);
	nl_list_for_each_entry(loc, &pktloc_name_ht[hash], list) {
		if (!strcasecmp(loc->name, name)) {
			loc->refcnt++;
			*result = loc;
			return 0;
		}
	}

	return -NLE_OBJ_NOTFOUND;
}

extern int pktloc_parse(void *scanner);

static void rtnl_pktloc_free(struct rtnl_pktloc *loc)
{
	if (!loc)
		return;

	free(loc->name);
	free(loc);
}

static int read_pktlocs(void)
{
	YY_BUFFER_STATE buf = NULL;
	yyscan_t scanner = NULL;
	static time_t last_read;
	struct stat st;
	char *path;
	int i, err;
	FILE *fd;

	if (build_sysconf_path(&path, "pktloc") < 0)
		return -NLE_NOMEM;

	/* if stat fails, just try to read the file */
	if (stat(path, &st) == 0) {
		/* Don't re-read file if file is unchanged */
		if (last_read == st.st_mtime)
			return 0;
	}

	NL_DBG(2, "Reading packet location file \"%s\"\n", path);

	if (!(fd = fopen(path, "r"))) {
		err = -NLE_PKTLOC_FILE;
		goto errout;
	}

	for (i = 0; i < PKTLOC_NAME_HT_SIZ; i++) {
		struct rtnl_pktloc *loc, *n;

		nl_list_for_each_entry_safe(loc, n, &pktloc_name_ht[i], list)
			rtnl_pktloc_put(loc);

		nl_init_list_head(&pktloc_name_ht[i]);
	}

	if ((err = pktloc_lex_init(&scanner)) < 0) {
		err = -NLE_FAILURE;
		goto errout_close;
	}

	buf = pktloc__create_buffer(fd, YY_BUF_SIZE, scanner);
	pktloc__switch_to_buffer(buf, scanner);

	if ((err = pktloc_parse(scanner)) != 0) {
		pktloc__delete_buffer(buf, scanner);
		err = -NLE_PARSE_ERR;
		goto errout_scanner;
	}

	last_read = st.st_mtime;

errout_scanner:
	pktloc_lex_destroy(scanner);
errout_close:
	fclose(fd);
errout:
	free(path);

	return err;
}

/** @endcond */

/**
 * Lookup packet location alias
 * @arg name		Name of packet location.
 * @arg result		Result pointer
 *
 * Tries to find a matching packet location alias for the supplied
 * packet location name.
 *
 * The file containing the packet location definitions is automatically
 * re-read if its modification time has changed since the last call.
 *
 * The returned packet location has to be returned after use by calling
 * rtnl_pktloc_put() in order to allow freeing its memory after the last
 * user has abandoned it.
 *
 * @return 0 on success or a negative error code.
 * @retval NLE_PKTLOC_FILE Unable to open packet location file.
 * @retval NLE_OBJ_NOTFOUND No matching packet location alias found.
 */
int rtnl_pktloc_lookup(const char *name, struct rtnl_pktloc **result)
{
	int err;

	if ((err = read_pktlocs()) < 0)
		return err;
	
	return __pktloc_lookup(name, result);
}

/**
 * Allocate packet location object
 */
struct rtnl_pktloc *rtnl_pktloc_alloc(void)
{
	struct rtnl_pktloc *loc;

	if (!(loc = calloc(1, sizeof(*loc))))
		return NULL;

	loc->refcnt = 1;
	nl_init_list_head(&loc->list);

	return loc;
}

/**
 * Return reference of a packet location
 * @arg loc		packet location object.
 */
void rtnl_pktloc_put(struct rtnl_pktloc *loc)
{
	if (!loc)
		return;

	loc->refcnt--;
	if (loc->refcnt <= 0)
		rtnl_pktloc_free(loc);
}

/**
 * Add a packet location to the hash table
 * @arg loc		packet location object
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_pktloc_add(struct rtnl_pktloc *loc)
{
	struct rtnl_pktloc *l;

	if (__pktloc_lookup(loc->name, &l) == 0) {
		rtnl_pktloc_put(l);
		return -NLE_EXIST;
	}

	NL_DBG(2, "New packet location entry \"%s\" align=%u layer=%u "
		  "offset=%u mask=%#x shift=%u refnt=%u\n",
		  loc->name, loc->align, loc->layer, loc->offset,
		  loc->mask, loc->shift, loc->refcnt);

	nl_list_add_tail(&loc->list, &pktloc_name_ht[pktloc_hash(loc->name)]);

	return 0;
}

void rtnl_pktloc_foreach(void (*cb)(struct rtnl_pktloc *, void *), void *arg)
{
	struct rtnl_pktloc *loc;
	int i;

	/* ignore errors */
	read_pktlocs();

	for (i = 0; i < PKTLOC_NAME_HT_SIZ; i++)
		nl_list_for_each_entry(loc, &pktloc_name_ht[i], list)
			cb(loc, arg);
}

static int __init pktloc_init(void)
{
	int i;

	for (i = 0; i < PKTLOC_NAME_HT_SIZ; i++)
		nl_init_list_head(&pktloc_name_ht[i]);
	
	return 0;
}

/** @} */
