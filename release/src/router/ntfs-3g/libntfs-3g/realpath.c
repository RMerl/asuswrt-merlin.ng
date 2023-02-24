/*
 * realpath.c - realpath() aware of device mapper
 * Originated from the util-linux project.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#include "param.h"
#include "realpath.h"

/* If there is no realpath() on the system, provide a dummy one. */
#ifndef HAVE_REALPATH
char *ntfs_realpath(const char *path, char *resolved_path)
{
       strncpy(resolved_path, path, PATH_MAX);
       resolved_path[PATH_MAX] = '\0';
       return resolved_path;
}
#endif


#ifdef linux

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 */
static char *
canonicalize_dm_name(const char *ptname, char *canonical)
{
	FILE	*f;
	size_t	sz;
	char	name[MAPPERNAMELTH + 16];
	char	path[sizeof(name) + 16];
	char	*res = NULL;

	snprintf(path, sizeof(path), "/sys/block/%s/dm/name", ptname);
	if (!(f = fopen(path, "r")))
		return NULL;

	/* read "<name>\n" from sysfs */
	if (fgets(name, sizeof(name), f) && (sz = strlen(name)) > 1) {
		name[sz - 1] = '\0';
		snprintf(path, sizeof(path), "/dev/mapper/%s", name);
		res = strcpy(canonical, path);
	}
	fclose(f);
	return res;
}

/*
 *		Canonicalize a device path
 *
 *	Workaround from "basinilya" for fixing device mapper paths.
 *
 *  Background (Phillip Susi, 2011-04-09)
 *	- ntfs-3g canonicalizes the device name so that if you mount with
 *	  /dev/mapper/foo, the device name listed in mtab is /dev/dm-n,
 *	  so you can not umount /dev/mapper/foo
 *	- umount won't even recognize and translate /dev/dm-n to the mount
 *	  point, apparently because of the '-' involved. Editing mtab and
 *	  removing the '-' allows you to umount /dev/dmn successfully.
 *
 *	This code restores the devmapper name after canonicalization,
 *	until a proper fix is implemented.
 */

char *ntfs_realpath_canonicalize(const char *path, char *canonical)
{
	char *p;

	if (path == NULL)
		return NULL;

	if (!ntfs_realpath(path, canonical))
		return NULL;

	p = strrchr(canonical, '/');
	if (p && strncmp(p, "/dm-", 4) == 0 && isdigit(*(p + 4))) {
		p = canonicalize_dm_name(p+1, canonical);
		if (p)
			return p;
	}

	return canonical;
}

#endif
