/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017-2020 Hitachi Power Grids
 *
 */
#ifndef __CONFIG_SOCFPGA_SECU1_H__
#define __CONFIG_SOCFPGA_SECU1_H__

#include <asm/arch/base_addr_ac5.h>

/* Eternal oscillator */
#define CFG_SYS_TIMER_RATE	40000000

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512MiB on SECU1 */

/*
 * We use bootcounter in i2c nvram of the RTC (0x68)
 * The offset fopr the bootcounter is 0x9e, which are
 * the last two bytes of the 128 bytes large NVRAM in the
 * RTC which begin at address 0x20
 */
#define CFG_SYS_I2C_RTC_ADDR         0x68

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOCFPGA_SECU1_H__ */
