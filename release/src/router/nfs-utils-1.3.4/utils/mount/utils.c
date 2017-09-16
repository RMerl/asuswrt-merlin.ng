/*
 * Copyright (C) 2010 Karel Zak <kzak@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sockaddr.h"
#include "nfs_mount.h"
#include "nls.h"
#include "xcommon.h"
#include "version.h"
#include "error.h"
#include "utils.h"
#include "mount.h"
#include "network.h"
#include "parse_dev.h"

extern int verbose;
extern char *progname;

/*
 * Choose the version of the nfs_mount_data structure that is appropriate
 * for the kernel that is doing the mount.
 *
 * NFS_MOUNT_VERSION:		maximum version supported by these sources
 * nfs_mount_data_version:	maximum version supported by the running kernel
 */
int discover_nfs_mount_data_version(int *string_ver)
{
	unsigned int kernel_version = linux_version_code();
	int ver = 0;

	*string_ver = 0;

	if (kernel_version) {
		if (kernel_version < MAKE_VERSION(2, 1, 32))
			ver = 1;
		else if (kernel_version < MAKE_VERSION(2, 2, 18))
			ver = 3;
		else if (kernel_version < MAKE_VERSION(2, 3, 0))
			ver = 4;
		else if (kernel_version < MAKE_VERSION(2, 3, 99))
			ver = 3;
		else if (kernel_version < MAKE_VERSION(2, 6, 3))
			ver = 4;
		else
			ver = 6;
	}
	if (ver > NFS_MOUNT_VERSION)
		ver = NFS_MOUNT_VERSION;
	else
		if (kernel_version > MAKE_VERSION(2, 6, 22))
			(*string_ver)++;

	return ver;
}

void print_one(char *spec, char *node, char *type, char *opts)
{
	if (!verbose)
		return;

	if (opts)
		printf(_("%s on %s type %s (%s)\n"), spec, node, type, opts);
	else
		printf(_("%s on %s type %s\n"), spec, node, type);
}

void mount_usage(void)
{
	printf(_("usage: %s remotetarget dir [-rvVwfnsh] [-o nfsoptions]\n"),
		progname);
	printf(_("options:\n"));
	printf(_("\t-r\t\tMount file system readonly\n"));
	printf(_("\t-v\t\tVerbose\n"));
	printf(_("\t-V\t\tPrint version\n"));
	printf(_("\t-w\t\tMount file system read-write\n"));
	printf(_("\t-f\t\tFake mount, do not actually mount\n"));
	printf(_("\t-n\t\tDo not update /etc/mtab\n"));
	printf(_("\t-s\t\tTolerate sloppy mount options rather than fail\n"));
	printf(_("\t-h\t\tPrint this help\n"));
	printf(_("\tnfsoptions\tRefer to mount.nfs(8) or nfs(5)\n\n"));
}

void umount_usage(void)
{
	printf(_("usage: %s dir [-fvnrlh]\n"), progname);
	printf(_("options:\n\t-f\tforce unmount\n"));
	printf(_("\t-v\tverbose\n"));
	printf(_("\t-n\tDo not update /etc/mtab\n"));
	printf(_("\t-r\tremount\n"));
	printf(_("\t-l\tlazy unmount\n"));
	printf(_("\t-h\tprint this help\n\n"));
}

int chk_mountpoint(const char *mount_point)
{
	struct stat sb;

	if (stat(mount_point, &sb) < 0){
		mount_error(NULL, mount_point, errno);
		return 1;
	}
	if (S_ISDIR(sb.st_mode) == 0){
		mount_error(NULL, mount_point, ENOTDIR);
		return 1;
	}
	if (getuid() != 0 && geteuid() != 0 && access(mount_point, X_OK) < 0) {
		mount_error(NULL, mount_point, errno);
		return 1;
	}

	return 0;
}

/*
 * Pick up certain mount options used during the original mount
 * from /etc/mtab.  The basics include the server's IP address and
 * the server pathname of the share to unregister.
 *
 * These options might also describe the mount port, mount protocol
 * version, and transport protocol used to punch through a firewall.
 * We will need this information to get through the firewall again
 * to do the umount.
 *
 * Note that option parsing failures won't necessarily cause the
 * umount request to fail.  Those values will be left zero in the
 * pmap tuple.  If the GETPORT call later fails to disambiguate them,
 * then we fail.
 */
int nfs_umount23(const char *devname, char *string)
{
	char *hostname = NULL, *dirname = NULL;
	struct mount_options *options;
	int result = EX_FAIL;

	if (!nfs_parse_devname(devname, &hostname, &dirname))
		return EX_USAGE;

	options = po_split(string);
	if (options) {
		result = nfs_umount_do_umnt(options, &hostname, &dirname);
		po_destroy(options);
	} else
		nfs_error(_("%s: option parsing error"), progname);

	free(hostname);
	free(dirname);
	return result;
}
