/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copied from kernel/include/uapi/linux/btrfs_btree.h.
 *
 * Only modified the header.
 */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __BTRFS_TREE_H__
#define __BTRFS_TREE_H__

#include <linux/types.h>

#define BTRFS_MAGIC 0x4D5F53665248425FULL /* ascii _BHRfS_M, no null */

/*
 * The max metadata block size (node size).
 *
 * This limit is somewhat artificial. The memmove and tree block locking cost
 * go up with larger node size.
 */
#define BTRFS_MAX_METADATA_BLOCKSIZE 65536

/*
 * We can actually store much bigger names, but lets not confuse the rest
 * of linux.
 *
 * btrfs_dir_item::name_len follows this limitation.
 */
#define BTRFS_NAME_LEN 255

/*
 * Objectids start from here.
 *
 * Check btrfs_disk_key for the meaning of objectids.
 */

/*
 * Root tree holds pointers to all of the tree roots.
 * Without special mention, the root tree contains the root bytenr of all other
 * trees, except the chunk tree and the log tree.
 *
 * The super block contains the root bytenr of this tree.
 */
#define BTRFS_ROOT_TREE_OBJECTID 1ULL

/*
 * Extent tree stores information about which extents are in use, and backrefs
 * for each extent.
 */
#define BTRFS_EXTENT_TREE_OBJECTID 2ULL

/*
 * Chunk tree stores btrfs logical address -> physical address mapping.
 *
 * The super block contains part of chunk tree for bootstrap, and contains
 * the root bytenr of this tree.
 */
#define BTRFS_CHUNK_TREE_OBJECTID 3ULL

/*
 * Device tree stores info about which areas of a given device are in use,
 * and physical address -> btrfs logical address mapping.
 */
#define BTRFS_DEV_TREE_OBJECTID 4ULL

/* The fs tree is the first subvolume tree, storing files and directories. */
#define BTRFS_FS_TREE_OBJECTID 5ULL

/* Shows the directory objectid inside the root tree. */
#define BTRFS_ROOT_TREE_DIR_OBJECTID 6ULL

/* Csum tree holds checksums of all the data extents. */
#define BTRFS_CSUM_TREE_OBJECTID 7ULL

/* Quota tree holds quota configuration and tracking. */
#define BTRFS_QUOTA_TREE_OBJECTID 8ULL

/* UUID tree stores items that use the BTRFS_UUID_KEY* types. */
#define BTRFS_UUID_TREE_OBJECTID 9ULL

/* Free space cache tree (v2 space cache) tracks free space in block groups. */
#define BTRFS_FREE_SPACE_TREE_OBJECTID 10ULL

/* Indicates device stats in the device tree. */
#define BTRFS_DEV_STATS_OBJECTID 0ULL

/* For storing balance parameters in the root tree. */
#define BTRFS_BALANCE_OBJECTID -4ULL

/* Orhpan objectid for tracking unlinked/truncated files. */
#define BTRFS_ORPHAN_OBJECTID -5ULL

/* Does write ahead logging to speed up fsyncs. */
#define BTRFS_TREE_LOG_OBJECTID -6ULL
#define BTRFS_TREE_LOG_FIXUP_OBJECTID -7ULL

/* For space balancing. */
#define BTRFS_TREE_RELOC_OBJECTID -8ULL
#define BTRFS_DATA_RELOC_TREE_OBJECTID -9ULL

/* Extent checksums, shared between the csum tree and log trees. */
#define BTRFS_EXTENT_CSUM_OBJECTID -10ULL

/* For storing free space cache (v1 space cache). */
#define BTRFS_FREE_SPACE_OBJECTID -11ULL

/* The inode number assigned to the special inode for storing free ino cache. */
#define BTRFS_FREE_INO_OBJECTID -12ULL

/* Dummy objectid represents multiple objectids. */
#define BTRFS_MULTIPLE_OBJECTIDS -255ULL

/* All files have objectids in this range. */
#define BTRFS_FIRST_FREE_OBJECTID 256ULL
#define BTRFS_LAST_FREE_OBJECTID -256ULL
#define BTRFS_FIRST_CHUNK_TREE_OBJECTID 256ULL


/*
 * The device items go into the chunk tree.
 *
 * The key is in the form
 * (BTRFS_DEV_ITEMS_OBJECTID, BTRFS_DEV_ITEM_KEY,  <device_id>)
 */
#define BTRFS_DEV_ITEMS_OBJECTID 1ULL

#define BTRFS_BTREE_INODE_OBJECTID 1

#define BTRFS_EMPTY_SUBVOL_DIR_OBJECTID 2

#define BTRFS_DEV_REPLACE_DEVID 0ULL

/*
 * Types start from here.
 *
 * Check btrfs_disk_key for details about types.
 */

/*
 * Inode items have the data typically returned from stat and store other
 * info about object characteristics.
 *
 * There is one for every file and dir in the FS.
 */
#define BTRFS_INODE_ITEM_KEY		1
/* reserve 2-11 close to the inode for later flexibility */
#define BTRFS_INODE_REF_KEY		12
#define BTRFS_INODE_EXTREF_KEY		13
#define BTRFS_XATTR_ITEM_KEY		24
#define BTRFS_ORPHAN_ITEM_KEY		48

/*
 * Dir items are the name -> inode pointers in a directory.
 *
 * There is one for every name in a directory.
 */
#define BTRFS_DIR_LOG_ITEM_KEY  60
#define BTRFS_DIR_LOG_INDEX_KEY 72
#define BTRFS_DIR_ITEM_KEY	84
#define BTRFS_DIR_INDEX_KEY	96

/* Stores info (position, size ...) about a data extent of a file */
#define BTRFS_EXTENT_DATA_KEY	108

/*
 * Extent csums are stored in a separate tree and hold csums for
 * an entire extent on disk.
 */
