// SPDX-License-Identifier: GPL-2.0+

/*
 * Crossported from the same named file of btrfs-progs.
 *
 * Minor modification to include headers.
 */
#include <linux/kernel.h>
#include <linux/rbtree.h>
#include <linux/errno.h>
#include <linux/bug.h>
#include <stdlib.h>
#include "extent-cache.h"
#include "common/rbtree-utils.h"

struct cache_extent_search_range {
	u64 objectid;
	u64 start;
	u64 size;
};

static int cache_tree_comp_range(struct rb_node *node, void *data)
{
	struct cache_extent *entry;
	struct cache_extent_search_range *range;

	range = (struct cache_extent_search_range *)data;
	entry = rb_entry(node, struct cache_extent, rb_node);

	if (entry->start + entry->size <= range->start)
		return 1;
	else if (range->start + range->size <= entry->start)
		return -1;
	else
		return 0;
}

static int cache_tree_comp_nodes(struct rb_node *node1, struct rb_node *node2)
{
	struct cache_extent *entry;
	struct cache_extent_search_range range;

	entry = rb_entry(node2, struct cache_extent, rb_node);
	range.start = entry->start;
	range.size = entry->size;

	return cache_tree_comp_range(node1, (void *)&range);
}

static int cache_tree_comp_range2(struct rb_node *node, void *data)
{
	struct cache_extent *entry;
	struct cache_extent_search_range *range;

	range = (struct cache_extent_search_range *)data;
	entry = rb_entry(node, struct cache_extent, rb_node);

	if (entry->objectid < range->objectid)
		return 1;
	else if (entry->objectid > range->objectid)
		return -1;
	else if (entry->start + entry->size <= range->start)
		return 1;
	else if (range->start + range->size <= entry->start)
		return -1;
	else
		return 0;
}

static int cache_tree_comp_nodes2(struct rb_node *node1, struct rb_node *node2)
{
	struct cache_extent *entry;
	struct cache_extent_search_range range;

	entry = rb_entry(node2, struct cache_extent, rb_node);
	range.objectid = entry->objectid;
	range.start = entry->start;
	range.size = entry->size;

	return cache_tree_comp_range2(node1, (void *)&range);
}

void cache_tree_init(struct cache_tree *tree)
{
	tree->root = RB_ROOT;
}

static struct cache_extent *alloc_cache_extent(u64 start, u64 size)
{
	struct cache_extent *pe = malloc(sizeof(*pe));

	if (!pe)
		return pe;

	pe->objectid = 0;
	pe->start = start;
	pe->size = size;
	return pe;
}

int add_cache_extent(struct cache_tree *tree, u64 start, u64 size)
{
	struct cache_extent *pe = alloc_cache_extent(start, size);
	int ret;

	if (!pe)
		return -ENOMEM;

	ret = insert_cache_extent(tree, pe);
	if (ret)
		free(pe);

	return ret;
}

int insert_cache_extent(struct cache_tree *tree, struct cache_extent *pe)
{
	return rb_insert(&tree->root, &pe->rb_node, cache_tree_comp_nodes);
}

int insert_cache_extent2(struct cache_tree *tree, struct cache_extent *pe)
{
	return rb_insert(&tree->root, &pe->rb_node, cache_tree_comp_nodes2);
}

struct cache_extent *lookup_cache_extent(struct cache_tree *tree,
					 u64 start, u64 size)
{
	struct rb_node *node;
	struct cache_extent *entry;
	struct cache_extent_search_range range;

	range.start = start;
	range.size = size;
	node = rb_search(&tree->root, &range, cache_tree_comp_range, NULL);
	if (!node)
		return NULL;

	entry = rb_entry(node, struct cache_extent, rb_node);
	return entry;
}

struct cache_extent *lookup_cache_extent2(struct cache_tree *tree,
					 u64 objectid, u64 start, u64 size)
{
	struct rb_node *node;
	struct cache_extent *entry;
	struct cache_extent_search_range range;

	range.objectid = objectid;
	range.start = start;
	range.size = size;
	node = rb_search(&tree->root, &range, cache_tree_comp_range2, NULL);
	if (!node)
		return NULL;

	entry = rb_entry(node, struct cache_extent, rb_node);
	return entry;
}

