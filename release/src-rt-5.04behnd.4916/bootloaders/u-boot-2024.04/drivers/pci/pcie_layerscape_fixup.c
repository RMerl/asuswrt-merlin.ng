// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2021 NXP
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 */

#include <common.h>
#include <dm.h>
#include <init.h>
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
#include <malloc.h>
#include <env.h>
#include "pcie_layerscape.h"
#include "pcie_layerscape_fixup_common.h"

int next_stream_id;

static int fdt_pcie_get_nodeoffset(void *blob, struct ls_pcie_rc *pcie_rc)
{
	int nodeoffset;
	uint svr;
	char *compat = NULL;

	/* find pci controller node */
	nodeoffset = fdt_node_offset_by_compat_reg(blob, "fsl,ls-pcie",
						   pcie_rc->dbi_res.start);
	if (nodeoffset < 0) {
#ifdef CONFIG_FSL_PCIE_COMPAT /* Compatible with older version of dts node */
		svr = (get_svr() >> SVR_VAR_PER_SHIFT) & 0xFFFFFE;
		if (svr == SVR_LS2088A || svr == SVR_LS2084A ||
		    svr == SVR_LS2048A || svr == SVR_LS2044A ||
		    svr == SVR_LS2081A || svr == SVR_LS2041A)
			compat = "fsl,ls2088a-pcie";
		else
			compat = CONFIG_FSL_PCIE_COMPAT;

		nodeoffset =
			fdt_node_offset_by_compat_reg(blob, compat,
						      pcie_rc->dbi_res.start);
#endif
	}

	return nodeoffset;
}

#if defined(CONFIG_FSL_LSCH3) || defined(CONFIG_FSL_LSCH2)
/*
 * Return next available LUT index.
 */
static int ls_pcie_next_lut_index(struct ls_pcie_rc *pcie_rc)
{
	if (pcie_rc->next_lut_index < PCIE_LUT_ENTRY_COUNT)
		return pcie_rc->next_lut_index++;
	else
		return -ENOSPC;  /* LUT is full */
}

static void lut_writel(struct ls_pcie_rc *pcie_rc, unsigned int value,
		       unsigned int offset)
{
	struct ls_pcie *pcie = pcie_rc->pcie;

	if (pcie->big_endian)
		out_be32(pcie->lut + offset, value);
	else
		out_le32(pcie->lut + offset, value);
}

/*
 * Program a single LUT entry
 */
static void ls_pcie_lut_set_mapping(struct ls_pcie_rc *pcie_rc, int index,
				    u32 devid, u32 streamid)
{
	/* leave mask as all zeroes, want to match all bits */
	lut_writel(pcie_rc, devid << 16, PCIE_LUT_UDR(index));
	lut_writel(pcie_rc, streamid | PCIE_LUT_ENABLE, PCIE_LUT_LDR(index));
}

/*
 * An msi-map is a property to be added to the pci controller
 * node.  It is a table, where each entry consists of 4 fields
 * e.g.:
 *
 *      msi-map = <[devid] [phandle-to-msi-ctrl] [stream-id] [count]
 *                 [devid] [phandle-to-msi-ctrl] [stream-id] [count]>;
 */
