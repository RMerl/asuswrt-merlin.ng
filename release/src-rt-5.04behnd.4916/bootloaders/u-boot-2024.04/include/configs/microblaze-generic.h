/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2010 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Microblaze is microblaze_0 */
#define XILINX_FSL_NUMBER	3

/* uart */
/* The following table includes the supported baudrates */
# define CFG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* architecture dependent code */
#if defined(CONFIG_CMD_PXE) && defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_PXE(func)	func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func)	func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_SPI_FLASH)
# define BOOT_TARGET_DEVICES_QSPI(func) func(QSPI, qspi, na)
#else
# define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#if defined(CONFIG_MTD_NOR_FLASH)
# define BOOT_TARGET_DEVICES_NOR(func)  func(NOR, nor, na)
#else
# define BOOT_TARGET_DEVICES_NOR(func)
#endif

#define BOOTENV_DEV_NOR(devtypeu, devtypel, instance) \
	"bootcmd_nor=cp.b ${script_offset_nor} ${scriptaddr} ${script_size_f} && " \
		"echo NOR: Trying to boot script at ${scriptaddr} && " \
		"source ${scriptaddr}; echo NOR: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_NOR(devtypeu, devtypel, instance) \
	"nor "

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=sf probe 0 0 0 && " \
	"sf read ${scriptaddr} ${script_offset_f} ${script_size_f} && " \
	"echo QSPI: Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; echo QSPI: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#define BOOT_TARGET_DEVICES_JTAG(func)	func(JTAG, jtag, na)

#define BOOTENV_DEV_JTAG(devtypeu, devtypel, instance) \
	"bootcmd_jtag=echo JTAG: Trying to boot script at ${scriptaddr} && " \
		"source ${scriptaddr}; echo JTAG: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_JTAG(devtypeu, devtypel, instance) \
	"jtag "

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_JTAG(func) \
	BOOT_TARGET_DEVICES_QSPI(func) \
	BOOT_TARGET_DEVICES_NOR(func) \
	BOOT_TARGET_DEVICES_DHCP(func) \
	BOOT_TARGET_DEVICES_PXE(func)

#include <config_distro_bootcmd.h>

#ifndef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
	"unlock=yes\0"\
	"nor0=flash-0\0"\
	"mtdparts=mtdparts=flash-0:"\
	"256k(u-boot),256k(env),3m(kernel),"\
	"1m(romfs),1m(cramfs),-(jffs2)\0"\
	"nc=setenv stdout nc;"\
	"setenv stdin nc\0" \
	"serial=setenv stdout serial;"\
	"setenv stdin serial\0"\
	"script_size_f=0x40000\0"\
	BOOTENV
#endif

/* SPL part */

#define CFG_SYS_UBOOT_BASE		CONFIG_TEXT_BASE

#endif	/* __CONFIG_H */
