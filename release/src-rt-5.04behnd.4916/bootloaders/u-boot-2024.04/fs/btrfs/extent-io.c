// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#include <linux/kernel.h>
#include <linux/bug.h>
#include <malloc.h>
#include <memalign.h>
#include "btrfs.h"
#include "ctree.h"
#include "extent-io.h"
#include "disk-io.h"

void extent_io_tree_init(struct extent_io_tree *tree)
{
	cache_tree_init(&tree->state);
	cache_tree_init(&tree->cache);
	tree->cache_size = 0;
}

static struct extent_state *alloc_extent_state(void)
{
	struct extent_state *state;

	state = malloc(sizeof(*state));
	if (!state)
		return NULL;
	state->cache_node.objectid = 0;
	state->refs = 1;
	state->state = 0;
	state->xprivate = 0;
	return state;
}

static void btrfs_free_extent_state(struct extent_state *state)
{
	state->refs--;
	BUG_ON(state->refs < 0);
	if (state->refs == 0)
		free(state);
}

static void free_extent_state_func(struct cache_extent *cache)
{
	struct extent_state *es;

	es = container_of(cache, struct extent_state, cache_node);
	btrfs_free_extent_state(es);
}

static void free_extent_buffer_final(struct extent_buffer *eb);
void extent_io_tree_cleanup(struct extent_io_tree *tree)
{
	cache_tree_free_extents(&tree->state, free_extent_state_func);
}

static inline void update_extent_state(struct extent_state *state)
{
	state->cache_node.start = state->start;
	state->cache_node.size = state->end + 1 - state->start;
}

/*
 * Utility function to look for merge candidates inside a given range.
 * Any extents with matching state are merged together into a single
 * extent in the tree. Extents with EXTENT_IO in their state field are
 * not merged
 */
static int merge_state(struct extent_io_tree *tree,
		       struct extent_state *state)
{
	struct extent_state *other;
	struct cache_extent *other_node;

	if (state->state & EXTENT_IOBITS)
		return 0;

	other_node = prev_cache_extent(&state->cache_node);
	if (other_node) {
		other = container_of(other_node, struct extent_state,
				     cache_node);
		if (other->end == state->start - 1 &&
		    other->state == state->state) {
			state->start = other->start;
			update_extent_state(state);
			remove_cache_extent(&tree->state, &other->cache_node);
			btrfs_free_extent_state(other);
		}
	}
	other_node = next_cache_extent(&state->cache_node);
	if (other_node) {
		other = container_of(other_node, struct extent_state,
				     cache_node);
		if (other->start == state->end + 1 &&
		    other->state == state->state) {
			other->start = state->start;
			update_extent_state(other);
			remove_cache_extent(&tree->state, &state->cache_node);
			btrfs_free_extent_state(state);
		}
	}
	return 0;
}

/*
 * insert an extent_state struct into the tree.  'bits' are set on the
 * struct before it is inserted.
 */
static int insert_state(struct extent_io_tree *tree,
			struct extent_state *state, u64 start, u64 end,
			int bits)
{
	int ret;

	BUG_ON(end < start);
	state->state |= bits;
	state->start = start;
	state->end = end;
	update_extent_state(state);
	ret = insert_cache_extent(&tree->state, &state->cache_node);
	BUG_ON(ret);
	merge_state(tree, state);
	return 0;
}

/*
 * split a given extent state struct in two, inserting the preallocated
 * struct 'prealloc' as the newly created second half.  'split' indicates an
 * offset inside 'orig' where it should be split.
 */
static int split_state(struct extent_io_tree *tree, struct extent_state *orig,
		       struct extent_state *prealloc, u64 split)
{
	int ret;
	prealloc->start = orig->start;
	prealloc->end = split - 1;
	prealloc->state = orig->state;
	update_extent_state(prealloc);
	orig->start = split;
	update_extent_state(orig);
	ret = insert_cache_extent(&tree->state, &prealloc->cache_node);
	BUG_ON(ret);
	return 0;
}

/*
 * clear some bits on a range in the tree.
 */
static int clear_state_bit(struct extent_io_tree *tree,
			    struct extent_state *state, int bits)
{
	int ret = state->state & bits;

	state->state &= ~bits;
	if (state->state == 0) {
		remove_cache_extent(&tree->state, &state->cache_node);
		btrfs_free_extent_state(state);
	} else {
		merge_state(tree, state);
	}
	return ret;
}