static void fdt_pcie_set_msi_map_entry_ls(void *blob,
					  struct ls_pcie_rc *pcie_rc,
					  u32 devid, u32 streamid)
{
	u32 *prop;
	u32 phandle;
	int nodeoffset;
	uint svr;
	char *compat = NULL;
	struct ls_pcie *pcie = pcie_rc->pcie;

	/* find pci controller node */
	nodeoffset = fdt_node_offset_by_compat_reg(blob, "fsl,ls-pcie",
						   pcie_rc->dbi_res.start);
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
					compat, pcie_rc->dbi_res.start);
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
static void fdt_pcie_set_iommu_map_entry_ls(void *blob,
					    struct ls_pcie_rc *pcie_rc,
					    u32 devid, u32 streamid)
{
	u32 *prop;
	u32 iommu_map[4];
	int nodeoffset;
	int lenp;
	struct ls_pcie *pcie = pcie_rc->pcie;

	nodeoffset = fdt_pcie_get_nodeoffset(blob, pcie_rc);
	if (nodeoffset < 0)
		return;

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

static int fdt_fixup_pcie_device_ls(void *blob, pci_dev_t bdf,
				    struct ls_pcie_rc *pcie_rc)
{
	int streamid, index;

	streamid = pcie_next_streamid(pcie_rc->stream_id_cur,
				      pcie_rc->pcie->idx);
	if (streamid < 0) {
		printf("ERROR: out of stream ids for BDF %d.%d.%d\n",
		       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
		return -ENOENT;
	}
	pcie_rc->stream_id_cur++;

	index = ls_pcie_next_lut_index(pcie_rc);
	if (index < 0) {
		printf("ERROR: out of LUT indexes for BDF %d.%d.%d\n",
		       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
		return -ENOENT;
	}

	/* map PCI b.d.f to streamID in LUT */
	ls_pcie_lut_set_mapping(pcie_rc, index, bdf >> 8, streamid);
	/* update msi-map in device tree */
	fdt_pcie_set_msi_map_entry_ls(blob, pcie_rc, bdf >> 8, streamid);
	/* update iommu-map in device tree */
	fdt_pcie_set_iommu_map_entry_ls(blob, pcie_rc, bdf >> 8, streamid);

	return 0;
}

struct extra_iommu_entry {
	int action;
	pci_dev_t bdf;
	int num_vfs;
	bool noari;
};

#define EXTRA_IOMMU_ENTRY_HOTPLUG	1
#define EXTRA_IOMMU_ENTRY_VFS		2

static struct extra_iommu_entry *get_extra_iommu_ents(void *blob,
						      int nodeoffset,
						      phys_addr_t addr,
						      int *cnt)
{
	const char *s, *p, *tok;
	struct extra_iommu_entry *entries;
	int i = 0, b, d, f;

	/*
	 * Retrieve extra IOMMU configuration from env var or from device tree.
	 * Env var is given priority.
	 */
	s = env_get("pci_iommu_extra");
	if (!s) {
		s = fdt_getprop(blob, nodeoffset, "pci-iommu-extra", NULL);
	} else {
		phys_addr_t pci_base;
		char *endp;

		/*
		 * In env var case the config string has "pci@0x..." in
		 * addition. Parse this part and match it by address against
		 * the input pci controller's registers base address.
		 */
		tok = s;
		p = strchrnul(s + 1, ',');
		s = NULL;
		do {
			if (!strncmp(tok, "pci", 3)) {
				pci_base = simple_strtoul(tok  + 4, &endp, 0);
				if (pci_base == addr) {
					s = endp + 1;
					break;
				}
			}
			p = strchrnul(p + 1, ',');
			tok = p + 1;
		} while (*p);
	}

	/*
	 * If no env var or device tree property found or pci register base
	 * address mismatches, bail out
	 */
	if (!s)
		return NULL;

	/*
	 * In order to find how many action entries to allocate, count number
	 * of actions by interating through the pairs of bdfs and actions.
	 */
	*cnt = 0;
	p = s;
	while (*p && strncmp(p, "pci", 3)) {
		if (*p == ',')
			(*cnt)++;
		p++;
	}
	if (!(*p))
		(*cnt)++;

	if (!(*cnt) || (*cnt) % 2) {
		printf("ERROR: invalid or odd extra iommu token count %d\n",
		       *cnt);
		return NULL;
	}
	*cnt = (*cnt) / 2;

	entries = malloc((*cnt) * sizeof(*entries));
	if (!entries) {
		printf("ERROR: fail to allocate extra iommu entries\n");
		return NULL;
	}

	/*
	 * Parse action entries one by one and store the information in the
	 * newly allocated actions array.
	 */
	p = s;
	while (p) {
		/* Extract BDF */
		b = simple_strtoul(p, (char **)&p, 0); p++;
		d = simple_strtoul(p, (char **)&p, 0); p++;
		f = simple_strtoul(p, (char **)&p, 0); p++;
		entries[i].bdf = PCI_BDF(b, d, f);

		/* Parse action */
		if (!strncmp(p, "hp", 2)) {
			/* Hot-plug entry */
			entries[i].action = EXTRA_IOMMU_ENTRY_HOTPLUG;
			p += 2;
		} else if (!strncmp(p, "vfs", 3) ||
			   !strncmp(p, "noari_vfs", 9)) {
			/* VFs or VFs with ARI disabled entry */
			entries[i].action = EXTRA_IOMMU_ENTRY_VFS;
			entries[i].noari = !strncmp(p, "noari_vfs", 9);

			/*
			 * Parse and store total number of VFs to allocate
			 * IOMMU entries for.
			 */
			p = strchr(p, '=');
			entries[i].num_vfs = simple_strtoul(p + 1, (char **)&p,
							    0);
			if (*p)
				p++;
		} else {
			printf("ERROR: invalid action in extra iommu entry\n");
			free(entries);

			return NULL;
		}

		if (!(*p) || !strncmp(p, "pci", 3))
			break;

		i++;
	}

	return entries;
}

static void get_vf_offset_and_stride(struct udevice *dev, int sriov_pos,
				     struct extra_iommu_entry *entry,
				     u16 *offset, u16 *stride)
{
	u16 tmp16;
	u32 tmp32;
	bool have_ari = false;
	int pos;
	struct udevice *pf_dev;

	dm_pci_read_config16(dev, sriov_pos + PCI_SRIOV_TOTAL_VF, &tmp16);
	if (entry->num_vfs > tmp16) {
		printf("WARN: requested no. of VFs %d exceeds total of %d\n",
		       entry->num_vfs, tmp16);
	}

	/*
	 * The code below implements the VF Discovery recomandations specified
	 * in PCIe base spec "9.2.1.2 VF Discovery", quoted below:
	 *
	 * VF Discovery
	 *
	 * The First VF Offset and VF Stride fields in the SR-IOV extended
	 * capability are 16-bit Routing ID offsets. These offsets are used to
	 * compute the Routing IDs for the VFs with the following restrictions:
	 *  - The value in NumVFs in a PF (Section 9.3.3.7) may affect the
	 *    values in First VF Offset (Section 9.3.3.9) and VF Stride
	 *    (Section 9.3.3.10) of that PF.
	 *  - The value in ARI Capable Hierarchy (Section 9.3.3.3.5) in the
	 *    lowest-numbered PF of the Device (for example PF0) may affect
	 *    the values in First VF Offset and VF Stride in all PFs of the
	 *    Device.
	 *  - NumVFs of a PF may only be changed when VF Enable
	 *    (Section 9.3.3.3.1) of that PF is Clear.
	 *  - ARI Capable Hierarchy (Section 9.3.3.3.5) may only be changed
	 *    when VF Enable is Clear in all PFs of a Device.
	 */

	/* Clear VF enable for all PFs */
	device_foreach_child(pf_dev, dev->parent) {
		dm_pci_read_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL,
				     &tmp16);
		tmp16 &= ~PCI_SRIOV_CTRL_VFE;
		dm_pci_write_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL,
				      tmp16);
	}

	/* Obtain a reference to PF0 device */
	if (dm_pci_bus_find_bdf(PCI_BDF(PCI_BUS(entry->bdf),
					PCI_DEV(entry->bdf), 0), &pf_dev)) {
		printf("WARN: failed to get PF0\n");
	}

	if (entry->noari)
		goto skip_ari;

	/* Check that connected downstream port supports ARI Forwarding */
	pos = dm_pci_find_capability(dev->parent, PCI_CAP_ID_EXP);
	dm_pci_read_config32(dev->parent, pos + PCI_EXP_DEVCAP2, &tmp32);
	if (!(tmp32 & PCI_EXP_DEVCAP2_ARI))
		goto skip_ari;

	/* Check that PF supports Alternate Routing ID */
	if (!dm_pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ARI))
		goto skip_ari;

	/* Set ARI Capable Hierarcy for PF0 */
	dm_pci_read_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL, &tmp16);
	tmp16 |= PCI_SRIOV_CTRL_ARI;
	dm_pci_write_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL, tmp16);
	have_ari = true;

