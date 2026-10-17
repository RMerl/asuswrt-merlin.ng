// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions to configure the BGX MAC.
 */

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
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-fdt.h>
#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

#include <mach/cvmx-global-resources.h>
#include <mach/cvmx-pko-internal-ports-range.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pip.h>

/* Enable this define to see BGX error messages */
/*#define DEBUG_BGX */

/* Enable this variable to trace functions called for initializing BGX */
static const int debug;

/**
 * cvmx_helper_bgx_override_autoneg(int xiface, int index) is a function pointer
 * to override enabling/disabling of autonegotiation for SGMII, 10G-KR or 40G-KR4
 * interfaces. This function is called when interface is initialized.
 */
int (*cvmx_helper_bgx_override_autoneg)(int xiface, int index) = NULL;

/*
 * cvmx_helper_bgx_override_fec(int xiface) is a function pointer
 * to override enabling/disabling of FEC for 10G interfaces. This function
 * is called when interface is initialized.
 */
int (*cvmx_helper_bgx_override_fec)(int xiface, int index) = NULL;

/**
 * Delay after enabling an interface based on the mode.  Different modes take
 * different amounts of time.
 */
static void
__cvmx_helper_bgx_interface_enable_delay(cvmx_helper_interface_mode_t mode)
{
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
		mdelay(250);
		break;
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		mdelay(100);
		break;
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		mdelay(50);
		break;
	default:
		mdelay(50);
		break;
	}
}

/**
 * @INTERNAL
 *
 * Returns number of ports based on interface
 * @param xiface Which xiface
 * @return Number of ports based on xiface
 */
int __cvmx_helper_bgx_enumerate(int xiface)
{
	cvmx_bgxx_cmr_tx_lmacs_t lmacs;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	lmacs.u64 = csr_rd_node(xi.node, CVMX_BGXX_CMR_TX_LMACS(xi.interface));
	return lmacs.s.lmacs;
}

/**
 * @INTERNAL
 *
 * Returns mode of each BGX LMAC (port).
 * This is different than 'cvmx_helper_interface_get_mode()' which
 * provides mode of an entire interface, but when BGX is in "mixed"
 * mode this function should be called instead to get the protocol
 * for each port (BGX LMAC) individually.
 * Both function return the same enumerated mode.
 *
 * @param xiface is the global interface identifier
 * @param index is the interface port index
 * @returns mode of the individual port
 */
cvmx_helper_interface_mode_t cvmx_helper_bgx_get_mode(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;

	cmr_config.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));

	switch (cmr_config.s.lmac_type) {
	case 0:
		return CVMX_HELPER_INTERFACE_MODE_SGMII;
	case 1:
		return CVMX_HELPER_INTERFACE_MODE_XAUI;
	case 2:
		return CVMX_HELPER_INTERFACE_MODE_RXAUI;
	case 3:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return cvmx_helper_interface_get_mode(xiface);
		pmd_control.u64 = csr_rd_node(
			xi.node,
			CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, xi.interface));
		if (pmd_control.s.train_en)
			return CVMX_HELPER_INTERFACE_MODE_10G_KR;
		else
			return CVMX_HELPER_INTERFACE_MODE_XFI;
		break;
	case 4:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return cvmx_helper_interface_get_mode(xiface);
		pmd_control.u64 = csr_rd_node(
			xi.node,
			CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, xi.interface));
		if (pmd_control.s.train_en)
			return CVMX_HELPER_INTERFACE_MODE_40G_KR4;
		else
			return CVMX_HELPER_INTERFACE_MODE_XLAUI;
		break;
	case 5:
		return CVMX_HELPER_INTERFACE_MODE_RGMII;
	default:
		return CVMX_HELPER_INTERFACE_MODE_DISABLED;
	}
}

static int __cvmx_helper_bgx_rgmii_speed(cvmx_helper_link_info_t link_info)
{
	cvmx_xcv_reset_t xcv_reset;
	cvmx_xcv_ctl_t xcv_ctl;
	cvmx_xcv_batch_crd_ret_t crd_ret;
	cvmx_xcv_dll_ctl_t dll_ctl;
	cvmx_xcv_comp_ctl_t comp_ctl;
	int speed;
	int up = link_info.s.link_up;
	int do_credits;

	if (link_info.s.speed == 100)
		speed = 1;
	else if (link_info.s.speed == 10)
		speed = 0;
	else
		speed = 2;

	xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
	xcv_ctl.u64 = csr_rd(CVMX_XCV_CTL);
	do_credits = up && !xcv_reset.s.enable;

	if (xcv_ctl.s.lpbk_int) {
		xcv_reset.s.clkrst = 0;
		csr_wr(CVMX_XCV_RESET, xcv_reset.u64);
	}

	if (up && (!xcv_reset.s.enable || xcv_ctl.s.speed != speed)) {
		if (debug)
			debug("%s: *** Enabling XCV block\n", __func__);
		/* Enable the XCV block */
		xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
		xcv_reset.s.enable = 1;
		csr_wr(CVMX_XCV_RESET, xcv_reset.u64);

		/* Set operating mode */
		xcv_ctl.u64 = csr_rd(CVMX_XCV_CTL);
		xcv_ctl.s.speed = speed;
		csr_wr(CVMX_XCV_CTL, xcv_ctl.u64);

		/* Configure DLL - enable or bypass */
		/* TX no bypass, RX bypass */
		dll_ctl.u64 = csr_rd(CVMX_XCV_DLL_CTL);
		dll_ctl.s.clkrx_set = 0;
		dll_ctl.s.clkrx_byp = 1;
		dll_ctl.s.clktx_byp = 0;
		csr_wr(CVMX_XCV_DLL_CTL, dll_ctl.u64);

		/* Enable */
		dll_ctl.u64 = csr_rd(CVMX_XCV_DLL_CTL);
		dll_ctl.s.refclk_sel = 0;
		csr_wr(CVMX_XCV_DLL_CTL, dll_ctl.u64);
		xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
		xcv_reset.s.dllrst = 0;
		csr_wr(CVMX_XCV_RESET, xcv_reset.u64);

		/* Delay deems to be need so XCV_DLL_CTL[CLK_SET] works */
		udelay(10);

		comp_ctl.u64 = csr_rd(CVMX_XCV_COMP_CTL);
		//comp_ctl.s.drv_pctl = 0;
		//comp_ctl.s.drv_nctl = 0;
		comp_ctl.s.drv_byp = 0;
		csr_wr(CVMX_XCV_COMP_CTL, comp_ctl.u64);

		/* enable */
		xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
		xcv_reset.s.comp = 1;
		csr_wr(CVMX_XCV_RESET, xcv_reset.u64);

		/* setup the RXC */
		xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
		xcv_reset.s.clkrst = !xcv_ctl.s.lpbk_int;
		csr_wr(CVMX_XCV_RESET, xcv_reset.u64);

		/* datapaths come out of the reset
		 * - the datapath resets will disengage BGX from the RGMII
		 *   interface
		 * - XCV will continue to return TX credits for each tick that
		 *   is sent on the TX data path
		 */
		xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
		xcv_reset.s.tx_dat_rst_n = 1;
		xcv_reset.s.rx_dat_rst_n = 1;
		csr_wr(CVMX_XCV_RESET, xcv_reset.u64);
	} else if (debug) {
		debug("%s: *** Not enabling XCV\n", __func__);
		debug("  up: %s, xcv_reset.s.enable: %d, xcv_ctl.s.speed: %d, speed: %d\n",
		      up ? "true" : "false", (unsigned int)xcv_reset.s.enable,
		      (unsigned int)xcv_ctl.s.speed, speed);
	}

	/* enable the packet flow
	 * - The packet resets will be only disengage on packet boundaries
	 * - XCV will continue to return TX credits for each tick that is
	 *   sent on the TX datapath
	 */
	xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
	xcv_reset.s.tx_pkt_rst_n = up;
	xcv_reset.s.rx_pkt_rst_n = up;
	csr_wr(CVMX_XCV_RESET, xcv_reset.u64);

	/* Full reset when link is down */
	if (!up) {
		if (debug)
			debug("%s: *** Disabling XCV reset\n", __func__);
		/* wait 2*MTU in time */
		mdelay(10);
		/* reset the world */
		csr_wr(CVMX_XCV_RESET, 0);
	}

	/* grant PKO TX credits */
	if (do_credits) {
		crd_ret.u64 = csr_rd(CVMX_XCV_BATCH_CRD_RET);
		crd_ret.s.crd_ret = 1;
		csr_wr(CVMX_XCV_BATCH_CRD_RET, crd_ret.u64);
	}

	return 0;
}

static void __cvmx_bgx_common_init_pknd(int xiface, int index)
{
	int num_ports;
	int num_chl = 16;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	int pknd;
	cvmx_bgxx_cmrx_rx_bp_on_t bgx_rx_bp_on;
	cvmx_bgxx_cmrx_rx_id_map_t cmr_rx_id_map;
	cvmx_bgxx_cmr_chan_msk_and_t chan_msk_and;
	cvmx_bgxx_cmr_chan_msk_or_t chan_msk_or;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	num_ports = cvmx_helper_ports_on_interface(xiface);
	/* Modify bp_on mark, depending on number of LMACS on that interface
	 * and write it for every port
	 */
	bgx_rx_bp_on.u64 = 0;
	bgx_rx_bp_on.s.mark = (CVMX_BGX_RX_FIFO_SIZE / (num_ports * 4 * 16));

	/* Setup pkind */
	pknd = cvmx_helper_get_pknd(xiface, index);
	cmr_rx_id_map.u64 = csr_rd_node(
		node, CVMX_BGXX_CMRX_RX_ID_MAP(index, xi.interface));
	cmr_rx_id_map.s.pknd = pknd;
	/* Change the default reassembly id (RID), as max 14 RIDs allowed */
	if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		cmr_rx_id_map.s.rid = ((4 * xi.interface) + 2 + index);
	csr_wr_node(node, CVMX_BGXX_CMRX_RX_ID_MAP(index, xi.interface),
		    cmr_rx_id_map.u64);
	/* Set backpressure channel mask AND/OR registers */
	chan_msk_and.u64 =
		csr_rd_node(node, CVMX_BGXX_CMR_CHAN_MSK_AND(xi.interface));
	chan_msk_or.u64 =
		csr_rd_node(node, CVMX_BGXX_CMR_CHAN_MSK_OR(xi.interface));
	chan_msk_and.s.msk_and |= ((1 << num_chl) - 1) << (16 * index);
	chan_msk_or.s.msk_or |= ((1 << num_chl) - 1) << (16 * index);
	csr_wr_node(node, CVMX_BGXX_CMR_CHAN_MSK_AND(xi.interface),
		    chan_msk_and.u64);
	csr_wr_node(node, CVMX_BGXX_CMR_CHAN_MSK_OR(xi.interface),
		    chan_msk_or.u64);
	/* set rx back pressure (bp_on) on value */
	csr_wr_node(node, CVMX_BGXX_CMRX_RX_BP_ON(index, xi.interface),
		    bgx_rx_bp_on.u64);
}

