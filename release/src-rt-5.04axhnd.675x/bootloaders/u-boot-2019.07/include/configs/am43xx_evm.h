/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am43xx_evm.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef __CONFIG_AM43XX_EVM_H
#define __CONFIG_AM43XX_EVM_H

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_MAX_RAM_BANK_SIZE	(1024 << 21)	/* 2GB */
#define CONFIG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */

#include <asm/arch/omap.h>

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_CLK		48000000
#if !defined(CONFIG_SPL_DM) || !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_REG_SIZE    (-4)
#define CONFIG_SYS_NS16550_SERIAL
#endif

/* I2C Configuration */
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

/* Power */
#ifndef CONFIG_DM_I2C
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#endif
#define CONFIG_POWER_TPS65218
#define CONFIG_POWER_TPS62362

/* SPL defines. */
#define CONFIG_SYS_SPL_ARGS_ADDR	(CONFIG_SYS_SDRAM_BASE + \
					 (128 << 20))

/* Enabling L2 Cache */
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE	0x48242000

/*
 * Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#if !defined(CONFIG_SPL_BUILD) && !defined(CONFIG_QSPI_BOOT)
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/*
 * When building U-Boot such that there is no previous loader
 * we need to call board_early_init_f.  This is taken care of in
 * s_init when we have SPL used.
 */

/* Now bring in the rest of the common code. */
#include <configs/ti_armv7_omap.h>

/* Always 64 KiB env size */
#define CONFIG_ENV_SIZE			(64 << 10)

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */

/* SPL USB Support */

#if defined(CONFIG_SPL_USB_HOST_SUPPORT) || !defined(CONFIG_SPL_BUILD)
#define CONFIG_SYS_USB_FAT_BOOT_PARTITION		1
#define CONFIG_USB_XHCI_OMAP

#define CONFIG_AM437X_USB2PHY2_HOST
#endif

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_SPL_USB_ETHER)
#undef CONFIG_USB_DWC3_PHY_OMAP
#undef CONFIG_USB_DWC3_OMAP
#undef CONFIG_USB_DWC3
#undef CONFIG_USB_DWC3_GADGET

#undef CONFIG_USB_GADGET_DOWNLOAD
#undef CONFIG_USB_GADGET_VBUS_DRAW
#undef CONFIG_USB_GADGET_MANUFACTURER
#undef CONFIG_USB_GADGET_VENDOR_NUM
#undef CONFIG_USB_GADGET_PRODUCT_NUM
#undef CONFIG_USB_GADGET_DUALSPEED
#endif

/*
 * Disable MMC DM for SPL build and can be re-enabled after adding
 * DM support in SPL
 */
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_TIMER
#endif

#ifndef CONFIG_SPL_BUILD
/* USB Device Firmware Update support */
#define DFUARGS \
	"dfu_bufsiz=0x10000\0" \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_QSPI_XIP
#else
#define DFUARGS
#endif

#ifdef CONFIG_QSPI_BOOT
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE           (64 << 10) /* 64 KB sectors */
#define CONFIG_ENV_OFFSET              0x110000
#define CONFIG_ENV_OFFSET_REDUND       0x120000
#endif

