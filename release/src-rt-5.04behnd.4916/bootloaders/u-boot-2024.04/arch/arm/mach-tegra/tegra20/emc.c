// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <fdtdec.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/apb_misc.h>
#include <asm/arch/clock.h>
#include <asm/arch/emc.h>
#include <asm/arch/tegra.h>

/*
 * The EMC registers have shadow registers.  When the EMC clock is updated
 * in the clock controller, the shadow registers are copied to the active
 * registers, allowing glitchless memory bus frequency changes.
 * This function updates the shadow registers for a new clock frequency,
 * and relies on the clock lock on the emc clock to avoid races between
 * multiple frequency changes
 */

/*
 * This table defines the ordering of the registers provided to
 * tegra_set_mmc()
 * TODO: Convert to fdt version once available
 */
static const unsigned long emc_reg_addr[TEGRA_EMC_NUM_REGS] = {
	0x2c,	/* RC */
	0x30,	/* RFC */
	0x34,	/* RAS */
	0x38,	/* RP */
	0x3c,	/* R2W */
	0x40,	/* W2R */
	0x44,	/* R2P */
	0x48,	/* W2P */
	0x4c,	/* RD_RCD */
	0x50,	/* WR_RCD */
	0x54,	/* RRD */
	0x58,	/* REXT */
	0x5c,	/* WDV */
	0x60,	/* QUSE */
	0x64,	/* QRST */
	0x68,	/* QSAFE */
	0x6c,	/* RDV */
	0x70,	/* REFRESH */
	0x74,	/* BURST_REFRESH_NUM */
	0x78,	/* PDEX2WR */
	0x7c,	/* PDEX2RD */
	0x80,	/* PCHG2PDEN */
	0x84,	/* ACT2PDEN */
	0x88,	/* AR2PDEN */
	0x8c,	/* RW2PDEN */
	0x90,	/* TXSR */
	0x94,	/* TCKE */
	0x98,	/* TFAW */
	0x9c,	/* TRPAB */
	0xa0,	/* TCLKSTABLE */
	0xa4,	/* TCLKSTOP */
	0xa8,	/* TREFBW */
	0xac,	/* QUSE_EXTRA */
	0x114,	/* FBIO_CFG6 */
	0xb0,	/* ODT_WRITE */
	0xb4,	/* ODT_READ */
	0x104,	/* FBIO_CFG5 */
	0x2bc,	/* CFG_DIG_DLL */
	0x2c0,	/* DLL_XFORM_DQS */
	0x2c4,	/* DLL_XFORM_QUSE */
	0x2e0,	/* ZCAL_REF_CNT */
	0x2e4,	/* ZCAL_WAIT_CNT */
	0x2a8,	/* AUTO_CAL_INTERVAL */
	0x2d0,	/* CFG_CLKTRIM_0 */
	0x2d4,	/* CFG_CLKTRIM_1 */
	0x2d8,	/* CFG_CLKTRIM_2 */
};

struct emc_ctlr *emc_get_controller(const void *blob)
{
	fdt_addr_t addr;
	int node;

	node = fdtdec_next_compatible(blob, 0, COMPAT_NVIDIA_TEGRA20_EMC);
	if (node > 0) {
		addr = fdtdec_get_addr(blob, node, "reg");
		if (addr != FDT_ADDR_T_NONE)
			return (struct emc_ctlr *)addr;
	}
	return NULL;
}

/* Error codes we use */
enum {
	ERR_NO_EMC_NODE = -10,
	ERR_NO_EMC_REG,
	ERR_NO_FREQ,
	ERR_FREQ_NOT_FOUND,
	ERR_BAD_REGS,
	ERR_NO_RAM_CODE,
	ERR_RAM_CODE_NOT_FOUND,
};

/**
 * Find EMC tables for the given ram code.
 *
 * The tegra EMC binding has two options, one using the ram code and one not.
 * We detect which is in use by looking for the nvidia,use-ram-code property.
 * If this is not present, then the EMC tables are directly below 'node',
 * otherwise we select the correct emc-tables subnode based on the 'ram_code'
 * value.
 *
 * @param blob		Device tree blob
 * @param node		EMC node (nvidia,tegra20-emc compatible string)
 * @param ram_code	RAM code to select (0-3, or -1 if unknown)
 * Return: 0 if ok, otherwise a -ve ERR_ code (see enum above)
 */
