// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright Altera Corporation (C) 2014-2015
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <div64.h>
#include <ram.h>
#include <reset.h>
#include <watchdog.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>

#include "sequencer.h"

#ifdef CONFIG_SPL_BUILD

struct altera_gen5_sdram_priv {
	struct ram_info info;
};

struct altera_gen5_sdram_platdata {
	struct socfpga_sdr *sdr;
};

struct sdram_prot_rule {
	u32	sdram_start;	/* SDRAM start address */
	u32	sdram_end;	/* SDRAM end address */
	u32	rule;		/* SDRAM protection rule number: 0-19 */
	int	valid;		/* Rule valid or not? 1 - valid, 0 not*/

	u32	security;
	u32	portmask;
	u32	result;
	u32	lo_prot_id;
	u32	hi_prot_id;
};

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

static unsigned long sdram_calculate_size(struct socfpga_sdr_ctrl *sdr_ctrl);

/**
 * get_errata_rows() - Up the number of DRAM rows to cover entire address space
 * @cfg:	SDRAM controller configuration data
 *
 * SDRAM Failure happens when accessing non-existent memory. Artificially
 * increase the number of rows so that the memory controller thinks it has
 * 4GB of RAM. This function returns such amount of rows.
 */
static int get_errata_rows(const struct socfpga_sdram_config *cfg)
{
	/* Define constant for 4G memory - used for SDRAM errata workaround */
#define MEMSIZE_4G	(4ULL * 1024ULL * 1024ULL * 1024ULL)
	const unsigned long long memsize = MEMSIZE_4G;
	const unsigned int cs =
		((cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_CSBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB) + 1;
	const unsigned int rows =
		(cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_ROWBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_ROWBITS_LSB;
	const unsigned int banks =
		(cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_BANKBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_BANKBITS_LSB;
	const unsigned int cols =
		(cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_COLBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_COLBITS_LSB;
	const unsigned int width = 8;

	unsigned long long newrows;
	int bits, inewrowslog2;

	debug("workaround rows - memsize %lld\n", memsize);
	debug("workaround rows - cs        %d\n", cs);
	debug("workaround rows - width     %d\n", width);
	debug("workaround rows - rows      %d\n", rows);
	debug("workaround rows - banks     %d\n", banks);
	debug("workaround rows - cols      %d\n", cols);

	newrows = lldiv(memsize, cs * (width / 8));
	debug("rows workaround - term1 %lld\n", newrows);

	newrows = lldiv(newrows, (1 << banks) * (1 << cols));
	debug("rows workaround - term2 %lld\n", newrows);

	/*
	 * Compute the hamming weight - same as number of bits set.
	 * Need to see if result is ordinal power of 2 before
	 * attempting log2 of result.
	 */
	bits = generic_hweight32(newrows);

	debug("rows workaround - bits %d\n", bits);

	if (bits != 1) {
		printf("SDRAM workaround failed, bits set %d\n", bits);
		return rows;
	}

	if (newrows > UINT_MAX) {
		printf("SDRAM workaround rangecheck failed, %lld\n", newrows);
		return rows;
	}

	inewrowslog2 = __ilog2(newrows);

	debug("rows workaround - ilog2 %d, %lld\n", inewrowslog2, newrows);

	if (inewrowslog2 == -1) {
		printf("SDRAM workaround failed, newrows %lld\n", newrows);
		return rows;
	}

	return inewrowslog2;
}

/* SDRAM protection rules vary from 0-19, a total of 20 rules. */
static void sdram_set_rule(struct socfpga_sdr_ctrl *sdr_ctrl,
			   struct sdram_prot_rule *prule)
{
	u32 lo_addr_bits;
	u32 hi_addr_bits;
	int ruleno = prule->rule;

	/* Select the rule */
	writel(ruleno, &sdr_ctrl->prot_rule_rdwr);

	/* Obtain the address bits */
	lo_addr_bits = prule->sdram_start >> 20ULL;
	hi_addr_bits = (prule->sdram_end - 1) >> 20ULL;

	debug("sdram set rule start %x, %d\n", lo_addr_bits,
	      prule->sdram_start);
	debug("sdram set rule end   %x, %d\n", hi_addr_bits,
	      prule->sdram_end);

	/* Set rule addresses */
	writel(lo_addr_bits | (hi_addr_bits << 12), &sdr_ctrl->prot_rule_addr);

	/* Set rule protection ids */
	writel(prule->lo_prot_id | (prule->hi_prot_id << 12),
	       &sdr_ctrl->prot_rule_id);

	/* Set the rule data */
	writel(prule->security | (prule->valid << 2) |
	       (prule->portmask << 3) | (prule->result << 13),
	       &sdr_ctrl->prot_rule_data);

	/* write the rule */
	writel(ruleno | (1 << 5), &sdr_ctrl->prot_rule_rdwr);

	/* Set rule number to 0 by default */
	writel(0, &sdr_ctrl->prot_rule_rdwr);
}

static void sdram_get_rule(struct socfpga_sdr_ctrl *sdr_ctrl,
			   struct sdram_prot_rule *prule)
{
	u32 addr;
	u32 id;
	u32 data;
	int ruleno = prule->rule;

	/* Read the rule */
	writel(ruleno, &sdr_ctrl->prot_rule_rdwr);
	writel(ruleno | (1 << 6), &sdr_ctrl->prot_rule_rdwr);

	/* Get the addresses */
	addr = readl(&sdr_ctrl->prot_rule_addr);
	prule->sdram_start = (addr & 0xFFF) << 20;
	prule->sdram_end = ((addr >> 12) & 0xFFF) << 20;

	/* Get the configured protection IDs */
	id = readl(&sdr_ctrl->prot_rule_id);
	prule->lo_prot_id = id & 0xFFF;
	prule->hi_prot_id = (id >> 12) & 0xFFF;

	/* Get protection data */
	data = readl(&sdr_ctrl->prot_rule_data);

	prule->security = data & 0x3;
	prule->valid = (data >> 2) & 0x1;
	prule->portmask = (data >> 3) & 0x3FF;
	prule->result = (data >> 13) & 0x1;
}

static void
sdram_set_protection_config(struct socfpga_sdr_ctrl *sdr_ctrl,
			    const u32 sdram_start, const u32 sdram_end)
{
	struct sdram_prot_rule rule;
	int rules;

	/* Start with accepting all SDRAM transaction */
	writel(0x0, &sdr_ctrl->protport_default);

	/* Clear all protection rules for warm boot case */
	memset(&rule, 0, sizeof(rule));

	for (rules = 0; rules < 20; rules++) {
		rule.rule = rules;
		sdram_set_rule(sdr_ctrl, &rule);
	}

	/* new rule: accept SDRAM */
	rule.sdram_start = sdram_start;
	rule.sdram_end = sdram_end;
	rule.lo_prot_id = 0x0;
	rule.hi_prot_id = 0xFFF;
	rule.portmask = 0x3FF;
	rule.security = 0x3;
	rule.result = 0;
	rule.valid = 1;
	rule.rule = 0;

	/* set new rule */
	sdram_set_rule(sdr_ctrl, &rule);

	/* default rule: reject everything */
	writel(0x3ff, &sdr_ctrl->protport_default);
}

static void sdram_dump_protection_config(struct socfpga_sdr_ctrl *sdr_ctrl)
{
	struct sdram_prot_rule rule;
	int rules;

	debug("SDRAM Prot rule, default %x\n",
	      readl(&sdr_ctrl->protport_default));

	for (rules = 0; rules < 20; rules++) {
		rule.rule = rules;
		sdram_get_rule(sdr_ctrl, &rule);
		debug("Rule %d, rules ...\n", rules);
		debug("    sdram start %x\n", rule.sdram_start);
		debug("    sdram end   %x\n", rule.sdram_end);
		debug("    low prot id %d, hi prot id %d\n",
		      rule.lo_prot_id,
		      rule.hi_prot_id);
		debug("    portmask %x\n", rule.portmask);
		debug("    security %d\n", rule.security);
		debug("    result %d\n", rule.result);
		debug("    valid %d\n", rule.valid);
	}
}

/**
 * sdram_write_verify() - write to register and verify the write.
 * @addr:	Register address
 * @val:	Value to be written and verified
 *
 * This function writes to a register, reads back the value and compares
 * the result with the written value to check if the data match.
 */
static unsigned sdram_write_verify(const u32 *addr, const u32 val)
{
	u32 rval;

	debug("   Write - Address 0x%p Data 0x%08x\n", addr, val);
	writel(val, addr);

	debug("   Read and verify...");
	rval = readl(addr);
	if (rval != val) {
		debug("FAIL - Address 0x%p Expected 0x%08x Data 0x%08x\n",
		      addr, val, rval);
		return -EINVAL;
	}

	debug("correct!\n");
	return 0;
}

/**
 * sdr_get_ctrlcfg() - Get the value of DRAM CTRLCFG register
 * @cfg:	SDRAM controller configuration data
 *
 * Return the value of DRAM CTRLCFG register.
 */
static u32 sdr_get_ctrlcfg(const struct socfpga_sdram_config *cfg)
{
	const u32 csbits =
		((cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_CSBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB) + 1;
	u32 addrorder =
		(cfg->ctrl_cfg & SDR_CTRLGRP_CTRLCFG_ADDRORDER_MASK) >>
			SDR_CTRLGRP_CTRLCFG_ADDRORDER_LSB;

	u32 ctrl_cfg = cfg->ctrl_cfg;

	/*
	 * SDRAM Failure When Accessing Non-Existent Memory
	 * Set the addrorder field of the SDRAM control register
	 * based on the CSBITs setting.
	 */
	if (csbits == 1) {
		if (addrorder != 0)
			debug("INFO: Changing address order to 0 (chip, row, bank, column)\n");
		addrorder = 0;
	} else if (csbits == 2) {
		if (addrorder != 2)
			debug("INFO: Changing address order to 2 (row, chip, bank, column)\n");
		addrorder = 2;
	}

	ctrl_cfg &= ~SDR_CTRLGRP_CTRLCFG_ADDRORDER_MASK;
	ctrl_cfg |= addrorder << SDR_CTRLGRP_CTRLCFG_ADDRORDER_LSB;

	return ctrl_cfg;
}

/**
 * sdr_get_addr_rw() - Get the value of DRAM ADDRW register
 * @cfg:	SDRAM controller configuration data
 *
 * Return the value of DRAM ADDRW register.
 */
static u32 sdr_get_addr_rw(const struct socfpga_sdram_config *cfg)
{
	/*
	 * SDRAM Failure When Accessing Non-Existent Memory
	 * Set SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB to
	 * log2(number of chip select bits). Since there's only
	 * 1 or 2 chip selects, log2(1) => 0, and log2(2) => 1,
	 * which is the same as "chip selects" - 1.
	 */
	const int rows = get_errata_rows(cfg);
	u32 dram_addrw = cfg->dram_addrw & ~SDR_CTRLGRP_DRAMADDRW_ROWBITS_MASK;

	return dram_addrw | (rows << SDR_CTRLGRP_DRAMADDRW_ROWBITS_LSB);
}

/**
 * sdr_load_regs() - Load SDRAM controller registers
 * @cfg:	SDRAM controller configuration data
 *
 * This function loads the register values into the SDRAM controller block.
 */
static void sdr_load_regs(struct socfpga_sdr_ctrl *sdr_ctrl,
			  const struct socfpga_sdram_config *cfg)
{
	const u32 ctrl_cfg = sdr_get_ctrlcfg(cfg);
	const u32 dram_addrw = sdr_get_addr_rw(cfg);

	debug("\nConfiguring CTRLCFG\n");
	writel(ctrl_cfg, &sdr_ctrl->ctrl_cfg);

	debug("Configuring DRAMTIMING1\n");
	writel(cfg->dram_timing1, &sdr_ctrl->dram_timing1);

	debug("Configuring DRAMTIMING2\n");
	writel(cfg->dram_timing2, &sdr_ctrl->dram_timing2);

	debug("Configuring DRAMTIMING3\n");
	writel(cfg->dram_timing3, &sdr_ctrl->dram_timing3);

	debug("Configuring DRAMTIMING4\n");
	writel(cfg->dram_timing4, &sdr_ctrl->dram_timing4);

	debug("Configuring LOWPWRTIMING\n");
	writel(cfg->lowpwr_timing, &sdr_ctrl->lowpwr_timing);

	debug("Configuring DRAMADDRW\n");
	writel(dram_addrw, &sdr_ctrl->dram_addrw);

	debug("Configuring DRAMIFWIDTH\n");
	writel(cfg->dram_if_width, &sdr_ctrl->dram_if_width);

	debug("Configuring DRAMDEVWIDTH\n");
	writel(cfg->dram_dev_width, &sdr_ctrl->dram_dev_width);

	debug("Configuring LOWPWREQ\n");
	writel(cfg->lowpwr_eq, &sdr_ctrl->lowpwr_eq);

	debug("Configuring DRAMINTR\n");
	writel(cfg->dram_intr, &sdr_ctrl->dram_intr);

	debug("Configuring STATICCFG\n");
	writel(cfg->static_cfg, &sdr_ctrl->static_cfg);

	debug("Configuring CTRLWIDTH\n");
	writel(cfg->ctrl_width, &sdr_ctrl->ctrl_width);

	debug("Configuring PORTCFG\n");
	writel(cfg->port_cfg, &sdr_ctrl->port_cfg);

	debug("Configuring FIFOCFG\n");
	writel(cfg->fifo_cfg, &sdr_ctrl->fifo_cfg);

	debug("Configuring MPPRIORITY\n");
	writel(cfg->mp_priority, &sdr_ctrl->mp_priority);

	debug("Configuring MPWEIGHT_MPWEIGHT_0\n");
	writel(cfg->mp_weight0, &sdr_ctrl->mp_weight0);
	writel(cfg->mp_weight1, &sdr_ctrl->mp_weight1);
	writel(cfg->mp_weight2, &sdr_ctrl->mp_weight2);
	writel(cfg->mp_weight3, &sdr_ctrl->mp_weight3);

	debug("Configuring MPPACING_MPPACING_0\n");
	writel(cfg->mp_pacing0, &sdr_ctrl->mp_pacing0);
	writel(cfg->mp_pacing1, &sdr_ctrl->mp_pacing1);
	writel(cfg->mp_pacing2, &sdr_ctrl->mp_pacing2);
	writel(cfg->mp_pacing3, &sdr_ctrl->mp_pacing3);

	debug("Configuring MPTHRESHOLDRST_MPTHRESHOLDRST_0\n");
	writel(cfg->mp_threshold0, &sdr_ctrl->mp_threshold0);
	writel(cfg->mp_threshold1, &sdr_ctrl->mp_threshold1);
	writel(cfg->mp_threshold2, &sdr_ctrl->mp_threshold2);

	debug("Configuring PHYCTRL_PHYCTRL_0\n");
	writel(cfg->phy_ctrl0, &sdr_ctrl->phy_ctrl0);

	debug("Configuring CPORTWIDTH\n");
	writel(cfg->cport_width, &sdr_ctrl->cport_width);

	debug("Configuring CPORTWMAP\n");
	writel(cfg->cport_wmap, &sdr_ctrl->cport_wmap);

	debug("Configuring CPORTRMAP\n");
	writel(cfg->cport_rmap, &sdr_ctrl->cport_rmap);

	debug("Configuring RFIFOCMAP\n");
	writel(cfg->rfifo_cmap, &sdr_ctrl->rfifo_cmap);

	debug("Configuring WFIFOCMAP\n");
	writel(cfg->wfifo_cmap, &sdr_ctrl->wfifo_cmap);

	debug("Configuring CPORTRDWR\n");
	writel(cfg->cport_rdwr, &sdr_ctrl->cport_rdwr);

	debug("Configuring DRAMODT\n");
	writel(cfg->dram_odt, &sdr_ctrl->dram_odt);

	debug("Configuring EXTRATIME1\n");
	writel(cfg->extratime1, &sdr_ctrl->extratime1);
}

/**
 * sdram_mmr_init_full() - Function to initialize SDRAM MMR
 * @sdr_phy_reg:	Value of the PHY control register 0
 *
 * Initialize the SDRAM MMR.
 */
int sdram_mmr_init_full(struct socfpga_sdr_ctrl *sdr_ctrl,
			unsigned int sdr_phy_reg)
{
	const struct socfpga_sdram_config *cfg = socfpga_get_sdram_config();
	const unsigned int rows =
		(cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_ROWBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_ROWBITS_LSB;
	int ret;

	writel(rows, &sysmgr_regs->iswgrp_handoff[4]);

	sdr_load_regs(sdr_ctrl, cfg);

	/* saving this value to SYSMGR.ISWGRP.HANDOFF.FPGA2SDR */
	writel(cfg->fpgaport_rst, &sysmgr_regs->iswgrp_handoff[3]);

	/* only enable if the FPGA is programmed */
	if (fpgamgr_test_fpga_ready()) {
		ret = sdram_write_verify(&sdr_ctrl->fpgaport_rst,
					 cfg->fpgaport_rst);
		if (ret)
			return ret;
	}

	/* Restore the SDR PHY Register if valid */
	if (sdr_phy_reg != 0xffffffff)
		writel(sdr_phy_reg, &sdr_ctrl->phy_ctrl0);

	/* Final step - apply configuration changes */
	debug("Configuring STATICCFG\n");
	clrsetbits_le32(&sdr_ctrl->static_cfg,
			SDR_CTRLGRP_STATICCFG_APPLYCFG_MASK,
			1 << SDR_CTRLGRP_STATICCFG_APPLYCFG_LSB);

	sdram_set_protection_config(sdr_ctrl, 0,
				    sdram_calculate_size(sdr_ctrl) - 1);

	sdram_dump_protection_config(sdr_ctrl);

	return 0;
}

/**
 * sdram_calculate_size() - Calculate SDRAM size
 *
 * Calculate SDRAM device size based on SDRAM controller parameters.
 * Size is specified in bytes.
 */
static unsigned long sdram_calculate_size(struct socfpga_sdr_ctrl *sdr_ctrl)
{
	unsigned long temp;
	unsigned long row, bank, col, cs, width;
	const struct socfpga_sdram_config *cfg = socfpga_get_sdram_config();
	const unsigned int csbits =
		((cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_CSBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB) + 1;
	const unsigned int rowbits =
		(cfg->dram_addrw & SDR_CTRLGRP_DRAMADDRW_ROWBITS_MASK) >>
			SDR_CTRLGRP_DRAMADDRW_ROWBITS_LSB;

	temp = readl(&sdr_ctrl->dram_addrw);
	col = (temp & SDR_CTRLGRP_DRAMADDRW_COLBITS_MASK) >>
		SDR_CTRLGRP_DRAMADDRW_COLBITS_LSB;

	/*
	 * SDRAM Failure When Accessing Non-Existent Memory
	 * Use ROWBITS from Quartus/QSys to calculate SDRAM size
	 * since the FB specifies we modify ROWBITs to work around SDRAM
	 * controller issue.
	 */
	row = readl(&sysmgr_regs->iswgrp_handoff[4]);
	if (row == 0)
		row = rowbits;
	/*
	 * If the stored handoff value for rows is greater than
	 * the field width in the sdr.dramaddrw register then
	 * something is very wrong. Revert to using the the #define
	 * value handed off by the SOCEDS tool chain instead of
	 * using a broken value.
	 */
	if (row > 31)
		row = rowbits;

	bank = (temp & SDR_CTRLGRP_DRAMADDRW_BANKBITS_MASK) >>
		SDR_CTRLGRP_DRAMADDRW_BANKBITS_LSB;

	/*
	 * SDRAM Failure When Accessing Non-Existent Memory
	 * Use CSBITs from Quartus/QSys to calculate SDRAM size
	 * since the FB specifies we modify CSBITs to work around SDRAM
	 * controller issue.
	 */
	cs = csbits;

	width = readl(&sdr_ctrl->dram_if_width);

	/* ECC would not be calculated as its not addressible */
	if (width == SDRAM_WIDTH_32BIT_WITH_ECC)
		width = 32;
	if (width == SDRAM_WIDTH_16BIT_WITH_ECC)
		width = 16;

	/* calculate the SDRAM size base on this info */
	temp = 1 << (row + bank + col);
	temp = temp * cs * (width  / 8);

	debug("%s returns %ld\n", __func__, temp);

	return temp;
}

static int altera_gen5_sdram_ofdata_to_platdata(struct udevice *dev)
{
	struct altera_gen5_sdram_platdata *plat = dev->platdata;

	plat->sdr = (struct socfpga_sdr *)devfdt_get_addr_index(dev, 0);
	if (!plat->sdr)
		return -ENODEV;

	return 0;
}

static int altera_gen5_sdram_probe(struct udevice *dev)
{
	int ret;
	unsigned long sdram_size;
	struct altera_gen5_sdram_platdata *plat = dev->platdata;
	struct altera_gen5_sdram_priv *priv = dev_get_priv(dev);
	struct socfpga_sdr_ctrl *sdr_ctrl = &plat->sdr->sdr_ctrl;
	struct reset_ctl_bulk resets;

	ret = reset_get_bulk(dev, &resets);
	if (ret) {
		dev_err(dev, "Can't get reset: %d\n", ret);
		return -ENODEV;
	}
	reset_deassert_bulk(&resets);

	if (sdram_mmr_init_full(sdr_ctrl, 0xffffffff) != 0) {
		puts("SDRAM init failed.\n");
		goto failed;
	}

	debug("SDRAM: Calibrating PHY\n");
	/* SDRAM calibration */
	if (sdram_calibration_full(plat->sdr) == 0) {
		puts("SDRAM calibration failed.\n");
		goto failed;
	}

	sdram_size = sdram_calculate_size(sdr_ctrl);
	debug("SDRAM: %ld MiB\n", sdram_size >> 20);

	/* Sanity check ensure correct SDRAM size specified */
	if (get_ram_size(0, sdram_size) != sdram_size) {
		puts("SDRAM size check failed!\n");
		goto failed;
	}

	priv->info.base = 0;
	priv->info.size = sdram_size;

	return 0;

failed:
	reset_release_bulk(&resets);
	return -ENODEV;
}

static int altera_gen5_sdram_get_info(struct udevice *dev,
				      struct ram_info *info)
{
	struct altera_gen5_sdram_priv *priv = dev_get_priv(dev);

	info->base = priv->info.base;
	info->size = priv->info.size;

	return 0;
}

static struct ram_ops altera_gen5_sdram_ops = {
	.get_info = altera_gen5_sdram_get_info,
};

static const struct udevice_id altera_gen5_sdram_ids[] = {
	{ .compatible = "altr,sdr-ctl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(altera_gen5_sdram) = {
	.name = "altr_sdr_ctl",
	.id = UCLASS_RAM,
	.of_match = altera_gen5_sdram_ids,
	.ops = &altera_gen5_sdram_ops,
	.ofdata_to_platdata = altera_gen5_sdram_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct altera_gen5_sdram_platdata),
	.probe = altera_gen5_sdram_probe,
	.priv_auto_alloc_size = sizeof(struct altera_gen5_sdram_priv),
};

#endif /* CONFIG_SPL_BUILD */
