/**
 * ntfsresize - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2002-2006 Szabolcs Szakacsits
 * Copyright (c) 2002-2005 Anton Altaparmakov
 * Copyright (c) 2002-2003 Richard Russon
 * Copyright (c) 2007      Yura Pakhuchiy
 * Copyright (c) 2011-2018 Jean-Pierre Andre
 *
 * This utility will resize an NTFS volume without data loss.
 *
 * WARNING FOR DEVELOPERS!!! Several external tools grep for text messages
 * to control execution thus if you would like to change any message
 * then PLEASE think twice before doing so then don't modify it. Thanks!
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "param.h"
#include "debug.h"
#include "types.h"
#include "support.h"
#include "endians.h"
#include "bootsect.h"
#include "device.h"
#include "attrib.h"
#include "volume.h"
#include "mft.h"
#include "bitmap.h"
#include "inode.h"
#include "runlist.h"
#include "utils.h"
/* #include "version.h" */
#include "misc.h"

#define BAN_NEW_TEXT 1	/* Respect the ban on new messages */
#define CLEAN_EXIT 0	/* traditionnally volume is not closed, there must be a reason */

static const char *EXEC_NAME = "ntfsresize";

static const char *resize_warning_msg =
"WARNING: Every sanity check passed and only the dangerous operations left.\n"
"Make sure that important data has been backed up! Power outage or computer\n"
"crash may result major data loss!\n";

static const char *resize_important_msg =
"You can go on to shrink the device for example with Linux fdisk.\n"
"IMPORTANT: When recreating the partition, make sure that you\n"
"  1)  create it at the same disk sector (use sector as the unit!)\n"
"  2)  create it with the same partition type (usually 7, HPFS/NTFS)\n"
"  3)  do not make it smaller than the new NTFS filesystem size\n"
"  4)  set the bootable flag for the partition if it existed before\n"
"Otherwise you won't be able to access NTFS or can't boot from the disk!\n"
"If you make a mistake and don't have a partition table backup then you\n"
"can recover the partition table by TestDisk or Parted's rescue mode.\n";

static const char *invalid_ntfs_msg =
"The device '%s' doesn't have a valid NTFS.\n"
"Maybe you selected the wrong partition? Or the whole disk instead of a\n"
"partition (e.g. /dev/hda, not /dev/hda1)? This error might also occur\n"
"if the disk was incorrectly repartitioned (see the ntfsresize FAQ).\n";

static const char *corrupt_volume_msg =
"NTFS is inconsistent. Run chkdsk /f on Windows then reboot it TWICE!\n"
"The usage of the /f parameter is very IMPORTANT! No modification was\n"
"and will be made to NTFS by this software until it gets repaired.\n";

static const char *hibernated_volume_msg =
"The NTFS partition is hibernated. Windows must be resumed and turned off\n"
"properly, so resizing could be done safely.\n";

static const char *unclean_journal_msg =
"The NTFS journal file is unclean. Please shutdown Windows properly before\n"
"using this software! Note, if you have run chkdsk previously then boot\n"
"Windows again which will automatically initialize the journal correctly.\n";

static const char *opened_volume_msg =
"This software has detected that the NTFS volume is already opened by another\n"
"software thus it refuses to progress to preserve data consistency.\n";

static const char *bad_sectors_warning_msg =
"****************************************************************************\n"
"* WARNING: The disk has bad sector. This means physical damage on the disk *\n"
"* surface caused by deterioration, manufacturing faults or other reason.   *\n"
"* The reliability of the disk may stay stable or degrade fast. We suggest  *\n"
"* making a full backup urgently by running 'ntfsclone --rescue ...' then   *\n"
"* run 'chkdsk /f /r' on Windows and rebooot it TWICE! Then you can resize  *\n"
"* NTFS safely by additionally using the --bad-sectors option of ntfsresize.*\n"
"****************************************************************************\n";

static const char *many_bad_sectors_msg =
"***************************************************************************\n"
"* WARNING: The disk has many bad sectors. This means physical damage      *\n"
"* on the disk surface caused by deterioration, manufacturing faults or    *\n"
"* other reason. We suggest to get a replacement disk as soon as possible. *\n"
"***************************************************************************\n";

enum mirror_source { MIRR_OLD, MIRR_NEWMFT, MIRR_MFT };

static struct {
	int verbose;
	int debug;
	int ro_flag;
	int force;
	int info;
	int infombonly;
	int expand;
	int reliable_size;
	int show_progress;
	int badsectors;
	int check;
	s64 bytes;
	char *volume;
} opt;

struct bitmap {
	s64 size;
	u8 *bm;
};

#define NTFS_PROGBAR		0x0001
#define NTFS_PROGBAR_SUPPRESS	0x0002

struct progress_bar {
	u64 start;
	u64 stop;
	int resolution;
	int flags;
	float unit;
};

struct llcn_t {
	s64 lcn;	/* last used LCN for a "special" file/attr type */
	s64 inode;	/* inode using it */
};

#define NTFSCK_PROGBAR		0x0001

			/* runlists which have to be processed later */
struct DELAYED {
	struct DELAYED *next;
	ATTR_TYPES type;
	MFT_REF mref;
	VCN lowest_vcn;
	int name_len;
	ntfschar *attr_name;
	runlist_element *rl;
	runlist *head_rl;
} ;

typedef struct {
	ntfs_inode *ni;		     /* inode being processed */
	ntfs_attr_search_ctx *ctx;   /* inode attribute being processed */
	s64 inuse;		     /* num of clusters in use */
	int multi_ref;		     /* num of clusters referenced many times */
	int outsider;		     /* num of clusters outside the volume */
	int show_outsider;	     /* controls showing the above information */
	int flags;
	struct bitmap lcn_bitmap;
} ntfsck_t;

typedef struct {
	ntfs_volume *vol;
	ntfs_inode *ni;		     /* inode being processed */
	s64 new_volume_size;	     /* in clusters; 0 = --info w/o --size */
	MFT_REF mref;                /* mft reference */
	MFT_RECORD *mrec;            /* mft record */
	ntfs_attr_search_ctx *ctx;   /* inode attribute being processed */
	u64 relocations;	     /* num of clusters to relocate */
	s64 inuse;		     /* num of clusters in use */
	runlist mftmir_rl;	     /* $MFTMirr AT_DATA's new position */
	s64 mftmir_old;		     /* $MFTMirr AT_DATA's old LCN */
	int dirty_inode;	     /* some inode data got relocated */
	int shrink;		     /* shrink = 1, enlarge = 0 */
	s64 badclusters;	     /* num of physically dead clusters */
	VCN mft_highest_vcn;	     /* used for relocating the $MFT */
	runlist_element *new_mft_start; /* new first run for $MFT:$DATA */
	struct DELAYED *delayed_runlists; /* runlists to process later */
	struct progress_bar progress;
	struct bitmap lcn_bitmap;
	/* Temporary statistics until all case is supported */
	struct llcn_t last_mft;
	struct llcn_t last_mftmir;
	struct llcn_t last_multi_mft;
	struct llcn_t last_sparse;
	struct llcn_t last_compressed;
	struct llcn_t last_lcn;
	s64 last_unsupp;	     /* last unsupported cluster */
	enum mirror_source mirr_from;
} ntfs_resize_t;

/* FIXME: This, lcn_bitmap and pos from find_free_cluster() will make a cluster
   allocation related structure, attached to ntfs_resize_t */
static s64 max_free_cluster_range = 0;

#define NTFS_MBYTE (1000 * 1000)

/* WARNING: don't modify the text, external tools grep for it */
#define ERR_PREFIX   "ERROR"
#define PERR_PREFIX  ERR_PREFIX "(%d): "
#define NERR_PREFIX  ERR_PREFIX ": "

#define DIRTY_NONE		(0)
#define DIRTY_INODE		(1)
#define DIRTY_ATTRIB		(2)

static s64 rounded_up_division(s64 numer, s64 denom)
{
	return (numer + (denom - 1)) / denom;
}

/**
 * perr_printf
 *
 * Print an error message.
 */
__attribute__((format(printf, 1, 2)))
static void perr_printf(const char *fmt, ...)
{
	va_list ap;
	int eo = errno;

	fprintf(stdout, PERR_PREFIX, eo);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fprintf(stdout, ": %s\n", strerror(eo));
	fflush(stdout);
	fflush(stderr);
}

__attribute__((format(printf, 1, 2)))
static void err_printf(const char *fmt, ...)
{
	va_list ap;

	fprintf(stdout, NERR_PREFIX);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fflush(stdout);
	fflush(stderr);
}

/**
 * err_exit
 *
 * Print and error message and exit the program.
 */
__attribute__((noreturn))
__attribute__((format(printf, 1, 2)))
static void err_exit(const char *fmt, ...)
{
	va_list ap;

	fprintf(stdout, NERR_PREFIX);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fflush(stdout);
	fflush(stderr);
	exit(1);
}

/**
 * perr_exit
 *
 * Print and error message and exit the program
 */
__attribute__((noreturn))
__attribute__((format(printf, 1, 2)))
static void perr_exit(const char *fmt, ...)
{
	va_list ap;
	int eo = errno;

	fprintf(stdout, PERR_PREFIX, eo);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	printf(": %s\n", strerror(eo));
	fflush(stdout);
	fflush(stderr);
	exit(1);
}

/**
 * usage - Print a list of the parameters to the program
 *
 * Print a list of the parameters and options for the program.
 *
 * Return:  none
 */
__attribute__((noreturn))
static void usage(int ret)
{

	printf("\nUsage: %s [OPTIONS] DEVICE\n"
		"    Resize an NTFS volume non-destructively, safely move any data if needed.\n"
		"\n"
		"    -c, --check            Check to ensure that the device is ready for resize\n"
		"    -i, --info             Estimate the smallest shrunken size or the smallest\n"
		"                                expansion size\n"
		"    -m, --info-mb-only     Estimate the smallest shrunken size possible,\n"
		"                                output size in MB only\n"
		"    -s, --size SIZE        Resize volume to SIZE[k|M|G] bytes\n"
		"    -x, --expand           Expand to full partition\n"
		"\n"
		"    -n, --no-action        Do not write to disk\n"
		"    -b, --bad-sectors      Support disks having bad sectors\n"
		"    -f, --force            Force to progress\n"
		"    -P, --no-progress-bar  Don't show progress bar\n"
		"    -v, --verbose          More output\n"
		"    -V, --version          Display version information\n"
		"    -h, --help             Display this help\n"
#ifdef DEBUG
		"    -d, --debug            Show debug information\n"
#endif
		"\n"
		"    The options -i and -x are exclusive of option -s, and -m is exclusive\n"
		"    of option -x. If options -i, -m, -s and -x are are all omitted\n"
		"    then the NTFS volume will be enlarged to the DEVICE size.\n"
		"\n", EXEC_NAME);
	printf("%s%s", ntfs_bugs, ntfs_home);
	printf("Ntfsresize FAQ: http://linux-ntfs.sourceforge.net/info/ntfsresize.html\n");
	exit(ret);
}

/**
 * proceed_question
 *
 * Force the user to confirm an action before performing it.
 * Copy-paste from e2fsprogs
 */
static void proceed_question(void)
{
	char buf[256];
	const char *short_yes = "yY";

	fflush(stdout);
	fflush(stderr);
	printf("Are you sure you want to proceed (y/[n])? ");
	buf[0] = 0;
	if (fgets(buf, sizeof(buf), stdin)
	    && !strchr(short_yes, buf[0])) {
		printf("OK quitting. NO CHANGES have been made to your "
				"NTFS volume.\n");
		exit(1);
	}
}

/**
 * version - Print version information about the program
 *
 * Print a copyright statement and a brief description of the program.
 *
 * Return:  none
 */
static void version(void)
{
	printf("\nResize an NTFS Volume, without data loss.\n\n");
	printf("Copyright (c) 2002-2006  Szabolcs Szakacsits\n");
	printf("Copyright (c) 2002-2005  Anton Altaparmakov\n");
	printf("Copyright (c) 2002-2003  Richard Russon\n");
	printf("Copyright (c) 2007       Yura Pakhuchiy\n");
	printf("Copyright (c) 2011-2018  Jean-Pierre Andre\n");
	printf("\n%s\n%s%s", ntfs_gpl, ntfs_bugs, ntfs_home);
}

/**
 * get_new_volume_size
 *
 * Convert a user-supplied string into a size.  Without any suffix the number
 * will be assumed to be in bytes.  If the number has a suffix of k, M or G it
 * will be scaled up by 1000, 1000000, or 1000000000.
 */
static s64 get_new_volume_size(char *s)
{
	s64 size;
	char *suffix;
	int prefix_kind = 1000;

	size = strtoll(s, &suffix, 10);
	if (size <= 0 || errno == ERANGE)
		err_exit("Illegal new volume size\n");

	if (!*suffix) {
		opt.reliable_size = 1;
		return size;
	}

	if (strlen(suffix) == 2 && suffix[1] == 'i')
		prefix_kind = 1024;
	else if (strlen(suffix) > 1)
		usage(1);

	/* We follow the SI prefixes:
	   http://physics.nist.gov/cuu/Units/prefixes.html
	   http://physics.nist.gov/cuu/Units/binary.html
	   Disk partitioning tools use prefixes as,
	                       k        M          G
	   fdisk 2.11x-      2^10     2^20      10^3*2^20
	   fdisk 2.11y+     10^3     10^6       10^9
	   cfdisk           10^3     10^6       10^9
	   sfdisk            2^10     2^20
	   parted            2^10     2^20  (may change)
	   fdisk (DOS)       2^10     2^20
	*/
	/* FIXME: check for overflow */
	switch (*suffix) {
	case 'G':
		size *= prefix_kind;
		/* FALLTHRU */
	case 'M':
		size *= prefix_kind;
		/* FALLTHRU */
	case 'k':
		size *= prefix_kind;
		break;
	default:
		usage(1);
	}

	return size;
}

/**
 * parse_options - Read and validate the programs command line
 *
 * Read the command line, verify the syntax and parse the options.
 * This function is very long, but quite simple.
 *
 * Return:  1 Success
 *	    0 Error, one or more problems
 */
static int parse_options(int argc, char **argv)
{
	static const char *sopt = "-bcdfhimnPs:vVx";
	static const struct option lopt[] = {
		{ "bad-sectors",no_argument,		NULL, 'b' },
		{ "check",	no_argument,		NULL, 'c' },
#ifdef DEBUG
		{ "debug",	no_argument,		NULL, 'd' },
#endif
		{ "force",	no_argument,		NULL, 'f' },
		{ "help",	no_argument,		NULL, 'h' },
		{ "info",	no_argument,		NULL, 'i' },
		{ "info-mb-only", no_argument,		NULL, 'm' },
		{ "no-action",	no_argument,		NULL, 'n' },
		{ "no-progress-bar", no_argument,	NULL, 'P' },
		{ "size",	required_argument,	NULL, 's' },
		{ "expand",	no_argument,		NULL, 'x' },
		{ "verbose",	no_argument,		NULL, 'v' },
		{ "version",	no_argument,		NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	int c;
	int err  = 0;
	int ver  = 0;
	int help = 0;

	memset(&opt, 0, sizeof(opt));
	opt.show_progress = 1;

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (!err && !opt.volume)
				opt.volume = argv[optind-1];
			else
				err++;
			break;
		case 'b':
			opt.badsectors++;
			break;
		case 'c':
			opt.check++;
			break;
		case 'd':
			opt.debug++;
			break;
		case 'f':
			opt.force++;
			break;
		case 'h':
			help++;
			break;
		case 'i':
			opt.info++;
			break;
		case 'm':
			opt.infombonly++;
			break;
		case 'n':
			opt.ro_flag = NTFS_MNT_RDONLY;
			break;
		case 'P':
			opt.show_progress = 0;
			break;
		case 's':
			if (!err && (opt.bytes == 0))
				opt.bytes = get_new_volume_size(optarg);
			else
				err++;
			break;
		case 'v':
			opt.verbose++;
			ntfs_log_set_levels(NTFS_LOG_LEVEL_VERBOSE);
			break;
		case 'V':
			ver++;
			break;
		case 'x':
			opt.expand++;
			break;
		case '?':
		default:
			if (optopt == 's') {
				printf("Option '%s' requires an argument.\n", argv[optind-1]);
			} else {
				printf("Unknown option '%s'.\n", argv[optind-1]);
			}
			err++;
			break;
		}
	}

	if (!help && !ver) {
		if (opt.volume == NULL) {
			if (argc > 1)
				printf("You must specify exactly one device.\n");
			err++;
		}
		if (opt.info || opt.infombonly) {
			opt.ro_flag = NTFS_MNT_RDONLY;
		}
		if (opt.bytes
		    && (opt.expand || opt.info || opt.infombonly)) {
			printf(NERR_PREFIX "Options --info(-mb-only) and --expand "
					"cannot be used with --size.\n");
			usage(1);
		}
		if (opt.expand && opt.infombonly) {
			printf(NERR_PREFIX "Options --info-mb-only "
					"cannot be used with --expand.\n");
			usage(1);
		}
	}

	/* Redirect stderr to stdout, note fflush()es are essential! */
	fflush(stdout);
	fflush(stderr);
	if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
		perr_exit("Failed to redirect stderr to stdout");
	fflush(stdout);
	fflush(stderr);

#ifdef DEBUG
	if (!opt.debug)
		if (!freopen("/dev/null", "w", stderr))
			perr_exit("Failed to redirect stderr to /dev/null");
#endif

	if (ver)
		version();
	if (help || err)
		usage(err > 0);

		/* tri-state 0 : done, 1 : error, -1 : proceed */
	return (err ? 1 : (help || ver ? 0 : -1));
}

static void print_advise(ntfs_volume *vol, s64 supp_lcn)
{
	s64 old_b, new_b, freed_b, old_mb, new_mb, freed_mb;

	old_b = vol->nr_clusters * vol->cluster_size;
	old_mb = rounded_up_division(old_b, NTFS_MBYTE);

	/* Take the next supported cluster (free or relocatable)
	   plus reserve a cluster for the backup boot sector */
	supp_lcn += 2;

	if (supp_lcn > vol->nr_clusters) {
		err_printf("Very rare fragmentation type detected. "
			   "Sorry, it's not supported yet.\n"
			   "Try to defragment your NTFS, perhaps it helps.\n");
		exit(1);
	}

	new_b = supp_lcn * vol->cluster_size;
	new_mb = rounded_up_division(new_b, NTFS_MBYTE);
	freed_b = (vol->nr_clusters - supp_lcn + 1) * vol->cluster_size;
	freed_mb = freed_b / NTFS_MBYTE;

	/* WARNING: don't modify the text, external tools grep for it */
	if (!opt.infombonly)
		printf("You might resize at %lld bytes ", (long long)new_b);
	if ((new_mb * NTFS_MBYTE) < old_b) {
		if (!opt.infombonly)
			printf("or %lld MB ", (long long)new_mb);
		else
			printf("Minsize (in MB): %lld\n", (long long)new_mb);
	}

	if (!opt.infombonly) {
		printf("(freeing ");
		if (freed_mb && (old_mb - new_mb))
			printf("%lld MB", (long long)(old_mb - new_mb));
		else
		        printf("%lld bytes", (long long)freed_b);
		printf(").\n");

		printf("Please make a test run using both the -n and -s "
			"options before real resizing!\n");
	}
}

