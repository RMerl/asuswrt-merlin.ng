/*
 * debugfs.c --- a program which allows you to attach an ext2fs
 * filesystem and play with it.
 *
 * Copyright (C) 1993 Theodore Ts'o.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Modifications by Robert Sanders <gt8134b@prism.gatech.edu>
 */

#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern int optind;
extern char *optarg;
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <fcntl.h>
#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#endif

#include "debugfs.h"
#include "uuid/uuid.h"
#include "e2p/e2p.h"

#include <ext2fs/ext2_ext_attr.h>

#include "../version.h"
#include "jfs_user.h"
#include "support/plausible.h"

#ifndef BUFSIZ
#define BUFSIZ 8192
#endif

#ifdef CONFIG_JBD_DEBUG		/* Enabled by configure --enable-jbd-debug */
int journal_enable_debug = -1;
#endif

/*
 * There must be only one definition if we're hooking in extra commands or
 * chaging default prompt. Use -DSKIP_GLOBDEF for that.
 */
#ifndef SKIP_GLOBDEFS
ss_request_table *extra_cmds;
const char *debug_prog_name;
#endif
int ss_sci_idx;

ext2_filsys	current_fs;
quota_ctx_t	current_qctx;
ext2_ino_t	root, cwd;
int		no_copy_xattrs;

static int debugfs_setup_tdb(const char *device_name, char *undo_file,
			     io_manager *io_ptr)
{
	errcode_t retval = ENOMEM;
	const char	*tdb_dir = NULL;
	char		*tdb_file = NULL;
	char		*dev_name, *tmp_name;

	/* (re)open a specific undo file */
	if (undo_file && undo_file[0] != 0) {
		retval = set_undo_io_backing_manager(*io_ptr);
		if (retval)
			goto err;
		*io_ptr = undo_io_manager;
		retval = set_undo_io_backup_file(undo_file);
		if (retval)
			goto err;
		printf("Overwriting existing filesystem; this can be undone "
			"using the command:\n"
			"    e2undo %s %s\n\n",
			undo_file, device_name);
		return retval;
	}

	/*
	 * Configuration via a conf file would be
	 * nice
	 */
	tdb_dir = ss_safe_getenv("E2FSPROGS_UNDO_DIR");
	if (!tdb_dir)
		tdb_dir = "/var/lib/e2fsprogs";

	if (!strcmp(tdb_dir, "none") || (tdb_dir[0] == 0) ||
	    access(tdb_dir, W_OK))
		return 0;

	tmp_name = strdup(device_name);
	if (!tmp_name)
		goto errout;
	dev_name = basename(tmp_name);
	tdb_file = malloc(strlen(tdb_dir) + 9 + strlen(dev_name) + 7 + 1);
	if (!tdb_file) {
		free(tmp_name);
		goto errout;
	}
	sprintf(tdb_file, "%s/debugfs-%s.e2undo", tdb_dir, dev_name);
	free(tmp_name);

	if ((unlink(tdb_file) < 0) && (errno != ENOENT)) {
		retval = errno;
		com_err("debugfs", retval,
			"while trying to delete %s", tdb_file);
		goto errout;
	}

	retval = set_undo_io_backing_manager(*io_ptr);
	if (retval)
		goto errout;
	*io_ptr = undo_io_manager;
	retval = set_undo_io_backup_file(tdb_file);
	if (retval)
		goto errout;
	printf("Overwriting existing filesystem; this can be undone "
		"using the command:\n"
		"    e2undo %s %s\n\n", tdb_file, device_name);

	free(tdb_file);
	return 0;
errout:
	free(tdb_file);
err:
	com_err("debugfs", retval, "while trying to setup undo file\n");
	return retval;
}

static void open_filesystem(char *device, int open_flags, blk64_t superblock,
			    blk64_t blocksize, int catastrophic,
			    char *data_filename, char *undo_file)
{
	int	retval;
	io_channel data_io = 0;
	io_manager io_ptr = unix_io_manager;

	if (superblock != 0 && blocksize == 0) {
		com_err(device, 0, "if you specify the superblock, you must also specify the block size");
		current_fs = NULL;
		return;
	}

	if (data_filename) {
		if ((open_flags & EXT2_FLAG_IMAGE_FILE) == 0) {
			com_err(device, 0,
				"The -d option is only valid when reading an e2image file");
			current_fs = NULL;
			return;
		}
		retval = unix_io_manager->open(data_filename, 0, &data_io);
		if (retval) {
			com_err(data_filename, 0, "while opening data source");
			current_fs = NULL;
			return;
		}
	}

	if (catastrophic)
		open_flags |= EXT2_FLAG_SKIP_MMP | EXT2_FLAG_IGNORE_SB_ERRORS;

	if (undo_file) {
		retval = debugfs_setup_tdb(device, undo_file, &io_ptr);
		if (retval)
			exit(1);
	}

try_open_again:
	retval = ext2fs_open(device, open_flags, superblock, blocksize,
			     io_ptr, &current_fs);
	if (retval && (retval == EXT2_ET_SB_CSUM_INVALID) &&
	    !(open_flags & EXT2_FLAG_IGNORE_CSUM_ERRORS)) {
		open_flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
		printf("Checksum errors in superblock!  Retrying...\n");
		goto try_open_again;
	}
	if (retval) {
		com_err(debug_prog_name, retval,
			"while trying to open %s", device);
		if (retval == EXT2_ET_BAD_MAGIC)
			check_plausibility(device, CHECK_FS_EXIST, NULL);
		current_fs = NULL;
		return;
	}
	current_fs->default_bitmap_type = EXT2FS_BMAP64_RBTREE;

	if (catastrophic)
		com_err(device, 0, "catastrophic mode - not reading inode or group bitmaps");
	else {
		retval = ext2fs_read_bitmaps(current_fs);
		if (retval) {
			com_err(device, retval,
				"while reading allocation bitmaps");
			goto errout;
		}
	}

	if (data_io) {
		retval = ext2fs_set_data_io(current_fs, data_io);
		if (retval) {
			com_err(device, retval,
				"while setting data source");
			goto errout;
		}
	}

	root = cwd = EXT2_ROOT_INO;
	return;

errout:
	retval = ext2fs_close_free(&current_fs);
	if (retval)
		com_err(device, retval, "while trying to close filesystem");
}

void do_open_filesys(int argc, char **argv, int sci_idx EXT2FS_ATTR((unused)),
		     void *infop EXT2FS_ATTR((unused)))
{
	int	c, err;
	int	catastrophic = 0;
	blk64_t	superblock = 0;
	blk64_t	blocksize = 0;
	int	open_flags = EXT2_FLAG_SOFTSUPP_FEATURES | EXT2_FLAG_64BITS; 
	char	*data_filename = 0;
	char	*undo_file = NULL;

	reset_getopt();
	while ((c = getopt(argc, argv, "iwfecb:s:d:Dz:")) != EOF) {
		switch (c) {
		case 'i':
			open_flags |= EXT2_FLAG_IMAGE_FILE;
			break;
		case 'w':
#ifdef READ_ONLY
			goto print_usage;
#else
			open_flags |= EXT2_FLAG_RW;
#endif /* READ_ONLY */
			break;
		case 'f':
			open_flags |= EXT2_FLAG_FORCE;
			break;
		case 'e':
			open_flags |= EXT2_FLAG_EXCLUSIVE;
			break;
		case 'c':
			catastrophic = 1;
			break;
		case 'd':
			data_filename = optarg;
			break;
		case 'D':
			open_flags |= EXT2_FLAG_DIRECT_IO;
			break;
		case 'b':
			blocksize = parse_ulong(optarg, argv[0],
						"block size", &err);
			if (err)
				return;
			break;
		case 's':
			err = strtoblk(argv[0], optarg,
				       "superblock block number", &superblock);
			if (err)
				return;
			break;
		case 'z':
			undo_file = optarg;
			break;
		default:
			goto print_usage;
		}
	}
	if (optind != argc-1) {
		goto print_usage;
	}
	if (check_fs_not_open(argv[0]))
		return;
	open_filesystem(argv[optind], open_flags,
			superblock, blocksize, catastrophic,
			data_filename, undo_file);
	return;

print_usage:
	fprintf(stderr, "%s: Usage: open [-s superblock] [-b blocksize] "
		"[-d image_filename] [-z undo_file] [-c] [-i] [-f] [-e] [-D] "
#ifndef READ_ONLY
		"[-w] "
#endif
		"<device>\n", argv[0]);
}

void do_lcd(int argc, char **argv, int sci_idx EXT2FS_ATTR((unused)),
	    void *infop EXT2FS_ATTR((unused)))
{
	if (argc != 2) {
		com_err(argv[0], 0, "Usage: %s %s", argv[0], "<native dir>");
		return;
	}

	if (chdir(argv[1]) == -1) {
		com_err(argv[0], errno,
			"while trying to change native directory to %s",
			argv[1]);
		return;
	}
}

static void close_filesystem(NOARGS)
{
	int	retval;

	if (current_fs->flags & EXT2_FLAG_IB_DIRTY) {
		retval = ext2fs_write_inode_bitmap(current_fs);
		if (retval)
			com_err("ext2fs_write_inode_bitmap", retval, 0);
	}
	if (current_fs->flags & EXT2_FLAG_BB_DIRTY) {
		retval = ext2fs_write_block_bitmap(current_fs);
		if (retval)
			com_err("ext2fs_write_block_bitmap", retval, 0);
	}
	if (current_qctx)
		quota_release_context(&current_qctx);
	retval = ext2fs_close_free(&current_fs);
	if (retval)
		com_err("ext2fs_close", retval, 0);
	return;
}

void do_close_filesys(int argc, char **argv, int sci_idx EXT2FS_ATTR((unused)),
		      void *infop EXT2FS_ATTR((unused)))
{
	int	c;

	if (check_fs_open(argv[0]))
		return;

	reset_getopt();
	while ((c = getopt (argc, argv, "a")) != EOF) {
		switch (c) {
		case 'a':
			current_fs->flags &= ~EXT2_FLAG_MASTER_SB_ONLY;
			break;
		default:
			goto print_usage;
		}
	}

	if (argc > optind) {
	print_usage:
		com_err(0, 0, "Usage: close_filesys [-a]");
		return;
	}

	close_filesystem();
}

