// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015 Google, Inc
 * (C) Copyright 2016 Heiko Stuebner <heiko@sntech.de>
 */

#include <bitfield.h>
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3066.h>
#include <asm/arch-rockchip/grf_rk3066.h>
#include <asm/arch-rockchip/hardware.h>
#include <dt-bindings/clock/rk3066a-cru.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/log2.h>
#include <linux/stringify.h>

struct rk3066_clk_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3066a_cru dtd;
#endif
};

struct pll_div {
	u32 nr;
	u32 nf;
	u32 no;
};

enum {
	VCO_MAX_HZ	= 1416U * 1000000,
	VCO_MIN_HZ	= 300 * 1000000,
	OUTPUT_MAX_HZ	= 1416U * 1000000,
	OUTPUT_MIN_HZ	= 30 * 1000000,
	FREF_MAX_HZ	= 1416U * 1000000,
	FREF_MIN_HZ	= 30 * 1000,
};

enum {
	/* PLL CON0 */
	PLL_OD_MASK		= GENMASK(3, 0),

	/* PLL CON1 */
	PLL_NF_MASK		= GENMASK(12, 0),

	/* PLL CON2 */
	PLL_BWADJ_MASK		= GENMASK(11, 0),

	/* PLL CON3 */
	PLL_RESET_SHIFT		= 5,

	/* GRF_SOC_STATUS0 */
	SOCSTS_DPLL_LOCK	= BIT(4),
	SOCSTS_APLL_LOCK	= BIT(5),
	SOCSTS_CPLL_LOCK	= BIT(6),
	SOCSTS_GPLL_LOCK	= BIT(7),
};

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _nr, _no) {\
	.nr = _nr, .nf = (u32)((u64)hz * _nr * _no / OSC_HZ), .no = _no};\
	_Static_assert(((u64)hz * _nr * _no / OSC_HZ) * OSC_HZ /\
		       (_nr * _no) == hz, #hz "Hz cannot be hit with PLL "\
		       "divisors on line " __stringify(__LINE__))

/* Keep divisors as low as possible to reduce jitter and power usage. */
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 2, 2);
static const struct pll_div cpll_init_cfg = PLL_DIVISORS(CPLL_HZ, 1, 2);

static int rk3066_clk_set_pll(struct rk3066_cru *cru, enum rk_clk_id clk_id,
			      const struct pll_div *div)
{
	int pll_id = rk_pll_id(clk_id);
	struct rk3066_pll *pll = &cru->pll[pll_id];
	/* All PLLs have the same VCO and output frequency range restrictions. */
	uint vco_hz = OSC_HZ / 1000 * div->nf / div->nr * 1000;
	uint output_hz = vco_hz / div->no;

	debug("%s: PLL at %x: nf=%d, nr=%d, no=%d, vco=%u Hz, output=%u Hz\n", __func__,
	      (uint)pll, div->nf, div->nr, div->no, vco_hz, output_hz);
	assert(vco_hz >= VCO_MIN_HZ && vco_hz <= VCO_MAX_HZ &&
	       output_hz >= OUTPUT_MIN_HZ && output_hz <= OUTPUT_MAX_HZ &&
	       (div->no == 1 || !(div->no % 2)));

	/* Enter reset. */
	rk_setreg(&pll->con3, BIT(PLL_RESET_SHIFT));

	rk_clrsetreg(&pll->con0,
		     CLKR_MASK | PLL_OD_MASK,
		     ((div->nr - 1) << CLKR_SHIFT) | (div->no - 1));
	rk_clrsetreg(&pll->con1, CLKF_MASK, div->nf - 1);

	rk_clrsetreg(&pll->con2, PLL_BWADJ_MASK, (div->nf >> 1) - 1);

	/* Exit reset. */
	rk_clrreg(&pll->con3, BIT(PLL_RESET_SHIFT));

	return 0;
}

