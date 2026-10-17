// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
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

/* Smalles Round-Robin quantum to use +1 */
#define CVMX_PKO3_RR_QUANTUM_MIN 0x10

static int debug; /* 1 for basic, 2 for detailed trace */

struct cvmx_pko3_dq {
	unsigned dq_count : 6; /* Number of descriptor queues */
	unsigned dq_base : 10; /* Descriptor queue start number */
#define CVMX_PKO3_SWIZZLE_IPD 0x0
};

/*
 * @INTERNAL
 * Descriptor Queue to IPD port mapping table.
 *
 * This pointer is per-core, contains the virtual address
 * of a global named block which has 2^12 entries per each
 * possible node.
 */
struct cvmx_pko3_dq *__cvmx_pko3_dq_table;

int cvmx_pko3_get_queue_base(int ipd_port)
{
	struct cvmx_pko3_dq *dq_table;
	int ret = -1;
	unsigned int i;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	/* get per-node table */
	if (cvmx_unlikely(!__cvmx_pko3_dq_table))
		__cvmx_pko3_dq_table_setup();

	i = CVMX_PKO3_SWIZZLE_IPD ^ xp.port;

	/* get per-node table */
	dq_table = __cvmx_pko3_dq_table + CVMX_PKO3_IPD_NUM_MAX * xp.node;

	if (cvmx_likely(dq_table[i].dq_count > 0))
		ret = xp.node << 10 | dq_table[i].dq_base;
	else if (debug)
		cvmx_printf("ERROR: %s: no queues for ipd_port=%#x\n", __func__,
			    ipd_port);

	return ret;
}

int cvmx_pko3_get_queue_num(int ipd_port)
{
	struct cvmx_pko3_dq *dq_table;
	int ret = -1;
	unsigned int i;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	/* get per-node table */
	if (cvmx_unlikely(!__cvmx_pko3_dq_table))
		__cvmx_pko3_dq_table_setup();

	i = CVMX_PKO3_SWIZZLE_IPD ^ xp.port;

	/* get per-node table */
	dq_table = __cvmx_pko3_dq_table + CVMX_PKO3_IPD_NUM_MAX * xp.node;

	if (cvmx_likely(dq_table[i].dq_count > 0))
		ret = dq_table[i].dq_count;
	else if (debug)
		debug("ERROR: %s: no queues for ipd_port=%#x\n", __func__,
		      ipd_port);

	return ret;
}

/**
 * @INTERNAL
 *
 * Initialize port/dq table contents
 */
static void __cvmx_pko3_dq_table_init(void *ptr)
{
	unsigned int size = sizeof(struct cvmx_pko3_dq) *
			    CVMX_PKO3_IPD_NUM_MAX * CVMX_MAX_NODES;

	memset(ptr, 0, size);
}

/**
 * @INTERNAL
 *
 * Find or allocate global port/dq map table
 * which is a named table, contains entries for
 * all possible OCI nodes.
 *
 * The table global pointer is stored in core-local variable
 * so that every core will call this function once, on first use.
 */
int __cvmx_pko3_dq_table_setup(void)
{
	void *ptr;

	ptr = cvmx_bootmem_alloc_named_range_once(
		/* size */
		sizeof(struct cvmx_pko3_dq) * CVMX_PKO3_IPD_NUM_MAX *
			CVMX_MAX_NODES,
		/* min_addr, max_addr, align */
		0ull, 0ull, sizeof(struct cvmx_pko3_dq),
		/* name */
		"cvmx_pko3_global_dq_table", __cvmx_pko3_dq_table_init);

	if (debug)
		debug("%s: dq_table_ptr=%p\n", __func__, ptr);

	if (!ptr)
		return -1;

	__cvmx_pko3_dq_table = ptr;
	return 0;
}

/*
 * @INTERNAL
 * Register a range of Descriptor Queues with an interface port
 *
 * This function populates the DQ-to-IPD translation table
 * used by the application to retrieve the DQ range (typically ordered
 * by priority) for a given IPD-port, which is either a physical port,
 * or a channel on a channelized interface (i.e. ILK).
 *
 * @param xiface is the physical interface number
 * @param index is either a physical port on an interface
 *        or a channel of an ILK interface
 * @param dq_base is the first Descriptor Queue number in a consecutive range
 * @param dq_count is the number of consecutive Descriptor Queues leading
 *        the same channel or port.
 *
 * Only a consecutive range of Descriptor Queues can be associated with any
 * given channel/port, and usually they are ordered from most to least
 * in terms of scheduling priority.
 *
 * Note: thus function only populates the node-local translation table.
 * NOTE: This function would be cleaner if it had a single ipd_port argument
 *
 * @returns 0 on success, -1 on failure.
 */
