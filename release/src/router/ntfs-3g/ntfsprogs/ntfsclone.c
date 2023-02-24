/**
 * ntfsclone - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2003-2006 Szabolcs Szakacsits
 * Copyright (c) 2004-2006 Anton Altaparmakov
 * Copyright (c) 2010-2018 Jean-Pierre Andre
 * Special image format support copyright (c) 2004 Per Olofsson
 *
 * Clone NTFS data and/or metadata to a sparse file, image, device or stdout.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

/*
 * FIXME: ntfsclone do bad things about endians handling. Fix it and remove
 * this note and define.
 */
#define NTFS_DO_NOT_CHECK_ENDIANS

#include "param.h"
#include "debug.h"
#include "types.h"
#include "support.h"
#include "endians.h"
#include "bootsect.h"
#include "device.h"
#include "attrib.h"
#include "mst.h"
#include "volume.h"
#include "mft.h"
#include "bitmap.h"
#include "inode.h"
#include "index.h"
#include "dir.h"
#include "runlist.h"
#include "ntfstime.h"
#include "utils.h"
/* #include "version.h" */
#include "misc.h"

#if defined(linux) && defined(_IO) && !defined(BLKGETSIZE)
#define BLKGETSIZE	_IO(0x12,96)  /* Get device size in 512-byte blocks. */
#endif
#if defined(linux) && defined(_IOR) && !defined(BLKGETSIZE64)
#define BLKGETSIZE64	_IOR(0x12,114,size_t)	/* Get device size in bytes. */
#endif

#if defined(linux) || defined(__uClinux__) || defined(__sun) \
		|| defined(__APPLE__) || defined(__DARWIN__)
  /* Make sure the presence of <windows.h> means compiling for Windows */
#undef HAVE_WINDOWS_H
#endif

#if defined(__sun) | defined(HAVE_WINDOWS_H)
#define NO_STATFS 1	/* statfs(2) and f_type are not universal */
#endif

#ifdef HAVE_WINDOWS_H
/*
 *		Replacements for functions which do not exist on Windows
 */
int setmode(int, int); /* from msvcrt.dll */

#define getpid() (0)
#define srandom(seed) srand(seed)
#define random() rand()
#define fsync(fd) (0)
#define ioctl(fd,code,buf) (-1)
#define ftruncate(fd, size) ntfs_device_win32_ftruncate(dev_out, size)
#define BINWMODE "wb"
#else
#define BINWMODE "w"
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

static const char *EXEC_NAME = "ntfsclone";

static const char *bad_sectors_warning_msg =
"*************************************************************************\n"
"* WARNING: The disk has one or more bad sectors. This means that damage *\n"
"* has occurred on the disk surface, possibly caused by deterioration of *\n"
"* the physical media, manufacturing faults or other reasons. The        *\n"
"* reliability of the disk may stay stable or degrade fast.              *\n"
"* Use the --rescue option to efficiently save as much data as possible! *\n"
"*************************************************************************\n";

static const char *dirty_volume_msg =
"Volume '%s' is scheduled for a check or it was shutdown \n"
"uncleanly. Please boot Windows or use the --force option to progress.\n";

static struct {
	int verbose;
	int quiet;
	int debug;
	int force;
	int overwrite;
	int std_out;
	int blkdev_out;		/* output file is block device */
	int metadata;		/* metadata only cloning */
	int no_action;		/* do not really restore */
	int ignore_fs_check;
	int rescue;
	int save_image;
	int new_serial;
	int metadata_image;
	int preserve_timestamps;
	int full_logfile;
	int restore_image;
	char *output;
	char *volume;
#ifndef NO_STATFS
	struct statfs stfs;
#endif
} opt;

struct bitmap {
	s64 size;
	u8 *bm;
};

struct progress_bar {
	u64 start;
	u64 stop;
	int resolution;
	float unit;
};

typedef struct {
	ntfs_inode *ni;			/* inode being processed */
	ntfs_attr_search_ctx *ctx;	/* inode attribute being processed */
	s64 inuse;			/* number of clusters in use */
	int more_use;			/* possibly allocated clusters */
	LCN current_lcn;
} ntfs_walk_clusters_ctx;

typedef int (ntfs_walk_op)(ntfs_inode *ni, void *data);

struct ntfs_walk_cluster {
	ntfs_walk_op *inode_op;		/* not implemented yet */
	ntfs_walk_clusters_ctx *image;
};


static ntfs_volume *vol = NULL;
static struct bitmap lcn_bitmap;

static int fd_in;
static int fd_out;
static FILE *stream_out = (FILE*)NULL;
struct ntfs_device *dev_out = (struct ntfs_device*)NULL;
static FILE *msg_out = NULL;

static int wipe = 0;
static unsigned int nr_used_mft_records   = 0;
static unsigned int wiped_unused_mft_data = 0;
static unsigned int wiped_unused_mft      = 0;
static unsigned int wiped_resident_data   = 0;
static unsigned int wiped_timestamp_data  = 0;

static le64 volume_serial_number; /* new random serial number */
static u64 full_device_size; /* full size, including the backup boot sector */

static BOOL image_is_host_endian = FALSE;

#define IMAGE_MAGIC "\0ntfsclone-image"
#define IMAGE_MAGIC_SIZE 16
#define IMAGE_OFFSET_OFFSET 46 /* must be the same for all versions ! */
#define IMAGE_HDR_ALIGN 8 /* alignment wanted after header */

/* This is the first endianness safe format version. */
#define NTFSCLONE_IMG_VER_MAJOR_ENDIANNESS_SAFE	10
#define NTFSCLONE_IMG_VER_MINOR_ENDIANNESS_SAFE	0

/*
 * Set the version to 10.0 to avoid colisions with old ntfsclone which
 * stupidly used the volume version as the image version...  )-:  I hope NTFS
 * never reaches version 10.0 and if it does one day I hope no-one is using
 * such an old ntfsclone by then...
 *
 * NOTE: Only bump the minor version if the image format and header are still
 * backwards compatible.  Otherwise always bump the major version.  If in
 * doubt, bump the major version.
 *
 * Moved to 10.1 : Alternate boot sector now saved. Still compatible.
 */
#define NTFSCLONE_IMG_VER_MAJOR	10
#define NTFSCLONE_IMG_VER_MINOR	1

enum { CMD_GAP, CMD_NEXT } ;

/* All values are in little endian. */
static struct image_hdr {
	char magic[IMAGE_MAGIC_SIZE];
	u8 major_ver;
	u8 minor_ver;
	/* the following is aligned dangerously (too late...) */
	le32 cluster_size;
	le64 device_size;
	sle64 nr_clusters;
	le64 inuse;
	le32 offset_to_image_data;	/* From start of image_hdr. */
} __attribute__((__packed__)) image_hdr;

static int compare_bitmaps(struct bitmap *a, BOOL copy);

#define NTFSCLONE_IMG_HEADER_SIZE_OLD	\
		(offsetof(struct image_hdr, offset_to_image_data))

#define NTFS_MBYTE (1000 * 1000)

#define ERR_PREFIX   "ERROR"
#define PERR_PREFIX  ERR_PREFIX "(%d): "
#define NERR_PREFIX  ERR_PREFIX ": "

#define LAST_METADATA_INODE	11

#define NTFS_SECTOR_SIZE	  512

#define rounded_up_division(a, b) (((a) + (b - 1)) / (b))

#define read_all(f, p, n)  io_all((f), (p), (n), 0)
#define write_all(f, p, n) io_all((f), (p), (n), 1)

__attribute__((format(printf, 1, 2)))
static void Printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(msg_out, fmt, ap);
	va_end(ap);
	fflush(msg_out);
}

__attribute__((format(printf, 1, 2)))
static void perr_printf(const char *fmt, ...)
{
	va_list ap;
	int eo = errno;

	Printf(PERR_PREFIX, eo);
	va_start(ap, fmt);
	vfprintf(msg_out, fmt, ap);
	va_end(ap);
	Printf(": %s\n", strerror(eo));
	fflush(msg_out);
}

__attribute__((format(printf, 1, 2)))
static void err_printf(const char *fmt, ...)
{
	va_list ap;

	Printf(NERR_PREFIX);
	va_start(ap, fmt);
	vfprintf(msg_out, fmt, ap);
	va_end(ap);
	fflush(msg_out);
}

__attribute__((noreturn))
__attribute__((format(printf, 1, 2)))
static void err_exit(const char *fmt, ...)
{
	va_list ap;

	Printf(NERR_PREFIX);
	va_start(ap, fmt);
	vfprintf(msg_out, fmt, ap);
	va_end(ap);
	fflush(msg_out);
	if (vol)
		ntfs_umount(vol,FALSE);
	exit(1);
}

__attribute__((noreturn))
__attribute__((format(printf, 1, 2)))
static void perr_exit(const char *fmt, ...)
{
	va_list ap;
	int eo = errno;

	Printf(PERR_PREFIX, eo);
	va_start(ap, fmt);
	vfprintf(msg_out, fmt, ap);
	va_end(ap);
	Printf(": %s\n", strerror(eo));
	fflush(msg_out);
	if (vol)
		ntfs_umount(vol,FALSE);
	exit(1);
}


__attribute__((noreturn))
static void usage(int ret)
{
	fprintf(stderr, "\nUsage: %s [OPTIONS] SOURCE\n"
		"    Efficiently clone NTFS to a sparse file, image, device or standard output.\n"
		"\n"
		"    -o, --output FILE      Clone NTFS to the non-existent FILE\n"
		"    -O, --overwrite FILE   Clone NTFS to FILE, overwriting if exists\n"
		"    -s, --save-image       Save to the special image format\n"
		"    -r, --restore-image    Restore from the special image format\n"
		"        --rescue           Continue after disk read errors\n"
		"    -m, --metadata         Clone *only* metadata (for NTFS experts)\n"
		"    -n, --no-action        Test restoring, without outputting anything\n"
		"        --ignore-fs-check  Ignore the filesystem check result\n"
		"        --new-serial       Set a new serial number\n"
		"        --new-half-serial  Set a partial new serial number\n"
		"    -t, --preserve-timestamps Do not clear the timestamps\n"
		"    -q, --quiet            Do not display any progress bars\n"
		"    -f, --force            Force to progress (DANGEROUS)\n"
		"        --full-logfile     Include the full logfile in metadata output\n"
		"    -h, --help             Display this help\n"
#ifdef DEBUG
		"    -d, --debug            Show debug information\n"
#endif
		"    -V, --version          Display version information\n"
		"\n"
		"    If FILE is '-' then send the image to the standard output. If SOURCE is '-'\n"
		"    and --restore-image is used then read the image from the standard input.\n"
		"\n", EXEC_NAME);
	fprintf(stderr, "%s%s", ntfs_bugs, ntfs_home);
	exit(ret);
}

/**
 * version
 */
__attribute__((noreturn))
static void version(void)
{
	fprintf(stderr,
		   "Efficiently clone, image, restore or rescue an NTFS Volume.\n\n"
		   "Copyright (c) 2003-2006 Szabolcs Szakacsits\n"
		   "Copyright (c) 2004-2006 Anton Altaparmakov\n"
		   "Copyright (c) 2010-2018 Jean-Pierre Andre\n\n");
	fprintf(stderr, "%s\n%s%s", ntfs_gpl, ntfs_bugs, ntfs_home);
	exit(0);
}

