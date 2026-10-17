// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 * Layerscape PCIe EP driver
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/devres.h>
#include <errno.h>
#include <pci_ep.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <linux/log2.h>
#include "pcie_layerscape.h"

DECLARE_GLOBAL_DATA_PTR;

static void ls_pcie_ep_enable_cfg(struct ls_pcie_ep *pcie_ep)
{
	struct ls_pcie *pcie = pcie_ep->pcie;
	u32 config;

	config = ctrl_readl(pcie,  PCIE_PF_CONFIG);
	config |= PCIE_CONFIG_READY;
	ctrl_writel(pcie, config, PCIE_PF_CONFIG);
}

static int ls_ep_set_bar(struct udevice *dev, uint fn, struct pci_bar *ep_bar)
{
	struct ls_pcie_ep *pcie_ep = dev_get_priv(dev);
	struct ls_pcie *pcie = pcie_ep->pcie;
	dma_addr_t bar_phys = ep_bar->phys_addr;
	enum pci_barno bar = ep_bar->barno;
	u32 reg = PCI_BASE_ADDRESS_0 + (4 * bar);
	int flags = ep_bar->flags;
	int type, idx;
	u64 size;

	idx  = bar;
	/* BAR size is 2^(aperture + 11) */
	size = max_t(size_t, ep_bar->size, FSL_PCIE_EP_MIN_APERTURE);

	if (!(flags & PCI_BASE_ADDRESS_SPACE))
		type = PCIE_ATU_TYPE_MEM;
	else
		type = PCIE_ATU_TYPE_IO;

	ls_pcie_atu_inbound_set(pcie, fn, 0, type, idx, bar, bar_phys);

	dbi_writel(pcie, lower_32_bits(size - 1), reg + PCIE_NO_SRIOV_BAR_BASE);
	dbi_writel(pcie, flags, reg);

	if (flags & PCI_BASE_ADDRESS_MEM_TYPE_64) {
		dbi_writel(pcie, upper_32_bits(size - 1),
			   reg + 4 + PCIE_NO_SRIOV_BAR_BASE);
		dbi_writel(pcie, 0, reg + 4);
	}

	return 0;
}

static struct pci_ep_ops ls_pcie_ep_ops = {
	.set_bar = ls_ep_set_bar,
};

static void ls_pcie_ep_setup_atu(struct ls_pcie_ep *pcie_ep, u32 pf)
{
	struct ls_pcie *pcie = pcie_ep->pcie;
	u32 vf_flag = 0;
	u64 phys = 0;

	phys = CFG_SYS_PCI_EP_MEMORY_BASE + pf * SZ_64M;

	phys = ALIGN(phys, PCIE_BAR0_SIZE);
	/* ATU 0 : INBOUND : map BAR0 */
	ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
				0 + pf * BAR_NUM, 0, phys);
	/* ATU 1 : INBOUND : map BAR1 */
	phys = ALIGN(phys + PCIE_BAR0_SIZE, PCIE_BAR1_SIZE);
	ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
				1 + pf * BAR_NUM, 1, phys);
	/* ATU 2 : INBOUND : map BAR2 */
	phys = ALIGN(phys + PCIE_BAR1_SIZE, PCIE_BAR2_SIZE);
	ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
				2 + pf * BAR_NUM, 2, phys);
	/* ATU 3 : INBOUND : map BAR2 */
	phys = ALIGN(phys + PCIE_BAR2_SIZE, PCIE_BAR4_SIZE);
	ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
				3 + pf * BAR_NUM, 4, phys);

	if (pcie_ep->sriov_flag) {
		vf_flag = 1;
		/* ATU 4 : INBOUND : map BAR0 */
		phys = ALIGN(phys + PCIE_BAR4_SIZE, PCIE_BAR0_SIZE);
		ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
					4 + pf * BAR_NUM, 0, phys);
		/* ATU 5 : INBOUND : map BAR1 */
		phys = ALIGN(phys + PCIE_BAR0_SIZE * PCIE_VF_NUM,
			     PCIE_BAR1_SIZE);
		ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
					5 + pf * BAR_NUM, 1, phys);
		/* ATU 6 : INBOUND : map BAR2 */
		phys = ALIGN(phys + PCIE_BAR1_SIZE * PCIE_VF_NUM,
			     PCIE_BAR2_SIZE);
		ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
					6 + pf * BAR_NUM, 2, phys);
		/* ATU 7 : INBOUND : map BAR4 */
		phys = ALIGN(phys + PCIE_BAR2_SIZE * PCIE_VF_NUM,
			     PCIE_BAR4_SIZE);
		ls_pcie_atu_inbound_set(pcie, pf, vf_flag, PCIE_ATU_TYPE_MEM,
					7 + pf * BAR_NUM, 4, phys);
	}

	/* ATU: OUTBOUND : map MEM */
	ls_pcie_atu_outbound_set(pcie, pf, PCIE_ATU_TYPE_MEM,
				 (u64)pcie_ep->addr_res.start +
				 pf * CFG_SYS_PCI_MEMORY_SIZE,
				 0, CFG_SYS_PCI_MEMORY_SIZE);
}

