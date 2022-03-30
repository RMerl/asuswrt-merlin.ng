// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <dt-bindings/memory/mpc83xx-sdram.h>

DECLARE_GLOBAL_DATA_PTR;

/* Masks for the CS config register */
static const u32 CSCONFIG_ENABLE = 0x80000000;

static const u32 BANK_BITS_2;
static const u32 BANK_BITS_3 = 0x00004000;

static const u32 ROW_BITS_12;
static const u32 ROW_BITS_13 = 0x00000100;
static const u32 ROW_BITS_14 = 0x00000200;

static const u32 COL_BITS_8;
static const u32 COL_BITS_9  = 0x00000001;
static const u32 COL_BITS_10 = 0x00000002;
static const u32 COL_BITS_11 = 0x00000003;

/* Shifts for the DDR SDRAM Timing Configuration 3 register */
static const uint TIMING_CFG3_EXT_REFREC_SHIFT = (31 - 15);

/* Shifts for the DDR SDRAM Timing Configuration 0 register */
static const uint TIMING_CFG0_RWT_SHIFT         = (31 - 1);
static const uint TIMING_CFG0_WRT_SHIFT         = (31 - 3);
static const uint TIMING_CFG0_RRT_SHIFT         = (31 - 5);
static const uint TIMING_CFG0_WWT_SHIFT         = (31 - 7);
static const uint TIMING_CFG0_ACT_PD_EXIT_SHIFT = (31 - 11);
static const uint TIMING_CFG0_PRE_PD_EXIT_SHIFT = (31 - 15);
static const uint TIMING_CFG0_ODT_PD_EXIT_SHIFT = (31 - 23);
static const uint TIMING_CFG0_MRS_CYC_SHIFT     = (31 - 31);

/* Shifts for the DDR SDRAM Timing Configuration 1 register */
static const uint TIMING_CFG1_PRETOACT_SHIFT = (31 - 3);
static const uint TIMING_CFG1_ACTTOPRE_SHIFT = (31 - 7);
static const uint TIMING_CFG1_ACTTORW_SHIFT  = (31 - 11);
static const uint TIMING_CFG1_CASLAT_SHIFT   = (31 - 15);
static const uint TIMING_CFG1_REFREC_SHIFT   = (31 - 19);
static const uint TIMING_CFG1_WRREC_SHIFT    = (31 - 23);
static const uint TIMING_CFG1_ACTTOACT_SHIFT = (31 - 27);
static const uint TIMING_CFG1_WRTORD_SHIFT   = (31 - 31);

/* Shifts for the DDR SDRAM Timing Configuration 2 register */
static const uint TIMING_CFG2_CPO_SHIFT	          = (31 - 8);
static const uint TIMING_CFG2_WR_DATA_DELAY_SHIFT = (31 - 21);
static const uint TIMING_CFG2_ADD_LAT_SHIFT       = (31 - 3);
static const uint TIMING_CFG2_WR_LAT_DELAY_SHIFT  = (31 - 12);
static const uint TIMING_CFG2_RD_TO_PRE_SHIFT     = (31 - 18);
static const uint TIMING_CFG2_CKE_PLS_SHIFT       = (31 - 25);
static const uint TIMING_CFG2_FOUR_ACT_SHIFT;

/* Shifts for the DDR SDRAM Control Configuration register */
static const uint SDRAM_CFG_SREN_SHIFT         = (31 - 1);
static const uint SDRAM_CFG_ECC_EN_SHIFT       = (31 - 2);
static const uint SDRAM_CFG_RD_EN_SHIFT        = (31 - 3);
static const uint SDRAM_CFG_SDRAM_TYPE_SHIFT   = (31 - 7);
static const uint SDRAM_CFG_DYN_PWR_SHIFT      = (31 - 10);
static const uint SDRAM_CFG_DBW_SHIFT          = (31 - 12);
static const uint SDRAM_CFG_NCAP_SHIFT         = (31 - 14);
static const uint SDRAM_CFG_2T_EN_SHIFT        = (31 - 16);
static const uint SDRAM_CFG_BA_INTLV_CTL_SHIFT = (31 - 23);
static const uint SDRAM_CFG_PCHB8_SHIFT        = (31 - 27);
static const uint SDRAM_CFG_HSE_SHIFT          = (31 - 28);
static const uint SDRAM_CFG_BI_SHIFT           = (31 - 31);

/* Shifts for the DDR SDRAM Control Configuration 2 register */
static const uint SDRAM_CFG2_FRC_SR_SHIFT = (31 - 0);
static const uint SDRAM_CFG2_DLL_RST_DIS  = (31 - 2);
static const uint SDRAM_CFG2_DQS_CFG      = (31 - 5);
static const uint SDRAM_CFG2_ODT_CFG      = (31 - 10);
static const uint SDRAM_CFG2_NUM_PR       = (31 - 19);

/* Shifts for the DDR SDRAM Mode register */
static const uint SDRAM_MODE_ESD_SHIFT = (31 - 15);
static const uint SDRAM_MODE_SD_SHIFT  = (31 - 31);

/* Shifts for the DDR SDRAM Mode 2 register */
static const uint SDRAM_MODE2_ESD2_SHIFT = (31 - 15);
static const uint SDRAM_MODE2_ESD3_SHIFT = (31 - 31);

