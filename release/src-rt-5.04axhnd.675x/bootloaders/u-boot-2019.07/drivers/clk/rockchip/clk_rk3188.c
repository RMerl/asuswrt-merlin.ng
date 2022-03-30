// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015 Google, Inc
 * (C) Copyright 2016 Heiko Stuebner <heiko@sntech.de>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <mapmem.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3188.h>
#include <asm/arch-rockchip/grf_rk3188.h>
#include <asm/arch-rockchip/hardware.h>
#include <dt-bindings/clock/rk3188-cru.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <linux/log2.h>

enum rk3188_clk_type {
	RK3188_CRU,
	RK3188A_CRU,
};

struct rk3188_clk_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3188_cru dtd;
#endif
};

struct pll_div {
	u32 nr;
	u32 nf;
	u32 no;
};

enum {
	VCO_MAX_HZ	= 2200U * 1000000,
	VCO_MIN_HZ	= 440 * 1000000,
	OUTPUT_MAX_HZ	= 2200U * 1000000,
	OUTPUT_MIN_HZ	= 30 * 1000000,
	FREF_MAX_HZ	= 2200U * 1000000,
	FREF_MIN_HZ	= 30 * 1000,
};

enum {
	/* PLL CON0 */
	PLL_OD_MASK		= 0x0f,

	/* PLL CON1 */
	PLL_NF_MASK		= 0x1fff,

	/* PLL CON2 */
	PLL_BWADJ_MASK		= 0x0fff,

	/* PLL CON3 */
	PLL_RESET_SHIFT		= 5,

	/* GRF_SOC_STATUS0 */
	SOCSTS_DPLL_LOCK	= 1 << 5,
	SOCSTS_APLL_LOCK	= 1 << 6,
	SOCSTS_CPLL_LOCK	= 1 << 7,
	SOCSTS_GPLL_LOCK	= 1 << 8,
};

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _nr, _no) {\
	.nr = _nr, .nf = (u32)((u64)hz * _nr * _no / OSC_HZ), .no = _no};\
	_Static_assert(((u64)hz * _nr * _no / OSC_HZ) * OSC_HZ /\
		       (_nr * _no) == hz, #hz "Hz cannot be hit with PLL "\
		       "divisors on line " __stringify(__LINE__));

/* Keep divisors as low as possible to reduce jitter and power usage */
#ifdef CONFIG_SPL_BUILD
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 2, 2);
static const struct pll_div cpll_init_cfg = PLL_DIVISORS(CPLL_HZ, 1, 2);
#endif

static int rkclk_set_pll(struct rk3188_cru *cru, enum rk_clk_id clk_id,
			 const struct pll_div *div, bool has_bwadj)
{
	int pll_id = rk_pll_id(clk_id);
	struct rk3188_pll *pll = &cru->pll[pll_id];
	/* All PLLs have same VCO and output frequency range restrictions. */
	uint vco_hz = OSC_HZ / 1000 * div->nf / div->nr * 1000;
	uint output_hz = vco_hz / div->no;

	debug("PLL at %x: nf=%d, nr=%d, no=%d, vco=%u Hz, output=%u Hz\n",
	      (uint)pll, div->nf, div->nr, div->no, vco_hz, output_hz);
	assert(vco_hz >= VCO_MIN_HZ && vco_hz <= VCO_MAX_HZ &&
	       output_hz >= OUTPUT_MIN_HZ && output_hz <= OUTPUT_MAX_HZ &&
	       (div->no == 1 || !(div->no % 2)));

	/* enter reset */
	rk_setreg(&pll->con3, 1 << PLL_RESET_SHIFT);

	rk_clrsetreg(&pll->con0,
		     CLKR_MASK << CLKR_SHIFT | PLL_OD_MASK,
		     ((div->nr - 1) << CLKR_SHIFT) | (div->no - 1));
	rk_clrsetreg(&pll->con1, CLKF_MASK, div->nf - 1);

	if (has_bwadj)
		rk_clrsetreg(&pll->con2, PLL_BWADJ_MASK, (div->nf >> 1) - 1);

	udelay(10);

	/* return from reset */
	rk_clrreg(&pll->con3, 1 << PLL_RESET_SHIFT);

	return 0;
}

