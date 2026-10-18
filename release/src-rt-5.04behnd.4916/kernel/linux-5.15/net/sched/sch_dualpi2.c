// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2019 Nokia.
 *
 * Author: Koen De Schepper <koen.de_schepper@nokia-bell-labs.com>
 * Author: Olga Albisser <olga@albisser.org>
 * Author: Henrik Steen <henrist@henrist.net>
 * Author: Olivier Tilmans <olivier.tilmans@nokia-bell-labs.com>
 *
 * DualPI Improved with a Square (dualpi2):
 * - Supports congestion controls that comply with the Prague requirements
 *   in RFC9331 (e.g. TCP-Prague)
 * - Supports coupled dual-queue with PI2 as defined in RFC9332
 * - Supports ECN L4S-identifier (IP.ECN==0b*1)
 *
 * note: DCTCP is not Prague compliant, so DCTCP & DualPI2 can only be
 *   used in DC context; BBRv3 (overwrites bbr) stopped Prague support,
 *   you should use TCP-Prague instead for low latency apps
 *
 * References:
 * - RFC9332: https://datatracker.ietf.org/doc/html/rfc9332
 * - De Schepper, Koen, et al. "PI 2: A linearized AQM for both classic and
 *   scalable TCP."  in proc. ACM CoNEXT'16, 2016.
 */

#if defined(CONFIG_BCM_KF_QDISC_DUALPI2)
#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/if_vlan.h>

#include <net/inet_ecn.h>
#include <net/pkt_sched.h>
#include <net/pkt_cls.h>

/* 32b enable to support flows with windows up to ~8.6 * 1e9 packets
 * i.e., twice the maximal snd_cwnd.
 * MAX_PROB must be consistent with the RNG in dualpi2_roll().
 */
#define MAX_PROB ((u32)(~((u32)0)))
/* alpha/beta values exchanged over netlink are in units of 256ns */
#define ALPHA_BETA_SHIFT 8
/* Scaled values of alpha/beta must fit in 32b to avoid overflow in later
 * computations. Consequently (see and dualpi2_scale_alpha_beta()), their
 * netlink-provided values can use at most 31b, i.e. be at most (2^23)-1
 * (~4MHz) as those are given in 1/256th. This enable to tune alpha/beta to
 * control flows whose maximal RTTs can be in usec up to few secs.
 */
#define ALPHA_BETA_MAX ((1U << 31) - 1)
/* Internal alpha/beta are in units of 64ns.
 * This enables to use all alpha/beta values in the allowed range without loss
 * of precision due to rounding when scaling them internally, e.g.,
 * scale_alpha_beta(1) will not round down to 0.
 */
#define ALPHA_BETA_GRANULARITY 6
#define ALPHA_BETA_SCALING (ALPHA_BETA_SHIFT - ALPHA_BETA_GRANULARITY)
/* We express the weights (wc, wl) in %, i.e., wc + wl = 100 */
#define MAX_WC 100

struct dualpi2_sched_data {
	struct Qdisc *l_queue;	/* The L4S LL queue */
	struct Qdisc *sch;	/* The classic queue (owner of this struct) */

	/* Registered tc filters */
	struct {
		struct tcf_proto __rcu *filters;
		struct tcf_block *block;
	} tcf;

	struct { /* PI2 parameters */
		u64	target;	/* Target delay in nanoseconds */
		u32	tupdate;/* Timer frequency in nanoseconds */
		u32	prob;	/* Base PI probability */
		u32	alpha;	/* Gain factor for the integral rate response */
		u32	beta;	/* Gain factor for the proportional response */
		struct hrtimer timer; /* prob update timer */
	} pi2;

	struct { /* Step AQM (L4S queue only) parameters */
		u32 thresh;	/* Step threshold */
		bool in_packets;/* Whether the step is in packets or time */
	} step;

	struct { /* Classic queue starvation protection */
		s32	credit; /* Credit (sign indicates which queue) */
		s32	init;	/* Reset value of the credit */
		u8	wc;	/* C queue weight (between 0 and MAX_WC) */
		u8	wl;	/* L queue weight (MAX_WC - wc) */
	} c_protection;

	/* General dualQ parameters */
	u8	coupling_factor;/* Coupling factor (k) between both queues */
	u8	ecn_mask;	/* Mask to match L4S packets */
	bool	drop_early;	/* Drop at enqueue instead of dequeue if true */
	bool	drop_overload;	/* Drop (1) on overload, or overflow (0) */
	bool	split_gso;	/* Split aggregated skb (1) or leave as is */

	/* Statistics */
	u64	c_head_ts;	/* Enqueue timestamp of the classic Q's head */
	u64	l_head_ts;	/* Enqueue timestamp of the L Q's head */
	u64	last_qdelay;	/* Q delay val at the last probability update */
	u32	packets_in_c;	/* Number of packets enqueued in C queue */
	u32	packets_in_l;	/* Number of packets enqueued in L queue */
	u32	maxq;		/* maximum queue size */
	u32	ecn_mark;	/* packets marked with ECN */
	u32	step_marks;	/* ECN marks due to the step AQM */

