/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2018 NXP
 */

#ifndef __LS1088_COMMON_H
#define __LS1088_COMMON_H

/* SPL build */
#ifdef CONFIG_SPL_BUILD
#define SPL_NO_BOARDINFO
#define SPL_NO_QIXIS
#define SPL_NO_PCI
#define SPL_NO_ENV
#define SPL_NO_RTC
#define SPL_NO_USB
#define SPL_NO_SATA
#define SPL_NO_QSPI
#define SPL_NO_IFC
#endif

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>
#include <asm/arch/soc.h>

#define LS1088ARDB_PB_BOARD            0x4A
/* Link Definitions */

/* Link Definitions */
#define CFG_SYS_FSL_QSPI_BASE	0x20000000

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CFG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#define CFG_SYS_DDR_BLOCK2_BASE	0x8080000000ULL
/*
 * SMP Definitinos
 */
#define CPU_RELEASE_ADDR		secondary_boot_addr

/* GPIO */

/* I2C */


/* Serial Port */
#define CFG_SYS_NS16550_CLK          (get_bus_freq(0) / 2)

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


#define CFG_SYS_NAND_BASE			0x530000000ULL
#define CFG_SYS_NAND_BASE_PHYS		0x30000000


/* MC firmware */
/* TODO Actual DPL max length needs to be confirmed with the MC FW team */
#define CFG_SYS_LS_MC_DPC_MAX_LENGTH	    0x20000
#define CFG_SYS_LS_MC_DRAM_DPC_OFFSET    0x00F00000
#define CFG_SYS_LS_MC_DPL_MAX_LENGTH	    0x20000
#define CFG_SYS_LS_MC_DRAM_DPL_OFFSET    0x00F20000
#define CFG_SYS_LS_MC_AIOP_IMG_MAX_LENGTH	0x200000
#define CFG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET	0x07000000

/*
 * Carve out a DDR region which will not be used by u-boot/Linux
 *
 * It will be used by MC and Debug Server. The MC region must be
 * 512MB aligned, so the min size to hide is 512MB.
 */

#if defined(CONFIG_FSL_MC_ENET)
#define CFG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE		(128UL * 1024 * 1024)
#endif

/* Miscellaneous configurable options */

/* Physical Memory Map */

#define HWCONFIG_BUFFER_SIZE		128

#ifndef SPL_NO_ENV
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
#endif

#endif /* __LS1088_COMMON_H */
