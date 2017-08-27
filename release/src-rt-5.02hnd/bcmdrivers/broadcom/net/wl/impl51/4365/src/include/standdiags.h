/*
 * Definitions for standalone diagnostics.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: standdiags.h 295331 2011-11-10 02:58:01Z $
 */

/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

typedef struct {
#if defined(mips)
	/*
	 * The mips "status" area:
	 *
	 * 0x0000f00	CP0:PRid	CP0:Status	CP0:Config	CP0:Config1
	 * 0x0000f10	D$size		I$size		watermark	sysClkInt counter
	 * 0x0000f20	CP0:BRCMcfg0b	CP0:PLLcfg1	CP0:PLLcfg2	CP0:ClkSync
	 * 0x0000f30	CP0:PLLcfg3	CP0:ResetCfg	CP0:BRCMcfg0	CP0:Config2
	 * 0x0000f40	CP0:Config3	CP0:Config7	CP0:IntCtl	CP0HWREna
	 * 0x0000f50	CP0:SRSCtl	CP0:SRSMap	CP0:EBase
	 *
	 * The "trap" area:
	 *
	 * 0x0000f60	k0		CP0:Status	CP0:Cause	CP0:epc
	 * 0x0000f70	CP0:badvaddr	CP0:errorPC	CP0:BRCMcfg0
	 * 0x0000f80	r0(zero)	r1(at)		r2(v0)		r3(v1)
	 * 0x0000f90	r4(a0)		r5(a1)		r6(a2)		r7(a3)
	 * 0x0000fa0	r8(t0)		r9(t1)		r10(t2)		r11(t3)
	 * 0x0000fb0	r12(t4)		r13(t5)		r14(t6)		r15(t7)
	 * 0x0000fc0	r16(s0)		r17(s1)		r18(s2)		r19(s3)
	 * 0x0000fd0	r20(s4)		r21(s5)		r22(s6)		r23(s7)
	 * 0x0000fe0	r24(t8)		r25(t9)		r26(k0)		r27(k1)
	 * 0x0000ff0	r28(gp)		r29(sp)		r30(s8/fp)	r31(ra)
	 */

	uint32	prid;
	uint32	st;
	uint32	cf;
	uint32	cf1;
	uint32	dc;
	uint32	ic;
	uint32	wm;
	uint32	clk;
	uint32	brcm0b;
	uint32	pll1;
	uint32	pll2;
	uint32	clksync;
	uint32	pll3;
	uint32	rst;
	uint32	brcm0;
	uint32	cf2;
	uint32	cf3;
	uint32	cf7;
	uint32	intctl;
	uint32	hwren;
	uint32	srsctl;
	uint32	srsmap;
	uint32	ebase;
	uint32	PAD;

	uint32	k0;
	uint32	Status;
	uint32	Cause;
	uint32	epc;
	uint32	badvaddr;
	uint32	errorPC;
	uint32	BRCMcfg0;
	uint32	PAD;

	uint32	r0;		/* (zero) */
	uint32	r1;		/* (at)	 */
	uint32	r2;		/* (v0)	 */
	uint32	r3;		/* (v1) */
	uint32	r4;		/* (a0) */
	uint32	r5;		/* (a1) */
	uint32	r6;		/* (a2) */
	uint32	r7;		/* (a3) */
	uint32	r8;		/* (t0) */
	uint32	r9;		/* (t1) */
	uint32	r10;		/* (t2) */
	uint32	r11;		/* (t3) */
	uint32	r12;		/* (t4) */
	uint32	r13;		/* (t5) */
	uint32	r14;		/* (t6) */
	uint32	r15;		/* (t7) */
	uint32	r16;		/* (s0) */
	uint32	r17;		/* (s1) */
	uint32	r18;		/* (s2) */
	uint32	r19;		/* (s3) */
	uint32	r20;		/* (s4) */
	uint32	r21;		/* (s5) */
	uint32	r22;		/* (s6) */
	uint32	r23;		/* (s7) */
	uint32	r24;		/* (t8) */
	uint32	r25;		/* (t9) */
	uint32	r26;		/* (k0) */
	uint32	r27;		/* (k1) */
	uint32	r28;		/* (gp) */
	uint32	r29;		/* (sp) */
	uint32	r30;		/* (s8/fp) */
	uint32	r31;		/* (ra) */
#else
	/*
	 * The arm "status" area:
	 *
	 * 0x0000f00	cpsr		spsr		CP15:idcode	CP15:ctrl
	 * 0x0000f10	CP15:auxctrl	CP15:cpacc	watermark	clock
	 * 0x0000f20	CP15:pid	CP15:cachetype	resetlog	freetimer
	 * 0x0000f30	CP15:TCMstat	CP15:MPU/TLBtype CP15:procf0	CP15:procf1
	 * 0x0000f40	CP15:debugf0	CP15:auxf0	CP15:mmf0	CP15:mmf1
	 * 0x0000f50	CP15:mmf2	CP15:mmf3	CP15:isf0	CP15:isf1
	 * 0x0000f60	CP15:isf2	CP15:isf3	CP15:isf4	CP15:isf5
	 *
	 * The "trap" area:
	 *
	 * 0x0000f80	r0		r1		r2		r3
	 * 0x0000f90	r4		r5		r6		r7
	 * 0x0000fa0	r8		r9		r10		r11
	 * 0x0000fb0	r12		r13		r14		r15(pc)
	 * 0x0000fc0	cpsr		spsr		sr13		sr14
	 */

	uint32	cpsr;
	uint32	spsr;
	uint32	idcode;
	uint32	ctrl;
	uint32	auxctrl;
	uint32	cpacc;
	uint32	wm;
	uint32	clk;
	uint32	pid;
	uint32	cachetype;
	uint32	resetlog;
	uint32	freetimer;
	uint32	TCMstat;
	uint32	MPUtype;
	uint32	procf0;
	uint32	procf1;
	uint32	debugf0;
	uint32	auxf0;
	uint32	mmf0;
	uint32	mmf1;
	uint32	mmf2;
	uint32	mmf3;
	uint32	isf0;
	uint32	isf1;
	uint32	isf2;
	uint32	isf3;
	uint32	isf4;
	uint32	isf5;
	uint32	PAD[4];
	uint32	r0;
	uint32	r1;
	uint32	r2;
	uint32	r3;
	uint32	r4;
	uint32	r5;
	uint32	r6;
	uint32	r7;
	uint32	r8;
	uint32	r9;
	uint32	r10;
	uint32	r11;
	uint32	r12;
	uint32	r13;
	uint32	r14;
	uint32	r15;
	uint32	tr_cpsr;
	uint32	tr_spsr;
	uint32	sr13;
	uint32	sr14;
#endif	/* mips vs arm */
} statarea_t;

#if defined(mips)
#define	stat_off	0xf00
#else
#define	stat_off	0x4000
#endif

#define	swaptest_off	0xe00