	struct { /* Deferred drop statistics */
		u32 cnt;	/* Packets dropped */
		u32 len;	/* Bytes dropped */
	} deferred_drops;

	bool	offload;	/* Split aggregated skb (1) or leave as is */
};

struct dualpi2_skb_cb {
	u64 ts;			/* Timestamp at enqueue */
	u8 apply_step:1,	/* Can we apply the step threshold */
	   classified:2,	/* Packet classification results */
	   ect:2;		/* Packet ECT codepoint */
};

enum dualpi2_classification_results {
	DUALPI2_C_CLASSIC	= 0,	/* C queue */
	DUALPI2_C_L4S		= 1,	/* L queue (scalable marking/classic drops) */
	DUALPI2_C_LLLL		= 2,	/* L queue (no drops/marks) */

	__DUALPI2_C_MAX /* Keep last*/
};

static struct dualpi2_skb_cb *dualpi2_skb_cb(struct sk_buff *skb)
{
	qdisc_cb_private_validate(skb, sizeof(struct dualpi2_skb_cb));
	return (struct dualpi2_skb_cb *)qdisc_skb_cb(skb)->data;
}

static u64 skb_sojourn_time(struct sk_buff *skb, u64 reference)
{
	return reference - dualpi2_skb_cb(skb)->ts;
}

static u64 head_enqueue_time(struct Qdisc *q)
{
	struct sk_buff *skb = qdisc_peek_head(q);

	return skb ? dualpi2_skb_cb(skb)->ts : 0;
}

static u32 dualpi2_scale_alpha_beta(u32 param)
{
	u64 tmp  = ((u64)param * MAX_PROB >> ALPHA_BETA_SCALING);

	do_div(tmp, NSEC_PER_SEC);
	return tmp;
}

static u32 dualpi2_unscale_alpha_beta(u32 param)
{
	u64 tmp = ((u64)param * NSEC_PER_SEC << ALPHA_BETA_SCALING);

	do_div(tmp, MAX_PROB);
	return tmp;
}

static ktime_t next_pi2_timeout(struct dualpi2_sched_data *q)
{
	return ktime_add_ns(ktime_get_ns(), q->pi2.tupdate);
}

static bool skb_is_l4s(struct sk_buff *skb)
{
	return dualpi2_skb_cb(skb)->classified == DUALPI2_C_L4S;
}

static bool skb_in_l_queue(struct sk_buff *skb)
{
	return dualpi2_skb_cb(skb)->classified != DUALPI2_C_CLASSIC;
}

static bool dualpi2_mark(struct dualpi2_sched_data *q, struct sk_buff *skb)
{
	if (INET_ECN_set_ce(skb)) {
		q->ecn_mark++;
		return true;
	}
	return false;
}

static void dualpi2_reset_c_protection(struct dualpi2_sched_data *q)
{
	q->c_protection.credit = q->c_protection.init;
}

/* This computes the initial credit value and WRR weight for the L queue (wl)
 * from the weight of the C queue (wc).
 * If wl > wc, the scheduler will start with the L queue when reset.
 */
static void dualpi2_calculate_c_protection(struct Qdisc *sch,
					   struct dualpi2_sched_data *q, u32 wc)
{
	q->c_protection.wc = wc;
	q->c_protection.wl = MAX_WC - wc;
	q->c_protection.init = (s32)psched_mtu(qdisc_dev(sch)) *
		((int)q->c_protection.wc - (int)q->c_protection.wl);
	dualpi2_reset_c_protection(q);
}

static bool dualpi2_roll(u32 prob)
{
	return prandom_u32() <= prob;
}

/* Packets in the C queue are subject to a marking probability pC, which is the
 * square of the internal PI2 probability (i.e., have an overall lower mark/drop
 * probability). If the qdisc is overloaded, ignore ECT values and only drop.
 *
 * Note that this marking scheme is also applied to L4S packets during overload.
 */
static bool dualpi2_classic_marking(struct dualpi2_sched_data *q,
				    struct sk_buff *skb, u32 prob,
				    bool overload)
{
	if (dualpi2_roll(prob) && dualpi2_roll(prob)) {
		if (overload || dualpi2_skb_cb(skb)->ect == INET_ECN_NOT_ECT)
			return true;
		dualpi2_mark(q, skb);
	}
	return false;
}

/* Packets in the L queue are subject to a marking probability pL given by the
 * internal PI2 probability scaled by the coupling factor.
 *
 * On overload (i.e., @local_l_prob is >= 100%):
 * - if the qdisc is configured to trade losses to preserve latency (i.e.,
 *   @q->drop_overload), apply classic drops first before marking.
 * - otherwise, preserve the "no loss" property of ECN at the cost of queueing
 *   delay, eventually resulting in taildrop behavior once sch->limit is
 *   reached.
 */
