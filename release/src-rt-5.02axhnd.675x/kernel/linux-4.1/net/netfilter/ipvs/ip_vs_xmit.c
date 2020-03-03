/*
 * ip_vs_xmit.c: various packet transmitters for IPVS
 *
 * Authors:     Wensong Zhang <wensong@linuxvirtualserver.org>
 *              Julian Anastasov <ja@ssi.bg>
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Changes:
 *
 * Description of forwarding methods:
 * - all transmitters are called from LOCAL_IN (remote clients) and
 * LOCAL_OUT (local clients) but for ICMP can be called from FORWARD
 * - not all connections have destination server, for example,
 * connections in backup server when fwmark is used
 * - bypass connections use daddr from packet
 * - we can use dst without ref while sending in RCU section, we use
 * ref when returning NF_ACCEPT for NAT-ed packet via loopback
 * LOCAL_OUT rules:
 * - skb->dev is NULL, skb->protocol is not set (both are set in POST_ROUTING)
 * - skb->pkt_type is not set yet
 * - the only place where we can see skb->sk != NULL
 */

#define KMSG_COMPONENT "IPVS"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/tcp.h>                  /* for tcphdr */
#include <net/ip.h>
#include <net/tcp.h>                    /* for csum_tcpudp_magic */
#include <net/udp.h>
#include <net/icmp.h>                   /* for icmp_send */
#include <net/route.h>                  /* for ip_route_output */
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <net/ip_tunnels.h>
#include <net/addrconf.h>
#include <linux/icmpv6.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <net/ip_vs.h>

enum {
	IP_VS_RT_MODE_LOCAL	= 1, /* Allow local dest */
	IP_VS_RT_MODE_NON_LOCAL	= 2, /* Allow non-local dest */
	IP_VS_RT_MODE_RDR	= 4, /* Allow redirect from remote daddr to
				      * local
				      */
	IP_VS_RT_MODE_CONNECT	= 8, /* Always bind route to saddr */
	IP_VS_RT_MODE_KNOWN_NH	= 16,/* Route via remote addr */
	IP_VS_RT_MODE_TUNNEL	= 32,/* Tunnel mode */
};

static inline struct ip_vs_dest_dst *ip_vs_dest_dst_alloc(void)
{
	return kmalloc(sizeof(struct ip_vs_dest_dst), GFP_ATOMIC);
}

static inline void ip_vs_dest_dst_free(struct ip_vs_dest_dst *dest_dst)
{
	kfree(dest_dst);
}

/*
 *      Destination cache to speed up outgoing route lookup
 */
static inline void
__ip_vs_dst_set(struct ip_vs_dest *dest, struct ip_vs_dest_dst *dest_dst,
		struct dst_entry *dst, u32 dst_cookie)
{
	struct ip_vs_dest_dst *old;

	old = rcu_dereference_protected(dest->dest_dst,
					lockdep_is_held(&dest->dst_lock));

	if (dest_dst) {
		dest_dst->dst_cache = dst;
		dest_dst->dst_cookie = dst_cookie;
	}
	rcu_assign_pointer(dest->dest_dst, dest_dst);

	if (old)
		call_rcu(&old->rcu_head, ip_vs_dest_dst_rcu_free);
}

static inline struct ip_vs_dest_dst *
__ip_vs_dst_check(struct ip_vs_dest *dest)
{
	struct ip_vs_dest_dst *dest_dst = rcu_dereference(dest->dest_dst);
	struct dst_entry *dst;

	if (!dest_dst)
		return NULL;
	dst = dest_dst->dst_cache;
	if (dst->obsolete &&
	    dst->ops->check(dst, dest_dst->dst_cookie) == NULL)
		return NULL;
	return dest_dst;
}

static inline bool
__mtu_check_toobig_v6(const struct sk_buff *skb, u32 mtu)
{
	if (IP6CB(skb)->frag_max_size) {
		/* frag_max_size tell us that, this packet have been
		 * defragmented by netfilter IPv6 conntrack module.
		 */
		if (IP6CB(skb)->frag_max_size > mtu)
			return true; /* largest fragment violate MTU */
	}
	else if (skb->len > mtu && !skb_is_gso(skb)) {
		return true; /* Packet size violate MTU size */
	}
	return false;
}

/* Get route to daddr, update *saddr, optionally bind route to saddr */
static struct rtable *do_output_route4(struct net *net, __be32 daddr,
				       int rt_mode, __be32 *saddr)
{
	struct flowi4 fl4;
	struct rtable *rt;
	int loop = 0;

	memset(&fl4, 0, sizeof(fl4));
	fl4.daddr = daddr;
	fl4.flowi4_flags = (rt_mode & IP_VS_RT_MODE_KNOWN_NH) ?
			   FLOWI_FLAG_KNOWN_NH : 0;

retry:
	rt = ip_route_output_key(net, &fl4);
	if (IS_ERR(rt)) {
		/* Invalid saddr ? */
		if (PTR_ERR(rt) == -EINVAL && *saddr &&
		    rt_mode & IP_VS_RT_MODE_CONNECT && !loop) {
			*saddr = 0;
			flowi4_update_output(&fl4, 0, 0, daddr, 0);
			goto retry;
		}
		IP_VS_DBG_RL("ip_route_output error, dest: %pI4\n", &daddr);
		return NULL;
	} else if (!*saddr && rt_mode & IP_VS_RT_MODE_CONNECT && fl4.saddr) {
		ip_rt_put(rt);
		*saddr = fl4.saddr;
		flowi4_update_output(&fl4, 0, 0, daddr, fl4.saddr);
		loop++;
		goto retry;
	}
	*saddr = fl4.saddr;
	return rt;
}

#ifdef CONFIG_IP_VS_IPV6
static inline int __ip_vs_is_local_route6(struct rt6_info *rt)
{
	return rt->dst.dev && rt->dst.dev->flags & IFF_LOOPBACK;
}
#endif