#define BTRFS_EXTENT_CSUM_KEY	128

/*
 * Root items point to tree roots.
 *
 * They are typically in the root tree used by the super block to find all the
 * other trees.
 */
#define BTRFS_ROOT_ITEM_KEY	132

/*
 * Root backrefs tie subvols and snapshots to the directory entries that
 * reference them.
 */
#define BTRFS_ROOT_BACKREF_KEY	144

/*
 * Root refs make a fast index for listing all of the snapshots and
 * subvolumes referenced by a given root.  They point directly to the
 * directory item in the root that references the subvol.
 */
#define BTRFS_ROOT_REF_KEY	156

/*
 * Extent items are in the extent tree.
 *
 * These record which blocks are used, and how many references there are.
 */
#define BTRFS_EXTENT_ITEM_KEY	168

/*
 * The same as the BTRFS_EXTENT_ITEM_KEY, except it's metadata we already know
 * the length, so we save the level in key->offset instead of the length.
 */
#define BTRFS_METADATA_ITEM_KEY	169

#define BTRFS_TREE_BLOCK_REF_KEY	176

#define BTRFS_EXTENT_DATA_REF_KEY	178

#define BTRFS_EXTENT_REF_V0_KEY		180

#define BTRFS_SHARED_BLOCK_REF_KEY	182

#define BTRFS_SHARED_DATA_REF_KEY	184

/*
 * Block groups give us hints into the extent allocation trees.
 *
 * Stores how many free space there is in a block group.
 */
#define BTRFS_BLOCK_GROUP_ITEM_KEY 192

/*
 * Every block group is represented in the free space tree by a free space info
 * item, which stores some accounting information. It is keyed on
 * (block_group_start, FREE_SPACE_INFO, block_group_length).
 */
#define BTRFS_FREE_SPACE_INFO_KEY 198

/*
 * A free space extent tracks an extent of space that is free in a block group.
 * It is keyed on (start, FREE_SPACE_EXTENT, length).
 */
#define BTRFS_FREE_SPACE_EXTENT_KEY 199

/*
 * When a block group becomes very fragmented, we convert it to use bitmaps
 * instead of extents.
 *
 * A free space bitmap is keyed on (start, FREE_SPACE_BITMAP, length).
 * The corresponding item is a bitmap with (length / sectorsize) bits.
 */
#define BTRFS_FREE_SPACE_BITMAP_KEY 200

#define BTRFS_DEV_EXTENT_KEY	204
#define BTRFS_DEV_ITEM_KEY	216
#define BTRFS_CHUNK_ITEM_KEY	228

/*
 * Records the overall state of the qgroups.
 *
 * There's only one instance of this key present,
 * (0, BTRFS_QGROUP_STATUS_KEY, 0)
 */
#define BTRFS_QGROUP_STATUS_KEY         240
/*
 * Records the currently used space of the qgroup.
 *
 * One key per qgroup, (0, BTRFS_QGROUP_INFO_KEY, qgroupid).
 */
#define BTRFS_QGROUP_INFO_KEY           242

/*
 * Contains the user configured limits for the qgroup.
 *
 * One key per qgroup, (0, BTRFS_QGROUP_LIMIT_KEY, qgroupid).
 */
#define BTRFS_QGROUP_LIMIT_KEY          244

/*
 * Records the child-parent relationship of qgroups. For
 * each relation, 2 keys are present:
 * (childid, BTRFS_QGROUP_RELATION_KEY, parentid)
 * (parentid, BTRFS_QGROUP_RELATION_KEY, childid)
 */
#define BTRFS_QGROUP_RELATION_KEY       246

/* Obsolete name, see BTRFS_TEMPORARY_ITEM_KEY. */
#define BTRFS_BALANCE_ITEM_KEY	248

/*
 * The key type for tree items that are stored persistently, but do not need to
 * exist for extended period of time. The items can exist in any tree.
 *
 * [subtype, BTRFS_TEMPORARY_ITEM_KEY, data]
 *
 * Existing items:
 *
 * - balance status item
 *   (BTRFS_BALANCE_OBJECTID, BTRFS_TEMPORARY_ITEM_KEY, 0)
 */
#define BTRFS_TEMPORARY_ITEM_KEY	248

/* Obsolete name, see BTRFS_PERSISTENT_ITEM_KEY */
#define BTRFS_DEV_STATS_KEY		249

/*
 * The key type for tree items that are stored persistently and usually exist
 * for a long period, eg. filesystem lifetime. The item kinds can be status
 * information, stats or preference values. The item can exist in any tree.
 *
 * [subtype, BTRFS_PERSISTENT_ITEM_KEY, data]
 *
 * Existing items:
 *
 * - device statistics, store IO stats in the device tree, one key for all
 *   stats
 *   (BTRFS_DEV_STATS_OBJECTID, BTRFS_DEV_STATS_KEY, 0)
 */
#define BTRFS_PERSISTENT_ITEM_KEY	249

/*
 * Persistently stores the device replace state in the device tree.
 *
 * The key is built like this: (0, BTRFS_DEV_REPLACE_KEY, 0).
 */
#define BTRFS_DEV_REPLACE_KEY	250

/*
 * Stores items that allow to quickly map UUIDs to something else.
 *
 * These items are part of the filesystem UUID tree.
 * The key is built like this:
 * (UUID_upper_64_bits, BTRFS_UUID_KEY*, UUID_lower_64_bits).
 */
#define BTRFS_UUID_KEY_SUBVOL	251	/* for UUIDs assigned to subvols */
#define BTRFS_UUID_KEY_RECEIVED_SUBVOL	252	/* for UUIDs assigned to
						 * received subvols */

/*
 * String items are for debugging.
 *
 * They just store a short string of data in the FS.
 */
#define BTRFS_STRING_ITEM_KEY	253


/* 32 bytes in various csum fields */
#define BTRFS_CSUM_SIZE 32

