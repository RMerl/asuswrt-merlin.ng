// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * PKOv3 helper file
 */

#include <errno.h>
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
#include <mach/cvmx-range.h>
#include <mach/cvmx-global-resources.h>

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
#include <mach/cvmx-ipd.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pko3.h>
#include <mach/cvmx-pko3-queue.h>
#include <mach/cvmx-pko3-resources.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

/* channels are present at L2 queue level by default */
static const enum cvmx_pko3_level_e cvmx_pko_default_channel_level =
	CVMX_PKO_L2_QUEUES;

static const int debug;

static int __pko_pkt_budget, __pko_pkt_quota;

/* These global variables are relevant for boot CPU only */
static cvmx_fpa3_gaura_t __cvmx_pko3_aura[CVMX_MAX_NODES];

/* This constant can not be modified, defined here for clarity only */
#define CVMX_PKO3_POOL_BUFFER_SIZE 4096 /* 78XX PKO requires 4KB */

/**
 * @INTERNAL
 *
 * Build an owner tag based on interface/port
 */
static int __cvmx_helper_pko3_res_owner(int ipd_port)
{
	int res_owner;
	const int res_owner_pfix = 0x19d0 << 14;

	ipd_port &= 0x3fff; /* 12-bit for local CHAN_E value + node */

	res_owner = res_owner_pfix | ipd_port;

	return res_owner;
}

/**
 * Configure an AURA/POOL designated for PKO internal use.
 *
 * This pool is used for (a) memory buffers that store PKO descriptor queues,
 * (b) buffers for use with PKO_SEND_JUMP_S sub-header.
 *
 * The buffers of type (a) are never accessed by software, and their number
 * should be at least equal to 4 times the number of descriptor queues
 * in use.
 *
 * Type (b) buffers are consumed by PKO3 command-composition code,
 * and are released by the hardware upon completion of transmission.
 *
 * @returns -1 if the pool could not be established or 12-bit AURA
 * that includes the node number for use in PKO3 initialization call.
 *
 * NOTE: Linux kernel should pass its own aura to PKO3 initialization
 * function so that the buffers can be mapped into kernel space
 * for when software needs to adccess their contents.
 *
 */
static int __cvmx_pko3_config_memory(unsigned int node)
{
	cvmx_fpa3_gaura_t aura;
	int aura_num;
	unsigned int buf_count;
	bool small_mem;
	int i, num_intf = 0;
	const unsigned int pkt_per_buf =
		(CVMX_PKO3_POOL_BUFFER_SIZE / sizeof(u64) / 16);
	const unsigned int base_buf_count = 1024 * 4;

	/* Simulator has limited memory, but uses one interface at a time */
	//	small_mem = cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM;
	small_mem = false;

	/* Count the number of live interfaces */
	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		int xiface = cvmx_helper_node_interface_to_xiface(node, i);

		if (CVMX_HELPER_INTERFACE_MODE_DISABLED !=
		    cvmx_helper_interface_get_mode(xiface))
			num_intf++;
	}

	buf_count = 1024;
	__pko_pkt_quota = buf_count * pkt_per_buf;
	__pko_pkt_budget = __pko_pkt_quota * num_intf;
	(void)small_mem;
	(void)base_buf_count;

	if (debug)
		debug("%s: Creating AURA with %u buffers for up to %d total packets, %d packets per interface\n",
		      __func__, buf_count, __pko_pkt_budget, __pko_pkt_quota);

	aura = cvmx_fpa3_setup_aura_and_pool(node, -1, "PKO3 AURA", NULL,
					     CVMX_PKO3_POOL_BUFFER_SIZE,
					     buf_count);

	if (!__cvmx_fpa3_aura_valid(aura)) {
		printf("ERROR: %s AURA create failed\n", __func__);
		return -1;
	}

	aura_num = aura.node << 10 | aura.laura;

	/* Store handle for destruction */
	__cvmx_pko3_aura[node] = aura;

	return aura_num;
}

