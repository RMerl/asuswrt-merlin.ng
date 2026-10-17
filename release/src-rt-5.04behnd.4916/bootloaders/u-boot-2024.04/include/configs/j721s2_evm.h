/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 J721S2 EVM
 *
 * Copyright (C) 2021 Texas Instruments Incorporated - https://www.ti.com/
 *	David Huang <d-huang@ti.com>
 */

#ifndef __CONFIG_J721S2_EVM_H
#define __CONFIG_J721S2_EVM_H

#include <linux/sizes.h>
#include <config_distro_bootcmd.h>

/* SPL Loader Configuration */
#if defined(CONFIG_TARGET_J721S2_A72_EVM)
#define CFG_SYS_UBOOT_BASE		0x50280000
/* Image load address in RAM for DFU boot*/
#else
#define CFG_SYS_UBOOT_BASE		0x50080000
#endif

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_J721S2_EVM_H */
