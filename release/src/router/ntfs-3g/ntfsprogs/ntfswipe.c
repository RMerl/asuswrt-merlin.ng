/**
 * ntfswipe - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2005 Anton Altaparmakov
 * Copyright (c) 2002-2005 Richard Russon
 * Copyright (c) 2004 Yura Pakhuchiy
 *
 * This utility will overwrite unused space on an NTFS volume.
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

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "ntfswipe.h"
#include "types.h"
#include "volume.h"
#include "utils.h"
#include "debug.h"
#include "dir.h"
#include "mst.h"
/* #include "version.h" */
#include "logging.h"
#include "list.h"
#include "mft.h"

static const char *EXEC_NAME = "ntfswipe";
static struct options opts;
static unsigned long int npasses = 0;

struct filename {
	char		*parent_name;
	struct ntfs_list_head list;	/* Previous/Next links */
	ntfschar	*uname;		/* Filename in unicode */
	int		 uname_len;	/* and its length */
		/* Allocated size (multiple of cluster size) */
	s64		 size_alloc;
	s64		 size_data;	/* Actual size of data */
	long long	 parent_mref;
	FILE_ATTR_FLAGS	 flags;
	time_t		 date_c;	/* Time created */
	time_t		 date_a;	/*	altered */
	time_t		 date_m;	/*	mft record changed */
	time_t		 date_r;	/*	read */
	char		*name;		/* Filename in current locale */
	FILE_NAME_TYPE_FLAGS name_space;
	char		 padding[7];	/* Unused: padding to 64 bit. */
};

struct data {
	struct ntfs_list_head list;	/* Previous/Next links */
	char		*name;		/* Stream name in current locale */
	ntfschar	*uname;		/* Unicode stream name */
	int		 uname_len;	/* and its length */
	int		 resident;	/* Stream is resident */
	int		 compressed;	/* Stream is compressed */
	int		 encrypted;	/* Stream is encrypted */
		/* Allocated size (multiple of cluster size) */
	s64		 size_alloc;
	s64		 size_data;	/* Actual size of data */
		/* Initialised size, may be less than data size */
	s64		 size_init;
	VCN		 size_vcn;	/* Highest VCN in the data runs */
	runlist_element *runlist;	/* Decoded data runs */
	int		 percent;	/* Amount potentially recoverable */
	void		*data;	       /* If resident, a pointer to the data */
	char		 padding[4];	/* Unused: padding to 64 bit. */
};

struct ufile {
	s64		 inode;		/* MFT record number */
	time_t		 date;		/* Last modification date/time */
	struct ntfs_list_head name;		/* A list of filenames */
	struct ntfs_list_head data;		/* A list of data streams */
	char		*pref_name;	/* Preferred filename */
	char		*pref_pname;	/*	     parent filename */
	s64		 max_size;	/* Largest size we find */
	int		 attr_list;	/* MFT record may be one of many */
	int		 directory;	/* MFT record represents a directory */
	MFT_RECORD	*mft;		/* Raw MFT record */
	char		 padding[4];	/* Unused: padding to 64 bit. */
};

#define NPAT 22

/* Taken from `shred' source */
static const unsigned int patterns[NPAT] = {
	0x000, 0xFFF,					/* 1-bit */
	0x555, 0xAAA,					/* 2-bit */
	0x249, 0x492, 0x6DB, 0x924, 0xB6D, 0xDB6,	/* 3-bit */
	0x111, 0x222, 0x333, 0x444, 0x666, 0x777,
	0x888, 0x999, 0xBBB, 0xCCC, 0xDDD, 0xEEE	/* 4-bit */
};


/**
 * version - Print version information about the program
 *
 * Print a copyright statement and a brief description of the program.
 *
 * Return:  none
 */
static void version(void)
{
	ntfs_log_info("\n%s v%s (libntfs-3g) - Overwrite the unused space on an NTFS "
			"Volume.\n\n", EXEC_NAME, VERSION);
	ntfs_log_info("Copyright (c) 2002-2005 Richard Russon\n");
	ntfs_log_info("Copyright (c) 2004 Yura Pakhuchiy\n");
	ntfs_log_info("\n%s\n%s%s\n", ntfs_gpl, ntfs_bugs, ntfs_home);
}

/**
 * usage - Print a list of the parameters to the program
 *
 * Print a list of the parameters and options for the program.
 *
 * Return:  none
 */
static void usage(void)
{
	ntfs_log_info("\nUsage: %s [options] device\n"
		"    -i       --info        Show volume information (default)\n"
		"\n"
		"    -d       --directory   Wipe directory indexes\n"
		"    -l       --logfile     Wipe the logfile (journal)\n"
		"    -m       --mft         Wipe mft space\n"
		"    -p       --pagefile    Wipe pagefile (swap space)\n"
		"    -t       --tails       Wipe file tails\n"
		"    -u       --unused      Wipe unused clusters\n"
		"    -U       --unused-fast Wipe unused clusters (fast)\n"
		"    -s       --undel       Wipe undelete data\n"
		"\n"
		"    -a       --all         Wipe all unused space\n"
		"\n"
		"    -c num   --count num   Number of times to write(default = 1)\n"
		"    -b list  --bytes list  List of values to write(default = 0)\n"
		"\n"
		"    -n       --no-action   Do not write to disk\n"
		"    -f       --force       Use less caution\n"
		"    -q       --quiet       Less output\n"
		"    -v       --verbose     More output\n"
		"    -V       --version     Version information\n"
		"    -h       --help        Print this help\n\n",
		EXEC_NAME);
	ntfs_log_info("%s%s\n", ntfs_bugs, ntfs_home);
}

/**
 * parse_list - Read a comma-separated list of numbers
 * @list:    The comma-separated list of numbers
 * @result:  Store the parsed list here (must be freed by caller)
 *
 * Read a comma-separated list of numbers and allocate an array of ints to store
 * them in.  The numbers can be in decimal, octal or hex.
 *
 * N.B.  The caller must free the memory returned in @result.
 * N.B.  If the function fails, @result is not changed.
 *
 * Return:  0  Error, invalid string
 *	    n  Success, the count of numbers parsed
 */
