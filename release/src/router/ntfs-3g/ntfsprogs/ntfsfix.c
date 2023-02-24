/**
 * ntfsfix - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2000-2006 Anton Altaparmakov
 * Copyright (c) 2002-2006 Szabolcs Szakacsits
 * Copyright (c) 2007      Yura Pakhuchiy
 * Copyright (c) 2011-2015 Jean-Pierre Andre
 *
 * This utility fixes some common NTFS problems, resets the NTFS journal file
 * and schedules an NTFS consistency check for the first boot into Windows.
 *
 *	Anton Altaparmakov <aia21@cantab.net>
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

/*
 * WARNING: This program might not work on architectures which do not allow
 * unaligned access. For those, the program would need to start using
 * get/put_unaligned macros (#include <asm/unaligned.h>), but not doing it yet,
 * since NTFS really mostly applies to ia32 only, which does allow unaligned
 * accesses. We might not actually have a problem though, since the structs are
 * defined as being packed so that might be enough for gcc to insert the
 * correct code.
 *
 * If anyone using a non-little endian and/or an aligned access only CPU tries
 * this program please let me know whether it works or not!
 *
 *	Anton Altaparmakov <aia21@cantab.net>
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
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "types.h"
#include "attrib.h"
#include "volume.h"
#include "bootsect.h"
#include "mft.h"
#include "device.h"
#include "logfile.h"
#include "runlist.h"
#include "mst.h"
#include "utils.h"
/* #include "version.h" */
#include "logging.h"
#include "misc.h"

#ifdef NO_NTFS_DEVICE_DEFAULT_IO_OPS
#	error "No default device io operations!  Cannot build ntfsfix.  \
You need to run ./configure without the --disable-default-device-io-ops \
switch if you want to be able to build the NTFS utilities."
#endif

static const char *EXEC_NAME = "ntfsfix";
static const char OK[]       = "OK\n";
static const char FAILED[]   = "FAILED\n";
static const char FOUND[]    = "FOUND\n";

#define DEFAULT_SECTOR_SIZE 512

static struct {
	char *volume;
	BOOL no_action;
	BOOL clear_bad_sectors;
	BOOL clear_dirty;
} opt;

/*
 *		Definitions for fixing the self-located MFT bug
 */

#define SELFLOC_LIMIT 16

struct MFT_SELF_LOCATED {
	ntfs_volume *vol;
	MFT_RECORD *mft0;
	MFT_RECORD *mft1;
	MFT_RECORD *mft2;
	ATTR_LIST_ENTRY *attrlist;
	ATTR_LIST_ENTRY *attrlist_to_ref1;
	MFT_REF mft_ref0;
	MFT_REF mft_ref1;
	LCN attrlist_lcn;
	BOOL attrlist_resident;
} ;

/**
 * usage
 */
__attribute__((noreturn))
static void usage(int ret)
{
	ntfs_log_info("%s v%s (libntfs-3g)\n"
		   "\n"
		   "Usage: %s [options] device\n"
		   "    Attempt to fix an NTFS partition.\n"
		   "\n"
		   "    -b, --clear-bad-sectors Clear the bad sector list\n"
		   "    -d, --clear-dirty       Clear the volume dirty flag\n"
		   "    -h, --help              Display this help\n"
		   "    -n, --no-action         Do not write anything\n"
		   "    -V, --version           Display version information\n"
		   "\n"
		   "For example: %s /dev/hda6\n\n",
		   EXEC_NAME, VERSION, EXEC_NAME,
		   EXEC_NAME);
	ntfs_log_info("%s%s", ntfs_bugs, ntfs_home);
	exit(ret);
}

/**
 * version
 */
__attribute__((noreturn))
static void version(void)
{
	ntfs_log_info("%s v%s\n\n"
		   "Attempt to fix an NTFS partition.\n\n"
		   "Copyright (c) 2000-2006 Anton Altaparmakov\n"
		   "Copyright (c) 2002-2006 Szabolcs Szakacsits\n"
		   "Copyright (c) 2007      Yura Pakhuchiy\n"
		   "Copyright (c) 2011-2015 Jean-Pierre Andre\n\n",
		   EXEC_NAME, VERSION);
	ntfs_log_info("%s\n%s%s", ntfs_gpl, ntfs_bugs, ntfs_home);
	exit(0);
}

/**
 * parse_options
 */
static void parse_options(int argc, char **argv)
{
	int c;
	static const char *sopt = "-bdhnV";
	static const struct option lopt[] = {
		{ "help",		no_argument,	NULL, 'h' },
		{ "no-action",		no_argument,	NULL, 'n' },
		{ "clear-bad-sectors",	no_argument,	NULL, 'b' },
		{ "clear-dirty",	no_argument,	NULL, 'd' },
		{ "version",		no_argument,	NULL, 'V' },
		{ NULL, 		0, NULL, 0 }
	};

	memset(&opt, 0, sizeof(opt));

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (!opt.volume)
				opt.volume = argv[optind - 1];
			else {
				ntfs_log_info("ERROR: Too many arguments.\n");
				usage(1);
			}
			break;
		case 'b':
			opt.clear_bad_sectors = TRUE;
			break;
		case 'd':
			opt.clear_dirty = TRUE;
			break;
		case 'n':
			opt.no_action = TRUE;
			break;
		case 'h':
			usage(0);
		case '?':
			usage(1);
			/* fall through */
		case 'V':
			version();
		default:
			ntfs_log_info("ERROR: Unknown option '%s'.\n", argv[optind - 1]);
			usage(1);
		}
	}

	if (opt.volume == NULL) {
		ntfs_log_info("ERROR: You must specify a device.\n");
		usage(1);
	}
}

/**
 * OLD_ntfs_volume_set_flags
 */
static int OLD_ntfs_volume_set_flags(ntfs_volume *vol, const le16 flags)
{
	MFT_RECORD *m = NULL;
	ATTR_RECORD *a;
	VOLUME_INFORMATION *c;
	ntfs_attr_search_ctx *ctx;
	int ret = -1;   /* failure */

	if (!vol) {
		errno = EINVAL;
		return -1;
	}
	if (ntfs_file_record_read(vol, FILE_Volume, &m, NULL)) {
		ntfs_log_perror("Failed to read $Volume");
		return -1;
	}
	/* Sanity check */
	if (!(m->flags & MFT_RECORD_IN_USE)) {
		ntfs_log_error("$Volume has been deleted. Cannot handle this "
				"yet. Run chkdsk to fix this.\n");
		errno = EIO;
		goto err_exit;
	}
	/* Get a pointer to the volume information attribute. */
	ctx = ntfs_attr_get_search_ctx(NULL, m);
	if (!ctx) {
		ntfs_log_debug("Failed to allocate attribute search "
				"context.\n");
		goto err_exit;
	}
	if (ntfs_attr_lookup(AT_VOLUME_INFORMATION, AT_UNNAMED, 0,
			CASE_SENSITIVE, 0, NULL, 0, ctx)) {
		ntfs_log_error("Attribute $VOLUME_INFORMATION was not found in "
				"$Volume!\n");
		goto err_out;
	}
	a = ctx->attr;
	/* Sanity check. */
	if (a->non_resident) {
		ntfs_log_error("Attribute $VOLUME_INFORMATION must be resident "
				"(and it isn't)!\n");
		errno = EIO;
		goto err_out;
	}
	/* Get a pointer to the value of the attribute. */
	c = (VOLUME_INFORMATION*)(le16_to_cpu(a->value_offset) + (char*)a);
	/* Sanity checks. */
	if ((char*)c + le32_to_cpu(a->value_length) >
			(char*)m + le32_to_cpu(m->bytes_in_use) ||
			le16_to_cpu(a->value_offset) +
			le32_to_cpu(a->value_length) > le32_to_cpu(a->length)) {
		ntfs_log_error("Attribute $VOLUME_INFORMATION in $Volume is "
				"corrupt!\n");
		errno = EIO;
		goto err_out;
	}
	/* Set the volume flags. */
	vol->flags = c->flags = flags;
	if (ntfs_mft_record_write(vol, FILE_Volume, m)) {
		ntfs_log_perror("Error writing $Volume");
		goto err_out;
	}
	ret = 0; /* success */
err_out:
	ntfs_attr_put_search_ctx(ctx);
err_exit:
	free(m);
	return ret;
}

