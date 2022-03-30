/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __ASM_INTEL_REGS_H
#define __ASM_INTEL_REGS_H

/* Access the memory-controller hub */
#define MCH_BASE_ADDRESS	0xfed10000
#define MCH_BASE_SIZE		0x8000
#define MCHBAR_REG(reg)		(MCH_BASE_ADDRESS + (reg))

#define MCHBAR_PEI_VERSION	0x5034
#define MCH_PKG_POWER_LIMIT_LO	0x59a0
#define MCH_PKG_POWER_LIMIT_HI	0x59a4
#define MCH_DDR_POWER_LIMIT_LO	0x58e0
#define MCH_DDR_POWER_LIMIT_HI	0x58e4

/* Access the Root Complex Register Block */
#define RCB_BASE_ADDRESS	0xfed1c000
#define RCB_REG(reg)		(RCB_BASE_ADDRESS + (reg))

#define SOFT_RESET_CTRL		0x38f4
#define SOFT_RESET_DATA		0x38f8

#endif
