// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <dm.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>

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

DECLARE_GLOBAL_DATA_PTR;

/** 2.5GHz with 100MHz reference clock */
#define R_2_5G_REFCLK100 0x0
/** 5.0GHz with 100MHz reference clock */
#define R_5G_REFCLK100 0x1
/** 8.0GHz with 100MHz reference clock */
#define R_8G_REFCLK100 0x2
/** 1.25GHz with 156.25MHz reference clock */
#define R_125G_REFCLK15625_KX 0x3
/** 3.125Ghz with 156.25MHz reference clock (XAUI) */
#define R_3125G_REFCLK15625_XAUI 0x4
/** 10.3125GHz with 156.25MHz reference clock (XFI/XLAUI) */
#define R_103125G_REFCLK15625_KR 0x5
/** 1.25GHz with 156.25MHz reference clock (SGMII) */
#define R_125G_REFCLK15625_SGMII 0x6
/** 5GHz with 156.25MHz reference clock (QSGMII) */
#define R_5G_REFCLK15625_QSGMII 0x7
/** 6.25GHz with 156.25MHz reference clock (RXAUI/25G) */
#define R_625G_REFCLK15625_RXAUI 0x8
/** 2.5GHz with 125MHz reference clock */
#define R_2_5G_REFCLK125 0x9
/** 5GHz with 125MHz reference clock */
#define R_5G_REFCLK125 0xa
/** 8GHz with 125MHz reference clock */
#define R_8G_REFCLK125 0xb
/** Must be last, number of modes */
#define R_NUM_LANE_MODES 0xc

int cvmx_qlm_is_ref_clock(int qlm, int reference_mhz)
{
	int ref_clock = cvmx_qlm_measure_clock(qlm);
	int mhz = ref_clock / 1000000;
	int range = reference_mhz / 10;

	return ((mhz >= reference_mhz - range) && (mhz <= reference_mhz + range));
}

static int __get_qlm_spd(int qlm, int speed)
{
	int qlm_spd = 0xf;

	if (cvmx_qlm_is_ref_clock(qlm, 100)) {
		if (speed == 1250)
			qlm_spd = 0x3;
		else if (speed == 2500)
			qlm_spd = 0x2;
		else if (speed == 5000)
			qlm_spd = 0x0;
		else
			qlm_spd = 0xf;
	} else if (cvmx_qlm_is_ref_clock(qlm, 125)) {
		if (speed == 1250)
			qlm_spd = 0xa;
		else if (speed == 2500)
			qlm_spd = 0x9;
		else if (speed == 3125)
			qlm_spd = 0x8;
		else if (speed == 5000)
			qlm_spd = 0x6;
		else if (speed == 6250)
			qlm_spd = 0x5;
		else
			qlm_spd = 0xf;
	} else if (cvmx_qlm_is_ref_clock(qlm, 156)) {
		if (speed == 1250)
			qlm_spd = 0x4;
		else if (speed == 2500)
			qlm_spd = 0x7;
		else if (speed == 3125)
			qlm_spd = 0xe;
		else if (speed == 3750)
			qlm_spd = 0xd;
		else if (speed == 5000)
			qlm_spd = 0xb;
		else if (speed == 6250)
			qlm_spd = 0xc;
		else
			qlm_spd = 0xf;
	} else if (cvmx_qlm_is_ref_clock(qlm, 161)) {
		if (speed == 6316)
			qlm_spd = 0xc;
	}
	return qlm_spd;
}

static void __set_qlm_pcie_mode_61xx(int pcie_port, int root_complex)
{
	int rc = root_complex ? 1 : 0;
	int ep = root_complex ? 0 : 1;
	cvmx_ciu_soft_prst1_t soft_prst1;
	cvmx_ciu_soft_prst_t soft_prst;
	cvmx_mio_rst_ctlx_t rst_ctl;

	if (pcie_port) {
		soft_prst1.u64 = csr_rd(CVMX_CIU_SOFT_PRST1);
		soft_prst1.s.soft_prst = 1;
		csr_wr(CVMX_CIU_SOFT_PRST1, soft_prst1.u64);
	} else {
		soft_prst.u64 = csr_rd(CVMX_CIU_SOFT_PRST);
		soft_prst.s.soft_prst = 1;
		csr_wr(CVMX_CIU_SOFT_PRST, soft_prst.u64);
	}

	rst_ctl.u64 = csr_rd(CVMX_MIO_RST_CTLX(pcie_port));

	rst_ctl.s.prst_link = rc;
	rst_ctl.s.rst_link = ep;
	rst_ctl.s.prtmode = rc;
	rst_ctl.s.rst_drv = rc;
	rst_ctl.s.rst_rcv = 0;
	rst_ctl.s.rst_chip = ep;
	csr_wr(CVMX_MIO_RST_CTLX(pcie_port), rst_ctl.u64);

	if (root_complex == 0) {
		if (pcie_port) {
			soft_prst1.u64 = csr_rd(CVMX_CIU_SOFT_PRST1);
			soft_prst1.s.soft_prst = 0;
			csr_wr(CVMX_CIU_SOFT_PRST1, soft_prst1.u64);
		} else {
			soft_prst.u64 = csr_rd(CVMX_CIU_SOFT_PRST);
			soft_prst.s.soft_prst = 0;
			csr_wr(CVMX_CIU_SOFT_PRST, soft_prst.u64);
		}
	}
}

/**
 * Configure qlm speed and mode. MIO_QLMX_CFG[speed,mode] are not set
 * for CN61XX.
 *
 * @param qlm     The QLM to configure
 * @param speed   The speed the QLM needs to be configured in Mhz.
 * @param mode    The QLM to be configured as SGMII/XAUI/PCIe.
 *                  QLM 0: 0 = PCIe0 1X4, 1 = Reserved, 2 = SGMII1, 3 = XAUI1
 *                  QLM 1: 0 = PCIe1 1x2, 1 = PCIe(0/1) 2x1, 2 - 3 = Reserved
 *                  QLM 2: 0 - 1 = Reserved, 2 = SGMII0, 3 = XAUI0
 * @param rc      Only used for PCIe, rc = 1 for root complex mode, 0 for EP
 *		  mode.
 * @param pcie2x1 Only used when QLM1 is in PCIE2x1 mode.  The QLM_SPD has a
 *		  different value on how PEMx needs to be configured:
 *                   0x0 - both PEM0 & PEM1 are in gen1 mode.
 *                   0x1 - PEM0 in gen2 and PEM1 in gen1 mode.
 *                   0x2 - PEM0 in gen1 and PEM1 in gen2 mode.
 *                   0x3 - both PEM0 & PEM1 are in gen2 mode.
 *               SPEED value is ignored in this mode. QLM_SPD is set based on
 *               pcie2x1 value in this mode.
 *
 * Return:       Return 0 on success or -1.
 */
static int octeon_configure_qlm_cn61xx(int qlm, int speed, int mode, int rc, int pcie2x1)
{
	cvmx_mio_qlmx_cfg_t qlm_cfg;

	/* The QLM speed varies for SGMII/XAUI and PCIe mode. And depends on
	 * reference clock.
	 */
	if (!OCTEON_IS_MODEL(OCTEON_CN61XX))
		return -1;

	if (qlm < 3) {
		qlm_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));
	} else {
		debug("WARNING: Invalid QLM(%d) passed\n", qlm);
		return -1;
	}

	switch (qlm) {
		/* SGMII/XAUI mode */
	case 2: {
		if (mode < 2) {
			qlm_cfg.s.qlm_spd = 0xf;
			break;
		}
		qlm_cfg.s.qlm_spd = __get_qlm_spd(qlm, speed);
		qlm_cfg.s.qlm_cfg = mode;
		break;
	}
	case 1: {
		if (mode == 1) { /* 2x1 mode */
			cvmx_mio_qlmx_cfg_t qlm0;

			/* When QLM0 is configured as PCIe(QLM_CFG=0x0)
			 * and enabled (QLM_SPD != 0xf), QLM1 cannot be
			 * configured as PCIe 2x1 mode (QLM_CFG=0x1)
			 * and enabled (QLM_SPD != 0xf).
			 */
			qlm0.u64 = csr_rd(CVMX_MIO_QLMX_CFG(0));
			if (qlm0.s.qlm_spd != 0xf && qlm0.s.qlm_cfg == 0) {
				debug("Invalid mode(%d) for QLM(%d) as QLM1 is PCIe mode\n",
				      mode, qlm);
				qlm_cfg.s.qlm_spd = 0xf;
				break;
			}

			/* Set QLM_SPD based on reference clock and mode */
			if (cvmx_qlm_is_ref_clock(qlm, 100)) {
				if (pcie2x1 == 0x3)
					qlm_cfg.s.qlm_spd = 0x0;
				else if (pcie2x1 == 0x1)
					qlm_cfg.s.qlm_spd = 0x2;
				else if (pcie2x1 == 0x2)
					qlm_cfg.s.qlm_spd = 0x1;
				else if (pcie2x1 == 0x0)
					qlm_cfg.s.qlm_spd = 0x3;
				else
					qlm_cfg.s.qlm_spd = 0xf;
			} else if (cvmx_qlm_is_ref_clock(qlm, 125)) {
				if (pcie2x1 == 0x3)
					qlm_cfg.s.qlm_spd = 0x4;
				else if (pcie2x1 == 0x1)
					qlm_cfg.s.qlm_spd = 0x6;
				else if (pcie2x1 == 0x2)
					qlm_cfg.s.qlm_spd = 0x9;
				else if (pcie2x1 == 0x0)
					qlm_cfg.s.qlm_spd = 0x7;
				else
					qlm_cfg.s.qlm_spd = 0xf;
			}
			qlm_cfg.s.qlm_cfg = mode;
			csr_wr(CVMX_MIO_QLMX_CFG(qlm), qlm_cfg.u64);

			/* Set PCIe mode bits */
			__set_qlm_pcie_mode_61xx(0, rc);
			__set_qlm_pcie_mode_61xx(1, rc);
			return 0;
		} else if (mode > 1) {
			debug("Invalid mode(%d) for QLM(%d).\n", mode, qlm);
			qlm_cfg.s.qlm_spd = 0xf;
			break;
		}

		/* Set speed and mode for PCIe 1x2 mode. */
		if (cvmx_qlm_is_ref_clock(qlm, 100)) {
			if (speed == 5000)
				qlm_cfg.s.qlm_spd = 0x1;
			else if (speed == 2500)
				qlm_cfg.s.qlm_spd = 0x2;
			else
				qlm_cfg.s.qlm_spd = 0xf;
		} else if (cvmx_qlm_is_ref_clock(qlm, 125)) {
			if (speed == 5000)
				qlm_cfg.s.qlm_spd = 0x4;
			else if (speed == 2500)
				qlm_cfg.s.qlm_spd = 0x6;
			else
				qlm_cfg.s.qlm_spd = 0xf;
		} else {
			qlm_cfg.s.qlm_spd = 0xf;
		}

		qlm_cfg.s.qlm_cfg = mode;
		csr_wr(CVMX_MIO_QLMX_CFG(qlm), qlm_cfg.u64);

		/* Set PCIe mode bits */
		__set_qlm_pcie_mode_61xx(1, rc);
		return 0;
	}
	case 0: {
		/* QLM_CFG = 0x1 - Reserved */
		if (mode == 1) {
			qlm_cfg.s.qlm_spd = 0xf;
			break;
		}
		/* QLM_CFG = 0x0 - PCIe 1x4(PEM0) */
		if (mode == 0 && speed != 5000 && speed != 2500) {
			qlm_cfg.s.qlm_spd = 0xf;
			break;
		}

		/* Set speed and mode */
		qlm_cfg.s.qlm_spd = __get_qlm_spd(qlm, speed);
		qlm_cfg.s.qlm_cfg = mode;
		csr_wr(CVMX_MIO_QLMX_CFG(qlm), qlm_cfg.u64);

		/* Set PCIe mode bits */
		if (mode == 0)
			__set_qlm_pcie_mode_61xx(0, rc);

		return 0;
	}
	default:
		debug("WARNING: Invalid QLM(%d) passed\n", qlm);
		qlm_cfg.s.qlm_spd = 0xf;
	}
	csr_wr(CVMX_MIO_QLMX_CFG(qlm), qlm_cfg.u64);
	return 0;
}

/* qlm      : DLM to configure
 * baud_mhz : speed of the DLM
 * ref_clk_sel  :  reference clock speed selection where:
 *			0:	100MHz
 *			1:	125MHz
 *			2:	156.25MHz
 *
 * ref_clk_input:  reference clock input where:
 *			0:	DLMC_REF_CLK0_[P,N]
 *			1:	DLMC_REF_CLK1_[P,N]
 *			2:	DLM0_REF_CLK_[P,N] (only valid for QLM 0)
 * is_sff7000_rxaui : boolean to indicate whether qlm is RXAUI on SFF7000
 */
static int __dlm_setup_pll_cn70xx(int qlm, int baud_mhz, int ref_clk_sel, int ref_clk_input,
				  int is_sff7000_rxaui)
{
	cvmx_gserx_dlmx_test_powerdown_t dlmx_test_powerdown;
	cvmx_gserx_dlmx_ref_ssp_en_t dlmx_ref_ssp_en;
	cvmx_gserx_dlmx_mpll_en_t dlmx_mpll_en;
	cvmx_gserx_dlmx_phy_reset_t dlmx_phy_reset;
	cvmx_gserx_dlmx_tx_amplitude_t tx_amplitude;
	cvmx_gserx_dlmx_tx_preemph_t tx_preemph;
	cvmx_gserx_dlmx_rx_eq_t rx_eq;
	cvmx_gserx_dlmx_ref_clkdiv2_t ref_clkdiv2;
	cvmx_gserx_dlmx_mpll_multiplier_t mpll_multiplier;
	int gmx_ref_clk = 100;

	debug("%s(%d, %d, %d, %d, %d)\n", __func__, qlm, baud_mhz, ref_clk_sel, ref_clk_input,
	      is_sff7000_rxaui);
	if (ref_clk_sel == 1)
		gmx_ref_clk = 125;
	else if (ref_clk_sel == 2)
		gmx_ref_clk = 156;

	if (qlm != 0 && ref_clk_input == 2) {
		printf("%s: Error: can only use reference clock inputs 0 or 1 for DLM %d\n",
		       __func__, qlm);
		return -1;
	}

	/* Hardware defaults are invalid */
	tx_amplitude.u64 = csr_rd(CVMX_GSERX_DLMX_TX_AMPLITUDE(qlm, 0));
	if (is_sff7000_rxaui) {
		tx_amplitude.s.tx0_amplitude = 100;
		tx_amplitude.s.tx1_amplitude = 100;
	} else {
		tx_amplitude.s.tx0_amplitude = 65;
		tx_amplitude.s.tx1_amplitude = 65;
	}

	csr_wr(CVMX_GSERX_DLMX_TX_AMPLITUDE(qlm, 0), tx_amplitude.u64);

	tx_preemph.u64 = csr_rd(CVMX_GSERX_DLMX_TX_PREEMPH(qlm, 0));

	if (is_sff7000_rxaui) {
		tx_preemph.s.tx0_preemph = 0;
		tx_preemph.s.tx1_preemph = 0;
	} else {
		tx_preemph.s.tx0_preemph = 22;
		tx_preemph.s.tx1_preemph = 22;
	}
	csr_wr(CVMX_GSERX_DLMX_TX_PREEMPH(qlm, 0), tx_preemph.u64);

	rx_eq.u64 = csr_rd(CVMX_GSERX_DLMX_RX_EQ(qlm, 0));
	rx_eq.s.rx0_eq = 0;
	rx_eq.s.rx1_eq = 0;
	csr_wr(CVMX_GSERX_DLMX_RX_EQ(qlm, 0), rx_eq.u64);

	/* 1. Write GSER0_DLM0_REF_USE_PAD[REF_USE_PAD] = 1 (to select
	 *    reference-clock input)
	 *    The documentation for this register in the HRM is useless since
	 *    it says it selects between two different clocks that are not
	 *    documented anywhere.  What it really does is select between
	 *    DLM0_REF_CLK_[P,N] if 1 and DLMC_REF_CLK[0,1]_[P,N] if 0.
	 *
	 *    This register must be 0 for DLMs 1 and 2 and can only be 1 for
	 *    DLM 0.
	 */
	csr_wr(CVMX_GSERX_DLMX_REF_USE_PAD(0, 0), ((ref_clk_input == 2) && (qlm == 0)) ? 1 : 0);

	/* Reference clock was already chosen before we got here */

	/* 2. Write GSER0_DLM0_REFCLK_SEL[REFCLK_SEL] if required for
	 *    reference-clock selection.
	 *
	 *    If GSERX_DLMX_REF_USE_PAD is 1 then this register is ignored.
	 */
	csr_wr(CVMX_GSERX_DLMX_REFCLK_SEL(0, 0), ref_clk_input & 1);

	/* Reference clock was already chosen before we got here */

	/* 3. If required, write GSER0_DLM0_REF_CLKDIV2[REF_CLKDIV2] (must be
	 *    set if reference clock > 100 MHz)
	 */
	/* Apply workaround for Errata (G-20669) MPLL may not come up. */
	ref_clkdiv2.u64 = csr_rd(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0));
	if (gmx_ref_clk == 100)
		ref_clkdiv2.s.ref_clkdiv2 = 0;
	else
		ref_clkdiv2.s.ref_clkdiv2 = 1;
	csr_wr(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0), ref_clkdiv2.u64);

	/* 1. Ensure GSER(0)_DLM(0..2)_PHY_RESET[PHY_RESET] is set. */
	dlmx_phy_reset.u64 = csr_rd(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0));
	dlmx_phy_reset.s.phy_reset = 1;
	csr_wr(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0), dlmx_phy_reset.u64);

	/* 2. If SGMII or QSGMII or RXAUI (i.e. if DLM0) set
	 *    GSER(0)_DLM(0)_MPLL_EN[MPLL_EN] to one.
	 */
	/* 7. Set GSER0_DLM0_MPLL_EN[MPLL_EN] = 1 */
	dlmx_mpll_en.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_EN(0, 0));
	dlmx_mpll_en.s.mpll_en = 1;
	csr_wr(CVMX_GSERX_DLMX_MPLL_EN(0, 0), dlmx_mpll_en.u64);

	/* 3. Set GSER(0)_DLM(0..2)_MPLL_MULTIPLIER[MPLL_MULTIPLIER]
	 *    to the value in the preceding table, which is different
	 *    than the desired setting prescribed by the HRM.
	 */
	mpll_multiplier.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
	if (gmx_ref_clk == 100)
		mpll_multiplier.s.mpll_multiplier = 35;
	else if (gmx_ref_clk == 125)
		mpll_multiplier.s.mpll_multiplier = 56;
	else
		mpll_multiplier.s.mpll_multiplier = 45;
	debug("%s: Setting mpll multiplier to %u for DLM%d, baud %d, clock rate %uMHz\n",
	      __func__, mpll_multiplier.s.mpll_multiplier, qlm, baud_mhz, gmx_ref_clk);

	csr_wr(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0), mpll_multiplier.u64);

	/* 5. Clear GSER0_DLM0_TEST_POWERDOWN[TEST_POWERDOWN] */
	dlmx_test_powerdown.u64 = csr_rd(CVMX_GSERX_DLMX_TEST_POWERDOWN(qlm, 0));
	dlmx_test_powerdown.s.test_powerdown = 0;
	csr_wr(CVMX_GSERX_DLMX_TEST_POWERDOWN(qlm, 0), dlmx_test_powerdown.u64);

	/* 6. Set GSER0_DLM0_REF_SSP_EN[REF_SSP_EN] = 1 */
	dlmx_ref_ssp_en.u64 = csr_rd(CVMX_GSERX_DLMX_REF_SSP_EN(qlm, 0));
	dlmx_ref_ssp_en.s.ref_ssp_en = 1;
	csr_wr(CVMX_GSERX_DLMX_REF_SSP_EN(0, 0), dlmx_ref_ssp_en.u64);

	/* 8. Clear GSER0_DLM0_PHY_RESET[PHY_RESET] = 0 */
	dlmx_phy_reset.u64 = csr_rd(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0));
	dlmx_phy_reset.s.phy_reset = 0;
	csr_wr(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0), dlmx_phy_reset.u64);

	/* 5. If PCIe or SATA (i.e. if DLM1 or DLM2), set both MPLL_EN
	 * and MPLL_EN_OVRD to one in GSER(0)_PHY(1..2)_OVRD_IN_LO.
	 */

	/* 6. Decrease MPLL_MULTIPLIER by one continually until it
	 * reaches the desired long-term setting, ensuring that each
	 * MPLL_MULTIPLIER value is constant for at least 1 msec before
	 * changing to the next value.  The desired long-term setting is
	 * as indicated in HRM tables 21-1, 21-2, and 21-3.  This is not
	 * required with the HRM sequence.
	 */
	mpll_multiplier.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
	__cvmx_qlm_set_mult(qlm, baud_mhz, mpll_multiplier.s.mpll_multiplier);

	/* 9. Poll until the MPLL locks. Wait for
	 *    GSER0_DLM0_MPLL_STATUS[MPLL_STATUS] = 1
	 */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_MPLL_STATUS(qlm, 0),
				  cvmx_gserx_dlmx_mpll_status_t, mpll_status, ==, 1, 10000)) {
		printf("PLL for DLM%d failed to lock\n", qlm);
		return -1;
	}
	return 0;
}

static int __dlm0_setup_tx_cn70xx(int speed, int ref_clk_sel)
{
	int need0, need1;
	cvmx_gmxx_inf_mode_t mode0, mode1;
	cvmx_gserx_dlmx_tx_rate_t rate;
	cvmx_gserx_dlmx_tx_en_t en;
	cvmx_gserx_dlmx_tx_cm_en_t cm_en;
	cvmx_gserx_dlmx_tx_data_en_t data_en;
	cvmx_gserx_dlmx_tx_reset_t tx_reset;

	debug("%s(%d, %d)\n", __func__, speed, ref_clk_sel);
	mode0.u64 = csr_rd(CVMX_GMXX_INF_MODE(0));
	mode1.u64 = csr_rd(CVMX_GMXX_INF_MODE(1));

	/* Which lanes do we need? */
	need0 = (mode0.s.mode != CVMX_GMX_INF_MODE_DISABLED);
	need1 = (mode1.s.mode != CVMX_GMX_INF_MODE_DISABLED) ||
		(mode0.s.mode == CVMX_GMX_INF_MODE_RXAUI);

	/* 1. Write GSER0_DLM0_TX_RATE[TXn_RATE] (Set according to required
	 *    data rate (see Table 21-1).
	 */
	rate.u64 = csr_rd(CVMX_GSERX_DLMX_TX_RATE(0, 0));
	debug("%s: speed: %d\n", __func__, speed);
	switch (speed) {
	case 1250:
	case 2500:
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_100MHZ: /* 100MHz */
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.tx0_rate = (mode0.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 2 : 0;
			rate.s.tx1_rate = (mode1.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 2 : 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	case 3125:
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.tx0_rate = (mode0.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 1 : 0;
			rate.s.tx1_rate = (mode1.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 1 : 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	case 5000: /* QSGMII only */
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_100MHZ: /* 100MHz */
			rate.s.tx0_rate = 0;
			rate.s.tx1_rate = 0;
			break;
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.tx0_rate = 0;
			rate.s.tx1_rate = 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	case 6250:
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.tx0_rate = 0;
			rate.s.tx1_rate = 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	default:
		printf("%s: Invalid rate %d\n", __func__, speed);
		return -1;
	}
	debug("%s: tx 0 rate: %d, tx 1 rate: %d\n", __func__, rate.s.tx0_rate, rate.s.tx1_rate);
	csr_wr(CVMX_GSERX_DLMX_TX_RATE(0, 0), rate.u64);

	/* 2. Set GSER0_DLM0_TX_EN[TXn_EN] = 1 */
	en.u64 = csr_rd(CVMX_GSERX_DLMX_TX_EN(0, 0));
	en.s.tx0_en = need0;
	en.s.tx1_en = need1;
	csr_wr(CVMX_GSERX_DLMX_TX_EN(0, 0), en.u64);

	/* 3 set GSER0_DLM0_TX_CM_EN[TXn_CM_EN] = 1 */
	cm_en.u64 = csr_rd(CVMX_GSERX_DLMX_TX_CM_EN(0, 0));
	cm_en.s.tx0_cm_en = need0;
	cm_en.s.tx1_cm_en = need1;
	csr_wr(CVMX_GSERX_DLMX_TX_CM_EN(0, 0), cm_en.u64);

	/* 4. Set GSER0_DLM0_TX_DATA_EN[TXn_DATA_EN] = 1 */
	data_en.u64 = csr_rd(CVMX_GSERX_DLMX_TX_DATA_EN(0, 0));
	data_en.s.tx0_data_en = need0;
	data_en.s.tx1_data_en = need1;
	csr_wr(CVMX_GSERX_DLMX_TX_DATA_EN(0, 0), data_en.u64);

	/* 5. Clear GSER0_DLM0_TX_RESET[TXn_DATA_EN] = 0 */
	tx_reset.u64 = csr_rd(CVMX_GSERX_DLMX_TX_RESET(0, 0));
	tx_reset.s.tx0_reset = !need0;
	tx_reset.s.tx1_reset = !need1;
	csr_wr(CVMX_GSERX_DLMX_TX_RESET(0, 0), tx_reset.u64);

	/* 6. Poll GSER0_DLM0_TX_STATUS[TXn_STATUS, TXn_CM_STATUS] until both
	 *    are set to 1. This prevents GMX from transmitting until the DLM
	 *    is ready.
	 */
	if (need0) {
		if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_TX_STATUS(0, 0),
					  cvmx_gserx_dlmx_tx_status_t, tx0_status, ==, 1, 10000)) {
			printf("DLM0 TX0 status fail\n");
			return -1;
		}
		if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_TX_STATUS(0, 0),
					  cvmx_gserx_dlmx_tx_status_t, tx0_cm_status, ==, 1,
					  10000)) {
			printf("DLM0 TX0 CM status fail\n");
			return -1;
		}
	}
	if (need1) {
		if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_TX_STATUS(0, 0),
					  cvmx_gserx_dlmx_tx_status_t, tx1_status, ==, 1, 10000)) {
			printf("DLM0 TX1 status fail\n");
			return -1;
		}
		if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_TX_STATUS(0, 0),
					  cvmx_gserx_dlmx_tx_status_t, tx1_cm_status, ==, 1,
					  10000)) {
			printf("DLM0 TX1 CM status fail\n");
			return -1;
		}
	}
	return 0;
}