int __cvmx_pko3_ipd_dq_register(int xiface, int index, unsigned int dq_base,
				unsigned int dq_count)
{
	struct cvmx_pko3_dq *dq_table;
	int ipd_port;
	unsigned int i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp;

	if (__cvmx_helper_xiface_is_null(xiface)) {
		ipd_port = cvmx_helper_node_to_ipd_port(xi.node,
							CVMX_PKO3_IPD_PORT_NULL);
	} else {
		int p;

		p = cvmx_helper_get_ipd_port(xiface, index);
		if (p < 0) {
			cvmx_printf("ERROR: %s: xiface %#x has no IPD port\n",
				    __func__, xiface);
			return -1;
		}
		ipd_port = p;
	}

	xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	i = CVMX_PKO3_SWIZZLE_IPD ^ xp.port;

	/* get per-node table */
	if (!__cvmx_pko3_dq_table)
		__cvmx_pko3_dq_table_setup();

	dq_table = __cvmx_pko3_dq_table + CVMX_PKO3_IPD_NUM_MAX * xi.node;

	if (debug)
		debug("%s: ipd_port=%#x ix=%#x dq %u cnt %u\n", __func__,
		      ipd_port, i, dq_base, dq_count);

	/* Check the IPD port has not already been configured */
	if (dq_table[i].dq_count > 0) {
		cvmx_printf("%s: ERROR: IPD %#x already registered\n", __func__,
			    ipd_port);
		return -1;
	}

	/* Store DQ# range in the queue lookup table */
	dq_table[i].dq_base = dq_base;
	dq_table[i].dq_count = dq_count;

	return 0;
}

/*
 * @INTERNAL
 * Convert normal CHAN_E (i.e. IPD port) value to compressed channel form
 * that is used to populate PKO_LUT.
 *
 * Note: This code may be model specific.
 */
