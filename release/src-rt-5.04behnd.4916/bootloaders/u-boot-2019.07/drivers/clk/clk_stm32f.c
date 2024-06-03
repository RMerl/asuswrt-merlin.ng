// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <stm32_rcc.h>

#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/stm32_pwr.h>

#include <dt-bindings/mfd/stm32f7-rcc.h>

#define RCC_CR_HSION			BIT(0)
#define RCC_CR_HSEON			BIT(16)
#define RCC_CR_HSERDY			BIT(17)
#define RCC_CR_HSEBYP			BIT(18)
#define RCC_CR_CSSON			BIT(19)
#define RCC_CR_PLLON			BIT(24)
#define RCC_CR_PLLRDY			BIT(25)
#define RCC_CR_PLLSAION			BIT(28)
#define RCC_CR_PLLSAIRDY		BIT(29)

#define RCC_PLLCFGR_PLLM_MASK		GENMASK(5, 0)
#define RCC_PLLCFGR_PLLN_MASK		GENMASK(14, 6)
#define RCC_PLLCFGR_PLLP_MASK		GENMASK(17, 16)
#define RCC_PLLCFGR_PLLQ_MASK		GENMASK(27, 24)
#define RCC_PLLCFGR_PLLSRC		BIT(22)
#define RCC_PLLCFGR_PLLM_SHIFT		0
#define RCC_PLLCFGR_PLLN_SHIFT		6
#define RCC_PLLCFGR_PLLP_SHIFT		16
#define RCC_PLLCFGR_PLLQ_SHIFT		24

#define RCC_CFGR_AHB_PSC_MASK		GENMASK(7, 4)
#define RCC_CFGR_APB1_PSC_MASK		GENMASK(12, 10)
#define RCC_CFGR_APB2_PSC_MASK		GENMASK(15, 13)
#define RCC_CFGR_SW0			BIT(0)
#define RCC_CFGR_SW1			BIT(1)
#define RCC_CFGR_SW_MASK		GENMASK(1, 0)
#define RCC_CFGR_SW_HSI			0
#define RCC_CFGR_SW_HSE			RCC_CFGR_SW0
#define RCC_CFGR_SW_PLL			RCC_CFGR_SW1
#define RCC_CFGR_SWS0			BIT(2)
#define RCC_CFGR_SWS1			BIT(3)
#define RCC_CFGR_SWS_MASK		GENMASK(3, 2)
#define RCC_CFGR_SWS_HSI		0
#define RCC_CFGR_SWS_HSE		RCC_CFGR_SWS0
#define RCC_CFGR_SWS_PLL		RCC_CFGR_SWS1
#define RCC_CFGR_HPRE_SHIFT		4
#define RCC_CFGR_PPRE1_SHIFT		10
#define RCC_CFGR_PPRE2_SHIFT		13

#define RCC_PLLSAICFGR_PLLSAIN_MASK	GENMASK(14, 6)
#define RCC_PLLSAICFGR_PLLSAIP_MASK	GENMASK(17, 16)
#define RCC_PLLSAICFGR_PLLSAIQ_MASK	GENMASK(27, 24)
#define RCC_PLLSAICFGR_PLLSAIR_MASK	GENMASK(30, 28)
#define RCC_PLLSAICFGR_PLLSAIN_SHIFT	6
#define RCC_PLLSAICFGR_PLLSAIP_SHIFT	16
#define RCC_PLLSAICFGR_PLLSAIQ_SHIFT	24
#define RCC_PLLSAICFGR_PLLSAIR_SHIFT	28
#define RCC_PLLSAICFGR_PLLSAIP_4	BIT(16)
#define RCC_PLLSAICFGR_PLLSAIQ_4	BIT(26)
#define RCC_PLLSAICFGR_PLLSAIR_3	BIT(29) | BIT(28)

#define RCC_DCKCFGRX_TIMPRE		BIT(24)
#define RCC_DCKCFGRX_CK48MSEL		BIT(27)
#define RCC_DCKCFGRX_SDMMC1SEL		BIT(28)
#define RCC_DCKCFGR2_SDMMC2SEL		BIT(29)

