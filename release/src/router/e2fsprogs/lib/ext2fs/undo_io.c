/*
 * undo_io.c --- This is the undo io manager that copies the old data that
 * copies the old data being overwritten into a tdb database
 *
 * Copyright IBM Corporation, 2007
 * Author Aneesh Kumar K.V <aneesh.kumar@linux.vnet.ibm.com>
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include "config.h"
#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#include <fcntl.h>
#include <time.h>
#ifdef __linux__
#include <sys/utsname.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <limits.h>

#include "ext2_fs.h"
#include "ext2fs.h"
#include "ext2fsP.h"

#ifdef __GNUC__
#define ATTR(x) __attribute__(x)
#else
#define ATTR(x)
#endif

#undef DEBUG

#ifdef DEBUG
# define dbg_printf(f, a...)  do {printf(f, ## a); fflush(stdout); } while (0)
#else
# define dbg_printf(f, a...)
#endif

/*
 * For checking structure magic numbers...
 */

#define EXT2_CHECK_MAGIC(struct, code) \
	  if ((struct)->magic != (code)) return (code)
/*
 * Undo file format: The file is cut up into undo_header.block_size blocks.
 * The first block contains the header.
 * The second block contains the superblock.
 * There is then a repeating series of blocks as follows:
 *   A key block, which contains undo_keys to map the following data blocks.
 *   Data blocks
 * (Note that there are pointers to the first key block and the sb, so this
 * order isn't strictly necessary.)
 */
#define E2UNDO_MAGIC "E2UNDO02"
#define KEYBLOCK_MAGIC 0xCADECADE

#define E2UNDO_STATE_FINISHED	0x1	/* undo file is complete */

#define E2UNDO_MIN_BLOCK_SIZE	1024	/* undo blocks are no less than 1KB */
#define E2UNDO_MAX_BLOCK_SIZE	1048576	/* undo blocks are no more than 1MB */

struct undo_header {
	char magic[8];		/* "E2UNDO02" */
	__le64 num_keys;	/* how many keys? */
	__le64 super_offset;	/* where in the file is the superblock copy? */
	__le64 key_offset;	/* where do the key/data block chunks start? */
	__le32 block_size;	/* block size of the undo file */
	__le32 fs_block_size;	/* block size of the target device */
	__le32 sb_crc;		/* crc32c of the superblock */
	__le32 state;		/* e2undo state flags */
	__le32 f_compat;	/* compatible features */
	__le32 f_incompat;	/* incompatible features (none so far) */
	__le32 f_rocompat;	/* ro compatible features (none so far) */
	__le32 pad32;		/* padding for fs_offset */
	__le64 fs_offset;	/* filesystem offset */
	__u8 padding[436];	/* padding */
	__le32 header_crc;	/* crc32c of this header (but not this field) */
};

#define E2UNDO_MAX_EXTENT_BLOCKS	512	/* max extent size, in blocks */

struct undo_key {
	__le64 fsblk;		/* where in the fs does the block go */
	__le32 blk_crc;		/* crc32c of the block */
	__le32 size;		/* how many bytes in this block? */
};

struct undo_key_block {
	__le32 magic;		/* KEYBLOCK_MAGIC number */
	__le32 crc;		/* block checksum */
	__le64 reserved;	/* zero */

#if __STDC_VERSION__ >= 199901L
	struct undo_key keys[];		/* keys, which come immediately after */
#else
	struct undo_key keys[0];	/* keys, which come immediately after */
#endif
};

struct undo_private_data {
	int	magic;

	/* the undo file io channel */
	io_channel undo_file;
	blk64_t undo_blk_num;			/* next free block */
	blk64_t key_blk_num;			/* current key block location */
	blk64_t super_blk_num;			/* superblock location */
	blk64_t first_key_blk;			/* first key block location */
	struct undo_key_block *keyb;
	size_t num_keys, keys_in_block;

	/* The backing io channel */
	io_channel real;

	unsigned long long tdb_data_size;
	int tdb_written;

	/* to support offset in unix I/O manager */
	ext2_loff_t offset;

	ext2fs_block_bitmap written_block_map;
	struct struct_ext2_filsys fake_fs;
	char *tdb_file;
	struct undo_header hdr;
};
#define KEYS_PER_BLOCK(d) (((d)->tdb_data_size / sizeof(struct undo_key)) - 1)

