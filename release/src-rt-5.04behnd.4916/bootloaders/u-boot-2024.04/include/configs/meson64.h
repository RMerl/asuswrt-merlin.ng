/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Amlogic Meson 64bits SoCs
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 */

#ifndef __MESON64_CONFIG_H
#define __MESON64_CONFIG_H

/* Generic Interrupt Controller Definitions */
#if (defined(CONFIG_MESON_AXG) || defined(CONFIG_MESON_G12A))
#define GICD_BASE			0xffc01000
#define GICC_BASE			0xffc02000
#elif defined(CONFIG_MESON_A1)
#define GICD_BASE			0xff901000
#define GICC_BASE			0xff902000
#else /* MESON GXL and GXBB */
#define GICD_BASE			0xc4301000
#define GICC_BASE			0xc4302000
#endif

/* Serial drivers */
/* The following table includes the supported baudrates */
#define CFG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, \
		230400, 250000, 460800, 500000, 1000000, 2000000, 4000000, \
		8000000 }

/* For splashscreen */
#ifdef CONFIG_VIDEO
#define STDOUT_CFG "vidconsole,serial"
#else
#define STDOUT_CFG "serial"
#endif

#ifdef CONFIG_USB_KEYBOARD
#define STDIN_CFG "usbkbd,serial"
#else
#define STDIN_CFG "serial"
#endif

#define CFG_SYS_SDRAM_BASE		0

/* ROM USB boot support, auto-execute boot.scr at scriptaddr */
#define BOOTENV_DEV_ROMUSB(devtypeu, devtypel, instance) \
	"bootcmd_romusb=" \
		"if test \"${boot_source}\" = \"usb\" && " \
				"test -n \"${scriptaddr}\"; then " \
			"echo '(ROM USB boot)'; " \
			"source ${scriptaddr}; " \
		"fi\0"

#define BOOTENV_DEV_NAME_ROMUSB(devtypeu, devtypel, instance)	\
		"romusb "

/*
 * Fallback to "USB DFU" boot if script is not at scriptaddr
 *
 * DFU will expose the kernel_addr_r memory range as DFU entry,
 * then with `dfu-util --detach`, booting the uploaded image
 * will be attempted:
 * $ dfu-util -a 0 -D fitImage
 * $ dfu-util -a 0 -e
 */
#if CONFIG_IS_ENABLED(USB_GADGET) && CONFIG_IS_ENABLED(DFU_RAM)
	#define BOOTENV_DEV_USB_DFU(devtypeu, devtypel, instance) \
		"bootcmd_usbdfu=" \
			"if test \"${boot_source}\" = \"usb\"; then " \
				"dfu 0 ram 0 60;" \
				"bootm ${kernel_addr_r};" \
			"fi\0"

	#define BOOTENV_DEV_NAME_USB_DFU(devtypeu, devtypel, instance) \
		"usbdfu "
#else
	#define BOOTENV_DEV_USB_DFU(devtypeu, devtypel, instance)
	#define BOOTENV_DEV_NAME_USB_DFU(devtypeu, devtypel, instance)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#ifdef CONFIG_CMD_NVME
	#define BOOT_TARGET_NVME(func) func(NVME, nvme, 0)
#else
	#define BOOT_TARGET_NVME(func)
#endif

#ifdef CONFIG_CMD_SCSI
	#define BOOT_TARGET_SCSI(func) func(SCSI, scsi, 0)
#else
	#define BOOT_TARGET_SCSI(func)
#endif

#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(ROMUSB, romusb, na)  \
	func(USB_DFU, usbdfu, na)  \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_NVME(func) \
	BOOT_TARGET_SCSI(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#endif

#define BOOTM_SIZE		__stringify(0x1700000)
#define KERNEL_ADDR_R		__stringify(0x08080000)
#define KERNEL_COMP_ADDR_R	__stringify(0x0d080000)
#define FDT_ADDR_R		__stringify(0x08008000)
#define SCRIPT_ADDR_R		__stringify(0x08000000)
#define PXEFILE_ADDR_R		__stringify(0x01080000)
#define FDTOVERLAY_ADDR_R	__stringify(0x01000000)
#define RAMDISK_ADDR_R		__stringify(0x13000000)

#include <config_distro_bootcmd.h>

#ifndef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
	"stdin=" STDIN_CFG "\0" \
	"stdout=" STDOUT_CFG "\0" \
	"stderr=" STDOUT_CFG "\0" \
	"kernel_comp_addr_r=" KERNEL_COMP_ADDR_R "\0" \
	"kernel_comp_size=0x2000000\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0" \
	"fdtoverlay_addr_r=" FDTOVERLAY_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"fdtfile=amlogic/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"dfu_alt_info=fitimage ram " KERNEL_ADDR_R " 0x4000000 \0" \
	BOOTENV
#endif


#endif /* __MESON64_CONFIG_H */
