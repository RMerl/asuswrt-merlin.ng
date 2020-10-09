/*
 * journal.c --- code for handling the "ext3" journal
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

#include "config.h"
#ifdef HAVE_SYS_MOUNT_H
#include <sys/param.h>
#include <sys/mount.h>
#define MNT_FL (MS_MGC_VAL | MS_RDONLY)
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#define E2FSCK_INCLUDE_INLINE_FUNCS
#include "uuid/uuid.h"
#include "journal.h"

#ifdef CONFIG_JBD_DEBUG		/* Enabled by configure --enable-jfs-debug */
static int bh_count = 0;
#endif

#if EXT2_FLAT_INCLUDES
#include "blkid.h"
#else
#include "blkid/blkid.h"
#endif

/*
 * Define USE_INODE_IO to use the inode_io.c / fileio.c codepaths.
 * This creates a larger static binary, and a smaller binary using
 * shared libraries.  It's also probably slightly less CPU-efficient,
 * which is why it's not on by default.  But, it's a good way of
 * testing the functions in inode_io.c and fileio.c.
 */
#undef USE_INODE_IO

/* Checksumming functions */
static int ext2fs_journal_verify_csum_type(journal_t *j,
					   journal_superblock_t *jsb)
{
	if (!journal_has_csum_v2or3(j))
		return 1;

	return jsb->s_checksum_type == JBD2_CRC32C_CHKSUM;
}

static __u32 ext2fs_journal_sb_csum(journal_superblock_t *jsb)
{
	__u32 crc, old_crc;

	old_crc = jsb->s_checksum;
	jsb->s_checksum = 0;
	crc = ext2fs_crc32c_le(~0, (unsigned char *)jsb,
			       sizeof(journal_superblock_t));
	jsb->s_checksum = old_crc;

	return crc;
}

static int ext2fs_journal_sb_csum_verify(journal_t *j,
					 journal_superblock_t *jsb)
{
	__u32 provided, calculated;

	if (!journal_has_csum_v2or3(j))
		return 1;

	provided = ext2fs_be32_to_cpu(jsb->s_checksum);
	calculated = ext2fs_journal_sb_csum(jsb);

	return provided == calculated;
}

static errcode_t ext2fs_journal_sb_csum_set(journal_t *j,
					    journal_superblock_t *jsb)
{
	__u32 crc;

	if (!journal_has_csum_v2or3(j))
		return 0;

	crc = ext2fs_journal_sb_csum(jsb);
	jsb->s_checksum = ext2fs_cpu_to_be32(crc);
	return 0;
}

/* Kernel compatibility functions for handling the journal.  These allow us
 * to use the recovery.c file virtually unchanged from the kernel, so we
 * don't have to do much to keep kernel and user recovery in sync.
 */
int journal_bmap(journal_t *journal, blk64_t block, unsigned long long *phys)
{
#ifdef USE_INODE_IO
	*phys = block;
	return 0;
#else
	struct inode	*inode = journal->j_inode;
	errcode_t	retval;
	blk64_t		pblk;

	if (!inode) {
		*phys = block;
		return 0;
	}

	retval = ext2fs_bmap2(inode->i_fs, inode->i_ino,
			      &inode->i_ext2, NULL, 0, block, 0, &pblk);
	*phys = pblk;
	return (int) retval;
#endif
}

struct buffer_head *getblk(kdev_t kdev, blk64_t blocknr, int blocksize)
{
	struct buffer_head *bh;
	int bufsize = sizeof(*bh) + kdev->k_fs->blocksize -
		sizeof(bh->b_data);
	errcode_t retval;

	retval = ext2fs_get_memzero(bufsize, &bh);
	if (retval)
		return NULL;

#ifdef CONFIG_JBD_DEBUG
	if (journal_enable_debug >= 3)
		bh_count++;
#endif
	jfs_debug(4, "getblk for block %llu (%d bytes)(total %d)\n",
		  (unsigned long long) blocknr, blocksize, bh_count);

