/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		RAW - implementation of IP "raw" sockets.
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 * Fixes:
 *		Alan Cox	:	verify_area() fixed up
 *		Alan Cox	:	ICMP error handling
 *		Alan Cox	:	EMSGSIZE if you send too big a packet
 *		Alan Cox	: 	Now uses generic datagrams and shared
 *					skbuff library. No more peek crashes,
 *					no more backlogs
 *		Alan Cox	:	Checks sk->broadcast.
 *		Alan Cox	:	Uses skb_free_datagram/skb_copy_datagram
 *		Alan Cox	:	Raw passes ip options too
 *		Alan Cox	:	Setsocketopt added
 *		Alan Cox	:	Fixed error return for broadcasts
 *		Alan Cox	:	Removed wake_up calls
 *		Alan Cox	:	Use ttl/tos
 *		Alan Cox	:	Cleaned up old debugging
 *		Alan Cox	:	Use new kernel side addresses
 *	Arnt Gulbrandsen	:	Fixed MSG_DONTROUTE in raw sockets.
 *		Alan Cox	:	BSD style RAW socket demultiplexing.
 *		Alan Cox	:	Beginnings of mrouted support.
 *		Alan Cox	:	Added IP_HDRINCL option.
 *		Alan Cox	:	Skip broadcast check if BSDism set.
 *		David S. Miller	:	New socket lookup architecture.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include <linux/types.h>
#include <linux/atomic.h>
#include <asm/byteorder.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/ioctls.h>
#include <linux/stddef.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <linux/sockios.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/mroute.h>
#include <linux/netdevice.h>
#include <linux/in_route.h>
#include <linux/route.h>
#include <linux/skbuff.h>
#include <linux/igmp.h>
#include <net/net_namespace.h>
#include <net/dst.h>
#include <net/sock.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/raw.h>
#include <net/snmp.h>
#include <net/tcp_states.h>
#include <net/inet_common.h>
#include <net/checksum.h>
#include <net/xfrm.h>
#include <linux/rtnetlink.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/compat.h>
#include <linux/uio.h>

struct raw_frag_vec {
	struct msghdr *msg;
	union {
		struct icmphdr icmph;
		char c[1];
	} hdr;
	int hlen;
};

static struct raw_hashinfo raw_v4_hashinfo = {
	.lock = __RW_LOCK_UNLOCKED(raw_v4_hashinfo.lock),
};

void raw_hash_sk(struct sock *sk)
{
	struct raw_hashinfo *h = sk->sk_prot->h.raw_hash;
	struct hlist_head *head;

	head = &h->ht[inet_sk(sk)->inet_num & (RAW_HTABLE_SIZE - 1)];

	write_lock_bh(&h->lock);
	sk_add_node(sk, head);
	sock_prot_inuse_add(sock_net(sk), sk->sk_prot, 1);
	write_unlock_bh(&h->lock);
}
EXPORT_SYMBOL_GPL(raw_hash_sk);

void raw_unhash_sk(struct sock *sk)
{
	struct raw_hashinfo *h = sk->sk_prot->h.raw_hash;

	write_lock_bh(&h->lock);
	if (sk_del_node_init(sk))
		sock_prot_inuse_add(sock_net(sk), sk->sk_prot, -1);
	write_unlock_bh(&h->lock);
}
EXPORT_SYMBOL_GPL(raw_unhash_sk);

static struct sock *__raw_v4_lookup(struct net *net, struct sock *sk,
		unsigned short num, __be32 raddr, __be32 laddr, int dif)
{
	sk_for_each_from(sk) {
		struct inet_sock *inet = inet_sk(sk);

		if (net_eq(sock_net(sk), net) && inet->inet_num == num	&&
		    !(inet->inet_daddr && inet->inet_daddr != raddr) 	&&
		    !(inet->inet_rcv_saddr && inet->inet_rcv_saddr != laddr) &&
		    !(sk->sk_bound_dev_if && sk->sk_bound_dev_if != dif))
			goto found; /* gotcha */
	}
	sk = NULL;
found:
	return sk;
}

/*
 *	0 - deliver
 *	1 - block
 */