static int cvmx_pko3_chan_2_xchan(uint16_t ipd_port)
{
	u16 xchan;
	u8 off;
	static const u8 *xchan_base;
	static const u8 xchan_base_cn78xx[16] = {
		/* IPD 0x000 */ 0x3c0 >> 4, /* LBK */
		/* IPD 0x100 */ 0x380 >> 4, /* DPI */
		/* IPD 0x200 */ 0xfff >> 4, /* not used */
		/* IPD 0x300 */ 0xfff >> 4, /* not used */
		/* IPD 0x400 */ 0x000 >> 4, /* ILK0 */
		/* IPD 0x500 */ 0x100 >> 4, /* ILK1 */
		/* IPD 0x600 */ 0xfff >> 4, /* not used */
		/* IPD 0x700 */ 0xfff >> 4, /* not used */
		/* IPD 0x800 */ 0x200 >> 4, /* BGX0 */
		/* IPD 0x900 */ 0x240 >> 4, /* BGX1 */
		/* IPD 0xa00 */ 0x280 >> 4, /* BGX2 */
		/* IPD 0xb00 */ 0x2c0 >> 4, /* BGX3 */
		/* IPD 0xc00 */ 0x300 >> 4, /* BGX4 */
		/* IPD 0xd00 */ 0x340 >> 4, /* BGX5 */
		/* IPD 0xe00 */ 0xfff >> 4, /* not used */
		/* IPD 0xf00 */ 0xfff >> 4  /* not used */
	};
	static const u8 xchan_base_cn73xx[16] = {
		/* IPD 0x000 */ 0x0c0 >> 4, /* LBK */
		/* IPD 0x100 */ 0x100 >> 4, /* DPI */
		/* IPD 0x200 */ 0xfff >> 4, /* not used */
		/* IPD 0x300 */ 0xfff >> 4, /* not used */
		/* IPD 0x400 */ 0xfff >> 4, /* not used */
		/* IPD 0x500 */ 0xfff >> 4, /* not used */
		/* IPD 0x600 */ 0xfff >> 4, /* not used */
		/* IPD 0x700 */ 0xfff >> 4, /* not used */
		/* IPD 0x800 */ 0x000 >> 4, /* BGX0 */
		/* IPD 0x900 */ 0x040 >> 4, /* BGX1 */
		/* IPD 0xa00 */ 0x080 >> 4, /* BGX2 */
		/* IPD 0xb00 */ 0xfff >> 4, /* not used */
		/* IPD 0xc00 */ 0xfff >> 4, /* not used */
		/* IPD 0xd00 */ 0xfff >> 4, /* not used */
		/* IPD 0xe00 */ 0xfff >> 4, /* not used */
		/* IPD 0xf00 */ 0xfff >> 4  /* not used */
	};
	static const u8 xchan_base_cn75xx[16] = {
		/* IPD 0x000 */ 0x040 >> 4, /* LBK */
		/* IPD 0x100 */ 0x080 >> 4, /* DPI */
		/* IPD 0x200 */ 0xeee >> 4, /* SRIO0  noop */
		/* IPD 0x300 */ 0xfff >> 4, /* not used */
		/* IPD 0x400 */ 0xfff >> 4, /* not used */
		/* IPD 0x500 */ 0xfff >> 4, /* not used */
		/* IPD 0x600 */ 0xfff >> 4, /* not used */
		/* IPD 0x700 */ 0xfff >> 4, /* not used */
		/* IPD 0x800 */ 0x000 >> 4, /* BGX0 */
		/* IPD 0x900 */ 0xfff >> 4, /* not used */
		/* IPD 0xa00 */ 0xfff >> 4, /* not used */
		/* IPD 0xb00 */ 0xfff >> 4, /* not used */
		/* IPD 0xc00 */ 0xfff >> 4, /* not used */
		/* IPD 0xd00 */ 0xfff >> 4, /* not used */
		/* IPD 0xe00 */ 0xfff >> 4, /* not used */
		/* IPD 0xf00 */ 0xfff >> 4  /* not used */
	};

	if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		xchan_base = xchan_base_cn73xx;
	if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		xchan_base = xchan_base_cn75xx;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		xchan_base = xchan_base_cn78xx;

	if (!xchan_base)
		return -1;

	xchan = ipd_port >> 8;

	/* ILKx, DPI has 8 bits logical channels, others just 6 */
	if (((xchan & 0xfe) == 0x04) || xchan == 0x01)
		off = ipd_port & 0xff;
	else
		off = ipd_port & 0x3f;

	xchan = xchan_base[xchan & 0xf];

	if (xchan == 0xff)
		return -1; /* Invalid IPD_PORT */
	else if (xchan == 0xee)
		return -2; /* LUT not used */
	else
		return (xchan << 4) | off;
}

/*
 * Map channel number in PKO
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param pq_num specifies the Port Queue (i.e. L1) queue number.
 * @param l2_l3_q_num  specifies L2/L3 queue number.
 * @param channel specifies the channel number to map to the queue.
 *
 * The channel assignment applies to L2 or L3 Shaper Queues depending
 * on the setting of channel credit level.
 *
 * @return returns none.
 */
void cvmx_pko3_map_channel(unsigned int node, unsigned int pq_num,
			   unsigned int l2_l3_q_num, uint16_t channel)
{
	union cvmx_pko_l3_l2_sqx_channel sqx_channel;
	cvmx_pko_lutx_t lutx;
	int xchan;

	sqx_channel.u64 =
		csr_rd_node(node, CVMX_PKO_L3_L2_SQX_CHANNEL(l2_l3_q_num));

	sqx_channel.s.cc_channel = channel;

	csr_wr_node(node, CVMX_PKO_L3_L2_SQX_CHANNEL(l2_l3_q_num),
		    sqx_channel.u64);

	/* Convert CHAN_E into compressed channel */
	xchan = cvmx_pko3_chan_2_xchan(channel);

	if (debug)
		debug("%s: ipd_port=%#x xchan=%#x\n", __func__, channel, xchan);

	if (xchan < 0) {
		if (xchan == -1)
			cvmx_printf("%s: ERROR: channel %#x not recognized\n",
				    __func__, channel);
		return;
	}

	lutx.u64 = 0;
	lutx.s.valid = 1;
	lutx.s.pq_idx = pq_num;
	lutx.s.queue_number = l2_l3_q_num;

	csr_wr_node(node, CVMX_PKO_LUTX(xchan), lutx.u64);

	if (debug)
		debug("%s: channel %#x (compressed=%#x) mapped L2/L3 SQ=%u, PQ=%u\n",
		      __func__, channel, xchan, l2_l3_q_num, pq_num);
}