skip_ari:
	if (!have_ari) {
		/*
		 * No ARI support or disabled so clear ARI Capable Hierarcy
		 * for PF0
		 */
		dm_pci_read_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL,
				     &tmp16);
		tmp16 &= ~PCI_SRIOV_CTRL_ARI;
		dm_pci_write_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL,
				      tmp16);
	}

	/* Set requested number of VFs */
	dm_pci_write_config16(dev, sriov_pos + PCI_SRIOV_NUM_VF,
			      entry->num_vfs);

	/* Read VF stride and offset with the configs just made */
	dm_pci_read_config16(dev, sriov_pos + PCI_SRIOV_VF_OFFSET, offset);
	dm_pci_read_config16(dev, sriov_pos + PCI_SRIOV_VF_STRIDE, stride);

	if (have_ari) {
		/* Reset to default ARI Capable Hierarcy bit for PF0 */
		dm_pci_read_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL,
				     &tmp16);
		tmp16 &= ~PCI_SRIOV_CTRL_ARI;
		dm_pci_write_config16(pf_dev, sriov_pos + PCI_SRIOV_CTRL,
				      tmp16);
	}
	/* Reset to default the number of VFs */
	dm_pci_write_config16(dev, sriov_pos + PCI_SRIOV_NUM_VF, 0);
}

