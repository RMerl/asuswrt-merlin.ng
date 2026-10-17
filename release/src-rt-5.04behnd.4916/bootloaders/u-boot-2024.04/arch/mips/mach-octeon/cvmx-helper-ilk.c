// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for ILK initialization, configuration,
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

int __cvmx_helper_ilk_enumerate(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	xi.interface -= CVMX_ILK_GBL_BASE();
	return cvmx_ilk_chans[xi.node][xi.interface];
}

/**
 * @INTERNAL
 * Initialize all tx calendar entries to the xoff state.
 * Initialize all rx calendar entries to the xon state. The rx calendar entries
 * must be in the xon state to allow new pko pipe assignments. If a calendar
 * entry is assigned a different pko pipe while in the xoff state, the old pko
 * pipe will stay in the xoff state even when no longer used by ilk.
 *
 * @param intf Interface whose calendar are to be initialized.
 */
static void __cvmx_ilk_clear_cal_cn78xx(int intf)
{
	cvmx_ilk_txx_cal_entryx_t tx_entry;
	cvmx_ilk_rxx_cal_entryx_t rx_entry;
	int i;
	int node = (intf >> 4) & 0xf;
	int interface = (intf & 0xf);

	/* Initialize all tx calendar entries to off */
	tx_entry.u64 = 0;
	tx_entry.s.ctl = XOFF;
	for (i = 0; i < CVMX_ILK_MAX_CAL; i++) {
		csr_wr_node(node, CVMX_ILK_TXX_CAL_ENTRYX(i, interface),
			    tx_entry.u64);
	}

	/* Initialize all rx calendar entries to on */
	rx_entry.u64 = 0;
	rx_entry.s.ctl = XOFF;
	for (i = 0; i < CVMX_ILK_MAX_CAL; i++) {
		csr_wr_node(node, CVMX_ILK_RXX_CAL_ENTRYX(i, interface),
			    rx_entry.u64);
	}
}

/**
 * @INTERNAL
 * Initialize all tx calendar entries to the xoff state.
 * Initialize all rx calendar entries to the xon state. The rx calendar entries
 * must be in the xon state to allow new pko pipe assignments. If a calendar
 * entry is assigned a different pko pipe while in the xoff state, the old pko
 * pipe will stay in the xoff state even when no longer used by ilk.
 *
 * @param interface whose calendar are to be initialized.
 */
static void __cvmx_ilk_clear_cal_cn68xx(int interface)
{
	cvmx_ilk_txx_idx_cal_t tx_idx;
	cvmx_ilk_txx_mem_cal0_t tx_cal0;
	cvmx_ilk_txx_mem_cal1_t tx_cal1;
	cvmx_ilk_rxx_idx_cal_t rx_idx;
	cvmx_ilk_rxx_mem_cal0_t rx_cal0;
	cvmx_ilk_rxx_mem_cal1_t rx_cal1;
	int i;

	/*
	 * First we initialize the tx calendar starting from entry 0,
	 * incrementing the entry with every write.
	 */
	tx_idx.u64 = 0;
	tx_idx.s.inc = 1;
	csr_wr(CVMX_ILK_TXX_IDX_CAL(interface), tx_idx.u64);

	/* Set state to xoff for all entries */
	tx_cal0.u64 = 0;
	tx_cal0.s.entry_ctl0 = XOFF;
	tx_cal0.s.entry_ctl1 = XOFF;
	tx_cal0.s.entry_ctl2 = XOFF;
	tx_cal0.s.entry_ctl3 = XOFF;

	tx_cal1.u64 = 0;
	tx_cal1.s.entry_ctl4 = XOFF;
	tx_cal1.s.entry_ctl5 = XOFF;
	tx_cal1.s.entry_ctl6 = XOFF;
	tx_cal1.s.entry_ctl7 = XOFF;

	/* Write all 288 entries */
	for (i = 0; i < CVMX_ILK_MAX_CAL_IDX; i++) {
		csr_wr(CVMX_ILK_TXX_MEM_CAL0(interface), tx_cal0.u64);
		csr_wr(CVMX_ILK_TXX_MEM_CAL1(interface), tx_cal1.u64);
	}

	/*
	 * Next we initialize the rx calendar starting from entry 0,
	 * incrementing the entry with every write.
	 */
	rx_idx.u64 = 0;
	rx_idx.s.inc = 1;
	csr_wr(CVMX_ILK_RXX_IDX_CAL(interface), rx_idx.u64);

	/* Set state to xon for all entries */
	rx_cal0.u64 = 0;
	rx_cal0.s.entry_ctl0 = XON;
	rx_cal0.s.entry_ctl1 = XON;
	rx_cal0.s.entry_ctl2 = XON;
	rx_cal0.s.entry_ctl3 = XON;

	rx_cal1.u64 = 0;
	rx_cal1.s.entry_ctl4 = XON;
	rx_cal1.s.entry_ctl5 = XON;
	rx_cal1.s.entry_ctl6 = XON;
	rx_cal1.s.entry_ctl7 = XON;

	/* Write all 288 entries */
	for (i = 0; i < CVMX_ILK_MAX_CAL_IDX; i++) {
		csr_wr(CVMX_ILK_RXX_MEM_CAL0(interface), rx_cal0.u64);
		csr_wr(CVMX_ILK_RXX_MEM_CAL1(interface), rx_cal1.u64);
	}
}

