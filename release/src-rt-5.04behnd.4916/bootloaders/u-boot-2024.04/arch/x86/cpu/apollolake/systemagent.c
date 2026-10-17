// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 Intel Corporation.
 * Take from coreboot project file of the same name
 */

#include <common.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/arch/systemagent.h>

void enable_bios_reset_cpl(void)
{
	/*
	 * Set bits 0+1 of BIOS_RESET_CPL to indicate to the CPU
	 * that BIOS has initialised memory and power management
	 *
	 * The FSP-S does not do this. If we leave this as zero then I believe
	 * the power-aware interrupts don't work in Linux, and CPU 0 always gets
	 * the interrupt.
	 */
	setbits_8(MCHBAR_REG(BIOS_RESET_CPL), 3);
}
