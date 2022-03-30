/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2012
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __TEGRA_COMMON_POST_H
#define __TEGRA_COMMON_POST_H

/*
 * Size of malloc() pool
 */
#ifdef CONFIG_DFU_OVER_USB
#define CONFIG_SYS_MALLOC_LEN	(SZ_4M + \
					CONFIG_SYS_DFU_DATA_BUF_SIZE + \
					CONFIG_SYS_DFU_MAX_FILE_SIZE)
#else
#define CONFIG_SYS_MALLOC_LEN		(4 << 20)	/* 4MB  */
#endif

#define CONFIG_SYS_NONCACHED_MEMORY	(1 << 20)	/* 1 MiB */

#ifndef CONFIG_SPL_BUILD
#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#endif
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

#ifdef CONFIG_TEGRA_KEYBOARD
#define STDIN_KBD_KBC ",tegra-kbc"
#else
#define STDIN_KBD_KBC ""
#endif

#ifdef CONFIG_USB_KEYBOARD
#define STDIN_KBD_USB ",usbkbd"
#define CONFIG_PREBOOT			"usb start"
#else
#define STDIN_KBD_USB ""
#endif

#ifdef CONFIG_LCD
#define STDOUT_LCD ",lcd"
#else
#define STDOUT_LCD ""
#endif

#ifdef CONFIG_DM_VIDEO
#define STDOUT_VIDEO ",vidconsole"
#else
#define STDOUT_VIDEO ""
#endif

#ifdef CONFIG_CROS_EC_KEYB
#define STDOUT_CROS_EC	",cros-ec-keyb"
#else
#define STDOUT_CROS_EC	""
#endif

#define TEGRA_DEVICE_SETTINGS \
	"stdin=serial" STDIN_KBD_KBC STDIN_KBD_USB STDOUT_CROS_EC "\0" \
	"stdout=serial" STDOUT_LCD STDOUT_VIDEO "\0" \
	"stderr=serial" STDOUT_LCD STDOUT_VIDEO "\0" \
	""

#ifndef BOARD_EXTRA_ENV_SETTINGS
#define BOARD_EXTRA_ENV_SETTINGS
#endif

#define CONFIG_SYS_LOAD_ADDR CONFIG_LOADADDR

#ifndef CONFIG_CHROMEOS_EXTRA_ENV_SETTINGS
#define CONFIG_CHROMEOS_EXTRA_ENV_SETTINGS
#endif

#ifdef CONFIG_ARM64
#define FDT_HIGH "ffffffffffffffff"
#define INITRD_HIGH "ffffffffffffffff"
#else
#define FDT_HIGH "ffffffff"
#define INITRD_HIGH "ffffffff"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	TEGRA_DEVICE_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	"fdt_high=" FDT_HIGH "\0" \
	"initrd_high=" INITRD_HIGH "\0" \
	BOOTENV \
	BOARD_EXTRA_ENV_SETTINGS \
	CONFIG_CHROMEOS_EXTRA_ENV_SETTINGS

#if defined(CONFIG_TEGRA20_SFLASH) || defined(CONFIG_TEGRA20_SLINK) || defined(CONFIG_TEGRA114_SPI)
#define CONFIG_TEGRA_SPI
#endif

/* overrides for SPL build here */
#ifdef CONFIG_SPL_BUILD

#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY

/* remove I2C support */
#ifdef CONFIG_SYS_I2C_TEGRA
#undef CONFIG_SYS_I2C_TEGRA
#endif

/* remove USB */
#ifdef CONFIG_USB_EHCI_TEGRA
#undef CONFIG_USB_EHCI_TEGRA
#endif

#endif /* CONFIG_SPL_BUILD */

#endif /* __TEGRA_COMMON_POST_H */
