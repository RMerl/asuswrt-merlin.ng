// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <malloc.h>

static int get_subvol_name(u64 subvolid, char *name, int max_len)
{
	struct btrfs_root_ref rref;
	struct btrfs_inode_ref iref;
	struct btrfs_root root;
	u64 dir;
	char tmp[max(BTRFS_VOL_NAME_MAX, BTRFS_NAME_MAX)];
	char *ptr;

	ptr = name + max_len - 1;
	*ptr = '\0';

	while (subvolid != BTRFS_FS_TREE_OBJECTID) {
		subvolid = btrfs_lookup_root_ref(subvolid, &rref, tmp);

		if (subvolid == -1ULL)
			return -1;

		ptr -= rref.name_len + 1;
		if (ptr < name)
			goto too_long;

		memcpy(ptr + 1, tmp, rref.name_len);
		*ptr = '/';

		if (btrfs_find_root(subvolid, &root, NULL))
			return -1;

		dir = rref.dirid;

		while (dir != BTRFS_FIRST_FREE_OBJECTID) {
			dir = btrfs_lookup_inode_ref(&root, dir, &iref, tmp);

			if (dir == -1ULL)
				return -1;

			ptr -= iref.name_len + 1;
			if (ptr < name)
				goto too_long;

			memcpy(ptr + 1, tmp, iref.name_len);
			*ptr = '/';
		}
	}

	if (ptr == name + max_len - 1) {
		name[0] = '/';
		name[1] = '\0';
	} else {
		memmove(name, ptr, name + max_len - ptr);
	}

	return 0;

too_long:
	printf("%s: subvolume name too long\n", __func__);
	return -1;
}

u64 btrfs_get_default_subvol_objectid(void)
{
	struct btrfs_dir_item item;

	if (btrfs_lookup_dir_item(&btrfs_info.tree_root,
				  btrfs_info.sb.root_dir_objectid, "default", 7,
				  &item))
		return BTRFS_FS_TREE_OBJECTID;
	return item.location.objectid;
}

static void list_subvols(u64 tree, char *nameptr, int max_name_len, int level)
{
	struct btrfs_key key, *found_key;
	struct btrfs_path path;
	struct btrfs_root_ref *ref;
	int res;

	key.objectid = tree;
	key.type = BTRFS_ROOT_REF_KEY;
	key.offset = 0;

	if (btrfs_search_tree(&btrfs_info.tree_root, &key, &path))
		return;

	do {
		found_key = btrfs_path_leaf_key(&path);
		if (btrfs_comp_keys_type(&key, found_key))
			break;

		ref = btrfs_path_item_ptr(&path, struct btrfs_root_ref);
		btrfs_root_ref_to_cpu(ref);

		printf("ID %llu parent %llu name ", found_key->offset, tree);
		if (nameptr && !get_subvol_name(found_key->offset, nameptr,
						max_name_len))
			printf("%s\n", nameptr);
		else
			printf("%.*s\n", (int) ref->name_len,
			       (const char *) (ref + 1));

		if (level > 0)
			list_subvols(found_key->offset, nameptr, max_name_len,
				     level - 1);
		else
			printf("%s: Too much recursion, maybe skipping some "
			       "subvolumes\n", __func__);
	} while (!(res = btrfs_next_slot(&path)));

	btrfs_free_path(&path);
}

void btrfs_list_subvols(void)
{
	char *nameptr = malloc(4096);

	list_subvols(BTRFS_FS_TREE_OBJECTID, nameptr, nameptr ? 4096 : 0, 40);

	if (nameptr)
		free(nameptr);
}
