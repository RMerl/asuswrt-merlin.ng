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

#include "version.h"
#include <linux/sizes.h>

#ifndef __ASSEMBLY__

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
#define CONFIG_SKIP_LOWLEVEL_INIT

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
 * Setting gd->relocaddr to CONFIG_SYS_TEXT_BASE in dram_init_banksize
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
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR +	\
					 CONFIG_SYS_INIT_RAM_SIZE -	\
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_LOAD_ADDR		0x2000000

/*
 * CONFIG_SYS_LOAD_ADDR - 1 MiB.
 */
#define CONFIG_SYS_FDT_SAVE_ADDRESS	0x1f00000
#define CONFIG_SYS_CBSIZE		512
#define CONFIG_SYS_MAXARGS		32

/*
 * Large kernel image bootm configuration.
 */
#define CONFIG_SYS_BOOTM_LEN		SZ_64M

/*
 * NS16550 configuration.
 */
#define V_NS16550_CLK			81000000

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * Serial console configuration.
 */
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600, \
					 115200}

/*
 * Informational display configuration.
 */
#define CONFIG_REVISION_TAG

/*
 * Command configuration.
 */
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_MMC

/*
 * Flash configuration.
 */
#define CONFIG_ST_SMI
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_MACRONIX

/*
 * Filesystem configuration.
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT4
#define CONFIG_FS_EXT4
#define CONFIG_CMD_FS_GENERIC

/*
 * Environment configuration.
 */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SIZE			(64 << 10) /* 64 KiB */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_OVERWRITE

/*
 * Save the prior stage provided DTB.
 */
#define CONFIG_PREBOOT					\
	"fdt addr ${fdtcontroladdr};"			\
	"fdt move ${fdtcontroladdr} ${fdtsaveaddr};"	\
	"fdt addr ${fdtsaveaddr};"
/*
 * Enable in-place RFS with this initrd_high setting.
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"fdtsaveaddr=" __stringify(CONFIG_SYS_FDT_SAVE_ADDRESS) "\0"	\
	"initrd_high=0xffffffff\0"					\
	"fdt_high=0xffffffff\0"

/*
 * Set fdtaddr to prior stage-provided DTB in board_late_init, when
 * writeable environment is available.
 */
#define CONFIG_BOARD_LATE_INIT

#endif /* __BCMSTB_H */
