// SPDX-License-Identifier: GPL-2.0+ OR X11
/*
 * Copyright 2018-2019 NXP
 *
 * PCIe Gen4 driver for NXP Layerscape SoCs
 * Author: Hou Zhiqiang <Minder.Hou@gmail.com>
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <pci.h>
#include <asm/io.h>
#include <errno.h>
#include <malloc.h>
#include <dm.h>
#include <linux/sizes.h>

#include "pcie_layerscape_gen4.h"

DECLARE_GLOBAL_DATA_PTR;

LIST_HEAD(ls_pcie_g4_list);

static u64 bar_size[4] = {
	PCIE_BAR0_SIZE,
	PCIE_BAR1_SIZE,
	PCIE_BAR2_SIZE,
	PCIE_BAR4_SIZE
};

static int ls_pcie_g4_ltssm(struct ls_pcie_g4 *pcie)
{
	u32 state;

	state = pf_ctrl_readl(pcie, PCIE_LTSSM_STA) & LTSSM_STATE_MASK;

	return state;
}

static int ls_pcie_g4_link_up(struct ls_pcie_g4 *pcie)
{
	int ltssm;

	ltssm = ls_pcie_g4_ltssm(pcie);
	if (ltssm != LTSSM_PCIE_L0)
		return 0;

	return 1;
}

static void ls_pcie_g4_ep_enable_cfg(struct ls_pcie_g4 *pcie)
{
	ccsr_writel(pcie, GPEX_CFG_READY, PCIE_CONFIG_READY);
}

static void ls_pcie_g4_cfg_set_target(struct ls_pcie_g4 *pcie, u32 target)
{
	ccsr_writel(pcie, PAB_AXI_AMAP_PEX_WIN_L(0), target);
	ccsr_writel(pcie, PAB_AXI_AMAP_PEX_WIN_H(0), 0);
}

static int ls_pcie_g4_outbound_win_set(struct ls_pcie_g4 *pcie, int idx,
				       int type, u64 phys, u64 bus_addr,
				       pci_size_t size)
{
	u32 val;
	u32 size_h, size_l;

	if (idx >= PAB_WINS_NUM)
		return -EINVAL;

	size_h = upper_32_bits(~(size - 1));
	size_l = lower_32_bits(~(size - 1));

	val = ccsr_readl(pcie, PAB_AXI_AMAP_CTRL(idx));
	val &= ~((AXI_AMAP_CTRL_TYPE_MASK << AXI_AMAP_CTRL_TYPE_SHIFT) |
		(AXI_AMAP_CTRL_SIZE_MASK << AXI_AMAP_CTRL_SIZE_SHIFT) |
		AXI_AMAP_CTRL_EN);
	val |= ((type & AXI_AMAP_CTRL_TYPE_MASK) << AXI_AMAP_CTRL_TYPE_SHIFT) |
		((size_l >> AXI_AMAP_CTRL_SIZE_SHIFT) <<
		AXI_AMAP_CTRL_SIZE_SHIFT) | AXI_AMAP_CTRL_EN;

	ccsr_writel(pcie, PAB_AXI_AMAP_CTRL(idx), val);

	ccsr_writel(pcie, PAB_AXI_AMAP_AXI_WIN(idx), lower_32_bits(phys));
	ccsr_writel(pcie, PAB_EXT_AXI_AMAP_AXI_WIN(idx), upper_32_bits(phys));
	ccsr_writel(pcie, PAB_AXI_AMAP_PEX_WIN_L(idx), lower_32_bits(bus_addr));
	ccsr_writel(pcie, PAB_AXI_AMAP_PEX_WIN_H(idx), upper_32_bits(bus_addr));
	ccsr_writel(pcie, PAB_EXT_AXI_AMAP_SIZE(idx), size_h);

	return 0;
}

static int ls_pcie_g4_rc_inbound_win_set(struct ls_pcie_g4 *pcie, int idx,
					 int type, u64 phys, u64 bus_addr,
					 pci_size_t size)
{
	u32 val;
	pci_size_t win_size = ~(size - 1);

	val = ccsr_readl(pcie, PAB_PEX_AMAP_CTRL(idx));

	val &= ~(PEX_AMAP_CTRL_TYPE_MASK << PEX_AMAP_CTRL_TYPE_SHIFT);
	val &= ~(PEX_AMAP_CTRL_EN_MASK << PEX_AMAP_CTRL_EN_SHIFT);
	val = (val | (type << PEX_AMAP_CTRL_TYPE_SHIFT));
	val = (val | (1 << PEX_AMAP_CTRL_EN_SHIFT));

	ccsr_writel(pcie, PAB_PEX_AMAP_CTRL(idx),
		    val | lower_32_bits(win_size));

	ccsr_writel(pcie, PAB_EXT_PEX_AMAP_SIZE(idx), upper_32_bits(win_size));
	ccsr_writel(pcie, PAB_PEX_AMAP_AXI_WIN(idx), lower_32_bits(phys));
	ccsr_writel(pcie, PAB_EXT_PEX_AMAP_AXI_WIN(idx), upper_32_bits(phys));
	ccsr_writel(pcie, PAB_PEX_AMAP_PEX_WIN_L(idx), lower_32_bits(bus_addr));
	ccsr_writel(pcie, PAB_PEX_AMAP_PEX_WIN_H(idx), upper_32_bits(bus_addr));

	return 0;
}

static void ls_pcie_g4_dump_wins(struct ls_pcie_g4 *pcie, int wins)
{
	int i;

	for (i = 0; i < wins; i++) {
		debug("APIO Win%d:\n", i);
		debug("\tLOWER PHYS:	0x%08x\n",
		      ccsr_readl(pcie, PAB_AXI_AMAP_AXI_WIN(i)));
		debug("\tUPPER PHYS:	0x%08x\n",
		      ccsr_readl(pcie, PAB_EXT_AXI_AMAP_AXI_WIN(i)));
		debug("\tLOWER BUS:	0x%08x\n",
		      ccsr_readl(pcie, PAB_AXI_AMAP_PEX_WIN_L(i)));
		debug("\tUPPER BUS:	0x%08x\n",
		      ccsr_readl(pcie, PAB_AXI_AMAP_PEX_WIN_H(i)));
		debug("\tSIZE:		0x%08x\n",
		      ccsr_readl(pcie, PAB_AXI_AMAP_CTRL(i)) &
		      (AXI_AMAP_CTRL_SIZE_MASK << AXI_AMAP_CTRL_SIZE_SHIFT));
		debug("\tEXT_SIZE:	0x%08x\n",
		      ccsr_readl(pcie, PAB_EXT_AXI_AMAP_SIZE(i)));
		debug("\tPARAM:		0x%08x\n",
		      ccsr_readl(pcie, PAB_AXI_AMAP_PCI_HDR_PARAM(i)));
		debug("\tCTRL:		0x%08x\n",
		      ccsr_readl(pcie, PAB_AXI_AMAP_CTRL(i)));
	}
}

static void ls_pcie_g4_setup_wins(struct ls_pcie_g4 *pcie)
{
	struct pci_region *io, *mem, *pref;
	int idx = 1;

	/* INBOUND WIN */
	ls_pcie_g4_rc_inbound_win_set(pcie, 0, IB_TYPE_MEM_F, 0, 0, SIZE_1T);

	/* OUTBOUND WIN 0: CFG */
	ls_pcie_g4_outbound_win_set(pcie, 0, PAB_AXI_TYPE_CFG,
				    pcie->cfg_res.start, 0,
				    fdt_resource_size(&pcie->cfg_res));

	pci_get_regions(pcie->bus, &io, &mem, &pref);

	if (io)
		/* OUTBOUND WIN: IO */
		ls_pcie_g4_outbound_win_set(pcie, idx++, PAB_AXI_TYPE_IO,
					    io->phys_start, io->bus_start,
					    io->size);

	if (mem)
		/* OUTBOUND WIN: MEM */
		ls_pcie_g4_outbound_win_set(pcie, idx++, PAB_AXI_TYPE_MEM,
					    mem->phys_start, mem->bus_start,
					    mem->size);

	if (pref)
		/* OUTBOUND WIN: perf MEM */
		ls_pcie_g4_outbound_win_set(pcie, idx++, PAB_AXI_TYPE_MEM,
					    pref->phys_start, pref->bus_start,
					    pref->size);

	ls_pcie_g4_dump_wins(pcie, idx);
}

