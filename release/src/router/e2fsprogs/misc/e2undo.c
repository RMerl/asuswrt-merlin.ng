/*
 * e2undo.c - Replay an undo log onto an ext2/3/4 filesystem
 *
 * Copyright IBM Corporation, 2007
 * Author Aneesh Kumar K.V <aneesh.kumar@linux.vnet.ibm.com>
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <fcntl.h>
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#include <unistd.h>
#include <libgen.h>
#include "ext2fs/ext2fs.h"
#include "support/nls-enable.h"

#undef DEBUG

#ifdef DEBUG
# define dbg_printf(f, a...)  do {printf(f, ## a); fflush(stdout); } while (0)
#else
# define dbg_printf(f, a...)
#endif

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
	__le32 f_compat;	/* compatible features (none so far) */
	__le32 f_incompat;	/* incompatible features (none so far) */
	__le32 f_rocompat;	/* ro compatible features (none so far) */
	__le32 pad32;		/* padding for fs_offset */
	__le64 fs_offset;	/* filesystem offset */
	__u8 padding[436];	/* padding */
	__le32 header_crc;	/* crc32c of the header (but not this field) */
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
#if __GNUC_PREREQ (4, 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
	struct undo_key keys[0];	/* keys, which come immediately after */
#if __GNUC_PREREQ (4, 8)
#pragma GCC diagnostic pop
#endif
};

struct undo_key_info {
	blk64_t fsblk;
	blk64_t fileblk;
	__u32 blk_crc;
	unsigned int size;
};

struct undo_context {
	struct undo_header hdr;
	io_channel undo_file;
	unsigned int blocksize, fs_blocksize;
	blk64_t super_block;
	size_t num_keys;
	struct undo_key_info *keys;
};
#define KEYS_PER_BLOCK(d) (((d)->blocksize / sizeof(struct undo_key)) - 1)

#define E2UNDO_FEATURE_COMPAT_FS_OFFSET 0x1	/* the filesystem offset */

static inline int e2undo_has_feature_fs_offset(struct undo_header *header) {
	return ext2fs_le32_to_cpu(header->f_compat) &
		E2UNDO_FEATURE_COMPAT_FS_OFFSET;
}

static char *prg_name;
static char *undo_file;

static void usage(void)
{
	fprintf(stderr,
		_("Usage: %s [-f] [-h] [-n] [-o offset] [-v] [-z undo_file] <transaction file> <filesystem>\n"), prg_name);
	exit(1);
}

static void dump_header(struct undo_header *hdr)
{
	printf("nr keys:\t%llu\n", ext2fs_le64_to_cpu(hdr->num_keys));
	printf("super block:\t%llu\n", ext2fs_le64_to_cpu(hdr->super_offset));
	printf("key block:\t%llu\n", ext2fs_le64_to_cpu(hdr->key_offset));
	printf("block size:\t%u\n", ext2fs_le32_to_cpu(hdr->block_size));
	printf("fs block size:\t%u\n", ext2fs_le32_to_cpu(hdr->fs_block_size));
	printf("super crc:\t0x%x\n", ext2fs_le32_to_cpu(hdr->sb_crc));
	printf("state:\t\t0x%x\n", ext2fs_le32_to_cpu(hdr->state));
	printf("compat:\t\t0x%x\n", ext2fs_le32_to_cpu(hdr->f_compat));
	printf("incompat:\t0x%x\n", ext2fs_le32_to_cpu(hdr->f_incompat));
	printf("rocompat:\t0x%x\n", ext2fs_le32_to_cpu(hdr->f_rocompat));
	if (e2undo_has_feature_fs_offset(hdr))
		printf("fs offset:\t%llu\n", ext2fs_le64_to_cpu(hdr->fs_offset));
	printf("header crc:\t0x%x\n", ext2fs_le32_to_cpu(hdr->header_crc));
}

