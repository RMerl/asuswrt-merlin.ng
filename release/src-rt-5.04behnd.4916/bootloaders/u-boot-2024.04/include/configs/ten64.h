/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright 2019-2021 Traverse Technologies
 */

#ifndef __TEN64_H
#define __TEN64_H

#include "ls1088a_common.h"


#define CFG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000

#define QSPI_NOR_BOOTCOMMAND	"run distro_bootcmd"
#define SD_BOOTCOMMAND		"run distro_bootcmd"

#define SD_FIRMWARE_PATH "firmware/traverse/ten64/"

#define QSPI_MC_INIT_CMD				\
	"sf probe 0:0 && sf read 0x80000000 0x300000 0x200000 &&"	\
	"sf read 0x80200000 0x5C0000 0x40000 &&"				\
	"fsl_mc start mc 0x80000000 0x80200000 && " \
	"sf read 0x8E000000 0x580000 0x40000 && fsl_mc lazyapply DPL 0x8E000000 && "\
	"echo 'default DPL loaded'\0"
#define SD_MC_INIT_CMD				\
	"mmcinfo; fatload mmc 0 0x80000000 " SD_FIRMWARE_PATH "mc_ls1088a.itb; "\
	"fatload mmc 0 0x80200000 " SD_FIRMWARE_PATH "dpc.0x1D-0x0D.dtb; "\
	"fsl_mc start mc 0x80000000 0x80200000 && "	\
	"fatload mmc 0 0x8E000000 " SD_FIRMWARE_PATH "eth-dpl-all.dtb && " \
	"fsl_mc lazyapply DPL 0x8E000000 && echo 'default DPL loaded'\0"

#define BOOT_TARGET_DEVICES(func) \
	func(NVME, nvme, 0) \
	func(USB, usb, 0) \
	func(MMC, mmc, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, 0) \
	func(PXE, pxe, 0)
#include <config_distro_bootcmd.h>

#define OPENWRT_NAND_BOOTCMD	\
	"bootcmd_openwrt_nand=ubi part ubi${openwrt_active_sys} && "\
	"ubi read $load_addr kernel && " \
	"setenv bootargs \"root=/dev/ubiblock0_1 earlycon ubi.mtd=ubi${openwrt_active_sys}\" &&"\
	"bootm $load_addr#ten64\0"
#undef CFG_EXTRA_ENV_SETTINGS

#if CONFIG_IS_ENABLED(CMD_BOOTMENU)
#define DEFAULT_MENU_ENTRIES \
	"bootmenu_0=Continue standard boot=run bootcmd\0" \
	"bootmenu_1=Boot into recovery=run bootcmd_recovery\0" \
	"bootmenu_2=Boot OpenWrt from NAND=run bootcmd_openwrt_nand\0"
#else
#define DEFAULT_MENU_ENTRIES ""
#endif /* CONFIG_IS_ENABLED(CMD_BOOTMENU) */

#define CFG_EXTRA_ENV_SETTINGS \
	"BOARD=ten64\0"					\
	"fdt_addr_r=0x90000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"kernel_addr_r=0x81000000\0"		\
	"load_addr=0xa0000000\0"		\
	BOOTENV \
	OPENWRT_NAND_BOOTCMD \
	"openwrt_active_sys=a\0" \
	"load_efi_dtb=mtd read devicetree $fdt_addr_r && fdt addr $fdt_addr_r && " \
	"fdt resize && fdt boardsetup\0" \
	"bootcmd_recovery=mtd read recovery 0xa0000000;  " \
	"setenv bootargs \"earlycon root=/dev/ram0 ramdisk_size=0x3000000\" && bootm 0xa0000000#ten64\0" \
	DEFAULT_MENU_ENTRIES

#endif /* __TEN64_H */
