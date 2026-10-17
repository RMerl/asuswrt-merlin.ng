// SPDX-License-Identifier: GPL-2.0
/*
 * PRU-ICSS platform driver for various TI SoCs
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <common.h>
#include <dm.h>
#include <dm/of_access.h>
#include <errno.h>
#include <clk.h>
#include <reset.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <power-domain.h>
#include <linux/pruss_driver.h>
#include <dm/device_compat.h>

#define PRUSS_CFG_IEPCLK	0x30
#define ICSSG_CFG_CORE_SYNC	0x3c

#define ICSSG_TASK_MGR_OFFSET	0x2a000

/* PRUSS_IEPCLK register bits */
#define PRUSS_IEPCLK_IEP_OCP_CLK_EN		BIT(0)

/* ICSSG CORE_SYNC register bits */
#define ICSSG_CORE_VBUSP_SYNC_EN		BIT(0)

/*
 * pruss_request_tm_region() - Request pruss for task manager region
 * @dev:	corresponding k3 device
 * @loc:	the task manager physical address
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
int pruss_request_tm_region(struct udevice *dev, phys_addr_t *loc)
{
	struct pruss *priv;

	priv = dev_get_priv(dev);
	if (!priv || !priv->mem_regions[PRUSS_MEM_DRAM0].pa)
		return -EINVAL;

	*loc = priv->mem_regions[PRUSS_MEM_DRAM0].pa + ICSSG_TASK_MGR_OFFSET;

	return 0;
}

/**
 * pruss_request_mem_region() - request a memory resource
 * @dev: the pruss device
 * @mem_id: the memory resource id
 * @region: pointer to memory region structure to be filled in
 *
 * This function allows a client driver to request a memory resource,
 * and if successful, will let the client driver own the particular
 * memory region until released using the pruss_release_mem_region()
 * API.
 *
 * Returns the memory region if requested resource is available, an
 * error otherwise
 */
int pruss_request_mem_region(struct udevice *dev, enum pruss_mem mem_id,
			     struct pruss_mem_region *region)
{
	struct pruss *pruss;

	pruss = dev_get_priv(dev);
	if (!pruss || !region)
		return -EINVAL;

	if (mem_id >= PRUSS_MEM_MAX)
		return -EINVAL;

	if (pruss->mem_in_use[mem_id])
		return -EBUSY;

	*region = pruss->mem_regions[mem_id];
	pruss->mem_in_use[mem_id] = region;

	return 0;
}

/**
 * pruss_release_mem_region() - release a memory resource
 * @dev: the pruss device
 * @region: the memory region to release
 *
 * This function is the complimentary function to
 * pruss_request_mem_region(), and allows the client drivers to
 * release back a memory resource.
 *
 * Returns 0 on success, an error code otherwise
 */
int pruss_release_mem_region(struct udevice *dev,
			     struct pruss_mem_region *region)
{
	struct pruss *pruss;
	int id;

	pruss = dev_get_priv(dev);
	if (!pruss || !region)
		return -EINVAL;

	/* find out the memory region being released */
	for (id = 0; id < PRUSS_MEM_MAX; id++) {
		if (pruss->mem_in_use[id] == region)
			break;
	}

	if (id == PRUSS_MEM_MAX)
		return -EINVAL;

	pruss->mem_in_use[id] = NULL;

	return 0;
}

/**
 * pruss_cfg_update() - configure a PRUSS CFG sub-module register
 * @dev: the pruss device
 * @reg: register offset within the CFG sub-module
 * @mask: bit mask to use for programming the @val
 * @val: value to write
 *
 * Programs a given register within the PRUSS CFG sub-module
 *
 * Returns 0 on success, or an error code otherwise
 */
int pruss_cfg_update(struct udevice *dev, unsigned int reg,
		     unsigned int mask, unsigned int val)
{
	struct pruss *pruss;

	pruss = dev_get_priv(dev);
	if (IS_ERR_OR_NULL(pruss))
		return -EINVAL;

	return regmap_update_bits(pruss->cfg, reg, mask, val);
}

/**
 * pruss_probe() - Basic probe
 * @dev:	corresponding k3 device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int pruss_probe(struct udevice *dev)
{
	const char *mem_names[PRUSS_MEM_MAX] = { "dram0", "dram1", "shrdram2" };
	ofnode sub_node, node, memories;
	struct udevice *syscon;
	struct pruss *priv;
	int ret, idx, i;

	priv = dev_get_priv(dev);
	node = dev_ofnode(dev);
	priv->dev = dev;
	memories = ofnode_find_subnode(node, "memories");

	for (i = 0; i < ARRAY_SIZE(mem_names); i++) {
		idx = ofnode_stringlist_search(memories, "reg-names", mem_names[i]);
		priv->mem_regions[i].pa = ofnode_get_addr_size_index(memories, idx,
						       (u64 *)&priv->mem_regions[i].size);
	}

	sub_node = ofnode_find_subnode(node, "cfg");
	ret = uclass_get_device_by_ofnode(UCLASS_SYSCON, sub_node,
					  &syscon);

	priv->cfg = syscon_get_regmap(syscon);
	if (IS_ERR(priv->cfg)) {
		dev_err(dev, "unable to get cfg regmap (%ld)\n",
			PTR_ERR(priv->cfg));
		return -ENODEV;
	}

	/*
	 * ToDo: To be modelled as clocks.
	 * The CORE block uses two multiplexers to allow software to
	 * select one of three source clocks (ICSSGn_CORE_CLK, ICSSGn_ICLK or
	 * ICSSGn_IEP_CLK) for the final clock source of the CORE block.
	 * The user needs to configure ICSSG_CORE_SYNC_REG[0] CORE_VBUSP_SYNC_EN
	 * bit & ICSSG_IEPCLK_REG[0] IEP_OCP_CLK_EN bit in order to select the
	 * clock source to the CORE block.
	 */
	ret = regmap_update_bits(priv->cfg, ICSSG_CFG_CORE_SYNC,
				 ICSSG_CORE_VBUSP_SYNC_EN,
				 ICSSG_CORE_VBUSP_SYNC_EN);
	if (ret)
		return ret;
	ret = regmap_update_bits(priv->cfg, PRUSS_CFG_IEPCLK,
				 PRUSS_IEPCLK_IEP_OCP_CLK_EN,
				 PRUSS_IEPCLK_IEP_OCP_CLK_EN);
	if (ret)
		return ret;

	dev_dbg(dev, "pruss successfully probed %s\n", dev->name);

	return 0;
}

static const struct udevice_id pruss_ids[] = {
	{ .compatible = "ti,am654-icssg"},
	{}
};

U_BOOT_DRIVER(pruss) = {
	.name = "pruss",
	.of_match = pruss_ids,
	.id = UCLASS_MISC,
	.probe = pruss_probe,
	.priv_auto = sizeof(struct pruss),
};
