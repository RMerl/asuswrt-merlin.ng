/*
 * tune2fs.c - Change the file system parameters on an ext2 file system
 *
 * Copyright (C) 1992, 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                                 Laboratoire MASI, Institut Blaise Pascal
 *                                 Universite Pierre et Marie Curie (Paris VI)
 *
 * Copyright 1995, 1996, 1997, 1998, 1999, 2000 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/*
 * History:
 * 93/06/01	- Creation
 * 93/10/31	- Added the -c option to change the maximal mount counts
 * 93/12/14	- Added -l flag to list contents of superblock
 *                M.J.E. Mol (marcel@duteca.et.tudelft.nl)
 *                F.W. ten Wolde (franky@duteca.et.tudelft.nl)
 * 93/12/29	- Added the -e option to change errors behavior
 * 94/02/27	- Ported to use the ext2fs library
 * 94/03/06	- Added the checks interval from Uwe Ohse (uwe@tirka.gun.de)
 */

#define _XOPEN_SOURCE 600 /* for inclusion of strptime() */
#include "config.h"
#include <fcntl.h>
#include <grp.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#include <pwd.h>
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>	/* for strcasecmp() */
#else
#define _BSD_SOURCE	/* for inclusion of strcasecmp() via <string.h> */
#define _DEFAULT_SOURCE	  /* since glibc 2.20 _BSD_SOURCE is deprecated */
#endif
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <limits.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "ext2fs/kernel-jbd.h"
#include "et/com_err.h"
#include "support/plausible.h"
#include "support/quotaio.h"
#include "uuid/uuid.h"
#include "e2p/e2p.h"
#include "util.h"
#include "blkid/blkid.h"

#include "../version.h"
#include "support/nls-enable.h"

#define QOPT_ENABLE	(1)
#define QOPT_DISABLE	(-1)

extern int ask_yn(const char *string, int def);

const char *program_name = "tune2fs";
char *device_name;
char *new_label, *new_last_mounted, *new_UUID;
char *io_options;
static int c_flag, C_flag, e_flag, f_flag, g_flag, i_flag, l_flag, L_flag;
static int m_flag, M_flag, Q_flag, r_flag, s_flag = -1, u_flag, U_flag, T_flag;
static int I_flag;
static int clear_mmp;
static time_t last_check_time;
static int print_label;
static int max_mount_count, mount_count, mount_flags;
static unsigned long interval;
static blk64_t reserved_blocks;
static double reserved_ratio;
static unsigned long resgid, resuid;
static unsigned short errors;
static int open_flag;
static char *features_cmd;
static char *mntopts_cmd;
static int stride, stripe_width;
static int stride_set, stripe_width_set;
static char *extended_cmd;
static unsigned long new_inode_size;
static char *ext_mount_opts;
static int quota_enable[MAXQUOTAS];
static int rewrite_checksums;
static int feature_64bit;
static int fsck_requested;
static char *undo_file;

int journal_size, journal_flags;
char *journal_device;
static blk64_t journal_location = ~0LL;

static struct list_head blk_move_list;

struct blk_move {
	struct list_head list;
	blk64_t old_loc;
	blk64_t new_loc;
};

errcode_t ext2fs_run_ext3_journal(ext2_filsys *fs);

static const char *fsck_explain = N_("\nThis operation requires a freshly checked filesystem.\n");

static const char *please_fsck = N_("Please run e2fsck -f on the filesystem.\n");
static const char *please_dir_fsck =
		N_("Please run e2fsck -fD on the filesystem.\n");

#ifdef CONFIG_BUILD_FINDFS
void do_findfs(int argc, char **argv);
#endif

#ifdef CONFIG_JBD_DEBUG		/* Enabled by configure --enable-jbd-debug */
int journal_enable_debug = -1;
#endif

static void usage(void)
{
	fprintf(stderr,
		_("Usage: %s [-c max_mounts_count] [-e errors_behavior] [-f] "
		  "[-g group]\n"
		  "\t[-i interval[d|m|w]] [-j] [-J journal_options] [-l]\n"
		  "\t[-m reserved_blocks_percent] [-o [^]mount_options[,...]]\n"
		  "\t[-r reserved_blocks_count] [-u user] [-C mount_count]\n"
		  "\t[-L volume_label] [-M last_mounted_dir]\n"
		  "\t[-O [^]feature[,...]] [-Q quota_options]\n"
		  "\t[-E extended-option[,...]] [-T last_check_time] "
		  "[-U UUID]\n\t[-I new_inode_size] [-z undo_file] device\n"),
		program_name);
	exit(1);
}

static __u32 ok_features[3] = {
	/* Compat */
	EXT3_FEATURE_COMPAT_HAS_JOURNAL |
		EXT2_FEATURE_COMPAT_DIR_INDEX,
	/* Incompat */
	EXT2_FEATURE_INCOMPAT_FILETYPE |
		EXT3_FEATURE_INCOMPAT_EXTENTS |
		EXT4_FEATURE_INCOMPAT_FLEX_BG |
		EXT4_FEATURE_INCOMPAT_EA_INODE|
		EXT4_FEATURE_INCOMPAT_MMP |
		EXT4_FEATURE_INCOMPAT_64BIT |
		EXT4_FEATURE_INCOMPAT_ENCRYPT |
		EXT4_FEATURE_INCOMPAT_CSUM_SEED |
		EXT4_FEATURE_INCOMPAT_LARGEDIR,
	/* R/O compat */
	EXT2_FEATURE_RO_COMPAT_LARGE_FILE |
		EXT4_FEATURE_RO_COMPAT_HUGE_FILE|
		EXT4_FEATURE_RO_COMPAT_DIR_NLINK|
		EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE|
		EXT4_FEATURE_RO_COMPAT_GDT_CSUM |
		EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER |
		EXT4_FEATURE_RO_COMPAT_QUOTA |
		EXT4_FEATURE_RO_COMPAT_METADATA_CSUM |
		EXT4_FEATURE_RO_COMPAT_READONLY |
		EXT4_FEATURE_RO_COMPAT_PROJECT |
		EXT4_FEATURE_RO_COMPAT_VERITY
};

static __u32 clear_ok_features[3] = {
	/* Compat */
	EXT3_FEATURE_COMPAT_HAS_JOURNAL |
		EXT2_FEATURE_COMPAT_RESIZE_INODE |
		EXT2_FEATURE_COMPAT_DIR_INDEX,
	/* Incompat */
	EXT2_FEATURE_INCOMPAT_FILETYPE |
		EXT4_FEATURE_INCOMPAT_FLEX_BG |
		EXT4_FEATURE_INCOMPAT_MMP |
		EXT4_FEATURE_INCOMPAT_64BIT |
		EXT4_FEATURE_INCOMPAT_CSUM_SEED,
	/* R/O compat */
	EXT2_FEATURE_RO_COMPAT_LARGE_FILE |
		EXT4_FEATURE_RO_COMPAT_HUGE_FILE|
		EXT4_FEATURE_RO_COMPAT_DIR_NLINK|
		EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE|
		EXT4_FEATURE_RO_COMPAT_GDT_CSUM |
		EXT4_FEATURE_RO_COMPAT_QUOTA |
		EXT4_FEATURE_RO_COMPAT_PROJECT |
		EXT4_FEATURE_RO_COMPAT_METADATA_CSUM |
		EXT4_FEATURE_RO_COMPAT_READONLY
};

/**
 * Try to get journal super block if any
 */
static int get_journal_sb(ext2_filsys jfs, char buf[SUPERBLOCK_SIZE])
{
	int retval;
	journal_superblock_t *jsb;

	if (!ext2fs_has_feature_journal_dev(jfs->super)) {
		return EXT2_ET_UNSUPP_FEATURE;
	}

	/* Get the journal superblock */
	if ((retval = io_channel_read_blk64(jfs->io,
	    ext2fs_journal_sb_start(jfs->blocksize), -SUPERBLOCK_SIZE, buf))) {
		com_err(program_name, retval, "%s",
		_("while reading journal superblock"));
		return retval;
	}

	jsb = (journal_superblock_t *) buf;
	if ((jsb->s_header.h_magic != (unsigned)ntohl(JFS_MAGIC_NUMBER)) ||
	    (jsb->s_header.h_blocktype != (unsigned)ntohl(JFS_SUPERBLOCK_V2))) {
		fputs(_("Journal superblock not found!\n"), stderr);
		return EXT2_ET_BAD_MAGIC;
	}

	return 0;
}

static __u8 *journal_user(__u8 uuid[UUID_SIZE], __u8 s_users[JFS_USERS_SIZE],
			  int nr_users)
{
	int i;
	for (i = 0; i < nr_users; i++) {
		if (memcmp(uuid, &s_users[i * UUID_SIZE], UUID_SIZE) == 0)
			return &s_users[i * UUID_SIZE];
	}

	return NULL;
}

/*
 * Remove an external journal from the filesystem
 */
static int remove_journal_device(ext2_filsys fs)
{
	char		*journal_path;
	ext2_filsys	jfs;
	char		buf[SUPERBLOCK_SIZE] __attribute__ ((aligned(8)));
	journal_superblock_t	*jsb;
	int		i, nr_users;
	errcode_t	retval;
	int		commit_remove_journal = 0;
	io_manager	io_ptr;

	if (f_flag)
		commit_remove_journal = 1; /* force removal even if error */

	uuid_unparse(fs->super->s_journal_uuid, buf);
	journal_path = blkid_get_devname(NULL, "UUID", buf);

	if (!journal_path) {
		journal_path =
			ext2fs_find_block_device(fs->super->s_journal_dev);
		if (!journal_path)
			goto no_valid_journal;
	}

#ifdef CONFIG_TESTIO_DEBUG
	if (getenv("TEST_IO_FLAGS") || getenv("TEST_IO_BLOCK")) {
		io_ptr = test_io_manager;
		test_io_backing_manager = unix_io_manager;
	} else
#endif
		io_ptr = unix_io_manager;
	retval = ext2fs_open(journal_path, EXT2_FLAG_RW|
			     EXT2_FLAG_JOURNAL_DEV_OK, 0,
			     fs->blocksize, io_ptr, &jfs);
	if (retval) {
		com_err(program_name, retval, "%s",
			_("while trying to open external journal"));
		goto no_valid_journal;
	}

	if ((retval = get_journal_sb(jfs, buf))) {
		if (retval == EXT2_ET_UNSUPP_FEATURE)
			fprintf(stderr, _("%s is not a journal device.\n"),
				journal_path);
		goto no_valid_journal;
	}

	jsb = (journal_superblock_t *) buf;
	/* Find the filesystem UUID */
	nr_users = ntohl(jsb->s_nr_users);
	if (nr_users > JFS_USERS_MAX) {
		fprintf(stderr, _("Journal superblock is corrupted, nr_users\n"
				 "is too high (%d).\n"), nr_users);
		commit_remove_journal = 1;
		goto no_valid_journal;
	}

	if (!journal_user(fs->super->s_uuid, jsb->s_users, nr_users)) {
		fputs(_("Filesystem's UUID not found on journal device.\n"),
		      stderr);
		commit_remove_journal = 1;
		goto no_valid_journal;
	}
	nr_users--;
	for (i = 0; i < nr_users; i++)
		memcpy(&jsb->s_users[i * 16], &jsb->s_users[(i + 1) * 16], 16);
	jsb->s_nr_users = htonl(nr_users);

	/* Write back the journal superblock */
	retval = io_channel_write_blk64(jfs->io,
					ext2fs_journal_sb_start(fs->blocksize),
					-SUPERBLOCK_SIZE, buf);
	if (retval) {
		com_err(program_name, retval,
			"while writing journal superblock.");
		goto no_valid_journal;
	}

	commit_remove_journal = 1;

no_valid_journal:
	if (commit_remove_journal == 0) {
		fputs(_("Cannot locate journal device. It was NOT removed\n"
			"Use -f option to remove missing journal device.\n"),
		      stderr);
		return 1;
	}
	fs->super->s_journal_dev = 0;
	memset(fs->super->s_jnl_blocks, 0, sizeof(fs->super->s_jnl_blocks));
	uuid_clear(fs->super->s_journal_uuid);
	ext2fs_mark_super_dirty(fs);
	fputs(_("Journal removed\n"), stdout);
	free(journal_path);

	return 0;
}

/* Helper function for remove_journal_inode */
static int release_blocks_proc(ext2_filsys fs, blk64_t *blocknr,
			       e2_blkcnt_t blockcnt EXT2FS_ATTR((unused)),
			       blk64_t ref_block EXT2FS_ATTR((unused)),
			       int ref_offset EXT2FS_ATTR((unused)),
			       void *private EXT2FS_ATTR((unused)))
{
	blk64_t	block;
	int	group;

	block = *blocknr;
	ext2fs_unmark_block_bitmap2(fs->block_map, block);
	group = ext2fs_group_of_blk2(fs, block);
	ext2fs_bg_free_blocks_count_set(fs, group, ext2fs_bg_free_blocks_count(fs, group) + 1);
	ext2fs_group_desc_csum_set(fs, group);
	ext2fs_free_blocks_count_add(fs->super, EXT2FS_CLUSTER_RATIO(fs));
	return 0;
}

/*
 * Remove the journal inode from the filesystem
 */
static errcode_t remove_journal_inode(ext2_filsys fs)
{
	struct ext2_inode	inode;
	errcode_t		retval;
	ext2_ino_t		ino = fs->super->s_journal_inum;

	retval = ext2fs_read_inode(fs, ino,  &inode);
	if (retval) {
		com_err(program_name, retval, "%s",
			_("while reading journal inode"));
		return retval;
	}
	if (ino == EXT2_JOURNAL_INO) {
		retval = ext2fs_read_bitmaps(fs);
		if (retval) {
			com_err(program_name, retval, "%s",
				_("while reading bitmaps"));
			return retval;
		}
		retval = ext2fs_block_iterate3(fs, ino,
					       BLOCK_FLAG_READ_ONLY, NULL,
					       release_blocks_proc, NULL);
		if (retval) {
			com_err(program_name, retval, "%s",
				_("while clearing journal inode"));
			return retval;
		}
		memset(&inode, 0, sizeof(inode));
		ext2fs_mark_bb_dirty(fs);
		fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	} else
		inode.i_flags &= ~EXT2_IMMUTABLE_FL;
	retval = ext2fs_write_inode(fs, ino, &inode);
	if (retval) {
		com_err(program_name, retval, "%s",
			_("while writing journal inode"));
		return retval;
	}
	fs->super->s_journal_inum = 0;
	memset(fs->super->s_jnl_blocks, 0, sizeof(fs->super->s_jnl_blocks));
	ext2fs_mark_super_dirty(fs);

	return 0;
}

