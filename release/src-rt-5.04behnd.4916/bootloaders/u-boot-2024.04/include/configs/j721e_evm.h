/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 J721E EVM
 *
 * Copyright (C) 2018-2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __CONFIG_J721E_EVM_H
#define __CONFIG_J721E_EVM_H

#include <linux/sizes.h>

/* FLASH Configuration */
#define CFG_SYS_FLASH_BASE		0x000000000

/* SPL Loader Configuration */
#if defined(CONFIG_TARGET_J721E_A72_EVM) || defined(CONFIG_TARGET_J7200_A72_EVM)
#define CFG_SYS_UBOOT_BASE		0x50280000
/* Image load address in RAM for DFU boot*/
#else
#define CFG_SYS_UBOOT_BASE		0x50080000
#endif

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>


#endif /* __CONFIG_J721E_EVM_H */
