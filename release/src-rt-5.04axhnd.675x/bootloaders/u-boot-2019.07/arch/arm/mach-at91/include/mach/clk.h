/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 */
#ifndef __ASM_ARM_ARCH_CLK_H__
#define __ASM_ARM_ARCH_CLK_H__

#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/global_data.h>

#define GCK_CSS_SLOW_CLK	0
#define GCK_CSS_MAIN_CLK	1
#define GCK_CSS_PLLA_CLK	2
#define GCK_CSS_UPLL_CLK	3
#define GCK_CSS_MCK_CLK		4
#define GCK_CSS_AUDIO_CLK	5

#define AT91_UTMI_PLL_CLK_FREQ	480000000

static inline unsigned long get_cpu_clk_rate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	return gd->arch.cpu_clk_rate_hz;
}

static inline unsigned long get_main_clk_rate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	return gd->arch.main_clk_rate_hz;
}

static inline unsigned long get_mck_clk_rate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	return gd->arch.mck_rate_hz;
}

static inline unsigned long get_plla_clk_rate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	return gd->arch.plla_rate_hz;
}

static inline unsigned long get_pllb_clk_rate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	return gd->arch.pllb_rate_hz;
}

static inline u32 get_pllb_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	return gd->arch.at91_pllb_usb_init;
}

#ifdef CPU_HAS_H32MXDIV
static inline unsigned int get_h32mxdiv(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	return readl(&pmc->mckr) & AT91_PMC_MCKR_H32MXDIV;
}
#else
static inline unsigned int get_h32mxdiv(void)
{
	return 0;
}
#endif

static inline unsigned long get_macb_pclk_rate(unsigned int dev_id)
{
	if (get_h32mxdiv())
		return get_mck_clk_rate() / 2;
	else
		return get_mck_clk_rate();
}

static inline unsigned long get_usart_clk_rate(unsigned int dev_id)
{
	if (get_h32mxdiv())
		return get_mck_clk_rate() / 2;
	else
		return get_mck_clk_rate();
}

static inline unsigned long get_lcdc_clk_rate(unsigned int dev_id)
{
	return get_mck_clk_rate();
}

static inline unsigned long get_spi_clk_rate(unsigned int dev_id)
{
	if (get_h32mxdiv())
		return get_mck_clk_rate() / 2;
	else
		return get_mck_clk_rate();
}

static inline unsigned long get_twi_clk_rate(unsigned int dev_id)
{
	if (get_h32mxdiv())
		return get_mck_clk_rate() / 2;
	else
		return get_mck_clk_rate();
}

static inline unsigned long get_mci_clk_rate(void)
{
	if (get_h32mxdiv())
		return get_mck_clk_rate() / 2;
	else
		return get_mck_clk_rate();
}

static inline unsigned long get_pit_clk_rate(void)
{
	if (get_h32mxdiv())
		return get_mck_clk_rate() / 2;
	else
		return get_mck_clk_rate();
}

int at91_clock_init(unsigned long main_clock);
void at91_periph_clk_enable(int id);
void at91_periph_clk_disable(int id);
int at91_enable_periph_generated_clk(u32 id, u32 clk_source, u32 div);
u32 at91_get_periph_generated_clk(u32 id);
void at91_system_clk_enable(int sys_clk);
void at91_system_clk_disable(int sys_clk);
int at91_upll_clk_enable(void);
int at91_upll_clk_disable(void);
void at91_usb_clk_init(u32 value);
int at91_pllb_clk_enable(u32 pllbr);
int at91_pllb_clk_disable(void);
void at91_pllicpr_init(u32 icpr);

#endif /* __ASM_ARM_ARCH_CLK_H__ */
