/*
 * e2fuzz.c -- Fuzz an ext4 image, for testing purposes.
 *
 * Copyright (C) 2014 Oracle.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */
#define _XOPEN_SOURCE		600
#define _FILE_OFFSET_BITS       64
#define _LARGEFILE64_SOURCE     1
#define _GNU_SOURCE		1

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"

static int dryrun = 0;
static int verbose = 0;
static int metadata_only = 1;
static unsigned long long user_corrupt_bytes = 0;
static double user_corrupt_pct = 0.0;

#if !defined HAVE_PWRITE64 && !defined HAVE_PWRITE
static ssize_t my_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	if (lseek(fd, offset, SEEK_SET) < 0)
		return 0;

	return write(fd, buf, count);
}
#endif /* !defined HAVE_PWRITE64 && !defined HAVE_PWRITE */

static int getseed(void)
{
	int r;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(0);
	}
	if (read(fd, &r, sizeof(r)) != sizeof(r))
		printf("Unable to read random seed!\n");
	close(fd);
	return r;
}

struct find_block {
	ext2_ino_t ino;
	ext2fs_block_bitmap bmap;
	struct ext2_inode *inode;
	blk64_t corrupt_blocks;
};

static int find_block_helper(ext2_filsys fs EXT2FS_ATTR((unused)),
			     blk64_t *blocknr, e2_blkcnt_t blockcnt,
			     blk64_t ref_blk EXT2FS_ATTR((unused)),
			     int ref_offset EXT2FS_ATTR((unused)),
			     void *priv_data)
{
	struct find_block *fb = (struct find_block *)priv_data;

	if (S_ISDIR(fb->inode->i_mode) || !metadata_only || blockcnt < 0) {
		ext2fs_mark_block_bitmap2(fb->bmap, *blocknr);
		fb->corrupt_blocks++;
	}

	return 0;
}

static errcode_t find_metadata_blocks(ext2_filsys fs, ext2fs_block_bitmap bmap,
				      off_t *corrupt_bytes)
{
	dgrp_t i;
	blk64_t b, c;
	ext2_inode_scan scan;
	ext2_ino_t ino;
	struct ext2_inode inode;
	struct find_block fb;
	errcode_t retval;

	*corrupt_bytes = 0;
	fb.corrupt_blocks = 0;

	/* Construct bitmaps of super/descriptor blocks */
	for (i = 0; i < fs->group_desc_count; i++) {
		ext2fs_reserve_super_and_bgd(fs, i, bmap);

		/* bitmaps and inode table */
		b = ext2fs_block_bitmap_loc(fs, i);
		ext2fs_mark_block_bitmap2(bmap, b);
		fb.corrupt_blocks++;

		b = ext2fs_inode_bitmap_loc(fs, i);
		ext2fs_mark_block_bitmap2(bmap, b);
		fb.corrupt_blocks++;

		c = ext2fs_inode_table_loc(fs, i);
		ext2fs_mark_block_bitmap_range2(bmap, c,
						fs->inode_blocks_per_group);
		fb.corrupt_blocks += fs->inode_blocks_per_group;
	}

	/* Scan inodes */
	fb.bmap = bmap;
	fb.inode = &inode;
	memset(&inode, 0, sizeof(inode));
	retval = ext2fs_open_inode_scan(fs, 0, &scan);
	if (retval)
		goto out;

	retval = ext2fs_get_next_inode_full(scan, &ino, &inode, sizeof(inode));
	if (retval)
		goto out2;
	while (ino) {
		if (inode.i_links_count == 0)
			goto next_loop;

		b = ext2fs_file_acl_block(fs, &inode);
		if (b) {
			ext2fs_mark_block_bitmap2(bmap, b);
			fb.corrupt_blocks++;
		}

		/*
		 * Inline data, sockets, devices, and symlinks have
		 * no blocks to iterate.
		 */
		if ((inode.i_flags & EXT4_INLINE_DATA_FL) ||
		    S_ISLNK(inode.i_mode) || S_ISFIFO(inode.i_mode) ||
		    S_ISCHR(inode.i_mode) || S_ISBLK(inode.i_mode) ||
		    S_ISSOCK(inode.i_mode))
			goto next_loop;
		fb.ino = ino;
		retval = ext2fs_block_iterate3(fs, ino, BLOCK_FLAG_READ_ONLY,
					       NULL, find_block_helper, &fb);
		if (retval)
			goto out2;
next_loop:
		retval = ext2fs_get_next_inode_full(scan, &ino, &inode,
						    sizeof(inode));
		if (retval)
			goto out2;
	}
out2:
	ext2fs_close_inode_scan(scan);
out:
	if (!retval)
		*corrupt_bytes = fb.corrupt_blocks * fs->blocksize;
	return retval;
}

static uint64_t rand_num(uint64_t min, uint64_t max)
{
	uint64_t x;
	unsigned int i;
	uint8_t *px = (uint8_t *)&x;

	for (i = 0; i < sizeof(x); i++)
		px[i] = random();

	return min + (uint64_t)((double)(max - min) * (x / (UINT64_MAX + 1.0)));
}

