/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef __LS1028A_QDS_H
#define __LS1028A_QDS_H

#include "ls1028a_common.h"

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000
#define COUNTER_FREQUENCY_REAL		(CONFIG_SYS_CLK_FREQ / 4)

/* DDR */
#define CONFIG_DIMM_SLOTS_PER_CTLR		2

#define CONFIG_QIXIS_I2C_ACCESS
#define CONFIG_SYS_I2C_EARLY_INIT

/*
 * QIXIS Definitions
 */
#define CONFIG_FSL_QIXIS

#ifdef CONFIG_FSL_QIXIS
#define QIXIS_BASE			0x7fb00000
#define QIXIS_BASE_PHYS			QIXIS_BASE
#define CONFIG_SYS_I2C_FPGA_ADDR	0x66
#define QIXIS_LBMAP_SWITCH		1
#define QIXIS_LBMAP_MASK		0x0f
#define QIXIS_LBMAP_SHIFT		5
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x00
#define QIXIS_LBMAP_SD			0x00
#define QIXIS_LBMAP_EMMC		0x00
#define QIXIS_LBMAP_QSPI		0x00
#define QIXIS_RCW_SRC_SD		0x8
#define QIXIS_RCW_SRC_EMMC		0x9
#define QIXIS_RCW_SRC_QSPI		0xf
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_RST_FORCE_MEM		0x01

#define CONFIG_SYS_FPGA_CSPR_EXT	(0x0)
#define CONFIG_SYS_FPGA_CSPR		(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS) | \
					CSPR_PORT_SIZE_8 | \
					CSPR_MSEL_GPCM | \
					CSPR_V)
#define CONFIG_SYS_FPGA_AMASK		IFC_AMASK(64 * 1024)
#define CONFIG_SYS_FPGA_CSOR		(CSOR_NOR_ADM_SHIFT(4) | \
					CSOR_NOR_NOR_MODE_AVD_NOR | \
					CSOR_NOR_TRHZ_80)
#endif

/* RTC */
#define CONFIG_SYS_RTC_BUS_NUM         1
#define I2C_MUX_CH_RTC                 0xB

/* Store environment at top of flash */
#define CONFIG_ENV_SIZE			0x2000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MONITOR_BASE CONFIG_SPL_TEXT_BASE
#else
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE
#endif

/* SATA */
#define CONFIG_SCSI_AHCI_PLAT

#define CONFIG_SYS_SATA1			AHCI_BASE_ADDR1
#ifndef CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT2
#endif
#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)
/* DSPI */
#ifdef CONFIG_FSL_DSPI
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SPI_FLASH_EON
#endif

#ifndef SPL_NO_ENV
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"board=ls1028aqds\0" \
	"hwconfig=fsl_ddr:bank_intlv=auto\0" \
	"ramdisk_addr=0x800000\0" \
	"ramdisk_size=0x2000000\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"fdt_addr=0x00f00000\0" \
	"kernel_addr=0x01000000\0" \
	"scriptaddr=0x80000000\0" \
	"scripthdraddr=0x80080000\0" \
	"fdtheader_addr_r=0x80100000\0" \
	"kernelheader_addr_r=0x80200000\0" \
	"load_addr=0xa0000000\0" \
	"kernel_addr_r=0x81000000\0" \
	"fdt_addr_r=0x90000000\0" \
	"ramdisk_addr_r=0xa0000000\0" \
	"kernel_start=0x1000000\0" \
	"kernelheader_start=0x800000\0" \
	"kernel_load=0xa0000000\0" \
	"kernel_size=0x2800000\0" \
	"kernelheader_size=0x40000\0" \
	"kernel_addr_sd=0x8000\0" \
	"kernel_size_sd=0x14000\0" \
	"kernelhdr_addr_sd=0x4000\0" \
	"kernelhdr_size_sd=0x10\0" \
	"console=ttyS0,115200\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	BOOTENV \
	"boot_scripts=ls1028aqds_boot.scr\0" \
	"boot_script_hdr=hdr_ls1028aqds_bs.out\0" \
	"scan_dev_for_boot_part=" \
		"part list ${devtype} ${devnum} devplist; " \
		"env exists devplist || setenv devplist 1; " \
		"for distro_bootpart in ${devplist}; do " \
		  "if fstype ${devtype} " \
			"${devnum}:${distro_bootpart} " \
			"bootfstype; then " \
			"run scan_dev_for_boot; " \
		  "fi; " \
		"done\0" \
	"scan_dev_for_boot=" \
		"echo Scanning ${devtype} " \
				"${devnum}:${distro_bootpart}...; " \
		"for prefix in ${boot_prefixes}; do " \
			"run scan_dev_for_scripts; " \
		"done;" \
		"\0" \
	"boot_a_script=" \
		"load ${devtype} ${devnum}:${distro_bootpart} " \
			"${scriptaddr} ${prefix}${script}; " \
		"env exists secureboot && load ${devtype} " \
			"${devnum}:${distro_bootpart} " \
			"${scripthdraddr} ${prefix}${boot_script_hdr} " \
			"&& esbc_validate ${scripthdraddr};" \
		"source ${scriptaddr}\0" \
	"sd_bootcmd=echo Trying load from SD ..;" \
		"mmcinfo; mmc read $load_addr " \
		"$kernel_addr_sd $kernel_size_sd && " \
		"env exists secureboot && mmc read $kernelheader_addr_r " \
		"$kernelhdr_addr_sd $kernelhdr_size_sd " \
		" && esbc_validate ${kernelheader_addr_r};" \
		"bootm $load_addr#$board\0" \
	"emmc_bootcmd=echo Trying load from EMMC ..;" \
		"mmcinfo; mmc dev 1; mmc read $load_addr " \
		"$kernel_addr_sd $kernel_size_sd && " \
		"env exists secureboot && mmc read $kernelheader_addr_r " \
		"$kernelhdr_addr_sd $kernelhdr_size_sd " \
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$board\0"
#endif
#endif /* __LS1028A_QDS_H */
