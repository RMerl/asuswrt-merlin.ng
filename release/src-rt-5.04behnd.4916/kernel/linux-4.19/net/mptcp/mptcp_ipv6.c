#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
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
	const struct {
		struct in6_addr saddr;
		struct in6_addr daddr;
		u32 seed;
		__be16 sport;
		__be16 dport;
	} __aligned(SIPHASH_ALIGNMENT) combined = {
		.saddr = *(struct in6_addr *)saddr,
		.daddr = *(struct in6_addr *)daddr,
		.seed = mptcp_seed++,
		.sport = sport,
		.dport = dport
	};

	return siphash(&combined, offsetofend(typeof(combined), dport),
		       &mptcp_secret);
}

u64 mptcp_v6_get_key(const __be32 *saddr, const __be32 *daddr,
		     __be16 sport, __be16 dport, u32 seed)
{
	const struct {
		struct in6_addr saddr;
		struct in6_addr daddr;
		u32 seed;
		__be16 sport;
		__be16 dport;
	} __aligned(SIPHASH_ALIGNMENT) combined = {
		.saddr = *(struct in6_addr *)saddr,
		.daddr = *(struct in6_addr *)daddr,
		.seed = seed,
		.sport = sport,
		.dport = dport
	};

	return siphash(&combined, offsetofend(typeof(combined), dport),
		       &mptcp_secret);
}

static void mptcp_v6_reqsk_destructor(struct request_sock *req)
{
	mptcp_reqsk_destructor(req);

	tcp_v6_reqsk_destructor(req);
}

