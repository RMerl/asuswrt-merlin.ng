/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * From linux/include/uapi/linux/btrfs_tree.h
 */

#ifndef __BTRFS_BTRFS_TREE_H__
#define __BTRFS_BTRFS_TREE_H__

#include <common.h>

#define BTRFS_VOL_NAME_MAX 255
#define BTRFS_NAME_MAX 255
#define BTRFS_LABEL_SIZE 256
#define BTRFS_FSID_SIZE 16
#define BTRFS_UUID_SIZE 16

/*
 * This header contains the structure definitions and constants used
 * by file system objects that can be retrieved using
 * the BTRFS_IOC_SEARCH_TREE ioctl.  That means basically anything that
 * is needed to describe a leaf node's key or item contents.
 */

/* holds pointers to all of the tree roots */
#define BTRFS_ROOT_TREE_OBJECTID 1ULL

/* stores information about which extents are in use, and reference counts */
#define BTRFS_EXTENT_TREE_OBJECTID 2ULL

/*
 * chunk tree stores translations from logical -> physical block numbering
 * the super block points to the chunk tree
 */
#define BTRFS_CHUNK_TREE_OBJECTID 3ULL

/*
 * stores information about which areas of a given device are in use.
 * one per device.  The tree of tree roots points to the device tree
 */
#define BTRFS_DEV_TREE_OBJECTID 4ULL

/* one per subvolume, storing files and directories */
#define BTRFS_FS_TREE_OBJECTID 5ULL

/* directory objectid inside the root tree */
#define BTRFS_ROOT_TREE_DIR_OBJECTID 6ULL

/* holds checksums of all the data extents */
#define BTRFS_CSUM_TREE_OBJECTID 7ULL

/* holds quota configuration and tracking */
#define BTRFS_QUOTA_TREE_OBJECTID 8ULL

/* for storing items that use the BTRFS_UUID_KEY* types */
#define BTRFS_UUID_TREE_OBJECTID 9ULL

/* tracks free space in block groups. */
#define BTRFS_FREE_SPACE_TREE_OBJECTID 10ULL

/* device stats in the device tree */
#define BTRFS_DEV_STATS_OBJECTID 0ULL

/* for storing balance parameters in the root tree */
#define BTRFS_BALANCE_OBJECTID -4ULL

/* orhpan objectid for tracking unlinked/truncated files */
#define BTRFS_ORPHAN_OBJECTID -5ULL

/* does write ahead logging to speed up fsyncs */
#define BTRFS_TREE_LOG_OBJECTID -6ULL
#define BTRFS_TREE_LOG_FIXUP_OBJECTID -7ULL

/* for space balancing */
#define BTRFS_TREE_RELOC_OBJECTID -8ULL
#define BTRFS_DATA_RELOC_TREE_OBJECTID -9ULL

/*
 * extent checksums all have this objectid
 * this allows them to share the logging tree
 * for fsyncs
 */
#define BTRFS_EXTENT_CSUM_OBJECTID -10ULL

/* For storing free space cache */
#define BTRFS_FREE_SPACE_OBJECTID -11ULL

/*
 * The inode number assigned to the special inode for storing
 * free ino cache
 */
#define BTRFS_FREE_INO_OBJECTID -12ULL

/* dummy objectid represents multiple objectids */
#define BTRFS_MULTIPLE_OBJECTIDS -255ULL

/*
 * All files have objectids in this range.
 */
#define BTRFS_FIRST_FREE_OBJECTID 256ULL
#define BTRFS_LAST_FREE_OBJECTID -256ULL
#define BTRFS_FIRST_CHUNK_TREE_OBJECTID 256ULL


/*
 * the device items go into the chunk tree.  The key is in the form
 * [ 1 BTRFS_DEV_ITEM_KEY device_id ]
 */
#define BTRFS_DEV_ITEMS_OBJECTID 1ULL

#define BTRFS_BTREE_INODE_OBJECTID 1

#define BTRFS_EMPTY_SUBVOL_DIR_OBJECTID 2

