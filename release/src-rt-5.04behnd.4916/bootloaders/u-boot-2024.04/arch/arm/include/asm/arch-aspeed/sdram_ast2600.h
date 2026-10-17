/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#ifndef _ASM_ARCH_SDRAM_AST2600_H
#define _ASM_ARCH_SDRAM_AST2600_H

/* keys for unlocking HW */
#define SDRAM_UNLOCK_KEY		0xFC600309
#define SDRAM_VIDEO_UNLOCK_KEY		0x00440003

/* Fixed priority DRAM Requests mask */
#define REQ_PRI_VGA_HW_CURSOR_R         0
#define REQ_PRI_VGA_CRT_R               1
#define REQ_PRI_SOC_DISPLAY_CTRL_R      2
#define REQ_PRI_PCIE_BUS1_RW            3
#define REQ_PRI_VIDEO_HIGH_PRI_W        4
#define REQ_PRI_CPU_RW                  5
#define REQ_PRI_SLI_RW                  6
#define REQ_PRI_PCIE_BUS2_RW            7
#define REQ_PRI_USB2_0_HUB_EHCI1_DMA_RW 8
#define REQ_PRI_USB2_0_DEV_EHCI2_DMA_RW 9
#define REQ_PRI_USB1_1_UHCI_HOST_RW     10
#define REQ_PRI_AHB_BUS_RW              11
#define REQ_PRI_CM3_DATA_RW             12
#define REQ_PRI_CM3_INST_R              13
#define REQ_PRI_MAC0_DMA_RW             14
#define REQ_PRI_MAC1_DMA_RW             15
#define REQ_PRI_SDIO_DMA_RW             16
#define REQ_PRI_PILOT_ENGINE_RW         17
#define REQ_PRI_XDMA1_RW                18
#define REQ_PRI_MCTP1_RW                19
#define REQ_PRI_VIDEO_FLAG_RW           20
#define REQ_PRI_VIDEO_LOW_PRI_W         21
#define REQ_PRI_2D_ENGINE_DATA_RW       22
#define REQ_PRI_ENC_ENGINE_RW           23
#define REQ_PRI_MCTP2_RW                24
#define REQ_PRI_XDMA2_RW                25
#define REQ_PRI_ECC_RSA_RW              26

#define MCR30_RESET_DLL_DELAY_EN	BIT(4)
#define MCR30_MODE_REG_SEL_SHIFT	1
#define MCR30_MODE_REG_SEL_MASK		GENMASK(3, 1)
#define MCR30_SET_MODE_REG		BIT(0)

#define MCR30_SET_MR(mr) ((mr << MCR30_MODE_REG_SEL_SHIFT) | MCR30_SET_MODE_REG)

#define MCR34_SELF_REFRESH_STATUS_MASK	GENMASK(30, 28)

#define MCR34_ODT_DELAY_SHIFT		12
#define MCR34_ODT_DELAY_MASK		GENMASK(15, 12)
#define MCR34_ODT_EXT_SHIFT		10
#define MCR34_ODT_EXT_MASK		GENMASK(11, 10)
#define MCR34_ODT_AUTO_ON		BIT(9)
#define MCR34_ODT_EN			BIT(8)
#define MCR34_RESETN_DIS		BIT(7)
#define MCR34_MREQI_DIS			BIT(6)
#define MCR34_MREQ_BYPASS_DIS		BIT(5)
#define MCR34_RGAP_CTRL_EN		BIT(4)
#define MCR34_CKE_OUT_IN_SELF_REF_DIS	BIT(3)
#define MCR34_FOURCE_SELF_REF_EN	BIT(2)
#define MCR34_AUTOPWRDN_EN		BIT(1)
#define MCR34_CKE_EN			BIT(0)

#define MCR38_RW_MAX_GRANT_CNT_RQ_SHIFT	16
#define MCR38_RW_MAX_GRANT_CNT_RQ_MASK	GENMASK(20, 16)

/* default request queued limitation mask (0xFFBBFFF4) */
#define MCR3C_DEFAULT_MASK                                                     \
	~(REQ_PRI_VGA_HW_CURSOR_R | REQ_PRI_VGA_CRT_R | REQ_PRI_PCIE_BUS1_RW | \
	  REQ_PRI_XDMA1_RW | REQ_PRI_2D_ENGINE_DATA_RW)

#define MCR50_RESET_ALL_INTR		BIT(31)
#define SDRAM_CONF_ECC_AUTO_SCRUBBING	BIT(9)
#define SDRAM_CONF_SCRAMBLE		BIT(8)
#define SDRAM_CONF_ECC_EN		BIT(7)
#define SDRAM_CONF_DUALX8		BIT(5)
#define SDRAM_CONF_DDR4			BIT(4)
#define SDRAM_CONF_VGA_SIZE_SHIFT	2
#define SDRAM_CONF_VGA_SIZE_MASK	GENMASK(3, 2)
#define SDRAM_CONF_CAP_SHIFT		0
#define SDRAM_CONF_CAP_MASK		GENMASK(1, 0)

