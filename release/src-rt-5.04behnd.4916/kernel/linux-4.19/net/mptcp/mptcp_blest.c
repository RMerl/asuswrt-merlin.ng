#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
// SPDX-License-Identifier: GPL-2.0
/*	MPTCP Scheduler to reduce HoL-blocking and spurious retransmissions.
 *
 *	Algorithm Design:
 *	Simone Ferlin <ferlin@simula.no>
 *	Ozgu Alay <ozgu@simula.no>
 *	Olivier Mehani <olivier.mehani@nicta.com.au>
 *	Roksana Boreli <roksana.boreli@nicta.com.au>
 *
 *	Initial Implementation:
 *	Simone Ferlin <ferlin@simula.no>
 *
 *	Additional Authors:
 *	Daniel Weber <weberd@cs.uni-bonn.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <net/mptcp.h>
#include <trace/events/tcp.h>

static unsigned char lambda __read_mostly = 12;
module_param(lambda, byte, 0644);
MODULE_PARM_DESC(lambda, "Divided by 10 for scaling factor of fast flow rate estimation");

static unsigned char max_lambda __read_mostly = 13;
module_param(max_lambda, byte, 0644);
MODULE_PARM_DESC(max_lambda, "Divided by 10 for maximum scaling factor of fast flow rate estimation");

static unsigned char min_lambda __read_mostly = 10;
module_param(min_lambda, byte, 0644);
MODULE_PARM_DESC(min_lambda, "Divided by 10 for minimum scaling factor of fast flow rate estimation");

static unsigned char dyn_lambda_good = 10; /* 1% */
module_param(dyn_lambda_good, byte, 0644);
MODULE_PARM_DESC(dyn_lambda_good, "Decrease of lambda in positive case.");

static unsigned char dyn_lambda_bad = 40; /* 4% */
module_param(dyn_lambda_bad, byte, 0644);
MODULE_PARM_DESC(dyn_lambda_bad, "Increase of lambda in negative case.");

struct blestsched_priv {
	u32 last_rbuf_opti;
	u32 min_srtt_us;
	u32 max_srtt_us;
};

struct blestsched_cb {
	bool retrans_flag;
	s16 lambda_1000; /* values range from min_lambda * 100 to max_lambda * 100 */
	u32 last_lambda_update;
};

static struct blestsched_priv *blestsched_get_priv(const struct tcp_sock *tp)
{
	return (struct blestsched_priv *)&tp->mptcp->mptcp_sched[0];
}

static struct blestsched_cb *blestsched_get_cb(const struct tcp_sock *tp)
{
	return (struct blestsched_cb *)&tp->mpcb->mptcp_sched[0];
}

static void blestsched_update_lambda(struct sock *meta_sk, struct sock *sk)
{
	struct blestsched_cb *blest_cb = blestsched_get_cb(tcp_sk(meta_sk));
	struct blestsched_priv *blest_p = blestsched_get_priv(tcp_sk(sk));

	if (tcp_jiffies32 - blest_cb->last_lambda_update < usecs_to_jiffies(blest_p->min_srtt_us >> 3))
		return;

	/* if there have been retransmissions of packets of the slow flow
	 * during the slow flows last RTT => increase lambda
	 * otherwise decrease
	 */
	if (blest_cb->retrans_flag) {
		/* need to slow down on the slow flow */
		blest_cb->lambda_1000 += dyn_lambda_bad;
	} else {
		/* use the slow flow more */
		blest_cb->lambda_1000 -= dyn_lambda_good;
	}
	blest_cb->retrans_flag = false;

	/* cap lambda_1000 to its value range */
	blest_cb->lambda_1000 = min_t(s16, blest_cb->lambda_1000, max_lambda * 100);
	blest_cb->lambda_1000 = max_t(s16, blest_cb->lambda_1000, min_lambda * 100);

	blest_cb->last_lambda_update = tcp_jiffies32;
}

/* how many bytes will sk send during the rtt of another, slower flow? */
static u32 blestsched_estimate_bytes(struct sock *sk, u32 time_8)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct blestsched_priv *blest_p = blestsched_get_priv(tp);
	struct blestsched_cb *blest_cb = blestsched_get_cb(mptcp_meta_tp(tp));
	u32 avg_rtt, num_rtts, ca_cwnd, packets;

	avg_rtt = (blest_p->min_srtt_us + blest_p->max_srtt_us) / 2;
	if (avg_rtt == 0)
		num_rtts = 1; /* sanity */
	else
		num_rtts = (time_8 / avg_rtt) + 1; /* round up */

	/* during num_rtts, how many bytes will be sent on the flow?
	 * assumes for simplification that Reno is applied as congestion-control
	 */
	if (tp->snd_ssthresh == TCP_INFINITE_SSTHRESH) {
		/* we are in initial slow start */
		if (num_rtts > 16)
			num_rtts = 16; /* cap for sanity */
		packets = tp->snd_cwnd * ((1 << num_rtts) - 1); /* cwnd + 2*cwnd + 4*cwnd */
	} else {
		ca_cwnd = max(tp->snd_cwnd, tp->snd_ssthresh + 1); /* assume we jump to CA already */
		packets = (ca_cwnd + (num_rtts - 1) / 2) * num_rtts;
	}

	return div_u64(((u64)packets) * tp->mss_cache * blest_cb->lambda_1000, 1000);
}

