// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper utilities for qlm.
 */

#include <log.h>
#include <time.h>
#include <asm/global_data.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-mio-defs.h>
#include <mach/cvmx-pciercx-defs.h>
#include <mach/cvmx-pemx-defs.h>
#include <mach/cvmx-pexp-defs.h>
#include <mach/cvmx-rst-defs.h>
#include <mach/cvmx-sata-defs.h>
#include <mach/cvmx-sli-defs.h>
#include <mach/cvmx-sriomaintx-defs.h>
#include <mach/cvmx-sriox-defs.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-jtag.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Their is a copy of this in bootloader qlm configuration, make sure
 * to update both the places till i figure out
 */
#define R_25G_REFCLK100		 0x0
#define R_5G_REFCLK100		 0x1
#define R_8G_REFCLK100		 0x2
#define R_125G_REFCLK15625_KX	 0x3
#define R_3125G_REFCLK15625_XAUI 0x4
#define R_103125G_REFCLK15625_KR 0x5
#define R_125G_REFCLK15625_SGMII 0x6
#define R_5G_REFCLK15625_QSGMII	 0x7
#define R_625G_REFCLK15625_RXAUI 0x8
#define R_25G_REFCLK125		 0x9
#define R_5G_REFCLK125		 0xa
#define R_8G_REFCLK125		 0xb

static const int REF_100MHZ = 100000000;
static const int REF_125MHZ = 125000000;
static const int REF_156MHZ = 156250000;

static qlm_jtag_uint32_t *__cvmx_qlm_jtag_xor_ref;

/**
 * Return the number of QLMs supported by the chip
 *
 * Return:  Number of QLMs
 */
int cvmx_qlm_get_num(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 5;
	else if (OCTEON_IS_MODEL(OCTEON_CN66XX))
		return 3;
	else if (OCTEON_IS_MODEL(OCTEON_CN63XX))
		return 3;
	else if (OCTEON_IS_MODEL(OCTEON_CN61XX))
		return 3;
	else if (OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return 2;
	else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 8;
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 7;
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 9;
	return 0;
}

/**
 * Return the qlm number based on the interface
 *
 * @param xiface  interface to look up
 *
 * Return: the qlm number based on the xiface
 */
int cvmx_qlm_interface(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (OCTEON_IS_MODEL(OCTEON_CN61XX)) {
		return (xi.interface == 0) ? 2 : 0;
	} else if (OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		return 2 - xi.interface;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		if (xi.interface == 0)
			return 0;

		debug("Warning: %s: Invalid interface %d\n",
		      __func__, xi.interface);
	} else if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		debug("Warning: not supported\n");
		return -1;
	}

	/* Must be cn68XX */
	switch (xi.interface) {
	case 1:
		return 0;
	default:
		return xi.interface;
	}

	return -1;
}

/**
 * Return the qlm number based for a port in the interface
 *
 * @param xiface  interface to look up
 * @param index  index in an interface
 *
 * Return: the qlm number based on the xiface
 */
int cvmx_qlm_lmac(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_bgxx_cmr_global_config_t gconfig;
		cvmx_gserx_phy_ctl_t phy_ctl;
		cvmx_gserx_cfg_t gserx_cfg;
		int qlm;

		if (xi.interface < 6) {
			if (xi.interface < 2) {
				gconfig.u64 =
					csr_rd_node(xi.node,
						    CVMX_BGXX_CMR_GLOBAL_CONFIG(xi.interface));
				if (gconfig.s.pmux_sds_sel)
					qlm = xi.interface + 2; /* QLM 2 or 3 */
				else
					qlm = xi.interface; /* QLM 0 or 1 */
			} else {
				qlm = xi.interface + 2; /* QLM 4-7 */
			}

			/* make sure the QLM is powered up and out of reset */
			phy_ctl.u64 = csr_rd_node(xi.node, CVMX_GSERX_PHY_CTL(qlm));
			if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
				return -1;
			gserx_cfg.u64 = csr_rd_node(xi.node, CVMX_GSERX_CFG(qlm));
			if (gserx_cfg.s.bgx)
				return qlm;
			else
				return -1;
		} else if (xi.interface <= 7) { /* ILK */
			int qlm;

			for (qlm = 4; qlm < 8; qlm++) {
				/* Make sure the QLM is powered and out of reset */
				phy_ctl.u64 = csr_rd_node(xi.node, CVMX_GSERX_PHY_CTL(qlm));
				if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
					continue;
				/* Make sure the QLM is in ILK mode */
				gserx_cfg.u64 = csr_rd_node(xi.node, CVMX_GSERX_CFG(qlm));
				if (gserx_cfg.s.ila)
					return qlm;
			}
		}
		return -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		cvmx_gserx_phy_ctl_t phy_ctl;
		cvmx_gserx_cfg_t gserx_cfg;
		int qlm;

		/* (interface)0->QLM2, 1->QLM3, 2->DLM5/3->DLM6 */
		if (xi.interface < 2) {
			qlm = xi.interface + 2; /* (0,1)->ret(2,3) */

			phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
			if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
				return -1;

			gserx_cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));
			if (gserx_cfg.s.bgx)
				return qlm;
			else
				return -1;
		} else if (xi.interface == 2) {
			cvmx_gserx_cfg_t g1, g2;

			g1.u64 = csr_rd(CVMX_GSERX_CFG(5));
			g2.u64 = csr_rd(CVMX_GSERX_CFG(6));
			/* Check if both QLM5 & QLM6 are BGX2 */
			if (g2.s.bgx) {
				if (g1.s.bgx) {
					cvmx_gserx_phy_ctl_t phy_ctl1;

					phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(5));
					phy_ctl1.u64 = csr_rd(CVMX_GSERX_PHY_CTL(6));
					if ((phy_ctl.s.phy_pd || phy_ctl.s.phy_reset) &&
					    (phy_ctl1.s.phy_pd || phy_ctl1.s.phy_reset))
						return -1;
					if (index >= 2)
						return 6;
					return 5;
				} else { /* QLM6 is BGX2 */
					phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(6));
					if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
						return -1;
					return 6;
				}
			} else if (g1.s.bgx) {
				phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(5));
				if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
					return -1;
				return 5;
			}
		}
		return -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_gserx_phy_ctl_t phy_ctl;
		cvmx_gserx_cfg_t gserx_cfg;
		int qlm;

		if (xi.interface == 0) {
			cvmx_gserx_cfg_t g1, g2;

			g1.u64 = csr_rd(CVMX_GSERX_CFG(4));
			g2.u64 = csr_rd(CVMX_GSERX_CFG(5));
			/* Check if both QLM4 & QLM5 are BGX0 */
			if (g2.s.bgx) {
				if (g1.s.bgx) {
					cvmx_gserx_phy_ctl_t phy_ctl1;

					phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(4));
					phy_ctl1.u64 = csr_rd(CVMX_GSERX_PHY_CTL(5));
					if ((phy_ctl.s.phy_pd || phy_ctl.s.phy_reset) &&
					    (phy_ctl1.s.phy_pd || phy_ctl1.s.phy_reset))
						return -1;
					if (index >= 2)
						return 5;
					return 4;
				}

				/* QLM5 is BGX0 */
				phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(5));
				if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
					return -1;
				return 5;
			} else if (g1.s.bgx) {
				phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(4));
				if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
					return -1;
				return 4;
			}
		} else if (xi.interface < 2) {
			qlm = (xi.interface == 1) ? 2 : 3;
			gserx_cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));
			if (gserx_cfg.s.srio)
				return qlm;
		}
		return -1;
	}
	return -1;
}

/**
 * Return if only DLM5/DLM6/DLM5+DLM6 is used by BGX
 *
 * @param BGX  BGX to search for.
 *
 * Return: muxes used 0 = DLM5+DLM6, 1 = DLM5, 2 = DLM6.
 */
int cvmx_qlm_mux_interface(int bgx)
{
	int mux = 0;
	cvmx_gserx_cfg_t gser1, gser2;
	int qlm1, qlm2;

	if (OCTEON_IS_MODEL(OCTEON_CN73XX) && bgx != 2)
		return -1;
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX) && bgx != 0)
		return -1;

	if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		qlm1 = 5;
		qlm2 = 6;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		qlm1 = 4;
		qlm2 = 5;
	} else {
		return -1;
	}

	gser1.u64 = csr_rd(CVMX_GSERX_CFG(qlm1));
	gser2.u64 = csr_rd(CVMX_GSERX_CFG(qlm2));

	if (gser1.s.bgx && gser2.s.bgx)
		mux = 0;
	else if (gser1.s.bgx)
		mux = 1; // BGX2 is using DLM5 only
	else if (gser2.s.bgx)
		mux = 2; // BGX2 is using DLM6 only

	return mux;
}

/**
 * Return number of lanes for a given qlm
 *
 * @param qlm    QLM to examine
 *
 * Return:  Number of lanes
 */
int cvmx_qlm_get_lanes(int qlm)
{
	if (OCTEON_IS_MODEL(OCTEON_CN61XX) && qlm == 1)
		return 2;
	else if (OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return 2;
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return (qlm < 4) ? 4 /*QLM0,1,2,3*/ : 2 /*DLM4,5,6*/;
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return (qlm == 2 || qlm == 3) ? 4 /*QLM2,3*/ : 2 /*DLM0,1,4,5*/;
	return 4;
}

/**
 * Get the QLM JTAG fields based on Octeon model on the supported chips.
 *
 * Return:  qlm_jtag_field_t structure
 */
const __cvmx_qlm_jtag_field_t *cvmx_qlm_jtag_get_field(void)
{
	/* Figure out which JTAG chain description we're using */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		return __cvmx_qlm_jtag_field_cn68xx;
	} else if (OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		   OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		return __cvmx_qlm_jtag_field_cn66xx;
	} else if (OCTEON_IS_MODEL(OCTEON_CN63XX)) {
		return __cvmx_qlm_jtag_field_cn63xx;
	}

	return NULL;
}

