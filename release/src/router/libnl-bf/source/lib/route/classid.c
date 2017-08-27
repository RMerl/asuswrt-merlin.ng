/*
 * lib/route/classid.c       ClassID Management
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup tc
 * @defgroup classid ClassID Management
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/tc.h>

struct classid_map
{
	uint32_t		classid;
	char *			name;
	struct nl_list_head	name_list;
};

#define CLASSID_NAME_HT_SIZ 256

static struct nl_list_head tbl_name[CLASSID_NAME_HT_SIZ];

static void *id_root = NULL;

static int compare_id(const void *pa, const void *pb)
{
	const struct classid_map *ma = pa;
	const struct classid_map *mb = pb;

	if (ma->classid < mb->classid)
		return -1;
	
	if (ma->classid > mb->classid)
		return 1;

	return 0;
}

/* djb2 */
static unsigned int classid_tbl_hash(const char *str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash % CLASSID_NAME_HT_SIZ;
}

static int classid_lookup(const char *name, uint32_t *result)
{
	struct classid_map *map;
	int n = classid_tbl_hash(name);

	nl_list_for_each_entry(map, &tbl_name[n], name_list) {
		if (!strcasecmp(map->name, name)) {
			*result = map->classid;
			return 0;
		}
	}

	return -NLE_OBJ_NOTFOUND;
}

static char *name_lookup(const uint32_t classid)
{
	void *res;
	struct classid_map cm = {
		.classid = classid,
		.name = "search entry",
	};

	if ((res = tfind(&cm, &id_root, &compare_id)))
		return (*(struct classid_map **) res)->name;

	return NULL;
}

/**
 * @name Traffic Control Handle Translations
 * @{
 */

/**
 * Convert a traffic control handle to a character string (Reentrant).
 * @arg handle		traffic control handle
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a tarffic control handle to a character string in the
 * form of \c MAJ:MIN and stores it in the specified destination buffer.
 *
 * @return The destination buffer or the type encoded in hexidecimal
 *         form if no match was found.
 */
char *rtnl_tc_handle2str(uint32_t handle, char *buf, size_t len)
{
	if (TC_H_ROOT == handle)
		snprintf(buf, len, "root");
	else if (TC_H_UNSPEC == handle)
		snprintf(buf, len, "none");
	else if (TC_H_INGRESS == handle)
		snprintf(buf, len, "ingress");
	else {
		char *name;

		if ((name = name_lookup(handle)))
			snprintf(buf, len, "%s", name);
		else if (0 == TC_H_MAJ(handle))
			snprintf(buf, len, ":%x", TC_H_MIN(handle));
		else if (0 == TC_H_MIN(handle))
			snprintf(buf, len, "%x:", TC_H_MAJ(handle) >> 16);
		else
			snprintf(buf, len, "%x:%x",
				TC_H_MAJ(handle) >> 16, TC_H_MIN(handle));
	}

	return buf;
}

/**
 * Convert a charactering strint to a traffic control handle
 * @arg str		traffic control handle as character string
 * @arg res		destination buffer
 *
 * Converts the provided character string specifying a traffic
 * control handle to the corresponding numeric value.
 *
 * The handle must be provided in one of the following formats:
 *  - NAME
 *  - root
 *  - none
 *  - MAJ:
 *  - :MIN
 *  - NAME:MIN
 *  - MAJ:MIN
 *  - MAJMIN
 *
 * @return 0 on success or a negative error code
 */
int rtnl_tc_str2handle(const char *str, uint32_t *res)
{
	char *colon, *end;
	uint32_t h, err;

	if (!strcasecmp(str, "root")) {
		*res = TC_H_ROOT;
		return 0;
	}

	if (!strcasecmp(str, "none")) {
		*res = TC_H_UNSPEC;
		return 0;
	}

	h = strtoul(str, &colon, 16);

	/* MAJ is not a number */
	if (colon == str) {
not_a_number:
		if (*colon == ':') {
			/* :YYYY */
			h = 0;
		} else {
			size_t len;
			char name[64] = { 0 };

			if (!(colon = strpbrk(str, ":"))) {
				/* NAME */
				return classid_lookup(str, res);
			} else {
				/* NAME:YYYY */
				len = colon - str;
				if (len >= sizeof(name))
					return -NLE_INVAL;

				memcpy(name, str, len);

				if ((err = classid_lookup(name, &h)) < 0)
					return err;

				/* Name must point to a qdisc alias */
				if (TC_H_MIN(h))
					return -NLE_INVAL;

				/* NAME: is not allowed */
				if (colon[1] == '\0')
					return -NLE_INVAL;

				goto update;
			}
		}
	}

	if (':' == *colon) {
		/* check if we would lose bits */
		if (TC_H_MAJ(h))
			return -NLE_RANGE;
		h <<= 16;

		if ('\0' == colon[1]) {
			/* XXXX: */
			*res = h;
		} else {
			/* XXXX:YYYY */
			uint32_t l;
			
update:
			l = strtoul(colon+1, &end, 16);

			/* check if we overlap with major part */
			if (TC_H_MAJ(l))
				return -NLE_RANGE;

			if ('\0' != *end)
				return -NLE_INVAL;

			*res = (h | l);
		}
	} else if ('\0' == *colon) {
		/* XXXXYYYY */
		*res = h;
	} else
		goto not_a_number;

	return 0;
}