/**
 * set_dirty_flag
 */
static int set_dirty_flag(ntfs_volume *vol)
{
	le16 flags;

	/* Porting note: We test for the current state of VOLUME_IS_DIRTY. This
	 * should actually be more appropriate than testing for NVolWasDirty. */
	if (vol->flags & VOLUME_IS_DIRTY)
		return 0;
	ntfs_log_info("Setting required flags on partition... ");
	/*
	 * Set chkdsk flag, i.e. mark the partition dirty so chkdsk will run
	 * and fix it for us.
	 */
	flags = vol->flags | VOLUME_IS_DIRTY;
	if (!opt.no_action && OLD_ntfs_volume_set_flags(vol, flags)) {
		ntfs_log_info(FAILED);
		ntfs_log_error("Error setting volume flags.\n");
		return -1;
	}
	vol->flags = flags;

	/* Porting note: libntfs-3g does not have the 'WasDirty' flag/property,
	 * and never touches the 'dirty' bit except when explicitly told to do
	 * so. Since we just wrote the VOLUME_IS_DIRTY bit to disk, and
	 * vol->flags is up-to-date, we can just ignore the NVolSetWasDirty
	 * statement. */
	/* NVolSetWasDirty(vol); */

	ntfs_log_info(OK);
	return 0;
}

/**
 * empty_journal
 */
static int empty_journal(ntfs_volume *vol)
{
	if (NVolLogFileEmpty(vol))
		return 0;
	ntfs_log_info("Going to empty the journal ($LogFile)... ");
	if (ntfs_logfile_reset(vol)) {
		ntfs_log_info(FAILED);
		ntfs_log_perror("Failed to reset $LogFile");
		return -1;
	}
	ntfs_log_info(OK);
	return 0;
}

/*
 *		Clear the sparse flag of an attribute
 */

static int clear_sparse(ntfs_attr *na, const char *name)
{
	ntfs_attr_search_ctx *ctx;
	int res;

	res = -1;
	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (ctx) {
		if (!ntfs_attr_lookup(na->type, na->name, na->name_len,
				CASE_SENSITIVE,	0, NULL, 0, ctx)) {
			na->data_flags &= ~ATTR_IS_SPARSE;
			ctx->attr->data_size = cpu_to_sle64(na->data_size);
			ctx->attr->initialized_size
					= cpu_to_sle64(na->initialized_size);
			ctx->attr->flags = na->data_flags;
			ctx->attr->compression_unit = 0;
			ntfs_inode_mark_dirty(ctx->ntfs_ino);
			NInoFileNameSetDirty(na->ni);
			res = 0;
		} else
			ntfs_log_perror("Could not locate attribute for %s",
						name);
		ntfs_attr_put_search_ctx(ctx);
	} else
		ntfs_log_perror("Could not get a search context for %s",
					name);
	return (res);
}

/**
 *		Clear the bad cluster marks (option)
 */
static int clear_badclus(ntfs_volume *vol)
{
	static ntfschar badstream[] = {
				const_cpu_to_le16('$'), const_cpu_to_le16('B'),
				const_cpu_to_le16('a'), const_cpu_to_le16('d')
	} ;
	ntfs_inode *ni;
	ntfs_attr *na;
	BOOL ok;

	ok = FALSE;
	ntfs_log_info("Going to un-mark the bad clusters ($BadClus)... ");
	ni = ntfs_inode_open(vol, FILE_BadClus);
	if (ni) {
		na = ntfs_attr_open(ni, AT_DATA, badstream, 4);
			/*
			 * chkdsk does not adjust the data size when
			 * moving clusters to $BadClus, so we have to
			 * check the runlist.
			 */
		if (na && !ntfs_attr_map_whole_runlist(na)) {
			if (na->rl
			    && na->rl[0].length && na->rl[1].length) {
			/*
			 * Truncate the stream to free all its clusters,
			 * (which requires setting the data size according
			 * to allocation), then reallocate a sparse stream
			 * to full size of volume and reset the data size.
			 * Note : the sparse flags should not be set.
			 */
				na->data_size = na->allocated_size;
				na->initialized_size = na->allocated_size;
				if (!ntfs_attr_truncate(na,0)
				    && !ntfs_attr_truncate(na,vol->nr_clusters
						<< vol->cluster_size_bits)) {
					na->initialized_size = 0;
					NInoFileNameSetDirty(ni);
					ok = TRUE;
				} else {
					ntfs_log_perror("Failed to un-mark the bad clusters");
				}
			} else {
				ntfs_log_info("No bad clusters...");
				ok = TRUE;
			}
			/*
			 * The sparse flags are not set after an initial
			 * formatting, so do the same.
			 */
			if (ok) {
				ni->flags &= ~FILE_ATTR_SPARSE_FILE;
				ok = !clear_sparse(na, "$BadClus::$Bad");
			}
			ntfs_attr_close(na);
		} else {
			ntfs_log_perror("Failed to open $BadClus::$Bad");
		}
		ntfs_inode_close(ni);
	} else {
		ntfs_log_perror("Failed to open inode FILE_BadClus");
	}
	if (ok)
		ntfs_log_info(OK);
	return (ok ? 0 : -1);
}

/**
 * fix_mftmirr
 */