/*
 * extent_buffer_bitmap_set - set an area of a bitmap
 * @eb: the extent buffer
 * @start: offset of the bitmap item in the extent buffer
 * @pos: bit number of the first bit
 * @len: number of bits to set
 */
void extent_buffer_bitmap_set(struct extent_buffer *eb, unsigned long start,
			      unsigned long pos, unsigned long len)
{
	u8 *p = (u8 *)eb->data + start + BIT_BYTE(pos);
	const unsigned int size = pos + len;
	int bits_to_set = BITS_PER_BYTE - (pos % BITS_PER_BYTE);
	u8 mask_to_set = BITMAP_FIRST_BYTE_MASK(pos);

	while (len >= bits_to_set) {
		*p |= mask_to_set;
		len -= bits_to_set;
		bits_to_set = BITS_PER_BYTE;
		mask_to_set = ~0;
		p++;
	}
	if (len) {
		mask_to_set &= BITMAP_LAST_BYTE_MASK(size);
		*p |= mask_to_set;
	}
}

/*
 * extent_buffer_bitmap_clear - clear an area of a bitmap
 * @eb: the extent buffer
 * @start: offset of the bitmap item in the extent buffer
 * @pos: bit number of the first bit
 * @len: number of bits to clear
 */
void extent_buffer_bitmap_clear(struct extent_buffer *eb, unsigned long start,
				unsigned long pos, unsigned long len)
{
	u8 *p = (u8 *)eb->data + start + BIT_BYTE(pos);
	const unsigned int size = pos + len;
	int bits_to_clear = BITS_PER_BYTE - (pos % BITS_PER_BYTE);
	u8 mask_to_clear = BITMAP_FIRST_BYTE_MASK(pos);

	while (len >= bits_to_clear) {
		*p &= ~mask_to_clear;
		len -= bits_to_clear;
		bits_to_clear = BITS_PER_BYTE;
		mask_to_clear = ~0;
		p++;
	}
	if (len) {
		mask_to_clear &= BITMAP_LAST_BYTE_MASK(size);
		*p &= ~mask_to_clear;
	}
}

/*
 * clear some bits on a range in the tree.
 */
int clear_extent_bits(struct extent_io_tree *tree, u64 start, u64 end, int bits)
{
	struct extent_state *state;
	struct extent_state *prealloc = NULL;
	struct cache_extent *node;
	u64 last_end;
	int err;
	int set = 0;

again:
	if (!prealloc) {
		prealloc = alloc_extent_state();
		if (!prealloc)
			return -ENOMEM;
	}

	/*
	 * this search will find the extents that end after
	 * our range starts
	 */
	node = search_cache_extent(&tree->state, start);
	if (!node)
		goto out;
	state = container_of(node, struct extent_state, cache_node);
	if (state->start > end)
		goto out;
	last_end = state->end;

	/*
	 *     | ---- desired range ---- |
	 *  | state | or
	 *  | ------------- state -------------- |
	 *
	 * We need to split the extent we found, and may flip
	 * bits on second half.
	 *
	 * If the extent we found extends past our range, we
	 * just split and search again.  It'll get split again
	 * the next time though.
	 *
	 * If the extent we found is inside our range, we clear
	 * the desired bit on it.
	 */
	if (state->start < start) {
		err = split_state(tree, state, prealloc, start);
		BUG_ON(err == -EEXIST);
		prealloc = NULL;
		if (err)
			goto out;
		if (state->end <= end) {
			set |= clear_state_bit(tree, state, bits);
			if (last_end == (u64)-1)
				goto out;
			start = last_end + 1;
		} else {
			start = state->start;
		}
		goto search_again;
	}
	/*
	 * | ---- desired range ---- |
	 *                        | state |
	 * We need to split the extent, and clear the bit
	 * on the first half
	 */
	if (state->start <= end && state->end > end) {
		err = split_state(tree, state, prealloc, end + 1);
		BUG_ON(err == -EEXIST);

		set |= clear_state_bit(tree, prealloc, bits);
		prealloc = NULL;
		goto out;
	}

	start = state->end + 1;
	set |= clear_state_bit(tree, state, bits);
	if (last_end == (u64)-1)
		goto out;
	start = last_end + 1;
	goto search_again;
out:
	if (prealloc)
		btrfs_free_extent_state(prealloc);
	return set;

search_again:
	if (start > end)
		goto out;
	goto again;
}

/*
 * set some bits on a range in the tree.
 */