static int __dlm0_setup_rx_cn70xx(int speed, int ref_clk_sel)
{
	int need0, need1;
	cvmx_gmxx_inf_mode_t mode0, mode1;
	cvmx_gserx_dlmx_rx_rate_t rate;
	cvmx_gserx_dlmx_rx_pll_en_t pll_en;
	cvmx_gserx_dlmx_rx_data_en_t data_en;
	cvmx_gserx_dlmx_rx_reset_t rx_reset;

	debug("%s(%d, %d)\n", __func__, speed, ref_clk_sel);
	mode0.u64 = csr_rd(CVMX_GMXX_INF_MODE(0));
	mode1.u64 = csr_rd(CVMX_GMXX_INF_MODE(1));

	/* Which lanes do we need? */
	need0 = (mode0.s.mode != CVMX_GMX_INF_MODE_DISABLED);
	need1 = (mode1.s.mode != CVMX_GMX_INF_MODE_DISABLED) ||
		(mode0.s.mode == CVMX_GMX_INF_MODE_RXAUI);

	/* 1. Write GSER0_DLM0_RX_RATE[RXn_RATE] (must match the
	 * GER0_DLM0_TX_RATE[TXn_RATE] setting).
	 */
	rate.u64 = csr_rd(CVMX_GSERX_DLMX_RX_RATE(0, 0));
	switch (speed) {
	case 1250:
	case 2500:
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_100MHZ: /* 100MHz */
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.rx0_rate = (mode0.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 2 : 0;
			rate.s.rx1_rate = (mode1.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 2 : 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	case 3125:
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.rx0_rate = (mode0.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 1 : 0;
			rate.s.rx1_rate = (mode1.s.mode == CVMX_GMX_INF_MODE_SGMII) ? 1 : 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	case 5000: /* QSGMII only */
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_100MHZ: /* 100MHz */
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.rx0_rate = 0;
			rate.s.rx1_rate = 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	case 6250:
		switch (ref_clk_sel) {
		case OCTEON_QLM_REF_CLK_125MHZ: /* 125MHz */
		case OCTEON_QLM_REF_CLK_156MHZ: /* 156.25MHz */
			rate.s.rx0_rate = 0;
			rate.s.rx1_rate = 0;
			break;
		default:
			printf("Invalid reference clock select %d\n", ref_clk_sel);
			return -1;
		}
		break;
	default:
		printf("%s: Invalid rate %d\n", __func__, speed);
		return -1;
	}
	debug("%s: rx 0 rate: %d, rx 1 rate: %d\n", __func__, rate.s.rx0_rate, rate.s.rx1_rate);
	csr_wr(CVMX_GSERX_DLMX_RX_RATE(0, 0), rate.u64);

	/* 2. Set GSER0_DLM0_RX_PLL_EN[RXn_PLL_EN] = 1 */
	pll_en.u64 = csr_rd(CVMX_GSERX_DLMX_RX_PLL_EN(0, 0));
	pll_en.s.rx0_pll_en = need0;
	pll_en.s.rx1_pll_en = need1;
	csr_wr(CVMX_GSERX_DLMX_RX_PLL_EN(0, 0), pll_en.u64);

	/* 3. Set GSER0_DLM0_RX_DATA_EN[RXn_DATA_EN] = 1 */
	data_en.u64 = csr_rd(CVMX_GSERX_DLMX_RX_DATA_EN(0, 0));
	data_en.s.rx0_data_en = need0;
	data_en.s.rx1_data_en = need1;
	csr_wr(CVMX_GSERX_DLMX_RX_DATA_EN(0, 0), data_en.u64);

	/* 4. Clear GSER0_DLM0_RX_RESET[RXn_DATA_EN] = 0. Now the GMX can be
	 * enabled: set GMX(0..1)_INF_MODE[EN] = 1
	 */
	rx_reset.u64 = csr_rd(CVMX_GSERX_DLMX_RX_RESET(0, 0));
	rx_reset.s.rx0_reset = !need0;
	rx_reset.s.rx1_reset = !need1;
	csr_wr(CVMX_GSERX_DLMX_RX_RESET(0, 0), rx_reset.u64);

	return 0;
}

static int a_clk;

static int __dlm2_sata_uctl_init_cn70xx(void)
{
	cvmx_sata_uctl_ctl_t uctl_ctl;
	const int MAX_A_CLK = 333000000; /* Max of 333Mhz */
	int divisor, a_clkdiv;

	/* Wait for all voltages to reach a stable stable. Ensure the
	 * reference clock is up and stable.
	 */

	/* 2. Wait for IOI reset to deassert. */

	/* 3. Optionally program the GPIO CSRs for SATA features.
	 *    a. For cold-presence detect:
	 *	 i. Select a GPIO for the input and program GPIO_SATA_CTL[sel]
	 *	    for port0 and port1.
	 *	 ii. Select a GPIO for the output and program
	 *	     GPIO_BIT_CFG*[OUTPUT_SEL] for port0 and port1.
	 *    b. For mechanical-presence detect, select a GPIO for the input
	 *	 and program GPIO_SATA_CTL[SEL] for port0/port1.
	 *    c. For LED activity, select a GPIO for the output and program
	 *	 GPIO_BIT_CFG*[OUTPUT_SEL] for port0/port1.
	 */

	/* 4. Assert all resets:
	 *    a. UAHC reset: SATA_UCTL_CTL[UAHC_RST] = 1
	 *    a. UCTL reset: SATA_UCTL_CTL[UCTL_RST] = 1
	 */

	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
	uctl_ctl.s.sata_uahc_rst = 1;
	uctl_ctl.s.sata_uctl_rst = 1;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	/* 5. Configure the ACLK:
	 *    a. Reset the clock dividers: SATA_UCTL_CTL[A_CLKDIV_RST] = 1.
	 *    b. Select the ACLK frequency (400 MHz maximum)
	 *	 i. SATA_UCTL_CTL[A_CLKDIV] = desired value,
	 *	 ii. SATA_UCTL_CTL[A_CLKDIV_EN] = 1 to enable the ACLK,
	 *    c. Deassert the ACLK clock divider reset:
	 *	 SATA_UCTL_CTL[A_CLKDIV_RST] = 0
	 */
	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
	uctl_ctl.s.a_clkdiv_rst = 1;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);

	divisor = (gd->bus_clk + MAX_A_CLK - 1) / MAX_A_CLK;
	if (divisor <= 4) {
		a_clkdiv = divisor - 1;
	} else if (divisor <= 6) {
		a_clkdiv = 4;
		divisor = 6;
	} else if (divisor <= 8) {
		a_clkdiv = 5;
		divisor = 8;
	} else if (divisor <= 16) {
		a_clkdiv = 6;
		divisor = 16;
	} else if (divisor <= 24) {
		a_clkdiv = 7;
		divisor = 24;
	} else {
		printf("Unable to determine SATA clock divisor\n");
		return -1;
	}

	/* Calculate the final clock rate */
	a_clk = gd->bus_clk / divisor;

	uctl_ctl.s.a_clkdiv_sel = a_clkdiv;
	uctl_ctl.s.a_clk_en = 1;
	uctl_ctl.s.a_clk_byp_sel = 0;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
	uctl_ctl.s.a_clkdiv_rst = 0;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	udelay(1);

	return 0;
}

static int __sata_dlm_init_cn70xx(int qlm, int baud_mhz, int ref_clk_sel, int ref_clk_input)
{
	cvmx_gserx_sata_cfg_t sata_cfg;
	cvmx_gserx_sata_lane_rst_t sata_lane_rst;
	cvmx_gserx_dlmx_phy_reset_t dlmx_phy_reset;
	cvmx_gserx_dlmx_test_powerdown_t dlmx_test_powerdown;
	cvmx_gserx_sata_ref_ssp_en_t ref_ssp_en;
	cvmx_gserx_dlmx_mpll_multiplier_t mpll_multiplier;
	cvmx_gserx_dlmx_ref_clkdiv2_t ref_clkdiv2;
	cvmx_sata_uctl_shim_cfg_t shim_cfg;
	cvmx_gserx_phyx_ovrd_in_lo_t ovrd_in;
	cvmx_sata_uctl_ctl_t uctl_ctl;
	int sata_ref_clk;

	debug("%s(%d, %d, %d, %d)\n", __func__, qlm, baud_mhz, ref_clk_sel, ref_clk_input);

	switch (ref_clk_sel) {
	case 0:
		sata_ref_clk = 100;
		break;
	case 1:
		sata_ref_clk = 125;
		break;
	case 2:
		sata_ref_clk = 156;
		break;
	default:
		printf("%s: Invalid reference clock select %d for qlm %d\n", __func__,
		       ref_clk_sel, qlm);
		return -1;
	}

	/* 5. Set GSERX0_SATA_CFG[SATA_EN] = 1 to configure DLM2 multiplexing.
	 */
	sata_cfg.u64 = csr_rd(CVMX_GSERX_SATA_CFG(0));
	sata_cfg.s.sata_en = 1;
	csr_wr(CVMX_GSERX_SATA_CFG(0), sata_cfg.u64);

	/* 1. Write GSER(0)_DLM2_REFCLK_SEL[REFCLK_SEL] if required for
	 *    reference-clock selection.
	 */
	if (ref_clk_input < 2) {
		csr_wr(CVMX_GSERX_DLMX_REFCLK_SEL(qlm, 0), ref_clk_input);
		csr_wr(CVMX_GSERX_DLMX_REF_USE_PAD(qlm, 0), 0);
	} else {
		csr_wr(CVMX_GSERX_DLMX_REF_USE_PAD(qlm, 0), 1);
	}

	ref_ssp_en.u64 = csr_rd(CVMX_GSERX_SATA_REF_SSP_EN(0));
	ref_ssp_en.s.ref_ssp_en = 1;
	csr_wr(CVMX_GSERX_SATA_REF_SSP_EN(0), ref_ssp_en.u64);

	/* Apply workaround for Errata (G-20669) MPLL may not come up. */

	/* Set REF_CLKDIV2 based on the Ref Clock */
	ref_clkdiv2.u64 = csr_rd(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0));
	if (sata_ref_clk == 100)
		ref_clkdiv2.s.ref_clkdiv2 = 0;
	else
		ref_clkdiv2.s.ref_clkdiv2 = 1;
	csr_wr(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0), ref_clkdiv2.u64);

	/* 1. Ensure GSER(0)_DLM(0..2)_PHY_RESET[PHY_RESET] is set. */
	dlmx_phy_reset.u64 = csr_rd(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0));
	dlmx_phy_reset.s.phy_reset = 1;
	csr_wr(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0), dlmx_phy_reset.u64);

	/* 2. If SGMII or QSGMII or RXAUI (i.e. if DLM0) set
	 *    GSER(0)_DLM(0)_MPLL_EN[MPLL_EN] to one.
	 */

	/* 3. Set GSER(0)_DLM(0..2)_MPLL_MULTIPLIER[MPLL_MULTIPLIER]
	 *    to the value in the preceding table, which is different
	 *    than the desired setting prescribed by the HRM.
	 */

	mpll_multiplier.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
	if (sata_ref_clk == 100)
		mpll_multiplier.s.mpll_multiplier = 35;
	else
		mpll_multiplier.s.mpll_multiplier = 56;
	csr_wr(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0), mpll_multiplier.u64);

	/* 3. Clear GSER0_DLM2_TEST_POWERDOWN[TEST_POWERDOWN] = 0 */
	dlmx_test_powerdown.u64 = csr_rd(CVMX_GSERX_DLMX_TEST_POWERDOWN(qlm, 0));
	dlmx_test_powerdown.s.test_powerdown = 0;
	csr_wr(CVMX_GSERX_DLMX_TEST_POWERDOWN(qlm, 0), dlmx_test_powerdown.u64);

	/* 4. Clear either/both lane0 and lane1 resets:
	 *    GSER0_SATA_LANE_RST[L0_RST, L1_RST] = 0.
	 */
	sata_lane_rst.u64 = csr_rd(CVMX_GSERX_SATA_LANE_RST(0));
	sata_lane_rst.s.l0_rst = 0;
	sata_lane_rst.s.l1_rst = 0;
	csr_wr(CVMX_GSERX_SATA_LANE_RST(0), sata_lane_rst.u64);

	udelay(1);

	/* 5. Clear GSER0_DLM2_PHY_RESET */
	dlmx_phy_reset.u64 = csr_rd(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0));
	dlmx_phy_reset.s.phy_reset = 0;
	csr_wr(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0), dlmx_phy_reset.u64);

	/* 6. If PCIe or SATA (i.e. if DLM1 or DLM2), set both MPLL_EN
	 * and MPLL_EN_OVRD to one in GSER(0)_PHY(1..2)_OVRD_IN_LO.
	 */
	ovrd_in.u64 = csr_rd(CVMX_GSERX_PHYX_OVRD_IN_LO(qlm, 0));
	ovrd_in.s.mpll_en = 1;
	ovrd_in.s.mpll_en_ovrd = 1;
	csr_wr(CVMX_GSERX_PHYX_OVRD_IN_LO(qlm, 0), ovrd_in.u64);

	/* 7. Decrease MPLL_MULTIPLIER by one continually until it reaches
	 *   the desired long-term setting, ensuring that each MPLL_MULTIPLIER
	 *   value is constant for at least 1 msec before changing to the next
	 *   value. The desired long-term setting is as indicated in HRM tables
	 *   21-1, 21-2, and 21-3. This is not required with the HRM
	 *   sequence.
	 */
	mpll_multiplier.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
	if (sata_ref_clk == 100)
		mpll_multiplier.s.mpll_multiplier = 0x1e;
	else
		mpll_multiplier.s.mpll_multiplier = 0x30;
	csr_wr(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0), mpll_multiplier.u64);

	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_MPLL_STATUS(qlm, 0),
				  cvmx_gserx_dlmx_mpll_status_t, mpll_status, ==, 1, 10000)) {
		printf("ERROR: SATA MPLL failed to set\n");
		return -1;
	}

	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_RX_STATUS(qlm, 0), cvmx_gserx_dlmx_rx_status_t,
				  rx0_status, ==, 1, 10000)) {
		printf("ERROR: SATA RX0_STATUS failed to set\n");
		return -1;
	}
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_DLMX_RX_STATUS(qlm, 0), cvmx_gserx_dlmx_rx_status_t,
				  rx1_status, ==, 1, 10000)) {
		printf("ERROR: SATA RX1_STATUS failed to set\n");
		return -1;
	}

	/* 8. Deassert UCTL and UAHC resets:
	 *    a. SATA_UCTL_CTL[UCTL_RST] = 0
	 *    b. SATA_UCTL_CTL[UAHC_RST] = 0
	 *    c. Wait 10 ACLK cycles before accessing any ACLK-only registers.
	 */
	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
	uctl_ctl.s.sata_uctl_rst = 0;
	uctl_ctl.s.sata_uahc_rst = 0;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	udelay(1);

	/* 9. Enable conditional SCLK of UCTL by writing
	 *    SATA_UCTL_CTL[CSCLK_EN] = 1
	 */
	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
	uctl_ctl.s.csclk_en = 1;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	/* 10. Initialize UAHC as described in the AHCI Specification (UAHC_*
	 *     registers
	 */

	/* set-up endian mode */
	shim_cfg.u64 = csr_rd(CVMX_SATA_UCTL_SHIM_CFG);
	shim_cfg.s.dma_endian_mode = 1;
	shim_cfg.s.csr_endian_mode = 3;
	csr_wr(CVMX_SATA_UCTL_SHIM_CFG, shim_cfg.u64);

	return 0;
}

/**
 * Initializes DLM 4 for SATA
 *
 * @param qlm		Must be 4.
 * @param baud_mhz	Baud rate for SATA
 * @param ref_clk_sel	Selects the speed of the reference clock where:
 *			0 = 100MHz, 1 = 125MHz and 2 = 156.25MHz
 * @param ref_clk_input	Reference clock input where 0 = external QLM clock,
 *			1 = qlmc_ref_clk0 and 2 = qlmc_ref_clk1
 */
static int __sata_dlm_init_cn73xx(int qlm, int baud_mhz, int ref_clk_sel, int ref_clk_input)
{
	cvmx_sata_uctl_shim_cfg_t shim_cfg;
	cvmx_gserx_refclk_sel_t refclk_sel;
	cvmx_gserx_phy_ctl_t phy_ctl;
	cvmx_gserx_rx_pwr_ctrl_p2_t pwr_ctrl_p2;
	cvmx_gserx_lanex_misc_cfg_0_t misc_cfg_0;
	cvmx_gserx_sata_lane_rst_t lane_rst;
	cvmx_gserx_pll_px_mode_0_t pmode_0;
	cvmx_gserx_pll_px_mode_1_t pmode_1;
	cvmx_gserx_lane_px_mode_0_t lane_pmode_0;
	cvmx_gserx_lane_px_mode_1_t lane_pmode_1;
	cvmx_gserx_cfg_t gserx_cfg;
	cvmx_sata_uctl_ctl_t uctl_ctl;
	int l;
	int i;

	/*
	 * 1. Configure the SATA
	 */

	/*
	 * 2. Configure the QLM Reference clock
	 *    Set GSERX_REFCLK_SEL.COM_CLK_SEL to source reference clock
	 *    from the external clock mux.
	 *      GSERX_REFCLK_SEL.USE_COM1 to select qlmc_refclkn/p_1 or
	 *      leave clear to select qlmc_refclkn/p_0
	 */
	refclk_sel.u64 = 0;
	if (ref_clk_input == 0) { /* External ref clock */
		refclk_sel.s.com_clk_sel = 0;
		refclk_sel.s.use_com1 = 0;
	} else if (ref_clk_input == 1) { /* Common reference clock 0 */
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 0;
	} else { /* Common reference clock 1 */
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 1;
	}

	if (ref_clk_sel != 0) {
		printf("Wrong reference clock selected for QLM4\n");
		return -1;
	}

	csr_wr(CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);

	/* Reset the QLM after changing the reference clock */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 1;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	udelay(1);

	/*
	 * 3. Configure the QLM for SATA mode set GSERX_CFG.SATA
	 */
	gserx_cfg.u64 = 0;
	gserx_cfg.s.sata = 1;
	csr_wr(CVMX_GSERX_CFG(qlm), gserx_cfg.u64);

	/*
	 * 12. Clear the appropriate lane resets
	 *     clear GSERX_SATA_LANE_RST.LX_RST  where X is the lane number 0-1.
	 */
	lane_rst.u64 = csr_rd(CVMX_GSERX_SATA_LANE_RST(qlm));
	lane_rst.s.l0_rst = 0;
	lane_rst.s.l1_rst = 0;
	csr_wr(CVMX_GSERX_SATA_LANE_RST(qlm), lane_rst.u64);
	csr_rd(CVMX_GSERX_SATA_LANE_RST(qlm));

	udelay(1);

	/*
	 * 4. Take the PHY out of reset
	 *    Write GSERX_PHY_CTL.PHY_RESET to a zero
	 */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 0;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	/* Wait for reset to complete and the PLL to lock */
	/* PCIe mode doesn't become ready until the PEM block attempts to bring
	 * the interface up. Skip this check for PCIe
	 */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm), cvmx_gserx_qlm_stat_t,
				  rst_rdy, ==, 1, 10000)) {
		printf("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", qlm);
		return -1;
	}

	/* Workaround for errata GSER-30310: SATA HDD Not Ready due to
	 * PHY SDLL/LDLL lockup at 3GHz
	 */
	for (i = 0; i < 2; i++) {
		cvmx_gserx_slicex_pcie1_mode_t pcie1;
		cvmx_gserx_slicex_pcie2_mode_t pcie2;
		cvmx_gserx_slicex_pcie3_mode_t pcie3;

		pcie1.u64 = csr_rd(CVMX_GSERX_SLICEX_PCIE1_MODE(i, qlm));
		pcie1.s.rx_pi_bwsel = 1;
		pcie1.s.rx_ldll_bwsel = 1;
		pcie1.s.rx_sdll_bwsel = 1;
		csr_wr(CVMX_GSERX_SLICEX_PCIE1_MODE(i, qlm), pcie1.u64);

		pcie2.u64 = csr_rd(CVMX_GSERX_SLICEX_PCIE2_MODE(i, qlm));
		pcie2.s.rx_pi_bwsel = 1;
		pcie2.s.rx_ldll_bwsel = 1;
		pcie2.s.rx_sdll_bwsel = 1;
		csr_wr(CVMX_GSERX_SLICEX_PCIE2_MODE(i, qlm), pcie2.u64);

		pcie3.u64 = csr_rd(CVMX_GSERX_SLICEX_PCIE3_MODE(i, qlm));
		pcie3.s.rx_pi_bwsel = 1;
		pcie3.s.rx_ldll_bwsel = 1;
		pcie3.s.rx_sdll_bwsel = 1;
		csr_wr(CVMX_GSERX_SLICEX_PCIE3_MODE(i, qlm), pcie3.u64);
	}

	/*
	 * 7. Change P2 termination
	 *    Clear GSERX_RX_PWR_CTRL_P2.P2_RX_SUBBLK_PD[0] (Termination)
	 */
	pwr_ctrl_p2.u64 = csr_rd(CVMX_GSERX_RX_PWR_CTRL_P2(qlm));
	pwr_ctrl_p2.s.p2_rx_subblk_pd &= 0x1e;
	csr_wr(CVMX_GSERX_RX_PWR_CTRL_P2(qlm), pwr_ctrl_p2.u64);

	/*
	 * 8. Modify the Electrical IDLE Detect on delay
	 *    Change GSERX_LANE(0..3)_MISC_CFG_0.EIE_DET_STL_ON_TIME to a 0x4
	 */
	for (i = 0; i < 2; i++) {
		misc_cfg_0.u64 = csr_rd(CVMX_GSERX_LANEX_MISC_CFG_0(i, qlm));
		misc_cfg_0.s.eie_det_stl_on_time = 4;
		csr_wr(CVMX_GSERX_LANEX_MISC_CFG_0(i, qlm), misc_cfg_0.u64);
	}

	/*
	 * 9. Modify the PLL and Lane Protocol Mode registers to configure
	 *    the PHY for SATA.
	 *    (Configure all 3 PLLs, doesn't matter what speed it is configured)
	 */

	/* Errata (GSER-26724) SATA never indicates GSER QLM_STAT[RST_RDY]
	 * We program PLL_PX_MODE_0 last due to this errata
	 */
	for (l = 0; l < 3; l++) {
		pmode_1.u64 = csr_rd(CVMX_GSERX_PLL_PX_MODE_1(l, qlm));
		lane_pmode_0.u64 = csr_rd(CVMX_GSERX_LANE_PX_MODE_0(l, qlm));
		lane_pmode_1.u64 = csr_rd(CVMX_GSERX_LANE_PX_MODE_1(l, qlm));

		pmode_1.s.pll_cpadj = 0x2;
		pmode_1.s.pll_opr = 0x0;
		pmode_1.s.pll_div = 0x1e;
		pmode_1.s.pll_pcie3en = 0x0;
		pmode_1.s.pll_16p5en = 0x0;

		lane_pmode_0.s.ctle = 0x0;
		lane_pmode_0.s.pcie = 0x0;
		lane_pmode_0.s.tx_ldiv = 0x0;
		lane_pmode_0.s.srate = 0;
		lane_pmode_0.s.tx_mode = 0x3;
		lane_pmode_0.s.rx_mode = 0x3;

		lane_pmode_1.s.vma_mm = 1;
		lane_pmode_1.s.vma_fine_cfg_sel = 0;
		lane_pmode_1.s.cdr_fgain = 0xa;
		lane_pmode_1.s.ph_acc_adj = 0x15;

		if (l == R_2_5G_REFCLK100)
			lane_pmode_0.s.rx_ldiv = 0x2;
		else if (l == R_5G_REFCLK100)
			lane_pmode_0.s.rx_ldiv = 0x1;
		else
			lane_pmode_0.s.rx_ldiv = 0x0;

		csr_wr(CVMX_GSERX_PLL_PX_MODE_1(l, qlm), pmode_1.u64);
		csr_wr(CVMX_GSERX_LANE_PX_MODE_0(l, qlm), lane_pmode_0.u64);
		csr_wr(CVMX_GSERX_LANE_PX_MODE_1(l, qlm), lane_pmode_1.u64);
	}

	for (l = 0; l < 3; l++) {
		pmode_0.u64 = csr_rd(CVMX_GSERX_PLL_PX_MODE_0(l, qlm));
		pmode_0.s.pll_icp = 0x1;
		pmode_0.s.pll_rloop = 0x3;
		pmode_0.s.pll_pcs_div = 0x5;
		csr_wr(CVMX_GSERX_PLL_PX_MODE_0(l, qlm), pmode_0.u64);
	}

	for (i = 0; i < 2; i++) {
		cvmx_gserx_slicex_rx_sdll_ctrl_t rx_sdll;

		rx_sdll.u64 = csr_rd(CVMX_GSERX_SLICEX_RX_SDLL_CTRL(i, qlm));
		rx_sdll.s.pcs_sds_oob_clk_ctrl = 2;
		rx_sdll.s.pcs_sds_rx_sdll_tune = 0;
		rx_sdll.s.pcs_sds_rx_sdll_swsel = 0;
		csr_wr(CVMX_GSERX_SLICEX_RX_SDLL_CTRL(i, qlm), rx_sdll.u64);
	}

	for (i = 0; i < 2; i++) {
		cvmx_gserx_lanex_misc_cfg_0_t misc_cfg;

		misc_cfg.u64 = csr_rd(CVMX_GSERX_LANEX_MISC_CFG_0(i, qlm));
		misc_cfg.s.use_pma_polarity = 0;
		misc_cfg.s.cfg_pcs_loopback = 0;
		misc_cfg.s.pcs_tx_mode_ovrrd_en = 0;
		misc_cfg.s.pcs_rx_mode_ovrrd_en = 0;
		misc_cfg.s.cfg_eie_det_cnt = 0;
		misc_cfg.s.eie_det_stl_on_time = 4;
		misc_cfg.s.eie_det_stl_off_time = 0;
		misc_cfg.s.tx_bit_order = 1;
		misc_cfg.s.rx_bit_order = 1;
		csr_wr(CVMX_GSERX_LANEX_MISC_CFG_0(i, qlm), misc_cfg.u64);
	}

	/* Wait for reset to complete and the PLL to lock */
	/* PCIe mode doesn't become ready until the PEM block attempts to bring
	 * the interface up. Skip this check for PCIe
	 */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm), cvmx_gserx_qlm_stat_t,
				  rst_rdy, ==, 1, 10000)) {
		printf("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", qlm);
		return -1;
	}

	/* Poll GSERX_SATA_STATUS for P0_RDY = 1 */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_SATA_STATUS(qlm), cvmx_gserx_sata_status_t,
				  p0_rdy, ==, 1, 10000)) {
		printf("QLM4: Timeout waiting for GSERX_SATA_STATUS[p0_rdy]\n");
		return -1;
	}

	/* Poll GSERX_SATA_STATUS for P1_RDY = 1 */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_SATA_STATUS(qlm), cvmx_gserx_sata_status_t,
				  p1_rdy, ==, 1, 10000)) {
		printf("QLM4: Timeout waiting for GSERX_SATA_STATUS[p1_rdy]\n");
		return -1;
	}

	udelay(2000);

	/* 6. Deassert UCTL and UAHC resets:
	 *    a. SATA_UCTL_CTL[UCTL_RST] = 0
	 *    b. SATA_UCTL_CTL[UAHC_RST] = 0
	 *    c. Wait 10 ACLK cycles before accessing any ACLK-only registers.
	 */
	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
	uctl_ctl.s.sata_uctl_rst = 0;
	uctl_ctl.s.sata_uahc_rst = 0;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	udelay(1);

	/* 7. Enable conditional SCLK of UCTL by writing
	 *    SATA_UCTL_CTL[CSCLK_EN] = 1
	 */
	uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
	uctl_ctl.s.csclk_en = 1;
	csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

	/* set-up endian mode */
	shim_cfg.u64 = csr_rd(CVMX_SATA_UCTL_SHIM_CFG);
	shim_cfg.s.dma_endian_mode = 1;
	shim_cfg.s.csr_endian_mode = 3;
	csr_wr(CVMX_SATA_UCTL_SHIM_CFG, shim_cfg.u64);

	return 0;
}

static int __dlm2_sata_uahc_init_cn70xx(int baud_mhz)
{
	cvmx_sata_uahc_gbl_cap_t gbl_cap;
	cvmx_sata_uahc_px_sctl_t sctl;
	cvmx_sata_uahc_gbl_pi_t pi;
	cvmx_sata_uahc_px_cmd_t cmd;
	cvmx_sata_uahc_px_sctl_t sctl0, sctl1;
	cvmx_sata_uahc_px_ssts_t ssts;
	cvmx_sata_uahc_px_tfd_t tfd;
	cvmx_sata_uahc_gbl_timer1ms_t gbl_timer1ms;
	u64 done;
	int result = -1;
	int retry_count = 0;
	int spd;

	/* From the synopsis data book, SATA_UAHC_GBL_TIMER1MS is the
	 * AMBA clock in MHz * 1000, which is a_clk(Hz) / 1000
	 */
	gbl_timer1ms.u32 = csr_rd32(CVMX_SATA_UAHC_GBL_TIMER1MS);
	gbl_timer1ms.s.timv = a_clk / 1000;
	csr_wr32(CVMX_SATA_UAHC_GBL_TIMER1MS, gbl_timer1ms.u32);
	gbl_timer1ms.u32 = csr_rd32(CVMX_SATA_UAHC_GBL_TIMER1MS);

	/* Set-u global capabilities reg (GBL_CAP) */
	gbl_cap.u32 = csr_rd32(CVMX_SATA_UAHC_GBL_CAP);
	debug("%s: SATA_UAHC_GBL_CAP before: 0x%x\n", __func__, gbl_cap.u32);
	gbl_cap.s.sss = 1;
	gbl_cap.s.smps = 1;
	csr_wr32(CVMX_SATA_UAHC_GBL_CAP, gbl_cap.u32);
	gbl_cap.u32 = csr_rd32(CVMX_SATA_UAHC_GBL_CAP);
	debug("%s: SATA_UAHC_GBL_CAP after: 0x%x\n", __func__, gbl_cap.u32);

	/* Set-up global hba control reg (interrupt enables) */
	/* Set-up port SATA control registers (speed limitation) */
	if (baud_mhz == 1500)
		spd = 1;
	else if (baud_mhz == 3000)
		spd = 2;
	else
		spd = 3;

	sctl.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(0));
	debug("%s: SATA_UAHC_P0_SCTL before: 0x%x\n", __func__, sctl.u32);
	sctl.s.spd = spd;
	csr_wr32(CVMX_SATA_UAHC_PX_SCTL(0), sctl.u32);
	sctl.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(0));
	debug("%s: SATA_UAHC_P0_SCTL after: 0x%x\n", __func__, sctl.u32);
	sctl.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(1));
	debug("%s: SATA_UAHC_P1_SCTL before: 0x%x\n", __func__, sctl.u32);
	sctl.s.spd = spd;
	csr_wr32(CVMX_SATA_UAHC_PX_SCTL(1), sctl.u32);
	sctl.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(1));
	debug("%s: SATA_UAHC_P1_SCTL after: 0x%x\n", __func__, sctl.u32);

	/* Set-up ports implemented reg. */
	pi.u32 = csr_rd32(CVMX_SATA_UAHC_GBL_PI);
	debug("%s: SATA_UAHC_GBL_PI before: 0x%x\n", __func__, pi.u32);
	pi.s.pi = 3;
	csr_wr32(CVMX_SATA_UAHC_GBL_PI, pi.u32);
	pi.u32 = csr_rd32(CVMX_SATA_UAHC_GBL_PI);
	debug("%s: SATA_UAHC_GBL_PI after: 0x%x\n", __func__, pi.u32);

