/* SPDX-License-Identifier: GPL-2.0
 * Copyright 2016-2019 NXP
 * Copyright 2019 Vladimir Oltean <olteanv@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

/* XHCI Support - enabled by default */

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

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

/* Serial Port */
#define CFG_SYS_NS16550_CLK		get_serial_clock()

/* I2C */

/* PCIe */
#define FSL_PCIE_COMPAT			"fsl,ls1021a-pcie"

#define HWCONFIG_BUFFER_SIZE		256

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS					\
	"bootargs=root=/dev/ram0 rw console=ttyS0,115200\0"		\
	"initrd_high=0xffffffff\0"					\
	"kernel_addr=0x61000000\0"					\
	"kernelheader_addr=0x60800000\0"				\
	"scriptaddr=0x80000000\0"					\
	"scripthdraddr=0x80080000\0"					\
	"fdtheader_addr_r=0x80100000\0"					\
	"kernelheader_addr_r=0x80200000\0"				\
	"kernel_addr_r=0x80008000\0"					\
	"kernelheader_size=0x40000\0"					\
	"fdt_addr_r=0x8f000000\0"					\
	"ramdisk_addr_r=0xa0000000\0"					\
	"load_addr=0x80008000\0"					\
	"kernel_size=0x2800000\0"					\
	"kernel_addr_sd=0x8000\0"					\
	"kernel_size_sd=0x14000\0"					\
	"kernelhdr_addr_sd=0x4000\0"					\
	"kernelhdr_size_sd=0x10\0"					\
	BOOTENV								\
	"boot_scripts=ls1021atsn_boot.scr\0"				\
	"boot_script_hdr=hdr_ls1021atsn_bs.out\0"			\
		"scan_dev_for_boot_part="				\
			"part list ${devtype} ${devnum} devplist; "	\
			"env exists devplist || setenv devplist 1; "	\
			"for distro_bootpart in ${devplist}; do "	\
			"if fstype ${devtype} "				\
				"${devnum}:${distro_bootpart} "		\
				"bootfstype; then "			\
				"run scan_dev_for_boot; "		\
			"fi; "						\
		"done\0"						\
	"scan_dev_for_boot="						\
		"echo Scanning ${devtype} "				\
				"${devnum}:${distro_bootpart}...; "	\
		"for prefix in ${boot_prefixes}; do "			\
			"run scan_dev_for_scripts; "			\
			"run scan_dev_for_extlinux; "			\
		"done;"							\
		"\0"							\
	"boot_a_script="						\
		"load ${devtype} ${devnum}:${distro_bootpart} "		\
			"${scriptaddr} ${prefix}${script}; "		\
		"env exists secureboot && load ${devtype} "		\
			"${devnum}:${distro_bootpart} "			\
			"${scripthdraddr} ${prefix}${boot_script_hdr} "	\
			"&& esbc_validate ${scripthdraddr};"		\
		"source ${scriptaddr}\0"				\
	"qspi_bootcmd=echo Trying load from qspi..;"			\
		"sf probe && sf read $load_addr "			\
		"$kernel_addr $kernel_size; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"				\
	"sd_bootcmd=echo Trying load from SD ..;"			\
		"mmcinfo && mmc read $load_addr "			\
		"$kernel_addr_sd $kernel_size_sd && "			\
		"env exists secureboot && mmc read $kernelheader_addr_r " \
		"$kernelhdr_addr_sd $kernelhdr_size_sd "		\
		" && esbc_validate ${kernelheader_addr_r};"		\
		"bootm $load_addr#$board\0"

/* Miscellaneous configurable options */
#define CFG_SYS_BOOTMAPSZ		(256 << 20)

/* Environment */

#endif
