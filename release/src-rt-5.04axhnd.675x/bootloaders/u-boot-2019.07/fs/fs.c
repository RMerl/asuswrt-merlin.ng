// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 */

#include <config.h>
#include <errno.h>
#include <common.h>
#include <mapmem.h>
#include <part.h>
#include <ext4fs.h>
#include <fat.h>
#include <fs.h>
#include <sandboxfs.h>
#include <ubifs_uboot.h>
#include <btrfs.h>
#include <asm/io.h>
#include <div64.h>
#include <linux/math64.h>
#include <efi_loader.h>

DECLARE_GLOBAL_DATA_PTR;

static struct blk_desc *fs_dev_desc;
static int fs_dev_part;
static disk_partition_t fs_partition;
static int fs_type = FS_TYPE_ANY;

static inline int fs_probe_unsupported(struct blk_desc *fs_dev_desc,
				      disk_partition_t *fs_partition)
{
	printf("** Unrecognized filesystem type **\n");
	return -1;
}

static inline int fs_ls_unsupported(const char *dirname)
{
	return -1;
}

/* generic implementation of ls in terms of opendir/readdir/closedir */
__maybe_unused
static int fs_ls_generic(const char *dirname)
{
	struct fs_dir_stream *dirs;
	struct fs_dirent *dent;
	int nfiles = 0, ndirs = 0;

	dirs = fs_opendir(dirname);
	if (!dirs)
		return -errno;

	while ((dent = fs_readdir(dirs))) {
		if (dent->type == FS_DT_DIR) {
			printf("            %s/\n", dent->name);
			ndirs++;
		} else {
			printf(" %8lld   %s\n", dent->size, dent->name);
			nfiles++;
		}
	}

	fs_closedir(dirs);

	printf("\n%d file(s), %d dir(s)\n\n", nfiles, ndirs);

	return 0;
}

static inline int fs_exists_unsupported(const char *filename)
{
	return 0;
}

static inline int fs_size_unsupported(const char *filename, loff_t *size)
{
	return -1;
}

static inline int fs_read_unsupported(const char *filename, void *buf,
				      loff_t offset, loff_t len,
				      loff_t *actread)
{
	return -1;
}

static inline int fs_write_unsupported(const char *filename, void *buf,
				      loff_t offset, loff_t len,
				      loff_t *actwrite)
{
	return -1;
}

static inline int fs_ln_unsupported(const char *filename, const char *target)
{
	return -1;
}

static inline void fs_close_unsupported(void)
{
}

static inline int fs_uuid_unsupported(char *uuid_str)
{
	return -1;
}

static inline int fs_opendir_unsupported(const char *filename,
					 struct fs_dir_stream **dirs)
{
	return -EACCES;
}

static inline int fs_unlink_unsupported(const char *filename)
{
	return -1;
}

static inline int fs_mkdir_unsupported(const char *dirname)
{
	return -1;
}

struct fstype_info {
	int fstype;
	char *name;
	/*
	 * Is it legal to pass NULL as .probe()'s  fs_dev_desc parameter? This
	 * should be false in most cases. For "virtual" filesystems which
	 * aren't based on a U-Boot block device (e.g. sandbox), this can be
	 * set to true. This should also be true for the dummy entry at the end
	 * of fstypes[], since that is essentially a "virtual" (non-existent)
	 * filesystem.
	 */
	bool null_dev_desc_ok;
	int (*probe)(struct blk_desc *fs_dev_desc,
		     disk_partition_t *fs_partition);
	int (*ls)(const char *dirname);
	int (*exists)(const char *filename);
	int (*size)(const char *filename, loff_t *size);
	int (*read)(const char *filename, void *buf, loff_t offset,
		    loff_t len, loff_t *actread);
	int (*write)(const char *filename, void *buf, loff_t offset,
		     loff_t len, loff_t *actwrite);
	void (*close)(void);
	int (*uuid)(char *uuid_str);
	/*
	 * Open a directory stream.  On success return 0 and directory
	 * stream pointer via 'dirsp'.  On error, return -errno.  See
	 * fs_opendir().
	 */
	int (*opendir)(const char *filename, struct fs_dir_stream **dirsp);
	/*
	 * Read next entry from directory stream.  On success return 0
	 * and directory entry pointer via 'dentp'.  On error return
	 * -errno.  See fs_readdir().
	 */
	int (*readdir)(struct fs_dir_stream *dirs, struct fs_dirent **dentp);
	/* see fs_closedir() */
	void (*closedir)(struct fs_dir_stream *dirs);
	int (*unlink)(const char *filename);
	int (*mkdir)(const char *dirname);
	int (*ln)(const char *filename, const char *target);
};