static void print_undo_mismatch(struct ext2_super_block *fs_super,
				struct ext2_super_block *undo_super)
{
	printf("%s",
	       _("The file system superblock doesn't match the undo file.\n"));
	if (memcmp(fs_super->s_uuid, undo_super->s_uuid,
		   sizeof(fs_super->s_uuid)))
		printf("%s", _("UUID does not match.\n"));
	if (fs_super->s_mtime != undo_super->s_mtime)
		printf("%s", _("Last mount time does not match.\n"));
	if (fs_super->s_wtime != undo_super->s_wtime)
		printf("%s", _("Last write time does not match.\n"));
	if (fs_super->s_kbytes_written != undo_super->s_kbytes_written)
		printf("%s", _("Lifetime write counter does not match.\n"));
}

static int check_filesystem(struct undo_context *ctx, io_channel channel)
{
	struct ext2_super_block super, *sb;
	char *buf;
	__u32 sb_crc;
	errcode_t retval;

	io_channel_set_blksize(channel, SUPERBLOCK_OFFSET);
	retval = io_channel_read_blk64(channel, 1, -SUPERBLOCK_SIZE, &super);
	if (retval) {
		com_err(prg_name, retval,
			"%s", _("while reading filesystem superblock."));
		return retval;
	}

	/*
	 * Compare the FS and the undo file superblock so that we can't apply
	 * e2undo "patches" out of order.
	 */
	retval = ext2fs_get_mem(ctx->blocksize, &buf);
	if (retval) {
		com_err(prg_name, retval, "%s", _("while allocating memory"));
		return retval;
	}
	retval = io_channel_read_blk64(ctx->undo_file, ctx->super_block,
				       -SUPERBLOCK_SIZE, buf);
	if (retval) {
		com_err(prg_name, retval, "%s", _("while fetching superblock"));
		goto out;
	}
	sb = (struct ext2_super_block *)buf;
	sb->s_magic = ~sb->s_magic;
	if (memcmp(&super, buf, sizeof(super))) {
		print_undo_mismatch(&super, (struct ext2_super_block *)buf);
		retval = -1;
		goto out;
	}
	sb_crc = ext2fs_crc32c_le(~0, (unsigned char *)buf, SUPERBLOCK_SIZE);
	if (ext2fs_le32_to_cpu(ctx->hdr.sb_crc) != sb_crc) {
		fprintf(stderr,
			_("Undo file superblock checksum doesn't match.\n"));
		retval = -1;
		goto out;
	}

out:
	ext2fs_free_mem(&buf);
	return retval;
}

static int key_compare(const void *a, const void *b)
{
	const struct undo_key_info *ka, *kb;

	ka = a;
	kb = b;
	return ka->fsblk - kb->fsblk;
}

