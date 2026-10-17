// SPDX-License-Identifier: GPL-2.0+ OR X11
/*
 * Copyright 2018-2021 NXP
 *
 * PCIe Gen4 driver for NXP Layerscape SoCs
 * Author: Hou Zhiqiang <Minder.Hou@gmail.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/io.h>
#include <errno.h>
#ifdef CONFIG_OF_BOARD_SETUP
#include <linux/libfdt.h>
#include <fdt_support.h>
#ifdef CONFIG_ARM
#include <asm/arch/clock.h>
#endif
#include "pcie_layerscape_gen4.h"
#include "pcie_layerscape_fixup_common.h"

#if defined(CONFIG_FSL_LSCH3) || defined(CONFIG_FSL_LSCH2)
/*
 * Return next available LUT index.
 */
static int ls_pcie_g4_next_lut_index(struct ls_pcie_g4 *pcie)
{
	if (pcie->next_lut_index < PCIE_LUT_ENTRY_COUNT)
		return pcie->next_lut_index++;

	return -ENOSPC;  /* LUT is full */
}

/*
 * Program a single LUT entry
 */
static void ls_pcie_g4_lut_set_mapping(struct ls_pcie_g4 *pcie, int index,
				       u32 devid, u32 streamid)
{
	/* leave mask as all zeroes, want to match all bits */
	lut_writel(pcie, devid << 16, PCIE_LUT_UDR(index));
	lut_writel(pcie, streamid | PCIE_LUT_ENABLE, PCIE_LUT_LDR(index));
}

/*
 * An msi-map is a property to be added to the pci controller
 * node.  It is a table, where each entry consists of 4 fields
 * e.g.:
 *
 *      msi-map = <[devid] [phandle-to-msi-ctrl] [stream-id] [count]
 *                 [devid] [phandle-to-msi-ctrl] [stream-id] [count]>;
 */
static void fdt_pcie_set_msi_map_entry_ls_gen4(void *blob,
					       struct ls_pcie_g4 *pcie,
					       u32 devid, u32 streamid)
{
	u32 *prop;
	u32 phandle;
	int nodeoff;

#ifdef CONFIG_FSL_PCIE_COMPAT
	nodeoff = fdt_node_offset_by_compat_reg(blob, CONFIG_FSL_PCIE_COMPAT,
						pcie->ccsr_res.start);
#else
#error "No CONFIG_FSL_PCIE_COMPAT defined"
#endif
	if (nodeoff < 0) {
		debug("%s: ERROR: failed to find pcie compatiable\n", __func__);
		return;
	}

	/* get phandle to MSI controller */
	prop = (u32 *)fdt_getprop(blob, nodeoff, "msi-parent", 0);
	if (!prop) {
		debug("\n%s: ERROR: missing msi-parent: PCIe%d\n",
		      __func__, pcie->idx);
		return;
	}
	phandle = fdt32_to_cpu(*prop);

	/* set one msi-map row */
	fdt_appendprop_u32(blob, nodeoff, "msi-map", devid);
	fdt_appendprop_u32(blob, nodeoff, "msi-map", phandle);
	fdt_appendprop_u32(blob, nodeoff, "msi-map", streamid);
	fdt_appendprop_u32(blob, nodeoff, "msi-map", 1);
}

/*
 * An iommu-map is a property to be added to the pci controller
 * node.  It is a table, where each entry consists of 4 fields
 * e.g.:
 *
 *      iommu-map = <[devid] [phandle-to-iommu-ctrl] [stream-id] [count]
 *                 [devid] [phandle-to-iommu-ctrl] [stream-id] [count]>;
 */
static void fdt_pcie_set_iommu_map_entry_ls_gen4(void *blob,
						 struct ls_pcie_g4 *pcie,
						 u32 devid, u32 streamid)
{
	u32 *prop;
	u32 iommu_map[4];
	int nodeoff;
	int lenp;

#ifdef CONFIG_FSL_PCIE_COMPAT
	nodeoff = fdt_node_offset_by_compat_reg(blob, CONFIG_FSL_PCIE_COMPAT,
						pcie->ccsr_res.start);
#else
#error "No CONFIG_FSL_PCIE_COMPAT defined"
#endif
	if (nodeoff < 0) {
		debug("%s: ERROR: failed to find pcie compatiable\n", __func__);
		return;
	}

	/* get phandle to iommu controller */
	prop = fdt_getprop_w(blob, nodeoff, "iommu-map", &lenp);
	if (!prop) {
		debug("\n%s: ERROR: missing iommu-map: PCIe%d\n",
		      __func__, pcie->idx);
		return;
	}

	/* set iommu-map row */
	iommu_map[0] = cpu_to_fdt32(devid);
	iommu_map[1] = *++prop;
	iommu_map[2] = cpu_to_fdt32(streamid);
	iommu_map[3] = cpu_to_fdt32(1);

	if (devid == 0)
		fdt_setprop_inplace(blob, nodeoff, "iommu-map", iommu_map, 16);
	else
		fdt_appendprop(blob, nodeoff, "iommu-map", iommu_map, 16);
}

