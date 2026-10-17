/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

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

#define VERSAL_CRP_BASEADDR	0xF1260000

#define VERSAL_SLCR_BASEADDR	0xF1060000
#define VERSAL_AXI_MUX_SEL	(VERSAL_SLCR_BASEADDR + 0x504)
#define VERSAL_OSPI_LINEAR_MODE	BIT(1)

struct crp_regs {
	u32 reserved0[128];
	u32 boot_mode_usr;
};

#define crp_base ((struct crp_regs *)VERSAL_CRP_BASEADDR)

#define VERSAL_PS_PMC_VERSION	0xF11A0004
#define VERSAL_PS_VER_MASK	GENMASK(7, 0)
#define VERSAL_PS_VER_SHIFT	12

/* Bootmode setting values */
#define BOOT_MODES_MASK	0x0000000F
#define QSPI_MODE_24BIT	0x00000001
#define QSPI_MODE_32BIT	0x00000002
#define SD_MODE		0x00000003 /* sd 0 */
#define SD_MODE1	0x00000005 /* sd 1 */
#define EMMC_MODE	0x00000006
#define USB_MODE	0x00000007
#define OSPI_MODE	0x00000008
#define SELECTMAP_MODE	0x0000000A
#define SD1_LSHFT_MODE	0x0000000E /* SD1 Level shifter */
#define JTAG_MODE	0x00000000
#define BOOT_MODE_USE_ALT	0x100
#define BOOT_MODE_ALT_SHIFT	12

#define FLASH_RESET_GPIO	0xc
#define WPROT_CRP	0xF126001C
#define RST_GPIO	0xF1260318
#define WPROT_LPD_MIO	0xFF080728
#define WPROT_PMC_MIO	0xF1060828
#define BOOT_MODE_DIR	0xF1020204
#define BOOT_MODE_OUT	0xF1020208
#define MIO_PIN_12	0xF1060030
#define BANK0_OUTPUT	0xF1020040
#define BANK0_TRI	0xF1060200