static int parse_list(char *list, int **result)
{
	char *ptr;
	char *end;
	int i;
	int count;
	int *mem = NULL;

	if (!list || !result)
		return 0;

	for (count = 0, ptr = list; ptr; ptr = strchr(ptr+1, ','))
		count++;

	mem = malloc((count+1) * sizeof(int));
	if (!mem) {
		ntfs_log_error("Couldn't allocate memory in parse_list().\n");
		return 0;
	}

	memset(mem, 0xFF, (count+1) * sizeof(int));

	for (ptr = list, i = 0; i < count; i++) {

		end = NULL;
		mem[i] = strtol(ptr, &end, 0);

		if (!end || (end == ptr) || ((*end != ',') && (*end != 0))) {
			ntfs_log_error("Invalid list '%s'\n", list);
			free(mem);
			return 0;
		}

		if ((mem[i] < 0) || (mem[i] > 255)) {
			ntfs_log_error("Bytes must be in range 0-255.\n");
			free(mem);
			return 0;
		}

		ptr = end + 1;
	}

	ntfs_log_debug("Parsing list '%s' - ", list);
	for (i = 0; i <= count; i++)
		ntfs_log_debug("0x%02x ", mem[i]);
	ntfs_log_debug("\n");

	*result = mem;
	return count;
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
static int parse_options(int argc, char *argv[])
{
	static const char *sopt = "-ab:c:dfh?ilmnpqtuUvVs";
	static struct option lopt[] = {
		{ "all",	no_argument,		NULL, 'a' },
		{ "bytes",	required_argument,	NULL, 'b' },
		{ "count",	required_argument,	NULL, 'c' },
		{ "directory",	no_argument,		NULL, 'd' },
		{ "force",	no_argument,		NULL, 'f' },
		{ "help",	no_argument,		NULL, 'h' },
		{ "info",	no_argument,		NULL, 'i' },
		{ "logfile",	no_argument,		NULL, 'l' },
		{ "mft",	no_argument,		NULL, 'm' },
		{ "no-action",	no_argument,		NULL, 'n' },
		//{ "no-wait",	no_argument,		NULL, 0   },
		{ "pagefile",	no_argument,		NULL, 'p' },
		{ "quiet",	no_argument,		NULL, 'q' },
		{ "tails",	no_argument,		NULL, 't' },
		{ "unused",	no_argument,		NULL, 'u' },
		{ "unused-fast",no_argument,		NULL, 'U' },
		{ "undel",	no_argument,		NULL, 's' },
		{ "verbose",	no_argument,		NULL, 'v' },
		{ "version",	no_argument,		NULL, 'V' },
		{ NULL,		0,			NULL, 0   }
	};

	int c = -1;
	char *end;
	int err  = 0;
	int ver  = 0;
	int help = 0;
	int levels = 0;

	opterr = 0; /* We'll handle the errors, thank you. */

	opts.count = 1;

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (!opts.device) {
				opts.device = argv[optind-1];
			} else {
				opts.device = NULL;
				err++;
			}
			break;

		case 'i':
			opts.info++;		/* and fall through */
			/* FALLTHRU */
		case 'a':
			opts.directory++;
			opts.logfile++;
			opts.mft++;
			opts.pagefile++;
			opts.tails++;
			opts.unused++;
			opts.undel++;
			break;
		case 'b':
			if (!opts.bytes) {
				if (!parse_list(optarg, &opts.bytes))
					err++;
			} else {
				err++;
			}
			break;
		case 'c':
			if (opts.count == 1) {
				end = NULL;
				opts.count = strtol(optarg, &end, 0);
				if (end && *end)
					err++;
			} else {
				err++;
			}
			break;
		case 'd':
			opts.directory++;
			break;
		case 'f':
			opts.force++;
			break;
		case 'h':
			help++;
			break;
		case 'l':
			opts.logfile++;
			break;
		case 'm':
			opts.mft++;
			break;
		case 'n':
			opts.noaction++;
			break;
		case 'p':
			opts.pagefile++;
			break;
		case 'q':
			opts.quiet++;
			ntfs_log_clear_levels(NTFS_LOG_LEVEL_QUIET);
			break;
		case 's':
			opts.undel++;
			break;
		case 't':
			opts.tails++;
			break;
		case 'u':
			opts.unused++;
			break;
		case 'U':
			opts.unused_fast++;
			break;
		case 'v':
			opts.verbose++;
			ntfs_log_set_levels(NTFS_LOG_LEVEL_VERBOSE);
			break;
		case 'V':
			ver++;
			break;
		case '?':
			if (strncmp (argv[optind-1], "--log-", 6) == 0) {
				if (!ntfs_log_parse_option (argv[optind-1]))
					err++;
				break;
			}
			/* fall through */
		default:
			if ((optopt == 'b') || (optopt == 'c')) {
				ntfs_log_error("Option '%s' requires an argument.\n", argv[optind-1]);
			} else {
				ntfs_log_error("Unknown option '%s'.\n", argv[optind-1]);
			}
			err++;
			break;
		}
	}

	if (opts.bytes && opts.undel) {
		ntfs_log_error("Options --bytes and --undel are not compatible.\n");
		err++;
	}
	/* Make sure we're in sync with the log levels */
	levels = ntfs_log_get_levels();
	if (levels & NTFS_LOG_LEVEL_VERBOSE)
		opts.verbose++;
	if (!(levels & NTFS_LOG_LEVEL_QUIET))
		opts.quiet++;

	if (help || ver) {
		opts.quiet = 0;
	} else {
		if (opts.device == NULL) {
			if (argc > 1)
				ntfs_log_error("You must specify exactly one device.\n");
			err++;
		}

		if (opts.quiet && opts.verbose) {
			ntfs_log_error("You may not use --quiet and --verbose at the same time.\n");
			err++;
		}

		/*
		if (opts.info && (opts.unused || opts.tails || opts.mft || opts.directory)) {
			ntfs_log_error("You may not use any other options with --info.\n");
			err++;
		}
		*/

		if ((opts.count < 1) || (opts.count > 100)) {
			ntfs_log_error("The iteration count must be between 1 and 100.\n");
			err++;
		}

		/* Create a default list */
		if (!opts.bytes) {
			opts.bytes = malloc(2 * sizeof(int));
			if (opts.bytes) {
				opts.bytes[0] =  0;
				opts.bytes[1] = -1;
			} else {
				ntfs_log_error("Couldn't allocate memory for byte list.\n");
				err++;
			}
		}

		if (!opts.directory && !opts.logfile && !opts.mft &&
		    !opts.pagefile && !opts.tails && !opts.unused &&
		    !opts.unused_fast && !opts.undel) {
			opts.info = 1;
		}
	}

	if (ver)
		version();
	if (help || err)
		usage();

		/* tri-state 0 : done, 1 : error, -1 : proceed */
	return (err ? 1 : (help || ver ? 0 : -1));
}

/**
 * wipe_unused - Wipe unused clusters
 * @vol:   An ntfs volume obtained from ntfs_mount
 * @byte:  Overwrite with this value
 * @act:   Wipe, test or info
 *
 * Read $Bitmap and wipe any clusters that are marked as not in use.
 *
 * Return: >0  Success, the attribute was wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_unused(ntfs_volume *vol, int byte, enum action act)
{
	s64 i;
	s64 total = 0;
	s64 result = 0;
	u8 *buffer = NULL;

	if (!vol || (byte < 0))
		return -1;

	if (act != act_info) {
		buffer = malloc(vol->cluster_size);
		if (!buffer) {
			ntfs_log_error("malloc failed\n");
			return -1;
		}
		memset(buffer, byte, vol->cluster_size);
	}

	for (i = 0; i < vol->nr_clusters; i++) {
		if (utils_cluster_in_use(vol, i)) {
			//ntfs_log_verbose("cluster %lld is in use\n", i);
			continue;
		}

		if (act == act_wipe) {
			//ntfs_log_verbose("cluster %lld is not in use\n", i);
			result = ntfs_pwrite(vol->dev, vol->cluster_size * i, vol->cluster_size, buffer);
			if (result != vol->cluster_size) {
				ntfs_log_error("write failed\n");
				goto free;
			}
		}

		total += vol->cluster_size;
	}

	ntfs_log_quiet("wipe_unused 0x%02x, %lld bytes\n", byte, (long long)total);
free:
	free(buffer);
	return total;
}

/**
 * wipe_unused_fast - Faster wipe unused clusters
 * @vol:   An ntfs volume obtained from ntfs_mount
 * @byte:  Overwrite with this value
 * @act:   Wipe, test or info
 *
 * Read $Bitmap and wipe any clusters that are marked as not in use.
 *
 * - read/write on a block basis (64 clusters, arbitrary)
 * - skip of fully used block
 * - skip non-used block already wiped
 *
 * Return: >0  Success, the attribute was wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_unused_fast(ntfs_volume *vol, int byte, enum action act)
{
	s64 i;
	s64 total = 0;
	s64 unused = 0;
	s64 result;
	u8 *buffer;
	u8 *big_buffer;
	u32 *u32_buffer;
	u32 u32_bytes;
	unsigned int blksize;
	unsigned int j,k;
	BOOL wipe_needed;

	if (!vol || (byte < 0))
		return -1;

	big_buffer = (u8*)malloc(vol->cluster_size*64);
	if (!big_buffer) {
		ntfs_log_error("malloc failed\n");
		return -1;
	}

	for (i = 0; i < vol->nr_clusters; i+=64) {
		blksize = vol->nr_clusters - i;
		if (blksize > 64)
			blksize = 64;
	   /* if all clusters in this block are used, ignore the block */
		result = 0;
		for (j = 0; j < blksize; j++) {
			if (utils_cluster_in_use(vol, i+j))
				result++;
		}
		unused += (blksize - result) * vol->cluster_size;

		if (result == blksize) {
			continue;
		}
		/*
		 * if all unused clusters in this block are already wiped,
		 * ignore the block
		 */
		if (ntfs_pread(vol->dev, vol->cluster_size * i,
					vol->cluster_size * blksize, big_buffer)
				!= vol->cluster_size * blksize) {
			ntfs_log_error("Read failed at cluster %lld\n",
					(long long)i);
			goto free;
		}

		result = 0;
		wipe_needed = FALSE;
		u32_bytes = (byte & 255)*0x01010101;
		buffer = big_buffer;
		for (j = 0; (j < blksize) && !wipe_needed; j++) {
			u32_buffer = (u32*)buffer;
			if (!utils_cluster_in_use(vol, i+j)) {
				for (k = 0; (k < vol->cluster_size)
					&& (*u32_buffer++ == u32_bytes); k+=4) {
				}
				if (k < vol->cluster_size)
					wipe_needed = TRUE;
			}
			buffer += vol->cluster_size;
		}

		if (!wipe_needed) { 
			continue;
		}
			/* else wipe unused clusters in the block */
		buffer = big_buffer;

		for (j = 0; j < blksize; j++) {
			if (!utils_cluster_in_use(vol, i+j)) {
				memset(buffer, byte, vol->cluster_size);
				total += vol->cluster_size;
			}
			buffer += vol->cluster_size;
		}

		if ((act == act_wipe)
			&& (ntfs_pwrite(vol->dev, vol->cluster_size * i,
				vol->cluster_size * blksize, big_buffer)
					!= vol->cluster_size * blksize)) {
			ntfs_log_error("Write failed at cluster %lld\n",
					(long long)i);
			goto free;
		}
	}

	ntfs_log_quiet("wipe_unused_fast 0x%02x, %lld bytes"
			" already wiped, %lld more bytes wiped\n",
			byte, (long long)(unused - total), (long long)total);
