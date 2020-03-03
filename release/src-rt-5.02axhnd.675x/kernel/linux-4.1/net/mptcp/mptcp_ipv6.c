#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation - IPv6-specific functions
 *
 *	Initial Design & Implementation:
 *	Sébastien Barré <sebastien.barre@uclouvain.be>
 *
 *	Current Maintainer:
 *	Jaakko Korkeaniemi <jaakko.korkeaniemi@aalto.fi>
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

#include <linux/export.h>
#include <linux/in6.h>
#include <linux/kernel.h>

#include <net/addrconf.h>
#include <net/flow.h>
#include <net/inet6_connection_sock.h>
#include <net/inet6_hashtables.h>
#include <net/inet_common.h>
#include <net/ipv6.h>
#include <net/ip6_checksum.h>
#include <net/ip6_route.h>
#include <net/mptcp.h>
#include <net/mptcp_v6.h>
#include <net/tcp.h>
#include <net/transp_v6.h>

__u32 mptcp_v6_get_nonce(const __be32 *saddr, const __be32 *daddr,
			 __be16 sport, __be16 dport)
{
	u32 secret[MD5_MESSAGE_BYTES / 4];
	u32 hash[MD5_DIGEST_WORDS];
	u32 i;

	memcpy(hash, saddr, 16);
	for (i = 0; i < 4; i++)
		secret[i] = mptcp_secret[i] + (__force u32)daddr[i];
	secret[4] = mptcp_secret[4] +
		    (((__force u16)sport << 16) + (__force u16)dport);
	secret[5] = mptcp_seed++;
	for (i = 6; i < MD5_MESSAGE_BYTES / 4; i++)
		secret[i] = mptcp_secret[i];

	md5_transform(hash, secret);

	return hash[0];
}

u64 mptcp_v6_get_key(const __be32 *saddr, const __be32 *daddr,
		     __be16 sport, __be16 dport, u32 seed)
{
	u32 secret[MD5_MESSAGE_BYTES / 4];
	u32 hash[MD5_DIGEST_WORDS];
	u32 i;

	memcpy(hash, saddr, 16);
	for (i = 0; i < 4; i++)
		secret[i] = mptcp_secret[i] + (__force u32)daddr[i];
	secret[4] = mptcp_secret[4] +
		    (((__force u16)sport << 16) + (__force u16)dport);
	secret[5] = seed;
	for (i = 6; i < MD5_MESSAGE_BYTES / 4; i++)
		secret[i] = mptcp_secret[i];

	md5_transform(hash, secret);

	return *((u64 *)hash);
}

static void mptcp_v6_reqsk_destructor(struct request_sock *req)
{
	mptcp_reqsk_destructor(req);

	tcp_v6_reqsk_destructor(req);
}

static int mptcp_v6_init_req(struct request_sock *req, struct sock *sk,
			     struct sk_buff *skb, bool want_cookie)
{
	tcp_request_sock_ipv6_ops.init_req(req, sk, skb, want_cookie);

	mptcp_rsk(req)->hash_entry.pprev = NULL;
	mptcp_rsk(req)->is_sub = 0;
	inet_rsk(req)->mptcp_rqsk = 1;

	/* In case of SYN-cookies, we wait for the isn to be generated - it is
	 * input to the key-generation.
	 */
	if (!want_cookie)
		mptcp_reqsk_init(req, sk, skb, false);

	return 0;
}

#ifdef CONFIG_SYN_COOKIES
static u32 mptcp_v6_cookie_init_seq(struct request_sock *req, struct sock *sk,
				    const struct sk_buff *skb, __u16 *mssp)
{
	__u32 isn = cookie_v6_init_sequence(req, sk, skb, mssp);

	tcp_rsk(req)->snt_isn = isn;

	mptcp_reqsk_init(req, sk, skb, true);

	return isn;
}
#endif

static int mptcp_v6_join_init_req(struct request_sock *req, struct sock *sk,
				  struct sk_buff *skb, bool want_cookie)
{
	struct mptcp_request_sock *mtreq = mptcp_rsk(req);
	struct mptcp_cb *mpcb = tcp_sk(sk)->mpcb;
	union inet_addr addr;
	int loc_id;
	bool low_prio = false;

