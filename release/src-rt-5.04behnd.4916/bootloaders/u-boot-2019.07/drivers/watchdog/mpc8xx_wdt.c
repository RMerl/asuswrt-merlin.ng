// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 CS Systemes d'Information
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <mpc8xx.h>
#include <asm/cpm_8xx.h>
#include <asm/io.h>

static void hw_watchdog_reset(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	out_be16(&immap->im_siu_conf.sc_swsr, 0x556c);	/* write magic1 */
	out_be16(&immap->im_siu_conf.sc_swsr, 0xaa39);	/* write magic2 */
}

static int mpc8xx_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	out_be32(&immap->im_siu_conf.sc_sypcr, CONFIG_SYS_SYPCR);

	if (!(in_be32(&immap->im_siu_conf.sc_sypcr) & SYPCR_SWE))
		return -EBUSY;
	return 0;

}

static int mpc8xx_wdt_stop(struct udevice *dev)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	out_be32(&immap->im_siu_conf.sc_sypcr, CONFIG_SYS_SYPCR & ~SYPCR_SWE);

	if (in_be32(&immap->im_siu_conf.sc_sypcr) & SYPCR_SWE)
		return -EBUSY;
	return 0;
}

static int mpc8xx_wdt_reset(struct udevice *dev)
{
	hw_watchdog_reset();

	return 0;
}

static const struct wdt_ops mpc8xx_wdt_ops = {
	.start = mpc8xx_wdt_start,
	.reset = mpc8xx_wdt_reset,
	.stop = mpc8xx_wdt_stop,
};

static const struct udevice_id mpc8xx_wdt_ids[] = {
	{ .compatible = "fsl,pq1-wdt" },
	{}
};

U_BOOT_DRIVER(wdt_mpc8xx) = {
	.name = "wdt_mpc8xx",
	.id = UCLASS_WDT,
	.of_match = mpc8xx_wdt_ids,
	.ops = &mpc8xx_wdt_ops,
};