#define E2UNDO_FEATURE_COMPAT_FS_OFFSET 0x1	/* the filesystem offset */

static inline void e2undo_set_feature_fs_offset(struct undo_header *header) {
	header->f_compat |= ext2fs_le32_to_cpu(E2UNDO_FEATURE_COMPAT_FS_OFFSET);
}

static inline void e2undo_clear_feature_fs_offset(struct undo_header *header) {
	header->f_compat &= ~ext2fs_le32_to_cpu(E2UNDO_FEATURE_COMPAT_FS_OFFSET);
}

static io_manager undo_io_backing_manager;
static char *tdb_file;
static int actual_size;

errcode_t set_undo_io_backing_manager(io_manager manager)
{
	/*
	 * We may want to do some validation later
	 */
	undo_io_backing_manager = manager;
	return 0;
}

errcode_t set_undo_io_backup_file(char *file_name)
{
	tdb_file = strdup(file_name);

	if (tdb_file == NULL) {
		return EXT2_ET_NO_MEMORY;
	}

	return 0;
}

static errcode_t write_undo_indexes(struct undo_private_data *data, int flush)
{
	errcode_t retval;
	struct ext2_super_block super;
	io_channel channel;
	int block_size;
	__u32 sb_crc, hdr_crc;

	/* Spit out a key block, if there's any data */
	if (data->keys_in_block) {
		data->keyb->magic = ext2fs_cpu_to_le32(KEYBLOCK_MAGIC);
		data->keyb->crc = 0;
		data->keyb->crc = ext2fs_cpu_to_le32(
					 ext2fs_crc32c_le(~0,
					 (unsigned char *)data->keyb,
					 data->tdb_data_size));
		dbg_printf("Writing keyblock to blk %llu\n", data->key_blk_num);
		retval = io_channel_write_blk64(data->undo_file,
						data->key_blk_num,
						1, data->keyb);
		if (retval)
			return retval;
		/* Move on to the next key block if it's full. */
		if (data->keys_in_block == KEYS_PER_BLOCK(data)) {
			memset(data->keyb, 0, data->tdb_data_size);
			data->keys_in_block = 0;
			data->key_blk_num = data->undo_blk_num;
			data->undo_blk_num++;
		}
	}

	/* Prepare superblock for write */
	channel = data->real;
	block_size = channel->block_size;

	io_channel_set_blksize(channel, SUPERBLOCK_OFFSET);
	retval = io_channel_read_blk64(channel, 1, -SUPERBLOCK_SIZE, &super);
	if (retval)
		goto err_out;
	sb_crc = ext2fs_crc32c_le(~0, (unsigned char *)&super, SUPERBLOCK_SIZE);
	super.s_magic = ~super.s_magic;

	/* Write the undo header to disk. */
	memcpy(data->hdr.magic, E2UNDO_MAGIC, sizeof(data->hdr.magic));
	data->hdr.num_keys = ext2fs_cpu_to_le64(data->num_keys);
	data->hdr.super_offset = ext2fs_cpu_to_le64(data->super_blk_num);
	data->hdr.key_offset = ext2fs_cpu_to_le64(data->first_key_blk);
	data->hdr.fs_block_size = ext2fs_cpu_to_le32(block_size);
	data->hdr.sb_crc = ext2fs_cpu_to_le32(sb_crc);
	data->hdr.fs_offset = ext2fs_cpu_to_le64(data->offset);
	if (data->offset)
		e2undo_set_feature_fs_offset(&data->hdr);
	else
		e2undo_clear_feature_fs_offset(&data->hdr);
	hdr_crc = ext2fs_crc32c_le(~0, (unsigned char *)&data->hdr,
				   sizeof(data->hdr) -
				   sizeof(data->hdr.header_crc));
	data->hdr.header_crc = ext2fs_cpu_to_le32(hdr_crc);
	retval = io_channel_write_blk64(data->undo_file, 0,
					-(int)sizeof(data->hdr),
					&data->hdr);
	if (retval)
		goto err_out;

	/*
	 * Record the entire superblock (in FS byte order) so that we can't
	 * apply e2undo files to the wrong FS or out of order.
	 */
	dbg_printf("Writing superblock to block %llu\n", data->super_blk_num);
	retval = io_channel_write_blk64(data->undo_file, data->super_blk_num,
					-SUPERBLOCK_SIZE, &super);
	if (retval)
		goto err_out;

	if (flush)
		retval = io_channel_flush(data->undo_file);
err_out:
	io_channel_set_blksize(channel, block_size);
	return retval;
}