#ifndef READ_ONLY
void do_init_filesys(int argc, char **argv, int sci_idx EXT2FS_ATTR((unused)),
		     void *infop EXT2FS_ATTR((unused)))
{
	struct ext2_super_block param;
	errcode_t	retval;
	int		err;
	blk64_t		blocks;

	if (common_args_process(argc, argv, 3, 3, "initialize",
				"<device> <blocks>", CHECK_FS_NOTOPEN))
		return;

	memset(&param, 0, sizeof(struct ext2_super_block));
	err = strtoblk(argv[0], argv[2], "blocks count", &blocks);
	if (err)
		return;
	ext2fs_blocks_count_set(&param, blocks);
	retval = ext2fs_initialize(argv[1], 0, &param,
				   unix_io_manager, &current_fs);
	if (retval) {
		com_err(argv[1], retval, "while initializing filesystem");
		current_fs = NULL;
		return;
	}
	root = cwd = EXT2_ROOT_INO;
	return;
}

static void print_features(struct ext2_super_block * s, FILE *f)
{
	int	i, j, printed=0;
	__u32	*mask = &s->s_feature_compat, m;

	fputs("Filesystem features:", f);
	for (i=0; i <3; i++,mask++) {
		for (j=0,m=1; j < 32; j++, m<<=1) {
			if (*mask & m) {
				fprintf(f, " %s", e2p_feature2string(i, m));
				printed++;
			}
		}
	}
	if (printed == 0)
		fputs("(none)", f);
	fputs("\n", f);
}
#endif /* READ_ONLY */

static void print_bg_opts(ext2_filsys fs, dgrp_t group, int mask,
			  const char *str, int *first, FILE *f)
{
	if (ext2fs_bg_flags_test(fs, group, mask)) {
		if (*first) {
			fputs("           [", f);
			*first = 0;
		} else
			fputs(", ", f);
		fputs(str, f);
	}
}

void do_show_super_stats(int argc, char *argv[],
			 int sci_idx EXT2FS_ATTR((unused)),
			 void *infop EXT2FS_ATTR((unused)))
{
	const char *units ="block";
	dgrp_t	i;
	FILE 	*out;
	int	c, header_only = 0;
	int	numdirs = 0, first, gdt_csum;

	reset_getopt();
	while ((c = getopt (argc, argv, "h")) != EOF) {
		switch (c) {
		case 'h':
			header_only++;
			break;
		default:
			goto print_usage;
		}
	}
	if (optind != argc) {
		goto print_usage;
	}
	if (check_fs_open(argv[0]))
		return;
	out = open_pager();

	if (ext2fs_has_feature_bigalloc(current_fs->super))
		units = "cluster";

	list_super2(current_fs->super, out);
	if (ext2fs_has_feature_metadata_csum(current_fs->super) &&
	    !ext2fs_superblock_csum_verify(current_fs,
					   current_fs->super)) {
		__u32 orig_csum = current_fs->super->s_checksum;

		ext2fs_superblock_csum_set(current_fs,
					   current_fs->super);
		fprintf(out, "Expected Checksum:        0x%08x\n",
			current_fs->super->s_checksum);
		current_fs->super->s_checksum = orig_csum;
	}
	for (i=0; i < current_fs->group_desc_count; i++)
		numdirs += ext2fs_bg_used_dirs_count(current_fs, i);
	fprintf(out, "Directories:              %u\n", numdirs);

	if (header_only) {
		close_pager(out);
		return;
	}

	gdt_csum = ext2fs_has_group_desc_csum(current_fs);
	for (i = 0; i < current_fs->group_desc_count; i++) {
		fprintf(out, " Group %2d: block bitmap at %llu, "
		        "inode bitmap at %llu, "
		        "inode table at %llu\n"
		        "           %u free %s%s, "
		        "%u free %s, "
		        "%u used %s%s",
		        i, ext2fs_block_bitmap_loc(current_fs, i),
		        ext2fs_inode_bitmap_loc(current_fs, i),
			ext2fs_inode_table_loc(current_fs, i),
		        ext2fs_bg_free_blocks_count(current_fs, i), units,
		        ext2fs_bg_free_blocks_count(current_fs, i) != 1 ?
			"s" : "",
		        ext2fs_bg_free_inodes_count(current_fs, i),
		        ext2fs_bg_free_inodes_count(current_fs, i) != 1 ?
			"inodes" : "inode",
		        ext2fs_bg_used_dirs_count(current_fs, i),
		        ext2fs_bg_used_dirs_count(current_fs, i) != 1 ? "directories"
 				: "directory", gdt_csum ? ", " : "\n");
		if (gdt_csum)
			fprintf(out, "%u unused %s\n",
				ext2fs_bg_itable_unused(current_fs, i),
				ext2fs_bg_itable_unused(current_fs, i) != 1 ?
				"inodes" : "inode");
		first = 1;
		print_bg_opts(current_fs, i, EXT2_BG_INODE_UNINIT, "Inode not init",
			      &first, out);
		print_bg_opts(current_fs, i, EXT2_BG_BLOCK_UNINIT, "Block not init",
			      &first, out);
		if (gdt_csum) {
			fprintf(out, "%sChecksum 0x%04x",
				first ? "           [":", ", ext2fs_bg_checksum(current_fs, i));
			first = 0;
		}
		if (!first)
			fputs("]\n", out);
	}
	close_pager(out);
	return;
print_usage:
	fprintf(stderr, "%s: Usage: show_super_stats [-h]\n", argv[0]);
}

#ifndef READ_ONLY
void do_dirty_filesys(int argc EXT2FS_ATTR((unused)),
		      char **argv EXT2FS_ATTR((unused)),
		      int sci_idx EXT2FS_ATTR((unused)),
		      void *infop EXT2FS_ATTR((unused)))
{
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;

	if (argv[1] && !strcmp(argv[1], "-clean"))
		current_fs->super->s_state |= EXT2_VALID_FS;
	else
		current_fs->super->s_state &= ~EXT2_VALID_FS;
	ext2fs_mark_super_dirty(current_fs);
}
#endif /* READ_ONLY */

struct list_blocks_struct {
	FILE		*f;
	e2_blkcnt_t	total;
	blk64_t		first_block, last_block;
	e2_blkcnt_t	first_bcnt, last_bcnt;
	e2_blkcnt_t	first;
};

static void finish_range(struct list_blocks_struct *lb)
{
	if (lb->first_block == 0)
		return;
	if (lb->first)
		lb->first = 0;
	else
		fprintf(lb->f, ", ");
	if (lb->first_block == lb->last_block)
		fprintf(lb->f, "(%lld):%llu",
			(long long)lb->first_bcnt, lb->first_block);
	else
		fprintf(lb->f, "(%lld-%lld):%llu-%llu",
			(long long)lb->first_bcnt, (long long)lb->last_bcnt,
			lb->first_block, lb->last_block);
	lb->first_block = 0;
}

static int list_blocks_proc(ext2_filsys fs EXT2FS_ATTR((unused)),
			    blk64_t *blocknr, e2_blkcnt_t blockcnt,
			    blk64_t ref_block EXT2FS_ATTR((unused)),
			    int ref_offset EXT2FS_ATTR((unused)),
			    void *private)
{
	struct list_blocks_struct *lb = (struct list_blocks_struct *) private;

	lb->total++;
	if (blockcnt >= 0) {
		/*
		 * See if we can add on to the existing range (if it exists)
		 */
		if (lb->first_block &&
		    (lb->last_block+1 == *blocknr) &&
		    (lb->last_bcnt+1 == blockcnt)) {
			lb->last_block = *blocknr;
			lb->last_bcnt = blockcnt;
			return 0;
		}
		/*
		 * Start a new range.
		 */
		finish_range(lb);
		lb->first_block = lb->last_block = *blocknr;
		lb->first_bcnt = lb->last_bcnt = blockcnt;
		return 0;
	}
	/*
	 * Not a normal block.  Always force a new range.
	 */
	finish_range(lb);
	if (lb->first)
		lb->first = 0;
	else
		fprintf(lb->f, ", ");
	if (blockcnt == -1)
		fprintf(lb->f, "(IND):%llu", (unsigned long long) *blocknr);
	else if (blockcnt == -2)
		fprintf(lb->f, "(DIND):%llu", (unsigned long long) *blocknr);
	else if (blockcnt == -3)
		fprintf(lb->f, "(TIND):%llu", (unsigned long long) *blocknr);
	return 0;
}

static void internal_dump_inode_extra(FILE *out,
				      const char *prefix EXT2FS_ATTR((unused)),
				      ext2_ino_t inode_num EXT2FS_ATTR((unused)),
				      struct ext2_inode_large *inode)
{
	fprintf(out, "Size of extra inode fields: %u\n", inode->i_extra_isize);
	if (inode->i_extra_isize > EXT2_INODE_SIZE(current_fs->super) -
			EXT2_GOOD_OLD_INODE_SIZE) {
		fprintf(stderr, "invalid inode->i_extra_isize (%u)\n",
				inode->i_extra_isize);
		return;
	}
}

static void dump_blocks(FILE *f, const char *prefix, ext2_ino_t inode)
{
	struct list_blocks_struct lb;

	fprintf(f, "%sBLOCKS:\n%s", prefix, prefix);
	lb.total = 0;
	lb.first_block = 0;
	lb.f = f;
	lb.first = 1;
	ext2fs_block_iterate3(current_fs, inode, BLOCK_FLAG_READ_ONLY, NULL,
			      list_blocks_proc, (void *)&lb);
	finish_range(&lb);
	if (lb.total)
		fprintf(f, "\n%sTOTAL: %lld\n", prefix, (long long)lb.total);
	fprintf(f,"\n");
}

static int int_log10(unsigned long long arg)
{
	int     l = 0;

	arg = arg / 10;
	while (arg) {
		l++;
		arg = arg / 10;
	}
	return l;
}

#define DUMP_LEAF_EXTENTS	0x01
#define DUMP_NODE_EXTENTS	0x02
#define DUMP_EXTENT_TABLE	0x04

