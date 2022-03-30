// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Binary instructions		instr rA,rS
 *
 * Logic instructions:		cntlzw
 * Arithmetic instructions:	extsb, extsh

 * The test contains a pre-built table of instructions, operands and
 * expected results. For each table entry, the test will cyclically use
 * different sets of operand registers and result registers.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_21 (ulong *code, ulong *cr, ulong *res, ulong op1);
extern ulong cpu_post_makecr (long v);

static struct cpu_post_twox_s
{
    ulong cmd;
    ulong op;
    ulong res;
} cpu_post_twox_table[] =
{
    {
	OP_EXTSB,
	3,
	3
    },
    {
	OP_EXTSB,
	0xff,
	-1
    },
    {
	OP_EXTSH,
	3,
	3
    },
    {
	OP_EXTSH,
	0xff,
	0xff
    },
    {
	OP_EXTSH,
	0xffff,
	-1
    },
    {
	OP_CNTLZW,
	0x000fffff,
	12
    },
};
static unsigned int cpu_post_twox_size = ARRAY_SIZE(cpu_post_twox_table);

int cpu_post_test_twox (void)
{
    int ret = 0;
    unsigned int i, reg;
    int flag = disable_interrupts();

    for (i = 0; i < cpu_post_twox_size && ret == 0; i++)
    {
	struct cpu_post_twox_s *test = cpu_post_twox_table + i;

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
		ASM_11X(test->cmd, reg1, reg0),
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
		ASM_11X(test->cmd, reg1, reg0) | BIT_C,
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
		cpu_post_exec_21 (code, & cr, & res, test->op);

		ret = res == test->res && cr == 0 ? 0 : -1;

		if (ret != 0)
		{
		    post_log ("Error at twox test %d !\n", i);
		}
	    }

	    if (ret == 0)
	    {
		cpu_post_exec_21 (codecr, & cr, & res, test->op);

		ret = res == test->res &&
		      (cr & 0xe0000000) == cpu_post_makecr (res) ? 0 : -1;

		if (ret != 0)
		{
		    post_log ("Error at twox test %d !\n", i);
		}
	    }
	}
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
