/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 - 2022, Xilinx, Inc.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.
 */

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

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

struct iou_scntrs_regs {
	u32 counter_control_register; /* 0x0 */
	u32 reserved0[7];
	u32 base_frequency_id_register; /* 0x20 */
};

struct crp_regs {
	u32 reserved0[128];
	u32 boot_mode_usr;	/* 0x200 */
};

#define VERSAL_NET_CRL_APB_BASEADDR		0xEB5E0000
#define VERSAL_NET_CRP_BASEADDR			0xF1260000
#define VERSAL_NET_IOU_SCNTR_SECURE		0xEC920000

#define CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	BIT(25)
#define IOU_SWITCH_CTRL_CLKACT_BIT		BIT(25)
#define IOU_SWITCH_CTRL_DIVISOR0_SHIFT		8
#define IOU_SCNTRS_CONTROL_EN			1

#define crlapb_base ((struct crlapb_regs *)VERSAL_NET_CRL_APB_BASEADDR)
#define crp_base ((struct crp_regs *)VERSAL_NET_CRP_BASEADDR)
#define iou_scntr_secure ((struct iou_scntrs_regs *)VERSAL_NET_IOU_SCNTR_SECURE)

#define PMC_TAP	0xF11A0000

#define PMC_TAP_IDCODE		(PMC_TAP + 0)
#define PMC_TAP_VERSION		(PMC_TAP + 0x4)
# define PMC_VERSION_MASK	GENMASK(7, 0)
# define PS_VERSION_MASK	GENMASK(15, 8)
# define PS_VERSION_PRODUCTION	0x20
# define RTL_VERSION_MASK	GENMASK(23, 16)
# define PLATFORM_MASK		GENMASK(27, 24)
# define PLATFORM_VERSION_MASK	GENMASK(31, 28)
#define PMC_TAP_USERCODE	(PMC_TAP + 0x8)

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

enum versal_net_platform {
	VERSAL_NET_SILICON = 0,
	VERSAL_NET_SPP = 1,
	VERSAL_NET_EMU = 2,
	VERSAL_NET_QEMU = 3,
};

#define VERSAL_SLCR_BASEADDR	0xF1060000
#define VERSAL_AXI_MUX_SEL	(VERSAL_SLCR_BASEADDR + 0x504)
#define VERSAL_OSPI_LINEAR_MODE	BIT(1)

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