#define RCC_DCKCFGR_PLLSAIDIVR_SHIFT    16
#define RCC_DCKCFGR_PLLSAIDIVR_MASK	GENMASK(17, 16)
#define RCC_DCKCFGR_PLLSAIDIVR_2	0

/*
 * RCC AHB1ENR specific definitions
 */
#define RCC_AHB1ENR_ETHMAC_EN		BIT(25)
#define RCC_AHB1ENR_ETHMAC_TX_EN	BIT(26)
#define RCC_AHB1ENR_ETHMAC_RX_EN	BIT(27)

/*
 * RCC APB1ENR specific definitions
 */
#define RCC_APB1ENR_TIM2EN		BIT(0)
#define RCC_APB1ENR_PWREN		BIT(28)

/*
 * RCC APB2ENR specific definitions
 */
#define RCC_APB2ENR_SYSCFGEN		BIT(14)
#define RCC_APB2ENR_SAI1EN		BIT(22)

enum pllsai_div {
	PLLSAIP,
	PLLSAIQ,
	PLLSAIR,
};

static const struct stm32_clk_info stm32f4_clk_info = {
	/* 180 MHz */
	.sys_pll_psc = {
		.pll_n = 360,
		.pll_p = 2,
		.pll_q = 8,
		.ahb_psc = AHB_PSC_1,
		.apb1_psc = APB_PSC_4,
		.apb2_psc = APB_PSC_2,
	},
	.has_overdrive = false,
	.v2 = false,
};

static const struct stm32_clk_info stm32f7_clk_info = {
	/* 200 MHz */
	.sys_pll_psc = {
		.pll_n = 400,
		.pll_p = 2,
		.pll_q = 8,
		.ahb_psc = AHB_PSC_1,
		.apb1_psc = APB_PSC_4,
		.apb2_psc = APB_PSC_2,
	},
	.has_overdrive = true,
	.v2 = true,
};

struct stm32_clk {
	struct stm32_rcc_regs *base;
	struct stm32_pwr_regs *pwr_regs;
	struct stm32_clk_info info;
	unsigned long hse_rate;
	bool pllsaip;
};

#ifdef CONFIG_VIDEO_STM32
static const u8 plldivr_table[] = { 0, 0, 2, 3, 4, 5, 6, 7 };
#endif
static const u8 pllsaidivr_table[] = { 2, 4, 8, 16 };

