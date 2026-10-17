/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#ifndef __LS1012ARDB_H__
#define __LS1012ARDB_H__

#include "ls1012a_common.h"

/* DDR */
#define CFG_SYS_SDRAM_SIZE		0x20000000

#undef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0)

#undef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS		\
	"verify=no\0"				\
	"fdt_high=0xffffffffffffffff\0"		\
	"kernel_addr=0x01000000\0"		\
	"scriptaddr=0x80000000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x96000000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0x96000000\0"		\
	"kernel_size=0x2800000\0"		\
	"bootm_size=0x10000000\0"		\
	"console=ttyS0,115200\0"		\
	BOOTENV					\
	"boot_scripts=ls1012afrdm_boot.scr\0"	\
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
	"installer=load usb 0:2 $load_addr "	\
		   "/flex_installer_arm64.itb; "	\
		   "bootm $load_addr#$board\0"	\
	"qspi_bootcmd=echo Trying load from qspi..;"	\
		"sf probe && sf read $load_addr "	\
		"$kernel_addr $kernel_size && bootm $load_addr#$board\0"

#ifdef CONFIG_TFABOOT
#undef QSPI_NOR_BOOTCOMMAND
#define QSPI_NOR_BOOTCOMMAND "run distro_bootcmd;run qspi_bootcmd"
#endif

#endif /* __LS1012ARDB_H__ */