/* Csum types */
enum btrfs_csum_type {
	BTRFS_CSUM_TYPE_CRC32	= 0,
	BTRFS_CSUM_TYPE_XXHASH	= 1,
	BTRFS_CSUM_TYPE_SHA256	= 2,
	BTRFS_CSUM_TYPE_BLAKE2	= 3,
};

/*
 * Flags definitions for directory entry item type.
 *
 * Used by:
 * struct btrfs_dir_item.type
 *
 * Values 0..7 must match common file type values in fs_types.h.
 */
#define BTRFS_FT_UNKNOWN	0
#define BTRFS_FT_REG_FILE	1
#define BTRFS_FT_DIR		2
#define BTRFS_FT_CHRDEV		3
#define BTRFS_FT_BLKDEV		4
#define BTRFS_FT_FIFO		5
#define BTRFS_FT_SOCK		6
#define BTRFS_FT_SYMLINK	7
#define BTRFS_FT_XATTR		8
#define BTRFS_FT_MAX		9

#define BTRFS_FSID_SIZE 16
#define BTRFS_UUID_SIZE 16

/*
 * The key defines the order in the tree, and so it also defines (optimal)
 * block layout.
 *
 * Objectid and offset are interpreted based on type.
 * While normally for objectid, it either represents a root number, or an
 * inode number.
 *
 * Type tells us things about the object, and is a kind of stream selector.
 * Check the following URL for full references about btrfs_disk_key/btrfs_key:
 * https://btrfs.wiki.kernel.org/index.php/Btree_Items
 *
 * btrfs_disk_key is in disk byte order.  struct btrfs_key is always
 * in cpu native order.  Otherwise they are identical and their sizes
 * should be the same (ie both packed)
 */
struct btrfs_disk_key {
	__le64 objectid;
	__u8 type;
	__le64 offset;
} __attribute__ ((__packed__));

struct btrfs_key {
	__u64 objectid;
	__u8 type;
	__u64 offset;
} __attribute__ ((__packed__));

struct btrfs_dev_item {
	/* The internal btrfs device id */
	__le64 devid;

	/* Size of the device */
	__le64 total_bytes;

	/* Bytes used */
	__le64 bytes_used;

	/* Optimal io alignment for this device */
	__le32 io_align;

	/* Optimal io width for this device */
	__le32 io_width;

	/* Minimal io size for this device */
	__le32 sector_size;

	/* Type and info about this device */
	__le64 type;

	/* Expected generation for this device */
	__le64 generation;

	/*
	 * Starting byte of this partition on the device,
	 * to allow for stripe alignment in the future.
	 */
	__le64 start_offset;

	/* Grouping information for allocation decisions */
	__le32 dev_group;

	/* Optimal seek speed 0-100 where 100 is fastest */
	__u8 seek_speed;

	/* Optimal bandwidth 0-100 where 100 is fastest */
	__u8 bandwidth;

	/* Btrfs generated uuid for this device */
	__u8 uuid[BTRFS_UUID_SIZE];

	/* UUID of FS who owns this device */
	__u8 fsid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

struct btrfs_stripe {
	__le64 devid;
	__le64 offset;
	__u8 dev_uuid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

struct btrfs_chunk {
	/* Size of this chunk in bytes */
	__le64 length;

	/* Objectid of the root referencing this chunk */
	__le64 owner;

	__le64 stripe_len;
	__le64 type;

	/* Optimal io alignment for this chunk */
	__le32 io_align;

	/* Optimal io width for this chunk */
	__le32 io_width;

	/* Minimal io size for this chunk */
	__le32 sector_size;

	/*
	 * 2^16 stripes is quite a lot, a second limit is the size of a single
	 * item in the btree.
	 */
	__le16 num_stripes;

