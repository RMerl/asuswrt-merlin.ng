/**
 * ntfscp - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2004-2007 Yura Pakhuchiy
 * Copyright (c) 2005 Anton Altaparmakov
 * Copyright (c) 2006 Hil Liao
 * Copyright (c) 2014-2019 Jean-Pierre Andre
 *
 * This utility will copy file to an NTFS volume.
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
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <signal.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include "types.h"
#include "attrib.h"
#include "utils.h"
#include "volume.h"
#include "dir.h"
#include "bitmap.h"
#include "debug.h"
/* #include "version.h" */
#include "logging.h"
#include "ntfstime.h"
#include "misc.h"

struct options {
	char		*device;	/* Device/File to work with */
	char		*src_file;	/* Source file */
	char		*dest_file;	/* Destination file */
	char		*attr_name;	/* Write to attribute with this name. */
	int		 force;		/* Override common sense */
	int		 quiet;		/* Less output */
	int		 verbose;	/* Extra output */
	int		 minfragments;	/* Do minimal fragmentation */
	int		 timestamp;	/* Copy the modification time */
	int		 noaction;	/* Do not write to disk */
	ATTR_TYPES	 attribute;	/* Write to this attribute. */
	int		 inode;		/* Treat dest_file as inode number. */
};

struct ALLOC_CONTEXT {
	ntfs_volume *vol;
	ntfs_attr *na;
	runlist_element *rl;
	unsigned char *buf;
	s64 gathered_clusters;
	s64 wanted_clusters;
	s64 new_size;
	s64 lcn;
	int rl_allocated;
	int rl_count;
} ;

enum STEP { STEP_ERR, STEP_ZERO, STEP_ONE } ;

static const char *EXEC_NAME = "ntfscp";
static struct options opts;
static volatile sig_atomic_t caught_terminate = 0;

/**
 * version - Print version information about the program
 *
 * Print a copyright statement and a brief description of the program.
 *
 * Return:  none
 */