static int fdt_fixup_pci_vfs(void *blob, struct extra_iommu_entry *entry,
			     struct ls_pcie_rc *pcie_rc)
{
	struct udevice *dev, *bus;
	u16 vf_offset, vf_stride;
	int i, sriov_pos;
	pci_dev_t bdf;

	if (dm_pci_bus_find_bdf(entry->bdf, &dev)) {
		printf("ERROR: BDF %d.%d.%d not found\n", PCI_BUS(entry->bdf),
		       PCI_DEV(entry->bdf), PCI_FUNC(entry->bdf));
		return 0;
	}

	sriov_pos = dm_pci_find_ext_capability(dev, PCI_EXT_CAP_ID_SRIOV);
	if (!sriov_pos) {
		printf("WARN: trying to set VFs on non-SRIOV dev\n");
		return 0;
	}

	get_vf_offset_and_stride(dev, sriov_pos, entry, &vf_offset, &vf_stride);

	for (bus = dev; device_is_on_pci_bus(bus);)
		bus = bus->parent;

	bdf = entry->bdf - PCI_BDF(dev_seq(bus), 0, 0) + (vf_offset << 8);

	for (i = 0; i < entry->num_vfs; i++) {
		if (fdt_fixup_pcie_device_ls(blob, bdf, pcie_rc) < 0)
			return -1;
		bdf += vf_stride << 8;
	}

	printf("Added %d iommu VF mappings for PF %d.%d.%d\n",
	       entry->num_vfs, PCI_BUS(entry->bdf),
	       PCI_DEV(entry->bdf), PCI_FUNC(entry->bdf));

	return 0;
}