/* Shifts for the DDR SDRAM Interval Configuration register */
static const uint SDRAM_INTERVAL_REFINT_SHIFT  = (31 - 15);
static const uint SDRAM_INTERVAL_BSTOPRE_SHIFT = (31 - 31);

/* Mask for the DDR SDRAM Mode Control register */
static const u32 SDRAM_CFG_MEM_EN = 0x80000000;

int dram_init(void)
{
	struct udevice *ram_ctrl;
	int ret;

	/* Current assumption: There is only one RAM controller */
	ret = uclass_first_device_err(UCLASS_RAM, &ram_ctrl);
	if (ret) {
		debug("%s: uclass_first_device_err failed: %d\n",
		      __func__, ret);
		return ret;
	}

	/* FIXME(mario.six@gdsys.cc): Set gd->ram_size? */

	return 0;
}

phys_size_t get_effective_memsize(void)
{
	if (!IS_ENABLED(CONFIG_VERY_BIG_RAM))
		return gd->ram_size;

	/* Limit stack to what we can reasonable map */
	return ((gd->ram_size > CONFIG_MAX_MEM_MAPPED) ?
		CONFIG_MAX_MEM_MAPPED : gd->ram_size);
}

/**
 * struct mpc83xx_sdram_priv - Private data for MPC83xx RAM controllers
 * @total_size: The total size of all RAM modules associated with this RAM
 *		controller in bytes
 */
struct mpc83xx_sdram_priv {
	ulong total_size;
};

/**
 * mpc83xx_sdram_static_init() - Statically initialize a RAM module.
 * @node:    Device tree node associated with ths module in question
 * @cs:      The chip select to use for this RAM module
 * @mapaddr: The address where the RAM module should be mapped
 * @size:    The size of the RAM module to be mapped in bytes
 *
 * Return: 0 if OK, -ve on error
 */
static int mpc83xx_sdram_static_init(ofnode node, u32 cs, u32 mapaddr, u32 size)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = size;
	u32 msize_log2 = __ilog2(msize);
	u32 auto_precharge, odt_rd_cfg, odt_wr_cfg, bank_bits, row_bits,
	    col_bits;
	u32 bank_bits_mask, row_bits_mask, col_bits_mask;

	/* Configure the DDR local access window */
	out_be32(&im->sysconf.ddrlaw[cs].bar, mapaddr & 0xfffff000);
	out_be32(&im->sysconf.ddrlaw[cs].ar, LBLAWAR_EN | (msize_log2 - 1));

	out_be32(&im->ddr.csbnds[cs].csbnds, (msize - 1) >> 24);

	auto_precharge = ofnode_read_u32_default(node, "auto_precharge", 0);
	switch (auto_precharge) {
	case AUTO_PRECHARGE_ENABLE:
	case AUTO_PRECHARGE_DISABLE:
		break;
	default:
		debug("%s: auto_precharge value %d invalid.\n",
		      ofnode_get_name(node), auto_precharge);
		return -EINVAL;
	}

	odt_rd_cfg = ofnode_read_u32_default(node, "odt_rd_cfg", 0);
	switch (odt_rd_cfg) {
	case ODT_RD_ONLY_OTHER_DIMM:
		if (!IS_ENABLED(CONFIG_ARCH_MPC8360) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC837X)) {
			debug("%s: odt_rd_cfg value %d invalid.\n",
			      ofnode_get_name(node), odt_rd_cfg);
			return -EINVAL;
		}
		/* fall through */
	case ODT_RD_NEVER:
	case ODT_RD_ONLY_CURRENT:
	case ODT_RD_ONLY_OTHER_CS:
		if (!IS_ENABLED(CONFIG_ARCH_MPC830X) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC831X) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC8360) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC837X)) {
			debug("%s: odt_rd_cfg value %d invalid.\n",
			      ofnode_get_name(node), odt_rd_cfg);
			return -EINVAL;
		}
		/* fall through */
	/* Only MPC832x knows this value */
	case ODT_RD_ALL:
		break;
	default:
		debug("%s: odt_rd_cfg value %d invalid.\n",
		      ofnode_get_name(node), odt_rd_cfg);
		return -EINVAL;
	}

	odt_wr_cfg = ofnode_read_u32_default(node, "odt_wr_cfg", 0);
	switch (odt_wr_cfg) {
	case ODT_WR_ONLY_OTHER_DIMM:
		if (!IS_ENABLED(CONFIG_ARCH_MPC8360) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC837X)) {
			debug("%s: odt_wr_cfg value %d invalid.\n",
			      ofnode_get_name(node), odt_wr_cfg);
			return -EINVAL;
		}
		/* fall through */
	case ODT_WR_NEVER:
	case ODT_WR_ONLY_CURRENT:
	case ODT_WR_ONLY_OTHER_CS:
		if (!IS_ENABLED(CONFIG_ARCH_MPC830X) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC831X) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC8360) &&
		    !IS_ENABLED(CONFIG_ARCH_MPC837X)) {
			debug("%s: odt_wr_cfg value %d invalid.\n",
			      ofnode_get_name(node), odt_wr_cfg);
			return -EINVAL;
		}
		/* fall through */
	/* MPC832x only knows this value */
	case ODT_WR_ALL:
		break;
	default:
		debug("%s: odt_wr_cfg value %d invalid.\n",
		      ofnode_get_name(node), odt_wr_cfg);
		return -EINVAL;
	}

	bank_bits = ofnode_read_u32_default(node, "bank_bits", 0);
	switch (bank_bits) {
	case 2:
		bank_bits_mask = BANK_BITS_2;
		break;
	case 3:
		bank_bits_mask = BANK_BITS_3;
		break;
	default:
		debug("%s: bank_bits value %d invalid.\n",
		      ofnode_get_name(node), bank_bits);
		return -EINVAL;
	}

	row_bits = ofnode_read_u32_default(node, "row_bits", 0);
	switch (row_bits) {
	case 12:
		row_bits_mask = ROW_BITS_12;
		break;
	case 13:
		row_bits_mask = ROW_BITS_13;
		break;
	case 14:
		row_bits_mask = ROW_BITS_14;
		break;
	default:
		debug("%s: row_bits value %d invalid.\n",
		      ofnode_get_name(node), row_bits);
		return -EINVAL;
	}

	col_bits = ofnode_read_u32_default(node, "col_bits", 0);
	switch (col_bits) {
	case 8:
		col_bits_mask = COL_BITS_8;
		break;
	case 9:
		col_bits_mask = COL_BITS_9;
		break;
	case 10:
		col_bits_mask = COL_BITS_10;
		break;
	case 11:
		col_bits_mask = COL_BITS_11;
		break;
	default:
		debug("%s: col_bits value %d invalid.\n",
		      ofnode_get_name(node), col_bits);
		return -EINVAL;
	}

	/* Write CS config value */
	out_be32(&im->ddr.cs_config[cs], CSCONFIG_ENABLE | auto_precharge |
					 odt_rd_cfg | odt_wr_cfg |
					 bank_bits_mask | row_bits_mask |
					 col_bits_mask);
	return 0;
}

