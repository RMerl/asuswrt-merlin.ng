// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <malloc.h>

u64 btrfs_lookup_inode_ref(struct btrfs_root *root, u64 inr,
			   struct btrfs_inode_ref *refp, char *name)
{
	struct btrfs_path path;
	struct btrfs_key *key;
	struct btrfs_inode_ref *ref;
	u64 res = -1ULL;

	key = btrfs_search_tree_key_type(root, inr, BTRFS_INODE_REF_KEY,
					       &path);

	if (!key)
		return -1ULL;

	ref = btrfs_path_item_ptr(&path, struct btrfs_inode_ref);
	btrfs_inode_ref_to_cpu(ref);

	if (refp)
		*refp = *ref;

	if (name) {
		if (ref->name_len > BTRFS_NAME_MAX) {
			printf("%s: inode name too long: %u\n", __func__,
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

int btrfs_lookup_inode(const struct btrfs_root *root,
		       struct btrfs_key *location,
		       struct btrfs_inode_item *item,
		       struct btrfs_root *new_root)
{
	struct btrfs_root tmp_root = *root;
	struct btrfs_path path;
	int res = -1;

	if (location->type == BTRFS_ROOT_ITEM_KEY) {
		if (btrfs_find_root(location->objectid, &tmp_root, NULL))
			return -1;

		location->objectid = tmp_root.root_dirid;
		location->type = BTRFS_INODE_ITEM_KEY;
		location->offset = 0;
	}

	if (btrfs_search_tree(&tmp_root, location, &path))
		return res;

	if (btrfs_comp_keys(location, btrfs_path_leaf_key(&path)))
		goto out;

	if (item) {
		*item = *btrfs_path_item_ptr(&path, struct btrfs_inode_item);
		btrfs_inode_item_to_cpu(item);
	}

	if (new_root)
		*new_root = tmp_root;

	res = 0;

out:
	btrfs_free_path(&path);
	return res;
}

int btrfs_readlink(const struct btrfs_root *root, u64 inr, char *target)
{
	struct btrfs_path path;
	struct btrfs_key key;
	struct btrfs_file_extent_item *extent;
	const char *data_ptr;
	int res = -1;

	key.objectid = inr;
	key.type = BTRFS_EXTENT_DATA_KEY;
	key.offset = 0;

	if (btrfs_search_tree(root, &key, &path))
		return -1;

	if (btrfs_comp_keys(&key, btrfs_path_leaf_key(&path)))
		goto out;

	extent = btrfs_path_item_ptr(&path, struct btrfs_file_extent_item);
	if (extent->type != BTRFS_FILE_EXTENT_INLINE) {
		printf("%s: Extent for symlink %llu not of INLINE type\n",
		       __func__, inr);
		goto out;
	}

	btrfs_file_extent_item_to_cpu_inl(extent);

	if (extent->compression != BTRFS_COMPRESS_NONE) {
		printf("%s: Symlink %llu extent data compressed!\n", __func__,
		       inr);
		goto out;
	} else if (extent->encryption != 0) {
		printf("%s: Symlink %llu extent data encrypted!\n", __func__,
		       inr);
		goto out;
	} else if (extent->ram_bytes >= btrfs_info.sb.sectorsize) {
		printf("%s: Symlink %llu extent data too long (%llu)!\n",
		       __func__, inr, extent->ram_bytes);
		goto out;
	}

	data_ptr = (const char *) extent
		   + offsetof(struct btrfs_file_extent_item, disk_bytenr);

	memcpy(target, data_ptr, extent->ram_bytes);
	target[extent->ram_bytes] = '\0';
	res = 0;
out:
	btrfs_free_path(&path);
	return res;
}

/* inr must be a directory (for regular files with multiple hard links this
   function returns only one of the parents of the file) */
static u64 get_parent_inode(struct btrfs_root *root, u64 inr,
			    struct btrfs_inode_item *inode_item)
{
	struct btrfs_key key;
	u64 res;

	if (inr == BTRFS_FIRST_FREE_OBJECTID) {
		if (root->objectid != btrfs_info.fs_root.objectid) {
			u64 parent;
			struct btrfs_root_ref ref;

			parent = btrfs_lookup_root_ref(root->objectid, &ref,
						       NULL);
			if (parent == -1ULL)
				return -1ULL;

			if (btrfs_find_root(parent, root, NULL))
				return -1ULL;

			inr = ref.dirid;
		}

		if (inode_item) {
			key.objectid = inr;
			key.type = BTRFS_INODE_ITEM_KEY;
			key.offset = 0;

			if (btrfs_lookup_inode(root, &key, inode_item, NULL))
				return -1ULL;
		}

		return inr;
	}

	res = btrfs_lookup_inode_ref(root, inr, NULL, NULL);
	if (res == -1ULL)
		return -1ULL;

	if (inode_item) {
		key.objectid = res;
		key.type = BTRFS_INODE_ITEM_KEY;
		key.offset = 0;

		if (btrfs_lookup_inode(root, &key, inode_item, NULL))
			return -1ULL;
	}

	return res;
}

static inline int next_length(const char *path)
{
	int res = 0;
	while (*path != '\0' && *path != '/' && res <= BTRFS_NAME_LEN)
		++res, ++path;
	return res;
}

static inline const char *skip_current_directories(const char *cur)
{
	while (1) {
		if (cur[0] == '/')
			++cur;
		else if (cur[0] == '.' && cur[1] == '/')
			cur += 2;
		else
			break;
	}

	return cur;
}

u64 btrfs_lookup_path(struct btrfs_root *root, u64 inr, const char *path,
		      u8 *type_p, struct btrfs_inode_item *inode_item_p,
		      int symlink_limit)
{
	struct btrfs_dir_item item;
	struct btrfs_inode_item inode_item;
	u8 type = BTRFS_FT_DIR;
	int len, have_inode = 0;
	const char *cur = path;

	if (*cur == '/') {
		++cur;
		inr = root->root_dirid;
	}

	do {
		cur = skip_current_directories(cur);

		len = next_length(cur);
		if (len > BTRFS_NAME_LEN) {
			printf("%s: Name too long at \"%.*s\"\n", __func__,
			       BTRFS_NAME_LEN, cur);
			return -1ULL;
		}

		if (len == 1 && cur[0] == '.')
			break;

		if (len == 2 && cur[0] == '.' && cur[1] == '.') {
			cur += 2;
			inr = get_parent_inode(root, inr, &inode_item);
			if (inr == -1ULL)
				return -1ULL;

			type = BTRFS_FT_DIR;
			continue;
		}

		if (!*cur)
			break;
		
		if (btrfs_lookup_dir_item(root, inr, cur, len, &item))
			return -1ULL;

		type = item.type;
		have_inode = 1;
		if (btrfs_lookup_inode(root, &item.location, &inode_item, root))
			return -1ULL;

		if (item.type == BTRFS_FT_SYMLINK && symlink_limit >= 0) {
			char *target;

			if (!symlink_limit) {
				printf("%s: Too much symlinks!\n", __func__);
				return -1ULL;
			}

			target = malloc(min(inode_item.size + 1,
					    (u64) btrfs_info.sb.sectorsize));
			if (!target)
				return -1ULL;

			if (btrfs_readlink(root, item.location.objectid,
					   target)) {
				free(target);
				return -1ULL;
			}

			inr = btrfs_lookup_path(root, inr, target, &type,
						&inode_item, symlink_limit - 1);

			free(target);

			if (inr == -1ULL)
				return -1ULL;
		} else if (item.type != BTRFS_FT_DIR && cur[len]) {
			printf("%s: \"%.*s\" not a directory\n", __func__,
			       (int) (cur - path + len), path);
			return -1ULL;
		} else {
			inr = item.location.objectid;
		}

		cur += len;
	} while (*cur);

	if (type_p)
		*type_p = type;

	if (inode_item_p) {
		if (!have_inode) {
			struct btrfs_key key;

			key.objectid = inr;
			key.type = BTRFS_INODE_ITEM_KEY;
			key.offset = 0;

			if (btrfs_lookup_inode(root, &key, &inode_item, NULL))
				return -1ULL;
		}

		*inode_item_p = inode_item;
	}

	return inr;
}

u64 btrfs_file_read(const struct btrfs_root *root, u64 inr, u64 offset,
		    u64 size, char *buf)
{
	struct btrfs_path path;
	struct btrfs_key key;
	struct btrfs_file_extent_item *extent;
	int res = 0;
	u64 rd, rd_all = -1ULL;

	key.objectid = inr;
	key.type = BTRFS_EXTENT_DATA_KEY;
	key.offset = offset;

	if (btrfs_search_tree(root, &key, &path))
		return -1ULL;

	if (btrfs_comp_keys(&key, btrfs_path_leaf_key(&path)) < 0) {
		if (btrfs_prev_slot(&path))
			goto out;

		if (btrfs_comp_keys_type(&key, btrfs_path_leaf_key(&path)))
			goto out;
	}

	rd_all = 0;

	do {
		if (btrfs_comp_keys_type(&key, btrfs_path_leaf_key(&path)))
			break;

		extent = btrfs_path_item_ptr(&path,
					     struct btrfs_file_extent_item);

		if (extent->type == BTRFS_FILE_EXTENT_INLINE) {
			btrfs_file_extent_item_to_cpu_inl(extent);
			rd = btrfs_read_extent_inline(&path, extent, offset,
						      size, buf);
		} else {
			btrfs_file_extent_item_to_cpu(extent);
			rd = btrfs_read_extent_reg(&path, extent, offset, size,
						   buf);
		}

		if (rd == -1ULL) {
			printf("%s: Error reading extent\n", __func__);
			rd_all = -1;
			goto out;
		}

		offset = 0;
		buf += rd;
		rd_all += rd;
		size -= rd;

		if (!size)
			break;
	} while (!(res = btrfs_next_slot(&path)));

	if (res)
		return -1ULL;

out:
	btrfs_free_path(&path);
	return rd_all;
}
