// SPDX-License-Identifier: GPL-2.0
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/tcp.h>
#include <linux/rcupdate.h>
#include <linux/rculist.h>
#include <net/inetpeer.h>
#include <net/tcp.h>
#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
#include <net/mptcp.h>
#endif

void tcp_fastopen_init_key_once(struct net *net)
{
	u8 key[TCP_FASTOPEN_KEY_LENGTH];
	struct tcp_fastopen_context *ctxt;

	rcu_read_lock();
	ctxt = rcu_dereference(net->ipv4.tcp_fastopen_ctx);
	if (ctxt) {
		rcu_read_unlock();
		return;
	}
	rcu_read_unlock();

	/* tcp_fastopen_reset_cipher publishes the new context
	 * atomically, so we allow this race happening here.
	 *
	 * All call sites of tcp_fastopen_cookie_gen also check
	 * for a valid cookie, so this is an acceptable risk.
	 */
	get_random_bytes(key, sizeof(key));
	tcp_fastopen_reset_cipher(net, NULL, key, sizeof(key));
}

static void tcp_fastopen_ctx_free(struct rcu_head *head)
{
	struct tcp_fastopen_context *ctx =
	    container_of(head, struct tcp_fastopen_context, rcu);
	crypto_free_cipher(ctx->tfm);
	kfree(ctx);
}

void tcp_fastopen_destroy_cipher(struct sock *sk)
{
	struct tcp_fastopen_context *ctx;

	ctx = rcu_dereference_protected(
			inet_csk(sk)->icsk_accept_queue.fastopenq.ctx, 1);
	if (ctx)
		call_rcu(&ctx->rcu, tcp_fastopen_ctx_free);
}

void tcp_fastopen_ctx_destroy(struct net *net)
{
	struct tcp_fastopen_context *ctxt;

	spin_lock(&net->ipv4.tcp_fastopen_ctx_lock);

	ctxt = rcu_dereference_protected(net->ipv4.tcp_fastopen_ctx,
				lockdep_is_held(&net->ipv4.tcp_fastopen_ctx_lock));
	rcu_assign_pointer(net->ipv4.tcp_fastopen_ctx, NULL);
	spin_unlock(&net->ipv4.tcp_fastopen_ctx_lock);

	if (ctxt)
		call_rcu(&ctxt->rcu, tcp_fastopen_ctx_free);
}

int tcp_fastopen_reset_cipher(struct net *net, struct sock *sk,
			      void *key, unsigned int len)
{
	struct tcp_fastopen_context *ctx, *octx;
	struct fastopen_queue *q;
	int err;

	ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;
	ctx->tfm = crypto_alloc_cipher("aes", 0, 0);

	if (IS_ERR(ctx->tfm)) {
		err = PTR_ERR(ctx->tfm);
error:		kfree(ctx);
		pr_err("TCP: TFO aes cipher alloc error: %d\n", err);
		return err;
	}
	err = crypto_cipher_setkey(ctx->tfm, key, len);
	if (err) {
		pr_err("TCP: TFO cipher key error: %d\n", err);
		crypto_free_cipher(ctx->tfm);
		goto error;
	}
	memcpy(ctx->key, key, len);


	spin_lock(&net->ipv4.tcp_fastopen_ctx_lock);
	if (sk) {
		q = &inet_csk(sk)->icsk_accept_queue.fastopenq;
		octx = rcu_dereference_protected(q->ctx,
			lockdep_is_held(&net->ipv4.tcp_fastopen_ctx_lock));
		rcu_assign_pointer(q->ctx, ctx);
	} else {
		octx = rcu_dereference_protected(net->ipv4.tcp_fastopen_ctx,
			lockdep_is_held(&net->ipv4.tcp_fastopen_ctx_lock));
		rcu_assign_pointer(net->ipv4.tcp_fastopen_ctx, ctx);
	}
	spin_unlock(&net->ipv4.tcp_fastopen_ctx_lock);

	if (octx)
		call_rcu(&octx->rcu, tcp_fastopen_ctx_free);
	return err;
}

static bool __tcp_fastopen_cookie_gen(struct sock *sk, const void *path,
				      struct tcp_fastopen_cookie *foc)
{
	struct tcp_fastopen_context *ctx;
	bool ok = false;

	rcu_read_lock();

