/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * LG Optimus Black codename sniper config
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/*
 * Clocks
 */

#define CFG_SYS_TIMERBASE	OMAP34XX_GPT2

#define V_NS16550_CLK		48000000
#define V_OSCK			26000000
#define V_SCLK			(V_OSCK >> 1)

/*
 * DRAM
 */

#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * Memory
 */

#define CFG_SYS_SDRAM_BASE		0x80000000

/*
 * I2C
 */

#define CFG_I2C_MULTI_BUS

/*
 * Input
 */

/*
 * SPL
 */

/*
 * Serial
 */

#define CFG_SYS_NS16550_CLK		V_NS16550_CLK
#define CFG_SYS_NS16550_COM3		OMAP34XX_UART3

#define CFG_SYS_BAUDRATE_TABLE	{ 4800, 9600, 19200, 38400, 57600, \
					  115200 }

/*
 * Environment
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x82000000\0" \
	"loadaddr=0x82000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"fdtaddr=0x88000000\0" \
	"ramdisk_addr_r=0x88080000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"scriptaddr=0x80000000\0" \
	"bootm_size=0x10000000\0" \
	"boot_mmc_dev=0\0" \
	"kernel_mmc_part=3\0" \
	"recovery_mmc_part=4\0" \
	"fdtfile=omap3-sniper.dtb\0" \
	"bootfile=/boot/extlinux/extlinux.conf\0" \
	"bootargs=console=ttyO2,115200 vram=5M,0x9FA00000 omapfb.vram=0:5M\0"

/*
 * Boot
 */

#endif
