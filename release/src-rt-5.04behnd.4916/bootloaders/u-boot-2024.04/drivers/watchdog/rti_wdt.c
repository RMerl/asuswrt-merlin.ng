// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Siemens AG, 2020
 *
 * Authors:
 *   Jan Kiszka <jan.kiszka@siemens.com>
 *
 * Derived from linux/drivers/watchdog/rti_wdt.c
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <power-domain.h>
#include <wdt.h>
#include <asm/io.h>
#include <remoteproc.h>

/* Timer register set definition */
#define RTIDWDCTRL		0x90
#define RTIDWDPRLD		0x94
#define RTIWDSTATUS		0x98
#define RTIWDKEY		0x9c
#define RTIDWDCNTR		0xa0
#define RTIWWDRXCTRL		0xa4
#define RTIWWDSIZECTRL		0xa8

#define RTIWWDRX_NMI		0xa

#define RTIWWDSIZE_50P		0x50

#define WDENABLE_KEY		0xa98559da

#define WDKEY_SEQ0		0xe51a
#define WDKEY_SEQ1		0xa35c

#define WDT_PRELOAD_SHIFT	13

#define WDT_PRELOAD_MAX		0xfff

struct rti_wdt_priv {
	phys_addr_t regs;
	unsigned int clk_hz;
};

#ifdef CONFIG_WDT_K3_RTI_LOAD_FW
#define RTI_WDT_FIT_PATH	"/fit-images/k3-rti-wdt-firmware"

static int rti_wdt_load_fw(struct udevice *dev)
{
	struct udevice *rproc_dev;
	int primary_core, ret;
	u32 cluster_mode;
	ofnode node;
	u64 rti_wdt_fw;
	u32 rti_wdt_fw_size;

	node = ofnode_path(RTI_WDT_FIT_PATH);
	if (!ofnode_valid(node))
		goto fit_error;

	ret = ofnode_read_u64(node, "load", &rti_wdt_fw);
	if (ret)
		goto fit_error;
	ret = ofnode_read_u32(node, "size", &rti_wdt_fw_size);
	if (ret)
		goto fit_error;

	node = ofnode_by_compatible(ofnode_null(), "ti,am654-r5fss");
	if (!ofnode_valid(node))
		goto dt_error;

	ret = ofnode_read_u32(node, "ti,cluster-mode", &cluster_mode);
	if (ret)
		cluster_mode = 1;

	node = ofnode_by_compatible(node, "ti,am654-r5f");
	if (!ofnode_valid(node))
		goto dt_error;

	ret = uclass_get_device_by_ofnode(UCLASS_REMOTEPROC, node, &rproc_dev);
	if (ret)
		return ret;

	primary_core = dev_seq(rproc_dev);

	ret = rproc_dev_init(primary_core);
	if (ret)
		goto fw_error;

	if (cluster_mode == 1) {
		ret = rproc_dev_init(primary_core + 1);
		if (ret)
			goto fw_error;
	}

	ret = rproc_load(primary_core, (ulong)rti_wdt_fw,
			 rti_wdt_fw_size);
	if (ret)
		goto fw_error;

	ret = rproc_start(primary_core);
	if (ret)
		goto fw_error;

	return 0;

fit_error:
	dev_err(dev, "No loadable firmware found under %s\n", RTI_WDT_FIT_PATH);
	return -ENOENT;

dt_error:
	dev_err(dev, "No compatible firmware target processor found\n");
	return -ENODEV;

fw_error:
	dev_err(dev, "Failed to load watchdog firmware into remote processor %d\n",
		primary_core);
	return ret;
}
#else
static inline int rti_wdt_load_fw(struct udevice *dev)
{
	return 0;
}
#endif

static int rti_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct rti_wdt_priv *priv = dev_get_priv(dev);
	u32 timer_margin;
	int ret;

	if (readl(priv->regs + RTIDWDCTRL) == WDENABLE_KEY)
		return -EBUSY;

	ret = rti_wdt_load_fw(dev);
	if (ret < 0)
		return ret;

	timer_margin = timeout_ms * priv->clk_hz / 1000;
	timer_margin >>= WDT_PRELOAD_SHIFT;
	if (timer_margin > WDT_PRELOAD_MAX)
		timer_margin = WDT_PRELOAD_MAX;

	writel(timer_margin, priv->regs + RTIDWDPRLD);
	writel(RTIWWDRX_NMI, priv->regs + RTIWWDRXCTRL);
	writel(RTIWWDSIZE_50P, priv->regs + RTIWWDSIZECTRL);

	readl(priv->regs + RTIWWDSIZECTRL);

	writel(WDENABLE_KEY, priv->regs + RTIDWDCTRL);

	return 0;
}

static int rti_wdt_reset(struct udevice *dev)
{
	struct rti_wdt_priv *priv = dev_get_priv(dev);
	u32 prld;

	/* Make sure we do not reset too early */
	prld = readl(priv->regs + RTIDWDPRLD) << WDT_PRELOAD_SHIFT;
	if (readl(priv->regs + RTIDWDCNTR) >= prld / 2)
		return -EPERM;

	writel(WDKEY_SEQ0, priv->regs + RTIWDKEY);
	writel(WDKEY_SEQ1, priv->regs + RTIWDKEY);

	return 0;
}

static int rti_wdt_probe(struct udevice *dev)
{
	struct rti_wdt_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->regs = devfdt_get_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	priv->clk_hz = clk_get_rate(&clk);

	/*
	 * If watchdog is running at 32k clock, it is not accurate.
	 * Adjust frequency down in this case so that it does not expire
	 * earlier than expected.
	 */
	if (priv->clk_hz < 32768)
		priv->clk_hz = priv->clk_hz * 9 / 10;

	return 0;
}

static const struct wdt_ops rti_wdt_ops = {
	.start = rti_wdt_start,
	.reset = rti_wdt_reset,
};

static const struct udevice_id rti_wdt_ids[] = {
	{ .compatible = "ti,j7-rti-wdt" },
	{ }
};

U_BOOT_DRIVER(rti_wdt) = {
	.name = "rti_wdt",
	.id = UCLASS_WDT,
	.of_match = rti_wdt_ids,
	.ops = &rti_wdt_ops,
	.probe = rti_wdt_probe,
	.priv_auto	= sizeof(struct rti_wdt_priv),
	.flags = DM_FLAG_LEAVE_PD_ON,
};
