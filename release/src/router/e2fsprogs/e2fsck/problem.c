/*
 * problem.c --- report filesystem problems to the user
 *
 * Copyright 1996, 1997 by Theodore Ts'o
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>

#include "e2fsck.h"

#include "problem.h"
#include "problemP.h"

#define PROMPT_NONE	0
#define PROMPT_FIX	1
#define PROMPT_CLEAR	2
#define PROMPT_RELOCATE	3
#define PROMPT_ALLOCATE 4
#define PROMPT_EXPAND	5
#define PROMPT_CONNECT	6
#define PROMPT_CREATE	7
#define PROMPT_SALVAGE	8
#define PROMPT_TRUNCATE	9
#define PROMPT_CLEAR_INODE 10
#define PROMPT_ABORT	11
#define PROMPT_SPLIT	12
#define PROMPT_CONTINUE	13
#define PROMPT_CLONE	14
#define PROMPT_DELETE	15
#define PROMPT_SUPPRESS 16
#define PROMPT_UNLINK	17
#define PROMPT_CLEAR_HTREE 18
#define PROMPT_RECREATE 19
#define PROMPT_OPTIMIZE 20
#define PROMPT_CLEAR_FLAG 21
#define PROMPT_NULL	22

/*
 * These are the prompts which are used to ask the user if they want
 * to fix a problem.
 */
static const char *prompt[] = {
	N_("(no prompt)"),	/* 0 */
	N_("Fix"),		/* 1 */
	N_("Clear"),		/* 2 */
	N_("Relocate"),		/* 3 */
	N_("Allocate"),		/* 4 */
	N_("Expand"),		/* 5 */
	N_("Connect to /lost+found"), /* 6 */
	N_("Create"),		/* 7 */
	N_("Salvage"),		/* 8 */
	N_("Truncate"),		/* 9 */
	N_("Clear inode"),	/* 10 */
	N_("Abort"),		/* 11 */
	N_("Split"),		/* 12 */
	N_("Continue"),		/* 13 */
	N_("Clone multiply-claimed blocks"), /* 14 */
	N_("Delete file"),	/* 15 */
	N_("Suppress messages"),/* 16 */
	N_("Unlink"),		/* 17 */
	N_("Clear HTree index"),/* 18 */
	N_("Recreate"),		/* 19 */
	N_("Optimize"),		/* 20 */
	N_("Clear flag"),	/* 21 */
	"",			/* 22 */
};

/*
 * These messages are printed when we are preen mode and we will be
 * automatically fixing the problem.
 */
static const char *preen_msg[] = {
	N_("(NONE)"),		/* 0 */
	N_("FIXED"),		/* 1 */
	N_("CLEARED"),		/* 2 */
	N_("RELOCATED"),	/* 3 */
	N_("ALLOCATED"),	/* 4 */
	N_("EXPANDED"),		/* 5 */
	N_("RECONNECTED"),	/* 6 */
	N_("CREATED"),		/* 7 */
	N_("SALVAGED"),		/* 8 */
	N_("TRUNCATED"),	/* 9 */
	N_("INODE CLEARED"),	/* 10 */
	N_("ABORTED"),		/* 11 */
	N_("SPLIT"),		/* 12 */
	N_("CONTINUING"),	/* 13 */
	N_("MULTIPLY-CLAIMED BLOCKS CLONED"), /* 14 */
	N_("FILE DELETED"),	/* 15 */
	N_("SUPPRESSED"),	/* 16 */
	N_("UNLINKED"),		/* 17 */
	N_("HTREE INDEX CLEARED"),/* 18 */
	N_("WILL RECREATE"),	/* 19 */
	N_("WILL OPTIMIZE"),	/* 20 */
	N_("FLAG CLEARED"),	/* 21 */
	"",			/* 22 */
};

#if __GNUC_PREREQ (4, 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

static struct e2fsck_problem problem_table[] = {

	/* Pre-Pass 1 errors */

	/* Block bitmap for group is not in group */
	{ PR_0_BB_NOT_GROUP, N_("@b @B for @g %g is not in @g.  (@b %b)\n"),
	  PROMPT_RELOCATE, PR_LATCH_RELOC, 0, 0, 0 },

	/* Inode bitmap for group is not in group */
	{ PR_0_IB_NOT_GROUP, N_("@i @B for @g %g is not in @g.  (@b %b)\n"),
	  PROMPT_RELOCATE, PR_LATCH_RELOC, 0, 0, 0 },

	/* Inode table for group is not in group.  (block nnnn) */
	{ PR_0_ITABLE_NOT_GROUP,
	  N_("@i table for @g %g is not in @g.  (@b %b)\n"
	  "WARNING: SEVERE DATA LOSS POSSIBLE.\n"),
	  PROMPT_RELOCATE, PR_LATCH_RELOC, 0, 0, 0 },

