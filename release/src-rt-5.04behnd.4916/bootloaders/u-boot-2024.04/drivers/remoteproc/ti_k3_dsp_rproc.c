// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 DSP Remoteproc driver
 *
 * Copyright (C) 2018-2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *	Suman Anna <s-anna@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <remoteproc.h>
#include <errno.h>
#include <clk.h>
#include <reset.h>
#include <asm/io.h>
#include <power-domain.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include "ti_sci_proc.h"

#define KEYSTONE_RPROC_LOCAL_ADDRESS_MASK	(SZ_16M - 1)

/**
 * struct k3_dsp_mem - internal memory structure
 * @cpu_addr: MPU virtual address of the memory region
 * @bus_addr: Bus address used to access the memory region
 * @dev_addr: Device address from remoteproc view
 * @size: Size of the memory region
 */
struct k3_dsp_mem {
	void __iomem *cpu_addr;
	phys_addr_t bus_addr;
	phys_addr_t dev_addr;
	size_t size;
};

/**
 * struct k3_dsp_boot_data - internal data structure used for boot
 * @boot_align_addr: Boot vector address alignment granularity
 * @uses_lreset: Flag to denote the need for local reset management
 */
struct k3_dsp_boot_data {
	u32 boot_align_addr;
	bool uses_lreset;
};

/**
 * struct k3_dsp_privdata - Structure representing Remote processor data.
 * @rproc_rst:		rproc reset control data
 * @tsp:		Pointer to TISCI proc contrl handle
 * @data:		Pointer to DSP specific boot data structure
 * @mem:		Array of available memories
 * @num_mem:		Number of available memories
 * @in_use: flag to tell if the core is already in use.
 */
struct k3_dsp_privdata {
	struct reset_ctl dsp_rst;
	struct ti_sci_proc tsp;
	struct k3_dsp_boot_data *data;
	struct k3_dsp_mem *mem;
	int num_mems;
	bool in_use;
};

/*
 * The C66x DSP cores have a local reset that affects only the CPU, and a
 * generic module reset that powers on the device and allows the DSP internal
 * memories to be accessed while the local reset is asserted. This function is
 * used to release the global reset on C66x DSPs to allow loading into the DSP
 * internal RAMs. This helper function is invoked in k3_dsp_load() before any
 * actual firmware loading and is undone only in k3_dsp_stop(). The local reset
 * on C71x cores is a no-op and the global reset cannot be released on C71x
 * cores until after the firmware images are loaded, so this function does
 * nothing for C71x cores.
 */
static int k3_dsp_prepare(struct udevice *dev)
{
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);
	struct k3_dsp_boot_data *data = dsp->data;
	int ret;

	/* local reset is no-op on C71x processors */
	if (!data->uses_lreset)
		return 0;

	ret = ti_sci_proc_power_domain_on(&dsp->tsp);
	if (ret)
		dev_err(dev, "cannot enable internal RAM loading, ret = %d\n",
			ret);

	return ret;
}

/*
 * This function is the counterpart to k3_dsp_prepare() and is used to assert
 * the global reset on C66x DSP cores (no-op for C71x DSP cores). This completes
 * the second step of powering down the C66x DSP cores. The cores themselves
 * are halted through the local reset in first step. This function is invoked
 * in k3_dsp_stop() after the local reset is asserted.
 */
static int k3_dsp_unprepare(struct udevice *dev)
{
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);
	struct k3_dsp_boot_data *data = dsp->data;

	/* local reset is no-op on C71x processors */
	if (!data->uses_lreset)
		return 0;

	return ti_sci_proc_power_domain_off(&dsp->tsp);
}

