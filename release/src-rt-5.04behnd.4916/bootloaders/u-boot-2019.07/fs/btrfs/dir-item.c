// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"

static int verify_dir_item(struct btrfs_dir_item *item, u32 start, u32 total)
{
	u16 max_len = BTRFS_NAME_LEN;
	u32 end;

	if (item->type >= BTRFS_FT_MAX) {
		printf("%s: invalid dir item type: %i\n", __func__, item->type);
		return 1;
	}

	if (item->type == BTRFS_FT_XATTR)
		max_len = 255; /* XATTR_NAME_MAX */

	end = start + sizeof(*item) + item->name_len;
	if (item->name_len > max_len || end > total) {
		printf("%s: invalid dir item name len: %u\n", __func__,
		       item->name_len);
		return 1;
	}

	return 0;
}

static struct btrfs_dir_item *
btrfs_match_dir_item_name(struct btrfs_path *path, const char *name,
			  int name_len)
{
	struct btrfs_dir_item *item;
	u32 total_len, cur = 0, this_len;
	const char *name_ptr;

	item = btrfs_path_item_ptr(path, struct btrfs_dir_item);

	total_len = btrfs_path_item_size(path);

	while (cur < total_len) {
		btrfs_dir_item_to_cpu(item);
		this_len = sizeof(*item) + item->name_len + item->data_len;
		name_ptr = (const char *) (item + 1);

		if (verify_dir_item(item, cur, total_len))
			return NULL;
		if (item->name_len == name_len && !memcmp(name_ptr, name,
							  name_len))
			return item;

		cur += this_len;
		item = (struct btrfs_dir_item *) ((u8 *) item + this_len);
	}

	return NULL;
}

int btrfs_lookup_dir_item(const struct btrfs_root *root, u64 dir,
			  const char *name, int name_len,
			  struct btrfs_dir_item *item)
{
	struct btrfs_path path;
	struct btrfs_key key;
	struct btrfs_dir_item *res = NULL;

	key.objectid = dir;
	key.type = BTRFS_DIR_ITEM_KEY;
	key.offset = btrfs_name_hash(name, name_len);

	if (btrfs_search_tree(root, &key, &path))
		return -1;

	if (btrfs_comp_keys_type(&key, btrfs_path_leaf_key(&path)))
		goto out;

	res = btrfs_match_dir_item_name(&path, name, name_len);
	if (res)
		*item = *res;
out:
	btrfs_free_path(&path);
	return res ? 0 : -1;
}

int btrfs_readdir(const struct btrfs_root *root, u64 dir,
		  btrfs_readdir_callback_t callback)
{
	struct btrfs_path path;
	struct btrfs_key key, *found_key;
	struct btrfs_dir_item *item;
	int res = 0;

	key.objectid = dir;
	key.type = BTRFS_DIR_INDEX_KEY;
	key.offset = 0;

	if (btrfs_search_tree(root, &key, &path))
		return -1;

	do {
		found_key = btrfs_path_leaf_key(&path);
		if (btrfs_comp_keys_type(&key, found_key))
			break;

		item = btrfs_path_item_ptr(&path, struct btrfs_dir_item);
		btrfs_dir_item_to_cpu(item);

		if (verify_dir_item(item, 0, sizeof(*item) + item->name_len))
			continue;
		if (item->type == BTRFS_FT_XATTR)
			continue;

		if (callback(root, item))
			break;
	} while (!(res = btrfs_next_slot(&path)));

	btrfs_free_path(&path);

	return res < 0 ? -1 : 0;
}
