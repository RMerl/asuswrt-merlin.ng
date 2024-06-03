/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Heiko Stuebner <heiko@sntech.de>
 */

#ifndef _ASM_ARCH_GRF_RK3188_H
#define _ASM_ARCH_GRF_RK3188_H

struct rk3188_grf_gpio_lh {
	u32 l;
	u32 h;
};

struct rk3188_grf {
	struct rk3188_grf_gpio_lh gpio_dir[4];
	struct rk3188_grf_gpio_lh gpio_do[4];
	struct rk3188_grf_gpio_lh gpio_en[4];

	u32 reserved[2];
	u32 gpio0c_iomux;
	u32 gpio0d_iomux;

	u32 gpio1a_iomux;
	u32 gpio1b_iomux;
	u32 gpio1c_iomux;
	u32 gpio1d_iomux;

	u32 gpio2a_iomux;
	u32 gpio2b_iomux;
	u32 gpio2c_iomux;
	u32 gpio2d_iomux;

	u32 gpio3a_iomux;
	u32 gpio3b_iomux;
	u32 gpio3c_iomux;
	u32 gpio3d_iomux;

	u32 soc_con0;
	u32 soc_con1;
	u32 soc_con2;
	u32 soc_status0;

	u32 busdmac_con[3];
	u32 peridmac_con[4];

	u32 cpu_con[6];
	u32 reserved0[2];

	u32 ddrc_con0;
	u32 ddrc_stat;

	u32 io_con[5];
	u32 soc_status1;

	u32 uoc0_con[4];
	u32 uoc1_con[4];
	u32 uoc2_con[2];
	u32 reserved1;
	u32 uoc3_con[2];
	u32 hsic_stat;
	u32 os_reg[8];

	u32 gpio0_p[3];
	u32 gpio1_p[3][4];

	u32 flash_data_p;
	u32 flash_cmd_p;
};
check_member(rk3188_grf, flash_cmd_p, 0x01a4);

/* GRF_SOC_CON0 */
enum {
	HSADC_CLK_DIR_SHIFT	= 15,
	HSADC_CLK_DIR_MASK	= 1,

	HSADC_SEL_SHIFT		= 14,
	HSADC_SEL_MASK		= 1,

	NOC_REMAP_SHIFT		= 12,
	NOC_REMAP_MASK		= 1,

	EMMC_FLASH_SEL_SHIFT	= 11,
	EMMC_FLASH_SEL_MASK	= 1,

	TZPC_REVISION_SHIFT	= 7,
	TZPC_REVISION_MASK	= 0xf,

	L2CACHE_ACC_SHIFT	= 5,
	L2CACHE_ACC_MASK	= 3,

	L2RD_WAIT_SHIFT		= 3,
	L2RD_WAIT_MASK		= 3,

	IMEMRD_WAIT_SHIFT	= 1,
	IMEMRD_WAIT_MASK	= 3,
};

/* GRF_SOC_CON1 */
enum {
	RKI2C4_SEL_SHIFT	= 15,
	RKI2C4_SEL_MASK		= 1,

	RKI2C3_SEL_SHIFT	= 14,
	RKI2C3_SEL_MASK		= 1,

	RKI2C2_SEL_SHIFT	= 13,
	RKI2C2_SEL_MASK		= 1,

	RKI2C1_SEL_SHIFT	= 12,
	RKI2C1_SEL_MASK		= 1,

	RKI2C0_SEL_SHIFT	= 11,
	RKI2C0_SEL_MASK		= 1,

	VCODEC_SEL_SHIFT	= 10,
	VCODEC_SEL_MASK		= 1,

	PERI_EMEM_PAUSE_SHIFT	= 9,
	PERI_EMEM_PAUSE_MASK	= 1,

	PERI_USB_PAUSE_SHIFT	= 8,
	PERI_USB_PAUSE_MASK	= 1,

	SMC_MUX_MODE_0_SHIFT	= 6,
	SMC_MUX_MODE_0_MASK	= 1,

	SMC_SRAM_MW_0_SHIFT	= 4,
	SMC_SRAM_MW_0_MASK	= 3,

	SMC_REMAP_0_SHIFT	= 3,
	SMC_REMAP_0_MASK	= 1,

	SMC_A_GT_M0_SYNC_SHIFT	= 2,
	SMC_A_GT_M0_SYNC_MASK	= 1,

