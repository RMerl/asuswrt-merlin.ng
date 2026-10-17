// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020,2021 NXP
 * Layerscape PCIe driver
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <pci.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <errno.h>
#include <malloc.h>
#include <dm.h>
#include <dm/devres.h>
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3) || \
	defined(CONFIG_ARM)
#include <asm/arch/clock.h>
#endif
#include "pcie_layerscape.h"

DECLARE_GLOBAL_DATA_PTR;

struct ls_pcie_drvdata {
	u32 lut_offset;
	u32 ctrl_offset;
	bool big_endian;
};

static void ls_pcie_cfg0_set_busdev(struct ls_pcie_rc *pcie_rc, u32 busdev)
{
	struct ls_pcie *pcie = pcie_rc->pcie;

	dbi_writel(pcie, PCIE_ATU_REGION_OUTBOUND | PCIE_ATU_REGION_INDEX0,
		   PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, busdev, PCIE_ATU_LOWER_TARGET);
}

static void ls_pcie_cfg1_set_busdev(struct ls_pcie_rc *pcie_rc, u32 busdev)
{
	struct ls_pcie *pcie = pcie_rc->pcie;

	dbi_writel(pcie, PCIE_ATU_REGION_OUTBOUND | PCIE_ATU_REGION_INDEX1,
		   PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, busdev, PCIE_ATU_LOWER_TARGET);
}

static void ls_pcie_setup_atu(struct ls_pcie_rc *pcie_rc)
{
	struct pci_region *io, *mem, *pref;
	unsigned long long offset = 0;
	struct ls_pcie *pcie = pcie_rc->pcie;
	int idx = 0;
	uint svr;

	svr = get_svr();
	if (((svr >> SVR_VAR_PER_SHIFT) & SVR_LS102XA_MASK) == SVR_LS102XA) {
		offset = LS1021_PCIE_SPACE_OFFSET +
			 LS1021_PCIE_SPACE_SIZE * pcie->idx;
	}

	/* ATU 0 : OUTBOUND : CFG0 */
	ls_pcie_atu_outbound_set(pcie, PCIE_ATU_REGION_INDEX0,
				 PCIE_ATU_TYPE_CFG0,
				 pcie_rc->cfg_res.start + offset,
				 0,
				 fdt_resource_size(&pcie_rc->cfg_res) / 2);
	/* ATU 1 : OUTBOUND : CFG1 */
	ls_pcie_atu_outbound_set(pcie, PCIE_ATU_REGION_INDEX1,
				 PCIE_ATU_TYPE_CFG1,
				 pcie_rc->cfg_res.start + offset +
				 fdt_resource_size(&pcie_rc->cfg_res) / 2,
				 0,
				 fdt_resource_size(&pcie_rc->cfg_res) / 2);

	pci_get_regions(pcie_rc->bus, &io, &mem, &pref);
	idx = PCIE_ATU_REGION_INDEX1 + 1;

	/* Fix the pcie memory map for LS2088A series SoCs */
	svr = (svr >> SVR_VAR_PER_SHIFT) & 0xFFFFFE;
	if (svr == SVR_LS2088A || svr == SVR_LS2084A ||
	    svr == SVR_LS2048A || svr == SVR_LS2044A ||
	    svr == SVR_LS2081A || svr == SVR_LS2041A) {
		if (io)
			io->phys_start = (io->phys_start &
					 (PCIE_PHYS_SIZE - 1)) +
					 LS2088A_PCIE1_PHYS_ADDR +
					 LS2088A_PCIE_PHYS_SIZE * pcie->idx;
		if (mem)
			mem->phys_start = (mem->phys_start &
					 (PCIE_PHYS_SIZE - 1)) +
					 LS2088A_PCIE1_PHYS_ADDR +
					 LS2088A_PCIE_PHYS_SIZE * pcie->idx;
		if (pref)
			pref->phys_start = (pref->phys_start &
					 (PCIE_PHYS_SIZE - 1)) +
					 LS2088A_PCIE1_PHYS_ADDR +
					 LS2088A_PCIE_PHYS_SIZE * pcie->idx;
	}

	if (io)
		/* ATU : OUTBOUND : IO */
		ls_pcie_atu_outbound_set(pcie, idx++,
					 PCIE_ATU_TYPE_IO,
					 io->phys_start + offset,
					 io->bus_start,
					 io->size);

	if (mem)
		/* ATU : OUTBOUND : MEM */
		ls_pcie_atu_outbound_set(pcie, idx++,
					 PCIE_ATU_TYPE_MEM,
					 mem->phys_start + offset,
					 mem->bus_start,
					 mem->size);

	if (pref)
		/* ATU : OUTBOUND : pref */
		ls_pcie_atu_outbound_set(pcie, idx++,
					 PCIE_ATU_TYPE_MEM,
					 pref->phys_start + offset,
					 pref->bus_start,
					 pref->size);

	ls_pcie_dump_atu(pcie, PCIE_ATU_REGION_NUM, PCIE_ATU_REGION_OUTBOUND);
}