static void dump_extents(FILE *f, const char *prefix, ext2_ino_t ino,
			 int flags, int logical_width, int physical_width)
{
	ext2_extent_handle_t	handle;
	struct ext2fs_extent	extent;
	struct ext2_extent_info info;
	int			op = EXT2_EXTENT_ROOT;
	unsigned int		printed = 0;
	errcode_t 		errcode;

	errcode = ext2fs_extent_open(current_fs, ino, &handle);
	if (errcode)
		return;

	if (flags & DUMP_EXTENT_TABLE)
		fprintf(f, "Level Entries %*s %*s Length Flags\n",
			(logical_width*2)+3, "Logical",
			(physical_width*2)+3, "Physical");
	else
		fprintf(f, "%sEXTENTS:\n%s", prefix, prefix);

	while (1) {
		errcode = ext2fs_extent_get(handle, op, &extent);

		if (errcode)
			break;

		op = EXT2_EXTENT_NEXT;

		if (extent.e_flags & EXT2_EXTENT_FLAGS_SECOND_VISIT)
			continue;

		if (extent.e_flags & EXT2_EXTENT_FLAGS_LEAF) {
			if ((flags & DUMP_LEAF_EXTENTS) == 0)
				continue;
		} else {
			if ((flags & DUMP_NODE_EXTENTS) == 0)
				continue;
		}

		errcode = ext2fs_extent_get_info(handle, &info);
		if (errcode)
			continue;

		if (!(extent.e_flags & EXT2_EXTENT_FLAGS_LEAF)) {
			if (extent.e_flags & EXT2_EXTENT_FLAGS_SECOND_VISIT)
				continue;

			if (flags & DUMP_EXTENT_TABLE) {
				fprintf(f, "%2d/%2d %3d/%3d %*llu - %*llu "
					"%*llu%*s %6u\n",
					info.curr_level, info.max_depth,
					info.curr_entry, info.num_entries,
					logical_width,
					extent.e_lblk,
					logical_width,
					extent.e_lblk + (extent.e_len - 1),
					physical_width,
					extent.e_pblk,
					physical_width+3, "", extent.e_len);
				continue;
			}

			fprintf(f, "%s(ETB%d):%llu",
				printed ? ", " : "", info.curr_level,
				extent.e_pblk);
			printed = 1;
			continue;
		}

		if (flags & DUMP_EXTENT_TABLE) {
			fprintf(f, "%2d/%2d %3d/%3d %*llu - %*llu "
				"%*llu - %*llu %6u %s\n",
				info.curr_level, info.max_depth,
				info.curr_entry, info.num_entries,
				logical_width,
				extent.e_lblk,
				logical_width,
				extent.e_lblk + (extent.e_len - 1),
				physical_width,
				extent.e_pblk,
				physical_width,
				extent.e_pblk + (extent.e_len - 1),
				extent.e_len,
				extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT ?
					"Uninit" : "");
			continue;
		}

		if (extent.e_len == 0)
			continue;
		else if (extent.e_len == 1)
			fprintf(f,
				"%s(%lld%s):%lld",
				printed ? ", " : "",
				extent.e_lblk,
				extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT ?
				"[u]" : "",
				extent.e_pblk);
		else
			fprintf(f,
				"%s(%lld-%lld%s):%lld-%lld",
				printed ? ", " : "",
				extent.e_lblk,
				extent.e_lblk + (extent.e_len - 1),
				extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT ?
					"[u]" : "",
				extent.e_pblk,
				extent.e_pblk + (extent.e_len - 1));
		printed = 1;
	}
	if (printed)
		fprintf(f, "\n");
	ext2fs_extent_free(handle);
}

static void dump_inline_data(FILE *out, const char *prefix, ext2_ino_t inode_num)
{
	errcode_t retval;
	size_t size;

	retval = ext2fs_inline_data_size(current_fs, inode_num, &size);
	if (!retval)
		fprintf(out, "%sSize of inline data: %zu\n", prefix, size);
}

static void dump_inline_symlink(FILE *out, ext2_ino_t inode_num,
				struct ext2_inode *inode, const char *prefix)
{
	errcode_t retval;
	char *buf = NULL;
	size_t size;

	retval = ext2fs_inline_data_size(current_fs, inode_num, &size);
	if (retval)
		goto out;

	retval = ext2fs_get_memzero(size + 1, &buf);
	if (retval)
		goto out;

	retval = ext2fs_inline_data_get(current_fs, inode_num,
					inode, buf, &size);
	if (retval)
		goto out;

	fprintf(out, "%sFast link dest: \"%.*s\"\n", prefix,
		(int)size, buf);
out:
	if (buf)
		ext2fs_free_mem(&buf);
	if (retval)
		com_err(__func__, retval, "while dumping link destination");
}

void internal_dump_inode(FILE *out, const char *prefix,
			 ext2_ino_t inode_num, struct ext2_inode *inode,
			 int do_dump_blocks)
{
	const char *i_type;
	char frag, fsize;
	int os = current_fs->super->s_creator_os;
	struct ext2_inode_large *large_inode;
	int is_large_inode = 0;

	if (EXT2_INODE_SIZE(current_fs->super) > EXT2_GOOD_OLD_INODE_SIZE)
		is_large_inode = 1;
	large_inode = (struct ext2_inode_large *) inode;

	if (LINUX_S_ISDIR(inode->i_mode)) i_type = "directory";
	else if (LINUX_S_ISREG(inode->i_mode)) i_type = "regular";
	else if (LINUX_S_ISLNK(inode->i_mode)) i_type = "symlink";
	else if (LINUX_S_ISBLK(inode->i_mode)) i_type = "block special";
	else if (LINUX_S_ISCHR(inode->i_mode)) i_type = "character special";
	else if (LINUX_S_ISFIFO(inode->i_mode)) i_type = "FIFO";
	else if (LINUX_S_ISSOCK(inode->i_mode)) i_type = "socket";
	else i_type = "bad type";
	fprintf(out, "%sInode: %u   Type: %s    ", prefix, inode_num, i_type);
	fprintf(out, "%sMode:  0%03o   Flags: 0x%x\n",
		prefix, inode->i_mode & 07777, inode->i_flags);
	if (is_large_inode && large_inode->i_extra_isize >= 24) {
		fprintf(out, "%sGeneration: %u    Version: 0x%08x:%08x\n",
			prefix, inode->i_generation, large_inode->i_version_hi,
			inode->osd1.linux1.l_i_version);
	} else {
		fprintf(out, "%sGeneration: %u    Version: 0x%08x\n", prefix,
			inode->i_generation, inode->osd1.linux1.l_i_version);
	}
	fprintf(out, "%sUser: %5d   Group: %5d",
		prefix, inode_uid(*inode), inode_gid(*inode));
	if (is_large_inode && large_inode->i_extra_isize >= 32)
		fprintf(out, "   Project: %5d", large_inode->i_projid);
	fputs("   Size: ", out);
	if (LINUX_S_ISREG(inode->i_mode) || LINUX_S_ISDIR(inode->i_mode))
		fprintf(out, "%llu\n", EXT2_I_SIZE(inode));
	else
		fprintf(out, "%u\n", inode->i_size);
	if (os == EXT2_OS_HURD)
		fprintf(out,
			"%sFile ACL: %u Translator: %u\n",
			prefix,
			inode->i_file_acl,
			inode->osd1.hurd1.h_i_translator);
	else
		fprintf(out, "%sFile ACL: %llu\n",
			prefix,
			inode->i_file_acl | ((long long)
				(inode->osd2.linux2.l_i_file_acl_high) << 32));
	if (os != EXT2_OS_HURD)
		fprintf(out, "%sLinks: %u   Blockcount: %llu\n",
			prefix, inode->i_links_count,
			(((unsigned long long)
			  inode->osd2.linux2.l_i_blocks_hi << 32)) +
			inode->i_blocks);
	else
		fprintf(out, "%sLinks: %u   Blockcount: %u\n",
			prefix, inode->i_links_count, inode->i_blocks);
	switch (os) {
	    case EXT2_OS_HURD:
		frag = inode->osd2.hurd2.h_i_frag;
		fsize = inode->osd2.hurd2.h_i_fsize;
		break;
	    default:
		frag = fsize = 0;
	}
	fprintf(out, "%sFragment:  Address: %u    Number: %u    Size: %u\n",
		prefix, inode->i_faddr, frag, fsize);
	if (is_large_inode && large_inode->i_extra_isize >= 24) {
		fprintf(out, "%s ctime: 0x%08x:%08x -- %s", prefix,
			inode->i_ctime, large_inode->i_ctime_extra,
			inode_time_to_string(inode->i_ctime,
					     large_inode->i_ctime_extra));
		fprintf(out, "%s atime: 0x%08x:%08x -- %s", prefix,
			inode->i_atime, large_inode->i_atime_extra,
			inode_time_to_string(inode->i_atime,
					     large_inode->i_atime_extra));
		fprintf(out, "%s mtime: 0x%08x:%08x -- %s", prefix,
			inode->i_mtime, large_inode->i_mtime_extra,
			inode_time_to_string(inode->i_mtime,
					     large_inode->i_mtime_extra));
		fprintf(out, "%scrtime: 0x%08x:%08x -- %s", prefix,
			large_inode->i_crtime, large_inode->i_crtime_extra,
			inode_time_to_string(large_inode->i_crtime,
					     large_inode->i_crtime_extra));
		if (inode->i_dtime)
			fprintf(out, "%s dtime: 0x%08x:(%08x) -- %s", prefix,
				large_inode->i_dtime, large_inode->i_ctime_extra,
				inode_time_to_string(inode->i_dtime,
						     large_inode->i_ctime_extra));
	} else {
		fprintf(out, "%sctime: 0x%08x -- %s", prefix, inode->i_ctime,
			time_to_string((__s32) inode->i_ctime));
		fprintf(out, "%satime: 0x%08x -- %s", prefix, inode->i_atime,
			time_to_string((__s32) inode->i_atime));
		fprintf(out, "%smtime: 0x%08x -- %s", prefix, inode->i_mtime,
			time_to_string((__s32) inode->i_mtime));
		if (inode->i_dtime)
			fprintf(out, "%sdtime: 0x%08x -- %s", prefix,
				inode->i_dtime,
				time_to_string((__s32) inode->i_dtime));
	}
	if (EXT2_INODE_SIZE(current_fs->super) > EXT2_GOOD_OLD_INODE_SIZE)
		internal_dump_inode_extra(out, prefix, inode_num,
					  (struct ext2_inode_large *) inode);
	dump_inode_attributes(out, inode_num);
	if (ext2fs_has_feature_metadata_csum(current_fs->super)) {
		__u32 crc = inode->i_checksum_lo;
		if (is_large_inode &&
		    large_inode->i_extra_isize >=
				(offsetof(struct ext2_inode_large,
					  i_checksum_hi) -
				 EXT2_GOOD_OLD_INODE_SIZE))
			crc |= ((__u32)large_inode->i_checksum_hi) << 16;
		fprintf(out, "Inode checksum: 0x%08x\n", crc);
	}

	if (LINUX_S_ISLNK(inode->i_mode) && ext2fs_is_fast_symlink(inode))
		fprintf(out, "%sFast link dest: \"%.*s\"\n", prefix,
			(int)EXT2_I_SIZE(inode), (char *)inode->i_block);
	else if (LINUX_S_ISLNK(inode->i_mode) &&
		   (inode->i_flags & EXT4_INLINE_DATA_FL))
		dump_inline_symlink(out, inode_num, inode, prefix);
	else if (LINUX_S_ISBLK(inode->i_mode) || LINUX_S_ISCHR(inode->i_mode)) {
		int major, minor;
		const char *devnote;

		if (inode->i_block[0]) {
			major = (inode->i_block[0] >> 8) & 255;
			minor = inode->i_block[0] & 255;
			devnote = "";
		} else {
			major = (inode->i_block[1] & 0xfff00) >> 8;
			minor = ((inode->i_block[1] & 0xff) |
				 ((inode->i_block[1] >> 12) & 0xfff00));
			devnote = "(New-style) ";
		}
		fprintf(out, "%sDevice major/minor number: %02d:%02d (hex %02x:%02x)\n",
			devnote, major, minor, major, minor);
	} else if (do_dump_blocks) {
		if (inode->i_flags & EXT4_EXTENTS_FL)
			dump_extents(out, prefix, inode_num,
				     DUMP_LEAF_EXTENTS|DUMP_NODE_EXTENTS, 0, 0);
		else if (inode->i_flags & EXT4_INLINE_DATA_FL)
			dump_inline_data(out, prefix, inode_num);
		else
			dump_blocks(out, prefix, inode_num);
	}
}

