// SPDX-License-Identifier: GPL-2.0+ OR X11
/*
 * Copyright 2019 NXP
 *
 * PCIe DM U-Boot driver for Freescale PowerPC SoCs
 * Author: Hou Zhiqiang <Zhiqiang.Hou@nxp.com>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include "pcie_fsl.h"

LIST_HEAD(fsl_pcie_list);

static int fsl_pcie_link_up(struct fsl_pcie *pcie);

static int fsl_pcie_addr_valid(struct fsl_pcie *pcie, pci_dev_t bdf)
{
	struct udevice *bus = pcie->bus;

	if (!pcie->enabled)
		return -ENXIO;

	if (PCI_BUS(bdf) < bus->seq)
		return -EINVAL;

	if (PCI_BUS(bdf) > bus->seq && (!fsl_pcie_link_up(pcie) || pcie->mode))
		return -EINVAL;

	if (PCI_BUS(bdf) == bus->seq && (PCI_DEV(bdf) > 0 || PCI_FUNC(bdf) > 0))
		return -EINVAL;

	if (PCI_BUS(bdf) == (bus->seq + 1) && (PCI_DEV(bdf) > 0))
		return -EINVAL;

	return 0;
}

static int fsl_pcie_read_config(struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	struct fsl_pcie *pcie = dev_get_priv(bus);
	ccsr_fsl_pci_t *regs = pcie->regs;
	u32 val;

	if (fsl_pcie_addr_valid(pcie, bdf)) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	bdf = bdf - PCI_BDF(bus->seq, 0, 0);
	val = bdf | (offset & 0xfc) | ((offset & 0xf00) << 16) | 0x80000000;
	out_be32(&regs->cfg_addr, val);

	sync();

	switch (size) {
	case PCI_SIZE_8:
		*valuep = in_8((u8 *)&regs->cfg_data + (offset & 3));
		break;
	case PCI_SIZE_16:
		*valuep = in_le16((u16 *)((u8 *)&regs->cfg_data +
			  (offset & 2)));
		break;
	case PCI_SIZE_32:
		*valuep = in_le32(&regs->cfg_data);
		break;
	}

	return 0;
}

static int fsl_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	struct fsl_pcie *pcie = dev_get_priv(bus);
	ccsr_fsl_pci_t *regs = pcie->regs;
	u32 val;
	u8 val_8;
	u16 val_16;
	u32 val_32;

	if (fsl_pcie_addr_valid(pcie, bdf))
		return 0;

	bdf = bdf - PCI_BDF(bus->seq, 0, 0);
	val = bdf | (offset & 0xfc) | ((offset & 0xf00) << 16) | 0x80000000;
	out_be32(&regs->cfg_addr, val);

	sync();

	switch (size) {
	case PCI_SIZE_8:
		val_8 = value;
		out_8((u8 *)&regs->cfg_data + (offset & 3), val_8);
		break;
	case PCI_SIZE_16:
		val_16 = value;
		out_le16((u16 *)((u8 *)&regs->cfg_data + (offset & 2)), val_16);
		break;
	case PCI_SIZE_32:
		val_32 = value;
		out_le32(&regs->cfg_data, val_32);
		break;
	}

	return 0;
}

static int fsl_pcie_hose_read_config(struct fsl_pcie *pcie, uint offset,
				     ulong *valuep, enum pci_size_t size)
{
	int ret;
	struct udevice *bus = pcie->bus;

	ret = fsl_pcie_read_config(bus, PCI_BDF(bus->seq, 0, 0),
				   offset, valuep, size);

	return ret;
}

static int fsl_pcie_hose_write_config(struct fsl_pcie *pcie, uint offset,
				      ulong value, enum pci_size_t size)
{
	struct udevice *bus = pcie->bus;

	return fsl_pcie_write_config(bus, PCI_BDF(bus->seq, 0, 0),
				     offset, value, size);
}

static int fsl_pcie_hose_read_config_byte(struct fsl_pcie *pcie, uint offset,
					  u8 *valuep)
{
	ulong val;
	int ret;

	ret = fsl_pcie_hose_read_config(pcie, offset, &val, PCI_SIZE_8);
	*valuep = val;

	return ret;
}

static int fsl_pcie_hose_read_config_word(struct fsl_pcie *pcie, uint offset,
					  u16 *valuep)
{
	ulong val;
	int ret;

	ret = fsl_pcie_hose_read_config(pcie, offset, &val, PCI_SIZE_16);
	*valuep = val;

	return ret;
}

static int fsl_pcie_hose_read_config_dword(struct fsl_pcie *pcie, uint offset,
					   u32 *valuep)
{
	ulong val;
	int ret;

	ret = fsl_pcie_hose_read_config(pcie, offset, &val, PCI_SIZE_32);
	*valuep = val;

	return ret;
}

static int fsl_pcie_hose_write_config_byte(struct fsl_pcie *pcie, uint offset,
					   u8 value)
{
	return fsl_pcie_hose_write_config(pcie, offset, value, PCI_SIZE_8);
}

static int fsl_pcie_hose_write_config_word(struct fsl_pcie *pcie, uint offset,
					   u16 value)
{
	return fsl_pcie_hose_write_config(pcie, offset, value, PCI_SIZE_16);
}

static int fsl_pcie_hose_write_config_dword(struct fsl_pcie *pcie, uint offset,
					    u32 value)
{
	return fsl_pcie_hose_write_config(pcie, offset, value, PCI_SIZE_32);
}

static int fsl_pcie_link_up(struct fsl_pcie *pcie)
{
	ccsr_fsl_pci_t *regs = pcie->regs;
	u16 ltssm;

	if (pcie->block_rev >= PEX_IP_BLK_REV_3_0) {
		ltssm = (in_be32(&regs->pex_csr0)
			& PEX_CSR0_LTSSM_MASK) >> PEX_CSR0_LTSSM_SHIFT;
		return ltssm == LTSSM_L0_REV3;
	}

	fsl_pcie_hose_read_config_word(pcie, PCI_LTSSM, &ltssm);

	return ltssm == LTSSM_L0;
}

static bool fsl_pcie_is_agent(struct fsl_pcie *pcie)
{
	u8 header_type;

	fsl_pcie_hose_read_config_byte(pcie, PCI_HEADER_TYPE, &header_type);

	return (header_type & 0x7f) == PCI_HEADER_TYPE_NORMAL;
}

static int fsl_pcie_setup_law(struct fsl_pcie *pcie)
{
	struct pci_region *io, *mem, *pref;

	pci_get_regions(pcie->bus, &io, &mem, &pref);

	if (mem)
		set_next_law(mem->phys_start,
			     law_size_bits(mem->size),
			     pcie->law_trgt_if);

	if (io)
		set_next_law(io->phys_start,
			     law_size_bits(io->size),
			     pcie->law_trgt_if);

	return 0;
}

static void fsl_pcie_config_ready(struct fsl_pcie *pcie)
{
	ccsr_fsl_pci_t *regs = pcie->regs;

	if (pcie->block_rev >= PEX_IP_BLK_REV_3_0) {
		setbits_be32(&regs->config, FSL_PCIE_V3_CFG_RDY);
		return;
	}

	fsl_pcie_hose_write_config_byte(pcie, FSL_PCIE_CFG_RDY, 0x1);
}

static int fsl_pcie_setup_outbound_win(struct fsl_pcie *pcie, int idx,
				       int type, u64 phys, u64 bus_addr,
				       pci_size_t size)
{
	ccsr_fsl_pci_t *regs = pcie->regs;
	pot_t *po = &regs->pot[idx];
	u32 war, sz;

	if (idx < 0)
		return -EINVAL;

	out_be32(&po->powbar, phys >> 12);
	out_be32(&po->potar, bus_addr >> 12);
#ifdef CONFIG_SYS_PCI_64BIT
	out_be32(&po->potear, bus_addr >> 44);
#else
	out_be32(&po->potear, 0);
#endif

	sz = (__ilog2_u64((u64)size) - 1);
	war = POWAR_EN | sz;

	if (type == PCI_REGION_IO)
		war |= POWAR_IO_READ | POWAR_IO_WRITE;
	else
		war |= POWAR_MEM_READ | POWAR_MEM_WRITE;

	out_be32(&po->powar, war);

	return 0;
}

static int fsl_pcie_setup_inbound_win(struct fsl_pcie *pcie, int idx,
				      bool pf, u64 phys, u64 bus_addr,
				      pci_size_t size)
{
	ccsr_fsl_pci_t *regs = pcie->regs;
	pit_t *pi = &regs->pit[idx];
	u32 sz = (__ilog2_u64(size) - 1);
	u32 flag = PIWAR_LOCAL;

	if (idx < 0)
		return -EINVAL;

	out_be32(&pi->pitar, phys >> 12);
	out_be32(&pi->piwbar, bus_addr >> 12);

#ifdef CONFIG_SYS_PCI_64BIT
	out_be32(&pi->piwbear, bus_addr >> 44);
#else
	out_be32(&pi->piwbear, 0);
#endif

	if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_A005434))
		flag = 0;

	flag |= PIWAR_EN | PIWAR_READ_SNOOP | PIWAR_WRITE_SNOOP;
	if (pf)
		flag |= PIWAR_PF;
	out_be32(&pi->piwar, flag | sz);

	return 0;
}

static int fsl_pcie_setup_outbound_wins(struct fsl_pcie *pcie)
{
	struct pci_region *io, *mem, *pref;
	int idx = 1; /* skip 0 */

	pci_get_regions(pcie->bus, &io, &mem, &pref);

	if (io)
		/* ATU : OUTBOUND : IO */
		fsl_pcie_setup_outbound_win(pcie, idx++,
					    PCI_REGION_IO,
					    io->phys_start,
					    io->bus_start,
					    io->size);

	if (mem)
		/* ATU : OUTBOUND : MEM */
		fsl_pcie_setup_outbound_win(pcie, idx++,
					    PCI_REGION_MEM,
					    mem->phys_start,
					    mem->bus_start,
					    mem->size);
	return 0;
}