static struct fstype_info fstypes[] = {
#ifdef CONFIG_FS_FAT
	{
		.fstype = FS_TYPE_FAT,
		.name = "fat",
		.null_dev_desc_ok = false,
		.probe = fat_set_blk_dev,
		.close = fat_close,
		.ls = fs_ls_generic,
		.exists = fat_exists,
		.size = fat_size,
		.read = fat_read_file,
#if CONFIG_IS_ENABLED(FAT_WRITE)
		.write = file_fat_write,
		.unlink = fat_unlink,
		.mkdir = fat_mkdir,
#else
		.write = fs_write_unsupported,
		.unlink = fs_unlink_unsupported,
		.mkdir = fs_mkdir_unsupported,
#endif
		.uuid = fs_uuid_unsupported,
		.opendir = fat_opendir,
		.readdir = fat_readdir,
		.closedir = fat_closedir,
		.ln = fs_ln_unsupported,
	},
#endif

#if CONFIG_IS_ENABLED(FS_EXT4)
	{
		.fstype = FS_TYPE_EXT,
		.name = "ext4",
		.null_dev_desc_ok = false,
		.probe = ext4fs_probe,
		.close = ext4fs_close,
		.ls = ext4fs_ls,
		.exists = ext4fs_exists,
		.size = ext4fs_size,
		.read = ext4_read_file,
#ifdef CONFIG_CMD_EXT4_WRITE
		.write = ext4_write_file,
		.ln = ext4fs_create_link,
#else
		.write = fs_write_unsupported,
		.ln = fs_ln_unsupported,
#endif
		.uuid = ext4fs_uuid,
		.opendir = fs_opendir_unsupported,
		.unlink = fs_unlink_unsupported,
		.mkdir = fs_mkdir_unsupported,
	},
#endif
#ifdef CONFIG_SANDBOX
	{
		.fstype = FS_TYPE_SANDBOX,
		.name = "sandbox",
		.null_dev_desc_ok = true,
		.probe = sandbox_fs_set_blk_dev,
		.close = sandbox_fs_close,
		.ls = sandbox_fs_ls,
		.exists = sandbox_fs_exists,
		.size = sandbox_fs_size,
		.read = fs_read_sandbox,
		.write = fs_write_sandbox,
		.uuid = fs_uuid_unsupported,
		.opendir = fs_opendir_unsupported,
		.unlink = fs_unlink_unsupported,
		.mkdir = fs_mkdir_unsupported,
		.ln = fs_ln_unsupported,
	},
#endif
#ifdef CONFIG_CMD_UBIFS
	{
		.fstype = FS_TYPE_UBIFS,
		.name = "ubifs",
		.null_dev_desc_ok = true,
		.probe = ubifs_set_blk_dev,
		.close = ubifs_close,
		.ls = ubifs_ls,
		.exists = ubifs_exists,
		.size = ubifs_size,
		.read = ubifs_read,
		.write = fs_write_unsupported,
		.uuid = fs_uuid_unsupported,
		.opendir = fs_opendir_unsupported,
		.unlink = fs_unlink_unsupported,
		.mkdir = fs_mkdir_unsupported,
		.ln = fs_ln_unsupported,
	},
#endif
#ifdef CONFIG_FS_BTRFS
	{
		.fstype = FS_TYPE_BTRFS,
		.name = "btrfs",
		.null_dev_desc_ok = false,
		.probe = btrfs_probe,
		.close = btrfs_close,
		.ls = btrfs_ls,
		.exists = btrfs_exists,
		.size = btrfs_size,
		.read = btrfs_read,
		.write = fs_write_unsupported,
		.uuid = btrfs_uuid,
		.opendir = fs_opendir_unsupported,
		.unlink = fs_unlink_unsupported,
		.mkdir = fs_mkdir_unsupported,
		.ln = fs_ln_unsupported,
	},
#endif
	{
		.fstype = FS_TYPE_ANY,
		.name = "unsupported",
		.null_dev_desc_ok = true,
		.probe = fs_probe_unsupported,
		.close = fs_close_unsupported,
		.ls = fs_ls_unsupported,
		.exists = fs_exists_unsupported,
		.size = fs_size_unsupported,
		.read = fs_read_unsupported,
		.write = fs_write_unsupported,
		.uuid = fs_uuid_unsupported,
		.opendir = fs_opendir_unsupported,
		.unlink = fs_unlink_unsupported,
		.mkdir = fs_mkdir_unsupported,
		.ln = fs_ln_unsupported,
	},
};

