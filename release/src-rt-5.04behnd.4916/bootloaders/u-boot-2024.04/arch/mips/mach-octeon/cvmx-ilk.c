// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Support library for the ILK
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
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pki.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

/*
 * global configurations.
 *
 * for cn68, the default is {0xf, 0xf0}. to disable the 2nd ILK, set
 * cvmx_ilk_lane_mask[CVMX_NUM_ILK_INTF] = {0xff, 0x0} and
 * cvmx_ilk_chans[CVMX_NUM_ILK_INTF] = {8, 0}
 */
unsigned short cvmx_ilk_lane_mask[CVMX_MAX_NODES][CVMX_NUM_ILK_INTF] = {
	[0 ... CVMX_MAX_NODES - 1] = { 0x000f, 0x00f0 }
};

int cvmx_ilk_chans[CVMX_MAX_NODES][CVMX_NUM_ILK_INTF] = {
	[0 ... CVMX_MAX_NODES - 1] = { 8, 8 }
};

static cvmx_ilk_intf_t cvmx_ilk_intf_cfg[CVMX_MAX_NODES][CVMX_NUM_ILK_INTF];

cvmx_ilk_LA_mode_t cvmx_ilk_LA_mode[CVMX_NUM_ILK_INTF] = { { 0, 0 }, { 0, 0 } };
/**
 * User-overrideable callback function that returns whether or not an interface
 * should use look-aside mode.
 *
 * @param interface - interface being checked
 * @param channel - channel number, can be 0 or 1 or -1 to see if LA mode
 *                  should be enabled for the interface.
 * @return 0 to not use LA-mode, 1 to use LA-mode.
 */
int cvmx_ilk_use_la_mode(int interface, int channel)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_0))
		return 0;

	if (interface >= CVMX_NUM_ILK_INTF) {
		debug("ERROR: invalid interface=%d in %s\n",
		      interface, __func__);
		return -1;
	}
	return cvmx_ilk_LA_mode[interface].ilk_LA_mode;
}

/**
 * User-overrideable callback function that returns whether or not an interface
 * in look-aside mode should enable the RX calendar.
 *
 * @param interface - interface to check
 * @return 1 to enable RX calendar, 0 to disable RX calendar.
 *
 * NOTE: For the CN68XX pass 2.0 this will enable the RX calendar for interface
 * 0 and not interface 1.  It is up to the customer to override this behavior.
 */
int cvmx_ilk_la_mode_enable_rx_calendar(int interface)
{
	/* There is an errata in the CN68XX pass 2.0 where if connected
	 * in a loopback configuration or back to back then only one interface
	 * can have the RX calendar enabled.
	 */
	if (interface >= CVMX_NUM_ILK_INTF) {
		debug("ERROR: invalid interface=%d in %s\n",
		      interface, __func__);
		return -1;
	}
	return cvmx_ilk_LA_mode[interface].ilk_LA_mode_cal_ena;
}

