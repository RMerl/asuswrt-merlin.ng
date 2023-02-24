/**
 * ntfsfallocate
 *
 * Copyright (c) 2013-2020 Jean-Pierre Andre
 *
 * This utility will allocate clusters to a specified attribute belonging
 * to a specified file or directory, to a specified length.
 *
 * WARNING : this can lead to configurations not supported by Windows
 * and Windows may crash (BSOD) when writing to preallocated clusters
 * which were not written to.
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
 * along with this program (in the main directory of the Linux-NTFS source
 * in the file COPYING); if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
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
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif

#include "types.h"
#include "attrib.h"
#include "inode.h"
#include "layout.h"
#include "volume.h"
#include "logging.h"
#include "runlist.h"
#include "dir.h"
#include "bitmap.h"
#include "lcnalloc.h"
#include "utils.h"
#include "misc.h"

const char *EXEC_NAME = "ntfsfallocate";

char *dev_name;
const char *file_name;
le32 attr_type;
ntfschar *attr_name = NULL;
u32 attr_name_len;
s64 opt_alloc_offs;
s64 opt_alloc_len;

ATTR_DEF *attr_defs;

static struct {
				/* -h, print usage and exit. */
	int no_action;		/*     do not write to device, only display
				       what would be done. */
	int no_size_change;	/* -n, do not change the apparent size */
	int quiet;		/* -q, quiet execution. */
	int verbose;		/* -v, verbose execution, given twice, really
				       verbose execution (debug mode). */
	int force;		/* -f, force allocation. */
				/* -V, print version and exit. */
} opts;

static const struct option lopt[] = {
	{ "offset",	required_argument,	NULL, 'o' },
	{ "length",	required_argument,	NULL, 'l' },
	{ "force",	no_argument,		NULL, 'f' },
	{ "help",	no_argument,		NULL, 'h' },
	{ "no-action",	no_argument,		NULL, 'N' },
	{ "no-size-change",	no_argument,	NULL, 'n' },
	{ "quiet",	no_argument,		NULL, 'q' },
	{ "version",	no_argument,		NULL, 'V' },
	{ "verbose",	no_argument,		NULL, 'v' },
	{ NULL,		0,			NULL, 0   }
};

/**
 * err_exit - error output and terminate; ignores quiet (-q)
 *
 *	DO NOT USE when allocations are not in initial state
 */
__attribute__((noreturn))
__attribute__((format(printf, 2, 3)))
static void err_exit(ntfs_volume *vol, const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "ERROR: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "Aborting...\n");
	if (vol && ntfs_umount(vol, 0))
		fprintf(stderr, "Warning: Could not umount %s\n", dev_name);
	exit(1);
}

/**
 * copyright - print copyright statements
 */
static void copyright(void)
{
	fprintf(stderr, "Copyright (c) 2013-2014 Jean-Pierre Andre\n"
			"Allocate clusters to a specified attribute of "
			"a specified file.\n");
}

/**
 * license - print license statement
 */
static void license(void)
{
	fprintf(stderr, "%s", ntfs_gpl);
}

/**
 * usage - print a list of the parameters to the program
 */
__attribute__((noreturn))
static void usage(int ret)
{
	copyright();
	fprintf(stderr, "Usage: %s [options] -l length device file [attr-type "
			"[attr-name]]\n"
			"    If attr-type is not specified, 0x80 (i.e. $DATA) "
			"is assumed.\n"
			"    If attr-name is not specified, an unnamed "
			"attribute is assumed.\n"
			"    -f         Force execution despite errors\n"
			"    -n         Do not change the apparent size of file\n"
			"    -l length  Allocate length bytes\n"
			"    -o offset  Start allocating at offset\n"
			"    -v         Verbose execution\n"
			"    -vv        Very verbose execution\n"
			"    -V         Display version information\n"
			"    -h         Display this help\n", EXEC_NAME);
	fprintf(stderr, "%s%s", ntfs_bugs, ntfs_home);
	exit(ret);
}

/**
 * err_exit - error output, display usage and exit
 */
__attribute__((noreturn))
__attribute__((format(printf, 1, 2)))
static void err_usage(const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "ERROR: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	usage(1);
}

/*
 *		Get a value option with a possible multiple suffix
 */