/**
 * @INTERNAL
 * Probe a SGMII interface and determine the number of ports
 * connected to it. The SGMII interface should still be down after
 * this call. This is used by interfaces using the bgx mac.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_bgx_probe(int xiface)
{
	return __cvmx_helper_bgx_enumerate(xiface);
}

/**
 * @INTERNAL
 * Return the size of the BGX TX_FIFO for a given LMAC,
 * or 0 if the requested LMAC is inactive.
 *
 * TBD: Need also to add a "__cvmx_helper_bgx_speed()" function to
 * return the speed of each LMAC.
 */
int __cvmx_helper_bgx_fifo_size(int xiface, unsigned int lmac)
{
	cvmx_bgxx_cmr_tx_lmacs_t lmacs;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned int tx_fifo_size = CVMX_BGX_TX_FIFO_SIZE;

	/* FIXME: Add validation for interface# < BGX_count */
	lmacs.u64 = csr_rd_node(xi.node, CVMX_BGXX_CMR_TX_LMACS(xi.interface));

	switch (lmacs.s.lmacs) {
	case 1:
		if (lmac > 0)
			return 0;
		else
			return tx_fifo_size;
	case 2:
		if (lmac > 1)
			return 0;
		else
			return tx_fifo_size >> 1;
	case 3:
		if (lmac > 2)
			return 0;
		else
			return tx_fifo_size >> 2;
	case 4:
		if (lmac > 3)
			return 0;
		else
			return tx_fifo_size >> 2;
	default:
		return 0;
	}
}

/**
 * @INTERNAL
 * Perform initialization required only once for an SGMII port.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_one_time(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	const u64 clock_mhz = 1200; /* todo: fixme */
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	cvmx_bgxx_gmp_pcs_linkx_timer_t gmp_timer;

	if (!cvmx_helper_is_port_valid(xi.interface, index))
		return 0;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	/*
	 * Write PCS*_LINK*_TIMER_COUNT_REG[COUNT] with the
	 * appropriate value. 1000BASE-X specifies a 10ms
	 * interval. SGMII specifies a 1.6ms interval.
	 */
	gmp_misc_ctl.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));
	/* Adjust the MAC mode if requested by device tree */
	gmp_misc_ctl.s.mac_phy = cvmx_helper_get_mac_phy_mode(xiface, index);
	gmp_misc_ctl.s.mode = cvmx_helper_get_1000x_mode(xiface, index);
	csr_wr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface),
		    gmp_misc_ctl.u64);

	gmp_timer.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_LINKX_TIMER(index, xi.interface));
	if (gmp_misc_ctl.s.mode)
		/* 1000BASE-X */
		gmp_timer.s.count = (10000ull * clock_mhz) >> 10;
	else
		/* SGMII */
		gmp_timer.s.count = (1600ull * clock_mhz) >> 10;

	csr_wr_node(node, CVMX_BGXX_GMP_PCS_LINKX_TIMER(index, xi.interface),
		    gmp_timer.u64);

	/*
	 * Write the advertisement register to be used as the
	 * tx_Config_Reg<D15:D0> of the autonegotiation.  In
	 * 1000BASE-X mode, tx_Config_Reg<D15:D0> is PCS*_AN*_ADV_REG.
	 * In SGMII PHY mode, tx_Config_Reg<D15:D0> is
	 * PCS*_SGM*_AN_ADV_REG.  In SGMII MAC mode,
	 * tx_Config_Reg<D15:D0> is the fixed value 0x4001, so this
	 * step can be skipped.
	 */
	if (gmp_misc_ctl.s.mode) {
		/* 1000BASE-X */
		cvmx_bgxx_gmp_pcs_anx_adv_t gmp_an_adv;

		gmp_an_adv.u64 = csr_rd_node(
			node, CVMX_BGXX_GMP_PCS_ANX_ADV(index, xi.interface));
		gmp_an_adv.s.rem_flt = 0;
		gmp_an_adv.s.pause = 3;
		gmp_an_adv.s.hfd = 1;
		gmp_an_adv.s.fd = 1;
		csr_wr_node(node,
			    CVMX_BGXX_GMP_PCS_ANX_ADV(index, xi.interface),
			    gmp_an_adv.u64);
	} else {
		if (gmp_misc_ctl.s.mac_phy) {
			/* PHY Mode */
			cvmx_bgxx_gmp_pcs_sgmx_an_adv_t gmp_sgmx_an_adv;

			gmp_sgmx_an_adv.u64 =
				csr_rd_node(node, CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(
							  index, xi.interface));
			gmp_sgmx_an_adv.s.dup = 1;
			gmp_sgmx_an_adv.s.speed = 2;
			csr_wr_node(node,
				    CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(index,
								  xi.interface),
				    gmp_sgmx_an_adv.u64);
		} else {
			/* MAC Mode - Nothing to do */
		}
	}
	return 0;
}

/**
 * @INTERNAL
 * Bring up the SGMII interface to be ready for packet I/O but
 * leave I/O disabled using the GMX override. This function
 * follows the bringup documented in 10.6.3 of the manual.
 *
 * @param xiface Interface to bringup
 * @param num_ports Number of ports on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init(int xiface, int num_ports)
{
	int index;
	int do_link_set = 1;

	for (index = 0; index < num_ports; index++) {
		int xipd_port = cvmx_helper_get_ipd_port(xiface, index);
		cvmx_helper_interface_mode_t mode;

		if (!cvmx_helper_is_port_valid(xiface, index))
			continue;

		__cvmx_helper_bgx_port_init(xipd_port, 0);

		mode = cvmx_helper_bgx_get_mode(xiface, index);
		if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
			continue;

		if (do_link_set)
			__cvmx_helper_bgx_sgmii_link_set(
				xipd_port,
				__cvmx_helper_bgx_sgmii_link_get(xipd_port));
	}

	return 0;
}

/**
 * @INTERNAL
 * Bringup and enable a SGMII interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled. This is used by interfaces using
 * the bgx mac.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_enable(int xiface)
{
	int num_ports;

	num_ports = cvmx_helper_ports_on_interface(xiface);
	__cvmx_helper_bgx_sgmii_hardware_init(xiface, num_ports);

	return 0;
}

/**
 * @INTERNAL
 * Initialize the SERDES link for the first time or after a loss
 * of link.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_link(int xiface, int index)
{
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	cvmx_bgxx_cmrx_config_t cmr_config;
	int phy_mode, mode_1000x;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int autoneg = 0;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	gmp_control.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface));
	/* Take PCS through a reset sequence */
	gmp_control.s.reset = 1;
	csr_wr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface),
		    gmp_control.u64);

	/* Wait until GMP_PCS_MRX_CONTROL[reset] comes out of reset */
	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface),
		    cvmx_bgxx_gmp_pcs_mrx_control_t, reset, ==, 0, 10000)) {
		debug("SGMII%d: Timeout waiting for port %d to finish reset\n",
		      interface, index);
		return -1;
	}

	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));

	gmp_control.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface));
	if (cvmx_helper_get_port_phy_present(xiface, index)) {
		gmp_control.s.pwr_dn = 0;
	} else {
		gmp_control.s.spdmsb = 1;
		gmp_control.s.spdlsb = 0;
		gmp_control.s.pwr_dn = 0;
	}
	/* Write GMP_PCS_MR*_CONTROL[RST_AN]=1 to ensure a fresh SGMII
	 * negotiation starts.
	 */
	autoneg = cvmx_helper_get_port_autonegotiation(xiface, index);
	gmp_control.s.rst_an = 1;
	gmp_control.s.an_en = (cmr_config.s.lmac_type != 5) && autoneg;
	csr_wr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface),
		    gmp_control.u64);

	phy_mode = cvmx_helper_get_mac_phy_mode(xiface, index);
	mode_1000x = cvmx_helper_get_1000x_mode(xiface, index);

	gmp_misc_ctl.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));
	gmp_misc_ctl.s.mac_phy = phy_mode;
	gmp_misc_ctl.s.mode = mode_1000x;
	csr_wr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface),
		    gmp_misc_ctl.u64);

	if (phy_mode || !autoneg)
		/* In PHY mode we can't query the link status so we just
		 * assume that the link is up
		 */
		return 0;

	/* Wait for GMP_PCS_MRX_CONTROL[an_cpt] to be set, indicating that
	 * SGMII autonegotiation is complete. In MAC mode this isn't an
	 * ethernet link, but a link between OCTEON and PHY.
	 */
	if (cmr_config.s.lmac_type != 5 &&
	    CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_GMP_PCS_MRX_STATUS(index, xi.interface),
		    cvmx_bgxx_gmp_pcs_mrx_status_t, an_cpt, ==, 1, 10000)) {
		debug("SGMII%d: Port %d link timeout\n", interface, index);
		return -1;
	}

	return 0;
}

