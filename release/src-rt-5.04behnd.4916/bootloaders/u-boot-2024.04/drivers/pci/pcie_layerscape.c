// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2020 NXP
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 */

#include <common.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <errno.h>
#include <malloc.h>
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3) || \
	defined(CONFIG_ARM)
#include <asm/arch/clock.h>
#endif
#include "pcie_layerscape.h"

DECLARE_GLOBAL_DATA_PTR;

LIST_HEAD(ls_pcie_list);

unsigned int dbi_readl(struct ls_pcie *pcie, unsigned int offset)
{
	return in_le32(pcie->dbi + offset);
}

void dbi_writel(struct ls_pcie *pcie, unsigned int value, unsigned int offset)
{
	out_le32(pcie->dbi + offset, value);
}

unsigned int ctrl_readl(struct ls_pcie *pcie, unsigned int offset)
{
	if (pcie->big_endian)
		return in_be32(pcie->ctrl + offset);
	else
		return in_le32(pcie->ctrl + offset);
}

void ctrl_writel(struct ls_pcie *pcie, unsigned int value,
		 unsigned int offset)
{
	if (pcie->big_endian)
		out_be32(pcie->ctrl + offset, value);
	else
		out_le32(pcie->ctrl + offset, value);
}

void ls_pcie_dbi_ro_wr_en(struct ls_pcie *pcie)
{
	u32 reg, val;

	reg = PCIE_MISC_CONTROL_1_OFF;
	val = dbi_readl(pcie, reg);
	val |= PCIE_DBI_RO_WR_EN;
	dbi_writel(pcie, val, reg);
}

void ls_pcie_dbi_ro_wr_dis(struct ls_pcie *pcie)
{
	u32 reg, val;

	reg = PCIE_MISC_CONTROL_1_OFF;
	val = dbi_readl(pcie, reg);
	val &= ~PCIE_DBI_RO_WR_EN;
	dbi_writel(pcie, val, reg);
}

static int ls_pcie_ltssm(struct ls_pcie *pcie)
{
	u32 state;
	uint svr;

	svr = get_svr();
	if (((svr >> SVR_VAR_PER_SHIFT) & SVR_LS102XA_MASK) == SVR_LS102XA) {
		state = ctrl_readl(pcie, LS1021_PEXMSCPORTSR(pcie->idx));
		state = (state >> LS1021_LTSSM_STATE_SHIFT) & LTSSM_STATE_MASK;
	} else {
		state = ctrl_readl(pcie, PCIE_PF_DBG) & LTSSM_STATE_MASK;
	}

	return state;
}

int ls_pcie_link_up(struct ls_pcie *pcie)
{
	int ltssm;

	ltssm = ls_pcie_ltssm(pcie);
	if (ltssm < LTSSM_PCIE_L0)
		return 0;

	return 1;
}

void ls_pcie_atu_outbound_set(struct ls_pcie *pcie, int idx, int type,
			      u64 phys, u64 bus_addr, u64 size)
{
	dbi_writel(pcie, PCIE_ATU_REGION_OUTBOUND | idx, PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, (u32)phys, PCIE_ATU_LOWER_BASE);
	dbi_writel(pcie, phys >> 32, PCIE_ATU_UPPER_BASE);
	dbi_writel(pcie, (u32)phys + size - 1, PCIE_ATU_LIMIT);
	dbi_writel(pcie, (u32)bus_addr, PCIE_ATU_LOWER_TARGET);
	dbi_writel(pcie, bus_addr >> 32, PCIE_ATU_UPPER_TARGET);
	dbi_writel(pcie, type, PCIE_ATU_CR1);
	dbi_writel(pcie, PCIE_ATU_ENABLE, PCIE_ATU_CR2);
}

/* Use bar match mode and MEM type as default */
void ls_pcie_atu_inbound_set(struct ls_pcie *pcie, u32 pf, u32 vf_flag,
			     int type, int idx, int bar, u64 phys)
{
	dbi_writel(pcie, PCIE_ATU_REGION_INBOUND | idx, PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, (u32)phys, PCIE_ATU_LOWER_TARGET);
	dbi_writel(pcie, phys >> 32, PCIE_ATU_UPPER_TARGET);
	dbi_writel(pcie, type | PCIE_ATU_FUNC_NUM(pf), PCIE_ATU_CR1);
	dbi_writel(pcie, PCIE_ATU_ENABLE | PCIE_ATU_BAR_MODE_ENABLE |
		   (vf_flag ? PCIE_ATU_FUNC_NUM_MATCH_EN : 0) |
		   (vf_flag ? PCIE_ATU_VFBAR_MATCH_MODE_EN : 0) |
		   PCIE_ATU_BAR_NUM(bar), PCIE_ATU_CR2);
}

void ls_pcie_dump_atu(struct ls_pcie *pcie, u32 win_num, u32 type)
{
	int win_idx;

	for (win_idx = 0; win_idx < win_num; win_idx++) {
		dbi_writel(pcie, type | win_idx, PCIE_ATU_VIEWPORT);
		debug("iATU%d:\n", win_idx);
		debug("\tLOWER PHYS 0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_LOWER_BASE));
		debug("\tUPPER PHYS 0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_UPPER_BASE));
		if (type == PCIE_ATU_REGION_OUTBOUND) {
			debug("\tLOWER BUS  0x%08x\n",
			      dbi_readl(pcie, PCIE_ATU_LOWER_TARGET));
			debug("\tUPPER BUS  0x%08x\n",
			      dbi_readl(pcie, PCIE_ATU_UPPER_TARGET));
			debug("\tLIMIT      0x%08x\n",
			      dbi_readl(pcie, PCIE_ATU_LIMIT));
		}
		debug("\tCR1        0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_CR1));
		debug("\tCR2        0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_CR2));
	}
}