static s64 option_value(const char *arg)
{
	s64 ll;
	char *s;
	s64 fact;
	int count;
	BOOL err;

	err = FALSE;
	ll = strtoll(arg, &s, 0);
	if ((ll >= LLONG_MAX) && (errno == ERANGE))
		err_exit((ntfs_volume*)NULL, "Too big value : %s\n",arg);
	if (*s) {
		count = 0;
		switch (*s++) {
		case 'E' : count++;
			/* FALLTHRU */
		case 'P' : count++;
			/* FALLTHRU */
		case 'T' : count++;
			/* FALLTHRU */
		case 'G' : count++;
			/* FALLTHRU */
		case 'M' : count++;
			/* FALLTHRU */
		case 'K' : count++;
			switch (*s++) {
			case 'i' :
				fact = 1024;
				if (*s++ != 'B')
					err = TRUE;
				break;
			case 'B' :
				fact = 1000;
				break;
			case '\0' :
				fact = 1024;
				s--;
				break;
			default :
				err = TRUE;
				fact = 1;
				break;
			}
			if (*s)
				err = TRUE;
			break;
		default :
			err = TRUE;
			break;
		}
		if (err)
			err_exit((ntfs_volume*)NULL,
					"Invalid suffix in : %s\n",arg);
		else
			while (count-- > 0) {
				if (ll > LLONG_MAX/1024)
					err_exit((ntfs_volume*)NULL,
						"Too big value : %s\n",arg);
				ll *= fact;
			}
	}
	return (ll);
}


/**
 * parse_options
 */
static void parse_options(int argc, char *argv[])
{
	long long ll;
	char *s, *s2;
	int c;

	opt_alloc_len = 0;
	opt_alloc_offs = 0;
	if (argc && *argv)
		EXEC_NAME = *argv;
	fprintf(stderr, "%s v%s (libntfs-3g)\n", EXEC_NAME, VERSION);
	while ((c = getopt_long(argc, argv, "fh?no:qvVl:", lopt, NULL)) != EOF) {
		switch (c) {
		case 'f':
			opts.force = 1;
			break;
		case 'n':
			opts.no_size_change = 1;
			break;
		case 'N':		/* Not proposed as a short option */
			opts.no_action = 1;
			break;
		case 'q':
			opts.quiet = 1;
			ntfs_log_clear_levels(NTFS_LOG_LEVEL_QUIET);
			break;
		case 'v':
			opts.verbose++;
			ntfs_log_set_levels(NTFS_LOG_LEVEL_VERBOSE);
			break;
		case 'V':
			/* Version number already printed */
			license();
			exit(0);
		case 'l':
			ll = option_value(argv[optind - 1]);
			if ((ll <= 0)
			    || (ll >= LLONG_MAX && errno == ERANGE))
				err_usage("Invalid length : %s\n",
					argv[optind - 1]);
			opt_alloc_len = ll;
			break;
		case 'o':
			ll = option_value(argv[optind - 1]);
			if ((ll < 0)
			    || (ll >= LLONG_MAX && errno == ERANGE))
				err_usage("Invalid offset : %s\n",
					argv[optind - 1]);
			opt_alloc_offs = ll;
			break;
		case 'h':
			usage(0);
		case '?':
		default:
			usage(1);
		}
	}
	if (!opt_alloc_len) {
		err_usage("Missing allocation length\n");
	}

	ntfs_log_verbose("length = %lli = 0x%llx\n",
			(long long)opt_alloc_len, (long long)opt_alloc_len);
	ntfs_log_verbose("offset = %lli = 0x%llx\n",
			(long long)opt_alloc_offs, (long long)opt_alloc_offs);

	if (optind == argc)
		usage(1);

	if (opts.verbose > 1)
		ntfs_log_set_levels(NTFS_LOG_LEVEL_DEBUG | NTFS_LOG_LEVEL_TRACE |
			NTFS_LOG_LEVEL_VERBOSE | NTFS_LOG_LEVEL_QUIET);

	/* Get the device. */
	dev_name = argv[optind++];
	ntfs_log_verbose("device name = %s\n", dev_name);

	if (optind == argc)
		usage(1);

	/* Get the file name. */
	file_name = argv[optind++];
	ntfs_log_verbose("file name = \"%s\"\n", file_name);

	/* Get the attribute type, if specified. */
	if (optind == argc) {
		attr_type = AT_DATA;
		attr_name = AT_UNNAMED;
		attr_name_len = 0;
	} else {
		unsigned long ul;

		s = argv[optind++];
		ul = strtoul(s, &s2, 0);
		if (*s2 || !ul || (ul >= ULONG_MAX && errno == ERANGE))
			err_usage("Invalid attribute type %s: %s\n", s,
					strerror(errno));
		attr_type = cpu_to_le32(ul);

		/* Get the attribute name, if specified. */
		if (optind != argc) {
			s = argv[optind++];
			/* Convert the string to little endian Unicode. */
			attr_name_len = ntfs_mbstoucs(s, &attr_name);
			if ((int)attr_name_len < 0)
				err_usage("Invalid attribute name "
						"\"%s\": %s\n",
						s, strerror(errno));

			/* Keep hold of the original string. */
			s2 = s;

			s = argv[optind++];
			if (optind != argc)
				usage(1);
		} else {
			attr_name = AT_UNNAMED;
			attr_name_len = 0;
		}
	}
	ntfs_log_verbose("attribute type = 0x%lx\n",
					(unsigned long)le32_to_cpu(attr_type));
	if (attr_name == AT_UNNAMED)
		ntfs_log_verbose("attribute name = \"\" (UNNAMED)\n");
	else
		ntfs_log_verbose("attribute name = \"%s\" (length %u "
				"Unicode characters)\n", s2,
				(unsigned int)attr_name_len);
}

