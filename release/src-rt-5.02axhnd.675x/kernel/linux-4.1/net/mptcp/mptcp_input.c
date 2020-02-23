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

#include <asm/unaligned.h>

#include <net/mptcp.h>
#include <net/mptcp_v4.h>
#include <net/mptcp_v6.h>

#include <linux/kconfig.h>

/* is seq1 < seq2 ? */
static inline bool before64(const u64 seq1, const u64 seq2)
{
	return (s64)(seq1 - seq2) < 0;
}

/* is seq1 > seq2 ? */
#define after64(seq1, seq2)	before64(seq2, seq1)

static inline void mptcp_become_fully_estab(struct sock *sk)
{
	tcp_sk(sk)->mptcp->fully_established = 1;

	if (is_master_tp(tcp_sk(sk)) &&
	    tcp_sk(sk)->mpcb->pm_ops->fully_established)
		tcp_sk(sk)->mpcb->pm_ops->fully_established(mptcp_meta_sk(sk));
}

/* Similar to tcp_tso_acked without any memory accounting */
static inline int mptcp_tso_acked_reinject(const struct sock *meta_sk,
					   struct sk_buff *skb)
{
	const struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	u32 packets_acked, len;

	BUG_ON(!after(TCP_SKB_CB(skb)->end_seq, meta_tp->snd_una));

	packets_acked = tcp_skb_pcount(skb);

	if (skb_unclone(skb, GFP_ATOMIC))
		return 0;

	len = meta_tp->snd_una - TCP_SKB_CB(skb)->seq;
	__pskb_trim_head(skb, len);

	TCP_SKB_CB(skb)->seq += len;
	skb->ip_summed = CHECKSUM_PARTIAL;
	skb->truesize	     -= len;

	/* Any change of skb->len requires recalculation of tso factor. */
	if (tcp_skb_pcount(skb) > 1)
		tcp_set_skb_tso_segs(meta_sk, skb, tcp_skb_mss(skb));
	packets_acked -= tcp_skb_pcount(skb);

	if (packets_acked) {
		BUG_ON(tcp_skb_pcount(skb) == 0);
		BUG_ON(!before(TCP_SKB_CB(skb)->seq, TCP_SKB_CB(skb)->end_seq));
	}

	return packets_acked;
}

/**
 * Cleans the meta-socket retransmission queue and the reinject-queue.
 * @sk must be the metasocket.
 */
static void mptcp_clean_rtx_queue(struct sock *meta_sk, u32 prior_snd_una)
{
	struct sk_buff *skb, *tmp;
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	bool acked = false;
	u32 acked_pcount;

	while ((skb = tcp_write_queue_head(meta_sk)) &&
	       skb != tcp_send_head(meta_sk)) {
		bool fully_acked = true;

		if (before(meta_tp->snd_una, TCP_SKB_CB(skb)->end_seq)) {
			if (tcp_skb_pcount(skb) == 1 ||
			    !after(meta_tp->snd_una, TCP_SKB_CB(skb)->seq))
				break;

			acked_pcount = tcp_tso_acked(meta_sk, skb);
			if (!acked_pcount)
				break;

			fully_acked = false;
		} else {
			acked_pcount = tcp_skb_pcount(skb);
		}

		acked = true;
		meta_tp->packets_out -= acked_pcount;
		meta_tp->retrans_stamp = 0;

		if (!fully_acked)
			break;

		tcp_unlink_write_queue(skb, meta_sk);

		if (mptcp_is_data_fin(skb)) {
			struct sock *sk_it;

			/* DATA_FIN has been acknowledged - now we can close
			 * the subflows
			 */
			mptcp_for_each_sk(mpcb, sk_it) {
				unsigned long delay = 0;

				/* If we are the passive closer, don't trigger
				 * subflow-fin until the subflow has been finned
				 * by the peer - thus we add a delay.
				 */
				if (mpcb->passive_close &&
				    sk_it->sk_state == TCP_ESTABLISHED)
					delay = inet_csk(sk_it)->icsk_rto << 3;

				mptcp_sub_close(sk_it, delay);
			}
		}
		sk_wmem_free_skb(meta_sk, skb);
	}
	/* Remove acknowledged data from the reinject queue */
	skb_queue_walk_safe(&mpcb->reinject_queue, skb, tmp) {
		if (before(meta_tp->snd_una, TCP_SKB_CB(skb)->end_seq)) {
			if (tcp_skb_pcount(skb) == 1 ||
			    !after(meta_tp->snd_una, TCP_SKB_CB(skb)->seq))
				break;

			mptcp_tso_acked_reinject(meta_sk, skb);
			break;
		}

		__skb_unlink(skb, &mpcb->reinject_queue);
		__kfree_skb(skb);
	}

	if (likely(between(meta_tp->snd_up, prior_snd_una, meta_tp->snd_una)))
		meta_tp->snd_up = meta_tp->snd_una;

	if (acked) {
		tcp_rearm_rto(meta_sk);
		/* Normally this is done in tcp_try_undo_loss - but MPTCP
		 * does not call this function.
		 */
		inet_csk(meta_sk)->icsk_retransmits = 0;
	}
}

/* Inspired by tcp_rcv_state_process */
static int mptcp_rcv_state_process(struct sock *meta_sk, struct sock *sk,
				   const struct sk_buff *skb, u32 data_seq,
				   u16 data_len)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk), *tp = tcp_sk(sk);
	const struct tcphdr *th = tcp_hdr(skb);

	/* State-machine handling if FIN has been enqueued and he has
	 * been acked (snd_una == write_seq) - it's important that this
	 * here is after sk_wmem_free_skb because otherwise
	 * sk_forward_alloc is wrong upon inet_csk_destroy_sock()
	 */
	switch (meta_sk->sk_state) {
	case TCP_FIN_WAIT1: {
		struct dst_entry *dst;
		int tmo;

		if (meta_tp->snd_una != meta_tp->write_seq)
			break;

		tcp_set_state(meta_sk, TCP_FIN_WAIT2);
		meta_sk->sk_shutdown |= SEND_SHUTDOWN;

		dst = __sk_dst_get(sk);
		if (dst)
			dst_confirm(dst);

		if (!sock_flag(meta_sk, SOCK_DEAD)) {
			/* Wake up lingering close() */
			meta_sk->sk_state_change(meta_sk);
			break;
		}

		if (meta_tp->linger2 < 0 ||
		    (data_len &&
		     after(data_seq + data_len - (mptcp_is_data_fin2(skb, tp) ? 1 : 0),
			   meta_tp->rcv_nxt))) {
			mptcp_send_active_reset(meta_sk, GFP_ATOMIC);
			tcp_done(meta_sk);
			NET_INC_STATS_BH(sock_net(meta_sk), LINUX_MIB_TCPABORTONDATA);
			return 1;
		}

		tmo = tcp_fin_time(meta_sk);
		if (tmo > TCP_TIMEWAIT_LEN) {
			inet_csk_reset_keepalive_timer(meta_sk, tmo - TCP_TIMEWAIT_LEN);
		} else if (mptcp_is_data_fin2(skb, tp) || sock_owned_by_user(meta_sk)) {
			/* Bad case. We could lose such FIN otherwise.
			 * It is not a big problem, but it looks confusing
			 * and not so rare event. We still can lose it now,
			 * if it spins in bh_lock_sock(), but it is really
			 * marginal case.
			 */
			inet_csk_reset_keepalive_timer(meta_sk, tmo);
		} else {
			meta_tp->ops->time_wait(meta_sk, TCP_FIN_WAIT2, tmo);
		}
		break;
	}
	case TCP_CLOSING:
	case TCP_LAST_ACK:
		if (meta_tp->snd_una == meta_tp->write_seq) {
			tcp_done(meta_sk);
			return 1;
		}
		break;
	}

	/* step 7: process the segment text */
	switch (meta_sk->sk_state) {
	case TCP_FIN_WAIT1:
	case TCP_FIN_WAIT2:
		/* RFC 793 says to queue data in these states,
		 * RFC 1122 says we MUST send a reset.
		 * BSD 4.4 also does reset.
		 */
		if (meta_sk->sk_shutdown & RCV_SHUTDOWN) {
			if (TCP_SKB_CB(skb)->end_seq != TCP_SKB_CB(skb)->seq &&
			    after(TCP_SKB_CB(skb)->end_seq - th->fin, tp->rcv_nxt) &&
			    !mptcp_is_data_fin2(skb, tp)) {
				NET_INC_STATS_BH(sock_net(meta_sk), LINUX_MIB_TCPABORTONDATA);
				mptcp_send_active_reset(meta_sk, GFP_ATOMIC);
				tcp_reset(meta_sk);
				return 1;
			}
		}
		break;
	}

	return 0;
}

/**
 * @return:
 *  i) 1: Everything's fine.
 *  ii) -1: A reset has been sent on the subflow - csum-failure
 *  iii) 0: csum-failure but no reset sent, because it's the last subflow.
 *	 Last packet should not be destroyed by the caller because it has
 *	 been done here.
 */
static int mptcp_verif_dss_csum(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct sk_buff *tmp, *tmp1, *last = NULL;
	__wsum csum_tcp = 0; /* cumulative checksum of pld + mptcp-header */
	int ans = 1, overflowed = 0, offset = 0, dss_csum_added = 0;
	int iter = 0;

	skb_queue_walk_safe(&sk->sk_receive_queue, tmp, tmp1) {
		unsigned int csum_len;

		if (before(tp->mptcp->map_subseq + tp->mptcp->map_data_len, TCP_SKB_CB(tmp)->end_seq))
			/* Mapping ends in the middle of the packet -
			 * csum only these bytes
			 */
			csum_len = tp->mptcp->map_subseq + tp->mptcp->map_data_len - TCP_SKB_CB(tmp)->seq;
		else
			csum_len = tmp->len;

		offset = 0;
		if (overflowed) {
			char first_word[4];
			first_word[0] = 0;
			first_word[1] = 0;
			first_word[2] = 0;
			first_word[3] = *(tmp->data);
			csum_tcp = csum_partial(first_word, 4, csum_tcp);
			offset = 1;
			csum_len--;
			overflowed = 0;
		}

		csum_tcp = skb_checksum(tmp, offset, csum_len, csum_tcp);

		/* Was it on an odd-length? Then we have to merge the next byte
		 * correctly (see above)
		 */
		if (csum_len != (csum_len & (~1)))
			overflowed = 1;

		if (mptcp_is_data_seq(tmp) && !dss_csum_added) {
			__be32 data_seq = htonl((u32)(tp->mptcp->map_data_seq >> 32));

			/* If a 64-bit dss is present, we increase the offset
			 * by 4 bytes, as the high-order 64-bits will be added
			 * in the final csum_partial-call.
			 */
			u32 offset = skb_transport_offset(tmp) +
				     TCP_SKB_CB(tmp)->dss_off;
			if (TCP_SKB_CB(tmp)->mptcp_flags & MPTCPHDR_SEQ64_SET)
				offset += 4;

			csum_tcp = skb_checksum(tmp, offset,
						MPTCP_SUB_LEN_SEQ_CSUM,
						csum_tcp);

			csum_tcp = csum_partial(&data_seq,
						sizeof(data_seq), csum_tcp);

			dss_csum_added = 1; /* Just do it once */
		}
		last = tmp;
		iter++;

		if (!skb_queue_is_last(&sk->sk_receive_queue, tmp) &&
		    !before(TCP_SKB_CB(tmp1)->seq,
			    tp->mptcp->map_subseq + tp->mptcp->map_data_len))
			break;
	}

	/* Now, checksum must be 0 */
	if (unlikely(csum_fold(csum_tcp))) {
		pr_err("%s csum is wrong: %#x data_seq %u dss_csum_added %d overflowed %d iterations %d\n",
		       __func__, csum_fold(csum_tcp), TCP_SKB_CB(last)->seq,
		       dss_csum_added, overflowed, iter);

		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_CSUMFAIL);
		tp->mptcp->send_mp_fail = 1;

		/* map_data_seq is the data-seq number of the
		 * mapping we are currently checking
		 */
		tp->mpcb->csum_cutoff_seq = tp->mptcp->map_data_seq;

		if (tp->mpcb->cnt_subflows > 1) {
			mptcp_send_reset(sk);
			ans = -1;
		} else {
			tp->mpcb->send_infinite_mapping = 1;

			/* Need to purge the rcv-queue as it's no more valid */
			while ((tmp = __skb_dequeue(&sk->sk_receive_queue)) != NULL) {
				tp->copied_seq = TCP_SKB_CB(tmp)->end_seq;
				kfree_skb(tmp);
			}

			ans = 0;
		}
	}

	return ans;
}

