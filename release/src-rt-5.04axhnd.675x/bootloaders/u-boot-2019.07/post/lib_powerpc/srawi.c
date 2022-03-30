// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Shift instructions:		srawi
 *
 * The test contains a pre-built table of instructions, operands and
 * expected results. For each table entry, the test will cyclically use
 * different sets of operand registers and result registers.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_21 (ulong *code, ulong *cr, ulong *res, ulong op);
extern ulong cpu_post_makecr (long v);

static struct cpu_post_srawi_s
{
    ulong cmd;
    ulong op1;
    uchar op2;
    ulong res;
} cpu_post_srawi_table[] =
{
    {
	OP_SRAWI,
	0x8000,
	3,
	0x1000
    },
    {
	OP_SRAWI,
	0x80000000,
	3,
	0xf0000000
    },
};
static unsigned int cpu_post_srawi_size = ARRAY_SIZE(cpu_post_srawi_table);

int cpu_post_test_srawi (void)
{
    int ret = 0;
    unsigned int i, reg;
    int flag = disable_interrupts();

    for (i = 0; i < cpu_post_srawi_size && ret == 0; i++)
    {
	struct cpu_post_srawi_s *test = cpu_post_srawi_table + i;

	for (reg = 0; reg < 32 && ret == 0; reg++)
	{
	    unsigned int reg0 = (reg + 0) % 32;
	    unsigned int reg1 = (reg + 1) % 32;
	    unsigned int stk = reg < 16 ? 31 : 15;
	    unsigned long code[] =
	    {
		ASM_STW(stk, 1, -4),
		ASM_ADDI(stk, 1, -16),
		ASM_STW(3, stk, 8),
		ASM_STW(reg0, stk, 4),
		ASM_STW(reg1, stk, 0),
		ASM_LWZ(reg0, stk, 8),
		ASM_11S(test->cmd, reg1, reg0, test->op2),
		ASM_STW(reg1, stk, 8),
		ASM_LWZ(reg1, stk, 0),
		ASM_LWZ(reg0, stk, 4),
		ASM_LWZ(3, stk, 8),
		ASM_ADDI(1, stk, 16),
		ASM_LWZ(stk, 1, -4),
		ASM_BLR,
	    };
	    unsigned long codecr[] =
	    {
		ASM_STW(stk, 1, -4),
		ASM_ADDI(stk, 1, -16),
		ASM_STW(3, stk, 8),
		ASM_STW(reg0, stk, 4),
		ASM_STW(reg1, stk, 0),
		ASM_LWZ(reg0, stk, 8),
		ASM_11S(test->cmd, reg1, reg0, test->op2) | BIT_C,
		ASM_STW(reg1, stk, 8),
		ASM_LWZ(reg1, stk, 0),
		ASM_LWZ(reg0, stk, 4),
		ASM_LWZ(3, stk, 8),
		ASM_ADDI(1, stk, 16),
		ASM_LWZ(stk, 1, -4),
		ASM_BLR,
	    };
	    ulong res;
	    ulong cr;

	    if (ret == 0)
	    {
		cr = 0;
		cpu_post_exec_21 (code, & cr, & res, test->op1);

		ret = res == test->res && cr == 0 ? 0 : -1;

		if (ret != 0)
		{
		    post_log ("Error at srawi test %d !\n", i);
		}
	    }

	    if (ret == 0)
	    {
		cpu_post_exec_21 (codecr, & cr, & res, test->op1);

		ret = res == test->res &&
		      (cr & 0xe0000000) == cpu_post_makecr (res) ? 0 : -1;

		if (ret != 0)
		{
		    post_log ("Error at srawi test %d !\n", i);
		}
	    }
	}
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