/*
 *		Save the initial runlist, to be restored on error
 */

static runlist_element *ntfs_save_rl(runlist_element *rl)
{
	runlist_element *save;
	int n;

	n = 0;
	save = (runlist_element*)NULL;
	if (rl) {
		while (rl[n].length)
			n++;
		save = (runlist_element*)malloc((n + 1)*sizeof(runlist_element));
		if (save) {
			memcpy(save, rl, (n + 1)*sizeof(runlist_element));
		}
	}
	return (save);
}

/*
 *		Free the common part of two runs
 */

static void free_common(ntfs_volume *vol, runlist_element *brl, s64 blth,
				runlist_element *grl, s64 glth)
{
	VCN begin_common;
	VCN end_common;

	begin_common = max(grl->vcn, brl->vcn);
	end_common = min(grl->vcn + glth, brl->vcn + blth);
	if (end_common > begin_common) {
		if (ntfs_bitmap_clear_run(vol->lcnbmp_na,
			brl->lcn + begin_common - brl->vcn,
					end_common - begin_common))
			ntfs_log_error("Failed to free %lld clusters "
				"from 0x%llx\n",
				(long long)end_common - begin_common,
				(long long)(brl->lcn + begin_common
							- brl->vcn));
	}
}

/*
 *		Restore the cluster allocations to initial state
 *
 *	If a new error occurs, only output a message
 */

static void ntfs_restore_rl(ntfs_attr *na, runlist_element *oldrl)
{
	runlist_element *brl; /* Pointer to bad runlist */
	runlist_element *grl; /* Pointer to good runlist */
	ntfs_volume *vol;

	vol = na->ni->vol;
		/* Examine allocated entries from the bad runlist */
	for (brl=na->rl; brl->length; brl++) {
		if (brl->lcn != LCN_HOLE) {
// TODO improve by examining both list in parallel
		/* Find the holes in the good runlist which overlap */
			for (grl=oldrl; grl->length
			   && (grl->vcn<=(brl->vcn+brl->length)); grl++) {
				if (grl->lcn == LCN_HOLE) {
					free_common(vol, brl, brl->length, grl,
						grl->length);
				}
			}
			/* Free allocations beyond the end of good runlist */
			if (grl && !grl->length
			    && ((brl->vcn + brl->length) > grl->vcn)) {
				free_common(vol, brl, brl->length, grl,
					brl->vcn + brl->length - grl->vcn);
			}
		}
	}
	free(na->rl);
	na->rl = oldrl;
	if (ntfs_attr_update_mapping_pairs(na, 0)) {
		ntfs_log_error("Failed to restore the original runlist\n");
	}
}

/*
 *		Zero newly allocated runs up to initialized_size
 */

