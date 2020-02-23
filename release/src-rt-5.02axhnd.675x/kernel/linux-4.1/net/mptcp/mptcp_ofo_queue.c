#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation - Fast algorithm for MPTCP meta-reordering
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
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <linux/skbuff.h>
#include <linux/slab.h>
#include <net/tcp.h>
#include <net/mptcp.h>

void mptcp_remove_shortcuts(const struct mptcp_cb *mpcb,
			    const struct sk_buff *skb)
{
	struct tcp_sock *tp;

	mptcp_for_each_tp(mpcb, tp) {
		if (tp->mptcp->shortcut_ofoqueue == skb) {
			tp->mptcp->shortcut_ofoqueue = NULL;
			return;
		}
	}
}

/* Does 'skb' fits after 'here' in the queue 'head' ?
 * If yes, we queue it and return 1
 */
static int mptcp_ofo_queue_after(struct sk_buff_head *head,
				 struct sk_buff *skb, struct sk_buff *here,
				 const struct tcp_sock *tp)
{
	struct sock *meta_sk = tp->meta_sk;
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	u32 seq = TCP_SKB_CB(skb)->seq;
	u32 end_seq = TCP_SKB_CB(skb)->end_seq;

	/* We want to queue skb after here, thus seq >= end_seq */
	if (before(seq, TCP_SKB_CB(here)->end_seq))
		return 0;

	if (seq == TCP_SKB_CB(here)->end_seq) {
		bool fragstolen = false;

		if (!tcp_try_coalesce(meta_sk, here, skb, &fragstolen)) {
			__skb_queue_after(&meta_tp->out_of_order_queue, here, skb);
			return 1;
		} else {
			kfree_skb_partial(skb, fragstolen);
			return -1;
		}
	}

	/* If here is the last one, we can always queue it */
	if (skb_queue_is_last(head, here)) {
		__skb_queue_after(head, here, skb);
		return 1;
	} else {
		struct sk_buff *skb1 = skb_queue_next(head, here);
		/* It's not the last one, but does it fits between 'here' and
		 * the one after 'here' ? Thus, does end_seq <= after_here->seq
		 */
		if (!after(end_seq, TCP_SKB_CB(skb1)->seq)) {
			__skb_queue_after(head, here, skb);
			return 1;
		}
	}

	return 0;
}

static void try_shortcut(struct sk_buff *shortcut, struct sk_buff *skb,
			 struct sk_buff_head *head, struct tcp_sock *tp)
{
	struct sock *meta_sk = tp->meta_sk;
	struct tcp_sock *tp_it, *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	struct sk_buff *skb1, *best_shortcut = NULL;
	u32 seq = TCP_SKB_CB(skb)->seq;
	u32 end_seq = TCP_SKB_CB(skb)->end_seq;
	u32 distance = 0xffffffff;

	/* First, check the tp's shortcut */
	if (!shortcut) {
		if (skb_queue_empty(head)) {
			__skb_queue_head(head, skb);
			goto end;
		}
	} else {
		int ret = mptcp_ofo_queue_after(head, skb, shortcut, tp);
		/* Does the tp's shortcut is a hit? If yes, we insert. */

		if (ret) {
			skb = (ret > 0) ? skb : NULL;
			goto end;
		}
	}

	/* Check the shortcuts of the other subsockets. */
	mptcp_for_each_tp(mpcb, tp_it) {
		shortcut = tp_it->mptcp->shortcut_ofoqueue;
		/* Can we queue it here? If yes, do so! */
		if (shortcut) {
			int ret = mptcp_ofo_queue_after(head, skb, shortcut, tp);

			if (ret) {
				skb = (ret > 0) ? skb : NULL;
				goto end;
			}
		}

		/* Could not queue it, check if we are close.
		 * We are looking for a shortcut, close enough to seq to
		 * set skb1 prematurely and thus improve the subsequent lookup,
		 * which tries to find a skb1 so that skb1->seq <= seq.
		 *
		 * So, here we only take shortcuts, whose shortcut->seq > seq,
		 * and minimize the distance between shortcut->seq and seq and
		 * set best_shortcut to this one with the minimal distance.
		 *
		 * That way, the subsequent while-loop is shortest.
		 */
		if (shortcut && after(TCP_SKB_CB(shortcut)->seq, seq)) {
			/* Are we closer than the current best shortcut? */
			if ((u32)(TCP_SKB_CB(shortcut)->seq - seq) < distance) {
				distance = (u32)(TCP_SKB_CB(shortcut)->seq - seq);
				best_shortcut = shortcut;
			}
		}
	}