/** Initialize a channelized port
 * This is intended for LOOP, ILK and NPI interfaces which have one MAC
 * per interface and need a channel per subinterface (e.g. ring).
 * Each channel then may have 'num_queues' descriptor queues
 * attached to it, which can also be prioritized or fair.
 */
static int __cvmx_pko3_config_chan_interface(int xiface, unsigned int num_chans,
					     u8 num_queues, bool prioritized)
{
	int l1_q_num;
	int l2_q_base;
	enum cvmx_pko3_level_e level;
	int res;
	int parent_q, child_q;
	unsigned int chan, dq;
	int pko_mac_num;
	u16 ipd_port;
	int res_owner, prio;
	unsigned int i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned int node = xi.node;
	char b1[12];

	if (num_queues == 0)
		num_queues = 1;
	if ((cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES) / num_chans) < 3)
		num_queues = 1;

	if (prioritized && num_queues > 1)
		prio = num_queues;
	else
		prio = -1;

	if (debug)
		debug("%s: configuring xiface %u:%u with %u chans %u queues each\n",
		      __func__, xi.node, xi.interface, num_chans, num_queues);

	/* all channels all go to the same mac */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, 0);
	if (pko_mac_num < 0) {
		printf("ERROR: %s: Invalid interface\n", __func__);
		return -1;
	}

	/* Resources of all channels on this port have common owner */
	ipd_port = cvmx_helper_get_ipd_port(xiface, 0);

	/* Build an identifiable owner */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	/* Start configuration at L1/PQ */
	level = CVMX_PKO_PORT_QUEUES;

	/* Reserve port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
		printf("ERROR: %s: Reserving L1 PQ\n", __func__);
		return -1;
	}

	res = cvmx_pko3_pq_config(node, pko_mac_num, l1_q_num);
	if (res < 0) {
		printf("ERROR: %s: Configuring L1 PQ\n", __func__);
		return -1;
	}

	/* next queue level = L2/SQ */
	level = __cvmx_pko3_sq_lvl_next(level);

	/* allocate level 2 queues, one per channel */
	l2_q_base =
		cvmx_pko_alloc_queues(node, level, res_owner, -1, num_chans);
	if (l2_q_base < 0) {
		printf("ERROR: %s: allocation L2 SQ\n", __func__);
		return -1;
	}

	/* Configre <num_chans> L2 children for PQ, non-prioritized */
	res = cvmx_pko3_sq_config_children(node, level, l1_q_num, l2_q_base,
					   num_chans, -1);

	if (res < 0) {
		printf("ERROR: %s: Failed channel queues\n", __func__);
		return -1;
	}

	/* map channels to l2 queues */
	for (chan = 0; chan < num_chans; chan++) {
		ipd_port = cvmx_helper_get_ipd_port(xiface, chan);
		cvmx_pko3_map_channel(node, l1_q_num, l2_q_base + chan,
				      ipd_port);
	}

	/* next queue level = L3/SQ */
	level = __cvmx_pko3_sq_lvl_next(level);
	parent_q = l2_q_base;

	do {
		child_q = cvmx_pko_alloc_queues(node, level, res_owner, -1,
						num_chans);

		if (child_q < 0) {
			printf("ERROR: %s: allocating %s\n", __func__,
			       __cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		for (i = 0; i < num_chans; i++) {
			res = cvmx_pko3_sq_config_children(
				node, level, parent_q + i, child_q + i, 1, 1);

			if (res < 0) {
				printf("ERROR: %s: configuring %s\n", __func__,
				       __cvmx_pko3_sq_str(b1, level, child_q));
				return -1;
			}

		} /* for i */

		parent_q = child_q;
		level = __cvmx_pko3_sq_lvl_next(level);

		/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		 level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	/* Configure DQs, num_dqs per chan */
	for (chan = 0; chan < num_chans; chan++) {
		res = cvmx_pko_alloc_queues(node, level, res_owner, -1,
					    num_queues);

		if (res < 0)
			goto _fail;
		dq = res;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0) && (dq & 7))
			debug("WARNING: %s: DQ# %u not integral of 8\n",
			      __func__, dq);

		res = cvmx_pko3_sq_config_children(node, level, parent_q + chan,
						   dq, num_queues, prio);
		if (res < 0)
			goto _fail;

		/* register DQ range with the translation table */
		res = __cvmx_pko3_ipd_dq_register(xiface, chan, dq, num_queues);
		if (res < 0)
			goto _fail;
	}

	return 0;
_fail:
	debug("ERROR: %s: configuring queues for xiface %u:%u chan %u\n",
	      __func__, xi.node, xi.interface, i);
	return -1;
}