#define SDRAM_CONF_CAP_256M		0
#define SDRAM_CONF_CAP_512M		1
#define SDRAM_CONF_CAP_1024M		2
#define SDRAM_CONF_CAP_2048M		3
#define SDRAM_CONF_ECC_SETUP		(SDRAM_CONF_ECC_AUTO_SCRUBBING | SDRAM_CONF_ECC_EN)

#define SDRAM_MISC_DDR4_TREFRESH	(1 << 3)

#define SDRAM_PHYCTRL0_PLL_LOCKED	BIT(4)
#define SDRAM_PHYCTRL0_NRST		BIT(2)
#define SDRAM_PHYCTRL0_INIT		BIT(0)

/* MCR0C */
#define SDRAM_REFRESH_PERIOD_ZQCS_SHIFT	16
#define SDRAM_REFRESH_PERIOD_ZQCS_MASK	GENMASK(31, 16)
#define SDRAM_REFRESH_PERIOD_SHIFT	8
#define SDRAM_REFRESH_PERIOD_MASK	GENMASK(15, 8)
#define SDRAM_REFRESH_ZQCS_EN		BIT(7)
#define SDRAM_RESET_DLL_ZQCL_EN		BIT(6)
#define SDRAM_LOW_PRI_REFRESH_EN	BIT(5)
#define SDRAM_FORCE_PRECHARGE_EN	BIT(4)
#define SDRAM_REFRESH_EN		BIT(0)

/* MCR14 */
#define SDRAM_WL_SETTING		GENMASK(23, 20)
#define SDRAM_CL_SETTING		GENMASK(19, 16)

#define SDRAM_TEST_LEN_SHIFT		4
#define SDRAM_TEST_LEN_MASK		0xfffff
#define SDRAM_TEST_START_ADDR_SHIFT	24
#define SDRAM_TEST_START_ADDR_MASK	0x3f

#define SDRAM_TEST_EN			(1 << 0)
#define SDRAM_TEST_MODE_SHIFT		1
#define SDRAM_TEST_MODE_MASK		(0x3 << SDRAM_TEST_MODE_SHIFT)
#define SDRAM_TEST_MODE_WO		(0x0 << SDRAM_TEST_MODE_SHIFT)
#define SDRAM_TEST_MODE_RB		(0x1 << SDRAM_TEST_MODE_SHIFT)
#define SDRAM_TEST_MODE_RW		(0x2 << SDRAM_TEST_MODE_SHIFT)

#define SDRAM_TEST_GEN_MODE_SHIFT	3
#define SDRAM_TEST_GEN_MODE_MASK	(7 << SDRAM_TEST_GEN_MODE_SHIFT)
#define SDRAM_TEST_TWO_MODES		(1 << 6)
#define SDRAM_TEST_ERRSTOP		(1 << 7)
#define SDRAM_TEST_DONE			(1 << 12)
#define SDRAM_TEST_FAIL			(1 << 13)

#define SDRAM_AC_TRFC_SHIFT		0
#define SDRAM_AC_TRFC_MASK		0xff

#define SDRAM_ECC_RANGE_ADDR_MASK	GENMASK(30, 20)
#define SDRAM_ECC_RANGE_ADDR_SHIFT	20

#ifndef __ASSEMBLY__
struct ast2600_sdrammc_regs {
	u32 protection_key;		/* offset 0x00 */
	u32 config;			/* offset 0x04 */
	u32 gm_protection_key;		/* offset 0x08 */
	u32 refresh_timing;		/* offset 0x0C */
	u32 ac_timing[4];		/* offset 0x10 ~ 0x1C */
	u32 mr01_mode_setting;		/* offset 0x20 */
	u32 mr23_mode_setting;		/* offset 0x24 */
	u32 mr45_mode_setting;		/* offset 0x28 */
	u32 mr6_mode_setting;		/* offset 0x2C */
	u32 mode_setting_control;	/* offset 0x30 */
	u32 power_ctrl;			/* offset 0x34 */
	u32 arbitration_ctrl;		/* offset 0x38 */
	u32 req_limit_mask;		/* offset 0x3C */
	u32 max_grant_len[4];		/* offset 0x40 ~ 0x4C */
	u32 intr_ctrl;			/* offset 0x50 */
	u32 ecc_range_ctrl;		/* offset 0x54 */
	u32 first_ecc_err_addr;		/* offset 0x58 */
	u32 last_ecc_err_addr;		/* offset 0x5C */
	u32 phy_ctrl[4];		/* offset 0x60 ~ 0x6C */
	u32 ecc_test_ctrl;		/* offset 0x70 */
	u32 test_addr;			/* offset 0x74 */
	u32 test_fail_dq_bit;		/* offset 0x78 */
	u32 test_init_val;		/* offset 0x7C */
	u32 req_input_ctrl;		/* offset 0x80 */
	u32 req_high_pri_ctrl;		/* offset 0x84 */
	u32 reserved0[6];		/* offset 0x88 ~ 0x9C */
};
#endif  /* __ASSEMBLY__ */

#endif  /* _ASM_ARCH_SDRAM_AST2600_H */