#ifndef CONFIG_SPL_BUILD
#include <environment/ti/dfu.h>
#include <environment/ti/mmc.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	DEFAULT_FIT_TI_ARGS \
	"fdtfile=undefined\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"console=ttyO0,115200n8\0" \
	"partitions=" \
		"uuid_disk=${uuid_gpt_disk};" \
		"name=rootfs,start=2MiB,size=-,uuid=${uuid_gpt_rootfs}\0" \
	"optargs=\0" \
	"usbroot=/dev/sda2 rw\0" \
	"usbrootfstype=ext4 rootwait\0" \
	"usbdev=0\0" \
	"ramroot=/dev/ram0 rw\0" \
	"ramrootfstype=ext2\0" \
	"usbargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${usbroot} " \
		"rootfstype=${usbrootfstype}\0" \
	"ramargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${ramroot} " \
		"rootfstype=${ramrootfstype}\0" \
	"loadramdisk=load ${devtype} ${devnum} ${rdaddr} ramdisk.gz\0" \
	"usbboot=" \
		"setenv devnum ${usbdev}; " \
		"setenv devtype usb; " \
		"usb start ${usbdev}; " \
		"if usb dev ${usbdev}; then " \
			"if run loadbootenv; then " \
				"echo Loaded environment from ${bootenv};" \
				"run importbootenv;" \
			"fi;" \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...;" \
				"run uenvcmd;" \
			"fi;" \
			"if run loadimage; then " \
				"run loadfdt; " \
				"echo Booting from usb ${usbdev}...; " \
				"run usbargs;" \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"fi;" \
		"fi\0" \
		"fi;" \
		"usb stop ${usbdev};\0" \
	"findfdt="\
		"if test $board_name = AM43EPOS; then " \
			"setenv fdtfile am43x-epos-evm.dtb; fi; " \
		"if test $board_name = AM43__GP; then " \
			"setenv fdtfile am437x-gp-evm.dtb; fi; " \
		"if test $board_name = AM43XXHS; then " \
			"setenv fdtfile am437x-gp-evm.dtb; fi; " \
		"if test $board_name = AM43__SK; then " \
			"setenv fdtfile am437x-sk-evm.dtb; fi; " \
		"if test $board_name = AM43_IDK; then " \
			"setenv fdtfile am437x-idk-evm.dtb; fi; " \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine device tree; fi; \0" \
	NANDARGS \
	NETARGS \
	DFUARGS \

#define CONFIG_BOOTCOMMAND \
	"if test ${boot_fit} -eq 1; then "	\
		"run update_to_fit;"	\
	"fi;"	\
	"run findfdt; " \
	"run envboot;" \
	"run mmcboot;" \
	"run usbboot;" \
	NANDBOOT \

#endif

#ifndef CONFIG_SPL_BUILD
/* CPSW Ethernet */
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT		10
#endif

#define PHY_ANEG_TIMEOUT	8000 /* PHY needs longer aneg time at 1G */

#define CONFIG_SYS_RX_ETH_BUFFER	64

/* NAND support */
#ifdef CONFIG_NAND
/* NAND: device related configs */
#define CONFIG_SYS_NAND_PAGE_SIZE	4096
#define CONFIG_SYS_NAND_OOBSIZE		224
#define CONFIG_SYS_NAND_BLOCK_SIZE	(256*1024)
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
/* NAND: driver related configs */
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH16_CODE_HW
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS	{ 2, 3, 4, 5, 6, 7, 8, 9, \
				10, 11, 12, 13, 14, 15, 16, 17, 18, 19, \
				20, 21, 22, 23, 24, 25, 26, 27, 28, 29, \
				30, 31, 32, 33, 34, 35, 36, 37, 38, 39, \
				40, 41, 42, 43, 44, 45, 46, 47, 48, 49, \
				50, 51, 52, 53, 54, 55, 56, 57, 58, 59, \
				60, 61, 62, 63, 64, 65, 66, 67, 68, 69, \
				70, 71, 72, 73, 74, 75, 76, 77, 78, 79, \
				80, 81, 82, 83, 84, 85, 86, 87, 88, 89, \
				90, 91, 92, 93, 94, 95, 96, 97, 98, 99, \
			100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
			110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
			120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
			130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
			140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
			150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
			160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
			170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
			180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
			190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
			200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
			}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	26
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x00180000
/* NAND: SPL related configs */
/* NAND: SPL falcon mode configs */
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x00300000 /* kernel offset */
#endif
#define NANDARGS \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandroot=ubi0:rootfs rw ubi.mtd=NAND.file-system,4096\0" \
	"nandrootfstype=ubifs rootwait=1\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${fdtaddr} NAND.u-boot-spl-os; " \
		"nand read ${loadaddr} NAND.kernel; " \
		"bootz ${loadaddr} - ${fdtaddr}\0"
#define NANDBOOT			"run nandboot; "
#else /* !CONFIG_NAND */
#define NANDARGS
#define NANDBOOT
#endif /* CONFIG_NAND */

#if defined(CONFIG_TI_SECURE_DEVICE)
/* Avoid relocating onto firewalled area at end of DRAM */
#define CONFIG_PRAM (64 * 1024)
#endif /* CONFIG_TI_SECURE_DEVICE */

#endif	/* __CONFIG_AM43XX_EVM_H */