free:
	free(big_buffer);
	return total;
}

/**
 * wipe_compressed_attribute - Wipe compressed $DATA attribute
 * @vol:	An ntfs volume obtained from ntfs_mount
 * @byte:	Overwrite with this value
 * @act:	Wipe, test or info
 * @na:		Opened ntfs attribute
 *
 * Return: >0  Success, the attribute was wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_compressed_attribute(ntfs_volume *vol, int byte,
						enum action act, ntfs_attr *na)
{
	unsigned char *buf;
	s64 size, offset, ret, wiped = 0;
	le16 block_size_le;
	u16 block_size;
	VCN cur_vcn = 0;
	runlist_element *rlc = na->rl;
	s64 cu_mask = na->compression_block_clusters - 1;
	runlist_element *restart = na->rl;

	while (rlc->length) {
		cur_vcn += rlc->length;
		if ((cur_vcn & cu_mask) ||
			(((rlc + 1)->length) && (rlc->lcn != LCN_HOLE))) {
			rlc++;
			continue;
		}

		if (rlc->lcn == LCN_HOLE) {
			runlist_element *rlt;

			offset = cur_vcn - rlc->length;
			if (offset == (offset & (~cu_mask))) {
				restart = rlc + 1;
				rlc++;
				continue;
			}
			offset = (offset & (~cu_mask))
						<< vol->cluster_size_bits;
			rlt = rlc;
			while ((rlt - 1)->lcn == LCN_HOLE) rlt--;
			while (1) {
				ret = ntfs_rl_pread(vol, restart,
    					offset - (restart->vcn
    					<< vol->cluster_size_bits),
					2, &block_size_le);
				block_size = le16_to_cpu(block_size_le);
				if (ret != 2) {
					ntfs_log_verbose("Internal error\n");
					ntfs_log_error("ntfs_rl_pread failed");
					return -1;
				}
				if (block_size == 0) {
					offset += 2;
					break;
				}
				block_size &= 0x0FFF;
				block_size += 3;
				offset += block_size;
				if (offset >= (((rlt->vcn) <<
						vol->cluster_size_bits) - 2))
					goto next;
			}
			size = (rlt->vcn << vol->cluster_size_bits) - offset;
		} else {
			size = na->allocated_size - na->data_size;
			offset = (cur_vcn << vol->cluster_size_bits) - size;
		}

		if (size < 0) {
			ntfs_log_verbose("Internal error\n");
			ntfs_log_error("bug or damaged fs: we want "
				"allocate buffer size %lld bytes",
				(long long)size);
			return -1;
		}

		if ((act == act_info) || (!size)) {
			wiped += size;
			if (rlc->lcn == LCN_HOLE)
				restart = rlc + 1;
			rlc++;
			continue;
		}

		buf = malloc(size);
		if (!buf) {
			ntfs_log_verbose("Not enough memory\n");
			ntfs_log_error("Not enough memory to allocate "
							"%lld bytes",
							(long long)size);
			return -1;
		}
		memset(buf, byte, size);

		ret = ntfs_rl_pwrite(vol, restart,
    			restart->vcn << vol->cluster_size_bits,
			offset, size, buf);
		free(buf);
		if (ret != size) {
			ntfs_log_verbose("Internal error\n");
			ntfs_log_error("ntfs_rl_pwrite failed, offset %llu, "
				"size %lld, vcn %lld",
				(unsigned long long)offset,
				(long long)size, (long long)rlc->vcn);
			return -1;
		}
		wiped += ret;
next:
		if (rlc->lcn == LCN_HOLE)
			restart = rlc + 1;
		rlc++;
	}

	return wiped;
}

/**
 * wipe_attribute - Wipe not compressed $DATA attribute
 * @vol:	An ntfs volume obtained from ntfs_mount
 * @byte:	Overwrite with this value
 * @act:	Wipe, test or info
 * @na:		Opened ntfs attribute
 *
 * Return: >0  Success, the attribute was wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_attribute(ntfs_volume *vol, int byte, enum action act,
								ntfs_attr *na)
{
	unsigned char *buf;
	s64 wiped;
	s64 size;
	u64 offset = na->data_size;

	if (!offset)
		return 0;
	if (na->data_flags & ATTR_IS_ENCRYPTED)
		offset = (((offset - 1) >> 10) + 1) << 10;
	size = (vol->cluster_size - offset) % vol->cluster_size;

	if (act == act_info)
		return size;

	buf = malloc(size);
	if (!buf) {
		ntfs_log_verbose("Not enough memory\n");
		ntfs_log_error("Not enough memory to allocate %lld bytes",
					(long long)size);
		return -1;
	}
	memset(buf, byte, size);

	wiped = ntfs_rl_pwrite(vol, na->rl, 0, offset, size, buf);
	if (wiped == -1) {
		ntfs_log_verbose("Internal error\n");
		ntfs_log_error("Couldn't wipe tail");
	}

	free(buf);
	return wiped;
}

/*
 *		Wipe a data attribute tail
 *
 * Return: >0  Success, the clusters were wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */

static s64 wipe_attr_tail(ntfs_inode *ni, ntfschar *name, int namelen,
					int byte, enum action act)
{
	ntfs_attr *na;
	ntfs_volume *vol = ni->vol;
	s64 wiped;

	wiped = -1;
	na = ntfs_attr_open(ni, AT_DATA, name, namelen);
	if (!na) {
		ntfs_log_error("Couldn't open $DATA attribute\n");
		goto close_attr;
	}

	if (!NAttrNonResident(na)) {
		ntfs_log_verbose("Resident $DATA attribute. Skipping.\n");
		goto close_attr;
	}

	if (ntfs_attr_map_whole_runlist(na)) {
		ntfs_log_verbose("Internal error\n");
		ntfs_log_error("Can't map runlist (inode %lld)\n",
				(long long)ni->mft_no);
		goto close_attr;
	}

	if (na->data_flags & ATTR_COMPRESSION_MASK)
		wiped = wipe_compressed_attribute(vol, byte, act, na);
	else
		wiped = wipe_attribute(vol, byte, act, na);

	if (wiped == -1) {
		ntfs_log_error(" (inode %lld)\n", (long long)ni->mft_no);
	}

close_attr:
	ntfs_attr_close(na);
	return (wiped);
}

/**
 * wipe_tails - Wipe the file tails in all its data attributes
 * @vol:   An ntfs volume obtained from ntfs_mount
 * @byte:  Overwrite with this value
 * @act:   Wipe, test or info
 *
 * Disk space is allocated in clusters.  If a file isn't an exact multiple of
 * the cluster size, there is some slack space at the end.  Wipe this space.
 *
 * Return: >0  Success, the clusters were wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_tails(ntfs_volume *vol, int byte, enum action act)
{
	s64 total = 0;
	s64 nr_mft_records, inode_num;
	ntfs_attr_search_ctx *ctx;
	ntfs_inode *ni;
	ATTR_RECORD *a;
	ntfschar *name;

	if (!vol || (byte < 0))
		return -1;

	nr_mft_records = vol->mft_na->initialized_size >>
			vol->mft_record_size_bits;

		/* Avoid getting fixup warnings on unitialized inodes */
	NVolSetNoFixupWarn(vol);

	for (inode_num = FILE_first_user; inode_num < nr_mft_records;
							inode_num++) {
		s64 attr_wiped;
		s64 wiped = 0;

		ntfs_log_verbose("Inode %lld - ", (long long)inode_num);
		ni = ntfs_inode_open(vol, inode_num);
		if (!ni) {
			if (opts.verbose)
				ntfs_log_verbose("Could not open inode\n");
			else
				ntfs_log_verbose("\r");
			continue;
		}

		if (ni->mrec->base_mft_record) {
			ntfs_log_verbose("Not base mft record. Skipping\n");
			goto close_inode;
		}

		ctx = ntfs_attr_get_search_ctx(ni, (MFT_RECORD*)NULL);
		if (!ctx) {
			ntfs_log_error("Can't get a context, aborting\n");
			ntfs_inode_close(ni);
			goto close_abort;
		}
		while (!ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE, 0,
							NULL, 0, ctx)) {
			a = ctx->attr;

			if (!ctx->al_entry || !ctx->al_entry->lowest_vcn) {
				name = (ntfschar*)((u8*)a
						+ le16_to_cpu(a->name_offset));
				attr_wiped = wipe_attr_tail(ni, name,
						a->name_length, byte, act);
				if (attr_wiped > 0)
					wiped += attr_wiped;
			}
		}
		ntfs_attr_put_search_ctx(ctx);
		if (wiped) {
			ntfs_log_verbose("Wiped %llu bytes\n",
					(unsigned long long)wiped);
			total += wiped;
		} else
			ntfs_log_verbose("Nothing to wipe\n");
