/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Xilinx ZynqMP eMMC Flash utility
 *
 * (C) Copyright 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * Siva Durga Prasad Paladugu <sivadur@xilinx.com>
 */

#ifndef __CONFIG_ZYNQMP_MINI_EMMC_H
#define __CONFIG_ZYNQMP_MINI_EMMC_H

#include <configs/xilinx_zynqmp_mini.h>

#define CONFIG_SYS_INIT_SP_ADDR	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MALLOC_LEN	0x800000

#endif /* __CONFIG_ZYNQMP_MINI_EMMC_H */