/**
 * @INTERNAL
 * Initialize all calendar entries.
 *
 * @param interface whose calendar is to be initialized.
 */
void __cvmx_ilk_clear_cal(int interface)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		__cvmx_ilk_clear_cal_cn68xx(interface);
	else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		__cvmx_ilk_clear_cal_cn78xx(interface);
}

void __cvmx_ilk_write_tx_cal_entry_cn68xx(int interface, int channel,
					  unsigned char bpid)
{
	cvmx_ilk_txx_idx_cal_t tx_idx;
	cvmx_ilk_txx_mem_cal0_t tx_cal0;
	cvmx_ilk_txx_mem_cal1_t tx_cal1;
	int entry;
	int window;
	int window_entry;

	/*
	 * The calendar has 288 entries. Each calendar entry represents a
	 * channel's flow control state or the link flow control state.
	 * Starting with the first entry, every sixteenth entry is used for the
	 * link flow control state. The other 15 entries are used for the
	 * channels flow control state:
	 * entry 0   ----> link flow control state
	 * entry 1   ----> channel 0 flow control state
	 * entry 2   ----> channel 1 flow control state
	 * ...
	 * entry 15  ----> channel 14 flow control state
	 * entry 16  ----> link flow control state
	 * entry 17  ----> channel 15 flow control state
	 *
	 * Also, the calendar is accessed via windows into it. Each window maps
	 * to 8 entries.
	 */
	entry = 1 + channel + (channel / 15);
	window = entry / 8;
	window_entry = entry % 8;

	/* Indicate the window we need to access */
	tx_idx.u64 = 0;
	tx_idx.s.index = window;
	csr_wr(CVMX_ILK_TXX_IDX_CAL(interface), tx_idx.u64);

	/* Get the window's current value */
	tx_cal0.u64 = csr_rd(CVMX_ILK_TXX_MEM_CAL0(interface));
	tx_cal1.u64 = csr_rd(CVMX_ILK_TXX_MEM_CAL1(interface));

	/* Force every sixteenth entry as link flow control state */
	if ((window & 1) == 0)
		tx_cal0.s.entry_ctl0 = LINK;

	/* Update the entry */
	switch (window_entry) {
	case 0:
		tx_cal0.s.entry_ctl0 = 0;
		tx_cal0.s.bpid0 = bpid;
		break;
	case 1:
		tx_cal0.s.entry_ctl1 = 0;
		tx_cal0.s.bpid1 = bpid;
		break;
	case 2:
		tx_cal0.s.entry_ctl2 = 0;
		tx_cal0.s.bpid2 = bpid;
		break;
	case 3:
		tx_cal0.s.entry_ctl3 = 0;
		tx_cal0.s.bpid3 = bpid;
		break;
	case 4:
		tx_cal1.s.entry_ctl4 = 0;
		tx_cal1.s.bpid4 = bpid;
		break;
	case 5:
		tx_cal1.s.entry_ctl5 = 0;
		tx_cal1.s.bpid5 = bpid;
		break;
	case 6:
		tx_cal1.s.entry_ctl6 = 0;
		tx_cal1.s.bpid6 = bpid;
		break;
	case 7:
		tx_cal1.s.entry_ctl7 = 0;
		tx_cal1.s.bpid7 = bpid;
		break;
	}

	/* Write the window new value */
	csr_wr(CVMX_ILK_TXX_MEM_CAL0(interface), tx_cal0.u64);
	csr_wr(CVMX_ILK_TXX_MEM_CAL1(interface), tx_cal1.u64);
}