static int icmp_filter(const struct sock *sk, const struct sk_buff *skb)
{
	struct icmphdr _hdr;
	const struct icmphdr *hdr;

	hdr = skb_header_pointer(skb, skb_transport_offset(skb),
				 sizeof(_hdr), &_hdr);
	if (!hdr)
		return 1;

	if (hdr->type < 32) {
		__u32 data = raw_sk(sk)->filter.data;

		return ((1U << hdr->type) & data) != 0;
	}

	/* Do not block unknown ICMP types */
	return 0;
}

/* IP input processing comes here for RAW socket delivery.
 * Caller owns SKB, so we must make clones.
 *
 * RFC 1122: SHOULD pass TOS value up to the transport layer.
 * -> It does. And not only TOS, but all IP header.
 */
static int raw_v4_input(struct sk_buff *skb, const struct iphdr *iph, int hash)
{
	struct sock *sk;
	struct hlist_head *head;
	int delivered = 0;
	struct net *net;

	read_lock(&raw_v4_hashinfo.lock);
	head = &raw_v4_hashinfo.ht[hash];
	if (hlist_empty(head))
		goto out;

	net = dev_net(skb->dev);
	sk = __raw_v4_lookup(net, __sk_head(head), iph->protocol,
			     iph->saddr, iph->daddr,
			     skb->dev->ifindex);

	while (sk) {
		delivered = 1;
		if ((iph->protocol != IPPROTO_ICMP || !icmp_filter(sk, skb)) &&
		    ip_mc_sf_allow(sk, iph->daddr, iph->saddr,
				   skb->dev->ifindex)) {
			struct sk_buff *clone = skb_clone(skb, GFP_ATOMIC);

			/* Not releasing hash table! */
			if (clone)
				raw_rcv(sk, clone);
		}
		sk = __raw_v4_lookup(net, sk_next(sk), iph->protocol,
				     iph->saddr, iph->daddr,
				     skb->dev->ifindex);
	}
out:
	read_unlock(&raw_v4_hashinfo.lock);
	return delivered;
}

int raw_local_deliver(struct sk_buff *skb, int protocol)
{
	int hash;
	struct sock *raw_sk;

	hash = protocol & (RAW_HTABLE_SIZE - 1);
	raw_sk = sk_head(&raw_v4_hashinfo.ht[hash]);

	/* If there maybe a raw socket we must check - if not we
	 * don't care less
	 */
	if (raw_sk && !raw_v4_input(skb, ip_hdr(skb), hash))
		raw_sk = NULL;

	return raw_sk != NULL;

}

static void raw_err(struct sock *sk, struct sk_buff *skb, u32 info)
{
	struct inet_sock *inet = inet_sk(sk);
	const int type = icmp_hdr(skb)->type;
	const int code = icmp_hdr(skb)->code;
	int err = 0;
	int harderr = 0;

	if (type == ICMP_DEST_UNREACH && code == ICMP_FRAG_NEEDED)
		ipv4_sk_update_pmtu(skb, sk, info);
	else if (type == ICMP_REDIRECT) {
		ipv4_sk_redirect(skb, sk);
		return;
	}

	/* Report error on raw socket, if:
	   1. User requested ip_recverr.
	   2. Socket is connected (otherwise the error indication
	      is useless without ip_recverr and error is hard.
	 */
	if (!inet->recverr && sk->sk_state != TCP_ESTABLISHED)
		return;

	switch (type) {
	default:
	case ICMP_TIME_EXCEEDED:
		err = EHOSTUNREACH;
		break;
	case ICMP_SOURCE_QUENCH:
		return;
	case ICMP_PARAMETERPROB:
		err = EPROTO;
		harderr = 1;
		break;
	case ICMP_DEST_UNREACH:
		err = EHOSTUNREACH;
		if (code > NR_ICMP_UNREACH)
			break;
		err = icmp_err_convert[code].errno;
		harderr = icmp_err_convert[code].fatal;
		if (code == ICMP_FRAG_NEEDED) {
			harderr = inet->pmtudisc != IP_PMTUDISC_DONT;
			err = EMSGSIZE;
		}
	}

	if (inet->recverr) {
		const struct iphdr *iph = (const struct iphdr *)skb->data;
		u8 *payload = skb->data + (iph->ihl << 2);

		if (inet->hdrincl)
			payload = skb->data;
		ip_icmp_error(sk, skb, err, 0, info, payload);
	}

	if (inet->recverr || harderr) {
		sk->sk_err = err;
		sk->sk_error_report(sk);
	}
}