/* Return 0 if the address is valid, -errno if not valid */
static int ls_pcie_addr_valid(struct ls_pcie_rc *pcie_rc, pci_dev_t bdf)
{
	struct udevice *bus = pcie_rc->bus;
	struct ls_pcie *pcie = pcie_rc->pcie;

	if (pcie->mode == PCI_HEADER_TYPE_NORMAL)
		return -ENODEV;

	if (!pcie_rc->enabled)
		return -ENXIO;

	if (PCI_BUS(bdf) < dev_seq(bus))
		return -EINVAL;

	if ((PCI_BUS(bdf) > dev_seq(bus)) && (!ls_pcie_link_up(pcie)))
		return -EINVAL;

	if (PCI_BUS(bdf) <= (dev_seq(bus) + 1) && (PCI_DEV(bdf) > 0))
		return -EINVAL;

	return 0;
}

static int ls_pcie_conf_address(const struct udevice *bus, pci_dev_t bdf,
				uint offset, void **paddress)
{
	struct ls_pcie_rc *pcie_rc = dev_get_priv(bus);
	struct ls_pcie *pcie = pcie_rc->pcie;
	u32 busdev;

	if (ls_pcie_addr_valid(pcie_rc, bdf))
		return -EINVAL;

	if (PCI_BUS(bdf) == dev_seq(bus)) {
		*paddress = pcie->dbi + offset;
		return 0;
	}

	busdev = PCIE_ATU_BUS(PCI_BUS(bdf) - dev_seq(bus)) |
		 PCIE_ATU_DEV(PCI_DEV(bdf)) |
		 PCIE_ATU_FUNC(PCI_FUNC(bdf));

	if (PCI_BUS(bdf) == dev_seq(bus) + 1) {
		ls_pcie_cfg0_set_busdev(pcie_rc, busdev);
		*paddress = pcie_rc->cfg0 + offset;
	} else {
		ls_pcie_cfg1_set_busdev(pcie_rc, busdev);
		*paddress = pcie_rc->cfg1 + offset;
	}
	return 0;
}

static int ls_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
			       uint offset, ulong *valuep,
			       enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, ls_pcie_conf_address,
					    bdf, offset, valuep, size);
}

static int ls_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong value,
				enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, ls_pcie_conf_address,
					     bdf, offset, value, size);
}

/* Clear multi-function bit */
static void ls_pcie_clear_multifunction(struct ls_pcie_rc *pcie_rc)
{
	struct ls_pcie *pcie = pcie_rc->pcie;

	writeb(PCI_HEADER_TYPE_BRIDGE, pcie->dbi + PCI_HEADER_TYPE);
}

/* Fix class value */
static void ls_pcie_fix_class(struct ls_pcie_rc *pcie_rc)
{
	struct ls_pcie *pcie = pcie_rc->pcie;

	writew(PCI_CLASS_BRIDGE_PCI, pcie->dbi + PCI_CLASS_DEVICE);
}

