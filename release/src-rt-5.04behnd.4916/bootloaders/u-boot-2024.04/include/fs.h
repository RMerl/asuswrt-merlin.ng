/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 */
#ifndef _FS_H
#define _FS_H

#include <rtc.h>

struct cmd_tbl;

#define FS_TYPE_ANY	0
#define FS_TYPE_FAT	1
#define FS_TYPE_EXT	2
#define FS_TYPE_SANDBOX	3
#define FS_TYPE_UBIFS	4
#define FS_TYPE_BTRFS	5
#define FS_TYPE_SQUASHFS 6
#define FS_TYPE_EROFS   7
#define FS_TYPE_SEMIHOSTING 8

struct blk_desc;

/**
 * do_fat_fsload - Run the fatload command
 *
 * @cmdtp: Command information for fatload
 * @flag: Command flags (CMD_FLAG_...)
 * @argc: Number of arguments
 * @argv: List of arguments
 * Return: result (see enum command_ret_t)
 */
int do_fat_fsload(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);

/**
 * do_ext2load - Run the ext2load command
 *
 * @cmdtp: Command information for ext2load
 * @flag: Command flags (CMD_FLAG_...)
 * @argc: Number of arguments
 * @argv: List of arguments
 * Return: result (see enum command_ret_t)
 */
int do_ext2load(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

/*
 * Tell the fs layer which block device and partition to use for future
 * commands. This also internally identifies the filesystem that is present
 * within the partition. The identification process may be limited to a
 * specific filesystem type by passing FS_* in the fstype parameter.
 *
 * Returns 0 on success.
 * Returns non-zero if there is an error accessing the disk or partition, or
 * no known filesystem type could be recognized on it.
 */
int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype);

/**
 * fs_set_type() - Tell fs layer which filesystem type is used
 *
 * This is needed when reading from a non-block device such as sandbox. It does
 * a similar job to fs_set_blk_dev() but just sets the filesystem type instead
 * of detecting it and loading it on the block device
 *
 * @type: Filesystem type to use (FS_TYPE...)
 */
void fs_set_type(int type);

/*
 * fs_set_blk_dev_with_part - Set current block device + partition
 *
 * Similar to fs_set_blk_dev(), but useful for cases where you already
 * know the blk_desc and part number.
 *
 * Returns 0 on success.
 * Returns non-zero if invalid partition or error accessing the disk.
 */
int fs_set_blk_dev_with_part(struct blk_desc *desc, int part);

/**
 * fs_close() - Unset current block device and partition
 *
 * fs_close() closes the connection to a file system opened with either
 * fs_set_blk_dev() or fs_set_dev_with_part().
 *
 * Many file functions implicitly call fs_close(), e.g. fs_closedir(),
 * fs_exist(), fs_ln(), fs_ls(), fs_mkdir(), fs_read(), fs_size(), fs_write(),
 * fs_unlink().
 */
void fs_close(void);

/**
 * fs_get_type() - Get type of current filesystem
 *
 * Return: filesystem type
 *
 * Returns filesystem type representing the current filesystem, or
 * FS_TYPE_ANY for any unrecognised filesystem.
 */
int fs_get_type(void);

/**
 * fs_get_type_name() - Get type of current filesystem
 *
 * Return: Pointer to filesystem name
 *
 * Returns a string describing the current filesystem, or the sentinel
 * "unsupported" for any unrecognised filesystem.
 */
const char *fs_get_type_name(void);

/*
 * Print the list of files on the partition previously set by fs_set_blk_dev(),
 * in directory "dirname".
 *
 * Returns 0 on success. Returns non-zero on error.
 */
int fs_ls(const char *dirname);

/*
 * Determine whether a file exists
 *
 * Returns 1 if the file exists, 0 if it doesn't exist.
 */
int fs_exists(const char *filename);

/*
 * fs_size - Determine a file's size
 *
 * @filename: Name of the file
 * @size: Size of file
 * Return: 0 if ok with valid *size, negative on error
 */
int fs_size(const char *filename, loff_t *size);

/**
 * fs_read() - read file from the partition previously set by fs_set_blk_dev()
 *
 * Note that not all filesystem drivers support either or both of offset != 0
 * and len != 0.
 *
 * @filename:	full path of the file to read from
 * @addr:	address of the buffer to write to
 * @offset:	offset in the file from where to start reading
 * @len:	the number of bytes to read. Use 0 to read entire file.
 * @actread:	returns the actual number of bytes read
 * Return:	0 if OK with valid *actread, -1 on error conditions
 */
int fs_read(const char *filename, ulong addr, loff_t offset, loff_t len,
	    loff_t *actread);

/**
 * fs_write() - write file to the partition previously set by fs_set_blk_dev()
 *
 * Note that not all filesystem drivers support offset != 0.
 *
 * @filename:	full path of the file to write to
 * @addr:	address of the buffer to read from
 * @offset:	offset in the file from where to start writing
 * @len:	the number of bytes to write
 * @actwrite:	returns the actual number of bytes written
 * Return:	0 if OK with valid *actwrite, -1 on error conditions
 */
int fs_write(const char *filename, ulong addr, loff_t offset, loff_t len,
	     loff_t *actwrite);

/*
 * Directory entry types, matches the subset of DT_x in posix readdir()
 * which apply to u-boot.
 */