static int rkclk_configure_ddr(struct rk3188_cru *cru, struct rk3188_grf *grf,
			       unsigned int hz, bool has_bwadj)
{
	static const struct pll_div dpll_cfg[] = {
		{.nf = 75, .nr = 1, .no = 6},
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
		debug("Unsupported SDRAM frequency");
		return -EINVAL;
	}

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK << DPLL_MODE_SHIFT,
		     DPLL_MODE_SLOW << DPLL_MODE_SHIFT);

	rkclk_set_pll(cru, CLK_DDR, &dpll_cfg[cfg], has_bwadj);

	/* wait for pll lock */
	while (!(readl(&grf->soc_status0) & SOCSTS_DPLL_LOCK))
		udelay(1);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK << DPLL_MODE_SHIFT,
		     DPLL_MODE_NORMAL << DPLL_MODE_SHIFT);

	return 0;
}

static int rkclk_configure_cpu(struct rk3188_cru *cru, struct rk3188_grf *grf,
			      unsigned int hz, bool has_bwadj)
{
	static const struct pll_div apll_cfg[] = {
		{.nf = 50, .nr = 1, .no = 2},
		{.nf = 67, .nr = 1, .no = 1},
	};
	int div_core_peri, div_aclk_core, cfg;

	/*
	 * We support two possible frequencies, the safe 600MHz
	 * which will work with default pmic settings and will
	 * be set in SPL to get away from the 24MHz default and
	 * the maximum of 1.6Ghz, which boards can set if they
	 * were able to get pmic support for it.
	 */
	switch (hz) {
	case APLL_SAFE_HZ:
		cfg = 0;
		div_core_peri = 1;
		div_aclk_core = 3;
		break;
	case APLL_HZ:
		cfg = 1;
		div_core_peri = 2;
		div_aclk_core = 3;
		break;
	default:
		debug("Unsupported ARMCLK frequency");
		return -EINVAL;
	}

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con, APLL_MODE_MASK << APLL_MODE_SHIFT,
		     APLL_MODE_SLOW << APLL_MODE_SHIFT);

	rkclk_set_pll(cru, CLK_ARM, &apll_cfg[cfg], has_bwadj);

	/* waiting for pll lock */
	while (!(readl(&grf->soc_status0) & SOCSTS_APLL_LOCK))
		udelay(1);

	/* Set divider for peripherals attached to the cpu core. */
	rk_clrsetreg(&cru->cru_clksel_con[0],
		CORE_PERI_DIV_MASK << CORE_PERI_DIV_SHIFT,
		div_core_peri << CORE_PERI_DIV_SHIFT);

	/* set up dependent divisor for aclk_core */
	rk_clrsetreg(&cru->cru_clksel_con[1],
		CORE_ACLK_DIV_MASK << CORE_ACLK_DIV_SHIFT,
		div_aclk_core << CORE_ACLK_DIV_SHIFT);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con, APLL_MODE_MASK << APLL_MODE_SHIFT,
		     APLL_MODE_NORMAL << APLL_MODE_SHIFT);

	return hz;
}

/* Get pll rate by id */
static uint32_t rkclk_pll_get_rate(struct rk3188_cru *cru,
				   enum rk_clk_id clk_id)
{
	uint32_t nr, no, nf;
	uint32_t con;
	int pll_id = rk_pll_id(clk_id);
	struct rk3188_pll *pll = &cru->pll[pll_id];
	static u8 clk_shift[CLK_COUNT] = {
		0xff, APLL_MODE_SHIFT, DPLL_MODE_SHIFT, CPLL_MODE_SHIFT,
		GPLL_MODE_SHIFT
	};
	uint shift;

	con = readl(&cru->cru_mode_con);
	shift = clk_shift[clk_id];
	switch ((con >> shift) & APLL_MODE_MASK) {
	case APLL_MODE_SLOW:
		return OSC_HZ;
	case APLL_MODE_NORMAL:
		/* normal mode */
		con = readl(&pll->con0);
		no = ((con >> CLKOD_SHIFT) & CLKOD_MASK) + 1;
		nr = ((con >> CLKR_SHIFT) & CLKR_MASK) + 1;
		con = readl(&pll->con1);
		nf = ((con >> CLKF_SHIFT) & CLKF_MASK) + 1;

		return (24 * nf / (nr * no)) * 1000000;
	case APLL_MODE_DEEP:
	default:
		return 32768;
	}
}