void raw_icmp_error(struct sk_buff *skb, int protocol, u32 info)
{
	int hash;
	struct sock *raw_sk;
	const struct iphdr *iph;
	struct net *net;

	hash = protocol & (RAW_HTABLE_SIZE - 1);

	read_lock(&raw_v4_hashinfo.lock);
	raw_sk = sk_head(&raw_v4_hashinfo.ht[hash]);
	if (raw_sk) {
		iph = (const struct iphdr *)skb->data;
		net = dev_net(skb->dev);

		while ((raw_sk = __raw_v4_lookup(net, raw_sk, protocol,
						iph->daddr, iph->saddr,
						skb->dev->ifindex)) != NULL) {
			raw_err(raw_sk, skb, info);
			raw_sk = sk_next(raw_sk);
			iph = (const struct iphdr *)skb->data;
		}
	}
	read_unlock(&raw_v4_hashinfo.lock);
}

static int raw_rcv_skb(struct sock *sk, struct sk_buff *skb)
{
	/* Charge it to the socket. */

	ipv4_pktinfo_prepare(sk, skb);
	if (sock_queue_rcv_skb(sk, skb) < 0) {
		kfree_skb(skb);
		return NET_RX_DROP;
	}

	return NET_RX_SUCCESS;
}

int raw_rcv(struct sock *sk, struct sk_buff *skb)
{
	if (!xfrm4_policy_check(sk, XFRM_POLICY_IN, skb)) {
		atomic_inc(&sk->sk_drops);
		kfree_skb(skb);
		return NET_RX_DROP;
	}
	nf_reset(skb);

	skb_push(skb, skb->data - skb_network_header(skb));

	raw_rcv_skb(sk, skb);
	return 0;
}

static int raw_send_hdrinc(struct sock *sk, struct flowi4 *fl4,
			   struct msghdr *msg, size_t length,
			   struct rtable **rtp,
			   unsigned int flags)
{
	struct inet_sock *inet = inet_sk(sk);
	struct net *net = sock_net(sk);
	struct iphdr *iph;
	struct sk_buff *skb;
	unsigned int iphlen;
	int err;
	struct rtable *rt = *rtp;
	int hlen, tlen;

	if (length > rt->dst.dev->mtu) {
		ip_local_error(sk, EMSGSIZE, fl4->daddr, inet->inet_dport,
			       rt->dst.dev->mtu);
		return -EMSGSIZE;
	}
	if (length < sizeof(struct iphdr))
		return -EINVAL;

	if (flags&MSG_PROBE)
		goto out;

	hlen = LL_RESERVED_SPACE(rt->dst.dev);
	tlen = rt->dst.dev->needed_tailroom;
	skb = sock_alloc_send_skb(sk,
				  length + hlen + tlen + 15,
				  flags & MSG_DONTWAIT, &err);
	if (!skb)
		goto error;
	skb_reserve(skb, hlen);

	skb->priority = sk->sk_priority;
	skb->mark = sk->sk_mark;
	skb_dst_set(skb, &rt->dst);
	*rtp = NULL;

	skb_reset_network_header(skb);
	iph = ip_hdr(skb);
	skb_put(skb, length);

	skb->ip_summed = CHECKSUM_NONE;

	sock_tx_timestamp(sk, &skb_shinfo(skb)->tx_flags);

	skb->transport_header = skb->network_header;
	err = -EFAULT;
	if (memcpy_from_msg(iph, msg, length))
		goto error_free;

	iphlen = iph->ihl * 4;

	/*
	 * We don't want to modify the ip header, but we do need to
	 * be sure that it won't cause problems later along the network
	 * stack.  Specifically we want to make sure that iph->ihl is a
	 * sane value.  If ihl points beyond the length of the buffer passed
	 * in, reject the frame as invalid
	 */
	err = -EINVAL;
	if (iphlen > length)
		goto error_free;

	if (iphlen >= sizeof(*iph)) {
		if (!iph->saddr)
			iph->saddr = fl4->saddr;
		iph->check   = 0;
		iph->tot_len = htons(length);
		if (!iph->id)
			ip_select_ident(net, skb, NULL);

		iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
	}
	if (iph->protocol == IPPROTO_ICMP)
		icmp_out_count(net, ((struct icmphdr *)
			skb_transport_header(skb))->type);

	err = NF_HOOK(NFPROTO_IPV4, NF_INET_LOCAL_OUT, sk, skb,
		      NULL, rt->dst.dev, dst_output_sk);
	if (err > 0)
		err = net_xmit_errno(err);
	if (err)
		goto error;
out:
	return 0;

error_free:
	kfree_skb(skb);
error:
	IP_INC_STATS(net, IPSTATS_MIB_OUTDISCARDS);
	if (err == -ENOBUFS && !inet->recverr)
		err = 0;
	return err;
}

