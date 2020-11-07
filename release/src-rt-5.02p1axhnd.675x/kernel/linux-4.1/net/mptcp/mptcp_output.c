#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation - Sending side
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

#include <linux/kconfig.h>
#include <linux/skbuff.h>
#include <linux/tcp.h>

#include <net/mptcp.h>
#include <net/mptcp_v4.h>
#include <net/mptcp_v6.h>
#include <net/sock.h>

static const int mptcp_dss_len = MPTCP_SUB_LEN_DSS_ALIGN +
				 MPTCP_SUB_LEN_ACK_ALIGN +
				 MPTCP_SUB_LEN_SEQ_ALIGN;

static inline int mptcp_sub_len_remove_addr(u16 bitfield)
{
	unsigned int c;
	for (c = 0; bitfield; c++)
		bitfield &= bitfield - 1;
	return MPTCP_SUB_LEN_REMOVE_ADDR + c - 1;
}

int mptcp_sub_len_remove_addr_align(u16 bitfield)
{
	return ALIGN(mptcp_sub_len_remove_addr(bitfield), 4);
}
EXPORT_SYMBOL(mptcp_sub_len_remove_addr_align);

/* get the data-seq and end-data-seq and store them again in the
 * tcp_skb_cb
 */
static bool mptcp_reconstruct_mapping(struct sk_buff *skb)
{
	const struct mp_dss *mpdss = (struct mp_dss *)TCP_SKB_CB(skb)->dss;
	u32 *p32;
	u16 *p16;

	if (!mptcp_is_data_seq(skb))
		return false;

	if (!mpdss->M)
		return false;

	/* Move the pointer to the data-seq */
	p32 = (u32 *)mpdss;
	p32++;
	if (mpdss->A) {
		p32++;
		if (mpdss->a)
			p32++;
	}

	TCP_SKB_CB(skb)->seq = ntohl(*p32);

	/* Get the data_len to calculate the end_data_seq */
	p32++;
	p32++;
	p16 = (u16 *)p32;
	TCP_SKB_CB(skb)->end_seq = ntohs(*p16) + TCP_SKB_CB(skb)->seq;

	return true;
}

static bool mptcp_is_reinjected(const struct sk_buff *skb)
{
	return TCP_SKB_CB(skb)->mptcp_flags & MPTCP_REINJECT;
}

static void mptcp_find_and_set_pathmask(const struct sock *meta_sk, struct sk_buff *skb)
{
	struct sk_buff *skb_it;

	skb_it = tcp_write_queue_head(meta_sk);

	tcp_for_write_queue_from(skb_it, meta_sk) {
		if (skb_it == tcp_send_head(meta_sk))
			break;

		if (TCP_SKB_CB(skb_it)->seq == TCP_SKB_CB(skb)->seq) {
			TCP_SKB_CB(skb)->path_mask = TCP_SKB_CB(skb_it)->path_mask;
			break;
		}
	}
}

/* Reinject data from one TCP subflow to the meta_sk. If sk == NULL, we are
 * coming from the meta-retransmit-timer
 */
static void __mptcp_reinject_data(struct sk_buff *orig_skb, struct sock *meta_sk,
				  struct sock *sk, int clone_it)
{
	struct sk_buff *skb, *skb1;
	const struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	u32 seq, end_seq;

	if (clone_it) {
		/* pskb_copy is necessary here, because the TCP/IP-headers
		 * will be changed when it's going to be reinjected on another
		 * subflow.
		 */
		skb = pskb_copy_for_clone(orig_skb, GFP_ATOMIC);
	} else {
		__skb_unlink(orig_skb, &sk->sk_write_queue);
		sock_set_flag(sk, SOCK_QUEUE_SHRUNK);
		sk->sk_wmem_queued -= orig_skb->truesize;
		sk_mem_uncharge(sk, orig_skb->truesize);
		skb = orig_skb;
	}
	if (unlikely(!skb))
		return;

	if (sk && !mptcp_reconstruct_mapping(skb)) {
		__kfree_skb(skb);
		return;
	}

	skb->sk = meta_sk;

	/* If it reached already the destination, we don't have to reinject it */
	if (!after(TCP_SKB_CB(skb)->end_seq, meta_tp->snd_una)) {
		__kfree_skb(skb);
		return;
	}

	/* Only reinject segments that are fully covered by the mapping */
	if (skb->len + (mptcp_is_data_fin(skb) ? 1 : 0) !=
	    TCP_SKB_CB(skb)->end_seq - TCP_SKB_CB(skb)->seq) {
		u32 seq = TCP_SKB_CB(skb)->seq;
		u32 end_seq = TCP_SKB_CB(skb)->end_seq;

		__kfree_skb(skb);

		/* Ok, now we have to look for the full mapping in the meta
		 * send-queue :S
		 */
		tcp_for_write_queue(skb, meta_sk) {
			/* Not yet at the mapping? */
			if (before(TCP_SKB_CB(skb)->seq, seq))
				continue;
			/* We have passed by the mapping */
			if (after(TCP_SKB_CB(skb)->end_seq, end_seq))
				return;

			__mptcp_reinject_data(skb, meta_sk, NULL, 1);
		}
		return;
	}

	/* Segment goes back to the MPTCP-layer. So, we need to zero the
	 * path_mask/dss.
	 */
	memset(TCP_SKB_CB(skb)->dss, 0 , mptcp_dss_len);

	/* We need to find out the path-mask from the meta-write-queue
	 * to properly select a subflow.
	 */
	mptcp_find_and_set_pathmask(meta_sk, skb);

	/* If it's empty, just add */
	if (skb_queue_empty(&mpcb->reinject_queue)) {
		skb_queue_head(&mpcb->reinject_queue, skb);
		return;
	}

	/* Find place to insert skb - or even we can 'drop' it, as the
	 * data is already covered by other skb's in the reinject-queue.
	 *
	 * This is inspired by code from tcp_data_queue.
	 */

	skb1 = skb_peek_tail(&mpcb->reinject_queue);
	seq = TCP_SKB_CB(skb)->seq;
	while (1) {
		if (!after(TCP_SKB_CB(skb1)->seq, seq))
			break;
		if (skb_queue_is_first(&mpcb->reinject_queue, skb1)) {
			skb1 = NULL;
			break;
		}
		skb1 = skb_queue_prev(&mpcb->reinject_queue, skb1);
	}

	/* Do skb overlap to previous one? */
	end_seq = TCP_SKB_CB(skb)->end_seq;
	if (skb1 && before(seq, TCP_SKB_CB(skb1)->end_seq)) {
		if (!after(end_seq, TCP_SKB_CB(skb1)->end_seq)) {
			/* All the bits are present. Don't reinject */
			__kfree_skb(skb);
			return;
		}
		if (seq == TCP_SKB_CB(skb1)->seq) {
			if (skb_queue_is_first(&mpcb->reinject_queue, skb1))
				skb1 = NULL;
			else
				skb1 = skb_queue_prev(&mpcb->reinject_queue, skb1);
		}
	}
	if (!skb1)
		__skb_queue_head(&mpcb->reinject_queue, skb);
	else
		__skb_queue_after(&mpcb->reinject_queue, skb1, skb);

	/* And clean segments covered by new one as whole. */
	while (!skb_queue_is_last(&mpcb->reinject_queue, skb)) {
		skb1 = skb_queue_next(&mpcb->reinject_queue, skb);

		if (!after(end_seq, TCP_SKB_CB(skb1)->seq))
			break;

		__skb_unlink(skb1, &mpcb->reinject_queue);
		__kfree_skb(skb1);
	}
	return;
}

/* Inserts data into the reinject queue */
void mptcp_reinject_data(struct sock *sk, int clone_it)
{
	struct sk_buff *skb_it, *tmp;
	struct tcp_sock *tp = tcp_sk(sk);
	struct sock *meta_sk = tp->meta_sk;

	/* It has already been closed - there is really no point in reinjecting */
	if (meta_sk->sk_state == TCP_CLOSE)
		return;

	skb_queue_walk_safe(&sk->sk_write_queue, skb_it, tmp) {
		struct tcp_skb_cb *tcb = TCP_SKB_CB(skb_it);
		/* Subflow syn's and fin's are not reinjected.
		 *
		 * As well as empty subflow-fins with a data-fin.
		 * They are reinjected below (without the subflow-fin-flag)
		 */
		if (tcb->tcp_flags & TCPHDR_SYN ||
		    (tcb->tcp_flags & TCPHDR_FIN && !mptcp_is_data_fin(skb_it)) ||
		    (tcb->tcp_flags & TCPHDR_FIN && mptcp_is_data_fin(skb_it) && !skb_it->len))
			continue;

		if (mptcp_is_reinjected(skb_it))
			continue;

		tcb->mptcp_flags |= MPTCP_REINJECT;
		__mptcp_reinject_data(skb_it, meta_sk, sk, clone_it);
	}

	skb_it = tcp_write_queue_tail(meta_sk);
	/* If sk has sent the empty data-fin, we have to reinject it too. */
	if (skb_it && mptcp_is_data_fin(skb_it) && skb_it->len == 0 &&
	    TCP_SKB_CB(skb_it)->path_mask & mptcp_pi_to_flag(tp->mptcp->path_index)) {
		__mptcp_reinject_data(skb_it, meta_sk, NULL, 1);
	}

	tp->pf = 1;

	mptcp_push_pending_frames(meta_sk);
}
EXPORT_SYMBOL(mptcp_reinject_data);