void __cvmx_ilk_write_tx_cal_entry_cn78xx(int intf, int channel,
					  unsigned char bpid)
{
	cvmx_ilk_txx_cal_entryx_t tx_cal;
	int calender_16_block = channel / 15;
	int calender_16_index = channel % 15 + 1;
	int index = calender_16_block * 16 + calender_16_index;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	/* Program the link status on first channel */
	if (calender_16_index == 1) {
		tx_cal.u64 = 0;
		tx_cal.s.ctl = 1;
		csr_wr_node(node, CVMX_ILK_TXX_CAL_ENTRYX(index - 1, interface),
			    tx_cal.u64);
	}
	tx_cal.u64 = 0;
	tx_cal.s.ctl = 0;
	tx_cal.s.channel = channel;
	csr_wr_node(node, CVMX_ILK_TXX_CAL_ENTRYX(index, interface),
		    tx_cal.u64);
}

/**
 * @INTERNAL
 * Setup the channel's tx calendar entry.
 *
 * @param interface channel belongs to
 * @param channel whose calendar entry is to be updated
 * @param bpid assigned to the channel
 */
void __cvmx_ilk_write_tx_cal_entry(int interface, int channel,
				   unsigned char bpid)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		__cvmx_ilk_write_tx_cal_entry_cn68xx(interface, channel, bpid);
	else
		__cvmx_ilk_write_tx_cal_entry_cn78xx(interface, channel, bpid);
}

void __cvmx_ilk_write_rx_cal_entry_cn78xx(int intf, int channel,
					  unsigned char bpid)
{
	cvmx_ilk_rxx_cal_entryx_t rx_cal;
	int calender_16_block = channel / 15;
	int calender_16_index = channel % 15 + 1;
	int index = calender_16_block * 16 + calender_16_index;
	int node = (intf >> 4) & 0xf;
	int interface = intf & 0xf;

	/* Program the link status on first channel */
	if (calender_16_index == 1) {
		rx_cal.u64 = 0;
		rx_cal.s.ctl = 1;
		csr_wr_node(node, CVMX_ILK_RXX_CAL_ENTRYX(index - 1, interface),
			    rx_cal.u64);
	}
	rx_cal.u64 = 0;
	rx_cal.s.ctl = 0;
	rx_cal.s.channel = channel;
	csr_wr_node(node, CVMX_ILK_RXX_CAL_ENTRYX(index, interface),
		    rx_cal.u64);
}