	ctx = rcu_dereference(inet_csk(sk)->icsk_accept_queue.fastopenq.ctx);
	if (!ctx)
		ctx = rcu_dereference(sock_net(sk)->ipv4.tcp_fastopen_ctx);

	if (ctx) {
		crypto_cipher_encrypt_one(ctx->tfm, foc->val, path);
		foc->len = TCP_FASTOPEN_COOKIE_SIZE;
		ok = true;
	}
	rcu_read_unlock();
	return ok;
}

/* Generate the fastopen cookie by doing aes128 encryption on both
 * the source and destination addresses. Pad 0s for IPv4 or IPv4-mapped-IPv6
 * addresses. For the longer IPv6 addresses use CBC-MAC.
 *
 * XXX (TFO) - refactor when TCP_FASTOPEN_COOKIE_SIZE != AES_BLOCK_SIZE.
 */
static bool tcp_fastopen_cookie_gen(struct sock *sk,
				    struct request_sock *req,
				    struct sk_buff *syn,
				    struct tcp_fastopen_cookie *foc)
{
	if (req->rsk_ops->family == AF_INET) {
		const struct iphdr *iph = ip_hdr(syn);

		__be32 path[4] = { iph->saddr, iph->daddr, 0, 0 };
		return __tcp_fastopen_cookie_gen(sk, path, foc);
	}

#if IS_ENABLED(CONFIG_IPV6)
	if (req->rsk_ops->family == AF_INET6) {
		const struct ipv6hdr *ip6h = ipv6_hdr(syn);
		struct tcp_fastopen_cookie tmp;

		if (__tcp_fastopen_cookie_gen(sk, &ip6h->saddr, &tmp)) {
			struct in6_addr *buf = &tmp.addr;
			int i;

			for (i = 0; i < 4; i++)
				buf->s6_addr32[i] ^= ip6h->daddr.s6_addr32[i];
			return __tcp_fastopen_cookie_gen(sk, buf, foc);
		}
	}
#endif
	return false;
}


/* If an incoming SYN or SYNACK frame contains a payload and/or FIN,
 * queue this additional data / FIN.
 */
void tcp_fastopen_add_skb(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (TCP_SKB_CB(skb)->end_seq == tp->rcv_nxt)
		return;

	skb = skb_clone(skb, GFP_ATOMIC);
	if (!skb)
		return;

	skb_dst_drop(skb);
	/* segs_in has been initialized to 1 in tcp_create_openreq_child().
	 * Hence, reset segs_in to 0 before calling tcp_segs_in()
	 * to avoid double counting.  Also, tcp_segs_in() expects
	 * skb->len to include the tcp_hdrlen.  Hence, it should
	 * be called before __skb_pull().
	 */
	tp->segs_in = 0;
	tcp_segs_in(tp, skb);
	__skb_pull(skb, tcp_hdrlen(skb));
	sk_forced_mem_schedule(sk, skb->truesize);
	skb_set_owner_r(skb, sk);

	TCP_SKB_CB(skb)->seq++;
	TCP_SKB_CB(skb)->tcp_flags &= ~TCPHDR_SYN;

	tp->rcv_nxt = TCP_SKB_CB(skb)->end_seq;
	__skb_queue_tail(&sk->sk_receive_queue, skb);
	tp->syn_data_acked = 1;

	/* u64_stats_update_begin(&tp->syncp) not needed here,
	 * as we certainly are not changing upper 32bit value (0)
	 */
	tp->bytes_received = skb->len;

	if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN)
		tcp_fin(sk);
}

static struct sock *tcp_fastopen_create_child(struct sock *sk,
					      struct sk_buff *skb,
					      struct request_sock *req)
{
	struct tcp_sock *tp;
	struct request_sock_queue *queue = &inet_csk(sk)->icsk_accept_queue;
#if !defined(CONFIG_BCM_KF_MPTCP) || !defined(CONFIG_BCM_MPTCP)
	struct sock *child;
#else
	struct sock *child, *meta_sk;
#endif
	bool own_req;
#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
	int ret;
#endif

	req->num_retrans = 0;
	req->num_timeout = 0;
	req->sk = NULL;

	child = inet_csk(sk)->icsk_af_ops->syn_recv_sock(sk, skb, req, NULL,
							 NULL, &own_req);
	if (!child)
		return NULL;