static bool dualpi2_scalable_marking(struct dualpi2_sched_data *q,
				     struct sk_buff *skb,
				     u64 local_l_prob, u32 prob,
				     bool overload)
{
	if (overload) {
		/* Apply classic drop */
		if (!q->drop_overload ||
		    !(dualpi2_roll(prob) && dualpi2_roll(prob)))
			goto mark;
		return true;
	}

	/* We can safely cut the upper 32b as overload==false*/
	if (dualpi2_roll(local_l_prob)) {
		/* Non-ECT packets could have classified as L4S by filters. */
		if (dualpi2_skb_cb(skb)->ect == INET_ECN_NOT_ECT)
			return true;
mark:
		dualpi2_mark(q, skb);
	}
	return false;
}

/* Decide whether a given packet must be dropped (or marked if ECT), according
 * to the PI2 probability.
 *
 * Never mark/drop if we have a standing queue of less than 2 MTUs.
 */
static bool must_drop(struct Qdisc *sch, struct dualpi2_sched_data *q,
		      struct sk_buff *skb)
{
	u64 local_l_prob;
	u32 prob;
	bool overload;

	if (sch->qstats.backlog < 2 * psched_mtu(qdisc_dev(sch)))
		return false;

	prob = READ_ONCE(q->pi2.prob);
	local_l_prob = (u64)prob * q->coupling_factor;
	overload = local_l_prob > MAX_PROB;

	switch (dualpi2_skb_cb(skb)->classified) {
	case DUALPI2_C_CLASSIC:
		return dualpi2_classic_marking(q, skb, prob, overload);
	case DUALPI2_C_L4S:
		return dualpi2_scalable_marking(q, skb, local_l_prob, prob,
						overload);
	default: /* DUALPI2_C_LLLL */
		return false;
	}
}

static void dualpi2_read_ect(struct sk_buff *skb)
{
	struct dualpi2_skb_cb *cb = dualpi2_skb_cb(skb);
	int wlen = skb_network_offset(skb);

	switch (skb_protocol(skb, true)) {
	case htons(ETH_P_IP):
		wlen += sizeof(struct iphdr);
		if (!pskb_may_pull(skb, wlen) ||
		    skb_try_make_writable(skb, wlen))
			goto not_ecn;

		cb->ect = ipv4_get_dsfield(ip_hdr(skb)) & INET_ECN_MASK;
		break;
	case htons(ETH_P_IPV6):
		wlen += sizeof(struct ipv6hdr);
		if (!pskb_may_pull(skb, wlen) ||
		    skb_try_make_writable(skb, wlen))
			goto not_ecn;

		cb->ect = ipv6_get_dsfield(ipv6_hdr(skb)) & INET_ECN_MASK;
		break;
	default:
		goto not_ecn;
	}
	return;

not_ecn:
	/* Non pullable/writable packets can only be dropped hence are
	 * classified as not ECT.
	 */
	cb->ect = INET_ECN_NOT_ECT;
}

static int dualpi2_skb_classify(struct dualpi2_sched_data *q,
				struct sk_buff *skb)
{
	struct dualpi2_skb_cb *cb = dualpi2_skb_cb(skb);
	struct tcf_result res;
	struct tcf_proto *fl;
	int result;

	dualpi2_read_ect(skb);
	if (cb->ect & q->ecn_mask) {
		cb->classified = DUALPI2_C_L4S;
		return NET_XMIT_SUCCESS;
	}

	if (TC_H_MAJ(skb->priority) == q->sch->handle &&
	    TC_H_MIN(skb->priority) < __DUALPI2_C_MAX) {
		cb->classified = TC_H_MIN(skb->priority);
		return NET_XMIT_SUCCESS;
	}

	fl = rcu_dereference_bh(q->tcf.filters);
	if (!fl) {
		cb->classified = DUALPI2_C_CLASSIC;
		return NET_XMIT_SUCCESS;
	}

	result = tcf_classify(skb, NULL, fl, &res, false);
	if (result >= 0) {
#ifdef CONFIG_NET_CLS_ACT
		switch (result) {
		case TC_ACT_STOLEN:
		case TC_ACT_QUEUED:
		case TC_ACT_TRAP:
			return NET_XMIT_SUCCESS | __NET_XMIT_STOLEN;
		case TC_ACT_SHOT:
			return NET_XMIT_SUCCESS | __NET_XMIT_BYPASS;
		}
#endif
		cb->classified = TC_H_MIN(res.classid) < __DUALPI2_C_MAX ?
			TC_H_MIN(res.classid) : DUALPI2_C_CLASSIC;
	}
	return NET_XMIT_SUCCESS;
}

