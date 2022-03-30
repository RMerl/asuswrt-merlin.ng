/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2013
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA124_FLOW_H_
#define _TEGRA124_FLOW_H_

struct flow_ctlr {
	u32 halt_cpu_events;	/* offset 0x00 */
	u32 halt_cop_events;	/* offset 0x04 */
	u32 cpu_csr;		/* offset 0x08 */
	u32 cop_csr;		/* offset 0x0c */
	u32 xrq_events;		/* offset 0x10 */
	u32 halt_cpu1_events;	/* offset 0x14 */
	u32 cpu1_csr;		/* offset 0x18 */
	u32 halt_cpu2_events;	/* offset 0x1c */
	u32 cpu2_csr;		/* offset 0x20 */
	u32 halt_cpu3_events;	/* offset 0x24 */
	u32 cpu3_csr;		/* offset 0x28 */
	u32 cluster_control;	/* offset 0x2c */
	u32 halt_cop1_events;	/* offset 0x30 */
	u32 halt_cop1_csr;	/* offset 0x34 */
	u32 cpu_pwr_csr;	/* offset 0x38 */
	u32 mpid;		/* offset 0x3c */
	u32 ram_repair;		/* offset 0x40 */
	u32 flow_dbg_sel;	/* offset 0x44 */
	u32 flow_dbg_cnt0;	/* offset 0x48 */
	u32 flow_dbg_cnt1;	/* offset 0x4c */
	u32 flow_dbg_qual;	/* offset 0x50 */
	u32 flow_ctrl_spare;	/* offset 0x54 */
	u32 ram_repair_cluster1;/* offset 0x58 */
};

/* HALT_COP_EVENTS_0, 0x04 */
#define EVENT_MSEC		(1 << 24)
#define EVENT_USEC		(1 << 25)
#define EVENT_JTAG		(1 << 28)
#define EVENT_MODE_STOP		(2 << 29)

/* FLOW_CTLR_CLUSTER_CONTROL_0 0x2c */
#define ACTIVE_LP		(1 << 0)

/* CPUn_CSR_0 */
#define CSR_ENABLE		(1 << 0)
#define CSR_IMMEDIATE_WAKE	(1 << 3)
#define CSR_WAIT_WFI_SHIFT	8
#define CSR_PWR_OFF_STS		(1 << 16)

#define RAM_REPAIR_REQ		BIT(0)
#define RAM_REPAIR_STS		BIT(1)
#define RAM_REPAIR_BYPASS_EN	BIT(2)

#endif	/*  _TEGRA124_FLOW_H_ */
