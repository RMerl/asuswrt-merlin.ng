/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Embedded Artists LPC3250 DevKit v2
 * Copyright (C) 2021  Trevor Woerner <twoerner@gmail.com>
 */

#ifndef __CONFIG_EA_LPC3250DEVKITV2_H__
#define __CONFIG_EA_LPC3250DEVKITV2_H__

#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * RAM
 */
#define CFG_SYS_SDRAM_BASE EMC_DYCS0_BASE

/*
 * cmd
 */

/*
 * SoC-specific config
 */
#include <asm/arch/config.h>

#endif