close_inode:
		ntfs_inode_close(ni);
	}
close_abort :
	NVolClearNoFixupWarn(vol);
	ntfs_log_quiet("wipe_tails 0x%02x, %lld bytes\n", byte,
				(long long)total);
	return total;
}

/**
 * wipe_mft - Wipe the MFT slack space
 * @vol:   An ntfs volume obtained from ntfs_mount
 * @byte:  Overwrite with this value
 * @act:   Wipe, test or info
 *
 * MFT Records are 1024 bytes long, but some of this space isn't used.  Wipe any
 * unused space at the end of the record and wipe any unused records.
 *
 * Return: >0  Success, the clusters were wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_mft(ntfs_volume *vol, int byte, enum action act)
{
	// by considering the individual attributes we might be able to
	// wipe a few more bytes at the attr's tail.
	s64 nr_mft_records, i;
	s64 total = 0;
	s64 result = 0;
	int size = 0;
	MFT_RECORD *rec = NULL;

	if (!vol || (byte < 0))
		return -1;

	rec = (MFT_RECORD*)malloc(vol->mft_record_size);
	if (!rec) {
		ntfs_log_error("malloc failed\n");
		return -1;
	}

	nr_mft_records = vol->mft_na->initialized_size >>
			vol->mft_record_size_bits;

	for (i = 0; i < nr_mft_records; i++) {
		if (utils_mftrec_in_use(vol, i)) {
			result = ntfs_attr_mst_pread(vol->mft_na, vol->mft_record_size * i,
				1, vol->mft_record_size, rec);
			if (result != 1) {
				ntfs_log_error("error attr mst read %lld\n",
						(long long)i);
				total = -1;	// XXX just negate result?
				goto free;
			}

			// We know that the end marker will only take 4 bytes
			size = le32_to_cpu(rec->bytes_in_use) - 4;

			if ((size <= 0) || (size > (int)vol->mft_record_size)) {
				ntfs_log_error("Bad mft record %lld\n",
						(long long)i);
				total = -1;
				goto free;
			}
			if (act == act_info) {
				//ntfs_log_info("mft %d\n", size);
				total += size;
				continue;
			}

			memset(((u8*) rec) + size, byte, vol->mft_record_size - size);
		} else {
			const u16 usa_offset =
				(vol->major_ver == 3) ? 0x0030 : 0x002A;
			const u32 usa_size = 1 +
				(vol->mft_record_size >> NTFS_BLOCK_SIZE_BITS);
			const u16 attrs_offset =
				((usa_offset + usa_size) + 7) & ~((u16) 7);
			const u32 bytes_in_use = attrs_offset + 8;

			if(usa_size > 0xFFFF || (usa_offset + usa_size) >
				(NTFS_BLOCK_SIZE - sizeof(u16)))
			{
				ntfs_log_error("%d: usa_size out of bounds "
					"(%u)\n", __LINE__, usa_size);
				total = -1;
				goto free;
			}

			if (act == act_info) {
				total += vol->mft_record_size;
				continue;
			}

			// Build the record from scratch
			memset(rec, 0, vol->mft_record_size);

			// Common values
			rec->magic = magic_FILE;
			rec->usa_ofs = cpu_to_le16(usa_offset);
			rec->usa_count = cpu_to_le16((u16) usa_size);
			rec->sequence_number = const_cpu_to_le16(0x0001);
			rec->attrs_offset = cpu_to_le16(attrs_offset);
			rec->bytes_in_use = cpu_to_le32(bytes_in_use);
			rec->bytes_allocated = cpu_to_le32(vol->mft_record_size);
			rec->next_attr_instance = const_cpu_to_le16(0x0001);

			// End marker.
			*((le32*) (((u8*) rec) + attrs_offset)) = const_cpu_to_le32(0xFFFFFFFF);
		}

		result = ntfs_attr_mst_pwrite(vol->mft_na, vol->mft_record_size * i,
			1, vol->mft_record_size, rec);
		if (result != 1) {
			ntfs_log_error("error attr mst write %lld\n",
					(long long)i);
			total = -1;
			goto free;
		}

		if ((vol->mft_record_size * (i+1)) <= vol->mftmirr_na->allocated_size)
		{
			// We have to reduce the update sequence number, or else...
			u16 offset;
			le16 *usnp;
			offset = le16_to_cpu(rec->usa_ofs);
			usnp = (le16*) (((u8*) rec) + offset);
			*usnp = cpu_to_le16(le16_to_cpu(*usnp) - 1);

			result = ntfs_attr_mst_pwrite(vol->mftmirr_na, vol->mft_record_size * i,
				1, vol->mft_record_size, rec);
			if (result != 1) {
				ntfs_log_error("error attr mst write %lld\n",
						(long long)i);
				total = -1;
				goto free;
			}
		}

		total += vol->mft_record_size;
	}

	ntfs_log_quiet("wipe_mft 0x%02x, %lld bytes\n", byte, (long long)total);
free:
	free(rec);
	return total;
}

/**
 * wipe_index_allocation - Wipe $INDEX_ALLOCATION attribute
 * @vol:		An ntfs volume obtained from ntfs_mount
 * @byte:		Overwrite with this value
 * @act:		Wipe, test or info
 * @naa:		Opened ntfs $INDEX_ALLOCATION attribute
 * @nab:		Opened ntfs $BITMAP attribute
 * @indx_record_size:	Size of INDX record
 *
 * Return: >0  Success, the clusters were wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_index_allocation(ntfs_volume *vol, int byte, enum action act
	__attribute__((unused)), ntfs_attr *naa, ntfs_attr *nab,
	u32 indx_record_size)
{
	s64 total = 0;
	s64 wiped = 0;
	s64 offset = 0;
	s64 obyte = 0;
	u64 wipe_offset;
	s64 wipe_size;
	u8 obit = 0;
	u8 mask;
	u8 *bitmap;
	u8 *buf;

	bitmap = malloc(nab->data_size);
	if (!bitmap) {
		ntfs_log_verbose("malloc failed\n");
		ntfs_log_error("Couldn't allocate %lld bytes",
				(long long)nab->data_size);
		return -1;
	}

	if (ntfs_attr_pread(nab, 0, nab->data_size, bitmap)
						!= nab->data_size) {
		ntfs_log_verbose("Internal error\n");
		ntfs_log_error("Couldn't read $BITMAP");
		total = -1;
		goto free_bitmap;
	}

	buf = malloc(indx_record_size);
	if (!buf) {
		ntfs_log_verbose("malloc failed\n");
		ntfs_log_error("Couldn't allocate %u bytes",
				(unsigned int)indx_record_size);
		total = -1;
		goto free_bitmap;
	}

	while (offset < naa->allocated_size) {
		mask = 1 << obit;
		if (bitmap[obyte] & mask) {
			INDEX_ALLOCATION *indx;

			s64 ret = ntfs_rl_pread(vol, naa->rl,
					offset, indx_record_size, buf);
			if (ret != indx_record_size) {
				ntfs_log_verbose("ntfs_rl_pread failed\n");
				ntfs_log_error("Couldn't read INDX record");
				total = -1;
				goto free_buf;
			}

			indx = (INDEX_ALLOCATION *) buf;
			if (ntfs_mst_post_read_fixup((NTFS_RECORD *)buf,
								indx_record_size))
				ntfs_log_error("damaged fs: mst_post_read_fixup failed");

			if ((le32_to_cpu(indx->index.allocated_size) + 0x18) !=
							indx_record_size) {
				ntfs_log_verbose("Internal error\n");
				ntfs_log_error("INDX record should be %u bytes",
						(unsigned int)indx_record_size);
				total = -1;
				goto free_buf;
			}

			wipe_offset = le32_to_cpu(indx->index.index_length) + 0x18;
			wipe_size = indx_record_size - wipe_offset;
			memset(buf + wipe_offset, byte, wipe_size);
			if (ntfs_mst_pre_write_fixup((NTFS_RECORD *)indx,
								indx_record_size))
				ntfs_log_error("damaged fs: mst_pre_write_protect failed");
			if (opts.verbose > 1)
				ntfs_log_verbose("+");
		} else {
			wipe_size = indx_record_size;
			memset(buf, byte, wipe_size);
			if (opts.verbose > 1)
				ntfs_log_verbose("x");
		}

		wiped = ntfs_rl_pwrite(vol, naa->rl, 0, offset, indx_record_size, buf);
		if (wiped != indx_record_size) {
			ntfs_log_verbose("ntfs_rl_pwrite failed\n");
			ntfs_log_error("Couldn't wipe tail of INDX record");
			total = -1;
			goto free_buf;
		}
		total += wipe_size;

		offset += indx_record_size;
		obit++;
		if (obit > 7) {
			obit = 0;
			obyte++;
		}
	}
	if ((opts.verbose > 1) && (wiped != -1))
		ntfs_log_verbose("\n\t");
free_buf:
	free(buf);
free_bitmap:
	free(bitmap);
	return total;
}

/**
 * get_indx_record_size - determine size of INDX record from $INDEX_ROOT
 * @nar:	Opened ntfs $INDEX_ROOT attribute
 *
 * Return: >0  Success, return INDX record size
 *          0  Error, something went wrong
 */