static int dualpi2_enqueue_skb(struct sk_buff *skb, struct Qdisc *sch,
			       struct sk_buff **to_free)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);
	struct dualpi2_skb_cb *cb;

	if (unlikely(qdisc_qlen(sch) >= sch->limit)) {
		qdisc_qstats_overlimit(sch);
		if (skb_in_l_queue(skb))
			qdisc_qstats_overlimit(q->l_queue);
		return qdisc_drop(skb, sch, to_free);
	}

	if (q->drop_early && must_drop(sch, q, skb)) {
		qdisc_drop(skb, sch, to_free);
		return NET_XMIT_SUCCESS | __NET_XMIT_BYPASS;
	}

	cb = dualpi2_skb_cb(skb);
	cb->ts = ktime_get_ns();

	if (qdisc_qlen(sch) > q->maxq)
		q->maxq = qdisc_qlen(sch);

	if (skb_in_l_queue(skb)) {
		/* Only apply the step if a queue is building up */
		dualpi2_skb_cb(skb)->apply_step =
			skb_is_l4s(skb) && qdisc_qlen(q->l_queue) > 1;
		/* Keep the overall qdisc stats consistent */
		++sch->q.qlen;
		qdisc_qstats_backlog_inc(sch, skb);
		++q->packets_in_l;
		if (!q->l_head_ts)
			q->l_head_ts = cb->ts;
		return qdisc_enqueue_tail(skb, q->l_queue);
	}
	++q->packets_in_c;
	if (!q->c_head_ts)
		q->c_head_ts = cb->ts;
	return qdisc_enqueue_tail(skb, sch);
}

/* Optionally, dualpi2 will split GSO skbs into independent skbs and enqueue
 * each of those individually. This yields the following benefits, at the
 * expense of CPU usage:
 * - Finer-grained AQM actions as the sub-packets of a burst no longer share the
 *   same fate (e.g., the random mark/drop probability is applied individually)
 * - Improved precision of the starvation protection/WRR scheduler at dequeue,
 *   as the size of the dequeued packets will be smaller.
 */
static int dualpi2_qdisc_enqueue(struct sk_buff *skb, struct Qdisc *sch,
				 struct sk_buff **to_free)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);
	int err;

	err = dualpi2_skb_classify(q, skb);
	if (err != NET_XMIT_SUCCESS) {
		if (err & __NET_XMIT_BYPASS)
			qdisc_qstats_drop(sch);
		__qdisc_drop(skb, to_free);
		return err;
	}

	if (q->split_gso && skb_is_gso(skb)) {
		netdev_features_t features;
		struct sk_buff *nskb, *next;
		int cnt, byte_len, orig_len;
		int err;

		features = netif_skb_features(skb);
		nskb = skb_gso_segment(skb, features & ~NETIF_F_GSO_MASK);
		if (IS_ERR_OR_NULL(nskb))
			return qdisc_drop(skb, sch, to_free);

		cnt = 1;
		byte_len = 0;
		orig_len = qdisc_pkt_len(skb);
		while (nskb) {
			next = nskb->next;
			skb_mark_not_on_list(nskb);
			qdisc_skb_cb(nskb)->pkt_len = nskb->len;
			dualpi2_skb_cb(nskb)->classified =
				dualpi2_skb_cb(skb)->classified;
			dualpi2_skb_cb(nskb)->ect = dualpi2_skb_cb(skb)->ect;
			err = dualpi2_enqueue_skb(nskb, sch, to_free);
			if (err == NET_XMIT_SUCCESS) {
				/* Compute the backlog adjustement that needs
				 * to be propagated in the qdisc tree to reflect
				 * all new skbs successfully enqueued.
				 */
				++cnt;
				byte_len += nskb->len;
			}
			nskb = next;
		}
		if (err == NET_XMIT_SUCCESS) {
			/* The caller will add the original skb stats to its
			 * backlog, compensate this.
			 */
			--cnt;
			byte_len -= orig_len;
		}
		qdisc_tree_reduce_backlog(sch, -cnt, -byte_len);
		consume_skb(skb);
		return err;
	}
	return dualpi2_enqueue_skb(skb, sch, to_free);
}

/* Select the queue from which the next packet can be dequeued, ensuring that
 * neither queue can starve the other with a WRR scheduler.
 *
 * The sign of the WRR credit determines the next queue, while the size of
 * the dequeued packet determines the magnitude of the WRR credit change. If
 * either queue is empty, the WRR credit is kept unchanged.
 *
 * As the dequeued packet can be dropped later, the caller has to perform the
 * qdisc_bstats_update() calls.
 */
static struct sk_buff *dequeue_packet(struct Qdisc *sch,
				      struct dualpi2_sched_data *q,
				      int *credit_change,
				      u64 now)
{
	struct sk_buff *skb = NULL;
	int c_len;

	*credit_change = 0;
	c_len = qdisc_qlen(sch) - qdisc_qlen(q->l_queue);
	if (qdisc_qlen(q->l_queue) && (!c_len || q->c_protection.credit <= 0)) {
		skb = __qdisc_dequeue_head(&q->l_queue->q);
		WRITE_ONCE(q->l_head_ts, head_enqueue_time(q->l_queue));
		if (c_len)
			*credit_change = q->c_protection.wc;
		qdisc_qstats_backlog_dec(q->l_queue, skb);
		/* Keep the global queue size consistent */
		--sch->q.qlen;
	} else if (c_len) {
		skb = __qdisc_dequeue_head(&sch->q);
		WRITE_ONCE(q->c_head_ts, head_enqueue_time(sch));
		if (qdisc_qlen(q->l_queue))
			*credit_change = (s32)(-1) * q->c_protection.wl;
	} else {
		dualpi2_reset_c_protection(q);
		return NULL;
	}
	*credit_change *= qdisc_pkt_len(skb);
	qdisc_qstats_backlog_dec(sch, skb);
	return skb;
}