/**
 * mpc83xx_sdram_spd_init() - Initialize a RAM module using a SPD flash.
 * @node:    Device tree node associated with ths module in question
 * @cs:      The chip select to use for this RAM module
 * @mapaddr: The address where the RAM module should be mapped
 * @size:    The size of the RAM module to be mapped in bytes
 *
 * Return: 0 if OK, -ve on error
 */
static int mpc83xx_sdram_spd_init(ofnode node, u32 cs, u32 mapaddr, u32 size)
{
	/* TODO(mario.six@gdsys.cc): Implement */
	return 0;
}

static int mpc83xx_sdram_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}

static int mpc83xx_sdram_probe(struct udevice *dev)
{
	struct mpc83xx_sdram_priv *priv = dev_get_priv(dev);
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	int ret = 0;
	ofnode subnode;
	/* DDR control driver register values */
	u32 dso, pz_override, nz_override, odt_term, ddr_type, mvref_sel, m_odr;
	u32 ddrcdr;
	/* DDR SDRAM Clock Control register values */
	u32 clock_adjust;
	/* DDR SDRAM Timing Configuration 3 register values */
	u32 ext_refresh_rec, ext_refresh_rec_mask;
	/* DDR SDRAM Timing Configuration 0 register values */
	u32 read_to_write, write_to_read, read_to_read, write_to_write,
	    active_powerdown_exit, precharge_powerdown_exit,
	    odt_powerdown_exit, mode_reg_set_cycle;
	u32 timing_cfg_0;
	/* DDR SDRAM Timing Configuration 1 register values */
	u32 precharge_to_activate, activate_to_precharge,
	    activate_to_readwrite, mcas_latency, refresh_recovery,
	    last_data_to_precharge, activate_to_activate,
	    last_write_data_to_read;
	u32 timing_cfg_1;
	/* DDR SDRAM Timing Configuration 2 register values */
	u32 additive_latency, mcas_to_preamble_override, write_latency,
	    read_to_precharge, write_cmd_to_write_data,
	    minimum_cke_pulse_width, four_activates_window;
	u32 timing_cfg_2;
	/* DDR SDRAM Control Configuration register values */
	u32 self_refresh, ecc, registered_dram, sdram_type,
	    dynamic_power_management, databus_width, nc_auto_precharge,
	    timing_2t, bank_interleaving_ctrl, precharge_bit_8, half_strength,
	    bypass_initialization;
	u32 sdram_cfg;
	/* DDR SDRAM Control Configuration 2 register values */
	u32 force_self_refresh, dll_reset, dqs_config, odt_config,
	    posted_refreshes;
	u32 sdram_cfg2;
	/* DDR SDRAM Mode Configuration register values */
	u32 sdmode, esdmode;
	u32 sdram_mode;
	/* DDR SDRAM Mode Configuration 2 register values */
	u32 esdmode2, esdmode3;
	u32 sdram_mode2;
	/* DDR SDRAM Interval Configuration register values */
	u32 refresh_interval, precharge_interval;
	u32 sdram_interval;

	priv->total_size = 0;

	/* Disable both banks initially (might be re-enabled in loop below) */
	out_be32(&im->ddr.cs_config[0], 0);
	out_be32(&im->ddr.cs_config[1], 0);

	dso = dev_read_u32_default(dev, "driver_software_override", 0);
	if (dso > 1) {
		debug("%s: driver_software_override value %d invalid.\n",
		      dev->name, dso);
		return -EINVAL;
	}

	pz_override = dev_read_u32_default(dev, "p_impedance_override", 0);

	switch (pz_override) {
	case DSO_P_IMPEDANCE_HIGHEST_Z:
	case DSO_P_IMPEDANCE_MUCH_HIGHER_Z:
	case DSO_P_IMPEDANCE_HIGHER_Z:
	case DSO_P_IMPEDANCE_NOMINAL:
	case DSO_P_IMPEDANCE_LOWER_Z:
		break;
	default:
		debug("%s: p_impedance_override value %d invalid.\n",
		      dev->name, pz_override);
		return -EINVAL;
	}

	nz_override = dev_read_u32_default(dev, "n_impedance_override", 0);

	switch (nz_override) {
	case DSO_N_IMPEDANCE_HIGHEST_Z:
	case DSO_N_IMPEDANCE_MUCH_HIGHER_Z:
	case DSO_N_IMPEDANCE_HIGHER_Z:
	case DSO_N_IMPEDANCE_NOMINAL:
	case DSO_N_IMPEDANCE_LOWER_Z:
		break;
	default:
		debug("%s: n_impedance_override value %d invalid.\n",
		      dev->name, nz_override);
		return -EINVAL;
	}

	odt_term = dev_read_u32_default(dev, "odt_termination_value", 0);
	if (odt_term > 1) {
		debug("%s: odt_termination_value value %d invalid.\n",
		      dev->name, odt_term);
		return -EINVAL;
	}

	ddr_type = dev_read_u32_default(dev, "ddr_type", 0);
	if (ddr_type > 1) {
		debug("%s: ddr_type value %d invalid.\n",
		      dev->name, ddr_type);
		return -EINVAL;
	}

	mvref_sel = dev_read_u32_default(dev, "mvref_sel", 0);
	if (mvref_sel > 1) {
		debug("%s: mvref_sel value %d invalid.\n",
		      dev->name, mvref_sel);
		return -EINVAL;
	}

	m_odr = dev_read_u32_default(dev, "m_odr", 0);
	if (mvref_sel > 1) {
		debug("%s: m_odr value %d invalid.\n",
		      dev->name, m_odr);
		return -EINVAL;
	}

	ddrcdr = dso << (31 - 1) |
		 pz_override << (31 - 5) |
		 nz_override << (31 - 9) |
		 odt_term << (31 - 12) |
		 ddr_type << (31 - 13) |
		 mvref_sel << (31 - 29) |
		 m_odr << (31 - 30) | 1;

	/* Configure the DDR control driver register */
	out_be32(&im->sysconf.ddrcdr, ddrcdr);

	dev_for_each_subnode(subnode, dev) {
		u32 val[3];
		u32 cs, addr, size;

		/* CS, map address, size -> three values */
		ofnode_read_u32_array(subnode, "reg", val, 3);

		cs = val[0];
		addr = val[1];
		size = val[2];

		if (cs > 1) {
			debug("%s: chip select value %d invalid.\n",
			      dev->name, cs);
			return -EINVAL;
		}

		/* TODO(mario.six@gdsys.cc): Sanity check for size. */

		if (ofnode_read_bool(subnode, "read-spd"))
			ret = mpc83xx_sdram_spd_init(subnode, cs, addr, size);
		else
			ret = mpc83xx_sdram_static_init(subnode, cs, addr,
							size);
		if (ret) {
			debug("%s: RAM init failed.\n", dev->name);
			return ret;
		}
	};

	/*
	 * TODO(mario.six@gdsys.cc): This should only occur for static
	 *			     configuration
	 */

	clock_adjust = dev_read_u32_default(dev, "clock_adjust", 0);
	switch (clock_adjust) {
	case CLOCK_ADJUST_025:
	case CLOCK_ADJUST_05:
	case CLOCK_ADJUST_075:
	case CLOCK_ADJUST_1:
		break;
	default:
		debug("%s: clock_adjust value %d invalid.\n",
		      dev->name, clock_adjust);
		return -EINVAL;
	}

	/* Configure the DDR SDRAM Clock Control register */
	out_be32(&im->ddr.sdram_clk_cntl, clock_adjust);

	ext_refresh_rec = dev_read_u32_default(dev, "ext_refresh_rec", 0);
	switch (ext_refresh_rec) {
	case 0:
		ext_refresh_rec_mask = 0 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	case 16:
		ext_refresh_rec_mask = 1 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	case 32:
		ext_refresh_rec_mask = 2 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	case 48:
		ext_refresh_rec_mask = 3 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	case 64:
		ext_refresh_rec_mask = 4 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	case 80:
		ext_refresh_rec_mask = 5 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	case 96:
		ext_refresh_rec_mask = 6 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	case 112:
		ext_refresh_rec_mask = 7 << TIMING_CFG3_EXT_REFREC_SHIFT;
		break;
	default:
		debug("%s: ext_refresh_rec value %d invalid.\n",
		      dev->name, ext_refresh_rec);
		return -EINVAL;
	}

	/* Configure the DDR SDRAM Timing Configuration 3 register */
	out_be32(&im->ddr.timing_cfg_3, ext_refresh_rec_mask);

	read_to_write = dev_read_u32_default(dev, "read_to_write", 0);
	if (read_to_write > 3) {
		debug("%s: read_to_write value %d invalid.\n",
		      dev->name, read_to_write);
		return -EINVAL;
	}

	write_to_read = dev_read_u32_default(dev, "write_to_read", 0);
	if (write_to_read > 3) {
		debug("%s: write_to_read value %d invalid.\n",
		      dev->name, write_to_read);
		return -EINVAL;
	}

	read_to_read = dev_read_u32_default(dev, "read_to_read", 0);
	if (read_to_read > 3) {
		debug("%s: read_to_read value %d invalid.\n",
		      dev->name, read_to_read);
		return -EINVAL;
	}

	write_to_write = dev_read_u32_default(dev, "write_to_write", 0);
	if (write_to_write > 3) {
		debug("%s: write_to_write value %d invalid.\n",
		      dev->name, write_to_write);
		return -EINVAL;
	}

	active_powerdown_exit =
		dev_read_u32_default(dev, "active_powerdown_exit", 0);
	if (active_powerdown_exit > 7) {
		debug("%s: active_powerdown_exit value %d invalid.\n",
		      dev->name, active_powerdown_exit);
		return -EINVAL;
	}

	precharge_powerdown_exit =
		dev_read_u32_default(dev, "precharge_powerdown_exit", 0);
	if (precharge_powerdown_exit > 7) {
		debug("%s: precharge_powerdown_exit value %d invalid.\n",
		      dev->name, precharge_powerdown_exit);
		return -EINVAL;
	}

	odt_powerdown_exit = dev_read_u32_default(dev, "odt_powerdown_exit", 0);
	if (odt_powerdown_exit > 15) {
		debug("%s: odt_powerdown_exit value %d invalid.\n",
		      dev->name, odt_powerdown_exit);
		return -EINVAL;
	}

	mode_reg_set_cycle = dev_read_u32_default(dev, "mode_reg_set_cycle", 0);
	if (mode_reg_set_cycle > 15) {
		debug("%s: mode_reg_set_cycle value %d invalid.\n",
		      dev->name, mode_reg_set_cycle);
		return -EINVAL;
	}

	timing_cfg_0 = read_to_write << TIMING_CFG0_RWT_SHIFT |
		       write_to_read << TIMING_CFG0_WRT_SHIFT |
		       read_to_read << TIMING_CFG0_RRT_SHIFT |
		       write_to_write << TIMING_CFG0_WWT_SHIFT |
		       active_powerdown_exit << TIMING_CFG0_ACT_PD_EXIT_SHIFT |
		       precharge_powerdown_exit << TIMING_CFG0_PRE_PD_EXIT_SHIFT |
		       odt_powerdown_exit << TIMING_CFG0_ODT_PD_EXIT_SHIFT |
		       mode_reg_set_cycle << TIMING_CFG0_MRS_CYC_SHIFT;

	out_be32(&im->ddr.timing_cfg_0, timing_cfg_0);

	precharge_to_activate =
		dev_read_u32_default(dev, "precharge_to_activate", 0);
	if (precharge_to_activate > 7 || precharge_to_activate == 0) {
		debug("%s: precharge_to_activate value %d invalid.\n",
		      dev->name, precharge_to_activate);
		return -EINVAL;
	}

	activate_to_precharge =
		dev_read_u32_default(dev, "activate_to_precharge", 0);
	if (activate_to_precharge > 19) {
		debug("%s: activate_to_precharge value %d invalid.\n",
		      dev->name, activate_to_precharge);
		return -EINVAL;
	}

	activate_to_readwrite =
		dev_read_u32_default(dev, "activate_to_readwrite", 0);
	if (activate_to_readwrite > 7 || activate_to_readwrite == 0) {
		debug("%s: activate_to_readwrite value %d invalid.\n",
		      dev->name, activate_to_readwrite);
		return -EINVAL;
	}

	mcas_latency = dev_read_u32_default(dev, "mcas_latency", 0);
	switch (mcas_latency) {
	case CASLAT_20:
	case CASLAT_25:
		if (!IS_ENABLED(CONFIG_ARCH_MPC8308)) {
			debug("%s: MCAS latency < 3.0 unsupported on MPC8308\n",
			      dev->name);
			return -EINVAL;
		}
		/* fall through */
	case CASLAT_30:
	case CASLAT_35:
	case CASLAT_40:
	case CASLAT_45:
	case CASLAT_50:
	case CASLAT_55:
	case CASLAT_60:
	case CASLAT_65:
	case CASLAT_70:
	case CASLAT_75:
	case CASLAT_80:
		break;
	default:
		debug("%s: mcas_latency value %d invalid.\n",
		      dev->name, mcas_latency);
		return -EINVAL;
	}

	refresh_recovery = dev_read_u32_default(dev, "refresh_recovery", 0);
	if (refresh_recovery > 23 || refresh_recovery < 8) {
		debug("%s: refresh_recovery value %d invalid.\n",
		      dev->name, refresh_recovery);
		return -EINVAL;
	}

	last_data_to_precharge =
		dev_read_u32_default(dev, "last_data_to_precharge", 0);
	if (last_data_to_precharge > 7 || last_data_to_precharge == 0) {
		debug("%s: last_data_to_precharge value %d invalid.\n",
		      dev->name, last_data_to_precharge);
		return -EINVAL;
	}

	activate_to_activate =
		dev_read_u32_default(dev, "activate_to_activate", 0);
	if (activate_to_activate > 7 || activate_to_activate == 0) {
		debug("%s: activate_to_activate value %d invalid.\n",
		      dev->name, activate_to_activate);
		return -EINVAL;
	}

	last_write_data_to_read =
		dev_read_u32_default(dev, "last_write_data_to_read", 0);
	if (last_write_data_to_read > 7 || last_write_data_to_read == 0) {
		debug("%s: last_write_data_to_read value %d invalid.\n",
		      dev->name, last_write_data_to_read);
		return -EINVAL;
	}

	timing_cfg_1 = precharge_to_activate << TIMING_CFG1_PRETOACT_SHIFT |
		       (activate_to_precharge > 15 ?
			activate_to_precharge - 16 :
			activate_to_precharge) << TIMING_CFG1_ACTTOPRE_SHIFT |
		       activate_to_readwrite << TIMING_CFG1_ACTTORW_SHIFT |
		       mcas_latency << TIMING_CFG1_CASLAT_SHIFT |
		       (refresh_recovery - 8) << TIMING_CFG1_REFREC_SHIFT |
		       last_data_to_precharge << TIMING_CFG1_WRREC_SHIFT |
		       activate_to_activate << TIMING_CFG1_ACTTOACT_SHIFT |
		       last_write_data_to_read << TIMING_CFG1_WRTORD_SHIFT;

	/* Configure the DDR SDRAM Timing Configuration 1 register */
	out_be32(&im->ddr.timing_cfg_1, timing_cfg_1);

	additive_latency = dev_read_u32_default(dev, "additive_latency", 0);
	if (additive_latency > 5) {
		debug("%s: additive_latency value %d invalid.\n",
		      dev->name, additive_latency);
		return -EINVAL;
	}

	mcas_to_preamble_override =
		dev_read_u32_default(dev, "mcas_to_preamble_override", 0);
	switch (mcas_to_preamble_override) {
	case READ_LAT_PLUS_1:
	case READ_LAT:
	case READ_LAT_PLUS_1_4:
	case READ_LAT_PLUS_1_2:
	case READ_LAT_PLUS_3_4:
	case READ_LAT_PLUS_5_4:
	case READ_LAT_PLUS_3_2:
	case READ_LAT_PLUS_7_4:
	case READ_LAT_PLUS_2:
	case READ_LAT_PLUS_9_4:
	case READ_LAT_PLUS_5_2:
	case READ_LAT_PLUS_11_4:
	case READ_LAT_PLUS_3:
	case READ_LAT_PLUS_13_4:
	case READ_LAT_PLUS_7_2:
	case READ_LAT_PLUS_15_4:
	case READ_LAT_PLUS_4:
	case READ_LAT_PLUS_17_4:
	case READ_LAT_PLUS_9_2:
	case READ_LAT_PLUS_19_4:
		break;
	default:
		debug("%s: mcas_to_preamble_override value %d invalid.\n",
		      dev->name, mcas_to_preamble_override);
		return -EINVAL;
	}

	write_latency = dev_read_u32_default(dev, "write_latency", 0);
	if (write_latency > 7 || write_latency == 0) {
		debug("%s: write_latency value %d invalid.\n",
		      dev->name, write_latency);
		return -EINVAL;
	}

	read_to_precharge = dev_read_u32_default(dev, "read_to_precharge", 0);
	if (read_to_precharge > 4 || read_to_precharge == 0) {
		debug("%s: read_to_precharge value %d invalid.\n",
		      dev->name, read_to_precharge);
		return -EINVAL;
	}

	write_cmd_to_write_data =
		dev_read_u32_default(dev, "write_cmd_to_write_data", 0);
	switch (write_cmd_to_write_data) {
	case CLOCK_DELAY_0:
	case CLOCK_DELAY_1_4:
	case CLOCK_DELAY_1_2:
	case CLOCK_DELAY_3_4:
	case CLOCK_DELAY_1:
	case CLOCK_DELAY_5_4:
	case CLOCK_DELAY_3_2:
		break;
	default:
		debug("%s: write_cmd_to_write_data value %d invalid.\n",
		      dev->name, write_cmd_to_write_data);
		return -EINVAL;
	}

	minimum_cke_pulse_width =
		dev_read_u32_default(dev, "minimum_cke_pulse_width", 0);
	if (minimum_cke_pulse_width > 4 || minimum_cke_pulse_width == 0) {
		debug("%s: minimum_cke_pulse_width value %d invalid.\n",
		      dev->name, minimum_cke_pulse_width);
		return -EINVAL;
	}

	four_activates_window =
		dev_read_u32_default(dev, "four_activates_window", 0);
	if (four_activates_window > 20 || four_activates_window == 0) {
		debug("%s: four_activates_window value %d invalid.\n",
		      dev->name, four_activates_window);
		return -EINVAL;
	}

	timing_cfg_2 = additive_latency << TIMING_CFG2_ADD_LAT_SHIFT |
		       mcas_to_preamble_override << TIMING_CFG2_CPO_SHIFT |
		       write_latency << TIMING_CFG2_WR_LAT_DELAY_SHIFT |
		       read_to_precharge << TIMING_CFG2_RD_TO_PRE_SHIFT |
		       write_cmd_to_write_data << TIMING_CFG2_WR_DATA_DELAY_SHIFT |
		       minimum_cke_pulse_width << TIMING_CFG2_CKE_PLS_SHIFT |
		       four_activates_window << TIMING_CFG2_FOUR_ACT_SHIFT;

	out_be32(&im->ddr.timing_cfg_2, timing_cfg_2);

	self_refresh = dev_read_u32_default(dev, "self_refresh", 0);
	switch (self_refresh) {
	case SREN_DISABLE:
	case SREN_ENABLE:
		break;
	default:
		debug("%s: self_refresh value %d invalid.\n",
		      dev->name, self_refresh);
		return -EINVAL;
	}

	ecc = dev_read_u32_default(dev, "ecc", 0);
	switch (ecc) {
	case ECC_DISABLE:
	case ECC_ENABLE:
		break;
	default:
		debug("%s: ecc value %d invalid.\n", dev->name, ecc);
		return -EINVAL;
	}

	registered_dram = dev_read_u32_default(dev, "registered_dram", 0);
	switch (registered_dram) {
	case RD_DISABLE:
	case RD_ENABLE:
		break;
	default:
		debug("%s: registered_dram value %d invalid.\n",
		      dev->name, registered_dram);
		return -EINVAL;
	}

	sdram_type = dev_read_u32_default(dev, "sdram_type", 0);
	switch (sdram_type) {
	case TYPE_DDR1:
	case TYPE_DDR2:
		break;
	default:
		debug("%s: sdram_type value %d invalid.\n",
		      dev->name, sdram_type);
		return -EINVAL;
	}

	dynamic_power_management =
		dev_read_u32_default(dev, "dynamic_power_management", 0);
	switch (dynamic_power_management) {
	case DYN_PWR_DISABLE:
	case DYN_PWR_ENABLE:
		break;
	default:
		debug("%s: dynamic_power_management value %d invalid.\n",
		      dev->name, dynamic_power_management);
		return -EINVAL;
	}

	databus_width = dev_read_u32_default(dev, "databus_width", 0);
	switch (databus_width) {
	case DATA_BUS_WIDTH_16:
	case DATA_BUS_WIDTH_32:
		break;
	default:
		debug("%s: databus_width value %d invalid.\n",
		      dev->name, databus_width);
		return -EINVAL;
	}

	nc_auto_precharge = dev_read_u32_default(dev, "nc_auto_precharge", 0);
	switch (nc_auto_precharge) {
	case NCAP_DISABLE:
	case NCAP_ENABLE:
		break;
	default:
		debug("%s: nc_auto_precharge value %d invalid.\n",
		      dev->name, nc_auto_precharge);
		return -EINVAL;
	}

	timing_2t = dev_read_u32_default(dev, "timing_2t", 0);
	switch (timing_2t) {
	case TIMING_1T:
	case TIMING_2T:
		break;
	default:
		debug("%s: timing_2t value %d invalid.\n",
		      dev->name, timing_2t);
		return -EINVAL;
	}

	bank_interleaving_ctrl =
		dev_read_u32_default(dev, "bank_interleaving_ctrl", 0);
	switch (bank_interleaving_ctrl) {
	case INTERLEAVE_NONE:
	case INTERLEAVE_1_AND_2:
		break;
	default:
		debug("%s: bank_interleaving_ctrl value %d invalid.\n",
		      dev->name, bank_interleaving_ctrl);
		return -EINVAL;
	}

	precharge_bit_8 = dev_read_u32_default(dev, "precharge_bit_8", 0);
	switch (precharge_bit_8) {
	case PRECHARGE_MA_10:
	case PRECHARGE_MA_8:
		break;
	default:
		debug("%s: precharge_bit_8 value %d invalid.\n",
		      dev->name, precharge_bit_8);
		return -EINVAL;
	}

	half_strength = dev_read_u32_default(dev, "half_strength", 0);
	switch (half_strength) {
	case STRENGTH_FULL:
	case STRENGTH_HALF:
		break;
	default:
		debug("%s: half_strength value %d invalid.\n",
		      dev->name, half_strength);
		return -EINVAL;
	}

	bypass_initialization =
		dev_read_u32_default(dev, "bypass_initialization", 0);
	switch (bypass_initialization) {
	case INITIALIZATION_DONT_BYPASS:
	case INITIALIZATION_BYPASS:
		break;
	default:
		debug("%s: bypass_initialization value %d invalid.\n",
		      dev->name, bypass_initialization);
		return -EINVAL;
	}

	sdram_cfg = self_refresh << SDRAM_CFG_SREN_SHIFT |
		    ecc << SDRAM_CFG_ECC_EN_SHIFT |
		    registered_dram << SDRAM_CFG_RD_EN_SHIFT |
		    sdram_type << SDRAM_CFG_SDRAM_TYPE_SHIFT |
		    dynamic_power_management << SDRAM_CFG_DYN_PWR_SHIFT |
		    databus_width << SDRAM_CFG_DBW_SHIFT |
		    nc_auto_precharge << SDRAM_CFG_NCAP_SHIFT |
		    timing_2t << SDRAM_CFG_2T_EN_SHIFT |
		    bank_interleaving_ctrl << SDRAM_CFG_BA_INTLV_CTL_SHIFT |
		    precharge_bit_8 << SDRAM_CFG_PCHB8_SHIFT |
		    half_strength << SDRAM_CFG_HSE_SHIFT |
		    bypass_initialization << SDRAM_CFG_BI_SHIFT;

	out_be32(&im->ddr.sdram_cfg, sdram_cfg);

	force_self_refresh = dev_read_u32_default(dev, "force_self_refresh", 0);
	switch (force_self_refresh) {
	case MODE_NORMAL:
	case MODE_REFRESH:
		break;
	default:
		debug("%s: force_self_refresh value %d invalid.\n",
		      dev->name, force_self_refresh);
		return -EINVAL;
	}

	dll_reset = dev_read_u32_default(dev, "dll_reset", 0);
	switch (dll_reset) {
	case DLL_RESET_ENABLE:
	case DLL_RESET_DISABLE:
		break;
	default:
		debug("%s: dll_reset value %d invalid.\n",
		      dev->name, dll_reset);
		return -EINVAL;
	}

	dqs_config = dev_read_u32_default(dev, "dqs_config", 0);
	switch (dqs_config) {
	case DQS_TRUE:
		break;
	default:
		debug("%s: dqs_config value %d invalid.\n",
		      dev->name, dqs_config);
		return -EINVAL;
	}

	odt_config = dev_read_u32_default(dev, "odt_config", 0);
	switch (odt_config) {
	case ODT_ASSERT_NEVER:
	case ODT_ASSERT_WRITES:
	case ODT_ASSERT_READS:
	case ODT_ASSERT_ALWAYS:
		break;
	default:
		debug("%s: odt_config value %d invalid.\n",
		      dev->name, odt_config);
		return -EINVAL;
	}

	posted_refreshes = dev_read_u32_default(dev, "posted_refreshes", 0);
	if (posted_refreshes > 8 || posted_refreshes == 0) {
		debug("%s: posted_refreshes value %d invalid.\n",
		      dev->name, posted_refreshes);
		return -EINVAL;
	}

	sdram_cfg2 = force_self_refresh << SDRAM_CFG2_FRC_SR_SHIFT |
		     dll_reset << SDRAM_CFG2_DLL_RST_DIS |
		     dqs_config << SDRAM_CFG2_DQS_CFG |
		     odt_config << SDRAM_CFG2_ODT_CFG |
		     posted_refreshes << SDRAM_CFG2_NUM_PR;

	out_be32(&im->ddr.sdram_cfg2, sdram_cfg2);

	sdmode = dev_read_u32_default(dev, "sdmode", 0);
	if (sdmode > 0xFFFF) {
		debug("%s: sdmode value %d invalid.\n",
		      dev->name, sdmode);
		return -EINVAL;
	}

	esdmode = dev_read_u32_default(dev, "esdmode", 0);
	if (esdmode > 0xFFFF) {
		debug("%s: esdmode value %d invalid.\n", dev->name, esdmode);
		return -EINVAL;
	}

	sdram_mode = sdmode << SDRAM_MODE_SD_SHIFT |
		     esdmode << SDRAM_MODE_ESD_SHIFT;

	out_be32(&im->ddr.sdram_mode, sdram_mode);

	esdmode2 = dev_read_u32_default(dev, "esdmode2", 0);
	if (esdmode2 > 0xFFFF) {
		debug("%s: esdmode2 value %d invalid.\n", dev->name, esdmode2);
		return -EINVAL;
	}

	esdmode3 = dev_read_u32_default(dev, "esdmode3", 0);
	if (esdmode3 > 0xFFFF) {
		debug("%s: esdmode3 value %d invalid.\n", dev->name, esdmode3);
		return -EINVAL;
	}

	sdram_mode2 = esdmode2 << SDRAM_MODE2_ESD2_SHIFT |
		      esdmode3 << SDRAM_MODE2_ESD3_SHIFT;

	out_be32(&im->ddr.sdram_mode2, sdram_mode2);

	refresh_interval = dev_read_u32_default(dev, "refresh_interval", 0);
	if (refresh_interval > 0xFFFF) {
		debug("%s: refresh_interval value %d invalid.\n",
		      dev->name, refresh_interval);
		return -EINVAL;
	}

	precharge_interval = dev_read_u32_default(dev, "precharge_interval", 0);
	if (precharge_interval > 0x3FFF) {
		debug("%s: precharge_interval value %d invalid.\n",
		      dev->name, precharge_interval);
		return -EINVAL;
	}

	sdram_interval = refresh_interval << SDRAM_INTERVAL_REFINT_SHIFT |
			 precharge_interval << SDRAM_INTERVAL_BSTOPRE_SHIFT;

	out_be32(&im->ddr.sdram_interval, sdram_interval);
	sync();

	/* Enable DDR controller */
	setbits_be32(&im->ddr.sdram_cfg, SDRAM_CFG_MEM_EN);
	sync();

	dev_for_each_subnode(subnode, dev) {
		u32 val[3];
		u32 addr, size;

		/* CS, map address, size -> three values */
		ofnode_read_u32_array(subnode, "reg", val, 3);

		addr = val[1];
		size = val[2];

		priv->total_size += get_ram_size((long int *)addr, size);
	};

	gd->ram_size = priv->total_size;

	return 0;
}

static int mpc83xx_sdram_get_info(struct udevice *dev, struct ram_info *info)
{
	/* TODO(mario.six@gdsys.cc): Implement */
	return 0;
}

static struct ram_ops mpc83xx_sdram_ops = {
	.get_info = mpc83xx_sdram_get_info,
};

static const struct udevice_id mpc83xx_sdram_ids[] = {
	{ .compatible = "fsl,mpc83xx-mem-controller" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mpc83xx_sdram) = {
	.name = "mpc83xx_sdram",
	.id = UCLASS_RAM,
	.of_match = mpc83xx_sdram_ids,
	.ops = &mpc83xx_sdram_ops,
	.ofdata_to_platdata = mpc83xx_sdram_ofdata_to_platdata,
	.probe = mpc83xx_sdram_probe,
	.priv_auto_alloc_size = sizeof(struct mpc83xx_sdram_priv),
};
