/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor
 */

#ifndef __LS1043A_COMMON_H
#define __LS1043A_COMMON_H

/* SPL build */
#ifdef CONFIG_SPL_BUILD
#define SPL_NO_FMAN
#define SPL_NO_DSPI
#define SPL_NO_PCIE
#define SPL_NO_ENV
#define SPL_NO_MISC
#define SPL_NO_USB
#define SPL_NO_SATA
#define SPL_NO_QE
#define SPL_NO_EEPROM
#endif
#if (defined(CONFIG_SPL_BUILD) && defined(CONFIG_NAND_BOOT))
#define SPL_NO_MMC
#endif
#if (defined(CONFIG_SPL_BUILD) && defined(CONFIG_SD_BOOT_QSPI))
#define SPL_NO_IFC
#endif

#define CONFIG_REMAKE_ELF
#define CONFIG_GICV2

#include <asm/arch/stream_id_lsch2.h>
#include <asm/arch/config.h>

/* Link Definitions */
#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_FSL_OCRAM_BASE + 0xfff0)
#endif

#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE      0x880000000ULL

#define CPU_RELEASE_ADDR               secondary_boot_func

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		25000000	/* 25MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1024 * 1024)

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK          (get_serial_clock())

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* SD boot SPL */
#ifdef CONFIG_SD_BOOT

#define CONFIG_SPL_MAX_SIZE		0x17000
#define CONFIG_SPL_STACK		0x1001e000
#define CONFIG_SPL_PAD_TO		0x1d000

#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000
#define CONFIG_SPL_BSS_START_ADDR	0x8f000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_U_BOOT_HDR_SIZE				(16 << 10)
/*
 * HDR would be appended at end of image and copied to DDR along
 * with U-Boot image. Here u-boot max. size is 512K. So if binary
 * size increases then increase this size in case of secure boot as
 * it uses raw u-boot image instead of fit image.
 */
#define CONFIG_SYS_MONITOR_LEN		(0x100000 + CONFIG_U_BOOT_HDR_SIZE)
#else
#define CONFIG_SYS_MONITOR_LEN		0x100000
#endif /* ifdef CONFIG_SECURE_BOOT */
#endif

/* NAND SPL */
#ifdef CONFIG_NAND_BOOT
#define CONFIG_SPL_PBL_PAD
#define CONFIG_SPL_MAX_SIZE		0x1a000
#define CONFIG_SPL_STACK		0x1001d000
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_SPL_MALLOC_START	0x80200000
#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_U_BOOT_HDR_SIZE				(16 << 10)
#endif /* ifdef CONFIG_SECURE_BOOT */

#ifdef CONFIG_U_BOOT_HDR_SIZE
/*
 * HDR would be appended at end of image and copied to DDR along
 * with U-Boot image. Here u-boot max. size is 512K. So if binary
 * size increases then increase this size in case of secure boot as
 * it uses raw u-boot image instead of fit image.
 */
#define CONFIG_SYS_MONITOR_LEN		(0x100000 + CONFIG_U_BOOT_HDR_SIZE)
#else
#define CONFIG_SYS_MONITOR_LEN		0x100000
#endif /* ifdef CONFIG_U_BOOT_HDR_SIZE */

#endif

/* IFC */
#ifndef SPL_NO_IFC
#if defined(CONFIG_TFABOOT) || \
	(!defined(CONFIG_QSPI_BOOT) && !defined(CONFIG_SD_BOOT_QSPI))
#define CONFIG_FSL_IFC
/*
 * CONFIG_SYS_FLASH_BASE has the final address (core view)
 * CONFIG_SYS_FLASH_BASE_PHYS has the final address (IFC view)
 * CONFIG_SYS_FLASH_BASE_PHYS_EARLY has the temporary IFC address
 * CONFIG_SYS_TEXT_BASE is linked to 0x60000000 for booting
 */
#define CONFIG_SYS_FLASH_BASE			0x60000000
#define CONFIG_SYS_FLASH_BASE_PHYS		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_FLASH_BASE_PHYS_EARLY	0x00000000

#ifdef CONFIG_MTD_NOR_FLASH
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45	/* count down from 45/5: 9..1 */
#endif
#endif
#endif

/* I2C */
#define CONFIG_SYS_I2C

/* PCIe */
#ifndef SPL_NO_PCIE
#define CONFIG_PCIE1		/* PCIE controller 1 */
#define CONFIG_PCIE2		/* PCIE controller 2 */
#define CONFIG_PCIE3		/* PCIE controller 3 */

#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif
#endif

/* Command line configuration */

/*  MMC  */
#ifndef SPL_NO_MMC
#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif
#endif

/*  DSPI  */
#ifndef SPL_NO_DSPI
#define CONFIG_FSL_DSPI
#ifdef CONFIG_FSL_DSPI
#define CONFIG_DM_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO	/* cs0 */
#define CONFIG_SPI_FLASH_SST		/* cs1 */
#define CONFIG_SPI_FLASH_EON		/* cs2 */
#endif
#endif