static int ntfs_inner_zero(ntfs_attr *na, runlist_element *rl)
{
	ntfs_volume *vol;
	char *buf;
	runlist_element *zrl;
	s64 cofs;
	s64 pos;
	s64 zeroed;
	int err;

	err = 0;
	vol = na->ni->vol;
	buf = (char*)malloc(vol->cluster_size);
	if (buf) {
		memset(buf, 0, vol->cluster_size);
		zrl = rl;
		pos = zrl->vcn << vol->cluster_size_bits;
		while (zrl->length
	 	    && !err
	    	    && (pos < na->initialized_size)) {
			for (cofs=0; cofs<zrl->length && !err; cofs++) {
				zeroed = ntfs_pwrite(vol->dev,
					(rl->lcn + cofs)
						<< vol->cluster_size_bits,
					vol->cluster_size, buf);
				if (zeroed != vol->cluster_size) {
					ntfs_log_error("Failed to zero at "
						"offset %lld\n",
						(long long)pos);
					errno = EIO;
					err = -1;
				}
				pos += vol->cluster_size;
			}
			zrl++;
			pos = zrl->vcn << vol->cluster_size_bits;
		}
		free(buf);
	} else {
		ntfs_log_error("Failed to allocate memory\n");
		errno = ENOSPC;
		err = -1;
	}
	return (err);
}

/*
 *		Merge newly allocated runs into runlist
 */

static int ntfs_merge_allocation(ntfs_attr *na, runlist_element *rl,
				s64 size)
{
	ntfs_volume *vol;
	int err;

	err = 0;
	vol = na->ni->vol;
	/* Newly allocated clusters before initialized size need be zeroed */
	if ((rl->vcn << vol->cluster_size_bits) < na->initialized_size) {
		err = ntfs_inner_zero(na, rl);
	}
	if (!err) {
		if (na->data_flags & ATTR_IS_SPARSE) {
			na->compressed_size += size;
			if (na->compressed_size >= na->allocated_size) {
				na->data_flags &= ~ATTR_IS_SPARSE;
				if (na->compressed_size > na->allocated_size) {
					ntfs_log_error("File size error : "
						"apparent %lld, "
						"compressed %lld > "
						"allocated %lld",
						(long long)na->data_size,
						(long long)na->compressed_size,
						(long long)na->allocated_size);
					errno = EIO;
					err = -1;
				}
			}
		}
	}
	if (!err) {
		rl = ntfs_runlists_merge(na->rl, rl);
		if (!rl) {
			ntfs_log_error("Failed to merge the new allocation\n");
			err = -1;
		} else {
			na->rl = rl;
				/* Update the runlist */
			if (ntfs_attr_update_mapping_pairs(na, 0)) {
				ntfs_log_error(
					"Failed to update the runlist\n");
				err = -1;
			}
		}
	}
	return (err);
}

static int ntfs_inner_allocation(ntfs_attr *na, s64 alloc_offs, s64 alloc_len)
{
	ntfs_volume *vol;
	runlist_element *rl;
	runlist_element *prl;
	runlist_element *rlc;
	VCN from_vcn;
	VCN end_vcn;
	LCN lcn_seek_from;
	VCN from_hole;
	VCN end_hole;
	s64 need;
	int err;
	BOOL done;

	err = 0;
	vol = na->ni->vol;
		/* Find holes which overlap the requested allocation */
	from_vcn = alloc_offs >> vol->cluster_size_bits;
	end_vcn = (alloc_offs + alloc_len + vol->cluster_size - 1)
			>> vol->cluster_size_bits;
	do {
		done = FALSE;
		rl = na->rl;
		while (rl->length
		    && ((rl->lcn >= 0)
		    	|| ((rl->vcn + rl->length) <= from_vcn)
			|| (rl->vcn >= end_vcn)))
				rl++;
		if (!rl->length)
			done = TRUE;
		else {
			from_hole = max(from_vcn, rl->vcn);
			end_hole = min(end_vcn, rl->vcn + rl->length);
			need = end_hole - from_hole;
			lcn_seek_from = -1;
			if (rl->vcn) {
					/* Avoid fragmentation when possible */
				prl = rl;
				if ((--prl)->lcn >= 0) {
					lcn_seek_from = prl->lcn
						+ from_hole - prl->vcn;
				}
			}
			if (need <= 0) {
				ntfs_log_error("Wrong hole size %lld\n",
							(long long)need);
				errno = EIO;
				err = -1;
			} else {
				rlc = ntfs_cluster_alloc(vol, from_hole, need,
					 lcn_seek_from, DATA_ZONE);
				if (!rlc)
					err = -1;
				else
					err = ntfs_merge_allocation(na, rlc,
						need << vol->cluster_size_bits);
			}
		}
	} while (!err && !done);
	return (err);
}

