/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _M68K_PTRACE_H
#define _M68K_PTRACE_H

/*
 * This struct defines the way the registers are stored on the
 * kernel stack during an exception.
 */
#ifndef __ASSEMBLY__

struct pt_regs {
	ulong d0;
	ulong d1;
	ulong d2;
	ulong d3;
	ulong d4;
	ulong d5;
	ulong d6;
	ulong d7;
	ulong a0;
	ulong a1;
	ulong a2;
	ulong a3;
	ulong a4;
	ulong a5;
	ulong a6;
#if defined(__M68K__)
	unsigned format:4;	/* frame format specifier */
	unsigned vector:12;	/* vector offset */
	unsigned short sr;
	unsigned long pc;
#else
	unsigned short sr;
	unsigned long pc;
#endif
};

#endif				/* #ifndef __ASSEMBLY__ */

#endif				/* #ifndef _M68K_PTRACE_H */