	spin_lock(&queue->fastopenq.lock);
	queue->fastopenq.qlen++;
	spin_unlock(&queue->fastopenq.lock);

	/* Initialize the child socket. Have to fix some values to take
	 * into account the child is a Fast Open socket and is created
	 * only out of the bits carried in the SYN packet.
	 */
	tp = tcp_sk(child);

	tp->fastopen_rsk = req;
	tcp_rsk(req)->tfo_listener = true;

	/* RFC1323: The window in SYN & SYN/ACK segments is never
	 * scaled. So correct it appropriately.
	 */
	tp->snd_wnd = ntohs(tcp_hdr(skb)->window);
	tp->max_window = tp->snd_wnd;

	/* Activate the retrans timer so that SYNACK can be retransmitted.
	 * The request socket is not added to the ehash
	 * because it's been added to the accept queue directly.
	 */
	inet_csk_reset_xmit_timer(child, ICSK_TIME_RETRANS,
				  TCP_TIMEOUT_INIT, TCP_RTO_MAX);

	refcount_set(&req->rsk_refcnt, 2);

#if !defined(CONFIG_BCM_KF_MPTCP) || !defined(CONFIG_BCM_MPTCP)
	/* Now finish processing the fastopen child socket. */
	tcp_init_transfer(child, BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB);

#endif
	tp->rcv_nxt = TCP_SKB_CB(skb)->seq + 1;

	tcp_fastopen_add_skb(child, skb);

	tcp_rsk(req)->rcv_nxt = tp->rcv_nxt;
	tp->rcv_wup = tp->rcv_nxt;
#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)

	meta_sk = child;
	ret = mptcp_check_req_fastopen(meta_sk, req);
	if (ret < 0)
		return NULL;

	if (ret == 0) {
		child = tcp_sk(meta_sk)->mpcb->master_sk;
		tp = tcp_sk(child);
	}

	/* Now finish processing the fastopen child socket. */
	tcp_init_transfer(child, BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB);

#endif
	/* tcp_conn_request() is sending the SYNACK,
	 * and queues the child into listener accept queue.
	 */
	return child;
}

static bool tcp_fastopen_queue_check(struct sock *sk)
{
	struct fastopen_queue *fastopenq;
	int max_qlen;

	/* Make sure the listener has enabled fastopen, and we don't
	 * exceed the max # of pending TFO requests allowed before trying
	 * to validating the cookie in order to avoid burning CPU cycles
	 * unnecessarily.
	 *
	 * XXX (TFO) - The implication of checking the max_qlen before
	 * processing a cookie request is that clients can't differentiate
	 * between qlen overflow causing Fast Open to be disabled
	 * temporarily vs a server not supporting Fast Open at all.
	 */
	fastopenq = &inet_csk(sk)->icsk_accept_queue.fastopenq;
	max_qlen = READ_ONCE(fastopenq->max_qlen);
	if (max_qlen == 0)
		return false;

	if (fastopenq->qlen >= max_qlen) {
		struct request_sock *req1;
		spin_lock(&fastopenq->lock);
		req1 = fastopenq->rskq_rst_head;
		if (!req1 || time_after(req1->rsk_timer.expires, jiffies)) {
			__NET_INC_STATS(sock_net(sk),
					LINUX_MIB_TCPFASTOPENLISTENOVERFLOW);
			spin_unlock(&fastopenq->lock);
			return false;
		}
		fastopenq->rskq_rst_head = req1->dl_next;
		fastopenq->qlen--;
		spin_unlock(&fastopenq->lock);
		reqsk_put(req1);
	}
	return true;
}

static bool tcp_fastopen_no_cookie(const struct sock *sk,
				   const struct dst_entry *dst,
				   int flag)
{
	return (READ_ONCE(sock_net(sk)->ipv4.sysctl_tcp_fastopen) & flag) ||
	       tcp_sk(sk)->fastopen_no_cookie ||
	       (dst && dst_metric(dst, RTAX_FASTOPEN_NO_COOKIE));
}

/* Returns true if we should perform Fast Open on the SYN. The cookie (foc)
 * may be updated and return the client in the SYN-ACK later. E.g., Fast Open
 * cookie request (foc->len == 0).
 */
