/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 * Jason Liu <r64343@freescale.com>
 *
 * Configuration settings for Freescale MX53 low cost board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

#define CONSOLE_DEV	"ttymxc0"

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SYS_FSL_CLK

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

#define CONFIG_WATCHDOG_TIMEOUT_MSECS 8000

#define CONFIG_BOARD_LATE_INIT
#define CONFIG_REVISION_TAG

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

/* Eth Configs */

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR	0x1F

/* USB Configs */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_MCS7830
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#define CONFIG_SYS_RTC_BUS_NUM		2
#define CONFIG_SYS_I2C_RTC_ADDR	0x30

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */

/* PMIC Controller */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_DIALOG_POWER
#define CONFIG_POWER_FSL
#define CONFIG_POWER_FSL_MC13892
#define CONFIG_SYS_DIALOG_PMIC_I2C_ADDR	0x48
#define CONFIG_SYS_FSL_PMIC_I2C_ADDR	0x8

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200

/* Command definition */

#define CONFIG_ETHPRIME		"FEC0"

#define CONFIG_LOADADDR		0x72000000	/* loadaddr env var */

#define PPD_CONFIG_NFS \
	"nfsserver=192.168.252.95\0" \
	"gatewayip=192.168.252.95\0" \
	"netmask=255.255.255.0\0" \
	"ipaddr=192.168.252.99\0" \
	"kernsize=0x2000\0" \
	"use_dhcp=0\0" \
	"nfsroot=/opt/springdale/rd\0" \
	"bootargs_nfs=setenv bootargs ${bootargs} root=/dev/nfs " \
		"${kern_ipconf} nfsroot=${nfsserver}:${nfsroot},v3,tcp rw\0" \
	"choose_ip=if test $use_dhcp = 1; then setenv kern_ipconf ip=dhcp; " \
		"setenv getcmd dhcp; else setenv kern_ipconf " \
		"ip=${ipaddr}:${nfsserver}:${gatewayip}:${netmask}::eth0:off; " \
		"setenv getcmd tftp; fi\0" \
	"nfs=run choose_ip setargs bootargs_nfs; ${getcmd} ${loadaddr} " \
		"${nfsserver}:${image}; bootm ${loadaddr}\0" \

#define CONFIG_EXTRA_ENV_SETTINGS \
	PPD_CONFIG_NFS \
	"image=/boot/fitImage\0" \
	"fdt_high=0xffffffff\0" \
	"dev=mmc\0" \
	"devnum=2\0" \
	"rootdev=mmcblk0p\0" \
	"quiet=quiet loglevel=0\0" \
	"console=" CONSOLE_DEV "\0" \
	"lvds=ldb\0" \
	"setargs=setenv bootargs ${lvds} jtag=on mem=2G " \
		"vt.global_cursor_default=0 bootcause=${bootcause} ${quiet} " \
		"console=${console} ${rtc_status}\0" \
	"bootargs_emmc=setenv bootargs root=/dev/${rootdev}${partnum} ro " \
		"rootwait ${bootargs}\0" \
	"doquiet=if ext2load ${dev} ${devnum}:5 0x7000A000 /boot/console; " \
		"then setenv quiet; fi\0" \
	"hasfirstboot=ext2load ${dev} ${devnum}:${partnum} 0x7000A000 " \
		"/boot/bootcause/firstboot\0" \
	"swappartitions=setexpr partnum 3 - ${partnum}\0" \
	"failbootcmd=" \
		"ppd_lcd_enable; " \
		"msg=\"Monitor failed to start.  " \
			"Try again, or contact GE Service for support.\"; " \
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
		"run bootargs_emmc; " \
		"bootm ${loadaddr}\0" \
	"tryboot=" \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run loadimage || run swappartitions && run loadimage || " \
			"setenv partnum 0 && echo MISSING IMAGE;" \
		"run doboot; " \
		"run failbootcmd\0" \
	"video-mode=" \
		"lcd:800x480-24@60,monitor=lcd\0" \

#define CONFIG_MMCBOOTCOMMAND \
	"if mmc dev ${devnum}; then " \
		"run doquiet; " \
		"run tryboot; " \
	"fi; " \

#define CONFIG_BOOTCOMMAND CONFIG_MMCBOOTCOMMAND

#define CONFIG_ARP_TIMEOUT	200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */

#define CONFIG_SYS_MAXARGS	48	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START       0x70000000
#define CONFIG_SYS_MEMTEST_END         0x70010000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* Physical Memory Map */
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(gd->bd->bi_dram[0].size)
#define PHYS_SDRAM_2			CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE		(gd->bd->bi_dram[1].size)
#define PHYS_SDRAM_SIZE			(gd->ram_size)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_ENV_OFFSET      (12 * 64 * 1024)
#define CONFIG_ENV_SIZE        (10 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV 0

#define CONFIG_CMD_FUSE
#define CONFIG_FSL_IIM

#define CONFIG_SYS_I2C_SPEED	100000

/* I2C1 */
#define CONFIG_SYS_NUM_I2C_BUSES	9
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_I2C_BUSES	{	{0, {I2C_NULL_HOP} }, \
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

/* Backlight Control */
#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK 66666000

#endif				/* __CONFIG_H */
