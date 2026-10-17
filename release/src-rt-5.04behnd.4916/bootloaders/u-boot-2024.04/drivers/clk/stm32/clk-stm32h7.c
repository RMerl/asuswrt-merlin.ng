// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/root.h>
#include <linux/bitops.h>

#include <dt-bindings/clock/stm32h7-clks.h>

/* RCC CR specific definitions */
#define RCC_CR_HSION			BIT(0)
#define RCC_CR_HSIRDY			BIT(2)

#define RCC_CR_HSEON			BIT(16)
#define RCC_CR_HSERDY			BIT(17)
#define RCC_CR_HSEBYP			BIT(18)
#define RCC_CR_PLL1ON			BIT(24)
#define RCC_CR_PLL1RDY			BIT(25)

#define RCC_CR_HSIDIV_MASK		GENMASK(4, 3)
#define RCC_CR_HSIDIV_SHIFT		3

#define RCC_CFGR_SW_MASK		GENMASK(2, 0)
#define RCC_CFGR_SW_HSI			0
#define RCC_CFGR_SW_CSI			1
#define RCC_CFGR_SW_HSE			2
#define RCC_CFGR_SW_PLL1		3
#define RCC_CFGR_TIMPRE			BIT(15)

#define RCC_PLLCKSELR_PLLSRC_HSI	0
#define RCC_PLLCKSELR_PLLSRC_CSI	1
#define RCC_PLLCKSELR_PLLSRC_HSE	2
#define RCC_PLLCKSELR_PLLSRC_NO_CLK	3

#define RCC_PLLCKSELR_PLLSRC_MASK	GENMASK(1, 0)

#define RCC_PLLCKSELR_DIVM1_SHIFT	4
#define RCC_PLLCKSELR_DIVM1_MASK	GENMASK(9, 4)

#define RCC_PLL1DIVR_DIVN1_MASK		GENMASK(8, 0)

#define RCC_PLL1DIVR_DIVP1_SHIFT	9
#define RCC_PLL1DIVR_DIVP1_MASK		GENMASK(15, 9)

#define RCC_PLL1DIVR_DIVQ1_SHIFT	16
#define RCC_PLL1DIVR_DIVQ1_MASK		GENMASK(22, 16)

#define RCC_PLL1DIVR_DIVR1_SHIFT	24
#define RCC_PLL1DIVR_DIVR1_MASK		GENMASK(30, 24)

#define RCC_PLL1FRACR_FRACN1_SHIFT	3
#define RCC_PLL1FRACR_FRACN1_MASK	GENMASK(15, 3)

#define RCC_PLLCFGR_PLL1RGE_SHIFT	2
#define		PLL1RGE_1_2_MHZ		0
#define		PLL1RGE_2_4_MHZ		1
#define		PLL1RGE_4_8_MHZ		2
#define		PLL1RGE_8_16_MHZ	3
#define RCC_PLLCFGR_DIVP1EN		BIT(16)
#define RCC_PLLCFGR_DIVQ1EN		BIT(17)
#define RCC_PLLCFGR_DIVR1EN		BIT(18)

#define RCC_D1CFGR_HPRE_MASK		GENMASK(3, 0)
#define RCC_D1CFGR_HPRE_DIVIDED		BIT(3)
#define RCC_D1CFGR_HPRE_DIVIDER		GENMASK(2, 0)

#define RCC_D1CFGR_HPRE_DIV2		8

#define RCC_D1CFGR_D1PPRE_SHIFT		4
#define RCC_D1CFGR_D1PPRE_DIVIDED	BIT(6)
#define RCC_D1CFGR_D1PPRE_DIVIDER	GENMASK(5, 4)

#define RCC_D1CFGR_D1CPRE_SHIFT		8
#define RCC_D1CFGR_D1CPRE_DIVIDER	GENMASK(10, 8)
#define RCC_D1CFGR_D1CPRE_DIVIDED	BIT(11)

#define RCC_D2CFGR_D2PPRE1_SHIFT	4
#define RCC_D2CFGR_D2PPRE1_DIVIDED	BIT(6)
#define RCC_D2CFGR_D2PPRE1_DIVIDER	GENMASK(5, 4)

#define RCC_D2CFGR_D2PPRE2_SHIFT	8
#define RCC_D2CFGR_D2PPRE2_DIVIDED	BIT(10)
#define RCC_D2CFGR_D2PPRE2_DIVIDER	GENMASK(9, 8)

#define RCC_D3CFGR_D3PPRE_SHIFT		4
#define RCC_D3CFGR_D3PPRE_DIVIDED	BIT(6)
#define RCC_D3CFGR_D3PPRE_DIVIDER	GENMASK(5, 4)

