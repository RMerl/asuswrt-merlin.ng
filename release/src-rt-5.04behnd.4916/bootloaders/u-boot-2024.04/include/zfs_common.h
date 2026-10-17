/*
 * ZFS filesystem port for Uboot by
 * Jorgen Lundman <lundman at lundman.net>
 *
 * zfsfs support
 * made from existing GRUB Sources by Sun, GNU and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ZFS_COMMON__
#define __ZFS_COMMON__

#include <part.h>

#define SECTOR_SIZE			0x200
#define SECTOR_BITS			9


typedef enum zfs_endian {
	UNKNOWN_ENDIAN = -2,
	LITTLE_ENDIAN = -1,
	BIG_ENDIAN = 0
} zfs_endian_t;


/* Endian macros. */
#define zfs_to_cpu16(x, a) (((a) == BIG_ENDIAN) ? be16_to_cpu(x) \
								: le16_to_cpu(x))
#define cpu_to_zfs16(x, a) (((a) == BIG_ENDIAN) ? cpu_to_be16(x) \
								: cpu_to_le16(x))

#define zfs_to_cpu32(x, a) (((a) == BIG_ENDIAN) ? be32_to_cpu(x) \
								: le32_to_cpu(x))
#define cpu_to_zfs32(x, a) (((a) == BIG_ENDIAN) ? cpu_to_be32(x) \
								: cpu_to_le32(x))

#define zfs_to_cpu64(x, a) (((a) == BIG_ENDIAN) ? be64_to_cpu(x) \
								: le64_to_cpu(x))
#define cpu_to_zfs64(x, a) (((a) == BIG_ENDIAN) ? cpu_to_be64(x) \
								: cpu_to_le64(x))


enum zfs_errors {
	ZFS_ERR_NONE = 0,
	ZFS_ERR_NOT_IMPLEMENTED_YET = -1,
	ZFS_ERR_BAD_FS = -2,
	ZFS_ERR_OUT_OF_MEMORY = -3,
	ZFS_ERR_FILE_NOT_FOUND = -4,
	ZFS_ERR_BAD_FILE_TYPE = -5,
	ZFS_ERR_OUT_OF_RANGE = -6,
};

struct zfs_filesystem {

	/* Block Device Descriptor */
	struct blk_desc *dev_desc;
};

struct device_s {
	uint64_t part_length;
};
typedef struct device_s *device_t;

struct zfs_file {
	device_t device;
	uint64_t size;
	void *data;
	uint64_t offset;
};

typedef struct zfs_file *zfs_file_t;

struct zfs_dirhook_info {
	int dir;
	int mtimeset;
	time_t mtime;
	time_t mtime2;
};




struct zfs_filesystem *zfsget_fs(void);
int zfs_open(zfs_file_t, const char *filename);
uint64_t zfs_read(zfs_file_t, char *buf, uint64_t len);
struct zfs_data *zfs_mount(device_t);
int zfs_close(zfs_file_t);
int zfs_ls(device_t dev, const char *path,
		   int (*hook) (const char *, const struct zfs_dirhook_info *));
int zfs_devread(int sector, int byte_offset, int byte_len, char *buf);
void zfs_set_blk_dev(struct blk_desc *rbdd, struct disk_partition *info);
void zfs_unmount(struct zfs_data *data);
int lzjb_decompress(void *, void *, uint32_t, uint32_t);
#endif