static inline void mptcp_prepare_skb(struct sk_buff *skb,
				     const struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);
	u32 inc = 0, end_seq = tcb->end_seq;

	if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN)
		end_seq--;
	/* If skb is the end of this mapping (end is always at mapping-boundary
	 * thanks to the splitting/trimming), then we need to increase
	 * data-end-seq by 1 if this here is a data-fin.
	 *
	 * We need to do -1 because end_seq includes the subflow-FIN.
	 */
	if (tp->mptcp->map_data_fin &&
	    end_seq == tp->mptcp->map_subseq + tp->mptcp->map_data_len) {
		inc = 1;

		/* We manually set the fin-flag if it is a data-fin. For easy
		 * processing in tcp_recvmsg.
		 */
		TCP_SKB_CB(skb)->tcp_flags |= TCPHDR_FIN;
	} else {
		/* We may have a subflow-fin with data but without data-fin */
		TCP_SKB_CB(skb)->tcp_flags &= ~TCPHDR_FIN;
	}

	/* Adapt data-seq's to the packet itself. We kinda transform the
	 * dss-mapping to a per-packet granularity. This is necessary to
	 * correctly handle overlapping mappings coming from different
	 * subflows. Otherwise it would be a complete mess.
	 */
	tcb->seq = ((u32)tp->mptcp->map_data_seq) + tcb->seq - tp->mptcp->map_subseq;
	tcb->end_seq = tcb->seq + skb->len + inc;
}

/**
 * @return: 1 if the segment has been eaten and can be suppressed,
 *          otherwise 0.
 */
static inline int mptcp_direct_copy(const struct sk_buff *skb,
				    struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	int chunk = min_t(unsigned int, skb->len, meta_tp->ucopy.len);
	int eaten = 0;

	__set_current_state(TASK_RUNNING);

	local_bh_enable();
	if (!skb_copy_datagram_msg(skb, 0, meta_tp->ucopy.msg, chunk)) {
		meta_tp->ucopy.len -= chunk;
		meta_tp->copied_seq += chunk;
		eaten = (chunk == skb->len);
		tcp_rcv_space_adjust(meta_sk);
	}
	local_bh_disable();
	return eaten;
}

static inline void mptcp_reset_mapping(struct tcp_sock *tp, u32 old_copied_seq)
{
	tp->mptcp->map_data_len = 0;
	tp->mptcp->map_data_seq = 0;
	tp->mptcp->map_subseq = 0;
	tp->mptcp->map_data_fin = 0;
	tp->mptcp->mapping_present = 0;

	/* In infinite mapping receiver mode, we have to advance the implied
	 * data-sequence number when we progress the subflow's data.
	 */
	if (tp->mpcb->infinite_mapping_rcv)
		tp->mpcb->infinite_rcv_seq += (tp->copied_seq - old_copied_seq);
}

/* The DSS-mapping received on the sk only covers the second half of the skb
 * (cut at seq). We trim the head from the skb.
 * Data will be freed upon kfree().
 *
 * Inspired by tcp_trim_head().
 */
static void mptcp_skb_trim_head(struct sk_buff *skb, struct sock *sk, u32 seq)
{
	int len = seq - TCP_SKB_CB(skb)->seq;
	u32 new_seq = TCP_SKB_CB(skb)->seq + len;

	__pskb_trim_head(skb, len);

	TCP_SKB_CB(skb)->seq = new_seq;

	skb->truesize -= len;
	atomic_sub(len, &sk->sk_rmem_alloc);
	sk_mem_uncharge(sk, len);
}

/* The DSS-mapping received on the sk only covers the first half of the skb
 * (cut at seq). We create a second skb (@return), and queue it in the rcv-queue
 * as further packets may resolve the mapping of the second half of data.
 *
 * Inspired by tcp_fragment().
 */
static int mptcp_skb_split_tail(struct sk_buff *skb, struct sock *sk, u32 seq)
{
	struct sk_buff *buff;
	int nsize;
	int nlen, len;
	u8 flags;

	len = seq - TCP_SKB_CB(skb)->seq;
	nsize = skb_headlen(skb) - len + tcp_sk(sk)->tcp_header_len;
	if (nsize < 0)
		nsize = 0;

	/* Get a new skb... force flag on. */
	buff = alloc_skb(nsize, GFP_ATOMIC);
	if (buff == NULL)
		return -ENOMEM;

	skb_reserve(buff, tcp_sk(sk)->tcp_header_len);
	skb_reset_transport_header(buff);

	flags = TCP_SKB_CB(skb)->tcp_flags;
	TCP_SKB_CB(skb)->tcp_flags = flags & ~(TCPHDR_FIN);
	TCP_SKB_CB(buff)->tcp_flags = flags;

	/* We absolutly need to call skb_set_owner_r before refreshing the
	 * truesize of buff, otherwise the moved data will account twice.
	 */
	skb_set_owner_r(buff, sk);
	nlen = skb->len - len - nsize;
	buff->truesize += nlen;
	skb->truesize -= nlen;

	/* Correct the sequence numbers. */
	TCP_SKB_CB(buff)->seq = TCP_SKB_CB(skb)->seq + len;
	TCP_SKB_CB(buff)->end_seq = TCP_SKB_CB(skb)->end_seq;
	TCP_SKB_CB(skb)->end_seq = TCP_SKB_CB(buff)->seq;

	skb_split(skb, buff, len);

	__skb_queue_after(&sk->sk_receive_queue, skb, buff);

	return 0;
}

/* @return: 0  everything is fine. Just continue processing
 *	    1  subflow is broken stop everything
 *	    -1 this packet was broken - continue with the next one.
 */
static int mptcp_prevalidate_skb(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct mptcp_cb *mpcb = tp->mpcb;

	/* If we are in infinite mode, the subflow-fin is in fact a data-fin. */
	if (!skb->len && (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN) &&
	    !mptcp_is_data_fin(skb) && !mpcb->infinite_mapping_rcv) {
		/* Remove a pure subflow-fin from the queue and increase
		 * copied_seq.
		 */
		tp->copied_seq = TCP_SKB_CB(skb)->end_seq;
		__skb_unlink(skb, &sk->sk_receive_queue);
		__kfree_skb(skb);
		return -1;
	}

	/* If we are not yet fully established and do not know the mapping for
	 * this segment, this path has to fallback to infinite or be torn down.
	 */
	if (!tp->mptcp->fully_established && !mptcp_is_data_seq(skb) &&
	    !tp->mptcp->mapping_present && !mpcb->infinite_mapping_rcv) {
		pr_err("%s %#x will fallback - pi %d from %pS, seq %u\n",
		       __func__, mpcb->mptcp_loc_token,
		       tp->mptcp->path_index, __builtin_return_address(0),
		       TCP_SKB_CB(skb)->seq);

		if (!is_master_tp(tp)) {
			MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_FBDATASUB);
			mptcp_send_reset(sk);
			return 1;
		}

		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_FBDATAINIT);

		mpcb->infinite_mapping_snd = 1;
		mpcb->infinite_mapping_rcv = 1;
		mpcb->infinite_rcv_seq = mptcp_get_rcv_nxt_64(mptcp_meta_tp(tp));

		mptcp_sub_force_close_all(mpcb, sk);

		/* We do a seamless fallback and should not send a inf.mapping. */
		mpcb->send_infinite_mapping = 0;
		tp->mptcp->fully_established = 1;
	}

	/* Receiver-side becomes fully established when a whole rcv-window has
	 * been received without the need to fallback due to the previous
	 * condition.
	 */
	if (!tp->mptcp->fully_established) {
		tp->mptcp->init_rcv_wnd -= skb->len;
		if (tp->mptcp->init_rcv_wnd < 0)
			mptcp_become_fully_estab(sk);
	}

	return 0;
}

/* @return: 0  everything is fine. Just continue processing
 *	    1  subflow is broken stop everything
 *	    -1 this packet was broken - continue with the next one.
 */
