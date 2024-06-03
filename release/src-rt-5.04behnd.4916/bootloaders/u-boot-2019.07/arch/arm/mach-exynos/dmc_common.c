// SPDX-License-Identifier: GPL-2.0+
/*
 * Mem setup common file for different types of DDR present on Exynos boards.
 *
 * Copyright (C) 2012 Samsung Electronics
 */

#include <common.h>
#include <asm/arch/spl.h>

#include "clock_init.h"
#include "common_setup.h"
#include "exynos5_setup.h"

#define ZQ_INIT_TIMEOUT	10000

int dmc_config_zq(struct mem_timings *mem, uint32_t *phy0_con16,
			uint32_t *phy1_con16, uint32_t *phy0_con17,
			uint32_t *phy1_con17)
{
	unsigned long val = 0;
	int i;

	/*
	 * ZQ Calibration:
	 * Select Driver Strength,
	 * long calibration for manual calibration
	 */
	val = PHY_CON16_RESET_VAL;
	val |= mem->zq_mode_dds << PHY_CON16_ZQ_MODE_DDS_SHIFT;
	val |= mem->zq_mode_term << PHY_CON16_ZQ_MODE_TERM_SHIFT;
	val |= ZQ_CLK_DIV_EN;
	writel(val, phy0_con16);
	writel(val, phy1_con16);

	/* Disable termination */
	if (mem->zq_mode_noterm)
		val |= PHY_CON16_ZQ_MODE_NOTERM_MASK;
	writel(val, phy0_con16);
	writel(val, phy1_con16);

	/* ZQ_MANUAL_START: Enable */
	val |= ZQ_MANUAL_STR;
	writel(val, phy0_con16);
	writel(val, phy1_con16);

	/* ZQ_MANUAL_START: Disable */
	val &= ~ZQ_MANUAL_STR;

	/*
	 * Since we are manaully calibrating the ZQ values,
	 * we are looping for the ZQ_init to complete.
	 */
	i = ZQ_INIT_TIMEOUT;
	while ((readl(phy0_con17) & ZQ_DONE) != ZQ_DONE && i > 0) {
		sdelay(100);
		i--;
	}
	if (!i)
		return -1;
	writel(val, phy0_con16);

	i = ZQ_INIT_TIMEOUT;
	while ((readl(phy1_con17) & ZQ_DONE) != ZQ_DONE && i > 0) {
		sdelay(100);
		i--;
	}
	if (!i)
		return -1;
	writel(val, phy1_con16);

	return 0;
}

void update_reset_dll(uint32_t *phycontrol0, enum ddr_mode mode)
{
	unsigned long val;

	if (mode == DDR_MODE_DDR3) {
		val = MEM_TERM_EN | PHY_TERM_EN | DMC_CTRL_SHGATE;
		writel(val, phycontrol0);
	}

	/* Update DLL Information: Force DLL Resyncronization */
	val = readl(phycontrol0);
	val |= FP_RSYNC;
	writel(val, phycontrol0);

	/* Reset Force DLL Resyncronization */
	val = readl(phycontrol0);
	val &= ~FP_RSYNC;
	writel(val, phycontrol0);
}

void dmc_config_mrs(struct mem_timings *mem, uint32_t *directcmd)
{
	int channel, chip;

	for (channel = 0; channel < mem->dmc_channels; channel++) {
		unsigned long mask;

		mask = channel << DIRECT_CMD_CHANNEL_SHIFT;
		for (chip = 0; chip < mem->chips_to_configure; chip++) {
			int i;

			mask |= chip << DIRECT_CMD_CHIP_SHIFT;

			/* Sending NOP command */
			writel(DIRECT_CMD_NOP | mask, directcmd);

			/*
			 * TODO(alim.akhtar@samsung.com): Do we need these
			 * delays? This one and the next were not there for
			 * DDR3.
			 */
			sdelay(0x10000);

			/* Sending EMRS/MRS commands */
			for (i = 0; i < MEM_TIMINGS_MSR_COUNT; i++) {
				writel(mem->direct_cmd_msr[i] | mask,
				       directcmd);
				sdelay(0x10000);
			}

			if (mem->send_zq_init) {
				/* Sending ZQINIT command */
				writel(DIRECT_CMD_ZQINIT | mask,
				       directcmd);

				sdelay(10000);
			}
		}
	}
}

void dmc_config_prech(struct mem_timings *mem, uint32_t *directcmd)
{
	int channel, chip;

	for (channel = 0; channel < mem->dmc_channels; channel++) {
		unsigned long mask;

		mask = channel << DIRECT_CMD_CHANNEL_SHIFT;
		for (chip = 0; chip < mem->chips_per_channel; chip++) {
			mask |= chip << DIRECT_CMD_CHIP_SHIFT;

			/* PALL (all banks precharge) CMD */
			writel(DIRECT_CMD_PALL | mask, directcmd);
			sdelay(0x10000);
		}
	}
}

void mem_ctrl_init(int reset)
{
	struct spl_machine_param *param = spl_get_machine_params();
	struct mem_timings *mem;
	int ret;

	mem = clock_get_mem_timings();

	/* If there are any other memory variant, add their init call below */
	if (param->mem_type == DDR_MODE_DDR3) {
		ret = ddr3_mem_ctrl_init(mem, reset);
		if (ret) {
			/* will hang if failed to init memory control */
			while (1)
				;
		}
	} else {
		/* will hang if unknow memory type  */
		while (1)
			;
	}
}