static void mptcp_combine_dfin(const struct sk_buff *skb,
			       const struct sock *meta_sk,
			       struct sock *subsk)
{
	const struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	const struct mptcp_cb *mpcb = meta_tp->mpcb;

	/* In infinite mapping we always try to combine */
	if (mpcb->infinite_mapping_snd)
		goto combine;

	/* Don't combine, if they didn't combine when closing - otherwise we end
	 * up in TIME_WAIT, even if our app is smart enough to avoid it.
	 */
	if (!mptcp_sk_can_recv(meta_sk) && !mpcb->dfin_combined)
		return;

	/* Don't combine if there is still outstanding data that remains to be
	 * DATA_ACKed, because otherwise we may never be able to deliver this.
	 */
	if (meta_tp->snd_una != TCP_SKB_CB(skb)->seq)
		return;

combine:
	if (tcp_close_state(subsk)) {
		subsk->sk_shutdown |= SEND_SHUTDOWN;
		TCP_SKB_CB(skb)->tcp_flags |= TCPHDR_FIN;
	}
}

static int mptcp_write_dss_mapping(const struct tcp_sock *tp, const struct sk_buff *skb,
				   __be32 *ptr)
{
	const struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);
	__be32 *start = ptr;
	__u16 data_len;

	*ptr++ = htonl(tcb->seq); /* data_seq */

	/* If it's a non-data DATA_FIN, we set subseq to 0 (draft v7) */
	if (mptcp_is_data_fin(skb) && skb->len == 0)
		*ptr++ = 0; /* subseq */
	else
		*ptr++ = htonl(tp->write_seq - tp->mptcp->snt_isn); /* subseq */

	if (tcb->mptcp_flags & MPTCPHDR_INF)
		data_len = 0;
	else
		data_len = tcb->end_seq - tcb->seq;

	if (tp->mpcb->dss_csum && data_len) {
		__be16 *p16 = (__be16 *)ptr;
		__be32 hdseq = mptcp_get_highorder_sndbits(skb, tp->mpcb);
		__wsum csum;

		*ptr = htonl(((data_len) << 16) |
			     (TCPOPT_EOL << 8) |
			     (TCPOPT_EOL));
		csum = csum_partial(ptr - 2, 12, skb->csum);
		p16++;
		*p16++ = csum_fold(csum_partial(&hdseq, sizeof(hdseq), csum));
	} else {
		*ptr++ = htonl(((data_len) << 16) |
			       (TCPOPT_NOP << 8) |
			       (TCPOPT_NOP));
	}

	return ptr - start;
}

static int mptcp_write_dss_data_ack(const struct tcp_sock *tp, const struct sk_buff *skb,
				    __be32 *ptr)
{
	struct mp_dss *mdss = (struct mp_dss *)ptr;
	__be32 *start = ptr;

	mdss->kind = TCPOPT_MPTCP;
	mdss->sub = MPTCP_SUB_DSS;
	mdss->rsv1 = 0;
	mdss->rsv2 = 0;
	mdss->F = mptcp_is_data_fin(skb) ? 1 : 0;
	mdss->m = 0;
	mdss->M = mptcp_is_data_seq(skb) ? 1 : 0;
	mdss->a = 0;
	mdss->A = 1;
	mdss->len = mptcp_sub_len_dss(mdss, tp->mpcb->dss_csum);
	ptr++;

	*ptr++ = htonl(mptcp_meta_tp(tp)->rcv_nxt);

	return ptr - start;
}

/* RFC6824 states that once a particular subflow mapping has been sent
 * out it must never be changed. However, packets may be split while
 * they are in the retransmission queue (due to SACK or ACKs) and that
 * arguably means that we would change the mapping (e.g. it splits it,
 * our sends out a subset of the initial mapping).
 *
 * Furthermore, the skb checksum is not always preserved across splits
 * (e.g. mptcp_fragment) which would mean that we need to recompute
 * the DSS checksum in this case.
 *
 * To avoid this we save the initial DSS mapping which allows us to
 * send the same DSS mapping even for fragmented retransmits.
 */
static void mptcp_save_dss_data_seq(const struct tcp_sock *tp, struct sk_buff *skb)
{
	struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);
	__be32 *ptr = (__be32 *)tcb->dss;

	tcb->mptcp_flags |= MPTCPHDR_SEQ;

	ptr += mptcp_write_dss_data_ack(tp, skb, ptr);
	ptr += mptcp_write_dss_mapping(tp, skb, ptr);
}

/* Write the saved DSS mapping to the header */
static int mptcp_write_dss_data_seq(const struct tcp_sock *tp, struct sk_buff *skb,
				    __be32 *ptr)
{
	__be32 *start = ptr;

	memcpy(ptr, TCP_SKB_CB(skb)->dss, mptcp_dss_len);

	/* update the data_ack */
	start[1] = htonl(mptcp_meta_tp(tp)->rcv_nxt);

	/* dss is in a union with inet_skb_parm and
	 * the IP layer expects zeroed IPCB fields.
	 */
	memset(TCP_SKB_CB(skb)->dss, 0 , mptcp_dss_len);

	return mptcp_dss_len/sizeof(*ptr);
}

static bool mptcp_skb_entail(struct sock *sk, struct sk_buff *skb, int reinject)
{
	struct tcp_sock *tp = tcp_sk(sk);
	const struct sock *meta_sk = mptcp_meta_sk(sk);
	const struct mptcp_cb *mpcb = tp->mpcb;
	struct tcp_skb_cb *tcb;
	struct sk_buff *subskb = NULL;

	if (!reinject)
		TCP_SKB_CB(skb)->mptcp_flags |= (mpcb->snd_hiseq_index ?
						  MPTCPHDR_SEQ64_INDEX : 0);

	subskb = pskb_copy_for_clone(skb, GFP_ATOMIC);
	if (!subskb)
		return false;

	/* At the subflow-level we need to call again tcp_init_tso_segs. We
	 * force this, by setting pcount to 0. It has been set to 1 prior to
	 * the call to mptcp_skb_entail.
	 */
	tcp_skb_pcount_set(subskb, 0);

	TCP_SKB_CB(skb)->path_mask |= mptcp_pi_to_flag(tp->mptcp->path_index);

	if (!(sk->sk_route_caps & NETIF_F_ALL_CSUM) &&
	    skb->ip_summed == CHECKSUM_PARTIAL) {
		subskb->csum = skb->csum = skb_checksum(skb, 0, skb->len, 0);
		subskb->ip_summed = skb->ip_summed = CHECKSUM_NONE;
	}

	tcb = TCP_SKB_CB(subskb);

	if (tp->mpcb->send_infinite_mapping &&
	    !tp->mpcb->infinite_mapping_snd &&
	    !before(tcb->seq, mptcp_meta_tp(tp)->snd_nxt)) {
		tp->mptcp->fully_established = 1;
		tp->mpcb->infinite_mapping_snd = 1;
		tp->mptcp->infinite_cutoff_seq = tp->write_seq;
		tcb->mptcp_flags |= MPTCPHDR_INF;
	}

	if (mptcp_is_data_fin(subskb))
		mptcp_combine_dfin(subskb, meta_sk, sk);

	mptcp_save_dss_data_seq(tp, subskb);

	tcb->seq = tp->write_seq;
	tcb->sacked = 0; /* reset the sacked field: from the point of view
			  * of this subflow, we are sending a brand new
			  * segment
			  */
	/* Take into account seg len */
	tp->write_seq += subskb->len + ((tcb->tcp_flags & TCPHDR_FIN) ? 1 : 0);
	tcb->end_seq = tp->write_seq;

	/* If it's a non-payload DATA_FIN (also no subflow-fin), the
	 * segment is not part of the subflow but on a meta-only-level.
	 */
	if (!mptcp_is_data_fin(subskb) || tcb->end_seq != tcb->seq) {
		tcp_add_write_queue_tail(sk, subskb);
		sk->sk_wmem_queued += subskb->truesize;
		sk_mem_charge(sk, subskb->truesize);
	} else {
		int err;

		/* Necessary to initialize for tcp_transmit_skb. mss of 1, as
		 * skb->len = 0 will force tso_segs to 1.
		 */
		tcp_init_tso_segs(sk, subskb, 1);
		/* Empty data-fins are sent immediatly on the subflow */
		err = tcp_transmit_skb(sk, subskb, 1, GFP_ATOMIC);

		/* It has not been queued, we can free it now. */
		kfree_skb(subskb);

		if (err)
			return false;
	}

	if (!tp->mptcp->fully_established) {
		tp->mptcp->second_packet = 1;
		tp->mptcp->last_end_data_seq = TCP_SKB_CB(skb)->end_seq;
	}

	return true;
}

