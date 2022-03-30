/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * This file contains the configuration parameters for qemu-mips64 target.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_QEMU_MIPS

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"panic=1\0"						\
	"bootfile=/tftpboot/vmlinux\0"					\
	"load=tftp ffffffff80500000 ${u-boot}\0"			\
	""

#define CONFIG_BOOTCOMMAND	"bootp;bootelf"

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

#define CONFIG_DRIVER_NE2000
#define CONFIG_DRIVER_NE2000_BASE	0xffffffffb4000300

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		115200
#define CONFIG_SYS_NS16550_COM1		0xffffffffb40003f8

#ifdef CONFIG_SYS_BIG_ENDIAN
#define CONFIG_IDE_SWAP_IO
#endif

#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x1f0
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x170
#define CONFIG_SYS_ATA_DATA_OFFSET	0
#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_BASE_ADDR	0xffffffffb4000000

#define CONFIG_SYS_IDE_MAXDEVICE	4

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_MALLOC_LEN		(256 << 10)

#define CONFIG_SYS_BOOTPARAMS_LEN	128*1024

#define CONFIG_SYS_MHZ			132

#define CONFIG_SYS_MIPS_TIMER_FREQ	(CONFIG_SYS_MHZ * 1000000)

/* Cached addr */
#define CONFIG_SYS_SDRAM_BASE		0xffffffff80000000

/* default load address */
#define CONFIG_SYS_LOAD_ADDR		0xffffffff81000000

#define CONFIG_SYS_MEMTEST_START	0xffffffff80100000
#define CONFIG_SYS_MEMTEST_END		0xffffffff80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/* The following #defines are needed to get flash environment right */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

/* We boot from this flash, selected with dip switch */
#define CONFIG_SYS_FLASH_BASE		0xffffffffbfc00000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	128

/* Address and size of Primary Environment Sector */
#define CONFIG_ENV_SIZE		0x8000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + (4 << 20) - CONFIG_ENV_SIZE)

#define CONFIG_ENV_OVERWRITE	1

#define MEM_SIZE		128

#endif /* __CONFIG_H */