struct sock *tcp_try_fastopen(struct sock *sk, struct sk_buff *skb,
			      struct request_sock *req,
			      struct tcp_fastopen_cookie *foc,
			      const struct dst_entry *dst)
{
	bool syn_data = TCP_SKB_CB(skb)->end_seq != TCP_SKB_CB(skb)->seq + 1;
	int tcp_fastopen = READ_ONCE(sock_net(sk)->ipv4.sysctl_tcp_fastopen);
	struct tcp_fastopen_cookie valid_foc = { .len = -1 };
	struct sock *child;

	if (foc->len == 0) /* Client requests a cookie */
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPFASTOPENCOOKIEREQD);

	if (!((tcp_fastopen & TFO_SERVER_ENABLE) &&
	      (syn_data || foc->len >= 0) &&
	      tcp_fastopen_queue_check(sk))) {
		foc->len = -1;
		return NULL;
	}

	if (tcp_fastopen_no_cookie(sk, dst, TFO_SERVER_COOKIE_NOT_REQD))
		goto fastopen;

	if (foc->len >= 0 &&  /* Client presents or requests a cookie */
	    tcp_fastopen_cookie_gen(sk, req, skb, &valid_foc) &&
	    foc->len == TCP_FASTOPEN_COOKIE_SIZE &&
	    foc->len == valid_foc.len &&
	    !memcmp(foc->val, valid_foc.val, foc->len)) {
		/* Cookie is valid. Create a (full) child socket to accept
		 * the data in SYN before returning a SYN-ACK to ack the
		 * data. If we fail to create the socket, fall back and
		 * ack the ISN only but includes the same cookie.
		 *
		 * Note: Data-less SYN with valid cookie is allowed to send
		 * data in SYN_RECV state.
		 */
fastopen:
		child = tcp_fastopen_create_child(sk, skb, req);
		if (child) {
			foc->len = -1;
			NET_INC_STATS(sock_net(sk),
				      LINUX_MIB_TCPFASTOPENPASSIVE);
			return child;
		}
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPFASTOPENPASSIVEFAIL);
	} else if (foc->len > 0) /* Client presents an invalid cookie */
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPFASTOPENPASSIVEFAIL);

	valid_foc.exp = foc->exp;
	*foc = valid_foc;
	return NULL;
}

bool tcp_fastopen_cookie_check(struct sock *sk, u16 *mss,
			       struct tcp_fastopen_cookie *cookie)
{
	const struct dst_entry *dst;

	tcp_fastopen_cache_get(sk, mss, cookie);

	/* Firewall blackhole issue check */
	if (tcp_fastopen_active_should_disable(sk)) {
		cookie->len = -1;
		return false;
	}

	dst = __sk_dst_get(sk);

	if (tcp_fastopen_no_cookie(sk, dst, TFO_CLIENT_NO_COOKIE)) {
		cookie->len = -1;
		return true;
	}
	return cookie->len > 0;
}

/* This function checks if we want to defer sending SYN until the first
 * write().  We defer under the following conditions:
 * 1. fastopen_connect sockopt is set
 * 2. we have a valid cookie
 * Return value: return true if we want to defer until application writes data
 *               return false if we want to send out SYN immediately
 */
bool tcp_fastopen_defer_connect(struct sock *sk, int *err)
{
	struct tcp_fastopen_cookie cookie = { .len = 0 };
	struct tcp_sock *tp = tcp_sk(sk);
	u16 mss;

	if (tp->fastopen_connect && !tp->fastopen_req) {
		if (tcp_fastopen_cookie_check(sk, &mss, &cookie)) {
			inet_sk(sk)->defer_connect = 1;
			return true;
		}

		/* Alloc fastopen_req in order for FO option to be included
		 * in SYN
		 */
		tp->fastopen_req = kzalloc(sizeof(*tp->fastopen_req),
					   sk->sk_allocation);
		if (tp->fastopen_req)
			tp->fastopen_req->cookie = cookie;
		else
			*err = -ENOBUFS;
	}
	return false;
}
EXPORT_SYMBOL(tcp_fastopen_defer_connect);