static int mptcp_detect_mapping(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk), *meta_tp = mptcp_meta_tp(tp);
	struct mptcp_cb *mpcb = tp->mpcb;
	struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);
	u32 *ptr;
	u32 data_seq, sub_seq, data_len, tcp_end_seq;
	bool set_infinite_rcv = false;

	/* If we are in infinite-mapping-mode, the subflow is guaranteed to be
	 * in-order at the data-level. Thus data-seq-numbers can be inferred
	 * from what is expected at the data-level.
	 */
	if (mpcb->infinite_mapping_rcv) {
		/* copied_seq may be bigger than tcb->seq (e.g., when the peer
		 * retransmits data that actually has already been acknowledged with
		 * newer data, if he did not receive our acks). Thus, we need
		 * to account for this overlap as well.
		 */
		tp->mptcp->map_data_seq = mpcb->infinite_rcv_seq - (tp->copied_seq - tcb->seq);
		tp->mptcp->map_subseq = tcb->seq;
		tp->mptcp->map_data_len = skb->len;
		tp->mptcp->map_data_fin = !!(TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN);
		tp->mptcp->mapping_present = 1;
		return 0;
	}

	/* No mapping here? Exit - it is either already set or still on its way */
	if (!mptcp_is_data_seq(skb)) {
		/* Too many packets without a mapping - this subflow is broken */
		if (!tp->mptcp->mapping_present &&
		    tp->rcv_nxt - tp->copied_seq > 65536) {
			MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_NODSSWINDOW);
			mptcp_send_reset(sk);
			return 1;
		}

		return 0;
	}

	ptr = mptcp_skb_set_data_seq(skb, &data_seq, mpcb);
	ptr++;
	sub_seq = get_unaligned_be32(ptr) + tp->mptcp->rcv_isn;
	ptr++;
	data_len = get_unaligned_be16(ptr);

	/* If it's an empty skb with DATA_FIN, sub_seq must get fixed.
	 * The draft sets it to 0, but we really would like to have the
	 * real value, to have an easy handling afterwards here in this
	 * function.
	 */
	if (mptcp_is_data_fin(skb) && skb->len == 0)
		sub_seq = TCP_SKB_CB(skb)->seq;

	/* If there is already a mapping - we check if it maps with the current
	 * one. If not - we reset.
	 */
	if (tp->mptcp->mapping_present &&
	    (data_seq != (u32)tp->mptcp->map_data_seq ||
	     sub_seq != tp->mptcp->map_subseq ||
	     data_len != tp->mptcp->map_data_len + tp->mptcp->map_data_fin ||
	     mptcp_is_data_fin(skb) != tp->mptcp->map_data_fin)) {
		/* Mapping in packet is different from what we want */
		pr_err("%s Mappings do not match!\n", __func__);
		pr_err("%s dseq %u mdseq %u, sseq %u msseq %u dlen %u mdlen %u dfin %d mdfin %d\n",
		       __func__, data_seq, (u32)tp->mptcp->map_data_seq,
		       sub_seq, tp->mptcp->map_subseq, data_len,
		       tp->mptcp->map_data_len, mptcp_is_data_fin(skb),
		       tp->mptcp->map_data_fin);
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_DSSNOMATCH);
		mptcp_send_reset(sk);
		return 1;
	}

	/* If the previous check was good, the current mapping is valid and we exit. */
	if (tp->mptcp->mapping_present)
		return 0;

	/* Mapping not yet set on this subflow - we set it here! */

	if (!data_len) {
		mpcb->infinite_mapping_rcv = 1;
		mpcb->send_infinite_mapping = 1;
		tp->mptcp->fully_established = 1;
		/* We need to repeat mp_fail's until the sender felt
		 * back to infinite-mapping - here we stop repeating it.
		 */
		tp->mptcp->send_mp_fail = 0;

		/* We have to fixup data_len - it must be the same as skb->len */
		data_len = skb->len + (mptcp_is_data_fin(skb) ? 1 : 0);
		sub_seq = tcb->seq;

		mptcp_sub_force_close_all(mpcb, sk);

		/* data_seq and so on are set correctly */

		/* At this point, the meta-ofo-queue has to be emptied,
		 * as the following data is guaranteed to be in-order at
		 * the data and subflow-level
		 */
		mptcp_purge_ofo_queue(meta_tp);

		set_infinite_rcv = true;
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_INFINITEMAPRX);
	}

	/* We are sending mp-fail's and thus are in fallback mode.
	 * Ignore packets which do not announce the fallback and still
	 * want to provide a mapping.
	 */
	if (tp->mptcp->send_mp_fail) {
		tp->copied_seq = TCP_SKB_CB(skb)->end_seq;
		__skb_unlink(skb, &sk->sk_receive_queue);
		__kfree_skb(skb);
		return -1;
	}

	/* FIN increased the mapping-length by 1 */
	if (mptcp_is_data_fin(skb))
		data_len--;

	/* Subflow-sequences of packet must be
	 * (at least partially) be part of the DSS-mapping's
	 * subflow-sequence-space.
	 *
	 * Basically the mapping is not valid, if either of the
	 * following conditions is true:
	 *
	 * 1. It's not a data_fin and
	 *    MPTCP-sub_seq >= TCP-end_seq
	 *
	 * 2. It's a data_fin and TCP-end_seq > TCP-seq and
	 *    MPTCP-sub_seq >= TCP-end_seq
	 *
	 * The previous two can be merged into:
	 *    TCP-end_seq > TCP-seq and MPTCP-sub_seq >= TCP-end_seq
	 *    Because if it's not a data-fin, TCP-end_seq > TCP-seq
	 *
	 * 3. It's a data_fin and skb->len == 0 and
	 *    MPTCP-sub_seq > TCP-end_seq
	 *
	 * 4. It's not a data_fin and TCP-end_seq > TCP-seq and
	 *    MPTCP-sub_seq + MPTCP-data_len <= TCP-seq
	 */

	/* subflow-fin is not part of the mapping - ignore it here ! */
	tcp_end_seq = tcb->end_seq;
	if (tcb->tcp_flags & TCPHDR_FIN)
		tcp_end_seq--;
	if ((!before(sub_seq, tcb->end_seq) && after(tcp_end_seq, tcb->seq)) ||
	    (mptcp_is_data_fin(skb) && skb->len == 0 && after(sub_seq, tcb->end_seq)) ||
	    (!after(sub_seq + data_len, tcb->seq) && after(tcp_end_seq, tcb->seq))) {
		/* Subflow-sequences of packet is different from what is in the
		 * packet's dss-mapping. The peer is misbehaving - reset
		 */
		pr_err("%s Packet's mapping does not map to the DSS sub_seq %u "
		       "end_seq %u, tcp_end_seq %u seq %u dfin %u len %u data_len %u"
		       "copied_seq %u\n", __func__, sub_seq, tcb->end_seq, tcp_end_seq, tcb->seq, mptcp_is_data_fin(skb),
		       skb->len, data_len, tp->copied_seq);
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_DSSTCPMISMATCH);
		mptcp_send_reset(sk);
		return 1;
	}

	/* Does the DSS had 64-bit seqnum's ? */
	if (!(tcb->mptcp_flags & MPTCPHDR_SEQ64_SET)) {
		/* Wrapped around? */
		if (unlikely(after(data_seq, meta_tp->rcv_nxt) && data_seq < meta_tp->rcv_nxt)) {
			tp->mptcp->map_data_seq = mptcp_get_data_seq_64(mpcb, !mpcb->rcv_hiseq_index, data_seq);
		} else {
			/* Else, access the default high-order bits */
			tp->mptcp->map_data_seq = mptcp_get_data_seq_64(mpcb, mpcb->rcv_hiseq_index, data_seq);
		}
	} else {
		tp->mptcp->map_data_seq = mptcp_get_data_seq_64(mpcb, (tcb->mptcp_flags & MPTCPHDR_SEQ64_INDEX) ? 1 : 0, data_seq);

		if (unlikely(tcb->mptcp_flags & MPTCPHDR_SEQ64_OFO)) {
			/* We make sure that the data_seq is invalid.
			 * It will be dropped later.
			 */
			tp->mptcp->map_data_seq += 0xFFFFFFFF;
			tp->mptcp->map_data_seq += 0xFFFFFFFF;
		}
	}

	if (set_infinite_rcv)
		mpcb->infinite_rcv_seq = tp->mptcp->map_data_seq;

	tp->mptcp->map_data_len = data_len;
	tp->mptcp->map_subseq = sub_seq;
	tp->mptcp->map_data_fin = mptcp_is_data_fin(skb) ? 1 : 0;
	tp->mptcp->mapping_present = 1;

	return 0;
}

/* Similar to tcp_sequence(...) */
static inline bool mptcp_sequence(const struct tcp_sock *meta_tp,
				 u64 data_seq, u64 end_data_seq)
{
	const struct mptcp_cb *mpcb = meta_tp->mpcb;
	u64 rcv_wup64;

	/* Wrap-around? */
	if (meta_tp->rcv_wup > meta_tp->rcv_nxt) {
		rcv_wup64 = ((u64)(mpcb->rcv_high_order[mpcb->rcv_hiseq_index] - 1) << 32) |
				meta_tp->rcv_wup;
	} else {
		rcv_wup64 = mptcp_get_data_seq_64(mpcb, mpcb->rcv_hiseq_index,
						  meta_tp->rcv_wup);
	}

	return	!before64(end_data_seq, rcv_wup64) &&
		!after64(data_seq, mptcp_get_rcv_nxt_64(meta_tp) + tcp_receive_window(meta_tp));
}

/* @return: 0  everything is fine. Just continue processing
 *	    -1 this packet was broken - continue with the next one.
 */
static int mptcp_validate_mapping(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct sk_buff *tmp, *tmp1;
	u32 tcp_end_seq;

	if (!tp->mptcp->mapping_present)
		return 0;

	/* either, the new skb gave us the mapping and the first segment
	 * in the sub-rcv-queue has to be trimmed ...
	 */
	tmp = skb_peek(&sk->sk_receive_queue);
	if (before(TCP_SKB_CB(tmp)->seq, tp->mptcp->map_subseq) &&
	    after(TCP_SKB_CB(tmp)->end_seq, tp->mptcp->map_subseq)) {
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_DSSTRIMHEAD);
		mptcp_skb_trim_head(tmp, sk, tp->mptcp->map_subseq);
	}

	/* ... or the new skb (tail) has to be split at the end. */
	tcp_end_seq = TCP_SKB_CB(skb)->end_seq;
	if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN)
		tcp_end_seq--;
	if (after(tcp_end_seq, tp->mptcp->map_subseq + tp->mptcp->map_data_len)) {
		u32 seq = tp->mptcp->map_subseq + tp->mptcp->map_data_len;
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_DSSSPLITTAIL);
		if (mptcp_skb_split_tail(skb, sk, seq)) { /* Allocation failed */
			/* TODO : maybe handle this here better.
			 * We now just force meta-retransmission.
			 */
			tp->copied_seq = TCP_SKB_CB(skb)->end_seq;
			__skb_unlink(skb, &sk->sk_receive_queue);
			__kfree_skb(skb);
			return -1;
		}
	}

	/* Now, remove old sk_buff's from the receive-queue.
	 * This may happen if the mapping has been lost for these segments and
	 * the next mapping has already been received.
	 */
	if (before(TCP_SKB_CB(skb_peek(&sk->sk_receive_queue))->seq, tp->mptcp->map_subseq)) {
		skb_queue_walk_safe(&sk->sk_receive_queue, tmp1, tmp) {
			if (!before(TCP_SKB_CB(tmp1)->seq, tp->mptcp->map_subseq))
				break;

			tp->copied_seq = TCP_SKB_CB(tmp1)->end_seq;
			__skb_unlink(tmp1, &sk->sk_receive_queue);

			MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_PURGEOLD);
			/* Impossible that we could free skb here, because his
			 * mapping is known to be valid from previous checks
			 */
			__kfree_skb(tmp1);
		}
	}

	return 0;
}

/* @return: 0  everything is fine. Just continue processing
 *	    1  subflow is broken stop everything
 *	    -1 this mapping has been put in the meta-receive-queue
 *	    -2 this mapping has been eaten by the application
 */