/* Return 0 if the address is valid, -errno if not valid */
static int ls_pcie_g4_addr_valid(struct ls_pcie_g4 *pcie, pci_dev_t bdf)
{
	struct udevice *bus = pcie->bus;

	if (pcie->mode == PCI_HEADER_TYPE_NORMAL)
		return -ENODEV;

	if (!pcie->enabled)
		return -ENXIO;

	if (PCI_BUS(bdf) < bus->seq)
		return -EINVAL;

	if ((PCI_BUS(bdf) > bus->seq) && (!ls_pcie_g4_link_up(pcie)))
		return -EINVAL;

	if (PCI_BUS(bdf) <= (bus->seq + 1) && (PCI_DEV(bdf) > 0))
		return -EINVAL;

	return 0;
}

void *ls_pcie_g4_conf_address(struct ls_pcie_g4 *pcie, pci_dev_t bdf,
			      int offset)
{
	struct udevice *bus = pcie->bus;
	u32 target;

	if (PCI_BUS(bdf) == bus->seq) {
		if (offset < INDIRECT_ADDR_BNDRY) {
			ccsr_set_page(pcie, 0);
			return pcie->ccsr + offset;
		}

		ccsr_set_page(pcie, OFFSET_TO_PAGE_IDX(offset));
		return pcie->ccsr + OFFSET_TO_PAGE_ADDR(offset);
	}

	target = PAB_TARGET_BUS(PCI_BUS(bdf) - bus->seq) |
		 PAB_TARGET_DEV(PCI_DEV(bdf)) |
		 PAB_TARGET_FUNC(PCI_FUNC(bdf));

	ls_pcie_g4_cfg_set_target(pcie, target);

	return pcie->cfg + offset;
}

