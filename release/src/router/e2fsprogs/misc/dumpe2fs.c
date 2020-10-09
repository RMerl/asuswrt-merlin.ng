/*
 * dumpe2fs.c		- List the control structures of a second
 *			  extended filesystem
 *
 * Copyright (C) 1992, 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                                 Laboratoire MASI, Institut Blaise Pascal
 *                                 Universite Pierre et Marie Curie (Paris VI)
 *
 * Copyright 1995, 1996, 1997 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/*
 * History:
 * 94/01/09	- Creation
 * 94/02/27	- Ported to use the ext2fs library
 */

#include "config.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ext2fs/ext2_fs.h"

#include "ext2fs/ext2fs.h"
#include "e2p/e2p.h"
#include "ext2fs/kernel-jbd.h"
#include <uuid/uuid.h>

#include "support/nls-enable.h"
#include "support/plausible.h"
#include "../version.h"

#define in_use(m, x)	(ext2fs_test_bit ((x), (m)))

static const char * program_name = "dumpe2fs";
static char * device_name = NULL;
static int hex_format = 0;
static int blocks64 = 0;

static void usage(void)
{
	fprintf(stderr, _("Usage: %s [-bfghimxV] [-o superblock=<num>] "
		 "[-o blocksize=<num>] device\n"), program_name);
	exit(1);
}

static void print_number(unsigned long long num)
{
	if (hex_format) {
		if (blocks64)
			printf("0x%08llx", num);
		else
			printf("0x%04llx", num);
	} else
		printf("%llu", num);
}

static void print_range(unsigned long long a, unsigned long long b)
{
	if (hex_format) {
		if (blocks64)
			printf("0x%08llx-0x%08llx", a, b);
		else
			printf("0x%04llx-0x%04llx", a, b);
	} else
		printf("%llu-%llu", a, b);
}

static void print_free(unsigned long group, char * bitmap,
		       unsigned long num, unsigned long offset, int ratio)
{
	int p = 0;
	unsigned long i;
	unsigned long j;

	offset /= ratio;
	offset += group * num;
	for (i = 0; i < num; i++)
		if (!in_use (bitmap, i))
		{
			if (p)
				printf (", ");
			print_number((i + offset) * ratio);
			for (j = i; j < num && !in_use (bitmap, j); j++)
				;
			if (--j != i) {
				fputc('-', stdout);
				print_number((j + offset) * ratio);
				i = j;
			}
			p = 1;
		}
}

static void print_bg_opt(int bg_flags, int mask,
			  const char *str, int *first)
{
	if (bg_flags & mask) {
		if (*first) {
			fputs(" [", stdout);
			*first = 0;
		} else
			fputs(", ", stdout);
		fputs(str, stdout);
	}
}
static void print_bg_opts(ext2_filsys fs, dgrp_t i)
{
	int first = 1, bg_flags = 0;

	if (ext2fs_has_group_desc_csum(fs))
		bg_flags = ext2fs_bg_flags(fs, i);

	print_bg_opt(bg_flags, EXT2_BG_INODE_UNINIT, "INODE_UNINIT",
 		     &first);
	print_bg_opt(bg_flags, EXT2_BG_BLOCK_UNINIT, "BLOCK_UNINIT",
 		     &first);
	print_bg_opt(bg_flags, EXT2_BG_INODE_ZEROED, "ITABLE_ZEROED",
 		     &first);
	if (!first)
		fputc(']', stdout);
	fputc('\n', stdout);
}

static void print_bg_rel_offset(ext2_filsys fs, blk64_t block, int itable,
				blk64_t first_block, blk64_t last_block)
{
	if ((block >= first_block) && (block <= last_block)) {
		if (itable && block == first_block)
			return;
		printf(" (+%u)", (unsigned)(block - first_block));
	} else if (ext2fs_has_feature_flex_bg(fs->super)) {
		dgrp_t flex_grp = ext2fs_group_of_blk2(fs, block);
		printf(" (bg #%u + %u)", flex_grp,
		       (unsigned)(block-ext2fs_group_first_block2(fs,flex_grp)));
	}
}