static struct fstype_info *fs_get_info(int fstype)
{
	struct fstype_info *info;
	int i;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes) - 1; i++, info++) {
		if (fstype == info->fstype)
			return info;
	}

	/* Return the 'unsupported' sentinel */
	return info;
}

/**
 * fs_get_type_name() - Get type of current filesystem
 *
 * Return: Pointer to filesystem name
 *
 * Returns a string describing the current filesystem, or the sentinel
 * "unsupported" for any unrecognised filesystem.
 */
const char *fs_get_type_name(void)
{
	return fs_get_info(fs_type)->name;
}

int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype)
{
	struct fstype_info *info;
	int part, i;
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	static int relocated;

	if (!relocated) {
		for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes);
				i++, info++) {
			info->name += gd->reloc_off;
			info->probe += gd->reloc_off;
			info->close += gd->reloc_off;
			info->ls += gd->reloc_off;
			info->read += gd->reloc_off;
			info->write += gd->reloc_off;
		}
		relocated = 1;
	}
#endif

	part = blk_get_device_part_str(ifname, dev_part_str, &fs_dev_desc,
					&fs_partition, 1);
	if (part < 0)
		return -1;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes); i++, info++) {
		if (fstype != FS_TYPE_ANY && info->fstype != FS_TYPE_ANY &&
				fstype != info->fstype)
			continue;

		if (!fs_dev_desc && !info->null_dev_desc_ok)
			continue;

		if (!info->probe(fs_dev_desc, &fs_partition)) {
			fs_type = info->fstype;
			fs_dev_part = part;
			return 0;
		}
	}

	return -1;
}

/* set current blk device w/ blk_desc + partition # */
int fs_set_blk_dev_with_part(struct blk_desc *desc, int part)
{
	struct fstype_info *info;
	int ret, i;

	if (part >= 1)
		ret = part_get_info(desc, part, &fs_partition);
	else
		ret = part_get_info_whole_disk(desc, &fs_partition);
	if (ret)
		return ret;
	fs_dev_desc = desc;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes); i++, info++) {
		if (!info->probe(fs_dev_desc, &fs_partition)) {
			fs_type = info->fstype;
			fs_dev_part = part;
			return 0;
		}
	}

	return -1;
}

static void fs_close(void)
{
	struct fstype_info *info = fs_get_info(fs_type);

	info->close();

	fs_type = FS_TYPE_ANY;
}

int fs_uuid(char *uuid_str)
{
	struct fstype_info *info = fs_get_info(fs_type);

	return info->uuid(uuid_str);
}

int fs_ls(const char *dirname)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->ls(dirname);

	fs_type = FS_TYPE_ANY;
	fs_close();

	return ret;
}

int fs_exists(const char *filename)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->exists(filename);

	fs_close();

	return ret;
}

int fs_size(const char *filename, loff_t *size)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->size(filename, size);

	fs_close();

	return ret;
}

#ifdef CONFIG_LMB
/* Check if a file may be read to the given address */
static int fs_read_lmb_check(const char *filename, ulong addr, loff_t offset,
			     loff_t len, struct fstype_info *info)
{
	struct lmb lmb;
	int ret;
	loff_t size;
	loff_t read_len;

	/* get the actual size of the file */
	ret = info->size(filename, &size);
	if (ret)
		return ret;
	if (offset >= size) {
		/* offset >= EOF, no bytes will be written */
		return 0;
	}
	read_len = size - offset;

	/* limit to 'len' if it is smaller */
	if (len && len < read_len)
		read_len = len;

	lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);
	lmb_dump_all(&lmb);

	if (lmb_alloc_addr(&lmb, addr, read_len) == addr)
		return 0;

	printf("** Reading file would overwrite reserved memory **\n");
	return -ENOSPC;
}
#endif

