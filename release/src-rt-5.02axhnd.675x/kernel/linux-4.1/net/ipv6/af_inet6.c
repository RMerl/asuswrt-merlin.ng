/*
 *	PF_INET6 socket protocol family
 *	Linux INET6 implementation
 *
 *	Authors:
 *	Pedro Roque		<roque@di.fc.ul.pt>
 *
 *	Adapted from linux/net/ipv4/af_inet.c
 *
 *	Fixes:
 *	piggy, Karl Knutson	:	Socket protocol table
 *	Hideaki YOSHIFUJI	:	sin6_scope_id support
 *	Arnaldo Melo		:	check proc_net_create return, cleanups
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#define pr_fmt(fmt) "IPv6: " fmt

#include <linux/module.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/sockios.h>
#include <linux/net.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/slab.h>

#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/icmpv6.h>
#include <linux/netfilter_ipv6.h>

#include <net/ip.h>
#include <net/ipv6.h>
#include <net/udp.h>
#include <net/udplite.h>
#include <net/tcp.h>
#include <net/ping.h>
#include <net/protocol.h>
#include <net/inet_common.h>
#include <net/route.h>
#include <net/transp_v6.h>
#include <net/ip6_route.h>
#include <net/addrconf.h>
#include <net/ndisc.h>
#ifdef CONFIG_IPV6_TUNNEL
#include <net/ip6_tunnel.h>
#endif

#include <asm/uaccess.h>
#include <linux/mroute6.h>

MODULE_AUTHOR("Cast of dozens");
MODULE_DESCRIPTION("IPv6 protocol stack for Linux");
MODULE_LICENSE("GPL");

/* The inetsw6 table contains everything that inet6_create needs to
 * build a new socket.
 */
static struct list_head inetsw6[SOCK_MAX];
static DEFINE_SPINLOCK(inetsw6_lock);

struct ipv6_params ipv6_defaults = {
	.disable_ipv6 = 0,
	.autoconf = 1,
};

static int disable_ipv6_mod;

module_param_named(disable, disable_ipv6_mod, int, 0444);
MODULE_PARM_DESC(disable, "Disable IPv6 module such that it is non-functional");

module_param_named(disable_ipv6, ipv6_defaults.disable_ipv6, int, 0444);
MODULE_PARM_DESC(disable_ipv6, "Disable IPv6 on all interfaces");

module_param_named(autoconf, ipv6_defaults.autoconf, int, 0444);
MODULE_PARM_DESC(autoconf, "Enable IPv6 address autoconfiguration on all interfaces");

static __inline__ struct ipv6_pinfo *inet6_sk_generic(struct sock *sk)
{
	const int offset = sk->sk_prot->obj_size - sizeof(struct ipv6_pinfo);

	return (struct ipv6_pinfo *)(((u8 *)sk) + offset);
}

#if !defined(CONFIG_BCM_MPTCP) || !defined(CONFIG_BCM_KF_MPTCP)
static int inet6_create(struct net *net, struct socket *sock, int protocol,
			int kern)
