/*
 * clock_ti816x.c
 *
 * Clocks for TI816X based boards
 *
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Antoine Tenart, <atenart@adeneo-embedded.com>
 *
 * Based on TI-PSP-04.00.02.14 :
 *
 * Copyright (C) 2009, Texas Instruments, Incorporated
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

#include <asm/emif.h>

#define CM_PLL_BASE		(CTRL_BASE + 0x0400)

/* Main PLL */
#define MAIN_N			64
#define MAIN_P			0x1
#define MAIN_INTFREQ1		0x8
#define MAIN_FRACFREQ1		0x800000
#define MAIN_MDIV1		0x2
#define MAIN_INTFREQ2		0xE
#define MAIN_FRACFREQ2		0x0
#define MAIN_MDIV2		0x1
#define MAIN_INTFREQ3		0x8
#define MAIN_FRACFREQ3		0xAAAAB0
#define MAIN_MDIV3		0x3
#define MAIN_INTFREQ4		0x9
#define MAIN_FRACFREQ4		0x55554F
#define MAIN_MDIV4		0x3
#define MAIN_INTFREQ5		0x9
#define MAIN_FRACFREQ5		0x374BC6
#define MAIN_MDIV5		0xC
#define MAIN_MDIV6		0x48
#define MAIN_MDIV7		0x4

/* DDR PLL */
#define DDR_N			59
#define DDR_P			0x1
#define DDR_MDIV1		0x2
#define DDR_INTFREQ2		0x8
#define DDR_FRACFREQ2		0xD99999
#define DDR_MDIV2		0x1E
#define DDR_INTFREQ3		0x8
#define DDR_FRACFREQ3		0x0
#define DDR_MDIV3		0x4
#define DDR_INTFREQ4		0xE /* Expansion DDR clk */
#define DDR_FRACFREQ4		0x0
#define DDR_MDIV4		0x4
#define DDR_INTFREQ5		0xE /* Expansion DDR clk */
#define DDR_FRACFREQ5		0x0
#define DDR_MDIV5		0x4

#define CONTROL_STATUS			(CTRL_BASE + 0x40)
#define DDR_RCD				(CTRL_BASE + 0x070C)
#define CM_TIMER1_CLKSEL		(PRCM_BASE + 0x390)
#define CM_ALWON_CUST_EFUSE_CLKCTRL	(PRCM_BASE + 0x1628)

#define INTCPS_SYSCONFIG	0x48200010
#define CM_SYSCLK10_CLKSEL	0x48180324

struct cm_pll {
	unsigned int mainpll_ctrl;	/* offset 0x400 */
	unsigned int mainpll_pwd;
	unsigned int mainpll_freq1;
	unsigned int mainpll_div1;
	unsigned int mainpll_freq2;
	unsigned int mainpll_div2;
	unsigned int mainpll_freq3;
	unsigned int mainpll_div3;
	unsigned int mainpll_freq4;
	unsigned int mainpll_div4;
	unsigned int mainpll_freq5;
	unsigned int mainpll_div5;
	unsigned int resv0[1];
	unsigned int mainpll_div6;
	unsigned int resv1[1];
	unsigned int mainpll_div7;
	unsigned int ddrpll_ctrl;	/* offset 0x440 */
	unsigned int ddrpll_pwd;
	unsigned int resv2[1];
	unsigned int ddrpll_div1;
	unsigned int ddrpll_freq2;
	unsigned int ddrpll_div2;
	unsigned int ddrpll_freq3;
	unsigned int ddrpll_div3;
	unsigned int ddrpll_freq4;
	unsigned int ddrpll_div4;
	unsigned int ddrpll_freq5;
	unsigned int ddrpll_div5;
	unsigned int videopll_ctrl;	/* offset 0x470 */
	unsigned int videopll_pwd;
	unsigned int videopll_freq1;
	unsigned int videopll_div1;
	unsigned int videopll_freq2;
	unsigned int videopll_div2;
	unsigned int videopll_freq3;
	unsigned int videopll_div3;
	unsigned int resv3[4];
	unsigned int audiopll_ctrl;	/* offset 0x4A0 */
	unsigned int audiopll_pwd;
	unsigned int resv4[2];
	unsigned int audiopll_freq2;
	unsigned int audiopll_div2;
	unsigned int audiopll_freq3;
	unsigned int audiopll_div3;
	unsigned int audiopll_freq4;
	unsigned int audiopll_div4;
	unsigned int audiopll_freq5;
	unsigned int audiopll_div5;
};