static void dump_inode(ext2_ino_t inode_num, struct ext2_inode *inode)
{
	FILE	*out;

	out = open_pager();
	internal_dump_inode(out, "", inode_num, inode, 1);
	close_pager(out);
}

void do_stat(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	     void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	inode;
	struct ext2_inode * inode_buf;

	if (check_fs_open(argv[0]))
		return;

	inode_buf = (struct ext2_inode *)
			malloc(EXT2_INODE_SIZE(current_fs->super));
	if (!inode_buf) {
		fprintf(stderr, "do_stat: can't allocate buffer\n");
		return;
	}

	if (common_inode_args_process(argc, argv, &inode, 0)) {
		free(inode_buf);
		return;
	}

	if (debugfs_read_inode2(inode, inode_buf, argv[0],
				EXT2_INODE_SIZE(current_fs->super), 0)) {
		free(inode_buf);
		return;
	}

	dump_inode(inode, inode_buf);
	free(inode_buf);
	return;
}

void do_dump_extents(int argc, char **argv, int sci_idx EXT2FS_ATTR((unused)),
		     void *infop EXT2FS_ATTR((unused)))
{
	struct ext2_inode inode;
	ext2_ino_t	ino;
	FILE		*out;
	int		c, flags = 0;
	int		logical_width;
	int		physical_width;

	reset_getopt();
	while ((c = getopt(argc, argv, "nl")) != EOF) {
		switch (c) {
		case 'n':
			flags |= DUMP_NODE_EXTENTS;
			break;
		case 'l':
			flags |= DUMP_LEAF_EXTENTS;
			break;
		}
	}

	if (argc != optind + 1) {
		com_err(0, 0, "Usage: dump_extents [-n] [-l] file");
		return;
	}

	if (flags == 0)
		flags = DUMP_NODE_EXTENTS | DUMP_LEAF_EXTENTS;
	flags |= DUMP_EXTENT_TABLE;

	if (check_fs_open(argv[0]))
		return;

	ino = string_to_inode(argv[optind]);
	if (ino == 0)
		return;

	if (debugfs_read_inode(ino, &inode, argv[0]))
		return;

	if ((inode.i_flags & EXT4_EXTENTS_FL) == 0) {
		fprintf(stderr, "%s: does not uses extent block maps\n",
			argv[optind]);
		return;
	}

	logical_width = int_log10((EXT2_I_SIZE(&inode)+current_fs->blocksize-1)/
				  current_fs->blocksize) + 1;
	if (logical_width < 5)
		logical_width = 5;
	physical_width = int_log10(ext2fs_blocks_count(current_fs->super)) + 1;
	if (physical_width < 5)
		physical_width = 5;

	out = open_pager();
	dump_extents(out, "", ino, flags, logical_width, physical_width);
	close_pager(out);
	return;
}

static int print_blocks_proc(ext2_filsys fs EXT2FS_ATTR((unused)),
			     blk64_t *blocknr,
			     e2_blkcnt_t blockcnt EXT2FS_ATTR((unused)),
			     blk64_t ref_block EXT2FS_ATTR((unused)),
			     int ref_offset EXT2FS_ATTR((unused)),
			     void *private EXT2FS_ATTR((unused)))
{
	printf("%llu ", *blocknr);
	return 0;
}

void do_blocks(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	       void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	inode;

	if (check_fs_open(argv[0]))
		return;

	if (common_inode_args_process(argc, argv, &inode, 0)) {
		return;
	}

	ext2fs_block_iterate3(current_fs, inode, BLOCK_FLAG_READ_ONLY, NULL,
			      print_blocks_proc, NULL);
	fputc('\n', stdout);
	return;
}

void do_chroot(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	       void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t inode;
	int retval;

	if (common_inode_args_process(argc, argv, &inode, 0))
		return;

	retval = ext2fs_check_directory(current_fs, inode);
	if (retval)  {
		com_err(argv[1], retval, 0);
		return;
	}
	root = inode;
}

#ifndef READ_ONLY
void do_clri(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	     void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t inode;
	struct ext2_inode inode_buf;

	if (common_inode_args_process(argc, argv, &inode, CHECK_FS_RW))
		return;

	if (debugfs_read_inode(inode, &inode_buf, argv[0]))
		return;
	memset(&inode_buf, 0, sizeof(inode_buf));
	if (debugfs_write_inode(inode, &inode_buf, argv[0]))
		return;
}

void do_freei(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	unsigned int	len = 1;
	int		err = 0;
	ext2_ino_t	inode;

	if (common_args_process(argc, argv, 2, 3, argv[0], "<file> [num]",
				CHECK_FS_RW | CHECK_FS_BITMAPS))
		return;
	if (check_fs_read_write(argv[0]))
		return;

	inode = string_to_inode(argv[1]);
	if (!inode)
		return;

	if (argc == 3) {
		len = parse_ulong(argv[2], argv[0], "length", &err);
		if (err)
			return;
	}

	if (len == 1 &&
	    !ext2fs_test_inode_bitmap2(current_fs->inode_map,inode))
		com_err(argv[0], 0, "Warning: inode already clear");
	while (len-- > 0)
		ext2fs_unmark_inode_bitmap2(current_fs->inode_map, inode++);
	ext2fs_mark_ib_dirty(current_fs);
}

void do_seti(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	     void *infop EXT2FS_ATTR((unused)))
{
	unsigned int	len = 1;
	int		err = 0;
	ext2_ino_t	inode;

	if (common_args_process(argc, argv, 2, 3, argv[0], "<file> [num]",
				CHECK_FS_RW | CHECK_FS_BITMAPS))
		return;
	if (check_fs_read_write(argv[0]))
		return;

	inode = string_to_inode(argv[1]);
	if (!inode)
		return;

	if (argc == 3) {
		len = parse_ulong(argv[2], argv[0], "length", &err);
		if (err)
			return;
	}

	if ((len == 1) &&
	    ext2fs_test_inode_bitmap2(current_fs->inode_map,inode))
		com_err(argv[0], 0, "Warning: inode already set");
	while (len-- > 0)
		ext2fs_mark_inode_bitmap2(current_fs->inode_map, inode++);
	ext2fs_mark_ib_dirty(current_fs);
}
#endif /* READ_ONLY */

void do_testi(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t inode;

	if (common_inode_args_process(argc, argv, &inode, CHECK_FS_BITMAPS))
		return;

	if (ext2fs_test_inode_bitmap2(current_fs->inode_map,inode))
		printf("Inode %u is marked in use\n", inode);
	else
		printf("Inode %u is not in use\n", inode);
}

#ifndef READ_ONLY
void do_freeb(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	blk64_t block;
	blk64_t count = 1;

	if (common_block_args_process(argc, argv, &block, &count))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	while (count-- > 0) {
		if (!ext2fs_test_block_bitmap2(current_fs->block_map,block))
			com_err(argv[0], 0, "Warning: block %llu already clear",
				block);
		ext2fs_unmark_block_bitmap2(current_fs->block_map,block);
		block++;
	}
	ext2fs_mark_bb_dirty(current_fs);
}

void do_setb(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	     void *infop EXT2FS_ATTR((unused)))
{
	blk64_t block;
	blk64_t count = 1;

	if (common_block_args_process(argc, argv, &block, &count))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	while (count-- > 0) {
		if (ext2fs_test_block_bitmap2(current_fs->block_map,block))
			com_err(argv[0], 0, "Warning: block %llu already set",
				block);
		ext2fs_mark_block_bitmap2(current_fs->block_map,block);
		block++;
	}
	ext2fs_mark_bb_dirty(current_fs);
}
#endif /* READ_ONLY */

void do_testb(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	blk64_t block;
	blk64_t count = 1;

	if (common_block_args_process(argc, argv, &block, &count))
		return;
	while (count-- > 0) {
		if (ext2fs_test_block_bitmap2(current_fs->block_map,block))
			printf("Block %llu marked in use\n", block);
		else
			printf("Block %llu not in use\n", block);
		block++;
	}
}

