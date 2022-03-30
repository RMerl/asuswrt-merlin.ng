// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <malloc.h>
#include <memalign.h>

int btrfs_comp_keys(struct btrfs_key *a, struct btrfs_key *b)
{
	if (a->objectid > b->objectid)
		return 1;
	if (a->objectid < b->objectid)
		return -1;
	if (a->type > b->type)
		return 1;
	if (a->type < b->type)
		return -1;
	if (a->offset > b->offset)
		return 1;
	if (a->offset < b->offset)
		return -1;
	return 0;
}

int btrfs_comp_keys_type(struct btrfs_key *a, struct btrfs_key *b)
{
	if (a->objectid > b->objectid)
		return 1;
	if (a->objectid < b->objectid)
		return -1;
	if (a->type > b->type)
		return 1;
	if (a->type < b->type)
		return -1;
	return 0;
}

static int generic_bin_search(void *addr, int item_size, struct btrfs_key *key,
			      int max, int *slot)
{
	int low = 0, high = max, mid, ret;
	struct btrfs_key *tmp;

	while (low < high) {
		mid = (low + high) / 2;

		tmp = (struct btrfs_key *) ((u8 *) addr + mid*item_size);
		ret = btrfs_comp_keys(tmp, key);

		if (ret < 0) {
			low = mid + 1;
		} else if (ret > 0) {
			high = mid;
		} else {
			*slot = mid;
			return 0;
		}
	}

	*slot = low;
	return 1;
}

int btrfs_bin_search(union btrfs_tree_node *p, struct btrfs_key *key,
		     int *slot)
{
	void *addr;
	unsigned long size;

	if (p->header.level) {
		addr = p->node.ptrs;
		size = sizeof(struct btrfs_key_ptr);
	} else {
		addr = p->leaf.items;
		size = sizeof(struct btrfs_item);
	}

	return generic_bin_search(addr, size, key, p->header.nritems, slot);
}

static void clear_path(struct btrfs_path *p)
{
	int i;

	for (i = 0; i < BTRFS_MAX_LEVEL; ++i) {
		p->nodes[i] = NULL;
		p->slots[i] = 0;
	}
}

void btrfs_free_path(struct btrfs_path *p)
{
	int i;

	for (i = 0; i < BTRFS_MAX_LEVEL; ++i) {
		if (p->nodes[i])
			free(p->nodes[i]);
	}

	clear_path(p);
}

static int read_tree_node(u64 physical, union btrfs_tree_node **buf)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct btrfs_header, hdr,
				 sizeof(struct btrfs_header));
	unsigned long size, offset = sizeof(*hdr);
	union btrfs_tree_node *res;
	u32 i;

	if (!btrfs_devread(physical, sizeof(*hdr), hdr))
		return -1;

	btrfs_header_to_cpu(hdr);

	if (hdr->level)
		size = sizeof(struct btrfs_node)
		       + hdr->nritems * sizeof(struct btrfs_key_ptr);
	else
		size = btrfs_info.sb.nodesize;

	res = malloc_cache_aligned(size);
	if (!res) {
		debug("%s: malloc failed\n", __func__);
		return -1;
	}

	if (!btrfs_devread(physical + offset, size - offset,
			   ((u8 *) res) + offset)) {
		free(res);
		return -1;
	}

	memcpy(&res->header, hdr, sizeof(*hdr));
	if (hdr->level)
		for (i = 0; i < hdr->nritems; ++i)
			btrfs_key_ptr_to_cpu(&res->node.ptrs[i]);
	else
		for (i = 0; i < hdr->nritems; ++i)
			btrfs_item_to_cpu(&res->leaf.items[i]);

	*buf = res;

	return 0;
}

int btrfs_search_tree(const struct btrfs_root *root, struct btrfs_key *key,
		      struct btrfs_path *p)
{
	u8 lvl, prev_lvl;
	int i, slot, ret;
	u64 logical, physical;
	union btrfs_tree_node *buf;

	clear_path(p);

	logical = root->bytenr;

	for (i = 0; i < BTRFS_MAX_LEVEL; ++i) {
		physical = btrfs_map_logical_to_physical(logical);
		if (physical == -1ULL)
			goto err;

		if (read_tree_node(physical, &buf))
			goto err;

		lvl = buf->header.level;
		if (i && prev_lvl != lvl + 1) {
			printf("%s: invalid level in header at %llu\n",
			       __func__, logical);
			goto err;
		}
		prev_lvl = lvl;

		ret = btrfs_bin_search(buf, key, &slot);
		if (ret < 0)
			goto err;
		if (ret && slot > 0 && lvl)
			slot -= 1;

		p->slots[lvl] = slot;
		p->nodes[lvl] = buf;

		if (lvl) {
			logical = buf->node.ptrs[slot].blockptr;
		} else {
			/*
			 * The path might be invalid if:
			 *   cur leaf max < searched value < next leaf min
			 *
			 * Jump to the next valid element if it exists.
			 */
			if (slot >= buf->header.nritems)
				if (btrfs_next_slot(p) < 0)
					goto err;
			break;
		}
	}

	return 0;
err:
	btrfs_free_path(p);
	return -1;
}

static int jump_leaf(struct btrfs_path *path, int dir)
{
	struct btrfs_path p;
	u32 slot;
	int level = 1, from_level, i;

	dir = dir >= 0 ? 1 : -1;

	p = *path;

	while (level < BTRFS_MAX_LEVEL) {
		if (!p.nodes[level])
			return 1;

		slot = p.slots[level];
		if ((dir > 0 && slot + dir >= p.nodes[level]->header.nritems)
		    || (dir < 0 && !slot))
			level++;
		else
			break;
	}

	if (level == BTRFS_MAX_LEVEL)
		return 1;

	p.slots[level] = slot + dir;
	level--;
	from_level = level;

	while (level >= 0) {
		u64 logical, physical;

		slot = p.slots[level + 1];
		logical = p.nodes[level + 1]->node.ptrs[slot].blockptr;
		physical = btrfs_map_logical_to_physical(logical);
		if (physical == -1ULL)
			goto err;

		if (read_tree_node(physical, &p.nodes[level]))
			goto err;

		if (dir > 0)
			p.slots[level] = 0;
		else
			p.slots[level] = p.nodes[level]->header.nritems - 1;
		level--;
	}

	/* Free rewritten nodes in path */
	for (i = 0; i <= from_level; ++i)
		free(path->nodes[i]);

	*path = p;
	return 0;

err:
	/* Free rewritten nodes in p */
	for (i = level + 1; i <= from_level; ++i)
		free(p.nodes[i]);
	return -1;
}

int btrfs_prev_slot(struct btrfs_path *p)
{
	if (!p->slots[0])
		return jump_leaf(p, -1);

	p->slots[0]--;
	return 0;
}

int btrfs_next_slot(struct btrfs_path *p)
{
	struct btrfs_leaf *leaf = &p->nodes[0]->leaf;

	if (p->slots[0] + 1 >= leaf->header.nritems)
		return jump_leaf(p, 1);

	p->slots[0]++;
	return 0;
}
