#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation - Linked Increase congestion control Algorithm (LIA)
 *
 *	Initial Design & Implementation:
 *	Sébastien Barré <sebastien.barre@uclouvain.be>
 *
 *	Current Maintainer & Author:
 *	Christoph Paasch <christoph.paasch@uclouvain.be>
 *
 *	Additional authors:
 *	Jaakko Korkeaniemi <jaakko.korkeaniemi@aalto.fi>
 *	Gregory Detal <gregory.detal@uclouvain.be>
 *	Fabien Duchêne <fabien.duchene@uclouvain.be>
 *	Andreas Seelinger <Andreas.Seelinger@rwth-aachen.de>
 *	Lavkesh Lahngir <lavkesh51@gmail.com>
 *	Andreas Ripke <ripke@neclab.eu>
 *	Vlad Dogaru <vlad.dogaru@intel.com>
 *	Octavian Purdila <octavian.purdila@intel.com>
 *	John Ronan <jronan@tssg.org>
 *	Catalin Nicutar <catalin.nicutar@gmail.com>
 *	Brandon Heller <brandonh@stanford.edu>
 *
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */
#include <net/tcp.h>
#include <net/mptcp.h>

#include <linux/module.h>

/* Scaling is done in the numerator with alpha_scale_num and in the denominator
 * with alpha_scale_den.
 *
 * To downscale, we just need to use alpha_scale.
 *
 * We have: alpha_scale = alpha_scale_num / (alpha_scale_den ^ 2)
 */
static int alpha_scale_den = 10;
static int alpha_scale_num = 32;
static int alpha_scale = 12;

struct mptcp_ccc {
	u64	alpha;
	bool	forced_update;
};

static inline int mptcp_ccc_sk_can_send(const struct sock *sk)
{
	return mptcp_sk_can_send(sk) && tcp_sk(sk)->srtt_us;
}

static inline u64 mptcp_get_alpha(const struct sock *meta_sk)
{
	return ((struct mptcp_ccc *)inet_csk_ca(meta_sk))->alpha;
}

static inline void mptcp_set_alpha(const struct sock *meta_sk, u64 alpha)
{
	((struct mptcp_ccc *)inet_csk_ca(meta_sk))->alpha = alpha;
}

static inline u64 mptcp_ccc_scale(u32 val, int scale)
{
	return (u64) val << scale;
}

static inline bool mptcp_get_forced(const struct sock *meta_sk)
{
	return ((struct mptcp_ccc *)inet_csk_ca(meta_sk))->forced_update;
}

static inline void mptcp_set_forced(const struct sock *meta_sk, bool force)
{
	((struct mptcp_ccc *)inet_csk_ca(meta_sk))->forced_update = force;
}

static void mptcp_ccc_recalc_alpha(const struct sock *sk)
{
	const struct mptcp_cb *mpcb = tcp_sk(sk)->mpcb;
	const struct sock *sub_sk;
	int best_cwnd = 0, best_rtt = 0, can_send = 0;
	u64 max_numerator = 0, sum_denominator = 0, alpha = 1;

	if (!mpcb)
		return;

	/* Only one subflow left - fall back to normal reno-behavior
	 * (set alpha to 1)
	 */
	if (mpcb->cnt_established <= 1)
		goto exit;

	/* Do regular alpha-calculation for multiple subflows */

	/* Find the max numerator of the alpha-calculation */
	mptcp_for_each_sk(mpcb, sub_sk) {
		struct tcp_sock *sub_tp = tcp_sk(sub_sk);
		u64 tmp;

		if (!mptcp_ccc_sk_can_send(sub_sk))
			continue;

		can_send++;

		/* We need to look for the path, that provides the max-value.
		 * Integer-overflow is not possible here, because
		 * tmp will be in u64.
		 */
		tmp = div64_u64(mptcp_ccc_scale(sub_tp->snd_cwnd,
				alpha_scale_num), (u64)sub_tp->srtt_us * sub_tp->srtt_us);

		if (tmp >= max_numerator) {
			max_numerator = tmp;
			best_cwnd = sub_tp->snd_cwnd;
			best_rtt = sub_tp->srtt_us;
		}
	}

	/* No subflow is able to send - we don't care anymore */
	if (unlikely(!can_send))
		goto exit;

	/* Calculate the denominator */
	mptcp_for_each_sk(mpcb, sub_sk) {
		struct tcp_sock *sub_tp = tcp_sk(sub_sk);

		if (!mptcp_ccc_sk_can_send(sub_sk))
			continue;

		sum_denominator += div_u64(
				mptcp_ccc_scale(sub_tp->snd_cwnd,
						alpha_scale_den) * best_rtt,
						sub_tp->srtt_us);
	}
	sum_denominator *= sum_denominator;
	if (unlikely(!sum_denominator)) {
		pr_err("%s: sum_denominator == 0, cnt_established:%d\n",
		       __func__, mpcb->cnt_established);
		mptcp_for_each_sk(mpcb, sub_sk) {
			struct tcp_sock *sub_tp = tcp_sk(sub_sk);
			pr_err("%s: pi:%d, state:%d\n, rtt:%u, cwnd: %u",
			       __func__, sub_tp->mptcp->path_index,
			       sub_sk->sk_state, sub_tp->srtt_us,
			       sub_tp->snd_cwnd);
		}
	}

	alpha = div64_u64(mptcp_ccc_scale(best_cwnd, alpha_scale_num), sum_denominator);

	if (unlikely(!alpha))
		alpha = 1;

exit:
	mptcp_set_alpha(mptcp_meta_sk(sk), alpha);
}