static int configure_clocks(struct udevice *dev)
{
	struct stm32_clk *priv = dev_get_priv(dev);
	struct stm32_rcc_regs *regs = priv->base;
	struct stm32_pwr_regs *pwr = priv->pwr_regs;
	struct pll_psc *sys_pll_psc = &priv->info.sys_pll_psc;

	/* Reset RCC configuration */
	setbits_le32(&regs->cr, RCC_CR_HSION);
	writel(0, &regs->cfgr); /* Reset CFGR */
	clrbits_le32(&regs->cr, (RCC_CR_HSEON | RCC_CR_CSSON
		| RCC_CR_PLLON | RCC_CR_PLLSAION));
	writel(0x24003010, &regs->pllcfgr); /* Reset value from RM */
	clrbits_le32(&regs->cr, RCC_CR_HSEBYP);
	writel(0, &regs->cir); /* Disable all interrupts */

	/* Configure for HSE+PLL operation */
	setbits_le32(&regs->cr, RCC_CR_HSEON);
	while (!(readl(&regs->cr) & RCC_CR_HSERDY))
		;

	setbits_le32(&regs->cfgr, ((
		sys_pll_psc->ahb_psc << RCC_CFGR_HPRE_SHIFT)
		| (sys_pll_psc->apb1_psc << RCC_CFGR_PPRE1_SHIFT)
		| (sys_pll_psc->apb2_psc << RCC_CFGR_PPRE2_SHIFT)));

	/* Configure the main PLL */
	setbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLSRC); /* pll source HSE */
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLM_MASK,
			sys_pll_psc->pll_m << RCC_PLLCFGR_PLLM_SHIFT);
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLN_MASK,
			sys_pll_psc->pll_n << RCC_PLLCFGR_PLLN_SHIFT);
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLP_MASK,
			((sys_pll_psc->pll_p >> 1) - 1) << RCC_PLLCFGR_PLLP_SHIFT);
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLQ_MASK,
			sys_pll_psc->pll_q << RCC_PLLCFGR_PLLQ_SHIFT);

	/* configure SDMMC clock */
	if (priv->info.v2) { /*stm32f7 case */
		if (priv->pllsaip)
			/* select PLLSAIP as 48MHz clock source */
			setbits_le32(&regs->dckcfgr2, RCC_DCKCFGRX_CK48MSEL);
		else
			/* select PLLQ as 48MHz clock source */
			clrbits_le32(&regs->dckcfgr2, RCC_DCKCFGRX_CK48MSEL);

		/* select 48MHz as SDMMC1 clock source */
		clrbits_le32(&regs->dckcfgr2, RCC_DCKCFGRX_SDMMC1SEL);

		/* select 48MHz as SDMMC2 clock source */
		clrbits_le32(&regs->dckcfgr2, RCC_DCKCFGR2_SDMMC2SEL);
	} else  { /* stm32f4 case */
		if (priv->pllsaip)
			/* select PLLSAIP as 48MHz clock source */
			setbits_le32(&regs->dckcfgr, RCC_DCKCFGRX_CK48MSEL);
		else
			/* select PLLQ as 48MHz clock source */
			clrbits_le32(&regs->dckcfgr, RCC_DCKCFGRX_CK48MSEL);

		/* select 48MHz as SDMMC1 clock source */
		clrbits_le32(&regs->dckcfgr, RCC_DCKCFGRX_SDMMC1SEL);
	}

	/*
	 * Configure the SAI PLL to generate LTDC pixel clock and
	 * 48 Mhz for SDMMC and USB
	 */
	clrsetbits_le32(&regs->pllsaicfgr, RCC_PLLSAICFGR_PLLSAIP_MASK,
			RCC_PLLSAICFGR_PLLSAIP_4);
	clrsetbits_le32(&regs->pllsaicfgr, RCC_PLLSAICFGR_PLLSAIR_MASK,
			RCC_PLLSAICFGR_PLLSAIR_3);
	clrsetbits_le32(&regs->pllsaicfgr, RCC_PLLSAICFGR_PLLSAIN_MASK,
			195 << RCC_PLLSAICFGR_PLLSAIN_SHIFT);

	clrsetbits_le32(&regs->dckcfgr, RCC_DCKCFGR_PLLSAIDIVR_MASK,
			RCC_DCKCFGR_PLLSAIDIVR_2 << RCC_DCKCFGR_PLLSAIDIVR_SHIFT);

	/* Enable the main PLL */
	setbits_le32(&regs->cr, RCC_CR_PLLON);
	while (!(readl(&regs->cr) & RCC_CR_PLLRDY))
		;

	/* Enable the SAI PLL */
	setbits_le32(&regs->cr, RCC_CR_PLLSAION);
	while (!(readl(&regs->cr) & RCC_CR_PLLSAIRDY))
		;
	setbits_le32(&regs->apb1enr, RCC_APB1ENR_PWREN);

	if (priv->info.has_overdrive) {
		/*
		 * Enable high performance mode
		 * System frequency up to 200 MHz
		 */
		setbits_le32(&pwr->cr1, PWR_CR1_ODEN);
		/* Infinite wait! */
		while (!(readl(&pwr->csr1) & PWR_CSR1_ODRDY))
			;
		/* Enable the Over-drive switch */
		setbits_le32(&pwr->cr1, PWR_CR1_ODSWEN);
		/* Infinite wait! */
		while (!(readl(&pwr->csr1) & PWR_CSR1_ODSWRDY))
			;
	}

	stm32_flash_latency_cfg(5);
	clrbits_le32(&regs->cfgr, (RCC_CFGR_SW0 | RCC_CFGR_SW1));
	setbits_le32(&regs->cfgr, RCC_CFGR_SW_PLL);

	while ((readl(&regs->cfgr) & RCC_CFGR_SWS_MASK) !=
			RCC_CFGR_SWS_PLL)
		;

