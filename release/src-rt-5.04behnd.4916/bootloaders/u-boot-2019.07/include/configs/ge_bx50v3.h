/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Timesys Corporation
 * Copyright (C) 2015 General Electric Company
 * Copyright (C) 2014 Advantech
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the GE MX6Q Bx50v3 boards.
 */

#ifndef __GE_BX50V3_CONFIG_H
#define __GE_BX50V3_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#define CONFIG_BOARD_NAME	"General Electric Bx50v3"

#define CONFIG_MXC_UART_BASE	UART3_BASE
#define CONSOLE_DEV	"ttymxc2"

#include "mx6_common.h"
#include <linux/sizes.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_WATCHDOG_TIMEOUT_MSECS 6000

#define CONFIG_MXC_UART

/* SATA Configs */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#endif

/* USB Configs */
#ifdef CONFIG_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#define CONFIG_USBD_HS
#define CONFIG_USB_GADGET_MASS_STORAGE
#endif

/* Networking Configs */
#ifdef CONFIG_NET
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME		"FEC"
#define CONFIG_FEC_MXC_PHYADDR		4
#define CONFIG_PHY_ATHEROS
#endif

/* Serial Flash */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_LOADADDR	0x12000000

#ifdef CONFIG_NFS_CMD
#define NETWORKBOOT \
        "setnetworkboot=" \
                "setenv ipaddr 172.16.2.10; setenv serverip 172.16.2.20; " \
                "setenv gatewayip 172.16.2.20; setenv nfsserver 172.16.2.20; " \
                "setenv netmask 255.255.255.0; setenv ethaddr ca:fe:de:ca:f0:11; " \
                "setenv bootargs root=/dev/nfs nfsroot=${nfsserver}:/srv/nfs/,v3,tcp rw rootwait" \
                "setenv bootargs $bootargs ip=${ipaddr}:${nfsserver}:${gatewayip}:${netmask}::eth0:off " \
                "setenv bootargs $bootargs cma=128M bootcause=POR console=${console} ${videoargs} " \
                "setenv bootargs $bootargs systemd.mask=helix-network-defaults.service " \
                "setenv bootargs $bootargs watchdog.handle_boot_enabled=1\0" \
        "networkboot=" \
                "run setnetworkboot; " \
                "nfs ${loadaddr} /srv/nfs/fitImage; " \
                "bootm ${loadaddr}#conf@${confidx}\0" \

#define CONFIG_NETWORKBOOTCOMMAND \
	"run networkboot; " \

#else
#define NETWORKBOOT \

#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	NETWORKBOOT \
	"bootcause=POR\0" \
	"image=/boot/fitImage\0" \
	"fdt_high=0xffffffff\0" \
	"dev=mmc\0" \
	"devnum=2\0" \
	"rootdev=mmcblk0p\0" \
	"quiet=quiet loglevel=0\0" \
	"console=" CONSOLE_DEV "\0" \
	"setargs=setenv bootargs root=/dev/${rootdev}${partnum} " \
		"ro rootwait cma=128M " \
		"bootcause=${bootcause} " \
		"${quiet} console=${console} ${rtc_status} " \
		"${videoargs}" "\0" \
	"doquiet=" \
		"if ext2load ${dev} ${devnum}:5 0x7000A000 /boot/console; " \
			"then setenv quiet; fi\0" \
	"hasfirstboot=" \
		"ext2load ${dev} ${devnum}:${partnum} 0x7000A000 " \
		"/boot/bootcause/firstboot\0" \
	"swappartitions=" \
		"setexpr partnum 3 - ${partnum}\0" \
	"failbootcmd=" \
		"bx50_backlight_enable; " \
		"msg=\"Monitor failed to start.  Try again, or contact GE Service for support.\"; " \
		"echo $msg; " \
		"setenv stdout vga; " \
		"echo \"\n\n\n\n    \" $msg; " \
		"setenv stdout serial; " \
		"mw.b 0x7000A000 0xbc; " \
		"mw.b 0x7000A001 0x00; " \
		"ext4write ${dev} ${devnum}:5 0x7000A000 /boot/failures 2\0" \
	"altbootcmd=" \
		"run doquiet; " \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run hasfirstboot || setenv partnum 0; " \
		"if test ${partnum} != 0; then " \
			"setenv bootcause REVERT; " \
			"run swappartitions loadimage doboot; " \
		"fi; " \
		"run failbootcmd\0" \
	"loadimage=" \
		"ext2load ${dev} ${devnum}:${partnum} ${loadaddr} ${image}\0" \
	"doboot=" \
		"echo Booting from ${dev}:${devnum}:${partnum} ...; " \
		"run setargs; " \
		"bootm ${loadaddr}#conf@${confidx}\0" \
	"tryboot=" \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run loadimage || run swappartitions && run loadimage || " \
		"setenv partnum 0 && echo MISSING IMAGE;" \
		"run doboot; " \
		"run failbootcmd\0" \

#define CONFIG_MMCBOOTCOMMAND \
	"if mmc dev ${devnum}; then " \
		"run doquiet; " \
		"run tryboot; " \
	"fi; " \

#define CONFIG_USBBOOTCOMMAND \
	"echo Unsupported; " \

#ifdef CONFIG_NFS_CMD
#define CONFIG_BOOTCOMMAND CONFIG_NETWORKBOOTCOMMAND
#elif CONFIG_CMD_USB
#define CONFIG_BOOTCOMMAND CONFIG_USBBOOTCOMMAND
#else
#define CONFIG_BOOTCOMMAND CONFIG_MMCBOOTCOMMAND
#endif


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

/* environment organization */
#define CONFIG_ENV_SIZE		(8 * 1024)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)

#define CONFIG_SYS_FSL_USDHC_NUM	3

/* Framebuffer */
#define CONFIG_HIDE_LOGO_VERSION
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP
#define CONFIG_CMD_BMP

#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK	66000000

#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(7, 12)
#define CONFIG_PCIE_IMX_POWER_GPIO	IMX_GPIO_NR(1, 5)

#define CONFIG_RTC_RX8010SJ
#define CONFIG_SYS_RTC_BUS_NUM 2
#define CONFIG_SYS_I2C_RTC_ADDR	0x32

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		  100000
#define CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_MXC_I2C3

#define CONFIG_SYS_NUM_I2C_BUSES        11
#define CONFIG_SYS_I2C_MAX_HOPS         1
#define CONFIG_SYS_I2C_BUSES	{	{0, {I2C_NULL_HOP} }, \
					{1, {I2C_NULL_HOP} }, \
					{2, {I2C_NULL_HOP} }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 0} } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 1} } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 2} } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 3} } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 4} } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 5} } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 6} } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 7} } }, \
				}

#define CONFIG_BCH

#endif	/* __GE_BX50V3_CONFIG_H */
