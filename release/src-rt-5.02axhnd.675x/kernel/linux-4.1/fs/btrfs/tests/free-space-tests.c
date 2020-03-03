/*
 * Copyright (C) 2013 Fusion IO.  All rights reserved.
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

#include <linux/slab.h>
#include "btrfs-tests.h"
#include "../ctree.h"
#include "../free-space-cache.h"

#define BITS_PER_BITMAP		(PAGE_CACHE_SIZE * 8)
static struct btrfs_block_group_cache *init_test_block_group(void)
{
	struct btrfs_block_group_cache *cache;

	cache = kzalloc(sizeof(*cache), GFP_NOFS);
	if (!cache)
		return NULL;
	cache->free_space_ctl = kzalloc(sizeof(*cache->free_space_ctl),
					GFP_NOFS);
	if (!cache->free_space_ctl) {
		kfree(cache);
		return NULL;
	}

	cache->key.objectid = 0;
	cache->key.offset = 1024 * 1024 * 1024;
	cache->key.type = BTRFS_BLOCK_GROUP_ITEM_KEY;
	cache->sectorsize = 4096;
	cache->full_stripe_len = 4096;

	spin_lock_init(&cache->lock);
	INIT_LIST_HEAD(&cache->list);
	INIT_LIST_HEAD(&cache->cluster_list);
	INIT_LIST_HEAD(&cache->bg_list);

	btrfs_init_free_space_ctl(cache);

	return cache;
}

/*
 * This test just does basic sanity checking, making sure we can add an exten
 * entry and remove space from either end and the middle, and make sure we can
 * remove space that covers adjacent extent entries.
 */
