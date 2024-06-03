#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
/*
 *  Desynchronized Multi-Channel TCP Congestion Control Algorithm
 *
 *  Implementation based on publications of "DMCTCP:Desynchronized Multi-Channel
 *  TCP for high speed access networks with tiny buffers" in 23rd international
 *  conference of Computer Communication and Networks (ICCCN), 2014, and
 *  "Exploring parallelism and desynchronization of TCP over high speed networks
 *  with tiny buffers" in Journal of Computer Communications Elsevier, 2015.
 *
 *  http://ieeexplore.ieee.org/abstract/document/6911722/
 *  https://doi.org/10.1016/j.comcom.2015.07.010
 *
 *  This prototype is for research purpose and is currently experimental code
 *  that only support a single path. Future support of multi-channel over
 *  multi-path requires channels grouping.
 *
 *  Initial Design and Implementation:
 *  Cheng Cui <Cheng.Cui@netapp.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 */
#include <net/tcp.h>
#include <net/mptcp.h>
#include <linux/module.h>

enum {
	MASTER_CHANNEL = 1,
	INI_MIN_CWND = 2,
};

/* private congestion control structure:
 * off_tstamp: the last backoff timestamp for loss synchronization event
 * off_subfid: the subflow which was backoff on off_tstamp
 */
struct mctcp_desync {
	u64	off_tstamp;
	u8	off_subfid;
};

static inline int mctcp_cc_sk_can_send(const struct sock *sk)
{
	return mptcp_sk_can_send(sk) && tcp_sk(sk)->srtt_us;
}

static void mctcp_desync_init(struct sock *sk)
{
	if (mptcp(tcp_sk(sk))) {
		struct mctcp_desync *ca = inet_csk_ca(mptcp_meta_sk(sk));
		ca->off_tstamp = 0;
		ca->off_subfid = 0;
    }
    /* If we do not mptcp, behave like reno: return */
}

static void mctcp_desync_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (!mptcp(tp)) {
		tcp_reno_cong_avoid(sk, ack, acked);
		return;
	} else if (!tcp_is_cwnd_limited(sk)) {
		return;
	} else {
		const struct mctcp_desync *ca = inet_csk_ca(mptcp_meta_sk(sk));
		const u8 subfid = tp->mptcp->path_index;

		/* current aggregated cwnd */
		u32 agg_cwnd = 0;
		u32 min_cwnd = 0xffffffff;
		u8 min_cwnd_subfid = 0;

		/* In "safe" area, increase */
		if (tcp_in_slow_start(tp)) {
			if (ca->off_subfid) {
				/* passed initial phase, allow slow start */
				tcp_slow_start(tp, acked);
			} else if (MASTER_CHANNEL == tp->mptcp->path_index) {
				/* master channel is normal slow start in
				 * initial phase */
				tcp_slow_start(tp, acked);
			} else {
				/* secondary channels increase slowly until
				 * the initial phase passed
				 */
				tp->snd_ssthresh = tp->snd_cwnd = INI_MIN_CWND;
			}
			return;
		} else {
			/* In dangerous area, increase slowly and linearly. */
			const struct mptcp_tcp_sock *mptcp;

			/* get total cwnd and the subflow that has min cwnd */
			mptcp_for_each_sub(tp->mpcb, mptcp) {
				const struct sock *sub_sk = mptcp_to_sock(mptcp);

				if (mctcp_cc_sk_can_send(sub_sk)) {
					const struct tcp_sock *sub_tp =
								tcp_sk(sub_sk);
					agg_cwnd += sub_tp->snd_cwnd;
					if(min_cwnd > sub_tp->snd_cwnd) {
						min_cwnd = sub_tp->snd_cwnd;
						min_cwnd_subfid =
						      sub_tp->mptcp->path_index;
					}
				}
			}
			/* the smallest subflow grows faster than others */
			if (subfid == min_cwnd_subfid) {
				tcp_cong_avoid_ai(tp, min_cwnd, acked);
			} else {
				tcp_cong_avoid_ai(tp, agg_cwnd - min_cwnd,
						  acked);
			}
		}
	}
}

static u32 mctcp_desync_ssthresh(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (!mptcp(tp)) {
		return max(tp->snd_cwnd >> 1U, 2U);
	} else {
		struct mctcp_desync *ca = inet_csk_ca(mptcp_meta_sk(sk));
		const u8 subfid = tp->mptcp->path_index;
		const struct mptcp_tcp_sock *mptcp;
		u32 max_cwnd = 0;
		u8 max_cwnd_subfid = 0;

		/* Find the subflow that has the max cwnd. */
		mptcp_for_each_sub(tp->mpcb, mptcp) {
			const struct sock *sub_sk = mptcp_to_sock(mptcp);

			if (mctcp_cc_sk_can_send(sub_sk)) {
				const struct tcp_sock *sub_tp = tcp_sk(sub_sk);
				if (max_cwnd < sub_tp->snd_cwnd) {
					max_cwnd = sub_tp->snd_cwnd;
					max_cwnd_subfid =
						sub_tp->mptcp->path_index;
				}
			}
		}
		/* Use high resolution clock. */
		if (subfid == max_cwnd_subfid) {
			u64 now = tcp_clock_us();
			u32 delta = tcp_stamp_us_delta(now, ca->off_tstamp);

			if (delta < (tp->srtt_us >> 3)) {
				/* desynchronize */
				return tp->snd_cwnd;
			} else {
				ca->off_tstamp = now;
				ca->off_subfid = subfid;
				return max(max_cwnd >> 1U, 2U);
			}
		} else {
			return tp->snd_cwnd;
		}
	}
}

static struct tcp_congestion_ops mctcp_desync = {
	.init       = mctcp_desync_init,
	.ssthresh   = mctcp_desync_ssthresh,
	.undo_cwnd  = tcp_reno_undo_cwnd,
	.cong_avoid = mctcp_desync_cong_avoid,
	.owner      = THIS_MODULE,
	.name       = "mctcpdesync",
};

static int __init mctcp_desync_register(void)
{
	BUILD_BUG_ON(sizeof(struct mctcp_desync) > ICSK_CA_PRIV_SIZE);
	return tcp_register_congestion_control(&mctcp_desync);
}

static void __exit mctcp_desync_unregister(void)
{
	tcp_unregister_congestion_control(&mctcp_desync);
}

module_init(mctcp_desync_register);
module_exit(mctcp_desync_unregister);

MODULE_AUTHOR("Cheng Cui");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MCTCP: DESYNCHRONIZED MULTICHANNEL TCP CONGESTION CONTROL");
MODULE_VERSION("1.0");
#endif