	/* We need to do this as early as possible. Because, if we fail later
	 * (e.g., get_local_id), then reqsk_free tries to remove the
	 * request-socket from the htb in mptcp_hash_request_remove as pprev
	 * may be different from NULL.
	 */
	mtreq->hash_entry.pprev = NULL;

	tcp_request_sock_ipv6_ops.init_req(req, sk, skb, want_cookie);

	mtreq->mptcp_loc_nonce = mptcp_v6_get_nonce(ipv6_hdr(skb)->saddr.s6_addr32,
						    ipv6_hdr(skb)->daddr.s6_addr32,
						    tcp_hdr(skb)->source,
						    tcp_hdr(skb)->dest);
	addr.in6 = inet_rsk(req)->ir_v6_loc_addr;
	loc_id = mpcb->pm_ops->get_local_id(AF_INET6, &addr, sock_net(sk), &low_prio);
	if (loc_id == -1)
		return -1;
	mtreq->loc_id = loc_id;
	mtreq->low_prio = low_prio;

	mptcp_join_reqsk_init(mpcb, req, skb);

	return 0;
}

/* Similar to tcp6_request_sock_ops */
struct request_sock_ops mptcp6_request_sock_ops __read_mostly = {
	.family		=	AF_INET6,
	.obj_size	=	sizeof(struct mptcp_request_sock),
	.rtx_syn_ack	=	tcp_rtx_synack,
	.send_ack	=	tcp_v6_reqsk_send_ack,
	.destructor	=	mptcp_v6_reqsk_destructor,
	.send_reset	=	tcp_v6_send_reset,
	.syn_ack_timeout =	tcp_syn_ack_timeout,
};

static void mptcp_v6_reqsk_queue_hash_add(struct sock *meta_sk,
					  struct request_sock *req,
					  const unsigned long timeout)
{
	const u32 h1 = inet6_synq_hash(&inet_rsk(req)->ir_v6_rmt_addr,
				      inet_rsk(req)->ir_rmt_port,
				      0, MPTCP_HASH_SIZE);
	inet6_csk_reqsk_queue_hash_add(meta_sk, req, timeout);

	rcu_read_lock();
	spin_lock(&mptcp_reqsk_hlock);
	hlist_nulls_add_head_rcu(&mptcp_rsk(req)->hash_entry, &mptcp_reqsk_htb[h1]);
	spin_unlock(&mptcp_reqsk_hlock);
	rcu_read_unlock();
}

static int mptcp_v6_join_request(struct sock *meta_sk, struct sk_buff *skb)
{
	return tcp_conn_request(&mptcp6_request_sock_ops,
				&mptcp_join_request_sock_ipv6_ops,
				meta_sk, skb);
}

