/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 */

#ifndef _RESET_MANAGER_H_
#define _RESET_MANAGER_H_

phys_addr_t socfpga_get_rstmgr_addr(void);

void reset_cpu(void);

void socfpga_per_reset(u32 reset, int set);
void socfpga_per_reset_all(void);

#define RSTMGR_CTRL_SWCOLDRSTREQ_LSB 0
#define RSTMGR_CTRL_SWWARMRSTREQ_LSB 1

/*
 * Define a reset identifier, from which a permodrst bank ID
 * and reset ID can be extracted using the subsequent macros
 * RSTMGR_RESET() and RSTMGR_BANK().
 */
#define RSTMGR_BANK_OFFSET	8
#define RSTMGR_BANK_MASK	0x7
#define RSTMGR_RESET_OFFSET	0
#define RSTMGR_RESET_MASK	0x1f
#define RSTMGR_DEFINE(_bank, _offset)		\
	((_bank) << RSTMGR_BANK_OFFSET) | ((_offset) << RSTMGR_RESET_OFFSET)

/* Extract reset ID from the reset identifier. */
#define RSTMGR_RESET(_reset)			\
	(((_reset) >> RSTMGR_RESET_OFFSET) & RSTMGR_RESET_MASK)

/* Extract bank ID from the reset identifier. */
#define RSTMGR_BANK(_reset)			\
	(((_reset) >> RSTMGR_BANK_OFFSET) & RSTMGR_BANK_MASK)

/* Create a human-readable reference to SoCFPGA reset. */
#define SOCFPGA_RESET(_name)	RSTMGR_##_name

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#include <asm/arch/reset_manager_gen5.h>
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#include <asm/arch/reset_manager_arria10.h>
#elif defined(CONFIG_TARGET_SOCFPGA_SOC64)
#include <asm/arch/reset_manager_soc64.h>
#endif

#endif /* _RESET_MANAGER_H_ */