/**
 * @INTERNAL
 * Configure an SGMII link to the specified speed after the SERDES
 * link is up.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 * @param link_info Link state to configure
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_link_speed(
	int xiface, int index, cvmx_helper_link_info_t link_info)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_miscx_ctl;
	cvmx_bgxx_gmp_gmi_prtx_cfg_t gmp_prtx_cfg;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	/* Disable GMX before we make any changes. */
	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		    cmr_config.u64);

	/* Wait for GMX to be idle */
	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
		    cvmx_bgxx_gmp_gmi_prtx_cfg_t, rx_idle, ==, 1, 10000) ||
	    CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
		    cvmx_bgxx_gmp_gmi_prtx_cfg_t, tx_idle, ==, 1, 10000)) {
		debug("SGMII%d:%d: Timeout waiting for port %d to be idle\n",
		      node, xi.interface, index);
		return -1;
	}

	/* Read GMX CFG again to make sure the disable completed */
	gmp_prtx_cfg.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface));

	/*
	 * Get the misc control for PCS. We will need to set the
	 * duplication amount.
	 */
	gmp_miscx_ctl.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));

	/*
	 * Use GMXENO to force the link down if the status we get says
	 * it should be down.
	 */
	gmp_miscx_ctl.s.gmxeno = !link_info.s.link_up;

	/* Only change the duplex setting if the link is up */
	if (link_info.s.link_up)
		gmp_prtx_cfg.s.duplex = link_info.s.full_duplex;

	/* Do speed based setting for GMX */
	switch (link_info.s.speed) {
	case 10:
		gmp_prtx_cfg.s.speed = 0;
		gmp_prtx_cfg.s.speed_msb = 1;
		gmp_prtx_cfg.s.slottime = 0;
		/* Setting from GMX-603 */
		gmp_miscx_ctl.s.samp_pt = 25;
		csr_wr_node(node,
			    CVMX_BGXX_GMP_GMI_TXX_SLOT(index, xi.interface),
			    64);
		csr_wr_node(node,
			    CVMX_BGXX_GMP_GMI_TXX_BURST(index, xi.interface),
			    0);
		break;
	case 100:
		gmp_prtx_cfg.s.speed = 0;
		gmp_prtx_cfg.s.speed_msb = 0;
		gmp_prtx_cfg.s.slottime = 0;
		gmp_miscx_ctl.s.samp_pt = 0x5;
		csr_wr_node(node,
			    CVMX_BGXX_GMP_GMI_TXX_SLOT(index, xi.interface),
			    64);
		csr_wr_node(node,
			    CVMX_BGXX_GMP_GMI_TXX_BURST(index, xi.interface),
			    0);
		break;
	case 1000:
		gmp_prtx_cfg.s.speed = 1;
		gmp_prtx_cfg.s.speed_msb = 0;
		gmp_prtx_cfg.s.slottime = 1;
		gmp_miscx_ctl.s.samp_pt = 1;
		csr_wr_node(node,
			    CVMX_BGXX_GMP_GMI_TXX_SLOT(index, xi.interface),
			    512);
		if (gmp_prtx_cfg.s.duplex)
			/* full duplex */
			csr_wr_node(node,
				    CVMX_BGXX_GMP_GMI_TXX_BURST(index,
								xi.interface),
				    0);
		else
			/* half duplex */
			csr_wr_node(node,
				    CVMX_BGXX_GMP_GMI_TXX_BURST(index,
								xi.interface),
				    8192);
		break;
	default:
		break;
	}

	/* Write the new misc control for PCS */
	csr_wr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface),
		    gmp_miscx_ctl.u64);

	/* Write the new GMX settings with the port still disabled */
	csr_wr_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
		    gmp_prtx_cfg.u64);

	/* Read GMX CFG again to make sure the config completed */
	csr_rd_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface));

	/* Enable back BGX. */
	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	if (debug)
		debug("%s: Enabling tx and rx packets on %d:%d\n", __func__,
		      xi.interface, index);
	cmr_config.s.data_pkt_tx_en = 1;
	cmr_config.s.data_pkt_rx_en = 1;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		    cmr_config.u64);

	return 0;
}

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set(). This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t __cvmx_helper_bgx_sgmii_link_get(int xipd_port)
{
	cvmx_helper_link_info_t result;
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);

	result.u64 = 0;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return result;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	gmp_control.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface));
	if (gmp_control.s.loopbck1) {
		int qlm = cvmx_qlm_lmac(xiface, index);
		int speed;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			speed = cvmx_qlm_get_gbaud_mhz_node(node, qlm);
		else
			speed = cvmx_qlm_get_gbaud_mhz(qlm);
		/* Force 1Gbps full duplex link for internal loopback */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = speed * 8 / 10;
		return result;
	}

	gmp_misc_ctl.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));
	if (gmp_misc_ctl.s.mac_phy ||
	    cvmx_helper_get_port_force_link_up(xiface, index)) {
		int qlm = cvmx_qlm_lmac(xiface, index);
		int speed;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			speed = cvmx_qlm_get_gbaud_mhz_node(node, qlm);
		else
			speed = cvmx_qlm_get_gbaud_mhz(qlm);
		/* PHY Mode */
		/* Note that this also works for 1000base-X mode */

		result.s.speed = speed * 8 / 10;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		return result;
	}

	/* MAC Mode */
	return __cvmx_helper_board_link_get(xipd_port);
}

/**
 * This sequence brings down the link for the XCV RGMII interface
 *
 * @param interface	Interface (BGX) number.  Port index is always 0
 */
static void __cvmx_helper_bgx_rgmii_link_set_down(int interface)
{
	union cvmx_xcv_reset xcv_reset;
	union cvmx_bgxx_cmrx_config cmr_config;
	union cvmx_bgxx_gmp_pcs_mrx_control mr_control;
	union cvmx_bgxx_cmrx_rx_fifo_len rx_fifo_len;
	union cvmx_bgxx_cmrx_tx_fifo_len tx_fifo_len;

	xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
	xcv_reset.s.rx_pkt_rst_n = 0;
	csr_wr(CVMX_XCV_RESET, xcv_reset.u64);
	csr_rd(CVMX_XCV_RESET);
	mdelay(10); /* Wait for 1 MTU */

	cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(0, interface));
	cmr_config.s.data_pkt_rx_en = 0;
	csr_wr(CVMX_BGXX_CMRX_CONFIG(0, interface), cmr_config.u64);

	/* Wait for RX and TX to be idle */
	do {
		rx_fifo_len.u64 =
			csr_rd(CVMX_BGXX_CMRX_RX_FIFO_LEN(0, interface));
		tx_fifo_len.u64 =
			csr_rd(CVMX_BGXX_CMRX_TX_FIFO_LEN(0, interface));
	} while (rx_fifo_len.s.fifo_len > 0 && tx_fifo_len.s.lmac_idle != 1);

	cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(0, interface));
	cmr_config.s.data_pkt_tx_en = 0;
	csr_wr(CVMX_BGXX_CMRX_CONFIG(0, interface), cmr_config.u64);

	xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
	xcv_reset.s.tx_pkt_rst_n = 0;
	csr_wr(CVMX_XCV_RESET, xcv_reset.u64);
	mr_control.u64 = csr_rd(CVMX_BGXX_GMP_PCS_MRX_CONTROL(0, interface));
	mr_control.s.pwr_dn = 1;
	csr_wr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(0, interface), mr_control.u64);
}

/**
 * Sets a BGS SGMII link down.
 *
 * @param node	Octeon node number
 * @param iface	BGX interface number
 * @param index	BGX port index
 */
static void __cvmx_helper_bgx_sgmii_link_set_down(int node, int iface,
						  int index)
{
	union cvmx_bgxx_gmp_pcs_miscx_ctl gmp_misc_ctl;
	union cvmx_bgxx_gmp_pcs_mrx_control gmp_control;
	union cvmx_bgxx_cmrx_config cmr_config;

	cmr_config.u64 = csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, iface));
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, iface), cmr_config.u64);

	gmp_misc_ctl.u64 =
		csr_rd_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, iface));

	/* Disable autonegotiation only when in MAC mode. */
	if (gmp_misc_ctl.s.mac_phy == 0) {
		gmp_control.u64 = csr_rd_node(
			node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, iface));
		gmp_control.s.an_en = 0;
		csr_wr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, iface),
			    gmp_control.u64);
	}

	/* Use GMXENO to force the link down.  It will get reenabled later... */
	gmp_misc_ctl.s.gmxeno = 1;
	csr_wr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, iface),
		    gmp_misc_ctl.u64);
	csr_rd_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, iface));
}

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead. This is used by interfaces
 * using the bgx mac.
 *
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_link_set(int xipd_port,
				     cvmx_helper_link_info_t link_info)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	const int iface = xi.interface;
	int rc = 0;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	cmr_config.u64 = csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, iface));
	if (link_info.s.link_up) {
		cmr_config.s.enable = 1;
		csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, iface),
			    cmr_config.u64);
		/* Apply workaround for errata BGX-22429 */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && index) {
			cvmx_bgxx_cmrx_config_t cmr0;

			cmr0.u64 = csr_rd_node(node,
					       CVMX_BGXX_CMRX_CONFIG(0, iface));
			cmr0.s.enable = 1;
			csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(0, iface),
				    cmr0.u64);
		}
		__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);
	} else if (cvmx_helper_bgx_is_rgmii(xi.interface, index)) {
		if (debug)
			debug("%s: Bringing down XCV RGMII interface %d\n",
			      __func__, xi.interface);
		__cvmx_helper_bgx_rgmii_link_set_down(xi.interface);
	} else { /* Link is down, not RGMII */
		__cvmx_helper_bgx_sgmii_link_set_down(node, iface, index);
		return 0;
	}
	rc = __cvmx_helper_bgx_sgmii_hardware_init_link_speed(xiface, index,
							      link_info);
	if (cvmx_helper_bgx_is_rgmii(xiface, index))
		rc = __cvmx_helper_bgx_rgmii_speed(link_info);

	return rc;
}