	bh->b_fs = kdev->k_fs;
	if (kdev->k_dev == K_DEV_FS)
		bh->b_io = kdev->k_fs->io;
	else
		bh->b_io = kdev->k_fs->journal_io;
	bh->b_size = blocksize;
	bh->b_blocknr = blocknr;

	return bh;
}

int sync_blockdev(kdev_t kdev)
{
	io_channel	io;

	if (kdev->k_dev == K_DEV_FS)
		io = kdev->k_fs->io;
	else
		io = kdev->k_fs->journal_io;

	return io_channel_flush(io) ? EIO : 0;
}

void ll_rw_block(int rw, int nr, struct buffer_head *bhp[])
{
	errcode_t retval;
	struct buffer_head *bh;

	for (; nr > 0; --nr) {
		bh = *bhp++;
		if (rw == READ && !bh->b_uptodate) {
			jfs_debug(3, "reading block %llu/%p\n",
				  bh->b_blocknr, (void *) bh);
			retval = io_channel_read_blk64(bh->b_io,
						     bh->b_blocknr,
						     1, bh->b_data);
			if (retval) {
				com_err(bh->b_fs->device_name, retval,
					"while reading block %llu\n",
					bh->b_blocknr);
				bh->b_err = (int) retval;
				continue;
			}
			bh->b_uptodate = 1;
		} else if (rw == WRITE && bh->b_dirty) {
			jfs_debug(3, "writing block %llu/%p\n",
				  bh->b_blocknr,
				  (void *) bh);
			retval = io_channel_write_blk64(bh->b_io,
						      bh->b_blocknr,
						      1, bh->b_data);
			if (retval) {
				com_err(bh->b_fs->device_name, retval,
					"while writing block %llu\n",
					bh->b_blocknr);
				bh->b_err = (int) retval;
				continue;
			}
			bh->b_dirty = 0;
			bh->b_uptodate = 1;
		} else {
			jfs_debug(3, "no-op %s for block %llu\n",
				  rw == READ ? "read" : "write",
				  bh->b_blocknr);
		}
	}
}

void mark_buffer_dirty(struct buffer_head *bh)
{
	bh->b_dirty = 1;
}

static void mark_buffer_clean(struct buffer_head *bh)
{
	bh->b_dirty = 0;
}

void brelse(struct buffer_head *bh)
{
	if (bh->b_dirty)
		ll_rw_block(WRITE, 1, &bh);
	jfs_debug(3, "freeing block %llu/%p (total %d)\n",
		  bh->b_blocknr, (void *) bh, --bh_count);
	ext2fs_free_mem(&bh);
}

int buffer_uptodate(struct buffer_head *bh)
{
	return bh->b_uptodate;
}

void mark_buffer_uptodate(struct buffer_head *bh, int val)
{
	bh->b_uptodate = val;
}

void wait_on_buffer(struct buffer_head *bh)
{
	if (!bh->b_uptodate)
		ll_rw_block(READ, 1, &bh);
}


static void ext2fs_clear_recover(ext2_filsys fs, int error)
{
	ext2fs_clear_feature_journal_needs_recovery(fs->super);

	/* if we had an error doing journal recovery, we need a full fsck */
	if (error)
		fs->super->s_state &= ~EXT2_VALID_FS;
	/*
	 * If we replayed the journal by definition the file system
	 * was mounted since the last time it was checked
	 */
	if (fs->super->s_lastcheck >= fs->super->s_mtime)
		fs->super->s_lastcheck = fs->super->s_mtime - 1;
	ext2fs_mark_super_dirty(fs);
}

/*
 * This is a helper function to check the validity of the journal.
 */
struct process_block_struct {
	e2_blkcnt_t	last_block;
};