static void list_desc(ext2_filsys fs, int grp_only)
{
	unsigned long i;
	blk64_t	first_block, last_block;
	blk64_t	super_blk, old_desc_blk, new_desc_blk;
	char *block_bitmap=NULL, *inode_bitmap=NULL;
	const char *units = _("blocks");
	int inode_blocks_per_group, old_desc_blocks, reserved_gdt;
	int		block_nbytes, inode_nbytes;
	int has_super;
	blk64_t		blk_itr = EXT2FS_B2C(fs, fs->super->s_first_data_block);
	ext2_ino_t	ino_itr = 1;
	errcode_t	retval;

	if (ext2fs_has_feature_bigalloc(fs->super))
		units = _("clusters");

	block_nbytes = EXT2_CLUSTERS_PER_GROUP(fs->super) / 8;
	inode_nbytes = EXT2_INODES_PER_GROUP(fs->super) / 8;

	if (fs->block_map)
		block_bitmap = malloc(block_nbytes);
	if (fs->inode_map)
		inode_bitmap = malloc(inode_nbytes);

	inode_blocks_per_group = ((fs->super->s_inodes_per_group *
				   EXT2_INODE_SIZE(fs->super)) +
				  EXT2_BLOCK_SIZE(fs->super) - 1) /
				 EXT2_BLOCK_SIZE(fs->super);
	reserved_gdt = fs->super->s_reserved_gdt_blocks;
	fputc('\n', stdout);
	first_block = fs->super->s_first_data_block;
	if (ext2fs_has_feature_meta_bg(fs->super))
		old_desc_blocks = fs->super->s_first_meta_bg;
	else
		old_desc_blocks = fs->desc_blocks;
	if (grp_only)
		printf("group:block:super:gdt:bbitmap:ibitmap:itable\n");
	for (i = 0; i < fs->group_desc_count; i++) {
		first_block = ext2fs_group_first_block2(fs, i);
		last_block = ext2fs_group_last_block2(fs, i);

		ext2fs_super_and_bgd_loc2(fs, i, &super_blk,
					  &old_desc_blk, &new_desc_blk, 0);

		if (grp_only) {
			printf("%lu:%llu:", i, first_block);
			if (i == 0 || super_blk)
				printf("%llu:", super_blk);
			else
				printf("-1:");
			if (old_desc_blk) {
				print_range(old_desc_blk,
					    old_desc_blk + old_desc_blocks - 1);
				printf(":");
			} else if (new_desc_blk)
				printf("%llu:", new_desc_blk);
			else
				printf("-1:");
			printf("%llu:%llu:%llu\n",
			       ext2fs_block_bitmap_loc(fs, i),
			       ext2fs_inode_bitmap_loc(fs, i),
			       ext2fs_inode_table_loc(fs, i));
			continue;
		}

		printf(_("Group %lu: (Blocks "), i);
		print_range(first_block, last_block);
		fputs(")", stdout);
		if (ext2fs_has_group_desc_csum(fs)) {
			unsigned csum = ext2fs_bg_checksum(fs, i);
			unsigned exp_csum = ext2fs_group_desc_csum(fs, i);

			printf(_(" csum 0x%04x"), csum);
			if (csum != exp_csum)
				printf(_(" (EXPECTED 0x%04x)"), exp_csum);
		}
		print_bg_opts(fs, i);
		has_super = ((i==0) || super_blk);
		if (has_super) {
			printf (_("  %s superblock at "),
				i == 0 ? _("Primary") : _("Backup"));
			print_number(super_blk);
		}
		if (old_desc_blk) {
			printf("%s", _(", Group descriptors at "));
			print_range(old_desc_blk,
				    old_desc_blk + old_desc_blocks - 1);
			if (reserved_gdt) {
				printf("%s", _("\n  Reserved GDT blocks at "));
				print_range(old_desc_blk + old_desc_blocks,
					    old_desc_blk + old_desc_blocks +
					    reserved_gdt - 1);
			}
		} else if (new_desc_blk) {
			fputc(has_super ? ',' : ' ', stdout);
			printf("%s", _(" Group descriptor at "));
			print_number(new_desc_blk);
			has_super++;
		}
		if (has_super)
			fputc('\n', stdout);
		fputs(_("  Block bitmap at "), stdout);
		print_number(ext2fs_block_bitmap_loc(fs, i));
		print_bg_rel_offset(fs, ext2fs_block_bitmap_loc(fs, i), 0,
				    first_block, last_block);
		if (ext2fs_has_feature_metadata_csum(fs->super))
			printf(_(", csum 0x%08x"),
			       ext2fs_block_bitmap_checksum(fs, i));
		if (getenv("DUMPE2FS_IGNORE_80COL"))
			fputs(_(","), stdout);
		else
			fputs(_("\n "), stdout);
		fputs(_(" Inode bitmap at "), stdout);
		print_number(ext2fs_inode_bitmap_loc(fs, i));
		print_bg_rel_offset(fs, ext2fs_inode_bitmap_loc(fs, i), 0,
				    first_block, last_block);
		if (ext2fs_has_feature_metadata_csum(fs->super))
			printf(_(", csum 0x%08x"),
			       ext2fs_inode_bitmap_checksum(fs, i));
		fputs(_("\n  Inode table at "), stdout);
		print_range(ext2fs_inode_table_loc(fs, i),
			    ext2fs_inode_table_loc(fs, i) +
			    inode_blocks_per_group - 1);
		print_bg_rel_offset(fs, ext2fs_inode_table_loc(fs, i), 1,
				    first_block, last_block);
		printf (_("\n  %u free %s, %u free inodes, "
			  "%u directories%s"),
			ext2fs_bg_free_blocks_count(fs, i), units,
			ext2fs_bg_free_inodes_count(fs, i),
			ext2fs_bg_used_dirs_count(fs, i),
			ext2fs_bg_itable_unused(fs, i) ? "" : "\n");
		if (ext2fs_bg_itable_unused(fs, i))
			printf (_(", %u unused inodes\n"),
				ext2fs_bg_itable_unused(fs, i));
		if (block_bitmap) {
			fputs(_("  Free blocks: "), stdout);
			retval = ext2fs_get_block_bitmap_range2(fs->block_map,
				 blk_itr, block_nbytes << 3, block_bitmap);
			if (retval)
				com_err("list_desc", retval,
					"while reading block bitmap");
			else
				print_free(i, block_bitmap,
					   fs->super->s_clusters_per_group,
					   fs->super->s_first_data_block,
					   EXT2FS_CLUSTER_RATIO(fs));
			fputc('\n', stdout);
			blk_itr += fs->super->s_clusters_per_group;
		}
		if (inode_bitmap) {
			fputs(_("  Free inodes: "), stdout);
			retval = ext2fs_get_inode_bitmap_range2(fs->inode_map,
				 ino_itr, inode_nbytes << 3, inode_bitmap);
			if (retval)
				com_err("list_desc", retval,
					"while reading inode bitmap");
			else
				print_free(i, inode_bitmap,
					   fs->super->s_inodes_per_group,
					   1, 1);
			fputc('\n', stdout);
			ino_itr += fs->super->s_inodes_per_group;
		}
	}
	if (block_bitmap)
		free(block_bitmap);
	if (inode_bitmap)
		free(inode_bitmap);
}