/* Fragment an skb and update the mptcp meta-data. Due to reinject, we
 * might need to undo some operations done by tcp_fragment.
 */
static int mptcp_fragment(struct sock *meta_sk, struct sk_buff *skb, u32 len,
			  gfp_t gfp, int reinject)
{
	int ret, diff, old_factor;
	struct sk_buff *buff;
	u8 flags;

	if (skb_headlen(skb) < len)
		diff = skb->len - len;
	else
		diff = skb->data_len;
	old_factor = tcp_skb_pcount(skb);

	/* The mss_now in tcp_fragment is used to set the tso_segs of the skb.
	 * At the MPTCP-level we do not care about the absolute value. All we
	 * care about is that it is set to 1 for accurate packets_out
	 * accounting.
	 */
	ret = tcp_fragment(meta_sk, skb, len, UINT_MAX, gfp);
	if (ret)
		return ret;

	buff = skb->next;

	flags = TCP_SKB_CB(skb)->mptcp_flags;
	TCP_SKB_CB(skb)->mptcp_flags = flags & ~(MPTCPHDR_FIN);
	TCP_SKB_CB(buff)->mptcp_flags = flags;
	TCP_SKB_CB(buff)->path_mask = TCP_SKB_CB(skb)->path_mask;

	/* If reinject == 1, the buff will be added to the reinject
	 * queue, which is currently not part of memory accounting. So
	 * undo the changes done by tcp_fragment and update the
	 * reinject queue. Also, undo changes to the packet counters.
	 */
	if (reinject == 1) {
		int undo = buff->truesize - diff;
		meta_sk->sk_wmem_queued -= undo;
		sk_mem_uncharge(meta_sk, undo);

		tcp_sk(meta_sk)->mpcb->reinject_queue.qlen++;
		meta_sk->sk_write_queue.qlen--;

		if (!before(tcp_sk(meta_sk)->snd_nxt, TCP_SKB_CB(buff)->end_seq)) {
			undo = old_factor - tcp_skb_pcount(skb) -
				tcp_skb_pcount(buff);
			if (undo)
				tcp_adjust_pcount(meta_sk, skb, -undo);
		}
	}

	return 0;
}

/* Inspired by tcp_write_wakeup */
int mptcp_write_wakeup(struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct sk_buff *skb;
	struct sock *sk_it;
	int ans = 0;

	if (meta_sk->sk_state == TCP_CLOSE)
		return -1;

	skb = tcp_send_head(meta_sk);
	if (skb &&
	    before(TCP_SKB_CB(skb)->seq, tcp_wnd_end(meta_tp))) {
		unsigned int mss;
		unsigned int seg_size = tcp_wnd_end(meta_tp) - TCP_SKB_CB(skb)->seq;
		struct sock *subsk = meta_tp->mpcb->sched_ops->get_subflow(meta_sk, skb, true);
		struct tcp_sock *subtp;
		if (!subsk)
			goto window_probe;
		subtp = tcp_sk(subsk);
		mss = tcp_current_mss(subsk);

		seg_size = min(tcp_wnd_end(meta_tp) - TCP_SKB_CB(skb)->seq,
			       tcp_wnd_end(subtp) - subtp->write_seq);

		if (before(meta_tp->pushed_seq, TCP_SKB_CB(skb)->end_seq))
			meta_tp->pushed_seq = TCP_SKB_CB(skb)->end_seq;

		/* We are probing the opening of a window
		 * but the window size is != 0
		 * must have been a result SWS avoidance ( sender )
		 */
		if (seg_size < TCP_SKB_CB(skb)->end_seq - TCP_SKB_CB(skb)->seq ||
		    skb->len > mss) {
			seg_size = min(seg_size, mss);
			TCP_SKB_CB(skb)->tcp_flags |= TCPHDR_PSH;
			if (mptcp_fragment(meta_sk, skb, seg_size,
					   GFP_ATOMIC, 0))
				return -1;
		} else if (!tcp_skb_pcount(skb)) {
			/* see mptcp_write_xmit on why we use UINT_MAX */
			tcp_set_skb_tso_segs(meta_sk, skb, UINT_MAX);
		}

		TCP_SKB_CB(skb)->tcp_flags |= TCPHDR_PSH;
		if (!mptcp_skb_entail(subsk, skb, 0))
			return -1;
		skb_mstamp_get(&skb->skb_mstamp);

		mptcp_check_sndseq_wrap(meta_tp, TCP_SKB_CB(skb)->end_seq -
						 TCP_SKB_CB(skb)->seq);
		tcp_event_new_data_sent(meta_sk, skb);

		__tcp_push_pending_frames(subsk, mss, TCP_NAGLE_PUSH);

		return 0;
	} else {
window_probe:
		if (between(meta_tp->snd_up, meta_tp->snd_una + 1,
			    meta_tp->snd_una + 0xFFFF)) {
			mptcp_for_each_sk(meta_tp->mpcb, sk_it) {
				if (mptcp_sk_can_send_ack(sk_it))
					tcp_xmit_probe_skb(sk_it, 1);
			}
		}

		/* At least one of the tcp_xmit_probe_skb's has to succeed */
		mptcp_for_each_sk(meta_tp->mpcb, sk_it) {
			int ret;

			if (!mptcp_sk_can_send_ack(sk_it))
				continue;

			ret = tcp_xmit_probe_skb(sk_it, 0);
			if (unlikely(ret > 0))
				ans = ret;
		}
		return ans;
	}
}