#else
int inet6_create(struct net *net, struct socket *sock, int protocol, int kern)
#endif
{
	struct inet_sock *inet;
	struct ipv6_pinfo *np;
	struct sock *sk;
	struct inet_protosw *answer;
	struct proto *answer_prot;
	unsigned char answer_flags;
	int try_loading_module = 0;
	int err;

	if (protocol < 0 || protocol >= IPPROTO_MAX)
		return -EINVAL;

	/* Look for the requested type/protocol pair. */
lookup_protocol:
	err = -ESOCKTNOSUPPORT;
	rcu_read_lock();
	list_for_each_entry_rcu(answer, &inetsw6[sock->type], list) {

		err = 0;
		/* Check the non-wild match. */
		if (protocol == answer->protocol) {
			if (protocol != IPPROTO_IP)
				break;
		} else {
			/* Check for the two wild cases. */
			if (IPPROTO_IP == protocol) {
				protocol = answer->protocol;
				break;
			}
			if (IPPROTO_IP == answer->protocol)
				break;
		}
		err = -EPROTONOSUPPORT;
	}

	if (err) {
		if (try_loading_module < 2) {
			rcu_read_unlock();
			/*
			 * Be more specific, e.g. net-pf-10-proto-132-type-1
			 * (net-pf-PF_INET6-proto-IPPROTO_SCTP-type-SOCK_STREAM)
			 */
			if (++try_loading_module == 1)
				request_module("net-pf-%d-proto-%d-type-%d",
						PF_INET6, protocol, sock->type);
			/*
			 * Fall back to generic, e.g. net-pf-10-proto-132
			 * (net-pf-PF_INET6-proto-IPPROTO_SCTP)
			 */
			else
				request_module("net-pf-%d-proto-%d",
						PF_INET6, protocol);
			goto lookup_protocol;
		} else
			goto out_rcu_unlock;
	}

	err = -EPERM;
	if (sock->type == SOCK_RAW && !kern &&
	    !ns_capable(net->user_ns, CAP_NET_RAW))
		goto out_rcu_unlock;

	sock->ops = answer->ops;
	answer_prot = answer->prot;
	answer_flags = answer->flags;
	rcu_read_unlock();

	WARN_ON(!answer_prot->slab);

	err = -ENOBUFS;
	sk = sk_alloc(net, PF_INET6, GFP_KERNEL, answer_prot);
	if (!sk)
		goto out;

	sock_init_data(sock, sk);

	err = 0;
	if (INET_PROTOSW_REUSE & answer_flags)
		sk->sk_reuse = SK_CAN_REUSE;

	inet = inet_sk(sk);
	inet->is_icsk = (INET_PROTOSW_ICSK & answer_flags) != 0;

	if (SOCK_RAW == sock->type) {
		inet->inet_num = protocol;
		if (IPPROTO_RAW == protocol)
			inet->hdrincl = 1;
	}

	sk->sk_destruct		= inet_sock_destruct;
	sk->sk_family		= PF_INET6;
	sk->sk_protocol		= protocol;

	sk->sk_backlog_rcv	= answer->prot->backlog_rcv;

	inet_sk(sk)->pinet6 = np = inet6_sk_generic(sk);
	np->hop_limit	= -1;
	np->mcast_hops	= IPV6_DEFAULT_MCASTHOPS;
	np->mc_loop	= 1;
	np->pmtudisc	= IPV6_PMTUDISC_WANT;
	sk->sk_ipv6only	= net->ipv6.sysctl.bindv6only;

	/* Init the ipv4 part of the socket since we can have sockets
	 * using v6 API for ipv4.
	 */
	inet->uc_ttl	= -1;

	inet->mc_loop	= 1;
	inet->mc_ttl	= 1;
	inet->mc_index	= 0;
	inet->mc_list	= NULL;
	inet->rcv_tos	= 0;

	if (net->ipv4.sysctl_ip_no_pmtu_disc)
		inet->pmtudisc = IP_PMTUDISC_DONT;
	else
		inet->pmtudisc = IP_PMTUDISC_WANT;
	/*
	 * Increment only the relevant sk_prot->socks debug field, this changes
	 * the previous behaviour of incrementing both the equivalent to
	 * answer->prot->socks (inet6_sock_nr) and inet_sock_nr.
	 *
	 * This allows better debug granularity as we'll know exactly how many
	 * UDPv6, TCPv6, etc socks were allocated, not the sum of all IPv6
	 * transport protocol socks. -acme
	 */
	sk_refcnt_debug_inc(sk);

	if (inet->inet_num) {
		/* It assumes that any protocol which allows
		 * the user to assign a number at socket
		 * creation time automatically shares.
		 */
		inet->inet_sport = htons(inet->inet_num);
		sk->sk_prot->hash(sk);
	}
	if (sk->sk_prot->init) {
		err = sk->sk_prot->init(sk);
		if (err) {
			sk_common_release(sk);
			goto out;
		}
	}
out:
	return err;
out_rcu_unlock:
	rcu_read_unlock();
	goto out;
}