static errcode_t undo_setup_tdb(struct undo_private_data *data)
{
	int i;
	errcode_t retval;

	if (data->tdb_written == 1)
		return 0;

	data->tdb_written = 1;

	/* Make a bitmap to track what we've written */
	memset(&data->fake_fs, 0, sizeof(data->fake_fs));
	data->fake_fs.blocksize = data->tdb_data_size;
	retval = ext2fs_alloc_generic_bmap(&data->fake_fs,
				EXT2_ET_MAGIC_BLOCK_BITMAP64,
				EXT2FS_BMAP64_RBTREE,
				0, ~1ULL, ~1ULL,
				"undo block map", &data->written_block_map);
	if (retval)
		return retval;

	/* Allocate key block */
	retval = ext2fs_get_mem(data->tdb_data_size, &data->keyb);
	if (retval)
		return retval;
	data->key_blk_num = data->first_key_blk;

	/* Record block size */
	dbg_printf("Undo block size %llu\n", data->tdb_data_size);
	dbg_printf("Keys per block %llu\n", KEYS_PER_BLOCK(data));
	data->hdr.block_size = ext2fs_cpu_to_le32(data->tdb_data_size);
	io_channel_set_blksize(data->undo_file, data->tdb_data_size);

	/* Ensure that we have space for header blocks */
	for (i = 0; i <= 2; i++) {
		retval = io_channel_read_blk64(data->undo_file, i, 1,
					       data->keyb);
		if (retval)
			memset(data->keyb, 0, data->tdb_data_size);
		retval = io_channel_write_blk64(data->undo_file, i, 1,
						data->keyb);
		if (retval)
			return retval;
		retval = io_channel_flush(data->undo_file);
		if (retval)
			return retval;
	}
	memset(data->keyb, 0, data->tdb_data_size);
	return 0;
}

static errcode_t undo_write_tdb(io_channel channel,
				unsigned long long block, int count)