static u32 blestsched_estimate_linger_time(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct blestsched_priv *blest_p = blestsched_get_priv(tp);
	u32 estimate, slope, inflight, cwnd;

	inflight = tcp_packets_in_flight(tp) + 1; /* take into account the new one */
	cwnd = tp->snd_cwnd;

	if (inflight >= cwnd) {
		estimate = blest_p->max_srtt_us;
	} else {
		slope = blest_p->max_srtt_us - blest_p->min_srtt_us;
		if (cwnd == 0)
			cwnd = 1; /* sanity */
		estimate = blest_p->min_srtt_us + (slope * inflight) / cwnd;
	}

	return (tp->srtt_us > estimate) ? tp->srtt_us : estimate;
}

/* This is the BLEST scheduler. This function decides on which flow to send
 * a given MSS. If all subflows are found to be busy or the currently best
 * subflow is estimated to possibly cause HoL-blocking, NULL is returned.
 */
struct sock *blest_get_available_subflow(struct sock *meta_sk, struct sk_buff *skb,
					 bool zero_wnd_test)
{
	struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;
	struct sock *bestsk, *minsk = NULL;
	struct tcp_sock *meta_tp, *besttp;
	struct mptcp_tcp_sock *mptcp;
	struct blestsched_priv *blest_p;
	u32 min_srtt = U32_MAX;

	/* Answer data_fin on same subflow!!! */
	if (meta_sk->sk_shutdown & RCV_SHUTDOWN &&
	    skb && mptcp_is_data_fin(skb)) {
		mptcp_for_each_sub(mpcb, mptcp) {
			bestsk = mptcp_to_sock(mptcp);

			if (tcp_sk(bestsk)->mptcp->path_index == mpcb->dfin_path_index &&
			    mptcp_is_available(bestsk, skb, zero_wnd_test))
				return bestsk;
		}
	}

	/* First, find the overall best subflow */
	mptcp_for_each_sub(mpcb, mptcp) {
		bestsk = mptcp_to_sock(mptcp);
		besttp = tcp_sk(bestsk);
		blest_p = blestsched_get_priv(besttp);

		/* Set of states for which we are allowed to send data */
		if (!mptcp_sk_can_send(bestsk))
			continue;

		/* We do not send data on this subflow unless it is
		 * fully established, i.e. the 4th ack has been received.
		 */
		if (besttp->mptcp->pre_established)
			continue;

		blest_p->min_srtt_us = min(blest_p->min_srtt_us, besttp->srtt_us);
		blest_p->max_srtt_us = max(blest_p->max_srtt_us, besttp->srtt_us);

		/* record minimal rtt */
		if (besttp->srtt_us < min_srtt) {
			min_srtt = besttp->srtt_us;
			minsk = bestsk;
		}
	}

	/* find the current best subflow according to the default scheduler */
	bestsk = get_available_subflow(meta_sk, skb, zero_wnd_test);

	/* if we decided to use a slower flow, we have the option of not using it at all */
	if (bestsk && minsk && bestsk != minsk) {
		u32 slow_linger_time, fast_bytes, slow_inflight_bytes, slow_bytes, avail_space;
		u32 buffered_bytes = 0;

		meta_tp = tcp_sk(meta_sk);
		besttp = tcp_sk(bestsk);

		blestsched_update_lambda(meta_sk, bestsk);

		/* if we send this SKB now, it will be acked in besttp->srtt seconds
		 * during this time: how many bytes will we send on the fast flow?
		 */
		slow_linger_time = blestsched_estimate_linger_time(bestsk);
		fast_bytes = blestsched_estimate_bytes(minsk, slow_linger_time);

		if (skb)
			buffered_bytes = skb->len;

		/* is the required space available in the mptcp meta send window?
		 * we assume that all bytes inflight on the slow path will be acked in besttp->srtt seconds
		 * (just like the SKB if it was sent now) -> that means that those inflight bytes will
		 * keep occupying space in the meta window until then
		 */
		slow_inflight_bytes = besttp->write_seq - besttp->snd_una;
		slow_bytes = buffered_bytes + slow_inflight_bytes; // bytes of this SKB plus those in flight already

		avail_space = (slow_bytes < meta_tp->snd_wnd) ? (meta_tp->snd_wnd - slow_bytes) : 0;

		if (fast_bytes > avail_space) {
			/* sending this SKB on the slow flow means
			 * we wouldn't be able to send all the data we'd like to send on the fast flow
			 * so don't do that
			 */
			return NULL;
		}
	}

	return bestsk;
}