/* Drop MSG TLP except for Vendor MSG */
static void ls_pcie_drop_msg_tlp(struct ls_pcie_rc *pcie_rc)
{
	struct ls_pcie *pcie = pcie_rc->pcie;
	u32 val;

	val = dbi_readl(pcie, PCIE_STRFMR1);
	val &= 0xDFFFFFFF;
	dbi_writel(pcie, val, PCIE_STRFMR1);
}

/* Disable all bars in RC mode */
static void ls_pcie_disable_bars(struct ls_pcie_rc *pcie_rc)
{
	struct ls_pcie *pcie = pcie_rc->pcie;

	dbi_writel(pcie, 0, PCIE_CS2_OFFSET + PCI_BASE_ADDRESS_0);
	dbi_writel(pcie, 0, PCIE_CS2_OFFSET + PCI_BASE_ADDRESS_1);
	dbi_writel(pcie, 0xfffffffe, PCIE_CS2_OFFSET + PCI_ROM_ADDRESS1);
}

static void ls_pcie_setup_ctrl(struct ls_pcie_rc *pcie_rc)
{
	struct ls_pcie *pcie = pcie_rc->pcie;

	ls_pcie_setup_atu(pcie_rc);

	ls_pcie_dbi_ro_wr_en(pcie);
	ls_pcie_fix_class(pcie_rc);
	ls_pcie_clear_multifunction(pcie_rc);
	ls_pcie_drop_msg_tlp(pcie_rc);
	ls_pcie_dbi_ro_wr_dis(pcie);

	ls_pcie_disable_bars(pcie_rc);
}

