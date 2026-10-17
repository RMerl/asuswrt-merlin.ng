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

/* Memory sizes */

/* OCRAM at 0x0 ; 32kB on MX23 ; 128kB on MX28 */
#define CFG_SYS_INIT_RAM_ADDR	0x00000000
#if defined(CONFIG_MX23)
#define CFG_SYS_INIT_RAM_SIZE	(32 * 1024)
#elif defined(CONFIG_MX28)
#define CFG_SYS_INIT_RAM_SIZE	(128 * 1024)
#endif

/* Point initial SP in SRAM so SPL can use it too. */

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

/*
 * Drivers
 */
/*
 * DUART Serial Driver.
 * Conflicts with AUART driver which can be set by board.
 */
#define CFG_PL011_CLOCK		24000000
#define CFG_PL01x_PORTS		{ (void *)MXS_UARTDBG_BASE }
/* Default baudrate can be overridden by board! */

/* NAND */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		0x60000000
#endif

#endif	/* __CONFIGS_MXS_H__ */