static int raw_probe_proto_opt(struct raw_frag_vec *rfv, struct flowi4 *fl4)
{
	int err;

	if (fl4->flowi4_proto != IPPROTO_ICMP)
		return 0;

	/* We only need the first two bytes. */
	rfv->hlen = 2;

	err = memcpy_from_msg(rfv->hdr.c, rfv->msg, rfv->hlen);
	if (err)
		return err;

	fl4->fl4_icmp_type = rfv->hdr.icmph.type;
	fl4->fl4_icmp_code = rfv->hdr.icmph.code;

	return 0;
}

static int raw_getfrag(void *from, char *to, int offset, int len, int odd,
		       struct sk_buff *skb)
{
	struct raw_frag_vec *rfv = from;

	if (offset < rfv->hlen) {
		int copy = min(rfv->hlen - offset, len);

		if (skb->ip_summed == CHECKSUM_PARTIAL)
			memcpy(to, rfv->hdr.c + offset, copy);
		else
			skb->csum = csum_block_add(
				skb->csum,
				csum_partial_copy_nocheck(rfv->hdr.c + offset,
							  to, copy, 0),
				odd);

		odd = 0;
		offset += copy;
		to += copy;
		len -= copy;

		if (!len)
			return 0;
	}

	offset -= rfv->hlen;

	return ip_generic_getfrag(rfv->msg, to, offset, len, odd, skb);
}

