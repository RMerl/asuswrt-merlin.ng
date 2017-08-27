/*
 * lib/route/qdisc/sfb.c		Stochastic Fair Blue Qdisc
 */
/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_sfb Stochastic Fair Blue (SFB)
 * @brief
 * Sch_sfb is an implementation of the Stochastic Fair Blue (SFB) queue
 * management algorithm for Linux.
 *
 * SFB aims to keep your queues short while avoiding packet drops,
 * maximising link utilisation and preserving fairness between flows.
 * SFB will detect flows that do not respond to congestion indications,
 * and limit them to a constant share of the link's capacity.
 *
 * SFB only deals with marking and/or droping packets.  Reordering of
 * packets is handled by sfb's child qdisc; by default, this is pfifo,
 * meaning that no reordering will happen.  You may want to use something
 * like prio or sfq as sfb's child.
 *
 * Unlike most other fair queueing policies, SFB doesn't actually keep
 * per-flow state; it manages flow information using a Bloom filter,
 * which means that hash collisions are reduced to a minimum while using
 * fairly little memory.
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/sfb.h>

/** @cond SKIP */
#define SCH_SFB_ATTR_REHASH	0x001
#define SCH_SFB_ATTR_WARMUP	0x002
#define SCH_SFB_ATTR_LIMIT	0x004
#define SCH_SFB_ATTR_TARGET	0x008
#define SCH_SFB_ATTR_MAX	0x010
#define SCH_SFB_ATTR_INCREMENT	0x020
#define SCH_SFB_ATTR_DECREMENT	0x040
#define SCH_SFB_ATTR_PEN_RATE	0x080
#define SCH_SFB_ATTR_PEN_BURST	0x100
/** @endcond */

static int sfb_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_sfb *sfb = data;
	struct tc_sfb_qopt *opts;

	if (!(tc->ce_mask & TCA_ATTR_OPTS))
		return 0;

	if (tc->tc_opts->d_size < sizeof(*opts))
		return -NLE_INVAL;

	opts = (struct tc_sfb_qopt *) tc->tc_opts->d_data;

	sfb->qsb_rehash_interval	= opts->rehash_interval;
	sfb->qsb_warmup_time		= opts->warmup_time;
	sfb->qsb_limit			= opts->limit;
	sfb->qsb_target			= opts->bin_size;
	sfb->qsb_max			= opts->max;
	sfb->qsb_increment		= opts->increment;
	sfb->qsb_decrement		= opts->decrement;
	sfb->qsb_penalty_rate		= opts->penalty_rate;
	sfb->qsb_penalty_burst		= opts->penalty_burst;

	sfb->qsb_mask = (SCH_SFB_ATTR_REHASH | SCH_SFB_ATTR_WARMUP |
			 SCH_SFB_ATTR_LIMIT | SCH_SFB_ATTR_MAX |
			 SCH_SFB_ATTR_TARGET |
			 SCH_SFB_ATTR_INCREMENT | SCH_SFB_ATTR_DECREMENT |
			 SCH_SFB_ATTR_PEN_RATE | SCH_SFB_ATTR_PEN_BURST);

	return 0;
}

static void sfb_dump_line(struct rtnl_tc *tc, void *data,
			  struct nl_dump_params *p)
{
	struct rtnl_sfb *sfb = data;

	if (sfb)
		nl_dump(p, " limit %u target %u max %u" ,
		        sfb->qsb_limit, sfb->qsb_target, sfb->qsb_max);
}

static void sfb_dump_details(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
	struct rtnl_sfb *sfb = data;

	if (sfb)
		nl_dump(p, "rehash_interval %us warmup_time %us "
			   "increment %u decrement %u "
			   "penalty_rate %up/s penalty_burst %up",
			sfb->qsb_rehash_interval, sfb->qsb_warmup_time,
			sfb->qsb_increment, sfb->qsb_decrement,
			sfb->qsb_penalty_rate, sfb->qsb_penalty_burst);
}

