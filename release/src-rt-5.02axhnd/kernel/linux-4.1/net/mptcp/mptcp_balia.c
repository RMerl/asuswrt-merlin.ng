#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation - Balia Congestion Control
 *	(Balanced Linked Adaptation Algorithm)
 *
 *	Analysis, Design and Implementation:
 *	Qiuyu Peng <qpeng@caltech.edu>
 *	Anwar Walid <anwar@research.bell-labs.com>
 *	Jaehyun Hwang <jhyun.hwang@samsung.com>
 *	Steven H. Low <slow@caltech.edu>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <net/tcp.h>
#include <net/mptcp.h>

#include <linux/module.h>

/* The variable 'rate' (i.e., x_r) will be scaled
 * e.g., from B/s to KB/s, MB/s, or GB/s
 * if max_rate > 2^rate_scale_limit
 */

static int rate_scale_limit = 25;
static int alpha_scale = 10;
static int scale_num = 5;

struct mptcp_balia {
	u64	ai;
	u64	md;
	bool	forced_update;
};

static inline int mptcp_balia_sk_can_send(const struct sock *sk)
{
	return mptcp_sk_can_send(sk) && tcp_sk(sk)->srtt_us;
}

static inline u64 mptcp_get_ai(const struct sock *meta_sk)
{
	return ((struct mptcp_balia *)inet_csk_ca(meta_sk))->ai;
}

static inline void mptcp_set_ai(const struct sock *meta_sk, u64 ai)
{
	((struct mptcp_balia *)inet_csk_ca(meta_sk))->ai = ai;
}

static inline u64 mptcp_get_md(const struct sock *meta_sk)
{
	return ((struct mptcp_balia *)inet_csk_ca(meta_sk))->md;
}

static inline void mptcp_set_md(const struct sock *meta_sk, u64 md)
{
	((struct mptcp_balia *)inet_csk_ca(meta_sk))->md = md;
}

static inline u64 mptcp_balia_scale(u64 val, int scale)
{
	return (u64) val << scale;
}

static inline bool mptcp_get_forced(const struct sock *meta_sk)
{
	return ((struct mptcp_balia *)inet_csk_ca(meta_sk))->forced_update;
}

static inline void mptcp_set_forced(const struct sock *meta_sk, bool force)
{
	((struct mptcp_balia *)inet_csk_ca(meta_sk))->forced_update = force;
}

static void mptcp_balia_recalc_ai(const struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	const struct mptcp_cb *mpcb = tp->mpcb;
	const struct sock *sub_sk;
	u64 max_rate = 0, rate = 0, sum_rate = 0;
	u64 alpha, ai = tp->snd_cwnd, md = (tp->snd_cwnd >> 1);
	int num_scale_down = 0;

	if (!mpcb)
		return;

	/* Only one subflow left - fall back to normal reno-behavior */
	if (mpcb->cnt_established <= 1)
		goto exit;

	/* Find max_rate first */
	mptcp_for_each_sk(mpcb, sub_sk) {
		struct tcp_sock *sub_tp = tcp_sk(sub_sk);
		u64 tmp;

		if (!mptcp_balia_sk_can_send(sub_sk))
			continue;

		tmp = div_u64((u64)tp->mss_cache * sub_tp->snd_cwnd
				* (USEC_PER_SEC << 3), sub_tp->srtt_us);
		sum_rate += tmp;

		if (tp == sub_tp)
			rate = tmp;

		if (tmp >= max_rate)
			max_rate = tmp;
	}

	/* At least, the current subflow should be able to send */
	if (unlikely(!rate))
		goto exit;

	alpha = div64_u64(max_rate, rate);

	/* Scale down max_rate if it is too high (e.g., >2^25) */
	while (max_rate > mptcp_balia_scale(1, rate_scale_limit)) {
		max_rate >>= scale_num;
		num_scale_down++;
	}

	if (num_scale_down) {
		sum_rate = 0;
		mptcp_for_each_sk(mpcb, sub_sk) {
			struct tcp_sock *sub_tp = tcp_sk(sub_sk);
			u64 tmp;

			if (!mptcp_balia_sk_can_send(sub_sk))
				continue;

			tmp = div_u64((u64)tp->mss_cache * sub_tp->snd_cwnd
				* (USEC_PER_SEC << 3), sub_tp->srtt_us);
			tmp >>= (scale_num * num_scale_down);

			sum_rate += tmp;
		}
		rate >>= (scale_num * num_scale_down);
	}

	/*	(sum_rate)^2 * 10 * w_r
	 * ai = ------------------------------------
	 *	(x_r + max_rate) * (4x_r + max_rate)
	 */
	sum_rate *= sum_rate;

	ai = div64_u64(sum_rate * 10, rate + max_rate);
	ai = div64_u64(ai * tp->snd_cwnd, (rate << 2) + max_rate);

	if (unlikely(!ai))
		ai = tp->snd_cwnd;

	md = ((tp->snd_cwnd >> 1) * min(mptcp_balia_scale(alpha, alpha_scale),
					mptcp_balia_scale(3, alpha_scale) >> 1))
					>> alpha_scale;

exit:
	mptcp_set_ai(sk, ai);
	mptcp_set_md(sk, md);
}

