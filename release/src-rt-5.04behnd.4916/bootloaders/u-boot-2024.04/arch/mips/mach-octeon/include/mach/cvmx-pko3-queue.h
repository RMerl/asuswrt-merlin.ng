/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_PKO3_QUEUE_H__
#define __CVMX_PKO3_QUEUE_H__

enum cvmx_pko3_level_e;

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
int __cvmx_pko3_dq_table_setup(void);

/*
 * Get the base Descriptor Queue number for an IPD port on the local node
 */
int cvmx_pko3_get_queue_base(int ipd_port);

/*
 * Get the number of Descriptor Queues assigned for an IPD port
 */
int cvmx_pko3_get_queue_num(int ipd_port);

/**
 * Get L1/Port Queue number assigned to interface port.
 *
 * @param xiface is interface number.
 * @param index is port index.
 */
int cvmx_pko3_get_port_queue(int xiface, int index);

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
				 unsigned int child_count, int stat_prio_count);

/*
 * @INTERNAL
 * Register a range of Descriptor Queues wth an interface port
 *
 * This function poulates the DQ-to-IPD translation table
 * used by the application to retrieve the DQ range (typically ordered
 * by priority) for a given IPD-port, which is either a physical port,
 * or a channel on a channelized interface (i.e. ILK).
 *
 * @param xiface is the physical interface number
 * @param index is either a physical port on an interface
 * @param or a channel of an ILK interface
 * @param dq_base is the first Descriptor Queue number in a consecutive range
 * @param dq_count is the number of consecutive Descriptor Queues leading
 * @param the same channel or port.
 *
 * Only a consecurive range of Descriptor Queues can be associated with any
 * given channel/port, and usually they are ordered from most to least
 * in terms of scheduling priority.
 *
 * Note: thus function only populates the node-local translation table.
 *
 * @returns 0 on success, -1 on failure.
 */
int __cvmx_pko3_ipd_dq_register(int xiface, int index, unsigned int dq_base, unsigned int dq_count);

/**
 * @INTERNAL
 *
 * Unregister DQs associated with CHAN_E (IPD port)
 */
int __cvmx_pko3_ipd_dq_unregister(int xiface, int index);

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
 * Return: returns none.
 */
void cvmx_pko3_map_channel(unsigned int node, unsigned int pq_num, unsigned int l2_l3_q_num,
			   u16 channel);

int cvmx_pko3_pq_config(unsigned int node, unsigned int mac_num, unsigned int pq_num);

int cvmx_pko3_port_cir_set(unsigned int node, unsigned int pq_num, unsigned long rate_kbips,
			   unsigned int burst_bytes, int adj_bytes);
int cvmx_pko3_dq_cir_set(unsigned int node, unsigned int pq_num, unsigned long rate_kbips,
			 unsigned int burst_bytes);
int cvmx_pko3_dq_pir_set(unsigned int node, unsigned int pq_num, unsigned long rate_kbips,
			 unsigned int burst_bytes);
typedef enum {
	CVMX_PKO3_SHAPE_RED_STALL,
	CVMX_PKO3_SHAPE_RED_DISCARD,
	CVMX_PKO3_SHAPE_RED_PASS
} red_action_t;

void cvmx_pko3_dq_red(unsigned int node, unsigned int dq_num, red_action_t red_act,
		      int8_t len_adjust);

/**
 * Macros to deal with short floating point numbers,
 * where unsigned exponent, and an unsigned normalized
 * mantissa are represented each with a defined field width.
 *
 */
#define CVMX_SHOFT_MANT_BITS 8
#define CVMX_SHOFT_EXP_BITS  4

/**
 * Convert short-float to an unsigned integer
 * Note that it will lose precision.
 */
#define CVMX_SHOFT_TO_U64(m, e)                                                                    \
	((((1ull << CVMX_SHOFT_MANT_BITS) | (m)) << (e)) >> CVMX_SHOFT_MANT_BITS)

/**
 * Convert to short-float from an unsigned integer
 */
#define CVMX_SHOFT_FROM_U64(ui, m, e)                                                              \
	do {                                                                                       \
		unsigned long long u;                                                              \
		unsigned int k;                                                                    \
		k = (1ull << (CVMX_SHOFT_MANT_BITS + 1)) - 1;                                      \
		(e) = 0;                                                                           \
		u = (ui) << CVMX_SHOFT_MANT_BITS;                                                  \
		while ((u) > k) {                                                                  \
			u >>= 1;                                                                   \
			(e)++;                                                                     \
		}                                                                                  \
		(m) = u & (k >> 1);                                                                \
	} while (0);

#define CVMX_SHOFT_MAX()                                                                           \
	CVMX_SHOFT_TO_U64((1 << CVMX_SHOFT_MANT_BITS) - 1, (1 << CVMX_SHOFT_EXP_BITS) - 1)
#define CVMX_SHOFT_MIN() CVMX_SHOFT_TO_U64(0, 0)

#endif /* __CVMX_PKO3_QUEUE_H__ */