/* FMan ucode */
#ifndef SPL_NO_FMAN
#define CONFIG_SYS_DPAA_FMAN
#ifdef CONFIG_SYS_DPAA_FMAN
#define CONFIG_SYS_FM_MURAM_SIZE	0x60000

#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_FMAN_FW_ADDR		0x900000
#define CONFIG_SYS_QE_FW_ADDR		0x940000


#else
#ifdef CONFIG_NAND_BOOT
/* Store Fman ucode at offeset 0x900000(72 blocks). */
#define CONFIG_SYS_FMAN_FW_ADDR		(72 * CONFIG_SYS_NAND_BLOCK_SIZE)
#elif defined(CONFIG_SD_BOOT)
/*
 * PBL SD boot image should stored at 0x1000(8 blocks), the size of the image is
 * about 1MB (2040 blocks), Env is stored after the image, and the env size is
 * 0x2000 (16 blocks), 8 + 2040 + 16 = 2064, enlarge it to 18432(0x4800).
 */
#define CONFIG_SYS_FMAN_FW_ADDR		(512 * 0x4800)
#define CONFIG_SYS_QE_FW_ADDR		(512 * 0x4A00)
#elif defined(CONFIG_QSPI_BOOT)
#define CONFIG_SYS_FMAN_FW_ADDR		0x40900000
#else
/* FMan fireware Pre-load address */
#define CONFIG_SYS_FMAN_FW_ADDR		0x60900000
#define CONFIG_SYS_QE_FW_ADDR		0x60940000
#endif
#endif
#define CONFIG_SYS_QE_FMAN_FW_LENGTH	0x10000
#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)
#endif
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000)

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

#ifndef SPL_NO_MISC
#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"fdt_high=0xffffffffffffffff\0"		\
	"initrd_high=0xffffffffffffffff\0"	\
	"fdt_addr=0x64f00000\0"		 	\
	"kernel_addr=0x61000000\0"		\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"kernel_start=0x1000000\0"		\
	"kernelheader_start=0x800000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernelheader_addr=0x60800000\0"	\
	"kernel_size=0x2800000\0"		\
	"kernelheader_size=0x40000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernel_size_sd=0x14000\0"		\
	"kernelhdr_addr_sd=0x4000\0"		\
	"kernelhdr_size_sd=0x10\0"		\
	"console=ttyS0,115200\0"		\
	"boot_os=y\0"				\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"	\
	BOOTENV					\
	"boot_scripts=ls1043ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1043ardb_bs.out\0"	\
	"scan_dev_for_boot_part="		\
		"part list ${devtype} ${devnum} devplist; "	\
		"env exists devplist || setenv devplist 1; "	\
		"for distro_bootpart in ${devplist}; do "	\
			"if fstype ${devtype} "			\
				"${devnum}:${distro_bootpart} "	\
				"bootfstype; then "		\
				"run scan_dev_for_boot; "	\
			"fi; "					\
		"done\0"			\
	"boot_a_script="					\
		"load ${devtype} ${devnum}:${distro_bootpart} "	\
			"${scriptaddr} ${prefix}${script}; "	\
		"env exists secureboot && load ${devtype} "	\
			"${devnum}:${distro_bootpart} "		\
			"${scripthdraddr} ${prefix}${boot_script_hdr}; " \
			"env exists secureboot "	\
			"&& esbc_validate ${scripthdraddr};"	\
		"source ${scriptaddr}\0"			\
	"qspi_bootcmd=echo Trying load from qspi..;"	\
		"sf probe && sf read $load_addr "	\
		"$kernel_addr $kernel_size; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"	\
	"nor_bootcmd=echo Trying load from nor..;"	\
		"cp.b $kernel_addr $load_addr "	\
		"$kernel_size; env exists secureboot "	\
		"&& cp.b $kernelheader_addr $kernelheader_addr_r "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"	    \
	"nand_bootcmd=echo Trying load from NAND..;"	\
		"nand info; nand read $load_addr "	\
		"$kernel_start $kernel_size; env exists secureboot "	\
		"&& nand read $kernelheader_addr_r $kernelheader_start "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"	\
	"sd_bootcmd=echo Trying load from SD ..;"       \
		"mmcinfo; mmc read $load_addr "         \
		"$kernel_addr_sd $kernel_size_sd && "     \
		"env exists secureboot && mmc read $kernelheader_addr_r "		\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "		\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$board\0"


#undef CONFIG_BOOTCOMMAND
#ifdef CONFIG_TFABOOT
#define QSPI_NOR_BOOTCOMMAND "run distro_bootcmd; run qspi_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#define SD_BOOTCOMMAND "run distro_bootcmd; run sd_bootcmd; "  \
			   "env exists secureboot && esbc_halt;"
#define IFC_NOR_BOOTCOMMAND "run distro_bootcmd; run nor_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#define IFC_NAND_BOOTCOMMAND "run distro_bootcmd; run nand_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#else
#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run qspi_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run sd_bootcmd; "  \
			   "env exists secureboot && esbc_halt;"
#else
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run nor_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#endif
#endif
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */

#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

#include <asm/arch/soc.h>

#endif /* __LS1043A_COMMON_H */