	/* Superblock corrupt */
	{ PR_0_SB_CORRUPT,
	  N_("\nThe @S could not be read or does not describe a valid ext2/ext3/ext4\n"
	  "@f.  If the @v is valid and it really contains an ext2/ext3/ext4\n"
	  "@f (and not swap or ufs or something else), then the @S\n"
	  "is corrupt, and you might try running e2fsck with an alternate @S:\n"
	  "    e2fsck -b 8193 <@v>\n"
	  " or\n"
	  "    e2fsck -b 32768 <@v>\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Filesystem size is wrong */
	{ PR_0_FS_SIZE_WRONG,
	  N_("The @f size (according to the @S) is %b @bs\n"
	  "The physical size of the @v is %c @bs\n"
	  "Either the @S or the partition table is likely to be corrupt!\n"),
	  PROMPT_ABORT, 0, 0, 0, 0 },

	/* Fragments not supported */
	{ PR_0_NO_FRAGMENTS,
	  N_("@S @b_size = %b, fragsize = %c.\n"
	  "This version of e2fsck does not support fragment sizes different\n"
	  "from the @b size.\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Superblock blocks_per_group = bbbb, should have been cccc */
	{ PR_0_BLOCKS_PER_GROUP,
	  N_("@S @bs_per_group = %b, should have been %c\n"),
	  PROMPT_NONE, PR_AFTER_CODE, PR_0_SB_CORRUPT, 0, 0 },

	/* Superblock first_data_block = bbbb, should have been cccc */
	{ PR_0_FIRST_DATA_BLOCK,
	  N_("@S first_data_@b = %b, should have been %c\n"),
	  PROMPT_NONE, PR_AFTER_CODE, PR_0_SB_CORRUPT, 0, 0 },

	/* Filesystem did not have a UUID; generating one */
	{ PR_0_ADD_UUID,
	  N_("@f did not have a UUID; generating one.\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Relocate hint */
	{ PR_0_RELOCATE_HINT,
	  /* xgettext:no-c-format */
	  N_("Note: if several inode or block bitmap blocks or part\n"
	  "of the inode table require relocation, you may wish to try\n"
	  "running e2fsck with the '-b %S' option first.  The problem\n"
	  "may lie only with the primary block group descriptors, and\n"
	  "the backup block group descriptors may be OK.\n\n"),
	  PROMPT_NONE, PR_PREEN_OK | PR_NOCOLLATE, 0, 0, 0 },

	/* Miscellaneous superblock corruption */
	{ PR_0_MISC_CORRUPT_SUPER,
	  N_("Corruption found in @S.  (%s = %N).\n"),
	  PROMPT_NONE, PR_AFTER_CODE, PR_0_SB_CORRUPT, 0, 0 },

	/* Error determining physical device size of filesystem */
	{ PR_0_GETSIZE_ERROR,
	  /* xgettext:no-c-format */
	  N_("Error determining size of the physical @v: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Inode count in superblock is incorrect */
	{ PR_0_INODE_COUNT_WRONG,
	  N_("@i count in @S is %i, @s %j.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	{ PR_0_HURD_CLEAR_FILETYPE,
	  N_("The Hurd does not support the filetype feature.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Superblock has an invalid journal (inode inum) */
	{ PR_0_JOURNAL_BAD_INODE,
	  /* xgettext:no-c-format */
	  N_("@S has an @n @j (@i %i).\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* External journal has multiple filesystem users (unsupported) */
	{ PR_0_JOURNAL_UNSUPP_MULTIFS,
	  N_("External @j has multiple @f users (unsupported).\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Can't find external journal */
	{ PR_0_CANT_FIND_JOURNAL,
	  N_("Can't find external @j\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* External journal has bad superblock */
	{ PR_0_EXT_JOURNAL_BAD_SUPER,
	  N_("External @j has bad @S\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Superblock has a bad journal UUID */
	{ PR_0_JOURNAL_BAD_UUID,
	  N_("External @j does not support this @f\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Filesystem journal superblock is an unknown type */
	{ PR_0_JOURNAL_UNSUPP_SUPER,
	  N_("@f @j @S is unknown type %N (unsupported).\n"
	     "It is likely that your copy of e2fsck is old and/or doesn't "
	     "support this @j format.\n"
	     "It is also possible the @j @S is corrupt.\n"),
	  PROMPT_ABORT, PR_NO_OK | PR_AFTER_CODE, PR_0_JOURNAL_BAD_SUPER,
	  0, 0 },

	/* Journal superblock is corrupt */
	{ PR_0_JOURNAL_BAD_SUPER,
	  N_("@j @S is corrupt.\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Superblock has_journal flag is clear but has a journal */
	{ PR_0_JOURNAL_HAS_JOURNAL,
	  N_("@S has_@j flag is clear, but a @j is present.\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Superblock needs_recovery flag is set but no journal is present */
	{ PR_0_JOURNAL_RECOVER_SET,
	  N_("@S needs_recovery flag is set, but no @j is present.\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Superblock needs_recovery flag is set, but journal has data */
	{ PR_0_JOURNAL_RECOVERY_CLEAR,
	  N_("@S needs_recovery flag is clear, but @j has data.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Ask if we should clear the journal */
	{ PR_0_JOURNAL_RESET_JOURNAL,
	  N_("Clear @j"),
	  PROMPT_NULL, PR_PREEN_NOMSG, 0, 0, 0 },

	/* Filesystem revision is 0, but feature flags are set */
	{ PR_0_FS_REV_LEVEL,
	  N_("@f has feature flag(s) set, but is a revision 0 @f.  "),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Clearing orphan inode */
	{ PR_0_ORPHAN_CLEAR_INODE,
	  N_("%s @o @i %i (uid=%Iu, gid=%Ig, mode=%Im, size=%Is)\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Illegal block found in orphaned inode */
	{ PR_0_ORPHAN_ILLEGAL_BLOCK_NUM,
	   N_("@I %B (%b) found in @o @i %i.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Already cleared block found in orphaned inode */
	{ PR_0_ORPHAN_ALREADY_CLEARED_BLOCK,
	   N_("Already cleared %B (%b) found in @o @i %i.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Illegal orphan inode in superblock */
	{ PR_0_ORPHAN_ILLEGAL_HEAD_INODE,
	  /* xgettext:no-c-format */
	  N_("@I @o @i %i in @S.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Illegal inode in orphaned inode list */
	{ PR_0_ORPHAN_ILLEGAL_INODE,
	  /* xgettext:no-c-format */
	  N_("@I @i %i in @o @i list.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Journal superblock has an unknown read-only feature flag set */
	{ PR_0_JOURNAL_UNSUPP_ROCOMPAT,
	  N_("@j @S has an unknown read-only feature flag set.\n"),
	  PROMPT_ABORT, 0, 0, 0, 0 },

	/* Journal superblock has an unknown incompatible feature flag set */
	{ PR_0_JOURNAL_UNSUPP_INCOMPAT,
	  N_("@j @S has an unknown incompatible feature flag set.\n"),
	  PROMPT_ABORT, 0, 0, 0, 0 },

	/* Journal version not supported by this e2fsck */
	{ PR_0_JOURNAL_UNSUPP_VERSION,
	  N_("@j version not supported by this e2fsck.\n"),
	  PROMPT_ABORT, 0, 0, 0, 0 },

	/* Moving journal from /file to hidden inode */
	{ PR_0_MOVE_JOURNAL,
	  /* xgettext:no-c-format */
	  N_("Moving @j from /%s to hidden @i.\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error moving journal to hidden file */
	{ PR_0_ERR_MOVE_JOURNAL,
	  /* xgettext:no-c-format */
	  N_("Error moving @j: %m\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Found invalid V2 journal superblock fields */
	{ PR_0_CLEAR_V2_JOURNAL,
	  N_("Found @n V2 @j @S fields (from V1 @j).\n"
	     "Clearing fields beyond the V1 @j @S...\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Ask if we should run the journal anyway */
	{ PR_0_JOURNAL_RUN,
	  N_("Run @j anyway"),
	  PROMPT_NULL, 0, 0, 0, 0 },

	/* Run the journal by default */
	{ PR_0_JOURNAL_RUN_DEFAULT,
	  N_("Recovery flag not set in backup @S, so running @j anyway.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Backing up journal inode block information */
	{ PR_0_BACKUP_JNL,
	  N_("Backing up @j @i @b information.\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Filesystem does not have resize_inode enabled, but
	 * s_reserved_gdt_blocks is nnnn; should be zero */
	{ PR_0_NONZERO_RESERVED_GDT_BLOCKS,
	  N_("@f does not have resize_@i enabled, but s_reserved_gdt_@bs\n"
	     "is %N; @s zero.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Resize_inode not enabled, but the resize inode is non-zero */
	{ PR_0_CLEAR_RESIZE_INODE,
	  N_("Resize_@i not enabled, but the resize @i is non-zero.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Resize inode not valid */
	{ PR_0_RESIZE_INODE_INVALID,
	  N_("Resize @i not valid.  "),
	  PROMPT_RECREATE, 0, 0, 0, 0 },

	/* Superblock last mount time is in the future */
	{ PR_0_FUTURE_SB_LAST_MOUNT,
	  N_("@S last mount time (%t,\n\tnow = %T) is in the future.\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Superblock last write time is in the future */
	{ PR_0_FUTURE_SB_LAST_WRITE,
	  N_("@S last write time (%t,\n\tnow = %T) is in the future.\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Superblock hint for external superblock should be xxxx */
	{ PR_0_EXTERNAL_JOURNAL_HINT,
	  /* xgettext:no-c-format */
	  N_("@S hint for external superblock @s %X.  "),
	     PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Adding dirhash hint to filesystem */
	{ PR_0_DIRHASH_HINT,
	  N_("Adding dirhash hint to @f.\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* group descriptor N checksum is invalid, should be yyyy. */
	{ PR_0_GDT_CSUM,
	  N_("@g descriptor %g checksum is %04x, should be %04y.  "),
	     PROMPT_FIX, PR_LATCH_BG_CHECKSUM, 0, 0, 0 },

	/* group descriptor N marked uninitialized without feature set. */
	{ PR_0_GDT_UNINIT,
	  /* xgettext:no-c-format */
	  N_("@g descriptor %g marked uninitialized without feature set.\n"),
	     PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Group descriptor N has invalid unused inodes count. */
	{ PR_0_GDT_ITABLE_UNUSED,
	  N_("@g descriptor %g has invalid unused inodes count %b.  "),
	     PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Last group block bitmap uninitialized. */
	{ PR_0_BB_UNINIT_LAST,
	  N_("Last @g @b @B uninitialized.  "),
	     PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Journal transaction was corrupt, replay was aborted */
	{ PR_0_JNL_TXN_CORRUPT,
	  /* xgettext:no-c-format */
	  N_("Journal transaction %i was corrupt, replay was aborted.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* The test_fs filesystem flag is set (and ext4 is available) */
	{ PR_0_CLEAR_TESTFS_FLAG,
	  N_("The test_fs flag is set (and ext4 is available).  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Last mount time is in the future (fudged) */
	{ PR_0_FUTURE_SB_LAST_MOUNT_FUDGED,
	  N_("@S last mount time is in the future.\n\t(by less than a day, "
	     "probably due to the hardware clock being incorrectly set)\n"),
	  PROMPT_NONE, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Last write time is in the future (fudged) */
	{ PR_0_FUTURE_SB_LAST_WRITE_FUDGED,
	  N_("@S last write time is in the future.\n\t(by less than a day, "
	     "probably due to the hardware clock being incorrectly set)\n"),
	  PROMPT_NONE, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* One or more block group descriptor checksums are invalid (latch) */
	{ PR_0_GDT_CSUM_LATCH,
	  N_("One or more @b @g descriptor checksums are invalid.  "),
	     PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Setting free inodes count to right (was wrong) */
	{ PR_0_FREE_INODE_COUNT,
	  N_("Setting free @is count to %j (was %i)\n"),
	  PROMPT_NONE, PR_PREEN_NOMSG, 0, 0, 0 },

	/* Setting free blocks count to right (was wrong) */
	{ PR_0_FREE_BLOCK_COUNT,
	  N_("Setting free @bs count to %c (was %b)\n"),
	  PROMPT_NONE, PR_PREEN_NOMSG, 0, 0, 0 },

	/* Making quota inode hidden */
	{ PR_0_HIDE_QUOTA,
	  N_("Hiding %U @q @i %i (%Q).\n"),
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Superblock has invalid MMP block. */
	{ PR_0_MMP_INVALID_BLK,
	  N_("@S has invalid MMP block.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Superblock has invalid MMP magic. */
	{ PR_0_MMP_INVALID_MAGIC,
	  N_("@S has invalid MMP magic.  "),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Opening file system failed */
	{ PR_0_OPEN_FAILED,
	  /* xgettext:no-c-format */
	  N_("ext2fs_open2: %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Checking group descriptor failed */
	{ PR_0_CHECK_DESC_FAILED,
	  /* xgettext:no-c-format */
	  N_("ext2fs_check_desc: %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Superblock metadata_csum supersedes uninit_bg; both feature
	 * bits cannot be set simultaneously. */
	{ PR_0_META_AND_GDT_CSUM_SET,
	  N_("@S metadata_csum supersedes uninit_bg; both feature "
	     "bits cannot be set simultaneously."),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Superblock MMP block checksum does not match MMP block. */
	{ PR_0_MMP_CSUM_INVALID,
	  N_("@S MMP @b checksum does not match.  "),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Superblock 64bit filesystem needs extents to access the whole disk */
	{ PR_0_64BIT_WITHOUT_EXTENTS,
	  N_("@S 64bit @f needs extents to access the whole disk.  "),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* The first_meta_bg is too big */
	{ PR_0_FIRST_META_BG_TOO_BIG,
	  N_("First_meta_bg is too big.  (%N, max value %g).  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* External journal superblock checksum does not match superblock */
	{ PR_0_EXT_JOURNAL_SUPER_CSUM_INVALID,
	  N_("External @j @S checksum does not match @S.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Superblock metadata_csum_seed means nothing without metadata_csum */
	{ PR_0_CSUM_SEED_WITHOUT_META_CSUM,
	  N_("@S metadata_csum_seed is not necessary without metadata_csum."),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Error initializing quota context */
	{ PR_0_QUOTA_INIT_CTX,
	  /* xgettext:no-c-format */
	  N_("Error initializing quota context in support library: %m\n"),
	  PROMPT_NULL, PR_FATAL, 0, 0, 0 },

	/* Bad required extra isize in superblock */
	{ PR_0_BAD_MIN_EXTRA_ISIZE,
	  N_("Bad required extra isize in @S (%N).  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Bad desired extra isize in superblock */
	{ PR_0_BAD_WANT_EXTRA_ISIZE,
	  N_("Bad desired extra isize in @S (%N).  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Invalid quota inode number */
	{ PR_0_INVALID_QUOTA_INO,
	  N_("Invalid %U @q @i %i.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Too many inodes in the filesystem */
	{ PR_0_INODE_COUNT_BIG,
	  N_("@S would have too many inodes (%N).\n"),
	  PROMPT_NONE, PR_AFTER_CODE, PR_0_SB_CORRUPT, 0, 0 },

	/* Meta_bg and resize_inode are not compatible, disable resize_inode*/
	{ PR_0_DISABLE_RESIZE_INODE,
	  N_("Resize_@i and meta_bg features are enabled. Those features are\n"
	     "not compatible. Resize @i should be disabled.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Pass 1 errors */

	/* Pass 1: Checking inodes, blocks, and sizes */
	{ PR_1_PASS_HEADER,
	  N_("Pass 1: Checking @is, @bs, and sizes\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },

	/* Root inode is not a directory */
	{ PR_1_ROOT_NO_DIR, N_("@r is not a @d.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Root inode has dtime set */
	{ PR_1_ROOT_DTIME,
	  N_("@r has dtime set (probably due to old mke2fs).  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Reserved inode has bad mode */
	{ PR_1_RESERVED_BAD_MODE,
	  N_("Reserved @i %i (%Q) has @n mode.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Deleted inode inum has zero dtime */
	{ PR_1_ZERO_DTIME,
	  /* xgettext:no-c-format */
	  N_("@D @i %i has zero dtime.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Inode inum is in use, but has dtime set */
	{ PR_1_SET_DTIME,
	  /* xgettext:no-c-format */
	  N_("@i %i is in use, but has dtime set.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Inode inum is a zero-length directory */
	{ PR_1_ZERO_LENGTH_DIR,
	  /* xgettext:no-c-format */
	  N_("@i %i is a @z @d.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Group block bitmap at block conflicts with some other fs block */
	{ PR_1_BB_CONFLICT,
	  N_("@g %g's @b @B at %b @C.\n"),
	  PROMPT_RELOCATE, 0, 0, 0, 0 },

	/* Group inode bitmap at block conflicts with some other fs block */
	{ PR_1_IB_CONFLICT,
	  N_("@g %g's @i @B at %b @C.\n"),
	  PROMPT_RELOCATE, 0, 0, 0, 0 },

	/* Group inode table at block conflicts with some other fs block */
	{ PR_1_ITABLE_CONFLICT,
	  N_("@g %g's @i table at %b @C.\n"),
	  PROMPT_RELOCATE, 0, 0, 0, 0 },

	/* Group block bitmap (block) is bad */
	{ PR_1_BB_BAD_BLOCK,
	  N_("@g %g's @b @B (%b) is bad.  "),
	  PROMPT_RELOCATE, 0, 0, 0, 0 },

	/* Group inode bitmap (block) is bad */
	{ PR_1_IB_BAD_BLOCK,
	  N_("@g %g's @i @B (%b) is bad.  "),
	  PROMPT_RELOCATE, 0, 0, 0, 0 },

	/* Inode inum, i_size is small, should be larger */
	{ PR_1_BAD_I_SIZE,
	  N_("@i %i, i_size is %Is, @s %N.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Inode inum, i_blocks is small, should be larger */
	{ PR_1_BAD_I_BLOCKS,
	  N_("@i %i, i_@bs is %Ib, @s %N.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Illegal block number in inode */
	{ PR_1_ILLEGAL_BLOCK_NUM,
	  N_("@I %B (%b) in @i %i.  "),
	  PROMPT_CLEAR, PR_LATCH_BLOCK, 0, 0, 0 },

	/* Block number overlaps filesystem metadata in inode */
	{ PR_1_BLOCK_OVERLAPS_METADATA,
	  N_("%B (%b) overlaps @f metadata in @i %i.  "),
	  PROMPT_CLEAR, PR_LATCH_BLOCK, 0, 0, 0 },

	/* Inode has illegal blocks (latch question) */
	{ PR_1_INODE_BLOCK_LATCH,
	  /* xgettext:no-c-format */
	  N_("@i %i has illegal @b(s).  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Too many illegal blocks in inode */
	{ PR_1_TOO_MANY_BAD_BLOCKS,
	  /* xgettext:no-c-format */
	  N_("Too many illegal @bs in @i %i.\n"),
	  PROMPT_CLEAR_INODE, PR_NO_OK, 0, 0, 0 },

	/* Illegal block number in bad block inode */
	{ PR_1_BB_ILLEGAL_BLOCK_NUM,
	  N_("@I %B (%b) in bad @b @i.  "),
	  PROMPT_CLEAR, PR_LATCH_BBLOCK, 0, 0, 0 },

	/* Bad block inode has illegal blocks (latch question) */
	{ PR_1_INODE_BBLOCK_LATCH,
	  N_("Bad @b @i has illegal @b(s).  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Duplicate or bad blocks in use! */
	{ PR_1_DUP_BLOCKS_PREENSTOP,
	  N_("Duplicate or bad @b in use!\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Bad block number used as bad block inode indirect block */
	{ PR_1_BBINODE_BAD_METABLOCK,
	  N_("Bad @b %b used as bad @b @i indirect @b.  "),
	  PROMPT_CLEAR, PR_LATCH_BBLOCK, 0, 0, 0 },

	/* Inconsistency can't be fixed prompt */
	{ PR_1_BBINODE_BAD_METABLOCK_PROMPT,
	  N_("\nThe bad @b @i has probably been corrupted.  You probably\n"
	     "should stop now and run ""e2fsck -c"" to scan for bad blocks\n"
	     "in the @f.\n"),
	  PROMPT_CONTINUE, PR_PREEN_NOMSG, 0, 0, 0 },

	/* Bad primary block */
	{ PR_1_BAD_PRIMARY_BLOCK,
	  N_("\nIf the @b is really bad, the @f can not be fixed.\n"),
	  PROMPT_NONE, PR_AFTER_CODE, PR_1_BAD_PRIMARY_BLOCK_PROMPT, 0, 0 },

	/* Bad primary block prompt */
	{ PR_1_BAD_PRIMARY_BLOCK_PROMPT,
	  N_("You can remove this @b from the bad @b list and hope\n"
	     "that the @b is really OK.  But there are no guarantees.\n\n"),
	  PROMPT_CLEAR, PR_PREEN_NOMSG, 0, 0, 0 },

	/* The primary superblock block is on the bad block list */
	{ PR_1_BAD_PRIMARY_SUPERBLOCK,
	  N_("The primary @S (%b) is on the bad @b list.\n"),
	  PROMPT_NONE, PR_AFTER_CODE, PR_1_BAD_PRIMARY_BLOCK, 0, 0 },

	/* Bad primary block group descriptors */
	{ PR_1_BAD_PRIMARY_GROUP_DESCRIPTOR,
	  N_("Block %b in the primary @g descriptors "
	     "is on the bad @b list\n"),
	  PROMPT_NONE, PR_AFTER_CODE, PR_1_BAD_PRIMARY_BLOCK, 0, 0 },

	/* Warning: Group number's superblock (block) is bad */
	{ PR_1_BAD_SUPERBLOCK,
	  N_("Warning: Group %g's @S (%b) is bad.\n"),
	  PROMPT_NONE, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Warning: Group number's copy of the group descriptors has a bad
	 * block */
	{ PR_1_BAD_GROUP_DESCRIPTORS,
	  N_("Warning: Group %g's copy of the @g descriptors has a bad "
	  "@b (%b).\n"),
	  PROMPT_NONE, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Block number claimed for no reason in process_bad_blocks */
	{ PR_1_PROGERR_CLAIMED_BLOCK,
	  N_("Programming error?  @b #%b claimed for no reason in "
	  "process_bad_@b.\n"),
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Allocating number contiguous block(s) in block group number */
	{ PR_1_RELOC_BLOCK_ALLOCATE,
	  N_("@A %N contiguous @b(s) in @b @g %g for %s: %m\n"),
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Allocating block buffer for relocating process */
	{ PR_1_RELOC_MEMORY_ALLOCATE,
	  /* xgettext:no-c-format */
	  N_("@A @b buffer for relocating %s\n"),
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Relocating group number's information from X to Y */
	{ PR_1_RELOC_FROM_TO,
	  N_("Relocating @g %g's %s from %b to %c...\n"),
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Relocating group number's information to X */
	{ PR_1_RELOC_TO,
	  /* xgettext:no-c-format */
	  N_("Relocating @g %g's %s to %c...\n"), /* xgettext:no-c-format */
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Warning: could not read block number of relocation process */
	{ PR_1_RELOC_READ_ERR,
	  N_("Warning: could not read @b %b of %s: %m\n"),
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Warning: could not write block number of relocation process */
	{ PR_1_RELOC_WRITE_ERR,
	  N_("Warning: could not write @b %b for %s: %m\n"),
	  PROMPT_NONE, PR_PREEN_OK, 0, 0, 0 },

	/* Error allocating inode bitmap */
	{ PR_1_ALLOCATE_IBITMAP_ERROR,
	  N_("@A @i @B (%N): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error allocating block bitmap */
	{ PR_1_ALLOCATE_BBITMAP_ERROR,
	  N_("@A @b @B (%N): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error allocating icount link information */
	{ PR_1_ALLOCATE_ICOUNT,
	  /* xgettext:no-c-format */
	  N_("@A icount link information: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error allocating directory block array */
	{ PR_1_ALLOCATE_DBCOUNT,
	  /* xgettext:no-c-format */
	  N_("@A @d @b array: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error while scanning inodes */
	{ PR_1_ISCAN_ERROR,
	  /* xgettext:no-c-format */
	  N_("Error while scanning @is (%i): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error while iterating over blocks in inode */
	{ PR_1_BLOCK_ITERATE,
	  /* xgettext:no-c-format */
	  N_("Error while iterating over @bs in @i %i: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error storing inode count information */
	{ PR_1_ICOUNT_STORE,
	  N_("Error storing @i count information (@i=%i, count=%N): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error storing directory block information */
	{ PR_1_ADD_DBLOCK,
	  N_("Error storing @d @b information "
	  "(@i=%i, @b=%b, num=%N): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error reading inode (for clearing) */
	{ PR_1_READ_INODE,
	  /* xgettext:no-c-format */
	  N_("Error reading @i %i: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Suppress messages prompt */
	{ PR_1_SUPPRESS_MESSAGES, "", PROMPT_SUPPRESS, PR_NO_OK, 0, 0, 0 },

	/* Imagic number has imagic flag set when fs doesn't support it */
	{ PR_1_SET_IMAGIC,
	  /* xgettext:no-c-format */
	  N_("@i %i has imagic flag set.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Immutable flag set on a device or socket inode */
	{ PR_1_SET_IMMUTABLE,
	  /* xgettext:no-c-format */
	  N_("Special (@v/socket/fifo/symlink) file (@i %i) has immutable\n"
	     "or append-only flag set.  "),
	  PROMPT_CLEAR, PR_PREEN_OK | PR_PREEN_NO | PR_NO_OK, 0, 0, 0 },

	/* Non-zero size for device, fifo or socket inode */
	{ PR_1_SET_NONZSIZE,
	  /* xgettext:no-c-format */
	  N_("Special (@v/socket/fifo) @i %i has non-zero size.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Filesystem has feature flag(s) set, but is a revision 0 filesystem */
	{ PR_1_FS_REV_LEVEL,
	  N_("@f has feature flag(s) set, but is a revision 0 @f.  "),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Journal inode is not in use, but contains data */
	{ PR_1_JOURNAL_INODE_NOT_CLEAR,
	  N_("@j @i is not in use, but contains data.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Journal is not a regular file */
	{ PR_1_JOURNAL_BAD_MODE,
	  N_("@j is not regular file.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Inode that was part of the orphan list */
	{ PR_1_LOW_DTIME,
	  /* xgettext:no-c-format */
	  N_("@i %i was part of the @o @i list.  "),
	  PROMPT_FIX, PR_LATCH_LOW_DTIME, 0, 0, 0 },

	/* Inodes that were part of a corrupted orphan linked list found
	 * (latch question) */
	{ PR_1_ORPHAN_LIST_REFUGEES,
	  N_("@is that were part of a corrupted orphan linked list found.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Error allocating refcount structure */
	{ PR_1_ALLOCATE_REFCOUNT,
	  N_("@A refcount structure (%N): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error reading extended attribute block */
	{ PR_1_READ_EA_BLOCK,
	  N_("Error reading @a @b %b for @i %i.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Inode number has a bad extended attribute block */
	{ PR_1_BAD_EA_BLOCK,
	  N_("@i %i has a bad @a @b %b.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Error reading Extended Attribute block while fixing refcount */
	{ PR_1_EXTATTR_READ_ABORT,
	  N_("Error reading @a @b %b (%m).  "),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Extended attribute number has reference count incorrect */
	{ PR_1_EXTATTR_REFCOUNT,
	  N_("@a @b %b has reference count %r, @s %N.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Error writing Extended Attribute block while fixing refcount */
	{ PR_1_EXTATTR_WRITE_ABORT,
	  N_("Error writing @a @b %b (%m).  "),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Extended attribute block has h_blocks > 1 */
	{ PR_1_EA_MULTI_BLOCK,
	  N_("@a @b %b has h_@bs > 1.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Allocating extended attribute region allocation structure */
	{ PR_1_EA_ALLOC_REGION_ABORT,
	  N_("@A @a region allocation structure.  "),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Extended Attribute block number is corrupt (allocation collision) */
	{ PR_1_EA_ALLOC_COLLISION,
	  N_("@a @b %b is corrupt (allocation collision).  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Extended attribute block number is corrupt (invalid name) */
	{ PR_1_EA_BAD_NAME,
	  N_("@a @b %b is corrupt (@n name).  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Extended attribute block number is corrupt (invalid value) */
	{ PR_1_EA_BAD_VALUE,
	  N_("@a @b %b is corrupt (@n value).  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Inode number is too big (latch question) */
	{ PR_1_INODE_TOOBIG,
	  /* xgettext:no-c-format */
	  N_("@i %i is too big.  "), PROMPT_TRUNCATE, 0, 0, 0, 0 },

	/* Problem causes directory to be too big */
	{ PR_1_TOOBIG_DIR,
	  N_("%B (%b) causes @d to be too big.  "),
	  PROMPT_CLEAR, PR_LATCH_TOOBIG, 0, 0, 0 },

	/* Problem causes file to be too big */
	{ PR_1_TOOBIG_REG,
	  N_("%B (%b) causes file to be too big.  "),
	  PROMPT_CLEAR, PR_LATCH_TOOBIG, 0, 0, 0 },

	/* Problem causes symlink to be too big */
	{ PR_1_TOOBIG_SYMLINK,
	  N_("%B (%b) causes symlink to be too big.  "),
	  PROMPT_CLEAR, PR_LATCH_TOOBIG, 0, 0, 0 },

	/* Inode has INDEX_FL flag set on filesystem without htree support  */
	{ PR_1_HTREE_SET,
	  /* xgettext:no-c-format */
	  N_("@i %i has INDEX_FL flag set on @f without htree support.\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Inode number has INDEX_FL flag set but is on a directory */
	{ PR_1_HTREE_NODIR,
	  /* xgettext:no-c-format */
	  N_("@i %i has INDEX_FL flag set but is not a @d.\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* htree directory has an invalid root node */
	{ PR_1_HTREE_BADROOT,
	  /* xgettext:no-c-format */
	  N_("@h %i has an @n root node.\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Htree directory has an unsupported hash version */
	{ PR_1_HTREE_HASHV,
	  N_("@h %i has an unsupported hash version (%N)\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Htree directory uses an Incompatible htree root node flag */
	{ PR_1_HTREE_INCOMPAT,
	  /* xgettext:no-c-format */
	  N_("@h %i uses an incompatible htree root node flag.\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Htree directory has a tree depth which is too big */
	{ PR_1_HTREE_DEPTH,
	  N_("@h %i has a tree depth (%N) which is too big\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Bad block inode has an indirect block number that conflicts with
	 * filesystem metadata */
	{ PR_1_BB_FS_BLOCK,
	  N_("Bad @b @i has an indirect @b (%b) that conflicts with\n"
	     "@f metadata.  "),
	  PROMPT_CLEAR, PR_LATCH_BBLOCK, 0, 0, 0 },

	/* Resize inode (re)creation failed */
	{ PR_1_RESIZE_INODE_CREATE,
	  /* xgettext:no-c-format */
	  N_("Resize @i (re)creation failed: %m."),
	  PROMPT_CONTINUE, 0, 0, 0, 0 },

	/* inode has a extra size i_extra_isize which is invalid */
	{ PR_1_EXTRA_ISIZE,
	  N_("@i %i has a extra size (%IS) which is @n\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Extended attribute in inode has a namelen which is invalid */
	{ PR_1_ATTR_NAME_LEN,
	  N_("@a in @i %i has a namelen (%N) which is @n\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Extended attribute in inode has a value offset which is invalid */
	{ PR_1_ATTR_VALUE_OFFSET,
	  N_("@a in @i %i has a value offset (%N) which is @n\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* extended attribute in inode has a value block which is invalid */
	{ PR_1_ATTR_VALUE_BLOCK,
	  N_("@a in @i %i has a value @b (%N) which is @n (must be 0)\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* extended attribute in inode has a value size which is invalid */
	{ PR_1_ATTR_VALUE_SIZE,
	  N_("@a in @i %i has a value size (%N) which is @n\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* extended attribute in inode has a hash which is invalid */
	{ PR_1_ATTR_HASH,
	  N_("@a in @i %i has a hash (%N) which is @n\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* inode is a type but it looks like it is really a directory */
	{ PR_1_TREAT_AS_DIRECTORY,
	  N_("@i %i is a %It but it looks like it is really a directory.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Error while reading extent tree in inode */
	{ PR_1_READ_EXTENT,
	  /* xgettext:no-c-format */
	  N_("Error while reading over @x tree in @i %i: %m\n"),
	  PROMPT_CLEAR_INODE, 0, 0, 0, 0 },

	/* Failure to iterate extents in inode */
	{ PR_1_EXTENT_ITERATE_FAILURE,
	  N_("Failed to iterate extents in @i %i\n"
	     "\t(op %s, blk %b, lblk %c): %m\n"),
	  PROMPT_CLEAR_INODE, 0, 0, 0, 0 },

	/* Inode has an invalid extent starting block */
	{ PR_1_EXTENT_BAD_START_BLK,
	  N_("@i %i has an @n extent\n\t(logical @b %c, @n physical @b %b, len %N)\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Inode has an invalid extent that ends beyond filesystem */
	{ PR_1_EXTENT_ENDS_BEYOND,
	  N_("@i %i has an @n extent\n\t(logical @b %c, physical @b %b, @n len %N)\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* inode has EXTENTS_FL flag set on filesystem without extents support*/
	{ PR_1_EXTENTS_SET,
	  /* xgettext:no-c-format */
	  N_("@i %i has EXTENTS_FL flag set on @f without extents support.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* inode is in extents format, but superblock is missing EXTENTS feature */
	{ PR_1_EXTENT_FEATURE,
	  /* xgettext:no-c-format */
	  N_("@i %i is in extent format, but @S is missing EXTENTS feature\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* inode missing EXTENTS_FL, but is an extent inode */
	{ PR_1_UNSET_EXTENT_FL,
	  /* xgettext:no-c-format */
	  N_("@i %i missing EXTENT_FL, but is in extents format\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Fast symlink has EXTENTS_FL set */
	{ PR_1_FAST_SYMLINK_EXTENT_FL,
	  /* xgettext:no-c-format */
	  N_("Fast symlink %i has EXTENT_FL set.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Extents are out of order */
	{ PR_1_OUT_OF_ORDER_EXTENTS,
	  N_("@i %i has out of order extents\n\t(@n logical @b %c, physical @b %b, len %N)\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	{ PR_1_EXTENT_HEADER_INVALID,
	  N_("@i %i has an invalid extent node (blk %b, lblk %c)\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Failed to convert subcluster block bitmap */
	{ PR_1_CONVERT_SUBCLUSTER,
	  /* xgettext:no-c-format */
	  N_("Error converting subcluster @b @B: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Quota inode is not a regular file */
	{ PR_1_QUOTA_BAD_MODE,
	  N_("@q @i is not a regular file.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Quota inode is not in use, but contains data */
	{ PR_1_QUOTA_INODE_NOT_CLEAR,
	  N_("@q @i is not in use, but contains data.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Quota inode is visible to the user */
	{ PR_1_QUOTA_INODE_NOT_HIDDEN,
	  N_("@q @i is visible to the user.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* The bad block inode looks invalid */
	{ PR_1_INVALID_BAD_INODE,
	  N_("The bad @b @i looks @n.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Extent has zero length extent */
	{ PR_1_EXTENT_LENGTH_ZERO,
	  N_("@i %i has zero length extent\n\t(@n logical @b %c, physical @b %b)\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* inode seems to contain garbage */
	{ PR_1_INODE_IS_GARBAGE,
	  /* xgettext:no-c-format */
	  N_("@i %i seems to contain garbage.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* inode passes checks, but checksum does not match inode */
	{ PR_1_INODE_ONLY_CSUM_INVALID,
	  /* xgettext:no-c-format */
	  N_("@i %i passes checks, but checksum does not match @i.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Inode extended attribute is corrupt (allocation collision) */
	{ PR_1_INODE_EA_ALLOC_COLLISION,
	  /* xgettext:no-c-format */
	  N_("@i %i @a is corrupt (allocation collision).  "),
	  PROMPT_CLEAR, 0, 0, 0, 0},

	/*
	 * Inode extent block passes checks, but checksum does not match
	 * extent
	 */
	{ PR_1_EXTENT_ONLY_CSUM_INVALID,
	  N_("@i %i extent block passes checks, but checksum does not match "
	     "extent\n\t(logical @b %c, physical @b %b, len %N)\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/*
	 * Inode extended attribute block passes checks, but checksum does not
	 * match block.
	 */
	{ PR_1_EA_BLOCK_ONLY_CSUM_INVALID,
	  N_("@i %i @a @b %b passes checks, but checksum does not match @b.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Interior extent node level number of inode doesn't first node down */
	{ PR_1_EXTENT_INDEX_START_INVALID,
	  N_("Interior @x node level %N of @i %i:\n"
	     "Logical start %b does not match logical start %c at next level.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Inode end of extent exceeds allowed value */
	{ PR_1_EXTENT_END_OUT_OF_BOUNDS,
	  N_("@i %i, end of extent exceeds allowed value\n\t(logical @b %c, physical @b %b, len %N)\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Inode has inline data, but superblock is missing INLINE_DATA feature */
	{ PR_1_INLINE_DATA_FEATURE,
	  /* xgettext:no-c-format */
	  N_("@i %i has inline data, but @S is missing INLINE_DATA feature\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* inode has INLINE_DATA_FL flag on filesystem without inline data */
	{ PR_1_INLINE_DATA_SET,
	  /* xgettext:no-c-format */
	  N_("@i %i has INLINE_DATA_FL flag on @f without inline data support.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/*
	 * Inode block conflicts with critical metadata, skipping block checks
	 */
	{ PR_1_CRITICAL_METADATA_COLLISION,
	  /* xgettext:no-c-format */
	  N_("@i %i block %b conflicts with critical metadata, skipping block checks.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Directory inode block <block> should be at block <otherblock> */
	{ PR_1_COLLAPSE_DBLOCK,
	  N_("@d @i %i @b %b should be at @b %c.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Extents/inlinedata flag set on a device or socket inode */
	{ PR_1_UNINIT_DBLOCK,
	  /* xgettext:no-c-format */
	  N_("@d @i %i has @x marked uninitialized at @b %c.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Inode logical block (physical block) violates cluster allocation */
	{ PR_1_MISALIGNED_CLUSTER,
	  N_("@i %i logical @b %b (physical @b %c) violates cluster allocation rules.\nWill fix in pass 1B.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Inode has INLINE_DATA_FL flag but extended attribute not found */
	{ PR_1_INLINE_DATA_NO_ATTR,
	  /* xgettext:no-c-format */
	  N_("@i %i has INLINE_DATA_FL flag but @a not found.  "),
	  PROMPT_TRUNCATE, 0, 0, 0, 0 },

	/* Special (device/socket/fifo) file (inode num) has extents
	 * or inline-data flag set */
	{ PR_1_SPECIAL_EXTENTS_IDATA,
	  /* xgettext:no-c-format */
	  N_("Special (@v/socket/fifo) file (@i %i) has extents\n"
	     "or inline-data flag set.  "),
	  PROMPT_CLEAR, PR_PREEN_OK | PR_PREEN_NO | PR_NO_OK, 0, 0, 0 },

	/* Inode has extent header but inline data flag is set */
	{ PR_1_CLEAR_INLINE_DATA_FOR_EXTENT,
	  /* xgettext:no-c-format */
	  N_("@i %i has @x header but inline data flag is set.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Inode seems to have inline data but extent flag is set */
	{ PR_1_CLEAR_EXTENT_FOR_INLINE_DATA,
	  /* xgettext:no-c-format */
	  N_("@i %i seems to have inline data but @x flag is set.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Inode seems to have block map but inline data and extent flags set */
	{ PR_1_CLEAR_EXTENT_INLINE_DATA_FLAGS,
	  /* xgettext:no-c-format */
	  N_("@i %i seems to have @b map but inline data and @x flags set.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Inode has inline data and extent flags but i_block contains junk */
	{ PR_1_CLEAR_EXTENT_INLINE_DATA_INODE,
	  /* xgettext:no-c-format */
	  N_("@i %i has inline data and @x flags set but i_block contains junk.\n"),
	  PROMPT_CLEAR_INODE, 0, 0, 0, 0 },

	/* Bad block list says the bad block list inode is bad */
	{ PR_1_BADBLOCKS_IN_BADBLOCKS,
	  N_("Bad block list says the bad block list @i is bad.  "),
	  PROMPT_CLEAR_INODE, 0, 0, 0, 0 },

	/* Error allocating extent region allocation structure */
	{ PR_1_EXTENT_ALLOC_REGION_ABORT,
	  N_("@A @x region allocation structure.  "),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Inode leaf has a duplicate extent mapping */
	{ PR_1_EXTENT_COLLISION,
	  N_("@i %i has a duplicate @x mapping\n\t(logical @b %c, @n physical @b %b, len %N)\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Error allocating memory for encrypted directory list */
	{ PR_1_ALLOCATE_ENCRYPTED_DIRLIST,
	  N_("@A memory for encrypted @d list\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Inode extent tree could be more shallow */
	{ PR_1_EXTENT_BAD_MAX_DEPTH,
	  N_("@i %i @x tree could be more shallow (%b; could be <= %c)\n"),
	  PROMPT_FIX, PR_NO_OK | PR_PREEN_NO | PR_PREEN_OK, 0, 0, 0 },

	/* inode num on bigalloc filesystem cannot be block mapped */
	{ PR_1_NO_BIGALLOC_BLOCKMAP_FILES,
	  /* xgettext:no-c-format */
	  N_("@i %i on bigalloc @f cannot be @b mapped.  "),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Inode has corrupt extent header */
	{ PR_1_MISSING_EXTENT_HEADER,
	  /* xgettext:no-c-format */
	  N_("@i %i has corrupt @x header.  "),
	  PROMPT_CLEAR_INODE, 0, 0, 0, 0 },

	/* Timestamp(s) on inode beyond 2310-04-04 are likely pre-1970. */
	{ PR_1_EA_TIME_OUT_OF_RANGE,
	  /* xgettext:no-c-format */
	  N_("Timestamp(s) on @i %i beyond 2310-04-04 are likely pre-1970.\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Inode has illegal extended attribute value inode */
	{ PR_1_ATTR_VALUE_EA_INODE,
	  N_("@i %i has @I @a value @i %N.\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Inode has invalid extended attribute. EA inode missing
	 * EA_INODE flag. */
	{ PR_1_ATTR_NO_EA_INODE_FL,
	  N_("@i %i has @n @a. EA @i %N missing EA_INODE flag.\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* EA inode for parent inode missing EA_INODE flag. */
	{ PR_1_ATTR_SET_EA_INODE_FL,
	  N_("EA @i %N for parent @i %i missing EA_INODE flag.\n "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Casefold flag set on a non-directory */
	{ PR_1_CASEFOLD_NONDIR,
	  N_("@i %i has the casefold flag set but is not a directory.  "),
	  PROMPT_CLEAR_FLAG, 0, 0, 0, 0 },

	/* Casefold flag set, but file system is missing the casefold feature */
	{ PR_1_CASEFOLD_FEATURE,
	  N_("@d %p has the casefold flag, but the\ncasefold feature is not enabled.  "),
	  PROMPT_CLEAR_FLAG, 0, 0, 0, 0 },

	/* Pass 1b errors */

	/* Pass 1B: Rescan for duplicate/bad blocks */
	{ PR_1B_PASS_HEADER,
	  N_("\nRunning additional passes to resolve @bs claimed by more than one @i...\n"
	  "Pass 1B: Rescanning for @m @bs\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },

	/* Duplicate/bad block(s) header */
	{ PR_1B_DUP_BLOCK_HEADER,
	  /* xgettext:no-c-format */
	  N_("@m @b(s) in @i %i:"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Duplicate/bad block(s) in inode */
	{ PR_1B_DUP_BLOCK,
	  " %b",
	  PROMPT_NONE, PR_LATCH_DBLOCK | PR_PREEN_NOHDR, 0, 0, 0 },

	/* Duplicate/bad block(s) end */
	{ PR_1B_DUP_BLOCK_END,
	  "\n",
	  PROMPT_NONE, PR_PREEN_NOHDR, 0, 0, 0 },

	/* Error while scanning inodes */
	{ PR_1B_ISCAN_ERROR,
	  /* xgettext:no-c-format */
	  N_("Error while scanning inodes (%i): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error allocating inode bitmap */
	{ PR_1B_ALLOCATE_IBITMAP_ERROR,
	  /* xgettext:no-c-format */
	  N_("@A @i @B (@i_dup_map): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error while iterating over blocks */
	{ PR_1B_BLOCK_ITERATE,
	  /* xgettext:no-c-format */
	  N_("Error while iterating over @bs in @i %i (%s): %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error adjusting EA refcount */
	{ PR_1B_ADJ_EA_REFCOUNT,
	  N_("Error adjusting refcount for @a @b %b (@i %i): %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Duplicate/bad block range in inode */
	{ PR_1B_DUP_RANGE,
	  " %b--%c",
	  PROMPT_NONE, PR_LATCH_DBLOCK | PR_PREEN_NOHDR, 0, 0, 0 },

	/* Pass 1C: Scan directories for inodes with multiply-claimed blocks. */
	{ PR_1C_PASS_HEADER,
	  N_("Pass 1C: Scanning directories for @is with @m @bs\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },


	/* Pass 1D: Reconciling multiply-claimed blocks */
	{ PR_1D_PASS_HEADER,
	  N_("Pass 1D: Reconciling @m @bs\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },

	/* File has duplicate blocks */
	{ PR_1D_DUP_FILE,
	  N_("File %Q (@i #%i, mod time %IM) \n"
	  "  has %r @m @b(s), shared with %N file(s):\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* List of files sharing duplicate blocks */
	{ PR_1D_DUP_FILE_LIST,
	  N_("\t%Q (@i #%i, mod time %IM)\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* File sharing blocks with filesystem metadata  */
	{ PR_1D_SHARE_METADATA,
	  N_("\t<@f metadata>\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Report of how many duplicate/bad inodes */
	{ PR_1D_NUM_DUP_INODES,
	  N_("(There are %N @is containing @m @bs.)\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Duplicated blocks already reassigned or cloned. */
	{ PR_1D_DUP_BLOCKS_DEALT,
	  N_("@m @bs already reassigned or cloned.\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Clone duplicate/bad blocks? */
	{ PR_1D_CLONE_QUESTION,
	  "", PROMPT_CLONE, PR_NO_OK, 0, 0, 0 },

	/* Delete file? */
	{ PR_1D_DELETE_QUESTION,
	  "", PROMPT_DELETE, 0, 0, 0, 0 },

	/* Couldn't clone file (error) */
	{ PR_1D_CLONE_ERROR,
	  /* xgettext:no-c-format */
	  N_("Couldn't clone file: %m\n"), PROMPT_NONE, 0, 0, 0, 0 },

	/* Pass 1E Extent tree optimization	*/

	/* Pass 1E: Optimizing extent trees */
	{ PR_1E_PASS_HEADER,
	  N_("Pass 1E: Optimizing @x trees\n"),
	  PROMPT_NONE, PR_HEADER | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Failed to optimize extent tree */
	{ PR_1E_OPTIMIZE_EXT_ERR,
	  /* xgettext:no-c-format */
	  N_("Failed to optimize @x tree %p (%i): %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Optimizing extent trees */
	{ PR_1E_OPTIMIZE_EXT_HEADER,
	  N_("Optimizing @x trees: "),
	  PROMPT_NONE, PR_MSG_ONLY, 0, 0, 0 },

	/* Rebuilding extent tree %d */
	{ PR_1E_OPTIMIZE_EXT,
	  " %i",
	  PROMPT_NONE, PR_LATCH_OPTIMIZE_EXT | PR_PREEN_NOHDR, 0, 0, 0 },

	/* Rebuilding extent tree end */
	{ PR_1E_OPTIMIZE_EXT_END,
	  "\n",
	  PROMPT_NONE, PR_PREEN_NOHDR, 0, 0, 0 },

	/* Internal error: extent tree depth too large */
	{ PR_1E_MAX_EXTENT_TREE_DEPTH,
	  N_("Internal error: max extent tree depth too large (%b; expected=%c).\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Inode extent tree could be shorter */
	{ PR_1E_CAN_COLLAPSE_EXTENT_TREE,
	  N_("@i %i @x tree (at level %b) could be shorter.  "),
	  PROMPT_OPTIMIZE, PR_NO_OK | PR_PREEN_NO | PR_PREEN_OK | PR_NOT_A_FIX, 0, 0, 0 },

	/* Inode extent tree could be narrower */
	{ PR_1E_CAN_NARROW_EXTENT_TREE,
	  N_("@i %i @x tree (at level %b) could be narrower.  "),
	  PROMPT_OPTIMIZE, PR_NO_OK | PR_PREEN_NO | PR_PREEN_OK | PR_NOT_A_FIX, 0, 0, 0 },

	/* Pass 2 errors */

	/* Pass 2: Checking directory structure */
	{ PR_2_PASS_HEADER,
	  N_("Pass 2: Checking @d structure\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },

	/* Bad inode number for '.' */
	{ PR_2_BAD_INODE_DOT,
	  /* xgettext:no-c-format */
	  N_("@n @i number for '.' in @d @i %i.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Entry 'xxxx' in /a/b/c has bad inode number.*/
	{ PR_2_BAD_INO,
	  N_("@E has @n @i #: %Di.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Entry 'xxxx' in /a/b/c has deleted/unused inode nnnnn.*/
	{ PR_2_UNUSED_INODE,
	  N_("@E has @D/unused @i %Di.  "),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Directory entry is link to '.' */
	{ PR_2_LINK_DOT,
	  N_("@E @L to '.'  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Directory entry points to inode now located in a bad block */
	{ PR_2_BB_INODE,
	  N_("@E points to @i (%Di) located in a bad @b.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Directory entry contains a link to a directory */
	{ PR_2_LINK_DIR,
	  N_("@E @L to @d %P (%Di).\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Directory entry contains a link to the root directory */
	{ PR_2_LINK_ROOT,
	  N_("@E @L to the @r.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Directory entry has illegal characters in its name */
	{ PR_2_BAD_NAME,
	  N_("@E has illegal characters in its name.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Missing '.' in directory inode */
	{ PR_2_MISSING_DOT,
	  /* xgettext:no-c-format */
	  N_("Missing '.' in @d @i %i.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Missing '..' in directory inode */
	{ PR_2_MISSING_DOT_DOT,
	  /* xgettext:no-c-format */
	  N_("Missing '..' in @d @i %i.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* First entry in directory inode doesn't contain '.' */
	{ PR_2_1ST_NOT_DOT,
	  N_("First @e '%Dn' (@i=%Di) in @d @i %i (%p) @s '.'\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Second entry in directory inode doesn't contain '..' */
	{ PR_2_2ND_NOT_DOT_DOT,
	  N_("Second @e '%Dn' (@i=%Di) in @d @i %i @s '..'\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* i_faddr should be zero */
	{ PR_2_FADDR_ZERO,
	  N_("i_faddr @F %IF, @s zero.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* i_file_acl should be zero */
	{ PR_2_FILE_ACL_ZERO,
	  N_("i_file_acl @F %If, @s zero.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* i_size_high should be zero */
	{ PR_2_DIR_SIZE_HIGH_ZERO,
	  N_("i_size_high @F %Id, @s zero.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* i_frag should be zero */
	{ PR_2_FRAG_ZERO,
	  N_("i_frag @F %N, @s zero.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* i_fsize should be zero */
	{ PR_2_FSIZE_ZERO,
	  N_("i_fsize @F %N, @s zero.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* inode has bad mode */
	{ PR_2_BAD_MODE,
	  N_("@i %i (%Q) has @n mode (%Im).\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* directory corrupted */
	{ PR_2_DIR_CORRUPTED,
	  N_("@d @i %i, %B, offset %N: @d corrupted\n"),
	  PROMPT_SALVAGE, 0, 0, 0, 0 },

	/* filename too long */
	{ PR_2_FILENAME_LONG,
	  N_("@d @i %i, %B, offset %N: filename too long\n"),
	  PROMPT_TRUNCATE, 0, 0, 0, 0 },

	/* Directory inode has a missing block (hole) */
	{ PR_2_DIRECTORY_HOLE,
	  N_("@d @i %i has an unallocated %B.  "),
	  PROMPT_ALLOCATE, 0, 0, 0, 0 },

	/* '.' is not NULL terminated */
	{ PR_2_DOT_NULL_TERM,
	  /* xgettext:no-c-format */
	  N_("'.' @d @e in @d @i %i is not NULL terminated\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* '..' is not NULL terminated */
	{ PR_2_DOT_DOT_NULL_TERM,
	  /* xgettext:no-c-format */
	  N_("'..' @d @e in @d @i %i is not NULL terminated\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Illegal character device inode */
	{ PR_2_BAD_CHAR_DEV,
	  N_("@i %i (%Q) is an @I character @v.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Illegal block device inode */
	{ PR_2_BAD_BLOCK_DEV,
	  N_("@i %i (%Q) is an @I @b @v.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Duplicate '.' entry */
	{ PR_2_DUP_DOT,
	  N_("@E is duplicate '.' @e.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Duplicate '..' entry */
	{ PR_2_DUP_DOT_DOT,
	  N_("@E is duplicate '..' @e.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Internal error: couldn't find dir_info */
	{ PR_2_NO_DIRINFO,
	  /* xgettext:no-c-format */
	  N_("Internal error: couldn't find dir_info for %i.\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Final rec_len is wrong */
	{ PR_2_FINAL_RECLEN,
	  N_("@E has rec_len of %Dr, @s %N.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Error allocating icount structure */
	{ PR_2_ALLOCATE_ICOUNT,
	  /* xgettext:no-c-format */
	  N_("@A icount structure: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error iterating over directory blocks */
	{ PR_2_DBLIST_ITERATE,
	  /* xgettext:no-c-format */
	  N_("Error iterating over @d @bs: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error reading directory block */
	{ PR_2_READ_DIRBLOCK,
	  N_("Error reading @d @b %b (@i %i): %m\n"),
	  PROMPT_CONTINUE, 0, 0, 0, 0 },

	/* Error writing directory block */
	{ PR_2_WRITE_DIRBLOCK,
	  N_("Error writing @d @b %b (@i %i): %m\n"),
	  PROMPT_CONTINUE, 0, 0, 0, 0 },

	/* Error allocating new directory block */
	{ PR_2_ALLOC_DIRBOCK,
	  /* xgettext:no-c-format */
	  N_("@A new @d @b for @i %i (%s): %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error deallocating inode */
	{ PR_2_DEALLOC_INODE,
	  /* xgettext:no-c-format */
	  N_("Error deallocating @i %i: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Directory entry for '.' is big.  Split? */
	{ PR_2_SPLIT_DOT,
	  /* xgettext:no-c-format */
	  N_("@d @e for '.' in %p (%i) is big.\n"),
	  PROMPT_SPLIT, PR_NO_OK, 0, 0, 0 },

	/* Illegal FIFO inode */
	{ PR_2_BAD_FIFO,
	  N_("@i %i (%Q) is an @I FIFO.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Illegal socket inode */
	{ PR_2_BAD_SOCKET,
	  N_("@i %i (%Q) is an @I socket.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Directory filetype not set */
	{ PR_2_SET_FILETYPE,
	  N_("Setting filetype for @E to %N.\n"),
	  PROMPT_NONE, PR_PREEN_OK | PR_NO_OK | PR_NO_NOMSG, 0, 0, 0 },

	/* Directory filetype incorrect */
	{ PR_2_BAD_FILETYPE,
	  N_("@E has an incorrect filetype (was %Dt, @s %N).\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Directory filetype set on filesystem */
	{ PR_2_CLEAR_FILETYPE,
	  N_("@E has filetype set.\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* Directory filename is null */
	{ PR_2_NULL_NAME,
	  N_("@E has a @z name.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Invalid symlink */
	{ PR_2_INVALID_SYMLINK,
	  N_("Symlink %Q (@i #%i) is @n.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* i_file_acl (extended attribute block) is bad */
	{ PR_2_FILE_ACL_BAD,
	  N_("@a @b @F @n (%If).\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Filesystem contains large files, but has no such flag in sb */
	{ PR_2_FEATURE_LARGE_FILES,
	  N_("@f contains large files, but lacks LARGE_FILE flag in @S.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Node in HTREE directory not referenced */
	{ PR_2_HTREE_NOTREF,
	  N_("@p @h %d: %B not referenced\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Node in HTREE directory referenced twice */
	{ PR_2_HTREE_DUPREF,
	  N_("@p @h %d: %B referenced twice\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Node in HTREE directory has bad min hash */
	{ PR_2_HTREE_MIN_HASH,
	  N_("@p @h %d: %B has bad min hash\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Node in HTREE directory has bad max hash */
	{ PR_2_HTREE_MAX_HASH,
	  N_("@p @h %d: %B has bad max hash\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Clear invalid HTREE directory */
	{ PR_2_HTREE_CLEAR,
	  N_("@n @h %d (%q).  "), PROMPT_CLEAR_HTREE, 0, 0, 0, 0 },

	/* Bad block in htree interior node */
	{ PR_2_HTREE_BADBLK,
	  N_("@p @h %d (%q): bad @b number %b.\n"),
	  PROMPT_CLEAR_HTREE, 0, 0, 0, 0 },

	/* Error adjusting EA refcount */
	{ PR_2_ADJ_EA_REFCOUNT,
	  N_("Error adjusting refcount for @a @b %b (@i %i): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Invalid HTREE root node */
	{ PR_2_HTREE_BAD_ROOT,
	  /* xgettext:no-c-format */
	  N_("@p @h %d: root node is @n\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Invalid HTREE limit */
	{ PR_2_HTREE_BAD_LIMIT,
	  N_("@p @h %d: %B has @n limit (%N)\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Invalid HTREE count */
	{ PR_2_HTREE_BAD_COUNT,
	  N_("@p @h %d: %B has @n count (%N)\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* HTREE interior node has out-of-order hashes in table */
	{ PR_2_HTREE_HASH_ORDER,
	  N_("@p @h %d: %B has an unordered hash table\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* Node in HTREE directory has invalid depth */
	{ PR_2_HTREE_BAD_DEPTH,
	  N_("@p @h %d: %B has @n depth (%N)\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Duplicate directory entry found */
	{ PR_2_DUPLICATE_DIRENT,
	  N_("Duplicate @E found.  "),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Non-unique filename found */
	{ PR_2_NON_UNIQUE_FILE, /* xgettext: no-c-format */
	  N_("@E has a non-unique filename.\nRename to %s"),
	  PROMPT_NULL, 0, 0, 0, 0 },

	/* Duplicate directory entry found */
	{ PR_2_REPORT_DUP_DIRENT,
	  N_("Duplicate @e '%Dn' found.\n\tMarking %p (%i) to be rebuilt.\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* i_blocks_hi should be zero */
	{ PR_2_BLOCKS_HI_ZERO,
	  N_("i_blocks_hi @F %N, @s zero.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Unexpected HTREE block */
	{ PR_2_UNEXPECTED_HTREE_BLOCK,
	  N_("Unexpected @b in @h %d (%q).\n"), PROMPT_CLEAR_HTREE, 0,
	  0, 0, 0 },

	/* Inode found in group where _INODE_UNINIT is set */
	{ PR_2_INOREF_BG_INO_UNINIT,
	  N_("@E references @i %Di in @g %g where _INODE_UNINIT is set.\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Inode found in group unused inodes area */
	{ PR_2_INOREF_IN_UNUSED,
	  N_("@E references @i %Di found in @g %g's unused inodes area.\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* i_blocks_hi should be zero */
	{ PR_2_I_FILE_ACL_HI_ZERO,
	  N_("i_file_acl_hi @F %N, @s zero.\n"),
	  PROMPT_CLEAR, PR_PREEN_OK, 0, 0, 0 },

	/* htree root node fails checksum */
	{ PR_2_HTREE_ROOT_CSUM_INVALID,
	  /* xgettext:no-c-format */
	  N_("@p @h %d: root node fails checksum.\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* htree internal node fails checksum */
	{ PR_2_HTREE_NODE_CSUM_INVALID,
	  /* xgettext:no-c-format */
	  N_("@p @h %d: internal node fails checksum.\n"),
	  PROMPT_CLEAR_HTREE, PR_PREEN_OK, 0, 0, 0 },

	/* leaf node has no checksum */
	{ PR_2_LEAF_NODE_MISSING_CSUM,
	  N_("@d @i %i, %B, offset %N: @d has no checksum.\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* leaf node passes checks but fails checksum */
	{ PR_2_LEAF_NODE_ONLY_CSUM_INVALID,
	  N_("@d @i %i, %B: @d passes checks but fails checksum.\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* inline directory inode size must be a multiple of 4 */
	{ PR_2_BAD_INLINE_DIR_SIZE,
	  N_("Inline @d @i %i size (%N) must be a multiple of 4.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* fixing size of inline directory inode failed */
	{ PR_2_FIX_INLINE_DIR_FAILED,
	  /* xgettext:no-c-format */
	  N_("Fixing size of inline @d @i %i failed.\n"),
	  PROMPT_TRUNCATE, 0, 0, 0, 0 },

	/* Encrypted directory entry is too short */
	{ PR_2_BAD_ENCRYPTED_NAME,
	  N_("Encrypted @E is too short.\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Pass 3 errors */

	/* Pass 3: Checking directory connectivity */
	{ PR_3_PASS_HEADER,
	  N_("Pass 3: Checking @d connectivity\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },

	/* Root inode not allocated */
	{ PR_3_NO_ROOT_INODE,
	  N_("@r not allocated.  "),
	  PROMPT_ALLOCATE, 0, 0, 0, 0 },

	/* No room in lost+found */
	{ PR_3_EXPAND_LF_DIR,
	  N_("No room in @l @d.  "),
	  PROMPT_EXPAND, 0, 0, 0, 0 },

	/* Unconnected directory inode */
	{ PR_3_UNCONNECTED_DIR,
	  /* xgettext:no-c-format */
	  N_("Unconnected @d @i %i (%p)\n"),
	  PROMPT_CONNECT, 0, 0, 0, 0 },

	/* /lost+found not found */
	{ PR_3_NO_LF_DIR,
	  N_("/@l not found.  "),
	  PROMPT_CREATE, PR_PREEN_OK, 0, 0, 0 },

	/* .. entry is incorrect */
	{ PR_3_BAD_DOT_DOT,
	  N_("'..' in %Q (%i) is %P (%j), @s %q (%d).\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Bad or non-existent /lost+found.  Cannot reconnect */
	{ PR_3_NO_LPF,
	  /* xgettext:no-c-format */
	  N_("Bad or non-existent /@l.  Cannot reconnect.\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Could not expand /lost+found */
	{ PR_3_CANT_EXPAND_LPF,
	  /* xgettext:no-c-format */
	  N_("Could not expand /@l: %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Could not reconnect inode */
	{ PR_3_CANT_RECONNECT,
	  /* xgettext:no-c-format */
	  N_("Could not reconnect %i: %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error while trying to find /lost+found */
	{ PR_3_ERR_FIND_LPF,
	  /* xgettext:no-c-format */
	  N_("Error while trying to find /@l: %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error in ext2fs_new_block while creating /lost+found */
	{ PR_3_ERR_LPF_NEW_BLOCK,
	  /* xgettext:no-c-format */
	  N_("ext2fs_new_@b: %m while trying to create /@l @d\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error in ext2fs_new_inode while creating /lost+found */
	{ PR_3_ERR_LPF_NEW_INODE,
	  /* xgettext:no-c-format */
	  N_("ext2fs_new_@i: %m while trying to create /@l @d\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error in ext2fs_new_dir_block while creating /lost+found */
	{ PR_3_ERR_LPF_NEW_DIR_BLOCK,
	  /* xgettext:no-c-format */
	  N_("ext2fs_new_dir_@b: %m while creating new @d @b\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error while writing directory block for /lost+found */
	{ PR_3_ERR_LPF_WRITE_BLOCK,
	  /* xgettext:no-c-format */
	  N_("ext2fs_write_dir_@b: %m while writing the @d @b for /@l\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error while adjusting inode count */
	{ PR_3_ADJUST_INODE,
	  /* xgettext:no-c-format */
	  N_("Error while adjusting @i count on @i %i\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Couldn't fix parent directory -- error */
	{ PR_3_FIX_PARENT_ERR,
	  /* xgettext:no-c-format */
	  N_("Couldn't fix parent of @i %i: %m\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Couldn't fix parent directory -- couldn't find it */
	{ PR_3_FIX_PARENT_NOFIND,
	  /* xgettext:no-c-format */
	  N_("Couldn't fix parent of @i %i: Couldn't find parent @d @e\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error allocating inode bitmap */
	{ PR_3_ALLOCATE_IBITMAP_ERROR,
	  N_("@A @i @B (%N): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error creating root directory */
	{ PR_3_CREATE_ROOT_ERROR,
	  /* xgettext:no-c-format */
	  N_("Error creating root @d (%s): %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error creating lost and found directory */
	{ PR_3_CREATE_LPF_ERROR,
	  /* xgettext:no-c-format */
	  N_("Error creating /@l @d (%s): %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Root inode is not directory; aborting */
	{ PR_3_ROOT_NOT_DIR_ABORT,
	  N_("@r is not a @d; aborting.\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Cannot proceed without a root inode. */
	{ PR_3_NO_ROOT_INODE_ABORT,
	  N_("Cannot proceed without a @r.\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Internal error: couldn't find dir_info */
	{ PR_3_NO_DIRINFO,
	  N_("Internal error: couldn't find dir_info for %i.\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Lost+found not a directory */
	{ PR_3_LPF_NOTDIR,
	  /* xgettext:no-c-format */
	  N_("/@l is not a @d (ino=%i)\n"),
	  PROMPT_UNLINK, 0, 0, 0, 0 },

	/* Lost+found has inline data */
	{ PR_3_LPF_INLINE_DATA,
	  N_("/@l has inline data\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Cannot allocate /lost+found. */
	{ PR_3_LPF_NO_SPACE,
	  N_("Cannot allocate space for /@l.\nPlace lost files in root directory instead"),
	  PROMPT_NULL, 0, 0, 0, 0 },

	/* Delete some files and re-run e2fsck. */
	{ PR_3_NO_SPACE_TO_RECOVER,
	  N_("Insufficient space to recover lost files!\nMove data off the @f and re-run e2fsck.\n\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Lost+found is encrypted */
	{ PR_3_LPF_ENCRYPTED,
	  N_("/@l is encrypted\n"),
	  PROMPT_CLEAR, 0, 0, 0, 0 },

	/* Pass 3A Directory Optimization	*/

	/* Pass 3A: Optimizing directories */
	{ PR_3A_PASS_HEADER,
	  N_("Pass 3A: Optimizing directories\n"),
	  PROMPT_NONE, PR_HEADER | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Error iterating over directories */
	{ PR_3A_OPTIMIZE_ITER,
	  /* xgettext:no-c-format */
	  N_("Failed to create dirs_to_hash iterator: %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Error rehash directory */
	{ PR_3A_OPTIMIZE_DIR_ERR,
	  N_("Failed to optimize directory %q (%d): %m\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	/* Rehashing dir header */
	{ PR_3A_OPTIMIZE_DIR_HEADER,
	  N_("Optimizing directories: "),
	  PROMPT_NONE, PR_MSG_ONLY, 0, 0, 0 },

	/* Rehashing directory %d */
	{ PR_3A_OPTIMIZE_DIR,
	  " %d",
	  PROMPT_NONE, PR_LATCH_OPTIMIZE_DIR | PR_PREEN_NOHDR, 0, 0, 0 },

	/* Rehashing dir end */
	{ PR_3A_OPTIMIZE_DIR_END,
	  "\n",
	  PROMPT_NONE, PR_PREEN_NOHDR, 0, 0, 0 },

	/* Pass 4 errors */

	/* Pass 4: Checking reference counts */
	{ PR_4_PASS_HEADER,
	  N_("Pass 4: Checking reference counts\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },

	/* Unattached zero-length inode */
	{ PR_4_ZERO_LEN_INODE,
	  /* xgettext:no-c-format */
	  N_("@u @z @i %i.  "),
	  PROMPT_CLEAR, PR_PREEN_OK|PR_NO_OK, 0, 0, 0 },

	/* Unattached inode */
	{ PR_4_UNATTACHED_INODE,
	  /* xgettext:no-c-format */
	  N_("@u @i %i\n"),
	  PROMPT_CONNECT, 0, 0, 0, 0 },

	/* Inode ref count wrong */
	{ PR_4_BAD_REF_COUNT,
	  N_("@i %i ref count is %Il, @s %N.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	{ PR_4_INCONSISTENT_COUNT,
	  N_("WARNING: PROGRAMMING BUG IN E2FSCK!\n"
	  "\tOR SOME BONEHEAD (YOU) IS CHECKING A MOUNTED (LIVE) FILESYSTEM.\n"
	  "@i_link_info[%i] is %N, @i.i_links_count is %Il.  "
	  "They @s the same!\n"),
	  PROMPT_NONE, 0, 0, 0, 0 },

	{ PR_4_EA_INODE_REF_COUNT,
	  N_("@a @i %i ref count is %N, @s %n. "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* directory exceeds max links, but no DIR_NLINK feature in superblock*/
	{ PR_4_DIR_NLINK_FEATURE,
	  N_("@d exceeds max links, but no DIR_NLINK feature in @S.\n"),
	  PROMPT_FIX, 0, 0, 0, 0 },

	/* Directory inode ref count set to overflow but could be exact value */
	{ PR_4_DIR_OVERFLOW_REF_COUNT,
	  N_("@d @i %i ref count set to overflow but could be exact value %N.  "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Pass 5 errors */

	/* Pass 5: Checking group summary information */
	{ PR_5_PASS_HEADER,
	  N_("Pass 5: Checking @g summary information\n"),
	  PROMPT_NONE, PR_HEADER, 0, 0, 0 },

	/* Padding at end of inode bitmap is not set. */
	{ PR_5_INODE_BMAP_PADDING,
	  N_("Padding at end of @i @B is not set. "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Padding at end of block bitmap is not set. */
	{ PR_5_BLOCK_BMAP_PADDING,
	  N_("Padding at end of @b @B is not set. "),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Block bitmap differences header */
	{ PR_5_BLOCK_BITMAP_HEADER,
	  N_("@b @B differences: "),
	  PROMPT_NONE, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Block not used, but marked in bitmap */
	{ PR_5_BLOCK_UNUSED,
	  " -%b",
	  PROMPT_NONE, PR_LATCH_BBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Block used, but not marked used in bitmap */
	{ PR_5_BLOCK_USED,
	  " +%b",
	  PROMPT_NONE, PR_LATCH_BBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Block bitmap differences end */
	{ PR_5_BLOCK_BITMAP_END,
	  "\n",
	  PROMPT_FIX, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Inode bitmap differences header */
	{ PR_5_INODE_BITMAP_HEADER,
	  N_("@i @B differences: "),
	  PROMPT_NONE, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Inode not used, but marked in bitmap */
	{ PR_5_INODE_UNUSED,
	  " -%i",
	  PROMPT_NONE, PR_LATCH_IBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Inode used, but not marked used in bitmap */
	{ PR_5_INODE_USED,
	  " +%i",
	  PROMPT_NONE, PR_LATCH_IBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Inode bitmap differences end */
	{ PR_5_INODE_BITMAP_END,
	  "\n",
	  PROMPT_FIX, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Free inodes count for group wrong */
	{ PR_5_FREE_INODE_COUNT_GROUP,
	  N_("Free @is count wrong for @g #%g (%i, counted=%j).\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Directories count for group wrong */
	{ PR_5_FREE_DIR_COUNT_GROUP,
	  N_("Directories count wrong for @g #%g (%i, counted=%j).\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Free inodes count wrong */
	{ PR_5_FREE_INODE_COUNT,
	  N_("Free @is count wrong (%i, counted=%j).\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Free blocks count for group wrong */
	{ PR_5_FREE_BLOCK_COUNT_GROUP,
	  N_("Free @bs count wrong for @g #%g (%b, counted=%c).\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Free blocks count wrong */
	{ PR_5_FREE_BLOCK_COUNT,
	  N_("Free @bs count wrong (%b, counted=%c).\n"),
	  PROMPT_FIX, PR_PREEN_OK | PR_NO_OK | PR_PREEN_NOMSG, 0, 0, 0 },

	/* Programming error: bitmap endpoints don't match */
	{ PR_5_BMAP_ENDPOINTS,
	  N_("PROGRAMMING ERROR: @f (#%N) @B endpoints (%b, %c) don't "
	  "match calculated @B endpoints (%i, %j)\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Internal error: fudging end of bitmap */
	{ PR_5_FUDGE_BITMAP_ERROR,
	  N_("Internal error: fudging end of bitmap (%N)\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error copying in replacement inode bitmap */
	{ PR_5_COPY_IBITMAP_ERROR,
	  /* xgettext:no-c-format */
	  N_("Error copying in replacement @i @B: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Error copying in replacement block bitmap */
	{ PR_5_COPY_BBITMAP_ERROR,
	  /* xgettext:no-c-format */
	  N_("Error copying in replacement @b @B: %m\n"),
	  PROMPT_NONE, PR_FATAL, 0, 0, 0 },

	/* Block range not used, but marked in bitmap */
	{ PR_5_BLOCK_RANGE_UNUSED,
	  " -(%b--%c)",
	  PROMPT_NONE, PR_LATCH_BBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Block range used, but not marked used in bitmap */
	{ PR_5_BLOCK_RANGE_USED,
	  " +(%b--%c)",
	  PROMPT_NONE, PR_LATCH_BBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Inode range not used, but marked in bitmap */
	{ PR_5_INODE_RANGE_UNUSED,
	  " -(%i--%j)",
	  PROMPT_NONE, PR_LATCH_IBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Inode range used, but not marked used in bitmap */
	{ PR_5_INODE_RANGE_USED,
	  " +(%i--%j)",
	  PROMPT_NONE, PR_LATCH_IBITMAP | PR_PREEN_OK | PR_PREEN_NOMSG,
	  0, 0, 0 },

	/* Group N block(s) in use but group is marked BLOCK_UNINIT */
	{ PR_5_BLOCK_UNINIT,
	  /* xgettext:no-c-format */
	  N_("@g %g @b(s) in use but @g is marked BLOCK_UNINIT\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Group N inode(s) in use but group is marked INODE_UNINIT */
	{ PR_5_INODE_UNINIT,
	  /* xgettext:no-c-format */
	  N_("@g %g @i(s) in use but @g is marked INODE_UNINIT\n"),
	  PROMPT_FIX, PR_PREEN_OK, 0, 0, 0 },

	/* Group N inode bitmap does not match checksum */
	{ PR_5_INODE_BITMAP_CSUM_INVALID,
	  /* xgettext:no-c-format */
	  N_("@g %g @i @B does not match checksum.\n"),
	  PROMPT_FIX, PR_LATCH_IBITMAP | PR_PREEN_OK, 0, 0, 0 },

	/* Group N block bitmap does not match checksum */
	{ PR_5_BLOCK_BITMAP_CSUM_INVALID,
	  /* xgettext:no-c-format */
	  N_("@g %g @b @B does not match checksum.\n"),
	  PROMPT_FIX, PR_LATCH_BBITMAP | PR_PREEN_OK, 0, 0, 0 },

	/* Post-Pass 5 errors */

	/* Recreate journal if E2F_FLAG_JOURNAL_INODE flag is set */
	{ PR_6_RECREATE_JOURNAL,
	  N_("Recreate @j"),
	  PROMPT_NULL, PR_PREEN_OK | PR_NO_OK, 0, 0, 0 },

	/* Update quota information if it is inconsistent */
	{ PR_6_UPDATE_QUOTAS,
	  N_("Update quota info for quota type %N"),
	  PROMPT_NULL, PR_PREEN_OK, 0, 0, 0 },

	/* Error setting block group checksum info */
	{ PR_6_SET_BG_CHECKSUM,
	  /* xgettext:no-c-format */
	  N_("Error setting @b @g checksum info: %m\n"),
	  PROMPT_NULL, PR_FATAL, 0, 0, 0 },

	/* Error writing file system info */
	{ PR_6_FLUSH_FILESYSTEM,
	  /* xgettext:no-c-format */
	  N_("Error writing file system info: %m\n"),
	  PROMPT_NULL, PR_FATAL, 0, 0, 0 },

	/* Error flushing writes to storage device */
	{ PR_6_IO_FLUSH,
	  /* xgettext:no-c-format */
	  N_("Error flushing writes to storage device: %m\n"),
	  PROMPT_NULL, PR_FATAL, 0, 0, 0 },

	/* Error writing quota information */
	{ PR_6_WRITE_QUOTAS,
	  N_("Error writing quota info for quota type %N: %m\n"),
	  PROMPT_NULL, 0, 0, 0, 0 },

	{ 0 }
};

/*
 * This is the latch flags register.  It allows several problems to be
 * "latched" together.  This means that the user has to answer but one
 * question for the set of problems, and all of the associated
 * problems will be either fixed or not fixed.
 */
static struct latch_descr pr_latch_info[] = {
	{ PR_LATCH_BLOCK, PR_1_INODE_BLOCK_LATCH, 0, 0 },
	{ PR_LATCH_BBLOCK, PR_1_INODE_BBLOCK_LATCH, 0, 0 },
	{ PR_LATCH_IBITMAP, PR_5_INODE_BITMAP_HEADER, PR_5_INODE_BITMAP_END, 0 },
	{ PR_LATCH_BBITMAP, PR_5_BLOCK_BITMAP_HEADER, PR_5_BLOCK_BITMAP_END, 0 },
	{ PR_LATCH_RELOC, PR_0_RELOCATE_HINT, 0, 0 },
	{ PR_LATCH_DBLOCK, PR_1B_DUP_BLOCK_HEADER, PR_1B_DUP_BLOCK_END, 0 },
	{ PR_LATCH_LOW_DTIME, PR_1_ORPHAN_LIST_REFUGEES, 0, 0 },
	{ PR_LATCH_TOOBIG, PR_1_INODE_TOOBIG, 0, 0 },
	{ PR_LATCH_OPTIMIZE_DIR, PR_3A_OPTIMIZE_DIR_HEADER, PR_3A_OPTIMIZE_DIR_END, 0 },
	{ PR_LATCH_BG_CHECKSUM, PR_0_GDT_CSUM_LATCH, 0, 0 },
	{ PR_LATCH_OPTIMIZE_EXT, PR_1E_OPTIMIZE_EXT_HEADER, PR_1E_OPTIMIZE_EXT_END, 0 },
	{ -1, 0, 0, 0 },
};
#if __GNUC_PREREQ (4, 6)
#pragma GCC diagnostic pop
#endif

static struct e2fsck_problem *find_problem(problem_t code)
{
	int	i;

	for (i=0; problem_table[i].e2p_code; i++) {
		if (problem_table[i].e2p_code == code)
			return &problem_table[i];
	}
	return 0;
}

static struct latch_descr *find_latch(int code)
{
	int	i;

	for (i=0; pr_latch_info[i].latch_code >= 0; i++) {
		if (pr_latch_info[i].latch_code == code)
			return &pr_latch_info[i];
	}
	return 0;
}

int end_problem_latch(e2fsck_t ctx, int mask)
{
	struct latch_descr *ldesc;
	struct problem_context pctx;
	int answer = -1;

	ldesc = find_latch(mask);
	if (ldesc->end_message && (ldesc->flags & PRL_LATCHED)) {
		clear_problem_context(&pctx);
		answer = fix_problem(ctx, ldesc->end_message, &pctx);
	}
	ldesc->flags &= ~(PRL_VARIABLE);
	return answer;
}

int set_latch_flags(int mask, int setflags, int clearflags)
{
	struct latch_descr *ldesc;

	ldesc = find_latch(mask);
	if (!ldesc)
		return -1;
	ldesc->flags |= setflags;
	ldesc->flags &= ~clearflags;
	return 0;
}

int get_latch_flags(int mask, int *value)
{
	struct latch_descr *ldesc;

	ldesc = find_latch(mask);
	if (!ldesc)
		return -1;
	*value = ldesc->flags;
	return 0;
}

void clear_problem_context(struct problem_context *ctx)
{
	memset(ctx, 0, sizeof(struct problem_context));
	ctx->blkcount = -1;
	ctx->group = -1;
}

static void reconfigure_bool(e2fsck_t ctx, struct e2fsck_problem *ptr,
			     const char *key, int mask, const char *name)
{
	int	val;

	val = (ptr->flags & mask);
	profile_get_boolean(ctx->profile, "problems", key, name, val, &val);
	if (val)
		ptr->flags |= mask;
	else
		ptr->flags &= ~mask;
}

static void print_problem(FILE *f, problem_t code, int answer, int fixed,
			  struct e2fsck_problem *ptr,
			  struct problem_context *pctx)
{
	if (ptr->flags & PR_HEADER) {
		fprintf(f, "<header code=\"0x%06x\"/>\n", code);
		return;
	}
	fprintf(f, "<problem code=\"0x%06x\" answer=\"%d\"", code, answer);
	if (pctx->errcode)
		fprintf(f, " errcode=\"%lu\"", pctx->errcode);
	if (fixed)
		fputs(" fixed=\"1\"", f);
	if (pctx->ino)
		fprintf(f, " ino=\"%u\"", pctx->ino);
	if (pctx->ino2)
		fprintf(f, " ino2=\"%u\"", pctx->ino2);
	if (pctx->dir)
		fprintf(f, " dir=\"%u\"", pctx->dir);
	if (pctx->blk)
		fprintf(f, " blk=\"%llu\"", pctx->blk);
	if (pctx->blk2)
		fprintf(f, " blk2=\"%llu\"", pctx->blk2);
	if (pctx->blkcount != (e2_blkcnt_t) -1)
		fprintf(f, " blkcount=\"%lld\"", pctx->blkcount);
	if (pctx->group != (dgrp_t) -1)
		fprintf(f, " group=\"%u\"", pctx->group);
	if (pctx->csum1)
		fprintf(f, " csum1=\"%u\"", pctx->csum1);
	if (pctx->csum2)
		fprintf(f, " csum2=\"%u\"", pctx->csum2);
	if (pctx->num)
		fprintf(f, " num=\"%llu\"", pctx->num);
	if (pctx->num2)
		fprintf(f, " num2=\"%llu\"", pctx->num2);
	if (pctx->str)
		fprintf(f, " str=\"%s\"", pctx->str);
	fputs("/>\n", f);
}

int fix_problem(e2fsck_t ctx, problem_t code, struct problem_context *pctx)
{
	ext2_filsys fs = ctx->fs;
	struct e2fsck_problem *ptr;
	struct latch_descr *ldesc = 0;
	const char *message;
	int		def_yn, answer, ans;
	int		print_answer = 0;
	int		suppress = 0;
	int		fixed = 0;

	ptr = find_problem(code);
	if (!ptr) {
		printf(_("Unhandled error code (0x%x)!\n"), code);
		return 0;
	}
	if (!(ptr->flags & PR_CONFIG)) {
		char	key[9], *new_desc = NULL;

		sprintf(key, "0x%06x", code);

		profile_get_string(ctx->profile, "problems", key,
				   "description", 0, &new_desc);
		if (new_desc)
			ptr->e2p_description = new_desc;

		reconfigure_bool(ctx, ptr, key, PR_PREEN_OK, "preen_ok");
		reconfigure_bool(ctx, ptr, key, PR_NO_OK, "no_ok");
		reconfigure_bool(ctx, ptr, key, PR_NO_DEFAULT, "no_default");
		reconfigure_bool(ctx, ptr, key, PR_MSG_ONLY, "print_message_only");
		reconfigure_bool(ctx, ptr, key, PR_PREEN_NOMSG, "preen_nomessage");
		reconfigure_bool(ctx, ptr, key, PR_NOCOLLATE, "no_collate");
		reconfigure_bool(ctx, ptr, key, PR_NO_NOMSG, "no_nomsg");
		reconfigure_bool(ctx, ptr, key, PR_PREEN_NOHDR, "preen_noheader");
		reconfigure_bool(ctx, ptr, key, PR_FORCE_NO, "force_no");
		reconfigure_bool(ctx, ptr, key, PR_NOT_A_FIX, "not_a_fix");
		profile_get_integer(ctx->profile, "options",
				    "max_count_problems", 0, 0,
				    &ptr->max_count);
		profile_get_integer(ctx->profile, "problems", key, "max_count",
				    ptr->max_count, &ptr->max_count);

		ptr->flags |= PR_CONFIG;
	}
	def_yn = 1;
	ptr->count++;
	if ((ptr->flags & PR_NO_DEFAULT) ||
	    ((ptr->flags & PR_PREEN_NO) && (ctx->options & E2F_OPT_PREEN)) ||
	    (ctx->options & E2F_OPT_NO))
		def_yn= 0;

	/*
	 * Do special latch processing.  This is where we ask the
	 * latch question, if it exists
	 */
	if (ptr->flags & PR_LATCH_MASK) {
		ldesc = find_latch(ptr->flags & PR_LATCH_MASK);
		if (ldesc->question && !(ldesc->flags & PRL_LATCHED)) {
			ans = fix_problem(ctx, ldesc->question, pctx);
			if (ans == 1)
				ldesc->flags |= PRL_YES;
			if (ans == 0)
				ldesc->flags |= PRL_NO;
			ldesc->flags |= PRL_LATCHED;
		}
		if (ldesc->flags & PRL_SUPPRESS)
			suppress++;
	}
	if ((ptr->flags & PR_PREEN_NOMSG) &&
	    (ctx->options & E2F_OPT_PREEN))
		suppress++;
	if ((ptr->flags & PR_NO_NOMSG) &&
	    ((ctx->options & E2F_OPT_NO) || (ptr->flags & PR_FORCE_NO)))
		suppress++;
	if (ptr->max_count && (ptr->count > ptr->max_count)) {
		if (ctx->options & (E2F_OPT_NO | E2F_OPT_YES))
			suppress++;
		if ((ctx->options & E2F_OPT_PREEN) &&
		    (ptr->flags & PR_PREEN_OK))
			suppress++;
		if ((ptr->flags & PR_LATCH_MASK) &&
		    (ldesc->flags & (PRL_YES | PRL_NO)))
			suppress++;
		if (ptr->count == ptr->max_count + 1) {
			if (ctx->problem_logf)
				fprintf(ctx->problem_logf,
					"<suppressed code=\"0x%06x\"/>\n",
					code);
			printf("...problem 0x%06x suppressed\n",
			       ptr->e2p_code);
			fflush(stdout);
		}
	}
	message = ptr->e2p_description;
	if (*message)
		message = _(message);
	if (!suppress) {
		if ((ctx->options & E2F_OPT_PREEN) &&
		    !(ptr->flags & PR_PREEN_NOHDR)) {
			printf("%s: ", ctx->device_name ?
			       ctx->device_name : ctx->filesystem_name);
		}
		if (*message)
			print_e2fsck_message(stdout, ctx, message, pctx, 1, 0);
	}
	if (ctx->logf && message)
		print_e2fsck_message(ctx->logf, ctx, message, pctx, 1, 0);
	if (!(ptr->flags & PR_PREEN_OK) && (ptr->prompt != PROMPT_NONE))
		preenhalt(ctx);

	if (ptr->flags & PR_FATAL)
		fatal_error(ctx, 0);

	if (ptr->prompt == PROMPT_NONE) {
		if (ptr->flags & PR_NOCOLLATE)
			answer = -1;
		else
			answer = def_yn;
	} else {
		if (ptr->flags & PR_FORCE_NO) {
			answer = 0;
			print_answer = 1;
		} else if (ctx->options & E2F_OPT_PREEN) {
			answer = def_yn;
			if (!(ptr->flags & PR_PREEN_NOMSG))
				print_answer = 1;
		} else if ((ptr->flags & PR_LATCH_MASK) &&
			   (ldesc->flags & (PRL_YES | PRL_NO))) {
			print_answer = 1;
			if (ldesc->flags & PRL_YES)
				answer = 1;
			else
				answer = 0;
		} else
			answer = ask(ctx, (ptr->prompt == PROMPT_NULL) ? "" :
				     _(prompt[(int) ptr->prompt]), def_yn);
		if (!answer && !(ptr->flags & PR_NO_OK))
			ext2fs_unmark_valid(fs);

		if (print_answer) {
			if (!suppress)
				printf("%s.\n", answer ?
				       _(preen_msg[(int) ptr->prompt]) :
				       _("IGNORED"));
			if (ctx->logf)
				fprintf(ctx->logf, "%s.\n", answer ?
					_(preen_msg[(int) ptr->prompt]) :
					_("IGNORED"));
		}
	}

	if ((ptr->prompt == PROMPT_ABORT) && answer)
		fatal_error(ctx, 0);

	if (ptr->flags & PR_AFTER_CODE)
		answer = fix_problem(ctx, ptr->second_code, pctx);

	if (answer && (ptr->prompt != PROMPT_NONE) &&
	    !(ptr->flags & PR_NOT_A_FIX)) {
		fixed = 1;
		ctx->flags |= E2F_FLAG_PROBLEMS_FIXED;
	}

	if (ctx->problem_logf)
		print_problem(ctx->problem_logf, code, answer, fixed,
			      ptr, pctx);

	return answer;
}

#ifdef UNITTEST

#include <stdlib.h>
#include <stdio.h>

errcode_t
profile_get_boolean(profile_t profile, const char *name, const char *subname,
		    const char *subsubname, int def_val, int *ret_boolean)
{
	return 0;
}

errcode_t
profile_get_integer(profile_t profile, const char *name, const char *subname,
		    const char *subsubname, int def_val, int *ret_int)
{
	return 0;
}

void print_e2fsck_message(FILE *f, e2fsck_t ctx, const char *msg,
			  struct problem_context *pctx, int first,
			  int recurse)
{
	return;
}

void fatal_error(e2fsck_t ctx, const char *msg)
{
	return;
}

void preenhalt(e2fsck_t ctx)
{
	return;
}

errcode_t
profile_get_string(profile_t profile, const char *name, const char *subname,
		   const char *subsubname, const char *def_val,
		   char **ret_string)
{
	return 0;
}

int ask (e2fsck_t ctx, const char * string, int def)
{
	return 0;
}

int verify_problem_table(e2fsck_t ctx)
{
	struct e2fsck_problem *curr, *prev = NULL;
	int rc = 0;

	for (prev = NULL, curr = problem_table; curr->e2p_code; prev = curr++) {
		if (prev == NULL)
			continue;

		if (curr->e2p_code > prev->e2p_code)
			continue;

		if (curr->e2p_code == prev->e2p_code)
			fprintf(stderr, "*** Duplicate in problem table:\n");
		else
			fprintf(stderr, "*** Unordered problem table:\n");

		fprintf(stderr, "curr code = 0x%08x: %s\n",
			curr->e2p_code, curr->e2p_description);
		fprintf(stderr, "*** prev code = 0x%08x: %s\n",
			prev->e2p_code, prev->e2p_description);

		fprintf(stderr, "*** This is a %sprogramming error in e2fsck\n",
			(curr->e2p_code == prev->e2p_code) ? "fatal " : "");

		rc = 1;
	}

	return rc;
}

int main(int argc, char *argv[])
{
	e2fsck_t ctx;
	int rc;

	memset(&ctx, 0, sizeof(ctx)); /* just to quiet compiler */
	rc = verify_problem_table(ctx);
	if (rc == 0)
		printf("e2fsck problem table verified\n");

	return rc;
}
#endif /* UNITTEST */
