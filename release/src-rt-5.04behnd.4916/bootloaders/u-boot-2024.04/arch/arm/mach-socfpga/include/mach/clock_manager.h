/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#ifndef _CLOCK_MANAGER_H_
#define _CLOCK_MANAGER_H_

phys_addr_t socfpga_get_clkmgr_addr(void);

#ifndef __ASSEMBLY__
void cm_wait_for_lock(u32 mask);
int cm_wait_for_fsm(void);
void cm_print_clock_quick_summary(void);
unsigned long cm_get_mpu_clk_hz(void);
unsigned int cm_get_qspi_controller_clk_hz(void);

#if defined(CONFIG_TARGET_SOCFPGA_SOC64)
int cm_set_qspi_controller_clk_hz(u32 clk_hz);
#endif
#endif

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#include <asm/arch/clock_manager_gen5.h>
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#include <asm/arch/clock_manager_arria10.h>
#elif defined(CONFIG_TARGET_SOCFPGA_STRATIX10)
#include <asm/arch/clock_manager_s10.h>
#elif defined(CONFIG_TARGET_SOCFPGA_AGILEX)
#include <asm/arch/clock_manager_agilex.h>
#elif IS_ENABLED(CONFIG_TARGET_SOCFPGA_N5X)
#include <asm/arch/clock_manager_n5x.h>
#endif

#endif /* _CLOCK_MANAGER_H_ */