/* BAR0 and BAR1 are 32bit BAR2 and BAR4 are 64bit */
static void ls_pcie_ep_setup_bar(void *bar_base, int bar, u32 size)
{
	u32 mask;

	/* The least inbound window is 4KiB */
	if (size < SZ_4K)
		mask = 0;
	else
		mask = size - 1;

	switch (bar) {
	case 0:
		writel(mask, bar_base + PCI_BASE_ADDRESS_0);
		break;
	case 1:
		writel(mask, bar_base + PCI_BASE_ADDRESS_1);
		break;
	case 2:
		writel(mask, bar_base + PCI_BASE_ADDRESS_2);
		writel(0, bar_base + PCI_BASE_ADDRESS_3);
		break;
	case 4:
		writel(mask, bar_base + PCI_BASE_ADDRESS_4);
		writel(0, bar_base + PCI_BASE_ADDRESS_5);
		break;
	default:
		break;
	}
}

static void ls_pcie_ep_setup_bars(void *bar_base)
{
	/* BAR0 - 32bit - MEM */
	ls_pcie_ep_setup_bar(bar_base, 0, PCIE_BAR0_SIZE);
	/* BAR1 - 32bit - MEM*/
	ls_pcie_ep_setup_bar(bar_base, 1, PCIE_BAR1_SIZE);
	/* BAR2 - 64bit - MEM */
	ls_pcie_ep_setup_bar(bar_base, 2, PCIE_BAR2_SIZE);
	/* BAR4 - 64bit - MEM */
	ls_pcie_ep_setup_bar(bar_base, 4, PCIE_BAR4_SIZE);
}

static void ls_pcie_ep_setup_vf_bars(void *bar_base)
{
	/* VF BAR0 MASK register at offset 0x19c*/
	bar_base += PCIE_SRIOV_VFBAR0 - PCI_BASE_ADDRESS_0;

	/* VF-BAR0 - 32bit - MEM */
	ls_pcie_ep_setup_bar(bar_base, 0, PCIE_BAR0_SIZE);
	/* VF-BAR1 - 32bit - MEM*/
	ls_pcie_ep_setup_bar(bar_base, 1, PCIE_BAR1_SIZE);
	/* VF-BAR2 - 64bit - MEM */
	ls_pcie_ep_setup_bar(bar_base, 2, PCIE_BAR2_SIZE);
	/* VF-BAR4 - 64bit - MEM */
	ls_pcie_ep_setup_bar(bar_base, 4, PCIE_BAR4_SIZE);
}

static void ls_pcie_setup_ep(struct ls_pcie_ep *pcie_ep)
{
	u32 sriov;
	u32 pf, vf;
	void *bar_base = NULL;
	struct ls_pcie *pcie = pcie_ep->pcie;

	sriov = readl(pcie->dbi + PCIE_SRIOV);
	if (PCI_EXT_CAP_ID(sriov) == PCI_EXT_CAP_ID_SRIOV) {
		pcie_ep->sriov_flag = 1;
		for (pf = 0; pf < PCIE_PF_NUM; pf++) {
			/*
			 * The VF_BARn_REG register's Prefetchable and Type bit
			 * fields are overwritten by a write to VF's BAR Mask
			 * register. Before writing to the VF_BARn_MASK_REG
			 * register, write 0b to the PCIE_MISC_CONTROL_1_OFF
			 * register.
			 */
			writel(0, pcie->dbi + PCIE_MISC_CONTROL_1_OFF);

			bar_base = pcie->dbi +
				   PCIE_MASK_OFFSET(pcie_ep->cfg2_flag, pf,
						    pcie_ep->pf1_offset);

			if (pcie_ep->cfg2_flag) {
				ctrl_writel(pcie,
					    PCIE_LCTRL0_VAL(pf, 0),
					    PCIE_PF_VF_CTRL);
				ls_pcie_ep_setup_bars(bar_base);

				for (vf = 1; vf <= PCIE_VF_NUM; vf++) {
					ctrl_writel(pcie,
						    PCIE_LCTRL0_VAL(pf, vf),
						    PCIE_PF_VF_CTRL);
					ls_pcie_ep_setup_vf_bars(bar_base);
				}
			} else {
				ls_pcie_ep_setup_bars(bar_base);
				ls_pcie_ep_setup_vf_bars(bar_base);
			}

			ls_pcie_ep_setup_atu(pcie_ep, pf);
		}

		if (pcie_ep->cfg2_flag)  /* Disable CFG2 */
			ctrl_writel(pcie, 0, PCIE_PF_VF_CTRL);
	} else {
		ls_pcie_ep_setup_bars(pcie->dbi + PCIE_NO_SRIOV_BAR_BASE);
		ls_pcie_ep_setup_atu(pcie_ep, 0);
	}

	ls_pcie_dump_atu(pcie, PCIE_ATU_REGION_NUM_SRIOV,
			 PCIE_ATU_REGION_INBOUND);

	ls_pcie_ep_enable_cfg(pcie_ep);
}