bool mptcp_write_xmit(struct sock *meta_sk, unsigned int mss_now, int nonagle,
		     int push_one, gfp_t gfp)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk), *subtp;
	struct sock *subsk = NULL;
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	struct sk_buff *skb;
	int reinject = 0;
	unsigned int sublimit;
	__u32 path_mask = 0;

	while ((skb = mpcb->sched_ops->next_segment(meta_sk, &reinject, &subsk,
						    &sublimit))) {
		unsigned int limit;

		subtp = tcp_sk(subsk);
		mss_now = tcp_current_mss(subsk);

		if (reinject == 1) {
			if (!after(TCP_SKB_CB(skb)->end_seq, meta_tp->snd_una)) {
				/* Segment already reached the peer, take the next one */
				__skb_unlink(skb, &mpcb->reinject_queue);
				__kfree_skb(skb);
				continue;
			}
		}

		/* If the segment was cloned (e.g. a meta retransmission),
		 * the header must be expanded/copied so that there is no
		 * corruption of TSO information.
		 */
		if (skb_unclone(skb, GFP_ATOMIC))
			break;

		if (unlikely(!tcp_snd_wnd_test(meta_tp, skb, mss_now)))
			break;

		/* Force tso_segs to 1 by using UINT_MAX.
		 * We actually don't care about the exact number of segments
		 * emitted on the subflow. We need just to set tso_segs, because
		 * we still need an accurate packets_out count in
		 * tcp_event_new_data_sent.
		 */
		tcp_set_skb_tso_segs(meta_sk, skb, UINT_MAX);

		/* Check for nagle, irregardless of tso_segs. If the segment is
		 * actually larger than mss_now (TSO segment), then
		 * tcp_nagle_check will have partial == false and always trigger
		 * the transmission.
		 * tcp_write_xmit has a TSO-level nagle check which is not
		 * subject to the MPTCP-level. It is based on the properties of
		 * the subflow, not the MPTCP-level.
		 */
		if (unlikely(!tcp_nagle_test(meta_tp, skb, mss_now,
					     (tcp_skb_is_last(meta_sk, skb) ?
					      nonagle : TCP_NAGLE_PUSH))))
			break;

		limit = mss_now;
		/* skb->len > mss_now is the equivalent of tso_segs > 1 in
		 * tcp_write_xmit. Otherwise split-point would return 0.
		 */
		if (skb->len > mss_now && !tcp_urg_mode(meta_tp))
			/* We limit the size of the skb so that it fits into the
			 * window. Call tcp_mss_split_point to avoid duplicating
			 * code.
			 * We really only care about fitting the skb into the
			 * window. That's why we use UINT_MAX. If the skb does
			 * not fit into the cwnd_quota or the NIC's max-segs
			 * limitation, it will be split by the subflow's
			 * tcp_write_xmit which does the appropriate call to
			 * tcp_mss_split_point.
			 */
			limit = tcp_mss_split_point(meta_sk, skb, mss_now,
						    UINT_MAX / mss_now,
						    nonagle);

		if (sublimit)
			limit = min(limit, sublimit);

		if (skb->len > limit &&
		    unlikely(mptcp_fragment(meta_sk, skb, limit, gfp, reinject)))
			break;

		if (!mptcp_skb_entail(subsk, skb, reinject))
			break;
		/* Nagle is handled at the MPTCP-layer, so
		 * always push on the subflow
		 */
		__tcp_push_pending_frames(subsk, mss_now, TCP_NAGLE_PUSH);
		path_mask |= mptcp_pi_to_flag(subtp->mptcp->path_index);
		skb_mstamp_get(&skb->skb_mstamp);

		if (!reinject) {
			mptcp_check_sndseq_wrap(meta_tp,
						TCP_SKB_CB(skb)->end_seq -
						TCP_SKB_CB(skb)->seq);
			tcp_event_new_data_sent(meta_sk, skb);
		}

		tcp_minshall_update(meta_tp, mss_now, skb);

		if (reinject > 0) {
			__skb_unlink(skb, &mpcb->reinject_queue);
			kfree_skb(skb);
		}

		if (push_one)
			break;
	}

	mptcp_for_each_sk(mpcb, subsk) {
		subtp = tcp_sk(subsk);

		if (!(path_mask & mptcp_pi_to_flag(subtp->mptcp->path_index)))
			continue;

		/* We have pushed data on this subflow. We ignore the call to
		 * cwnd_validate in tcp_write_xmit as is_cwnd_limited will never
		 * be true (we never push more than what the cwnd can accept).
		 * We need to ensure that we call tcp_cwnd_validate with
		 * is_cwnd_limited set to true if we have filled the cwnd.
		 */
		tcp_cwnd_validate(subsk, tcp_packets_in_flight(subtp) >=
				  subtp->snd_cwnd);
	}

	return !meta_tp->packets_out && tcp_send_head(meta_sk);
}

void mptcp_write_space(struct sock *sk)
{
	mptcp_push_pending_frames(mptcp_meta_sk(sk));
}

u32 __mptcp_select_window(struct sock *sk)
{
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct tcp_sock *tp = tcp_sk(sk), *meta_tp = mptcp_meta_tp(tp);
	struct sock *meta_sk = mptcp_meta_sk(sk);
	int mss, free_space, full_space, window;

	/* MSS for the peer's data.  Previous versions used mss_clamp
	 * here.  I don't know if the value based on our guesses
	 * of peer's MSS is better for the performance.  It's more correct
	 * but may be worse for the performance because of rcv_mss
	 * fluctuations.  --SAW  1998/11/1
	 */
	mss = icsk->icsk_ack.rcv_mss;
	free_space = tcp_space(meta_sk);
	full_space = min_t(int, meta_tp->window_clamp,
			tcp_full_space(meta_sk));

	if (mss > full_space)
		mss = full_space;

	if (free_space < (full_space >> 1)) {
		/* If free_space is decreasing due to mostly meta-level
		 * out-of-order packets, don't turn off the quick-ack mode.
		 */
		if (meta_tp->rcv_nxt - meta_tp->copied_seq > ((full_space - free_space) >> 1))
			icsk->icsk_ack.quick = 0;

		if (tcp_memory_pressure)
			/* TODO this has to be adapted when we support different
			 * MSS's among the subflows.
			 */
			meta_tp->rcv_ssthresh = min(meta_tp->rcv_ssthresh,
						    4U * meta_tp->advmss);

		if (free_space < mss)
			return 0;
	}

	if (free_space > meta_tp->rcv_ssthresh)
		free_space = meta_tp->rcv_ssthresh;

	/* Don't do rounding if we are using window scaling, since the
	 * scaled window will not line up with the MSS boundary anyway.
	 */
	window = meta_tp->rcv_wnd;
	if (tp->rx_opt.rcv_wscale) {
		window = free_space;

		/* Advertise enough space so that it won't get scaled away.
		 * Import case: prevent zero window announcement if
		 * 1<<rcv_wscale > mss.
		 */
		if (((window >> tp->rx_opt.rcv_wscale) << tp->
		     rx_opt.rcv_wscale) != window)
			window = (((window >> tp->rx_opt.rcv_wscale) + 1)
				  << tp->rx_opt.rcv_wscale);
	} else {
		/* Get the largest window that is a nice multiple of mss.
		 * Window clamp already applied above.
		 * If our current window offering is within 1 mss of the
		 * free space we just keep it. This prevents the divide
		 * and multiply from happening most of the time.
		 * We also don't do any window rounding when the free space
		 * is too small.
		 */
		if (window <= free_space - mss || window > free_space)
			window = (free_space / mss) * mss;
		else if (mss == full_space &&
			 free_space > window + (full_space >> 1))
			window = free_space;
	}

	return window;
}

void mptcp_syn_options(const struct sock *sk, struct tcp_out_options *opts,
		       unsigned *remaining)
{
	const struct tcp_sock *tp = tcp_sk(sk);

	opts->options |= OPTION_MPTCP;
	if (is_master_tp(tp)) {
		opts->mptcp_options |= OPTION_MP_CAPABLE | OPTION_TYPE_SYN;
		opts->mptcp_ver = tcp_sk(sk)->mptcp_ver;
		*remaining -= MPTCP_SUB_LEN_CAPABLE_SYN_ALIGN;
		opts->mp_capable.sender_key = tp->mptcp_loc_key;
		opts->dss_csum = !!sysctl_mptcp_checksum;
	} else {
		const struct mptcp_cb *mpcb = tp->mpcb;

		opts->mptcp_options |= OPTION_MP_JOIN | OPTION_TYPE_SYN;
		*remaining -= MPTCP_SUB_LEN_JOIN_SYN_ALIGN;
		opts->mp_join_syns.token = mpcb->mptcp_rem_token;
		opts->mp_join_syns.low_prio  = tp->mptcp->low_prio;
		opts->addr_id = tp->mptcp->loc_id;
		opts->mp_join_syns.sender_nonce = tp->mptcp->mptcp_loc_nonce;
	}
}

void mptcp_synack_options(struct request_sock *req,
			  struct tcp_out_options *opts, unsigned *remaining)
{
	struct mptcp_request_sock *mtreq;
	mtreq = mptcp_rsk(req);

	opts->options |= OPTION_MPTCP;
	/* MPCB not yet set - thus it's a new MPTCP-session */
	if (!mtreq->is_sub) {
		opts->mptcp_options |= OPTION_MP_CAPABLE | OPTION_TYPE_SYNACK;
		opts->mptcp_ver = mtreq->mptcp_ver;
		opts->mp_capable.sender_key = mtreq->mptcp_loc_key;
		opts->dss_csum = !!sysctl_mptcp_checksum || mtreq->dss_csum;
		*remaining -= MPTCP_SUB_LEN_CAPABLE_SYN_ALIGN;
	} else {
		opts->mptcp_options |= OPTION_MP_JOIN | OPTION_TYPE_SYNACK;
		opts->mp_join_syns.sender_truncated_mac =
				mtreq->mptcp_hash_tmac;
		opts->mp_join_syns.sender_nonce = mtreq->mptcp_loc_nonce;
		opts->mp_join_syns.low_prio = mtreq->low_prio;
		opts->addr_id = mtreq->loc_id;
		*remaining -= MPTCP_SUB_LEN_JOIN_SYNACK_ALIGN;
	}
}

