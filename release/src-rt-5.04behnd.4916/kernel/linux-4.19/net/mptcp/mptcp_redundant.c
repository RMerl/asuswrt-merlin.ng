#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
/*
 *	MPTCP Scheduler to reduce latency and jitter.
 *
 *	This scheduler sends all packets redundantly on all available subflows.
 *
 *	Initial Design & Implementation:
 *	Tobias Erbshaeusser <erbshauesser@dvs.tu-darmstadt.de>
 *	Alexander Froemmgen <froemmge@dvs.tu-darmstadt.de>
 *
 *	Initial corrections & modifications:
 *	Christian Pinedo <christian.pinedo@ehu.eus>
 *	Igor Lopez <igor.lopez@ehu.eus>
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <net/mptcp.h>

/* Struct to store the data of a single subflow */
struct redsched_priv {
	/* The skb or NULL */
	struct sk_buff *skb;
	/* End sequence number of the skb. This number should be checked
	 * to be valid before the skb field is used
	 */
	u32 skb_end_seq;
};

/* Struct to store the data of the control block */
struct redsched_cb {
	/* The next subflow where a skb should be sent or NULL */
	struct tcp_sock *next_subflow;
};

/* Returns the socket data from a given subflow socket */
static struct redsched_priv *redsched_get_priv(struct tcp_sock *tp)
{
	return (struct redsched_priv *)&tp->mptcp->mptcp_sched[0];
}

/* Returns the control block data from a given meta socket */
static struct redsched_cb *redsched_get_cb(struct tcp_sock *tp)
{
	return (struct redsched_cb *)&tp->mpcb->mptcp_sched[0];
}

static bool redsched_get_active_valid_sks(struct sock *meta_sk)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	struct mptcp_tcp_sock *mptcp;
	int active_valid_sks = 0;

	mptcp_for_each_sub(mpcb, mptcp) {
		struct sock *sk = mptcp_to_sock(mptcp);

		if (subflow_is_active((struct tcp_sock *)sk) &&
		    !mptcp_is_def_unavailable(sk))
			active_valid_sks++;
	}

	return active_valid_sks;
}

static bool redsched_use_subflow(struct sock *meta_sk,
				 int active_valid_sks,
				 struct tcp_sock *tp,
				 struct sk_buff *skb)
{
	if (!skb || !mptcp_is_available((struct sock *)tp, skb, false))
		return false;

	if (TCP_SKB_CB(skb)->path_mask != 0)
		return subflow_is_active(tp);

	if (TCP_SKB_CB(skb)->path_mask == 0) {
		if (active_valid_sks == -1)
			active_valid_sks = redsched_get_active_valid_sks(meta_sk);

		if (subflow_is_backup(tp) && active_valid_sks > 0)
			return false;
		else
			return true;
	}

	return false;
}

#define mptcp_entry_next_rcu(__mptcp)						\
	hlist_entry_safe(rcu_dereference_raw(hlist_next_rcu(			\
		&(__mptcp)->node)), struct mptcp_tcp_sock, node)

static void redsched_update_next_subflow(struct tcp_sock *tp,
					 struct redsched_cb *red_cb)
{
	struct mptcp_tcp_sock *mptcp = mptcp_entry_next_rcu(tp->mptcp);

	if (mptcp)
		red_cb->next_subflow = mptcp->tp;
	else
		red_cb->next_subflow = NULL;
}