int set_extent_bits(struct extent_io_tree *tree, u64 start, u64 end, int bits)
{
	struct extent_state *state;
	struct extent_state *prealloc = NULL;
	struct cache_extent *node;
	int err = 0;
	u64 last_start;
	u64 last_end;
again:
	if (!prealloc) {
		prealloc = alloc_extent_state();
		if (!prealloc)
			return -ENOMEM;
	}

	/*
	 * this search will find the extents that end after
	 * our range starts
	 */
	node = search_cache_extent(&tree->state, start);
	if (!node) {
		err = insert_state(tree, prealloc, start, end, bits);
		BUG_ON(err == -EEXIST);
		prealloc = NULL;
		goto out;
	}

	state = container_of(node, struct extent_state, cache_node);
	last_start = state->start;
	last_end = state->end;

	/*
	 * | ---- desired range ---- |
	 * | state |
	 *
	 * Just lock what we found and keep going
	 */
	if (state->start == start && state->end <= end) {
		state->state |= bits;
		merge_state(tree, state);
		if (last_end == (u64)-1)
			goto out;
		start = last_end + 1;
		goto search_again;
	}
	/*
	 *     | ---- desired range ---- |
	 * | state |
	 *   or
	 * | ------------- state -------------- |
	 *
	 * We need to split the extent we found, and may flip bits on
	 * second half.
	 *
	 * If the extent we found extends past our
	 * range, we just split and search again.  It'll get split
	 * again the next time though.
	 *
	 * If the extent we found is inside our range, we set the
	 * desired bit on it.
	 */
	if (state->start < start) {
		err = split_state(tree, state, prealloc, start);
		BUG_ON(err == -EEXIST);
		prealloc = NULL;
		if (err)
			goto out;
		if (state->end <= end) {
			state->state |= bits;
			start = state->end + 1;
			merge_state(tree, state);
			if (last_end == (u64)-1)
				goto out;
			start = last_end + 1;
		} else {
			start = state->start;
		}
		goto search_again;
	}
	/*
	 * | ---- desired range ---- |
	 *     | state | or               | state |
	 *
	 * There's a hole, we need to insert something in it and
	 * ignore the extent we found.
	 */
	if (state->start > start) {
		u64 this_end;
		if (end < last_start)
			this_end = end;
		else
			this_end = last_start -1;
		err = insert_state(tree, prealloc, start, this_end,
				bits);
		BUG_ON(err == -EEXIST);
		prealloc = NULL;
		if (err)
			goto out;
		start = this_end + 1;
		goto search_again;
	}
	/*
	 * | ---- desired range ---- |
	 * | ---------- state ---------- |
	 * We need to split the extent, and set the bit
	 * on the first half
	 */
	err = split_state(tree, state, prealloc, end + 1);
	BUG_ON(err == -EEXIST);

	state->state |= bits;
	merge_state(tree, prealloc);
	prealloc = NULL;
out:
	if (prealloc)
		btrfs_free_extent_state(prealloc);
	return err;
search_again:
	if (start > end)
		goto out;
	goto again;
}

int set_extent_dirty(struct extent_io_tree *tree, u64 start, u64 end)
{
	return set_extent_bits(tree, start, end, EXTENT_DIRTY);
}

int clear_extent_dirty(struct extent_io_tree *tree, u64 start, u64 end)
{
	return clear_extent_bits(tree, start, end, EXTENT_DIRTY);
}

int find_first_extent_bit(struct extent_io_tree *tree, u64 start,
			  u64 *start_ret, u64 *end_ret, int bits)
{
	struct cache_extent *node;
	struct extent_state *state;
	int ret = 1;

	/*
	 * this search will find all the extents that end after
	 * our range starts.
	 */
	node = search_cache_extent(&tree->state, start);
	if (!node)
		goto out;

	while(1) {
		state = container_of(node, struct extent_state, cache_node);
		if (state->end >= start && (state->state & bits)) {
			*start_ret = state->start;
			*end_ret = state->end;
			ret = 0;
			break;
		}
		node = next_cache_extent(node);
		if (!node)
			break;
	}
out:
	return ret;
}

int test_range_bit(struct extent_io_tree *tree, u64 start, u64 end,
		   int bits, int filled)
{
	struct extent_state *state = NULL;
	struct cache_extent *node;
	int bitset = 0;

	node = search_cache_extent(&tree->state, start);
	while (node && start <= end) {
		state = container_of(node, struct extent_state, cache_node);

		if (filled && state->start > start) {
			bitset = 0;
			break;
		}
		if (state->start > end)
			break;
		if (state->state & bits) {
			bitset = 1;
			if (!filled)
				break;
		} else if (filled) {
			bitset = 0;
			break;
		}
		start = state->end + 1;
		if (start > end)
			break;
		node = next_cache_extent(node);
		if (!node) {
			if (filled)
				bitset = 0;
			break;
		}
	}
	return bitset;
}

