/*
 *
 * Copyright (c) 2014 Jean-Pierre Andre
 *
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef IOCTL_H
#define IOCTL_H

/*
 * Using an "unsigned long cmd" internally, like in <sys/ioctl.h> for Linux
 * Note however that fuse truncates the arg to 32 bits, and that
 * some commands (e.g. FITRIM) do not fit in a signed 32 bit field.
 */
int ntfs_ioctl(ntfs_inode *ni, unsigned long cmd, void *arg,
                        unsigned int flags, void *data);

#endif /* IOCTL_H */