	if (best_shortcut)
		skb1 = best_shortcut;
	else
		skb1 = skb_peek_tail(head);

	if (seq == TCP_SKB_CB(skb1)->end_seq) {
		bool fragstolen = false;

		if (!tcp_try_coalesce(meta_sk, skb1, skb, &fragstolen)) {
			__skb_queue_after(&meta_tp->out_of_order_queue, skb1, skb);
		} else {
			kfree_skb_partial(skb, fragstolen);
			skb = NULL;
		}

		goto end;
	}

	/* Find the insertion point, starting from best_shortcut if available.
	 *
	 * Inspired from tcp_data_queue_ofo.
	 */
	while (1) {
		/* skb1->seq <= seq */
		if (!after(TCP_SKB_CB(skb1)->seq, seq))
			break;
		if (skb_queue_is_first(head, skb1)) {
			skb1 = NULL;
			break;
		}
		skb1 = skb_queue_prev(head, skb1);
	}

	/* Do skb overlap to previous one? */
	if (skb1 && before(seq, TCP_SKB_CB(skb1)->end_seq)) {
		if (!after(end_seq, TCP_SKB_CB(skb1)->end_seq)) {
			/* All the bits are present. */
			__kfree_skb(skb);
			skb = NULL;
			goto end;
		}
		if (seq == TCP_SKB_CB(skb1)->seq) {
			if (skb_queue_is_first(head, skb1))
				skb1 = NULL;
			else
				skb1 = skb_queue_prev(head, skb1);
		}
	}
	if (!skb1)
		__skb_queue_head(head, skb);
	else
		__skb_queue_after(head, skb1, skb);

	/* And clean segments covered by new one as whole. */
	while (!skb_queue_is_last(head, skb)) {
		skb1 = skb_queue_next(head, skb);

		if (!after(end_seq, TCP_SKB_CB(skb1)->seq))
			break;

		__skb_unlink(skb1, head);
		mptcp_remove_shortcuts(mpcb, skb1);
		__kfree_skb(skb1);
	}

end:
	if (skb) {
		skb_set_owner_r(skb, meta_sk);
		tp->mptcp->shortcut_ofoqueue = skb;
	}

	return;
}

/**
 * @sk: the subflow that received this skb.
 */
void mptcp_add_meta_ofo_queue(const struct sock *meta_sk, struct sk_buff *skb,
			      struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	try_shortcut(tp->mptcp->shortcut_ofoqueue, skb,
		     &tcp_sk(meta_sk)->out_of_order_queue, tp);
}

bool mptcp_prune_ofo_queue(struct sock *sk)
{
	struct tcp_sock *tp	= tcp_sk(sk);
	bool res		= false;

	if (!skb_queue_empty(&tp->out_of_order_queue)) {
		NET_INC_STATS_BH(sock_net(sk), LINUX_MIB_OFOPRUNED);
		mptcp_purge_ofo_queue(tp);

		/* No sack at the mptcp-level */
		sk_mem_reclaim(sk);
		res = true;
	}

	return res;
}

void mptcp_ofo_queue(struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct sk_buff *skb;

	while ((skb = skb_peek(&meta_tp->out_of_order_queue)) != NULL) {
		u32 old_rcv_nxt = meta_tp->rcv_nxt;
		if (after(TCP_SKB_CB(skb)->seq, meta_tp->rcv_nxt))
			break;

		if (!after(TCP_SKB_CB(skb)->end_seq, meta_tp->rcv_nxt)) {
			__skb_unlink(skb, &meta_tp->out_of_order_queue);
			mptcp_remove_shortcuts(meta_tp->mpcb, skb);
			__kfree_skb(skb);
			continue;
		}

		__skb_unlink(skb, &meta_tp->out_of_order_queue);
		mptcp_remove_shortcuts(meta_tp->mpcb, skb);

		__skb_queue_tail(&meta_sk->sk_receive_queue, skb);
		meta_tp->rcv_nxt = TCP_SKB_CB(skb)->end_seq;
		mptcp_check_rcvseq_wrap(meta_tp, old_rcv_nxt);

		if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN)
			mptcp_fin(meta_sk);
	}
}

void mptcp_purge_ofo_queue(struct tcp_sock *meta_tp)
{
	struct sk_buff_head *head = &meta_tp->out_of_order_queue;
	struct sk_buff *skb, *tmp;

	skb_queue_walk_safe(head, skb, tmp) {
		__skb_unlink(skb, head);
		mptcp_remove_shortcuts(meta_tp->mpcb, skb);
		kfree_skb(skb);
	}
}
#endif