static int raw_sendmsg(struct sock *sk, struct msghdr *msg, size_t len)
{
	struct inet_sock *inet = inet_sk(sk);
	struct ipcm_cookie ipc;
	struct rtable *rt = NULL;
	struct flowi4 fl4;
	int free = 0;
	__be32 daddr;
	__be32 saddr;
	u8  tos;
	int err;
	struct ip_options_data opt_copy;
	struct raw_frag_vec rfv;
	int hdrincl;

	err = -EMSGSIZE;
	if (len > 0xFFFF)
		goto out;

	/* hdrincl should be READ_ONCE(inet->hdrincl)
	 * but READ_ONCE() doesn't work with bit fields
	 */
	hdrincl = inet->hdrincl;
	/*
	 *	Check the flags.
	 */

	err = -EOPNOTSUPP;
	if (msg->msg_flags & MSG_OOB)	/* Mirror BSD error message */
		goto out;               /* compatibility */

	/*
	 *	Get and verify the address.
	 */

	if (msg->msg_namelen) {
		DECLARE_SOCKADDR(struct sockaddr_in *, usin, msg->msg_name);
		err = -EINVAL;
		if (msg->msg_namelen < sizeof(*usin))
			goto out;
		if (usin->sin_family != AF_INET) {
			pr_info_once("%s: %s forgot to set AF_INET. Fix it!\n",
				     __func__, current->comm);
			err = -EAFNOSUPPORT;
			if (usin->sin_family)
				goto out;
		}
		daddr = usin->sin_addr.s_addr;
		/* ANK: I did not forget to get protocol from port field.
		 * I just do not know, who uses this weirdness.
		 * IP_HDRINCL is much more convenient.
		 */
	} else {
		err = -EDESTADDRREQ;
		if (sk->sk_state != TCP_ESTABLISHED)
			goto out;
		daddr = inet->inet_daddr;
	}

	ipc.addr = inet->inet_saddr;
	ipc.opt = NULL;
	ipc.tx_flags = 0;
	ipc.ttl = 0;
	ipc.tos = -1;
	ipc.oif = sk->sk_bound_dev_if;

	if (msg->msg_controllen) {
		err = ip_cmsg_send(sock_net(sk), msg, &ipc, false);
		if (unlikely(err)) {
			kfree(ipc.opt);
			goto out;
		}
		if (ipc.opt)
			free = 1;
	}

	saddr = ipc.addr;
	ipc.addr = daddr;

	if (!ipc.opt) {
		struct ip_options_rcu *inet_opt;

		rcu_read_lock();
		inet_opt = rcu_dereference(inet->inet_opt);
		if (inet_opt) {
			memcpy(&opt_copy, inet_opt,
			       sizeof(*inet_opt) + inet_opt->opt.optlen);
			ipc.opt = &opt_copy.opt;
		}
		rcu_read_unlock();
	}

	if (ipc.opt) {
		err = -EINVAL;
		/* Linux does not mangle headers on raw sockets,
		 * so that IP options + IP_HDRINCL is non-sense.
		 */
		if (hdrincl)
			goto done;
		if (ipc.opt->opt.srr) {
			if (!daddr)
				goto done;
			daddr = ipc.opt->opt.faddr;
		}
	}
	tos = get_rtconn_flags(&ipc, sk);
	if (msg->msg_flags & MSG_DONTROUTE)
		tos |= RTO_ONLINK;

	if (ipv4_is_multicast(daddr)) {
		if (!ipc.oif)
			ipc.oif = inet->mc_index;
		if (!saddr)
			saddr = inet->mc_addr;
	} else if (!ipc.oif)
		ipc.oif = inet->uc_index;

	flowi4_init_output(&fl4, ipc.oif, sk->sk_mark, tos,
			   RT_SCOPE_UNIVERSE,
			   hdrincl ? IPPROTO_RAW : sk->sk_protocol,
			   inet_sk_flowi_flags(sk) |
			    (hdrincl ? FLOWI_FLAG_KNOWN_NH : 0),
			   daddr, saddr, 0, 0);

	if (!hdrincl) {
		rfv.msg = msg;
		rfv.hlen = 0;

		err = raw_probe_proto_opt(&rfv, &fl4);
		if (err)
			goto done;
	}

	security_sk_classify_flow(sk, flowi4_to_flowi(&fl4));
	rt = ip_route_output_flow(sock_net(sk), &fl4, sk);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		rt = NULL;
		goto done;
	}

	err = -EACCES;
	if (rt->rt_flags & RTCF_BROADCAST && !sock_flag(sk, SOCK_BROADCAST))
		goto done;

	if (msg->msg_flags & MSG_CONFIRM)
		goto do_confirm;
back_from_confirm:

	if (hdrincl)
		err = raw_send_hdrinc(sk, &fl4, msg, len,
				      &rt, msg->msg_flags);

	 else {
		sock_tx_timestamp(sk, &ipc.tx_flags);

		if (!ipc.addr)
			ipc.addr = fl4.daddr;
		lock_sock(sk);
		err = ip_append_data(sk, &fl4, raw_getfrag,
				     &rfv, len, 0,
				     &ipc, &rt, msg->msg_flags);
		if (err)
			ip_flush_pending_frames(sk);
		else if (!(msg->msg_flags & MSG_MORE)) {
			err = ip_push_pending_frames(sk, &fl4);
			if (err == -ENOBUFS && !inet->recverr)
				err = 0;
		}
		release_sock(sk);
	}
done:
	if (free)
		kfree(ipc.opt);
	ip_rt_put(rt);

out:
	if (err < 0)
		return err;
	return len;

do_confirm:
	dst_confirm(&rt->dst);
	if (!(msg->msg_flags & MSG_PROBE) || len)
		goto back_from_confirm;
	err = 0;
	goto done;
}

static void raw_close(struct sock *sk, long timeout)
{
	/*
	 * Raw sockets may have direct kernel references. Kill them.
	 */
	ip_ra_control(sk, 0, NULL);

	sk_common_release(sk);
}

static void raw_destroy(struct sock *sk)
{
	lock_sock(sk);
	ip_flush_pending_frames(sk);
	release_sock(sk);
}