/**
 * Get the QLM JTAG length by going through qlm_jtag_field for each
 * Octeon model that is supported
 *
 * Return: return the length.
 */
int cvmx_qlm_jtag_get_length(void)
{
	const __cvmx_qlm_jtag_field_t *qlm_ptr = cvmx_qlm_jtag_get_field();
	int length = 0;

	/* Figure out how many bits are in the JTAG chain */
	while (qlm_ptr && qlm_ptr->name) {
		if (qlm_ptr->stop_bit > length)
			length = qlm_ptr->stop_bit + 1;
		qlm_ptr++;
	}
	return length;
}

/**
 * Initialize the QLM layer
 */
void cvmx_qlm_init(void)
{
	if (OCTEON_IS_OCTEON3())
		return;

	/* ToDo: No support for non-Octeon 3 yet */
	printf("Please add support for unsupported Octeon SoC\n");
}

/**
 * Lookup the bit information for a JTAG field name
 *
 * @param name   Name to lookup
 *
 * Return: Field info, or NULL on failure
 */
static const __cvmx_qlm_jtag_field_t *__cvmx_qlm_lookup_field(const char *name)
{
	const __cvmx_qlm_jtag_field_t *ptr = cvmx_qlm_jtag_get_field();

	while (ptr->name) {
		if (strcmp(name, ptr->name) == 0)
			return ptr;
		ptr++;
	}

	debug("%s: Illegal field name %s\n", __func__, name);
	return NULL;
}

/**
 * Get a field in a QLM JTAG chain
 *
 * @param qlm    QLM to get
 * @param lane   Lane in QLM to get
 * @param name   String name of field
 *
 * Return: JTAG field value
 */
uint64_t cvmx_qlm_jtag_get(int qlm, int lane, const char *name)
{
	const __cvmx_qlm_jtag_field_t *field = __cvmx_qlm_lookup_field(name);
	int qlm_jtag_length = cvmx_qlm_jtag_get_length();
	int num_lanes = cvmx_qlm_get_lanes(qlm);

	if (!field)
		return 0;

	/* Capture the current settings */
	cvmx_helper_qlm_jtag_capture(qlm);
	/*
	 * Shift past lanes we don't care about. CN6XXX/7XXX shifts lane 0 first,
	 * CN3XXX/5XXX shifts lane 3 first
	 */
	/* Shift to the start of the field */
	cvmx_helper_qlm_jtag_shift_zeros(qlm,
					 qlm_jtag_length * (num_lanes - 1 - lane));
	cvmx_helper_qlm_jtag_shift_zeros(qlm, field->start_bit);
	/* Shift out the value and return it */
	return cvmx_helper_qlm_jtag_shift(qlm, field->stop_bit - field->start_bit + 1, 0);
}

/**
 * Set a field in a QLM JTAG chain
 *
 * @param qlm    QLM to set
 * @param lane   Lane in QLM to set, or -1 for all lanes
 * @param name   String name of field
 * @param value  Value of the field
 */
void cvmx_qlm_jtag_set(int qlm, int lane, const char *name, uint64_t value)
{
	int i, l;
	u32 shift_values[CVMX_QLM_JTAG_UINT32];
	int num_lanes = cvmx_qlm_get_lanes(qlm);
	const __cvmx_qlm_jtag_field_t *field = __cvmx_qlm_lookup_field(name);
	int qlm_jtag_length = cvmx_qlm_jtag_get_length();
	int total_length = qlm_jtag_length * num_lanes;
	int bits = 0;

	if (!field)
		return;

	/* Get the current state */
	cvmx_helper_qlm_jtag_capture(qlm);
	for (i = 0; i < CVMX_QLM_JTAG_UINT32; i++)
		shift_values[i] = cvmx_helper_qlm_jtag_shift(qlm, 32, 0);

	/* Put new data in our local array */
	for (l = 0; l < num_lanes; l++) {
		u64 new_value = value;
		int bits;
		int adj_lanes;

		if (l != lane && lane != -1)
			continue;

		adj_lanes = (num_lanes - 1 - l) * qlm_jtag_length;

		for (bits = field->start_bit + adj_lanes; bits <= field->stop_bit + adj_lanes;
		     bits++) {
			if (new_value & 1)
				shift_values[bits / 32] |= 1 << (bits & 31);
			else
				shift_values[bits / 32] &= ~(1 << (bits & 31));
			new_value >>= 1;
		}
	}

	/* Shift out data and xor with reference */
	while (bits < total_length) {
		u32 shift = shift_values[bits / 32] ^ __cvmx_qlm_jtag_xor_ref[qlm][bits / 32];
		int width = total_length - bits;

		if (width > 32)
			width = 32;
		cvmx_helper_qlm_jtag_shift(qlm, width, shift);
		bits += 32;
	}

	/* Update the new data */
	cvmx_helper_qlm_jtag_update(qlm);

	/*
	 * Always give the QLM 1ms to settle after every update. This may not
	 * always be needed, but some of the options make significant
	 * electrical changes
	 */
	udelay(1000);
}

/**
 * Errata G-16094: QLM Gen2 Equalizer Default Setting Change.
 * CN68XX pass 1.x and CN66XX pass 1.x QLM tweak. This function tweaks the
 * JTAG setting for a QLMs to run better at 5 and 6.25Ghz.
 */
void __cvmx_qlm_speed_tweak(void)
{
	cvmx_mio_qlmx_cfg_t qlm_cfg;
	int num_qlms = cvmx_qlm_get_num();
	int qlm;

	/* Workaround for Errata (G-16467) */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X)) {
		for (qlm = 0; qlm < num_qlms; qlm++) {
			int ir50dac;

			/*
			 * This workaround only applies to QLMs running at
			 * 6.25Ghz
			 */
			if (cvmx_qlm_get_gbaud_mhz(qlm) == 6250) {
#ifdef CVMX_QLM_DUMP_STATE
				debug("%s:%d: QLM%d: Applying workaround for Errata G-16467\n",
				      __func__, __LINE__, qlm);
				cvmx_qlm_display_registers(qlm);
				debug("\n");
#endif
				cvmx_qlm_jtag_set(qlm, -1, "cfg_cdr_trunc", 0);
				/* Hold the QLM in reset */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_rst_n_set", 0);
				cvmx_qlm_jtag_set(qlm, -1, "cfg_rst_n_clr", 1);
				/* Forcfe TX to be idle */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_tx_idle_clr", 0);
				cvmx_qlm_jtag_set(qlm, -1, "cfg_tx_idle_set", 1);
				if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_0)) {
					ir50dac = cvmx_qlm_jtag_get(qlm, 0, "ir50dac");
					while (++ir50dac <= 31)
						cvmx_qlm_jtag_set(qlm, -1, "ir50dac", ir50dac);
				}
				cvmx_qlm_jtag_set(qlm, -1, "div4_byp", 0);
				cvmx_qlm_jtag_set(qlm, -1, "clkf_byp", 16);
				cvmx_qlm_jtag_set(qlm, -1, "serdes_pll_byp", 1);
				cvmx_qlm_jtag_set(qlm, -1, "spdsel_byp", 1);
#ifdef CVMX_QLM_DUMP_STATE
				debug("%s:%d: QLM%d: Done applying workaround for Errata G-16467\n",
				      __func__, __LINE__, qlm);
				cvmx_qlm_display_registers(qlm);
				debug("\n\n");
#endif
				/*
				 * The QLM will be taken out of reset later
				 * when ILK/XAUI are initialized.
				 */
			}
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_X) ||
		   OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_X)) {
		/* Loop through the QLMs */
		for (qlm = 0; qlm < num_qlms; qlm++) {
			/* Read the QLM speed */
			qlm_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));

			/* If the QLM is at 6.25Ghz or 5Ghz then program JTAG */
			if (qlm_cfg.s.qlm_spd == 5 || qlm_cfg.s.qlm_spd == 12 ||
			    qlm_cfg.s.qlm_spd == 0 || qlm_cfg.s.qlm_spd == 6 ||
			    qlm_cfg.s.qlm_spd == 11) {
				cvmx_qlm_jtag_set(qlm, -1, "rx_cap_gen2", 0x1);
				cvmx_qlm_jtag_set(qlm, -1, "rx_eq_gen2", 0x8);
			}
		}
	}
}

/**
 * Errata G-16174: QLM Gen2 PCIe IDLE DAC change.
 * CN68XX pass 1.x, CN66XX pass 1.x and CN63XX pass 1.0-2.2 QLM tweak.
 * This function tweaks the JTAG setting for a QLMs for PCIe to run better.
 */
void __cvmx_qlm_pcie_idle_dac_tweak(void)
{
	int num_qlms = 0;
	int qlm;

	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_X))
		num_qlms = 5;
	else if (OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_X))
		num_qlms = 3;
	else if (OCTEON_IS_MODEL(OCTEON_CN63XX))
		num_qlms = 3;
	else
		return;

	/* Loop through the QLMs */
	for (qlm = 0; qlm < num_qlms; qlm++)
		cvmx_qlm_jtag_set(qlm, -1, "idle_dac", 0x2);
}

void __cvmx_qlm_pcie_cfg_rxd_set_tweak(int qlm, int lane)
{
	if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_MODEL(OCTEON_CNF71XX))
		cvmx_qlm_jtag_set(qlm, lane, "cfg_rxd_set", 0x1);
}

/**
 * Get the speed (Gbaud) of the QLM in Mhz for a given node.
 *
 * @param node   node of the QLM
 * @param qlm    QLM to examine
 *
 * Return: Speed in Mhz
 */