/* bind for INET6 API */
int inet6_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_in6 *addr = (struct sockaddr_in6 *)uaddr;
	struct sock *sk = sock->sk;
	struct inet_sock *inet = inet_sk(sk);
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct net *net = sock_net(sk);
	__be32 v4addr = 0;
	unsigned short snum;
	int addr_type = 0;
	int err = 0;

	/* If the socket has its own bind function then use it. */
	if (sk->sk_prot->bind)
		return sk->sk_prot->bind(sk, uaddr, addr_len);

	if (addr_len < SIN6_LEN_RFC2133)
		return -EINVAL;

	if (addr->sin6_family != AF_INET6)
		return -EAFNOSUPPORT;

	addr_type = ipv6_addr_type(&addr->sin6_addr);
	if ((addr_type & IPV6_ADDR_MULTICAST) && sock->type == SOCK_STREAM)
		return -EINVAL;

	snum = ntohs(addr->sin6_port);
	if (snum && snum < PROT_SOCK && !ns_capable(net->user_ns, CAP_NET_BIND_SERVICE))
		return -EACCES;

	lock_sock(sk);

	/* Check these errors (active socket, double bind). */
	if (sk->sk_state != TCP_CLOSE || inet->inet_num) {
		err = -EINVAL;
		goto out;
	}

	/* Check if the address belongs to the host. */
	if (addr_type == IPV6_ADDR_MAPPED) {
		int chk_addr_ret;

		/* Binding to v4-mapped address on a v6-only socket
		 * makes no sense
		 */
		if (sk->sk_ipv6only) {
			err = -EINVAL;
			goto out;
		}

		/* Reproduce AF_INET checks to make the bindings consistent */
		v4addr = addr->sin6_addr.s6_addr32[3];
		chk_addr_ret = inet_addr_type(net, v4addr);
		if (!net->ipv4.sysctl_ip_nonlocal_bind &&
		    !(inet->freebind || inet->transparent) &&
		    v4addr != htonl(INADDR_ANY) &&
		    chk_addr_ret != RTN_LOCAL &&
		    chk_addr_ret != RTN_MULTICAST &&
		    chk_addr_ret != RTN_BROADCAST) {
			err = -EADDRNOTAVAIL;
			goto out;
		}
	} else {
		if (addr_type != IPV6_ADDR_ANY) {
			struct net_device *dev = NULL;

			rcu_read_lock();
			if (__ipv6_addr_needs_scope_id(addr_type)) {
				if (addr_len >= sizeof(struct sockaddr_in6) &&
				    addr->sin6_scope_id) {
					/* Override any existing binding, if another one
					 * is supplied by user.
					 */
					sk->sk_bound_dev_if = addr->sin6_scope_id;
				}

				/* Binding to link-local address requires an interface */
				if (!sk->sk_bound_dev_if) {
					err = -EINVAL;
					goto out_unlock;
				}
				dev = dev_get_by_index_rcu(net, sk->sk_bound_dev_if);
				if (!dev) {
					err = -ENODEV;
					goto out_unlock;
				}
			}

			/* ipv4 addr of the socket is invalid.  Only the
			 * unspecified and mapped address have a v4 equivalent.
			 */
			v4addr = LOOPBACK4_IPV6;
			if (!(addr_type & IPV6_ADDR_MULTICAST))	{
				if (!(inet->freebind || inet->transparent) &&
				    !ipv6_chk_addr(net, &addr->sin6_addr,
						   dev, 0)) {
					err = -EADDRNOTAVAIL;
					goto out_unlock;
				}
			}
			rcu_read_unlock();
		}
	}

	inet->inet_rcv_saddr = v4addr;
	inet->inet_saddr = v4addr;

	sk->sk_v6_rcv_saddr = addr->sin6_addr;

	if (!(addr_type & IPV6_ADDR_MULTICAST))
		np->saddr = addr->sin6_addr;

	/* Make sure we are allowed to bind here. */
	if (sk->sk_prot->get_port(sk, snum)) {
		inet_reset_saddr(sk);
		err = -EADDRINUSE;
		goto out;
	}

	if (addr_type != IPV6_ADDR_ANY) {
		sk->sk_userlocks |= SOCK_BINDADDR_LOCK;
		if (addr_type != IPV6_ADDR_MAPPED)
			sk->sk_ipv6only = 1;
	}
	if (snum)
		sk->sk_userlocks |= SOCK_BINDPORT_LOCK;
	inet->inet_sport = htons(inet->inet_num);
	inet->inet_dport = 0;
	inet->inet_daddr = 0;