	/* Sub stripes only matter for raid10 */
	__le16 sub_stripes;
	struct btrfs_stripe stripe;
	/* additional stripes go here */
} __attribute__ ((__packed__));

#define BTRFS_FREE_SPACE_EXTENT	1
#define BTRFS_FREE_SPACE_BITMAP	2

struct btrfs_free_space_entry {
	__le64 offset;
	__le64 bytes;
	__u8 type;
} __attribute__ ((__packed__));

struct btrfs_free_space_header {
	struct btrfs_disk_key location;
	__le64 generation;
	__le64 num_entries;
	__le64 num_bitmaps;
} __attribute__ ((__packed__));

#define BTRFS_HEADER_FLAG_WRITTEN	(1ULL << 0)
#define BTRFS_HEADER_FLAG_RELOC		(1ULL << 1)

/* Super block flags */
/* Errors detected */
#define BTRFS_SUPER_FLAG_ERROR		(1ULL << 2)

#define BTRFS_SUPER_FLAG_SEEDING	(1ULL << 32)
#define BTRFS_SUPER_FLAG_METADUMP	(1ULL << 33)
#define BTRFS_SUPER_FLAG_METADUMP_V2	(1ULL << 34)
#define BTRFS_SUPER_FLAG_CHANGING_FSID	(1ULL << 35)
#define BTRFS_SUPER_FLAG_CHANGING_FSID_V2 (1ULL << 36)


/*
 * Items in the extent tree are used to record the objectid of the
 * owner of the block and the number of references.
 */
struct btrfs_extent_item {
	__le64 refs;
	__le64 generation;
	__le64 flags;
} __attribute__ ((__packed__));

struct btrfs_extent_item_v0 {
	__le32 refs;
} __attribute__ ((__packed__));


#define BTRFS_EXTENT_FLAG_DATA		(1ULL << 0)
#define BTRFS_EXTENT_FLAG_TREE_BLOCK	(1ULL << 1)

/* Use full backrefs for extent pointers in the block */
#define BTRFS_BLOCK_FLAG_FULL_BACKREF	(1ULL << 8)

/*
 * This flag is only used internally by scrub and may be changed at any time
 * it is only declared here to avoid collisions.
 */
#define BTRFS_EXTENT_FLAG_SUPER		(1ULL << 48)

struct btrfs_tree_block_info {
	struct btrfs_disk_key key;
	__u8 level;
} __attribute__ ((__packed__));

struct btrfs_extent_data_ref {
	__le64 root;
	__le64 objectid;
	__le64 offset;
	__le32 count;
} __attribute__ ((__packed__));

struct btrfs_shared_data_ref {
	__le32 count;
} __attribute__ ((__packed__));

struct btrfs_extent_inline_ref {
	__u8 type;
	__le64 offset;
} __attribute__ ((__packed__));

/* Old style backrefs item */
struct btrfs_extent_ref_v0 {
	__le64 root;
	__le64 generation;
	__le64 objectid;
	__le32 count;
} __attribute__ ((__packed__));


/* Dev extents record used space on individual devices.
 *
 * The owner field points back to the chunk allocation mapping tree that
 * allocated the extent.
 * The chunk tree uuid field is a way to double check the owner.
 */
struct btrfs_dev_extent {
	__le64 chunk_tree;
	__le64 chunk_objectid;
	__le64 chunk_offset;
	__le64 length;
	__u8 chunk_tree_uuid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

struct btrfs_inode_ref {
	__le64 index;
	__le16 name_len;
	/* Name goes here */
} __attribute__ ((__packed__));

struct btrfs_inode_extref {
	__le64 parent_objectid;
	__le64 index;
	__le16 name_len;
	__u8   name[0];
	/* Name goes here */
} __attribute__ ((__packed__));

struct btrfs_timespec {
	__le64 sec;
	__le32 nsec;
} __attribute__ ((__packed__));

/* Inode flags */
#define BTRFS_INODE_NODATASUM		(1 << 0)
#define BTRFS_INODE_NODATACOW		(1 << 1)
#define BTRFS_INODE_READONLY		(1 << 2)
#define BTRFS_INODE_NOCOMPRESS		(1 << 3)
#define BTRFS_INODE_PREALLOC		(1 << 4)
#define BTRFS_INODE_SYNC		(1 << 5)
#define BTRFS_INODE_IMMUTABLE		(1 << 6)
#define BTRFS_INODE_APPEND		(1 << 7)
#define BTRFS_INODE_NODUMP		(1 << 8)
#define BTRFS_INODE_NOATIME		(1 << 9)
#define BTRFS_INODE_DIRSYNC		(1 << 10)
#define BTRFS_INODE_COMPRESS		(1 << 11)

#define BTRFS_INODE_ROOT_ITEM_INIT	(1 << 31)

#define BTRFS_INODE_FLAG_MASK						\
	(BTRFS_INODE_NODATASUM |					\
	 BTRFS_INODE_NODATACOW |					\
	 BTRFS_INODE_READONLY |						\
	 BTRFS_INODE_NOCOMPRESS |					\
	 BTRFS_INODE_PREALLOC |						\
	 BTRFS_INODE_SYNC |						\
	 BTRFS_INODE_IMMUTABLE |					\
	 BTRFS_INODE_APPEND |						\
	 BTRFS_INODE_NODUMP |						\
	 BTRFS_INODE_NOATIME |						\
	 BTRFS_INODE_DIRSYNC |						\
	 BTRFS_INODE_COMPRESS |						\
	 BTRFS_INODE_ROOT_ITEM_INIT)

struct btrfs_inode_item {
	/* Nfs style generation number */
	__le64 generation;
	/* Transid that last touched this inode */
	__le64 transid;
	__le64 size;
	__le64 nbytes;
	__le64 block_group;
	__le32 nlink;
	__le32 uid;
	__le32 gid;
	__le32 mode;
	__le64 rdev;
	__le64 flags;

	/* Modification sequence number for NFS */
	__le64 sequence;

	/*
	 * A little future expansion, for more than this we can just grow the
	 * inode item and version it
	 */
	__le64 reserved[4];
	struct btrfs_timespec atime;
	struct btrfs_timespec ctime;
	struct btrfs_timespec mtime;
	struct btrfs_timespec otime;
} __attribute__ ((__packed__));

struct btrfs_dir_log_item {
	__le64 end;
} __attribute__ ((__packed__));

struct btrfs_dir_item {
	struct btrfs_disk_key location;
	__le64 transid;
	__le16 data_len;
	__le16 name_len;
	__u8 type;
} __attribute__ ((__packed__));

#define BTRFS_ROOT_SUBVOL_RDONLY	(1ULL << 0)

/*
 * Internal in-memory flag that a subvolume has been marked for deletion but
 * still visible as a directory
 */
#define BTRFS_ROOT_SUBVOL_DEAD		(1ULL << 48)

struct btrfs_root_item {
	struct btrfs_inode_item inode;
	__le64 generation;
	__le64 root_dirid;
	__le64 bytenr;
	__le64 byte_limit;
	__le64 bytes_used;
	__le64 last_snapshot;
	__le64 flags;
	__le32 refs;
	struct btrfs_disk_key drop_progress;
	__u8 drop_level;
	__u8 level;

	/*
	 * The following fields appear after subvol_uuids+subvol_times
	 * were introduced.
	 */