/*
 * @INTERNAL
 * This function configures port queue scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param port_queue is the port queue number to be configured.
 * @param mac_num is the mac number of the mac that will be tied to this port_queue.
 */
static void cvmx_pko_configure_port_queue(int node, int port_queue, int mac_num)
{
	cvmx_pko_l1_sqx_topology_t pko_l1_topology;
	cvmx_pko_l1_sqx_shape_t pko_l1_shape;
	cvmx_pko_l1_sqx_link_t pko_l1_link;

	pko_l1_topology.u64 = 0;
	pko_l1_topology.s.link = mac_num;
	csr_wr_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(port_queue),
		    pko_l1_topology.u64);

	pko_l1_shape.u64 = 0;
	pko_l1_shape.s.link = mac_num;
	csr_wr_node(node, CVMX_PKO_L1_SQX_SHAPE(port_queue), pko_l1_shape.u64);

	pko_l1_link.u64 = 0;
	pko_l1_link.s.link = mac_num;
	csr_wr_node(node, CVMX_PKO_L1_SQX_LINK(port_queue), pko_l1_link.u64);
}

/*
 * @INTERNAL
 * This function configures level 2 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level3 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l3 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy children.
 * @param child_rr_prio is the round robin children priority.
 */
static void cvmx_pko_configure_l2_queue(int node, int queue, int parent_queue,
					int prio, int rr_quantum,
					int child_base, int child_rr_prio)
{
	cvmx_pko_l2_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l2_sqx_topology_t pko_child_topology;
	cvmx_pko_l1_sqx_topology_t pko_parent_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 =
		csr_rd_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	csr_wr_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(parent_queue),
		    pko_parent_topology.u64);

	if (debug > 1)
		debug("CVMX_PKO_L1_SQX_TOPOLOGY(%u): PRIO_ANCHOR=%u PARENT=%u\n",
		      parent_queue, pko_parent_topology.s.prio_anchor,
		      pko_parent_topology.s.link);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	csr_wr_node(node, CVMX_PKO_L2_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* child topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	csr_wr_node(node, CVMX_PKO_L2_SQX_TOPOLOGY(queue),
		    pko_child_topology.u64);
}

/*
 * @INTERNAL
 * This function configures level 3 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level3 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l3 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy children.
 * @param child_rr_prio is the round robin children priority.
 */