/*
 * Update the default mount options
 */
static int update_mntopts(ext2_filsys fs, char *mntopts)
{
	struct ext2_super_block *sb = fs->super;

	if (e2p_edit_mntopts(mntopts, &sb->s_default_mount_opts, ~0)) {
		fprintf(stderr, _("Invalid mount option set: %s\n"),
			mntopts);
		return 1;
	}
	ext2fs_mark_super_dirty(fs);

	return 0;
}

static void check_fsck_needed(ext2_filsys fs, const char *prompt)
{
	/* Refuse to modify anything but a freshly checked valid filesystem. */
	if (!(fs->super->s_state & EXT2_VALID_FS) ||
	    (fs->super->s_state & EXT2_ERROR_FS) ||
	    (fs->super->s_lastcheck < fs->super->s_mtime)) {
		puts(_(fsck_explain));
		puts(_(please_fsck));
		if (mount_flags & EXT2_MF_READONLY)
			printf("%s", _("(and reboot afterwards!)\n"));
		exit(1);
	}

	/* Give the admin a few seconds to bail out of a dangerous op. */
	if (!getenv("TUNE2FS_FORCE_PROMPT") && (!isatty(0) || !isatty(1)))
		return;

	puts(prompt);
	proceed_question(5);
}

static void request_dir_fsck_afterwards(ext2_filsys fs)
{
	static int requested;

	if (requested++)
		return;
	fsck_requested++;
	fs->super->s_state &= ~EXT2_VALID_FS;
	puts(_(fsck_explain));
	puts(_(please_dir_fsck));
	if (mount_flags & EXT2_MF_READONLY)
		printf("%s", _("(and reboot afterwards!)\n"));
}

static void request_fsck_afterwards(ext2_filsys fs)
{
	static int requested = 0;

	if (requested++)
		return;
	fsck_requested++;
	fs->super->s_state &= ~EXT2_VALID_FS;
	printf("\n%s\n", _(please_fsck));
	if (mount_flags & EXT2_MF_READONLY)
		printf("%s", _("(and reboot afterwards!)\n"));
}

static void convert_64bit(ext2_filsys fs, int direction)
{
	/*
	 * Is resize2fs going to demand a fsck run? Might as well tell the
	 * user now.
	 */
	if (!fsck_requested &&
	    ((fs->super->s_state & EXT2_ERROR_FS) ||
	     !(fs->super->s_state & EXT2_VALID_FS) ||
	     fs->super->s_lastcheck < fs->super->s_mtime))
		request_fsck_afterwards(fs);
	if (fsck_requested)
		fprintf(stderr, _("After running e2fsck, please run `resize2fs %s %s"),
			direction > 0 ? "-b" : "-s", fs->device_name);
	else
		fprintf(stderr, _("Please run `resize2fs %s %s"),
			direction > 0 ? "-b" : "-s", fs->device_name);

	if (undo_file)
		fprintf(stderr, _(" -z \"%s\""), undo_file);
	if (direction > 0)
		fprintf(stderr, _("' to enable 64-bit mode.\n"));
	else
		fprintf(stderr, _("' to disable 64-bit mode.\n"));
}

/*
 * Rewrite directory blocks with checksums
 */
struct rewrite_dir_context {
	char *buf;
	errcode_t errcode;
	ext2_ino_t dir;
	int is_htree;
};

static int rewrite_dir_block(ext2_filsys fs,
			     blk64_t	*blocknr,
			     e2_blkcnt_t blockcnt EXT2FS_ATTR((unused)),
			     blk64_t	ref_block EXT2FS_ATTR((unused)),
			     int	ref_offset EXT2FS_ATTR((unused)),
			     void	*priv_data)
{
	struct ext2_dx_countlimit *dcl = NULL;
	struct rewrite_dir_context *ctx = priv_data;
	int dcl_offset, changed = 0;

	ctx->errcode = ext2fs_read_dir_block4(fs, *blocknr, ctx->buf, 0,
					      ctx->dir);
	if (ctx->errcode)
		return BLOCK_ABORT;

	/* if htree node... */
	if (ctx->is_htree)
		ext2fs_get_dx_countlimit(fs, (struct ext2_dir_entry *)ctx->buf,
					 &dcl, &dcl_offset);
	if (dcl) {
		if (!ext2fs_has_feature_metadata_csum(fs->super)) {
			/* Ensure limit is the max size */
			int max_entries = (fs->blocksize - dcl_offset) /
					  sizeof(struct ext2_dx_entry);
			if (ext2fs_le16_to_cpu(dcl->limit) != max_entries) {
				changed = 1;
				dcl->limit = ext2fs_cpu_to_le16(max_entries);
			}
		} else {
			/* If htree block is full then rebuild the dir */
			if (ext2fs_le16_to_cpu(dcl->count) ==
			    ext2fs_le16_to_cpu(dcl->limit)) {
				request_dir_fsck_afterwards(fs);
				return 0;
			}
			/*
			 * Ensure dcl->limit is small enough to leave room for
			 * the checksum tail.
			 */
			int max_entries = (fs->blocksize - (dcl_offset +
						sizeof(struct ext2_dx_tail))) /
					  sizeof(struct ext2_dx_entry);
			if (ext2fs_le16_to_cpu(dcl->limit) != max_entries)
				dcl->limit = ext2fs_cpu_to_le16(max_entries);
			/* Always rewrite checksum */
			changed = 1;
		}
	} else {
		unsigned int rec_len, name_size;
		char *top = ctx->buf + fs->blocksize;
		struct ext2_dir_entry *de = (struct ext2_dir_entry *)ctx->buf;
		struct ext2_dir_entry *last_de = NULL, *penultimate_de = NULL;

		/* Find last and penultimate dirent */
		while ((char *)de < top) {
			penultimate_de = last_de;
			last_de = de;
			ctx->errcode = ext2fs_get_rec_len(fs, de, &rec_len);
			if (!ctx->errcode && !rec_len)
				ctx->errcode = EXT2_ET_DIR_CORRUPTED;
			if (ctx->errcode)
				return BLOCK_ABORT;
			de = (struct ext2_dir_entry *)(((char *)de) + rec_len);
		}
		ctx->errcode = ext2fs_get_rec_len(fs, last_de, &rec_len);
		if (ctx->errcode)
			return BLOCK_ABORT;
		name_size = ext2fs_dirent_name_len(last_de);

		if (!ext2fs_has_feature_metadata_csum(fs->super)) {
			if (!penultimate_de)
				return 0;
			if (last_de->inode ||
			    name_size ||
			    rec_len != sizeof(struct ext2_dir_entry_tail))
				return 0;
			/*
			 * The last dirent is unused and the right length to
			 * have stored a checksum.  Erase it.
			 */
			ctx->errcode = ext2fs_get_rec_len(fs, penultimate_de,
							  &rec_len);
			if (!rec_len)
				ctx->errcode = EXT2_ET_DIR_CORRUPTED;
			if (ctx->errcode)
				return BLOCK_ABORT;
			ext2fs_set_rec_len(fs, rec_len +
					sizeof(struct ext2_dir_entry_tail),
					penultimate_de);
			changed = 1;
		} else {
			unsigned csum_size = sizeof(struct ext2_dir_entry_tail);
			struct ext2_dir_entry_tail *t;

			/*
			 * If the last dirent looks like the tail, just update
			 * the checksum.
			 */
			if (!last_de->inode &&
			    rec_len == csum_size) {
				t = (struct ext2_dir_entry_tail *)last_de;
				t->det_reserved_name_len =
						EXT2_DIR_NAME_LEN_CSUM;
				changed = 1;
				goto out;
			}
			if (name_size & 3)
				name_size = (name_size & ~3) + 4;
			/* If there's not enough space for the tail, e2fsck */
			if (rec_len <= (8 + name_size + csum_size)) {
				request_dir_fsck_afterwards(fs);
				return 0;
			}
			/* Shorten that last de and insert the tail */
			ext2fs_set_rec_len(fs, rec_len - csum_size, last_de);
			t = EXT2_DIRENT_TAIL(ctx->buf, fs->blocksize);
			ext2fs_initialize_dirent_tail(fs, t);

			/* Always update checksum */
			changed = 1;
		}
	}

out:
	if (!changed)
		return 0;

	ctx->errcode = ext2fs_write_dir_block4(fs, *blocknr, ctx->buf,
					       0, ctx->dir);
	if (ctx->errcode)
		return BLOCK_ABORT;

	return 0;
}

static errcode_t rewrite_directory(ext2_filsys fs, ext2_ino_t dir,
				   struct ext2_inode *inode)
{
	errcode_t	retval;
	struct rewrite_dir_context ctx;

	retval = ext2fs_get_mem(fs->blocksize, &ctx.buf);
	if (retval)
		return retval;

	ctx.is_htree = (inode->i_flags & EXT2_INDEX_FL);
	ctx.dir = dir;
	ctx.errcode = 0;
	retval = ext2fs_block_iterate3(fs, dir, BLOCK_FLAG_READ_ONLY |
						BLOCK_FLAG_DATA_ONLY,
				       0, rewrite_dir_block, &ctx);

	ext2fs_free_mem(&ctx.buf);
	if (retval)
		return retval;

	return ctx.errcode;
}

/*
 * Context information that does not change across rewrite_one_inode()
 * invocations.
 */
struct rewrite_context {
	ext2_filsys fs;
	struct ext2_inode *zero_inode;
	char *ea_buf;
	int inode_size;
};

#define fatal_err(code, args...)		\
	do {					\
		com_err(__func__, code, args);	\
		exit(1);			\
	} while (0);

static void update_ea_inode_hash(struct rewrite_context *ctx, ext2_ino_t ino,
				 struct ext2_inode *inode)
{
	errcode_t retval;
	ext2_file_t file;
	__u32 hash;

	retval = ext2fs_file_open(ctx->fs, ino, 0, &file);
	if (retval)
		fatal_err(retval, "open ea_inode");
	retval = ext2fs_file_read(file, ctx->ea_buf, inode->i_size,
				  NULL);
	if (retval)
		fatal_err(retval, "read ea_inode");
	retval = ext2fs_file_close(file);
	if (retval)
		fatal_err(retval, "close ea_inode");

	hash = ext2fs_crc32c_le(ctx->fs->csum_seed,
				(unsigned char *) ctx->ea_buf, inode->i_size);
	ext2fs_set_ea_inode_hash(inode, hash);
}

static int update_xattr_entry_hashes(ext2_filsys fs,
				     struct ext2_ext_attr_entry *entry,
				     struct ext2_ext_attr_entry *end)
{
	int modified = 0;
	errcode_t retval;

	while (entry < end && !EXT2_EXT_IS_LAST_ENTRY(entry)) {
		if (entry->e_value_inum) {
			retval = ext2fs_ext_attr_hash_entry2(fs, entry, NULL,
							     &entry->e_hash);
			if (retval)
				fatal_err(retval, "hash ea_inode entry");
			modified = 1;
		}
		entry = EXT2_EXT_ATTR_NEXT(entry);
	}
	return modified;
}

static void update_inline_xattr_hashes(struct rewrite_context *ctx,
				       struct ext2_inode_large *inode)
{
	struct ext2_ext_attr_entry *start, *end;
	__u32 *ea_magic;

	if (inode->i_extra_isize == 0)
		return;

	if (inode->i_extra_isize & 3 ||
	    inode->i_extra_isize > ctx->inode_size - EXT2_GOOD_OLD_INODE_SIZE)
		fatal_err(EXT2_ET_INODE_CORRUPTED, "bad i_extra_isize")

	ea_magic = (__u32 *)((char *)inode + EXT2_GOOD_OLD_INODE_SIZE +
				inode->i_extra_isize);
	if (*ea_magic != EXT2_EXT_ATTR_MAGIC)
		return;

	start = (struct ext2_ext_attr_entry *)(ea_magic + 1);
	end = (struct ext2_ext_attr_entry *)((char *)inode + ctx->inode_size);

	update_xattr_entry_hashes(ctx->fs, start, end);
}

static void update_block_xattr_hashes(struct rewrite_context *ctx,
				      char *block_buf)
{
	struct ext2_ext_attr_header *header;
	struct ext2_ext_attr_entry *start, *end;

	header = (struct ext2_ext_attr_header *)block_buf;
	if (header->h_magic != EXT2_EXT_ATTR_MAGIC)
		return;

	start = (struct ext2_ext_attr_entry *)(header+1);
	end = (struct ext2_ext_attr_entry *)(block_buf + ctx->fs->blocksize);

	if (update_xattr_entry_hashes(ctx->fs, start, end))
		ext2fs_ext_attr_block_rehash(header, end);
}

static void rewrite_one_inode(struct rewrite_context *ctx, ext2_ino_t ino,
			      struct ext2_inode *inode)
{
	blk64_t file_acl_block;
	errcode_t retval;

	if (!ext2fs_test_inode_bitmap2(ctx->fs->inode_map, ino)) {
		if (!memcmp(inode, ctx->zero_inode, ctx->inode_size))
			return;
		memset(inode, 0, ctx->inode_size);
	}

	if (inode->i_flags & EXT4_EA_INODE_FL)
		update_ea_inode_hash(ctx, ino, inode);

	if (ctx->inode_size != EXT2_GOOD_OLD_INODE_SIZE)
		update_inline_xattr_hashes(ctx,
					   (struct ext2_inode_large *)inode);