static int fix_mftmirr(ntfs_volume *vol)
{
	s64 l, br;
	unsigned char *m, *m2;
	int i, ret = -1; /* failure */
	BOOL done;

	ntfs_log_info("\nProcessing $MFT and $MFTMirr...\n");

	/* Load data from $MFT and $MFTMirr and compare the contents. */
	m = (u8*)malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	if (!m) {
		ntfs_log_perror("Failed to allocate memory");
		return -1;
	}
	m2 = (u8*)malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	if (!m2) {
		ntfs_log_perror("Failed to allocate memory");
		free(m);
		return -1;
	}

	ntfs_log_info("Reading $MFT... ");
	l = ntfs_attr_mst_pread(vol->mft_na, 0, vol->mftmirr_size,
			vol->mft_record_size, m);
	if (l != vol->mftmirr_size) {
		ntfs_log_info(FAILED);
		if (l != -1)
			errno = EIO;
		ntfs_log_perror("Failed to read $MFT");
		goto error_exit;
	}
	ntfs_log_info(OK);

	ntfs_log_info("Reading $MFTMirr... ");
	l = ntfs_attr_mst_pread(vol->mftmirr_na, 0, vol->mftmirr_size,
			vol->mft_record_size, m2);
	if (l != vol->mftmirr_size) {
		ntfs_log_info(FAILED);
		if (l != -1)
			errno = EIO;
		ntfs_log_perror("Failed to read $MFTMirr");
		goto error_exit;
	}
	ntfs_log_info(OK);

	/*
	 * FIXME: Need to actually check the $MFTMirr for being real. Otherwise
	 * we might corrupt the partition if someone is experimenting with
	 * software RAID and the $MFTMirr is not actually in the position we
	 * expect it to be... )-:
	 * FIXME: We should emit a warning it $MFTMirr is damaged and ask
	 * user whether to recreate it from $MFT or whether to abort. - The
	 * warning needs to include the danger of software RAID arrays.
	 * Maybe we should go as far as to detect whether we are running on a
	 * MD disk and if yes then bomb out right at the start of the program?
	 */

	ntfs_log_info("Comparing $MFTMirr to $MFT... ");
	done = FALSE;
	/*
	 * Since 2017, Windows 10 does not mirror to full $MFTMirr when
	 * using big clusters, and some records may be found different.
	 * Nevertheless chkdsk.exe mirrors it fully, so we do similarly.
	 */
	for (i = 0; i < vol->mftmirr_size; ++i) {
		MFT_RECORD *mrec, *mrec2;
		const char *ESTR[12] = { "$MFT", "$MFTMirr", "$LogFile",
			"$Volume", "$AttrDef", "root directory", "$Bitmap",
			"$Boot", "$BadClus", "$Secure", "$UpCase", "$Extend" };
		const char *s;
		BOOL use_mirr;

		if (i < 12)
			s = ESTR[i];
		else if (i < 16)
			s = "system file";
		else
			s = "mft record";

		use_mirr = FALSE;
		mrec = (MFT_RECORD*)(m + i * vol->mft_record_size);
		if (mrec->flags & MFT_RECORD_IN_USE) {
			if (ntfs_is_baad_record(mrec->magic)) {
				ntfs_log_info(FAILED);
				ntfs_log_error("$MFT error: Incomplete multi "
						"sector transfer detected in "
						"%s.\nCannot handle this yet. "
						")-:\n", s);
				goto error_exit;
			}
			if (!ntfs_is_mft_record(mrec->magic)) {
				ntfs_log_info(FAILED);
				ntfs_log_error("$MFT error: Invalid mft "
						"record for %s.\nCannot "
						"handle this yet. )-:\n", s);
				goto error_exit;
			}
		}
		mrec2 = (MFT_RECORD*)(m2 + i * vol->mft_record_size);
		if (mrec2->flags & MFT_RECORD_IN_USE) {
			if (ntfs_is_baad_record(mrec2->magic)) {
				ntfs_log_info(FAILED);
				ntfs_log_error("$MFTMirr error: Incomplete "
						"multi sector transfer "
						"detected in %s.\n", s);
				goto error_exit;
			}
			if (!ntfs_is_mft_record(mrec2->magic)) {
				ntfs_log_info(FAILED);
				ntfs_log_error("$MFTMirr error: Invalid mft "
						"record for %s.\n", s);
				goto error_exit;
			}
			/* $MFT is corrupt but $MFTMirr is ok, use $MFTMirr. */
			if (!(mrec->flags & MFT_RECORD_IN_USE) &&
					!ntfs_is_mft_record(mrec->magic))
				use_mirr = TRUE;
		}
		if (memcmp(mrec, mrec2, ntfs_mft_record_get_data_size(mrec))) {
			if (!done) {
				done = TRUE;
				ntfs_log_info(FAILED);
			}
			ntfs_log_info("Correcting differences in $MFT%s "
					"record %d...", use_mirr ? "" : "Mirr",
					i);
			br = ntfs_mft_record_write(vol, i,
					use_mirr ? mrec2 : mrec);
			if (br) {
				ntfs_log_info(FAILED);
				ntfs_log_perror("Error correcting $MFT%s",
						use_mirr ? "" : "Mirr");
				goto error_exit;
			}
			ntfs_log_info(OK);
		}
	}
	if (!done)
		ntfs_log_info(OK);
	ntfs_log_info("Processing of $MFT and $MFTMirr completed "
			"successfully.\n");
	ret = 0;
error_exit:
	free(m);
	free(m2);
	return ret;
}

/*
 *		Rewrite the $UpCase file as default
 *
 *	Returns 0 if could be written
 */

static int rewrite_upcase(ntfs_volume *vol, ntfs_attr *na)
{
	s64 l;
	int res;

		/* writing the $UpCase may require bitmap updates */
	res = -1;
	vol->lcnbmp_ni = ntfs_inode_open(vol, FILE_Bitmap);
	if (!vol->lcnbmp_ni) {
		ntfs_log_perror("Failed to open bitmap inode");
	} else {
		vol->lcnbmp_na = ntfs_attr_open(vol->lcnbmp_ni, AT_DATA,
					AT_UNNAMED, 0);
		if (!vol->lcnbmp_na) {
			ntfs_log_perror("Failed to open bitmap data attribute");
		} else {
			/* minimal consistency check on the bitmap */
			if (((vol->lcnbmp_na->data_size << 3)
				< vol->nr_clusters)
			    || ((vol->lcnbmp_na->data_size << 3)
				>= (vol->nr_clusters << 1))
			    || (vol->lcnbmp_na->data_size
					> vol->lcnbmp_na->allocated_size)) {
				ntfs_log_error("Corrupt cluster map size %lld"
					" (allocated %lld minimum %lld)\n",
					(long long)vol->lcnbmp_na->data_size, 
					(long long)vol->lcnbmp_na->allocated_size,
					(long long)(vol->nr_clusters + 7) >> 3);
			} else {
				ntfs_log_info("Rewriting $UpCase file\n");
				l = ntfs_attr_pwrite(na, 0, vol->upcase_len*2,
							vol->upcase);
				if (l != vol->upcase_len*2) {
					ntfs_log_error("Failed to rewrite $UpCase\n");
				} else {
					ntfs_log_info("$UpCase has been set to default\n");
					res = 0;
				}
			}
			ntfs_attr_close(vol->lcnbmp_na);
			vol->lcnbmp_na = (ntfs_attr*)NULL;
		}
		ntfs_inode_close(vol->lcnbmp_ni);
		vol->lcnbmp_ni = (ntfs_inode*)NULL;
	}
	return (res);
}

/*
 *		Fix the $UpCase file
 *
 *	Returns 0 if the table is valid or has been fixed
 */

