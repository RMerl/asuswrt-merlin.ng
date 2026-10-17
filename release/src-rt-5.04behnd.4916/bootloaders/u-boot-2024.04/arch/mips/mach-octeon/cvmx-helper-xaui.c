// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for XAUI initialization, configuration,
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
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

int __cvmx_helper_xaui_enumerate(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	union cvmx_gmxx_hg2_control gmx_hg2_control;

	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		enum cvmx_qlm_mode qlm_mode =
			cvmx_qlm_get_dlm_mode(0, interface);

		if (qlm_mode == CVMX_QLM_MODE_RXAUI)
			return 1;
		return 0;
	}
	/* If HiGig2 is enabled return 16 ports, otherwise return 1 port */
	gmx_hg2_control.u64 = csr_rd(CVMX_GMXX_HG2_CONTROL(interface));
	if (gmx_hg2_control.s.hg2tx_en)
		return 16;
	else
		return 1;
}

/**
 * @INTERNAL
 * Probe a XAUI interface and determine the number of ports
 * connected to it. The XAUI interface should still be down
 * after this call.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_xaui_probe(int xiface)
{
	int i, ports;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	union cvmx_gmxx_inf_mode mode;

	/*
	 * CN63XX Pass 1.0 errata G-14395 requires the QLM De-emphasis
	 * be programmed.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_0)) {
		union cvmx_ciu_qlm2 ciu_qlm;

		ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM2);
		ciu_qlm.s.txbypass = 1;
		ciu_qlm.s.txdeemph = 0x5;
		ciu_qlm.s.txmargin = 0x1a;
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

		if (mio_rst_boot.cn63xx.qlm2_spd == 0xb) {
			union cvmx_ciu_qlm2 ciu_qlm;

			ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM2);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 0xa;
			ciu_qlm.s.txmargin = 0x1f;
			csr_wr(CVMX_CIU_QLM2, ciu_qlm.u64);
		}
	}

	/*
	 * Check if QLM is configured correct for XAUI/RXAUI, verify
	 * the speed as well as mode.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
		int qlm = cvmx_qlm_interface(xiface);
		enum cvmx_qlm_mode mode = cvmx_qlm_get_mode(qlm);

		if (mode != CVMX_QLM_MODE_XAUI && mode != CVMX_QLM_MODE_RXAUI)
			return 0;
	}

	ports = __cvmx_helper_xaui_enumerate(xiface);

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

	if (!OCTEON_IS_MODEL(OCTEON_CN68XX) &&
	    !OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		/*
		 * Setup PKO to support 16 ports for HiGig2 virtual
		 * ports. We're pointing all of the PKO packet ports
		 * for this interface to the XAUI. This allows us to
		 * use HiGig2 backpressure per port.
		 */
		for (i = 0; i < 16; i++) {
			union cvmx_pko_mem_port_ptrs pko_mem_port_ptrs;

			pko_mem_port_ptrs.u64 = 0;
			/*
			 * We set each PKO port to have equal priority
			 * in a round robin fashion.
			 */
			pko_mem_port_ptrs.s.static_p = 0;
			pko_mem_port_ptrs.s.qos_mask = 0xff;
			/* All PKO ports map to the same XAUI hardware port */
			pko_mem_port_ptrs.s.eid = interface * 4;
			pko_mem_port_ptrs.s.pid = interface * 16 + i;
			pko_mem_port_ptrs.s.bp_port = interface * 16 + i;
			csr_wr(CVMX_PKO_MEM_PORT_PTRS, pko_mem_port_ptrs.u64);
		}
	}

	return ports;
}

