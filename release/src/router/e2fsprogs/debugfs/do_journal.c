/*
 * do_journal.c --- Scribble onto the journal!
 *
 * Copyright (C) 2014 Oracle.  This file may be redistributed
 * under the terms of the GNU Public License.
 */

#include "config.h"
#include <stdio.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern int optind;
extern char *optarg;
#endif
#include <ctype.h>
#include <unistd.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "debugfs.h"
#include "ext2fs/kernel-jbd.h"
#include "journal.h"

#undef DEBUG

#ifdef DEBUG
# define dbg_printf(f, a...)  do {printf("JFS DEBUG: " f, ## a); \
	fflush(stdout); \
} while (0)
#else
# define dbg_printf(f, a...)
#endif

#define JOURNAL_CHECK_TRANS_MAGIC(x)	\
	do { \
		if ((x)->magic != J_TRANS_MAGIC) \
			return EXT2_ET_INVALID_ARGUMENT; \
	} while (0)

#define J_TRANS_MAGIC		0xD15EA5ED
#define J_TRANS_OPEN		1
#define J_TRANS_COMMITTED	2
struct journal_transaction_s {
	unsigned int magic;
	ext2_filsys fs;
	journal_t *journal;
	blk64_t block;
	blk64_t start, end;
	tid_t tid;
	int flags;
};

typedef struct journal_transaction_s journal_transaction_t;

static journal_t *current_journal = NULL;

static void journal_dump_trans(journal_transaction_t *trans EXT2FS_ATTR((unused)),
			       const char *tag EXT2FS_ATTR((unused)))
{
	dbg_printf("TRANS %p(%s): tid=%u start=%llu block=%llu end=%llu "
		   "flags=0x%x\n", trans, tag, trans->tid, trans->start,
		   trans->block, trans->end, trans->flags);
}

static errcode_t journal_commit_trans(journal_transaction_t *trans)
{
	struct buffer_head *bh, *cbh = NULL;
	struct commit_header *commit;
#ifdef HAVE_SYS_TIME_H
	struct timeval tv;
#endif
	errcode_t err;

	JOURNAL_CHECK_TRANS_MAGIC(trans);

	if ((trans->flags & J_TRANS_COMMITTED) ||
	    !(trans->flags & J_TRANS_OPEN))
		return EXT2_ET_INVALID_ARGUMENT;

	bh = getblk(trans->journal->j_dev, 0, trans->journal->j_blocksize);
	if (bh == NULL)
		return ENOMEM;

	/* write the descriptor block header */
	commit = (struct commit_header *)bh->b_data;
	commit->h_magic = ext2fs_cpu_to_be32(JFS_MAGIC_NUMBER);
	commit->h_blocktype = ext2fs_cpu_to_be32(JFS_COMMIT_BLOCK);
	commit->h_sequence = ext2fs_cpu_to_be32(trans->tid);
	if (jfs_has_feature_checksum(trans->journal)) {
		__u32 csum_v1 = ~0;
		blk64_t cblk;

		cbh = getblk(trans->journal->j_dev, 0,
			     trans->journal->j_blocksize);
		if (cbh == NULL) {
			err = ENOMEM;
			goto error;
		}

		for (cblk = trans->start; cblk < trans->block; cblk++) {
			err = journal_bmap(trans->journal, cblk,
					   &cbh->b_blocknr);
			if (err)
				goto error;
			mark_buffer_uptodate(cbh, 0);
			ll_rw_block(READ, 1, &cbh);
			err = cbh->b_err;
			if (err)
				goto error;
			csum_v1 = ext2fs_crc32_be(csum_v1,
					(unsigned char const *)cbh->b_data,
					cbh->b_size);
		}

		commit->h_chksum_type = JFS_CRC32_CHKSUM;
		commit->h_chksum_size = JFS_CRC32_CHKSUM_SIZE;
		commit->h_chksum[0] = ext2fs_cpu_to_be32(csum_v1);
	} else {
		commit->h_chksum_type = 0;
		commit->h_chksum_size = 0;
		commit->h_chksum[0] = 0;
	}
#ifdef HAVE_SYS_TIME_H
	gettimeofday(&tv, NULL);
	commit->h_commit_sec = ext2fs_cpu_to_be32(tv.tv_sec);
	commit->h_commit_nsec = ext2fs_cpu_to_be32(tv.tv_usec * 1000);
#else
	commit->h_commit_sec = 0;
	commit->h_commit_nsec = 0;
#endif

	/* Write block */
	jbd2_commit_block_csum_set(trans->journal, bh);
	err = journal_bmap(trans->journal, trans->block, &bh->b_blocknr);
	if (err)
		goto error;

	dbg_printf("Writing commit block at %llu:%llu\n", trans->block,
		   bh->b_blocknr);
	mark_buffer_dirty(bh);
	ll_rw_block(WRITE, 1, &bh);
	err = bh->b_err;
	if (err)
		goto error;
	trans->flags |= J_TRANS_COMMITTED;
	trans->flags &= ~J_TRANS_OPEN;
	trans->block++;

	ext2fs_set_feature_journal_needs_recovery(trans->fs->super);
	ext2fs_mark_super_dirty(trans->fs);
error:
	if (cbh)
		brelse(cbh);
	brelse(bh);
	return err;
}

