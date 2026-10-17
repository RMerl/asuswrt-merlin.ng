// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for SGMII initialization, configuration,
 * and monitoring.
 */

#include <time.h>
#include <log.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

/**
 * @INTERNAL
 * Perform initialization required only once for an SGMII port.
 *
 * @param interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_sgmii_hardware_init_one_time(int interface, int index)
{
	const u64 clock_mhz = 1200; /* todo: fixme */
	union cvmx_pcsx_miscx_ctl_reg pcsx_miscx_ctl_reg;
	union cvmx_pcsx_linkx_timer_count_reg pcsx_linkx_timer_count_reg;
	union cvmx_gmxx_prtx_cfg gmxx_prtx_cfg;

	if (!cvmx_helper_is_port_valid(interface, index))
		return 0;

	/* Disable GMX */
	gmxx_prtx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));
	gmxx_prtx_cfg.s.en = 0;
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), gmxx_prtx_cfg.u64);

	/*
	 * Write PCS*_LINK*_TIMER_COUNT_REG[COUNT] with the
	 * appropriate value. 1000BASE-X specifies a 10ms
	 * interval. SGMII specifies a 1.6ms interval.
	 */
	pcsx_miscx_ctl_reg.u64 =
		csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));
	/* Adjust the MAC mode if requested by device tree */
	pcsx_miscx_ctl_reg.s.mac_phy =
		cvmx_helper_get_mac_phy_mode(interface, index);
	pcsx_miscx_ctl_reg.s.mode =
		cvmx_helper_get_1000x_mode(interface, index);
	csr_wr(CVMX_PCSX_MISCX_CTL_REG(index, interface),
	       pcsx_miscx_ctl_reg.u64);

	pcsx_linkx_timer_count_reg.u64 =
		csr_rd(CVMX_PCSX_LINKX_TIMER_COUNT_REG(index, interface));
	if (pcsx_miscx_ctl_reg.s.mode)
		/* 1000BASE-X */
		pcsx_linkx_timer_count_reg.s.count =
			(10000ull * clock_mhz) >> 10;
	else
		/* SGMII */
		pcsx_linkx_timer_count_reg.s.count =
			(1600ull * clock_mhz) >> 10;

	csr_wr(CVMX_PCSX_LINKX_TIMER_COUNT_REG(index, interface),
	       pcsx_linkx_timer_count_reg.u64);

	/*
	 * Write the advertisement register to be used as the
	 * tx_Config_Reg<D15:D0> of the autonegotiation.  In
	 * 1000BASE-X mode, tx_Config_Reg<D15:D0> is PCS*_AN*_ADV_REG.
	 * In SGMII PHY mode, tx_Config_Reg<D15:D0> is
	 * PCS*_SGM*_AN_ADV_REG.  In SGMII MAC mode,
	 * tx_Config_Reg<D15:D0> is the fixed value 0x4001, so this
	 * step can be skipped.
	 */
	if (pcsx_miscx_ctl_reg.s.mode) {
		/* 1000BASE-X */
		union cvmx_pcsx_anx_adv_reg pcsx_anx_adv_reg;

		pcsx_anx_adv_reg.u64 =
			csr_rd(CVMX_PCSX_ANX_ADV_REG(index, interface));
		pcsx_anx_adv_reg.s.rem_flt = 0;
		pcsx_anx_adv_reg.s.pause = 3;
		pcsx_anx_adv_reg.s.hfd = 1;
		pcsx_anx_adv_reg.s.fd = 1;
		csr_wr(CVMX_PCSX_ANX_ADV_REG(index, interface),
		       pcsx_anx_adv_reg.u64);
	} else {
		if (pcsx_miscx_ctl_reg.s.mac_phy) {
			/* PHY Mode */
			union cvmx_pcsx_sgmx_an_adv_reg pcsx_sgmx_an_adv_reg;

			pcsx_sgmx_an_adv_reg.u64 = csr_rd(
				CVMX_PCSX_SGMX_AN_ADV_REG(index, interface));
			pcsx_sgmx_an_adv_reg.s.dup = 1;
			pcsx_sgmx_an_adv_reg.s.speed = 2;
			csr_wr(CVMX_PCSX_SGMX_AN_ADV_REG(index, interface),
			       pcsx_sgmx_an_adv_reg.u64);
		} else {
			/* MAC Mode - Nothing to do */
		}
	}
	return 0;
}