const struct cm_alwon *cmalwon = (struct cm_alwon *)CM_ALWON_BASE;
const struct cm_def *cmdef = (struct cm_def *)CM_DEFAULT_BASE;
const struct cm_pll *cmpll = (struct cm_pll *)CM_PLL_BASE;
const struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

void enable_dmm_clocks(void)
{
	writel(PRCM_MOD_EN, &cmdef->dmmclkctrl);
	/* Wait for dmm to be fully functional, including OCP */
	while (((readl(&cmdef->dmmclkctrl) >> 17) & 0x3) != 0)
		;
}

void enable_emif_clocks(void)
{
	writel(PRCM_MOD_EN, &cmdef->fwclkctrl);
	writel(PRCM_MOD_EN, &cmdef->l3fastclkstctrl);
	writel(PRCM_MOD_EN, &cmdef->emif0clkctrl);
	writel(PRCM_MOD_EN, &cmdef->emif1clkctrl);

	/* Wait for clocks to be active */
	while ((readl(&cmdef->l3fastclkstctrl) & 0x300) != 0x300)
		;
	/* Wait for emif0 to be fully functional, including OCP */
	while (((readl(&cmdef->emif0clkctrl) >> 17) & 0x3) != 0)
		;
	/* Wait for emif1 to be fully functional, including OCP */
	while (((readl(&cmdef->emif1clkctrl) >> 17) & 0x3) != 0)
		;
}

/* assume delay is aprox at least 1us */
static void ddr_delay(int d)
{
	int i;

	/*
	 * read a control register.
	 * this is a bit more delay and cannot be optimized by the compiler
	 * assuming one read takes 200 cycles and A8 is runing 1 GHz
	 * somewhat conservative setting
	 */
	for (i = 0; i < 50*d; i++)
		readl(CONTROL_STATUS);
}

static void main_pll_init_ti816x(void)
{
	u32 main_pll_ctrl = 0;

	/* Put the PLL in bypass mode by setting BIT2 in its ctrl reg */
	main_pll_ctrl = readl(&cmpll->mainpll_ctrl);
	main_pll_ctrl &= 0xFFFFFFFB;
	main_pll_ctrl |= BIT(2);
	writel(main_pll_ctrl, &cmpll->mainpll_ctrl);

	/* Enable PLL by setting BIT3 in its ctrl reg */
	main_pll_ctrl = readl(&cmpll->mainpll_ctrl);
	main_pll_ctrl &= 0xFFFFFFF7;
	main_pll_ctrl |= BIT(3);
	writel(main_pll_ctrl, &cmpll->mainpll_ctrl);

	/* Write the values of N,P in the CTRL reg  */
	main_pll_ctrl = readl(&cmpll->mainpll_ctrl);
	main_pll_ctrl &= 0xFF;
	main_pll_ctrl |= (MAIN_N<<16 | MAIN_P<<8);
	writel(main_pll_ctrl, &cmpll->mainpll_ctrl);

	/* Power up clock1-7 */
	writel(0x0, &cmpll->mainpll_pwd);

	/* Program the freq and divider values for clock1-7 */
	writel((1<<31 | 1<<28 | (MAIN_INTFREQ1<<24) | MAIN_FRACFREQ1),
		&cmpll->mainpll_freq1);
	writel(((1<<8) | MAIN_MDIV1), &cmpll->mainpll_div1);

	writel((1<<31 | 1<<28 | (MAIN_INTFREQ2<<24) | MAIN_FRACFREQ2),
		&cmpll->mainpll_freq2);
	writel(((1<<8) | MAIN_MDIV2), &cmpll->mainpll_div2);

	writel((1<<31 | 1<<28 | (MAIN_INTFREQ3<<24) | MAIN_FRACFREQ3),
		&cmpll->mainpll_freq3);
	writel(((1<<8) | MAIN_MDIV3), &cmpll->mainpll_div3);

	writel((1<<31 | 1<<28 | (MAIN_INTFREQ4<<24) | MAIN_FRACFREQ4),
		&cmpll->mainpll_freq4);
	writel(((1<<8) | MAIN_MDIV4), &cmpll->mainpll_div4);

	writel((1<<31 | 1<<28 | (MAIN_INTFREQ5<<24) | MAIN_FRACFREQ5),
		&cmpll->mainpll_freq5);
	writel(((1<<8) | MAIN_MDIV5), &cmpll->mainpll_div5);

	writel((1<<8 | MAIN_MDIV6), &cmpll->mainpll_div6);

	writel((1<<8 | MAIN_MDIV7), &cmpll->mainpll_div7);

	/* Wait for PLL to lock */
	while ((readl(&cmpll->mainpll_ctrl) & BIT(7)) != BIT(7))
		;

	/* Put the PLL in normal mode, disable bypass */
	main_pll_ctrl = readl(&cmpll->mainpll_ctrl);
	main_pll_ctrl &= 0xFFFFFFFB;
	writel(main_pll_ctrl, &cmpll->mainpll_ctrl);
}