/* copy from mptcp_sched.c: mptcp_rcv_buf_optimization */
static struct sk_buff *mptcp_blest_rcv_buf_optimization(struct sock *sk, int penal)
{
	struct sock *meta_sk;
	const struct tcp_sock *tp = tcp_sk(sk);
	struct mptcp_tcp_sock *mptcp;
	struct sk_buff *skb_head;
	struct blestsched_priv *blest_p = blestsched_get_priv(tp);
	struct blestsched_cb *blest_cb;

	meta_sk = mptcp_meta_sk(sk);
	skb_head = tcp_rtx_queue_head(meta_sk);

	if (!skb_head)
		return NULL;

	/* If penalization is optional (coming from mptcp_next_segment() and
	 * We are not send-buffer-limited we do not penalize. The retransmission
	 * is just an optimization to fix the idle-time due to the delay before
	 * we wake up the application.
	 */
	if (!penal && sk_stream_memory_free(meta_sk))
		goto retrans;

	/* Record the occurrence of a retransmission to update the lambda value */
	blest_cb = blestsched_get_cb(tcp_sk(meta_sk));
	blest_cb->retrans_flag = true;

	/* Only penalize again after an RTT has elapsed */
	if (tcp_jiffies32 - blest_p->last_rbuf_opti < usecs_to_jiffies(tp->srtt_us >> 3))
		goto retrans;

	/* Half the cwnd of the slow flows */
	mptcp_for_each_sub(tp->mpcb, mptcp) {
		struct tcp_sock *tp_it = mptcp->tp;

		if (tp_it != tp &&
		    TCP_SKB_CB(skb_head)->path_mask & mptcp_pi_to_flag(tp_it->mptcp->path_index)) {
			if (tp->srtt_us < tp_it->srtt_us && inet_csk((struct sock *)tp_it)->icsk_ca_state == TCP_CA_Open) {
				u32 prior_cwnd = tp_it->snd_cwnd;

				tp_it->snd_cwnd = max(tp_it->snd_cwnd >> 1U, 1U);

				/* If in slow start, do not reduce the ssthresh */
				if (prior_cwnd >= tp_it->snd_ssthresh)
					tp_it->snd_ssthresh = max(tp_it->snd_ssthresh >> 1U, 2U);

				blest_p->last_rbuf_opti = tcp_jiffies32;
			}
		}
	}

retrans:

	/* Segment not yet injected into this path? Take it!!! */
	if (!(TCP_SKB_CB(skb_head)->path_mask & mptcp_pi_to_flag(tp->mptcp->path_index))) {
		bool do_retrans = false;
		mptcp_for_each_sub(tp->mpcb, mptcp) {
			struct tcp_sock *tp_it = mptcp->tp;

			if (tp_it != tp &&
			    TCP_SKB_CB(skb_head)->path_mask & mptcp_pi_to_flag(tp_it->mptcp->path_index)) {
				if (tp_it->snd_cwnd <= 4) {
					do_retrans = true;
					break;
				}

				if (4 * tp->srtt_us >= tp_it->srtt_us) {
					do_retrans = false;
					break;
				} else {
					do_retrans = true;
				}
			}
		}

		if (do_retrans && mptcp_is_available(sk, skb_head, false)) {
			trace_mptcp_retransmit(sk, skb_head);
			return skb_head;
		}
	}
	return NULL;
}

/* copy from mptcp_sched.c: __mptcp_next_segment */
/* Returns the next segment to be sent from the mptcp meta-queue.
 * (chooses the reinject queue if any segment is waiting in it, otherwise,
 * chooses the normal write queue).
 * Sets *@reinject to 1 if the returned segment comes from the
 * reinject queue. Sets it to 0 if it is the regular send-head of the meta-sk,
 * and sets it to -1 if it is a meta-level retransmission to optimize the
 * receive-buffer.
 */
static struct sk_buff *__mptcp_blest_next_segment(struct sock *meta_sk, int *reinject)
{
	const struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;
	struct sk_buff *skb = NULL;

	*reinject = 0;

