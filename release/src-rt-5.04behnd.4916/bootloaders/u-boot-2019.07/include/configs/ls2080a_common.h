/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright (C) 2014 Freescale Semiconductor
 */

#ifndef __LS2_COMMON_H
#define __LS2_COMMON_H

#define CONFIG_REMAKE_ELF
#define CONFIG_GICV3

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>

/* Link Definitions */
#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_FSL_OCRAM_BASE + 0xfff0)
#endif

/* We need architecture specific misc initializations */

/* Link Definitions */
#ifndef CONFIG_TFABOOT
#ifndef CONFIG_QSPI_BOOT
#else
#define CONFIG_ENV_SIZE			0x2000          /* 8KB */
#define CONFIG_ENV_OFFSET		0x300000        /* 3MB */
#define CONFIG_ENV_SECT_SIZE		0x40000
#endif
#endif

#define CONFIG_SKIP_LOWLEVEL_INIT

#ifndef CONFIG_SYS_FSL_DDR4
#define CONFIG_SYS_DDR_RAW_TIMING
#endif

#define CONFIG_SYS_FSL_DDR_INTLV_256B	/* force 256 byte interleaving */

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE	0x8080000000ULL
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	2

/*
 * SMP Definitinos
 */
#define CPU_RELEASE_ADDR		secondary_boot_func

#define CONFIG_SYS_FSL_OTHER_DDR_NUM_CTRLS
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
#define CONFIG_SYS_DP_DDR_BASE		0x6000000000ULL
/*
 * DDR controller use 0 as the base address for binding.
 * It is mapped to CONFIG_SYS_DP_DDR_BASE for core to access.
 */
#define CONFIG_SYS_DP_DDR_BASE_PHY	0
#define CONFIG_DP_DDR_CTRL		2
#define CONFIG_DP_DDR_NUM_CTRLS		1
#endif

/* Generic Timer Definitions */
/*
 * This is not an accurate number. It is used in start.S. The frequency
 * will be udpated later when get_bus_freq(0) is available.
 */
#define COUNTER_FREQUENCY		25000000	/* 25MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2048 * 1024)

/* I2C */
#define CONFIG_SYS_I2C

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     1
#define CONFIG_SYS_NS16550_CLK          (get_serial_clock())

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* IFC */
#define CONFIG_FSL_IFC

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
 * CONFIG_SYS_FLASH_BASE has the final address (core view)
 * CONFIG_SYS_FLASH_BASE_PHYS has the final address (IFC view)
 * CONFIG_SYS_FLASH_BASE_PHYS_EARLY has the temporary IFC address
 * CONFIG_SYS_TEXT_BASE is linked to 0x30000000 for booting
 */

#define CONFIG_SYS_FLASH_BASE			0x580000000ULL
#define CONFIG_SYS_FLASH_BASE_PHYS		0x80000000
#define CONFIG_SYS_FLASH_BASE_PHYS_EARLY	0x00000000

#define CONFIG_SYS_FLASH1_BASE_PHYS		0xC0000000
#define CONFIG_SYS_FLASH1_BASE_PHYS_EARLY	0x8000000

#ifndef __ASSEMBLY__
unsigned long long get_qixis_addr(void);
#endif
#define QIXIS_BASE				get_qixis_addr()
#define QIXIS_BASE_PHYS				0x20000000
#define QIXIS_BASE_PHYS_EARLY			0xC000000
#define QIXIS_STAT_PRES1			0xb
#define QIXIS_SDID_MASK				0x07
#define QIXIS_ESDHC_NO_ADAPTER			0x7

#define CONFIG_SYS_NAND_BASE			0x530000000ULL
#define CONFIG_SYS_NAND_BASE_PHYS		0x30000000

/* MC firmware */
/* TODO Actual DPL max length needs to be confirmed with the MC FW team */
#define CONFIG_SYS_LS_MC_DPC_MAX_LENGTH	    0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET    0x00F00000
#define CONFIG_SYS_LS_MC_DPL_MAX_LENGTH	    0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET    0x00F20000
/* For LS2085A */
#define CONFIG_SYS_LS_MC_AIOP_IMG_MAX_LENGTH	0x200000
#define CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET	0x07000000

/* Define phy_reset function to boot the MC based on mcinitcmd.
 * This happens late enough to properly fixup u-boot env MAC addresses.
 */
#define CONFIG_RESET_PHY_R

/*
 * Carve out a DDR region which will not be used by u-boot/Linux
 *
 * It will be used by MC and Debug Server. The MC region must be
 * 512MB aligned, so the min size to hide is 512MB.
 */
#ifdef CONFIG_FSL_MC_ENET
#define CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE		(128UL * 1024 * 1024)
#endif

/* Command line configuration */

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000)

/* Physical Memory Map */
/* fixme: these need to be checked against the board */
#define CONFIG_CHIP_SELECTS_PER_CTRL	4

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
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

#ifndef CONFIG_TFABOOT
#ifdef CONFIG_SD_BOOT
#define CONFIG_BOOTCOMMAND	"mmc read 0x80200000 0x6800 0x800;"\
				" fsl_mc apply dpl 0x80200000 &&" \
				" mmc read $kernel_load $kernel_start" \
				" $kernel_size && bootm $kernel_load"
#else
#define CONFIG_BOOTCOMMAND	"fsl_mc apply dpl 0x580d00000 &&" \
				" cp.b $kernel_start $kernel_load" \
				" $kernel_size && bootm $kernel_load"
#endif
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000
#define CONFIG_SPL_MAX_SIZE		0x16000
#define CONFIG_SPL_STACK		(CONFIG_SYS_FSL_OCRAM_BASE + 0x9ff0)
#define CONFIG_SPL_TARGET		"u-boot-with-spl.bin"

#ifdef CONFIG_NAND_BOOT
#define CONFIG_SYS_NAND_U_BOOT_DST	0x80400000
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
#endif
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00100000
#define CONFIG_SYS_SPL_MALLOC_START	0x80200000
#define CONFIG_SYS_MONITOR_LEN		(1024 * 1024)

#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

#include <asm/arch/soc.h>

#endif /* __LS2_COMMON_H */