static u32 get_indx_record_size(ntfs_attr *nar)
{
	u32 indx_record_size;
	le32 indx_record_size_le;

	if (ntfs_attr_pread(nar, 8, 4, &indx_record_size_le) != 4) {
		ntfs_log_verbose("Couldn't determine size of INDX record\n");
		ntfs_log_error("ntfs_attr_pread failed");
		return 0;
	}

	indx_record_size = le32_to_cpu(indx_record_size_le);
	if (!indx_record_size) {
		ntfs_log_verbose("Internal error\n");
		ntfs_log_error("INDX record should be 0");
	}
	return indx_record_size;
}

/**
 * wipe_directory - Wipe the directory indexes
 * @vol:	An ntfs volume obtained from ntfs_mount
 * @byte:	Overwrite with this value
 * @act:	Wipe, test or info
 *
 * Directories are kept in sorted B+ Trees.  Index blocks may not be full.  Wipe
 * the unused space at the ends of these blocks.
 *
 * Return: >0  Success, the clusters were wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_directory(ntfs_volume *vol, int byte, enum action act)
{
	s64 total = 0;
	s64 nr_mft_records, inode_num;
	ntfs_inode *ni;
	ntfs_attr *naa;
	ntfs_attr *nab;
	ntfs_attr *nar;

	if (!vol || (byte < 0))
		return -1;

	nr_mft_records = vol->mft_na->initialized_size >>
			vol->mft_record_size_bits;

		/* Avoid getting fixup warnings on unitialized inodes */
	NVolSetNoFixupWarn(vol);

	for (inode_num = 5; inode_num < nr_mft_records; inode_num++) {
		u32 indx_record_size;
		s64 wiped;

		ntfs_log_verbose("Inode %lld - ", (long long)inode_num);
		ni = ntfs_inode_open(vol, inode_num);
		if (!ni) {
			if (opts.verbose > 2)
				ntfs_log_verbose("Could not open inode\n");
			else
				ntfs_log_verbose("\r");
			continue;
		}

		if (ni->mrec->base_mft_record) {
			if (opts.verbose > 2)
				ntfs_log_verbose("Not base mft record. Skipping\n");
			else
				ntfs_log_verbose("\r");
			goto close_inode;
		}

		naa = ntfs_attr_open(ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
		if (!naa) {
			if (opts.verbose > 2)
				ntfs_log_verbose("Couldn't open $INDEX_ALLOCATION\n");
			else
				ntfs_log_verbose("\r");
			goto close_inode;
		}

		if (!NAttrNonResident(naa)) {
			ntfs_log_verbose("Resident $INDEX_ALLOCATION\n");
			ntfs_log_error("damaged fs: Resident $INDEX_ALLOCATION "
					"(inode %lld)\n", (long long)inode_num);
			goto close_attr_allocation;
		}

		if (ntfs_attr_map_whole_runlist(naa)) {
			ntfs_log_verbose("Internal error\n");
			ntfs_log_error("Can't map runlist for $INDEX_ALLOCATION "
					"(inode %lld)\n", (long long)inode_num);
			goto close_attr_allocation;
		}

		nab = ntfs_attr_open(ni, AT_BITMAP, NTFS_INDEX_I30, 4);
		if (!nab) {
			ntfs_log_verbose("Couldn't open $BITMAP\n");
			ntfs_log_error("damaged fs: $INDEX_ALLOCATION is present, "
					"but we can't open $BITMAP with same "
					"name (inode %lld)\n", (long long)inode_num);
			goto close_attr_allocation;
		}

		nar = ntfs_attr_open(ni, AT_INDEX_ROOT, NTFS_INDEX_I30, 4);
		if (!nar) {
			ntfs_log_verbose("Couldn't open $INDEX_ROOT\n");
			ntfs_log_error("damaged fs: $INDEX_ALLOCATION is present, but "
					"we can't open $INDEX_ROOT with same name"
					" (inode %lld)\n", (long long)inode_num);
			goto close_attr_bitmap;
		}

		if (NAttrNonResident(nar)) {
			ntfs_log_verbose("Not resident $INDEX_ROOT\n");
			ntfs_log_error("damaged fs: Not resident $INDEX_ROOT "
					"(inode %lld)\n", (long long)inode_num);
			goto close_attr_root;
		}

		indx_record_size = get_indx_record_size(nar);
		if (!indx_record_size) {
			ntfs_log_error(" (inode %lld)\n", (long long)inode_num);
			goto close_attr_root;
		}

		wiped = wipe_index_allocation(vol, byte, act,
						naa, nab, indx_record_size);
		if (wiped == -1) {
			ntfs_log_error(" (inode %lld)\n",
					(long long)inode_num);
			goto close_attr_root;
		}

		if (wiped) {
			ntfs_log_verbose("Wiped %llu bytes\n",
					(unsigned long long)wiped);
			total += wiped;
		} else
			ntfs_log_verbose("Nothing to wipe\n");
close_attr_root:
		ntfs_attr_close(nar);
close_attr_bitmap:
		ntfs_attr_close(nab);
close_attr_allocation:
		ntfs_attr_close(naa);
close_inode:
		ntfs_inode_close(ni);
	}

	NVolClearNoFixupWarn(vol);
	ntfs_log_quiet("wipe_directory 0x%02x, %lld bytes\n", byte,
			(long long)total);
	return total;
}