{
	int size, sz;
	unsigned long long block_num, backing_blk_num;
	errcode_t retval = 0;
	ext2_loff_t offset;
	struct undo_private_data *data;
	unsigned char *read_ptr;
	unsigned long long end_block;
	unsigned long long data_size;
	struct undo_key *key;
	__u32 blk_crc;

	data = (struct undo_private_data *) channel->private_data;

	if (data->undo_file == NULL) {
		/*
		 * Transaction database not initialized
		 */
		return 0;
	}

	if (count == 1)
		size = channel->block_size;
	else {
		if (count < 0)
			size = -count;
		else
			size = count * channel->block_size;
	}

	retval = undo_setup_tdb(data);
	if (retval)
		return retval;
	/*
	 * Data is stored in tdb database as blocks of tdb_data_size size
	 * This helps in efficient lookup further.
	 *
	 * We divide the disk to blocks of tdb_data_size.
	 */
	offset = (block * channel->block_size) + data->offset ;
	block_num = offset / data->tdb_data_size;
	end_block = (offset + size - 1) / data->tdb_data_size;

	while (block_num <= end_block) {
		__u32 keysz;

		/*
		 * Check if we have the record already
		 */
		if (ext2fs_test_block_bitmap2(data->written_block_map,
						   block_num)) {
			/* Try the next block */
			block_num++;
			continue;
		}
		ext2fs_mark_block_bitmap2(data->written_block_map, block_num);

		/*
		 * Read one block using the backing I/O manager
		 * The backing I/O manager block size may be
		 * different from the tdb_data_size.
		 * Also we need to recalculate the block number with respect
		 * to the backing I/O manager.
		 */
		offset = block_num * data->tdb_data_size +
				(data->offset % data->tdb_data_size);
		backing_blk_num = (offset - data->offset) / channel->block_size;

		retval = ext2fs_get_mem(data->tdb_data_size, &read_ptr);
		if (retval) {
			return retval;
		}

		memset(read_ptr, 0, data->tdb_data_size);
		actual_size = 0;
		if ((data->tdb_data_size % channel->block_size) == 0)
			sz = data->tdb_data_size / channel->block_size;
		else
			sz = -data->tdb_data_size;
		retval = io_channel_read_blk64(data->real, backing_blk_num,
					     sz, read_ptr);
		if (retval) {
			if (retval != EXT2_ET_SHORT_READ) {
				free(read_ptr);
				return retval;
			}
			/*
			 * short read so update the record size
			 * accordingly
			 */
			data_size = actual_size;
		} else {
			data_size = data->tdb_data_size;
		}
		if (data_size == 0) {
			free(read_ptr);
			block_num++;
			continue;
		}
		dbg_printf("Read %llu bytes from FS block %llu (blk=%llu cnt=%llu)\n",
		       data_size, backing_blk_num, block, data->tdb_data_size);
		if ((data_size % data->undo_file->block_size) == 0)
			sz = data_size / data->undo_file->block_size;
		else
			sz = -data_size;;
		/* extend this key? */
		if (data->keys_in_block) {
			key = data->keyb->keys + data->keys_in_block - 1;
			keysz = ext2fs_le32_to_cpu(key->size);
		} else {
			key = NULL;
			keysz = 0;
		}
		if (key != NULL &&
		    (ext2fs_le64_to_cpu(key->fsblk) * channel->block_size +
		     channel->block_size - 1 +
		     keysz) / channel->block_size == backing_blk_num &&
		    E2UNDO_MAX_EXTENT_BLOCKS * data->tdb_data_size >
		    keysz + data_size) {
			blk_crc = ext2fs_le32_to_cpu(key->blk_crc);
			blk_crc = ext2fs_crc32c_le(blk_crc, read_ptr, data_size);
			key->blk_crc = ext2fs_cpu_to_le32(blk_crc);
			key->size = ext2fs_cpu_to_le32(keysz + data_size);
		} else {
			data->num_keys++;
			key = data->keyb->keys + data->keys_in_block;
			data->keys_in_block++;
			key->fsblk = ext2fs_cpu_to_le64(backing_blk_num);
			blk_crc = ext2fs_crc32c_le(~0, read_ptr, data_size);
			key->blk_crc = ext2fs_cpu_to_le32(blk_crc);
			key->size = ext2fs_cpu_to_le32(data_size);
		}
		dbg_printf("Writing block %llu to offset %llu size %d key %zu\n",
		       block_num,
		       data->undo_blk_num,
		       sz, data->num_keys - 1);
		retval = io_channel_write_blk64(data->undo_file,
					data->undo_blk_num, sz, read_ptr);
		if (retval) {
			free(read_ptr);
			return retval;
		}
		data->undo_blk_num++;
		free(read_ptr);

		/* Write out the key block */
		retval = write_undo_indexes(data, 0);
		if (retval)
			return retval;

		/* Next block */
		block_num++;
	}

	return retval;
}

static errcode_t undo_io_read_error(io_channel channel ATTR((unused)),
				    unsigned long block ATTR((unused)),
				    int count ATTR((unused)),
				    void *data ATTR((unused)),
				    size_t size ATTR((unused)),
				    int actual,
				    errcode_t error ATTR((unused)))
{
	actual_size = actual;
	return error;
}

static void undo_err_handler_init(io_channel channel)
{
	channel->read_error = undo_io_read_error;
}

static int check_filesystem(struct undo_header *hdr, io_channel undo_file,
			    unsigned int blocksize, blk64_t super_block,
			    io_channel channel)
{
	struct ext2_super_block super, *sb;
	char *buf;
	__u32 sb_crc;
	errcode_t retval;

	io_channel_set_blksize(channel, SUPERBLOCK_OFFSET);
	retval = io_channel_read_blk64(channel, 1, -SUPERBLOCK_SIZE, &super);
	if (retval)
		return retval;

	/*
	 * Compare the FS and the undo file superblock so that we don't
	 * append to something that doesn't match this FS.
	 */
	retval = ext2fs_get_mem(blocksize, &buf);
	if (retval)
		return retval;
	retval = io_channel_read_blk64(undo_file, super_block,
				       -SUPERBLOCK_SIZE, buf);
	if (retval)
		goto out;
	sb = (struct ext2_super_block *)buf;
	sb->s_magic = ~sb->s_magic;
	if (memcmp(&super, buf, sizeof(super))) {
		retval = -1;
		goto out;
	}
	sb_crc = ext2fs_crc32c_le(~0, (unsigned char *)buf, SUPERBLOCK_SIZE);
	if (ext2fs_le32_to_cpu(hdr->sb_crc) != sb_crc) {
		retval = -1;
		goto out;
	}

out:
	ext2fs_free_mem(&buf);
	return retval;
}