static int ls_pcie_g4_read_config(struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong *valuep,
				  enum pci_size_t size)
{
	struct ls_pcie_g4 *pcie = dev_get_priv(bus);
	void *address;
	int ret = 0;

	if (ls_pcie_g4_addr_valid(pcie, bdf)) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	address = ls_pcie_g4_conf_address(pcie, bdf, offset);

	switch (size) {
	case PCI_SIZE_8:
		*valuep = readb(address);
		break;
	case PCI_SIZE_16:
		*valuep = readw(address);
		break;
	case PCI_SIZE_32:
		*valuep = readl(address);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int ls_pcie_g4_write_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong value,
				   enum pci_size_t size)
{
	struct ls_pcie_g4 *pcie = dev_get_priv(bus);
	void *address;

	if (ls_pcie_g4_addr_valid(pcie, bdf))
		return 0;

	address = ls_pcie_g4_conf_address(pcie, bdf, offset);

	switch (size) {
	case PCI_SIZE_8:
		writeb(value, address);
		return 0;
	case PCI_SIZE_16:
		writew(value, address);
		return 0;
	case PCI_SIZE_32:
		writel(value, address);
		return 0;
	default:
		return -EINVAL;
	}
}

static void ls_pcie_g4_setup_ctrl(struct ls_pcie_g4 *pcie)
{
	u32 val;

	/* Fix class code */
	val = ccsr_readl(pcie, GPEX_CLASSCODE);
	val &= ~(GPEX_CLASSCODE_MASK << GPEX_CLASSCODE_SHIFT);
	val |= PCI_CLASS_BRIDGE_PCI << GPEX_CLASSCODE_SHIFT;
	ccsr_writel(pcie, GPEX_CLASSCODE, val);

	/* Enable APIO and Memory/IO/CFG Wins */
	val = ccsr_readl(pcie, PAB_AXI_PIO_CTRL(0));
	val |= APIO_EN | MEM_WIN_EN | IO_WIN_EN | CFG_WIN_EN;
	ccsr_writel(pcie, PAB_AXI_PIO_CTRL(0), val);

	ls_pcie_g4_setup_wins(pcie);

	pcie->stream_id_cur = 0;
}

static void ls_pcie_g4_ep_inbound_win_set(struct ls_pcie_g4 *pcie, int pf,
					  int bar, u64 phys)
{
	u32 val;

	/* PF BAR1 is for MSI-X and only need to enable */
	if (bar == 1) {
		ccsr_writel(pcie, PAB_PEX_BAR_AMAP(pf, bar), BAR_AMAP_EN);
		return;
	}

	val = upper_32_bits(phys);
	ccsr_writel(pcie, PAB_EXT_PEX_BAR_AMAP(pf, bar), val);
	val = lower_32_bits(phys) | BAR_AMAP_EN;
	ccsr_writel(pcie, PAB_PEX_BAR_AMAP(pf, bar), val);
}

static void ls_pcie_g4_ep_setup_wins(struct ls_pcie_g4 *pcie, int pf)
{
	u64 phys;
	int bar;
	u32 val;

	if ((!pcie->sriov_support && pf > LS_G4_PF0) || pf > LS_G4_PF1)
		return;

	phys = CONFIG_SYS_PCI_EP_MEMORY_BASE + PCIE_BAR_SIZE * 4 * pf;
	for (bar = 0; bar < PF_BAR_NUM; bar++) {
		ls_pcie_g4_ep_inbound_win_set(pcie, pf, bar, phys);
		phys += PCIE_BAR_SIZE;
	}

	/* OUTBOUND: map MEM */
	ls_pcie_g4_outbound_win_set(pcie, pf, PAB_AXI_TYPE_MEM,
				    pcie->cfg_res.start +
				    CONFIG_SYS_PCI_MEMORY_SIZE * pf, 0x0,
				    CONFIG_SYS_PCI_MEMORY_SIZE);

	val = ccsr_readl(pcie, PAB_AXI_AMAP_PCI_HDR_PARAM(pf));
	val &= ~FUNC_NUM_PCIE_MASK;
	val |= pf;
	ccsr_writel(pcie, PAB_AXI_AMAP_PCI_HDR_PARAM(pf), val);
}

static void ls_pcie_g4_ep_enable_bar(struct ls_pcie_g4 *pcie, int pf,
				     int bar, bool vf_bar, bool enable)
{
	u32 val;
	u32 bar_pos = BAR_POS(bar, pf, vf_bar);

	val = ccsr_readl(pcie, GPEX_BAR_ENABLE);
	if (enable)
		val |= 1 << bar_pos;
	else
		val &= ~(1 << bar_pos);
	ccsr_writel(pcie, GPEX_BAR_ENABLE, val);
}

static void ls_pcie_g4_ep_set_bar_size(struct ls_pcie_g4 *pcie, int pf,
				       int bar, bool vf_bar, u64 size)
{
	u32 bar_pos = BAR_POS(bar, pf, vf_bar);
	u32 mask_l = lower_32_bits(~(size - 1));
	u32 mask_h = upper_32_bits(~(size - 1));

	ccsr_writel(pcie, GPEX_BAR_SELECT, bar_pos);
	ccsr_writel(pcie, GPEX_BAR_SIZE_LDW, mask_l);
	ccsr_writel(pcie, GPEX_BAR_SIZE_UDW, mask_h);
}

static void ls_pcie_g4_ep_setup_bar(struct ls_pcie_g4 *pcie, int pf,
				    int bar, bool vf_bar, u64 size)
{
	bool en = size ? true : false;

	ls_pcie_g4_ep_enable_bar(pcie, pf, bar, vf_bar, en);
	ls_pcie_g4_ep_set_bar_size(pcie, pf, bar, vf_bar, size);
}

static void ls_pcie_g4_ep_setup_bars(struct ls_pcie_g4 *pcie, int pf)
{
	int bar;

	/* Setup PF BARs */
	for (bar = 0; bar < PF_BAR_NUM; bar++)
		ls_pcie_g4_ep_setup_bar(pcie, pf, bar, false, bar_size[bar]);

	if (!pcie->sriov_support)
		return;

	/* Setup VF BARs */
	for (bar = 0; bar < VF_BAR_NUM; bar++)
		ls_pcie_g4_ep_setup_bar(pcie, pf, bar, true, bar_size[bar]);
}

static void ls_pcie_g4_set_sriov(struct ls_pcie_g4 *pcie, int pf)
{
	unsigned int val;

	val =  ccsr_readl(pcie, GPEX_SRIOV_INIT_VFS_TOTAL_VF(pf));
	val &= ~(TTL_VF_MASK << TTL_VF_SHIFT);
	val |= PCIE_VF_NUM << TTL_VF_SHIFT;
	val &= ~(INI_VF_MASK << INI_VF_SHIFT);
	val |= PCIE_VF_NUM << INI_VF_SHIFT;
	ccsr_writel(pcie, GPEX_SRIOV_INIT_VFS_TOTAL_VF(pf), val);

	val =  ccsr_readl(pcie, PCIE_SRIOV_VF_OFFSET_STRIDE);
	val += PCIE_VF_NUM * pf - pf;
	ccsr_writel(pcie, GPEX_SRIOV_VF_OFFSET_STRIDE(pf), val);
}

static void ls_pcie_g4_setup_ep(struct ls_pcie_g4 *pcie)
{
	u32 pf, sriov;
	u32 val;
	int i;

	/* Enable APIO and Memory Win */
	val = ccsr_readl(pcie, PAB_AXI_PIO_CTRL(0));
	val |= APIO_EN | MEM_WIN_EN;
	ccsr_writel(pcie, PAB_AXI_PIO_CTRL(0), val);

	sriov = ccsr_readl(pcie, PCIE_SRIOV_CAPABILITY);
	if (PCI_EXT_CAP_ID(sriov) == PCI_EXT_CAP_ID_SRIOV)
		pcie->sriov_support = 1;

	pf = pcie->sriov_support ? PCIE_PF_NUM : 1;

	for (i = 0; i < pf; i++) {
		ls_pcie_g4_ep_setup_bars(pcie, i);
		ls_pcie_g4_ep_setup_wins(pcie, i);
		if (pcie->sriov_support)
			ls_pcie_g4_set_sriov(pcie, i);
	}

	ls_pcie_g4_ep_enable_cfg(pcie);
	ls_pcie_g4_dump_wins(pcie, pf);
}

static int ls_pcie_g4_probe(struct udevice *dev)
{
	struct ls_pcie_g4 *pcie = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	u32 link_ctrl_sta;
	u32 val;
	int ret;

	pcie->bus = dev;

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "ccsr", &pcie->ccsr_res);
	if (ret) {
		printf("ls-pcie-g4: resource \"ccsr\" not found\n");
		return ret;
	}