int cvmx_qlm_get_gbaud_mhz_node(int node, int qlm)
{
	cvmx_gserx_lane_mode_t lane_mode;
	cvmx_gserx_cfg_t cfg;

	if (!octeon_has_feature(OCTEON_FEATURE_MULTINODE))
		return 0;

	if (qlm >= 8)
		return -1; /* FIXME for OCI */
	/* Check if QLM is configured */
	cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));
	if (cfg.u64 == 0)
		return -1;
	if (cfg.s.pcie) {
		int pem = 0;
		cvmx_pemx_cfg_t pemx_cfg;

		switch (qlm) {
		case 0: /* Either PEM0 x4 of PEM0 x8 */
			pem = 0;
			break;
		case 1: /* Either PEM0 x4 of PEM1 x4 */
			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(0));
			if (pemx_cfg.cn78xx.lanes8)
				pem = 0;
			else
				pem = 1;
			break;
		case 2: /* Either PEM2 x4 of PEM2 x8 */
			pem = 2;
			break;
		case 3: /* Either PEM2 x8 of PEM3 x4 or x8 */
			/* Can be last 4 lanes of PEM2 */
			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(2));
			if (pemx_cfg.cn78xx.lanes8) {
				pem = 2;
			} else {
				pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(3));
				if (pemx_cfg.cn78xx.lanes8)
					pem = 3;
				else
					pem = 2;
			}
			break;
		case 4: /* Either PEM3 x8 of PEM3 x4 */
			pem = 3;
			break;
		default:
			debug("QLM%d: Should be in PCIe mode\n", qlm);
			break;
		}
		pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(pem));
		switch (pemx_cfg.s.md) {
		case 0: /* Gen1 */
			return 2500;
		case 1: /* Gen2 */
			return 5000;
		case 2: /* Gen3 */
			return 8000;
		default:
			return 0;
		}
	} else {
		lane_mode.u64 = csr_rd_node(node, CVMX_GSERX_LANE_MODE(qlm));
		switch (lane_mode.s.lmode) {
		case R_25G_REFCLK100:
			return 2500;
		case R_5G_REFCLK100:
			return 5000;
		case R_8G_REFCLK100:
			return 8000;
		case R_125G_REFCLK15625_KX:
			return 1250;
		case R_3125G_REFCLK15625_XAUI:
			return 3125;
		case R_103125G_REFCLK15625_KR:
			return 10312;
		case R_125G_REFCLK15625_SGMII:
			return 1250;
		case R_5G_REFCLK15625_QSGMII:
			return 5000;
		case R_625G_REFCLK15625_RXAUI:
			return 6250;
		case R_25G_REFCLK125:
			return 2500;
		case R_5G_REFCLK125:
			return 5000;
		case R_8G_REFCLK125:
			return 8000;
		default:
			return 0;
		}
	}
}

/**
 * Get the speed (Gbaud) of the QLM in Mhz.
 *
 * @param qlm    QLM to examine
 *
 * Return: Speed in Mhz
 */
int cvmx_qlm_get_gbaud_mhz(int qlm)
{
	if (OCTEON_IS_MODEL(OCTEON_CN63XX)) {
		if (qlm == 2) {
			cvmx_gmxx_inf_mode_t inf_mode;

			inf_mode.u64 = csr_rd(CVMX_GMXX_INF_MODE(0));
			switch (inf_mode.s.speed) {
			case 0:
				return 5000; /* 5     Gbaud */
			case 1:
				return 2500; /* 2.5   Gbaud */
			case 2:
				return 2500; /* 2.5   Gbaud */
			case 3:
				return 1250; /* 1.25  Gbaud */
			case 4:
				return 1250; /* 1.25  Gbaud */
			case 5:
				return 6250; /* 6.25  Gbaud */
			case 6:
				return 5000; /* 5     Gbaud */
			case 7:
				return 2500; /* 2.5   Gbaud */
			case 8:
				return 3125; /* 3.125 Gbaud */
			case 9:
				return 2500; /* 2.5   Gbaud */
			case 10:
				return 1250; /* 1.25  Gbaud */
			case 11:
				return 5000; /* 5     Gbaud */
			case 12:
				return 6250; /* 6.25  Gbaud */
			case 13:
				return 3750; /* 3.75  Gbaud */
			case 14:
				return 3125; /* 3.125 Gbaud */
			default:
				return 0; /* Disabled */
			}
		} else {
			cvmx_sriox_status_reg_t status_reg;

			status_reg.u64 = csr_rd(CVMX_SRIOX_STATUS_REG(qlm));
			if (status_reg.s.srio) {
				cvmx_sriomaintx_port_0_ctl2_t sriomaintx_port_0_ctl2;

				sriomaintx_port_0_ctl2.u32 =
					csr_rd(CVMX_SRIOMAINTX_PORT_0_CTL2(qlm));
				switch (sriomaintx_port_0_ctl2.s.sel_baud) {
				case 1:
					return 1250; /* 1.25  Gbaud */
				case 2:
					return 2500; /* 2.5   Gbaud */
				case 3:
					return 3125; /* 3.125 Gbaud */
				case 4:
					return 5000; /* 5     Gbaud */
				case 5:
					return 6250; /* 6.250 Gbaud */
				default:
					return 0; /* Disabled */
				}
			} else {
				cvmx_pciercx_cfg032_t pciercx_cfg032;

				pciercx_cfg032.u32 = csr_rd(CVMX_PCIERCX_CFG032(qlm));
				switch (pciercx_cfg032.s.ls) {
				case 1:
					return 2500;
				case 2:
					return 5000;
				case 4:
					return 8000;
				default: {
					cvmx_mio_rst_boot_t mio_rst_boot;

					mio_rst_boot.u64 = csr_rd(CVMX_MIO_RST_BOOT);
					if (qlm == 0 && mio_rst_boot.s.qlm0_spd == 0xf)
						return 0;

					if (qlm == 1 && mio_rst_boot.s.qlm1_spd == 0xf)
						return 0;

					/* Best guess I can make */
					return 5000;
				}
				}
			}
		}
	} else if (OCTEON_IS_OCTEON2()) {
		cvmx_mio_qlmx_cfg_t qlm_cfg;

		qlm_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));
		switch (qlm_cfg.s.qlm_spd) {
		case 0:
			return 5000; /* 5     Gbaud */
		case 1:
			return 2500; /* 2.5   Gbaud */
		case 2:
			return 2500; /* 2.5   Gbaud */
		case 3:
			return 1250; /* 1.25  Gbaud */
		case 4:
			return 1250; /* 1.25  Gbaud */
		case 5:
			return 6250; /* 6.25  Gbaud */
		case 6:
			return 5000; /* 5     Gbaud */
		case 7:
			return 2500; /* 2.5   Gbaud */
		case 8:
			return 3125; /* 3.125 Gbaud */
		case 9:
			return 2500; /* 2.5   Gbaud */
		case 10:
			return 1250; /* 1.25  Gbaud */
		case 11:
			return 5000; /* 5     Gbaud */
		case 12:
			return 6250; /* 6.25  Gbaud */
		case 13:
			return 3750; /* 3.75  Gbaud */
		case 14:
			return 3125; /* 3.125 Gbaud */
		default:
			return 0; /* Disabled */
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		cvmx_gserx_dlmx_mpll_multiplier_t mpll_multiplier;
		u64 meas_refclock;
		u64 freq;

		/* Measure the reference clock */
		meas_refclock = cvmx_qlm_measure_clock(qlm);
		/* Multiply to get the final frequency */
		mpll_multiplier.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
		freq = meas_refclock * mpll_multiplier.s.mpll_multiplier;
		freq = (freq + 500000) / 1000000;

		return freq;
	} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		return cvmx_qlm_get_gbaud_mhz_node(cvmx_get_node_num(), qlm);
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_gserx_lane_mode_t lane_mode;

		lane_mode.u64 = csr_rd(CVMX_GSERX_LANE_MODE(qlm));
		switch (lane_mode.s.lmode) {
		case R_25G_REFCLK100:
			return 2500;
		case R_5G_REFCLK100:
			return 5000;
		case R_8G_REFCLK100:
			return 8000;
		case R_125G_REFCLK15625_KX:
			return 1250;
		case R_3125G_REFCLK15625_XAUI:
			return 3125;
		case R_103125G_REFCLK15625_KR:
			return 10312;
		case R_125G_REFCLK15625_SGMII:
			return 1250;
		case R_5G_REFCLK15625_QSGMII:
			return 5000;
		case R_625G_REFCLK15625_RXAUI:
			return 6250;
		case R_25G_REFCLK125:
			return 2500;
		case R_5G_REFCLK125:
			return 5000;
		case R_8G_REFCLK125:
			return 8000;
		default:
			return 0;
		}
	}
	return 0;
}