static int rk3066_clk_configure_ddr(struct rk3066_cru *cru, struct rk3066_grf *grf,
				    unsigned int hz)
{
	static const struct pll_div dpll_cfg[] = {
		{.nf = 25, .nr = 2, .no = 1},
		{.nf = 400, .nr = 9, .no = 2},
		{.nf = 500, .nr = 9, .no = 2},
		{.nf = 100, .nr = 3, .no = 1},
	};
	int cfg;

	switch (hz) {
	case 300000000:
		cfg = 0;
		break;
	case 533000000:	/* actually 533.3P MHz */
		cfg = 1;
		break;
	case 666000000:	/* actually 666.6P MHz */
		cfg = 2;
		break;
	case 800000000:
		cfg = 3;
		break;
	default:
		debug("%s: unsupported SDRAM frequency", __func__);
		return -EINVAL;
	}

	/* Enter PLL slow mode. */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK,
		     PLL_MODE_SLOW << DPLL_MODE_SHIFT);

	rk3066_clk_set_pll(cru, CLK_DDR, &dpll_cfg[cfg]);

	/* Wait for PLL lock. */
	while (!(readl(&grf->soc_status0) & SOCSTS_DPLL_LOCK))
		udelay(1);

	/* Enter PLL normal mode. */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK,
		     PLL_MODE_NORMAL << DPLL_MODE_SHIFT);

	return 0;
}

static int rk3066_clk_configure_cpu(struct rk3066_cru *cru, struct rk3066_grf *grf,
				    unsigned int hz)
{
	static const struct pll_div apll_cfg[] = {
		{.nf = 50, .nr = 1, .no = 2},
		{.nf = 59, .nr = 1, .no = 1},
	};
	int div_core_peri, div_cpu_aclk, cfg;

	/*
	 * We support two possible frequencies, the safe 600MHz
	 * which will work with default pmic settings and will
	 * be set to get away from the 24MHz default and
	 * the maximum of 1.416Ghz, which boards can set if they
	 * were able to get pmic support for it.
	 */
	switch (hz) {
	case APLL_SAFE_HZ:
		cfg = 0;
		div_core_peri = 1;
		div_cpu_aclk = 3;
		break;
	case APLL_HZ:
		cfg = 1;
		div_core_peri = 2;
		div_cpu_aclk = 3;
		break;
	default:
		debug("unsupported ARMCLK frequency");
		return -EINVAL;
	}

	/* Enter PLL slow mode. */
	rk_clrsetreg(&cru->cru_mode_con, APLL_MODE_MASK,
		     PLL_MODE_SLOW << APLL_MODE_SHIFT);

	rk3066_clk_set_pll(cru, CLK_ARM, &apll_cfg[cfg]);

	/* Wait for PLL lock. */
	while (!(readl(&grf->soc_status0) & SOCSTS_APLL_LOCK))
		udelay(1);

	/* Set divider for peripherals attached to the CPU core. */
	rk_clrsetreg(&cru->cru_clksel_con[0],
		     CORE_PERI_DIV_MASK,
		     div_core_peri << CORE_PERI_DIV_SHIFT);

	/* Set up dependent divisor for cpu_aclk. */
	rk_clrsetreg(&cru->cru_clksel_con[1],
		     CPU_ACLK_DIV_MASK,
		     div_cpu_aclk << CPU_ACLK_DIV_SHIFT);

	/* Enter PLL normal mode. */
	rk_clrsetreg(&cru->cru_mode_con, APLL_MODE_MASK,
		     PLL_MODE_NORMAL << APLL_MODE_SHIFT);

	return hz;
}

static uint32_t rk3066_clk_pll_get_rate(struct rk3066_cru *cru,
					enum rk_clk_id clk_id)
{
	u32 nr, no, nf;
	u32 con;
	int pll_id = rk_pll_id(clk_id);
	struct rk3066_pll *pll = &cru->pll[pll_id];
	static u8 clk_shift[CLK_COUNT] = {
		0xff, APLL_MODE_SHIFT, DPLL_MODE_SHIFT, CPLL_MODE_SHIFT,
		GPLL_MODE_SHIFT
	};
	uint shift;

