#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
/*
 *	MPTCP implementation - IPv4-specific functions
 *
 *	Initial Design & Implementation:
 *	Sébastien Barré <sebastien.barre@uclouvain.be>
 *
 *	Current Maintainer:
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

#include <linux/export.h>
#include <linux/ip.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/tcp.h>

#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/mptcp.h>
#include <net/mptcp_v4.h>
#include <net/request_sock.h>
#include <net/tcp.h>

u32 mptcp_v4_get_nonce(__be32 saddr, __be32 daddr, __be16 sport, __be16 dport)
{
	return siphash_4u32((__force u32)saddr, (__force u32)daddr,
			    (__force u32)sport << 16 | (__force u32)dport,
			    mptcp_seed++, &mptcp_secret);
}

u64 mptcp_v4_get_key(__be32 saddr, __be32 daddr, __be16 sport, __be16 dport,
		     u32 seed)
{
	return siphash_2u64((__force u64)saddr << 32 | (__force u64)daddr,
			    (__force u64)seed << 32 | (__force u64)sport << 16 | (__force u64)dport,
			    &mptcp_secret);
}


static void mptcp_v4_reqsk_destructor(struct request_sock *req)
{
	mptcp_reqsk_destructor(req);

	tcp_v4_reqsk_destructor(req);
}

static int mptcp_v4_init_req(struct request_sock *req, const struct sock *sk,
			     struct sk_buff *skb, bool want_cookie)
{
	tcp_request_sock_ipv4_ops.init_req(req, sk, skb, want_cookie);

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
static u32 mptcp_v4_cookie_init_seq(struct request_sock *req, const struct sock *sk,
				    const struct sk_buff *skb, __u16 *mssp)
{
	__u32 isn = cookie_v4_init_sequence(req, sk, skb, mssp);

	tcp_rsk(req)->snt_isn = isn;

	mptcp_reqsk_init(req, sk, skb, true);

	return isn;
}
#endif

/* May be called without holding the meta-level lock */
static int mptcp_v4_join_init_req(struct request_sock *req, const struct sock *meta_sk,
				  struct sk_buff *skb, bool want_cookie)
{
	struct mptcp_request_sock *mtreq = mptcp_rsk(req);
	const struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;
	union inet_addr addr;
	int loc_id;
	bool low_prio = false;

	/* We need to do this as early as possible. Because, if we fail later
	 * (e.g., get_local_id), then reqsk_free tries to remove the
	 * request-socket from the htb in mptcp_hash_request_remove as pprev
	 * may be different from NULL.
	 */
	mtreq->hash_entry.pprev = NULL;

	tcp_request_sock_ipv4_ops.init_req(req, meta_sk, skb, want_cookie);

	mtreq->mptcp_loc_nonce = mptcp_v4_get_nonce(ip_hdr(skb)->saddr,
						    ip_hdr(skb)->daddr,
						    tcp_hdr(skb)->source,
						    tcp_hdr(skb)->dest);
	addr.ip = inet_rsk(req)->ir_loc_addr;
	loc_id = mpcb->pm_ops->get_local_id(meta_sk, AF_INET, &addr, &low_prio);
	if (loc_id == -1)
		return -1;
	mtreq->loc_id = loc_id;
	mtreq->low_prio = low_prio;

	mptcp_join_reqsk_init(mpcb, req, skb);

	return 0;
}

/* Similar to tcp_request_sock_ops */
struct request_sock_ops mptcp_request_sock_ops __read_mostly = {
	.family		=	PF_INET,
	.obj_size	=	sizeof(struct mptcp_request_sock),
	.rtx_syn_ack	=	tcp_rtx_synack,
	.send_ack	=	tcp_v4_reqsk_send_ack,
	.destructor	=	mptcp_v4_reqsk_destructor,
	.send_reset	=	tcp_v4_send_reset,
	.syn_ack_timeout =	tcp_syn_ack_timeout,
};

/* Similar to: tcp_v4_conn_request
 * May be called without holding the meta-level lock
 */
static int mptcp_v4_join_request(struct sock *meta_sk, struct sk_buff *skb)
{
	return tcp_conn_request(&mptcp_request_sock_ops,
				&mptcp_join_request_sock_ipv4_ops,
				meta_sk, skb);
}

/* Similar to: tcp_v4_do_rcv
 * We only process join requests here. (either the SYN or the final ACK)
 */
