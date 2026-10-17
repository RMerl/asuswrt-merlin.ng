/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2019, 2021 NXP
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

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

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(1u * 1024 * 1024 * 1024)

#define CFG_SYS_DDR_SDRAM_BASE      0x80000000UL
#define CFG_SYS_SDRAM_BASE          CFG_SYS_DDR_SDRAM_BASE

/*
 * IFC Definitions
 */
#if !defined(CONFIG_QSPI_BOOT) && !defined(CONFIG_SD_BOOT_QSPI)
#define CFG_SYS_FLASH_BASE		0x60000000
#define CFG_SYS_FLASH_BASE_PHYS	CFG_SYS_FLASH_BASE

#define CFG_SYS_NOR0_CSPR_EXT	(0x0)
#define CFG_SYS_NOR0_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CFG_SYS_NOR_AMASK		IFC_AMASK(128 * 1024 * 1024)

/* NOR Flash Timing Params */
#define CFG_SYS_NOR_CSOR		(CSOR_NOR_ADM_SHIFT(4) | \
					CSOR_NOR_TRHZ_80)
#define CFG_SYS_NOR_FTIM0		(FTIM0_NOR_TACSE(0x4) | \
					FTIM0_NOR_TEADC(0x5) | \
					FTIM0_NOR_TAVDS(0x0) | \
					FTIM0_NOR_TEAHC(0x5))
#define CFG_SYS_NOR_FTIM1		(FTIM1_NOR_TACO(0x35) | \
					FTIM1_NOR_TRAD_NOR(0x1A) | \
					FTIM1_NOR_TSEQRAD_NOR(0x13))
#define CFG_SYS_NOR_FTIM2		(FTIM2_NOR_TCS(0x4) | \
					FTIM2_NOR_TCH(0x4) | \
					FTIM2_NOR_TWP(0x1c) | \
					FTIM2_NOR_TWPH(0x0e))
#define CFG_SYS_NOR_FTIM3		0

#define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH_BASE_PHYS }

#define CFG_SYS_WRITE_SWAPPED_DATA
#endif

/* CPLD */

#define CFG_SYS_CPLD_BASE	0x7fb00000
#define CPLD_BASE_PHYS		CFG_SYS_CPLD_BASE

#define CFG_SYS_FPGA_CSPR_EXT        (0x0)
#define CFG_SYS_FPGA_CSPR		(CSPR_PHYS_ADDR(CPLD_BASE_PHYS) | \
					CSPR_PORT_SIZE_8 | \
					CSPR_MSEL_GPCM | \
					CSPR_V)
#define CFG_SYS_FPGA_AMASK		IFC_AMASK(64 * 1024)
#define CFG_SYS_FPGA_CSOR		(CSOR_NOR_ADM_SHIFT(4) | \
					CSOR_NOR_NOR_MODE_AVD_NOR | \
					CSOR_NOR_TRHZ_80)

/* CPLD Timing parameters for IFC GPCM */
#define CFG_SYS_FPGA_FTIM0		(FTIM0_GPCM_TACSE(0xf) | \
					FTIM0_GPCM_TEADC(0xf) | \
					FTIM0_GPCM_TEAHC(0xf))
#define CFG_SYS_FPGA_FTIM1		(FTIM1_GPCM_TACO(0xff) | \
					FTIM1_GPCM_TRAD(0x3f))
#define CFG_SYS_FPGA_FTIM2		(FTIM2_GPCM_TCS(0xf) | \
					FTIM2_GPCM_TCH(0xf) | \
					FTIM2_GPCM_TWP(0xff))
#define CFG_SYS_FPGA_FTIM3           0x0
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NOR0_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NOR0_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NOR_FTIM3
#define CFG_SYS_CSPR1_EXT		CFG_SYS_FPGA_CSPR_EXT
#define CFG_SYS_CSPR1		CFG_SYS_FPGA_CSPR
#define CFG_SYS_AMASK1		CFG_SYS_FPGA_AMASK
#define CFG_SYS_CSOR1		CFG_SYS_FPGA_CSOR
#define CFG_SYS_CS1_FTIM0		CFG_SYS_FPGA_FTIM0
#define CFG_SYS_CS1_FTIM1		CFG_SYS_FPGA_FTIM1
#define CFG_SYS_CS1_FTIM2		CFG_SYS_FPGA_FTIM2
#define CFG_SYS_CS1_FTIM3		CFG_SYS_FPGA_FTIM3

/*
 * Serial Port
 */
#ifndef CONFIG_LPUART
#define CFG_SYS_NS16550_CLK		get_serial_clock()
#endif

/*
 * I2C
 */

/* GPIO */

#define CFG_SMP_PEN_ADDR		0x01ee0200

#define HWCONFIG_BUFFER_SIZE		256

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#ifdef CONFIG_LPUART
#define CFG_EXTRA_ENV_SETTINGS       \
	"bootargs=root=/dev/ram0 rw console=ttyLP0,115200 "	\
		"cma=64M@0x0-0xb0000000\0" \
	"initrd_high=0xffffffff\0"      \
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
	"othbootargs=cma=64M@0x0-0xb0000000\0"	\
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
#define CFG_EXTRA_ENV_SETTINGS	\
	"bootargs=root=/dev/ram0 rw console=ttyS0,115200 "	\
		"cma=64M@0x0-0xb0000000\0" \
	"initrd_high=0xffffffff\0"      \
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
	"othbootargs=cma=64M@0x0-0xb0000000\0"	\
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

/*
 * Miscellaneous configurable options
 */
#define CFG_SYS_BOOTMAPSZ		(256 << 20)

/*
 * Environment
 */

#include <asm/fsl_secure_boot.h>

#endif
