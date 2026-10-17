/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Paweł Jarosz <paweljarosz3691@gmail.com>
 */

#ifndef _ASM_ARCH_CRU_RK3066_H
#define _ASM_ARCH_CRU_RK3066_H

#include <linux/bitops.h>
#include <linux/bitfield.h>

#define REG(name, h, l) \
	name##_MASK = GENMASK(h, l), \
	name##_SHIFT = __bf_shf(name##_MASK)

#define OSC_HZ		(24 * 1000 * 1000)

#define APLL_HZ		(1416 * 1000000)
#define APLL_SAFE_HZ	(600 * 1000000)
#define GPLL_HZ		(594 * 1000000)
#define CPLL_HZ		(384 * 1000000)

/* The SRAM is clocked off aclk_cpu, so we want to max it out for boot speed */
#define CPU_ACLK_HZ	297000000
#define CPU_HCLK_HZ	148500000
#define CPU_PCLK_HZ	74250000
#define CPU_H2P_HZ	74250000

#define PERI_ACLK_HZ	148500000
#define PERI_HCLK_HZ	148500000
#define PERI_PCLK_HZ	74250000

/* Private data for the clock driver - used by rockchip_get_cru() */
struct rk3066_clk_priv {
	struct rk3066_grf *grf;
	struct rk3066_cru *cru;
	ulong rate;
	bool has_bwadj;
};

struct rk3066_cru {
	struct rk3066_pll {
		u32 con0;
		u32 con1;
		u32 con2;
		u32 con3;
	} pll[4];
	u32 cru_mode_con;
	u32 cru_clksel_con[35];
	u32 cru_clkgate_con[10];
	u32 reserved1[2];
	u32 cru_glb_srst_fst_value;
	u32 cru_glb_srst_snd_value;
	u32 reserved2[2];
	u32 cru_softrst_con[9];
	u32 cru_misc_con;
	u32 reserved3[2];
	u32 cru_glb_cnt_th;
};

check_member(rk3066_cru, cru_glb_cnt_th, 0x0140);

/* CRU_CLKSEL0_CON */
enum {
	REG(CPU_ACLK_PLL, 8, 8),
	CPU_ACLK_PLL_SELECT_APLL	= 0,
	CPU_ACLK_PLL_SELECT_GPLL,

	REG(CORE_PERI_DIV, 7, 6),

	REG(A9_CORE_DIV, 4, 0),
};

/* CRU_CLKSEL1_CON */
enum {
	REG(AHB2APB_DIV, 15, 14),

	REG(CPU_PCLK_DIV, 13, 12),

	REG(CPU_HCLK_DIV, 9, 8),

	REG(CPU_ACLK_DIV, 2, 0),
};

/* CRU_CLKSEL10_CON */
enum {
	REG(PERI_SEL_PLL, 15, 15),
	PERI_SEL_CPLL		= 0,
	PERI_SEL_GPLL,

	REG(PERI_PCLK_DIV, 13, 12),

	REG(PERI_HCLK_DIV, 9, 8),

	REG(PERI_ACLK_DIV, 4, 0),
};

/* CRU_CLKSEL11_CON */
enum {
	REG(MMC0_DIV, 5, 0),
};

/* CRU_CLKSEL12_CON */
enum {
	REG(UART_PLL, 15, 15),
	UART_PLL_SELECT_GENERAL	= 0,
	UART_PLL_SELECT_CODEC,

	REG(EMMC_DIV, 13, 8),

	REG(SDIO_DIV, 5, 0),
};

/* CRU_CLKSEL24_CON */
enum {
	REG(SARADC_DIV, 15, 8),
};

/* CRU_CLKSEL25_CON */
enum {
	REG(SPI1_DIV, 14, 8),

	REG(SPI0_DIV, 6, 0),
};

/* CRU_CLKSEL34_CON */
enum {
	REG(TSADC_DIV, 15, 0),
};

/* CRU_MODE_CON */
enum {
	REG(GPLL_MODE, 13, 12),

	REG(CPLL_MODE, 9, 8),

	REG(DPLL_MODE, 5, 4),

	REG(APLL_MODE, 1, 0),
	PLL_MODE_SLOW		= 0,
	PLL_MODE_NORMAL,
	PLL_MODE_DEEP,
};

/* CRU_APLL_CON0 */
enum {
	REG(CLKR, 13, 8),

	REG(CLKOD, 3, 0),
};

/* CRU_APLL_CON1 */
enum {
	REG(CLKF, 12, 0),
};

#endif
