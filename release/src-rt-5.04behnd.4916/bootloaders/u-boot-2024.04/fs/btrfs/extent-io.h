// SPDX-License-Identifier: GPL-2.0+

/*
 * Crossported from btrfs-progs/extent_io.h
 *
 * Modification includes:
 * - extent_buffer:data
 *   Use pointer to provide better alignment.
 * - Remove max_cache_size related interfaces
 *   Includes free_extent_buffer_nocache()
 *   As we don't cache eb in U-Boot.
 * - Include headers
 *
 * Write related functions are kept as we still need to modify dummy extent
 * buffers even in RO environment.
 */
#ifndef __BTRFS_EXTENT_IO_H__
#define __BTRFS_EXTENT_IO_H__

#include <linux/types.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/bitops.h>
#include <fs_internal.h>
#include "extent-cache.h"

#define EXTENT_DIRTY		(1U << 0)
#define EXTENT_WRITEBACK	(1U << 1)
#define EXTENT_UPTODATE		(1U << 2)
#define EXTENT_LOCKED		(1U << 3)
#define EXTENT_NEW		(1U << 4)
#define EXTENT_DELALLOC		(1U << 5)
#define EXTENT_DEFRAG		(1U << 6)
#define EXTENT_DEFRAG_DONE	(1U << 7)
#define EXTENT_BUFFER_FILLED	(1U << 8)
#define EXTENT_CSUM		(1U << 9)
#define EXTENT_BAD_TRANSID	(1U << 10)
#define EXTENT_BUFFER_DUMMY	(1U << 11)
#define EXTENT_IOBITS (EXTENT_LOCKED | EXTENT_WRITEBACK)

#define BLOCK_GROUP_DATA	(1U << 1)
#define BLOCK_GROUP_METADATA	(1U << 2)
#define BLOCK_GROUP_SYSTEM	(1U << 4)

/*
 * The extent buffer bitmap operations are done with byte granularity instead of
 * word granularity for two reasons:
 * 1. The bitmaps must be little-endian on disk.
 * 2. Bitmap items are not guaranteed to be aligned to a word and therefore a
 *    single word in a bitmap may straddle two pages in the extent buffer.
 */
#define BIT_BYTE(nr) ((nr) / BITS_PER_BYTE)
#define BYTE_MASK ((1 << BITS_PER_BYTE) - 1)
#define BITMAP_FIRST_BYTE_MASK(start) \
	((BYTE_MASK << ((start) & (BITS_PER_BYTE - 1))) & BYTE_MASK)
#define BITMAP_LAST_BYTE_MASK(nbits) \
	(BYTE_MASK >> (-(nbits) & (BITS_PER_BYTE - 1)))

static inline int le_test_bit(int nr, const u8 *addr)
{
	return 1U & (addr[BIT_BYTE(nr)] >> (nr & (BITS_PER_BYTE-1)));
}

struct btrfs_fs_info;

struct extent_io_tree {
	struct cache_tree state;
	struct cache_tree cache;
	u64 cache_size;
};

struct extent_state {
	struct cache_extent cache_node;
	u64 start;
	u64 end;
	int refs;
	unsigned long state;
	u64 xprivate;
};

struct extent_buffer {
	struct cache_extent cache_node;
	u64 start;
	u32 len;
	int refs;
	u32 flags;
	struct btrfs_fs_info *fs_info;
	char *data;
};

static inline void extent_buffer_get(struct extent_buffer *eb)
{
	eb->refs++;
}

void extent_io_tree_init(struct extent_io_tree *tree);
void extent_io_tree_cleanup(struct extent_io_tree *tree);
int set_extent_bits(struct extent_io_tree *tree, u64 start, u64 end, int bits);
int clear_extent_bits(struct extent_io_tree *tree, u64 start, u64 end, int bits);
int find_first_extent_bit(struct extent_io_tree *tree, u64 start,
			  u64 *start_ret, u64 *end_ret, int bits);
int test_range_bit(struct extent_io_tree *tree, u64 start, u64 end,
		   int bits, int filled);
int set_extent_dirty(struct extent_io_tree *tree, u64 start, u64 end);
int clear_extent_dirty(struct extent_io_tree *tree, u64 start, u64 end);
static inline int set_extent_buffer_uptodate(struct extent_buffer *eb)
{
	eb->flags |= EXTENT_UPTODATE;
	return 0;
}

static inline int clear_extent_buffer_uptodate(struct extent_buffer *eb)
{
	eb->flags &= ~EXTENT_UPTODATE;
	return 0;
}

static inline int extent_buffer_uptodate(struct extent_buffer *eb)
{
	if (!eb || IS_ERR(eb))
		return 0;
	if (eb->flags & EXTENT_UPTODATE)
		return 1;
	return 0;
}

int set_state_private(struct extent_io_tree *tree, u64 start, u64 xprivate);
int get_state_private(struct extent_io_tree *tree, u64 start, u64 *xprivate);
struct extent_buffer *find_extent_buffer(struct extent_io_tree *tree,
					 u64 bytenr, u32 blocksize);
struct extent_buffer *find_first_extent_buffer(struct extent_io_tree *tree,
					       u64 start);
struct extent_buffer *alloc_extent_buffer(struct btrfs_fs_info *fs_info,
					  u64 bytenr, u32 blocksize);
struct extent_buffer *btrfs_clone_extent_buffer(struct extent_buffer *src);
struct extent_buffer *alloc_dummy_extent_buffer(struct btrfs_fs_info *fs_info,
						u64 bytenr, u32 blocksize);
void free_extent_buffer(struct extent_buffer *eb);
int read_extent_from_disk(struct blk_desc *desc, struct disk_partition *part,
			  u64 physical, struct extent_buffer *eb,
			  unsigned long offset, unsigned long len);
int memcmp_extent_buffer(const struct extent_buffer *eb, const void *ptrv,
			 unsigned long start, unsigned long len);
void read_extent_buffer(const struct extent_buffer *eb, void *dst,
			unsigned long start, unsigned long len);
void write_extent_buffer(struct extent_buffer *eb, const void *src,
			 unsigned long start, unsigned long len);
void copy_extent_buffer(struct extent_buffer *dst, struct extent_buffer *src,
			unsigned long dst_offset, unsigned long src_offset,
			unsigned long len);
void memmove_extent_buffer(struct extent_buffer *dst, unsigned long dst_offset,
			   unsigned long src_offset, unsigned long len);
void memset_extent_buffer(struct extent_buffer *eb, char c,
			  unsigned long start, unsigned long len);
int extent_buffer_test_bit(struct extent_buffer *eb, unsigned long start,
			   unsigned long nr);
int set_extent_buffer_dirty(struct extent_buffer *eb);
int clear_extent_buffer_dirty(struct extent_buffer *eb);
void extent_buffer_bitmap_clear(struct extent_buffer *eb, unsigned long start,
				unsigned long pos, unsigned long len);
void extent_buffer_bitmap_set(struct extent_buffer *eb, unsigned long start,
			      unsigned long pos, unsigned long len);

#endif
