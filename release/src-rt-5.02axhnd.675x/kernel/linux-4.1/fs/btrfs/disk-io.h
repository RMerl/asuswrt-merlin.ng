/*
 * Copyright (C) 2007 Oracle.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#ifndef __DISKIO__
#define __DISKIO__

#define BTRFS_SUPER_INFO_OFFSET (64 * 1024)
#define BTRFS_SUPER_INFO_SIZE 4096

#define BTRFS_SUPER_MIRROR_MAX	 3
#define BTRFS_SUPER_MIRROR_SHIFT 12

enum btrfs_wq_endio_type {
	BTRFS_WQ_ENDIO_DATA = 0,
	BTRFS_WQ_ENDIO_METADATA = 1,
	BTRFS_WQ_ENDIO_FREE_SPACE = 2,
	BTRFS_WQ_ENDIO_RAID56 = 3,
	BTRFS_WQ_ENDIO_DIO_REPAIR = 4,
};

static inline u64 btrfs_sb_offset(int mirror)
{
	u64 start = 16 * 1024;
	if (mirror)
		return start << (BTRFS_SUPER_MIRROR_SHIFT * mirror);
	return BTRFS_SUPER_INFO_OFFSET;
}

struct btrfs_device;
struct btrfs_fs_devices;

struct extent_buffer *read_tree_block(struct btrfs_root *root, u64 bytenr,
				      u64 parent_transid);
void readahead_tree_block(struct btrfs_root *root, u64 bytenr);
int reada_tree_block_flagged(struct btrfs_root *root, u64 bytenr,
			 int mirror_num, struct extent_buffer **eb);
struct extent_buffer *btrfs_find_create_tree_block(struct btrfs_root *root,
						   u64 bytenr);
void clean_tree_block(struct btrfs_trans_handle *trans,
		      struct btrfs_fs_info *fs_info, struct extent_buffer *buf);
int open_ctree(struct super_block *sb,
	       struct btrfs_fs_devices *fs_devices,
	       char *options);
void close_ctree(struct btrfs_root *root);
int write_ctree_super(struct btrfs_trans_handle *trans,
		      struct btrfs_root *root, int max_mirrors);
struct buffer_head *btrfs_read_dev_super(struct block_device *bdev);
int btrfs_commit_super(struct btrfs_root *root);
struct extent_buffer *btrfs_find_tree_block(struct btrfs_fs_info *fs_info,
					    u64 bytenr);
struct btrfs_root *btrfs_read_fs_root(struct btrfs_root *tree_root,
				      struct btrfs_key *location);
int btrfs_init_fs_root(struct btrfs_root *root);
int btrfs_insert_fs_root(struct btrfs_fs_info *fs_info,
			 struct btrfs_root *root);
void btrfs_free_fs_roots(struct btrfs_fs_info *fs_info);

struct btrfs_root *btrfs_get_fs_root(struct btrfs_fs_info *fs_info,
				     struct btrfs_key *key,
				     bool check_ref);
static inline struct btrfs_root *
btrfs_read_fs_root_no_name(struct btrfs_fs_info *fs_info,
			   struct btrfs_key *location)
{
	return btrfs_get_fs_root(fs_info, location, true);
}

int btrfs_cleanup_fs_roots(struct btrfs_fs_info *fs_info);
void btrfs_btree_balance_dirty(struct btrfs_root *root);
void btrfs_btree_balance_dirty_nodelay(struct btrfs_root *root);
void btrfs_drop_and_free_fs_root(struct btrfs_fs_info *fs_info,
				 struct btrfs_root *root);
void btrfs_free_fs_root(struct btrfs_root *root);

#ifdef CONFIG_BTRFS_FS_RUN_SANITY_TESTS
struct btrfs_root *btrfs_alloc_dummy_root(void);
#endif

/*
 * This function is used to grab the root, and avoid it is freed when we
 * access it. But it doesn't ensure that the tree is not dropped.
 *
 * If you want to ensure the whole tree is safe, you should use
 * 	fs_info->subvol_srcu
 */
static inline struct btrfs_root *btrfs_grab_fs_root(struct btrfs_root *root)
{
	if (atomic_inc_not_zero(&root->refs))
		return root;
	return NULL;
}

static inline void btrfs_put_fs_root(struct btrfs_root *root)
{
	if (atomic_dec_and_test(&root->refs))
		kfree(root);
}

void btrfs_mark_buffer_dirty(struct extent_buffer *buf);
int btrfs_buffer_uptodate(struct extent_buffer *buf, u64 parent_transid,
			  int atomic);
int btrfs_set_buffer_uptodate(struct extent_buffer *buf);
int btrfs_read_buffer(struct extent_buffer *buf, u64 parent_transid);
u32 btrfs_csum_data(char *data, u32 seed, size_t len);
void btrfs_csum_final(u32 crc, char *result);
int btrfs_bio_wq_end_io(struct btrfs_fs_info *info, struct bio *bio,
			enum btrfs_wq_endio_type metadata);
int btrfs_wq_submit_bio(struct btrfs_fs_info *fs_info, struct inode *inode,
			int rw, struct bio *bio, int mirror_num,
			unsigned long bio_flags, u64 bio_offset,
			extent_submit_bio_hook_t *submit_bio_start,
			extent_submit_bio_hook_t *submit_bio_done);
unsigned long btrfs_async_submit_limit(struct btrfs_fs_info *info);
int btrfs_write_tree_block(struct extent_buffer *buf);
int btrfs_wait_tree_block_writeback(struct extent_buffer *buf);
int btrfs_init_log_root_tree(struct btrfs_trans_handle *trans,
			     struct btrfs_fs_info *fs_info);
int btrfs_add_log_tree(struct btrfs_trans_handle *trans,
		       struct btrfs_root *root);
void btrfs_cleanup_one_transaction(struct btrfs_transaction *trans,
				  struct btrfs_root *root);
struct btrfs_root *btrfs_create_tree(struct btrfs_trans_handle *trans,
				     struct btrfs_fs_info *fs_info,
				     u64 objectid);
int btree_lock_page_hook(struct page *page, void *data,
				void (*flush_fn)(void *));
int btrfs_calc_num_tolerated_disk_barrier_failures(
	struct btrfs_fs_info *fs_info);
int __init btrfs_end_io_wq_init(void);
void btrfs_end_io_wq_exit(void);

#ifdef CONFIG_DEBUG_LOCK_ALLOC
void btrfs_init_lockdep(void);
void btrfs_set_buffer_lockdep_class(u64 objectid,
			            struct extent_buffer *eb, int level);
#else
static inline void btrfs_init_lockdep(void)
{ }
static inline void btrfs_set_buffer_lockdep_class(u64 objectid,
					struct extent_buffer *eb, int level)
{
}
#endif
#endif