static int do_step_aqm(struct dualpi2_sched_data *q, struct sk_buff *skb,
		       u64 now)
{
	u64 qdelay = 0;

	if (q->step.in_packets)
		qdelay = qdisc_qlen(q->l_queue);
	else
		qdelay = skb_sojourn_time(skb, now);

	if (dualpi2_skb_cb(skb)->apply_step && qdelay > q->step.thresh) {
		if (!dualpi2_skb_cb(skb)->ect)
			/* Drop this non-ECT packet */
			return 1;
		if (dualpi2_mark(q, skb))
			++q->step_marks;
	}
	qdisc_bstats_update(q->l_queue, skb);
	return 0;
}

static struct sk_buff *dualpi2_qdisc_dequeue(struct Qdisc *sch)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);
	struct sk_buff *skb;
	int credit_change;
	u64 now;

	now = ktime_get_ns();

pick_packet:
	skb = dequeue_packet(sch, q, &credit_change, now);
	if (!skb)
		goto exit;

	if (!q->drop_early && must_drop(sch, q, skb))
		goto drop_and_retry;

	if (skb_in_l_queue(skb) && do_step_aqm(q, skb, now)) {
		qdisc_qstats_drop(q->l_queue);
		goto drop_and_retry;
	}

	q->c_protection.credit += credit_change;
	qdisc_bstats_update(sch, skb);

exit:
	/* We cannot call qdisc_tree_reduce_backlog() if our qlen is 0,
	 * or HTB crashes.
	 */
	if (q->deferred_drops.cnt && qdisc_qlen(sch)) {
		qdisc_tree_reduce_backlog(sch, q->deferred_drops.cnt,
					  q->deferred_drops.len);
		q->deferred_drops.cnt = 0;
		q->deferred_drops.len = 0;
	}
	return skb;

drop_and_retry:
	++q->deferred_drops.cnt;
	q->deferred_drops.len += qdisc_pkt_len(skb);
	consume_skb(skb);
	qdisc_qstats_drop(sch);
	goto pick_packet;
}

static s64 __scale_delta(u64 diff)
{
	do_div(diff, 1 << ALPHA_BETA_GRANULARITY);
	return diff;
}

static void get_queue_delays(struct dualpi2_sched_data *q, u64 *qdelay_c,
			     u64 *qdelay_l)
{
	u64 now, qc, ql;

	now = ktime_get_ns();
	qc = READ_ONCE(q->c_head_ts);
	ql = READ_ONCE(q->l_head_ts);

	*qdelay_c = qc ? now - qc : 0;
	*qdelay_l = ql ? now - ql : 0;
}

static u32 calculate_probability(struct Qdisc *sch)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);
	u32 new_prob;
	u64 qdelay_c;
	u64 qdelay_l;
	u64 qdelay;
	s64 delta;

	get_queue_delays(q, &qdelay_c, &qdelay_l);
	qdelay = max(qdelay_l, qdelay_c);
	/* Alpha and beta take at most 32b, i.e, the delay difference would
	 * overflow for queuing delay differences > ~4.2sec.
	 */
	delta = ((s64)qdelay - q->pi2.target) * q->pi2.alpha;
	delta += ((s64)qdelay - q->last_qdelay) * q->pi2.beta;
	if (delta > 0) {
		new_prob = __scale_delta(delta) + q->pi2.prob;
		if (new_prob < q->pi2.prob)
			new_prob = MAX_PROB;
	} else {
		new_prob = q->pi2.prob - __scale_delta(delta * -1);
		if (new_prob > q->pi2.prob)
			new_prob = 0;
	}
	q->last_qdelay = qdelay;
	/* If we do not drop on overload, ensure we cap the L4S probability to
	 * 100% to keep window fairness when overflowing.
	 */
	if (!q->drop_overload)
		return min_t(u32, new_prob, MAX_PROB / q->coupling_factor);
	return new_prob;
}

static enum hrtimer_restart dualpi2_timer(struct hrtimer *timer)
{
	struct dualpi2_sched_data *q = from_timer(q, timer, pi2.timer);

	WRITE_ONCE(q->pi2.prob, calculate_probability(q->sch));

	hrtimer_set_expires(&q->pi2.timer, next_pi2_timeout(q));
	return HRTIMER_RESTART;
}