static int process_fs(const char *fsname)
{
	errcode_t ret;
	int flags, fd;
	ext2_filsys fs = NULL;
	ext2fs_block_bitmap corrupt_map;
	loff_t hsize, count, off, offset, corrupt_bytes;
	unsigned char c;
	loff_t i;

	/* If mounted rw, force dryrun mode */
	ret = ext2fs_check_if_mounted(fsname, &flags);
	if (ret) {
		fprintf(stderr, "%s: failed to determine filesystem mount "
			"state.\n", fsname);
		return 1;
	}

	if (!dryrun && (flags & EXT2_MF_MOUNTED) &&
	    !(flags & EXT2_MF_READONLY)) {
		fprintf(stderr, "%s: is mounted rw, performing dry run.\n",
			fsname);
		dryrun = 1;
	}

	/* Ensure the fs is clean and does not have errors */
	ret = ext2fs_open(fsname, EXT2_FLAG_64BITS, 0, 0, unix_io_manager,
			  &fs);
	if (ret) {
		fprintf(stderr, "%s: failed to open filesystem.\n",
			fsname);
		return 1;
	}

	if ((fs->super->s_state & EXT2_ERROR_FS)) {
		fprintf(stderr, "%s: errors detected, run fsck.\n",
			fsname);
		goto fail;
	}

	if (!dryrun && (fs->super->s_state & EXT2_VALID_FS) == 0) {
		fprintf(stderr, "%s: unclean shutdown, performing dry run.\n",
			fsname);
		dryrun = 1;
	}

	/* Construct a bitmap of whatever we're corrupting */
	if (!metadata_only) {
		/* Load block bitmap */
		ret = ext2fs_read_block_bitmap(fs);
		if (ret) {
			fprintf(stderr, "%s: error while reading block bitmap\n",
				fsname);
			goto fail;
		}
		corrupt_map = fs->block_map;
		corrupt_bytes = (ext2fs_blocks_count(fs->super) -
				 ext2fs_free_blocks_count(fs->super)) *
				fs->blocksize;
	} else {
		ret = ext2fs_allocate_block_bitmap(fs, "metadata block map",
						   &corrupt_map);
		if (ret) {
			fprintf(stderr, "%s: unable to create block bitmap\n",
				fsname);
			goto fail;
		}

		/* Iterate everything... */
		ret = find_metadata_blocks(fs, corrupt_map, &corrupt_bytes);
		if (ret) {
			fprintf(stderr, "%s: while finding metadata\n",
				fsname);
			goto fail;
		}
	}

	/* Run around corrupting things */
	fd = open(fsname, O_RDWR);
	if (fd < 0) {
		perror(fsname);
		goto fail;
	}
	srandom(getseed());
	hsize = fs->blocksize * ext2fs_blocks_count(fs->super);
	if (user_corrupt_bytes > 0)
		count = user_corrupt_bytes;
	else if (user_corrupt_pct > 0.0)
		count = user_corrupt_pct * corrupt_bytes / 100;
	else
		count = rand_num(0, corrupt_bytes / 100);
	offset = 4096; /* never corrupt superblock */
	for (i = 0; i < count; i++) {
		do
			off = rand_num(offset, hsize);
		while (!ext2fs_test_block_bitmap2(corrupt_map,
						    off / fs->blocksize));
		c = rand() % 256;
		if ((rand() % 2) && c < 128)
			c |= 0x80;
		if (verbose)
			printf("Corrupting byte %lld in block %lld to 0x%x\n",
			       off % fs->blocksize,
			       off / fs->blocksize, c);
		if (dryrun)
			continue;
#ifdef HAVE_PWRITE64
		if (pwrite64(fd, &c, sizeof(c), off) != sizeof(c)) {
			perror(fsname);
			goto fail3;
		}
#elif HAVE_PWRITE
		if (pwrite(fd, &c, sizeof(c), off) != sizeof(c)) {
			perror(fsname);
			goto fail3;
		}
#else
		if (my_pwrite(fd, &c, sizeof(c), off) != sizeof(c)) {
			perror(fsname);
			goto fail3;
		}
#endif
	}
	close(fd);

	/* Clean up */
	ret = ext2fs_close_free(&fs);
	if (ret) {
		fprintf(stderr, "%s: error while closing filesystem\n",
			fsname);
		return 1;
	}

	return 0;
fail3:
	close(fd);
	if (corrupt_map != fs->block_map)
		ext2fs_free_block_bitmap(corrupt_map);
fail:
	ext2fs_close_free(&fs);
	return 1;
}

static void print_help(const char *progname)
{
	printf("Usage: %s OPTIONS device\n", progname);
	printf("-b:	Corrupt this many bytes.\n");
	printf("-d:	Fuzz data blocks too.\n");
	printf("-n:	Dry run only.\n");
	printf("-v:	Verbose output.\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "b:dnv")) != -1) {
		switch (c) {
		case 'b':
			if (optarg[strlen(optarg) - 1] == '%') {
				user_corrupt_pct = strtod(optarg, NULL);
				if (user_corrupt_pct > 100 ||
				    user_corrupt_pct < 0) {
					fprintf(stderr, "%s: Invalid percentage.\n",
						optarg);
					return 1;
				}
			} else
				user_corrupt_bytes = strtoull(optarg, NULL, 0);
			if (errno) {
				perror(optarg);
				return 1;
			}
			break;
		case 'd':
			metadata_only = 0;
			break;
		case 'n':
			dryrun = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			print_help(argv[0]);
		}
	}

	for (c = optind; c < argc; c++)
		if (process_fs(argv[c]))
			return 1;
	return 0;
}
