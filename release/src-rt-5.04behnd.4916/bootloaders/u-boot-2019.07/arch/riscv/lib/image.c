// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Western Digital Corporation or its affiliates.
 * Authors:
 *	Atish Patra <atish.patra@wdc.com>
 * Based on arm/lib/image.c
 */

#include <common.h>
#include <mapmem.h>
#include <errno.h>
#include <linux/sizes.h>
#include <linux/stddef.h>

DECLARE_GLOBAL_DATA_PTR;

/* ASCII version of "RISCV" defined in Linux kernel */
#define LINUX_RISCV_IMAGE_MAGIC 0x5643534952

struct linux_image_h {
	uint32_t	code0;		/* Executable code */
	uint32_t	code1;		/* Executable code */
	uint64_t	text_offset;	/* Image load offset */
	uint64_t	image_size;	/* Effective Image size */
	uint64_t	res1;		/* reserved */
	uint64_t	res2;		/* reserved */
	uint64_t	res3;		/* reserved */
	uint64_t	magic;		/* Magic number */
	uint32_t	res4;		/* reserved */
	uint32_t	res5;		/* reserved */
};

int booti_setup(ulong image, ulong *relocated_addr, ulong *size,
		bool force_reloc)
{
	struct linux_image_h *lhdr;

	lhdr = (struct linux_image_h *)map_sysmem(image, 0);

	if (lhdr->magic != LINUX_RISCV_IMAGE_MAGIC) {
		puts("Bad Linux RISCV Image magic!\n");
		return -EINVAL;
	}

	if (lhdr->image_size == 0) {
		puts("Image lacks image_size field, error!\n");
		return -EINVAL;
	}
	*size = lhdr->image_size;
	*relocated_addr = gd->ram_base + lhdr->text_offset;

	unmap_sysmem(lhdr);

	return 0;
}