static void fdt_fixup_pcie_ls(void *blob)
{
	struct udevice *dev, *bus;
	struct ls_pcie_rc *pcie_rc;
	pci_dev_t bdf;
	struct extra_iommu_entry *entries;
	int i, cnt, nodeoffset;


	/* Scan all known buses */
	for (pci_find_first_device(&dev);
	     dev;
	     pci_find_next_device(&dev)) {
		for (bus = dev; device_is_on_pci_bus(bus);)
			bus = bus->parent;

		/* Only do the fixups for layerscape PCIe controllers */
		if (!device_is_compatible(bus, "fsl,ls-pcie") &&
		    !device_is_compatible(bus, CONFIG_FSL_PCIE_COMPAT))
			continue;

		pcie_rc = dev_get_priv(bus);

		/* the DT fixup must be relative to the hose first_busno */
		bdf = dm_pci_get_bdf(dev) - PCI_BDF(dev_seq(bus), 0, 0);

		if (fdt_fixup_pcie_device_ls(blob, bdf, pcie_rc) < 0)
			break;
	}

	if (!IS_ENABLED(CONFIG_PCI_IOMMU_EXTRA_MAPPINGS))
		return;

	list_for_each_entry(pcie_rc, &ls_pcie_list, list) {
		nodeoffset = fdt_pcie_get_nodeoffset(blob, pcie_rc);
		if (nodeoffset < 0) {
			printf("ERROR: couldn't find pci node\n");
			continue;
		}

		entries = get_extra_iommu_ents(blob, nodeoffset,
					       pcie_rc->dbi_res.start, &cnt);
		if (!entries)
			continue;

		for (i = 0; i < cnt; i++) {
			if (entries[i].action == EXTRA_IOMMU_ENTRY_HOTPLUG) {
				bdf = entries[i].bdf;
				printf("Added iommu map for hotplug %d.%d.%d\n",
				       PCI_BUS(bdf), PCI_DEV(bdf),
				       PCI_FUNC(bdf));
				if (fdt_fixup_pcie_device_ls(blob, bdf,
							     pcie_rc) < 0) {
					free(entries);
					return;
				}
			} else if (entries[i].action == EXTRA_IOMMU_ENTRY_VFS) {
				if (fdt_fixup_pci_vfs(blob, &entries[i],
						      pcie_rc) < 0) {
					free(entries);
					return;
				}
			} else {
				printf("Invalid action %d for BDF %d.%d.%d\n",
				       entries[i].action,
				       PCI_BUS(entries[i].bdf),
				       PCI_DEV(entries[i].bdf),
				       PCI_FUNC(entries[i].bdf));
			}
		}
		free(entries);
	}
}
#endif

static void ft_pcie_rc_fix(void *blob, struct ls_pcie_rc *pcie_rc)
{
	int off;
	struct ls_pcie *pcie = pcie_rc->pcie;

	off = fdt_pcie_get_nodeoffset(blob, pcie_rc);
	if (off < 0)
		return;

	if (pcie_rc->enabled && pcie->mode == PCI_HEADER_TYPE_BRIDGE)
		fdt_set_node_status(blob, off, FDT_STATUS_OKAY);
	else
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
}

static void ft_pcie_ep_fix(void *blob, struct ls_pcie_rc *pcie_rc)
{
	int off;
	struct ls_pcie *pcie = pcie_rc->pcie;

	off = fdt_node_offset_by_compat_reg(blob, CONFIG_FSL_PCIE_EP_COMPAT,
					    pcie_rc->dbi_res.start);
	if (off < 0)
		return;

	if (pcie_rc->enabled && pcie->mode == PCI_HEADER_TYPE_NORMAL)
		fdt_set_node_status(blob, off, FDT_STATUS_OKAY);
	else
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
}

static void ft_pcie_ls_setup(void *blob, struct ls_pcie_rc *pcie_rc)
{
	ft_pcie_ep_fix(blob, pcie_rc);
	ft_pcie_rc_fix(blob, pcie_rc);

	pcie_rc->stream_id_cur = 0;
	pcie_rc->next_lut_index = 0;
}

/* Fixup Kernel DT for PCIe */
void ft_pci_setup_ls(void *blob, struct bd_info *bd)
{
	struct ls_pcie_rc *pcie_rc;

#if defined(CONFIG_FSL_LSCH3) || defined(CONFIG_FSL_LSCH2)
	pcie_board_fix_fdt(blob);
#endif

	list_for_each_entry(pcie_rc, &ls_pcie_list, list)
		ft_pcie_ls_setup(blob, pcie_rc);

#if defined(CONFIG_FSL_LSCH3) || defined(CONFIG_FSL_LSCH2)
	next_stream_id = FSL_PEX_STREAM_ID_START;
	fdt_fixup_pcie_ls(blob);
#endif
}

#else /* !CONFIG_OF_BOARD_SETUP */
void ft_pci_setup_ls(void *blob, struct bd_info *bd)
{
}
#endif