static enum cvmx_qlm_mode __cvmx_qlm_get_mode_cn70xx(int qlm)
{
	switch (qlm) {
	case 0: /* DLM0/DLM1 - SGMII/QSGMII/RXAUI */
	{
		union cvmx_gmxx_inf_mode inf_mode0, inf_mode1;

		inf_mode0.u64 = csr_rd(CVMX_GMXX_INF_MODE(0));
		inf_mode1.u64 = csr_rd(CVMX_GMXX_INF_MODE(1));

		/* SGMII0 SGMII1 */
		switch (inf_mode0.s.mode) {
		case CVMX_GMX_INF_MODE_SGMII:
			switch (inf_mode1.s.mode) {
			case CVMX_GMX_INF_MODE_SGMII:
				return CVMX_QLM_MODE_SGMII_SGMII;
			case CVMX_GMX_INF_MODE_QSGMII:
				return CVMX_QLM_MODE_SGMII_QSGMII;
			default:
				return CVMX_QLM_MODE_SGMII_DISABLED;
			}
		case CVMX_GMX_INF_MODE_QSGMII:
			switch (inf_mode1.s.mode) {
			case CVMX_GMX_INF_MODE_SGMII:
				return CVMX_QLM_MODE_QSGMII_SGMII;
			case CVMX_GMX_INF_MODE_QSGMII:
				return CVMX_QLM_MODE_QSGMII_QSGMII;
			default:
				return CVMX_QLM_MODE_QSGMII_DISABLED;
			}
		case CVMX_GMX_INF_MODE_RXAUI:
			return CVMX_QLM_MODE_RXAUI_1X2;
		default:
			switch (inf_mode1.s.mode) {
			case CVMX_GMX_INF_MODE_SGMII:
				return CVMX_QLM_MODE_DISABLED_SGMII;
			case CVMX_GMX_INF_MODE_QSGMII:
				return CVMX_QLM_MODE_DISABLED_QSGMII;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
		}
	}
	case 1: /* Sata / pem0 */
	{
		union cvmx_gserx_sata_cfg sata_cfg;
		union cvmx_pemx_cfg pem0_cfg;

		sata_cfg.u64 = csr_rd(CVMX_GSERX_SATA_CFG(0));
		pem0_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));

		switch (pem0_cfg.cn70xx.md) {
		case CVMX_PEM_MD_GEN2_2LANE:
		case CVMX_PEM_MD_GEN1_2LANE:
			return CVMX_QLM_MODE_PCIE_1X2;
		case CVMX_PEM_MD_GEN2_1LANE:
		case CVMX_PEM_MD_GEN1_1LANE:
			if (sata_cfg.s.sata_en)
				/* Both PEM0 and PEM1 */
				return CVMX_QLM_MODE_PCIE_2X1;

			/* Only PEM0 */
			return CVMX_QLM_MODE_PCIE_1X1;
		case CVMX_PEM_MD_GEN2_4LANE:
		case CVMX_PEM_MD_GEN1_4LANE:
			return CVMX_QLM_MODE_PCIE;
		default:
			return CVMX_QLM_MODE_DISABLED;
		}
	}
	case 2: {
		union cvmx_gserx_sata_cfg sata_cfg;
		union cvmx_pemx_cfg pem0_cfg, pem1_cfg, pem2_cfg;

		sata_cfg.u64 = csr_rd(CVMX_GSERX_SATA_CFG(0));
		pem0_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));
		pem1_cfg.u64 = csr_rd(CVMX_PEMX_CFG(1));
		pem2_cfg.u64 = csr_rd(CVMX_PEMX_CFG(2));

		if (sata_cfg.s.sata_en)
			return CVMX_QLM_MODE_SATA_2X1;
		if (pem0_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_4LANE ||
		    pem0_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_4LANE)
			return CVMX_QLM_MODE_PCIE;
		if (pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_2LANE ||
		    pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_2LANE) {
			return CVMX_QLM_MODE_PCIE_1X2;
		}
		if (pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_1LANE ||
		    pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_1LANE) {
			if (pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_1LANE ||
			    pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_1LANE) {
				return CVMX_QLM_MODE_PCIE_2X1;
			} else {
				return CVMX_QLM_MODE_PCIE_1X1;
			}
		}
		if (pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_1LANE ||
		    pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_1LANE)
			return CVMX_QLM_MODE_PCIE_2X1;
		return CVMX_QLM_MODE_DISABLED;
	}
	default:
		return CVMX_QLM_MODE_DISABLED;
	}

	return CVMX_QLM_MODE_DISABLED;
}

/*
 * Get the DLM mode for the interface based on the interface type.
 *
 * @param interface_type   0 - SGMII/QSGMII/RXAUI interface
 *                         1 - PCIe
 *                         2 - SATA
 * @param interface        interface to use
 * Return:  the qlm mode the interface is
 */
enum cvmx_qlm_mode cvmx_qlm_get_dlm_mode(int interface_type, int interface)
{
	switch (interface_type) {
	case 0: /* SGMII/QSGMII/RXAUI */
	{
		enum cvmx_qlm_mode qlm_mode = __cvmx_qlm_get_mode_cn70xx(0);

		switch (interface) {
		case 0:
			switch (qlm_mode) {
			case CVMX_QLM_MODE_SGMII_SGMII:
			case CVMX_QLM_MODE_SGMII_DISABLED:
			case CVMX_QLM_MODE_SGMII_QSGMII:
				return CVMX_QLM_MODE_SGMII;
			case CVMX_QLM_MODE_QSGMII_QSGMII:
			case CVMX_QLM_MODE_QSGMII_DISABLED:
			case CVMX_QLM_MODE_QSGMII_SGMII:
				return CVMX_QLM_MODE_QSGMII;
			case CVMX_QLM_MODE_RXAUI_1X2:
				return CVMX_QLM_MODE_RXAUI;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
		case 1:
			switch (qlm_mode) {
			case CVMX_QLM_MODE_SGMII_SGMII:
			case CVMX_QLM_MODE_DISABLED_SGMII:
			case CVMX_QLM_MODE_QSGMII_SGMII:
				return CVMX_QLM_MODE_SGMII;
			case CVMX_QLM_MODE_QSGMII_QSGMII:
			case CVMX_QLM_MODE_DISABLED_QSGMII:
			case CVMX_QLM_MODE_SGMII_QSGMII:
				return CVMX_QLM_MODE_QSGMII;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
		default:
			return qlm_mode;
		}
	}
	case 1: /* PCIe */
	{
		enum cvmx_qlm_mode qlm_mode1 = __cvmx_qlm_get_mode_cn70xx(1);
		enum cvmx_qlm_mode qlm_mode2 = __cvmx_qlm_get_mode_cn70xx(2);

		switch (interface) {
		case 0: /* PCIe0 can be DLM1 with 1, 2 or 4 lanes */
			return qlm_mode1;
		case 1:
			/*
			 * PCIe1 can be in DLM1 1 lane(1), DLM2 1 lane(0)
			 * or 2 lanes(0-1)
			 */
			if (qlm_mode1 == CVMX_QLM_MODE_PCIE_2X1)
				return CVMX_QLM_MODE_PCIE_2X1;
			else if (qlm_mode2 == CVMX_QLM_MODE_PCIE_1X2 ||
				 qlm_mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return qlm_mode2;
			else
				return CVMX_QLM_MODE_DISABLED;
		case 2: /* PCIe2 can be DLM2 1 lanes(1) */
			if (qlm_mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return qlm_mode2;
			else
				return CVMX_QLM_MODE_DISABLED;
		default:
			return CVMX_QLM_MODE_DISABLED;
		}
	}
	case 2: /* SATA */
	{
		enum cvmx_qlm_mode qlm_mode = __cvmx_qlm_get_mode_cn70xx(2);

		if (qlm_mode == CVMX_QLM_MODE_SATA_2X1)
			return CVMX_QLM_MODE_SATA_2X1;
		else
			return CVMX_QLM_MODE_DISABLED;
	}
	default:
		return CVMX_QLM_MODE_DISABLED;
	}
}

static enum cvmx_qlm_mode __cvmx_qlm_get_mode_cn6xxx(int qlm)
{
	cvmx_mio_qlmx_cfg_t qlmx_cfg;

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		qlmx_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlmx_cfg.s.qlm_cfg) {
		case 0: /* PCIE */
			return CVMX_QLM_MODE_PCIE;
		case 1: /* ILK */
			return CVMX_QLM_MODE_ILK;
		case 2: /* SGMII */
			return CVMX_QLM_MODE_SGMII;
		case 3: /* XAUI */
			return CVMX_QLM_MODE_XAUI;
		case 7: /* RXAUI */
			return CVMX_QLM_MODE_RXAUI;
		default:
			return CVMX_QLM_MODE_DISABLED;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		qlmx_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlmx_cfg.s.qlm_cfg) {
		case 0x9: /* SGMII */
			return CVMX_QLM_MODE_SGMII;
		case 0xb: /* XAUI */
			return CVMX_QLM_MODE_XAUI;
		case 0x0: /* PCIE gen2 */
		case 0x8: /* PCIE gen2 (alias) */
		case 0x2: /* PCIE gen1 */
		case 0xa: /* PCIE gen1 (alias) */
			return CVMX_QLM_MODE_PCIE;
		case 0x1: /* SRIO 1x4 short */
		case 0x3: /* SRIO 1x4 long */
			return CVMX_QLM_MODE_SRIO_1X4;
		case 0x4: /* SRIO 2x2 short */
		case 0x6: /* SRIO 2x2 long */
			return CVMX_QLM_MODE_SRIO_2X2;
		case 0x5: /* SRIO 4x1 short */
		case 0x7: /* SRIO 4x1 long */
			if (!OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_0))
				return CVMX_QLM_MODE_SRIO_4X1;
			fallthrough;
		default:
			return CVMX_QLM_MODE_DISABLED;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN63XX)) {
		cvmx_sriox_status_reg_t status_reg;
		/* For now skip qlm2 */
		if (qlm == 2) {
			cvmx_gmxx_inf_mode_t inf_mode;

			inf_mode.u64 = csr_rd(CVMX_GMXX_INF_MODE(0));
			if (inf_mode.s.speed == 15)
				return CVMX_QLM_MODE_DISABLED;
			else if (inf_mode.s.mode == 0)
				return CVMX_QLM_MODE_SGMII;
			else
				return CVMX_QLM_MODE_XAUI;
		}
		status_reg.u64 = csr_rd(CVMX_SRIOX_STATUS_REG(qlm));
		if (status_reg.s.srio)
			return CVMX_QLM_MODE_SRIO_1X4;
		else
			return CVMX_QLM_MODE_PCIE;
	} else if (OCTEON_IS_MODEL(OCTEON_CN61XX)) {
		qlmx_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlm) {
		case 0:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 0: /* PCIe 1x4 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE;
			case 2: /* SGMII */
				return CVMX_QLM_MODE_SGMII;
			case 3: /* XAUI */
				return CVMX_QLM_MODE_XAUI;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 1:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 0: /* PCIe 1x2 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_1X2;
			case 1: /* PCIe 2x1 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_2X1;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 2:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 2: /* SGMII */
				return CVMX_QLM_MODE_SGMII;
			case 3: /* XAUI */
				return CVMX_QLM_MODE_XAUI;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		qlmx_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlm) {
		case 0:
			if (qlmx_cfg.s.qlm_cfg == 2) /* SGMII */
				return CVMX_QLM_MODE_SGMII;
			break;
		case 1:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 0: /* PCIe 1x2 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_1X2;
			case 1: /* PCIe 2x1 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_2X1;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		}
	}
	return CVMX_QLM_MODE_DISABLED;
}

/**
 * @INTERNAL
 * Decrement the MPLL Multiplier for the DLM as per Errata G-20669
 *
 * @param qlm            DLM to configure
 * @param baud_mhz       Speed of the DLM configured at
 * @param old_multiplier MPLL_MULTIPLIER value to decrement
 */
void __cvmx_qlm_set_mult(int qlm, int baud_mhz, int old_multiplier)
{
	cvmx_gserx_dlmx_mpll_multiplier_t mpll_multiplier;
	cvmx_gserx_dlmx_ref_clkdiv2_t clkdiv;
	u64 meas_refclock, mult;

	if (!OCTEON_IS_MODEL(OCTEON_CN70XX))
		return;

	if (qlm == -1)
		return;

	meas_refclock = cvmx_qlm_measure_clock(qlm);
	if (meas_refclock == 0) {
		printf("DLM%d: Reference clock not running\n", qlm);
		return;
	}

	/*
	 * The baud rate multiplier needs to be adjusted on the CN70XX if
	 * the reference clock is > 100MHz.
	 */
	if (qlm == 0) {
		clkdiv.u64 = csr_rd(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0));
		if (clkdiv.s.ref_clkdiv2)
			baud_mhz *= 2;
	}
	mult = (uint64_t)baud_mhz * 1000000 + (meas_refclock / 2);
	mult /= meas_refclock;

	/*
	 * 6. Decrease MPLL_MULTIPLIER by one continually until it reaches
	 * the desired long-term setting, ensuring that each MPLL_MULTIPLIER
	 * value is constant for at least 1 msec before changing to the next
	 * value. The desired long-term setting is as indicated in HRM tables
	 * 21-1, 21-2, and 21-3. This is not required with the HRM
	 * sequence.
	 */
	do {
		mpll_multiplier.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
		mpll_multiplier.s.mpll_multiplier = --old_multiplier;
		csr_wr(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0), mpll_multiplier.u64);
		/* Wait for 1 ms */
		udelay(1000);
	} while (old_multiplier > (int)mult);
}

enum cvmx_qlm_mode cvmx_qlm_get_mode_cn78xx(int node, int qlm)
{
	cvmx_gserx_cfg_t gserx_cfg;
	int qlm_mode[2][9] = { { -1, -1, -1, -1, -1, -1, -1, -1 },
			       { -1, -1, -1, -1, -1, -1, -1, -1 } };