/* This gets rid of all the nasties in af_inet. -DaveM */
static int raw_bind(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct inet_sock *inet = inet_sk(sk);
	struct sockaddr_in *addr = (struct sockaddr_in *) uaddr;
	int ret = -EINVAL;
	int chk_addr_ret;

	if (sk->sk_state != TCP_CLOSE || addr_len < sizeof(struct sockaddr_in))
		goto out;
	chk_addr_ret = inet_addr_type(sock_net(sk), addr->sin_addr.s_addr);
	ret = -EADDRNOTAVAIL;
	if (addr->sin_addr.s_addr && chk_addr_ret != RTN_LOCAL &&
	    chk_addr_ret != RTN_MULTICAST && chk_addr_ret != RTN_BROADCAST)
		goto out;
	inet->inet_rcv_saddr = inet->inet_saddr = addr->sin_addr.s_addr;
	if (chk_addr_ret == RTN_MULTICAST || chk_addr_ret == RTN_BROADCAST)
		inet->inet_saddr = 0;  /* Use device */
	sk_dst_reset(sk);
	ret = 0;
out:	return ret;
}

/*
 *	This should be easy, if there is something there
 *	we return it, otherwise we block.
 */

static int raw_recvmsg(struct sock *sk, struct msghdr *msg, size_t len,
		       int noblock, int flags, int *addr_len)
{
	struct inet_sock *inet = inet_sk(sk);
	size_t copied = 0;
	int err = -EOPNOTSUPP;
	DECLARE_SOCKADDR(struct sockaddr_in *, sin, msg->msg_name);
	struct sk_buff *skb;

	if (flags & MSG_OOB)
		goto out;

	if (flags & MSG_ERRQUEUE) {
		err = ip_recv_error(sk, msg, len, addr_len);
		goto out;
	}

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb)
		goto out;

	copied = skb->len;
	if (len < copied) {
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}

	err = skb_copy_datagram_msg(skb, 0, msg, copied);
	if (err)
		goto done;

	sock_recv_ts_and_drops(msg, sk, skb);

	/* Copy the address. */
	if (sin) {
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = ip_hdr(skb)->saddr;
		sin->sin_port = 0;
		memset(&sin->sin_zero, 0, sizeof(sin->sin_zero));
		*addr_len = sizeof(*sin);
	}
	if (inet->cmsg_flags)
		ip_cmsg_recv(msg, skb);
	if (flags & MSG_TRUNC)
		copied = skb->len;
done:
	skb_free_datagram(sk, skb);
out:
	if (err)
		return err;
	return copied;
}

static int raw_init(struct sock *sk)
{
	struct raw_sock *rp = raw_sk(sk);

	if (inet_sk(sk)->inet_num == IPPROTO_ICMP)
		memset(&rp->filter, 0, sizeof(rp->filter));
	return 0;
}

static int raw_seticmpfilter(struct sock *sk, char __user *optval, int optlen)
{
	if (optlen > sizeof(struct icmp_filter))
		optlen = sizeof(struct icmp_filter);
	if (copy_from_user(&raw_sk(sk)->filter, optval, optlen))
		return -EFAULT;
	return 0;
}

static int raw_geticmpfilter(struct sock *sk, char __user *optval, int __user *optlen)
{
	int len, ret = -EFAULT;

	if (get_user(len, optlen))
		goto out;
	ret = -EINVAL;
	if (len < 0)
		goto out;
	if (len > sizeof(struct icmp_filter))
		len = sizeof(struct icmp_filter);
	ret = -EFAULT;
	if (put_user(len, optlen) ||
	    copy_to_user(optval, &raw_sk(sk)->filter, len))
		goto out;
	ret = 0;
out:	return ret;
}

static int do_raw_setsockopt(struct sock *sk, int level, int optname,
			  char __user *optval, unsigned int optlen)
{
	if (optname == ICMP_FILTER) {
		if (inet_sk(sk)->inet_num != IPPROTO_ICMP)
			return -EOPNOTSUPP;
		else
			return raw_seticmpfilter(sk, optval, optlen);
	}
	return -ENOPROTOOPT;
}

static int raw_setsockopt(struct sock *sk, int level, int optname,
			  char __user *optval, unsigned int optlen)
{
	if (level != SOL_RAW)
		return ip_setsockopt(sk, level, optname, optval, optlen);
	return do_raw_setsockopt(sk, level, optname, optval, optlen);
}