int mptcp_v6_do_rcv(struct sock *meta_sk, struct sk_buff *skb)
{
	const struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;
	struct sock *child, *rsk = NULL;
	int ret;

	if (!(TCP_SKB_CB(skb)->mptcp_flags & MPTCPHDR_JOIN)) {
		struct tcphdr *th = tcp_hdr(skb);
		const struct ipv6hdr *ip6h = ipv6_hdr(skb);
		struct sock *sk;

		sk = __inet6_lookup_established(sock_net(meta_sk),
						&tcp_hashinfo,
						&ip6h->saddr, th->source,
						&ip6h->daddr, ntohs(th->dest),
						tcp_v6_iif(skb));

		if (!sk) {
			kfree_skb(skb);
			return 0;
		}
		if (is_meta_sk(sk)) {
			WARN("%s Did not find a sub-sk!\n", __func__);
			kfree_skb(skb);
			sock_put(sk);
			return 0;
		}

		if (sk->sk_state == TCP_TIME_WAIT) {
			inet_twsk_put(inet_twsk(sk));
			kfree_skb(skb);
			return 0;
		}

		ret = tcp_v6_do_rcv(sk, skb);
		sock_put(sk);

		return ret;
	}
	TCP_SKB_CB(skb)->mptcp_flags = 0;

	/* Has been removed from the tk-table. Thus, no new subflows.
	 *
	 * Check for close-state is necessary, because we may have been closed
	 * without passing by mptcp_close().
	 *
	 * When falling back, no new subflows are allowed either.
	 */
	if (meta_sk->sk_state == TCP_CLOSE || !tcp_sk(meta_sk)->inside_tk_table ||
	    mpcb->infinite_mapping_rcv || mpcb->send_infinite_mapping)
		goto reset_and_discard;

	child = tcp_v6_hnd_req(meta_sk, skb);

	if (!child)
		goto discard;

	if (child != meta_sk) {
		sock_rps_save_rxhash(child, skb);
		/* We don't call tcp_child_process here, because we hold
		 * already the meta-sk-lock and are sure that it is not owned
		 * by the user.
		 */
		ret = tcp_rcv_state_process(child, skb, tcp_hdr(skb), skb->len);
		bh_unlock_sock(child);
		sock_put(child);
		if (ret) {
			rsk = child;
			goto reset_and_discard;
		}
	} else {
		if (tcp_hdr(skb)->syn) {
			mptcp_v6_join_request(meta_sk, skb);
			goto discard;
		}
		goto reset_and_discard;
	}
	return 0;

reset_and_discard:
	if (reqsk_queue_len(&inet_csk(meta_sk)->icsk_accept_queue)) {
		const struct tcphdr *th = tcp_hdr(skb);
		struct request_sock *req;
		/* If we end up here, it means we should not have matched on the
		 * request-socket. But, because the request-sock queue is only
		 * destroyed in mptcp_close, the socket may actually already be
		 * in close-state (e.g., through shutdown()) while still having
		 * pending request sockets.
		 */
		req = inet6_csk_search_req(meta_sk, th->source,
					   &ipv6_hdr(skb)->saddr,
					   &ipv6_hdr(skb)->daddr, tcp_v6_iif(skb));

		if (req) {
			inet_csk_reqsk_queue_drop(meta_sk, req);
			reqsk_put(req);
		}
	}

	tcp_v6_send_reset(rsk, skb);
discard:
	kfree_skb(skb);
	return 0;
}

/* After this, the ref count of the meta_sk associated with the request_sock
 * is incremented. Thus it is the responsibility of the caller
 * to call sock_put() when the reference is not needed anymore.
 */
struct sock *mptcp_v6_search_req(const __be16 rport, const struct in6_addr *raddr,
				 const struct in6_addr *laddr, const struct net *net)
{
	const struct mptcp_request_sock *mtreq;
	struct sock *meta_sk = NULL;
	const struct hlist_nulls_node *node;
	const u32 hash = inet6_synq_hash(raddr, rport, 0, MPTCP_HASH_SIZE);

	rcu_read_lock();
begin:
	hlist_nulls_for_each_entry_rcu(mtreq, node, &mptcp_reqsk_htb[hash],
				       hash_entry) {
		struct inet_request_sock *treq = inet_rsk(rev_mptcp_rsk(mtreq));
		meta_sk = mtreq->mptcp_mpcb->meta_sk;

		if (inet_rsk(rev_mptcp_rsk(mtreq))->ir_rmt_port == rport &&
		    rev_mptcp_rsk(mtreq)->rsk_ops->family == AF_INET6 &&
		    ipv6_addr_equal(&treq->ir_v6_rmt_addr, raddr) &&
		    ipv6_addr_equal(&treq->ir_v6_loc_addr, laddr) &&
		    net_eq(net, sock_net(meta_sk)))
			goto found;
		meta_sk = NULL;
	}
	/* A request-socket is destroyed by RCU. So, it might have been recycled
	 * and put into another hash-table list. So, after the lookup we may
	 * end up in a different list. So, we may need to restart.
	 *
	 * See also the comment in __inet_lookup_established.
	 */
	if (get_nulls_value(node) != hash + MPTCP_REQSK_NULLS_BASE)
		goto begin;

found:
	if (meta_sk && unlikely(!atomic_inc_not_zero(&meta_sk->sk_refcnt)))
		meta_sk = NULL;
	rcu_read_unlock();

	return meta_sk;
}

