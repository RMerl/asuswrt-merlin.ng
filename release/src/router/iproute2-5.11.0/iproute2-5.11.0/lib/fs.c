/*
 * fs.c         filesystem APIs
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	David Ahern <dsa@cumulusnetworks.com>
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "utils.h"

#define CGROUP2_FS_NAME "cgroup2"

/* if not already mounted cgroup2 is mounted here for iproute2's use */
#define MNT_CGRP2_PATH  "/var/run/cgroup2"

/* return mount path of first occurrence of given fstype */
static char *find_fs_mount(const char *fs_to_find)
{
	char path[4096];
	char fstype[128];    /* max length of any filesystem name */
	char *mnt = NULL;
	FILE *fp;

	fp = fopen("/proc/mounts", "r");
	if (!fp) {
		fprintf(stderr,
			"Failed to open mounts file: %s\n", strerror(errno));
		return NULL;
	}

	while (fscanf(fp, "%*s %4095s %127s %*s %*d %*d\n",
		      path, fstype) == 2) {
		if (strcmp(fstype, fs_to_find) == 0) {
			mnt = strdup(path);
			break;
		}
	}

	fclose(fp);

	return mnt;
}

/* caller needs to free string returned */
char *find_cgroup2_mount(bool do_mount)
{
	char *mnt = find_fs_mount(CGROUP2_FS_NAME);

	if (mnt)
		return mnt;

	if (!do_mount) {
		fprintf(stderr, "Failed to find cgroup2 mount\n");
		return NULL;
	}

	mnt = strdup(MNT_CGRP2_PATH);
	if (!mnt) {
		fprintf(stderr, "Failed to allocate memory for cgroup2 path\n");
		return NULL;

	}

	if (make_path(mnt, 0755)) {
		fprintf(stderr, "Failed to setup cgroup2 directory\n");
		free(mnt);
		return NULL;
	}

	if (mount("none", mnt, CGROUP2_FS_NAME, 0, NULL)) {
		/* EBUSY means already mounted */
		if (errno == EBUSY)
			goto out;

		if (errno == ENODEV) {
			fprintf(stderr,
				"Failed to mount cgroup2. Are CGROUPS enabled in your kernel?\n");
		} else {
			fprintf(stderr,
				"Failed to mount cgroup2: %s\n",
				strerror(errno));
		}
		free(mnt);
		return NULL;
	}
out:
	return mnt;
}

__u64 get_cgroup2_id(const char *path)
{
	char fh_buf[sizeof(struct file_handle) + sizeof(__u64)] = { 0 };
	struct file_handle *fhp = (struct file_handle *)fh_buf;
	union {
		__u64 id;
		unsigned char bytes[sizeof(__u64)];
	} cg_id = { .id = 0 };
	char *mnt = NULL;
	int mnt_fd = -1;
	int mnt_id;

	if (!path) {
		fprintf(stderr, "Invalid cgroup2 path\n");
		return 0;
	}

	fhp->handle_bytes = sizeof(__u64);
	if (name_to_handle_at(AT_FDCWD, path, fhp, &mnt_id, 0) < 0) {
		/* try at cgroup2 mount */

		while (*path == '/')
			path++;
		if (*path == '\0') {
			fprintf(stderr, "Invalid cgroup2 path\n");
			goto out;
		}

		mnt = find_cgroup2_mount(false);
		if (!mnt)
			goto out;

		mnt_fd = open(mnt, O_RDONLY);
		if (mnt_fd < 0) {
			fprintf(stderr, "Failed to open cgroup2 mount\n");
			goto out;
		}

		fhp->handle_bytes = sizeof(__u64);
		if (name_to_handle_at(mnt_fd, path, fhp, &mnt_id, 0) < 0) {
			fprintf(stderr, "Failed to get cgroup2 ID: %s\n",
					strerror(errno));
			goto out;
		}
	}
	if (fhp->handle_bytes != sizeof(__u64)) {
		fprintf(stderr, "Invalid size of cgroup2 ID\n");
		goto out;
	}

	memcpy(cg_id.bytes, fhp->f_handle, sizeof(__u64));

out:
	if (mnt_fd >= 0)
		close(mnt_fd);
	free(mnt);

	return cg_id.id;
}