static errcode_t journal_add_revoke_to_trans(journal_transaction_t *trans,
					     blk64_t *revoke_list,
					     size_t revoke_len)
{
	journal_revoke_header_t *jrb;
	void *buf;
	size_t i, offset;
	blk64_t curr_blk;
	unsigned int sz;
	unsigned csum_size = 0;
	struct buffer_head *bh;
	errcode_t err;

	JOURNAL_CHECK_TRANS_MAGIC(trans);

	if ((trans->flags & J_TRANS_COMMITTED) ||
	    !(trans->flags & J_TRANS_OPEN))
		return EXT2_ET_INVALID_ARGUMENT;

	if (revoke_len == 0)
		return 0;

	/* Do we need to leave space at the end for a checksum? */
	if (journal_has_csum_v2or3(trans->journal))
		csum_size = sizeof(struct journal_revoke_tail);

	curr_blk = trans->block;

	bh = getblk(trans->journal->j_dev, curr_blk,
		    trans->journal->j_blocksize);
	if (bh == NULL)
		return ENOMEM;
	jrb = buf = bh->b_data;
	jrb->r_header.h_magic = ext2fs_cpu_to_be32(JFS_MAGIC_NUMBER);
	jrb->r_header.h_blocktype = ext2fs_cpu_to_be32(JFS_REVOKE_BLOCK);
	jrb->r_header.h_sequence = ext2fs_cpu_to_be32(trans->tid);
	offset = sizeof(*jrb);

	if (jfs_has_feature_64bit(trans->journal))
		sz = 8;
	else
		sz = 4;

	for (i = 0; i < revoke_len; i++) {
		/* Block full, write to journal */
		if (offset + sz > trans->journal->j_blocksize - csum_size) {
			jrb->r_count = ext2fs_cpu_to_be32(offset);
			jbd2_revoke_csum_set(trans->journal, bh);

			err = journal_bmap(trans->journal, curr_blk,
					   &bh->b_blocknr);
			if (err)
				goto error;
			dbg_printf("Writing revoke block at %llu:%llu\n",
				   curr_blk, bh->b_blocknr);
			mark_buffer_dirty(bh);
			ll_rw_block(WRITE, 1, &bh);
			err = bh->b_err;
			if (err)
				goto error;

			offset = sizeof(*jrb);
			curr_blk++;
		}

		if (revoke_list[i] >=
		    ext2fs_blocks_count(trans->journal->j_fs_dev->k_fs->super)) {
			err = EXT2_ET_BAD_BLOCK_NUM;
			goto error;
		}

		if (jfs_has_feature_64bit(trans->journal))
			*((__u64 *)(&((char *)buf)[offset])) =
				ext2fs_cpu_to_be64(revoke_list[i]);
		else
			*((__u32 *)(&((char *)buf)[offset])) =
				ext2fs_cpu_to_be32(revoke_list[i]);
		offset += sz;
	}

	if (offset > 0) {
		jrb->r_count = ext2fs_cpu_to_be32(offset);
		jbd2_revoke_csum_set(trans->journal, bh);

		err = journal_bmap(trans->journal, curr_blk, &bh->b_blocknr);
		if (err)
			goto error;
		dbg_printf("Writing revoke block at %llu:%llu\n",
			   curr_blk, bh->b_blocknr);
		mark_buffer_dirty(bh);
		ll_rw_block(WRITE, 1, &bh);
		err = bh->b_err;
		if (err)
			goto error;
		curr_blk++;
	}

error:
	trans->block = curr_blk;
	brelse(bh);
	return err;
}