/**
 * @INTERNAL
 * Bringup XAUI interface. After this call packet I/O should be
 * fully functional.
 *
 * @param index port on interface to bring up
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_xaui_init(int index, int xiface)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;
	cvmx_bgxx_spux_control1_t spu_control1;
	cvmx_bgxx_spux_an_control_t spu_an_control;
	cvmx_bgxx_spux_an_adv_t spu_an_adv;
	cvmx_bgxx_spux_fec_control_t spu_fec_control;
	cvmx_bgxx_spu_dbg_control_t spu_dbg_control;
	cvmx_bgxx_smux_tx_append_t smu_tx_append;
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_helper_interface_mode_t mode;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int use_auto_neg = 0;
	int kr_mode = 0;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);

	if (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR ||
	    mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4) {
		kr_mode = 1;
		if (cvmx_helper_bgx_override_autoneg)
			use_auto_neg =
				cvmx_helper_bgx_override_autoneg(xiface, index);
		else
			use_auto_neg = cvmx_helper_get_port_autonegotiation(
				xiface, index);
	}

	/* NOTE: This code was moved first, out of order compared to the HRM
	 * because the RESET causes all SPU registers to loose their value
	 */
	/* 4. Next, bring up the SMU/SPU and the BGX reconciliation layer
	 * logic:
	 */
	/* 4a. Take SMU/SPU through a reset sequence. Write
	 * BGX(0..5)_SPU(0..3)_CONTROL1[RESET] = 1. Read
	 * BGX(0..5)_SPU(0..3)_CONTROL1[RESET] until it changes value to 0. Keep
	 * BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1 to disable
	 * reception.
	 */
	spu_control1.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
	spu_control1.s.reset = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
		    spu_control1.u64);

	/* 1. Wait for PCS to come out of reset */
	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
		    cvmx_bgxx_spux_control1_t, reset, ==, 0, 10000)) {
		debug("BGX%d:%d: SPU stuck in reset\n", node, interface);
		return -1;
	}

	/* 2. Write BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] to 0,
	 * BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 1 and
	 * BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1.
	 */
	spu_control1.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
	spu_control1.s.lo_pwr = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
		    spu_control1.u64);

	spu_misc_control.u64 = csr_rd_node(
		node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));
	spu_misc_control.s.rx_packet_dis = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface),
		    spu_misc_control.u64);

	/* 3. At this point, it may be appropriate to disable all BGX and
	 * SMU/SPU interrupts, as a number of them will occur during bring-up
	 * of the Link.
	 * - zero BGX(0..5)_SMU(0..3)_RX_INT
	 * - zero BGX(0..5)_SMU(0..3)_TX_INT
	 * - zero BGX(0..5)_SPU(0..3)_INT
	 */
	csr_wr_node(node, CVMX_BGXX_SMUX_RX_INT(index, xi.interface),
		    csr_rd_node(node,
				CVMX_BGXX_SMUX_RX_INT(index, xi.interface)));
	csr_wr_node(node, CVMX_BGXX_SMUX_TX_INT(index, xi.interface),
		    csr_rd_node(node,
				CVMX_BGXX_SMUX_TX_INT(index, xi.interface)));
	csr_wr_node(node, CVMX_BGXX_SPUX_INT(index, xi.interface),
		    csr_rd_node(node, CVMX_BGXX_SPUX_INT(index, xi.interface)));

	/* 4. Configure the BGX LMAC. */
	/* 4a. Configure the LMAC type (40GBASE-R/10GBASE-R/RXAUI/XAUI) and
	 * SerDes selection in the BGX(0..5)_CMR(0..3)_CONFIG register, but keep
	 * the ENABLE, DATA_PKT_TX_EN and DATA_PKT_RX_EN bits clear.
	 */
	/* Already done in bgx_setup_one_time */

	/* 4b. Write BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 1 and
	 * BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1.
	 */
	/* 4b. Initialize the selected SerDes lane(s) in the QLM. See Section
	 * 28.1.2.2 in the GSER chapter.
	 */
	/* Already done in QLM setup */

	/* 4c. For 10GBASE-KR or 40GBASE-KR, enable link training by writing
	 * BGX(0..5)_SPU(0..3)_BR_PMD_CONTROL[TRAIN_EN] = 1.
	 */

	if (kr_mode && !OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		csr_wr_node(node,
			    CVMX_BGXX_SPUX_BR_PMD_LP_CUP(index, interface), 0);
		csr_wr_node(node,
			    CVMX_BGXX_SPUX_BR_PMD_LD_CUP(index, interface), 0);
		csr_wr_node(node,
			    CVMX_BGXX_SPUX_BR_PMD_LD_REP(index, interface), 0);
		pmd_control.u64 = csr_rd_node(
			node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface));
		pmd_control.s.train_en = 1;
		csr_wr_node(node,
			    CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface),
			    pmd_control.u64);
	}

	/* 4d. Program all other relevant BGX configuration while
	 * BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] = 0. This includes all things
	 * described in this chapter.
	 */
	/* Always add FCS to PAUSE frames */
	smu_tx_append.u64 = csr_rd_node(
		node, CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface));
	smu_tx_append.s.fcs_c = 1;
	csr_wr_node(node, CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface),
		    smu_tx_append.u64);

	/* 4e. If Forward Error Correction is desired for 10GBASE-R or
	 * 40GBASE-R, enable it by writing
	 * BGX(0..5)_SPU(0..3)_FEC_CONTROL[FEC_EN] = 1.
	 */
	/* FEC is optional for 10GBASE-KR, 40GBASE-KR4, and XLAUI. We're going
	 * to disable it by default
	 */
	spu_fec_control.u64 = csr_rd_node(
		node, CVMX_BGXX_SPUX_FEC_CONTROL(index, xi.interface));
	if (cvmx_helper_bgx_override_fec)
		spu_fec_control.s.fec_en =
			cvmx_helper_bgx_override_fec(xiface, index);
	else
		spu_fec_control.s.fec_en =
			cvmx_helper_get_port_fec(xiface, index);
	csr_wr_node(node, CVMX_BGXX_SPUX_FEC_CONTROL(index, xi.interface),
		    spu_fec_control.u64);

	/* 4f. If Auto-Negotiation is desired, configure and enable
	 * Auto-Negotiation as described in Section 33.6.2.
	 */
	spu_an_control.u64 = csr_rd_node(
		node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface));
	/* Disable extended next pages */
	spu_an_control.s.xnp_en = 0;
	spu_an_control.s.an_en = use_auto_neg;
	csr_wr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface),
		    spu_an_control.u64);

	spu_fec_control.u64 = csr_rd_node(
		node, CVMX_BGXX_SPUX_FEC_CONTROL(index, xi.interface));
	spu_an_adv.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_AN_ADV(index, xi.interface));
	spu_an_adv.s.fec_req = spu_fec_control.s.fec_en;
	spu_an_adv.s.fec_able = 1;
	spu_an_adv.s.a100g_cr10 = 0;
	spu_an_adv.s.a40g_cr4 = 0;
	spu_an_adv.s.a40g_kr4 = (mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4);
	spu_an_adv.s.a10g_kr = (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR);
	spu_an_adv.s.a10g_kx4 = 0;
	spu_an_adv.s.a1g_kx = 0;
	spu_an_adv.s.xnp_able = 0;
	spu_an_adv.s.rf = 0;
	csr_wr_node(node, CVMX_BGXX_SPUX_AN_ADV(index, xi.interface),
		    spu_an_adv.u64);

	/* 3. Set BGX(0..5)_SPU_DBG_CONTROL[AN_ARB_LINK_CHK_EN] = 1. */
	spu_dbg_control.u64 =
		csr_rd_node(node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface));
	spu_dbg_control.s.an_nonce_match_dis = 1; /* Needed for loopback */
	spu_dbg_control.s.an_arb_link_chk_en |= kr_mode;
	csr_wr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface),
		    spu_dbg_control.u64);

	/* 4. Execute the link bring-up sequence in Section 33.6.3. */

	/* 5. If the auto-negotiation protocol is successful,
	 * BGX(0..5)_SPU(0..3)_AN_ADV[AN_COMPLETE] is set along with
	 * BGX(0..5)_SPU(0..3)_INT[AN_COMPLETE] when the link is up.
	 */

	/* 3h. Set BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] = 1 and
	 * BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 0 to enable the LMAC.
	 */
	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	cmr_config.s.enable = 1;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		    cmr_config.u64);
	/* Apply workaround for errata BGX-22429 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && index) {
		cvmx_bgxx_cmrx_config_t cmr0;

		cmr0.u64 = csr_rd_node(node,
				       CVMX_BGXX_CMRX_CONFIG(0, xi.interface));
		cmr0.s.enable = 1;
		csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(0, xi.interface),
			    cmr0.u64);
	}

	spu_control1.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
	spu_control1.s.lo_pwr = 0;
	csr_wr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
		    spu_control1.u64);

	/* 4g. Set the polarity and lane swapping of the QLM SerDes. Refer to
	 * Section 33.4.1, BGX(0..5)_SPU(0..3)_MISC_CONTROL[XOR_TXPLRT,XOR_RXPLRT]
	 * and BGX(0..5)_SPU(0..3)_MISC_CONTROL[TXPLRT,RXPLRT].
	 */

	/* 4c. Write BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 0. */
	spu_control1.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
	spu_control1.s.lo_pwr = 0;
	csr_wr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
		    spu_control1.u64);

	/* 4d. Select Deficit Idle Count mode and unidirectional enable/disable
	 * via BGX(0..5)_SMU(0..3)_TX_CTL[DIC_EN,UNI_EN].
	 */
	smu_tx_ctl.u64 =
		csr_rd_node(node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface));
	smu_tx_ctl.s.dic_en = 1;
	smu_tx_ctl.s.uni_en = 0;
	csr_wr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface),
		    smu_tx_ctl.u64);

	{
		/* Calculate the number of s-clk cycles per usec. */
		const u64 clock_mhz = 1200; /* todo: fixme */
		cvmx_bgxx_spu_dbg_control_t dbg_control;

		dbg_control.u64 = csr_rd_node(
			node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface));
		dbg_control.s.us_clk_period = clock_mhz - 1;
		csr_wr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface),
			    dbg_control.u64);
	}
	/* The PHY often takes at least 100ms to stabilize */
	__cvmx_helper_bgx_interface_enable_delay(mode);
	return 0;
}

static void __cvmx_bgx_start_training(int node, int unit, int index)
{
	cvmx_bgxx_spux_int_t spu_int;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;
	cvmx_bgxx_spux_an_control_t an_control;

	/* Clear the training interrupts (W1C) */
	spu_int.u64 = 0;
	spu_int.s.training_failure = 1;
	spu_int.s.training_done = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_INT(index, unit), spu_int.u64);

	/* These registers aren't cleared when training is restarted. Manually
	 * clear them as per Errata BGX-20968.
	 */
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_LP_CUP(index, unit), 0);
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_CUP(index, unit), 0);
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_REP(index, unit), 0);

	/* Disable autonegotiation */
	an_control.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, unit));
	an_control.s.an_en = 0;
	csr_wr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, unit),
		    an_control.u64);
	udelay(1);

	/* Restart training */
	pmd_control.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit));
	pmd_control.s.train_en = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit),
		    pmd_control.u64);

	udelay(1);
	pmd_control.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit));
	pmd_control.s.train_restart = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit),
		    pmd_control.u64);
}

static void __cvmx_bgx_restart_training(int node, int unit, int index)
{
	cvmx_bgxx_spux_int_t spu_int;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;

	/* Clear the training interrupts (W1C) */
	spu_int.u64 = 0;
	spu_int.s.training_failure = 1;
	spu_int.s.training_done = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_INT(index, unit), spu_int.u64);

	udelay(1700); /* Wait 1.7 msec */

	/* These registers aren't cleared when training is restarted. Manually
	 * clear them as per Errata BGX-20968.
	 */
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_LP_CUP(index, unit), 0);
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_CUP(index, unit), 0);
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_REP(index, unit), 0);

	/* Restart training */
	pmd_control.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit));
	pmd_control.s.train_restart = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit),
		    pmd_control.u64);
}

/*
 * @INTERNAL
 * Wrapper function to configure the BGX, does not enable.
 *
 * @param xipd_port IPD/PKO port to configure.
 * @param phy_pres  If set, enable disparity, only applies to RXAUI interface
 *
 * @return Zero on success, negative on failure.
 */