#define RCC_D1CCIPR_FMCSRC_MASK		GENMASK(1, 0)
#define		FMCSRC_HCLKD1		0
#define		FMCSRC_PLL1_Q_CK	1
#define		FMCSRC_PLL2_R_CK	2
#define		FMCSRC_PER_CK		3

#define RCC_D1CCIPR_QSPISRC_MASK	GENMASK(5, 4)
#define RCC_D1CCIPR_QSPISRC_SHIFT	4
#define		QSPISRC_HCLKD1		0
#define		QSPISRC_PLL1_Q_CK	1
#define		QSPISRC_PLL2_R_CK	2
#define		QSPISRC_PER_CK		3

#define PWR_CR3				0x0c
#define PWR_CR3_SCUEN			BIT(2)
#define PWR_D3CR			0x18
#define PWR_D3CR_VOS_MASK		GENMASK(15, 14)
#define PWR_D3CR_VOS_SHIFT		14
#define		VOS_SCALE_3		1
#define		VOS_SCALE_2		2
#define		VOS_SCALE_1		3
#define PWR_D3CR_VOSREADY		BIT(13)

struct stm32_rcc_regs {
	u32 cr;		/* 0x00 Source Control Register */
	u32 icscr;	/* 0x04 Internal Clock Source Calibration Register */
	u32 crrcr;	/* 0x08 Clock Recovery RC Register */
	u32 reserved1;	/* 0x0c reserved */
	u32 cfgr;	/* 0x10 Clock Configuration Register */
	u32 reserved2;	/* 0x14 reserved */
	u32 d1cfgr;	/* 0x18 Domain 1 Clock Configuration Register */
	u32 d2cfgr;	/* 0x1c Domain 2 Clock Configuration Register */
	u32 d3cfgr;	/* 0x20 Domain 3 Clock Configuration Register */
	u32 reserved3;	/* 0x24 reserved */
	u32 pllckselr;	/* 0x28 PLLs Clock Source Selection Register */
	u32 pllcfgr;	/* 0x2c PLLs Configuration Register */
	u32 pll1divr;	/* 0x30 PLL1 Dividers Configuration Register */
	u32 pll1fracr;	/* 0x34 PLL1 Fractional Divider Register */
	u32 pll2divr;	/* 0x38 PLL2 Dividers Configuration Register */
	u32 pll2fracr;	/* 0x3c PLL2 Fractional Divider Register */
	u32 pll3divr;	/* 0x40 PLL3 Dividers Configuration Register */
	u32 pll3fracr;	/* 0x44 PLL3 Fractional Divider Register */
	u32 reserved4;	/* 0x48 reserved */
	u32 d1ccipr;	/* 0x4c Domain 1 Kernel Clock Configuration Register */
	u32 d2ccip1r;	/* 0x50 Domain 2 Kernel Clock Configuration Register */
	u32 d2ccip2r;	/* 0x54 Domain 2 Kernel Clock Configuration Register */
	u32 d3ccipr;	/* 0x58 Domain 3 Kernel Clock Configuration Register */
	u32 reserved5;	/* 0x5c reserved */
	u32 cier;	/* 0x60 Clock Source Interrupt Enable Register */
	u32 cifr;	/* 0x64 Clock Source Interrupt Flag Register */
	u32 cicr;	/* 0x68 Clock Source Interrupt Clear Register */
	u32 reserved6;	/* 0x6c reserved */
	u32 bdcr;	/* 0x70 Backup Domain Control Register */
	u32 csr;	/* 0x74 Clock Control and Status Register */
	u32 reserved7;	/* 0x78 reserved */

	u32 ahb3rstr;	/* 0x7c AHB3 Peripheral Reset Register */
	u32 ahb1rstr;	/* 0x80 AHB1 Peripheral Reset Register */
	u32 ahb2rstr;	/* 0x84 AHB2 Peripheral Reset Register */
	u32 ahb4rstr;	/* 0x88 AHB4 Peripheral Reset Register */

	u32 apb3rstr;	/* 0x8c APB3 Peripheral Reset Register */
	u32 apb1lrstr;	/* 0x90 APB1 low Peripheral Reset Register */
	u32 apb1hrstr;	/* 0x94 APB1 high Peripheral Reset Register */
	u32 apb2rstr;	/* 0x98 APB2 Clock Register */
	u32 apb4rstr;	/* 0x9c APB4 Clock Register */

	u32 gcr;	/* 0xa0 Global Control Register */
	u32 reserved8;	/* 0xa4 reserved */
	u32 d3amr;	/* 0xa8 D3 Autonomous mode Register */
	u32 reserved9[9];/* 0xac to 0xcc reserved */
	u32 rsr;	/* 0xd0 Reset Status Register */
	u32 ahb3enr;	/* 0xd4 AHB3 Clock Register */
	u32 ahb1enr;	/* 0xd8 AHB1 Clock Register */
	u32 ahb2enr;	/* 0xdc AHB2 Clock Register */
	u32 ahb4enr;	/* 0xe0 AHB4 Clock Register */