static inline bool crosses_local_route_boundary(int skb_af, struct sk_buff *skb,
						int rt_mode,
						bool new_rt_is_local)
{
	bool rt_mode_allow_local = !!(rt_mode & IP_VS_RT_MODE_LOCAL);
	bool rt_mode_allow_non_local = !!(rt_mode & IP_VS_RT_MODE_LOCAL);
	bool rt_mode_allow_redirect = !!(rt_mode & IP_VS_RT_MODE_RDR);
	bool source_is_loopback;
	bool old_rt_is_local;

#ifdef CONFIG_IP_VS_IPV6
	if (skb_af == AF_INET6) {
		int addr_type = ipv6_addr_type(&ipv6_hdr(skb)->saddr);

		source_is_loopback =
			(!skb->dev || skb->dev->flags & IFF_LOOPBACK) &&
			(addr_type & IPV6_ADDR_LOOPBACK);
		old_rt_is_local = __ip_vs_is_local_route6(
			(struct rt6_info *)skb_dst(skb));
	} else
#endif
	{
		source_is_loopback = ipv4_is_loopback(ip_hdr(skb)->saddr);
		old_rt_is_local = skb_rtable(skb)->rt_flags & RTCF_LOCAL;
	}

	if (unlikely(new_rt_is_local)) {
		if (!rt_mode_allow_local)
			return true;
		if (!rt_mode_allow_redirect && !old_rt_is_local)
			return true;
	} else {
		if (!rt_mode_allow_non_local)
			return true;
		if (source_is_loopback)
			return true;
	}
	return false;
}

static inline void maybe_update_pmtu(int skb_af, struct sk_buff *skb, int mtu)
{
	struct sock *sk = skb->sk;
	struct rtable *ort = skb_rtable(skb);

	if (!skb->dev && sk && sk_fullsock(sk))
		ort->dst.ops->update_pmtu(&ort->dst, sk, NULL, mtu);
}

static inline bool ensure_mtu_is_adequate(int skb_af, int rt_mode,
					  struct ip_vs_iphdr *ipvsh,
					  struct sk_buff *skb, int mtu)
{
#ifdef CONFIG_IP_VS_IPV6
	if (skb_af == AF_INET6) {
		struct net *net = dev_net(skb_dst(skb)->dev);

		if (unlikely(__mtu_check_toobig_v6(skb, mtu))) {
			if (!skb->dev)
				skb->dev = net->loopback_dev;
			/* only send ICMP too big on first fragment */
			if (!ipvsh->fragoffs)
				icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
			IP_VS_DBG(1, "frag needed for %pI6c\n",
				  &ipv6_hdr(skb)->saddr);
			return false;
		}
	} else
#endif
	{
		struct netns_ipvs *ipvs = net_ipvs(skb_net(skb));

		/* If we're going to tunnel the packet and pmtu discovery
		 * is disabled, we'll just fragment it anyway
		 */
		if ((rt_mode & IP_VS_RT_MODE_TUNNEL) && !sysctl_pmtu_disc(ipvs))
			return true;

		if (unlikely(ip_hdr(skb)->frag_off & htons(IP_DF) &&
			     skb->len > mtu && !skb_is_gso(skb))) {
			icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED,
				  htonl(mtu));
			IP_VS_DBG(1, "frag needed for %pI4\n",
				  &ip_hdr(skb)->saddr);
			return false;
		}
	}

	return true;
}

/* Get route to destination or remote server */
static int
__ip_vs_get_out_rt(int skb_af, struct sk_buff *skb, struct ip_vs_dest *dest,
		   __be32 daddr, int rt_mode, __be32 *ret_saddr,
		   struct ip_vs_iphdr *ipvsh)
{
	struct net *net = dev_net(skb_dst(skb)->dev);
	struct ip_vs_dest_dst *dest_dst;
	struct rtable *rt;			/* Route to the other host */
	int mtu;
	int local, noref = 1;

	if (dest) {
		dest_dst = __ip_vs_dst_check(dest);
		if (likely(dest_dst))
			rt = (struct rtable *) dest_dst->dst_cache;
		else {
			dest_dst = ip_vs_dest_dst_alloc();
			spin_lock_bh(&dest->dst_lock);
			if (!dest_dst) {
				__ip_vs_dst_set(dest, NULL, NULL, 0);
				spin_unlock_bh(&dest->dst_lock);
				goto err_unreach;
			}
			rt = do_output_route4(net, dest->addr.ip, rt_mode,
					      &dest_dst->dst_saddr.ip);
			if (!rt) {
				__ip_vs_dst_set(dest, NULL, NULL, 0);
				spin_unlock_bh(&dest->dst_lock);
				ip_vs_dest_dst_free(dest_dst);
				goto err_unreach;
			}
			__ip_vs_dst_set(dest, dest_dst, &rt->dst, 0);
			spin_unlock_bh(&dest->dst_lock);
			IP_VS_DBG(10, "new dst %pI4, src %pI4, refcnt=%d\n",
				  &dest->addr.ip, &dest_dst->dst_saddr.ip,
				  atomic_read(&rt->dst.__refcnt));
		}
		if (ret_saddr)
			*ret_saddr = dest_dst->dst_saddr.ip;
	} else {
		__be32 saddr = htonl(INADDR_ANY);

		noref = 0;

		/* For such unconfigured boxes avoid many route lookups
		 * for performance reasons because we do not remember saddr
		 */
		rt_mode &= ~IP_VS_RT_MODE_CONNECT;
		rt = do_output_route4(net, daddr, rt_mode, &saddr);
		if (!rt)
			goto err_unreach;
		if (ret_saddr)
			*ret_saddr = saddr;
	}

	local = (rt->rt_flags & RTCF_LOCAL) ? 1 : 0;
	if (unlikely(crosses_local_route_boundary(skb_af, skb, rt_mode,
						  local))) {
		IP_VS_DBG_RL("We are crossing local and non-local addresses"
			     " daddr=%pI4\n", &daddr);
		goto err_put;
	}

	if (unlikely(local)) {
		/* skb to local stack, preserve old route */
		if (!noref)
			ip_rt_put(rt);
		return local;
	}