int __cvmx_helper_bgx_port_init(int xipd_port, int phy_pres)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_helper_interface_mode_t mode;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);

	__cvmx_bgx_common_init_pknd(xiface, index);

	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII ||
	    mode == CVMX_HELPER_INTERFACE_MODE_RGMII) {
		cvmx_bgxx_gmp_gmi_txx_thresh_t gmi_tx_thresh;
		cvmx_bgxx_gmp_gmi_txx_append_t gmp_txx_append;
		cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_t gmp_sgmii_ctl;

		/* Set TX Threshold */
		gmi_tx_thresh.u64 = 0;
		gmi_tx_thresh.s.cnt = 0x20;
		csr_wr_node(xi.node,
			    CVMX_BGXX_GMP_GMI_TXX_THRESH(index, xi.interface),
			    gmi_tx_thresh.u64);
		__cvmx_helper_bgx_sgmii_hardware_init_one_time(xiface, index);
		gmp_txx_append.u64 = csr_rd_node(
			xi.node,
			CVMX_BGXX_GMP_GMI_TXX_APPEND(index, xi.interface));
		gmp_sgmii_ctl.u64 = csr_rd_node(
			xi.node,
			CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(index, xi.interface));
		gmp_sgmii_ctl.s.align = gmp_txx_append.s.preamble ? 0 : 1;
		csr_wr_node(xi.node,
			    CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(index,
							    xi.interface),
			    gmp_sgmii_ctl.u64);
		if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII) {
			/* Disable XCV interface when initialized */
			union cvmx_xcv_reset xcv_reset;

			if (debug)
				debug("%s: Disabling RGMII XCV interface\n",
				      __func__);
			xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
			xcv_reset.s.enable = 0;
			xcv_reset.s.tx_pkt_rst_n = 0;
			xcv_reset.s.rx_pkt_rst_n = 0;
			csr_wr(CVMX_XCV_RESET, xcv_reset.u64);
		}
	} else {
		int res, cred;
		cvmx_bgxx_smux_tx_thresh_t smu_tx_thresh;

		res = __cvmx_helper_bgx_xaui_init(index, xiface);
		if (res == -1) {
#ifdef DEBUG_BGX
			debug("Failed to enable XAUI for %d:BGX(%d,%d)\n",
			      xi.node, xi.interface, index);
#endif
			return res;
		}
		/* See BVX_SMU_TX_THRESH register descriptin */
		cred = __cvmx_helper_bgx_fifo_size(xiface, index) >> 4;
		smu_tx_thresh.u64 = 0;
		smu_tx_thresh.s.cnt = cred - 10;
		csr_wr_node(xi.node,
			    CVMX_BGXX_SMUX_TX_THRESH(index, xi.interface),
			    smu_tx_thresh.u64);
		if (debug)
			debug("%s: BGX%d:%d TX-thresh=%d\n", __func__,
			      xi.interface, index,
			      (unsigned int)smu_tx_thresh.s.cnt);

		/* Set disparity for RXAUI interface as described in the
		 * Marvell RXAUI Interface specification.
		 */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI && phy_pres) {
			cvmx_bgxx_spux_misc_control_t misc_control;

			misc_control.u64 = csr_rd_node(
				xi.node, CVMX_BGXX_SPUX_MISC_CONTROL(
						 index, xi.interface));
			misc_control.s.intlv_rdisp = 1;
			csr_wr_node(xi.node,
				    CVMX_BGXX_SPUX_MISC_CONTROL(index,
								xi.interface),
				    misc_control.u64);
		}
	}
	return 0;
}

/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again. This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
int __cvmx_helper_bgx_sgmii_configure_loopback(int xipd_port,
					       int enable_internal,
					       int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_mrx_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	if (cvmx_helper_bgx_is_rgmii(xi.interface, index)) {
		cvmx_xcv_ctl_t xcv_ctl;
		cvmx_helper_link_info_t link_info;

		xcv_ctl.u64 = csr_rd(CVMX_XCV_CTL);
		xcv_ctl.s.lpbk_int = enable_internal;
		xcv_ctl.s.lpbk_ext = enable_external;
		csr_wr(CVMX_XCV_CTL, xcv_ctl.u64);

		/* Initialize link and speed */
		__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);
		link_info = __cvmx_helper_bgx_sgmii_link_get(xipd_port);
		__cvmx_helper_bgx_sgmii_hardware_init_link_speed(xiface, index,
								 link_info);
		__cvmx_helper_bgx_rgmii_speed(link_info);
	} else {
		gmp_mrx_control.u64 = csr_rd_node(
			node,
			CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface));
		gmp_mrx_control.s.loopbck1 = enable_internal;
		csr_wr_node(node,
			    CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface),
			    gmp_mrx_control.u64);

		gmp_misc_ctl.u64 = csr_rd_node(
			node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));
		gmp_misc_ctl.s.loopbck2 = enable_external;
		csr_wr_node(node,
			    CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface),
			    gmp_misc_ctl.u64);
		__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);
	}

	return 0;
}

