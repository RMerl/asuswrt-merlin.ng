/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

#include <asm/ibmpc.h>

#ifndef __CONFIG_X86_COMMON_H
#define __CONFIG_X86_COMMON_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SHOW_BOOT_PROGRESS
#define CONFIG_PHYSMEM

#define CONFIG_LMB

#undef CONFIG_ZLIB
#undef CONFIG_GZIP
#define CONFIG_SYS_BOOTM_LEN		(16 << 20)

/* SATA AHCI storage */
#ifdef CONFIG_SCSI_AHCI
#define CONFIG_LBA48
#define CONFIG_SYS_64BIT_LBA

#endif

/* Generic TPM interfaced through LPC bus */
#define CONFIG_TPM_TIS_BASE_ADDRESS        0xfed40000

/*-----------------------------------------------------------------------
 * Real Time Clock Configuration
 */
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS	0
#define CONFIG_SYS_ISA_IO      CONFIG_SYS_ISA_IO_BASE_ADDRESS

/*-----------------------------------------------------------------------
 * Serial Configuration
 */
#define CONFIG_SYS_NS16550_PORT_MAPPED

/*-----------------------------------------------------------------------
 * Command line configuration.
 */

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	\
	"ext2load scsi 0:3 01000000 /boot/vmlinuz; zboot 01000000"
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE			115200
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE			512

#define CONFIG_SYS_MEMTEST_START		0x00100000
#define CONFIG_SYS_MEMTEST_END			0x01000000
#define CONFIG_SYS_LOAD_ADDR			0x20000000

/*-----------------------------------------------------------------------
 * CPU Features
 */

#define CONFIG_SYS_STACK_SIZE			(32 * 1024)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MALLOC_LEN			0x200000

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*-----------------------------------------------------------------------
 * Environment configuration
 */
#define CONFIG_ENV_SIZE			0x01000

/*-----------------------------------------------------------------------
 * PCI configuration
 */
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

/*-----------------------------------------------------------------------
 * USB configuration
 */

#define CONFIG_TFTP_TSIZE
#define CONFIG_BOOTP_BOOTFILESIZE

/* Default environment */
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_HOSTNAME		"x86"
#define CONFIG_BOOTFILE		"bzImage"
#define CONFIG_LOADADDR		0x1000000
#define CONFIG_RAMDISK_ADDR	0x4000000
#if defined(CONFIG_GENERATE_ACPI_TABLE) || defined(CONFIG_EFI_STUB)
#define CONFIG_OTHBOOTARGS	"othbootargs=\0"
#else
#define CONFIG_OTHBOOTARGS	"othbootargs=acpi=off\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS			\
	CONFIG_STD_DEVICES_SETTINGS			\
	"pciconfighost=1\0"				\
	"netdev=eth0\0"					\
	"consoledev=ttyS0\0"				\
	CONFIG_OTHBOOTARGS				\
	"ramdiskaddr=0x4000000\0"			\
	"ramdiskfile=initramfs.gz\0"

#define CONFIG_RAMBOOTCOMMAND				\
	"setenv bootargs root=/dev/ram rw "		\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftpboot $loadaddr $bootfile;"			\
	"tftpboot $ramdiskaddr $ramdiskfile;"		\
	"zboot $loadaddr 0 $ramdiskaddr $filesize"

#define CONFIG_NFSBOOTCOMMAND				\
	"setenv bootargs root=/dev/nfs rw "		\
	"nfsroot=$serverip:$rootpath "			\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftpboot $loadaddr $bootfile;"			\
	"zboot $loadaddr"


#endif	/* __CONFIG_H */
