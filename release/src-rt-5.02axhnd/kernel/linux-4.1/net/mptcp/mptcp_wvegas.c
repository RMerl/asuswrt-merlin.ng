#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation - WEIGHTED VEGAS
 *
 *	Algorithm design:
 *	Yu Cao <cyAnalyst@126.com>
 *	Mingwei Xu <xmw@csnet1.cs.tsinghua.edu.cn>
 *	Xiaoming Fu <fu@cs.uni-goettinggen.de>
 *
 *	Implementation:
 *	Yu Cao <cyAnalyst@126.com>
 *	Enhuan Dong <deh13@mails.tsinghua.edu.cn>
 *
 *	Ported to the official MPTCP-kernel:
 *	Christoph Paasch <christoph.paasch@uclouvain.be>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/skbuff.h>
#include <net/tcp.h>
#include <net/mptcp.h>
#include <linux/module.h>
#include <linux/tcp.h>

static int initial_alpha = 2;
static int total_alpha = 10;
static int gamma = 1;

module_param(initial_alpha, int, 0644);
MODULE_PARM_DESC(initial_alpha, "initial alpha for all subflows");
module_param(total_alpha, int, 0644);
MODULE_PARM_DESC(total_alpha, "total alpha for all subflows");
module_param(gamma, int, 0644);
MODULE_PARM_DESC(gamma, "limit on increase (scale by 2)");

#define MPTCP_WVEGAS_SCALE 16

/* wVegas variables */
struct wvegas {
	u32	beg_snd_nxt;	/* right edge during last RTT */
	u8	doing_wvegas_now;/* if true, do wvegas for this RTT */

	u16	cnt_rtt;		/* # of RTTs measured within last RTT */
	u32 sampled_rtt; /* cumulative RTTs measured within last RTT (in usec) */
	u32	base_rtt;	/* the min of all wVegas RTT measurements seen (in usec) */

	u64 instant_rate; /* cwnd / srtt_us, unit: pkts/us * 2^16 */
	u64 weight; /* the ratio of subflow's rate to the total rate, * 2^16 */
	int alpha; /* alpha for each subflows */

	u32 queue_delay; /* queue delay*/
};


static inline u64 mptcp_wvegas_scale(u32 val, int scale)
{
	return (u64) val << scale;
}

static void wvegas_enable(const struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	struct wvegas *wvegas = inet_csk_ca(sk);

	wvegas->doing_wvegas_now = 1;

	wvegas->beg_snd_nxt = tp->snd_nxt;

	wvegas->cnt_rtt = 0;
	wvegas->sampled_rtt = 0;

	wvegas->instant_rate = 0;
	wvegas->alpha = initial_alpha;
	wvegas->weight = mptcp_wvegas_scale(1, MPTCP_WVEGAS_SCALE);

	wvegas->queue_delay = 0;
}

static inline void wvegas_disable(const struct sock *sk)
{
	struct wvegas *wvegas = inet_csk_ca(sk);

	wvegas->doing_wvegas_now = 0;
}

static void mptcp_wvegas_init(struct sock *sk)
{
	struct wvegas *wvegas = inet_csk_ca(sk);

	wvegas->base_rtt = 0x7fffffff;
	wvegas_enable(sk);
}

static inline u64 mptcp_wvegas_rate(u32 cwnd, u32 rtt_us)
{
	return div_u64(mptcp_wvegas_scale(cwnd, MPTCP_WVEGAS_SCALE), rtt_us);
}

static void mptcp_wvegas_pkts_acked(struct sock *sk, u32 cnt, s32 rtt_us)
{
	struct wvegas *wvegas = inet_csk_ca(sk);
	u32 vrtt;

	if (rtt_us < 0)
		return;

	vrtt = rtt_us + 1;

	if (vrtt < wvegas->base_rtt)
		wvegas->base_rtt = vrtt;

	wvegas->sampled_rtt += vrtt;
	wvegas->cnt_rtt++;
}

static void mptcp_wvegas_state(struct sock *sk, u8 ca_state)
{
	if (ca_state == TCP_CA_Open)
		wvegas_enable(sk);
	else
		wvegas_disable(sk);
}

static void mptcp_wvegas_cwnd_event(struct sock *sk, enum tcp_ca_event event)
{
	if (event == CA_EVENT_CWND_RESTART) {
		mptcp_wvegas_init(sk);
	} else if (event == CA_EVENT_LOSS) {
		struct wvegas *wvegas = inet_csk_ca(sk);
		wvegas->instant_rate = 0;
	}
}

static inline u32 mptcp_wvegas_ssthresh(const struct tcp_sock *tp)
{
	return  min(tp->snd_ssthresh, tp->snd_cwnd - 1);
}