static int __cvmx_helper_bgx_xaui_link_init(int index, int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_bgxx_spux_status2_t spu_status2;
	cvmx_bgxx_spux_br_status2_t br_status2;
	cvmx_bgxx_spux_int_t spu_int;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;
	cvmx_bgxx_spux_an_control_t spu_an_control;
	cvmx_bgxx_spux_an_status_t spu_an_status;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_helper_interface_mode_t mode;
	int use_training = 0;
	int rgmii_first = 0;
	int qlm = cvmx_qlm_lmac(xiface, index);
	int use_ber = 0;
	u64 err_blks;
	u64 ber_cnt;
	u64 error_debounce;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	rgmii_first = cvmx_helper_bgx_is_rgmii(xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR ||
	    mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4)
		use_training = 1;

	if ((mode == CVMX_HELPER_INTERFACE_MODE_XFI ||
	     mode == CVMX_HELPER_INTERFACE_MODE_XLAUI ||
	     mode == CVMX_HELPER_INTERFACE_MODE_10G_KR ||
	     mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4))
		use_ber = 1;

	/* Disable packet reception, CMR as well as SPU block */
	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		    cmr_config.u64);
	spu_misc_control.u64 = csr_rd_node(
		node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));
	spu_misc_control.s.rx_packet_dis = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface),
		    spu_misc_control.u64);

	spu_an_control.u64 = csr_rd_node(
		node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface));
	if (spu_an_control.s.an_en) {
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
			cvmx_bgxx_spux_int_t spu_int;

			spu_int.u64 = csr_rd_node(
				node, CVMX_BGXX_SPUX_INT(index, xi.interface));
			if (!spu_int.s.an_link_good) {
				static u64 restart_auto_neg[2][6][4] = {
					[0 ... 1][0 ... 5] = { [0 ... 3] = 0 }
				};
				u64 now = get_timer(0);
				u64 next_restart =
					restart_auto_neg[node][xi.interface]
							[index] +
					2000;

				if (now >= next_restart)
					return -1;

				restart_auto_neg[node][xi.interface][index] =
					now;

				/* Clear the auto negotiation (W1C) */
				spu_int.u64 = 0;
				spu_int.s.an_complete = 1;
				spu_int.s.an_link_good = 1;
				spu_int.s.an_page_rx = 1;
				csr_wr_node(node,
					    CVMX_BGXX_SPUX_INT(index,
							       xi.interface),
					    spu_int.u64);
				/* Restart auto negotiation */
				spu_an_control.u64 = csr_rd_node(
					node, CVMX_BGXX_SPUX_AN_CONTROL(
						      index, xi.interface));
				spu_an_control.s.an_restart = 1;
				csr_wr_node(node,
					    CVMX_BGXX_SPUX_AN_CONTROL(
						    index, xi.interface),
					    spu_an_control.u64);
				return -1;
			}
		} else {
			spu_an_status.u64 = csr_rd_node(
				node,
				CVMX_BGXX_SPUX_AN_STATUS(index, xi.interface));
			if (!spu_an_status.s.an_complete) {
				static u64 restart_auto_neg[2][6][4] = {
					[0 ... 1][0 ... 5] = { [0 ... 3] = 0 }
				};
				u64 now = get_timer(0);
				u64 next_restart =
					restart_auto_neg[node][xi.interface]
							[index] +
					2000;
				if (now >= next_restart) {
#ifdef DEBUG_BGX
					debug("WARNING: BGX%d:%d: Waiting for autoneg to complete\n",
					      xi.interface, index);
#endif
					return -1;
				}

				restart_auto_neg[node][xi.interface][index] =
					now;
				/* Restart auto negotiation */
				spu_an_control.u64 = csr_rd_node(
					node, CVMX_BGXX_SPUX_AN_CONTROL(
						      index, xi.interface));
				spu_an_control.s.an_restart = 1;
				csr_wr_node(node,
					    CVMX_BGXX_SPUX_AN_CONTROL(
						    index, xi.interface),
					    spu_an_control.u64);
				return -1;
			}
		}
	}

	if (use_training) {
		spu_int.u64 = csr_rd_node(
			node, CVMX_BGXX_SPUX_INT(index, xi.interface));
		pmd_control.u64 = csr_rd_node(
			node,
			CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, xi.interface));
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) &&
		    pmd_control.s.train_en == 0) {
			__cvmx_bgx_start_training(node, xi.interface, index);
			return -1;
		}
		cvmx_qlm_gser_errata_27882(node, qlm, index);
		spu_int.u64 = csr_rd_node(
			node, CVMX_BGXX_SPUX_INT(index, xi.interface));

		if (spu_int.s.training_failure &&
		    !OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
			__cvmx_bgx_restart_training(node, xi.interface, index);
			return -1;
		}
		if (!spu_int.s.training_done) {
			debug("Waiting for link training\n");
			return -1;
		}
	}

	/* (GSER-21957) GSER RX Equalization may make >= 5gbaud non-KR
	 * channel with DXAUI, RXAUI, XFI and XLAUI, we need to perform
	 * RX equalization when the link is receiving data the first time
	 */
	if (use_training == 0) {
		int lane = index;
		cvmx_bgxx_spux_control1_t control1;

		cmr_config.u64 = csr_rd_node(
			node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		control1.u64 = csr_rd_node(
			node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
		if (control1.s.loopbck) {
			/* Skip RX equalization when in loopback */
		} else if (mode == CVMX_HELPER_INTERFACE_MODE_XLAUI ||
			   mode == CVMX_HELPER_INTERFACE_MODE_XAUI) {
			lane = -1;
			if (__cvmx_qlm_rx_equalization(node, qlm, lane)) {
#ifdef DEBUG_BGX
				debug("%d:%d:%d: Waiting for RX Equalization on QLM%d\n",
				      node, xi.interface, index, qlm);
#endif
				return -1;
			}
			/* If BGX2 uses both dlms, then configure other dlm also. */
			if (OCTEON_IS_MODEL(OCTEON_CN73XX) &&
			    xi.interface == 2) {
				if (__cvmx_qlm_rx_equalization(node, 6, lane)) {
#ifdef DEBUG_BGX
					debug("%d:%d:%d: Waiting for RX Equalization on QLM%d\n",
					      node, xi.interface, index, qlm);
#endif
					return -1;
				}
			}
			/* RXAUI */
		} else if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI) {
			lane = index * 2;
			if (OCTEON_IS_MODEL(OCTEON_CN73XX) && index >= 2 &&
			    xi.interface == 2) {
				lane = 0;
			}
			if (rgmii_first)
				lane--;
			if (__cvmx_qlm_rx_equalization(node, qlm, lane) ||
			    __cvmx_qlm_rx_equalization(node, qlm, lane + 1)) {
#ifdef DEBUG_BGX
				debug("%d:%d:%d: Waiting for RX Equalization on QLM%d\n",
				      node, xi.interface, index, qlm);
#endif
				return -1;
			}
			/* XFI */
		} else if (cmr_config.s.lmac_type != 5) {
			if (rgmii_first)
				lane--;
			if (OCTEON_IS_MODEL(OCTEON_CN73XX) && index >= 2 &&
			    xi.interface == 2) {
				lane = index - 2;
			} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX) &&
				   index >= 2) {
				lane = index - 2;
			}
			if (__cvmx_qlm_rx_equalization(node, qlm, lane)) {
#ifdef DEBUG_BGX
				debug("%d:%d:%d: Waiting for RX Equalization on QLM%d\n",
				      node, xi.interface, index, qlm);
#endif
				return -1;
			}
		}
	}

	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
		    cvmx_bgxx_spux_control1_t, reset, ==, 0, 10000)) {
#ifdef DEBUG_BGX
		debug("ERROR: %d:BGX%d:%d: PCS in reset", node, xi.interface,
		      index);
#endif
		return -1;
	}

	if (use_ber) {
		if (CVMX_WAIT_FOR_FIELD64_NODE(
			    node,
			    CVMX_BGXX_SPUX_BR_STATUS1(index, xi.interface),
			    cvmx_bgxx_spux_br_status1_t, blk_lock, ==, 1,
			    10000)) {
#ifdef DEBUG_BGX
			debug("ERROR: %d:BGX%d:%d: BASE-R PCS block not locked\n",
			      node, xi.interface, index);

			if (mode == CVMX_HELPER_INTERFACE_MODE_XLAUI ||
			    mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4) {
				cvmx_bgxx_spux_br_algn_status_t bstatus;

				bstatus.u64 = csr_rd_node(
					node, CVMX_BGXX_SPUX_BR_ALGN_STATUS(
						      index, xi.interface));
				debug("ERROR: %d:BGX%d:%d: LANE BLOCK_LOCK:%x LANE MARKER_LOCK:%x\n",
				      node, xi.interface, index,
				      bstatus.s.block_lock,
				      bstatus.s.marker_lock);
			}
#endif
			return -1;
		}
	} else {
		/* (5) Check to make sure that the link appears up and stable.
		 */
		/* Wait for PCS to be aligned */
		if (CVMX_WAIT_FOR_FIELD64_NODE(
			    node, CVMX_BGXX_SPUX_BX_STATUS(index, xi.interface),
			    cvmx_bgxx_spux_bx_status_t, alignd, ==, 1, 10000)) {
#ifdef DEBUG_BGX
			debug("ERROR: %d:BGX%d:%d: PCS not aligned\n", node,
			      xi.interface, index);
#endif
			return -1;
		}
	}

	if (use_ber) {
		/* Set the BGXX_SPUX_BR_STATUS2.latched_lock bit (latching low).
		 * This will be checked prior to enabling packet tx and rx,
		 * ensuring block lock is sustained throughout the BGX link-up
		 * procedure
		 */
		br_status2.u64 = csr_rd_node(
			node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));
		br_status2.s.latched_lock = 1;
		csr_wr_node(node,
			    CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface),
			    br_status2.u64);
	}

	/* Clear rcvflt bit (latching high) and read it back */
	spu_status2.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface));
	spu_status2.s.rcvflt = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface),
		    spu_status2.u64);

	spu_status2.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface));
	if (spu_status2.s.rcvflt) {
#ifdef DEBUG_BGX
		debug("ERROR: %d:BGX%d:%d: Receive fault, need to retry\n",
		      node, xi.interface, index);
#endif
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && use_training)
			__cvmx_bgx_restart_training(node, xi.interface, index);
		/* debug("training restarting\n"); */
		return -1;
	}

	/* Wait for MAC RX to be ready */
	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_SMUX_RX_CTL(index, xi.interface),
		    cvmx_bgxx_smux_rx_ctl_t, status, ==, 0, 10000)) {
#ifdef DEBUG_BGX
		debug("ERROR: %d:BGX%d:%d: RX not ready\n", node, xi.interface,
		      index);
#endif
		return -1;
	}

	/* Wait for BGX RX to be idle */
	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_SMUX_CTRL(index, xi.interface),
		    cvmx_bgxx_smux_ctrl_t, rx_idle, ==, 1, 10000)) {
#ifdef DEBUG_BGX
		debug("ERROR: %d:BGX%d:%d: RX not idle\n", node, xi.interface,
		      index);
#endif
		return -1;
	}

	/* Wait for GMX TX to be idle */
	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_SMUX_CTRL(index, xi.interface),
		    cvmx_bgxx_smux_ctrl_t, tx_idle, ==, 1, 10000)) {
#ifdef DEBUG_BGX
		debug("ERROR: %d:BGX%d:%d: TX not idle\n", node, xi.interface,
		      index);
#endif
		return -1;
	}

	/* rcvflt should still be 0 */
	spu_status2.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface));
	if (spu_status2.s.rcvflt) {
#ifdef DEBUG_BGX
		debug("ERROR: %d:BGX%d:%d: Receive fault, need to retry\n",
		      node, xi.interface, index);
#endif
		return -1;
	}

	/* Receive link is latching low. Force it high and verify it */
	spu_status1.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface));
	spu_status1.s.rcv_lnk = 1;
	csr_wr_node(node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface),
		    spu_status1.u64);

	if (CVMX_WAIT_FOR_FIELD64_NODE(
		    node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface),
		    cvmx_bgxx_spux_status1_t, rcv_lnk, ==, 1, 10000)) {
#ifdef DEBUG_BGX
		debug("ERROR: %d:BGX%d:%d: Receive link down\n", node,
		      xi.interface, index);
#endif
		return -1;
	}

	if (use_ber) {
		/* Clearing BER_CNT and ERR_BLKs */
		br_status2.u64 = csr_rd_node(
			node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));

		/* If set, clear the LATCHED_BER by writing it to a one.  */
		if (br_status2.s.latched_ber)
			csr_wr_node(node,
				    CVMX_BGXX_SPUX_BR_STATUS2(index,
							      xi.interface),
				    br_status2.u64);

		error_debounce = get_timer(0);

		/* Clear error counts */
		err_blks = 0;
		ber_cnt = 0;

		/* Verify that the link is up and  error free for 100ms */
		while (get_timer(error_debounce) < 100) {
			spu_status1.u64 = csr_rd_node(
				node,
				CVMX_BGXX_SPUX_STATUS1(index, xi.interface));
			/* Checking that Receive link is still up (rcv_lnk = 1 (up)) */
			if (!spu_status1.s.rcv_lnk) {
#ifdef DEBUG_BGX
				debug("ERROR: %d:BGX%d:%d: Receive link down\n",
				      node, xi.interface, index);
#endif
				return -1;
			}

			/* Checking if latched_ber = 1 (BER >= 10e^4) */
			br_status2.u64 = csr_rd_node(
				node,
				CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));
			err_blks += br_status2.s.err_blks;
			ber_cnt += br_status2.s.ber_cnt;

			if (br_status2.s.latched_ber) {
#ifdef DEBUG_BGX
				debug("ERROR: %d:BGX%d:%d: BER test failed, BER >= 10e^4, need to retry\n",
				      node, xi.interface, index);
#endif
				return -1;
			}
			/* Checking that latched BLOCK_LOCK is still set (Block Lock never lost) */
			if (!br_status2.s.latched_lock) {
#ifdef DEBUG_BGX
				debug("ERROR: %d:BGX%d:%d: BASE-R PCS block lock lost, need to retry\n",
				      node, xi.interface, index);
#endif
				return -1;
			}

			/* Check error counters. Must be 0 (this error rate#
			 * is much higher than 1E-12)
			 */
			if (err_blks > 0) {
#ifdef DEBUG_BGX
				debug("ERROR: %d:BGX%d:%d: BASE-R errored-blocks (%llu) detected, need to retry\n",
				      node, xi.interface, index,
				      (unsigned long long)err_blks);
#endif
				return -1;
			}

			if (ber_cnt > 0) {
#ifdef DEBUG_BGX
				debug("ERROR: %d:BGX%d:%d: BASE-R bit-errors (%llu) detected, need to retry\n",
				      node, xi.interface, index,
				      (unsigned long long)ber_cnt);
#endif
				return -1;
			}

			udelay(1000);
		}

		/* Clear out the BGX error counters/bits. These errors are
		 * expected as part of the BGX link up procedure
		 */
		/* BIP_ERR counters clear as part of this read */
		csr_rd_node(node,
			    CVMX_BGXX_SPUX_BR_BIP_ERR_CNT(index, xi.interface));
		/* BER_CNT and ERR_BLKs clear as part of this read */
		br_status2.u64 = csr_rd_node(
			node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));
	}

	/* (7) Enable packet transmit and receive */
	spu_misc_control.u64 = csr_rd_node(
		node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));
	spu_misc_control.s.rx_packet_dis = 0;
	csr_wr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface),
		    spu_misc_control.u64);

	if (debug)
		debug("%s: Enabling tx and rx data packets\n", __func__);
	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	cmr_config.s.data_pkt_tx_en = 1;
	cmr_config.s.data_pkt_rx_en = 1;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		    cmr_config.u64);
	return 0;
}

int __cvmx_helper_bgx_xaui_enable(int xiface)
{
	int index;
	cvmx_helper_interface_mode_t mode;
	int num_ports = cvmx_helper_ports_on_interface(xiface);

	for (index = 0; index < num_ports; index++) {
		int res;
		int xipd_port = cvmx_helper_get_ipd_port(xiface, index);
		int phy_pres;
		struct cvmx_xiface xi =
			cvmx_helper_xiface_to_node_interface(xiface);
		static int count
			[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE]
			[CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] = {
				[0 ... CVMX_MAX_NODES -
				 1][0 ... CVMX_HELPER_MAX_IFACE -
				    1] = { [0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE -
					    1] = 0 }
			};

		mode = cvmx_helper_bgx_get_mode(xiface, index);

		/* Set disparity for RXAUI interface as described in the
		 * Marvell RXAUI Interface specification.
		 */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI &&
		    (cvmx_helper_get_port_phy_present(xiface, index)))
			phy_pres = 1;
		else
			phy_pres = 0;
		__cvmx_helper_bgx_port_init(xipd_port, phy_pres);

retry_link:
		res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
		/* RX Equalization or autonegotiation can take little longer
		 * retry the link maybe 5 times for now
		 */
		if (res == -1 && count[xi.node][xi.interface][index] < 5) {
			count[xi.node][xi.interface][index]++;
#ifdef DEBUG_BGX
			debug("%d:BGX(%d,%d): Failed to get link, retrying\n",
			      xi.node, xi.interface, index);
#endif
			goto retry_link;
		}

		if (res == -1) {
#ifdef DEBUG_BGX
			debug("%d:BGX(%d,%d): Failed to get link\n", xi.node,
			      xi.interface, index);
#endif
			continue;
		}
	}
	return 0;
}

