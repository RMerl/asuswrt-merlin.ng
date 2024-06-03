/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Timesys Corporation
 * Copyright (C) 2016 Advantech Corporation
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 */

#ifndef __ADVANTECH_DMSBA16_CONFIG_H
#define __ADVANTECH_DMSBA16_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#define CONFIG_BOARD_NAME	"Advantech DMS-BA16"

#define CONFIG_MXC_UART_BASE	UART4_BASE
#define CONSOLE_DEV	"ttymxc3"
#define CONFIG_EXTRA_BOOTARGS	"panic=10"

#define CONFIG_BOOT_DIR	""
#define CONFIG_LOADCMD "fatload"
#define CONFIG_RFSPART "2"

#include "mx6_common.h"
#include <linux/sizes.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_MXC_UART

/* SATA Configs */
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48

/* MMC Configs */
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR      0

/* USB Configs */
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#define CONFIG_USBD_HS

/* Networking Configs */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME		"FEC"
#define CONFIG_FEC_MXC_PHYADDR		4
#define CONFIG_PHY_ATHEROS

/* Serial Flash */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_LOADADDR	0x12000000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=" CONFIG_BOOT_DIR "/uImage\0" \
	"uboot=u-boot.imx\0" \
	"fdt_file=" CONFIG_BOOT_DIR "/" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_addr=0x18000000\0" \
	"boot_fdt=yes\0" \
	"ip_dyn=yes\0" \
	"console=" CONSOLE_DEV "\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
	"sddev=0\0" \
	"emmcdev=1\0" \
	"partnum=1\0" \
	"loadcmd=" CONFIG_LOADCMD "\0" \
	"rfspart=" CONFIG_RFSPART "\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"update_sf_uboot=" \
		"if tftp $loadaddr $uboot; then " \
			"sf probe; " \
			"sf erase 0 0xC0000; " \
			"sf write $loadaddr 0x400 $filesize; " \
			"echo 'U-Boot upgraded. Please reset'; " \
		"fi\0" \
	"setargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/${rootdev} rw rootwait " CONFIG_EXTRA_BOOTARGS "\0" \
	"loadbootscript=" \
		"${loadcmd} ${dev} ${devnum}:${partnum} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from ${dev}:${devnum}:${partnum};" \
		" source\0" \
	"loadimage=" \
		"${loadcmd} ${dev} ${devnum}:${partnum} ${loadaddr} ${image}\0" \
	"loadfdt=${loadcmd} ${dev} ${devnum}:${partnum} ${fdt_addr} ${fdt_file}\0" \
	"tryboot=" \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loadimage; then " \
				"run doboot; " \
			"fi; " \
		"fi;\0" \
	"doboot=echo Booting from ${dev}:${devnum}:${partnum} ...; " \
		"run setargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootm; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootm; " \
		"fi;\0" \

#define CONFIG_BOOTCOMMAND \
	"usb start; " \
	"setenv dev usb; " \
	"setenv devnum 0; " \
	"setenv rootdev sda${rfspart}; " \
	"run tryboot; " \
	\
	"setenv dev mmc; " \
	"setenv rootdev mmcblk0p${rfspart}; " \
	\
	"setenv devnum ${sddev}; " \
	"if mmc dev ${devnum}; then " \
		"run tryboot; " \
	"fi; " \
	\
	"setenv devnum ${emmcdev}; " \
	"setenv rootdev mmcblk${emmcdev}p${rfspart}; " \
	"if mmc dev ${devnum}; then " \
		"run tryboot; " \
	"fi; " \
	\
	"bmode usb; " \

#define CONFIG_ARP_TIMEOUT     200UL

/* Miscellaneous configurable options */

#define CONFIG_SYS_MEMTEST_START       0x10000000
#define CONFIG_SYS_MEMTEST_END         0x10010000
#define CONFIG_SYS_MEMTEST_SCRATCH     0x10800000

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */

#define CONFIG_ENV_SIZE                 (8 * 1024)
#define CONFIG_ENV_OFFSET               (768 * 1024)
#define CONFIG_ENV_SECT_SIZE            (64 * 1024)

#define CONFIG_SYS_FSL_USDHC_NUM        3

/* Framebuffer */
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP
#endif

#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK         66000000

#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO      IMX_GPIO_NR(7, 12)
#define CONFIG_PCIE_IMX_POWER_GPIO      IMX_GPIO_NR(1, 5)
#endif

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED            100000
#define CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_MXC_I2C3

#endif	/* __ADVANTECH_DMSBA16_CONFIG_H */