/**
 * @INTERNAL
 * Bringup XAUI interface. After this call packet I/O should be
 * fully functional.
 *
 * @param interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_xaui_link_init(int interface)
{
	union cvmx_gmxx_prtx_cfg gmx_cfg;
	union cvmx_pcsxx_control1_reg xaui_ctl;
	union cvmx_pcsxx_misc_ctl_reg misc_ctl;
	union cvmx_gmxx_tx_xaui_ctl tx_ctl;

	/* (1) Interface has already been enabled. */

	/* (2) Disable GMX. */
	misc_ctl.u64 = csr_rd(CVMX_PCSXX_MISC_CTL_REG(interface));
	misc_ctl.s.gmxeno = 1;
	csr_wr(CVMX_PCSXX_MISC_CTL_REG(interface), misc_ctl.u64);

	/* (3) Disable GMX and PCSX interrupts. */
	csr_wr(CVMX_GMXX_RXX_INT_EN(0, interface), 0x0);
	csr_wr(CVMX_GMXX_TX_INT_EN(interface), 0x0);
	csr_wr(CVMX_PCSXX_INT_EN_REG(interface), 0x0);

	/* (4) Bring up the PCSX and GMX reconciliation layer. */
	/* (4)a Set polarity and lane swapping. */
	/* (4)b */
	tx_ctl.u64 = csr_rd(CVMX_GMXX_TX_XAUI_CTL(interface));
	/* Enable better IFG packing and improves performance */
	tx_ctl.s.dic_en = 1;
	tx_ctl.s.uni_en = 0;
	csr_wr(CVMX_GMXX_TX_XAUI_CTL(interface), tx_ctl.u64);

	/* (4)c Aply reset sequence */
	xaui_ctl.u64 = csr_rd(CVMX_PCSXX_CONTROL1_REG(interface));
	xaui_ctl.s.lo_pwr = 0;

	/*
	 * Errata G-15618 requires disabling PCS soft reset in some
	 * OCTEON II models.
	 */
	if (!OCTEON_IS_MODEL(OCTEON_CN63XX) &&
	    !OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_X) &&
	    !OCTEON_IS_MODEL(OCTEON_CN68XX))
		xaui_ctl.s.reset = 1;
	csr_wr(CVMX_PCSXX_CONTROL1_REG(interface), xaui_ctl.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X) && interface != 1) {
		/*
		 * Note that GMX 1 was skipped as GMX0 is on the same
		 * QLM and will always be done first
		 *
		 * Workaround for Errata (G-16467).
		 */
		int qlm = interface;
#ifdef CVMX_QLM_DUMP_STATE
		debug("%s:%d: XAUI%d: Applying workaround for Errata G-16467\n",
		      __func__, __LINE__, qlm);
		cvmx_qlm_display_registers(qlm);
		debug("\n");
#endif
		/*
		 * This workaround only applies to QLMs running XAUI
		 * at 6.25Ghz
		 */
		if ((cvmx_qlm_get_gbaud_mhz(qlm) == 6250) &&
		    (cvmx_qlm_jtag_get(qlm, 0, "clkf_byp") != 20)) {
			/* Wait 100us for links to stabalize */
			udelay(100);
			cvmx_qlm_jtag_set(qlm, -1, "clkf_byp", 20);
			/* Allow the QLM to exit reset */
			cvmx_qlm_jtag_set(qlm, -1, "cfg_rst_n_clr", 0);
			/* Wait 100us for links to stabalize */
			udelay(100);
			/* Allow TX on QLM */
			cvmx_qlm_jtag_set(qlm, -1, "cfg_tx_idle_set", 0);
		}
#ifdef CVMX_QLM_DUMP_STATE
		debug("%s:%d: XAUI%d: Done applying workaround for Errata G-16467\n",
		      __func__, __LINE__, qlm);
		cvmx_qlm_display_registers(qlm);
		debug("\n\n");
#endif
	}

	/* Wait for PCS to come out of reset */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_PCSXX_CONTROL1_REG(interface),
				  cvmx_pcsxx_control1_reg_t, reset, ==, 0,
				  10000))
		return -1;
	/* Wait for PCS to be aligned */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_PCSXX_10GBX_STATUS_REG(interface),
				  cvmx_pcsxx_10gbx_status_reg_t, alignd, ==, 1,
				  10000))
		return -1;
	/* Wait for RX to be ready */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GMXX_RX_XAUI_CTL(interface),
				  cvmx_gmxx_rx_xaui_ctl_t, status, ==, 0,
				  10000))
		return -1;

	/* (6) Configure GMX */

	/* Wait for GMX RX to be idle */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GMXX_PRTX_CFG(0, interface),
				  cvmx_gmxx_prtx_cfg_t, rx_idle, ==, 1, 10000))
		return -1;
	/* Wait for GMX TX to be idle */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_GMXX_PRTX_CFG(0, interface),
				  cvmx_gmxx_prtx_cfg_t, tx_idle, ==, 1, 10000))
		return -1;

	/* GMX configure */
	gmx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(0, interface));
	gmx_cfg.s.speed = 1;
	gmx_cfg.s.speed_msb = 0;
	gmx_cfg.s.slottime = 1;
	csr_wr(CVMX_GMXX_TX_PRTS(interface), 1);
	csr_wr(CVMX_GMXX_TXX_SLOT(0, interface), 512);
	csr_wr(CVMX_GMXX_TXX_BURST(0, interface), 8192);
	csr_wr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

	/* Wait for receive link */
	if (CVMX_WAIT_FOR_FIELD64(CVMX_PCSXX_STATUS1_REG(interface),
				  cvmx_pcsxx_status1_reg_t, rcv_lnk, ==, 1,
				  10000))
		return -1;
	if (CVMX_WAIT_FOR_FIELD64(CVMX_PCSXX_STATUS2_REG(interface),
				  cvmx_pcsxx_status2_reg_t, xmtflt, ==, 0,
				  10000))
		return -1;
	if (CVMX_WAIT_FOR_FIELD64(CVMX_PCSXX_STATUS2_REG(interface),
				  cvmx_pcsxx_status2_reg_t, rcvflt, ==, 0,
				  10000))
		return -1;

	/* (8) Enable packet reception */
	misc_ctl.s.gmxeno = 0;
	csr_wr(CVMX_PCSXX_MISC_CTL_REG(interface), misc_ctl.u64);

	/* Clear all error interrupts before enabling the interface. */
	csr_wr(CVMX_GMXX_RXX_INT_REG(0, interface), ~0x0ull);
	csr_wr(CVMX_GMXX_TX_INT_REG(interface), ~0x0ull);
	csr_wr(CVMX_PCSXX_INT_REG(interface), ~0x0ull);

	/* Enable GMX */
	gmx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(0, interface));
	gmx_cfg.s.en = 1;
	csr_wr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

	return 0;
}

