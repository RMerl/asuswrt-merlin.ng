/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * From linux/fs/btrfs/ctree.h
 *   Copyright (C) 2007,2008 Oracle.  All rights reserved.
 *
 * Modified in 2017 by Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#ifndef __BTRFS_CTREE_H__
#define __BTRFS_CTREE_H__

#include <common.h>
#include <compiler.h>
#include "btrfs_tree.h"

#define BTRFS_MAGIC 0x4D5F53665248425FULL /* ascii _BHRfS_M, no null */

#define BTRFS_MAX_MIRRORS 3

#define BTRFS_MAX_LEVEL 8

#define BTRFS_COMPAT_EXTENT_TREE_V0

/*
 * the max metadata block size.  This limit is somewhat artificial,
 * but the memmove costs go through the roof for larger blocks.
 */
#define BTRFS_MAX_METADATA_BLOCKSIZE 65536

/*
 * we can actually store much bigger names, but lets not confuse the rest
 * of linux
 */
#define BTRFS_NAME_LEN 255

/*
 * Theoretical limit is larger, but we keep this down to a sane
 * value. That should limit greatly the possibility of collisions on
 * inode ref items.
 */
#define BTRFS_LINK_MAX 65535U

static const int btrfs_csum_sizes[] = { 4 };

/* four bytes for CRC32 */
#define BTRFS_EMPTY_DIR_SIZE 0

/* ioprio of readahead is set to idle */
#define BTRFS_IOPRIO_READA (IOPRIO_PRIO_VALUE(IOPRIO_CLASS_IDLE, 0))

#define BTRFS_DIRTY_METADATA_THRESH	SZ_32M

#define BTRFS_MAX_EXTENT_SIZE SZ_128M

/*
 * File system states
 */
#define BTRFS_FS_STATE_ERROR		0
#define BTRFS_FS_STATE_REMOUNTING	1
#define BTRFS_FS_STATE_TRANS_ABORTED	2
#define BTRFS_FS_STATE_DEV_REPLACING	3
#define BTRFS_FS_STATE_DUMMY_FS_INFO	4

#define BTRFS_BACKREF_REV_MAX		256
#define BTRFS_BACKREF_REV_SHIFT		56
#define BTRFS_BACKREF_REV_MASK		(((u64)BTRFS_BACKREF_REV_MAX - 1) << \
					 BTRFS_BACKREF_REV_SHIFT)

#define BTRFS_OLD_BACKREF_REV		0
#define BTRFS_MIXED_BACKREF_REV		1

/*
 * every tree block (leaf or node) starts with this header.
 */
struct btrfs_header {
	/* these first four must match the super block */
	__u8 csum[BTRFS_CSUM_SIZE];
	__u8 fsid[BTRFS_FSID_SIZE]; /* FS specific uuid */
	__u64 bytenr; /* which block this node is supposed to live in */
	__u64 flags;

	/* allowed to be different from the super from here on down */
	__u8 chunk_tree_uuid[BTRFS_UUID_SIZE];
	__u64 generation;
	__u64 owner;
	__u32 nritems;
	__u8 level;
} __attribute__ ((__packed__));

/*
 * this is a very generous portion of the super block, giving us
 * room to translate 14 chunks with 3 stripes each.
 */
#define BTRFS_SYSTEM_CHUNK_ARRAY_SIZE 2048

/*
 * just in case we somehow lose the roots and are not able to mount,
 * we store an array of the roots from previous transactions
 * in the super.
 */
#define BTRFS_NUM_BACKUP_ROOTS 4
struct btrfs_root_backup {
	__u64 tree_root;
	__u64 tree_root_gen;

	__u64 chunk_root;
	__u64 chunk_root_gen;

	__u64 extent_root;
	__u64 extent_root_gen;

	__u64 fs_root;
	__u64 fs_root_gen;

	__u64 dev_root;
	__u64 dev_root_gen;

	__u64 csum_root;
	__u64 csum_root_gen;

	__u64 total_bytes;
	__u64 bytes_used;
	__u64 num_devices;
	/* future */
	__u64 unused_64[4];

	__u8 tree_root_level;
	__u8 chunk_root_level;
	__u8 extent_root_level;
	__u8 fs_root_level;
	__u8 dev_root_level;
	__u8 csum_root_level;
	/* future and to align */
	__u8 unused_8[10];
} __attribute__ ((__packed__));

/*
 * the super block basically lists the main trees of the FS
 * it currently lacks any block count etc etc
 */
struct btrfs_super_block {
	__u8 csum[BTRFS_CSUM_SIZE];
	/* the first 4 fields must match struct btrfs_header */
	__u8 fsid[BTRFS_FSID_SIZE];    /* FS specific uuid */
	__u64 bytenr; /* this block number */
	__u64 flags;

	/* allowed to be different from the btrfs_header from here own down */
	__u64 magic;
	__u64 generation;
	__u64 root;
	__u64 chunk_root;
	__u64 log_root;