	if (likely(!(rt_mode & IP_VS_RT_MODE_TUNNEL))) {
		mtu = dst_mtu(&rt->dst);
	} else {
		mtu = dst_mtu(&rt->dst) - sizeof(struct iphdr);
		if (mtu < 68) {
			IP_VS_DBG_RL("%s(): mtu less than 68\n", __func__);
			goto err_put;
		}
		maybe_update_pmtu(skb_af, skb, mtu);
	}

	if (!ensure_mtu_is_adequate(skb_af, rt_mode, ipvsh, skb, mtu))
		goto err_put;

	skb_dst_drop(skb);
	if (noref) {
		if (!local)
			skb_dst_set_noref(skb, &rt->dst);
		else
			skb_dst_set(skb, dst_clone(&rt->dst));
	} else
		skb_dst_set(skb, &rt->dst);

	return local;

err_put:
	if (!noref)
		ip_rt_put(rt);
	return -1;

err_unreach:
	dst_link_failure(skb);
	return -1;
}

#ifdef CONFIG_IP_VS_IPV6
static struct dst_entry *
__ip_vs_route_output_v6(struct net *net, struct in6_addr *daddr,
			struct in6_addr *ret_saddr, int do_xfrm)
{
	struct dst_entry *dst;
	struct flowi6 fl6 = {
		.daddr = *daddr,
	};

	dst = ip6_route_output(net, NULL, &fl6);
	if (dst->error)
		goto out_err;
	if (!ret_saddr)
		return dst;
	if (ipv6_addr_any(&fl6.saddr) &&
	    ipv6_dev_get_saddr(net, ip6_dst_idev(dst)->dev,
			       &fl6.daddr, 0, &fl6.saddr) < 0)
		goto out_err;
	if (do_xfrm) {
		dst = xfrm_lookup(net, dst, flowi6_to_flowi(&fl6), NULL, 0);
		if (IS_ERR(dst)) {
			dst = NULL;
			goto out_err;
		}
	}
	*ret_saddr = fl6.saddr;
	return dst;

out_err:
	dst_release(dst);
	IP_VS_DBG_RL("ip6_route_output error, dest: %pI6\n", daddr);
	return NULL;
}

/*
 * Get route to destination or remote server
 */
static int
__ip_vs_get_out_rt_v6(int skb_af, struct sk_buff *skb, struct ip_vs_dest *dest,
		      struct in6_addr *daddr, struct in6_addr *ret_saddr,
		      struct ip_vs_iphdr *ipvsh, int do_xfrm, int rt_mode)
{
	struct net *net = dev_net(skb_dst(skb)->dev);
	struct ip_vs_dest_dst *dest_dst;
	struct rt6_info *rt;			/* Route to the other host */
	struct dst_entry *dst;
	int mtu;
	int local, noref = 1;

	if (dest) {
		dest_dst = __ip_vs_dst_check(dest);
		if (likely(dest_dst))
			rt = (struct rt6_info *) dest_dst->dst_cache;
		else {
			u32 cookie;

			dest_dst = ip_vs_dest_dst_alloc();
			spin_lock_bh(&dest->dst_lock);
			if (!dest_dst) {
				__ip_vs_dst_set(dest, NULL, NULL, 0);
				spin_unlock_bh(&dest->dst_lock);
				goto err_unreach;
			}
			dst = __ip_vs_route_output_v6(net, &dest->addr.in6,
						      &dest_dst->dst_saddr.in6,
						      do_xfrm);
			if (!dst) {
				__ip_vs_dst_set(dest, NULL, NULL, 0);
				spin_unlock_bh(&dest->dst_lock);
				ip_vs_dest_dst_free(dest_dst);
				goto err_unreach;
			}
			rt = (struct rt6_info *) dst;
			cookie = rt->rt6i_node ? rt->rt6i_node->fn_sernum : 0;
			__ip_vs_dst_set(dest, dest_dst, &rt->dst, cookie);
			spin_unlock_bh(&dest->dst_lock);
			IP_VS_DBG(10, "new dst %pI6, src %pI6, refcnt=%d\n",
				  &dest->addr.in6, &dest_dst->dst_saddr.in6,
				  atomic_read(&rt->dst.__refcnt));
		}
		if (ret_saddr)
			*ret_saddr = dest_dst->dst_saddr.in6;
	} else {
		noref = 0;
		dst = __ip_vs_route_output_v6(net, daddr, ret_saddr, do_xfrm);
		if (!dst)
			goto err_unreach;
		rt = (struct rt6_info *) dst;
	}

	local = __ip_vs_is_local_route6(rt);

	if (unlikely(crosses_local_route_boundary(skb_af, skb, rt_mode,
						  local))) {
		IP_VS_DBG_RL("We are crossing local and non-local addresses"
			     " daddr=%pI6\n", daddr);
		goto err_put;
	}

	if (unlikely(local)) {
		/* skb to local stack, preserve old route */
		if (!noref)
			dst_release(&rt->dst);
		return local;
	}

	/* MTU checking */
	if (likely(!(rt_mode & IP_VS_RT_MODE_TUNNEL)))
		mtu = dst_mtu(&rt->dst);
	else {
		mtu = dst_mtu(&rt->dst) - sizeof(struct ipv6hdr);
		if (mtu < IPV6_MIN_MTU) {
			IP_VS_DBG_RL("%s(): mtu less than %d\n", __func__,
				     IPV6_MIN_MTU);
			goto err_put;
		}
		maybe_update_pmtu(skb_af, skb, mtu);
	}

	if (!ensure_mtu_is_adequate(skb_af, rt_mode, ipvsh, skb, mtu))
		goto err_put;

	skb_dst_drop(skb);
	if (noref) {
		if (!local)
			skb_dst_set_noref(skb, &rt->dst);
		else
			skb_dst_set(skb, dst_clone(&rt->dst));
	} else
		skb_dst_set(skb, &rt->dst);

	return local;

err_put:
	if (!noref)
		dst_release(&rt->dst);
	return -1;

err_unreach:
	dst_link_failure(skb);
	return -1;
}
#endif


