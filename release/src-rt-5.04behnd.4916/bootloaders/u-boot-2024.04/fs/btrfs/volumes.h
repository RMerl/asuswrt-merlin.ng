// SPDX-License-Identifier: GPL-2.0+

#ifndef __BTRFS_VOLUMES_H__
#define __BTRFS_VOLUMES_H__

#include <fs_internal.h>
#include "ctree.h"

#define BTRFS_STRIPE_LEN	SZ_64K

struct btrfs_device {
	struct list_head dev_list;
	struct btrfs_root *dev_root;
	struct btrfs_fs_devices *fs_devices;

	struct blk_desc *desc;
	struct disk_partition *part;

	u64 total_devs;
	u64 super_bytes_used;

	u64 generation;

	/* the internal btrfs device id */
	u64 devid;

	/* size of the device */
	u64 total_bytes;

	/* bytes used */
	u64 bytes_used;

	/* optimal io alignment for this device */
	u32 io_align;

	/* optimal io width for this device */
	u32 io_width;

	/* minimal io size for this device */
	u32 sector_size;

	/* type and info about this device */
	u64 type;

	/* physical drive uuid (or lvm uuid) */
	u8 uuid[BTRFS_UUID_SIZE];
};

struct btrfs_fs_devices {
	u8 fsid[BTRFS_FSID_SIZE]; /* FS specific uuid */
	u8 metadata_uuid[BTRFS_FSID_SIZE]; /* FS specific uuid */

	u64 latest_devid;
	u64 lowest_devid;
	u64 latest_trans;

	u64 total_rw_bytes;

	struct list_head devices;
	struct list_head list;

	int seeding;
	struct btrfs_fs_devices *seed;
};

struct btrfs_bio_stripe {
	struct btrfs_device *dev;
	u64 physical;
};

struct btrfs_multi_bio {
	int error;
	int num_stripes;
	struct btrfs_bio_stripe stripes[];
};

struct map_lookup {
	struct cache_extent ce;
	u64 type;
	int io_align;
	int io_width;
	int stripe_len;
	int sector_size;
	int num_stripes;
	int sub_stripes;
	struct btrfs_bio_stripe stripes[];
};

struct btrfs_raid_attr {
	int sub_stripes;	/* sub_stripes info for map */
	int dev_stripes;	/* stripes per dev */
	int devs_max;		/* max devs to use */
	int devs_min;		/* min devs needed */
	int tolerated_failures; /* max tolerated fail devs */
	int devs_increment;	/* ndevs has to be a multiple of this */
	int ncopies;		/* how many copies to data has */
	int nparity;		/* number of stripes worth of bytes to store
				 * parity information */
	const char raid_name[8]; /* name of the raid */
	u64 bg_flag;		/* block group flag of the raid */
};

extern const struct btrfs_raid_attr btrfs_raid_array[BTRFS_NR_RAID_TYPES];

static inline enum btrfs_raid_types btrfs_bg_flags_to_raid_index(u64 flags)
{
	if (flags & BTRFS_BLOCK_GROUP_RAID10)
		return BTRFS_RAID_RAID10;
	else if (flags & BTRFS_BLOCK_GROUP_RAID1)
		return BTRFS_RAID_RAID1;
	else if (flags & BTRFS_BLOCK_GROUP_RAID1C3)
		return BTRFS_RAID_RAID1C3;
	else if (flags & BTRFS_BLOCK_GROUP_RAID1C4)
		return BTRFS_RAID_RAID1C4;
	else if (flags & BTRFS_BLOCK_GROUP_DUP)
		return BTRFS_RAID_DUP;
	else if (flags & BTRFS_BLOCK_GROUP_RAID0)
		return BTRFS_RAID_RAID0;
	else if (flags & BTRFS_BLOCK_GROUP_RAID5)
		return BTRFS_RAID_RAID5;
	else if (flags & BTRFS_BLOCK_GROUP_RAID6)
		return BTRFS_RAID_RAID6;

	return BTRFS_RAID_SINGLE; /* BTRFS_BLOCK_GROUP_SINGLE */
}