static void free_nothing(void *arg)
{
}

static void classid_map_free(struct classid_map *map)
{
	if (!map)
		return;

	free(map->name);
	free(map);
}

static void clear_hashtable(void)
{
	int i;

	for (i = 0; i < CLASSID_NAME_HT_SIZ; i++) {
		struct classid_map *map, *n;

		nl_list_for_each_entry_safe(map, n, &tbl_name[i], name_list)
			classid_map_free(map);

		nl_init_list_head(&tbl_name[i]);

	}

	if (id_root) {
		tdestroy(&id_root, &free_nothing);
		id_root = NULL;
	}
}

static int classid_map_add(uint32_t classid, const char *name)
{
	struct classid_map *map;
	int n;

	if (!(map = calloc(1, sizeof(*map))))
		return -NLE_NOMEM;

	map->classid = classid;
	map->name = strdup(name);

	n = classid_tbl_hash(map->name);
	nl_list_add_tail(&map->name_list, &tbl_name[n]);

	if (!tsearch((void *) map, &id_root, &compare_id)) {
		classid_map_free(map);
		return -NLE_NOMEM;
	}

	return 0;
}

/**
 * (Re-)read classid file
 * 
 * Rereads the contents of the classid file (typically found at the location
 * /etc/libnl/classid) and refreshes the classid maps.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_tc_read_classid_file(void)
{
	static time_t last_read;
	struct stat st = {0};
	char buf[256], *path;
	FILE *fd;
	int err;

	if (build_sysconf_path(&path, "classid") < 0)
		return -NLE_NOMEM;

	/* if stat fails, just (re-)read the file */
	if (stat(path, &st) == 0) {
		/* Don't re-read file if file is unchanged */
		if (last_read == st.st_mtime) {
			err = 0;
			goto errout;
		}
	}

	if (!(fd = fopen(path, "r"))) {
		err = -nl_syserr2nlerr(errno);
		goto errout;
	}

	clear_hashtable();

	while (fgets(buf, sizeof(buf), fd)) {
		uint32_t classid;
		char *ptr, *tok;

		/* ignore comments and empty lines */
		if (*buf == '#' || *buf == '\n' || *buf == '\r')
			continue;

		/* token 1 */
		if (!(tok = strtok_r(buf, " \t", &ptr))) {
			err = -NLE_INVAL;
			goto errout_close;
		}

		if ((err = rtnl_tc_str2handle(tok, &classid)) < 0)
			goto errout_close;

		if (!(tok = strtok_r(NULL, " \t\n\r#", &ptr))) {
			err = -NLE_INVAL;
			goto errout_close;
		}

		if ((err = classid_map_add(classid, tok)) < 0)
			goto errout_close;
	}

	err = 0;
	last_read = st.st_mtime;

errout_close:
	fclose(fd);
errout:
	free(path);

	return err;

}

int rtnl_classid_generate(const char *name, uint32_t *result, uint32_t parent)
{
	static uint32_t base = 0x4000 << 16;
	uint32_t classid;
	char *path;
	FILE *fd;
	int err = 0;

	if (parent == TC_H_ROOT || parent == TC_H_INGRESS) {
		do {
			base += (1 << 16);
			if (base == TC_H_MAJ(TC_H_ROOT))
				base = 0x4000 << 16;
		} while (name_lookup(base));

		classid = base;
	} else {
		classid = TC_H_MAJ(parent);
		do {
			if (TC_H_MIN(++classid) == TC_H_MIN(TC_H_ROOT))
				return -NLE_RANGE;
		} while (name_lookup(classid));
	}

	NL_DBG(2, "Generated new classid %#x\n", classid);

	if (build_sysconf_path(&path, "classid") < 0)
		return -NLE_NOMEM;

	if (!(fd = fopen(path, "a"))) {
		err = -nl_syserr2nlerr(errno);
		goto errout;
	}

	fprintf(fd, "%x:", TC_H_MAJ(classid) >> 16);
	if (TC_H_MIN(classid))
		fprintf(fd, "%x", TC_H_MIN(classid));
	fprintf(fd, "\t\t\t%s\n", name);

	fclose(fd);

	if ((err = classid_map_add(classid, name)) < 0) {
		/* 
		 * Error adding classid map, re-read classid file is best
		 * option here. It is likely to fail as well but better
		 * than nothing, entry was added to the file already anyway.
		 */
		rtnl_tc_read_classid_file();
	}

	*result = classid;
	err = 0;
errout:
	free(path);

	return err;
}

/** @} */

static void __init classid_init(void)
{
	int err, i;

	for (i = 0; i < CLASSID_NAME_HT_SIZ; i++)
		nl_init_list_head(&tbl_name[i]);

	if ((err = rtnl_tc_read_classid_file()) < 0)
		fprintf(stderr, "Failed to read classid file: %s\n", nl_geterror(err));
}

static void __exit classid_exit(void)
{
	void free_map(void *map) {
		free(((struct classid_map *)map)->name);
		free(map);
	};
	tdestroy(id_root, free_map);
}

/** @} */