/**
 * Initialize and start the ILK interface.
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param lane_mask the lane group for this interface
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_start_interface(int interface, unsigned short lane_mask)
{
	int res = -1;
	int other_intf, this_qlm, other_qlm;
	unsigned short uni_mask;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	cvmx_ilk_ser_cfg_t ilk_ser_cfg;
	int node = (interface >> 4) & 0xf;

	interface &= 0xf;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (lane_mask == 0)
		return res;

	/* check conflicts between 2 ilk interfaces. 1 lane can be assigned to 1
	 * interface only
	 */
	other_intf = !interface;
	if (cvmx_ilk_lane_mask[node][other_intf] & lane_mask) {
		debug("ILK%d:%d: %s: lane assignment conflict\n", node,
		      interface, __func__);
		return res;
	}

	/* check the legality of the lane mask. interface 0 can have 8 lanes,
	 * while interface 1 can have 4 lanes at most
	 */
	uni_mask = lane_mask >> (interface * 4);
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		cvmx_mio_qlmx_cfg_t mio_qlmx_cfg, other_mio_qlmx_cfg;

		if ((uni_mask != 0x1 && uni_mask != 0x3 && uni_mask != 0xf &&
		     uni_mask != 0xff) ||
		    (interface == 1 && lane_mask > 0xf0)) {
			debug("ILK%d: %s: incorrect lane mask: 0x%x\n",
			      interface, __func__, uni_mask);
			return res;
		}
		/* check the availability of qlms. qlm_cfg = 001 means the chip
		 * is fused to give this qlm to ilk
		 */
		this_qlm = interface + CVMX_ILK_QLM_BASE();
		other_qlm = other_intf + CVMX_ILK_QLM_BASE();
		mio_qlmx_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(this_qlm));
		other_mio_qlmx_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(other_qlm));
		if (mio_qlmx_cfg.s.qlm_cfg != 1 ||
		    (uni_mask == 0xff && other_mio_qlmx_cfg.s.qlm_cfg != 1)) {
			debug("ILK%d: %s: qlm unavailable\n", interface,
			      __func__);
			return res;
		}
		/* Has 8 lanes */
		lane_mask &= 0xff;
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		int qlm;
		unsigned short lane_mask_all = 0;

		/* QLM 4 - QLM 7 can be configured for ILK. Get the lane mask
		 * of all the qlms that are configured for ilk
		 */
		for (qlm = 4; qlm < 8; qlm++) {
			cvmx_gserx_cfg_t gserx_cfg;
			cvmx_gserx_phy_ctl_t phy_ctl;

			/* Make sure QLM is powered and out of reset */
			phy_ctl.u64 =
				csr_rd_node(node, CVMX_GSERX_PHY_CTL(qlm));
			if (phy_ctl.s.phy_pd || phy_ctl.s.phy_reset)
				continue;

			/* Make sure QLM is in ILK mode */
			gserx_cfg.u64 = csr_rd_node(node, CVMX_GSERX_CFG(qlm));
			if (gserx_cfg.s.ila)
				lane_mask_all |= ((1 << 4) - 1)
						 << (4 * (qlm - 4));
		}

		if ((lane_mask_all & lane_mask) != lane_mask) {
			debug("ILK%d: %s: incorrect lane mask: 0x%x\n",
			      interface, __func__, lane_mask);
			return res;
		}
	}

	/* power up the serdes */
	ilk_ser_cfg.u64 = csr_rd_node(node, CVMX_ILK_SER_CFG);
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		if (ilk_ser_cfg.cn68xx.ser_pwrup == 0) {
			ilk_ser_cfg.cn68xx.ser_rxpol_auto = 1;
			ilk_ser_cfg.cn68xx.ser_rxpol = 0;
			ilk_ser_cfg.cn68xx.ser_txpol = 0;
			ilk_ser_cfg.cn68xx.ser_reset_n = 0xff;
			ilk_ser_cfg.cn68xx.ser_haul = 0;
		}
		ilk_ser_cfg.cn68xx.ser_pwrup |=
			((interface == 0) && (lane_mask > 0xf)) ?
				0x3 :
				      (1 << interface);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		ilk_ser_cfg.cn78xx.ser_rxpol_auto = 1;
		ilk_ser_cfg.cn78xx.ser_rxpol = 0;
		ilk_ser_cfg.cn78xx.ser_txpol = 0;
		ilk_ser_cfg.cn78xx.ser_reset_n = 0xffff;
	}
	csr_wr_node(node, CVMX_ILK_SER_CFG, ilk_ser_cfg.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X)) {
		/* Workaround for Errata (G-16467) */
		int qlm = (interface) ? 2 : 1;
		int start_qlm, end_qlm;

		/* Apply the workaround to both the QLMs if configured for x8 lanes */
		if (cvmx_pop(lane_mask) > 4) {
			start_qlm = 1;
			end_qlm = 2;
		} else {
			start_qlm = qlm;
			end_qlm = qlm;
		}

		for (qlm = start_qlm; qlm <= end_qlm; qlm++) {
#ifdef CVMX_QLM_DUMP_STATE
			debug("%s:%d: ILK%d: Applying workaround for Errata G-16467\n",
			      __func__, __LINE__, qlm);
			cvmx_qlm_display_registers(qlm);
			debug("\n");
#endif
			/* This workaround only applies to QLMs running ILK at 6.25Ghz */
			if ((cvmx_qlm_get_gbaud_mhz(qlm) == 6250) &&
			    (cvmx_qlm_jtag_get(qlm, 0, "clkf_byp") != 20)) {
				udelay(100); /* Wait 100us for links to stabalize */
				cvmx_qlm_jtag_set(qlm, -1, "clkf_byp", 20);
				/* Allow the QLM to exit reset */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_rst_n_clr", 0);
				udelay(100); /* Wait 100us for links to stabalize */
				/* Allow TX on QLM */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_tx_idle_set",
						  0);
			}
#ifdef CVMX_QLM_DUMP_STATE
			debug("%s:%d: ILK%d: Done applying workaround for Errata G-16467\n",
			      __func__, __LINE__, qlm);
			cvmx_qlm_display_registers(qlm);
			debug("\n\n");
#endif
		}
	}

	/* Initialize all calendar entries to xoff state */
	__cvmx_ilk_clear_cal((node << 4) | interface);

	/* Enable ILK LA mode if configured. */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		if (cvmx_ilk_use_la_mode(interface, 0)) {
			cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
			cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;

			ilk_txx_cfg1.u64 = csr_rd(CVMX_ILK_TXX_CFG1(interface));
			ilk_rxx_cfg1.u64 = csr_rd(CVMX_ILK_RXX_CFG1(interface));
			ilk_txx_cfg1.s.la_mode = 1;
			ilk_txx_cfg1.s.tx_link_fc_jam = 1;
			ilk_txx_cfg1.s.rx_link_fc_ign = 1;
			ilk_rxx_cfg1.s.la_mode = 1;
			csr_wr(CVMX_ILK_TXX_CFG1(interface), ilk_txx_cfg1.u64);
			csr_wr(CVMX_ILK_RXX_CFG1(interface), ilk_rxx_cfg1.u64);
			cvmx_ilk_intf_cfg[node][interface].la_mode =
				1; /* Enable look-aside mode */
		} else {
			cvmx_ilk_intf_cfg[node][interface].la_mode =
				0; /* Disable look-aside mode */
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		cvmx_ilk_intf_cfg[node][interface].la_mode = 0;

	/* configure the lane enable of the interface */
	ilk_txx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_TXX_CFG0(interface));
	ilk_rxx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG0(interface));
	ilk_rxx_cfg0.s.lane_ena = lane_mask;
	ilk_txx_cfg0.s.lane_ena = lane_mask;
	csr_wr_node(node, CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
	csr_wr_node(node, CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);

	/* For 10.3125Gbs data rate, set SER_LIMIT to 0x3ff for x8 & x12 mode */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_gserx_lane_mode_t lmode0, lmode1;

		lmode0.u64 = csr_rd_node(node, CVMX_GSERX_LANE_MODE(5));
		lmode1.u64 = csr_rd_node(node, CVMX_GSERX_LANE_MODE(7));
		if ((lmode0.s.lmode == 5 || lmode1.s.lmode == 5) &&
		    (lane_mask == 0xfff || lane_mask == 0xfff0 ||
		     lane_mask == 0xff || lane_mask == 0xff00)) {
			cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;

			ilk_txx_cfg1.u64 =
				csr_rd_node(node, CVMX_ILK_TXX_CFG1(interface));
			ilk_txx_cfg1.s.ser_limit = 0x3ff;
			csr_wr_node(node, CVMX_ILK_TXX_CFG1(interface),
				    ilk_txx_cfg1.u64);
		}
	}

	/* write to local cache. for lane speed, if interface 0 has 8 lanes,
	 * assume both qlms have the same speed
	 */
	cvmx_ilk_intf_cfg[node][interface].intf_en = 1;
	res = 0;

	return res;
}