	u32 apb3enr;	/* 0xe4 APB3 Clock Register */
	u32 apb1lenr;	/* 0xe8 APB1 low Clock Register */
	u32 apb1henr;	/* 0xec APB1 high Clock Register */
	u32 apb2enr;	/* 0xf0 APB2 Clock Register */
	u32 apb4enr;	/* 0xf4 APB4 Clock Register */
};

#define RCC_AHB3ENR	offsetof(struct stm32_rcc_regs, ahb3enr)
#define RCC_AHB1ENR	offsetof(struct stm32_rcc_regs, ahb1enr)
#define RCC_AHB2ENR	offsetof(struct stm32_rcc_regs, ahb2enr)
#define RCC_AHB4ENR	offsetof(struct stm32_rcc_regs, ahb4enr)
#define RCC_APB3ENR	offsetof(struct stm32_rcc_regs, apb3enr)
#define RCC_APB1LENR	offsetof(struct stm32_rcc_regs, apb1lenr)
#define RCC_APB1HENR	offsetof(struct stm32_rcc_regs, apb1henr)
#define RCC_APB2ENR	offsetof(struct stm32_rcc_regs, apb2enr)
#define RCC_APB4ENR	offsetof(struct stm32_rcc_regs, apb4enr)

struct clk_cfg {
	u32 gate_offset;
	u8  gate_bit_idx;
	const char *name;
};

/*
 * the way all these entries are sorted in this array could seem
 * unlogical, but we are dependant of kernel DT_bindings,
 * where clocks are separate in 2 banks, peripheral clocks and
 * kernel clocks.
 */