static int process_journal_block(ext2_filsys fs,
				 blk64_t	*block_nr,
				 e2_blkcnt_t blockcnt,
				 blk64_t ref_block EXT2FS_ATTR((unused)),
				 int ref_offset EXT2FS_ATTR((unused)),
				 void *priv_data)
{
	struct process_block_struct *p;
	blk64_t	blk = *block_nr;

	p = (struct process_block_struct *) priv_data;

	if (!blk || blk < fs->super->s_first_data_block ||
	    blk >= ext2fs_blocks_count(fs->super))
		return BLOCK_ABORT;

	if (blockcnt >= 0)
		p->last_block = blockcnt;
	return 0;
}

static errcode_t ext2fs_get_journal(ext2_filsys fs, journal_t **ret_journal)
{
	struct process_block_struct pb;
	struct ext2_super_block *sb = fs->super;
	struct ext2_super_block jsuper;
	struct buffer_head	*bh;
	struct inode		*j_inode = NULL;
	struct kdev_s		*dev_fs = NULL, *dev_journal;
	const char		*journal_name = 0;
	journal_t		*journal = NULL;
	errcode_t		retval = 0;
	io_manager		io_ptr = 0;
	unsigned long long	start = 0;
	int			ext_journal = 0;
	int			tried_backup_jnl = 0;

	retval = ext2fs_get_memzero(sizeof(journal_t), &journal);
	if (retval)
		return retval;

	retval = ext2fs_get_memzero(2 * sizeof(struct kdev_s), &dev_fs);
	if (retval)
		goto errout;
	dev_journal = dev_fs+1;

	dev_fs->k_fs = dev_journal->k_fs = fs;
	dev_fs->k_dev = K_DEV_FS;
	dev_journal->k_dev = K_DEV_JOURNAL;

	journal->j_dev = dev_journal;
	journal->j_fs_dev = dev_fs;
	journal->j_inode = NULL;
	journal->j_blocksize = fs->blocksize;

	if (uuid_is_null(sb->s_journal_uuid)) {
		if (!sb->s_journal_inum) {
			retval = EXT2_ET_BAD_INODE_NUM;
			goto errout;
		}
		retval = ext2fs_get_memzero(sizeof(*j_inode), &j_inode);
		if (retval)
			goto errout;

		j_inode->i_fs = fs;
		j_inode->i_ino = sb->s_journal_inum;

		retval = ext2fs_read_inode(fs, sb->s_journal_inum,
					   &j_inode->i_ext2);
		if (retval) {
try_backup_journal:
			if (sb->s_jnl_backup_type != EXT3_JNL_BACKUP_BLOCKS ||
			    tried_backup_jnl)
				goto errout;
			memset(&j_inode->i_ext2, 0, sizeof(struct ext2_inode));
			memcpy(&j_inode->i_ext2.i_block[0], sb->s_jnl_blocks,
			       EXT2_N_BLOCKS*4);
			j_inode->i_ext2.i_size_high = sb->s_jnl_blocks[15];
			j_inode->i_ext2.i_size = sb->s_jnl_blocks[16];
			j_inode->i_ext2.i_links_count = 1;
			j_inode->i_ext2.i_mode = LINUX_S_IFREG | 0600;
			tried_backup_jnl++;
		}
		if (!j_inode->i_ext2.i_links_count ||
		    !LINUX_S_ISREG(j_inode->i_ext2.i_mode)) {
			retval = EXT2_ET_NO_JOURNAL;
			goto try_backup_journal;
		}
		if (EXT2_I_SIZE(&j_inode->i_ext2) / journal->j_blocksize <
		    JFS_MIN_JOURNAL_BLOCKS) {
			retval = EXT2_ET_JOURNAL_TOO_SMALL;
			goto try_backup_journal;
		}
		pb.last_block = -1;
		retval = ext2fs_block_iterate3(fs, j_inode->i_ino,
					       BLOCK_FLAG_HOLE, 0,
					       process_journal_block, &pb);
		if ((pb.last_block + 1) * fs->blocksize <
		    (int) EXT2_I_SIZE(&j_inode->i_ext2)) {
			retval = EXT2_ET_JOURNAL_TOO_SMALL;
			goto try_backup_journal;
		}
		if (tried_backup_jnl && (fs->flags & EXT2_FLAG_RW)) {
			retval = ext2fs_write_inode(fs, sb->s_journal_inum,
						    &j_inode->i_ext2);
			if (retval)
				goto errout;
		}

		journal->j_maxlen = EXT2_I_SIZE(&j_inode->i_ext2) /
			journal->j_blocksize;

#ifdef USE_INODE_IO
		retval = ext2fs_inode_io_intern2(fs, sb->s_journal_inum,
						 &j_inode->i_ext2,
						 &journal_name);
		if (retval)
			goto errout;

		io_ptr = inode_io_manager;
#else
		journal->j_inode = j_inode;
		fs->journal_io = fs->io;
		retval = (errcode_t)journal_bmap(journal, 0, &start);
		if (retval)
			goto errout;
#endif
	} else {
		ext_journal = 1;
		if (!fs->journal_name) {
			char uuid[37];
			blkid_cache blkid;

			blkid_get_cache(&blkid, NULL);
			uuid_unparse(sb->s_journal_uuid, uuid);
			fs->journal_name = blkid_get_devname(blkid,
							      "UUID", uuid);
			if (!fs->journal_name)
				fs->journal_name = blkid_devno_to_devname(sb->s_journal_dev);
			blkid_put_cache(blkid);
		}
		journal_name = fs->journal_name;

		if (!journal_name) {
			retval = EXT2_ET_LOAD_EXT_JOURNAL;
			goto errout;
		}

		jfs_debug(1, "Using journal file %s\n", journal_name);
		io_ptr = unix_io_manager;
	}

#if 0
	test_io_backing_manager = io_ptr;
	io_ptr = test_io_manager;
#endif
#ifndef USE_INODE_IO
	if (ext_journal)
#endif
	{
		retval = io_ptr->open(journal_name, fs->flags & EXT2_FLAG_RW,
				      &fs->journal_io);
	}
	if (retval)
		goto errout;

	io_channel_set_blksize(fs->journal_io, fs->blocksize);

	if (ext_journal) {
		blk64_t maxlen;

		start = ext2fs_journal_sb_start(fs->blocksize) - 1;
		bh = getblk(dev_journal, start, fs->blocksize);
		if (!bh) {
			retval = EXT2_ET_NO_MEMORY;
			goto errout;
		}
		ll_rw_block(READ, 1, &bh);
		retval = bh->b_err;
		if (retval) {
			brelse(bh);
			goto errout;
		}
		memcpy(&jsuper, start ? bh->b_data :
				bh->b_data + SUPERBLOCK_OFFSET,
		       sizeof(jsuper));
#ifdef WORDS_BIGENDIAN
		if (jsuper.s_magic == ext2fs_swab16(EXT2_SUPER_MAGIC))
			ext2fs_swap_super(&jsuper);
#endif
		if (jsuper.s_magic != EXT2_SUPER_MAGIC ||
		    !ext2fs_has_feature_journal_dev(&jsuper)) {
			retval = EXT2_ET_LOAD_EXT_JOURNAL;
			brelse(bh);
			goto errout;
		}
		/* Make sure the journal UUID is correct */
		if (memcmp(jsuper.s_uuid, fs->super->s_journal_uuid,
			   sizeof(jsuper.s_uuid))) {
			retval = EXT2_ET_LOAD_EXT_JOURNAL;
			brelse(bh);
			goto errout;
		}

		/* Check the superblock checksum */
		if (ext2fs_has_feature_metadata_csum(&jsuper)) {
			struct struct_ext2_filsys fsx;
			struct ext2_super_block	superx;
			void *p;

			p = start ? bh->b_data : bh->b_data + SUPERBLOCK_OFFSET;
			memcpy(&fsx, fs, sizeof(fsx));
			memcpy(&superx, fs->super, sizeof(superx));
			fsx.super = &superx;
			ext2fs_set_feature_metadata_csum(fsx.super);
			if (!ext2fs_superblock_csum_verify(&fsx, p)) {
				retval = EXT2_ET_LOAD_EXT_JOURNAL;
				brelse(bh);
				goto errout;
			}
		}
		brelse(bh);

		maxlen = ext2fs_blocks_count(&jsuper);
		journal->j_maxlen = (maxlen < 1ULL << 32) ? maxlen :
				    (1ULL << 32) - 1;
		start++;
	}

	bh = getblk(dev_journal, start, journal->j_blocksize);
	if (!bh) {
		retval = EXT2_ET_NO_MEMORY;
		goto errout;
	}

	journal->j_sb_buffer = bh;
	journal->j_superblock = (journal_superblock_t *)bh->b_data;

#ifdef USE_INODE_IO
	if (j_inode)
		ext2fs_free_mem(&j_inode);
#endif

	*ret_journal = journal;
	return 0;

errout:
	if (dev_fs)
		ext2fs_free_mem(&dev_fs);
	if (j_inode)
		ext2fs_free_mem(&j_inode);
	if (journal)
		ext2fs_free_mem(&journal);
	return retval;
}