	retval = ext2fs_write_inode_full(ctx->fs, ino, inode, ctx->inode_size);
	if (retval)
		fatal_err(retval, "while writing inode");

	retval = ext2fs_fix_extents_checksums(ctx->fs, ino, inode);
	if (retval)
		fatal_err(retval, "while rewriting extents");

	if (LINUX_S_ISDIR(inode->i_mode) &&
	    ext2fs_inode_has_valid_blocks2(ctx->fs, inode)) {
		retval = rewrite_directory(ctx->fs, ino, inode);
		if (retval)
			fatal_err(retval, "while rewriting directories");
	}

	file_acl_block = ext2fs_file_acl_block(ctx->fs, inode);
	if (!file_acl_block)
		return;

	retval = ext2fs_read_ext_attr3(ctx->fs, file_acl_block, ctx->ea_buf,
				       ino);
	if (retval)
		fatal_err(retval, "while rewriting extended attribute");

	update_block_xattr_hashes(ctx, ctx->ea_buf);
	retval = ext2fs_write_ext_attr3(ctx->fs, file_acl_block, ctx->ea_buf,
					ino);
	if (retval)
		fatal_err(retval, "while rewriting extended attribute");
}

/*
 * Forcibly set checksums in all inodes.
 */
static void rewrite_inodes(ext2_filsys fs)
{
	ext2_inode_scan	scan;
	errcode_t	retval;
	ext2_ino_t	ino;
	struct ext2_inode *inode;
	int pass;
	struct rewrite_context ctx = {
		.fs = fs,
		.inode_size = EXT2_INODE_SIZE(fs->super),
	};

	if (fs->super->s_creator_os == EXT2_OS_HURD)
		return;

	retval = ext2fs_get_mem(ctx.inode_size, &inode);
	if (retval)
		fatal_err(retval, "while allocating memory");

	retval = ext2fs_get_memzero(ctx.inode_size, &ctx.zero_inode);
	if (retval)
		fatal_err(retval, "while allocating memory");

	retval = ext2fs_get_mem(64 * 1024, &ctx.ea_buf);
	if (retval)
		fatal_err(retval, "while allocating memory");

	/*
	 * Extended attribute inodes have a lookup hash that needs to be
	 * recalculated with the new csum_seed. Other inodes referencing xattr
	 * inodes need this value to be up to date. That's why we do two passes:
	 *
	 * pass 1: update xattr inodes to update their lookup hash as well as
	 *         other checksums.
	 *
	 * pass 2: go over other inodes to update their checksums.
	 */
	if (ext2fs_has_feature_ea_inode(fs->super))
		pass = 1;
	else
		pass = 2;
	for (;pass <= 2; pass++) {
		retval = ext2fs_open_inode_scan(fs, 0, &scan);
		if (retval)
			fatal_err(retval, "while opening inode scan");

		do {
			retval = ext2fs_get_next_inode_full(scan, &ino, inode,
							    ctx.inode_size);
			if (retval)
				fatal_err(retval, "while getting next inode");
			if (!ino)
				break;

			if (((pass == 1) &&
			     (inode->i_flags & EXT4_EA_INODE_FL)) ||
			    ((pass == 2) &&
			     !(inode->i_flags & EXT4_EA_INODE_FL)))
				rewrite_one_inode(&ctx, ino, inode);
		} while (ino);

		ext2fs_close_inode_scan(scan);
	}

	ext2fs_free_mem(&ctx.zero_inode);
	ext2fs_free_mem(&ctx.ea_buf);
	ext2fs_free_mem(&inode);
}

static void rewrite_metadata_checksums(ext2_filsys fs)
{
	errcode_t retval;
	dgrp_t i;

	fs->flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
	ext2fs_init_csum_seed(fs);
	for (i = 0; i < fs->group_desc_count; i++)
		ext2fs_group_desc_csum_set(fs, i);
	retval = ext2fs_read_bitmaps(fs);
	if (retval)
		fatal_err(retval, "while reading bitmaps");
	rewrite_inodes(fs);
	ext2fs_mark_ib_dirty(fs);
	ext2fs_mark_bb_dirty(fs);
	ext2fs_mmp_update2(fs, 1);
	fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	fs->flags &= ~EXT2_FLAG_IGNORE_CSUM_ERRORS;
	if (ext2fs_has_feature_metadata_csum(fs->super))
		fs->super->s_checksum_type = EXT2_CRC32C_CHKSUM;
	else
		fs->super->s_checksum_type = 0;
	ext2fs_mark_super_dirty(fs);
}

static void enable_uninit_bg(ext2_filsys fs)
{
	struct ext2_group_desc *gd;
	dgrp_t i;

	for (i = 0; i < fs->group_desc_count; i++) {
		gd = ext2fs_group_desc(fs, fs->group_desc, i);
		gd->bg_itable_unused = 0;
		gd->bg_flags = EXT2_BG_INODE_ZEROED;
		ext2fs_group_desc_csum_set(fs, i);
	}
	fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
}

static errcode_t zero_empty_inodes(ext2_filsys fs)
{
	int length = EXT2_INODE_SIZE(fs->super);
	struct ext2_inode *inode = NULL;
	ext2_inode_scan	scan;
	errcode_t	retval;
	ext2_ino_t	ino;

	retval = ext2fs_open_inode_scan(fs, 0, &scan);
	if (retval)
		goto out;

	retval = ext2fs_get_mem(length, &inode);
	if (retval)
		goto out;

	do {
		retval = ext2fs_get_next_inode_full(scan, &ino, inode, length);
		if (retval)
			goto out;
		if (!ino)
			break;
		if (!ext2fs_test_inode_bitmap2(fs->inode_map, ino)) {
			memset(inode, 0, length);
			retval = ext2fs_write_inode_full(fs, ino, inode,
							 length);
			if (retval)
				goto out;
		}
	} while (1);

out:
	ext2fs_free_mem(&inode);
	ext2fs_close_inode_scan(scan);
	return retval;
}

static errcode_t disable_uninit_bg(ext2_filsys fs, __u32 csum_feature_flag)
{
	struct ext2_group_desc *gd;
	dgrp_t i;
	errcode_t retval;
	blk64_t b, c, d;

	/* Load bitmaps to ensure that the uninit ones get written out */
	fs->super->s_feature_ro_compat |= csum_feature_flag;
	retval = ext2fs_read_bitmaps(fs);
	fs->super->s_feature_ro_compat &= ~csum_feature_flag;
	if (retval) {
		com_err("disable_uninit_bg", retval,
			"while reading bitmaps");
		request_fsck_afterwards(fs);
		return retval;
	}
	ext2fs_mark_ib_dirty(fs);
	ext2fs_mark_bb_dirty(fs);

	/* If we're only turning off uninit_bg, zero the inodes */
	if (csum_feature_flag == EXT4_FEATURE_RO_COMPAT_GDT_CSUM) {
		retval = zero_empty_inodes(fs);
		if (retval) {
			com_err("disable_uninit_bg", retval,
				"while zeroing unused inodes");
			request_fsck_afterwards(fs);
			return retval;
		}
	}

	/* The bbitmap is zeroed; we must mark group metadata blocks in use */
	for (i = 0; i < fs->group_desc_count; i++) {
		b = ext2fs_block_bitmap_loc(fs, i);
		ext2fs_mark_block_bitmap2(fs->block_map, b);
		b = ext2fs_inode_bitmap_loc(fs, i);
		ext2fs_mark_block_bitmap2(fs->block_map, b);

		retval = ext2fs_super_and_bgd_loc2(fs, i, &b, &c, &d, NULL);
		if (retval == 0 && b)
			ext2fs_mark_block_bitmap2(fs->block_map, b);
		if (retval == 0 && c)
			ext2fs_mark_block_bitmap2(fs->block_map, c);
		if (retval == 0 && d)
			ext2fs_mark_block_bitmap2(fs->block_map, d);
		if (retval) {
			com_err("disable_uninit_bg", retval,
				"while initializing block bitmaps");
			request_fsck_afterwards(fs);
		}

		gd = ext2fs_group_desc(fs, fs->group_desc, i);
		gd->bg_itable_unused = 0;
		gd->bg_flags = 0;
		ext2fs_group_desc_csum_set(fs, i);
	}
	fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	ext2fs_mark_super_dirty(fs);

	return 0;
}

static void
try_confirm_csum_seed_support(void)
{
	if (access("/sys/fs/ext4/features/metadata_csum_seed", R_OK))
		fputs(_("WARNING: Could not confirm kernel support for "
			"metadata_csum_seed.\n  This requires Linux >= "
			"v4.4.\n"), stderr);
}

/*
 * Update the feature set as provided by the user.
 */
static int update_feature_set(ext2_filsys fs, char *features)
{
	struct ext2_super_block *sb = fs->super;
	__u32		old_features[3];
	int		type_err;
	unsigned int	mask_err;
	errcode_t	err;
	enum quota_type qtype;

#define FEATURE_ON(type, mask) (!(old_features[(type)] & (mask)) && \
				((&sb->s_feature_compat)[(type)] & (mask)))
#define FEATURE_OFF(type, mask) ((old_features[(type)] & (mask)) && \
				 !((&sb->s_feature_compat)[(type)] & (mask)))