static const struct clk_cfg clk_map[] = {
	{RCC_AHB3ENR,  31, "d1sram1"},	/* peripheral clocks */
	{RCC_AHB3ENR,  30, "itcm"},
	{RCC_AHB3ENR,  29, "dtcm2"},
	{RCC_AHB3ENR,  28, "dtcm1"},
	{RCC_AHB3ENR,   8, "flitf"},
	{RCC_AHB3ENR,   5, "jpgdec"},
	{RCC_AHB3ENR,   4, "dma2d"},
	{RCC_AHB3ENR,   0, "mdma"},
	{RCC_AHB1ENR,  28, "usb2ulpi"},
	{RCC_AHB1ENR,  17, "eth1rx"},
	{RCC_AHB1ENR,  16, "eth1tx"},
	{RCC_AHB1ENR,  15, "eth1mac"},
	{RCC_AHB1ENR,  14, "art"},
	{RCC_AHB1ENR,  26, "usb1ulpi"},
	{RCC_AHB1ENR,   1, "dma2"},
	{RCC_AHB1ENR,   0, "dma1"},
	{RCC_AHB2ENR,  31, "d2sram3"},
	{RCC_AHB2ENR,  30, "d2sram2"},
	{RCC_AHB2ENR,  29, "d2sram1"},
	{RCC_AHB2ENR,   5, "hash"},
	{RCC_AHB2ENR,   4, "crypt"},
	{RCC_AHB2ENR,   0, "camitf"},
	{RCC_AHB4ENR,  28, "bkpram"},
	{RCC_AHB4ENR,  25, "hsem"},
	{RCC_AHB4ENR,  21, "bdma"},
	{RCC_AHB4ENR,  19, "crc"},
	{RCC_AHB4ENR,  10, "gpiok"},
	{RCC_AHB4ENR,   9, "gpioj"},
	{RCC_AHB4ENR,   8, "gpioi"},
	{RCC_AHB4ENR,   7, "gpioh"},
	{RCC_AHB4ENR,   6, "gpiog"},
	{RCC_AHB4ENR,   5, "gpiof"},
	{RCC_AHB4ENR,   4, "gpioe"},
	{RCC_AHB4ENR,   3, "gpiod"},
	{RCC_AHB4ENR,   2, "gpioc"},
	{RCC_AHB4ENR,   1, "gpiob"},
	{RCC_AHB4ENR,   0, "gpioa"},
	{RCC_APB3ENR,   6, "wwdg1"},
	{RCC_APB1LENR, 29, "dac12"},
	{RCC_APB1LENR, 11, "wwdg2"},
	{RCC_APB1LENR,  8, "tim14"},
	{RCC_APB1LENR,  7, "tim13"},
	{RCC_APB1LENR,  6, "tim12"},
	{RCC_APB1LENR,  5, "tim7"},
	{RCC_APB1LENR,  4, "tim6"},
	{RCC_APB1LENR,  3, "tim5"},
	{RCC_APB1LENR,  2, "tim4"},
	{RCC_APB1LENR,  1, "tim3"},
	{RCC_APB1LENR,  0, "tim2"},
	{RCC_APB1HENR,  5, "mdios"},
	{RCC_APB1HENR,  4, "opamp"},
	{RCC_APB1HENR,  1, "crs"},
	{RCC_APB2ENR,  18, "tim17"},
	{RCC_APB2ENR,  17, "tim16"},
	{RCC_APB2ENR,  16, "tim15"},
	{RCC_APB2ENR,   1, "tim8"},
	{RCC_APB2ENR,   0, "tim1"},
	{RCC_APB4ENR,  26, "tmpsens"},
	{RCC_APB4ENR,  16, "rtcapb"},
	{RCC_APB4ENR,  15, "vref"},
	{RCC_APB4ENR,  14, "comp12"},
	{RCC_APB4ENR,   1, "syscfg"},
	{RCC_AHB3ENR,  16, "sdmmc1"},	/* kernel clocks */
	{RCC_AHB3ENR,  14, "quadspi"},
	{RCC_AHB3ENR,  12, "fmc"},
	{RCC_AHB1ENR,  27, "usb2otg"},
	{RCC_AHB1ENR,  25, "usb1otg"},
	{RCC_AHB1ENR,   5, "adc12"},
	{RCC_AHB2ENR,   9, "sdmmc2"},
	{RCC_AHB2ENR,   6, "rng"},
	{RCC_AHB4ENR,  24, "adc3"},
	{RCC_APB3ENR,   4, "dsi"},
	{RCC_APB3ENR,   3, "ltdc"},
	{RCC_APB1LENR, 31, "usart8"},
	{RCC_APB1LENR, 30, "usart7"},
	{RCC_APB1LENR, 27, "hdmicec"},
	{RCC_APB1LENR, 23, "i2c3"},
	{RCC_APB1LENR, 22, "i2c2"},
	{RCC_APB1LENR, 21, "i2c1"},
	{RCC_APB1LENR, 20, "uart5"},
	{RCC_APB1LENR, 19, "uart4"},
	{RCC_APB1LENR, 18, "usart3"},
	{RCC_APB1LENR, 17, "usart2"},
	{RCC_APB1LENR, 16, "spdifrx"},
	{RCC_APB1LENR, 15, "spi3"},
	{RCC_APB1LENR, 14, "spi2"},
	{RCC_APB1LENR,  9, "lptim1"},
	{RCC_APB1HENR,  8, "fdcan"},
	{RCC_APB1HENR,  2, "swp"},
	{RCC_APB2ENR,  29, "hrtim"},
	{RCC_APB2ENR,  28, "dfsdm1"},
	{RCC_APB2ENR,  24, "sai3"},
	{RCC_APB2ENR,  23, "sai2"},
	{RCC_APB2ENR,  22, "sai1"},
	{RCC_APB2ENR,  20, "spi5"},
	{RCC_APB2ENR,  13, "spi4"},
	{RCC_APB2ENR,  12, "spi1"},
	{RCC_APB2ENR,   5, "usart6"},
	{RCC_APB2ENR,   4, "usart1"},
	{RCC_APB4ENR,  21, "sai4a"},
	{RCC_APB4ENR,  21, "sai4b"},
	{RCC_APB4ENR,  12, "lptim5"},
	{RCC_APB4ENR,  11, "lptim4"},
	{RCC_APB4ENR,  10, "lptim3"},
	{RCC_APB4ENR,   9, "lptim2"},
	{RCC_APB4ENR,   7, "i2c4"},
	{RCC_APB4ENR,   5,  "spi6"},
	{RCC_APB4ENR,   3, "lpuart1"},
};

struct stm32_clk {
	struct stm32_rcc_regs *rcc_base;
	struct regmap *pwr_regmap;
};

struct pll_psc {
	u8	divm;
	u16	divn;
	u8	divp;
	u8	divq;
	u8	divr;
};

/*
 * OSC_HSE = 25 MHz
 * VCO = 500MHz
 * pll1_p = 250MHz / pll1_q = 250MHz pll1_r = 250Mhz
 */
struct pll_psc sys_pll_psc = {
	.divm = 4,
	.divn = 80,
	.divp = 2,
	.divq = 2,
	.divr = 2,
};

enum apb {
	APB1,
	APB2,
};