	/*
	 * This generation number is used to test if the new fields are valid
	 * and up to date while reading the root item. Every time the root item
	 * is written out, the "generation" field is copied into this field. If
	 * anyone ever mounted the fs with an older kernel, we will have
	 * mismatching generation values here and thus must invalidate the
	 * new fields. See btrfs_update_root and btrfs_find_last_root for
	 * details.
	 * The offset of generation_v2 is also used as the start for the memset
	 * when invalidating the fields.
	 */
	__le64 generation_v2;
	__u8 uuid[BTRFS_UUID_SIZE];
	__u8 parent_uuid[BTRFS_UUID_SIZE];
	__u8 received_uuid[BTRFS_UUID_SIZE];
	__le64 ctransid; /* Updated when an inode changes */
	__le64 otransid; /* Trans when created */
	__le64 stransid; /* Trans when sent. Non-zero for received subvol. */
	__le64 rtransid; /* Trans when received. Non-zero for received subvol.*/
	struct btrfs_timespec ctime;
	struct btrfs_timespec otime;
	struct btrfs_timespec stime;
	struct btrfs_timespec rtime;
	__le64 reserved[8]; /* For future */
} __attribute__ ((__packed__));

/* This is used for both forward and backward root refs */
struct btrfs_root_ref {
	__le64 dirid;
	__le64 sequence;
	__le16 name_len;
} __attribute__ ((__packed__));

struct btrfs_disk_balance_args {
	/*
	 * Profiles to operate on.
	 *
	 * SINGLE is denoted by BTRFS_AVAIL_ALLOC_BIT_SINGLE.
	 */
	__le64 profiles;

	/*
	 * Usage filter
	 * BTRFS_BALANCE_ARGS_USAGE with a single value means '0..N'
	 * BTRFS_BALANCE_ARGS_USAGE_RANGE - range syntax, min..max
	 */
	union {
		__le64 usage;
		struct {
			__le32 usage_min;
			__le32 usage_max;
		};
	};

	/* Devid filter */
	__le64 devid;

	/* Devid subset filter [pstart..pend) */
	__le64 pstart;
	__le64 pend;

	/* Btrfs virtual address space subset filter [vstart..vend) */
	__le64 vstart;
	__le64 vend;

	/*
	 * Profile to convert to.
	 *
	 * SINGLE is denoted by BTRFS_AVAIL_ALLOC_BIT_SINGLE.
	 */
	__le64 target;

	/* BTRFS_BALANCE_ARGS_* */
	__le64 flags;

	/*
	 * BTRFS_BALANCE_ARGS_LIMIT with value 'limit'.
	 * BTRFS_BALANCE_ARGS_LIMIT_RANGE - the extend version can use minimum
	 * and maximum.
	 */
	union {
		__le64 limit;
		struct {
			__le32 limit_min;
			__le32 limit_max;
		};
	};

	/*
	 * Process chunks that cross stripes_min..stripes_max devices,
	 * BTRFS_BALANCE_ARGS_STRIPES_RANGE.
	 */
	__le32 stripes_min;
	__le32 stripes_max;

	__le64 unused[6];
} __attribute__ ((__packed__));

/*
 * Stores balance parameters to disk so that balance can be properly
 * resumed after crash or unmount.
 */
struct btrfs_balance_item {
	/* BTRFS_BALANCE_* */
	__le64 flags;

	struct btrfs_disk_balance_args data;
	struct btrfs_disk_balance_args meta;
	struct btrfs_disk_balance_args sys;

	__le64 unused[4];
} __attribute__ ((__packed__));

enum {
	BTRFS_FILE_EXTENT_INLINE   = 0,
	BTRFS_FILE_EXTENT_REG      = 1,
	BTRFS_FILE_EXTENT_PREALLOC = 2,
	BTRFS_NR_FILE_EXTENT_TYPES = 3,
};

enum btrfs_compression_type {
	BTRFS_COMPRESS_NONE  = 0,
	BTRFS_COMPRESS_ZLIB  = 1,
	BTRFS_COMPRESS_LZO   = 2,
	BTRFS_COMPRESS_ZSTD  = 3,
	BTRFS_NR_COMPRESS_TYPES = 4,
};

struct btrfs_file_extent_item {
	/* Transaction id that created this extent */
	__le64 generation;
	/*
	 * Max number of bytes to hold this extent in ram.
	 *
	 * When we split a compressed extent we can't know how big each of the
	 * resulting pieces will be.  So, this is an upper limit on the size of
	 * the extent in ram instead of an exact limit.
	 */
	__le64 ram_bytes;

	/*
	 * 32 bits for the various ways we might encode the data,
	 * including compression and encryption.  If any of these
	 * are set to something a given disk format doesn't understand
	 * it is treated like an incompat flag for reading and writing,
	 * but not for stat.
	 */
	__u8 compression;
	__u8 encryption;
	__le16 other_encoding; /* Spare for later use */

	/* Are we inline data or a real extent? */
	__u8 type;

	/*
	 * Disk space consumed by the extent, checksum blocks are not included
	 * in these numbers
	 *
	 * At this offset in the structure, the inline extent data start.
	 */
	__le64 disk_bytenr;
	__le64 disk_num_bytes;

	/*
	 * The logical offset inside the file extent.
	 *
	 * This allows a file extent to point into the middle of an existing
	 * extent on disk, sharing it between two snapshots (useful if some
	 * bytes in the middle of the extent have changed).
	 */
	__le64 offset;

	/*
	 * The logical number of bytes this file extent is referencing (no
	 * csums included).
	 *
	 * This always reflects the size uncompressed and without encoding.
	 */
	__le64 num_bytes;

} __attribute__ ((__packed__));

struct btrfs_csum_item {
	__u8 csum;
} __attribute__ ((__packed__));

enum btrfs_dev_stat_values {
	/* Disk I/O failure stats */
	BTRFS_DEV_STAT_WRITE_ERRS, /* EIO or EREMOTEIO from lower layers */
	BTRFS_DEV_STAT_READ_ERRS, /* EIO or EREMOTEIO from lower layers */
	BTRFS_DEV_STAT_FLUSH_ERRS, /* EIO or EREMOTEIO from lower layers */

	/* Stats for indirect indications for I/O failures */
	BTRFS_DEV_STAT_CORRUPTION_ERRS, /* Checksum error, bytenr error or
					 * contents is illegal: this is an
					 * indication that the block was damaged
					 * during read or write, or written to
					 * wrong location or read from wrong
					 * location */
	BTRFS_DEV_STAT_GENERATION_ERRS, /* An indication that blocks have not
					 * been written */