#ifdef CONFIG_ETH_DESIGNWARE
	/* gate the SYSCFG clock, needed to set RMII ethernet interface */
	setbits_le32(&regs->apb2enr, RCC_APB2ENR_SYSCFGEN);
#endif

	return 0;
}

static bool stm32_clk_get_ck48msel(struct stm32_clk *priv)
{
	struct stm32_rcc_regs *regs = priv->base;

	if (priv->info.v2) /*stm32f7 case */
		return readl(&regs->dckcfgr2) & RCC_DCKCFGRX_CK48MSEL;
	else

		return readl(&regs->dckcfgr) & RCC_DCKCFGRX_CK48MSEL;
}

static unsigned long stm32_clk_get_pllsai_vco_rate(struct stm32_clk *priv)
{
	struct stm32_rcc_regs *regs = priv->base;
	u16 pllm, pllsain;

	pllm = (readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLM_MASK);
	pllsain = ((readl(&regs->pllsaicfgr) & RCC_PLLSAICFGR_PLLSAIN_MASK)
		  >> RCC_PLLSAICFGR_PLLSAIN_SHIFT);

	return ((priv->hse_rate / pllm) * pllsain);
}

static unsigned long stm32_clk_get_pllsai_rate(struct stm32_clk *priv,
					       enum pllsai_div output)
{
	struct stm32_rcc_regs *regs = priv->base;
	u16 pll_div_output;

	switch (output) {
	case PLLSAIP:
		pll_div_output = ((((readl(&regs->pllsaicfgr)
				  & RCC_PLLSAICFGR_PLLSAIP_MASK)
				  >> RCC_PLLSAICFGR_PLLSAIP_SHIFT) + 1) << 1);
		break;
	case PLLSAIQ:
		pll_div_output = (readl(&regs->pllsaicfgr)
				  & RCC_PLLSAICFGR_PLLSAIQ_MASK)
				  >> RCC_PLLSAICFGR_PLLSAIQ_SHIFT;
		break;
	case PLLSAIR:
		pll_div_output = (readl(&regs->pllsaicfgr)
				  & RCC_PLLSAICFGR_PLLSAIR_MASK)
				  >> RCC_PLLSAICFGR_PLLSAIR_SHIFT;
		break;
	default:
		pr_err("incorrect PLLSAI output %d\n", output);
		return -EINVAL;
	}

	return (stm32_clk_get_pllsai_vco_rate(priv) / pll_div_output);
}

static bool stm32_get_timpre(struct stm32_clk *priv)
{
	struct stm32_rcc_regs *regs = priv->base;
	u32 val;

	if (priv->info.v2) /*stm32f7 case */
		val = readl(&regs->dckcfgr2);
	else
		val = readl(&regs->dckcfgr);
	/* get timer prescaler */
	return !!(val & RCC_DCKCFGRX_TIMPRE);
}

static u32 stm32_get_hclk_rate(struct stm32_rcc_regs *regs, u32 sysclk)
{
	u8 shift;
	/* Prescaler table lookups for clock computation */
	u8 ahb_psc_table[16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
	};

	shift = ahb_psc_table[(
		(readl(&regs->cfgr) & RCC_CFGR_AHB_PSC_MASK)
		>> RCC_CFGR_HPRE_SHIFT)];

	return sysclk >> shift;
};

static u8 stm32_get_apb_shift(struct stm32_rcc_regs *regs, enum apb apb)
{
	/* Prescaler table lookups for clock computation */
	u8 apb_psc_table[8] = {
		0, 0, 0, 0, 1, 2, 3, 4
	};

	if (apb == APB1)
		return apb_psc_table[(
		       (readl(&regs->cfgr) & RCC_CFGR_APB1_PSC_MASK)
		       >> RCC_CFGR_PPRE1_SHIFT)];
	else /* APB2 */
		return apb_psc_table[(
		       (readl(&regs->cfgr) & RCC_CFGR_APB2_PSC_MASK)
		       >> RCC_CFGR_PPRE2_SHIFT)];
};