static int fix_upcase(ntfs_volume *vol)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	ntfschar *upcase;
	s64 l;
	u32 upcase_len;
	u32 k;
	int res;

	res = -1;
	ni = (ntfs_inode*)NULL;
	na = (ntfs_attr*)NULL;
	/* Now load the upcase table from $UpCase. */
	ntfs_log_debug("Loading $UpCase...\n");
	ni = ntfs_inode_open(vol, FILE_UpCase);
	if (!ni) {
		ntfs_log_perror("Failed to open inode FILE_UpCase");
		goto error_exit;
	}
	/* Get an ntfs attribute for $UpCase/$DATA. */
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}
	/*
	 * Note: Normally, the upcase table has a length equal to 65536
	 * 2-byte Unicode characters but allow for different cases, so no
	 * checks done. Just check we don't overflow 32-bits worth of Unicode
	 * characters.
	 */
	if (na->data_size & ~0x1ffffffffULL) {
		ntfs_log_error("Error: Upcase table is too big (max 32-bit "
				"allowed).\n");
		errno = EINVAL;
		goto error_exit;
	}
	upcase_len = na->data_size >> 1;
	upcase = (ntfschar*)ntfs_malloc(na->data_size);
	if (!upcase)
		goto error_exit;
	/* Read in the $DATA attribute value into the buffer. */
	l = ntfs_attr_pread(na, 0, na->data_size, upcase);
	if (l != na->data_size) {
		ntfs_log_error("Failed to read $UpCase, unexpected length "
			       "(%lld != %lld).\n", (long long)l,
			       (long long)na->data_size);
		errno = EIO;
		goto error_exit;
	}
	/* Consistency check of $UpCase, restricted to plain ASCII chars */
	k = 0x20;
	while ((k < upcase_len)
	    && (k < 0x7f)
	    && (le16_to_cpu(upcase[k])
			== ((k < 'a') || (k > 'z') ? k : k + 'A' - 'a')))
		k++;
	if (k < 0x7f) {
		ntfs_log_error("Corrupted file $UpCase\n");
		if (!opt.no_action) {
			/* rewrite the $UpCase file from default */
			res = rewrite_upcase(vol, na);
			/* free the bad upcase record */
			if (!res)
				free(upcase);
		} else {
			/* keep the default upcase but return an error */
			free(upcase);
		}
	} else {
			/* accept the upcase table read from $UpCase */
		free(vol->upcase);
		vol->upcase = upcase;
		vol->upcase_len = upcase_len;
		res = 0;
	}
error_exit :
	/* Done with the $UpCase mft record. */
	if (na)
		ntfs_attr_close(na);
	if (ni && ntfs_inode_close(ni)) {
		ntfs_log_perror("Failed to close $UpCase");
	}
	return (res);
}

/*
 *		Rewrite the boot sector
 *
 *	Returns 0 if successful
 */

static int rewrite_boot(struct ntfs_device *dev, char *full_bs,
				s32 sector_size)
{
	s64 bw;
	int res;

	res = -1;
	ntfs_log_info("Rewriting the bootsector\n");
	bw = ntfs_pwrite(dev, 0, sector_size, full_bs);
	if (bw == sector_size)
		res = 0;
	else {
		if (bw != -1)
			errno = EINVAL;
		if (!bw)
			ntfs_log_error("Failed to rewrite the bootsector (size=0)\n");
		else
			ntfs_log_perror("Error rewriting the bootsector");
	}
	return (res);
}

/*
 *		Locate an unnamed attribute in an MFT record
 *
 *	Returns NULL if not found (with no error message)
 */

static ATTR_RECORD *find_unnamed_attr(MFT_RECORD *mrec, ATTR_TYPES type)
{
	ATTR_RECORD *a;
	u32 offset;
	s32 space;

			/* fetch the requested attribute */
	offset = le16_to_cpu(mrec->attrs_offset);
	space = le32_to_cpu(mrec->bytes_in_use) - offset;
	a = (ATTR_RECORD*)((char*)mrec + offset);
	while ((space >= (s32)offsetof(ATTR_RECORD, resident_end))
	    && (a->type != AT_END)
	    && (le32_to_cpu(a->length) <= (u32)space)
	    && !(le32_to_cpu(a->length) & 7)
	    && ((a->type != type) || a->name_length)) {
		offset += le32_to_cpu(a->length);
		space -= le32_to_cpu(a->length);
		a = (ATTR_RECORD*)((char*)mrec + offset);
	}
	if ((offset >= le32_to_cpu(mrec->bytes_in_use))
	    || (a->type != type)
	    || a->name_length)
		a = (ATTR_RECORD*)NULL;
	return (a);
}

/*
 *		First condition for having a self-located MFT :
 *		only 16 MFT records are defined in MFT record 0
 *
 *	Only low-level library functions can be used.
 *
 *	Returns TRUE if the condition is met.
 */

static BOOL short_mft_selfloc_condition(struct MFT_SELF_LOCATED *selfloc)
{
	BOOL ok;
	ntfs_volume *vol;
	MFT_RECORD *mft0;
	ATTR_RECORD *a;
	runlist_element *rl;
	u16 seqn;

	ok = FALSE;
	vol = selfloc->vol;
	mft0 = selfloc->mft0;
	if ((ntfs_pread(vol->dev,
			vol->mft_lcn << vol->cluster_size_bits,
			vol->mft_record_size, mft0)
				== vol->mft_record_size)
	    && !ntfs_mst_post_read_fixup((NTFS_RECORD*)mft0,
			vol->mft_record_size)
	    && !ntfs_mft_record_check(vol, 0, mft0)) {
		a = find_unnamed_attr(mft0,AT_DATA);
		if (a
		    && a->non_resident
		    && (((sle64_to_cpu(a->highest_vcn) + 1)
					<< vol->cluster_size_bits)
				== (SELFLOC_LIMIT*vol->mft_record_size))) {
			rl = ntfs_mapping_pairs_decompress(vol, a, NULL);
			if (rl) {
				/*
				 * The first error condition is having only
				 * 16 entries mapped in the first MFT record.
				 */
				if ((rl[0].lcn >= 0)
				  && ((rl[0].length << vol->cluster_size_bits)
					== SELFLOC_LIMIT*vol->mft_record_size)
				  && (rl[1].vcn == rl[0].length)
				  && (rl[1].lcn == LCN_RL_NOT_MAPPED)) {
					ok = TRUE;
					seqn = le16_to_cpu(
						mft0->sequence_number);
					selfloc->mft_ref0
						= ((MFT_REF)seqn) << 48;
				}
				free(rl);
			}
		}
	}
	return (ok);
}

/*
 *		Second condition for having a self-located MFT :
 *		The 16th MFT record is defined in MFT record >= 16
 *
 *	Only low-level library functions can be used.
 *
 *	Returns TRUE if the condition is met.
 */