#ifndef READ_ONLY
static void modify_u8(char *com, const char *prompt,
		      const char *format, __u8 *val)
{
	char buf[200];
	unsigned long v;
	char *tmp;

	sprintf(buf, format, *val);
	printf("%30s    [%s] ", prompt, buf);
	if (!fgets(buf, sizeof(buf), stdin))
		return;
	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';
	if (!buf[0])
		return;
	v = strtoul(buf, &tmp, 0);
	if (*tmp)
		com_err(com, 0, "Bad value - %s", buf);
	else
		*val = v;
}

static void modify_u16(char *com, const char *prompt,
		       const char *format, __u16 *val)
{
	char buf[200];
	unsigned long v;
	char *tmp;

	sprintf(buf, format, *val);
	printf("%30s    [%s] ", prompt, buf);
	if (!fgets(buf, sizeof(buf), stdin))
		return;
	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';
	if (!buf[0])
		return;
	v = strtoul(buf, &tmp, 0);
	if (*tmp)
		com_err(com, 0, "Bad value - %s", buf);
	else
		*val = v;
}

static void modify_u32(char *com, const char *prompt,
		       const char *format, __u32 *val)
{
	char buf[200];
	unsigned long v;
	char *tmp;

	sprintf(buf, format, *val);
	printf("%30s    [%s] ", prompt, buf);
	if (!fgets(buf, sizeof(buf), stdin))
		return;
	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';
	if (!buf[0])
		return;
	v = strtoul(buf, &tmp, 0);
	if (*tmp)
		com_err(com, 0, "Bad value - %s", buf);
	else
		*val = v;
}


void do_modify_inode(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		     void *infop EXT2FS_ATTR((unused)))
{
	struct ext2_inode inode;
	ext2_ino_t	inode_num;
	int 		i;
	unsigned char	*frag, *fsize;
	char		buf[80];
	int 		os;
	const char	*hex_format = "0x%x";
	const char	*octal_format = "0%o";
	const char	*decimal_format = "%d";
	const char	*unsignedlong_format = "%lu";

	if (common_inode_args_process(argc, argv, &inode_num, CHECK_FS_RW))
		return;

	os = current_fs->super->s_creator_os;

	if (debugfs_read_inode(inode_num, &inode, argv[1]))
		return;

	modify_u16(argv[0], "Mode", octal_format, &inode.i_mode);
	modify_u16(argv[0], "User ID", decimal_format, &inode.i_uid);
	modify_u16(argv[0], "Group ID", decimal_format, &inode.i_gid);
	modify_u32(argv[0], "Size", unsignedlong_format, &inode.i_size);
	modify_u32(argv[0], "Creation time", decimal_format, &inode.i_ctime);
	modify_u32(argv[0], "Modification time", decimal_format, &inode.i_mtime);
	modify_u32(argv[0], "Access time", decimal_format, &inode.i_atime);
	modify_u32(argv[0], "Deletion time", decimal_format, &inode.i_dtime);
	modify_u16(argv[0], "Link count", decimal_format, &inode.i_links_count);
	if (os == EXT2_OS_LINUX)
		modify_u16(argv[0], "Block count high", unsignedlong_format,
			   &inode.osd2.linux2.l_i_blocks_hi);
	modify_u32(argv[0], "Block count", unsignedlong_format, &inode.i_blocks);
	modify_u32(argv[0], "File flags", hex_format, &inode.i_flags);
	modify_u32(argv[0], "Generation", hex_format, &inode.i_generation);
#if 0
	modify_u32(argv[0], "Reserved1", decimal_format, &inode.i_reserved1);
#endif
	modify_u32(argv[0], "File acl", decimal_format, &inode.i_file_acl);

	modify_u32(argv[0], "High 32bits of size", decimal_format,
		   &inode.i_size_high);

	if (os == EXT2_OS_HURD)
		modify_u32(argv[0], "Translator Block",
			    decimal_format, &inode.osd1.hurd1.h_i_translator);

	modify_u32(argv[0], "Fragment address", decimal_format, &inode.i_faddr);
	switch (os) {
	    case EXT2_OS_HURD:
		frag = &inode.osd2.hurd2.h_i_frag;
		fsize = &inode.osd2.hurd2.h_i_fsize;
		break;
	    default:
		frag = fsize = 0;
	}
	if (frag)
		modify_u8(argv[0], "Fragment number", decimal_format, frag);
	if (fsize)
		modify_u8(argv[0], "Fragment size", decimal_format, fsize);

	for (i=0;  i < EXT2_NDIR_BLOCKS; i++) {
		sprintf(buf, "Direct Block #%u", i);
		modify_u32(argv[0], buf, decimal_format, &inode.i_block[i]);
	}
	modify_u32(argv[0], "Indirect Block", decimal_format,
		    &inode.i_block[EXT2_IND_BLOCK]);
	modify_u32(argv[0], "Double Indirect Block", decimal_format,
		    &inode.i_block[EXT2_DIND_BLOCK]);
	modify_u32(argv[0], "Triple Indirect Block", decimal_format,
		    &inode.i_block[EXT2_TIND_BLOCK]);
	if (debugfs_write_inode(inode_num, &inode, argv[1]))
		return;
}
#endif /* READ_ONLY */

void do_change_working_dir(int argc, char *argv[],
			   int sci_idx EXT2FS_ATTR((unused)),
			   void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	inode;
	int		retval;

	if (common_inode_args_process(argc, argv, &inode, 0))
		return;

	retval = ext2fs_check_directory(current_fs, inode);
	if (retval) {
		com_err(argv[1], retval, 0);
		return;
	}
	cwd = inode;
	return;
}

void do_print_working_directory(int argc, char *argv[],
				int sci_idx EXT2FS_ATTR((unused)),
				void *infop EXT2FS_ATTR((unused)))
{
	int	retval;
	char	*pathname = NULL;

	if (common_args_process(argc, argv, 1, 1,
				"print_working_directory", "", 0))
		return;

	retval = ext2fs_get_pathname(current_fs, cwd, 0, &pathname);
	if (retval) {
		com_err(argv[0], retval,
			"while trying to get pathname of cwd");
	}
	printf("[pwd]   INODE: %6u  PATH: %s\n",
	       cwd, pathname ? pathname : "NULL");
        if (pathname) {
		free(pathname);
		pathname = NULL;
        }
	retval = ext2fs_get_pathname(current_fs, root, 0, &pathname);
	if (retval) {
		com_err(argv[0], retval,
			"while trying to get pathname of root");
	}
	printf("[root]  INODE: %6u  PATH: %s\n",
	       root, pathname ? pathname : "NULL");
	if (pathname) {
		free(pathname);
		pathname = NULL;
	}
	return;
}

#ifndef READ_ONLY
static void make_link(char *sourcename, char *destname)
{
	ext2_ino_t	ino;
	struct ext2_inode inode;
	int		retval;
	ext2_ino_t	dir;
	char		*dest, *cp, *base_name;

	/*
	 * Get the source inode
	 */
	ino = string_to_inode(sourcename);
	if (!ino)
		return;
	base_name = strrchr(sourcename, '/');
	if (base_name)
		base_name++;
	else
		base_name = sourcename;
	/*
	 * Figure out the destination.  First see if it exists and is
	 * a directory.
	 */
	if (! (retval=ext2fs_namei(current_fs, root, cwd, destname, &dir)))
		dest = base_name;
	else {
		/*
		 * OK, it doesn't exist.  See if it is
		 * '<dir>/basename' or 'basename'
		 */
		cp = strrchr(destname, '/');
		if (cp) {
			*cp = 0;
			dir = string_to_inode(destname);
			if (!dir)
				return;
			dest = cp+1;
		} else {
			dir = cwd;
			dest = destname;
		}
	}

	if (debugfs_read_inode(ino, &inode, sourcename))
		return;

	retval = ext2fs_link(current_fs, dir, dest, ino,
			     ext2_file_type(inode.i_mode));
	if (retval)
		com_err("make_link", retval, 0);
	return;
}


void do_link(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	     void *infop EXT2FS_ATTR((unused)))
{
	if (common_args_process(argc, argv, 3, 3, "link",
				"<source file> <dest_name>", CHECK_FS_RW))
		return;

	make_link(argv[1], argv[2]);
}

static int mark_blocks_proc(ext2_filsys fs, blk64_t *blocknr,
			    e2_blkcnt_t blockcnt EXT2FS_ATTR((unused)),
			    blk64_t ref_block EXT2FS_ATTR((unused)),
			    int ref_offset EXT2FS_ATTR((unused)),
			    void *private EXT2FS_ATTR((unused)))
{
	blk64_t	block;

	block = *blocknr;
	ext2fs_block_alloc_stats2(fs, block, +1);
	return 0;
}

void do_undel(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	ino;
	struct ext2_inode inode;

	if (common_args_process(argc, argv, 2, 3, "undelete",
				"<inode_num> [dest_name]",
				CHECK_FS_RW | CHECK_FS_BITMAPS))
		return;

	ino = string_to_inode(argv[1]);
	if (!ino)
		return;

	if (debugfs_read_inode(ino, &inode, argv[1]))
		return;

	if (ext2fs_test_inode_bitmap2(current_fs->inode_map, ino)) {
		com_err(argv[1], 0, "Inode is not marked as deleted");
		return;
	}

	/*
	 * XXX this function doesn't handle changing the links count on the
	 * parent directory when undeleting a directory.
	 */
	inode.i_links_count = LINUX_S_ISDIR(inode.i_mode) ? 2 : 1;
	inode.i_dtime = 0;

	if (debugfs_write_inode(ino, &inode, argv[0]))
		return;

	ext2fs_block_iterate3(current_fs, ino, BLOCK_FLAG_READ_ONLY, NULL,
			      mark_blocks_proc, NULL);

	ext2fs_inode_alloc_stats2(current_fs, ino, +1, 0);

	if (argc > 2)
		make_link(argv[1], argv[2]);
}

