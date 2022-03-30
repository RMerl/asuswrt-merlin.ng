// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <malloc.h>

struct chunk_map_item {
	struct rb_node node;
	u64 logical;
	u64 length;
	u64 physical;
};

static int add_chunk_mapping(struct btrfs_key *key, struct btrfs_chunk *chunk)
{
	struct btrfs_stripe *stripe;
	u64 block_profile = chunk->type & BTRFS_BLOCK_GROUP_PROFILE_MASK;
	struct rb_node **new = &(btrfs_info.chunks_root.rb_node), *prnt = NULL;
	struct chunk_map_item *map_item;

	if (block_profile && block_profile != BTRFS_BLOCK_GROUP_DUP) {
		printf("%s: unsupported chunk profile %llu\n", __func__,
		       block_profile);
		return -1;
	} else if (!chunk->length) {
		printf("%s: zero length chunk\n", __func__);
		return -1;
	}

	stripe = &chunk->stripe;
	btrfs_stripe_to_cpu(stripe);

	while (*new) {
		struct chunk_map_item *this;

		this = rb_entry(*new, struct chunk_map_item, node);

		prnt = *new;
		if (key->offset < this->logical) {
			new = &((*new)->rb_left);
		} else if (key->offset > this->logical) {
			new = &((*new)->rb_right);
		} else {
			debug("%s: Logical address %llu already in map!\n",
			      __func__, key->offset);
			return 0;
		}
	}

	map_item = malloc(sizeof(struct chunk_map_item));
	if (!map_item)
		return -1;

	map_item->logical = key->offset;
	map_item->length = chunk->length;
	map_item->physical = le64_to_cpu(chunk->stripe.offset);
	rb_link_node(&map_item->node, prnt, new);
	rb_insert_color(&map_item->node, &btrfs_info.chunks_root);

	debug("%s: Mapping %llu to %llu\n", __func__, map_item->logical,
	      map_item->physical);

	return 0;
}

u64 btrfs_map_logical_to_physical(u64 logical)
{
	struct rb_node *node = btrfs_info.chunks_root.rb_node;

	while (node) {
		struct chunk_map_item *item;

		item = rb_entry(node, struct chunk_map_item, node);

		if (item->logical > logical)
			node = node->rb_left;
		else if (logical >= item->logical + item->length)
			node = node->rb_right;
		else
			return item->physical + logical - item->logical;
	}

	printf("%s: Cannot map logical address %llu to physical\n", __func__,
	       logical);

	return -1ULL;
}

void btrfs_chunk_map_exit(void)
{
	struct rb_node *now, *next;
	struct chunk_map_item *item;

	for (now = rb_first_postorder(&btrfs_info.chunks_root); now; now = next)
	{
		item = rb_entry(now, struct chunk_map_item, node);
		next = rb_next_postorder(now);
		free(item);
	}
}

int btrfs_chunk_map_init(void)
{
	u8 sys_chunk_array_copy[sizeof(btrfs_info.sb.sys_chunk_array)];
	u8 * const start = sys_chunk_array_copy;
	u8 * const end = start + btrfs_info.sb.sys_chunk_array_size;
	u8 *cur;
	struct btrfs_key *key;
	struct btrfs_chunk *chunk;

	btrfs_info.chunks_root = RB_ROOT;

	memcpy(sys_chunk_array_copy, btrfs_info.sb.sys_chunk_array,
	       sizeof(sys_chunk_array_copy));

	for (cur = start; cur < end;) {
		key = (struct btrfs_key *) cur;
		cur += sizeof(struct btrfs_key);
		chunk = (struct btrfs_chunk *) cur;

		btrfs_key_to_cpu(key);
		btrfs_chunk_to_cpu(chunk);

		if (key->type != BTRFS_CHUNK_ITEM_KEY) {
			printf("%s: invalid key type %u\n", __func__,
			       key->type);
			return -1;
		}

		if (add_chunk_mapping(key, chunk))
			return -1;

		cur += sizeof(struct btrfs_chunk);
		cur += sizeof(struct btrfs_stripe) * (chunk->num_stripes - 1);
	}

	return 0;
}

int btrfs_read_chunk_tree(void)
{
	struct btrfs_path path;
	struct btrfs_key key, *found_key;
	struct btrfs_chunk *chunk;
	int res = 0;

	key.objectid = BTRFS_FIRST_CHUNK_TREE_OBJECTID;
	key.type = BTRFS_CHUNK_ITEM_KEY;
	key.offset = 0;

	if (btrfs_search_tree(&btrfs_info.chunk_root, &key, &path))
		return -1;

	do {
		found_key = btrfs_path_leaf_key(&path);
		if (btrfs_comp_keys_type(&key, found_key))
			continue;

		chunk = btrfs_path_item_ptr(&path, struct btrfs_chunk);
		btrfs_chunk_to_cpu(chunk);
		if (add_chunk_mapping(found_key, chunk)) {
			res = -1;
			break;
		}
	} while (!(res = btrfs_next_slot(&path)));

	btrfs_free_path(&path);

	if (res < 0)
		return -1;

	return 0;
}