	con = readl(&cru->cru_mode_con);
	shift = clk_shift[clk_id];
	switch (FIELD_GET(APLL_MODE_MASK, con >> shift)) {
	case PLL_MODE_SLOW:
		return OSC_HZ;
	case PLL_MODE_NORMAL:
		/* normal mode */
		con = readl(&pll->con0);
		no = bitfield_extract_by_mask(con, CLKOD_MASK) + 1;
		nr = bitfield_extract_by_mask(con, CLKR_MASK) + 1;
		con = readl(&pll->con1);
		nf = bitfield_extract_by_mask(con, CLKF_MASK) + 1;

		return (OSC_HZ * nf) / (nr * no);
	case PLL_MODE_DEEP:
	default:
		return 32768;
	}
}

static ulong rk3066_clk_mmc_get_clk(struct rk3066_cru *cru, uint gclk_rate,
				    int periph)
{
	uint div;
	u32 con;

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
		con = readl(&cru->cru_clksel_con[12]);
		div = bitfield_extract_by_mask(con, EMMC_DIV_MASK);
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		con = readl(&cru->cru_clksel_con[11]);
		div = bitfield_extract_by_mask(con, MMC0_DIV_MASK);
		break;
	case HCLK_SDIO:
	case SCLK_SDIO:
		con = readl(&cru->cru_clksel_con[12]);
		div = bitfield_extract_by_mask(con, SDIO_DIV_MASK);
		break;
	default:
		return -EINVAL;
	}

	return DIV_TO_RATE(gclk_rate, div) / 2;
}

static ulong rk3066_clk_mmc_set_clk(struct rk3066_cru *cru, uint gclk_rate,
				    int  periph, uint freq)
{
	int src_clk_div;

	debug("%s: gclk_rate=%u\n", __func__, gclk_rate);
	/* MMC clock by default divides by 2 internally, so need to provide double in CRU. */
	src_clk_div = DIV_ROUND_UP(gclk_rate / 2, freq) - 1;
	assert(src_clk_div <= 0x3f);

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     EMMC_DIV_MASK,
			     src_clk_div << EMMC_DIV_SHIFT);
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		rk_clrsetreg(&cru->cru_clksel_con[11],
			     MMC0_DIV_MASK,
			     src_clk_div << MMC0_DIV_SHIFT);
		break;
	case HCLK_SDIO:
	case SCLK_SDIO:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     SDIO_DIV_MASK,
			     src_clk_div << SDIO_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rk3066_clk_mmc_get_clk(cru, gclk_rate, periph);
}

static ulong rk3066_clk_spi_get_clk(struct rk3066_cru *cru, uint gclk_rate,
				    int periph)
{
	uint div;
	u32 con;

	switch (periph) {
	case SCLK_SPI0:
		con = readl(&cru->cru_clksel_con[25]);
		div = bitfield_extract_by_mask(con, SPI0_DIV_MASK);
		break;
	case SCLK_SPI1:
		con = readl(&cru->cru_clksel_con[25]);
		div = bitfield_extract_by_mask(con, SPI1_DIV_MASK);
		break;
	default:
		return -EINVAL;
	}

	return DIV_TO_RATE(gclk_rate, div);
}

static ulong rk3066_clk_spi_set_clk(struct rk3066_cru *cru, uint gclk_rate,
				    int periph, uint freq)
{
	int src_clk_div = DIV_ROUND_UP(gclk_rate, freq) - 1;

	assert(src_clk_div < 128);
	switch (periph) {
	case SCLK_SPI0:
		assert(src_clk_div <= SPI0_DIV_MASK >> SPI0_DIV_SHIFT);
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI0_DIV_MASK,
			     src_clk_div << SPI0_DIV_SHIFT);
		break;
	case SCLK_SPI1:
		assert(src_clk_div <= SPI1_DIV_MASK >> SPI1_DIV_SHIFT);
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI1_DIV_MASK,
			     src_clk_div << SPI1_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rk3066_clk_spi_get_clk(cru, gclk_rate, periph);
}