void __cvmx_ilk_write_rx_cal_entry_cn68xx(int interface, int channel,
					  unsigned char pipe)
{
	cvmx_ilk_rxx_idx_cal_t rx_idx;
	cvmx_ilk_rxx_mem_cal0_t rx_cal0;
	cvmx_ilk_rxx_mem_cal1_t rx_cal1;
	int entry;
	int window;
	int window_entry;

	/*
	 * The calendar has 288 entries. Each calendar entry represents a
	 * channel's flow control state or the link flow control state.
	 * Starting with the first entry, every sixteenth entry is used for the
	 * link flow control state. The other 15 entries are used for the
	 * channels flow control state:
	 * entry 0   ----> link flow control state
	 * entry 1   ----> channel 0 flow control state
	 * entry 2   ----> channel 1 flow control state
	 * ...
	 * entry 15  ----> channel 14 flow control state
	 * entry 16  ----> link flow control state
	 * entry 17  ----> channel 15 flow control state
	 *
	 * Also, the calendar is accessed via windows into it. Each window maps
	 * to 8 entries.
	 */
	entry = 1 + channel + (channel / 15);
	window = entry / 8;
	window_entry = entry % 8;

	/* Indicate the window we need to access */
	rx_idx.u64 = 0;
	rx_idx.s.index = window;
	csr_wr(CVMX_ILK_RXX_IDX_CAL(interface), rx_idx.u64);

	/* Get the window's current value */
	rx_cal0.u64 = csr_rd(CVMX_ILK_RXX_MEM_CAL0(interface));
	rx_cal1.u64 = csr_rd(CVMX_ILK_RXX_MEM_CAL1(interface));

	/* Force every sixteenth entry as link flow control state */
	if ((window & 1) == 0)
		rx_cal0.s.entry_ctl0 = LINK;

	/* Update the entry */
	switch (window_entry) {
	case 0:
		rx_cal0.s.entry_ctl0 = 0;
		rx_cal0.s.port_pipe0 = pipe;
		break;
	case 1:
		rx_cal0.s.entry_ctl1 = 0;
		rx_cal0.s.port_pipe1 = pipe;
		break;
	case 2:
		rx_cal0.s.entry_ctl2 = 0;
		rx_cal0.s.port_pipe2 = pipe;
		break;
	case 3:
		rx_cal0.s.entry_ctl3 = 0;
		rx_cal0.s.port_pipe3 = pipe;
		break;
	case 4:
		rx_cal1.s.entry_ctl4 = 0;
		rx_cal1.s.port_pipe4 = pipe;
		break;
	case 5:
		rx_cal1.s.entry_ctl5 = 0;
		rx_cal1.s.port_pipe5 = pipe;
		break;
	case 6:
		rx_cal1.s.entry_ctl6 = 0;
		rx_cal1.s.port_pipe6 = pipe;
		break;
	case 7:
		rx_cal1.s.entry_ctl7 = 0;
		rx_cal1.s.port_pipe7 = pipe;
		break;
	}

	/* Write the window new value */
	csr_wr(CVMX_ILK_RXX_MEM_CAL0(interface), rx_cal0.u64);
	csr_wr(CVMX_ILK_RXX_MEM_CAL1(interface), rx_cal1.u64);
}

/**
 * @INTERNAL
 * Setup the channel's rx calendar entry.
 *
 * @param interface channel belongs to
 * @param channel whose calendar entry is to be updated
 * @param pipe PKO assigned to the channel
 */
void __cvmx_ilk_write_rx_cal_entry(int interface, int channel,
				   unsigned char pipe)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		__cvmx_ilk_write_rx_cal_entry_cn68xx(interface, channel, pipe);
	else
		__cvmx_ilk_write_rx_cal_entry_cn78xx(interface, channel, pipe);
}

/**
 * @INTERNAL
 * Probe a ILK interface and determine the number of ports
 * connected to it. The ILK interface should still be down
 * after this call.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_ilk_probe(int xiface)
{
	int res = 0;
	int interface;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!octeon_has_feature(OCTEON_FEATURE_ILK))
		return res;

	interface = xi.interface - CVMX_ILK_GBL_BASE();
	if (interface >= CVMX_NUM_ILK_INTF)
		return 0;

	/* the configuration should be done only once */
	if (cvmx_ilk_get_intf_ena(xiface))
		return cvmx_ilk_chans[xi.node][interface];

	/* configure lanes and enable the link */
	res = cvmx_ilk_start_interface(((xi.node << 4) | interface),
				       cvmx_ilk_lane_mask[xi.node][interface]);
	if (res < 0)
		return 0;

	res = __cvmx_helper_ilk_enumerate(xiface);

	return res;
}