out:
	release_sock(sk);
	return err;
out_unlock:
	rcu_read_unlock();
	goto out;
}
EXPORT_SYMBOL(inet6_bind);

int inet6_release(struct socket *sock)
{
	struct sock *sk = sock->sk;

	if (!sk)
		return -EINVAL;

	/* Free mc lists */
	ipv6_sock_mc_close(sk);

	/* Free ac lists */
	ipv6_sock_ac_close(sk);

	return inet_release(sock);
}
EXPORT_SYMBOL(inet6_release);

void inet6_destroy_sock(struct sock *sk)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct sk_buff *skb;
	struct ipv6_txoptions *opt;

	/* Release rx options */

	skb = xchg(&np->pktoptions, NULL);
	if (skb)
		kfree_skb(skb);

	skb = xchg(&np->rxpmtu, NULL);
	if (skb)
		kfree_skb(skb);

	/* Free flowlabels */
	fl6_free_socklist(sk);

	/* Free tx options */

	opt = xchg((__force struct ipv6_txoptions **)&np->opt, NULL);
	if (opt) {
		atomic_sub(opt->tot_len, &sk->sk_omem_alloc);
		txopt_put(opt);
	}
}
EXPORT_SYMBOL_GPL(inet6_destroy_sock);

/*
 *	This does both peername and sockname.
 */

int inet6_getname(struct socket *sock, struct sockaddr *uaddr,
		 int *uaddr_len, int peer)
{
	struct sockaddr_in6 *sin = (struct sockaddr_in6 *)uaddr;
	struct sock *sk = sock->sk;
	struct inet_sock *inet = inet_sk(sk);
	struct ipv6_pinfo *np = inet6_sk(sk);

	sin->sin6_family = AF_INET6;
	sin->sin6_flowinfo = 0;
	sin->sin6_scope_id = 0;
	if (peer) {
		if (!inet->inet_dport)
			return -ENOTCONN;
		if (((1 << sk->sk_state) & (TCPF_CLOSE | TCPF_SYN_SENT)) &&
		    peer == 1)
			return -ENOTCONN;
		sin->sin6_port = inet->inet_dport;
		sin->sin6_addr = sk->sk_v6_daddr;
		if (np->sndflow)
			sin->sin6_flowinfo = np->flow_label;
	} else {
		if (ipv6_addr_any(&sk->sk_v6_rcv_saddr))
			sin->sin6_addr = np->saddr;
		else
			sin->sin6_addr = sk->sk_v6_rcv_saddr;

		sin->sin6_port = inet->inet_sport;
	}
	sin->sin6_scope_id = ipv6_iface_scope_id(&sin->sin6_addr,
						 sk->sk_bound_dev_if);
	*uaddr_len = sizeof(*sin);
	return 0;
}
EXPORT_SYMBOL(inet6_getname);

int inet6_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);

	switch (cmd) {
	case SIOCGSTAMP:
		return sock_get_timestamp(sk, (struct timeval __user *)arg);

	case SIOCGSTAMPNS:
		return sock_get_timestampns(sk, (struct timespec __user *)arg);

	case SIOCADDRT:
	case SIOCDELRT:

		return ipv6_route_ioctl(net, cmd, (void __user *)arg);

	case SIOCSIFADDR:
		return addrconf_add_ifaddr(net, (void __user *) arg);
	case SIOCDIFADDR:
		return addrconf_del_ifaddr(net, (void __user *) arg);
	case SIOCSIFDSTADDR:
		return addrconf_set_dstaddr(net, (void __user *) arg);
	default:
		if (!sk->sk_prot->ioctl)
			return -ENOIOCTLCMD;
		return sk->sk_prot->ioctl(sk, cmd, arg);
	}
	/*NOTREACHED*/
	return 0;
}
EXPORT_SYMBOL(inet6_ioctl);