static int __cvmx_helper_need_g15618(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN63XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_X) ||
	    OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 1;
	else
		return 0;
}

/**
 * @INTERNAL
 * Initialize the SERTES link for the first time or after a loss
 * of link.
 *
 * @param interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_sgmii_hardware_init_link(int interface, int index)
{
	union cvmx_pcsx_mrx_control_reg control_reg;
	union cvmx_pcsx_miscx_ctl_reg pcsx_miscx_ctl_reg;
	bool phy_mode;
	bool an_disable; /** Disable autonegotiation */
	bool mode_1000x; /** 1000Base-X mode */

	if (!cvmx_helper_is_port_valid(interface, index))
		return 0;

	/*
	 * Take PCS through a reset sequence.
	 * PCS*_MR*_CONTROL_REG[PWR_DN] should be cleared to zero.
	 * Write PCS*_MR*_CONTROL_REG[RESET]=1 (while not changing the
	 * value of the other PCS*_MR*_CONTROL_REG bits).  Read
	 * PCS*_MR*_CONTROL_REG[RESET] until it changes value to
	 * zero.
	 */
	control_reg.u64 = csr_rd(CVMX_PCSX_MRX_CONTROL_REG(index, interface));

	/*
	 * Errata G-15618 requires disabling PCS soft reset in CN63XX
	 * pass upto 2.1.
	 */
	if (!__cvmx_helper_need_g15618()) {
		control_reg.s.reset = 1;
		csr_wr(CVMX_PCSX_MRX_CONTROL_REG(index, interface),
		       control_reg.u64);
		if (CVMX_WAIT_FOR_FIELD64(
			    CVMX_PCSX_MRX_CONTROL_REG(index, interface),
			    cvmx_pcsx_mrx_control_reg_t, reset, ==, 0, 10000)) {
			debug("SGMII%x: Timeout waiting for port %d to finish reset\n",
			      interface, index);
			return -1;
		}
	}

	/*
	 * Write PCS*_MR*_CONTROL_REG[RST_AN]=1 to ensure a fresh
	 * sgmii negotiation starts.
	 */
	phy_mode = cvmx_helper_get_mac_phy_mode(interface, index);
	an_disable = (phy_mode ||
		      !cvmx_helper_get_port_autonegotiation(interface, index));

	control_reg.s.an_en = !an_disable;

	/* Force a PCS reset by powering down the PCS interface
	 * This is needed to deal with broken Qualcomm/Atheros PHYs and switches
	 * which never recover if PCS is not power cycled.  The alternative
	 * is to power cycle or hardware reset the Qualcomm devices whenever
	 * SGMII is initialized.
	 *
	 * This is needed for the QCA8033 PHYs as well as the QCA833X switches
	 * to work.  The QCA8337 switch has additional SGMII problems and is
	 * best avoided if at all possible.  Failure to power cycle PCS prevents
	 * any traffic from flowing between Octeon and Qualcomm devices if there
	 * is a warm reset.  Even a software reset to the Qualcomm device will
	 * not work.
	 *
	 * Note that this problem has been reported between Qualcomm and other
	 * vendor's processors as well so this problem is not unique to
	 * Qualcomm and Octeon.
	 *
	 * Power cycling PCS doesn't hurt anything with non-Qualcomm devices
	 * other than adding a 25ms delay during initialization.
	 */
	control_reg.s.pwr_dn = 1;
	csr_wr(CVMX_PCSX_MRX_CONTROL_REG(index, interface), control_reg.u64);
	csr_rd(CVMX_PCSX_MRX_CONTROL_REG(index, interface));

	/* 25ms should be enough, 10ms is too short */
	mdelay(25);

	control_reg.s.pwr_dn = 0;
	csr_wr(CVMX_PCSX_MRX_CONTROL_REG(index, interface), control_reg.u64);

	/* The Cortina PHY runs in 1000base-X mode */
	mode_1000x = cvmx_helper_get_1000x_mode(interface, index);
	pcsx_miscx_ctl_reg.u64 =
		csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));
	pcsx_miscx_ctl_reg.s.mode = mode_1000x;
	pcsx_miscx_ctl_reg.s.mac_phy = phy_mode;
	csr_wr(CVMX_PCSX_MISCX_CTL_REG(index, interface),
	       pcsx_miscx_ctl_reg.u64);
	if (an_disable)
		/* In PHY mode we can't query the link status so we just
		 * assume that the link is up.
		 */
		return 0;

	/*
	 * Wait for PCS*_MR*_STATUS_REG[AN_CPT] to be set, indicating
	 * that sgmii autonegotiation is complete. In MAC mode this
	 * isn't an ethernet link, but a link between Octeon and the
	 * PHY.
	 */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_PCSX_MRX_STATUS_REG(index, interface),
				  union cvmx_pcsx_mrx_status_reg, an_cpt, ==, 1,
				  10000)) {
		debug("SGMII%x: Port %d link timeout\n", interface, index);
		return -1;
	}
	return 0;
}