static void list_bad_blocks(ext2_filsys fs, int dump)
{
	badblocks_list		bb_list = 0;
	badblocks_iterate	bb_iter;
	blk_t			blk;
	errcode_t		retval;
	const char		*header, *fmt;

	retval = ext2fs_read_bb_inode(fs, &bb_list);
	if (retval) {
		com_err("ext2fs_read_bb_inode", retval, 0);
		return;
	}
	retval = ext2fs_badblocks_list_iterate_begin(bb_list, &bb_iter);
	if (retval) {
		com_err("ext2fs_badblocks_list_iterate_begin", retval,
			"%s", _("while printing bad block list"));
		return;
	}
	if (dump) {
		header = fmt = "%u\n";
	} else {
		header =  _("Bad blocks: %u");
		fmt = ", %u";
	}
	while (ext2fs_badblocks_list_iterate(bb_iter, &blk)) {
		printf(header ? header : fmt, blk);
		header = 0;
	}
	ext2fs_badblocks_list_iterate_end(bb_iter);
	if (!dump)
		fputc('\n', stdout);
	ext2fs_badblocks_list_free(bb_list);
}

static void print_inline_journal_information(ext2_filsys fs)
{
	journal_superblock_t	*jsb;
	struct ext2_inode	inode;
	ext2_file_t		journal_file;
	errcode_t		retval;
	ext2_ino_t		ino = fs->super->s_journal_inum;
	char			buf[1024];

	if (fs->flags & EXT2_FLAG_IMAGE_FILE)
		return;
	retval = ext2fs_read_inode(fs, ino,  &inode);
	if (retval) {
		com_err(program_name, retval, "%s",
			_("while reading journal inode"));
		exit(1);
	}
	retval = ext2fs_file_open2(fs, ino, &inode, 0, &journal_file);
	if (retval) {
		com_err(program_name, retval, "%s",
			_("while opening journal inode"));
		exit(1);
	}
	retval = ext2fs_file_read(journal_file, buf, sizeof(buf), 0);
	if (retval) {
		com_err(program_name, retval, "%s",
			_("while reading journal super block"));
		exit(1);
	}
	ext2fs_file_close(journal_file);
	jsb = (journal_superblock_t *) buf;
	if (be32_to_cpu(jsb->s_header.h_magic) != JFS_MAGIC_NUMBER) {
		fprintf(stderr, "%s",
			_("Journal superblock magic number invalid!\n"));
		exit(1);
	}
	e2p_list_journal_super(stdout, buf, fs->blocksize, 0);
}