#define FEATURE_CHANGED(type, mask) ((mask) & \
		     (old_features[(type)] ^ (&sb->s_feature_compat)[(type)]))

	old_features[E2P_FEATURE_COMPAT] = sb->s_feature_compat;
	old_features[E2P_FEATURE_INCOMPAT] = sb->s_feature_incompat;
	old_features[E2P_FEATURE_RO_INCOMPAT] = sb->s_feature_ro_compat;

	if (e2p_edit_feature2(features, &sb->s_feature_compat,
			      ok_features, clear_ok_features,
			      &type_err, &mask_err)) {
		if (!mask_err)
			fprintf(stderr,
				_("Invalid filesystem option set: %s\n"),
				features);
		else if (type_err & E2P_FEATURE_NEGATE_FLAG)
			fprintf(stderr, _("Clearing filesystem feature '%s' "
					  "not supported.\n"),
				e2p_feature2string(type_err &
						   E2P_FEATURE_TYPE_MASK,
						   mask_err));
		else
			fprintf(stderr, _("Setting filesystem feature '%s' "
					  "not supported.\n"),
				e2p_feature2string(type_err, mask_err));
		return 1;
	}

	if (FEATURE_OFF(E2P_FEATURE_COMPAT, EXT3_FEATURE_COMPAT_HAS_JOURNAL)) {
		if ((mount_flags & EXT2_MF_MOUNTED) &&
		    !(mount_flags & EXT2_MF_READONLY)) {
			fputs(_("The has_journal feature may only be "
				"cleared when the filesystem is\n"
				"unmounted or mounted "
				"read-only.\n"), stderr);
			return 1;
		}
		if (ext2fs_has_feature_journal_needs_recovery(sb) &&
		    f_flag < 2) {
			fputs(_("The needs_recovery flag is set.  "
				"Please run e2fsck before clearing\n"
				"the has_journal flag.\n"), stderr);
			return 1;
		}
		if (sb->s_journal_inum) {
			if (remove_journal_inode(fs))
				return 1;
		}
		if (sb->s_journal_dev) {
			if (remove_journal_device(fs))
				return 1;
		}
	}

	if (FEATURE_ON(E2P_FEATURE_RO_INCOMPAT,
		EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER)) {
		if (ext2fs_has_feature_meta_bg(sb)) {
			fputs(_("Setting filesystem feature 'sparse_super' "
				"not supported\nfor filesystems with "
				"the meta_bg feature enabled.\n"),
				stderr);
			return 1;
		}
	}

	if (FEATURE_ON(E2P_FEATURE_INCOMPAT, EXT4_FEATURE_INCOMPAT_MMP)) {
		int error;

		if ((mount_flags & EXT2_MF_MOUNTED) ||
		    (mount_flags & EXT2_MF_READONLY)) {
			fputs(_("The multiple mount protection feature can't\n"
				"be set if the filesystem is mounted or\n"
				"read-only.\n"), stderr);
			return 1;
		}

		error = ext2fs_mmp_init(fs);
		if (error) {
			fputs(_("\nError while enabling multiple mount "
				"protection feature."), stderr);
			return 1;
		}

		/*
		 * We want to update group desc with the new free blocks count
		 */
		fs->flags &= ~EXT2_FLAG_SUPER_ONLY;

		printf(_("Multiple mount protection has been enabled "
			 "with update interval %ds.\n"),
		       sb->s_mmp_update_interval);
	}

	if (FEATURE_OFF(E2P_FEATURE_INCOMPAT, EXT4_FEATURE_INCOMPAT_MMP)) {
		int error;

		if (mount_flags & EXT2_MF_READONLY) {
			fputs(_("The multiple mount protection feature cannot\n"
				"be disabled if the filesystem is readonly.\n"),
				stderr);
			return 1;
		}

		error = ext2fs_read_bitmaps(fs);
		if (error) {
			fputs(_("Error while reading bitmaps\n"), stderr);
			return 1;
		}

		error = ext2fs_mmp_read(fs, sb->s_mmp_block, NULL);
		if (error) {
			struct mmp_struct *mmp_cmp = fs->mmp_cmp;

			if (error == EXT2_ET_MMP_MAGIC_INVALID)
				printf(_("Magic number in MMP block does not "
					 "match. expected: %x, actual: %x\n"),
					 EXT4_MMP_MAGIC, mmp_cmp->mmp_magic);
			else
				com_err(program_name, error, "%s",
					_("while reading MMP block."));
			goto mmp_error;
		}

		/* We need to force out the group descriptors as well */
		fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
		ext2fs_block_alloc_stats2(fs, sb->s_mmp_block, -1);
mmp_error:
		sb->s_mmp_block = 0;
		sb->s_mmp_update_interval = 0;
	}

	if (FEATURE_ON(E2P_FEATURE_COMPAT, EXT3_FEATURE_COMPAT_HAS_JOURNAL)) {
		/*
		 * If adding a journal flag, let the create journal
		 * code below handle setting the flag and creating the
		 * journal.  We supply a default size if necessary.
		 */
		if (!journal_size)
			journal_size = -1;
		ext2fs_clear_feature_journal(sb);
	}

	if (FEATURE_ON(E2P_FEATURE_COMPAT, EXT2_FEATURE_COMPAT_DIR_INDEX)) {
		if (!sb->s_def_hash_version)
			sb->s_def_hash_version = EXT2_HASH_HALF_MD4;
		if (uuid_is_null((unsigned char *) sb->s_hash_seed))
			uuid_generate((unsigned char *) sb->s_hash_seed);
	}

	if (FEATURE_OFF(E2P_FEATURE_INCOMPAT, EXT4_FEATURE_INCOMPAT_FLEX_BG)) {
		if (ext2fs_check_desc(fs)) {
			fputs(_("Clearing the flex_bg flag would "
				"cause the the filesystem to be\n"
				"inconsistent.\n"), stderr);
			return 1;
		}
	}

	if (FEATURE_OFF(E2P_FEATURE_RO_INCOMPAT,
			    EXT4_FEATURE_RO_COMPAT_HUGE_FILE)) {
		if ((mount_flags & EXT2_MF_MOUNTED) &&
		    !(mount_flags & EXT2_MF_READONLY)) {
			fputs(_("The huge_file feature may only be "
				"cleared when the filesystem is\n"
				"unmounted or mounted "
				"read-only.\n"), stderr);
			return 1;
		}
	}

	if (FEATURE_ON(E2P_FEATURE_RO_INCOMPAT,
		       EXT4_FEATURE_RO_COMPAT_METADATA_CSUM)) {
		check_fsck_needed(fs,
			_("Enabling checksums could take some time."));
		if (mount_flags & EXT2_MF_MOUNTED) {
			fputs(_("Cannot enable metadata_csum on a mounted "
				"filesystem!\n"), stderr);
			exit(1);
		}
		if (!ext2fs_has_feature_extents(fs->super))
			printf("%s",
			       _("Extents are not enabled.  The file extent "
				 "tree can be checksummed, whereas block maps "
				 "cannot.  Not enabling extents reduces the "
				 "coverage of metadata checksumming.  "
				 "Re-run with -O extent to rectify.\n"));
		if (!ext2fs_has_feature_64bit(fs->super))
			printf("%s",
			       _("64-bit filesystem support is not enabled.  "
				 "The larger fields afforded by this feature "
				 "enable full-strength checksumming.  "
				 "Run resize2fs -b to rectify.\n"));
		rewrite_checksums = 1;
		/* metadata_csum supersedes uninit_bg */
		ext2fs_clear_feature_gdt_csum(fs->super);

		/* if uninit_bg was previously off, rewrite group desc */
		if (!(old_features[E2P_FEATURE_RO_INCOMPAT] &
		      EXT4_FEATURE_RO_COMPAT_GDT_CSUM))
			enable_uninit_bg(fs);

		/*
		 * Since metadata_csum supersedes uninit_bg, pretend like
		 * uninit_bg has been off all along.
		 */
		old_features[E2P_FEATURE_RO_INCOMPAT] &=
			~EXT4_FEATURE_RO_COMPAT_GDT_CSUM;
	}

	if (FEATURE_OFF(E2P_FEATURE_RO_INCOMPAT,
			EXT4_FEATURE_RO_COMPAT_METADATA_CSUM)) {
		__u32	test_features[3];

		check_fsck_needed(fs,
			_("Disabling checksums could take some time."));
		if (mount_flags & EXT2_MF_MOUNTED) {
			fputs(_("Cannot disable metadata_csum on a mounted "
				"filesystem!\n"), stderr);
			exit(1);
		}
		rewrite_checksums = 1;

		/* Enable uninit_bg unless the user expressly turned it off */
		memcpy(test_features, old_features, sizeof(test_features));
		test_features[E2P_FEATURE_RO_INCOMPAT] |=
						EXT4_FEATURE_RO_COMPAT_GDT_CSUM;
		e2p_edit_feature2(features, test_features, ok_features,
				  clear_ok_features, NULL, NULL);
		if (test_features[E2P_FEATURE_RO_INCOMPAT] &
						EXT4_FEATURE_RO_COMPAT_GDT_CSUM)
			ext2fs_set_feature_gdt_csum(fs->super);

		/*
		 * If we're turning off metadata_csum and not turning on
		 * uninit_bg, rewrite group desc.
		 */
		if (!ext2fs_has_feature_gdt_csum(fs->super)) {
			err = disable_uninit_bg(fs,
					EXT4_FEATURE_RO_COMPAT_METADATA_CSUM);
			if (err)
				return 1;
		} else
			/*
			 * metadata_csum previously provided uninit_bg, so if
			 * we're also setting the uninit_bg feature bit,
			 * pretend like it was previously enabled.  Checksums
			 * will be rewritten with crc16 later.
			 */
			old_features[E2P_FEATURE_RO_INCOMPAT] |=
				EXT4_FEATURE_RO_COMPAT_GDT_CSUM;
		fs->super->s_checksum_seed = 0;
		ext2fs_clear_feature_csum_seed(fs->super);
	}

	if (FEATURE_ON(E2P_FEATURE_RO_INCOMPAT,
		       EXT4_FEATURE_RO_COMPAT_GDT_CSUM)) {
		if (mount_flags & EXT2_MF_MOUNTED) {
			fputs(_("Cannot enable uninit_bg on a mounted "
				"filesystem!\n"), stderr);
			exit(1);
		}

		/* Do not enable uninit_bg when metadata_csum enabled */
		if (ext2fs_has_feature_metadata_csum(fs->super))
			ext2fs_clear_feature_gdt_csum(fs->super);
		else
			enable_uninit_bg(fs);
	}

	if (FEATURE_OFF(E2P_FEATURE_RO_INCOMPAT,
			EXT4_FEATURE_RO_COMPAT_GDT_CSUM)) {
		if (mount_flags & EXT2_MF_MOUNTED) {
			fputs(_("Cannot disable uninit_bg on a mounted "
				"filesystem!\n"), stderr);
			exit(1);
		}

		err = disable_uninit_bg(fs,
				EXT4_FEATURE_RO_COMPAT_GDT_CSUM);
		if (err)
			return 1;
	}

	/*
	 * We don't actually toggle 64bit; resize2fs does that.  But this
	 * must come after the metadata_csum feature_on so that it won't
	 * complain about the lack of 64bit.
	 */
	if (FEATURE_ON(E2P_FEATURE_INCOMPAT,
		       EXT4_FEATURE_INCOMPAT_64BIT)) {
		if (mount_flags & EXT2_MF_MOUNTED) {
			fprintf(stderr, _("Cannot enable 64-bit mode "
					  "while mounted!\n"));
			exit(1);
		}
		ext2fs_clear_feature_64bit(sb);
		feature_64bit = 1;
	}
	if (FEATURE_OFF(E2P_FEATURE_INCOMPAT,
			EXT4_FEATURE_INCOMPAT_64BIT)) {
		if (mount_flags & EXT2_MF_MOUNTED) {
			fprintf(stderr, _("Cannot disable 64-bit mode "
					  "while mounted!\n"));
			exit(1);
		}
		ext2fs_set_feature_64bit(sb);
		feature_64bit = -1;
	}

	if (FEATURE_ON(E2P_FEATURE_RO_INCOMPAT,
				EXT4_FEATURE_RO_COMPAT_QUOTA)) {
		/*
		 * Set the Q_flag here and handle the quota options in the code
		 * below.
		 */
		if (!Q_flag) {
			Q_flag = 1;
			/* Enable usr/grp quota by default */
			for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
				if (qtype != PRJQUOTA)
					quota_enable[qtype] = QOPT_ENABLE;
				else
					quota_enable[qtype] = QOPT_DISABLE;
			}
		}
		ext2fs_clear_feature_quota(sb);
	}

	if (FEATURE_ON(E2P_FEATURE_RO_INCOMPAT,
		       EXT4_FEATURE_RO_COMPAT_PROJECT)) {
		if (fs->super->s_inode_size == EXT2_GOOD_OLD_INODE_SIZE) {
			fprintf(stderr, _("Cannot enable project feature; "
					  "inode size too small.\n"));
			exit(1);
		}
		Q_flag = 1;
		quota_enable[PRJQUOTA] = QOPT_ENABLE;
	}

	if (FEATURE_OFF(E2P_FEATURE_RO_INCOMPAT,
			EXT4_FEATURE_RO_COMPAT_PROJECT)) {
		Q_flag = 1;
		quota_enable[PRJQUOTA] = QOPT_DISABLE;
	}

	if (FEATURE_OFF(E2P_FEATURE_RO_INCOMPAT,
				EXT4_FEATURE_RO_COMPAT_QUOTA)) {
		/*
		 * Set the Q_flag here and handle the quota options in the code
		 * below.
		 */
		if (Q_flag)
			fputs(_("\nWarning: '^quota' option overrides '-Q'"
				"arguments.\n"), stderr);
		Q_flag = 1;
		/* Disable all quota by default */
		for (qtype = 0; qtype < MAXQUOTAS; qtype++)
			quota_enable[qtype] = QOPT_DISABLE;
	}

	if (FEATURE_ON(E2P_FEATURE_INCOMPAT, EXT4_FEATURE_INCOMPAT_ENCRYPT)) {
		if (ext2fs_has_feature_casefold(sb)) {
			fputs(_("Cannot enable encrypt feature on filesystems "
				"with the encoding feature enabled.\n"),
			      stderr);
			return 1;
		}
		fs->super->s_encrypt_algos[0] =
			EXT4_ENCRYPTION_MODE_AES_256_XTS;
		fs->super->s_encrypt_algos[1] =
			EXT4_ENCRYPTION_MODE_AES_256_CTS;
	}

	if (FEATURE_ON(E2P_FEATURE_INCOMPAT,
		EXT4_FEATURE_INCOMPAT_CSUM_SEED)) {
		if (!ext2fs_has_feature_metadata_csum(sb)) {
			fputs(_("Setting feature 'metadata_csum_seed' "
				"is only supported\non filesystems with "
				"the metadata_csum feature enabled.\n"),
				stderr);
			return 1;
		}
		try_confirm_csum_seed_support();
		fs->super->s_checksum_seed = fs->csum_seed;
	}

	if (FEATURE_OFF(E2P_FEATURE_INCOMPAT,
		EXT4_FEATURE_INCOMPAT_CSUM_SEED)) {
		__le32 uuid_seed;

		uuid_seed = ext2fs_crc32c_le(~0, fs->super->s_uuid,
					sizeof(fs->super->s_uuid));
		if (fs->super->s_checksum_seed != uuid_seed) {
			if (mount_flags & (EXT2_MF_BUSY|EXT2_MF_MOUNTED)) {
				fputs(_("UUID has changed since enabling "
		"metadata_csum.  Filesystem must be unmounted "
		"\nto safely rewrite all metadata to match the new UUID.\n"),
				      stderr);
				return 1;
			}
			check_fsck_needed(fs, _("Recalculating checksums "
						"could take some time."));
			rewrite_checksums = 1;
		}
	}

	if (sb->s_rev_level == EXT2_GOOD_OLD_REV &&
	    (sb->s_feature_compat || sb->s_feature_ro_compat ||
	     sb->s_feature_incompat))
		ext2fs_update_dynamic_rev(fs);

	if (FEATURE_CHANGED(E2P_FEATURE_RO_INCOMPAT,
			    EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER) ||
	    FEATURE_OFF(E2P_FEATURE_RO_INCOMPAT,
			EXT4_FEATURE_RO_COMPAT_HUGE_FILE) ||
	    FEATURE_CHANGED(E2P_FEATURE_INCOMPAT,
			    EXT2_FEATURE_INCOMPAT_FILETYPE) ||
	    FEATURE_CHANGED(E2P_FEATURE_COMPAT,
			    EXT2_FEATURE_COMPAT_RESIZE_INODE) ||
	    FEATURE_OFF(E2P_FEATURE_RO_INCOMPAT,
			EXT2_FEATURE_RO_COMPAT_LARGE_FILE))
		request_fsck_afterwards(fs);

	if ((old_features[E2P_FEATURE_COMPAT] != sb->s_feature_compat) ||
	    (old_features[E2P_FEATURE_INCOMPAT] != sb->s_feature_incompat) ||
	    (old_features[E2P_FEATURE_RO_INCOMPAT] != sb->s_feature_ro_compat))
		ext2fs_mark_super_dirty(fs);

	return 0;
}

/*
 * Add a journal to the filesystem.
 */
static int add_journal(ext2_filsys fs)
{
	unsigned long journal_blocks;
	errcode_t	retval;
	ext2_filsys	jfs;
	io_manager	io_ptr;

	if (ext2fs_has_feature_journal(fs->super)) {
		fputs(_("The filesystem already has a journal.\n"), stderr);
		goto err;
	}
	if (journal_device) {
		if (!check_plausibility(journal_device, CHECK_BLOCK_DEV,
					NULL))
			proceed_question(-1);
		check_mount(journal_device, 0, _("journal"));
#ifdef CONFIG_TESTIO_DEBUG
		if (getenv("TEST_IO_FLAGS") || getenv("TEST_IO_BLOCK")) {
			io_ptr = test_io_manager;
			test_io_backing_manager = unix_io_manager;
		} else
#endif
			io_ptr = unix_io_manager;
		retval = ext2fs_open(journal_device, EXT2_FLAG_RW|
				     EXT2_FLAG_JOURNAL_DEV_OK, 0,
				     fs->blocksize, io_ptr, &jfs);
		if (retval) {
			com_err(program_name, retval,
				_("\n\twhile trying to open journal on %s\n"),
				journal_device);
			goto err;
		}
		printf(_("Creating journal on device %s: "),
		       journal_device);
		fflush(stdout);

		retval = ext2fs_add_journal_device(fs, jfs);
		ext2fs_close_free(&jfs);
		if (retval) {
			com_err(program_name, retval,
				_("while adding filesystem to journal on %s"),
				journal_device);
			goto err;
		}
		fputs(_("done\n"), stdout);
	} else if (journal_size) {
		fputs(_("Creating journal inode: "), stdout);
		fflush(stdout);
		journal_blocks = figure_journal_size(journal_size, fs);

		if (journal_location_string)
			journal_location =
				parse_num_blocks2(journal_location_string,
						  fs->super->s_log_block_size);
		retval = ext2fs_add_journal_inode2(fs, journal_blocks,
						   journal_location,
						   journal_flags);
		if (retval) {
			fprintf(stderr, "\n");
			com_err(program_name, retval, "%s",
				_("\n\twhile trying to create journal file"));
			return retval;
		} else
			fputs(_("done\n"), stdout);
		/*
		 * If the filesystem wasn't mounted, we need to force
		 * the block group descriptors out.
		 */
		if ((mount_flags & EXT2_MF_MOUNTED) == 0)
			fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	}
	print_check_message(fs->super->s_max_mnt_count,
			    fs->super->s_checkinterval);
	return 0;

err:
	free(journal_device);
	return 1;
}

