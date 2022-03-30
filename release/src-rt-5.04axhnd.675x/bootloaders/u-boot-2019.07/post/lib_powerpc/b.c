// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Branch instructions:		b, bl, bc
 *
 * The first 2 instructions (b, bl) are verified by jumping
 * to a fixed address and checking whether control was transferred
 * to that very point. For the bl instruction the value of the
 * link register is checked as well (using mfspr).
 * To verify the bc instruction various combinations of the BI/BO
 * fields, the CTR and the condition register values are
 * checked. The list of such combinations is pre-built and
 * linked in U-Boot at build time.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_11 (ulong *code, ulong *res, ulong op1);
extern void cpu_post_exec_31 (ulong *code, ulong *ctr, ulong *lr, ulong *jump,
    ulong cr);

static int cpu_post_test_bc (ulong cmd, ulong bo, ulong bi,
    int pjump, int decr, int link, ulong pctr, ulong cr)
{
    int ret = 0;
    ulong lr = 0;
    ulong ctr = pctr;
    ulong jump;

    unsigned long code[] =
    {
	ASM_MTCR(6),
	ASM_MFLR(6),
	ASM_MTCTR(3),
	ASM_MTLR(4),
	ASM_LI(5, 1),
	ASM_3O(cmd, bo, bi, 8),
	ASM_LI(5, 0),
	ASM_MFCTR(3),
	ASM_MFLR(4),
	ASM_MTLR(6),
	ASM_BLR,
    };

    cpu_post_exec_31 (code, &ctr, &lr, &jump, cr);

    if (ret == 0)
	ret = pjump == jump ? 0 : -1;
    if (ret == 0)
    {
	if (decr)
	    ret = pctr == ctr + 1 ? 0 : -1;
	else
	    ret = pctr == ctr ? 0 : -1;
    }
    if (ret == 0)
    {
	if (link)
	    ret = lr == (ulong) code + 24 ? 0 : -1;
	else
	    ret = lr == 0 ? 0 : -1;
    }

    return ret;
}

int cpu_post_test_b (void)
{
    int ret = 0;
    unsigned int i;
    int flag = disable_interrupts();

    if (ret == 0)
    {
	ulong code[] =
	{
	   ASM_MFLR(4),
	   ASM_MTLR(3),
	   ASM_B(4),
	   ASM_MFLR(3),
	   ASM_MTLR(4),
	   ASM_BLR,
	};
	ulong res;

	cpu_post_exec_11 (code, &res, 0);

	ret = res == 0 ? 0 : -1;

	if (ret != 0)
	{
	    post_log ("Error at b1 test !\n");
	}
    }

    if (ret == 0)
    {
	ulong code[] =
	{
	   ASM_MFLR(4),
	   ASM_MTLR(3),
	   ASM_BL(4),
	   ASM_MFLR(3),
	   ASM_MTLR(4),
	   ASM_BLR,
	};
	ulong res;

	cpu_post_exec_11 (code, &res, 0);

	ret = res == (ulong)code + 12 ? 0 : -1;

	if (ret != 0)
	{
	    post_log ("Error at b2 test !\n");
	}
    }

    if (ret == 0)
    {
	ulong cc, cd;
	int cond;
	ulong ctr;
	int link;

	i = 0;

	for (cc = 0; cc < 4 && ret == 0; cc++)
	{
	    for (cd = 0; cd < 4 && ret == 0; cd++)
	    {
		for (link = 0; link <= 1 && ret == 0; link++)
		{
		    for (cond = 0; cond <= 1 && ret == 0; cond++)
		    {
			for (ctr = 1; ctr <= 2 && ret == 0; ctr++)
			{
			    int decr = cd < 2;
			    int cr = cond ? 0x80000000 : 0x00000000;
			    int jumpc = cc >= 2 ||
					(cc == 0 && !cond) ||
					(cc == 1 && cond);
			    int jumpd = cd >= 2 ||
					(cd == 0 && ctr != 1) ||
					(cd == 1 && ctr == 1);
			    int jump = jumpc && jumpd;

			    ret = cpu_post_test_bc (link ? OP_BCL : OP_BC,
				(cc << 3) + (cd << 1), 0, jump, decr, link,
				ctr, cr);

			    if (ret != 0)
			    {
				post_log ("Error at b3 test %d !\n", i);
			    }

			    i++;
			}
		    }
		}
	    }
	}
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