/*
 * Try to re-open the undo file, so that we can resume where we left off.
 * That way, the user can pass the same undo file to various programs as
 * part of an FS upgrade instead of having to create multiple files and
 * then apply them in correct order.
 */
static errcode_t try_reopen_undo_file(int undo_fd,
				      struct undo_private_data *data)
{
	struct undo_header hdr;
	struct undo_key *dkey;
	ext2fs_struct_stat statbuf;
	unsigned int blocksize, fs_blocksize;
	blk64_t super_block, lblk;
	size_t num_keys, keys_per_block, i;
	__u32 hdr_crc, key_crc;
	errcode_t retval;

	/* Zero size already? */
	retval = ext2fs_fstat(undo_fd, &statbuf);
	if (retval)
		goto bad_file;
	if (statbuf.st_size == 0)
		goto out;

	/* check the file header */
	retval = io_channel_read_blk64(data->undo_file, 0, -(int)sizeof(hdr),
				       &hdr);
	if (retval)
		goto bad_file;

	if (memcmp(hdr.magic, E2UNDO_MAGIC,
		    sizeof(hdr.magic)))
		goto bad_file;
	hdr_crc = ext2fs_crc32c_le(~0, (unsigned char *)&hdr,
				   sizeof(struct undo_header) -
				   sizeof(__u32));
	if (ext2fs_le32_to_cpu(hdr.header_crc) != hdr_crc)
		goto bad_file;
	blocksize = ext2fs_le32_to_cpu(hdr.block_size);
	fs_blocksize = ext2fs_le32_to_cpu(hdr.fs_block_size);
	if (blocksize > E2UNDO_MAX_BLOCK_SIZE ||
	    blocksize < E2UNDO_MIN_BLOCK_SIZE ||
	    !blocksize || !fs_blocksize)
		goto bad_file;
	super_block = ext2fs_le64_to_cpu(hdr.super_offset);
	num_keys = ext2fs_le64_to_cpu(hdr.num_keys);
	io_channel_set_blksize(data->undo_file, blocksize);
	/*
	 * Do not compare hdr.f_compat with the available compatible
	 * features set, because a "missing" compatible feature should
	 * not cause any problems.
	 */
	if (hdr.f_incompat || hdr.f_rocompat)
		goto bad_file;

	/* Superblock matches this FS? */
	if (check_filesystem(&hdr, data->undo_file, blocksize, super_block,
			     data->real) != 0) {
		retval = EXT2_ET_UNDO_FILE_WRONG;
		goto out;
	}

	/* Try to set ourselves up */
	data->tdb_data_size = blocksize;
	retval = undo_setup_tdb(data);
	if (retval)
		goto bad_file;
	data->num_keys = num_keys;
	data->super_blk_num = super_block;
	data->first_key_blk = ext2fs_le64_to_cpu(hdr.key_offset);

	/* load the written block map */
	keys_per_block = KEYS_PER_BLOCK(data);
	lblk = data->first_key_blk;
	dbg_printf("nr_keys=%lu, kpb=%zu, blksz=%u\n",
		   num_keys, keys_per_block, blocksize);
	for (i = 0; i < num_keys; i += keys_per_block) {
		size_t j, max_j;
		__le32 crc;

		data->key_blk_num = lblk;
		retval = io_channel_read_blk64(data->undo_file,
					       lblk, 1, data->keyb);
		if (retval)
			goto bad_key_replay;

		/* check keys */
		if (ext2fs_le32_to_cpu(data->keyb->magic) != KEYBLOCK_MAGIC) {
			retval = EXT2_ET_UNDO_FILE_CORRUPT;
			goto bad_key_replay;
		}
		crc = data->keyb->crc;
		data->keyb->crc = 0;
		key_crc = ext2fs_crc32c_le(~0, (unsigned char *)data->keyb,
					   blocksize);
		if (ext2fs_le32_to_cpu(crc) != key_crc) {
			retval = EXT2_ET_UNDO_FILE_CORRUPT;
			goto bad_key_replay;
		}

		/* load keys from key block */
		lblk++;
		max_j = data->num_keys - i;
		if (max_j > keys_per_block)
			max_j = keys_per_block;
		for (j = 0, dkey = data->keyb->keys;
		     j < max_j;
		     j++, dkey++) {
			blk64_t fsblk = ext2fs_le64_to_cpu(dkey->fsblk);
			blk64_t undo_blk = fsblk * fs_blocksize / blocksize;
			size_t size = ext2fs_le32_to_cpu(dkey->size);

			ext2fs_mark_block_bitmap_range2(data->written_block_map,
					 undo_blk,
					(size + blocksize - 1) / blocksize);
			lblk += (size + blocksize - 1) / blocksize;
			data->undo_blk_num = lblk;
			data->keys_in_block = j + 1;
		}
	}
	dbg_printf("Reopen undo, keyblk=%llu undoblk=%llu nrkeys=%zu kib=%zu\n",
		   data->key_blk_num, data->undo_blk_num, data->num_keys,
		   data->keys_in_block);

