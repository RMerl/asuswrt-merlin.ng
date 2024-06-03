// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Store instructions:		stb(x)(u), sth(x)(u), stw(x)(u)
 *
 * All operations are performed on a 16-byte array. The array
 * is 4-byte aligned. The base register points to offset 8.
 * The immediate offset (index register) ranges in [-8 ... +7].
 * The test cases are composed so that they do not
 * cause alignment exceptions.
 * The test contains a pre-built table describing all test cases.
 * The table entry contains:
 * the instruction opcode, the value of the index register and
 * the value of the source register. After executing the
 * instruction, the test verifies the contents of the array
 * and the value of the base register (it must change for "store
 * with update" instructions).
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_12w (ulong *code, ulong *op1, ulong op2, ulong op3);
extern void cpu_post_exec_11w (ulong *code, ulong *op1, ulong op2);

static struct cpu_post_store_s
{
    ulong cmd;
    uint width;
    int update;
    int index;
    ulong offset;
    ulong value;
} cpu_post_store_table[] =
{
    {
	OP_STW,
	4,
	0,
	0,
	-4,
	0xff00ff00
    },
    {
	OP_STH,
	2,
	0,
	0,
	-2,
	0xff00
    },
    {
	OP_STB,
	1,
	0,
	0,
	-1,
	0xff
    },
    {
	OP_STWU,
	4,
	1,
	0,
	-4,
	0xff00ff00
    },
    {
	OP_STHU,
	2,
	1,
	0,
	-2,
	0xff00
    },
    {
	OP_STBU,
	1,
	1,
	0,
	-1,
	0xff
    },
    {
	OP_STWX,
	4,
	0,
	1,
	-4,
	0xff00ff00
    },
    {
	OP_STHX,
	2,
	0,
	1,
	-2,
	0xff00
    },
    {
	OP_STBX,
	1,
	0,
	1,
	-1,
	0xff
    },
    {
	OP_STWUX,
	4,
	1,
	1,
	-4,
	0xff00ff00
    },
    {
	OP_STHUX,
	2,
	1,
	1,
	-2,
	0xff00
    },
    {
	OP_STBUX,
	1,
	1,
	1,
	-1,
	0xff
    },
};
static unsigned int cpu_post_store_size = ARRAY_SIZE(cpu_post_store_table);

int cpu_post_test_store (void)
{
    int ret = 0;
    unsigned int i;
    int flag = disable_interrupts();

    for (i = 0; i < cpu_post_store_size && ret == 0; i++)
    {
	struct cpu_post_store_s *test = cpu_post_store_table + i;
	uchar data[16] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	ulong base0 = (ulong) (data + 8);
	ulong base = base0;

	if (test->index)
	{
	    ulong code[] =
	    {
		ASM_12(test->cmd, 5, 3, 4),
		ASM_BLR,
	    };

	    cpu_post_exec_12w (code, &base, test->offset, test->value);
	}
	else
	{
	    ulong code[] =
	    {
		ASM_11I(test->cmd, 4, 3, test->offset),
		ASM_BLR,
	    };

	    cpu_post_exec_11w (code, &base, test->value);
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
		ret = *(uchar *)(base0 + test->offset) == test->value ?
		      0 : -1;
		break;
	    case 2:
		ret = *(ushort *)(base0 + test->offset) == test->value ?
		      0 : -1;
		break;
	    case 4:
		ret = *(ulong *)(base0 + test->offset) == test->value ?
		      0 : -1;
		break;
	    }
	}

	if (ret != 0)
	{
	    post_log ("Error at store test %d !\n", i);
	}
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