int configure_clocks(struct udevice *dev)
{
	struct stm32_clk *priv = dev_get_priv(dev);
	struct stm32_rcc_regs *regs = priv->rcc_base;
	uint8_t *pwr_base = (uint8_t *)regmap_get_range(priv->pwr_regmap, 0);
	uint32_t pllckselr = 0;
	uint32_t pll1divr = 0;
	uint32_t pllcfgr = 0;

	/* Switch on HSI */
	setbits_le32(&regs->cr, RCC_CR_HSION);
	while (!(readl(&regs->cr) & RCC_CR_HSIRDY))
		;

	/* Reset CFGR, now HSI is the default system clock */
	writel(0, &regs->cfgr);

	/* Set all kernel domain clock registers to reset value*/
	writel(0x0, &regs->d1ccipr);
	writel(0x0, &regs->d2ccip1r);
	writel(0x0, &regs->d2ccip2r);

	/* Set voltage scaling at scale 1 (1,15 - 1,26 Volts) */
	clrsetbits_le32(pwr_base + PWR_D3CR, PWR_D3CR_VOS_MASK,
			VOS_SCALE_1 << PWR_D3CR_VOS_SHIFT);
	/* Lock supply configuration update */
	clrbits_le32(pwr_base + PWR_CR3, PWR_CR3_SCUEN);
	while (!(readl(pwr_base + PWR_D3CR) & PWR_D3CR_VOSREADY))
		;

	/* disable HSE to configure it  */
	clrbits_le32(&regs->cr, RCC_CR_HSEON);
	while ((readl(&regs->cr) & RCC_CR_HSERDY))
		;

	/* clear HSE bypass and set it ON */
	clrbits_le32(&regs->cr, RCC_CR_HSEBYP);
	/* Switch on HSE */
	setbits_le32(&regs->cr, RCC_CR_HSEON);
	while (!(readl(&regs->cr) & RCC_CR_HSERDY))
		;

	/* pll setup, disable it */
	clrbits_le32(&regs->cr, RCC_CR_PLL1ON);
	while ((readl(&regs->cr) & RCC_CR_PLL1RDY))
		;

	/* Select HSE as PLL clock source */
	pllckselr |= RCC_PLLCKSELR_PLLSRC_HSE;
	pllckselr |= sys_pll_psc.divm << RCC_PLLCKSELR_DIVM1_SHIFT;
	writel(pllckselr, &regs->pllckselr);

	pll1divr |= (sys_pll_psc.divr - 1) << RCC_PLL1DIVR_DIVR1_SHIFT;
	pll1divr |= (sys_pll_psc.divq - 1) << RCC_PLL1DIVR_DIVQ1_SHIFT;
	pll1divr |= (sys_pll_psc.divp - 1) << RCC_PLL1DIVR_DIVP1_SHIFT;
	pll1divr |= (sys_pll_psc.divn - 1);
	writel(pll1divr, &regs->pll1divr);

	pllcfgr |= PLL1RGE_4_8_MHZ << RCC_PLLCFGR_PLL1RGE_SHIFT;
	pllcfgr |= RCC_PLLCFGR_DIVP1EN;
	pllcfgr |= RCC_PLLCFGR_DIVQ1EN;
	pllcfgr |= RCC_PLLCFGR_DIVR1EN;
	writel(pllcfgr, &regs->pllcfgr);

	/* pll setup, enable it */
	setbits_le32(&regs->cr, RCC_CR_PLL1ON);

	/* set HPRE (/2) DI clk --> 125MHz */
	clrsetbits_le32(&regs->d1cfgr, RCC_D1CFGR_HPRE_MASK,
			RCC_D1CFGR_HPRE_DIV2);

	/*  select PLL1 as system clock source (sys_ck)*/
	clrsetbits_le32(&regs->cfgr, RCC_CFGR_SW_MASK, RCC_CFGR_SW_PLL1);
	while ((readl(&regs->cfgr) & RCC_CFGR_SW_MASK) != RCC_CFGR_SW_PLL1)
		;

	/* sdram: use pll1_q as fmc_k clk */
	clrsetbits_le32(&regs->d1ccipr, RCC_D1CCIPR_FMCSRC_MASK,
			FMCSRC_PLL1_Q_CK);

	return 0;
}

static u32 stm32_get_HSI_divider(struct stm32_rcc_regs *regs)
{
	u32 divider;

	/* get HSI divider value */
	divider = readl(&regs->cr) & RCC_CR_HSIDIV_MASK;
	divider = divider >> RCC_CR_HSIDIV_SHIFT;

	return divider;
};

enum pllsrc {
	HSE,
	LSE,
	HSI,
	CSI,
	I2S,
	TIMER,
	PLLSRC_NB,
};