#define btrfs_multi_bio_size(n) (sizeof(struct btrfs_multi_bio) + \
			    (sizeof(struct btrfs_bio_stripe) * (n)))
#define btrfs_map_lookup_size(n) (sizeof(struct map_lookup) + \
				 (sizeof(struct btrfs_bio_stripe) * (n)))

#define BTRFS_RAID5_P_STRIPE ((u64)-2)
#define BTRFS_RAID6_Q_STRIPE ((u64)-1)

static inline u64 calc_stripe_length(u64 type, u64 length, int num_stripes)
{
	u64 stripe_size;

	if (type & BTRFS_BLOCK_GROUP_RAID0) {
		stripe_size = length;
		stripe_size /= num_stripes;
	} else if (type & BTRFS_BLOCK_GROUP_RAID10) {
		stripe_size = length * 2;
		stripe_size /= num_stripes;
	} else if (type & BTRFS_BLOCK_GROUP_RAID5) {
		stripe_size = length;
		stripe_size /= (num_stripes - 1);
	} else if (type & BTRFS_BLOCK_GROUP_RAID6) {
		stripe_size = length;
		stripe_size /= (num_stripes - 2);
	} else {
		stripe_size = length;
	}
	return stripe_size;
}

#ifndef READ
#define READ 0
#define WRITE 1
#define READA 2
#endif

int __btrfs_map_block(struct btrfs_fs_info *fs_info, int rw,
		      u64 logical, u64 *length, u64 *type,
		      struct btrfs_multi_bio **multi_ret, int mirror_num,
		      u64 **raid_map);
int btrfs_map_block(struct btrfs_fs_info *fs_info, int rw,
		    u64 logical, u64 *length,
		    struct btrfs_multi_bio **multi_ret, int mirror_num,
		    u64 **raid_map_ret);
int btrfs_next_bg(struct btrfs_fs_info *map_tree, u64 *logical,
		     u64 *size, u64 type);
static inline int btrfs_next_bg_metadata(struct btrfs_fs_info *fs_info,
					 u64 *logical, u64 *size)
{
	return btrfs_next_bg(fs_info, logical, size,
			BTRFS_BLOCK_GROUP_METADATA);
}
static inline int btrfs_next_bg_system(struct btrfs_fs_info *fs_info,
				       u64 *logical, u64 *size)
{
	return btrfs_next_bg(fs_info, logical, size,
			BTRFS_BLOCK_GROUP_SYSTEM);
}
int btrfs_read_sys_array(struct btrfs_fs_info *fs_info);
int btrfs_read_chunk_tree(struct btrfs_fs_info *fs_info);
int btrfs_open_devices(struct btrfs_fs_devices *fs_devices);
int btrfs_close_devices(struct btrfs_fs_devices *fs_devices);
void btrfs_close_all_devices(void);
int btrfs_num_copies(struct btrfs_fs_info *fs_info, u64 logical, u64 len);
int btrfs_scan_one_device(struct blk_desc *desc, struct disk_partition *part,
			  struct btrfs_fs_devices **fs_devices_ret,
			  u64 *total_devs);
struct list_head *btrfs_scanned_uuids(void);
struct btrfs_device *btrfs_find_device(struct btrfs_fs_info *fs_info, u64 devid,
				       u8 *uuid, u8 *fsid);
int btrfs_check_chunk_valid(struct btrfs_fs_info *fs_info,
			    struct extent_buffer *leaf,
			    struct btrfs_chunk *chunk,
			    int slot, u64 logical);
u64 btrfs_stripe_length(struct btrfs_fs_info *fs_info,
			struct extent_buffer *leaf,
			struct btrfs_chunk *chunk);
#endif
