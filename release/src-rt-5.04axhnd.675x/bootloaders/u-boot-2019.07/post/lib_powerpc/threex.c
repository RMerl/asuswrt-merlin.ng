// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Ternary instructions		instr rA,rS,rB
 *
 * Logic instructions:		or, orc, xor, nand, nor, eqv
 * Shift instructions:		slw, srw, sraw
 *
 * The test contains a pre-built table of instructions, operands and
 * expected results. For each table entry, the test will cyclically use
 * different sets of operand registers and result registers.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_22 (ulong *code, ulong *cr, ulong *res, ulong op1,
    ulong op2);
extern ulong cpu_post_makecr (long v);

static struct cpu_post_threex_s
{
    ulong cmd;
    ulong op1;
    ulong op2;
    ulong res;
} cpu_post_threex_table[] =
{
    {
	OP_OR,
	0x1234,
	0x5678,
	0x1234 | 0x5678
    },
    {
	OP_ORC,
	0x1234,
	0x5678,
	0x1234 | ~0x5678
    },
    {
	OP_XOR,
	0x1234,
	0x5678,
	0x1234 ^ 0x5678
    },
    {
	OP_NAND,
	0x1234,
	0x5678,
	~(0x1234 & 0x5678)
    },
    {
	OP_NOR,
	0x1234,
	0x5678,
	~(0x1234 | 0x5678)
    },
    {
	OP_EQV,
	0x1234,
	0x5678,
	~(0x1234 ^ 0x5678)
    },
    {
	OP_SLW,
	0x80,
	16,
	0x800000
    },
    {
	OP_SLW,
	0x80,
	32,
	0
    },
    {
	OP_SRW,
	0x800000,
	16,
	0x80
    },
    {
	OP_SRW,
	0x800000,
	32,
	0
    },
    {
	OP_SRAW,
	0x80000000,
	3,
	0xf0000000
    },
    {
	OP_SRAW,
	0x8000,
	3,
	0x1000
    },
};
static unsigned int cpu_post_threex_size = ARRAY_SIZE(cpu_post_threex_table);

int cpu_post_test_threex (void)
{
    int ret = 0;
    unsigned int i, reg;
    int flag = disable_interrupts();

    for (i = 0; i < cpu_post_threex_size && ret == 0; i++)
    {
	struct cpu_post_threex_s *test = cpu_post_threex_table + i;

	for (reg = 0; reg < 32 && ret == 0; reg++)
	{
	    unsigned int reg0 = (reg + 0) % 32;
	    unsigned int reg1 = (reg + 1) % 32;
	    unsigned int reg2 = (reg + 2) % 32;
	    unsigned int stk = reg < 16 ? 31 : 15;
	    unsigned long code[] =
	    {
		ASM_STW(stk, 1, -4),
		ASM_ADDI(stk, 1, -24),
		ASM_STW(3, stk, 12),
		ASM_STW(4, stk, 16),
		ASM_STW(reg0, stk, 8),
		ASM_STW(reg1, stk, 4),
		ASM_STW(reg2, stk, 0),
		ASM_LWZ(reg1, stk, 12),
		ASM_LWZ(reg0, stk, 16),
		ASM_12X(test->cmd, reg2, reg1, reg0),
		ASM_STW(reg2, stk, 12),
		ASM_LWZ(reg2, stk, 0),
		ASM_LWZ(reg1, stk, 4),
		ASM_LWZ(reg0, stk, 8),
		ASM_LWZ(3, stk, 12),
		ASM_ADDI(1, stk, 24),
		ASM_LWZ(stk, 1, -4),
		ASM_BLR,
	    };
	    unsigned long codecr[] =
	    {
		ASM_STW(stk, 1, -4),
		ASM_ADDI(stk, 1, -24),
		ASM_STW(3, stk, 12),
		ASM_STW(4, stk, 16),
		ASM_STW(reg0, stk, 8),
		ASM_STW(reg1, stk, 4),
		ASM_STW(reg2, stk, 0),
		ASM_LWZ(reg1, stk, 12),
		ASM_LWZ(reg0, stk, 16),
		ASM_12X(test->cmd, reg2, reg1, reg0) | BIT_C,
		ASM_STW(reg2, stk, 12),
		ASM_LWZ(reg2, stk, 0),
		ASM_LWZ(reg1, stk, 4),
		ASM_LWZ(reg0, stk, 8),
		ASM_LWZ(3, stk, 12),
		ASM_ADDI(1, stk, 24),
		ASM_LWZ(stk, 1, -4),
		ASM_BLR,
	    };
	    ulong res;
	    ulong cr;

	    if (ret == 0)
	    {
		cr = 0;
		cpu_post_exec_22 (code, & cr, & res, test->op1, test->op2);

		ret = res == test->res && cr == 0 ? 0 : -1;

		if (ret != 0)
		{
		    post_log ("Error at threex test %d !\n", i);
		}
	    }

	    if (ret == 0)
	    {
		cpu_post_exec_22 (codecr, & cr, & res, test->op1, test->op2);

		ret = res == test->res &&
		      (cr & 0xe0000000) == cpu_post_makecr (res) ? 0 : -1;

		if (ret != 0)
		{
		    post_log ("Error at threex test %d !\n", i);
		}
	    }
	}
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