static void unlink_file_by_name(char *filename)
{
	int		retval;
	ext2_ino_t	dir;
	char		*base_name;

	base_name = strrchr(filename, '/');
	if (base_name) {
		*base_name++ = '\0';
		dir = string_to_inode(filename);
		if (!dir)
			return;
	} else {
		dir = cwd;
		base_name = filename;
	}
	retval = ext2fs_unlink(current_fs, dir, base_name, 0, 0);
	if (retval)
		com_err("unlink_file_by_name", retval, 0);
	return;
}

void do_unlink(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	       void *infop EXT2FS_ATTR((unused)))
{
	if (common_args_process(argc, argv, 2, 2, "link",
				"<pathname>", CHECK_FS_RW))
		return;

	unlink_file_by_name(argv[1]);
}

void do_copy_inode(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		   void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	src_ino, dest_ino;
	unsigned char	buf[4096];

	if (common_args_process(argc, argv, 3, 3, "copy_inode",
				"<source file> <dest_name>", CHECK_FS_RW))
		return;

	src_ino = string_to_inode(argv[1]);
	if (!src_ino)
		return;

	dest_ino = string_to_inode(argv[2]);
	if (!dest_ino)
		return;

	if (debugfs_read_inode2(src_ino, (struct ext2_inode *) buf,
				argv[0], sizeof(buf), 0))
		return;

	if (debugfs_write_inode2(dest_ino, (struct ext2_inode *) buf,
				 argv[0], sizeof(buf), 0))
		return;
}

#endif /* READ_ONLY */