static BOOL attrlist_selfloc_condition(struct MFT_SELF_LOCATED *selfloc)
{
	ntfs_volume *vol;
	ATTR_RECORD *a;
	ATTR_LIST_ENTRY *attrlist;
	ATTR_LIST_ENTRY *al;
	runlist_element *rl;
	VCN vcn;
	leVCN levcn;
	u32 length;
	int ok;

	ok = FALSE;
	length = 0;
	vol = selfloc->vol;
	a = find_unnamed_attr(selfloc->mft0,AT_ATTRIBUTE_LIST);
	if (a) {
		selfloc->attrlist_resident = !a->non_resident;
		selfloc->attrlist_lcn = 0;
		if (a->non_resident) {
			attrlist = selfloc->attrlist;
			rl = ntfs_mapping_pairs_decompress(vol, a, NULL);
			if (rl
			    && (rl->lcn >= 0)
			    && (sle64_to_cpu(a->data_size) < vol->cluster_size)
			    && (ntfs_pread(vol->dev,
					rl->lcn << vol->cluster_size_bits,
					vol->cluster_size, attrlist) == vol->cluster_size)) {
				selfloc->attrlist_lcn = rl->lcn;
				al = attrlist;
				length = sle64_to_cpu(a->data_size);
			}
		} else {
			al = (ATTR_LIST_ENTRY*)
				((char*)a + le16_to_cpu(a->value_offset));
			length = le32_to_cpu(a->value_length);
		}
		if (length) {
			/* search for a data attribute defining entry 16 */
			vcn = (SELFLOC_LIMIT*vol->mft_record_size)
					>> vol->cluster_size_bits;
			levcn = cpu_to_sle64(vcn);
			while ((length > 0)
			    && al->length
			    && ((al->type != AT_DATA)
				|| ((leVCN)al->lowest_vcn != levcn))) {
				length -= le16_to_cpu(al->length);
				al = (ATTR_LIST_ENTRY*)
					((char*)al + le16_to_cpu(al->length));
			}
			if ((length > 0)
			    && al->length
			    && (al->type == AT_DATA)
			    && !al->name_length
			    && ((leVCN)al->lowest_vcn == levcn)
			    && (MREF_LE(al->mft_reference) >= SELFLOC_LIMIT)) {
				selfloc->mft_ref1
					= le64_to_cpu(al->mft_reference);
				selfloc->attrlist_to_ref1 = al;
				ok = TRUE;
			}
		}
	}
	return (ok);
}

/*
 *		Third condition for having a self-located MFT :
 *		The location of the second part of the MFT is defined in itself
 *
 *	To locate the second part, we have to assume the first and the
 *	second part of the MFT data are contiguous.
 *
 *	Only low-level library functions can be used.
 *
 *	Returns TRUE if the condition is met.
 */

static BOOL self_mapped_selfloc_condition(struct MFT_SELF_LOCATED *selfloc)
{
	BOOL ok;
	s64 inum;
	u64 offs;
	VCN lowest_vcn;
	MFT_RECORD *mft1;
	ATTR_RECORD *a;
	ntfs_volume *vol;
	runlist_element *rl;

	ok = FALSE;
	vol = selfloc->vol;
	mft1 = selfloc->mft1;
	inum = MREF(selfloc->mft_ref1);
	offs = 	(vol->mft_lcn << vol->cluster_size_bits)
			+ (inum << vol->mft_record_size_bits);
	if ((ntfs_pread(vol->dev, offs, vol->mft_record_size,
			mft1) == vol->mft_record_size)
	    && !ntfs_mst_post_read_fixup((NTFS_RECORD*)mft1,
			vol->mft_record_size)
	    && !ntfs_mft_record_check(vol, inum, mft1)) {

		lowest_vcn = (SELFLOC_LIMIT*vol->mft_record_size)
				>> vol->cluster_size_bits;
		a = find_unnamed_attr(mft1,AT_DATA);
		if (a
		    && (mft1->flags & MFT_RECORD_IN_USE)
		    && ((VCN)sle64_to_cpu(a->lowest_vcn) == lowest_vcn)
		    && (le64_to_cpu(mft1->base_mft_record)
				== selfloc->mft_ref0)
		    && ((u16)MSEQNO(selfloc->mft_ref1)
				== le16_to_cpu(mft1->sequence_number))) {
			rl = ntfs_mapping_pairs_decompress(vol, a, NULL);
			if ((rl[0].lcn == LCN_RL_NOT_MAPPED)
			   && !rl[0].vcn
			   && (rl[0].length == lowest_vcn)
			   && (rl[1].vcn == lowest_vcn)
			   && ((u64)(rl[1].lcn << vol->cluster_size_bits)
					<= offs)
			   && ((u64)((rl[1].lcn + rl[1].length)
					<< vol->cluster_size_bits) > offs)) {
				ok = TRUE;
			}
		}
	}
	return (ok);
}

/*
 *		Fourth condition, to be able to fix a self-located MFT :
 *		The MFT record 15 must be available.
 *
 *	The MFT record 15 is expected to be marked in use, we assume
 *	it is available if it has no parent, no name and no attr list.
 *
 *	Only low-level library functions can be used.
 *
 *	Returns TRUE if the condition is met.
 */

static BOOL spare_record_selfloc_condition(struct MFT_SELF_LOCATED *selfloc)
{
	BOOL ok;
	s64 inum;
	u64 offs;
	MFT_RECORD *mft2;
	ntfs_volume *vol;

	ok = FALSE;
	vol = selfloc->vol;
	mft2 = selfloc->mft2;
	inum = SELFLOC_LIMIT - 1;
	offs = 	(vol->mft_lcn << vol->cluster_size_bits)
			+ (inum << vol->mft_record_size_bits);
	if ((ntfs_pread(vol->dev, offs, vol->mft_record_size,
			mft2) == vol->mft_record_size)
	    && !ntfs_mst_post_read_fixup((NTFS_RECORD*)mft2,
			vol->mft_record_size)
	    && !ntfs_mft_record_check(vol, inum, mft2)) {
		if (!mft2->base_mft_record
		    && (mft2->flags & MFT_RECORD_IN_USE)
		    && !find_unnamed_attr(mft2,AT_ATTRIBUTE_LIST)
		    && !find_unnamed_attr(mft2,AT_FILE_NAME)) {
			ok = TRUE;
		}
	}
	return (ok);
}

/*
 *		Fix a self-located MFT by swapping two MFT records
 *
 *	Only low-level library functions can be used.
 *
 *	Returns 0 if the MFT corruption could be fixed.
 */
