/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright (C) 2015-2019 Altera Corporation <www.altera.com>
 */

#ifndef __CONFIG_SOCFGPA_ARRIA10_H__
#define __CONFIG_SOCFGPA_ARRIA10_H__

#include <asm/arch/base_addr_a10.h>

/* Booting Linux */
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/*
 * U-Boot general configurations
 */

/* Memory configurations  */
#define PHYS_SDRAM_1_SIZE		0x40000000

/*
 * Serial / UART configurations
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_BAUDRATE_TABLE {4800, 9600, 19200, 38400, 57600, 115200}

/*
 * L4 OSC1 Timer 0
 */
/* reload value when timer count to zero */
#define TIMER_LOAD_VAL			0xFFFFFFFF

/*
 * Flash configurations
 */
#define CONFIG_SYS_MAX_FLASH_BANKS     1

/* SPL memory allocation configuration, this is for FAT implementation */
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00015000

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOCFGPA_ARRIA10_H__ */