void do_find_free_block(int argc, char *argv[],
			int sci_idx EXT2FS_ATTR((unused)),
			void *infop EXT2FS_ATTR((unused)))
{
	blk64_t	free_blk, goal, first_free = 0;
 	int		count;
	errcode_t	retval;
	char		*tmp;

	if ((argc > 3) || (argc==2 && *argv[1] == '?')) {
		com_err(argv[0], 0, "Usage: find_free_block [count [goal]]");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	if (argc > 1) {
		count = strtol(argv[1],&tmp,0);
		if (*tmp) {
			com_err(argv[0], 0, "Bad count - %s", argv[1]);
			return;
		}
 	} else
		count = 1;

	if (argc > 2) {
		goal = strtol(argv[2], &tmp, 0);
		if (*tmp) {
			com_err(argv[0], 0, "Bad goal - %s", argv[1]);
			return;
		}
	}
	else
		goal = current_fs->super->s_first_data_block;

	printf("Free blocks found: ");
	free_blk = goal - 1;
	while (count-- > 0) {
		retval = ext2fs_new_block2(current_fs, free_blk + 1, 0,
					   &free_blk);
		if (first_free) {
			if (first_free == free_blk)
				break;
		} else
			first_free = free_blk;
		if (retval) {
			com_err("ext2fs_new_block", retval, 0);
			return;
		} else
			printf("%llu ", free_blk);
	}
 	printf("\n");
}

void do_find_free_inode(int argc, char *argv[],
			int sci_idx EXT2FS_ATTR((unused)),
			void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	free_inode, dir;
	int		mode;
	int		retval;
	char		*tmp;

	if (argc > 3 || (argc>1 && *argv[1] == '?')) {
		com_err(argv[0], 0, "Usage: find_free_inode [dir [mode]]");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	if (argc > 1) {
		dir = strtol(argv[1], &tmp, 0);
		if (*tmp) {
			com_err(argv[0], 0, "Bad dir - %s", argv[1]);
			return;
		}
	}
	else
		dir = root;
	if (argc > 2) {
		mode = strtol(argv[2], &tmp, 0);
		if (*tmp) {
			com_err(argv[0], 0, "Bad mode - %s", argv[2]);
			return;
		}
	} else
		mode = 010755;

	retval = ext2fs_new_inode(current_fs, dir, mode, 0, &free_inode);
	if (retval)
		com_err("ext2fs_new_inode", retval, 0);
	else
		printf("Free inode found: %u\n", free_inode);
}

#ifndef READ_ONLY
void do_write(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	errcode_t	retval;

	if (common_args_process(argc, argv, 3, 3, "write",
				"<native file> <new file>", CHECK_FS_RW))
		return;

	retval = do_write_internal(current_fs, cwd, argv[1], argv[2], root);
	if (retval)
		com_err(argv[0], retval, 0);
}

void do_mknod(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	unsigned long	major, minor;
	errcode_t 	retval;
	int		nr;
	struct stat	st;

	if (check_fs_open(argv[0]))
		return;
	if (argc < 3 || argv[2][1]) {
	usage:
		com_err(argv[0], 0, "Usage: mknod <name> [p| [c|b] <major> <minor>]");
		return;
	}

	minor = major = 0;
	switch (argv[2][0]) {
		case 'p':
			st.st_mode = S_IFIFO;
			nr = 3;
			break;
		case 'c':
			st.st_mode = S_IFCHR;
			nr = 5;
			break;
		case 'b':
			st.st_mode = S_IFBLK;
			nr = 5;
			break;
		default:
			nr = 0;
	}

	if (nr == 5) {
		major = strtoul(argv[3], argv+3, 0);
		minor = strtoul(argv[4], argv+4, 0);
		if (major > 65535 || minor > 65535 || argv[3][0] || argv[4][0])
			nr = 0;
	}

	if (argc != nr)
		goto usage;

	st.st_rdev = makedev(major, minor);
	retval = do_mknod_internal(current_fs, cwd, argv[1],
				   st.st_mode, st.st_rdev);
	if (retval)
		com_err(argv[0], retval, 0);
}

void do_mkdir(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	errcode_t retval;

	if (common_args_process(argc, argv, 2, 2, "mkdir",
				"<filename>", CHECK_FS_RW))
		return;

	retval = do_mkdir_internal(current_fs, cwd, argv[1], root);
	if (retval)
		com_err(argv[0], retval, 0);

}

static int release_blocks_proc(ext2_filsys fs, blk64_t *blocknr,
			       e2_blkcnt_t blockcnt EXT2FS_ATTR((unused)),
			       blk64_t ref_block EXT2FS_ATTR((unused)),
			       int ref_offset EXT2FS_ATTR((unused)),
			       void *private)
{
	blk64_t	block = *blocknr;
	blk64_t *last_cluster = (blk64_t *)private;
	blk64_t cluster = EXT2FS_B2C(fs, block);

	if (cluster == *last_cluster)
		return 0;

	*last_cluster = cluster;

	ext2fs_block_alloc_stats2(fs, block, -1);
	return 0;
}

static void kill_file_by_inode(ext2_ino_t inode)
{
	struct ext2_inode inode_buf;

	if (debugfs_read_inode(inode, &inode_buf, 0))
		return;
	inode_buf.i_dtime = current_fs->now ? current_fs->now : time(0);
	if (debugfs_write_inode(inode, &inode_buf, 0))
		return;
	if (ext2fs_inode_has_valid_blocks2(current_fs, &inode_buf)) {
		blk64_t last_cluster = 0;
		ext2fs_block_iterate3(current_fs, inode, BLOCK_FLAG_READ_ONLY,
				      NULL, release_blocks_proc, &last_cluster);
	}
	printf("\n");
	ext2fs_inode_alloc_stats2(current_fs, inode, -1,
				  LINUX_S_ISDIR(inode_buf.i_mode));
}


void do_kill_file(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		  void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t inode_num;

	if (common_inode_args_process(argc, argv, &inode_num, CHECK_FS_RW))
		return;

	kill_file_by_inode(inode_num);
}

void do_rm(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	   void *infop EXT2FS_ATTR((unused)))
{
	int retval;
	ext2_ino_t inode_num;
	struct ext2_inode inode;

	if (common_args_process(argc, argv, 2, 2, "rm",
				"<filename>", CHECK_FS_RW))
		return;

	retval = ext2fs_namei(current_fs, root, cwd, argv[1], &inode_num);
	if (retval) {
		com_err(argv[0], retval, "while trying to resolve filename");
		return;
	}

	if (debugfs_read_inode(inode_num, &inode, argv[0]))
		return;

	if (LINUX_S_ISDIR(inode.i_mode)) {
		com_err(argv[0], 0, "file is a directory");
		return;
	}

	--inode.i_links_count;
	if (debugfs_write_inode(inode_num, &inode, argv[0]))
		return;

	unlink_file_by_name(argv[1]);
	if (inode.i_links_count == 0)
		kill_file_by_inode(inode_num);
}

struct rd_struct {
	ext2_ino_t	parent;
	int		empty;
};

static int rmdir_proc(ext2_ino_t dir EXT2FS_ATTR((unused)),
		      int	entry EXT2FS_ATTR((unused)),
		      struct ext2_dir_entry *dirent,
		      int	offset EXT2FS_ATTR((unused)),
		      int	blocksize EXT2FS_ATTR((unused)),
		      char	*buf EXT2FS_ATTR((unused)),
		      void	*private)
{
	struct rd_struct *rds = (struct rd_struct *) private;

	if (dirent->inode == 0)
		return 0;
	if ((ext2fs_dirent_name_len(dirent) == 1) && (dirent->name[0] == '.'))
		return 0;
	if ((ext2fs_dirent_name_len(dirent) == 2) && (dirent->name[0] == '.') &&
	    (dirent->name[1] == '.')) {
		rds->parent = dirent->inode;
		return 0;
	}
	rds->empty = 0;
	return 0;
}

void do_rmdir(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	int retval;
	ext2_ino_t inode_num;
	struct ext2_inode inode;
	struct rd_struct rds;

	if (common_args_process(argc, argv, 2, 2, "rmdir",
				"<filename>", CHECK_FS_RW))
		return;

	retval = ext2fs_namei(current_fs, root, cwd, argv[1], &inode_num);
	if (retval) {
		com_err(argv[0], retval, "while trying to resolve filename");
		return;
	}

	if (debugfs_read_inode(inode_num, &inode, argv[0]))
		return;

	if (!LINUX_S_ISDIR(inode.i_mode)) {
		com_err(argv[0], 0, "file is not a directory");
		return;
	}

	rds.parent = 0;
	rds.empty = 1;

	retval = ext2fs_dir_iterate2(current_fs, inode_num, 0,
				    0, rmdir_proc, &rds);
	if (retval) {
		com_err(argv[0], retval, "while iterating over directory");
		return;
	}
	if (rds.empty == 0) {
		com_err(argv[0], 0, "directory not empty");
		return;
	}

	inode.i_links_count = 0;
	if (debugfs_write_inode(inode_num, &inode, argv[0]))
		return;

	unlink_file_by_name(argv[1]);
	kill_file_by_inode(inode_num);

	if (rds.parent) {
		if (debugfs_read_inode(rds.parent, &inode, argv[0]))
			return;
		if (inode.i_links_count > 1)
			inode.i_links_count--;
		if (debugfs_write_inode(rds.parent, &inode, argv[0]))
			return;
	}
}
#endif /* READ_ONLY */

void do_show_debugfs_params(int argc EXT2FS_ATTR((unused)),
			    char *argv[] EXT2FS_ATTR((unused)),
			    int sci_idx EXT2FS_ATTR((unused)),
			    void *infop EXT2FS_ATTR((unused)))
{
	if (current_fs)
		printf("Open mode: read-%s\n",
		       current_fs->flags & EXT2_FLAG_RW ? "write" : "only");
	printf("Filesystem in use: %s\n",
	       current_fs ? current_fs->device_name : "--none--");
}

#ifndef READ_ONLY
void do_expand_dir(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		    void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t inode;
	int retval;

	if (common_inode_args_process(argc, argv, &inode, CHECK_FS_RW))
		return;

	retval = ext2fs_expand_dir(current_fs, inode);
	if (retval)
		com_err("ext2fs_expand_dir", retval, 0);
	return;
}

void do_features(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		 void *infop EXT2FS_ATTR((unused)))
{
	int	i;

	if (check_fs_open(argv[0]))
		return;

	if ((argc != 1) && check_fs_read_write(argv[0]))
		return;
	for (i=1; i < argc; i++) {
		if (e2p_edit_feature(argv[i],
				     &current_fs->super->s_feature_compat, 0))
			com_err(argv[0], 0, "Unknown feature: %s\n",
				argv[i]);
		else
			ext2fs_mark_super_dirty(current_fs);
	}
	print_features(current_fs->super, stdout);
}
#endif /* READ_ONLY */

void do_bmap(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	     void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	ino;
	blk64_t		blk, pblk = 0;
	int		c, err, flags = 0, ret_flags = 0;
	errcode_t	errcode;

	if (check_fs_open(argv[0]))
		return;

	reset_getopt();
	while ((c = getopt (argc, argv, "a")) != EOF) {
		switch (c) {
		case 'a':
			flags |= BMAP_ALLOC;
			break;
		default:
			goto print_usage;
		}
	}

	if (argc <= optind+1) {
	print_usage:
		com_err(0, 0,
			"Usage: bmap [-a] <file> logical_blk [physical_blk]");
		return;
	}

	ino = string_to_inode(argv[optind++]);
	if (!ino)
		return;
	err = strtoblk(argv[0], argv[optind++], "logical block", &blk);
	if (err)
		return;

	if (argc > optind+1)
		goto print_usage;

	if (argc == optind+1) {
		err = strtoblk(argv[0], argv[optind++],
			       "physical block", &pblk);
		if (err)
			return;
		if (flags & BMAP_ALLOC) {
			com_err(0, 0, "Can't set and allocate a block");
			return;
		}
		flags |= BMAP_SET;
	}

	errcode = ext2fs_bmap2(current_fs, ino, 0, 0, flags, blk,
			       &ret_flags, &pblk);
	if (errcode) {
		com_err(argv[0], errcode,
			"while mapping logical block %llu\n", blk);
		return;
	}
	printf("%llu", pblk);
	if (ret_flags & BMAP_RET_UNINIT)
		fputs(" (uninit)", stdout);
	fputc('\n', stdout);
}

void do_imap(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	     void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	ino;
	unsigned long 	group, block, block_nr, offset;

	if (common_args_process(argc, argv, 2, 2, argv[0],
				"<file>", 0))
		return;
	ino = string_to_inode(argv[1]);
	if (!ino)
		return;

	group = (ino - 1) / EXT2_INODES_PER_GROUP(current_fs->super);
	offset = ((ino - 1) % EXT2_INODES_PER_GROUP(current_fs->super)) *
		EXT2_INODE_SIZE(current_fs->super);
	block = offset >> EXT2_BLOCK_SIZE_BITS(current_fs->super);
	if (!ext2fs_inode_table_loc(current_fs, (unsigned)group)) {
		com_err(argv[0], 0, "Inode table for group %lu is missing\n",
			group);
		return;
	}
	block_nr = ext2fs_inode_table_loc(current_fs, (unsigned)group) +
		block;
	offset &= (EXT2_BLOCK_SIZE(current_fs->super) - 1);

	printf("Inode %u is part of block group %lu\n"
	       "\tlocated at block %lu, offset 0x%04lx\n", ino, group,
	       block_nr, offset);

}

void do_idump(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	struct ext2_inode_large *inode;
	ext2_ino_t	ino;
	unsigned char	*buf;
	errcode_t	err;
	unsigned int	isize, size, offset = 0;
	int		c, mode = 0;

	reset_getopt();
	while ((c = getopt (argc, argv, "bex")) != EOF) {
		if (mode || c == '?') {
			com_err(argv[0], 0,
				"Usage: inode_dump [-b]|[-e] <file>");
			return;
		}
		mode = c;
	}
	if (optind != argc-1)
		return;

	if (check_fs_open(argv[0]))
		return;

	ino = string_to_inode(argv[optind]);
	if (!ino)
		return;

	isize = EXT2_INODE_SIZE(current_fs->super);
	err = ext2fs_get_mem(isize, &buf);
	if (err) {
		com_err(argv[0], err, "while allocating memory");
		return;
	}

	err = ext2fs_read_inode_full(current_fs, ino,
				     (struct ext2_inode *)buf, isize);
	if (err) {
		com_err(argv[0], err, "while reading inode %u", ino);
		goto err;
	}

	inode = (struct ext2_inode_large *) buf;
	size = isize;
	switch (mode) {
	case 'b':
		offset = ((char *) (&inode->i_block)) - ((char *) buf);
		size = sizeof(inode->i_block);
		break;
	case 'x':
	case 'e':
		if (size <= EXT2_GOOD_OLD_INODE_SIZE) {
			com_err(argv[0], 0, "No extra space in inode");
			goto err;
		}
		offset = EXT2_GOOD_OLD_INODE_SIZE + inode->i_extra_isize;
		if (offset > size)
			goto err;
		size -= offset;
		break;
	}
	if (mode == 'x')
		raw_inode_xattr_dump(stdout, buf + offset, size);
	else
		do_byte_hexdump(stdout, buf + offset, size);
err:
	ext2fs_free_mem(&buf);
}

#ifndef READ_ONLY
void do_set_current_time(int argc, char *argv[],
			 int sci_idx EXT2FS_ATTR((unused)),
			 void *infop EXT2FS_ATTR((unused)))
{
	__s64 now;

	if (common_args_process(argc, argv, 2, 2, argv[0],
				"<time>", 0))
		return;

	now = string_to_time(argv[1]);
	if (now == -1) {
		com_err(argv[0], 0, "Couldn't parse argument as a time: %s\n",
			argv[1]);
		return;

	} else {
		printf("Setting current time to %s\n", time_to_string(now));
		current_fs->now = now;
	}
}
#endif /* READ_ONLY */

static int find_supp_feature(__u32 *supp, int feature_type, char *name)
{
	int compat, bit, ret;
	unsigned int feature_mask;

	if (name) {
		if (feature_type == E2P_FS_FEATURE)
			ret = e2p_string2feature(name, &compat, &feature_mask);
		else
			ret = e2p_jrnl_string2feature(name, &compat,
						      &feature_mask);
		if (ret)
			return ret;

		if (!(supp[compat] & feature_mask))
			return 1;
	} else {
	        for (compat = 0; compat < 3; compat++) {
		        for (bit = 0, feature_mask = 1; bit < 32;
			     bit++, feature_mask <<= 1) {
			        if (supp[compat] & feature_mask) {
					if (feature_type == E2P_FS_FEATURE)
						fprintf(stdout, " %s",
						e2p_feature2string(compat,
						feature_mask));
					else
						fprintf(stdout, " %s",
						e2p_jrnl_feature2string(compat,
						feature_mask));
				}
	        	}
		}
	        fprintf(stdout, "\n");
	}

	return 0;
}

void do_supported_features(int argc, char *argv[],
			   int sci_idx EXT2FS_ATTR((unused)),
			   void *infop EXT2FS_ATTR((unused)))
{
        int	ret;
	__u32	supp[3] = { EXT2_LIB_FEATURE_COMPAT_SUPP,
			    EXT2_LIB_FEATURE_INCOMPAT_SUPP,
			    EXT2_LIB_FEATURE_RO_COMPAT_SUPP };
	__u32	jrnl_supp[3] = { JFS_KNOWN_COMPAT_FEATURES,
				 JFS_KNOWN_INCOMPAT_FEATURES,
				 JFS_KNOWN_ROCOMPAT_FEATURES };

	if (argc > 1) {
		ret = find_supp_feature(supp, E2P_FS_FEATURE, argv[1]);
		if (ret) {
			ret = find_supp_feature(jrnl_supp, E2P_JOURNAL_FEATURE,
						argv[1]);
		}
		if (ret)
			com_err(argv[0], 0, "Unknown feature: %s\n", argv[1]);
		else
			fprintf(stdout, "Supported feature: %s\n", argv[1]);
	} else {
		fprintf(stdout, "Supported features:");
		ret = find_supp_feature(supp, E2P_FS_FEATURE, NULL);
		ret = find_supp_feature(jrnl_supp, E2P_JOURNAL_FEATURE, NULL);
	}
}

#ifndef READ_ONLY
void do_punch(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
	      void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	ino;
	blk64_t		start, end;
	int		err;
	errcode_t	errcode;

	if (common_args_process(argc, argv, 3, 4, argv[0],
				"<file> start_blk [end_blk]",
				CHECK_FS_RW | CHECK_FS_BITMAPS))
		return;

	ino = string_to_inode(argv[1]);
	if (!ino)
		return;
	err = strtoblk(argv[0], argv[2], "logical block", &start);
	if (err)
		return;
	if (argc == 4) {
		err = strtoblk(argv[0], argv[3], "logical block", &end);
		if (err)
			return;
	} else
		end = ~0;

	errcode = ext2fs_punch(current_fs, ino, 0, 0, start, end);

	if (errcode) {
		com_err(argv[0], errcode,
			"while truncating inode %u from %llu to %llu\n", ino,
			(unsigned long long) start, (unsigned long long) end);
		return;
	}
}

void do_fallocate(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		  void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t	ino;
	blk64_t		start, end;
	int		err;
	errcode_t	errcode;

	if (common_args_process(argc, argv, 3, 4, argv[0],
				"<file> start_blk [end_blk]",
				CHECK_FS_RW | CHECK_FS_BITMAPS))
		return;

	ino = string_to_inode(argv[1]);
	if (!ino)
		return;
	err = strtoblk(argv[0], argv[2], "logical block", &start);
	if (err)
		return;
	if (argc == 4) {
		err = strtoblk(argv[0], argv[3], "logical block", &end);
		if (err)
			return;
	} else
		end = ~0;

	errcode = ext2fs_fallocate(current_fs, EXT2_FALLOCATE_INIT_BEYOND_EOF,
				   ino, NULL, ~0ULL, start, end - start + 1);

	if (errcode) {
		com_err(argv[0], errcode,
			"while fallocating inode %u from %llu to %llu\n", ino,
			(unsigned long long) start, (unsigned long long) end);
		return;
	}
}
#endif /* READ_ONLY */

void do_symlink(int argc, char *argv[], int sci_idx EXT2FS_ATTR((unused)),
		void *infop EXT2FS_ATTR((unused)))
{
	errcode_t	retval;

	if (common_args_process(argc, argv, 3, 3, "symlink",
				"<filename> <target>", CHECK_FS_RW))
		return;

	retval = do_symlink_internal(current_fs, cwd, argv[1], argv[2], root);
	if (retval)
		com_err(argv[0], retval, 0);

}

#if CONFIG_MMP
void do_dump_mmp(int argc EXT2FS_ATTR((unused)), char *argv[],
		 int sci_idx EXT2FS_ATTR((unused)),
		 void *infop EXT2FS_ATTR((unused)))
{
	struct mmp_struct *mmp_s;
	unsigned long long mmp_block;
	time_t t;
	errcode_t retval = 0;

	if (check_fs_open(argv[0]))
		return;

	if (argc > 1) {
		char *end = NULL;
		mmp_block = strtoull(argv[1], &end, 0);
		if (end == argv[0] || mmp_block == 0) {
			fprintf(stderr, "%s: invalid MMP block '%s' given\n",
				argv[0], argv[1]);
			return;
		}
	} else {
		mmp_block = current_fs->super->s_mmp_block;
	}

	if (mmp_block == 0) {
		fprintf(stderr, "%s: MMP: not active on this filesystem.\n",
			argv[0]);
		return;
	}

	if (current_fs->mmp_buf == NULL) {
		retval = ext2fs_get_mem(current_fs->blocksize,
					&current_fs->mmp_buf);
		if (retval) {
			com_err(argv[0], retval, "allocating MMP buffer.\n");
			return;
		}
	}

	mmp_s = current_fs->mmp_buf;

	retval = ext2fs_mmp_read(current_fs, mmp_block, current_fs->mmp_buf);
	if (retval) {
		com_err(argv[0], retval, "reading MMP block %llu.\n",
			mmp_block);
		return;
	}

	t = mmp_s->mmp_time;
	fprintf(stdout, "block_number: %llu\n", current_fs->super->s_mmp_block);
	fprintf(stdout, "update_interval: %d\n",
		current_fs->super->s_mmp_update_interval);
	fprintf(stdout, "check_interval: %d\n", mmp_s->mmp_check_interval);
	fprintf(stdout, "sequence: %08x\n", mmp_s->mmp_seq);
	fprintf(stdout, "time: %lld -- %s", mmp_s->mmp_time, ctime(&t));
	fprintf(stdout, "node_name: %.*s\n",
		EXT2_LEN_STR(mmp_s->mmp_nodename));
	fprintf(stdout, "device_name: %.*s\n",
		EXT2_LEN_STR(mmp_s->mmp_bdevname));
	fprintf(stdout, "magic: 0x%x\n", mmp_s->mmp_magic);
	fprintf(stdout, "checksum: 0x%08x\n", mmp_s->mmp_checksum);
}
#else
void do_dump_mmp(int argc EXT2FS_ATTR((unused)),
		 char *argv[] EXT2FS_ATTR((unused)),
		 int sci_idx EXT2FS_ATTR((unused)),
		 void *infop EXT2FS_ATTR((unused)))
{
	fprintf(stdout, "MMP is unsupported, please recompile with "
	                "--enable-mmp\n");
}
#endif

static int source_file(const char *cmd_file, int ss_idx)
{
	FILE		*f;
	char		buf[BUFSIZ];
	char		*cp;
	int		exit_status = 0;
	int		retval;

	if (strcmp(cmd_file, "-") == 0)
		f = stdin;
	else {
		f = fopen(cmd_file, "r");
		if (!f) {
			perror(cmd_file);
			exit(1);
		}
	}
	fflush(stdout);
	fflush(stderr);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	while (!feof(f)) {
		if (fgets(buf, sizeof(buf), f) == NULL)
			break;
		if (buf[0] == '#') {
			printf("%s", buf);
			continue;
		}
		cp = strchr(buf, '\n');
		if (cp)
			*cp = 0;
		cp = strchr(buf, '\r');
		if (cp)
			*cp = 0;
		printf("debugfs: %s\n", buf);
		retval = ss_execute_line(ss_idx, buf);
		if (retval) {
			ss_perror(ss_idx, retval, buf);
			exit_status++;
		}
	}
	if (f != stdin)
		fclose(f);
	return exit_status;
}

int main(int argc, char **argv)
{
	int		retval;
	const char	*usage = 
		"Usage: %s [-b blocksize] [-s superblock] [-f cmd_file] "
		"[-R request] [-d data_source_device] [-i] [-n] [-D] [-V] ["
#ifndef READ_ONLY
		"[-w] [-z undo_file] "
#endif
		"[-c]] [device]";
	int		c;
	int		open_flags = EXT2_FLAG_SOFTSUPP_FEATURES | EXT2_FLAG_64BITS;
	char		*request = 0;
	int		exit_status = 0;
	char		*cmd_file = 0;
	blk64_t		superblock = 0;
	blk64_t		blocksize = 0;
	int		catastrophic = 0;
	char		*data_filename = 0;
#ifdef READ_ONLY
	const char	*opt_string = "nicR:f:b:s:Vd:D";
#else
	const char	*opt_string = "niwcR:f:b:s:Vd:Dz:";
	char		*undo_file = NULL;
#endif
#ifdef CONFIG_JBD_DEBUG
	char		*jbd_debug;
#endif

	if (debug_prog_name == 0)
#ifdef READ_ONLY
		debug_prog_name = "rdebugfs";
#else
		debug_prog_name = "debugfs";
#endif
	add_error_table(&et_ext2_error_table);
	fprintf (stderr, "%s %s (%s)\n", debug_prog_name,
		 E2FSPROGS_VERSION, E2FSPROGS_DATE);

#ifdef CONFIG_JBD_DEBUG
	jbd_debug = ss_safe_getenv("DEBUGFS_JBD_DEBUG");
	if (jbd_debug) {
		int res = sscanf(jbd_debug, "%d", &journal_enable_debug);

		if (res != 1) {
			fprintf(stderr,
				"DEBUGFS_JBD_DEBUG \"%s\" not an integer\n\n",
				jbd_debug);
			exit(1);
		}
	}
#endif
	while ((c = getopt (argc, argv, opt_string)) != EOF) {
		switch (c) {
		case 'R':
			request = optarg;
			break;
		case 'f':
			cmd_file = optarg;
			break;
		case 'd':
			data_filename = optarg;
			break;
		case 'i':
			open_flags |= EXT2_FLAG_IMAGE_FILE;
			break;
		case 'n':
			open_flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
			break;
#ifndef READ_ONLY
		case 'w':
			open_flags |= EXT2_FLAG_RW;
			break;
#endif
		case 'D':
			open_flags |= EXT2_FLAG_DIRECT_IO;
			break;
		case 'b':
			blocksize = parse_ulong(optarg, argv[0],
						"block size", 0);
			break;
		case 's':
			retval = strtoblk(argv[0], optarg,
					  "superblock block number",
					  &superblock);
			if (retval)
				return 1;
			break;
		case 'c':
			catastrophic = 1;
			break;
		case 'V':
			/* Print version number and exit */
			fprintf(stderr, "\tUsing %s\n",
				error_message(EXT2_ET_BASE));
			exit(0);
		case 'z':
			undo_file = optarg;
			break;
		default:
			com_err(argv[0], 0, usage, debug_prog_name);
			return 1;
		}
	}
	if (optind < argc)
		open_filesystem(argv[optind], open_flags,
				superblock, blocksize, catastrophic,
				data_filename, undo_file);

	ss_sci_idx = ss_create_invocation(debug_prog_name, "0.0", (char *) NULL,
					  &debug_cmds, &retval);
	if (retval) {
		ss_perror(ss_sci_idx, retval, "creating invocation");
		exit(1);
	}
	ss_get_readline(ss_sci_idx);

	(void) ss_add_request_table(ss_sci_idx, &ss_std_requests, 1, &retval);
	if (retval) {
		ss_perror(ss_sci_idx, retval, "adding standard requests");
		exit (1);
	}
	if (extra_cmds)
		ss_add_request_table(ss_sci_idx, extra_cmds, 1, &retval);
	if (retval) {
		ss_perror(ss_sci_idx, retval, "adding extra requests");
		exit (1);
	}
	if (request) {
		retval = 0;
		retval = ss_execute_line(ss_sci_idx, request);
		if (retval) {
			ss_perror(ss_sci_idx, retval, request);
			exit_status++;
		}
	} else if (cmd_file) {
		exit_status = source_file(cmd_file, ss_sci_idx);
	} else {
		ss_listen(ss_sci_idx);
	}

	ss_delete_invocation(ss_sci_idx);

	if (current_fs)
		close_filesystem();

	remove_error_table(&et_ext2_error_table);
	return exit_status;
}