	if (qlm >= 8)
		return CVMX_QLM_MODE_OCI;

	if (qlm_mode[node][qlm] != -1)
		return qlm_mode[node][qlm];

	gserx_cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));
	if (gserx_cfg.s.pcie) {
		switch (qlm) {
		case 0: /* Either PEM0 x4 or PEM0 x8 */
		case 1: /* Either PEM0 x8 or PEM1 x4 */
		{
			cvmx_pemx_cfg_t pemx_cfg;

			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(0));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM0 x8 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else {
				/* PEM0 x4 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE;
			}
			break;
		}
		case 2: /* Either PEM2 x4 or PEM2 x8 */
		{
			cvmx_pemx_cfg_t pemx_cfg;

			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(2));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM2 x8 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else {
				/* PEM2 x4 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE;
			}
			break;
		}
		case 3: /* Either PEM2 x8 or PEM3 x4 or PEM3 x8 */
		{
			cvmx_pemx_cfg_t pemx_cfg;

			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(2));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM2 x8 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE_1X8;
			}

			/* Can be first 4 lanes of PEM3 */
			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(3));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM3 x8 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else {
				/* PEM2 x4 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE;
			}
			break;
		}
		case 4: /* Either PEM3 x8 or PEM3 x4 */
		{
			cvmx_pemx_cfg_t pemx_cfg;

			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(3));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM3 x8 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else {
				/* PEM3 x4 */
				qlm_mode[node][qlm] = CVMX_QLM_MODE_PCIE;
			}
			break;
		}
		default:
			qlm_mode[node][qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		}
	} else if (gserx_cfg.s.ila) {
		qlm_mode[node][qlm] = CVMX_QLM_MODE_ILK;
	} else if (gserx_cfg.s.bgx) {
		cvmx_bgxx_cmrx_config_t cmr_config;
		cvmx_bgxx_spux_br_pmd_control_t pmd_control;
		int bgx = (qlm < 2) ? qlm : qlm - 2;

		cmr_config.u64 = csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(0, bgx));
		pmd_control.u64 = csr_rd_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(0, bgx));

		switch (cmr_config.s.lmac_type) {
		case 0:
			qlm_mode[node][qlm] = CVMX_QLM_MODE_SGMII;
			break;
		case 1:
			qlm_mode[node][qlm] = CVMX_QLM_MODE_XAUI;
			break;
		case 2:
			qlm_mode[node][qlm] = CVMX_QLM_MODE_RXAUI;
			break;
		case 3:
			/*
			 * Use training to determine if we're in 10GBASE-KR
			 * or XFI
			 */
			if (pmd_control.s.train_en)
				qlm_mode[node][qlm] = CVMX_QLM_MODE_10G_KR;
			else
				qlm_mode[node][qlm] = CVMX_QLM_MODE_XFI;
			break;
		case 4:
			/*
			 * Use training to determine if we're in 40GBASE-KR
			 * or XLAUI
			 */
			if (pmd_control.s.train_en)
				qlm_mode[node][qlm] = CVMX_QLM_MODE_40G_KR4;
			else
				qlm_mode[node][qlm] = CVMX_QLM_MODE_XLAUI;
			break;
		default:
			qlm_mode[node][qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		}
	} else {
		qlm_mode[node][qlm] = CVMX_QLM_MODE_DISABLED;
	}

	return qlm_mode[node][qlm];
}

enum cvmx_qlm_mode __cvmx_qlm_get_mode_cn73xx(int qlm)
{
	cvmx_gserx_cfg_t gserx_cfg;
	int qlm_mode[7] = { -1, -1, -1, -1, -1, -1, -1 };

	if (qlm_mode[qlm] != -1)
		return qlm_mode[qlm];

	if (qlm > 6) {
		debug("Invalid QLM(%d) passed\n", qlm);
		return -1;
	}