static void parse_options(int argc, char **argv)
{
	static const char *sopt = "-dfhmno:O:qrstV";
	static const struct option lopt[] = {
#ifdef DEBUG
		{ "debug",	      no_argument,	 NULL, 'd' },
#endif
		{ "quiet",	      no_argument,	 NULL, 'q' },
		{ "force",	      no_argument,	 NULL, 'f' },
		{ "help",	      no_argument,	 NULL, 'h' },
		{ "metadata",	      no_argument,	 NULL, 'm' },
		{ "no-action",	      no_argument,	 NULL, 'n' },
		{ "output",	      required_argument, NULL, 'o' },
		{ "overwrite",	      required_argument, NULL, 'O' },
		{ "restore-image",    no_argument,	 NULL, 'r' },
		{ "ignore-fs-check",  no_argument,	 NULL, 'C' },
		{ "rescue",           no_argument,	 NULL, 'R' },
		{ "new-serial",       no_argument,	 NULL, 'I' },
		{ "new-half-serial",  no_argument,	 NULL, 'i' },
		{ "full-logfile",     no_argument,	 NULL, 'l' },
		{ "save-image",	      no_argument,	 NULL, 's' },
		{ "preserve-timestamps",   no_argument,  NULL, 't' },
		{ "version",	      no_argument,	 NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	int c;

	memset(&opt, 0, sizeof(opt));

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (opt.volume)
				usage(1);
			opt.volume = argv[optind-1];
			break;
		case 'd':
			opt.debug++;
			break;
		case 'q':
			opt.quiet++;
			break;
		case 'f':
			opt.force++;
			break;
		case 'h':
			usage(0);
		case '?':
			usage(1);
		case 'i':	/* not proposed as a short option */
			opt.new_serial |= 1;
			break;
		case 'I':	/* not proposed as a short option */
			opt.new_serial |= 2;
			break;
		case 'l':
			opt.full_logfile++;
			break;
		case 'm':
			opt.metadata++;
			break;
		case 'n':
			opt.no_action++;
			break;
		case 'O':
			opt.overwrite++;
			/* FALLTHRU */
		case 'o':
			if (opt.output)
				usage(1);
			opt.output = optarg;
			break;
		case 'r':
			opt.restore_image++;
			break;
		case 'C':
			opt.ignore_fs_check++;
			break;
		case 'R':
			opt.rescue++;
			break;
		case 's':
			opt.save_image++;
			break;
		case 't':
			opt.preserve_timestamps++;
			break;
		case 'V':
			version();
			break;
		default:
			err_printf("Unknown option '%s'.\n", argv[optind-1]);
			usage(1);
		}
	}

	if (!opt.no_action && (opt.output == NULL)) {
		err_printf("You must specify an output file.\n");
		usage(1);
	}

	if (!opt.no_action && (strcmp(opt.output, "-") == 0))
		opt.std_out++;

	if (opt.volume == NULL) {
		err_printf("You must specify a device file.\n");
		usage(1);
	}

	if (!opt.restore_image && !strcmp(opt.volume, "-")) {
		err_printf("Only special images can be read from standard input\n");
		usage(1);
	}

	if (opt.metadata && opt.save_image) {
		opt.metadata_image++;
		opt.save_image = 0;
	}

	if (opt.metadata && opt.restore_image)
		err_exit("Restoring only metadata from an image is not "
			 "supported!\n");

	if (opt.metadata && !opt.metadata_image && opt.std_out)
		err_exit("Cloning only metadata to stdout isn't supported!\n");

	if (opt.ignore_fs_check && !opt.metadata && !opt.rescue)
		err_exit("Filesystem check can be ignored only for metadata "
			 "cloning or rescue situations!\n");

	if (opt.save_image && opt.restore_image)
		err_exit("Saving and restoring an image at the same time "
			 "is not supported!\n");

	if (opt.no_action && !opt.restore_image)
		err_exit("A restoring test requires the restore option!\n");

	if (opt.no_action && opt.output)
		err_exit("A restoring test requires not defining any output!\n");

	if (!opt.no_action && !opt.std_out) {
		struct stat st;
#ifdef HAVE_WINDOWS_H
		BOOL blkdev = opt.output[0] && (opt.output[1] == ':')
					&& !opt.output[2];

		if (!blkdev && (stat(opt.output, &st) == -1)) {
#else
		if (stat(opt.output, &st) == -1) {
#endif
			if (errno != ENOENT)
				perr_exit("Couldn't access '%s'", opt.output);
		} else {
			if (!opt.overwrite)
				err_exit("Output file '%s' already exists.\n"
					 "Use option --overwrite if you want to"
					 " replace its content.\n", opt.output);

#ifdef HAVE_WINDOWS_H
			if (blkdev) {
#else
			if (S_ISBLK(st.st_mode)) {
#endif
				opt.blkdev_out = 1;
				if (opt.metadata && !opt.force)
					err_exit("Cloning only metadata to a "
					     "block device does not usually "
					     "make sense, aborting...\n"
					     "If you were instructed to do "
					     "this by a developer and/or are "
					     "sure that this is what you want "
					     "to do, run this utility again "
					     "but this time add the force "
					     "option, i.e. add '--force' to "
					     "the command line arguments.");
			}
		}
	}

	/*
	 * Send messages, debug information and library messages to stdout,
	 * but, if outputing to stdout send them to stderr
	 */
	if (opt.std_out) {
		msg_out = stderr;
		ntfs_log_set_handler(ntfs_log_handler_stderr);
	} else {
		msg_out = stdout;
		ntfs_log_set_handler(ntfs_log_handler_outerr);
	}
}

/*
 * Initialize the random number generator with the current
 * time, and generate a 64-bit random number for the serial
 * number
 */
static void generate_serial_number(void) {
	u64 sn;

		/* different values for parallel processes */
	srandom(time((time_t*)NULL) ^ (getpid() << 16));
	sn = ((u64)random() << 32) | ((u64)random() & 0xffffffff);
	volume_serial_number = cpu_to_le64(sn);
}

static void progress_init(struct progress_bar *p, u64 start, u64 stop, int res)
{
	p->start = start;
	p->stop = stop;
	p->unit = 100.0 / (stop - start);
	p->resolution = res;
}


static void progress_update(struct progress_bar *p, u64 current)
{
	float percent = p->unit * current;

	if (opt.quiet)
		return;

	if (current != p->stop) {
		if ((current - p->start) % p->resolution)
			return;
		Printf("%6.2f percent completed\r", percent);
	} else
		Printf("100.00 percent completed\n");
	fflush(msg_out);
}

static s64 is_critical_metadata(ntfs_walk_clusters_ctx *image, runlist *rl)
{
	s64 inode = image->ni->mft_no;

	if (inode <= LAST_METADATA_INODE) {

		/* Don't save bad sectors (both $Bad and unnamed are ignored */
		if (inode == FILE_BadClus && image->ctx->attr->type == AT_DATA)
			return 0;

		if ((inode != FILE_LogFile) || opt.full_logfile)
			return rl->length;

		if (image->ctx->attr->type == AT_DATA) {

			/* Save at least the first 16 KiB of FILE_LogFile */
			s64 s = (s64)16384 - rl->vcn * vol->cluster_size;
			if (s > 0) {
				s = rounded_up_division(s, vol->cluster_size);
				if (rl->length < s)
					s = rl->length;
				return s;
			}
			return 0;
		}
	}

	if (image->ctx->attr->type != AT_DATA)
		return rl->length;

	return 0;
}

static off_t tellin(int in)
{
	return (lseek(in, 0, SEEK_CUR));
}

static int io_all(void *fd, void *buf, int count, int do_write)
{
	int i;
	struct ntfs_device *dev = fd;

	while (count > 0) {
		if (do_write) {
			if (opt.no_action) {
				i = count;
			} else {
				if (opt.save_image || opt.metadata_image)
					i = fwrite(buf, 1, count, stream_out);
#ifdef HAVE_WINDOWS_H
				else if (dev_out)
					i = dev_out->d_ops->write(dev_out,
								buf, count);
#endif
				else
					i = write(*(int *)fd, buf, count);
			}
		} else if (opt.restore_image)
			i = read(*(int *)fd, buf, count);
		else
			i = dev->d_ops->read(dev, buf, count);
		if (i < 0) {
			if (errno != EAGAIN && errno != EINTR)
				return -1;
		} else if (i == 0 && !do_write && opt.restore_image) {
			return -1;
		} else {
			count -= i;
			buf = i + (char *) buf;
		}
	}
	return 0;
}


static void rescue_sector(void *fd, u32 bytes_per_sector, off_t pos, void *buff)
{
	const char badsector_magic[] = "BadSectoR";
	struct ntfs_device *dev = fd;

	if (opt.restore_image) {
		if (!opt.no_action
		    && (lseek(*(int *)fd, pos, SEEK_SET) == (off_t)-1))
			perr_exit("lseek");
	} else {
		if (vol->dev->d_ops->seek(dev, pos, SEEK_SET) == (off_t)-1)
			perr_exit("seek input");
	}

	if (read_all(fd, buff, bytes_per_sector) == -1) {
		Printf("WARNING: Can't read sector at %llu, lost data.\n",
			(unsigned long long)pos);
		memset(buff, '?', bytes_per_sector);
		memmove(buff, badsector_magic, sizeof(badsector_magic));
	}
}

/*
 *		Read a cluster, try to rescue if cannot read
 */

static void read_rescue(void *fd, char *buff, u32 csize, u32 bytes_per_sector,
				u64 rescue_lcn)
{
	off_t rescue_pos;

	if (read_all(fd, buff, csize) == -1) {

		if (errno != EIO)
			perr_exit("read_all");
		else if (opt.rescue){
			u32 i;

			rescue_pos = (off_t)(rescue_lcn * csize);
			for (i = 0; i < csize; i += bytes_per_sector)
				rescue_sector(fd, bytes_per_sector,
						rescue_pos + i, buff + i);
		} else {
			Printf("%s", bad_sectors_warning_msg);
			err_exit("Disk is faulty, can't make full backup!");
		}
	}
}

static void copy_cluster(int rescue, u64 rescue_lcn, u64 lcn)
{
	char *buff;
	/* vol is NULL if opt.restore_image is set */
	s32 csize = le32_to_cpu(image_hdr.cluster_size);
	BOOL backup_bootsector;
	void *fd = (void *)&fd_in;
	off_t rescue_pos;
	NTFS_BOOT_SECTOR *bs;
	le64 mask;
	static u16 bytes_per_sector = NTFS_SECTOR_SIZE;

	if (!opt.restore_image) {
		csize = vol->cluster_size;
		bytes_per_sector = vol->sector_size;
		fd = vol->dev;
	}

	rescue_pos = (off_t)(rescue_lcn * csize);
	buff = (char*)ntfs_malloc(csize);
	if (!buff)
		err_exit("Not enough memory");

		/* possible partial cluster holding the backup boot sector */
	backup_bootsector = (lcn + 1)*csize >= full_device_size;
	if (backup_bootsector) {
		csize = full_device_size - lcn*csize;
		if (csize < 0) {
			err_exit("Corrupted input, copy aborted");
		}
	}

// need reading when not about to write ?
	if (read_all(fd, buff, csize) == -1) {

		if (errno != EIO) {
			if (!errno && opt.restore_image)
				err_exit("Short image file...\n");
			else
				perr_exit("read_all");
		}
		else if (rescue){
			s32 i;
			for (i = 0; i < csize; i += bytes_per_sector)
				rescue_sector(fd, bytes_per_sector,
						rescue_pos + i, buff + i);
		} else {
			Printf("%s", bad_sectors_warning_msg);
			err_exit("Disk is faulty, can't make full backup!");
		}
	}

		/* Set the new serial number if requested */
	if (opt.new_serial
	    && !opt.save_image
	    && (!lcn || backup_bootsector)) {
			/*
			 * For updating the backup boot sector, we need to
			 * know the sector size, but this is not recorded
			 * in the image header, so we collect it on the fly
			 * while reading the first boot sector.
			 */
		if (!lcn) {
			bs = (NTFS_BOOT_SECTOR*)buff;
			bytes_per_sector = le16_to_cpu(bs->bpb.bytes_per_sector);
			if ((bytes_per_sector > csize)
			    || (bytes_per_sector < NTFS_SECTOR_SIZE))
				bytes_per_sector = NTFS_SECTOR_SIZE;
		} else
			bs = (NTFS_BOOT_SECTOR*)(buff
						+ csize - bytes_per_sector);
		if (opt.new_serial & 2)
			bs->volume_serial_number = volume_serial_number;
		else {
			mask = const_cpu_to_le64(~0x0ffffffffULL);
			bs->volume_serial_number
			    = (volume_serial_number & mask)
				| (bs->volume_serial_number & ~mask);
		}
			/* Show the new full serial after merging */
		if (!lcn)
			Printf("New serial number      : 0x%llx\n",
				(long long)le64_to_cpu(
						bs->volume_serial_number));
	}

	if (opt.save_image || (opt.metadata_image && wipe)) {
		char cmd = CMD_NEXT;
		if (write_all(&fd_out, &cmd, sizeof(cmd)) == -1)
			perr_exit("write_all");
	}

	if ((!opt.metadata_image || wipe)
	    && (write_all(&fd_out, buff, csize) == -1)) {
#ifndef NO_STATFS
		int err = errno;
		perr_printf("Write failed");
		if (err == EIO && opt.stfs.f_type == 0x517b)
			Printf("Apparently you tried to clone to a remote "
			       "Windows computer but they don't\nhave "
			       "efficient sparse file handling by default. "
			       "Please try a different method.\n");
		exit(1);
#else
		perr_printf("Write failed");
#endif
	}
	free(buff);
}

static s64 lseek_out(int fd, s64 pos, int mode)
{
	s64 ret;

	if (dev_out)
		ret = (dev_out->d_ops->seek)(dev_out, pos, mode);
	else
		ret = lseek(fd, pos, mode);
	return (ret);
}

static void lseek_to_cluster(s64 lcn)
{
	off_t pos;

	pos = (off_t)(lcn * vol->cluster_size);

	if (vol->dev->d_ops->seek(vol->dev, pos, SEEK_SET) == (off_t)-1)
		perr_exit("lseek input");

	if (opt.std_out || opt.save_image || opt.metadata_image)
		return;

	if (lseek_out(fd_out, pos, SEEK_SET) == (off_t)-1)
			perr_exit("lseek output");
}

static void gap_to_cluster(s64 gap)
{
	sle64 count;
	char buf[1 + sizeof(count)];

	if (gap) {
		count = cpu_to_sle64(gap);
		buf[0] = CMD_GAP;
		memcpy(&buf[1], &count, sizeof(count));
		if (write_all(&fd_out, buf, sizeof(buf)) == -1)
			perr_exit("write_all");
	}
}

static void image_skip_clusters(s64 count)
{
	if (opt.save_image && count > 0) {
		sle64 count_buf;
		char buff[1 + sizeof(count)];

		buff[0] = CMD_GAP;
		count_buf = cpu_to_sle64(count);
		memcpy(buff + 1, &count_buf, sizeof(count_buf));

		if (write_all(&fd_out, buff, sizeof(buff)) == -1)
			perr_exit("write_all");
	}
}

static void write_image_hdr(void)
{
	char alignment[IMAGE_HDR_ALIGN];

	if (opt.save_image || opt.metadata_image) {
		int alignsize = le32_to_cpu(image_hdr.offset_to_image_data)
				- sizeof(image_hdr);
		memset(alignment,0,IMAGE_HDR_ALIGN);
		if ((alignsize < 0)
			|| write_all(&fd_out, &image_hdr, sizeof(image_hdr))
			|| write_all(&fd_out, alignment, alignsize))
			perr_exit("write_all");
	}
}

static void clone_ntfs(u64 nr_clusters, int more_use)
{
	u64 cl, last_cl;  /* current and last used cluster */
	void *buf;
	u32 csize = vol->cluster_size;
	u64 p_counter = 0;
	char alignment[IMAGE_HDR_ALIGN];
	struct progress_bar progress;

	if (opt.save_image)
		Printf("Saving NTFS to image ...\n");
	else
		Printf("Cloning NTFS ...\n");

	if (opt.new_serial)
		generate_serial_number();

	buf = ntfs_calloc(csize);
	if (!buf)
		perr_exit("clone_ntfs");

	progress_init(&progress, p_counter, nr_clusters, 100);

	if (opt.save_image) {
		int alignsize = le32_to_cpu(image_hdr.offset_to_image_data)
				- sizeof(image_hdr);
		memset(alignment,0,IMAGE_HDR_ALIGN);
		if ((alignsize < 0)
			|| write_all(&fd_out, &image_hdr, sizeof(image_hdr))
			|| write_all(&fd_out, alignment, alignsize))
			perr_exit("write_all");
	}

		/* save suspicious clusters if required */
	if (more_use && opt.ignore_fs_check) {
		compare_bitmaps(&lcn_bitmap, TRUE);
	}
		/* Examine up to the alternate boot sector */
	for (last_cl = cl = 0; cl <= (u64)vol->nr_clusters; cl++) {

		if (ntfs_bit_get(lcn_bitmap.bm, cl)) {
			progress_update(&progress, ++p_counter);
			lseek_to_cluster(cl);
			image_skip_clusters(cl - last_cl - 1);

			copy_cluster(opt.rescue, cl, cl);
			last_cl = cl;
			continue;
		}

		if (opt.std_out && !opt.save_image) {
			progress_update(&progress, ++p_counter);
			if (write_all(&fd_out, buf, csize) == -1)
				perr_exit("write_all");
		}
	}
	image_skip_clusters(cl - last_cl - 1);
	free(buf);
}

static void write_empty_clusters(s32 csize, s64 count,
				 struct progress_bar *progress, u64 *p_counter)
{
	s64 i;
	char *buff;

	buff = (char*)ntfs_malloc(csize);
	if (!buff)
		err_exit("Not enough memory");

	memset(buff, 0, csize);

	for (i = 0; i < count; i++) {
		if (write_all(&fd_out, buff, csize) == -1)
			perr_exit("write_all");
		progress_update(progress, ++(*p_counter));
	}
	free(buff);
}

static void restore_image(void)
{
	s64 pos = 0, count;
	s32 csize = le32_to_cpu(image_hdr.cluster_size);
	char cmd;
	u64 p_counter = 0;
	struct progress_bar progress;

	Printf("Restoring NTFS from image ...\n");

	progress_init(&progress, p_counter, opt.std_out ?
		      (u64)sle64_to_cpu(image_hdr.nr_clusters) + 1 :
		      le64_to_cpu(image_hdr.inuse) + 1,
		      100);

	if (opt.new_serial)
		generate_serial_number();

		/* Restore up to the alternate boot sector */
	while (pos <= sle64_to_cpu(image_hdr.nr_clusters)) {
		if (read_all(&fd_in, &cmd, sizeof(cmd)) == -1) {
			if (pos == sle64_to_cpu(image_hdr.nr_clusters)) {
				/* alternate boot sector no present in old images */
				Printf("Warning : no alternate boot"
						" sector in image\n");
				break;
			} else
				perr_exit("read_all");
		}

		if (cmd == CMD_GAP) {
			if (!image_is_host_endian) {
				sle64 lecount;

				/* little endian image, on any computer */
				if (read_all(&fd_in, &lecount,
						sizeof(lecount)) == -1)
					perr_exit("read_all");
				count = sle64_to_cpu(lecount);
			} else {
				/* big endian image on big endian computer */
				if (read_all(&fd_in, &count,
						sizeof(count)) == -1)
					perr_exit("read_all");
			}
			if (!count)
				err_exit("Bad offset at input location 0x%llx\n",
					(long long)tellin(fd_in) - 9);
			if (opt.std_out) {
				if ((!p_counter && count) || (count < 0))
					err_exit("Cannot restore a metadata"
						" image to stdout\n");
				else
					write_empty_clusters(csize, count,
						     &progress, &p_counter);
			} else {
				if (((pos + count) < 0)
				   || ((pos + count)
					> sle64_to_cpu(image_hdr.nr_clusters)))
					err_exit("restore_image: corrupt image "
						"at input offset %lld\n",
						(long long)tellin(fd_in) - 9);
				else {
					if (!opt.no_action
					    && (lseek_out(fd_out, count * csize,
							SEEK_CUR) == (off_t)-1))
						perr_exit("restore_image: lseek");
				}
			}
			pos += count;
		} else if (cmd == CMD_NEXT) {
			copy_cluster(0, 0, pos);
			pos++;
			progress_update(&progress, ++p_counter);
		} else
			err_exit("Invalid command code %d at input offset 0x%llx\n",
					cmd, (long long)tellin(fd_in) - 1);
	}
}

static void wipe_index_entry_timestams(INDEX_ENTRY *e)
{
	static const struct timespec zero_time = { .tv_sec = 0, .tv_nsec = 0 };
	sle64 timestamp = timespec2ntfs(zero_time);

	/* FIXME: can fall into infinite loop if corrupted */
	while (!(e->ie_flags & INDEX_ENTRY_END)) {

		e->key.file_name.creation_time = timestamp;
		e->key.file_name.last_data_change_time = timestamp;
		e->key.file_name.last_mft_change_time = timestamp;
		e->key.file_name.last_access_time = timestamp;

		wiped_timestamp_data += 32;

		e = (INDEX_ENTRY *)((u8 *)e + le16_to_cpu(e->length));
	}
}

static void wipe_index_allocation_timestamps(ntfs_inode *ni, ATTR_RECORD *attr)
{
	INDEX_ALLOCATION *indexa, *tmp_indexa;
	INDEX_ENTRY *entry;
	INDEX_ROOT *indexr;
	u8 *bitmap, *byte;
	int bit;
	ntfs_attr *na;
	ntfschar *name;
	u32 name_len;

	indexr = ntfs_index_root_get(ni, attr);
	if (!indexr) {
		perr_printf("Failed to read $INDEX_ROOT attribute of inode "
			    "%lld", (long long)ni->mft_no);
		return;
	}

	if (indexr->type != AT_FILE_NAME)
		goto out_indexr;

	name = (ntfschar *)((u8 *)attr + le16_to_cpu(attr->name_offset));
	name_len = attr->name_length;

	byte = bitmap = ntfs_attr_readall(ni, AT_BITMAP, name, name_len,
						NULL);
	if (!byte) {
		perr_printf("Failed to read $BITMAP attribute");
		goto out_indexr;
	}

	na = ntfs_attr_open(ni, AT_INDEX_ALLOCATION, name, name_len);
	if (!na) {
		perr_printf("Failed to open $INDEX_ALLOCATION attribute");
		goto out_bitmap;
	}

	if (!na->data_size)
		goto out_na;

	tmp_indexa = indexa = ntfs_malloc(na->data_size);
	if (!tmp_indexa)
		goto out_na;

	if (ntfs_attr_pread(na, 0, na->data_size, indexa) != na->data_size) {
		perr_printf("Failed to read $INDEX_ALLOCATION attribute");
		goto out_indexa;
	}

	bit = 0;
	while ((u8 *)tmp_indexa < (u8 *)indexa + na->data_size) {
		if (*byte & (1 << bit)) {
			if (ntfs_mst_post_read_fixup((NTFS_RECORD *)tmp_indexa,
					le32_to_cpu(
					indexr->index_block_size))) {
				perr_printf("Damaged INDX record");
				goto out_indexa;
			}
			entry = (INDEX_ENTRY *)((u8 *)tmp_indexa + le32_to_cpu(
				tmp_indexa->index.entries_offset) + 0x18);

			wipe_index_entry_timestams(entry);

			if (ntfs_mft_usn_dec((MFT_RECORD *)tmp_indexa))
				perr_exit("ntfs_mft_usn_dec");

			if (ntfs_mst_pre_write_fixup((NTFS_RECORD *)tmp_indexa,
					le32_to_cpu(
					indexr->index_block_size))) {
				perr_printf("INDX write fixup failed");
				goto out_indexa;
			}
		}
		tmp_indexa = (INDEX_ALLOCATION *)((u8 *)tmp_indexa +
				le32_to_cpu(indexr->index_block_size));
		bit++;
		if (bit > 7) {
			bit = 0;
			byte++;
		}
	}
	if (ntfs_rl_pwrite(vol, na->rl, 0, 0, na->data_size, indexa) != na->data_size)
		perr_printf("ntfs_rl_pwrite failed for inode %lld",
				(long long)ni->mft_no);
out_indexa:
	free(indexa);
out_na:
	ntfs_attr_close(na);
out_bitmap:
	free(bitmap);
out_indexr:
	free(indexr);
}

static void wipe_index_root_timestamps(ATTR_RECORD *attr, sle64 timestamp)
{
	INDEX_ENTRY *entry;
	INDEX_ROOT *iroot;

	iroot = (INDEX_ROOT *)((u8 *)attr + le16_to_cpu(attr->value_offset));
	entry = (INDEX_ENTRY *)((u8 *)iroot +
			le32_to_cpu(iroot->index.entries_offset) + 0x10);

	while (!(entry->ie_flags & INDEX_ENTRY_END)) {

		if (iroot->type == AT_FILE_NAME) {

			entry->key.file_name.creation_time = timestamp;
			entry->key.file_name.last_access_time = timestamp;
			entry->key.file_name.last_data_change_time = timestamp;
			entry->key.file_name.last_mft_change_time = timestamp;

			wiped_timestamp_data += 32;

		} else if (ntfs_names_are_equal(NTFS_INDEX_Q,
				sizeof(NTFS_INDEX_Q) / 2 - 1,
				(ntfschar *)((char *)attr +
					    le16_to_cpu(attr->name_offset)),
				attr->name_length, CASE_SENSITIVE, NULL, 0)) {

			QUOTA_CONTROL_ENTRY *quota_q;

			quota_q = (QUOTA_CONTROL_ENTRY *)((u8 *)entry +
					le16_to_cpu(entry->data_offset));
			/*
			 *  FIXME: no guarantee it's indeed /$Extend/$Quota:$Q.
			 *  For now, as a minimal safeguard, we check only for
			 *  quota version 2 ...
			 */
			if (le32_to_cpu(quota_q->version) == 2) {
				quota_q->change_time = timestamp;
				wiped_timestamp_data += 4;
			}
		}

		entry = (INDEX_ENTRY*)((u8*)entry + le16_to_cpu(entry->length));
	}
}

#define WIPE_TIMESTAMPS(atype, attr, timestamp)			\
do {								\
	atype *ats;						\
	ats = (atype *)((char *)(attr) + le16_to_cpu((attr)->value_offset)); \
								\
	ats->creation_time = (timestamp);	       		\
	ats->last_data_change_time = (timestamp);		\
	ats->last_mft_change_time= (timestamp);			\
	ats->last_access_time = (timestamp);			\
								\
	wiped_timestamp_data += 32;				\
								\
} while (0)

static void wipe_timestamps(ntfs_walk_clusters_ctx *image)
{
	static const struct timespec zero_time = { .tv_sec = 0, .tv_nsec = 0 };
	ATTR_RECORD *a = image->ctx->attr;
	sle64 timestamp = timespec2ntfs(zero_time);

	if (a->type == AT_FILE_NAME)
		WIPE_TIMESTAMPS(FILE_NAME_ATTR, a, timestamp);

	else if (a->type == AT_STANDARD_INFORMATION)
		WIPE_TIMESTAMPS(STANDARD_INFORMATION, a, timestamp);

	else if (a->type == AT_INDEX_ROOT)
		wipe_index_root_timestamps(a, timestamp);
}

static void wipe_resident_data(ntfs_walk_clusters_ctx *image)
{
	ATTR_RECORD *a;
	u32 i;
	int n = 0;
	u8 *p;

	a = image->ctx->attr;
	p = (u8*)a + le16_to_cpu(a->value_offset);

	if (image->ni->mft_no <= LAST_METADATA_INODE)
		return;

	if (a->type != AT_DATA)
		return;

	for (i = 0; i < le32_to_cpu(a->value_length); i++) {
		if (p[i]) {
			p[i] = 0;
			n++;
		}
	}

	wiped_resident_data += n;
}

static int wipe_data(char *p, int pos, int len)
{
	int wiped = 0;

	for (p += pos; --len >= 0;) {
		if (p[len]) {
			p[len] = 0;
			wiped++;
		}
	}

	return wiped;
}

static void wipe_unused_mft_data(ntfs_inode *ni)
{
	int unused;
	MFT_RECORD *m = ni->mrec;

	/* FIXME: broken MFTMirr update was fixed in libntfs, check if OK now */
	if (ni->mft_no <= LAST_METADATA_INODE)
		return;

	unused = le32_to_cpu(m->bytes_allocated) - le32_to_cpu(m->bytes_in_use);
	wiped_unused_mft_data += wipe_data((char *)m,
			le32_to_cpu(m->bytes_in_use), unused);
}

static void wipe_unused_mft(ntfs_inode *ni)
{
	int unused;
	MFT_RECORD *m = ni->mrec;

	/* FIXME: broken MFTMirr update was fixed in libntfs, check if OK now */
	if (ni->mft_no <= LAST_METADATA_INODE)
		return;

	unused = le32_to_cpu(m->bytes_in_use) - sizeof(MFT_RECORD);
	wiped_unused_mft += wipe_data((char *)m, sizeof(MFT_RECORD), unused);
}

static void clone_logfile_parts(ntfs_walk_clusters_ctx *image, runlist *rl)
{
	s64 offset = 0, lcn, vcn;

	while (1) {

		vcn = offset / image->ni->vol->cluster_size;
		lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
		if (lcn < 0)
			break;

		lseek_to_cluster(lcn);

		if ((lcn + 1) != image->current_lcn) {
			/* do not duplicate a cluster */
			if (opt.metadata_image && wipe)
				gap_to_cluster(lcn - image->current_lcn);

			copy_cluster(opt.rescue, lcn, lcn);
		}
		image->current_lcn = lcn + 1;
		if (opt.metadata_image && !wipe)
			image->inuse++;

		if (offset == 0)
			offset = NTFS_BLOCK_SIZE >> 1;
		else
			offset <<= 1;
	}
}

/*
 *		In-memory wiping of MFT record or MFTMirr record
 *	(only for metadata images)
 *
 *	The resident data and (optionally) the timestamps are wiped.
 */

static void wipe_mft(char *mrec, u32 mrecsz, u64 mft_no)
{
	ntfs_walk_clusters_ctx image;
	ntfs_attr_search_ctx *ctx;
	ntfs_inode ni;

	ni.mft_no = mft_no;
	ni.mrec = (MFT_RECORD*)mrec;
	ni.vol = vol; /* Hmm */
	image.ni = &ni;
	ntfs_mst_post_read_fixup_warn((NTFS_RECORD*)mrec,mrecsz,FALSE);
	wipe_unused_mft_data(&ni);
	if (!(((MFT_RECORD*)mrec)->flags & MFT_RECORD_IN_USE)) {
		wipe_unused_mft(&ni);
	} else {
			/* ctx with no ntfs_inode prevents from searching external attrs */
		if (!(ctx = ntfs_attr_get_search_ctx((ntfs_inode*)NULL, (MFT_RECORD*)mrec)))
			perr_exit("ntfs_get_attr_search_ctx");

		while (!ntfs_attr_lookup(AT_UNUSED, NULL, 0, CASE_SENSITIVE, 0,
						NULL, 0, ctx)) {
			if (ctx->attr->type == AT_END)
				break;

			image.ctx = ctx;
			if (!ctx->attr->non_resident
			    && (mft_no > LAST_METADATA_INODE))
				wipe_resident_data(&image);
			if (!opt.preserve_timestamps)
				wipe_timestamps(&image);
		}
		ntfs_attr_put_search_ctx(ctx);
	}
	ntfs_mft_usn_dec((MFT_RECORD*)mrec);
	ntfs_mst_pre_write_fixup((NTFS_RECORD*)mrec,mrecsz);
}

/*
 *		In-memory wiping of a directory record (I30)
 *	(only for metadata images)
 *
 *	The timestamps are (optionally) wiped
 */

static void wipe_indx(char *mrec, u32 mrecsz)
{
	INDEX_ENTRY *entry;
	INDEX_ALLOCATION *indexa;

	if (ntfs_mst_post_read_fixup((NTFS_RECORD *)mrec, mrecsz)) {
		perr_printf("Damaged INDX record");
		goto out_indexa;
	}
	indexa = (INDEX_ALLOCATION*)mrec;
		/*
		 * The index bitmap is not checked, obsoleted records are
		 * wiped if they pass the safety checks
		 */
	if ((indexa->magic == magic_INDX)
	    && (le32_to_cpu(indexa->index.entries_offset) >= sizeof(INDEX_HEADER))
	    && (le32_to_cpu(indexa->index.allocated_size) <= mrecsz)) {
		entry = (INDEX_ENTRY *)((u8 *)mrec + le32_to_cpu(
				indexa->index.entries_offset) + 0x18);
		wipe_index_entry_timestams(entry);
	}

	if (ntfs_mft_usn_dec((MFT_RECORD *)mrec))
		perr_exit("ntfs_mft_usn_dec");

	if (ntfs_mst_pre_write_fixup((NTFS_RECORD *)mrec, mrecsz)) {
		perr_printf("INDX write fixup failed");
		goto out_indexa;
	}
out_indexa : ;
}

/*
 *		Output a set of related clusters (MFT record or index block)
 */

static void write_set(char *buff, u32 csize, s64 *current_lcn,
			runlist_element *rl, u32 wi, u32 wj, u32 cnt)
{
	u32 k;
	s64 target_lcn;
	char cmd = CMD_NEXT;

	for (k=0; k<cnt; k++) {
		target_lcn = rl[wi].lcn + wj;
		if (target_lcn != *current_lcn)
			gap_to_cluster(target_lcn - *current_lcn);
		if ((write_all(&fd_out, &cmd, sizeof(cmd)) == -1)
		    || (write_all(&fd_out, &buff[k*csize], csize) == -1))
			perr_exit("Failed to write_all");
		*current_lcn = target_lcn + 1;
			
		if (++wj >= rl[wi].length) {
			wj = 0;
			wi++;
		}
	}
}

/*
 *		Copy and wipe the full MFT or MFTMirr data.
 *	(only for metadata images)
 *
 *	Data are read and written by full clusters, but the wiping is done
 *	per MFT record.
 */

static void copy_wipe_mft(ntfs_walk_clusters_ctx *image, runlist *rl)
{
	char *buff;
	void *fd;
	s64 mft_no;
	u32 mft_record_size;
	u32 csize;
	u32 buff_size;
	u32 bytes_per_sector;
	u32 records_per_set;
	u32 clusters_per_set;
	u32 wi,wj; /* indexes for reading */
	u32 ri,rj; /* indexes for writing */
	u32 k; /* lcn within run */
	u32 r; /* mft_record within set */
	s64 current_lcn;

	current_lcn = image->current_lcn;
	mft_record_size = image->ni->vol->mft_record_size;
	csize = image->ni->vol->cluster_size;
	bytes_per_sector = image->ni->vol->sector_size;
	fd = image->ni->vol->dev;
		/*
		 * Depending on the sizes, there may be several records
		 * per cluster, or several clusters per record.
		 * Anyway, records are read and rescued by full clusters.
		 */
	if (csize >= mft_record_size) {
		records_per_set = csize/mft_record_size;
		clusters_per_set = 1;
		buff_size = csize;
	} else {
		clusters_per_set = mft_record_size/csize;
		records_per_set = 1;
		buff_size = mft_record_size;
	}
	buff = (char*)ntfs_malloc(buff_size);
	if (!buff)
		err_exit("Not enough memory");

	mft_no = 0;
	ri = rj = 0;
	wi = wj = 0;
	if (rl[ri].length)
		lseek_to_cluster(rl[ri].lcn);
	while (rl[ri].length) {
		for (k=0; (k<clusters_per_set) && rl[ri].length; k++) {
			read_rescue(fd, &buff[k*csize], csize, bytes_per_sector,
							rl[ri].lcn + rj);
			if (++rj >= rl[ri].length) {
				rj = 0;
				if (rl[++ri].length)
					lseek_to_cluster(rl[ri].lcn);
			}
		}
		if (k == clusters_per_set) {
			for (r=0; r<records_per_set; r++) {
				if (!strncmp(&buff[r*mft_record_size],"FILE",4))
					wipe_mft(&buff[r*mft_record_size],
						mft_record_size, mft_no);
				mft_no++;
			}
			write_set(buff, csize, &current_lcn,
					rl, wi, wj, clusters_per_set);
			wj += clusters_per_set;
			while (rl[wi].length && (wj >= rl[wi].length))
				wj -= rl[wi++].length;
		} else {
			err_exit("Short last MFT record\n");
		}
	}
	image->current_lcn = current_lcn;
	free(buff);
}

/*
 *		Copy and wipe the non-resident part of a directory index
 *	(only for metadata images)
 *
 *	Data are read and written by full clusters, but the wiping is done
 *	per index record.
 */

static void copy_wipe_i30(ntfs_walk_clusters_ctx *image, runlist *rl)
{
	char *buff;
	void *fd;
	u32 indx_record_size;
	u32 csize;
	u32 buff_size;
	u32 bytes_per_sector;
	u32 records_per_set;
	u32 clusters_per_set;
	u32 wi,wj; /* indexes for reading */
	u32 ri,rj; /* indexes for writing */
	u32 k; /* lcn within run */
	u32 r; /* mft_record within set */
	s64 current_lcn;

	current_lcn = image->current_lcn;
	csize = image->ni->vol->cluster_size;
	bytes_per_sector = image->ni->vol->sector_size;
	fd = image->ni->vol->dev;
		/*
		 * Depending on the sizes, there may be several records
		 * per cluster, or several clusters per record.
		 * Anyway, records are read and rescued by full clusters.
		 */
	indx_record_size = image->ni->vol->indx_record_size;
	if (csize >= indx_record_size) {
		records_per_set = csize/indx_record_size;
		clusters_per_set = 1;
		buff_size = csize;
	} else {
		clusters_per_set = indx_record_size/csize;
		records_per_set = 1;
		buff_size = indx_record_size;
	}
	buff = (char*)ntfs_malloc(buff_size);
	if (!buff)
		err_exit("Not enough memory");

	ri = rj = 0;
	wi = wj = 0;
	if (rl[ri].length)
		lseek_to_cluster(rl[ri].lcn);
	while (rl[ri].length) {
		for (k=0; (k<clusters_per_set) && rl[ri].length; k++) {
			read_rescue(fd, &buff[k*csize], csize, bytes_per_sector,
							rl[ri].lcn + rj);
			if (++rj >= rl[ri].length) {
				rj = 0;
				if (rl[++ri].length)
					lseek_to_cluster(rl[ri].lcn);
			}
		}
		if (k == clusters_per_set) {
			/* wipe records_per_set records */
			if (!opt.preserve_timestamps)
				for (r=0; r<records_per_set; r++) {
					if (!strncmp(&buff[r*indx_record_size],"INDX",4))
						wipe_indx(&buff[r*indx_record_size],
							indx_record_size);
				}
			write_set(buff, csize, &current_lcn,
					rl, wi, wj, clusters_per_set);
			wj += clusters_per_set;
			while (rl[wi].length && (wj >= rl[wi].length))
				wj -= rl[wi++].length;
		} else {
			err_exit("Short last directory index record\n");
		}
	}
	image->current_lcn = current_lcn;
	free(buff);
}

static void dump_clusters(ntfs_walk_clusters_ctx *image, runlist *rl)
{
	s64 i, len; /* number of clusters to copy */

	if (opt.restore_image)
		err_exit("Bug : invalid dump_clusters()\n");

	if ((opt.std_out && !opt.metadata_image) || !opt.metadata)
		return;
	if (!(len = is_critical_metadata(image, rl)))
		return;

	lseek_to_cluster(rl->lcn);
	if (opt.metadata_image ? wipe : !wipe) {
		if (opt.metadata_image)
			gap_to_cluster(rl->lcn - image->current_lcn);
		/* FIXME: this could give pretty suboptimal performance */
		for (i = 0; i < len; i++)
			copy_cluster(opt.rescue, rl->lcn + i, rl->lcn + i);
		if (opt.metadata_image)
			image->current_lcn = rl->lcn + len;
	}
}

static void walk_runs(struct ntfs_walk_cluster *walk)
{
	int i, j;
	runlist *rl;
	ATTR_RECORD *a;
	ntfs_attr_search_ctx *ctx;
	BOOL mft_data;
	BOOL index_i30;

	ctx = walk->image->ctx;
	a = ctx->attr;

	if (!a->non_resident) {
		if (wipe) {
			wipe_resident_data(walk->image);
			if (!opt.preserve_timestamps)
				wipe_timestamps(walk->image);
		}
		return;
	}

	if (wipe
	    && !opt.preserve_timestamps
	    && walk->image->ctx->attr->type == AT_INDEX_ALLOCATION)
		wipe_index_allocation_timestamps(walk->image->ni, a);

	if (!(rl = ntfs_mapping_pairs_decompress(vol, a, NULL)))
		perr_exit("ntfs_decompress_mapping_pairs");

		/* special wipings for MFT records and directory indexes */
	mft_data = ((walk->image->ni->mft_no == FILE_MFT)
			|| (walk->image->ni->mft_no == FILE_MFTMirr))
		    && (a->type == AT_DATA);
	index_i30 = (walk->image->ctx->attr->type == AT_INDEX_ALLOCATION)
		    && (a->name_length == 4)
		    && !memcmp((char*)a + le16_to_cpu(a->name_offset),
					NTFS_INDEX_I30,8);

	for (i = 0; rl[i].length; i++) {
		s64 lcn = rl[i].lcn;
		s64 lcn_length = rl[i].length;

		if (lcn == LCN_HOLE || lcn == LCN_RL_NOT_MAPPED)
			continue;

		/* FIXME: ntfs_mapping_pairs_decompress should return error */
		if (lcn < 0 || lcn_length < 0)
			err_exit("Corrupt runlist in inode %lld attr %x LCN "
				 "%llx length %llx\n",
				(long long)ctx->ntfs_ino->mft_no,
				(unsigned int)le32_to_cpu(a->type),
				(long long)lcn, (long long)lcn_length);

		if (opt.metadata_image ? wipe && !mft_data && !index_i30 : !wipe)
			dump_clusters(walk->image, rl + i);

		for (j = 0; j < lcn_length; j++) {
			u64 k = (u64)lcn + j;
			if (ntfs_bit_get_and_set(lcn_bitmap.bm, k, 1)) {
				if (opt.ignore_fs_check)
					Printf("Cluster %llu is referenced"
						" twice!\n",
						(unsigned long long)k);
				else
					err_exit("Cluster %llu referenced"
						" twice!\nYou didn't shutdown"
						" your Windows properly?\n",
						(unsigned long long)k);
			}
		}

		if (!opt.metadata_image)
			walk->image->inuse += lcn_length;
			/*
			 * For a metadata image, we have to compute the
			 * number of metadata clusters for the percentages
			 * to be displayed correctly while restoring.
			 */
		if (!wipe && opt.metadata_image) {
			if ((walk->image->ni->mft_no == FILE_LogFile)
			    && (walk->image->ctx->attr->type == AT_DATA)) {
					/* 16 KiB of FILE_LogFile */
				walk->image->inuse
				   += is_critical_metadata(walk->image,rl);
			} else {
				if ((walk->image->ni->mft_no
						<= LAST_METADATA_INODE)
				   || (walk->image->ctx->attr->type != AT_DATA))
					walk->image->inuse += lcn_length;
			}
		}
	}
	if (wipe && opt.metadata_image) {
		ntfs_attr *na;
		/*
		 * Non-resident metadata has to be wiped globally,
		 * because its logical blocks may be larger than
		 * a cluster and split over two extents.
		 */
		if (mft_data && !a->lowest_vcn) {
			na = ntfs_attr_open(walk->image->ni,
					AT_DATA, NULL, 0);
			if (na) {
				na->rl = rl;
				rl = (runlist_element*)NULL;
				if (!ntfs_attr_map_whole_runlist(na)) {
					copy_wipe_mft(walk->image,na->rl);
				} else
					perr_exit("Failed to map data of inode %lld",
						(long long)walk->image->ni->mft_no);
				ntfs_attr_close(na);
			} else
				perr_exit("Failed to open data of inode %lld",
					(long long)walk->image->ni->mft_no);
		}
		if (index_i30 && !a->lowest_vcn) {
			na = ntfs_attr_open(walk->image->ni,
					AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
			if (na) {
				na->rl = rl;
				rl = (runlist_element*)NULL;
				if (!ntfs_attr_map_whole_runlist(na)) {
					copy_wipe_i30(walk->image,na->rl);
				} else
					perr_exit("Failed to map index of inode %lld",
						(long long)walk->image->ni->mft_no);
				ntfs_attr_close(na);
			} else
				perr_exit("Failed to open index of inode %lld",
					(long long)walk->image->ni->mft_no);
		}
	}
	if (opt.metadata
	    && (opt.metadata_image || !wipe)
	    && (walk->image->ni->mft_no == FILE_LogFile)
	    && (walk->image->ctx->attr->type == AT_DATA))
		clone_logfile_parts(walk->image, rl);

	free(rl);
}


static void walk_attributes(struct ntfs_walk_cluster *walk)
{
	ntfs_attr_search_ctx *ctx;

	if (!(ctx = ntfs_attr_get_search_ctx(walk->image->ni, NULL)))
		perr_exit("ntfs_get_attr_search_ctx");

	while (!ntfs_attrs_walk(ctx)) {
		if (ctx->attr->type == AT_END)
			break;

		walk->image->ctx = ctx;
		walk_runs(walk);
	}

	ntfs_attr_put_search_ctx(ctx);
}

/*
 *		Compare the actual bitmap to the list of clusters
 *	allocated to identified files.
 *
 *	Clusters found in use, though not marked in the bitmap are copied
 *	if the option --ignore-fs-checks is set.
 */

static int compare_bitmaps(struct bitmap *a, BOOL copy)
{
	s64 i, pos, count;
	int mismatch = 0;
	int more_use = 0;
	s64 new_cl;
	u8 bm[NTFS_BUF_SIZE];

	Printf("Accounting clusters ...\n");

	pos = 0;
	new_cl = 0;
	while (1) {
		count = ntfs_attr_pread(vol->lcnbmp_na, pos, NTFS_BUF_SIZE, bm);
		if (count == -1)
			perr_exit("Couldn't get $Bitmap $DATA");

		if (count == 0) {
			/* the backup bootsector need not be accounted for */
			if (((vol->nr_clusters + 7) >> 3) > pos)
				err_exit("$Bitmap size is smaller than expected"
					 " (%lld < %lld)\n",
					(long long)pos, (long long)a->size);
			break;
		}

		for (i = 0; i < count; i++, pos++) {
			s64 cl;  /* current cluster */

			if (a->size <= pos)
				goto done;

			if (a->bm[pos] == bm[i])
				continue;

			for (cl = pos * 8; cl < (pos + 1) * 8; cl++) {
				char bit;

				bit = ntfs_bit_get(a->bm, cl);
				if (bit == ntfs_bit_get(bm, i * 8 + cl % 8))
					continue;

				if (!bit)
					more_use++;
				if (opt.ignore_fs_check && !bit && copy) {
					lseek_to_cluster(cl);
					if (opt.save_image
					   || (opt.metadata
						&& opt.metadata_image)) {
						gap_to_cluster(cl - new_cl);
						new_cl = cl + 1;
					}
					copy_cluster(opt.rescue, cl, cl);
				}

				if (++mismatch > 10)
					continue;

				Printf("Cluster accounting failed at %lld "
				       "(0x%llx): %s cluster in $Bitmap\n",
				       (long long)cl, (unsigned long long)cl,
				       bit ? "missing" : "extra");
			}
		}
	}
done:
	if (mismatch) {
		Printf("Totally %d cluster accounting mismatches.\n", mismatch);
		if (opt.ignore_fs_check) {
			Printf("WARNING: The NTFS inconsistency was overruled "
			       "by the --ignore-fs-check option.\n");
			if (new_cl) {
				gap_to_cluster(-new_cl);
			}
			return (more_use);
		}
		err_exit("Filesystem check failed! Windows wasn't shutdown "
			 "properly or inconsistent\nfilesystem. Please run "
			 "chkdsk /f on Windows then reboot it TWICE.\n");
	}
	return (more_use);
}


static void mft_record_write_with_same_usn(ntfs_volume *volume, ntfs_inode *ni)
{
	if (ntfs_mft_usn_dec(ni->mrec))
		perr_exit("ntfs_mft_usn_dec");

	if (ntfs_mft_record_write(volume, ni->mft_no, ni->mrec))
		perr_exit("ntfs_mft_record_write");
}

static void mft_inode_write_with_same_usn(ntfs_volume *volume, ntfs_inode *ni)
{
	s32 i;

	mft_record_write_with_same_usn(volume, ni);

	if (ni->nr_extents <= 0)
		return;

	for (i = 0; i < ni->nr_extents; ++i) {
		ntfs_inode *eni = ni->extent_nis[i];
		mft_record_write_with_same_usn(volume, eni);
	}
}

static int walk_clusters(ntfs_volume *volume, struct ntfs_walk_cluster *walk)
{
	s64 inode = 0;
	s64 last_mft_rec;
	u64 nr_clusters;
	ntfs_inode *ni;
	struct progress_bar progress;

	if (opt.restore_image || (!opt.metadata && wipe))
		err_exit("Bug : invalid walk_clusters()\n");
	Printf("Scanning volume ...\n");

	last_mft_rec = (volume->mft_na->initialized_size >>
			volume->mft_record_size_bits) - 1;
	walk->image->current_lcn = 0;
	progress_init(&progress, inode, last_mft_rec, 100);

	NVolSetNoFixupWarn(volume);
	for (; inode <= last_mft_rec; inode++) {

		int err, deleted_inode;
		MFT_REF mref = (MFT_REF)inode;

		progress_update(&progress, inode);

		/* FIXME: Terrible kludge for libntfs not being able to return
		   a deleted MFT record as inode */
		ni = ntfs_calloc(sizeof(ntfs_inode));
		if (!ni)
			perr_exit("walk_clusters");

		ni->vol = volume;

		err = ntfs_file_record_read(volume, mref, &ni->mrec, NULL);
		if (err == -1) {
			free(ni);
			continue;
		}

		deleted_inode = !(ni->mrec->flags & MFT_RECORD_IN_USE);

		if (deleted_inode && !opt.metadata_image) {

			ni->mft_no = MREF(mref);
			if (wipe) {
				wipe_unused_mft(ni);
				wipe_unused_mft_data(ni);
				mft_record_write_with_same_usn(volume, ni);
			}
		}

		free(ni->mrec);
		free(ni);

		if (deleted_inode)
			continue;

		if ((ni = ntfs_inode_open(volume, mref)) == NULL) {
			/* FIXME: continue only if it make sense, e.g.
			   MFT record not in use based on $MFT bitmap */
			if (errno == EIO || errno == ENOENT)
				continue;
			perr_exit("Reading inode %lld failed",
				(long long)inode);
		}

		if (wipe)
			nr_used_mft_records++;

		if (ni->mrec->base_mft_record)
			goto out;

		walk->image->ni = ni;
		walk_attributes(walk);
out:
		if (wipe && !opt.metadata_image) {
			int i;

			wipe_unused_mft_data(ni);
			for (i = 0; i < ni->nr_extents; ++i) {
				wipe_unused_mft_data(ni->extent_nis[i]);
			}
			mft_inode_write_with_same_usn(volume, ni);
		}

		if (ntfs_inode_close(ni))
			perr_exit("ntfs_inode_close for inode %lld",
				(long long)inode);
	}
	if (opt.metadata) {
		if (opt.metadata_image && wipe && opt.ignore_fs_check) {
			gap_to_cluster(-walk->image->current_lcn);
			compare_bitmaps(&lcn_bitmap, TRUE);
			walk->image->current_lcn = 0;
		}
		if (opt.metadata_image ? wipe : !wipe) {
				/* also get the backup bootsector */
			nr_clusters = vol->nr_clusters;
			lseek_to_cluster(nr_clusters);
			if (opt.metadata_image && wipe)
				gap_to_cluster(nr_clusters
					- walk->image->current_lcn);
			copy_cluster(opt.rescue, nr_clusters, nr_clusters);
			walk->image->current_lcn = nr_clusters;
		}
			/* Not counted, for compatibility with older versions */
		if (!opt.metadata_image)
			walk->image->inuse++;
	}
	return 0;
}


/*
 * $Bitmap can overlap the end of the volume. Any bits in this region
 * must be set. This region also encompasses the backup boot sector.
 */
static void bitmap_file_data_fixup(s64 cluster, struct bitmap *bm)
{
	for (; cluster < bm->size << 3; cluster++)
		ntfs_bit_set(bm->bm, (u64)cluster, 1);
}


/*
 * Allocate a block of memory with one bit for each cluster of the disk.
 * All the bits are set to 0, except those representing the region beyond the
 * end of the disk.
 */
static void setup_lcn_bitmap(void)
{
	/* Determine lcn bitmap byte size and allocate it. */
	/* include the alternate boot sector in the bitmap count */
	lcn_bitmap.size = rounded_up_division(vol->nr_clusters + 1, 8);

	lcn_bitmap.bm = ntfs_calloc(lcn_bitmap.size);
	if (!lcn_bitmap.bm)
		perr_exit("Failed to allocate internal buffer");

	bitmap_file_data_fixup(vol->nr_clusters, &lcn_bitmap);
}


static s64 volume_size(ntfs_volume *volume, s64 nr_clusters)
{
	return nr_clusters * volume->cluster_size;
}


static void print_volume_size(const char *str, s64 bytes)
{
	Printf("%s: %lld bytes (%lld MB)\n", str, (long long)bytes,
			(long long)rounded_up_division(bytes, NTFS_MBYTE));
}


static void print_disk_usage(const char *spacer, u32 cluster_size,
		s64 nr_clusters, s64 inuse)
{
	s64 total, used;

	total = nr_clusters * cluster_size;
	used = inuse * cluster_size;

	Printf("Space in use       %s: %lld MB (%.1f%%)   ", spacer,
			(long long)rounded_up_division(used, NTFS_MBYTE),
			100.0 * ((float)used / total));

	Printf("\n");
}

static void print_image_info(void)
{
	Printf("Ntfsclone image version: %d.%d\n",
			image_hdr.major_ver, image_hdr.minor_ver);
	Printf("Cluster size           : %u bytes\n",
			(unsigned)le32_to_cpu(image_hdr.cluster_size));
	print_volume_size("Image volume size      ",
			sle64_to_cpu(image_hdr.nr_clusters) *
			le32_to_cpu(image_hdr.cluster_size));
	Printf("Image device size      : %lld bytes\n",
			(long long)le64_to_cpu(image_hdr.device_size));
	print_disk_usage("    ", le32_to_cpu(image_hdr.cluster_size),
			sle64_to_cpu(image_hdr.nr_clusters),
			le64_to_cpu(image_hdr.inuse));
	Printf("Offset to image data   : %u (0x%x) bytes\n",
			(unsigned)le32_to_cpu(image_hdr.offset_to_image_data),
			(unsigned)le32_to_cpu(image_hdr.offset_to_image_data));
}

static void check_if_mounted(const char *device, unsigned long new_mntflag)
{
	unsigned long mntflag;

	if (ntfs_check_if_mounted(device, &mntflag))
		perr_exit("Failed to check '%s' mount state", device);

	if (mntflag & NTFS_MF_MOUNTED) {
		if (!(mntflag & NTFS_MF_READONLY))
			err_exit("Device '%s' is mounted read-write. "
				 "You must 'umount' it first.\n", device);
		if (!new_mntflag)
			err_exit("Device '%s' is mounted. "
				 "You must 'umount' it first.\n", device);
	}
}

/**
 * mount_volume -
 *
 * First perform some checks to determine if the volume is already mounted, or
 * is dirty (Windows wasn't shutdown properly).  If everything is OK, then mount
 * the volume (load the metadata into memory).
 */
static void mount_volume(unsigned long new_mntflag)
{
	check_if_mounted(opt.volume, new_mntflag);

	if (!(vol = ntfs_mount(opt.volume, new_mntflag))) {

		int err = errno;

		perr_printf("Opening '%s' as NTFS failed", opt.volume);
		if (err == EINVAL) {
			Printf("Apparently device '%s' doesn't have a "
			       "valid NTFS. Maybe you selected\nthe whole "
			       "disk instead of a partition (e.g. /dev/hda, "
			       "not /dev/hda1)?\n", opt.volume);
		}
		/*
		 * Retry with recovering the log file enabled.
		 * Normally avoided in order to get the original log file
		 * data, but needed when remounting the metadata of a
		 * volume improperly unmounted from Windows.
		 * If the full log file was requested, it must be kept
		 * as is, so we just remount read-only.
		 */
		if (!(new_mntflag & (NTFS_MNT_RDONLY | NTFS_MNT_RECOVER))) {
			if (opt.full_logfile) {
				Printf("Retrying read-only to ignore"
							" the log file...\n");
				vol = ntfs_mount(opt.volume,
					 new_mntflag | NTFS_MNT_RDONLY);
			} else {
				Printf("Trying to recover...\n");
				vol = ntfs_mount(opt.volume,
					new_mntflag | NTFS_MNT_RECOVER);
			}
			Printf("... %s\n",(vol ? "Successful" : "Failed"));
		}
		if (!vol)
			exit(1);
	}

	if (vol->flags & VOLUME_IS_DIRTY)
		if (opt.force-- <= 0)
			err_exit(dirty_volume_msg, opt.volume);

	if (NTFS_MAX_CLUSTER_SIZE < vol->cluster_size)
		err_exit("Cluster size %u is too large!\n",
				(unsigned int)vol->cluster_size);

	Printf("NTFS volume version: %d.%d\n", vol->major_ver, vol->minor_ver);
	if (ntfs_version_is_supported(vol))
		perr_exit("Unknown NTFS version");

	Printf("Cluster size       : %u bytes\n",
			(unsigned int)vol->cluster_size);
	print_volume_size("Current volume size",
			  volume_size(vol, vol->nr_clusters));
}

static struct ntfs_walk_cluster backup_clusters = { NULL, NULL };

static int device_offset_valid(int fd, s64 ofs)
{
	char ch;

	if (lseek(fd, ofs, SEEK_SET) >= 0 && read(fd, &ch, 1) == 1)
		return 0;
	return -1;
}

static s64 device_size_get(int fd)
{
	s64 high, low;
#ifdef BLKGETSIZE64
	{	u64 size;

		if (ioctl(fd, BLKGETSIZE64, &size) >= 0) {
			ntfs_log_debug("BLKGETSIZE64 nr bytes = %llu "
				"(0x%llx).\n", (unsigned long long)size,
				(unsigned long long)size);
			return (s64)size;
		}
	}
#endif
#ifdef BLKGETSIZE
	{	unsigned long size;

		if (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			ntfs_log_debug("BLKGETSIZE nr 512 byte blocks = %lu "
				"(0x%lx).\n", size, size);
			return (s64)size * 512;
		}
	}
#endif
#ifdef FDGETPRM
	{       struct floppy_struct this_floppy;

		if (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			ntfs_log_debug("FDGETPRM nr 512 byte blocks = %lu "
				"(0x%lx).\n", this_floppy.size,
				this_floppy.size);
			return (s64)this_floppy.size * 512;
		}
	}
#endif
	/*
	 * We couldn't figure it out by using a specialized ioctl,
	 * so do binary search to find the size of the device.
	 */
	low = 0LL;
	for (high = 1024LL; !device_offset_valid(fd, high); high <<= 1)
		low = high;
	while (low < high - 1LL) {
		const s64 mid = (low + high) / 2;

		if (!device_offset_valid(fd, mid))
			low = mid;
		else
			high = mid;
	}
	lseek(fd, 0LL, SEEK_SET);
	return (low + 1LL);
}

static void fsync_clone(int fd)
{
	Printf("Syncing ...\n");
	if (opt.save_image && stream_out && fflush(stream_out))
		perr_exit("fflush");
	if (fsync(fd) && errno != EINVAL)
		perr_exit("fsync");
}

static void set_filesize(s64 filesize)
{
#ifndef NO_STATFS
	long fs_type = 0; /* Unknown filesystem type */

	if (fstatfs(fd_out, &opt.stfs) == -1)
		Printf("WARNING: Couldn't get filesystem type: "
		       "%s\n", strerror(errno));
	else
		fs_type = opt.stfs.f_type;

	if (fs_type == 0x52654973)
		Printf("WARNING: You're using ReiserFS, it has very poor "
		       "performance creating\nlarge sparse files. The next "
		       "operation might take a very long time!\n"
		       "Creating sparse output file ...\n");
	else if (fs_type == 0x517b)
		Printf("WARNING: You're using SMBFS and if the remote share "
		       "isn't Samba but a Windows\ncomputer then the clone "
		       "operation will be very inefficient and may fail!\n");
#endif

	if (!opt.no_action && (ftruncate(fd_out, filesize) == -1)) {
		int err = errno;
		perr_printf("ftruncate failed for file '%s'", opt.output);
#ifndef NO_STATFS
		if (fs_type)
			Printf("Destination filesystem type is 0x%lx.\n",
			       (unsigned long)fs_type);
#endif
		if (err == E2BIG) {
			Printf("Your system or the destination filesystem "
			       "doesn't support large files.\n");
#ifndef NO_STATFS
			if (fs_type == 0x517b) {
				Printf("SMBFS needs minimum Linux kernel "
				       "version 2.4.25 and\n the 'lfs' option"
				       "\nfor smbmount to have large "
				       "file support.\n");
			}
#endif
		} else if (err == EPERM) {
			Printf("Apparently the destination filesystem doesn't "
			       "support sparse files.\nYou can overcome this "
			       "by using the more efficient --save-image "
			       "option\nof ntfsclone. Use the --restore-image "
			       "option to restore the image.\n");
		}
		exit(1);
	}
		/*
		 * If truncate just created a sparse file, the ability
		 * to generically store big files has been checked, but no
		 * space has been reserved and available space has probably
		 * not been checked. Better reset the file so that we write
		 * sequentially to the end.
		 */
	if (!opt.no_action) {
#ifdef HAVE_WINDOWS_H
		if (ftruncate(fd_out, 0))
			Printf("Failed to reset the output file.\n");
#else
		struct stat st;
		int s;

		s = fstat(fd_out, &st);
		if (s || (!st.st_blocks && ftruncate(fd_out, 0)))
			Printf("Failed to reset the output file.\n");
#endif
			/* Proceed even if ftruncate failed */
	}
}

static s64 open_image(void)
{
	if (strcmp(opt.volume, "-") == 0) {
		if ((fd_in = fileno(stdin)) == -1)
			perr_exit("fileno for stdin failed");
#ifdef HAVE_WINDOWS_H
		if (setmode(fd_in,O_BINARY) == -1)
			perr_exit("setting binary stdin failed");
#endif
	} else {
		if ((fd_in = open(opt.volume, O_RDONLY | O_BINARY)) == -1)
			perr_exit("failed to open image");
	}
	if (read_all(&fd_in, &image_hdr, NTFSCLONE_IMG_HEADER_SIZE_OLD) == -1)
		perr_exit("read_all");
	if (memcmp(image_hdr.magic, IMAGE_MAGIC, IMAGE_MAGIC_SIZE) != 0)
		err_exit("Input file is not an image! (invalid magic)\n");
	if (image_hdr.major_ver < NTFSCLONE_IMG_VER_MAJOR_ENDIANNESS_SAFE) {
		image_hdr.major_ver = NTFSCLONE_IMG_VER_MAJOR;
		image_hdr.minor_ver = NTFSCLONE_IMG_VER_MINOR;
#if (__BYTE_ORDER == __BIG_ENDIAN)
		/*
		 * old image read on a big endian computer,
		 * assuming it was created big endian and read cpu-wise,
		 * so we should translate to little endian
		 */
		Printf("Old image format detected.  If the image was created "
				"on a little endian architecture it will not "
				"work.  Use a more recent version of "
				"ntfsclone to recreate the image.\n");
		image_hdr.cluster_size = cpu_to_le32(image_hdr.cluster_size);
		image_hdr.device_size = cpu_to_sle64(image_hdr.device_size);
		image_hdr.nr_clusters = cpu_to_sle64(image_hdr.nr_clusters);
		image_hdr.inuse = cpu_to_sle64(image_hdr.inuse);
#endif
		image_hdr.offset_to_image_data =
				const_cpu_to_le32((sizeof(image_hdr)
				    + IMAGE_HDR_ALIGN - 1) & -IMAGE_HDR_ALIGN);
		image_is_host_endian = TRUE;
	} else {
			/* safe image : little endian data */
		le32 offset_to_image_data;
		int delta;

		if (image_hdr.major_ver > NTFSCLONE_IMG_VER_MAJOR)
			err_exit("Do not know how to handle image format "
					"version %d.%d.  Please obtain a "
					"newer version of ntfsclone.\n",
					image_hdr.major_ver,
					image_hdr.minor_ver);
		/* Read the image header data offset. */
		if (read_all(&fd_in, &offset_to_image_data,
				sizeof(offset_to_image_data)) == -1)
			perr_exit("read_all");
			/* do not translate little endian data */
		image_hdr.offset_to_image_data = offset_to_image_data;
		/*
		 * Read any fields from the header that we have not read yet so
		 * that the input stream is positioned correctly.  This means
		 * we can support future minor versions that just extend the
		 * header in a backwards compatible way.
		 */
		delta = le32_to_cpu(offset_to_image_data)
				- (NTFSCLONE_IMG_HEADER_SIZE_OLD +
				sizeof(image_hdr.offset_to_image_data));
		if (delta > 0) {
			char *dummy_buf;

			dummy_buf = malloc(delta);
			if (!dummy_buf)
				perr_exit("malloc dummy_buffer");
			if (read_all(&fd_in, dummy_buf, delta) == -1)
				perr_exit("read_all");
			free(dummy_buf);
		}
	}
	return le64_to_cpu(image_hdr.device_size);
}

static s64 open_volume(void)
{
	s64 device_size;

	mount_volume(NTFS_MNT_RDONLY);

	device_size = ntfs_device_size_get(vol->dev, 1);
	if (device_size <= 0)
		err_exit("Couldn't get device size (%lld)!\n",
			(long long)device_size);

	print_volume_size("Current device size", device_size);

	if (device_size < vol->nr_clusters * vol->cluster_size)
		err_exit("Current NTFS volume size is bigger than the device "
			 "size (%lld)!\nCorrupt partition table or incorrect "
			 "device partitioning?\n", (long long)device_size);

	return device_size;
}

static void initialise_image_hdr(s64 device_size, s64 inuse)
{
	memcpy(image_hdr.magic, IMAGE_MAGIC, IMAGE_MAGIC_SIZE);
	image_hdr.major_ver = NTFSCLONE_IMG_VER_MAJOR;
	image_hdr.minor_ver = NTFSCLONE_IMG_VER_MINOR;
	image_hdr.cluster_size = cpu_to_le32(vol->cluster_size);
	image_hdr.device_size = cpu_to_le64(device_size);
	image_hdr.nr_clusters = cpu_to_sle64(vol->nr_clusters);
	image_hdr.inuse = cpu_to_le64(inuse);
	image_hdr.offset_to_image_data = cpu_to_le32((sizeof(image_hdr)
			 + IMAGE_HDR_ALIGN - 1) & -IMAGE_HDR_ALIGN);
}

static void check_output_device(s64 input_size)
{
	if (opt.blkdev_out) {
		s64 dest_size;

		if (dev_out)
			dest_size = ntfs_device_size_get(dev_out, 1);
		else
			dest_size = device_size_get(fd_out);
		if (dest_size < input_size)
			err_exit("Output device is too small (%lld) to fit the "
				 "NTFS image (%lld).\n",
				(long long)dest_size, (long long)input_size);

		check_if_mounted(opt.output, 0);
	} else
		set_filesize(input_size);
}

static void ignore_bad_clusters(ntfs_walk_clusters_ctx *image)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	runlist *rl;
	s64 nr_bad_clusters = 0;
	static le16 Bad[4] = {
		const_cpu_to_le16('$'), const_cpu_to_le16('B'),
		const_cpu_to_le16('a'), const_cpu_to_le16('d')
	} ;

	if (!(ni = ntfs_inode_open(vol, FILE_BadClus)))
		perr_exit("ntfs_open_inode");

	na = ntfs_attr_open(ni, AT_DATA, Bad, 4);
	if (!na)
		perr_exit("ntfs_attr_open");
	if (ntfs_attr_map_whole_runlist(na))
		perr_exit("ntfs_attr_map_whole_runlist");

	for (rl = na->rl; rl->length; rl++) {
		s64 lcn = rl->lcn;

		if (lcn == LCN_HOLE || lcn < 0)
			continue;

		for (; lcn < rl->lcn + rl->length; lcn++, nr_bad_clusters++) {
			if (ntfs_bit_get_and_set(lcn_bitmap.bm, lcn, 0))
				image->inuse--;
		}
	}
	if (nr_bad_clusters)
		Printf("WARNING: The disk has %lld or more bad sectors"
		       " (hardware faults).\n", (long long)nr_bad_clusters);
	ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		perr_exit("ntfs_inode_close failed for $BadClus");
}

static void check_dest_free_space(u64 src_bytes)
{
#ifndef HAVE_WINDOWS_H
	u64 dest_bytes;
	struct statvfs stvfs;
	struct stat st;

	if (opt.metadata || opt.blkdev_out || opt.std_out)
		return;
	/*
	 * TODO: save_image needs a bit more space than src_bytes
	 * due to the free space encoding overhead.
	 */
	if (fstatvfs(fd_out, &stvfs) == -1) {
		Printf("WARNING: Unknown free space on the destination: %s\n",
		       strerror(errno));
		return;
	}
	
	/* If file is a FIFO then there is no point in checking the size. */
	if (!fstat(fd_out, &st)) {
		if (S_ISFIFO(st.st_mode))
			return;
	} else
		Printf("WARNING: fstat failed: %s\n", strerror(errno));

	dest_bytes = (u64)stvfs.f_frsize * stvfs.f_bfree;
	if (!dest_bytes)
		dest_bytes = (u64)stvfs.f_bsize * stvfs.f_bfree;

	if (dest_bytes < src_bytes)
		err_exit("Destination doesn't have enough free space: "
			 "%llu MB < %llu MB\n",
			 (unsigned long long)rounded_up_division(dest_bytes, NTFS_MBYTE),
			 (unsigned long long)rounded_up_division(src_bytes,  NTFS_MBYTE));
#endif
}

int main(int argc, char **argv)
{
	ntfs_walk_clusters_ctx image;
	s64 device_size;        /* input device size in bytes */
	s64 ntfs_size;
	unsigned int wiped_total = 0;

	/* make sure the layout of header is not affected by alignments */
	if (offsetof(struct image_hdr, offset_to_image_data)
			!= IMAGE_OFFSET_OFFSET) {
		fprintf(stderr,"ntfsclone is not compiled properly. "
				"Please fix\n");
		exit(1);
	}
	/* print to stderr, stdout can be an NTFS image ... */
	fprintf(stderr, "%s v%s (libntfs-3g)\n", EXEC_NAME, VERSION);
	msg_out = stderr;

	parse_options(argc, argv);

	utils_set_locale();

	if (opt.restore_image) {
		device_size = open_image();
		ntfs_size = sle64_to_cpu(image_hdr.nr_clusters) *
				le32_to_cpu(image_hdr.cluster_size);
	} else {
		device_size = open_volume();
		ntfs_size = vol->nr_clusters * vol->cluster_size;
	}
	// FIXME: This needs to be the cluster size...
	ntfs_size += 512; /* add backup boot sector */
	full_device_size = device_size;

	if (opt.std_out) {
		if ((fd_out = fileno(stdout)) == -1)
			perr_exit("fileno for stdout failed");
		stream_out = stdout;
#ifdef HAVE_WINDOWS_H
		if (setmode(fileno(stdout),O_BINARY) == -1)
			perr_exit("setting binary stdout failed");
#endif
	} else {
		/* device_size_get() might need to read() */
		int flags = O_RDWR | O_BINARY;

		fd_out = 0;
		if (!opt.blkdev_out) {
			flags |= O_CREAT | O_TRUNC;
			if (!opt.overwrite)
				flags |= O_EXCL;
		}

		if (opt.save_image || opt.metadata_image) {
			stream_out = fopen(opt.output,BINWMODE);
			if (!stream_out)
				perr_exit("Opening file '%s' failed",
						opt.output);
			fd_out = fileno(stream_out);
		} else {
#ifdef HAVE_WINDOWS_H
			if (!opt.no_action) {
				dev_out = ntfs_device_alloc(opt.output, 0,
					&ntfs_device_default_io_ops, NULL);
				if (!dev_out
				    || (dev_out->d_ops->open)(dev_out, flags))
					perr_exit("Opening volume '%s' failed",
							opt.output);
			}
#else
			if (!opt.no_action
			    && ((fd_out = open(opt.output, flags,
						S_IRUSR | S_IWUSR)) == -1))
				perr_exit("Opening file '%s' failed",
						opt.output);
#endif
		}

		if (!opt.save_image && !opt.metadata_image && !opt.no_action)
			check_output_device(ntfs_size);
	}

	if (opt.restore_image) {
		print_image_info();
		restore_image();
		if (!opt.no_action)
			fsync_clone(fd_out);
		exit(0);
	}

	setup_lcn_bitmap();
	memset(&image, 0, sizeof(image));
	backup_clusters.image = &image;

	walk_clusters(vol, &backup_clusters);
	image.more_use = compare_bitmaps(&lcn_bitmap,
				opt.metadata && !opt.metadata_image);
	print_disk_usage("", vol->cluster_size, vol->nr_clusters, image.inuse);

	check_dest_free_space(vol->cluster_size * image.inuse);

	ignore_bad_clusters(&image);

	if (opt.save_image)
		initialise_image_hdr(device_size, image.inuse);

	if ((opt.std_out && !opt.metadata_image) || !opt.metadata) {
		s64 nr_clusters_to_save = image.inuse;
		if (opt.std_out && !opt.save_image)
			nr_clusters_to_save = vol->nr_clusters;
		nr_clusters_to_save++; /* account for the backup boot sector */

		clone_ntfs(nr_clusters_to_save, image.more_use);
		fsync_clone(fd_out);
		if (opt.save_image)
			fclose(stream_out);
		ntfs_umount(vol,FALSE);
		free(lcn_bitmap.bm);
		exit(0);
	}

	wipe = 1;
	if (opt.metadata_image) {
		initialise_image_hdr(device_size, image.inuse);
		write_image_hdr();
	} else {
		if (dev_out) {
			(dev_out->d_ops->close)(dev_out);
			dev_out = NULL;
		} else
			fsync_clone(fd_out); /* sync copy before mounting */
		opt.volume = opt.output;
	/* 'force' again mount for dirty volumes (e.g. after resize).
	   FIXME: use mount flags to avoid potential side-effects in future */
		opt.force++;
		ntfs_umount(vol,FALSE);
		mount_volume(0 /*NTFS_MNT_NOATIME*/);
	}

	free(lcn_bitmap.bm);
	setup_lcn_bitmap();
	memset(&image, 0, sizeof(image));
	backup_clusters.image = &image;

	walk_clusters(vol, &backup_clusters);

	Printf("Num of MFT records       = %10lld\n",
			(long long)vol->mft_na->initialized_size >>
			vol->mft_record_size_bits);
	Printf("Num of used MFT records  = %10u\n", nr_used_mft_records);

	Printf("Wiped unused MFT data    = %10u\n", wiped_unused_mft_data);
	Printf("Wiped deleted MFT data   = %10u\n", wiped_unused_mft);
	Printf("Wiped resident user data = %10u\n", wiped_resident_data);
	Printf("Wiped timestamp data     = %10u\n", wiped_timestamp_data);

	wiped_total += wiped_unused_mft_data;
	wiped_total += wiped_unused_mft;
	wiped_total += wiped_resident_data;
	wiped_total += wiped_timestamp_data;
	Printf("Wiped totally            = %10u\n", wiped_total);

	if (opt.metadata_image)
		fclose(stream_out);
	else
		fsync_clone(fd_out);
	ntfs_umount(vol,FALSE);
	free(lcn_bitmap.bm);
	return (0);
}
