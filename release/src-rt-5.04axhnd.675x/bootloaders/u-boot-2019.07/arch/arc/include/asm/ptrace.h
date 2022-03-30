/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. All rights reserved.
 */

#ifndef __ASM_ARC_PTRACE_H
#define __ASM_ARC_PTRACE_H

struct pt_regs {
	long bta;
	long lp_start;
	long lp_end;
	long lp_count;
	long status32;
	long ret;
	long blink;
	long fp;
	long r26;	/* gp */
	long r25;
	long r24;
	long r23;
	long r22;
	long r21;
	long r20;
	long r19;
	long r18;
	long r17;
	long r16;
	long r15;
	long r14;
	long r13;
	long r12;
	long r11;
	long r10;
	long r9;
	long r8;
	long r7;
	long r6;
	long r5;
	long r4;
	long r3;
	long r2;
	long r1;
	long r0;
	long sp;
	long ecr;
};

#endif /* __ASM_ARC_PTRACE_H */