static errcode_t ext2fs_journal_fix_bad_inode(ext2_filsys fs)
{
	struct ext2_super_block *sb = fs->super;
	int recover = ext2fs_has_feature_journal_needs_recovery(fs->super);
	int has_journal = ext2fs_has_feature_journal(fs->super);

	if (has_journal || sb->s_journal_inum) {
		/* The journal inode is bogus, remove and force full fsck */
		return EXT2_ET_BAD_INODE_NUM;
	} else if (recover) {
		return EXT2_ET_UNSUPP_FEATURE;
	}
	return 0;
}

#define V1_SB_SIZE	0x0024
static void clear_v2_journal_fields(journal_t *journal)
{
	ext2_filsys fs = journal->j_dev->k_fs;

	memset(((char *) journal->j_superblock) + V1_SB_SIZE, 0,
	       fs->blocksize-V1_SB_SIZE);
	mark_buffer_dirty(journal->j_sb_buffer);
}


static errcode_t ext2fs_journal_load(journal_t *journal)
{
	ext2_filsys fs = journal->j_dev->k_fs;
	journal_superblock_t *jsb;
	struct buffer_head *jbh = journal->j_sb_buffer;

	ll_rw_block(READ, 1, &jbh);
	if (jbh->b_err)
		return jbh->b_err;

	jsb = journal->j_superblock;
	/* If we don't even have JFS_MAGIC, we probably have a wrong inode */
	if (jsb->s_header.h_magic != htonl(JFS_MAGIC_NUMBER))
		return ext2fs_journal_fix_bad_inode(fs);

	switch (ntohl(jsb->s_header.h_blocktype)) {
	case JFS_SUPERBLOCK_V1:
		journal->j_format_version = 1;
		if (jsb->s_feature_compat ||
		    jsb->s_feature_incompat ||
		    jsb->s_feature_ro_compat ||
		    jsb->s_nr_users)
			clear_v2_journal_fields(journal);
		break;

	case JFS_SUPERBLOCK_V2:
		journal->j_format_version = 2;
		if (ntohl(jsb->s_nr_users) > 1 &&
		    uuid_is_null(fs->super->s_journal_uuid))
			clear_v2_journal_fields(journal);
		if (ntohl(jsb->s_nr_users) > 1)
			return EXT2_ET_JOURNAL_UNSUPP_VERSION;
		break;

	/*
	 * These should never appear in a journal super block, so if
	 * they do, the journal is badly corrupted.
	 */
	case JFS_DESCRIPTOR_BLOCK:
	case JFS_COMMIT_BLOCK:
	case JFS_REVOKE_BLOCK:
		return EXT2_ET_CORRUPT_JOURNAL_SB;

	/* If we don't understand the superblock major type, but there
	 * is a magic number, then it is likely to be a new format we
	 * just don't understand, so leave it alone. */
	default:
		return EXT2_ET_JOURNAL_UNSUPP_VERSION;
	}

	if (JFS_HAS_INCOMPAT_FEATURE(journal, ~JFS_KNOWN_INCOMPAT_FEATURES))
		return EXT2_ET_UNSUPP_FEATURE;

	if (JFS_HAS_RO_COMPAT_FEATURE(journal, ~JFS_KNOWN_ROCOMPAT_FEATURES))
		return EXT2_ET_RO_UNSUPP_FEATURE;

	/* Checksum v1-3 are mutually exclusive features. */
	if (jfs_has_feature_csum2(journal) && jfs_has_feature_csum3(journal))
		return EXT2_ET_CORRUPT_JOURNAL_SB;

	if (journal_has_csum_v2or3(journal) &&
	    jfs_has_feature_checksum(journal))
		return EXT2_ET_CORRUPT_JOURNAL_SB;

	if (!ext2fs_journal_verify_csum_type(journal, jsb) ||
	    !ext2fs_journal_sb_csum_verify(journal, jsb))
		return EXT2_ET_CORRUPT_JOURNAL_SB;

	if (journal_has_csum_v2or3(journal))
		journal->j_csum_seed = jbd2_chksum(journal, ~0, jsb->s_uuid,
						   sizeof(jsb->s_uuid));

	/* We have now checked whether we know enough about the journal
	 * format to be able to proceed safely, so any other checks that
	 * fail we should attempt to recover from. */
	if (jsb->s_blocksize != htonl(journal->j_blocksize))
		return EXT2_ET_CORRUPT_JOURNAL_SB;

	if (ntohl(jsb->s_maxlen) < journal->j_maxlen)
		journal->j_maxlen = ntohl(jsb->s_maxlen);
	else if (ntohl(jsb->s_maxlen) > journal->j_maxlen)
		return EXT2_ET_CORRUPT_JOURNAL_SB;

	journal->j_tail_sequence = ntohl(jsb->s_sequence);
	journal->j_transaction_sequence = journal->j_tail_sequence;
	journal->j_tail = ntohl(jsb->s_start);
	journal->j_first = ntohl(jsb->s_first);
	journal->j_last = ntohl(jsb->s_maxlen);

	return 0;
}

