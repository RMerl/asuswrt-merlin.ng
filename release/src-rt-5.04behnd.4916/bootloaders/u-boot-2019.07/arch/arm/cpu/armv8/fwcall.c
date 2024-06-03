// SPDX-License-Identifier: GPL-2.0+
/**
 * (C) Copyright 2014, Cavium Inc.
 * (C) Copyright 2017, Xilinx Inc.
 *
**/

#include <asm-offsets.h>
#include <config.h>
#include <version.h>
#include <asm/macro.h>
#include <asm/psci.h>
#include <asm/system.h>

/*
 * Issue the hypervisor call
 *
 * x0~x7: input arguments
 * x0~x3: output arguments
 */
static void hvc_call(struct pt_regs *args)
{
	asm volatile(
		"ldr x0, %0\n"
		"ldr x1, %1\n"
		"ldr x2, %2\n"
		"ldr x3, %3\n"
		"ldr x4, %4\n"
		"ldr x5, %5\n"
		"ldr x6, %6\n"
		"hvc	#0\n"
		"str x0, %0\n"
		"str x1, %1\n"
		"str x2, %2\n"
		"str x3, %3\n"
		: "+m" (args->regs[0]), "+m" (args->regs[1]),
		  "+m" (args->regs[2]), "+m" (args->regs[3])
		: "m" (args->regs[4]), "m" (args->regs[5]),
		  "m" (args->regs[6])
		: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17");
}

/*
 * void smc_call(arg0, arg1...arg7)
 *
 * issue the secure monitor call
 *
 * x0~x7: input arguments
 * x0~x3: output arguments
 */

void smc_call(struct pt_regs *args)
{
	asm volatile(
		"ldr x0, %0\n"
		"ldr x1, %1\n"
		"ldr x2, %2\n"
		"ldr x3, %3\n"
		"ldr x4, %4\n"
		"ldr x5, %5\n"
		"ldr x6, %6\n"
		"smc	#0\n"
		"str x0, %0\n"
		"str x1, %1\n"
		"str x2, %2\n"
		"str x3, %3\n"
		: "+m" (args->regs[0]), "+m" (args->regs[1]),
		  "+m" (args->regs[2]), "+m" (args->regs[3])
		: "m" (args->regs[4]), "m" (args->regs[5]),
		  "m" (args->regs[6])
		: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17");
}

/*
 * For now, all systems we support run at least in EL2 and thus
 * trigger PSCI calls to EL3 using SMC. If anyone ever wants to
 * use PSCI on U-Boot running below a hypervisor, please detect
 * this and set the flag accordingly.
 */
static const bool use_smc_for_psci = true;

void __noreturn psci_system_reset(void)
{
	struct pt_regs regs;

	regs.regs[0] = ARM_PSCI_0_2_FN_SYSTEM_RESET;

	if (use_smc_for_psci)
		smc_call(&regs);
	else
		hvc_call(&regs);

	while (1)
		;
}

void __noreturn psci_system_off(void)
{
	struct pt_regs regs;

	regs.regs[0] = ARM_PSCI_0_2_FN_SYSTEM_OFF;

	if (use_smc_for_psci)
		smc_call(&regs);
	else
		hvc_call(&regs);

	while (1)
		;
}
