// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ahb.h>
#include <asm/arch/clock.h>
#include <asm/arch/flow.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/ap.h>
#include "../cpu.h"

/* Tegra124-specific CPU init code */

static void enable_cpu_power_rail(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	debug("%s entry\n", __func__);

	/* un-tristate PWR_I2C SCL/SDA, rest of the defaults are correct */
	pinmux_tristate_disable(PMUX_PINGRP_PWR_I2C_SCL_PZ6);
	pinmux_tristate_disable(PMUX_PINGRP_PWR_I2C_SDA_PZ7);

	pmic_enable_cpu_vdd();

	/*
	 * Set CPUPWRGOOD_TIMER - APB clock is 1/2 of SCLK (102MHz),
	 * set it for 5ms as per SysEng (102MHz*5ms = 510000 (7C830h).
	 */
	writel(0x7C830, &pmc->pmc_cpupwrgood_timer);

	/* Set polarity to 0 (normal) and enable CPUPWRREQ_OE */
	clrbits_le32(&pmc->pmc_cntrl, CPUPWRREQ_POL);
	setbits_le32(&pmc->pmc_cntrl, CPUPWRREQ_OE);
}

static void enable_cpu_clocks(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	struct clk_pll_info *pllinfo = &tegra_pll_info_table[CLOCK_ID_XCPU];
	u32 reg;

	debug("%s entry\n", __func__);

	/* Wait for PLL-X to lock */
	do {
		reg = readl(&clkrst->crc_pll_simple[SIMPLE_PLLX].pll_base);
		debug("%s: PLLX base = 0x%08X\n", __func__, reg);
	} while ((reg & (1 << pllinfo->lock_det)) == 0);

	debug("%s: PLLX locked, delay for stable clocks\n", __func__);
	/* Wait until all clocks are stable */
	udelay(PLL_STABILIZATION_DELAY);

	debug("%s: Setting CCLK_BURST and DIVIDER\n", __func__);
	writel(CCLK_BURST_POLICY, &clkrst->crc_cclk_brst_pol);
	writel(SUPER_CCLK_DIVIDER, &clkrst->crc_super_cclk_div);

	debug("%s: Enabling clock to all CPUs\n", __func__);
	/* Enable the clock to all CPUs */
	reg = CLR_CPU3_CLK_STP | CLR_CPU2_CLK_STP | CLR_CPU1_CLK_STP |
		CLR_CPU0_CLK_STP;
	writel(reg, &clkrst->crc_clk_cpu_cmplx_clr);

	debug("%s: Enabling main CPU complex clocks\n", __func__);
	/* Always enable the main CPU complex clocks */
	clock_enable(PERIPH_ID_CPU);
	clock_enable(PERIPH_ID_CPULP);
	clock_enable(PERIPH_ID_CPUG);

	debug("%s: Done\n", __func__);
}

static void remove_cpu_resets(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	debug("%s entry\n", __func__);

	/* Take the slow and fast partitions out of reset */
	reg = CLR_NONCPURESET;
	writel(reg, &clkrst->crc_rst_cpulp_cmplx_clr);
	writel(reg, &clkrst->crc_rst_cpug_cmplx_clr);

	/* Clear the SW-controlled reset of the slow cluster */
	reg = CLR_CPURESET0 | CLR_DBGRESET0 | CLR_CORERESET0 | CLR_CXRESET0 |
		CLR_L2RESET | CLR_PRESETDBG;
	writel(reg, &clkrst->crc_rst_cpulp_cmplx_clr);

	/* Clear the SW-controlled reset of the fast cluster */
	reg = CLR_CPURESET0 | CLR_DBGRESET0 | CLR_CORERESET0 | CLR_CXRESET0 |
		CLR_CPURESET1 | CLR_DBGRESET1 | CLR_CORERESET1 | CLR_CXRESET1 |
		CLR_CPURESET2 | CLR_DBGRESET2 | CLR_CORERESET2 | CLR_CXRESET2 |
		CLR_CPURESET3 | CLR_DBGRESET3 | CLR_CORERESET3 | CLR_CXRESET3 |
		CLR_L2RESET | CLR_PRESETDBG;
	writel(reg, &clkrst->crc_rst_cpug_cmplx_clr);
}

static void tegra124_ram_repair(void)
{
	struct flow_ctlr *flow = (struct flow_ctlr *)NV_PA_FLOW_BASE;
	u32 ram_repair_timeout; /*usec*/
	u32 val;

	/*
	 * Request the Flow Controller perform RAM repair whenever it turns on
	 * a power rail that requires RAM repair.
	 */
	clrbits_le32(&flow->ram_repair, RAM_REPAIR_BYPASS_EN);

	/* Request SW trigerred RAM repair by setting req  bit */
	/* cluster 0 */
	setbits_le32(&flow->ram_repair, RAM_REPAIR_REQ);
	/* Wait for completion (status == 0) */
	ram_repair_timeout = 500;
	do {
		udelay(1);
		val = readl(&flow->ram_repair);
	} while (!(val & RAM_REPAIR_STS) && ram_repair_timeout--);
	if (!ram_repair_timeout)
		debug("Ram Repair cluster0 failed\n");

	/* cluster 1 */
	setbits_le32(&flow->ram_repair_cluster1, RAM_REPAIR_REQ);
	/* Wait for completion (status == 0) */
	ram_repair_timeout = 500;
	do {
		udelay(1);
		val = readl(&flow->ram_repair_cluster1);
	} while (!(val & RAM_REPAIR_STS) && ram_repair_timeout--);

	if (!ram_repair_timeout)
		debug("Ram Repair cluster1 failed\n");
}

/**
 * Tegra124 requires some special clock initialization, including setting up
 * the DVC I2C, turning on MSELECT and selecting the G CPU cluster
 */