static int fix_selfloc_conditions(struct MFT_SELF_LOCATED *selfloc)
{
	MFT_RECORD *mft1;
	MFT_RECORD *mft2;
	ATTR_RECORD *a;
	ATTR_LIST_ENTRY *al;
	ntfs_volume *vol;
	s64 offs;
	s64 offsm;
	s64 offs1;
	s64 offs2;
	s64 inum;
	u16 usa_ofs;
	int res;

	res = 0;
			/*
			 * In MFT1, we must fix :
			 * - the self-reference, if present,
			 * - its own sequence number, must be 15
			 * - the sizes of the data attribute.
			 */
	vol = selfloc->vol;
	mft1 = selfloc->mft1;
	mft2 = selfloc->mft2;
	usa_ofs = le16_to_cpu(mft1->usa_ofs);
	if (usa_ofs >= 48)
		mft1->mft_record_number = const_cpu_to_le32(SELFLOC_LIMIT - 1);
	mft1->sequence_number = const_cpu_to_le16(SELFLOC_LIMIT - 1);
	a = find_unnamed_attr(mft1,AT_DATA);
	if (a) {
		a->allocated_size = const_cpu_to_sle64(0);
		a->data_size = const_cpu_to_sle64(0);
		a->initialized_size = const_cpu_to_sle64(0);
	} else
		res = -1; /* bug : it has been found earlier */

			/*
			 * In MFT2, we must fix :
			 * - the self-reference, if present
			 */
	usa_ofs = le16_to_cpu(mft2->usa_ofs);
	if (usa_ofs >= 48)
		mft2->mft_record_number = cpu_to_le32(MREF(selfloc->mft_ref1));

			/*
			 * In the attribute list, we must fix :
			 * - the reference to MFT1
			 */
	al = selfloc->attrlist_to_ref1;
	al->mft_reference = MK_LE_MREF(SELFLOC_LIMIT - 1, SELFLOC_LIMIT - 1);

			/*
			 * All fixes done, we can write all if allowed
			 */
	if (!res && !opt.no_action) {
		inum = SELFLOC_LIMIT - 1;
		offs2 = (vol->mft_lcn << vol->cluster_size_bits)
			+ (inum << vol->mft_record_size_bits);
		inum = MREF(selfloc->mft_ref1);
		offs1 = (vol->mft_lcn << vol->cluster_size_bits)
			+ (inum << vol->mft_record_size_bits);

			/* rewrite the attribute list */
		if (selfloc->attrlist_resident) {
				/* write mft0 and mftmirr if it is resident */
			offs = vol->mft_lcn << vol->cluster_size_bits;
			offsm = vol->mftmirr_lcn << vol->cluster_size_bits;
			if (ntfs_mst_pre_write_fixup(
					(NTFS_RECORD*)selfloc->mft0,
					vol->mft_record_size)
			    || (ntfs_pwrite(vol->dev, offs, vol->mft_record_size,
					selfloc->mft0) != vol->mft_record_size)
			    || (ntfs_pwrite(vol->dev, offsm, vol->mft_record_size,
					selfloc->mft0) != vol->mft_record_size))
				res = -1;
		} else {
				/* write a full cluster if non resident */
			offs = selfloc->attrlist_lcn << vol->cluster_size_bits;
			if (ntfs_pwrite(vol->dev, offs, vol->cluster_size,
					selfloc->attrlist) != vol->cluster_size)
				res = -1;
		}
			/* replace MFT2 by MFT1 and replace MFT1 by MFT2 */
		if (!res 
		    && (ntfs_mst_pre_write_fixup((NTFS_RECORD*)selfloc->mft1,
					vol->mft_record_size)
			|| ntfs_mst_pre_write_fixup((NTFS_RECORD*)selfloc->mft2,
					vol->mft_record_size)
			|| (ntfs_pwrite(vol->dev, offs2, vol->mft_record_size,
					mft1) != vol->mft_record_size)
			|| (ntfs_pwrite(vol->dev, offs1, vol->mft_record_size,
					mft2) != vol->mft_record_size)))
				res = -1;
	}
	return (res);
}

/*
 *		Detect and fix a Windows XP bug, leading to a corrupt MFT
 *
 *	Windows cannot boot anymore, so chkdsk cannot be started, which
 *	is a good point, because chkdsk would have deleted all the files.
 *	Older ntfs-3g fell into an endless recursion (recent versions
 *	refuse to mount).
 *
 *	This situation is very rare, but it was fun to fix it.
 *
 *	The corrupted condition is :
 *		- MFT entry 0 has only the runlist for MFT entries 0-15
 *		- The attribute list for MFT shows the second part
 *			in an MFT record beyond 15
 *	Of course, this record has to be read in order to know where it is.
 *
 *	Sample case, met in 2011 (Windows XP) :
 *		MFT record 0 has : stdinfo, nonres attrlist, the first
 *				part of MFT data (entries 0-15), and bitmap
 *		MFT record 16 has the name
 *		MFT record 17 has the third part of MFT data (16-117731)
 *		MFT record 18 has the second part of MFT data (117732-170908)
 *
 *	Assuming the second part of the MFT is contiguous to the first
 *	part, we can find it, and fix the condition by relocating it
 *	and swapping it with MFT record 15.
 *	This record number 15 appears to be hardcoded into Windows NTFS.
 *
 *	Only low-level library functions can be used.
 *
 *	Returns 0 if the conditions for the error was met and
 *			this error could be fixed,
 *		-1 if the condition was not met or some error
 *			which could not be fixed was encountered.
 */

static int fix_self_located_mft(ntfs_volume *vol)
{
	struct MFT_SELF_LOCATED selfloc;
	BOOL res;

	ntfs_log_info("Checking for self-located MFT segment... ");
	res = -1;
	selfloc.vol = vol;
	selfloc.mft0 = (MFT_RECORD*)malloc(vol->mft_record_size);
	selfloc.mft1 = (MFT_RECORD*)malloc(vol->mft_record_size);
	selfloc.mft2 = (MFT_RECORD*)malloc(vol->mft_record_size);
	selfloc.attrlist = (ATTR_LIST_ENTRY*)malloc(vol->cluster_size);
	if (selfloc.mft0 && selfloc.mft1 && selfloc.mft2
	    && selfloc.attrlist) {
		if (short_mft_selfloc_condition(&selfloc)
		    && attrlist_selfloc_condition(&selfloc)
		    && self_mapped_selfloc_condition(&selfloc)
		    && spare_record_selfloc_condition(&selfloc)) {
			ntfs_log_info(FOUND);
			ntfs_log_info("Fixing the self-located MFT segment... ");
			res = fix_selfloc_conditions(&selfloc);
			ntfs_log_info(res ? FAILED : OK);
		} else {
			ntfs_log_info(OK);
			res = -1;
		}
		free(selfloc.mft0);
		free(selfloc.mft1);
		free(selfloc.mft2);
		free(selfloc.attrlist);
	}
	return (res);
}

/*
 *		Try an alternate boot sector and fix the real one
 *
 *	Only after successful checks is the boot sector rewritten.
 *
 *	The alternate boot sector is not rewritten, either because it
 *	was found correct, or because we truncated the file system
 *	and the last actual sector might be part of some file.
 *
 *	Returns 0 if successful
 */

static int try_fix_boot(ntfs_volume *vol, char *full_bs,
			s64 read_sector, s64 fix_sectors, s32 sector_size)
{
	s64 br;
	int res;
	s64 got_sectors;
	le16 sector_size_le;
	NTFS_BOOT_SECTOR *bs;

	res = -1;
	br = ntfs_pread(vol->dev, read_sector*sector_size,
					sector_size, full_bs);
	if (br != sector_size) {
		if (br != -1)
			errno = EINVAL;
		if (!br)
			ntfs_log_error("Failed to read alternate bootsector (size=0)\n");
		else
			ntfs_log_perror("Error reading alternate bootsector");
	} else {
		bs = (NTFS_BOOT_SECTOR*)full_bs;
		got_sectors = sle64_to_cpu(bs->number_of_sectors);
		bs->number_of_sectors = cpu_to_sle64(fix_sectors);
		/* alignment problem on Sparc, even doing memcpy() */
		sector_size_le = cpu_to_le16(sector_size);
		if (!memcmp(&sector_size_le, &bs->bpb.bytes_per_sector,2)
		    && ntfs_boot_sector_is_ntfs(bs)
		    && !ntfs_boot_sector_parse(vol, bs)) {
			ntfs_log_info("The alternate bootsector is usable\n");
			if (fix_sectors != got_sectors)
				ntfs_log_info("Set sector count to %lld instead of %lld\n",
						(long long)fix_sectors,
						(long long)got_sectors);
			/* fix the normal boot sector */
			if (!opt.no_action) {
				res = rewrite_boot(vol->dev, full_bs,
							sector_size);
			} else
				res = 0;
		}
		if (!res && !opt.no_action)
			ntfs_log_info("The boot sector has been rewritten\n");
	}
	return (res);
}