/* return NF_ACCEPT to allow forwarding or other NF_xxx on error */
static inline int ip_vs_tunnel_xmit_prepare(struct sk_buff *skb,
					    struct ip_vs_conn *cp)
{
	int ret = NF_ACCEPT;

	skb->ipvs_property = 1;
	if (unlikely(cp->flags & IP_VS_CONN_F_NFCT))
		ret = ip_vs_confirm_conntrack(skb);
	if (ret == NF_ACCEPT) {
		nf_reset(skb);
		skb_forward_csum(skb);
		if (!skb->sk)
			skb_sender_cpu_clear(skb);
	}
	return ret;
}

/* In the event of a remote destination, it's possible that we would have
 * matches against an old socket (particularly a TIME-WAIT socket). This
 * causes havoc down the line (ip_local_out et. al. expect regular sockets
 * and invalid memory accesses will happen) so simply drop the association
 * in this case.
*/
static inline void ip_vs_drop_early_demux_sk(struct sk_buff *skb)
{
	/* If dev is set, the packet came from the LOCAL_IN callback and
	 * not from a local TCP socket.
	 */
	if (skb->dev)
		skb_orphan(skb);
}

/* return NF_STOLEN (sent) or NF_ACCEPT if local=1 (not sent) */
static inline int ip_vs_nat_send_or_cont(int pf, struct sk_buff *skb,
					 struct ip_vs_conn *cp, int local)
{
	int ret = NF_STOLEN;

	skb->ipvs_property = 1;
	if (likely(!(cp->flags & IP_VS_CONN_F_NFCT)))
		ip_vs_notrack(skb);
	else
		ip_vs_update_conntrack(skb, cp, 1);

	/* Remove the early_demux association unless it's bound for the
	 * exact same port and address on this host after translation.
	 */
	if (!local || cp->vport != cp->dport ||
	    !ip_vs_addr_equal(cp->af, &cp->vaddr, &cp->daddr))
		ip_vs_drop_early_demux_sk(skb);

	if (!local) {
		skb_forward_csum(skb);
		if (!skb->sk)
			skb_sender_cpu_clear(skb);
		NF_HOOK(pf, NF_INET_LOCAL_OUT, NULL, skb,
			NULL, skb_dst(skb)->dev, dst_output_sk);
	} else
		ret = NF_ACCEPT;

	return ret;
}

/* return NF_STOLEN (sent) or NF_ACCEPT if local=1 (not sent) */
static inline int ip_vs_send_or_cont(int pf, struct sk_buff *skb,
				     struct ip_vs_conn *cp, int local)
{
	int ret = NF_STOLEN;

	skb->ipvs_property = 1;
	if (likely(!(cp->flags & IP_VS_CONN_F_NFCT)))
		ip_vs_notrack(skb);
	if (!local) {
		ip_vs_drop_early_demux_sk(skb);
		skb_forward_csum(skb);
		if (!skb->sk)
			skb_sender_cpu_clear(skb);
		NF_HOOK(pf, NF_INET_LOCAL_OUT, NULL, skb,
			NULL, skb_dst(skb)->dev, dst_output_sk);
	} else
		ret = NF_ACCEPT;
	return ret;
}


/*
 *      NULL transmitter (do nothing except return NF_ACCEPT)
 */
int
ip_vs_null_xmit(struct sk_buff *skb, struct ip_vs_conn *cp,
		struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	/* we do not touch skb and do not need pskb ptr */
	return ip_vs_send_or_cont(NFPROTO_IPV4, skb, cp, 1);
}


/*
 *      Bypass transmitter
 *      Let packets bypass the destination when the destination is not
 *      available, it may be only used in transparent cache cluster.
 */
int
ip_vs_bypass_xmit(struct sk_buff *skb, struct ip_vs_conn *cp,
		  struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	struct iphdr  *iph = ip_hdr(skb);

	EnterFunction(10);

	rcu_read_lock();
	if (__ip_vs_get_out_rt(cp->af, skb, NULL, iph->daddr,
			       IP_VS_RT_MODE_NON_LOCAL, NULL, ipvsh) < 0)
		goto tx_error;

	ip_send_check(iph);

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	ip_vs_send_or_cont(NFPROTO_IPV4, skb, cp, 0);
	rcu_read_unlock();

	LeaveFunction(10);
	return NF_STOLEN;

 tx_error:
	kfree_skb(skb);
	rcu_read_unlock();
	LeaveFunction(10);
	return NF_STOLEN;
}

#ifdef CONFIG_IP_VS_IPV6
int
ip_vs_bypass_xmit_v6(struct sk_buff *skb, struct ip_vs_conn *cp,
		     struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	EnterFunction(10);

	rcu_read_lock();
	if (__ip_vs_get_out_rt_v6(cp->af, skb, NULL, &ipvsh->daddr.in6, NULL,
				  ipvsh, 0, IP_VS_RT_MODE_NON_LOCAL) < 0)
		goto tx_error;

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	ip_vs_send_or_cont(NFPROTO_IPV6, skb, cp, 0);
	rcu_read_unlock();

	LeaveFunction(10);
	return NF_STOLEN;

 tx_error:
	kfree_skb(skb);
	rcu_read_unlock();
	LeaveFunction(10);
	return NF_STOLEN;
}
#endif

/*
 *      NAT transmitter (only for outside-to-inside nat forwarding)
 *      Not used for related ICMP
 */