int mptcp_v4_do_rcv(struct sock *meta_sk, struct sk_buff *skb)
{
	const struct tcphdr *th = tcp_hdr(skb);
	const struct iphdr *iph = ip_hdr(skb);
	struct sock *child, *rsk = NULL, *sk;
	int ret;

	sk = inet_lookup_established(sock_net(meta_sk), &tcp_hashinfo,
				     iph->saddr, th->source, iph->daddr,
				     th->dest, inet_iif(skb));

	if (!sk)
		goto new_subflow;

	if (is_meta_sk(sk)) {
		WARN("%s Did not find a sub-sk - did found the meta!\n", __func__);
		sock_put(sk);
		goto discard;
	}

	if (sk->sk_state == TCP_TIME_WAIT) {
		inet_twsk_put(inet_twsk(sk));
		goto discard;
	}

	if (sk->sk_state == TCP_NEW_SYN_RECV) {
		struct request_sock *req = inet_reqsk(sk);
		bool req_stolen;

		if (!mptcp_can_new_subflow(meta_sk))
			goto reset_and_discard;

		local_bh_disable();
		child = tcp_check_req(meta_sk, skb, req, false, &req_stolen);
		if (!child) {
			reqsk_put(req);
			local_bh_enable();
			goto discard;
		}

		if (child != meta_sk) {
			ret = mptcp_finish_handshake(child, skb);
			if (ret) {
				rsk = child;
				local_bh_enable();
				goto reset_and_discard;
			}

			local_bh_enable();
			return 0;
		}

		/* tcp_check_req failed */
		reqsk_put(req);

		local_bh_enable();
		goto discard;
	}

	ret = tcp_v4_do_rcv(sk, skb);
	sock_put(sk);

	return ret;

new_subflow:
	if (!mptcp_can_new_subflow(meta_sk))
		goto reset_and_discard;

	child = tcp_v4_cookie_check(meta_sk, skb);
	if (!child)
		goto discard;

	if (child != meta_sk) {
		ret = mptcp_finish_handshake(child, skb);
		if (ret) {
			rsk = child;
			goto reset_and_discard;
		}
	}

	if (tcp_hdr(skb)->syn) {
		local_bh_disable();
		mptcp_v4_join_request(meta_sk, skb);
		local_bh_enable();
	}

discard:
	kfree_skb(skb);
	return 0;

reset_and_discard:
	tcp_v4_send_reset(rsk, skb);
	goto discard;
}

/* Create a new IPv4 subflow.
 *
 * We are in user-context and meta-sock-lock is hold.
 */
int __mptcp_init4_subsockets(struct sock *meta_sk, const struct mptcp_loc4 *loc,
			     __be16 sport, struct mptcp_rem4 *rem,
			     struct sock **subsk)
{
	struct tcp_sock *tp;
	struct sock *sk;
	struct sockaddr_in loc_in, rem_in;
	struct socket_alloc sock_full;
	struct socket *sock = (struct socket *)&sock_full;
	int ret;

	/** First, create and prepare the new socket */
	memcpy(&sock_full, meta_sk->sk_socket, sizeof(sock_full));
	sock->state = SS_UNCONNECTED;
	sock->ops = NULL;

	ret = inet_create(sock_net(meta_sk), sock, IPPROTO_TCP, 1);
	if (unlikely(ret < 0)) {
		net_err_ratelimited("%s inet_create failed ret: %d\n",
				    __func__, ret);
		return ret;
	}

	sk = sock->sk;
	tp = tcp_sk(sk);

	/* All subsockets need the MPTCP-lock-class */
	lockdep_set_class_and_name(&(sk)->sk_lock.slock, &meta_slock_key, meta_slock_key_name);
	lockdep_init_map(&(sk)->sk_lock.dep_map, meta_key_name, &meta_key, 0);

	ret = mptcp_add_sock(meta_sk, sk, loc->loc4_id, rem->rem4_id, GFP_KERNEL);
	if (ret) {
		net_err_ratelimited("%s mptcp_add_sock failed ret: %d\n",
				    __func__, ret);
		goto error;
	}

	tp->mptcp->slave_sk = 1;
	tp->mptcp->low_prio = loc->low_prio;

	/* Initializing the timer for an MPTCP subflow */
	timer_setup(&tp->mptcp->mptcp_ack_timer, mptcp_ack_handler, 0);

	/** Then, connect the socket to the peer */
	loc_in.sin_family = AF_INET;
	rem_in.sin_family = AF_INET;
	loc_in.sin_port = sport;
	if (rem->port)
		rem_in.sin_port = rem->port;
	else
		rem_in.sin_port = inet_sk(meta_sk)->inet_dport;
	loc_in.sin_addr = loc->addr;
	rem_in.sin_addr = rem->addr;