static void print_journal_information(ext2_filsys fs)
{
	errcode_t	retval;
	char		buf[1024];
	journal_superblock_t	*jsb;

	/* Get the journal superblock */
	if ((retval = io_channel_read_blk64(fs->io,
					    fs->super->s_first_data_block + 1,
					    -1024, buf))) {
		com_err(program_name, retval, "%s",
			_("while reading journal superblock"));
		exit(1);
	}
	jsb = (journal_superblock_t *) buf;
	if ((jsb->s_header.h_magic != (unsigned) ntohl(JFS_MAGIC_NUMBER)) ||
	    (jsb->s_header.h_blocktype !=
	     (unsigned) ntohl(JFS_SUPERBLOCK_V2))) {
		com_err(program_name, 0, "%s",
			_("Couldn't find journal superblock magic numbers"));
		exit(1);
	}
	e2p_list_journal_super(stdout, buf, fs->blocksize, 0);
}

static int check_mmp(ext2_filsys fs)
{
	int retval;

	/* This won't actually start MMP on the filesystem, since fs is opened
	 * readonly, but it will do the proper activity checking for us. */
	retval = ext2fs_mmp_start(fs);
	if (retval) {
		com_err(program_name, retval, _("while trying to open %s"),
			fs->device_name);
		if (retval == EXT2_ET_MMP_FAILED ||
		    retval == EXT2_ET_MMP_FSCK_ON ||
		    retval == EXT2_ET_MMP_CSUM_INVALID ||
		    retval == EXT2_ET_MMP_UNKNOWN_SEQ) {
			if (fs->mmp_buf) {
				struct mmp_struct *mmp = fs->mmp_buf;
				time_t mmp_time = mmp->mmp_time;

				fprintf(stderr,
					"%s: MMP update by '%.*s%.*s' at %s",
					program_name,
					EXT2_LEN_STR(mmp->mmp_nodename),
					EXT2_LEN_STR(mmp->mmp_bdevname),
					ctime(&mmp_time));
			}
			retval = 1;
		} else {
			retval = 2;
		}
	} else {
		printf("%s: it is safe to mount '%s', MMP is clean\n",
		       program_name, fs->device_name);
	}

	return retval;
}

static void print_mmp_block(ext2_filsys fs)
{
	struct mmp_struct *mmp;
	time_t mmp_time;
	errcode_t retval;

	if (fs->mmp_buf == NULL) {
		retval = ext2fs_get_mem(fs->blocksize, &fs->mmp_buf);
		if (retval) {
			com_err(program_name, retval,
				_("failed to alloc MMP buffer\n"));
			return;
		}
	}

	retval = ext2fs_mmp_read(fs, fs->super->s_mmp_block, fs->mmp_buf);
	/* this is only dumping, not checking status, so OK to skip this */
	if (retval == EXT2_ET_OP_NOT_SUPPORTED)
		return;
	if (retval) {
		com_err(program_name, retval,
			_("reading MMP block %llu from '%s'\n"),
			fs->super->s_mmp_block, fs->device_name);
		return;
	}

	mmp = fs->mmp_buf;
	mmp_time = mmp->mmp_time;
	printf("MMP_block:\n");
	printf("    mmp_magic: 0x%x\n", mmp->mmp_magic);
	printf("    mmp_check_interval: %d\n", mmp->mmp_check_interval);
	printf("    mmp_sequence: %#08x\n", mmp->mmp_seq);
	printf("    mmp_update_date: %s", ctime(&mmp_time));
	printf("    mmp_update_time: %lld\n", mmp->mmp_time);
	printf("    mmp_node_name: %.*s\n",
	       EXT2_LEN_STR(mmp->mmp_nodename));
	printf("    mmp_device_name: %.*s\n",
	       EXT2_LEN_STR(mmp->mmp_bdevname));
}