void tegra124_init_clocks(void)
{
	struct flow_ctlr *flow = (struct flow_ctlr *)NV_PA_FLOW_BASE;
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 val;

	debug("%s entry\n", __func__);

	/* Set active CPU cluster to G */
	clrbits_le32(&flow->cluster_control, 1);

	/* Change the oscillator drive strength */
	val = readl(&clkrst->crc_osc_ctrl);
	val &= ~OSC_XOFS_MASK;
	val |= (OSC_DRIVE_STRENGTH << OSC_XOFS_SHIFT);
	writel(val, &clkrst->crc_osc_ctrl);

	/* Update same value in PMC_OSC_EDPD_OVER XOFS field for warmboot */
	val = readl(&pmc->pmc_osc_edpd_over);
	val &= ~PMC_XOFS_MASK;
	val |= (OSC_DRIVE_STRENGTH << PMC_XOFS_SHIFT);
	writel(val, &pmc->pmc_osc_edpd_over);

	/* Set HOLD_CKE_LOW_EN to 1 */
	setbits_le32(&pmc->pmc_cntrl2, HOLD_CKE_LOW_EN);

	debug("Setting up PLLX\n");
	init_pllx();

	val = (1 << CLK_SYS_RATE_AHB_RATE_SHIFT);
	writel(val, &clkrst->crc_clk_sys_rate);

	/* Enable clocks to required peripherals. TBD - minimize this list */
	debug("Enabling clocks\n");

	clock_set_enable(PERIPH_ID_CACHE2, 1);
	clock_set_enable(PERIPH_ID_GPIO, 1);
	clock_set_enable(PERIPH_ID_TMR, 1);
	clock_set_enable(PERIPH_ID_CPU, 1);
	clock_set_enable(PERIPH_ID_EMC, 1);
	clock_set_enable(PERIPH_ID_I2C5, 1);
	clock_set_enable(PERIPH_ID_APBDMA, 1);
	clock_set_enable(PERIPH_ID_MEM, 1);
	clock_set_enable(PERIPH_ID_CORESIGHT, 1);
	clock_set_enable(PERIPH_ID_MSELECT, 1);
	clock_set_enable(PERIPH_ID_DVFS, 1);

	/*
	 * Set MSELECT clock source as PLLP (00), and ask for a clock
	 * divider that would set the MSELECT clock at 102MHz for a
	 * PLLP base of 408MHz.
	 */
	clock_ll_set_source_divisor(PERIPH_ID_MSELECT, 0,
				    CLK_DIVIDER(NVBL_PLLP_KHZ, 102000));

	/* Give clock time to stabilize */
	udelay(IO_STABILIZATION_DELAY);

	/* I2C5 (DVC) gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(PERIPH_ID_I2C5, 3, 16);

	/* Give clock time to stabilize */
	udelay(IO_STABILIZATION_DELAY);

	/* Take required peripherals out of reset */
	debug("Taking periphs out of reset\n");
	reset_set_enable(PERIPH_ID_CACHE2, 0);
	reset_set_enable(PERIPH_ID_GPIO, 0);
	reset_set_enable(PERIPH_ID_TMR, 0);
	reset_set_enable(PERIPH_ID_COP, 0);
	reset_set_enable(PERIPH_ID_EMC, 0);
	reset_set_enable(PERIPH_ID_I2C5, 0);
	reset_set_enable(PERIPH_ID_APBDMA, 0);
	reset_set_enable(PERIPH_ID_MEM, 0);
	reset_set_enable(PERIPH_ID_CORESIGHT, 0);
	reset_set_enable(PERIPH_ID_MSELECT, 0);
	reset_set_enable(PERIPH_ID_DVFS, 0);

	debug("%s exit\n", __func__);
}

static bool is_partition_powered(u32 partid)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 reg;

	/* Get power gate status */
	reg = readl(&pmc->pmc_pwrgate_status);
	return !!(reg & (1 << partid));
}

static void power_partition(u32 partid)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	debug("%s: part ID = %08X\n", __func__, partid);
	/* Is the partition already on? */
	if (!is_partition_powered(partid)) {
		/* No, toggle the partition power state (OFF -> ON) */
		debug("power_partition, toggling state\n");
		writel(START_CP | partid, &pmc->pmc_pwrgate_toggle);

		/* Wait for the power to come up */
		while (!is_partition_powered(partid))
			;

		/* Give I/O signals time to stabilize */
		udelay(IO_STABILIZATION_DELAY);
	}
}

void powerup_cpus(void)
{
	/* We boot to the fast cluster */
	debug("%s entry: G cluster\n", __func__);

	/* Power up the fast cluster rail partition */
	debug("%s: CRAIL\n", __func__);
	power_partition(CRAIL);

	/* Power up the fast cluster non-CPU partition */
	debug("%s: C0NC\n", __func__);
	power_partition(C0NC);

	/* Power up the fast cluster CPU0 partition */
	debug("%s: CE0\n", __func__);
	power_partition(CE0);

	debug("%s: done\n", __func__);
}

void start_cpu(u32 reset_vector)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	debug("%s entry, reset_vector = %x\n", __func__, reset_vector);

	tegra124_init_clocks();

	/* Set power-gating timer multiplier */
	writel((MULT_8 << TIMER_MULT_SHIFT) | (MULT_8 << TIMER_MULT_CPU_SHIFT),
	       &pmc->pmc_pwrgate_timer_mult);

	enable_cpu_power_rail();
	powerup_cpus();
	tegra124_ram_repair();
	enable_cpu_clocks();
	clock_enable_coresight(1);
	writel(reset_vector, EXCEP_VECTOR_CPU_RESET_VECTOR);
	remove_cpu_resets();
	debug("%s exit, should continue @ reset_vector\n", __func__);
}
