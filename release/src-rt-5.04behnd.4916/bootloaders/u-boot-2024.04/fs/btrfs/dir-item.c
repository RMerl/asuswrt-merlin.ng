// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#include "btrfs.h"
#include "disk-io.h"

static int verify_dir_item(struct btrfs_root *root,
		    struct extent_buffer *leaf,
		    struct btrfs_dir_item *dir_item)
{
	u16 namelen = BTRFS_NAME_LEN;
	u8 type = btrfs_dir_type(leaf, dir_item);

	if (type == BTRFS_FT_XATTR)
		namelen = XATTR_NAME_MAX;

	if (btrfs_dir_name_len(leaf, dir_item) > namelen) {
		fprintf(stderr, "invalid dir item name len: %u\n",
			(unsigned)btrfs_dir_data_len(leaf, dir_item));
		return 1;
	}

	/* BTRFS_MAX_XATTR_SIZE is the same for all dir items */
	if ((btrfs_dir_data_len(leaf, dir_item) +
	     btrfs_dir_name_len(leaf, dir_item)) >
			BTRFS_MAX_XATTR_SIZE(root->fs_info)) {
		fprintf(stderr, "invalid dir item name + data len: %u + %u\n",
			(unsigned)btrfs_dir_name_len(leaf, dir_item),
			(unsigned)btrfs_dir_data_len(leaf, dir_item));
		return 1;
	}

	return 0;
}

struct btrfs_dir_item *btrfs_match_dir_item_name(struct btrfs_root *root,
			      struct btrfs_path *path,
			      const char *name, int name_len)
{
	struct btrfs_dir_item *dir_item;
	unsigned long name_ptr;
	u32 total_len;
	u32 cur = 0;
	u32 this_len;
	struct extent_buffer *leaf;

	leaf = path->nodes[0];
	dir_item = btrfs_item_ptr(leaf, path->slots[0], struct btrfs_dir_item);
	total_len = btrfs_item_size_nr(leaf, path->slots[0]);
	if (verify_dir_item(root, leaf, dir_item))
		return NULL;

	while(cur < total_len) {
		this_len = sizeof(*dir_item) +
			btrfs_dir_name_len(leaf, dir_item) +
			btrfs_dir_data_len(leaf, dir_item);
		if (this_len > (total_len - cur)) {
			fprintf(stderr, "invalid dir item size\n");
			return NULL;
		}

		name_ptr = (unsigned long)(dir_item + 1);

		if (btrfs_dir_name_len(leaf, dir_item) == name_len &&
		    memcmp_extent_buffer(leaf, name, name_ptr, name_len) == 0)
			return dir_item;

		cur += this_len;
		dir_item = (struct btrfs_dir_item *)((char *)dir_item +
						     this_len);
	}
	return NULL;
}

struct btrfs_dir_item *btrfs_lookup_dir_item(struct btrfs_trans_handle *trans,
					     struct btrfs_root *root,
					     struct btrfs_path *path, u64 dir,
					     const char *name, int name_len,
					     int mod)
{
	int ret;
	struct btrfs_key key;
	int ins_len = mod < 0 ? -1 : 0;
	int cow = mod != 0;
	struct btrfs_key found_key;
	struct extent_buffer *leaf;

	key.objectid = dir;
	key.type = BTRFS_DIR_ITEM_KEY;

	key.offset = btrfs_name_hash(name, name_len);

	ret = btrfs_search_slot(trans, root, &key, path, ins_len, cow);
	if (ret < 0)
		return ERR_PTR(ret);
	if (ret > 0) {
		if (path->slots[0] == 0)
			return NULL;
		path->slots[0]--;
	}

	leaf = path->nodes[0];
	btrfs_item_key_to_cpu(leaf, &found_key, path->slots[0]);

	if (found_key.objectid != dir ||
	    found_key.type != BTRFS_DIR_ITEM_KEY ||
	    found_key.offset != key.offset)
		return NULL;

	return btrfs_match_dir_item_name(root, path, name, name_len);
}

int btrfs_iter_dir(struct btrfs_root *root, u64 ino,
		   btrfs_iter_dir_callback_t callback)
{
	struct btrfs_path path;
	struct btrfs_key key;
	int ret;

	btrfs_init_path(&path);
	key.objectid = ino;
	key.type = BTRFS_DIR_INDEX_KEY;
	key.offset = 0;

	ret = btrfs_search_slot(NULL, root, &key, &path, 0, 0);
	if (ret < 0)
		return ret;
	/* Should not happen */
	if (ret == 0) {
		ret = -EUCLEAN;
		goto out;
	}
	if (path.slots[0] >= btrfs_header_nritems(path.nodes[0])) {
		ret = btrfs_next_leaf(root, &path);
		if (ret < 0)
			goto out;
		if (ret > 0) {
			ret = 0;
			goto out;
		}
	}
	do {
		struct btrfs_dir_item *di;

		btrfs_item_key_to_cpu(path.nodes[0], &key, path.slots[0]);
		if (key.objectid != ino || key.type != BTRFS_DIR_INDEX_KEY)
			break;
		di = btrfs_item_ptr(path.nodes[0], path.slots[0],
				    struct btrfs_dir_item);
		if (verify_dir_item(root, path.nodes[0], di)) {
			ret = -EUCLEAN;
			goto out;
		}
		ret = callback(root, path.nodes[0], di);
		if (ret < 0)
			goto out;
	} while (!(ret = btrfs_next_item(root, &path)));

	if (ret > 0)
		ret = 0;
out:
	btrfs_release_path(&path);
	return ret;
}