static int find_emc_tables(const void *blob, int node, int ram_code)
{
	int need_ram_code;
	int depth;
	int offset;

	/* If we are using RAM codes, scan through the tables for our code */
	need_ram_code = fdtdec_get_bool(blob, node, "nvidia,use-ram-code");
	if (!need_ram_code)
		return node;
	if (ram_code == -1) {
		debug("%s: RAM code required but not supplied\n", __func__);
		return ERR_NO_RAM_CODE;
	}

	offset = node;
	depth = 0;
	do {
		/*
		 * Sadly there is no compatible string so we cannot use
		 * fdtdec_next_compatible_subnode().
		 */
		offset = fdt_next_node(blob, offset, &depth);
		if (depth <= 0)
			break;

		/* Make sure this is a direct subnode */
		if (depth != 1)
			continue;
		if (strcmp("emc-tables", fdt_get_name(blob, offset, NULL)))
			continue;

		if (fdtdec_get_int(blob, offset, "nvidia,ram-code", -1)
				== ram_code)
			return offset;
	} while (1);

	debug("%s: Could not find tables for RAM code %d\n", __func__,
	      ram_code);
	return ERR_RAM_CODE_NOT_FOUND;
}

/**
 * Decode the EMC node of the device tree, returning a pointer to the emc
 * controller and the table to be used for the given rate.
 *
 * @param blob	Device tree blob
 * @param rate	Clock speed of memory controller in Hz (=2x memory bus rate)
 * @param emcp	Returns address of EMC controller registers
 * @param tablep Returns pointer to table to program into EMC. There are
 *		TEGRA_EMC_NUM_REGS entries, destined for offsets as per the
 *		emc_reg_addr array.
 * Return: 0 if ok, otherwise a -ve error code which will allow someone to
 * figure out roughly what went wrong by looking at this code.
 */
static int decode_emc(const void *blob, unsigned rate, struct emc_ctlr **emcp,
		      const u32 **tablep)
{
	struct apb_misc_pp_ctlr *pp =
		(struct apb_misc_pp_ctlr *)NV_PA_APB_MISC_BASE;
	int ram_code;
	int depth;
	int node;

	ram_code = (readl(&pp->strapping_opt_a) & RAM_CODE_MASK)
			>> RAM_CODE_SHIFT;
	/*
	 * The EMC clock rate is twice the bus rate, and the bus rate is
	 * measured in kHz
	 */
	rate = rate / 2 / 1000;

	node = fdtdec_next_compatible(blob, 0, COMPAT_NVIDIA_TEGRA20_EMC);
	if (node < 0) {
		debug("%s: No EMC node found in FDT\n", __func__);
		return ERR_NO_EMC_NODE;
	}
	*emcp = (struct emc_ctlr *)fdtdec_get_addr(blob, node, "reg");
	if (*emcp == (struct emc_ctlr *)FDT_ADDR_T_NONE) {
		debug("%s: No EMC node reg property\n", __func__);
		return ERR_NO_EMC_REG;
	}

	/* Work out the parent node which contains our EMC tables */
	node = find_emc_tables(blob, node, ram_code & 3);
	if (node < 0)
		return node;

	depth = 0;
	for (;;) {
		int node_rate;

		node = fdtdec_next_compatible_subnode(blob, node,
				COMPAT_NVIDIA_TEGRA20_EMC_TABLE, &depth);
		if (node < 0)
			break;
		node_rate = fdtdec_get_int(blob, node, "clock-frequency", -1);
		if (node_rate == -1) {
			debug("%s: Missing clock-frequency\n", __func__);
			return ERR_NO_FREQ; /* we expect this property */
		}

		if (node_rate == rate)
			break;
	}
	if (node < 0) {
		debug("%s: No node found for clock frequency %d\n", __func__,
		      rate);
		return ERR_FREQ_NOT_FOUND;
	}

	*tablep = fdtdec_locate_array(blob, node, "nvidia,emc-registers",
				      TEGRA_EMC_NUM_REGS);
	if (!*tablep) {
		debug("%s: node '%s' array missing / wrong size\n", __func__,
		      fdt_get_name(blob, node, NULL));
		return ERR_BAD_REGS;
	}

	/* All seems well */
	return 0;
}

int tegra_set_emc(const void *blob, unsigned rate)
{
	struct emc_ctlr *emc;
	const u32 *table = NULL;
	int err, i;

	err = decode_emc(blob, rate, &emc, &table);
	if (err) {
		debug("Warning: no valid EMC (%d), memory timings unset\n",
		       err);
		return err;
	}

	debug("%s: Table found, setting EMC values as follows:\n", __func__);
	for (i = 0; i < TEGRA_EMC_NUM_REGS; i++) {
		u32 value = fdt32_to_cpu(table[i]);
		u32 addr = (uintptr_t)emc + emc_reg_addr[i];

		debug("   %#x: %#x\n", addr, value);
		writel(value, addr);
	}

	/* trigger emc with new settings */
	clock_adjust_periph_pll_div(PERIPH_ID_EMC, CLOCK_ID_MEMORY,
				clock_get_rate(CLOCK_ID_MEMORY), NULL);
	debug("EMC clock set to %lu\n",
	      clock_get_periph_rate(PERIPH_ID_EMC, CLOCK_ID_MEMORY));

	return 0;
}
