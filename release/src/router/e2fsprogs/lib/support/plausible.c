/*
 * plausible.c --- Figure out if a pathname is ext* or something else.
 *
 * Copyright 2014, Oracle, Inc.
 *
 * Some parts are:
 * Copyright 1995, 1996, 1997, 1998, 1999, 2000 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include "config.h"
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MAGIC_H
#include <magic.h>
#endif
#include "plausible.h"
#include "ext2fs/ext2fs.h"
#include "nls-enable.h"
#include "blkid/blkid.h"

#ifdef HAVE_MAGIC_H
static magic_t (*dl_magic_open)(int);
static const char *(*dl_magic_file)(magic_t, const char *);
static int (*dl_magic_load)(magic_t, const char *);
static void (*dl_magic_close)(magic_t);

/*
 * NO_CHECK functionality was only added in file 4.20.
 * Older systems like RHEL 5.x still have file 4.17
 */
#ifndef MAGIC_NO_CHECK_COMPRESS
#define MAGIC_NO_CHECK_COMPRESS 0x0001000
#endif
#ifndef MAGIC_NO_CHECK_ELF
#define MAGIC_NO_CHECK_ELF 0x0010000
#endif

#ifdef HAVE_DLOPEN
#include <dlfcn.h>

static void *magic_handle;

static int magic_library_available(void)
{
	if (!magic_handle) {
		magic_handle = dlopen("libmagic.so.1", RTLD_NOW);
		if (!magic_handle)
			return 0;

		dl_magic_open = (magic_t (*)(int))
			dlsym(magic_handle, "magic_open");
		dl_magic_file = (const char *(*)(magic_t, const char *))
			dlsym(magic_handle, "magic_file");
		dl_magic_load = (int (*)(magic_t, const char *))
			dlsym(magic_handle, "magic_load");
		dl_magic_close = (void (*)(magic_t))
			dlsym(magic_handle, "magic_close");
	}

	if (!dl_magic_open || !dl_magic_file ||
	    !dl_magic_load || !dl_magic_close)
		return 0;
	return 1;
}
#else
static int magic_library_available(void)
{
	dl_magic_open = magic_open;
	dl_magic_file = magic_file;
	dl_magic_load = magic_load;
	dl_magic_close = magic_close;

	return 1;
}
#endif
#endif

static void print_ext2_info(const char *device)

{
	struct ext2_super_block	*sb;
	ext2_filsys		fs;
	errcode_t		retval;
	time_t			tm;

	retval = ext2fs_open2(device, 0, EXT2_FLAG_64BITS, 0, 0,
			      unix_io_manager, &fs);
	if (retval)
		return;
	sb = fs->super;

	if (sb->s_mtime) {
		tm = sb->s_mtime;
		if (sb->s_last_mounted[0])
			printf(_("\tlast mounted on %.*s on %s"),
			       EXT2_LEN_STR(sb->s_last_mounted), ctime(&tm));
		else
			printf(_("\tlast mounted on %s"), ctime(&tm));
	} else if (sb->s_mkfs_time) {
		tm = sb->s_mkfs_time;
		printf(_("\tcreated on %s"), ctime(&tm));
	} else if (sb->s_wtime) {
		tm = sb->s_wtime;
		printf(_("\tlast modified on %s"), ctime(&tm));
	}
	ext2fs_close_free(&fs);
}

/*
 * return 1 if there is no partition table, 0 if a partition table is
 * detected, and -1 on an error.
 */
#ifdef HAVE_BLKID_PROBE_ENABLE_PARTITIONS
static int check_partition_table(const char *device)
{
	blkid_probe pr;
	const char *value;
	int ret;

	pr = blkid_new_probe_from_filename(device);
	if (!pr)
		return -1;

	ret = blkid_probe_enable_partitions(pr, 1);
	if (ret < 0)
		goto errout;

	ret = blkid_probe_enable_superblocks(pr, 0);
	if (ret < 0)
		goto errout;

	ret = blkid_do_fullprobe(pr);
	if (ret < 0)
		goto errout;

	ret = blkid_probe_lookup_value(pr, "PTTYPE", &value, NULL);
	if (ret == 0)
		fprintf(stderr, _("Found a %s partition table in %s\n"),
			value, device);
	else
		ret = 1;

errout:
	blkid_free_probe(pr);
	return ret;
}
#else
static int check_partition_table(const char *device EXT2FS_ATTR((unused)))
{
	return -1;
}
#endif