/*
 *		Try the alternate boot sector if the normal one is bad
 *
 *	Actually :
 *	- first try the last sector of the partition (expected location)
 *	- then try the last sector as shown in the main boot sector,
 *		(could be meaningful for an undersized partition)
 *	- finally try truncating the file system actual size of partition
 *		(could be meaningful for an oversized partition)
 *
 *	if successful, rewrite the normal boot sector accordingly
 *
 *	Returns 0 if successful
 */

static int try_alternate_boot(ntfs_volume *vol, char *full_bs,
			s32 sector_size, s64 shown_sectors)
{
	s64 actual_sectors;
	int res;

	res = -1;
	ntfs_log_info("Trying the alternate boot sector\n");

		/*
		 * We do not rely on the sector size defined in the
		 * boot sector, supposed to be corrupt, so we try to get
		 * the actual sector size and defaulting to 512 if failed
		 * to get. This value is only used to guess the alternate
		 * boot sector location and it is checked against the
		 * value found in the sector itself. It should not damage
		 * anything if wrong.
		 *
		 * Note : the real last sector is not accounted for here.
		 */
	actual_sectors = ntfs_device_size_get(vol->dev,sector_size) - 1;

		/* first try the actual last sector */
	if ((actual_sectors > 0)
	    && !try_fix_boot(vol, full_bs, actual_sectors,
				actual_sectors, sector_size))
		res = 0;

		/* then try the shown last sector, if less than actual */
	if (res
	    && (shown_sectors > 0)
	    && (shown_sectors < actual_sectors)
	    && !try_fix_boot(vol, full_bs, shown_sectors,
				shown_sectors, sector_size))
		res = 0;

		/* then try reducing the number of sectors to actual value */
	if (res
	    && (shown_sectors > actual_sectors)
	    && !try_fix_boot(vol, full_bs, 0, actual_sectors, sector_size))
		res = 0;

	return (res);
}

/*
 *		Check and fix the alternate boot sector
 *
 *	The alternate boot sector is usually in the last sector of a
 *	partition, which should not be used by the file system
 *	(the sector count in the boot sector should be less than
 *	the total sector count in the partition).
 *
 *	chkdsk never changes the count in the boot sector.
 *	- If this is less than the total count, chkdsk place the
 *	  alternate boot sector into the sector,
 *	- if the count is the same as the total count, chkdsk place
 *	  the alternate boot sector into the middle sector (half
 *	  the total count rounded upwards)
 *	- if the count is greater than the total count, chkdsk
 *	  declares the file system as raw, and refuses to fix anything.
 *
 *	Here, we check and fix the alternate boot sector, only in the
 *	first situation where the file system does not overflow on the
 *	last sector.
 *
 *	Note : when shrinking a partition, ntfsresize cannot determine
 *	the future size of the partition. As a consequence the number of
 *	sectors in the boot sectors may be less than the possible size.
 *
 *	Returns 0 if successful
 */

static int check_alternate_boot(ntfs_volume *vol)
{
	s64 got_sectors;
	s64 actual_sectors;
	s64 last_sector_off;
	char *full_bs;
	char *alt_bs;
	NTFS_BOOT_SECTOR *bs;
	s64 br;
	s64 bw;
	int res;

	res = -1;
	full_bs = (char*)malloc(vol->sector_size);
	alt_bs = (char*)malloc(vol->sector_size);
	if (!full_bs || !alt_bs) {
		ntfs_log_info("Error : failed to allocate memory\n");
		goto error_exit;
	}
	/* Now read both bootsectors. */
	br = ntfs_pread(vol->dev, 0, vol->sector_size, full_bs);
	if (br == vol->sector_size) {
		bs = (NTFS_BOOT_SECTOR*)full_bs;
		got_sectors = sle64_to_cpu(bs->number_of_sectors);
		actual_sectors = ntfs_device_size_get(vol->dev,
						vol->sector_size);
		if (actual_sectors > got_sectors) {
			last_sector_off = (actual_sectors - 1)
						<< vol->sector_size_bits;
			ntfs_log_info("Checking the alternate boot sector... ");
			br = ntfs_pread(vol->dev, last_sector_off,
						vol->sector_size, alt_bs);
		} else {
			ntfs_log_info("Checking file system overflow... ");
			br = -1;
		}
		/* accept getting no byte, needed for short image files */
		if (br >= 0) {
			if ((br != vol->sector_size)
			    || memcmp(full_bs, alt_bs, vol->sector_size)) {
				if (opt.no_action) {
					ntfs_log_info("BAD\n");
				} else {
					bw = ntfs_pwrite(vol->dev,
						last_sector_off,
						vol->sector_size, full_bs);
					if (bw == vol->sector_size) {
						ntfs_log_info("FIXED\n");
						res = 0;
					} else {
						ntfs_log_info(FAILED);
					}
				}
			} else {
				ntfs_log_info(OK);
				res = 0;
			}
		} else {
			ntfs_log_info(FAILED);
		}
	} else {
		ntfs_log_info("Error : could not read the boot sector again\n");
	}
	free(full_bs);
	free(alt_bs);

error_exit :
	return (res);
}

/*
 *		Try to fix problems which may arise in the start up sequence
 *
 *	This is a replay of the normal start up sequence with fixes when
 *	some problem arise.
 *
 *	Returns 0 if there was an error and a fix is available
 */

static int fix_startup(struct ntfs_device *dev, unsigned long flags)
{
	s64 br;
	ntfs_volume *vol;
	BOOL dev_open;
	s64 shown_sectors;
	char *full_bs;
	NTFS_BOOT_SECTOR *bs;
	s32 sector_size;
	int res;
	int eo;

	errno = 0;
	res = -1;
	dev_open = FALSE;
	full_bs = (char*)NULL;
	if (!dev || !dev->d_ops || !dev->d_name) {
		errno = EINVAL;
		ntfs_log_perror("%s: dev = %p", __FUNCTION__, dev);
		vol = (ntfs_volume*)NULL;
		goto error_exit;
	}

	/* Allocate the volume structure. */
	vol = ntfs_volume_alloc();
	if (!vol)
		goto error_exit;
	
	/* Create the default upcase table. */
	vol->upcase_len = ntfs_upcase_build_default(&vol->upcase);
	if (!vol->upcase_len || !vol->upcase)
		goto error_exit;

	/* Default with no locase table and case sensitive file names */
	vol->locase = (ntfschar*)NULL;
	NVolSetCaseSensitive(vol);
	
		/* by default, all files are shown and not marked hidden */
	NVolSetShowSysFiles(vol);
	NVolSetShowHidFiles(vol);
	NVolClearHideDotFiles(vol);
	if (flags & NTFS_MNT_RDONLY)
		NVolSetReadOnly(vol);
	
	/* ...->open needs bracketing to compile with glibc 2.7 */
	if ((dev->d_ops->open)(dev, NVolReadOnly(vol) ? O_RDONLY: O_RDWR)) {
		ntfs_log_perror("Error opening '%s'", dev->d_name);
		goto error_exit;
	}
	dev_open = TRUE;
	/* Attach the device to the volume. */
	vol->dev = dev;
	
	sector_size = ntfs_device_sector_size_get(dev);
	if (sector_size <= 0)
		sector_size = DEFAULT_SECTOR_SIZE;
	full_bs = (char*)malloc(sector_size);
	if (!full_bs)
		goto error_exit;
	/* Now read the bootsector. */
	br = ntfs_pread(dev, 0, sector_size, full_bs);
	if (br != sector_size) {
		if (br != -1)
			errno = EINVAL;
		if (!br)
			ntfs_log_error("Failed to read bootsector (size=0)\n");
		else
			ntfs_log_perror("Error reading bootsector");
		goto error_exit;
	}
	bs = (NTFS_BOOT_SECTOR*)full_bs;
	if (!ntfs_boot_sector_is_ntfs(bs)
		/* get the bootsector data, only fails when inconsistent */
	    || (ntfs_boot_sector_parse(vol, bs) < 0)) {
		shown_sectors = sle64_to_cpu(bs->number_of_sectors);
		/* boot sector is wrong, try the alternate boot sector */
		if (try_alternate_boot(vol, full_bs, sector_size,
						shown_sectors)) {
			errno = EINVAL;
			goto error_exit;
		}
		res = 0;
	} else {
		res = fix_self_located_mft(vol);
	}
error_exit:
	if (res) {
		switch (errno) {
		case ENOMEM :
			ntfs_log_error("Failed to allocate memory\n");
			break;
		case EINVAL :
			ntfs_log_error("Unrecoverable error\n");
			break;
		default :
			break;
		}
	}
	eo = errno;
	free(full_bs);
	if (vol) {
		free(vol->upcase);
		free(vol);
	}
	if (dev_open) {
		(dev->d_ops->close)(dev);
	}
	errno = eo;
	return (res);
}