/** Initialize a single Ethernet port with PFC-style channels
 *
 * One interface can contain multiple ports, this function is per-port
 * Here, a physical port is allocated 8 logical channel, one per VLAN
 * tag priority, one DQ is assigned to each channel, and all 8 DQs
 * are registered for that IPD port.
 * Note that the DQs are arrange such that the Ethernet QoS/PCP field
 * can be used as an offset to the value returned by cvmx_pko_base_queue_get().
 *
 * For HighGig2 mode, 16 channels may be desired, instead of 8,
 * but this function does not support that.
 */
static int __cvmx_pko3_config_pfc_interface(int xiface, unsigned int port)
{
	enum cvmx_pko3_level_e level;
	int pko_mac_num;
	int l1_q_num, l2_q_base;
	int child_q, parent_q;
	int dq_base;
	int res;
	const unsigned int num_chans = 8;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned int node = xi.node;
	u16 ipd_port;
	int res_owner;
	char b1[12];
	unsigned int i;

	if (debug)
		debug("%s: configuring xiface %u:%u port %u with %u PFC channels\n",
		      __func__, node, xi.interface, port, num_chans);

	/* Get MAC number for the iface/port */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, port);
	if (pko_mac_num < 0) {
		printf("ERROR: %s: Invalid interface\n", __func__);
		return -1;
	}

	ipd_port = cvmx_helper_get_ipd_port(xiface, port);

	/* Build an identifiable owner identifier */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	level = CVMX_PKO_PORT_QUEUES;

	/* Allocate port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
		printf("ERROR: %s: allocation L1 PQ\n", __func__);
		return -1;
	}

	res = cvmx_pko3_pq_config(xi.node, pko_mac_num, l1_q_num);
	if (res < 0) {
		printf("ERROR: %s: Configuring %s\n", __func__,
		       __cvmx_pko3_sq_str(b1, level, l1_q_num));
		return -1;
	}

	/* Determine the next queue level */
	level = __cvmx_pko3_sq_lvl_next(level);

	/* Allocate 'num_chans' L2 queues, one per channel */
	l2_q_base =
		cvmx_pko_alloc_queues(node, level, res_owner, -1, num_chans);
	if (l2_q_base < 0) {
		printf("ERROR: %s: allocation L2 SQ\n", __func__);
		return -1;
	}

	/* Configre <num_chans> L2 children for PQ, with static priority */
	res = cvmx_pko3_sq_config_children(node, level, l1_q_num, l2_q_base,
					   num_chans, num_chans);

	if (res < 0) {
		printf("ERROR: %s: Configuring %s for PFC\n", __func__,
		       __cvmx_pko3_sq_str(b1, level, l1_q_num));
		return -1;
	}

	/* Map each of the allocated channels */
	for (i = 0; i < num_chans; i++) {
		u16 chan;

		/* Get CHAN_E value for this PFC channel, PCP in low 3 bits */
		chan = ipd_port | cvmx_helper_prio2qos(i);

		cvmx_pko3_map_channel(node, l1_q_num, l2_q_base + i, chan);
	}

	/* Iterate through the levels until DQ and allocate 'num_chans'
	 * consecutive queues at each level and hook them up
	 * one-to-one with the parent level queues
	 */

	parent_q = l2_q_base;
	level = __cvmx_pko3_sq_lvl_next(level);

	do {
		child_q = cvmx_pko_alloc_queues(node, level, res_owner, -1,
						num_chans);

		if (child_q < 0) {
			printf("ERROR: %s: allocating %s\n", __func__,
			       __cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		for (i = 0; i < num_chans; i++) {
			res = cvmx_pko3_sq_config_children(
				node, level, parent_q + i, child_q + i, 1, 1);

			if (res < 0) {
				printf("ERROR: %s: configuring %s\n", __func__,
				       __cvmx_pko3_sq_str(b1, level, child_q));
				return -1;
			}

		} /* for i */

		parent_q = child_q;
		level = __cvmx_pko3_sq_lvl_next(level);

		/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		 level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	dq_base = cvmx_pko_alloc_queues(node, level, res_owner, -1, num_chans);
	if (dq_base < 0) {
		printf("ERROR: %s: allocating %s\n", __func__,
		       __cvmx_pko3_sq_str(b1, level, dq_base));
		return -1;
	}

	/* Configure DQs in QoS order, so that QoS/PCP can be index */
	for (i = 0; i < num_chans; i++) {
		int dq_num = dq_base + cvmx_helper_prio2qos(i);

		res = cvmx_pko3_sq_config_children(node, level, parent_q + i,
						   dq_num, 1, 1);
		if (res < 0) {
			printf("ERROR: %s: configuring %s\n", __func__,
			       __cvmx_pko3_sq_str(b1, level, dq_num));
			return -1;
		}
	}

	/* register entire DQ range with the IPD translation table */
	__cvmx_pko3_ipd_dq_register(xiface, port, dq_base, num_chans);

	return 0;
}

/**
 * Initialize a simple interface with a a given number of
 * fair or prioritized queues.
 * This function will assign one channel per sub-interface.
 */
int __cvmx_pko3_config_gen_interface(int xiface, uint8_t subif, u8 num_queues,
				     bool prioritized)
{
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);
	u8 node = xi.node;
	int l1_q_num;
	int parent_q, child_q;
	int dq;
	int res, res_owner;
	int pko_mac_num;
	enum cvmx_pko3_level_e level;
	u16 ipd_port;
	int static_pri;
	char b1[12];

	num_queues = 1;

	if (num_queues == 0) {
		num_queues = 1;
		printf("WARNING: %s: xiface %#x misconfigured\n", __func__,
		       xiface);
	}

	/* Configure DQs relative priority (a.k.a. scheduling) */
	if (prioritized) {
		/* With 8 queues or fewer, use static priority, else WRR */
		static_pri = (num_queues < 9) ? num_queues : 0;
	} else {
		/* Set equal-RR scheduling among queues */
		static_pri = -1;
	}

	if (debug)
		debug("%s: configuring xiface %u:%u/%u nq=%u %s\n", __func__,
		      xi.node, xi.interface, subif, num_queues,
		      (prioritized) ? "qos" : "fair");

	/* Get MAC number for the iface/port */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, subif);
	if (pko_mac_num < 0) {
		printf("ERROR: %s: Invalid interface %u:%u\n", __func__,
		       xi.node, xi.interface);
		return -1;
	}

	ipd_port = cvmx_helper_get_ipd_port(xiface, subif);

	if (debug)
		debug("%s: xiface %u:%u/%u ipd_port=%#03x\n", __func__, xi.node,
		      xi.interface, subif, ipd_port);

	/* Build an identifiable owner identifier */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	level = CVMX_PKO_PORT_QUEUES;

	/* Reserve port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
		printf("ERROR %s: xiface %u:%u/%u failed allocation L1 PQ\n",
		       __func__, xi.node, xi.interface, subif);
		return -1;
	}

	res = cvmx_pko3_pq_config(node, pko_mac_num, l1_q_num);
	if (res < 0) {
		printf("ERROR %s: Configuring L1 PQ\n", __func__);
		return -1;
	}

	parent_q = l1_q_num;

	/* Determine the next queue level */
	level = __cvmx_pko3_sq_lvl_next(level);

	/* Simply chain queues 1-to-1 from L2 to one before DQ level */
	do {
		/* allocate next level queue */
		child_q = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

		if (child_q < 0) {
			printf("ERROR: %s: allocating %s\n", __func__,
			       __cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* Configre newly allocated queue */
		res = cvmx_pko3_sq_config_children(node, level, parent_q,
						   child_q, 1, 1);

		if (res < 0) {
			printf("ERROR: %s: configuring %s\n", __func__,
			       __cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* map IPD/channel to L2/L3 queues */
		if (level == cvmx_pko_default_channel_level)
			cvmx_pko3_map_channel(node, l1_q_num, child_q,
					      ipd_port);

		/* Prepare for next level */
		level = __cvmx_pko3_sq_lvl_next(level);
		parent_q = child_q;

		/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		 level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	/* Allocate descriptor queues for the port */
	dq = cvmx_pko_alloc_queues(node, level, res_owner, -1, num_queues);
	if (dq < 0) {
		printf("ERROR: %s: could not reserve DQs\n", __func__);
		return -1;
	}

	res = cvmx_pko3_sq_config_children(node, level, parent_q, dq,
					   num_queues, static_pri);
	if (res < 0) {
		printf("ERROR: %s: configuring %s\n", __func__,
		       __cvmx_pko3_sq_str(b1, level, dq));
		return -1;
	}

	/* register DQ/IPD translation */
	__cvmx_pko3_ipd_dq_register(xiface, subif, dq, num_queues);

	if (debug)
		debug("%s: xiface %u:%u/%u qs %u-%u\n", __func__, xi.node,
		      xi.interface, subif, dq, dq + num_queues - 1);
	return 0;
}

/** Initialize the NULL interface
 *
 * A NULL interface is a special case in that it is not
 * one of the enumerated interfaces in the system, and does
 * not apply to input either. Still, it can be very handy
 * for dealing with packets that should be discarded in
 * a generic, streamlined way.
 *
 * The Descriptor Queue 0 will be reserved for the NULL interface
 * and the normalized (i.e. IPD) port number has the all-ones value.
 */
static int __cvmx_pko3_config_null_interface(unsigned int node)
{
	int l1_q_num;
	int parent_q, child_q;
	enum cvmx_pko3_level_e level;
	int i, res, res_owner;
	int xiface, ipd_port;
	int num_dq = 1;	  /* # of DQs for NULL */
	const int dq = 0; /* Reserve DQ#0 for NULL */
	char pko_mac_num;
	char b1[12];

	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		pko_mac_num = 0x1C; /* MAC# 28 virtual MAC for NULL */
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		pko_mac_num = 0x0F; /* MAC# 16 virtual MAC for NULL */
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		pko_mac_num = 0x0A; /* MAC# 10 virtual MAC for NULL */
	else
		return -1;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
		num_dq = 8;

	if (debug)
		debug("%s: null iface dq=%u-%u\n", __func__, dq,
		      dq + num_dq - 1);

	ipd_port = cvmx_helper_node_to_ipd_port(node, CVMX_PKO3_IPD_PORT_NULL);

	/* Build an identifiable owner identifier by MAC# for easy release */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);
	if (res_owner < 0) {
		debug("%s: ERROR Invalid interface\n", __func__);
		return -1;
	}

	level = CVMX_PKO_PORT_QUEUES;

	/* Allocate a port queue */
	l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
		debug("%s: ERROR reserving L1 SQ\n", __func__);
		return -1;
	}

	res = cvmx_pko3_pq_config(node, pko_mac_num, l1_q_num);
	if (res < 0) {
		printf("ERROR: %s: PQ/L1 queue configuration\n", __func__);
		return -1;
	}

	parent_q = l1_q_num;

	/* Determine the next queue level */
	level = __cvmx_pko3_sq_lvl_next(level);

	/* Simply chain queues 1-to-1 from L2 to one before DQ level */
	do {
		/* allocate next level queue */
		child_q = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

		if (child_q < 0) {
			printf("ERROR: %s: allocating %s\n", __func__,
			       __cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* Configre newly allocated queue */
		res = cvmx_pko3_sq_config_children(node, level, parent_q,
						   child_q, 1, 1);

		if (res < 0) {
			printf("ERROR: %s: configuring %s\n", __func__,
			       __cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* Prepare for next level */
		level = __cvmx_pko3_sq_lvl_next(level);
		parent_q = child_q;

		/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		 level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	/* Reserve 'num_dq' DQ's at 0 by convention */
	res = cvmx_pko_alloc_queues(node, level, res_owner, dq, num_dq);
	if (dq != res) {
		debug("%s: ERROR: could not reserve DQs\n", __func__);
		return -1;
	}

	res = cvmx_pko3_sq_config_children(node, level, parent_q, dq, num_dq,
					   num_dq);
	if (res < 0) {
		printf("ERROR: %s: configuring %s\n", __func__,
		       __cvmx_pko3_sq_str(b1, level, dq));
		return -1;
	}

	/* NULL interface does not need to map to a CHAN_E */

	/* register DQ/IPD translation */
	xiface = cvmx_helper_node_interface_to_xiface(node, __CVMX_XIFACE_NULL);
	__cvmx_pko3_ipd_dq_register(xiface, 0, dq, num_dq);

	/* open the null DQs here */
	for (i = 0; i < num_dq; i++) {
		unsigned int limit = 128; /* NULL never really uses much */

		cvmx_pko_dq_open(node, dq + i);
		cvmx_pko3_dq_set_limit(node, dq + i, limit);
	}

	return 0;
}

/** Open all descriptor queues belonging to an interface/port
 * @INTERNAL
 */
int __cvmx_pko3_helper_dqs_activate(int xiface, int index, bool min_pad)
{
	int ipd_port, dq_base, dq_count, i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned int limit;

	/* Get local IPD port for the interface */
	ipd_port = cvmx_helper_get_ipd_port(xiface, index);
	if (ipd_port < 0) {
		printf("ERROR: %s: No IPD port for interface %d port %d\n",
		       __func__, xiface, index);
		return -1;
	}

	/* Get DQ# range for the IPD port */
	dq_base = cvmx_pko3_get_queue_base(ipd_port);
	dq_count = cvmx_pko3_get_queue_num(ipd_port);
	if (dq_base < 0 || dq_count <= 0) {
		printf("ERROR: %s: No descriptor queues for interface %d port %d\n",
		       __func__, xiface, index);
		return -1;
	}

	/* Mask out node from global DQ# */
	dq_base &= (1 << 10) - 1;

	limit = __pko_pkt_quota / dq_count /
		cvmx_helper_interface_enumerate(xiface);

	for (i = 0; i < dq_count; i++) {
		/* FIXME: 2ms at 1Gbps max packet rate, make speed dependent */
		cvmx_pko_dq_open(xi.node, dq_base + i);
		cvmx_pko3_dq_options(xi.node, dq_base + i, min_pad);

		if (debug)
			debug("%s: DQ%u limit %d\n", __func__, dq_base + i,
			      limit);

		cvmx_pko3_dq_set_limit(xi.node, dq_base + i, limit);
		__pko_pkt_budget -= limit;
	}

	if (__pko_pkt_budget < 0)
		printf("WARNING: %s: PKO buffer deficit %d\n", __func__,
		       __pko_pkt_budget);
	else if (debug)
		debug("%s: PKO remaining packet budget: %d\n", __func__,
		      __pko_pkt_budget);

	return i;
}

/** Configure and initialize PKO3 for an interface
 *
 * @param xiface is the interface number to configure
 * @return 0 on success.
 */
int cvmx_helper_pko3_init_interface(int xiface)
{
	cvmx_helper_interface_mode_t mode;
	int node, iface, subif, num_ports;
	bool fcs_enable, pad_enable, pad_enable_pko;
	u8 fcs_sof_off = 0;
	u8 num_queues = 1;
	bool qos = false, pfc = false;
	int res = -1;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	node = xi.node;
	iface = xi.interface;
	mode = cvmx_helper_interface_get_mode(xiface);
	num_ports = cvmx_helper_interface_enumerate(xiface);
	subif = 0;

	if ((unsigned int)iface <
	    NUM_ELEMENTS(__cvmx_pko_queue_static_config[node].pknd.pko_cfg_iface)) {
		pfc = __cvmx_pko_queue_static_config[node]
			      .pknd.pko_cfg_iface[iface]
			      .pfc_enable;
		num_queues = __cvmx_pko_queue_static_config[node]
				     .pknd.pko_cfg_iface[iface]
				     .queues_per_port;
		qos = __cvmx_pko_queue_static_config[node]
			      .pknd.pko_cfg_iface[iface]
			      .qos_enable;
	}

	/* Force 8 DQs per port for pass 1.0 to circumvent limitations */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
		num_queues = 8;

	/* For ILK there is one IPD port per channel */
	if (mode == CVMX_HELPER_INTERFACE_MODE_ILK)
		num_ports = __cvmx_helper_ilk_enumerate(xiface);

	/* Skip non-existent interfaces */
	if (num_ports < 1) {
		debug("ERROR: %s: invalid iface %u:%u\n", __func__, node,
		      iface);
		return -1;
	}

	if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) {
		num_queues = __cvmx_pko_queue_static_config[node]
				     .pknd.pko_cfg_loop.queues_per_port;
		qos = __cvmx_pko_queue_static_config[node]
			      .pknd.pko_cfg_loop.qos_enable;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_ports,
							num_queues, qos);
		if (res < 0)
			goto __cfg_error;
	} else if (mode == CVMX_HELPER_INTERFACE_MODE_NPI) {
		num_queues = __cvmx_pko_queue_static_config[node]
				     .pknd.pko_cfg_npi.queues_per_port;
		qos = __cvmx_pko_queue_static_config[node]
			      .pknd.pko_cfg_npi.qos_enable;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_ports,
							num_queues, qos);
		if (res < 0)
			goto __cfg_error;
	}
	/* ILK-specific queue configuration */
	else if (mode == CVMX_HELPER_INTERFACE_MODE_ILK) {
		unsigned int num_chans = __cvmx_helper_ilk_enumerate(xiface);

		num_queues = 8;
		qos = true;
		pfc = false;

		if (num_chans >= 128)
			num_queues = 1;
		else if (num_chans >= 64)
			num_queues = 2;
		else if (num_chans >= 32)
			num_queues = 4;
		else
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_chans,
							num_queues, qos);
	}
	/* Setup all ethernet configured for PFC */
	else if (pfc) {
		/* PFC interfaces have 8 prioritized queues */
		for (subif = 0; subif < num_ports; subif++) {
			res = __cvmx_pko3_config_pfc_interface(xiface, subif);
			if (res < 0)
				goto __cfg_error;

			/* Enable PFC/CBFC on BGX */
			__cvmx_helper_bgx_xaui_config_pfc(node, iface, subif,
							  true);
		}
	} else {
		/* All other interfaces follow static configuration */
		for (subif = 0; subif < num_ports; subif++) {
			res = __cvmx_pko3_config_gen_interface(xiface, subif,
							       num_queues, qos);
			if (res < 0)
				goto __cfg_error;
		}
	}

	fcs_enable = __cvmx_helper_get_has_fcs(xiface);
	pad_enable = __cvmx_helper_get_pko_padding(xiface);

	/* Do not use PKO PAD/FCS generation on o78p1.x on BGX interfaces */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		pad_enable_pko = false;
	else
		pad_enable_pko = pad_enable;

	if (debug)
		debug("%s: iface %u:%u FCS=%d pad=%d pko=%d\n", __func__, node,
		      iface, fcs_enable, pad_enable, pad_enable_pko);

	/* Setup interface options */
	for (subif = 0; subif < num_ports; subif++) {
		/* Open interface/port DQs to allow transmission to begin */
		res = __cvmx_pko3_helper_dqs_activate(xiface, subif,
						      pad_enable_pko);

		if (res < 0)
			goto __cfg_error;

		/* ILK has only one MAC, subif == logical-channel */
		if (mode == CVMX_HELPER_INTERFACE_MODE_ILK && subif > 0)
			continue;

		/* LOOP has only one MAC, subif == logical-channel */
		if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP && subif > 0)
			continue;

		/* NPI has only one MAC, subif == 'ring' */
		if (mode == CVMX_HELPER_INTERFACE_MODE_NPI && subif > 0)
			continue;

		/* for sRIO there is 16 byte sRIO header, outside of FCS */
		if (mode == CVMX_HELPER_INTERFACE_MODE_SRIO)
			fcs_sof_off = 16;

		if (iface >= CVMX_HELPER_MAX_GMX) {
			/* Non-BGX interface, use PKO for FCS/PAD */
			res = cvmx_pko3_interface_options(xiface, subif,
							  fcs_enable,
							  pad_enable_pko,
							  fcs_sof_off);
		} else if (pad_enable == pad_enable_pko) {
			/* BGX interface: FCS/PAD done by PKO */
			res = cvmx_pko3_interface_options(xiface, subif,
							  fcs_enable,
							  pad_enable,
							  fcs_sof_off);
			cvmx_helper_bgx_tx_options(node, iface, subif, false,
						   false);
		} else {
			/* BGX interface: FCS/PAD done by BGX */
			res = cvmx_pko3_interface_options(xiface, subif, false,
							  false, fcs_sof_off);
			cvmx_helper_bgx_tx_options(node, iface, subif,
						   fcs_enable, pad_enable);
		}

		if (res < 0)
			debug("WARNING: %s: option set failed on iface %u:%u/%u\n",
			      __func__, node, iface, subif);
		if (debug)
			debug("%s: face %u:%u/%u fifo size %d\n", __func__,
			      node, iface, subif,
			      cvmx_pko3_port_fifo_size(xiface, subif));
	}
	return 0;

__cfg_error:
	debug("ERROR: %s: failed on iface %u:%u/%u\n", __func__, node, iface,
	      subif);
	return -1;
}

/**
 * Global initialization for PKO3
 *
 * Should only be called once on each node
 *
 * TBD: Resolve the kernel case.
 * When Linux eats up the entire memory, bootmem will be unable to
 * satisfy our request, and the memory needs to come from Linux free pages.
 */
int __cvmx_helper_pko3_init_global(unsigned int node, uint16_t gaura)
{
	int res;

	res = cvmx_pko3_hw_init_global(node, gaura);
	if (res < 0) {
		debug("ERROR: %s:failed block initialization\n", __func__);
		return res;
	}

	/* configure channel level */
	cvmx_pko3_channel_credit_level(node, cvmx_pko_default_channel_level);

	/* add NULL MAC/DQ setup */
	res = __cvmx_pko3_config_null_interface(node);
	if (res < 0)
		debug("ERROR: %s: creating NULL interface\n", __func__);

	return res;
}

/**
 * Global initialization for PKO3
 *
 * Should only be called once on each node
 *
 * When Linux eats up the entire memory, bootmem will be unable to
 * satisfy our request, and the memory needs to come from Linux free pages.
 */
int cvmx_helper_pko3_init_global(unsigned int node)
{
	void *ptr;
	int res = -1;
	unsigned int aura_num = ~0;
	cvmx_fpa3_gaura_t aura;

	/* Allocate memory required by PKO3 */
	res = __cvmx_pko3_config_memory(node);
	if (res < 0) {
		debug("ERROR: %s: PKO3 memory allocation error\n", __func__);
		return res;
	}

	aura_num = res;
	aura = __cvmx_pko3_aura[node];

	/* Exercise the FPA to make sure the AURA is functional */
	ptr = cvmx_fpa3_alloc(aura);

	if (!ptr) {
		res = -1;
	} else {
		cvmx_fpa3_free_nosync(ptr, aura, 0);
		res = 0;
	}

	if (res < 0) {
		debug("ERROR: %s: FPA failure AURA=%u:%d\n", __func__,
		      aura.node, aura.laura);
		return -1;
	}

	res = __cvmx_helper_pko3_init_global(node, aura_num);

	if (res < 0)
		debug("ERROR: %s: failed to start PPKO\n", __func__);

	return res;
}