	data->hdr.state = hdr.state & ~E2UNDO_STATE_FINISHED;
	data->hdr.f_compat = hdr.f_compat;
	data->hdr.f_incompat = hdr.f_incompat;
	data->hdr.f_rocompat = hdr.f_rocompat;
	return retval;

bad_key_replay:
	data->key_blk_num = data->undo_blk_num = 0;
	data->keys_in_block = 0;
	ext2fs_free_mem(&data->keyb);
	ext2fs_free_generic_bitmap(data->written_block_map);
	data->tdb_written = 0;
	goto out;
bad_file:
	retval = EXT2_ET_UNDO_FILE_CORRUPT;
out:
	return retval;
}

static void undo_atexit(void *p)
{
	struct undo_private_data *data = p;
	errcode_t err;

	err = write_undo_indexes(data, 1);
	io_channel_close(data->undo_file);

	com_err(data->tdb_file, err, "while force-closing undo file");
}

static errcode_t undo_open(const char *name, int flags, io_channel *channel)
{
	io_channel	io = NULL;
	struct undo_private_data *data = NULL;
	int		undo_fd = -1;
	errcode_t	retval;

	if (name == 0)
		return EXT2_ET_BAD_DEVICE_NAME;
	retval = ext2fs_get_mem(sizeof(struct struct_io_channel), &io);
	if (retval)
		goto cleanup;
	memset(io, 0, sizeof(struct struct_io_channel));
	io->magic = EXT2_ET_MAGIC_IO_CHANNEL;
	retval = ext2fs_get_mem(sizeof(struct undo_private_data), &data);
	if (retval)
		goto cleanup;

	io->manager = undo_io_manager;
	retval = ext2fs_get_mem(strlen(name)+1, &io->name);
	if (retval)
		goto cleanup;

	strcpy(io->name, name);
	io->private_data = data;
	io->block_size = 1024;
	io->read_error = 0;
	io->write_error = 0;
	io->refcount = 1;

	memset(data, 0, sizeof(struct undo_private_data));
	data->magic = EXT2_ET_MAGIC_UNIX_IO_CHANNEL;
	data->super_blk_num = 1;
	data->first_key_blk = 2;
	data->undo_blk_num = 3;

	if (undo_io_backing_manager) {
		retval = undo_io_backing_manager->open(name, flags,
						       &data->real);
		if (retval)
			goto cleanup;

		data->tdb_file = strdup(tdb_file);
		if (data->tdb_file == NULL)
			goto cleanup;
		undo_fd = ext2fs_open_file(data->tdb_file, O_RDWR | O_CREAT,
					   0600);
		if (undo_fd < 0)
			goto cleanup;

		retval = undo_io_backing_manager->open(data->tdb_file,
						       IO_FLAG_RW,
						       &data->undo_file);
		if (retval)
			goto cleanup;
	} else {
		data->real = NULL;
		data->undo_file = NULL;
	}

	if (data->real)
		io->flags = (io->flags & ~CHANNEL_FLAGS_DISCARD_ZEROES) |
			    (data->real->flags & CHANNEL_FLAGS_DISCARD_ZEROES);

	/*
	 * setup err handler for read so that we know
	 * when the backing manager fails do short read
	 */
	if (data->real)
		undo_err_handler_init(data->real);

	if (data->undo_file) {
		retval = try_reopen_undo_file(undo_fd, data);
		if (retval)
			goto cleanup;
	}
	retval = ext2fs_add_exit_fn(undo_atexit, data);
	if (retval)
		goto cleanup;

	*channel = io;
	if (undo_fd >= 0)
		close(undo_fd);
	return retval;

cleanup:
	ext2fs_remove_exit_fn(undo_atexit, data);
	if (undo_fd >= 0)
		close(undo_fd);
	if (data && data->undo_file)
		io_channel_close(data->undo_file);
	if (data && data->tdb_file)
		free(data->tdb_file);
	if (data && data->real)
		io_channel_close(data->real);
	if (data)
		ext2fs_free_mem(&data);
	if (io)
		ext2fs_free_mem(&io);
	return retval;
}

