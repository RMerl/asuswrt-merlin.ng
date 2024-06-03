/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2010 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "../board/xilinx/microblaze-generic/xparameters.h"

/* MicroBlaze CPU */
#define	MICROBLAZE_V5		1

/* linear and spi flash memory */
#ifdef XILINX_FLASH_START
#define	FLASH
#undef	SPIFLASH
#undef	RAMENV	/* hold environment in flash */
#else
#undef	FLASH
#undef	SPIFLASH
#define	RAMENV	/* hold environment in RAM */
#endif

/* uart */
/* The following table includes the supported baudrates */
# define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* setting reset address */
/*#define	CONFIG_SYS_RESET_ADDRESS	CONFIG_SYS_TEXT_BASE*/

#define CONFIG_SYS_MALLOC_LEN	0xC0000

/* Stack location before relocation */
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_TEXT_BASE - \
					 CONFIG_SYS_MALLOC_F_LEN)

/*
 * CFI flash memory layout - Example
 * CONFIG_SYS_FLASH_BASE = 0x2200_0000;
 * CONFIG_SYS_FLASH_SIZE = 0x0080_0000;	  8MB
 *
 * SECT_SIZE = 0x20000;			128kB is one sector
 * CONFIG_ENV_SIZE = SECT_SIZE;		128kB environment store
 *
 * 0x2200_0000	CONFIG_SYS_FLASH_BASE
 *					FREE		256kB
 * 0x2204_0000	CONFIG_ENV_ADDR
 *					ENV_AREA	128kB
 * 0x2206_0000
 *					FREE
 * 0x2280_0000	CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SIZE
 *
 */

#ifdef FLASH
# define CONFIG_SYS_FLASH_BASE		XILINX_FLASH_START
# define CONFIG_SYS_FLASH_SIZE		XILINX_FLASH_SIZE
/* ?empty sector */
# define CONFIG_SYS_FLASH_EMPTY_INFO	1
/* max number of memory banks */
# define CONFIG_SYS_MAX_FLASH_BANKS	1
/* max number of sectors on one chip */
# define CONFIG_SYS_MAX_FLASH_SECT	512
/* hardware flash protection */
/* use buffered writes (20x faster) */
# ifdef	RAMENV
#  define CONFIG_ENV_SIZE	0x1000
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)

# else	/* FLASH && !RAMENV */
/* 128K(one sector) for env */
#  define CONFIG_ENV_SECT_SIZE	0x20000
#  define CONFIG_ENV_ADDR \
			(CONFIG_SYS_FLASH_BASE + (2 * CONFIG_ENV_SECT_SIZE))
#  define CONFIG_ENV_SIZE	0x20000
# endif /* FLASH && !RAMBOOT */
#else /* !FLASH */

#ifdef SPIFLASH
# ifdef	RAMENV
#  define CONFIG_ENV_SIZE	0x1000
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)

# else	/* SPIFLASH && !RAMENV */
/* 128K(two sectors) for env */
#  define CONFIG_ENV_SECT_SIZE	0x10000
#  define CONFIG_ENV_SIZE	(2 * CONFIG_ENV_SECT_SIZE)
/* Warning: adjust the offset in respect of other flash content and size */
#  define CONFIG_ENV_OFFSET	(128 * CONFIG_ENV_SECT_SIZE) /* at 8MB */
# endif /* SPIFLASH && !RAMBOOT */
#else /* !SPIFLASH */

/* ENV in RAM */
# define CONFIG_ENV_SIZE	0x1000
# define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)
#endif /* !SPIFLASH */
#endif /* !FLASH */

#if defined(XILINX_USE_ICACHE)
# define CONFIG_ICACHE
#else
# undef CONFIG_ICACHE
#endif

#if defined(XILINX_USE_DCACHE)
# define CONFIG_DCACHE
#else
# undef CONFIG_DCACHE
#endif

#ifndef XILINX_DCACHE_BYTE_SIZE
#define XILINX_DCACHE_BYTE_SIZE	32768
#endif

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

#if defined(CONFIG_MTD_PARTITIONS)
/* MTD partitions */

/* default mtd partition table */
#endif

/* size of console buffer */
#define	CONFIG_SYS_CBSIZE	512
/* max number of command args */
#define	CONFIG_SYS_MAXARGS	15
/* default load address */
#define	CONFIG_SYS_LOAD_ADDR	0

#define	CONFIG_HOSTNAME		"microblaze-generic"
#define	CONFIG_BOOTCOMMAND	"base 0;tftp 11000000 image.img;bootm"

/* architecture dependent code */
#define	CONFIG_SYS_USR_EXCEP	/* user exception */

#define	CONFIG_PREBOOT	"echo U-BOOT for ${hostname};setenv preboot;echo"

#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define	CONFIG_EXTRA_ENV_SETTINGS	"unlock=yes\0" \
					"nor0=flash-0\0"\
					"mtdparts=mtdparts=flash-0:"\
					"256k(u-boot),256k(env),3m(kernel),"\
					"1m(romfs),1m(cramfs),-(jffs2)\0"\
					"nc=setenv stdout nc;"\
					"setenv stdin nc\0" \
					"serial=setenv stdout serial;"\
					"setenv stdin serial\0"
#endif

/* Enable flat device tree support */
#define CONFIG_LMB		1

#if defined(CONFIG_XILINX_AXIEMAC)
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN	1
#endif

/* SPL part */

#ifdef CONFIG_SYS_FLASH_BASE
# define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_FLASH_BASE
#endif

/* for booting directly linux */

#define CONFIG_SYS_FDT_BASE		(CONFIG_SYS_FLASH_BASE + \
					 0x40000)
#define CONFIG_SYS_FDT_SIZE		(16 << 10)
#define CONFIG_SYS_SPL_ARGS_ADDR	(CONFIG_SYS_TEXT_BASE + \
					 0x1000000)

/* SP location before relocation, must use scratch RAM */
/* BRAM start */
#define CONFIG_SYS_INIT_RAM_ADDR	0x0
/* BRAM size - will be generated */
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

# define CONFIG_SPL_STACK_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 CONFIG_SYS_MALLOC_F_LEN)

/* Just for sure that there is a space for stack */
#define CONFIG_SPL_STACK_SIZE		0x100

#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE

#define CONFIG_SPL_MAX_FOOTPRINT	(CONFIG_SYS_INIT_RAM_SIZE - \
					 CONFIG_SYS_INIT_RAM_ADDR - \
					 CONFIG_SYS_MALLOC_F_LEN - \
					 CONFIG_SPL_STACK_SIZE)

#endif	/* __CONFIG_H */