static void version(void)
{
	ntfs_log_info("\n%s v%s (libntfs-3g) - Copy file to an NTFS "
		"volume.\n\n", EXEC_NAME, VERSION);
	ntfs_log_info("Copyright (c) 2004-2007 Yura Pakhuchiy\n");
	ntfs_log_info("Copyright (c) 2005 Anton Altaparmakov\n");
	ntfs_log_info("Copyright (c) 2006 Hil Liao\n");
	ntfs_log_info("Copyright (c) 2014 Jean-Pierre Andre\n");
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
	ntfs_log_info("\nUsage: %s [options] device src_file dest_file\n\n"
		"    -a, --attribute NUM   Write to this attribute\n"
		"    -i, --inode           Treat dest_file as inode number\n"
		"    -f, --force           Use less caution\n"
		"    -h, --help            Print this help\n"
		"    -m, --min_fragments   Do minimal fragmentation\n"
		"    -N, --attr-name NAME  Write to attribute with this name\n"
		"    -n, --no-action       Do not write to disk\n"
		"    -q, --quiet           Less output\n"
		"    -t, --timestamp       Copy the modification time\n"
		"    -V, --version         Version information\n"
		"    -v, --verbose         More output\n\n",
		EXEC_NAME);
	ntfs_log_info("%s%s\n", ntfs_bugs, ntfs_home);
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
	static const char *sopt = "-a:ifh?mN:no:qtVv";
	static const struct option lopt[] = {
		{ "attribute",	required_argument,	NULL, 'a' },
		{ "inode",	no_argument,		NULL, 'i' },
		{ "force",	no_argument,		NULL, 'f' },
		{ "help",	no_argument,		NULL, 'h' },
		{ "min-fragments", no_argument,		NULL, 'm' },
		{ "attr-name",	required_argument,	NULL, 'N' },
		{ "no-action",	no_argument,		NULL, 'n' },
		{ "quiet",	no_argument,		NULL, 'q' },
		{ "timestamp",	no_argument,		NULL, 't' },
		{ "version",	no_argument,		NULL, 'V' },
		{ "verbose",	no_argument,		NULL, 'v' },
		{ NULL,		0,			NULL, 0   }
	};

	char *s;
	int c = -1;
	int err  = 0;
	int ver  = 0;
	int help = 0;
	int levels = 0;
	s64 attr;

	opts.device = NULL;
	opts.src_file = NULL;
	opts.dest_file = NULL;
	opts.attr_name = NULL;
	opts.inode = 0;
	opts.attribute = AT_DATA;
	opts.timestamp = 0;

	opterr = 0; /* We'll handle the errors, thank you. */

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (!opts.device) {
				opts.device = argv[optind - 1];
			} else if (!opts.src_file) {
				opts.src_file = argv[optind - 1];
			} else if (!opts.dest_file) {
				opts.dest_file = argv[optind - 1];
			} else {
				ntfs_log_error("You must specify exactly two "
						"files.\n");
				err++;
			}
			break;
		case 'a':
			if (opts.attribute != AT_DATA) {
				ntfs_log_error("You can specify only one "
						"attribute.\n");
				err++;
				break;
			}

			attr = strtol(optarg, &s, 0);
			if (*s) {
				ntfs_log_error("Couldn't parse attribute.\n");
				err++;
			} else
				opts.attribute = (ATTR_TYPES)cpu_to_le32(attr);
			break;
		case 'i':
			opts.inode++;
			break;
		case 'f':
			opts.force++;
			break;
		case 'h':
			help++;
			break;
		case 'm':
			opts.minfragments++;
			break;
		case 'N':
			if (opts.attr_name) {
				ntfs_log_error("You can specify only one "
						"attribute name.\n");
				err++;
			} else
				opts.attr_name = argv[optind - 1];
			break;
		case 'n':
			opts.noaction++;
			break;
		case 'q':
			opts.quiet++;
			ntfs_log_clear_levels(NTFS_LOG_LEVEL_QUIET);
			break;
		case 't':
			opts.timestamp++;
			break;
		case 'V':
			ver++;
			break;
		case 'v':
			opts.verbose++;
			ntfs_log_set_levels(NTFS_LOG_LEVEL_VERBOSE);
			break;
		case '?':
			if (strncmp(argv[optind - 1], "--log-", 6) == 0) {
				if (!ntfs_log_parse_option(argv[optind - 1]))
					err++;
				break;
			}
			/* fall through */
		default:
			ntfs_log_error("Unknown option '%s'.\n",
					argv[optind - 1]);
			err++;
			break;
		}
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
		if (!opts.device) {
			ntfs_log_error("You must specify a device.\n");
			err++;
		} else if (!opts.src_file) {
			ntfs_log_error("You must specify a source file.\n");
			err++;
		} else if (!opts.dest_file) {
			ntfs_log_error("You must specify a destination "
					"file.\n");
			err++;
		}

		if (opts.quiet && opts.verbose) {
			ntfs_log_error("You may not use --quiet and --verbose "
					"at the same time.\n");
			err++;
		}
		if (opts.timestamp
		    && (opts.attr_name || (opts.attribute != AT_DATA))) {
			ntfs_log_error("Setting --timestamp is only possible"
					" with unname data attribute.\n");
			err++;
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
 * signal_handler - Handle SIGINT and SIGTERM: abort write, sync and exit.
 */
static void signal_handler(int arg __attribute__((unused)))
{
	caught_terminate++;
}

/*
 *		Search for the next '0' in a bitmap chunk
 *
 *	Returns the position of next '0'
 *		or -1 if there are no more '0's
 */

static int next_zero(struct ALLOC_CONTEXT *alctx, s32 bufpos, s32 count)
{
	s32 index;
	unsigned int q,b;

	index = -1;
	while ((index < 0) && (bufpos < count)) {
		q = alctx->buf[bufpos >> 3];
		if (q == 255)
			bufpos = (bufpos | 7) + 1;
		else {
			b = bufpos & 7;
			while ((b < 8)
			    && ((1 << b) & q))
				b++;
			if (b < 8) {
				index = (bufpos & -8) | b;
			} else {
				bufpos = (bufpos | 7) + 1;
			}
		}
	}
	return (index);
}

/*
 *		Search for the next '1' in a bitmap chunk
 *
 *	Returns the position of next '1'
 *		or -1 if there are no more '1's
 */

static int next_one(struct ALLOC_CONTEXT *alctx, s32 bufpos, s32 count)
{
	s32 index;
	unsigned int q,b;

	index = -1;
	while ((index < 0) && (bufpos < count)) {
		q = alctx->buf[bufpos >> 3];
		if (q == 0)
			bufpos = (bufpos | 7) + 1;
		else {
			b = bufpos & 7;
			while ((b < 8)
			    && !((1 << b) & q))
				b++;
			if (b < 8) {
				index = (bufpos & -8) | b;
			} else {
				bufpos = (bufpos | 7) + 1;
			}
		}
	}
	return (index);
}

/*
 *		Allocate a bigger runlist when needed
 *
 *	The allocation is done by multiple of 4096 entries to avoid
 *	frequent reallocations.
 *
 *	Returns 0 if successful
 *		-1 otherwise, with errno set accordingly
 */

static int run_alloc(struct ALLOC_CONTEXT *alctx, s32 count)
{
	runlist_element *prl;
	int err;

	err = 0;
	if (count > alctx->rl_allocated) {
		prl = (runlist_element*)ntfs_malloc(
			(alctx->rl_allocated + 4096)*sizeof(runlist_element));
		if (prl) {
			if (alctx->rl) {
				memcpy(prl, alctx->rl, alctx->rl_allocated
						*sizeof(runlist_element));
				free(alctx->rl);
			}
			alctx->rl = prl;
			alctx->rl_allocated += 4096;
		} else
			err = -1;
	}
	return (err);
}

/*
 *		Merge a new run into the current optimal runlist
 *
 *	The new run is inserted only if it leads to improving the runlist.
 *	Runs in the current list are dropped when inserting the new one
 *	make them unneeded.
 *	The current runlist is sorted by run sizes, and there is no
 *	terminator.
 *
 *	Returns 0 if successful
 *		-1 otherwise, with errno set accordingly
 */

static int merge_run(struct ALLOC_CONTEXT *alctx, s64 lcn, s32 count)
{
	s64 excess;
	BOOL replace;
	int k;
	int drop;
	int err;

	err = 0;
	if (alctx->rl_count) {
		excess = alctx->gathered_clusters + count
				- alctx->wanted_clusters;
		if (alctx->rl_count > 1)
			/* replace if we can reduce the number of runs */
			replace = excess > (alctx->rl[0].length
						+ alctx->rl[1].length);
		else
			/* replace if we can shorten a single run */
			replace = (excess > alctx->rl[0].length)
				&& (count < alctx->rl[0].length);
	} else
		replace = FALSE;
	if (replace) {
			/* Using this run, we can now drop smaller runs */
		drop = 0;
		excess = alctx->gathered_clusters + count
					- alctx->wanted_clusters;
			/* Compute how many clusters we can drop */
		while ((drop < alctx->rl_count)
		    && (alctx->rl[drop].length <= excess)) {
			excess -= alctx->rl[drop].length;
			drop++;
		}
		k = 0;
		while (((k + drop) < alctx->rl_count)
		   && (alctx->rl[k + drop].length < count)) {
			alctx->rl[k] = alctx->rl[k + drop];
			k++;
		}
		alctx->rl[k].length = count;
		alctx->rl[k].lcn = lcn;
		if (drop > 1) {
			while ((k + drop) < alctx->rl_count) {
				alctx->rl[k + 1] = alctx->rl[k + drop];
				k++;
			}
		}
		alctx->rl_count -= (drop - 1);
		alctx->gathered_clusters = alctx->wanted_clusters + excess;
	} else {
		if (alctx->gathered_clusters < alctx->wanted_clusters) {
			/* We had not gathered enough clusters */
			if (!run_alloc(alctx, alctx->rl_count + 1)) {
				k = alctx->rl_count - 1;
				while ((k >= 0)
				    && (alctx->rl[k].length > count)) {
					alctx->rl[k+1] = alctx->rl[k];
					k--;
				}
				alctx->rl[k+1].length = count;
				alctx->rl[k+1].lcn = lcn;
				alctx->rl_count++;
				alctx->gathered_clusters += count;
			}
		}
	}
	return (err);
}

/*
 *		Examine a buffer from the global bitmap
 *	in order to locate free runs of clusters
 *
 *	Returns STEP_ZERO or STEP_ONE depending on whether the last
 *		bit examined was in a search for '0' or '1'. This must be
 *		put as argument to next examination.
 *	Returns STEP_ERR if there was an error.
 */

static enum STEP examine_buf(struct ALLOC_CONTEXT *alctx, s64 pos, s64 br,
			enum STEP step)
{
	s32 count;
	s64 offbuf; /* first bit available in buf */
	s32 bufpos; /* bit index in buf */
	s32 index;

	bufpos = pos & ((alctx->vol->cluster_size << 3) - 1);
	offbuf = pos - bufpos;
	while (bufpos < (br << 3)) {
		if (step == STEP_ZERO) {
				/* find first zero */
			index = next_zero(alctx, bufpos, br << 3);
			if (index >= 0) {
				alctx->lcn = offbuf + index;
				step = STEP_ONE;
				bufpos = index;
			} else {
				bufpos = br << 3;
			}
		} else {
				/* find first one */
			index = next_one(alctx, bufpos, br << 3);
			if (index >= 0) {
				count = offbuf + index - alctx->lcn;
				step = STEP_ZERO;
				bufpos = index;
				if (merge_run(alctx, alctx->lcn, count)) {
					step = STEP_ERR;
					bufpos = br << 3;
				}
			} else {
				bufpos = br << 3;
			}
		}
	}
	return (step);
}

/*
 *		Sort the final runlist by lcn's and insert a terminator
 *
 *	Returns 0 if successful
 *		-1 otherwise, with errno set accordingly
 */

static int sort_runlist(struct ALLOC_CONTEXT *alctx)
{
	LCN lcn;
	VCN vcn;
	s64 length;
	BOOL sorted;
	int err;
	int k;

	err = 0;
			/* This sorting can be much improved... */
	do {
		sorted = TRUE;
		for (k=0; (k+1)<alctx->rl_count; k++) {
			if (alctx->rl[k+1].lcn < alctx->rl[k].lcn) {
				length = alctx->rl[k].length;
				lcn = alctx->rl[k].lcn;
				alctx->rl[k] = alctx->rl[k+1];
				alctx->rl[k+1].length = length;
				alctx->rl[k+1].lcn = lcn;
				sorted = FALSE;
			}
		}
	} while (!sorted);
		/* compute the vcns */
	vcn = 0;
	for (k=0; k<alctx->rl_count; k++) {
		alctx->rl[k].vcn = vcn;
		vcn += alctx->rl[k].length;
	}
		/* Shorten the last run if we got too much */
	if (vcn > alctx->wanted_clusters) {
		k = alctx->rl_count - 1;
		alctx->rl[k].length -= vcn - alctx->wanted_clusters;
		vcn = alctx->wanted_clusters;
	}
		/* Append terminator */
	if (run_alloc(alctx, alctx->rl_count + 1))
		err = -1;
	else {
		k = alctx->rl_count++;
		alctx->rl[k].vcn = vcn;
		alctx->rl[k].length = 0;
		alctx->rl[k].lcn = LCN_ENOENT;
	}
	return (err);
}

/*
 *		Update the sizes of an attribute
 *
 *	Returns 0 if successful
 *		-1 otherwise, with errno set accordingly
 */

static int set_sizes(struct ALLOC_CONTEXT *alctx, ntfs_attr_search_ctx *ctx)
{
	ntfs_attr *na;
	ntfs_inode *ni;
	ATTR_RECORD *attr;

	na = alctx->na;
				/* Compute the sizes */
	na->data_size = alctx->new_size;
	na->initialized_size = 0;
	na->allocated_size = alctx->wanted_clusters
					<< alctx->vol->cluster_size_bits;
		/* Feed the sizes into the attribute */
	attr = ctx->attr;
	attr->non_resident = 1;
	attr->data_size = cpu_to_sle64(na->data_size);
	attr->initialized_size = cpu_to_sle64(na->initialized_size);
	attr->allocated_size = cpu_to_sle64(na->allocated_size);
	if (na->data_flags & ATTR_IS_SPARSE)
		attr->compressed_size = cpu_to_sle64(na->compressed_size);
		/* Copy the unnamed data attribute sizes to inode */
	if ((opts.attribute == AT_DATA) && !na->name_len) {
		ni = na->ni;
		ni->data_size = na->data_size;
		if (na->data_flags & ATTR_IS_SPARSE) {
			ni->allocated_size = na->compressed_size;
			ni->flags |= FILE_ATTR_SPARSE_FILE;
		} else
			ni->allocated_size = na->allocated_size;
	}
	return (0);
}

/*
 *		Assign a runlist to an attribute and store
 *
 *	Returns 0 if successful
 *		-1 otherwise, with errno set accordingly
 */

static int assign_runlist(struct ALLOC_CONTEXT *alctx)
{
	ntfs_attr *na;
	ntfs_attr_search_ctx *ctx;
	int k;
	int err;

	err = 0;
	na = alctx->na;
	if (na->rl)
		free(na->rl);
	na->rl = alctx->rl;
			/* Allocate the clusters */
	for (k=0; ((k + 1) < alctx->rl_count) && !err; k++) {
		if (ntfs_bitmap_set_run(alctx->vol->lcnbmp_na,
				alctx->rl[k].lcn, alctx->rl[k].length)) {
			err = -1;
		}
	}
	na->allocated_size = alctx->wanted_clusters
					<< alctx->vol->cluster_size_bits;
	NAttrSetNonResident(na);
	NAttrSetFullyMapped(na);
	if (err || ntfs_attr_update_mapping_pairs(na, 0)) {
		err = -1;
	} else {
		ctx = ntfs_attr_get_search_ctx(alctx->na->ni, NULL);
		if (ctx) {
			if (ntfs_attr_lookup(opts.attribute, na->name,
					na->name_len,
					CASE_SENSITIVE, 0, NULL, 0, ctx)) {
				err = -1;
			} else {
				if (set_sizes(alctx, ctx))
					err = -1;
			}
		} else
			err = -1;
		ntfs_attr_put_search_ctx(ctx);
	}
	return (err);
}

/*
 *		Find the runs which minimize fragmentation
 *
 *	Only the first and second data zones are examined, the MFT zone
 *	is preserved.
 *
 *	Returns 0 if successful
 *		-1 otherwise, with errno set accordingly
 */

static int find_best_runs(struct ALLOC_CONTEXT *alctx)
{
	ntfs_volume *vol;
	s64 pos; /* bit index in bitmap */
	s64 br; /* byte count in buf */
	int err;
	enum STEP step;

	err = 0;
	vol = alctx->vol;
			/* examine the first data zone */
	pos = vol->mft_zone_end;
	br = vol->cluster_size;
	step = STEP_ZERO;
	while ((step != STEP_ERR)
	    && (br == vol->cluster_size)
	    && (pos < vol->nr_clusters)) {
		br = ntfs_attr_pread(vol->lcnbmp_na,
			(pos >> 3) & -vol->cluster_size,
			vol->cluster_size, alctx->buf);
		if (br > 0) {
			step = examine_buf(alctx, pos, br, step);
			pos = (pos | ((vol->cluster_size << 3) - 1)) + 1;
		}
	}
			/* examine the second data zone */
	pos = 0;
	br = vol->cluster_size;
	step = STEP_ZERO;
	while ((step != STEP_ERR)
	    && (br == vol->cluster_size)
	    && (pos < vol->mft_zone_start)) {
		br = ntfs_attr_pread(vol->lcnbmp_na,
			(pos >> 3) & -vol->cluster_size,
			vol->cluster_size, alctx->buf);
		if (br > 0) {
			step = examine_buf(alctx, pos, br, step);
			pos = (pos | ((vol->cluster_size << 3) - 1)) + 1;
		}
	}
	if (alctx->gathered_clusters < alctx->wanted_clusters) {
		errno = ENOSPC;
		ntfs_log_error("Error : not enough space on device\n");
		err = -1;
	} else {
		if ((step == STEP_ERR) || sort_runlist(alctx))
			err = -1;
	}
	return (err);
}

/*
 *		Preallocate clusters with minimal fragmentation
 *
 *	Returns 0 if successful
 *		-1 otherwise, with errno set accordingly
 */

static int preallocate(ntfs_attr *na, s64 new_size)
{
	struct ALLOC_CONTEXT *alctx;
	ntfs_volume *vol;
	int err;

	err = 0;
	vol = na->ni->vol;
	alctx = (struct ALLOC_CONTEXT*)ntfs_malloc(sizeof(struct ALLOC_CONTEXT));
	if (alctx) {
		alctx->buf = (unsigned char*)ntfs_malloc(vol->cluster_size);
		if (alctx->buf) {
			alctx->na = na;
			alctx->vol = vol;
			alctx->rl_count = 0;
			alctx->rl_allocated = 0;
			alctx->rl = (runlist_element*)NULL;
			alctx->new_size = new_size;
			alctx->wanted_clusters = (new_size
				+ vol->cluster_size - 1)
					>> vol->cluster_size_bits;
			alctx->gathered_clusters = 0;
			if (find_best_runs(alctx))
				err = -1;
			if (!err && !opts.noaction) {
				if (assign_runlist(alctx))
					err = -1;
			} else
				free(alctx->rl);
			free(alctx->buf);
		} else
			err = -1;
		free(alctx);
	} else
		err = -1;
	return (err);
}

/**
 * Create a regular file under the given directory inode
 *
 * It is a wrapper function to ntfs_create(...)
 *
 * Return:  the created file inode
 */
static ntfs_inode *ntfs_new_file(ntfs_inode *dir_ni,
			  const char *filename)
{
	ntfschar *ufilename;
	/* inode to the file that is being created */
	ntfs_inode *ni;
	int ufilename_len;

	/* ntfs_mbstoucs(...) will allocate memory for ufilename if it's NULL */
	ufilename = NULL;
	ufilename_len = ntfs_mbstoucs(filename, &ufilename);
	if (ufilename_len == -1) {
		ntfs_log_perror("ERROR: Failed to convert '%s' to unicode",
					filename);
		return NULL;
	}
	ni = ntfs_create(dir_ni, const_cpu_to_le32(0), ufilename, ufilename_len, S_IFREG);
	free(ufilename);
	return ni;
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
	FILE *in;
	struct stat st;
	ntfs_volume *vol;
	ntfs_inode *out;
	ntfs_attr *na;
	int flags = 0;
	int res;
	int result = 1;
	s64 new_size;
	u64 offset;
	char *buf;
	s64 br, bw;
	ntfschar *attr_name;
	int attr_name_len = 0;
#ifdef HAVE_WINDOWS_H
	char *unix_name;
#endif

	ntfs_log_set_handler(ntfs_log_handler_stderr);

	res = parse_options(argc, argv);
	if (res >= 0)
		return (res);

	utils_set_locale();

	/* Set SIGINT handler. */
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		ntfs_log_perror("Failed to set SIGINT handler");
		return 1;
	}
	/* Set SIGTERM handler. */
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		ntfs_log_perror("Failed to set SIGTERM handler");
		return 1;
	}

	if (opts.noaction)
		flags = NTFS_MNT_RDONLY;
	if (opts.force)
		flags |= NTFS_MNT_RECOVER;

	vol = utils_mount_volume(opts.device, flags);
	if (!vol) {
		ntfs_log_perror("ERROR: couldn't mount volume");
		return 1;
	}

	if ((vol->flags & VOLUME_IS_DIRTY) && !opts.force)
		goto umount;

	NVolSetCompression(vol); /* allow compression */
	if (ntfs_volume_get_free_space(vol)) {
		ntfs_log_perror("ERROR: couldn't get free space");
		goto umount;
	}

	{
		struct stat fst;
		if (stat(opts.src_file, &fst) == -1) {
			ntfs_log_perror("ERROR: Couldn't stat source file");
			goto umount;
		}
		new_size = fst.st_size;
	}
	ntfs_log_verbose("New file size: %lld\n", (long long)new_size);

	in = fopen(opts.src_file, "r");
	if (!in) {
		ntfs_log_perror("ERROR: Couldn't open source file");
		goto umount;
	}

	if (opts.inode) {
		s64 inode_num;
		char *s;

		inode_num = strtoll(opts.dest_file, &s, 0);
		if (*s) {
			ntfs_log_error("ERROR: Couldn't parse inode number.\n");
			goto close_src;
		}
		out = ntfs_inode_open(vol, inode_num);
	} else {
#ifdef HAVE_WINDOWS_H
		unix_name = ntfs_utils_unix_path(opts.dest_file);
		if (unix_name) {
			out = ntfs_pathname_to_inode(vol, NULL, unix_name);
  		} else
			out = (ntfs_inode*)NULL;
#else
		out = ntfs_pathname_to_inode(vol, NULL, opts.dest_file);
#endif
	}
	if (!out) {
		/* Copy the file if the dest_file's parent dir can be opened. */
		char *parent_dirname;
		char *filename;
		ntfs_inode *dir_ni;
		ntfs_inode *ni;
		char *dirname_last_whack;

#ifdef HAVE_WINDOWS_H
		filename = basename(unix_name);
		parent_dirname = strdup(unix_name);
#else
		filename = basename(opts.dest_file);
		parent_dirname = strdup(opts.dest_file);
#endif
		if (!parent_dirname) {
			ntfs_log_perror("strdup() failed");
			goto close_src;
		}
		dirname_last_whack = strrchr(parent_dirname, '/');
		if (dirname_last_whack) {
			if (dirname_last_whack == parent_dirname)
				dirname_last_whack[1] = 0;
			else
				*dirname_last_whack = 0;
			dir_ni = ntfs_pathname_to_inode(vol, NULL,
					parent_dirname);
		} else {
			ntfs_log_verbose("Target path does not contain '/'. "
					"Using root directory as parent.\n");
			dir_ni = ntfs_inode_open(vol, FILE_root);
		}
		if (dir_ni) {
			if (!(dir_ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)) {
				/* Remove the last '/' for estetic reasons. */
				dirname_last_whack[0] = 0;
				ntfs_log_error("The file '%s' already exists "
						"and is not a directory. "
						"Aborting.\n", parent_dirname);
				free(parent_dirname);
				ntfs_inode_close(dir_ni);
				goto close_src;
			}
			ntfs_log_verbose("Creating a new file '%s' under '%s'"
					 "\n", filename, parent_dirname);
			ni = ntfs_new_file(dir_ni, filename);
			ntfs_inode_close(dir_ni);
			if (!ni) {
				ntfs_log_perror("Failed to create '%s' under "
						"'%s'", filename,
						parent_dirname);
				free(parent_dirname);
				goto close_src;
			}
			out = ni;
		} else {
			ntfs_log_perror("ERROR: Couldn't open '%s'",
					parent_dirname);
			free(parent_dirname);
			goto close_src;
		}
		free(parent_dirname);
	}
	/* The destination is a directory. */
	if ((out->mrec->flags & MFT_RECORD_IS_DIRECTORY) && !opts.inode) {
		char *filename;
		char *overwrite_filename;
		int overwrite_filename_len;
		ntfs_inode *ni;
		ntfs_inode *dir_ni;
		int filename_len;
		int dest_dirname_len;

		filename = basename(opts.src_file);
		dir_ni = out;
		filename_len = strlen(filename);
		dest_dirname_len = strlen(opts.dest_file);
		overwrite_filename_len = filename_len+dest_dirname_len + 2;
		overwrite_filename = malloc(overwrite_filename_len);
		if (!overwrite_filename) {
			ntfs_log_perror("ERROR: Failed to allocate %i bytes "
					"memory for the overwrite filename",
					overwrite_filename_len);
			ntfs_inode_close(out);
			goto close_src;
		}
#ifdef HAVE_WINDOWS_H
		strcpy(overwrite_filename, unix_name);
#else
		strcpy(overwrite_filename, opts.dest_file);
#endif
		if (overwrite_filename[dest_dirname_len - 1] != '/') {
			strcat(overwrite_filename, "/");
		}
		strcat(overwrite_filename, filename);
		ni = ntfs_pathname_to_inode(vol, dir_ni, overwrite_filename);
		/* Does a file with the same name exist in the dest dir? */
		if (ni) {
			ntfs_log_verbose("Destination path has a file with "
					"the same name\nOverwriting the file "
					"'%s'\n", overwrite_filename);
			ntfs_inode_close(out);
			out = ni;
		} else {
			ntfs_log_verbose("Creating a new file '%s' under "
					"'%s'\n", filename, opts.dest_file);
			ni = ntfs_new_file(dir_ni, filename);
			ntfs_inode_close(dir_ni);
			if (!ni) {
				ntfs_log_perror("ERROR: Failed to create the "
						"destination file under '%s'",
						opts.dest_file);
				free(overwrite_filename);
				goto close_src;
			}
			out = ni;
		}
		free(overwrite_filename);
	}

	attr_name = ntfs_str2ucs(opts.attr_name, &attr_name_len);
	if (!attr_name) {
		ntfs_log_perror("ERROR: Failed to parse attribute name '%s'",
				opts.attr_name);
		goto close_dst;
	}

	na = ntfs_attr_open(out, opts.attribute, attr_name, attr_name_len);
	if (!na) {
		if (errno != ENOENT) {
			ntfs_log_perror("ERROR: Couldn't open attribute");
			goto close_dst;
		}
		/* Requested attribute isn't present, add it. */
		if (ntfs_attr_add(out, opts.attribute, attr_name,
				attr_name_len, NULL, 0)) {
			ntfs_log_perror("ERROR: Couldn't add attribute");
			goto close_dst;
		}
		na = ntfs_attr_open(out, opts.attribute, attr_name,
				attr_name_len);
		if (!na) {
			ntfs_log_perror("ERROR: Couldn't open just added "
					"attribute");
			goto close_dst;
		}
	}

	ntfs_log_verbose("Old file size: %lld\n", (long long)na->data_size);
	if (opts.minfragments && NAttrCompressed(na)) {
		ntfs_log_info("Warning : Cannot avoid fragmentation"
				" of a compressed attribute\n");
		opts.minfragments = 0;
		}
	if (na->data_size && opts.minfragments) {
		if (ntfs_attr_truncate(na, 0)) {
			ntfs_log_perror(
				"ERROR: Couldn't truncate existing attribute");
			goto close_attr;
		}
	}
	if (na->data_size != new_size) {
		if (opts.minfragments) {
			/*
			 * Do a standard truncate() to check whether the
			 * attribute has to be made non-resident.
			 * If still resident, preallocation is not needed.
			 */
			if (ntfs_attr_truncate(na, new_size)) {
				ntfs_log_perror(
					"ERROR: Couldn't resize attribute");
				goto close_attr;
			}
			if (NAttrNonResident(na)
			   && preallocate(na, new_size)) {
				ntfs_log_perror(
				    "ERROR: Couldn't preallocate attribute");
				goto close_attr;
			}
		} else {
			if (ntfs_attr_truncate_solid(na, new_size)) {
				ntfs_log_perror(
					"ERROR: Couldn't resize attribute");
				goto close_attr;
			}
		}
	}

	buf = malloc(NTFS_BUF_SIZE);
	if (!buf) {
		ntfs_log_perror("ERROR: malloc failed");
		goto close_attr;
	}

	ntfs_log_verbose("Starting write.\n");
	offset = 0;
	while (!feof(in)) {
		if (caught_terminate) {
			ntfs_log_error("SIGTERM or SIGINT received.  "
					"Aborting write.\n");
			break;
		}
		br = fread(buf, 1, NTFS_BUF_SIZE, in);
		if (!br) {
			if (!feof(in)) ntfs_log_perror("ERROR: fread failed");
			break;
		}
		bw = ntfs_attr_pwrite(na, offset, br, buf);
		if (bw != br) {
			ntfs_log_perror("ERROR: ntfs_attr_pwrite failed");
			break;
		}
		offset += bw;
	}
	if ((na->data_flags & ATTR_COMPRESSION_MASK)
	    && ntfs_attr_pclose(na))
		ntfs_log_perror("ERROR: ntfs_attr_pclose failed");
	ntfs_log_verbose("Syncing.\n");
	result = 0;
	free(buf);
close_attr:
	ntfs_attr_close(na);
	if (opts.timestamp) {
		if (!fstat(fileno(in),&st)) {
			s64 change_time = st.st_mtime*10000000LL
					+ NTFS_TIME_OFFSET;
			out->last_data_change_time = cpu_to_sle64(change_time);
			ntfs_inode_update_times(out, 0);
		} else {
			ntfs_log_error("Failed to get the time stamp.\n");
		}
	}
close_dst:
	while (ntfs_inode_close(out) && !opts.noaction) {
		if (errno != EBUSY) {
			ntfs_log_error("Sync failed. Run chkdsk.\n");
			break;
		}
		ntfs_log_error("Device busy.  Will retry sync in 3 seconds.\n");
		sleep(3);
	}
close_src:
	fclose(in);
umount:
	ntfs_umount(vol, FALSE);
	ntfs_log_verbose("Done.\n");
	return result;
}
