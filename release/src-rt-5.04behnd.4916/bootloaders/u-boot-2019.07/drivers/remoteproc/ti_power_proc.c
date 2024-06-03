// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015-2016
 * Texas Instruments Incorporated - http://www.ti.com/
 */
#define pr_fmt(fmt) "%s: " fmt, __func__
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <remoteproc.h>
#include <mach/psc_defs.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct ti_powerproc_privdata - power processor private data
 * @loadaddr:	base address for loading the power processor
 * @psc_module:	psc module address.
 */
struct ti_powerproc_privdata {
	phys_addr_t loadaddr;
	u32 psc_module;
};

/**
 * ti_of_to_priv() - generate private data from device tree
 * @dev:	corresponding ti remote processor device
 * @priv:	pointer to driver specific private data
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int ti_of_to_priv(struct udevice *dev,
			 struct ti_powerproc_privdata *priv)
{
	int node = dev_of_offset(dev);
	const void *blob = gd->fdt_blob;
	int tmp;

	if (!blob) {
		debug("'%s' no dt?\n", dev->name);
		return -EINVAL;
	}

	priv->loadaddr = fdtdec_get_addr(blob, node, "reg");
	if (priv->loadaddr == FDT_ADDR_T_NONE) {
		debug("'%s': no 'reg' property\n", dev->name);
		return -EINVAL;
	}

	tmp = fdtdec_get_int(blob, node, "ti,lpsc_module", -EINVAL);
	if (tmp < 0) {
		debug("'%s': no 'ti,lpsc_module' property\n", dev->name);
		return tmp;
	}
	priv->psc_module = tmp;

	return 0;
}

/**
 * ti_powerproc_probe() - Basic probe
 * @dev:	corresponding ti remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int ti_powerproc_probe(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	struct ti_powerproc_privdata *priv;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);
	priv = dev_get_priv(dev);

	ret = ti_of_to_priv(dev, priv);

	debug("%s probed with slave_addr=0x%08lX module=%d(%d)\n",
	      uc_pdata->name, priv->loadaddr, priv->psc_module, ret);

	return ret;
}

/**
 * ti_powerproc_load() - Loadup the TI remote processor
 * @dev:	corresponding ti remote processor device
 * @addr:	Address in memory where image binary is stored
 * @size:	Size in bytes of the image binary
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int ti_powerproc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	struct ti_powerproc_privdata *priv;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata) {
		debug("%s: no uc pdata!\n", dev->name);
		return -EINVAL;
	}

	priv = dev_get_priv(dev);
	ret = psc_module_keep_in_reset_enabled(priv->psc_module, false);
	if (ret) {
		debug("%s Unable to disable module '%d'(ret=%d)\n",
		      uc_pdata->name, priv->psc_module, ret);
		return ret;
	}

	debug("%s: Loading binary from 0x%08lX, size 0x%08lX to 0x%08lX\n",
	      uc_pdata->name, addr, size, priv->loadaddr);

	memcpy((void *)priv->loadaddr, (void *)addr, size);

	debug("%s: Complete!\n", uc_pdata->name);
	return 0;
}

/**
 * ti_powerproc_start() - (replace: short desc)
 * @dev:	corresponding ti remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int ti_powerproc_start(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	struct ti_powerproc_privdata *priv;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata) {
		debug("%s: no uc pdata!\n", dev->name);
		return -EINVAL;
	}

	priv = dev_get_priv(dev);
	ret = psc_disable_module(priv->psc_module);
	if (ret) {
		debug("%s Unable to disable module '%d'(ret=%d)\n",
		      uc_pdata->name, priv->psc_module, ret);
		return ret;
	}

	ret = psc_module_release_from_reset(priv->psc_module);
	if (ret) {
		debug("%s Failed to wait for module '%d'(ret=%d)\n",
		      uc_pdata->name, priv->psc_module, ret);
		return ret;
	}
	ret = psc_enable_module(priv->psc_module);
	if (ret) {
		debug("%s Unable to disable module '%d'(ret=%d)\n",
		      uc_pdata->name, priv->psc_module, ret);
		return ret;
	}

	return 0;
}

static const struct dm_rproc_ops ti_powerproc_ops = {
	.load = ti_powerproc_load,
	.start = ti_powerproc_start,
};

static const struct udevice_id ti_powerproc_ids[] = {
	{.compatible = "ti,power-processor"},
	{}
};

U_BOOT_DRIVER(ti_powerproc) = {
	.name = "ti_power_proc",
	.of_match = ti_powerproc_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &ti_powerproc_ops,
	.probe = ti_powerproc_probe,
	.priv_auto_alloc_size = sizeof(struct ti_powerproc_privdata),
};
