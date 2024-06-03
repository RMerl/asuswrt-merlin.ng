/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#ifndef __BTRFS_BTRFS_H__
#define __BTRFS_BTRFS_H__

#include <linux/rbtree.h>
#include "conv-funcs.h"

struct btrfs_info {
	struct btrfs_super_block sb;

	struct btrfs_root tree_root;
	struct btrfs_root fs_root;
	struct btrfs_root chunk_root;

	struct rb_root chunks_root;
};

extern struct btrfs_info btrfs_info;

/* hash.c */
void btrfs_hash_init(void);
u32 btrfs_crc32c(u32, const void *, size_t);
u32 btrfs_csum_data(char *, u32, size_t);
void btrfs_csum_final(u32, void *);

static inline u64 btrfs_name_hash(const char *name, int len)
{
	return btrfs_crc32c((u32) ~1, name, len);
}

/* dev.c */
extern struct blk_desc *btrfs_blk_desc;
extern disk_partition_t *btrfs_part_info;

int btrfs_devread(u64, int, void *);

/* chunk-map.c */
u64 btrfs_map_logical_to_physical(u64);
int btrfs_chunk_map_init(void);
void btrfs_chunk_map_exit(void);
int btrfs_read_chunk_tree(void);

/* compression.c */
u32 btrfs_decompress(u8 type, const char *, u32, char *, u32);

/* super.c */
int btrfs_read_superblock(void);

/* dir-item.c */
typedef int (*btrfs_readdir_callback_t)(const struct btrfs_root *,
					struct btrfs_dir_item *);

int btrfs_lookup_dir_item(const struct btrfs_root *, u64, const char *, int,
			   struct btrfs_dir_item *);
int btrfs_readdir(const struct btrfs_root *, u64, btrfs_readdir_callback_t);

/* root.c */
int btrfs_find_root(u64, struct btrfs_root *, struct btrfs_root_item *);
u64 btrfs_lookup_root_ref(u64, struct btrfs_root_ref *, char *);

/* inode.c */
u64 btrfs_lookup_inode_ref(struct btrfs_root *, u64, struct btrfs_inode_ref *,
			    char *);
int btrfs_lookup_inode(const struct btrfs_root *, struct btrfs_key *,
		        struct btrfs_inode_item *, struct btrfs_root *);
int btrfs_readlink(const struct btrfs_root *, u64, char *);
u64 btrfs_lookup_path(struct btrfs_root *, u64, const char *, u8 *,
		       struct btrfs_inode_item *, int);
u64 btrfs_file_read(const struct btrfs_root *, u64, u64, u64, char *);

/* subvolume.c */
u64 btrfs_get_default_subvol_objectid(void);

/* extent-io.c */
u64 btrfs_read_extent_inline(struct btrfs_path *,
			      struct btrfs_file_extent_item *, u64, u64,
			      char *);
u64 btrfs_read_extent_reg(struct btrfs_path *, struct btrfs_file_extent_item *,
			   u64, u64, char *);

#endif /* !__BTRFS_BTRFS_H__ */