void mptcp_established_options(struct sock *sk, struct sk_buff *skb,
			       struct tcp_out_options *opts, unsigned *size)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct mptcp_cb *mpcb = tp->mpcb;
	const struct tcp_skb_cb *tcb = skb ? TCP_SKB_CB(skb) : NULL;

	/* We are coming from tcp_current_mss with the meta_sk as an argument.
	 * It does not make sense to check for the options, because when the
	 * segment gets sent, another subflow will be chosen.
	 */
	if (!skb && is_meta_sk(sk))
		return;

	/* In fallback mp_fail-mode, we have to repeat it until the fallback
	 * has been done by the sender
	 */
	if (unlikely(tp->mptcp->send_mp_fail)) {
		opts->options |= OPTION_MPTCP;
		opts->mptcp_options |= OPTION_MP_FAIL;
		*size += MPTCP_SUB_LEN_FAIL;
	}

	if (unlikely(tp->send_mp_fclose)) {
		opts->options |= OPTION_MPTCP;
		opts->mptcp_options |= OPTION_MP_FCLOSE;
		opts->mp_capable.receiver_key = mpcb->mptcp_rem_key;
		*size += MPTCP_SUB_LEN_FCLOSE_ALIGN;
		return;
	}

	/* 1. If we are the sender of the infinite-mapping, we need the
	 *    MPTCPHDR_INF-flag, because a retransmission of the
	 *    infinite-announcment still needs the mptcp-option.
	 *
	 *    We need infinite_cutoff_seq, because retransmissions from before
	 *    the infinite-cutoff-moment still need the MPTCP-signalling to stay
	 *    consistent.
	 *
	 * 2. If we are the receiver of the infinite-mapping, we always skip
	 *    mptcp-options, because acknowledgments from before the
	 *    infinite-mapping point have already been sent out.
	 *
	 * I know, the whole infinite-mapping stuff is ugly...
	 *
	 * TODO: Handle wrapped data-sequence numbers
	 *       (even if it's very unlikely)
	 */
	if (unlikely(mpcb->infinite_mapping_snd) &&
	    ((mpcb->send_infinite_mapping && tcb &&
	      mptcp_is_data_seq(skb) &&
	      !(tcb->mptcp_flags & MPTCPHDR_INF) &&
	      !before(tcb->seq, tp->mptcp->infinite_cutoff_seq)) ||
	     !mpcb->send_infinite_mapping))
		return;

	if (unlikely(tp->mptcp->include_mpc)) {
		opts->options |= OPTION_MPTCP;
		opts->mptcp_options |= OPTION_MP_CAPABLE |
				       OPTION_TYPE_ACK;
		*size += MPTCP_SUB_LEN_CAPABLE_ACK_ALIGN;
		opts->mptcp_ver = mpcb->mptcp_ver;
		opts->mp_capable.sender_key = mpcb->mptcp_loc_key;
		opts->mp_capable.receiver_key = mpcb->mptcp_rem_key;
		opts->dss_csum = mpcb->dss_csum;

		if (skb)
			tp->mptcp->include_mpc = 0;
	}
	if (unlikely(tp->mptcp->pre_established)) {
		opts->options |= OPTION_MPTCP;
		opts->mptcp_options |= OPTION_MP_JOIN | OPTION_TYPE_ACK;
		*size += MPTCP_SUB_LEN_JOIN_ACK_ALIGN;
	}

	if (unlikely(mpcb->addr_signal) && mpcb->pm_ops->addr_signal) {
		mpcb->pm_ops->addr_signal(sk, size, opts, skb);
		if (opts->add_addr_v6)
			/* Skip subsequent options */
			return;
	}

	if (!tp->mptcp->include_mpc && !tp->mptcp->pre_established) {
		opts->options |= OPTION_MPTCP;
		opts->mptcp_options |= OPTION_DATA_ACK;
		/* If !skb, we come from tcp_current_mss and thus we always
		 * assume that the DSS-option will be set for the data-packet.
		 */
		if (skb && !mptcp_is_data_seq(skb)) {
			*size += MPTCP_SUB_LEN_ACK_ALIGN;
		} else {
			/* Doesn't matter, if csum included or not. It will be
			 * either 10 or 12, and thus aligned = 12
			 */
			*size += MPTCP_SUB_LEN_ACK_ALIGN +
				 MPTCP_SUB_LEN_SEQ_ALIGN;
		}

		*size += MPTCP_SUB_LEN_DSS_ALIGN;
	}

	if (unlikely(tp->mptcp->send_mp_prio) &&
	    MAX_TCP_OPTION_SPACE - *size >= MPTCP_SUB_LEN_PRIO_ALIGN) {
		opts->options |= OPTION_MPTCP;
		opts->mptcp_options |= OPTION_MP_PRIO;
		if (skb)
			tp->mptcp->send_mp_prio = 0;
		*size += MPTCP_SUB_LEN_PRIO_ALIGN;
	}

	return;
}

u16 mptcp_select_window(struct sock *sk)
{
	u16 new_win		= tcp_select_window(sk);
	struct tcp_sock *tp	= tcp_sk(sk);
	struct tcp_sock *meta_tp = mptcp_meta_tp(tp);

	meta_tp->rcv_wnd	= tp->rcv_wnd;
	meta_tp->rcv_wup	= meta_tp->rcv_nxt;

	return new_win;
}