static void parse_extended_opts(const char *opts, blk64_t *superblock,
				int *blocksize)
{
	char	*buf, *token, *next, *p, *arg, *badopt = 0;
	int	len;
	int	do_usage = 0;

	len = strlen(opts);
	buf = malloc(len+1);
	if (!buf) {
		fprintf(stderr, "%s",
			_("Couldn't allocate memory to parse options!\n"));
		exit(1);
	}
	strcpy(buf, opts);
	for (token = buf; token && *token; token = next) {
		p = strchr(token, ',');
		next = 0;
		if (p) {
			*p = 0;
			next = p+1;
		}
		arg = strchr(token, '=');
		if (arg) {
			*arg = 0;
			arg++;
		}
		if (strcmp(token, "superblock") == 0 ||
		    strcmp(token, "sb") == 0) {
			if (!arg) {
				do_usage++;
				badopt = token;
				continue;
			}
			*superblock = strtoul(arg, &p, 0);
			if (*p) {
				fprintf(stderr,
					_("Invalid superblock parameter: %s\n"),
					arg);
				do_usage++;
				continue;
			}
		} else if (strcmp(token, "blocksize") == 0 ||
			   strcmp(token, "bs") == 0) {
			if (!arg) {
				do_usage++;
				badopt = token;
				continue;
			}
			*blocksize = strtoul(arg, &p, 0);
			if (*p) {
				fprintf(stderr,
					_("Invalid blocksize parameter: %s\n"),
					arg);
				do_usage++;
				continue;
			}
		} else {
			do_usage++;
			badopt = token;
		}
	}
	if (do_usage) {
		fprintf(stderr, _("\nBad extended option(s) specified: %s\n\n"
			"Extended options are separated by commas, "
			"and may take an argument which\n"
			"\tis set off by an equals ('=') sign.\n\n"
			"Valid extended options are:\n"
			"\tsuperblock=<superblock number>\n"
			"\tblocksize=<blocksize>\n"),
			badopt ? badopt : "");
		free(buf);
		exit(1);
	}
	free(buf);
}

