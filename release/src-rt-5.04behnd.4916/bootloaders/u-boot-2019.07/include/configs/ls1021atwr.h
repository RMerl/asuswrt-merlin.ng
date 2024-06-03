/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARMV7_PSCI_1_0

#define CONFIG_ARMV7_SECURE_BASE	OCRAM_BASE_S_ADDR

#define CONFIG_SYS_FSL_CLK

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_DEEP_SLEEP

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 16 * 1024 * 1024)

#define CONFIG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000

#define DDR_SDRAM_CFG			0x470c0008
#define DDR_CS0_BNDS			0x008000bf
#define DDR_CS0_CONFIG			0x80014302
#define DDR_TIMING_CFG_0		0x50550004
#define DDR_TIMING_CFG_1		0xbcb38c56
#define DDR_TIMING_CFG_2		0x0040d120
#define DDR_TIMING_CFG_3		0x010e1000
#define DDR_TIMING_CFG_4		0x00000001
#define DDR_TIMING_CFG_5		0x03401400
#define DDR_SDRAM_CFG_2			0x00401010
#define DDR_SDRAM_MODE			0x00061c60
#define DDR_SDRAM_MODE_2		0x00180000
#define DDR_SDRAM_INTERVAL		0x18600618
#define DDR_DDR_WRLVL_CNTL		0x8655f605
#define DDR_DDR_WRLVL_CNTL_2		0x05060607
#define DDR_DDR_WRLVL_CNTL_3		0x05050505
#define DDR_DDR_CDR1			0x80040000
#define DDR_DDR_CDR2			0x00000001
#define DDR_SDRAM_CLK_CNTL		0x02000000
#define DDR_DDR_ZQ_CNTL			0x89080600
#define DDR_CS0_CONFIG_2		0
#define DDR_SDRAM_CFG_MEM_EN		0x80000000
#define SDRAM_CFG2_D_INIT		0x00000010
#define DDR_CDR2_VREF_TRAIN_EN		0x00000080
#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG_BI			0x00000001

#ifdef CONFIG_RAMBOOT_PBL
#define CONFIG_SYS_FSL_PBL_PBI	board/freescale/ls1021atwr/ls102xa_pbi.cfg
#endif

#ifdef CONFIG_SD_BOOT
#ifdef CONFIG_SD_BOOT_QSPI
#define CONFIG_SYS_FSL_PBL_RCW	\
	board/freescale/ls1021atwr/ls102xa_rcw_sd_qspi.cfg
#else
#define CONFIG_SYS_FSL_PBL_RCW	\
	board/freescale/ls1021atwr/ls102xa_rcw_sd_ifc.cfg
#endif

#ifdef CONFIG_SECURE_BOOT
/*
 * HDR would be appended at end of image and copied to DDR along
 * with U-Boot image.
 */
#define CONFIG_U_BOOT_HDR_SIZE				(16 << 10)
#endif /* ifdef CONFIG_SECURE_BOOT */

#define CONFIG_SPL_MAX_SIZE		0x1a000
#define CONFIG_SPL_STACK		0x1001d000
#define CONFIG_SPL_PAD_TO		0x1c000

#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_TEXT_BASE + \
		CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000
#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

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

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(1u * 1024 * 1024 * 1024)

#define CONFIG_SYS_DDR_SDRAM_BASE      0x80000000UL
#define CONFIG_SYS_SDRAM_BASE          CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_CHIP_SELECTS_PER_CTRL	4

/*
 * IFC Definitions
 */
#if !defined(CONFIG_QSPI_BOOT) && !defined(CONFIG_SD_BOOT_QSPI)
#define CONFIG_FSL_IFC
#define CONFIG_SYS_FLASH_BASE		0x60000000
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_NOR0_CSPR_EXT	(0x0)
#define CONFIG_SYS_NOR0_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CONFIG_SYS_NOR_AMASK		IFC_AMASK(128 * 1024 * 1024)

/* NOR Flash Timing Params */
#define CONFIG_SYS_NOR_CSOR		(CSOR_NOR_ADM_SHIFT(4) | \
					CSOR_NOR_TRHZ_80)
