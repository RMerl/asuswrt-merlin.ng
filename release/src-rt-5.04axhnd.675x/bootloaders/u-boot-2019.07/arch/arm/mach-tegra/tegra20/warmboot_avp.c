// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010 - 2011
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/flow.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/apb_misc.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/warmboot.h>
#include "warmboot_avp.h"

#define DEBUG_RESET_CORESIGHT

void wb_start(void)
{
	struct apb_misc_pp_ctlr *apb_misc =
				(struct apb_misc_pp_ctlr *)NV_PA_APB_MISC_BASE;
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	struct flow_ctlr *flow = (struct flow_ctlr *)NV_PA_FLOW_BASE;
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	union osc_ctrl_reg osc_ctrl;
	union pllx_base_reg pllx_base;
	union pllx_misc_reg pllx_misc;
	union scratch3_reg scratch3;
	u32 reg;

	/* enable JTAG & TBE */
	writel(CONFIG_CTL_TBE | CONFIG_CTL_JTAG, &apb_misc->cfg_ctl);

	/* Are we running where we're supposed to be? */
	asm volatile (
		"adr	%0, wb_start;"	/* reg: wb_start address */
		: "=r"(reg)		/* output */
					/* no input, no clobber list */
	);

	if (reg != NV_WB_RUN_ADDRESS)
		goto do_reset;

	/* Are we running with AVP? */
	if (readl(NV_PA_PG_UP_BASE + PG_UP_TAG_0) != PG_UP_TAG_AVP)
		goto do_reset;

#ifdef DEBUG_RESET_CORESIGHT
	/* Assert CoreSight reset */
	reg = readl(&clkrst->crc_rst_dev[TEGRA_DEV_U]);
	reg |= SWR_CSITE_RST;
	writel(reg, &clkrst->crc_rst_dev[TEGRA_DEV_U]);
#endif

	/* TODO: Set the drive strength - maybe make this a board parameter? */
	osc_ctrl.word = readl(&clkrst->crc_osc_ctrl);
	osc_ctrl.xofs = 4;
	osc_ctrl.xoe = 1;
	writel(osc_ctrl.word, &clkrst->crc_osc_ctrl);

	/* Power up the CPU complex if necessary */
	if (!(readl(&pmc->pmc_pwrgate_status) & PWRGATE_STATUS_CPU)) {
		reg = PWRGATE_TOGGLE_PARTID_CPU | PWRGATE_TOGGLE_START;
		writel(reg, &pmc->pmc_pwrgate_toggle);
		while (!(readl(&pmc->pmc_pwrgate_status) & PWRGATE_STATUS_CPU))
			;
	}

	/* Remove the I/O clamps from the CPU power partition. */
	reg = readl(&pmc->pmc_remove_clamping);
	reg |= CPU_CLMP;
	writel(reg, &pmc->pmc_remove_clamping);

	reg = EVENT_ZERO_VAL_20 | EVENT_MSEC | EVENT_MODE_STOP;
	writel(reg, &flow->halt_cop_events);

	/* Assert CPU complex reset */
	reg = readl(&clkrst->crc_rst_dev[TEGRA_DEV_L]);
	reg |= CPU_RST;
	writel(reg, &clkrst->crc_rst_dev[TEGRA_DEV_L]);

	/* Hold both CPUs in reset */
	reg = CPU_CMPLX_CPURESET0 | CPU_CMPLX_CPURESET1 | CPU_CMPLX_DERESET0 |
	      CPU_CMPLX_DERESET1 | CPU_CMPLX_DBGRESET0 | CPU_CMPLX_DBGRESET1;
	writel(reg, &clkrst->crc_cpu_cmplx_set);

	/* Halt CPU1 at the flow controller for uni-processor configurations */
	writel(EVENT_MODE_STOP, &flow->halt_cpu1_events);

	/*
	 * Set the CPU reset vector. SCRATCH41 contains the physical
	 * address of the CPU-side restoration code.
	 */
	reg = readl(&pmc->pmc_scratch41);
	writel(reg, EXCEP_VECTOR_CPU_RESET_VECTOR);

	/* Select CPU complex clock source */
	writel(CCLK_PLLP_BURST_POLICY, &clkrst->crc_cclk_brst_pol);

	/* Start the CPU0 clock and stop the CPU1 clock */
	reg = CPU_CMPLX_CPU_BRIDGE_CLKDIV_4 | CPU_CMPLX_CPU0_CLK_STP_RUN |
	      CPU_CMPLX_CPU1_CLK_STP_STOP;
	writel(reg, &clkrst->crc_clk_cpu_cmplx);

	/* Enable the CPU complex clock */
	reg = readl(&clkrst->crc_clk_out_enb[TEGRA_DEV_L]);
	reg |= CLK_ENB_CPU;
	writel(reg, &clkrst->crc_clk_out_enb[TEGRA_DEV_L]);

	/* Make sure the resets were held for at least 2 microseconds */
	reg = readl(TIMER_USEC_CNTR);
	while (readl(TIMER_USEC_CNTR) <= (reg + 2))
		;

#ifdef DEBUG_RESET_CORESIGHT
	/*
	 * De-assert CoreSight reset.
	 * NOTE: We're leaving the CoreSight clock on the oscillator for
	 *	now. It will be restored to its original clock source
	 *	when the CPU-side restoration code runs.
	 */
	reg = readl(&clkrst->crc_rst_dev[TEGRA_DEV_U]);
	reg &= ~SWR_CSITE_RST;
	writel(reg, &clkrst->crc_rst_dev[TEGRA_DEV_U]);
#endif

	/* Unlock the CPU CoreSight interfaces */
	reg = 0xC5ACCE55;
	writel(reg, CSITE_CPU_DBG0_LAR);
	writel(reg, CSITE_CPU_DBG1_LAR);

	/*
	 * Sample the microsecond timestamp again. This is the time we must
	 * use when returning from LP0 for PLL stabilization delays.
	 */
	reg = readl(TIMER_USEC_CNTR);
	writel(reg, &pmc->pmc_scratch1);

	pllx_base.word = 0;
	pllx_misc.word = 0;
	scratch3.word = readl(&pmc->pmc_scratch3);

	/* Get the OSC. For 19.2 MHz, use 19 to make the calculations easier */
	reg = (readl(TIMER_USEC_CFG) & USEC_CFG_DIVISOR_MASK) + 1;

	/*
	 * According to the TRM, for 19.2MHz OSC, the USEC_DIVISOR is 0x5f, and
	 * USEC_DIVIDEND is 0x04. So, if USEC_DIVISOR > 26, OSC is 19.2 MHz.
	 *
	 * reg is used to calculate the pllx freq, which is used to determine if
	 * to set dccon or not.
	 */
	if (reg > 26)
		reg = 19;

	/* PLLX_BASE.PLLX_DIVM */
	if (scratch3.pllx_base_divm == reg)
		reg = 0;
	else
		reg = 1;

	/* PLLX_BASE.PLLX_DIVN */
	pllx_base.divn = scratch3.pllx_base_divn;
	reg = scratch3.pllx_base_divn << reg;

	/* PLLX_BASE.PLLX_DIVP */
	pllx_base.divp = scratch3.pllx_base_divp;
	reg = reg >> scratch3.pllx_base_divp;

	pllx_base.bypass = 1;

	/* PLLX_MISC_DCCON must be set for pllx frequency > 600 MHz. */
	if (reg > 600)
		pllx_misc.dccon = 1;

	/* PLLX_MISC_LFCON */
	pllx_misc.lfcon = scratch3.pllx_misc_lfcon;

	/* PLLX_MISC_CPCON */
	pllx_misc.cpcon = scratch3.pllx_misc_cpcon;

	writel(pllx_misc.word, &clkrst->crc_pll_simple[SIMPLE_PLLX].pll_misc);
	writel(pllx_base.word, &clkrst->crc_pll_simple[SIMPLE_PLLX].pll_base);

	pllx_base.enable = 1;
	writel(pllx_base.word, &clkrst->crc_pll_simple[SIMPLE_PLLX].pll_base);
	pllx_base.bypass = 0;
	writel(pllx_base.word, &clkrst->crc_pll_simple[SIMPLE_PLLX].pll_base);

	writel(0, flow->halt_cpu_events);

	reg = CPU_CMPLX_CPURESET0 | CPU_CMPLX_DBGRESET0 | CPU_CMPLX_DERESET0;
	writel(reg, &clkrst->crc_cpu_cmplx_clr);

	reg = PLLM_OUT1_RSTN_RESET_DISABLE | PLLM_OUT1_CLKEN_ENABLE |
	      PLLM_OUT1_RATIO_VAL_8;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_MEMORY].pll_out[0]);

	reg = SCLK_SWAKE_FIQ_SRC_PLLM_OUT1 | SCLK_SWAKE_IRQ_SRC_PLLM_OUT1 |
	      SCLK_SWAKE_RUN_SRC_PLLM_OUT1 | SCLK_SWAKE_IDLE_SRC_PLLM_OUT1 |
	      SCLK_SYS_STATE_IDLE;
	writel(reg, &clkrst->crc_sclk_brst_pol);

	/* avp_resume: no return after the write */
	reg = readl(&clkrst->crc_rst_dev[TEGRA_DEV_L]);
	reg &= ~CPU_RST;
	writel(reg, &clkrst->crc_rst_dev[TEGRA_DEV_L]);

	/* avp_halt: */
avp_halt:
	reg = EVENT_MODE_STOP | EVENT_JTAG;
	writel(reg, flow->halt_cop_events);
	goto avp_halt;

do_reset:
	/*
	 * Execution comes here if something goes wrong. The chip is reset and
	 * a cold boot is performed.
	 */
	writel(SWR_TRIG_SYS_RST, &clkrst->crc_rst_dev[TEGRA_DEV_L]);
	goto do_reset;
}

/*
 * wb_end() is a dummy function, and must be directly following wb_start(),
 * and is used to calculate the size of wb_start().
 */
void wb_end(void)
{
}
