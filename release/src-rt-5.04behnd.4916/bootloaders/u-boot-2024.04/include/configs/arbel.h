/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#ifndef __CONFIG_ARBEL_H
#define __CONFIG_ARBEL_H

#define CFG_SYS_SDRAM_BASE		0x0
#define CFG_SYS_BOOTMAPSZ		(128 << 20)
#define CFG_SYS_BOOTM_LEN		(20 << 20)
#define CFG_SYS_INIT_RAM_ADDR	CFG_SYS_SDRAM_BASE
#define CFG_SYS_INIT_RAM_SIZE	0x8000

#define CFG_SYS_BAUDRATE_TABLE	\
	{ 9600, 14400, 19200, 38400, 57600, 115200, 230400, 380400, 460800, 921600 }


/* Default environemnt variables */
#define CFG_EXTRA_ENV_SETTINGS   "uimage_flash_addr=80400000\0"   \
		"stdin=serial\0"   \
		"stdout=serial\0"   \
		"stderr=serial\0"    \
		"ethact=gmac1\0"   \
		"autostart=no\0"   \
		"ethaddr=00:00:F7:A0:00:FC\0"    \
		"eth1addr=00:00:F7:A0:00:FD\0"   \
		"eth2addr=00:00:F7:A0:00:FE\0"    \
		"eth3addr=00:00:F7:A0:00:FF\0"    \
		"serverip=192.168.0.1\0"    \
		"ipaddr=192.168.0.2\0"    \
		"romboot=echo Booting Kernel from flash at 0x${uimage_flash_addr}; " \
		"echo Using bootargs: ${bootargs};bootm ${uimage_flash_addr}\0" \
		"earlycon=uart8250,mmio32,0xf0000000\0" \
		"console=ttyS0,115200n8\0" \
		"common_bootargs=setenv bootargs earlycon=${earlycon} root=/dev/ram " \
		"console=${console} ramdisk_size=48000\0" \
		"\0"

#endif