static errcode_t journal_add_blocks_to_trans(journal_transaction_t *trans,
				      blk64_t *block_list, size_t block_len,
				      FILE *fp)
{
	blk64_t curr_blk, jdb_blk;
	size_t i, j;
	int csum_size = 0;
	journal_header_t *jdb;
	journal_block_tag_t *jdbt;
	int tag_bytes;
	void *buf = NULL, *jdb_buf = NULL;
	struct buffer_head *bh = NULL, *data_bh;
	errcode_t err;

	JOURNAL_CHECK_TRANS_MAGIC(trans);

	if ((trans->flags & J_TRANS_COMMITTED) ||
	    !(trans->flags & J_TRANS_OPEN))
		return EXT2_ET_INVALID_ARGUMENT;

	if (block_len == 0)
		return 0;

	/* Do we need to leave space at the end for a checksum? */
	if (journal_has_csum_v2or3(trans->journal))
		csum_size = sizeof(struct journal_block_tail);

	curr_blk = jdb_blk = trans->block;

	data_bh = getblk(trans->journal->j_dev, curr_blk,
			 trans->journal->j_blocksize);
	if (data_bh == NULL)
		return ENOMEM;
	buf = data_bh->b_data;

	/* write the descriptor block header */
	bh = getblk(trans->journal->j_dev, curr_blk,
		    trans->journal->j_blocksize);
	if (bh == NULL) {
		err = ENOMEM;
		goto error;
	}
	jdb = jdb_buf = bh->b_data;
	jdb->h_magic = ext2fs_cpu_to_be32(JFS_MAGIC_NUMBER);
	jdb->h_blocktype = ext2fs_cpu_to_be32(JFS_DESCRIPTOR_BLOCK);
	jdb->h_sequence = ext2fs_cpu_to_be32(trans->tid);
	jdbt = (journal_block_tag_t *)(jdb + 1);

	curr_blk++;
	for (i = 0; i < block_len; i++) {
		j = fread(data_bh->b_data, trans->journal->j_blocksize, 1, fp);
		if (j != 1) {
			err = errno;
			goto error;
		}

		tag_bytes = journal_tag_bytes(trans->journal);

		/* No space left in descriptor block, write it out */
		if ((char *)jdbt + tag_bytes >
		    (char *)jdb_buf + trans->journal->j_blocksize - csum_size) {
			jbd2_descr_block_csum_set(trans->journal, bh);
			err = journal_bmap(trans->journal, jdb_blk,
					   &bh->b_blocknr);
			if (err)
				goto error;
			dbg_printf("Writing descriptor block at %llu:%llu\n",
				   jdb_blk, bh->b_blocknr);
			mark_buffer_dirty(bh);
			ll_rw_block(WRITE, 1, &bh);
			err = bh->b_err;
			if (err)
				goto error;

			jdbt = (journal_block_tag_t *)(jdb + 1);
			jdb_blk = curr_blk;
			curr_blk++;
		}

		if (block_list[i] >=
		    ext2fs_blocks_count(trans->journal->j_fs_dev->k_fs->super)) {
			err = EXT2_ET_BAD_BLOCK_NUM;
			goto error;
		}

		/* Fill out the block tag */
		jdbt->t_blocknr = ext2fs_cpu_to_be32(block_list[i] & 0xFFFFFFFF);
		jdbt->t_flags = 0;
		if (jdbt != (journal_block_tag_t *)(jdb + 1))
			jdbt->t_flags |= ext2fs_cpu_to_be16(JFS_FLAG_SAME_UUID);
		else {
			memcpy(jdbt + tag_bytes,
			       trans->journal->j_superblock->s_uuid,
			       sizeof(trans->journal->j_superblock->s_uuid));
			tag_bytes += 16;
		}
		if (i == block_len - 1)
			jdbt->t_flags |= ext2fs_cpu_to_be16(JFS_FLAG_LAST_TAG);
		if (*((__u32 *)buf) == ext2fs_cpu_to_be32(JFS_MAGIC_NUMBER)) {
			*((__u32 *)buf) = 0;
			jdbt->t_flags |= ext2fs_cpu_to_be16(JFS_FLAG_ESCAPE);
		}
		if (jfs_has_feature_64bit(trans->journal))
			jdbt->t_blocknr_high = ext2fs_cpu_to_be32(block_list[i] >> 32);
		jbd2_block_tag_csum_set(trans->journal, jdbt, data_bh,
					trans->tid);

		/* Write the data block */
		err = journal_bmap(trans->journal, curr_blk,
				   &data_bh->b_blocknr);
		if (err)
			goto error;
		dbg_printf("Writing data block %llu at %llu:%llu tag %d\n",
			   block_list[i], curr_blk, data_bh->b_blocknr,
			   tag_bytes);
		mark_buffer_dirty(data_bh);
		ll_rw_block(WRITE, 1, &data_bh);
		err = data_bh->b_err;
		if (err)
			goto error;

		curr_blk++;
		jdbt = (journal_block_tag_t *)(((char *)jdbt) + tag_bytes);
	}

	/* Write out the last descriptor block */
	if (jdbt != (journal_block_tag_t *)(jdb + 1)) {
		jbd2_descr_block_csum_set(trans->journal, bh);
		err = journal_bmap(trans->journal, jdb_blk, &bh->b_blocknr);
		if (err)
			goto error;
		dbg_printf("Writing descriptor block at %llu:%llu\n",
			   jdb_blk, bh->b_blocknr);
		mark_buffer_dirty(bh);
		ll_rw_block(WRITE, 1, &bh);
		err = bh->b_err;
		if (err)
			goto error;
	}

error:
	trans->block = curr_blk;
	if (bh)
		brelse(bh);
	brelse(data_bh);
	return err;
}