static int mptcp_queue_skb(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk), *meta_tp = mptcp_meta_tp(tp);
	struct sock *meta_sk = mptcp_meta_sk(sk);
	struct mptcp_cb *mpcb = tp->mpcb;
	struct sk_buff *tmp, *tmp1;
	u64 rcv_nxt64 = mptcp_get_rcv_nxt_64(meta_tp);
	u32 old_copied_seq = tp->copied_seq;
	bool data_queued = false;

	/* Have we not yet received the full mapping? */
	if (!tp->mptcp->mapping_present ||
	    before(tp->rcv_nxt, tp->mptcp->map_subseq + tp->mptcp->map_data_len))
		return 0;

	/* Is this an overlapping mapping? rcv_nxt >= end_data_seq
	 * OR
	 * This mapping is out of window
	 */
	if (!before64(rcv_nxt64, tp->mptcp->map_data_seq + tp->mptcp->map_data_len + tp->mptcp->map_data_fin) ||
	    !mptcp_sequence(meta_tp, tp->mptcp->map_data_seq,
			    tp->mptcp->map_data_seq + tp->mptcp->map_data_len + tp->mptcp->map_data_fin)) {
		skb_queue_walk_safe(&sk->sk_receive_queue, tmp1, tmp) {
			__skb_unlink(tmp1, &sk->sk_receive_queue);
			tp->copied_seq = TCP_SKB_CB(tmp1)->end_seq;
			__kfree_skb(tmp1);

			if (!skb_queue_empty(&sk->sk_receive_queue) &&
			    !before(TCP_SKB_CB(tmp)->seq,
				    tp->mptcp->map_subseq + tp->mptcp->map_data_len))
				break;
		}

		mptcp_reset_mapping(tp, old_copied_seq);

		return -1;
	}

	/* Record it, because we want to send our data_fin on the same path */
	if (tp->mptcp->map_data_fin) {
		mpcb->dfin_path_index = tp->mptcp->path_index;
		mpcb->dfin_combined = !!(sk->sk_shutdown & RCV_SHUTDOWN);
	}

	/* Verify the checksum */
	if (mpcb->dss_csum && !mpcb->infinite_mapping_rcv) {
		int ret = mptcp_verif_dss_csum(sk);

		if (ret <= 0) {
			mptcp_reset_mapping(tp, old_copied_seq);
			return 1;
		}
	}

	if (before64(rcv_nxt64, tp->mptcp->map_data_seq)) {
		/* Seg's have to go to the meta-ofo-queue */
		skb_queue_walk_safe(&sk->sk_receive_queue, tmp1, tmp) {
			tp->copied_seq = TCP_SKB_CB(tmp1)->end_seq;
			mptcp_prepare_skb(tmp1, sk);
			__skb_unlink(tmp1, &sk->sk_receive_queue);
			/* MUST be done here, because fragstolen may be true later.
			 * Then, kfree_skb_partial will not account the memory.
			 */
			skb_orphan(tmp1);

			if (!mpcb->in_time_wait) /* In time-wait, do not receive data */
				mptcp_add_meta_ofo_queue(meta_sk, tmp1, sk);
			else
				__kfree_skb(tmp1);

			if (!skb_queue_empty(&sk->sk_receive_queue) &&
			    !before(TCP_SKB_CB(tmp)->seq,
				    tp->mptcp->map_subseq + tp->mptcp->map_data_len))
				break;
		}
		tcp_enter_quickack_mode(sk);
	} else {
		/* Ready for the meta-rcv-queue */
		skb_queue_walk_safe(&sk->sk_receive_queue, tmp1, tmp) {
			int eaten = 0;
			bool fragstolen = false;
			u32 old_rcv_nxt = meta_tp->rcv_nxt;

			tp->copied_seq = TCP_SKB_CB(tmp1)->end_seq;
			mptcp_prepare_skb(tmp1, sk);
			__skb_unlink(tmp1, &sk->sk_receive_queue);
			/* MUST be done here, because fragstolen may be true.
			 * Then, kfree_skb_partial will not account the memory.
			 */
			skb_orphan(tmp1);

			/* This segment has already been received */
			if (!after(TCP_SKB_CB(tmp1)->end_seq, meta_tp->rcv_nxt)) {
				__kfree_skb(tmp1);
				goto next;
			}

			/* Is direct copy possible ? */
			if (TCP_SKB_CB(tmp1)->seq == meta_tp->rcv_nxt &&
			    meta_tp->ucopy.task == current &&
			    meta_tp->copied_seq == meta_tp->rcv_nxt &&
			    meta_tp->ucopy.len && sock_owned_by_user(meta_sk))
				eaten = mptcp_direct_copy(tmp1, meta_sk);

			if (mpcb->in_time_wait) /* In time-wait, do not receive data */
				eaten = 1;

			if (!eaten)
				eaten = tcp_queue_rcv(meta_sk, tmp1, 0, &fragstolen);

			meta_tp->rcv_nxt = TCP_SKB_CB(tmp1)->end_seq;
			mptcp_check_rcvseq_wrap(meta_tp, old_rcv_nxt);

			if ((TCP_SKB_CB(tmp1)->tcp_flags & TCPHDR_FIN) &&
			    !mpcb->in_time_wait)
				mptcp_fin(meta_sk);

			/* Check if this fills a gap in the ofo queue */
			if (!skb_queue_empty(&meta_tp->out_of_order_queue))
				mptcp_ofo_queue(meta_sk);

			if (eaten)
				kfree_skb_partial(tmp1, fragstolen);

			data_queued = true;
next:
			if (!skb_queue_empty(&sk->sk_receive_queue) &&
			    !before(TCP_SKB_CB(tmp)->seq,
				    tp->mptcp->map_subseq + tp->mptcp->map_data_len))
				break;
		}
	}

	inet_csk(meta_sk)->icsk_ack.lrcvtime = tcp_time_stamp;
	mptcp_reset_mapping(tp, old_copied_seq);

	return data_queued ? -1 : -2;
}

void mptcp_data_ready(struct sock *sk)
{
	struct sock *meta_sk = mptcp_meta_sk(sk);
	struct sk_buff *skb, *tmp;
	int queued = 0;

	/* restart before the check, because mptcp_fin might have changed the
	 * state.
	 */
restart:
	/* If the meta cannot receive data, there is no point in pushing data.
	 * If we are in time-wait, we may still be waiting for the final FIN.
	 * So, we should proceed with the processing.
	 */
	if (!mptcp_sk_can_recv(meta_sk) && !tcp_sk(sk)->mpcb->in_time_wait) {
		skb_queue_purge(&sk->sk_receive_queue);
		tcp_sk(sk)->copied_seq = tcp_sk(sk)->rcv_nxt;
		goto exit;
	}

	/* Iterate over all segments, detect their mapping (if we don't have
	 * one yet), validate them and push everything one level higher.
	 */
	skb_queue_walk_safe(&sk->sk_receive_queue, skb, tmp) {
		int ret;
		/* Pre-validation - e.g., early fallback */
		ret = mptcp_prevalidate_skb(sk, skb);
		if (ret < 0)
			goto restart;
		else if (ret > 0)
			break;

		/* Set the current mapping */
		ret = mptcp_detect_mapping(sk, skb);
		if (ret < 0)
			goto restart;
		else if (ret > 0)
			break;

		/* Validation */
		if (mptcp_validate_mapping(sk, skb) < 0)
			goto restart;

		/* Push a level higher */
		ret = mptcp_queue_skb(sk);
		if (ret < 0) {
			if (ret == -1)
				queued = ret;
			goto restart;
		} else if (ret == 0) {
			continue;
		} else { /* ret == 1 */
			break;
		}
	}

exit:
	if (tcp_sk(sk)->close_it) {
		tcp_send_ack(sk);
		tcp_sk(sk)->ops->time_wait(sk, TCP_TIME_WAIT, 0);
	}

	if (queued == -1 && !sock_flag(meta_sk, SOCK_DEAD))
		meta_sk->sk_data_ready(meta_sk);
}


int mptcp_check_req(struct sk_buff *skb, struct net *net)
{
	const struct tcphdr *th = tcp_hdr(skb);
	struct sock *meta_sk = NULL;

	/* MPTCP structures not initialized */
	if (mptcp_init_failed)
		return 0;

	if (skb->protocol == htons(ETH_P_IP))
		meta_sk = mptcp_v4_search_req(th->source, ip_hdr(skb)->saddr,
					      ip_hdr(skb)->daddr, net);
#if IS_ENABLED(CONFIG_IPV6)
	else /* IPv6 */
		meta_sk = mptcp_v6_search_req(th->source, &ipv6_hdr(skb)->saddr,
					      &ipv6_hdr(skb)->daddr, net);
#endif /* CONFIG_IPV6 */

	if (!meta_sk)
		return 0;

	TCP_SKB_CB(skb)->mptcp_flags |= MPTCPHDR_JOIN;

	bh_lock_sock_nested(meta_sk);
	if (sock_owned_by_user(meta_sk)) {
		skb->sk = meta_sk;
		if (unlikely(sk_add_backlog(meta_sk, skb,
					    meta_sk->sk_rcvbuf + meta_sk->sk_sndbuf))) {
			bh_unlock_sock(meta_sk);
			NET_INC_STATS_BH(net, LINUX_MIB_TCPBACKLOGDROP);
			sock_put(meta_sk); /* Taken by mptcp_search_req */
			kfree_skb(skb);
			return 1;
		}
	} else if (skb->protocol == htons(ETH_P_IP)) {
		tcp_v4_do_rcv(meta_sk, skb);
#if IS_ENABLED(CONFIG_IPV6)
	} else { /* IPv6 */
		tcp_v6_do_rcv(meta_sk, skb);
#endif /* CONFIG_IPV6 */
	}
	bh_unlock_sock(meta_sk);
	sock_put(meta_sk); /* Taken by mptcp_vX_search_req */
	return 1;
}

struct mp_join *mptcp_find_join(const struct sk_buff *skb)
{
	const struct tcphdr *th = tcp_hdr(skb);
	unsigned char *ptr;
	int length = (th->doff * 4) - sizeof(struct tcphdr);

	/* Jump through the options to check whether JOIN is there */
	ptr = (unsigned char *)(th + 1);
	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return NULL;
		case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2)	/* "silly options" */
				return NULL;
			if (opsize > length)
				return NULL;  /* don't parse partial options */
			if (opcode == TCPOPT_MPTCP &&
			    ((struct mptcp_option *)(ptr - 2))->sub == MPTCP_SUB_JOIN) {
				return (struct mp_join *)(ptr - 2);
			}
			ptr += opsize - 2;
			length -= opsize;
		}
	}
	return NULL;
}

int mptcp_lookup_join(struct sk_buff *skb, struct inet_timewait_sock *tw)
{
	const struct mptcp_cb *mpcb;
	struct sock *meta_sk;
	u32 token;
	bool meta_v4;
	struct mp_join *join_opt = mptcp_find_join(skb);
	if (!join_opt)
		return 0;

	/* MPTCP structures were not initialized, so return error */
	if (mptcp_init_failed)
		return -1;

	token = join_opt->u.syn.token;
	meta_sk = mptcp_hash_find(dev_net(skb_dst(skb)->dev), token);
	if (!meta_sk) {
		MPTCP_INC_STATS_BH(dev_net(skb_dst(skb)->dev), MPTCP_MIB_JOINNOTOKEN);
		mptcp_debug("%s:mpcb not found:%x\n", __func__, token);
		return -1;
	}

	meta_v4 = meta_sk->sk_family == AF_INET;
	if (meta_v4) {
		if (skb->protocol == htons(ETH_P_IPV6)) {
			mptcp_debug("SYN+MP_JOIN with IPV6 address on pure IPV4 meta\n");
			sock_put(meta_sk); /* Taken by mptcp_hash_find */
			return -1;
		}
	} else if (skb->protocol == htons(ETH_P_IP) && meta_sk->sk_ipv6only) {
		mptcp_debug("SYN+MP_JOIN with IPV4 address on IPV6_V6ONLY meta\n");
		sock_put(meta_sk); /* Taken by mptcp_hash_find */
		return -1;
	}

	mpcb = tcp_sk(meta_sk)->mpcb;
	if (mpcb->infinite_mapping_rcv || mpcb->send_infinite_mapping) {
		/* We are in fallback-mode on the reception-side -
		 * no new subflows!
		 */
		sock_put(meta_sk); /* Taken by mptcp_hash_find */
		MPTCP_INC_STATS_BH(sock_net(meta_sk), MPTCP_MIB_JOINFALLBACK);
		return -1;
	}

	/* Coming from time-wait-sock processing in tcp_v4_rcv.
	 * We have to deschedule it before continuing, because otherwise
	 * mptcp_v4_do_rcv will hit again on it inside tcp_v4_hnd_req.
	 */
	if (tw) {
		inet_twsk_deschedule(tw);
		inet_twsk_put(tw);
	}

	TCP_SKB_CB(skb)->mptcp_flags |= MPTCPHDR_JOIN;
	/* OK, this is a new syn/join, let's create a new open request and
	 * send syn+ack
	 */
	bh_lock_sock_nested(meta_sk);
	if (sock_owned_by_user(meta_sk)) {
		skb->sk = meta_sk;
		if (unlikely(sk_add_backlog(meta_sk, skb,
					    meta_sk->sk_rcvbuf + meta_sk->sk_sndbuf))) {
			bh_unlock_sock(meta_sk);
			NET_INC_STATS_BH(sock_net(meta_sk),
					 LINUX_MIB_TCPBACKLOGDROP);
			sock_put(meta_sk); /* Taken by mptcp_hash_find */
			kfree_skb(skb);
			return 1;
		}
	} else if (skb->protocol == htons(ETH_P_IP)) {
		tcp_v4_do_rcv(meta_sk, skb);
#if IS_ENABLED(CONFIG_IPV6)
	} else {
		tcp_v6_do_rcv(meta_sk, skb);
#endif /* CONFIG_IPV6 */
	}
	bh_unlock_sock(meta_sk);
	sock_put(meta_sk); /* Taken by mptcp_hash_find */
	return 1;
}

