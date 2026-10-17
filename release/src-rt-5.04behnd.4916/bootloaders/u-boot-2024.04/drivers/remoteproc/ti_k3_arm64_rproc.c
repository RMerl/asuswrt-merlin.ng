// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 ARM64 Remoteproc driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <remoteproc.h>
#include <errno.h>
#include <clk.h>
#include <reset.h>
#include <asm/io.h>
#include <power-domain.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include "ti_sci_proc.h"

#define INVALID_ID	0xffff

#define GTC_CNTCR_REG	0x0
#define GTC_CNTFID0_REG	0x20
#define GTC_CNTR_EN	0x3

/**
 * struct k3_arm64_privdata - Structure representing Remote processor data.
 * @rproc_pwrdmn:	rproc power domain data
 * @rproc_rst:		rproc reset control data
 * @sci:		Pointer to TISCI handle
 * @tsp:		TISCI processor control helper structure
 * @gtc_clk:		GTC clock description
 * @gtc_base:		Timer base address.
 */
struct k3_arm64_privdata {
	bool has_cluster_node;
	struct power_domain cluster_pwrdmn;
	struct power_domain rproc_pwrdmn;
	struct power_domain gtc_pwrdmn;
	struct reset_ctl rproc_rst;
	struct ti_sci_proc tsp;
	struct clk gtc_clk;
	void *gtc_base;
};

/**
 * k3_arm64_load() - Load up the Remote processor image
 * @dev:	rproc device pointer
 * @addr:	Address at which image is available
 * @size:	size of the image
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_arm64_load(struct udevice *dev, ulong addr, ulong size)
{
	struct k3_arm64_privdata *rproc = dev_get_priv(dev);
	ulong gtc_rate;
	int ret;

	dev_dbg(dev, "%s addr = 0x%lx, size = 0x%lx\n", __func__, addr, size);

	/* request for the processor */
	ret = ti_sci_proc_request(&rproc->tsp);
	if (ret)
		return ret;

	ret = power_domain_on(&rproc->gtc_pwrdmn);
	if (ret) {
		dev_err(dev, "power_domain_on(&rproc->gtc_pwrdmn) failed: %d\n",
			ret);
		return ret;
	}

	gtc_rate = clk_get_rate(&rproc->gtc_clk);
	dev_dbg(dev, "GTC RATE= %d\n", (u32) gtc_rate);
	/* Store the clock frequency down for GTC users to pick  up */
	writel((u32)gtc_rate, rproc->gtc_base + GTC_CNTFID0_REG);

	/* Enable the timer before starting remote core */
	writel(GTC_CNTR_EN, rproc->gtc_base + GTC_CNTCR_REG);

	/*
	 * Setting the right clock frequency would have taken care by
	 * assigned-clock-rates during the device probe. So no need to
	 * set the frequency again here.
	 */
	if (rproc->has_cluster_node) {
		ret = power_domain_on(&rproc->cluster_pwrdmn);
		if (ret) {
			dev_err(dev,
				"power_domain_on(&rproc->cluster_pwrdmn) failed: %d\n",
				ret);
			return ret;
		}
	}

	return ti_sci_proc_set_config(&rproc->tsp, addr, 0, 0);
}

/**
 * k3_arm64_start() - Start the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_arm64_start(struct udevice *dev)
{
	struct k3_arm64_privdata *rproc = dev_get_priv(dev);
	int ret;

	dev_dbg(dev, "%s\n", __func__);
	ret = power_domain_on(&rproc->rproc_pwrdmn);
	if (ret) {
		dev_err(dev,
			"power_domain_on(&rproc->rproc_pwrdmn) failed: %d\n",
			ret);
		return ret;
	}

	return ti_sci_proc_release(&rproc->tsp);
}

/**
 * k3_arm64_init() - Initialize the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_arm64_init(struct udevice *dev)
{
	dev_dbg(dev, "%s\n", __func__);

	/* Enable the module */
	dev_dbg(dev, "%s: rproc successfully initialized\n", __func__);

	return 0;
}

static const struct dm_rproc_ops k3_arm64_ops = {
	.init = k3_arm64_init,
	.load = k3_arm64_load,
	.start = k3_arm64_start,
};

static int ti_sci_proc_of_to_priv(struct udevice *dev, struct ti_sci_proc *tsp)
{
	dev_dbg(dev, "%s\n", __func__);

	tsp->sci = ti_sci_get_by_phandle(dev, "ti,sci");
	if (IS_ERR(tsp->sci)) {
		dev_err(dev, "ti_sci get failed: %ld\n", PTR_ERR(tsp->sci));
		return PTR_ERR(tsp->sci);
	}

	tsp->proc_id = dev_read_u32_default(dev, "ti,sci-proc-id", INVALID_ID);
	if (tsp->proc_id == INVALID_ID) {
		dev_err(dev, "proc id not populated\n");
		return -ENOENT;
	}
	tsp->host_id = dev_read_u32_default(dev, "ti,sci-host-id", INVALID_ID);
	tsp->ops = &tsp->sci->ops.proc_ops;

	return 0;
}

/**
 * k3_of_to_priv() - generate private data from device tree
 * @dev:	corresponding k3 remote processor device
 * @priv:	pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_arm64_of_to_priv(struct udevice *dev,
			       struct k3_arm64_privdata *rproc)
{
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	/* Cluster needs to be powered on if firewalls are being configured */
	rproc->has_cluster_node = true;
	ret = power_domain_get_by_index(dev, &rproc->cluster_pwrdmn, 2);
	if (ret) {
		dev_dbg(dev, "warning: power_domain_get_cluster() failed: %d\n", ret);
		rproc->has_cluster_node = false;
	}

	ret = power_domain_get_by_index(dev, &rproc->rproc_pwrdmn, 1);
	if (ret) {
		dev_err(dev, "power_domain_get_rproc() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_get_by_index(dev, &rproc->gtc_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_index(dev, 0, &rproc->gtc_clk);
	if (ret) {
		dev_err(dev, "clk_get failed: %d\n", ret);
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &rproc->rproc_rst);
	if (ret) {
		dev_err(dev, "reset_get() failed: %d\n", ret);
		return ret;
	}

	ret = ti_sci_proc_of_to_priv(dev, &rproc->tsp);
	if (ret)
		return ret;

	rproc->gtc_base = dev_read_addr_ptr(dev);
	if (!rproc->gtc_base) {
		dev_err(dev, "Get address failed\n");
		return -ENODEV;
	}

	return 0;
}

/**
 * k3_arm64_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_arm64_probe(struct udevice *dev)
{
	struct k3_arm64_privdata *priv;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	priv = dev_get_priv(dev);

	ret = k3_arm64_of_to_priv(dev, priv);
	if (ret) {
		dev_dbg(dev, "%s: Probe failed with error %d\n", __func__, ret);
		return ret;
	}

	dev_dbg(dev, "Remoteproc successfully probed\n");

	return 0;
}

static const struct udevice_id k3_arm64_ids[] = {
	{ .compatible = "ti,am654-arm64"},
	{ .compatible = "ti,am654-rproc"},
	{}
};

U_BOOT_DRIVER(k3_arm64) = {
	.name = "k3_arm64",
	.of_match = k3_arm64_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &k3_arm64_ops,
	.probe = k3_arm64_probe,
	.priv_auto	= sizeof(struct k3_arm64_privdata),
	.flags = DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