	BTRFS_DEV_STAT_VALUES_MAX
};

struct btrfs_dev_stats_item {
	/*
	 * Grow this item struct at the end for future enhancements and keep
	 * the existing values unchanged.
	 */
	__le64 values[BTRFS_DEV_STAT_VALUES_MAX];
} __attribute__ ((__packed__));

#define BTRFS_DEV_REPLACE_ITEM_CONT_READING_FROM_SRCDEV_MODE_ALWAYS	0
#define BTRFS_DEV_REPLACE_ITEM_CONT_READING_FROM_SRCDEV_MODE_AVOID	1

struct btrfs_dev_replace_item {
	/*
	 * Grow this item struct at the end for future enhancements and keep
	 * the existing values unchanged.
	 */
	__le64 src_devid;
	__le64 cursor_left;
	__le64 cursor_right;
	__le64 cont_reading_from_srcdev_mode;

	__le64 replace_state;
	__le64 time_started;
	__le64 time_stopped;
	__le64 num_write_errors;
	__le64 num_uncorrectable_read_errors;
} __attribute__ ((__packed__));

/* Different types of block groups (and chunks) */
#define BTRFS_BLOCK_GROUP_DATA		(1ULL << 0)
#define BTRFS_BLOCK_GROUP_SYSTEM	(1ULL << 1)
#define BTRFS_BLOCK_GROUP_METADATA	(1ULL << 2)
#define BTRFS_BLOCK_GROUP_RAID0		(1ULL << 3)
#define BTRFS_BLOCK_GROUP_RAID1		(1ULL << 4)
#define BTRFS_BLOCK_GROUP_DUP		(1ULL << 5)
#define BTRFS_BLOCK_GROUP_RAID10	(1ULL << 6)
#define BTRFS_BLOCK_GROUP_RAID5         (1ULL << 7)
#define BTRFS_BLOCK_GROUP_RAID6         (1ULL << 8)
#define BTRFS_BLOCK_GROUP_RAID1C3       (1ULL << 9)
#define BTRFS_BLOCK_GROUP_RAID1C4       (1ULL << 10)
#define BTRFS_BLOCK_GROUP_RESERVED	(BTRFS_AVAIL_ALLOC_BIT_SINGLE | \
					 BTRFS_SPACE_INFO_GLOBAL_RSV)

enum btrfs_raid_types {
	BTRFS_RAID_RAID10,
	BTRFS_RAID_RAID1,
	BTRFS_RAID_DUP,
	BTRFS_RAID_RAID0,
	BTRFS_RAID_SINGLE,
	BTRFS_RAID_RAID5,
	BTRFS_RAID_RAID6,
	BTRFS_RAID_RAID1C3,
	BTRFS_RAID_RAID1C4,
	BTRFS_NR_RAID_TYPES
};

#define BTRFS_BLOCK_GROUP_TYPE_MASK	(BTRFS_BLOCK_GROUP_DATA |    \
					 BTRFS_BLOCK_GROUP_SYSTEM |  \
					 BTRFS_BLOCK_GROUP_METADATA)

#define BTRFS_BLOCK_GROUP_PROFILE_MASK	(BTRFS_BLOCK_GROUP_RAID0 |   \
					 BTRFS_BLOCK_GROUP_RAID1 |   \
					 BTRFS_BLOCK_GROUP_RAID1C3 | \
					 BTRFS_BLOCK_GROUP_RAID1C4 | \
					 BTRFS_BLOCK_GROUP_RAID5 |   \
					 BTRFS_BLOCK_GROUP_RAID6 |   \
					 BTRFS_BLOCK_GROUP_DUP |     \
					 BTRFS_BLOCK_GROUP_RAID10)
#define BTRFS_BLOCK_GROUP_RAID56_MASK	(BTRFS_BLOCK_GROUP_RAID5 |   \
					 BTRFS_BLOCK_GROUP_RAID6)

#define BTRFS_BLOCK_GROUP_RAID1_MASK	(BTRFS_BLOCK_GROUP_RAID1 |   \
					 BTRFS_BLOCK_GROUP_RAID1C3 | \
					 BTRFS_BLOCK_GROUP_RAID1C4)

/*
 * We need a bit for restriper to be able to tell when chunks of type
 * SINGLE are available.  This "extended" profile format is used in
 * fs_info->avail_*_alloc_bits (in-memory) and balance item fields
 * (on-disk).  The corresponding on-disk bit in chunk.type is reserved
 * to avoid remappings between two formats in future.
 */
#define BTRFS_AVAIL_ALLOC_BIT_SINGLE	(1ULL << 48)

/*
 * A fake block group type that is used to communicate global block reserve
 * size to userspace via the SPACE_INFO ioctl.
 */
#define BTRFS_SPACE_INFO_GLOBAL_RSV	(1ULL << 49)

#define BTRFS_EXTENDED_PROFILE_MASK	(BTRFS_BLOCK_GROUP_PROFILE_MASK | \
					 BTRFS_AVAIL_ALLOC_BIT_SINGLE)

static inline __u64 chunk_to_extended(__u64 flags)
{
	if ((flags & BTRFS_BLOCK_GROUP_PROFILE_MASK) == 0)
		flags |= BTRFS_AVAIL_ALLOC_BIT_SINGLE;

	return flags;
}
static inline __u64 extended_to_chunk(__u64 flags)
{
	return flags & ~BTRFS_AVAIL_ALLOC_BIT_SINGLE;
}

struct btrfs_block_group_item {
	__le64 used;
	__le64 chunk_objectid;
	__le64 flags;
} __attribute__ ((__packed__));

struct btrfs_free_space_info {
	__le32 extent_count;
	__le32 flags;
} __attribute__ ((__packed__));