static void rl_set(runlist *rl, VCN vcn, LCN lcn, s64 len)
{
	rl->vcn = vcn;
	rl->lcn = lcn;
	rl->length = len;
}

static int rl_items(runlist *rl)
{
	int i = 0;

	while (rl[i++].length)
		;

	return i;
}

static void dump_run(runlist_element *r)
{
	ntfs_log_verbose(" %8lld  %8lld (0x%08llx)  %lld\n", (long long)r->vcn,
			 (long long)r->lcn, (long long)r->lcn,
			 (long long)r->length);
}

static void dump_runlist(runlist *rl)
{
	while (rl->length)
		dump_run(rl++);
}

/**
 * nr_clusters_to_bitmap_byte_size
 *
 * Take the number of clusters in the volume and calculate the size of $Bitmap.
 * The size must be always a multiple of 8 bytes.
 */
static s64 nr_clusters_to_bitmap_byte_size(s64 nr_clusters)
{
	s64 bm_bsize;

	bm_bsize = rounded_up_division(nr_clusters, 8);
	bm_bsize = (bm_bsize + 7) & ~7;

	return bm_bsize;
}

static void collect_resize_constraints(ntfs_resize_t *resize, runlist *rl)
{
	s64 inode, last_lcn;
	ATTR_FLAGS flags;
	ATTR_TYPES atype;
	struct llcn_t *llcn = NULL;
	int ret, supported = 0;

	last_lcn = rl->lcn + (rl->length - 1);

	inode = resize->ni->mft_no;
	flags = resize->ctx->attr->flags;
	atype = resize->ctx->attr->type;

	if ((ret = ntfs_inode_badclus_bad(inode, resize->ctx->attr)) != 0) {
		if (ret == -1)
			perr_exit("Bad sector list check failed");
		return;
	}

	if (inode == FILE_Bitmap) {
		llcn = &resize->last_lcn;
		if (atype == AT_DATA && NInoAttrList(resize->ni))
		    err_exit("Highly fragmented $Bitmap isn't supported yet.");

		supported = 1;

	} else if (NInoAttrList(resize->ni)) {
		llcn = &resize->last_multi_mft;

		if (inode != FILE_MFTMirr)
			supported = 1;

	} else if (flags & ATTR_IS_SPARSE) {
		llcn = &resize->last_sparse;
		supported = 1;

	} else if (flags & ATTR_IS_COMPRESSED) {
		llcn = &resize->last_compressed;
		supported = 1;

	} else if (inode == FILE_MFTMirr) {
		llcn = &resize->last_mftmir;
		supported = 1;

		/* Fragmented $MFTMirr DATA attribute isn't supported yet */
		if (atype == AT_DATA)
			if (rl[1].length != 0 || rl->vcn)
				supported = 0;
	} else {
		llcn = &resize->last_lcn;
		supported = 1;
	}

	if (llcn->lcn < last_lcn) {
		llcn->lcn = last_lcn;
		llcn->inode = inode;
	}

	if (supported)
		return;

	if (resize->last_unsupp < last_lcn)
		resize->last_unsupp = last_lcn;
}


static void collect_relocation_info(ntfs_resize_t *resize, runlist *rl)
{
	s64 lcn, lcn_length, start, len, inode;
	s64 new_vol_size;	/* (last LCN on the volume) + 1 */

	lcn = rl->lcn;
	lcn_length = rl->length;
	inode = resize->ni->mft_no;
	new_vol_size = resize->new_volume_size;

	if (lcn + lcn_length <= new_vol_size)
		return;

	if (inode == FILE_Bitmap && resize->ctx->attr->type == AT_DATA)
		return;

	start = lcn;
	len = lcn_length;

	if (lcn < new_vol_size) {
		start = new_vol_size;
		len = lcn_length - (new_vol_size - lcn);

		if ((!opt.info && !opt.infombonly) && (inode == FILE_MFTMirr)) {
			err_printf("$MFTMirr can't be split up yet. Please try "
				   "a different size.\n");
			print_advise(resize->vol, lcn + lcn_length - 1);
			exit(1);
		}
	}

	resize->relocations += len;

	if ((!opt.info && !opt.infombonly) || !resize->new_volume_size)
		return;

	printf("Relocation needed for inode %8lld attr 0x%x LCN 0x%08llx "
			"length %6lld\n", (long long)inode,
			(unsigned int)le32_to_cpu(resize->ctx->attr->type),
			(unsigned long long)start, (long long)len);
}

/**
 * build_lcn_usage_bitmap
 *
 * lcn_bitmap has one bit for each cluster on the disk.  Initially, lcn_bitmap
 * has no bits set.  As each attribute record is read the bits in lcn_bitmap are
 * checked to ensure that no other file already references that cluster.
 *
 * This serves as a rudimentary "chkdsk" operation.
 */
static void build_lcn_usage_bitmap(ntfs_volume *vol, ntfsck_t *fsck)
{
	s64 inode;
	ATTR_RECORD *a;
	runlist *rl;
	int i, j;
	struct bitmap *lcn_bitmap = &fsck->lcn_bitmap;

	a = fsck->ctx->attr;
	inode = fsck->ni->mft_no;

	if (!a->non_resident)
		return;

	if (!(rl = ntfs_mapping_pairs_decompress(vol, a, NULL))) {
		int err = errno;
		perr_printf("ntfs_decompress_mapping_pairs");
		if (err == EIO)
			printf("%s", corrupt_volume_msg);
		exit(1);
	}


	for (i = 0; rl[i].length; i++) {
		s64 lcn = rl[i].lcn;
		s64 lcn_length = rl[i].length;

		/* CHECKME: LCN_RL_NOT_MAPPED check isn't needed */
		if (lcn == LCN_HOLE || lcn == LCN_RL_NOT_MAPPED)
			continue;

		/* FIXME: ntfs_mapping_pairs_decompress should return error */
		if (lcn < 0 || lcn_length <= 0)
			err_exit("Corrupt runlist in inode %lld attr %x LCN "
				 "%llx length %llx\n", (long long)inode,
				 (unsigned int)le32_to_cpu(a->type),
				 (long long)lcn, (long long)lcn_length);

		for (j = 0; j < lcn_length; j++) {
			u64 k = (u64)lcn + j;

			if (k >= (u64)vol->nr_clusters) {
				long long outsiders = lcn_length - j;

				fsck->outsider += outsiders;

				if (++fsck->show_outsider <= 10 || opt.verbose)
					printf("Outside of the volume reference"
					       " for inode %lld at %lld:%lld\n",
					       (long long)inode, (long long)k,
					       (long long)outsiders);

				break;
			}

			if (ntfs_bit_get_and_set(lcn_bitmap->bm, k, 1)) {
				if (++fsck->multi_ref <= 10 || opt.verbose)
					printf("Cluster %lld is referenced "
					       "multiple times!\n",
					       (long long)k);
				continue;
			}
		}
		fsck->inuse += lcn_length;
	}
	free(rl);
}


static ntfs_attr_search_ctx *attr_get_search_ctx(ntfs_inode *ni, MFT_RECORD *mrec)
{
	ntfs_attr_search_ctx *ret;

	if ((ret = ntfs_attr_get_search_ctx(ni, mrec)) == NULL)
		perr_printf("ntfs_attr_get_search_ctx");

	return ret;
}

/**
 * walk_attributes
 *
 * For a given MFT Record, iterate through all its attributes.  Any non-resident
 * data runs will be marked in lcn_bitmap.
 */
static int walk_attributes(ntfs_volume *vol, ntfsck_t *fsck)
{
	if (!(fsck->ctx = attr_get_search_ctx(fsck->ni, NULL)))
		return -1;

	while (!ntfs_attrs_walk(fsck->ctx)) {
		if (fsck->ctx->attr->type == AT_END)
			break;
		build_lcn_usage_bitmap(vol, fsck);
	}

	ntfs_attr_put_search_ctx(fsck->ctx);
	return 0;
}

/**
 * compare_bitmaps
 *
 * Compare two bitmaps.  In this case, $Bitmap as read from the disk and
 * lcn_bitmap which we built from the MFT Records.
 */
static void compare_bitmaps(ntfs_volume *vol, struct bitmap *a)
{
	s64 i, pos, count;
	int mismatch = 0;
	int backup_boot = 0;
	u8 bm[NTFS_BUF_SIZE];

        if (!opt.infombonly)
		printf("Accounting clusters ...\n");

	pos = 0;
	while (1) {
		count = ntfs_attr_pread(vol->lcnbmp_na, pos, NTFS_BUF_SIZE, bm);
		if (count == -1)
			perr_exit("Couldn't get $Bitmap $DATA");

		if (count == 0) {
			if (a->size > pos)
				err_exit("$Bitmap size is smaller than expected"
					 " (%lld != %lld)\n",
					 (long long)a->size, (long long)pos);
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

				if (!mismatch && !bit && !backup_boot &&
						cl == vol->nr_clusters / 2) {
					/* FIXME: call also boot sector check */
					backup_boot = 1;
					printf("Found backup boot sector in "
					       "the middle of the volume.\n");
					continue;
				}

				if (++mismatch > 10 && !opt.verbose)
					continue;

				printf("Cluster accounting failed at %lld "
						"(0x%llx): %s cluster in "
						"$Bitmap\n", (long long)cl,
						(unsigned long long)cl,
						bit ? "missing" : "extra");
			}
		}
	}
done:
	if (mismatch) {
		printf("Filesystem check failed! Totally %d cluster "
		       "accounting mismatches.\n", mismatch);
		err_printf("%s", corrupt_volume_msg);
		exit(1);
	}
}

/**
 * progress_init
 *
 * Create and scale our progress bar.
 */
static void progress_init(struct progress_bar *p, u64 start, u64 stop, int flags)
{
	p->start = start;
	p->stop = stop;
	p->unit = 100.0 / (stop - start);
	p->resolution = 100;
	p->flags = flags;
}

/**
 * progress_update
 *
 * Update the progress bar and tell the user.
 */
static void progress_update(struct progress_bar *p, u64 current)
{
	float percent;

	if (!(p->flags & NTFS_PROGBAR))
		return;
	if (p->flags & NTFS_PROGBAR_SUPPRESS)
		return;

	/* WARNING: don't modify the texts, external tools grep for them */
	percent = p->unit * current;
	if (current != p->stop) {
		if ((current - p->start) % p->resolution)
			return;
		printf("%6.2f percent completed\r", percent);
	} else
		printf("100.00 percent completed\n");
	fflush(stdout);
}

static int inode_close(ntfs_inode *ni)
{
	if (ntfs_inode_close(ni)) {
		perr_printf("ntfs_inode_close for inode %llu",
			    (unsigned long long)ni->mft_no);
		return -1;
	}
	return 0;
}

/**
 * walk_inodes
 *
 * Read each record in the MFT, skipping the unused ones, and build up a bitmap
 * from all the non-resident attributes.
 */
static int build_allocation_bitmap(ntfs_volume *vol, ntfsck_t *fsck)
{
	s64 nr_mft_records, inode = 0;
	ntfs_inode *ni;
	struct progress_bar progress;
	int pb_flags = 0;	/* progress bar flags */

	/* WARNING: don't modify the text, external tools grep for it */
        if (!opt.infombonly)
		printf("Checking filesystem consistency ...\n");

	if (fsck->flags & NTFSCK_PROGBAR)
		pb_flags |= NTFS_PROGBAR;

	nr_mft_records = vol->mft_na->initialized_size >>
			vol->mft_record_size_bits;

	progress_init(&progress, inode, nr_mft_records - 1, pb_flags);

	for (; inode < nr_mft_records; inode++) {
        	if (!opt.infombonly)
			progress_update(&progress, inode);

		if ((ni = ntfs_inode_open(vol, (MFT_REF)inode)) == NULL) {
			/* FIXME: continue only if it make sense, e.g.
			   MFT record not in use based on $MFT bitmap */
			if (errno == EIO || errno == ENOENT)
				continue;
			perr_printf("Reading inode %lld failed",
					(long long)inode);
			return -1;
		}

		if (ni->mrec->base_mft_record)
			goto close_inode;

		fsck->ni = ni;
		if (walk_attributes(vol, fsck) != 0) {
			inode_close(ni);
			return -1;
		}
close_inode:
		if (inode_close(ni) != 0)
			return -1;
	}
	return 0;
}

static void build_resize_constraints(ntfs_resize_t *resize)
{
	s64 i;
	runlist *rl;

	if (!resize->ctx->attr->non_resident)
		return;

	if (!(rl = ntfs_mapping_pairs_decompress(resize->vol,
						 resize->ctx->attr, NULL)))
		perr_exit("ntfs_decompress_mapping_pairs");

	for (i = 0; rl[i].length; i++) {
		/* CHECKME: LCN_RL_NOT_MAPPED check isn't needed */
		if (rl[i].lcn == LCN_HOLE || rl[i].lcn == LCN_RL_NOT_MAPPED)
			continue;

		collect_resize_constraints(resize, rl + i);
		if (resize->shrink)
			collect_relocation_info(resize, rl + i);
	}
	free(rl);
}

static void resize_constraints_by_attributes(ntfs_resize_t *resize)
{
	if (!(resize->ctx = attr_get_search_ctx(resize->ni, NULL)))
		exit(1);

	while (!ntfs_attrs_walk(resize->ctx)) {
		if (resize->ctx->attr->type == AT_END)
			break;
		build_resize_constraints(resize);
	}

	ntfs_attr_put_search_ctx(resize->ctx);
}

static void set_resize_constraints(ntfs_resize_t *resize)
{
	s64 nr_mft_records, inode;
	ntfs_inode *ni;

        if (!opt.infombonly)
		printf("Collecting resizing constraints ...\n");

	nr_mft_records = resize->vol->mft_na->initialized_size >>
			resize->vol->mft_record_size_bits;

	for (inode = 0; inode < nr_mft_records; inode++) {

		ni = ntfs_inode_open(resize->vol, (MFT_REF)inode);
		if (ni == NULL) {
			if (errno == EIO || errno == ENOENT)
				continue;
			perr_exit("Reading inode %lld failed",
					(long long)inode);
		}

		if (ni->mrec->base_mft_record)
			goto close_inode;

		resize->ni = ni;
		resize_constraints_by_attributes(resize);
close_inode:
		if (inode_close(ni) != 0)
			exit(1);
	}
}

static void rl_fixup(runlist **rl)
{
	runlist *tmp = *rl;

	if (tmp->lcn == LCN_RL_NOT_MAPPED) {
		s64 unmapped_len = tmp->length;

		ntfs_log_verbose("Skip unmapped run at the beginning ...\n");

		if (!tmp->length)
			err_exit("Empty unmapped runlist! Please report!\n");
		(*rl)++;
		for (tmp = *rl; tmp->length; tmp++)
			tmp->vcn -= unmapped_len;
	}

	for (tmp = *rl; tmp->length; tmp++) {
		if (tmp->lcn == LCN_RL_NOT_MAPPED) {
			ntfs_log_verbose("Skip unmapped run at the end  ...\n");

			if (tmp[1].length)
				err_exit("Unmapped runlist in the middle! "
					 "Please report!\n");
			tmp->lcn = LCN_ENOENT;
			tmp->length = 0;
		}
	}
}

/*
 *		Plug a replacement (partial) runlist into full runlist
 *
 *	Returns 0 if successful
 *		-1 if failed
 */

static int replace_runlist(ntfs_attr *na, const runlist_element *reprl,
				VCN lowest_vcn)
{
	const runlist_element *prep;
	const runlist_element *pold;
	runlist_element *pnew;
	runlist_element *newrl;
	VCN nextvcn;
	s32 oldcnt, newcnt;
	s32 newsize;
	int r;

	r = -1; /* default return */
		/* allocate a new runlist able to hold both */
	oldcnt = 0;
	while (na->rl[oldcnt].length)
		oldcnt++;
	newcnt = 0;
	while (reprl[newcnt].length)
		newcnt++;
	newsize = ((oldcnt + newcnt)*sizeof(runlist_element) + 4095) & -4096;
	newrl = (runlist_element*)malloc(newsize);
	if (newrl) {
		/* copy old runs until reaching replaced ones */
		pnew = newrl;
		pold = na->rl;
		while (pold->length
		    && ((pold->vcn + pold->length)
				 <= (reprl[0].vcn + lowest_vcn))) {
			*pnew = *pold;
			pnew++;
			pold++;
		}
		/* split a possible old run partially overlapped */
		if (pold->length
		    && (pold->vcn < (reprl[0].vcn + lowest_vcn))) {
			pnew->vcn = pold->vcn;
			pnew->lcn = pold->lcn;
			pnew->length = reprl[0].vcn + lowest_vcn - pold->vcn;
			pnew++;
		}
		/* copy new runs */
		prep = reprl;
		nextvcn = prep->vcn + lowest_vcn;
		while (prep->length) {
			pnew->vcn = prep->vcn + lowest_vcn;
			pnew->lcn = prep->lcn;
			pnew->length = prep->length;
			nextvcn = pnew->vcn + pnew->length;
			pnew++;
			prep++;
		}
		/* locate the first fully replaced old run */
		while (pold->length
		    && ((pold->vcn + pold->length) <= nextvcn)) {
			pold++;
		}
		/* split a possible old run partially overlapped */
		if (pold->length
		    && (pold->vcn < nextvcn)) {
			pnew->vcn = nextvcn;
			pnew->lcn = pold->lcn + nextvcn - pold->vcn;
			pnew->length = pold->length - nextvcn + pold->vcn;
			pnew++;
		}
		/* copy old runs beyond replaced ones */
		while (pold->length) {
			*pnew = *pold;
			pnew++;
			pold++;
		}
		/* the terminator is same as the old one */
		*pnew = *pold;
		/* deallocate the old runlist and replace */
		free(na->rl);
		na->rl = newrl;
		r = 0;
	}
	return (r);
}

