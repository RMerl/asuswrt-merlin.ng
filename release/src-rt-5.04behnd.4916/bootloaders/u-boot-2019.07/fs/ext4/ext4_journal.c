// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * Journal data structures and headers for Journaling feature of ext4
 * have been referred from JBD2 (Journaling Block device 2)
 * implementation in Linux Kernel.
 * Written by Stephen C. Tweedie <sct@redhat.com>
 *
 * Copyright 1998-2000 Red Hat, Inc --- All Rights Reserved
 */

#include <common.h>
#include <ext4fs.h>
#include <malloc.h>
#include <ext_common.h>
#include "ext4_common.h"

static struct revoke_blk_list *revk_blk_list;
static struct revoke_blk_list *prev_node;
static int first_node = true;

int gindex;
int gd_index;
int jrnl_blk_idx;
struct journal_log *journal_ptr[MAX_JOURNAL_ENTRIES];
struct dirty_blocks *dirty_block_ptr[MAX_JOURNAL_ENTRIES];

int ext4fs_init_journal(void)
{
	int i;
	char *temp = NULL;
	struct ext_filesystem *fs = get_fs();

	/* init globals */
	revk_blk_list = NULL;
	prev_node = NULL;
	gindex = 0;
	gd_index = 0;
	jrnl_blk_idx = 1;

	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		journal_ptr[i] = zalloc(sizeof(struct journal_log));
		if (!journal_ptr[i])
			goto fail;
		dirty_block_ptr[i] = zalloc(sizeof(struct dirty_blocks));
		if (!dirty_block_ptr[i])
			goto fail;
		journal_ptr[i]->buf = NULL;
		journal_ptr[i]->blknr = -1;

		dirty_block_ptr[i]->buf = NULL;
		dirty_block_ptr[i]->blknr = -1;
	}

	if (fs->blksz == 4096) {
		temp = zalloc(fs->blksz);
		if (!temp)
			goto fail;
		journal_ptr[gindex]->buf = zalloc(fs->blksz);
		if (!journal_ptr[gindex]->buf)
			goto fail;
		ext4fs_devread(0, 0, fs->blksz, temp);
		memcpy(temp + SUPERBLOCK_SIZE, fs->sb, SUPERBLOCK_SIZE);
		memcpy(journal_ptr[gindex]->buf, temp, fs->blksz);
		journal_ptr[gindex++]->blknr = 0;
		free(temp);
	} else {
		journal_ptr[gindex]->buf = zalloc(fs->blksz);
		if (!journal_ptr[gindex]->buf)
			goto fail;
		memcpy(journal_ptr[gindex]->buf, fs->sb, SUPERBLOCK_SIZE);
		journal_ptr[gindex++]->blknr = 1;
	}

	/* Check the file system state using journal super block */
	if (ext4fs_check_journal_state(SCAN))
		goto fail;
	/* Check the file system state using journal super block */
	if (ext4fs_check_journal_state(RECOVER))
		goto fail;

	return 0;
fail:
	return -1;
}

void ext4fs_dump_metadata(void)
{
	struct ext_filesystem *fs = get_fs();
	int i;
	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		if (dirty_block_ptr[i]->blknr == -1)
			break;
		put_ext4((uint64_t) ((uint64_t)dirty_block_ptr[i]->blknr *
				(uint64_t)fs->blksz), dirty_block_ptr[i]->buf,
								fs->blksz);
	}
}

void ext4fs_free_journal(void)
{
	int i;
	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		if (dirty_block_ptr[i]->blknr == -1)
			break;
		if (dirty_block_ptr[i]->buf)
			free(dirty_block_ptr[i]->buf);
	}

	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		if (journal_ptr[i]->blknr == -1)
			break;
		if (journal_ptr[i]->buf)
			free(journal_ptr[i]->buf);
	}

	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		if (journal_ptr[i])
			free(journal_ptr[i]);
		if (dirty_block_ptr[i])
			free(dirty_block_ptr[i]);
	}
	gindex = 0;
	gd_index = 0;
	jrnl_blk_idx = 1;
}