static int ntfs_full_allocation(ntfs_attr *na, ntfs_attr_search_ctx *ctx,
				s64 alloc_offs, s64 alloc_len)
{
	ATTR_RECORD *attr;
	ntfs_inode *ni;
	s64 initialized_size;
	s64 data_size;
	int err;

	err = 0;
	initialized_size = na->initialized_size;
	data_size = na->data_size;

	if (na->allocated_size <= alloc_offs) {
		/*
		 * Request is fully beyond what was already allocated :
		 * only need to expand the attribute
		 */
		err = ntfs_attr_truncate(na, alloc_offs);
		if (!err)
			err = ntfs_attr_truncate_solid(na,
						alloc_offs + alloc_len);
	} else {
		/*
		 * Request overlaps what was already allocated :
		 * We may have to fill existing holes, and force zeroes
		 * into clusters which are visible.
		 */
		if ((alloc_offs + alloc_len) > na->allocated_size)
			err = ntfs_attr_truncate(na, alloc_offs + alloc_len);
		if (!err)
			err = ntfs_inner_allocation(na, alloc_offs, alloc_len);
	}
		/* Set the sizes, even after an error, to keep consistency */
	na->initialized_size = initialized_size;
		/* Restore the original apparent size if requested or error */
	if (err || opts.no_size_change
	    || ((alloc_offs + alloc_len) < data_size))
		na->data_size = data_size;
	else {
		/*
		 * "man 1 fallocate" does not define the new apparent size
		 * when size change is allowed (no --keep-size).
		 * Assuming the same as no FALLOC_FL_KEEP_SIZE in fallocate(2) :
		 * "the file size will be changed if offset + len is greater
		 * than the  file  size"
// TODO check the behavior of another file system
		 */
		na->data_size = alloc_offs + alloc_len;
	}

	if (!err) {
	/* Find the attribute, which may have been relocated for allocations */
		if (ntfs_attr_lookup(attr_type, attr_name, attr_name_len,
					CASE_SENSITIVE, 0, NULL, 0, ctx)) {
			err = -1;
			ntfs_log_error("Failed to locate the attribute\n");
		} else {
				/* Feed the sizes into the attribute */
			attr = ctx->attr;
			attr->data_size = cpu_to_sle64(na->data_size);
			attr->initialized_size
				= cpu_to_sle64(na->initialized_size);
			attr->allocated_size
				= cpu_to_sle64(na->allocated_size);
			if (na->data_flags & ATTR_IS_SPARSE)
				attr->compressed_size
					= cpu_to_sle64(na->compressed_size);
			/* Copy the unnamed data attribute sizes to inode */
			if ((attr_type == AT_DATA) && !attr_name_len) {
				ni = na->ni;
				ni->data_size = na->data_size;
				if (na->data_flags & ATTR_IS_SPARSE) {
					ni->allocated_size
						= na->compressed_size;
					ni->flags |= FILE_ATTR_SPARSE_FILE;
				} else
					ni->allocated_size
						= na->allocated_size;
			}
		}
	}
	return (err);
}


/*
 *		Do the actual allocations
 */