static void ddr_pll_bypass_ti816x(void)
{
	u32 ddr_pll_ctrl = 0;

	/* Put the PLL in bypass mode by setting BIT2 in its ctrl reg */
	ddr_pll_ctrl = readl(&cmpll->ddrpll_ctrl);
	ddr_pll_ctrl &= 0xFFFFFFFB;
	ddr_pll_ctrl |= BIT(2);
	writel(ddr_pll_ctrl, &cmpll->ddrpll_ctrl);
}

static void ddr_pll_init_ti816x(void)
{
	u32 ddr_pll_ctrl = 0;
	/* Enable PLL by setting BIT3 in its ctrl reg */
	ddr_pll_ctrl = readl(&cmpll->ddrpll_ctrl);
	ddr_pll_ctrl &= 0xFFFFFFF7;
	ddr_pll_ctrl |= BIT(3);
	writel(ddr_pll_ctrl, &cmpll->ddrpll_ctrl);

	/* Write the values of N,P in the CTRL reg  */
	ddr_pll_ctrl = readl(&cmpll->ddrpll_ctrl);
	ddr_pll_ctrl &= 0xFF;
	ddr_pll_ctrl |= (DDR_N<<16 | DDR_P<<8);
	writel(ddr_pll_ctrl, &cmpll->ddrpll_ctrl);

	ddr_delay(10);

	/* Power up clock1-5 */
	writel(0x0, &cmpll->ddrpll_pwd);

	/* Program the freq and divider values for clock1-3 */
	writel(((0<<8) | DDR_MDIV1), &cmpll->ddrpll_div1);
	ddr_delay(1);
	writel(((1<<8) | DDR_MDIV1), &cmpll->ddrpll_div1);
	writel((1<<31 | 1<<28 | (DDR_INTFREQ2<<24) | DDR_FRACFREQ2),
		&cmpll->ddrpll_freq2);
	writel(((1<<8) | DDR_MDIV2), &cmpll->ddrpll_div2);
	writel(((0<<8) | DDR_MDIV3), &cmpll->ddrpll_div3);
	ddr_delay(1);
	writel(((1<<8) | DDR_MDIV3), &cmpll->ddrpll_div3);
	ddr_delay(1);
	writel((0<<31 | 1<<28 | (DDR_INTFREQ3<<24) | DDR_FRACFREQ3),
		&cmpll->ddrpll_freq3);
	ddr_delay(1);
	writel((1<<31 | 1<<28 | (DDR_INTFREQ3<<24) | DDR_FRACFREQ3),
		&cmpll->ddrpll_freq3);

	ddr_delay(5);

	/* Wait for PLL to lock */
	while ((readl(&cmpll->ddrpll_ctrl) & BIT(7)) != BIT(7))
		;

	/* Power up RCD */
	writel(BIT(0), DDR_RCD);
}