static int e2undo_setup_tdb(const char *name, io_manager *io_ptr)
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
	tdb_file = malloc(strlen(tdb_dir) + 8 + strlen(dev_name) + 7 + 1);
	if (!tdb_file) {
		free(tmp_name);
		goto errout;
	}
	sprintf(tdb_file, "%s/e2undo-%s.e2undo", tdb_dir, dev_name);
	free(tmp_name);

	if ((unlink(tdb_file) < 0) && (errno != ENOENT)) {
		retval = errno;
		com_err(prg_name, retval,
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
	com_err(prg_name, retval, "while trying to setup undo file\n");
	return retval;
}

int main(int argc, char *argv[])
{
	int c, force = 0, dry_run = 0, verbose = 0, dump = 0;
	io_channel channel;
	errcode_t retval;
	int mount_flags, csum_error = 0, io_error = 0;
	size_t i, keys_per_block;
	char *device_name, *tdb_file;
	io_manager manager = unix_io_manager;
	struct undo_context undo_ctx;
	char *buf;
	struct undo_key_block *keyb;
	struct undo_key *dkey;
	struct undo_key_info *ikey;
	__u32 key_crc, blk_crc, hdr_crc;
	blk64_t lblk;
	ext2_filsys fs;
	__u64 offset = 0;
	char opt_offset_string[40] = { 0 };

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
	set_com_err_gettext(gettext);
#endif
	add_error_table(&et_ext2_error_table);

	prg_name = argv[0];
	while ((c = getopt(argc, argv, "fhno:vz:")) != EOF) {
		switch (c) {
		case 'f':
			force = 1;
			break;
		case 'h':
			dump = 1;
			break;
		case 'n':
			dry_run = 1;
			break;
		case 'o':
			offset = strtoull(optarg, &buf, 0);
			if (*buf) {
				com_err(prg_name, 0,
						_("illegal offset - %s"), optarg);
				exit(1);
			}
			/* used to indicate that an offset was specified */
			opt_offset_string[0] = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'z':
			undo_file = optarg;
			break;
		default:
			usage();
		}
	}

	if (argc != optind + 2)
		usage();

	tdb_file = argv[optind];
	device_name = argv[optind+1];

	if (undo_file && strcmp(tdb_file, undo_file) == 0) {
		printf(_("Will not write to an undo file while replaying it.\n"));
		exit(1);
	}

	/* Interpret the undo file */
	retval = manager->open(tdb_file, IO_FLAG_EXCLUSIVE,
			       &undo_ctx.undo_file);
	if (retval) {
		com_err(prg_name, errno,
				_("while opening undo file `%s'\n"), tdb_file);
		exit(1);
	}
	retval = io_channel_read_blk64(undo_ctx.undo_file, 0,
				       -(int)sizeof(undo_ctx.hdr),
				       &undo_ctx.hdr);
	if (retval) {
		com_err(prg_name, retval, _("while reading undo file"));
		exit(1);
	}
	if (memcmp(undo_ctx.hdr.magic, E2UNDO_MAGIC,
		    sizeof(undo_ctx.hdr.magic))) {
		fprintf(stderr, _("%s: Not an undo file.\n"), tdb_file);
		exit(1);
	}
	if (dump) {
		dump_header(&undo_ctx.hdr);
		exit(1);
	}
	hdr_crc = ext2fs_crc32c_le(~0, (unsigned char *)&undo_ctx.hdr,
				   sizeof(struct undo_header) -
				   sizeof(__u32));
	if (!force && ext2fs_le32_to_cpu(undo_ctx.hdr.header_crc) != hdr_crc) {
		fprintf(stderr, _("%s: Header checksum doesn't match.\n"),
			tdb_file);
		exit(1);
	}
	undo_ctx.blocksize = ext2fs_le32_to_cpu(undo_ctx.hdr.block_size);
	undo_ctx.fs_blocksize = ext2fs_le32_to_cpu(undo_ctx.hdr.fs_block_size);
	if (undo_ctx.blocksize == 0 || undo_ctx.fs_blocksize == 0) {
		fprintf(stderr, _("%s: Corrupt undo file header.\n"), tdb_file);
		exit(1);
	}
	if (!force && undo_ctx.blocksize > E2UNDO_MAX_BLOCK_SIZE) {
		fprintf(stderr, _("%s: Undo block size too large.\n"),
			tdb_file);
		exit(1);
	}
	if (!force && undo_ctx.blocksize < E2UNDO_MIN_BLOCK_SIZE) {
		fprintf(stderr, _("%s: Undo block size too small.\n"),
			tdb_file);
		exit(1);
	}
	undo_ctx.super_block = ext2fs_le64_to_cpu(undo_ctx.hdr.super_offset);
	undo_ctx.num_keys = ext2fs_le64_to_cpu(undo_ctx.hdr.num_keys);
	io_channel_set_blksize(undo_ctx.undo_file, undo_ctx.blocksize);
	/*
	 * Do not compare undo_ctx.hdr.f_compat with the available compatible
	 * features set, because a "missing" compatible feature should
	 * not cause any problems.
	 */
	if (!force && (undo_ctx.hdr.f_incompat || undo_ctx.hdr.f_rocompat)) {
		fprintf(stderr, _("%s: Unknown undo file feature set.\n"),
			tdb_file);
		exit(1);
	}

	/* open the fs */
	retval = ext2fs_check_if_mounted(device_name, &mount_flags);
	if (retval) {
		com_err(prg_name, retval, _("Error while determining whether "
				"%s is mounted."), device_name);
		exit(1);
	}

	if (mount_flags & EXT2_MF_MOUNTED) {
		com_err(prg_name, retval, "%s", _("e2undo should only be run "
						"on unmounted filesystems"));
		exit(1);
	}

	if (undo_file) {
		retval = e2undo_setup_tdb(device_name, &manager);
		if (retval)
			exit(1);
	}

	retval = manager->open(device_name,
			       IO_FLAG_EXCLUSIVE | (dry_run ? 0 : IO_FLAG_RW),
			       &channel);
	if (retval) {
		com_err(prg_name, retval,
				_("while opening `%s'"), device_name);
		exit(1);
	}

	if (*opt_offset_string || e2undo_has_feature_fs_offset(&undo_ctx.hdr)) {
		if (!*opt_offset_string)
			offset = ext2fs_le64_to_cpu(undo_ctx.hdr.fs_offset);
		retval = snprintf(opt_offset_string, sizeof(opt_offset_string),
						  "offset=%llu", offset);
		if ((size_t) retval >= sizeof(opt_offset_string)) {
			/* should not happen... */
			com_err(prg_name, 0, _("specified offset is too large"));
			exit(1);
		}
		io_channel_set_options(channel, opt_offset_string);
	}

	if (!force && check_filesystem(&undo_ctx, channel))
		exit(1);

	/* prepare to read keys */
	retval = ext2fs_get_mem(sizeof(struct undo_key_info) * undo_ctx.num_keys,
				&undo_ctx.keys);
	if (retval) {
		com_err(prg_name, retval, "%s", _("while allocating memory"));
		exit(1);
	}
	ikey = undo_ctx.keys;
	retval = ext2fs_get_mem(undo_ctx.blocksize, &keyb);
	if (retval) {
		com_err(prg_name, retval, "%s", _("while allocating memory"));
		exit(1);
	}
	retval = ext2fs_get_mem(E2UNDO_MAX_EXTENT_BLOCKS * undo_ctx.blocksize,
				&buf);
	if (retval) {
		com_err(prg_name, retval, "%s", _("while allocating memory"));
		exit(1);
	}

	/* load keys */
	keys_per_block = KEYS_PER_BLOCK(&undo_ctx);
	lblk = ext2fs_le64_to_cpu(undo_ctx.hdr.key_offset);
	dbg_printf("nr_keys=%lu, kpb=%zu, blksz=%u\n",
		   undo_ctx.num_keys, keys_per_block, undo_ctx.blocksize);
	for (i = 0; i < undo_ctx.num_keys; i += keys_per_block) {
		size_t j, max_j;
		__le32 crc;

		retval = io_channel_read_blk64(undo_ctx.undo_file,
					       lblk, 1, keyb);
		if (retval) {
			com_err(prg_name, retval, "%s", _("while reading keys"));
			if (force) {
				io_error = 1;
				undo_ctx.num_keys = i - 1;
				break;
			}
			exit(1);
		}

		/* check keys */
		if (!force &&
		    ext2fs_le32_to_cpu(keyb->magic) != KEYBLOCK_MAGIC) {
			fprintf(stderr, _("%s: wrong key magic at %llu\n"),
				tdb_file, lblk);
			exit(1);
		}
		crc = keyb->crc;
		keyb->crc = 0;
		key_crc = ext2fs_crc32c_le(~0, (unsigned char *)keyb,
					   undo_ctx.blocksize);
		if (!force && ext2fs_le32_to_cpu(crc) != key_crc) {
			fprintf(stderr,
				_("%s: key block checksum error at %llu.\n"),
				tdb_file, lblk);
			exit(1);
		}

		/* load keys from key block */
		lblk++;
		max_j = undo_ctx.num_keys - i;
		if (max_j > keys_per_block)
			max_j = keys_per_block;
		for (j = 0, dkey = keyb->keys;
		     j < max_j;
		     j++, ikey++, dkey++) {
			ikey->fsblk = ext2fs_le64_to_cpu(dkey->fsblk);
			ikey->fileblk = lblk;
			ikey->blk_crc = ext2fs_le32_to_cpu(dkey->blk_crc);
			ikey->size = ext2fs_le32_to_cpu(dkey->size);
			lblk += (ikey->size + undo_ctx.blocksize - 1) /
				undo_ctx.blocksize;

			if (E2UNDO_MAX_EXTENT_BLOCKS * undo_ctx.blocksize <
			    ikey->size) {
				com_err(prg_name, retval,
					_("%s: block %llu is too long."),
					tdb_file, ikey->fsblk);
				exit(1);
			}

			/* check each block's crc */
			retval = io_channel_read_blk64(undo_ctx.undo_file,
						       ikey->fileblk,
						       -(int)ikey->size,
						       buf);
			if (retval) {
				com_err(prg_name, retval,
					_("while fetching block %llu."),
					ikey->fileblk);
				if (!force)
					exit(1);
				io_error = 1;
				continue;
			}

			blk_crc = ext2fs_crc32c_le(~0, (unsigned char *)buf,
						   ikey->size);
			if (blk_crc != ikey->blk_crc) {
				fprintf(stderr,
					_("checksum error in filesystem block "
					  "%llu (undo blk %llu)\n"),
					ikey->fsblk, ikey->fileblk);
				if (!force)
					exit(1);
				csum_error = 1;
			}
		}
	}
	ext2fs_free_mem(&keyb);

	/* sort keys in fs block order */
	qsort(undo_ctx.keys, undo_ctx.num_keys, sizeof(struct undo_key_info),
	      key_compare);

	/* replay */
	io_channel_set_blksize(channel, undo_ctx.fs_blocksize);
	for (i = 0, ikey = undo_ctx.keys; i < undo_ctx.num_keys; i++, ikey++) {
		retval = io_channel_read_blk64(undo_ctx.undo_file,
					       ikey->fileblk,
					       -(int)ikey->size,
					       buf);
		if (retval) {
			com_err(prg_name, retval,
				_("while fetching block %llu."),
				ikey->fileblk);
			io_error = 1;
			continue;
		}

		if (verbose)
			printf("Replayed block of size %u from %llu to %llu\n",
				ikey->size, ikey->fileblk, ikey->fsblk);
		if (dry_run)
			continue;
		retval = io_channel_write_blk64(channel, ikey->fsblk,
						-(int)ikey->size, buf);
		if (retval) {
			com_err(prg_name, retval,
				_("while writing block %llu."), ikey->fsblk);
			io_error = 1;
		}
	}

	if (csum_error)
		fprintf(stderr, _("Undo file corruption; run e2fsck NOW!\n"));
	if (io_error)
		fprintf(stderr, _("IO error during replay; run e2fsck NOW!\n"));
	if (!(ext2fs_le32_to_cpu(undo_ctx.hdr.state) & E2UNDO_STATE_FINISHED)) {
		force = 1;
		fprintf(stderr, _("Incomplete undo record; run e2fsck.\n"));
	}
	ext2fs_free_mem(&buf);
	ext2fs_free_mem(&undo_ctx.keys);
	io_channel_close(channel);

	/* If there were problems, try to force a fsck */
	if (!dry_run && (force || csum_error || io_error)) {
		retval = ext2fs_open2(device_name, NULL,
				   EXT2_FLAG_RW | EXT2_FLAG_64BITS, 0, 0,
				   manager, &fs);
		if (retval)
			goto out;
		fs->super->s_state &= ~EXT2_VALID_FS;
		if (csum_error || io_error)
			fs->super->s_state |= EXT2_ERROR_FS;
		ext2fs_mark_super_dirty(fs);
		ext2fs_close_free(&fs);
	}

out:
	io_channel_close(undo_ctx.undo_file);

	return csum_error;
}