/**
 * set pipe group base and length for the interface
 *
 * @param xiface    The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param pipe_base the base of the pipe group
 * @param pipe_len  the length of the pipe group
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_set_pipe(int xiface, int pipe_base, unsigned int pipe_len)
{
	int res = -1;
	cvmx_ilk_txx_pipe_t ilk_txx_pipe;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface - CVMX_ILK_GBL_BASE();

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	/* set them in ilk tx section */
	ilk_txx_pipe.u64 = csr_rd_node(xi.node, CVMX_ILK_TXX_PIPE(interface));
	ilk_txx_pipe.s.base = pipe_base;
	ilk_txx_pipe.s.nump = pipe_len;
	csr_wr_node(xi.node, CVMX_ILK_TXX_PIPE(interface), ilk_txx_pipe.u64);
	res = 0;

	return res;
}

/**
 * set logical channels for tx
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param pch     pointer to an array of pipe-channel pair
 * @param num_chs the number of entries in the pipe-channel array
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_tx_set_channel(int interface, cvmx_ilk_pipe_chan_t *pch,
			    unsigned int num_chs)
{
	int res = -1;
	cvmx_ilk_txx_idx_pmap_t ilk_txx_idx_pmap;
	cvmx_ilk_txx_mem_pmap_t ilk_txx_mem_pmap;
	unsigned int i;

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (!pch || num_chs > CVMX_ILK_MAX_PIPES)
		return res;

	if (cvmx_ilk_use_la_mode(interface, 0)) {
		ilk_txx_idx_pmap.u64 = 0;
		ilk_txx_mem_pmap.u64 = 0;
		for (i = 0; i < num_chs; i++) {
			ilk_txx_idx_pmap.s.index = pch->pipe;
			ilk_txx_mem_pmap.s.channel = pch->chan;
			ilk_txx_mem_pmap.s.remap = 1;
			csr_wr(CVMX_ILK_TXX_IDX_PMAP(interface),
			       ilk_txx_idx_pmap.u64);
			csr_wr(CVMX_ILK_TXX_MEM_PMAP(interface),
			       ilk_txx_mem_pmap.u64);
			pch++;
		}
	} else {
		/* write the pair to ilk tx */
		ilk_txx_mem_pmap.u64 = 0;
		ilk_txx_idx_pmap.u64 = 0;
		for (i = 0; i < num_chs; i++) {
			ilk_txx_idx_pmap.s.index = pch->pipe;
			ilk_txx_mem_pmap.s.channel = pch->chan;
			csr_wr(CVMX_ILK_TXX_IDX_PMAP(interface),
			       ilk_txx_idx_pmap.u64);
			csr_wr(CVMX_ILK_TXX_MEM_PMAP(interface),
			       ilk_txx_mem_pmap.u64);
			pch++;
		}
	}
	res = 0;

	return res;
}