const struct proto_ops inet6_stream_ops = {
	.family		   = PF_INET6,
	.owner		   = THIS_MODULE,
	.release	   = inet6_release,
	.bind		   = inet6_bind,
	.connect	   = inet_stream_connect,	/* ok		*/
	.socketpair	   = sock_no_socketpair,	/* a do nothing	*/
	.accept		   = inet_accept,		/* ok		*/
	.getname	   = inet6_getname,
	.poll		   = tcp_poll,			/* ok		*/
	.ioctl		   = inet6_ioctl,		/* must change  */
	.listen		   = inet_listen,		/* ok		*/
	.shutdown	   = inet_shutdown,		/* ok		*/
	.setsockopt	   = sock_common_setsockopt,	/* ok		*/
	.getsockopt	   = sock_common_getsockopt,	/* ok		*/
	.sendmsg	   = inet_sendmsg,		/* ok		*/
	.recvmsg	   = inet_recvmsg,		/* ok		*/
	.mmap		   = sock_no_mmap,
	.sendpage	   = inet_sendpage,
	.splice_read	   = tcp_splice_read,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_sock_common_setsockopt,
	.compat_getsockopt = compat_sock_common_getsockopt,
#endif
};

const struct proto_ops inet6_dgram_ops = {
	.family		   = PF_INET6,
	.owner		   = THIS_MODULE,
	.release	   = inet6_release,
	.bind		   = inet6_bind,
	.connect	   = inet_dgram_connect,	/* ok		*/
	.socketpair	   = sock_no_socketpair,	/* a do nothing	*/
	.accept		   = sock_no_accept,		/* a do nothing	*/
	.getname	   = inet6_getname,
	.poll		   = udp_poll,			/* ok		*/
	.ioctl		   = inet6_ioctl,		/* must change  */
	.listen		   = sock_no_listen,		/* ok		*/
	.shutdown	   = inet_shutdown,		/* ok		*/
	.setsockopt	   = sock_common_setsockopt,	/* ok		*/
	.getsockopt	   = sock_common_getsockopt,	/* ok		*/
	.sendmsg	   = inet_sendmsg,		/* ok		*/
	.recvmsg	   = inet_recvmsg,		/* ok		*/
	.mmap		   = sock_no_mmap,
	.sendpage	   = sock_no_sendpage,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_sock_common_setsockopt,
	.compat_getsockopt = compat_sock_common_getsockopt,
#endif
};

static const struct net_proto_family inet6_family_ops = {
	.family = PF_INET6,
	.create = inet6_create,
	.owner	= THIS_MODULE,
};

int inet6_register_protosw(struct inet_protosw *p)
{
	struct list_head *lh;
	struct inet_protosw *answer;
	struct list_head *last_perm;
	int protocol = p->protocol;
	int ret;

	spin_lock_bh(&inetsw6_lock);

	ret = -EINVAL;
	if (p->type >= SOCK_MAX)
		goto out_illegal;

	/* If we are trying to override a permanent protocol, bail. */
	answer = NULL;
	ret = -EPERM;
	last_perm = &inetsw6[p->type];
	list_for_each(lh, &inetsw6[p->type]) {
		answer = list_entry(lh, struct inet_protosw, list);

		/* Check only the non-wild match. */
		if (INET_PROTOSW_PERMANENT & answer->flags) {
			if (protocol == answer->protocol)
				break;
			last_perm = lh;
		}

		answer = NULL;
	}
	if (answer)
		goto out_permanent;

	/* Add the new entry after the last permanent entry if any, so that
	 * the new entry does not override a permanent entry when matched with
	 * a wild-card protocol. But it is allowed to override any existing
	 * non-permanent entry.  This means that when we remove this entry, the
	 * system automatically returns to the old behavior.
	 */
	list_add_rcu(&p->list, last_perm);
	ret = 0;
out:
	spin_unlock_bh(&inetsw6_lock);
	return ret;

out_permanent:
	pr_err("Attempt to override permanent protocol %d\n", protocol);
	goto out;

out_illegal:
	pr_err("Ignoring attempt to register invalid socket type %d\n",
	       p->type);
	goto out;
}
EXPORT_SYMBOL(inet6_register_protosw);

void
inet6_unregister_protosw(struct inet_protosw *p)
{
	if (INET_PROTOSW_PERMANENT & p->flags) {
		pr_err("Attempt to unregister permanent protocol %d\n",
		       p->protocol);
	} else {
		spin_lock_bh(&inetsw6_lock);
		list_del_rcu(&p->list);
		spin_unlock_bh(&inetsw6_lock);

		synchronize_net();
	}
}
EXPORT_SYMBOL(inet6_unregister_protosw);