retry0:
	/* Clear port SERR and IS registers */
	csr_wr32(CVMX_SATA_UAHC_PX_SERR(0), csr_rd32(CVMX_SATA_UAHC_PX_SERR(0)));
	csr_wr32(CVMX_SATA_UAHC_PX_IS(0), csr_rd32(CVMX_SATA_UAHC_PX_IS(0)));

	/* Set spin-up, power on, FIS RX enable, start, active */
	cmd.u32 = csr_rd32(CVMX_SATA_UAHC_PX_CMD(0));
	debug("%s: SATA_UAHC_P0_CMD before: 0x%x\n", __func__, cmd.u32);
	cmd.s.fre = 1;
	cmd.s.sud = 1;
	cmd.s.pod = 1;
	cmd.s.st = 1;
	cmd.s.icc = 1;
	cmd.s.fbscp = 1; /* Enable FIS-based switching */
	csr_wr32(CVMX_SATA_UAHC_PX_CMD(0), cmd.u32);
	cmd.u32 = csr_rd32(CVMX_SATA_UAHC_PX_CMD(0));
	debug("%s: SATA_UAHC_P0_CMD after: 0x%x\n", __func__, cmd.u32);

	sctl0.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(0));
	sctl0.s.det = 1;
	csr_wr32(CVMX_SATA_UAHC_PX_SCTL(0), sctl0.u32);

	/* check status */
	done = get_timer(0);
	while (1) {
		ssts.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SSTS(0));

		if (ssts.s.ipm == 1 && ssts.s.det == 3) {
			result = 0;
			break;
		} else if (get_timer(done) > 100) {
			result = -1;
			break;
		}

		udelay(100);
	}

	if (result != -1) {
		/* Clear the PxSERR Register, by writing '1s' to each
		 * implemented bit location
		 */
		csr_wr32(CVMX_SATA_UAHC_PX_SERR(0), -1);

		/*
		 * Wait for indication that SATA drive is ready. This is
		 * determined via an examination of PxTFD.STS. If PxTFD.STS.BSY
		 * PxTFD.STS.DRQ, and PxTFD.STS.ERR are all '0', prior to the
		 * maximum allowed time as specified in the ATA/ATAPI-7
		 * specification, the device is ready.
		 */
		/*
		 * Wait for the device to be ready. BSY(7), DRQ(3), and ERR(0)
		 * must be clear
		 */
		done = get_timer(0);
		while (1) {
			tfd.u32 = csr_rd32(CVMX_SATA_UAHC_PX_TFD(0));
			if ((tfd.s.sts & 0x89) == 0) {
				result = 0;
				break;
			} else if (get_timer(done) > 500) {
				if (retry_count < 3) {
					sctl0.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(0));
					sctl0.s.det = 1; /* Perform interface reset */
					csr_wr32(CVMX_SATA_UAHC_PX_SCTL(0), sctl0.u32);
					udelay(1000); /* 1ms dicated by AHCI 1.3 spec */
					sctl0.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(0));
					sctl0.s.det = 0; /* Perform interface reset */
					csr_wr32(CVMX_SATA_UAHC_PX_SCTL(0), sctl0.u32);
					retry_count++;
					goto retry0;
				}
				result = -1;
				break;
			}

			udelay(100);
		}
	}

	if (result == -1)
		printf("SATA0: not available\n");
	else
		printf("SATA0: available\n");

	sctl1.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(1));
	sctl1.s.det = 1;
	csr_wr32(CVMX_SATA_UAHC_PX_SCTL(1), sctl1.u32);

	result = -1;
	retry_count = 0;

retry1:
	/* Clear port SERR and IS registers */
	csr_wr32(CVMX_SATA_UAHC_PX_SERR(1), csr_rd32(CVMX_SATA_UAHC_PX_SERR(1)));
	csr_wr32(CVMX_SATA_UAHC_PX_IS(1), csr_rd32(CVMX_SATA_UAHC_PX_IS(1)));

	/* Set spin-up, power on, FIS RX enable, start, active */
	cmd.u32 = csr_rd32(CVMX_SATA_UAHC_PX_CMD(1));
	debug("%s: SATA_UAHC_P1_CMD before: 0x%x\n", __func__, cmd.u32);
	cmd.s.fre = 1;
	cmd.s.sud = 1;
	cmd.s.pod = 1;
	cmd.s.st = 1;
	cmd.s.icc = 1;
	cmd.s.fbscp = 1; /* Enable FIS-based switching */
	csr_wr32(CVMX_SATA_UAHC_PX_CMD(1), cmd.u32);
	cmd.u32 = csr_rd32(CVMX_SATA_UAHC_PX_CMD(1));
	debug("%s: SATA_UAHC_P1_CMD after: 0x%x\n", __func__, cmd.u32);

	/* check status */
	done = get_timer(0);
	while (1) {
		ssts.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SSTS(1));

		if (ssts.s.ipm == 1 && ssts.s.det == 3) {
			result = 0;
			break;
		} else if (get_timer(done) > 1000) {
			result = -1;
			break;
		}

		udelay(100);
	}

	if (result != -1) {
		/* Clear the PxSERR Register, by writing '1s' to each
		 * implemented bit location
		 */
		csr_wr32(CVMX_SATA_UAHC_PX_SERR(1), csr_rd32(CVMX_SATA_UAHC_PX_SERR(1)));

		/*
		 * Wait for indication that SATA drive is ready. This is
		 * determined via an examination of PxTFD.STS. If PxTFD.STS.BSY
		 * PxTFD.STS.DRQ, and PxTFD.STS.ERR are all '0', prior to the
		 * maximum allowed time as specified in the ATA/ATAPI-7
		 * specification, the device is ready.
		 */
		/*
		 * Wait for the device to be ready. BSY(7), DRQ(3), and ERR(0)
		 * must be clear
		 */
		done = get_timer(0);
		while (1) {
			tfd.u32 = csr_rd32(CVMX_SATA_UAHC_PX_TFD(1));
			if ((tfd.s.sts & 0x89) == 0) {
				result = 0;
				break;
			} else if (get_timer(done) > 500) {
				if (retry_count < 3) {
					sctl0.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(1));
					sctl0.s.det = 1; /* Perform interface reset */
					csr_wr32(CVMX_SATA_UAHC_PX_SCTL(1), sctl0.u32);
					udelay(1000); /* 1ms dicated by AHCI 1.3 spec */
					sctl0.u32 = csr_rd32(CVMX_SATA_UAHC_PX_SCTL(1));
					sctl0.s.det = 0; /* Perform interface reset */
					csr_wr32(CVMX_SATA_UAHC_PX_SCTL(1), sctl0.u32);
					retry_count++;
					goto retry1;
				}
				result = -1;
				break;
			}

			udelay(100);
		}
	}

	if (result == -1)
		printf("SATA1: not available\n");
	else
		printf("SATA1: available\n");

	return 0;
}

static int __sata_bist_cn70xx(int qlm, int baud_mhz, int ref_clk_sel, int ref_clk_input)
{
	cvmx_sata_uctl_bist_status_t bist_status;
	cvmx_sata_uctl_ctl_t uctl_ctl;
	cvmx_sata_uctl_shim_cfg_t shim_cfg;
	u64 done;
	int result = -1;

	debug("%s(%d, %d, %d, %d)\n", __func__, qlm, baud_mhz, ref_clk_sel, ref_clk_input);
	bist_status.u64 = csr_rd(CVMX_SATA_UCTL_BIST_STATUS);

	{
		if (__dlm2_sata_uctl_init_cn70xx()) {
			printf("ERROR: Failed to initialize SATA UCTL CSRs\n");
			return -1;
		}
		if (OCTEON_IS_MODEL(OCTEON_CN73XX))
			result = __sata_dlm_init_cn73xx(qlm, baud_mhz, ref_clk_sel, ref_clk_input);
		else
			result = __sata_dlm_init_cn70xx(qlm, baud_mhz, ref_clk_sel, ref_clk_input);
		if (result) {
			printf("ERROR: Failed to initialize SATA GSER CSRs\n");
			return -1;
		}

		uctl_ctl.u64 = csr_rd(CVMX_SATA_UCTL_CTL);
		uctl_ctl.s.start_bist = 1;
		csr_wr(CVMX_SATA_UCTL_CTL, uctl_ctl.u64);

		/* Set-up for a 1 sec timer. */
		done = get_timer(0);
		while (1) {
			bist_status.u64 = csr_rd(CVMX_SATA_UCTL_BIST_STATUS);
			if ((bist_status.s.uctl_xm_r_bist_ndone |
			     bist_status.s.uctl_xm_w_bist_ndone |
			     bist_status.s.uahc_p0_rxram_bist_ndone |
			     bist_status.s.uahc_p1_rxram_bist_ndone |
			     bist_status.s.uahc_p0_txram_bist_ndone |
			     bist_status.s.uahc_p1_txram_bist_ndone) == 0) {
				result = 0;
				break;
			} else if (get_timer(done) > 1000) {
				result = -1;
				break;
			}

			udelay(100);
		}
		if (result == -1) {
			printf("ERROR: SATA_UCTL_BIST_STATUS = 0x%llx\n",
			       (unsigned long long)bist_status.u64);
			return -1;
		}

		debug("%s: Initializing UAHC\n", __func__);
		if (__dlm2_sata_uahc_init_cn70xx(baud_mhz)) {
			printf("ERROR: Failed to initialize SATA UAHC CSRs\n");
			return -1;
		}
	}

	/* Change CSR_ENDIAN_MODE to big endian to use Open Source AHCI SATA
	 * driver
	 */
	shim_cfg.u64 = csr_rd(CVMX_SATA_UCTL_SHIM_CFG);
	shim_cfg.s.csr_endian_mode = 1;
	csr_wr(CVMX_SATA_UCTL_SHIM_CFG, shim_cfg.u64);

	return 0;
}

static int __setup_sata(int qlm, int baud_mhz, int ref_clk_sel, int ref_clk_input)
{
	debug("%s(%d, %d, %d, %d)\n", __func__, qlm, baud_mhz, ref_clk_sel, ref_clk_input);
	return __sata_bist_cn70xx(qlm, baud_mhz, ref_clk_sel, ref_clk_input);
}

static int __dlmx_setup_pcie_cn70xx(int qlm, enum cvmx_qlm_mode mode, int gen2, int rc,
				    int ref_clk_sel, int ref_clk_input)
{
	cvmx_gserx_dlmx_phy_reset_t dlmx_phy_reset;
	cvmx_gserx_dlmx_test_powerdown_t dlmx_test_powerdown;
	cvmx_gserx_dlmx_mpll_multiplier_t mpll_multiplier;
	cvmx_gserx_dlmx_ref_clkdiv2_t ref_clkdiv2;
	static const u8 ref_clk_mult[2] = { 35, 56 }; /* 100 & 125 MHz ref clock supported. */

	debug("%s(%d, %d, %d, %d, %d, %d)\n", __func__, qlm, mode, gen2, rc, ref_clk_sel,
	      ref_clk_input);
	if (rc == 0) {
		debug("Skipping initializing PCIe dlm %d in endpoint mode\n", qlm);
		return 0;
	}

	if (qlm > 0 && ref_clk_input > 1) {
		printf("%s: Error: ref_clk_input can only be 0 or 1 for QLM %d\n",
		       __func__, qlm);
		return -1;
	}

	if (ref_clk_sel > OCTEON_QLM_REF_CLK_125MHZ) {
		printf("%s: Error: ref_clk_sel can only be 100 or 125 MHZ.\n", __func__);
		return -1;
	}

	/* 1. Write GSER0_DLM(1..2)_REFCLK_SEL[REFCLK_SEL] if required for
	 *    reference-clock selection
	 */

	csr_wr(CVMX_GSERX_DLMX_REFCLK_SEL(qlm, 0), ref_clk_input);

	/* 2. If required, write GSER0_DLM(1..2)_REF_CLKDIV2[REF_CLKDIV2] = 1
	 *    (must be set if reference clock >= 100 MHz)
	 */

	/* 4. Configure the PCIE PIPE:
	 *  a. Write GSER0_PCIE_PIPE_PORT_SEL[PIPE_PORT_SEL] to configure the
	 *     PCIE PIPE.
	 *	0x0 = disables all pipes
	 *	0x1 = enables pipe0 only (PEM0 4-lane)
	 *	0x2 = enables pipes 0 and 1 (PEM0 and PEM1 2-lanes each)
	 *	0x3 = enables pipes 0, 1, 2, and 3 (PEM0, PEM1, and PEM3 are
	 *	      one-lane each)
	 *  b. Configure GSER0_PCIE_PIPE_PORT_SEL[CFG_PEM1_DLM2]. If PEM1 is
	 *     to be configured, this bit must reflect which DLM it is logically
	 *     tied to. This bit sets multiplexing logic in GSER, and it is used
	 *     by the RST logic to determine when the MAC can come out of reset.
	 *	0 = PEM1 is tied to DLM1 (for 3 x 1 PCIe mode).
	 *	1 = PEM1 is tied to DLM2 (for all other PCIe modes).
	 */
	if (qlm == 1) {
		cvmx_gserx_pcie_pipe_port_sel_t pipe_port;

		pipe_port.u64 = csr_rd(CVMX_GSERX_PCIE_PIPE_PORT_SEL(0));
		pipe_port.s.cfg_pem1_dlm2 = (mode == CVMX_QLM_MODE_PCIE_1X1) ? 1 : 0;
		pipe_port.s.pipe_port_sel =
				(mode == CVMX_QLM_MODE_PCIE) ? 1 : /* PEM0 only */
				(mode == CVMX_QLM_MODE_PCIE_1X2) ? 2 : /* PEM0-1 */
				(mode == CVMX_QLM_MODE_PCIE_1X1) ? 3 : /* PEM0-2 */
				(mode == CVMX_QLM_MODE_PCIE_2X1) ? 3 : /* PEM0-1 */
				0; /* PCIe disabled */
		csr_wr(CVMX_GSERX_PCIE_PIPE_PORT_SEL(0), pipe_port.u64);
	}

	/* Apply workaround for Errata (G-20669) MPLL may not come up. */

	/* Set REF_CLKDIV2 based on the Ref Clock */
	ref_clkdiv2.u64 = csr_rd(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0));
	ref_clkdiv2.s.ref_clkdiv2 = ref_clk_sel > 0;
	csr_wr(CVMX_GSERX_DLMX_REF_CLKDIV2(qlm, 0), ref_clkdiv2.u64);

	/* 1. Ensure GSER(0)_DLM(0..2)_PHY_RESET[PHY_RESET] is set. */
	dlmx_phy_reset.u64 = csr_rd(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0));
	dlmx_phy_reset.s.phy_reset = 1;
	csr_wr(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0), dlmx_phy_reset.u64);

	/* 2. If SGMII or QSGMII or RXAUI (i.e. if DLM0) set
	 *    GSER(0)_DLM(0)_MPLL_EN[MPLL_EN] to one.
	 */

	/* 3. Set GSER(0)_DLM(0..2)_MPLL_MULTIPLIER[MPLL_MULTIPLIER]
	 *    to the value in the preceding table, which is different
	 *    than the desired setting prescribed by the HRM.
	 */
	mpll_multiplier.u64 = csr_rd(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
	mpll_multiplier.s.mpll_multiplier = ref_clk_mult[ref_clk_sel];
	debug("%s: Setting MPLL multiplier to %d\n", __func__,
	      (int)mpll_multiplier.s.mpll_multiplier);
	csr_wr(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0), mpll_multiplier.u64);
	/* 5. Clear GSER0_DLM(1..2)_TEST_POWERDOWN. Configurations that only
	 *    use DLM1 need not clear GSER0_DLM2_TEST_POWERDOWN
	 */
	dlmx_test_powerdown.u64 = csr_rd(CVMX_GSERX_DLMX_TEST_POWERDOWN(qlm, 0));
	dlmx_test_powerdown.s.test_powerdown = 0;
	csr_wr(CVMX_GSERX_DLMX_TEST_POWERDOWN(qlm, 0), dlmx_test_powerdown.u64);

	/* 6. Clear GSER0_DLM(1..2)_PHY_RESET. Configurations that use only
	 *    need DLM1 need not clear GSER0_DLM2_PHY_RESET
	 */
	dlmx_phy_reset.u64 = csr_rd(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0));
	dlmx_phy_reset.s.phy_reset = 0;
	csr_wr(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0), dlmx_phy_reset.u64);

	/* 6. Decrease MPLL_MULTIPLIER by one continually until it reaches
	 *    the desired long-term setting, ensuring that each MPLL_MULTIPLIER
	 *   value is constant for at least 1 msec before changing to the next
	 *   value. The desired long-term setting is as indicated in HRM tables
	 *   21-1, 21-2, and 21-3. This is not required with the HRM
	 *   sequence.
	 */
	/* This is set when initializing PCIe after soft reset is asserted. */

	/* 7. Write the GSER0_PCIE_PIPE_RST register to take the appropriate
	 *    PIPE out of reset. There is a PIPEn_RST bit for each PIPE. Clear
	 *    the appropriate bits based on the configuration (reset is
	 *     active high).
	 */
	if (qlm == 1) {
		cvmx_pemx_cfg_t pemx_cfg;
		cvmx_pemx_on_t pemx_on;
		cvmx_gserx_pcie_pipe_rst_t pipe_rst;
		cvmx_rst_ctlx_t rst_ctl;

		switch (mode) {
		case CVMX_QLM_MODE_PCIE:     /* PEM0 on DLM1 & DLM2 */
		case CVMX_QLM_MODE_PCIE_1X2: /* PEM0 on DLM1 */
		case CVMX_QLM_MODE_PCIE_1X1: /* PEM0 on DLM1 using lane 0 */
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));
			pemx_cfg.cn70xx.hostmd = rc;
			if (mode == CVMX_QLM_MODE_PCIE_1X1) {
				pemx_cfg.cn70xx.md =
					gen2 ? CVMX_PEM_MD_GEN2_1LANE : CVMX_PEM_MD_GEN1_1LANE;
			} else if (mode == CVMX_QLM_MODE_PCIE) {
				pemx_cfg.cn70xx.md =
					gen2 ? CVMX_PEM_MD_GEN2_4LANE : CVMX_PEM_MD_GEN1_4LANE;
			} else {
				pemx_cfg.cn70xx.md =
					gen2 ? CVMX_PEM_MD_GEN2_2LANE : CVMX_PEM_MD_GEN1_2LANE;
			}
			csr_wr(CVMX_PEMX_CFG(0), pemx_cfg.u64);

			rst_ctl.u64 = csr_rd(CVMX_RST_CTLX(0));
			rst_ctl.s.rst_drv = 1;
			csr_wr(CVMX_RST_CTLX(0), rst_ctl.u64);

			/* PEM0 is on DLM1&2 which is pipe0 */
			pipe_rst.u64 = csr_rd(CVMX_GSERX_PCIE_PIPE_RST(0));
			pipe_rst.s.pipe0_rst = 0;
			csr_wr(CVMX_GSERX_PCIE_PIPE_RST(0), pipe_rst.u64);

			pemx_on.u64 = csr_rd(CVMX_PEMX_ON(0));
			pemx_on.s.pemon = 1;
			csr_wr(CVMX_PEMX_ON(0), pemx_on.u64);
			break;
		case CVMX_QLM_MODE_PCIE_2X1: /* PEM0 and PEM1 on DLM1 */
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));
			pemx_cfg.cn70xx.hostmd = rc;
			pemx_cfg.cn70xx.md = gen2 ? CVMX_PEM_MD_GEN2_1LANE : CVMX_PEM_MD_GEN1_1LANE;
			csr_wr(CVMX_PEMX_CFG(0), pemx_cfg.u64);

			rst_ctl.u64 = csr_rd(CVMX_RST_CTLX(0));
			rst_ctl.s.rst_drv = 1;
			csr_wr(CVMX_RST_CTLX(0), rst_ctl.u64);

			/* PEM0 is on DLM1 which is pipe0 */
			pipe_rst.u64 = csr_rd(CVMX_GSERX_PCIE_PIPE_RST(0));
			pipe_rst.s.pipe0_rst = 0;
			csr_wr(CVMX_GSERX_PCIE_PIPE_RST(0), pipe_rst.u64);

			pemx_on.u64 = csr_rd(CVMX_PEMX_ON(0));
			pemx_on.s.pemon = 1;
			csr_wr(CVMX_PEMX_ON(0), pemx_on.u64);

			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(1));
			pemx_cfg.cn70xx.hostmd = 1;
			pemx_cfg.cn70xx.md = gen2 ? CVMX_PEM_MD_GEN2_1LANE : CVMX_PEM_MD_GEN1_1LANE;
			csr_wr(CVMX_PEMX_CFG(1), pemx_cfg.u64);
			rst_ctl.u64 = csr_rd(CVMX_RST_CTLX(1));
			rst_ctl.s.rst_drv = 1;
			csr_wr(CVMX_RST_CTLX(1), rst_ctl.u64);
			/* PEM1 is on DLM2 which is pipe1 */
			pipe_rst.u64 = csr_rd(CVMX_GSERX_PCIE_PIPE_RST(0));
			pipe_rst.s.pipe1_rst = 0;
			csr_wr(CVMX_GSERX_PCIE_PIPE_RST(0), pipe_rst.u64);
			pemx_on.u64 = csr_rd(CVMX_PEMX_ON(1));
			pemx_on.s.pemon = 1;
			csr_wr(CVMX_PEMX_ON(1), pemx_on.u64);
			break;
		default:
			break;
		}
	} else {
		cvmx_pemx_cfg_t pemx_cfg;
		cvmx_pemx_on_t pemx_on;
		cvmx_gserx_pcie_pipe_rst_t pipe_rst;
		cvmx_rst_ctlx_t rst_ctl;

		switch (mode) {
		case CVMX_QLM_MODE_PCIE_1X2: /* PEM1 on DLM2 */
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(1));
			pemx_cfg.cn70xx.hostmd = 1;
			pemx_cfg.cn70xx.md = gen2 ? CVMX_PEM_MD_GEN2_2LANE : CVMX_PEM_MD_GEN1_2LANE;
			csr_wr(CVMX_PEMX_CFG(1), pemx_cfg.u64);

			rst_ctl.u64 = csr_rd(CVMX_RST_CTLX(1));
			rst_ctl.s.rst_drv = 1;
			csr_wr(CVMX_RST_CTLX(1), rst_ctl.u64);

			/* PEM1 is on DLM1 lane 0, which is pipe1 */
			pipe_rst.u64 = csr_rd(CVMX_GSERX_PCIE_PIPE_RST(0));
			pipe_rst.s.pipe1_rst = 0;
			csr_wr(CVMX_GSERX_PCIE_PIPE_RST(0), pipe_rst.u64);

			pemx_on.u64 = csr_rd(CVMX_PEMX_ON(1));
			pemx_on.s.pemon = 1;
			csr_wr(CVMX_PEMX_ON(1), pemx_on.u64);
			break;
		case CVMX_QLM_MODE_PCIE_2X1: /* PEM1 and PEM2 on DLM2 */
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(1));
			pemx_cfg.cn70xx.hostmd = 1;
			pemx_cfg.cn70xx.md = gen2 ? CVMX_PEM_MD_GEN2_1LANE : CVMX_PEM_MD_GEN1_1LANE;
			csr_wr(CVMX_PEMX_CFG(1), pemx_cfg.u64);

			rst_ctl.u64 = csr_rd(CVMX_RST_CTLX(1));
			rst_ctl.s.rst_drv = 1;
			csr_wr(CVMX_RST_CTLX(1), rst_ctl.u64);

			/* PEM1 is on DLM2 lane 0, which is pipe2 */
			pipe_rst.u64 = csr_rd(CVMX_GSERX_PCIE_PIPE_RST(0));
			pipe_rst.s.pipe2_rst = 0;
			csr_wr(CVMX_GSERX_PCIE_PIPE_RST(0), pipe_rst.u64);

			pemx_on.u64 = csr_rd(CVMX_PEMX_ON(1));
			pemx_on.s.pemon = 1;
			csr_wr(CVMX_PEMX_ON(1), pemx_on.u64);

			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(2));
			pemx_cfg.cn70xx.hostmd = 1;
			pemx_cfg.cn70xx.md = gen2 ? CVMX_PEM_MD_GEN2_1LANE : CVMX_PEM_MD_GEN1_1LANE;
			csr_wr(CVMX_PEMX_CFG(2), pemx_cfg.u64);

			rst_ctl.u64 = csr_rd(CVMX_RST_CTLX(2));
			rst_ctl.s.rst_drv = 1;
			csr_wr(CVMX_RST_CTLX(2), rst_ctl.u64);

			/* PEM2 is on DLM2 lane 1, which is pipe3 */
			pipe_rst.u64 = csr_rd(CVMX_GSERX_PCIE_PIPE_RST(0));
			pipe_rst.s.pipe3_rst = 0;
			csr_wr(CVMX_GSERX_PCIE_PIPE_RST(0), pipe_rst.u64);

			pemx_on.u64 = csr_rd(CVMX_PEMX_ON(2));
			pemx_on.s.pemon = 1;
			csr_wr(CVMX_PEMX_ON(2), pemx_on.u64);
			break;
		default:
			break;
		}
	}
	return 0;
}

/**
 * Configure dlm speed and mode for cn70xx.
 *
 * @param qlm     The DLM to configure
 * @param speed   The speed the DLM needs to be configured in Mhz.
 * @param mode    The DLM to be configured as SGMII/XAUI/PCIe.
 *                  DLM 0: has 2 interfaces which can be configured as
 *                         SGMII/QSGMII/RXAUI. Need to configure both at the
 *                         same time. These are valid option
 *				CVMX_QLM_MODE_QSGMII,
 *				CVMX_QLM_MODE_SGMII_SGMII,
 *				CVMX_QLM_MODE_SGMII_DISABLED,
 *				CVMX_QLM_MODE_DISABLED_SGMII,
 *				CVMX_QLM_MODE_SGMII_QSGMII,
 *				CVMX_QLM_MODE_QSGMII_QSGMII,
 *				CVMX_QLM_MODE_QSGMII_DISABLED,
 *				CVMX_QLM_MODE_DISABLED_QSGMII,
 *				CVMX_QLM_MODE_QSGMII_SGMII,
 *				CVMX_QLM_MODE_RXAUI_1X2
 *
 *                  DLM 1: PEM0/1 in PCIE_1x4/PCIE_2x1/PCIE_1X1
 *                  DLM 2: PEM0/1/2 in PCIE_1x4/PCIE_1x2/PCIE_2x1/PCIE_1x1
 * @param rc      Only used for PCIe, rc = 1 for root complex mode, 0 for EP mode.
 * @param gen2    Only used for PCIe, gen2 = 1, in GEN2 mode else in GEN1 mode.
 *
 * @param ref_clk_input  The reference-clock input to use to configure QLM
 * @param ref_clk_sel    The reference-clock selection to use to configure QLM
 *
 * Return:       Return 0 on success or -1.
 */