	/* this will help find the new super based on the log root */
	__u64 log_root_transid;
	__u64 total_bytes;
	__u64 bytes_used;
	__u64 root_dir_objectid;
	__u64 num_devices;
	__u32 sectorsize;
	__u32 nodesize;
	__u32 __unused_leafsize;
	__u32 stripesize;
	__u32 sys_chunk_array_size;
	__u64 chunk_root_generation;
	__u64 compat_flags;
	__u64 compat_ro_flags;
	__u64 incompat_flags;
	__u16 csum_type;
	__u8 root_level;
	__u8 chunk_root_level;
	__u8 log_root_level;
	struct btrfs_dev_item dev_item;

	char label[BTRFS_LABEL_SIZE];

	__u64 cache_generation;
	__u64 uuid_tree_generation;

	/* future expansion */
	__u64 reserved[30];
	__u8 sys_chunk_array[BTRFS_SYSTEM_CHUNK_ARRAY_SIZE];
	struct btrfs_root_backup super_roots[BTRFS_NUM_BACKUP_ROOTS];
} __attribute__ ((__packed__));

/*
 * Compat flags that we support.  If any incompat flags are set other than the
 * ones specified below then we will fail to mount
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
	 BTRFS_FEATURE_INCOMPAT_RAID56 |		\
	 BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF |		\
	 BTRFS_FEATURE_INCOMPAT_SKINNY_METADATA |	\
	 BTRFS_FEATURE_INCOMPAT_NO_HOLES)

#define BTRFS_FEATURE_INCOMPAT_SAFE_SET			\
	(BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF)
#define BTRFS_FEATURE_INCOMPAT_SAFE_CLEAR		0ULL

/*
 * A leaf is full of items. offset and size tell us where to find
 * the item in the leaf (relative to the start of the data area)
 */
struct btrfs_item {
	struct btrfs_key key;
	__u32 offset;
	__u32 size;
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
 * all non-leaf blocks are nodes, they hold only keys and pointers to
 * other blocks
 */
struct btrfs_key_ptr {
	struct btrfs_key key;
	__u64 blockptr;
	__u64 generation;
} __attribute__ ((__packed__));

struct btrfs_node {
	struct btrfs_header header;
	struct btrfs_key_ptr ptrs[];
} __attribute__ ((__packed__));

union btrfs_tree_node {
	struct btrfs_header header;
	struct btrfs_leaf leaf;
	struct btrfs_node node;
};

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;
typedef __u64 u64;

struct btrfs_path {
	union btrfs_tree_node *nodes[BTRFS_MAX_LEVEL];
	u32 slots[BTRFS_MAX_LEVEL];
};

struct btrfs_root {
	u64 objectid;
	u64 bytenr;
	u64 root_dirid;
};

int btrfs_comp_keys(struct btrfs_key *, struct btrfs_key *);
int btrfs_comp_keys_type(struct btrfs_key *, struct btrfs_key *);
int btrfs_bin_search(union btrfs_tree_node *, struct btrfs_key *, int *);
void btrfs_free_path(struct btrfs_path *);
int btrfs_search_tree(const struct btrfs_root *, struct btrfs_key *,
		      struct btrfs_path *);
int btrfs_prev_slot(struct btrfs_path *);
int btrfs_next_slot(struct btrfs_path *);

static inline struct btrfs_key *btrfs_path_leaf_key(struct btrfs_path *p) {
	return &p->nodes[0]->leaf.items[p->slots[0]].key;
}

static inline struct btrfs_key *
btrfs_search_tree_key_type(const struct btrfs_root *root, u64 objectid,
			   u8 type, struct btrfs_path *path)
{
	struct btrfs_key key, *res;

	key.objectid = objectid;
	key.type = type;
	key.offset = 0;

	if (btrfs_search_tree(root, &key, path))
		return NULL;

	res = btrfs_path_leaf_key(path);
	if (btrfs_comp_keys_type(&key, res)) {
		btrfs_free_path(path);
		return NULL;
	}

	return res;
}

static inline u32 btrfs_path_item_size(struct btrfs_path *p)
{
	return p->nodes[0]->leaf.items[p->slots[0]].size;
}

static inline void *btrfs_leaf_data(struct btrfs_leaf *leaf, u32 slot)
{
	return ((u8 *) leaf) + sizeof(struct btrfs_header)
	       + leaf->items[slot].offset;
}

static inline void *btrfs_path_leaf_data(struct btrfs_path *p)
{
	return btrfs_leaf_data(&p->nodes[0]->leaf, p->slots[0]);
}

#define btrfs_item_ptr(l,s,t)			\
	((t *) btrfs_leaf_data((l),(s)))

#define btrfs_path_item_ptr(p,t)		\
	((t *) btrfs_path_leaf_data((p)))

#endif /* __BTRFS_CTREE_H__ */
