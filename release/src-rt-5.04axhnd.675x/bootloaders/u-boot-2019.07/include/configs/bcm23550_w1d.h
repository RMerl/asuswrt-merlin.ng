/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Broadcom Corporation.
 */

#ifndef __BCM23550_W1D_H
#define __BCM23550_W1D_H

#include <linux/sizes.h>
#include <asm/arch/sysmap.h>

/* CPU, chip, mach, etc */
#define CONFIG_KONA
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_KONA_RESET_S

/*
 * Memory configuration
 */

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_SDRAM_SIZE		0x20000000

#define CONFIG_SYS_MALLOC_LEN		SZ_4M	/* see armv7/start.S. */

/* GPIO Driver */
#define CONFIG_KONA_GPIO

/* MMC/SD Driver */
#define CONFIG_SYS_SDIO_BASE0 SDIO1_BASE_ADDR
#define CONFIG_SYS_SDIO_BASE1 SDIO2_BASE_ADDR
#define CONFIG_SYS_SDIO_BASE2 SDIO3_BASE_ADDR
#define CONFIG_SYS_SDIO_BASE3 SDIO4_BASE_ADDR
#define CONFIG_SYS_SDIO0_MAX_CLK 48000000
#define CONFIG_SYS_SDIO1_MAX_CLK 48000000
#define CONFIG_SYS_SDIO2_MAX_CLK 48000000
#define CONFIG_SYS_SDIO3_MAX_CLK 48000000
#define CONFIG_SYS_SDIO0 "sdio1"
#define CONFIG_SYS_SDIO1 "sdio2"
#define CONFIG_SYS_SDIO2 "sdio3"
#define CONFIG_SYS_SDIO3 "sdio4"

/* I2C Driver */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_KONA
#define CONFIG_SYS_SPD_BUS_NUM	3	/* Start with PMU bus */
#define CONFIG_SYS_MAX_I2C_BUS	4
#define CONFIG_SYS_I2C_BASE0	BSC1_BASE_ADDR
#define CONFIG_SYS_I2C_BASE1	BSC2_BASE_ADDR
#define CONFIG_SYS_I2C_BASE2	BSC3_BASE_ADDR
#define CONFIG_SYS_I2C_BASE3	PMU_BSC_BASE_ADDR

/* Timer Driver */
#define CONFIG_SYS_TIMER_RATE		32000
#define CONFIG_SYS_TIMER_COUNTER	(TIMER_BASE_ADDR + 4) /* STCLO offset */

/* Init functions */

/* Some commands use this as the default load address */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

/* No mtest functions as recommended */

/*
 * This is the initial SP which is used only briefly for relocating the u-boot
 * image to the top of SDRAM. After relocation u-boot moves the stack to the
 * proper place.
 */
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE

/* Serial Info */
#define CONFIG_SYS_NS16550_SERIAL
/* Post pad 3 bytes after each reg addr */
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		13000000
#define CONFIG_SYS_NS16550_COM1		0x3e000000

/* must fit into GPT:u-boot-env partition */
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_OFFSET		(0x00011a00 * 512)
#define CONFIG_ENV_SIZE			(8 * 512)

/* console configuration */
#define CONFIG_SYS_CBSIZE		1024	/* Console buffer size */
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * One partition type must be defined for part.c
 * This is necessary for the fatls command to work on an SD card
 * for example.
 */

/* version string, parser, etc */

#define CONFIG_MX_CYCLIC

/* Initial upstream - boot to cmd prompt only */
#define CONFIG_BOOTCOMMAND		""

#define CONFIG_USBID_ADDR		0x34052c46

#define CONFIG_SYS_L2CACHE_OFF

#endif /* __BCM23550_W1D_H */