/*
 * return 1 if the device looks plausible, creating the file if necessary
 */
int check_plausibility(const char *device, int flags, int *ret_is_dev)
{
	int fd, ret, is_dev = 0;
	ext2fs_struct_stat s;
	int fl = O_RDONLY;
	blkid_cache cache = NULL;
	char *fs_type = NULL;
	char *fs_label = NULL;

	fd = ext2fs_open_file(device, fl, 0666);
	if ((fd < 0) && (errno == ENOENT) && (flags & NO_SIZE)) {
		fprintf(stderr, _("The file %s does not exist and no "
				  "size was specified.\n"), device);
		exit(1);
	}
	if ((fd < 0) && (errno == ENOENT) && (flags & CREATE_FILE)) {
		fl |= O_CREAT;
		fd = ext2fs_open_file(device, fl, 0666);
		if (fd >= 0 && (flags & VERBOSE_CREATE))
			printf(_("Creating regular file %s\n"), device);
	}
	if (fd < 0) {
		fprintf(stderr, _("Could not open %s: %s\n"),
			device, error_message(errno));
		if (errno == ENOENT)
			fputs(_("\nThe device apparently does not exist; "
				"did you specify it correctly?\n"), stderr);
		exit(1);
	}

	if (ext2fs_fstat(fd, &s) < 0) {
		perror("stat");
		exit(1);
	}
	close(fd);

	if (S_ISBLK(s.st_mode))
		is_dev = 1;
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
	/* On FreeBSD, all disk devices are character specials */
	if (S_ISCHR(s.st_mode))
		is_dev = 1;
#endif
	if (ret_is_dev)
		*ret_is_dev = is_dev;

	if ((flags & CHECK_BLOCK_DEV) && !is_dev) {
		printf(_("%s is not a block special device.\n"), device);
		return 0;
	}

	/*
	 * Note: we use the older-style blkid API's here because we
	 * want as much functionality to be available when using the
	 * internal blkid library, when e2fsprogs is compiled for
	 * non-Linux systems that will probably not have the libraries
	 * from util-linux available.  We only use the newer
	 * blkid-probe interfaces to access functionality not
	 * available in the original blkid library.
	 */
	if ((flags & CHECK_FS_EXIST) && blkid_get_cache(&cache, NULL) >= 0) {
		fs_type = blkid_get_tag_value(cache, "TYPE", device);
		if (fs_type)
			fs_label = blkid_get_tag_value(cache, "LABEL", device);
		blkid_put_cache(cache);
	}

	if (fs_type) {
		if (fs_label)
			printf(_("%s contains a %s file system labelled '%s'\n"),
			       device, fs_type, fs_label);
		else
			printf(_("%s contains a %s file system\n"), device,
			       fs_type);
		if (strncmp(fs_type, "ext", 3) == 0)
			print_ext2_info(device);
		free(fs_type);
		free(fs_label);
		return 0;
	}

#ifdef HAVE_MAGIC_H
	if ((flags & CHECK_FS_EXIST) &&
	    !getenv("E2FSPROGS_LIBMAGIC_SUPPRESS") &&
	    magic_library_available()) {
		const char *msg;
		magic_t mag;
		int has_magic = 0;

		mag = dl_magic_open(MAGIC_RAW | MAGIC_SYMLINK | MAGIC_DEVICES |
				    MAGIC_ERROR | MAGIC_NO_CHECK_ELF |
				    MAGIC_NO_CHECK_COMPRESS);
		dl_magic_load(mag, NULL);

		msg = dl_magic_file(mag, device);
		if (msg && strcmp(msg, "data") && strcmp(msg, "empty")) {
			printf(_("%s contains `%s' data\n"), device, msg);
			has_magic = 1;
		}

		dl_magic_close(mag);
		return !has_magic;
	}
#endif

	ret = check_partition_table(device);
	if (ret >= 0)
		return ret;

	return 1;
}

