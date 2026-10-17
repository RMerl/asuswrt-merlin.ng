// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2017 Intel Corporation
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

struct bridge_cfg {
	int compat_id;
	u32  mask_noc;
	u32  mask_rstmgr;
};

static const struct bridge_cfg bridge_cfg_tbl[] = {
	{
		COMPAT_ALTERA_SOCFPGA_H2F_BRG,
		ALT_SYSMGR_NOC_H2F_SET_MSK,
		ALT_RSTMGR_BRGMODRST_H2F_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_LWH2F_BRG,
		ALT_SYSMGR_NOC_LWH2F_SET_MSK,
		ALT_RSTMGR_BRGMODRST_LWH2F_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2H_BRG,
		ALT_SYSMGR_NOC_F2H_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2H_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2SDR0,
		ALT_SYSMGR_NOC_F2SDR0_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2SSDRAM0_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2SDR1,
		ALT_SYSMGR_NOC_F2SDR1_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2SSDRAM1_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2SDR2,
		ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2SSDRAM2_SET_MSK,
	},
};

/* Disable the watchdog (toggle reset to watchdog) */
void socfpga_watchdog_disable(void)
{
	/* assert reset for watchdog */
	setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_A10_PER1MODRST,
		     ALT_RSTMGR_PER1MODRST_WD0_SET_MSK);
}

/* Release NOC ddr scheduler from reset */
void socfpga_reset_deassert_noc_ddr_scheduler(void)
{
	clrbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_A10_BRGMODRST,
		     ALT_RSTMGR_BRGMODRST_DDRSCH_SET_MSK);
}

static int get_bridge_init_val(const void *blob, int compat_id)
{
	int node;

	node = fdtdec_next_compatible(blob, 0, compat_id);
	if (node < 0)
		return 0;

	return fdtdec_get_uint(blob, node, "init-val", 0);
}

/* Enable bridges (hps2fpga, lwhps2fpga, fpga2hps, fpga2sdram) per handoff */
int socfpga_reset_deassert_bridges_handoff(void)
{
	u32 mask_noc = 0, mask_rstmgr = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(bridge_cfg_tbl); i++) {
		if (get_bridge_init_val(gd->fdt_blob,
					bridge_cfg_tbl[i].compat_id)) {
			mask_noc |= bridge_cfg_tbl[i].mask_noc;
			mask_rstmgr |= bridge_cfg_tbl[i].mask_rstmgr;
		}
	}

	/* clear idle request to all bridges */
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_A10_NOC_IDLEREQ_CLR,
		     mask_noc);

	/* Release bridges from reset state per handoff value */
	clrbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_A10_BRGMODRST,
		     mask_rstmgr);

	/* Poll until all idleack to 0, timeout at 1000ms */
	return wait_for_bit_le32((const void *)(socfpga_get_sysmgr_addr() +
				 SYSMGR_A10_NOC_IDLEACK),
				 mask_noc, false, 1000, false);
}

/* Release L4 OSC1 Watchdog Timer 0 from reset through reset manager */
void socfpga_reset_deassert_osc1wd0(void)
{
	clrbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_A10_PER1MODRST,
		     ALT_RSTMGR_PER1MODRST_WD0_SET_MSK);
}

/*
 * Assert or de-assert SoCFPGA reset manager reset.
 */
void socfpga_per_reset(u32 reset, int set)
{
	unsigned long reg;
	u32 rstmgr_bank = RSTMGR_BANK(reset);

	switch (rstmgr_bank) {
	case 0:
		reg = RSTMGR_A10_MPUMODRST;
		break;
	case 1:
		reg = RSTMGR_A10_PER0MODRST;
		break;
	case 2:
		reg = RSTMGR_A10_PER1MODRST;
		break;
	case 3:
		reg = RSTMGR_A10_BRGMODRST;
		break;
	case 4:
		reg = RSTMGR_A10_SYSMODRST;
		break;

	default:
		return;
	}

	if (set)
		setbits_le32(socfpga_get_rstmgr_addr() + reg,
			     1 << RSTMGR_RESET(reset));
	else
		clrbits_le32(socfpga_get_rstmgr_addr() + reg,
			     1 << RSTMGR_RESET(reset));
}

/*
 * Assert reset on every peripheral but L4WD0.
 * Watchdog must be kept intact to prevent glitches
 * and/or hangs.
 * For the Arria10, we disable all the peripherals except L4 watchdog0,
 * L4 Timer 0, and ECC.
 */