struct cache_extent *search_cache_extent(struct cache_tree *tree, u64 start)
{
	struct rb_node *next;
	struct rb_node *node;
	struct cache_extent *entry;
	struct cache_extent_search_range range;

	range.start = start;
	range.size = 1;
	node = rb_search(&tree->root, &range, cache_tree_comp_range, &next);
	if (!node)
		node = next;
	if (!node)
		return NULL;

	entry = rb_entry(node, struct cache_extent, rb_node);
	return entry;
}

struct cache_extent *search_cache_extent2(struct cache_tree *tree,
					 u64 objectid, u64 start)
{
	struct rb_node *next;
	struct rb_node *node;
	struct cache_extent *entry;
	struct cache_extent_search_range range;

	range.objectid = objectid;
	range.start = start;
	range.size = 1;
	node = rb_search(&tree->root, &range, cache_tree_comp_range2, &next);
	if (!node)
		node = next;
	if (!node)
		return NULL;

	entry = rb_entry(node, struct cache_extent, rb_node);
	return entry;
}

struct cache_extent *first_cache_extent(struct cache_tree *tree)
{
	struct rb_node *node = rb_first(&tree->root);

	if (!node)
		return NULL;
	return rb_entry(node, struct cache_extent, rb_node);
}

struct cache_extent *last_cache_extent(struct cache_tree *tree)
{
	struct rb_node *node = rb_last(&tree->root);

	if (!node)
		return NULL;
	return rb_entry(node, struct cache_extent, rb_node);
}

struct cache_extent *prev_cache_extent(struct cache_extent *pe)
{
	struct rb_node *node = rb_prev(&pe->rb_node);

	if (!node)
		return NULL;
	return rb_entry(node, struct cache_extent, rb_node);
}

struct cache_extent *next_cache_extent(struct cache_extent *pe)
{
	struct rb_node *node = rb_next(&pe->rb_node);

	if (!node)
		return NULL;
	return rb_entry(node, struct cache_extent, rb_node);
}

void remove_cache_extent(struct cache_tree *tree, struct cache_extent *pe)
{
	rb_erase(&pe->rb_node, &tree->root);
}

void cache_tree_free_extents(struct cache_tree *tree,
			     free_cache_extent free_func)
{
	struct cache_extent *ce;

	while ((ce = first_cache_extent(tree))) {
		remove_cache_extent(tree, ce);
		free_func(ce);
	}
}

static void free_extent_cache(struct cache_extent *pe)
{
	free(pe);
}

void free_extent_cache_tree(struct cache_tree *tree)
{
	cache_tree_free_extents(tree, free_extent_cache);
}

int add_merge_cache_extent(struct cache_tree *tree, u64 start, u64 size)
{
	struct cache_extent *cache;
	struct cache_extent *next = NULL;
	struct cache_extent *prev = NULL;
	int next_merged = 0;
	int prev_merged = 0;
	int ret = 0;

	if (cache_tree_empty(tree))
		goto insert;

	cache = search_cache_extent(tree, start);
	if (!cache) {
		/*
		 * Either the tree is completely empty, or the no range after
		 * start.
		 * Either way, the last cache_extent should be prev.
		 */
		prev = last_cache_extent(tree);
	} else if (start <= cache->start) {
		next = cache;
		prev = prev_cache_extent(cache);
	} else {
		prev = cache;
		next = next_cache_extent(cache);
	}

	/*
	 * Ensure the range to be inserted won't cover with existings
	 * Or we will need extra loop to do merge
	 */
	BUG_ON(next && start + size > next->start);
	BUG_ON(prev && prev->start + prev->size > start);

	if (next && start + size == next->start) {
		next_merged = 1;
		next->size = next->start + next->size - start;
		next->start = start;
	}
	if (prev && prev->start + prev->size == start) {
		prev_merged = 1;
		if (next_merged) {
			next->size = next->start + next->size - prev->start;
			next->start = prev->start;
			remove_cache_extent(tree, prev);
			free(prev);
		} else {
			prev->size = start + size - prev->start;
		}
	}
insert:
	if (!prev_merged && !next_merged)
		ret = add_cache_extent(tree, start, size);
	return ret;
}
