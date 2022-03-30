// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <config.h>
#include <malloc.h>
#include <linux/time.h>

struct btrfs_info btrfs_info;

static int readdir_callback(const struct btrfs_root *root,
			    struct btrfs_dir_item *item)
{
	static const char typestr[BTRFS_FT_MAX][4] = {
		[BTRFS_FT_UNKNOWN]  = " ? ",
		[BTRFS_FT_REG_FILE] = "   ",
		[BTRFS_FT_DIR]      = "DIR",
		[BTRFS_FT_CHRDEV]   = "CHR",
		[BTRFS_FT_BLKDEV]   = "BLK",
		[BTRFS_FT_FIFO]     = "FIF",
		[BTRFS_FT_SOCK]     = "SCK",
		[BTRFS_FT_SYMLINK]  = "SYM",
		[BTRFS_FT_XATTR]    = " ? ",
	};
	struct btrfs_inode_item inode;
	const char *name = (const char *) (item + 1);
	char filetime[32], *target = NULL;
	time_t mtime;

	if (btrfs_lookup_inode(root, &item->location, &inode, NULL)) {
		printf("%s: Cannot find inode item for directory entry %.*s!\n",
		       __func__, item->name_len, name);
		return 0;
	}

	mtime = inode.mtime.sec;
	ctime_r(&mtime, filetime);

	if (item->type == BTRFS_FT_SYMLINK) {
		target = malloc(min(inode.size + 1,
				    (u64) btrfs_info.sb.sectorsize));

		if (target && btrfs_readlink(root, item->location.objectid,
					     target)) {
			free(target);
			target = NULL;
		}

		if (!target)
			printf("%s: Cannot read symlink target!\n", __func__);
	}

	printf("<%s> ", typestr[item->type]);
	if (item->type == BTRFS_FT_CHRDEV || item->type == BTRFS_FT_BLKDEV)
		printf("%4u,%5u  ", (unsigned int) (inode.rdev >> 20),
			(unsigned int) (inode.rdev & 0xfffff));
	else
		printf("%10llu  ", inode.size);

	printf("%24.24s  %.*s", filetime, item->name_len, name);

	if (item->type == BTRFS_FT_SYMLINK) {
		printf(" -> %s", target ? target : "?");
		if (target)
			free(target);
	}

	printf("\n");

	return 0;
}

int btrfs_probe(struct blk_desc *fs_dev_desc, disk_partition_t *fs_partition)
{
	btrfs_blk_desc = fs_dev_desc;
	btrfs_part_info = fs_partition;

	memset(&btrfs_info, 0, sizeof(btrfs_info));

	btrfs_hash_init();
	if (btrfs_read_superblock())
		return -1;

	if (btrfs_chunk_map_init()) {
		printf("%s: failed to init chunk map\n", __func__);
		return -1;
	}

	btrfs_info.tree_root.objectid = 0;
	btrfs_info.tree_root.bytenr = btrfs_info.sb.root;
	btrfs_info.chunk_root.objectid = 0;
	btrfs_info.chunk_root.bytenr = btrfs_info.sb.chunk_root;

	if (btrfs_read_chunk_tree()) {
		printf("%s: failed to read chunk tree\n", __func__);
		return -1;
	}

	if (btrfs_find_root(btrfs_get_default_subvol_objectid(),
			    &btrfs_info.fs_root, NULL)) {
		printf("%s: failed to find default subvolume\n", __func__);
		return -1;
	}

	return 0;
}

int btrfs_ls(const char *path)
{
	struct btrfs_root root = btrfs_info.fs_root;
	u64 inr;
	u8 type;

	inr = btrfs_lookup_path(&root, root.root_dirid, path, &type, NULL, 40);

	if (inr == -1ULL) {
		printf("Cannot lookup path %s\n", path);
		return -1;
	}

	if (type != BTRFS_FT_DIR) {
		printf("Not a directory: %s\n", path);
		return -1;
	}

	if (btrfs_readdir(&root, inr, readdir_callback)) {
		printf("An error occured while listing directory %s\n", path);
		return -1;
	}

	return 0;
}

int btrfs_exists(const char *file)
{
	struct btrfs_root root = btrfs_info.fs_root;
	u64 inr;
	u8 type;

	inr = btrfs_lookup_path(&root, root.root_dirid, file, &type, NULL, 40);

	return (inr != -1ULL && type == BTRFS_FT_REG_FILE);
}

int btrfs_size(const char *file, loff_t *size)
{
	struct btrfs_root root = btrfs_info.fs_root;
	struct btrfs_inode_item inode;
	u64 inr;
	u8 type;

	inr = btrfs_lookup_path(&root, root.root_dirid, file, &type, &inode,
				40);

	if (inr == -1ULL) {
		printf("Cannot lookup file %s\n", file);
		return -1;
	}

	if (type != BTRFS_FT_REG_FILE) {
		printf("Not a regular file: %s\n", file);
		return -1;
	}

	*size = inode.size;
	return 0;
}

int btrfs_read(const char *file, void *buf, loff_t offset, loff_t len,
	       loff_t *actread)
{
	struct btrfs_root root = btrfs_info.fs_root;
	struct btrfs_inode_item inode;
	u64 inr, rd;
	u8 type;

	inr = btrfs_lookup_path(&root, root.root_dirid, file, &type, &inode,
				40);

	if (inr == -1ULL) {
		printf("Cannot lookup file %s\n", file);
		return -1;
	}

	if (type != BTRFS_FT_REG_FILE) {
		printf("Not a regular file: %s\n", file);
		return -1;
	}

	if (!len)
		len = inode.size;

	if (len > inode.size - offset)
		len = inode.size - offset;

	rd = btrfs_file_read(&root, inr, offset, len, buf);
	if (rd == -1ULL) {
		printf("An error occured while reading file %s\n", file);
		return -1;
	}

	*actread = rd;
	return 0;
}

void btrfs_close(void)
{
	btrfs_chunk_map_exit();
}

int btrfs_uuid(char *uuid_str)
{
#ifdef CONFIG_LIB_UUID
	uuid_bin_to_str(btrfs_info.sb.fsid, uuid_str, UUID_STR_FORMAT_STD);
	return 0;
#endif
	return -ENOSYS;
}