static int fsl_pcie_setup_inbound_wins(struct fsl_pcie *pcie)
{
	phys_addr_t phys_start = CONFIG_SYS_PCI_MEMORY_PHYS;
	pci_addr_t bus_start = CONFIG_SYS_PCI_MEMORY_BUS;
	u64 sz = min((u64)gd->ram_size, (1ull << 32));
	pci_size_t pci_sz;
	int idx;

	if (pcie->block_rev >= PEX_IP_BLK_REV_2_2)
		idx = 2;
	else
		idx = 3;

	pci_sz = 1ull << __ilog2_u64(sz);

	dev_dbg(pcie->bus, "R0 bus_start: %llx phys_start: %llx size: %llx\n",
		(u64)bus_start, (u64)phys_start, (u64)sz);

	/* if we aren't an exact power of two match, pci_sz is smaller
	 * round it up to the next power of two.  We report the actual
	 * size to pci region tracking.
	 */
	if (pci_sz != sz)
		sz = 2ull << __ilog2_u64(sz);

	fsl_pcie_setup_inbound_win(pcie, idx--, true,
				   CONFIG_SYS_PCI_MEMORY_PHYS,
				   CONFIG_SYS_PCI_MEMORY_BUS, sz);
#if defined(CONFIG_PHYS_64BIT) && defined(CONFIG_SYS_PCI_64BIT)
	/*
	 * On 64-bit capable systems, set up a mapping for all of DRAM
	 * in high pci address space.
	 */
	pci_sz = 1ull << __ilog2_u64(gd->ram_size);
	/* round up to the next largest power of two */
	if (gd->ram_size > pci_sz)
		pci_sz = 1ull << (__ilog2_u64(gd->ram_size) + 1);

	dev_dbg(pcie->bus, "R64 bus_start: %llx phys_start: %llx size: %llx\n",
		(u64)CONFIG_SYS_PCI64_MEMORY_BUS,
		(u64)CONFIG_SYS_PCI_MEMORY_PHYS, (u64)pci_sz);

	fsl_pcie_setup_inbound_win(pcie, idx--, true,
				   CONFIG_SYS_PCI_MEMORY_PHYS,
				   CONFIG_SYS_PCI64_MEMORY_BUS, pci_sz);
#endif

	return 0;
}