static int octeon_configure_qlm_cn70xx(int qlm, int speed, int mode, int rc, int gen2,
				       int ref_clk_sel, int ref_clk_input)
{
	debug("%s(%d, %d, %d, %d, %d, %d, %d)\n", __func__, qlm, speed, mode, rc, gen2, ref_clk_sel,
	      ref_clk_input);
	switch (qlm) {
	case 0: {
		int is_sff7000_rxaui = 0;
		cvmx_gmxx_inf_mode_t inf_mode0, inf_mode1;

		inf_mode0.u64 = csr_rd(CVMX_GMXX_INF_MODE(0));
		inf_mode1.u64 = csr_rd(CVMX_GMXX_INF_MODE(1));
		if (inf_mode0.s.en || inf_mode1.s.en) {
			debug("DLM0 already configured\n");
			return -1;
		}

		switch (mode) {
		case CVMX_QLM_MODE_SGMII_SGMII:
			debug("  Mode SGMII SGMII\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_SGMII;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_SGMII;
			break;
		case CVMX_QLM_MODE_SGMII_QSGMII:
			debug("  Mode SGMII QSGMII\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_SGMII;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_QSGMII;
			break;
		case CVMX_QLM_MODE_SGMII_DISABLED:
			debug("  Mode SGMII Disabled\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_SGMII;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_DISABLED;
			break;
		case CVMX_QLM_MODE_DISABLED_SGMII:
			debug("Mode Disabled SGMII\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_DISABLED;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_SGMII;
			break;
		case CVMX_QLM_MODE_QSGMII_SGMII:
			debug("  Mode QSGMII SGMII\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_QSGMII;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_SGMII;
			break;
		case CVMX_QLM_MODE_QSGMII_QSGMII:
			debug("  Mode QSGMII QSGMII\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_QSGMII;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_QSGMII;
			break;
		case CVMX_QLM_MODE_QSGMII_DISABLED:
			debug("  Mode QSGMII Disabled\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_QSGMII;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_DISABLED;
			break;
		case CVMX_QLM_MODE_DISABLED_QSGMII:
			debug("Mode Disabled QSGMII\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_DISABLED;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_QSGMII;
			break;
		case CVMX_QLM_MODE_RXAUI:
			debug("  Mode RXAUI\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_RXAUI;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_DISABLED;

			break;
		default:
			debug("  Mode Disabled Disabled\n");
			inf_mode0.s.mode = CVMX_GMX_INF_MODE_DISABLED;
			inf_mode1.s.mode = CVMX_GMX_INF_MODE_DISABLED;
			break;
		}
		csr_wr(CVMX_GMXX_INF_MODE(0), inf_mode0.u64);
		csr_wr(CVMX_GMXX_INF_MODE(1), inf_mode1.u64);

		/* Bringup the PLL */
		if (__dlm_setup_pll_cn70xx(qlm, speed, ref_clk_sel, ref_clk_input,
					   is_sff7000_rxaui))
			return -1;

		/* TX Lanes */
		if (__dlm0_setup_tx_cn70xx(speed, ref_clk_sel))
			return -1;

		/* RX Lanes */
		if (__dlm0_setup_rx_cn70xx(speed, ref_clk_sel))
			return -1;

		/* Enable the interface */
		inf_mode0.u64 = csr_rd(CVMX_GMXX_INF_MODE(0));
		if (inf_mode0.s.mode != CVMX_GMX_INF_MODE_DISABLED)
			inf_mode0.s.en = 1;
		csr_wr(CVMX_GMXX_INF_MODE(0), inf_mode0.u64);
		inf_mode1.u64 = csr_rd(CVMX_GMXX_INF_MODE(1));
		if (inf_mode1.s.mode != CVMX_GMX_INF_MODE_DISABLED)
			inf_mode1.s.en = 1;
		csr_wr(CVMX_GMXX_INF_MODE(1), inf_mode1.u64);
		break;
	}
	case 1:
		switch (mode) {
		case CVMX_QLM_MODE_PCIE: /* PEM0 on DLM1 & DLM2 */
			debug("  Mode PCIe\n");
			if (__dlmx_setup_pcie_cn70xx(1, mode, gen2, rc, ref_clk_sel, ref_clk_input))
				return -1;
			if (__dlmx_setup_pcie_cn70xx(2, mode, gen2, rc, ref_clk_sel, ref_clk_input))
				return -1;
			break;
		case CVMX_QLM_MODE_PCIE_1X2: /* PEM0 on DLM1 */
		case CVMX_QLM_MODE_PCIE_2X1: /* PEM0 & PEM1 on DLM1 */
		case CVMX_QLM_MODE_PCIE_1X1: /* PEM0 on DLM1, only 1 lane */
			debug("  Mode PCIe 1x2, 2x1 or 1x1\n");
			if (__dlmx_setup_pcie_cn70xx(qlm, mode, gen2, rc, ref_clk_sel,
						     ref_clk_input))
				return -1;
			break;
		case CVMX_QLM_MODE_DISABLED:
			debug("  Mode disabled\n");
			break;
		default:
			debug("DLM1 illegal mode specified\n");
			return -1;
		}
		break;
	case 2:
		switch (mode) {
		case CVMX_QLM_MODE_SATA_2X1:
			debug("%s: qlm 2, mode is SATA 2x1\n", __func__);
			/* DLM2 is SATA, PCIE2 is disabled */
			if (__setup_sata(qlm, speed, ref_clk_sel, ref_clk_input))
				return -1;
			break;
		case CVMX_QLM_MODE_PCIE:
			debug("  Mode PCIe\n");
			/* DLM2 is PCIE0, PCIE1-2 are disabled. */
			/* Do nothing, its initialized in DLM1 */
			break;
		case CVMX_QLM_MODE_PCIE_1X2: /* PEM1 on DLM2 */
		case CVMX_QLM_MODE_PCIE_2X1: /* PEM1 & PEM2 on DLM2 */
			debug("  Mode PCIe 1x2 or 2x1\n");
			if (__dlmx_setup_pcie_cn70xx(qlm, mode, gen2, rc, ref_clk_sel,
						     ref_clk_input))
				return -1;
			break;
		case CVMX_QLM_MODE_DISABLED:
			debug("  Mode Disabled\n");
			break;
		default:
			debug("DLM2 illegal mode specified\n");
			return -1;
		}
	default:
		return -1;
	}

	return 0;
}

/**
 * Disables DFE for the specified QLM lane(s).
 * This function should only be called for low-loss channels.
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param lane     Lane to configure, or -1 all lanes
 * @param baud_mhz The speed the QLM needs to be configured in Mhz.
 * @param mode     The QLM to be configured as SGMII/XAUI/PCIe.
 */
void octeon_qlm_dfe_disable(int node, int qlm, int lane, int baud_mhz, int mode)
{
	int num_lanes = cvmx_qlm_get_lanes(qlm);
	int l;
	cvmx_gserx_lanex_rx_loop_ctrl_t loop_ctrl;
	cvmx_gserx_lanex_rx_valbbd_ctrl_0_t ctrl_0;
	cvmx_gserx_lanex_rx_valbbd_ctrl_1_t ctrl_1;
	cvmx_gserx_lanex_rx_valbbd_ctrl_2_t ctrl_2;
	cvmx_gserx_lane_vma_fine_ctrl_2_t lane_vma_fine_ctrl_2;

	/* Interfaces below 5Gbaud are already manually tuned. */
	if (baud_mhz < 5000)
		return;

	/* Don't run on PCIe links, SATA or KR.  These interfaces use training */
	switch (mode) {
	case CVMX_QLM_MODE_10G_KR_1X2:
	case CVMX_QLM_MODE_10G_KR:
	case CVMX_QLM_MODE_40G_KR4:
		return;
	case CVMX_QLM_MODE_PCIE_1X1:
	case CVMX_QLM_MODE_PCIE_2X1:
	case CVMX_QLM_MODE_PCIE_1X2:
	case CVMX_QLM_MODE_PCIE:
	case CVMX_QLM_MODE_PCIE_1X8:
		return;
	case CVMX_QLM_MODE_SATA_2X1:
		return;
	default:
		break;
	}

	/* Updating pre_ctle minimum to 0. This works best for short channels */
	lane_vma_fine_ctrl_2.u64 = csr_rd_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_2(qlm));
	lane_vma_fine_ctrl_2.s.rx_prectle_gain_min_fine = 0;
	csr_wr_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_2(qlm), lane_vma_fine_ctrl_2.u64);

	for (l = 0; l < num_lanes; l++) {
		if (lane != -1 && lane != l)
			continue;

		/* 1. Write GSERX_LANEx_RX_LOOP_CTRL = 0x0270
		 * (var "loop_ctrl" with bits 8 & 1 cleared).
		 * bit<1> dfe_en_byp = 1'b0
		 */
		loop_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_LOOP_CTRL(l, qlm));
		loop_ctrl.s.cfg_rx_lctrl = loop_ctrl.s.cfg_rx_lctrl & 0x3fd;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_LOOP_CTRL(l, qlm), loop_ctrl.u64);

		/* 2. Write GSERX_LANEx_RX_VALBBD_CTRL_1 = 0x0000
		 * (var "ctrl1" with all bits cleared)
		 * bits<14:11> CFG_RX_DFE_C3_MVAL = 4'b0000
		 * bit<10> CFG_RX_DFE_C3_MSGN = 1'b0
		 * bits<9:6> CFG_RX_DFE_C2_MVAL = 4'b0000
		 * bit<5> CFG_RX_DFE_C2_MSGN = 1'b0
		 * bits<4:0> CFG_RX_DFE_C1_MVAL = 5'b00000
		 */
		ctrl_1.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_1(l, qlm));
		ctrl_1.s.dfe_c3_mval = 0;
		ctrl_1.s.dfe_c3_msgn = 0;
		ctrl_1.s.dfe_c2_mval = 0;
		ctrl_1.s.dfe_c2_msgn = 0;
		ctrl_1.s.dfe_c2_mval = 0;
		ctrl_1.s.dfe_c1_mval = 0;
		ctrl_1.s.dfe_c1_msgn = 0;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_1(l, qlm), ctrl_1.u64);

		/* 3. Write GSERX_LANEx_RX_VALBBD_CTRL_0 = 0x2400
		 * (var "ctrl0" with following bits set/cleared)
		 * bits<11:10> CFG_RX_DFE_GAIN = 0x1
		 * bits<9:6> CFG_RX_DFE_C5_MVAL = 4'b0000
		 * bit<5> CFG_RX_DFE_C5_MSGN = 1'b0
		 * bits<4:1> CFG_RX_DFE_C4_MVAL = 4'b0000
		 * bit<0> CFG_RX_DFE_C4_MSGN = 1'b0
		 */
		ctrl_0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_0(l, qlm));
		ctrl_0.s.dfe_gain = 0x1;
		ctrl_0.s.dfe_c5_mval = 0;
		ctrl_0.s.dfe_c5_msgn = 0;
		ctrl_0.s.dfe_c4_mval = 0;
		ctrl_0.s.dfe_c4_msgn = 0;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_0(l, qlm), ctrl_0.u64);

		/* 4. Write GSER(0..13)_LANE(0..3)_RX_VALBBD_CTRL_2 = 0x003F
		 * //enable DFE tap overrides
		 * bit<5> dfe_ovrd_en = 1
		 * bit<4> dfe_c5_ovrd_val = 1
		 * bit<3> dfe_c4_ovrd_val = 1
		 * bit<2> dfe_c3_ovrd_val = 1
		 * bit<1> dfe_c2_ovrd_val = 1
		 * bit<0> dfe_c1_ovrd_val = 1
		 */
		ctrl_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_2(l, qlm));
		ctrl_2.s.dfe_ovrd_en = 0x1;
		ctrl_2.s.dfe_c5_ovrd_val = 0x1;
		ctrl_2.s.dfe_c4_ovrd_val = 0x1;
		ctrl_2.s.dfe_c3_ovrd_val = 0x1;
		ctrl_2.s.dfe_c2_ovrd_val = 0x1;
		ctrl_2.s.dfe_c1_ovrd_val = 0x1;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_2(l, qlm), ctrl_2.u64);
	}
}

/**
 * Disables DFE, uses fixed CTLE Peak value and AGC settings
 * for the specified QLM lane(s).
 * This function should only be called for low-loss channels.
 * This function prevents Rx equalization from happening on all lanes in a QLM
 * This function should be called for all lanes being used in the QLM.
 *
 * @param  node           Node to configure
 * @param  qlm            QLM to configure
 * @param  lane           Lane to configure, or -1 all lanes
 * @param  baud_mhz       The speed the QLM needs to be configured in Mhz.
 * @param  mode           The QLM to be configured as SGMII/XAUI/PCIe.
 * @param  ctle_zero      Equalizer Peaking control
 * @param  agc_pre_ctle   Pre-CTLE gain
 * @param  agc_post_ctle  Post-CTLE gain
 * Return: Zero on success, negative on failure
 */

int octeon_qlm_dfe_disable_ctle_agc(int node, int qlm, int lane, int baud_mhz, int mode,
				    int ctle_zero, int agc_pre_ctle, int agc_post_ctle)
{
	int num_lanes = cvmx_qlm_get_lanes(qlm);
	int l;
	cvmx_gserx_lanex_rx_loop_ctrl_t loop_ctrl;
	cvmx_gserx_lanex_rx_valbbd_ctrl_0_t ctrl_0;
	cvmx_gserx_lanex_pwr_ctrl_t lanex_pwr_ctrl;
	cvmx_gserx_lane_mode_t lmode;
	cvmx_gserx_lane_px_mode_1_t px_mode_1;
	cvmx_gserx_lanex_rx_cfg_5_t rx_cfg_5;
	cvmx_gserx_lanex_rx_cfg_2_t rx_cfg_2;
	cvmx_gserx_lanex_rx_ctle_ctrl_t ctle_ctrl;

	/* Check tuning constraints */
	if (ctle_zero < 0 || ctle_zero > 15) {
		printf("Error: N%d.QLM%d: Invalid CTLE_ZERO(%d).  Must be between -1 and 15.\n",
		       node, qlm, ctle_zero);
		return -1;
	}
	if (agc_pre_ctle < 0 || agc_pre_ctle > 15) {
		printf("Error: N%d.QLM%d: Invalid AGC_Pre_CTLE(%d)\n",
		       node, qlm, agc_pre_ctle);
		return -1;
	}

	if (agc_post_ctle < 0 || agc_post_ctle > 15) {
		printf("Error: N%d.QLM%d: Invalid AGC_Post_CTLE(%d)\n",
		       node, qlm, agc_post_ctle);
		return -1;
	}

	/* Interfaces below 5Gbaud are already manually tuned. */
	if (baud_mhz < 5000)
		return 0;

	/* Don't run on PCIe links, SATA or KR.  These interfaces use training */
	switch (mode) {
	case CVMX_QLM_MODE_10G_KR_1X2:
	case CVMX_QLM_MODE_10G_KR:
	case CVMX_QLM_MODE_40G_KR4:
		return 0;
	case CVMX_QLM_MODE_PCIE_1X1:
	case CVMX_QLM_MODE_PCIE_2X1:
	case CVMX_QLM_MODE_PCIE_1X2:
	case CVMX_QLM_MODE_PCIE:
	case CVMX_QLM_MODE_PCIE_1X8:
		return 0;
	case CVMX_QLM_MODE_SATA_2X1:
		return 0;
	default:
		break;
	}

	lmode.u64 = csr_rd_node(node, CVMX_GSERX_LANE_MODE(qlm));

	/* 1. Enable VMA manual mode for the QLM's lane mode */
	px_mode_1.u64 = csr_rd_node(node, CVMX_GSERX_LANE_PX_MODE_1(lmode.s.lmode, qlm));
	px_mode_1.s.vma_mm = 1;
	csr_wr_node(node, CVMX_GSERX_LANE_PX_MODE_1(lmode.s.lmode, qlm), px_mode_1.u64);

	/* 2. Disable DFE */
	octeon_qlm_dfe_disable(node, qlm, lane, baud_mhz, mode);

	for (l = 0; l < num_lanes; l++) {
		if (lane != -1 && lane != l)
			continue;

		/* 3. Write GSERX_LANEx_RX_VALBBD_CTRL_0.CFG_RX_AGC_GAIN = 0x2 */
		ctrl_0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_0(l, qlm));
		ctrl_0.s.agc_gain = 0x2;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_0(l, qlm), ctrl_0.u64);

		/* 4. Write GSERX_LANEx_RX_LOOP_CTRL
		 * bit<8> lctrl_men = 1'b1
		 * bit<0> cdr_en_byp = 1'b1
		 */
		loop_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_LOOP_CTRL(l, qlm));
		loop_ctrl.s.cfg_rx_lctrl = loop_ctrl.s.cfg_rx_lctrl | 0x101;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_LOOP_CTRL(l, qlm), loop_ctrl.u64);

		/* 5. Write GSERX_LANEx_PWR_CTRL = 0x0040 (var "lanex_pwr_ctrl" with
		 * following bits set)
		 * bit<6> RX_LCTRL_OVRRD_EN = 1'b1
		 * all other bits cleared.
		 */
		lanex_pwr_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PWR_CTRL(l, qlm));
		lanex_pwr_ctrl.s.rx_lctrl_ovrrd_en = 1;
		csr_wr_node(node, CVMX_GSERX_LANEX_PWR_CTRL(l, qlm), lanex_pwr_ctrl.u64);

		/* --Setting AGC in manual mode and configuring CTLE-- */
		rx_cfg_5.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_CFG_5(l, qlm));
		rx_cfg_5.s.rx_agc_men_ovrrd_val = 1;
		rx_cfg_5.s.rx_agc_men_ovrrd_en = 1;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_CFG_5(l, qlm), rx_cfg_5.u64);

		ctle_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_CTLE_CTRL(l, qlm));
		ctle_ctrl.s.pcs_sds_rx_ctle_zero = ctle_zero;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_CTLE_CTRL(l, qlm), ctle_ctrl.u64);

		rx_cfg_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_CFG_2(l, qlm));
		rx_cfg_2.s.rx_sds_rx_agc_mval = (agc_pre_ctle << 4) | agc_post_ctle;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_CFG_2(l, qlm), rx_cfg_2.u64);
	}
	return 0;
}

/**
 * Some QLM speeds need to override the default tuning parameters
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param baud_mhz Desired speed in MHz
 * @param lane     Lane the apply the tuning parameters
 * @param tx_swing Voltage swing.  The higher the value the lower the voltage,
 *		   the default value is 7.
 * @param tx_pre   pre-cursor pre-emphasis
 * @param tx_post  post-cursor pre-emphasis.
 * @param tx_gain   Transmit gain. Range 0-7
 * @param tx_vboost Transmit voltage boost. Range 0-1
 */
void octeon_qlm_tune_per_lane_v3(int node, int qlm, int baud_mhz, int lane, int tx_swing,
				 int tx_pre, int tx_post, int tx_gain, int tx_vboost)
{
	cvmx_gserx_cfg_t gserx_cfg;
	cvmx_gserx_lanex_tx_cfg_0_t tx_cfg0;
	cvmx_gserx_lanex_tx_pre_emphasis_t pre_emphasis;
	cvmx_gserx_lanex_tx_cfg_1_t tx_cfg1;
	cvmx_gserx_lanex_tx_cfg_3_t tx_cfg3;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;
	cvmx_gserx_lanex_pcs_ctlifc_0_t pcs_ctlifc_0;
	cvmx_gserx_lanex_pcs_ctlifc_2_t pcs_ctlifc_2;
	int bgx, lmac;

	/* Do not apply QLM tuning to PCIe and KR interfaces. */
	gserx_cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));
	if (gserx_cfg.s.pcie)
		return;

	/* Apply the QLM tuning only to cn73xx and cn78xx models only */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		bgx = (qlm < 2) ? qlm : (qlm - 2);
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		bgx = (qlm < 4) ? (qlm - 2) : 2;
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		bgx = 0;
	else
		return;

	if ((OCTEON_IS_MODEL(OCTEON_CN73XX) && qlm == 6) ||
	    (OCTEON_IS_MODEL(OCTEON_CNF75XX) && qlm == 5))
		lmac = 2;
	else
		lmac = lane;

	/* No need to tune 10G-KR and 40G-KR interfaces */
	pmd_control.u64 = csr_rd_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(lmac, bgx));
	if (pmd_control.s.train_en)
		return;

	if (tx_pre != -1 && tx_post == -1)
		tx_post = 0;

	if (tx_post != -1 && tx_pre == -1)
		tx_pre = 0;

	/* Check tuning constraints */
	if (tx_swing < -1 || tx_swing > 25) {
		printf("ERROR: N%d:QLM%d: Lane %d: Invalid TX_SWING(%d). TX_SWING must be <= 25.\n",
		       node, qlm, lane, tx_swing);
		return;
	}

	if (tx_pre < -1 || tx_pre > 10) {
		printf("ERROR: N%d:QLM%d: Lane %d: Invalid TX_PRE(%d). TX_PRE must be <= 10.\n",
		       node, qlm, lane, tx_swing);
		return;
	}

	if (tx_post < -1 || tx_post > 31) {
		printf("ERROR: N%d:QLM%d: Lane %d: Invalid TX_POST(%d). TX_POST must be <= 15.\n",
		       node, qlm, lane, tx_swing);
		return;
	}

	if (tx_pre >= 0 && tx_post >= 0 && tx_swing >= 0 &&
	    tx_pre + tx_post - tx_swing > 2) {
		printf("ERROR: N%d.QLM%d: Lane %d: TX_PRE(%d) + TX_POST(%d) - TX_SWING(%d) must be <= 2\n",
		       node, qlm, lane, tx_pre, tx_post, tx_swing);
		return;
	}

	if (tx_pre >= 0 && tx_post >= 0 && tx_swing >= 0 &&
	    tx_pre + tx_post + tx_swing > 35) {
		printf("ERROR: N%d.QLM%d: Lane %d: TX_PRE(%d) + TX_POST(%d) + TX_SWING(%d) must be <= 35\n",
		       node, qlm, lane, tx_pre, tx_post, tx_swing);
		return;
	}

	if (tx_gain < -1 || tx_gain > 7) {
		printf("ERROR: N%d.QLM%d: Lane %d: Invalid TX_GAIN(%d). TX_GAIN must be between 0 and 7\n",
		       node, qlm, lane, tx_gain);
		return;
	}

	if (tx_vboost < -1 || tx_vboost > 1) {
		printf("ERROR: N%d.QLM%d: Lane %d: Invalid TX_VBOOST(%d).  TX_VBOOST must be 0 or 1.\n",
		       node, qlm, lane, tx_vboost);
		return;
	}

	debug("N%d.QLM%d: Lane %d: TX_SWING=%d, TX_PRE=%d, TX_POST=%d, TX_GAIN=%d, TX_VBOOST=%d\n",
	      node, qlm, lane, tx_swing, tx_pre, tx_post, tx_gain, tx_vboost);

	/* Complete the Tx swing and Tx equilization programming */
	/* 1) Enable Tx swing and Tx emphasis overrides */
	tx_cfg1.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_TX_CFG_1(lane, qlm));
	tx_cfg1.s.tx_swing_ovrrd_en = (tx_swing != -1);
	tx_cfg1.s.tx_premptap_ovrrd_val = (tx_pre != -1) && (tx_post != -1);
	tx_cfg1.s.tx_vboost_en_ovrrd_en = (tx_vboost != -1); /* Vboost override */
	;
	csr_wr_node(node, CVMX_GSERX_LANEX_TX_CFG_1(lane, qlm), tx_cfg1.u64);
	/* 2) Program the Tx swing and Tx emphasis Pre-cursor and Post-cursor values */
	/* CFG_TX_PREMPTAP[8:4] = Lane X's TX post-cursor value (C+1) */
	/* CFG_TX_PREMPTAP[3:0] = Lane X's TX pre-cursor value (C-1) */
	if (tx_swing != -1) {
		tx_cfg0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_TX_CFG_0(lane, qlm));
		tx_cfg0.s.cfg_tx_swing = tx_swing;
		csr_wr_node(node, CVMX_GSERX_LANEX_TX_CFG_0(lane, qlm), tx_cfg0.u64);
	}

	if ((tx_pre != -1) && (tx_post != -1)) {
		pre_emphasis.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_TX_PRE_EMPHASIS(lane, qlm));
		pre_emphasis.s.cfg_tx_premptap = (tx_post << 4) | tx_pre;
		csr_wr_node(node, CVMX_GSERX_LANEX_TX_PRE_EMPHASIS(lane, qlm), pre_emphasis.u64);
	}

	/* Apply TX gain settings */
	if (tx_gain != -1) {
		tx_cfg3.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_TX_CFG_3(lane, qlm));
		tx_cfg3.s.pcs_sds_tx_gain = tx_gain;
		csr_wr_node(node, CVMX_GSERX_LANEX_TX_CFG_3(lane, qlm), tx_cfg3.u64);
	}

	/* Apply TX vboot settings */
	if (tx_vboost != -1) {
		tx_cfg3.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_TX_CFG_3(lane, qlm));
		tx_cfg3.s.cfg_tx_vboost_en = tx_vboost;
		csr_wr_node(node, CVMX_GSERX_LANEX_TX_CFG_3(lane, qlm), tx_cfg3.u64);
	}

	/* 3) Program override for the Tx coefficient request */
	pcs_ctlifc_0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(lane, qlm));
	if (((tx_pre != -1) && (tx_post != -1)) || (tx_swing != -1))
		pcs_ctlifc_0.s.cfg_tx_coeff_req_ovrrd_val = 0x1;
	if (tx_vboost != -1)
		pcs_ctlifc_0.s.cfg_tx_vboost_en_ovrrd_val = 1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(lane, qlm), pcs_ctlifc_0.u64);

	/* 4) Enable the Tx coefficient request override enable */
	pcs_ctlifc_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	if (((tx_pre != -1) && (tx_post != -1)) || (tx_swing != -1))
		pcs_ctlifc_2.s.cfg_tx_coeff_req_ovrrd_en = 0x1;
	if (tx_vboost != -1)
		pcs_ctlifc_2.s.cfg_tx_vboost_en_ovrrd_en = 1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), pcs_ctlifc_2.u64);

	/* 5) Issue a Control Interface Configuration Override request to start the Tx equalizer */
	pcs_ctlifc_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	pcs_ctlifc_2.s.ctlifc_ovrrd_req = 0x1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), pcs_ctlifc_2.u64);

	/* 6) Wait 1 ms for the request to complete */
	udelay(1000);

	/* Steps 7 & 8 required for subsequent Tx swing and Tx equilization adjustment */
	/* 7) Disable the Tx coefficient request override enable */
	pcs_ctlifc_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	pcs_ctlifc_2.s.cfg_tx_coeff_req_ovrrd_en = 0;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), pcs_ctlifc_2.u64);
	/* 8) Issue a Control Interface Configuration Override request */
	pcs_ctlifc_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
	pcs_ctlifc_2.s.ctlifc_ovrrd_req = 0x1;
	csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), pcs_ctlifc_2.u64);
}

/**
 * Some QLM speeds need to override the default tuning parameters
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param baud_mhz Desired speed in MHz
 * @param tx_swing Voltage swing.  The higher the value the lower the voltage,
 *		   the default value is 7.
 * @param tx_premptap bits [0:3] pre-cursor pre-emphasis, bits[4:8] post-cursor
 *		      pre-emphasis.
 * @param tx_gain   Transmit gain. Range 0-7
 * @param tx_vboost Transmit voltage boost. Range 0-1
 *
 */
void octeon_qlm_tune_v3(int node, int qlm, int baud_mhz, int tx_swing, int tx_premptap, int tx_gain,
			int tx_vboost)
{
	int lane;
	int num_lanes = cvmx_qlm_get_lanes(qlm);

	for (lane = 0; lane < num_lanes; lane++) {
		int tx_pre = (tx_premptap == -1) ? -1 : tx_premptap & 0xf;
		int tx_post = (tx_premptap == -1) ? -1 : (tx_premptap >> 4) & 0x1f;

		octeon_qlm_tune_per_lane_v3(node, qlm, baud_mhz, lane, tx_swing, tx_pre, tx_post,
					    tx_gain, tx_vboost);
	}
}

/**
 * Some QLMs need to override the default pre-ctle for low loss channels.
 *
 * @param node     Node to configure
 * @param qlm      QLM to configure
 * @param pre_ctle pre-ctle settings for low loss channels
 */
void octeon_qlm_set_channel_v3(int node, int qlm, int pre_ctle)
{
	cvmx_gserx_lane_vma_fine_ctrl_2_t lane_vma_fine_ctrl_2;

	lane_vma_fine_ctrl_2.u64 = csr_rd_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_2(qlm));
	lane_vma_fine_ctrl_2.s.rx_prectle_gain_min_fine = pre_ctle;
	csr_wr_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_2(qlm), lane_vma_fine_ctrl_2.u64);
}

static void __qlm_init_errata_20844(int node, int qlm)
{
	int lane;

	/* Only applies to CN78XX pass 1.x */
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
		return;

	/* Errata GSER-20844: Electrical Idle logic can coast
	 * 1) After the link first comes up write the following
	 * register on each lane to prevent the application logic
	 * from stomping on the Coast inputs. This is a one time write,
	 * or if you prefer you could put it in the link up loop and
	 * write it every time the link comes up.
	 * 1a) Then write GSER(0..13)_LANE(0..3)_PCS_CTLIFC_2
	 * Set CTLIFC_OVRRD_REQ (later)
	 * Set CFG_RX_CDR_COAST_REQ_OVRRD_EN
	 * Its not clear if #1 and #1a can be combined, lets try it
	 * this way first.
	 */
	for (lane = 0; lane < 4; lane++) {
		cvmx_gserx_lanex_rx_misc_ovrrd_t misc_ovrrd;
		cvmx_gserx_lanex_pcs_ctlifc_2_t ctlifc_2;

		ctlifc_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
		ctlifc_2.s.cfg_rx_cdr_coast_req_ovrrd_en = 1;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), ctlifc_2.u64);

		misc_ovrrd.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm));
		misc_ovrrd.s.cfg_rx_eie_det_ovrrd_en = 1;
		misc_ovrrd.s.cfg_rx_eie_det_ovrrd_val = 0;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm), misc_ovrrd.u64);

		udelay(1);

		misc_ovrrd.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm));
		misc_ovrrd.s.cfg_rx_eie_det_ovrrd_en = 1;
		misc_ovrrd.s.cfg_rx_eie_det_ovrrd_val = 1;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm), misc_ovrrd.u64);
		ctlifc_2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm));
		ctlifc_2.s.ctlifc_ovrrd_req = 1;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(lane, qlm), ctlifc_2.u64);
	}
}

/** CN78xx reference clock register settings */
struct refclk_settings_cn78xx {
	bool valid; /** Reference clock speed supported */
	union cvmx_gserx_pll_px_mode_0 mode_0;
	union cvmx_gserx_pll_px_mode_1 mode_1;
	union cvmx_gserx_lane_px_mode_0 pmode_0;
	union cvmx_gserx_lane_px_mode_1 pmode_1;
};

/** Default reference clock for various modes */
static const u8 def_ref_clk_cn78xx[R_NUM_LANE_MODES] = { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 };

/**
 * This data structure stores the reference clock for each mode for each QLM.
 *
 * It is indexed first by the node number, then the QLM number and then the
 * lane mode.  It is initialized to the default values.
 */
static u8 ref_clk_cn78xx[CVMX_MAX_NODES][8][R_NUM_LANE_MODES] = {
	{ { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 } },
	{ { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 } },
	{ { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 } },
	{ { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 },
	  { 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 1 } }
};

/**
 * This data structure contains the register values for the cn78xx PLLs
 * It is indexed first by the reference clock and second by the mode.
 * Note that not all combinations are supported.
 */