static blk64_t journal_guess_blocks(journal_t *journal, blk64_t data_blocks,
				    blk64_t revoke_blocks)
{
	blk64_t ret = 1;
	unsigned int bs, sz;

	/* Estimate # of revoke blocks */
	bs = journal->j_blocksize;
	if (journal_has_csum_v2or3(journal))
		bs -= sizeof(struct journal_revoke_tail);
	sz = jfs_has_feature_64bit(journal) ? sizeof(__u64) : sizeof(__u32);
	ret += revoke_blocks * sz / bs;

	/* Estimate # of data blocks */
	bs = journal->j_blocksize - 16;
	if (journal_has_csum_v2or3(journal))
		bs -= sizeof(struct journal_block_tail);
	sz = journal_tag_bytes(journal);
	ret += data_blocks * sz / bs;

	ret += data_blocks;

	return ret;
}

static errcode_t journal_open_trans(journal_t *journal,
				    journal_transaction_t *trans,
				    blk64_t blocks)
{
	trans->fs = journal->j_fs_dev->k_fs;
	trans->journal = journal;
	trans->flags = J_TRANS_OPEN;

	if (journal->j_tail == 0) {
		/* Clean journal, start at the tail */
		trans->tid = journal->j_tail_sequence;
		trans->start = journal->j_first;
	} else {
		/* Put new transaction at the head of the list */
		trans->tid = journal->j_transaction_sequence;
		trans->start = journal->j_head;
	}

	trans->block = trans->start;
	if (trans->start + blocks > journal->j_last)
		return ENOSPC;
	trans->end = trans->block + blocks;
	journal_dump_trans(trans, "new transaction");

	trans->magic = J_TRANS_MAGIC;
	return 0;
}