static const char * const pllsrc_name[PLLSRC_NB] = {
	[HSE] = "clk-hse",
	[LSE] = "clk-lse",
	[HSI] = "clk-hsi",
	[CSI] = "clk-csi",
	[I2S] = "clk-i2s",
	[TIMER] = "timer-clk"
};

static ulong stm32_get_rate(struct stm32_rcc_regs *regs, enum pllsrc pllsrc)
{
	struct clk clk;
	struct udevice *fixed_clock_dev = NULL;
	u32 divider;
	int ret;
	const char *name = pllsrc_name[pllsrc];

	log_debug("pllsrc name %s\n", name);

	clk.id = 0;
	ret = uclass_get_device_by_name(UCLASS_CLK, name, &fixed_clock_dev);
	if (ret) {
		log_err("Can't find clk %s (%d)", name, ret);
		return 0;
	}

	ret = clk_request(fixed_clock_dev, &clk);
	if (ret) {
		log_err("Can't request %s clk (%d)", name, ret);
		return 0;
	}

	divider = 0;
	if (pllsrc == HSI)
		divider = stm32_get_HSI_divider(regs);

	log_debug("divider %d rate %ld\n", divider, clk_get_rate(&clk));

	return clk_get_rate(&clk) >> divider;
};

enum pll1_output {
	PLL1_P_CK,
	PLL1_Q_CK,
	PLL1_R_CK,
};

static u32 stm32_get_PLL1_rate(struct stm32_rcc_regs *regs,
			       enum pll1_output output)
{
	ulong pllsrc = 0;
	u32 divm1, divn1, divp1, divq1, divr1, fracn1;
	ulong vco, rate;

	/* get the PLLSRC */
	switch (readl(&regs->pllckselr) & RCC_PLLCKSELR_PLLSRC_MASK) {
	case RCC_PLLCKSELR_PLLSRC_HSI:
		pllsrc = stm32_get_rate(regs, HSI);
		break;
	case RCC_PLLCKSELR_PLLSRC_CSI:
		pllsrc = stm32_get_rate(regs, CSI);
		break;
	case RCC_PLLCKSELR_PLLSRC_HSE:
		pllsrc = stm32_get_rate(regs, HSE);
		break;
	case RCC_PLLCKSELR_PLLSRC_NO_CLK:
		/* shouldn't happen */
		log_err("wrong value for RCC_PLLCKSELR register\n");
		pllsrc = 0;
		break;
	}

	/* pllsrc = 0 ? no need to go ahead */
	if (!pllsrc)
		return pllsrc;

	/* get divm1, divp1, divn1 and divr1 */
	divm1 = readl(&regs->pllckselr) & RCC_PLLCKSELR_DIVM1_MASK;
	divm1 = divm1 >> RCC_PLLCKSELR_DIVM1_SHIFT;

	divn1 = (readl(&regs->pll1divr) & RCC_PLL1DIVR_DIVN1_MASK) + 1;

	divp1 = readl(&regs->pll1divr) & RCC_PLL1DIVR_DIVP1_MASK;
	divp1 = (divp1 >> RCC_PLL1DIVR_DIVP1_SHIFT) + 1;

	divq1 = readl(&regs->pll1divr) & RCC_PLL1DIVR_DIVQ1_MASK;
	divq1 = (divq1 >> RCC_PLL1DIVR_DIVQ1_SHIFT) + 1;

	divr1 = readl(&regs->pll1divr) & RCC_PLL1DIVR_DIVR1_MASK;
	divr1 = (divr1 >> RCC_PLL1DIVR_DIVR1_SHIFT) + 1;

	fracn1 = readl(&regs->pll1fracr) & RCC_PLL1DIVR_DIVR1_MASK;
	fracn1 = fracn1 & RCC_PLL1DIVR_DIVR1_SHIFT;

	vco = (pllsrc / divm1) * divn1;
	rate = (pllsrc * fracn1) / (divm1 * 8192);

	log_debug("divm1 = %d divn1 = %d divp1 = %d divq1 = %d divr1 = %d\n",
		  divm1, divn1, divp1, divq1, divr1);
	log_debug("fracn1 = %d vco = %ld rate = %ld\n",
		  fracn1, vco, rate);

	switch (output) {
	case PLL1_P_CK:
		return (vco + rate) / divp1;
		break;
	case PLL1_Q_CK:
		return (vco + rate) / divq1;
		break;

	case PLL1_R_CK:
		return (vco + rate) / divr1;
		break;
	}

	return -EINVAL;
}

