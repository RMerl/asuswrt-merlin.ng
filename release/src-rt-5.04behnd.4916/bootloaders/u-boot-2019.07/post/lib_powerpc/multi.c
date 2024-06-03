// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Load/store multiple word instructions:	lmw, stmw
 *
 * 27 consecutive words are loaded from a source memory buffer
 * into GPRs r5 through r31. After that, 27 consecutive words are stored
 * from the GPRs r5 through r31 into a target memory buffer. The contents
 * of the source and target buffers are then compared.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern void cpu_post_exec_02(ulong *code, ulong op1, ulong op2);

int cpu_post_test_multi(void)
{
	int ret = 0;
	unsigned int i;
	ulong src[27], dst[27];
	int flag = disable_interrupts();

	ulong code[] = {
		ASM_LMW(5, 3, 0),	/* lmw	r5, 0(r3)	*/
		ASM_STMW(5, 4, 0),	/* stmr	r5, 0(r4)	*/
		ASM_BLR,		/* blr			*/
	};

	for (i = 0; i < ARRAY_SIZE(src); ++i) {
		src[i] = i;
		dst[i] = 0;
	}

	cpu_post_exec_02(code, (ulong) src, (ulong) dst);

	ret = memcmp(src, dst, sizeof(dst)) == 0 ? 0 : -1;

	if (ret != 0)
		post_log("Error at multi test !\n");

	if (flag)
		enable_interrupts();

	return ret;
}

#endif