static void handle_quota_options(ext2_filsys fs)
{
	errcode_t retval;
	quota_ctx_t qctx;
	ext2_ino_t qf_ino;
	enum quota_type qtype;
	unsigned int qtype_bits = 0;
	int need_dirty = 0;

	for (qtype = 0 ; qtype < MAXQUOTAS; qtype++)
		if (quota_enable[qtype] != 0)
			break;
	if (qtype == MAXQUOTAS)
		/* Nothing to do. */
		return;

	if (quota_enable[PRJQUOTA] == QOPT_ENABLE &&
	    fs->super->s_inode_size == EXT2_GOOD_OLD_INODE_SIZE) {
		fprintf(stderr, _("Cannot enable project quota; "
				  "inode size too small.\n"));
		exit(1);
	}

	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		if (quota_enable[qtype] == QOPT_ENABLE)
			qtype_bits |= 1 << qtype;
	}

	retval = quota_init_context(&qctx, fs, qtype_bits);
	if (retval) {
		com_err(program_name, retval,
			_("while initializing quota context in support library"));
		exit(1);
	}

	if (qtype_bits)
		quota_compute_usage(qctx);

	for (qtype = 0 ; qtype < MAXQUOTAS; qtype++) {
		if (quota_enable[qtype] == QOPT_ENABLE &&
		    *quota_sb_inump(fs->super, qtype) == 0) {
			if ((qf_ino = quota_file_exists(fs, qtype)) > 0) {
				retval = quota_update_limits(qctx, qf_ino,
							     qtype);
				if (retval) {
					com_err(program_name, retval,
						_("while updating quota limits (%d)"),
						qtype);
					exit(1);
				}
			}
			retval = quota_write_inode(qctx, 1 << qtype);
			if (retval) {
				com_err(program_name, retval,
					_("while writing quota file (%d)"),
					qtype);
				exit(1);
			}
			/* Enable Quota feature if one of quota enabled */
			if (!ext2fs_has_feature_quota(fs->super)) {
				ext2fs_set_feature_quota(fs->super);
				need_dirty = 1;
			}
			if (qtype == PRJQUOTA &&
			    !ext2fs_has_feature_project(fs->super)) {
				ext2fs_set_feature_project(fs->super);
				need_dirty = 1;
			}
		} else if (quota_enable[qtype] == QOPT_DISABLE) {
			retval = quota_remove_inode(fs, qtype);
			if (retval) {
				com_err(program_name, retval,
					_("while removing quota file (%d)"),
					qtype);
				exit(1);
			}
			if (qtype == PRJQUOTA) {
				ext2fs_clear_feature_project(fs->super);
				need_dirty = 1;
			}
		}
	}

	quota_release_context(&qctx);
	/* Clear Quota feature if all quota types disabled. */
	if (!qtype_bits) {
		for (qtype = 0 ; qtype < MAXQUOTAS; qtype++)
			if (*quota_sb_inump(fs->super, qtype))
				break;
		if (qtype == MAXQUOTAS) {
			ext2fs_clear_feature_quota(fs->super);
			need_dirty = 1;
		}

	}
	if (need_dirty)
		ext2fs_mark_super_dirty(fs);
	return;
}

static int option_handle_function(char *token)
{
	if (strncmp(token, "usr", 3) == 0) {
		quota_enable[USRQUOTA] = QOPT_ENABLE;
	} else if (strncmp(token, "^usr", 4) == 0) {
		quota_enable[USRQUOTA] = QOPT_DISABLE;
	} else if (strncmp(token, "grp", 3) == 0) {
		quota_enable[GRPQUOTA] = QOPT_ENABLE;
	} else if (strncmp(token, "^grp", 4) == 0) {
		quota_enable[GRPQUOTA] = QOPT_DISABLE;
	} else if (strncmp(token, "prj", 3) == 0) {
		quota_enable[PRJQUOTA] = QOPT_ENABLE;
	} else if (strncmp(token, "^prj", 4) == 0) {
		quota_enable[PRJQUOTA] = QOPT_DISABLE;
	} else {
		fputs(_("\nBad quota options specified.\n\n"
			"Following valid quota options are available "
			"(pass by separating with comma):\n"
			"\t[^]usr[quota]\n"
			"\t[^]grp[quota]\n"
			"\t[^]prj[quota]\n"
			"\n\n"), stderr);
		return 1;
	}
	return 0;
}

static void parse_e2label_options(int argc, char ** argv)
{
	if ((argc < 2) || (argc > 3)) {
		fputs(_("Usage: e2label device [newlabel]\n"), stderr);
		exit(1);
	}
	io_options = strchr(argv[1], '?');
	if (io_options)
		*io_options++ = 0;
	device_name = blkid_get_devname(NULL, argv[1], NULL);
	if (!device_name) {
		com_err("e2label", 0, _("Unable to resolve '%s'"),
			argv[1]);
		exit(1);
	}
	open_flag = EXT2_FLAG_JOURNAL_DEV_OK;
	if (argc == 3) {
		open_flag |= EXT2_FLAG_RW;
		L_flag = 1;
		new_label = argv[2];
	} else
		print_label++;
}

static time_t parse_time(char *str)
{
	struct	tm	ts;

	if (strcmp(str, "now") == 0) {
		return (time(0));
	}
	memset(&ts, 0, sizeof(ts));
#ifdef HAVE_STRPTIME
	strptime(str, "%Y%m%d%H%M%S", &ts);
#else
	sscanf(str, "%4d%2d%2d%2d%2d%2d", &ts.tm_year, &ts.tm_mon,
	       &ts.tm_mday, &ts.tm_hour, &ts.tm_min, &ts.tm_sec);
	ts.tm_year -= 1900;
	ts.tm_mon -= 1;
	if (ts.tm_year < 0 || ts.tm_mon < 0 || ts.tm_mon > 11 ||
	    ts.tm_mday < 0 || ts.tm_mday > 31 || ts.tm_hour > 23 ||
	    ts.tm_min > 59 || ts.tm_sec > 61)
		ts.tm_mday = 0;
#endif
	if (ts.tm_mday == 0) {
		com_err(program_name, 0,
			_("Couldn't parse date/time specifier: %s"),
			str);
		usage();
	}
	ts.tm_isdst = -1;
	return (mktime(&ts));
}

static void parse_tune2fs_options(int argc, char **argv)
{
	int c;
	char *tmp;
	struct group *gr;
	struct passwd *pw;
	int ret;
	char optstring[100] = "c:e:fg:i:jlm:o:r:s:u:C:E:I:J:L:M:O:T:U:z:Q:";

	open_flag = 0;
	printf("tune2fs %s (%s)\n", E2FSPROGS_VERSION, E2FSPROGS_DATE);
	while ((c = getopt(argc, argv, optstring)) != EOF)
		switch (c) {
		case 'c':
			max_mount_count = strtol(optarg, &tmp, 0);
			if (*tmp || max_mount_count > 16000) {
				com_err(program_name, 0,
					_("bad mounts count - %s"),
					optarg);
				usage();
			}
			if (max_mount_count == 0)
				max_mount_count = -1;
			c_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'C':
			mount_count = strtoul(optarg, &tmp, 0);
			if (*tmp || mount_count > 16000) {
				com_err(program_name, 0,
					_("bad mounts count - %s"),
					optarg);
				usage();
			}
			C_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'e':
			if (strcmp(optarg, "continue") == 0)
				errors = EXT2_ERRORS_CONTINUE;
			else if (strcmp(optarg, "remount-ro") == 0)
				errors = EXT2_ERRORS_RO;
			else if (strcmp(optarg, "panic") == 0)
				errors = EXT2_ERRORS_PANIC;
			else {
				com_err(program_name, 0,
					_("bad error behavior - %s"),
					optarg);
				usage();
			}
			e_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'E':
			extended_cmd = optarg;
			open_flag |= EXT2_FLAG_RW;
			break;
		case 'f': /* Force */
			f_flag++;
			break;
		case 'g':
			resgid = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				gr = getgrnam(optarg);
				if (gr == NULL)
					tmp = optarg;
				else {
					resgid = gr->gr_gid;
					*tmp = 0;
				}
			}
			if (*tmp) {
				com_err(program_name, 0,
					_("bad gid/group name - %s"),
					optarg);
				usage();
			}
			g_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'i':
			interval = strtoul(optarg, &tmp, 0);
			switch (*tmp) {
			case 's':
				tmp++;
				break;
			case '\0':
			case 'd':
			case 'D': /* days */
				interval *= 86400;
				if (*tmp != '\0')
					tmp++;
				break;
			case 'm':
			case 'M': /* months! */
				interval *= 86400 * 30;
				tmp++;
				break;
			case 'w':
			case 'W': /* weeks */
				interval *= 86400 * 7;
				tmp++;
				break;
			}
			if (*tmp) {
				com_err(program_name, 0,
					_("bad interval - %s"), optarg);
				usage();
			}
			i_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'j':
			if (!journal_size)
				journal_size = -1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'J':
			parse_journal_opts(optarg);
			open_flag = EXT2_FLAG_RW;
			break;
		case 'l':
			l_flag = 1;
			break;
		case 'L':
			new_label = optarg;
			L_flag = 1;
			open_flag |= EXT2_FLAG_RW |
				EXT2_FLAG_JOURNAL_DEV_OK;
			break;
		case 'm':
			reserved_ratio = strtod(optarg, &tmp);
			if (*tmp || reserved_ratio > 50 ||
			    reserved_ratio < 0) {
				com_err(program_name, 0,
					_("bad reserved block ratio - %s"),
					optarg);
				usage();
			}
			m_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'M':
			new_last_mounted = optarg;
			M_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'o':
			if (mntopts_cmd) {
				com_err(program_name, 0, "%s",
					_("-o may only be specified once"));
				usage();
			}
			mntopts_cmd = optarg;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'O':
			if (features_cmd) {
				com_err(program_name, 0, "%s",
					_("-O may only be specified once"));
				usage();
			}
			features_cmd = optarg;
			open_flag = EXT2_FLAG_RW;
			break;
		case 'Q':
			Q_flag = 1;
			ret = parse_quota_opts(optarg, option_handle_function);
			if (ret)
				exit(1);
			open_flag = EXT2_FLAG_RW;
			break;
		case 'r':
			reserved_blocks = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				com_err(program_name, 0,
					_("bad reserved blocks count - %s"),
					optarg);
				usage();
			}
			r_flag = 1;
			open_flag = EXT2_FLAG_RW;
			break;
		case 's': /* Deprecated */
			s_flag = atoi(optarg);
			open_flag = EXT2_FLAG_RW;
			break;
		case 'T':
			T_flag = 1;
			last_check_time = parse_time(optarg);
			open_flag = EXT2_FLAG_RW;
			break;
		case 'u':
				resuid = strtoul(optarg, &tmp, 0);
				if (*tmp) {
					pw = getpwnam(optarg);
					if (pw == NULL)
						tmp = optarg;
					else {
						resuid = pw->pw_uid;
						*tmp = 0;
					}
				}
				if (*tmp) {
					com_err(program_name, 0,
						_("bad uid/user name - %s"),
						optarg);
					usage();
				}
				u_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
		case 'U':
			new_UUID = optarg;
			U_flag = 1;
			open_flag = EXT2_FLAG_RW |
				EXT2_FLAG_JOURNAL_DEV_OK;
			break;
		case 'I':
			new_inode_size = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				com_err(program_name, 0,
					_("bad inode size - %s"),
					optarg);
				usage();
			}
			if (!((new_inode_size &
			       (new_inode_size - 1)) == 0)) {
				com_err(program_name, 0,
					_("Inode size must be a "
					  "power of two- %s"),
					optarg);
				usage();
			}
			open_flag = EXT2_FLAG_RW;
			I_flag = 1;
			break;
		case 'z':
			undo_file = optarg;
			break;
		default:
			usage();
		}
	if (optind < argc - 1 || optind == argc)
		usage();
	if (!open_flag && !l_flag)
		usage();
	io_options = strchr(argv[optind], '?');
	if (io_options)
		*io_options++ = 0;
	device_name = blkid_get_devname(NULL, argv[optind], NULL);
	if (!device_name) {
		com_err(program_name, 0, _("Unable to resolve '%s'"),
			argv[optind]);
		exit(1);
	}
}

#ifdef CONFIG_BUILD_FINDFS
void do_findfs(int argc, char **argv)
{
	char	*dev;

	if ((argc != 2) ||
	    (strncmp(argv[1], "LABEL=", 6) && strncmp(argv[1], "UUID=", 5))) {
		fprintf(stderr, "Usage: findfs LABEL=<label>|UUID=<uuid>\n");
		exit(2);
	}
	dev = blkid_get_devname(NULL, argv[1], NULL);
	if (!dev) {
		com_err("findfs", 0, _("Unable to resolve '%s'"),
			argv[1]);
		exit(1);
	}
	puts(dev);
	exit(0);
}
#endif