cvmx_helper_link_info_t __cvmx_helper_bgx_xaui_link_get(int xipd_port)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_bgxx_smux_rx_ctl_t smu_rx_ctl;
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_helper_link_info_t result;
	cvmx_helper_interface_mode_t mode;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;
	cvmx_bgxx_spux_br_status2_t br_status2;

	result.u64 = 0;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_get(xipd_port);

	/* Reading current rx/tx link status */
	spu_status1.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface));
	smu_tx_ctl.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface));
	smu_rx_ctl.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_SMUX_RX_CTL(index, xi.interface));
	/* Reading tx/rx packet enables */
	cmr_config.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	spu_misc_control.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));

	if (smu_tx_ctl.s.ls == 0 && smu_rx_ctl.s.status == 0 &&
	    cmr_config.s.data_pkt_tx_en == 1 &&
	    cmr_config.s.data_pkt_rx_en == 1 &&
	    spu_misc_control.s.rx_packet_dis == 0 &&
	    spu_status1.s.rcv_lnk) {
		int lanes;
		int qlm = cvmx_qlm_lmac(xiface, index);
		u64 speed;

		result.s.link_up = 1;
		result.s.full_duplex = 1;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			speed = cvmx_qlm_get_gbaud_mhz_node(xi.node, qlm);
		else
			speed = cvmx_qlm_get_gbaud_mhz(qlm);

		cmr_config.u64 = csr_rd_node(
			xi.node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		switch (cmr_config.s.lmac_type) {
		default:
		case 1: // XAUI
			speed = (speed * 8 + 5) / 10;
			lanes = 4;
			break;
		case 2: // RXAUI
			speed = (speed * 8 + 5) / 10;
			lanes = 2;
			break;
		case 3: // XFI
			speed = (speed * 64 + 33) / 66;
			lanes = 1;
			break;
		case 4: // XLAUI
			/* Adjust the speed when XLAUI is configured at 6.250Gbps */
			if (speed == 6250)
				speed = 6445;
			speed = (speed * 64 + 33) / 66;
			lanes = 4;
			break;
		}

		if (debug)
			debug("%s: baud: %llu, lanes: %d\n", __func__,
			      (unsigned long long)speed, lanes);
		speed *= lanes;
		result.s.speed = speed;
	} else {
		int res;
		u64 err_blks = 0;
		u64 ber_cnt = 0;

		/* Check for err_blk and ber errors if 10G or 40G */
		if ((mode == CVMX_HELPER_INTERFACE_MODE_XFI ||
		     mode == CVMX_HELPER_INTERFACE_MODE_XLAUI ||
		     mode == CVMX_HELPER_INTERFACE_MODE_10G_KR ||
		     mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4)) {
			br_status2.u64 = csr_rd_node(
				xi.node,
				CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));
			err_blks = br_status2.s.err_blks;
			ber_cnt = br_status2.s.ber_cnt;
		}

		/* Checking if the link is up and error-free but we are receiving remote-faults */
		if (smu_tx_ctl.s.ls != 1 && smu_rx_ctl.s.status != 1 &&
		    cmr_config.s.data_pkt_tx_en == 1 &&
		    cmr_config.s.data_pkt_rx_en == 1 &&
		    spu_misc_control.s.rx_packet_dis == 0 &&
		    err_blks == 0 && ber_cnt == 0 &&
		    spu_status1.s.rcv_lnk) {
			result.s.init_success = 1;
#ifdef DEBUG_BGX
			debug("Receiving remote-fault ordered sets %d:BGX(%d,%d)\n",
			      xi.node, xi.interface, index);
#endif

		} else {
			res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
			if (res == -1) {
#ifdef DEBUG_BGX
				debug("Failed to get %d:BGX(%d,%d) link\n",
				      xi.node, xi.interface, index);
#endif
			} else {
#ifdef DEBUG_BGX
				debug("Link initialization successful %d:BGX(%d,%d)\n",
				      xi.node, xi.interface, index);
#endif
				result.s.init_success = 1;
			}
		}
	}

	return result;
}

int __cvmx_helper_bgx_xaui_link_set(int xipd_port,
				    cvmx_helper_link_info_t link_info)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_bgxx_smux_rx_ctl_t smu_rx_ctl;
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_helper_interface_mode_t mode;
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_set(xipd_port, link_info);

	/* Reading current rx/tx link status */
	smu_tx_ctl.u64 =
		csr_rd_node(node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface));
	smu_rx_ctl.u64 =
		csr_rd_node(node, CVMX_BGXX_SMUX_RX_CTL(index, xi.interface));
	spu_status1.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface));
	/* Reading tx/rx packet enables */
	cmr_config.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	spu_misc_control.u64 = csr_rd_node(
		xi.node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));

	/* If the link shouldn't be up, then just return */
	if (!link_info.s.link_up)
		return 0;

	/* Do nothing if both RX and TX are happy and packet
	 * transmission/reception is enabled
	 */
	if (smu_tx_ctl.s.ls == 0 && smu_rx_ctl.s.status == 0 &&
	    cmr_config.s.data_pkt_tx_en == 1 &&
	    cmr_config.s.data_pkt_rx_en == 1 &&
	    spu_misc_control.s.rx_packet_dis == 0 && spu_status1.s.rcv_lnk)
		return 0;

	/* Bring the link up */
	return __cvmx_helper_bgx_xaui_link_init(index, xiface);
}

int __cvmx_helper_bgx_xaui_configure_loopback(int xipd_port,
					      int enable_internal,
					      int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_spux_control1_t spu_control1;
	cvmx_bgxx_smux_ext_loopback_t smu_ext_loopback;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	/* INT_BEAT_GEN must be set for loopback if the QLMs are not clocked.
	 * Set it whenever we use internal loopback
	 */
	if (enable_internal) {
		cvmx_bgxx_cmrx_config_t cmr_config;

		cmr_config.u64 = csr_rd_node(
			node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		cmr_config.s.int_beat_gen = 1;
		csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
			    cmr_config.u64);
	}
	/* Set the internal loop */
	spu_control1.u64 =
		csr_rd_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
	spu_control1.s.loopbck = enable_internal;
	csr_wr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
		    spu_control1.u64);
	/* Set the external loop */
	smu_ext_loopback.u64 = csr_rd_node(
		node, CVMX_BGXX_SMUX_EXT_LOOPBACK(index, xi.interface));
	smu_ext_loopback.s.en = enable_external;
	csr_wr_node(node, CVMX_BGXX_SMUX_EXT_LOOPBACK(index, xi.interface),
		    smu_ext_loopback.u64);

	return __cvmx_helper_bgx_xaui_link_init(index, xiface);
}

int __cvmx_helper_bgx_mixed_enable(int xiface)
{
	int index;
	int num_ports = cvmx_helper_ports_on_interface(xiface);
	cvmx_helper_interface_mode_t mode;

	for (index = 0; index < num_ports; index++) {
		int xipd_port, phy_pres = 0;

		if (!cvmx_helper_is_port_valid(xiface, index))
			continue;

		mode = cvmx_helper_bgx_get_mode(xiface, index);

		xipd_port = cvmx_helper_get_ipd_port(xiface, index);

		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI &&
		    (cvmx_helper_get_port_phy_present(xiface, index)))
			phy_pres = 1;

		if (__cvmx_helper_bgx_port_init(xipd_port, phy_pres))
			continue;

		/* For RGMII interface, initialize the link after PKO is setup */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
			continue;
		/* Call SGMII init code for lmac_type = 0|5 */
		else if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII) {
			int do_link_set = 1;

			if (do_link_set)
				__cvmx_helper_bgx_sgmii_link_set(
					xipd_port,
					__cvmx_helper_bgx_sgmii_link_get(
						xipd_port));
			/* All other lmac type call XAUI init code */
		} else {
			int res;
			struct cvmx_xiface xi =
				cvmx_helper_xiface_to_node_interface(xiface);
			static int count
				[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE]
				[CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] = {
					[0 ... CVMX_MAX_NODES -
					 1][0 ... CVMX_HELPER_MAX_IFACE -
					    1] = { [0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE -
						    1] = 0 }
				};

retry_link:
			res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
			/* RX Equalization or autonegotiation can take little
			 * longer retry the link maybe 5 times for now
			 */
			if (res == -1 &&
			    count[xi.node][xi.interface][index] < 5) {
				count[xi.node][xi.interface][index]++;
				goto retry_link;
			}

			if (res == -1) {
#ifdef DEBUG_BGX
				debug("Failed to get %d:BGX(%d,%d) link\n",
				      xi.node, xi.interface, index);
#endif
				continue;
			}
		}
	}
	return 0;
}

cvmx_helper_link_info_t __cvmx_helper_bgx_mixed_link_get(int xipd_port)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII ||
	    mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_get(xipd_port);
	else
		return __cvmx_helper_bgx_xaui_link_get(xipd_port);
}

int __cvmx_helper_bgx_mixed_link_set(int xipd_port,
				     cvmx_helper_link_info_t link_info)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII ||
	    mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_set(xipd_port, link_info);
	else
		return __cvmx_helper_bgx_xaui_link_set(xipd_port, link_info);
}

int __cvmx_helper_bgx_mixed_configure_loopback(int xipd_port,
					       int enable_internal,
					       int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII ||
	    mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_configure_loopback(
			xipd_port, enable_internal, enable_external);
	else
		return __cvmx_helper_bgx_xaui_configure_loopback(
			xipd_port, enable_internal, enable_external);
}

/**
 * @INTERNAL
 * Configure Priority-Based Flow Control (a.k.a. PFC/CBFC)
 * on a specific BGX interface/port.
 */
void __cvmx_helper_bgx_xaui_config_pfc(unsigned int node,
				       unsigned int interface,
				       unsigned int index, bool pfc_enable)
{
	int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_bgxx_smux_cbfc_ctl_t cbfc_ctl;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	cbfc_ctl.u64 =
		csr_rd_node(node, CVMX_BGXX_SMUX_CBFC_CTL(index, xi.interface));

	/* Enable all PFC controls if requiested */
	cbfc_ctl.s.rx_en = pfc_enable;
	cbfc_ctl.s.tx_en = pfc_enable;
	if (debug)
		debug("%s: CVMX_BGXX_SMUX_CBFC_CTL(%d,%d)=%#llx\n", __func__,
		      index, xi.interface, (unsigned long long)cbfc_ctl.u64);
	csr_wr_node(node, CVMX_BGXX_SMUX_CBFC_CTL(index, xi.interface),
		    cbfc_ctl.u64);
}

/**
 * Function to control the generation of FCS, padding by the BGX
 *
 */
