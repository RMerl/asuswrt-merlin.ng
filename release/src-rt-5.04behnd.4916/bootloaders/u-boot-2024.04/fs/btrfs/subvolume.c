// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#include <malloc.h>
#include "ctree.h"
#include "btrfs.h"
#include "disk-io.h"

/*
 * Resolve the path of ino inside subvolume @root into @path_ret.
 *
 * @path_ret must be at least PATH_MAX size.
 */
static int get_path_in_subvol(struct btrfs_root *root, u64 ino, char *path_ret)
{
	struct btrfs_path path;
	struct btrfs_key key;
	char *tmp;
	u64 cur = ino;
	int ret = 0;

	tmp = malloc(PATH_MAX);
	if (!tmp)
		return -ENOMEM;
	tmp[0] = '\0';

	btrfs_init_path(&path);
	while (cur != BTRFS_FIRST_FREE_OBJECTID) {
		struct btrfs_inode_ref *iref;
		int name_len;

		btrfs_release_path(&path);
		key.objectid = cur;
		key.type = BTRFS_INODE_REF_KEY;
		key.offset = (u64)-1;

		ret = btrfs_search_slot(NULL, root, &key, &path, 0, 0);
		/* Impossible */
		if (ret == 0)
			ret = -EUCLEAN;
		if (ret < 0)
			goto out;
		ret = btrfs_previous_item(root, &path, cur,
					  BTRFS_INODE_REF_KEY);
		if (ret > 0)
			ret = -ENOENT;
		if (ret < 0)
			goto out;

		strncpy(tmp, path_ret, PATH_MAX);
		iref = btrfs_item_ptr(path.nodes[0], path.slots[0],
				      struct btrfs_inode_ref);
		name_len = btrfs_inode_ref_name_len(path.nodes[0],
						    iref);
		if (name_len > BTRFS_NAME_LEN) {
			ret = -ENAMETOOLONG;
			goto out;
		}
		read_extent_buffer(path.nodes[0], path_ret,
				   (unsigned long)(iref + 1), name_len);
		path_ret[name_len] = '/';
		path_ret[name_len + 1] = '\0';
		strncat(path_ret, tmp, PATH_MAX);

		btrfs_item_key_to_cpu(path.nodes[0], &key, path.slots[0]);
		cur = key.offset;
	}
out:
	btrfs_release_path(&path);
	free(tmp);
	return ret;
}

static int list_one_subvol(struct btrfs_root *root, char *path_ret)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	struct btrfs_root *tree_root = fs_info->tree_root;
	struct btrfs_path path;
	struct btrfs_key key;
	char *tmp;
	u64 cur = root->root_key.objectid;
	int ret = 0;

	tmp = malloc(PATH_MAX);
	if (!tmp)
		return -ENOMEM;
	tmp[0] = '\0';
	path_ret[0] = '\0';
	btrfs_init_path(&path);
	while (cur != BTRFS_FS_TREE_OBJECTID) {
		struct btrfs_root_ref *rr;
		struct btrfs_key location;
		int name_len;
		u64 ino;

		key.objectid = cur;
		key.type = BTRFS_ROOT_BACKREF_KEY;
		key.offset = (u64)-1;
		btrfs_release_path(&path);

		ret = btrfs_search_slot(NULL, tree_root, &key, &path, 0, 0);
		if (ret == 0)
			ret = -EUCLEAN;
		if (ret < 0)
			goto out;
		ret = btrfs_previous_item(tree_root, &path, cur,
					  BTRFS_ROOT_BACKREF_KEY);
		if (ret > 0)
			ret = -ENOENT;
		if (ret < 0)
			goto out;

		/* Get the subvolume name */
		rr = btrfs_item_ptr(path.nodes[0], path.slots[0],
				    struct btrfs_root_ref);
		strncpy(tmp, path_ret, PATH_MAX);
		name_len = btrfs_root_ref_name_len(path.nodes[0], rr);
		if (name_len > BTRFS_NAME_LEN) {
			ret = -ENAMETOOLONG;
			goto out;
		}
		ino = btrfs_root_ref_dirid(path.nodes[0], rr);
		read_extent_buffer(path.nodes[0], path_ret,
				   (unsigned long)(rr + 1), name_len);
		path_ret[name_len] = '/';
		path_ret[name_len + 1] = '\0';
		strncat(path_ret, tmp, PATH_MAX);

		/* Get the path inside the parent subvolume */
		btrfs_item_key_to_cpu(path.nodes[0], &key, path.slots[0]);
		location.objectid = key.offset;
		location.type = BTRFS_ROOT_ITEM_KEY;
		location.offset = (u64)-1;
		root = btrfs_read_fs_root(fs_info, &location);
		if (IS_ERR(root)) {
			ret = PTR_ERR(root);
			goto out;
		}
		ret = get_path_in_subvol(root, ino, path_ret);
		if (ret < 0)
			goto out;
		cur = key.offset;
	}
	/* Add the leading '/' */
	strncpy(tmp, path_ret, PATH_MAX);
	strncpy(path_ret, "/", PATH_MAX);
	strncat(path_ret, tmp, PATH_MAX);
out:
	btrfs_release_path(&path);
	free(tmp);
	return ret;
}

static int list_subvolums(struct btrfs_fs_info *fs_info)
{
	struct btrfs_root *tree_root = fs_info->tree_root;
	struct btrfs_root *root;
	struct btrfs_path path;
	struct btrfs_key key;
	char *result;
	int ret = 0;

	result = malloc(PATH_MAX);
	if (!result)
		return -ENOMEM;

	ret = list_one_subvol(fs_info->fs_root, result);
	if (ret < 0)
		goto out;
	root = fs_info->fs_root;
	printf("ID %llu gen %llu path %.*s\n",
		root->root_key.objectid, btrfs_root_generation(&root->root_item),
		PATH_MAX, result);

	key.objectid = BTRFS_FIRST_FREE_OBJECTID;
	key.type = BTRFS_ROOT_ITEM_KEY;
	key.offset = 0;
	btrfs_init_path(&path);
	ret = btrfs_search_slot(NULL, tree_root, &key, &path, 0, 0);
	if (ret < 0)
		goto out;
	while (1) {
		if (path.slots[0] >= btrfs_header_nritems(path.nodes[0]))
			goto next;

		btrfs_item_key_to_cpu(path.nodes[0], &key, path.slots[0]);
		if (key.objectid > BTRFS_LAST_FREE_OBJECTID)
			break;
		if (key.objectid < BTRFS_FIRST_FREE_OBJECTID ||
		    key.type != BTRFS_ROOT_ITEM_KEY)
			goto next;
		key.offset = (u64)-1;
		root = btrfs_read_fs_root(fs_info, &key);
		if (IS_ERR(root)) {
			ret = PTR_ERR(root);
			if (ret == -ENOENT)
				goto next;
			goto out;
		}
		ret = list_one_subvol(root, result);
		if (ret < 0)
			goto out;
		printf("ID %llu gen %llu path %.*s\n",
			root->root_key.objectid,
			btrfs_root_generation(&root->root_item),
			PATH_MAX, result);
next:
		ret = btrfs_next_item(tree_root, &path);
		if (ret < 0)
			goto out;
		if (ret > 0) {
			ret = 0;
			break;
		}
	}
out:
	free(result);
	return ret;
}

void btrfs_list_subvols(void)
{
	struct btrfs_fs_info *fs_info = current_fs_info;
	int ret;

	if (!fs_info)
		return;
	ret = list_subvolums(fs_info);
	if (ret < 0)
		error("failed to list subvolume: %d", ret);
}
