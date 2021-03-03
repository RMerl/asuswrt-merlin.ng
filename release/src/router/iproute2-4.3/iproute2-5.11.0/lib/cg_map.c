/*
 * cg_map.c	cgroup v2 cache
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Dmitry Yakunin <zeil@yandex-team.ru>
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <linux/types.h>
#include <linux/limits.h>
#include <ftw.h>

#include "cg_map.h"
#include "list.h"
#include "utils.h"

struct cg_cache {
	struct hlist_node id_hash;
	__u64	id;
	char	path[];
};

#define IDMAP_SIZE	1024
static struct hlist_head id_head[IDMAP_SIZE];

static struct cg_cache *cg_get_by_id(__u64 id)
{
	unsigned int h = id & (IDMAP_SIZE - 1);
	struct hlist_node *n;

	hlist_for_each(n, &id_head[h]) {
		struct cg_cache *cg;

		cg = container_of(n, struct cg_cache, id_hash);
		if (cg->id == id)
			return cg;
	}

	return NULL;
}

static struct cg_cache *cg_entry_create(__u64 id, const char *path)
{
	unsigned int h = id & (IDMAP_SIZE - 1);
	struct cg_cache *cg;

	cg = malloc(sizeof(*cg) + strlen(path) + 1);
	if (!cg) {
		fprintf(stderr,
			"Failed to allocate memory for cgroup2 cache entry");
		return NULL;
	}
	cg->id = id;
	strcpy(cg->path, path);

	hlist_add_head(&cg->id_hash, &id_head[h]);

	return cg;
}

static int mntlen;

static int nftw_fn(const char *fpath, const struct stat *sb,
		   int typeflag, struct FTW *ftw)
{
	const char *path;
	__u64 id;

	if (typeflag != FTW_D)
		return 0;

	id = get_cgroup2_id(fpath);
	if (!id)
		return -1;

	path = fpath + mntlen;
	if (*path == '\0')
		/* root cgroup */
		path = "/";
	if (!cg_entry_create(id, path))
		return -1;

	return 0;
}

static void cg_init_map(void)
{
	char *mnt;

	mnt = find_cgroup2_mount(false);
	if (!mnt)
		return;

	mntlen = strlen(mnt);
	(void) nftw(mnt, nftw_fn, 1024, FTW_MOUNT);

	free(mnt);
}

const char *cg_id_to_path(__u64 id)
{
	static int initialized;
	static char buf[64];

	const struct cg_cache *cg;
	char *path;

	if (!initialized) {
		cg_init_map();
		initialized = 1;
	}

	cg = cg_get_by_id(id);
	if (cg)
		return cg->path;

	path = get_cgroup2_path(id, false);
	if (path) {
		cg = cg_entry_create(id, path);
		free(path);
		if (cg)
			return cg->path;
	}

	snprintf(buf, sizeof(buf), "unreachable:%llx", id);
	return buf;
}