static const struct refclk_settings_cn78xx refclk_settings_cn78xx[R_NUM_LANE_MODES][4] = {
	{   /* 0	R_2_5G_REFCLK100 */
	{ /* 100MHz reference clock */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x4, .pll_rloop = 0x3, .pll_pcs_div = 0x5 },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x19 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 125MHz reference clock */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0x5 },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x1,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 156.25MHz reference clock */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0x5 },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x10 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{
		  /* 161.1328125MHz reference clock */
		  .valid = false,
	  } },
	{
		/* 1	R_5G_REFCLK100 */
		{ /* 100MHz reference clock */
		  .valid = true,
		  .mode_0.s = { .pll_icp = 0x4, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
		  .mode_1.s = { .pll_16p5en = 0x0,
				.pll_cpadj = 0x2,
				.pll_pcie3en = 0x0,
				.pll_opr = 0x0,
				.pll_div = 0x19 },
		  .pmode_0.s = { .ctle = 0x0,
				 .pcie = 0x1,
				 .tx_ldiv = 0x0,
				 .rx_ldiv = 0x0,
				 .srate = 0x0,
				 .tx_mode = 0x3,
				 .rx_mode = 0x3 },
		  .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
				 .vma_mm = 0x0,
				 .cdr_fgain = 0xa,
				 .ph_acc_adj = 0x14 } },
		{ /* 125MHz reference clock */
		  .valid = true,
		  .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
		  .mode_1.s = { .pll_16p5en = 0x0,
				.pll_cpadj = 0x1,
				.pll_pcie3en = 0x0,
				.pll_opr = 0x0,
				.pll_div = 0x14 },
		  .pmode_0.s = { .ctle = 0x0,
				 .pcie = 0x1,
				 .tx_ldiv = 0x0,
				 .rx_ldiv = 0x0,
				 .srate = 0x0,
				 .tx_mode = 0x3,
				 .rx_mode = 0x3 },
		  .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
				 .vma_mm = 0x0,
				 .cdr_fgain = 0xa,
				 .ph_acc_adj = 0x14 } },
		{ /* 156.25MHz reference clock */
		  .valid = true,
		  .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
		  .mode_1.s = { .pll_16p5en = 0x0,
				.pll_cpadj = 0x2,
				.pll_pcie3en = 0x0,
				.pll_opr = 0x0,
				.pll_div = 0x10 },
		  .pmode_0.s = { .ctle = 0x0,
				 .pcie = 0x1,
				 .tx_ldiv = 0x0,
				 .rx_ldiv = 0x0,
				 .srate = 0x0,
				 .tx_mode = 0x3,
				 .rx_mode = 0x3 },
		  .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
				 .vma_mm = 0x0,
				 .cdr_fgain = 0xa,
				 .ph_acc_adj = 0x14 } },
		{
			/* 161.1328125MHz reference clock */
			.valid = false,
		},
	},
	{   /* 2	R_8G_REFCLK100 */
	{ /* 100MHz reference clock */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x5, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x1,
			  .pll_opr = 0x1,
			  .pll_div = 0x28 },
	    .pmode_0.s = { .ctle = 0x3,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xb,
			   .ph_acc_adj = 0x23 } },
	{ /* 125MHz reference clock */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x2, .pll_rloop = 0x5, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x1,
			  .pll_pcie3en = 0x1,
			  .pll_opr = 0x1,
			  .pll_div = 0x20 },
	    .pmode_0.s = { .ctle = 0x3,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xb,
			   .ph_acc_adj = 0x23 } },
	{ /* 156.25MHz reference clock not supported */
	    .valid = false } },
	{
		/* 3	R_125G_REFCLK15625_KX */
		{ /* 100MHz reference */
		  .valid = true,
		  .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x28 },
		  .mode_1.s = { .pll_16p5en = 0x1,
				.pll_cpadj = 0x2,
				.pll_pcie3en = 0x0,
				.pll_opr = 0x0,
				.pll_div = 0x19 },
		  .pmode_0.s = { .ctle = 0x0,
				 .pcie = 0x0,
				 .tx_ldiv = 0x2,
				 .rx_ldiv = 0x2,
				 .srate = 0x0,
				 .tx_mode = 0x3,
				 .rx_mode = 0x3 },
		  .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
				 .vma_mm = 0x1,
				 .cdr_fgain = 0xc,
				 .ph_acc_adj = 0x1e } },
		{ /* 125MHz reference */
		  .valid = true,
		  .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x28 },
		  .mode_1.s = { .pll_16p5en = 0x1,
				.pll_cpadj = 0x2,
				.pll_pcie3en = 0x0,
				.pll_opr = 0x0,
				.pll_div = 0x14 },
		  .pmode_0.s = { .ctle = 0x0,
				 .pcie = 0x0,
				 .tx_ldiv = 0x2,
				 .rx_ldiv = 0x2,
				 .srate = 0x0,
				 .tx_mode = 0x3,
				 .rx_mode = 0x3 },
		  .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
				 .vma_mm = 0x1,
				 .cdr_fgain = 0xc,
				 .ph_acc_adj = 0x1e } },
		{ /* 156.25MHz reference */
		  .valid = true,
		  .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x28 },
		  .mode_1.s = { .pll_16p5en = 0x1,
				.pll_cpadj = 0x3,
				.pll_pcie3en = 0x0,
				.pll_opr = 0x0,
				.pll_div = 0x10 },
		  .pmode_0.s = { .ctle = 0x0,
				 .pcie = 0x0,
				 .tx_ldiv = 0x2,
				 .rx_ldiv = 0x2,
				 .srate = 0x0,
				 .tx_mode = 0x3,
				 .rx_mode = 0x3 },
		  .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
				 .vma_mm = 0x1,
				 .cdr_fgain = 0xc,
				 .ph_acc_adj = 0x1e } },
		{
			/* 161.1328125MHz reference clock */
			.valid = false,
		},
	},
	{   /* 4	R_3125G_REFCLK15625_XAUI */
	{ /* 100MHz reference */
	    .valid = false },
	{ /* 125MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x14 },
	    .mode_1.s = { .pll_16p5en = 0x1,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x19 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } },
	{ /* 156.25MHz reference, default */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x14 },
	    .mode_1.s = { .pll_16p5en = 0x1,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } },
	{
		  /* 161.1328125MHz reference clock */
		  .valid = false,
	  } },
	{   /* 5	R_103125G_REFCLK15625_KR */
	{ /* 100MHz reference */
	    .valid = false },
	{ /* 125MHz reference */
	    .valid = false },
	{ /* 156.25MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x5, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x1,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x1,
			  .pll_div = 0x21 },
	    .pmode_0.s = { .ctle = 0x3,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x1,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0xf } },
	{ /* 161.1328125 reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x5, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x1,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x1,
			  .pll_div = 0x20 },
	    .pmode_0.s = { .ctle = 0x3,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x1,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0xf } } },
	{   /* 6	R_125G_REFCLK15625_SGMII */
	{ /* 100MHz reference clock */
	    .valid = 1,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x28 },
	    .mode_1.s = { .pll_16p5en = 0x1,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x19 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x2,
			   .rx_ldiv = 0x2,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } },
	{ /* 125MHz reference clock */
	    .valid = 1,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x28 },
	    .mode_1.s = { .pll_16p5en = 0x1,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x2,
			   .rx_ldiv = 0x2,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } },
	{ /* 156.25MHz reference clock */
	    .valid = 1,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0x28 },
	    .mode_1.s = { .pll_16p5en = 0x1,
			  .pll_cpadj = 0x3,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x10 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x2,
			   .rx_ldiv = 0x2,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } } },
	{   /* 7	R_5G_REFCLK15625_QSGMII */
	{ /* 100MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x4, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0, .pll_cpadj = 0x2, .pll_pcie3en = 0x0,
			  .pll_div = 0x19 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } },
	{ /* 125MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0, .pll_cpadj = 0x1, .pll_pcie3en = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } },
	{ /* 156.25MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0, .pll_cpadj = 0x2, .pll_pcie3en = 0x0,
			  .pll_div = 0x10 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xc,
			   .ph_acc_adj = 0x1e } },
	{
		  /* 161.1328125MHz reference clock */
		  .valid = false,
	  } },
	{   /* 8	R_625G_REFCLK15625_RXAUI */
	{ /* 100MHz reference */
	    .valid = false },
	{ /* 125MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x19 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 156.25MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 161.1328125 reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x1, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } } },
	{   /* 9	R_2_5G_REFCLK125 */
	{ /* 100MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x4, .pll_rloop = 0x3, .pll_pcs_div = 0x5 },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x19 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 125MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0x5 },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x1,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 156,25MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0x5 },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x10 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x1,
			   .rx_ldiv = 0x1,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x1,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{
		  /* 161.1328125MHz reference clock */
		  .valid = false,
	  } },
	{   /* 0xa	R_5G_REFCLK125 */
	{ /* 100MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x4, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x19 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 125MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x1,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x14 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{ /* 156.25MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x3, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x0,
			  .pll_opr = 0x0,
			  .pll_div = 0x10 },
	    .pmode_0.s = { .ctle = 0x0,
			   .pcie = 0x1,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xa,
			   .ph_acc_adj = 0x14 } },
	{
		  /* 161.1328125MHz reference clock */
		  .valid = false,
	  } },
	{   /* 0xb	R_8G_REFCLK125 */
	{ /* 100MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x3, .pll_rloop = 0x5, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x2,
			  .pll_pcie3en = 0x1,
			  .pll_opr = 0x1,
			  .pll_div = 0x28 },
	    .pmode_0.s = { .ctle = 0x3,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xb,
			   .ph_acc_adj = 0x23 } },
	{ /* 125MHz reference */
	    .valid = true,
	    .mode_0.s = { .pll_icp = 0x2, .pll_rloop = 0x5, .pll_pcs_div = 0xa },
	    .mode_1.s = { .pll_16p5en = 0x0,
			  .pll_cpadj = 0x1,
			  .pll_pcie3en = 0x1,
			  .pll_opr = 0x1,
			  .pll_div = 0x20 },
	    .pmode_0.s = { .ctle = 0x3,
			   .pcie = 0x0,
			   .tx_ldiv = 0x0,
			   .rx_ldiv = 0x0,
			   .srate = 0x0,
			   .tx_mode = 0x3,
			   .rx_mode = 0x3 },
	    .pmode_1.s = { .vma_fine_cfg_sel = 0x0,
			   .vma_mm = 0x0,
			   .cdr_fgain = 0xb,
			   .ph_acc_adj = 0x23 } },
	{ /* 156.25MHz reference */
	    .valid = false },
	{
		  /* 161.1328125MHz reference clock */
		  .valid = false,
	  } }
};

/**
 * Set a non-standard reference clock for a node, qlm and lane mode.
 *
 * @INTERNAL
 *
 * @param node		node number the reference clock is used with
 * @param qlm		qlm number the reference clock is hooked up to
 * @param lane_mode	current lane mode selected for the QLM
 * @param ref_clk_sel	0 = 100MHz, 1 = 125MHz, 2 = 156.25MHz,
 *			3 = 161.1328125MHz
 *
 * Return: 0 for success or -1 if the reference clock selector is not supported
 *
 * NOTE: This must be called before __qlm_setup_pll_cn78xx.
 */
static int __set_qlm_ref_clk_cn78xx(int node, int qlm, int lane_mode, int ref_clk_sel)
{
	if (ref_clk_sel > 3 || ref_clk_sel < 0 ||
	    !refclk_settings_cn78xx[lane_mode][ref_clk_sel].valid) {
		debug("%s: Invalid reference clock %d for lane mode %d for node %d, QLM %d\n",
		      __func__, ref_clk_sel, lane_mode, node, qlm);
		return -1;
	}
	debug("%s(%d, %d, 0x%x, %d)\n", __func__, node, qlm, lane_mode, ref_clk_sel);
	ref_clk_cn78xx[node][qlm][lane_mode] = ref_clk_sel;
	return 0;
}

/**
 * KR - Inverted Tx Coefficient Direction Change.  Changing Pre & Post Tap inc/dec direction
 *
 *
 * @INTERNAL
 *
 * @param node	Node number to configure
 * @param qlm	QLM number to configure
 */
static void __qlm_kr_inc_dec_gser26636(int node, int qlm)
{
	cvmx_gserx_rx_txdir_ctrl_1_t rx_txdir_ctrl;

	/* Apply workaround for Errata GSER-26636,
	 * KR training coefficient update inverted
	 */
	rx_txdir_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_RX_TXDIR_CTRL_1(qlm));
	rx_txdir_ctrl.s.rx_precorr_chg_dir = 1;
	rx_txdir_ctrl.s.rx_tap1_chg_dir = 1;
	csr_wr_node(node, CVMX_GSERX_RX_TXDIR_CTRL_1(qlm), rx_txdir_ctrl.u64);
}

/**
 * Updating the RX EQ settings to support wider temperature range
 * @INTERNAL
 *
 * @param node	Node number to configure
 * @param qlm	QLM number to configure
 */
static void __qlm_rx_eq_temp_gser27140(int node, int qlm)
{
	int lane;
	int num_lanes = cvmx_qlm_get_lanes(qlm);
	cvmx_gserx_lanex_rx_valbbd_ctrl_0_t rx_valbbd_ctrl_0;
	cvmx_gserx_lane_vma_fine_ctrl_2_t lane_vma_fine_ctrl_2;
	cvmx_gserx_lane_vma_fine_ctrl_0_t lane_vma_fine_ctrl_0;
	cvmx_gserx_rx_txdir_ctrl_1_t rx_txdir_ctrl_1;
	cvmx_gserx_eq_wait_time_t eq_wait_time;
	cvmx_gserx_rx_txdir_ctrl_2_t rx_txdir_ctrl_2;
	cvmx_gserx_rx_txdir_ctrl_0_t rx_txdir_ctrl_0;

	for (lane = 0; lane < num_lanes; lane++) {
		rx_valbbd_ctrl_0.u64 =
			csr_rd_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_0(lane, qlm));
		rx_valbbd_ctrl_0.s.agc_gain = 3;
		rx_valbbd_ctrl_0.s.dfe_gain = 2;
		csr_wr_node(node, CVMX_GSERX_LANEX_RX_VALBBD_CTRL_0(lane, qlm),
			    rx_valbbd_ctrl_0.u64);
	}

	/* do_pre_ctle_limits_work_around: */
	lane_vma_fine_ctrl_2.u64 = csr_rd_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_2(qlm));
	//lane_vma_fine_ctrl_2.s.rx_prectle_peak_max_fine = 11;
	lane_vma_fine_ctrl_2.s.rx_prectle_gain_max_fine = 11;
	//lane_vma_fine_ctrl_2.s.rx_prectle_peak_min_fine = 6;
	lane_vma_fine_ctrl_2.s.rx_prectle_gain_min_fine = 6;
	csr_wr_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_2(qlm), lane_vma_fine_ctrl_2.u64);

	/* do_inc_dec_thres_work_around: */
	rx_txdir_ctrl_0.u64 = csr_rd_node(node, CVMX_GSERX_RX_TXDIR_CTRL_0(qlm));
	rx_txdir_ctrl_0.s.rx_boost_hi_thrs = 11;
	rx_txdir_ctrl_0.s.rx_boost_lo_thrs = 4;
	rx_txdir_ctrl_0.s.rx_boost_hi_val = 15;
	csr_wr_node(node, CVMX_GSERX_RX_TXDIR_CTRL_0(qlm), rx_txdir_ctrl_0.u64);

	/* do_sdll_iq_work_around: */
	lane_vma_fine_ctrl_0.u64 = csr_rd_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_0(qlm));
	lane_vma_fine_ctrl_0.s.rx_sdll_iq_max_fine = 14;
	lane_vma_fine_ctrl_0.s.rx_sdll_iq_min_fine = 8;
	lane_vma_fine_ctrl_0.s.rx_sdll_iq_step_fine = 2;

	/* do_vma_window_work_around_2: */
	lane_vma_fine_ctrl_0.s.vma_window_wait_fine = 5;
	lane_vma_fine_ctrl_0.s.lms_wait_time_fine = 5;

	csr_wr_node(node, CVMX_GSERX_LANE_VMA_FINE_CTRL_0(qlm), lane_vma_fine_ctrl_0.u64);

	/* Set dfe_tap_1_lo_thres_val: */
	rx_txdir_ctrl_1.u64 = csr_rd_node(node, CVMX_GSERX_RX_TXDIR_CTRL_1(qlm));
	rx_txdir_ctrl_1.s.rx_tap1_lo_thrs = 8;
	rx_txdir_ctrl_1.s.rx_tap1_hi_thrs = 0x17;
	csr_wr_node(node, CVMX_GSERX_RX_TXDIR_CTRL_1(qlm), rx_txdir_ctrl_1.u64);

	/* do_rxeq_wait_cnt_work_around: */
	eq_wait_time.u64 = csr_rd_node(node, CVMX_GSERX_EQ_WAIT_TIME(qlm));
	eq_wait_time.s.rxeq_wait_cnt = 6;
	csr_wr_node(node, CVMX_GSERX_EQ_WAIT_TIME(qlm), eq_wait_time.u64);

	/* do_write_rx_txdir_precorr_thresholds: */
	rx_txdir_ctrl_2.u64 = csr_rd_node(node, CVMX_GSERX_RX_TXDIR_CTRL_2(qlm));
	rx_txdir_ctrl_2.s.rx_precorr_hi_thrs = 0xc0;
	rx_txdir_ctrl_2.s.rx_precorr_lo_thrs = 0x40;
	csr_wr_node(node, CVMX_GSERX_RX_TXDIR_CTRL_2(qlm), rx_txdir_ctrl_2.u64);
}

/* Errata GSER-26150: 10G PHY PLL Temperature Failure
 * This workaround must be completed after the final deassertion of
 * GSERx_PHY_CTL[PHY_RESET]
 */
static int __qlm_errata_gser_26150(int node, int qlm, int is_pcie)
{
	int num_lanes = 4;
	int i;
	cvmx_gserx_glbl_pll_cfg_3_t pll_cfg_3;
	cvmx_gserx_glbl_misc_config_1_t misc_config_1;

	/* PCIe only requires the LC-VCO parameters to be updated */
	if (is_pcie) {
		/* Update PLL parameters */
		/* Step 1: Set GSER()_GLBL_PLL_CFG_3[PLL_VCTRL_SEL_LCVCO_VAL] = 0x2, and
		 * GSER()_GLBL_PLL_CFG_3[PCS_SDS_PLL_VCO_AMP] = 0
		 */
		pll_cfg_3.u64 = csr_rd_node(node, CVMX_GSERX_GLBL_PLL_CFG_3(qlm));
		pll_cfg_3.s.pcs_sds_pll_vco_amp = 0;
		pll_cfg_3.s.pll_vctrl_sel_lcvco_val = 2;
		csr_wr_node(node, CVMX_GSERX_GLBL_PLL_CFG_3(qlm), pll_cfg_3.u64);

		/* Step 2: Set GSER()_GLBL_MISC_CONFIG_1[PCS_SDS_TRIM_CHP_REG] = 0x2. */
		misc_config_1.u64 = csr_rd_node(node, CVMX_GSERX_GLBL_MISC_CONFIG_1(qlm));
		misc_config_1.s.pcs_sds_trim_chp_reg = 2;
		csr_wr_node(node, CVMX_GSERX_GLBL_MISC_CONFIG_1(qlm), misc_config_1.u64);
		return 0;
	}

	/* Applying this errata twice causes problems */
	pll_cfg_3.u64 = csr_rd_node(node, CVMX_GSERX_GLBL_PLL_CFG_3(qlm));
	if (pll_cfg_3.s.pll_vctrl_sel_lcvco_val == 0x2)
		return 0;

	/* (GSER-26150) 10 Gb temperature excursions can cause lock failure */
	/* Change the calibration point of the VCO at start up to shift some
	 * available range of the VCO from -deltaT direction to the +deltaT
	 * ramp direction allowing a greater range of VCO temperatures before
	 * experiencing the failure.
	 */

	/* Check for DLMs on CN73XX and CNF75XX */
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) && (qlm == 5 || qlm == 6))
		num_lanes = 2;

	/* Put PHY in P2 Power-down state  Need to Power down all lanes in a
	 * QLM/DLM to force PHY to P2 state
	 */
	for (i = 0; i < num_lanes; i++) {
		cvmx_gserx_lanex_pcs_ctlifc_0_t ctlifc0;
		cvmx_gserx_lanex_pcs_ctlifc_1_t ctlifc1;
		cvmx_gserx_lanex_pcs_ctlifc_2_t ctlifc2;

		/* Step 1: Set Set GSER()_LANE(lane_n)_PCS_CTLIFC_0[CFG_TX_PSTATE_REQ_OVERRD_VAL]
		 * = 0x3
		 * Select P2 power state for Tx lane
		 */
		ctlifc0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(i, qlm));
		ctlifc0.s.cfg_tx_pstate_req_ovrrd_val = 0x3;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(i, qlm), ctlifc0.u64);
		/* Step 2: Set GSER()_LANE(lane_n)_PCS_CTLIFC_1[CFG_RX_PSTATE_REQ_OVERRD_VAL]
		 * = 0x3
		 * Select P2 power state for Rx lane
		 */
		ctlifc1.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_1(i, qlm));
		ctlifc1.s.cfg_rx_pstate_req_ovrrd_val = 0x3;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_1(i, qlm), ctlifc1.u64);
		/* Step 3: Set GSER()_LANE(lane_n)_PCS_CTLIFC_2[CFG_TX_PSTATE_REQ_OVRRD_EN] = 1
		 * Enable Tx power state override and Set
		 * GSER()_LANE(lane_n)_PCS_CTLIFC_2[CFG_RX_PSTATE_REQ_OVRRD_EN] = 1
		 * Enable Rx power state override
		 */
		ctlifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm));
		ctlifc2.s.cfg_tx_pstate_req_ovrrd_en = 0x1;
		ctlifc2.s.cfg_rx_pstate_req_ovrrd_en = 0x1;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm), ctlifc2.u64);
		/* Step 4: Set GSER()_LANE(lane_n)_PCS_CTLIFC_2[CTLIFC_OVRRD_REQ] = 1
		 * Start the CTLIFC override state machine
		 */
		ctlifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm));
		ctlifc2.s.ctlifc_ovrrd_req = 0x1;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm), ctlifc2.u64);
	}

	/* Update PLL parameters */
	/* Step 5: Set GSER()_GLBL_PLL_CFG_3[PLL_VCTRL_SEL_LCVCO_VAL] = 0x2, and
	 * GSER()_GLBL_PLL_CFG_3[PCS_SDS_PLL_VCO_AMP] = 0
	 */
	pll_cfg_3.u64 = csr_rd_node(node, CVMX_GSERX_GLBL_PLL_CFG_3(qlm));
	pll_cfg_3.s.pcs_sds_pll_vco_amp = 0;
	pll_cfg_3.s.pll_vctrl_sel_lcvco_val = 2;
	csr_wr_node(node, CVMX_GSERX_GLBL_PLL_CFG_3(qlm), pll_cfg_3.u64);

	/* Step 6: Set GSER()_GLBL_MISC_CONFIG_1[PCS_SDS_TRIM_CHP_REG] = 0x2. */
	misc_config_1.u64 = csr_rd_node(node, CVMX_GSERX_GLBL_MISC_CONFIG_1(qlm));
	misc_config_1.s.pcs_sds_trim_chp_reg = 2;
	csr_wr_node(node, CVMX_GSERX_GLBL_MISC_CONFIG_1(qlm), misc_config_1.u64);

	/* Wake up PHY and transition to P0 Power-up state to bring-up the lanes,
	 * need to wake up all PHY lanes
	 */
	for (i = 0; i < num_lanes; i++) {
		cvmx_gserx_lanex_pcs_ctlifc_0_t ctlifc0;
		cvmx_gserx_lanex_pcs_ctlifc_1_t ctlifc1;
		cvmx_gserx_lanex_pcs_ctlifc_2_t ctlifc2;
		/* Step 7: Set GSER()_LANE(lane_n)_PCS_CTLIFC_0[CFG_TX_PSTATE_REQ_OVERRD_VAL] = 0x0
		 * Select P0 power state for Tx lane
		 */
		ctlifc0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(i, qlm));
		ctlifc0.s.cfg_tx_pstate_req_ovrrd_val = 0x0;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_0(i, qlm), ctlifc0.u64);
		/* Step 8: Set GSER()_LANE(lane_n)_PCS_CTLIFC_1[CFG_RX_PSTATE_REQ_OVERRD_VAL] = 0x0
		 * Select P0 power state for Rx lane
		 */
		ctlifc1.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_1(i, qlm));
		ctlifc1.s.cfg_rx_pstate_req_ovrrd_val = 0x0;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_1(i, qlm), ctlifc1.u64);
		/* Step 9: Set GSER()_LANE(lane_n)_PCS_CTLIFC_2[CFG_TX_PSTATE_REQ_OVRRD_EN] = 1
		 * Enable Tx power state override and Set
		 * GSER()_LANE(lane_n)_PCS_CTLIFC_2[CFG_RX_PSTATE_REQ_OVRRD_EN] = 1
		 * Enable Rx power state override
		 */
		ctlifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm));
		ctlifc2.s.cfg_tx_pstate_req_ovrrd_en = 0x1;
		ctlifc2.s.cfg_rx_pstate_req_ovrrd_en = 0x1;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm), ctlifc2.u64);
		/* Step 10: Set GSER()_LANE(lane_n)_PCS_CTLIFC_2[CTLIFC_OVRRD_REQ] = 1
		 * Start the CTLIFC override state machine
		 */
		ctlifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm));
		ctlifc2.s.ctlifc_ovrrd_req = 0x1;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm), ctlifc2.u64);
	}

	/* Step 11: Wait 10 msec */
	mdelay(10);

	/* Release Lane Tx/Rx Power state override enables. */
	for (i = 0; i < num_lanes; i++) {
		cvmx_gserx_lanex_pcs_ctlifc_2_t ctlifc2;

		ctlifc2.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm));
		ctlifc2.s.cfg_tx_pstate_req_ovrrd_en = 0x0;
		ctlifc2.s.cfg_rx_pstate_req_ovrrd_en = 0x0;
		csr_wr_node(node, CVMX_GSERX_LANEX_PCS_CTLIFC_2(i, qlm), ctlifc2.u64);
	}

	/* Step 12:  Poll GSER()_PLL_STAT.[PLL_LOCK] = 1
	 * Poll and check that PLL is locked
	 */
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_GSERX_PLL_STAT(qlm), cvmx_gserx_pll_stat_t,
				       pll_lock, ==, 1, 10000)) {
		printf("%d:QLM%d: Timeout waiting for GSERX_PLL_STAT[pll_lock]\n", node, qlm);
		return -1;
	}

	/* Step 13:  Poll GSER()_QLM_STAT.[RST_RDY] = 1
	 * Poll and check that QLM/DLM is Ready
	 */
	if (is_pcie == 0 &&
	    CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_GSERX_QLM_STAT(qlm), cvmx_gserx_qlm_stat_t,
				       rst_rdy, ==, 1, 10000)) {
		printf("%d:QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", node, qlm);
		return -1;
	}

	return 0;
}

/**
 * Configure all of the PLLs for a particular node and qlm
 * @INTERNAL
 *
 * @param node	Node number to configure
 * @param qlm	QLM number to configure
 */
static void __qlm_setup_pll_cn78xx(int node, int qlm)
{
	cvmx_gserx_pll_px_mode_0_t mode_0;
	cvmx_gserx_pll_px_mode_1_t mode_1;
	cvmx_gserx_lane_px_mode_0_t pmode_0;
	cvmx_gserx_lane_px_mode_1_t pmode_1;
	int lane_mode;
	int ref_clk;
	const struct refclk_settings_cn78xx *clk_settings;

	for (lane_mode = 0; lane_mode < R_NUM_LANE_MODES; lane_mode++) {
		mode_0.u64 = csr_rd_node(node, CVMX_GSERX_PLL_PX_MODE_0(lane_mode, qlm));
		mode_1.u64 = csr_rd_node(node, CVMX_GSERX_PLL_PX_MODE_1(lane_mode, qlm));
		pmode_0.u64 = 0;
		pmode_1.u64 = 0;
		ref_clk = ref_clk_cn78xx[node][qlm][lane_mode];
		clk_settings = &refclk_settings_cn78xx[lane_mode][ref_clk];
		debug("%s(%d, %d): lane_mode: 0x%x, ref_clk: %d\n", __func__, node, qlm, lane_mode,
		      ref_clk);

		if (!clk_settings->valid) {
			printf("%s: Error: reference clock %d is not supported for lane mode %d on qlm %d\n",
			       __func__, ref_clk, lane_mode, qlm);
			continue;
		}

		mode_0.s.pll_icp = clk_settings->mode_0.s.pll_icp;
		mode_0.s.pll_rloop = clk_settings->mode_0.s.pll_rloop;
		mode_0.s.pll_pcs_div = clk_settings->mode_0.s.pll_pcs_div;

		mode_1.s.pll_16p5en = clk_settings->mode_1.s.pll_16p5en;
		mode_1.s.pll_cpadj = clk_settings->mode_1.s.pll_cpadj;
		mode_1.s.pll_pcie3en = clk_settings->mode_1.s.pll_pcie3en;
		mode_1.s.pll_opr = clk_settings->mode_1.s.pll_opr;
		mode_1.s.pll_div = clk_settings->mode_1.s.pll_div;

		pmode_0.u64 = clk_settings->pmode_0.u64;

		pmode_1.u64 = clk_settings->pmode_1.u64;

		csr_wr_node(node, CVMX_GSERX_PLL_PX_MODE_1(lane_mode, qlm), mode_1.u64);
		csr_wr_node(node, CVMX_GSERX_LANE_PX_MODE_0(lane_mode, qlm), pmode_0.u64);
		csr_wr_node(node, CVMX_GSERX_LANE_PX_MODE_1(lane_mode, qlm), pmode_1.u64);
		csr_wr_node(node, CVMX_GSERX_PLL_PX_MODE_0(lane_mode, qlm), mode_0.u64);
	}
}