#define BTRFS_FREE_SPACE_USING_BITMAPS (1ULL << 0)

#define BTRFS_QGROUP_LEVEL_SHIFT		48
static inline __u64 btrfs_qgroup_level(__u64 qgroupid)
{
	return qgroupid >> BTRFS_QGROUP_LEVEL_SHIFT;
}

/* Is subvolume quota turned on? */
#define BTRFS_QGROUP_STATUS_FLAG_ON		(1ULL << 0)

/* Is qgroup rescan running? */
#define BTRFS_QGROUP_STATUS_FLAG_RESCAN		(1ULL << 1)

/*
 * Some qgroup entries are known to be out of date, either because the
 * configuration has changed in a way that makes a rescan necessary, or
 * because the fs has been mounted with a non-qgroup-aware version.
 */
#define BTRFS_QGROUP_STATUS_FLAG_INCONSISTENT	(1ULL << 2)

#define BTRFS_QGROUP_STATUS_VERSION        1

struct btrfs_qgroup_status_item {
	__le64 version;
	/*
	 * The generation is updated during every commit. As older
	 * versions of btrfs are not aware of qgroups, it will be
	 * possible to detect inconsistencies by checking the
	 * generation on mount time.
	 */
	__le64 generation;

	/* Flag definitions see above */
	__le64 flags;

	/*
	 * Only used during scanning to record the progress of the scan.
	 * It contains a logical address.
	 */
	__le64 rescan;
} __attribute__ ((__packed__));

struct btrfs_qgroup_info_item {
	__le64 generation;
	__le64 rfer;
	__le64 rfer_cmpr;
	__le64 excl;
	__le64 excl_cmpr;
} __attribute__ ((__packed__));

/*
 * Flags definition for qgroup limits
 *
 * Used by:
 * struct btrfs_qgroup_limit.flags
 * struct btrfs_qgroup_limit_item.flags
 */
#define BTRFS_QGROUP_LIMIT_MAX_RFER	(1ULL << 0)
#define BTRFS_QGROUP_LIMIT_MAX_EXCL	(1ULL << 1)
#define BTRFS_QGROUP_LIMIT_RSV_RFER	(1ULL << 2)
#define BTRFS_QGROUP_LIMIT_RSV_EXCL	(1ULL << 3)
#define BTRFS_QGROUP_LIMIT_RFER_CMPR	(1ULL << 4)
#define BTRFS_QGROUP_LIMIT_EXCL_CMPR	(1ULL << 5)

struct btrfs_qgroup_limit_item {
	/* Only updated when any of the other values change. */
	__le64 flags;
	__le64 max_rfer;
	__le64 max_excl;
	__le64 rsv_rfer;
	__le64 rsv_excl;
} __attribute__ ((__packed__));

/*
 * Just in case we somehow lose the roots and are not able to mount,
 * we store an array of the roots from previous transactions in the super.
 */
#define BTRFS_NUM_BACKUP_ROOTS 4
struct btrfs_root_backup {
	__le64 tree_root;
	__le64 tree_root_gen;

	__le64 chunk_root;
	__le64 chunk_root_gen;

	__le64 extent_root;
	__le64 extent_root_gen;

	__le64 fs_root;
	__le64 fs_root_gen;

	__le64 dev_root;
	__le64 dev_root_gen;

	__le64 csum_root;
	__le64 csum_root_gen;

	__le64 total_bytes;
	__le64 bytes_used;
	__le64 num_devices;
	/* future */
	__le64 unused_64[4];

	u8 tree_root_level;
	u8 chunk_root_level;
	u8 extent_root_level;
	u8 fs_root_level;
	u8 dev_root_level;
	u8 csum_root_level;
	/* future and to align */
	u8 unused_8[10];
} __attribute__ ((__packed__));

/*
 * This is a very generous portion of the super block, giving us room to
 * translate 14 chunks with 3 stripes each.
 */
#define BTRFS_SYSTEM_CHUNK_ARRAY_SIZE 2048

#define BTRFS_LABEL_SIZE 256

/* The super block basically lists the main trees of the FS. */
struct btrfs_super_block {
	/* The first 4 fields must match struct btrfs_header */
	u8 csum[BTRFS_CSUM_SIZE];
	/* FS specific UUID, visible to user */
	u8 fsid[BTRFS_FSID_SIZE];
	__le64 bytenr; /* this block number */
	__le64 flags;

	/* Allowed to be different from the btrfs_header from here own down. */
	__le64 magic;
	__le64 generation;
	__le64 root;
	__le64 chunk_root;
	__le64 log_root;

	/* This will help find the new super based on the log root. */
	__le64 log_root_transid;
	__le64 total_bytes;
	__le64 bytes_used;
	__le64 root_dir_objectid;
	__le64 num_devices;
	__le32 sectorsize;
	__le32 nodesize;
	__le32 __unused_leafsize;
	__le32 stripesize;
	__le32 sys_chunk_array_size;
	__le64 chunk_root_generation;
	__le64 compat_flags;
	__le64 compat_ro_flags;
	__le64 incompat_flags;
	__le16 csum_type;
	u8 root_level;
	u8 chunk_root_level;
	u8 log_root_level;
	struct btrfs_dev_item dev_item;

	char label[BTRFS_LABEL_SIZE];

	__le64 cache_generation;
	__le64 uuid_tree_generation;

	/* The UUID written into btree blocks */
	u8 metadata_uuid[BTRFS_FSID_SIZE];

	/* Future expansion */
	__le64 reserved[28];
	u8 sys_chunk_array[BTRFS_SYSTEM_CHUNK_ARRAY_SIZE];
	struct btrfs_root_backup super_roots[BTRFS_NUM_BACKUP_ROOTS];
} __attribute__ ((__packed__));

