// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/iproc-common/armpll.h>
#include <asm/iproc-common/sysmap.h>

#define NELEMS(x)	(sizeof(x) / sizeof(x[0]))

struct armpll_parameters {
	unsigned int mode;
	unsigned int ndiv_int;
	unsigned int ndiv_frac;
	unsigned int pdiv;
	unsigned int freqid;
};

struct armpll_parameters armpll_clk_tab[] = {
	{   25, 64,      1, 1, 0},
	{  100, 64,      1, 1, 2},
	{  400, 64,      1, 1, 6},
	{  448, 71, 713050, 1, 6},
	{  500, 80,      1, 1, 6},
	{  560, 89, 629145, 1, 6},
	{  600, 96,      1, 1, 6},
	{  800, 64,      1, 1, 7},
	{  896, 71, 713050, 1, 7},
	{ 1000, 80,      1, 1, 7},
	{ 1100, 88,      1, 1, 7},
	{ 1120, 89, 629145, 1, 7},
	{ 1200, 96,      1, 1, 7},
};

uint32_t armpll_config(uint32_t clkmhz)
{
	uint32_t freqid;
	uint32_t ndiv_frac;
	uint32_t pll;
	uint32_t status = 1;
	uint32_t timeout_countdown;
	int i;

	for (i = 0; i < NELEMS(armpll_clk_tab); i++) {
		if (armpll_clk_tab[i].mode == clkmhz) {
			status = 0;
			break;
		}
	}

	if (status) {
		printf("Error: Clock configuration not supported\n");
		goto armpll_config_done;
	}

	/* Enable write access */
	writel(IPROC_REG_WRITE_ACCESS, IHOST_PROC_CLK_WR_ACCESS);

	if (clkmhz == 25)
		freqid = 0;
	else
		freqid = 2;

	/* Bypass ARM clock and run on sysclk */
	writel(1 << IHOST_PROC_CLK_POLICY_FREQ__PRIV_ACCESS_MODE |
	       freqid << IHOST_PROC_CLK_POLICY_FREQ__POLICY3_FREQ_R |
	       freqid << IHOST_PROC_CLK_POLICY_FREQ__POLICY2_FREQ_R |
	       freqid << IHOST_PROC_CLK_POLICY_FREQ__POLICY1_FREQ_R |
	       freqid << IHOST_PROC_CLK_POLICY_FREQ__POLICY0_FREQ_R,
	       IHOST_PROC_CLK_POLICY_FREQ);

	writel(1 << IHOST_PROC_CLK_POLICY_CTL__GO |
	       1 << IHOST_PROC_CLK_POLICY_CTL__GO_AC,
	       IHOST_PROC_CLK_POLICY_CTL);

	/* Poll CCU until operation complete */
	timeout_countdown = 0x100000;
	while (readl(IHOST_PROC_CLK_POLICY_CTL) &
	       (1 << IHOST_PROC_CLK_POLICY_CTL__GO)) {
		timeout_countdown--;
		if (timeout_countdown == 0) {
			printf("CCU polling timedout\n");
			status = 1;
			goto armpll_config_done;
		}
	}

	if (clkmhz == 25 || clkmhz == 100) {
		status = 0;
		goto armpll_config_done;
	}

	/* Now it is safe to program the PLL */
	pll = readl(IHOST_PROC_CLK_PLLARMB);
	pll &= ~((1 << IHOST_PROC_CLK_PLLARMB__PLLARM_NDIV_FRAC_WIDTH) - 1);
	ndiv_frac =
		((1 << IHOST_PROC_CLK_PLLARMB__PLLARM_NDIV_FRAC_WIDTH) - 1) &
		 (armpll_clk_tab[i].ndiv_frac <<
		 IHOST_PROC_CLK_PLLARMB__PLLARM_NDIV_FRAC_R);
	pll |= ndiv_frac;
	writel(pll, IHOST_PROC_CLK_PLLARMB);

	writel(1 << IHOST_PROC_CLK_PLLARMA__PLLARM_LOCK |
	       armpll_clk_tab[i].ndiv_int <<
			IHOST_PROC_CLK_PLLARMA__PLLARM_NDIV_INT_R |
	       armpll_clk_tab[i].pdiv <<
			IHOST_PROC_CLK_PLLARMA__PLLARM_PDIV_R |
	       1 << IHOST_PROC_CLK_PLLARMA__PLLARM_SOFT_RESETB,
	       IHOST_PROC_CLK_PLLARMA);

	/* Poll ARM PLL Lock until operation complete */
	timeout_countdown = 0x100000;
	while (readl(IHOST_PROC_CLK_PLLARMA) &
	       (1 << IHOST_PROC_CLK_PLLARMA__PLLARM_LOCK)) {
		timeout_countdown--;
		if (timeout_countdown == 0) {
			printf("ARM PLL lock failed\n");
			status = 1;
			goto armpll_config_done;
		}
	}

	pll = readl(IHOST_PROC_CLK_PLLARMA);
	pll |= (1 << IHOST_PROC_CLK_PLLARMA__PLLARM_SOFT_POST_RESETB);
	writel(pll, IHOST_PROC_CLK_PLLARMA);

	/* Set the policy */
	writel(1 << IHOST_PROC_CLK_POLICY_FREQ__PRIV_ACCESS_MODE |
	       armpll_clk_tab[i].freqid <<
			IHOST_PROC_CLK_POLICY_FREQ__POLICY3_FREQ_R |
	       armpll_clk_tab[i].freqid <<
			IHOST_PROC_CLK_POLICY_FREQ__POLICY2_FREQ_R |
	       armpll_clk_tab[i].freqid <<
			IHOST_PROC_CLK_POLICY_FREQ__POLICY1_FREQ_R |
	       armpll_clk_tab[i+4].freqid <<
			IHOST_PROC_CLK_POLICY_FREQ__POLICY0_FREQ_R,
	       IHOST_PROC_CLK_POLICY_FREQ);

	writel(IPROC_CLKCT_HDELAY_SW_EN, IHOST_PROC_CLK_CORE0_CLKGATE);
	writel(IPROC_CLKCT_HDELAY_SW_EN, IHOST_PROC_CLK_CORE1_CLKGATE);
	writel(IPROC_CLKCT_HDELAY_SW_EN, IHOST_PROC_CLK_ARM_SWITCH_CLKGATE);
	writel(IPROC_CLKCT_HDELAY_SW_EN, IHOST_PROC_CLK_ARM_PERIPH_CLKGATE);
	writel(IPROC_CLKCT_HDELAY_SW_EN, IHOST_PROC_CLK_APB0_CLKGATE);

	writel(1 << IHOST_PROC_CLK_POLICY_CTL__GO |
	       1 << IHOST_PROC_CLK_POLICY_CTL__GO_AC,
	       IHOST_PROC_CLK_POLICY_CTL);

	/* Poll CCU until operation complete */
	timeout_countdown = 0x100000;
	while (readl(IHOST_PROC_CLK_POLICY_CTL) &
	       (1 << IHOST_PROC_CLK_POLICY_CTL__GO)) {
		timeout_countdown--;
		if (timeout_countdown == 0) {
			printf("CCU polling failed\n");
			status = 1;
			goto armpll_config_done;
		}
	}

	status = 0;
armpll_config_done:
	/* Disable access to PLL registers */
	writel(0, IHOST_PROC_CLK_WR_ACCESS);

	return status;
}