/**
 * k3_dsp_load() - Load up the Remote processor image
 * @dev:	rproc device pointer
 * @addr:	Address at which image is available
 * @size:	size of the image
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_dsp_load(struct udevice *dev, ulong addr, ulong size)
{
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);
	struct k3_dsp_boot_data *data = dsp->data;
	u32 boot_vector;
	int ret;

	if (dsp->in_use) {
		dev_err(dev,
			"Invalid op: Trying to load/start on already running core %d\n",
			dsp->tsp.proc_id);
		return -EINVAL;
	}

	dev_dbg(dev, "%s addr = 0x%lx, size = 0x%lx\n", __func__, addr, size);
	ret = ti_sci_proc_request(&dsp->tsp);
	if (ret)
		return ret;

	ret = k3_dsp_prepare(dev);
	if (ret) {
		dev_err(dev, "DSP prepare failed for core %d\n",
			dsp->tsp.proc_id);
		goto proc_release;
	}

	ret = rproc_elf_load_image(dev, addr, size);
	if (ret < 0) {
		dev_err(dev, "Loading elf failed %d\n", ret);
		goto unprepare;
	}

	boot_vector = rproc_elf_get_boot_addr(dev, addr);
	if (boot_vector & (data->boot_align_addr - 1)) {
		ret = -EINVAL;
		dev_err(dev, "Boot vector 0x%x not aligned on 0x%x boundary\n",
			boot_vector, data->boot_align_addr);
		goto proc_release;
	}

	dev_dbg(dev, "%s: Boot vector = 0x%x\n", __func__, boot_vector);

	ret = ti_sci_proc_set_config(&dsp->tsp, boot_vector, 0, 0);
unprepare:
	if (ret)
		k3_dsp_unprepare(dev);
proc_release:
	ti_sci_proc_release(&dsp->tsp);
	return ret;
}

/**
 * k3_dsp_start() - Start the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_dsp_start(struct udevice *dev)
{
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);
	struct k3_dsp_boot_data *data = dsp->data;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = ti_sci_proc_request(&dsp->tsp);
	if (ret)
		return ret;

	if (!data->uses_lreset) {
		ret = ti_sci_proc_power_domain_on(&dsp->tsp);
		if (ret)
			goto proc_release;
	}

	ret = reset_deassert(&dsp->dsp_rst);
	if (ret) {
		if (!data->uses_lreset)
			ti_sci_proc_power_domain_off(&dsp->tsp);
	}

	dsp->in_use = true;
proc_release:
	ti_sci_proc_release(&dsp->tsp);

	return ret;
}

static int k3_dsp_stop(struct udevice *dev)
{
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);

	dev_dbg(dev, "%s\n", __func__);

	dsp->in_use = false;
	ti_sci_proc_request(&dsp->tsp);
	reset_assert(&dsp->dsp_rst);
	ti_sci_proc_power_domain_off(&dsp->tsp);
	ti_sci_proc_release(&dsp->tsp);

	return 0;
}

/**
 * k3_dsp_init() - Initialize the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_dsp_init(struct udevice *dev)
{
	dev_dbg(dev, "%s\n", __func__);

	return 0;
}

static int k3_dsp_reset(struct udevice *dev)
{
	dev_dbg(dev, "%s\n", __func__);

	return 0;
}

static void *k3_dsp_da_to_va(struct udevice *dev, ulong da, ulong len)
{
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);
	phys_addr_t bus_addr, dev_addr;
	void __iomem *va = NULL;
	size_t size;
	u32 offset;
	int i;

	dev_dbg(dev, "%s\n", __func__);

	if (len <= 0)
		return NULL;

	for (i = 0; i < dsp->num_mems; i++) {
		bus_addr = dsp->mem[i].bus_addr;
		dev_addr = dsp->mem[i].dev_addr;
		size = dsp->mem[i].size;

		if (da >= dev_addr && ((da + len) <= (dev_addr + size))) {
			offset = da - dev_addr;
			va = dsp->mem[i].cpu_addr + offset;
			return (__force void *)va;
		}

		if (da >= bus_addr && (da + len) <= (bus_addr + size)) {
			offset = da - bus_addr;
			va = dsp->mem[i].cpu_addr + offset;
			return (__force void *)va;
		}
	}

	/* Assume it is DDR region and return da */
	return map_physmem(da, len, MAP_NOCACHE);
}

static const struct dm_rproc_ops k3_dsp_ops = {
	.init = k3_dsp_init,
	.load = k3_dsp_load,
	.start = k3_dsp_start,
	.stop = k3_dsp_stop,
	.reset = k3_dsp_reset,
	.device_to_virt = k3_dsp_da_to_va,
};

static int ti_sci_proc_of_to_priv(struct udevice *dev, struct ti_sci_proc *tsp)
{
	u32 ids[2];
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	tsp->sci = ti_sci_get_by_phandle(dev, "ti,sci");
	if (IS_ERR(tsp->sci)) {
		dev_err(dev, "ti_sci get failed: %ld\n", PTR_ERR(tsp->sci));
		return PTR_ERR(tsp->sci);
	}

	ret = dev_read_u32_array(dev, "ti,sci-proc-ids", ids, 2);
	if (ret) {
		dev_err(dev, "Proc IDs not populated %d\n", ret);
		return ret;
	}

	tsp->ops = &tsp->sci->ops.proc_ops;
	tsp->proc_id = ids[0];
	tsp->host_id = ids[1];
	tsp->dev_id = dev_read_u32_default(dev, "ti,sci-dev-id",
					   TI_SCI_RESOURCE_NULL);
	if (tsp->dev_id == TI_SCI_RESOURCE_NULL) {
		dev_err(dev, "Device ID not populated %d\n", ret);
		return -ENODEV;
	}

	return 0;
}

