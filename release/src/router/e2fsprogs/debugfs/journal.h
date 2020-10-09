/*
 * journal.h
 *
 * Copyright (C) 2000 Andreas Dilger
 * Copyright (C) 2000 Theodore Ts'o
 *
 * Parts of the code are based on fs/jfs/journal.c by Stephen C. Tweedie
 * Copyright (C) 1999 Red Hat Software
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License version 2 or at your discretion
 * any later version.
 */

#include "jfs_user.h"

/* journal.c */
errcode_t ext2fs_open_journal(ext2_filsys fs, journal_t **j);
errcode_t ext2fs_close_journal(ext2_filsys fs, journal_t **j);
errcode_t ext2fs_run_ext3_journal(ext2_filsys *fs);
void jbd2_commit_block_csum_set(journal_t *j, struct buffer_head *bh);
void jbd2_revoke_csum_set(journal_t *j, struct buffer_head *bh);
void jbd2_descr_block_csum_set(journal_t *j, struct buffer_head *bh);
void jbd2_block_tag_csum_set(journal_t *j, journal_block_tag_t *tag,
			     struct buffer_head *bh, __u32 sequence);
