/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _ASM_X86_IST_H
#define _ASM_X86_IST_H

/*
 * Include file for the interface to IST BIOS
 * Copyright 2002 Andy Grover <andrew.grover@intel.com>
 */


#include <linux/types.h>

struct ist_info {
	__u32 signature;
	__u32 command;
	__u32 event;
	__u32 perf_level;
};

#ifdef __KERNEL__

extern struct ist_info ist_info;

#endif	/* __KERNEL__ */
#endif /* _ASM_X86_IST_H */