static ulong rockchip_mmc_get_clk(struct rk3188_cru *cru, uint gclk_rate,
				  int periph)
{
	uint div;
	u32 con;

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
		con = readl(&cru->cru_clksel_con[12]);
		div = (con >> EMMC_DIV_SHIFT) & EMMC_DIV_MASK;
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		con = readl(&cru->cru_clksel_con[11]);
		div = (con >> MMC0_DIV_SHIFT) & MMC0_DIV_MASK;
		break;
	case HCLK_SDIO:
	case SCLK_SDIO:
		con = readl(&cru->cru_clksel_con[12]);
		div = (con >> SDIO_DIV_SHIFT) & SDIO_DIV_MASK;
		break;
	default:
		return -EINVAL;
	}

	return DIV_TO_RATE(gclk_rate, div) / 2;
}

static ulong rockchip_mmc_set_clk(struct rk3188_cru *cru, uint gclk_rate,
				  int  periph, uint freq)
{
	int src_clk_div;

	debug("%s: gclk_rate=%u\n", __func__, gclk_rate);
	/* mmc clock defaulg div 2 internal, need provide double in cru */
	src_clk_div = DIV_ROUND_UP(gclk_rate / 2, freq) - 1;
	assert(src_clk_div <= 0x3f);

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     EMMC_DIV_MASK << EMMC_DIV_SHIFT,
			     src_clk_div << EMMC_DIV_SHIFT);
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		rk_clrsetreg(&cru->cru_clksel_con[11],
			     MMC0_DIV_MASK << MMC0_DIV_SHIFT,
			     src_clk_div << MMC0_DIV_SHIFT);
		break;
	case HCLK_SDIO:
	case SCLK_SDIO:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     SDIO_DIV_MASK << SDIO_DIV_SHIFT,
			     src_clk_div << SDIO_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rockchip_mmc_get_clk(cru, gclk_rate, periph);
}

static ulong rockchip_spi_get_clk(struct rk3188_cru *cru, uint gclk_rate,
				  int periph)
{
	uint div;
	u32 con;

	switch (periph) {
	case SCLK_SPI0:
		con = readl(&cru->cru_clksel_con[25]);
		div = (con >> SPI0_DIV_SHIFT) & SPI0_DIV_MASK;
		break;
	case SCLK_SPI1:
		con = readl(&cru->cru_clksel_con[25]);
		div = (con >> SPI1_DIV_SHIFT) & SPI1_DIV_MASK;
		break;
	default:
		return -EINVAL;
	}

	return DIV_TO_RATE(gclk_rate, div);
}

static ulong rockchip_spi_set_clk(struct rk3188_cru *cru, uint gclk_rate,
				  int periph, uint freq)
{
	int src_clk_div = DIV_ROUND_UP(gclk_rate, freq) - 1;

	assert(src_clk_div < 128);
	switch (periph) {
	case SCLK_SPI0:
		assert(src_clk_div <= SPI0_DIV_MASK);
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI0_DIV_MASK << SPI0_DIV_SHIFT,
			     src_clk_div << SPI0_DIV_SHIFT);
		break;
	case SCLK_SPI1:
		assert(src_clk_div <= SPI1_DIV_MASK);
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI1_DIV_MASK << SPI1_DIV_SHIFT,
			     src_clk_div << SPI1_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rockchip_spi_get_clk(cru, gclk_rate, periph);
}