static void fdt_fixup_pcie_ls_gen4(void *blob)
{
	struct udevice *dev, *bus;
	struct ls_pcie_g4 *pcie;
	int streamid;
	int index;
	pci_dev_t bdf;

	/* Scan all known buses */
	for (pci_find_first_device(&dev); dev; pci_find_next_device(&dev)) {
		for (bus = dev; device_is_on_pci_bus(bus);)
			bus = bus->parent;
		pcie = dev_get_priv(bus);

		streamid = pcie_next_streamid(pcie->stream_id_cur, pcie->idx);
		if (streamid < 0) {
			debug("ERROR: no stream ids free\n");
			continue;
		} else {
			pcie->stream_id_cur++;
		}

		index = ls_pcie_g4_next_lut_index(pcie);
		if (index < 0) {
			debug("ERROR: no LUT indexes free\n");
			continue;
		}

		/* the DT fixup must be relative to the hose first_busno */
		bdf = dm_pci_get_bdf(dev) - PCI_BDF(dev_seq(bus), 0, 0);
		/* map PCI b.d.f to streamID in LUT */
		ls_pcie_g4_lut_set_mapping(pcie, index, bdf >> 8, streamid);
		/* update msi-map in device tree */
		fdt_pcie_set_msi_map_entry_ls_gen4(blob, pcie, bdf >> 8,
						   streamid);
		/* update iommu-map in device tree */
		fdt_pcie_set_iommu_map_entry_ls_gen4(blob, pcie, bdf >> 8,
						     streamid);
	}
}
#endif

static void ft_pcie_ep_layerscape_gen4_fix(void *blob, struct ls_pcie_g4 *pcie)
{
	int off;

	off = fdt_node_offset_by_compat_reg(blob, CONFIG_FSL_PCIE_EP_COMPAT,
					    pcie->ccsr_res.start);

	if (off < 0) {
		debug("%s: ERROR: failed to find pcie compatiable\n",
		      __func__);
		return;
	}

	if (pcie->enabled && pcie->mode == PCI_HEADER_TYPE_NORMAL)
		fdt_set_node_status(blob, off, FDT_STATUS_OKAY);
	else
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
}

static void ft_pcie_rc_layerscape_gen4_fix(void *blob, struct ls_pcie_g4 *pcie)
{
	int off;

#ifdef CONFIG_FSL_PCIE_COMPAT
	off = fdt_node_offset_by_compat_reg(blob, CONFIG_FSL_PCIE_COMPAT,
					    pcie->ccsr_res.start);
#else
#error "No CONFIG_FSL_PCIE_COMPAT defined"
#endif
	if (off < 0) {
		debug("%s: ERROR: failed to find pcie compatiable\n", __func__);
		return;
	}

	if (pcie->enabled && pcie->mode == PCI_HEADER_TYPE_BRIDGE)
		fdt_set_node_status(blob, off, FDT_STATUS_OKAY);
	else
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
}

static void ft_pcie_layerscape_gen4_setup(void *blob, struct ls_pcie_g4 *pcie)
{
	ft_pcie_rc_layerscape_gen4_fix(blob, pcie);
	ft_pcie_ep_layerscape_gen4_fix(blob, pcie);

	pcie->stream_id_cur = 0;
	pcie->next_lut_index = 0;
}

/* Fixup Kernel DT for PCIe */
void ft_pci_setup_ls_gen4(void *blob, struct bd_info *bd)
{
	struct ls_pcie_g4 *pcie;

	list_for_each_entry(pcie, &ls_pcie_g4_list, list)
		ft_pcie_layerscape_gen4_setup(blob, pcie);

#if defined(CONFIG_FSL_LSCH3) || defined(CONFIG_FSL_LSCH2)
	fdt_fixup_pcie_ls_gen4(blob);
#endif
}

#else /* !CONFIG_OF_BOARD_SETUP */
void ft_pci_setup_ls_gen4(void *blob, struct bd_info *bd)
{
}
#endif