/**
 * set pkind for rx
 *
 * @param xiface    The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param chpknd    pointer to an array of channel-pkind pair
 * @param num_pknd the number of entries in the channel-pkind array
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_set_pknd(int xiface, cvmx_ilk_chan_pknd_t *chpknd,
			 unsigned int num_pknd)
{
	int res = -1;
	cvmx_ilk_rxf_idx_pmap_t ilk_rxf_idx_pmap;
	unsigned int i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface - CVMX_ILK_GBL_BASE();

	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (!chpknd || num_pknd > CVMX_ILK_MAX_PKNDS)
		return res;

	res = 0;

	for (i = 0; i < num_pknd; i++) {
		ilk_rxf_idx_pmap.u64 = 0;
		/* write the pair to ilk rx. note the channels for different
		 * interfaces are given in *chpknd and interface is not used
		 * as a param
		 */
		if (chpknd->chan < 2 &&
		    cvmx_ilk_use_la_mode(interface, chpknd->chan)) {
			ilk_rxf_idx_pmap.s.index =
				interface * 256 + 128 + chpknd->chan;
			csr_wr(CVMX_ILK_RXF_IDX_PMAP, ilk_rxf_idx_pmap.u64);
			csr_wr(CVMX_ILK_RXF_MEM_PMAP, chpknd->pknd);
		}
		ilk_rxf_idx_pmap.s.index = interface * 256 + chpknd->chan;
		csr_wr(CVMX_ILK_RXF_IDX_PMAP, ilk_rxf_idx_pmap.u64);
		csr_wr(CVMX_ILK_RXF_MEM_PMAP, chpknd->pknd);
		chpknd++;
	}

	return res;
}

/**
 * configure calendar for rx
 *
 * @param intf The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_cal_conf(int intf, int cal_depth, cvmx_ilk_cal_entry_t *pent)
{
	int res = -1, i;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	int num_entries;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (cal_depth < CVMX_ILK_RX_MIN_CAL || cal_depth > CVMX_ILK_MAX_CAL ||
	    (OCTEON_IS_MODEL(OCTEON_CN68XX) && !pent))
		return res;

	/* mandatory link-level fc as workarounds for ILK-15397 and
	 * ILK-15479
	 */
	/* TODO: test effectiveness */

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Update the calendar for each channel */
		if ((cvmx_ilk_use_la_mode(interface, 0) == 0) ||
		    (cvmx_ilk_use_la_mode(interface, 0) &&
		     cvmx_ilk_la_mode_enable_rx_calendar(interface))) {
			for (i = 0; i < cal_depth; i++) {
				__cvmx_ilk_write_rx_cal_entry(
					interface, i, pent[i].pipe_bpid);
			}
		}

		/* Update the depth */
		ilk_rxx_cfg0.u64 = csr_rd(CVMX_ILK_RXX_CFG0(interface));
		num_entries = 1 + cal_depth + (cal_depth - 1) / 15;
		ilk_rxx_cfg0.s.cal_depth = num_entries;
		if (cvmx_ilk_use_la_mode(interface, 0)) {
			ilk_rxx_cfg0.s.mproto_ign = 1;
			ilk_rxx_cfg0.s.lnk_stats_ena = 1;
			ilk_rxx_cfg0.s.lnk_stats_wrap = 1;
		}
		csr_wr(CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		ilk_rxx_cfg0.u64 =
			csr_rd_node(node, CVMX_ILK_RXX_CFG0(interface));
		/*
		 * Make sure cal_ena is 0 for programming the calendar table,
		 * as per Errata ILK-19398
		 */
		ilk_rxx_cfg0.s.cal_ena = 0;
		csr_wr_node(node, CVMX_ILK_RXX_CFG0(interface),
			    ilk_rxx_cfg0.u64);

		for (i = 0; i < cal_depth; i++)
			__cvmx_ilk_write_rx_cal_entry(intf, i, 0);

		ilk_rxx_cfg0.u64 =
			csr_rd_node(node, CVMX_ILK_RXX_CFG0(interface));
		num_entries = 1 + cal_depth + (cal_depth - 1) / 15;
		ilk_rxx_cfg0.s.cal_depth = num_entries;
		csr_wr_node(node, CVMX_ILK_RXX_CFG0(interface),
			    ilk_rxx_cfg0.u64);
	}

	return 0;
}

/**
 * set high water mark for rx
 *
 * @param intf      The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param hi_wm     high water mark for this interface
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_set_hwm(int intf, int hi_wm)
{
	int res = -1;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (hi_wm <= 0)
		return res;

	/* set the hwm */
	ilk_rxx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG1(interface));
	ilk_rxx_cfg1.s.rx_fifo_hwm = hi_wm;
	csr_wr_node(node, CVMX_ILK_RXX_CFG1(interface), ilk_rxx_cfg1.u64);
	res = 0;

	return res;
}

/**
 * enable calendar for rx
 *
 * @param intf      The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_rx_cal_ena(int intf, unsigned char cal_ena)
{
	int res = -1;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (cvmx_ilk_use_la_mode(interface, 0) &&
	    !cvmx_ilk_la_mode_enable_rx_calendar(interface))
		return 0;

	/* set the enable */
	ilk_rxx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG0(interface));
	ilk_rxx_cfg0.s.cal_ena = cal_ena;
	csr_wr_node(node, CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);
	csr_rd_node(node, CVMX_ILK_RXX_CFG0(interface));
	res = 0;

	return res;
}

