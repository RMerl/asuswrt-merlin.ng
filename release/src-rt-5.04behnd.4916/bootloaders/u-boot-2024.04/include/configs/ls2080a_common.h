/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright (C) 2014 Freescale Semiconductor
 */

#ifndef __LS2_COMMON_H
#define __LS2_COMMON_H

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>

/* Link Definitions */

/* We need architecture specific misc initializations */

/* Link Definitions */

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CFG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#define CFG_SYS_DDR_BLOCK2_BASE	0x8080000000ULL

/*
 * SMP Definitinos
 */
#define CPU_RELEASE_ADDR		secondary_boot_addr

/*
 * This is not an accurate number. It is used in start.S. The frequency
 * will be udpated later when get_bus_freq(0) is available.
 */

/* GPIO */

/* I2C */

/* Serial Port */
#define CFG_SYS_NS16550_CLK          (get_serial_clock())

/*
 * During booting, IFC is mapped at the region of 0x30000000.
 * But this region is limited to 256MB. To accommodate NOR, promjet
 * and FPGA. This region is divided as below:
 * 0x30000000 - 0x37ffffff : 128MB : NOR flash
 * 0x38000000 - 0x3BFFFFFF : 64MB  : Promjet
 * 0x3C000000 - 0x40000000 : 64MB  : FPGA etc
 *
 * To accommodate bigger NOR flash and other devices, we will map IFC
 * chip selects to as below:
 * 0x5_1000_0000..0x5_1fff_ffff	Memory Hole
 * 0x5_2000_0000..0x5_3fff_ffff	IFC CSx (FPGA, NAND and others 512MB)
 * 0x5_4000_0000..0x5_7fff_ffff	ASIC or others 1GB
 * 0x5_8000_0000..0x5_bfff_ffff	IFC CS0 1GB (NOR/Promjet)
 * 0x5_C000_0000..0x5_ffff_ffff	IFC CS1 1GB (NOR/Promjet)
 *
 * For e.g. NOR flash at CS0 will be mapped to 0x580000000 after relocation.
 * CFG_SYS_FLASH_BASE has the final address (core view)
 * CFG_SYS_FLASH_BASE_PHYS has the final address (IFC view)
 * CFG_SYS_FLASH_BASE_PHYS_EARLY has the temporary IFC address
 * CONFIG_TEXT_BASE is linked to 0x30000000 for booting
 */

#define CFG_SYS_FLASH_BASE			0x580000000ULL
#define CFG_SYS_FLASH_BASE_PHYS		0x80000000
#define CFG_SYS_FLASH_BASE_PHYS_EARLY	0x00000000

#define CFG_SYS_FLASH1_BASE_PHYS		0xC0000000
#define CFG_SYS_FLASH1_BASE_PHYS_EARLY	0x8000000

#ifndef __ASSEMBLY__
unsigned long long get_qixis_addr(void);
#endif
#define QIXIS_BASE				get_qixis_addr()
#define QIXIS_BASE_PHYS				0x20000000
#define QIXIS_BASE_PHYS_EARLY			0xC000000
#define QIXIS_STAT_PRES1			0xb
#define QIXIS_SDID_MASK				0x07
#define QIXIS_ESDHC_NO_ADAPTER			0x7

#define CFG_SYS_NAND_BASE			0x530000000ULL
#define CFG_SYS_NAND_BASE_PHYS		0x30000000

/* MC firmware */
/* TODO Actual DPL max length needs to be confirmed with the MC FW team */
#define CFG_SYS_LS_MC_DPC_MAX_LENGTH	    0x20000
#define CFG_SYS_LS_MC_DRAM_DPC_OFFSET    0x00F00000
#define CFG_SYS_LS_MC_DPL_MAX_LENGTH	    0x20000
#define CFG_SYS_LS_MC_DRAM_DPL_OFFSET    0x00F20000
/* For LS2085A */
#define CFG_SYS_LS_MC_AIOP_IMG_MAX_LENGTH	0x200000
#define CFG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET	0x07000000

/*
 * Carve out a DDR region which will not be used by u-boot/Linux
 *
 * It will be used by MC and Debug Server. The MC region must be
 * 512MB aligned, so the min size to hide is 512MB.
 */
#ifdef CONFIG_FSL_MC_ENET
#define CFG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE		(128UL * 1024 * 1024)
#endif

/* Miscellaneous configurable options */

/* Physical Memory Map */
/* fixme: these need to be checked against the board */

#define HWCONFIG_BUFFER_SIZE		128

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x581000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"console=ttyAMA0,38400n8\0"		\
	"mcinitcmd=fsl_mc start mc 0x580a00000"	\
	" 0x580e00000 \0"

#ifdef CONFIG_NAND_BOOT
#define CFG_SYS_NAND_U_BOOT_DST	0x80400000
#define CFG_SYS_NAND_U_BOOT_START	CFG_SYS_NAND_U_BOOT_DST
#endif

#include <asm/arch/soc.h>

#endif /* __LS2_COMMON_H */