static int _fs_read(const char *filename, ulong addr, loff_t offset, loff_t len,
		    int do_lmb_check, loff_t *actread)
{
	struct fstype_info *info = fs_get_info(fs_type);
	void *buf;
	int ret;

#ifdef CONFIG_LMB
	if (do_lmb_check) {
		ret = fs_read_lmb_check(filename, addr, offset, len, info);
		if (ret)
			return ret;
	}
#endif

	/*
	 * We don't actually know how many bytes are being read, since len==0
	 * means read the whole file.
	 */
	buf = map_sysmem(addr, len);
	ret = info->read(filename, buf, offset, len, actread);
	unmap_sysmem(buf);

	/* If we requested a specific number of bytes, check we got it */
	if (ret == 0 && len && *actread != len)
		debug("** %s shorter than offset + len **\n", filename);
	fs_close();

	return ret;
}

int fs_read(const char *filename, ulong addr, loff_t offset, loff_t len,
	    loff_t *actread)
{
	return _fs_read(filename, addr, offset, len, 0, actread);
}

int fs_write(const char *filename, ulong addr, loff_t offset, loff_t len,
	     loff_t *actwrite)
{
	struct fstype_info *info = fs_get_info(fs_type);
	void *buf;
	int ret;

	buf = map_sysmem(addr, len);
	ret = info->write(filename, buf, offset, len, actwrite);
	unmap_sysmem(buf);

	if (ret < 0 && len != *actwrite) {
		printf("** Unable to write file %s **\n", filename);
		ret = -1;
	}
	fs_close();

	return ret;
}

struct fs_dir_stream *fs_opendir(const char *filename)
{
	struct fstype_info *info = fs_get_info(fs_type);
	struct fs_dir_stream *dirs = NULL;
	int ret;

	ret = info->opendir(filename, &dirs);
	fs_close();
	if (ret) {
		errno = -ret;
		return NULL;
	}

	dirs->desc = fs_dev_desc;
	dirs->part = fs_dev_part;

	return dirs;
}

struct fs_dirent *fs_readdir(struct fs_dir_stream *dirs)
{
	struct fstype_info *info;
	struct fs_dirent *dirent;
	int ret;

	fs_set_blk_dev_with_part(dirs->desc, dirs->part);
	info = fs_get_info(fs_type);

	ret = info->readdir(dirs, &dirent);
	fs_close();
	if (ret) {
		errno = -ret;
		return NULL;
	}

	return dirent;
}

void fs_closedir(struct fs_dir_stream *dirs)
{
	struct fstype_info *info;

	if (!dirs)
		return;

	fs_set_blk_dev_with_part(dirs->desc, dirs->part);
	info = fs_get_info(fs_type);

	info->closedir(dirs);
	fs_close();
}

int fs_unlink(const char *filename)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->unlink(filename);

	fs_type = FS_TYPE_ANY;
	fs_close();

	return ret;
}

int fs_mkdir(const char *dirname)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->mkdir(dirname);

	fs_type = FS_TYPE_ANY;
	fs_close();

	return ret;
}

int fs_ln(const char *fname, const char *target)
{
	struct fstype_info *info = fs_get_info(fs_type);
	int ret;

	ret = info->ln(fname, target);

	if (ret < 0) {
		printf("** Unable to create link %s -> %s **\n", fname, target);
		ret = -1;
	}
	fs_close();

	return ret;
}

int do_size(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype)
{
	loff_t size;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], fstype))
		return 1;

	if (fs_size(argv[3], &size) < 0)
		return CMD_RET_FAILURE;

	env_set_hex("filesize", size);

	return 0;
}