	pcie->idx = (pcie->ccsr_res.start - PCIE_SYS_BASE_ADDR) /
		    PCIE_CCSR_SIZE;

	list_add(&pcie->list, &ls_pcie_g4_list);

	pcie->enabled = is_serdes_configured(PCIE_SRDS_PRTCL(pcie->idx));
	if (!pcie->enabled) {
		printf("PCIe%d: %s disabled\n", pcie->idx, dev->name);
		return 0;
	}

	pcie->ccsr = map_physmem(pcie->ccsr_res.start,
				 fdt_resource_size(&pcie->ccsr_res),
				 MAP_NOCACHE);

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "config", &pcie->cfg_res);
	if (ret) {
		printf("%s: resource \"config\" not found\n", dev->name);
		return ret;
	}

	pcie->cfg = map_physmem(pcie->cfg_res.start,
				fdt_resource_size(&pcie->cfg_res),
				MAP_NOCACHE);

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "lut", &pcie->lut_res);
	if (ret) {
		printf("ls-pcie-g4: resource \"lut\" not found\n");
		return ret;
	}

	pcie->lut = map_physmem(pcie->lut_res.start,
				fdt_resource_size(&pcie->lut_res),
				MAP_NOCACHE);

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "pf_ctrl", &pcie->pf_ctrl_res);
	if (ret) {
		printf("ls-pcie-g4: resource \"pf_ctrl\" not found\n");
		return ret;
	}

	pcie->pf_ctrl = map_physmem(pcie->pf_ctrl_res.start,
				    fdt_resource_size(&pcie->pf_ctrl_res),
				    MAP_NOCACHE);

	pcie->big_endian = fdtdec_get_bool(fdt, node, "big-endian");

	debug("%s ccsr:%lx, cfg:0x%lx, big-endian:%d\n",
	      dev->name, (unsigned long)pcie->ccsr, (unsigned long)pcie->cfg,
	      pcie->big_endian);

	pcie->mode = readb(pcie->ccsr + PCI_HEADER_TYPE) & 0x7f;

	if (pcie->mode == PCI_HEADER_TYPE_NORMAL) {
		printf("PCIe%u: %s %s", pcie->idx, dev->name, "Endpoint");
		ls_pcie_g4_setup_ep(pcie);
	} else {
		printf("PCIe%u: %s %s", pcie->idx, dev->name, "Root Complex");
		ls_pcie_g4_setup_ctrl(pcie);
	}

	/* Enable Amba & PEX PIO */
	val = ccsr_readl(pcie, PAB_CTRL);
	val |= PAB_CTRL_APIO_EN | PAB_CTRL_PPIO_EN;
	ccsr_writel(pcie, PAB_CTRL, val);

	val = ccsr_readl(pcie, PAB_PEX_PIO_CTRL(0));
	val |= PPIO_EN;
	ccsr_writel(pcie, PAB_PEX_PIO_CTRL(0), val);

	if (!ls_pcie_g4_link_up(pcie)) {
		/* Let the user know there's no PCIe link */
		printf(": no link\n");
		return 0;
	}

	/* Print the negotiated PCIe link width */
	link_ctrl_sta = ccsr_readl(pcie, PCIE_LINK_CTRL_STA);
	printf(": x%d gen%d\n",
	       (link_ctrl_sta >> PCIE_LINK_WIDTH_SHIFT & PCIE_LINK_WIDTH_MASK),
	       (link_ctrl_sta >> PCIE_LINK_SPEED_SHIFT) & PCIE_LINK_SPEED_MASK);

	return 0;
}

static const struct dm_pci_ops ls_pcie_g4_ops = {
	.read_config	= ls_pcie_g4_read_config,
	.write_config	= ls_pcie_g4_write_config,
};

static const struct udevice_id ls_pcie_g4_ids[] = {
	{ .compatible = "fsl,lx2160a-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_layerscape_gen4) = {
	.name = "pcie_layerscape_gen4",
	.id = UCLASS_PCI,
	.of_match = ls_pcie_g4_ids,
	.ops = &ls_pcie_g4_ops,
	.probe	= ls_pcie_g4_probe,
	.priv_auto_alloc_size = sizeof(struct ls_pcie_g4),
};