int mptcp_do_join_short(struct sk_buff *skb,
			const struct mptcp_options_received *mopt,
			struct net *net)
{
	struct sock *meta_sk;
	u32 token;
	bool meta_v4;

	token = mopt->mptcp_rem_token;
	meta_sk = mptcp_hash_find(net, token);
	if (!meta_sk) {
		MPTCP_INC_STATS_BH(dev_net(skb_dst(skb)->dev), MPTCP_MIB_JOINNOTOKEN);
		mptcp_debug("%s:mpcb not found:%x\n", __func__, token);
		return -1;
	}

	meta_v4 = meta_sk->sk_family == AF_INET;
	if (meta_v4) {
		if (skb->protocol == htons(ETH_P_IPV6)) {
			mptcp_debug("SYN+MP_JOIN with IPV6 address on pure IPV4 meta\n");
			sock_put(meta_sk); /* Taken by mptcp_hash_find */
			return -1;
		}
	} else if (skb->protocol == htons(ETH_P_IP) && meta_sk->sk_ipv6only) {
		mptcp_debug("SYN+MP_JOIN with IPV4 address on IPV6_V6ONLY meta\n");
		sock_put(meta_sk); /* Taken by mptcp_hash_find */
		return -1;
	}

	TCP_SKB_CB(skb)->mptcp_flags |= MPTCPHDR_JOIN;

	/* OK, this is a new syn/join, let's create a new open request and
	 * send syn+ack
	 */
	bh_lock_sock(meta_sk);

	/* This check is also done in mptcp_vX_do_rcv. But, there we cannot
	 * call tcp_vX_send_reset, because we hold already two socket-locks.
	 * (the listener and the meta from above)
	 *
	 * And the send-reset will try to take yet another one (ip_send_reply).
	 * Thus, we propagate the reset up to tcp_rcv_state_process.
	 */
	if (tcp_sk(meta_sk)->mpcb->infinite_mapping_rcv ||
	    tcp_sk(meta_sk)->mpcb->send_infinite_mapping ||
	    meta_sk->sk_state == TCP_CLOSE || !tcp_sk(meta_sk)->inside_tk_table) {
		MPTCP_INC_STATS_BH(sock_net(meta_sk), MPTCP_MIB_JOINFALLBACK);
		bh_unlock_sock(meta_sk);
		sock_put(meta_sk); /* Taken by mptcp_hash_find */
		return -1;
	}

	if (sock_owned_by_user(meta_sk)) {
		skb->sk = meta_sk;
		if (unlikely(sk_add_backlog(meta_sk, skb,
					    meta_sk->sk_rcvbuf + meta_sk->sk_sndbuf)))
			NET_INC_STATS_BH(net, LINUX_MIB_TCPBACKLOGDROP);
		else
			/* Must make sure that upper layers won't free the
			 * skb if it is added to the backlog-queue.
			 */
			skb_get(skb);
	} else {
		/* mptcp_v4_do_rcv tries to free the skb - we prevent this, as
		 * the skb will finally be freed by tcp_v4_do_rcv (where we are
		 * coming from)
		 */
		skb_get(skb);
		if (skb->protocol == htons(ETH_P_IP)) {
			tcp_v4_do_rcv(meta_sk, skb);
#if IS_ENABLED(CONFIG_IPV6)
		} else { /* IPv6 */
			tcp_v6_do_rcv(meta_sk, skb);
#endif /* CONFIG_IPV6 */
		}
	}

	bh_unlock_sock(meta_sk);
	sock_put(meta_sk); /* Taken by mptcp_hash_find */
	return 0;
}

/**
 * Equivalent of tcp_fin() for MPTCP
 * Can be called only when the FIN is validly part
 * of the data seqnum space. Not before when we get holes.
 */
void mptcp_fin(struct sock *meta_sk)
{
	struct sock *sk = NULL, *sk_it;
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;

	mptcp_for_each_sk(mpcb, sk_it) {
		if (tcp_sk(sk_it)->mptcp->path_index == mpcb->dfin_path_index) {
			sk = sk_it;
			break;
		}
	}

	if (!sk || sk->sk_state == TCP_CLOSE)
		sk = mptcp_select_ack_sock(meta_sk);

	inet_csk_schedule_ack(sk);

	meta_sk->sk_shutdown |= RCV_SHUTDOWN;
	sock_set_flag(meta_sk, SOCK_DONE);

	switch (meta_sk->sk_state) {
	case TCP_SYN_RECV:
	case TCP_ESTABLISHED:
		/* Move to CLOSE_WAIT */
		tcp_set_state(meta_sk, TCP_CLOSE_WAIT);
		inet_csk(sk)->icsk_ack.pingpong = 1;
		break;

	case TCP_CLOSE_WAIT:
	case TCP_CLOSING:
		/* Received a retransmission of the FIN, do
		 * nothing.
		 */
		break;
	case TCP_LAST_ACK:
		/* RFC793: Remain in the LAST-ACK state. */
		break;

	case TCP_FIN_WAIT1:
		/* This case occurs when a simultaneous close
		 * happens, we must ack the received FIN and
		 * enter the CLOSING state.
		 */
		tcp_send_ack(sk);
		tcp_set_state(meta_sk, TCP_CLOSING);
		break;
	case TCP_FIN_WAIT2:
		/* Received a FIN -- send ACK and enter TIME_WAIT. */
		tcp_send_ack(sk);
		meta_tp->ops->time_wait(meta_sk, TCP_TIME_WAIT, 0);
		break;
	default:
		/* Only TCP_LISTEN and TCP_CLOSE are left, in these
		 * cases we should never reach this piece of code.
		 */
		pr_err("%s: Impossible, meta_sk->sk_state=%d\n", __func__,
		       meta_sk->sk_state);
		break;
	}

	/* It _is_ possible, that we have something out-of-order _after_ FIN.
	 * Probably, we should reset in this case. For now drop them.
	 */
	mptcp_purge_ofo_queue(meta_tp);
	sk_mem_reclaim(meta_sk);

	if (!sock_flag(meta_sk, SOCK_DEAD)) {
		meta_sk->sk_state_change(meta_sk);

		/* Do not send POLL_HUP for half duplex close. */
		if (meta_sk->sk_shutdown == SHUTDOWN_MASK ||
		    meta_sk->sk_state == TCP_CLOSE)
			sk_wake_async(meta_sk, SOCK_WAKE_WAITD, POLL_HUP);
		else
			sk_wake_async(meta_sk, SOCK_WAKE_WAITD, POLL_IN);
	}

	return;
}

static void mptcp_xmit_retransmit_queue(struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct sk_buff *skb;

	if (!meta_tp->packets_out)
		return;

	tcp_for_write_queue(skb, meta_sk) {
		if (skb == tcp_send_head(meta_sk))
			break;

		if (mptcp_retransmit_skb(meta_sk, skb))
			return;

		if (skb == tcp_write_queue_head(meta_sk))
			inet_csk_reset_xmit_timer(meta_sk, ICSK_TIME_RETRANS,
						  inet_csk(meta_sk)->icsk_rto,
						  TCP_RTO_MAX);
	}
}

/* Handle the DATA_ACK */
static void mptcp_data_ack(struct sock *sk, const struct sk_buff *skb)
{
	struct sock *meta_sk = mptcp_meta_sk(sk);
	struct tcp_sock *meta_tp = tcp_sk(meta_sk), *tp = tcp_sk(sk);
	struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);
	u32 prior_snd_una = meta_tp->snd_una;
	int prior_packets;
	u32 nwin, data_ack, data_seq;
	u16 data_len = 0;

	/* A valid packet came in - subflow is operational again */
	tp->pf = 0;

	/* Even if there is no data-ack, we stop retransmitting.
	 * Except if this is a SYN/ACK. Then it is just a retransmission
	 */
	if (tp->mptcp->pre_established && !tcp_hdr(skb)->syn) {
		tp->mptcp->pre_established = 0;
		sk_stop_timer(sk, &tp->mptcp->mptcp_ack_timer);
	}

	/* If we are in infinite mapping mode, rx_opt.data_ack has been
	 * set by mptcp_clean_rtx_infinite.
	 */
	if (!(tcb->mptcp_flags & MPTCPHDR_ACK) && !tp->mpcb->infinite_mapping_snd)
		goto exit;

	data_ack = tp->mptcp->rx_opt.data_ack;

	if (unlikely(!tp->mptcp->fully_established) &&
	    tp->mptcp->snt_isn + 1 != TCP_SKB_CB(skb)->ack_seq)
		/* As soon as a subflow-data-ack (not acking syn, thus snt_isn + 1)
		 * includes a data-ack, we are fully established
		 */
		mptcp_become_fully_estab(sk);

	/* Get the data_seq */
	if (mptcp_is_data_seq(skb)) {
		data_seq = tp->mptcp->rx_opt.data_seq;
		data_len = tp->mptcp->rx_opt.data_len;
	} else {
		data_seq = meta_tp->snd_wl1;
	}

	/* If the ack is older than previous acks
	 * then we can probably ignore it.
	 */
	if (before(data_ack, prior_snd_una))
		goto exit;

	/* If the ack includes data we haven't sent yet, discard
	 * this segment (RFC793 Section 3.9).
	 */
	if (after(data_ack, meta_tp->snd_nxt))
		goto exit;

	/*** Now, update the window  - inspired by tcp_ack_update_window ***/
	nwin = ntohs(tcp_hdr(skb)->window);

	if (likely(!tcp_hdr(skb)->syn))
		nwin <<= tp->rx_opt.snd_wscale;

	if (tcp_may_update_window(meta_tp, data_ack, data_seq, nwin)) {
		tcp_update_wl(meta_tp, data_seq);

		/* Draft v09, Section 3.3.5:
		 * [...] It should only update its local receive window values
		 * when the largest sequence number allowed (i.e.  DATA_ACK +
		 * receive window) increases. [...]
		 */
		if (meta_tp->snd_wnd != nwin &&
		    !before(data_ack + nwin, tcp_wnd_end(meta_tp))) {
			meta_tp->snd_wnd = nwin;

			if (nwin > meta_tp->max_window)
				meta_tp->max_window = nwin;
		}
	}
	/*** Done, update the window ***/

	/* We passed data and got it acked, remove any soft error
	 * log. Something worked...
	 */
	sk->sk_err_soft = 0;
	inet_csk(meta_sk)->icsk_probes_out = 0;
	meta_tp->rcv_tstamp = tcp_time_stamp;
	prior_packets = meta_tp->packets_out;
	if (!prior_packets)
		goto no_queue;

	meta_tp->snd_una = data_ack;

	mptcp_clean_rtx_queue(meta_sk, prior_snd_una);

	/* We are in loss-state, and something got acked, retransmit the whole
	 * queue now!
	 */
	if (inet_csk(meta_sk)->icsk_ca_state == TCP_CA_Loss &&
	    after(data_ack, prior_snd_una)) {
		mptcp_xmit_retransmit_queue(meta_sk);
		inet_csk(meta_sk)->icsk_ca_state = TCP_CA_Open;
	}

	/* Simplified version of tcp_new_space, because the snd-buffer
	 * is handled by all the subflows.
	 */
	if (sock_flag(meta_sk, SOCK_QUEUE_SHRUNK)) {
		sock_reset_flag(meta_sk, SOCK_QUEUE_SHRUNK);
		if (meta_sk->sk_socket &&
		    test_bit(SOCK_NOSPACE, &meta_sk->sk_socket->flags))
			meta_sk->sk_write_space(meta_sk);
	}

	if (meta_sk->sk_state != TCP_ESTABLISHED &&
	    mptcp_rcv_state_process(meta_sk, sk, skb, data_seq, data_len))
		return;