/**
 * @INTERNAL
 * Configure an SGMII link to the specified speed after the SERTES
 * link is up.
 *
 * @param interface to init
 * @param index     Index of prot on the interface
 * @param link_info Link state to configure
 *
 * @return Zero on success, negative on failure
 */
static int
__cvmx_helper_sgmii_hardware_init_link_speed(int interface, int index,
					     cvmx_helper_link_info_t link_info)
{
	int is_enabled;
	union cvmx_gmxx_prtx_cfg gmxx_prtx_cfg;
	union cvmx_pcsx_miscx_ctl_reg pcsx_miscx_ctl_reg;

	if (!cvmx_helper_is_port_valid(interface, index))
		return 0;

	/* Disable GMX before we make any changes. Remember the enable state */
	gmxx_prtx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));
	is_enabled = gmxx_prtx_cfg.s.en;
	gmxx_prtx_cfg.s.en = 0;
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), gmxx_prtx_cfg.u64);

	/* Wait for GMX to be idle */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GMXX_PRTX_CFG(index, interface),
				  cvmx_gmxx_prtx_cfg_t, rx_idle, ==, 1,
				  10000) ||
	    CVMX_WAIT_FOR_FIELD64(CVMX_GMXX_PRTX_CFG(index, interface),
				  cvmx_gmxx_prtx_cfg_t, tx_idle, ==, 1,
				  10000)) {
		debug("SGMII%d: Timeout waiting for port %d to be idle\n",
		      interface, index);
		return -1;
	}

	/* Read GMX CFG again to make sure the disable completed */
	gmxx_prtx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));

	/*
	 * Get the misc control for PCS. We will need to set the
	 * duplication amount.
	 */
	pcsx_miscx_ctl_reg.u64 =
		csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));

	/*
	 * Use GMXENO to force the link down if the status we get says
	 * it should be down.
	 */
	pcsx_miscx_ctl_reg.s.gmxeno = !link_info.s.link_up;

	/* Only change the duplex setting if the link is up */
	if (link_info.s.link_up)
		gmxx_prtx_cfg.s.duplex = link_info.s.full_duplex;

	/* Do speed based setting for GMX */
	switch (link_info.s.speed) {
	case 10:
		gmxx_prtx_cfg.s.speed = 0;
		gmxx_prtx_cfg.s.speed_msb = 1;
		gmxx_prtx_cfg.s.slottime = 0;
		/* Setting from GMX-603 */
		pcsx_miscx_ctl_reg.s.samp_pt = 25;
		csr_wr(CVMX_GMXX_TXX_SLOT(index, interface), 64);
		csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 0);
		break;
	case 100:
		gmxx_prtx_cfg.s.speed = 0;
		gmxx_prtx_cfg.s.speed_msb = 0;
		gmxx_prtx_cfg.s.slottime = 0;
		pcsx_miscx_ctl_reg.s.samp_pt = 0x5;
		csr_wr(CVMX_GMXX_TXX_SLOT(index, interface), 64);
		csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 0);
		break;
	case 1000:
		gmxx_prtx_cfg.s.speed = 1;
		gmxx_prtx_cfg.s.speed_msb = 0;
		gmxx_prtx_cfg.s.slottime = 1;
		pcsx_miscx_ctl_reg.s.samp_pt = 1;
		csr_wr(CVMX_GMXX_TXX_SLOT(index, interface), 512);
		if (gmxx_prtx_cfg.s.duplex)
			/* full duplex */
			csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 0);
		else
			/* half duplex */
			csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 8192);
		break;
	default:
		break;
	}

	/* Write the new misc control for PCS */
	csr_wr(CVMX_PCSX_MISCX_CTL_REG(index, interface),
	       pcsx_miscx_ctl_reg.u64);

	/* Write the new GMX settings with the port still disabled */
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), gmxx_prtx_cfg.u64);

	/* Read GMX CFG again to make sure the config completed */
	gmxx_prtx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));

	/* Restore the enabled / disabled state */
	gmxx_prtx_cfg.s.en = is_enabled;
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), gmxx_prtx_cfg.u64);

	return 0;
}