static const struct nla_policy dualpi2_policy[TCA_DUALPI2_MAX + 1] = {
	[TCA_DUALPI2_LIMIT] = {.type = NLA_U32},
	[TCA_DUALPI2_TARGET] = {.type = NLA_U32},
	[TCA_DUALPI2_TUPDATE] = {.type = NLA_U32},
	[TCA_DUALPI2_ALPHA] = {.type = NLA_U32},
	[TCA_DUALPI2_BETA] = {.type = NLA_U32},
	[TCA_DUALPI2_STEP_THRESH] = {.type = NLA_U32},
	[TCA_DUALPI2_STEP_PACKETS] = {.type = NLA_U8},
	[TCA_DUALPI2_COUPLING] = {.type = NLA_U8},
	[TCA_DUALPI2_DROP_OVERLOAD] = {.type = NLA_U8},
	[TCA_DUALPI2_DROP_EARLY] = {.type = NLA_U8},
	[TCA_DUALPI2_C_PROTECTION] = {.type = NLA_U8},
	[TCA_DUALPI2_ECN_MASK] = {.type = NLA_U8},
	[TCA_DUALPI2_SPLIT_GSO] = {.type = NLA_U8},
};

static int dualpi2_change(struct Qdisc *sch, struct nlattr *opt,
			  struct netlink_ext_ack *extack)
{
	struct nlattr *tb[TCA_DUALPI2_MAX + 1];
	struct dualpi2_sched_data *q;
	int old_backlog;
	int old_qlen;
	int err;

	if (!opt)
		return -EINVAL;
	err = nla_parse_nested_deprecated(tb, TCA_DUALPI2_MAX, opt,
					  dualpi2_policy, extack);
	if (err < 0)
		return err;

	q = qdisc_priv(sch);
	sch_tree_lock(sch);

	if (tb[TCA_DUALPI2_LIMIT]) {
		u32 limit = nla_get_u32(tb[TCA_DUALPI2_LIMIT]);

		if (!limit) {
			NL_SET_ERR_MSG_ATTR(extack, tb[TCA_DUALPI2_LIMIT],
					    "limit must be greater than 0 !");
			return -EINVAL;
		}
		sch->limit = limit;
	}

	if (tb[TCA_DUALPI2_TARGET])
		q->pi2.target = (u64)nla_get_u32(tb[TCA_DUALPI2_TARGET]) *
			NSEC_PER_USEC;

	if (tb[TCA_DUALPI2_TUPDATE]) {
		u64 tupdate = nla_get_u32(tb[TCA_DUALPI2_TUPDATE]);

		if (!tupdate) {
			NL_SET_ERR_MSG_ATTR(extack, tb[TCA_DUALPI2_TUPDATE],
					    "tupdate cannot be 0us!");
			return -EINVAL;
		}
		q->pi2.tupdate = tupdate * NSEC_PER_USEC;
	}

	if (tb[TCA_DUALPI2_ALPHA]) {
		u32 alpha = nla_get_u32(tb[TCA_DUALPI2_ALPHA]);

		if (alpha > ALPHA_BETA_MAX) {
			NL_SET_ERR_MSG_ATTR(extack, tb[TCA_DUALPI2_ALPHA],
					    "alpha is too large!");
			return -EINVAL;
		}
		q->pi2.alpha = dualpi2_scale_alpha_beta(alpha);
	}

	if (tb[TCA_DUALPI2_BETA]) {
		u32 beta = nla_get_u32(tb[TCA_DUALPI2_BETA]);

		if (beta > ALPHA_BETA_MAX) {
			NL_SET_ERR_MSG_ATTR(extack, tb[TCA_DUALPI2_BETA],
					    "beta is too large!");
			return -EINVAL;
		}
		q->pi2.beta = dualpi2_scale_alpha_beta(beta);
	}

	if (tb[TCA_DUALPI2_STEP_THRESH])
		q->step.thresh = nla_get_u32(tb[TCA_DUALPI2_STEP_THRESH]) *
			NSEC_PER_USEC;

	if (tb[TCA_DUALPI2_COUPLING]) {
		u8 coupling = nla_get_u8(tb[TCA_DUALPI2_COUPLING]);

		if (!coupling) {
			NL_SET_ERR_MSG_ATTR(extack, tb[TCA_DUALPI2_COUPLING],
					    "Must use a non-zero coupling!");
			return -EINVAL;
		}
		q->coupling_factor = coupling;
	}

	if (tb[TCA_DUALPI2_STEP_PACKETS])
		q->step.in_packets = !!nla_get_u8(tb[TCA_DUALPI2_STEP_PACKETS]);

	if (tb[TCA_DUALPI2_DROP_OVERLOAD])
		q->drop_overload = !!nla_get_u8(tb[TCA_DUALPI2_DROP_OVERLOAD]);

	if (tb[TCA_DUALPI2_DROP_EARLY])
		q->drop_early = !!nla_get_u8(tb[TCA_DUALPI2_DROP_EARLY]);

	if (tb[TCA_DUALPI2_C_PROTECTION]) {
		u8 wc = nla_get_u8(tb[TCA_DUALPI2_C_PROTECTION]);

		if (wc > MAX_WC) {
			NL_SET_ERR_MSG_ATTR(extack,
					    tb[TCA_DUALPI2_C_PROTECTION],
					    "c_protection must be <= 100!");
			return -EINVAL;
		}
		dualpi2_calculate_c_protection(sch, q, wc);
	}

	if (tb[TCA_DUALPI2_ECN_MASK])
		q->ecn_mask = nla_get_u8(tb[TCA_DUALPI2_ECN_MASK]);

