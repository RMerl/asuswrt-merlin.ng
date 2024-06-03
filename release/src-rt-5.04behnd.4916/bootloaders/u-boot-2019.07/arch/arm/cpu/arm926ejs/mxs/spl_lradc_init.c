// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 Battery measurement init
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

#include "mxs_init.h"

void mxs_lradc_init(void)
{
	struct mxs_lradc_regs *regs = (struct mxs_lradc_regs *)MXS_LRADC_BASE;

	debug("SPL: Initialisating LRADC\n");

	writel(LRADC_CTRL0_SFTRST, &regs->hw_lradc_ctrl0_clr);
	writel(LRADC_CTRL0_CLKGATE, &regs->hw_lradc_ctrl0_clr);
	writel(LRADC_CTRL0_ONCHIP_GROUNDREF, &regs->hw_lradc_ctrl0_clr);

	clrsetbits_le32(&regs->hw_lradc_ctrl3,
			LRADC_CTRL3_CYCLE_TIME_MASK,
			LRADC_CTRL3_CYCLE_TIME_6MHZ);

	clrsetbits_le32(&regs->hw_lradc_ctrl4,
			LRADC_CTRL4_LRADC7SELECT_MASK |
			LRADC_CTRL4_LRADC6SELECT_MASK,
			LRADC_CTRL4_LRADC7SELECT_CHANNEL7 |
			LRADC_CTRL4_LRADC6SELECT_CHANNEL10);
}

void mxs_lradc_enable_batt_measurement(void)
{
	struct mxs_lradc_regs *regs = (struct mxs_lradc_regs *)MXS_LRADC_BASE;

	debug("SPL: Enabling LRADC battery measurement\n");

	/* Check if the channel is present at all. */
	if (!(readl(&regs->hw_lradc_status) & LRADC_STATUS_CHANNEL7_PRESENT)) {
		debug("SPL: LRADC channel 7 is not present - aborting\n");
		return;
	}

	debug("SPL: LRADC channel 7 is present - configuring\n");

	writel(LRADC_CTRL1_LRADC7_IRQ_EN, &regs->hw_lradc_ctrl1_clr);
	writel(LRADC_CTRL1_LRADC7_IRQ, &regs->hw_lradc_ctrl1_clr);

	clrsetbits_le32(&regs->hw_lradc_conversion,
			LRADC_CONVERSION_SCALE_FACTOR_MASK,
			LRADC_CONVERSION_SCALE_FACTOR_LI_ION);
	writel(LRADC_CONVERSION_AUTOMATIC, &regs->hw_lradc_conversion_set);

	/* Configure the channel. */
	writel((1 << 7) << LRADC_CTRL2_DIVIDE_BY_TWO_OFFSET,
		&regs->hw_lradc_ctrl2_clr);
	writel(0xffffffff, &regs->hw_lradc_ch7_clr);
	clrbits_le32(&regs->hw_lradc_ch7, LRADC_CH_NUM_SAMPLES_MASK);
	writel(LRADC_CH_ACCUMULATE, &regs->hw_lradc_ch7_clr);

	/* Schedule the channel. */
	writel(1 << 7, &regs->hw_lradc_ctrl0_set);

	/* Start the channel sampling. */
	writel(((1 << 7) << LRADC_DELAY_TRIGGER_LRADCS_OFFSET) |
		((1 << 3) << LRADC_DELAY_TRIGGER_DELAYS_OFFSET) |
		100, &regs->hw_lradc_delay3);

	writel(0xffffffff, &regs->hw_lradc_ch7_clr);
	writel(LRADC_DELAY_KICK, &regs->hw_lradc_delay3_set);

	debug("SPL: LRADC channel 7 configuration complete\n");
}
