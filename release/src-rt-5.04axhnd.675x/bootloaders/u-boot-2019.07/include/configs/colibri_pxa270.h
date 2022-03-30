/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Toradex Colibri PXA270 configuration file
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 * Copyright (C) 2015-2016 Marcel Ziswiler <marcel@ziswiler.com>
 */

#ifndef	__CONFIG_H
#define	__CONFIG_H

/*
 * High Level Board Configuration Options
 */
#define	CONFIG_CPU_PXA27X		1	/* Marvell PXA270 CPU */
/* Avoid overwriting factory configuration block */
#define CONFIG_BOARD_SIZE_LIMIT		0x40000

/*
 * Environment settings
 */
#define	CONFIG_ENV_OVERWRITE
#define	CONFIG_SYS_MALLOC_LEN		(128 * 1024)
#define	CONFIG_ARCH_CPU_INIT
#define	CONFIG_BOOTCOMMAND						\
	"if fatload mmc 0 0xa0000000 uImage; then "			\
		"bootm 0xa0000000; "					\
	"fi; "								\
	"if usb reset && fatload usb 0 0xa0000000 uImage; then "	\
		"bootm 0xa0000000; "					\
	"fi; "								\
	"bootm 0xc0000;"
#define	CONFIG_TIMESTAMP
#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS

/*
 * Serial Console Configuration
 */

/*
 * Bootloader Components Configuration
 */

/* I2C support */
#ifdef CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PXA
#define CONFIG_PXA_STD_I2C
#define CONFIG_PXA_PWR_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#endif

/* LCD support */
#ifdef CONFIG_LCD
#define CONFIG_PXA_LCD
#define CONFIG_PXA_VGA
#define CONFIG_LCD_LOGO
#endif

/*
 * Networking Configuration
 */
#ifdef	CONFIG_CMD_NET

#define	CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x08000000
#define DM9000_IO			(CONFIG_DM9000_BASE)
#define DM9000_DATA			(CONFIG_DM9000_BASE + 4)
#define	CONFIG_NET_RETRY_COUNT		10

#define	CONFIG_BOOTP_BOOTFILESIZE
#endif

#define	CONFIG_SYS_DEVICE_NULLDEV	1

/*
 * Clock Configuration
 */
#define	CONFIG_SYS_CPUSPEED		0x290		/* 520MHz */

/*
 * DRAM Map
 */
#define	PHYS_SDRAM_1			0xa0000000	/* SDRAM Bank #1 */
#define	PHYS_SDRAM_1_SIZE		0x04000000	/* 64 MB */

#define	CONFIG_SYS_DRAM_BASE		0xa0000000	/* CS0 */
#define	CONFIG_SYS_DRAM_SIZE		0x04000000	/* 64 MB DRAM */

#define CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM */

#define	CONFIG_SYS_LOAD_ADDR		PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		0x5c010000

/*
 * NOR FLASH
 */
#ifdef	CONFIG_CMD_FLASH
#define	PHYS_FLASH_1			0x00000000	/* Flash Bank #1 */
#define	PHYS_FLASH_SIZE			0x02000000	/* 32 MB */
#define	CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define	CONFIG_SYS_FLASH_CFI_WIDTH      FLASH_CFI_32BIT

#define	CONFIG_SYS_MAX_FLASH_SECT	(4 + 255)
#define	CONFIG_SYS_MAX_FLASH_BANKS	1

#define	CONFIG_SYS_FLASH_ERASE_TOUT	(25 * CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_WRITE_TOUT	(25 * CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_LOCK_TOUT	(25 * CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_UNLOCK_TOUT	(25 * CONFIG_SYS_HZ)
#endif

#define	CONFIG_SYS_MONITOR_BASE		0x0
#define	CONFIG_SYS_MONITOR_LEN		0x40000

/* Skip factory configuration block */
#define	CONFIG_ENV_ADDR			\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN + 0x40000)
#define	CONFIG_ENV_SIZE			0x40000
#define	CONFIG_ENV_SECT_SIZE		0x40000

/*
 * GPIO settings
 */
#define	CONFIG_SYS_GPSR0_VAL	0x00000000
#define	CONFIG_SYS_GPSR1_VAL	0x00020000
#define	CONFIG_SYS_GPSR2_VAL	0x0002c000
#define	CONFIG_SYS_GPSR3_VAL	0x00000000

#define	CONFIG_SYS_GPCR0_VAL	0x00000000
#define	CONFIG_SYS_GPCR1_VAL	0x00000000
#define	CONFIG_SYS_GPCR2_VAL	0x00000000
#define	CONFIG_SYS_GPCR3_VAL	0x00000000

#define	CONFIG_SYS_GPDR0_VAL	0xc8008000
#define	CONFIG_SYS_GPDR1_VAL	0xfc02a981
#define	CONFIG_SYS_GPDR2_VAL	0x92c3ffff
#define	CONFIG_SYS_GPDR3_VAL	0x0061e804

#define	CONFIG_SYS_GAFR0_L_VAL	0x80100000
#define	CONFIG_SYS_GAFR0_U_VAL	0xa5c00010
#define	CONFIG_SYS_GAFR1_L_VAL	0x6992901a
#define	CONFIG_SYS_GAFR1_U_VAL	0xaaa50008
#define	CONFIG_SYS_GAFR2_L_VAL	0xaaaaaaaa
#define	CONFIG_SYS_GAFR2_U_VAL	0x4109a002
#define	CONFIG_SYS_GAFR3_L_VAL	0x54000310
#define	CONFIG_SYS_GAFR3_U_VAL	0x00005401

#define	CONFIG_SYS_PSSR_VAL	0x30

/*
 * Clock settings
 */
#define	CONFIG_SYS_CKEN		0x00500240
#define	CONFIG_SYS_CCCR		0x02000290

/*
 * Memory settings
 */
#define	CONFIG_SYS_MSC0_VAL	0x9ee1c5f2
#define	CONFIG_SYS_MSC1_VAL	0x9ee1f994
#define	CONFIG_SYS_MSC2_VAL	0x9ee19ee1
#define	CONFIG_SYS_MDCNFG_VAL	0x090009c9
#define	CONFIG_SYS_MDREFR_VAL	0x2003a031
#define	CONFIG_SYS_MDMRS_VAL	0x00220022
#define	CONFIG_SYS_FLYCNFG_VAL	0x00010001
#define	CONFIG_SYS_SXCNFG_VAL	0x40044004

/*
 * PCMCIA and CF Interfaces
 */
#define	CONFIG_SYS_MECR_VAL	0x00000000
#define	CONFIG_SYS_MCMEM0_VAL	0x00028307
#define	CONFIG_SYS_MCMEM1_VAL	0x00014307
#define	CONFIG_SYS_MCATT0_VAL	0x00038787
#define	CONFIG_SYS_MCATT1_VAL	0x0001c787
#define	CONFIG_SYS_MCIO0_VAL	0x0002830f
#define	CONFIG_SYS_MCIO1_VAL	0x0001430f

#include "pxa-common.h"

#endif /* __CONFIG_H */