static int __cvmx_helper_ilk_init_port_cn68xx(int xiface)
{
	int i, j, res = -1;
	static int pipe_base = 0, pknd_base;
	static cvmx_ilk_pipe_chan_t *pch = NULL, *tmp;
	static cvmx_ilk_chan_pknd_t *chpknd = NULL, *tmp1;
	static cvmx_ilk_cal_entry_t *calent = NULL, *tmp2;
	int enable_rx_cal = 1;
	int interface;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int intf;
	int num_chans;

	interface = xi.interface - CVMX_ILK_GBL_BASE();
	intf = (xi.node << 4) | interface;
	if (interface >= CVMX_NUM_ILK_INTF)
		return 0;

	num_chans = cvmx_ilk_chans[0][interface];

	/* set up channel to pkind mapping */
	if (pknd_base == 0)
		pknd_base = cvmx_helper_get_pknd(xiface, 0);

	/* set up the group of pipes available to ilk */
	if (pipe_base == 0)
		pipe_base =
			__cvmx_pko_get_pipe(interface + CVMX_ILK_GBL_BASE(), 0);

	if (pipe_base == -1) {
		pipe_base = 0;
		return 0;
	}

	res = cvmx_ilk_set_pipe(xiface, pipe_base,
				cvmx_ilk_chans[0][interface]);
	if (res < 0)
		return 0;

	/* set up pipe to channel mapping */
	i = pipe_base;
	if (!pch) {
		pch = (cvmx_ilk_pipe_chan_t *)cvmx_bootmem_alloc(
			num_chans * sizeof(cvmx_ilk_pipe_chan_t),
			sizeof(cvmx_ilk_pipe_chan_t));
		if (!pch)
			return 0;
	}

	memset(pch, 0, num_chans * sizeof(cvmx_ilk_pipe_chan_t));
	tmp = pch;
	for (j = 0; j < num_chans; j++) {
		tmp->pipe = i++;
		tmp->chan = j;
		tmp++;
	}
	res = cvmx_ilk_tx_set_channel(interface, pch,
				      cvmx_ilk_chans[0][interface]);
	if (res < 0) {
		res = 0;
		goto err_free_pch;
	}
	pipe_base += cvmx_ilk_chans[0][interface];
	i = pknd_base;
	if (!chpknd) {
		chpknd = (cvmx_ilk_chan_pknd_t *)cvmx_bootmem_alloc(
			CVMX_ILK_MAX_PKNDS * sizeof(cvmx_ilk_chan_pknd_t),
			sizeof(cvmx_ilk_chan_pknd_t));
		if (!chpknd) {
			pipe_base -= cvmx_ilk_chans[xi.node][interface];
			res = 0;
			goto err_free_pch;
		}
	}

	memset(chpknd, 0, CVMX_ILK_MAX_PKNDS * sizeof(cvmx_ilk_chan_pknd_t));
	tmp1 = chpknd;
	for (j = 0; j < cvmx_ilk_chans[xi.node][interface]; j++) {
		tmp1->chan = j;
		tmp1->pknd = i++;
		tmp1++;
	}

	res = cvmx_ilk_rx_set_pknd(xiface, chpknd,
				   cvmx_ilk_chans[xi.node][interface]);
	if (res < 0) {
		pipe_base -= cvmx_ilk_chans[xi.node][interface];
		res = 0;
		goto err_free_chpknd;
	}
	pknd_base += cvmx_ilk_chans[xi.node][interface];

	/* Set up tx calendar */
	if (!calent) {
		calent = (cvmx_ilk_cal_entry_t *)cvmx_bootmem_alloc(
			CVMX_ILK_MAX_PIPES * sizeof(cvmx_ilk_cal_entry_t),
			sizeof(cvmx_ilk_cal_entry_t));
		if (!calent) {
			pipe_base -= cvmx_ilk_chans[xi.node][interface];
			pknd_base -= cvmx_ilk_chans[xi.node][interface];
			res = 0;
			goto err_free_chpknd;
		}
	}

	memset(calent, 0, CVMX_ILK_MAX_PIPES * sizeof(cvmx_ilk_cal_entry_t));
	tmp1 = chpknd;
	tmp2 = calent;
	for (j = 0; j < cvmx_ilk_chans[xi.node][interface]; j++) {
		tmp2->pipe_bpid = tmp1->pknd;
		tmp2->ent_ctrl = PIPE_BPID;
		tmp1++;
		tmp2++;
	}
	res = cvmx_ilk_cal_setup_tx(intf, cvmx_ilk_chans[xi.node][interface],
				    calent, 1);
	if (res < 0) {
		pipe_base -= cvmx_ilk_chans[xi.node][interface];
		pknd_base -= cvmx_ilk_chans[xi.node][interface];
		res = 0;
		goto err_free_calent;
	}

	/* set up rx calendar. allocated memory can be reused.
	 * this is because max pkind is always less than max pipe
	 */
	memset(calent, 0, CVMX_ILK_MAX_PIPES * sizeof(cvmx_ilk_cal_entry_t));
	tmp = pch;
	tmp2 = calent;
	for (j = 0; j < cvmx_ilk_chans[0][interface]; j++) {
		tmp2->pipe_bpid = tmp->pipe;
		tmp2->ent_ctrl = PIPE_BPID;
		tmp++;
		tmp2++;
	}
	if (cvmx_ilk_use_la_mode(interface, 0))
		enable_rx_cal = cvmx_ilk_la_mode_enable_rx_calendar(interface);
	else
		enable_rx_cal = 1;

	res = cvmx_ilk_cal_setup_rx(intf, cvmx_ilk_chans[xi.node][interface],
				    calent, CVMX_ILK_RX_FIFO_WM, enable_rx_cal);
	if (res < 0) {
		pipe_base -= cvmx_ilk_chans[xi.node][interface];
		pknd_base -= cvmx_ilk_chans[xi.node][interface];
		res = 0;
		goto err_free_calent;
	}
	goto out;

err_free_calent:
	/* no free() for cvmx_bootmem_alloc() */

err_free_chpknd:
	/* no free() for cvmx_bootmem_alloc() */

err_free_pch:
	/* no free() for cvmx_bootmem_alloc() */
out:
	return res;
}