static void mptcp_balia_init(struct sock *sk)
{
	if (mptcp(tcp_sk(sk))) {
		mptcp_set_forced(sk, 0);
		mptcp_set_ai(sk, 0);
		mptcp_set_md(sk, 0);
	}
}

static void mptcp_balia_cwnd_event(struct sock *sk, enum tcp_ca_event event)
{
	if (event == CA_EVENT_COMPLETE_CWR || event == CA_EVENT_LOSS)
		mptcp_balia_recalc_ai(sk);
}

static void mptcp_balia_set_state(struct sock *sk, u8 ca_state)
{
	if (!mptcp(tcp_sk(sk)))
		return;

	mptcp_set_forced(sk, 1);
}

static void mptcp_balia_cong_avoid(struct sock *sk, u32 ack, u32 acked)
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
		mptcp_balia_recalc_ai(sk);
		return;
	}

	if (mptcp_get_forced(mptcp_meta_sk(sk))) {
		mptcp_balia_recalc_ai(sk);
		mptcp_set_forced(sk, 0);
	}

	if (mpcb->cnt_established > 1)
		snd_cwnd = (int) mptcp_get_ai(sk);
	else
		snd_cwnd = tp->snd_cwnd;

	if (tp->snd_cwnd_cnt >= snd_cwnd) {
		if (tp->snd_cwnd < tp->snd_cwnd_clamp) {
			tp->snd_cwnd++;
			mptcp_balia_recalc_ai(sk);
		}

		tp->snd_cwnd_cnt = 0;
	} else {
		tp->snd_cwnd_cnt++;
	}
}

static u32 mptcp_balia_ssthresh(struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	const struct mptcp_cb *mpcb = tp->mpcb;

	if (unlikely(!mptcp(tp) || mpcb->cnt_established <= 1))
		return tcp_reno_ssthresh(sk);
	else
		return max((u32)(tp->snd_cwnd - mptcp_get_md(sk)), 1U);
}

static struct tcp_congestion_ops mptcp_balia = {
	.init		= mptcp_balia_init,
	.ssthresh	= mptcp_balia_ssthresh,
	.cong_avoid	= mptcp_balia_cong_avoid,
	.cwnd_event	= mptcp_balia_cwnd_event,
	.set_state	= mptcp_balia_set_state,
	.owner		= THIS_MODULE,
	.name		= "balia",
};

static int __init mptcp_balia_register(void)
{
	BUILD_BUG_ON(sizeof(struct mptcp_balia) > ICSK_CA_PRIV_SIZE);
	return tcp_register_congestion_control(&mptcp_balia);
}

static void __exit mptcp_balia_unregister(void)
{
	tcp_unregister_congestion_control(&mptcp_balia);
}

module_init(mptcp_balia_register);
module_exit(mptcp_balia_unregister);

MODULE_AUTHOR("Jaehyun Hwang, Anwar Walid, Qiuyu Peng, Steven H. Low");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MPTCP BALIA CONGESTION CONTROL ALGORITHM");
MODULE_VERSION("0.1");
#endif