/**
 * set up calendar for rx
 *
 * @param intf      The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 * @param hi_wm     high water mark for this interface
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_cal_setup_rx(int intf, int cal_depth, cvmx_ilk_cal_entry_t *pent,
			  int hi_wm, unsigned char cal_ena)
{
	int res = -1;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	res = cvmx_ilk_rx_cal_conf(intf, cal_depth, pent);
	if (res < 0)
		return res;

	res = cvmx_ilk_rx_set_hwm(intf, hi_wm);
	if (res < 0)
		return res;

	res = cvmx_ilk_rx_cal_ena(intf, cal_ena);
	return res;
}

/**
 * configure calendar for tx
 *
 * @param intf      The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_tx_cal_conf(int intf, int cal_depth, cvmx_ilk_cal_entry_t *pent)
{
	int res = -1, i;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	int num_entries;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	if (cal_depth < CVMX_ILK_TX_MIN_CAL || cal_depth > CVMX_ILK_MAX_CAL ||
	    (OCTEON_IS_MODEL(OCTEON_CN68XX) && !pent))
		return res;

	/* mandatory link-level fc as workarounds for ILK-15397 and
	 * ILK-15479
	 */
	/* TODO: test effectiveness */

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Update the calendar for each channel */
		for (i = 0; i < cal_depth; i++) {
			__cvmx_ilk_write_tx_cal_entry(interface, i,
						      pent[i].pipe_bpid);
		}

		/* Set the depth (must be multiple of 8)*/
		ilk_txx_cfg0.u64 = csr_rd(CVMX_ILK_TXX_CFG0(interface));
		num_entries = 1 + cal_depth + (cal_depth - 1) / 15;
		ilk_txx_cfg0.s.cal_depth = (num_entries + 7) & ~7;
		csr_wr(CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		ilk_txx_cfg0.u64 =
			csr_rd_node(node, CVMX_ILK_TXX_CFG0(interface));
		/*
		 * Make sure cal_ena is 0 for programming the calendar table,
		 * as per Errata ILK-19398
		 */
		ilk_txx_cfg0.s.cal_ena = 0;
		csr_wr_node(node, CVMX_ILK_TXX_CFG0(interface),
			    ilk_txx_cfg0.u64);

		for (i = 0; i < cal_depth; i++)
			__cvmx_ilk_write_tx_cal_entry(intf, i, 0);

		ilk_txx_cfg0.u64 =
			csr_rd_node(node, CVMX_ILK_TXX_CFG0(interface));
		num_entries = 1 + cal_depth + (cal_depth - 1) / 15;
		/* cal_depth[2:0] needs to be zero, round up */
		ilk_txx_cfg0.s.cal_depth = (num_entries + 7) & 0x1f8;
		csr_wr_node(node, CVMX_ILK_TXX_CFG0(interface),
			    ilk_txx_cfg0.u64);
	}

	return 0;
}

/**
 * enable calendar for tx
 *
 * @param intf	    The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_tx_cal_ena(int intf, unsigned char cal_ena)
{
	int res = -1;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	/* set the enable */
	ilk_txx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_TXX_CFG0(interface));
	ilk_txx_cfg0.s.cal_ena = cal_ena;
	csr_wr_node(node, CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
	csr_rd_node(node, CVMX_ILK_TXX_CFG0(interface));
	res = 0;

	return res;
}

/**
 * set up calendar for tx
 *
 * @param intf      The identifier of the packet interface to configure and
 *                  use as a ILK interface. cn68xx has 2 interfaces: ilk0 and
 *                  ilk1.
 *
 * @param cal_depth the number of calendar entries
 * @param pent      pointer to calendar entries
 * @param cal_ena   enable or disable calendar
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_cal_setup_tx(int intf, int cal_depth, cvmx_ilk_cal_entry_t *pent,
			  unsigned char cal_ena)
{
	int res = -1;

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	res = cvmx_ilk_tx_cal_conf(intf, cal_depth, pent);
	if (res < 0)
		return res;

	res = cvmx_ilk_tx_cal_ena(intf, cal_ena);
	return res;
}

/* #define CVMX_ILK_STATS_ENA 1 */
#ifdef CVMX_ILK_STATS_ENA
static void cvmx_ilk_reg_dump_rx(int intf)
{
	int i;
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
	cvmx_ilk_rxx_int_t ilk_rxx_int;
	cvmx_ilk_rxx_jabber_t ilk_rxx_jabber;
	cvmx_ilk_rx_lnex_cfg_t ilk_rx_lnex_cfg;
	cvmx_ilk_rx_lnex_int_t ilk_rx_lnex_int;
	cvmx_ilk_gbl_cfg_t ilk_gbl_cfg;
	cvmx_ilk_ser_cfg_t ilk_ser_cfg;
	cvmx_ilk_rxf_idx_pmap_t ilk_rxf_idx_pmap;
	cvmx_ilk_rxf_mem_pmap_t ilk_rxf_mem_pmap;
	cvmx_ilk_rxx_idx_cal_t ilk_rxx_idx_cal;
	cvmx_ilk_rxx_mem_cal0_t ilk_rxx_mem_cal0;
	cvmx_ilk_rxx_mem_cal1_t ilk_rxx_mem_cal1;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	ilk_rxx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG0(interface));
	debug("ilk rxx cfg0: 0x%16lx\n", ilk_rxx_cfg0.u64);

	ilk_rxx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG1(interface));
	debug("ilk rxx cfg1: 0x%16lx\n", ilk_rxx_cfg1.u64);

	ilk_rxx_int.u64 = csr_rd_node(node, CVMX_ILK_RXX_INT(interface));
	debug("ilk rxx int: 0x%16lx\n", ilk_rxx_int.u64);
	csr_wr_node(node, CVMX_ILK_RXX_INT(interface), ilk_rxx_int.u64);

	ilk_rxx_jabber.u64 = csr_rd_node(node, CVMX_ILK_RXX_JABBER(interface));
	debug("ilk rxx jabber: 0x%16lx\n", ilk_rxx_jabber.u64);