void mptcp_options_write(__be32 *ptr, struct tcp_sock *tp,
			 const struct tcp_out_options *opts,
			 struct sk_buff *skb)
{
	if (unlikely(OPTION_MP_CAPABLE & opts->mptcp_options)) {
		struct mp_capable *mpc = (struct mp_capable *)ptr;

		mpc->kind = TCPOPT_MPTCP;

		if ((OPTION_TYPE_SYN & opts->mptcp_options) ||
		    (OPTION_TYPE_SYNACK & opts->mptcp_options)) {
			mpc->sender_key = opts->mp_capable.sender_key;
			mpc->len = MPTCP_SUB_LEN_CAPABLE_SYN;
			mpc->ver = opts->mptcp_ver;
			ptr += MPTCP_SUB_LEN_CAPABLE_SYN_ALIGN >> 2;
		} else if (OPTION_TYPE_ACK & opts->mptcp_options) {
			mpc->sender_key = opts->mp_capable.sender_key;
			mpc->receiver_key = opts->mp_capable.receiver_key;
			mpc->len = MPTCP_SUB_LEN_CAPABLE_ACK;
			mpc->ver = opts->mptcp_ver;
			ptr += MPTCP_SUB_LEN_CAPABLE_ACK_ALIGN >> 2;
		}

		mpc->sub = MPTCP_SUB_CAPABLE;
		mpc->a = opts->dss_csum;
		mpc->b = 0;
		mpc->rsv = 0;
		mpc->h = 1;
	}
	if (unlikely(OPTION_MP_JOIN & opts->mptcp_options)) {
		struct mp_join *mpj = (struct mp_join *)ptr;

		mpj->kind = TCPOPT_MPTCP;
		mpj->sub = MPTCP_SUB_JOIN;
		mpj->rsv = 0;

		if (OPTION_TYPE_SYN & opts->mptcp_options) {
			mpj->len = MPTCP_SUB_LEN_JOIN_SYN;
			mpj->u.syn.token = opts->mp_join_syns.token;
			mpj->u.syn.nonce = opts->mp_join_syns.sender_nonce;
			mpj->b = opts->mp_join_syns.low_prio;
			mpj->addr_id = opts->addr_id;
			ptr += MPTCP_SUB_LEN_JOIN_SYN_ALIGN >> 2;
		} else if (OPTION_TYPE_SYNACK & opts->mptcp_options) {
			mpj->len = MPTCP_SUB_LEN_JOIN_SYNACK;
			mpj->u.synack.mac =
				opts->mp_join_syns.sender_truncated_mac;
			mpj->u.synack.nonce = opts->mp_join_syns.sender_nonce;
			mpj->b = opts->mp_join_syns.low_prio;
			mpj->addr_id = opts->addr_id;
			ptr += MPTCP_SUB_LEN_JOIN_SYNACK_ALIGN >> 2;
		} else if (OPTION_TYPE_ACK & opts->mptcp_options) {
			mpj->len = MPTCP_SUB_LEN_JOIN_ACK;
			mpj->addr_id = 0; /* addr_id is rsv (RFC 6824, p. 21) */
			memcpy(mpj->u.ack.mac, &tp->mptcp->sender_mac[0], 20);
			ptr += MPTCP_SUB_LEN_JOIN_ACK_ALIGN >> 2;
		}
	}
	if (unlikely(OPTION_ADD_ADDR & opts->mptcp_options)) {
		struct mp_add_addr *mpadd = (struct mp_add_addr *)ptr;
		struct mptcp_cb *mpcb = tp->mpcb;

		mpadd->kind = TCPOPT_MPTCP;
		if (opts->add_addr_v4) {
			mpadd->sub = MPTCP_SUB_ADD_ADDR;
			mpadd->ipver = 4;
			mpadd->addr_id = opts->add_addr4.addr_id;
			mpadd->u.v4.addr = opts->add_addr4.addr;
			if (mpcb->mptcp_ver < MPTCP_VERSION_1) {
				mpadd->len = MPTCP_SUB_LEN_ADD_ADDR4;
				ptr += MPTCP_SUB_LEN_ADD_ADDR4_ALIGN >> 2;
			} else {
				memcpy((char *)mpadd->u.v4.mac - 2,
				       (char *)&opts->add_addr4.trunc_mac, 8);
				mpadd->len = MPTCP_SUB_LEN_ADD_ADDR4_VER1;
				ptr += MPTCP_SUB_LEN_ADD_ADDR4_ALIGN_VER1 >> 2;
			}
		} else if (opts->add_addr_v6) {
			mpadd->sub = MPTCP_SUB_ADD_ADDR;
			mpadd->ipver = 6;
			mpadd->addr_id = opts->add_addr6.addr_id;
			memcpy(&mpadd->u.v6.addr, &opts->add_addr6.addr,
			       sizeof(mpadd->u.v6.addr));
			if (mpcb->mptcp_ver < MPTCP_VERSION_1) {
				mpadd->len = MPTCP_SUB_LEN_ADD_ADDR6;
				ptr += MPTCP_SUB_LEN_ADD_ADDR6_ALIGN >> 2;
			} else {
				memcpy((char *)mpadd->u.v6.mac - 2,
				       (char *)&opts->add_addr6.trunc_mac, 8);
				mpadd->len = MPTCP_SUB_LEN_ADD_ADDR6_VER1;
				ptr += MPTCP_SUB_LEN_ADD_ADDR6_ALIGN_VER1 >> 2;
			}
		}

		MPTCP_INC_STATS_BH(sock_net((struct sock *)tp), MPTCP_MIB_ADDADDRTX);
	}
	if (unlikely(OPTION_REMOVE_ADDR & opts->mptcp_options)) {
		struct mp_remove_addr *mprem = (struct mp_remove_addr *)ptr;
		u8 *addrs_id;
		int id, len, len_align;

		len = mptcp_sub_len_remove_addr(opts->remove_addrs);
		len_align = mptcp_sub_len_remove_addr_align(opts->remove_addrs);

		mprem->kind = TCPOPT_MPTCP;
		mprem->len = len;
		mprem->sub = MPTCP_SUB_REMOVE_ADDR;
		mprem->rsv = 0;
		addrs_id = &mprem->addrs_id;

		mptcp_for_each_bit_set(opts->remove_addrs, id)
			*(addrs_id++) = id;

		/* Fill the rest with NOP's */
		if (len_align > len) {
			int i;
			for (i = 0; i < len_align - len; i++)
				*(addrs_id++) = TCPOPT_NOP;
		}

		ptr += len_align >> 2;

		MPTCP_INC_STATS_BH(sock_net((struct sock *)tp), MPTCP_MIB_REMADDRTX);
	}
	if (unlikely(OPTION_MP_FAIL & opts->mptcp_options)) {
		struct mp_fail *mpfail = (struct mp_fail *)ptr;

		mpfail->kind = TCPOPT_MPTCP;
		mpfail->len = MPTCP_SUB_LEN_FAIL;
		mpfail->sub = MPTCP_SUB_FAIL;
		mpfail->rsv1 = 0;
		mpfail->rsv2 = 0;
		mpfail->data_seq = htonll(tp->mpcb->csum_cutoff_seq);

		ptr += MPTCP_SUB_LEN_FAIL_ALIGN >> 2;
	}
	if (unlikely(OPTION_MP_FCLOSE & opts->mptcp_options)) {
		struct mp_fclose *mpfclose = (struct mp_fclose *)ptr;

		mpfclose->kind = TCPOPT_MPTCP;
		mpfclose->len = MPTCP_SUB_LEN_FCLOSE;
		mpfclose->sub = MPTCP_SUB_FCLOSE;
		mpfclose->rsv1 = 0;
		mpfclose->rsv2 = 0;
		mpfclose->key = opts->mp_capable.receiver_key;

		ptr += MPTCP_SUB_LEN_FCLOSE_ALIGN >> 2;
	}

	if (OPTION_DATA_ACK & opts->mptcp_options) {
		if (!mptcp_is_data_seq(skb))
			ptr += mptcp_write_dss_data_ack(tp, skb, ptr);
		else
			ptr += mptcp_write_dss_data_seq(tp, skb, ptr);
	}
	if (unlikely(OPTION_MP_PRIO & opts->mptcp_options)) {
		struct mp_prio *mpprio = (struct mp_prio *)ptr;

		mpprio->kind = TCPOPT_MPTCP;
		mpprio->len = MPTCP_SUB_LEN_PRIO;
		mpprio->sub = MPTCP_SUB_PRIO;
		mpprio->rsv = 0;
		mpprio->b = tp->mptcp->low_prio;
		mpprio->addr_id = TCPOPT_NOP;

		ptr += MPTCP_SUB_LEN_PRIO_ALIGN >> 2;
	}
}

/* Sends the datafin */
void mptcp_send_fin(struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct sk_buff *skb = tcp_write_queue_tail(meta_sk);
	int mss_now;

	if ((1 << meta_sk->sk_state) & (TCPF_CLOSE_WAIT | TCPF_LAST_ACK))
		meta_tp->mpcb->passive_close = 1;

	/* Optimization, tack on the FIN if we have a queue of
	 * unsent frames.  But be careful about outgoing SACKS
	 * and IP options.
	 */
	mss_now = mptcp_current_mss(meta_sk);

	if (tcp_send_head(meta_sk) != NULL) {
		TCP_SKB_CB(skb)->mptcp_flags |= MPTCPHDR_FIN;
		TCP_SKB_CB(skb)->end_seq++;
		meta_tp->write_seq++;
	} else {
		/* Socket is locked, keep trying until memory is available. */
		for (;;) {
			skb = alloc_skb_fclone(MAX_TCP_HEADER,
					       meta_sk->sk_allocation);
			if (skb)
				break;
			yield();
		}
		/* Reserve space for headers and prepare control bits. */
		skb_reserve(skb, MAX_TCP_HEADER);

		tcp_init_nondata_skb(skb, meta_tp->write_seq, TCPHDR_ACK);
		TCP_SKB_CB(skb)->end_seq++;
		TCP_SKB_CB(skb)->mptcp_flags |= MPTCPHDR_FIN;
		tcp_queue_skb(meta_sk, skb);
	}
	__tcp_push_pending_frames(meta_sk, mss_now, TCP_NAGLE_OFF);
}

void mptcp_send_active_reset(struct sock *meta_sk, gfp_t priority)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	struct sock *sk;

	if (!mpcb->cnt_subflows)
		return;

	WARN_ON(meta_tp->send_mp_fclose);

	/* First - select a socket */
	sk = mptcp_select_ack_sock(meta_sk);

	/* May happen if no subflow is in an appropriate state, OR
	 * we are in infinite mode or about to go there - just send a reset */
	if (!sk || mpcb->infinite_mapping_snd || mpcb->send_infinite_mapping ||
	    mpcb->infinite_mapping_rcv) {

		/* tcp_done must be handled with bh disabled */
		if (!in_serving_softirq())
			local_bh_disable();

		mptcp_sub_force_close_all(mpcb, NULL);

		if (!in_serving_softirq())
			local_bh_enable();
		return;
	}


	tcp_sk(sk)->send_mp_fclose = 1;
	/** Reset all other subflows */

	/* tcp_done must be handled with bh disabled */
	if (!in_serving_softirq())
		local_bh_disable();

	mptcp_sub_force_close_all(mpcb, sk);

	if (!in_serving_softirq())
		local_bh_enable();

	tcp_send_ack(sk);
	inet_csk_reset_keepalive_timer(sk, inet_csk(sk)->icsk_rto);

	meta_tp->send_mp_fclose = 1;

	MPTCP_INC_STATS(sock_net(meta_sk), MPTCP_MIB_FASTCLOSETX);
}

