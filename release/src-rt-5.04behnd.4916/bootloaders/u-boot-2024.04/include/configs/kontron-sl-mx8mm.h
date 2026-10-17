/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Kontron Electronics GmbH
 *
 * Configuration settings for the Kontron SL/BL i.MX8M-Mini boards and modules (N81xx).
 */
#ifndef __KONTRON_MX8MM_CONFIG_H
#define __KONTRON_MX8MM_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#ifdef CONFIG_SPL_BUILD
#include <config.h>
#endif

/* RAM */
#define PHYS_SDRAM			DDR_CSD1_BASE_ADDR
#define PHYS_SDRAM_SIZE			(SZ_4G)
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x200000

/* Board and environment settings */

#ifdef CONFIG_USB_EHCI_HCD
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

/* GUID for capsule updatable firmware image */
#define KONTRON_SL_MX8MM_FIT_IMAGE_GUID \
	EFI_GUID(0xd488e45a, 0x4929, 0x4b55, 0x8c, 0x14, \
		 0x86, 0xce, 0xa2, 0xcd, 0x66, 0x29)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na)
#include <config_distro_bootcmd.h>
/* Do not try to probe USB net adapters for net boot */
#undef BOOTENV_RUN_NET_USB_START
#define BOOTENV_RUN_NET_USB_START

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CFG_MALLOC_F_ADDR		0x930000
#endif

#define CFG_EXTRA_ENV_SETTINGS BOOTENV

#endif /* __KONTRON_MX8MM_CONFIG_H */