int
ip_vs_nat_xmit(struct sk_buff *skb, struct ip_vs_conn *cp,
	       struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	struct rtable *rt;		/* Route to the other host */
	int local, rc, was_input;

	EnterFunction(10);

	rcu_read_lock();
	/* check if it is a connection of no-client-port */
	if (unlikely(cp->flags & IP_VS_CONN_F_NO_CPORT)) {
		__be16 _pt, *p;

		p = skb_header_pointer(skb, ipvsh->len, sizeof(_pt), &_pt);
		if (p == NULL)
			goto tx_error;
		ip_vs_conn_fill_cport(cp, *p);
		IP_VS_DBG(10, "filled cport=%d\n", ntohs(*p));
	}

	was_input = rt_is_input_route(skb_rtable(skb));
	local = __ip_vs_get_out_rt(cp->af, skb, cp->dest, cp->daddr.ip,
				   IP_VS_RT_MODE_LOCAL |
				   IP_VS_RT_MODE_NON_LOCAL |
				   IP_VS_RT_MODE_RDR, NULL, ipvsh);
	if (local < 0)
		goto tx_error;
	rt = skb_rtable(skb);
	/*
	 * Avoid duplicate tuple in reply direction for NAT traffic
	 * to local address when connection is sync-ed
	 */
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	if (cp->flags & IP_VS_CONN_F_SYNC && local) {
		enum ip_conntrack_info ctinfo;
		struct nf_conn *ct = nf_ct_get(skb, &ctinfo);

		if (ct && !nf_ct_is_untracked(ct)) {
			IP_VS_DBG_RL_PKT(10, AF_INET, pp, skb, 0,
					 "ip_vs_nat_xmit(): "
					 "stopping DNAT to local address");
			goto tx_error;
		}
	}
#endif

	/* From world but DNAT to loopback address? */
	if (local && ipv4_is_loopback(cp->daddr.ip) && was_input) {
		IP_VS_DBG_RL_PKT(1, AF_INET, pp, skb, 0, "ip_vs_nat_xmit(): "
				 "stopping DNAT to loopback address");
		goto tx_error;
	}

	/* copy-on-write the packet before mangling it */
	if (!skb_make_writable(skb, sizeof(struct iphdr)))
		goto tx_error;

	if (skb_cow(skb, rt->dst.dev->hard_header_len))
		goto tx_error;

	/* mangle the packet */
	if (pp->dnat_handler && !pp->dnat_handler(skb, pp, cp, ipvsh))
		goto tx_error;
	ip_hdr(skb)->daddr = cp->daddr.ip;
	ip_send_check(ip_hdr(skb));

	IP_VS_DBG_PKT(10, AF_INET, pp, skb, 0, "After DNAT");

	/* FIXME: when application helper enlarges the packet and the length
	   is larger than the MTU of outgoing device, there will be still
	   MTU problem. */

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	rc = ip_vs_nat_send_or_cont(NFPROTO_IPV4, skb, cp, local);
	rcu_read_unlock();

	LeaveFunction(10);
	return rc;

  tx_error:
	kfree_skb(skb);
	rcu_read_unlock();
	LeaveFunction(10);
	return NF_STOLEN;
}

#ifdef CONFIG_IP_VS_IPV6
int
ip_vs_nat_xmit_v6(struct sk_buff *skb, struct ip_vs_conn *cp,
		  struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	struct rt6_info *rt;		/* Route to the other host */
	int local, rc;

	EnterFunction(10);

	rcu_read_lock();
	/* check if it is a connection of no-client-port */
	if (unlikely(cp->flags & IP_VS_CONN_F_NO_CPORT && !ipvsh->fragoffs)) {
		__be16 _pt, *p;
		p = skb_header_pointer(skb, ipvsh->len, sizeof(_pt), &_pt);
		if (p == NULL)
			goto tx_error;
		ip_vs_conn_fill_cport(cp, *p);
		IP_VS_DBG(10, "filled cport=%d\n", ntohs(*p));
	}

	local = __ip_vs_get_out_rt_v6(cp->af, skb, cp->dest, &cp->daddr.in6,
				      NULL, ipvsh, 0,
				      IP_VS_RT_MODE_LOCAL |
				      IP_VS_RT_MODE_NON_LOCAL |
				      IP_VS_RT_MODE_RDR);
	if (local < 0)
		goto tx_error;
	rt = (struct rt6_info *) skb_dst(skb);
	/*
	 * Avoid duplicate tuple in reply direction for NAT traffic
	 * to local address when connection is sync-ed
	 */
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	if (cp->flags & IP_VS_CONN_F_SYNC && local) {
		enum ip_conntrack_info ctinfo;
		struct nf_conn *ct = nf_ct_get(skb, &ctinfo);

		if (ct && !nf_ct_is_untracked(ct)) {
			IP_VS_DBG_RL_PKT(10, AF_INET6, pp, skb, 0,
					 "ip_vs_nat_xmit_v6(): "
					 "stopping DNAT to local address");
			goto tx_error;
		}
	}
#endif

	/* From world but DNAT to loopback address? */
	if (local && skb->dev && !(skb->dev->flags & IFF_LOOPBACK) &&
	    ipv6_addr_type(&rt->rt6i_dst.addr) & IPV6_ADDR_LOOPBACK) {
		IP_VS_DBG_RL_PKT(1, AF_INET6, pp, skb, 0,
				 "ip_vs_nat_xmit_v6(): "
				 "stopping DNAT to loopback address");
		goto tx_error;
	}

	/* copy-on-write the packet before mangling it */
	if (!skb_make_writable(skb, sizeof(struct ipv6hdr)))
		goto tx_error;

	if (skb_cow(skb, rt->dst.dev->hard_header_len))
		goto tx_error;

	/* mangle the packet */
	if (pp->dnat_handler && !pp->dnat_handler(skb, pp, cp, ipvsh))
		goto tx_error;
	ipv6_hdr(skb)->daddr = cp->daddr.in6;

	IP_VS_DBG_PKT(10, AF_INET6, pp, skb, 0, "After DNAT");

	/* FIXME: when application helper enlarges the packet and the length
	   is larger than the MTU of outgoing device, there will be still
	   MTU problem. */

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	rc = ip_vs_nat_send_or_cont(NFPROTO_IPV6, skb, cp, local);
	rcu_read_unlock();

	LeaveFunction(10);
	return rc;

tx_error:
	LeaveFunction(10);
	kfree_skb(skb);
	rcu_read_unlock();
	return NF_STOLEN;
}
#endif

/* When forwarding a packet, we must ensure that we've got enough headroom
 * for the encapsulation packet in the skb.  This also gives us an
 * opportunity to figure out what the payload_len, dsfield, ttl, and df
 * values should be, so that we won't need to look at the old ip header
 * again
 */