/**
 * Get the lane mode for the specified node and QLM.
 *
 * @param ref_clk_sel	The reference-clock selection to use to configure QLM
 *			 0 = REF_100MHZ
 *			 1 = REF_125MHZ
 *			 2 = REF_156MHZ
 * @param baud_mhz   The speed the QLM needs to be configured in Mhz.
 * @param[out] alt_pll_settings	If non-NULL this will be set if non-default PLL
 *				settings are required for the mode.
 *
 * Return: lane mode to use or -1 on error
 *
 * NOTE: In some modes
 */
static int __get_lane_mode_for_speed_and_ref_clk(int ref_clk_sel, int baud_mhz,
						 bool *alt_pll_settings)
{
	if (alt_pll_settings)
		*alt_pll_settings = false;
	switch (baud_mhz) {
	case 98304:
	case 49152:
	case 24576:
	case 12288:
		if (ref_clk_sel != 3) {
			printf("Error: Invalid ref clock\n");
			return -1;
		}
		return 0x5;
	case 6144:
	case 3072:
		if (ref_clk_sel != 3) {
			printf("Error: Invalid ref clock\n");
			return -1;
		}
		return 0x8;
	case 1250:
		if (alt_pll_settings)
			*alt_pll_settings = (ref_clk_sel != 2);
		return R_125G_REFCLK15625_SGMII;
	case 2500:
		if (ref_clk_sel == 0)
			return R_2_5G_REFCLK100;

		if (alt_pll_settings)
			*alt_pll_settings = (ref_clk_sel != 1);
		return R_2_5G_REFCLK125;
	case 3125:
		if (ref_clk_sel == 2) {
			return R_3125G_REFCLK15625_XAUI;
		} else if (ref_clk_sel == 1) {
			if (alt_pll_settings)
				*alt_pll_settings = true;
			return R_3125G_REFCLK15625_XAUI;
		}

		printf("Error: Invalid speed\n");
		return -1;
	case 5000:
		if (ref_clk_sel == 0) {
			return R_5G_REFCLK100;
		} else if (ref_clk_sel == 1) {
			if (alt_pll_settings)
				*alt_pll_settings = (ref_clk_sel != 1);
			return R_5G_REFCLK125;
		} else {
			return R_5G_REFCLK15625_QSGMII;
		}
	case 6250:
		if (ref_clk_sel != 0) {
			if (alt_pll_settings)
				*alt_pll_settings = (ref_clk_sel != 2);
			return R_625G_REFCLK15625_RXAUI;
		}

		printf("Error: Invalid speed\n");
		return -1;
	case 6316:
		if (ref_clk_sel != 3) {
			printf("Error: Invalid speed\n");
		} else {
			*alt_pll_settings = true;
			return R_625G_REFCLK15625_RXAUI;
		}
	case 8000:
		if (ref_clk_sel == 0)
			return R_8G_REFCLK100;
		else if (ref_clk_sel == 1)
			return R_8G_REFCLK125;

		printf("Error: Invalid speed\n");
		return -1;
	case 103125:
		if (ref_clk_sel == 3 && alt_pll_settings)
			*alt_pll_settings = true;

		if (ref_clk_sel == 2 || ref_clk_sel == 3)
			return R_103125G_REFCLK15625_KR;

	default:
		printf("Error: Invalid speed\n");
		return -1;
	}

	return -1;
}

/*
 * Errata PEM-31375 PEM RSL accesses to PCLK registers can timeout
 * during speed change. Change SLI_WINDOW_CTL[time] to 525us
 */
static void __set_sli_window_ctl_errata_31375(int node)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) ||
	    OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_sli_window_ctl_t window_ctl;

		window_ctl.u64 = csr_rd_node(node, CVMX_PEXP_SLI_WINDOW_CTL);
		/* Configure SLI_WINDOW_CTL only once */
		if (window_ctl.s.time != 8191)
			return;

		window_ctl.s.time = gd->bus_clk * 525ull / 1000000;
		csr_wr_node(node, CVMX_PEXP_SLI_WINDOW_CTL, window_ctl.u64);
	}
}

static void __cvmx_qlm_pcie_errata_ep_cn78xx(int node, int pem)
{
	cvmx_pciercx_cfg031_t cfg031;
	cvmx_pciercx_cfg032_t cfg032;
	cvmx_pciercx_cfg040_t cfg040;
	cvmx_pemx_cfg_t pemx_cfg;
	cvmx_pemx_on_t pemx_on;
	int low_qlm, high_qlm;
	int qlm, lane;
	u64 start_cycle;

	pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(pem));

	/* Errata (GSER-21178) PCIe gen3 doesn't work, continued */

	/* Wait for the link to come up as Gen1 */
	printf("PCIe%d: Waiting for EP out of reset\n", pem);
	while (pemx_on.s.pemoor == 0) {
		udelay(1000);
		pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(pem));
	}

	/* Enable gen3 speed selection */
	printf("PCIe%d: Enabling Gen3 for EP\n", pem);
	/* Force Gen1 for initial link bringup. We'll fix it later */
	pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(pem));
	pemx_cfg.s.md = 2;
	csr_wr_node(node, CVMX_PEMX_CFG(pem), pemx_cfg.u64);
	cfg031.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG031(pem));
	cfg031.s.mls = 2;
	cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG031(pem), cfg031.u32);
	cfg040.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG040(pem));
	cfg040.s.tls = 3;
	cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG040(pem), cfg040.u32);

	/* Wait up to 10ms for the link speed change to complete */
	start_cycle = get_timer(0);
	do {
		if (get_timer(start_cycle) > 10)
			return;

		mdelay(1);
		cfg032.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG032(pem));
	} while (cfg032.s.ls != 3);

	pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(pem));
	low_qlm = pem; /* FIXME */
	high_qlm = (pemx_cfg.cn78xx.lanes8) ? low_qlm + 1 : low_qlm;

	/* Toggle cfg_rx_dll_locken_ovrrd_en and rx_resetn_ovrrd_en across
	 * all QM lanes in use
	 */
	for (qlm = low_qlm; qlm <= high_qlm; qlm++) {
		for (lane = 0; lane < 4; lane++) {
			cvmx_gserx_lanex_rx_misc_ovrrd_t misc_ovrrd;
			cvmx_gserx_lanex_pwr_ctrl_t pwr_ctrl;

			misc_ovrrd.u64 =
				csr_rd_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, pem));
			misc_ovrrd.s.cfg_rx_dll_locken_ovrrd_en = 1;
			csr_wr_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, pem),
				    misc_ovrrd.u64);
			pwr_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PWR_CTRL(lane, pem));
			pwr_ctrl.s.rx_resetn_ovrrd_en = 1;
			csr_wr_node(node, CVMX_GSERX_LANEX_PWR_CTRL(lane, pem), pwr_ctrl.u64);
		}
	}
	for (qlm = low_qlm; qlm <= high_qlm; qlm++) {
		for (lane = 0; lane < 4; lane++) {
			cvmx_gserx_lanex_rx_misc_ovrrd_t misc_ovrrd;
			cvmx_gserx_lanex_pwr_ctrl_t pwr_ctrl;

			misc_ovrrd.u64 =
				csr_rd_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, pem));
			misc_ovrrd.s.cfg_rx_dll_locken_ovrrd_en = 0;
			csr_wr_node(node, CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, pem),
				    misc_ovrrd.u64);
			pwr_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PWR_CTRL(lane, pem));
			pwr_ctrl.s.rx_resetn_ovrrd_en = 0;
			csr_wr_node(node, CVMX_GSERX_LANEX_PWR_CTRL(lane, pem), pwr_ctrl.u64);
		}
	}

	//printf("PCIe%d: Waiting for EP link up at Gen3\n", pem);
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PEMX_ON(pem), cvmx_pemx_on_t, pemoor, ==, 1,
				       1000000)) {
		printf("PCIe%d: Timeout waiting for EP link up at Gen3\n", pem);
		return;
	}
}

static void __cvmx_qlm_pcie_errata_cn78xx(int node, int qlm)
{
	int pem, i, q;
	int is_8lanes;
	int is_high_lanes;
	int low_qlm, high_qlm, is_host;
	int need_ep_monitor;
	cvmx_pemx_cfg_t pem_cfg, pem3_cfg;
	cvmx_gserx_slice_cfg_t slice_cfg;
	cvmx_gserx_rx_pwr_ctrl_p1_t pwr_ctrl_p1;
	cvmx_rst_soft_prstx_t soft_prst;

	/* Only applies to CN78XX pass 1.x */
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		return;

	/* Determine the PEM for this QLM, whether we're in 8 lane mode,
	 * and whether these are the top lanes of the 8
	 */
	switch (qlm) {
	case 0: /* First 4 lanes of PEM0 */
		pem_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(0));
		pem = 0;
		is_8lanes = pem_cfg.cn78xx.lanes8;
		is_high_lanes = 0;
		break;
	case 1: /* Either last 4 lanes of PEM0, or PEM1 */
		pem_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(0));
		pem = (pem_cfg.cn78xx.lanes8) ? 0 : 1;
		is_8lanes = pem_cfg.cn78xx.lanes8;
		is_high_lanes = is_8lanes;
		break;
	case 2: /* First 4 lanes of PEM2 */
		pem_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(2));
		pem = 2;
		is_8lanes = pem_cfg.cn78xx.lanes8;
		is_high_lanes = 0;
		break;
	case 3: /* Either last 4 lanes of PEM2, or PEM3 */
		pem_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(2));
		pem3_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(3));
		pem = (pem_cfg.cn78xx.lanes8) ? 2 : 3;
		is_8lanes = (pem == 2) ? pem_cfg.cn78xx.lanes8 : pem3_cfg.cn78xx.lanes8;
		is_high_lanes = (pem == 2) && is_8lanes;
		break;
	case 4: /* Last 4 lanes of PEM3 */
		pem = 3;
		is_8lanes = 1;
		is_high_lanes = 1;
		break;
	default:
		return;
	}

	/* These workaround must be applied once per PEM. Since we're called per
	 * QLM, wait for the 2nd half of 8 lane setups before doing the workaround
	 */
	if (is_8lanes && !is_high_lanes)
		return;

	pem_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(pem));
	is_host = pem_cfg.cn78xx.hostmd;
	low_qlm = (is_8lanes) ? qlm - 1 : qlm;
	high_qlm = qlm;
	qlm = -1;

	if (!is_host) {
		/* Read the current slice config value. If its at the value we will
		 * program then skip doing the workaround. We're probably doing a
		 * hot reset and the workaround is already applied
		 */
		slice_cfg.u64 = csr_rd_node(node, CVMX_GSERX_SLICE_CFG(low_qlm));
		if (slice_cfg.s.tx_rx_detect_lvl_enc == 7 && OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			return;
	}

	if (is_host && OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
		/* (GSER-XXXX) GSER PHY needs to be reset at initialization */
		cvmx_gserx_phy_ctl_t phy_ctl;

		for (q = low_qlm; q <= high_qlm; q++) {
			phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(q));
			phy_ctl.s.phy_reset = 1;
			csr_wr_node(node, CVMX_GSERX_PHY_CTL(q), phy_ctl.u64);
		}
		udelay(5);

		for (q = low_qlm; q <= high_qlm; q++) {
			phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(q));
			phy_ctl.s.phy_reset = 0;
			csr_wr_node(node, CVMX_GSERX_PHY_CTL(q), phy_ctl.u64);
		}
		udelay(5);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
		/* (GSER-20936) GSER has wrong PCIe RX detect reset value */
		for (q = low_qlm; q <= high_qlm; q++) {
			slice_cfg.u64 = csr_rd_node(node, CVMX_GSERX_SLICE_CFG(q));
			slice_cfg.s.tx_rx_detect_lvl_enc = 7;
			csr_wr_node(node, CVMX_GSERX_SLICE_CFG(q), slice_cfg.u64);
		}

		/* Clear the bit in GSERX_RX_PWR_CTRL_P1[p1_rx_subblk_pd]
		 * that coresponds to "Lane DLL"
		 */
		for (q = low_qlm; q <= high_qlm; q++) {
			pwr_ctrl_p1.u64 = csr_rd_node(node, CVMX_GSERX_RX_PWR_CTRL_P1(q));
			pwr_ctrl_p1.s.p1_rx_subblk_pd &= ~4;
			csr_wr_node(node, CVMX_GSERX_RX_PWR_CTRL_P1(q), pwr_ctrl_p1.u64);
		}

		/* Errata (GSER-20888) GSER incorrect synchronizers hurts PCIe
		 * Override TX Power State machine TX reset control signal
		 */
		for (q = low_qlm; q <= high_qlm; q++) {
			for (i = 0; i < 4; i++) {
				cvmx_gserx_lanex_tx_cfg_0_t tx_cfg;
				cvmx_gserx_lanex_pwr_ctrl_t pwr_ctrl;

				tx_cfg.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_TX_CFG_0(i, q));
				tx_cfg.s.tx_resetn_ovrrd_val = 1;
				csr_wr_node(node, CVMX_GSERX_LANEX_TX_CFG_0(i, q), tx_cfg.u64);
				pwr_ctrl.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_PWR_CTRL(i, q));
				pwr_ctrl.s.tx_p2s_resetn_ovrrd_en = 1;
				csr_wr_node(node, CVMX_GSERX_LANEX_PWR_CTRL(i, q), pwr_ctrl.u64);
			}
		}
	}

	if (!is_host) {
		cvmx_pciercx_cfg089_t cfg089;
		cvmx_pciercx_cfg090_t cfg090;
		cvmx_pciercx_cfg091_t cfg091;
		cvmx_pciercx_cfg092_t cfg092;
		cvmx_pciercx_cfg548_t cfg548;
		cvmx_pciercx_cfg554_t cfg554;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
			/* Errata (GSER-21178) PCIe gen3 doesn't work */
			/* The starting equalization hints are incorrect on CN78XX pass 1.x. Fix
			 * them for the 8 possible lanes. It doesn't hurt to program them even
			 * for lanes not in use
			 */
			cfg089.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG089(pem));
			cfg089.s.l1urph = 2;
			cfg089.s.l1utp = 7;
			cfg089.s.l0urph = 2;
			cfg089.s.l0utp = 7;
			cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG089(pem), cfg089.u32);
			cfg090.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG090(pem));
			cfg090.s.l3urph = 2;
			cfg090.s.l3utp = 7;
			cfg090.s.l2urph = 2;
			cfg090.s.l2utp = 7;
			cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG090(pem), cfg090.u32);
			cfg091.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG091(pem));
			cfg091.s.l5urph = 2;
			cfg091.s.l5utp = 7;
			cfg091.s.l4urph = 2;
			cfg091.s.l4utp = 7;
			cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG091(pem), cfg091.u32);
			cfg092.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG092(pem));
			cfg092.s.l7urph = 2;
			cfg092.s.l7utp = 7;
			cfg092.s.l6urph = 2;
			cfg092.s.l6utp = 7;
			cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG092(pem), cfg092.u32);
			/* FIXME: Disable phase 2 and phase 3 equalization */
			cfg548.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG548(pem));
			cfg548.s.ep2p3d = 1;
			cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG548(pem), cfg548.u32);
		}
		/* Errata (GSER-21331) GEN3 Equalization may fail */
		/* Disable preset #10 and disable the 2ms timeout */
		cfg554.u32 = cvmx_pcie_cfgx_read_node(node, pem, CVMX_PCIERCX_CFG554(pem));
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			cfg554.s.p23td = 1;
		cfg554.s.prv = 0x3ff;
		cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG554(pem), cfg554.u32);

		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
			need_ep_monitor = (pem_cfg.s.md == 2);
			if (need_ep_monitor) {
				cvmx_pciercx_cfg031_t cfg031;
				cvmx_pciercx_cfg040_t cfg040;

				/* Force Gen1 for initial link bringup. We'll
				 * fix it later
				 */
				pem_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(pem));
				pem_cfg.s.md = 0;
				csr_wr_node(node, CVMX_PEMX_CFG(pem), pem_cfg.u64);
				cfg031.u32 = cvmx_pcie_cfgx_read_node(node, pem,
								      CVMX_PCIERCX_CFG031(pem));
				cfg031.s.mls = 0;
				cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG031(pem),
							  cfg031.u32);
				cfg040.u32 = cvmx_pcie_cfgx_read_node(node, pem,
								      CVMX_PCIERCX_CFG040(pem));
				cfg040.s.tls = 1;
				cvmx_pcie_cfgx_write_node(node, pem, CVMX_PCIERCX_CFG040(pem),
							  cfg040.u32);
				__cvmx_qlm_pcie_errata_ep_cn78xx(node, pem);
			}
			return;
		}
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
		/* De-assert the SOFT_RST bit for this QLM (PEM), causing the PCIe
		 * workarounds code above to take effect.
		 */
		soft_prst.u64 = csr_rd_node(node, CVMX_RST_SOFT_PRSTX(pem));
		soft_prst.s.soft_prst = 0;
		csr_wr_node(node, CVMX_RST_SOFT_PRSTX(pem), soft_prst.u64);
		udelay(1);

		/* Assert the SOFT_RST bit for this QLM (PEM), putting the PCIe back into
		 * reset state with disturbing the workarounds.
		 */
		soft_prst.u64 = csr_rd_node(node, CVMX_RST_SOFT_PRSTX(pem));
		soft_prst.s.soft_prst = 1;
		csr_wr_node(node, CVMX_RST_SOFT_PRSTX(pem), soft_prst.u64);
	}
	udelay(1);
}

/**
 * Setup the PEM to either driver or receive reset from PRST based on RC or EP
 *
 * @param node   Node to use in a Numa setup
 * @param pem    Which PEM to setuo
 * @param is_endpoint
 *               Non zero if PEM is a EP
 */
static void __setup_pem_reset(int node, int pem, int is_endpoint)
{
	cvmx_rst_ctlx_t rst_ctl;

	/* Make sure is_endpoint is either 0 or 1 */
	is_endpoint = (is_endpoint != 0);
	rst_ctl.u64 = csr_rd_node(node, CVMX_RST_CTLX(pem));
	rst_ctl.s.prst_link = 0;	  /* Link down causes soft reset */
	rst_ctl.s.rst_link = is_endpoint; /* EP PERST causes a soft reset */
	rst_ctl.s.rst_drv = !is_endpoint; /* Drive if RC */
	rst_ctl.s.rst_rcv = is_endpoint;  /* Only read PERST in EP mode */
	rst_ctl.s.rst_chip = 0;		  /* PERST doesn't pull CHIP_RESET */
	csr_wr_node(node, CVMX_RST_CTLX(pem), rst_ctl.u64);
}

/**
 * Configure QLM speed and mode for cn78xx.
 *
 * @param node    Node to configure the QLM
 * @param qlm     The QLM to configure
 * @param baud_mhz   The speed the QLM needs to be configured in Mhz.
 * @param mode    The QLM to be configured as SGMII/XAUI/PCIe.
 * @param rc      Only used for PCIe, rc = 1 for root complex mode, 0 for EP mode.
 * @param gen3    Only used for PCIe
 *			gen3 = 2 GEN3 mode
 *			gen3 = 1 GEN2 mode
 *			gen3 = 0 GEN1 mode
 *
 * @param ref_clk_sel    The reference-clock selection to use to configure QLM
 *			 0 = REF_100MHZ
 *			 1 = REF_125MHZ
 *			 2 = REF_156MHZ
 *			 3 = REF_161MHZ
 * @param ref_clk_input  The reference-clock input to use to configure QLM
 *
 * Return:       Return 0 on success or -1.
 */
