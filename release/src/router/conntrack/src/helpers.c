/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (or any later at your option).
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "helper.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

static LIST_HEAD(helper_list);

void helper_register(struct ctd_helper *helper)
{
	list_add(&helper->head, &helper_list);
}

static struct ctd_helper *
__helper_find(const char *helper_name, uint8_t l4proto)
{
	struct ctd_helper *cur, *helper = NULL;

	list_for_each_entry(cur, &helper_list, head) {
		if (strncmp(cur->name, helper_name, CTD_HELPER_NAME_LEN) != 0)
			continue;

		if (cur->l4proto != l4proto)
			continue;

		helper = cur;
		break;
	}
	return helper;
}

struct ctd_helper *
helper_find(const char *libdir_path,
	    const char *helper_name, uint8_t l4proto, int flag)
{
	char path[PATH_MAX];
	struct ctd_helper *helper;
	struct stat sb;

	helper = __helper_find(helper_name, l4proto);
	if (helper != NULL)
		return helper;

	snprintf(path, sizeof(path), "%s/ct_helper_%s.so",
		libdir_path, helper_name);

	if (stat(path, &sb) != 0) {
		if (errno == ENOENT)
			return NULL;
		fprintf(stderr, "%s: %s\n", path,
			strerror(errno));
		return NULL;
	}

	if (dlopen(path, flag) == NULL) {
		fprintf(stderr, "%s: %s\n", path, dlerror());
		return NULL;
	}

	return __helper_find(helper_name, l4proto);
}