/**
 * @INTERNAL
 * Bring up the SGMII interface to be ready for packet I/O but
 * leave I/O disabled using the GMX override. This function
 * follows the bringup documented in 10.6.3 of the manual.
 *
 * @param interface to bringup
 * @param num_ports Number of ports on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_sgmii_hardware_init(int interface, int num_ports)
{
	int index;
	int do_link_set = 1;

	/*
	 * CN63XX Pass 1.0 errata G-14395 requires the QLM De-emphasis
	 * be programmed.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_0)) {
		union cvmx_ciu_qlm2 ciu_qlm;

		ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM2);
		ciu_qlm.s.txbypass = 1;
		ciu_qlm.s.txdeemph = 0xf;
		ciu_qlm.s.txmargin = 0xd;
		csr_wr(CVMX_CIU_QLM2, ciu_qlm.u64);
	}

	/*
	 * CN63XX Pass 2.x errata G-15273 requires the QLM De-emphasis
	 * be programmed when using a 156.25Mhz ref clock.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS2_X)) {
		/* Read the QLM speed pins */
		union cvmx_mio_rst_boot mio_rst_boot;

		mio_rst_boot.u64 = csr_rd(CVMX_MIO_RST_BOOT);

		if (mio_rst_boot.cn63xx.qlm2_spd == 4) {
			union cvmx_ciu_qlm2 ciu_qlm;

			ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM2);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 0x0;
			ciu_qlm.s.txmargin = 0xf;
			csr_wr(CVMX_CIU_QLM2, ciu_qlm.u64);
		}
	}

	__cvmx_helper_setup_gmx(interface, num_ports);

	for (index = 0; index < num_ports; index++) {
		int ipd_port = cvmx_helper_get_ipd_port(interface, index);

		if (!cvmx_helper_is_port_valid(interface, index))
			continue;
		__cvmx_helper_sgmii_hardware_init_one_time(interface, index);
		if (do_link_set)
			__cvmx_helper_sgmii_link_set(ipd_port,
						     __cvmx_helper_sgmii_link_get(ipd_port));
	}

	return 0;
}

int __cvmx_helper_sgmii_enumerate(int xiface)
{
	if (OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return 2;
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		struct cvmx_xiface xi =
			cvmx_helper_xiface_to_node_interface(xiface);
		enum cvmx_qlm_mode qlm_mode =
			cvmx_qlm_get_dlm_mode(0, xi.interface);

		if (qlm_mode == CVMX_QLM_MODE_SGMII)
			return 1;
		else if (qlm_mode == CVMX_QLM_MODE_QSGMII)
			return 4;
		return 0;
	}
	return 4;
}

/**
 * @INTERNAL
 * Probe a SGMII interface and determine the number of ports
 * connected to it. The SGMII interface should still be down after
 * this call.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_sgmii_probe(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	union cvmx_gmxx_inf_mode mode;
	int ports;

	/*
	 * Check if QLM is configured correct for SGMII, verify the
	 * speed as well as mode.
	 */
	if (OCTEON_IS_OCTEON2()) {
		int qlm = cvmx_qlm_interface(xiface);

		if (cvmx_qlm_get_mode(qlm) != CVMX_QLM_MODE_SGMII)
			return 0;
	}

	/* Do not enable the interface if is not in SGMII mode */
	ports = __cvmx_helper_sgmii_enumerate(xiface);

	if (ports <= 0)
		return 0;

	/*
	 * Due to errata GMX-700 on CN56XXp1.x and CN52XXp1.x, the
	 * interface needs to be enabled before IPD otherwise per port
	 * backpressure may not work properly.
	 */
	mode.u64 = csr_rd(CVMX_GMXX_INF_MODE(interface));
	mode.s.en = 1;
	csr_wr(CVMX_GMXX_INF_MODE(interface), mode.u64);

	return ports;
}