#ifdef CONFIG_COMPAT
static int compat_raw_setsockopt(struct sock *sk, int level, int optname,
				 char __user *optval, unsigned int optlen)
{
	if (level != SOL_RAW)
		return compat_ip_setsockopt(sk, level, optname, optval, optlen);
	return do_raw_setsockopt(sk, level, optname, optval, optlen);
}
#endif

static int do_raw_getsockopt(struct sock *sk, int level, int optname,
			  char __user *optval, int __user *optlen)
{
	if (optname == ICMP_FILTER) {
		if (inet_sk(sk)->inet_num != IPPROTO_ICMP)
			return -EOPNOTSUPP;
		else
			return raw_geticmpfilter(sk, optval, optlen);
	}
	return -ENOPROTOOPT;
}

static int raw_getsockopt(struct sock *sk, int level, int optname,
			  char __user *optval, int __user *optlen)
{
	if (level != SOL_RAW)
		return ip_getsockopt(sk, level, optname, optval, optlen);
	return do_raw_getsockopt(sk, level, optname, optval, optlen);
}

#ifdef CONFIG_COMPAT
static int compat_raw_getsockopt(struct sock *sk, int level, int optname,
				 char __user *optval, int __user *optlen)
{
	if (level != SOL_RAW)
		return compat_ip_getsockopt(sk, level, optname, optval, optlen);
	return do_raw_getsockopt(sk, level, optname, optval, optlen);
}
#endif

static int raw_ioctl(struct sock *sk, int cmd, unsigned long arg)
{
	switch (cmd) {
	case SIOCOUTQ: {
		int amount = sk_wmem_alloc_get(sk);

		return put_user(amount, (int __user *)arg);
	}
	case SIOCINQ: {
		struct sk_buff *skb;
		int amount = 0;

		spin_lock_bh(&sk->sk_receive_queue.lock);
		skb = skb_peek(&sk->sk_receive_queue);
		if (skb)
			amount = skb->len;
		spin_unlock_bh(&sk->sk_receive_queue.lock);
		return put_user(amount, (int __user *)arg);
	}

	default:
#ifdef CONFIG_IP_MROUTE
		return ipmr_ioctl(sk, cmd, (void __user *)arg);
#else
		return -ENOIOCTLCMD;
#endif
	}
}

#ifdef CONFIG_COMPAT
static int compat_raw_ioctl(struct sock *sk, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case SIOCOUTQ:
	case SIOCINQ:
		return -ENOIOCTLCMD;
	default:
#ifdef CONFIG_IP_MROUTE
		return ipmr_compat_ioctl(sk, cmd, compat_ptr(arg));
#else
		return -ENOIOCTLCMD;
#endif
	}
}
#endif

struct proto raw_prot = {
	.name		   = "RAW",
	.owner		   = THIS_MODULE,
	.close		   = raw_close,
	.destroy	   = raw_destroy,
	.connect	   = ip4_datagram_connect,
	.disconnect	   = udp_disconnect,
	.ioctl		   = raw_ioctl,
	.init		   = raw_init,
	.setsockopt	   = raw_setsockopt,
	.getsockopt	   = raw_getsockopt,
	.sendmsg	   = raw_sendmsg,
	.recvmsg	   = raw_recvmsg,
	.bind		   = raw_bind,
	.backlog_rcv	   = raw_rcv_skb,
	.release_cb	   = ip4_datagram_release_cb,
	.hash		   = raw_hash_sk,
	.unhash		   = raw_unhash_sk,
	.obj_size	   = sizeof(struct raw_sock),
	.h.raw_hash	   = &raw_v4_hashinfo,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_raw_setsockopt,
	.compat_getsockopt = compat_raw_getsockopt,
	.compat_ioctl	   = compat_raw_ioctl,
#endif
};

#ifdef CONFIG_PROC_FS
static struct sock *raw_get_first(struct seq_file *seq)
{
	struct sock *sk;
	struct raw_iter_state *state = raw_seq_private(seq);

	for (state->bucket = 0; state->bucket < RAW_HTABLE_SIZE;
			++state->bucket) {
		sk_for_each(sk, &state->h->ht[state->bucket])
			if (sock_net(sk) == seq_file_net(seq))
				goto found;
	}
	sk = NULL;
found:
	return sk;
}

static struct sock *raw_get_next(struct seq_file *seq, struct sock *sk)
{
	struct raw_iter_state *state = raw_seq_private(seq);