static errcode_t journal_close_trans(journal_transaction_t *trans)
{
	journal_t *journal;

	JOURNAL_CHECK_TRANS_MAGIC(trans);

	if (!(trans->flags & J_TRANS_COMMITTED))
		return 0;

	journal = trans->journal;
	if (journal->j_tail == 0) {
		/* Update the tail */
		journal->j_tail_sequence = trans->tid;
		journal->j_tail = trans->start;
		journal->j_superblock->s_start = ext2fs_cpu_to_be32(trans->start);
	}

	/* Update the head */
	journal->j_head = trans->end + 1;
	journal->j_transaction_sequence = trans->tid + 1;

	trans->magic = 0;

	/* Mark ourselves as needing recovery */
	if (!ext2fs_has_feature_journal_needs_recovery(trans->fs->super)) {
		ext2fs_set_feature_journal_needs_recovery(trans->fs->super);
		ext2fs_mark_super_dirty(trans->fs);
	}

	return 0;
}

#define JOURNAL_WRITE_NO_COMMIT		1
static errcode_t journal_write(journal_t *journal,
			       int flags, blk64_t *block_list,
			       size_t block_len, blk64_t *revoke_list,
			       size_t revoke_len, FILE *fp)
{
	blk64_t blocks;
	journal_transaction_t trans;
	errcode_t err;

	if (revoke_len > 0) {
		jfs_set_feature_revoke(journal);
		mark_buffer_dirty(journal->j_sb_buffer);
	}

	blocks = journal_guess_blocks(journal, block_len, revoke_len);
	err = journal_open_trans(journal, &trans, blocks);
	if (err)
		goto error;

	err = journal_add_blocks_to_trans(&trans, block_list, block_len, fp);
	if (err)
		goto error;

	err = journal_add_revoke_to_trans(&trans, revoke_list, revoke_len);
	if (err)
		goto error;

	if (!(flags & JOURNAL_WRITE_NO_COMMIT)) {
		err = journal_commit_trans(&trans);
		if (err)
			goto error;
	}

	err = journal_close_trans(&trans);
	if (err)
		goto error;
error:
	return err;
}

void do_journal_write(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		      void *infop EXT2FS_ATTR((unused)))
{
	blk64_t *blist = NULL, *rlist = NULL;
	size_t bn = 0, rn = 0;
	FILE *fp = NULL;
	int opt;
	int flags = 0;
	errcode_t err;

	if (current_journal == NULL) {
		printf("Journal not open.\n");
		return;
	}

	reset_getopt();
	while ((opt = getopt(argc, argv, "b:r:c")) != -1) {
		switch (opt) {
		case 'b':
			err = read_list(optarg, &blist, &bn);
			if (err)
				com_err(argv[0], err,
					"while reading block list");
			break;
		case 'r':
			err = read_list(optarg, &rlist, &rn);
			if (err)
				com_err(argv[0], err,
					"while reading revoke list");
			break;
		case 'c':
			flags |= JOURNAL_WRITE_NO_COMMIT;
			break;
		default:
			printf("%s [-b blocks] [-r revoke] [-c] file\n",
			       argv[0]);
			printf("-b: Write these blocks into transaction.\n");
			printf("-c: Do not commit transaction.\n");
			printf("-r: Revoke these blocks from transaction.\n");

			goto out;
		}
	}

	if (bn > 0 && optind != argc - 1) {
		printf("Need a file to read blocks from.\n");
		return;
	}

	if (bn > 0) {
		fp = fopen(argv[optind], "r");
		if (fp == NULL) {
			com_err(argv[0], errno,
				"while opening journal data file");
			goto out;
		}
	}

	err = journal_write(current_journal, flags, blist, bn,
			    rlist, rn, fp);
	if (err)
		com_err("journal_write", err, "while writing journal");

	if (fp)
		fclose(fp);
out:
	if (blist)
		free(blist);
	if (rlist)
		free(rlist);
}