/*
 *		Expand the new runlist in new extent(s)
 *
 *	This implies allocating inode extents and, generally, creating
 *	an attribute list and allocating clusters for the list, and
 *	shuffle the existing attributes accordingly.
 *
 *	Sometimes the runlist being reallocated is within an extent,
 *	so we have a partial runlist to plug into an existing one
 *	whose other parts have already been processed or will have
 *	to be processed later, and we must not interfere with the
 *	processing of these parts.
 *
 *	This cannot be done on the runlist part stored in a single
 *	extent, it has to be done globally for the file.
 *
 *	We use the standard library functions, so we must wait until
 *	the new global bitmap and the new MFT bitmap are saved to
 *	disk and usable for the allocation of a new extent and creation
 *	of an attribute list.
 *
 *	Aborts if something goes wrong. There should be no data damage,
 *	because the old runlist is still in use and the bootsector has
 *	not been updated yet, so the initial clusters can be accessed.
 */

static void expand_attribute_runlist(ntfs_volume *vol, struct DELAYED *delayed)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	ATTR_TYPES type;
	MFT_REF mref;
	runlist_element *rl;

		/* open the inode */
	mref = delayed->mref;
#ifndef BAN_NEW_TEXT
	ntfs_log_verbose("Processing a delayed update for inode %lld\n",
					(long long)mref);
#endif
	type = delayed->type;
	rl = delayed->rl;

	/* The MFT inode is permanently open, do not reopen or close */
	if (mref == FILE_MFT)
		ni = vol->mft_ni;
	else
		ni = ntfs_inode_open(vol,mref);
	if (ni) {
		if (mref == FILE_MFT)
			na = (type == AT_DATA ? vol->mft_na : vol->mftbmp_na);
		else
			na = ntfs_attr_open(ni, type,
					delayed->attr_name, delayed->name_len);
		if (na) {
			/*
			 * The runlist is first updated in memory, and
			 * the updated one is used for updating on device
			 */
			if (!ntfs_attr_map_whole_runlist(na)) {
				if (replace_runlist(na,rl,delayed->lowest_vcn)
				    || ntfs_attr_update_mapping_pairs(na,0))
					perr_exit("Could not update runlist "
						"for attribute 0x%lx in inode %lld",
						(long)le32_to_cpu(type),(long long)mref);
			} else
				perr_exit("Could not map attribute 0x%lx in inode %lld",
					(long)le32_to_cpu(type),(long long)mref);
			if (mref != FILE_MFT)
				ntfs_attr_close(na);
		} else
			perr_exit("Could not open attribute 0x%lx in inode %lld",
				(long)le32_to_cpu(type),(long long)mref);
		ntfs_inode_mark_dirty(ni);
		if ((mref != FILE_MFT) && ntfs_inode_close(ni))
			perr_exit("Failed to close inode %lld through the library",
				(long long)mref);
	} else
		perr_exit("Could not open inode %lld through the library",
			(long long)mref);
}

/*
 *		Reload the MFT before merging delayed updates of runlist
 *
 *	The delayed updates of runlists are those which imply updating
 *	the runlists which overflow from their original MFT record.
 *	Such updates must be done in the new location of the MFT and
 *	the allocations must be recorded in the new location of the
 *	MFT bitmap.
 *	The MFT data and MFT bitmap may themselves have delayed parts
 *	of their runlists, and at this stage, their runlists may have
 *	been partially updated on disk, and partially to be updated.
 *	Their in-memory runlists still point at the old location, they
 *	are obsolete, and we have to read the partially updated runlist
 *	from the device before merging the delayed updates.
 *
 *	Returns 0 if successful
 *		-1 otherwise
 */

static int reload_mft(ntfs_resize_t *resize)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	int r;
	int xi;

	r = 0;
		/* get the base inode */
	ni = resize->vol->mft_ni;
	if (!ntfs_file_record_read(resize->vol, FILE_MFT, &ni->mrec, NULL)) {
		for (xi=0; !r && xi<resize->vol->mft_ni->nr_extents; xi++) {
			r = ntfs_file_record_read(resize->vol,
					ni->extent_nis[xi]->mft_no,
					&ni->extent_nis[xi]->mrec, NULL);
		}

		if (!r) {
			/* reopen the MFT bitmap, and swap vol->mftbmp_na */
			na = ntfs_attr_open(resize->vol->mft_ni,
						AT_BITMAP, NULL, 0);
			if (na && !ntfs_attr_map_whole_runlist(na)) {
				ntfs_attr_close(resize->vol->mftbmp_na);
				resize->vol->mftbmp_na = na;
			} else
				r = -1;
		}

		if (!r) {
			/* reopen the MFT data, and swap vol->mft_na */
			na = ntfs_attr_open(resize->vol->mft_ni,
						AT_DATA, NULL, 0);
			if (na && !ntfs_attr_map_whole_runlist(na)) {
				ntfs_attr_close(resize->vol->mft_na);
				resize->vol->mft_na = na;
			} else
				r = -1;
		}
	} else
		r = -1;
	return (r);
}

/*
 *		Re-record the MFT extents in MFT bitmap
 *
 *	When both MFT data and MFT bitmap have delayed runlists, MFT data
 *	is updated first, and the extents may be recorded at old location.
 */

static int record_mft_in_bitmap(ntfs_resize_t *resize)
{
	ntfs_inode *ni;
	int r;
	int xi;

	r = 0;
		/* get the base inode */
	ni = resize->vol->mft_ni;
	for (xi=0; !r && xi<resize->vol->mft_ni->nr_extents; xi++) {
		r = ntfs_bitmap_set_run(resize->vol->mftbmp_na,
					ni->extent_nis[xi]->mft_no, 1);
	}
	return (r);
}

/*
 *		Process delayed runlist updates
 */

static void delayed_updates(ntfs_resize_t *resize)
{
	struct DELAYED *delayed;
	struct DELAYED *delayed_mft_data;
	int nr_extents;

	if (ntfs_volume_get_free_space(resize->vol))
		err_exit("Failed to determine free space\n");

	delayed_mft_data = (struct DELAYED*)NULL;
	if (resize->delayed_runlists && reload_mft(resize))
		err_exit("Failed to reload the MFT for delayed updates\n");

		/*
		 * Important : updates to MFT must come first, so that
		 * the new location of MFT is used for adding needed extents.
		 * Now, there are runlists in the MFT bitmap and MFT data.
		 * Extents to MFT bitmap have to be stored in the new MFT
		 * data, and extents to MFT data have to be recorded in
		 * the MFT bitmap.
		 * So we update MFT data first, and we record the MFT
		 * extents again in the MFT bitmap if they were recorded
		 * in the old location.
		 *
		 * However, if we are operating in "no action" mode, the
		 * MFT records to update are not written to their new location
		 * and the MFT data runlist has to be updated last in order
		 * to have the entries read from their old location.
		 * In this situation the MFT bitmap is never written to
		 * disk, so the same extents are reallocated repeatedly,
		 * which is not what would be done in a real resizing.
		 */

	if (opt.ro_flag
	    && resize->delayed_runlists
	    && (resize->delayed_runlists->mref == FILE_MFT)
	    && (resize->delayed_runlists->type == AT_DATA)) {
			/* Update the MFT data runlist later */
		delayed_mft_data = resize->delayed_runlists;
		resize->delayed_runlists = resize->delayed_runlists->next;
	}

	while (resize->delayed_runlists) {
		delayed = resize->delayed_runlists;
		expand_attribute_runlist(resize->vol, delayed);
		if (delayed->mref == FILE_MFT) {
			if (delayed->type == AT_BITMAP)
				record_mft_in_bitmap(resize);
			if (delayed->type == AT_DATA)
				resize->mirr_from = MIRR_MFT;
		}
		resize->delayed_runlists = resize->delayed_runlists->next;
		if (delayed->attr_name)
			free(delayed->attr_name);
		free(delayed->head_rl);
		free(delayed);
	}
	if (opt.ro_flag && delayed_mft_data) {
		/* in "no action" mode, check updating the MFT runlist now */
		expand_attribute_runlist(resize->vol, delayed_mft_data);
		resize->mirr_from = MIRR_MFT;
		if (delayed_mft_data->attr_name)
			free(delayed_mft_data->attr_name);
		free(delayed_mft_data->head_rl);
		free(delayed_mft_data);
	}
	/* Beware of MFT fragmentation when the target size is too small */
	nr_extents = resize->vol->mft_ni->nr_extents;
	if (nr_extents > 2) {
		printf("WARNING: The MFT is now severely fragmented"
			" (%d extents)\n", nr_extents);
	}
}

/*
 *		Queue a runlist replacement for later update
 *
 *	Store the attribute identification relative to base inode
 */

static void replace_later(ntfs_resize_t *resize, runlist *rl, runlist *head_rl)
{
	struct DELAYED *delayed;
	struct DELAYED *previous;
	ATTR_RECORD *a;
	MFT_REF mref;
	leMFT_REF lemref;
	int name_len;
	ntfschar *attr_name;

		/* save the attribute parameters, to be able to find it later */
	a = resize->ctx->attr;
	name_len = a->name_length;
	attr_name = (ntfschar*)NULL;
	if (name_len) {
		attr_name = (ntfschar*)ntfs_malloc(name_len*sizeof(ntfschar));
		if (attr_name)
			memcpy(attr_name,(u8*)a + le16_to_cpu(a->name_offset),
					name_len*sizeof(ntfschar));
	}
	delayed = (struct DELAYED*)ntfs_malloc(sizeof(struct DELAYED));
	if (delayed && (attr_name || !name_len)) {
		lemref = resize->ctx->mrec->base_mft_record;
		if (lemref)
			mref = le64_to_cpu(lemref);
		else
			mref = resize->mref;
		delayed->mref = MREF(mref);
		delayed->type = a->type;
		delayed->attr_name = attr_name;
		delayed->name_len = name_len;
		delayed->lowest_vcn = sle64_to_cpu(a->lowest_vcn);
		delayed->rl = rl;
		delayed->head_rl = head_rl;
		/* Queue ahead of list if this is MFT or head is not MFT */
		if ((delayed->mref == FILE_MFT)
		    || !resize->delayed_runlists
		    || (resize->delayed_runlists->mref != FILE_MFT)) {
			delayed->next = resize->delayed_runlists;
			resize->delayed_runlists = delayed;
		} else {
			/* Queue after all MFTs is this is not MFT */
			previous = resize->delayed_runlists;
			while (previous->next
			    && (previous->next->mref == FILE_MFT))
				previous = previous->next;
			delayed->next = previous->next;
			previous->next = delayed;
		}
	} else
		perr_exit("Could not store delayed update data");
}

/*
 *		Replace the runlist in an attribute
 *
 *	This sometimes requires expanding the runlist into another extent,
 *	which has to be done globally on the attribute. Is so, the action
 *	is put in a delay queue, and the caller must not free the runlist.
 *
 *	Returns 0 if the replacement could be done
 *		1 when it has been put in the delay queue.
 */

static int replace_attribute_runlist(ntfs_resize_t *resize, runlist *rl)
{
	int mp_size, l;
	int must_delay;
	void *mp;
	runlist *head_rl;
	ntfs_volume *vol;
	ntfs_attr_search_ctx *ctx;
	ATTR_RECORD *a;

	vol = resize->vol;
	ctx = resize->ctx;
	a = ctx->attr;
	head_rl = rl;
	rl_fixup(&rl);

	if ((mp_size = ntfs_get_size_for_mapping_pairs(vol, rl, 0, INT_MAX)) == -1)
		perr_exit("ntfs_get_size_for_mapping_pairs");

	if (a->name_length) {
		u16 name_offs = le16_to_cpu(a->name_offset);
		u16 mp_offs = le16_to_cpu(a->mapping_pairs_offset);

		if (name_offs >= mp_offs)
			err_exit("Attribute name is after mapping pairs! "
				 "Please report!\n");
	}

	/* CHECKME: don't trust mapping_pairs is always the last item in the
	   attribute, instead check for the real size/space */
	l = (int)le32_to_cpu(a->length) - le16_to_cpu(a->mapping_pairs_offset);
	must_delay = 0;
	if (mp_size > l) {
		s32 remains_size;
		char *next_attr;

		ntfs_log_verbose("Enlarging attribute header ...\n");

		mp_size = (mp_size + 7) & ~7;

		ntfs_log_verbose("Old mp size      : %d\n", l);
		ntfs_log_verbose("New mp size      : %d\n", mp_size);
		ntfs_log_verbose("Bytes in use     : %u\n", (unsigned int)
				 le32_to_cpu(ctx->mrec->bytes_in_use));

		next_attr = (char *)a + le32_to_cpu(a->length);
		l = mp_size - l;

		ntfs_log_verbose("Bytes in use new : %u\n", l + (unsigned int)
				 le32_to_cpu(ctx->mrec->bytes_in_use));
		ntfs_log_verbose("Bytes allocated  : %u\n", (unsigned int)
				 le32_to_cpu(ctx->mrec->bytes_allocated));

		remains_size = le32_to_cpu(ctx->mrec->bytes_in_use);
		remains_size -= (next_attr - (char *)ctx->mrec);

		ntfs_log_verbose("increase         : %d\n", l);
		ntfs_log_verbose("shift            : %lld\n",
				 (long long)remains_size);
		if (le32_to_cpu(ctx->mrec->bytes_in_use) + l >
				le32_to_cpu(ctx->mrec->bytes_allocated)) {
#ifndef BAN_NEW_TEXT
			ntfs_log_verbose("Queuing expansion for later processing\n");
#endif
			must_delay = 1;
			replace_later(resize,rl,head_rl);
		} else {
			memmove(next_attr + l, next_attr, remains_size);
			ctx->mrec->bytes_in_use = cpu_to_le32(l +
					le32_to_cpu(ctx->mrec->bytes_in_use));
			a->length = cpu_to_le32(le32_to_cpu(a->length) + l);
		}
	}

	if (!must_delay) {
		mp = ntfs_calloc(mp_size);
		if (!mp)
			perr_exit("ntfsc_calloc couldn't get memory");

		if (ntfs_mapping_pairs_build(vol, (u8*)mp, mp_size, rl, 0, NULL))
			perr_exit("ntfs_mapping_pairs_build");

		memmove((u8*)a + le16_to_cpu(a->mapping_pairs_offset), mp, mp_size);

		free(mp);
	}
	return (must_delay);
}

static void set_bitmap_range(struct bitmap *bm, s64 pos, s64 length, u8 bit)
{
	while (length--)
		ntfs_bit_set(bm->bm, pos++, bit);
}

static void set_bitmap_clusters(struct bitmap *bm, runlist *rl, u8 bit)
{
	for (; rl->length; rl++)
		set_bitmap_range(bm, rl->lcn, rl->length, bit);
}

static void release_bitmap_clusters(struct bitmap *bm, runlist *rl)
{
	max_free_cluster_range = 0;
	set_bitmap_clusters(bm, rl, 0);
}

static void set_max_free_zone(s64 length, s64 end, runlist_element *rle)
{
	if (length > rle->length) {
		rle->lcn = end - length;
		rle->length = length;
	}
}

static int find_free_cluster(struct bitmap *bm,
			     runlist_element *rle,
			     s64 nr_vol_clusters,
			     int hint)
{
	/* FIXME: get rid of this 'static' variable */
	static s64 pos = 0;
	s64 i, items = rle->length;
	s64 free_zone = 0;

	if (pos >= nr_vol_clusters)
		pos = 0;
	if (!max_free_cluster_range)
		max_free_cluster_range = nr_vol_clusters;
	rle->lcn = rle->length = 0;
	if (hint)
		pos = nr_vol_clusters / 2;
	i = pos;

	do {
		if (!ntfs_bit_get(bm->bm, i)) {
			if (++free_zone == items) {
				set_max_free_zone(free_zone, i + 1, rle);
				break;
			}
		} else {
			set_max_free_zone(free_zone, i, rle);
			free_zone = 0;
		}
		if (++i == nr_vol_clusters) {
			set_max_free_zone(free_zone, i, rle);
			i = free_zone = 0;
		}
		if (rle->length == max_free_cluster_range)
			break;
	} while (i != pos);

	if (i)
		set_max_free_zone(free_zone, i, rle);

	if (!rle->lcn) {
		errno = ENOSPC;
		return -1;
	}
	if (rle->length < items && rle->length < max_free_cluster_range) {
		max_free_cluster_range = rle->length;
		ntfs_log_verbose("Max free range: %7lld     \n",
				 (long long)max_free_cluster_range);
	}
	pos = rle->lcn + items;
	if (pos == nr_vol_clusters)
		pos = 0;

	set_bitmap_range(bm, rle->lcn, rle->length, 1);
	return 0;
}

static runlist *alloc_cluster(struct bitmap *bm,
			      s64 items,
			      s64 nr_vol_clusters,
			      int hint)
{
	runlist_element rle;
	runlist *rl = NULL;
	int rl_size, runs = 0;
	s64 vcn = 0;

	if (items <= 0) {
		errno = EINVAL;
		return NULL;
	}

	while (items > 0) {

		if (runs)
			hint = 0;
		rle.length = items;
		if (find_free_cluster(bm, &rle, nr_vol_clusters, hint) == -1)
			return NULL;

		rl_size = (runs + 2) * sizeof(runlist_element);
		if (!(rl = (runlist *)realloc(rl, rl_size)))
			return NULL;

		rl_set(rl + runs, vcn, rle.lcn, rle.length);

		vcn += rle.length;
		items -= rle.length;
		runs++;
	}

	rl_set(rl + runs, vcn, -1LL, 0LL);

	if (runs > 1) {
		ntfs_log_verbose("Multi-run allocation:    \n");
		dump_runlist(rl);
	}
	return rl;
}

static int read_all(struct ntfs_device *dev, void *buf, int count)
{
	int i;

	while (count > 0) {

		i = count;
		if (!NDevReadOnly(dev))
			i = dev->d_ops->read(dev, buf, count);

		if (i < 0) {
			if (errno != EAGAIN && errno != EINTR)
				return -1;
		} else if (i > 0) {
			count -= i;
			buf = i + (char *)buf;
		} else
			err_exit("Unexpected end of file!\n");
	}
	return 0;
}

static int write_all(struct ntfs_device *dev, void *buf, int count)
{
	int i;

	while (count > 0) {

		i = count;
		if (!NDevReadOnly(dev))
			i = dev->d_ops->write(dev, buf, count);

		if (i < 0) {
			if (errno != EAGAIN && errno != EINTR)
				return -1;
		} else {
			count -= i;
			buf = i + (char *)buf;
		}
	}
	return 0;
}

/**
 * write_mft_record
 *
 * Write an MFT Record back to the disk.  If the read-only command line option
 * was given, this function will do nothing.
 */
static int write_mft_record(ntfs_volume *v, const MFT_REF mref, MFT_RECORD *buf)
{
	if (ntfs_mft_record_write(v, mref, buf))
		perr_exit("ntfs_mft_record_write");

//	if (v->dev->d_ops->sync(v->dev) == -1)
//		perr_exit("Failed to sync device");

	return 0;
}

static void lseek_to_cluster(ntfs_volume *vol, s64 lcn)
{
	off_t pos;

	pos = (off_t)(lcn * vol->cluster_size);

	if (vol->dev->d_ops->seek(vol->dev, pos, SEEK_SET) == (off_t)-1)
		perr_exit("seek failed to position %lld", (long long)lcn);
}

