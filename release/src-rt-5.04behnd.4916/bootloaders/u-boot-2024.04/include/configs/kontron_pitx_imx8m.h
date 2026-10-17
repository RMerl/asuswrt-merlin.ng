/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __KONTRON_PITX_IMX8M_H
#define __KONTRON_PITX_IMX8M_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

/* GUID for capsule updatable firmware image */
#define KONTRON_PITX_IMX8M_FIT_IMAGE_GUID \
	EFI_GUID(0xc898e959, 0x5b1f, 0x4e6d, 0x88, 0xe0, \
		 0x40, 0xd4, 0x5c, 0xca, 0x13, 0x99)

#ifdef CONFIG_SPL_BUILD

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CFG_MALLOC_F_ADDR		0x182000
/* For RAW image gives a error info not panic */


#define CFG_POWER_PFUZE100_I2C_ADDR  0x08
#endif

/* ENET1 Config */
#if defined(CONFIG_CMD_NET)
#define CFG_FEC_MXC_PHYADDR          0

#define PHY_ANEG_TIMEOUT		20000

#endif

#define ENV_MEM_LAYOUT_SETTINGS \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=0x42000000\0" \
	"fdt_addr_r=0x48000000\0" \
	"fdtoverlay_addr_r=0x49000000\0" \
	"ramdisk_addr_r=0x48080000\0" \
	"scriptaddr=0x40000000\0" \
	"pxefile_addr_r=0x40100000\0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na) \
	func(PXE, pxe, 0)

#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	"image=Image\0" \
	"console=ttymxc2,115200\0" \
	"boot_fdt=try\0" \
	"fdtfile=freescale/imx8mq-kontron-pitx-imx8m.dtb\0" \
	"dfu_alt_info=mmc 0=flash-bin raw 0x42 0x1000 mmcpart 1\0"\
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV


#define CFG_SYS_INIT_RAM_ADDR        0x40000000
#define CFG_SYS_INIT_RAM_SIZE        0x80000

#define CFG_SYS_SDRAM_BASE           0x40000000
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			0xC0000000 /* 3GB DDR */

#define CFG_MXC_UART_BASE		UART_BASE_ADDR(3)

#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR       0

#endif