static void cvmx_pko_configure_l3_queue(int node, int queue, int parent_queue,
					int prio, int rr_quantum,
					int child_base, int child_rr_prio)
{
	cvmx_pko_l3_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l3_sqx_topology_t pko_child_topology;
	cvmx_pko_l2_sqx_topology_t pko_parent_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 =
		csr_rd_node(node, CVMX_PKO_L2_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	csr_wr_node(node, CVMX_PKO_L2_SQX_TOPOLOGY(parent_queue),
		    pko_parent_topology.u64);

	if (debug > 1)
		debug("CVMX_PKO_L2_SQX_TOPOLOGY(%u): PRIO_ANCHOR=%u PARENT=%u\n",
		      parent_queue, pko_parent_topology.s.prio_anchor,
		      pko_parent_topology.s.parent);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	csr_wr_node(node, CVMX_PKO_L3_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* child topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	csr_wr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(queue),
		    pko_child_topology.u64);
}

/*
 * @INTERNAL
 * This function configures level 4 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level4 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l4 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy children.
 * @param child_rr_prio is the round robin children priority.
 */
static void cvmx_pko_configure_l4_queue(int node, int queue, int parent_queue,
					int prio, int rr_quantum,
					int child_base, int child_rr_prio)
{
	cvmx_pko_l4_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l4_sqx_topology_t pko_child_topology;
	cvmx_pko_l3_sqx_topology_t pko_parent_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 =
		csr_rd_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	csr_wr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(parent_queue),
		    pko_parent_topology.u64);

	if (debug > 1)
		debug("CVMX_PKO_L3_SQX_TOPOLOGY(%u): PRIO_ANCHOR=%u PARENT=%u\n",
		      parent_queue, pko_parent_topology.s.prio_anchor,
		      pko_parent_topology.s.parent);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	csr_wr_node(node, CVMX_PKO_L4_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	csr_wr_node(node, CVMX_PKO_L4_SQX_TOPOLOGY(queue),
		    pko_child_topology.u64);
}

/*
 * @INTERNAL
 * This function configures level 5 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level5 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l5 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy children.
 * @param child_rr_prio is the round robin children priority.
 */
static void cvmx_pko_configure_l5_queue(int node, int queue, int parent_queue,
					int prio, int rr_quantum,
					int child_base, int child_rr_prio)
{
	cvmx_pko_l5_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l4_sqx_topology_t pko_parent_topology;
	cvmx_pko_l5_sqx_topology_t pko_child_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 =
		csr_rd_node(node, CVMX_PKO_L4_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	csr_wr_node(node, CVMX_PKO_L4_SQX_TOPOLOGY(parent_queue),
		    pko_parent_topology.u64);

	if (debug > 1)
		debug("CVMX_PKO_L4_SQX_TOPOLOGY(%u): PRIO_ANCHOR=%u PARENT=%u\n",
		      parent_queue, pko_parent_topology.s.prio_anchor,
		      pko_parent_topology.s.parent);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	csr_wr_node(node, CVMX_PKO_L5_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	csr_wr_node(node, CVMX_PKO_L5_SQX_TOPOLOGY(queue),
		    pko_child_topology.u64);
}

/*
 * @INTERNAL
 * This function configures descriptor queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be configured.
 * @param parent_queue is the parent queue at next level for this dq.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy children.
 * @param child_rr_prio is the round robin children priority.
 */
static void cvmx_pko_configure_dq(int node, int dq, int parent_queue, int prio,
				  int rr_quantum, int child_base,
				  int child_rr_prio)
{
	cvmx_pko_dqx_schedule_t pko_dq_sched;
	cvmx_pko_dqx_topology_t pko_dq_topology;
	cvmx_pko_l5_sqx_topology_t pko_parent_topology;
	cvmx_pko_dqx_wm_ctl_t pko_dq_wm_ctl;
	unsigned long long parent_topology_reg;
	char lvl;

	if (debug)
		debug("%s: dq %u parent %u child_base %u\n", __func__, dq,
		      parent_queue, child_base);

	if (__cvmx_pko3_sq_lvl_max() == CVMX_PKO_L5_QUEUES) {
		parent_topology_reg = CVMX_PKO_L5_SQX_TOPOLOGY(parent_queue);
		lvl = 5;
	} else if (__cvmx_pko3_sq_lvl_max() == CVMX_PKO_L3_QUEUES) {
		parent_topology_reg = CVMX_PKO_L3_SQX_TOPOLOGY(parent_queue);
		lvl = 3;
	} else {
		return;
	}

	if (debug)
		debug("%s: parent_topology_reg=%#llx\n", __func__,
		      parent_topology_reg);

	/* parent topology configuration */
	pko_parent_topology.u64 = csr_rd_node(node, parent_topology_reg);
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	csr_wr_node(node, parent_topology_reg, pko_parent_topology.u64);

	if (debug > 1)
		debug("CVMX_PKO_L%d_SQX_TOPOLOGY(%u): PRIO_ANCHOR=%u PARENT=%u\n",
		      lvl, parent_queue, pko_parent_topology.s.prio_anchor,
		      pko_parent_topology.s.parent);

	/* scheduler configuration for this dq in the parent queue */
	pko_dq_sched.u64 = 0;
	pko_dq_sched.s.prio = prio;
	pko_dq_sched.s.rr_quantum = rr_quantum;
	csr_wr_node(node, CVMX_PKO_DQX_SCHEDULE(dq), pko_dq_sched.u64);

	/* topology configuration */
	pko_dq_topology.u64 = 0;
	pko_dq_topology.s.parent = parent_queue;
	csr_wr_node(node, CVMX_PKO_DQX_TOPOLOGY(dq), pko_dq_topology.u64);

	/* configure for counting packets, not bytes at this level */
	pko_dq_wm_ctl.u64 = 0;
	pko_dq_wm_ctl.s.kind = 1;
	pko_dq_wm_ctl.s.enable = 0;
	csr_wr_node(node, CVMX_PKO_DQX_WM_CTL(dq), pko_dq_wm_ctl.u64);

	if (debug > 1) {
		pko_dq_sched.u64 = csr_rd_node(node, CVMX_PKO_DQX_SCHEDULE(dq));
		pko_dq_topology.u64 =
			csr_rd_node(node, CVMX_PKO_DQX_TOPOLOGY(dq));
		debug("CVMX_PKO_DQX_TOPOLOGY(%u)PARENT=%u CVMX_PKO_DQX_SCHEDULE(%u) PRIO=%u Q=%u\n",
		      dq, pko_dq_topology.s.parent, dq, pko_dq_sched.s.prio,
		      pko_dq_sched.s.rr_quantum);
	}
}

/*
 * @INTERNAL
 * The following structure selects the Scheduling Queue configuration
 * routine for each of the supported levels.
 * The initial content of the table will be setup in accordance
 * to the specific SoC model and its implemented resources
 */
struct pko3_cfg_tab_s {
	/* function pointer for to configure the given level, last=DQ */
	struct {
		u8 parent_level;
		void (*cfg_sq_func)(int node, int queue, int parent_queue,
				    int prio, int rr_quantum, int child_base,
				    int child_rr_prio);
		//XXX for debugging exagerated size
	} lvl[256];
};

static const struct pko3_cfg_tab_s pko3_cn78xx_cfg = {
	{ [CVMX_PKO_L2_QUEUES] = { CVMX_PKO_PORT_QUEUES,
				   cvmx_pko_configure_l2_queue },
	  [CVMX_PKO_L3_QUEUES] = { CVMX_PKO_L2_QUEUES,
				   cvmx_pko_configure_l3_queue },
	  [CVMX_PKO_L4_QUEUES] = { CVMX_PKO_L3_QUEUES,
				   cvmx_pko_configure_l4_queue },
	  [CVMX_PKO_L5_QUEUES] = { CVMX_PKO_L4_QUEUES,
				   cvmx_pko_configure_l5_queue },
	  [CVMX_PKO_DESCR_QUEUES] = { CVMX_PKO_L5_QUEUES,
				      cvmx_pko_configure_dq } }
};

static const struct pko3_cfg_tab_s pko3_cn73xx_cfg = {
	{ [CVMX_PKO_L2_QUEUES] = { CVMX_PKO_PORT_QUEUES,
				   cvmx_pko_configure_l2_queue },
	  [CVMX_PKO_L3_QUEUES] = { CVMX_PKO_L2_QUEUES,
				   cvmx_pko_configure_l3_queue },
	  [CVMX_PKO_DESCR_QUEUES] = { CVMX_PKO_L3_QUEUES,
				      cvmx_pko_configure_dq } }
};

/*
 * Configure Port Queue and its children Scheduler Queue
 *
 * Port Queues (a.k.a L1) are assigned 1-to-1 to MACs.
 * L2 Scheduler Queues are used for specifying channels, and thus there
 * could be multiple L2 SQs attached to a single L1 PQ, either in a
 * fair round-robin scheduling, or with static and/or round-robin priorities.
 *
 * @param node on which to operate
 * @param mac_num is the LMAC number to that is associated with the Port Queue,
 * @param pq_num is the number of the L1 PQ attached to the MAC
 *
 * @returns 0 on success, -1 on failure.
 */
int cvmx_pko3_pq_config(unsigned int node, unsigned int mac_num,
			unsigned int pq_num)
{
	char b1[10];

	if (debug)
		debug("%s: MAC%u -> %s\n", __func__, mac_num,
		      __cvmx_pko3_sq_str(b1, CVMX_PKO_PORT_QUEUES, pq_num));

	cvmx_pko_configure_port_queue(node, pq_num, mac_num);

	return 0;
}

/*
 * Configure L3 through L5 Scheduler Queues and Descriptor Queues
 *
 * The Scheduler Queues in Levels 3 to 5 and Descriptor Queues are
 * configured one-to-one or many-to-one to a single parent Scheduler
 * Queues. The level of the parent SQ is specified in an argument,
 * as well as the number of children to attach to the specific parent.
 * The children can have fair round-robin or priority-based scheduling
 * when multiple children are assigned a single parent.
 *
 * @param node on which to operate
 * @param child_level  is the level of the child queue
 * @param parent_queue is the number of the parent Scheduler Queue
 * @param child_base is the number of the first child SQ or DQ to assign to
 * @param child_count is the number of consecutive children to assign
 * @param stat_prio_count is the priority setting for the children L2 SQs
 *
 * If <stat_prio_count> is -1, the Ln children will have equal Round-Robin
 * relationship with eachother. If <stat_prio_count> is 0, all Ln children
 * will be arranged in Weighted-Round-Robin, with the first having the most
 * precedence. If <stat_prio_count> is between 1 and 8, it indicates how
 * many children will have static priority settings (with the first having
 * the most precedence), with the remaining Ln children having WRR scheduling.
 *
 * @returns 0 on success, -1 on failure.
 *
 * Note: this function supports the configuration of node-local unit.
 */
int cvmx_pko3_sq_config_children(unsigned int node,
				 enum cvmx_pko3_level_e child_level,
				 unsigned int parent_queue,
				 unsigned int child_base,
				 unsigned int child_count, int stat_prio_count)
{
	enum cvmx_pko3_level_e parent_level;
	unsigned int num_elem = 0;
	unsigned int rr_quantum, rr_count;
	unsigned int child, prio, rr_prio;
	const struct pko3_cfg_tab_s *cfg_tbl = NULL;
	char b1[10], b2[10];

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		num_elem = NUM_ELEMENTS(pko3_cn78xx_cfg.lvl);
		cfg_tbl = &pko3_cn78xx_cfg;
	}
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		num_elem = NUM_ELEMENTS(pko3_cn73xx_cfg.lvl);
		cfg_tbl = &pko3_cn73xx_cfg;
	}

	if (!cfg_tbl || child_level >= num_elem) {
		cvmx_printf("ERROR: %s: model or level %#x invalid\n", __func__,
			    child_level);
		return -1;
	}

	parent_level = cfg_tbl->lvl[child_level].parent_level;

	if (!cfg_tbl->lvl[child_level].cfg_sq_func ||
	    cfg_tbl->lvl[child_level].parent_level == 0) {
		cvmx_printf("ERROR: %s: queue level %#x invalid\n", __func__,
			    child_level);
		return -1;
	}

	/* First static priority is 0 - top precedence */
	prio = 0;

	if (stat_prio_count > (signed int)child_count)
		stat_prio_count = child_count;

	/* Valid PRIO field is 0..9, limit maximum static priorities */
	if (stat_prio_count > 9)
		stat_prio_count = 9;

	/* Special case of a single child */
	if (child_count == 1) {
		rr_count = 0;
		rr_prio = 0xF;
		/* Special case for Fair-RR */
	} else if (stat_prio_count < 0) {
		rr_count = child_count;
		rr_prio = 0;
	} else {
		rr_count = child_count - stat_prio_count;
		rr_prio = stat_prio_count;
	}

	/* Compute highest RR_QUANTUM */
	if (stat_prio_count > 0)
		rr_quantum = CVMX_PKO3_RR_QUANTUM_MIN * rr_count;
	else
		rr_quantum = CVMX_PKO3_RR_QUANTUM_MIN;

	if (debug)
		debug("%s: Parent %s child_base %u rr_pri %u\n", __func__,
		      __cvmx_pko3_sq_str(b1, parent_level, parent_queue),
		      child_base, rr_prio);

	/* Parent is configured with child */

	for (child = child_base; child < (child_base + child_count); child++) {
		if (debug)
			debug("%s: Child %s of %s prio %u rr_quantum %#x\n",
			      __func__,
			      __cvmx_pko3_sq_str(b1, child_level, child),
			      __cvmx_pko3_sq_str(b2, parent_level,
						 parent_queue),
			      prio, rr_quantum);

		cfg_tbl->lvl[child_level].cfg_sq_func(node, child, parent_queue,
						      prio, rr_quantum,
						      child_base, rr_prio);

		if (prio < rr_prio)
			prio++;
		else if (stat_prio_count > 0)
			rr_quantum -= CVMX_PKO3_RR_QUANTUM_MIN;
	} /* for child */

	return 0;
}