static int fsl_pcie_init_atmu(struct fsl_pcie *pcie)
{
	fsl_pcie_setup_outbound_wins(pcie);
	fsl_pcie_setup_inbound_wins(pcie);

	return 0;
}

static int fsl_pcie_init_port(struct fsl_pcie *pcie)
{
	ccsr_fsl_pci_t *regs = pcie->regs;
	u32 val_32;
	u16 val_16;

	fsl_pcie_init_atmu(pcie);

	if (IS_ENABLED(CONFIG_FSL_PCIE_DISABLE_ASPM)) {
		val_32 = 0;
		fsl_pcie_hose_read_config_dword(pcie, PCI_LCR, &val_32);
		val_32 &= ~0x03;
		fsl_pcie_hose_write_config_dword(pcie, PCI_LCR, val_32);
		udelay(1);
	}

	if (IS_ENABLED(CONFIG_FSL_PCIE_RESET)) {
		u16 ltssm;
		int i;

		if (pcie->block_rev >= PEX_IP_BLK_REV_3_0) {
			/* assert PCIe reset */
			setbits_be32(&regs->pdb_stat, 0x08000000);
			(void)in_be32(&regs->pdb_stat);
			udelay(1000);
			/* clear PCIe reset */
			clrbits_be32(&regs->pdb_stat, 0x08000000);
			asm("sync;isync");
			for (i = 0; i < 100 && !fsl_pcie_link_up(pcie); i++)
				udelay(1000);
		} else {
			fsl_pcie_hose_read_config_word(pcie, PCI_LTSSM, &ltssm);
			if (ltssm == 1) {
				/* assert PCIe reset */
				setbits_be32(&regs->pdb_stat, 0x08000000);
				(void)in_be32(&regs->pdb_stat);
				udelay(100);
				/* clear PCIe reset */
				clrbits_be32(&regs->pdb_stat, 0x08000000);
				asm("sync;isync");
				for (i = 0; i < 100 &&
				     !fsl_pcie_link_up(pcie); i++)
					udelay(1000);
			}
		}
	}

	if (IS_ENABLED(CONFIG_SYS_P4080_ERRATUM_PCIE_A003) &&
	    !fsl_pcie_link_up(pcie)) {
		serdes_corenet_t *srds_regs;

		srds_regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
		val_32 = in_be32(&srds_regs->srdspccr0);

		if ((val_32 >> 28) == 3) {
			int i;

			out_be32(&srds_regs->srdspccr0, 2 << 28);
			setbits_be32(&regs->pdb_stat, 0x08000000);
			in_be32(&regs->pdb_stat);
			udelay(100);
			clrbits_be32(&regs->pdb_stat, 0x08000000);
			asm("sync;isync");
			for (i = 0; i < 100 && !fsl_pcie_link_up(pcie); i++)
				udelay(1000);
		}
	}

	/*
	 * The Read-Only Write Enable bit defaults to 1 instead of 0.
	 * Set to 0 to protect the read-only registers.
	 */
	if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_A007815))
		clrbits_be32(&regs->dbi_ro_wr_en, 0x01);

	/*
	 * Enable All Error Interrupts except
	 * - Master abort (pci)
	 * - Master PERR (pci)
	 * - ICCA (PCIe)
	 */
	out_be32(&regs->peer, ~0x20140);

	/* set URR, FER, NFER (but not CER) */
	fsl_pcie_hose_read_config_dword(pcie, PCI_DCR, &val_32);
	val_32 |= 0xf000e;
	fsl_pcie_hose_write_config_dword(pcie, PCI_DCR, val_32);

	/* Clear all error indications */
	out_be32(&regs->pme_msg_det, 0xffffffff);
	out_be32(&regs->pme_msg_int_en, 0xffffffff);
	out_be32(&regs->pedr, 0xffffffff);

	fsl_pcie_hose_read_config_word(pcie, PCI_DSR, &val_16);
	if (val_16)
		fsl_pcie_hose_write_config_word(pcie, PCI_DSR, 0xffff);

	fsl_pcie_hose_read_config_word(pcie, PCI_SEC_STATUS, &val_16);
	if (val_16)
		fsl_pcie_hose_write_config_word(pcie, PCI_SEC_STATUS, 0xffff);

	return 0;
}