static int sfb_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_sfb *sfb = data;
	struct tc_sfb_qopt opts = {0};

	if (!sfb)
		BUG();

	opts.rehash_interval	= sfb->qsb_rehash_interval;
	opts.warmup_time	= sfb->qsb_warmup_time;
	opts.limit		= sfb->qsb_limit;	
	opts.bin_size 		= sfb->qsb_target;
	opts.max		= sfb->qsb_max;
	opts.increment		= sfb->qsb_increment;
	opts.decrement		= sfb->qsb_decrement;
	opts.penalty_rate	= sfb->qsb_penalty_rate;
	opts.penalty_burst	= sfb->qsb_penalty_burst;

	return nlmsg_append(msg, &opts, sizeof(opts), NL_DONTPAD);
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set the rehash interval of SFB qdisc.
 * 
 * @arg qdisc		SFB qdisc to be modified.
 * @arg interval_secs	Specifies how often, in seconds, before we will switch
 * to a fresh Bloom filter and a different set of hash functions.  Since Bloom
 * filters are more resistent to hash collisions that hash tables, this may be
 * set to a fairly large value.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_rehash_interval(struct rtnl_qdisc *qdisc, int interval_secs)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_rehash_interval = interval_secs;
	sfb->qsb_mask |= SCH_SFB_ATTR_REHASH;
}

/**
 * Get rehash interval of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Rehash interval in seconds, or a negative error code.
 */
int rtnl_sfb_get_rehash_interval(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_REHASH)
		return sfb->qsb_rehash_interval;
	else
		return -NLE_NOATTR;
}

/**
 * Set the warmup time of SFB qdisc.
 * 
 * @arg qdisc		SFB qdisc to be modified.
 * @arg warmup_secs	In seconds, how long we will perform double buffering
 * before switching to a new bloom filter.  This should be long enough to make
 * sure that the new filter is ``primed'' before it is used, a few tens of 
 * seconds should be enough.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_warmup_time(struct rtnl_qdisc *qdisc, int warmup_secs)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_warmup_time = warmup_secs;
	sfb->qsb_mask |= SCH_SFB_ATTR_WARMUP;
}

/**
 * Get warmup time of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Warmup time in seconds, or a negative error code.
 */
int rtnl_sfb_get_warmup_time(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_WARMUP)
		return sfb->qsb_warmup_time;
	else
		return -NLE_NOATTR;
}

/**
 * Set the total packet limit of SFB qdisc.
 * 
 * @arg qdisc		SFB qdisc to be modified.
 * @arg interval	The hard limit on the number of packets in all of sfb's
 * aggregates (total qdisc limit).  Set it to some large value, it should never
 * be reached anyway.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_limit(struct rtnl_qdisc *qdisc, int interval)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_limit = interval;
	sfb->qsb_mask |= SCH_SFB_ATTR_REHASH;
}

/**
 * Get rehash interval of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Limit in packets, or a negative error code.
 */
int rtnl_sfb_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_REHASH)
		return sfb->qsb_limit;
	else
		return -NLE_NOATTR;
}

/**
 * Set the target number of packets per aggregate of SFB qdisc.
 * 
 * @arg qdisc		SFB qdisc to be modified.
 * @arg target_packets	the target per-flow queue size in packets.  sfb will
 * try to keep each per-aggregate queue between 0 and target.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_target_packets(struct rtnl_qdisc *qdisc, int target_packets)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_target = target_packets;
	sfb->qsb_mask |= SCH_SFB_ATTR_TARGET;
}

/**
 * Get rehash interval of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Target packets per aggregate, or a negative error code.
 */
int rtnl_sfb_get_target_packets(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_TARGET)
		return sfb->qsb_target;
	else
		return -NLE_NOATTR;
}

/**
 * Set the max packets per aggregate of SFB qdisc.
 * 
 * @arg qdisc		SFB qdisc to be modified.
 * @arg max_packets	The maximum number of packets queued for a given
 * aggregate.  It should be very slightly larger than target, in order to
 * absorb transient bursts.  Setting this to more than roughly 1.5 times target
 * will cause instabilities with which Blue is not designed to cope.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_max_packets(struct rtnl_qdisc *qdisc, int max_packets)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_max = max_packets;
	sfb->qsb_mask |= SCH_SFB_ATTR_MAX;
}

/**
 * Get max packets per aggregate of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Max packets, or a negative error code.
 */
int rtnl_sfb_get_max_packets(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_MAX)
		return sfb->qsb_max;
	else
		return -NLE_NOATTR;
}

