// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for RGMII (MGMT) initialization, configuration,
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

#include <mach/cvmx-pki.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-agl.h>
#include <mach/cvmx-agl-defs.h>

/*
 * @param port to enable
 *
 * @return Zero on success, negative on failure
 */
int cvmx_agl_enable(int port)
{
	cvmx_agl_gmx_rxx_frm_ctl_t rxx_frm_ctl;

	rxx_frm_ctl.u64 = 0;
	rxx_frm_ctl.s.pre_align = 1;
	/* When set, disables the length check for non-min sized pkts with
	 * padding in the client data
	 */
	rxx_frm_ctl.s.pad_len = 1;
	/* When set, disables the length check for VLAN pkts */
	rxx_frm_ctl.s.vlan_len = 1;
	/* When set, PREAMBLE checking is  less strict */
	rxx_frm_ctl.s.pre_free = 1;
	/* Control Pause Frames can match station SMAC */
	rxx_frm_ctl.s.ctl_smac = 0;
	/* Control Pause Frames can match globally assign Multicast address */
	rxx_frm_ctl.s.ctl_mcst = 1;
	rxx_frm_ctl.s.ctl_bck = 1;  /* Forward pause information to TX block */
	rxx_frm_ctl.s.ctl_drp = 1;  /* Drop Control Pause Frames */
	rxx_frm_ctl.s.pre_strp = 1; /* Strip off the preamble */
	/* This port is configured to send PREAMBLE+SFD to begin every frame.
	 * GMX checks that the PREAMBLE is sent correctly
	 */
	rxx_frm_ctl.s.pre_chk = 1;
	csr_wr(CVMX_AGL_GMX_RXX_FRM_CTL(port), rxx_frm_ctl.u64);

	return 0;
}

cvmx_helper_link_info_t cvmx_agl_link_get(int port)
{
	cvmx_helper_link_info_t result;
	int interface, port_index;

	/* Fake IPD port is used on some older models. */
	if (port < 0)
		return __cvmx_helper_board_link_get(port);

	/* Simulator does not have PHY, use some defaults. */
	interface = cvmx_helper_get_interface_num(port);
	port_index = cvmx_helper_get_interface_index_num(port);
	if (cvmx_helper_get_port_force_link_up(interface, port_index)) {
		result.u64 = 0;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		result.s.speed = 1000;
		return result;
	}

	return __cvmx_helper_board_link_get(port);
}

/*
 * Set MII/RGMII link based on mode.
 *
 * @param port   interface port to set the link.
 * @param link_info  Link status
 *
 * @return       0 on success and 1 on failure
 */
int cvmx_agl_link_set(int port, cvmx_helper_link_info_t link_info)
{
	cvmx_agl_gmx_prtx_cfg_t agl_gmx_prtx;

	/* Disable GMX before we make any changes. */
	agl_gmx_prtx.u64 = csr_rd(CVMX_AGL_GMX_PRTX_CFG(port));
	agl_gmx_prtx.s.en = 0;
	agl_gmx_prtx.s.tx_en = 0;
	agl_gmx_prtx.s.rx_en = 0;
	csr_wr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
		u64 one_second = 0x1000000; /* todo: this needs checking */

		/* Wait for GMX to be idle */
		if (CVMX_WAIT_FOR_FIELD64(CVMX_AGL_GMX_PRTX_CFG(port),
					  cvmx_agl_gmx_prtx_cfg_t, rx_idle, ==,
					  1, one_second) ||
		    CVMX_WAIT_FOR_FIELD64(CVMX_AGL_GMX_PRTX_CFG(port),
					  cvmx_agl_gmx_prtx_cfg_t, tx_idle, ==,
					  1, one_second)) {
			debug("AGL%d: Timeout waiting for GMX to be idle\n",
			      port);
			return -1;
		}
	}

	agl_gmx_prtx.u64 = csr_rd(CVMX_AGL_GMX_PRTX_CFG(port));

	/* Set duplex mode */
	if (!link_info.s.link_up)
		agl_gmx_prtx.s.duplex = 1; /* Force full duplex on down links */
	else
		agl_gmx_prtx.s.duplex = link_info.s.full_duplex;

	switch (link_info.s.speed) {
	case 10:
		agl_gmx_prtx.s.speed = 0;
		agl_gmx_prtx.s.slottime = 0;
		if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
			agl_gmx_prtx.s.speed_msb = 1;
			agl_gmx_prtx.s.burst = 1;
		}
		break;

	case 100:
		agl_gmx_prtx.s.speed = 0;
		agl_gmx_prtx.s.slottime = 0;
		if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
			agl_gmx_prtx.s.speed_msb = 0;
			agl_gmx_prtx.s.burst = 1;
		}
		break;

	case 1000:
		/* 1000 MBits is only supported on 6XXX chips */
		if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
			agl_gmx_prtx.s.speed_msb = 0;
			agl_gmx_prtx.s.speed = 1;
			agl_gmx_prtx.s.slottime =
				1; /* Only matters for half-duplex */
			agl_gmx_prtx.s.burst = agl_gmx_prtx.s.duplex;
		}
		break;

		/* No link */
	case 0:
	default:
		break;
	}

	/* Write the new GMX setting with the port still disabled. */
	csr_wr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	/* Read GMX CFG again to make sure the config is completed. */
	agl_gmx_prtx.u64 = csr_rd(CVMX_AGL_GMX_PRTX_CFG(port));

	if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
		cvmx_agl_gmx_txx_clk_t agl_clk;
		cvmx_agl_prtx_ctl_t prt_ctl;

		prt_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(port));
		agl_clk.u64 = csr_rd(CVMX_AGL_GMX_TXX_CLK(port));
		/* MII (both speeds) and RGMII 1000 setting */
		agl_clk.s.clk_cnt = 1;
		/* Check other speeds for RGMII mode */
		if (prt_ctl.s.mode == 0 || OCTEON_IS_OCTEON3()) {
			if (link_info.s.speed == 10)
				agl_clk.s.clk_cnt = 50;
			else if (link_info.s.speed == 100)
				agl_clk.s.clk_cnt = 5;
		}
		csr_wr(CVMX_AGL_GMX_TXX_CLK(port), agl_clk.u64);
	}

	/* Enable transmit and receive ports */
	agl_gmx_prtx.s.tx_en = 1;
	agl_gmx_prtx.s.rx_en = 1;
	csr_wr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	/* Enable the link. */
	agl_gmx_prtx.s.en = 1;
	csr_wr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	if (OCTEON_IS_OCTEON3()) {
		union cvmx_agl_prtx_ctl agl_prtx_ctl;
		/* Enable the interface, set clkrst */
		agl_prtx_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(port));
		agl_prtx_ctl.s.clkrst = 1;
		csr_wr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
		csr_rd(CVMX_AGL_PRTX_CTL(port));
		agl_prtx_ctl.s.enable = 1;
		csr_wr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
		/* Read the value back to force the previous write */
		csr_rd(CVMX_AGL_PRTX_CTL(port));
	}

	return 0;
}