/* Create a new IPv6 subflow.
 *
 * We are in user-context and meta-sock-lock is hold.
 */
int mptcp_init6_subsockets(struct sock *meta_sk, const struct mptcp_loc6 *loc,
			   struct mptcp_rem6 *rem)
{
	struct tcp_sock *tp;
	struct sock *sk;
	struct sockaddr_in6 loc_in, rem_in;
	struct socket sock;
	int ret;

	/** First, create and prepare the new socket */

	sock.type = meta_sk->sk_socket->type;
	sock.state = SS_UNCONNECTED;
	sock.wq = meta_sk->sk_socket->wq;
	sock.file = meta_sk->sk_socket->file;
	sock.ops = NULL;

	ret = inet6_create(sock_net(meta_sk), &sock, IPPROTO_TCP, 1);
	if (unlikely(ret < 0)) {
		mptcp_debug("%s inet6_create failed ret: %d\n", __func__, ret);
		return ret;
	}

	sk = sock.sk;
	tp = tcp_sk(sk);

	/* All subsockets need the MPTCP-lock-class */
	lockdep_set_class_and_name(&(sk)->sk_lock.slock, &meta_slock_key, "slock-AF_INET-MPTCP");
	lockdep_init_map(&(sk)->sk_lock.dep_map, "sk_lock-AF_INET-MPTCP", &meta_key, 0);

	if (mptcp_add_sock(meta_sk, sk, loc->loc6_id, rem->rem6_id, GFP_KERNEL))
		goto error;

	tp->mptcp->slave_sk = 1;
	tp->mptcp->low_prio = loc->low_prio;

	/* Initializing the timer for an MPTCP subflow */
	setup_timer(&tp->mptcp->mptcp_ack_timer, mptcp_ack_handler, (unsigned long)sk);

	/** Then, connect the socket to the peer */
	loc_in.sin6_family = AF_INET6;
	rem_in.sin6_family = AF_INET6;
	loc_in.sin6_port = 0;
	if (rem->port)
		rem_in.sin6_port = rem->port;
	else
		rem_in.sin6_port = inet_sk(meta_sk)->inet_dport;
	loc_in.sin6_addr = loc->addr;
	rem_in.sin6_addr = rem->addr;

	if (loc->if_idx)
		sk->sk_bound_dev_if = loc->if_idx;

	ret = sock.ops->bind(&sock, (struct sockaddr *)&loc_in, sizeof(struct sockaddr_in6));
	if (ret < 0) {
		mptcp_debug("%s: MPTCP subsocket bind()failed, error %d\n",
			    __func__, ret);
		goto error;
	}

	mptcp_debug("%s: token %#x pi %d src_addr:%pI6:%d dst_addr:%pI6:%d ifidx: %u\n",
		    __func__, tcp_sk(meta_sk)->mpcb->mptcp_loc_token,
		    tp->mptcp->path_index, &loc_in.sin6_addr,
		    ntohs(loc_in.sin6_port), &rem_in.sin6_addr,
		    ntohs(rem_in.sin6_port), loc->if_idx);

	if (tcp_sk(meta_sk)->mpcb->pm_ops->init_subsocket_v6)
		tcp_sk(meta_sk)->mpcb->pm_ops->init_subsocket_v6(sk, rem->addr);

	ret = sock.ops->connect(&sock, (struct sockaddr *)&rem_in,
				sizeof(struct sockaddr_in6), O_NONBLOCK);
	if (ret < 0 && ret != -EINPROGRESS) {
		mptcp_debug("%s: MPTCP subsocket connect() failed, error %d\n",
			    __func__, ret);
		goto error;
	}

	MPTCP_INC_STATS(sock_net(meta_sk), MPTCP_MIB_JOINSYNTX);

	sk_set_socket(sk, meta_sk->sk_socket);
	sk->sk_wq = meta_sk->sk_wq;