#ifdef CONFIG_SPL_BUILD
static void rkclk_init(struct rk3188_cru *cru, struct rk3188_grf *grf,
		       bool has_bwadj)
{
	u32 aclk_div, hclk_div, pclk_div, h2p_div;

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK << GPLL_MODE_SHIFT |
		     CPLL_MODE_MASK << CPLL_MODE_SHIFT,
		     GPLL_MODE_SLOW << GPLL_MODE_SHIFT |
		     CPLL_MODE_SLOW << CPLL_MODE_SHIFT);

	/* init pll */
	rkclk_set_pll(cru, CLK_GENERAL, &gpll_init_cfg, has_bwadj);
	rkclk_set_pll(cru, CLK_CODEC, &cpll_init_cfg, has_bwadj);

	/* waiting for pll lock */
	while ((readl(&grf->soc_status0) &
			(SOCSTS_CPLL_LOCK | SOCSTS_GPLL_LOCK)) !=
			(SOCSTS_CPLL_LOCK | SOCSTS_GPLL_LOCK))
		udelay(1);

	/*
	 * cpu clock pll source selection and
	 * reparent aclk_cpu_pre from apll to gpll
	 * set up dependent divisors for PCLK/HCLK and ACLK clocks.
	 */
	aclk_div = DIV_ROUND_UP(GPLL_HZ, CPU_ACLK_HZ) - 1;
	assert((aclk_div + 1) * CPU_ACLK_HZ == GPLL_HZ && aclk_div <= 0x1f);

	rk_clrsetreg(&cru->cru_clksel_con[0],
		     CPU_ACLK_PLL_MASK << CPU_ACLK_PLL_SHIFT |
		     A9_CPU_DIV_MASK << A9_CPU_DIV_SHIFT,
		     CPU_ACLK_PLL_SELECT_GPLL << CPU_ACLK_PLL_SHIFT |
		     aclk_div << A9_CPU_DIV_SHIFT);

	hclk_div = ilog2(CPU_ACLK_HZ / CPU_HCLK_HZ);
	assert((1 << hclk_div) * CPU_HCLK_HZ == CPU_ACLK_HZ && hclk_div < 0x3);
	pclk_div = ilog2(CPU_ACLK_HZ / CPU_PCLK_HZ);
	assert((1 << pclk_div) * CPU_PCLK_HZ == CPU_ACLK_HZ && pclk_div < 0x4);
	h2p_div = ilog2(CPU_HCLK_HZ / CPU_H2P_HZ);
	assert((1 << h2p_div) * CPU_H2P_HZ == CPU_HCLK_HZ && pclk_div < 0x3);

	rk_clrsetreg(&cru->cru_clksel_con[1],
		     AHB2APB_DIV_MASK << AHB2APB_DIV_SHIFT |
		     CPU_PCLK_DIV_MASK << CPU_PCLK_DIV_SHIFT |
		     CPU_HCLK_DIV_MASK << CPU_HCLK_DIV_SHIFT,
		     h2p_div << AHB2APB_DIV_SHIFT |
		     pclk_div << CPU_PCLK_DIV_SHIFT |
		     hclk_div << CPU_HCLK_DIV_SHIFT);

	/*
	 * peri clock pll source selection and
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
		     PERI_PCLK_DIV_MASK << PERI_PCLK_DIV_SHIFT |
		     PERI_HCLK_DIV_MASK << PERI_HCLK_DIV_SHIFT |
		     PERI_ACLK_DIV_MASK << PERI_ACLK_DIV_SHIFT,
		     PERI_SEL_GPLL << PERI_SEL_PLL_SHIFT |
		     pclk_div << PERI_PCLK_DIV_SHIFT |
		     hclk_div << PERI_HCLK_DIV_SHIFT |
		     aclk_div << PERI_ACLK_DIV_SHIFT);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK << GPLL_MODE_SHIFT |
		     CPLL_MODE_MASK << CPLL_MODE_SHIFT,
		     GPLL_MODE_NORMAL << GPLL_MODE_SHIFT |
		     CPLL_MODE_NORMAL << CPLL_MODE_SHIFT);

	rockchip_mmc_set_clk(cru, PERI_HCLK_HZ, HCLK_SDMMC, 16000000);
}
#endif

static ulong rk3188_clk_get_rate(struct clk *clk)
{
	struct rk3188_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate, gclk_rate;

	gclk_rate = rkclk_pll_get_rate(priv->cru, CLK_GENERAL);
	switch (clk->id) {
	case 1 ... 4:
		new_rate = rkclk_pll_get_rate(priv->cru, clk->id);
		break;
	case HCLK_EMMC:
	case HCLK_SDMMC:
	case HCLK_SDIO:
	case SCLK_EMMC:
	case SCLK_SDMMC:
	case SCLK_SDIO:
		new_rate = rockchip_mmc_get_clk(priv->cru, PERI_HCLK_HZ,
						clk->id);
		break;
	case SCLK_SPI0:
	case SCLK_SPI1:
		new_rate = rockchip_spi_get_clk(priv->cru, PERI_PCLK_HZ,
						clk->id);
		break;
	case PCLK_I2C0:
	case PCLK_I2C1:
	case PCLK_I2C2:
	case PCLK_I2C3:
	case PCLK_I2C4:
		return gclk_rate;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static ulong rk3188_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3188_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3188_cru *cru = priv->cru;
	ulong new_rate;

	switch (clk->id) {
	case PLL_APLL:
		new_rate = rkclk_configure_cpu(priv->cru, priv->grf, rate,
					       priv->has_bwadj);
		break;
	case CLK_DDR:
		new_rate = rkclk_configure_ddr(priv->cru, priv->grf, rate,
					       priv->has_bwadj);
		break;
	case HCLK_EMMC:
	case HCLK_SDMMC:
	case HCLK_SDIO:
	case SCLK_EMMC:
	case SCLK_SDMMC:
	case SCLK_SDIO:
		new_rate = rockchip_mmc_set_clk(cru, PERI_HCLK_HZ,
						clk->id, rate);
		break;
	case SCLK_SPI0:
	case SCLK_SPI1:
		new_rate = rockchip_spi_set_clk(cru, PERI_PCLK_HZ,
						clk->id, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static struct clk_ops rk3188_clk_ops = {
	.get_rate	= rk3188_clk_get_rate,
	.set_rate	= rk3188_clk_set_rate,
};

static int rk3188_clk_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3188_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);
#endif

	return 0;
}

static int rk3188_clk_probe(struct udevice *dev)
{
	struct rk3188_clk_priv *priv = dev_get_priv(dev);
	enum rk3188_clk_type type = dev_get_driver_data(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(priv->grf))
		return PTR_ERR(priv->grf);
	priv->has_bwadj = (type == RK3188A_CRU) ? 1 : 0;

#ifdef CONFIG_SPL_BUILD
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3188_clk_plat *plat = dev_get_platdata(dev);

	priv->cru = map_sysmem(plat->dtd.reg[0], plat->dtd.reg[1]);
#endif

	rkclk_init(priv->cru, priv->grf, priv->has_bwadj);
#endif

	return 0;
}

static int rk3188_clk_bind(struct udevice *dev)
{
	int ret;
	struct udevice *sys_child;
	struct sysreset_reg *priv;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset",
				 &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rk3188_cru,
						    cru_glb_srst_fst_value);
		priv->glb_srst_snd_value = offsetof(struct rk3188_cru,
						    cru_glb_srst_snd_value);
		sys_child->priv = priv;
	}

#if CONFIG_IS_ENABLED(CONFIG_RESET_ROCKCHIP)
	ret = offsetof(struct rk3188_cru, cru_softrst_con[0]);
	ret = rockchip_reset_bind(dev, ret, 9);
	if (ret)
		debug("Warning: software reset driver bind faile\n");
#endif

	return 0;
}

static const struct udevice_id rk3188_clk_ids[] = {
	{ .compatible = "rockchip,rk3188-cru", .data = RK3188_CRU },
	{ .compatible = "rockchip,rk3188a-cru", .data = RK3188A_CRU },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3188_cru) = {
	.name			= "rockchip_rk3188_cru",
	.id			= UCLASS_CLK,
	.of_match		= rk3188_clk_ids,
	.priv_auto_alloc_size	= sizeof(struct rk3188_clk_priv),
	.platdata_auto_alloc_size = sizeof(struct rk3188_clk_plat),
	.ops			= &rk3188_clk_ops,
	.bind			= rk3188_clk_bind,
	.ofdata_to_platdata	= rk3188_clk_ofdata_to_platdata,
	.probe			= rk3188_clk_probe,
};
