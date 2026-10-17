// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for AGL (RGMII) initialization, configuration,
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
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-agl.h>
#include <mach/cvmx-pki.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-pko-defs.h>

int __cvmx_helper_agl_enumerate(int xiface)
{
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		union cvmx_agl_prtx_ctl agl_prtx_ctl;

		agl_prtx_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(0));
		if (agl_prtx_ctl.s.mode == 0) /* RGMII */
			return 1;
	}
	return 0;
}

/**
 * @INTERNAL
 * Convert interface to port to assess CSRs.
 *
 * @param xiface  Interface to probe
 * @return  The port corresponding to the interface
 */
int cvmx_helper_agl_get_port(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return xi.interface - 4;
	return -1;
}

/**
 * @INTERNAL
 * Probe a RGMII interface and determine the number of ports
 * connected to it. The RGMII interface should still be down
 * after this call.
 *
 * @param interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_agl_probe(int interface)
{
	int port = cvmx_helper_agl_get_port(interface);
	union cvmx_agl_gmx_bist gmx_bist;
	union cvmx_agl_gmx_prtx_cfg gmx_prtx_cfg;
	union cvmx_agl_prtx_ctl agl_prtx_ctl;
	int result;

	result = __cvmx_helper_agl_enumerate(interface);
	if (result == 0)
		return 0;

	/* Check BIST status */
	gmx_bist.u64 = csr_rd(CVMX_AGL_GMX_BIST);
	if (gmx_bist.u64)
		printf("Management port AGL failed BIST (0x%016llx) on AGL%d\n",
		       CAST64(gmx_bist.u64), port);

	/* Disable the external input/output */
	gmx_prtx_cfg.u64 = csr_rd(CVMX_AGL_GMX_PRTX_CFG(port));
	gmx_prtx_cfg.s.en = 0;
	csr_wr(CVMX_AGL_GMX_PRTX_CFG(port), gmx_prtx_cfg.u64);

	/* Set the rgx_ref_clk MUX with AGL_PRTx_CTL[REFCLK_SEL]. Default value
	 * is 0 (RGMII REFCLK). Recommended to use RGMII RXC(1) or sclk/4 (2)
	 * to save cost.
	 */

	agl_prtx_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(port));
	agl_prtx_ctl.s.clkrst = 0;
	agl_prtx_ctl.s.dllrst = 0;
	agl_prtx_ctl.s.clktx_byp = 0;

	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		bool tx_enable_bypass;
		int tx_delay;

		agl_prtx_ctl.s.refclk_sel =
			cvmx_helper_get_agl_refclk_sel(interface, port);
		agl_prtx_ctl.s.clkrx_set =
			cvmx_helper_get_agl_rx_clock_skew(interface, port);
		agl_prtx_ctl.s.clkrx_byp =
			cvmx_helper_get_agl_rx_clock_delay_bypass(interface,
								  port);
		cvmx_helper_cfg_get_rgmii_tx_clk_delay(
			interface, port, &tx_enable_bypass, &tx_delay);
		agl_prtx_ctl.s.clktx_byp = tx_enable_bypass;
		agl_prtx_ctl.s.clktx_set = tx_delay;
	}
	csr_wr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
	/* Force write out before wait */
	csr_rd(CVMX_AGL_PRTX_CTL(port));
	udelay(500);

	/* Enable the componsation controller */
	agl_prtx_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(port));
	agl_prtx_ctl.s.drv_byp = 0;
	csr_wr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
	/* Force write out before wait */
	csr_rd(CVMX_AGL_PRTX_CTL(port));

	if (!OCTEON_IS_OCTEON3()) {
		/* Enable the interface */
		agl_prtx_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(port));
		agl_prtx_ctl.s.enable = 1;
		csr_wr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
		/* Read the value back to force the previous write */
		agl_prtx_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(port));
	}

	/* Enable the compensation controller */
	agl_prtx_ctl.u64 = csr_rd(CVMX_AGL_PRTX_CTL(port));
	agl_prtx_ctl.s.comp = 1;
	csr_wr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
	/* Force write out before wait */
	csr_rd(CVMX_AGL_PRTX_CTL(port));

	/* for componsation state to lock. */
	udelay(500);

	return result;
}

/**
 * @INTERNAL
 * Bringup and enable a RGMII interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_agl_enable(int interface)
{
	int port = cvmx_helper_agl_get_port(interface);
	int ipd_port = cvmx_helper_get_ipd_port(interface, port);
	union cvmx_pko_mem_port_ptrs pko_mem_port_ptrs;
	union cvmx_pko_reg_read_idx read_idx;
	int do_link_set = 1;
	int i;

	/* Setup PKO for AGL interface. Back pressure is not supported. */
	pko_mem_port_ptrs.u64 = 0;
	read_idx.u64 = 0;
	read_idx.s.inc = 1;
	csr_wr(CVMX_PKO_REG_READ_IDX, read_idx.u64);

	for (i = 0; i < 40; i++) {
		pko_mem_port_ptrs.u64 = csr_rd(CVMX_PKO_MEM_PORT_PTRS);
		if (pko_mem_port_ptrs.s.pid == 24) {
			pko_mem_port_ptrs.s.eid = 10;
			pko_mem_port_ptrs.s.bp_port = 40;
			csr_wr(CVMX_PKO_MEM_PORT_PTRS, pko_mem_port_ptrs.u64);
			break;
		}
	}

	cvmx_agl_enable(port);
	if (do_link_set)
		cvmx_agl_link_set(port, cvmx_agl_link_get(ipd_port));

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
cvmx_helper_link_info_t __cvmx_helper_agl_link_get(int ipd_port)
{
	return cvmx_agl_link_get(ipd_port);
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
int __cvmx_helper_agl_link_set(int ipd_port, cvmx_helper_link_info_t link_info)
{
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int port = cvmx_helper_agl_get_port(interface);

	return cvmx_agl_link_set(port, link_info);
}