static struct sock *red_get_available_subflow(struct sock *meta_sk,
					      struct sk_buff *skb,
					      bool zero_wnd_test)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	struct redsched_cb *red_cb = redsched_get_cb(meta_tp);
	struct tcp_sock *first_tp = red_cb->next_subflow, *tp;
	struct mptcp_tcp_sock *mptcp;
	int found = 0;

	/* Answer data_fin on same subflow */
	if (meta_sk->sk_shutdown & RCV_SHUTDOWN &&
	    skb && mptcp_is_data_fin(skb)) {
		mptcp_for_each_sub(mpcb, mptcp) {
			struct sock *sk = mptcp_to_sock(mptcp);

			if (tcp_sk(sk)->mptcp->path_index ==
				mpcb->dfin_path_index &&
			    mptcp_is_available(sk, skb, zero_wnd_test))
				return sk;
		}
	}

	if (!first_tp && !hlist_empty(&mpcb->conn_list)) {
		first_tp = hlist_entry_safe(rcu_dereference_raw(hlist_first_rcu(&mpcb->conn_list)),
					    struct mptcp_tcp_sock, node)->tp;
	}
	tp = first_tp;

	/* still NULL (no subflow in conn_list?) */
	if (!first_tp)
		return NULL;

	/* Search for a subflow to send it.
	 *
	 * We want to pick a subflow that is after 'first_tp' in the list of subflows.
	 * Thus, the first mptcp_for_each_sub()-loop tries to walk the list up
	 * to the subflow 'tp' and then checks whether any one of the remaining
	 * ones is eligible to send.
	 * The second mptcp_for_each-sub()-loop is then iterating from the
	 * beginning of the list up to 'first_tp'.
	 */
	mptcp_for_each_sub(mpcb, mptcp) {
		/* We go up to the subflow 'tp' and start from there */
		if (tp == mptcp->tp)
			found = 1;

		if (!found)
			continue;
		tp = mptcp->tp;

		if (mptcp_is_available((struct sock *)tp, skb,
				       zero_wnd_test)) {
			redsched_update_next_subflow(tp, red_cb);
			return (struct sock *)tp;
		}
	}

	mptcp_for_each_sub(mpcb, mptcp) {
		tp = mptcp->tp;

		if (tp == first_tp)
			break;

		if (mptcp_is_available((struct sock *)tp, skb,
				       zero_wnd_test)) {
			redsched_update_next_subflow(tp, red_cb);
			return (struct sock *)tp;
		}
	}

	/* No space */
	return NULL;
}

/* Corrects the stored skb pointers if they are invalid */
static void redsched_correct_skb_pointers(struct sock *meta_sk,
					  struct redsched_priv *red_p)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);

	if (red_p->skb && !after(red_p->skb_end_seq, meta_tp->snd_una))
		red_p->skb = NULL;
}

/* Returns the next skb from the queue */
static struct sk_buff *redsched_next_skb_from_queue(struct sk_buff_head *queue,
						    struct sk_buff *previous,
						    struct sock *meta_sk)
{
	struct sk_buff *skb;

	if (!previous)
		return skb_peek(queue);

	/* sk_data->skb stores the last scheduled packet for this subflow.
	 * If sk_data->skb was scheduled but not sent (e.g., due to nagle),
	 * we have to schedule it again.
	 *
	 * For the redundant scheduler, there are two cases:
	 * 1. sk_data->skb was not sent on another subflow:
	 *    we have to schedule it again to ensure that we do not
	 *    skip this packet.
	 * 2. sk_data->skb was already sent on another subflow:
	 *    with regard to the redundant semantic, we have to
	 *    schedule it again. However, we keep it simple and ignore it,
	 *    as it was already sent by another subflow.
	 *    This might be changed in the future.
	 *
	 * For case 1, send_head is equal previous, as only a single
	 * packet can be skipped.
	 */
	if (tcp_send_head(meta_sk) == previous)
		return tcp_send_head(meta_sk);

	skb = skb_rb_next(previous);
	if (skb)
		return skb;

	return tcp_send_head(meta_sk);
}

