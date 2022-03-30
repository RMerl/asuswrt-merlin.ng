// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Nelson Integration, LLC 2016
 * Author: Eric Nelson<eric@nelint.com>
 *
 */
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <part.h>
#include <linux/ctype.h>
#include <linux/list.h>

struct block_cache_node {
	struct list_head lh;
	int iftype;
	int devnum;
	lbaint_t start;
	lbaint_t blkcnt;
	unsigned long blksz;
	char *cache;
};

static LIST_HEAD(block_cache);

static struct block_cache_stats _stats = {
	.max_blocks_per_entry = 8,
	.max_entries = 32
};

static struct block_cache_node *cache_find(int iftype, int devnum,
					   lbaint_t start, lbaint_t blkcnt,
					   unsigned long blksz)
{
	struct block_cache_node *node;

	list_for_each_entry(node, &block_cache, lh)
		if ((node->iftype == iftype) &&
		    (node->devnum == devnum) &&
		    (node->blksz == blksz) &&
		    (node->start <= start) &&
		    (node->start + node->blkcnt >= start + blkcnt)) {
			if (block_cache.next != &node->lh) {
				/* maintain MRU ordering */
				list_del(&node->lh);
				list_add(&node->lh, &block_cache);
			}
			return node;
		}
	return 0;
}

int blkcache_read(int iftype, int devnum,
		  lbaint_t start, lbaint_t blkcnt,
		  unsigned long blksz, void *buffer)
{
	struct block_cache_node *node = cache_find(iftype, devnum, start,
						   blkcnt, blksz);
	if (node) {
		const char *src = node->cache + (start - node->start) * blksz;
		memcpy(buffer, src, blksz * blkcnt);
		debug("hit: start " LBAF ", count " LBAFU "\n",
		      start, blkcnt);
		++_stats.hits;
		return 1;
	}

	debug("miss: start " LBAF ", count " LBAFU "\n",
	      start, blkcnt);
	++_stats.misses;
	return 0;
}

void blkcache_fill(int iftype, int devnum,
		   lbaint_t start, lbaint_t blkcnt,
		   unsigned long blksz, void const *buffer)
{
	lbaint_t bytes;
	struct block_cache_node *node;

	/* don't cache big stuff */
	if (blkcnt > _stats.max_blocks_per_entry)
		return;

	if (_stats.max_entries == 0)
		return;

	bytes = blksz * blkcnt;
	if (_stats.max_entries <= _stats.entries) {
		/* pop LRU */
		node = (struct block_cache_node *)block_cache.prev;
		list_del(&node->lh);
		_stats.entries--;
		debug("drop: start " LBAF ", count " LBAFU "\n",
		      node->start, node->blkcnt);
		if (node->blkcnt * node->blksz < bytes) {
			free(node->cache);
			node->cache = 0;
		}
	} else {
		node = malloc(sizeof(*node));
		if (!node)
			return;
		node->cache = 0;
	}

	if (!node->cache) {
		node->cache = malloc(bytes);
		if (!node->cache) {
			free(node);
			return;
		}
	}

	debug("fill: start " LBAF ", count " LBAFU "\n",
	      start, blkcnt);

	node->iftype = iftype;
	node->devnum = devnum;
	node->start = start;
	node->blkcnt = blkcnt;
	node->blksz = blksz;
	memcpy(node->cache, buffer, bytes);
	list_add(&node->lh, &block_cache);
	_stats.entries++;
}

void blkcache_invalidate(int iftype, int devnum)
{
	struct list_head *entry, *n;
	struct block_cache_node *node;

	list_for_each_safe(entry, n, &block_cache) {
		node = (struct block_cache_node *)entry;
		if ((node->iftype == iftype) &&
		    (node->devnum == devnum)) {
			list_del(entry);
			free(node->cache);
			free(node);
			--_stats.entries;
		}
	}
}

void blkcache_configure(unsigned blocks, unsigned entries)
{
	struct block_cache_node *node;
	if ((blocks != _stats.max_blocks_per_entry) ||
	    (entries != _stats.max_entries)) {
		/* invalidate cache */
		while (!list_empty(&block_cache)) {
			node = (struct block_cache_node *)block_cache.next;
			list_del(&node->lh);
			free(node->cache);
			free(node);
		}
		_stats.entries = 0;
	}

	_stats.max_blocks_per_entry = blocks;
	_stats.max_entries = entries;

	_stats.hits = 0;
	_stats.misses = 0;
}

void blkcache_stats(struct block_cache_stats *stats)
{
	memcpy(stats, &_stats, sizeof(*stats));
	_stats.hits = 0;
	_stats.misses = 0;
}
