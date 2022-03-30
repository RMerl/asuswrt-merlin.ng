/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for Xilinx Versal QSPI Flash utility
 *
 * (C) Copyright 2018-2019 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * Siva Durga Prasad Paladugu <sivadur@xilinx.com>
 */

#ifndef __CONFIG_VERSAL_MINI_QSPI_H
#define __CONFIG_VERSAL_MINI_QSPI_H

#include <configs/xilinx_versal_mini.h>

#undef CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_TEXT_BASE + 0x20000)

#endif /* __CONFIG_VERSAL_MINI_QSPI_H */