/* Make sure we wrap around the log correctly! */
#define wrap(journal, var)						\
do {									\
	if (var >= (journal)->j_last)					\
		var -= ((journal)->j_last - (journal)->j_first);	\
} while (0)

/*
 * Count the number of in-use tags in a journal descriptor block.
 */

static int count_tags(journal_t *journal, char *buf)
{
	char			*tagp;
	journal_block_tag_t	*tag;
	int			nr = 0, size = journal->j_blocksize;
	int			tag_bytes = journal_tag_bytes(journal);

	if (journal_has_csum_v2or3(journal))
		size -= sizeof(struct journal_block_tail);

	tagp = buf + sizeof(journal_header_t);

	while ((tagp - buf + tag_bytes) <= size) {
		tag = (journal_block_tag_t *) tagp;

		nr++;
		tagp += tag_bytes;
		if (!(tag->t_flags & ext2fs_cpu_to_be16(JFS_FLAG_SAME_UUID)))
			tagp += 16;

		if (tag->t_flags & ext2fs_cpu_to_be16(JFS_FLAG_LAST_TAG))
			break;
	}

	return nr;
}

static errcode_t journal_find_head(journal_t *journal)
{
	unsigned int		next_commit_ID;
	blk64_t			next_log_block, head_block;
	int			err;
	journal_superblock_t	*sb;
	journal_header_t	*tmp;
	struct buffer_head	*bh;
	unsigned int		sequence;
	int			blocktype;

	/*
	 * First thing is to establish what we expect to find in the log
	 * (in terms of transaction IDs), and where (in terms of log
	 * block offsets): query the superblock.
	 */

	sb = journal->j_superblock;
	next_commit_ID = ext2fs_be32_to_cpu(sb->s_sequence);
	next_log_block = ext2fs_be32_to_cpu(sb->s_start);
	head_block = next_log_block;

	if (next_log_block == 0)
		return 0;

	bh = getblk(journal->j_dev, 0, journal->j_blocksize);
	if (bh == NULL)
		return ENOMEM;

	/*
	 * Now we walk through the log, transaction by transaction,
	 * making sure that each transaction has a commit block in the
	 * expected place.  Each complete transaction gets replayed back
	 * into the main filesystem.
	 */
	while (1) {
		dbg_printf("Scanning for sequence ID %u at %lu/%lu\n",
			  next_commit_ID, (unsigned long)next_log_block,
			  journal->j_last);

		/* Skip over each chunk of the transaction looking
		 * either the next descriptor block or the final commit
		 * record. */
		err = journal_bmap(journal, next_log_block, &bh->b_blocknr);
		if (err)
			goto err;
		mark_buffer_uptodate(bh, 0);
		ll_rw_block(READ, 1, &bh);
		err = bh->b_err;
		if (err)
			goto err;

		next_log_block++;
		wrap(journal, next_log_block);

		/* What kind of buffer is it?
		 *
		 * If it is a descriptor block, check that it has the
		 * expected sequence number.  Otherwise, we're all done
		 * here. */

		tmp = (journal_header_t *)bh->b_data;

		if (tmp->h_magic != ext2fs_cpu_to_be32(JFS_MAGIC_NUMBER)) {
			dbg_printf("JBD2: wrong magic 0x%x\n", tmp->h_magic);
			goto err;
		}

		blocktype = ext2fs_be32_to_cpu(tmp->h_blocktype);
		sequence = ext2fs_be32_to_cpu(tmp->h_sequence);
		dbg_printf("Found magic %d, sequence %d\n",
			  blocktype, sequence);

		if (sequence != next_commit_ID) {
			dbg_printf("JBD2: Wrong sequence %d (wanted %d)\n",
				   sequence, next_commit_ID);
			goto err;
		}

		/* OK, we have a valid descriptor block which matches
		 * all of the sequence number checks.  What are we going
		 * to do with it?  That depends on the pass... */

		switch (blocktype) {
		case JFS_DESCRIPTOR_BLOCK:
			next_log_block += count_tags(journal, bh->b_data);
			wrap(journal, next_log_block);
			continue;

		case JFS_COMMIT_BLOCK:
			head_block = next_log_block;
			next_commit_ID++;
			continue;

		case JFS_REVOKE_BLOCK:
			continue;

		default:
			dbg_printf("Unrecognised magic %d, end of scan.\n",
				  blocktype);
			err = -EINVAL;
			goto err;
		}
	}

err:
	if (err == 0) {
		dbg_printf("head seq=%d blk=%llu\n", next_commit_ID,
			   head_block);
		journal->j_transaction_sequence = next_commit_ID;
		journal->j_head = head_block;
	}
	brelse(bh);
	return err;
}