static int test_extents(struct btrfs_block_group_cache *cache)
{
	int ret = 0;

	test_msg("Running extent only tests\n");

	/* First just make sure we can remove an entire entry */
	ret = btrfs_add_free_space(cache, 0, 4 * 1024 * 1024);
	if (ret) {
		test_msg("Error adding initial extents %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 0, 4 * 1024 * 1024);
	if (ret) {
		test_msg("Error removing extent %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, 0, 4 * 1024 * 1024)) {
		test_msg("Full remove left some lingering space\n");
		return -1;
	}

	/* Ok edge and middle cases now */
	ret = btrfs_add_free_space(cache, 0, 4 * 1024 * 1024);
	if (ret) {
		test_msg("Error adding half extent %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 3 * 1024 * 1024, 1 * 1024 * 1024);
	if (ret) {
		test_msg("Error removing tail end %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 0, 1 * 1024 * 1024);
	if (ret) {
		test_msg("Error removing front end %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 2 * 1024 * 1024, 4096);
	if (ret) {
		test_msg("Error removing middle piece %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, 0, 1 * 1024 * 1024)) {
		test_msg("Still have space at the front\n");
		return -1;
	}

	if (test_check_exists(cache, 2 * 1024 * 1024, 4096)) {
		test_msg("Still have space in the middle\n");
		return -1;
	}

	if (test_check_exists(cache, 3 * 1024 * 1024, 1 * 1024 * 1024)) {
		test_msg("Still have space at the end\n");
		return -1;
	}

	/* Cleanup */
	__btrfs_remove_free_space_cache(cache->free_space_ctl);

	return 0;
}

static int test_bitmaps(struct btrfs_block_group_cache *cache)
{
	u64 next_bitmap_offset;
	int ret;

	test_msg("Running bitmap only tests\n");

	ret = test_add_free_space_entry(cache, 0, 4 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't create a bitmap entry %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 0, 4 * 1024 * 1024);
	if (ret) {
		test_msg("Error removing bitmap full range %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, 0, 4 * 1024 * 1024)) {
		test_msg("Left some space in bitmap\n");
		return -1;
	}

	ret = test_add_free_space_entry(cache, 0, 4 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add to our bitmap entry %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 1 * 1024 * 1024, 2 * 1024 * 1024);
	if (ret) {
		test_msg("Couldn't remove middle chunk %d\n", ret);
		return ret;
	}

	/*
	 * The first bitmap we have starts at offset 0 so the next one is just
	 * at the end of the first bitmap.
	 */
	next_bitmap_offset = (u64)(BITS_PER_BITMAP * 4096);

	/* Test a bit straddling two bitmaps */
	ret = test_add_free_space_entry(cache, next_bitmap_offset -
				   (2 * 1024 * 1024), 4 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add space that straddles two bitmaps %d\n",
				ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, next_bitmap_offset -
				      (1 * 1024 * 1024), 2 * 1024 * 1024);
	if (ret) {
		test_msg("Couldn't remove overlapping space %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, next_bitmap_offset - (1 * 1024 * 1024),
			 2 * 1024 * 1024)) {
		test_msg("Left some space when removing overlapping\n");
		return -1;
	}

	__btrfs_remove_free_space_cache(cache->free_space_ctl);

	return 0;
}

/* This is the high grade jackassery */
static int test_bitmaps_and_extents(struct btrfs_block_group_cache *cache)
{
	u64 bitmap_offset = (u64)(BITS_PER_BITMAP * 4096);
	int ret;

	test_msg("Running bitmap and extent tests\n");

	/*
	 * First let's do something simple, an extent at the same offset as the
	 * bitmap, but the free space completely in the extent and then
	 * completely in the bitmap.
	 */
	ret = test_add_free_space_entry(cache, 4 * 1024 * 1024, 1 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't create bitmap entry %d\n", ret);
		return ret;
	}

	ret = test_add_free_space_entry(cache, 0, 1 * 1024 * 1024, 0);
	if (ret) {
		test_msg("Couldn't add extent entry %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 0, 1 * 1024 * 1024);
	if (ret) {
		test_msg("Couldn't remove extent entry %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, 0, 1 * 1024 * 1024)) {
		test_msg("Left remnants after our remove\n");
		return -1;
	}

	/* Now to add back the extent entry and remove from the bitmap */
	ret = test_add_free_space_entry(cache, 0, 1 * 1024 * 1024, 0);
	if (ret) {
		test_msg("Couldn't re-add extent entry %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 4 * 1024 * 1024, 1 * 1024 * 1024);
	if (ret) {
		test_msg("Couldn't remove from bitmap %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, 4 * 1024 * 1024, 1 * 1024 * 1024)) {
		test_msg("Left remnants in the bitmap\n");
		return -1;
	}

	/*
	 * Ok so a little more evil, extent entry and bitmap at the same offset,
	 * removing an overlapping chunk.
	 */
	ret = test_add_free_space_entry(cache, 1 * 1024 * 1024, 4 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add to a bitmap %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 512 * 1024, 3 * 1024 * 1024);
	if (ret) {
		test_msg("Couldn't remove overlapping space %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, 512 * 1024, 3 * 1024 * 1024)) {
		test_msg("Left over pieces after removing overlapping\n");
		return -1;
	}

	__btrfs_remove_free_space_cache(cache->free_space_ctl);

	/* Now with the extent entry offset into the bitmap */
	ret = test_add_free_space_entry(cache, 4 * 1024 * 1024, 4 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add space to the bitmap %d\n", ret);
		return ret;
	}

	ret = test_add_free_space_entry(cache, 2 * 1024 * 1024, 2 * 1024 * 1024, 0);
	if (ret) {
		test_msg("Couldn't add extent to the cache %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 3 * 1024 * 1024, 4 * 1024 * 1024);
	if (ret) {
		test_msg("Problem removing overlapping space %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, 3 * 1024 * 1024, 4 * 1024 * 1024)) {
		test_msg("Left something behind when removing space");
		return -1;
	}

	/*
	 * This has blown up in the past, the extent entry starts before the
	 * bitmap entry, but we're trying to remove an offset that falls
	 * completely within the bitmap range and is in both the extent entry
	 * and the bitmap entry, looks like this
	 *
	 *   [ extent ]
	 *      [ bitmap ]
	 *        [ del ]
	 */
	__btrfs_remove_free_space_cache(cache->free_space_ctl);
	ret = test_add_free_space_entry(cache, bitmap_offset + 4 * 1024 * 1024,
				   4 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add bitmap %d\n", ret);
		return ret;
	}

	ret = test_add_free_space_entry(cache, bitmap_offset - 1 * 1024 * 1024,
				   5 * 1024 * 1024, 0);
	if (ret) {
		test_msg("Couldn't add extent entry %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, bitmap_offset + 1 * 1024 * 1024,
				      5 * 1024 * 1024);
	if (ret) {
		test_msg("Failed to free our space %d\n", ret);
		return ret;
	}

	if (test_check_exists(cache, bitmap_offset + 1 * 1024 * 1024,
			 5 * 1024 * 1024)) {
		test_msg("Left stuff over\n");
		return -1;
	}

	__btrfs_remove_free_space_cache(cache->free_space_ctl);

	/*
	 * This blew up before, we have part of the free space in a bitmap and
	 * then the entirety of the rest of the space in an extent.  This used
	 * to return -EAGAIN back from btrfs_remove_extent, make sure this
	 * doesn't happen.
	 */
	ret = test_add_free_space_entry(cache, 1 * 1024 * 1024, 2 * 1024 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add bitmap entry %d\n", ret);
		return ret;
	}

	ret = test_add_free_space_entry(cache, 3 * 1024 * 1024, 1 * 1024 * 1024, 0);
	if (ret) {
		test_msg("Couldn't add extent entry %d\n", ret);
		return ret;
	}

	ret = btrfs_remove_free_space(cache, 1 * 1024 * 1024, 3 * 1024 * 1024);
	if (ret) {
		test_msg("Error removing bitmap and extent overlapping %d\n", ret);
		return ret;
	}

	__btrfs_remove_free_space_cache(cache->free_space_ctl);
	return 0;
}

/* Used by test_steal_space_from_bitmap_to_extent(). */
static bool test_use_bitmap(struct btrfs_free_space_ctl *ctl,
			    struct btrfs_free_space *info)
{
	return ctl->free_extents > 0;
}

/* Used by test_steal_space_from_bitmap_to_extent(). */
static int
check_num_extents_and_bitmaps(const struct btrfs_block_group_cache *cache,
			      const int num_extents,
			      const int num_bitmaps)
{
	if (cache->free_space_ctl->free_extents != num_extents) {
		test_msg("Incorrect # of extent entries in the cache: %d, expected %d\n",
			 cache->free_space_ctl->free_extents, num_extents);
		return -EINVAL;
	}
	if (cache->free_space_ctl->total_bitmaps != num_bitmaps) {
		test_msg("Incorrect # of extent entries in the cache: %d, expected %d\n",
			 cache->free_space_ctl->total_bitmaps, num_bitmaps);
		return -EINVAL;
	}
	return 0;
}

/* Used by test_steal_space_from_bitmap_to_extent(). */
static int check_cache_empty(struct btrfs_block_group_cache *cache)
{
	u64 offset;
	u64 max_extent_size;

	/*
	 * Now lets confirm that there's absolutely no free space left to
	 * allocate.
	 */
	if (cache->free_space_ctl->free_space != 0) {
		test_msg("Cache free space is not 0\n");
		return -EINVAL;
	}

	/* And any allocation request, no matter how small, should fail now. */
	offset = btrfs_find_space_for_alloc(cache, 0, 4096, 0,
					    &max_extent_size);
	if (offset != 0) {
		test_msg("Space allocation did not fail, returned offset: %llu",
			 offset);
		return -EINVAL;
	}

	/* And no extent nor bitmap entries in the cache anymore. */
	return check_num_extents_and_bitmaps(cache, 0, 0);
}

/*
 * Before we were able to steal free space from a bitmap entry to an extent
 * entry, we could end up with 2 entries representing a contiguous free space.
 * One would be an extent entry and the other a bitmap entry. Since in order
 * to allocate space to a caller we use only 1 entry, we couldn't return that
 * whole range to the caller if it was requested. This forced the caller to
 * either assume ENOSPC or perform several smaller space allocations, which
 * wasn't optimal as they could be spread all over the block group while under
 * concurrency (extra overhead and fragmentation).
 *
 * This stealing approach is benefical, since we always prefer to allocate from
 * extent entries, both for clustered and non-clustered allocation requests.
 */
static int
test_steal_space_from_bitmap_to_extent(struct btrfs_block_group_cache *cache)
{
	int ret;
	u64 offset;
	u64 max_extent_size;

	bool (*use_bitmap_op)(struct btrfs_free_space_ctl *,
			      struct btrfs_free_space *);

	test_msg("Running space stealing from bitmap to extent\n");

	/*
	 * For this test, we want to ensure we end up with an extent entry
	 * immediately adjacent to a bitmap entry, where the bitmap starts
	 * at an offset where the extent entry ends. We keep adding and
	 * removing free space to reach into this state, but to get there
	 * we need to reach a point where marking new free space doesn't
	 * result in adding new extent entries or merging the new space
	 * with existing extent entries - the space ends up being marked
	 * in an existing bitmap that covers the new free space range.
	 *
	 * To get there, we need to reach the threshold defined set at
	 * cache->free_space_ctl->extents_thresh, which currently is
	 * 256 extents on a x86_64 system at least, and a few other
	 * conditions (check free_space_cache.c). Instead of making the
	 * test much longer and complicated, use a "use_bitmap" operation
	 * that forces use of bitmaps as soon as we have at least 1
	 * extent entry.
	 */
	use_bitmap_op = cache->free_space_ctl->op->use_bitmap;
	cache->free_space_ctl->op->use_bitmap = test_use_bitmap;

	/*
	 * Extent entry covering free space range [128Mb - 256Kb, 128Mb - 128Kb[
	 */
	ret = test_add_free_space_entry(cache, 128 * 1024 * 1024 - 256 * 1024,
					128 * 1024, 0);
	if (ret) {
		test_msg("Couldn't add extent entry %d\n", ret);
		return ret;
	}

	/* Bitmap entry covering free space range [128Mb + 512Kb, 256Mb[ */
	ret = test_add_free_space_entry(cache, 128 * 1024 * 1024 + 512 * 1024,
					128 * 1024 * 1024 - 512 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add bitmap entry %d\n", ret);
		return ret;
	}

	ret = check_num_extents_and_bitmaps(cache, 2, 1);
	if (ret)
		return ret;

	/*
	 * Now make only the first 256Kb of the bitmap marked as free, so that
	 * we end up with only the following ranges marked as free space:
	 *
	 * [128Mb - 256Kb, 128Mb - 128Kb[
	 * [128Mb + 512Kb, 128Mb + 768Kb[
	 */
	ret = btrfs_remove_free_space(cache,
				      128 * 1024 * 1024 + 768 * 1024,
				      128 * 1024 * 1024 - 768 * 1024);
	if (ret) {
		test_msg("Failed to free part of bitmap space %d\n", ret);
		return ret;
	}

	/* Confirm that only those 2 ranges are marked as free. */
	if (!test_check_exists(cache, 128 * 1024 * 1024 - 256 * 1024,
			       128 * 1024)) {
		test_msg("Free space range missing\n");
		return -ENOENT;
	}
	if (!test_check_exists(cache, 128 * 1024 * 1024 + 512 * 1024,
			       256 * 1024)) {
		test_msg("Free space range missing\n");
		return -ENOENT;
	}

	/*
	 * Confirm that the bitmap range [128Mb + 768Kb, 256Mb[ isn't marked
	 * as free anymore.
	 */
	if (test_check_exists(cache, 128 * 1024 * 1024 + 768 * 1024,
			      128 * 1024 * 1024 - 768 * 1024)) {
		test_msg("Bitmap region not removed from space cache\n");
		return -EINVAL;
	}

	/*
	 * Confirm that the region [128Mb + 256Kb, 128Mb + 512Kb[, which is
	 * covered by the bitmap, isn't marked as free.
	 */
	if (test_check_exists(cache, 128 * 1024 * 1024 + 256 * 1024,
			      256 * 1024)) {
		test_msg("Invalid bitmap region marked as free\n");
		return -EINVAL;
	}

	/*
	 * Confirm that the region [128Mb, 128Mb + 256Kb[, which is covered
	 * by the bitmap too, isn't marked as free either.
	 */
	if (test_check_exists(cache, 128 * 1024 * 1024,
			      256 * 1024)) {
		test_msg("Invalid bitmap region marked as free\n");
		return -EINVAL;
	}

	/*
	 * Now lets mark the region [128Mb, 128Mb + 512Kb[ as free too. But,
	 * lets make sure the free space cache marks it as free in the bitmap,
	 * and doesn't insert a new extent entry to represent this region.
	 */
	ret = btrfs_add_free_space(cache, 128 * 1024 * 1024, 512 * 1024);
	if (ret) {
		test_msg("Error adding free space: %d\n", ret);
		return ret;
	}
	/* Confirm the region is marked as free. */
	if (!test_check_exists(cache, 128 * 1024 * 1024, 512 * 1024)) {
		test_msg("Bitmap region not marked as free\n");
		return -ENOENT;
	}

	/*
	 * Confirm that no new extent entries or bitmap entries were added to
	 * the cache after adding that free space region.
	 */
	ret = check_num_extents_and_bitmaps(cache, 2, 1);
	if (ret)
		return ret;

	/*
	 * Now lets add a small free space region to the right of the previous
	 * one, which is not contiguous with it and is part of the bitmap too.
	 * The goal is to test that the bitmap entry space stealing doesn't
	 * steal this space region.
	 */
	ret = btrfs_add_free_space(cache, 128 * 1024 * 1024 + 16 * 1024 * 1024,
				   4096);
	if (ret) {
		test_msg("Error adding free space: %d\n", ret);
		return ret;
	}

	/*
	 * Confirm that no new extent entries or bitmap entries were added to
	 * the cache after adding that free space region.
	 */
	ret = check_num_extents_and_bitmaps(cache, 2, 1);
	if (ret)
		return ret;

	/*
	 * Now mark the region [128Mb - 128Kb, 128Mb[ as free too. This will
	 * expand the range covered by the existing extent entry that represents
	 * the free space [128Mb - 256Kb, 128Mb - 128Kb[.
	 */
	ret = btrfs_add_free_space(cache, 128 * 1024 * 1024 - 128 * 1024,
				   128 * 1024);
	if (ret) {
		test_msg("Error adding free space: %d\n", ret);
		return ret;
	}
	/* Confirm the region is marked as free. */
	if (!test_check_exists(cache, 128 * 1024 * 1024 - 128 * 1024,
			       128 * 1024)) {
		test_msg("Extent region not marked as free\n");
		return -ENOENT;
	}

	/*
	 * Confirm that our extent entry didn't stole all free space from the
	 * bitmap, because of the small 4Kb free space region.
	 */
	ret = check_num_extents_and_bitmaps(cache, 2, 1);
	if (ret)
		return ret;

	/*
	 * So now we have the range [128Mb - 256Kb, 128Mb + 768Kb[ as free
	 * space. Without stealing bitmap free space into extent entry space,
	 * we would have all this free space represented by 2 entries in the
	 * cache:
	 *
	 * extent entry covering range: [128Mb - 256Kb, 128Mb[
	 * bitmap entry covering range: [128Mb, 128Mb + 768Kb[
	 *
	 * Attempting to allocate the whole free space (1Mb) would fail, because
	 * we can't allocate from multiple entries.
	 * With the bitmap free space stealing, we get a single extent entry
	 * that represents the 1Mb free space, and therefore we're able to
	 * allocate the whole free space at once.
	 */
	if (!test_check_exists(cache, 128 * 1024 * 1024 - 256 * 1024,
			       1 * 1024 * 1024)) {
		test_msg("Expected region not marked as free\n");
		return -ENOENT;
	}

	if (cache->free_space_ctl->free_space != (1 * 1024 * 1024 + 4096)) {
		test_msg("Cache free space is not 1Mb + 4Kb\n");
		return -EINVAL;
	}

	offset = btrfs_find_space_for_alloc(cache,
					    0, 1 * 1024 * 1024, 0,
					    &max_extent_size);
	if (offset != (128 * 1024 * 1024 - 256 * 1024)) {
		test_msg("Failed to allocate 1Mb from space cache, returned offset is: %llu\n",
			 offset);
		return -EINVAL;
	}

	/* All that remains is a 4Kb free space region in a bitmap. Confirm. */
	ret = check_num_extents_and_bitmaps(cache, 1, 1);
	if (ret)
		return ret;

	if (cache->free_space_ctl->free_space != 4096) {
		test_msg("Cache free space is not 4Kb\n");
		return -EINVAL;
	}

	offset = btrfs_find_space_for_alloc(cache,
					    0, 4096, 0,
					    &max_extent_size);
	if (offset != (128 * 1024 * 1024 + 16 * 1024 * 1024)) {
		test_msg("Failed to allocate 4Kb from space cache, returned offset is: %llu\n",
			 offset);
		return -EINVAL;
	}

	ret = check_cache_empty(cache);
	if (ret)
		return ret;

	__btrfs_remove_free_space_cache(cache->free_space_ctl);

	/*
	 * Now test a similar scenario, but where our extent entry is located
	 * to the right of the bitmap entry, so that we can check that stealing
	 * space from a bitmap to the front of an extent entry works.
	 */

	/*
	 * Extent entry covering free space range [128Mb + 128Kb, 128Mb + 256Kb[
	 */
	ret = test_add_free_space_entry(cache, 128 * 1024 * 1024 + 128 * 1024,
					128 * 1024, 0);
	if (ret) {
		test_msg("Couldn't add extent entry %d\n", ret);
		return ret;
	}

	/* Bitmap entry covering free space range [0, 128Mb - 512Kb[ */
	ret = test_add_free_space_entry(cache, 0,
					128 * 1024 * 1024 - 512 * 1024, 1);
	if (ret) {
		test_msg("Couldn't add bitmap entry %d\n", ret);
		return ret;
	}

	ret = check_num_extents_and_bitmaps(cache, 2, 1);
	if (ret)
		return ret;

	/*
	 * Now make only the last 256Kb of the bitmap marked as free, so that
	 * we end up with only the following ranges marked as free space:
	 *
	 * [128Mb + 128b, 128Mb + 256Kb[
	 * [128Mb - 768Kb, 128Mb - 512Kb[
	 */
	ret = btrfs_remove_free_space(cache,
				      0,
				      128 * 1024 * 1024 - 768 * 1024);
	if (ret) {
		test_msg("Failed to free part of bitmap space %d\n", ret);
		return ret;
	}

	/* Confirm that only those 2 ranges are marked as free. */
	if (!test_check_exists(cache, 128 * 1024 * 1024 + 128 * 1024,
			       128 * 1024)) {
		test_msg("Free space range missing\n");
		return -ENOENT;
	}
	if (!test_check_exists(cache, 128 * 1024 * 1024 - 768 * 1024,
			       256 * 1024)) {
		test_msg("Free space range missing\n");
		return -ENOENT;
	}

	/*
	 * Confirm that the bitmap range [0, 128Mb - 768Kb[ isn't marked
	 * as free anymore.
	 */
	if (test_check_exists(cache, 0,
			      128 * 1024 * 1024 - 768 * 1024)) {
		test_msg("Bitmap region not removed from space cache\n");
		return -EINVAL;
	}

	/*
	 * Confirm that the region [128Mb - 512Kb, 128Mb[, which is
	 * covered by the bitmap, isn't marked as free.
	 */
	if (test_check_exists(cache, 128 * 1024 * 1024 - 512 * 1024,
			      512 * 1024)) {
		test_msg("Invalid bitmap region marked as free\n");
		return -EINVAL;
	}

	/*
	 * Now lets mark the region [128Mb - 512Kb, 128Mb[ as free too. But,
	 * lets make sure the free space cache marks it as free in the bitmap,
	 * and doesn't insert a new extent entry to represent this region.
	 */
	ret = btrfs_add_free_space(cache, 128 * 1024 * 1024 - 512 * 1024,
				   512 * 1024);
	if (ret) {
		test_msg("Error adding free space: %d\n", ret);
		return ret;
	}
	/* Confirm the region is marked as free. */
	if (!test_check_exists(cache, 128 * 1024 * 1024 - 512 * 1024,
			       512 * 1024)) {
		test_msg("Bitmap region not marked as free\n");
		return -ENOENT;
	}

	/*
	 * Confirm that no new extent entries or bitmap entries were added to
	 * the cache after adding that free space region.
	 */
	ret = check_num_extents_and_bitmaps(cache, 2, 1);
	if (ret)
		return ret;

	/*
	 * Now lets add a small free space region to the left of the previous
	 * one, which is not contiguous with it and is part of the bitmap too.
	 * The goal is to test that the bitmap entry space stealing doesn't
	 * steal this space region.
	 */
	ret = btrfs_add_free_space(cache, 32 * 1024 * 1024, 8192);
	if (ret) {
		test_msg("Error adding free space: %d\n", ret);
		return ret;
	}

	/*
	 * Now mark the region [128Mb, 128Mb + 128Kb[ as free too. This will
	 * expand the range covered by the existing extent entry that represents
	 * the free space [128Mb + 128Kb, 128Mb + 256Kb[.
	 */
	ret = btrfs_add_free_space(cache, 128 * 1024 * 1024, 128 * 1024);
	if (ret) {
		test_msg("Error adding free space: %d\n", ret);
		return ret;
	}
	/* Confirm the region is marked as free. */
	if (!test_check_exists(cache, 128 * 1024 * 1024, 128 * 1024)) {
		test_msg("Extent region not marked as free\n");
		return -ENOENT;
	}

	/*
	 * Confirm that our extent entry didn't stole all free space from the
	 * bitmap, because of the small 8Kb free space region.
	 */
	ret = check_num_extents_and_bitmaps(cache, 2, 1);
	if (ret)
		return ret;

	/*
	 * So now we have the range [128Mb - 768Kb, 128Mb + 256Kb[ as free
	 * space. Without stealing bitmap free space into extent entry space,
	 * we would have all this free space represented by 2 entries in the
	 * cache:
	 *
	 * extent entry covering range: [128Mb, 128Mb + 256Kb[
	 * bitmap entry covering range: [128Mb - 768Kb, 128Mb[
	 *
	 * Attempting to allocate the whole free space (1Mb) would fail, because
	 * we can't allocate from multiple entries.
	 * With the bitmap free space stealing, we get a single extent entry
	 * that represents the 1Mb free space, and therefore we're able to
	 * allocate the whole free space at once.
	 */
	if (!test_check_exists(cache, 128 * 1024 * 1024 - 768 * 1024,
			       1 * 1024 * 1024)) {
		test_msg("Expected region not marked as free\n");
		return -ENOENT;
	}

	if (cache->free_space_ctl->free_space != (1 * 1024 * 1024 + 8192)) {
		test_msg("Cache free space is not 1Mb + 8Kb\n");
		return -EINVAL;
	}

	offset = btrfs_find_space_for_alloc(cache,
					    0, 1 * 1024 * 1024, 0,
					    &max_extent_size);
	if (offset != (128 * 1024 * 1024 - 768 * 1024)) {
		test_msg("Failed to allocate 1Mb from space cache, returned offset is: %llu\n",
			 offset);
		return -EINVAL;
	}

	/* All that remains is a 8Kb free space region in a bitmap. Confirm. */
	ret = check_num_extents_and_bitmaps(cache, 1, 1);
	if (ret)
		return ret;

	if (cache->free_space_ctl->free_space != 8192) {
		test_msg("Cache free space is not 8Kb\n");
		return -EINVAL;
	}

	offset = btrfs_find_space_for_alloc(cache,
					    0, 8192, 0,
					    &max_extent_size);
	if (offset != (32 * 1024 * 1024)) {
		test_msg("Failed to allocate 8Kb from space cache, returned offset is: %llu\n",
			 offset);
		return -EINVAL;
	}

	ret = check_cache_empty(cache);
	if (ret)
		return ret;

	cache->free_space_ctl->op->use_bitmap = use_bitmap_op;
	__btrfs_remove_free_space_cache(cache->free_space_ctl);

	return 0;
}

int btrfs_test_free_space_cache(void)
{
	struct btrfs_block_group_cache *cache;
	int ret;

	test_msg("Running btrfs free space cache tests\n");

	cache = init_test_block_group();
	if (!cache) {
		test_msg("Couldn't run the tests\n");
		return 0;
	}

	ret = test_extents(cache);
	if (ret)
		goto out;
	ret = test_bitmaps(cache);
	if (ret)
		goto out;
	ret = test_bitmaps_and_extents(cache);
	if (ret)
		goto out;

	ret = test_steal_space_from_bitmap_to_extent(cache);
out:
	__btrfs_remove_free_space_cache(cache->free_space_ctl);
	kfree(cache->free_space_ctl);
	kfree(cache);
	test_msg("Free space cache tests finished\n");
	return ret;
}
