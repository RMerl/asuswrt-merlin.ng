// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"

static void read_root_item(struct btrfs_path *p, struct btrfs_root_item *item)
{
	u32 len;
	int reset = 0;

	len = btrfs_path_item_size(p);
	memcpy(item, btrfs_path_item_ptr(p, struct btrfs_root_item), len);
	btrfs_root_item_to_cpu(item);

	if (len < sizeof(*item))
		reset = 1;
	if (!reset && item->generation != item->generation_v2) {
		if (item->generation_v2 != 0)
			printf("%s: generation != generation_v2 in root item",
			       __func__);
		reset = 1;
	}
	if (reset) {
		memset(&item->generation_v2, 0,
		       sizeof(*item) - offsetof(struct btrfs_root_item,
						generation_v2));
	}
}

int btrfs_find_root(u64 objectid, struct btrfs_root *root,
		    struct btrfs_root_item *root_item)
{
	struct btrfs_path path;
	struct btrfs_root_item my_root_item;

	if (!btrfs_search_tree_key_type(&btrfs_info.tree_root, objectid,
					BTRFS_ROOT_ITEM_KEY, &path))
		return -1;

	if (!root_item)
		root_item = &my_root_item;
	read_root_item(&path, root_item);

	if (root) {
		root->objectid = objectid;
		root->bytenr = root_item->bytenr;
		root->root_dirid = root_item->root_dirid;
	}

	btrfs_free_path(&path);
	return 0;
}

u64 btrfs_lookup_root_ref(u64 subvolid, struct btrfs_root_ref *refp, char *name)
{
	struct btrfs_path path;
	struct btrfs_key *key;
	struct btrfs_root_ref *ref;
	u64 res = -1ULL;

	key = btrfs_search_tree_key_type(&btrfs_info.tree_root, subvolid,
					       BTRFS_ROOT_BACKREF_KEY, &path);

	if (!key)
		return -1ULL;

	ref = btrfs_path_item_ptr(&path, struct btrfs_root_ref);
	btrfs_root_ref_to_cpu(ref);

	if (refp)
		*refp = *ref;

	if (name) {
		if (ref->name_len > BTRFS_VOL_NAME_MAX) {
			printf("%s: volume name too long: %u\n", __func__,
			       ref->name_len);
			goto out;
		}

		memcpy(name, ref + 1, ref->name_len);
	}

	res = key->offset;
out:
	btrfs_free_path(&path);
	return res;
}