static int __cvmx_helper_ilk_init_port_cn78xx(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface;
	int intf;

	interface = xi.interface - CVMX_ILK_GBL_BASE();
	intf = (xi.node << 4) | interface;
	if (interface >= CVMX_NUM_ILK_INTF)
		return 0;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		struct cvmx_pki_style_config style_cfg;
		int num_channels = cvmx_ilk_chans[xi.node][interface];
		int index, i;

		for (i = 0; i < num_channels; i++) {
			int pknd;

			index = (i % 8);

			/* Set jabber to allow max sized packets */
			if (i == 0)
				csr_wr_node(xi.node,
					    CVMX_ILK_RXX_JABBER(interface),
					    0xfff8);

			/* Setup PKND */
			pknd = cvmx_helper_get_pknd(xiface, index);
			csr_wr_node(xi.node, CVMX_ILK_RXX_CHAX(i, interface),
				    pknd);
			cvmx_pki_read_style_config(
				0, pknd, CVMX_PKI_CLUSTER_ALL, &style_cfg);
			style_cfg.parm_cfg.qpg_port_sh = 0;
			/* 256 channels */
			style_cfg.parm_cfg.qpg_port_msb = 8;
			cvmx_pki_write_style_config(
				0, pknd, CVMX_PKI_CLUSTER_ALL, &style_cfg);
		}

		cvmx_ilk_cal_setup_tx(intf, num_channels, NULL, 1);
		cvmx_ilk_cal_setup_rx(intf, num_channels, NULL,
				      CVMX_ILK_RX_FIFO_WM, 1);
	}
	return 0;
}

static int __cvmx_helper_ilk_init_port(int xiface)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return __cvmx_helper_ilk_init_port_cn68xx(xiface);
	else
		return __cvmx_helper_ilk_init_port_cn78xx(xiface);
}