/**
 * fix_mount
 */
static int fix_mount(void)
{
	int ret = 0; /* default success */
	ntfs_volume *vol;
	struct ntfs_device *dev;
	unsigned long flags;

	ntfs_log_info("Attempting to correct errors... ");

	dev = ntfs_device_alloc(opt.volume, 0, &ntfs_device_default_io_ops,
			NULL);
	if (!dev) {
		ntfs_log_info(FAILED);
		ntfs_log_perror("Failed to allocate device");
		return -1;
	}
	flags = (opt.no_action ? NTFS_MNT_RDONLY : 0);
	vol = ntfs_volume_startup(dev, flags);
	if (!vol) {
		ntfs_log_info(FAILED);
		ntfs_log_perror("Failed to startup volume");

		/* Try fixing the bootsector and MFT, then redo the startup */
		if (!fix_startup(dev, flags)) {
			if (opt.no_action)
				ntfs_log_info("The startup data can be fixed, "
						"but no change was requested\n");
			else
				vol = ntfs_volume_startup(dev, flags);
		}
		if (!vol) {
			ntfs_log_error("Volume is corrupt. You should run chkdsk.\n");
			ntfs_device_free(dev);
			return -1;
		}
		if (opt.no_action)
			ret = -1; /* error present and not fixed */
	}
		/* if option -n proceed despite errors, to display them all */
	if ((!ret || opt.no_action) && (fix_mftmirr(vol) < 0))
		ret = -1;
	if ((!ret || opt.no_action) && (fix_upcase(vol) < 0))
		ret = -1;
	if ((!ret || opt.no_action) && (set_dirty_flag(vol) < 0))
		ret = -1;
	if ((!ret || opt.no_action) && (empty_journal(vol) < 0))
		ret = -1;
	/*
	 * ntfs_umount() will invoke ntfs_device_free() for us.
	 * Ignore the returned error resulting from partial mounting.
	 */
	ntfs_umount(vol, 1);
	return ret;
}

/**
 * main
 */
int main(int argc, char **argv)
{
	ntfs_volume *vol;
	unsigned long mnt_flags;
	unsigned long flags;
	int ret = 1; /* failure */
	BOOL force = FALSE;

	ntfs_log_set_handler(ntfs_log_handler_outerr);

	parse_options(argc, argv);

	if (!ntfs_check_if_mounted(opt.volume, &mnt_flags)) {
		if ((mnt_flags & NTFS_MF_MOUNTED) &&
				!(mnt_flags & NTFS_MF_READONLY) && !force) {
			ntfs_log_error("Refusing to operate on read-write "
					"mounted device %s.\n", opt.volume);
			exit(1);
		}
	} else
		ntfs_log_perror("Failed to determine whether %s is mounted",
				opt.volume);
	/* Attempt a full mount first. */
	flags = (opt.no_action ? NTFS_MNT_RDONLY : 0);
	ntfs_log_info("Mounting volume... ");
	vol = ntfs_mount(opt.volume, flags);
	if (vol) {
		ntfs_log_info(OK);
		ntfs_log_info("Processing of $MFT and $MFTMirr completed "
				"successfully.\n");
	} else {
		ntfs_log_info(FAILED);
		if (fix_mount() < 0) {
			if (opt.no_action)
				ntfs_log_info("No change made\n");
			exit(1);
		}
		vol = ntfs_mount(opt.volume, 0);
		if (!vol) {
			ntfs_log_perror("Remount failed");
			exit(1);
		}
	}
	if (check_alternate_boot(vol)) {
		ntfs_log_error("Error: Failed to fix the alternate boot sector\n");
		exit(1);
	}
	/* So the unmount does not clear it again. */

	/* Porting note: The WasDirty flag was set here to prevent ntfs_unmount
	 * from clearing the dirty bit (which might have been set in
	 * fix_mount()). So the intention is to leave the dirty bit set.
	 *
	 * libntfs-3g does not automatically set or clear dirty flags on
	 * mount/unmount, this means that the assumption that the dirty flag is
	 * now set does not hold. So we need to set it if not already set.
	 *
	 * However clear the flag if requested to do so, at this stage
	 * mounting was successful.
	 */
	if (opt.clear_dirty)
		vol->flags &= ~VOLUME_IS_DIRTY;
	else
		vol->flags |= VOLUME_IS_DIRTY;
	if (!opt.no_action && ntfs_volume_write_flags(vol, vol->flags)) {
		ntfs_log_error("Error: Failed to set volume dirty flag (%d "
			"(%s))!\n", errno, strerror(errno));
	}

	/* Check NTFS version is ok for us (in $Volume) */
	ntfs_log_info("NTFS volume version is %i.%i.\n", vol->major_ver,
			vol->minor_ver);
	if (ntfs_version_is_supported(vol)) {
		ntfs_log_error("Error: Unknown NTFS version.\n");
		goto error_exit;
	}
	if (opt.clear_bad_sectors && !opt.no_action) {
		if (clear_badclus(vol)) {
			ntfs_log_error("Error: Failed to un-mark bad sectors.\n");
			goto error_exit;
		}
	}
	if (vol->major_ver >= 3) {
		/*
		 * FIXME: If on NTFS 3.0+, check for presence of the usn
		 * journal and stamp it if present.
		 */
	}
	/* FIXME: We should be marking the quota out of date, too. */
	/* That's all for now! */
	ntfs_log_info("NTFS partition %s was processed successfully.\n",
			vol->dev->d_name);
	/* Set return code to 0. */
	ret = 0;
error_exit:
	if (ntfs_umount(vol, 1)) {
		ntfs_log_info("Failed to unmount partition\n");
		ret = 1;
	}
	if (ret)
		exit(ret);
	return ret;
}