void cvmx_helper_bgx_tx_options(unsigned int node, unsigned int interface,
				unsigned int index, bool fcs_enable,
				bool pad_enable)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_gmp_gmi_txx_append_t gmp_txx_append;
	cvmx_bgxx_gmp_gmi_txx_min_pkt_t gmp_min_pkt;
	cvmx_bgxx_smux_tx_min_pkt_t smu_min_pkt;
	cvmx_bgxx_smux_tx_append_t smu_tx_append;
	int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!cvmx_helper_is_port_valid(xiface, index))
		return;

	if (debug)
		debug("%s: interface %u:%d/%d, fcs: %s, pad: %s\n", __func__,
		      xi.node, xi.interface, index,
		      fcs_enable ? "true" : "false",
		      pad_enable ? "true" : "false");

	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));

	(void)cmr_config; /* In case we need LMAC_TYPE later */

	/* Setting options for both BGX subsystems, regardless of LMAC type */

	/* Set GMP (SGMII) Tx options */
	gmp_min_pkt.u64 = 0;
	/* per HRM Sec 34.3.4.4 */
	gmp_min_pkt.s.min_size = 59;
	csr_wr_node(node, CVMX_BGXX_GMP_GMI_TXX_MIN_PKT(index, xi.interface),
		    gmp_min_pkt.u64);
	gmp_txx_append.u64 = csr_rd_node(
		node, CVMX_BGXX_GMP_GMI_TXX_APPEND(index, xi.interface));
	gmp_txx_append.s.fcs = fcs_enable;
	gmp_txx_append.s.pad = pad_enable;
	csr_wr_node(node, CVMX_BGXX_GMP_GMI_TXX_APPEND(index, xi.interface),
		    gmp_txx_append.u64);

	/* Set SMUX (XAUI/XFI) Tx options */
	/* HRM Sec 33.3.4.3 should read 64 */
	smu_min_pkt.u64 = 0;
	smu_min_pkt.s.min_size = 0x40;
	csr_wr_node(node, CVMX_BGXX_SMUX_TX_MIN_PKT(index, xi.interface),
		    smu_min_pkt.u64);
	smu_tx_append.u64 = csr_rd_node(
		node, CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface));
	smu_tx_append.s.fcs_d = fcs_enable; /* Set data-packet FCS */
	smu_tx_append.s.pad = pad_enable;
	csr_wr_node(node, CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface),
		    smu_tx_append.u64);
}

/**
 * Set mac for the ipd_port
 *
 * @param xipd_port ipd_port to set the mac
 * @param bcst      If set, accept all broadcast packets
 * @param mcst      Multicast mode
 *		    0 = Force reject all multicast packets
 *		    1 = Force accept all multicast packets
 *		    2 = use the address filter CAM.
 * @param mac       mac address for the ipd_port, or 0 to disable MAC filtering
 */
void cvmx_helper_bgx_set_mac(int xipd_port, int bcst, int mcst, u64 mac)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	int index;
	cvmx_bgxx_cmr_rx_adrx_cam_t adr_cam;
	cvmx_bgxx_cmrx_rx_adr_ctl_t adr_ctl;
	cvmx_bgxx_cmrx_config_t cmr_config;
	int saved_state_tx, saved_state_rx;

	index = cvmx_helper_get_interface_index_num(xipd_port);

	if (!cvmx_helper_is_port_valid(xiface, index))
		return;

	if (debug)
		debug("%s: interface %u:%d/%d\n", __func__, xi.node,
		      xi.interface, index);

	cmr_config.u64 =
		csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	saved_state_tx = cmr_config.s.data_pkt_tx_en;
	saved_state_rx = cmr_config.s.data_pkt_rx_en;
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		    cmr_config.u64);

	/* Set the mac */
	adr_cam.u64 = 0;
	adr_cam.s.id = index;

	if (mac != 0ull)
		adr_cam.s.en = 1;
	adr_cam.s.adr = mac;

	csr_wr_node(node, CVMX_BGXX_CMR_RX_ADRX_CAM(index * 8, xi.interface),
		    adr_cam.u64);

	adr_ctl.u64 = csr_rd_node(
		node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, xi.interface));
	if (mac != 0ull)
		adr_ctl.s.cam_accept =
			1; /* Accept the packet on DMAC CAM address */
	else
		adr_ctl.s.cam_accept = 0; /* No filtering, promiscuous */

	adr_ctl.s.mcst_mode = mcst;   /* Use the address filter CAM */
	adr_ctl.s.bcst_accept = bcst; /* Accept all broadcast packets */
	csr_wr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, xi.interface),
		    adr_ctl.u64);
	/* Set SMAC for PAUSE frames */
	csr_wr_node(node, CVMX_BGXX_GMP_GMI_SMACX(index, xi.interface), mac);

	/* Restore back the interface state */
	cmr_config.s.data_pkt_tx_en = saved_state_tx;
	cmr_config.s.data_pkt_rx_en = saved_state_rx;
	csr_wr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		    cmr_config.u64);

	/* Wait 100ms after bringing up the link to give the PHY some time */
	if (cmr_config.s.enable) {
		cvmx_helper_interface_mode_t mode;

		mode = cvmx_helper_bgx_get_mode(xiface, index);
		__cvmx_helper_bgx_interface_enable_delay(mode);
	}
}

/**
 * Disables the sending of flow control (pause) frames on the specified
 * BGX port(s).
 *
 * @param xiface Which xiface
 * @param port_mask Mask (4bits) of which ports on the interface to disable
 *                  backpressure on.
 *                  1 => disable backpressure
 *                  0 => enable backpressure
 *
 * @return 0 on success
 *         -1 on error
 *
 * FIXME: Should change the API to handle a single port in every
 * invocation, for consistency with other API calls.
 */
int cvmx_bgx_set_backpressure_override(int xiface, unsigned int port_mask)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_bgxx_cmr_rx_ovr_bp_t rx_ovr_bp;
	int node = xi.node;

	if (xi.interface >= CVMX_HELPER_MAX_GMX)
		return 0;

	if (debug)
		debug("%s: interface %u:%d port_mask=%#x\n", __func__, xi.node,
		      xi.interface, port_mask);

	/* Check for valid arguments */
	rx_ovr_bp.u64 = 0;
	rx_ovr_bp.s.en = port_mask; /* Per port Enable back pressure override */
	rx_ovr_bp.s.ign_fifo_bp =
		port_mask; /* Ignore the RX FIFO full when computing BP */

	csr_wr_node(node, CVMX_BGXX_CMR_RX_OVR_BP(xi.interface), rx_ovr_bp.u64);
	return 0;
}

int cvmx_bgx_set_flowctl_mode(int xipd_port, cvmx_qos_proto_t qos,
			      cvmx_qos_pkt_mode_t fc_mode)
{
	int node, xiface, iface, index, mode;
	struct cvmx_xiface xi;
	const struct {
		int bck;
		int drp;
	} fcmode[4] = { [CVMX_QOS_PKT_MODE_HWONLY] = { 1, 1 },
			[CVMX_QOS_PKT_MODE_SWONLY] = { 0, 0 },
			[CVMX_QOS_PKT_MODE_HWSW] = { 1, 0 },
			[CVMX_QOS_PKT_MODE_DROP] = { 0, 1 } };

	xiface = cvmx_helper_get_interface_num(xipd_port);
	xi = cvmx_helper_xiface_to_node_interface(xiface);
	node = xi.node;
	iface = xi.interface;

	if (xi.interface >= CVMX_HELPER_MAX_GMX)
		return 0;

	index = cvmx_helper_get_interface_index_num(xipd_port);
	mode = cvmx_helper_bgx_get_mode(xiface, index);
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_XAUI: {
		cvmx_bgxx_smux_tx_ctl_t txctl;
		cvmx_bgxx_smux_cbfc_ctl_t cbfc;
		cvmx_bgxx_smux_rx_frm_ctl_t frmctl;
		cvmx_bgxx_smux_hg2_control_t hg2ctl;

		txctl.u64 =
			csr_rd_node(node, CVMX_BGXX_SMUX_TX_CTL(index, iface));
		cbfc.u64 = csr_rd_node(node,
				       CVMX_BGXX_SMUX_CBFC_CTL(index, iface));
		frmctl.u64 = csr_rd_node(
			node, CVMX_BGXX_SMUX_RX_FRM_CTL(index, iface));
		hg2ctl.u64 = csr_rd_node(
			node, CVMX_BGXX_SMUX_HG2_CONTROL(index, iface));
		switch (qos) {
		case CVMX_QOS_PROTO_PAUSE:
			cbfc.u64 = 0;
			hg2ctl.u64 = 0;
			frmctl.s.ctl_bck = fcmode[fc_mode].bck;
			frmctl.s.ctl_drp = fcmode[fc_mode].drp;
			frmctl.s.ctl_mcst = 1;
			txctl.s.l2p_bp_conv = 1;
			break;
		case CVMX_QOS_PROTO_PFC:
			hg2ctl.u64 = 0;
			hg2ctl.s.logl_en = 0xff;
			frmctl.s.ctl_bck = fcmode[fc_mode].bck;
			frmctl.s.ctl_drp = fcmode[fc_mode].drp;
			frmctl.s.ctl_mcst = 1;
			cbfc.s.bck_en = fcmode[fc_mode].bck;
			cbfc.s.drp_en = fcmode[fc_mode].drp;
			cbfc.s.phys_en = 0;
			cbfc.s.logl_en = 0xff;
			cbfc.s.tx_en = 1;
			cbfc.s.rx_en = 1;
			break;
		case CVMX_QOS_PROTO_NONE:
			cbfc.u64 = 0;
			hg2ctl.u64 = 0;
			frmctl.s.ctl_bck = fcmode[CVMX_QOS_PKT_MODE_DROP].bck;
			frmctl.s.ctl_drp = fcmode[CVMX_QOS_PKT_MODE_DROP].drp;
			txctl.s.l2p_bp_conv = 0;
			break;
		default:
			break;
		}
		csr_wr_node(node, CVMX_BGXX_SMUX_CBFC_CTL(index, iface),
			    cbfc.u64);
		csr_wr_node(node, CVMX_BGXX_SMUX_RX_FRM_CTL(index, iface),
			    frmctl.u64);
		csr_wr_node(node, CVMX_BGXX_SMUX_HG2_CONTROL(index, iface),
			    hg2ctl.u64);
		csr_wr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, iface),
			    txctl.u64);
		break;
	}
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_RGMII: {
		cvmx_bgxx_gmp_gmi_rxx_frm_ctl_t gmi_frmctl;

		gmi_frmctl.u64 = csr_rd_node(
			node, CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(index, iface));
		switch (qos) {
		case CVMX_QOS_PROTO_PAUSE:
			gmi_frmctl.s.ctl_bck = fcmode[fc_mode].bck;
			gmi_frmctl.s.ctl_drp = fcmode[fc_mode].drp;
			gmi_frmctl.s.ctl_mcst = 1;
			break;
		case CVMX_QOS_PROTO_NONE:
			gmi_frmctl.s.ctl_bck =
				fcmode[CVMX_QOS_PKT_MODE_DROP].bck;
			gmi_frmctl.s.ctl_drp =
				fcmode[CVMX_QOS_PKT_MODE_DROP].drp;
			break;
		default:
			break;
		}
		csr_wr_node(node, CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(index, iface),
			    gmi_frmctl.u64);
	}
	} /*switch*/

	return 0;
}