	/* If we are in fallback-mode, just take from the meta-send-queue */
	if (mpcb->infinite_mapping_snd || mpcb->send_infinite_mapping)
		return tcp_send_head(meta_sk);

	skb = skb_peek(&mpcb->reinject_queue);

	if (skb) {
		*reinject = 1;
	} else {
		skb = tcp_send_head(meta_sk);

		if (!skb && meta_sk->sk_socket &&
		    test_bit(SOCK_NOSPACE, &meta_sk->sk_socket->flags) &&
		    sk_stream_wspace(meta_sk) < sk_stream_min_wspace(meta_sk)) {
			struct sock *subsk = blest_get_available_subflow(meta_sk, NULL,
									 false);
			if (!subsk)
				return NULL;

			skb = mptcp_blest_rcv_buf_optimization(subsk, 0);
			if (skb)
				*reinject = -1;
		}
	}
	return skb;
}

/* copy from mptcp_sched.c: mptcp_next_segment */
static struct sk_buff *mptcp_blest_next_segment(struct sock *meta_sk,
						int *reinject,
						struct sock **subsk,
						unsigned int *limit)
{
	struct sk_buff *skb = __mptcp_blest_next_segment(meta_sk, reinject);
	unsigned int mss_now;
	struct tcp_sock *subtp;
	u16 gso_max_segs;
	u32 max_len, max_segs, window, needed;

	/* As we set it, we have to reset it as well. */
	*limit = 0;

	if (!skb)
		return NULL;

	*subsk = blest_get_available_subflow(meta_sk, skb, false);
	if (!*subsk)
		return NULL;

	subtp = tcp_sk(*subsk);
	mss_now = tcp_current_mss(*subsk);

	if (!*reinject && unlikely(!tcp_snd_wnd_test(tcp_sk(meta_sk), skb, mss_now))) {
		skb = mptcp_blest_rcv_buf_optimization(*subsk, 1);
		if (skb)
			*reinject = -1;
		else
			return NULL;
	}

	/* No splitting required, as we will only send one single segment */
	if (skb->len <= mss_now)
		return skb;

	/* The following is similar to tcp_mss_split_point, but
	 * we do not care about nagle, because we will anyways
	 * use TCP_NAGLE_PUSH, which overrides this.
	 *
	 * So, we first limit according to the cwnd/gso-size and then according
	 * to the subflow's window.
	 */

	gso_max_segs = (*subsk)->sk_gso_max_segs;
	if (!gso_max_segs) /* No gso supported on the subflow's NIC */
		gso_max_segs = 1;
	max_segs = min_t(unsigned int, tcp_cwnd_test(subtp, skb), gso_max_segs);
	if (!max_segs)
		return NULL;

	max_len = mss_now * max_segs;
	window = tcp_wnd_end(subtp) - subtp->write_seq;

	needed = min(skb->len, window);
	if (max_len <= skb->len)
		/* Take max_win, which is actually the cwnd/gso-size */
		*limit = max_len;
	else
		/* Or, take the window */
		*limit = needed;

	return skb;
}

static void blestsched_init(struct sock *sk)
{
	struct blestsched_priv *blest_p = blestsched_get_priv(tcp_sk(sk));
	struct blestsched_cb *blest_cb = blestsched_get_cb(tcp_sk(mptcp_meta_sk(sk)));

	blest_p->last_rbuf_opti = tcp_jiffies32;
	blest_p->min_srtt_us = U32_MAX;
	blest_p->max_srtt_us = 0;

	if (!blest_cb->lambda_1000) {
		blest_cb->lambda_1000 = lambda * 100;
		blest_cb->last_lambda_update = tcp_jiffies32;
	}
}

static struct mptcp_sched_ops mptcp_sched_blest = {
	.get_subflow = blest_get_available_subflow,
	.next_segment = mptcp_blest_next_segment,
	.init = blestsched_init,
	.name = "blest",
	.owner = THIS_MODULE,
};

static int __init blest_register(void)
{
	BUILD_BUG_ON(sizeof(struct blestsched_priv) > MPTCP_SCHED_SIZE);
	BUILD_BUG_ON(sizeof(struct blestsched_cb) > MPTCP_SCHED_DATA_SIZE);

	if (mptcp_register_scheduler(&mptcp_sched_blest))
		return -1;

	return 0;
}

static void blest_unregister(void)
{
	mptcp_unregister_scheduler(&mptcp_sched_blest);
}

module_init(blest_register);
module_exit(blest_unregister);

MODULE_AUTHOR("Simone Ferlin, Daniel Weber");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BLEST scheduler for MPTCP, based on default minimum RTT scheduler");
MODULE_VERSION("0.95");
#endif