#define LNE_NUM_DBG 4
	for (i = 0; i < LNE_NUM_DBG; i++) {
		ilk_rx_lnex_cfg.u64 =
			csr_rd_node(node, CVMX_ILK_RX_LNEX_CFG(i));
		debug("ilk rx lnex cfg lane: %d  0x%16lx\n", i,
		      ilk_rx_lnex_cfg.u64);
	}

	for (i = 0; i < LNE_NUM_DBG; i++) {
		ilk_rx_lnex_int.u64 =
			csr_rd_node(node, CVMX_ILK_RX_LNEX_INT(i));
		debug("ilk rx lnex int lane: %d  0x%16lx\n", i,
		      ilk_rx_lnex_int.u64);
		csr_wr_node(node, CVMX_ILK_RX_LNEX_INT(i), ilk_rx_lnex_int.u64);
	}

	ilk_gbl_cfg.u64 = csr_rd_node(node, CVMX_ILK_GBL_CFG);
	debug("ilk gbl cfg: 0x%16lx\n", ilk_gbl_cfg.u64);

	ilk_ser_cfg.u64 = csr_rd_node(node, CVMX_ILK_SER_CFG);
	debug("ilk ser cfg: 0x%16lx\n", ilk_ser_cfg.u64);

#define CHAN_NUM_DBG 8
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_rxf_idx_pmap.u64 = 0;
		ilk_rxf_idx_pmap.s.index = interface * 256;
		ilk_rxf_idx_pmap.s.inc = 1;
		csr_wr(CVMX_ILK_RXF_IDX_PMAP, ilk_rxf_idx_pmap.u64);
		for (i = 0; i < CHAN_NUM_DBG; i++) {
			ilk_rxf_mem_pmap.u64 = csr_rd(CVMX_ILK_RXF_MEM_PMAP);
			debug("ilk rxf mem pmap chan: %3d  0x%16lx\n", i,
			      ilk_rxf_mem_pmap.u64);
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_rxx_chax_t rxx_chax;

		for (i = 0; i < CHAN_NUM_DBG; i++) {
			rxx_chax.u64 = csr_rd_node(
				node, CVMX_ILK_RXX_CHAX(i, interface));
			debug("ilk chan: %d  pki chan: 0x%x\n", i,
			      rxx_chax.s.port_kind);
		}
	}

#define CAL_NUM_DBG 2
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_rxx_idx_cal.u64 = 0;
		ilk_rxx_idx_cal.s.inc = 1;
		csr_wr(CVMX_ILK_RXX_IDX_CAL(interface), ilk_rxx_idx_cal.u64);
		for (i = 0; i < CAL_NUM_DBG; i++) {
			ilk_rxx_idx_cal.u64 =
				csr_rd(CVMX_ILK_RXX_IDX_CAL(interface));
			debug("ilk rxx idx cal: 0x%16lx\n",
			      ilk_rxx_idx_cal.u64);

			ilk_rxx_mem_cal0.u64 =
				csr_rd(CVMX_ILK_RXX_MEM_CAL0(interface));
			debug("ilk rxx mem cal0: 0x%16lx\n",
			      ilk_rxx_mem_cal0.u64);
			ilk_rxx_mem_cal1.u64 =
				csr_rd(CVMX_ILK_RXX_MEM_CAL1(interface));
			debug("ilk rxx mem cal1: 0x%16lx\n",
			      ilk_rxx_mem_cal1.u64);
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_rxx_cal_entryx_t rxx_cal_entryx;

		for (i = 0; i < CAL_NUM_DBG; i++) {
			rxx_cal_entryx.u64 = csr_rd_node(
				node, CVMX_ILK_RXX_CAL_ENTRYX(i, interface));
			debug("ilk rxx cal idx: %d\n", i);
			debug("ilk rxx cal ctl: 0x%x\n", rxx_cal_entryx.s.ctl);
			debug("ilk rxx cal pko chan: 0x%x\n",
			      rxx_cal_entryx.s.channel);
		}
	}
}

