/*
 * Copyright (C) 2013 Marek Vasut <marex@denx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __CONFIGS_MXS_H__
#define __CONFIGS_MXS_H__

/*
 * Includes
 */

#if defined(CONFIG_MX23) && defined(CONFIG_MX28)
#error Select either CONFIG_MX23 or CONFIG_MX28 , never both!
#elif !defined(CONFIG_MX23) && !defined(CONFIG_MX28)
#error Select one of CONFIG_MX23 or CONFIG_MX28 !
#endif

#include <asm/arch/regs-base.h>

#if defined(CONFIG_MX23)
#include <asm/arch/iomux-mx23.h>
#elif defined(CONFIG_MX28)
#include <asm/arch/iomux-mx28.h>
#endif

/*
 * CPU specifics
 */

/* Startup hooks */

/* SPL */
#ifndef CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH	"arch/arm/cpu/arm926ejs/mxs"
#endif

/* Memory sizes */
#define CONFIG_SYS_MALLOC_LEN		0x00400000	/* 4 MB for malloc */
#define CONFIG_SYS_MEMTEST_START	0x40000000	/* Memtest start adr */
#define CONFIG_SYS_MEMTEST_END		0x40400000	/* 4 MB RAM test */

/* OCRAM at 0x0 ; 32kB on MX23 ; 128kB on MX28 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00000000
#if defined(CONFIG_MX23)
#define CONFIG_SYS_INIT_RAM_SIZE	(32 * 1024)
#elif defined(CONFIG_MX28)
#define CONFIG_SYS_INIT_RAM_SIZE	(128 * 1024)
#endif

/* Point initial SP in SRAM so SPL can use it too. */
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/*
 * We need to sacrifice first 4 bytes of RAM here to avoid triggering some
 * strange BUG in ROM corrupting first 4 bytes of RAM when loading U-Boot
 * binary. In case there was more of this mess, 0x100 bytes are skipped.
 *
 * In case of a HAB boot, we cannot for some weird reason use the first 4KiB
 * of DRAM when loading. Moreover, we use the first 4 KiB for IVT and CST
 * blocks, thus U-Boot starts at offset +8 KiB of DRAM start.
 *
 * As for the SPL, we must avoid the first 4 KiB as well, but we load the
 * IVT and CST to 0x8000, so we don't need to waste the subsequent 4 KiB.
 */

/* U-Boot general configuration */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */

/* Booting Linux */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS

/*
 * Drivers
 */

/* APBH DMA */

/* GPIO */
#define CONFIG_MXS_GPIO

/*
 * DUART Serial Driver.
 * Conflicts with AUART driver which can be set by board.
 */
#define CONFIG_PL011_CLOCK		24000000
#define CONFIG_PL01x_PORTS		{ (void *)MXS_UARTDBG_BASE }
/* Default baudrate can be overridden by board! */

/* FEC Ethernet on SoC */
#ifdef CONFIG_FEC_MXC
#ifndef CONFIG_ETHPRIME
#define CONFIG_ETHPRIME			"FEC0"
#endif
#ifndef CONFIG_FEC_XCV_TYPE
#define CONFIG_FEC_XCV_TYPE		RMII
#endif
#endif

/* LCD */
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_MXS
#endif

/* NAND */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x60000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#endif

/* OCOTP */
#ifdef CONFIG_CMD_FUSE
#define CONFIG_MXS_OCOTP
#endif

/* SPI */
#ifdef CONFIG_CMD_SPI
#define CONFIG_SPI_HALF_DUPLEX
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI_MXS
#define CONFIG_EHCI_IS_TDI
#endif

#endif	/* __CONFIGS_MXS_H__ */