	if (tb[TCA_DUALPI2_SPLIT_GSO])
		q->split_gso = !!nla_get_u8(tb[TCA_DUALPI2_SPLIT_GSO]);

	old_qlen = qdisc_qlen(sch);
	old_backlog = sch->qstats.backlog;
	while (qdisc_qlen(sch) > sch->limit) {
		struct sk_buff *skb = __qdisc_dequeue_head(&sch->q);

		qdisc_qstats_backlog_dec(sch, skb);
		rtnl_qdisc_drop(skb, sch);
	}
	qdisc_tree_reduce_backlog(sch, old_qlen - qdisc_qlen(sch),
				  old_backlog - sch->qstats.backlog);

	sch_tree_unlock(sch);
	return 0;
}

/* Default alpha/beta values give a 10dB stability margin with max_rtt=100ms. */
static void dualpi2_reset_default(struct dualpi2_sched_data *q)
{
	q->sch->limit = 10000;				/* Max 125ms at 1Gbps */

	q->pi2.target = 15 * NSEC_PER_MSEC;
	q->pi2.tupdate = 16 * NSEC_PER_MSEC;
	q->pi2.alpha = dualpi2_scale_alpha_beta(41);	/* ~0.16 Hz * 256 */
	q->pi2.beta = dualpi2_scale_alpha_beta(819);	/* ~3.20 Hz * 256 */

	q->step.thresh = 1 * NSEC_PER_MSEC;
	q->step.in_packets = false;

	dualpi2_calculate_c_protection(q->sch, q, 10);	/* wc=10%, wl=90% */

	q->ecn_mask = INET_ECN_ECT_1;
	q->coupling_factor = 2;		/* window fairness for equal RTTs */
	q->drop_overload = true;	/* Preserve latency by dropping */
	q->drop_early = false;		/* PI2 drops on dequeue */
	q->split_gso = true;
}

static int dualpi2_offload(struct Qdisc *sch, u32 parentid, u32 classid,
			   enum tc_pon_command command)
{
	struct tc_dualp2_qopt_offload qopt = {0};
	struct net_device *dev = qdisc_dev(sch);

	if (!tc_can_offload(dev) || !dev->netdev_ops->ndo_setup_tc)
		return -EINVAL;

	qopt.parent = parentid;
	qopt.handle = classid;
	qopt.command = command;

	return dev->netdev_ops->ndo_setup_tc(dev, TC_SETUP_QDISC_DUALPI2, &qopt);
}

static int dualpi2_init(struct Qdisc *sch, struct nlattr *opt,
			struct netlink_ext_ack *extack)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);
	int err;

	q->l_queue = qdisc_create_dflt(sch->dev_queue, &pfifo_qdisc_ops,
				       TC_H_MAKE(sch->handle, 1), extack);
	if (!q->l_queue)
		return -ENOMEM;

	err = tcf_block_get(&q->tcf.block, &q->tcf.filters, sch, extack);
	if (err)
		return err;

	q->sch = sch;
	dualpi2_reset_default(q);

	if (opt) {
		err = dualpi2_change(sch, opt, extack);

		if (err)
			return err;
	}

	if (!dualpi2_offload(sch, sch->parent, sch->handle, TC_DUALPI2_REPLACE)) {
		q->offload = true;
		return 0;
	}

	hrtimer_init(&q->pi2.timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED);
	q->pi2.timer.function = dualpi2_timer;
	hrtimer_start(&q->pi2.timer, next_pi2_timeout(q),
		      HRTIMER_MODE_ABS_PINNED);
	return 0;
}

static u32 convert_ns_to_usec(u64 ns)
{
	do_div(ns, NSEC_PER_USEC);
	return ns;
}

static int dualpi2_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts;

	opts = nla_nest_start_noflag(skb, TCA_OPTIONS);
	if (!opts)
		goto nla_put_failure;

	if (nla_put_u32(skb, TCA_DUALPI2_LIMIT, sch->limit) ||
	    nla_put_u32(skb, TCA_DUALPI2_TARGET,
			convert_ns_to_usec(q->pi2.target)) ||
	    nla_put_u32(skb, TCA_DUALPI2_TUPDATE,
			convert_ns_to_usec(q->pi2.tupdate)) ||
	    nla_put_u32(skb, TCA_DUALPI2_ALPHA,
			dualpi2_unscale_alpha_beta(q->pi2.alpha)) ||
	    nla_put_u32(skb, TCA_DUALPI2_BETA,
			dualpi2_unscale_alpha_beta(q->pi2.beta)) ||
	    nla_put_u32(skb, TCA_DUALPI2_STEP_THRESH, q->step.in_packets ?
			q->step.thresh : convert_ns_to_usec(q->step.thresh)) ||
	    nla_put_u8(skb, TCA_DUALPI2_COUPLING, q->coupling_factor) ||
	    nla_put_u8(skb, TCA_DUALPI2_DROP_OVERLOAD, q->drop_overload) ||
	    nla_put_u8(skb, TCA_DUALPI2_STEP_PACKETS, q->step.in_packets) ||
	    nla_put_u8(skb, TCA_DUALPI2_DROP_EARLY, q->drop_early) ||
	    nla_put_u8(skb, TCA_DUALPI2_C_PROTECTION, q->c_protection.wc) ||
	    nla_put_u8(skb, TCA_DUALPI2_ECN_MASK, q->ecn_mask) ||
	    nla_put_u8(skb, TCA_DUALPI2_SPLIT_GSO, q->split_gso))
		goto nla_put_failure;

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -1;
}

