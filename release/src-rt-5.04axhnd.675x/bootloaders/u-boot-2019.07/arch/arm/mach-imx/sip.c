// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 */

#include <common.h>
#include <asm/arch/sys_proto.h>

unsigned long call_imx_sip(unsigned long id, unsigned long reg0,
			   unsigned long reg1, unsigned long reg2)
{
	struct pt_regs regs;

	regs.regs[0] = id;
	regs.regs[1] = reg0;
	regs.regs[2] = reg1;
	regs.regs[3] = reg2;

	smc_call(&regs);

	return regs.regs[0];
}

/*
 * Do an SMC call to return 2 registers by having reg1 passed in by reference
 */
unsigned long call_imx_sip_ret2(unsigned long id, unsigned long reg0,
				unsigned long *reg1, unsigned long reg2,
				unsigned long reg3)
{
	struct pt_regs regs;

	regs.regs[0] = id;
	regs.regs[1] = reg0;
	regs.regs[2] = *reg1;
	regs.regs[3] = reg2;
	regs.regs[4] = reg3;

	smc_call(&regs);

	*reg1 = regs.regs[1];

	return regs.regs[0];
}
