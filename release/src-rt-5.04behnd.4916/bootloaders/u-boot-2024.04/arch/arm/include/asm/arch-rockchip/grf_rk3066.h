/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Paweł Jarosz <paweljarosz3691@gmail.com>
 */

#ifndef _ASM_ARCH_GRF_RK3066_H
#define _ASM_ARCH_GRF_RK3066_H

#include <linux/bitops.h>
#include <linux/bitfield.h>

#define REG(name, h, l) \
	name##_MASK = GENMASK(h, l), \
	name##_SHIFT = __bf_shf(name##_MASK)

struct rk3066_grf_gpio_lh {
	u32 l;
	u32 h;
};

struct rk3066_grf {
	struct rk3066_grf_gpio_lh gpio_dir[7];
	struct rk3066_grf_gpio_lh gpio_do[7];
	struct rk3066_grf_gpio_lh gpio_en[7];

	u32 gpio0a_iomux;
	u32 gpio0b_iomux;
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

	u32 gpio4a_iomux;
	u32 gpio4b_iomux;
	u32 gpio4c_iomux;
	u32 gpio4d_iomux;

	u32 reserved0[5];

	u32 gpio6b_iomux;

	u32 reserved1[2];

	struct rk3066_grf_gpio_lh gpio_pull[7];

	u32 soc_con0;
	u32 soc_con1;
	u32 soc_con2;

	u32 soc_status0;

	u32 dmac1_con[3];
	u32 dmac2_con[4];

	u32 uoc0_con[3];
	u32 uoc1_con[4];
	u32 ddrc_con;
	u32 ddrc_stat;

	u32 reserved2[10];

	u32 os_reg[4];
};

check_member(rk3066_grf, os_reg[3], 0x01d4);

/* GRF_GPIO1B_IOMUX */
enum {
	REG(GPIO1B1, 2, 2),
	GPIO1B1_GPIO		= 0,
	GPIO1B1_UART2_SOUT,

	REG(GPIO1B0, 0, 0),
	GPIO1B0_GPIO		= 0,
	GPIO1B0_UART2_SIN
};

/* GRF_GPIO3B_IOMUX */
enum {
	REG(GPIO3B6, 12, 12),
	GPIO3B6_GPIO		= 0,
	GPIO3B6_SDMMC0_DECTN,

	REG(GPIO3B5, 10, 10),
	GPIO3B5_GPIO		= 0,
	GPIO3B5_SDMMC0_DATA3,

	REG(GPIO3B4, 8, 8),
	GPIO3B4_GPIO		= 0,
	GPIO3B4_SDMMC0_DATA2,

	REG(GPIO3B3, 6, 6),
	GPIO3B3_GPIO		= 0,
	GPIO3B3_SDMMC0_DATA1,

	REG(GPIO3B2, 4, 4),
	GPIO3B2_GPIO		= 0,
	GPIO3B2_SDMMC0_DATA0,

	REG(GPIO3B1, 2, 2),
	GPIO3B1_GPIO		= 0,
	GPIO3B1_SDMMC0_CMD,

	REG(GPIO3B0, 0, 0),
	GPIO3B0_GPIO		= 0,
	GPIO3B0_SDMMC0_CLKOUT,
};

/* GRF_SOC_CON0 */
enum {
	REG(SMC_MUX_CON, 13, 13),

	REG(NOC_REMAP, 12, 12),

	REG(EMMC_FLASH_SEL, 11, 11),

	REG(TZPC_REVISION, 10, 7),

	REG(L2CACHE_ACC, 6, 5),

	REG(L2RD_WAIT, 4, 3),

	REG(IMEMRD_WAIT, 2, 1),

	REG(SOC_REMAP, 0, 0),
};

/* GRF_SOC_CON1 */
enum {
	REG(RKI2C4_SEL, 15, 15),

	REG(RKI2C3_SEL, 14, 14),

	REG(RKI2C2_SEL, 13, 13),

	REG(RKI2C1_SEL, 12, 12),

	REG(RKI2C0_SEL, 11, 11),

	REG(VCODEC_SEL, 10, 10),

	REG(PERI_EMEM_PAUSE, 9, 9),

	REG(PERI_USB_PAUSE, 8, 8),

	REG(SMC_MUX_MODE_0, 6, 6),

	REG(SMC_SRAM_MW_0, 5, 4),

	REG(SMC_REMAP_0, 3, 3),

	REG(SMC_A_GT_M0_SYNC, 2, 2),

	REG(EMAC_SPEED, 1, 1),

	REG(EMAC_MODE, 0, 0),
};

/* GRF_SOC_CON2 */
enum {
	REG(MSCH4_MAINDDR3, 7, 7),
	MSCH4_MAINDDR3_DDR3	= 1,

	REG(EMAC_NEWRCV_EN, 6, 6),

	REG(SW_ADDR15_EN, 5, 5),

	REG(SW_ADDR16_EN, 4, 4),

	REG(SW_ADDR17_EN, 3, 3),

	REG(BANK2_TO_RANK_EN, 2, 2),

	REG(RANK_TO_ROW15_EN, 1, 1),

	REG(UPCTL_C_ACTIVE_IN, 0, 0),
	UPCTL_C_ACTIVE_IN_MAY	= 0,
	UPCTL_C_ACTIVE_IN_WILL,
};

/* GRF_DDRC_CON0 */
enum {
	REG(DTO_LB, 12, 11),

	REG(DTO_TE, 10, 9),

	REG(DTO_PDR, 8, 7),

	REG(DTO_PDD, 6, 5),

	REG(DTO_IOM, 4, 3),

	REG(DTO_OE, 2, 1),

	REG(ATO_AE, 0, 0),
};
#endif
