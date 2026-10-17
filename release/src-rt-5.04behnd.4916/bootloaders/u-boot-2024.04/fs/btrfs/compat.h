// SPDX-License-Identifier: GPL-2.0+

#ifndef __BTRFS_COMPAT_H__
#define __BTRFS_COMPAT_H__

#include <linux/errno.h>
#include <fs_internal.h>
#include <uuid.h>

/* Provide a compatibility layer to make code syncing easier */

/* A simple wraper to for error() used in btrfs-progs */
#define error(fmt, ...)		pr_err("BTRFS: " fmt "\n", ##__VA_ARGS__)

#define ASSERT(c) assert(c)

#define BTRFS_UUID_UNPARSED_SIZE	37

/* No <linux/limits.h> so have to define it here */
#define XATTR_NAME_MAX		255
#define PATH_MAX		4096

/*
 * Macros to generate set/get funcs for the struct fields
 * assume there is a lefoo_to_cpu for every type, so lets make a simple
 * one for u8:
 */
#define le8_to_cpu(v) (v)
#define cpu_to_le8(v) (v)
#define __le8 u8

/*
 * Macros to generate set/get funcs for the struct fields
 * assume there is a lefoo_to_cpu for every type, so lets make a simple
 * one for u8:
 */
#define le8_to_cpu(v) (v)
#define cpu_to_le8(v) (v)
#define __le8 u8

#define get_unaligned_le8(p) (*((u8 *)(p)))
#define get_unaligned_8(p) (*((u8 *)(p)))
#define put_unaligned_le8(val,p) ((*((u8 *)(p))) = (val))
#define put_unaligned_8(val,p) ((*((u8 *)(p))) = (val))

/*
 * Read data from device specified by @desc and @part
 *
 * U-Boot equivalent of pread().
 *
 * Return the bytes of data read.
 * Return <0 for error.
 */
static inline int __btrfs_devread(struct blk_desc *desc,
				  struct disk_partition *part,
				  void *buf, size_t size, u64 offset)
{
	lbaint_t sector;
	int byte_offset;
	int ret;

	sector = offset >> desc->log2blksz;
	byte_offset = offset % desc->blksz;

	/* fs_devread() return 0 for error, >0 for success */
	ret = fs_devread(desc, part, sector, byte_offset, size, buf);
	if (!ret)
		return -EIO;
	return size;
}

static inline void uuid_unparse(const u8 *uuid, char *out)
{
	return uuid_bin_to_str((unsigned char *)uuid, out, 0);
}

static inline int is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

#endif