	gserx_cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));
	if (gserx_cfg.s.pcie) {
		cvmx_pemx_cfg_t pemx_cfg;

		switch (qlm) {
		case 0: /* Either PEM0 x4 or PEM0 x8 */
		case 1: /* Either PEM0 x8 or PEM1 x4 */
		{
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM0 x8 */
				qlm_mode[qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else {
				/* PEM0/PEM1 x4 */
				qlm_mode[qlm] = CVMX_QLM_MODE_PCIE;
			}
			break;
		}
		case 2: /* Either PEM2 x4 or PEM2 x8 */
		{
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(2));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM2 x8 */
				qlm_mode[qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else {
				/* PEM2 x4 */
				qlm_mode[qlm] = CVMX_QLM_MODE_PCIE;
			}
			break;
		}
		case 5:
		case 6:						/* PEM3 x2 */
			qlm_mode[qlm] = CVMX_QLM_MODE_PCIE_1X2; /* PEM3 x2 */
			break;
		case 3: /* Either PEM2 x8 or PEM3 x4 */
		{
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(2));
			if (pemx_cfg.cn78xx.lanes8) {
				/* PEM2 x8 */
				qlm_mode[qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else {
				/* PEM3 x4 */
				qlm_mode[qlm] = CVMX_QLM_MODE_PCIE;
			}
			break;
		}
		default:
			qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		}
	} else if (gserx_cfg.s.bgx) {
		cvmx_bgxx_cmrx_config_t cmr_config;
		cvmx_bgxx_cmr_rx_lmacs_t bgx_cmr_rx_lmacs;
		cvmx_bgxx_spux_br_pmd_control_t pmd_control;
		int bgx = 0;
		int start = 0, end = 4, index;
		int lane_mask = 0, train_mask = 0;
		int mux = 0; // 0:BGX2 (DLM5/DLM6), 1:BGX2(DLM5), 2:BGX2(DLM6)

		if (qlm < 4) {
			bgx = qlm - 2;
		} else if (qlm == 5 || qlm == 6) {
			bgx = 2;
			mux = cvmx_qlm_mux_interface(bgx);
			if (mux == 0) {
				start = 0;
				end = 4;
			} else if (mux == 1) {
				start = 0;
				end = 2;
			} else if (mux == 2) {
				start = 2;
				end = 4;
			} else {
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
				return qlm_mode[qlm];
			}
		}

		for (index = start; index < end; index++) {
			cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(index, bgx));
			pmd_control.u64 = csr_rd(CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx));
			lane_mask |= (cmr_config.s.lmac_type << (index * 4));
			train_mask |= (pmd_control.s.train_en << (index * 4));
		}

		/* Need to include DLM5 lmacs when only DLM6 DLM is used */
		if (mux == 2)
			bgx_cmr_rx_lmacs.u64 = csr_rd(CVMX_BGXX_CMR_RX_LMACS(2));
		switch (lane_mask) {
		case 0:
			if (mux == 1) {
				qlm_mode[qlm] = CVMX_QLM_MODE_SGMII_2X1;
			} else if (mux == 2) {
				qlm_mode[qlm] = CVMX_QLM_MODE_SGMII_2X1;
				bgx_cmr_rx_lmacs.s.lmacs = 4;
			}
			qlm_mode[qlm] = CVMX_QLM_MODE_SGMII;
			break;
		case 0x1:
			qlm_mode[qlm] = CVMX_QLM_MODE_XAUI;
			break;
		case 0x2:
			if (mux == 1) {
				// NONE+RXAUI
				qlm_mode[qlm] = CVMX_QLM_MODE_RXAUI_1X2;
			} else if (mux == 0) {
				// RXAUI+SGMII
				qlm_mode[qlm] = CVMX_QLM_MODE_MIXED;
			} else {
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 0x202:
			if (mux == 2) {
				// RXAUI+RXAUI
				qlm_mode[qlm] = CVMX_QLM_MODE_RXAUI_1X2;
				bgx_cmr_rx_lmacs.s.lmacs = 4;
			} else if (mux == 1) {
				// RXAUI+RXAUI
				qlm_mode[qlm] = CVMX_QLM_MODE_RXAUI_1X2;
			} else if (mux == 0) {
				qlm_mode[qlm] = CVMX_QLM_MODE_RXAUI;
			} else {
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 0x22:
			qlm_mode[qlm] = CVMX_QLM_MODE_RXAUI;
			break;
		case 0x3333:
			/*
			 * Use training to determine if we're in 10GBASE-KR
			 * or XFI
			 */
			if (train_mask)
				qlm_mode[qlm] = CVMX_QLM_MODE_10G_KR;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_XFI;
			break;
		case 0x4:
			/*
			 * Use training to determine if we're in 40GBASE-KR
			 * or XLAUI
			 */
			if (train_mask)
				qlm_mode[qlm] = CVMX_QLM_MODE_40G_KR4;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_XLAUI;
			break;
		case 0x0005:
			qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_SGMII;
			break;
		case 0x3335:
			if (train_mask)
				qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_10G_KR;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_XFI;
			break;
		case 0x45:
			if (train_mask)
				qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_40G_KR4;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_XLAUI;
			break;
		case 0x225:
			qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_RXAUI;
			break;
		case 0x15:
			qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_XAUI;
			break;

		case 0x200:
			if (mux == 2) {
				qlm_mode[qlm] = CVMX_QLM_MODE_RXAUI_1X2;
				bgx_cmr_rx_lmacs.s.lmacs = 4;
			} else
		case 0x205:
		case 0x233:
		case 0x3302:
		case 0x3305:
			if (mux == 0)
				qlm_mode[qlm] = CVMX_QLM_MODE_MIXED;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		case 0x3300:
			if (mux == 0) {
				qlm_mode[qlm] = CVMX_QLM_MODE_MIXED;
			} else if (mux == 2) {
				if (train_mask)
					qlm_mode[qlm] = CVMX_QLM_MODE_10G_KR_1X2;
				else
					qlm_mode[qlm] = CVMX_QLM_MODE_XFI_1X2;
				bgx_cmr_rx_lmacs.s.lmacs = 4;
			} else {
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 0x33:
			if (mux == 1 || mux == 2) {
				if (train_mask)
					qlm_mode[qlm] = CVMX_QLM_MODE_10G_KR_1X2;
				else
					qlm_mode[qlm] = CVMX_QLM_MODE_XFI_1X2;
				if (mux == 2)
					bgx_cmr_rx_lmacs.s.lmacs = 4;
			} else {
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 0x0035:
			if (mux == 0)
				qlm_mode[qlm] = CVMX_QLM_MODE_MIXED;
			else if (train_mask)
				qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_10G_KR_1X1;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_RGMII_XFI_1X1;
			break;
		case 0x235:
			if (mux == 0)
				qlm_mode[qlm] = CVMX_QLM_MODE_MIXED;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		default:
			qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		}
		if (mux == 2) {
			csr_wr(CVMX_BGXX_CMR_RX_LMACS(2), bgx_cmr_rx_lmacs.u64);
			csr_wr(CVMX_BGXX_CMR_TX_LMACS(2), bgx_cmr_rx_lmacs.u64);
		}
	} else if (gserx_cfg.s.sata) {
		qlm_mode[qlm] = CVMX_QLM_MODE_SATA_2X1;
	} else {
		qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
	}

	return qlm_mode[qlm];
}

enum cvmx_qlm_mode __cvmx_qlm_get_mode_cnf75xx(int qlm)
{
	cvmx_gserx_cfg_t gserx_cfg;
	int qlm_mode[9] = { -1, -1, -1, -1, -1, -1, -1 };

	if (qlm_mode[qlm] != -1)
		return qlm_mode[qlm];

	if (qlm > 9) {
		debug("Invalid QLM(%d) passed\n", qlm);
		return -1;
	}

	if ((qlm == 2 || qlm == 3) && (OCTEON_IS_MODEL(OCTEON_CNF75XX))) {
		cvmx_sriox_status_reg_t status_reg;
		int port = (qlm == 2) ? 0 : 1;

		status_reg.u64 = csr_rd(CVMX_SRIOX_STATUS_REG(port));
		/* FIXME add different width */
		if (status_reg.s.srio)
			qlm_mode[qlm] = CVMX_QLM_MODE_SRIO_1X4;
		else
			qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
		return qlm_mode[qlm];
	}

	gserx_cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));
	if (gserx_cfg.s.pcie) {
		switch (qlm) {
		case 0: /* Either PEM0 x2 or PEM0 x4 */
		case 1: /* Either PEM1 x2 or PEM0 x4 */
		{
			/* FIXME later */
			qlm_mode[qlm] = CVMX_QLM_MODE_PCIE;
			break;
		}
		default:
			qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		}
	} else if (gserx_cfg.s.bgx) {
		cvmx_bgxx_cmrx_config_t cmr_config;
		cvmx_bgxx_spux_br_pmd_control_t pmd_control;
		int bgx = 0;
		int start = 0, end = 4, index;
		int lane_mask = 0, train_mask = 0;
		int mux = 0; // 0:BGX0 (DLM4/DLM5), 1:BGX0(DLM4), 2:BGX0(DLM5)
		cvmx_gserx_cfg_t gser1, gser2;

		gser1.u64 = csr_rd(CVMX_GSERX_CFG(4));
		gser2.u64 = csr_rd(CVMX_GSERX_CFG(5));
		if (gser1.s.bgx && gser2.s.bgx) {
			start = 0;
			end = 4;
		} else if (gser1.s.bgx) {
			start = 0;
			end = 2;
			mux = 1;
		} else if (gser2.s.bgx) {
			start = 2;
			end = 4;
			mux = 2;
		} else {
			qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			return qlm_mode[qlm];
		}

		for (index = start; index < end; index++) {
			cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(index, bgx));
			pmd_control.u64 = csr_rd(CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx));
			lane_mask |= (cmr_config.s.lmac_type << (index * 4));
			train_mask |= (pmd_control.s.train_en << (index * 4));
		}

		switch (lane_mask) {
		case 0:
			if (mux == 1 || mux == 2)
				qlm_mode[qlm] = CVMX_QLM_MODE_SGMII_2X1;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_SGMII;
			break;
		case 0x3300:
			if (mux == 0)
				qlm_mode[qlm] = CVMX_QLM_MODE_MIXED;
			else if (mux == 2)
				if (train_mask)
					qlm_mode[qlm] = CVMX_QLM_MODE_10G_KR_1X2;
				else
					qlm_mode[qlm] = CVMX_QLM_MODE_XFI_1X2;
			else
				qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		default:
			qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
			break;
		}
	} else {
		qlm_mode[qlm] = CVMX_QLM_MODE_DISABLED;
	}

	return qlm_mode[qlm];
}

/*
 * Read QLM and return mode.
 */
enum cvmx_qlm_mode cvmx_qlm_get_mode(int qlm)
{
	if (OCTEON_IS_OCTEON2())
		return __cvmx_qlm_get_mode_cn6xxx(qlm);
	else if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return __cvmx_qlm_get_mode_cn70xx(qlm);
	else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return cvmx_qlm_get_mode_cn78xx(cvmx_get_node_num(), qlm);
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return __cvmx_qlm_get_mode_cn73xx(qlm);
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return __cvmx_qlm_get_mode_cnf75xx(qlm);

	return CVMX_QLM_MODE_DISABLED;
}

int cvmx_qlm_measure_clock_cn7xxx(int node, int qlm)
{
	cvmx_gserx_cfg_t cfg;
	cvmx_gserx_refclk_sel_t refclk_sel;
	cvmx_gserx_lane_mode_t lane_mode;

	if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		if (node != 0 || qlm >= 7)
			return -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		if (qlm >= 8 || node > 1)
			return -1; /* FIXME for OCI */
	} else {
		debug("%s: Unsupported OCTEON model\n", __func__);
		return -1;
	}

	cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));

	if (cfg.s.pcie) {
		refclk_sel.u64 = csr_rd_node(node, CVMX_GSERX_REFCLK_SEL(qlm));
		if (refclk_sel.s.pcie_refclk125)
			return REF_125MHZ; /* Ref 125 Mhz */
		else
			return REF_100MHZ; /* Ref 100Mhz */
	}

	lane_mode.u64 = csr_rd_node(node, CVMX_GSERX_LANE_MODE(qlm));
	switch (lane_mode.s.lmode) {
	case R_25G_REFCLK100:
		return REF_100MHZ;
	case R_5G_REFCLK100:
		return REF_100MHZ;
	case R_8G_REFCLK100:
		return REF_100MHZ;
	case R_125G_REFCLK15625_KX:
		return REF_156MHZ;
	case R_3125G_REFCLK15625_XAUI:
		return REF_156MHZ;
	case R_103125G_REFCLK15625_KR:
		return REF_156MHZ;
	case R_125G_REFCLK15625_SGMII:
		return REF_156MHZ;
	case R_5G_REFCLK15625_QSGMII:
		return REF_156MHZ;
	case R_625G_REFCLK15625_RXAUI:
		return REF_156MHZ;
	case R_25G_REFCLK125:
		return REF_125MHZ;
	case R_5G_REFCLK125:
		return REF_125MHZ;
	case R_8G_REFCLK125:
		return REF_125MHZ;
	default:
		return 0;
	}
}

/**
 * Measure the reference clock of a QLM on a multi-node setup
 *
 * @param node   node to measure
 * @param qlm    QLM to measure
 *
 * Return: Clock rate in Hz
 */
