/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018 Alexander Graf <agraf@suse.de>
 */

#ifndef _SETJMP_H_
#define _SETJMP_H_	1

/*
 * This really should be opaque, but the EFI implementation wrongly
 * assumes that a 'struct jmp_buf_data' is defined.
 */
struct jmp_buf_data {
	/* x2, x8, x9, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, sp */
	unsigned long s_regs[12];	/* s0 - s11 */
	unsigned long ra;
	unsigned long sp;
};

typedef struct jmp_buf_data jmp_buf[1];

int setjmp(jmp_buf jmp);
void longjmp(jmp_buf jmp, int ret);

#endif /* _SETJMP_H_ */
