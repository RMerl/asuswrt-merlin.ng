/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _LAUNCH_H_
#define _LAUNCH_H_

#ifndef __ASSEMBLY__

struct cpulaunch_t {
	unsigned long pc;
	unsigned long gp;
	unsigned long sp;
	unsigned long a0;
	unsigned long _pad[3]; /* pad to cache line size to avoid thrashing */
	unsigned long flags;
};

extern char launch_wait_code_start;
extern char launch_wait_code_end;

void join_coherent_domain(int ncores);
void boot_vpe1(void);

#else

#define	LAUNCH_PC		0
#define	LAUNCH_GP		4
#define	LAUNCH_SP		8
#define	LAUNCH_A0		12
#define	LAUNCH_FLAGS		28

#endif

#define LOG2CPULAUNCH		5

#define LAUNCH_FREADY		1
#define LAUNCH_FGO		2
#define LAUNCH_FGONE		4

#define LAUNCH_WAITCODE		0x00000d00
#define SCRLAUNCH		0x00000e00
#define CPULAUNCH		0x00000f00
#define NCPULAUNCH		8

/* Polling period in count cycles for secondary CPU's */
#define LAUNCHPERIOD		10000

#endif /* _LAUNCH_H_ */