static void copy_clusters(ntfs_resize_t *resize, s64 dest, s64 src, s64 len)
{
	s64 i;
	char *buff;
	ntfs_volume *vol = resize->vol;

	buff = (char*)ntfs_malloc(vol->cluster_size);
	if (!buff)
		perr_exit("ntfs_malloc");

	for (i = 0; i < len; i++) {

		lseek_to_cluster(vol, src + i);

		if (read_all(vol->dev, buff, vol->cluster_size) == -1) {
			perr_printf("Failed to read from the disk");
			if (errno == EIO)
				printf("%s", bad_sectors_warning_msg);
			exit(1);
		}

		lseek_to_cluster(vol, dest + i);

		if (write_all(vol->dev, buff, vol->cluster_size) == -1) {
			perr_printf("Failed to write to the disk");
			if (errno == EIO)
				printf("%s", bad_sectors_warning_msg);
			exit(1);
		}

		resize->relocations++;
		progress_update(&resize->progress, resize->relocations);
	}
	free(buff);
}

static void relocate_clusters(ntfs_resize_t *r, runlist *dest_rl, s64 src_lcn)
{
	/* collect_shrink_constraints() ensured $MFTMir DATA is one run */
	if (r->mref == FILE_MFTMirr && r->ctx->attr->type == AT_DATA) {
		if (!r->mftmir_old) {
			r->mftmir_rl.lcn = dest_rl->lcn;
			r->mftmir_rl.length = dest_rl->length;
			r->mftmir_old = src_lcn;
		} else
			err_exit("Multi-run $MFTMirr. Please report!\n");
	}

	for (; dest_rl->length; src_lcn += dest_rl->length, dest_rl++)
		copy_clusters(r, dest_rl->lcn, src_lcn, dest_rl->length);
}

static void rl_split_run(runlist **rl, int run, s64 pos)
{
	runlist *rl_new, *rle_new, *rle;
	int items, new_size, size_head, size_tail;
	s64 len_head, len_tail;

	items = rl_items(*rl);
	new_size = (items + 1) * sizeof(runlist_element);
	size_head = run * sizeof(runlist_element);
	size_tail = (items - run - 1) * sizeof(runlist_element);

	rl_new = ntfs_malloc(new_size);
	if (!rl_new)
		perr_exit("ntfs_malloc");

	rle_new = rl_new + run;
	rle = *rl + run;

	memmove(rl_new, *rl, size_head);
	memmove(rle_new + 2, rle + 1, size_tail);

	len_tail = rle->length - (pos - rle->lcn);
	len_head = rle->length - len_tail;

	rl_set(rle_new, rle->vcn, rle->lcn, len_head);
	rl_set(rle_new + 1, rle->vcn + len_head, rle->lcn + len_head, len_tail);

	ntfs_log_verbose("Splitting run at cluster %lld:\n", (long long)pos);
	dump_run(rle); dump_run(rle_new); dump_run(rle_new + 1);

	free(*rl);
	*rl = rl_new;
}

static void rl_insert_at_run(runlist **rl, int run, runlist *ins)
{
	int items, ins_items;
	int new_size, size_tail;
	runlist *rle;
	s64 vcn;

	items  = rl_items(*rl);
	ins_items = rl_items(ins) - 1;
	new_size = ((items - 1) + ins_items) * sizeof(runlist_element);
	size_tail = (items - run - 1) * sizeof(runlist_element);

	if (!(*rl = (runlist *)realloc(*rl, new_size)))
		perr_exit("realloc");

	rle = *rl + run;

	memmove(rle + ins_items, rle + 1, size_tail);

	for (vcn = rle->vcn; ins->length; rle++, vcn += ins->length, ins++) {
		rl_set(rle, vcn, ins->lcn, ins->length);
//		dump_run(rle);
	}

	return;

	/* FIXME: fast path if ins_items = 1 */
//	(*rl + run)->lcn = ins->lcn;
}

static void relocate_run(ntfs_resize_t *resize, runlist **rl, int run)
{
	s64 lcn, lcn_length;
	s64 new_vol_size;	/* (last LCN on the volume) + 1 */
	runlist *relocate_rl;	/* relocate runlist to relocate_rl */
	int hint;

	lcn = (*rl + run)->lcn;
	lcn_length = (*rl + run)->length;
	new_vol_size = resize->new_volume_size;

	if (lcn + lcn_length <= new_vol_size)
		return;

	if (lcn < new_vol_size) {
		rl_split_run(rl, run, new_vol_size);
		return;
	}

	hint = (resize->mref == FILE_MFTMirr) ? 1 : 0;
	if ((resize->mref == FILE_MFT)
	    && (resize->ctx->attr->type == AT_DATA)
	    && !run
	    && resize->new_mft_start) {
		relocate_rl = resize->new_mft_start;
	} else
		if (!(relocate_rl = alloc_cluster(&resize->lcn_bitmap,
					lcn_length, new_vol_size, hint)))
			perr_exit("Cluster allocation failed for %llu:%lld",
				  (unsigned long long)resize->mref,
				  (long long)lcn_length);

	/* FIXME: check $MFTMirr DATA isn't multi-run (or support it) */
	ntfs_log_verbose("Relocate record %7llu:0x%x:%08lld:0x%08llx:0x%08llx "
			 "--> 0x%08llx\n", (unsigned long long)resize->mref,
			 (unsigned int)le32_to_cpu(resize->ctx->attr->type),
			 (long long)lcn_length,
			 (unsigned long long)(*rl + run)->vcn,
			 (unsigned long long)lcn,
			 (unsigned long long)relocate_rl->lcn);

	relocate_clusters(resize, relocate_rl, lcn);
	rl_insert_at_run(rl, run, relocate_rl);

	/* We don't release old clusters in the bitmap, that area isn't
	   used by the allocator and will be truncated later on */

		/* Do not free the relocated MFT start */
	if ((resize->mref != FILE_MFT)
	    || (resize->ctx->attr->type != AT_DATA)
	    || run
	    || !resize->new_mft_start)
		free(relocate_rl);

	resize->dirty_inode = DIRTY_ATTRIB;
}

static void relocate_attribute(ntfs_resize_t *resize)
{
	ATTR_RECORD *a;
	runlist *rl;
	int i;

	a = resize->ctx->attr;

	if (!a->non_resident)
		return;

	if (!(rl = ntfs_mapping_pairs_decompress(resize->vol, a, NULL)))
		perr_exit("ntfs_decompress_mapping_pairs");

	for (i = 0; rl[i].length; i++) {
		s64 lcn = rl[i].lcn;
		s64 lcn_length = rl[i].length;

		if (lcn == LCN_HOLE || lcn == LCN_RL_NOT_MAPPED)
			continue;

		/* FIXME: ntfs_mapping_pairs_decompress should return error */
		if (lcn < 0 || lcn_length <= 0)
			err_exit("Corrupt runlist in MTF %llu attr %x LCN "
				 "%llx length %llx\n",
				 (unsigned long long)resize->mref,
				 (unsigned int)le32_to_cpu(a->type),
				 (long long)lcn, (long long)lcn_length);

		relocate_run(resize, &rl, i);
	}

	if (resize->dirty_inode == DIRTY_ATTRIB) {
		if (!replace_attribute_runlist(resize, rl))
			free(rl);
		resize->dirty_inode = DIRTY_INODE;
	} else
		free(rl);
}

static int is_mftdata(ntfs_resize_t *resize)
{
	/*
	 * We must update the MFT own DATA record at the end of the second
	 * step, because the old MFT must be kept available for processing
	 * the other files.
	 */

	if (resize->ctx->attr->type != AT_DATA)
		return 0;

	if (resize->mref == 0)
		return 1;

	if (MREF_LE(resize->mrec->base_mft_record) == 0 &&
	    MSEQNO_LE(resize->mrec->base_mft_record) != 0)
		return 1;

	return 0;
}

static int handle_mftdata(ntfs_resize_t *resize, int do_mftdata)
{
	ATTR_RECORD *attr = resize->ctx->attr;
	VCN highest_vcn, lowest_vcn;

	if (do_mftdata) {

		if (!is_mftdata(resize))
			return 0;

		highest_vcn = sle64_to_cpu(attr->highest_vcn);
		lowest_vcn  = sle64_to_cpu(attr->lowest_vcn);

		if (resize->mft_highest_vcn != highest_vcn)
			return 0;

		if (lowest_vcn == 0)
			resize->mft_highest_vcn = lowest_vcn;
		else
			resize->mft_highest_vcn = lowest_vcn - 1;

	} else if (is_mftdata(resize)) {

		highest_vcn = sle64_to_cpu(attr->highest_vcn);

		if (resize->mft_highest_vcn < highest_vcn)
			resize->mft_highest_vcn = highest_vcn;

		return 0;
	}

	return 1;
}

static void relocate_attributes(ntfs_resize_t *resize, int do_mftdata)
{
	int ret;
	leMFT_REF lemref;
	MFT_REF base_mref;

	if (!(resize->ctx = attr_get_search_ctx(NULL, resize->mrec)))
		exit(1);

	lemref = resize->mrec->base_mft_record;
	if (lemref)
		base_mref = MREF(le64_to_cpu(lemref));
	else
		base_mref = resize->mref;
	while (!ntfs_attrs_walk(resize->ctx)) {
		if (resize->ctx->attr->type == AT_END)
			break;

		if (handle_mftdata(resize, do_mftdata) == 0)
			continue;

		ret = ntfs_inode_badclus_bad(resize->mref, resize->ctx->attr);
		if (ret == -1)
			perr_exit("Bad sector list check failed");
		else if (ret == 1)
			continue;

		if (resize->mref == FILE_Bitmap &&
		    resize->ctx->attr->type == AT_DATA)
			continue;

		/* Do not relocate bad clusters */
		if ((base_mref == FILE_BadClus)
		    && (resize->ctx->attr->type == AT_DATA))
			continue;

		relocate_attribute(resize);
	}

	ntfs_attr_put_search_ctx(resize->ctx);
}

static void relocate_inode(ntfs_resize_t *resize, MFT_REF mref, int do_mftdata)
{
	ntfs_volume *vol = resize->vol;

	if (ntfs_file_record_read(vol, mref, &resize->mrec, NULL)) {
		/* FIXME: continue only if it make sense, e.g.
		   MFT record not in use based on $MFT bitmap */
		if (errno == EIO || errno == ENOENT)
			return;
		perr_exit("ntfs_file_record_record");
	}

	if (!(resize->mrec->flags & MFT_RECORD_IN_USE))
		return;

	resize->mref = mref;
	resize->dirty_inode = DIRTY_NONE;

	relocate_attributes(resize, do_mftdata);

//		if (vol->dev->d_ops->sync(vol->dev) == -1)
//			perr_exit("Failed to sync device");
		/* relocate MFT during second step, even if not dirty */
	if ((mref == FILE_MFT) && do_mftdata && resize->new_mft_start) {
		s64 pos;

			/* write the MFT own record at its new location */
		pos = (resize->new_mft_start->lcn
					<< vol->cluster_size_bits)
			+ (FILE_MFT << vol->mft_record_size_bits);
		if (!opt.ro_flag
		    && (ntfs_mst_pwrite(vol->dev, pos, 1,
				vol->mft_record_size, resize->mrec) != 1))
			perr_exit("Couldn't update MFT own record");
	} else {
		if ((resize->dirty_inode == DIRTY_INODE)
		   && write_mft_record(vol, mref, resize->mrec)) {
			perr_exit("Couldn't update record %llu",
						(unsigned long long)mref);
		}
	}
}

static void relocate_inodes(ntfs_resize_t *resize)
{
	s64 nr_mft_records;
	MFT_REF mref;
	VCN highest_vcn;
	s64 length;

	printf("Relocating needed data ...\n");

	progress_init(&resize->progress, 0, resize->relocations, resize->progress.flags);
	resize->relocations = 0;

	resize->mrec = ntfs_malloc(resize->vol->mft_record_size);
	if (!resize->mrec)
		perr_exit("ntfs_malloc failed");

	nr_mft_records = resize->vol->mft_na->initialized_size >>
			resize->vol->mft_record_size_bits;

		/*
		 * If we need to relocate the first run of the MFT DATA,
		 * do it now, to have a better chance of getting at least
		 * 16 records in the first chunk. This is mandatory to be
		 * later able to read an MFT extent in record 15.
		 * Should this fail, we can stop with no damage, the volume
		 * is still in its initial state.
		 */
	if (!resize->vol->mft_na->rl)
		err_exit("Internal error : no runlist for $MFT\n");

	if ((resize->vol->mft_na->rl->lcn + resize->vol->mft_na->rl->length)
			>= resize->new_volume_size) {
		/*
		 * The length of the first run is normally found in
		 * mft_na. However in some rare circumstance, this is
		 * merged with the first run of an extent of MFT,
		 * which implies there is a single run in the base record.
		 * So we have to make sure not to overflow from the
		 * runs present in the base extent.
		 */
		length = resize->vol->mft_na->rl->length;
		if (ntfs_file_record_read(resize->vol, FILE_MFT,
				&resize->mrec, NULL)
		    || !(resize->ctx = attr_get_search_ctx(NULL,
				resize->mrec))) {
			err_exit("Could not read the base record of MFT\n");
		}
		while (!ntfs_attrs_walk(resize->ctx)
		   && (resize->ctx->attr->type != AT_DATA)) { }
		if (resize->ctx->attr->type == AT_DATA) {
			sle64 high_le;

			high_le = resize->ctx->attr->highest_vcn;
			if (sle64_to_cpu(high_le) < length)
				length = sle64_to_cpu(high_le) + 1;
		} else {
			err_exit("Could not find the DATA of MFT\n");
		}
		ntfs_attr_put_search_ctx(resize->ctx);
		resize->new_mft_start = alloc_cluster(&resize->lcn_bitmap,
				length, resize->new_volume_size, 0);
		if (!resize->new_mft_start
		    || (((resize->new_mft_start->length
			<< resize->vol->cluster_size_bits)
			    >> resize->vol->mft_record_size_bits) < 16)) {
			err_exit("Could not allocate 16 records in"
					" the first MFT chunk\n");
		}
		resize->mirr_from = MIRR_NEWMFT;
	}

	for (mref = 0; mref < (MFT_REF)nr_mft_records; mref++)
		relocate_inode(resize, mref, 0);

	while (1) {
		highest_vcn = resize->mft_highest_vcn;
		mref = nr_mft_records;
		do {
			relocate_inode(resize, --mref, 1);
			if (resize->mft_highest_vcn == 0)
				goto done;
		} while (mref);

		if (highest_vcn == resize->mft_highest_vcn)
			err_exit("Sanity check failed! Highest_vcn = %lld. "
				 "Please report!\n", (long long)highest_vcn);
	}
done:
	free(resize->mrec);
}

static void print_hint(ntfs_volume *vol, const char *s, struct llcn_t llcn)
{
	s64 runs_b, runs_mb;

	if (llcn.lcn == 0)
		return;

	runs_b = llcn.lcn * vol->cluster_size;
	runs_mb = rounded_up_division(runs_b, NTFS_MBYTE);
	printf("%-19s: %9lld MB      %8lld\n", s, (long long)runs_mb,
			(long long)llcn.inode);
}

/**
 * advise_on_resize
 *
 * The metadata file $Bitmap has one bit for each cluster on disk.  This has
 * already been read into lcn_bitmap.  By looking for the last used cluster on
 * the disk, we can work out by how much we can shrink the volume.
 */
static void advise_on_resize(ntfs_resize_t *resize)
{
	ntfs_volume *vol = resize->vol;

	if (opt.verbose) {
		printf("Estimating smallest shrunken size supported ...\n");
		printf("File feature         Last used at      By inode\n");
		print_hint(vol, "$MFT",		resize->last_mft);
		print_hint(vol, "Multi-Record", resize->last_multi_mft);
		print_hint(vol, "$MFTMirr",	resize->last_mftmir);
		print_hint(vol, "Compressed",	resize->last_compressed);
		print_hint(vol, "Sparse",	resize->last_sparse);
		print_hint(vol, "Ordinary",	resize->last_lcn);
	}

	print_advise(vol, resize->last_unsupp);
}

/**
 * bitmap_file_data_fixup
 *
 * $Bitmap can overlap the end of the volume. Any bits in this region
 * must be set. This region also encompasses the backup boot sector.
 */
static void bitmap_file_data_fixup(s64 cluster, struct bitmap *bm)
{
	for (; cluster < bm->size << 3; cluster++)
		ntfs_bit_set(bm->bm, (u64)cluster, 1);
}

/*
 *		Open the attribute $BadClust:$Bad and get its runlist
 */

static ntfs_attr *open_badclust_bad_attr(ntfs_attr_search_ctx *ctx)
{
	ntfs_inode *base_ni;
	ntfs_attr *na;
	static ntfschar Bad[4] = {
		const_cpu_to_le16('$'), const_cpu_to_le16('B'),
		const_cpu_to_le16('a'), const_cpu_to_le16('d')
	} ;

	base_ni = ctx->base_ntfs_ino;
	if (!base_ni)
		base_ni = ctx->ntfs_ino;

	na = ntfs_attr_open(base_ni, AT_DATA, Bad, 4);
	if (!na) {
		err_printf("Could not access the bad sector list\n");
	} else {
		if (ntfs_attr_map_whole_runlist(na) || !na->rl) {
			err_printf("Could not decode the bad sector list\n");
			ntfs_attr_close(na);
			ntfs_inode_close(base_ni);
			na = (ntfs_attr*)NULL;
		}
	}
	return (na);
}

/**
 * truncate_badclust_bad_attr
 *
 * The metadata file $BadClus needs to be shrunk.
 *
 * FIXME: this function should go away and instead using a generalized
 * "truncate_bitmap_data_attr()"
 */
static void truncate_badclust_bad_attr(ntfs_resize_t *resize)
{
	ntfs_inode *base_ni;
	ntfs_attr *na;
	ntfs_attr_search_ctx *ctx;
	s64 nr_clusters = resize->new_volume_size;
	ntfs_volume *vol = resize->vol;

	na = open_badclust_bad_attr(resize->ctx);
	if (!na) {
		err_printf("Could not access the bad sector list\n");
		exit(1);
	}
	base_ni = na->ni;
	if (ntfs_attr_truncate(na,nr_clusters << vol->cluster_size_bits)) {
		err_printf("Could not adjust the bad sector list\n");
		exit(1);
	}
		/* Clear the sparse flags, even if there are bad clusters */
	na->ni->flags &= ~FILE_ATTR_SPARSE_FILE;
	na->data_flags &= ~ATTR_IS_SPARSE;
	ctx = resize->ctx;
	ctx->attr->data_size = cpu_to_sle64(na->data_size);
	ctx->attr->initialized_size = cpu_to_sle64(na->initialized_size);
	ctx->attr->flags = na->data_flags;
	ctx->attr->compression_unit = 0;
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	NInoFileNameSetDirty(na->ni);
	NInoFileNameSetDirty(na->ni);

	ntfs_attr_close(na);
	ntfs_inode_mark_dirty(base_ni);
}