exit:
	mptcp_push_pending_frames(meta_sk);

	return;

no_queue:
	if (tcp_send_head(meta_sk))
		tcp_ack_probe(meta_sk);

	mptcp_push_pending_frames(meta_sk);

	return;
}

void mptcp_clean_rtx_infinite(const struct sk_buff *skb, struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk), *meta_tp = tcp_sk(mptcp_meta_sk(sk));

	if (!tp->mpcb->infinite_mapping_snd)
		return;

	/* The difference between both write_seq's represents the offset between
	 * data-sequence and subflow-sequence. As we are infinite, this must
	 * match.
	 *
	 * Thus, from this difference we can infer the meta snd_una.
	 */
	tp->mptcp->rx_opt.data_ack = meta_tp->snd_nxt - tp->snd_nxt +
				     tp->snd_una;

	mptcp_data_ack(sk, skb);
}

/**** static functions used by mptcp_parse_options */

static void mptcp_send_reset_rem_id(const struct mptcp_cb *mpcb, u8 rem_id)
{
	struct sock *sk_it, *tmpsk;

	mptcp_for_each_sk_safe(mpcb, sk_it, tmpsk) {
		if (tcp_sk(sk_it)->mptcp->rem_id == rem_id) {
			mptcp_reinject_data(sk_it, 0);
			mptcp_send_reset(sk_it);
		}
	}
}

static inline bool is_valid_addropt_opsize(u8 mptcp_ver,
					   struct mp_add_addr *mpadd,
					   int opsize)
{
#if IS_ENABLED(CONFIG_IPV6)
	if (mptcp_ver < MPTCP_VERSION_1 && mpadd->ipver == 6) {
		return opsize == MPTCP_SUB_LEN_ADD_ADDR6 ||
		       opsize == MPTCP_SUB_LEN_ADD_ADDR6 + 2;
	}
	if (mptcp_ver >= MPTCP_VERSION_1 && mpadd->ipver == 6)
		return opsize == MPTCP_SUB_LEN_ADD_ADDR6_VER1 ||
		       opsize == MPTCP_SUB_LEN_ADD_ADDR6_VER1 + 2;
#endif
	if (mptcp_ver < MPTCP_VERSION_1 && mpadd->ipver == 4) {
		return opsize == MPTCP_SUB_LEN_ADD_ADDR4 ||
		       opsize == MPTCP_SUB_LEN_ADD_ADDR4 + 2;
	}
	if (mptcp_ver >= MPTCP_VERSION_1 && mpadd->ipver == 4) {
		return opsize == MPTCP_SUB_LEN_ADD_ADDR4_VER1 ||
		       opsize == MPTCP_SUB_LEN_ADD_ADDR4_VER1 + 2;
	}
	return false;
}

void mptcp_parse_options(const uint8_t *ptr, int opsize,
			 struct mptcp_options_received *mopt,
			 const struct sk_buff *skb,
			 struct tcp_sock *tp)
{
	const struct mptcp_option *mp_opt = (struct mptcp_option *)ptr;

	/* If the socket is mp-capable we would have a mopt. */
	if (!mopt)
		return;

	switch (mp_opt->sub) {
	case MPTCP_SUB_CAPABLE:
	{
		const struct mp_capable *mpcapable = (struct mp_capable *)ptr;

		if (opsize != MPTCP_SUB_LEN_CAPABLE_SYN &&
		    opsize != MPTCP_SUB_LEN_CAPABLE_ACK) {
			mptcp_debug("%s: mp_capable: bad option size %d\n",
				    __func__, opsize);
			break;
		}

		/* MPTCP-RFC 6824:
		 * "If receiving a message with the 'B' flag set to 1, and this
		 * is not understood, then this SYN MUST be silently ignored;
		 */
		if (mpcapable->b) {
			mopt->drop_me = 1;
			break;
		}

		/* MPTCP-RFC 6824:
		 * "An implementation that only supports this method MUST set
		 *  bit "H" to 1, and bits "C" through "G" to 0."
		 */
		if (!mpcapable->h)
			break;

		mopt->saw_mpc = 1;
		mopt->dss_csum = sysctl_mptcp_checksum || mpcapable->a;

		if (opsize >= MPTCP_SUB_LEN_CAPABLE_SYN)
			mopt->mptcp_sender_key = mpcapable->sender_key;
		if (opsize == MPTCP_SUB_LEN_CAPABLE_ACK)
			mopt->mptcp_receiver_key = mpcapable->receiver_key;

		mopt->mptcp_ver = mpcapable->ver;
		break;
	}
	case MPTCP_SUB_JOIN:
	{
		const struct mp_join *mpjoin = (struct mp_join *)ptr;

		if (opsize != MPTCP_SUB_LEN_JOIN_SYN &&
		    opsize != MPTCP_SUB_LEN_JOIN_SYNACK &&
		    opsize != MPTCP_SUB_LEN_JOIN_ACK) {
			mptcp_debug("%s: mp_join: bad option size %d\n",
				    __func__, opsize);
			break;
		}

		/* saw_mpc must be set, because in tcp_check_req we assume that
		 * it is set to support falling back to reg. TCP if a rexmitted
		 * SYN has no MP_CAPABLE or MP_JOIN
		 */
		switch (opsize) {
		case MPTCP_SUB_LEN_JOIN_SYN:
			mopt->is_mp_join = 1;
			mopt->saw_mpc = 1;
			mopt->low_prio = mpjoin->b;
			mopt->rem_id = mpjoin->addr_id;
			mopt->mptcp_rem_token = mpjoin->u.syn.token;
			mopt->mptcp_recv_nonce = mpjoin->u.syn.nonce;
			break;
		case MPTCP_SUB_LEN_JOIN_SYNACK:
			mopt->saw_mpc = 1;
			mopt->low_prio = mpjoin->b;
			mopt->rem_id = mpjoin->addr_id;
			mopt->mptcp_recv_tmac = mpjoin->u.synack.mac;
			mopt->mptcp_recv_nonce = mpjoin->u.synack.nonce;
			break;
		case MPTCP_SUB_LEN_JOIN_ACK:
			mopt->saw_mpc = 1;
			mopt->join_ack = 1;
			memcpy(mopt->mptcp_recv_mac, mpjoin->u.ack.mac, 20);
			break;
		}
		break;
	}
	case MPTCP_SUB_DSS:
	{
		const struct mp_dss *mdss = (struct mp_dss *)ptr;
		struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);

		/* We check opsize for the csum and non-csum case. We do this,
		 * because the draft says that the csum SHOULD be ignored if
		 * it has not been negotiated in the MP_CAPABLE but still is
		 * present in the data.
		 *
		 * It will get ignored later in mptcp_queue_skb.
		 */
		if (opsize != mptcp_sub_len_dss(mdss, 0) &&
		    opsize != mptcp_sub_len_dss(mdss, 1)) {
			mptcp_debug("%s: mp_dss: bad option size %d\n",
				    __func__, opsize);
			break;
		}

		ptr += 4;

		if (mdss->A) {
			tcb->mptcp_flags |= MPTCPHDR_ACK;

			if (mdss->a) {
				mopt->data_ack = (u32) get_unaligned_be64(ptr);
				ptr += MPTCP_SUB_LEN_ACK_64;
			} else {
				mopt->data_ack = get_unaligned_be32(ptr);
				ptr += MPTCP_SUB_LEN_ACK;
			}
		}

		tcb->dss_off = (ptr - skb_transport_header(skb));

		if (mdss->M) {
			if (mdss->m) {
				u64 data_seq64 = get_unaligned_be64(ptr);

				tcb->mptcp_flags |= MPTCPHDR_SEQ64_SET;
				mopt->data_seq = (u32) data_seq64;

				ptr += 12; /* 64-bit dseq + subseq */
			} else {
				mopt->data_seq = get_unaligned_be32(ptr);
				ptr += 8; /* 32-bit dseq + subseq */
			}
			mopt->data_len = get_unaligned_be16(ptr);

			tcb->mptcp_flags |= MPTCPHDR_SEQ;

			/* Is a check-sum present? */
			if (opsize == mptcp_sub_len_dss(mdss, 1))
				tcb->mptcp_flags |= MPTCPHDR_DSS_CSUM;

			/* DATA_FIN only possible with DSS-mapping */
			if (mdss->F)
				tcb->mptcp_flags |= MPTCPHDR_FIN;
		}

		break;
	}
	case MPTCP_SUB_ADD_ADDR:
	{
		struct mp_add_addr *mpadd = (struct mp_add_addr *)ptr;

		/* If tcp_sock is not available, MPTCP version can't be
		 * retrieved and ADD_ADDR opsize validation is not possible.
		 */
		if (!tp)
			break;

		if (!is_valid_addropt_opsize(tp->mpcb->mptcp_ver,
					     mpadd, opsize)) {
			mptcp_debug("%s: mp_add_addr: bad option size %d\n",
				    __func__, opsize);
			break;
		}

		/* We have to manually parse the options if we got two of them. */
		if (mopt->saw_add_addr) {
			mopt->more_add_addr = 1;
			break;
		}
		mopt->saw_add_addr = 1;
		mopt->add_addr_ptr = ptr;
		break;
	}
	case MPTCP_SUB_REMOVE_ADDR:
		if ((opsize - MPTCP_SUB_LEN_REMOVE_ADDR) < 0) {
			mptcp_debug("%s: mp_remove_addr: bad option size %d\n",
				    __func__, opsize);
			break;
		}

		if (mopt->saw_rem_addr) {
			mopt->more_rem_addr = 1;
			break;
		}
		mopt->saw_rem_addr = 1;
		mopt->rem_addr_ptr = ptr;
		break;
	case MPTCP_SUB_PRIO:
	{
		const struct mp_prio *mpprio = (struct mp_prio *)ptr;

		if (opsize != MPTCP_SUB_LEN_PRIO &&
		    opsize != MPTCP_SUB_LEN_PRIO_ADDR) {
			mptcp_debug("%s: mp_prio: bad option size %d\n",
				    __func__, opsize);
			break;
		}

		mopt->saw_low_prio = 1;
		mopt->low_prio = mpprio->b;

		if (opsize == MPTCP_SUB_LEN_PRIO_ADDR) {
			mopt->saw_low_prio = 2;
			mopt->prio_addr_id = mpprio->addr_id;
		}
		break;
	}
	case MPTCP_SUB_FAIL:
		if (opsize != MPTCP_SUB_LEN_FAIL) {
			mptcp_debug("%s: mp_fail: bad option size %d\n",
				    __func__, opsize);
			break;
		}
		mopt->mp_fail = 1;
		break;
	case MPTCP_SUB_FCLOSE:
		if (opsize != MPTCP_SUB_LEN_FCLOSE) {
			mptcp_debug("%s: mp_fclose: bad option size %d\n",
				    __func__, opsize);
			break;
		}

		mopt->mp_fclose = 1;
		mopt->mptcp_sender_key = ((struct mp_fclose *)ptr)->key;

		break;
	default:
		mptcp_debug("%s: Received unkown subtype: %d\n",
			    __func__, mp_opt->sub);
		break;
	}
}