int do_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype)
{
	unsigned long addr;
	const char *addr_str;
	const char *filename;
	loff_t bytes;
	loff_t pos;
	loff_t len_read;
	int ret;
	unsigned long time;
	char *ep;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 7)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], (argc >= 3) ? argv[2] : NULL, fstype))
		return 1;

	if (argc >= 4) {
		addr = simple_strtoul(argv[3], &ep, 16);
		if (ep == argv[3] || *ep != '\0')
			return CMD_RET_USAGE;
	} else {
		addr_str = env_get("loadaddr");
		if (addr_str != NULL)
			addr = simple_strtoul(addr_str, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;
	}
	if (argc >= 5) {
		filename = argv[4];
	} else {
		filename = env_get("bootfile");
		if (!filename) {
			puts("** No boot file defined **\n");
			return 1;
		}
	}
	if (argc >= 6)
		bytes = simple_strtoul(argv[5], NULL, 16);
	else
		bytes = 0;
	if (argc >= 7)
		pos = simple_strtoul(argv[6], NULL, 16);
	else
		pos = 0;

#ifdef CONFIG_CMD_BOOTEFI
	efi_set_bootdev(argv[1], (argc > 2) ? argv[2] : "",
			(argc > 4) ? argv[4] : "");
#endif
	time = get_timer(0);
	ret = _fs_read(filename, addr, pos, bytes, 1, &len_read);
	time = get_timer(time);
	if (ret < 0)
		return 1;

	printf("%llu bytes read in %lu ms", len_read, time);
	if (time > 0) {
		puts(" (");
		print_size(div_u64(len_read, time) * 1000, "/s");
		puts(")");
	}
	puts("\n");

	env_set_hex("fileaddr", addr);
	env_set_hex("filesize", len_read);

	return 0;
}

int do_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
	int fstype)
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], (argc >= 3) ? argv[2] : NULL, fstype))
		return 1;

	if (fs_ls(argc >= 4 ? argv[3] : "/"))
		return 1;

	return 0;
}

int file_exists(const char *dev_type, const char *dev_part, const char *file,
		int fstype)
{
	if (fs_set_blk_dev(dev_type, dev_part, fstype))
		return 0;

	return fs_exists(file);
}

int do_save(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype)
{
	unsigned long addr;
	const char *filename;
	loff_t bytes;
	loff_t pos;
	loff_t len;
	int ret;
	unsigned long time;

	if (argc < 6 || argc > 7)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], fstype))
		return 1;

	addr = simple_strtoul(argv[3], NULL, 16);
	filename = argv[4];
	bytes = simple_strtoul(argv[5], NULL, 16);
	if (argc >= 7)
		pos = simple_strtoul(argv[6], NULL, 16);
	else
		pos = 0;

	time = get_timer(0);
	ret = fs_write(filename, addr, pos, bytes, &len);
	time = get_timer(time);
	if (ret < 0)
		return 1;

	printf("%llu bytes written in %lu ms", len, time);
	if (time > 0) {
		puts(" (");
		print_size(div_u64(len, time) * 1000, "/s");
		puts(")");
	}
	puts("\n");

	return 0;
}

int do_fs_uuid(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype)
{
	int ret;
	char uuid[37];
	memset(uuid, 0, sizeof(uuid));

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], fstype))
		return 1;

	ret = fs_uuid(uuid);
	if (ret)
		return CMD_RET_FAILURE;

	if (argc == 4)
		env_set(argv[3], uuid);
	else
		printf("%s\n", uuid);

	return CMD_RET_SUCCESS;
}

int do_fs_type(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct fstype_info *info;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], FS_TYPE_ANY))
		return 1;

	info = fs_get_info(fs_type);

	if (argc == 4)
		env_set(argv[3], info->name);
	else
		printf("%s\n", info->name);

	fs_close();

	return CMD_RET_SUCCESS;
}

int do_rm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
	  int fstype)
{
	if (argc != 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], fstype))
		return 1;

	if (fs_unlink(argv[3]))
		return 1;

	return 0;
}

int do_mkdir(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
	     int fstype)
{
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], fstype))
		return 1;

	ret = fs_mkdir(argv[3]);
	if (ret) {
		printf("** Unable to create a directory \"%s\" **\n", argv[3]);
		return 1;
	}

	return 0;
}

int do_ln(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
	  int fstype)
{
	if (argc != 5)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], fstype))
		return 1;

	if (fs_ln(argv[3], argv[4]))
		return 1;

	return 0;
}
