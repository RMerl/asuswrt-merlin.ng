// SPDX-License-Identifier: GPL-2.0+

#include "ctree.h"

int btrfs_find_last_root(struct btrfs_root *root, u64 objectid,
			struct btrfs_root_item *item, struct btrfs_key *key)
{
	struct btrfs_path *path;
	struct btrfs_key search_key;
	struct btrfs_key found_key;
	struct extent_buffer *l;
	int ret;
	int slot;

	path = btrfs_alloc_path();
	if (!path)
		return -ENOMEM;

	search_key.objectid = objectid;
	search_key.type = BTRFS_ROOT_ITEM_KEY;
	search_key.offset = (u64)-1;

	ret = btrfs_search_slot(NULL, root, &search_key, path, 0, 0);
	if (ret < 0)
		goto out;
	if (path->slots[0] == 0) {
		ret = -ENOENT;
		goto out;
	}

	BUG_ON(ret == 0);
	l = path->nodes[0];
	slot = path->slots[0] - 1;
	btrfs_item_key_to_cpu(l, &found_key, slot);
	if (found_key.type != BTRFS_ROOT_ITEM_KEY ||
	    found_key.objectid != objectid) {
		ret = -ENOENT;
		goto out;
	}
	read_extent_buffer(l, item, btrfs_item_ptr_offset(l, slot),
			   sizeof(*item));
	memcpy(key, &found_key, sizeof(found_key));
	ret = 0;
out:
	btrfs_free_path(path);
	return ret;
}