/**
 * wipe_logfile - Wipe the logfile (journal)
 * @vol:   An ntfs volume obtained from ntfs_mount
 * @byte:  Overwrite with this value
 * @act:   Wipe, test or info
 *
 * The logfile journals the metadata to give the volume fault-tolerance.  If the
 * volume is in a consistent state, then this information can be erased.
 *
 * Return: >0  Success, the clusters were wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_logfile(ntfs_volume *vol, int byte, enum action act
	__attribute__((unused)))
{
	const int NTFS_BUF_SIZE2 = 8192;
	//FIXME(?): We might need to zero the LSN field of every single mft
	//record as well. (But, first try without doing that and see what
	//happens, since chkdsk might pickup the pieces and do it for us...)
	ntfs_inode *ni;
	ntfs_attr *na;
	s64 len, pos, count;
	char buf[NTFS_BUF_SIZE2];
	int eo;

	/* We can wipe logfile only with 0xff. */
	byte = 0xff;

	if (!vol || (byte < 0))
		return -1;

	//ntfs_log_quiet("wipe_logfile(not implemented) 0x%02x\n", byte);

	if ((ni = ntfs_inode_open(vol, FILE_LogFile)) == NULL) {
		ntfs_log_debug("Failed to open inode FILE_LogFile.\n");
		return -1;
	}

	if ((na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0)) == NULL) {
		ntfs_log_debug("Failed to open $FILE_LogFile/$DATA.\n");
		goto error_exit;
	}

	/* The $DATA attribute of the $LogFile has to be non-resident. */
	if (!NAttrNonResident(na)) {
		ntfs_log_debug("$LogFile $DATA attribute is resident!?!\n");
		errno = EIO;
		goto io_error_exit;
	}

	/* Get length of $LogFile contents. */
	len = na->data_size;
	if (!len) {
		ntfs_log_debug("$LogFile has zero length, no disk write "
				"needed.\n");
		return 0;
	}

	/* Read $LogFile until its end. We do this as a check for correct
	   length thus making sure we are decompressing the mapping pairs
	   array correctly and hence writing below is safe as well. */
	pos = 0;
	while ((count = ntfs_attr_pread(na, pos, NTFS_BUF_SIZE2, buf)) > 0)
		pos += count;

	if (count == -1 || pos != len) {
		ntfs_log_debug("Amount of $LogFile data read does not "
			"correspond to expected length!\n");
		if (count != -1)
			errno = EIO;
		goto io_error_exit;
	}

	/* Fill the buffer with @byte's. */
	memset(buf, byte, NTFS_BUF_SIZE2);

	/* Set the $DATA attribute. */
	pos = 0;
	while ((count = len - pos) > 0) {
		if (count > NTFS_BUF_SIZE2)
			count = NTFS_BUF_SIZE2;

		if ((count = ntfs_attr_pwrite(na, pos, count, buf)) <= 0) {
			ntfs_log_debug("Failed to set the $LogFile attribute "
					"value.\n");
			if (count != -1)
				errno = EIO;
			goto io_error_exit;
		}

		pos += count;
	}

	ntfs_attr_close(na);
	ntfs_inode_close(ni);
	ntfs_log_quiet("wipe_logfile 0x%02x, %lld bytes\n", byte,
			(long long)pos);
	return pos;

io_error_exit:
	eo = errno;
	ntfs_attr_close(na);
	errno = eo;
error_exit:
	eo = errno;
	ntfs_inode_close(ni);
	errno = eo;
	return -1;
}

/**
 * wipe_pagefile - Wipe the pagefile (swap space)
 * @vol:   An ntfs volume obtained from ntfs_mount
 * @byte:  Overwrite with this value
 * @act:   Wipe, test or info
 *
 * pagefile.sys is used by Windows as extra virtual memory (swap space).
 * Windows recreates the file at bootup, so it can be wiped without harm.
 *
 * Return: >0  Success, the clusters were wiped
 *          0  Nothing to wipe
 *         -1  Error, something went wrong
 */
static s64 wipe_pagefile(ntfs_volume *vol, int byte, enum action act
	__attribute__((unused)))
{
	// wipe completely, chkdsk doesn't do anything, booting writes header
	const int NTFS_BUF_SIZE2 = 4096;
	ntfs_inode *ni;
	ntfs_attr *na;
	s64 len, pos, count;
	char buf[NTFS_BUF_SIZE2];
	int eo;

	if (!vol || (byte < 0))
		return -1;

	//ntfs_log_quiet("wipe_pagefile(not implemented) 0x%02x\n", byte);

	ni = ntfs_pathname_to_inode(vol, NULL, "pagefile.sys");
	if (!ni) {
		ntfs_log_debug("Failed to open inode of pagefile.sys.\n");
		return 0;
	}

	if ((na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0)) == NULL) {
		ntfs_log_debug("Failed to open pagefile.sys/$DATA.\n");
		goto error_exit;
	}

	/* The $DATA attribute of the pagefile.sys has to be non-resident. */
	if (!NAttrNonResident(na)) {
		ntfs_log_debug("pagefile.sys $DATA attribute is resident!?!\n");
		errno = EIO;
		goto io_error_exit;
	}

	/* Get length of pagefile.sys contents. */
	len = na->data_size;
	if (!len) {
		ntfs_log_debug("pagefile.sys has zero length, no disk write "
				"needed.\n");
		return 0;
	}

	memset(buf, byte, NTFS_BUF_SIZE2);

	/* Set the $DATA attribute. */
	pos = 0;
	while ((count = len - pos) > 0) {
		if (count > NTFS_BUF_SIZE2)
			count = NTFS_BUF_SIZE2;

		if ((count = ntfs_attr_pwrite(na, pos, count, buf)) <= 0) {
			ntfs_log_debug("Failed to set the pagefile.sys "
					"attribute value.\n");
			if (count != -1)
				errno = EIO;
			goto io_error_exit;
		}

		pos += count;
	}

	ntfs_attr_close(na);
	ntfs_inode_close(ni);
	ntfs_log_quiet("wipe_pagefile 0x%02x, %lld bytes\n", byte,
			(long long)pos);
	return pos;

io_error_exit:
	eo = errno;
	ntfs_attr_close(na);
	errno = eo;
error_exit:
	eo = errno;
	ntfs_inode_close(ni);
	errno = eo;
	return -1;
}

/**
 * Part of ntfsprogs.
 * Modified: removed logging, signal handling, removed data.
 *
 * free_file - Release the resources used by a file object
 * \param file  The unwanted file object
 *
 * This will free up the memory used by a file object and iterate through the
 * object's children, freeing their resources too.
 *
 * \return  none
 */
static void free_file (struct ufile *file)
{
	struct ntfs_list_head *item = NULL, *tmp = NULL;
	struct filename *f = NULL;
	struct data *d = NULL;

	if (file == NULL)
		return;

	ntfs_list_for_each_safe(item, tmp, &(file->name)) {
		/* List of filenames */

		f = ntfs_list_entry(item, struct filename, list);
		if (f->name != NULL)
			free(f->name);
		if (f->parent_name != NULL) {
			free(f->parent_name);
		}
		free(f);
	}

	ntfs_list_for_each_safe(item, tmp, &(file->data)) {
		/* List of data streams */

		d = ntfs_list_entry(item, struct data, list);
		if (d->name != NULL)
			free(d->name);
		if (d->runlist != NULL)
			free(d->runlist);
		free(d);
	}


	free(file->mft);
	free(file);
}

/**
 * Fills the given buffer with one of predefined patterns.
 * \param pat_no Pass number.
 * \param buffer Buffer to be filled.
 * \param buflen Length of the buffer.
 */
static void fill_buffer (
		unsigned long int 		pat_no,
		unsigned char * const 		buffer,
		const size_t 			buflen,
		int * const			selected )
		/*@requires notnull buffer @*/ /*@sets *buffer @*/
{

	size_t i;
#if (!defined HAVE_MEMCPY) && (!defined HAVE_STRING_H)
	size_t j;
#endif
	unsigned int bits;

	if ((buffer == NULL) || (buflen == 0))
		return;

	/* De-select all patterns once every npasses calls. */
	if (pat_no % npasses == 0) {
		for (i = 0; i < NPAT; i++) {
			selected[i] = 0;
		}
        }
        pat_no %= npasses;
	/* double check for npasses >= NPAT + 3: */
        for (i = 0; i < NPAT; i++) {
		if (selected[i] == 0)
			break;
	}
	if (i >= NPAT) {
		for (i = 0; i < NPAT; i++) {
			selected[i] = 0;
		}
	}

	/* The first, last and middle passess will be using a random pattern */
	if ((pat_no == 0) || (pat_no == npasses-1) || (pat_no == npasses/2)) {
#if (!defined __STRICT_ANSI__) && (defined HAVE_RANDOM)
		bits = (unsigned int)(random() & 0xFFF);
#else
		bits = (unsigned int)(rand() & 0xFFF);
#endif
	} else {
		/* For other passes, one of the fixed patterns is selected. */
		do {
#if (!defined __STRICT_ANSI__) && (defined HAVE_RANDOM)
			i = (size_t)random() % NPAT;
#else
			i = (size_t)rand() % NPAT;
#endif
		} while (selected[i] == 1);
		bits = 	patterns[i];
		selected[i] = 1;
    	}

	buffer[0] = (unsigned char) bits;
	buffer[1] = (unsigned char) bits;
	buffer[2] = (unsigned char) bits;
	for (i = 3; i < buflen / 2; i *= 2) {
#ifdef HAVE_MEMCPY
		memcpy(buffer + i, buffer, i);
#elif defined HAVE_STRING_H
		strncpy((char *)(buffer + i), (char *)buffer, i);
#else
		for (j = 0; j < i; j++) {
			buffer[i+j] = buffer[j];
		}
#endif
	}
	if (i < buflen) {
#ifdef HAVE_MEMCPY
		memcpy(buffer + i, buffer, buflen - i);
#elif defined HAVE_STRING_H
		strncpy((char *)(buffer + i), (char *)buffer, buflen - i);
#else
		for (j=0; j<buflen - i; j++) {
			buffer[i+j] = buffer[j];
		}
#endif
	}
}