/**
 * @INTERNAL
 * Bringup and enable ILK interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_ilk_enable(int xiface)
{
	if (__cvmx_helper_ilk_init_port(xiface) < 0)
		return -1;

	return cvmx_ilk_enable(xiface);
}

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by ILK link status.
 *
 * @param ipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t __cvmx_helper_ilk_link_get(int ipd_port)
{
	cvmx_helper_link_info_t result;
	int xiface = cvmx_helper_get_interface_num(ipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface;
	int retry_count = 0;
	cvmx_ilk_rxx_cfg1_t ilk_rxx_cfg1;
	cvmx_ilk_rxx_int_t ilk_rxx_int;
	int lane_mask = 0;
	int i;
	int node = xi.node;

	result.u64 = 0;
	interface = xi.interface - CVMX_ILK_GBL_BASE();

retry:
	retry_count++;
	if (retry_count > 200)
		goto fail;

	/* Read RX config and status bits */
	ilk_rxx_cfg1.u64 = csr_rd_node(node, CVMX_ILK_RXX_CFG1(interface));
	ilk_rxx_int.u64 = csr_rd_node(node, CVMX_ILK_RXX_INT(interface));

	if (ilk_rxx_cfg1.s.rx_bdry_lock_ena == 0) {
		/* (GSER-21957) GSER RX Equalization may make >= 5gbaud non-KR
		 * channel better
		 */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			int qlm, lane_mask;

			for (qlm = 4; qlm < 8; qlm++) {
				lane_mask = 1 << (qlm - 4) * 4;
				if (lane_mask &
				    cvmx_ilk_lane_mask[node][interface]) {
					if (__cvmx_qlm_rx_equalization(
						    node, qlm, -1))
						goto retry;
				}
			}
		}

		/* Clear the boundary lock status bit */
		ilk_rxx_int.u64 = 0;
		ilk_rxx_int.s.word_sync_done = 1;
		csr_wr_node(node, CVMX_ILK_RXX_INT(interface), ilk_rxx_int.u64);

		/* We need to start looking for word boundary lock */
		ilk_rxx_cfg1.s.rx_bdry_lock_ena =
			cvmx_ilk_lane_mask[node][interface];
		ilk_rxx_cfg1.s.rx_align_ena = 0;
		csr_wr_node(node, CVMX_ILK_RXX_CFG1(interface),
			    ilk_rxx_cfg1.u64);
		//debug("ILK%d: Looking for word boundary lock\n", interface);
		udelay(50);
		goto retry;
	}

	if (ilk_rxx_cfg1.s.rx_align_ena == 0) {
		if (ilk_rxx_int.s.word_sync_done) {
			/* Clear the lane align status bits */
			ilk_rxx_int.u64 = 0;
			ilk_rxx_int.s.lane_align_fail = 1;
			ilk_rxx_int.s.lane_align_done = 1;
			csr_wr_node(node, CVMX_ILK_RXX_INT(interface),
				    ilk_rxx_int.u64);

			ilk_rxx_cfg1.s.rx_align_ena = 1;
			csr_wr_node(node, CVMX_ILK_RXX_CFG1(interface),
				    ilk_rxx_cfg1.u64);
			//printf("ILK%d: Looking for lane alignment\n", interface);
		}
		udelay(50);
		goto retry;
	}

	if (ilk_rxx_int.s.lane_align_fail) {
		ilk_rxx_cfg1.s.rx_bdry_lock_ena = 0;
		ilk_rxx_cfg1.s.rx_align_ena = 0;
		csr_wr_node(node, CVMX_ILK_RXX_CFG1(interface),
			    ilk_rxx_cfg1.u64);
		//debug("ILK%d: Lane alignment failed\n", interface);
		goto fail;
	}

	lane_mask = ilk_rxx_cfg1.s.rx_bdry_lock_ena;

	if (ilk_rxx_cfg1.s.pkt_ena == 0 && ilk_rxx_int.s.lane_align_done) {
		cvmx_ilk_txx_cfg1_t ilk_txx_cfg1;

		ilk_txx_cfg1.u64 =
			csr_rd_node(node, CVMX_ILK_TXX_CFG1(interface));
		ilk_rxx_cfg1.u64 =
			csr_rd_node(node, CVMX_ILK_RXX_CFG1(interface));
		ilk_rxx_cfg1.s.pkt_ena = ilk_txx_cfg1.s.pkt_ena;
		csr_wr_node(node, CVMX_ILK_RXX_CFG1(interface),
			    ilk_rxx_cfg1.u64);

		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			/*
			 * Enable rxf_ctl_perr, rxf_lnk0_perr, rxf_lnk1_perr,
			 * pop_empty, push_full.
			 */
			csr_wr(CVMX_ILK_GBL_INT_EN, 0x1f);
			/* Enable bad_pipe, bad_seq, txf_err */
			csr_wr(CVMX_ILK_TXX_INT_EN(interface), 0x7);

			/*
			 * Enable crc24_err, lane_bad_word,
			 * pkt_drop_{rid,rxf,sop}
			 */
			csr_wr(CVMX_ILK_RXX_INT_EN(interface), 0x1e2);
		}
		/* Need to enable ILK interrupts for 78xx */

		for (i = 0; i < CVMX_ILK_MAX_LANES(); i++) {
			if ((1 << i) & lane_mask) {
				/* clear pending interrupts, before enabling. */
				csr_wr_node(node, CVMX_ILK_RX_LNEX_INT(i),
					    0x1ff);
				/* Enable bad_64b67b, bdry_sync_loss, crc32_err,
				 * dskew_fifo_ovfl, scrm_sync_loss,
				 * serdes_lock_loss, stat_msg, ukwn_cntl_word
				 */
				if (OCTEON_IS_MODEL(OCTEON_CN68XX))
					csr_wr(CVMX_ILK_RX_LNEX_INT_EN(i),
					       0x1ff);
			}
		}

		//debug("ILK%d: Lane alignment complete\n", interface);
	}

	/* Enable error interrupts, now link is up */
	cvmx_error_enable_group(CVMX_ERROR_GROUP_ILK,
				node | (interface << 2) | (lane_mask << 4));

	result.s.link_up = 1;
	result.s.full_duplex = 1;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		int qlm = cvmx_qlm_lmac(xiface, 0);

		result.s.speed = cvmx_qlm_get_gbaud_mhz(qlm) * 64 / 67;
	} else {
		result.s.speed =
			cvmx_qlm_get_gbaud_mhz(1 + interface) * 64 / 67;
	}
	result.s.speed *= cvmx_pop(lane_mask);

	return result;

