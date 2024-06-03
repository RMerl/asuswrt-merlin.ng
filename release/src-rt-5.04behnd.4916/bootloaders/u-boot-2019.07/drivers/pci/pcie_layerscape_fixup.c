// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 */

#include <common.h>
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
#include "pcie_layerscape.h"

#if defined(CONFIG_FSL_LSCH3) || defined(CONFIG_FSL_LSCH2)
/*
 * Return next available LUT index.
 */
static int ls_pcie_next_lut_index(struct ls_pcie *pcie)
{
	if (pcie->next_lut_index < PCIE_LUT_ENTRY_COUNT)
		return pcie->next_lut_index++;
	else
		return -ENOSPC;  /* LUT is full */
}

/* returns the next available streamid for pcie, -errno if failed */
static int ls_pcie_next_streamid(void)
{
	static int next_stream_id = FSL_PEX_STREAM_ID_START;

	if (next_stream_id > FSL_PEX_STREAM_ID_END)
		return -EINVAL;

	return next_stream_id++;
}

static void lut_writel(struct ls_pcie *pcie, unsigned int value,
		       unsigned int offset)
{
	if (pcie->big_endian)
		out_be32(pcie->lut + offset, value);
	else
		out_le32(pcie->lut + offset, value);
}

/*
 * Program a single LUT entry
 */
static void ls_pcie_lut_set_mapping(struct ls_pcie *pcie, int index, u32 devid,
				    u32 streamid)
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
static void fdt_pcie_set_msi_map_entry(void *blob, struct ls_pcie *pcie,
				       u32 devid, u32 streamid)
{
	u32 *prop;
	u32 phandle;
	int nodeoffset;
	uint svr;
	char *compat = NULL;

	/* find pci controller node */
	nodeoffset = fdt_node_offset_by_compat_reg(blob, "fsl,ls-pcie",
						   pcie->dbi_res.start);
	if (nodeoffset < 0) {
#ifdef CONFIG_FSL_PCIE_COMPAT /* Compatible with older version of dts node */
		svr = (get_svr() >> SVR_VAR_PER_SHIFT) & 0xFFFFFE;
		if (svr == SVR_LS2088A || svr == SVR_LS2084A ||
		    svr == SVR_LS2048A || svr == SVR_LS2044A ||
		    svr == SVR_LS2081A || svr == SVR_LS2041A)
			compat = "fsl,ls2088a-pcie";
		else
			compat = CONFIG_FSL_PCIE_COMPAT;
		if (compat)
			nodeoffset = fdt_node_offset_by_compat_reg(blob,
					compat, pcie->dbi_res.start);
#endif
		if (nodeoffset < 0)
			return;
	}

	/* get phandle to MSI controller */
	prop = (u32 *)fdt_getprop(blob, nodeoffset, "msi-parent", 0);
	if (prop == NULL) {
		debug("\n%s: ERROR: missing msi-parent: PCIe%d\n",
		      __func__, pcie->idx);
		return;
	}
	phandle = fdt32_to_cpu(*prop);

	/* set one msi-map row */
	fdt_appendprop_u32(blob, nodeoffset, "msi-map", devid);
	fdt_appendprop_u32(blob, nodeoffset, "msi-map", phandle);
	fdt_appendprop_u32(blob, nodeoffset, "msi-map", streamid);
	fdt_appendprop_u32(blob, nodeoffset, "msi-map", 1);
}

/*
 * An iommu-map is a property to be added to the pci controller
 * node.  It is a table, where each entry consists of 4 fields
 * e.g.:
 *
 *      iommu-map = <[devid] [phandle-to-iommu-ctrl] [stream-id] [count]
 *                 [devid] [phandle-to-iommu-ctrl] [stream-id] [count]>;
 */
static void fdt_pcie_set_iommu_map_entry(void *blob, struct ls_pcie *pcie,
				       u32 devid, u32 streamid)
{
	u32 *prop;
	u32 iommu_map[4];
	int nodeoffset;
	int lenp;
	uint svr;
	char *compat = NULL;

	/* find pci controller node */
	nodeoffset = fdt_node_offset_by_compat_reg(blob, "fsl,ls-pcie",
						   pcie->dbi_res.start);
	if (nodeoffset < 0) {
#ifdef CONFIG_FSL_PCIE_COMPAT /* Compatible with older version of dts node */
		svr = (get_svr() >> SVR_VAR_PER_SHIFT) & 0xFFFFFE;
		if (svr == SVR_LS2088A || svr == SVR_LS2084A ||
		    svr == SVR_LS2048A || svr == SVR_LS2044A ||
		    svr == SVR_LS2081A || svr == SVR_LS2041A)
			compat = "fsl,ls2088a-pcie";
		else
			compat = CONFIG_FSL_PCIE_COMPAT;

		if (compat)
			nodeoffset = fdt_node_offset_by_compat_reg(blob,
						compat, pcie->dbi_res.start);
#endif
		if (nodeoffset < 0)
			return;
	}

	/* get phandle to iommu controller */
	prop = fdt_getprop_w(blob, nodeoffset, "iommu-map", &lenp);
	if (prop == NULL) {
		debug("\n%s: ERROR: missing iommu-map: PCIe%d\n",
		      __func__, pcie->idx);
		return;
	}

	/* set iommu-map row */
	iommu_map[0] = cpu_to_fdt32(devid);
	iommu_map[1] = *++prop;
	iommu_map[2] = cpu_to_fdt32(streamid);
	iommu_map[3] = cpu_to_fdt32(1);

	if (devid == 0) {
		fdt_setprop_inplace(blob, nodeoffset, "iommu-map",
				    iommu_map, 16);
	} else {
		fdt_appendprop(blob, nodeoffset, "iommu-map", iommu_map, 16);
	}
}