static int fsl_pcie_fixup_classcode(struct fsl_pcie *pcie)
{
	ccsr_fsl_pci_t *regs = pcie->regs;
	u32 val;

	setbits_be32(&regs->dbi_ro_wr_en, 0x01);
	fsl_pcie_hose_read_config_dword(pcie, PCI_CLASS_REVISION, &val);
	val &= 0xff;
	val |= PCI_CLASS_BRIDGE_PCI << 16;
	fsl_pcie_hose_write_config_dword(pcie, PCI_CLASS_REVISION, val);
	clrbits_be32(&regs->dbi_ro_wr_en, 0x01);

	return 0;
}

static int fsl_pcie_init_rc(struct fsl_pcie *pcie)
{
	return fsl_pcie_fixup_classcode(pcie);
}

static int fsl_pcie_init_ep(struct fsl_pcie *pcie)
{
	fsl_pcie_config_ready(pcie);

	return 0;
}

static int fsl_pcie_probe(struct udevice *dev)
{
	struct fsl_pcie *pcie = dev_get_priv(dev);
	ccsr_fsl_pci_t *regs = pcie->regs;
	u16 val_16;

	pcie->bus = dev;
	pcie->block_rev = in_be32(&regs->block_rev1);

	list_add(&pcie->list, &fsl_pcie_list);
	pcie->enabled = is_serdes_configured(PCIE1 + pcie->idx);
	if (!pcie->enabled) {
		printf("PCIe%d: %s disabled\n", pcie->idx, dev->name);
		return 0;
	}

	fsl_pcie_setup_law(pcie);

	pcie->mode = fsl_pcie_is_agent(pcie);

	fsl_pcie_init_port(pcie);

	printf("PCIe%d: %s ", pcie->idx, dev->name);

	if (pcie->mode) {
		printf("Endpoint");
		fsl_pcie_init_ep(pcie);
	} else {
		printf("Root Complex");
		fsl_pcie_init_rc(pcie);
	}

	if (!fsl_pcie_link_up(pcie)) {
		printf(": %s\n", pcie->mode ? "undetermined link" : "no link");
		return 0;
	}

	fsl_pcie_hose_read_config_word(pcie, PCI_LSR, &val_16);
	printf(": x%d gen%d\n", (val_16 & 0x3f0) >> 4, (val_16 & 0xf));

	return 0;
}