int octeon_configure_qlm_cn78xx(int node, int qlm, int baud_mhz, int mode, int rc, int gen3,
				int ref_clk_sel, int ref_clk_input)
{
	cvmx_gserx_phy_ctl_t phy_ctl;
	cvmx_gserx_lane_mode_t lmode;
	cvmx_gserx_cfg_t cfg;
	cvmx_gserx_refclk_sel_t refclk_sel;

	int is_pcie = 0;
	int is_ilk = 0;
	int is_bgx = 0;
	int lane_mode = 0;
	int lmac_type = 0;
	bool alt_pll = false;
	int num_ports = 0;
	int lane_to_sds = 0;

	debug("%s(node: %d, qlm: %d, baud_mhz: %d, mode: %d, rc: %d, gen3: %d, ref_clk_sel: %d, ref_clk_input: %d\n",
	      __func__, node, qlm, baud_mhz, mode, rc, gen3, ref_clk_sel, ref_clk_input);
	if (OCTEON_IS_MODEL(OCTEON_CN76XX) && qlm > 4) {
		debug("%s: qlm %d not present on CN76XX\n", __func__, qlm);
		return -1;
	}

	/* Errata PEM-31375 PEM RSL accesses to PCLK registers can timeout
	 * during speed change. Change SLI_WINDOW_CTL[time] to 525us
	 */
	__set_sli_window_ctl_errata_31375(node);

	cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));
	/* If PEM is in EP, no need to do anything */

	if (cfg.s.pcie && rc == 0) {
		debug("%s: node %d, qlm %d is in PCIe endpoint mode, returning\n",
		      __func__, node, qlm);
		return 0;
	}

	/* Set the reference clock to use */
	refclk_sel.u64 = 0;
	if (ref_clk_input == 0) { /* External ref clock */
		refclk_sel.s.com_clk_sel = 0;
		refclk_sel.s.use_com1 = 0;
	} else if (ref_clk_input == 1) {
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 0;
	} else {
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 1;
	}

	csr_wr_node(node, CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);

	/* Reset the QLM after changing the reference clock */
	phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 1;
	phy_ctl.s.phy_pd = 1;
	csr_wr_node(node, CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	udelay(1000);

	/* Always restore the reference clocks for a QLM */
	memcpy(ref_clk_cn78xx[node][qlm], def_ref_clk_cn78xx, sizeof(def_ref_clk_cn78xx));
	switch (mode) {
	case CVMX_QLM_MODE_PCIE:
	case CVMX_QLM_MODE_PCIE_1X8: {
		cvmx_pemx_cfg_t pemx_cfg;
		cvmx_pemx_on_t pemx_on;

		is_pcie = 1;

		if (ref_clk_sel == 0) {
			refclk_sel.u64 = csr_rd_node(node, CVMX_GSERX_REFCLK_SEL(qlm));
			refclk_sel.s.pcie_refclk125 = 0;
			csr_wr_node(node, CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);
			if (gen3 == 0) /* Gen1 mode */
				lane_mode = R_2_5G_REFCLK100;
			else if (gen3 == 1) /* Gen2 mode */
				lane_mode = R_5G_REFCLK100;
			else
				lane_mode = R_8G_REFCLK100;
		} else if (ref_clk_sel == 1) {
			refclk_sel.u64 = csr_rd_node(node, CVMX_GSERX_REFCLK_SEL(qlm));
			refclk_sel.s.pcie_refclk125 = 1;
			csr_wr_node(node, CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);
			if (gen3 == 0) /* Gen1 mode */
				lane_mode = R_2_5G_REFCLK125;
			else if (gen3 == 1) /* Gen2 mode */
				lane_mode = R_5G_REFCLK125;
			else
				lane_mode = R_8G_REFCLK125;
		} else {
			printf("Invalid reference clock for PCIe on QLM%d\n", qlm);
			return -1;
		}

		switch (qlm) {
		case 0: /* Either x4 or x8 based on PEM0 */
		{
			cvmx_rst_soft_prstx_t rst_prst;

			rst_prst.u64 = csr_rd_node(node, CVMX_RST_SOFT_PRSTX(0));
			rst_prst.s.soft_prst = rc;
			csr_wr_node(node, CVMX_RST_SOFT_PRSTX(0), rst_prst.u64);
			__setup_pem_reset(node, 0, !rc);

			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(0));
			pemx_cfg.cn78xx.lanes8 = (mode == CVMX_QLM_MODE_PCIE_1X8);
			pemx_cfg.cn78xx.hostmd = rc;
			pemx_cfg.cn78xx.md = gen3;
			csr_wr_node(node, CVMX_PEMX_CFG(0), pemx_cfg.u64);
			/* x8 mode waits for QLM1 setup before turning on the PEM */
			if (mode == CVMX_QLM_MODE_PCIE) {
				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(0));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(0), pemx_on.u64);
			}
			break;
		}
		case 1: /* Either PEM0 x8 or PEM1 x4 */
		{
			if (mode == CVMX_QLM_MODE_PCIE) {
				cvmx_rst_soft_prstx_t rst_prst;
				cvmx_pemx_cfg_t pemx_cfg;

				rst_prst.u64 = csr_rd_node(node, CVMX_RST_SOFT_PRSTX(1));
				rst_prst.s.soft_prst = rc;
				csr_wr_node(node, CVMX_RST_SOFT_PRSTX(1), rst_prst.u64);
				__setup_pem_reset(node, 1, !rc);

				pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(1));
				pemx_cfg.cn78xx.lanes8 = 0;
				pemx_cfg.cn78xx.hostmd = rc;
				pemx_cfg.cn78xx.md = gen3;
				csr_wr_node(node, CVMX_PEMX_CFG(1), pemx_cfg.u64);

				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(1));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(1), pemx_on.u64);
			} else {
				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(0));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(0), pemx_on.u64);
			}
			break;
		}
		case 2: /* Either PEM2 x4 or PEM2 x8 */
		{
			cvmx_rst_soft_prstx_t rst_prst;

			rst_prst.u64 = csr_rd_node(node, CVMX_RST_SOFT_PRSTX(2));
			rst_prst.s.soft_prst = rc;
			csr_wr_node(node, CVMX_RST_SOFT_PRSTX(2), rst_prst.u64);
			__setup_pem_reset(node, 2, !rc);

			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(2));
			pemx_cfg.cn78xx.lanes8 = (mode == CVMX_QLM_MODE_PCIE_1X8);
			pemx_cfg.cn78xx.hostmd = rc;
			pemx_cfg.cn78xx.md = gen3;
			csr_wr_node(node, CVMX_PEMX_CFG(2), pemx_cfg.u64);
			/* x8 mode waits for QLM3 setup before turning on the PEM */
			if (mode == CVMX_QLM_MODE_PCIE) {
				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(2));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(2), pemx_on.u64);
			}
			break;
		}
		case 3: /* Either PEM2 x8 or PEM3 x4 */
		{
			pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(2));
			if (pemx_cfg.cn78xx.lanes8) {
				/* Last 4 lanes of PEM2 */
				/* PEMX_CFG already setup */
				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(2));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(2), pemx_on.u64);
			}
			/* Check if PEM3 uses QLM3 and in x4 lane mode */
			if (mode == CVMX_QLM_MODE_PCIE) {
				cvmx_rst_soft_prstx_t rst_prst;

				rst_prst.u64 = csr_rd_node(node, CVMX_RST_SOFT_PRSTX(3));
				rst_prst.s.soft_prst = rc;
				csr_wr_node(node, CVMX_RST_SOFT_PRSTX(3), rst_prst.u64);
				__setup_pem_reset(node, 3, !rc);

				pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(3));
				pemx_cfg.cn78xx.lanes8 = 0;
				pemx_cfg.cn78xx.hostmd = rc;
				pemx_cfg.cn78xx.md = gen3;
				csr_wr_node(node, CVMX_PEMX_CFG(3), pemx_cfg.u64);

				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(3));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(3), pemx_on.u64);
			}
			break;
		}
		case 4: /* Either PEM3 x4 or PEM3 x8 */
		{
			if (mode == CVMX_QLM_MODE_PCIE_1X8) {
				/* Last 4 lanes of PEM3 */
				/* PEMX_CFG already setup */
				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(3));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(3), pemx_on.u64);
			} else {
				/* 4 lanes of PEM3 */
				cvmx_pemx_qlm_t pemx_qlm;
				cvmx_rst_soft_prstx_t rst_prst;

				rst_prst.u64 = csr_rd_node(node, CVMX_RST_SOFT_PRSTX(3));
				rst_prst.s.soft_prst = rc;
				csr_wr_node(node, CVMX_RST_SOFT_PRSTX(3), rst_prst.u64);
				__setup_pem_reset(node, 3, !rc);

				pemx_cfg.u64 = csr_rd_node(node, CVMX_PEMX_CFG(3));
				pemx_cfg.cn78xx.lanes8 = 0;
				pemx_cfg.cn78xx.hostmd = rc;
				pemx_cfg.cn78xx.md = gen3;
				csr_wr_node(node, CVMX_PEMX_CFG(3), pemx_cfg.u64);
				/* PEM3 is on QLM4 */
				pemx_qlm.u64 = csr_rd_node(node, CVMX_PEMX_QLM(3));
				pemx_qlm.cn78xx.pem3qlm = 1;
				csr_wr_node(node, CVMX_PEMX_QLM(3), pemx_qlm.u64);
				pemx_on.u64 = csr_rd_node(node, CVMX_PEMX_ON(3));
				pemx_on.s.pemon = 1;
				csr_wr_node(node, CVMX_PEMX_ON(3), pemx_on.u64);
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case CVMX_QLM_MODE_ILK:
		is_ilk = 1;
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz, &alt_pll);
		if (lane_mode == -1)
			return -1;
		/* FIXME: Set lane_mode for other speeds */
		break;
	case CVMX_QLM_MODE_SGMII:
		is_bgx = 1;
		lmac_type = 0;
		lane_to_sds = 1;
		num_ports = 4;
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz, &alt_pll);
		debug("%s: SGMII lane mode: %d, alternate PLL: %s\n", __func__, lane_mode,
		      alt_pll ? "true" : "false");
		if (lane_mode == -1)
			return -1;
		break;
	case CVMX_QLM_MODE_XAUI:
		is_bgx = 5;
		lmac_type = 1;
		lane_to_sds = 0xe4;
		num_ports = 1;
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz, &alt_pll);
		debug("%s: XAUI lane mode: %d\n", __func__, lane_mode);
		if (lane_mode == -1)
			return -1;
		break;
	case CVMX_QLM_MODE_RXAUI:
		is_bgx = 3;
		lmac_type = 2;
		lane_to_sds = 0;
		num_ports = 2;
		debug("%s: RXAUI lane mode: %d\n", __func__, lane_mode);
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz, &alt_pll);
		if (lane_mode == -1)
			return -1;
		break;
	case CVMX_QLM_MODE_XFI: /* 10GR_4X1 */
	case CVMX_QLM_MODE_10G_KR:
		is_bgx = 1;
		lmac_type = 3;
		lane_to_sds = 1;
		num_ports = 4;
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz, &alt_pll);
		debug("%s: XFI/10G_KR lane mode: %d\n", __func__, lane_mode);
		if (lane_mode == -1)
			return -1;
		break;
	case CVMX_QLM_MODE_XLAUI: /* 40GR4_1X4 */
	case CVMX_QLM_MODE_40G_KR4:
		is_bgx = 5;
		lmac_type = 4;
		lane_to_sds = 0xe4;
		num_ports = 1;
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz, &alt_pll);
		debug("%s: XLAUI/40G_KR4 lane mode: %d\n", __func__, lane_mode);
		if (lane_mode == -1)
			return -1;
		break;
	case CVMX_QLM_MODE_DISABLED:
		/* Power down the QLM */
		phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));
		phy_ctl.s.phy_pd = 1;
		phy_ctl.s.phy_reset = 1;
		csr_wr_node(node, CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);
		/* Disable all modes */
		csr_wr_node(node, CVMX_GSERX_CFG(qlm), 0);
		/* Do nothing */
		return 0;
	default:
		break;
	}

	if (alt_pll) {
		debug("%s: alternate PLL settings used for node %d, qlm %d, lane mode %d, reference clock %d\n",
		      __func__, node, qlm, lane_mode, ref_clk_sel);
		if (__set_qlm_ref_clk_cn78xx(node, qlm, lane_mode, ref_clk_sel)) {
			printf("%s: Error: reference clock %d is not supported for node %d, qlm %d\n",
			       __func__, ref_clk_sel, node, qlm);
			return -1;
		}
	}

	/* Power up PHY, but keep it in reset */
	phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_pd = 0;
	phy_ctl.s.phy_reset = 1;
	csr_wr_node(node, CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	/* Errata GSER-20788: GSER(0..13)_CFG[BGX_QUAD]=1 is broken. Force the
	 * BGX_QUAD bit to be clear for CN78XX pass 1.x
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		is_bgx &= 3;

	/* Set GSER for the interface mode */
	cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));
	cfg.s.ila = is_ilk;
	cfg.s.bgx = is_bgx & 1;
	cfg.s.bgx_quad = (is_bgx >> 2) & 1;
	cfg.s.bgx_dual = (is_bgx >> 1) & 1;
	cfg.s.pcie = is_pcie;
	csr_wr_node(node, CVMX_GSERX_CFG(qlm), cfg.u64);

	/* Lane mode */
	lmode.u64 = csr_rd_node(node, CVMX_GSERX_LANE_MODE(qlm));
	lmode.s.lmode = lane_mode;
	csr_wr_node(node, CVMX_GSERX_LANE_MODE(qlm), lmode.u64);

	/* BGX0-1 can connect to QLM0-1 or QLM 2-3. Program the select bit if we're
	 * one of these QLMs and we're using BGX
	 */
	if (qlm < 4 && is_bgx) {
		int bgx = qlm & 1;
		int use_upper = (qlm >> 1) & 1;
		cvmx_bgxx_cmr_global_config_t global_cfg;

		global_cfg.u64 = csr_rd_node(node, CVMX_BGXX_CMR_GLOBAL_CONFIG(bgx));
		global_cfg.s.pmux_sds_sel = use_upper;
		csr_wr_node(node, CVMX_BGXX_CMR_GLOBAL_CONFIG(bgx), global_cfg.u64);
	}

	/* Bring phy out of reset */
	phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 0;
	csr_wr_node(node, CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);
	csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));

	/*
	 * Wait 250 ns until the management interface is ready to accept
	 * read/write commands.
	 */
	udelay(1);

	if (is_bgx) {
		int bgx = (qlm < 2) ? qlm : qlm - 2;
		cvmx_bgxx_cmrx_config_t cmr_config;
		int index;

		for (index = 0; index < num_ports; index++) {
			cmr_config.u64 = csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, bgx));
			cmr_config.s.enable = 0;
			cmr_config.s.data_pkt_tx_en = 0;
			cmr_config.s.data_pkt_rx_en = 0;
			cmr_config.s.lmac_type = lmac_type;
			cmr_config.s.lane_to_sds = ((lane_to_sds == 1) ?
						    index : ((lane_to_sds == 0) ?
							     (index ? 0xe : 4) :
							     lane_to_sds));
			csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, bgx), cmr_config.u64);
		}
		csr_wr_node(node, CVMX_BGXX_CMR_TX_LMACS(bgx), num_ports);
		csr_wr_node(node, CVMX_BGXX_CMR_RX_LMACS(bgx), num_ports);

		/* Enable/disable training for 10G_KR/40G_KR4/XFI/XLAUI modes */
		for (index = 0; index < num_ports; index++) {
			cvmx_bgxx_spux_br_pmd_control_t spu_pmd_control;

			spu_pmd_control.u64 =
				csr_rd_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx));

			if (mode == CVMX_QLM_MODE_10G_KR || mode == CVMX_QLM_MODE_40G_KR4)
				spu_pmd_control.s.train_en = 1;
			else if (mode == CVMX_QLM_MODE_XFI || mode == CVMX_QLM_MODE_XLAUI)
				spu_pmd_control.s.train_en = 0;

			csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx),
				    spu_pmd_control.u64);
		}
	}

	/* Configure the gser pll */
	if (!is_pcie)
		__qlm_setup_pll_cn78xx(node, qlm);

	/* Wait for reset to complete and the PLL to lock */
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_GSERX_PLL_STAT(qlm),
				       cvmx_gserx_pll_stat_t,
				       pll_lock, ==, 1, 10000)) {
		printf("%d:QLM%d: Timeout waiting for GSERX_PLL_STAT[pll_lock]\n",
		       node, qlm);
		return -1;
	}

	/* Perform PCIe errata workaround */
	if (is_pcie)
		__cvmx_qlm_pcie_errata_cn78xx(node, qlm);
	else
		__qlm_init_errata_20844(node, qlm);

	/* Wait for reset to complete and the PLL to lock */
	/* PCIe mode doesn't become ready until the PEM block attempts to bring
	 * the interface up. Skip this check for PCIe
	 */
	if (!is_pcie && CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_GSERX_QLM_STAT(qlm),
						   cvmx_gserx_qlm_stat_t, rst_rdy,
						   ==, 1, 10000)) {
		printf("%d:QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n",
		       node, qlm);
		return -1;
	}

	/* Errata GSER-26150: 10G PHY PLL Temperature Failure */
	/* This workaround must be completed after the final deassertion of
	 * GSERx_PHY_CTL[PHY_RESET].
	 * Apply the workaround to 10.3125Gbps and 8Gbps only.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
	    (baud_mhz == 103125 || (is_pcie && gen3 == 2)))
		__qlm_errata_gser_26150(0, qlm, is_pcie);

	/* Errata GSER-26636: 10G-KR/40G-KR - Inverted Tx Coefficient Direction
	 * Change. Applied to all 10G standards (required for KR) but also
	 * applied to other standards in case software training is used
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && baud_mhz == 103125)
		__qlm_kr_inc_dec_gser26636(node, qlm);

	/* Errata GSER-25992: RX EQ Default Settings Update (CTLE Bias) */
	/* This workaround will only be applied to Pass 1.x */
	/* It will also only be applied if the SERDES data-rate is 10G */
	/* or if PCIe Gen3 (gen3=2 is PCIe Gen3) */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
	    (baud_mhz == 103125 || (is_pcie && gen3 == 2)))
		cvmx_qlm_gser_errata_25992(node, qlm);

	/* Errata GSER-27140: Updating the RX EQ settings due to temperature
	 * drift sensitivities
	 */
	/* This workaround will also only be applied if the SERDES data-rate is 10G */
	if (baud_mhz == 103125)
		__qlm_rx_eq_temp_gser27140(node, qlm);

	/* Reduce the voltage amplitude coming from Marvell PHY and also change
	 * DFE threshold settings for RXAUI interface
	 */
	if (is_bgx && mode == CVMX_QLM_MODE_RXAUI) {
		int l;

		for (l = 0; l < 4; l++) {
			cvmx_gserx_lanex_rx_cfg_4_t cfg4;
			cvmx_gserx_lanex_tx_cfg_0_t cfg0;
			/* Change the Q/QB error sampler 0 threshold from 0xD to 0xF */
			cfg4.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_RX_CFG_4(l, qlm));
			cfg4.s.cfg_rx_errdet_ctrl = 0xcf6f;
			csr_wr_node(node, CVMX_GSERX_LANEX_RX_CFG_4(l, qlm), cfg4.u64);
			/* Reduce the voltage swing to roughly 460mV */
			cfg0.u64 = csr_rd_node(node, CVMX_GSERX_LANEX_TX_CFG_0(l, qlm));
			cfg0.s.cfg_tx_swing = 0x12;
			csr_wr_node(node, CVMX_GSERX_LANEX_TX_CFG_0(l, qlm), cfg0.u64);
		}
	}

	return 0;
}

static int __is_qlm_valid_bgx_cn73xx(int qlm)
{
	if (qlm == 2 || qlm == 3 || qlm == 5 || qlm == 6)
		return 0;
	return 1;
}

/**
 * Configure QLM/DLM speed and mode for cn73xx.
 *
 * @param qlm     The QLM to configure
 * @param baud_mhz   The speed the QLM needs to be configured in Mhz.
 * @param mode    The QLM to be configured as SGMII/XAUI/PCIe.
 * @param rc      Only used for PCIe, rc = 1 for root complex mode, 0 for EP mode.
 * @param gen3    Only used for PCIe
 *			gen3 = 2 GEN3 mode
 *			gen3 = 1 GEN2 mode
 *			gen3 = 0 GEN1 mode
 *
 * @param ref_clk_sel   The reference-clock selection to use to configure QLM
 *			0 = REF_100MHZ
 *			1 = REF_125MHZ
 *			2 = REF_156MHZ
 *			3 = REF_161MHZ
 *
 * @param ref_clk_input  The reference-clock input to use to configure QLM
 *			 0 = QLM/DLM reference clock input
 *			 1 = common reference clock input 0
 *			 2 = common reference clock input 1
 *
 * Return:       Return 0 on success or -1.
 */
static int octeon_configure_qlm_cn73xx(int qlm, int baud_mhz, int mode, int rc, int gen3,
				       int ref_clk_sel, int ref_clk_input)
{
	cvmx_gserx_phy_ctl_t phy_ctl;
	cvmx_gserx_lane_mode_t lmode;
	cvmx_gserx_cfg_t cfg;
	cvmx_gserx_refclk_sel_t refclk_sel;
	int is_pcie = 0;
	int is_bgx = 0;
	int lane_mode = 0;
	short lmac_type[4] = { 0 };
	short sds_lane[4] = { 0 };
	bool alt_pll = false;
	int enable_training = 0;
	int additional_lmacs = 0;

	debug("%s(qlm: %d, baud_mhz: %d, mode: %d, rc: %d, gen3: %d, ref_clk_sel: %d, ref_clk_input: %d\n",
	      __func__, qlm, baud_mhz, mode, rc, gen3, ref_clk_sel, ref_clk_input);

	/* Don't configure QLM4 if it is not in SATA mode */
	if (qlm == 4) {
		if (mode == CVMX_QLM_MODE_SATA_2X1)
			return __setup_sata(qlm, baud_mhz, ref_clk_sel, ref_clk_input);

		printf("Invalid mode for QLM4\n");
		return 0;
	}

	cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));

	/* Errata PEM-31375 PEM RSL accesses to PCLK registers can timeout
	 * during speed change. Change SLI_WINDOW_CTL[time] to 525us
	 */
	__set_sli_window_ctl_errata_31375(0);
	/* If PEM is in EP, no need to do anything */
	if (cfg.s.pcie && rc == 0 &&
	    (mode == CVMX_QLM_MODE_PCIE || mode == CVMX_QLM_MODE_PCIE_1X8 ||
	     mode == CVMX_QLM_MODE_PCIE_1X2)) {
		debug("%s: qlm %d is in PCIe endpoint mode, returning\n", __func__, qlm);
		return 0;
	}

	/* Set the reference clock to use */
	refclk_sel.u64 = 0;
	if (ref_clk_input == 0) { /* External ref clock */
		refclk_sel.s.com_clk_sel = 0;
		refclk_sel.s.use_com1 = 0;
	} else if (ref_clk_input == 1) {
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 0;
	} else {
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 1;
	}

	csr_wr(CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);

	/* Reset the QLM after changing the reference clock */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 1;
	phy_ctl.s.phy_pd = 1;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	udelay(1000);

	/* Check if QLM is a valid BGX interface */
	if (mode != CVMX_QLM_MODE_PCIE && mode != CVMX_QLM_MODE_PCIE_1X2 &&
	    mode != CVMX_QLM_MODE_PCIE_1X8) {
		if (__is_qlm_valid_bgx_cn73xx(qlm))
			return -1;
	}

	switch (mode) {
	case CVMX_QLM_MODE_PCIE:
	case CVMX_QLM_MODE_PCIE_1X2:
	case CVMX_QLM_MODE_PCIE_1X8: {
		cvmx_pemx_cfg_t pemx_cfg;
		cvmx_pemx_on_t pemx_on;
		cvmx_pemx_qlm_t pemx_qlm;
		cvmx_rst_soft_prstx_t rst_prst;
		int port = 0;

		is_pcie = 1;

		if (qlm < 5 && mode == CVMX_QLM_MODE_PCIE_1X2) {
			printf("Invalid PCIe mode(%d) for QLM%d\n", mode, qlm);
			return -1;
		}

		if (ref_clk_sel == 0) {
			refclk_sel.u64 = csr_rd(CVMX_GSERX_REFCLK_SEL(qlm));
			refclk_sel.s.pcie_refclk125 = 0;
			csr_wr(CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);
			if (gen3 == 0) /* Gen1 mode */
				lane_mode = R_2_5G_REFCLK100;
			else if (gen3 == 1) /* Gen2 mode */
				lane_mode = R_5G_REFCLK100;
			else
				lane_mode = R_8G_REFCLK100;
		} else if (ref_clk_sel == 1) {
			refclk_sel.u64 = csr_rd(CVMX_GSERX_REFCLK_SEL(qlm));
			refclk_sel.s.pcie_refclk125 = 1;
			csr_wr(CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);
			if (gen3 == 0) /* Gen1 mode */
				lane_mode = R_2_5G_REFCLK125;
			else if (gen3 == 1) /* Gen2 mode */
				lane_mode = R_5G_REFCLK125;
			else
				lane_mode = R_8G_REFCLK125;
		} else {
			printf("Invalid reference clock for PCIe on QLM%d\n", qlm);
			return -1;
		}

		switch (qlm) {
		case 0: /* Either x4 or x8 based on PEM0 */
			rst_prst.u64 = csr_rd(CVMX_RST_SOFT_PRSTX(0));
			rst_prst.s.soft_prst = rc;
			csr_wr(CVMX_RST_SOFT_PRSTX(0), rst_prst.u64);
			__setup_pem_reset(0, 0, !rc);

			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));
			pemx_cfg.cn78xx.lanes8 = (mode == CVMX_QLM_MODE_PCIE_1X8);
			pemx_cfg.cn78xx.hostmd = rc;
			pemx_cfg.cn78xx.md = gen3;
			csr_wr(CVMX_PEMX_CFG(0), pemx_cfg.u64);
			/* x8 mode waits for QLM1 setup before turning on the PEM */
			if (mode == CVMX_QLM_MODE_PCIE) {
				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(0));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(0), pemx_on.u64);
			}
			break;
		case 1: /* Either PEM0 x8 or PEM1 x4 */
			if (mode == CVMX_QLM_MODE_PCIE) {
				rst_prst.u64 = csr_rd(CVMX_RST_SOFT_PRSTX(1));
				rst_prst.s.soft_prst = rc;
				csr_wr(CVMX_RST_SOFT_PRSTX(1), rst_prst.u64);
				__setup_pem_reset(0, 1, !rc);

				pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(1));
				pemx_cfg.cn78xx.lanes8 = 0;
				pemx_cfg.cn78xx.hostmd = rc;
				pemx_cfg.cn78xx.md = gen3;
				csr_wr(CVMX_PEMX_CFG(1), pemx_cfg.u64);

				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(1));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(1), pemx_on.u64);
			} else { /* x8 mode */
				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(0));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(0), pemx_on.u64);
			}
			break;
		case 2: /* Either PEM2 x4 or PEM2 x8 or BGX0 */
		{
			pemx_qlm.u64 = csr_rd(CVMX_PEMX_QLM(2));
			pemx_qlm.cn73xx.pemdlmsel = 0;
			csr_wr(CVMX_PEMX_QLM(2), pemx_qlm.u64);

			rst_prst.u64 = csr_rd(CVMX_RST_SOFT_PRSTX(2));
			rst_prst.s.soft_prst = rc;
			csr_wr(CVMX_RST_SOFT_PRSTX(2), rst_prst.u64);
			__setup_pem_reset(0, 2, !rc);

			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(2));
			pemx_cfg.cn78xx.lanes8 = (mode == CVMX_QLM_MODE_PCIE_1X8);
			pemx_cfg.cn78xx.hostmd = rc;
			pemx_cfg.cn78xx.md = gen3;
			csr_wr(CVMX_PEMX_CFG(2), pemx_cfg.u64);
			/* x8 mode waits for QLM3 setup before turning on the PEM */
			if (mode == CVMX_QLM_MODE_PCIE) {
				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(2));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(2), pemx_on.u64);
			}
			break;
		}
		case 3: /* Either PEM2 x8 or PEM3 x4 or BGX1 */
			/* PEM2/PEM3 are configured to use QLM2/3 */
			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(2));
			if (pemx_cfg.cn78xx.lanes8) {
				/* Last 4 lanes of PEM2 */
				/* PEMX_CFG already setup */
				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(2));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(2), pemx_on.u64);
			}
			/* Check if PEM3 uses QLM3 and in x4 lane mode */
			if (mode == CVMX_QLM_MODE_PCIE) {
				pemx_qlm.u64 = csr_rd(CVMX_PEMX_QLM(3));
				pemx_qlm.cn73xx.pemdlmsel = 0;
				csr_wr(CVMX_PEMX_QLM(3), pemx_qlm.u64);

				rst_prst.u64 = csr_rd(CVMX_RST_SOFT_PRSTX(3));
				rst_prst.s.soft_prst = rc;
				csr_wr(CVMX_RST_SOFT_PRSTX(3), rst_prst.u64);
				__setup_pem_reset(0, 3, !rc);

				pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(3));
				pemx_cfg.cn78xx.lanes8 = 0;
				pemx_cfg.cn78xx.hostmd = rc;
				pemx_cfg.cn78xx.md = gen3;
				csr_wr(CVMX_PEMX_CFG(3), pemx_cfg.u64);

				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(3));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(3), pemx_on.u64);
			}
			break;
		case 5: /* PEM2/PEM3 x2 or BGX2 */
		case 6:
			port = (qlm == 5) ? 2 : 3;
			if (mode == CVMX_QLM_MODE_PCIE_1X2) {
				/* PEM2/PEM3 are configured to use DLM5/6 */
				pemx_qlm.u64 = csr_rd(CVMX_PEMX_QLM(port));
				pemx_qlm.cn73xx.pemdlmsel = 1;
				csr_wr(CVMX_PEMX_QLM(port), pemx_qlm.u64);
				/* 2 lanes of PEM3 */
				rst_prst.u64 = csr_rd(CVMX_RST_SOFT_PRSTX(port));
				rst_prst.s.soft_prst = rc;
				csr_wr(CVMX_RST_SOFT_PRSTX(port), rst_prst.u64);
				__setup_pem_reset(0, port, !rc);

				pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(port));
				pemx_cfg.cn78xx.lanes8 = 0;
				pemx_cfg.cn78xx.hostmd = rc;
				pemx_cfg.cn78xx.md = gen3;
				csr_wr(CVMX_PEMX_CFG(port), pemx_cfg.u64);

				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(port));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(port), pemx_on.u64);
			}
			break;
		default:
			break;
		}
		break;
	}
	case CVMX_QLM_MODE_SGMII:
		is_bgx = 1;
		lmac_type[0] = 0;
		lmac_type[1] = 0;
		lmac_type[2] = 0;
		lmac_type[3] = 0;
		sds_lane[0] = 0;
		sds_lane[1] = 1;
		sds_lane[2] = 2;
		sds_lane[3] = 3;
		break;
	case CVMX_QLM_MODE_SGMII_2X1:
		if (qlm == 5) {
			is_bgx = 1;
			lmac_type[0] = 0;
			lmac_type[1] = 0;
			lmac_type[2] = -1;
			lmac_type[3] = -1;
			sds_lane[0] = 0;
			sds_lane[1] = 1;
		} else if (qlm == 6) {
			is_bgx = 1;
			lmac_type[0] = -1;
			lmac_type[1] = -1;
			lmac_type[2] = 0;
			lmac_type[3] = 0;
			sds_lane[2] = 2;
			sds_lane[3] = 3;
			additional_lmacs = 2;
		}
		break;
	case CVMX_QLM_MODE_XAUI:
		is_bgx = 5;
		lmac_type[0] = 1;
		lmac_type[1] = -1;
		lmac_type[2] = -1;
		lmac_type[3] = -1;
		sds_lane[0] = 0xe4;
		break;
	case CVMX_QLM_MODE_RXAUI:
		is_bgx = 3;
		lmac_type[0] = 2;
		lmac_type[1] = 2;
		lmac_type[2] = -1;
		lmac_type[3] = -1;
		sds_lane[0] = 0x4;
		sds_lane[1] = 0xe;
		break;
	case CVMX_QLM_MODE_RXAUI_1X2:
		if (qlm == 5) {
			is_bgx = 3;
			lmac_type[0] = 2;
			lmac_type[1] = -1;
			lmac_type[2] = -1;
			lmac_type[3] = -1;
			sds_lane[0] = 0x4;
		}
		if (qlm == 6) {
			is_bgx = 3;
			lmac_type[0] = -1;
			lmac_type[1] = -1;
			lmac_type[2] = 2;
			lmac_type[3] = -1;
			sds_lane[2] = 0xe;
			additional_lmacs = 2;
		}
		break;
	case CVMX_QLM_MODE_10G_KR:
		enable_training = 1;
	case CVMX_QLM_MODE_XFI: /* 10GR_4X1 */
		is_bgx = 1;
		lmac_type[0] = 3;
		lmac_type[1] = 3;
		lmac_type[2] = 3;
		lmac_type[3] = 3;
		sds_lane[0] = 0;
		sds_lane[1] = 1;
		sds_lane[2] = 2;
		sds_lane[3] = 3;
		break;
	case CVMX_QLM_MODE_10G_KR_1X2:
		enable_training = 1;
	case CVMX_QLM_MODE_XFI_1X2:
		if (qlm == 5) {
			is_bgx = 1;
			lmac_type[0] = 3;
			lmac_type[1] = 3;
			lmac_type[2] = -1;
			lmac_type[3] = -1;
			sds_lane[0] = 0;
			sds_lane[1] = 1;
		} else if (qlm == 6) {
			is_bgx = 1;
			lmac_type[0] = -1;
			lmac_type[1] = -1;
			lmac_type[2] = 3;
			lmac_type[3] = 3;
			sds_lane[2] = 2;
			sds_lane[3] = 3;
			additional_lmacs = 2;
		}
		break;
	case CVMX_QLM_MODE_40G_KR4:
		enable_training = 1;
	case CVMX_QLM_MODE_XLAUI: /* 40GR4_1X4 */
		is_bgx = 5;
		lmac_type[0] = 4;
		lmac_type[1] = -1;
		lmac_type[2] = -1;
		lmac_type[3] = -1;
		sds_lane[0] = 0xe4;
		break;
	case CVMX_QLM_MODE_RGMII_SGMII:
		is_bgx = 1;
		lmac_type[0] = 5;
		lmac_type[1] = 0;
		lmac_type[2] = 0;
		lmac_type[3] = 0;
		sds_lane[0] = 0;
		sds_lane[1] = 1;
		sds_lane[2] = 2;
		sds_lane[3] = 3;
		break;
	case CVMX_QLM_MODE_RGMII_SGMII_1X1:
		if (qlm == 5) {
			is_bgx = 1;
			lmac_type[0] = 5;
			lmac_type[1] = 0;
			lmac_type[2] = -1;
			lmac_type[3] = -1;
			sds_lane[0] = 0;
			sds_lane[1] = 1;
		}
		break;
	case CVMX_QLM_MODE_RGMII_SGMII_2X1:
		if (qlm == 6) {
			is_bgx = 1;
			lmac_type[0] = 5;
			lmac_type[1] = -1;
			lmac_type[2] = 0;
			lmac_type[3] = 0;
			sds_lane[0] = 0;
			sds_lane[2] = 0;
			sds_lane[3] = 1;
		}
		break;
	case CVMX_QLM_MODE_RGMII_10G_KR:
		enable_training = 1;
	case CVMX_QLM_MODE_RGMII_XFI:
		is_bgx = 1;
		lmac_type[0] = 5;
		lmac_type[1] = 3;
		lmac_type[2] = 3;
		lmac_type[3] = 3;
		sds_lane[0] = 0;
		sds_lane[1] = 1;
		sds_lane[2] = 2;
		sds_lane[3] = 3;
		break;
	case CVMX_QLM_MODE_RGMII_10G_KR_1X1:
		enable_training = 1;
	case CVMX_QLM_MODE_RGMII_XFI_1X1:
		if (qlm == 5) {
			is_bgx = 3;
			lmac_type[0] = 5;
			lmac_type[1] = 3;
			lmac_type[2] = -1;
			lmac_type[3] = -1;
			sds_lane[0] = 0;
			sds_lane[1] = 1;
		}
		break;
	case CVMX_QLM_MODE_RGMII_40G_KR4:
		enable_training = 1;
	case CVMX_QLM_MODE_RGMII_XLAUI:
		is_bgx = 5;
		lmac_type[0] = 5;
		lmac_type[1] = 4;
		lmac_type[2] = -1;
		lmac_type[3] = -1;
		sds_lane[0] = 0x0;
		sds_lane[1] = 0xe4;
		break;
	case CVMX_QLM_MODE_RGMII_RXAUI:
		is_bgx = 3;
		lmac_type[0] = 5;
		lmac_type[1] = 2;
		lmac_type[2] = 2;
		lmac_type[3] = -1;
		sds_lane[0] = 0x0;
		sds_lane[1] = 0x4;
		sds_lane[2] = 0xe;
		break;
	case CVMX_QLM_MODE_RGMII_XAUI:
		is_bgx = 5;
		lmac_type[0] = 5;
		lmac_type[1] = 1;
		lmac_type[2] = -1;
		lmac_type[3] = -1;
		sds_lane[0] = 0;
		sds_lane[1] = 0xe4;
		break;
	default:
		break;
	}

	if (is_pcie == 0)
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz, &alt_pll);
	debug("%s: %d lane mode: %d, alternate PLL: %s\n", __func__, mode, lane_mode,
	      alt_pll ? "true" : "false");
	if (lane_mode == -1)
		return -1;

	if (alt_pll) {
		debug("%s: alternate PLL settings used for qlm %d, lane mode %d, reference clock %d\n",
		      __func__, qlm, lane_mode, ref_clk_sel);
		if (__set_qlm_ref_clk_cn78xx(0, qlm, lane_mode, ref_clk_sel)) {
			printf("%s: Error: reference clock %d is not supported for qlm %d, lane mode: 0x%x\n",
			       __func__, ref_clk_sel, qlm, lane_mode);
			return -1;
		}
	}

	/* Power up PHY, but keep it in reset */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_pd = 0;
	phy_ctl.s.phy_reset = 1;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	/* Set GSER for the interface mode */
	cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));
	cfg.s.bgx = is_bgx & 1;
	cfg.s.bgx_quad = (is_bgx >> 2) & 1;
	cfg.s.bgx_dual = (is_bgx >> 1) & 1;
	cfg.s.pcie = is_pcie;
	csr_wr(CVMX_GSERX_CFG(qlm), cfg.u64);

	/* Lane mode */
	lmode.u64 = csr_rd(CVMX_GSERX_LANE_MODE(qlm));
	lmode.s.lmode = lane_mode;
	csr_wr(CVMX_GSERX_LANE_MODE(qlm), lmode.u64);

	/* Program lmac_type to figure out the type of BGX interface configured */
	if (is_bgx) {
		int bgx = (qlm < 4) ? qlm - 2 : 2;
		cvmx_bgxx_cmrx_config_t cmr_config;
		cvmx_bgxx_cmr_rx_lmacs_t rx_lmacs;
		cvmx_bgxx_spux_br_pmd_control_t spu_pmd_control;
		int index, total_lmacs = 0;

		for (index = 0; index < 4; index++) {
			cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(index, bgx));
			cmr_config.s.enable = 0;
			cmr_config.s.data_pkt_rx_en = 0;
			cmr_config.s.data_pkt_tx_en = 0;
			if (lmac_type[index] != -1) {
				cmr_config.s.lmac_type = lmac_type[index];
				cmr_config.s.lane_to_sds = sds_lane[index];
				total_lmacs++;
				/* RXAUI takes up 2 lmacs */
				if (lmac_type[index] == 2)
					total_lmacs += 1;
			}
			csr_wr(CVMX_BGXX_CMRX_CONFIG(index, bgx), cmr_config.u64);

			/* Errata (TBD) RGMII doesn't turn on clock if its by
			 * itself. Force them on
			 */
			if (lmac_type[index] == 5) {
				cvmx_bgxx_cmr_global_config_t global_config;

				global_config.u64 = csr_rd(CVMX_BGXX_CMR_GLOBAL_CONFIG(bgx));
				global_config.s.bgx_clk_enable = 1;
				csr_wr(CVMX_BGXX_CMR_GLOBAL_CONFIG(bgx), global_config.u64);
			}

			/* Enable training for 10G_KR/40G_KR4 modes */
			if (enable_training == 1 &&
			    (lmac_type[index] == 3 || lmac_type[index] == 4)) {
				spu_pmd_control.u64 =
					csr_rd(CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx));
				spu_pmd_control.s.train_en = 1;
				csr_wr(CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx),
				       spu_pmd_control.u64);
			}
		}

		/* Update the total number of lmacs */
		rx_lmacs.u64 = csr_rd(CVMX_BGXX_CMR_RX_LMACS(bgx));
		rx_lmacs.s.lmacs = total_lmacs + additional_lmacs;
		csr_wr(CVMX_BGXX_CMR_RX_LMACS(bgx), rx_lmacs.u64);
		csr_wr(CVMX_BGXX_CMR_TX_LMACS(bgx), rx_lmacs.u64);
	}

	/* Bring phy out of reset */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 0;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	/*
	 * Wait 1us until the management interface is ready to accept
	 * read/write commands.
	 */
	udelay(1);

	/* Wait for reset to complete and the PLL to lock */
	/* PCIe mode doesn't become ready until the PEM block attempts to bring
	 * the interface up. Skip this check for PCIe
	 */
	if (!is_pcie && CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm),
					      cvmx_gserx_qlm_stat_t,
					      rst_rdy, ==, 1, 10000)) {
		printf("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", qlm);
		return -1;
	}

	/* Configure the gser pll */
	if (!is_pcie)
		__qlm_setup_pll_cn78xx(0, qlm);

	/* Wait for reset to complete and the PLL to lock */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_PLL_STAT(qlm), cvmx_gserx_pll_stat_t,
				  pll_lock, ==, 1, 10000)) {
		printf("QLM%d: Timeout waiting for GSERX_PLL_STAT[pll_lock]\n", qlm);
		return -1;
	}

	/* Errata GSER-26150: 10G PHY PLL Temperature Failure */
	/* This workaround must be completed after the final deassertion of
	 * GSERx_PHY_CTL[PHY_RESET].
	 * Apply the workaround to 10.3125Gbps and 8Gbps only.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_0) &&
	    (baud_mhz == 103125 || (is_pcie && gen3 == 2)))
		__qlm_errata_gser_26150(0, qlm, is_pcie);

	/* Errata GSER-26636: 10G-KR/40G-KR - Inverted Tx Coefficient Direction
	 * Change. Applied to all 10G standards (required for KR) but also
	 * applied to other standards in case software training is used
	 */
	if (baud_mhz == 103125)
		__qlm_kr_inc_dec_gser26636(0, qlm);

	/* Errata GSER-25992: RX EQ Default Settings Update (CTLE Bias) */
	/* This workaround will only be applied to Pass 1.x */
	/* It will also only be applied if the SERDES data-rate is 10G */
	/* or if PCIe Gen3 (gen3=2 is PCIe Gen3) */
	if (baud_mhz == 103125 || (is_pcie && gen3 == 2))
		cvmx_qlm_gser_errata_25992(0, qlm);

	/* Errata GSER-27140: Updating the RX EQ settings due to temperature
	 * drift sensitivities
	 */
	/* This workaround will also only be applied if the SERDES data-rate is 10G */
	if (baud_mhz == 103125)
		__qlm_rx_eq_temp_gser27140(0, qlm);

	/* Reduce the voltage amplitude coming from Marvell PHY and also change
	 * DFE threshold settings for RXAUI interface
	 */
	if (is_bgx) {
		int l;

		for (l = 0; l < 4; l++) {
			cvmx_gserx_lanex_rx_cfg_4_t cfg4;
			cvmx_gserx_lanex_tx_cfg_0_t cfg0;

			if (lmac_type[l] == 2) {
				/* Change the Q/QB error sampler 0 threshold from 0xD to 0xF */
				cfg4.u64 = csr_rd(CVMX_GSERX_LANEX_RX_CFG_4(l, qlm));
				cfg4.s.cfg_rx_errdet_ctrl = 0xcf6f;
				csr_wr(CVMX_GSERX_LANEX_RX_CFG_4(l, qlm), cfg4.u64);
				/* Reduce the voltage swing to roughly 460mV */
				cfg0.u64 = csr_rd(CVMX_GSERX_LANEX_TX_CFG_0(l, qlm));
				cfg0.s.cfg_tx_swing = 0x12;
				csr_wr(CVMX_GSERX_LANEX_TX_CFG_0(l, qlm), cfg0.u64);
			}
		}
	}

	return 0;
}

