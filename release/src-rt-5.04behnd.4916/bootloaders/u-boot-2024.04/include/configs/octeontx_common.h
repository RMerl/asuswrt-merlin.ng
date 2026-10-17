/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX_COMMON_H__
#define __OCTEONTX_COMMON_H__

#ifdef CONFIG_DISTRO_DEFAULTS
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0)

#include <config_distro_bootcmd.h>
/* Extra environment variables */
#define CFG_EXTRA_ENV_SETTINGS	\
	"loadaddr=0x20080000\0"		\
	"kernel_addr_r=0x02000000\0"	\
	"ramdisk_addr_r=0x03000000\0"	\
	"scriptaddr=0x04000000\0"	\
	BOOTENV

#else

/** Extra environment settings */
#define CFG_EXTRA_ENV_SETTINGS	\
	"loadaddr=20080000\0"		\
	"autoload=0\0"

#endif /* ifdef CONFIG_DISTRO_DEFAULTS*/

/** Maximum size of image supported for bootm (and bootable FIT images) */

/** Memory base address */
#define CFG_SYS_SDRAM_BASE		CONFIG_TEXT_BASE

/** Stack starting address */

/** Heap size for U-Boot */

/** EMMC specific defines */

#endif /* __OCTEONTX_COMMON_H__ */