/**
 * realloc_bitmap_data_attr
 *
 * Reallocate the metadata file $Bitmap.  It must be large enough for one bit
 * per cluster of the shrunken volume.  Also it must be a of 8 bytes in size.
 */
static void realloc_bitmap_data_attr(ntfs_resize_t *resize,
				     runlist **rl,
				     s64 nr_bm_clusters)
{
	s64 i;
	ntfs_volume *vol = resize->vol;
	ATTR_RECORD *a = resize->ctx->attr;
	s64 new_size = resize->new_volume_size;
	struct bitmap *bm = &resize->lcn_bitmap;

	if (!(*rl = ntfs_mapping_pairs_decompress(vol, a, NULL)))
		perr_exit("ntfs_mapping_pairs_decompress");

	release_bitmap_clusters(bm, *rl);
	free(*rl);

	for (i = vol->nr_clusters; i < new_size; i++)
		ntfs_bit_set(bm->bm, i, 0);

	if (!(*rl = alloc_cluster(bm, nr_bm_clusters, new_size, 0)))
		perr_exit("Couldn't allocate $Bitmap clusters");
}

static void realloc_lcn_bitmap(ntfs_resize_t *resize, s64 bm_bsize)
{
	u8 *tmp;

	if (!(tmp = realloc(resize->lcn_bitmap.bm, bm_bsize)))
		perr_exit("realloc");

	resize->lcn_bitmap.bm = tmp;
	resize->lcn_bitmap.size = bm_bsize;
	bitmap_file_data_fixup(resize->new_volume_size, &resize->lcn_bitmap);
}

/**
 * truncate_bitmap_data_attr
 */
static void truncate_bitmap_data_attr(ntfs_resize_t *resize)
{
	ATTR_RECORD *a;
	runlist *rl;
	ntfs_attr *lcnbmp_na;
	s64 bm_bsize, size;
	s64 nr_bm_clusters;
	int truncated;
	ntfs_volume *vol = resize->vol;

	a = resize->ctx->attr;
	if (!a->non_resident)
		/* FIXME: handle resident attribute value */
		err_exit("Resident attribute in $Bitmap isn't supported!\n");

	bm_bsize = nr_clusters_to_bitmap_byte_size(resize->new_volume_size);
	nr_bm_clusters = rounded_up_division(bm_bsize, vol->cluster_size);

	if (resize->shrink) {
		realloc_bitmap_data_attr(resize, &rl, nr_bm_clusters);
		realloc_lcn_bitmap(resize, bm_bsize);
	} else {
		realloc_lcn_bitmap(resize, bm_bsize);
		realloc_bitmap_data_attr(resize, &rl, nr_bm_clusters);
	}
		/*
		 * Delayed relocations may require cluster allocations
		 * through the library, to hold added attribute lists,
		 * be sure they will be within the new limits.
		 */
	lcnbmp_na = resize->vol->lcnbmp_na;
	lcnbmp_na->data_size = bm_bsize;
	lcnbmp_na->initialized_size = bm_bsize;
	lcnbmp_na->allocated_size = nr_bm_clusters << vol->cluster_size_bits;
	vol->lcnbmp_ni->data_size = bm_bsize;
	vol->lcnbmp_ni->allocated_size = lcnbmp_na->allocated_size;
	a->highest_vcn = cpu_to_sle64(nr_bm_clusters - 1LL);
	a->allocated_size = cpu_to_sle64(nr_bm_clusters * vol->cluster_size);
	a->data_size = cpu_to_sle64(bm_bsize);
	a->initialized_size = cpu_to_sle64(bm_bsize);

	truncated = !replace_attribute_runlist(resize, rl);

	/*
	 * FIXME: update allocated/data sizes and timestamps in $FILE_NAME
	 * attribute too, for now chkdsk will do this for us.
	 */

	size = ntfs_rl_pwrite(vol, rl, 0, 0, bm_bsize, resize->lcn_bitmap.bm);
	if (bm_bsize != size) {
		if (size == -1)
			perr_exit("Couldn't write $Bitmap");
		err_exit("Couldn't write full $Bitmap file (%lld from %lld)\n",
				(long long)size, (long long)bm_bsize);
	}

	if (truncated) {
			/* switch to the new bitmap runlist */
		free(lcnbmp_na->rl);
		lcnbmp_na->rl = rl;
	}
}

/**
 * lookup_data_attr
 *
 * Find the $DATA attribute (with or without a name) for the given MFT reference
 * (inode number).
 */
static void lookup_data_attr(ntfs_volume *vol,
			     MFT_REF mref,
			     const char *aname,
			     ntfs_attr_search_ctx **ctx)
{
	ntfs_inode *ni;
	ntfschar *ustr;
	int len = 0;

	if (!(ni = ntfs_inode_open(vol, mref)))
		perr_exit("ntfs_open_inode");

	if (!(*ctx = attr_get_search_ctx(ni, NULL)))
		exit(1);

	if ((ustr = ntfs_str2ucs(aname, &len)) == NULL) {
		perr_printf("Couldn't convert '%s' to Unicode", aname);
		exit(1);
	}

	if (ntfs_attr_lookup(AT_DATA, ustr, len, CASE_SENSITIVE,
			 0, NULL, 0, *ctx))
		perr_exit("ntfs_lookup_attr");

	ntfs_ucsfree(ustr);
}

static void close_inode_and_context(ntfs_attr_search_ctx *ctx)
{
	ntfs_inode *ni;

	ni = ctx->base_ntfs_ino;
	if (!ni)
		ni = ctx->ntfs_ino;
	ntfs_attr_put_search_ctx(ctx);
	if (ni)
		ntfs_inode_close(ni);
}

static int check_bad_sectors(ntfs_volume *vol)
{
	ntfs_attr_search_ctx *ctx;
	ntfs_attr *na;
	runlist *rl;
	s64 i, badclusters = 0;

	ntfs_log_verbose("Checking for bad sectors ...\n");

	lookup_data_attr(vol, FILE_BadClus, "$Bad", &ctx);

	na = open_badclust_bad_attr(ctx);
	if (!na) {
		err_printf("Could not access the bad sector list\n");
		exit(1);
	}
	rl = na->rl;
	for (i = 0; rl[i].length; i++) {
		/* CHECKME: LCN_RL_NOT_MAPPED check isn't needed */
		if (rl[i].lcn == LCN_HOLE || rl[i].lcn == LCN_RL_NOT_MAPPED)
			continue;

		badclusters += rl[i].length;
		ntfs_log_verbose("Bad cluster: %#8llx - %#llx    (%lld)\n",
				 (long long)rl[i].lcn,
				 (long long)rl[i].lcn + rl[i].length - 1,
				 (long long)rl[i].length);
	}

	if (badclusters) {
		printf("%sThis software has detected that the disk has at least"
		       " %lld bad sector%s.\n",
		       !opt.badsectors ? NERR_PREFIX : "WARNING: ",
		       (long long)badclusters, badclusters - 1 ? "s" : "");
		if (!opt.badsectors) {
			printf("%s", bad_sectors_warning_msg);
			exit(1);
		} else
			printf("WARNING: Bad sectors can cause reliability "
			       "problems and massive data loss!!!\n");
	}

	ntfs_attr_close(na);
#if CLEAN_EXIT
	close_inode_and_context(ctx);
#else
	ntfs_attr_put_search_ctx(ctx);
#endif

	return badclusters;
}

/**
 * truncate_badclust_file
 *
 * Shrink the $BadClus file to match the new volume size.
 */
static void truncate_badclust_file(ntfs_resize_t *resize)
{
	printf("Updating $BadClust file ...\n");

	lookup_data_attr(resize->vol, FILE_BadClus, "$Bad", &resize->ctx);
	/* FIXME: sanity_check_attr(ctx->attr); */
	resize->mref = FILE_BadClus;
	truncate_badclust_bad_attr(resize);

	close_inode_and_context(resize->ctx);
}

/**
 * truncate_bitmap_file
 *
 * Shrink the $Bitmap file to match the new volume size.
 */
static void truncate_bitmap_file(ntfs_resize_t *resize)
{
	ntfs_volume *vol = resize->vol;

	printf("Updating $Bitmap file ...\n");

	lookup_data_attr(resize->vol, FILE_Bitmap, NULL, &resize->ctx);
	resize->mref = FILE_Bitmap;
	truncate_bitmap_data_attr(resize);

	if (resize->new_mft_start) {
		s64 pos;

			/* write the MFT record at its new location */
		pos = (resize->new_mft_start->lcn << vol->cluster_size_bits)
			+ (FILE_Bitmap << vol->mft_record_size_bits);
		if (!opt.ro_flag
		    && (ntfs_mst_pwrite(vol->dev, pos, 1,
				vol->mft_record_size, resize->ctx->mrec) != 1))
			perr_exit("Couldn't update $Bitmap at new location");
	} else {
		if (write_mft_record(vol, resize->ctx->ntfs_ino->mft_no,
			     resize->ctx->mrec))
			perr_exit("Couldn't update $Bitmap");
	}

		/* If successful, update cache and sync $Bitmap */
	memcpy(vol->lcnbmp_ni->mrec,resize->ctx->mrec,vol->mft_record_size);
	ntfs_inode_mark_dirty(vol->lcnbmp_ni);
	NInoFileNameSetDirty(vol->lcnbmp_ni);
	ntfs_inode_sync(vol->lcnbmp_ni);

#if CLEAN_EXIT
	close_inode_and_context(resize->ctx);
#else
	ntfs_attr_put_search_ctx(resize->ctx);
#endif
}

/**
 * setup_lcn_bitmap
 *
 * Allocate a block of memory with one bit for each cluster of the disk.
 * All the bits are set to 0, except those representing the region beyond the
 * end of the disk.
 */
static int setup_lcn_bitmap(struct bitmap *bm, s64 nr_clusters)
{
	/* Determine lcn bitmap byte size and allocate it. */
	bm->size = rounded_up_division(nr_clusters, 8);

	bm->bm = ntfs_calloc(bm->size);
	if (!bm->bm)
		return -1;

	bitmap_file_data_fixup(nr_clusters, bm);
	return 0;
}

/**
 * update_bootsector
 *
 * FIXME: should be done using ntfs_* functions
 */
static void update_bootsector(ntfs_resize_t *r)
{
	NTFS_BOOT_SECTOR *bs;
	ntfs_volume *vol = r->vol;
	s64  bs_size = vol->sector_size;

	printf("Updating Boot record ...\n");

	bs = (NTFS_BOOT_SECTOR*)ntfs_malloc(vol->sector_size);
	if (!bs)
		perr_exit("ntfs_malloc");

	if (vol->dev->d_ops->seek(vol->dev, 0, SEEK_SET) == (off_t)-1)
		perr_exit("lseek");

	if (vol->dev->d_ops->read(vol->dev, bs, bs_size) == -1)
		perr_exit("read() error");

	if (bs->bpb.sectors_per_cluster > 128)
		bs->number_of_sectors = cpu_to_sle64(r->new_volume_size
				<< (256 - bs->bpb.sectors_per_cluster));
	else
		bs->number_of_sectors = cpu_to_sle64(r->new_volume_size *
				bs->bpb.sectors_per_cluster);

	if (r->mftmir_old || (r->mirr_from == MIRR_MFT)) {
		r->progress.flags |= NTFS_PROGBAR_SUPPRESS;
		/* Be sure the MFTMirr holds the updated MFT runlist */
		switch (r->mirr_from) {
		case MIRR_MFT :
			/* The late updates of MFT have not been synced */
			ntfs_inode_sync(vol->mft_ni);
			copy_clusters(r, r->mftmir_rl.lcn,
				vol->mft_na->rl->lcn, r->mftmir_rl.length);
			break;
		case MIRR_NEWMFT :
			copy_clusters(r, r->mftmir_rl.lcn,
				 r->new_mft_start->lcn, r->mftmir_rl.length);
			break;
		default :
			copy_clusters(r, r->mftmir_rl.lcn, r->mftmir_old,
				      r->mftmir_rl.length);
			break;
		}
		if (r->mftmir_old)
			bs->mftmirr_lcn = cpu_to_sle64(r->mftmir_rl.lcn);
		r->progress.flags &= ~NTFS_PROGBAR_SUPPRESS;
	}
		/* Set the start of the relocated MFT */
	if (r->new_mft_start) {
		bs->mft_lcn = cpu_to_sle64(r->new_mft_start->lcn);
			/* no more need for the new MFT start */
		free(r->new_mft_start);
		r->new_mft_start = (runlist_element*)NULL;
	}

	if (vol->dev->d_ops->seek(vol->dev, 0, SEEK_SET) == (off_t)-1)
		perr_exit("lseek");

	if (!opt.ro_flag)
		if (vol->dev->d_ops->write(vol->dev, bs, bs_size) == -1)
			perr_exit("write() error");
		/*
		 * Set the backup boot sector, if the target size is
		 * either not defined or is defined with no multiplier
		 * suffix and is a multiple of the sector size.
		 * With these conditions we can be confident enough that
		 * the partition size is already defined or it will be
		 * later defined with the same exact value.
		 */
	if (!opt.ro_flag && opt.reliable_size
	    && !(opt.bytes % vol->sector_size)) {
		if (vol->dev->d_ops->seek(vol->dev, opt.bytes
				- vol->sector_size, SEEK_SET) == (off_t)-1)
			perr_exit("lseek");
		if (vol->dev->d_ops->write(vol->dev, bs, bs_size) == -1)
			perr_exit("write() error");
	}
	free(bs);
}

/**
 * vol_size
 */
static s64 vol_size(ntfs_volume *v, s64 nr_clusters)
{
	/* add one sector_size for the backup boot sector */
	return nr_clusters * v->cluster_size + v->sector_size;
}

/**
 * print_vol_size
 *
 * Print the volume size in bytes and decimal megabytes.
 */
static void print_vol_size(const char *str, s64 bytes)
{
	printf("%s: %lld bytes (%lld MB)\n", str, (long long)bytes,
			(long long)rounded_up_division(bytes, NTFS_MBYTE));
}

/**
 * print_disk_usage
 *
 * Display the amount of disk space in use.
 */
static void print_disk_usage(ntfs_volume *vol, s64 nr_used_clusters)
{
	s64 total, used;

	total = vol->nr_clusters * vol->cluster_size;
	used = nr_used_clusters * vol->cluster_size;

	/* WARNING: don't modify the text, external tools grep for it */
        if (!opt.infombonly) {
		printf("Space in use       : %lld MB (%.1f%%)\n",
	        	(long long)rounded_up_division(used, NTFS_MBYTE),
	        	100.0 * ((float)used / total));
	}
}

static void print_num_of_relocations(ntfs_resize_t *resize)
{
	s64 relocations = resize->relocations * resize->vol->cluster_size;

	printf("Needed relocations : %lld (%lld MB)\n",
			(long long)resize->relocations, (long long)
			rounded_up_division(relocations, NTFS_MBYTE));
}

static ntfs_volume *check_volume(void)
{
	ntfs_volume *myvol = NULL;

	/*
	 * Pass NTFS_MNT_FORENSIC so that the mount process does not modify the
	 * volume at all.  We will do the logfile emptying and dirty setting
	 * later if needed.
	 */
	if (!(myvol = ntfs_mount(opt.volume, opt.ro_flag | NTFS_MNT_FORENSIC)))
	{
		int err = errno;

		perr_printf("Opening '%s' as NTFS failed", opt.volume);
		switch (err) {
		case EINVAL :
			printf(invalid_ntfs_msg, opt.volume);
			break;
		case EIO :
			printf("%s", corrupt_volume_msg);
			break;
		case EPERM :
			printf("%s", hibernated_volume_msg);
			break;
		case EOPNOTSUPP :
			printf("%s", unclean_journal_msg);
			break;
		case EBUSY :
			printf("%s", opened_volume_msg);
			break;
		default :
			break;
		}
		exit(1);
	}
	return myvol;
}


/**
 * mount_volume
 *
 * First perform some checks to determine if the volume is already mounted, or
 * is dirty (Windows wasn't shutdown properly).  If everything is OK, then mount
 * the volume (load the metadata into memory).
 */
static ntfs_volume *mount_volume(void)
{
	unsigned long mntflag;
	ntfs_volume *vol = NULL;

	if (ntfs_check_if_mounted(opt.volume, &mntflag)) {
		perr_printf("Failed to check '%s' mount state", opt.volume);
		printf("Probably /etc/mtab is missing. It's too risky to "
		       "continue. You might try\nan another Linux distro.\n");
		exit(1);
	}
	if (mntflag & NTFS_MF_MOUNTED) {
		if (!(mntflag & NTFS_MF_READONLY))
			err_exit("Device '%s' is mounted read-write. "
				 "You must 'umount' it first.\n", opt.volume);
		if (!opt.ro_flag)
			err_exit("Device '%s' is mounted. "
				 "You must 'umount' it first.\n", opt.volume);
	}
	vol = check_volume();

	if (vol->flags & VOLUME_IS_DIRTY)
		if (opt.force-- <= 0)
			err_exit("Volume is scheduled for check.\nRun chkdsk /f"
				 " and please try again, or see option -f.\n");

	if (NTFS_MAX_CLUSTER_SIZE < vol->cluster_size)
		err_exit("Cluster size %u is too large!\n",
			(unsigned int)vol->cluster_size);

	if (ntfs_volume_get_free_space(vol))
		err_exit("Failed to update the free space\n");

	if (!opt.infombonly) {
		printf("Device name        : %s\n", opt.volume);
		printf("NTFS volume version: %d.%d\n",
				vol->major_ver, vol->minor_ver);
	}
	if (ntfs_version_is_supported(vol))
		perr_exit("Unknown NTFS version");

	if (!opt.infombonly) {
		printf("Cluster size       : %u bytes\n",
			(unsigned int)vol->cluster_size);
		print_vol_size("Current volume size",
			vol_size(vol, vol->nr_clusters));
	}

	return vol;
}

/**
 * prepare_volume_fixup
 *
 * Set the volume's dirty flag and wipe the filesystem journal.  When Windows
 * boots it will automatically run chkdsk to check for any problems.  If the
 * read-only command line option was given, this function will do nothing.
 */