static int fsl_pcie_ofdata_to_platdata(struct udevice *dev)
{
	struct fsl_pcie *pcie = dev_get_priv(dev);
	int ret;

	pcie->regs = dev_remap_addr(dev);
	if (!pcie->regs) {
		pr_err("\"reg\" resource not found\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "law_trgt_if", &pcie->law_trgt_if);
	if (ret < 0) {
		pr_err("\"law_trgt_if\" not found\n");
		return ret;
	}

	pcie->idx = (dev_read_addr(dev) - 0xffe240000) / 0x10000;

	return 0;
}

static const struct dm_pci_ops fsl_pcie_ops = {
	.read_config	= fsl_pcie_read_config,
	.write_config	= fsl_pcie_write_config,
};

static const struct udevice_id fsl_pcie_ids[] = {
	{ .compatible = "fsl,pcie-t2080" },
	{ }
};

U_BOOT_DRIVER(fsl_pcie) = {
	.name = "fsl_pcie",
	.id = UCLASS_PCI,
	.of_match = fsl_pcie_ids,
	.ops = &fsl_pcie_ops,
	.ofdata_to_platdata = fsl_pcie_ofdata_to_platdata,
	.probe = fsl_pcie_probe,
	.priv_auto_alloc_size = sizeof(struct fsl_pcie),
};