int main (int argc, char ** argv)
{
	errcode_t	retval;
	errcode_t	retval_csum = 0;
	const char	*error_csum = NULL;
	ext2_filsys	fs;
	int		print_badblocks = 0;
	blk64_t		use_superblock = 0;
	int		use_blocksize = 0;
	int		image_dump = 0;
	int		mmp_check = 0;
	int		mmp_info = 0;
	int		force = 0;
	int		flags;
	int		header_only = 0;
	int		c;
	int		grp_only = 0;

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
	set_com_err_gettext(gettext);
#endif
	add_error_table(&et_ext2_error_table);
	if (argc && *argv) {
		if (strrchr(*argv, '/'))
			program_name = strrchr(*argv, '/') + 1;
		else
			program_name = *argv;

		if (strstr(program_name, "mmpstatus") != NULL) {
			mmp_check = 1;
			header_only = 1;
		}
	}

	if (!mmp_check)
		fprintf(stderr, "dumpe2fs %s (%s)\n", E2FSPROGS_VERSION,
			E2FSPROGS_DATE);

	while ((c = getopt(argc, argv, "bfghimxVo:")) != EOF) {
		switch (c) {
		case 'b':
			print_badblocks++;
			break;
		case 'f':
			force++;
			break;
		case 'g':
			grp_only++;
			break;
		case 'h':
			header_only++;
			break;
		case 'i':
			if (mmp_check)
				mmp_info++;
			else
				image_dump++;
			break;
		case 'm':
			mmp_check++;
			header_only++;
			if (image_dump) {
				mmp_info = image_dump;
				image_dump = 0;
			}
			break;
		case 'o':
			parse_extended_opts(optarg, &use_superblock,
					    &use_blocksize);
			break;
		case 'V':
			/* Print version number and exit */
			fprintf(stderr, _("\tUsing %s\n"),
				error_message(EXT2_ET_BASE));
			exit(0);
		case 'x':
			hex_format++;
			break;
		default:
			usage();
		}
	}
	if (optind != argc - 1)
		usage();

	device_name = argv[optind++];
	flags = EXT2_FLAG_JOURNAL_DEV_OK | EXT2_FLAG_SOFTSUPP_FEATURES |
		EXT2_FLAG_64BITS;
	if (force)
		flags |= EXT2_FLAG_FORCE;
	if (image_dump)
		flags |= EXT2_FLAG_IMAGE_FILE;
try_open_again:
	if (use_superblock && !use_blocksize) {
		for (use_blocksize = EXT2_MIN_BLOCK_SIZE;
		     use_blocksize <= EXT2_MAX_BLOCK_SIZE;
		     use_blocksize *= 2) {
			retval = ext2fs_open (device_name, flags,
					      use_superblock,
					      use_blocksize, unix_io_manager,
					      &fs);
			if (!retval)
				break;
		}
	} else {
		retval = ext2fs_open(device_name, flags, use_superblock,
				     use_blocksize, unix_io_manager, &fs);
	}
	flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
	if (retval && !retval_csum) {
		retval_csum = retval;
		error_csum = _("while trying to open %s");
		goto try_open_again;
	}
	if (retval) {
		com_err(program_name, retval, _("while trying to open %s"),
			device_name);
		printf("%s", _("Couldn't find valid filesystem superblock.\n"));
		if (retval == EXT2_ET_BAD_MAGIC)
			check_plausibility(device_name, CHECK_FS_EXIST, NULL);
		goto out;
	}
	fs->default_bitmap_type = EXT2FS_BMAP64_RBTREE;
	if (ext2fs_has_feature_64bit(fs->super))
		blocks64 = 1;
	if (mmp_check) {
		if (ext2fs_has_feature_mmp(fs->super) &&
		    fs->super->s_mmp_block != 0) {
			if (mmp_info) {
				print_mmp_block(fs);
				printf("    mmp_block_number: ");
				print_number(fs->super->s_mmp_block);
				printf("\n");
			} else {
				retval = check_mmp(fs);
			}
			if (!retval && retval_csum)
				retval = 2;
		} else {
			fprintf(stderr, _("%s: MMP feature not enabled.\n"),
				program_name);
			retval = 2;
		}
	} else if (print_badblocks) {
		list_bad_blocks(fs, 1);
	} else {
		if (grp_only)
			goto just_descriptors;
		list_super(fs->super);
		if (ext2fs_has_feature_journal_dev(fs->super)) {
			print_journal_information(fs);

			goto out_close;
		}
		if (ext2fs_has_feature_journal(fs->super) &&
		    (fs->super->s_journal_inum != 0))
			print_inline_journal_information(fs);
		if (ext2fs_has_feature_mmp(fs->super) &&
		    fs->super->s_mmp_block != 0)
			print_mmp_block(fs);
		list_bad_blocks(fs, 0);
		if (header_only)
			goto out_close;

		fs->flags &= ~EXT2_FLAG_IGNORE_CSUM_ERRORS;
try_bitmaps_again:
		retval = ext2fs_read_bitmaps(fs);
		if (retval && !retval_csum) {
			fs->flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
			retval_csum = retval;
			error_csum = _("while trying to read '%s' bitmaps\n");
			goto try_bitmaps_again;
		}
just_descriptors:
		list_desc(fs, grp_only);
	}
out_close:
	if (retval_csum) {
		com_err(program_name, retval_csum, error_csum, device_name);
		printf("%s", _("*** Run e2fsck now!\n\n"));
		if (!retval)
			retval = retval_csum;
	}
	ext2fs_close_free(&fs);
	remove_error_table(&et_ext2_error_table);
out:
	return retval;
}
