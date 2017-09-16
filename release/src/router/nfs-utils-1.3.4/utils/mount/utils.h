/*
 * utils.h -- misc utils for mount and umount
 *
 * Copyright (C) 2010 Karel Zak <kzak@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
 *
 */

#ifndef _NFS_UTILS_MOUNT_UTILS_H
#define _NFS_UTILS_MOUNT_UTILS_H

#include "parse_opt.h"

int discover_nfs_mount_data_version(int *string_ver);
void print_one(char *spec, char *node, char *type, char *opts);
void mount_usage(void);
void umount_usage(void);
int chk_mountpoint(const char *mount_point);

int nfs_umount23(const char *devname, char *string);

#endif	/* !_NFS_UTILS_MOUNT_UTILS_H */