static int parse_extended_opts(ext2_filsys fs, const char *opts)
{
	char	*buf, *token, *next, *p, *arg;
	int	len, hash_alg;
	int	r_usage = 0;

	len = strlen(opts);
	buf = malloc(len+1);
	if (!buf) {
		fprintf(stderr, "%s",
			_("Couldn't allocate memory to parse options!\n"));
		return 1;
	}
	strcpy(buf, opts);
	for (token = buf; token && *token; token = next) {
		p = strchr(token, ',');
		next = 0;
		if (p) {
			*p = 0;
			next = p+1;
		}
		arg = strchr(token, '=');
		if (arg) {
			*arg = 0;
			arg++;
		}
		if (strcmp(token, "clear-mmp") == 0 ||
		    strcmp(token, "clear_mmp") == 0) {
			clear_mmp = 1;
		} else if (strcmp(token, "mmp_update_interval") == 0) {
			unsigned long intv;
			if (!arg) {
				r_usage++;
				continue;
			}
			intv = strtoul(arg, &p, 0);
			if (*p) {
				fprintf(stderr,
					_("Invalid mmp_update_interval: %s\n"),
					arg);
				r_usage++;
				continue;
			}
			if (intv == 0) {
				intv = EXT4_MMP_UPDATE_INTERVAL;
			} else if (intv > EXT4_MMP_MAX_UPDATE_INTERVAL) {
				fprintf(stderr,
					_("mmp_update_interval too big: %lu\n"),
					intv);
				r_usage++;
				continue;
			}
			printf(P_("Setting multiple mount protection update "
				  "interval to %lu second\n",
				  "Setting multiple mount protection update "
				  "interval to %lu seconds\n", intv),
			       intv);
			fs->super->s_mmp_update_interval = intv;
			ext2fs_mark_super_dirty(fs);
		} else if (!strcmp(token, "force_fsck")) {
			fs->super->s_state |= EXT2_ERROR_FS;
			printf(_("Setting filesystem error flag to force fsck.\n"));
			ext2fs_mark_super_dirty(fs);
		} else if (!strcmp(token, "test_fs")) {
			fs->super->s_flags |= EXT2_FLAGS_TEST_FILESYS;
			printf("Setting test filesystem flag\n");
			ext2fs_mark_super_dirty(fs);
		} else if (!strcmp(token, "^test_fs")) {
			fs->super->s_flags &= ~EXT2_FLAGS_TEST_FILESYS;
			printf("Clearing test filesystem flag\n");
			ext2fs_mark_super_dirty(fs);
		} else if (strcmp(token, "stride") == 0) {
			if (!arg) {
				r_usage++;
				continue;
			}
			stride = strtoul(arg, &p, 0);
			if (*p) {
				fprintf(stderr,
					_("Invalid RAID stride: %s\n"),
					arg);
				r_usage++;
				continue;
			}
			stride_set = 1;
		} else if (strcmp(token, "stripe-width") == 0 ||
			   strcmp(token, "stripe_width") == 0) {
			if (!arg) {
				r_usage++;
				continue;
			}
			stripe_width = strtoul(arg, &p, 0);
			if (*p) {
				fprintf(stderr,
					_("Invalid RAID stripe-width: %s\n"),
					arg);
				r_usage++;
				continue;
			}
			stripe_width_set = 1;
		} else if (strcmp(token, "hash_alg") == 0 ||
			   strcmp(token, "hash-alg") == 0) {
			if (!arg) {
				r_usage++;
				continue;
			}
			hash_alg = e2p_string2hash(arg);
			if (hash_alg < 0) {
				fprintf(stderr,
					_("Invalid hash algorithm: %s\n"),
					arg);
				r_usage++;
				continue;
			}
			fs->super->s_def_hash_version = hash_alg;
			printf(_("Setting default hash algorithm "
				 "to %s (%d)\n"),
			       arg, hash_alg);
			ext2fs_mark_super_dirty(fs);
		} else if (!strcmp(token, "mount_opts")) {
			if (!arg) {
				r_usage++;
				continue;
			}
			if (strlen(arg) >= sizeof(fs->super->s_mount_opts)) {
				fprintf(stderr,
					"Extended mount options too long\n");
				continue;
			}
			ext_mount_opts = strdup(arg);
		} else
			r_usage++;
	}
	if (r_usage) {
		fprintf(stderr, "%s", _("\nBad options specified.\n\n"
			"Extended options are separated by commas, "
			"and may take an argument which\n"
			"\tis set off by an equals ('=') sign.\n\n"
			"Valid extended options are:\n"
			"\tclear_mmp\n"
			"\thash_alg=<hash algorithm>\n"
			"\tmount_opts=<extended default mount options>\n"
			"\tmmp_update_interval=<mmp update interval in seconds>\n"
			"\tstride=<RAID per-disk chunk size in blocks>\n"
			"\tstripe_width=<RAID stride*data disks in blocks>\n"
			"\tforce_fsck\n"
			"\ttest_fs\n"
			"\t^test_fs\n"));
		free(buf);
		return 1;
	}
	free(buf);

	return 0;
}

/*
 * Fill in the block bitmap bmap with the information regarding the
 * blocks to be moved
 */
static int get_move_bitmaps(ext2_filsys fs, int new_ino_blks_per_grp,
			    ext2fs_block_bitmap bmap)
{
	dgrp_t i;
	int retval;
	ext2_badblocks_list bb_list = 0;
	blk64_t j, needed_blocks = 0;
	blk64_t start_blk, end_blk;

	retval = ext2fs_read_bb_inode(fs, &bb_list);
	if (retval)
		return retval;

	for (i = 0; i < fs->group_desc_count; i++) {
		start_blk = ext2fs_inode_table_loc(fs, i) +
					fs->inode_blocks_per_group;

		end_blk = ext2fs_inode_table_loc(fs, i) +
					new_ino_blks_per_grp;

		for (j = start_blk; j < end_blk; j++) {
			if (ext2fs_test_block_bitmap2(fs->block_map, j)) {
				/*
				 * IF the block is a bad block we fail
				 */
				if (ext2fs_badblocks_list_test(bb_list, j)) {
					ext2fs_badblocks_list_free(bb_list);
					return ENOSPC;
				}

				ext2fs_mark_block_bitmap2(bmap, j);
			} else {
				/*
				 * We are going to use this block for
				 * inode table. So mark them used.
				 */
				ext2fs_mark_block_bitmap2(fs->block_map, j);
			}
		}
		needed_blocks += end_blk - start_blk;
	}

	ext2fs_badblocks_list_free(bb_list);
	if (needed_blocks > ext2fs_free_blocks_count(fs->super))
		return ENOSPC;

	return 0;
}

static int ext2fs_is_meta_block(ext2_filsys fs, blk64_t blk)
{
	dgrp_t group;
	group = ext2fs_group_of_blk2(fs, blk);
	if (ext2fs_block_bitmap_loc(fs, group) == blk)
		return 1;
	if (ext2fs_inode_bitmap_loc(fs, group) == blk)
		return 1;
	return 0;
}

static int ext2fs_is_block_in_group(ext2_filsys fs, dgrp_t group, blk64_t blk)
{
	blk64_t start_blk, end_blk;
	start_blk = fs->super->s_first_data_block +
			EXT2_GROUPS_TO_BLOCKS(fs->super, group);
	/*
	 * We cannot get new block beyond end_blk for for the last block group
	 * so we can check with EXT2_BLOCKS_PER_GROUP even for last block group
	 */
	end_blk   = start_blk + EXT2_BLOCKS_PER_GROUP(fs->super);
	if (blk >= start_blk && blk <= end_blk)
		return 1;
	return 0;
}

static int move_block(ext2_filsys fs, ext2fs_block_bitmap bmap)
{

	char *buf;
	dgrp_t group = 0;
	errcode_t retval;
	int meta_data = 0;
	blk64_t blk, new_blk, goal;
	struct blk_move *bmv;

	retval = ext2fs_get_mem(fs->blocksize, &buf);
	if (retval)
		return retval;

	for (new_blk = blk = fs->super->s_first_data_block;
	     blk < ext2fs_blocks_count(fs->super); blk++) {
		if (!ext2fs_test_block_bitmap2(bmap, blk))
			continue;

		if (ext2fs_is_meta_block(fs, blk)) {
			/*
			 * If the block is mapping a fs meta data block
			 * like group desc/block bitmap/inode bitmap. We
			 * should find a block in the same group and fix
			 * the respective fs metadata pointers. Otherwise
			 * fail
			 */
			group = ext2fs_group_of_blk2(fs, blk);
			goal = ext2fs_group_first_block2(fs, group);
			meta_data = 1;

		} else {
			goal = new_blk;
		}
		retval = ext2fs_new_block2(fs, goal, NULL, &new_blk);
		if (retval)
			goto err_out;

		/* new fs meta data block should be in the same group */
		if (meta_data && !ext2fs_is_block_in_group(fs, group, new_blk)) {
			retval = ENOSPC;
			goto err_out;
		}

		/* Mark this block as allocated */
		ext2fs_mark_block_bitmap2(fs->block_map, new_blk);

		/* Add it to block move list */
		retval = ext2fs_get_mem(sizeof(struct blk_move), &bmv);
		if (retval)
			goto err_out;

		bmv->old_loc = blk;
		bmv->new_loc = new_blk;

		list_add(&(bmv->list), &blk_move_list);

		retval = io_channel_read_blk64(fs->io, blk, 1, buf);
		if (retval)
			goto err_out;

		retval = io_channel_write_blk64(fs->io, new_blk, 1, buf);
		if (retval)
			goto err_out;
	}

err_out:
	ext2fs_free_mem(&buf);
	return retval;
}

static blk64_t translate_block(blk64_t blk)
{
	struct list_head *entry;
	struct blk_move *bmv;

	list_for_each(entry, &blk_move_list) {
		bmv = list_entry(entry, struct blk_move, list);
		if (bmv->old_loc == blk)
			return bmv->new_loc;
	}

	return 0;
}

static int process_block(ext2_filsys fs EXT2FS_ATTR((unused)),
			 blk64_t *block_nr,
			 e2_blkcnt_t blockcnt EXT2FS_ATTR((unused)),
			 blk64_t ref_block EXT2FS_ATTR((unused)),
			 int ref_offset EXT2FS_ATTR((unused)),
			 void *priv_data)
{
	int ret = 0;
	blk64_t new_blk;
	ext2fs_block_bitmap bmap = (ext2fs_block_bitmap) priv_data;

	if (!ext2fs_test_block_bitmap2(bmap, *block_nr))
		return 0;
	new_blk = translate_block(*block_nr);
	if (new_blk) {
		*block_nr = new_blk;
		/*
		 * This will force the ext2fs_write_inode in the iterator
		 */
		ret |= BLOCK_CHANGED;
	}

	return ret;
}

static int inode_scan_and_fix(ext2_filsys fs, ext2fs_block_bitmap bmap)
{
	errcode_t retval = 0;
	ext2_ino_t ino;
	blk64_t blk;
	char *block_buf = 0;
	struct ext2_inode inode;
	ext2_inode_scan	scan = NULL;

	retval = ext2fs_get_mem(fs->blocksize * 3, &block_buf);
	if (retval)
		return retval;

	retval = ext2fs_open_inode_scan(fs, 0, &scan);
	if (retval)
		goto err_out;

	while (1) {
		retval = ext2fs_get_next_inode(scan, &ino, &inode);
		if (retval)
			goto err_out;

		if (!ino)
			break;

		if (inode.i_links_count == 0)
			continue; /* inode not in use */

		/* FIXME!!
		 * If we end up modifying the journal inode
		 * the sb->s_jnl_blocks will differ. But a
		 * subsequent e2fsck fixes that.
		 * Do we need to fix this ??
		 */

		if (ext2fs_file_acl_block(fs, &inode) &&
		    ext2fs_test_block_bitmap2(bmap,
					ext2fs_file_acl_block(fs, &inode))) {
			blk = translate_block(ext2fs_file_acl_block(fs,
								    &inode));
			if (!blk)
				continue;

			ext2fs_file_acl_block_set(fs, &inode, blk);

			/*
			 * Write the inode to disk so that inode table
			 * resizing can work
			 */
			retval = ext2fs_write_inode(fs, ino, &inode);
			if (retval)
				goto err_out;
		}

		if (!ext2fs_inode_has_valid_blocks2(fs, &inode))
			continue;

		retval = ext2fs_block_iterate3(fs, ino, 0, block_buf,
					       process_block, bmap);
		if (retval)
			goto err_out;

	}

err_out:
	ext2fs_free_mem(&block_buf);
	ext2fs_close_inode_scan(scan);

	return retval;
}

/*
 * We need to scan for inode and block bitmaps that may need to be
 * moved.  This can take place if the filesystem was formatted for
 * RAID arrays using the mke2fs's extended option "stride".
 */
static int group_desc_scan_and_fix(ext2_filsys fs, ext2fs_block_bitmap bmap)
{
	dgrp_t i;
	blk64_t blk, new_blk;

	for (i = 0; i < fs->group_desc_count; i++) {
		blk = ext2fs_block_bitmap_loc(fs, i);
		if (ext2fs_test_block_bitmap2(bmap, blk)) {
			new_blk = translate_block(blk);
			if (!new_blk)
				continue;
			ext2fs_block_bitmap_loc_set(fs, i, new_blk);
		}

		blk = ext2fs_inode_bitmap_loc(fs, i);
		if (ext2fs_test_block_bitmap2(bmap, blk)) {
			new_blk = translate_block(blk);
			if (!new_blk)
				continue;
			ext2fs_inode_bitmap_loc_set(fs, i, new_blk);
		}
	}
	return 0;
}

