/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Niklaus Giger, niklaus.giger@member.fsf.org
 */

#ifndef _VXWORKS_H_
#define _VXWORKS_H_

#include <efi_api.h>

/*
 * Physical address of memory base for VxWorks x86
 * This is LOCAL_MEM_LOCAL_ADRS in the VxWorks kernel configuration.
 */
#define VXWORKS_PHYS_MEM_BASE	0x100000

/* x86 bootline offset relative to LOCAL_MEM_LOCAL_ADRS in VxWorks */
#define X86_BOOT_LINE_OFFSET	0x1200

/*
 * VxWorks x86 E820 related stuff
 *
 * VxWorks on x86 gets E820 information from pre-defined offset @
 * 0x4a00 and 0x4000. At 0x4a00 it's an information table defined
 * by VxWorks and the actual E820 table entries starts from 0x4000.
 * As defined by the BIOS E820 spec, the maximum number of E820 table
 * entries is 128 and each entry occupies 20 bytes, so it's 128 * 20
 * = 2560 (0xa00) bytes in total. That's where VxWorks stores some
 * information that is retrieved from the BIOS E820 call and saved
 * later for sanity test during the kernel boot-up.
 */
#define E820_DATA_OFFSET	0x4000
#define E820_INFO_OFFSET	0x4a00

/* E820 info signatiure "SMAP" - System MAP */
#define E820_SIGNATURE	0x534d4150

struct e820_info {
	u32 sign;	/* "SMAP" signature */
	u32 x0;		/* don't care, used by VxWorks */
	u32 x1;		/* don't care, used by VxWorks */
	u32 x2;		/* don't care, used by VxWorks */
	u32 addr;	/* last e820 table entry addr */
	u32 x3;		/* don't care, used by VxWorks */
	u32 entries;	/* e820 table entry count */
	u32 error;	/* must be zero */
};

/*
 * VxWorks bootloader stores its size at a pre-defined offset @ 0x5004.
 * Later when VxWorks kernel boots up and system memory information is
 * retrieved from the E820 table, the bootloader size will be subtracted
 * from the total system memory size to calculate the size of available
 * memory for the OS.
 */
#define BOOT_IMAGE_SIZE_OFFSET	0x5004

/*
 * When booting from EFI BIOS, VxWorks bootloader stores the EFI GOP
 * framebuffer info at a pre-defined offset @ 0x6100. When VxWorks kernel
 * boots up, its EFI console driver tries to find such a block and if
 * the signature matches, the framebuffer information will be used to
 * initialize the driver.
 *
 * However it is not necessary to prepare an EFI environment for VxWorks's
 * EFI console driver to function (eg: EFI loader in U-Boot). If U-Boot has
 * already initialized the graphics card and set it to a VESA mode that is
 * compatible with EFI GOP, we can simply prepare such a block for VxWorks.
 */
#define EFI_GOP_INFO_OFFSET	0x6100

/* EFI GOP info signatiure */
#define EFI_GOP_INFO_MAGIC	0xfeedface

struct efi_gop_info {
	u32 magic;			/* signature */
	struct efi_gop_mode_info info;	/* EFI GOP mode info structure */
	phys_addr_t fb_base;		/* framebuffer base address */
	u32 fb_size;			/* framebuffer size */
};

int do_bootvx(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
void boot_prep_vxworks(bootm_headers_t *images);
void boot_jump_vxworks(bootm_headers_t *images);

#endif
