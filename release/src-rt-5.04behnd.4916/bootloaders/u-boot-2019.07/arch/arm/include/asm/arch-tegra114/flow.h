/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef _TEGRA114_FLOW_H_
#define _TEGRA114_FLOW_H_

struct flow_ctlr {
	u32 halt_cpu_events;
	u32 halt_cop_events;
	u32 cpu_csr;
	u32 cop_csr;
	u32 xrq_events;
	u32 halt_cpu1_events;
	u32 cpu1_csr;
	u32 halt_cpu2_events;
	u32 cpu2_csr;
	u32 halt_cpu3_events;
	u32 cpu3_csr;
	u32 cluster_control;
};

#endif	/* _TEGRA114_FLOW_H_ */