static int expand_inode_table(ext2_filsys fs, unsigned long new_ino_size)
{
	dgrp_t i;
	blk64_t blk;
	errcode_t retval;
	int new_ino_blks_per_grp;
	unsigned int j;
	char *old_itable = NULL, *new_itable = NULL;
	char *tmp_old_itable = NULL, *tmp_new_itable = NULL;
	unsigned long old_ino_size;
	int old_itable_size, new_itable_size;

	old_itable_size = fs->inode_blocks_per_group * fs->blocksize;
	old_ino_size = EXT2_INODE_SIZE(fs->super);

	new_ino_blks_per_grp = ext2fs_div_ceil(
					EXT2_INODES_PER_GROUP(fs->super) *
					new_ino_size,
					fs->blocksize);

	new_itable_size = new_ino_blks_per_grp * fs->blocksize;

	retval = ext2fs_get_mem(old_itable_size, &old_itable);
	if (retval)
		return retval;

	retval = ext2fs_get_mem(new_itable_size, &new_itable);
	if (retval)
		goto err_out;

	tmp_old_itable = old_itable;
	tmp_new_itable = new_itable;

	for (i = 0; i < fs->group_desc_count; i++) {
		blk = ext2fs_inode_table_loc(fs, i);
		retval = io_channel_read_blk64(fs->io, blk,
				fs->inode_blocks_per_group, old_itable);
		if (retval)
			goto err_out;

		for (j = 0; j < EXT2_INODES_PER_GROUP(fs->super); j++) {
			memcpy(new_itable, old_itable, old_ino_size);

			memset(new_itable+old_ino_size, 0,
					new_ino_size - old_ino_size);

			new_itable += new_ino_size;
			old_itable += old_ino_size;
		}

		/* reset the pointer */
		old_itable = tmp_old_itable;
		new_itable = tmp_new_itable;

		retval = io_channel_write_blk64(fs->io, blk,
					new_ino_blks_per_grp, new_itable);
		if (retval)
			goto err_out;
	}

	/* Update the meta data */
	fs->inode_blocks_per_group = new_ino_blks_per_grp;
	ext2fs_free_inode_cache(fs->icache);
	fs->icache = 0;
	fs->super->s_inode_size = new_ino_size;

err_out:
	if (old_itable)
		ext2fs_free_mem(&old_itable);

	if (new_itable)
		ext2fs_free_mem(&new_itable);

	return retval;
}

static errcode_t ext2fs_calculate_summary_stats(ext2_filsys fs)
{
	blk64_t		blk;
	ext2_ino_t	ino;
	unsigned int	group = 0;
	unsigned int	count = 0;
	int		total_free = 0;
	int		group_free = 0;

	/*
	 * First calculate the block statistics
	 */
	for (blk = fs->super->s_first_data_block;
	     blk < ext2fs_blocks_count(fs->super); blk++) {
		if (!ext2fs_fast_test_block_bitmap2(fs->block_map, blk)) {
			group_free++;
			total_free++;
		}
		count++;
		if ((count == fs->super->s_blocks_per_group) ||
		    (blk == ext2fs_blocks_count(fs->super)-1)) {
			ext2fs_bg_free_blocks_count_set(fs, group++,
							group_free);
			count = 0;
			group_free = 0;
		}
	}
	total_free = EXT2FS_C2B(fs, total_free);
	ext2fs_free_blocks_count_set(fs->super, total_free);

	/*
	 * Next, calculate the inode statistics
	 */
	group_free = 0;
	total_free = 0;
	count = 0;
	group = 0;

	/* Protect loop from wrap-around if s_inodes_count maxed */
	for (ino = 1; ino <= fs->super->s_inodes_count && ino > 0; ino++) {
		if (!ext2fs_fast_test_inode_bitmap2(fs->inode_map, ino)) {
			group_free++;
			total_free++;
		}
		count++;
		if ((count == fs->super->s_inodes_per_group) ||
		    (ino == fs->super->s_inodes_count)) {
			ext2fs_bg_free_inodes_count_set(fs, group++,
							group_free);
			count = 0;
			group_free = 0;
		}
	}
	fs->super->s_free_inodes_count = total_free;
	ext2fs_mark_super_dirty(fs);
	return 0;
}

#define list_for_each_safe(pos, pnext, head) \
	for (pos = (head)->next, pnext = pos->next; pos != (head); \
	     pos = pnext, pnext = pos->next)

static void free_blk_move_list(void)
{
	struct list_head *entry, *tmp;
	struct blk_move *bmv;

	list_for_each_safe(entry, tmp, &blk_move_list) {
		bmv = list_entry(entry, struct blk_move, list);
		list_del(entry);
		ext2fs_free_mem(&bmv);
	}
	return;
}

static int resize_inode(ext2_filsys fs, unsigned long new_size)
{
	errcode_t retval;
	int new_ino_blks_per_grp;
	ext2fs_block_bitmap bmap;

	retval = ext2fs_read_inode_bitmap(fs);
	if (retval) {
		fputs(_("Failed to read inode bitmap\n"), stderr);
		return retval;
	}
	retval = ext2fs_read_block_bitmap(fs);
	if (retval) {
		fputs(_("Failed to read block bitmap\n"), stderr);
		return retval;
	}
	INIT_LIST_HEAD(&blk_move_list);


	new_ino_blks_per_grp = ext2fs_div_ceil(
					EXT2_INODES_PER_GROUP(fs->super)*
					new_size,
					fs->blocksize);

	/* We may change the file system.
	 * Mark the file system as invalid so that
	 * the user is prompted to run fsck.
	 */
	fs->super->s_state &= ~EXT2_VALID_FS;

	retval = ext2fs_allocate_block_bitmap(fs, _("blocks to be moved"),
						&bmap);
	if (retval) {
		fputs(_("Failed to allocate block bitmap when "
				"increasing inode size\n"), stderr);
		return retval;
	}
	retval = get_move_bitmaps(fs, new_ino_blks_per_grp, bmap);
	if (retval) {
		fputs(_("Not enough space to increase inode size \n"), stderr);
		goto err_out;
	}
	retval = move_block(fs, bmap);
	if (retval) {
		fputs(_("Failed to relocate blocks during inode resize \n"),
		      stderr);
		goto err_out;
	}
	retval = inode_scan_and_fix(fs, bmap);
	if (retval)
		goto err_out_undo;

	retval = group_desc_scan_and_fix(fs, bmap);
	if (retval)
		goto err_out_undo;

	retval = expand_inode_table(fs, new_size);
	if (retval)
		goto err_out_undo;

	ext2fs_calculate_summary_stats(fs);

	fs->super->s_state |= EXT2_VALID_FS;
	/* mark super block and block bitmap as dirty */
	ext2fs_mark_super_dirty(fs);
	ext2fs_mark_bb_dirty(fs);

err_out:
	free_blk_move_list();
	ext2fs_free_block_bitmap(bmap);

	return retval;

err_out_undo:
	free_blk_move_list();
	ext2fs_free_block_bitmap(bmap);
	fputs(_("Error in resizing the inode size.\n"
			"Run e2undo to undo the "
			"file system changes. \n"), stderr);

	return retval;
}

static int tune2fs_setup_tdb(const char *name, io_manager *io_ptr)
{
	errcode_t retval = 0;
	const char *tdb_dir;
	char *tdb_file = NULL;
	char *dev_name, *tmp_name;

	/* (re)open a specific undo file */
	if (undo_file && undo_file[0] != 0) {
		retval = set_undo_io_backing_manager(*io_ptr);
		if (retval)
			goto err;
		*io_ptr = undo_io_manager;
		retval = set_undo_io_backup_file(undo_file);
		if (retval)
			goto err;
		printf(_("Overwriting existing filesystem; this can be undone "
			 "using the command:\n"
			 "    e2undo %s %s\n\n"),
			 undo_file, name);
		return retval;
	}

	/*
	 * Configuration via a conf file would be
	 * nice
	 */
	tdb_dir = getenv("E2FSPROGS_UNDO_DIR");
	if (!tdb_dir)
		tdb_dir = "/var/lib/e2fsprogs";

	if (!strcmp(tdb_dir, "none") || (tdb_dir[0] == 0) ||
	    access(tdb_dir, W_OK))
		return 0;

	tmp_name = strdup(name);
	if (!tmp_name)
		goto errout;
	dev_name = basename(tmp_name);
	tdb_file = malloc(strlen(tdb_dir) + 9 + strlen(dev_name) + 7 + 1);
	if (!tdb_file) {
		free(tmp_name);
		goto errout;
	}
	sprintf(tdb_file, "%s/tune2fs-%s.e2undo", tdb_dir, dev_name);
	free(tmp_name);

	if ((unlink(tdb_file) < 0) && (errno != ENOENT)) {
		retval = errno;
		com_err(program_name, retval,
			_("while trying to delete %s"), tdb_file);
		goto errout;
	}

	retval = set_undo_io_backing_manager(*io_ptr);
	if (retval)
		goto errout;
	*io_ptr = undo_io_manager;
	retval = set_undo_io_backup_file(tdb_file);
	if (retval)
		goto errout;
	printf(_("Overwriting existing filesystem; this can be undone "
		 "using the command:\n"
		 "    e2undo %s %s\n\n"),
		 tdb_file, name);

	free(tdb_file);
	return 0;
errout:
	free(tdb_file);
err:
	com_err("tune2fs", retval, "while trying to setup undo file\n");
	return retval;
}

static int
fs_update_journal_user(struct ext2_super_block *sb, __u8 old_uuid[UUID_SIZE])
{
	int retval, nr_users, start;
	journal_superblock_t *jsb;
	ext2_filsys jfs;
	__u8 *j_uuid;
	char *journal_path;
	char uuid[UUID_STR_SIZE];
	char buf[SUPERBLOCK_SIZE] __attribute__ ((aligned(8)));

	if (!ext2fs_has_feature_journal(sb) || uuid_is_null(sb->s_journal_uuid))
		return 0;

	uuid_unparse(sb->s_journal_uuid, uuid);
	journal_path = blkid_get_devname(NULL, "UUID", uuid);
	if (!journal_path)
		return 0;

	retval = ext2fs_open2(journal_path, io_options,
			      EXT2_FLAG_JOURNAL_DEV_OK | EXT2_FLAG_RW,
			      0, 0, unix_io_manager, &jfs);
	if (retval) {
		com_err(program_name, retval,
			_("while trying to open %s"),
			journal_path);
		return retval;
	}

	retval = get_journal_sb(jfs, buf);
	if (retval != 0) {
		if (retval == EXT2_ET_UNSUPP_FEATURE)
			fprintf(stderr, _("%s is not a journal device.\n"),
				journal_path);
		return retval;
	}

	jsb = (journal_superblock_t *) buf;
	/* Find the filesystem UUID */
	nr_users = ntohl(jsb->s_nr_users);
	if (nr_users > JFS_USERS_MAX) {
		fprintf(stderr, _("Journal superblock is corrupted, nr_users\n"
				 "is too high (%d).\n"), nr_users);
		return EXT2_ET_CORRUPT_JOURNAL_SB;
	}

	j_uuid = journal_user(old_uuid, jsb->s_users, nr_users);
	if (j_uuid == NULL) {
		fputs(_("Filesystem's UUID not found on journal device.\n"),
		      stderr);
		return EXT2_ET_LOAD_EXT_JOURNAL;
	}

	memcpy(j_uuid, sb->s_uuid, UUID_SIZE);

	start = ext2fs_journal_sb_start(jfs->blocksize);
	/* Write back the journal superblock */
	retval = io_channel_write_blk64(jfs->io, start, -SUPERBLOCK_SIZE, buf);
	if (retval != 0) {
		com_err(program_name, retval,
			"while writing journal superblock.");
		return retval;
	}

	ext2fs_close(jfs);

	return 0;
}