#define BTRFS_DEV_REPLACE_DEVID 0ULL

/*
 * inode items have the data typically returned from stat and store other
 * info about object characteristics.  There is one for every file and dir in
 * the FS
 */
#define BTRFS_INODE_ITEM_KEY		1
#define BTRFS_INODE_REF_KEY		12
#define BTRFS_INODE_EXTREF_KEY		13
#define BTRFS_XATTR_ITEM_KEY		24
#define BTRFS_ORPHAN_ITEM_KEY		48
/* reserve 2-15 close to the inode for later flexibility */

/*
 * dir items are the name -> inode pointers in a directory.  There is one
 * for every name in a directory.
 */
#define BTRFS_DIR_LOG_ITEM_KEY  60
#define BTRFS_DIR_LOG_INDEX_KEY 72
#define BTRFS_DIR_ITEM_KEY	84
#define BTRFS_DIR_INDEX_KEY	96
/*
 * extent data is for file data
 */
#define BTRFS_EXTENT_DATA_KEY	108

/*
 * extent csums are stored in a separate tree and hold csums for
 * an entire extent on disk.
 */
#define BTRFS_EXTENT_CSUM_KEY	128

/*
 * root items point to tree roots.  They are typically in the root
 * tree used by the super block to find all the other trees
 */
#define BTRFS_ROOT_ITEM_KEY	132

/*
 * root backrefs tie subvols and snapshots to the directory entries that
 * reference them
 */
#define BTRFS_ROOT_BACKREF_KEY	144

/*
 * root refs make a fast index for listing all of the snapshots and
 * subvolumes referenced by a given root.  They point directly to the
 * directory item in the root that references the subvol
 */
#define BTRFS_ROOT_REF_KEY	156

/*
 * extent items are in the extent map tree.  These record which blocks
 * are used, and how many references there are to each block
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
 * block groups give us hints into the extent allocation trees.  Which
 * blocks are free etc etc
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
 * instead of extents. A free space bitmap is keyed on
 * (start, FREE_SPACE_BITMAP, length); the corresponding item is a bitmap with
 * (length / sectorsize) bits.
 */
#define BTRFS_FREE_SPACE_BITMAP_KEY 200

#define BTRFS_DEV_EXTENT_KEY	204
#define BTRFS_DEV_ITEM_KEY	216
#define BTRFS_CHUNK_ITEM_KEY	228

/*
 * Records the overall state of the qgroups.
 * There's only one instance of this key present,
 * (0, BTRFS_QGROUP_STATUS_KEY, 0)
 */
#define BTRFS_QGROUP_STATUS_KEY         240
/*
 * Records the currently used space of the qgroup.
 * One key per qgroup, (0, BTRFS_QGROUP_INFO_KEY, qgroupid).
 */
#define BTRFS_QGROUP_INFO_KEY           242
/*
 * Contains the user configured limits for the qgroup.
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

/*
 * Obsolete name, see BTRFS_TEMPORARY_ITEM_KEY.
 */
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

/*
 * Obsolete name, see BTRFS_PERSISTENT_ITEM_KEY
 */
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
 * Persistantly stores the device replace state in the device tree.
 * The key is built like this: (0, BTRFS_DEV_REPLACE_KEY, 0).
 */
#define BTRFS_DEV_REPLACE_KEY	250

/*
 * Stores items that allow to quickly map UUIDs to something else.
 * These items are part of the filesystem UUID tree.
 * The key is built like this:
 * (UUID_upper_64_bits, BTRFS_UUID_KEY*, UUID_lower_64_bits).
 */
#if BTRFS_UUID_SIZE != 16
#error "UUID items require BTRFS_UUID_SIZE == 16!"
#endif
#define BTRFS_UUID_KEY_SUBVOL	251	/* for UUIDs assigned to subvols */
#define BTRFS_UUID_KEY_RECEIVED_SUBVOL	252	/* for UUIDs assigned to
						 * received subvols */

/*
 * string items are for debugging.  They just store a short string of
 * data in the FS
 */
#define BTRFS_STRING_ITEM_KEY	253