static int ntfs_fallocate(ntfs_inode *ni, s64 alloc_offs, s64 alloc_len)
{
	s64 allocated_size;
	s64 data_size;
	ntfs_attr_search_ctx *ctx;
	ntfs_attr *na;
	runlist_element *oldrl;
	const char *errmess;
	int save_errno;
	int err;

	err = 0;
	/* Open the specified attribute. */
	na = ntfs_attr_open(ni, attr_type, attr_name, attr_name_len);
	if (!na) {
		ntfs_log_perror("Failed to open attribute 0x%lx: ",
				(unsigned long)le32_to_cpu(attr_type));
		err = -1;
	} else {
		errmess = (const char*)NULL;
		if (na->data_flags & ATTR_IS_COMPRESSED) {
			errmess= "Cannot fallocate a compressed file";
		}

		/* Locate the attribute record, needed for updating sizes */
		ctx = ntfs_attr_get_search_ctx(ni, NULL);
		if (!ctx) {
			errmess = "Failed to allocate a search context";
		}
		if (errmess) {
			ntfs_log_error("%s\n",errmess);
			err = -1;
		} else {
			/* Get and save the initial allocations */
			allocated_size = na->allocated_size;
			data_size = ni->data_size;
			if (na->rl)
				err = ntfs_attr_map_whole_runlist(na);
			if (!err) {
				if (na->rl)
					oldrl = ntfs_save_rl(na->rl);
				else
					oldrl = (runlist_element*)NULL;
				if (!na->rl || oldrl) {
					err = ntfs_full_allocation(na, ctx,
							alloc_offs, alloc_len);
					if (err) {
						save_errno = errno;
						ni->allocated_size
							= allocated_size;
						ni->data_size = data_size;
						ntfs_restore_rl(na, oldrl);
						errno = save_errno;
					} else {
						free(oldrl);
	/* Mark file name dirty, to update the sizes in directories */
						NInoFileNameSetDirty(ni);
						NInoSetDirty(ni);
					}
				} else
					err = -1;
			}
			ntfs_attr_put_search_ctx(ctx);
		}
		/* Close the attribute. */
		ntfs_attr_close(na);
	}
	return (err);
}

/**
 * main
 */
int main(int argc, char **argv)
{
	unsigned long mnt_flags, ul;
	int err;
	ntfs_inode *ni;
	ntfs_volume *vol;
#ifdef HAVE_WINDOWS_H
	char *unix_name;
#endif

	vol = (ntfs_volume*)NULL;
	ntfs_log_set_handler(ntfs_log_handler_outerr);

	/* Initialize opts to zero / required values. */
	memset(&opts, 0, sizeof(opts));

	/* Parse command line options. */
	parse_options(argc, argv);

	utils_set_locale();

	/* Make sure the file system is not mounted. */
	if (ntfs_check_if_mounted(dev_name, &mnt_flags))
		ntfs_log_perror("Failed to determine whether %s is mounted",
				dev_name);
	else if (mnt_flags & NTFS_MF_MOUNTED) {
		ntfs_log_error("%s is mounted.\n", dev_name);
		if (!opts.force)
			err_exit((ntfs_volume*)NULL, "Refusing to run!\n");
		fprintf(stderr, "ntfsfallocate forced anyway. Hope /etc/mtab "
				"is incorrect.\n");
	}

	/* Mount the device. */
	if (opts.no_action) {
		ntfs_log_quiet("Running in READ-ONLY mode!\n");
		ul = NTFS_MNT_RDONLY;
	} else
		if (opts.force)
			ul = NTFS_MNT_RECOVER;
		else
			ul = 0;
	vol = ntfs_mount(dev_name, ul);
	if (!vol)
		err_exit(vol, "Failed to mount %s: %s\n", dev_name,
			strerror(errno));

	if ((vol->flags & VOLUME_IS_DIRTY) && !opts.force)
		err_exit(vol, "Volume is dirty, please run chkdsk.\n");

	if (ntfs_volume_get_free_space(vol))
		err_exit(vol, "Failed to get free clusters %s: %s\n",
					dev_name, strerror(errno));

	/* Open the specified inode. */
#ifdef HAVE_WINDOWS_H
	unix_name = ntfs_utils_unix_path(file_name);
	if (unix_name) {
		ni = ntfs_pathname_to_inode(vol, NULL, unix_name);
		free(unix_name);
	} else
		ni = (ntfs_inode*)NULL;
#else
	ni = ntfs_pathname_to_inode(vol, NULL, file_name);
#endif
	if (!ni)
		err_exit(vol, "Failed to open file \"%s\": %s\n", file_name,
				strerror(errno));
	if (!opts.no_action)
		err = ntfs_fallocate(ni, opt_alloc_offs, opt_alloc_len);

	/* Close the inode. */
	if (ntfs_inode_close(ni)) {
		err = -1;
		err_exit(vol, "Failed to close inode \"%s\" : %s\n", file_name,
				strerror(errno));
	}

	/* Unmount the volume. */
	err = ntfs_umount(vol, 0);
	vol = (ntfs_volume*)NULL;
	if (err)
		ntfs_log_perror("Warning: Failed to umount %s", dev_name);

	/* Free the attribute name if it exists. */
	if (attr_name_len)
		ntfs_ucsfree(attr_name);

	ntfs_log_quiet("ntfsfallocate completed successfully. Have a nice day.\n");
	return 0;
}