static void mptcp_ccc_init(struct sock *sk)
{
	if (mptcp(tcp_sk(sk))) {
		mptcp_set_forced(mptcp_meta_sk(sk), 0);
		mptcp_set_alpha(mptcp_meta_sk(sk), 1);
	}
	/* If we do not mptcp, behave like reno: return */
}

static void mptcp_ccc_cwnd_event(struct sock *sk, enum tcp_ca_event event)
{
	if (event == CA_EVENT_LOSS)
		mptcp_ccc_recalc_alpha(sk);
}

static void mptcp_ccc_set_state(struct sock *sk, u8 ca_state)
{
	if (!mptcp(tcp_sk(sk)))
		return;

	mptcp_set_forced(mptcp_meta_sk(sk), 1);
}

static void mptcp_ccc_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	struct tcp_sock *tp = tcp_sk(sk);
	const struct mptcp_cb *mpcb = tp->mpcb;
	int snd_cwnd;

	if (!mptcp(tp)) {
		tcp_reno_cong_avoid(sk, ack, acked);
		return;
	}

	if (!tcp_is_cwnd_limited(sk))
		return;

	if (tp->snd_cwnd <= tp->snd_ssthresh) {
		/* In "safe" area, increase. */
		tcp_slow_start(tp, acked);
		mptcp_ccc_recalc_alpha(sk);
		return;
	}

	if (mptcp_get_forced(mptcp_meta_sk(sk))) {
		mptcp_ccc_recalc_alpha(sk);
		mptcp_set_forced(mptcp_meta_sk(sk), 0);
	}

	if (mpcb->cnt_established > 1) {
		u64 alpha = mptcp_get_alpha(mptcp_meta_sk(sk));

		/* This may happen, if at the initialization, the mpcb
		 * was not yet attached to the sock, and thus
		 * initializing alpha failed.
		 */
		if (unlikely(!alpha))
			alpha = 1;

		snd_cwnd = (int) div_u64 ((u64) mptcp_ccc_scale(1, alpha_scale),
						alpha);

		/* snd_cwnd_cnt >= max (scale * tot_cwnd / alpha, cwnd)
		 * Thus, we select here the max value.
		 */
		if (snd_cwnd < tp->snd_cwnd)
			snd_cwnd = tp->snd_cwnd;
	} else {
		snd_cwnd = tp->snd_cwnd;
	}

	if (tp->snd_cwnd_cnt >= snd_cwnd) {
		if (tp->snd_cwnd < tp->snd_cwnd_clamp) {
			tp->snd_cwnd++;
			mptcp_ccc_recalc_alpha(sk);
		}

		tp->snd_cwnd_cnt = 0;
	} else {
		tp->snd_cwnd_cnt++;
	}
}

static struct tcp_congestion_ops mptcp_ccc = {
	.init		= mptcp_ccc_init,
	.ssthresh	= tcp_reno_ssthresh,
	.cong_avoid	= mptcp_ccc_cong_avoid,
	.cwnd_event	= mptcp_ccc_cwnd_event,
	.set_state	= mptcp_ccc_set_state,
	.owner		= THIS_MODULE,
	.name		= "lia",
};

static int __init mptcp_ccc_register(void)
{
	BUILD_BUG_ON(sizeof(struct mptcp_ccc) > ICSK_CA_PRIV_SIZE);
	return tcp_register_congestion_control(&mptcp_ccc);
}

static void __exit mptcp_ccc_unregister(void)
{
	tcp_unregister_congestion_control(&mptcp_ccc);
}

module_init(mptcp_ccc_register);
module_exit(mptcp_ccc_unregister);

MODULE_AUTHOR("Christoph Paasch, Sébastien Barré");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MPTCP LINKED INCREASE CONGESTION CONTROL ALGORITHM");
MODULE_VERSION("0.1");
#endif