#define FS_DT_DIR  4         /* directory */
#define FS_DT_REG  8         /* regular file */
#define FS_DT_LNK  10        /* symbolic link */

#define FS_DIRENT_NAME_LEN 256

/**
 * struct fs_dirent - directory entry
 *
 * A directory entry, returned by fs_readdir(). Returns information
 * about the file/directory at the current directory entry position.
 */
struct fs_dirent {
	/** @type:		one of FS_DT_x (not a mask) */
	unsigned int type;
	/** @size:		file size */
	loff_t size;
	/** @flags:		attribute flags (FS_ATTR_*) */
	u32 attr;
	/** create_time:	time of creation */
	struct rtc_time create_time;
	/** access_time:	time of last access */
	struct rtc_time access_time;
	/** change_time:	time of last modification */
	struct rtc_time change_time;
	/** name:		file name */
	char name[FS_DIRENT_NAME_LEN];
};

/* Note: fs_dir_stream should be treated as opaque to the user of fs layer */
struct fs_dir_stream {
	/* private to fs. layer: */
	struct blk_desc *desc;
	int part;
};

/*
 * fs_opendir - Open a directory
 *
 * @filename: the path to directory to open
 * Return: a pointer to the directory stream or NULL on error and errno
 *    set appropriately
 */
struct fs_dir_stream *fs_opendir(const char *filename);

/*
 * fs_readdir - Read the next directory entry in the directory stream.
 *
 * Works in an analogous way to posix readdir().  The previously returned
 * directory entry is no longer valid after calling fs_readdir() again.
 * After fs_closedir() is called, the returned directory entry is no
 * longer valid.
 *
 * @dirs: the directory stream
 * Return: the next directory entry (only valid until next fs_readdir() or
 *    fs_closedir() call, do not attempt to free()) or NULL if the end of
 *    the directory is reached.
 */
struct fs_dirent *fs_readdir(struct fs_dir_stream *dirs);

/*
 * fs_closedir - close a directory stream
 *
 * @dirs: the directory stream
 */
void fs_closedir(struct fs_dir_stream *dirs);

/*
 * fs_unlink - delete a file or directory
 *
 * If a given name is a directory, it will be deleted only if it's empty
 *
 * @filename: Name of file or directory to delete
 * Return: 0 on success, -1 on error conditions
 */
int fs_unlink(const char *filename);

/*
 * fs_mkdir - Create a directory
 *
 * @filename: Name of directory to create
 * Return: 0 on success, -1 on error conditions
 */
int fs_mkdir(const char *filename);

/*
 * Common implementation for various filesystem commands, optionally limited
 * to a specific filesystem type via the fstype parameter.
 */
int do_size(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	    int fstype);
int do_load(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	    int fstype);
int do_ls(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	  int fstype);
int file_exists(const char *dev_type, const char *dev_part, const char *file,
		int fstype);
int do_save(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	    int fstype);
int do_rm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	  int fstype);
int do_mkdir(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	     int fstype);
int do_ln(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	  int fstype);

/*
 * Determine the UUID of the specified filesystem and print it. Optionally it is
 * possible to store the UUID directly in env.
 */
int do_fs_uuid(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[],
	       int fstype);

/*
 * Determine the type of the specified filesystem and print it. Optionally it is
 * possible to store the type directly in env.
 */
int do_fs_type(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

/**
 * do_fs_types - List supported filesystems.
 *
 * @cmdtp: Command information for fstypes
 * @flag: Command flags (CMD_FLAG_...)
 * @argc: Number of arguments
 * @argv: List of arguments
 * Return: result (see enum command_ret_t)
 */
int do_fs_types(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);

/**
 * fs_read_alloc() - Allocate space for a file and read it
 *
 * You must call fs_set_blk_dev() or a similar function before calling this,
 * since that sets up the block device to use.
 *
 * The file is terminated with a nul character
 *
 * @fname: Filename to read
 * @size: Size of file to read (must be correct!)
 * @align: Alignment to use for memory allocation (0 for default)
 * @bufp: On success, returns the allocated buffer with the nul-terminated file
 *	in it
 * Return: 0 if OK, -ENOMEM if out of memory, -EIO if read failed
 */
int fs_read_alloc(const char *fname, ulong size, uint align, void **bufp);

/**
 * fs_load_alloc() - Load a file into allocated space
 *
 * The file is terminated with a nul character
 *
 * @ifname: Interface name to read from (e.g. "mmc")
 * @dev_part_str: Device and partition string (e.g. "1:2")
 * @fname: Filename to read
 * @max_size: Maximum allowed size for the file (use 0 for 1GB)
 * @align: Alignment to use for memory allocation (0 for default)
 * @bufp: On success, returns the allocated buffer with the nul-terminated file
 *	in it
 * @sizep: On success, returns the size of the file
 * Return: 0 if OK, -ENOMEM if out of memory, -ENOENT if the file does not
 * exist, -ENOMEDIUM if the device does not exist, -E2BIG if the file is too
 * large (greater than @max_size), -EIO if read failed
 */
int fs_load_alloc(const char *ifname, const char *dev_part_str,
		  const char *fname, ulong max_size, ulong align, void **bufp,
		  ulong *sizep);

#endif /* _FS_H */
