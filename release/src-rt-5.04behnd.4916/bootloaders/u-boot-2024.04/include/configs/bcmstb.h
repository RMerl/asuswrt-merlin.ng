/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 *
 * Configuration settings for the Broadcom BCMSTB SoC family.
 */

#ifndef __BCMSTB_H
#define __BCMSTB_H

#include <linux/sizes.h>

#ifndef __ASSEMBLY__

#include <linux/stringify.h>
#include <linux/types.h>

struct bcmstb_boot_parameters {
	u32 r0;
	u32 r1;
	u32 r2;
	u32 r3;
	u32 sp;
	u32 lr;
};

extern struct bcmstb_boot_parameters bcmstb_boot_parameters;

extern phys_addr_t prior_stage_fdt_address;

#endif /* __ASSEMBLY__ */

/*
 * CPU configuration.
 */

/*
 * Memory configuration.
 *
 * The prior stage BOLT bootloader sets up memory for us.
 *
 * An example boot memory layout after loading everything is:
 *
 *	 0x0000 8000	vmlinux.bin.gz
 *	       :	[~31 MiB uncompressed max]
 *	 0x01ef f000	FIT containing signed public key
 *	       :	[~2 KiB in size]
 *	 0x01f0 0000	DTB copied from prior-stage-provided region
 *	       :	[~1 MiB max]
 *	 0x0200 0000	FIT containing ramdisk and device tree
 *             :	  initramfs.cpio.gz
 *	       :	  [~208 MiB uncompressed max, to CMA/bmem low address]
 *	       :	  [~80 MiB compressed max, to PSB low address]
 *             :	  device tree binary
 *             :	  [~60 KiB]
 *	 0x0700 0000	Prior stage bootloader (PSB)
 *	       :
 *	 0x0761 7000	Prior-stage-provided device tree binary (DTB)
 *	       :	[~40 KiB in size]
 *	 0x0f00 0000	Contiguous memory allocator (CMA/bmem) low address
 *	       :
 *	 0x8010 0000	U-Boot code at ELF load address
 *	       :	[~500 KiB in size, stripped]
 *	 0xc000 0000	Top of RAM
 *
 * Setting gd->relocaddr to CONFIG_TEXT_BASE in dram_init_banksize
 * prevents U-Boot from relocating itself when it is run as an ELF
 * program by the prior stage bootloader.
 *
 * We want to keep the ramdisk and FDT in the FIT image in-place, to
 * accommodate stblinux's bmem and CMA regions.  To accomplish this,
 * we set initrd_high and fdt_high to 0xffffffff, and the load and
 * entry addresses of the FIT ramdisk entry to 0x0.
 *
 * Overwriting the prior stage bootloader causes memory instability,
 * so the compressed initramfs needs to fit between the load address
 * and the PSB low address.  In BOLT's default configuration this
 * limits the compressed size of the initramfs to approximately 80
 * MiB.  However, BOLT can be configured to allow loading larger
 * initramfs images, in which case this limitation is eliminated.
 */
#define CFG_SYS_SDRAM_BASE		0x00000000
#define CFG_SYS_INIT_RAM_SIZE	0x100000

/*
 * CONFIG_SYS_LOAD_ADDR - 1 MiB.
 */

/*
 * Large kernel image bootm configuration.
 */

/*
 * Informational display configuration.
 */

/*
 * Command configuration.
 */

/*
 * Flash configuration.
 */

/*
 * Filesystem configuration.
 */

/*
 * Environment configuration.
 */

/*
 * Save the prior stage provided DTB.
 */
/*
 * Enable in-place RFS with this initrd_high setting.
 */
#define CFG_EXTRA_ENV_SETTINGS					\
	"fdtsaveaddr=" __stringify(CONFIG_SYS_FDT_SAVE_ADDRESS) "\0"	\
	"initrd_high=0xffffffff\0"					\
	"fdt_high=0xffffffff\0"

/*
 * Set fdtaddr to prior stage-provided DTB in board_late_init, when
 * writeable environment is available.
 */

#endif /* __BCMSTB_H */
