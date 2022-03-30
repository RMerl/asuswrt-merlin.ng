/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Xilinx ZynqMP Nand Flash utility
 *
 * (C) Copyright 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * Siva Durga Prasad Paladugu <sivadur@xilinx.com>
 */

#ifndef __CONFIG_ZYNQMP_MINI_NAND_H
#define __CONFIG_ZYNQMP_MINI_NAND_H

#include <configs/xilinx_zynqmp_mini.h>

#define CONFIG_SYS_SDRAM_SIZE	0x1000000
#define CONFIG_SYS_SDRAM_BASE	0x0
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x40000)
#define CONFIG_SYS_MALLOC_LEN		0x800000

#endif /* __CONFIG_ZYNQMP_MINI_NAND_H */