static u32 stm32_get_timer_rate(struct stm32_clk *priv, u32 sysclk,
				enum apb apb)
{
	struct stm32_rcc_regs *regs = priv->base;
	u8 shift = stm32_get_apb_shift(regs, apb);

	if (stm32_get_timpre(priv))
		/*
		 * if APB prescaler is configured to a
		 * division factor of 1, 2 or 4
		 */
		switch (shift) {
		case 0:
		case 1:
		case 2:
			return stm32_get_hclk_rate(regs, sysclk);
		default:
			return (sysclk >> shift) * 4;
		}
	else
		/*
		 * if APB prescaler is configured to a
		 * division factor of 1
		 */
		if (shift == 0)
			return sysclk;
		else
			return (sysclk >> shift) * 2;
};

static ulong stm32_clk_get_rate(struct clk *clk)
{
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->base;
	u32 sysclk = 0;
	u32 vco;
	u32 sdmmcxsel_bit;
	u32 saidivr;
	u32 pllsai_rate;
	u16 pllm, plln, pllp, pllq;

	if ((readl(&regs->cfgr) & RCC_CFGR_SWS_MASK) ==
			RCC_CFGR_SWS_PLL) {
		pllm = (readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLM_MASK);
		plln = ((readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLN_MASK)
			>> RCC_PLLCFGR_PLLN_SHIFT);
		pllp = ((((readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLP_MASK)
			>> RCC_PLLCFGR_PLLP_SHIFT) + 1) << 1);
		pllq = ((readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLQ_MASK)
			>> RCC_PLLCFGR_PLLQ_SHIFT);
		vco = (priv->hse_rate / pllm) * plln;
		sysclk = vco / pllp;
	} else {
		return -EINVAL;
	}

	switch (clk->id) {
	/*
	 * AHB CLOCK: 3 x 32 bits consecutive registers are used :
	 * AHB1, AHB2 and AHB3
	 */
	case STM32F7_AHB1_CLOCK(GPIOA) ... STM32F7_AHB3_CLOCK(QSPI):
		return stm32_get_hclk_rate(regs, sysclk);
	/* APB1 CLOCK */
	case STM32F7_APB1_CLOCK(TIM2) ... STM32F7_APB1_CLOCK(UART8):
		/* For timer clock, an additionnal prescaler is used*/
		switch (clk->id) {
		case STM32F7_APB1_CLOCK(TIM2):
		case STM32F7_APB1_CLOCK(TIM3):
		case STM32F7_APB1_CLOCK(TIM4):
		case STM32F7_APB1_CLOCK(TIM5):
		case STM32F7_APB1_CLOCK(TIM6):
		case STM32F7_APB1_CLOCK(TIM7):
		case STM32F7_APB1_CLOCK(TIM12):
		case STM32F7_APB1_CLOCK(TIM13):
		case STM32F7_APB1_CLOCK(TIM14):
			return stm32_get_timer_rate(priv, sysclk, APB1);
		}
		return (sysclk >> stm32_get_apb_shift(regs, APB1));

	/* APB2 CLOCK */
	case STM32F7_APB2_CLOCK(TIM1) ... STM32F7_APB2_CLOCK(DSI):
		switch (clk->id) {
		/*
		 * particular case for SDMMC1 and SDMMC2 :
		 * 48Mhz source clock can be from main PLL or from
		 * PLLSAIP
		 */
		case STM32F7_APB2_CLOCK(SDMMC1):
		case STM32F7_APB2_CLOCK(SDMMC2):
			if (clk->id == STM32F7_APB2_CLOCK(SDMMC1))
				sdmmcxsel_bit = RCC_DCKCFGRX_SDMMC1SEL;
			else
				sdmmcxsel_bit = RCC_DCKCFGR2_SDMMC2SEL;

			if (readl(&regs->dckcfgr2) & sdmmcxsel_bit)
				/* System clock is selected as SDMMC1 clock */
				return sysclk;
			/*
			 * 48 MHz can be generated by either PLLSAIP
			 * or by PLLQ depending of CK48MSEL bit of RCC_DCKCFGR
			 */
			if (stm32_clk_get_ck48msel(priv))
				return stm32_clk_get_pllsai_rate(priv, PLLSAIP);
			else
				return (vco / pllq);
			break;

		/* For timer clock, an additionnal prescaler is used*/
		case STM32F7_APB2_CLOCK(TIM1):
		case STM32F7_APB2_CLOCK(TIM8):
		case STM32F7_APB2_CLOCK(TIM9):
		case STM32F7_APB2_CLOCK(TIM10):
		case STM32F7_APB2_CLOCK(TIM11):
			return stm32_get_timer_rate(priv, sysclk, APB2);
		break;

		/* particular case for LTDC clock */
		case STM32F7_APB2_CLOCK(LTDC):
			saidivr = readl(&regs->dckcfgr);
			saidivr = (saidivr & RCC_DCKCFGR_PLLSAIDIVR_MASK)
				  >> RCC_DCKCFGR_PLLSAIDIVR_SHIFT;
			pllsai_rate = stm32_clk_get_pllsai_rate(priv, PLLSAIR);

			return pllsai_rate / pllsaidivr_table[saidivr];
		}
		return (sysclk >> stm32_get_apb_shift(regs, APB2));

	default:
		pr_err("clock index %ld out of range\n", clk->id);
		return -EINVAL;
	}
}