static u32 stm32_get_apb_psc(struct stm32_rcc_regs *regs, enum apb apb)
{
	u16 prescaler_table[8] = {2, 4, 8, 16, 64, 128, 256, 512};
	u32 d2cfgr = readl(&regs->d2cfgr);

	if (apb == APB1) {
		if (d2cfgr & RCC_D2CFGR_D2PPRE1_DIVIDED)
			/* get D2 domain APB1 prescaler */
			return prescaler_table[
				((d2cfgr & RCC_D2CFGR_D2PPRE1_DIVIDER)
				>> RCC_D2CFGR_D2PPRE1_SHIFT)];
	} else  { /* APB2 */
		if (d2cfgr & RCC_D2CFGR_D2PPRE2_DIVIDED)
			/* get D2 domain APB2 prescaler */
			return prescaler_table[
				((d2cfgr & RCC_D2CFGR_D2PPRE2_DIVIDER)
				>> RCC_D2CFGR_D2PPRE2_SHIFT)];
	}

	return 1;
};

static u32 stm32_get_timer_rate(struct stm32_clk *priv, u32 sysclk,
				enum apb apb)
{
	struct stm32_rcc_regs *regs = priv->rcc_base;
u32 psc = stm32_get_apb_psc(regs, apb);

	if (readl(&regs->cfgr) & RCC_CFGR_TIMPRE)
		/*
		 * if APB prescaler is configured to a
		 * division factor of 1, 2 or 4
		 */
		switch (psc) {
		case 1:
		case 2:
		case 4:
			return sysclk;
		case 8:
			return sysclk / 2;
		case 16:
			return sysclk / 4;
		default:
			log_err("unexpected prescaler value (%d)\n", psc);
			return 0;
		}
	else
		switch (psc) {
		case 1:
			return sysclk;
		case 2:
		case 4:
		case 8:
		case 16:
			return sysclk / psc;
		default:
			log_err("unexpected prescaler value (%d)\n", psc);
			return 0;
		}
};

static ulong stm32_clk_get_rate(struct clk *clk)
{
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->rcc_base;
	ulong sysclk = 0;
	u32 gate_offset;
	u32 d1cfgr, d3cfgr;
	/* prescaler table lookups for clock computation */
	u16 prescaler_table[8] = {2, 4, 8, 16, 64, 128, 256, 512};
	u8 source, idx;

	/*
	 * get system clock (sys_ck) source
	 * can be HSI_CK, CSI_CK, HSE_CK or pll1_p_ck
	 */
	source = readl(&regs->cfgr) & RCC_CFGR_SW_MASK;
	switch (source) {
	case RCC_CFGR_SW_PLL1:
		sysclk = stm32_get_PLL1_rate(regs, PLL1_P_CK);
		break;
	case RCC_CFGR_SW_HSE:
		sysclk = stm32_get_rate(regs, HSE);
		break;

	case RCC_CFGR_SW_CSI:
		sysclk = stm32_get_rate(regs, CSI);
		break;

	case RCC_CFGR_SW_HSI:
		sysclk = stm32_get_rate(regs, HSI);
		break;
	}

	/* sysclk = 0 ? no need to go ahead */
	if (!sysclk)
		return sysclk;

	dev_dbg(clk->dev, "system clock: source = %d freq = %ld\n",
		source, sysclk);

	d1cfgr = readl(&regs->d1cfgr);

	if (d1cfgr & RCC_D1CFGR_D1CPRE_DIVIDED) {
		/* get D1 domain Core prescaler */
		idx = (d1cfgr & RCC_D1CFGR_D1CPRE_DIVIDER) >>
		      RCC_D1CFGR_D1CPRE_SHIFT;
		sysclk = sysclk / prescaler_table[idx];
	}

	if (d1cfgr & RCC_D1CFGR_HPRE_DIVIDED) {
		/* get D1 domain AHB prescaler */
		idx = d1cfgr & RCC_D1CFGR_HPRE_DIVIDER;
		sysclk = sysclk / prescaler_table[idx];
	}

	gate_offset = clk_map[clk->id].gate_offset;

	dev_dbg(clk->dev, "clk->id=%ld gate_offset=0x%x sysclk=%ld\n",
		clk->id, gate_offset, sysclk);

	switch (gate_offset) {
	case RCC_AHB3ENR:
	case RCC_AHB1ENR:
	case RCC_AHB2ENR:
	case RCC_AHB4ENR:
		return sysclk;
		break;

	case RCC_APB3ENR:
		if (d1cfgr & RCC_D1CFGR_D1PPRE_DIVIDED) {
			/* get D1 domain APB3 prescaler */
			idx = (d1cfgr & RCC_D1CFGR_D1PPRE_DIVIDER) >>
			      RCC_D1CFGR_D1PPRE_SHIFT;
			sysclk = sysclk / prescaler_table[idx];
		}

		dev_dbg(clk->dev, "system clock: freq after APB3 prescaler = %ld\n",
			sysclk);

		return sysclk;
		break;

	case RCC_APB4ENR:
		d3cfgr = readl(&regs->d3cfgr);
		if (d3cfgr & RCC_D3CFGR_D3PPRE_DIVIDED) {
			/* get D3 domain APB4 prescaler */
			idx = (d3cfgr & RCC_D3CFGR_D3PPRE_DIVIDER) >>
			      RCC_D3CFGR_D3PPRE_SHIFT;
			sysclk = sysclk / prescaler_table[idx];
		}

		dev_dbg(clk->dev,
			"system clock: freq after APB4 prescaler = %ld\n",
			sysclk);

		return sysclk;
		break;

	case RCC_APB1LENR:
	case RCC_APB1HENR:
		/* special case for GPT timers */
		switch (clk->id) {
		case TIM14_CK:
		case TIM13_CK:
		case TIM12_CK:
		case TIM7_CK:
		case TIM6_CK:
		case TIM5_CK:
		case TIM4_CK:
		case TIM3_CK:
		case TIM2_CK:
			return stm32_get_timer_rate(priv, sysclk, APB1);
		}

		dev_dbg(clk->dev,
			"system clock: freq after APB1 prescaler = %ld\n",
			sysclk);

		return (sysclk / stm32_get_apb_psc(regs, APB1));
		break;

	case RCC_APB2ENR:
		/* special case for timers */
		switch (clk->id) {
		case TIM17_CK:
		case TIM16_CK:
		case TIM15_CK:
		case TIM8_CK:
		case TIM1_CK:
			return stm32_get_timer_rate(priv, sysclk, APB2);
		}

		dev_dbg(clk->dev,
			"system clock: freq after APB2 prescaler = %ld\n",
			sysclk);

		return (sysclk / stm32_get_apb_psc(regs, APB2));

		break;

	default:
		dev_err(clk->dev, "unexpected gate_offset value (0x%x)\n",
			gate_offset);
		return -EINVAL;
		break;
	}
}