static errcode_t undo_close(io_channel channel)
{
	struct undo_private_data *data;
	errcode_t	err, retval = 0;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (--channel->refcount > 0)
		return 0;
	/* Before closing write the file system identity */
	if (!getenv("UNDO_IO_SIMULATE_UNFINISHED"))
		data->hdr.state = ext2fs_cpu_to_le32(E2UNDO_STATE_FINISHED);
	err = write_undo_indexes(data, 1);
	ext2fs_remove_exit_fn(undo_atexit, data);
	if (data->real)
		retval = io_channel_close(data->real);
	if (data->tdb_file)
		free(data->tdb_file);
	if (data->undo_file)
		io_channel_close(data->undo_file);
	ext2fs_free_mem(&data->keyb);
	if (data->written_block_map)
		ext2fs_free_generic_bitmap(data->written_block_map);
	ext2fs_free_mem(&channel->private_data);
	if (channel->name)
		ext2fs_free_mem(&channel->name);
	ext2fs_free_mem(&channel);

	if (err)
		return err;
	return retval;
}

static errcode_t undo_set_blksize(io_channel channel, int blksize)
{
	struct undo_private_data *data;
	errcode_t		retval = 0;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (blksize > E2UNDO_MAX_BLOCK_SIZE || blksize < E2UNDO_MIN_BLOCK_SIZE)
		return EXT2_ET_INVALID_ARGUMENT;

	if (data->real)
		retval = io_channel_set_blksize(data->real, blksize);
	/*
	 * Set the block size used for tdb
	 */
	if (!data->tdb_data_size || !data->tdb_written)
		data->tdb_data_size = blksize;
	channel->block_size = blksize;
	return retval;
}

static errcode_t undo_read_blk64(io_channel channel, unsigned long long block,
			       int count, void *buf)
{
	errcode_t	retval = 0;
	struct undo_private_data *data;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (data->real)
		retval = io_channel_read_blk64(data->real, block, count, buf);

	return retval;
}

static errcode_t undo_read_blk(io_channel channel, unsigned long block,
			       int count, void *buf)
{
	return undo_read_blk64(channel, block, count, buf);
}

static errcode_t undo_write_blk64(io_channel channel, unsigned long long block,
				int count, const void *buf)
{
	struct undo_private_data *data;
	errcode_t	retval = 0;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);
	/*
	 * First write the existing content into database
	 */
	retval = undo_write_tdb(channel, block, count);
	if (retval)
		 return retval;
	if (data->real)
		retval = io_channel_write_blk64(data->real, block, count, buf);

	return retval;
}

static errcode_t undo_write_blk(io_channel channel, unsigned long block,
				int count, const void *buf)
{
	return undo_write_blk64(channel, block, count, buf);
}

static errcode_t undo_write_byte(io_channel channel, unsigned long offset,
				 int size, const void *buf)
{
	struct undo_private_data *data;
	errcode_t	retval = 0;
	ext2_loff_t	location;
	unsigned long blk_num, count;;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	location = offset + data->offset;
	blk_num = location/channel->block_size;
	/*
	 * the size specified may spread across multiple blocks
	 * also make sure we account for the fact that block start
	 * offset for tdb is different from the backing I/O manager
	 * due to possible different block size
	 */
	count = (size + (location % channel->block_size) +
			channel->block_size  -1)/channel->block_size;
	retval = undo_write_tdb(channel, blk_num, count);
	if (retval)
		return retval;
	if (data->real && data->real->manager->write_byte)
		retval = io_channel_write_byte(data->real, offset, size, buf);

	return retval;
}