static ulong rk3066_clk_saradc_get_clk(struct rk3066_cru *cru, int periph)
{
	u32 div, con;

	switch (periph) {
	case SCLK_SARADC:
		con = readl(&cru->cru_clksel_con[24]);
		div = bitfield_extract_by_mask(con, SARADC_DIV_MASK);
		break;
	case SCLK_TSADC:
		con = readl(&cru->cru_clksel_con[34]);
		div = bitfield_extract_by_mask(con, TSADC_DIV_MASK);
		break;
	default:
		return -EINVAL;
	}
	return DIV_TO_RATE(PERI_PCLK_HZ, div);
}

static ulong rk3066_clk_saradc_set_clk(struct rk3066_cru *cru, uint hz,
				       int periph)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(PERI_PCLK_HZ, hz) - 1;
	assert(src_clk_div < 128);

	switch (periph) {
	case SCLK_SARADC:
		rk_clrsetreg(&cru->cru_clksel_con[24],
			     SARADC_DIV_MASK,
			     src_clk_div << SARADC_DIV_SHIFT);
		break;
	case SCLK_TSADC:
		rk_clrsetreg(&cru->cru_clksel_con[34],
			     SARADC_DIV_MASK,
			     src_clk_div << SARADC_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rk3066_clk_saradc_get_clk(cru, periph);
}

static void rk3066_clk_init(struct rk3066_cru *cru, struct rk3066_grf *grf)
{
	u32 aclk_div, hclk_div, pclk_div, h2p_div;

	/* Enter PLL slow mode. */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK |
		     CPLL_MODE_MASK,
		     PLL_MODE_SLOW << GPLL_MODE_SHIFT |
		     PLL_MODE_SLOW << CPLL_MODE_SHIFT);

	/* Init PLL. */
	rk3066_clk_set_pll(cru, CLK_GENERAL, &gpll_init_cfg);
	rk3066_clk_set_pll(cru, CLK_CODEC, &cpll_init_cfg);

	/* Wait for PLL lock. */
	while ((readl(&grf->soc_status0) &
		(SOCSTS_CPLL_LOCK | SOCSTS_GPLL_LOCK)) !=
	       (SOCSTS_CPLL_LOCK | SOCSTS_GPLL_LOCK))
		udelay(1);

	/*
	 * Select CPU clock PLL source and
	 * reparent aclk_cpu_pre from APPL to GPLL.
	 * Set up dependent divisors for PCLK/HCLK and ACLK clocks.
	 */
	aclk_div = DIV_ROUND_UP(GPLL_HZ, CPU_ACLK_HZ) - 1;
	assert((aclk_div + 1) * CPU_ACLK_HZ == GPLL_HZ && aclk_div <= 0x1f);

	rk_clrsetreg(&cru->cru_clksel_con[0],
		     CPU_ACLK_PLL_MASK |
		     A9_CORE_DIV_MASK,
		     CPU_ACLK_PLL_SELECT_GPLL << CPU_ACLK_PLL_SHIFT |
		     aclk_div << A9_CORE_DIV_SHIFT);

	hclk_div = ilog2(CPU_ACLK_HZ / CPU_HCLK_HZ);
	assert((1 << hclk_div) * CPU_HCLK_HZ == CPU_ACLK_HZ && hclk_div < 0x3);
	pclk_div = ilog2(CPU_ACLK_HZ / CPU_PCLK_HZ);
	assert((1 << pclk_div) * CPU_PCLK_HZ == CPU_ACLK_HZ && pclk_div < 0x4);
	h2p_div = ilog2(CPU_HCLK_HZ / CPU_H2P_HZ);
	assert((1 << h2p_div) * CPU_H2P_HZ == CPU_HCLK_HZ && pclk_div < 0x3);

	rk_clrsetreg(&cru->cru_clksel_con[1],
		     AHB2APB_DIV_MASK |
		     CPU_PCLK_DIV_MASK |
		     CPU_HCLK_DIV_MASK,
		     h2p_div << AHB2APB_DIV_SHIFT |
		     pclk_div << CPU_PCLK_DIV_SHIFT |
		     hclk_div << CPU_HCLK_DIV_SHIFT);

	/*
	 * Select PERI clock PLL source and
	 * set up dependent divisors for PCLK/HCLK and ACLK clocks.
	 */
	aclk_div = GPLL_HZ / PERI_ACLK_HZ - 1;
	assert((aclk_div + 1) * PERI_ACLK_HZ == GPLL_HZ && aclk_div < 0x1f);

	hclk_div = ilog2(PERI_ACLK_HZ / PERI_HCLK_HZ);
	assert((1 << hclk_div) * PERI_HCLK_HZ ==
	       PERI_ACLK_HZ && (hclk_div < 0x4));

	pclk_div = ilog2(PERI_ACLK_HZ / PERI_PCLK_HZ);
	assert((1 << pclk_div) * PERI_PCLK_HZ ==
	       PERI_ACLK_HZ && (pclk_div < 0x4));

	rk_clrsetreg(&cru->cru_clksel_con[10],
		     PERI_PCLK_DIV_MASK |
		     PERI_HCLK_DIV_MASK |
		     PERI_ACLK_DIV_MASK,
		     PERI_SEL_GPLL << PERI_SEL_PLL_SHIFT |
		     pclk_div << PERI_PCLK_DIV_SHIFT |
		     hclk_div << PERI_HCLK_DIV_SHIFT |
		     aclk_div << PERI_ACLK_DIV_SHIFT);

	/* Enter PLL normal mode. */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK |
		     CPLL_MODE_MASK,
		     PLL_MODE_NORMAL << GPLL_MODE_SHIFT |
		     PLL_MODE_NORMAL << CPLL_MODE_SHIFT);

	rk3066_clk_mmc_set_clk(cru, PERI_HCLK_HZ, HCLK_SDMMC, 16000000);
}