/** Parse only MPTCP options */
void tcp_parse_mptcp_options(const struct sk_buff *skb,
			     struct mptcp_options_received *mopt)
{
	const struct tcphdr *th = tcp_hdr(skb);
	int length = (th->doff * 4) - sizeof(struct tcphdr);
	const unsigned char *ptr = (const unsigned char *)(th + 1);

	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return;
		case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2)	/* "silly options" */
				return;
			if (opsize > length)
				return;	/* don't parse partial options */
			if (opcode == TCPOPT_MPTCP)
				mptcp_parse_options(ptr - 2, opsize, mopt, skb, NULL);
		}
		ptr += opsize - 2;
		length -= opsize;
	}
}

int mptcp_check_rtt(const struct tcp_sock *tp, int time)
{
	struct mptcp_cb *mpcb = tp->mpcb;
	struct sock *sk;
	u32 rtt_max = 0;

	/* In MPTCP, we take the max delay across all flows,
	 * in order to take into account meta-reordering buffers.
	 */
	mptcp_for_each_sk(mpcb, sk) {
		if (!mptcp_sk_can_recv(sk))
			continue;

		if (rtt_max < tcp_sk(sk)->rcv_rtt_est.rtt)
			rtt_max = tcp_sk(sk)->rcv_rtt_est.rtt;
	}
	if (time < (rtt_max >> 3) || !rtt_max)
		return 1;

	return 0;
}

static void mptcp_handle_add_addr(const unsigned char *ptr, struct sock *sk)
{
	struct mp_add_addr *mpadd = (struct mp_add_addr *)ptr;
	struct mptcp_cb *mpcb = tcp_sk(sk)->mpcb;
	__be16 port = 0;
	union inet_addr addr;
	sa_family_t family;

	if (mpadd->ipver == 4) {
		char *recv_hmac;
		u8 hash_mac_check[20];
		u8 no_key[8];
		int msg_parts = 0;

		if (mpcb->mptcp_ver < MPTCP_VERSION_1)
			goto skip_hmac_v4;

		*(u64 *)no_key = 0;
		recv_hmac = (char *)mpadd->u.v4.mac;
		if (mpadd->len == MPTCP_SUB_LEN_ADD_ADDR4_VER1) {
			recv_hmac -= sizeof(mpadd->u.v4.port);
			msg_parts = 2;
		} else if (mpadd->len == MPTCP_SUB_LEN_ADD_ADDR4_VER1 + 2) {
			msg_parts = 3;
		}
		mptcp_hmac_sha1((u8 *)&mpcb->mptcp_rem_key,
				(u8 *)no_key,
				(u32 *)hash_mac_check, msg_parts,
				1, (u8 *)&mpadd->addr_id,
				4, (u8 *)&mpadd->u.v4.addr.s_addr,
				2, (u8 *)&mpadd->u.v4.port);
		if (memcmp(hash_mac_check, recv_hmac, 8) != 0)
			/* ADD_ADDR2 discarded */
			return;
skip_hmac_v4:
		if ((mpcb->mptcp_ver == MPTCP_VERSION_0 &&
		     mpadd->len == MPTCP_SUB_LEN_ADD_ADDR4 + 2) ||
		     (mpcb->mptcp_ver == MPTCP_VERSION_1 &&
		     mpadd->len == MPTCP_SUB_LEN_ADD_ADDR4_VER1 + 2))
			port  = mpadd->u.v4.port;
		family = AF_INET;
		addr.in = mpadd->u.v4.addr;
#if IS_ENABLED(CONFIG_IPV6)
	} else if (mpadd->ipver == 6) {
		char *recv_hmac;
		u8 hash_mac_check[20];
		u8 no_key[8];
		int msg_parts = 0;

		if (mpcb->mptcp_ver < MPTCP_VERSION_1)
			goto skip_hmac_v6;

		*(u64 *)no_key = 0;
		recv_hmac = (char *)mpadd->u.v6.mac;
		if (mpadd->len == MPTCP_SUB_LEN_ADD_ADDR6_VER1) {
			recv_hmac -= sizeof(mpadd->u.v6.port);
			msg_parts = 2;
		} else if (mpadd->len == MPTCP_SUB_LEN_ADD_ADDR6_VER1 + 2) {
			msg_parts = 3;
		}
		mptcp_hmac_sha1((u8 *)&mpcb->mptcp_rem_key,
				(u8 *)no_key,
				(u32 *)hash_mac_check, msg_parts,
				1, (u8 *)&mpadd->addr_id,
				16, (u8 *)&mpadd->u.v6.addr.s6_addr,
				2, (u8 *)&mpadd->u.v6.port);
		if (memcmp(hash_mac_check, recv_hmac, 8) != 0)
			/* ADD_ADDR2 discarded */
			return;
skip_hmac_v6:
		if ((mpcb->mptcp_ver == MPTCP_VERSION_0 &&
		     mpadd->len == MPTCP_SUB_LEN_ADD_ADDR6 + 2) ||
		     (mpcb->mptcp_ver == MPTCP_VERSION_1 &&
		     mpadd->len == MPTCP_SUB_LEN_ADD_ADDR6_VER1 + 2))
			port  = mpadd->u.v6.port;
		family = AF_INET6;
		addr.in6 = mpadd->u.v6.addr;
#endif /* CONFIG_IPV6 */
	} else {
		return;
	}

	if (mpcb->pm_ops->add_raddr)
		mpcb->pm_ops->add_raddr(mpcb, &addr, family, port, mpadd->addr_id);

	MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_ADDADDRRX);
}

static void mptcp_handle_rem_addr(const unsigned char *ptr, struct sock *sk)
{
	struct mp_remove_addr *mprem = (struct mp_remove_addr *)ptr;
	int i;
	u8 rem_id;
	struct mptcp_cb *mpcb = tcp_sk(sk)->mpcb;

	for (i = 0; i <= mprem->len - MPTCP_SUB_LEN_REMOVE_ADDR; i++) {
		rem_id = (&mprem->addrs_id)[i];

		if (mpcb->pm_ops->rem_raddr)
			mpcb->pm_ops->rem_raddr(mpcb, rem_id);
		mptcp_send_reset_rem_id(mpcb, rem_id);

		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_REMADDRSUB);
	}

	MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_REMADDRRX);
}

static void mptcp_parse_addropt(const struct sk_buff *skb, struct sock *sk)
{
	struct tcphdr *th = tcp_hdr(skb);
	unsigned char *ptr;
	int length = (th->doff * 4) - sizeof(struct tcphdr);

	/* Jump through the options to check whether ADD_ADDR is there */
	ptr = (unsigned char *)(th + 1);
	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return;
		case TCPOPT_NOP:
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2)
				return;
			if (opsize > length)
				return;  /* don't parse partial options */
			if (opcode == TCPOPT_MPTCP &&
			    ((struct mptcp_option *)ptr)->sub == MPTCP_SUB_ADD_ADDR) {
				u8 mptcp_ver = tcp_sk(sk)->mpcb->mptcp_ver;
				struct mp_add_addr *mpadd = (struct mp_add_addr *)ptr;

				if (!is_valid_addropt_opsize(mptcp_ver, mpadd,
							     opsize))
					goto cont;

				mptcp_handle_add_addr(ptr, sk);
			}
			if (opcode == TCPOPT_MPTCP &&
			    ((struct mptcp_option *)ptr)->sub == MPTCP_SUB_REMOVE_ADDR) {
				if ((opsize - MPTCP_SUB_LEN_REMOVE_ADDR) < 0)
					goto cont;

				mptcp_handle_rem_addr(ptr, sk);
			}
cont:
			ptr += opsize - 2;
			length -= opsize;
		}
	}
	return;
}

static inline int mptcp_mp_fail_rcvd(struct sock *sk, const struct tcphdr *th)
{
	struct mptcp_tcp_sock *mptcp = tcp_sk(sk)->mptcp;
	struct sock *meta_sk = mptcp_meta_sk(sk);
	struct mptcp_cb *mpcb = tcp_sk(sk)->mpcb;

	if (unlikely(mptcp->rx_opt.mp_fail)) {
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_MPFAILRX);
		mptcp->rx_opt.mp_fail = 0;

		if (!th->rst && !mpcb->infinite_mapping_snd) {
			mpcb->send_infinite_mapping = 1;
			/* We resend everything that has not been acknowledged */
			meta_sk->sk_send_head = tcp_write_queue_head(meta_sk);

			/* We artificially restart the whole send-queue. Thus,
			 * it is as if no packets are in flight
			 */
			tcp_sk(meta_sk)->packets_out = 0;

			/* If the snd_nxt already wrapped around, we have to
			 * undo the wrapping, as we are restarting from snd_una
			 * on.
			 */
			if (tcp_sk(meta_sk)->snd_nxt < tcp_sk(meta_sk)->snd_una) {
				mpcb->snd_high_order[mpcb->snd_hiseq_index] -= 2;
				mpcb->snd_hiseq_index = mpcb->snd_hiseq_index ? 0 : 1;
			}
			tcp_sk(meta_sk)->snd_nxt = tcp_sk(meta_sk)->snd_una;

			/* Trigger a sending on the meta. */
			mptcp_push_pending_frames(meta_sk);

			mptcp_sub_force_close_all(mpcb, sk);
		}

		return 0;
	}

	if (unlikely(mptcp->rx_opt.mp_fclose)) {
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_FASTCLOSERX);
		mptcp->rx_opt.mp_fclose = 0;
		if (mptcp->rx_opt.mptcp_sender_key != mpcb->mptcp_loc_key)
			return 0;

		mptcp_sub_force_close_all(mpcb, NULL);

		tcp_reset(meta_sk);

		return 1;
	}

	return 0;
}

static inline void mptcp_path_array_check(struct sock *meta_sk)
{
	struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;

	if (unlikely(mpcb->list_rcvd)) {
		mpcb->list_rcvd = 0;
		if (mpcb->pm_ops->new_remote_address)
			mpcb->pm_ops->new_remote_address(meta_sk);
	}
}

int mptcp_handle_options(struct sock *sk, const struct tcphdr *th,
			 const struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct mptcp_options_received *mopt = &tp->mptcp->rx_opt;

	if (tp->mpcb->infinite_mapping_rcv || tp->mpcb->infinite_mapping_snd)
		return 0;

	if (mptcp_mp_fail_rcvd(sk, th))
		return 1;

	/* RFC 6824, Section 3.3:
	 * If a checksum is not present when its use has been negotiated, the
	 * receiver MUST close the subflow with a RST as it is considered broken.
	 */
	if (mptcp_is_data_seq(skb) && tp->mpcb->dss_csum &&
	    !(TCP_SKB_CB(skb)->mptcp_flags & MPTCPHDR_DSS_CSUM)) {
		mptcp_send_reset(sk);
		return 1;
	}

	/* We have to acknowledge retransmissions of the third
	 * ack.
	 */
	if (mopt->join_ack) {
		tcp_send_delayed_ack(sk);
		mopt->join_ack = 0;
	}

	if (mopt->saw_add_addr || mopt->saw_rem_addr) {
		if (mopt->more_add_addr || mopt->more_rem_addr) {
			mptcp_parse_addropt(skb, sk);
		} else {
			if (mopt->saw_add_addr)
				mptcp_handle_add_addr(mopt->add_addr_ptr, sk);
			if (mopt->saw_rem_addr)
				mptcp_handle_rem_addr(mopt->rem_addr_ptr, sk);
		}

		mopt->more_add_addr = 0;
		mopt->saw_add_addr = 0;
		mopt->more_rem_addr = 0;
		mopt->saw_rem_addr = 0;
	}
	if (mopt->saw_low_prio) {
		if (mopt->saw_low_prio == 1) {
			tp->mptcp->rcv_low_prio = mopt->low_prio;
		} else {
			struct sock *sk_it;
			mptcp_for_each_sk(tp->mpcb, sk_it) {
				struct mptcp_tcp_sock *mptcp = tcp_sk(sk_it)->mptcp;
				if (mptcp->rem_id == mopt->prio_addr_id)
					mptcp->rcv_low_prio = mopt->low_prio;
			}
		}
		mopt->saw_low_prio = 0;
	}

	mptcp_data_ack(sk, skb);

	mptcp_path_array_check(mptcp_meta_sk(sk));
	/* Socket may have been mp_killed by a REMOVE_ADDR */
	if (tp->mp_killed)
		return 1;

	return 0;
}