	EMAC_SPEED_SHIFT	= 1,
	EMAC_SPEEC_MASK		= 1,

	EMAC_MODE_SHIFT		= 0,
	EMAC_MODE_MASK		= 1,
};

/* GRF_SOC_CON2 */
enum {
	SDIO_CLK_OUT_SR_SHIFT	= 15,
	SDIO_CLK_OUT_SR_MASK	= 1,

	MEM_EMA_L2C_SHIFT	= 11,
	MEM_EMA_L2C_MASK	= 7,

	MEM_EMA_A9_SHIFT	= 8,
	MEM_EMA_A9_MASK		= 7,

	MSCH4_MAINDDR3_SHIFT	= 7,
	MSCH4_MAINDDR3_MASK	= 1,
	MSCH4_MAINDDR3_DDR3	= 1,

	EMAC_NEWRCV_EN_SHIFT	= 6,
	EMAC_NEWRCV_EN_MASK	= 1,

	SW_ADDR15_EN_SHIFT	= 5,
	SW_ADDR15_EN_MASK	= 1,

	SW_ADDR16_EN_SHIFT	= 4,
	SW_ADDR16_EN_MASK	= 1,

	SW_ADDR17_EN_SHIFT	= 3,
	SW_ADDR17_EN_MASK	= 1,

	BANK2_TO_RANK_EN_SHIFT	= 2,
	BANK2_TO_RANK_EN_MASK	= 1,

	RANK_TO_ROW15_EN_SHIFT	= 1,
	RANK_TO_ROW15_EN_MASK	= 1,

	UPCTL_C_ACTIVE_IN_SHIFT = 0,
	UPCTL_C_ACTIVE_IN_MASK	= 1,
	UPCTL_C_ACTIVE_IN_MAY	= 0,
	UPCTL_C_ACTIVE_IN_WILL,
};

/* GRF_DDRC_CON0 */
enum {
	DDR_16BIT_EN_SHIFT	= 15,
	DDR_16BIT_EN_MASK	= 1,

	DTO_LB_SHIFT		= 11,
	DTO_LB_MASK		= 3,

	DTO_TE_SHIFT		= 9,
	DTO_TE_MASK		= 3,

	DTO_PDR_SHIFT		= 7,
	DTO_PDR_MASK		= 3,

	DTO_PDD_SHIFT		= 5,
	DTO_PDD_MASK		= 3,

	DTO_IOM_SHIFT		= 3,
	DTO_IOM_MASK		= 3,

	DTO_OE_SHIFT		= 1,
	DTO_OE_MASK		= 3,

	ATO_AE_SHIFT		= 0,
	ATO_AE_MASK		= 1,
};

/* GRF_UOC_CON0 */
enum {
	SIDDQ_SHIFT		= 13,
	SIDDQ_MASK		= 1 << SIDDQ_SHIFT,

	BYPASSSEL_SHIFT		= 9,
	BYPASSSEL_MASK		= 1 << BYPASSSEL_SHIFT,

	BYPASSDMEN_SHIFT	= 8,
	BYPASSDMEN_MASK		= 1 << BYPASSDMEN_SHIFT,

	UOC_DISABLE_SHIFT	= 4,
	UOC_DISABLE_MASK	= 1 << UOC_DISABLE_SHIFT,

	COMMON_ON_N_SHIFT	= 0,
	COMMON_ON_N_MASK	= 1 << COMMON_ON_N_SHIFT,
};

/* GRF_UOC_CON2 */
enum {
	SOFT_CON_SEL_SHIFT	= 2,
	SOFT_CON_SEL_MASK	= 1 << SOFT_CON_SEL_SHIFT,
};

/* GRF_UOC0_CON3 */
enum {
	TERMSEL_FULLSPEED_SHIFT	= 5,
	TERMSEL_FULLSPEED_MASK	= 1 << TERMSEL_FULLSPEED_SHIFT,

	XCVRSELECT_SHIFT	= 3,
	XCVRSELECT_FSTRANSC	= 1,
	XCVRSELECT_MASK		= 3 << XCVRSELECT_SHIFT,

	OPMODE_SHIFT		= 1,
	OPMODE_NODRIVING	= 1,
	OPMODE_MASK		= 3 << OPMODE_SHIFT,

	SUSPENDN_SHIFT		= 0,
	SUSPENDN_MASK		= 1 << SUSPENDN_SHIFT,
};

#endif
