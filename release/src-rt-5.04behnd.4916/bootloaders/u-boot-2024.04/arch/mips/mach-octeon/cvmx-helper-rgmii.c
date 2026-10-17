// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for RGMII/GMII/MII initialization, configuration,
 * and monitoring.
 */

#include <log.h>
#include <time.h>
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
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>

#include <mach/cvmx-hwpko.h>

#include <mach/cvmx-asxx-defs.h>
#include <mach/cvmx-dbg-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-npi-defs.h>
#include <mach/cvmx-pko-defs.h>

/**
 * @INTERNAL
 * Probe RGMII ports and determine the number present
 *
 * @param xiface Interface to probe
 *
 * @return Number of RGMII/GMII/MII ports (0-4).
 */
int __cvmx_helper_rgmii_probe(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int num_ports = 0;
	union cvmx_gmxx_inf_mode mode;

	mode.u64 = csr_rd(CVMX_GMXX_INF_MODE(xi.interface));

	if (mode.s.type)
		debug("ERROR: Unsupported Octeon model in %s\n", __func__);
	else
		debug("ERROR: Unsupported Octeon model in %s\n", __func__);
	return num_ports;
}

/**
 * @INTERNAL
 * Configure all of the ASX, GMX, and PKO regsiters required
 * to get RGMII to function on the supplied interface.
 *
 * @param xiface PKO Interface to configure (0 or 1)
 *
 * @return Zero on success
 */