/**
 * @INTERNAL
 * Bringup and enable a SGMII interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_sgmii_enable(int xiface)
{
	int num_ports = cvmx_helper_ports_on_interface(xiface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int index;

	/* Setup PKND and BPID */
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		for (index = 0; index < num_ports; index++) {
			union cvmx_gmxx_bpid_msk bpid_msk;
			union cvmx_gmxx_bpid_mapx bpid_map;
			union cvmx_gmxx_prtx_cfg gmxx_prtx_cfg;

			if (!cvmx_helper_is_port_valid(interface, index))
				continue;
			/* Setup PKIND */
			gmxx_prtx_cfg.u64 =
				csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));
			gmxx_prtx_cfg.s.pknd =
				cvmx_helper_get_pknd(interface, index);
			csr_wr(CVMX_GMXX_PRTX_CFG(index, interface),
			       gmxx_prtx_cfg.u64);

			/* Setup BPID */
			bpid_map.u64 =
				csr_rd(CVMX_GMXX_BPID_MAPX(index, interface));
			bpid_map.s.val = 1;
			bpid_map.s.bpid =
				cvmx_helper_get_bpid(interface, index);
			csr_wr(CVMX_GMXX_BPID_MAPX(index, interface),
			       bpid_map.u64);

			bpid_msk.u64 = csr_rd(CVMX_GMXX_BPID_MSK(interface));
			bpid_msk.s.msk_or |= (1 << index);
			bpid_msk.s.msk_and &= ~(1 << index);
			csr_wr(CVMX_GMXX_BPID_MSK(interface), bpid_msk.u64);
		}
	}

	__cvmx_helper_sgmii_hardware_init(interface, num_ports);

	/* CN68XX adds the padding and FCS in PKO, not GMX */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		union cvmx_gmxx_txx_append gmxx_txx_append_cfg;

		for (index = 0; index < num_ports; index++) {
			if (!cvmx_helper_is_port_valid(interface, index))
				continue;
			gmxx_txx_append_cfg.u64 =
				csr_rd(CVMX_GMXX_TXX_APPEND(index, interface));
			gmxx_txx_append_cfg.s.fcs = 0;
			gmxx_txx_append_cfg.s.pad = 0;
			csr_wr(CVMX_GMXX_TXX_APPEND(index, interface),
			       gmxx_txx_append_cfg.u64);
		}
	}

	/* Enable running disparity check for QSGMII interface */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX) && num_ports > 1) {
		union cvmx_gmxx_qsgmii_ctl qsgmii_ctl;

		qsgmii_ctl.u64 = 0;
		qsgmii_ctl.s.disparity = 1;
		csr_wr(CVMX_GMXX_QSGMII_CTL(interface), qsgmii_ctl.u64);
	}

	for (index = 0; index < num_ports; index++) {
		union cvmx_gmxx_txx_append append_cfg;
		union cvmx_gmxx_txx_sgmii_ctl sgmii_ctl;
		union cvmx_gmxx_prtx_cfg gmxx_prtx_cfg;

		if (!cvmx_helper_is_port_valid(interface, index))
			continue;
		/*
		 * Clear the align bit if preamble is set to attain
		 * maximum tx rate.
		 */
		append_cfg.u64 = csr_rd(CVMX_GMXX_TXX_APPEND(index, interface));
		sgmii_ctl.u64 =
			csr_rd(CVMX_GMXX_TXX_SGMII_CTL(index, interface));
		sgmii_ctl.s.align = append_cfg.s.preamble ? 0 : 1;
		csr_wr(CVMX_GMXX_TXX_SGMII_CTL(index, interface),
		       sgmii_ctl.u64);

		gmxx_prtx_cfg.u64 =
			csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));
		gmxx_prtx_cfg.s.en = 1;
		csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), gmxx_prtx_cfg.u64);
	}
	return 0;
}

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set().
 *
 * @param ipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t __cvmx_helper_sgmii_link_get(int ipd_port)
{
	cvmx_helper_link_info_t result;
	union cvmx_pcsx_miscx_ctl_reg pcsx_miscx_ctl_reg;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);
	union cvmx_pcsx_mrx_control_reg pcsx_mrx_control_reg;
	int speed = 1000;
	int qlm;

	result.u64 = 0;

	if (!cvmx_helper_is_port_valid(interface, index))
		return result;

	if (OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		union cvmx_gmxx_inf_mode inf_mode;

		inf_mode.u64 = csr_rd(CVMX_GMXX_INF_MODE(interface));
		if (inf_mode.s.rate & (1 << index))
			speed = 2500;
		else
			speed = 1000;
	} else if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
		qlm = cvmx_qlm_interface(interface);
		speed = cvmx_qlm_get_gbaud_mhz(qlm) * 8 / 10;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		speed = cvmx_qlm_get_gbaud_mhz(0) * 8 / 10;
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		speed = cvmx_qlm_get_gbaud_mhz(0) * 8 / 10;
		if (cvmx_qlm_get_dlm_mode(0, interface) == CVMX_QLM_MODE_SGMII)
			speed >>= 1;
		else
			speed >>= 2;
	}

	pcsx_mrx_control_reg.u64 =
		csr_rd(CVMX_PCSX_MRX_CONTROL_REG(index, interface));
	if (pcsx_mrx_control_reg.s.loopbck1) {
		/* Force 1Gbps full duplex link for internal loopback */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = speed;
		return result;
	}

	pcsx_miscx_ctl_reg.u64 =
		csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));
	if (pcsx_miscx_ctl_reg.s.mac_phy ||
	    cvmx_helper_get_port_force_link_up(interface, index)) {
		/* PHY Mode */
		/* Note that this also works for 1000base-X mode */

		result.s.speed = speed;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		return result;
	}

	/* MAC Mode */
	return __cvmx_helper_board_link_get(ipd_port);
}

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead.
 *
 * @param ipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_sgmii_link_set(int ipd_port,
				 cvmx_helper_link_info_t link_info)
{
	union cvmx_pcsx_mrx_control_reg control_reg;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	if (!cvmx_helper_is_port_valid(interface, index))
		return 0;

	/* For some devices, i.e. the Qualcomm QCA8337 switch we need to power
	 * down the PCS interface when the link goes down and power it back
	 * up when the link returns.
	 */
	if (link_info.s.link_up || !__cvmx_helper_need_g15618()) {
		__cvmx_helper_sgmii_hardware_init_link(interface, index);
	} else {
		union cvmx_pcsx_miscx_ctl_reg pcsx_miscx_ctl_reg;

		pcsx_miscx_ctl_reg.u64 =
			csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));

		/* Disable autonegotiation when MAC mode is enabled or
		 * autonegotiation is disabled.
		 */
		control_reg.u64 =
			csr_rd(CVMX_PCSX_MRX_CONTROL_REG(index, interface));
		if (pcsx_miscx_ctl_reg.s.mac_phy == 0 ||
		    !cvmx_helper_get_port_autonegotiation(interface, index)) {
			control_reg.s.an_en = 0;
			control_reg.s.spdmsb = 1;
			control_reg.s.spdlsb = 0;
			control_reg.s.dup = 1;
		}
		csr_wr(CVMX_PCSX_MRX_CONTROL_REG(index, interface),
		       control_reg.u64);
		csr_rd(CVMX_PCSX_MRX_CONTROL_REG(index, interface));
		/*
		 * Use GMXENO to force the link down it will get
		 * reenabled later...
		 */
		pcsx_miscx_ctl_reg.s.gmxeno = 1;
		csr_wr(CVMX_PCSX_MISCX_CTL_REG(index, interface),
		       pcsx_miscx_ctl_reg.u64);
		csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));
		return 0;
	}
	return __cvmx_helper_sgmii_hardware_init_link_speed(interface, index,
							    link_info);
}

/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again.
 *
 * @param ipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
int __cvmx_helper_sgmii_configure_loopback(int ipd_port, int enable_internal,
					   int enable_external)
{
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);
	union cvmx_pcsx_mrx_control_reg pcsx_mrx_control_reg;
	union cvmx_pcsx_miscx_ctl_reg pcsx_miscx_ctl_reg;

	if (!cvmx_helper_is_port_valid(interface, index))
		return 0;

	pcsx_mrx_control_reg.u64 =
		csr_rd(CVMX_PCSX_MRX_CONTROL_REG(index, interface));
	pcsx_mrx_control_reg.s.loopbck1 = enable_internal;
	csr_wr(CVMX_PCSX_MRX_CONTROL_REG(index, interface),
	       pcsx_mrx_control_reg.u64);

	pcsx_miscx_ctl_reg.u64 =
		csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));
	pcsx_miscx_ctl_reg.s.loopbck2 = enable_external;
	csr_wr(CVMX_PCSX_MISCX_CTL_REG(index, interface),
	       pcsx_miscx_ctl_reg.u64);

	__cvmx_helper_sgmii_hardware_init_link(interface, index);
	return 0;
}
