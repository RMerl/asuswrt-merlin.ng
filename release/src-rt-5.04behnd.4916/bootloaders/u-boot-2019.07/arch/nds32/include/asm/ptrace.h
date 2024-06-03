/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Copyright (C) 2010 Shawn Lin (nobuhiro@andestech.com)
 * Copyright (C) 2011 Macpaul Lin (macpaul@andestech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_NDS_PTRACE_H
#define __ASM_NDS_PTRACE_H

#define USR_MODE	0x00
#define SU_MODE		0x01
#define HV_MODE		0x10
#define MODE_MASK	(0x03<<3)
#define GIE_BIT		0x01

#ifndef __ASSEMBLY__

/* this struct defines the way the registers are stored on the
   stack during a system call. */

#define NDS32_REG long

struct pt_regs {
	NDS32_REG ir0;
	NDS32_REG ipsw;
	NDS32_REG ipc;
	NDS32_REG sp;
	NDS32_REG orig_r0;
	NDS32_REG pipsw;
	NDS32_REG pipc;
	NDS32_REG pp0;
	NDS32_REG pp1;
	NDS32_REG d0hi;
	NDS32_REG d0lo;
	NDS32_REG d1hi;
	NDS32_REG d1lo;
	NDS32_REG r[26];	/* r0 - r25 */
	NDS32_REG p0;		/* r26 - used by OS */
	NDS32_REG p1;		/* r27 - used by OS */
	NDS32_REG fp;		/* r28 */
	NDS32_REG gp;		/* r29 */
	NDS32_REG lp;		/* r30 */
	NDS32_REG fucop_ctl;
	NDS32_REG osp;
};

#define processor_mode(regs) \
	(((regs)->ipsw & MODE_MASK) >> 3)

#define interrupts_enabled(regs) \
	((regs)->ipsw & GIE_BIT)

/*
 * Offsets used by 'ptrace' system call interface.
 * These can't be changed without breaking binary compatibility
 * with MkLinux, etc.
 */
#define PT_R0	0
#define PT_R1	1
#define PT_R2	2
#define PT_R3	3
#define PT_R4	4
#define PT_R5	5
#define PT_R6	6
#define PT_R7	7
#define PT_R8	8
#define PT_R9	9
#define PT_R10	10
#define PT_R11	11
#define PT_R12	12
#define PT_R13	13
#define PT_R14	14
#define PT_R15	15
#define PT_R16	16
#define PT_R17	17
#define PT_R18	18
#define PT_R19	19
#define PT_R20	20
#define PT_R21	21
#define PT_R22	22
#define PT_R23	23
#define PT_R24	24
#define PT_R25	25

#endif	/* __ASSEMBLY__ */

#endif /* __ASM_NDS_PTRACE_H */