static struct sk_buff *
ip_vs_prepare_tunneled_skb(struct sk_buff *skb, int skb_af,
			   unsigned int max_headroom, __u8 *next_protocol,
			   __u32 *payload_len, __u8 *dsfield, __u8 *ttl,
			   __be16 *df)
{
	struct sk_buff *new_skb = NULL;
	struct iphdr *old_iph = NULL;
#ifdef CONFIG_IP_VS_IPV6
	struct ipv6hdr *old_ipv6h = NULL;
#endif

	ip_vs_drop_early_demux_sk(skb);

	if (skb_headroom(skb) < max_headroom || skb_cloned(skb)) {
		new_skb = skb_realloc_headroom(skb, max_headroom);
		if (!new_skb)
			goto error;
		if (skb->sk)
			skb_set_owner_w(new_skb, skb->sk);
		consume_skb(skb);
		skb = new_skb;
	}

#ifdef CONFIG_IP_VS_IPV6
	if (skb_af == AF_INET6) {
		old_ipv6h = ipv6_hdr(skb);
		*next_protocol = IPPROTO_IPV6;
		if (payload_len)
			*payload_len =
				ntohs(old_ipv6h->payload_len) +
				sizeof(*old_ipv6h);
		*dsfield = ipv6_get_dsfield(old_ipv6h);
		*ttl = old_ipv6h->hop_limit;
		if (df)
			*df = 0;
	} else
#endif
	{
		old_iph = ip_hdr(skb);
		/* Copy DF, reset fragment offset and MF */
		if (df)
			*df = (old_iph->frag_off & htons(IP_DF));
		*next_protocol = IPPROTO_IPIP;

		/* fix old IP header checksum */
		ip_send_check(old_iph);
		*dsfield = ipv4_get_dsfield(old_iph);
		*ttl = old_iph->ttl;
		if (payload_len)
			*payload_len = ntohs(old_iph->tot_len);
	}

	return skb;
error:
	kfree_skb(skb);
	return ERR_PTR(-ENOMEM);
}

static inline int __tun_gso_type_mask(int encaps_af, int orig_af)
{
	if (encaps_af == AF_INET) {
		if (orig_af == AF_INET)
			return SKB_GSO_IPIP;

		return SKB_GSO_SIT;
	}

	/* GSO: we need to provide proper SKB_GSO_ value for IPv6:
	 * SKB_GSO_SIT/IPV6
	 */
	return 0;
}

/*
 *   IP Tunneling transmitter
 *
 *   This function encapsulates the packet in a new IP packet, its
 *   destination will be set to cp->daddr. Most code of this function
 *   is taken from ipip.c.
 *
 *   It is used in VS/TUN cluster. The load balancer selects a real
 *   server from a cluster based on a scheduling algorithm,
 *   encapsulates the request packet and forwards it to the selected
 *   server. For example, all real servers are configured with
 *   "ifconfig tunl0 <Virtual IP Address> up". When the server receives
 *   the encapsulated packet, it will decapsulate the packet, processe
 *   the request and return the response packets directly to the client
 *   without passing the load balancer. This can greatly increase the
 *   scalability of virtual server.
 *
 *   Used for ANY protocol
 */
int
ip_vs_tunnel_xmit(struct sk_buff *skb, struct ip_vs_conn *cp,
		  struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	struct net *net = skb_net(skb);
	struct netns_ipvs *ipvs = net_ipvs(net);
	struct rtable *rt;			/* Route to the other host */
	__be32 saddr;				/* Source for tunnel */
	struct net_device *tdev;		/* Device to other host */
	__u8 next_protocol = 0;
	__u8 dsfield = 0;
	__u8 ttl = 0;
	__be16 df = 0;
	__be16 *dfp = NULL;
	struct iphdr  *iph;			/* Our new IP header */
	unsigned int max_headroom;		/* The extra header space needed */
	int ret, local;

	EnterFunction(10);

	rcu_read_lock();
	local = __ip_vs_get_out_rt(cp->af, skb, cp->dest, cp->daddr.ip,
				   IP_VS_RT_MODE_LOCAL |
				   IP_VS_RT_MODE_NON_LOCAL |
				   IP_VS_RT_MODE_CONNECT |
				   IP_VS_RT_MODE_TUNNEL, &saddr, ipvsh);
	if (local < 0)
		goto tx_error;
	if (local) {
		rcu_read_unlock();
		return ip_vs_send_or_cont(NFPROTO_IPV4, skb, cp, 1);
	}

	rt = skb_rtable(skb);
	tdev = rt->dst.dev;

	/*
	 * Okay, now see if we can stuff it in the buffer as-is.
	 */
	max_headroom = LL_RESERVED_SPACE(tdev) + sizeof(struct iphdr);

	/* We only care about the df field if sysctl_pmtu_disc(ipvs) is set */
	dfp = sysctl_pmtu_disc(ipvs) ? &df : NULL;
	skb = ip_vs_prepare_tunneled_skb(skb, cp->af, max_headroom,
					 &next_protocol, NULL, &dsfield,
					 &ttl, dfp);
	if (IS_ERR(skb))
		goto tx_error;

	skb = iptunnel_handle_offloads(
		skb, false, __tun_gso_type_mask(AF_INET, cp->af));
	if (IS_ERR(skb))
		goto tx_error;

	skb->transport_header = skb->network_header;

	skb_push(skb, sizeof(struct iphdr));
	skb_reset_network_header(skb);
	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));

	/*
	 *	Push down and install the IPIP header.
	 */
	iph			=	ip_hdr(skb);
	iph->version		=	4;
	iph->ihl		=	sizeof(struct iphdr)>>2;
	iph->frag_off		=	df;
	iph->protocol		=	next_protocol;
	iph->tos		=	dsfield;
	iph->daddr		=	cp->daddr.ip;
	iph->saddr		=	saddr;
	iph->ttl		=	ttl;
	ip_select_ident(net, skb, NULL);

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	ret = ip_vs_tunnel_xmit_prepare(skb, cp);
	if (ret == NF_ACCEPT)
		ip_local_out(skb);
	else if (ret == NF_DROP)
		kfree_skb(skb);
	rcu_read_unlock();

	LeaveFunction(10);

	return NF_STOLEN;

  tx_error:
	if (!IS_ERR(skb))
		kfree_skb(skb);
	rcu_read_unlock();
	LeaveFunction(10);
	return NF_STOLEN;
}

