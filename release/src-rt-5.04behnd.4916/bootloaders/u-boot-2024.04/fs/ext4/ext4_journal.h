/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * Journal data structures and headers for Journaling feature of ext4
 * have been referred from JBD2 (Journaling Block device 2)
 * implementation in Linux Kernel.
 *
 * Written by Stephen C. Tweedie <sct@redhat.com>
 *
 * Copyright 1998-2000 Red Hat, Inc --- All Rights Reserved
 */

#ifndef __EXT4_JRNL__
#define __EXT4_JRNL__

#define EXT4_FEATURE_COMPAT_HAS_JOURNAL		0x0004

#define EXT2_JOURNAL_INO		8	/* Journal inode */
#define EXT2_JOURNAL_SUPERBLOCK	0	/* Journal  Superblock number */

#define JBD2_FEATURE_COMPAT_CHECKSUM	0x00000001
#define EXT3_JOURNAL_MAGIC_NUMBER	0xc03b3998U
#define TRANSACTION_RUNNING		1
#define TRANSACTION_COMPLETE		0
#define EXT3_FEATURE_INCOMPAT_RECOVER	0x0004	/* Needs recovery */
#define EXT3_JOURNAL_DESCRIPTOR_BLOCK	1
#define EXT3_JOURNAL_COMMIT_BLOCK	2
#define EXT3_JOURNAL_SUPERBLOCK_V1	3
#define EXT3_JOURNAL_SUPERBLOCK_V2	4
#define EXT3_JOURNAL_REVOKE_BLOCK	5
#define EXT3_JOURNAL_FLAG_ESCAPE	1
#define EXT3_JOURNAL_FLAG_SAME_UUID	2
#define EXT3_JOURNAL_FLAG_DELETED	4
#define EXT3_JOURNAL_FLAG_LAST_TAG	8

/* Maximum entries in 1 journal transaction */
#define MAX_JOURNAL_ENTRIES 100
struct journal_log {
	char *buf;
	int blknr;
};

struct dirty_blocks {
	char *buf;
	int blknr;
};

/* Standard header for all descriptor blocks: */
struct journal_header_t {
	__be32 h_magic;
	__be32 h_blocktype;
	__be32 h_sequence;
};

/* The journal superblock.  All fields are in big-endian byte order. */
struct journal_superblock_t {
	/* 0x0000 */
	struct journal_header_t s_header;

	/* Static information describing the journal */
	__be32 s_blocksize;	/* journal device blocksize */
	__be32 s_maxlen;		/* total blocks in journal file */
	__be32 s_first;		/* first block of log information */

	/* Dynamic information describing the current state of the log */
	__be32 s_sequence;	/* first commit ID expected in log */
	__be32 s_start;		/* blocknr of start of log */

	/* Error value, as set by journal_abort(). */
	__be32 s_errno;

	/* Remaining fields are only valid in a version-2 superblock */
	__be32 s_feature_compat;	/* compatible feature set */
	__be32 s_feature_incompat;	/* incompatible feature set */
	__be32 s_feature_ro_compat;	/* readonly-compatible feature set */
	/* 0x0030 */
	__u8 s_uuid[16];	/* 128-bit uuid for journal */

	/* 0x0040 */
	__be32 s_nr_users;	/* Nr of filesystems sharing log */

	__be32 s_dynsuper;	/* Blocknr of dynamic superblock copy */

	/* 0x0048 */
	__be32 s_max_transaction;	/* Limit of journal blocks per trans. */
	__be32 s_max_trans_data;	/* Limit of data blocks per trans. */

	/* 0x0050 */
	__be32 s_padding[44];

	/* 0x0100 */
	__u8 s_users[16 * 48];	/* ids of all fs'es sharing the log */
	/* 0x0400 */
} ;

struct ext3_journal_block_tag {
	__be32 block;
	__be32 flags;
};

struct journal_revoke_header_t {
	struct journal_header_t r_header;
	__be32 r_count;		/* Count of bytes used in the block */
};

struct revoke_blk_list {
	char *content;		/* revoke block itself */
	struct revoke_blk_list *next;
};

extern struct ext2_data *ext4fs_root;

int ext4fs_init_journal(void);
int ext4fs_log_gdt(char *gd_table);
int ext4fs_check_journal_state(int recovery_flag);
int ext4fs_log_journal(char *journal_buffer, uint32_t blknr);
int ext4fs_put_metadata(char *metadata_buffer, uint32_t blknr);
void ext4fs_update_journal(void);
void ext4fs_dump_metadata(void);
void ext4fs_push_revoke_blk(char *buffer);
void ext4fs_free_journal(void);
void ext4fs_free_revoke_blks(void);
#endif