int inet6_sk_rebuild_header(struct sock *sk)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct dst_entry *dst;

	dst = __sk_dst_check(sk, np->dst_cookie);

	if (!dst) {
		struct inet_sock *inet = inet_sk(sk);
		struct in6_addr *final_p, final;
		struct flowi6 fl6;

		memset(&fl6, 0, sizeof(fl6));
		fl6.flowi6_proto = sk->sk_protocol;
		fl6.daddr = sk->sk_v6_daddr;
		fl6.saddr = np->saddr;
		fl6.flowlabel = np->flow_label;
		fl6.flowi6_oif = sk->sk_bound_dev_if;
		fl6.flowi6_mark = sk->sk_mark;
		fl6.fl6_dport = inet->inet_dport;
		fl6.fl6_sport = inet->inet_sport;
		security_sk_classify_flow(sk, flowi6_to_flowi(&fl6));

		rcu_read_lock();
		final_p = fl6_update_dst(&fl6, rcu_dereference(np->opt),
					 &final);
		rcu_read_unlock();

		dst = ip6_dst_lookup_flow(sk, &fl6, final_p);
		if (IS_ERR(dst)) {
			sk->sk_route_caps = 0;
			sk->sk_err_soft = -PTR_ERR(dst);
			return PTR_ERR(dst);
		}

		__ip6_dst_store(sk, dst, NULL, NULL);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(inet6_sk_rebuild_header);

bool ipv6_opt_accepted(const struct sock *sk, const struct sk_buff *skb,
		       const struct inet6_skb_parm *opt)
{
	const struct ipv6_pinfo *np = inet6_sk(sk);

	if (np->rxopt.all) {
		if ((opt->hop && (np->rxopt.bits.hopopts ||
				  np->rxopt.bits.ohopopts)) ||
		    (ip6_flowinfo((struct ipv6hdr *) skb_network_header(skb)) &&
		     np->rxopt.bits.rxflow) ||
		    (opt->srcrt && (np->rxopt.bits.srcrt ||
		     np->rxopt.bits.osrcrt)) ||
		    ((opt->dst1 || opt->dst0) &&
		     (np->rxopt.bits.dstopts || np->rxopt.bits.odstopts)))
			return true;
	}
	return false;
}
EXPORT_SYMBOL_GPL(ipv6_opt_accepted);

static struct packet_type ipv6_packet_type __read_mostly = {
	.type = cpu_to_be16(ETH_P_IPV6),
	.func = ipv6_rcv,
};

static int __init ipv6_packet_init(void)
{
	dev_add_pack(&ipv6_packet_type);
	return 0;
}

static void ipv6_packet_cleanup(void)
{
	dev_remove_pack(&ipv6_packet_type);
}

static int __net_init ipv6_init_mibs(struct net *net)
{
	int i;

	net->mib.udp_stats_in6 = alloc_percpu(struct udp_mib);
	if (!net->mib.udp_stats_in6)
		return -ENOMEM;
	net->mib.udplite_stats_in6 = alloc_percpu(struct udp_mib);
	if (!net->mib.udplite_stats_in6)
		goto err_udplite_mib;
	net->mib.ipv6_statistics = alloc_percpu(struct ipstats_mib);
	if (!net->mib.ipv6_statistics)
		goto err_ip_mib;

	for_each_possible_cpu(i) {
		struct ipstats_mib *af_inet6_stats;
		af_inet6_stats = per_cpu_ptr(net->mib.ipv6_statistics, i);
		u64_stats_init(&af_inet6_stats->syncp);
	}


	net->mib.icmpv6_statistics = alloc_percpu(struct icmpv6_mib);
	if (!net->mib.icmpv6_statistics)
		goto err_icmp_mib;
	net->mib.icmpv6msg_statistics = kzalloc(sizeof(struct icmpv6msg_mib),
						GFP_KERNEL);
	if (!net->mib.icmpv6msg_statistics)
		goto err_icmpmsg_mib;
	return 0;

err_icmpmsg_mib:
	free_percpu(net->mib.icmpv6_statistics);
err_icmp_mib:
	free_percpu(net->mib.ipv6_statistics);
err_ip_mib:
	free_percpu(net->mib.udplite_stats_in6);
err_udplite_mib:
	free_percpu(net->mib.udp_stats_in6);
	return -ENOMEM;
}

static void ipv6_cleanup_mibs(struct net *net)
{
	free_percpu(net->mib.udp_stats_in6);
	free_percpu(net->mib.udplite_stats_in6);
	free_percpu(net->mib.ipv6_statistics);
	free_percpu(net->mib.icmpv6_statistics);
	kfree(net->mib.icmpv6msg_statistics);
}

static int __net_init inet6_net_init(struct net *net)
{
	int err = 0;

	net->ipv6.sysctl.bindv6only = 0;
	net->ipv6.sysctl.icmpv6_time = 1*HZ;
	net->ipv6.sysctl.flowlabel_consistency = 1;
	net->ipv6.sysctl.auto_flowlabels = 0;
	net->ipv6.sysctl.idgen_retries = 3;
	net->ipv6.sysctl.idgen_delay = 1 * HZ;
	atomic_set(&net->ipv6.fib6_sernum, 1);

	err = ipv6_init_mibs(net);
	if (err)
		return err;
#ifdef CONFIG_PROC_FS
	err = udp6_proc_init(net);
	if (err)
		goto out;
	err = tcp6_proc_init(net);
	if (err)
		goto proc_tcp6_fail;
	err = ac6_proc_init(net);
	if (err)
		goto proc_ac6_fail;
#endif
	return err;

#ifdef CONFIG_PROC_FS
proc_ac6_fail:
	tcp6_proc_exit(net);
proc_tcp6_fail:
	udp6_proc_exit(net);
out:
	ipv6_cleanup_mibs(net);
	return err;
#endif
}

static void __net_exit inet6_net_exit(struct net *net)
{
#ifdef CONFIG_PROC_FS
	udp6_proc_exit(net);
	tcp6_proc_exit(net);
	ac6_proc_exit(net);
#endif
	ipv6_cleanup_mibs(net);
}

static struct pernet_operations inet6_net_ops = {
	.init = inet6_net_init,
	.exit = inet6_net_exit,
};

static const struct ipv6_stub ipv6_stub_impl = {
	.ipv6_sock_mc_join = ipv6_sock_mc_join,
	.ipv6_sock_mc_drop = ipv6_sock_mc_drop,
	.ipv6_dst_lookup = ip6_dst_lookup,
	.udpv6_encap_enable = udpv6_encap_enable,
	.ndisc_send_na = ndisc_send_na,
	.nd_tbl	= &nd_tbl,
};

static int __init inet6_init(void)
{
	struct list_head *r;
	int err = 0;

	sock_skb_cb_check_size(sizeof(struct inet6_skb_parm));

	/* Register the socket-side information for inet6_create.  */
	for (r = &inetsw6[0]; r < &inetsw6[SOCK_MAX]; ++r)
		INIT_LIST_HEAD(r);

	if (disable_ipv6_mod) {
		pr_info("Loaded, but administratively disabled, reboot required to enable\n");
		goto out;
	}

	err = proto_register(&tcpv6_prot, 1);
	if (err)
		goto out;

	err = proto_register(&udpv6_prot, 1);
	if (err)
		goto out_unregister_tcp_proto;

	err = proto_register(&udplitev6_prot, 1);
	if (err)
		goto out_unregister_udp_proto;

	err = proto_register(&rawv6_prot, 1);
	if (err)
		goto out_unregister_udplite_proto;

	err = proto_register(&pingv6_prot, 1);
	if (err)
		goto out_unregister_ping_proto;

	/* We MUST register RAW sockets before we create the ICMP6,
	 * IGMP6, or NDISC control sockets.
	 */
	err = rawv6_init();
	if (err)
		goto out_unregister_raw_proto;

	/* Register the family here so that the init calls below will
	 * be able to create sockets. (?? is this dangerous ??)
	 */
	err = sock_register(&inet6_family_ops);
	if (err)
		goto out_sock_register_fail;

	/*
	 *	ipngwg API draft makes clear that the correct semantics
	 *	for TCP and UDP is to consider one TCP and UDP instance
	 *	in a host available by both INET and INET6 APIs and
	 *	able to communicate via both network protocols.
	 */

	err = register_pernet_subsys(&inet6_net_ops);
	if (err)
		goto register_pernet_fail;
	err = ip6_mr_init();
	if (err)
		goto ipmr_fail;
	err = icmpv6_init();
	if (err)
		goto icmp_fail;
	err = ndisc_init();
	if (err)
		goto ndisc_fail;
	err = igmp6_init();
	if (err)
		goto igmp_fail;

	ipv6_stub = &ipv6_stub_impl;

	err = ipv6_netfilter_init();
	if (err)
		goto netfilter_fail;
	/* Create /proc/foo6 entries. */
#ifdef CONFIG_PROC_FS
	err = -ENOMEM;
	if (raw6_proc_init())
		goto proc_raw6_fail;
	if (udplite6_proc_init())
		goto proc_udplite6_fail;
	if (ipv6_misc_proc_init())
		goto proc_misc6_fail;
	if (if6_proc_init())
		goto proc_if6_fail;
#endif
	err = ip6_route_init();
	if (err)
		goto ip6_route_fail;
	err = ndisc_late_init();
	if (err)
		goto ndisc_late_fail;
	err = ip6_flowlabel_init();
	if (err)
		goto ip6_flowlabel_fail;
	err = addrconf_init();
	if (err)
		goto addrconf_fail;

	/* Init v6 extension headers. */
	err = ipv6_exthdrs_init();
	if (err)
		goto ipv6_exthdrs_fail;

	err = ipv6_frag_init();
	if (err)
		goto ipv6_frag_fail;

	/* Init v6 transport protocols. */
	err = udpv6_init();
	if (err)
		goto udpv6_fail;

	err = udplitev6_init();
	if (err)
		goto udplitev6_fail;

	err = tcpv6_init();
	if (err)
		goto tcpv6_fail;

	err = ipv6_packet_init();
	if (err)
		goto ipv6_packet_fail;

	err = pingv6_init();
	if (err)
		goto pingv6_fail;

#ifdef CONFIG_SYSCTL
	err = ipv6_sysctl_register();
	if (err)
		goto sysctl_fail;
#endif
out:
	return err;

#ifdef CONFIG_SYSCTL
sysctl_fail:
	pingv6_exit();
#endif
pingv6_fail:
	ipv6_packet_cleanup();
ipv6_packet_fail:
	tcpv6_exit();
tcpv6_fail:
	udplitev6_exit();
udplitev6_fail:
	udpv6_exit();
udpv6_fail:
	ipv6_frag_exit();
ipv6_frag_fail:
	ipv6_exthdrs_exit();
ipv6_exthdrs_fail:
	addrconf_cleanup();
addrconf_fail:
	ip6_flowlabel_cleanup();
ip6_flowlabel_fail:
	ndisc_late_cleanup();
ndisc_late_fail:
	ip6_route_cleanup();
ip6_route_fail:
#ifdef CONFIG_PROC_FS
	if6_proc_exit();
proc_if6_fail:
	ipv6_misc_proc_exit();
proc_misc6_fail:
	udplite6_proc_exit();
proc_udplite6_fail:
	raw6_proc_exit();
proc_raw6_fail:
#endif
	ipv6_netfilter_fini();
netfilter_fail:
	igmp6_cleanup();
igmp_fail:
	ndisc_cleanup();
ndisc_fail:
	ip6_mr_cleanup();
icmp_fail:
	unregister_pernet_subsys(&inet6_net_ops);
ipmr_fail:
	icmpv6_cleanup();
register_pernet_fail:
	sock_unregister(PF_INET6);
	rtnl_unregister_all(PF_INET6);
out_sock_register_fail:
	rawv6_exit();
out_unregister_ping_proto:
	proto_unregister(&pingv6_prot);
out_unregister_raw_proto:
	proto_unregister(&rawv6_prot);
out_unregister_udplite_proto:
	proto_unregister(&udplitev6_prot);
out_unregister_udp_proto:
	proto_unregister(&udpv6_prot);
out_unregister_tcp_proto:
	proto_unregister(&tcpv6_prot);
	goto out;
}
module_init(inet6_init);

MODULE_ALIAS_NETPROTO(PF_INET6);