int set_state_private(struct extent_io_tree *tree, u64 start, u64 private)
{
	struct cache_extent *node;
	struct extent_state *state;
	int ret = 0;

	node = search_cache_extent(&tree->state, start);
	if (!node) {
		ret = -ENOENT;
		goto out;
	}
	state = container_of(node, struct extent_state, cache_node);
	if (state->start != start) {
		ret = -ENOENT;
		goto out;
	}
	state->xprivate = private;
out:
	return ret;
}

int get_state_private(struct extent_io_tree *tree, u64 start, u64 *private)
{
	struct cache_extent *node;
	struct extent_state *state;
	int ret = 0;

	node = search_cache_extent(&tree->state, start);
	if (!node) {
		ret = -ENOENT;
		goto out;
	}
	state = container_of(node, struct extent_state, cache_node);
	if (state->start != start) {
		ret = -ENOENT;
		goto out;
	}
	*private = state->xprivate;
out:
	return ret;
}

static struct extent_buffer *__alloc_extent_buffer(struct btrfs_fs_info *info,
						   u64 bytenr, u32 blocksize)
{
	struct extent_buffer *eb;

	eb = calloc(1, sizeof(struct extent_buffer));
	if (!eb)
		return NULL;
	eb->data = malloc_cache_aligned(blocksize);
	if (!eb->data) {
		free(eb);
		return NULL;
	}

	eb->start = bytenr;
	eb->len = blocksize;
	eb->refs = 1;
	eb->flags = 0;
	eb->cache_node.start = bytenr;
	eb->cache_node.size = blocksize;
	eb->fs_info = info;
	memset_extent_buffer(eb, 0, 0, blocksize);

	return eb;
}

struct extent_buffer *btrfs_clone_extent_buffer(struct extent_buffer *src)
{
	struct extent_buffer *new;

	new = __alloc_extent_buffer(src->fs_info, src->start, src->len);
	if (!new)
		return NULL;

	copy_extent_buffer(new, src, 0, 0, src->len);
	new->flags |= EXTENT_BUFFER_DUMMY;

	return new;
}

static void free_extent_buffer_final(struct extent_buffer *eb)
{
	BUG_ON(eb->refs);
	if (!(eb->flags & EXTENT_BUFFER_DUMMY)) {
		struct extent_io_tree *tree = &eb->fs_info->extent_cache;

		remove_cache_extent(&tree->cache, &eb->cache_node);
		BUG_ON(tree->cache_size < eb->len);
		tree->cache_size -= eb->len;
	}
	free(eb->data);
	free(eb);
}

static void free_extent_buffer_internal(struct extent_buffer *eb, bool free_now)
{
	if (!eb || IS_ERR(eb))
		return;

	eb->refs--;
	BUG_ON(eb->refs < 0);
	if (eb->refs == 0) {
		if (eb->flags & EXTENT_DIRTY) {
			error(
			"dirty eb leak (aborted trans): start %llu len %u",
				eb->start, eb->len);
		}
		if (eb->flags & EXTENT_BUFFER_DUMMY || free_now)
			free_extent_buffer_final(eb);
	}
}

void free_extent_buffer(struct extent_buffer *eb)
{
	free_extent_buffer_internal(eb, 1);
}

struct extent_buffer *find_extent_buffer(struct extent_io_tree *tree,
					 u64 bytenr, u32 blocksize)
{
	struct extent_buffer *eb = NULL;
	struct cache_extent *cache;

	cache = lookup_cache_extent(&tree->cache, bytenr, blocksize);
	if (cache && cache->start == bytenr &&
	    cache->size == blocksize) {
		eb = container_of(cache, struct extent_buffer, cache_node);
		eb->refs++;
	}
	return eb;
}

struct extent_buffer *find_first_extent_buffer(struct extent_io_tree *tree,
					       u64 start)
{
	struct extent_buffer *eb = NULL;
	struct cache_extent *cache;

	cache = search_cache_extent(&tree->cache, start);
	if (cache) {
		eb = container_of(cache, struct extent_buffer, cache_node);
		eb->refs++;
	}
	return eb;
}