static void ext2fs_journal_release(ext2_filsys fs, journal_t *journal,
				   int reset, int drop)
{
	journal_superblock_t *jsb;

	if (drop)
		mark_buffer_clean(journal->j_sb_buffer);
	else if (fs->flags & EXT2_FLAG_RW) {
		jsb = journal->j_superblock;
		jsb->s_sequence = htonl(journal->j_tail_sequence);
		if (reset)
			jsb->s_start = 0; /* this marks the journal as empty */
		ext2fs_journal_sb_csum_set(journal, jsb);
		mark_buffer_dirty(journal->j_sb_buffer);
	}
	brelse(journal->j_sb_buffer);

	if (fs && fs->journal_io) {
		if (fs->io != fs->journal_io)
			io_channel_close(fs->journal_io);
		fs->journal_io = NULL;
		free(fs->journal_name);
		fs->journal_name = NULL;
	}

#ifndef USE_INODE_IO
	if (journal->j_inode)
		ext2fs_free_mem(&journal->j_inode);
#endif
	if (journal->j_fs_dev)
		ext2fs_free_mem(&journal->j_fs_dev);
	ext2fs_free_mem(&journal);
}

/*
 * This function makes sure that the superblock fields regarding the
 * journal are consistent.
 */
static errcode_t ext2fs_check_ext3_journal(ext2_filsys fs)
{
	struct ext2_super_block *sb = fs->super;
	journal_t *journal;
	int recover = ext2fs_has_feature_journal_needs_recovery(fs->super);
	errcode_t retval;

	/* If we don't have any journal features, don't do anything more */
	if (!ext2fs_has_feature_journal(sb) &&
	    !recover && sb->s_journal_inum == 0 && sb->s_journal_dev == 0 &&
	    uuid_is_null(sb->s_journal_uuid))
		return 0;

	retval = ext2fs_get_journal(fs, &journal);
	if (retval)
		return retval;

	retval = ext2fs_journal_load(journal);
	if (retval)
		goto err;

	/*
	 * We want to make the flags consistent here.  We will not leave with
	 * needs_recovery set but has_journal clear.  We can't get in a loop
	 * with -y, -n, or -p, only if a user isn't making up their mind.
	 */
	if (!ext2fs_has_feature_journal(sb)) {
		retval = EXT2_ET_JOURNAL_FLAGS_WRONG;
		goto err;
	}

	if (ext2fs_has_feature_journal(sb) &&
	    !ext2fs_has_feature_journal_needs_recovery(sb) &&
	    journal->j_superblock->s_start != 0) {
		retval = EXT2_ET_JOURNAL_FLAGS_WRONG;
		goto err;
	}

	/*
	 * If we don't need to do replay the journal, check to see if
	 * the journal's errno is set; if so, we need to mark the file
	 * system as being corrupt and clear the journal's s_errno.
	 */
	if (!ext2fs_has_feature_journal_needs_recovery(sb) &&
	    journal->j_superblock->s_errno) {
		fs->super->s_state |= EXT2_ERROR_FS;
		ext2fs_mark_super_dirty(fs);
		journal->j_superblock->s_errno = 0;
		ext2fs_journal_sb_csum_set(journal, journal->j_superblock);
		mark_buffer_dirty(journal->j_sb_buffer);
	}

err:
	ext2fs_journal_release(fs, journal, 0, retval ? 1 : 0);
	return retval;
}