static int mptcp_v6_init_req(struct request_sock *req, const struct sock *sk,
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
static u32 mptcp_v6_cookie_init_seq(struct request_sock *req, const struct sock *sk,
				    const struct sk_buff *skb, __u16 *mssp)
{
	__u32 isn = cookie_v6_init_sequence(req, sk, skb, mssp);

	tcp_rsk(req)->snt_isn = isn;

	mptcp_reqsk_init(req, sk, skb, true);

	return isn;
}
#endif

/* May be called without holding the meta-level lock */
static int mptcp_v6_join_init_req(struct request_sock *req, const struct sock *meta_sk,
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

	tcp_request_sock_ipv6_ops.init_req(req, meta_sk, skb, want_cookie);

	mtreq->mptcp_loc_nonce = mptcp_v6_get_nonce(ipv6_hdr(skb)->saddr.s6_addr32,
						    ipv6_hdr(skb)->daddr.s6_addr32,
						    tcp_hdr(skb)->source,
						    tcp_hdr(skb)->dest);
	addr.in6 = inet_rsk(req)->ir_v6_loc_addr;
	loc_id = mpcb->pm_ops->get_local_id(meta_sk, AF_INET6, &addr, &low_prio);
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

/* Similar to: tcp_v6_conn_request
 * May be called without holding the meta-level lock
 */
static int mptcp_v6_join_request(struct sock *meta_sk, struct sk_buff *skb)
{
	return tcp_conn_request(&mptcp6_request_sock_ops,
				&mptcp_join_request_sock_ipv6_ops,
				meta_sk, skb);
}

int mptcp_v6_do_rcv(struct sock *meta_sk, struct sk_buff *skb)
{
	const struct tcphdr *th = tcp_hdr(skb);
	const struct ipv6hdr *ip6h = ipv6_hdr(skb);
	struct sock *child, *rsk = NULL, *sk;
	int ret;

	sk = __inet6_lookup_established(sock_net(meta_sk),
					&tcp_hashinfo,
					&ip6h->saddr, th->source,
					&ip6h->daddr, ntohs(th->dest),
					tcp_v6_iif(skb), tcp_v6_sdif(skb));

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

	ret = tcp_v6_do_rcv(sk, skb);
	sock_put(sk);

	return ret;

new_subflow:
	if (!mptcp_can_new_subflow(meta_sk))
		goto reset_and_discard;

	child = tcp_v6_cookie_check(meta_sk, skb);
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
		mptcp_v6_join_request(meta_sk, skb);
		local_bh_enable();
	}

discard:
	kfree_skb(skb);
	return 0;

reset_and_discard:
	tcp_v6_send_reset(rsk, skb);
	goto discard;
}

/* Create a new IPv6 subflow.
 *
 * We are in user-context and meta-sock-lock is hold.
 */
int __mptcp_init6_subsockets(struct sock *meta_sk, const struct mptcp_loc6 *loc,
			     __be16 sport, struct mptcp_rem6 *rem,
			     struct sock **subsk)
{
	struct tcp_sock *tp;
	struct sock *sk;
	struct sockaddr_in6 loc_in, rem_in;
	struct socket_alloc sock_full;
	struct socket *sock = (struct socket *)&sock_full;
	int ret;

	/** First, create and prepare the new socket */
	memcpy(&sock_full, meta_sk->sk_socket, sizeof(sock_full));
	sock->state = SS_UNCONNECTED;
	sock->ops = NULL;

	ret = inet6_create(sock_net(meta_sk), sock, IPPROTO_TCP, 1);
	if (unlikely(ret < 0)) {
		net_err_ratelimited("%s inet6_create failed ret: %d\n",
				    __func__, ret);
		return ret;
	}

	sk = sock->sk;
	tp = tcp_sk(sk);

	/* All subsockets need the MPTCP-lock-class */
	lockdep_set_class_and_name(&(sk)->sk_lock.slock, &meta_slock_key, meta_slock_key_name);
	lockdep_init_map(&(sk)->sk_lock.dep_map, meta_key_name, &meta_key, 0);

	ret = mptcp_add_sock(meta_sk, sk, loc->loc6_id, rem->rem6_id, GFP_KERNEL);
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
	loc_in.sin6_family = AF_INET6;
	rem_in.sin6_family = AF_INET6;
	loc_in.sin6_port = sport;
	if (rem->port)
		rem_in.sin6_port = rem->port;
	else
		rem_in.sin6_port = inet_sk(meta_sk)->inet_dport;
	loc_in.sin6_addr = loc->addr;
	rem_in.sin6_addr = rem->addr;

	if (loc->if_idx)
		sk->sk_bound_dev_if = loc->if_idx;

	ret = kernel_bind(sock, (struct sockaddr *)&loc_in,
			  sizeof(struct sockaddr_in6));
	if (ret < 0) {
		net_err_ratelimited("%s: token %#x bind() to %pI6 index %d failed, error %d\n",
				    __func__, tcp_sk(meta_sk)->mpcb->mptcp_loc_token,
				    &loc_in.sin6_addr, loc->if_idx, ret);
		goto error;
	}

	mptcp_debug("%s: token %#x pi %d src_addr:%pI6:%d dst_addr:%pI6:%d ifidx: %u\n",
		    __func__, tcp_sk(meta_sk)->mpcb->mptcp_loc_token,
		    tp->mptcp->path_index, &loc_in.sin6_addr,
		    ntohs(loc_in.sin6_port), &rem_in.sin6_addr,
		    ntohs(rem_in.sin6_port), loc->if_idx);

	if (tcp_sk(meta_sk)->mpcb->pm_ops->init_subsocket_v6)
		tcp_sk(meta_sk)->mpcb->pm_ops->init_subsocket_v6(sk, rem->addr);

	ret = kernel_connect(sock, (struct sockaddr *)&rem_in,
			     sizeof(struct sockaddr_in6), O_NONBLOCK);
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
EXPORT_SYMBOL(__mptcp_init6_subsockets);

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

	ops->slab_name = kasprintf(GFP_KERNEL, "request_sock_%s", "MPTCP6");
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

void mptcp_pm_v6_undo(void)
{
	kmem_cache_destroy(mptcp6_request_sock_ops.slab);
	kfree(mptcp6_request_sock_ops.slab_name);
}
#endif