#define CONFIG_SYS_NOR_FTIM0		(FTIM0_NOR_TACSE(0x4) | \
					FTIM0_NOR_TEADC(0x5) | \
					FTIM0_NOR_TAVDS(0x0) | \
					FTIM0_NOR_TEAHC(0x5))
#define CONFIG_SYS_NOR_FTIM1		(FTIM1_NOR_TACO(0x35) | \
					FTIM1_NOR_TRAD_NOR(0x1A) | \
					FTIM1_NOR_TSEQRAD_NOR(0x13))
#define CONFIG_SYS_NOR_FTIM2		(FTIM2_NOR_TCS(0x4) | \
					FTIM2_NOR_TCH(0x4) | \
					FTIM2_NOR_TWP(0x1c) | \
					FTIM2_NOR_TWPH(0x0e))
#define CONFIG_SYS_NOR_FTIM3		0

#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45	/* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE_PHYS }

#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
#define CONFIG_SYS_WRITE_SWAPPED_DATA
#endif

/* CPLD */

#define CONFIG_SYS_CPLD_BASE	0x7fb00000
#define CPLD_BASE_PHYS		CONFIG_SYS_CPLD_BASE

#define CONFIG_SYS_FPGA_CSPR_EXT        (0x0)
#define CONFIG_SYS_FPGA_CSPR		(CSPR_PHYS_ADDR(CPLD_BASE_PHYS) | \
					CSPR_PORT_SIZE_8 | \
					CSPR_MSEL_GPCM | \
					CSPR_V)
#define CONFIG_SYS_FPGA_AMASK		IFC_AMASK(64 * 1024)
#define CONFIG_SYS_FPGA_CSOR		(CSOR_NOR_ADM_SHIFT(4) | \
					CSOR_NOR_NOR_MODE_AVD_NOR | \
					CSOR_NOR_TRHZ_80)

/* CPLD Timing parameters for IFC GPCM */
#define CONFIG_SYS_FPGA_FTIM0		(FTIM0_GPCM_TACSE(0xf) | \
					FTIM0_GPCM_TEADC(0xf) | \
					FTIM0_GPCM_TEAHC(0xf))
#define CONFIG_SYS_FPGA_FTIM1		(FTIM1_GPCM_TACO(0xff) | \
					FTIM1_GPCM_TRAD(0x3f))
#define CONFIG_SYS_FPGA_FTIM2		(FTIM2_GPCM_TCS(0xf) | \
					FTIM2_GPCM_TCH(0xf) | \
					FTIM2_GPCM_TWP(0xff))
#define CONFIG_SYS_FPGA_FTIM3           0x0
#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NOR0_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR1_EXT		CONFIG_SYS_FPGA_CSPR_EXT
#define CONFIG_SYS_CSPR1		CONFIG_SYS_FPGA_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_FPGA_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_FPGA_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_FPGA_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_FPGA_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_FPGA_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_FPGA_FTIM3

/*
 * Serial Port
 */
#ifdef CONFIG_LPUART
#define CONFIG_LPUART_32B_REG
#else
#define CONFIG_SYS_NS16550_SERIAL
#ifndef CONFIG_DM_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#endif
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#endif

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM		1
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x53
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5

/*
 * MMC
 */

/* SPI */
#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
/* QSPI */
#define QSPI0_AMBA_BASE			0x40000000
#define FSL_QSPI_FLASH_SIZE		(1 << 24)
#define FSL_QSPI_FLASH_NUM		2

/* DSPI */
#endif

/* DM SPI */
#if defined(CONFIG_FSL_DSPI) || defined(CONFIG_FSL_QSPI)
#define CONFIG_DM_SPI_FLASH
#endif

/*
 * Video
 */
#ifdef CONFIG_VIDEO_FSL_DCU_FB
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO

#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_DVI_BUS_NUM	1
#define CONFIG_SYS_I2C_DVI_ADDR		0x39
#endif

/*
 * eTSEC
 */

