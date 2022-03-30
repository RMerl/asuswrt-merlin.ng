// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 Remoteproc driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
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
#include <linux/soc/ti/ti_sci_protocol.h>

#define INVALID_ID	0xffff

#define GTC_CNTCR_REG	0x0
#define GTC_CNTR_EN	0x3

/**
 * struct k3_rproc_privdata - Structure representing Remote processor data.
 * @rproc_pwrdmn:	rproc power domain data
 * @rproc_rst:		rproc reset control data
 * @sci:		Pointer to TISCI handle
 * @gtc_base:		Timer base address.
 * @proc_id:		TISCI processor ID
 * @host_id:		TISCI host id to which the processor gets assigned to.
 */
struct k3_rproc_privdata {
	struct power_domain rproc_pwrdmn;
	struct power_domain gtc_pwrdmn;
	struct reset_ctl rproc_rst;
	const struct ti_sci_handle *sci;
	void *gtc_base;
	u16 proc_id;
	u16 host_id;
};

/**
 * k3_rproc_load() - Load up the Remote processor image
 * @dev:	rproc device pointer
 * @addr:	Address at which image is available
 * @size:	size of the image
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_rproc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct k3_rproc_privdata *rproc = dev_get_priv(dev);
	const struct ti_sci_proc_ops *pops = &rproc->sci->ops.proc_ops;
	int ret;

	dev_dbg(dev, "%s addr = 0x%lx, size = 0x%lx\n", __func__, addr, size);

	/* request for the processor */
	ret = pops->proc_request(rproc->sci, rproc->proc_id);
	if (ret) {
		dev_err(dev, "Requesting processor failed %d\n", ret);
		return ret;
	}

	ret = pops->set_proc_boot_cfg(rproc->sci, rproc->proc_id, addr, 0, 0);
	if (ret) {
		dev_err(dev, "set_proc_boot_cfg failed %d\n", ret);
		return ret;
	}

	dev_dbg(dev, "%s: rproc successfully loaded\n", __func__);

	return 0;
}

/**
 * k3_rproc_start() - Start the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_rproc_start(struct udevice *dev)
{
	struct k3_rproc_privdata *rproc = dev_get_priv(dev);
	const struct ti_sci_proc_ops *pops = &rproc->sci->ops.proc_ops;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = power_domain_on(&rproc->gtc_pwrdmn);
	if (ret) {
		dev_err(dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	/* Enable the timer before starting remote core */
	writel(GTC_CNTR_EN, rproc->gtc_base + GTC_CNTCR_REG);

	/*
	 * Setting the right clock frequency would have taken care by
	 * assigned-clock-rates during the device probe. So no need to
	 * set the frequency again here.
	 */
	ret = power_domain_on(&rproc->rproc_pwrdmn);
	if (ret) {
		dev_err(dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	if (rproc->host_id != INVALID_ID) {
		ret = pops->proc_handover(rproc->sci, rproc->proc_id,
					  rproc->host_id);
		if (ret) {
			dev_err(dev, "Handover processor failed %d\n", ret);
			return ret;
		}
	} else {
		ret = pops->proc_release(rproc->sci, rproc->proc_id);
		if (ret) {
			dev_err(dev, "Processor release failed %d\n", ret);
			return ret;
		}
	}

	dev_dbg(dev, "%s: rproc successfully started\n", __func__);

	return 0;
}

/**
 * k3_rproc_init() - Initialize the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_rproc_init(struct udevice *dev)
{
	dev_dbg(dev, "%s\n", __func__);

	/* Enable the module */
	dev_dbg(dev, "%s: rproc successfully initialized\n", __func__);

	return 0;
}

static const struct dm_rproc_ops k3_rproc_ops = {
	.init = k3_rproc_init,
	.load = k3_rproc_load,
	.start = k3_rproc_start,
};

/**
 * k3_of_to_priv() - generate private data from device tree
 * @dev:	corresponding k3 remote processor device
 * @priv:	pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_rproc_of_to_priv(struct udevice *dev,
			       struct k3_rproc_privdata *rproc)
{
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = power_domain_get_by_index(dev, &rproc->rproc_pwrdmn, 1);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_get_by_index(dev, &rproc->gtc_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &rproc->rproc_rst);
	if (ret) {
		dev_err(dev, "reset_get() failed: %d\n", ret);
		return ret;
	}

	rproc->sci = ti_sci_get_by_phandle(dev, "ti,sci");
	if (IS_ERR(rproc->sci)) {
		dev_err(dev, "ti_sci get failed: %d\n", ret);
		return PTR_ERR(rproc->sci);
	}

	rproc->gtc_base = dev_read_addr_ptr(dev);
	if (!rproc->gtc_base) {
		dev_err(dev, "Get address failed\n");
		return -ENODEV;
	}

	rproc->proc_id = dev_read_u32_default(dev, "ti,sci-proc-id",
					      INVALID_ID);
	rproc->host_id = dev_read_u32_default(dev, "ti,sci-host-id",
					      INVALID_ID);

	return 0;
}

/**
 * k3_rproc_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_rproc_probe(struct udevice *dev)
{
	struct k3_rproc_privdata *priv;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	priv = dev_get_priv(dev);

	ret = k3_rproc_of_to_priv(dev, priv);
	if (ret) {
		dev_dbg(dev, "%s: Probe failed with error %d\n", __func__, ret);
		return ret;
	}

	dev_dbg(dev, "Remoteproc successfully probed\n");

	return 0;
}

static const struct udevice_id k3_rproc_ids[] = {
	{ .compatible = "ti,am654-rproc"},
	{}
};

U_BOOT_DRIVER(k3_rproc) = {
	.name = "k3_rproc",
	.of_match = k3_rproc_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &k3_rproc_ops,
	.probe = k3_rproc_probe,
	.priv_auto_alloc_size = sizeof(struct k3_rproc_privdata),
};