	do {
		sk = sk_next(sk);
try_again:
		;
	} while (sk && sock_net(sk) != seq_file_net(seq));

	if (!sk && ++state->bucket < RAW_HTABLE_SIZE) {
		sk = sk_head(&state->h->ht[state->bucket]);
		goto try_again;
	}
	return sk;
}

static struct sock *raw_get_idx(struct seq_file *seq, loff_t pos)
{
	struct sock *sk = raw_get_first(seq);

	if (sk)
		while (pos && (sk = raw_get_next(seq, sk)) != NULL)
			--pos;
	return pos ? NULL : sk;
}

void *raw_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct raw_iter_state *state = raw_seq_private(seq);

	read_lock(&state->h->lock);
	return *pos ? raw_get_idx(seq, *pos - 1) : SEQ_START_TOKEN;
}
EXPORT_SYMBOL_GPL(raw_seq_start);

void *raw_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct sock *sk;

	if (v == SEQ_START_TOKEN)
		sk = raw_get_first(seq);
	else
		sk = raw_get_next(seq, v);
	++*pos;
	return sk;
}
EXPORT_SYMBOL_GPL(raw_seq_next);

void raw_seq_stop(struct seq_file *seq, void *v)
{
	struct raw_iter_state *state = raw_seq_private(seq);

	read_unlock(&state->h->lock);
}
EXPORT_SYMBOL_GPL(raw_seq_stop);

static void raw_sock_seq_show(struct seq_file *seq, struct sock *sp, int i)
{
	struct inet_sock *inet = inet_sk(sp);
	__be32 dest = inet->inet_daddr,
	       src = inet->inet_rcv_saddr;
	__u16 destp = 0,
	      srcp  = inet->inet_num;

	seq_printf(seq, "%4d: %08X:%04X %08X:%04X"
		" %02X %08X:%08X %02X:%08lX %08X %5u %8d %lu %d %pK %d\n",
		i, src, srcp, dest, destp, sp->sk_state,
		sk_wmem_alloc_get(sp),
		sk_rmem_alloc_get(sp),
		0, 0L, 0,
		from_kuid_munged(seq_user_ns(seq), sock_i_uid(sp)),
		0, sock_i_ino(sp),
		atomic_read(&sp->sk_refcnt), sp, atomic_read(&sp->sk_drops));
}

static int raw_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN)
		seq_printf(seq, "  sl  local_address rem_address   st tx_queue "
				"rx_queue tr tm->when retrnsmt   uid  timeout "
				"inode ref pointer drops\n");
	else
		raw_sock_seq_show(seq, v, raw_seq_private(seq)->bucket);
	return 0;
}

static const struct seq_operations raw_seq_ops = {
	.start = raw_seq_start,
	.next  = raw_seq_next,
	.stop  = raw_seq_stop,
	.show  = raw_seq_show,
};

int raw_seq_open(struct inode *ino, struct file *file,
		 struct raw_hashinfo *h, const struct seq_operations *ops)
{
	int err;
	struct raw_iter_state *i;

	err = seq_open_net(ino, file, ops, sizeof(struct raw_iter_state));
	if (err < 0)
		return err;

	i = raw_seq_private((struct seq_file *)file->private_data);
	i->h = h;
	return 0;
}
EXPORT_SYMBOL_GPL(raw_seq_open);

static int raw_v4_seq_open(struct inode *inode, struct file *file)
{
	return raw_seq_open(inode, file, &raw_v4_hashinfo, &raw_seq_ops);
}

static const struct file_operations raw_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = raw_v4_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release_net,
};

static __net_init int raw_init_net(struct net *net)
{
	if (!proc_create("raw", S_IRUGO, net->proc_net, &raw_seq_fops))
		return -ENOMEM;

	return 0;
}

static __net_exit void raw_exit_net(struct net *net)
{
	remove_proc_entry("raw", net->proc_net);
}

static __net_initdata struct pernet_operations raw_net_ops = {
	.init = raw_init_net,
	.exit = raw_exit_net,
};

int __init raw_proc_init(void)
{
	return register_pernet_subsys(&raw_net_ops);
}

void __init raw_proc_exit(void)
{
	unregister_pernet_subsys(&raw_net_ops);
}
#endif /* CONFIG_PROC_FS */