static int ls_pcie_ep_probe(struct udevice *dev)
{
	struct ls_pcie_ep *pcie_ep = dev_get_priv(dev);
	struct ls_pcie *pcie;
	u16 link_sta;
	int ret;
	u32 svr;

	pcie = devm_kzalloc(dev, sizeof(*pcie), GFP_KERNEL);
	if (!pcie)
		return -ENOMEM;

	pcie_ep->pcie = pcie;

	pcie->dbi = devfdt_get_addr_index_ptr(dev, 0);
	if (!pcie->dbi)
		return -EINVAL;

	pcie->ctrl = devfdt_get_addr_index_ptr(dev, 1);
	if (!pcie->ctrl)
		return -EINVAL;

	ret = fdt_get_named_resource(gd->fdt_blob, dev_of_offset(dev),
				     "reg", "reg-names",
				     "addr_space", &pcie_ep->addr_res);
	if (ret) {
		printf("%s: resource \"addr_space\" not found\n", dev->name);
		return ret;
	}

	pcie->idx = ((unsigned long)pcie->dbi - PCIE_SYS_BASE_ADDR) /
		    PCIE_CCSR_SIZE;

	/* This controller is disabled by RCW */
	if (!is_serdes_configured(PCIE_SRDS_PRTCL(pcie->idx)))
		return 0;

	pcie->big_endian = fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev),
					   "big-endian");

	svr = SVR_SOC_VER(get_svr());

	if (svr == SVR_LX2160A || svr == SVR_LX2162A ||
	    svr == SVR_LX2120A || svr == SVR_LX2080A ||
	    svr == SVR_LX2122A || svr == SVR_LX2082A)
		pcie_ep->pf1_offset = LX2160_PCIE_PF1_OFFSET;
	else
		pcie_ep->pf1_offset = LS_PCIE_PF1_OFFSET;

	if (svr == SVR_LS2080A || svr == SVR_LS2085A)
		pcie_ep->cfg2_flag = 1;
	else
		pcie_ep->cfg2_flag = 0;

	pcie->mode = readb(pcie->dbi + PCI_HEADER_TYPE) & 0x7f;
	if (pcie->mode != PCI_HEADER_TYPE_NORMAL)
		return 0;

	pcie_ep->max_functions = fdtdec_get_int(gd->fdt_blob,
						dev_of_offset(dev),
						"max-functions", 1);
	pcie_ep->num_ib_wins = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					      "num-ib-windows", 8);
	pcie_ep->num_ob_wins = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					      "num-ob-windows", 8);

	printf("PCIe%u: %s %s", PCIE_SRDS_PRTCL(pcie->idx), dev->name,
	       "Endpoint");
	ls_pcie_setup_ep(pcie_ep);

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

static int ls_pcie_ep_remove(struct udevice *dev)
{
	return 0;
}

const struct udevice_id ls_pcie_ep_ids[] = {
	{ .compatible = "fsl,ls-pcie-ep" },
	{ }
};

U_BOOT_DRIVER(pci_layerscape_ep) = {
	.name = "pci_layerscape_ep",
	.id	= UCLASS_PCI_EP,
	.of_match = ls_pcie_ep_ids,
	.ops = &ls_pcie_ep_ops,
	.probe = ls_pcie_ep_probe,
	.remove = ls_pcie_ep_remove,
	.priv_auto	= sizeof(struct ls_pcie_ep),
};
