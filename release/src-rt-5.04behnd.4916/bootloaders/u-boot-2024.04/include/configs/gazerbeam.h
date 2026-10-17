/*
 * (C) Copyright 2015
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * DDR Setup
 */
#define CFG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */
/* TODO: Check: Can this be unified with CFG_SYS_SDRAM_BASE? */
#define CFG_SYS_DDR_SDRAM_BASE	CFG_SYS_SDRAM_BASE

/*
 * Memory test
 * TODO: Migrate!
 */

/*
 * The reserved memory
 */

/*
 * Initial RAM Base Address Setup
 */
#define CFG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CFG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM */

/*
 * FLASH on the Local Bus
 */
#define CFG_SYS_FLASH_BASE		0xFE000000 /* FLASH base address */
#define CFG_SYS_FLASH_SIZE		8 /* FLASH size is up to 8M */

#define CFG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */

/*
 * Environment Configuration
 */

#define CFG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS1\0"						\
	"u-boot=u-boot.bin\0"						\
	"kernel_addr=1000000\0"					\
	"fdt_addr=C00000\0"						\
	"fdtfile=hrcon.dtb\0"				\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};era " __stringify(CONFIG_SYS_MONITOR_BASE)\
		" +${filesize};cp.b ${fileaddr} "			\
		__stringify(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"	\
	"upd=run load update\0"						\

#endif	/* __CONFIG_H */
