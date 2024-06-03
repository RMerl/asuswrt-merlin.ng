/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) STMicroelectronics SA 2017
 * Author(s): Patrice CHOTARD, <patrice.chotard@st.com> for STMicroelectronics.
 */

#ifndef __STM32_RCC_H_
#define __STM32_RCC_H_

#define AHB_PSC_1			0
#define AHB_PSC_2			0x8
#define AHB_PSC_4			0x9
#define AHB_PSC_8			0xA
#define AHB_PSC_16			0xB
#define AHB_PSC_64			0xC
#define AHB_PSC_128			0xD
#define AHB_PSC_256			0xE
#define AHB_PSC_512			0xF

#define APB_PSC_1			0
#define APB_PSC_2			0x4
#define APB_PSC_4			0x5
#define APB_PSC_8			0x6
#define APB_PSC_16			0x7

struct pll_psc {
	u8	pll_m;
	u16	pll_n;
	u8	pll_p;
	u8	pll_q;
	u8	ahb_psc;
	u8	apb1_psc;
	u8	apb2_psc;
};

struct stm32_clk_info {
	struct pll_psc sys_pll_psc;
	bool has_overdrive;
	bool v2;
};

enum soc_family {
	STM32F42X,
	STM32F469,
	STM32F7,
	STM32MP1,
};

enum apb {
	APB1,
	APB2,
};

struct stm32_rcc_clk {
	char *drv_name;
	enum soc_family soc;
};

struct stm32_rcc_regs {
	u32 cr;		/* RCC clock control */
	u32 pllcfgr;	/* RCC PLL configuration */
	u32 cfgr;	/* RCC clock configuration */
	u32 cir;	/* RCC clock interrupt */
	u32 ahb1rstr;	/* RCC AHB1 peripheral reset */
	u32 ahb2rstr;	/* RCC AHB2 peripheral reset */
	u32 ahb3rstr;	/* RCC AHB3 peripheral reset */
	u32 rsv0;
	u32 apb1rstr;	/* RCC APB1 peripheral reset */
	u32 apb2rstr;	/* RCC APB2 peripheral reset */
	u32 rsv1[2];
	u32 ahb1enr;	/* RCC AHB1 peripheral clock enable */
	u32 ahb2enr;	/* RCC AHB2 peripheral clock enable */
	u32 ahb3enr;	/* RCC AHB3 peripheral clock enable */
	u32 rsv2;
	u32 apb1enr;	/* RCC APB1 peripheral clock enable */
	u32 apb2enr;	/* RCC APB2 peripheral clock enable */
	u32 rsv3[2];
	u32 ahb1lpenr;	/* RCC AHB1 periph clk enable in low pwr mode */
	u32 ahb2lpenr;	/* RCC AHB2 periph clk enable in low pwr mode */
	u32 ahb3lpenr;	/* RCC AHB3 periph clk enable in low pwr mode */
	u32 rsv4;
	u32 apb1lpenr;	/* RCC APB1 periph clk enable in low pwr mode */
	u32 apb2lpenr;	/* RCC APB2 periph clk enable in low pwr mode */
	u32 rsv5[2];
	u32 bdcr;	/* RCC Backup domain control */
	u32 csr;	/* RCC clock control & status */
	u32 rsv6[2];
	u32 sscgr;	/* RCC spread spectrum clock generation */
	u32 plli2scfgr;	/* RCC PLLI2S configuration */
	/* below registers are only available on STM32F46x and STM32F7 SoCs*/
	u32 pllsaicfgr;	/* PLLSAI configuration */
	u32 dckcfgr;	/* dedicated clocks configuration register */
	/* Below registers are only available on STM32F7 SoCs */
	u32 dckcfgr2;	/* dedicated clocks configuration register */
};

#endif /* __STM32_RCC_H_ */