static int __rmac_pll_config(int baud_mhz, int qlm, int mode)
{
	cvmx_gserx_pll_px_mode_0_t pmode0;
	cvmx_gserx_pll_px_mode_1_t pmode1;
	cvmx_gserx_lane_px_mode_0_t lmode0;
	cvmx_gserx_lane_px_mode_1_t lmode1;
	cvmx_gserx_lane_mode_t lmode;

	switch (baud_mhz) {
	case 98304:
		pmode0.u64 = 0x1a0a;
		pmode1.u64 = 0x3228;
		lmode0.u64 = 0x600f;
		lmode1.u64 = 0xa80f;
		break;
	case 49152:
		if (mode == CVMX_QLM_MODE_SDL) {
			pmode0.u64 = 0x3605;
			pmode1.u64 = 0x0814;
			lmode0.u64 = 0x000f;
			lmode1.u64 = 0x6814;
		} else {
			pmode0.u64 = 0x1a0a;
			pmode1.u64 = 0x3228;
			lmode0.u64 = 0x650f;
			lmode1.u64 = 0xe80f;
		}
		break;
	case 24576:
		pmode0.u64 = 0x1a0a;
		pmode1.u64 = 0x3228;
		lmode0.u64 = 0x6a0f;
		lmode1.u64 = 0xe80f;
		break;
	case 12288:
		pmode0.u64 = 0x1a0a;
		pmode1.u64 = 0x3228;
		lmode0.u64 = 0x6f0f;
		lmode1.u64 = 0xe80f;
		break;
	case 6144:
		pmode0.u64 = 0x160a;
		pmode1.u64 = 0x1019;
		lmode0.u64 = 0x000f;
		lmode1.u64 = 0x2814;
		break;
	case 3072:
		pmode0.u64 = 0x160a;
		pmode1.u64 = 0x1019;
		lmode0.u64 = 0x050f;
		lmode1.u64 = 0x6814;
		break;
	default:
		printf("Invalid speed for CPRI/SDL configuration\n");
		return -1;
	}

	lmode.u64 = csr_rd(CVMX_GSERX_LANE_MODE(qlm));
	csr_wr(CVMX_GSERX_PLL_PX_MODE_0(lmode.s.lmode, qlm), pmode0.u64);
	csr_wr(CVMX_GSERX_PLL_PX_MODE_1(lmode.s.lmode, qlm), pmode1.u64);
	csr_wr(CVMX_GSERX_LANE_PX_MODE_0(lmode.s.lmode, qlm), lmode0.u64);
	csr_wr(CVMX_GSERX_LANE_PX_MODE_1(lmode.s.lmode, qlm), lmode1.u64);
	return 0;
}

/**
 * Configure QLM/DLM speed and mode for cnf75xx.
 *
 * @param qlm     The QLM to configure
 * @param baud_mhz   The speed the QLM needs to be configured in Mhz.
 * @param mode    The QLM to be configured as SGMII/XAUI/PCIe.
 * @param rc      Only used for PCIe, rc = 1 for root complex mode, 0 for EP mode.
 * @param gen3    Only used for PCIe
 *			gen3 = 2 GEN3 mode
 *			gen3 = 1 GEN2 mode
 *			gen3 = 0 GEN1 mode
 *
 * @param ref_clk_sel    The reference-clock selection to use to configure QLM
 *			 0 = REF_100MHZ
 *			 1 = REF_125MHZ
 *			 2 = REF_156MHZ
 *			 3 = REF_122MHZ
 * @param ref_clk_input  The reference-clock input to use to configure QLM
 *
 * Return:       Return 0 on success or -1.
 */
static int octeon_configure_qlm_cnf75xx(int qlm, int baud_mhz, int mode, int rc, int gen3,
					int ref_clk_sel, int ref_clk_input)
{
	cvmx_gserx_phy_ctl_t phy_ctl;
	cvmx_gserx_lane_mode_t lmode;
	cvmx_gserx_cfg_t cfg;
	cvmx_gserx_refclk_sel_t refclk_sel;
	int is_pcie = 0;
	int is_bgx = 0;
	int is_srio = 0;
	int is_rmac = 0;
	int is_rmac_pipe = 0;
	int lane_mode = 0;
	short lmac_type[4] = { 0 };
	short sds_lane[4] = { 0 };
	bool alt_pll = false;
	int enable_training = 0;
	int additional_lmacs = 0;
	int port = (qlm == 3) ? 1 : 0;
	cvmx_sriox_status_reg_t status_reg;

	debug("%s(qlm: %d, baud_mhz: %d, mode: %d, rc: %d, gen3: %d, ref_clk_sel: %d, ref_clk_input: %d\n",
	      __func__, qlm, baud_mhz, mode, rc, gen3, ref_clk_sel, ref_clk_input);
	if (qlm > 8) {
		printf("Invalid qlm%d passed\n", qlm);
		return -1;
	}

	/* Errata PEM-31375 PEM RSL accesses to PCLK registers can timeout
	 *  during speed change. Change SLI_WINDOW_CTL[time] to 525us
	 */
	__set_sli_window_ctl_errata_31375(0);

	cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));

	/* If PEM is in EP, no need to do anything */
	if (cfg.s.pcie && rc == 0) {
		debug("%s: qlm %d is in PCIe endpoint mode, returning\n", __func__, qlm);
		return 0;
	}

	if (cfg.s.srio && rc == 0) {
		debug("%s: qlm %d is in SRIO endpoint mode, returning\n", __func__, qlm);
		return 0;
	}

	/* Set the reference clock to use */
	refclk_sel.u64 = 0;
	if (ref_clk_input == 0) { /* External ref clock */
		refclk_sel.s.com_clk_sel = 0;
		refclk_sel.s.use_com1 = 0;
	} else if (ref_clk_input == 1) {
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 0;
	} else {
		refclk_sel.s.com_clk_sel = 1;
		refclk_sel.s.use_com1 = 1;
	}

	csr_wr(CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);

	/* Reset the QLM after changing the reference clock */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 1;
	phy_ctl.s.phy_pd = 1;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	udelay(1000);

	switch (mode) {
	case CVMX_QLM_MODE_PCIE:
	case CVMX_QLM_MODE_PCIE_1X2:
	case CVMX_QLM_MODE_PCIE_2X1: {
		cvmx_pemx_cfg_t pemx_cfg;
		cvmx_pemx_on_t pemx_on;
		cvmx_rst_soft_prstx_t rst_prst;

		is_pcie = 1;

		if (qlm > 1) {
			printf("Invalid PCIe mode for QLM%d\n", qlm);
			return -1;
		}

		if (ref_clk_sel == 0) {
			refclk_sel.u64 = csr_rd(CVMX_GSERX_REFCLK_SEL(qlm));
			refclk_sel.s.pcie_refclk125 = 0;
			csr_wr(CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);
			if (gen3 == 0) /* Gen1 mode */
				lane_mode = R_2_5G_REFCLK100;
			else if (gen3 == 1) /* Gen2 mode */
				lane_mode = R_5G_REFCLK100;
			else
				lane_mode = R_8G_REFCLK100;
		} else if (ref_clk_sel == 1) {
			refclk_sel.u64 = csr_rd(CVMX_GSERX_REFCLK_SEL(qlm));
			refclk_sel.s.pcie_refclk125 = 1;
			csr_wr(CVMX_GSERX_REFCLK_SEL(qlm), refclk_sel.u64);
			if (gen3 == 0) /* Gen1 mode */
				lane_mode = R_2_5G_REFCLK125;
			else if (gen3 == 1) /* Gen2 mode */
				lane_mode = R_5G_REFCLK125;
			else
				lane_mode = R_8G_REFCLK125;
		} else {
			printf("Invalid reference clock for PCIe on QLM%d\n", qlm);
			return -1;
		}

		switch (qlm) {
		case 0: /* Either x4 or x2 based on PEM0 */
			rst_prst.u64 = csr_rd(CVMX_RST_SOFT_PRSTX(0));
			rst_prst.s.soft_prst = rc;
			csr_wr(CVMX_RST_SOFT_PRSTX(0), rst_prst.u64);
			__setup_pem_reset(0, 0, !rc);

			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));
			pemx_cfg.cnf75xx.hostmd = rc;
			pemx_cfg.cnf75xx.lanes8 = (mode == CVMX_QLM_MODE_PCIE);
			pemx_cfg.cnf75xx.md = gen3;
			csr_wr(CVMX_PEMX_CFG(0), pemx_cfg.u64);
			/* x4 mode waits for QLM1 setup before turning on the PEM */
			if (mode == CVMX_QLM_MODE_PCIE_1X2 || mode == CVMX_QLM_MODE_PCIE_2X1) {
				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(0));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(0), pemx_on.u64);
			}
			break;
		case 1: /* Either PEM0 x4 or PEM1 x2 */
			if (mode == CVMX_QLM_MODE_PCIE_1X2 || mode == CVMX_QLM_MODE_PCIE_2X1) {
				rst_prst.u64 = csr_rd(CVMX_RST_SOFT_PRSTX(1));
				rst_prst.s.soft_prst = rc;
				csr_wr(CVMX_RST_SOFT_PRSTX(1), rst_prst.u64);
				__setup_pem_reset(0, 1, !rc);

				pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(1));
				pemx_cfg.cnf75xx.hostmd = rc;
				pemx_cfg.cnf75xx.md = gen3;
				csr_wr(CVMX_PEMX_CFG(1), pemx_cfg.u64);

				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(1));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(1), pemx_on.u64);
			} else {
				pemx_on.u64 = csr_rd(CVMX_PEMX_ON(0));
				pemx_on.s.pemon = 1;
				csr_wr(CVMX_PEMX_ON(0), pemx_on.u64);
			}
			break;
		default:
			break;
		}
		break;
	}
	case CVMX_QLM_MODE_SRIO_1X4:
	case CVMX_QLM_MODE_SRIO_2X2:
	case CVMX_QLM_MODE_SRIO_4X1: {
		int spd = 0xf;

		if (cvmx_fuse_read(1601)) {
			debug("SRIO is not supported on cnf73xx model\n");
			return -1;
		}

		switch (baud_mhz) {
		case 1250:
			switch (ref_clk_sel) {
			case 0: /* 100 MHz ref clock */
				spd = 0x3;
				break;
			case 1: /* 125 MHz ref clock */
				spd = 0xa;
				break;
			case 2: /* 156.25 MHz ref clock */
				spd = 0x4;
				break;
			default:
				spd = 0xf; /* Disabled */
				break;
			}
			break;
		case 2500:
			switch (ref_clk_sel) {
			case 0: /* 100 MHz ref clock */
				spd = 0x2;
				break;
			case 1: /* 125 MHz ref clock */
				spd = 0x9;
				break;
			case 2: /* 156.25 MHz ref clock */
				spd = 0x7;
				break;
			default:
				spd = 0xf; /* Disabled */
				break;
			}
			break;
		case 3125:
			switch (ref_clk_sel) {
			case 1: /* 125 MHz ref clock */
				spd = 0x8;
				break;
			case 2: /* 156.25 MHz ref clock */
				spd = 0xe;
				break;
			default:
				spd = 0xf; /* Disabled */
				break;
			}
			break;
		case 5000:
			switch (ref_clk_sel) {
			case 0: /* 100 MHz ref clock */
				spd = 0x0;
				break;
			case 1: /* 125 MHz ref clock */
				spd = 0x6;
				break;
			case 2: /* 156.25 MHz ref clock */
				spd = 0xb;
				break;
			default:
				spd = 0xf; /* Disabled */
				break;
			}
			break;
		default:
			spd = 0xf;
			break;
		}

		if (spd == 0xf) {
			printf("ERROR: Invalid SRIO speed (%d) configured for QLM%d\n", baud_mhz,
			       qlm);
			return -1;
		}

		status_reg.u64 = csr_rd(CVMX_SRIOX_STATUS_REG(port));
		status_reg.s.spd = spd;
		csr_wr(CVMX_SRIOX_STATUS_REG(port), status_reg.u64);
		is_srio = 1;
		break;
	}

	case CVMX_QLM_MODE_SGMII_2X1:
		if (qlm == 4) {
			is_bgx = 1;
			lmac_type[0] = 0;
			lmac_type[1] = 0;
			lmac_type[2] = -1;
			lmac_type[3] = -1;
			sds_lane[0] = 0;
			sds_lane[1] = 1;
		} else if (qlm == 5) {
			is_bgx = 1;
			lmac_type[0] = -1;
			lmac_type[1] = -1;
			lmac_type[2] = 0;
			lmac_type[3] = 0;
			sds_lane[2] = 2;
			sds_lane[3] = 3;
			additional_lmacs = 2;
		}
		break;
	case CVMX_QLM_MODE_10G_KR_1X2:
		enable_training = 1;
	case CVMX_QLM_MODE_XFI_1X2:
		if (qlm == 5) {
			is_bgx = 1;
			lmac_type[0] = -1;
			lmac_type[1] = -1;
			lmac_type[2] = 3;
			lmac_type[3] = 3;
			sds_lane[2] = 2;
			sds_lane[3] = 3;
			additional_lmacs = 2;
		}
		break;
	case CVMX_QLM_MODE_CPRI: /* CPRI / JESD204B */
		is_rmac = 1;
		break;
	case CVMX_QLM_MODE_SDL: /* Serdes Lite (SDL) */
		is_rmac = 1;
		is_rmac_pipe = 1;
		lane_mode = 1;
		break;
	default:
		break;
	}

	if (is_rmac_pipe == 0 && is_pcie == 0) {
		lane_mode = __get_lane_mode_for_speed_and_ref_clk(ref_clk_sel, baud_mhz,
								  &alt_pll);
	}

	debug("%s: %d lane mode: %d, alternate PLL: %s\n", __func__, mode, lane_mode,
	      alt_pll ? "true" : "false");
	if (lane_mode == -1)
		return -1;

	if (alt_pll) {
		debug("%s: alternate PLL settings used for qlm %d, lane mode %d, reference clock %d\n",
		      __func__, qlm, lane_mode, ref_clk_sel);
		if (__set_qlm_ref_clk_cn78xx(0, qlm, lane_mode, ref_clk_sel)) {
			printf("%s: Error: reference clock %d is not supported for qlm %d\n",
			       __func__, ref_clk_sel, qlm);
			return -1;
		}
	}

	/* Power up PHY, but keep it in reset */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_pd = 0;
	phy_ctl.s.phy_reset = 1;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	/* Set GSER for the interface mode */
	cfg.u64 = csr_rd(CVMX_GSERX_CFG(qlm));
	cfg.s.bgx = is_bgx & 1;
	cfg.s.bgx_quad = (is_bgx >> 2) & 1;
	cfg.s.bgx_dual = (is_bgx >> 1) & 1;
	cfg.s.pcie = is_pcie;
	cfg.s.srio = is_srio;
	cfg.s.rmac = is_rmac;
	cfg.s.rmac_pipe = is_rmac_pipe;
	csr_wr(CVMX_GSERX_CFG(qlm), cfg.u64);

	/* Lane mode */
	lmode.u64 = csr_rd(CVMX_GSERX_LANE_MODE(qlm));
	lmode.s.lmode = lane_mode;
	csr_wr(CVMX_GSERX_LANE_MODE(qlm), lmode.u64);

	/* Because of the Errata where quad mode does not work, program
	 * lmac_type to figure out the type of BGX interface configured
	 */
	if (is_bgx) {
		int bgx = 0;
		cvmx_bgxx_cmrx_config_t cmr_config;
		cvmx_bgxx_cmr_rx_lmacs_t rx_lmacs;
		cvmx_bgxx_spux_br_pmd_control_t spu_pmd_control;
		int index, total_lmacs = 0;

		for (index = 0; index < 4; index++) {
			cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(index, bgx));
			cmr_config.s.enable = 0;
			cmr_config.s.data_pkt_rx_en = 0;
			cmr_config.s.data_pkt_tx_en = 0;
			if (lmac_type[index] != -1) {
				cmr_config.s.lmac_type = lmac_type[index];
				cmr_config.s.lane_to_sds = sds_lane[index];
				total_lmacs++;
			}
			csr_wr(CVMX_BGXX_CMRX_CONFIG(index, bgx), cmr_config.u64);

			/* Enable training for 10G_KR/40G_KR4 modes */
			if (enable_training == 1 &&
			    (lmac_type[index] == 3 || lmac_type[index] == 4)) {
				spu_pmd_control.u64 =
					csr_rd(CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx));
				spu_pmd_control.s.train_en = 1;
				csr_wr(CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, bgx),
				       spu_pmd_control.u64);
			}
		}

		/* Update the total number of lmacs */
		rx_lmacs.u64 = csr_rd(CVMX_BGXX_CMR_RX_LMACS(bgx));
		rx_lmacs.s.lmacs = total_lmacs + additional_lmacs;
		csr_wr(CVMX_BGXX_CMR_RX_LMACS(bgx), rx_lmacs.u64);
		csr_wr(CVMX_BGXX_CMR_TX_LMACS(bgx), rx_lmacs.u64);
	}

	/* Bring phy out of reset */
	phy_ctl.u64 = csr_rd(CVMX_GSERX_PHY_CTL(qlm));
	phy_ctl.s.phy_reset = 0;
	csr_wr(CVMX_GSERX_PHY_CTL(qlm), phy_ctl.u64);

	/*
	 * Wait 1us until the management interface is ready to accept
	 * read/write commands.
	 */
	udelay(1);

	if (is_srio) {
		status_reg.u64 = csr_rd(CVMX_SRIOX_STATUS_REG(port));
		status_reg.s.srio = 1;
		csr_wr(CVMX_SRIOX_STATUS_REG(port), status_reg.u64);
		return 0;
	}

	/* Wait for reset to complete and the PLL to lock */
	/* PCIe mode doesn't become ready until the PEM block attempts to bring
	 * the interface up. Skip this check for PCIe
	 */
	if (!is_pcie && CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm), cvmx_gserx_qlm_stat_t,
					      rst_rdy, ==, 1, 10000)) {
		printf("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", qlm);
		return -1;
	}

	/* Configure the gser pll */
	if (is_rmac)
		__rmac_pll_config(baud_mhz, qlm, mode);
	else if (!(is_pcie || is_srio))
		__qlm_setup_pll_cn78xx(0, qlm);

	/* Wait for reset to complete and the PLL to lock */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_PLL_STAT(qlm), cvmx_gserx_pll_stat_t,
				  pll_lock, ==, 1, 10000)) {
		printf("QLM%d: Timeout waiting for GSERX_PLL_STAT[pll_lock]\n", qlm);
		return -1;
	}

	/* Errata GSER-27140: Updating the RX EQ settings due to temperature
	 * drift sensitivities
	 */
	/* This workaround will also only be applied if the SERDES data-rate is 10G */
	if (baud_mhz == 103125)
		__qlm_rx_eq_temp_gser27140(0, qlm);

	return 0;
}

/**
 * Configure qlm/dlm speed and mode.
 * @param qlm     The QLM or DLM to configure
 * @param speed   The speed the QLM needs to be configured in Mhz.
 * @param mode    The QLM to be configured as SGMII/XAUI/PCIe.
 * @param rc      Only used for PCIe, rc = 1 for root complex mode, 0 for EP
 *		  mode.
 * @param pcie_mode Only used when qlm/dlm are in pcie mode.
 * @param ref_clk_sel Reference clock to use for 70XX where:
 *			0: 100MHz
 *			1: 125MHz
 *			2: 156.25MHz
 *			3: 122MHz (Used by RMAC)
 * @param ref_clk_input	This selects which reference clock input to use.  For
 *			cn70xx:
 *				0: DLMC_REF_CLK0
 *				1: DLMC_REF_CLK1
 *				2: DLM0_REF_CLK
 *			cn61xx: (not used)
 *			cn78xx/cn76xx/cn73xx:
 *				0: Internal clock (QLM[0-7]_REF_CLK)
 *				1: QLMC_REF_CLK0
 *				2: QLMC_REF_CLK1
 *
 * Return:       Return 0 on success or -1.
 */
int octeon_configure_qlm(int qlm, int speed, int mode, int rc, int pcie_mode, int ref_clk_sel,
			 int ref_clk_input)
{
	int node = 0; // ToDo: corrently only node 0 is supported

	debug("%s(%d, %d, %d, %d, %d, %d, %d)\n", __func__, qlm, speed, mode, rc, pcie_mode,
	      ref_clk_sel, ref_clk_input);
	if (OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return octeon_configure_qlm_cn61xx(qlm, speed, mode, rc, pcie_mode);
	else if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return octeon_configure_qlm_cn70xx(qlm, speed, mode, rc, pcie_mode, ref_clk_sel,
						   ref_clk_input);
	else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return octeon_configure_qlm_cn78xx(node, qlm, speed, mode, rc, pcie_mode,
						   ref_clk_sel, ref_clk_input);
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return octeon_configure_qlm_cn73xx(qlm, speed, mode, rc, pcie_mode, ref_clk_sel,
						   ref_clk_input);
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return octeon_configure_qlm_cnf75xx(qlm, speed, mode, rc, pcie_mode, ref_clk_sel,
						    ref_clk_input);
	else
		return -1;
}

void octeon_init_qlm(int node)
{
	int qlm;
	cvmx_gserx_phy_ctl_t phy_ctl;
	cvmx_gserx_cfg_t cfg;
	int baud_mhz;
	int pem;

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX))
		return;

	for (qlm = 0; qlm < 8; qlm++) {
		phy_ctl.u64 = csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));
		if (phy_ctl.s.phy_reset == 0) {
			cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));
			if (cfg.s.pcie)
				__cvmx_qlm_pcie_errata_cn78xx(node, qlm);
			else
				__qlm_init_errata_20844(node, qlm);

			baud_mhz = cvmx_qlm_get_gbaud_mhz_node(node, qlm);
			if (baud_mhz == 6250 || baud_mhz == 6316)
				octeon_qlm_tune_v3(node, qlm, baud_mhz, 0xa, 0xa0, -1, -1);
			else if (baud_mhz == 103125)
				octeon_qlm_tune_v3(node, qlm, baud_mhz, 0xd, 0xd0, -1, -1);
		}
	}

	/* Setup how each PEM drives the PERST lines */
	for (pem = 0; pem < 4; pem++) {
		cvmx_rst_ctlx_t rst_ctl;

		rst_ctl.u64 = csr_rd_node(node, CVMX_RST_CTLX(pem));
		__setup_pem_reset(node, pem, !rst_ctl.s.host_mode);
	}
}
