// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx AXI platforms watchdog timer driver.
 *
 * Author(s):	Michal Simek <michal.simek@xilinx.com>
 *		Shreenidhi Shedi <yesshedi@gmail.com>
 *
 * Copyright (c) 2011-2018 Xilinx Inc.
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <linux/io.h>

#define XWT_CSR0_WRS_MASK	0x00000008 /* Reset status Mask */
#define XWT_CSR0_WDS_MASK	0x00000004 /* Timer state Mask */
#define XWT_CSR0_EWDT1_MASK	0x00000002 /* Enable bit 1 Mask*/
#define XWT_CSRX_EWDT2_MASK	0x00000001 /* Enable bit 2 Mask */

struct watchdog_regs {
	u32 twcsr0; /* 0x0 */
	u32 twcsr1; /* 0x4 */
	u32 tbr; /* 0x8 */
};

struct xlnx_wdt_platdata {
	bool enable_once;
	struct watchdog_regs *regs;
};

static int xlnx_wdt_reset(struct udevice *dev)
{
	u32 reg;
	struct xlnx_wdt_platdata *platdata = dev_get_platdata(dev);

	debug("%s ", __func__);

	/* Read the current contents of TCSR0 */
	reg = readl(&platdata->regs->twcsr0);

	/* Clear the watchdog WDS bit */
	if (reg & (XWT_CSR0_EWDT1_MASK | XWT_CSRX_EWDT2_MASK))
		writel(reg | XWT_CSR0_WDS_MASK, &platdata->regs->twcsr0);

	return 0;
}

static int xlnx_wdt_stop(struct udevice *dev)
{
	u32 reg;
	struct xlnx_wdt_platdata *platdata = dev_get_platdata(dev);

	if (platdata->enable_once) {
		debug("Can't stop Xilinx watchdog.\n");
		return -EBUSY;
	}

	/* Read the current contents of TCSR0 */
	reg = readl(&platdata->regs->twcsr0);

	writel(reg & ~XWT_CSR0_EWDT1_MASK, &platdata->regs->twcsr0);
	writel(~XWT_CSRX_EWDT2_MASK, &platdata->regs->twcsr1);

	debug("Watchdog disabled!\n");

	return 0;
}

static int xlnx_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct xlnx_wdt_platdata *platdata = dev_get_platdata(dev);

	debug("%s:\n", __func__);

	writel((XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK | XWT_CSR0_EWDT1_MASK),
	       &platdata->regs->twcsr0);

	writel(XWT_CSRX_EWDT2_MASK, &platdata->regs->twcsr1);

	return 0;
}

static int xlnx_wdt_probe(struct udevice *dev)
{
	debug("%s: Probing wdt%u\n", __func__, dev->seq);

	return 0;
}

static int xlnx_wdt_ofdata_to_platdata(struct udevice *dev)
{
	struct xlnx_wdt_platdata *platdata = dev_get_platdata(dev);

	platdata->regs = (struct watchdog_regs *)dev_read_addr(dev);
	if (IS_ERR(platdata->regs))
		return PTR_ERR(platdata->regs);

	platdata->enable_once = dev_read_u32_default(dev,
						     "xlnx,wdt-enable-once", 0);

	debug("%s: wdt-enable-once %d\n", __func__, platdata->enable_once);

	return 0;
}

static const struct wdt_ops xlnx_wdt_ops = {
	.start = xlnx_wdt_start,
	.reset = xlnx_wdt_reset,
	.stop = xlnx_wdt_stop,
};

static const struct udevice_id xlnx_wdt_ids[] = {
	{ .compatible = "xlnx,xps-timebase-wdt-1.00.a", },
	{ .compatible = "xlnx,xps-timebase-wdt-1.01.a", },
	{},
};

U_BOOT_DRIVER(xlnx_wdt) = {
	.name = "xlnx_wdt",
	.id = UCLASS_WDT,
	.of_match = xlnx_wdt_ids,
	.probe = xlnx_wdt_probe,
	.platdata_auto_alloc_size = sizeof(struct xlnx_wdt_platdata),
	.ofdata_to_platdata = xlnx_wdt_ofdata_to_platdata,
	.ops = &xlnx_wdt_ops,
};