#ifdef CONFIG_IP_VS_IPV6
int
ip_vs_tunnel_xmit_v6(struct sk_buff *skb, struct ip_vs_conn *cp,
		     struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	struct rt6_info *rt;		/* Route to the other host */
	struct in6_addr saddr;		/* Source for tunnel */
	struct net_device *tdev;	/* Device to other host */
	__u8 next_protocol = 0;
	__u32 payload_len = 0;
	__u8 dsfield = 0;
	__u8 ttl = 0;
	struct ipv6hdr  *iph;		/* Our new IP header */
	unsigned int max_headroom;	/* The extra header space needed */
	int ret, local;

	EnterFunction(10);

	rcu_read_lock();
	local = __ip_vs_get_out_rt_v6(cp->af, skb, cp->dest, &cp->daddr.in6,
				      &saddr, ipvsh, 1,
				      IP_VS_RT_MODE_LOCAL |
				      IP_VS_RT_MODE_NON_LOCAL |
				      IP_VS_RT_MODE_TUNNEL);
	if (local < 0)
		goto tx_error;
	if (local) {
		rcu_read_unlock();
		return ip_vs_send_or_cont(NFPROTO_IPV6, skb, cp, 1);
	}

	rt = (struct rt6_info *) skb_dst(skb);
	tdev = rt->dst.dev;

	/*
	 * Okay, now see if we can stuff it in the buffer as-is.
	 */
	max_headroom = LL_RESERVED_SPACE(tdev) + sizeof(struct ipv6hdr);

	skb = ip_vs_prepare_tunneled_skb(skb, cp->af, max_headroom,
					 &next_protocol, &payload_len,
					 &dsfield, &ttl, NULL);
	if (IS_ERR(skb))
		goto tx_error;

	skb = iptunnel_handle_offloads(
		skb, false, __tun_gso_type_mask(AF_INET6, cp->af));
	if (IS_ERR(skb))
		goto tx_error;

	skb->transport_header = skb->network_header;

	skb_push(skb, sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));

	/*
	 *	Push down and install the IPIP header.
	 */
	iph			=	ipv6_hdr(skb);
	iph->version		=	6;
	iph->nexthdr		=	next_protocol;
	iph->payload_len	=	htons(payload_len);
	memset(&iph->flow_lbl, 0, sizeof(iph->flow_lbl));
	ipv6_change_dsfield(iph, 0, dsfield);
	iph->daddr = cp->daddr.in6;
	iph->saddr = saddr;
	iph->hop_limit		=	ttl;

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	ret = ip_vs_tunnel_xmit_prepare(skb, cp);
	if (ret == NF_ACCEPT)
		ip6_local_out(skb);
	else if (ret == NF_DROP)
		kfree_skb(skb);
	rcu_read_unlock();

	LeaveFunction(10);

	return NF_STOLEN;

tx_error:
	if (!IS_ERR(skb))
		kfree_skb(skb);
	rcu_read_unlock();
	LeaveFunction(10);
	return NF_STOLEN;
}
#endif


/*
 *      Direct Routing transmitter
 *      Used for ANY protocol
 */
int
ip_vs_dr_xmit(struct sk_buff *skb, struct ip_vs_conn *cp,
	      struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	int local;

	EnterFunction(10);

	rcu_read_lock();
	local = __ip_vs_get_out_rt(cp->af, skb, cp->dest, cp->daddr.ip,
				   IP_VS_RT_MODE_LOCAL |
				   IP_VS_RT_MODE_NON_LOCAL |
				   IP_VS_RT_MODE_KNOWN_NH, NULL, ipvsh);
	if (local < 0)
		goto tx_error;
	if (local) {
		rcu_read_unlock();
		return ip_vs_send_or_cont(NFPROTO_IPV4, skb, cp, 1);
	}

	ip_send_check(ip_hdr(skb));

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	ip_vs_send_or_cont(NFPROTO_IPV4, skb, cp, 0);
	rcu_read_unlock();

	LeaveFunction(10);
	return NF_STOLEN;

  tx_error:
	kfree_skb(skb);
	rcu_read_unlock();
	LeaveFunction(10);
	return NF_STOLEN;
}

#ifdef CONFIG_IP_VS_IPV6
int
ip_vs_dr_xmit_v6(struct sk_buff *skb, struct ip_vs_conn *cp,
		 struct ip_vs_protocol *pp, struct ip_vs_iphdr *ipvsh)
{
	int local;

	EnterFunction(10);

	rcu_read_lock();
	local = __ip_vs_get_out_rt_v6(cp->af, skb, cp->dest, &cp->daddr.in6,
				      NULL, ipvsh, 0,
				      IP_VS_RT_MODE_LOCAL |
				      IP_VS_RT_MODE_NON_LOCAL);
	if (local < 0)
		goto tx_error;
	if (local) {
		rcu_read_unlock();
		return ip_vs_send_or_cont(NFPROTO_IPV6, skb, cp, 1);
	}

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	ip_vs_send_or_cont(NFPROTO_IPV6, skb, cp, 0);
	rcu_read_unlock();

	LeaveFunction(10);
	return NF_STOLEN;

tx_error:
	kfree_skb(skb);
	rcu_read_unlock();
	LeaveFunction(10);
	return NF_STOLEN;
}
#endif


/*
 *	ICMP packet transmitter
 *	called by the ip_vs_in_icmp
 */
int
ip_vs_icmp_xmit(struct sk_buff *skb, struct ip_vs_conn *cp,
		struct ip_vs_protocol *pp, int offset, unsigned int hooknum,
		struct ip_vs_iphdr *iph)
{
	struct rtable	*rt;	/* Route to the other host */
	int rc;
	int local;
	int rt_mode, was_input;

	EnterFunction(10);

