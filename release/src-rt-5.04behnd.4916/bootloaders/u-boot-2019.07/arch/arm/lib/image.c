// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <mapmem.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define LINUX_ARM64_IMAGE_MAGIC 0x644d5241

/* See Documentation/arm64/booting.txt in the Linux kernel */
struct Image_header {
	uint32_t	code0;		/* Executable code */
	uint32_t	code1;		/* Executable code */
	uint64_t	text_offset;	/* Image load offset, LE */
	uint64_t	image_size;	/* Effective Image size, LE */
	uint64_t	flags;		/* Kernel flags, LE */
	uint64_t	res2;		/* reserved */
	uint64_t	res3;		/* reserved */
	uint64_t	res4;		/* reserved */
	uint32_t	magic;		/* Magic number */
	uint32_t	res5;
};

int booti_setup(ulong image, ulong *relocated_addr, ulong *size,
		bool force_reloc)
{
	struct Image_header *ih;
	uint64_t dst;
	uint64_t image_size, text_offset;

	*relocated_addr = image;

	ih = (struct Image_header *)map_sysmem(image, 0);

	if (ih->magic != le32_to_cpu(LINUX_ARM64_IMAGE_MAGIC)) {
		puts("Bad Linux ARM64 Image magic!\n");
		return 1;
	}

	/*
	 * Prior to Linux commit a2c1d73b94ed, the text_offset field
	 * is of unknown endianness.  In these cases, the image_size
	 * field is zero, and we can assume a fixed value of 0x80000.
	 */
	if (ih->image_size == 0) {
		puts("Image lacks image_size field, assuming 16MiB\n");
		image_size = 16 << 20;
		text_offset = 0x80000;
	} else {
		image_size = le64_to_cpu(ih->image_size);
		text_offset = le64_to_cpu(ih->text_offset);
	}

	*size = image_size;

	/*
	 * If bit 3 of the flags field is set, the 2MB aligned base of the
	 * kernel image can be anywhere in physical memory, so respect
	 * images->ep.  Otherwise, relocate the image to the base of RAM
	 * since memory below it is not accessible via the linear mapping.
	 */
	if (!force_reloc && (le64_to_cpu(ih->flags) & BIT(3)))
		dst = image - text_offset;
	else
		dst = gd->bd->bi_dram[0].start;

	*relocated_addr = ALIGN(dst, SZ_2M) + text_offset;

	unmap_sysmem(ih);

	return 0;
}
