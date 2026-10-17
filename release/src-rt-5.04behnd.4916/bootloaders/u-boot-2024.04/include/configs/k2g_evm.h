/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for TI's k2g-evm
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __CONFIG_K2G_EVM_H
#define __CONFIG_K2G_EVM_H

/* Network */
#define CFG_KSNET_CPSW_NUM_PORTS	2
#define PHY_ANEG_TIMEOUT	10000 /* PHY needs longer aneg time */

#include <configs/ti_armv7_keystone2.h>

#endif /* __CONFIG_K2G_EVM_H */