/* 32 bytes in various csum fields */
#define BTRFS_CSUM_SIZE 32

/* csum types */
#define BTRFS_CSUM_TYPE_CRC32	0

/*
 * flags definitions for directory entry item type
 *
 * Used by:
 * struct btrfs_dir_item.type
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

/*
 * The key defines the order in the tree, and so it also defines (optimal)
 * block layout.
 *
 * objectid corresponds to the inode number.
 *
 * type tells us things about the object, and is a kind of stream selector.
 * so for a given inode, keys with type of 1 might refer to the inode data,
 * type of 2 may point to file data in the btree and type == 3 may point to
 * extents.
 *
 * offset is the starting byte offset for this key in the stream.
 */

struct btrfs_key {
	__u64 objectid;
	__u8 type;
	__u64 offset;
} __attribute__ ((__packed__));

struct btrfs_dev_item {
	/* the internal btrfs device id */
	__u64 devid;

	/* size of the device */
	__u64 total_bytes;

	/* bytes used */
	__u64 bytes_used;

	/* optimal io alignment for this device */
	__u32 io_align;

	/* optimal io width for this device */
	__u32 io_width;

	/* minimal io size for this device */
	__u32 sector_size;

	/* type and info about this device */
	__u64 type;

	/* expected generation for this device */
	__u64 generation;

	/*
	 * starting byte of this partition on the device,
	 * to allow for stripe alignment in the future
	 */
	__u64 start_offset;

	/* grouping information for allocation decisions */
	__u32 dev_group;

	/* seek speed 0-100 where 100 is fastest */
	__u8 seek_speed;

	/* bandwidth 0-100 where 100 is fastest */
	__u8 bandwidth;

	/* btrfs generated uuid for this device */
	__u8 uuid[BTRFS_UUID_SIZE];

	/* uuid of FS who owns this device */
	__u8 fsid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

struct btrfs_stripe {
	__u64 devid;
	__u64 offset;
	__u8 dev_uuid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

struct btrfs_chunk {
	/* size of this chunk in bytes */
	__u64 length;

	/* objectid of the root referencing this chunk */
	__u64 owner;

	__u64 stripe_len;
	__u64 type;

	/* optimal io alignment for this chunk */
	__u32 io_align;

	/* optimal io width for this chunk */
	__u32 io_width;

	/* minimal io size for this chunk */
	__u32 sector_size;

	/* 2^16 stripes is quite a lot, a second limit is the size of a single
	 * item in the btree
	 */
	__u16 num_stripes;