#ifdef CONFIG_TSEC_ENET
#define CONFIG_MII_DEFAULT_TSEC		1
#define CONFIG_TSEC1			1
#define CONFIG_TSEC1_NAME		"eTSEC1"
#define CONFIG_TSEC2			1
#define CONFIG_TSEC2_NAME		"eTSEC2"
#define CONFIG_TSEC3			1
#define CONFIG_TSEC3_NAME		"eTSEC3"

#define TSEC1_PHY_ADDR			2
#define TSEC2_PHY_ADDR			0
#define TSEC3_PHY_ADDR			1

#define TSEC1_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX			0
#define TSEC2_PHYIDX			0
#define TSEC3_PHYIDX			0

#define CONFIG_ETHPRIME			"eTSEC1"

#define CONFIG_PHY_ATHEROS

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#endif

/* PCIe */
#define CONFIG_PCIE1		/* PCIE controller 1 */
#define CONFIG_PCIE2		/* PCIE controller 2 */

#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif

#define CONFIG_CMDLINE_TAG

#define CONFIG_PEN_ADDR_BIG_ENDIAN
#define CONFIG_LAYERSCAPE_NS_ACCESS
#define CONFIG_SMP_PEN_ADDR		0x01ee0200
#define COUNTER_FREQUENCY		12500000

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		256

#define CONFIG_FSL_DEVICE_DISABLE

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#ifdef CONFIG_LPUART
#define CONFIG_EXTRA_ENV_SETTINGS       \
	"bootargs=root=/dev/ram0 rw console=ttyLP0,115200\0" \
	"initrd_high=0xffffffff\0"      \
	"fdt_high=0xffffffff\0"		\
	"fdt_addr=0x64f00000\0"		\
	"kernel_addr=0x65000000\0"	\
	"scriptaddr=0x80000000\0"	\
	"scripthdraddr=0x80080000\0"	\
	"fdtheader_addr_r=0x80100000\0"	\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"	\
	"fdt_addr_r=0x90000000\0"	\
	"ramdisk_addr_r=0xa0000000\0"	\
	"load_addr=0xa0000000\0"	\
	"kernel_size=0x2800000\0"	\
	"kernel_addr_sd=0x8000\0"	\
	"kernel_size_sd=0x14000\0"	\
	BOOTENV				\
	"boot_scripts=ls1021atwr_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1021atwr_bs.out\0"	\
		"scan_dev_for_boot_part="	\
			"part list ${devtype} ${devnum} devplist; "	\
			"env exists devplist || setenv devplist 1; "	\
			"for distro_bootpart in ${devplist}; do "	\
			"if fstype ${devtype} "				\
				"${devnum}:${distro_bootpart} "		\
				"bootfstype; then "			\
				"run scan_dev_for_boot; "		\
			"fi; "			\
		"done\0"			\
	"scan_dev_for_boot="				  \
		"echo Scanning ${devtype} "		  \
				"${devnum}:${distro_bootpart}...; "  \
		"for prefix in ${boot_prefixes}; do "	  \
			"run scan_dev_for_scripts; "	  \
		"done;"					  \
		"\0"					  \
	"boot_a_script="				  \
		"load ${devtype} ${devnum}:${distro_bootpart} "  \
			"${scriptaddr} ${prefix}${script}; "    \
		"env exists secureboot && load ${devtype} "     \
			"${devnum}:${distro_bootpart} "		\
			"${scripthdraddr} ${prefix}${boot_script_hdr}; " \
			"env exists secureboot "	\
			"&& esbc_validate ${scripthdraddr};"    \
		"source ${scriptaddr}\0"	  \
	"installer=load mmc 0:2 $load_addr "	\
		"/flex_installer_arm32.itb; "		\
		"bootm $load_addr#ls1021atwr\0"	\
	"qspi_bootcmd=echo Trying load from qspi..;"	\
		"sf probe && sf read $load_addr "	\
		"$kernel_addr $kernel_size && bootm $load_addr#$board\0"	\
	"nor_bootcmd=echo Trying load from nor..;"	\
		"cp.b $kernel_addr $load_addr "		\
		"$kernel_size && bootm $load_addr#$board\0"