	/* The ICMP packet for VS/TUN, VS/DR and LOCALNODE will be
	   forwarded directly here, because there is no need to
	   translate address/port back */
	if (IP_VS_FWD_METHOD(cp) != IP_VS_CONN_F_MASQ) {
		if (cp->packet_xmit)
			rc = cp->packet_xmit(skb, cp, pp, iph);
		else
			rc = NF_ACCEPT;
		/* do not touch skb anymore */
		atomic_inc(&cp->in_pkts);
		goto out;
	}

	/*
	 * mangle and send the packet here (only for VS/NAT)
	 */
	was_input = rt_is_input_route(skb_rtable(skb));

	/* LOCALNODE from FORWARD hook is not supported */
	rt_mode = (hooknum != NF_INET_FORWARD) ?
		  IP_VS_RT_MODE_LOCAL | IP_VS_RT_MODE_NON_LOCAL |
		  IP_VS_RT_MODE_RDR : IP_VS_RT_MODE_NON_LOCAL;
	rcu_read_lock();
	local = __ip_vs_get_out_rt(cp->af, skb, cp->dest, cp->daddr.ip, rt_mode,
				   NULL, iph);
	if (local < 0)
		goto tx_error;
	rt = skb_rtable(skb);

	/*
	 * Avoid duplicate tuple in reply direction for NAT traffic
	 * to local address when connection is sync-ed
	 */
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	if (cp->flags & IP_VS_CONN_F_SYNC && local) {
		enum ip_conntrack_info ctinfo;
		struct nf_conn *ct = nf_ct_get(skb, &ctinfo);

		if (ct && !nf_ct_is_untracked(ct)) {
			IP_VS_DBG(10, "%s(): "
				  "stopping DNAT to local address %pI4\n",
				  __func__, &cp->daddr.ip);
			goto tx_error;
		}
	}
#endif

	/* From world but DNAT to loopback address? */
	if (local && ipv4_is_loopback(cp->daddr.ip) && was_input) {
		IP_VS_DBG(1, "%s(): "
			  "stopping DNAT to loopback %pI4\n",
			  __func__, &cp->daddr.ip);
		goto tx_error;
	}

	/* copy-on-write the packet before mangling it */
	if (!skb_make_writable(skb, offset))
		goto tx_error;

	if (skb_cow(skb, rt->dst.dev->hard_header_len))
		goto tx_error;

	ip_vs_nat_icmp(skb, pp, cp, 0);

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	rc = ip_vs_nat_send_or_cont(NFPROTO_IPV4, skb, cp, local);
	rcu_read_unlock();
	goto out;

  tx_error:
	kfree_skb(skb);
	rcu_read_unlock();
	rc = NF_STOLEN;
  out:
	LeaveFunction(10);
	return rc;
}

#ifdef CONFIG_IP_VS_IPV6
int
ip_vs_icmp_xmit_v6(struct sk_buff *skb, struct ip_vs_conn *cp,
		struct ip_vs_protocol *pp, int offset, unsigned int hooknum,
		struct ip_vs_iphdr *ipvsh)
{
	struct rt6_info	*rt;	/* Route to the other host */
	int rc;
	int local;
	int rt_mode;

	EnterFunction(10);

	/* The ICMP packet for VS/TUN, VS/DR and LOCALNODE will be
	   forwarded directly here, because there is no need to
	   translate address/port back */
	if (IP_VS_FWD_METHOD(cp) != IP_VS_CONN_F_MASQ) {
		if (cp->packet_xmit)
			rc = cp->packet_xmit(skb, cp, pp, ipvsh);
		else
			rc = NF_ACCEPT;
		/* do not touch skb anymore */
		atomic_inc(&cp->in_pkts);
		goto out;
	}

	/*
	 * mangle and send the packet here (only for VS/NAT)
	 */

	/* LOCALNODE from FORWARD hook is not supported */
	rt_mode = (hooknum != NF_INET_FORWARD) ?
		  IP_VS_RT_MODE_LOCAL | IP_VS_RT_MODE_NON_LOCAL |
		  IP_VS_RT_MODE_RDR : IP_VS_RT_MODE_NON_LOCAL;
	rcu_read_lock();
	local = __ip_vs_get_out_rt_v6(cp->af, skb, cp->dest, &cp->daddr.in6,
				      NULL, ipvsh, 0, rt_mode);
	if (local < 0)
		goto tx_error;
	rt = (struct rt6_info *) skb_dst(skb);
	/*
	 * Avoid duplicate tuple in reply direction for NAT traffic
	 * to local address when connection is sync-ed
	 */
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	if (cp->flags & IP_VS_CONN_F_SYNC && local) {
		enum ip_conntrack_info ctinfo;
		struct nf_conn *ct = nf_ct_get(skb, &ctinfo);

		if (ct && !nf_ct_is_untracked(ct)) {
			IP_VS_DBG(10, "%s(): "
				  "stopping DNAT to local address %pI6\n",
				  __func__, &cp->daddr.in6);
			goto tx_error;
		}
	}
#endif

	/* From world but DNAT to loopback address? */
	if (local && skb->dev && !(skb->dev->flags & IFF_LOOPBACK) &&
	    ipv6_addr_type(&rt->rt6i_dst.addr) & IPV6_ADDR_LOOPBACK) {
		IP_VS_DBG(1, "%s(): "
			  "stopping DNAT to loopback %pI6\n",
			  __func__, &cp->daddr.in6);
		goto tx_error;
	}

	/* copy-on-write the packet before mangling it */
	if (!skb_make_writable(skb, offset))
		goto tx_error;

	if (skb_cow(skb, rt->dst.dev->hard_header_len))
		goto tx_error;

	ip_vs_nat_icmp_v6(skb, pp, cp, 0);

	/* Another hack: avoid icmp_send in ip_fragment */
	skb->ignore_df = 1;

	rc = ip_vs_nat_send_or_cont(NFPROTO_IPV6, skb, cp, local);
	rcu_read_unlock();
	goto out;

tx_error:
	kfree_skb(skb);
	rcu_read_unlock();
	rc = NF_STOLEN;
out:
	LeaveFunction(10);
	return rc;
}
#endif