#ifndef BUILD_AS_LIB
int main(int argc, char **argv)
#else
int tune2fs_main(int argc, char **argv)
#endif  /* BUILD_AS_LIB */
{
	errcode_t retval;
	ext2_filsys fs;
	struct ext2_super_block *sb;
	io_manager io_ptr, io_ptr_orig = NULL;
	int rc = 0;
	char default_undo_file[1] = { 0 };

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
	set_com_err_gettext(gettext);
#endif
	if (argc && *argv)
		program_name = *argv;
	add_error_table(&et_ext2_error_table);

#ifdef CONFIG_BUILD_FINDFS
	if (strcmp(get_progname(argv[0]), "findfs") == 0)
		do_findfs(argc, argv);
#endif
	if (strcmp(get_progname(argv[0]), "e2label") == 0)
		parse_e2label_options(argc, argv);
	else
		parse_tune2fs_options(argc, argv);

#ifdef CONFIG_TESTIO_DEBUG
	if (getenv("TEST_IO_FLAGS") || getenv("TEST_IO_DEBUG")) {
		io_ptr = test_io_manager;
		test_io_backing_manager = unix_io_manager;
	} else
#endif
		io_ptr = unix_io_manager;

retry_open:
	if ((open_flag & EXT2_FLAG_RW) == 0 || f_flag)
		open_flag |= EXT2_FLAG_SKIP_MMP;

	open_flag |= EXT2_FLAG_64BITS | EXT2_FLAG_JOURNAL_DEV_OK;

	/* keep the filesystem struct around to dump MMP data */
	open_flag |= EXT2_FLAG_NOFREE_ON_ERROR;

	retval = ext2fs_open2(device_name, io_options, open_flag,
			      0, 0, io_ptr, &fs);
	if (retval) {
		com_err(program_name, retval,
			_("while trying to open %s"),
			device_name);
		if (retval == EXT2_ET_MMP_FSCK_ON ||
		    retval == EXT2_ET_MMP_UNKNOWN_SEQ)
			dump_mmp_msg(fs->mmp_buf,
				     _("If you are sure the filesystem "
				       "is not in use on any node, run:\n"
				       "'tune2fs -f -E clear_mmp {device}'\n"));
		else if (retval == EXT2_ET_MMP_FAILED)
			dump_mmp_msg(fs->mmp_buf, NULL);
		else if (retval == EXT2_ET_MMP_MAGIC_INVALID)
			fprintf(stderr,
				_("MMP block magic is bad. Try to fix it by "
				  "running:\n'e2fsck -f %s'\n"), device_name);
		else if (retval == EXT2_ET_BAD_MAGIC)
			check_plausibility(device_name, CHECK_FS_EXIST, NULL);
		else if (retval != EXT2_ET_MMP_FAILED)
			fprintf(stderr, "%s",
			     _("Couldn't find valid filesystem superblock.\n"));

		ext2fs_free(fs);
		exit(1);
	}
	if (ext2fs_has_feature_journal_dev(fs->super)) {
		fprintf(stderr, "%s", _("Cannot modify a journal device.\n"));
		ext2fs_free(fs);
		exit(1);
	}
	fs->default_bitmap_type = EXT2FS_BMAP64_RBTREE;

	if (I_flag) {
		/*
		 * Check the inode size is right so we can issue an
		 * error message and bail before setting up the tdb
		 * file.
		 */
		if (new_inode_size == EXT2_INODE_SIZE(fs->super)) {
			fprintf(stderr, _("The inode size is already %lu\n"),
				new_inode_size);
			rc = 1;
			goto closefs;
		}
		if (new_inode_size < EXT2_INODE_SIZE(fs->super)) {
			fprintf(stderr, "%s",
				_("Shrinking inode size is not supported\n"));
			rc = 1;
			goto closefs;
		}
		if (new_inode_size > fs->blocksize) {
			fprintf(stderr, _("Invalid inode size %lu (max %d)\n"),
				new_inode_size, fs->blocksize);
			rc = 1;
			goto closefs;
		}
		check_fsck_needed(fs,
			_("Resizing inodes could take some time."));
		/*
		 * If inode resize is requested use the
		 * Undo I/O manager
		 */
		undo_file = default_undo_file;
	}

	/* Set up an undo file */
	if (undo_file && io_ptr_orig == NULL) {
		io_ptr_orig = io_ptr;
		retval = tune2fs_setup_tdb(device_name, &io_ptr);
		if (retval) {
			rc = 1;
			goto closefs;
		}
		if (io_ptr != io_ptr_orig) {
			ext2fs_close_free(&fs);
			goto retry_open;
		}
	}

	sb = fs->super;
	fs->flags &= ~EXT2_FLAG_MASTER_SB_ONLY;

	if (print_label) {
		/* For e2label emulation */
		printf("%.*s\n", EXT2_LEN_STR(sb->s_volume_name));
		remove_error_table(&et_ext2_error_table);
		goto closefs;
	}

	retval = ext2fs_check_if_mounted(device_name, &mount_flags);
	if (retval) {
		com_err("ext2fs_check_if_mount", retval,
			_("while determining whether %s is mounted."),
			device_name);
		rc = 1;
		goto closefs;
	}

#ifdef NO_RECOVERY
	/* Warn if file system needs recovery and it is opened for writing. */
	if ((open_flag & EXT2_FLAG_RW) && !(mount_flags & EXT2_MF_MOUNTED) &&
	    (sb->s_feature_compat & EXT3_FEATURE_COMPAT_HAS_JOURNAL) &&
	    (sb->s_feature_incompat & EXT3_FEATURE_INCOMPAT_RECOVER)) {
		fprintf(stderr,
_("Warning: The journal is dirty. You may wish to replay the journal like:\n\n"
  "\te2fsck -E journal_only %s\n\n"
  "then rerun this command.  Otherwise, any changes made may be overwritten\n"
  "by journal recovery.\n"), device_name);
	}
#else
	/* Recover the journal if possible. */
	if ((open_flag & EXT2_FLAG_RW) && !(mount_flags & (EXT2_MF_BUSY | EXT2_MF_MOUNTED)) &&
	    ext2fs_has_feature_journal_needs_recovery(fs->super)) {
		errcode_t err;

		printf(_("Recovering journal.\n"));
		err = ext2fs_run_ext3_journal(&fs);
		if (err) {
			com_err("tune2fs", err, "while recovering journal.\n");
			printf(_("Please run e2fsck -fy %s.\n"), argv[1]);
			if (fs)
				ext2fs_close_free(&fs);
			exit(1);
		}
		sb = fs->super;
	}
#endif

	/* Normally we only need to write out the superblock */
	fs->flags |= EXT2_FLAG_SUPER_ONLY;

	if (c_flag) {
		sb->s_max_mnt_count = max_mount_count;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting maximal mount count to %d\n"),
		       max_mount_count);
	}
	if (C_flag) {
		sb->s_mnt_count = mount_count;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting current mount count to %d\n"), mount_count);
	}
	if (e_flag) {
		sb->s_errors = errors;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting error behavior to %d\n"), errors);
	}
	if (g_flag) {
		sb->s_def_resgid = resgid;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting reserved blocks gid to %lu\n"), resgid);
	}
	if (i_flag) {
		if ((unsigned long long)interval >= (1ULL << 32)) {
			com_err(program_name, 0,
				_("interval between checks is too big (%lu)"),
				interval);
			rc = 1;
			goto closefs;
		}
		sb->s_checkinterval = interval;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting interval between checks to %lu seconds\n"),
		       interval);
	}
	if (m_flag) {
		ext2fs_r_blocks_count_set(sb, reserved_ratio *
					  ext2fs_blocks_count(sb) / 100.0);
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting reserved blocks percentage to %g%% (%llu blocks)\n"),
			reserved_ratio, ext2fs_r_blocks_count(sb));
	}
	if (r_flag) {
		if (reserved_blocks > ext2fs_blocks_count(sb)/2) {
			com_err(program_name, 0,
				_("reserved blocks count is too big (%llu)"),
				reserved_blocks);
			rc = 1;
			goto closefs;
		}
		ext2fs_r_blocks_count_set(sb, reserved_blocks);
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting reserved blocks count to %llu\n"),
		       reserved_blocks);
	}
	if (s_flag == 1) {
		if (ext2fs_has_feature_sparse_super(sb)) {
			fputs(_("\nThe filesystem already has sparse "
				"superblocks.\n"), stderr);
		} else if (ext2fs_has_feature_meta_bg(sb)) {
			fputs(_("\nSetting the sparse superblock flag not "
				"supported\nfor filesystems with "
				"the meta_bg feature enabled.\n"),
				stderr);
			rc = 1;
			goto closefs;
		} else {
			ext2fs_set_feature_sparse_super(sb);
			sb->s_state &= ~EXT2_VALID_FS;
			ext2fs_mark_super_dirty(fs);
			printf(_("\nSparse superblock flag set.  %s"),
			       _(please_fsck));
		}
	}
	if (s_flag == 0) {
		fputs(_("\nClearing the sparse superblock flag not supported.\n"),
		      stderr);
		rc = 1;
		goto closefs;
	}
	if (T_flag) {
		sb->s_lastcheck = last_check_time;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting time filesystem last checked to %s\n"),
		       ctime(&last_check_time));
	}
	if (u_flag) {
		sb->s_def_resuid = resuid;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting reserved blocks uid to %lu\n"), resuid);
	}
	if (L_flag) {
		if (strlen(new_label) > sizeof(sb->s_volume_name))
			fputs(_("Warning: label too long, truncating.\n"),
			      stderr);
		memset(sb->s_volume_name, 0, sizeof(sb->s_volume_name));
		strncpy(sb->s_volume_name, new_label,
			sizeof(sb->s_volume_name));
		ext2fs_mark_super_dirty(fs);
	}
	if (M_flag) {
		memset(sb->s_last_mounted, 0, sizeof(sb->s_last_mounted));
		strncpy(sb->s_last_mounted, new_last_mounted,
			sizeof(sb->s_last_mounted));
		ext2fs_mark_super_dirty(fs);
	}
	if (mntopts_cmd) {
		rc = update_mntopts(fs, mntopts_cmd);
		if (rc)
			goto closefs;
	}
	if (features_cmd) {
		rc = update_feature_set(fs, features_cmd);
		if (rc)
			goto closefs;
	}
	if (extended_cmd) {
		rc = parse_extended_opts(fs, extended_cmd);
		if (rc)
			goto closefs;
		if (clear_mmp && !f_flag) {
			fputs(_("Error in using clear_mmp. "
				"It must be used with -f\n"),
			      stderr);
			goto closefs;
		}
	}
	if (clear_mmp) {
		rc = ext2fs_mmp_clear(fs);
		goto closefs;
	}
	if (journal_size || journal_device) {
		rc = add_journal(fs);
		if (rc)
			goto closefs;
	}

	if (Q_flag) {
		if (mount_flags & EXT2_MF_MOUNTED) {
			fputs(_("The quota feature may only be changed when "
				"the filesystem is unmounted.\n"), stderr);
			rc = 1;
			goto closefs;
		}
		handle_quota_options(fs);
	}

	if (U_flag) {
		int set_csum = 0;
		dgrp_t i;
		char buf[SUPERBLOCK_SIZE] __attribute__ ((aligned(8)));
		__u8 old_uuid[UUID_SIZE];

		if (!ext2fs_has_feature_csum_seed(fs->super) &&
		    (ext2fs_has_feature_metadata_csum(fs->super) ||
		     ext2fs_has_feature_ea_inode(fs->super))) {
			check_fsck_needed(fs,
				_("Setting the UUID on this "
				  "filesystem could take some time."));
			rewrite_checksums = 1;
		}

		if (ext2fs_has_group_desc_csum(fs)) {
			/*
			 * Changing the UUID on a metadata_csum FS requires
			 * rewriting all metadata, which can race with a
			 * mounted fs.  Don't allow that unless we're saving
			 * the checksum seed.
			 */
			if ((mount_flags & EXT2_MF_MOUNTED) &&
			    !ext2fs_has_feature_csum_seed(fs->super) &&
			    ext2fs_has_feature_metadata_csum(fs->super)) {
				fputs(_("The UUID may only be "
					"changed when the filesystem is "
					"unmounted.\n"), stderr);
				fputs(_("If you only use kernels newer than "
					"v4.4, run 'tune2fs -O "
					"metadata_csum_seed' and re-run this "
					"command.\n"), stderr);
				try_confirm_csum_seed_support();
				exit(1);
			}

			/*
			 * Determine if the block group checksums are
			 * correct so we know whether or not to set
			 * them later on.
			 */
			for (i = 0; i < fs->group_desc_count; i++)
				if (!ext2fs_group_desc_csum_verify(fs, i))
					break;
			if (i >= fs->group_desc_count)
				set_csum = 1;
		}

		memcpy(old_uuid, sb->s_uuid, UUID_SIZE);
		if ((strcasecmp(new_UUID, "null") == 0) ||
		    (strcasecmp(new_UUID, "clear") == 0)) {
			uuid_clear(sb->s_uuid);
		} else if (strcasecmp(new_UUID, "time") == 0) {
			uuid_generate_time(sb->s_uuid);
		} else if (strcasecmp(new_UUID, "random") == 0) {
			uuid_generate(sb->s_uuid);
		} else if (uuid_parse(new_UUID, sb->s_uuid)) {
			com_err(program_name, 0, "%s",
				_("Invalid UUID format\n"));
			rc = 1;
			goto closefs;
		}
		ext2fs_init_csum_seed(fs);
		if (set_csum) {
			for (i = 0; i < fs->group_desc_count; i++)
				ext2fs_group_desc_csum_set(fs, i);
			fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
		}

		/* If this is a journal dev, we need to copy UUID into jsb */
		if (!(rc = get_journal_sb(fs, buf))) {
			journal_superblock_t *jsb;

			jsb = (journal_superblock_t *) buf;
			fputs(_("Need to update journal superblock.\n"), stdout);
			memcpy(jsb->s_uuid, sb->s_uuid, sizeof(sb->s_uuid));

			/* Writeback the journal superblock */
			if ((rc = io_channel_write_blk64(fs->io,
				ext2fs_journal_sb_start(fs->blocksize),
					-SUPERBLOCK_SIZE, buf)))
				goto closefs;
		} else if (rc != EXT2_ET_UNSUPP_FEATURE)
			goto closefs;
		else {
			rc = 0; /** Reset rc to avoid ext2fs_mmp_stop() */

			if ((rc = fs_update_journal_user(sb, old_uuid)))
				goto closefs;
		}

		ext2fs_mark_super_dirty(fs);
	}

	if (I_flag) {
		if (mount_flags & EXT2_MF_MOUNTED) {
			fputs(_("The inode size may only be "
				"changed when the filesystem is "
				"unmounted.\n"), stderr);
			rc = 1;
			goto closefs;
		}
		if (ext2fs_has_feature_flex_bg(fs->super)) {
			fputs(_("Changing the inode size not supported for "
				"filesystems with the flex_bg\n"
				"feature enabled.\n"),
			      stderr);
			rc = 1;
			goto closefs;
		}
		/*
		 * We want to update group descriptor also
		 * with the new free inode count
		 */
		if (rewrite_checksums)
			fs->flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
		fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
		retval = resize_inode(fs, new_inode_size);
		if (rewrite_checksums)
			fs->flags &= ~EXT2_FLAG_IGNORE_CSUM_ERRORS;
		if (retval == 0) {
			printf(_("Setting inode size %lu\n"),
							new_inode_size);
			rewrite_checksums = 1;
		} else {
			printf("%s", _("Failed to change inode size\n"));
			rc = 1;
			goto closefs;
		}
	}

	if (rewrite_checksums)
		rewrite_metadata_checksums(fs);

	if (l_flag)
		list_super(sb);
	if (stride_set) {
		sb->s_raid_stride = stride;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting stride size to %d\n"), stride);
	}
	if (stripe_width_set) {
		sb->s_raid_stripe_width = stripe_width;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting stripe width to %d\n"), stripe_width);
	}
	if (ext_mount_opts) {
		strncpy((char *)(fs->super->s_mount_opts), ext_mount_opts,
			sizeof(fs->super->s_mount_opts));
		fs->super->s_mount_opts[sizeof(fs->super->s_mount_opts)-1] = 0;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting extended default mount options to '%s'\n"),
		       ext_mount_opts);
		free(ext_mount_opts);
	}

	free(device_name);
	remove_error_table(&et_ext2_error_table);

closefs:
	if (rc) {
		ext2fs_mmp_stop(fs);
#ifndef BUILD_AS_LIB
		exit(1);
#endif
	}

	if (feature_64bit)
		convert_64bit(fs, feature_64bit);
	return (ext2fs_close_free(&fs) ? 1 : 0);
}