/**
 * Destroys the specified record's filenames and data.
 *
 * \param nv The filesystem.
 * \param record The record (i-node number), which filenames & data
 * to destroy.
 * \return 0 in case of no errors, other values otherwise.
 */
static int destroy_record(ntfs_volume *nv, const s64 record,
	unsigned char * const buf)
{
	struct ufile *file = NULL;
	runlist_element *rl = NULL;
	ntfs_attr *mft = NULL;

	ntfs_attr_search_ctx *ctx = NULL;
	int ret_wfs = 0;
	unsigned long int pass, i;
	s64 j;
	unsigned char * a_offset;
	int selected[NPAT];

	file = (struct ufile *) malloc(sizeof(struct ufile));
	if (file == NULL) {
		return -1;
	}

	NTFS_INIT_LIST_HEAD(&(file->name));
	NTFS_INIT_LIST_HEAD(&(file->data));
	file->inode = record;

	file->mft = (MFT_RECORD*)malloc(nv->mft_record_size);
	if (file->mft == NULL) {
		free_file (file);
		return -1;
	}

	mft = ntfs_attr_open(nv->mft_ni, AT_DATA, AT_UNNAMED, 0);
	if (mft == NULL) {
		free_file(file);
		return -2;
	}

		/* Avoid getting fixup warnings on unitialized inodes */
	NVolSetNoFixupWarn(nv);
	/* Read the MFT reocrd of the i-node */
	if (ntfs_attr_mst_pread(mft, nv->mft_record_size * record, 1LL,
		nv->mft_record_size, file->mft) < 1) {

		NVolClearNoFixupWarn(nv);
		ntfs_attr_close(mft);
		free_file(file);
		return -3;
	}
	NVolClearNoFixupWarn(nv);
	ntfs_attr_close(mft);
	mft = NULL;

	ctx = ntfs_attr_get_search_ctx(NULL, file->mft);
	if (ctx == NULL) {
		free_file(file);
		return -4;
	}

	/* Wiping file names */
	while (1 == 1) {

        	if (ntfs_attr_lookup(AT_FILE_NAME, NULL, 0, CASE_SENSITIVE,
			0LL, NULL, 0, ctx) != 0) {
			break;	/* None / no more of that type */
		}
		if (ctx->attr == NULL)
			break;

		/* We know this will always be resident.
		   Find the offset of the data, including the MFT record. */
		a_offset = ((unsigned char *) ctx->attr
			+ le16_to_cpu(ctx->attr->value_offset));

		for (pass = 0; pass < npasses; pass++) {
			fill_buffer(pass, a_offset,
				le32_to_cpu(ctx->attr->value_length),
				selected);

			if ( !opts.noaction ) {
				if (ntfs_mft_records_write(nv,
					MK_MREF(record, 0), 1LL,
					ctx->mrec) != 0) {
					ret_wfs = -5;
					break;
				}
				/* Flush after each writing, if more than
				   1 overwriting needs to be done. Allow I/O
				   bufferring (efficiency), if just one
				   pass is needed. */
				if (npasses > 1) {
					nv->dev->d_ops->sync(nv->dev);
				}
			}

		}

		/* Wiping file name length */
		for (pass = 0; pass < npasses; pass++) {

			fill_buffer (pass, (unsigned char *)
				&(ctx->attr->value_length), sizeof(u32),
				selected);

			if (!opts.noaction) {
				if (ntfs_mft_records_write(nv,
					MK_MREF(record, 0),
					1LL, ctx->mrec) != 0) {
					ret_wfs = -5;
					break;
				}

				if (npasses > 1) {
					nv->dev->d_ops->sync(nv->dev);
				}
			}
		}
		ctx->attr->value_length = const_cpu_to_le32(0);
		if (!opts.noaction) {
			if (ntfs_mft_records_write(nv, MK_MREF(record, 0),
					1LL, ctx->mrec) != 0) {
				ret_wfs = -5;
				break;
			}
		}
	}

	ntfs_attr_reinit_search_ctx(ctx);

	/* Wiping file data */
	while (1 == 1) {
        	if (ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE, 0LL,
			NULL, 0, ctx) != 0) {
			break;	/* None / no more of that type */
		}
		if (ctx->attr == NULL)
			break;

		if (ctx->attr->non_resident == 0) {
			/* attribute is resident (part of MFT record) */
			/* find the offset of the data, including the MFT record */
			a_offset = ((unsigned char *) ctx->attr
				+ le16_to_cpu(ctx->attr->value_offset));

			/* Wiping the data itself */
			for (pass = 0; pass < npasses; pass++) {

				fill_buffer (pass, a_offset,
					le32_to_cpu(ctx->attr->value_length),
					selected);

				if (!opts.noaction) {
					if (ntfs_mft_records_write(nv,
						MK_MREF(record, 0),
						1LL, ctx->mrec) != 0) {
						ret_wfs = -5;
						break;
					}

					if (npasses > 1) {
						nv->dev->d_ops->sync(nv->dev);
					}
				}
			}

			/* Wiping data length */
			for (pass = 0; pass < npasses; pass++) {

				fill_buffer(pass, (unsigned char *)
					&(ctx->attr->value_length),
					sizeof(u32), selected);

				if (!opts.noaction) {
					if (ntfs_mft_records_write(nv,
						MK_MREF(record, 0),
						1LL, ctx->mrec) != 0) {
						ret_wfs = -5;
						break;
					}

					if (npasses > 1) {
						nv->dev->d_ops->sync(nv->dev);
					}
				}
			}
			ctx->attr->value_length = const_cpu_to_le32(0);
			if ( !opts.noaction ) {
				if (ntfs_mft_records_write(nv,
					MK_MREF(record, 0),
					1LL, ctx->mrec) != 0) {
					ret_wfs = -5;
					break;
				}
			}
		} else {
				/* Non-resident here */

			rl = ntfs_mapping_pairs_decompress(nv,
				ctx->attr, NULL);
			if (rl == NULL)	{
				continue;
			}

			if (rl[0].length <= 0) {
				continue;
			}

			for (i = 0; (rl[i].length > 0) && (ret_wfs == 0); i++) {
				if (rl[i].lcn == -1) {
					continue;
				}
				for (j = rl[i].lcn;
					(j < rl[i].lcn + rl[i].length)
					&& (ret_wfs == 0); j++)	{

					if (utils_cluster_in_use(nv, j) != 0)
						continue;
					for (pass = 0;
						pass < npasses;
						pass++)	{

						fill_buffer(pass, buf,
						 (size_t) nv->cluster_size,
						 selected);
						if (!opts.noaction) {
							if (ntfs_cluster_write(
								nv, j, 1LL,
								buf) < 1) {
								ret_wfs = -5;
								break;
							}

							if (npasses > 1) {
							 nv->dev->d_ops->sync
							  (nv->dev);
							}
						}
					}
				}
			}

			/* Wipe the data length here */
			for (pass = 0; pass < npasses; pass++) {
				fill_buffer(pass, (unsigned char *)
					&(ctx->attr->lowest_vcn),
					sizeof(VCN), selected);
				fill_buffer(pass, (unsigned char *)
					&(ctx->attr->highest_vcn),
					sizeof(VCN), selected);
				fill_buffer(pass, (unsigned char *)
					&(ctx->attr->allocated_size),
					sizeof(s64), selected);
				fill_buffer(pass, (unsigned char *)
					&(ctx->attr->data_size),
					sizeof(s64), selected);
				fill_buffer(pass, (unsigned char *)
					&(ctx->attr->initialized_size),
					sizeof(s64), selected);
				fill_buffer(pass, (unsigned char *)
					&(ctx->attr->compressed_size),
					sizeof(s64), selected);

				if ( !opts.noaction ) {
					if (ntfs_mft_records_write(nv,
						MK_MREF (record, 0),
						1LL, ctx->mrec) != 0) {
						ret_wfs = -5;
						break;
					}

					if (npasses > 1) {
						nv->dev->d_ops->sync(nv->dev);
					}
				}
			}
			ctx->attr->lowest_vcn = const_cpu_to_sle64(0);
			ctx->attr->highest_vcn = const_cpu_to_sle64(0);
			ctx->attr->allocated_size = const_cpu_to_sle64(0);
			ctx->attr->data_size = const_cpu_to_sle64(0);
			ctx->attr->initialized_size = const_cpu_to_sle64(0);
			ctx->attr->compressed_size = const_cpu_to_sle64(0);
			if (!opts.noaction) {
				if (ntfs_mft_records_write(nv,
					MK_MREF (record, 0),
					1LL, ctx->mrec) != 0) {
					ret_wfs = -5;
					break;
				}
			}
		}	/* end of resident check */
	} /* end of 'wiping file data' loop */

	ntfs_attr_put_search_ctx(ctx);
	free_file(file);

	return ret_wfs;
}

