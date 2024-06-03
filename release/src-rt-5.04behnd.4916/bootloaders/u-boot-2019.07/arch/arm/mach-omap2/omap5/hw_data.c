// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * HW data initialization for OMAP5
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Sricharan R <r.sricharan@ti.com>
 */
#include <common.h>
#include <palmas.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>
#include <asm/arch/clock.h>
#include <asm/omap_gpio.h>
#include <asm/io.h>
#include <asm/emif.h>

struct prcm_regs const **prcm =
			(struct prcm_regs const **) OMAP_SRAM_SCRATCH_PRCM_PTR;
struct dplls const **dplls_data =
			(struct dplls const **) OMAP_SRAM_SCRATCH_DPLLS_PTR;
struct vcores_data const **omap_vcores =
		(struct vcores_data const **) OMAP_SRAM_SCRATCH_VCORES_PTR;
struct omap_sys_ctrl_regs const **ctrl =
	(struct omap_sys_ctrl_regs const **)OMAP_SRAM_SCRATCH_SYS_CTRL;

/* OPP NOM FREQUENCY for ES1.0 */
static const struct dpll_params mpu_dpll_params_800mhz[NUM_SYS_CLKS] = {
	{200, 2, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1000, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{375, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{400, 12, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{375, 17, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

/* OPP NOM FREQUENCY for OMAP5 ES2.0, and DRA7 ES1.0 */
static const struct dpll_params mpu_dpll_params_1ghz[NUM_SYS_CLKS] = {
	{250, 2, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{500, 9, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 20 MHz   */
	{119, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{625, 11, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{500, 12, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{625, 23, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 38.4 MHz */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr532[NUM_SYS_CLKS] = {
	{266, 2, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{443, 6, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 16.8 MHz */
	{277, 4, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 19.2 MHz */
	{368, 8, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1}		/* 38.4 MHz */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr532_es2[NUM_SYS_CLKS] = {
	{266, 2, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{443, 6, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 16.8 MHz */
	{277, 4, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 19.2 MHz */
	{368, 8, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6}		/* 38.4 MHz */
};

static const struct dpll_params
		core_dpll_params_2128mhz_dra7xx[NUM_SYS_CLKS] = {
	{266, 2, 2, 1, -1, 4, 62, 5, -1, 5, 4, 6},		/* 12 MHz   */
	{266, 4, 2, 1, -1, 4, 62, 5, -1, 5, 4, 6},		/* 20 MHz   */
	{443, 6, 2, 1, -1, 4, 62, 5, -1, 5, 4, 6},		/* 16.8 MHz */
	{277, 4, 2, 1, -1, 4, 62, 5, -1, 5, 4, 6},		/* 19.2 MHz */
	{368, 8, 2, 1, -1, 4, 62, 5, -1, 5, 4, 6},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 2, 1, -1, 4, 62, 5, -1, 5, 4, 6},		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz[NUM_SYS_CLKS] = {
	{32, 0, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{160, 6, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz_es2[NUM_SYS_CLKS] = {
	{32, 0, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{160, 6, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz_dra7xx[NUM_SYS_CLKS] = {
	{32, 0, 4, 1, 3, 4, 4, 2, -1, -1, -1, -1},		/* 12 MHz   */
	{96, 4, 4, 1, 3, 4, 10, 2, -1, -1, -1, -1},		/* 20 MHz   */
	{160, 6, 4, 1, 3, 4, 4, 2, -1, -1, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, 1, 3, 4, 4, 2, -1, -1, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, 1, 3, 4, 4, 2, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, 1, 3, 4, 4, 2, -1, -1, -1, -1},		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz_dra76x[NUM_SYS_CLKS] = {
	{32, 0, 4, 1, 3, 4, 8, 2, -1, -1, -1, -1},		/* 12 MHz   */
	{96, 4, 4, 1, 3, 4, 8, 2, -1, -1, -1, -1},		/* 20 MHz   */
	{160, 6, 4, 1, 3, 4, 8, 2, -1, -1, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, 1, 3, 4, 8, 2, -1, -1, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, 1, 3, 4, 8, 2, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, 1, 3, 4, 8, 2, -1, -1, -1, -1},		/* 38.4 MHz */
};

static const struct dpll_params iva_dpll_params_2330mhz[NUM_SYS_CLKS] = {
	{1165, 11, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{208, 2, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{182, 2, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{224, 4, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{91, 2, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params iva_dpll_params_2330mhz_dra7xx[NUM_SYS_CLKS] = {
	{1165, 11, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{233, 3, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 20 MHz */
	{208, 2, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{182, 2, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{224, 4, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{91, 2, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 38.4 MHz */
};

/* ABE M & N values with sys_clk as source */
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
static const struct dpll_params
		abe_dpll_params_sysclk_196608khz[NUM_SYS_CLKS] = {
	{49, 5, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{35, 5, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{46, 8, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{34, 8, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{64, 24, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};
#endif

/* ABE M & N values with 32K clock as source */
#ifndef CONFIG_SYS_OMAP_ABE_SYSCK
static const struct dpll_params abe_dpll_params_32k_196608khz = {
	750, 0, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1
};
#endif

/* ABE M & N values with sysclk2(22.5792 MHz) as input */
static const struct dpll_params
		abe_dpll_params_sysclk2_361267khz[NUM_SYS_CLKS] = {
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{16, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 20 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 38.4 MHz */
};

static const struct dpll_params usb_dpll_params_1920mhz[NUM_SYS_CLKS] = {
	{400, 4, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{480, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 20 MHz   */
	{400, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{400, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{480, 12, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{400, 15, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 38.4 MHz */
};

static const struct dpll_params ddr_dpll_params_2664mhz[NUM_SYS_CLKS] = {
	{111, 0, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{333, 4, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 20 MHz   */
	{555, 6, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{555, 7, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{666, 12, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{555, 15, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 38.4 MHz */
};

static const struct dpll_params ddr_dpll_params_2128mhz[NUM_SYS_CLKS] = {
	{266, 2, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{266, 4, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 20 MHz   */
	{190, 2, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{665, 11, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{532, 12, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{665, 23, 2, 1, 8, -1, -1, -1, -1, -1, -1, -1},		/* 38.4 MHz */
};

static const struct dpll_params gmac_dpll_params_2000mhz[NUM_SYS_CLKS] = {
	{250, 2, 4, 10, 40, 8, 10, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{250, 4, 4, 10, 40, 8, 10, -1, -1, -1, -1, -1},		/* 20 MHz   */
	{119, 1, 4, 10, 40, 8, 10, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{625, 11, 4, 10, 40, 8, 10, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{500, 12, 4, 10, 40, 8, 10, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{625, 23, 4, 10, 40, 8, 10, -1, -1, -1, -1, -1},	/* 38.4 MHz */
};

struct dplls omap5_dplls_es1 = {
	.mpu = mpu_dpll_params_800mhz,
	.core = core_dpll_params_2128mhz_ddr532,
	.per = per_dpll_params_768mhz,
	.iva = iva_dpll_params_2330mhz,
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
	.abe = abe_dpll_params_sysclk_196608khz,
#else
	.abe = &abe_dpll_params_32k_196608khz,
#endif
	.usb = usb_dpll_params_1920mhz,
	.ddr = NULL
};

struct dplls omap5_dplls_es2 = {
	.mpu = mpu_dpll_params_1ghz,
	.core = core_dpll_params_2128mhz_ddr532_es2,
	.per = per_dpll_params_768mhz_es2,
	.iva = iva_dpll_params_2330mhz,
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
	.abe = abe_dpll_params_sysclk_196608khz,
#else
	.abe = &abe_dpll_params_32k_196608khz,
#endif
	.usb = usb_dpll_params_1920mhz,
	.ddr = NULL
};

struct dplls dra76x_dplls = {
	.mpu = mpu_dpll_params_1ghz,
	.core = core_dpll_params_2128mhz_dra7xx,
	.per = per_dpll_params_768mhz_dra76x,
	.abe = abe_dpll_params_sysclk2_361267khz,
	.iva = iva_dpll_params_2330mhz_dra7xx,
	.usb = usb_dpll_params_1920mhz,
	.ddr =	ddr_dpll_params_2664mhz,
	.gmac = gmac_dpll_params_2000mhz,
};

struct dplls dra7xx_dplls = {
	.mpu = mpu_dpll_params_1ghz,
	.core = core_dpll_params_2128mhz_dra7xx,
	.per = per_dpll_params_768mhz_dra7xx,
	.abe = abe_dpll_params_sysclk2_361267khz,
	.iva = iva_dpll_params_2330mhz_dra7xx,
	.usb = usb_dpll_params_1920mhz,
	.ddr = ddr_dpll_params_2128mhz,
	.gmac = gmac_dpll_params_2000mhz,
};

struct dplls dra72x_dplls = {
	.mpu = mpu_dpll_params_1ghz,
	.core = core_dpll_params_2128mhz_dra7xx,
	.per = per_dpll_params_768mhz_dra7xx,
	.abe = abe_dpll_params_sysclk2_361267khz,
	.iva = iva_dpll_params_2330mhz_dra7xx,
	.usb = usb_dpll_params_1920mhz,
	.ddr =	ddr_dpll_params_2664mhz,
	.gmac = gmac_dpll_params_2000mhz,
};

struct pmic_data palmas = {
	.base_offset = PALMAS_SMPS_BASE_VOLT_UV,
	.step = 10000, /* 10 mV represented in uV */
	/*
	 * Offset codes 1-6 all give the base voltage in Palmas
	 * Offset code 0 switches OFF the SMPS
	 */
	.start_code = 6,
	.i2c_slave_addr	= SMPS_I2C_SLAVE_ADDR,
	.pmic_bus_init	= sri2c_init,
	.pmic_write	= omap_vc_bypass_send_value,
	.gpio_en = 0,
};

/* The TPS659038 and TPS65917 are software-compatible, use common struct */
struct pmic_data tps659038 = {
	.base_offset = PALMAS_SMPS_BASE_VOLT_UV,
	.step = 10000, /* 10 mV represented in uV */
	/*
	 * Offset codes 1-6 all give the base voltage in Palmas
	 * Offset code 0 switches OFF the SMPS
	 */
	.start_code = 6,
	.i2c_slave_addr	= TPS659038_I2C_SLAVE_ADDR,
	.pmic_bus_init	= gpi2c_init,
	.pmic_write	= palmas_i2c_write_u8,
	.gpio_en = 0,
};

/* The LP87565*/
struct pmic_data lp87565 = {
	.base_offset = LP873X_BUCK_BASE_VOLT_UV,
	.step = 5000, /* 5 mV represented in uV */
	/*
	 * Offset codes 0 - 0x13 Invalid.
	 * Offset codes 0x14 0x17 give 10mV steps
	 * Offset codes 0x17 through 0x9D give 5mV steps
	 * So let us start with our operating range from .73V
	 */
	.start_code = 0x17,
	.i2c_slave_addr = 0x60,
	.pmic_bus_init  = gpi2c_init,
	.pmic_write     = palmas_i2c_write_u8,
};

/* The LP8732 and LP8733 are software-compatible, use common struct */
struct pmic_data lp8733 = {
	.base_offset = LP873X_BUCK_BASE_VOLT_UV,
	.step = 5000, /* 5 mV represented in uV */
	/*
	 * Offset codes 0 - 0x13 Invalid.
	 * Offset codes 0x14 0x17 give 10mV steps
	 * Offset codes 0x17 through 0x9D give 5mV steps
	 * So let us start with our operating range from .73V
	 */
	.start_code = 0x17,
	.i2c_slave_addr = 0x60,
	.pmic_bus_init  = gpi2c_init,
	.pmic_write     = palmas_i2c_write_u8,
};

struct vcores_data omap5430_volts = {
	.mpu.value[OPP_NOM] = VDD_MPU,
	.mpu.addr = SMPS_REG_ADDR_12_MPU,
	.mpu.pmic = &palmas,

	.core.value[OPP_NOM] = VDD_CORE,
	.core.addr = SMPS_REG_ADDR_8_CORE,
	.core.pmic = &palmas,

	.mm.value[OPP_NOM] = VDD_MM,
	.mm.addr = SMPS_REG_ADDR_45_IVA,
	.mm.pmic = &palmas,
};

struct vcores_data omap5430_volts_es2 = {
	.mpu.value[OPP_NOM] = VDD_MPU_ES2,
	.mpu.addr = SMPS_REG_ADDR_12_MPU,
	.mpu.pmic = &palmas,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.core.value[OPP_NOM] = VDD_CORE_ES2,
	.core.addr = SMPS_REG_ADDR_8_CORE,
	.core.pmic = &palmas,

	.mm.value[OPP_NOM] = VDD_MM_ES2,
	.mm.addr = SMPS_REG_ADDR_45_IVA,
	.mm.pmic = &palmas,
	.mm.abb_tx_done_mask = OMAP_ABB_MM_TXDONE_MASK,

	.mpu.efuse.reg[OPP_NOM]	= OMAP5_ES2_PROD_MPU_OPNO_VMIN,
	.mpu.efuse.reg_bits	= OMAP5_ES2_PROD_REGBITS,

	.core.efuse.reg[OPP_NOM] = OMAP5_ES2_PROD_CORE_OPNO_VMIN,
	.core.efuse.reg_bits	= OMAP5_ES2_PROD_REGBITS,

	.mm.efuse.reg[OPP_NOM]	= OMAP5_ES2_PROD_MM_OPNO_VMIN,
	.mm.efuse.reg_bits	= OMAP5_ES2_PROD_REGBITS,
};

/*
 * Enable essential clock domains, modules and
 * do some additional special settings needed
 */
void enable_basic_clocks(void)
{
	u32 const clk_domains_essential[] = {
		(*prcm)->cm_l4per_clkstctrl,
		(*prcm)->cm_l3init_clkstctrl,
		(*prcm)->cm_memif_clkstctrl,
		(*prcm)->cm_l4cfg_clkstctrl,
#ifdef CONFIG_DRIVER_TI_CPSW
		(*prcm)->cm_gmac_clkstctrl,
#endif
		0
	};

	u32 const clk_modules_hw_auto_essential[] = {
		(*prcm)->cm_l3_gpmc_clkctrl,
		(*prcm)->cm_memif_emif_1_clkctrl,
		(*prcm)->cm_memif_emif_2_clkctrl,
		(*prcm)->cm_l4cfg_l4_cfg_clkctrl,
		(*prcm)->cm_wkup_gpio1_clkctrl,
		(*prcm)->cm_l4per_gpio2_clkctrl,
		(*prcm)->cm_l4per_gpio3_clkctrl,
		(*prcm)->cm_l4per_gpio4_clkctrl,
		(*prcm)->cm_l4per_gpio5_clkctrl,
		(*prcm)->cm_l4per_gpio6_clkctrl,
		(*prcm)->cm_l4per_gpio7_clkctrl,
		(*prcm)->cm_l4per_gpio8_clkctrl,
#ifdef CONFIG_SCSI_AHCI_PLAT
		(*prcm)->cm_l3init_ocp2scp3_clkctrl,
#endif
		0
	};

	u32 const clk_modules_explicit_en_essential[] = {
		(*prcm)->cm_wkup_gptimer1_clkctrl,
		(*prcm)->cm_l3init_hsmmc1_clkctrl,
		(*prcm)->cm_l3init_hsmmc2_clkctrl,
		(*prcm)->cm_l4per_gptimer2_clkctrl,
		(*prcm)->cm_wkup_wdtimer2_clkctrl,
		(*prcm)->cm_l4per_uart3_clkctrl,
		(*prcm)->cm_l4per_i2c1_clkctrl,
#ifdef CONFIG_DRIVER_TI_CPSW
		(*prcm)->cm_gmac_gmac_clkctrl,
#endif

#ifdef CONFIG_TI_QSPI
		(*prcm)->cm_l4per_qspi_clkctrl,
#endif
#ifdef CONFIG_SCSI_AHCI_PLAT
		(*prcm)->cm_l3init_sata_clkctrl,
#endif
		0
	};

	/* Enable optional additional functional clock for GPIO4 */
	setbits_le32((*prcm)->cm_l4per_gpio4_clkctrl,
			GPIO4_CLKCTRL_OPTFCLKEN_MASK);

	/* Enable 192 MHz clock for MMC1 & MMC2 */
	setbits_le32((*prcm)->cm_l3init_hsmmc1_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_MASK);
	setbits_le32((*prcm)->cm_l3init_hsmmc2_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_MASK);

	/* Set the correct clock dividers for mmc */
	clrbits_le32((*prcm)->cm_l3init_hsmmc1_clkctrl,
		     HSMMC_CLKCTRL_CLKSEL_DIV_MASK);
	clrbits_le32((*prcm)->cm_l3init_hsmmc2_clkctrl,
		     HSMMC_CLKCTRL_CLKSEL_DIV_MASK);

	/* Select 32KHz clock as the source of GPTIMER1 */
	setbits_le32((*prcm)->cm_wkup_gptimer1_clkctrl,
			GPTIMER1_CLKCTRL_CLKSEL_MASK);

	do_enable_clocks(clk_domains_essential,
			 clk_modules_hw_auto_essential,
			 clk_modules_explicit_en_essential,
			 1);

#ifdef CONFIG_TI_QSPI
	setbits_le32((*prcm)->cm_l4per_qspi_clkctrl, (1<<24));
#endif

#ifdef CONFIG_SCSI_AHCI_PLAT
	/* Enable optional functional clock for SATA */
	setbits_le32((*prcm)->cm_l3init_sata_clkctrl,
		     SATA_CLKCTRL_OPTFCLKEN_MASK);
#endif

	/* Enable SCRM OPT clocks for PER and CORE dpll */
	setbits_le32((*prcm)->cm_wkupaon_scrm_clkctrl,
			OPTFCLKEN_SCRM_PER_MASK);
	setbits_le32((*prcm)->cm_wkupaon_scrm_clkctrl,
			OPTFCLKEN_SCRM_CORE_MASK);
}

void enable_basic_uboot_clocks(void)
{
	u32 const clk_domains_essential[] = {
#if defined(CONFIG_DRA7XX)
		(*prcm)->cm_ipu_clkstctrl,
#endif
		0
	};

	u32 const clk_modules_hw_auto_essential[] = {
		(*prcm)->cm_l3init_hsusbtll_clkctrl,
		0
	};

	u32 const clk_modules_explicit_en_essential[] = {
		(*prcm)->cm_l4per_mcspi1_clkctrl,
		(*prcm)->cm_l4per_i2c2_clkctrl,
		(*prcm)->cm_l4per_i2c3_clkctrl,
		(*prcm)->cm_l4per_i2c4_clkctrl,
#if defined(CONFIG_DRA7XX)
		(*prcm)->cm_ipu_i2c5_clkctrl,
#else
		(*prcm)->cm_l4per_i2c5_clkctrl,
#endif
		(*prcm)->cm_l3init_hsusbhost_clkctrl,
		(*prcm)->cm_l3init_fsusb_clkctrl,
		0
	};
	do_enable_clocks(clk_domains_essential,
			 clk_modules_hw_auto_essential,
			 clk_modules_explicit_en_essential,
			 1);
}

#ifdef CONFIG_TI_EDMA3
void enable_edma3_clocks(void)
{
	u32 const clk_domains_edma3[] = {
		0
	};

	u32 const clk_modules_hw_auto_edma3[] = {
		(*prcm)->cm_l3main1_tptc1_clkctrl,
		(*prcm)->cm_l3main1_tptc2_clkctrl,
		0
	};

	u32 const clk_modules_explicit_en_edma3[] = {
		0
	};

	do_enable_clocks(clk_domains_edma3,
			 clk_modules_hw_auto_edma3,
			 clk_modules_explicit_en_edma3,
			 1);
}

void disable_edma3_clocks(void)
{
	u32 const clk_domains_edma3[] = {
		0
	};

	u32 const clk_modules_disable_edma3[] = {
		(*prcm)->cm_l3main1_tptc1_clkctrl,
		(*prcm)->cm_l3main1_tptc2_clkctrl,
		0
	};

	do_disable_clocks(clk_domains_edma3,
			  clk_modules_disable_edma3,
			  1);
}
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_OMAP)
void enable_usb_clocks(int index)
{
	u32 cm_l3init_usb_otg_ss_clkctrl = 0;

	if (index == 0) {
		cm_l3init_usb_otg_ss_clkctrl =
			(*prcm)->cm_l3init_usb_otg_ss1_clkctrl;
		/* Enable 960 MHz clock for dwc3 */
		setbits_le32((*prcm)->cm_l3init_usb_otg_ss1_clkctrl,
			     OPTFCLKEN_REFCLK960M);

		/* Enable 32 KHz clock for USB_PHY1 */
		setbits_le32((*prcm)->cm_coreaon_usb_phy1_core_clkctrl,
			     USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);

		/* Enable 32 KHz clock for USB_PHY3 */
		if (is_dra7xx())
			setbits_le32((*prcm)->cm_coreaon_usb_phy3_core_clkctrl,
				     USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);
	} else if (index == 1) {
		cm_l3init_usb_otg_ss_clkctrl =
			(*prcm)->cm_l3init_usb_otg_ss2_clkctrl;
		/* Enable 960 MHz clock for dwc3 */
		setbits_le32((*prcm)->cm_l3init_usb_otg_ss2_clkctrl,
			     OPTFCLKEN_REFCLK960M);

		/* Enable 32 KHz clock for dwc3 */
		setbits_le32((*prcm)->cm_coreaon_usb_phy2_core_clkctrl,
			     USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);

		/* Enable 60 MHz clock for USB2PHY2 */
		setbits_le32((*prcm)->cm_coreaon_l3init_60m_gfclk_clkctrl,
			     L3INIT_CLKCTRL_OPTFCLKEN_60M_GFCLK);
	}

	u32 const clk_domains_usb[] = {
		0
	};

	u32 const clk_modules_hw_auto_usb[] = {
		(*prcm)->cm_l3init_ocp2scp1_clkctrl,
		cm_l3init_usb_otg_ss_clkctrl,
		0
	};

	u32 const clk_modules_explicit_en_usb[] = {
		0
	};

	do_enable_clocks(clk_domains_usb,
			 clk_modules_hw_auto_usb,
			 clk_modules_explicit_en_usb,
			 1);
}

void disable_usb_clocks(int index)
{
	u32 cm_l3init_usb_otg_ss_clkctrl = 0;

	if (index == 0) {
		cm_l3init_usb_otg_ss_clkctrl =
			(*prcm)->cm_l3init_usb_otg_ss1_clkctrl;
		/* Disable 960 MHz clock for dwc3 */
		clrbits_le32((*prcm)->cm_l3init_usb_otg_ss1_clkctrl,
			     OPTFCLKEN_REFCLK960M);

		/* Disable 32 KHz clock for USB_PHY1 */
		clrbits_le32((*prcm)->cm_coreaon_usb_phy1_core_clkctrl,
			     USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);

		/* Disable 32 KHz clock for USB_PHY3 */
		if (is_dra7xx())
			clrbits_le32((*prcm)->cm_coreaon_usb_phy3_core_clkctrl,
				     USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);
	} else if (index == 1) {
		cm_l3init_usb_otg_ss_clkctrl =
			(*prcm)->cm_l3init_usb_otg_ss2_clkctrl;
		/* Disable 960 MHz clock for dwc3 */
		clrbits_le32((*prcm)->cm_l3init_usb_otg_ss2_clkctrl,
			     OPTFCLKEN_REFCLK960M);

		/* Disable 32 KHz clock for dwc3 */
		clrbits_le32((*prcm)->cm_coreaon_usb_phy2_core_clkctrl,
			     USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);

		/* Disable 60 MHz clock for USB2PHY2 */
		clrbits_le32((*prcm)->cm_coreaon_l3init_60m_gfclk_clkctrl,
			     L3INIT_CLKCTRL_OPTFCLKEN_60M_GFCLK);
	}

	u32 const clk_domains_usb[] = {
		0
	};

	u32 const clk_modules_disable[] = {
		(*prcm)->cm_l3init_ocp2scp1_clkctrl,
		cm_l3init_usb_otg_ss_clkctrl,
		0
	};

	do_disable_clocks(clk_domains_usb,
			  clk_modules_disable,
			  1);
}
#endif

const struct ctrl_ioregs ioregs_omap5430 = {
	.ctrl_ddrch = DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN,
	.ctrl_lpddr2ch = DDR_IO_I_34OHM_SR_FASTEST_WD_CK_CKE_NCS_CA_PULL_DOWN,
	.ctrl_ddrio_0 = DDR_IO_0_DDR2_DQ_INT_EN_ALL_DDR3_CA_DIS_ALL,
	.ctrl_ddrio_1 = DDR_IO_1_DQ_OUT_EN_ALL_DQ_INT_EN_ALL,
	.ctrl_ddrio_2 = DDR_IO_2_CA_OUT_EN_ALL_CA_INT_EN_ALL,
};

const struct ctrl_ioregs ioregs_omap5432_es1 = {
	.ctrl_ddrch = DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL,
	.ctrl_lpddr2ch = 0x0,
	.ctrl_ddr3ch = DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL,
	.ctrl_ddrio_0 = DDR_IO_0_VREF_CELLS_DDR3_VALUE,
	.ctrl_ddrio_1 = DDR_IO_1_VREF_CELLS_DDR3_VALUE,
	.ctrl_ddrio_2 = DDR_IO_2_VREF_CELLS_DDR3_VALUE,
	.ctrl_emif_sdram_config_ext = SDRAM_CONFIG_EXT_RD_LVL_11_SAMPLES,
	.ctrl_emif_sdram_config_ext_final = SDRAM_CONFIG_EXT_RD_LVL_4_SAMPLES,
};

const struct ctrl_ioregs ioregs_omap5432_es2 = {
	.ctrl_ddrch = DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL_ES2,
	.ctrl_lpddr2ch = 0x0,
	.ctrl_ddr3ch = DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL_ES2,
	.ctrl_ddrio_0 = DDR_IO_0_VREF_CELLS_DDR3_VALUE_ES2,
	.ctrl_ddrio_1 = DDR_IO_1_VREF_CELLS_DDR3_VALUE_ES2,
	.ctrl_ddrio_2 = DDR_IO_2_VREF_CELLS_DDR3_VALUE_ES2,
	.ctrl_emif_sdram_config_ext = SDRAM_CONFIG_EXT_RD_LVL_11_SAMPLES,
	.ctrl_emif_sdram_config_ext_final = SDRAM_CONFIG_EXT_RD_LVL_4_SAMPLES,
};

const struct ctrl_ioregs ioregs_dra7xx_es1 = {
	.ctrl_ddrch = 0x40404040,
	.ctrl_lpddr2ch = 0x40404040,
	.ctrl_ddr3ch = 0x80808080,
	.ctrl_ddrio_0 = 0x00094A40,
	.ctrl_ddrio_1 = 0x04A52000,
	.ctrl_ddrio_2 = 0x84210000,
	.ctrl_emif_sdram_config_ext = 0x0001C1A7,
	.ctrl_emif_sdram_config_ext_final = 0x0001C1A7,
	.ctrl_ddr_ctrl_ext_0 = 0xA2000000,
};

const struct ctrl_ioregs ioregs_dra72x_es1 = {
	.ctrl_ddrch = 0x40404040,
	.ctrl_lpddr2ch = 0x40404040,
	.ctrl_ddr3ch = 0x60606080,
	.ctrl_ddrio_0 = 0x00094A40,
	.ctrl_ddrio_1 = 0x04A52000,
	.ctrl_ddrio_2 = 0x84210000,
	.ctrl_emif_sdram_config_ext = 0x0001C1A7,
	.ctrl_emif_sdram_config_ext_final = 0x0001C1A7,
	.ctrl_ddr_ctrl_ext_0 = 0xA2000000,
};

const struct ctrl_ioregs ioregs_dra72x_es2 = {
	.ctrl_ddrch = 0x40404040,
	.ctrl_lpddr2ch = 0x40404040,
	.ctrl_ddr3ch = 0x60606060,
	.ctrl_ddrio_0 = 0x00094A40,
	.ctrl_ddrio_1 = 0x00000000,
	.ctrl_ddrio_2 = 0x00000000,
	.ctrl_emif_sdram_config_ext = 0x0001C1A7,
	.ctrl_emif_sdram_config_ext_final = 0x0001C1A7,
	.ctrl_ddr_ctrl_ext_0 = 0xA2000000,
};

void __weak hw_data_init(void)
{
	u32 omap_rev = omap_revision();

	switch (omap_rev) {

	case OMAP5430_ES1_0:
	case OMAP5432_ES1_0:
	*prcm = &omap5_es1_prcm;
	*dplls_data = &omap5_dplls_es1;
	*omap_vcores = &omap5430_volts;
	*ctrl = &omap5_ctrl;
	break;

	case OMAP5430_ES2_0:
	case OMAP5432_ES2_0:
	*prcm = &omap5_es2_prcm;
	*dplls_data = &omap5_dplls_es2;
	*omap_vcores = &omap5430_volts_es2;
	*ctrl = &omap5_ctrl;
	break;

	case DRA762_ABZ_ES1_0:
	case DRA762_ACD_ES1_0:
	case DRA762_ES1_0:
	*prcm = &dra7xx_prcm;
	*dplls_data = &dra76x_dplls;
	*ctrl = &dra7xx_ctrl;
	break;

	case DRA752_ES1_0:
	case DRA752_ES1_1:
	case DRA752_ES2_0:
	*prcm = &dra7xx_prcm;
	*dplls_data = &dra7xx_dplls;
	*ctrl = &dra7xx_ctrl;
	break;

	case DRA722_ES1_0:
	case DRA722_ES2_0:
	case DRA722_ES2_1:
	*prcm = &dra7xx_prcm;
	*dplls_data = &dra72x_dplls;
	*ctrl = &dra7xx_ctrl;
	break;

	default:
		printf("\n INVALID OMAP REVISION ");
	}
}

void get_ioregs(const struct ctrl_ioregs **regs)
{
	u32 omap_rev = omap_revision();

	switch (omap_rev) {
	case OMAP5430_ES1_0:
	case OMAP5430_ES2_0:
		*regs = &ioregs_omap5430;
		break;
	case OMAP5432_ES1_0:
		*regs = &ioregs_omap5432_es1;
		break;
	case OMAP5432_ES2_0:
		*regs = &ioregs_omap5432_es2;
		break;
	case DRA752_ES1_0:
	case DRA752_ES1_1:
	case DRA752_ES2_0:
	case DRA762_ES1_0:
	case DRA762_ACD_ES1_0:
	case DRA762_ABZ_ES1_0:
		*regs = &ioregs_dra7xx_es1;
		break;
	case DRA722_ES1_0:
		*regs = &ioregs_dra72x_es1;
		break;
	case DRA722_ES2_0:
	case DRA722_ES2_1:
		*regs = &ioregs_dra72x_es2;
		break;

	default:
		printf("\n INVALID OMAP REVISION ");
	}
}
