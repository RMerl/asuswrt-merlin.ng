// SPDX-License-Identifier: GPL-2.0+

/*
 * Crossported from the same named file of btrfs-progs.
 *
 * Minor modification to include headers.
 */
#ifndef __BTRFS_EXTENT_CACHE_H__
#define __BTRFS_EXTENT_CACHE_H__

#include <linux/rbtree.h>
#include <linux/types.h>

struct cache_tree {
	struct rb_root root;
};

struct cache_extent {
	struct rb_node rb_node;
	u64 objectid;
	u64 start;
	u64 size;
};

void cache_tree_init(struct cache_tree *tree);

struct cache_extent *first_cache_extent(struct cache_tree *tree);
struct cache_extent *last_cache_extent(struct cache_tree *tree);
struct cache_extent *prev_cache_extent(struct cache_extent *pe);
struct cache_extent *next_cache_extent(struct cache_extent *pe);

/*
 * Find a cache_extent which covers start.
 *
 * If not found, return next cache_extent if possible.
 */
struct cache_extent *search_cache_extent(struct cache_tree *tree, u64 start);

/*
 * Find a cache_extent which restrictly covers start.
 *
 * If not found, return NULL.
 */
struct cache_extent *lookup_cache_extent(struct cache_tree *tree,
					 u64 start, u64 size);

/*
 * Add an non-overlap extent into cache tree
 *
 * If [start, start+size) overlap with existing one, it will return -EEXIST.
 */
int add_cache_extent(struct cache_tree *tree, u64 start, u64 size);

/*
 * Same with add_cache_extent, but with cache_extent strcut.
 */
int insert_cache_extent(struct cache_tree *tree, struct cache_extent *pe);
void remove_cache_extent(struct cache_tree *tree, struct cache_extent *pe);

static inline int cache_tree_empty(struct cache_tree *tree)
{
	return RB_EMPTY_ROOT(&tree->root);
}

typedef void (*free_cache_extent)(struct cache_extent *pe);

void cache_tree_free_extents(struct cache_tree *tree,
			     free_cache_extent free_func);

#define FREE_EXTENT_CACHE_BASED_TREE(name, free_func)		\
static void free_##name##_tree(struct cache_tree *tree)		\
{								\
	cache_tree_free_extents(tree, free_func);		\
}

void free_extent_cache_tree(struct cache_tree *tree);

/*
 * Search a cache_extent with same objectid, and covers start.
 *
 * If not found, return next if possible.
 */
struct cache_extent *search_cache_extent2(struct cache_tree *tree,
					  u64 objectid, u64 start);
/*
 * Search a cache_extent with same objectid, and covers the range
 * [start, start + size)
 *
 * If not found, return next cache_extent if possible.
 */
struct cache_extent *lookup_cache_extent2(struct cache_tree *tree,
					  u64 objectid, u64 start, u64 size);
int insert_cache_extent2(struct cache_tree *tree, struct cache_extent *pe);

/*
 * Insert a cache_extent range [start, start + size).
 *
 * This function may merge with existing cache_extent.
 * NOTE: caller must ensure the inserted range won't cover with any existing
 * range.
 */
int add_merge_cache_extent(struct cache_tree *tree, u64 start, u64 size);

#endif