static ulong stm32_set_rate(struct clk *clk, ulong rate)
{
#ifdef CONFIG_VIDEO_STM32
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->base;
	u32 pllsair_rate, pllsai_vco_rate, current_rate;
	u32 best_div, best_diff, diff;
	u16 div;
	u8 best_plldivr, best_pllsaidivr;
	u8 i, j;
	bool found = false;

	/* Only set_rate for LTDC clock is implemented */
	if (clk->id != STM32F7_APB2_CLOCK(LTDC)) {
		pr_err("set_rate not implemented for clock index %ld\n",
		       clk->id);
		return 0;
	}

	if (rate == stm32_clk_get_rate(clk))
		/* already set to requested rate */
		return rate;

	/* get the current PLLSAIR output freq */
	pllsair_rate = stm32_clk_get_pllsai_rate(priv, PLLSAIR);
	best_div = pllsair_rate / rate;

	/* look into pllsaidivr_table if this divider is available*/
	for (i = 0 ; i < sizeof(pllsaidivr_table); i++)
		if (best_div == pllsaidivr_table[i]) {
			/* set pll_saidivr with found value */
			clrsetbits_le32(&regs->dckcfgr,
					RCC_DCKCFGR_PLLSAIDIVR_MASK,
					pllsaidivr_table[i]);
			return rate;
		}

	/*
	 * As no pllsaidivr value is suitable to obtain requested freq,
	 * test all combination of pllsaidivr * pllsair and find the one
	 * which give freq closest to requested rate.
	 */

	pllsai_vco_rate = stm32_clk_get_pllsai_vco_rate(priv);
	best_diff = ULONG_MAX;
	best_pllsaidivr = 0;
	best_plldivr = 0;
	/*
	 * start at index 2 of plldivr_table as divider value at index 0
	 * and 1 are 0)
	 */
	for (i = 2; i < sizeof(plldivr_table); i++) {
		for (j = 0; j < sizeof(pllsaidivr_table); j++) {
			div = plldivr_table[i] * pllsaidivr_table[j];
			current_rate = pllsai_vco_rate / div;
			/* perfect combination is found ? */
			if (current_rate == rate) {
				best_pllsaidivr = j;
				best_plldivr = i;
				found = true;
				break;
			}

			diff = (current_rate > rate) ?
			       current_rate - rate : rate - current_rate;

			/* found a better combination ? */
			if (diff < best_diff) {
				best_diff = diff;
				best_pllsaidivr = j;
				best_plldivr = i;
			}
		}

		if (found)
			break;
	}

	/* Disable the SAI PLL */
	clrbits_le32(&regs->cr, RCC_CR_PLLSAION);

	/* set pll_saidivr with found value */
	clrsetbits_le32(&regs->dckcfgr, RCC_DCKCFGR_PLLSAIDIVR_MASK,
			best_pllsaidivr << RCC_DCKCFGR_PLLSAIDIVR_SHIFT);

	/* set pllsair with found value */
	clrsetbits_le32(&regs->pllsaicfgr, RCC_PLLSAICFGR_PLLSAIR_MASK,
			plldivr_table[best_plldivr]
			<< RCC_PLLSAICFGR_PLLSAIR_SHIFT);

	/* Enable the SAI PLL */
	setbits_le32(&regs->cr, RCC_CR_PLLSAION);
	while (!(readl(&regs->cr) & RCC_CR_PLLSAIRDY))
		;

	div = plldivr_table[best_plldivr] * pllsaidivr_table[best_pllsaidivr];
	return pllsai_vco_rate / div;
#else
	return 0;
#endif
}