fail:
	if (ilk_rxx_cfg1.s.pkt_ena) {
		/* Disable the interface */
		ilk_rxx_cfg1.s.pkt_ena = 0;
		csr_wr_node(node, CVMX_ILK_RXX_CFG1(interface),
			    ilk_rxx_cfg1.u64);

		/* Disable error interrupts */
		for (i = 0; i < CVMX_ILK_MAX_LANES(); i++) {
			/* Disable bad_64b67b, bdry_sync_loss, crc32_err,
			 * dskew_fifo_ovfl, scrm_sync_loss, serdes_lock_loss,
			 * stat_msg, ukwn_cntl_word
			 */
			if ((1 << i) & lane_mask) {
				csr_wr_node(node, CVMX_ILK_RX_LNEX_INT(i),
					    0x1ff);
				if (OCTEON_IS_MODEL(OCTEON_CN68XX))
					csr_wr(CVMX_ILK_RX_LNEX_INT_EN(i),
					       ~0x1ff);
			}
		}
		/* Disable error interrupts */
		cvmx_error_enable_group(CVMX_ERROR_GROUP_ILK, 0);
	}

	return result;
}

/**
 * @INTERNAL
 * Set the link state of an IPD/PKO port.
 *
 * @param ipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_ilk_link_set(int ipd_port, cvmx_helper_link_info_t link_info)
{
	/* Do nothing */
	return 0;
}