	/* sub stripes only matter for raid10 */
	__u16 sub_stripes;
	struct btrfs_stripe stripe;
	/* additional stripes go here */
} __attribute__ ((__packed__));

#define BTRFS_FREE_SPACE_EXTENT	1
#define BTRFS_FREE_SPACE_BITMAP	2

struct btrfs_free_space_entry {
	__u64 offset;
	__u64 bytes;
	__u8 type;
} __attribute__ ((__packed__));

struct btrfs_free_space_header {
	struct btrfs_key location;
	__u64 generation;
	__u64 num_entries;
	__u64 num_bitmaps;
} __attribute__ ((__packed__));

#define BTRFS_HEADER_FLAG_WRITTEN	(1ULL << 0)
#define BTRFS_HEADER_FLAG_RELOC		(1ULL << 1)

/* Super block flags */
/* Errors detected */
#define BTRFS_SUPER_FLAG_ERROR		(1ULL << 2)

#define BTRFS_SUPER_FLAG_SEEDING	(1ULL << 32)
#define BTRFS_SUPER_FLAG_METADUMP	(1ULL << 33)


/*
 * items in the extent btree are used to record the objectid of the
 * owner of the block and the number of references
 */

struct btrfs_extent_item {
	__u64 refs;
	__u64 generation;
	__u64 flags;
} __attribute__ ((__packed__));


#define BTRFS_EXTENT_FLAG_DATA		(1ULL << 0)
#define BTRFS_EXTENT_FLAG_TREE_BLOCK	(1ULL << 1)

/* following flags only apply to tree blocks */

/* use full backrefs for extent pointers in the block */
#define BTRFS_BLOCK_FLAG_FULL_BACKREF	(1ULL << 8)

/*
 * this flag is only used internally by scrub and may be changed at any time
 * it is only declared here to avoid collisions
 */
#define BTRFS_EXTENT_FLAG_SUPER		(1ULL << 48)

struct btrfs_tree_block_info {
	struct btrfs_key key;
	__u8 level;
} __attribute__ ((__packed__));

struct btrfs_extent_data_ref {
	__u64 root;
	__u64 objectid;
	__u64 offset;
	__u32 count;
} __attribute__ ((__packed__));

struct btrfs_shared_data_ref {
	__u32 count;
} __attribute__ ((__packed__));

struct btrfs_extent_inline_ref {
	__u8 type;
	__u64 offset;
} __attribute__ ((__packed__));

/* dev extents record free space on individual devices.  The owner
 * field points back to the chunk allocation mapping tree that allocated
 * the extent.  The chunk tree uuid field is a way to double check the owner
 */
struct btrfs_dev_extent {
	__u64 chunk_tree;
	__u64 chunk_objectid;
	__u64 chunk_offset;
	__u64 length;
	__u8 chunk_tree_uuid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

struct btrfs_inode_ref {
	__u64 index;
	__u16 name_len;
	/* name goes here */
} __attribute__ ((__packed__));

struct btrfs_inode_extref {
	__u64 parent_objectid;
	__u64 index;
	__u16 name_len;
	__u8   name[0];
	/* name goes here */
} __attribute__ ((__packed__));

struct btrfs_timespec {
	__u64 sec;
	__u32 nsec;
} __attribute__ ((__packed__));

struct btrfs_inode_item {
	/* nfs style generation number */
	__u64 generation;
	/* transid that last touched this inode */
	__u64 transid;
	__u64 size;
	__u64 nbytes;
	__u64 block_group;
	__u32 nlink;
	__u32 uid;
	__u32 gid;
	__u32 mode;
	__u64 rdev;
	__u64 flags;

	/* modification sequence number for NFS */
	__u64 sequence;

	/*
	 * a little future expansion, for more than this we can
	 * just grow the inode item and version it
	 */
	__u64 reserved[4];
	struct btrfs_timespec atime;
	struct btrfs_timespec ctime;
	struct btrfs_timespec mtime;
	struct btrfs_timespec otime;
} __attribute__ ((__packed__));

struct btrfs_dir_log_item {
	__u64 end;
} __attribute__ ((__packed__));

struct btrfs_dir_item {
	struct btrfs_key location;
	__u64 transid;
	__u16 data_len;
	__u16 name_len;
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
	__u64 generation;
	__u64 root_dirid;
	__u64 bytenr;
	__u64 byte_limit;
	__u64 bytes_used;
	__u64 last_snapshot;
	__u64 flags;
	__u32 refs;
	struct btrfs_key drop_progress;
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
	 * the offset of generation_v2 is also used as the start for the memset
	 * when invalidating the fields.
	 */
	__u64 generation_v2;
	__u8 uuid[BTRFS_UUID_SIZE];
	__u8 parent_uuid[BTRFS_UUID_SIZE];
	__u8 received_uuid[BTRFS_UUID_SIZE];
	__u64 ctransid; /* updated when an inode changes */
	__u64 otransid; /* trans when created */
	__u64 stransid; /* trans when sent. non-zero for received subvol */
	__u64 rtransid; /* trans when received. non-zero for received subvol */
	struct btrfs_timespec ctime;
	struct btrfs_timespec otime;
	struct btrfs_timespec stime;
	struct btrfs_timespec rtime;
	__u64 reserved[8]; /* for future */
} __attribute__ ((__packed__));

/*
 * this is used for both forward and backward root refs
 */
struct btrfs_root_ref {
	__u64 dirid;
	__u64 sequence;
	__u16 name_len;
} __attribute__ ((__packed__));

#define BTRFS_FILE_EXTENT_INLINE 0
#define BTRFS_FILE_EXTENT_REG 1
#define BTRFS_FILE_EXTENT_PREALLOC 2

enum btrfs_compression_type {
	BTRFS_COMPRESS_NONE  = 0,
	BTRFS_COMPRESS_ZLIB  = 1,
	BTRFS_COMPRESS_LZO   = 2,
	BTRFS_COMPRESS_ZSTD  = 3,
	BTRFS_COMPRESS_TYPES = 3,
	BTRFS_COMPRESS_LAST  = 4,
};

struct btrfs_file_extent_item {
	/*
	 * transaction id that created this extent
	 */
	__u64 generation;
	/*
	 * max number of bytes to hold this extent in ram
	 * when we split a compressed extent we can't know how big
	 * each of the resulting pieces will be.  So, this is
	 * an upper limit on the size of the extent in ram instead of
	 * an exact limit.
	 */
	__u64 ram_bytes;