static u64 mptcp_wvegas_weight(const struct mptcp_cb *mpcb, const struct sock *sk)
{
	u64 total_rate = 0;
	struct sock *sub_sk;
	const struct wvegas *wvegas = inet_csk_ca(sk);

	if (!mpcb)
		return wvegas->weight;


	mptcp_for_each_sk(mpcb, sub_sk) {
		struct wvegas *sub_wvegas = inet_csk_ca(sub_sk);

		/* sampled_rtt is initialized by 0 */
		if (mptcp_sk_can_send(sub_sk) && (sub_wvegas->sampled_rtt > 0))
			total_rate += sub_wvegas->instant_rate;
	}

	if (total_rate && wvegas->instant_rate)
		return div64_u64(mptcp_wvegas_scale(wvegas->instant_rate, MPTCP_WVEGAS_SCALE), total_rate);
	else
		return wvegas->weight;
}

static void mptcp_wvegas_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct wvegas *wvegas = inet_csk_ca(sk);

	if (!wvegas->doing_wvegas_now) {
		tcp_reno_cong_avoid(sk, ack, acked);
		return;
	}

	if (after(ack, wvegas->beg_snd_nxt)) {
		wvegas->beg_snd_nxt  = tp->snd_nxt;

		if (wvegas->cnt_rtt <= 2) {
			tcp_reno_cong_avoid(sk, ack, acked);
		} else {
			u32 rtt, diff, q_delay;
			u64 target_cwnd;

			rtt = wvegas->sampled_rtt / wvegas->cnt_rtt;
			target_cwnd = div_u64(((u64)tp->snd_cwnd * wvegas->base_rtt), rtt);

			diff = div_u64((u64)tp->snd_cwnd * (rtt - wvegas->base_rtt), rtt);

			if (diff > gamma && tp->snd_cwnd <= tp->snd_ssthresh) {
				tp->snd_cwnd = min(tp->snd_cwnd, (u32)target_cwnd+1);
				tp->snd_ssthresh = mptcp_wvegas_ssthresh(tp);

			} else if (tp->snd_cwnd <= tp->snd_ssthresh) {
				tcp_slow_start(tp, acked);
			} else {
				if (diff >= wvegas->alpha) {
					wvegas->instant_rate = mptcp_wvegas_rate(tp->snd_cwnd, rtt);
					wvegas->weight = mptcp_wvegas_weight(tp->mpcb, sk);
					wvegas->alpha = max(2U, (u32)((wvegas->weight * total_alpha) >> MPTCP_WVEGAS_SCALE));
				}
				if (diff > wvegas->alpha) {
					tp->snd_cwnd--;
					tp->snd_ssthresh = mptcp_wvegas_ssthresh(tp);
				} else if (diff < wvegas->alpha) {
					tp->snd_cwnd++;
				}

				/* Try to drain link queue if needed*/
				q_delay = rtt - wvegas->base_rtt;
				if ((wvegas->queue_delay == 0) || (wvegas->queue_delay > q_delay))
					wvegas->queue_delay = q_delay;

				if (q_delay >= 2 * wvegas->queue_delay) {
					u32 backoff_factor = div_u64(mptcp_wvegas_scale(wvegas->base_rtt, MPTCP_WVEGAS_SCALE), 2 * rtt);
					tp->snd_cwnd = ((u64)tp->snd_cwnd * backoff_factor) >> MPTCP_WVEGAS_SCALE;
					wvegas->queue_delay = 0;
				}
			}

			if (tp->snd_cwnd < 2)
				tp->snd_cwnd = 2;
			else if (tp->snd_cwnd > tp->snd_cwnd_clamp)
				tp->snd_cwnd = tp->snd_cwnd_clamp;

			tp->snd_ssthresh = tcp_current_ssthresh(sk);
		}

		wvegas->cnt_rtt = 0;
		wvegas->sampled_rtt = 0;
	}
	/* Use normal slow start */
	else if (tp->snd_cwnd <= tp->snd_ssthresh)
		tcp_slow_start(tp, acked);
}


static struct tcp_congestion_ops mptcp_wvegas __read_mostly = {
	.init		= mptcp_wvegas_init,
	.ssthresh	= tcp_reno_ssthresh,
	.cong_avoid	= mptcp_wvegas_cong_avoid,
	.pkts_acked	= mptcp_wvegas_pkts_acked,
	.set_state	= mptcp_wvegas_state,
	.cwnd_event	= mptcp_wvegas_cwnd_event,

	.owner		= THIS_MODULE,
	.name		= "wvegas",
};

static int __init mptcp_wvegas_register(void)
{
	BUILD_BUG_ON(sizeof(struct wvegas) > ICSK_CA_PRIV_SIZE);
	tcp_register_congestion_control(&mptcp_wvegas);
	return 0;
}

static void __exit mptcp_wvegas_unregister(void)
{
	tcp_unregister_congestion_control(&mptcp_wvegas);
}

module_init(mptcp_wvegas_register);
module_exit(mptcp_wvegas_unregister);

MODULE_AUTHOR("Yu Cao, Enhuan Dong");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MPTCP wVegas");
MODULE_VERSION("0.1");
#endif
