// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Load instructions:		lbz(x)(u), lhz(x)(u), lha(x)(u), lwz(x)(u)
 *
 * All operations are performed on a 16-byte array. The array
 * is 4-byte aligned. The base register points to offset 8.
 * The immediate offset (index register) ranges in [-8 ... +7].
 * The test cases are composed so that they do not
 * cause alignment exceptions.
 * The test contains a pre-built table describing all test cases.
 * The table entry contains:
 * the instruction opcode, the array contents, the value of the index
 * register and the expected value of the destination register.
 * After executing the instruction, the test verifies the
 * value of the destination register and the value of the base
 * register (it must change for "load with update" instructions).
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_22w (ulong *code, ulong *op1, ulong op2, ulong *op3);
extern void cpu_post_exec_21w (ulong *code, ulong *op1, ulong *op2);

static struct cpu_post_load_s
{
    ulong cmd;
    uint width;
    int update;
    int index;
    ulong offset;
} cpu_post_load_table[] =
{
    {
	OP_LWZ,
	4,
	0,
	0,
	4
    },
    {
	OP_LHA,
	3,
	0,
	0,
	2
    },
    {
	OP_LHZ,
	2,
	0,
	0,
	2
    },
    {
	OP_LBZ,
	1,
	0,
	0,
	1
    },
    {
	OP_LWZU,
	4,
	1,
	0,
	4
    },
    {
	OP_LHAU,
	3,
	1,
	0,
	2
    },
    {
	OP_LHZU,
	2,
	1,
	0,
	2
    },
    {
	OP_LBZU,
	1,
	1,
	0,
	1
    },
    {
	OP_LWZX,
	4,
	0,
	1,
	4
    },
    {
	OP_LHAX,
	3,
	0,
	1,
	2
    },
    {
	OP_LHZX,
	2,
	0,
	1,
	2
    },
    {
	OP_LBZX,
	1,
	0,
	1,
	1
    },
    {
	OP_LWZUX,
	4,
	1,
	1,
	4
    },
    {
	OP_LHAUX,
	3,
	1,
	1,
	2
    },
    {
	OP_LHZUX,
	2,
	1,
	1,
	2
    },
    {
	OP_LBZUX,
	1,
	1,
	1,
	1
    },
};
static unsigned int cpu_post_load_size = ARRAY_SIZE(cpu_post_load_table);

int cpu_post_test_load (void)
{
    int ret = 0;
    unsigned int i;
    int flag = disable_interrupts();

    for (i = 0; i < cpu_post_load_size && ret == 0; i++)
    {
	struct cpu_post_load_s *test = cpu_post_load_table + i;
	uchar data[16] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	ulong base0 = (ulong) (data + 8);
	ulong base = base0;
	ulong value;

	if (test->index)
	{
	    ulong code[] =
	    {
		ASM_12(test->cmd, 5, 3, 4),
		ASM_BLR,
	    };

	    cpu_post_exec_22w (code, &base, test->offset, &value);
	}
	else
	{
	    ulong code[] =
	    {
		ASM_11I(test->cmd, 4, 3, test->offset),
		ASM_BLR,
	    };

	    cpu_post_exec_21w (code, &base, &value);
	}

	if (ret == 0)
	{
	   if (test->update)
	       ret = base == base0 + test->offset ? 0 : -1;
	   else
	       ret = base == base0 ? 0 : -1;
	}

	if (ret == 0)
	{
	    switch (test->width)
	    {
	    case 1:
		ret = *(uchar *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    case 2:
		ret = *(ushort *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    case 3:
		ret = *(short *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    case 4:
		ret = *(ulong *)(base0 + test->offset) == value ?
		      0 : -1;
		break;
	    }
	}

	if (ret != 0)
	{
	    post_log ("Error at load test %d !\n", i);
	}
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
