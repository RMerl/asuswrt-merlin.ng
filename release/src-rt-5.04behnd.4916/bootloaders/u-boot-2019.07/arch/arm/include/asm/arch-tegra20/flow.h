/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010, 2011
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _FLOW_H_
#define _FLOW_H_

struct flow_ctlr {
	u32	halt_cpu_events;
	u32	halt_cop_events;
	u32	cpu_csr;
	u32	cop_csr;
	u32	halt_cpu1_events;
	u32	cpu1_csr;
};

#endif