static void update_journal_csum(journal_t *journal, int ver)
{
	journal_superblock_t *jsb;

	if (journal->j_format_version < 2)
		return;

	if (journal->j_tail != 0 ||
	    ext2fs_has_feature_journal_needs_recovery(
					journal->j_fs_dev->k_fs->super)) {
		printf("Journal needs recovery, will not add csums.\n");
		return;
	}

	/* metadata_csum implies journal csum v3 */
	jsb = journal->j_superblock;
	if (ext2fs_has_feature_metadata_csum(journal->j_fs_dev->k_fs->super)) {
		printf("Setting csum v%d\n", ver);
		switch (ver) {
		case 2:
			jfs_clear_feature_csum3(journal);
			jfs_set_feature_csum2(journal);
			jfs_clear_feature_checksum(journal);
			break;
		case 3:
			jfs_set_feature_csum3(journal);
			jfs_clear_feature_csum2(journal);
			jfs_clear_feature_checksum(journal);
			break;
		default:
			printf("Unknown checksum v%d\n", ver);
			break;
		}
		journal->j_superblock->s_checksum_type = JBD2_CRC32C_CHKSUM;
		journal->j_csum_seed = jbd2_chksum(journal, ~0, jsb->s_uuid,
						   sizeof(jsb->s_uuid));
	} else {
		jfs_clear_feature_csum3(journal);
		jfs_clear_feature_csum2(journal);
		jfs_set_feature_checksum(journal);
	}
}

static void update_uuid(journal_t *journal)
{
	size_t z;
	ext2_filsys fs;

	if (journal->j_format_version < 2)
		return;

	for (z = 0; z < sizeof(journal->j_superblock->s_uuid); z++)
		if (journal->j_superblock->s_uuid[z])
			break;
	if (z == 0)
		return;

	fs = journal->j_fs_dev->k_fs;
	if (!ext2fs_has_feature_64bit(fs->super))
		return;

	if (jfs_has_feature_64bit(journal) &&
	    ext2fs_has_feature_64bit(fs->super))
		return;

	if (journal->j_tail != 0 ||
	    ext2fs_has_feature_journal_needs_recovery(fs->super)) {
		printf("Journal needs recovery, will not set 64bit.\n");
		return;
	}

	memcpy(journal->j_superblock->s_uuid, fs->super->s_uuid,
	       sizeof(fs->super->s_uuid));
}