static int stm32_clk_enable(struct clk *clk)
{
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->rcc_base;
	u32 gate_offset;
	u32 gate_bit_index;
	unsigned long clk_id = clk->id;

	gate_offset = clk_map[clk_id].gate_offset;
	gate_bit_index = clk_map[clk_id].gate_bit_idx;

	dev_dbg(clk->dev, "clkid=%ld gate offset=0x%x bit_index=%d name=%s\n",
		clk->id, gate_offset, gate_bit_index,
		clk_map[clk_id].name);

	setbits_le32(&regs->cr + (gate_offset / 4), BIT(gate_bit_index));

	return 0;
}

static int stm32_clk_probe(struct udevice *dev)
{
	struct stm32_clk *priv = dev_get_priv(dev);
	struct udevice *syscon;
	fdt_addr_t addr;
	int err;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->rcc_base = (struct stm32_rcc_regs *)addr;

	/* get corresponding syscon phandle */
	err = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "st,syscfg", &syscon);

	if (err) {
		dev_err(dev, "unable to find syscon device\n");
		return err;
	}

	priv->pwr_regmap = syscon_get_regmap(syscon);
	if (!priv->pwr_regmap) {
		dev_err(dev, "unable to find regmap\n");
		return -ENODEV;
	}

	configure_clocks(dev);

	return 0;
}

static int stm32_clk_of_xlate(struct clk *clk,
			struct ofnode_phandle_args *args)
{
	if (args->args_count != 1) {
		dev_dbg(clk->dev, "Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count) {
		clk->id = args->args[0];
		/*
		 * this computation convert DT clock index which is used to
		 * point into 2 separate clock arrays (peripheral and kernel
		 * clocks bank) (see include/dt-bindings/clock/stm32h7-clks.h)
		 * into index to point into only one array where peripheral
		 * and kernel clocks are consecutive
		 */
		if (clk->id >= KERN_BANK) {
			clk->id -= KERN_BANK;
			clk->id += LAST_PERIF_BANK - PERIF_BANK + 1;
		} else {
			clk->id -= PERIF_BANK;
		}
	} else {
		clk->id = 0;
	}

	dev_dbg(clk->dev, "clk->id %ld\n", clk->id);

	return 0;
}

static struct clk_ops stm32_clk_ops = {
	.of_xlate	= stm32_clk_of_xlate,
	.enable		= stm32_clk_enable,
	.get_rate	= stm32_clk_get_rate,
};

U_BOOT_DRIVER(stm32h7_clk) = {
	.name			= "stm32h7_rcc_clock",
	.id			= UCLASS_CLK,
	.ops			= &stm32_clk_ops,
	.probe			= stm32_clk_probe,
	.priv_auto	= sizeof(struct stm32_clk),
	.flags			= DM_FLAG_PRE_RELOC,
};