#else
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"bootargs=root=/dev/ram0 rw console=ttyS0,115200\0" \
	"initrd_high=0xffffffff\0"      \
	"fdt_high=0xffffffff\0"		\
	"fdt_addr=0x64f00000\0"		\
	"kernel_addr=0x61000000\0"	\
	"kernelheader_addr=0x60800000\0"	\
	"scriptaddr=0x80000000\0"	\
	"scripthdraddr=0x80080000\0"	\
	"fdtheader_addr_r=0x80100000\0"	\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"	\
	"kernelheader_size=0x40000\0"	\
	"fdt_addr_r=0x90000000\0"	\
	"ramdisk_addr_r=0xa0000000\0"	\
	"load_addr=0xa0000000\0"	\
	"kernel_size=0x2800000\0"	\
	"kernel_addr_sd=0x8000\0"	\
	"kernel_size_sd=0x14000\0"	\
	"kernelhdr_addr_sd=0x4000\0"		\
	"kernelhdr_size_sd=0x10\0"		\
	BOOTENV				\
	"boot_scripts=ls1021atwr_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1021atwr_bs.out\0"	\
		"scan_dev_for_boot_part="	\
			"part list ${devtype} ${devnum} devplist; "	\
			"env exists devplist || setenv devplist 1; "	\
			"for distro_bootpart in ${devplist}; do "	\
			"if fstype ${devtype} "				\
				"${devnum}:${distro_bootpart} "		\
				"bootfstype; then "			\
				"run scan_dev_for_boot; "		\
			"fi; "			\
		"done\0"			\
	"scan_dev_for_boot="				  \
		"echo Scanning ${devtype} "		  \
				"${devnum}:${distro_bootpart}...; "  \
		"for prefix in ${boot_prefixes}; do "	  \
			"run scan_dev_for_scripts; "	  \
		"done;"					  \
		"\0"					  \
	"boot_a_script="				  \
		"load ${devtype} ${devnum}:${distro_bootpart} "  \
			"${scriptaddr} ${prefix}${script}; "    \
		"env exists secureboot && load ${devtype} "     \
			"${devnum}:${distro_bootpart} "		\
			"${scripthdraddr} ${prefix}${boot_script_hdr} " \
			"&& esbc_validate ${scripthdraddr};"    \
		"source ${scriptaddr}\0"	  \
	"qspi_bootcmd=echo Trying load from qspi..;"	\
		"sf probe && sf read $load_addr "	\
		"$kernel_addr $kernel_size; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0" \
	"nor_bootcmd=echo Trying load from nor..;"	\
		"cp.b $kernel_addr $load_addr "		\
		"$kernel_size; env exists secureboot "	\
		"&& cp.b $kernelheader_addr $kernelheader_addr_r "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"	\
	"sd_bootcmd=echo Trying load from SD ..;"       \
		"mmcinfo && mmc read $load_addr "	\
		"$kernel_addr_sd $kernel_size_sd && "	\
		"env exists secureboot && mmc read $kernelheader_addr_r "		\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "		\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$board\0"
#endif

#undef CONFIG_BOOTCOMMAND
#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run qspi_bootcmd"	\
			   "env exists secureboot && esbc_halt"
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run sd_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#else
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run nor_bootcmd;"	\
			   "env exists secureboot && esbc_halt;"
#endif

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

#define CONFIG_SYS_LOAD_ADDR		0x82000000

#define CONFIG_LS102XA_STREAM_ID

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MONITOR_BASE CONFIG_SPL_TEXT_BASE
#else
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE    /* start of monitor */
#endif

#define CONFIG_SYS_QE_FW_ADDR     0x60940000

/*
 * Environment
 */
#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_SD_BOOT)
#define CONFIG_ENV_OFFSET		0x300000
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			0x20000
#elif defined(CONFIG_QSPI_BOOT)
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_OFFSET		0x300000
#define CONFIG_ENV_SECT_SIZE		0x10000
#else
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x300000)
#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_SECT_SIZE		0x20000 /* 128K (one sector) */
#endif

#include <asm/fsl_secure_boot.h>
#define CONFIG_SYS_BOOTM_LEN	(64 << 20) /* Increase max gunzip size */

#endif