static void update_64bit_flag(journal_t *journal)
{
	if (journal->j_format_version < 2)
		return;

	if (!ext2fs_has_feature_64bit(journal->j_fs_dev->k_fs->super))
		return;

	if (jfs_has_feature_64bit(journal) &&
	    ext2fs_has_feature_64bit(journal->j_fs_dev->k_fs->super))
		return;

	if (journal->j_tail != 0 ||
	    ext2fs_has_feature_journal_needs_recovery(
				journal->j_fs_dev->k_fs->super)) {
		printf("Journal needs recovery, will not set 64bit.\n");
		return;
	}

	jfs_set_feature_64bit(journal);
}

void do_journal_open(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		     void *infop EXT2FS_ATTR((unused)))
{
	int opt, enable_csum = 0, csum_ver = 3;
	journal_t *journal;
	errcode_t err;

	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	if (check_fs_bitmaps(argv[0]))
		return;
	if (current_journal) {
		printf("Journal is already open.\n");
		return;
	}
	if (!ext2fs_has_feature_journal(current_fs->super)) {
		printf("Journalling is not enabled on this filesystem.\n");
		return;
	}

	reset_getopt();
	while ((opt = getopt(argc, argv, "cv:f:")) != -1) {
		switch (opt) {
		case 'c':
			enable_csum = 1;
			break;
		case 'f':
			if (current_fs->journal_name)
				free(current_fs->journal_name);
			current_fs->journal_name = strdup(optarg);
			break;
		case 'v':
			csum_ver = atoi(optarg);
			if (csum_ver != 2 && csum_ver != 3) {
				printf("Unknown journal csum v%d\n", csum_ver);
				csum_ver = 3;
			}
			break;
		default:
			printf("%s: [-c] [-v ver] [-f ext_jnl]\n", argv[0]);
			printf("-c: Enable journal checksumming.\n");
			printf("-v: Use this version checksum format.\n");
			printf("-f: Load this external journal.\n");
		}
	}

	err = ext2fs_open_journal(current_fs, &current_journal);
	if (err) {
		com_err(argv[0], err, "while opening journal");
		return;
	}
	journal = current_journal;

	dbg_printf("JOURNAL: seq=%u tailseq=%u start=%lu first=%lu "
		   "maxlen=%lu\n", journal->j_tail_sequence,
		   journal->j_transaction_sequence, journal->j_tail,
		   journal->j_first, journal->j_last);

	update_uuid(journal);
	update_64bit_flag(journal);
	if (enable_csum)
		update_journal_csum(journal, csum_ver);

	err = journal_find_head(journal);
	if (err)
		com_err(argv[0], err, "while examining journal");
}

void do_journal_close(int argc EXT2FS_ATTR((unused)),
		      char *argv[] EXT2FS_ATTR((unused)),
		      int sci_idx EXT2FS_ATTR((unused)),
		      void *infop EXT2FS_ATTR((unused)))
{
	if (current_journal == NULL) {
		printf("Journal not open.\n");
		return;
	}

	ext2fs_close_journal(current_fs, &current_journal);
}

void do_journal_run(int argc EXT2FS_ATTR((unused)), char *argv[],
		    int sci_idx EXT2FS_ATTR((unused)),
		    void *infop EXT2FS_ATTR((unused)))
{
	errcode_t err;

	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	if (check_fs_bitmaps(argv[0]))
		return;
	if (current_journal) {
		printf("Please close the journal before recovering it.\n");
		return;
	}

	err = ext2fs_run_ext3_journal(&current_fs);
	if (err)
		com_err("journal_run", err, "while recovering journal");
	else {
		ext2fs_clear_feature_journal_needs_recovery(current_fs->super);
		ext2fs_mark_super_dirty(current_fs);
	}
}