static ulong rk3066_clk_get_rate(struct clk *clk)
{
	struct rk3066_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate, gclk_rate;

	gclk_rate = rk3066_clk_pll_get_rate(priv->cru, CLK_GENERAL);
	switch (clk->id) {
	case 1 ... 4:
		new_rate = rk3066_clk_pll_get_rate(priv->cru, clk->id);
		break;
	case HCLK_EMMC:
	case HCLK_SDMMC:
	case HCLK_SDIO:
	case SCLK_EMMC:
	case SCLK_SDMMC:
	case SCLK_SDIO:
		new_rate = rk3066_clk_mmc_get_clk(priv->cru, PERI_HCLK_HZ,
						  clk->id);
		break;
	case SCLK_SPI0:
	case SCLK_SPI1:
		new_rate = rk3066_clk_spi_get_clk(priv->cru, PERI_PCLK_HZ,
						  clk->id);
		break;
	case PCLK_I2C0:
	case PCLK_I2C1:
	case PCLK_I2C2:
	case PCLK_I2C3:
	case PCLK_I2C4:
		return gclk_rate;
	case SCLK_SARADC:
	case SCLK_TSADC:
		new_rate = rk3066_clk_saradc_get_clk(priv->cru, clk->id);
		break;
	case SCLK_TIMER0:
	case SCLK_TIMER1:
	case SCLK_TIMER2:
	case SCLK_UART0:
	case SCLK_UART1:
	case SCLK_UART2:
	case SCLK_UART3:
		return OSC_HZ;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static ulong rk3066_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3066_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3066_cru *cru = priv->cru;
	ulong new_rate;

	switch (clk->id) {
	case PLL_APLL:
		new_rate = rk3066_clk_configure_cpu(priv->cru, priv->grf, rate);
		break;
	case CLK_DDR:
		new_rate = rk3066_clk_configure_ddr(priv->cru, priv->grf, rate);
		break;
	case HCLK_EMMC:
	case HCLK_SDMMC:
	case HCLK_SDIO:
	case SCLK_EMMC:
	case SCLK_SDMMC:
	case SCLK_SDIO:
		new_rate = rk3066_clk_mmc_set_clk(cru, PERI_HCLK_HZ,
						  clk->id, rate);
		break;
	case SCLK_SPI0:
	case SCLK_SPI1:
		new_rate = rk3066_clk_spi_set_clk(cru, PERI_PCLK_HZ,
						  clk->id, rate);
		break;
	case SCLK_SARADC:
	case SCLK_TSADC:
		new_rate = rk3066_clk_saradc_set_clk(cru, rate, clk->id);
		break;
	case PLL_CPLL:
	case PLL_GPLL:
	case ACLK_CPU:
	case HCLK_CPU:
	case PCLK_CPU:
	case ACLK_PERI:
	case HCLK_PERI:
	case PCLK_PERI:
		return 0;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static int rk3066_clk_enable(struct clk *clk)
{
	struct rk3066_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case HCLK_NANDC0:
		rk_clrreg(&priv->cru->cru_clkgate_con[5], BIT(9));
		break;
	case HCLK_SDMMC:
		rk_clrreg(&priv->cru->cru_clkgate_con[5], BIT(10));
		break;
	case HCLK_SDIO:
		rk_clrreg(&priv->cru->cru_clkgate_con[5], BIT(11));
		break;
	}

	return 0;
}

static int rk3066_clk_disable(struct clk *clk)
{
	struct rk3066_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case HCLK_NANDC0:
		rk_setreg(&priv->cru->cru_clkgate_con[5], BIT(9));
		break;
	case HCLK_SDMMC:
		rk_setreg(&priv->cru->cru_clkgate_con[5], BIT(10));
		break;
	case HCLK_SDIO:
		rk_setreg(&priv->cru->cru_clkgate_con[5], BIT(11));
		break;
	}

	return 0;
}

static struct clk_ops rk3066_clk_ops = {
	.disable	= rk3066_clk_disable,
	.enable	= rk3066_clk_enable,
	.get_rate	= rk3066_clk_get_rate,
	.set_rate	= rk3066_clk_set_rate,
};

static int rk3066_clk_of_to_plat(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		struct rk3066_clk_priv *priv = dev_get_priv(dev);

		priv->cru = dev_read_addr_ptr(dev);
	}

	return 0;
}