static void fdt_fixup_pcie(void *blob)
{
	struct udevice *dev, *bus;
	struct ls_pcie *pcie;
	int streamid;
	int index;
	pci_dev_t bdf;

	/* Scan all known buses */
	for (pci_find_first_device(&dev);
	     dev;
	     pci_find_next_device(&dev)) {
		for (bus = dev; device_is_on_pci_bus(bus);)
			bus = bus->parent;
		pcie = dev_get_priv(bus);

		streamid = ls_pcie_next_streamid();
		if (streamid < 0) {
			debug("ERROR: no stream ids free\n");
			continue;
		}

		index = ls_pcie_next_lut_index(pcie);
		if (index < 0) {
			debug("ERROR: no LUT indexes free\n");
			continue;
		}

		/* the DT fixup must be relative to the hose first_busno */
		bdf = dm_pci_get_bdf(dev) - PCI_BDF(bus->seq, 0, 0);
		/* map PCI b.d.f to streamID in LUT */
		ls_pcie_lut_set_mapping(pcie, index, bdf >> 8,
					streamid);
		/* update msi-map in device tree */
		fdt_pcie_set_msi_map_entry(blob, pcie, bdf >> 8,
					   streamid);
		/* update iommu-map in device tree */
		fdt_pcie_set_iommu_map_entry(blob, pcie, bdf >> 8,
					     streamid);
	}
}
#endif

static void ft_pcie_rc_fix(void *blob, struct ls_pcie *pcie)
{
	int off;
	uint svr;
	char *compat = NULL;

	off = fdt_node_offset_by_compat_reg(blob, "fsl,ls-pcie",
					    pcie->dbi_res.start);
	if (off < 0) {
#ifdef CONFIG_FSL_PCIE_COMPAT /* Compatible with older version of dts node */
		svr = (get_svr() >> SVR_VAR_PER_SHIFT) & 0xFFFFFE;
		if (svr == SVR_LS2088A || svr == SVR_LS2084A ||
		    svr == SVR_LS2048A || svr == SVR_LS2044A ||
		    svr == SVR_LS2081A || svr == SVR_LS2041A)
			compat = "fsl,ls2088a-pcie";
		else
			compat = CONFIG_FSL_PCIE_COMPAT;
		if (compat)
			off = fdt_node_offset_by_compat_reg(blob,
					compat, pcie->dbi_res.start);
#endif
		if (off < 0)
			return;
	}

	if (pcie->enabled && pcie->mode == PCI_HEADER_TYPE_BRIDGE)
		fdt_set_node_status(blob, off, FDT_STATUS_OKAY, 0);
	else
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);
}

static void ft_pcie_ep_fix(void *blob, struct ls_pcie *pcie)
{
	int off;

	off = fdt_node_offset_by_compat_reg(blob, "fsl,ls-pcie-ep",
					    pcie->dbi_res.start);
	if (off < 0)
		return;

	if (pcie->enabled && pcie->mode == PCI_HEADER_TYPE_NORMAL)
		fdt_set_node_status(blob, off, FDT_STATUS_OKAY, 0);
	else
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);
}

static void ft_pcie_ls_setup(void *blob, struct ls_pcie *pcie)
{
	ft_pcie_ep_fix(blob, pcie);
	ft_pcie_rc_fix(blob, pcie);
}

/* Fixup Kernel DT for PCIe */
void ft_pci_setup(void *blob, bd_t *bd)
{
	struct ls_pcie *pcie;

	list_for_each_entry(pcie, &ls_pcie_list, list)
		ft_pcie_ls_setup(blob, pcie);

#if defined(CONFIG_FSL_LSCH3) || defined(CONFIG_FSL_LSCH2)
	fdt_fixup_pcie(blob);
#endif
}

#else /* !CONFIG_OF_BOARD_SETUP */
void ft_pci_setup(void *blob, bd_t *bd)
{
}
#endif
