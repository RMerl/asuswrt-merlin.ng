/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

#define VERSAL_CRL_APB_BASEADDR	0xFF5E0000

#define CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	BIT(25)

#define IOU_SWITCH_CTRL_CLKACT_BIT	BIT(25)
#define IOU_SWITCH_CTRL_DIVISOR0_SHIFT	8

struct crlapb_regs {
	u32 reserved0[67];
	u32 cpu_r5_ctrl;
	u32 reserved;
	u32 iou_switch_ctrl; /* 0x114 */
	u32 reserved1[13];
	u32 timestamp_ref_ctrl; /* 0x14c */
	u32 reserved3[108];
	u32 rst_cpu_r5;
	u32 reserved2[17];
	u32 rst_timestamp; /* 0x348 */
};

#define crlapb_base ((struct crlapb_regs *)VERSAL_CRL_APB_BASEADDR)

#define VERSAL_IOU_SCNTR_SECURE	0xFF140000

#define IOU_SCNTRS_CONTROL_EN	1

struct iou_scntrs_regs {
	u32 counter_control_register; /* 0x0 */
	u32 reserved0[7];
	u32 base_frequency_id_register; /* 0x20 */
};

#define iou_scntr_secure ((struct iou_scntrs_regs *)VERSAL_IOU_SCNTR_SECURE)

#define VERSAL_TCM_BASE_ADDR	0xFFE00000
#define VERSAL_TCM_SIZE		0x40000

#define VERSAL_RPU_BASEADDR	0xFF9A0000

struct rpu_regs {
	u32 rpu_glbl_ctrl;
	u32 reserved0[63];
	u32 rpu0_cfg; /* 0x100 */
	u32 reserved1[63];
	u32 rpu1_cfg; /* 0x200 */
};

#define rpu_base ((struct rpu_regs *)VERSAL_RPU_BASEADDR)
