/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#ifndef __LS1012A2G5RDB_H__
#define __LS1012A2G5RDB_H__

#include "ls1012a_common.h"

/* DDR */
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1
#define CONFIG_SYS_SDRAM_SIZE		0x40000000
#define CONFIG_CMD_MEMINFO
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

/*  MMC  */
#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif

/* SATA */
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT

#define CONFIG_SYS_SATA				AHCI_BASE_ADDR

#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)
#define CONFIG_CMD_MEMINFO
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"verify=no\0"				\
	"fdt_high=0xffffffffffffffff\0"		\
	"initrd_high=0xffffffffffffffff\0"	\
	"fdt_addr=0x00f00000\0"			\
	"kernel_addr=0x01000000\0"		\
	"kernelheader_addr=0x800000\0"		\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernelheader_size=0x40000\0"		\
	"console=ttyS0,115200\0"		\
	BOOTENV					\
	"boot_scripts=ls1012ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1012ardb_bs.out\0"	\
	"scan_dev_for_boot_part="		\
	     "part list ${devtype} ${devnum} devplist; "	\
	     "env exists devplist || setenv devplist 1; "	\
	     "for distro_bootpart in ${devplist}; do "		\
		  "if fstype ${devtype} "			\
		      "${devnum}:${distro_bootpart} "		\
		      "bootfstype; then "			\
		      "run scan_dev_for_boot; "	\
		  "fi; "			\
	      "done\0"				\
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
	"installer=load mmc 0:2 $load_addr "	\
		   "/flex_installer_arm64.itb; "	\
		   "bootm $load_addr#$board\0"	\
	"qspi_bootcmd=echo Trying load from qspi..;"	\
		"sf probe && sf read $load_addr "	\
		"$kernel_addr $kernel_size; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"

#undef CONFIG_BOOTCOMMAND
#ifdef CONFIG_TFABOOT
#undef QSPI_NOR_BOOTCOMMAND
#define QSPI_NOR_BOOTCOMMAND "pfe stop;run distro_bootcmd; run qspi_bootcmd; " \
			     "env exists secureboot && esbc_halt;"
#else
#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
#define CONFIG_BOOTCOMMAND "pfe stop;run distro_bootcmd; run qspi_bootcmd; " \
			   "env exists secureboot && esbc_halt;"
#endif
#endif

#define DEFAULT_PFE_MDIO_NAME "PFE_MDIO"
#define DEFAULT_PFE_MDIO1_NAME "PFE_MDIO1"

#include <asm/fsl_secure_boot.h>

#endif /* __LS1012A2G5RDB_H__ */