/**
 * Starts search for deleted inodes and undelete data on the given
 * NTFS filesystem.
 * \param FS The filesystem.
 * \return 0 in case of no errors, other values otherwise.
 */
static int wipe_unrm(ntfs_volume *nv)
{
	int ret_wfs = 0, ret;
	ntfs_attr *bitmapattr = NULL;
	s64 bmpsize, size, nr_mft_records, i, j, k;
	unsigned char b;
	unsigned char * buf = NULL;

#define MYBUF_SIZE 8192
	unsigned char *mybuf;
#define MINIM(x, y) ( ((x)<(y))?(x):(y) )

	mybuf = (unsigned char *) malloc(MYBUF_SIZE);
	if (mybuf == NULL) {
		return -1;
	}

	buf = (unsigned char *) malloc(nv->cluster_size);
	if (buf == NULL) {
		free (mybuf);
		return -1;
	}

	bitmapattr = ntfs_attr_open(nv->mft_ni, AT_BITMAP, AT_UNNAMED, 0);
	if (bitmapattr == NULL) {
		free (buf);
		free (mybuf);
		return -2;
	}
	bmpsize = bitmapattr->initialized_size;

	nr_mft_records = nv->mft_na->initialized_size
		>> nv->mft_record_size_bits;

	/* just like ntfsundelete; detects i-node numbers fine */
	for (i = 0; (i < bmpsize) && (ret_wfs==0); i += MYBUF_SIZE) {

		/* read a part of the file bitmap */
		size = ntfs_attr_pread(bitmapattr, i,
			MINIM((bmpsize - i), MYBUF_SIZE), mybuf);
		if (size < 0)
			break;

		/* parse each byte of the just-read part of the bitmap */
		for (j = 0; (j < size) && (ret_wfs==0); j++) {
			b = mybuf[j];
			/* parse each bit of the byte Bit 1 means 'in use'. */
			for (k = 0; (k < CHAR_BIT) && (ret_wfs==0);
					k++, b>>=1) {
				/* (i+j)*8+k is the i-node bit number */
				if (((i+j)*CHAR_BIT+k) >= nr_mft_records) {
					goto done;
				}
				if ((b & 1) != 0) {
					/* i-node is in use, skip it */
					continue;
				}
				/* wiping the i-node here: */
				ret = destroy_record (nv,
					(i+j)*CHAR_BIT+k, buf);
				if (ret != 0) {
					ret_wfs = ret;
				}
			}
		}
	}
done:
	ntfs_attr_close(bitmapattr);
	free(buf);
	free(mybuf);

	ntfs_log_quiet("wipe_undelete\n");
	return ret_wfs;
}



/**
 * print_summary - Tell the user what we are about to do
 *
 * List the operations about to be performed.  The output will be silenced by
 * the --quiet option.
 *
 * Return:  none
 */
static void print_summary(void)
{
	int i;

	if (opts.noaction)
		ntfs_log_quiet("%s is in 'no-action' mode, it will NOT write to disk."
			 "\n\n", EXEC_NAME);

	ntfs_log_quiet("%s is about to wipe:\n", EXEC_NAME);
	if (opts.unused)
		ntfs_log_quiet("\tunused disk space\n");
	if (opts.unused_fast)
		ntfs_log_quiet("\tunused disk space (fast)\n");
	if (opts.tails)
		ntfs_log_quiet("\tfile tails\n");
	if (opts.mft)
		ntfs_log_quiet("\tunused mft areas\n");
	if (opts.directory)
		ntfs_log_quiet("\tunused directory index space\n");
	if (opts.logfile)
		ntfs_log_quiet("\tthe logfile (journal)\n");
	if (opts.pagefile)
		ntfs_log_quiet("\tthe pagefile (swap space)\n");
	if (opts.undel)
		ntfs_log_quiet("\tundelete data\n");

	ntfs_log_quiet("\n%s will overwrite these areas with: ", EXEC_NAME);
	if (opts.bytes) {
		for (i = 0; opts.bytes[i] >= 0; i++)
			ntfs_log_quiet("0x%02x ", opts.bytes[i]);
	}
	ntfs_log_quiet("\n");
	if (opts.undel)
		ntfs_log_quiet("(however undelete data will be overwritten"
			" by random values)\n");

	if (opts.count > 1)
		ntfs_log_quiet("%s will repeat these operations %d times.\n", EXEC_NAME, opts.count);
	ntfs_log_quiet("\n");
}

/**
 * main - Begin here
 *
 * Start from here.
 *
 * Return:  0  Success, the program worked
 *	    1  Error, something went wrong
 */
int main(int argc, char *argv[])
{
	ntfs_volume *vol;
	int result = 1;
	int flags = 0;
	int res;
	int i, j;
	enum action act = act_info;

	ntfs_log_set_handler(ntfs_log_handler_outerr);

	res = parse_options(argc, argv);
	if (res >= 0)
		return (res);

	utils_set_locale();

	if (!opts.info)
		print_summary();

	if (opts.info || opts.noaction)
		flags = NTFS_MNT_RDONLY;
	if (opts.force)
		flags |= NTFS_MNT_RECOVER;

	vol = utils_mount_volume(opts.device, flags);
	if (!vol)
		goto free;

	if ((vol->flags & VOLUME_IS_DIRTY) && !opts.force)
		goto umount;

	if (opts.info) {
		act = act_info;
		opts.count = 1;
	} else if (opts.noaction) {
		act = act_test;
	} else {
		act = act_wipe;
	}

	/* Even if the output it quieted, you still get 5 seconds to abort. */
	if ((act == act_wipe) && !opts.force) {
		ntfs_log_quiet("\n%s will begin in 5 seconds, press CTRL-C to abort.\n", EXEC_NAME);
		sleep(5);
	}

	for (i = 0; opts.bytes[i] >= 0; i++) {
		npasses = i+1;
	}
	if (npasses == 0) {
		npasses = opts.count;
	}
#ifdef HAVE_TIME_H
	srandom(time(NULL));
#else
	/* use a pointer as a pseudorandom value */
	srandom((int)vol + npasses);
#endif
	ntfs_log_info("\n");
	for (i = 0; i < opts.count; i++) {
		int byte;
		s64 total = 0;
		s64 wiped = 0;

		for (j = 0; byte = opts.bytes[j], byte >= 0; j++) {

			if (opts.directory) {
				wiped = wipe_directory(vol, byte, act);
				if (wiped < 0)
					goto umount;
				else
					total += wiped;
			}

			if (opts.tails) {
				wiped = wipe_tails(vol, byte, act);
				if (wiped < 0)
					goto umount;
				else
					total += wiped;
			}

			if (opts.logfile) {
				wiped = wipe_logfile(vol, byte, act);
				if (wiped < 0)
					goto umount;
				else
					total += wiped;
			}

			if (opts.mft) {
				wiped = wipe_mft(vol, byte, act);
				if (wiped < 0)
					goto umount;
				else
					total += wiped;
			}

			if (opts.pagefile) {
				wiped = wipe_pagefile(vol, byte, act);
				if (wiped < 0)
					goto umount;
				else
					total += wiped;
			}

			if (opts.unused || opts.unused_fast) {
				if (opts.unused_fast)
					wiped = wipe_unused_fast(vol, byte,
								act);
				else
					wiped = wipe_unused(vol, byte, act);
				if (wiped < 0)
					goto umount;
				else
					total += wiped;
			}

			if (opts.undel) {
				wiped = wipe_unrm(vol);
				if (wiped != 0)
					goto umount;
				/*
				else
					total += wiped;
				*/
			}

			if (act == act_info)
				break;
		}

		if (opts.noaction || opts.info)
			ntfs_log_info("%lld bytes would be wiped"
					" (excluding undelete data)\n",
					(long long)total);
		else
			ntfs_log_info("%lld bytes were wiped"
					" (excluding undelete data)\n",
					(long long)total);
	}
	result = 0;
umount:
	ntfs_umount(vol, FALSE);
free:
	if (opts.bytes)
		free(opts.bytes);
	return result;
}
