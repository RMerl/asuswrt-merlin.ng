/*
 * namespace.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include <sys/statvfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>

#include "utils.h"
#include "namespace.h"

static void bind_etc(const char *name)
{
	char etc_netns_path[sizeof(NETNS_ETC_DIR) + NAME_MAX];
	char netns_name[PATH_MAX];
	char etc_name[PATH_MAX];
	struct dirent *entry;
	DIR *dir;

	if (strlen(name) >= NAME_MAX)
		return;

	snprintf(etc_netns_path, sizeof(etc_netns_path), "%s/%s", NETNS_ETC_DIR, name);
	dir = opendir(etc_netns_path);
	if (!dir)
		return;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		snprintf(netns_name, sizeof(netns_name), "%s/%s", etc_netns_path, entry->d_name);
		snprintf(etc_name, sizeof(etc_name), "/etc/%s", entry->d_name);
		if (mount(netns_name, etc_name, "none", MS_BIND, NULL) < 0) {
			fprintf(stderr, "Bind %s -> %s failed: %s\n",
				netns_name, etc_name, strerror(errno));
		}
	}
	closedir(dir);
}

int netns_switch(char *name)
{
	char net_path[PATH_MAX];
	int netns;
	unsigned long mountflags = 0;
	struct statvfs fsstat;

	snprintf(net_path, sizeof(net_path), "%s/%s", NETNS_RUN_DIR, name);
	netns = open(net_path, O_RDONLY | O_CLOEXEC);
	if (netns < 0) {
		fprintf(stderr, "Cannot open network namespace \"%s\": %s\n",
			name, strerror(errno));
		return -1;
	}

	if (setns(netns, CLONE_NEWNET) < 0) {
		fprintf(stderr, "setting the network namespace \"%s\" failed: %s\n",
			name, strerror(errno));
		close(netns);
		return -1;
	}
	close(netns);

	if (unshare(CLONE_NEWNS) < 0) {
		fprintf(stderr, "unshare failed: %s\n", strerror(errno));
		return -1;
	}
	/* Don't let any mounts propagate back to the parent */
	if (mount("", "/", "none", MS_SLAVE | MS_REC, NULL)) {
		fprintf(stderr, "\"mount --make-rslave /\" failed: %s\n",
			strerror(errno));
		return -1;
	}

	/* Mount a version of /sys that describes the network namespace */

	if (umount2("/sys", MNT_DETACH) < 0) {
		/* If this fails, perhaps there wasn't a sysfs instance mounted. Good. */
		if (statvfs("/sys", &fsstat) == 0) {
			/* We couldn't umount the sysfs, we'll attempt to overlay it.
			 * A read-only instance can't be shadowed with a read-write one. */
			if (fsstat.f_flag & ST_RDONLY)
				mountflags = MS_RDONLY;
		}
	}
	if (mount(name, "/sys", "sysfs", mountflags, NULL) < 0) {
		fprintf(stderr, "mount of /sys failed: %s\n",strerror(errno));
		return -1;
	}

	/* Setup bind mounts for config files in /etc */
	bind_etc(name);
	return 0;
}

int netns_get_fd(const char *name)
{
	char pathbuf[PATH_MAX];
	const char *path, *ptr;

	path = name;
	ptr = strchr(name, '/');
	if (!ptr) {
		snprintf(pathbuf, sizeof(pathbuf), "%s/%s",
			NETNS_RUN_DIR, name );
		path = pathbuf;
	}
	return open(path, O_RDONLY);
}

int netns_foreach(int (*func)(char *nsname, void *arg), void *arg)
{
	DIR *dir;
	struct dirent *entry;

	dir = opendir(NETNS_RUN_DIR);
	if (!dir) {
		if (errno == ENOENT)
			return 0;

		fprintf(stderr, "Failed to open directory %s: %s\n",
			NETNS_RUN_DIR, strerror(errno));
		return -1;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		if (func(entry->d_name, arg))
			break;
	}

	closedir(dir);
	return 0;
}