int ext4fs_log_gdt(char *gd_table)
{
	struct ext_filesystem *fs = get_fs();
	short i;
	long int var = fs->gdtable_blkno;
	for (i = 0; i < fs->no_blk_pergdt; i++) {
		journal_ptr[gindex]->buf = zalloc(fs->blksz);
		if (!journal_ptr[gindex]->buf)
			return -ENOMEM;
		memcpy(journal_ptr[gindex]->buf, gd_table, fs->blksz);
		gd_table += fs->blksz;
		journal_ptr[gindex++]->blknr = var++;
	}

	return 0;
}

/*
 * This function stores the backup copy of meta data in RAM
 * journal_buffer -- Buffer containing meta data
 * blknr -- Block number on disk of the meta data buffer
 */
int ext4fs_log_journal(char *journal_buffer, uint32_t blknr)
{
	struct ext_filesystem *fs = get_fs();
	short i;

	if (!journal_buffer) {
		printf("Invalid input arguments %s\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		if (journal_ptr[i]->blknr == -1)
			break;
		if (journal_ptr[i]->blknr == blknr)
			return 0;
	}

	journal_ptr[gindex]->buf = zalloc(fs->blksz);
	if (!journal_ptr[gindex]->buf)
		return -ENOMEM;

	memcpy(journal_ptr[gindex]->buf, journal_buffer, fs->blksz);
	journal_ptr[gindex++]->blknr = blknr;

	return 0;
}

/*
 * This function stores the modified meta data in RAM
 * metadata_buffer -- Buffer containing meta data
 * blknr -- Block number on disk of the meta data buffer
 */
int ext4fs_put_metadata(char *metadata_buffer, uint32_t blknr)
{
	struct ext_filesystem *fs = get_fs();
	if (!metadata_buffer) {
		printf("Invalid input arguments %s\n", __func__);
		return -EINVAL;
	}
	if (dirty_block_ptr[gd_index]->buf)
		assert(dirty_block_ptr[gd_index]->blknr == blknr);
	else
		dirty_block_ptr[gd_index]->buf = zalloc(fs->blksz);

	if (!dirty_block_ptr[gd_index]->buf)
		return -ENOMEM;
	memcpy(dirty_block_ptr[gd_index]->buf, metadata_buffer, fs->blksz);
	dirty_block_ptr[gd_index++]->blknr = blknr;

	return 0;
}

void print_revoke_blks(char *revk_blk)
{
	int offset;
	int max;
	long int blocknr;
	struct journal_revoke_header_t *header;

	if (revk_blk == NULL)
		return;

	header = (struct journal_revoke_header_t *) revk_blk;
	offset = sizeof(struct journal_revoke_header_t);
	max = be32_to_cpu(header->r_count);
	printf("total bytes %d\n", max);

	while (offset < max) {
		blocknr = be32_to_cpu(*((__be32 *)(revk_blk + offset)));
		printf("revoke blknr is %ld\n", blocknr);
		offset += 4;
	}
}

static struct revoke_blk_list *_get_node(void)
{
	struct revoke_blk_list *tmp_node;
	tmp_node = zalloc(sizeof(struct revoke_blk_list));
	if (tmp_node == NULL)
		return NULL;
	tmp_node->content = NULL;
	tmp_node->next = NULL;

	return tmp_node;
}

void ext4fs_push_revoke_blk(char *buffer)
{
	struct revoke_blk_list *node = NULL;
	struct ext_filesystem *fs = get_fs();
	if (buffer == NULL) {
		printf("buffer ptr is NULL\n");
		return;
	}
	node = _get_node();
	if (!node) {
		printf("_get_node: malloc failed\n");
		return;
	}

	node->content = zalloc(fs->blksz);
	if (node->content == NULL)
		return;
	memcpy(node->content, buffer, fs->blksz);

	if (first_node == true) {
		revk_blk_list = node;
		prev_node = node;
		 first_node = false;
	} else {
		prev_node->next = node;
		prev_node = node;
	}
}

void ext4fs_free_revoke_blks(void)
{
	struct revoke_blk_list *tmp_node = revk_blk_list;
	struct revoke_blk_list *next_node = NULL;

	while (tmp_node != NULL) {
		if (tmp_node->content)
			free(tmp_node->content);
		tmp_node = tmp_node->next;
	}

	tmp_node = revk_blk_list;
	while (tmp_node != NULL) {
		next_node = tmp_node->next;
		free(tmp_node);
		tmp_node = next_node;
	}

	revk_blk_list = NULL;
	prev_node = NULL;
	first_node = true;
}

int check_blknr_for_revoke(long int blknr, int sequence_no)
{
	struct journal_revoke_header_t *header;
	int offset;
	int max;
	long int blocknr;
	char *revk_blk;
	struct revoke_blk_list *tmp_revk_node = revk_blk_list;
	while (tmp_revk_node != NULL) {
		revk_blk = tmp_revk_node->content;

		header = (struct journal_revoke_header_t *) revk_blk;
		if (sequence_no < be32_to_cpu(header->r_header.h_sequence)) {
			offset = sizeof(struct journal_revoke_header_t);
			max = be32_to_cpu(header->r_count);

			while (offset < max) {
				blocknr = be32_to_cpu(*((__be32 *)
						  (revk_blk + offset)));
				if (blocknr == blknr)
					goto found;
				offset += 4;
			}
		}
		tmp_revk_node = tmp_revk_node->next;
	}

	return -1;

found:
	return 0;
}

/*
 * This function parses the journal blocks and replays the
 * suceessful transactions. A transaction is successfull
 * if commit block is found for a descriptor block
 * The tags in descriptor block contain the disk block
 * numbers of the metadata  to be replayed
 */
void recover_transaction(int prev_desc_logical_no)
{
	struct ext2_inode inode_journal;
	struct ext_filesystem *fs = get_fs();
	struct journal_header_t *jdb;
	long int blknr;
	char *p_jdb;
	int ofs, flags;
	int i;
	struct ext3_journal_block_tag *tag;
	char *temp_buff = zalloc(fs->blksz);
	char *metadata_buff = zalloc(fs->blksz);
	if (!temp_buff || !metadata_buff)
		goto fail;
	i = prev_desc_logical_no;
	ext4fs_read_inode(ext4fs_root, EXT2_JOURNAL_INO,
			  (struct ext2_inode *)&inode_journal);
	blknr = read_allocated_block((struct ext2_inode *)
				     &inode_journal, i, NULL);
	ext4fs_devread((lbaint_t)blknr * fs->sect_perblk, 0, fs->blksz,
		       temp_buff);
	p_jdb = (char *)temp_buff;
	jdb = (struct journal_header_t *) temp_buff;
	ofs = sizeof(struct journal_header_t);

	do {
		tag = (struct ext3_journal_block_tag *)(p_jdb + ofs);
		ofs += sizeof(struct ext3_journal_block_tag);

		if (ofs > fs->blksz)
			break;

		flags = be32_to_cpu(tag->flags);
		if (!(flags & EXT3_JOURNAL_FLAG_SAME_UUID))
			ofs += 16;

		i++;
		debug("\t\ttag %u\n", be32_to_cpu(tag->block));
		if (revk_blk_list != NULL) {
			if (check_blknr_for_revoke(be32_to_cpu(tag->block),
				be32_to_cpu(jdb->h_sequence)) == 0)
				continue;
		}
		blknr = read_allocated_block(&inode_journal, i, NULL);
		ext4fs_devread((lbaint_t)blknr * fs->sect_perblk, 0,
			       fs->blksz, metadata_buff);
		put_ext4((uint64_t)((uint64_t)be32_to_cpu(tag->block) * (uint64_t)fs->blksz),
			 metadata_buff, (uint32_t) fs->blksz);
	} while (!(flags & EXT3_JOURNAL_FLAG_LAST_TAG));
fail:
	free(temp_buff);
	free(metadata_buff);
}

void print_jrnl_status(int recovery_flag)
{
	if (recovery_flag == RECOVER)
		printf("Journal Recovery Completed\n");
	else
		printf("Journal Scan Completed\n");
}

int ext4fs_check_journal_state(int recovery_flag)
{
	int i;
	int DB_FOUND = NO;
	long int blknr;
	int transaction_state = TRANSACTION_COMPLETE;
	int prev_desc_logical_no = 0;
	int curr_desc_logical_no = 0;
	int ofs, flags;
	struct ext2_inode inode_journal;
	struct journal_superblock_t *jsb = NULL;
	struct journal_header_t *jdb = NULL;
	char *p_jdb = NULL;
	struct ext3_journal_block_tag *tag = NULL;
	char *temp_buff = NULL;
	char *temp_buff1 = NULL;
	struct ext_filesystem *fs = get_fs();

	temp_buff = zalloc(fs->blksz);
	if (!temp_buff)
		return -ENOMEM;
	temp_buff1 = zalloc(fs->blksz);
	if (!temp_buff1) {
		free(temp_buff);
		return -ENOMEM;
	}

	ext4fs_read_inode(ext4fs_root, EXT2_JOURNAL_INO, &inode_journal);
	blknr = read_allocated_block(&inode_journal, EXT2_JOURNAL_SUPERBLOCK,
				     NULL);
	ext4fs_devread((lbaint_t)blknr * fs->sect_perblk, 0, fs->blksz,
		       temp_buff);
	jsb = (struct journal_superblock_t *) temp_buff;

	if (le32_to_cpu(fs->sb->feature_incompat) & EXT3_FEATURE_INCOMPAT_RECOVER) {
		if (recovery_flag == RECOVER)
			printf("Recovery required\n");
	} else {
		if (recovery_flag == RECOVER)
			printf("File System is consistent\n");
		goto end;
	}

	if (be32_to_cpu(jsb->s_start) == 0)
		goto end;

	if (!(jsb->s_feature_compat &
				cpu_to_be32(JBD2_FEATURE_COMPAT_CHECKSUM)))
		jsb->s_feature_compat |=
				cpu_to_be32(JBD2_FEATURE_COMPAT_CHECKSUM);

	i = be32_to_cpu(jsb->s_first);
	while (1) {
		blknr = read_allocated_block(&inode_journal, i, NULL);
		memset(temp_buff1, '\0', fs->blksz);
		ext4fs_devread((lbaint_t)blknr * fs->sect_perblk,
			       0, fs->blksz, temp_buff1);
		jdb = (struct journal_header_t *) temp_buff1;

		if (be32_to_cpu(jdb->h_blocktype) ==
		    EXT3_JOURNAL_DESCRIPTOR_BLOCK) {
			if (be32_to_cpu(jdb->h_sequence) !=
			    be32_to_cpu(jsb->s_sequence)) {
				print_jrnl_status(recovery_flag);
				break;
			}

			curr_desc_logical_no = i;
			if (transaction_state == TRANSACTION_COMPLETE)
				transaction_state = TRANSACTION_RUNNING;
			else
				return -1;
			p_jdb = (char *)temp_buff1;
			ofs = sizeof(struct journal_header_t);
			do {
				tag = (struct ext3_journal_block_tag *)
				    (p_jdb + ofs);
				ofs += sizeof(struct ext3_journal_block_tag);
				if (ofs > fs->blksz)
					break;
				flags = be32_to_cpu(tag->flags);
				if (!(flags & EXT3_JOURNAL_FLAG_SAME_UUID))
					ofs += 16;
				i++;
				debug("\t\ttag %u\n", be32_to_cpu(tag->block));
			} while (!(flags & EXT3_JOURNAL_FLAG_LAST_TAG));
			i++;
			DB_FOUND = YES;
		} else if (be32_to_cpu(jdb->h_blocktype) ==
				EXT3_JOURNAL_COMMIT_BLOCK) {
			if (be32_to_cpu(jdb->h_sequence) !=
			     be32_to_cpu(jsb->s_sequence)) {
				print_jrnl_status(recovery_flag);
				break;
			}

			if (transaction_state == TRANSACTION_RUNNING ||
					(DB_FOUND == NO)) {
				transaction_state = TRANSACTION_COMPLETE;
				i++;
				jsb->s_sequence =
					cpu_to_be32(be32_to_cpu(
						jsb->s_sequence) + 1);
			}
			prev_desc_logical_no = curr_desc_logical_no;
			if ((recovery_flag == RECOVER) && (DB_FOUND == YES))
				recover_transaction(prev_desc_logical_no);

			DB_FOUND = NO;
		} else if (be32_to_cpu(jdb->h_blocktype) ==
				EXT3_JOURNAL_REVOKE_BLOCK) {
			if (be32_to_cpu(jdb->h_sequence) !=
			    be32_to_cpu(jsb->s_sequence)) {
				print_jrnl_status(recovery_flag);
				break;
			}
			if (recovery_flag == SCAN)
				ext4fs_push_revoke_blk((char *)jdb);
			i++;
		} else {
			debug("Else Case\n");
			if (be32_to_cpu(jdb->h_sequence) !=
			    be32_to_cpu(jsb->s_sequence)) {
				print_jrnl_status(recovery_flag);
				break;
			}
		}
	}

end:
	if (recovery_flag == RECOVER) {
		uint32_t new_feature_incompat;
		jsb->s_start = cpu_to_be32(1);
		jsb->s_sequence = cpu_to_be32(be32_to_cpu(jsb->s_sequence) + 1);
		/* get the superblock */
		ext4_read_superblock((char *)fs->sb);
		new_feature_incompat = le32_to_cpu(fs->sb->feature_incompat);
		new_feature_incompat |= EXT3_FEATURE_INCOMPAT_RECOVER;
		fs->sb->feature_incompat = cpu_to_le32(new_feature_incompat);

		/* Update the super block */
		put_ext4((uint64_t) (SUPERBLOCK_SIZE),
			 (struct ext2_sblock *)fs->sb,
			 (uint32_t) SUPERBLOCK_SIZE);
		ext4_read_superblock((char *)fs->sb);

		blknr = read_allocated_block(&inode_journal,
					 EXT2_JOURNAL_SUPERBLOCK, NULL);
		put_ext4((uint64_t) ((uint64_t)blknr * (uint64_t)fs->blksz),
			 (struct journal_superblock_t *)temp_buff,
			 (uint32_t) fs->blksz);
		ext4fs_free_revoke_blks();
	}
	free(temp_buff);
	free(temp_buff1);

	return 0;
}

static void update_descriptor_block(long int blknr)
{
	int i;
	long int jsb_blknr;
	struct journal_header_t jdb;
	struct ext3_journal_block_tag tag;
	struct ext2_inode inode_journal;
	struct journal_superblock_t *jsb = NULL;
	char *buf = NULL;
	char *temp = NULL;
	struct ext_filesystem *fs = get_fs();
	char *temp_buff = zalloc(fs->blksz);
	if (!temp_buff)
		return;

	ext4fs_read_inode(ext4fs_root, EXT2_JOURNAL_INO, &inode_journal);
	jsb_blknr = read_allocated_block(&inode_journal,
					 EXT2_JOURNAL_SUPERBLOCK, NULL);
	ext4fs_devread((lbaint_t)jsb_blknr * fs->sect_perblk, 0, fs->blksz,
		       temp_buff);
	jsb = (struct journal_superblock_t *) temp_buff;

	jdb.h_blocktype = cpu_to_be32(EXT3_JOURNAL_DESCRIPTOR_BLOCK);
	jdb.h_magic = cpu_to_be32(EXT3_JOURNAL_MAGIC_NUMBER);
	jdb.h_sequence = jsb->s_sequence;
	buf = zalloc(fs->blksz);
	if (!buf) {
		free(temp_buff);
		return;
	}
	temp = buf;
	memcpy(buf, &jdb, sizeof(struct journal_header_t));
	temp += sizeof(struct journal_header_t);

	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		if (journal_ptr[i]->blknr == -1)
			break;

		tag.block = cpu_to_be32(journal_ptr[i]->blknr);
		tag.flags = cpu_to_be32(EXT3_JOURNAL_FLAG_SAME_UUID);
		memcpy(temp, &tag, sizeof(struct ext3_journal_block_tag));
		temp = temp + sizeof(struct ext3_journal_block_tag);
	}

	tag.block = cpu_to_be32(journal_ptr[--i]->blknr);
	tag.flags = cpu_to_be32(EXT3_JOURNAL_FLAG_LAST_TAG);
	memcpy(temp - sizeof(struct ext3_journal_block_tag), &tag,
	       sizeof(struct ext3_journal_block_tag));
	put_ext4((uint64_t) ((uint64_t)blknr * (uint64_t)fs->blksz), buf, (uint32_t) fs->blksz);

	free(temp_buff);
	free(buf);
}