static errcode_t recover_ext3_journal(ext2_filsys fs)
{
	journal_t *journal;
	errcode_t retval;

	journal_init_revoke_caches();
	retval = ext2fs_get_journal(fs, &journal);
	if (retval)
		return retval;

	retval = ext2fs_journal_load(journal);
	if (retval)
		goto errout;

	retval = journal_init_revoke(journal, 1024);
	if (retval)
		goto errout;

	retval = -journal_recover(journal);
	if (retval)
		goto errout;

	if (journal->j_failed_commit) {
		journal->j_superblock->s_errno = -EINVAL;
		mark_buffer_dirty(journal->j_sb_buffer);
	}

errout:
	journal_destroy_revoke(journal);
	journal_destroy_revoke_caches();
	ext2fs_journal_release(fs, journal, 1, 0);
	return retval;
}

errcode_t ext2fs_run_ext3_journal(ext2_filsys *fsp)
{
	ext2_filsys fs = *fsp;
	io_manager io_ptr = fs->io->manager;
	errcode_t	retval, recover_retval;
	io_stats	stats = 0;
	unsigned long long kbytes_written = 0;
	char *fsname;
	int fsflags;
	int fsblocksize;

	if (!(fs->flags & EXT2_FLAG_RW))
		return EXT2_ET_FILE_RO;

	if (fs->flags & EXT2_FLAG_DIRTY)
		ext2fs_flush(fs);	/* Force out any modifications */

	recover_retval = recover_ext3_journal(fs);

	/*
	 * Reload the filesystem context to get up-to-date data from disk
	 * because journal recovery will change the filesystem under us.
	 */
	if (fs->super->s_kbytes_written &&
	    fs->io->manager->get_stats)
		fs->io->manager->get_stats(fs->io, &stats);
	if (stats && stats->bytes_written)
		kbytes_written = stats->bytes_written >> 10;

	ext2fs_mmp_stop(fs);
	fsname = fs->device_name;
	fs->device_name = NULL;
	fsflags = fs->flags;
	fsblocksize = fs->blocksize;
	ext2fs_free(fs);
	*fsp = NULL;
	retval = ext2fs_open(fsname, fsflags, 0, fsblocksize, io_ptr, fsp);
	ext2fs_free_mem(&fsname);
	if (retval)
		return retval;

	fs = *fsp;
	fs->flags |= EXT2_FLAG_MASTER_SB_ONLY;
	fs->super->s_kbytes_written += kbytes_written;

	/* Set the superblock flags */
	ext2fs_clear_recover(fs, recover_retval != 0);

	/*
	 * Do one last sanity check, and propagate journal->s_errno to
	 * the EXT2_ERROR_FS flag in the fs superblock if needed.
	 */
	retval = ext2fs_check_ext3_journal(fs);
	return retval ? retval : recover_retval;
}

