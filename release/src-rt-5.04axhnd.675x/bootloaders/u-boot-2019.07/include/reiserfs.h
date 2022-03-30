/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright 2000-2002 by Hans Reiser, licensing governed by reiserfs/README
 *
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2000, 2001  Free Software Foundation, Inc.
 *
 *  (C) Copyright 2003 Sysgo Real-Time Solutions, AG <www.elinos.com>
 *  Pavel Bartusek <pba@sysgo.de>
 */

/* An implementation for the ReiserFS filesystem ported from GRUB.
 * Some parts of this code (mainly the structures and defines) are
 * from the original reiser fs code, as found in the linux kernel.
 */


#define SECTOR_SIZE		0x200
#define SECTOR_BITS		9

/* Error codes */
typedef enum
{
  ERR_NONE = 0,
  ERR_BAD_FILENAME,
  ERR_BAD_FILETYPE,
  ERR_BAD_GZIP_DATA,
  ERR_BAD_GZIP_HEADER,
  ERR_BAD_PART_TABLE,
  ERR_BAD_VERSION,
  ERR_BELOW_1MB,
  ERR_BOOT_COMMAND,
  ERR_BOOT_FAILURE,
  ERR_BOOT_FEATURES,
  ERR_DEV_FORMAT,
  ERR_DEV_VALUES,
  ERR_EXEC_FORMAT,
  ERR_FILELENGTH,
  ERR_FILE_NOT_FOUND,
  ERR_FSYS_CORRUPT,
  ERR_FSYS_MOUNT,
  ERR_GEOM,
  ERR_NEED_LX_KERNEL,
  ERR_NEED_MB_KERNEL,
  ERR_NO_DISK,
  ERR_NO_PART,
  ERR_NUMBER_PARSING,
  ERR_OUTSIDE_PART,
  ERR_READ,
  ERR_SYMLINK_LOOP,
  ERR_UNRECOGNIZED,
  ERR_WONT_FIT,
  ERR_WRITE,
  ERR_BAD_ARGUMENT,
  ERR_UNALIGNED,
  ERR_PRIVILEGED,
  ERR_DEV_NEED_INIT,
  ERR_NO_DISK_SPACE,
  ERR_NUMBER_OVERFLOW,

  MAX_ERR_NUM
} reiserfs_error_t;


void reiserfs_set_blk_dev(struct blk_desc *rbdd, disk_partition_t *info);
extern int reiserfs_ls (char *dirname);
extern int reiserfs_open (char *filename);
extern int reiserfs_read (char *buf, unsigned len);
extern int reiserfs_mount (unsigned part_length);