void socfpga_per_reset_all(void)
{
	const u32 l4wd0 = (1 << RSTMGR_RESET(SOCFPGA_RESET(L4WD0)) |
			  (1 << RSTMGR_RESET(SOCFPGA_RESET(L4SYSTIMER0))));
	unsigned mask_ecc_ocp =
		ALT_RSTMGR_PER0MODRST_EMACECC0_SET_MSK |
		ALT_RSTMGR_PER0MODRST_EMACECC1_SET_MSK |
		ALT_RSTMGR_PER0MODRST_EMACECC2_SET_MSK |
		ALT_RSTMGR_PER0MODRST_USBECC0_SET_MSK |
		ALT_RSTMGR_PER0MODRST_USBECC1_SET_MSK |
		ALT_RSTMGR_PER0MODRST_NANDECC_SET_MSK |
		ALT_RSTMGR_PER0MODRST_QSPIECC_SET_MSK |
		ALT_RSTMGR_PER0MODRST_SDMMCECC_SET_MSK;

	/* disable all components except ECC_OCP, L4 Timer0 and L4 WD0 */
	writel(~l4wd0, socfpga_get_rstmgr_addr() + RSTMGR_A10_PER1MODRST);
	setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_A10_PER0MODRST,
		     ~mask_ecc_ocp);

	/* Finally disable the ECC_OCP */
	setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_A10_PER0MODRST,
		     mask_ecc_ocp);
}

int socfpga_bridges_reset(void)
{
	int ret;

	/* Disable all the bridges (hps2fpga, lwhps2fpga, fpga2hps,
	   fpga2sdram) */
	/* set idle request to all bridges */
	writel(ALT_SYSMGR_NOC_H2F_SET_MSK |
		ALT_SYSMGR_NOC_LWH2F_SET_MSK |
		ALT_SYSMGR_NOC_F2H_SET_MSK |
		ALT_SYSMGR_NOC_F2SDR0_SET_MSK |
		ALT_SYSMGR_NOC_F2SDR1_SET_MSK |
		ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
		socfpga_get_sysmgr_addr() + SYSMGR_A10_NOC_IDLEREQ_SET);

	/* Enable the NOC timeout */
	writel(ALT_SYSMGR_NOC_TMO_EN_SET_MSK,
	       socfpga_get_sysmgr_addr() + SYSMGR_A10_NOC_TIMEOUT);

	/* Poll until all idleack to 1 */
	ret = wait_for_bit_le32((const void *)(socfpga_get_sysmgr_addr() +
				SYSMGR_A10_NOC_IDLEACK),
				ALT_SYSMGR_NOC_H2F_SET_MSK |
				ALT_SYSMGR_NOC_LWH2F_SET_MSK |
				ALT_SYSMGR_NOC_F2H_SET_MSK |
				ALT_SYSMGR_NOC_F2SDR0_SET_MSK |
				ALT_SYSMGR_NOC_F2SDR1_SET_MSK |
				ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
				true, 10000, false);
	if (ret)
		return ret;

	/* Poll until all idlestatus to 1 */
	ret = wait_for_bit_le32((const void *)(socfpga_get_sysmgr_addr() +
				SYSMGR_A10_NOC_IDLESTATUS),
				ALT_SYSMGR_NOC_H2F_SET_MSK |
				ALT_SYSMGR_NOC_LWH2F_SET_MSK |
				ALT_SYSMGR_NOC_F2H_SET_MSK |
				ALT_SYSMGR_NOC_F2SDR0_SET_MSK |
				ALT_SYSMGR_NOC_F2SDR1_SET_MSK |
				ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
				true, 10000, false);
	if (ret)
		return ret;

	/* Put all bridges (except NOR DDR scheduler) into reset state */
	setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_A10_BRGMODRST,
		     (ALT_RSTMGR_BRGMODRST_H2F_SET_MSK |
		      ALT_RSTMGR_BRGMODRST_LWH2F_SET_MSK |
		      ALT_RSTMGR_BRGMODRST_F2H_SET_MSK |
		      ALT_RSTMGR_BRGMODRST_F2SSDRAM0_SET_MSK |
		      ALT_RSTMGR_BRGMODRST_F2SSDRAM1_SET_MSK |
		      ALT_RSTMGR_BRGMODRST_F2SSDRAM2_SET_MSK));

	/* Disable NOC timeout */
	writel(0, socfpga_get_sysmgr_addr() + SYSMGR_A10_NOC_TIMEOUT);

	return 0;
}