	return 0;

error:
	/* May happen if mptcp_add_sock fails first */
	if (!mptcp(tp)) {
		tcp_close(sk, 0);
	} else {
		local_bh_disable();
		mptcp_sub_force_close(sk);
		local_bh_enable();
	}
	return ret;
}
EXPORT_SYMBOL(mptcp_init6_subsockets);

const struct inet_connection_sock_af_ops mptcp_v6_specific = {
	.queue_xmit	   = inet6_csk_xmit,
	.send_check	   = tcp_v6_send_check,
	.rebuild_header	   = inet6_sk_rebuild_header,
	.sk_rx_dst_set	   = inet6_sk_rx_dst_set,
	.conn_request	   = mptcp_conn_request,
	.syn_recv_sock	   = tcp_v6_syn_recv_sock,
	.net_header_len	   = sizeof(struct ipv6hdr),
	.net_frag_header_len = sizeof(struct frag_hdr),
	.setsockopt	   = ipv6_setsockopt,
	.getsockopt	   = ipv6_getsockopt,
	.addr2sockaddr	   = inet6_csk_addr2sockaddr,
	.sockaddr_len	   = sizeof(struct sockaddr_in6),
	.bind_conflict	   = inet6_csk_bind_conflict,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_ipv6_setsockopt,
	.compat_getsockopt = compat_ipv6_getsockopt,
#endif
	.mtu_reduced	   = tcp_v6_mtu_reduced,
};

const struct inet_connection_sock_af_ops mptcp_v6_mapped = {
	.queue_xmit	   = ip_queue_xmit,
	.send_check	   = tcp_v4_send_check,
	.rebuild_header	   = inet_sk_rebuild_header,
	.sk_rx_dst_set	   = inet_sk_rx_dst_set,
	.conn_request	   = mptcp_conn_request,
	.syn_recv_sock	   = tcp_v6_syn_recv_sock,
	.net_header_len	   = sizeof(struct iphdr),
	.setsockopt	   = ipv6_setsockopt,
	.getsockopt	   = ipv6_getsockopt,
	.addr2sockaddr	   = inet6_csk_addr2sockaddr,
	.sockaddr_len	   = sizeof(struct sockaddr_in6),
	.bind_conflict	   = inet6_csk_bind_conflict,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_ipv6_setsockopt,
	.compat_getsockopt = compat_ipv6_getsockopt,
#endif
	.mtu_reduced	   = tcp_v4_mtu_reduced,
};

struct tcp_request_sock_ops mptcp_request_sock_ipv6_ops;
struct tcp_request_sock_ops mptcp_join_request_sock_ipv6_ops;

int mptcp_pm_v6_init(void)
{
	int ret = 0;
	struct request_sock_ops *ops = &mptcp6_request_sock_ops;

	mptcp_request_sock_ipv6_ops = tcp_request_sock_ipv6_ops;
	mptcp_request_sock_ipv6_ops.init_req = mptcp_v6_init_req;
#ifdef CONFIG_SYN_COOKIES
	mptcp_request_sock_ipv6_ops.cookie_init_seq = mptcp_v6_cookie_init_seq;
#endif

	mptcp_join_request_sock_ipv6_ops = tcp_request_sock_ipv6_ops;
	mptcp_join_request_sock_ipv6_ops.init_req = mptcp_v6_join_init_req;
	mptcp_join_request_sock_ipv6_ops.queue_hash_add = mptcp_v6_reqsk_queue_hash_add;

	ops->slab_name = kasprintf(GFP_KERNEL, "request_sock_%s", "MPTCP6");
	if (ops->slab_name == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	ops->slab = kmem_cache_create(ops->slab_name, ops->obj_size, 0,
				      SLAB_DESTROY_BY_RCU|SLAB_HWCACHE_ALIGN,
				      NULL);

	if (ops->slab == NULL) {
		ret =  -ENOMEM;
		goto err_reqsk_create;
	}

out:
	return ret;

err_reqsk_create:
	kfree(ops->slab_name);
	ops->slab_name = NULL;
	goto out;
}

void mptcp_pm_v6_undo(void)
{
	kmem_cache_destroy(mptcp6_request_sock_ops.slab);
	kfree(mptcp6_request_sock_ops.slab_name);
}
#endif