int cvmx_qlm_measure_clock_node(int node, int qlm)
{
	if (octeon_has_feature(OCTEON_FEATURE_MULTINODE))
		return cvmx_qlm_measure_clock_cn7xxx(node, qlm);
	else
		return cvmx_qlm_measure_clock(qlm);
}

/**
 * Measure the reference clock of a QLM
 *
 * @param qlm    QLM to measure
 *
 * Return: Clock rate in Hz
 */
int cvmx_qlm_measure_clock(int qlm)
{
	cvmx_mio_ptp_clock_cfg_t ptp_clock;
	u64 count;
	u64 start_cycle, stop_cycle;
	int evcnt_offset = 0x10;
	int incr_count = 1;
	int ref_clock[16] = { 0 };

	if (ref_clock[qlm])
		return ref_clock[qlm];

	if (OCTEON_IS_OCTEON3() && !OCTEON_IS_MODEL(OCTEON_CN70XX))
		return cvmx_qlm_measure_clock_cn7xxx(cvmx_get_node_num(), qlm);

	if (OCTEON_IS_MODEL(OCTEON_CN70XX) && qlm == 0) {
		cvmx_gserx_dlmx_ref_clkdiv2_t ref_clkdiv2;

		ref_clkdiv2.u64 = csr_rd(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0));
		if (ref_clkdiv2.s.ref_clkdiv2)
			incr_count = 2;
	}

	/* Fix reference clock for OCI QLMs */

	/* Disable the PTP event counter while we configure it */
	ptp_clock.u64 = csr_rd(CVMX_MIO_PTP_CLOCK_CFG); /* For CN63XXp1 errata */
	ptp_clock.s.evcnt_en = 0;
	csr_wr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);

	/* Count on rising edge, Choose which QLM to count */
	ptp_clock.u64 = csr_rd(CVMX_MIO_PTP_CLOCK_CFG); /* For CN63XXp1 errata */
	ptp_clock.s.evcnt_edge = 0;
	ptp_clock.s.evcnt_in = evcnt_offset + qlm;
	csr_wr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);

	/* Clear MIO_PTP_EVT_CNT */
	csr_rd(CVMX_MIO_PTP_EVT_CNT); /* For CN63XXp1 errata */
	count = csr_rd(CVMX_MIO_PTP_EVT_CNT);
	csr_wr(CVMX_MIO_PTP_EVT_CNT, -count);

	/* Set MIO_PTP_EVT_CNT to 1 billion */
	csr_wr(CVMX_MIO_PTP_EVT_CNT, 1000000000);

	/* Enable the PTP event counter */
	ptp_clock.u64 = csr_rd(CVMX_MIO_PTP_CLOCK_CFG); /* For CN63XXp1 errata */
	ptp_clock.s.evcnt_en = 1;
	csr_wr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);

	start_cycle = get_ticks();
	/* Wait for 50ms */
	mdelay(50);

	/* Read the counter */
	csr_rd(CVMX_MIO_PTP_EVT_CNT); /* For CN63XXp1 errata */
	count = csr_rd(CVMX_MIO_PTP_EVT_CNT);
	stop_cycle = get_ticks();

	/* Disable the PTP event counter */
	ptp_clock.u64 = csr_rd(CVMX_MIO_PTP_CLOCK_CFG); /* For CN63XXp1 errata */
	ptp_clock.s.evcnt_en = 0;
	csr_wr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);

	/* Clock counted down, so reverse it */
	count = 1000000000 - count;
	count *= incr_count;

	/* Return the rate */
	ref_clock[qlm] = count * gd->cpu_clk / (stop_cycle - start_cycle);

	return ref_clock[qlm];
}

/*
 * Perform RX equalization on a QLM
 *
 * @param node	Node the QLM is on
 * @param qlm	QLM to perform RX equalization on
 * @param lane	Lane to use, or -1 for all lanes
 *
 * Return: Zero on success, negative if any lane failed RX equalization
 */
int __cvmx_qlm_rx_equalization(int node, int qlm, int lane)
{
	cvmx_gserx_phy_ctl_t phy_ctl;
	cvmx_gserx_br_rxx_ctl_t rxx_ctl;
	cvmx_gserx_br_rxx_eer_t rxx_eer;
	cvmx_gserx_rx_eie_detsts_t eie_detsts;
	int fail, gbaud, l, lane_mask;
	enum cvmx_qlm_mode mode;
	int max_lanes = cvmx_qlm_get_lanes(qlm);
	cvmx_gserx_lane_mode_t lmode;
	cvmx_gserx_lane_px_mode_1_t pmode_1;
	int pending = 0;
	u64 timeout;

	/* Don't touch QLMs if it is reset or powered down */
	phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));
	if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
		return -1;

	/*
	 * Check whether GSER PRBS pattern matcher is enabled on any of the
	 * applicable lanes. Can't complete RX Equalization while pattern
	 * matcher is enabled because it causes errors
	 */
	for (l = 0; l < max_lanes; l++) {
		cvmx_gserx_lanex_lbert_cfg_t lbert_cfg;

		if (lane != -1 && lane != l)
			continue;

		lbert_cfg.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_LBERT_CFG(l, qlm));
		if (lbert_cfg.s.lbert_pm_en == 1)
			return -1;
	}

	/* Get Lane Mode */
	lmode.u64 = csr_rd_node(node, CVMX_GSERX_LANE_MODE(qlm));

	/*
	 * Check to see if in VMA manual mode is set. If in VMA manual mode
	 * don't complete rx equalization
	 */
	pmode_1.u64 = csr_rd_node(node, CVMX_GSERX_LANE_PX_MODE_1(lmode.s.lmode, qlm));
	if (pmode_1.s.vma_mm == 1) {
#ifdef DEBUG_QLM
		debug("N%d:QLM%d: VMA Manual (manual DFE) selected. Not completing Rx equalization\n",
		      node, qlm);
#endif
		return 0;
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		gbaud = cvmx_qlm_get_gbaud_mhz_node(node, qlm);
		mode = cvmx_qlm_get_mode_cn78xx(node, qlm);
	} else {
		gbaud = cvmx_qlm_get_gbaud_mhz(qlm);
		mode = cvmx_qlm_get_mode(qlm);
	}

	/* Apply RX Equalization for speed >= 8G */
	if (qlm < 8) {
		if (gbaud < 6250)
			return 0;
	}

	/* Don't run on PCIe Links */
	if (mode == CVMX_QLM_MODE_PCIE || mode == CVMX_QLM_MODE_PCIE_1X8 ||
	    mode == CVMX_QLM_MODE_PCIE_1X2 || mode == CVMX_QLM_MODE_PCIE_2X1)
		return -1;

	fail = 0;

	/*
	 * Before completing Rx equalization wait for
	 * GSERx_RX_EIE_DETSTS[CDRLOCK] to be set.
	 * This ensures the rx data is valid
	 */
	if (lane == -1) {
		/*
		 * check all 4 Lanes (cdrlock = 1111/b) for CDR Lock with
		 * lane == -1
		 */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_GSERX_RX_EIE_DETSTS(qlm),
					       cvmx_gserx_rx_eie_detsts_t, cdrlock, ==,
					       (1 << max_lanes) - 1, 500)) {
#ifdef DEBUG_QLM
			eie_detsts.u64 = csr_rd_node(node, CVMX_GSERX_RX_EIE_DETSTS(qlm));
			debug("ERROR: %d:QLM%d: CDR Lock not detected for all 4 lanes. CDR_LOCK(0x%x)\n",
			      node, qlm, eie_detsts.s.cdrlock);
#endif
			return -1;
		}
	} else {
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_GSERX_RX_EIE_DETSTS(qlm),
					       cvmx_gserx_rx_eie_detsts_t, cdrlock, &, (1 << lane),
					       500)) {
#ifdef DEBUG_QLM
			eie_detsts.u64 = csr_rd_node(node, CVMX_GSERX_RX_EIE_DETSTS(qlm));
			debug("ERROR: %d:QLM%d: CDR Lock not detected for Lane%d CDR_LOCK(0x%x)\n",
			      node, qlm, lane, eie_detsts.s.cdrlock);
#endif
			return -1;
		}
	}

	/*
	 * Errata (GSER-20075) GSER(0..13)_BR_RX3_EER[RXT_ERR] is
	 * GSER(0..13)_BR_RX2_EER[RXT_ERR]. Since lanes 2-3 trigger at the
	 * same time, we need to setup lane 3 before we loop through the lanes
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (lane == -1 || lane == 3)) {
		/* Enable software control */
		rxx_ctl.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_CTL(3, qlm));
		rxx_ctl.s.rxt_swm = 1;
		csr_wr_node(node, CVMX_GSERX_BR_RXX_CTL(3, qlm), rxx_ctl.u64);

		/* Clear the completion flag */
		rxx_eer.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_EER(3, qlm));
		rxx_eer.s.rxt_esv = 0;
		csr_wr_node(node, CVMX_GSERX_BR_RXX_EER(3, qlm), rxx_eer.u64);
		/* Initiate a new request on lane 2 */
		if (lane == 3) {
			rxx_eer.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_EER(2, qlm));
			rxx_eer.s.rxt_eer = 1;
			csr_wr_node(node, CVMX_GSERX_BR_RXX_EER(2, qlm), rxx_eer.u64);
		}
	}

	for (l = 0; l < max_lanes; l++) {
		if (lane != -1 && lane != l)
			continue;

		/*
		 * Skip lane 3 on 78p1.x due to Errata (GSER-20075).
		 * Handled above
		 */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && l == 3) {
			/*
			 * Need to add lane 3 to pending list for 78xx
			 * pass 1.x
			 */
			pending |= 1 << 3;
			continue;
		}
		/* Enable software control */
		rxx_ctl.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_CTL(l, qlm));
		rxx_ctl.s.rxt_swm = 1;
		csr_wr_node(node, CVMX_GSERX_BR_RXX_CTL(l, qlm), rxx_ctl.u64);

		/* Clear the completion flag and initiate a new request */
		rxx_eer.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_EER(l, qlm));
		rxx_eer.s.rxt_esv = 0;
		rxx_eer.s.rxt_eer = 1;
		csr_wr_node(node, CVMX_GSERX_BR_RXX_EER(l, qlm), rxx_eer.u64);
		pending |= 1 << l;
	}

	/*
	 * Wait for 250ms, approx 10x times measured value, as XFI/XLAUI
	 * can take 21-23ms, other interfaces can take 2-3ms.
	 */
	timeout = get_timer(0);

	lane_mask = 0;
	while (pending) {
		/* Wait for RX equalization to complete */
		for (l = 0; l < max_lanes; l++) {
			lane_mask = 1 << l;
			/* Only check lanes that are pending */
			if (!(pending & lane_mask))
				continue;

			/*
			 * Read the registers for checking Electrical Idle/CDR
			 * lock and the status of the RX equalization
			 */
			eie_detsts.u64 = csr_rd_node(node, CVMX_GSERX_RX_EIE_DETSTS(qlm));
			rxx_eer.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_EER(l, qlm));

			/*
			 * Mark failure if lane entered Electrical Idle or lost
			 * CDR Lock. The bit for the lane will have cleared in
			 * either EIESTS or CDRLOCK
			 */
			if (!(eie_detsts.s.eiests & eie_detsts.s.cdrlock & lane_mask)) {
				fail |= lane_mask;
				pending &= ~lane_mask;
			} else if (rxx_eer.s.rxt_esv) {
				pending &= ~lane_mask;
			}
		}

		/* Breakout of the loop on timeout */
		if (get_timer(timeout) > 250)
			break;
	}

	lane_mask = 0;
	/* Cleanup and report status */
	for (l = 0; l < max_lanes; l++) {
		if (lane != -1 && lane != l)
			continue;

		lane_mask = 1 << l;
		rxx_eer.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_EER(l, qlm));
		/* Switch back to hardware control */
		rxx_ctl.u64 = csr_rd_node(node, CVMX_GSERX_BR_RXX_CTL(l, qlm));
		rxx_ctl.s.rxt_swm = 0;
		csr_wr_node(node, CVMX_GSERX_BR_RXX_CTL(l, qlm), rxx_ctl.u64);

		/* Report status */
		if (fail & lane_mask) {
#ifdef DEBUG_QLM
			debug("%d:QLM%d: Lane%d RX equalization lost CDR Lock or entered Electrical Idle\n",
			      node, qlm, l);
#endif
		} else if ((pending & lane_mask) || !rxx_eer.s.rxt_esv) {
#ifdef DEBUG_QLM
			debug("%d:QLM%d: Lane %d RX equalization timeout\n", node, qlm, l);
#endif
			fail |= 1 << l;
		} else {
#ifdef DEBUG_QLM
			char *dir_label[4] = { "Hold", "Inc", "Dec", "Hold" };
#ifdef DEBUG_QLM_RX
			cvmx_gserx_lanex_rx_aeq_out_0_t rx_aeq_out_0;
			cvmx_gserx_lanex_rx_aeq_out_1_t rx_aeq_out_1;
			cvmx_gserx_lanex_rx_aeq_out_2_t rx_aeq_out_2;
			cvmx_gserx_lanex_rx_vma_status_0_t rx_vma_status_0;
#endif
			debug("%d:QLM%d: Lane%d: RX equalization completed.\n", node, qlm, l);
			debug("    Tx Direction Hints TXPRE: %s, TXMAIN: %s, TXPOST: %s, Figure of Merit: %d\n",
			      dir_label[(rxx_eer.s.rxt_esm) & 0x3],
			      dir_label[((rxx_eer.s.rxt_esm) >> 2) & 0x3],
			      dir_label[((rxx_eer.s.rxt_esm) >> 4) & 0x3], rxx_eer.s.rxt_esm >> 6);

#ifdef DEBUG_QLM_RX
			rx_aeq_out_0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_AEQ_OUT_0(l, qlm));
			rx_aeq_out_1.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_AEQ_OUT_1(l, qlm));
			rx_aeq_out_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_AEQ_OUT_2(l, qlm));
			rx_vma_status_0.u64 =
				csr_rd_node(node, CVMX_GSERX_LANEX_RX_VMA_STATUS_0(l, qlm));
			debug("    DFE Tap1:%lu, Tap2:%ld, Tap3:%ld, Tap4:%ld, Tap5:%ld\n",
			      (unsigned int long)cvmx_bit_extract(rx_aeq_out_1.u64, 0, 5),
			      (unsigned int long)cvmx_bit_extract_smag(rx_aeq_out_1.u64, 5, 9),
			      (unsigned int long)cvmx_bit_extract_smag(rx_aeq_out_1.u64, 10, 14),
			      (unsigned int long)cvmx_bit_extract_smag(rx_aeq_out_0.u64, 0, 4),
			      (unsigned int long)cvmx_bit_extract_smag(rx_aeq_out_0.u64, 5, 9));
			debug("    Pre-CTLE Gain:%lu, Post-CTLE Gain:%lu, CTLE Peak:%lu, CTLE Pole:%lu\n",
			      (unsigned int long)cvmx_bit_extract(rx_aeq_out_2.u64, 4, 4),
			      (unsigned int long)cvmx_bit_extract(rx_aeq_out_2.u64, 0, 4),
			      (unsigned int long)cvmx_bit_extract(rx_vma_status_0.u64, 2, 4),
			      (unsigned int long)cvmx_bit_extract(rx_vma_status_0.u64, 0, 2));