#define FILEID_INO32_GEN 1

/* caller needs to free string returned */
char *get_cgroup2_path(__u64 id, bool full)
{
	char fh_buf[sizeof(struct file_handle) + sizeof(__u64)] = { 0 };
	struct file_handle *fhp = (struct file_handle *)fh_buf;
	union {
		__u64 id;
		unsigned char bytes[sizeof(__u64)];
	} cg_id = { .id = id };
	int mnt_fd = -1, fd = -1;
	char link_buf[PATH_MAX];
	char *path = NULL;
	char fd_path[64];
	int link_len;
	char *mnt = NULL;

	if (!id) {
		fprintf(stderr, "Invalid cgroup2 ID\n");
		goto out;
	}

	mnt = find_cgroup2_mount(false);
	if (!mnt)
		goto out;

	mnt_fd = open(mnt, O_RDONLY);
	if (mnt_fd < 0) {
		fprintf(stderr, "Failed to open cgroup2 mount\n");
		goto out;
	}

	fhp->handle_bytes = sizeof(__u64);
	fhp->handle_type = FILEID_INO32_GEN;
	memcpy(fhp->f_handle, cg_id.bytes, sizeof(__u64));

	fd = open_by_handle_at(mnt_fd, fhp, 0);
	if (fd < 0) {
		fprintf(stderr, "Failed to open cgroup2 by ID\n");
		goto out;
	}

	snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d", fd);
	link_len = readlink(fd_path, link_buf, sizeof(link_buf) - 1);
	if (link_len < 0) {
		fprintf(stderr,
			"Failed to read value of symbolic link %s\n",
			fd_path);
		goto out;
	}
	link_buf[link_len] = '\0';

	if (full)
		path = strdup(link_buf);
	else
		path = strdup(link_buf + strlen(mnt));
	if (!path)
		fprintf(stderr,
			"Failed to allocate memory for cgroup2 path\n");

out:
	if (fd >= 0)
		close(fd);
	if (mnt_fd >= 0)
		close(mnt_fd);
	free(mnt);

	return path;
}

int make_path(const char *path, mode_t mode)
{
	char *dir, *delim;
	int rc = -1;

	delim = dir = strdup(path);
	if (dir == NULL) {
		fprintf(stderr, "strdup failed copying path");
		return -1;
	}

	/* skip '/' -- it had better exist */
	if (*delim == '/')
		delim++;

	while (1) {
		delim = strchr(delim, '/');
		if (delim)
			*delim = '\0';

		rc = mkdir(dir, mode);
		if (rc && errno != EEXIST) {
			fprintf(stderr, "mkdir failed for %s: %s\n",
				dir, strerror(errno));
			goto out;
		}

		if (delim == NULL)
			break;

		*delim = '/';
		delim++;
		if (*delim == '\0')
			break;
	}
	rc = 0;
out:
	free(dir);

	return rc;
}

int get_command_name(const char *pid, char *comm, size_t len)
{
	char path[PATH_MAX];
	char line[128];
	FILE *fp;

	if (snprintf(path, sizeof(path),
		     "/proc/%s/status", pid) >= sizeof(path)) {
		return -1;
	}

	fp = fopen(path, "r");
	if (!fp)
		return -1;

	comm[0] = '\0';
	while (fgets(line, sizeof(line), fp)) {
		char *nl, *name;

		name = strstr(line, "Name:");
		if (!name)
			continue;

		name += 5;
		while (isspace(*name))
			name++;

		nl = strchr(name, '\n');
		if (nl)
			*nl = '\0';

		strlcpy(comm, name, len);
		break;
	}

	fclose(fp);

	return 0;
}