static void peripheral_enable(void)
{
	/* Wake-up the l3_slow clock */
	writel(PRCM_MOD_EN, &cmalwon->l3slowclkstctrl);

	/*
	 * Note on Timers:
	 * There are 8 timers(0-7) out of which timer 0 is a secure timer.
	 * Timer 0 mux should not be changed
	 *
	 * To access the timer registers we need the to be
	 * enabled which is what we do in the first step
	 */

	/* Enable timer1 */
	writel(PRCM_MOD_EN, &cmalwon->timer1clkctrl);
	/* Select timer1 clock to be CLKIN (27MHz) */
	writel(BIT(1), CM_TIMER1_CLKSEL);

	/* Wait for timer1 to be ON-ACTIVE */
	while (((readl(&cmalwon->l3slowclkstctrl)
					& (0x80000<<1))>>20) != 1)
		;
	/* Wait for timer1 to be enabled */
	while (((readl(&cmalwon->timer1clkctrl) & 0x30000)>>16) != 0)
		;
	/* Active posted mode */
	writel(PRCM_MOD_EN, (DM_TIMER1_BASE + 0x54));
	while (readl(DM_TIMER1_BASE + 0x10) & BIT(0))
		;
	/* Start timer1  */
	writel(BIT(0), (DM_TIMER1_BASE + 0x38));

	/* eFuse */
	writel(PRCM_MOD_EN, CM_ALWON_CUST_EFUSE_CLKCTRL);
	while (readl(CM_ALWON_CUST_EFUSE_CLKCTRL) != PRCM_MOD_EN)
		;

	/* Enable gpio0 */
	writel(PRCM_MOD_EN, &cmalwon->gpio0clkctrl);
	while (readl(&cmalwon->gpio0clkctrl) != PRCM_MOD_EN)
		;
	writel((BIT(1) | BIT(8)), &cmalwon->gpio0clkctrl);

	/* Enable gpio1 */
	writel(PRCM_MOD_EN, &cmalwon->gpio1clkctrl);
	while (readl(&cmalwon->gpio1clkctrl) != PRCM_MOD_EN)
		;
	writel((BIT(1) | BIT(8)), &cmalwon->gpio1clkctrl);

	/* Enable spi */
	writel(PRCM_MOD_EN, &cmalwon->spiclkctrl);
	while (readl(&cmalwon->spiclkctrl) != PRCM_MOD_EN)
		;

	/* Enable i2c0 */
	writel(PRCM_MOD_EN, &cmalwon->i2c0clkctrl);
	while (readl(&cmalwon->i2c0clkctrl) != PRCM_MOD_EN)
		;

	/* Enable ethernet0 */
	writel(PRCM_MOD_EN, &cmalwon->ethclkstctrl);
	writel(PRCM_MOD_EN, &cmalwon->ethernet0clkctrl);
	writel(PRCM_MOD_EN, &cmalwon->ethernet1clkctrl);

	/* Enable hsmmc */
	writel(PRCM_MOD_EN, &cmalwon->sdioclkctrl);
	while (readl(&cmalwon->sdioclkctrl) != PRCM_MOD_EN)
		;
}

void setup_clocks_for_console(void)
{
	/* Fix ROM code bug - from TI-PSP-04.00.02.14 */
	writel(0x0, CM_SYSCLK10_CLKSEL);

	ddr_pll_bypass_ti816x();

	/* Enable uart0-2 */
	writel(PRCM_MOD_EN, &cmalwon->uart0clkctrl);
	while (readl(&cmalwon->uart0clkctrl) != PRCM_MOD_EN)
		;
	writel(PRCM_MOD_EN, &cmalwon->uart1clkctrl);
	while (readl(&cmalwon->uart1clkctrl) != PRCM_MOD_EN)
		;
	writel(PRCM_MOD_EN, &cmalwon->uart2clkctrl);
	while (readl(&cmalwon->uart2clkctrl) != PRCM_MOD_EN)
		;
	while ((readl(&cmalwon->l3slowclkstctrl) & 0x2100) != 0x2100)
		;
}

void setup_early_clocks(void)
{
	setup_clocks_for_console();
}

void prcm_init(void)
{
	/* Enable the control */
	writel(PRCM_MOD_EN, &cmalwon->controlclkctrl);

	main_pll_init_ti816x();
	ddr_pll_init_ti816x();

	/*
	 * With clk freqs setup to desired values,
	 * enable the required peripherals
	 */
	peripheral_enable();
}