#endif
#endif
		}
	}

	return (fail) ? -1 : 0;
}

/**
 * Errata GSER-27882 -GSER 10GBASE-KR Transmit Equalizer
 * Training may not update PHY Tx Taps. This function is not static
 * so we can share it with BGX KR
 *
 * @param node	Node to apply errata workaround
 * @param qlm	QLM to apply errata workaround
 * @param lane	Lane to apply the errata
 */
int cvmx_qlm_gser_errata_27882(int node, int qlm, int lane)
{
	cvmx_gserx_lanex_pcs_ctlifc_0_t clifc0;
	cvmx_gserx_lanex_pcs_ctlifc_2_t clifc2;

	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_0) || OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_1) ||
	      OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_2) || OCTEON_IS_MODEL(OCTEON_CNF75XX_PASS1_0) ||
	      OCTEON_IS_MODEL(OCTEON_CN78XX)))
		return 0;

	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_GSERX_RX_EIE_DETSTS(qlm),
				       cvmx_gserx_rx_eie_detsts_t, cdrlock, &,
				       (1 << lane), 200))
		return -1;

	clifc0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(lane, qlm));
	clifc0.s.cfg_tx_coeff_req_ovrrd_val = 1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(lane, qlm), clifc0.u64);
	clifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	clifc2.s.cfg_tx_coeff_req_ovrrd_en = 1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), clifc2.u64);
	clifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	clifc2.s.ctlifc_ovrrd_req = 1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), clifc2.u64);
	clifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	clifc2.s.cfg_tx_coeff_req_ovrrd_en = 0;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), clifc2.u64);
	clifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	clifc2.s.ctlifc_ovrrd_req = 1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), clifc2.u64);
	return 0;
}

/**
 * Updates the RX EQ Default Settings Update (CTLE Bias) to support longer
 * SERDES channels
 *
 * @INTERNAL
 *
 * @param node	Node number to configure
 * @param qlm	QLM number to configure
 */
void cvmx_qlm_gser_errata_25992(int node, int qlm)
{
	int lane;
	int num_lanes = cvmx_qlm_get_lanes(qlm);

	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_0) || OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_1) ||
	      OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_2) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		return;

	for (lane = 0; lane < num_lanes; lane++) {
		cvmx_gserx_lanex_rx_ctle_ctrl_t rx_ctle_ctrl;
		cvmx_gserx_lanex_rx_cfg_4_t rx_cfg_4;

		rx_ctle_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_CTLE_CTRL(lane, qlm));
		rx_ctle_ctrl.s.pcs_sds_rx_ctle_bias_ctrl = 3;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_CTLE_CTRL(lane, qlm), rx_ctle_ctrl.u64);

		rx_cfg_4.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_CFG_4(lane, qlm));
		rx_cfg_4.s.cfg_rx_errdet_ctrl = 0xcd6f;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_CFG_4(lane, qlm), rx_cfg_4.u64);
	}
}

void cvmx_qlm_display_registers(int qlm)
{
	int num_lanes = cvmx_qlm_get_lanes(qlm);
	int lane;
	const __cvmx_qlm_jtag_field_t *ptr = cvmx_qlm_jtag_get_field();

	debug("%29s", "Field[<stop bit>:<start bit>]");
	for (lane = 0; lane < num_lanes; lane++)
		debug("\t      Lane %d", lane);
	debug("\n");

	while (ptr && ptr->name) {
		debug("%20s[%3d:%3d]", ptr->name, ptr->stop_bit, ptr->start_bit);
		for (lane = 0; lane < num_lanes; lane++) {
			u64 val;
			int tx_byp = 0;

			/*
			 * Make sure serdes_tx_byp is set for displaying
			 * TX amplitude and TX demphasis field values.
			 */
			if (strncmp(ptr->name, "biasdrv_", 8) == 0 ||
			    strncmp(ptr->name, "tcoeff_", 7) == 0) {
				tx_byp = cvmx_qlm_jtag_get(qlm, lane, "serdes_tx_byp");
				if (tx_byp == 0) {
					debug("\t \t");
					continue;
				}
			}
			val = cvmx_qlm_jtag_get(qlm, lane, ptr->name);
			debug("\t%4llu (0x%04llx)", (unsigned long long)val,
			      (unsigned long long)val);
		}
		debug("\n");
		ptr++;
	}
}

/* ToDo: CVMX_DUMP_GSER removed for now (unused!) */