static int ls_pcie_probe(struct udevice *dev)
{
	const struct ls_pcie_drvdata *drvdata = (void *)dev_get_driver_data(dev);
	struct ls_pcie_rc *pcie_rc = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	struct ls_pcie *pcie;
	u16 link_sta;
	uint svr;
	int ret;
	fdt_size_t cfg_size;

	pcie_rc->bus = dev;

	pcie = devm_kzalloc(dev, sizeof(*pcie), GFP_KERNEL);
	if (!pcie)
		return -ENOMEM;

	pcie_rc->pcie = pcie;

	/* try resource name of the official binding first */
	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "regs", &pcie_rc->dbi_res);
	if (ret)
		ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
					     "dbi", &pcie_rc->dbi_res);
	if (ret) {
		printf("ls-pcie: resource \"dbi\" not found\n");
		return ret;
	}

	pcie->idx = (pcie_rc->dbi_res.start - PCIE_SYS_BASE_ADDR) /
		    PCIE_CCSR_SIZE;

	list_add(&pcie_rc->list, &ls_pcie_list);

	pcie_rc->enabled = is_serdes_configured(PCIE_SRDS_PRTCL(pcie->idx));
	if (!pcie_rc->enabled) {
		printf("PCIe%d: %s disabled\n", PCIE_SRDS_PRTCL(pcie->idx),
		       dev->name);
		return 0;
	}

	pcie->dbi = map_physmem(pcie_rc->dbi_res.start,
				fdt_resource_size(&pcie_rc->dbi_res),
				MAP_NOCACHE);

	pcie->mode = readb(pcie->dbi + PCI_HEADER_TYPE) & 0x7f;
	if (pcie->mode == PCI_HEADER_TYPE_NORMAL)
		return 0;

	if (drvdata) {
		pcie->lut = pcie->dbi + drvdata->lut_offset;
	} else {
		ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
					     "lut", &pcie_rc->lut_res);
		if (!ret)
			pcie->lut = map_physmem(pcie_rc->lut_res.start,
						fdt_resource_size(&pcie_rc->lut_res),
						MAP_NOCACHE);
	}

	if (drvdata) {
		pcie->ctrl = pcie->lut + drvdata->ctrl_offset;
	} else {
		ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
					     "ctrl", &pcie_rc->ctrl_res);
		if (!ret)
			pcie->ctrl = map_physmem(pcie_rc->ctrl_res.start,
						 fdt_resource_size(&pcie_rc->ctrl_res),
						 MAP_NOCACHE);
		if (!pcie->ctrl)
			pcie->ctrl = pcie->lut;
	}

	if (!pcie->ctrl) {
		printf("%s: NOT find CTRL\n", dev->name);
		return -1;
	}

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "config", &pcie_rc->cfg_res);
	if (ret) {
		printf("%s: resource \"config\" not found\n", dev->name);
		return ret;
	}

	cfg_size = fdt_resource_size(&pcie_rc->cfg_res);
	if (cfg_size < SZ_8K) {
		printf("PCIe%d: %s Invalid size(0x%llx) for resource \"config\",expected minimum 0x%x\n",
		       PCIE_SRDS_PRTCL(pcie->idx), dev->name, (u64)cfg_size, SZ_8K);
		return 0;
	}

	/*
	 * Fix the pcie memory map address and PF control registers address
	 * for LS2088A series SoCs
	 */
	svr = get_svr();
	svr = (svr >> SVR_VAR_PER_SHIFT) & 0xFFFFFE;
	if (svr == SVR_LS2088A || svr == SVR_LS2084A ||
	    svr == SVR_LS2048A || svr == SVR_LS2044A ||
	    svr == SVR_LS2081A || svr == SVR_LS2041A) {
		pcie_rc->cfg_res.start = LS2088A_PCIE1_PHYS_ADDR +
					 LS2088A_PCIE_PHYS_SIZE * pcie->idx;
		pcie_rc->cfg_res.end = pcie_rc->cfg_res.start + cfg_size;
		pcie->ctrl = pcie->lut + 0x40000;
	}

	pcie_rc->cfg0 = map_physmem(pcie_rc->cfg_res.start,
				    fdt_resource_size(&pcie_rc->cfg_res),
				    MAP_NOCACHE);
	pcie_rc->cfg1 = pcie_rc->cfg0 +
			fdt_resource_size(&pcie_rc->cfg_res) / 2;

	if (drvdata)
		pcie->big_endian = drvdata->big_endian;
	else
		pcie->big_endian = fdtdec_get_bool(fdt, node, "big-endian");

	debug("%s dbi:%lx lut:%lx ctrl:0x%lx cfg0:0x%lx, big-endian:%d\n",
	      dev->name, (unsigned long)pcie->dbi, (unsigned long)pcie->lut,
	      (unsigned long)pcie->ctrl, (unsigned long)pcie_rc->cfg0,
	      pcie->big_endian);

	printf("PCIe%u: %s %s", PCIE_SRDS_PRTCL(pcie->idx), dev->name,
	       "Root Complex");
	ls_pcie_setup_ctrl(pcie_rc);

	if (!ls_pcie_link_up(pcie)) {
		/* Let the user know there's no PCIe link */
		printf(": no link\n");
		return 0;
	}

	/* Print the negotiated PCIe link width */
	link_sta = readw(pcie->dbi + PCIE_LINK_STA);
	printf(": x%d gen%d\n", (link_sta & PCIE_LINK_WIDTH_MASK) >> 4,
	       link_sta & PCIE_LINK_SPEED_MASK);

	return 0;
}

static const struct dm_pci_ops ls_pcie_ops = {
	.read_config	= ls_pcie_read_config,
	.write_config	= ls_pcie_write_config,
};

static const struct ls_pcie_drvdata ls1028a_drvdata = {
	.lut_offset = 0x80000,
	.ctrl_offset = 0x40000,
	.big_endian = false,
};

static const struct udevice_id ls_pcie_ids[] = {
	{ .compatible = "fsl,ls-pcie" },
	{ .compatible = "fsl,ls1028a-pcie", .data = (ulong)&ls1028a_drvdata },
	{ .compatible = "fsl,ls1088a-pcie", .data = (ulong)&ls1028a_drvdata },
	{ }
};

U_BOOT_DRIVER(pci_layerscape) = {
	.name = "pci_layerscape",
	.id = UCLASS_PCI,
	.of_match = ls_pcie_ids,
	.ops = &ls_pcie_ops,
	.probe	= ls_pcie_probe,
	.priv_auto	= sizeof(struct ls_pcie_rc),
};
