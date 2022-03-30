/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 */
#ifndef _ASM_ARCH_SDRAM_AST2500_H
#define _ASM_ARCH_SDRAM_AST2500_H

#define SDRAM_UNLOCK_KEY		0xfc600309
#define SDRAM_VIDEO_UNLOCK_KEY		0x2003000f

#define SDRAM_PCR_CKE_EN		(1 << 0)
#define SDRAM_PCR_AUTOPWRDN_EN		(1 << 1)
#define SDRAM_PCR_CKE_DELAY_SHIFT	4
#define SDRAM_PCR_CKE_DELAY_MASK	7
#define SDRAM_PCR_RESETN_DIS		(1 << 7)
#define SDRAM_PCR_ODT_EN		(1 << 8)
#define SDRAM_PCR_ODT_AUTO_ON		(1 << 10)
#define SDRAM_PCR_ODT_EXT_EN		(1 << 11)
#define SDRAM_PCR_TCKE_PW_SHIFT		12
#define SDRAM_PCR_TCKE_PW_MASK		7
#define SDRAM_PCR_RGAP_CTRL_EN		(1 << 15)
#define SDRAM_PCR_MREQI_DIS		(1 << 17)

/* Fixed priority DRAM Requests mask */
#define SDRAM_REQ_VGA_HW_CURSOR		(1 << 0)
#define SDRAM_REQ_VGA_TEXT_CG_FONT	(1 << 1)
#define SDRAM_REQ_VGA_TEXT_ASCII	(1 << 2)
#define SDRAM_REQ_VGA_CRT		(1 << 3)
#define SDRAM_REQ_SOC_DC_CURSOR		(1 << 4)
#define SDRAM_REQ_SOC_DC_OCD		(1 << 5)
#define SDRAM_REQ_SOC_DC_CRT		(1 << 6)
#define SDRAM_REQ_VIDEO_HIPRI_WRITE	(1 << 7)
#define SDRAM_REQ_USB20_EHCI1		(1 << 8)
#define SDRAM_REQ_USB20_EHCI2		(1 << 9)
#define SDRAM_REQ_CPU			(1 << 10)
#define SDRAM_REQ_AHB2			(1 << 11)
#define SDRAM_REQ_AHB			(1 << 12)
#define SDRAM_REQ_MAC0			(1 << 13)
#define SDRAM_REQ_MAC1			(1 << 14)
#define SDRAM_REQ_PCIE			(1 << 16)
#define SDRAM_REQ_XDMA			(1 << 17)
#define SDRAM_REQ_ENCRYPTION		(1 << 18)
#define SDRAM_REQ_VIDEO_FLAG		(1 << 21)
#define SDRAM_REQ_VIDEO_LOW_PRI_WRITE	(1 << 28)
#define SDRAM_REQ_2D_RW			(1 << 29)
#define SDRAM_REQ_MEMCHECK		(1 << 30)

#define SDRAM_ICR_RESET_ALL		(1 << 31)

#define SDRAM_CONF_CAP_SHIFT		0
#define SDRAM_CONF_CAP_MASK		3
#define SDRAM_CONF_DDR4			(1 << 4)
#define SDRAM_CONF_SCRAMBLE		(1 << 8)
#define SDRAM_CONF_SCRAMBLE_PAT2	(1 << 9)
#define SDRAM_CONF_CACHE_EN		(1 << 10)
#define SDRAM_CONF_CACHE_INIT_EN	(1 << 12)
#define SDRAM_CONF_DUALX8		(1 << 13)
#define SDRAM_CONF_CACHE_INIT_DONE	(1 << 19)

#define SDRAM_CONF_CAP_128M		0
#define SDRAM_CONF_CAP_256M		1
#define SDRAM_CONF_CAP_512M		2
#define SDRAM_CONF_CAP_1024M		3

#define SDRAM_MISC_DDR4_TREFRESH	(1 << 3)

#define SDRAM_PHYCTRL0_INIT		(1 << 0)
#define SDRAM_PHYCTRL0_AUTO_UPDATE	(1 << 1)
#define SDRAM_PHYCTRL0_NRST		(1 << 2)

#define SDRAM_REFRESH_CYCLES_SHIFT	0
#define SDRAM_REFRESH_CYCLES_MASK	0xf
#define SDRAM_REFRESH_ZQCS_EN		(1 << 7)
#define SDRAM_REFRESH_PERIOD_SHIFT	8
#define SDRAM_REFRESH_PERIOD_MASK	0xf

#define SDRAM_TEST_LEN_SHIFT		4
#define SDRAM_TEST_LEN_MASK		0xfffff
#define SDRAM_TEST_START_ADDR_SHIFT	24
#define SDRAM_TEST_START_ADDR_MASK	0x3f

#define SDRAM_TEST_EN			(1 << 0)
#define SDRAM_TEST_MODE_SHIFT		1
#define SDRAM_TEST_MODE_MASK		3
#define SDRAM_TEST_MODE_WO		0
#define SDRAM_TEST_MODE_RB		1
#define SDRAM_TEST_MODE_RW		2
#define SDRAM_TEST_GEN_MODE_SHIFT	3
#define SDRAM_TEST_GEN_MODE_MASK	7
#define SDRAM_TEST_TWO_MODES		(1 << 6)
#define SDRAM_TEST_ERRSTOP		(1 << 7)
#define SDRAM_TEST_DONE			(1 << 12)
#define SDRAM_TEST_FAIL			(1 << 13)

#define SDRAM_AC_TRFC_SHIFT		0
#define SDRAM_AC_TRFC_MASK		0xff

#ifndef __ASSEMBLY__

struct ast2500_sdrammc_regs {
	u32 protection_key;
	u32 config;
	u32 gm_protection_key;
	u32 refresh_timing;
	u32 ac_timing[3];
	u32 misc_control;
	u32 mr46_mode_setting;
	u32 mr5_mode_setting;
	u32 mode_setting_control;
	u32 mr02_mode_setting;
	u32 mr13_mode_setting;
	u32 power_control;
	u32 req_limit_mask;
	u32 pri_group_setting;
	u32 max_grant_len[4];
	u32 intr_ctrl;
	u32 ecc_range_ctrl;
	u32 first_ecc_err_addr;
	u32 last_ecc_err_addr;
	u32 phy_ctrl[4];
	u32 ecc_test_ctrl;
	u32 test_addr;
	u32 test_fail_dq_bit;
	u32 test_init_val;
	u32 phy_debug_ctrl;
	u32 phy_debug_data;
	u32 reserved1[30];
	u32 scu_passwd;
	u32 reserved2[7];
	u32 scu_mpll;
	u32 reserved3[19];
	u32 scu_hwstrap;
};

#endif  /* __ASSEMBLY__ */

#endif  /* _ASM_ARCH_SDRAM_AST2500_H */