/**
 * @INTERNAL
 * Bringup and enable a XAUI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_xaui_enable(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;

	__cvmx_helper_setup_gmx(interface, 1);

	/* Setup PKND and BPID */
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		union cvmx_gmxx_bpid_msk bpid_msk;
		union cvmx_gmxx_bpid_mapx bpid_map;
		union cvmx_gmxx_prtx_cfg gmxx_prtx_cfg;
		union cvmx_gmxx_txx_append gmxx_txx_append_cfg;

		/* Setup PKIND */
		gmxx_prtx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(0, interface));
		gmxx_prtx_cfg.s.pknd = cvmx_helper_get_pknd(interface, 0);
		csr_wr(CVMX_GMXX_PRTX_CFG(0, interface), gmxx_prtx_cfg.u64);

		/* Setup BPID */
		bpid_map.u64 = csr_rd(CVMX_GMXX_BPID_MAPX(0, interface));
		bpid_map.s.val = 1;
		bpid_map.s.bpid = cvmx_helper_get_bpid(interface, 0);
		csr_wr(CVMX_GMXX_BPID_MAPX(0, interface), bpid_map.u64);

		bpid_msk.u64 = csr_rd(CVMX_GMXX_BPID_MSK(interface));
		bpid_msk.s.msk_or |= 1;
		bpid_msk.s.msk_and &= ~1;
		csr_wr(CVMX_GMXX_BPID_MSK(interface), bpid_msk.u64);

		/* CN68XX adds the padding and FCS in PKO, not GMX */
		gmxx_txx_append_cfg.u64 =
			csr_rd(CVMX_GMXX_TXX_APPEND(0, interface));
		gmxx_txx_append_cfg.s.fcs = 0;
		gmxx_txx_append_cfg.s.pad = 0;
		csr_wr(CVMX_GMXX_TXX_APPEND(0, interface),
		       gmxx_txx_append_cfg.u64);
	}

	/* 70XX eval boards use Marvel phy, set disparity accordingly. */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		union cvmx_gmxx_rxaui_ctl rxaui_ctl;

		rxaui_ctl.u64 = csr_rd(CVMX_GMXX_RXAUI_CTL(interface));
		rxaui_ctl.s.disparity = 1;
		csr_wr(CVMX_GMXX_RXAUI_CTL(interface), rxaui_ctl.u64);
	}

	__cvmx_helper_xaui_link_init(interface);

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
cvmx_helper_link_info_t __cvmx_helper_xaui_link_get(int ipd_port)
{
	int interface = cvmx_helper_get_interface_num(ipd_port);
	union cvmx_gmxx_tx_xaui_ctl gmxx_tx_xaui_ctl;
	union cvmx_gmxx_rx_xaui_ctl gmxx_rx_xaui_ctl;
	union cvmx_pcsxx_status1_reg pcsxx_status1_reg;
	cvmx_helper_link_info_t result;

	gmxx_tx_xaui_ctl.u64 = csr_rd(CVMX_GMXX_TX_XAUI_CTL(interface));
	gmxx_rx_xaui_ctl.u64 = csr_rd(CVMX_GMXX_RX_XAUI_CTL(interface));
	pcsxx_status1_reg.u64 = csr_rd(CVMX_PCSXX_STATUS1_REG(interface));
	result.u64 = 0;

	/* Only return a link if both RX and TX are happy */
	if (gmxx_tx_xaui_ctl.s.ls == 0 && gmxx_rx_xaui_ctl.s.status == 0 &&
	    pcsxx_status1_reg.s.rcv_lnk == 1) {
		union cvmx_pcsxx_misc_ctl_reg misc_ctl;

		result.s.link_up = 1;
		result.s.full_duplex = 1;
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			union cvmx_mio_qlmx_cfg qlm_cfg;
			int lanes;
			int qlm = (interface == 1) ? 0 : interface;

			qlm_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(qlm));
			result.s.speed = cvmx_qlm_get_gbaud_mhz(qlm) * 8 / 10;
			lanes = (qlm_cfg.s.qlm_cfg == 7) ? 2 : 4;
			result.s.speed *= lanes;
		} else if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
			int qlm = cvmx_qlm_interface(interface);

			result.s.speed = cvmx_qlm_get_gbaud_mhz(qlm) * 8 / 10;
			result.s.speed *= 4;
		} else {
			result.s.speed = 10000;
		}
		misc_ctl.u64 = csr_rd(CVMX_PCSXX_MISC_CTL_REG(interface));
		if (misc_ctl.s.gmxeno)
			__cvmx_helper_xaui_link_init(interface);
	} else {
		/* Disable GMX and PCSX interrupts. */
		csr_wr(CVMX_GMXX_RXX_INT_EN(0, interface), 0x0);
		csr_wr(CVMX_GMXX_TX_INT_EN(interface), 0x0);
		csr_wr(CVMX_PCSXX_INT_EN_REG(interface), 0x0);
	}
	return result;
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
int __cvmx_helper_xaui_link_set(int ipd_port, cvmx_helper_link_info_t link_info)
{
	int interface = cvmx_helper_get_interface_num(ipd_port);
	union cvmx_gmxx_tx_xaui_ctl gmxx_tx_xaui_ctl;
	union cvmx_gmxx_rx_xaui_ctl gmxx_rx_xaui_ctl;

	gmxx_tx_xaui_ctl.u64 = csr_rd(CVMX_GMXX_TX_XAUI_CTL(interface));
	gmxx_rx_xaui_ctl.u64 = csr_rd(CVMX_GMXX_RX_XAUI_CTL(interface));

	/* If the link shouldn't be up, then just return */
	if (!link_info.s.link_up)
		return 0;

	/* Do nothing if both RX and TX are happy */
	if (gmxx_tx_xaui_ctl.s.ls == 0 && gmxx_rx_xaui_ctl.s.status == 0)
		return 0;

	/* Bring the link up */
	return __cvmx_helper_xaui_link_init(interface);
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
extern int __cvmx_helper_xaui_configure_loopback(int ipd_port,
						 int enable_internal,
						 int enable_external)
{
	int interface = cvmx_helper_get_interface_num(ipd_port);
	union cvmx_pcsxx_control1_reg pcsxx_control1_reg;
	union cvmx_gmxx_xaui_ext_loopback gmxx_xaui_ext_loopback;

	/* Set the internal loop */
	pcsxx_control1_reg.u64 = csr_rd(CVMX_PCSXX_CONTROL1_REG(interface));
	pcsxx_control1_reg.s.loopbck1 = enable_internal;
	csr_wr(CVMX_PCSXX_CONTROL1_REG(interface), pcsxx_control1_reg.u64);

	/* Set the external loop */
	gmxx_xaui_ext_loopback.u64 =
		csr_rd(CVMX_GMXX_XAUI_EXT_LOOPBACK(interface));
	gmxx_xaui_ext_loopback.s.en = enable_external;
	csr_wr(CVMX_GMXX_XAUI_EXT_LOOPBACK(interface),
	       gmxx_xaui_ext_loopback.u64);

	/* Take the link through a reset */
	return __cvmx_helper_xaui_link_init(interface);
}
