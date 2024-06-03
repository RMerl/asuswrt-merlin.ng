// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Condition register istructions:	mtcr, mfcr, mcrxr,
 *					crand, crandc, cror, crorc, crxor,
 *					crnand, crnor, creqv, mcrf
 *
 * The mtcrf/mfcr instructions is tested by loading different
 * values into the condition register (mtcrf), moving its value
 * to a general-purpose register (mfcr) and comparing this value
 * with the expected one.
 * The mcrxr instruction is tested by loading a fixed value
 * into the XER register (mtspr), moving XER value to the
 * condition register (mcrxr), moving it to a general-purpose
 * register (mfcr) and comparing the value of this register with
 * the expected one.
 * The rest of instructions is tested by loading a fixed
 * value into the condition register (mtcrf), executing each
 * instruction several times to modify all 4-bit condition
 * fields, moving the value of the conditional register to a
 * general-purpose register (mfcr) and comparing it with the
 * expected one.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_11 (ulong *code, ulong *res, ulong op1);
extern void cpu_post_exec_21x (ulong *code, ulong *op1, ulong *op2, ulong op3);

static ulong cpu_post_cr_table1[] =
{
    0xaaaaaaaa,
    0x55555555,
};
static unsigned int cpu_post_cr_size1 = ARRAY_SIZE(cpu_post_cr_table1);

static struct cpu_post_cr_s2 {
    ulong xer;
    ulong cr;
} cpu_post_cr_table2[] =
{
    {
	0xa0000000,
	1
    },
    {
	0x40000000,
	5
    },
};
static unsigned int cpu_post_cr_size2 = ARRAY_SIZE(cpu_post_cr_table2);

static struct cpu_post_cr_s3 {
    ulong cr;
    ulong cs;
    ulong cd;
    ulong res;
} cpu_post_cr_table3[] =
{
    {
	0x01234567,
	0,
	4,
	0x01230567
    },
    {
	0x01234567,
	7,
	0,
	0x71234567
    },
};
static unsigned int cpu_post_cr_size3 = ARRAY_SIZE(cpu_post_cr_table3);

static struct cpu_post_cr_s4 {
    ulong cmd;
    ulong cr;
    ulong op1;
    ulong op2;
    ulong op3;
    ulong res;
} cpu_post_cr_table4[] =
{
    {
	OP_CRAND,
	0x0000ffff,
	0,
	16,
	0,
	0x0000ffff
    },
    {
	OP_CRAND,
	0x0000ffff,
	16,
	17,
	0,
	0x8000ffff
    },
    {
	OP_CRANDC,
	0x0000ffff,
	0,
	16,
	0,
	0x0000ffff
    },
    {
	OP_CRANDC,
	0x0000ffff,
	16,
	0,
	0,
	0x8000ffff
    },
    {
	OP_CROR,
	0x0000ffff,
	0,
	16,
	0,
	0x8000ffff
    },
    {
	OP_CROR,
	0x0000ffff,
	0,
	1,
	0,
	0x0000ffff
    },
    {
	OP_CRORC,
	0x0000ffff,
	0,
	16,
	0,
	0x0000ffff
    },
    {
	OP_CRORC,
	0x0000ffff,
	0,
	0,
	0,
	0x8000ffff
    },
    {
	OP_CRXOR,
	0x0000ffff,
	0,
	0,
	0,
	0x0000ffff
    },
    {
	OP_CRXOR,
	0x0000ffff,
	0,
	16,
	0,
	0x8000ffff
    },
    {
	OP_CRNAND,
	0x0000ffff,
	0,
	16,
	0,
	0x8000ffff
    },
    {
	OP_CRNAND,
	0x0000ffff,
	16,
	17,
	0,
	0x0000ffff
    },
    {
	OP_CRNOR,
	0x0000ffff,
	0,
	16,
	0,
	0x0000ffff
    },
    {
	OP_CRNOR,
	0x0000ffff,
	0,
	1,
	0,
	0x8000ffff
    },
    {
	OP_CREQV,
	0x0000ffff,
	0,
	0,
	0,
	0x8000ffff
    },
    {
	OP_CREQV,
	0x0000ffff,
	0,
	16,
	0,
	0x0000ffff
    },
};
static unsigned int cpu_post_cr_size4 = ARRAY_SIZE(cpu_post_cr_table4);

int cpu_post_test_cr (void)
{
    int ret = 0;
    unsigned int i;
    unsigned long cr_sav;
    int flag = disable_interrupts();

    asm ( "mfcr %0" : "=r" (cr_sav) : );

    for (i = 0; i < cpu_post_cr_size1 && ret == 0; i++)
    {
	ulong cr = cpu_post_cr_table1[i];
	ulong res;

	unsigned long code[] =
	{
	    ASM_MTCR(3),
	    ASM_MFCR(3),
	    ASM_BLR,
	};

	cpu_post_exec_11 (code, &res, cr);

	ret = res == cr ? 0 : -1;

	if (ret != 0)
	{
	    post_log ("Error at cr1 test %d !\n", i);
	}
    }

    for (i = 0; i < cpu_post_cr_size2 && ret == 0; i++)
    {
	struct cpu_post_cr_s2 *test = cpu_post_cr_table2 + i;
	ulong res;
	ulong xer;

	unsigned long code[] =
	{
	    ASM_MTXER(3),
	    ASM_MCRXR(test->cr),
	    ASM_MFCR(3),
	    ASM_MFXER(4),
	    ASM_BLR,
	};

	cpu_post_exec_21x (code, &res, &xer, test->xer);

	ret = xer == 0 && ((res << (4 * test->cr)) & 0xe0000000) == test->xer ?
	      0 : -1;

	if (ret != 0)
	{
	    post_log ("Error at cr2 test %d !\n", i);
	}
    }

    for (i = 0; i < cpu_post_cr_size3 && ret == 0; i++)
    {
	struct cpu_post_cr_s3 *test = cpu_post_cr_table3 + i;
	ulong res;

	unsigned long code[] =
	{
	    ASM_MTCR(3),
	    ASM_MCRF(test->cd, test->cs),
	    ASM_MFCR(3),
	    ASM_BLR,
	};

	cpu_post_exec_11 (code, &res, test->cr);

	ret = res == test->res ? 0 : -1;

	if (ret != 0)
	{
	    post_log ("Error at cr3 test %d !\n", i);
	}
    }

    for (i = 0; i < cpu_post_cr_size4 && ret == 0; i++)
    {
	struct cpu_post_cr_s4 *test = cpu_post_cr_table4 + i;
	ulong res;

	unsigned long code[] =
	{
	    ASM_MTCR(3),
	    ASM_12F(test->cmd, test->op3, test->op1, test->op2),
	    ASM_MFCR(3),
	    ASM_BLR,
	};

	cpu_post_exec_11 (code, &res, test->cr);

	ret = res == test->res ? 0 : -1;

	if (ret != 0)
	{
	    post_log ("Error at cr4 test %d !\n", i);
	}
    }

    asm ( "mtcr %0" : : "r" (cr_sav));

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