errcode_t ext2fs_open_journal(ext2_filsys fs, journal_t **j)
{
	journal_t *journal;
	errcode_t retval;

	journal_init_revoke_caches();
	retval = ext2fs_get_journal(fs, &journal);
	if (retval)
		return retval;

	retval = ext2fs_journal_load(journal);
	if (retval)
		goto errout;

	retval = journal_init_revoke(journal, 1024);
	if (retval)
		goto errout;

	if (journal->j_failed_commit) {
		journal->j_superblock->s_errno = -EINVAL;
		mark_buffer_dirty(journal->j_sb_buffer);
	}

	*j = journal;
	return 0;

errout:
	journal_destroy_revoke(journal);
	journal_destroy_revoke_caches();
	ext2fs_journal_release(fs, journal, 1, 0);
	return retval;
}

errcode_t ext2fs_close_journal(ext2_filsys fs, journal_t **j)
{
	journal_t *journal = *j;

	journal_destroy_revoke(journal);
	journal_destroy_revoke_caches();
	ext2fs_journal_release(fs, journal, 0, 0);
	*j = NULL;

	return 0;
}

void jbd2_commit_block_csum_set(journal_t *j, struct buffer_head *bh)
{
	struct commit_header *h;
	__u32 csum;

	if (!journal_has_csum_v2or3(j))
		return;

	h = (struct commit_header *)(bh->b_data);
	h->h_chksum_type = 0;
	h->h_chksum_size = 0;
	h->h_chksum[0] = 0;
	csum = jbd2_chksum(j, j->j_csum_seed, bh->b_data, j->j_blocksize);
	h->h_chksum[0] = ext2fs_cpu_to_be32(csum);
}