/*
 * The following code block is to deal with middle box issues with TFO:
 * Middlebox firewall issues can potentially cause server's data being
 * blackholed after a successful 3WHS using TFO.
 * The proposed solution is to disable active TFO globally under the
 * following circumstances:
 *   1. client side TFO socket receives out of order FIN
 *   2. client side TFO socket receives out of order RST
 *   3. client side TFO socket has timed out three times consecutively during
 *      or after handshake
 * We disable active side TFO globally for 1hr at first. Then if it
 * happens again, we disable it for 2h, then 4h, 8h, ...
 * And we reset the timeout back to 1hr when we see a successful active
 * TFO connection with data exchanges.
 */

/* Disable active TFO and record current jiffies and
 * tfo_active_disable_times
 */
void tcp_fastopen_active_disable(struct sock *sk)
{
	struct net *net = sock_net(sk);

	/* Paired with READ_ONCE() in tcp_fastopen_active_should_disable() */
	WRITE_ONCE(net->ipv4.tfo_active_disable_stamp, jiffies);

	/* Paired with smp_rmb() in tcp_fastopen_active_should_disable().
	 * We want net->ipv4.tfo_active_disable_stamp to be updated first.
	 */
	smp_mb__before_atomic();
	atomic_inc(&net->ipv4.tfo_active_disable_times);

	NET_INC_STATS(net, LINUX_MIB_TCPFASTOPENBLACKHOLE);
}

/* Calculate timeout for tfo active disable
 * Return true if we are still in the active TFO disable period
 * Return false if timeout already expired and we should use active TFO
 */
bool tcp_fastopen_active_should_disable(struct sock *sk)
{
	unsigned int tfo_bh_timeout = sock_net(sk)->ipv4.sysctl_tcp_fastopen_blackhole_timeout;
	int tfo_da_times = atomic_read(&sock_net(sk)->ipv4.tfo_active_disable_times);
	unsigned long timeout;
	int multiplier;

	if (!tfo_da_times)
		return false;

	/* Paired with smp_mb__before_atomic() in tcp_fastopen_active_disable() */
	smp_rmb();

	/* Limit timout to max: 2^6 * initial timeout */
	multiplier = 1 << min(tfo_da_times - 1, 6);

	/* Paired with the WRITE_ONCE() in tcp_fastopen_active_disable(). */
	timeout = READ_ONCE(sock_net(sk)->ipv4.tfo_active_disable_stamp) +
		  multiplier * tfo_bh_timeout * HZ;
	if (time_before(jiffies, timeout))
		return true;

	/* Mark check bit so we can check for successful active TFO
	 * condition and reset tfo_active_disable_times
	 */
	tcp_sk(sk)->syn_fastopen_ch = 1;
	return false;
}

/* Disable active TFO if FIN is the only packet in the ofo queue
 * and no data is received.
 * Also check if we can reset tfo_active_disable_times if data is
 * received successfully on a marked active TFO sockets opened on
 * a non-loopback interface
 */
void tcp_fastopen_active_disable_ofo_check(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct dst_entry *dst;
	struct sk_buff *skb;

	if (!tp->syn_fastopen)
		return;

	if (!tp->data_segs_in) {
		skb = skb_rb_first(&tp->out_of_order_queue);
		if (skb && !skb_rb_next(skb)) {
			if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN) {
				tcp_fastopen_active_disable(sk);
				return;
			}
		}
	} else if (tp->syn_fastopen_ch &&
		   atomic_read(&sock_net(sk)->ipv4.tfo_active_disable_times)) {
		dst = sk_dst_get(sk);
		if (!(dst && dst->dev && (dst->dev->flags & IFF_LOOPBACK)))
			atomic_set(&sock_net(sk)->ipv4.tfo_active_disable_times, 0);
		dst_release(dst);
	}
}

void tcp_fastopen_active_detect_blackhole(struct sock *sk, bool expired)
{
	u32 timeouts = inet_csk(sk)->icsk_retransmits;
	struct tcp_sock *tp = tcp_sk(sk);

	/* Broken middle-boxes may black-hole Fast Open connection during or
	 * even after the handshake. Be extremely conservative and pause
	 * Fast Open globally after hitting the third consecutive timeout or
	 * exceeding the configured timeout limit.
	 */
	if ((tp->syn_fastopen || tp->syn_data || tp->syn_data_acked) &&
	    (timeouts == 2 || (timeouts < 2 && expired))) {
		tcp_fastopen_active_disable(sk);
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPFASTOPENACTIVEFAIL);
	}
}
