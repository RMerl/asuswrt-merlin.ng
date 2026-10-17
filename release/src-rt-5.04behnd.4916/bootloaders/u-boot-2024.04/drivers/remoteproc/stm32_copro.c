// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */
#define LOG_CATEGORY UCLASS_REMOTEPROC

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <remoteproc.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/printk.h>

/**
 * struct stm32_copro_privdata - power processor private data
 * @reset_ctl:		reset controller handle
 * @hold_boot:		hold boot controller handle
 * @rsc_table_addr:	resource table address
 */
struct stm32_copro_privdata {
	struct reset_ctl reset_ctl;
	struct reset_ctl hold_boot;
	ulong rsc_table_addr;
};

/**
 * stm32_copro_probe() - Basic probe
 * @dev:	corresponding STM32 remote processor device
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_probe(struct udevice *dev)
{
	struct stm32_copro_privdata *priv;
	int ret;

	priv = dev_get_priv(dev);

	ret = reset_get_by_name(dev, "mcu_rst", &priv->reset_ctl);
	if (ret) {
		dev_err(dev, "failed to get reset (%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "hold_boot", &priv->hold_boot);
	if (ret) {
		dev_err(dev, "failed to get hold boot (%d)\n", ret);
		return ret;
	}

	dev_dbg(dev, "probed\n");

	return 0;
}

/**
 * stm32_copro_device_to_virt() - Convert device address to virtual address
 * @dev:	corresponding STM32 remote processor device
 * @da:		device address
 * @size:	Size of the memory region @da is pointing to
 * Return: converted virtual address
 */
static void *stm32_copro_device_to_virt(struct udevice *dev, ulong da,
					ulong size)
{
	fdt32_t in_addr = cpu_to_be32(da), end_addr;
	u64 paddr;

	paddr = dev_translate_dma_address(dev, &in_addr);
	if (paddr == OF_BAD_ADDR) {
		dev_err(dev, "Unable to convert address %ld\n", da);
		return NULL;
	}

	end_addr = cpu_to_be32(da + size - 1);
	if (dev_translate_dma_address(dev, &end_addr) == OF_BAD_ADDR) {
		dev_err(dev, "Unable to convert address %ld\n", da + size - 1);
		return NULL;
	}

	return phys_to_virt(paddr);
}

/**
 * stm32_copro_load() - Loadup the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * @addr:	Address in memory where image is stored
 * @size:	Size in bytes of the image
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_load(struct udevice *dev, ulong addr, ulong size)
{
	struct stm32_copro_privdata *priv;
	ulong rsc_table_size;
	int ret;

	priv = dev_get_priv(dev);

	ret = reset_assert(&priv->hold_boot);
	if (ret) {
		dev_err(dev, "Unable to assert hold boot (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_assert(&priv->reset_ctl);
	if (ret) {
		dev_err(dev, "Unable to assert reset line (ret=%d)\n", ret);
		return ret;
	}

	if (rproc_elf32_load_rsc_table(dev, addr, size, &priv->rsc_table_addr,
				       &rsc_table_size)) {
		priv->rsc_table_addr = 0;
		dev_warn(dev, "No valid resource table for this firmware\n");
	}

	return rproc_elf32_load_image(dev, addr, size);
}

/**
 * stm32_copro_start() - Start the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_start(struct udevice *dev)
{
	struct stm32_copro_privdata *priv;
	int ret;

	priv = dev_get_priv(dev);

	ret = reset_deassert(&priv->hold_boot);
	if (ret) {
		dev_err(dev, "Unable to deassert hold boot (ret=%d)\n", ret);
		return ret;
	}

	/*
	 * Once copro running, reset hold boot flag to avoid copro
	 * rebooting autonomously (error should never occur)
	 */
	ret = reset_assert(&priv->hold_boot);
	if (ret)
		dev_err(dev, "Unable to assert hold boot (ret=%d)\n", ret);

	/* indicates that copro is running */
	writel(TAMP_COPRO_STATE_CRUN, TAMP_COPRO_STATE);
	/* Store rsc_address in bkp register */
	writel(priv->rsc_table_addr, TAMP_COPRO_RSC_TBL_ADDRESS);

	return 0;
}

/**
 * stm32_copro_reset() - Reset the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_reset(struct udevice *dev)
{
	struct stm32_copro_privdata *priv;
	int ret;

	priv = dev_get_priv(dev);

	ret = reset_assert(&priv->hold_boot);
	if (ret) {
		dev_err(dev, "Unable to assert hold boot (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_assert(&priv->reset_ctl);
	if (ret) {
		dev_err(dev, "Unable to assert reset line (ret=%d)\n", ret);
		return ret;
	}

	writel(TAMP_COPRO_STATE_OFF, TAMP_COPRO_STATE);

	return 0;
}

/**
 * stm32_copro_stop() - Stop the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_stop(struct udevice *dev)
{
	return stm32_copro_reset(dev);
}

/**
 * stm32_copro_is_running() - Is the STM32 remote processor running
 * @dev:	corresponding STM32 remote processor device
 * Return: 0 if the remote processor is running, 1 otherwise
 */
static int stm32_copro_is_running(struct udevice *dev)
{
	return (readl(TAMP_COPRO_STATE) == TAMP_COPRO_STATE_OFF);
}

static const struct dm_rproc_ops stm32_copro_ops = {
	.load = stm32_copro_load,
	.start = stm32_copro_start,
	.stop =  stm32_copro_stop,
	.reset = stm32_copro_reset,
	.is_running = stm32_copro_is_running,
	.device_to_virt = stm32_copro_device_to_virt,
};

static const struct udevice_id stm32_copro_ids[] = {
	{.compatible = "st,stm32mp1-m4"},
	{}
};

U_BOOT_DRIVER(stm32_copro) = {
	.name = "stm32_m4_proc",
	.of_match = stm32_copro_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &stm32_copro_ops,
	.probe = stm32_copro_probe,
	.priv_auto	= sizeof(struct stm32_copro_privdata),
};