static void prepare_volume_fixup(ntfs_volume *vol)
{
	printf("Schedule chkdsk for NTFS consistency check at Windows boot "
			"time ...\n");
	vol->flags |= VOLUME_IS_DIRTY;
	if (ntfs_volume_write_flags(vol, vol->flags))
		perr_exit("Failed to set the volume dirty");

	/* Porting note: This flag does not exist in libntfs-3g. The dirty flag
	 * is never modified by libntfs-3g on unmount and we set it above. We
	 * can safely comment out this statement. */
	/* NVolSetWasDirty(vol); */

	if (vol->dev->d_ops->sync(vol->dev) == -1)
		perr_exit("Failed to sync device");
	printf("Resetting $LogFile ... (this might take a while)\n");
	if (ntfs_logfile_reset(vol))
		perr_exit("Failed to reset $LogFile");
	if (vol->dev->d_ops->sync(vol->dev) == -1)
		perr_exit("Failed to sync device");
}

static void set_disk_usage_constraint(ntfs_resize_t *resize)
{
	/* last lcn for a filled up volume (no empty space) */
	s64 last = resize->inuse - 1;

	if (resize->last_unsupp < last)
		resize->last_unsupp = last;
}

static void check_resize_constraints(ntfs_resize_t *resize)
{
	s64 new_size = resize->new_volume_size;

	/* FIXME: resize.shrink true also if only -i is used */
	if (!resize->shrink)
		return;

	if (resize->inuse == resize->vol->nr_clusters)
		err_exit("Volume is full. To shrink it, "
			 "delete unused files.\n");

	if (opt.info || opt.infombonly)
		return;

	/* FIXME: reserve some extra space so Windows can boot ... */
	if (new_size < resize->inuse)
		err_exit("New size can't be less than the space already"
			 " occupied by data.\nYou either need to delete unused"
			 " files or see the -i option.\n");

	if (new_size <= resize->last_unsupp)
		err_exit("The fragmentation type, you have, isn't "
			 "supported yet. Rerun ntfsresize\nwith "
			 "the -i option to estimate the smallest "
			 "shrunken volume size supported.\n");

	print_num_of_relocations(resize);
}

static void check_cluster_allocation(ntfs_volume *vol, ntfsck_t *fsck)
{
	memset(fsck, 0, sizeof(ntfsck_t));

	if (opt.show_progress)
		fsck->flags |= NTFSCK_PROGBAR;

	if (setup_lcn_bitmap(&fsck->lcn_bitmap, vol->nr_clusters) != 0)
		perr_exit("Failed to setup allocation bitmap");
	if (build_allocation_bitmap(vol, fsck) != 0)
		exit(1);
	if (fsck->outsider || fsck->multi_ref) {
		err_printf("Filesystem check failed!\n");
		if (fsck->outsider)
			err_printf("%d clusters are referenced outside "
				   "of the volume.\n", fsck->outsider);
		if (fsck->multi_ref)
			err_printf("%d clusters are referenced multiple"
				   " times.\n", fsck->multi_ref);
		printf("%s", corrupt_volume_msg);
		exit(1);
	}

	compare_bitmaps(vol, &fsck->lcn_bitmap);
}

/*
 *		Following are functions to expand an NTFS file system
 *	to the beginning of a partition. The old metadata can be
 *	located according to the backup bootsector, provided it can
 *	still be found at the end of the partition.
 *
 *	The data itself is kept in place, and this is only possible
 *	if the expanded size is a multiple of cluster size, and big
 *	enough to hold the new $Boot, $Bitmap and $MFT
 *
 *	The volume cannot be mounted because the layout of data does
 *	not match the volume parameters. The alignments of MFT entries
 *	and index blocks may be different in the new volume and the old
 *	one. The "ntfs_volume" structure is only partially usable,
 *	"ntfs_inode" and "search_context" cannot be used until the
 *	metadata has been moved and the volume is opened.
 *
 *	Currently, no part of this new code is called from old code,
 *	and the only change in old code is the processing of options.
 *	Deduplication of code should be done later when the code is
 *	proved safe.
 *
 */

typedef struct EXPAND {
	ntfs_volume *vol;
	u64 original_sectors;
	u64 new_sectors;
	u64 bitmap_allocated;
	u64 bitmap_size;
	u64 boot_size;
	u64 mft_size;
	LCN mft_lcn;
	s64 byte_increment;
	s64 sector_increment;
	s64 cluster_increment;
	u8 *bitmap;
	u8 *mft_bitmap;
	char *bootsector;
	MFT_RECORD *mrec;
	struct progress_bar *progress;
	struct DELAYED *delayed_runlists; /* runlists to process later */
} expand_t;

/*
 *		Locate an attribute in an MFT record
 *
 *	Returns NULL if not found (with no error message)
 */

static ATTR_RECORD *find_attr(MFT_RECORD *mrec, ATTR_TYPES type,
					ntfschar *name, int namelen)
{
	ATTR_RECORD *a;
	u32 offset;
	ntfschar *attrname;

			/* fetch the requested attribute */
	offset = le16_to_cpu(mrec->attrs_offset);
	a = (ATTR_RECORD*)((char*)mrec + offset);
	attrname = (ntfschar*)((char*)a + le16_to_cpu(a->name_offset));
	while ((a->type != AT_END)
	    && ((a->type != type)
		|| (a->name_length != namelen)
		|| (namelen && memcmp(attrname,name,2*namelen)))
	    && (offset < le32_to_cpu(mrec->bytes_in_use))) {
		offset += le32_to_cpu(a->length);
		a = (ATTR_RECORD*)((char*)mrec + offset);
		if (namelen)
			attrname = (ntfschar*)((char*)a
				+ le16_to_cpu(a->name_offset));
	}
	if ((a->type != type)
	    || (a->name_length != namelen)
	    || (namelen && memcmp(attrname,name,2*namelen)))
		a = (ATTR_RECORD*)NULL;
	return (a);
}

/*
 *		Read an MFT record and find an unnamed attribute
 *
 *	Returns NULL if fails to read or attribute is not found
 */

static ATTR_RECORD *get_unnamed_attr(expand_t *expand, ATTR_TYPES type,
							s64 inum)
{
	ntfs_volume *vol;
	ATTR_RECORD *a;
	MFT_RECORD *mrec;
	s64 pos;
	BOOL found;
	int got;

	found = FALSE;
	a = (ATTR_RECORD*)NULL;
	mrec = expand->mrec;
	vol = expand->vol;
	pos = (vol->mft_lcn << vol->cluster_size_bits)
		+ (inum << vol->mft_record_size_bits)
		+ expand->byte_increment;
	got = ntfs_mst_pread(vol->dev, pos, 1, vol->mft_record_size, mrec);
	if ((got == 1) && (mrec->flags & MFT_RECORD_IN_USE)) {
		a = find_attr(expand->mrec, type, NULL, 0);
		found = a && (a->type == type) && !a->name_length;
	}
		/* not finding the attribute list is not an error */
	if (!found && (type != AT_ATTRIBUTE_LIST)) {
		err_printf("Could not find attribute 0x%lx in inode %lld\n",
				(long)le32_to_cpu(type), (long long)inum);
		a = (ATTR_RECORD*)NULL;
	}
	return (a);
}

/*
 *		Read an MFT record and find an unnamed attribute
 *
 *	Returns NULL if fails
 */

static ATTR_RECORD *read_and_get_attr(expand_t *expand, ATTR_TYPES type,
				s64 inum, ntfschar *name, int namelen)
{
	ntfs_volume *vol;
	ATTR_RECORD *a;
	MFT_RECORD *mrec;
	s64 pos;
	int got;

	a = (ATTR_RECORD*)NULL;
	mrec = expand->mrec;
	vol = expand->vol;
	pos = (vol->mft_lcn << vol->cluster_size_bits)
		+ (inum << vol->mft_record_size_bits)
		+ expand->byte_increment;
	got = ntfs_mst_pread(vol->dev, pos, 1, vol->mft_record_size, mrec);
	if ((got == 1) && (mrec->flags & MFT_RECORD_IN_USE)) {
		a = find_attr(expand->mrec, type, name, namelen);
	}
	if (!a) {
		err_printf("Could not find attribute 0x%lx in inode %lld\n",
				(long)le32_to_cpu(type), (long long)inum);
	}
	return (a);
}

/*
 *		Get the size allocated to the unnamed data of some inode
 *
 *	Returns zero if fails.
 */

static s64 get_data_size(expand_t *expand, s64 inum)
{
	ATTR_RECORD *a;
	s64 size;

	size = 0;
			/* get the size of unnamed $DATA */
	a = get_unnamed_attr(expand, AT_DATA, inum);
	if (a && a->non_resident)
		size = sle64_to_cpu(a->allocated_size);
	if (!size) {
		err_printf("Bad record %lld, could not get its size\n",
					(long long)inum);
	}
	return (size);
}

/*
 *		Get the MFT bitmap
 *
 *	Returns NULL if fails.
 */

static u8 *get_mft_bitmap(expand_t *expand)
{
	ATTR_RECORD *a;
	ntfs_volume *vol;
	runlist_element *rl;
	runlist_element *prl;
	u32 bitmap_size;
	BOOL ok;

	expand->mft_bitmap = (u8*)NULL;
	vol = expand->vol;
			/* get the runlist of unnamed bitmap */
	a = get_unnamed_attr(expand, AT_BITMAP, FILE_MFT);
	ok = TRUE;
	bitmap_size = sle64_to_cpu(a->allocated_size);
	if (a
	    && a->non_resident
	    && ((bitmap_size << (vol->mft_record_size_bits + 3))
			>= expand->mft_size)) {
// rl in extent not implemented
		rl = ntfs_mapping_pairs_decompress(expand->vol, a,
						(runlist_element*)NULL);
		expand->mft_bitmap = (u8*)ntfs_calloc(bitmap_size);
		if (rl && expand->mft_bitmap) {
			for (prl=rl; prl->length && ok; prl++) {
				lseek_to_cluster(vol,
					prl->lcn + expand->cluster_increment);
				ok = !read_all(vol->dev, expand->mft_bitmap
					+ (prl->vcn << vol->cluster_size_bits),
					prl->length << vol->cluster_size_bits);
			}
			if (!ok) {
				err_printf("Could not read the MFT bitmap\n");
				free(expand->mft_bitmap);
				expand->mft_bitmap = (u8*)NULL;
			}
			free(rl);
		} else {
			err_printf("Could not get the MFT bitmap\n");
		}
	} else
		err_printf("Invalid MFT bitmap\n");
	return (expand->mft_bitmap);
}

/*
 *		Check for bad sectors
 *
 *	Deduplication to be done when proved safe
 */

static int check_expand_bad_sectors(expand_t *expand, ATTR_RECORD *a)
{
	runlist *rl;
	int res;
	s64 i, badclusters = 0;

	res = 0;
	ntfs_log_verbose("Checking for bad sectors ...\n");

	if (find_attr(expand->mrec, AT_ATTRIBUTE_LIST, NULL, 0)) {
		err_printf("Hopelessly many bad sectors have been detected!\n");
		err_printf("%s", many_bad_sectors_msg);
		res = -1;
	} else {

	/*
	 * FIXME: The below would be partial for non-base records in the
	 * not yet supported multi-record case. Alternatively use audited
	 * ntfs_attr_truncate after an umount & mount.
	 */
		rl = ntfs_mapping_pairs_decompress(expand->vol, a, NULL);
		if (!rl) {
			perr_printf("Decompressing $BadClust:"
					"$Bad mapping pairs failed");
			res = -1;
		} else {
			for (i = 0; rl[i].length; i++) {
		/* CHECKME: LCN_RL_NOT_MAPPED check isn't needed */
				if (rl[i].lcn == LCN_HOLE
				    || rl[i].lcn == LCN_RL_NOT_MAPPED)
					continue;

				badclusters += rl[i].length;
				ntfs_log_verbose("Bad cluster: %#8llx - %#llx"
						"    (%lld)\n",
						(long long)rl[i].lcn,
						(long long)rl[i].lcn
							+ rl[i].length - 1,
						(long long)rl[i].length);
			}

			if (badclusters) {
				err_printf("%sThis software has detected that"
					" the disk has at least"
					" %lld bad sector%s.\n",
					!opt.badsectors ? NERR_PREFIX
							: "WARNING: ",
					(long long)badclusters,
					badclusters - 1 ? "s" : "");
				if (!opt.badsectors) {
					err_printf("%s", bad_sectors_warning_msg);
					res = -1;
				} else
					err_printf("WARNING: Bad sectors can cause"
						" reliability problems"
						" and massive data loss!!!\n");
			}
		free(rl);
		}
	}
	return (res);
}

/*
 *		Check miscellaneous expansion constraints
 */

static int check_expand_constraints(expand_t *expand)
{
	static ntfschar bad[] = {
			const_cpu_to_le16('$'), const_cpu_to_le16('B'),
			const_cpu_to_le16('a'), const_cpu_to_le16('d')
	} ;
	ATTR_RECORD *a;
	runlist_element *rl;
	VOLUME_INFORMATION *volinfo;
	VOLUME_FLAGS flags;
	int res;

	if (opt.verbose)
		ntfs_log_verbose("Checking for expansion constraints...\n");
	res = 0;
		/* extents for $MFT are not supported */
	if (get_unnamed_attr(expand, AT_ATTRIBUTE_LIST, FILE_MFT)) {
		err_printf("The $MFT is too much fragmented\n");
		res = -1;
	}
		/* fragmented $MFTMirr is not supported */
	a = get_unnamed_attr(expand, AT_DATA, FILE_MFTMirr);
	if (a) {
		rl = ntfs_mapping_pairs_decompress(expand->vol, a, NULL);
		if (!rl || !rl[0].length || rl[1].length) {
			err_printf("$MFTMirr is bad or fragmented\n");
			res = -1;
		}
		free(rl);
	}
		/* fragmented $Boot is not supported */
	a = get_unnamed_attr(expand, AT_DATA, FILE_Boot);
	if (a) {
		rl = ntfs_mapping_pairs_decompress(expand->vol, a, NULL);
		if (!rl || !rl[0].length || rl[1].length) {
			err_printf("$Boot is bad or fragmented\n");
			res = -1;
		}
		free(rl);
	}
		/* Volume should not be marked dirty */
	a = get_unnamed_attr(expand, AT_VOLUME_INFORMATION, FILE_Volume);
	if (a) {
		volinfo = (VOLUME_INFORMATION*)
				(le16_to_cpu(a->value_offset) + (char*)a);
		flags = volinfo->flags;
		if ((flags & VOLUME_IS_DIRTY) && (opt.force-- <= 0)) {
			err_printf("Volume is scheduled for check.\nRun chkdsk /f"
			 " and please try again, or see option -f.\n");
			res = -1;
		}
	} else {
		err_printf("Could not get Volume flags\n");
		res = -1;
	}

		/* There should not be too many bad clusters */
	a = read_and_get_attr(expand, AT_DATA, FILE_BadClus, bad, 4);
	if (!a || !a->non_resident) {
		err_printf("Resident attribute in $BadClust! Please report to "
			 	"%s\n", NTFS_DEV_LIST);
		res = -1;
	} else
		if (check_expand_bad_sectors(expand,a))
			res = -1;
	return (res);
}

/*
 *		Compute the new sizes and check whether the NTFS file
 *	system can be expanded
 *
 *	The partition has to have been expanded,
 *	the extra space must be able to hold the $MFT, $Boot, and $Bitmap
 *	the extra space must be a multiple of cluster size
 *
 *	Returns TRUE if the partition can be expanded,
 *		FALSE if it canno be expanded or option --info was set
 */

static BOOL can_expand(expand_t *expand, ntfs_volume *vol)
{
	s64 old_sector_count;
	s64 sectors_needed;
	s64 clusters;
	s64 minimum_size;
	s64 got;
	s64 advice;
	s64 bitmap_bits;
	BOOL ok;

	ok = TRUE;
	old_sector_count = vol->nr_clusters
			<< (vol->cluster_size_bits - vol->sector_size_bits);
		/* do not include the space lost near the end */
	expand->cluster_increment = (expand->new_sectors
			 >> (vol->cluster_size_bits - vol->sector_size_bits))
				- vol->nr_clusters;
	expand->byte_increment = expand->cluster_increment
					<< vol->cluster_size_bits;
	expand->sector_increment = expand->byte_increment
					>> vol->sector_size_bits;
	printf("Sectors allocated to volume :  old %lld current %lld difference %lld\n",
			(long long)old_sector_count,
			(long long)(old_sector_count + expand->sector_increment),
			(long long)expand->sector_increment);
	printf("Clusters allocated to volume : old %lld current %lld difference %lld\n",
			(long long)vol->nr_clusters,
			(long long)(vol->nr_clusters
					+ expand->cluster_increment),
			(long long)expand->cluster_increment);
		/* the new size must be bigger */
	if ((expand->sector_increment < 0)
	    || (!expand->sector_increment && !opt.info)) {
		err_printf("Cannot expand volume : the partition has not been expanded\n");
		ok = FALSE;
	}
			/* the old bootsector must match the backup */
	got = ntfs_pread(expand->vol->dev, expand->byte_increment,
				vol->sector_size, expand->mrec);
	if ((got != vol->sector_size)
	    || memcmp(expand->bootsector,expand->mrec,vol->sector_size)) {
		err_printf("The backup bootsector does not match the old bootsector\n");
		ok = FALSE;
	}
	if (ok) {
			/* read the first MFT record, to get the MFT size */
		expand->mft_size = get_data_size(expand, FILE_MFT);
			/* read the 6th MFT record, to get the $Boot size */
		expand->boot_size = get_data_size(expand, FILE_Boot);
		if (!expand->mft_size || !expand->boot_size) {
			ok = FALSE;
		} else {
			/*
			 * The bitmap is one bit per full cluster,
			 * accounting for the backup bootsector.
			 * When evaluating the minimal size, the bitmap
			 * size must be adapted to the minimal size :
			 *  bits = clusters + ceil(clusters/clustersize)
			 */
			if (opt.info) {
				clusters = (((expand->original_sectors + 1)
						<< vol->sector_size_bits)
						+ expand->mft_size
						+ expand->boot_size)
						    >> vol->cluster_size_bits;
				bitmap_bits = ((clusters + 1)
						    << vol->cluster_size_bits)
						/ (vol->cluster_size + 1);
			} else {
				bitmap_bits = (expand->new_sectors + 1)
			    		>> (vol->cluster_size_bits
						- vol->sector_size_bits);
			}
			/* byte size must be a multiple of 8 */
			expand->bitmap_size = ((bitmap_bits + 63) >> 3) & -8;
			expand->bitmap_allocated = ((expand->bitmap_size - 1)
				| (vol->cluster_size - 1)) + 1;
			expand->mft_lcn = (expand->boot_size
					+ expand->bitmap_allocated)
						>> vol->cluster_size_bits;
			/*
			 * Check whether $Boot, $Bitmap and $MFT can fit
			 * into the expanded space.
			 */
			sectors_needed = (expand->boot_size + expand->mft_size
					 + expand->bitmap_allocated)
						>> vol->sector_size_bits;
			if (!opt.info
			    && (sectors_needed >= expand->sector_increment)) {
				err_printf("The expanded space cannot hold the new metadata\n");
				err_printf("   expanded space %lld sectors\n",
					(long long)expand->sector_increment);
				err_printf("   needed space %lld sectors\n",
					(long long)sectors_needed);
				ok = FALSE;
			}
		}
	}
	if (ok) {
		advice = expand->byte_increment;
		/* the increment must be an integral number of clusters */
		if (expand->byte_increment & (vol->cluster_size - 1)) {
			err_printf("Cannot expand volume without copying the data :\n");
			err_printf("There are %d sectors in a cluster,\n",
				(int)(vol->cluster_size/vol->sector_size));
			err_printf("  and the sector difference is not a multiple of %d\n",
				(int)(vol->cluster_size/vol->sector_size));
			advice = expand->byte_increment & ~vol->cluster_size;
			ok = FALSE;
		}
		if (!ok)
			err_printf("You should increase the beginning of partition by %d sectors\n",
				(int)((expand->byte_increment - advice)
					>> vol->sector_size_bits));
	}
	if (ok)
		ok = !check_expand_constraints(expand);
	if (ok && opt.info) {
		minimum_size = (expand->original_sectors
						<< vol->sector_size_bits)
					+ expand->boot_size
					+ expand->mft_size
					+ expand->bitmap_allocated;

		printf("You must expand the partition to at least %lld bytes,\n",
			(long long)(minimum_size + vol->sector_size));
		printf("and you may add a multiple of %ld bytes to this size.\n",
			(long)vol->cluster_size);
		printf("The minimum NTFS volume size is %lld bytes\n",
			(long long)minimum_size);
		ok = FALSE;
	}
	return (ok);
}