/**
 * Set the underflow increment of SFB qdisc.
 * 
 * @arg qdisc		SFB qdisc to be modified.
 * @arg increment	16-bit fixed point integer that represents the value by
 * which per-flow dropping probability is decreased on queue underflow.  This
 * should be a small fraction of a percent, and increment should be a few times
 * smaller than decrement.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_underflow_increment(struct rtnl_qdisc *qdisc, int increment)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_increment = increment;
	sfb->qsb_mask |= SCH_SFB_ATTR_INCREMENT;
}

/**
 * Get underflow increment of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Underflow increment in 16-bit fixed point, or a negative error code.
 */
int rtnl_sfb_get_underflow_increment(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_INCREMENT)
		return sfb->qsb_increment;
	else
		return -NLE_NOATTR;
}

/**
 * Set the overflow decrement of SFB qdisc.
 * 
 * @arg qdisc		SFB qdisc to be modified.
 * @arg decrement	16-bit fixed point integer that represents the value by
 * which per-flow dropping probability is increased on queue overflow.  This
 * should be a small fraction of a percent, and decrement should be a few times
 * larger than increment.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_overflow_decrement(struct rtnl_qdisc *qdisc, int decrement)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_decrement = decrement;
	sfb->qsb_mask |= SCH_SFB_ATTR_DECREMENT;
}

/**
 * Get overflow decrement of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Overflow decrement in 16-bit fixed point, or a negative error code.
 */
int rtnl_sfb_get_overflow_decrement(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_DECREMENT)
		return sfb->qsb_decrement;
	else
		return -NLE_NOATTR;
}

/**
 * Set the penalty rate of SFB qdisc.
 *
 * When a flow doesn't back off in response to congestion indication, sfb will
 * categorise it as ``non-reactive'' and will rate-limit it.  
 *
 * @arg qdisc		SFB qdisc to be modified.
 * @arg packets_per_sec	Total throughput that non-reactive flows are allowed to
 * use in packets per second.  You should set penalty_rate to some reasonable
 * fraction of your up-link throughput (the default values are probably too
 * small).
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_penalty_rate(struct rtnl_qdisc *qdisc, int packets_per_sec)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_penalty_rate = packets_per_sec;
	sfb->qsb_mask |= SCH_SFB_ATTR_PEN_RATE;
}

/**
 * Get the penalty rate of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Penalty rate in packets per second, or a negative error code.
 */
int rtnl_sfb_get_penalty_rate(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_PEN_RATE)
		return sfb->qsb_penalty_rate;
	else
		return -NLE_NOATTR;
}

/**
 * Set the penalty burst of SFB qdisc.
 *
 * When a flow doesn't back off in response to congestion indication, sfb will
 * categorise it as ``non-reactive'' and will rate-limit it.  
 *
 * @arg qdisc		SFB qdisc to be modified.
 * @arg burst_packets   Number of packets in the token bucket for penalty rate
 * limiting.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfb_set_penalty_burst(struct rtnl_qdisc *qdisc, int burst_packets)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfb->qsb_penalty_burst = burst_packets;
	sfb->qsb_mask |= SCH_SFB_ATTR_PEN_BURST;
}

/**
 * Get the penalty burst of SFB qdisc.
 * @arg qdisc		SFB qdisc.
 * @return Penalty burst size in packets, or a negative error code.
 */
int rtnl_sfb_get_penalty_burst(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfb *sfb;
	
	if (!(sfb = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfb->qsb_mask & SCH_SFB_ATTR_PEN_RATE)
		return sfb->qsb_penalty_burst;
	else
		return -NLE_NOATTR;
}

/** @} */

static struct rtnl_tc_ops sfb_ops = {
	.to_kind		= "sfb",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_sfb),
	.to_msg_parser		= sfb_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= sfb_dump_line,
	    [NL_DUMP_DETAILS]	= sfb_dump_details,
	},
	.to_msg_fill		= sfb_msg_fill,
};

static void __init sfb_init(void)
{
	rtnl_tc_register(&sfb_ops);
}

static void __exit sfb_exit(void)
{
	rtnl_tc_unregister(&sfb_ops);
}

/** @} */