static void cvmx_ilk_reg_dump_tx(int intf)
{
	int i;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
	cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
	cvmx_ilk_txx_idx_pmap_t ilk_txx_idx_pmap;
	cvmx_ilk_txx_mem_pmap_t ilk_txx_mem_pmap;
	cvmx_ilk_txx_int_t ilk_txx_int;
	cvmx_ilk_txx_pipe_t ilk_txx_pipe;
	cvmx_ilk_txx_idx_cal_t ilk_txx_idx_cal;
	cvmx_ilk_txx_mem_cal0_t ilk_txx_mem_cal0;
	cvmx_ilk_txx_mem_cal1_t ilk_txx_mem_cal1;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	ilk_txx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_TXX_CFG0(interface));
	debug("ilk txx cfg0: 0x%16lx\n", ilk_txx_cfg0.u64);

	ilk_txx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_TXX_CFG1(interface));
	debug("ilk txx cfg1: 0x%16lx\n", ilk_txx_cfg1.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_txx_pipe.u64 = csr_rd(CVMX_ILK_TXX_PIPE(interface));
		debug("ilk txx pipe: 0x%16lx\n", ilk_txx_pipe.u64);

		ilk_txx_idx_pmap.u64 = 0;
		ilk_txx_idx_pmap.s.index = ilk_txx_pipe.s.base;
		ilk_txx_idx_pmap.s.inc = 1;
		csr_wr(CVMX_ILK_TXX_IDX_PMAP(interface), ilk_txx_idx_pmap.u64);
		for (i = 0; i < CHAN_NUM_DBG; i++) {
			ilk_txx_mem_pmap.u64 =
				csr_rd(CVMX_ILK_TXX_MEM_PMAP(interface));
			debug("ilk txx mem pmap pipe: %3d  0x%16lx\n",
			      ilk_txx_pipe.s.base + i, ilk_txx_mem_pmap.u64);
		}
	}

	ilk_txx_int.u64 = csr_rd_node(node, CVMX_ILK_TXX_INT(interface));
	debug("ilk txx int: 0x%16lx\n", ilk_txx_int.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_txx_idx_cal.u64 = 0;
		ilk_txx_idx_cal.s.inc = 1;
		csr_wr(CVMX_ILK_TXX_IDX_CAL(interface), ilk_txx_idx_cal.u64);
		for (i = 0; i < CAL_NUM_DBG; i++) {
			ilk_txx_idx_cal.u64 =
				csr_rd(CVMX_ILK_TXX_IDX_CAL(interface));
			debug("ilk txx idx cal: 0x%16lx\n",
			      ilk_txx_idx_cal.u64);

			ilk_txx_mem_cal0.u64 =
				csr_rd(CVMX_ILK_TXX_MEM_CAL0(interface));
			debug("ilk txx mem cal0: 0x%16lx\n",
			      ilk_txx_mem_cal0.u64);
			ilk_txx_mem_cal1.u64 =
				csr_rd(CVMX_ILK_TXX_MEM_CAL1(interface));
			debug("ilk txx mem cal1: 0x%16lx\n",
			      ilk_txx_mem_cal1.u64);
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ilk_txx_cal_entryx_t txx_cal_entryx;

		for (i = 0; i < CAL_NUM_DBG; i++) {
			txx_cal_entryx.u64 = csr_rd_node(
				node, CVMX_ILK_TXX_CAL_ENTRYX(i, interface));
			debug("ilk txx cal idx: %d\n", i);
			debug("ilk txx cal ctl: 0x%x\n", txx_cal_entryx.s.ctl);
			debug("ilk txx cal pki chan: 0x%x\n",
			      txx_cal_entryx.s.channel);
		}
	}
}
#endif

/**
 * show run time status
 *
 * @param interface The identifier of the packet interface to enable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return nothing
 */
#ifdef CVMX_ILK_RUNTIME_DBG
void cvmx_ilk_runtime_status(int interface)
{
	cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
	cvmx_ilk_txx_flow_ctl0_t ilk_txx_flow_ctl0;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
	cvmx_ilk_rxx_int_t ilk_rxx_int;
	cvmx_ilk_rxx_flow_ctl0_t ilk_rxx_flow_ctl0;
	cvmx_ilk_rxx_flow_ctl1_t ilk_rxx_flow_ctl1;
	cvmx_ilk_gbl_int_t ilk_gbl_int;

	debug("\nilk run-time status: interface: %d\n", interface);

	ilk_txx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_TXX_CFG1(interface));
	debug("\nilk txx cfg1: 0x%16lx\n", ilk_txx_cfg1.u64);
	if (ilk_txx_cfg1.s.rx_link_fc)
		debug("link flow control received\n");
	if (ilk_txx_cfg1.s.tx_link_fc)
		debug("link flow control sent\n");

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_txx_flow_ctl0.u64 =
			csr_rd(CVMX_ILK_TXX_FLOW_CTL0(interface));
		debug("\nilk txx flow ctl0: 0x%16lx\n", ilk_txx_flow_ctl0.u64);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		int i;
		cvmx_ilk_txx_cha_xonx_t txx_cha_xonx;

		for (i = 0; i < 4; i++) {
			txx_cha_xonx.u64 = csr_rd_node(
				node, CVMX_ILK_TXX_CHA_XONX(i, interface));
			debug("\nilk txx cha xon: 0x%16lx\n", txx_cha_xonx.u64);
		}
	}

	ilk_rxx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG1(interface));
	debug("\nilk rxx cfg1: 0x%16lx\n", ilk_rxx_cfg1.u64);
	debug("rx fifo count: %d\n", ilk_rxx_cfg1.s.rx_fifo_cnt);

	ilk_rxx_int.u64 = csr_rd_node(node, CVMX_ILK_RXX_INT(interface));
	debug("\nilk rxx int: 0x%16lx\n", ilk_rxx_int.u64);
	if (ilk_rxx_int.s.pkt_drop_rxf)
		debug("rx fifo packet drop\n");
	if (ilk_rxx_int.u64)
		csr_wr_node(node, CVMX_ILK_RXX_INT(interface), ilk_rxx_int.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		ilk_rxx_flow_ctl0.u64 =
			csr_rd(CVMX_ILK_RXX_FLOW_CTL0(interface));
		debug("\nilk rxx flow ctl0: 0x%16lx\n", ilk_rxx_flow_ctl0.u64);

		ilk_rxx_flow_ctl1.u64 =
			csr_rd(CVMX_ILK_RXX_FLOW_CTL1(interface));
		debug("\nilk rxx flow ctl1: 0x%16lx\n", ilk_rxx_flow_ctl1.u64);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		int i;
		cvmx_ilk_rxx_cha_xonx_t rxx_cha_xonx;

		for (i = 0; i < 4; i++) {
			rxx_cha_xonx.u64 = csr_rd_node(
				node, CVMX_ILK_RXX_CHA_XONX(i, interface));
			debug("\nilk rxx cha xon: 0x%16lx\n", rxx_cha_xonx.u64);
		}
	}

	ilk_gbl_int.u64 = csr_rd_node(node, CVMX_ILK_GBL_INT);
	debug("\nilk gbl int: 0x%16lx\n", ilk_gbl_int.u64);
	if (ilk_gbl_int.s.rxf_push_full)
		debug("rx fifo overflow\n");
	if (ilk_gbl_int.u64)
		csr_wr_node(node, CVMX_ILK_GBL_INT, ilk_gbl_int.u64);
}
#endif