static int dualpi2_dump_stats(struct Qdisc *sch, struct gnet_dump *d)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);
	struct tc_dualpi2_xstats st = {
		.prob		= READ_ONCE(q->pi2.prob),
		.packets_in_c	= q->packets_in_c,
		.packets_in_l	= q->packets_in_l,
		.maxq		= q->maxq,
		.ecn_mark	= q->ecn_mark,
		.credit		= q->c_protection.credit,
		.step_marks	= q->step_marks,
	};
	u64 qc, ql;

	get_queue_delays(q, &qc, &ql);
	st.delay_l = convert_ns_to_usec(ql);
	st.delay_c = convert_ns_to_usec(qc);
	return gnet_stats_copy_app(d, &st, sizeof(st));
}

static void dualpi2_reset(struct Qdisc *sch)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);

	qdisc_reset_queue(sch);
	qdisc_reset_queue(q->l_queue);
	q->c_head_ts = 0;
	q->l_head_ts = 0;
	q->pi2.prob = 0;
	q->packets_in_c = 0;
	q->packets_in_l = 0;
	q->maxq = 0;
	q->ecn_mark = 0;
	q->step_marks = 0;
	dualpi2_reset_c_protection(q);
}

static void dualpi2_destroy(struct Qdisc *sch)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);

	dualpi2_offload(sch, sch->parent, sch->handle, TC_DUALPI2_DESTROY);

	q->pi2.tupdate = 0;
	if (!q->offload)
		hrtimer_cancel(&q->pi2.timer);
	if (q->l_queue)
		qdisc_put(q->l_queue);
	tcf_block_put(q->tcf.block);
}

static struct Qdisc *dualpi2_leaf(struct Qdisc *sch, unsigned long arg)
{
	return NULL;
}

static unsigned long dualpi2_find(struct Qdisc *sch, u32 classid)
{
	return 0;
}

static unsigned long dualpi2_bind(struct Qdisc *sch, unsigned long parent,
				  u32 classid)
{
	return 0;
}

static void dualpi2_unbind(struct Qdisc *q, unsigned long cl)
{
}

static struct tcf_block *dualpi2_tcf_block(struct Qdisc *sch, unsigned long cl,
					   struct netlink_ext_ack *extack)
{
	struct dualpi2_sched_data *q = qdisc_priv(sch);

	if (cl)
		return NULL;
	return q->tcf.block;
}

static void dualpi2_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	unsigned int i;

	if (arg->stop)
		return;

	/* We statically define only 2 queues */
	for (i = 0; i < 2; i++) {
		if (arg->count < arg->skip) {
			arg->count++;
			continue;
		}
		if (arg->fn(sch, i + 1, arg) < 0) {
			arg->stop = 1;
			break;
		}
		arg->count++;
	}
}

/* Minimal class support to handler tc filters */
static const struct Qdisc_class_ops dualpi2_class_ops = {
	.leaf		=	dualpi2_leaf,
	.find		=	dualpi2_find,
	.tcf_block	=	dualpi2_tcf_block,
	.bind_tcf	=	dualpi2_bind,
	.unbind_tcf	=	dualpi2_unbind,
	.walk		=	dualpi2_walk,
};

static struct Qdisc_ops dualpi2_qdisc_ops __read_mostly = {
	.id		= "dualpi2",
	.cl_ops		= &dualpi2_class_ops,
	.priv_size	= sizeof(struct dualpi2_sched_data),
	.enqueue	= dualpi2_qdisc_enqueue,
	.dequeue	= dualpi2_qdisc_dequeue,
	.peek		= qdisc_peek_dequeued,
	.init		= dualpi2_init,
	.destroy	= dualpi2_destroy,
	.reset		= dualpi2_reset,
	.change		= dualpi2_change,
	.dump		= dualpi2_dump,
	.dump_stats	= dualpi2_dump_stats,
	.owner		= THIS_MODULE,
};

static int __init dualpi2_module_init(void)
{
	return register_qdisc(&dualpi2_qdisc_ops);
}

static void __exit dualpi2_module_exit(void)
{
	unregister_qdisc(&dualpi2_qdisc_ops);
}

module_init(dualpi2_module_init);
module_exit(dualpi2_module_exit);

MODULE_DESCRIPTION("Dual Queue with Proportional Integral controller Improved with a Square (dualpi2) scheduler");
MODULE_AUTHOR("Koen De Schepper");
MODULE_AUTHOR("Olga Albisser");
MODULE_AUTHOR("Henrik Steen");
MODULE_AUTHOR("Olivier Tilmans");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
#endif
