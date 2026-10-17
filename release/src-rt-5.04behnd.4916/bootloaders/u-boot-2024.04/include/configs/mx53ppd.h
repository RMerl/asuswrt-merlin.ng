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

/* USB Configs */
#define CFG_MXC_USB_PORT	1
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS	0

/* Command definition */

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

#define CFG_EXTRA_ENV_SETTINGS \
	PPD_CONFIG_NFS \
	"image=/boot/fitImage\0" \
	"dev=mmc\0" \
	"devnum=2\0" \
	"rootdev=mmcblk0p\0" \
	"quiet=quiet loglevel=0\0" \
	"lvds=ldb\0" \
	"setargs=setenv bootargs ${lvds} jtag=on mem=2G " \
		"vt.global_cursor_default=0 bootcause=${bootcause} ${quiet}\0" \
	"bootargs_emmc=setenv bootargs root=/dev/${rootdev}${partnum} ro " \
		"rootwait ${bootargs}\0" \
	"doquiet=" \
		"if ext2load ${dev} ${devnum}:5 0x7000A000 /boot/console; " \
			"then setenv quiet; fi\0" \
	"hasfirstboot=" \
		"test -e ${dev} ${devnum}:${partnum} /boot/bootcause/firstboot\0" \
	"swappartitions=" \
		"setexpr partnum 3 - ${partnum}\0" \
	"failbootcmd=" \
		"cls; " \
		"setcurs 5 4; " \
		"lcdputs \"Monitor failed to start. " \
		"Try again, or contact GE Service for support.\"; " \
		"bootcount reset; " \
		"while true; do sleep 1; done; \0" \
	"altbootcmd=" \
		"run doquiet; " \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run hasfirstboot || setenv partnum 0; " \
		"if test ${partnum} != 0; then " \
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

/* Miscellaneous configurable options */

#define CFG_SYS_BOOTMAPSZ (256 << 20)     /* 256M */

/* Physical Memory Map */
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(gd->bd->bi_dram[0].size)
#define PHYS_SDRAM_2			CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE		(gd->bd->bi_dram[1].size)
#define PHYS_SDRAM_SIZE			(gd->ram_size)

#define CFG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CFG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CFG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

/* FLASH and environment organization */

#endif				/* __CONFIG_H */