static int rk3066_clk_probe(struct udevice *dev)
{
	struct rk3066_clk_priv *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(priv->grf))
		return PTR_ERR(priv->grf);

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3066_clk_plat *plat = dev_get_plat(dev);

	priv->cru = map_sysmem(plat->dtd.reg[0], plat->dtd.reg[1]);
#endif

	if (IS_ENABLED(CONFIG_TPL_BUILD)) {
		rk3066_clk_init(priv->cru, priv->grf);

		/* Init CPU frequency. */
		rk3066_clk_configure_cpu(priv->cru, priv->grf, APLL_SAFE_HZ);
	}

	return 0;
}

static int rk3066_clk_bind(struct udevice *dev)
{
	struct udevice *sys_child;
	struct sysreset_reg *priv;
	int reg_offset, ret;

	/* The reset driver does not have a device node, so bind it here. */
	ret = device_bind(dev, DM_DRIVER_GET(sysreset_rockchip), "sysreset",
			  NULL, ofnode_null(), &sys_child);
	if (ret) {
		dev_dbg(dev, "Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rk3066_cru,
						    cru_glb_srst_fst_value);
		priv->glb_srst_snd_value = offsetof(struct rk3066_cru,
						    cru_glb_srst_snd_value);
		dev_set_priv(sys_child, priv);
	}

	if (CONFIG_IS_ENABLED(RESET_ROCKCHIP)) {
		reg_offset = offsetof(struct rk3066_cru, cru_softrst_con[0]);
		ret = rockchip_reset_bind(dev, reg_offset, 9);
		if (ret)
			dev_dbg(dev, "Warning: software reset driver bind failed\n");
	}

	return 0;
}

static const struct udevice_id rk3066_clk_ids[] = {
	{ .compatible = "rockchip,rk3066a-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3066a_cru) = {
	.name		= "rockchip_rk3066a_cru",
	.id		= UCLASS_CLK,
	.ops		= &rk3066_clk_ops,
	.probe		= rk3066_clk_probe,
	.bind		= rk3066_clk_bind,
	.of_match	= rk3066_clk_ids,
	.of_to_plat	= rk3066_clk_of_to_plat,
	.priv_auto	= sizeof(struct rk3066_clk_priv),
	.plat_auto	= sizeof(struct rk3066_clk_plat),
};