static void mptcp_ack_retransmit_timer(struct sock *sk)
{
	struct sk_buff *skb;
	struct tcp_sock *tp = tcp_sk(sk);
	struct inet_connection_sock *icsk = inet_csk(sk);

	if (inet_csk(sk)->icsk_af_ops->rebuild_header(sk))
		goto out; /* Routing failure or similar */

	if (!tp->retrans_stamp)
		tp->retrans_stamp = tcp_time_stamp ? : 1;

	if (tcp_write_timeout(sk)) {
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_JOINACKRTO);
		tp->mptcp->pre_established = 0;
		sk_stop_timer(sk, &tp->mptcp->mptcp_ack_timer);
		tp->ops->send_active_reset(sk, GFP_ATOMIC);
		goto out;
	}

	skb = alloc_skb(MAX_TCP_HEADER, GFP_ATOMIC);
	if (skb == NULL) {
		sk_reset_timer(sk, &tp->mptcp->mptcp_ack_timer,
			       jiffies + icsk->icsk_rto);
		return;
	}

	/* Reserve space for headers and prepare control bits */
	skb_reserve(skb, MAX_TCP_HEADER);
	tcp_init_nondata_skb(skb, tp->snd_una, TCPHDR_ACK);

	MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_JOINACKRXMIT);

	if (tcp_transmit_skb(sk, skb, 0, GFP_ATOMIC) > 0) {
		/* Retransmission failed because of local congestion,
		 * do not backoff.
		 */
		if (!icsk->icsk_retransmits)
			icsk->icsk_retransmits = 1;
		sk_reset_timer(sk, &tp->mptcp->mptcp_ack_timer,
			       jiffies + icsk->icsk_rto);
		return;
	}


	icsk->icsk_retransmits++;
	icsk->icsk_rto = min(icsk->icsk_rto << 1, TCP_RTO_MAX);
	sk_reset_timer(sk, &tp->mptcp->mptcp_ack_timer,
		       jiffies + icsk->icsk_rto);
	if (retransmits_timed_out(sk, sysctl_tcp_retries1 + 1, 0, 0))
		__sk_dst_reset(sk);

out:;
}

void mptcp_ack_handler(unsigned long data)
{
	struct sock *sk = (struct sock *)data;
	struct sock *meta_sk = mptcp_meta_sk(sk);

	bh_lock_sock(meta_sk);
	if (sock_owned_by_user(meta_sk)) {
		/* Try again later */
		sk_reset_timer(sk, &tcp_sk(sk)->mptcp->mptcp_ack_timer,
			       jiffies + (HZ / 20));
		goto out_unlock;
	}

	if (sk->sk_state == TCP_CLOSE)
		goto out_unlock;
	if (!tcp_sk(sk)->mptcp->pre_established)
		goto out_unlock;

	mptcp_ack_retransmit_timer(sk);

	sk_mem_reclaim(sk);

out_unlock:
	bh_unlock_sock(meta_sk);
	sock_put(sk);
}

/* Similar to tcp_retransmit_skb
 *
 * The diff is that we handle the retransmission-stats (retrans_stamp) at the
 * meta-level.
 */
int mptcp_retransmit_skb(struct sock *meta_sk, struct sk_buff *skb)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct sock *subsk;
	unsigned int limit, mss_now;
	int err = -1;

	/* Do not sent more than we queued. 1/4 is reserved for possible
	 * copying overhead: fragmentation, tunneling, mangling etc.
	 *
	 * This is a meta-retransmission thus we check on the meta-socket.
	 */
	if (atomic_read(&meta_sk->sk_wmem_alloc) >
	    min(meta_sk->sk_wmem_queued + (meta_sk->sk_wmem_queued >> 2), meta_sk->sk_sndbuf)) {
		return -EAGAIN;
	}

	/* We need to make sure that the retransmitted segment can be sent on a
	 * subflow right now. If it is too big, it needs to be fragmented.
	 */
	subsk = meta_tp->mpcb->sched_ops->get_subflow(meta_sk, skb, false);
	if (!subsk) {
		/* We want to increase icsk_retransmits, thus return 0, so that
		 * mptcp_meta_retransmit_timer enters the desired branch.
		 */
		err = 0;
		goto failed;
	}
	mss_now = tcp_current_mss(subsk);

	/* If the segment was cloned (e.g. a meta retransmission), the header
	 * must be expanded/copied so that there is no corruption of TSO
	 * information.
	 */
	if (skb_unclone(skb, GFP_ATOMIC)) {
		err = -ENOMEM;
		goto failed;
	}

	/* Must have been set by mptcp_write_xmit before */
	BUG_ON(!tcp_skb_pcount(skb));

	limit = mss_now;
	/* skb->len > mss_now is the equivalent of tso_segs > 1 in
	 * tcp_write_xmit. Otherwise split-point would return 0.
	 */
	if (skb->len > mss_now && !tcp_urg_mode(meta_tp))
		limit = tcp_mss_split_point(meta_sk, skb, mss_now,
					    UINT_MAX / mss_now,
					    TCP_NAGLE_OFF);

	if (skb->len > limit &&
	    unlikely(mptcp_fragment(meta_sk, skb, limit,
				    GFP_ATOMIC, 0)))
		goto failed;

	if (!mptcp_skb_entail(subsk, skb, -1))
		goto failed;
	skb_mstamp_get(&skb->skb_mstamp);

	/* Update global TCP statistics. */
	MPTCP_INC_STATS_BH(sock_net(meta_sk), MPTCP_MIB_RETRANSSEGS);

	/* Diff to tcp_retransmit_skb */

	/* Save stamp of the first retransmit. */
	if (!meta_tp->retrans_stamp)
		meta_tp->retrans_stamp = tcp_skb_timestamp(skb);

	__tcp_push_pending_frames(subsk, mss_now, TCP_NAGLE_PUSH);

	return 0;

failed:
	NET_INC_STATS_BH(sock_net(meta_sk), LINUX_MIB_TCPRETRANSFAIL);
	return err;
}

/* Similar to tcp_retransmit_timer
 *
 * The diff is that we have to handle retransmissions of the FAST_CLOSE-message
 * and that we don't have an srtt estimation at the meta-level.
 */
void mptcp_meta_retransmit_timer(struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	struct inet_connection_sock *meta_icsk = inet_csk(meta_sk);
	int err;

	/* In fallback, retransmission is handled at the subflow-level */
	if (!meta_tp->packets_out || mpcb->infinite_mapping_snd)
		return;

	WARN_ON(tcp_write_queue_empty(meta_sk));

	if (!meta_tp->snd_wnd && !sock_flag(meta_sk, SOCK_DEAD) &&
	    !((1 << meta_sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV))) {
		/* Receiver dastardly shrinks window. Our retransmits
		 * become zero probes, but we should not timeout this
		 * connection. If the socket is an orphan, time it out,
		 * we cannot allow such beasts to hang infinitely.
		 */
		struct inet_sock *meta_inet = inet_sk(meta_sk);
		if (meta_sk->sk_family == AF_INET) {
			net_dbg_ratelimited("MPTCP: Peer %pI4:%u/%u unexpectedly shrunk window %u:%u (repaired)\n",
					    &meta_inet->inet_daddr,
					    ntohs(meta_inet->inet_dport),
					    meta_inet->inet_num, meta_tp->snd_una,
					    meta_tp->snd_nxt);
		}
#if IS_ENABLED(CONFIG_IPV6)
		else if (meta_sk->sk_family == AF_INET6) {
			net_dbg_ratelimited("MPTCP: Peer %pI6:%u/%u unexpectedly shrunk window %u:%u (repaired)\n",
					    &meta_sk->sk_v6_daddr,
					    ntohs(meta_inet->inet_dport),
					    meta_inet->inet_num, meta_tp->snd_una,
					    meta_tp->snd_nxt);
		}
#endif
		if (tcp_time_stamp - meta_tp->rcv_tstamp > TCP_RTO_MAX) {
			tcp_write_err(meta_sk);
			return;
		}

		mptcp_retransmit_skb(meta_sk, tcp_write_queue_head(meta_sk));
		goto out_reset_timer;
	}

	if (tcp_write_timeout(meta_sk))
		return;

	if (meta_icsk->icsk_retransmits == 0)
		NET_INC_STATS_BH(sock_net(meta_sk), LINUX_MIB_TCPTIMEOUTS);

	meta_icsk->icsk_ca_state = TCP_CA_Loss;

	err = mptcp_retransmit_skb(meta_sk, tcp_write_queue_head(meta_sk));
	if (err > 0) {
		/* Retransmission failed because of local congestion,
		 * do not backoff.
		 */
		if (!meta_icsk->icsk_retransmits)
			meta_icsk->icsk_retransmits = 1;
		inet_csk_reset_xmit_timer(meta_sk, ICSK_TIME_RETRANS,
					  min(meta_icsk->icsk_rto, TCP_RESOURCE_PROBE_INTERVAL),
					  TCP_RTO_MAX);
		return;
	}

	/* Increase the timeout each time we retransmit.  Note that
	 * we do not increase the rtt estimate.  rto is initialized
	 * from rtt, but increases here.  Jacobson (SIGCOMM 88) suggests
	 * that doubling rto each time is the least we can get away with.
	 * In KA9Q, Karn uses this for the first few times, and then
	 * goes to quadratic.  netBSD doubles, but only goes up to *64,
	 * and clamps at 1 to 64 sec afterwards.  Note that 120 sec is
	 * defined in the protocol as the maximum possible RTT.  I guess
	 * we'll have to use something other than TCP to talk to the
	 * University of Mars.
	 *
	 * PAWS allows us longer timeouts and large windows, so once
	 * implemented ftp to mars will work nicely. We will have to fix
	 * the 120 second clamps though!
	 */
	meta_icsk->icsk_backoff++;
	meta_icsk->icsk_retransmits++;

out_reset_timer:
	/* If stream is thin, use linear timeouts. Since 'icsk_backoff' is
	 * used to reset timer, set to 0. Recalculate 'icsk_rto' as this
	 * might be increased if the stream oscillates between thin and thick,
	 * thus the old value might already be too high compared to the value
	 * set by 'tcp_set_rto' in tcp_input.c which resets the rto without
	 * backoff. Limit to TCP_THIN_LINEAR_RETRIES before initiating
	 * exponential backoff behaviour to avoid continue hammering
	 * linear-timeout retransmissions into a black hole
	 */
	if (meta_sk->sk_state == TCP_ESTABLISHED &&
	    (meta_tp->thin_lto || sysctl_tcp_thin_linear_timeouts) &&
	    tcp_stream_is_thin(meta_tp) &&
	    meta_icsk->icsk_retransmits <= TCP_THIN_LINEAR_RETRIES) {
		meta_icsk->icsk_backoff = 0;
		/* We cannot do the same as in tcp_write_timer because the
		 * srtt is not set here.
		 */
		mptcp_set_rto(meta_sk);
	} else {
		/* Use normal (exponential) backoff */
		meta_icsk->icsk_rto = min(meta_icsk->icsk_rto << 1, TCP_RTO_MAX);
	}
	inet_csk_reset_xmit_timer(meta_sk, ICSK_TIME_RETRANS, meta_icsk->icsk_rto, TCP_RTO_MAX);

	return;
}