static struct sk_buff *mptcp_red_next_segment(struct sock *meta_sk,
					      int *reinject,
					      struct sock **subsk,
					      unsigned int *limit)
{
	struct tcp_sock *meta_tp = tcp_sk(meta_sk);
	struct mptcp_cb *mpcb = meta_tp->mpcb;
	struct redsched_cb *red_cb = redsched_get_cb(meta_tp);
	struct tcp_sock *first_tp = red_cb->next_subflow, *tp;
	struct mptcp_tcp_sock *mptcp;
	int active_valid_sks = -1;
	struct sk_buff *skb;
	int found = 0;

	/* As we set it, we have to reset it as well. */
	*limit = 0;

	if (skb_queue_empty(&mpcb->reinject_queue) &&
	    skb_queue_empty(&meta_sk->sk_write_queue))
		/* Nothing to send */
		return NULL;

	/* First try reinjections */
	skb = skb_peek(&mpcb->reinject_queue);
	if (skb) {
		*subsk = get_available_subflow(meta_sk, skb, false);
		if (!*subsk)
			return NULL;
		*reinject = 1;
		return skb;
	}

	/* Then try indistinctly redundant and normal skbs */

	if (!first_tp && !hlist_empty(&mpcb->conn_list)) {
		first_tp = hlist_entry_safe(rcu_dereference_raw(hlist_first_rcu(&mpcb->conn_list)),
					    struct mptcp_tcp_sock, node)->tp;
	}

	/* still NULL (no subflow in conn_list?) */
	if (!first_tp)
		return NULL;

	tp = first_tp;

	*reinject = 0;
	active_valid_sks = redsched_get_active_valid_sks(meta_sk);

	/* We want to pick a subflow that is after 'first_tp' in the list of subflows.
	 * Thus, the first mptcp_for_each_sub()-loop tries to walk the list up
	 * to the subflow 'tp' and then checks whether any one of the remaining
	 * ones can send a segment.
	 * The second mptcp_for_each-sub()-loop is then iterating from the
	 * beginning of the list up to 'first_tp'.
	 */
	mptcp_for_each_sub(mpcb, mptcp) {
		struct redsched_priv *red_p;

		if (tp == mptcp->tp)
			found = 1;

		if (!found)
			continue;

		tp = mptcp->tp;

		/* Correct the skb pointers of the current subflow */
		red_p = redsched_get_priv(tp);
		redsched_correct_skb_pointers(meta_sk, red_p);

		skb = redsched_next_skb_from_queue(&meta_sk->sk_write_queue,
						   red_p->skb, meta_sk);
		if (skb && redsched_use_subflow(meta_sk, active_valid_sks, tp,
						skb)) {
			red_p->skb = skb;
			red_p->skb_end_seq = TCP_SKB_CB(skb)->end_seq;
			redsched_update_next_subflow(tp, red_cb);
			*subsk = (struct sock *)tp;

			if (TCP_SKB_CB(skb)->path_mask)
				*reinject = -1;
			return skb;
		}
	}

	mptcp_for_each_sub(mpcb, mptcp) {
		struct redsched_priv *red_p;

		tp = mptcp->tp;

		if (tp == first_tp)
			break;

		/* Correct the skb pointers of the current subflow */
		red_p = redsched_get_priv(tp);
		redsched_correct_skb_pointers(meta_sk, red_p);

		skb = redsched_next_skb_from_queue(&meta_sk->sk_write_queue,
						   red_p->skb, meta_sk);
		if (skb && redsched_use_subflow(meta_sk, active_valid_sks, tp,
						skb)) {
			red_p->skb = skb;
			red_p->skb_end_seq = TCP_SKB_CB(skb)->end_seq;
			redsched_update_next_subflow(tp, red_cb);
			*subsk = (struct sock *)tp;

			if (TCP_SKB_CB(skb)->path_mask)
				*reinject = -1;
			return skb;
		}
	}

	/* Nothing to send */
	return NULL;
}

static void redsched_release(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct redsched_cb *red_cb = redsched_get_cb(tp);

	/* Check if the next subflow would be the released one. If yes correct
	 * the pointer
	 */
	if (red_cb->next_subflow == tp)
		redsched_update_next_subflow(tp, red_cb);
}

static struct mptcp_sched_ops mptcp_sched_red = {
	.get_subflow = red_get_available_subflow,
	.next_segment = mptcp_red_next_segment,
	.release = redsched_release,
	.name = "redundant",
	.owner = THIS_MODULE,
};

static int __init red_register(void)
{
	BUILD_BUG_ON(sizeof(struct redsched_priv) > MPTCP_SCHED_SIZE);
	BUILD_BUG_ON(sizeof(struct redsched_cb) > MPTCP_SCHED_DATA_SIZE);

	if (mptcp_register_scheduler(&mptcp_sched_red))
		return -1;

	return 0;
}

static void red_unregister(void)
{
	mptcp_unregister_scheduler(&mptcp_sched_red);
}

module_init(red_register);
module_exit(red_unregister);

MODULE_AUTHOR("Tobias Erbshaeusser, Alexander Froemmgen");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("REDUNDANT MPTCP");
MODULE_VERSION("0.90");
#endif