static int set_bitmap(expand_t *expand, runlist_element *rl)
{
	int res;
	s64 lcn;
	s64 lcn_end;
	BOOL reallocated;

	res = -1;
	reallocated = FALSE;
	if ((rl->lcn >= 0)
	    && (rl->length > 0)
	    && ((rl->lcn + rl->length)
		    <= (expand->vol->nr_clusters + expand->cluster_increment))) {
		lcn = rl->lcn;
		lcn_end = lcn + rl->length;
		while ((lcn & 7) && (lcn < lcn_end)) {
			if (expand->bitmap[lcn >> 3] & 1 << (lcn & 7))
				reallocated = TRUE;
			expand->bitmap[lcn >> 3] |= 1 << (lcn & 7);
			lcn++;
		}
		while ((lcn_end - lcn) >= 8) {
			if (expand->bitmap[lcn >> 3])
				reallocated = TRUE;
			expand->bitmap[lcn >> 3] = 255;
			lcn += 8;
		}
		while (lcn < lcn_end) {
			if (expand->bitmap[lcn >> 3] & 1 << (lcn & 7))
				reallocated = TRUE;
			expand->bitmap[lcn >> 3] |= 1 << (lcn & 7);
			lcn++;
		}
		if (reallocated)
			err_printf("Reallocated cluster found in run"
				" lcn 0x%llx length %lld\n",
				(long long)rl->lcn,(long long)rl->length);
		else
			res = 0;
	} else {
		err_printf("Bad run : lcn 0x%llx length %lld\n",
			(long long)rl->lcn,(long long)rl->length);
	}
	return (res);
}

/*
 *		Write the backup bootsector
 *
 *	When this has been done, the resizing cannot be done again
 */

static int write_bootsector(expand_t *expand)
{
	ntfs_volume *vol;
	s64 bw;
	int res;

	res = -1;
	vol = expand->vol;
	if (opt.verbose)
		ntfs_log_verbose("Rewriting the backup bootsector\n");
	if (opt.ro_flag)
		bw = vol->sector_size;
	else 
		bw = ntfs_pwrite(vol->dev,
				expand->new_sectors*vol->sector_size,
				vol->sector_size, expand->bootsector);
	if (bw == vol->sector_size)
		res = 0;
	else {
		if (bw != -1)
			errno = EINVAL;
		if (!bw)
			err_printf("Failed to rewrite the bootsector (size=0)\n");
		else
			err_printf("Error rewriting the bootsector");
	}
	return (res);
}

/*
 *		Write the new main bitmap
 */

static int write_bitmap(expand_t *expand)
{
	ntfs_volume *vol;
	s64 bw;
	u64 cluster;
	int res;

	res = -1;
	vol = expand->vol;
	cluster = vol->nr_clusters + expand->cluster_increment;
	while (cluster < (expand->bitmap_size << 3)) {
		expand->bitmap[cluster >> 3] |= 1 << (cluster & 7);
		cluster++;
	}
	if (opt.verbose)
		ntfs_log_verbose("Writing the new bitmap...\n");
		/* write the full allocation (to avoid having to read) */
	if (opt.ro_flag)
		bw = expand->bitmap_allocated;
	else
		bw = ntfs_pwrite(vol->dev, expand->boot_size,
					expand->bitmap_allocated, expand->bitmap);
	if (bw == (s64)expand->bitmap_allocated)
		res = 0;
	else {
		if (bw != -1)
			errno = EINVAL;
		if (!bw)
			err_printf("Failed to write the bitmap (size=0)\n");
		else
			err_printf("Error rewriting the bitmap");
	}
	return (res);
}

/*
 *		Copy the $MFT to $MFTMirr
 *
 *	The $MFTMirr is not relocated as it should be kept away from $MFT.
 *	Apart from the backup bootsector, this is the only part which is
 *	overwritten. This has no effect on being able to redo the resizing
 *	if something goes wrong, as the $MFTMirr is never read. However
 *	this is done near the end of the resizing.
 */

static int copy_mftmirr(expand_t *expand)
{
	ntfs_volume *vol;
	s64 pos;
	s64 inum;
	int res;
	u16 usa_ofs;
	le16 *pusn;
	u16 usn;

	if (opt.verbose)
		ntfs_log_verbose("Copying $MFT to $MFTMirr...\n");
	vol = expand->vol;
	res = 0;
	for (inum=FILE_MFT; !res && (inum<=FILE_Volume); inum++) {
			/* read the new $MFT */
		pos = (expand->mft_lcn << vol->cluster_size_bits)
			+ (inum << vol->mft_record_size_bits);
		if (ntfs_mst_pread(vol->dev, pos, 1, vol->mft_record_size,
				expand->mrec) == 1) {
				/* overwrite the old $MFTMirr */
			pos = (vol->mftmirr_lcn << vol->cluster_size_bits)
				+ (inum << vol->mft_record_size_bits)
				+ expand->byte_increment;
			usa_ofs = le16_to_cpu(expand->mrec->usa_ofs);
			pusn = (le16*)((u8*)expand->mrec + usa_ofs);
			usn = le16_to_cpu(*pusn) - 1;
			if (!usn || (usn == 0xffff))
				usn = -2;
			*pusn = cpu_to_le16(usn);
			if (!opt.ro_flag
			    && (ntfs_mst_pwrite(vol->dev, pos, 1,
				    vol->mft_record_size, expand->mrec) != 1)) {
				err_printf("Failed to overwrite the old $MFTMirr\n");
				res = -1;
			}
		} else {
			err_printf("Failed to write the new $MFT\n");
			res = -1;
		}
	}
	return (res);
}

/*
 *		Copy the $Boot, including the bootsector
 *
 *	When the bootsector has been copied, repair tools are able to
 *	fix things, but this is dangerous if the other metadata do
 *	not point to actual user data. So this must be done near the end
 *	of resizing.
 */

static int copy_boot(expand_t *expand)
{
	NTFS_BOOT_SECTOR *bs;
	char *buf;
	ntfs_volume *vol;
	s64 mftmirr_lcn;
	s64 written;
	u32 boot_cnt;
	u32 hidden_sectors;
	le32 hidden_sectors_le;
	int res;

	if (opt.verbose)
		ntfs_log_verbose("Copying $Boot...\n");
	vol = expand->vol;
	res = 0;
	buf = (char*)ntfs_malloc(vol->cluster_size);
	if (buf) {
			/* set the new volume parameters in the bootsector */
		bs = (NTFS_BOOT_SECTOR*)expand->bootsector;
		bs->number_of_sectors = cpu_to_sle64(expand->new_sectors);
		bs->mft_lcn = cpu_to_sle64(expand->mft_lcn);
		mftmirr_lcn = vol->mftmirr_lcn + expand->cluster_increment;
		bs->mftmirr_lcn = cpu_to_sle64(mftmirr_lcn);
			/* the hidden sectors are needed to boot into windows */
		memcpy(&hidden_sectors_le,&bs->bpb.hidden_sectors,4);
				/* alignment messed up on the Sparc */
		if (hidden_sectors_le) {
			hidden_sectors = le32_to_cpu(hidden_sectors_le);
			if (hidden_sectors >= expand->sector_increment)
				hidden_sectors -= expand->sector_increment;
			else
				hidden_sectors = 0;
			hidden_sectors_le = cpu_to_le32(hidden_sectors);
			memcpy(&bs->bpb.hidden_sectors,&hidden_sectors_le,4);
		}
		written = 0;
		boot_cnt = expand->boot_size >> vol->cluster_size_bits;
		while (!res && (written < boot_cnt)) {
			lseek_to_cluster(vol, expand->cluster_increment + written);
			if (!read_all(vol->dev, buf, vol->cluster_size)) {
				if (!written)
					memcpy(buf, expand->bootsector, vol->sector_size);
				lseek_to_cluster(vol, written);
				if (!opt.ro_flag
				    && write_all(vol->dev, buf, vol->cluster_size)) {
					err_printf("Failed to write the new $Boot\n");
					res = -1;
				} else
					written++;
			} else {
				err_printf("Failed to read the old $Boot\n");
				res = -1;
			}
		}
		free(buf);
	} else {
		err_printf("Failed to allocate buffer\n");
		res = -1;
	}
	return (res);
}

/*
 *		Process delayed runlist updates
 *
 *	This is derived from delayed_updates() and they should
 *	both be merged when the new code is considered safe.
 */

static void delayed_expand(ntfs_volume *vol, struct DELAYED *delayed,
			struct progress_bar *progress)
{
	unsigned long count;
	struct DELAYED *current;
	int step = 100;

	if (delayed) {
		if (opt.verbose)
			ntfs_log_verbose("Delayed updating of overflowing runlists...\n");
		count = 0;
			/* count by steps because of inappropriate resolution */
		for (current=delayed; current; current=current->next)
			count += step;
		progress_init(progress, 0, count,
					(opt.show_progress ? NTFS_PROGBAR : 0));
		current = delayed;
		count = 0;
		while (current) {
			delayed = current;
			if (!opt.ro_flag)
				expand_attribute_runlist(vol, delayed);
			count += step;
			progress_update(progress, count);
			current = current->next;
			if (delayed->attr_name)
				free(delayed->attr_name);
			free(delayed->head_rl);
			free(delayed);
		}
	}
}

/*
 *		Expand the sizes in indexes for inodes which were expanded
 *
 *	Only the new $Bitmap sizes are identified as needed to be
 *	adjusted in index. The $BadClus is only expanded in an
 *	alternate data stream, whose sizes are not present in the index.
 *
 *	This is modifying the initial data, and can only be done when
 *	the volume has been reopened after expanding.
 */

static int expand_index_sizes(expand_t *expand)
{
	ntfs_inode *ni;
	int res;

	res = -1;
	ni = ntfs_inode_open(expand->vol, FILE_Bitmap);
	if (ni) {
		NInoSetDirty(ni);
		NInoFileNameSetDirty(ni);
		ntfs_inode_close(ni);
		res = 0;
	}
	return (res);
}

/*
 *		Update a runlist into an attribute
 *
 *	This is derived from replace_attribute_runlist() and they should
 *	both be merged when the new code is considered safe.
 */

static int update_runlist(expand_t *expand, s64 inum,
				ATTR_RECORD *a, runlist_element *rl)
{
	ntfs_resize_t resize;
	ntfs_attr_search_ctx ctx;
	ntfs_volume *vol;
	MFT_RECORD *mrec;
	runlist *head_rl;
	int mp_size;
	int l;
	int must_delay;
	void *mp;

	vol = expand->vol;
	mrec = expand->mrec;
	head_rl = rl;
	rl_fixup(&rl);
	if ((mp_size = ntfs_get_size_for_mapping_pairs(vol, rl,
				0, INT_MAX)) == -1)
		perr_exit("ntfs_get_size_for_mapping_pairs");

	if (a->name_length) {
		u16 name_offs = le16_to_cpu(a->name_offset);
		u16 mp_offs = le16_to_cpu(a->mapping_pairs_offset);

		if (name_offs >= mp_offs)
			err_exit("Attribute name is after mapping pairs! "
				 "Please report!\n");
	}

	/* CHECKME: don't trust mapping_pairs is always the last item in the
	   attribute, instead check for the real size/space */
	l = (int)le32_to_cpu(a->length) - le16_to_cpu(a->mapping_pairs_offset);
	must_delay = 0;
	if (mp_size > l) {
		s32 remains_size;
		char *next_attr;

		ntfs_log_verbose("Enlarging attribute header ...\n");

		mp_size = (mp_size + 7) & ~7;

		ntfs_log_verbose("Old mp size      : %d\n", l);
		ntfs_log_verbose("New mp size      : %d\n", mp_size);
		ntfs_log_verbose("Bytes in use     : %u\n", (unsigned int)
				 le32_to_cpu(mrec->bytes_in_use));

		next_attr = (char *)a + le32_to_cpu(a->length);
		l = mp_size - l;

		ntfs_log_verbose("Bytes in use new : %u\n", l + (unsigned int)
				 le32_to_cpu(mrec->bytes_in_use));
		ntfs_log_verbose("Bytes allocated  : %u\n", (unsigned int)
				 le32_to_cpu(mrec->bytes_allocated));

		remains_size = le32_to_cpu(mrec->bytes_in_use);
		remains_size -= (next_attr - (char *)mrec);

		ntfs_log_verbose("increase         : %d\n", l);
		ntfs_log_verbose("shift            : %lld\n",
				 (long long)remains_size);
		if (le32_to_cpu(mrec->bytes_in_use) + l >
				le32_to_cpu(mrec->bytes_allocated)) {
			ntfs_log_verbose("Queuing expansion for later processing\n");
				/* hack for reusing unmodified old code ! */
			resize.ctx = &ctx;
			ctx.attr = a;
			ctx.mrec = mrec;
			resize.mref = inum;
			resize.delayed_runlists = expand->delayed_runlists;
			resize.mirr_from = MIRR_OLD;
			must_delay = 1;
			replace_later(&resize,rl,head_rl);
			expand->delayed_runlists = resize.delayed_runlists;
		} else {
			memmove(next_attr + l, next_attr, remains_size);
			mrec->bytes_in_use = cpu_to_le32(l +
					le32_to_cpu(mrec->bytes_in_use));
			a->length = cpu_to_le32(le32_to_cpu(a->length) + l);
		}
	}

	if (!must_delay) {
		mp = ntfs_calloc(mp_size);
		if (!mp)
			perr_exit("ntfsc_calloc couldn't get memory");

		if (ntfs_mapping_pairs_build(vol, (u8*)mp, mp_size, rl, 0, NULL))
			perr_exit("ntfs_mapping_pairs_build");

		memmove((u8*)a + le16_to_cpu(a->mapping_pairs_offset), mp, mp_size);

		free(mp);
	}
	return (must_delay);
}

/*
 *		Create a minimal valid MFT record
 */

static int minimal_record(expand_t *expand, MFT_RECORD *mrec)
{
	int usa_count;
	u32 bytes_in_use;

	memset(mrec,0,expand->vol->mft_record_size);
	mrec->magic = magic_FILE;
	mrec->usa_ofs = const_cpu_to_le16(sizeof(MFT_RECORD));
	usa_count = expand->vol->mft_record_size / NTFS_BLOCK_SIZE + 1;
	mrec->usa_count = cpu_to_le16(usa_count);
	bytes_in_use = (sizeof(MFT_RECORD) + 2*usa_count + 7) & -8;
	memset(((char*)mrec) + bytes_in_use, 255, 4);  /* AT_END */
	bytes_in_use += 8;
	mrec->bytes_in_use = cpu_to_le32(bytes_in_use);
	mrec->bytes_allocated = cpu_to_le32(expand->vol->mft_record_size);
	return (0);
}

/*
 *		Rebase all runlists of an MFT record
 *
 *	Iterate through all its attributes and offset the non resident ones
 */

static int rebase_runlists(expand_t *expand, s64 inum)
{
	MFT_RECORD *mrec;
	ATTR_RECORD *a;
	runlist_element *rl;
	runlist_element *prl;
	u32 offset;
	int res;

	res = 0;
	mrec = expand->mrec;
	offset = le16_to_cpu(mrec->attrs_offset);
	a = (ATTR_RECORD*)((char*)mrec + offset);
	while (!res && (a->type != AT_END)
			&& (offset < le32_to_cpu(mrec->bytes_in_use))) {
		if (a->non_resident) {
			rl = ntfs_mapping_pairs_decompress(expand->vol, a,
						(runlist_element*)NULL);
			if (rl) {
				for (prl=rl; prl->length; prl++)
					if (prl->lcn >= 0) {
						prl->lcn += expand->cluster_increment;
						if (set_bitmap(expand,prl))
							res = -1;
						}
				if (update_runlist(expand,inum,a,rl)) {
					ntfs_log_verbose("Runlist updating has to be delayed\n");
				} else
					free(rl);
			} else {
				err_printf("Could not get a runlist of inode %lld\n",
						(long long)inum);
				res = -1;
			}
		}
		offset += le32_to_cpu(a->length);
		a = (ATTR_RECORD*)((char*)mrec + offset);
	}
	return (res);
}

/*
 *		Rebase the runlists present in records with relocated $DATA
 *
 *	The returned runlist is the old rebased runlist for $DATA,
 *	which is generally different from the new computed runlist.
 */