void jbd2_revoke_csum_set(journal_t *j, struct buffer_head *bh)
{
	struct journal_revoke_tail *tail;
	__u32 csum;

	if (!journal_has_csum_v2or3(j))
		return;

	tail = (struct journal_revoke_tail *)(bh->b_data + j->j_blocksize -
			sizeof(struct journal_revoke_tail));
	tail->r_checksum = 0;
	csum = jbd2_chksum(j, j->j_csum_seed, bh->b_data, j->j_blocksize);
	tail->r_checksum = ext2fs_cpu_to_be32(csum);
}

void jbd2_descr_block_csum_set(journal_t *j, struct buffer_head *bh)
{
	struct journal_block_tail *tail;
	__u32 csum;

	if (!journal_has_csum_v2or3(j))
		return;

	tail = (struct journal_block_tail *)(bh->b_data + j->j_blocksize -
			sizeof(struct journal_block_tail));
	tail->t_checksum = 0;
	csum = jbd2_chksum(j, j->j_csum_seed, bh->b_data, j->j_blocksize);
	tail->t_checksum = ext2fs_cpu_to_be32(csum);
}

void jbd2_block_tag_csum_set(journal_t *j, journal_block_tag_t *tag,
			     struct buffer_head *bh, __u32 sequence)
{
	journal_block_tag3_t *tag3 = (journal_block_tag3_t *)tag;
	__u32 csum32;
	__be32 seq;

	if (!journal_has_csum_v2or3(j))
		return;

	seq = ext2fs_cpu_to_be32(sequence);
	csum32 = jbd2_chksum(j, j->j_csum_seed, (__u8 *)&seq, sizeof(seq));
	csum32 = jbd2_chksum(j, csum32, bh->b_data, bh->b_size);

	if (jfs_has_feature_csum3(j))
		tag3->t_checksum = ext2fs_cpu_to_be32(csum32);
	else
		tag->t_checksum = ext2fs_cpu_to_be16(csum32);
}