	/*
	 * 32 bits for the various ways we might encode the data,
	 * including compression and encryption.  If any of these
	 * are set to something a given disk format doesn't understand
	 * it is treated like an incompat flag for reading and writing,
	 * but not for stat.
	 */
	__u8 compression;
	__u8 encryption;
	__u16 other_encoding; /* spare for later use */

	/* are we inline data or a real extent? */
	__u8 type;

	/*
	 * disk space consumed by the extent, checksum blocks are included
	 * in these numbers
	 *
	 * At this offset in the structure, the inline extent data start.
	 */
	__u64 disk_bytenr;
	__u64 disk_num_bytes;
	/*
	 * the logical offset in file blocks (no csums)
	 * this extent record is for.  This allows a file extent to point
	 * into the middle of an existing extent on disk, sharing it
	 * between two snapshots (useful if some bytes in the middle of the
	 * extent have changed
	 */
	__u64 offset;
	/*
	 * the logical number of file blocks (no csums included).  This
	 * always reflects the size uncompressed and without encoding.
	 */
	__u64 num_bytes;

} __attribute__ ((__packed__));

struct btrfs_csum_item {
	__u8 csum;
} __attribute__ ((__packed__));

/* different types of block groups (and chunks) */
#define BTRFS_BLOCK_GROUP_DATA		(1ULL << 0)
#define BTRFS_BLOCK_GROUP_SYSTEM	(1ULL << 1)
#define BTRFS_BLOCK_GROUP_METADATA	(1ULL << 2)
#define BTRFS_BLOCK_GROUP_RAID0		(1ULL << 3)
#define BTRFS_BLOCK_GROUP_RAID1		(1ULL << 4)
#define BTRFS_BLOCK_GROUP_DUP		(1ULL << 5)
#define BTRFS_BLOCK_GROUP_RAID10	(1ULL << 6)
#define BTRFS_BLOCK_GROUP_RAID5         (1ULL << 7)
#define BTRFS_BLOCK_GROUP_RAID6         (1ULL << 8)
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
	BTRFS_NR_RAID_TYPES
};

#define BTRFS_BLOCK_GROUP_TYPE_MASK	(BTRFS_BLOCK_GROUP_DATA |    \
					 BTRFS_BLOCK_GROUP_SYSTEM |  \
					 BTRFS_BLOCK_GROUP_METADATA)

#define BTRFS_BLOCK_GROUP_PROFILE_MASK	(BTRFS_BLOCK_GROUP_RAID0 |   \
					 BTRFS_BLOCK_GROUP_RAID1 |   \
					 BTRFS_BLOCK_GROUP_RAID5 |   \
					 BTRFS_BLOCK_GROUP_RAID6 |   \
					 BTRFS_BLOCK_GROUP_DUP |     \
					 BTRFS_BLOCK_GROUP_RAID10)
#define BTRFS_BLOCK_GROUP_RAID56_MASK	(BTRFS_BLOCK_GROUP_RAID5 |   \
					 BTRFS_BLOCK_GROUP_RAID6)

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

#endif /* __BTRFS_BTRFS_TREE_H__ */