static int stm32_clk_enable(struct clk *clk)
{
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->base;
	u32 offset = clk->id / 32;
	u32 bit_index = clk->id % 32;

	debug("%s: clkid = %ld, offset from AHB1ENR is %d, bit_index = %d\n",
	      __func__, clk->id, offset, bit_index);
	setbits_le32(&regs->ahb1enr + offset, BIT(bit_index));

	return 0;
}

static int stm32_clk_probe(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	struct udevice *fixed_clock_dev = NULL;
	struct clk clk;
	int err;

	debug("%s\n", __func__);

	struct stm32_clk *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct stm32_rcc_regs *)addr;
	priv->pllsaip = true;

	switch (dev_get_driver_data(dev)) {
	case STM32F42X:
		priv->pllsaip = false;
		/* fallback into STM32F469 case */
	case STM32F469:
		memcpy(&priv->info, &stm32f4_clk_info,
		       sizeof(struct stm32_clk_info));
		break;

	case STM32F7:
		memcpy(&priv->info, &stm32f7_clk_info,
		       sizeof(struct stm32_clk_info));
		break;
	default:
		return -EINVAL;
	}

	/* retrieve HSE frequency (external oscillator) */
	err = uclass_get_device_by_name(UCLASS_CLK, "clk-hse",
					&fixed_clock_dev);

	if (err) {
		pr_err("Can't find fixed clock (%d)", err);
		return err;
	}

	err = clk_request(fixed_clock_dev, &clk);
	if (err) {
		pr_err("Can't request %s clk (%d)", fixed_clock_dev->name,
		       err);
		return err;
	}

	/*
	 * set pllm factor accordingly to the external oscillator
	 * frequency (HSE). For STM32F4 and STM32F7, we want VCO
	 * freq at 1MHz
	 * if input PLL frequency is 25Mhz, divide it by 25
	 */
	clk.id = 0;
	priv->hse_rate = clk_get_rate(&clk);

	if (priv->hse_rate < 1000000) {
		pr_err("%s: unexpected HSE clock rate = %ld \"n", __func__,
		       priv->hse_rate);
		return -EINVAL;
	}

	priv->info.sys_pll_psc.pll_m = priv->hse_rate / 1000000;

	if (priv->info.has_overdrive) {
		err = dev_read_phandle_with_args(dev, "st,syscfg", NULL, 0, 0,
						 &args);
		if (err) {
			debug("%s: can't find syscon device (%d)\n", __func__,
			      err);
			return err;
		}

		priv->pwr_regs = (struct stm32_pwr_regs *)ofnode_get_addr(args.node);
	}

	configure_clocks(dev);

	return 0;
}

static int stm32_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	debug("%s(clk=%p)\n", __func__, clk);

	if (args->args_count != 2) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = args->args[1];
	else
		clk->id = 0;

	return 0;
}

static struct clk_ops stm32_clk_ops = {
	.of_xlate	= stm32_clk_of_xlate,
	.enable		= stm32_clk_enable,
	.get_rate	= stm32_clk_get_rate,
	.set_rate	= stm32_set_rate,
};

U_BOOT_DRIVER(stm32fx_clk) = {
	.name			= "stm32fx_rcc_clock",
	.id			= UCLASS_CLK,
	.ops			= &stm32_clk_ops,
	.probe			= stm32_clk_probe,
	.priv_auto_alloc_size	= sizeof(struct stm32_clk),
	.flags			= DM_FLAG_PRE_RELOC,
};