int __cvmx_helper_rgmii_enable(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int num_ports = cvmx_helper_ports_on_interface(interface);
	int port;
	union cvmx_gmxx_inf_mode mode;
	union cvmx_asxx_tx_prt_en asx_tx;
	union cvmx_asxx_rx_prt_en asx_rx;

	mode.u64 = csr_rd(CVMX_GMXX_INF_MODE(interface));

	if (num_ports == -1)
		return -1;
	if (mode.s.en == 0)
		return -1;

	/* Configure the ASX registers needed to use the RGMII ports */
	asx_tx.u64 = 0;
	asx_tx.s.prt_en = cvmx_build_mask(num_ports);
	csr_wr(CVMX_ASXX_TX_PRT_EN(interface), asx_tx.u64);

	asx_rx.u64 = 0;
	asx_rx.s.prt_en = cvmx_build_mask(num_ports);
	csr_wr(CVMX_ASXX_RX_PRT_EN(interface), asx_rx.u64);

	/* Configure the GMX registers needed to use the RGMII ports */
	for (port = 0; port < num_ports; port++) {
		/*
		 * Configure more flexible RGMII preamble
		 * checking. Pass 1 doesn't support this feature.
		 */
		union cvmx_gmxx_rxx_frm_ctl frm_ctl;

		frm_ctl.u64 = csr_rd(CVMX_GMXX_RXX_FRM_CTL(port, interface));
		/* New field, so must be compile time */
		frm_ctl.s.pre_free = 1;
		csr_wr(CVMX_GMXX_RXX_FRM_CTL(port, interface), frm_ctl.u64);

		/*
		 * Each pause frame transmitted will ask for about 10M
		 * bit times before resume.  If buffer space comes
		 * available before that time has expired, an XON
		 * pause frame (0 time) will be transmitted to restart
		 * the flow.
		 */
		csr_wr(CVMX_GMXX_TXX_PAUSE_PKT_TIME(port, interface), 20000);
		csr_wr(CVMX_GMXX_TXX_PAUSE_PKT_INTERVAL(port, interface),
		       19000);

		csr_wr(CVMX_ASXX_TX_CLK_SETX(port, interface), 24);
		csr_wr(CVMX_ASXX_RX_CLK_SETX(port, interface), 24);
	}

	__cvmx_helper_setup_gmx(interface, num_ports);

	/* enable the ports now */
	for (port = 0; port < num_ports; port++) {
		union cvmx_gmxx_prtx_cfg gmx_cfg;

		cvmx_helper_link_autoconf(
			cvmx_helper_get_ipd_port(interface, port));
		gmx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(port, interface));
		gmx_cfg.s.en = 1;
		csr_wr(CVMX_GMXX_PRTX_CFG(port, interface), gmx_cfg.u64);
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
cvmx_helper_link_info_t __cvmx_helper_rgmii_link_get(int ipd_port)
{
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);
	union cvmx_asxx_prt_loop asxx_prt_loop;

	asxx_prt_loop.u64 = csr_rd(CVMX_ASXX_PRT_LOOP(interface));
	if (asxx_prt_loop.s.int_loop & (1 << index)) {
		/* Force 1Gbps full duplex on internal loopback */
		cvmx_helper_link_info_t result;

		result.u64 = 0;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		result.s.speed = 1000;
		return result;
	} else {
		return __cvmx_helper_board_link_get(ipd_port);
	}
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
cvmx_helper_link_info_t __cvmx_helper_gmii_link_get(int ipd_port)
{
	cvmx_helper_link_info_t result;
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	if (index == 0) {
		result = __cvmx_helper_rgmii_link_get(ipd_port);
	} else {
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		result.s.speed = 1000;
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
int __cvmx_helper_rgmii_link_set(int ipd_port,
				 cvmx_helper_link_info_t link_info)
{
	int result = 0;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);
	union cvmx_gmxx_prtx_cfg original_gmx_cfg;
	union cvmx_gmxx_prtx_cfg new_gmx_cfg;
	union cvmx_pko_mem_queue_qos pko_mem_queue_qos;
	union cvmx_pko_mem_queue_qos pko_mem_queue_qos_save[16];
	union cvmx_gmxx_tx_ovr_bp gmx_tx_ovr_bp;
	union cvmx_gmxx_tx_ovr_bp gmx_tx_ovr_bp_save;
	int i;

	/* Read the current settings so we know the current enable state */
	original_gmx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));
	new_gmx_cfg = original_gmx_cfg;

	/* Disable the lowest level RX */
	csr_wr(CVMX_ASXX_RX_PRT_EN(interface),
	       csr_rd(CVMX_ASXX_RX_PRT_EN(interface)) & ~(1 << index));

	memset(pko_mem_queue_qos_save, 0, sizeof(pko_mem_queue_qos_save));
	/* Disable all queues so that TX should become idle */
	for (i = 0; i < cvmx_pko_get_num_queues(ipd_port); i++) {
		int queue = cvmx_pko_get_base_queue(ipd_port) + i;

		csr_wr(CVMX_PKO_REG_READ_IDX, queue);
		pko_mem_queue_qos.u64 = csr_rd(CVMX_PKO_MEM_QUEUE_QOS);
		pko_mem_queue_qos.s.pid = ipd_port;
		pko_mem_queue_qos.s.qid = queue;
		pko_mem_queue_qos_save[i] = pko_mem_queue_qos;
		pko_mem_queue_qos.s.qos_mask = 0;
		csr_wr(CVMX_PKO_MEM_QUEUE_QOS, pko_mem_queue_qos.u64);
	}

	/* Disable backpressure */
	gmx_tx_ovr_bp.u64 = csr_rd(CVMX_GMXX_TX_OVR_BP(interface));
	gmx_tx_ovr_bp_save = gmx_tx_ovr_bp;
	gmx_tx_ovr_bp.s.bp &= ~(1 << index);
	gmx_tx_ovr_bp.s.en |= 1 << index;
	csr_wr(CVMX_GMXX_TX_OVR_BP(interface), gmx_tx_ovr_bp.u64);
	csr_rd(CVMX_GMXX_TX_OVR_BP(interface));

	/*
	 * Poll the GMX state machine waiting for it to become
	 * idle. Preferably we should only change speed when it is
	 * idle. If it doesn't become idle we will still do the speed
	 * change, but there is a slight chance that GMX will
	 * lockup.
	 */
	csr_wr(CVMX_NPI_DBG_SELECT, interface * 0x800 + index * 0x100 + 0x880);
	CVMX_WAIT_FOR_FIELD64(CVMX_DBG_DATA, cvmx_dbg_data_t, data & 7, ==, 0,
			      10000);
	CVMX_WAIT_FOR_FIELD64(CVMX_DBG_DATA, cvmx_dbg_data_t, data & 0xf, ==, 0,
			      10000);

	/* Disable the port before we make any changes */
	new_gmx_cfg.s.en = 0;
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), new_gmx_cfg.u64);
	csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));

	/* Set full/half duplex */
	if (!link_info.s.link_up)
		/* Force full duplex on down links */
		new_gmx_cfg.s.duplex = 1;
	else
		new_gmx_cfg.s.duplex = link_info.s.full_duplex;

	/* Set the link speed. Anything unknown is set to 1Gbps */
	if (link_info.s.speed == 10) {
		new_gmx_cfg.s.slottime = 0;
		new_gmx_cfg.s.speed = 0;
	} else if (link_info.s.speed == 100) {
		new_gmx_cfg.s.slottime = 0;
		new_gmx_cfg.s.speed = 0;
	} else {
		new_gmx_cfg.s.slottime = 1;
		new_gmx_cfg.s.speed = 1;
	}

	/* Adjust the clocks */
	if (link_info.s.speed == 10) {
		csr_wr(CVMX_GMXX_TXX_CLK(index, interface), 50);
		csr_wr(CVMX_GMXX_TXX_SLOT(index, interface), 0x40);
		csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 0);
	} else if (link_info.s.speed == 100) {
		csr_wr(CVMX_GMXX_TXX_CLK(index, interface), 5);
		csr_wr(CVMX_GMXX_TXX_SLOT(index, interface), 0x40);
		csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 0);
	} else {
		csr_wr(CVMX_GMXX_TXX_CLK(index, interface), 1);
		csr_wr(CVMX_GMXX_TXX_SLOT(index, interface), 0x200);
		csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 0x2000);
	}

	/* Do a read to make sure all setup stuff is complete */
	csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));

	/* Save the new GMX setting without enabling the port */
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), new_gmx_cfg.u64);

	/* Enable the lowest level RX */
	if (link_info.s.link_up)
		csr_wr(CVMX_ASXX_RX_PRT_EN(interface),
		       csr_rd(CVMX_ASXX_RX_PRT_EN(interface)) | (1 << index));

	/* Re-enable the TX path */
	for (i = 0; i < cvmx_pko_get_num_queues(ipd_port); i++) {
		int queue = cvmx_pko_get_base_queue(ipd_port) + i;

		csr_wr(CVMX_PKO_REG_READ_IDX, queue);
		csr_wr(CVMX_PKO_MEM_QUEUE_QOS, pko_mem_queue_qos_save[i].u64);
	}

	/* Restore backpressure */
	csr_wr(CVMX_GMXX_TX_OVR_BP(interface), gmx_tx_ovr_bp_save.u64);

	/* Restore the GMX enable state. Port config is complete */
	new_gmx_cfg.s.en = original_gmx_cfg.s.en;
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), new_gmx_cfg.u64);

	return result;
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
int __cvmx_helper_rgmii_configure_loopback(int ipd_port, int enable_internal,
					   int enable_external)
{
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);
	int original_enable;
	union cvmx_gmxx_prtx_cfg gmx_cfg;
	union cvmx_asxx_prt_loop asxx_prt_loop;

	/* Read the current enable state and save it */
	gmx_cfg.u64 = csr_rd(CVMX_GMXX_PRTX_CFG(index, interface));
	original_enable = gmx_cfg.s.en;
	/* Force port to be disabled */
	gmx_cfg.s.en = 0;
	if (enable_internal) {
		/* Force speed if we're doing internal loopback */
		gmx_cfg.s.duplex = 1;
		gmx_cfg.s.slottime = 1;
		gmx_cfg.s.speed = 1;
		csr_wr(CVMX_GMXX_TXX_CLK(index, interface), 1);
		csr_wr(CVMX_GMXX_TXX_SLOT(index, interface), 0x200);
		csr_wr(CVMX_GMXX_TXX_BURST(index, interface), 0x2000);
	}
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);

	/* Set the loopback bits */
	asxx_prt_loop.u64 = csr_rd(CVMX_ASXX_PRT_LOOP(interface));
	if (enable_internal)
		asxx_prt_loop.s.int_loop |= 1 << index;
	else
		asxx_prt_loop.s.int_loop &= ~(1 << index);
	if (enable_external)
		asxx_prt_loop.s.ext_loop |= 1 << index;
	else
		asxx_prt_loop.s.ext_loop &= ~(1 << index);
	csr_wr(CVMX_ASXX_PRT_LOOP(interface), asxx_prt_loop.u64);

	/* Force enables in internal loopback */
	if (enable_internal) {
		u64 tmp;

		tmp = csr_rd(CVMX_ASXX_TX_PRT_EN(interface));
		csr_wr(CVMX_ASXX_TX_PRT_EN(interface), (1 << index) | tmp);
		tmp = csr_rd(CVMX_ASXX_RX_PRT_EN(interface));
		csr_wr(CVMX_ASXX_RX_PRT_EN(interface), (1 << index) | tmp);
		original_enable = 1;
	}

	/* Restore the enable state */
	gmx_cfg.s.en = original_enable;
	csr_wr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);
	return 0;
}