/* In case of fastopen, some data can already be in the write queue.
 * We need to update the sequence number of the segments as they
 * were initially TCP sequence numbers.
 */
static void mptcp_rcv_synsent_fastopen(struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct tcp_sock *master_tp = tcp_sk(meta_tp->mpcb->master_sk);
	struct sk_buff *skb;
	u32 new_mapping = meta_tp->write_seq - master_tp->snd_una;

	/* There should only be one skb in write queue: the data not
	 * acknowledged in the SYN+ACK. In this case, we need to map
	 * this data to data sequence numbers.
	 */
	skb_queue_walk(&meta_sk->sk_write_queue, skb) {
		/* If the server only acknowledges partially the data sent in
		 * the SYN, we need to trim the acknowledged part because
		 * we don't want to retransmit this already received data.
		 * When we reach this point, tcp_ack() has already cleaned up
		 * fully acked segments. However, tcp trims partially acked
		 * segments only when retransmitting. Since MPTCP comes into
		 * play only now, we will fake an initial transmit, and
		 * retransmit_skb() will not be called. The following fragment
		 * comes from __tcp_retransmit_skb().
		 */
		if (before(TCP_SKB_CB(skb)->seq, master_tp->snd_una)) {
			BUG_ON(before(TCP_SKB_CB(skb)->end_seq,
				      master_tp->snd_una));
			/* tcp_trim_head can only returns ENOMEM if skb is
			 * cloned. It is not the case here (see
			 * tcp_send_syn_data).
			 */
			BUG_ON(tcp_trim_head(meta_sk, skb, master_tp->snd_una -
					     TCP_SKB_CB(skb)->seq));
		}

		TCP_SKB_CB(skb)->seq += new_mapping;
		TCP_SKB_CB(skb)->end_seq += new_mapping;
	}

	/* We can advance write_seq by the number of bytes unacknowledged
	 * and that were mapped in the previous loop.
	 */
	meta_tp->write_seq += master_tp->write_seq - master_tp->snd_una;

	/* The packets from the master_sk will be entailed to it later
	 * Until that time, its write queue is empty, and
	 * write_seq must align with snd_una
	 */
	master_tp->snd_nxt = master_tp->write_seq = master_tp->snd_una;
	master_tp->packets_out = 0;

	/* Although these data have been sent already over the subsk,
	 * They have never been sent over the meta_sk, so we rewind
	 * the send_head so that tcp considers it as an initial send
	 * (instead of retransmit).
	 */
	meta_sk->sk_send_head = tcp_write_queue_head(meta_sk);
}

/* The skptr is needed, because if we become MPTCP-capable, we have to switch
 * from meta-socket to master-socket.
 *
 * @return: 1 - we want to reset this connection
 *	    2 - we want to discard the received syn/ack
 *	    0 - everything is fine - continue
 */
int mptcp_rcv_synsent_state_process(struct sock *sk, struct sock **skptr,
				    const struct sk_buff *skb,
				    const struct mptcp_options_received *mopt)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (mptcp(tp)) {
		u8 hash_mac_check[20];
		struct mptcp_cb *mpcb = tp->mpcb;

		mptcp_hmac_sha1((u8 *)&mpcb->mptcp_rem_key,
				(u8 *)&mpcb->mptcp_loc_key,
				(u32 *)hash_mac_check, 2,
				4, (u8 *)&tp->mptcp->rx_opt.mptcp_recv_nonce,
				4, (u8 *)&tp->mptcp->mptcp_loc_nonce);
		if (memcmp(hash_mac_check,
			   (char *)&tp->mptcp->rx_opt.mptcp_recv_tmac, 8)) {
			MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_JOINSYNACKMAC);
			mptcp_sub_force_close(sk);
			return 1;
		}

		/* Set this flag in order to postpone data sending
		 * until the 4th ack arrives.
		 */
		tp->mptcp->pre_established = 1;
		tp->mptcp->rcv_low_prio = tp->mptcp->rx_opt.low_prio;

		mptcp_hmac_sha1((u8 *)&mpcb->mptcp_loc_key,
				(u8 *)&mpcb->mptcp_rem_key,
				(u32 *)&tp->mptcp->sender_mac[0], 2,
				4, (u8 *)&tp->mptcp->mptcp_loc_nonce,
				4, (u8 *)&tp->mptcp->rx_opt.mptcp_recv_nonce);

		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_JOINSYNACKRX);
	} else if (mopt->saw_mpc) {
		struct sock *meta_sk = sk;

		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_MPCAPABLEACTIVEACK);
		if (mopt->mptcp_ver > tcp_sk(sk)->mptcp_ver)
			/* TODO Consider adding new MPTCP_INC_STATS entry */
			goto fallback;

		if (mptcp_create_master_sk(sk, mopt->mptcp_sender_key,
					   mopt->mptcp_ver,
					   ntohs(tcp_hdr(skb)->window)))
			return 2;

		sk = tcp_sk(sk)->mpcb->master_sk;
		*skptr = sk;
		tp = tcp_sk(sk);

		sk->sk_bound_dev_if = skb->skb_iif;

		/* If fastopen was used data might be in the send queue. We
		 * need to update their sequence number to MPTCP-level seqno.
		 * Note that it can happen in rare cases that fastopen_req is
		 * NULL and syn_data is 0 but fastopen indeed occurred and
		 * data has been queued in the write queue (but not sent).
		 * Example of such rare cases: connect is non-blocking and
		 * TFO is configured to work without cookies.
		 */
		if (!skb_queue_empty(&meta_sk->sk_write_queue))
			mptcp_rcv_synsent_fastopen(meta_sk);

		/* -1, because the SYN consumed 1 byte. In case of TFO, we
		 * start the subflow-sequence number as if the data of the SYN
		 * is not part of any mapping.
		 */
		tp->mptcp->snt_isn = tp->snd_una - 1;
		tp->mpcb->dss_csum = mopt->dss_csum;
		if (tp->mpcb->dss_csum)
			MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_CSUMENABLED);

		tp->mptcp->include_mpc = 1;

		/* Ensure that fastopen is handled at the meta-level. */
		tp->fastopen_req = NULL;

		sk_set_socket(sk, mptcp_meta_sk(sk)->sk_socket);
		sk->sk_wq = mptcp_meta_sk(sk)->sk_wq;

		 /* hold in sk_clone_lock due to initialization to 2 */
		sock_put(sk);
	} else {
		MPTCP_INC_STATS_BH(sock_net(sk), MPTCP_MIB_MPCAPABLEACTIVEFALLBACK);
fallback:
		tp->request_mptcp = 0;

		if (tp->inside_tk_table)
			mptcp_hash_remove(tp);
	}

	if (mptcp(tp))
		tp->mptcp->rcv_isn = TCP_SKB_CB(skb)->seq;

	return 0;
}

bool mptcp_should_expand_sndbuf(const struct sock *sk)
{
	const struct sock *sk_it;
	const struct sock *meta_sk = mptcp_meta_sk(sk);
	const struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	int cnt_backups = 0;
	int backup_available = 0;

	/* We circumvent this check in tcp_check_space, because we want to
	 * always call sk_write_space. So, we reproduce the check here.
	 */
	if (!meta_sk->sk_socket ||
	    !test_bit(SOCK_NOSPACE, &meta_sk->sk_socket->flags))
		return false;

	/* If the user specified a specific send buffer setting, do
	 * not modify it.
	 */
	if (meta_sk->sk_userlocks & SOCK_SNDBUF_LOCK)
		return false;

	/* If we are under global TCP memory pressure, do not expand.  */
	if (sk_under_memory_pressure(meta_sk))
		return false;

	/* If we are under soft global TCP memory pressure, do not expand.  */
	if (sk_memory_allocated(meta_sk) >= sk_prot_mem_limits(meta_sk, 0))
		return false;


	/* For MPTCP we look for a subsocket that could send data.
	 * If we found one, then we update the send-buffer.
	 */
	mptcp_for_each_sk(meta_tp->mpcb, sk_it) {
		struct tcp_sock *tp_it = tcp_sk(sk_it);

		if (!mptcp_sk_can_send(sk_it))
			continue;

		/* Backup-flows have to be counted - if there is no other
		 * subflow we take the backup-flow into account.
		 */
		if (tp_it->mptcp->rcv_low_prio || tp_it->mptcp->low_prio)
			cnt_backups++;

		if (tp_it->packets_out < tp_it->snd_cwnd) {
			if (tp_it->mptcp->rcv_low_prio || tp_it->mptcp->low_prio) {
				backup_available = 1;
				continue;
			}
			return true;
		}
	}

	/* Backup-flow is available for sending - update send-buffer */
	if (meta_tp->mpcb->cnt_established == cnt_backups && backup_available)
		return true;
	return false;
}

void mptcp_init_buffer_space(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct sock *meta_sk = mptcp_meta_sk(sk);
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	int space;

	tcp_init_buffer_space(sk);

	if (is_master_tp(tp)) {
		meta_tp->rcvq_space.space = meta_tp->rcv_wnd;
		meta_tp->rcvq_space.time = tcp_time_stamp;
		meta_tp->rcvq_space.seq = meta_tp->copied_seq;

		/* If there is only one subflow, we just use regular TCP
		 * autotuning. User-locks are handled already by
		 * tcp_init_buffer_space
		 */
		meta_tp->window_clamp = tp->window_clamp;
		meta_tp->rcv_ssthresh = tp->rcv_ssthresh;
		meta_sk->sk_rcvbuf = sk->sk_rcvbuf;
		meta_sk->sk_sndbuf = sk->sk_sndbuf;

		return;
	}

	if (meta_sk->sk_userlocks & SOCK_RCVBUF_LOCK)
		goto snd_buf;

	/* Adding a new subflow to the rcv-buffer space. We make a simple
	 * addition, to give some space to allow traffic on the new subflow.
	 * Autotuning will increase it further later on.
	 */
	space = min(meta_sk->sk_rcvbuf + sk->sk_rcvbuf, sysctl_tcp_rmem[2]);
	if (space > meta_sk->sk_rcvbuf) {
		meta_tp->window_clamp += tp->window_clamp;
		meta_tp->rcv_ssthresh += tp->rcv_ssthresh;
		meta_sk->sk_rcvbuf = space;
	}

snd_buf:
	if (meta_sk->sk_userlocks & SOCK_SNDBUF_LOCK)
		return;

	/* Adding a new subflow to the send-buffer space. We make a simple
	 * addition, to give some space to allow traffic on the new subflow.
	 * Autotuning will increase it further later on.
	 */
	space = min(meta_sk->sk_sndbuf + sk->sk_sndbuf, sysctl_tcp_wmem[2]);
	if (space > meta_sk->sk_sndbuf) {
		meta_sk->sk_sndbuf = space;
		meta_sk->sk_write_space(meta_sk);
	}
}

void mptcp_tcp_set_rto(struct sock *sk)
{
	tcp_set_rto(sk);
	mptcp_set_rto(sk);
}
#endif