static void update_commit_block(long int blknr)
{
	struct journal_header_t jdb;
	struct ext_filesystem *fs = get_fs();
	char *buf = NULL;
	struct ext2_inode inode_journal;
	struct journal_superblock_t *jsb;
	long int jsb_blknr;
	char *temp_buff = zalloc(fs->blksz);
	if (!temp_buff)
		return;

	ext4fs_read_inode(ext4fs_root, EXT2_JOURNAL_INO,
			  &inode_journal);
	jsb_blknr = read_allocated_block(&inode_journal,
					 EXT2_JOURNAL_SUPERBLOCK, NULL);
	ext4fs_devread((lbaint_t)jsb_blknr * fs->sect_perblk, 0, fs->blksz,
		       temp_buff);
	jsb = (struct journal_superblock_t *) temp_buff;

	jdb.h_blocktype = cpu_to_be32(EXT3_JOURNAL_COMMIT_BLOCK);
	jdb.h_magic = cpu_to_be32(EXT3_JOURNAL_MAGIC_NUMBER);
	jdb.h_sequence = jsb->s_sequence;
	buf = zalloc(fs->blksz);
	if (!buf) {
		free(temp_buff);
		return;
	}
	memcpy(buf, &jdb, sizeof(struct journal_header_t));
	put_ext4((uint64_t) ((uint64_t)blknr * (uint64_t)fs->blksz), buf, (uint32_t) fs->blksz);

	free(temp_buff);
	free(buf);
}

void ext4fs_update_journal(void)
{
	struct ext2_inode inode_journal;
	struct ext_filesystem *fs = get_fs();
	long int blknr;
	int i;
	ext4fs_read_inode(ext4fs_root, EXT2_JOURNAL_INO, &inode_journal);
	blknr = read_allocated_block(&inode_journal, jrnl_blk_idx++, NULL);
	update_descriptor_block(blknr);
	for (i = 0; i < MAX_JOURNAL_ENTRIES; i++) {
		if (journal_ptr[i]->blknr == -1)
			break;
		blknr = read_allocated_block(&inode_journal, jrnl_blk_idx++,
					     NULL);
		put_ext4((uint64_t) ((uint64_t)blknr * (uint64_t)fs->blksz),
			 journal_ptr[i]->buf, fs->blksz);
	}
	blknr = read_allocated_block(&inode_journal, jrnl_blk_idx++, NULL);
	update_commit_block(blknr);
	printf("update journal finished\n");
}
