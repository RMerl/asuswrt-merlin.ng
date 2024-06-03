/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#ifndef _CLOCK_MANAGER_H_
#define _CLOCK_MANAGER_H_

#ifndef __ASSEMBLER__
void cm_wait_for_lock(u32 mask);
int cm_wait_for_fsm(void);
void cm_print_clock_quick_summary(void);
#endif

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#include <asm/arch/clock_manager_gen5.h>
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#include <asm/arch/clock_manager_arria10.h>
#elif defined(CONFIG_TARGET_SOCFPGA_STRATIX10)
#include <asm/arch/clock_manager_s10.h>
#endif

#endif /* _CLOCK_MANAGER_H_ */