static runlist_element *rebase_runlists_meta(expand_t *expand, s64 inum)
{
	MFT_RECORD *mrec;
	ATTR_RECORD *a;
	ntfs_volume *vol;
	runlist_element *rl;
	runlist_element *old_rl;
	runlist_element *prl;
	runlist_element new_rl[2];
	s64 data_size;
	s64 allocated_size;
	s64 lcn;
	u64 lth;
	u32 offset;
	BOOL keeprl;
	int res;

	res = 0;
	old_rl = (runlist_element*)NULL;
	vol = expand->vol;
	mrec = expand->mrec;
	switch (inum) {
	case FILE_Boot :
		lcn = 0;
		lth = expand->boot_size >> vol->cluster_size_bits;
		data_size = expand->boot_size;
		break;
	case FILE_Bitmap :
		lcn = expand->boot_size >> vol->cluster_size_bits;
		lth = expand->bitmap_allocated >> vol->cluster_size_bits;
		data_size = expand->bitmap_size;
		break;
	case FILE_MFT :
		lcn = (expand->boot_size + expand->bitmap_allocated)
				>> vol->cluster_size_bits;
		lth = expand->mft_size >> vol->cluster_size_bits;
		data_size = expand->mft_size;
		break;
	case FILE_BadClus :
		lcn = 0; /* not used */
		lth = vol->nr_clusters + expand->cluster_increment;
		data_size = lth << vol->cluster_size_bits;
		break;
	default :
		lcn = lth = data_size = 0;
		res = -1;
	}
	allocated_size = lth << vol->cluster_size_bits;
	offset = le16_to_cpu(mrec->attrs_offset);
	a = (ATTR_RECORD*)((char*)mrec + offset);
	while (!res && (a->type != AT_END)
			&& (offset < le32_to_cpu(mrec->bytes_in_use))) {
		if (a->non_resident) {
			keeprl = FALSE;
			rl = ntfs_mapping_pairs_decompress(vol, a,
						(runlist_element*)NULL);
			if (rl) {
				/* rebase the old runlist */
				for (prl=rl; prl->length; prl++)
					if (prl->lcn >= 0) {
						prl->lcn += expand->cluster_increment;
						if ((a->type != AT_DATA)
						    && set_bitmap(expand,prl))
							res = -1;
					}
				/* relocated unnamed data (not $BadClus) */
				if ((a->type == AT_DATA)
				    && !a->name_length
				    && (inum != FILE_BadClus)) {
					old_rl = rl;
					rl = new_rl;
					keeprl = TRUE;
					rl[0].vcn = 0;
					rl[0].lcn = lcn;
					rl[0].length = lth;
					rl[1].vcn = lth;
					rl[1].lcn = LCN_ENOENT;
					rl[1].length = 0;
					if (set_bitmap(expand,rl))
						res = -1;
					a->data_size = cpu_to_sle64(data_size);
					a->initialized_size = a->data_size;
					a->allocated_size
						= cpu_to_sle64(allocated_size);
					a->highest_vcn = cpu_to_sle64(lth - 1);
				}
				/* expand the named data for $BadClus */
				if ((a->type == AT_DATA)
				    && a->name_length
				    && (inum == FILE_BadClus)) {
					old_rl = rl;
					keeprl = TRUE;
					prl = rl;
					if (prl->length) {
						while (prl[1].length)
							prl++;
						prl->length = lth - prl->vcn;
						prl[1].vcn = lth;
					} else
						prl->vcn = lth;
					a->data_size = cpu_to_sle64(data_size);
					/* do not change the initialized size */
					a->allocated_size
						= cpu_to_sle64(allocated_size);
					a->highest_vcn = cpu_to_sle64(lth - 1);
				}
				if (!res && update_runlist(expand,inum,a,rl))
					res = -1;
				if (!keeprl)
					free(rl);
			} else {
				err_printf("Could not get the data runlist of inode %lld\n",
						(long long)inum);
				res = -1;
			}
		}
		offset += le32_to_cpu(a->length);
		a = (ATTR_RECORD*)((char*)mrec + offset);
	}
	if (res && old_rl) {
		free(old_rl);
		old_rl = (runlist_element*)NULL;
	}
	return (old_rl);
}

/*
 *		Rebase all runlists in an MFT record
 *
 *	Read from the old $MFT, rebase the runlists,
 *	and write to the new $MFT
 */

static int rebase_inode(expand_t *expand, const runlist_element *prl,
		s64 inum, s64 jnum)
{
	MFT_RECORD *mrec;
	runlist_element *rl;
	ntfs_volume *vol;
	s64 pos;
	int res;

	res = 0;
	vol = expand->vol;
	mrec = expand->mrec;
	if (expand->mft_bitmap[inum >> 3] & (1 << (inum & 7))) {
		pos = (prl->lcn << vol->cluster_size_bits)
			+ ((inum - jnum) << vol->mft_record_size_bits);
		if ((ntfs_mst_pread(vol->dev, pos, 1,
					vol->mft_record_size, mrec) == 1)
		    && (mrec->flags & MFT_RECORD_IN_USE)) {
			switch (inum) {
			case FILE_Bitmap :
			case FILE_Boot :
			case FILE_BadClus :
				rl = rebase_runlists_meta(expand, inum);
				if (rl)
					free(rl);
				else
					res = -1;
				break;
			default :
			   	res = rebase_runlists(expand, inum);
				break;
			}
		} else {
			err_printf("Could not read the $MFT entry %lld\n",
					(long long)inum);
			res = -1;
		}
	} else {
			/*
			 * Replace unused records (possibly uninitialized)
			 * by minimal valid records, not marked in use
			 */
		res = minimal_record(expand,mrec);
	}
	if (!res) {
		pos = (expand->mft_lcn << vol->cluster_size_bits)
			+ (inum << vol->mft_record_size_bits);
		if (opt.verbose)
			ntfs_log_verbose("Rebasing inode %lld cluster 0x%llx\n",
				(long long)inum,
				(long long)(pos >> vol->cluster_size_bits));
		if (!opt.ro_flag
		    && (ntfs_mst_pwrite(vol->dev, pos, 1,
				vol->mft_record_size, mrec) != 1)) {
			err_printf("Could not write the $MFT entry %lld\n",
					(long long)inum);
			res = -1;
		}
	}
	return (res);
}

/*
 *		Rebase all runlists
 *
 *	First get the $MFT and define its location in the expanded space,
 *	then rebase the other inodes and write them to the new $MFT
 */

static int rebase_all_inodes(expand_t *expand)
{
	ntfs_volume *vol;
	MFT_RECORD *mrec;
	s64 inum;
	s64 jnum;
	s64 inodecnt;
	s64 pos;
	s64 got;
	int res;
	runlist_element *mft_rl;
	runlist_element *prl;

	res = 0;
	mft_rl = (runlist_element*)NULL;
	vol = expand->vol;
	mrec = expand->mrec;
	inum = 0;
	pos = (vol->mft_lcn + expand->cluster_increment)
				<< vol->cluster_size_bits;
	got = ntfs_mst_pread(vol->dev, pos, 1,
			vol->mft_record_size, mrec);
	if ((got == 1) && (mrec->flags & MFT_RECORD_IN_USE)) {
		pos = expand->mft_lcn << vol->cluster_size_bits;
		if (opt.verbose)
			ntfs_log_verbose("Rebasing inode %lld cluster 0x%llx\n",
				(long long)inum,
				(long long)(pos >> vol->cluster_size_bits));
		mft_rl = rebase_runlists_meta(expand, FILE_MFT);
		if (!mft_rl
		    || (!opt.ro_flag
			&& (ntfs_mst_pwrite(vol->dev, pos, 1,
				vol->mft_record_size, mrec) != 1)))
			res = -1;
		else {
			for (prl=mft_rl; prl->length; prl++) { }
			inodecnt = (prl->vcn << vol->cluster_size_bits)
				>> vol->mft_record_size_bits;
			progress_init(expand->progress, 0, inodecnt,
				(opt.show_progress ? NTFS_PROGBAR : 0));
			prl = mft_rl;
			jnum = 0;
			do {
				inum++;
				while (prl->length
				    && ((inum << vol->mft_record_size_bits)
					>= ((prl->vcn + prl->length)
						<< vol->cluster_size_bits))) {
					prl++;
					jnum = inum;
				}
				progress_update(expand->progress, inum);
				if (prl->length) {
					res = rebase_inode(expand,
						prl,inum,jnum);
				}
			} while (!res && prl->length);
			free(mft_rl);
		}
	} else {
		err_printf("Could not read the old $MFT\n");
		res = -1;
	}
	return (res);
}



/*
 *		Get the old volume parameters from the backup bootsector
 *
 */

static ntfs_volume *get_volume_data(expand_t *expand, struct ntfs_device *dev,
			s32 sector_size)
{
	s64 br;
	ntfs_volume *vol;
	le16 sector_size_le;
	NTFS_BOOT_SECTOR *bs;
	BOOL ok;

	ok = FALSE;
	vol = (ntfs_volume*)ntfs_malloc(sizeof(ntfs_volume));
	expand->bootsector = (char*)ntfs_malloc(sector_size);
	if (vol && expand->bootsector) {
		expand->vol = vol;
		vol->dev = dev;
		br = ntfs_pread(dev, expand->new_sectors*sector_size,
				 sector_size, expand->bootsector);
		if (br != sector_size) {
			if (br != -1)
				errno = EINVAL;
			if (!br)
				err_printf("Failed to read the backup bootsector (size=0)\n");
			else
				err_printf("Error reading the backup bootsector");
		} else {
			bs = (NTFS_BOOT_SECTOR*)expand->bootsector;
		/* alignment problem on Sparc, even doing memcpy() */
			sector_size_le = cpu_to_le16(sector_size);
			if (!memcmp(&sector_size_le,
						&bs->bpb.bytes_per_sector,2)
			    && ntfs_boot_sector_is_ntfs(bs)
			    && !ntfs_boot_sector_parse(vol, bs)) {
				expand->original_sectors
				    = sle64_to_cpu(bs->number_of_sectors);
				expand->mrec = (MFT_RECORD*)
					ntfs_malloc(vol->mft_record_size);
				if (expand->mrec
				    && can_expand(expand,vol)) {
					ntfs_log_verbose("Resizing is possible\n");
					ok = TRUE;
				}
			} else
				err_printf("Could not get the old volume parameters "
					"from the backup bootsector\n");
		}
		if (!ok) {
			free(vol);
			free(expand->bootsector);
		}
	}
	return (ok ? vol : (ntfs_volume*)NULL);
}

static int really_expand(expand_t *expand)
{
	ntfs_volume *vol;
	struct ntfs_device *dev;
	int res;

	res = -1;

	expand->bitmap = (u8*)ntfs_calloc(expand->bitmap_allocated);
	if (expand->bitmap
	    && get_mft_bitmap(expand)) {
		printf("\n*** WARNING ***\n\n");
		printf("Expanding a volume is an experimental new feature\n");
		if (!opt.ro_flag)
			printf("A first check with option -n is recommended\n");
		printf("\nShould something go wrong during the actual"
			 " resizing (power outage, etc.),\n");
		printf("just restart the procedure, but DO NOT TRY to repair"
			" with chkdsk or similar,\n");
		printf("until the resizing is over,"
			" you would LOSE YOUR DATA !\n");
		printf("\nYou have been warned !\n\n");
		if (!opt.ro_flag && (opt.force-- <= 0))
			proceed_question();
		if (!rebase_all_inodes(expand)
		    && !write_bitmap(expand)
		    && !copy_mftmirr(expand)
		    && !copy_boot(expand)) {
			free(expand->vol);
			expand->vol = (ntfs_volume*)NULL;
			free(expand->mft_bitmap);
			expand->mft_bitmap = (u8*)NULL;
			if (!opt.ro_flag) {
				/* the volume must be dirty, do not check */
				opt.force++;
				vol = mount_volume();
				if (vol) {
					dev = vol->dev;
					ntfs_log_verbose("Remounting the updated volume\n");
					expand->vol = vol;
					ntfs_log_verbose("Delayed runlist updatings\n");
					delayed_expand(vol, expand->delayed_runlists,
						expand->progress);
					expand->delayed_runlists
						= (struct DELAYED*)NULL;
					expand_index_sizes(expand);
		/* rewriting the backup bootsector, no return ticket now ! */
					res = write_bootsector(expand);
					if (dev->d_ops->sync(dev) == -1) {
						printf("Could not sync\n");
						res = -1;
					}
					ntfs_umount(vol,0);
					if (!res)
						printf("\nResizing completed successfully\n");
				}
			} else {
				ntfs_log_verbose("Delayed runlist updatings\n");
				delayed_expand(expand->vol,
						expand->delayed_runlists,
						expand->progress);
				expand->delayed_runlists
						= (struct DELAYED*)NULL;
				printf("\nAll checks have been completed successfully\n");
				printf("Cannot check further in no-action mode\n");
			}
			free(expand->bootsector);
			free(expand->mrec);
		}
		free(expand->bitmap);
	} else {
		err_printf("Failed to allocate memory\n");
	}
	return (res);
}

/*
 *		Expand a volume to beginning of partition
 *
 *	We rely on the backup bootsector to determine the original
 *	volume size and metadata.
 */

static int expand_to_beginning(void)
{
	expand_t expand;
	struct progress_bar progress;
	int ret;
	ntfs_volume *vol;
	struct ntfs_device *dev;
	int sector_size;
	s64 new_sectors;
	        
	ret = -1;
	dev = ntfs_device_alloc(opt.volume, 0, &ntfs_device_default_io_ops,
			NULL);
	if (dev) {
	        if (!(*dev->d_ops->open)(dev,
				(opt.ro_flag ? O_RDONLY : O_RDWR))) {
			sector_size = ntfs_device_sector_size_get(dev);
			if (sector_size <= 0) {
				sector_size = 512;
				new_sectors = ntfs_device_size_get(dev,
								sector_size);
				if (!new_sectors) {
					sector_size = 4096;
					new_sectors = ntfs_device_size_get(dev,
								sector_size);
				}
			} else
				new_sectors = ntfs_device_size_get(dev,
								sector_size);
			if (new_sectors) {
				new_sectors--; /* last sector not counted */
				expand.new_sectors = new_sectors;
				expand.progress = &progress;
				expand.delayed_runlists = (struct DELAYED*)NULL;
				vol = get_volume_data(&expand,dev,sector_size);
				if (vol) {
					expand.vol = vol;
					ret = really_expand(&expand);
				}
			}
			(*dev->d_ops->close)(dev);
		} else {
			err_exit("Couldn't open volume '%s'!\n", opt.volume);
		}
		ntfs_device_free(dev);
	}
	return (ret);
}


int main(int argc, char **argv)
{
	ntfsck_t fsck;
	ntfs_resize_t resize;
	s64 new_size = 0;	/* in clusters; 0 = --info w/o --size */
	s64 device_size;        /* in bytes */
	ntfs_volume *vol = NULL;
	int res;

	ntfs_log_set_handler(ntfs_log_handler_outerr);

	printf("%s v%s (libntfs-3g)\n", EXEC_NAME, VERSION);

	res = parse_options(argc, argv);
	if (res >= 0)
		return (res);

	utils_set_locale();

		/*
		 * If we're just checking the device, we'll do it first,
		 * and exit out, no matter what we find.
		 */
	if (opt.check) {
		vol = check_volume();
#if CLEAN_EXIT
		if (vol)
			ntfs_umount(vol,0);
#endif
		exit(0);
	} else {
		if (opt.expand) {
			/*
			 * If we are to expand to beginning of partition, do
			 * not try to mount : when merging two partitions,
			 * the beginning of the partition would contain an
			 * old filesystem which is not the one to expand.
			 */
			if (expand_to_beginning() && !opt.info)
				exit(1);
			return (0);
		}
	}

	if (!(vol = mount_volume()))
		err_exit("Couldn't open volume '%s'!\n", opt.volume);

	device_size = ntfs_device_size_get(vol->dev, vol->sector_size);
	device_size *= vol->sector_size;
	if (device_size <= 0)
		err_exit("Couldn't get device size (%lld)!\n",
			(long long)device_size);

	if (!opt.infombonly)
		print_vol_size("Current device size", device_size);

	if (device_size < vol->nr_clusters * vol->cluster_size)
		err_exit("Current NTFS volume size is bigger than the device "
			 "size!\nCorrupt partition table or incorrect device "
			 "partitioning?\n");

	if (!opt.bytes && !opt.info && !opt.infombonly) {
		opt.bytes = device_size;
		opt.reliable_size = 1;
	}

	/* Backup boot sector at the end of device isn't counted in NTFS
	   volume size thus we have to reserve space for it. */
	if (opt.bytes > vol->sector_size)
		new_size = (opt.bytes - vol->sector_size) / vol->cluster_size;
	else
		new_size = 0;

	if (!opt.info && !opt.infombonly) {
		print_vol_size("New volume size    ", vol_size(vol, new_size));
		if (device_size < opt.bytes)
			err_exit("New size can't be bigger than the device size"
				 ".\nIf you want to enlarge NTFS then first "
				 "enlarge the device size by e.g. fdisk.\n");
	}

	if (!opt.info && !opt.infombonly && (new_size == vol->nr_clusters ||
			  (opt.bytes == device_size &&
			   new_size == vol->nr_clusters - 1))) {
		printf("Nothing to do: NTFS volume size is already OK.\n");
		exit(0);
	}

	memset(&resize, 0, sizeof(resize));
	resize.vol = vol;
	resize.new_volume_size = new_size;
	/* This is also true if --info was used w/o --size (new_size = 0) */
	if (new_size < vol->nr_clusters)
		resize.shrink = 1;
	if (opt.show_progress)
		resize.progress.flags |= NTFS_PROGBAR;
	/*
	 * Checking and __reporting__ of bad sectors must be done before cluster
	 * allocation check because chkdsk doesn't fix $Bitmap's w/ bad sectors
	 * thus users would (were) quite confused why chkdsk doesn't work.
	 */
	resize.badclusters = check_bad_sectors(vol);

	NVolSetNoFixupWarn(vol);
	check_cluster_allocation(vol, &fsck);

	print_disk_usage(vol, fsck.inuse);

	resize.inuse = fsck.inuse;
	resize.lcn_bitmap = fsck.lcn_bitmap;
	resize.mirr_from = MIRR_OLD;

	set_resize_constraints(&resize);
	set_disk_usage_constraint(&resize);
	check_resize_constraints(&resize);

	if (opt.info || opt.infombonly) {
		advise_on_resize(&resize);
		exit(0);
	}

	if (opt.force-- <= 0 && !opt.ro_flag) {
		printf("%s", resize_warning_msg);
		proceed_question();
	}

	/* FIXME: performance - relocate logfile here if it's needed */
	prepare_volume_fixup(vol);

	if (resize.relocations)
		relocate_inodes(&resize);

	truncate_badclust_file(&resize);
	truncate_bitmap_file(&resize);
	delayed_updates(&resize);
	update_bootsector(&resize);

	/* We don't create backup boot sector because we don't know where the
	   partition will be split. The scheduled chkdsk will fix it */

	if (opt.ro_flag) {
		printf("The read-only test run ended successfully.\n");
		exit(0);
	}

	/* WARNING: don't modify the texts, external tools grep for them */
	printf("Syncing device ...\n");
	if (vol->dev->d_ops->sync(vol->dev) == -1)
		perr_exit("fsync");

	printf("Successfully resized NTFS on device '%s'.\n", vol->dev->d_name);
	if (resize.shrink)
		printf("%s", resize_important_msg);
	if (resize.lcn_bitmap.bm)
		free(resize.lcn_bitmap.bm);
	if (vol)
		ntfs_umount(vol,0);
	return 0;
}