static int k3_dsp_of_get_memories(struct udevice *dev)
{
	static const char * const mem_names[] = {"l2sram", "l1pram", "l1dram"};
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);
	int i;

	dev_dbg(dev, "%s\n", __func__);

	dsp->num_mems = ARRAY_SIZE(mem_names);
	dsp->mem = calloc(dsp->num_mems, sizeof(*dsp->mem));
	if (!dsp->mem)
		return -ENOMEM;

	for (i = 0; i < dsp->num_mems; i++) {
		/* C71 cores only have a L1P Cache, there are no L1P SRAMs */
		if (device_is_compatible(dev, "ti,j721e-c71-dsp") &&
		    !strcmp(mem_names[i], "l1pram")) {
			dsp->mem[i].bus_addr = FDT_ADDR_T_NONE;
			dsp->mem[i].dev_addr = FDT_ADDR_T_NONE;
			dsp->mem[i].cpu_addr = NULL;
			dsp->mem[i].size = 0;
			continue;
		}

		dsp->mem[i].bus_addr = dev_read_addr_size_name(dev, mem_names[i],
					  (fdt_addr_t *)&dsp->mem[i].size);
		if (dsp->mem[i].bus_addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "%s bus address not found\n", mem_names[i]);
			return -EINVAL;
		}
		dsp->mem[i].cpu_addr = map_physmem(dsp->mem[i].bus_addr,
						   dsp->mem[i].size,
						   MAP_NOCACHE);
		dsp->mem[i].dev_addr = dsp->mem[i].bus_addr &
					KEYSTONE_RPROC_LOCAL_ADDRESS_MASK;

		dev_dbg(dev, "memory %8s: bus addr %pa size 0x%zx va %p da %pa\n",
			mem_names[i], &dsp->mem[i].bus_addr,
			dsp->mem[i].size, dsp->mem[i].cpu_addr,
			&dsp->mem[i].dev_addr);
	}

	return 0;
}

/**
 * k3_of_to_priv() - generate private data from device tree
 * @dev:	corresponding k3 dsp processor device
 * @dsp:	pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_dsp_of_to_priv(struct udevice *dev, struct k3_dsp_privdata *dsp)
{
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = reset_get_by_index(dev, 0, &dsp->dsp_rst);
	if (ret) {
		dev_err(dev, "reset_get() failed: %d\n", ret);
		return ret;
	}

	ret = ti_sci_proc_of_to_priv(dev, &dsp->tsp);
	if (ret)
		return ret;

	ret =  k3_dsp_of_get_memories(dev);
	if (ret)
		return ret;

	dsp->data = (struct k3_dsp_boot_data *)dev_get_driver_data(dev);

	return 0;
}

/**
 * k3_dsp_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_dsp_probe(struct udevice *dev)
{
	struct k3_dsp_privdata *dsp;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	dsp = dev_get_priv(dev);

	ret = k3_dsp_of_to_priv(dev, dsp);
	if (ret) {
		dev_dbg(dev, "%s: Probe failed with error %d\n", __func__, ret);
		return ret;
	}

	/*
	 * The DSP local resets are deasserted by default on Power-On-Reset.
	 * Assert the local resets to ensure the DSPs don't execute bogus code
	 * in .load() callback when the module reset is released to support
	 * internal memory loading. This is needed for C66x DSPs, and is a
	 * no-op on C71x DSPs.
	 */
	reset_assert(&dsp->dsp_rst);

	dev_dbg(dev, "Remoteproc successfully probed\n");

	return 0;
}

static int k3_dsp_remove(struct udevice *dev)
{
	struct k3_dsp_privdata *dsp = dev_get_priv(dev);

	free(dsp->mem);

	return 0;
}

static const struct k3_dsp_boot_data c66_data = {
	.boot_align_addr = SZ_1K,
	.uses_lreset = true,
};

static const struct k3_dsp_boot_data c71_data = {
	.boot_align_addr = SZ_2M,
	.uses_lreset = false,
};

static const struct udevice_id k3_dsp_ids[] = {
	{ .compatible = "ti,j721e-c66-dsp", .data = (ulong)&c66_data, },
	{ .compatible = "ti,j721e-c71-dsp", .data = (ulong)&c71_data, },
	{}
};

U_BOOT_DRIVER(k3_dsp) = {
	.name = "k3_dsp",
	.of_match = k3_dsp_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &k3_dsp_ops,
	.probe = k3_dsp_probe,
	.remove = k3_dsp_remove,
	.priv_auto	= sizeof(struct k3_dsp_privdata),
};