/**
 * enable interface
 *
 * @param xiface    The identifier of the packet interface to enable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_ilk_enable(int xiface)
{
	int res = -1;
	int retry_count = 0;
	cvmx_helper_link_info_t result;
	cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
#ifdef CVMX_ILK_STATS_ENA
	cvmx_ilk_rxx_cfg0_t ilk_rxx_cfg0;
	cvmx_ilk_txx_cfg0_t ilk_txx_cfg0;
#endif
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	int interface = xi.interface - CVMX_ILK_GBL_BASE();

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	if (interface >= CVMX_NUM_ILK_INTF)
		return res;

	result.u64 = 0;

#ifdef CVMX_ILK_STATS_ENA
	debug("\n");
	debug("<<<< ILK%d: Before enabling ilk\n", interface);
	cvmx_ilk_reg_dump_rx(intf);
	cvmx_ilk_reg_dump_tx(intf);
#endif

	/* RX packet will be enabled only if link is up */

	/* TX side */
	ilk_txx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_TXX_CFG1(interface));
	ilk_txx_cfg1.s.pkt_ena = 1;
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		if (cvmx_ilk_use_la_mode(interface, 0)) {
			ilk_txx_cfg1.s.la_mode = 1;
			ilk_txx_cfg1.s.tx_link_fc_jam = 1;
		}
	}
	csr_wr_node(node, CVMX_ILK_TXX_CFG1(interface), ilk_txx_cfg1.u64);
	csr_rd_node(node, CVMX_ILK_TXX_CFG1(interface));

#ifdef CVMX_ILK_STATS_ENA
	/* RX side stats */
	ilk_rxx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG0(interface));
	ilk_rxx_cfg0.s.lnk_stats_ena = 1;
	csr_wr_node(node, CVMX_ILK_RXX_CFG0(interface), ilk_rxx_cfg0.u64);

	/* TX side stats */
	ilk_txx_cfg0.u64 = csr_rd_node(node, CVMX_ILK_TXX_CFG0(interface));
	ilk_txx_cfg0.s.lnk_stats_ena = 1;
	csr_wr_node(node, CVMX_ILK_TXX_CFG0(interface), ilk_txx_cfg0.u64);
#endif

retry:
	retry_count++;
	if (retry_count > 10)
		goto out;

	/* Make sure the link is up, so that packets can be sent. */
	result = __cvmx_helper_ilk_link_get(
		cvmx_helper_get_ipd_port((interface + CVMX_ILK_GBL_BASE()), 0));

	/* Small delay before another retry. */
	udelay(100);

	ilk_rxx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG1(interface));
	if (ilk_rxx_cfg1.s.pkt_ena == 0)
		goto retry;

out:

#ifdef CVMX_ILK_STATS_ENA
	debug(">>>> ILK%d: After ILK is enabled\n", interface);
	cvmx_ilk_reg_dump_rx(intf);
	cvmx_ilk_reg_dump_tx(intf);
#endif

	if (result.s.link_up)
		return 0;

	return -1;
}

/**
 * Provide interface enable status
 *
 * @param xiface The identifier of the packet xiface to disable. cn68xx
 *                  has 2 interfaces: ilk0 and ilk1.
 *
 * @return Zero, not enabled; One, enabled.
 */
int cvmx_ilk_get_intf_ena(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface - CVMX_ILK_GBL_BASE();
	return cvmx_ilk_intf_cfg[xi.node][interface].intf_en;
}
