/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 *
 * This is set up for u-boot.
 *
 * This version now uses the ydevconfig mechanism to set up partitions.
 */

#include <common.h>
#include <div64.h>

#include <config.h>
#include "nand.h"
#include "yaffscfg.h"
#include "yaffsfs.h"
#include "yaffs_packedtags2.h"
#include "yaffs_mtdif.h"
#include "yaffs_mtdif2.h"
#if 0
#include <errno.h>
#else
#include "malloc.h"
#endif

unsigned yaffs_trace_mask = 0x0; /* Disable logging */
static int yaffs_errno;


void yaffs_bug_fn(const char *fn, int n)
{
	printf("yaffs bug at %s:%d\n", fn, n);
}

void *yaffsfs_malloc(size_t x)
{
	return malloc(x);
}

void yaffsfs_free(void *x)
{
	free(x);
}

void yaffsfs_SetError(int err)
{
	yaffs_errno = err;
}

int yaffsfs_GetLastError(void)
{
	return yaffs_errno;
}


int yaffsfs_GetError(void)
{
	return yaffs_errno;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

void *yaffs_malloc(size_t size)
{
	return malloc(size);
}

void yaffs_free(void *ptr)
{
	free(ptr);
}

void yaffsfs_LocalInitialisation(void)
{
	/* No locking used */
}


static const char *yaffs_file_type_str(struct yaffs_stat *stat)
{
	switch (stat->st_mode & S_IFMT) {
	case S_IFREG: return "regular file";
	case S_IFDIR: return "directory";
	case S_IFLNK: return "symlink";
	default: return "unknown";
	}
}

static const char *yaffs_error_str(void)
{
	int error = yaffsfs_GetLastError();

	if (error < 0)
		error = -error;

	switch (error) {
	case EBUSY: return "Busy";
	case ENODEV: return "No such device";
	case EINVAL: return "Invalid parameter";
	case ENFILE: return "Too many open files";
	case EBADF:  return "Bad handle";
	case EACCES: return "Wrong permissions";
	case EXDEV:  return "Not on same device";
	case ENOENT: return "No such entry";
	case ENOSPC: return "Device full";
	case EROFS:  return "Read only file system";
	case ERANGE: return "Range error";
	case ENOTEMPTY: return "Not empty";
	case ENAMETOOLONG: return "Name too long";
	case ENOMEM: return "Out of memory";
	case EFAULT: return "Fault";
	case EEXIST: return "Name exists";
	case ENOTDIR: return "Not a directory";
	case EISDIR: return "Not permitted on a directory";
	case ELOOP:  return "Symlink loop";
	case 0: return "No error";
	default: return "Unknown error";
	}
}

void cmd_yaffs_tracemask(unsigned set, unsigned mask)
{
	if (set)
		yaffs_trace_mask = mask;

	printf("yaffs trace mask: %08x\n", yaffs_trace_mask);
}

static int yaffs_regions_overlap(int a, int b, int x, int y)
{
	return	(a <= x && x <= b) ||
		(a <= y && y <= b) ||
		(x <= a && a <= y) ||
		(x <= b && b <= y);
}

void cmd_yaffs_devconfig(char *_mp, int flash_dev,
			int start_block, int end_block)
{
	struct mtd_info *mtd = NULL;
	struct yaffs_dev *dev = NULL;
	struct yaffs_dev *chk;
	char *mp = NULL;
	struct nand_chip *chip;

	mtd = get_nand_dev_by_index(flash_dev);
	if (!mtd) {
		pr_err("\nno NAND devices available\n");
		return;
	}

	dev = calloc(1, sizeof(*dev));
	mp = strdup(_mp);

	if (!dev || !mp) {
		/* Alloc error */
		printf("Failed to allocate memory\n");
		goto err;
	}

	if (flash_dev >= CONFIG_SYS_MAX_NAND_DEVICE) {
		printf("Flash device invalid\n");
		goto err;
	}

	if (end_block == 0)
		end_block = lldiv(mtd->size, mtd->erasesize - 1);

	if (end_block < start_block) {
		printf("Bad start/end\n");
		goto err;
	}

	chip =  mtd_to_nand(mtd);

	/* Check for any conflicts */
	yaffs_dev_rewind();
	while (1) {
		chk = yaffs_next_dev();
		if (!chk)
			break;
		if (strcmp(chk->param.name, mp) == 0) {
			printf("Mount point name already used\n");
			goto err;
		}
		if (chk->driver_context == mtd &&
			yaffs_regions_overlap(
				chk->param.start_block, chk->param.end_block,
				start_block, end_block)) {
			printf("Region overlaps with partition %s\n",
				chk->param.name);
			goto err;
		}

	}

	/* Seems sane, so configure */
	memset(dev, 0, sizeof(*dev));
	dev->param.name = mp;
	dev->driver_context = mtd;
	dev->param.start_block = start_block;
	dev->param.end_block = end_block;
	dev->param.chunks_per_block = mtd->erasesize / mtd->writesize;
	dev->param.total_bytes_per_chunk = mtd->writesize;
	dev->param.is_yaffs2 = 1;
	dev->param.use_nand_ecc = 1;
	dev->param.n_reserved_blocks = 5;
	if (chip->ecc.layout->oobavail < sizeof(struct yaffs_packed_tags2))
		dev->param.inband_tags = 1;
	dev->param.n_caches = 10;
	dev->param.write_chunk_tags_fn = nandmtd2_write_chunk_tags;
	dev->param.read_chunk_tags_fn = nandmtd2_read_chunk_tags;
	dev->param.erase_fn = nandmtd_EraseBlockInNAND;
	dev->param.initialise_flash_fn = nandmtd_InitialiseNAND;
	dev->param.bad_block_fn = nandmtd2_MarkNANDBlockBad;
	dev->param.query_block_fn = nandmtd2_QueryNANDBlock;

	yaffs_add_device(dev);

	printf("Configures yaffs mount %s: dev %d start block %d, end block %d %s\n",
		mp, flash_dev, start_block, end_block,
		dev->param.inband_tags ? "using inband tags" : "");
	return;

err:
	free(dev);
	free(mp);
}

void cmd_yaffs_dev_ls(void)
{
	struct yaffs_dev *dev;
	int flash_dev;
	int free_space;

	yaffs_dev_rewind();

	while (1) {
		dev = yaffs_next_dev();
		if (!dev)
			return;
		flash_dev = nand_mtd_to_devnum(dev->driver_context);
		printf("%-10s %5d 0x%05x 0x%05x %s",
			dev->param.name, flash_dev,
			dev->param.start_block, dev->param.end_block,
			dev->param.inband_tags ? "using inband tags, " : "");

		free_space = yaffs_freespace(dev->param.name);
		if (free_space < 0)
			printf("not mounted\n");
		else
			printf("free 0x%x\n", free_space);

	}
}

void make_a_file(char *yaffsName, char bval, int sizeOfFile)
{
	int outh;
	int i;
	unsigned char buffer[100];

	outh = yaffs_open(yaffsName,
				O_CREAT | O_RDWR | O_TRUNC,
				S_IREAD | S_IWRITE);
	if (outh < 0) {
		printf("Error opening file: %d. %s\n", outh, yaffs_error_str());
		return;
	}

	memset(buffer, bval, 100);

	do {
		i = sizeOfFile;
		if (i > 100)
			i = 100;
		sizeOfFile -= i;

		yaffs_write(outh, buffer, i);

	} while (sizeOfFile > 0);


	yaffs_close(outh);
}

void read_a_file(char *fn)
{
	int h;
	int i = 0;
	unsigned char b;

	h = yaffs_open(fn, O_RDWR, 0);
	if (h < 0) {
		printf("File not found\n");
		return;
	}

	while (yaffs_read(h, &b, 1) > 0) {
		printf("%02x ", b);
		i++;
		if (i > 32) {
			printf("\n");
			i = 0;
		}
	}
	printf("\n");
	yaffs_close(h);
}

void cmd_yaffs_mount(char *mp)
{
	int retval = yaffs_mount(mp);
	if (retval < 0)
		printf("Error mounting %s, return value: %d, %s\n", mp,
			yaffsfs_GetError(), yaffs_error_str());
}


void cmd_yaffs_umount(char *mp)
{
	if (yaffs_unmount(mp) == -1)
		printf("Error umounting %s, return value: %d, %s\n", mp,
			yaffsfs_GetError(), yaffs_error_str());
}

void cmd_yaffs_write_file(char *yaffsName, char bval, int sizeOfFile)
{
	make_a_file(yaffsName, bval, sizeOfFile);
}


void cmd_yaffs_read_file(char *fn)
{
	read_a_file(fn);
}


void cmd_yaffs_mread_file(char *fn, char *addr)
{
	int h;
	struct yaffs_stat s;

	yaffs_stat(fn, &s);

	printf("Copy %s to 0x%p... ", fn, addr);
	h = yaffs_open(fn, O_RDWR, 0);
	if (h < 0) {
		printf("File not found\n");
		return;
	}

	yaffs_read(h, addr, (int)s.st_size);
	printf("\t[DONE]\n");

	yaffs_close(h);
}


void cmd_yaffs_mwrite_file(char *fn, char *addr, int size)
{
	int outh;

	outh = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (outh < 0)
		printf("Error opening file: %d, %s\n", outh, yaffs_error_str());

	yaffs_write(outh, addr, size);

	yaffs_close(outh);
}


void cmd_yaffs_ls(const char *mountpt, int longlist)
{
	int i;
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat stat;
	char tempstr[255];

	d = yaffs_opendir(mountpt);

	if (!d) {
		printf("opendir failed, %s\n", yaffs_error_str());
		return;
	}

	for (i = 0; (de = yaffs_readdir(d)) != NULL; i++) {
		if (longlist) {
			sprintf(tempstr, "%s/%s", mountpt, de->d_name);
			yaffs_lstat(tempstr, &stat);
			printf("%-25s\t%7ld",
					de->d_name,
					(long)stat.st_size);
			printf(" %5d %s\n",
					stat.st_ino,
					yaffs_file_type_str(&stat));
		} else {
			printf("%s\n", de->d_name);
		}
	}

	yaffs_closedir(d);
}


void cmd_yaffs_mkdir(const char *dir)
{
	int retval = yaffs_mkdir(dir, 0);

	if (retval < 0)
		printf("yaffs_mkdir returning error: %d, %s\n",
			retval, yaffs_error_str());
}

void cmd_yaffs_rmdir(const char *dir)
{
	int retval = yaffs_rmdir(dir);

	if (retval < 0)
		printf("yaffs_rmdir returning error: %d, %s\n",
			retval, yaffs_error_str());
}

void cmd_yaffs_rm(const char *path)
{
	int retval = yaffs_unlink(path);

	if (retval < 0)
		printf("yaffs_unlink returning error: %d, %s\n",
			retval, yaffs_error_str());
}

void cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	int retval = yaffs_rename(newPath, oldPath);

	if (retval < 0)
		printf("yaffs_unlink returning error: %d, %s\n",
			retval, yaffs_error_str());
}
