# SPDX-License-Identifier: GPL-2.0
# Makefile for YAFFS direct test
# YAFFS: Yet another Flash File System. A NAND-flash specific file system.
#
# Copyright (C) 2003 Aleph One Ltd.
#
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
#

obj-y := \
	yaffs_allocator.o yaffs_attribs.o yaffs_bitmap.o yaffs_uboot_glue.o\
	yaffs_checkptrw.o yaffs_ecc.o yaffs_error.o \
	yaffsfs.o yaffs_guts.o yaffs_nameval.o yaffs_nand.o\
	yaffs_packedtags1.o yaffs_packedtags2.o yaffs_qsort.o \
	yaffs_summary.o yaffs_tagscompat.o yaffs_verify.o yaffs_yaffs1.o \
	yaffs_yaffs2.o yaffs_mtdif.o yaffs_mtdif2.o

ccflags-y = -DCONFIG_YAFFS_DIRECT -DCONFIG_YAFFS_SHORT_NAMES_IN_RAM \
		-DCONFIG_YAFFS_YAFFS2 -DNO_Y_INLINE \
		-DCONFIG_YAFFS_PROVIDE_DEFS -DCONFIG_YAFFSFS_PROVIDE_VALUES