/*
 * Feature flags
 *
 * Used by:
 * struct btrfs_super_block::(compat|compat_ro|incompat)_flags
 * struct btrfs_ioctl_feature_flags
 */
#define BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE		(1ULL << 0)

/*
 * Older kernels (< 4.9) on big-endian systems produced broken free space tree
 * bitmaps, and btrfs-progs also used to corrupt the free space tree (versions
 * < 4.7.3).  If this bit is clear, then the free space tree cannot be trusted.
 * btrfs-progs can also intentionally clear this bit to ask the kernel to
 * rebuild the free space tree, however this might not work on older kernels
 * that do not know about this bit. If not sure, clear the cache manually on
 * first mount when booting older kernel versions.
 */
#define BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE_VALID	(1ULL << 1)

#define BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF	(1ULL << 0)
#define BTRFS_FEATURE_INCOMPAT_DEFAULT_SUBVOL	(1ULL << 1)
#define BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS	(1ULL << 2)
#define BTRFS_FEATURE_INCOMPAT_COMPRESS_LZO	(1ULL << 3)
#define BTRFS_FEATURE_INCOMPAT_COMPRESS_ZSTD	(1ULL << 4)

/*
 * Older kernels tried to do bigger metadata blocks, but the
 * code was pretty buggy.  Lets not let them try anymore.
 */
#define BTRFS_FEATURE_INCOMPAT_BIG_METADATA	(1ULL << 5)

#define BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF	(1ULL << 6)
#define BTRFS_FEATURE_INCOMPAT_RAID56		(1ULL << 7)
#define BTRFS_FEATURE_INCOMPAT_SKINNY_METADATA	(1ULL << 8)
#define BTRFS_FEATURE_INCOMPAT_NO_HOLES		(1ULL << 9)
#define BTRFS_FEATURE_INCOMPAT_METADATA_UUID	(1ULL << 10)
#define BTRFS_FEATURE_INCOMPAT_RAID1C34		(1ULL << 11)

/*
 * Compat flags that we support.
 *
 * If any incompat flags are set other than the ones specified below then we
 * will fail to mount.
 */
#define BTRFS_FEATURE_COMPAT_SUPP		0ULL
#define BTRFS_FEATURE_COMPAT_SAFE_SET		0ULL
#define BTRFS_FEATURE_COMPAT_SAFE_CLEAR		0ULL

#define BTRFS_FEATURE_COMPAT_RO_SUPP			\
	(BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE |	\
	 BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE_VALID)

#define BTRFS_FEATURE_COMPAT_RO_SAFE_SET	0ULL
#define BTRFS_FEATURE_COMPAT_RO_SAFE_CLEAR	0ULL

#define BTRFS_FEATURE_INCOMPAT_SUPP			\
	(BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF |		\
	 BTRFS_FEATURE_INCOMPAT_DEFAULT_SUBVOL |	\
	 BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS |		\
	 BTRFS_FEATURE_INCOMPAT_BIG_METADATA |		\
	 BTRFS_FEATURE_INCOMPAT_COMPRESS_LZO |		\
	 BTRFS_FEATURE_INCOMPAT_COMPRESS_ZSTD |		\
	 BTRFS_FEATURE_INCOMPAT_RAID56 |		\
	 BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF |		\
	 BTRFS_FEATURE_INCOMPAT_SKINNY_METADATA |	\
	 BTRFS_FEATURE_INCOMPAT_NO_HOLES	|	\
	 BTRFS_FEATURE_INCOMPAT_METADATA_UUID	|	\
	 BTRFS_FEATURE_INCOMPAT_RAID1C34)

#define BTRFS_FEATURE_INCOMPAT_SAFE_SET			\
	(BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF)
#define BTRFS_FEATURE_INCOMPAT_SAFE_CLEAR		0ULL

#define BTRFS_BACKREF_REV_MAX		256
#define BTRFS_BACKREF_REV_SHIFT		56
#define BTRFS_BACKREF_REV_MASK		(((u64)BTRFS_BACKREF_REV_MAX - 1) << \
					 BTRFS_BACKREF_REV_SHIFT)

#define BTRFS_OLD_BACKREF_REV		0
#define BTRFS_MIXED_BACKREF_REV		1

#define BTRFS_MAX_LEVEL 8

/* Every tree block (leaf or node) starts with this header. */
struct btrfs_header {
	/* These first four must match the super block */
	u8 csum[BTRFS_CSUM_SIZE];
	u8 fsid[BTRFS_FSID_SIZE]; /* FS specific uuid */
	__le64 bytenr; /* Which block this node is supposed to live in */
	__le64 flags;

	/* Allowed to be different from the super from here on down. */
	u8 chunk_tree_uuid[BTRFS_UUID_SIZE];
	__le64 generation;
	__le64 owner;
	__le32 nritems;
	u8 level;
} __attribute__ ((__packed__));

/*
 * A leaf is full of items. Offset and size tell us where to find
 * the item in the leaf (relative to the start of the data area).
 */
struct btrfs_item {
	struct btrfs_disk_key key;
	__le32 offset;
	__le32 size;
} __attribute__ ((__packed__));

/*
 * leaves have an item area and a data area:
 * [item0, item1....itemN] [free space] [dataN...data1, data0]
 *
 * The data is separate from the items to get the keys closer together
 * during searches.
 */
struct btrfs_leaf {
	struct btrfs_header header;
	struct btrfs_item items[];
} __attribute__ ((__packed__));

/*
 * All non-leaf blocks are nodes, they hold only keys and pointers to children
 * blocks.
 */
struct btrfs_key_ptr {
	struct btrfs_disk_key key;
	__le64 blockptr;
	__le64 generation;
} __attribute__ ((__packed__));

struct btrfs_node {
	struct btrfs_header header;
	struct btrfs_key_ptr ptrs[];
} __attribute__ ((__packed__));

#endif /* __BTRFS_TREE_H__ */