static errcode_t undo_discard(io_channel channel, unsigned long long block,
			      unsigned long long count)
{
	struct undo_private_data *data;
	errcode_t	retval = 0;
	int icount;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (count > INT_MAX)
		return EXT2_ET_UNIMPLEMENTED;
	icount = count;

	/*
	 * First write the existing content into database
	 */
	retval = undo_write_tdb(channel, block, icount);
	if (retval)
		return retval;
	if (data->real)
		retval = io_channel_discard(data->real, block, count);

	return retval;
}

static errcode_t undo_zeroout(io_channel channel, unsigned long long block,
			      unsigned long long count)
{
	struct undo_private_data *data;
	errcode_t	retval = 0;
	int icount;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (count > INT_MAX)
		return EXT2_ET_UNIMPLEMENTED;
	icount = count;

	/*
	 * First write the existing content into database
	 */
	retval = undo_write_tdb(channel, block, icount);
	if (retval)
		return retval;
	if (data->real)
		retval = io_channel_zeroout(data->real, block, count);

	return retval;
}

static errcode_t undo_cache_readahead(io_channel channel,
				      unsigned long long block,
				      unsigned long long count)
{
	struct undo_private_data *data;
	errcode_t	retval = 0;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (data->real)
		retval = io_channel_cache_readahead(data->real, block, count);

	return retval;
}

/*
 * Flush data buffers to disk.
 */
static errcode_t undo_flush(io_channel channel)
{
	errcode_t	retval = 0;
	struct undo_private_data *data;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (data->real)
		retval = io_channel_flush(data->real);

	return retval;
}

static errcode_t undo_set_option(io_channel channel, const char *option,
				 const char *arg)
{
	errcode_t	retval = 0;
	struct undo_private_data *data;
	unsigned long tmp;
	char *end;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (!strcmp(option, "tdb_data_size")) {
		if (!arg)
			return EXT2_ET_INVALID_ARGUMENT;

		tmp = strtoul(arg, &end, 0);
		if (*end)
			return EXT2_ET_INVALID_ARGUMENT;
		if (tmp > E2UNDO_MAX_BLOCK_SIZE || tmp < E2UNDO_MIN_BLOCK_SIZE)
			return EXT2_ET_INVALID_ARGUMENT;
		if (!data->tdb_data_size || !data->tdb_written) {
			data->tdb_written = -1;
			data->tdb_data_size = tmp;
		}
		return 0;
	}
	/*
	 * Need to support offset option to work with
	 * Unix I/O manager
	 */
	if (data->real && data->real->manager->set_option) {
		retval = data->real->manager->set_option(data->real,
							option, arg);
	}
	if (!retval && !strcmp(option, "offset")) {
		if (!arg)
			return EXT2_ET_INVALID_ARGUMENT;

		tmp = strtoul(arg, &end, 0);
		if (*end)
			return EXT2_ET_INVALID_ARGUMENT;
		data->offset = tmp;
	}
	return retval;
}

static errcode_t undo_get_stats(io_channel channel, io_stats *stats)
{
	errcode_t	retval = 0;
	struct undo_private_data *data;

	EXT2_CHECK_MAGIC(channel, EXT2_ET_MAGIC_IO_CHANNEL);
	data = (struct undo_private_data *) channel->private_data;
	EXT2_CHECK_MAGIC(data, EXT2_ET_MAGIC_UNIX_IO_CHANNEL);

	if (data->real)
		retval = (data->real->manager->get_stats)(data->real, stats);

	return retval;
}

static struct struct_io_manager struct_undo_manager = {
	.magic		= EXT2_ET_MAGIC_IO_MANAGER,
	.name		= "Undo I/O Manager",
	.open		= undo_open,
	.close		= undo_close,
	.set_blksize	= undo_set_blksize,
	.read_blk	= undo_read_blk,
	.write_blk	= undo_write_blk,
	.flush		= undo_flush,
	.write_byte	= undo_write_byte,
	.set_option	= undo_set_option,
	.get_stats	= undo_get_stats,
	.read_blk64	= undo_read_blk64,
	.write_blk64	= undo_write_blk64,
	.discard	= undo_discard,
	.zeroout	= undo_zeroout,
	.cache_readahead	= undo_cache_readahead,
};

io_manager undo_io_manager = &struct_undo_manager;