	if (loc->if_idx)
		sk->sk_bound_dev_if = loc->if_idx;

	ret = kernel_bind(sock, (struct sockaddr *)&loc_in,
			  sizeof(struct sockaddr_in));
	if (ret < 0) {
		net_err_ratelimited("%s: token %#x bind() to %pI4 index %d failed, error %d\n",
				    __func__, tcp_sk(meta_sk)->mpcb->mptcp_loc_token,
				    &loc_in.sin_addr, loc->if_idx, ret);
		goto error;
	}

	mptcp_debug("%s: token %#x pi %d src_addr:%pI4:%d dst_addr:%pI4:%d ifidx: %d\n",
		    __func__, tcp_sk(meta_sk)->mpcb->mptcp_loc_token,
		    tp->mptcp->path_index, &loc_in.sin_addr,
		    ntohs(loc_in.sin_port), &rem_in.sin_addr,
		    ntohs(rem_in.sin_port), loc->if_idx);

	if (tcp_sk(meta_sk)->mpcb->pm_ops->init_subsocket_v4)
		tcp_sk(meta_sk)->mpcb->pm_ops->init_subsocket_v4(sk, rem->addr);

	ret = kernel_connect(sock, (struct sockaddr *)&rem_in,
			     sizeof(struct sockaddr_in), O_NONBLOCK);
	if (ret < 0 && ret != -EINPROGRESS) {
		net_err_ratelimited("%s: MPTCP subsocket connect() failed, error %d\n",
				    __func__, ret);
		goto error;
	}

	MPTCP_INC_STATS(sock_net(meta_sk), MPTCP_MIB_JOINSYNTX);

	sk_set_socket(sk, meta_sk->sk_socket);
	sk->sk_wq = meta_sk->sk_wq;

	if (subsk)
		*subsk = sk;

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
EXPORT_SYMBOL(__mptcp_init4_subsockets);

const struct inet_connection_sock_af_ops mptcp_v4_specific = {
	.queue_xmit	   = ip_queue_xmit,
	.send_check	   = tcp_v4_send_check,
	.rebuild_header	   = inet_sk_rebuild_header,
	.sk_rx_dst_set	   = inet_sk_rx_dst_set,
	.conn_request	   = mptcp_conn_request,
	.syn_recv_sock	   = tcp_v4_syn_recv_sock,
	.net_header_len	   = sizeof(struct iphdr),
	.setsockopt	   = ip_setsockopt,
	.getsockopt	   = ip_getsockopt,
	.addr2sockaddr	   = inet_csk_addr2sockaddr,
	.sockaddr_len	   = sizeof(struct sockaddr_in),
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_ip_setsockopt,
	.compat_getsockopt = compat_ip_getsockopt,
#endif
	.mtu_reduced	   = tcp_v4_mtu_reduced,
};

struct tcp_request_sock_ops mptcp_request_sock_ipv4_ops;
struct tcp_request_sock_ops mptcp_join_request_sock_ipv4_ops;

/* General initialization of IPv4 for MPTCP */
int mptcp_pm_v4_init(void)
{
	int ret = 0;
	struct request_sock_ops *ops = &mptcp_request_sock_ops;

	mptcp_request_sock_ipv4_ops = tcp_request_sock_ipv4_ops;
	mptcp_request_sock_ipv4_ops.init_req = mptcp_v4_init_req;
#ifdef CONFIG_SYN_COOKIES
	mptcp_request_sock_ipv4_ops.cookie_init_seq = mptcp_v4_cookie_init_seq;
#endif
	mptcp_join_request_sock_ipv4_ops = tcp_request_sock_ipv4_ops;
	mptcp_join_request_sock_ipv4_ops.init_req = mptcp_v4_join_init_req;

	ops->slab_name = kasprintf(GFP_KERNEL, "request_sock_%s", "MPTCP");
	if (ops->slab_name == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	ops->slab = kmem_cache_create(ops->slab_name, ops->obj_size, 0,
				      SLAB_TYPESAFE_BY_RCU|SLAB_HWCACHE_ALIGN,
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

void mptcp_pm_v4_undo(void)
{
	kmem_cache_destroy(mptcp_request_sock_ops.slab);
	kfree(mptcp_request_sock_ops.slab_name);
}
#endif