struct extent_buffer *alloc_extent_buffer(struct btrfs_fs_info *fs_info,
					  u64 bytenr, u32 blocksize)
{
	struct extent_buffer *eb;
	struct extent_io_tree *tree = &fs_info->extent_cache;
	struct cache_extent *cache;

	cache = lookup_cache_extent(&tree->cache, bytenr, blocksize);
	if (cache && cache->start == bytenr &&
	    cache->size == blocksize) {
		eb = container_of(cache, struct extent_buffer, cache_node);
		eb->refs++;
	} else {
		int ret;

		if (cache) {
			eb = container_of(cache, struct extent_buffer,
					  cache_node);
			free_extent_buffer(eb);
		}
		eb = __alloc_extent_buffer(fs_info, bytenr, blocksize);
		if (!eb)
			return NULL;
		ret = insert_cache_extent(&tree->cache, &eb->cache_node);
		if (ret) {
			free(eb);
			return NULL;
		}
		tree->cache_size += blocksize;
	}
	return eb;
}

/*
 * Allocate a dummy extent buffer which won't be inserted into extent buffer
 * cache.
 *
 * This mostly allows super block read write using existing eb infrastructure
 * without pulluting the eb cache.
 *
 * This is especially important to avoid injecting eb->start == SZ_64K, as
 * fuzzed image could have invalid tree bytenr covers super block range,
 * and cause ref count underflow.
 */
struct extent_buffer *alloc_dummy_extent_buffer(struct btrfs_fs_info *fs_info,
						u64 bytenr, u32 blocksize)
{
	struct extent_buffer *ret;

	ret = __alloc_extent_buffer(fs_info, bytenr, blocksize);
	if (!ret)
		return NULL;

	ret->flags |= EXTENT_BUFFER_DUMMY;

	return ret;
}

int read_extent_from_disk(struct blk_desc *desc, struct disk_partition *part,
			  u64 physical, struct extent_buffer *eb,
			  unsigned long offset, unsigned long len)
{
	int ret;

	ret = __btrfs_devread(desc, part, eb->data + offset, len, physical);
	if (ret < 0)
		goto out;
	if (ret != len) {
		ret = -EIO;
		goto out;
	}
	ret = 0;
out:
	return ret;
}

int memcmp_extent_buffer(const struct extent_buffer *eb, const void *ptrv,
			 unsigned long start, unsigned long len)
{
	return memcmp(eb->data + start, ptrv, len);
}

void read_extent_buffer(const struct extent_buffer *eb, void *dst,
			unsigned long start, unsigned long len)
{
	memcpy(dst, eb->data + start, len);
}

void write_extent_buffer(struct extent_buffer *eb, const void *src,
			 unsigned long start, unsigned long len)
{
	memcpy(eb->data + start, src, len);
}

void copy_extent_buffer(struct extent_buffer *dst, struct extent_buffer *src,
			unsigned long dst_offset, unsigned long src_offset,
			unsigned long len)
{
	memcpy(dst->data + dst_offset, src->data + src_offset, len);
}

void memmove_extent_buffer(struct extent_buffer *dst, unsigned long dst_offset,
			   unsigned long src_offset, unsigned long len)
{
	memmove(dst->data + dst_offset, dst->data + src_offset, len);
}

void memset_extent_buffer(struct extent_buffer *eb, char c,
			  unsigned long start, unsigned long len)
{
	memset(eb->data + start, c, len);
}

int extent_buffer_test_bit(struct extent_buffer *eb, unsigned long start,
			   unsigned long nr)
{
	return le_test_bit(nr, (u8 *)eb->data + start);
}

int set_extent_buffer_dirty(struct extent_buffer *eb)
{
	struct extent_io_tree *tree = &eb->fs_info->extent_cache;
	if (!(eb->flags & EXTENT_DIRTY)) {
		eb->flags |= EXTENT_DIRTY;
		set_extent_dirty(tree, eb->start, eb->start + eb->len - 1);
		extent_buffer_get(eb);
	}
	return 0;
}

int clear_extent_buffer_dirty(struct extent_buffer *eb)
{
	struct extent_io_tree *tree = &eb->fs_info->extent_cache;
	if (eb->flags & EXTENT_DIRTY) {
		eb->flags &= ~EXTENT_DIRTY;
		clear_extent_dirty(tree, eb->start, eb->start + eb->len - 1);
		free_extent_buffer(eb);
	}
	return 0;
}