void mptcp_sub_retransmit_timer(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	tcp_retransmit_timer(sk);

	if (!tp->fastopen_rsk) {
		mptcp_reinject_data(sk, 1);
		mptcp_set_rto(sk);
	}
}

/* Modify values to an mptcp-level for the initial window of new subflows */
void mptcp_select_initial_window(int __space, __u32 mss, __u32 *rcv_wnd,
				__u32 *window_clamp, int wscale_ok,
				__u8 *rcv_wscale, __u32 init_rcv_wnd,
				 const struct sock *sk)
{
	struct mptcp_cb *mpcb = tcp_sk(sk)->mpcb;

	*window_clamp = mpcb->orig_window_clamp;
	__space = tcp_win_from_space(mpcb->orig_sk_rcvbuf);

	tcp_select_initial_window(__space, mss, rcv_wnd, window_clamp,
				  wscale_ok, rcv_wscale, init_rcv_wnd, sk);
}

static inline u64 mptcp_calc_rate(const struct sock *meta_sk, unsigned int mss,
				  unsigned int (*mss_cb)(struct sock *sk))
{
	struct sock *sk;
	u64 rate = 0;

	mptcp_for_each_sk(tcp_sk(meta_sk)->mpcb, sk) {
		struct tcp_sock *tp = tcp_sk(sk);
		int this_mss;
		u64 this_rate;

		if (!mptcp_sk_can_send(sk))
			continue;

		/* Do not consider subflows without a RTT estimation yet
		 * otherwise this_rate >>> rate.
		 */
		if (unlikely(!tp->srtt_us))
			continue;

		this_mss = mss_cb(sk);

		/* If this_mss is smaller than mss, it means that a segment will
		 * be splitted in two (or more) when pushed on this subflow. If
		 * you consider that mss = 1428 and this_mss = 1420 then two
		 * segments will be generated: a 1420-byte and 8-byte segment.
		 * The latter will introduce a large overhead as for a single
		 * data segment 2 slots will be used in the congestion window.
		 * Therefore reducing by ~2 the potential throughput of this
		 * subflow. Indeed, 1428 will be send while 2840 could have been
		 * sent if mss == 1420 reducing the throughput by 2840 / 1428.
		 *
		 * The following algorithm take into account this overhead
		 * when computing the potential throughput that MPTCP can
		 * achieve when generating mss-byte segments.
		 *
		 * The formulae is the following:
		 *  \sum_{\forall sub} ratio * \frac{mss * cwnd_sub}{rtt_sub}
		 * Where ratio is computed as follows:
		 *  \frac{mss}{\ceil{mss / mss_sub} * mss_sub}
		 *
		 * ratio gives the reduction factor of the theoretical
		 * throughput a subflow can achieve if MPTCP uses a specific
		 * MSS value.
		 */
		this_rate = div64_u64((u64)mss * mss * (USEC_PER_SEC << 3) *
				      max(tp->snd_cwnd, tp->packets_out),
				      (u64)tp->srtt_us *
				      DIV_ROUND_UP(mss, this_mss) * this_mss);
		rate += this_rate;
	}

	return rate;
}

static unsigned int __mptcp_current_mss(const struct sock *meta_sk,
					unsigned int (*mss_cb)(struct sock *sk))
{
	unsigned int mss = 0;
	u64 rate = 0;
	struct sock *sk;

	mptcp_for_each_sk(tcp_sk(meta_sk)->mpcb, sk) {
		int this_mss;
		u64 this_rate;

		if (!mptcp_sk_can_send(sk))
			continue;

		this_mss = mss_cb(sk);

		/* Same mss values will produce the same throughput. */
		if (this_mss == mss)
			continue;

		/* See whether using this mss value can theoretically improve
		 * the performances.
		 */
		this_rate = mptcp_calc_rate(meta_sk, this_mss, mss_cb);
		if (this_rate >= rate) {
			mss = this_mss;
			rate = this_rate;
		}
	}

	return mss;
}

unsigned int mptcp_current_mss(struct sock *meta_sk)
{
	unsigned int mss = __mptcp_current_mss(meta_sk, tcp_current_mss);

	/* If no subflow is available, we take a default-mss from the
	 * meta-socket.
	 */
	return !mss ? tcp_current_mss(meta_sk) : mss;
}

static unsigned int mptcp_select_size_mss(struct sock *sk)
{
	return tcp_sk(sk)->mss_cache;
}

int mptcp_select_size(const struct sock *meta_sk, bool sg)
{
	unsigned int mss = __mptcp_current_mss(meta_sk, mptcp_select_size_mss);

	if (sg) {
		if (mptcp_sk_can_gso(meta_sk)) {
			mss = SKB_WITH_OVERHEAD(2048 - MAX_TCP_HEADER);
		} else {
			int pgbreak = SKB_MAX_HEAD(MAX_TCP_HEADER);

			if (mss >= pgbreak &&
			    mss <= pgbreak + (MAX_SKB_FRAGS - 1) * PAGE_SIZE)
				mss = pgbreak;
		}
	}

	return !mss ? tcp_sk(meta_sk)->mss_cache : mss;
}

int mptcp_check_snd_buf(const struct tcp_sock *tp)
{
	const struct sock *sk;
	u32 rtt_max = tp->srtt_us;
	u64 bw_est;

	if (!tp->srtt_us)
		return tp->reordering + 1;

	mptcp_for_each_sk(tp->mpcb, sk) {
		if (!mptcp_sk_can_send(sk))
			continue;

		if (rtt_max < tcp_sk(sk)->srtt_us)
			rtt_max = tcp_sk(sk)->srtt_us;
	}

	bw_est = div64_u64(((u64)tp->snd_cwnd * rtt_max) << 16,
				(u64)tp->srtt_us);

	return max_t(unsigned int, (u32)(bw_est >> 16),
			tp->reordering + 1);
}

unsigned int mptcp_xmit_size_goal(const struct sock *meta_sk, u32 mss_now,
				  int large_allowed)
{
	struct sock *sk;
	u32 xmit_size_goal = 0;

	if (large_allowed && mptcp_sk_can_gso(meta_sk)) {
		mptcp_for_each_sk(tcp_sk(meta_sk)->mpcb, sk) {
			int this_size_goal;

			if (!mptcp_sk_can_send(sk))
				continue;

			this_size_goal = tcp_xmit_size_goal(sk, mss_now, 1);
			if (this_size_goal > xmit_size_goal)
				xmit_size_goal = this_size_goal;
		}
	}

	return max(xmit_size_goal, mss_now);
}

#endif
