/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 - 2017 Xilinx.
 *
 * Configuration settings for the Xilinx Zynq CSE board.
 * See zynq-common.h for Zynq common configs
 */

#ifndef __CONFIG_ZYNQ_CSE_H
#define __CONFIG_ZYNQ_CSE_H

#include <configs/zynq-common.h>

/* Undef unneeded configs */
#undef CFG_EXTRA_ENV_SETTINGS

#undef CFG_SYS_INIT_RAM_ADDR
#undef CFG_SYS_INIT_RAM_SIZE
#define CFG_SYS_INIT_RAM_ADDR	0xFFFDE000
#define CFG_SYS_INIT_RAM_SIZE	0x1000

#endif /* __CONFIG_ZYNQ_CSE_H */
