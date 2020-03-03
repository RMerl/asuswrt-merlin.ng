/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		ROUTE - implementation of the IP router.
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Alan Cox, <gw4pts@gw4pts.ampr.org>
 *		Linus Torvalds, <Linus.Torvalds@helsinki.fi>
 *		Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 * Fixes:
 *		Alan Cox	:	Verify area fixes.
 *		Alan Cox	:	cli() protects routing changes
 *		Rui Oliveira	:	ICMP routing table updates
 *		(rco@di.uminho.pt)	Routing table insertion and update
 *		Linus Torvalds	:	Rewrote bits to be sensible
 *		Alan Cox	:	Added BSD route gw semantics
 *		Alan Cox	:	Super /proc >4K
 *		Alan Cox	:	MTU in route table
 *		Alan Cox	: 	MSS actually. Also added the window
 *					clamper.
 *		Sam Lantinga	:	Fixed route matching in rt_del()
 *		Alan Cox	:	Routing cache support.
 *		Alan Cox	:	Removed compatibility cruft.
 *		Alan Cox	:	RTF_REJECT support.
 *		Alan Cox	:	TCP irtt support.
 *		Jonathan Naylor	:	Added Metric support.
 *	Miquel van Smoorenburg	:	BSD API fixes.
 *	Miquel van Smoorenburg	:	Metrics.
 *		Alan Cox	:	Use __u32 properly
 *		Alan Cox	:	Aligned routing errors more closely with BSD
 *					our system is still very different.
 *		Alan Cox	:	Faster /proc handling
 *	Alexey Kuznetsov	:	Massive rework to support tree based routing,
 *					routing caches and better behaviour.
 *
 *		Olaf Erb	:	irtt wasn't being copied right.
 *		Bjorn Ekwall	:	Kerneld route support.
 *		Alan Cox	:	Multicast fixed (I hope)
 * 		Pavel Krauz	:	Limited broadcast fixed
 *		Mike McLagan	:	Routing by source
 *	Alexey Kuznetsov	:	End of old history. Split to fib.c and
 *					route.c and rewritten from scratch.
 *		Andi Kleen	:	Load-limit warning messages.
 *	Vitaly E. Lavrov	:	Transparent proxy revived after year coma.
 *	Vitaly E. Lavrov	:	Race condition in ip_route_input_slow.
 *	Tobias Ringstrom	:	Uninitialized res.type in ip_route_output_slow.
 *	Vladimir V. Ivanov	:	IP rule info (flowid) is really useful.
 *		Marc Boucher	:	routing by fwmark
 *	Robert Olsson		:	Added rt_cache statistics
 *	Arnaldo C. Melo		:	Convert proc stuff to seq_file
 *	Eric Dumazet		:	hashed spinlocks and rt_check_expire() fixes.
 * 	Ilia Sotnikov		:	Ignore TOS on PMTUD and Redirect
 * 	Ilia Sotnikov		:	Removed TOS from hash calculations
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#define pr_fmt(fmt) "IPv4: " fmt

#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/igmp.h>
#include <linux/pkt_sched.h>
#include <linux/mroute.h>
#include <linux/netfilter_ipv4.h>
#include <linux/random.h>
#include <linux/rcupdate.h>
#include <linux/times.h>
#include <linux/slab.h>
#include <linux/jhash.h>
#include <net/dst.h>
#include <net/net_namespace.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/inetpeer.h>
#include <net/sock.h>
#include <net/ip_fib.h>
#include <net/arp.h>
#include <net/tcp.h>
#include <net/icmp.h>
#include <net/xfrm.h>
#include <net/netevent.h>
#include <net/rtnetlink.h>
#ifdef CONFIG_SYSCTL
#include <linux/sysctl.h>
#include <linux/kmemleak.h>
#endif
#include <net/secure_seq.h>

#define RT_FL_TOS(oldflp4) \
	((oldflp4)->flowi4_tos & (IPTOS_RT_MASK | RTO_ONLINK))

#define RT_GC_TIMEOUT (300*HZ)

static int ip_rt_max_size;
static int ip_rt_redirect_number __read_mostly	= 9;
static int ip_rt_redirect_load __read_mostly	= HZ / 50;
static int ip_rt_redirect_silence __read_mostly	= ((HZ / 50) << (9 + 1));
static int ip_rt_error_cost __read_mostly	= HZ;
static int ip_rt_error_burst __read_mostly	= 5 * HZ;
static int ip_rt_mtu_expires __read_mostly	= 10 * 60 * HZ;
static u32 ip_rt_min_pmtu __read_mostly		= 512 + 20 + 20;
static int ip_rt_min_advmss __read_mostly	= 256;

static int ip_rt_gc_timeout __read_mostly	= RT_GC_TIMEOUT;

static int ip_min_valid_pmtu __read_mostly	= IPV4_MIN_MTU;

/*
 *	Interface to generic destination cache.
 */

static struct dst_entry *ipv4_dst_check(struct dst_entry *dst, u32 cookie);
static unsigned int	 ipv4_default_advmss(const struct dst_entry *dst);
static unsigned int	 ipv4_mtu(const struct dst_entry *dst);
static struct dst_entry *ipv4_negative_advice(struct dst_entry *dst);
static void		 ipv4_link_failure(struct sk_buff *skb);
static void		 ip_rt_update_pmtu(struct dst_entry *dst, struct sock *sk,
					   struct sk_buff *skb, u32 mtu);
static void		 ip_do_redirect(struct dst_entry *dst, struct sock *sk,
					struct sk_buff *skb);
static void		ipv4_dst_destroy(struct dst_entry *dst);

static u32 *ipv4_cow_metrics(struct dst_entry *dst, unsigned long old)
{
	WARN_ON(1);
	return NULL;
}

static struct neighbour *ipv4_neigh_lookup(const struct dst_entry *dst,
					   struct sk_buff *skb,
					   const void *daddr);

static struct dst_ops ipv4_dst_ops = {
	.family =		AF_INET,
	.check =		ipv4_dst_check,
	.default_advmss =	ipv4_default_advmss,
	.mtu =			ipv4_mtu,
	.cow_metrics =		ipv4_cow_metrics,
	.destroy =		ipv4_dst_destroy,
	.negative_advice =	ipv4_negative_advice,
	.link_failure =		ipv4_link_failure,
	.update_pmtu =		ip_rt_update_pmtu,
	.redirect =		ip_do_redirect,
	.local_out =		__ip_local_out,
	.neigh_lookup =		ipv4_neigh_lookup,
};

#define ECN_OR_COST(class)	TC_PRIO_##class

const __u8 ip_tos2prio[16] = {
	TC_PRIO_BESTEFFORT,
	ECN_OR_COST(BESTEFFORT),
	TC_PRIO_BESTEFFORT,
	ECN_OR_COST(BESTEFFORT),
	TC_PRIO_BULK,
	ECN_OR_COST(BULK),
	TC_PRIO_BULK,
	ECN_OR_COST(BULK),
	TC_PRIO_INTERACTIVE,
	ECN_OR_COST(INTERACTIVE),
	TC_PRIO_INTERACTIVE,
	ECN_OR_COST(INTERACTIVE),
	TC_PRIO_INTERACTIVE_BULK,
	ECN_OR_COST(INTERACTIVE_BULK),
	TC_PRIO_INTERACTIVE_BULK,
	ECN_OR_COST(INTERACTIVE_BULK)
};
EXPORT_SYMBOL(ip_tos2prio);

static DEFINE_PER_CPU(struct rt_cache_stat, rt_cache_stat);
#define RT_CACHE_STAT_INC(field) raw_cpu_inc(rt_cache_stat.field)

#ifdef CONFIG_PROC_FS
static void *rt_cache_seq_start(struct seq_file *seq, loff_t *pos)
{
	if (*pos)
		return NULL;
	return SEQ_START_TOKEN;
}

static void *rt_cache_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void rt_cache_seq_stop(struct seq_file *seq, void *v)
{
}

static int rt_cache_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN)
		seq_printf(seq, "%-127s\n",
			   "Iface\tDestination\tGateway \tFlags\t\tRefCnt\tUse\t"
			   "Metric\tSource\t\tMTU\tWindow\tIRTT\tTOS\tHHRef\t"
			   "HHUptod\tSpecDst");
	return 0;
}

static const struct seq_operations rt_cache_seq_ops = {
	.start  = rt_cache_seq_start,
	.next   = rt_cache_seq_next,
	.stop   = rt_cache_seq_stop,
	.show   = rt_cache_seq_show,
};

static int rt_cache_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &rt_cache_seq_ops);
}

static const struct file_operations rt_cache_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rt_cache_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release,
};


static void *rt_cpu_seq_start(struct seq_file *seq, loff_t *pos)
{
	int cpu;

	if (*pos == 0)
		return SEQ_START_TOKEN;

	for (cpu = *pos-1; cpu < nr_cpu_ids; ++cpu) {
		if (!cpu_possible(cpu))
			continue;
		*pos = cpu+1;
		return &per_cpu(rt_cache_stat, cpu);
	}
	return NULL;
}

static void *rt_cpu_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	int cpu;

	for (cpu = *pos; cpu < nr_cpu_ids; ++cpu) {
		if (!cpu_possible(cpu))
			continue;
		*pos = cpu+1;
		return &per_cpu(rt_cache_stat, cpu);
	}
	return NULL;

}

static void rt_cpu_seq_stop(struct seq_file *seq, void *v)
{

}

static int rt_cpu_seq_show(struct seq_file *seq, void *v)
{
	struct rt_cache_stat *st = v;

	if (v == SEQ_START_TOKEN) {
		seq_printf(seq, "entries  in_hit in_slow_tot in_slow_mc in_no_route in_brd in_martian_dst in_martian_src  out_hit out_slow_tot out_slow_mc  gc_total gc_ignored gc_goal_miss gc_dst_overflow in_hlist_search out_hlist_search\n");
		return 0;
	}

	seq_printf(seq,"%08x  %08x %08x %08x %08x %08x %08x %08x "
		   " %08x %08x %08x %08x %08x %08x %08x %08x %08x \n",
		   dst_entries_get_slow(&ipv4_dst_ops),
		   0, /* st->in_hit */
		   st->in_slow_tot,
		   st->in_slow_mc,
		   st->in_no_route,
		   st->in_brd,
		   st->in_martian_dst,
		   st->in_martian_src,

		   0, /* st->out_hit */
		   st->out_slow_tot,
		   st->out_slow_mc,

		   0, /* st->gc_total */
		   0, /* st->gc_ignored */
		   0, /* st->gc_goal_miss */
		   0, /* st->gc_dst_overflow */
		   0, /* st->in_hlist_search */
		   0  /* st->out_hlist_search */
		);
	return 0;
}

static const struct seq_operations rt_cpu_seq_ops = {
	.start  = rt_cpu_seq_start,
	.next   = rt_cpu_seq_next,
	.stop   = rt_cpu_seq_stop,
	.show   = rt_cpu_seq_show,
};


static int rt_cpu_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &rt_cpu_seq_ops);
}

static const struct file_operations rt_cpu_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rt_cpu_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release,
};

#ifdef CONFIG_IP_ROUTE_CLASSID
static int rt_acct_proc_show(struct seq_file *m, void *v)
{
	struct ip_rt_acct *dst, *src;
	unsigned int i, j;

	dst = kcalloc(256, sizeof(struct ip_rt_acct), GFP_KERNEL);
	if (!dst)
		return -ENOMEM;

	for_each_possible_cpu(i) {
		src = (struct ip_rt_acct *)per_cpu_ptr(ip_rt_acct, i);
		for (j = 0; j < 256; j++) {
			dst[j].o_bytes   += src[j].o_bytes;
			dst[j].o_packets += src[j].o_packets;
			dst[j].i_bytes   += src[j].i_bytes;
			dst[j].i_packets += src[j].i_packets;
		}
	}

	seq_write(m, dst, 256 * sizeof(struct ip_rt_acct));
	kfree(dst);
	return 0;
}

static int rt_acct_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rt_acct_proc_show, NULL);
}

static const struct file_operations rt_acct_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= rt_acct_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int __net_init ip_rt_do_proc_init(struct net *net)
{
	struct proc_dir_entry *pde;

	pde = proc_create("rt_cache", S_IRUGO, net->proc_net,
			  &rt_cache_seq_fops);
	if (!pde)
		goto err1;

	pde = proc_create("rt_cache", S_IRUGO,
			  net->proc_net_stat, &rt_cpu_seq_fops);
	if (!pde)
		goto err2;

#ifdef CONFIG_IP_ROUTE_CLASSID
	pde = proc_create("rt_acct", 0, net->proc_net, &rt_acct_proc_fops);
	if (!pde)
		goto err3;
#endif
	return 0;

#ifdef CONFIG_IP_ROUTE_CLASSID
err3:
	remove_proc_entry("rt_cache", net->proc_net_stat);
#endif
err2:
	remove_proc_entry("rt_cache", net->proc_net);
err1:
	return -ENOMEM;
}

static void __net_exit ip_rt_do_proc_exit(struct net *net)
{
	remove_proc_entry("rt_cache", net->proc_net_stat);
	remove_proc_entry("rt_cache", net->proc_net);
#ifdef CONFIG_IP_ROUTE_CLASSID
	remove_proc_entry("rt_acct", net->proc_net);
#endif
}

static struct pernet_operations ip_rt_proc_ops __net_initdata =  {
	.init = ip_rt_do_proc_init,
	.exit = ip_rt_do_proc_exit,
};

static int __init ip_rt_proc_init(void)
{
	return register_pernet_subsys(&ip_rt_proc_ops);
}

#else
static inline int ip_rt_proc_init(void)
{
	return 0;
}
#endif /* CONFIG_PROC_FS */

static inline bool rt_is_expired(const struct rtable *rth)
{
	return rth->rt_genid != rt_genid_ipv4(dev_net(rth->dst.dev));
}

void rt_cache_flush(struct net *net)
{
	rt_genid_bump_ipv4(net);
}

static struct neighbour *ipv4_neigh_lookup(const struct dst_entry *dst,
					   struct sk_buff *skb,
					   const void *daddr)
{
	struct net_device *dev = dst->dev;
	const __be32 *pkey = daddr;
	const struct rtable *rt;
	struct neighbour *n;

	rt = (const struct rtable *) dst;
	if (rt->rt_gateway)
		pkey = (const __be32 *) &rt->rt_gateway;
	else if (skb)
		pkey = &ip_hdr(skb)->daddr;

	n = __ipv4_neigh_lookup(dev, *(__force u32 *)pkey);
	if (n)
		return n;
	return neigh_create(&arp_tbl, pkey, dev);
}

#define IP_IDENTS_SZ 2048u
struct ip_ident_bucket {
	atomic_t	id;
	u32		stamp32;
};

static struct ip_ident_bucket *ip_idents __read_mostly;

/* In order to protect privacy, we add a perturbation to identifiers
 * if one generator is seldom used. This makes hard for an attacker
 * to infer how many packets were sent between two points in time.
 */
u32 ip_idents_reserve(u32 hash, int segs)
{
	struct ip_ident_bucket *bucket = ip_idents + hash % IP_IDENTS_SZ;
	u32 old = ACCESS_ONCE(bucket->stamp32);
	u32 now = (u32)jiffies;
	u32 delta = 0;

	if (old != now && cmpxchg(&bucket->stamp32, old, now) == old)
		delta = prandom_u32_max(now - old);

	return atomic_add_return(segs + delta, &bucket->id) - segs;
}
EXPORT_SYMBOL(ip_idents_reserve);

void __ip_select_ident(struct net *net, struct iphdr *iph, int segs)
{
	static u32 ip_idents_hashrnd __read_mostly;
	u32 hash, id;

	net_get_random_once(&ip_idents_hashrnd, sizeof(ip_idents_hashrnd));

	hash = jhash_3words((__force u32)iph->daddr,
			    (__force u32)iph->saddr,
			    iph->protocol ^ net_hash_mix(net),
			    ip_idents_hashrnd);
	id = ip_idents_reserve(hash, segs);
	iph->id = htons(id);
}
EXPORT_SYMBOL(__ip_select_ident);

static void __build_flow_key(struct flowi4 *fl4, const struct sock *sk,
			     const struct iphdr *iph,
			     int oif, u8 tos,
			     u8 prot, u32 mark, int flow_flags)
{
	if (sk) {
		const struct inet_sock *inet = inet_sk(sk);

		oif = sk->sk_bound_dev_if;
		mark = sk->sk_mark;
		tos = RT_CONN_FLAGS(sk);
		prot = inet->hdrincl ? IPPROTO_RAW : sk->sk_protocol;
	}
	flowi4_init_output(fl4, oif, mark, tos,
			   RT_SCOPE_UNIVERSE, prot,
			   flow_flags,
			   iph->daddr, iph->saddr, 0, 0);
}

static void build_skb_flow_key(struct flowi4 *fl4, const struct sk_buff *skb,
			       const struct sock *sk)
{
	const struct iphdr *iph = ip_hdr(skb);
	int oif = skb->dev->ifindex;
	u8 tos = RT_TOS(iph->tos);
	u8 prot = iph->protocol;
	u32 mark = skb->mark;

	__build_flow_key(fl4, sk, iph, oif, tos, prot, mark, 0);
}

static void build_sk_flow_key(struct flowi4 *fl4, const struct sock *sk)
{
	const struct inet_sock *inet = inet_sk(sk);
	const struct ip_options_rcu *inet_opt;
	__be32 daddr = inet->inet_daddr;

	rcu_read_lock();
	inet_opt = rcu_dereference(inet->inet_opt);
	if (inet_opt && inet_opt->opt.srr)
		daddr = inet_opt->opt.faddr;
	flowi4_init_output(fl4, sk->sk_bound_dev_if, sk->sk_mark,
			   RT_CONN_FLAGS(sk), RT_SCOPE_UNIVERSE,
			   inet->hdrincl ? IPPROTO_RAW : sk->sk_protocol,
			   inet_sk_flowi_flags(sk),
			   daddr, inet->inet_saddr, 0, 0);
	rcu_read_unlock();
}

static void ip_rt_build_flow_key(struct flowi4 *fl4, const struct sock *sk,
				 const struct sk_buff *skb)
{
	if (skb)
		build_skb_flow_key(fl4, skb, sk);
	else
		build_sk_flow_key(fl4, sk);
}

static inline void rt_free(struct rtable *rt)
{
	call_rcu(&rt->dst.rcu_head, dst_rcu_free);
}

static DEFINE_SPINLOCK(fnhe_lock);

static void fnhe_flush_routes(struct fib_nh_exception *fnhe)
{
	struct rtable *rt;

	rt = rcu_dereference(fnhe->fnhe_rth_input);
	if (rt) {
		RCU_INIT_POINTER(fnhe->fnhe_rth_input, NULL);
		rt_free(rt);
	}
	rt = rcu_dereference(fnhe->fnhe_rth_output);
	if (rt) {
		RCU_INIT_POINTER(fnhe->fnhe_rth_output, NULL);
		rt_free(rt);
	}
}

static struct fib_nh_exception *fnhe_oldest(struct fnhe_hash_bucket *hash)
{
	struct fib_nh_exception *fnhe, *oldest;

	oldest = rcu_dereference(hash->chain);
	for (fnhe = rcu_dereference(oldest->fnhe_next); fnhe;
	     fnhe = rcu_dereference(fnhe->fnhe_next)) {
		if (time_before(fnhe->fnhe_stamp, oldest->fnhe_stamp))
			oldest = fnhe;
	}
	fnhe_flush_routes(oldest);
	return oldest;
}

static inline u32 fnhe_hashfun(__be32 daddr)
{
	static u32 fnhe_hashrnd __read_mostly;
	u32 hval;

	net_get_random_once(&fnhe_hashrnd, sizeof(fnhe_hashrnd));
	hval = jhash_1word((__force u32) daddr, fnhe_hashrnd);
	return hash_32(hval, FNHE_HASH_SHIFT);
}

static void fill_route_from_fnhe(struct rtable *rt, struct fib_nh_exception *fnhe)
{
	rt->rt_pmtu = fnhe->fnhe_pmtu;
	rt->dst.expires = fnhe->fnhe_expires;

	if (fnhe->fnhe_gw) {
		rt->rt_flags |= RTCF_REDIRECTED;
		rt->rt_gateway = fnhe->fnhe_gw;
		rt->rt_uses_gateway = 1;
	}
}

static void update_or_create_fnhe(struct fib_nh *nh, __be32 daddr, __be32 gw,
				  u32 pmtu, unsigned long expires)
{
	struct fnhe_hash_bucket *hash;
	struct fib_nh_exception *fnhe;
	struct rtable *rt;
	u32 genid, hval;
	unsigned int i;
	int depth;

	genid = fnhe_genid(dev_net(nh->nh_dev));
	hval = fnhe_hashfun(daddr);

	spin_lock_bh(&fnhe_lock);

	hash = rcu_dereference(nh->nh_exceptions);
	if (!hash) {
		hash = kzalloc(FNHE_HASH_SIZE * sizeof(*hash), GFP_ATOMIC);
		if (!hash)
			goto out_unlock;
		rcu_assign_pointer(nh->nh_exceptions, hash);
	}

	hash += hval;

	depth = 0;
	for (fnhe = rcu_dereference(hash->chain); fnhe;
	     fnhe = rcu_dereference(fnhe->fnhe_next)) {
		if (fnhe->fnhe_daddr == daddr)
			break;
		depth++;
	}

	if (fnhe) {
		if (fnhe->fnhe_genid != genid)
			fnhe->fnhe_genid = genid;
		if (gw)
			fnhe->fnhe_gw = gw;
		if (pmtu)
			fnhe->fnhe_pmtu = pmtu;
		fnhe->fnhe_expires = max(1UL, expires);
		/* Update all cached dsts too */
		rt = rcu_dereference(fnhe->fnhe_rth_input);
		if (rt)
			fill_route_from_fnhe(rt, fnhe);
		rt = rcu_dereference(fnhe->fnhe_rth_output);
		if (rt)
			fill_route_from_fnhe(rt, fnhe);
	} else {
		if (depth > FNHE_RECLAIM_DEPTH)
			fnhe = fnhe_oldest(hash);
		else {
			fnhe = kzalloc(sizeof(*fnhe), GFP_ATOMIC);
			if (!fnhe)
				goto out_unlock;

			fnhe->fnhe_next = hash->chain;
			rcu_assign_pointer(hash->chain, fnhe);
		}
		fnhe->fnhe_genid = genid;
		fnhe->fnhe_daddr = daddr;
		fnhe->fnhe_gw = gw;
		fnhe->fnhe_pmtu = pmtu;
		fnhe->fnhe_expires = expires;

		/* Exception created; mark the cached routes for the nexthop
		 * stale, so anyone caching it rechecks if this exception
		 * applies to them.
		 */
		rt = rcu_dereference(nh->nh_rth_input);
		if (rt)
			rt->dst.obsolete = DST_OBSOLETE_KILL;

		for_each_possible_cpu(i) {
			struct rtable __rcu **prt;
			prt = per_cpu_ptr(nh->nh_pcpu_rth_output, i);
			rt = rcu_dereference(*prt);
			if (rt)
				rt->dst.obsolete = DST_OBSOLETE_KILL;
		}
	}

	fnhe->fnhe_stamp = jiffies;

out_unlock:
	spin_unlock_bh(&fnhe_lock);
}

static void __ip_do_redirect(struct rtable *rt, struct sk_buff *skb, struct flowi4 *fl4,
			     bool kill_route)
{
	__be32 new_gw = icmp_hdr(skb)->un.gateway;
	__be32 old_gw = ip_hdr(skb)->saddr;
	struct net_device *dev = skb->dev;
	struct in_device *in_dev;
	struct fib_result res;
	struct neighbour *n;
	struct net *net;

	switch (icmp_hdr(skb)->code & 7) {
	case ICMP_REDIR_NET:
	case ICMP_REDIR_NETTOS:
	case ICMP_REDIR_HOST:
	case ICMP_REDIR_HOSTTOS:
		break;

	default:
		return;
	}

	if (rt->rt_gateway != old_gw)
		return;

	in_dev = __in_dev_get_rcu(dev);
	if (!in_dev)
		return;

	net = dev_net(dev);
	if (new_gw == old_gw || !IN_DEV_RX_REDIRECTS(in_dev) ||
	    ipv4_is_multicast(new_gw) || ipv4_is_lbcast(new_gw) ||
	    ipv4_is_zeronet(new_gw))
		goto reject_redirect;

	if (!IN_DEV_SHARED_MEDIA(in_dev)) {
		if (!inet_addr_onlink(in_dev, new_gw, old_gw))
			goto reject_redirect;
		if (IN_DEV_SEC_REDIRECTS(in_dev) && ip_fib_check_default(new_gw, dev))
			goto reject_redirect;
	} else {
		if (inet_addr_type(net, new_gw) != RTN_UNICAST)
			goto reject_redirect;
	}

	n = ipv4_neigh_lookup(&rt->dst, NULL, &new_gw);
	if (!IS_ERR(n)) {
		if (!(n->nud_state & NUD_VALID)) {
			neigh_event_send(n, NULL);
		} else {
			if (fib_lookup(net, fl4, &res) == 0) {
				struct fib_nh *nh = &FIB_RES_NH(res);

				update_or_create_fnhe(nh, fl4->daddr, new_gw,
						0, jiffies + ip_rt_gc_timeout);
			}
			if (kill_route)
				rt->dst.obsolete = DST_OBSOLETE_KILL;
			call_netevent_notifiers(NETEVENT_NEIGH_UPDATE, n);
		}
		neigh_release(n);
	}
	return;

reject_redirect:
#ifdef CONFIG_IP_ROUTE_VERBOSE
	if (IN_DEV_LOG_MARTIANS(in_dev)) {
		const struct iphdr *iph = (const struct iphdr *) skb->data;
		__be32 daddr = iph->daddr;
		__be32 saddr = iph->saddr;

		net_info_ratelimited("Redirect from %pI4 on %s about %pI4 ignored\n"
				     "  Advised path = %pI4 -> %pI4\n",
				     &old_gw, dev->name, &new_gw,
				     &saddr, &daddr);
	}
#endif
	;
}

static void ip_do_redirect(struct dst_entry *dst, struct sock *sk, struct sk_buff *skb)
{
	struct rtable *rt;
	struct flowi4 fl4;
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	int oif = skb->dev->ifindex;
	u8 tos = RT_TOS(iph->tos);
	u8 prot = iph->protocol;
	u32 mark = skb->mark;

	rt = (struct rtable *) dst;

	__build_flow_key(&fl4, sk, iph, oif, tos, prot, mark, 0);
	__ip_do_redirect(rt, skb, &fl4, true);
}

static struct dst_entry *ipv4_negative_advice(struct dst_entry *dst)
{
	struct rtable *rt = (struct rtable *)dst;
	struct dst_entry *ret = dst;

	if (rt) {
		if (dst->obsolete > 0) {
			ip_rt_put(rt);
			ret = NULL;
		} else if ((rt->rt_flags & RTCF_REDIRECTED) ||
			   rt->dst.expires) {
			ip_rt_put(rt);
			ret = NULL;
		}
	}
	return ret;
}

/*
 * Algorithm:
 *	1. The first ip_rt_redirect_number redirects are sent
 *	   with exponential backoff, then we stop sending them at all,
 *	   assuming that the host ignores our redirects.
 *	2. If we did not see packets requiring redirects
 *	   during ip_rt_redirect_silence, we assume that the host
 *	   forgot redirected route and start to send redirects again.
 *
 * This algorithm is much cheaper and more intelligent than dumb load limiting
 * in icmp.c.
 *
 * NOTE. Do not forget to inhibit load limiting for redirects (redundant)
 * and "frag. need" (breaks PMTU discovery) in icmp.c.
 */

void ip_rt_send_redirect(struct sk_buff *skb)
{
	struct rtable *rt = skb_rtable(skb);
	struct in_device *in_dev;
	struct inet_peer *peer;
	struct net *net;
	int log_martians;

	rcu_read_lock();
	in_dev = __in_dev_get_rcu(rt->dst.dev);
	if (!in_dev || !IN_DEV_TX_REDIRECTS(in_dev)) {
		rcu_read_unlock();
		return;
	}
	log_martians = IN_DEV_LOG_MARTIANS(in_dev);
	rcu_read_unlock();

	net = dev_net(rt->dst.dev);
	peer = inet_getpeer_v4(net->ipv4.peers, ip_hdr(skb)->saddr, 1);
	if (!peer) {
		icmp_send(skb, ICMP_REDIRECT, ICMP_REDIR_HOST,
			  rt_nexthop(rt, ip_hdr(skb)->daddr));
		return;
	}

	/* No redirected packets during ip_rt_redirect_silence;
	 * reset the algorithm.
	 */
	if (time_after(jiffies, peer->rate_last + ip_rt_redirect_silence))
		peer->rate_tokens = 0;

	/* Too many ignored redirects; do not send anything
	 * set dst.rate_last to the last seen redirected packet.
	 */
	if (peer->rate_tokens >= ip_rt_redirect_number) {
		peer->rate_last = jiffies;
		goto out_put_peer;
	}

	/* Check for load limit; set rate_last to the latest sent
	 * redirect.
	 */
	if (peer->rate_tokens == 0 ||
	    time_after(jiffies,
		       (peer->rate_last +
			(ip_rt_redirect_load << peer->rate_tokens)))) {
		__be32 gw = rt_nexthop(rt, ip_hdr(skb)->daddr);

		icmp_send(skb, ICMP_REDIRECT, ICMP_REDIR_HOST, gw);
		peer->rate_last = jiffies;
		++peer->rate_tokens;
#ifdef CONFIG_IP_ROUTE_VERBOSE
		if (log_martians &&
		    peer->rate_tokens == ip_rt_redirect_number)
			net_warn_ratelimited("host %pI4/if%d ignores redirects for %pI4 to %pI4\n",
					     &ip_hdr(skb)->saddr, inet_iif(skb),
					     &ip_hdr(skb)->daddr, &gw);
#endif
	}
out_put_peer:
	inet_putpeer(peer);
}

static int ip_error(struct sk_buff *skb)
{
	struct in_device *in_dev = __in_dev_get_rcu(skb->dev);
	struct rtable *rt = skb_rtable(skb);
	struct inet_peer *peer;
	unsigned long now;
	struct net *net;
	bool send;
	int code;

	/* IP on this device is disabled. */
	if (!in_dev)
		goto out;

	net = dev_net(rt->dst.dev);
	if (!IN_DEV_FORWARD(in_dev)) {
		switch (rt->dst.error) {
		case EHOSTUNREACH:
			IP_INC_STATS_BH(net, IPSTATS_MIB_INADDRERRORS);
			break;

		case ENETUNREACH:
			IP_INC_STATS_BH(net, IPSTATS_MIB_INNOROUTES);
			break;
		}
		goto out;
	}

	switch (rt->dst.error) {
	case EINVAL:
	default:
		goto out;
	case EHOSTUNREACH:
		code = ICMP_HOST_UNREACH;
		break;
	case ENETUNREACH:
		code = ICMP_NET_UNREACH;
		IP_INC_STATS_BH(net, IPSTATS_MIB_INNOROUTES);
		break;
	case EACCES:
		code = ICMP_PKT_FILTERED;
		break;
	}

	peer = inet_getpeer_v4(net->ipv4.peers, ip_hdr(skb)->saddr, 1);

	send = true;
	if (peer) {
		now = jiffies;
		peer->rate_tokens += now - peer->rate_last;
		if (peer->rate_tokens > ip_rt_error_burst)
			peer->rate_tokens = ip_rt_error_burst;
		peer->rate_last = now;
		if (peer->rate_tokens >= ip_rt_error_cost)
			peer->rate_tokens -= ip_rt_error_cost;
		else
			send = false;
		inet_putpeer(peer);
	}
	if (send)
		icmp_send(skb, ICMP_DEST_UNREACH, code, 0);

out:	kfree_skb(skb);
	return 0;
}

static void __ip_rt_update_pmtu(struct rtable *rt, struct flowi4 *fl4, u32 mtu)
{
	struct dst_entry *dst = &rt->dst;
	struct fib_result res;

	if (dst_metric_locked(dst, RTAX_MTU))
		return;

	if (ipv4_mtu(dst) < mtu)
		return;

	if (mtu < ip_rt_min_pmtu)
		mtu = ip_rt_min_pmtu;

	if (rt->rt_pmtu == mtu &&
	    time_before(jiffies, dst->expires - ip_rt_mtu_expires / 2))
		return;

	rcu_read_lock();
	if (fib_lookup(dev_net(dst->dev), fl4, &res) == 0) {
		struct fib_nh *nh = &FIB_RES_NH(res);

		update_or_create_fnhe(nh, fl4->daddr, 0, mtu,
				      jiffies + ip_rt_mtu_expires);
	}
	rcu_read_unlock();
}

static void ip_rt_update_pmtu(struct dst_entry *dst, struct sock *sk,
			      struct sk_buff *skb, u32 mtu)
{
	struct rtable *rt = (struct rtable *) dst;
	struct flowi4 fl4;

	ip_rt_build_flow_key(&fl4, sk, skb);
	__ip_rt_update_pmtu(rt, &fl4, mtu);
}

void ipv4_update_pmtu(struct sk_buff *skb, struct net *net, u32 mtu,
		      int oif, u32 mark, u8 protocol, int flow_flags)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	struct flowi4 fl4;
	struct rtable *rt;

	if (!mark)
		mark = IP4_REPLY_MARK(net, skb->mark);

	__build_flow_key(&fl4, NULL, iph, oif,
			 RT_TOS(iph->tos), protocol, mark, flow_flags);
	rt = __ip_route_output_key(net, &fl4);
	if (!IS_ERR(rt)) {
		__ip_rt_update_pmtu(rt, &fl4, mtu);
		ip_rt_put(rt);
	}
}
EXPORT_SYMBOL_GPL(ipv4_update_pmtu);

static void __ipv4_sk_update_pmtu(struct sk_buff *skb, struct sock *sk, u32 mtu)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	struct flowi4 fl4;
	struct rtable *rt;

	__build_flow_key(&fl4, sk, iph, 0, 0, 0, 0, 0);

	if (!fl4.flowi4_mark)
		fl4.flowi4_mark = IP4_REPLY_MARK(sock_net(sk), skb->mark);

	rt = __ip_route_output_key(sock_net(sk), &fl4);
	if (!IS_ERR(rt)) {
		__ip_rt_update_pmtu(rt, &fl4, mtu);
		ip_rt_put(rt);
	}
}

void ipv4_sk_update_pmtu(struct sk_buff *skb, struct sock *sk, u32 mtu)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	struct flowi4 fl4;
	struct rtable *rt;
	struct dst_entry *odst = NULL;
	bool new = false;

	bh_lock_sock(sk);

	if (!ip_sk_accept_pmtu(sk))
		goto out;

	odst = sk_dst_get(sk);

	if (sock_owned_by_user(sk) || !odst) {
		__ipv4_sk_update_pmtu(skb, sk, mtu);
		goto out;
	}

	__build_flow_key(&fl4, sk, iph, 0, 0, 0, 0, 0);

	rt = (struct rtable *)odst;
	if (odst->obsolete && !odst->ops->check(odst, 0)) {
		rt = ip_route_output_flow(sock_net(sk), &fl4, sk);
		if (IS_ERR(rt))
			goto out;

		new = true;
	}

	__ip_rt_update_pmtu((struct rtable *) rt->dst.path, &fl4, mtu);

	if (!dst_check(&rt->dst, 0)) {
		if (new)
			dst_release(&rt->dst);

		rt = ip_route_output_flow(sock_net(sk), &fl4, sk);
		if (IS_ERR(rt))
			goto out;

		new = true;
	}

	if (new)
		sk_dst_set(sk, &rt->dst);

out:
	bh_unlock_sock(sk);
	dst_release(odst);
}
EXPORT_SYMBOL_GPL(ipv4_sk_update_pmtu);

void ipv4_redirect(struct sk_buff *skb, struct net *net,
		   int oif, u32 mark, u8 protocol, int flow_flags)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	struct flowi4 fl4;
	struct rtable *rt;

	__build_flow_key(&fl4, NULL, iph, oif,
			 RT_TOS(iph->tos), protocol, mark, flow_flags);
	rt = __ip_route_output_key(net, &fl4);
	if (!IS_ERR(rt)) {
		__ip_do_redirect(rt, skb, &fl4, false);
		ip_rt_put(rt);
	}
}
EXPORT_SYMBOL_GPL(ipv4_redirect);

void ipv4_sk_redirect(struct sk_buff *skb, struct sock *sk)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	struct flowi4 fl4;
	struct rtable *rt;

	__build_flow_key(&fl4, sk, iph, 0, 0, 0, 0, 0);
	rt = __ip_route_output_key(sock_net(sk), &fl4);
	if (!IS_ERR(rt)) {
		__ip_do_redirect(rt, skb, &fl4, false);
		ip_rt_put(rt);
	}
}
EXPORT_SYMBOL_GPL(ipv4_sk_redirect);

static struct dst_entry *ipv4_dst_check(struct dst_entry *dst, u32 cookie)
{
	struct rtable *rt = (struct rtable *) dst;

	/* All IPV4 dsts are created with ->obsolete set to the value
	 * DST_OBSOLETE_FORCE_CHK which forces validation calls down
	 * into this function always.
	 *
	 * When a PMTU/redirect information update invalidates a route,
	 * this is indicated by setting obsolete to DST_OBSOLETE_KILL or
	 * DST_OBSOLETE_DEAD by dst_free().
	 */
	if (dst->obsolete != DST_OBSOLETE_FORCE_CHK || rt_is_expired(rt))
		return NULL;
	return dst;
}

static void ipv4_link_failure(struct sk_buff *skb)
{
	struct rtable *rt;

	icmp_send(skb, ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, 0);

	rt = skb_rtable(skb);
	if (rt)
		dst_set_expires(&rt->dst, 0);
}

static int ip_rt_bug(struct sock *sk, struct sk_buff *skb)
{
	pr_debug("%s: %pI4 -> %pI4, %s\n",
		 __func__, &ip_hdr(skb)->saddr, &ip_hdr(skb)->daddr,
		 skb->dev ? skb->dev->name : "?");
	kfree_skb(skb);
	WARN_ON(1);
	return 0;
}

/*
   We do not cache source address of outgoing interface,
   because it is used only by IP RR, TS and SRR options,
   so that it out of fast path.

   BTW remember: "addr" is allowed to be not aligned
   in IP options!
 */

void ip_rt_get_source(u8 *addr, struct sk_buff *skb, struct rtable *rt)
{
	__be32 src;

	if (rt_is_output_route(rt))
		src = ip_hdr(skb)->saddr;
	else {
		struct fib_result res;
		struct flowi4 fl4;
		struct iphdr *iph;

		iph = ip_hdr(skb);

		memset(&fl4, 0, sizeof(fl4));
		fl4.daddr = iph->daddr;
		fl4.saddr = iph->saddr;
		fl4.flowi4_tos = RT_TOS(iph->tos);
		fl4.flowi4_oif = rt->dst.dev->ifindex;
		fl4.flowi4_iif = skb->dev->ifindex;
		fl4.flowi4_mark = skb->mark;

		rcu_read_lock();
		if (fib_lookup(dev_net(rt->dst.dev), &fl4, &res) == 0)
			src = FIB_RES_PREFSRC(dev_net(rt->dst.dev), res);
		else
			src = inet_select_addr(rt->dst.dev,
					       rt_nexthop(rt, iph->daddr),
					       RT_SCOPE_UNIVERSE);
		rcu_read_unlock();
	}
	memcpy(addr, &src, 4);
}

#ifdef CONFIG_IP_ROUTE_CLASSID
static void set_class_tag(struct rtable *rt, u32 tag)
{
	if (!(rt->dst.tclassid & 0xFFFF))
		rt->dst.tclassid |= tag & 0xFFFF;
	if (!(rt->dst.tclassid & 0xFFFF0000))
		rt->dst.tclassid |= tag & 0xFFFF0000;
}
#endif

static unsigned int ipv4_default_advmss(const struct dst_entry *dst)
{
	unsigned int advmss = dst_metric_raw(dst, RTAX_ADVMSS);

	if (advmss == 0) {
		advmss = max_t(unsigned int, dst->dev->mtu - 40,
			       ip_rt_min_advmss);
		if (advmss > 65535 - 40)
			advmss = 65535 - 40;
	}
	return advmss;
}

static unsigned int ipv4_mtu(const struct dst_entry *dst)
{
	const struct rtable *rt = (const struct rtable *) dst;
	unsigned int mtu = rt->rt_pmtu;

	if (!mtu || time_after_eq(jiffies, rt->dst.expires))
		mtu = dst_metric_raw(dst, RTAX_MTU);

	if (mtu)
		return mtu;

	mtu = dst->dev->mtu;

	if (unlikely(dst_metric_locked(dst, RTAX_MTU))) {
		if (rt->rt_uses_gateway && mtu > 576)
			mtu = 576;
	}

	return min_t(unsigned int, mtu, IP_MAX_MTU);
}

static struct fib_nh_exception *find_exception(struct fib_nh *nh, __be32 daddr)
{
	struct fnhe_hash_bucket *hash = rcu_dereference(nh->nh_exceptions);
	struct fib_nh_exception *fnhe;
	u32 hval;

	if (!hash)
		return NULL;

	hval = fnhe_hashfun(daddr);

	for (fnhe = rcu_dereference(hash[hval].chain); fnhe;
	     fnhe = rcu_dereference(fnhe->fnhe_next)) {
		if (fnhe->fnhe_daddr == daddr)
			return fnhe;
	}
	return NULL;
}

static bool rt_bind_exception(struct rtable *rt, struct fib_nh_exception *fnhe,
			      __be32 daddr)
{
	bool ret = false;

	spin_lock_bh(&fnhe_lock);

	if (daddr == fnhe->fnhe_daddr) {
		struct rtable __rcu **porig;
		struct rtable *orig;
		int genid = fnhe_genid(dev_net(rt->dst.dev));

		if (rt_is_input_route(rt))
			porig = &fnhe->fnhe_rth_input;
		else
			porig = &fnhe->fnhe_rth_output;
		orig = rcu_dereference(*porig);

		if (fnhe->fnhe_genid != genid) {
			fnhe->fnhe_genid = genid;
			fnhe->fnhe_gw = 0;
			fnhe->fnhe_pmtu = 0;
			fnhe->fnhe_expires = 0;
			fnhe_flush_routes(fnhe);
			orig = NULL;
		}
		fill_route_from_fnhe(rt, fnhe);
		if (!rt->rt_gateway)
			rt->rt_gateway = daddr;

		if (!(rt->dst.flags & DST_NOCACHE)) {
			rcu_assign_pointer(*porig, rt);
			if (orig)
				rt_free(orig);
			ret = true;
		}

		fnhe->fnhe_stamp = jiffies;
	}
	spin_unlock_bh(&fnhe_lock);

	return ret;
}

static bool rt_cache_route(struct fib_nh *nh, struct rtable *rt)
{
	struct rtable *orig, *prev, **p;
	bool ret = true;

	if (rt_is_input_route(rt)) {
		p = (struct rtable **)&nh->nh_rth_input;
	} else {
		p = (struct rtable **)raw_cpu_ptr(nh->nh_pcpu_rth_output);
	}
	orig = *p;

	prev = cmpxchg(p, orig, rt);
	if (prev == orig) {
		if (orig)
			rt_free(orig);
	} else
		ret = false;

	return ret;
}

struct uncached_list {
	spinlock_t		lock;
	struct list_head	head;
};

static DEFINE_PER_CPU_ALIGNED(struct uncached_list, rt_uncached_list);

static void rt_add_uncached_list(struct rtable *rt)
{
	struct uncached_list *ul = raw_cpu_ptr(&rt_uncached_list);

	rt->rt_uncached_list = ul;

	spin_lock_bh(&ul->lock);
	list_add_tail(&rt->rt_uncached, &ul->head);
	spin_unlock_bh(&ul->lock);
}

static void ipv4_dst_destroy(struct dst_entry *dst)
{
	struct rtable *rt = (struct rtable *) dst;

	if (!list_empty(&rt->rt_uncached)) {
		struct uncached_list *ul = rt->rt_uncached_list;

		spin_lock_bh(&ul->lock);
		list_del(&rt->rt_uncached);
		spin_unlock_bh(&ul->lock);
	}
}

void rt_flush_dev(struct net_device *dev)
{
	struct net *net = dev_net(dev);
	struct rtable *rt;
	int cpu;

	for_each_possible_cpu(cpu) {
		struct uncached_list *ul = &per_cpu(rt_uncached_list, cpu);

		spin_lock_bh(&ul->lock);
		list_for_each_entry(rt, &ul->head, rt_uncached) {
			if (rt->dst.dev != dev)
				continue;
			rt->dst.dev = net->loopback_dev;
			dev_hold(rt->dst.dev);
			dev_put(dev);
		}
		spin_unlock_bh(&ul->lock);
	}
}

static bool rt_cache_valid(const struct rtable *rt)
{
	return	rt &&
		rt->dst.obsolete == DST_OBSOLETE_FORCE_CHK &&
		!rt_is_expired(rt);
}

static void rt_set_nexthop(struct rtable *rt, __be32 daddr,
			   const struct fib_result *res,
			   struct fib_nh_exception *fnhe,
			   struct fib_info *fi, u16 type, u32 itag)
{
	bool cached = false;

	if (fi) {
		struct fib_nh *nh = &FIB_RES_NH(*res);

		if (nh->nh_gw && nh->nh_scope == RT_SCOPE_LINK) {
			rt->rt_gateway = nh->nh_gw;
			rt->rt_uses_gateway = 1;
		}
		dst_init_metrics(&rt->dst, fi->fib_metrics, true);
#ifdef CONFIG_IP_ROUTE_CLASSID
		rt->dst.tclassid = nh->nh_tclassid;
#endif
		if (unlikely(fnhe))
			cached = rt_bind_exception(rt, fnhe, daddr);
		else if (!(rt->dst.flags & DST_NOCACHE))
			cached = rt_cache_route(nh, rt);
		if (unlikely(!cached)) {
			/* Routes we intend to cache in nexthop exception or
			 * FIB nexthop have the DST_NOCACHE bit clear.
			 * However, if we are unsuccessful at storing this
			 * route into the cache we really need to set it.
			 */
			rt->dst.flags |= DST_NOCACHE;
			if (!rt->rt_gateway)
				rt->rt_gateway = daddr;
			rt_add_uncached_list(rt);
		}
	} else
		rt_add_uncached_list(rt);

#ifdef CONFIG_IP_ROUTE_CLASSID
#ifdef CONFIG_IP_MULTIPLE_TABLES
	set_class_tag(rt, res->tclassid);
#endif
	set_class_tag(rt, itag);
#endif
}

static struct rtable *rt_dst_alloc(struct net_device *dev,
				   bool nopolicy, bool noxfrm, bool will_cache)
{
	return dst_alloc(&ipv4_dst_ops, dev, 1, DST_OBSOLETE_FORCE_CHK,
			 (will_cache ? 0 : (DST_HOST | DST_NOCACHE)) |
			 (nopolicy ? DST_NOPOLICY : 0) |
			 (noxfrm ? DST_NOXFRM : 0));
}

/* called in rcu_read_lock() section */
static int ip_route_input_mc(struct sk_buff *skb, __be32 daddr, __be32 saddr,
				u8 tos, struct net_device *dev, int our)
{
	struct rtable *rth;
	struct in_device *in_dev = __in_dev_get_rcu(dev);
	u32 itag = 0;
	int err;

	/* Primary sanity checks. */

	if (!in_dev)
		return -EINVAL;

	if (ipv4_is_multicast(saddr) || ipv4_is_lbcast(saddr) ||
	    skb->protocol != htons(ETH_P_IP))
		goto e_inval;

	if (likely(!IN_DEV_ROUTE_LOCALNET(in_dev)))
		if (ipv4_is_loopback(saddr))
			goto e_inval;

	if (ipv4_is_zeronet(saddr)) {
#if defined(CONFIG_BCM_KF_MISC_BACKPORTS)
		/* Backporting latest kernel fix(1d2f4ebbbeb1ec055dcd3cf3dba833cfd0a84f3a)
			to allow IGMP packets with zero source IP */
		if (!ipv4_is_local_multicast(daddr) 
		    && ip_hdr(skb)->protocol != IPPROTO_IGMP)
#else
		if (!ipv4_is_local_multicast(daddr))
#endif
			goto e_inval;
	} else {
		err = fib_validate_source(skb, saddr, 0, tos, 0, dev,
					  in_dev, &itag);
		if (err < 0)
			goto e_err;
	}
	rth = rt_dst_alloc(dev_net(dev)->loopback_dev,
			   IN_DEV_CONF_GET(in_dev, NOPOLICY), false, false);
	if (!rth)
		goto e_nobufs;

#ifdef CONFIG_IP_ROUTE_CLASSID
	rth->dst.tclassid = itag;
#endif
	rth->dst.output = ip_rt_bug;

	rth->rt_genid	= rt_genid_ipv4(dev_net(dev));
	rth->rt_flags	= RTCF_MULTICAST;
	rth->rt_type	= RTN_MULTICAST;
	rth->rt_is_input= 1;
	rth->rt_iif	= 0;
	rth->rt_pmtu	= 0;
	rth->rt_gateway	= 0;
	rth->rt_uses_gateway = 0;
	INIT_LIST_HEAD(&rth->rt_uncached);
	if (our) {
		rth->dst.input= ip_local_deliver;
		rth->rt_flags |= RTCF_LOCAL;
	}

#ifdef CONFIG_IP_MROUTE
	if (!ipv4_is_local_multicast(daddr) && IN_DEV_MFORWARD(in_dev))
		rth->dst.input = ip_mr_input;
#endif
	RT_CACHE_STAT_INC(in_slow_mc);

	skb_dst_set(skb, &rth->dst);
	return 0;

e_nobufs:
	return -ENOBUFS;
e_inval:
	return -EINVAL;
e_err:
	return err;
}


static void ip_handle_martian_source(struct net_device *dev,
				     struct in_device *in_dev,
				     struct sk_buff *skb,
				     __be32 daddr,
				     __be32 saddr)
{
	RT_CACHE_STAT_INC(in_martian_src);
#ifdef CONFIG_IP_ROUTE_VERBOSE
	if (IN_DEV_LOG_MARTIANS(in_dev) && net_ratelimit()) {
		/*
		 *	RFC1812 recommendation, if source is martian,
		 *	the only hint is MAC header.
		 */
		pr_warn("martian source %pI4 from %pI4, on dev %s\n",
			&daddr, &saddr, dev->name);
		if (dev->hard_header_len && skb_mac_header_was_set(skb)) {
			print_hex_dump(KERN_WARNING, "ll header: ",
				       DUMP_PREFIX_OFFSET, 16, 1,
				       skb_mac_header(skb),
				       dev->hard_header_len, true);
		}
	}
#endif
}

static void ip_del_fnhe(struct fib_nh *nh, __be32 daddr)
{
	struct fnhe_hash_bucket *hash;
	struct fib_nh_exception *fnhe, __rcu **fnhe_p;
	u32 hval = fnhe_hashfun(daddr);

	spin_lock_bh(&fnhe_lock);

	hash = rcu_dereference_protected(nh->nh_exceptions,
					 lockdep_is_held(&fnhe_lock));
	hash += hval;

	fnhe_p = &hash->chain;
	fnhe = rcu_dereference_protected(*fnhe_p, lockdep_is_held(&fnhe_lock));
	while (fnhe) {
		if (fnhe->fnhe_daddr == daddr) {
			rcu_assign_pointer(*fnhe_p, rcu_dereference_protected(
				fnhe->fnhe_next, lockdep_is_held(&fnhe_lock)));
			fnhe_flush_routes(fnhe);
			kfree_rcu(fnhe, rcu);
			break;
		}
		fnhe_p = &fnhe->fnhe_next;
		fnhe = rcu_dereference_protected(fnhe->fnhe_next,
						 lockdep_is_held(&fnhe_lock));
	}

	spin_unlock_bh(&fnhe_lock);
}

/* called in rcu_read_lock() section */
static int __mkroute_input(struct sk_buff *skb,
			   const struct fib_result *res,
			   struct in_device *in_dev,
			   __be32 daddr, __be32 saddr, u32 tos)
{
	struct fib_nh_exception *fnhe;
	struct rtable *rth;
	int err;
	struct in_device *out_dev;
	unsigned int flags = 0;
	bool do_cache;
	u32 itag = 0;

	/* get a working reference to the output device */
	out_dev = __in_dev_get_rcu(FIB_RES_DEV(*res));
	if (!out_dev) {
		net_crit_ratelimited("Bug in ip_route_input_slow(). Please report.\n");
		return -EINVAL;
	}

	err = fib_validate_source(skb, saddr, daddr, tos, FIB_RES_OIF(*res),
				  in_dev->dev, in_dev, &itag);
	if (err < 0) {
		ip_handle_martian_source(in_dev->dev, in_dev, skb, daddr,
					 saddr);

		goto cleanup;
	}

	do_cache = res->fi && !itag;
	if (out_dev == in_dev && err && IN_DEV_TX_REDIRECTS(out_dev) &&
	    skb->protocol == htons(ETH_P_IP) &&
	    (IN_DEV_SHARED_MEDIA(out_dev) ||
	     inet_addr_onlink(out_dev, saddr, FIB_RES_GW(*res))))
		IPCB(skb)->flags |= IPSKB_DOREDIRECT;

	if (skb->protocol != htons(ETH_P_IP)) {
		/* Not IP (i.e. ARP). Do not create route, if it is
		 * invalid for proxy arp. DNAT routes are always valid.
		 *
		 * Proxy arp feature have been extended to allow, ARP
		 * replies back to the same interface, to support
		 * Private VLAN switch technologies. See arp.c.
		 */
		if (out_dev == in_dev &&
		    IN_DEV_PROXY_ARP_PVLAN(in_dev) == 0) {
			err = -EINVAL;
			goto cleanup;
		}
	}

	fnhe = find_exception(&FIB_RES_NH(*res), daddr);
	if (do_cache) {
		if (fnhe) {
			rth = rcu_dereference(fnhe->fnhe_rth_input);
			if (rth && rth->dst.expires &&
			    time_after(jiffies, rth->dst.expires)) {
				ip_del_fnhe(&FIB_RES_NH(*res), daddr);
				fnhe = NULL;
			} else {
				goto rt_cache;
			}
		}

		rth = rcu_dereference(FIB_RES_NH(*res).nh_rth_input);

rt_cache:
		if (rt_cache_valid(rth)) {
			skb_dst_set_noref(skb, &rth->dst);
			goto out;
		}
	}

	rth = rt_dst_alloc(out_dev->dev,
			   IN_DEV_CONF_GET(in_dev, NOPOLICY),
			   IN_DEV_CONF_GET(out_dev, NOXFRM), do_cache);
	if (!rth) {
		err = -ENOBUFS;
		goto cleanup;
	}

	rth->rt_genid = rt_genid_ipv4(dev_net(rth->dst.dev));
	rth->rt_flags = flags;
	rth->rt_type = res->type;
	rth->rt_is_input = 1;
	rth->rt_iif 	= 0;
	rth->rt_pmtu	= 0;
	rth->rt_gateway	= 0;
	rth->rt_uses_gateway = 0;
	INIT_LIST_HEAD(&rth->rt_uncached);
	RT_CACHE_STAT_INC(in_slow_tot);

	rth->dst.input = ip_forward;
	rth->dst.output = ip_output;

	rt_set_nexthop(rth, daddr, res, fnhe, res->fi, res->type, itag);
	skb_dst_set(skb, &rth->dst);
out:
	err = 0;
 cleanup:
	return err;
}

static int ip_mkroute_input(struct sk_buff *skb,
			    struct fib_result *res,
			    const struct flowi4 *fl4,
			    struct in_device *in_dev,
			    __be32 daddr, __be32 saddr, u32 tos)
{
#ifdef CONFIG_IP_ROUTE_MULTIPATH
	if (res->fi && res->fi->fib_nhs > 1)
		fib_select_multipath(res);
#endif

	/* create a routing cache entry */
	return __mkroute_input(skb, res, in_dev, daddr, saddr, tos);
}

/*
 *	NOTE. We drop all the packets that has local source
 *	addresses, because every properly looped back packet
 *	must have correct destination already attached by output routine.
 *
 *	Such approach solves two big problems:
 *	1. Not simplex devices are handled properly.
 *	2. IP spoofing attempts are filtered with 100% of guarantee.
 *	called with rcu_read_lock()
 */

static int ip_route_input_slow(struct sk_buff *skb, __be32 daddr, __be32 saddr,
			       u8 tos, struct net_device *dev)
{
	struct fib_result res;
	struct in_device *in_dev = __in_dev_get_rcu(dev);
	struct flowi4	fl4;
	unsigned int	flags = 0;
	u32		itag = 0;
	struct rtable	*rth;
	int		err = -EINVAL;
	struct net    *net = dev_net(dev);
	bool do_cache;

	/* IP on this device is disabled. */

	if (!in_dev)
		goto out;

	/* Check for the most weird martians, which can be not detected
	   by fib_lookup.
	 */

	if (ipv4_is_multicast(saddr) || ipv4_is_lbcast(saddr))
		goto martian_source;

	res.fi = NULL;
	if (ipv4_is_lbcast(daddr) || (saddr == 0 && daddr == 0))
		goto brd_input;

	/* Accept zero addresses only to limited broadcast;
	 * I even do not know to fix it or not. Waiting for complains :-)
	 */
	if (ipv4_is_zeronet(saddr))
		goto martian_source;

	if (ipv4_is_zeronet(daddr))
		goto martian_destination;

	/* Following code try to avoid calling IN_DEV_NET_ROUTE_LOCALNET(),
	 * and call it once if daddr or/and saddr are loopback addresses
	 */
	if (ipv4_is_loopback(daddr)) {
		if (!IN_DEV_NET_ROUTE_LOCALNET(in_dev, net))
			goto martian_destination;
	} else if (ipv4_is_loopback(saddr)) {
		if (!IN_DEV_NET_ROUTE_LOCALNET(in_dev, net))
			goto martian_source;
	}

	/*
	 *	Now we are ready to route packet.
	 */
	fl4.flowi4_oif = 0;
	fl4.flowi4_iif = dev->ifindex;
	fl4.flowi4_mark = skb->mark;
	fl4.flowi4_tos = tos;
	fl4.flowi4_scope = RT_SCOPE_UNIVERSE;
	fl4.daddr = daddr;
	fl4.saddr = saddr;
	err = fib_lookup(net, &fl4, &res);
	if (err != 0) {
		if (!IN_DEV_FORWARD(in_dev))
			err = -EHOSTUNREACH;
		goto no_route;
	}

	if (res.type == RTN_BROADCAST)
		goto brd_input;

	if (res.type == RTN_LOCAL) {
		err = fib_validate_source(skb, saddr, daddr, tos,
					  0, dev, in_dev, &itag);
		if (err < 0)
			goto martian_source_keep_err;
		goto local_input;
	}

	if (!IN_DEV_FORWARD(in_dev)) {
		err = -EHOSTUNREACH;
		goto no_route;
	}
	if (res.type != RTN_UNICAST)
		goto martian_destination;

	err = ip_mkroute_input(skb, &res, &fl4, in_dev, daddr, saddr, tos);
out:	return err;

brd_input:
	if (skb->protocol != htons(ETH_P_IP))
		goto e_inval;

	if (!ipv4_is_zeronet(saddr)) {
		err = fib_validate_source(skb, saddr, 0, tos, 0, dev,
					  in_dev, &itag);
		if (err < 0)
			goto martian_source_keep_err;
	}
	flags |= RTCF_BROADCAST;
	res.type = RTN_BROADCAST;
	RT_CACHE_STAT_INC(in_brd);

local_input:
	do_cache = false;
	if (res.fi) {
		if (!itag) {
			rth = rcu_dereference(FIB_RES_NH(res).nh_rth_input);
			if (rt_cache_valid(rth)) {
				skb_dst_set_noref(skb, &rth->dst);
				err = 0;
				goto out;
			}
			do_cache = true;
		}
	}

	rth = rt_dst_alloc(net->loopback_dev,
			   IN_DEV_CONF_GET(in_dev, NOPOLICY), false, do_cache);
	if (!rth)
		goto e_nobufs;

	rth->dst.input= ip_local_deliver;
	rth->dst.output= ip_rt_bug;
#ifdef CONFIG_IP_ROUTE_CLASSID
	rth->dst.tclassid = itag;
#endif

	rth->rt_genid = rt_genid_ipv4(net);
	rth->rt_flags 	= flags|RTCF_LOCAL;
	rth->rt_type	= res.type;
	rth->rt_is_input = 1;
	rth->rt_iif	= 0;
	rth->rt_pmtu	= 0;
	rth->rt_gateway	= 0;
	rth->rt_uses_gateway = 0;
	INIT_LIST_HEAD(&rth->rt_uncached);
	RT_CACHE_STAT_INC(in_slow_tot);
	if (res.type == RTN_UNREACHABLE) {
		rth->dst.input= ip_error;
		rth->dst.error= -err;
		rth->rt_flags 	&= ~RTCF_LOCAL;
	}
	if (do_cache) {
		if (unlikely(!rt_cache_route(&FIB_RES_NH(res), rth))) {
			rth->dst.flags |= DST_NOCACHE;
			rt_add_uncached_list(rth);
		}
	}
	skb_dst_set(skb, &rth->dst);
	err = 0;
	goto out;

no_route:
	RT_CACHE_STAT_INC(in_no_route);
	res.type = RTN_UNREACHABLE;
	res.fi = NULL;
	goto local_input;

	/*
	 *	Do not cache martian addresses: they should be logged (RFC1812)
	 */
martian_destination:
	RT_CACHE_STAT_INC(in_martian_dst);
#ifdef CONFIG_IP_ROUTE_VERBOSE
	if (IN_DEV_LOG_MARTIANS(in_dev))
		net_warn_ratelimited("martian destination %pI4 from %pI4, dev %s\n",
				     &daddr, &saddr, dev->name);
#endif

e_inval:
	err = -EINVAL;
	goto out;

e_nobufs:
	err = -ENOBUFS;
	goto out;

martian_source:
	err = -EINVAL;
martian_source_keep_err:
	ip_handle_martian_source(dev, in_dev, skb, daddr, saddr);
	goto out;
}

int ip_route_input_noref(struct sk_buff *skb, __be32 daddr, __be32 saddr,
			 u8 tos, struct net_device *dev)
{
	int res;

	tos &= IPTOS_RT_MASK;
	rcu_read_lock();

	/* Multicast recognition logic is moved from route cache to here.
	   The problem was that too many Ethernet cards have broken/missing
	   hardware multicast filters :-( As result the host on multicasting
	   network acquires a lot of useless route cache entries, sort of
	   SDR messages from all the world. Now we try to get rid of them.
	   Really, provided software IP multicast filter is organized
	   reasonably (at least, hashed), it does not result in a slowdown
	   comparing with route cache reject entries.
	   Note, that multicast routers are not affected, because
	   route cache entry is created eventually.
	 */
	if (ipv4_is_multicast(daddr)) {
		struct in_device *in_dev = __in_dev_get_rcu(dev);

		if (in_dev) {
			int our = ip_check_mc_rcu(in_dev, daddr, saddr,
						  ip_hdr(skb)->protocol);
			if (our
#ifdef CONFIG_IP_MROUTE
				||
			    (!ipv4_is_local_multicast(daddr) &&
			     IN_DEV_MFORWARD(in_dev))
#endif
			   ) {
				int res = ip_route_input_mc(skb, daddr, saddr,
							    tos, dev, our);
				rcu_read_unlock();
				return res;
			}
		}
		rcu_read_unlock();
		return -EINVAL;
	}
	res = ip_route_input_slow(skb, daddr, saddr, tos, dev);
	rcu_read_unlock();
	return res;
}
EXPORT_SYMBOL(ip_route_input_noref);

/* called with rcu_read_lock() */
static struct rtable *__mkroute_output(const struct fib_result *res,
				       const struct flowi4 *fl4, int orig_oif,
				       struct net_device *dev_out,
				       unsigned int flags)
{
	struct fib_info *fi = res->fi;
	struct fib_nh_exception *fnhe;
	struct in_device *in_dev;
	u16 type = res->type;
	struct rtable *rth;
	bool do_cache;

	in_dev = __in_dev_get_rcu(dev_out);
	if (!in_dev)
		return ERR_PTR(-EINVAL);

	if (likely(!IN_DEV_ROUTE_LOCALNET(in_dev)))
		if (ipv4_is_loopback(fl4->saddr) && !(dev_out->flags & IFF_LOOPBACK))
			return ERR_PTR(-EINVAL);

	if (ipv4_is_lbcast(fl4->daddr))
		type = RTN_BROADCAST;
	else if (ipv4_is_multicast(fl4->daddr))
		type = RTN_MULTICAST;
	else if (ipv4_is_zeronet(fl4->daddr))
		return ERR_PTR(-EINVAL);

	if (dev_out->flags & IFF_LOOPBACK)
		flags |= RTCF_LOCAL;

	do_cache = true;
	if (type == RTN_BROADCAST) {
		flags |= RTCF_BROADCAST | RTCF_LOCAL;
		fi = NULL;
	} else if (type == RTN_MULTICAST) {
		flags |= RTCF_MULTICAST | RTCF_LOCAL;
		if (!ip_check_mc_rcu(in_dev, fl4->daddr, fl4->saddr,
				     fl4->flowi4_proto))
			flags &= ~RTCF_LOCAL;
		else
			do_cache = false;
		/* If multicast route do not exist use
		 * default one, but do not gateway in this case.
		 * Yes, it is hack.
		 */
		if (fi && res->prefixlen < 4)
			fi = NULL;
	} else if ((type == RTN_LOCAL) && (orig_oif != 0) &&
		   (orig_oif != dev_out->ifindex)) {
		/* For local routes that require a particular output interface
		 * we do not want to cache the result.  Caching the result
		 * causes incorrect behaviour when there are multiple source
		 * addresses on the interface, the end result being that if the
		 * intended recipient is waiting on that interface for the
		 * packet he won't receive it because it will be delivered on
		 * the loopback interface and the IP_PKTINFO ipi_ifindex will
		 * be set to the loopback interface as well.
		 */
		fi = NULL;
	}

	fnhe = NULL;
	do_cache &= fi != NULL;
	if (do_cache) {
		struct rtable __rcu **prth;
		struct fib_nh *nh = &FIB_RES_NH(*res);

		fnhe = find_exception(nh, fl4->daddr);
		if (fnhe) {
			prth = &fnhe->fnhe_rth_output;
			rth = rcu_dereference(*prth);
			if (rth && rth->dst.expires &&
			    time_after(jiffies, rth->dst.expires)) {
				ip_del_fnhe(nh, fl4->daddr);
				fnhe = NULL;
			} else {
				goto rt_cache;
			}
		}

		if (unlikely(fl4->flowi4_flags &
			     FLOWI_FLAG_KNOWN_NH &&
			     !(nh->nh_gw &&
			       nh->nh_scope == RT_SCOPE_LINK))) {
			do_cache = false;
			goto add;
		}
		prth = raw_cpu_ptr(nh->nh_pcpu_rth_output);
		rth = rcu_dereference(*prth);

rt_cache:
		if (rt_cache_valid(rth)) {
			dst_hold(&rth->dst);
			return rth;
		}
	}

add:
	rth = rt_dst_alloc(dev_out,
			   IN_DEV_CONF_GET(in_dev, NOPOLICY),
			   IN_DEV_CONF_GET(in_dev, NOXFRM),
			   do_cache);
	if (!rth)
		return ERR_PTR(-ENOBUFS);

	rth->dst.output = ip_output;

	rth->rt_genid = rt_genid_ipv4(dev_net(dev_out));
	rth->rt_flags	= flags;
	rth->rt_type	= type;
	rth->rt_is_input = 0;
	rth->rt_iif	= orig_oif ? : 0;
	rth->rt_pmtu	= 0;
	rth->rt_gateway = 0;
	rth->rt_uses_gateway = 0;
	INIT_LIST_HEAD(&rth->rt_uncached);

	RT_CACHE_STAT_INC(out_slow_tot);

	if (flags & RTCF_LOCAL)
		rth->dst.input = ip_local_deliver;
	if (flags & (RTCF_BROADCAST | RTCF_MULTICAST)) {
		if (flags & RTCF_LOCAL &&
		    !(dev_out->flags & IFF_LOOPBACK)) {
			rth->dst.output = ip_mc_output;
			RT_CACHE_STAT_INC(out_slow_mc);
		}
#ifdef CONFIG_IP_MROUTE
		if (type == RTN_MULTICAST) {
			if (IN_DEV_MFORWARD(in_dev) &&
			    !ipv4_is_local_multicast(fl4->daddr)) {
				rth->dst.input = ip_mr_input;
				rth->dst.output = ip_mc_output;
			}
		}
#endif
	}

	rt_set_nexthop(rth, fl4->daddr, res, fnhe, fi, type, 0);

	return rth;
}

/*
 * Major route resolver routine.
 */

struct rtable *__ip_route_output_key(struct net *net, struct flowi4 *fl4)
{
	struct net_device *dev_out = NULL;
	__u8 tos = RT_FL_TOS(fl4);
	unsigned int flags = 0;
	struct fib_result res;
	struct rtable *rth;
	int orig_oif;

	res.tclassid	= 0;
	res.fi		= NULL;
	res.table	= NULL;

	orig_oif = fl4->flowi4_oif;

	fl4->flowi4_iif = LOOPBACK_IFINDEX;
	fl4->flowi4_tos = tos & IPTOS_RT_MASK;
	fl4->flowi4_scope = ((tos & RTO_ONLINK) ?
			 RT_SCOPE_LINK : RT_SCOPE_UNIVERSE);

	rcu_read_lock();
	if (fl4->saddr) {
		rth = ERR_PTR(-EINVAL);
		if (ipv4_is_multicast(fl4->saddr) ||
		    ipv4_is_lbcast(fl4->saddr) ||
		    ipv4_is_zeronet(fl4->saddr))
			goto out;

		/* I removed check for oif == dev_out->oif here.
		   It was wrong for two reasons:
		   1. ip_dev_find(net, saddr) can return wrong iface, if saddr
		      is assigned to multiple interfaces.
		   2. Moreover, we are allowed to send packets with saddr
		      of another iface. --ANK
		 */

		if (fl4->flowi4_oif == 0 &&
		    (ipv4_is_multicast(fl4->daddr) ||
		     ipv4_is_lbcast(fl4->daddr))) {
			/* It is equivalent to inet_addr_type(saddr) == RTN_LOCAL */
			dev_out = __ip_dev_find(net, fl4->saddr, false);
			if (!dev_out)
				goto out;

			/* Special hack: user can direct multicasts
			   and limited broadcast via necessary interface
			   without fiddling with IP_MULTICAST_IF or IP_PKTINFO.
			   This hack is not just for fun, it allows
			   vic,vat and friends to work.
			   They bind socket to loopback, set ttl to zero
			   and expect that it will work.
			   From the viewpoint of routing cache they are broken,
			   because we are not allowed to build multicast path
			   with loopback source addr (look, routing cache
			   cannot know, that ttl is zero, so that packet
			   will not leave this host and route is valid).
			   Luckily, this hack is good workaround.
			 */

			fl4->flowi4_oif = dev_out->ifindex;
			goto make_route;
		}

		if (!(fl4->flowi4_flags & FLOWI_FLAG_ANYSRC)) {
			/* It is equivalent to inet_addr_type(saddr) == RTN_LOCAL */
			if (!__ip_dev_find(net, fl4->saddr, false))
				goto out;
		}
	}


	if (fl4->flowi4_oif) {
		dev_out = dev_get_by_index_rcu(net, fl4->flowi4_oif);
		rth = ERR_PTR(-ENODEV);
		if (!dev_out)
			goto out;

		/* RACE: Check return value of inet_select_addr instead. */
		if (!(dev_out->flags & IFF_UP) || !__in_dev_get_rcu(dev_out)) {
			rth = ERR_PTR(-ENETUNREACH);
			goto out;
		}
		if (ipv4_is_local_multicast(fl4->daddr) ||
		    ipv4_is_lbcast(fl4->daddr)) {
			if (!fl4->saddr)
				fl4->saddr = inet_select_addr(dev_out, 0,
							      RT_SCOPE_LINK);
			goto make_route;
		}
		if (!fl4->saddr) {
			if (ipv4_is_multicast(fl4->daddr))
				fl4->saddr = inet_select_addr(dev_out, 0,
							      fl4->flowi4_scope);
			else if (!fl4->daddr)
				fl4->saddr = inet_select_addr(dev_out, 0,
							      RT_SCOPE_HOST);
		}
	}

	if (!fl4->daddr) {
		fl4->daddr = fl4->saddr;
		if (!fl4->daddr)
			fl4->daddr = fl4->saddr = htonl(INADDR_LOOPBACK);
		dev_out = net->loopback_dev;
		fl4->flowi4_oif = LOOPBACK_IFINDEX;
		res.type = RTN_LOCAL;
		flags |= RTCF_LOCAL;
		goto make_route;
	}

	if (fib_lookup(net, fl4, &res)) {
		res.fi = NULL;
		res.table = NULL;
		if (fl4->flowi4_oif) {
			/* Apparently, routing tables are wrong. Assume,
			   that the destination is on link.

			   WHY? DW.
			   Because we are allowed to send to iface
			   even if it has NO routes and NO assigned
			   addresses. When oif is specified, routing
			   tables are looked up with only one purpose:
			   to catch if destination is gatewayed, rather than
			   direct. Moreover, if MSG_DONTROUTE is set,
			   we send packet, ignoring both routing tables
			   and ifaddr state. --ANK


			   We could make it even if oif is unknown,
			   likely IPv6, but we do not.
			 */

			if (fl4->saddr == 0)
				fl4->saddr = inet_select_addr(dev_out, 0,
							      RT_SCOPE_LINK);
			res.type = RTN_UNICAST;
			goto make_route;
		}
		rth = ERR_PTR(-ENETUNREACH);
		goto out;
	}

	if (res.type == RTN_LOCAL) {
		if (!fl4->saddr) {
			if (res.fi->fib_prefsrc)
				fl4->saddr = res.fi->fib_prefsrc;
			else
				fl4->saddr = fl4->daddr;
		}
		dev_out = net->loopback_dev;
		fl4->flowi4_oif = dev_out->ifindex;
		flags |= RTCF_LOCAL;
		goto make_route;
	}

#ifdef CONFIG_IP_ROUTE_MULTIPATH
	if (res.fi->fib_nhs > 1 && fl4->flowi4_oif == 0)
		fib_select_multipath(&res);
	else
#endif
	if (!res.prefixlen &&
	    res.table->tb_num_default > 1 &&
	    res.type == RTN_UNICAST && !fl4->flowi4_oif)
		fib_select_default(&res);

	if (!fl4->saddr)
		fl4->saddr = FIB_RES_PREFSRC(net, res);

	dev_out = FIB_RES_DEV(res);
	fl4->flowi4_oif = dev_out->ifindex;


make_route:
	rth = __mkroute_output(&res, fl4, orig_oif, dev_out, flags);

out:
	rcu_read_unlock();
	return rth;
}
EXPORT_SYMBOL_GPL(__ip_route_output_key);

static struct dst_entry *ipv4_blackhole_dst_check(struct dst_entry *dst, u32 cookie)
{
	return NULL;
}

static unsigned int ipv4_blackhole_mtu(const struct dst_entry *dst)
{
	unsigned int mtu = dst_metric_raw(dst, RTAX_MTU);

	return mtu ? : dst->dev->mtu;
}

static void ipv4_rt_blackhole_update_pmtu(struct dst_entry *dst, struct sock *sk,
					  struct sk_buff *skb, u32 mtu)
{
}

static void ipv4_rt_blackhole_redirect(struct dst_entry *dst, struct sock *sk,
				       struct sk_buff *skb)
{
}

static u32 *ipv4_rt_blackhole_cow_metrics(struct dst_entry *dst,
					  unsigned long old)
{
	return NULL;
}

static struct dst_ops ipv4_dst_blackhole_ops = {
	.family			=	AF_INET,
	.check			=	ipv4_blackhole_dst_check,
	.mtu			=	ipv4_blackhole_mtu,
	.default_advmss		=	ipv4_default_advmss,
	.update_pmtu		=	ipv4_rt_blackhole_update_pmtu,
	.redirect		=	ipv4_rt_blackhole_redirect,
	.cow_metrics		=	ipv4_rt_blackhole_cow_metrics,
	.neigh_lookup		=	ipv4_neigh_lookup,
};

struct dst_entry *ipv4_blackhole_route(struct net *net, struct dst_entry *dst_orig)
{
	struct rtable *ort = (struct rtable *) dst_orig;
	struct rtable *rt;

	rt = dst_alloc(&ipv4_dst_blackhole_ops, NULL, 1, DST_OBSOLETE_NONE, 0);
	if (rt) {
		struct dst_entry *new = &rt->dst;

		new->__use = 1;
		new->input = dst_discard;
		new->output = dst_discard_sk;

		new->dev = ort->dst.dev;
		if (new->dev)
			dev_hold(new->dev);

		rt->rt_is_input = ort->rt_is_input;
		rt->rt_iif = ort->rt_iif;
		rt->rt_pmtu = ort->rt_pmtu;

		rt->rt_genid = rt_genid_ipv4(net);
		rt->rt_flags = ort->rt_flags;
		rt->rt_type = ort->rt_type;
		rt->rt_gateway = ort->rt_gateway;
		rt->rt_uses_gateway = ort->rt_uses_gateway;

		INIT_LIST_HEAD(&rt->rt_uncached);

		dst_free(new);
	}

	dst_release(dst_orig);

	return rt ? &rt->dst : ERR_PTR(-ENOMEM);
}

struct rtable *ip_route_output_flow(struct net *net, struct flowi4 *flp4,
				    struct sock *sk)
{
	struct rtable *rt = __ip_route_output_key(net, flp4);

	if (IS_ERR(rt))
		return rt;

	if (flp4->flowi4_proto)
		rt = (struct rtable *)xfrm_lookup_route(net, &rt->dst,
							flowi4_to_flowi(flp4),
							sk, 0);

	return rt;
}
EXPORT_SYMBOL_GPL(ip_route_output_flow);

static int rt_fill_info(struct net *net,  __be32 dst, __be32 src,
			struct flowi4 *fl4, struct sk_buff *skb, u32 portid,
			u32 seq, int event, int nowait, unsigned int flags)
{
	struct rtable *rt = skb_rtable(skb);
	struct rtmsg *r;
	struct nlmsghdr *nlh;
	unsigned long expires = 0;
	u32 error;
	u32 metrics[RTAX_MAX];

	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*r), flags);
	if (!nlh)
		return -EMSGSIZE;

	r = nlmsg_data(nlh);
	r->rtm_family	 = AF_INET;
	r->rtm_dst_len	= 32;
	r->rtm_src_len	= 0;
	r->rtm_tos	= fl4->flowi4_tos;
	r->rtm_table	= RT_TABLE_MAIN;
	if (nla_put_u32(skb, RTA_TABLE, RT_TABLE_MAIN))
		goto nla_put_failure;
	r->rtm_type	= rt->rt_type;
	r->rtm_scope	= RT_SCOPE_UNIVERSE;
	r->rtm_protocol = RTPROT_UNSPEC;
	r->rtm_flags	= (rt->rt_flags & ~0xFFFF) | RTM_F_CLONED;
	if (rt->rt_flags & RTCF_NOTIFY)
		r->rtm_flags |= RTM_F_NOTIFY;
	if (IPCB(skb)->flags & IPSKB_DOREDIRECT)
		r->rtm_flags |= RTCF_DOREDIRECT;

	if (nla_put_in_addr(skb, RTA_DST, dst))
		goto nla_put_failure;
	if (src) {
		r->rtm_src_len = 32;
		if (nla_put_in_addr(skb, RTA_SRC, src))
			goto nla_put_failure;
	}
	if (rt->dst.dev &&
	    nla_put_u32(skb, RTA_OIF, rt->dst.dev->ifindex))
		goto nla_put_failure;
#ifdef CONFIG_IP_ROUTE_CLASSID
	if (rt->dst.tclassid &&
	    nla_put_u32(skb, RTA_FLOW, rt->dst.tclassid))
		goto nla_put_failure;
#endif
	if (!rt_is_input_route(rt) &&
	    fl4->saddr != src) {
		if (nla_put_in_addr(skb, RTA_PREFSRC, fl4->saddr))
			goto nla_put_failure;
	}
	if (rt->rt_uses_gateway &&
	    nla_put_in_addr(skb, RTA_GATEWAY, rt->rt_gateway))
		goto nla_put_failure;

	expires = rt->dst.expires;
	if (expires) {
		unsigned long now = jiffies;

		if (time_before(now, expires))
			expires -= now;
		else
			expires = 0;
	}

	memcpy(metrics, dst_metrics_ptr(&rt->dst), sizeof(metrics));
	if (rt->rt_pmtu && expires)
		metrics[RTAX_MTU - 1] = rt->rt_pmtu;
	if (rtnetlink_put_metrics(skb, metrics) < 0)
		goto nla_put_failure;

	if (fl4->flowi4_mark &&
	    nla_put_u32(skb, RTA_MARK, fl4->flowi4_mark))
		goto nla_put_failure;

	error = rt->dst.error;

	if (rt_is_input_route(rt)) {
#ifdef CONFIG_IP_MROUTE
		if (ipv4_is_multicast(dst) && !ipv4_is_local_multicast(dst) &&
		    IPV4_DEVCONF_ALL(net, MC_FORWARDING)) {
#if defined(CONFIG_BCM_KF_MROUTE)
			int err = ipmr_get_route(net, skb,
						 fl4->saddr, fl4->daddr,
						 r, nowait, rt->rt_iif);
#else
			int err = ipmr_get_route(net, skb,
						 fl4->saddr, fl4->daddr,
						 r, nowait);
#endif
			if (err <= 0) {
				if (!nowait) {
					if (err == 0)
						return 0;
					goto nla_put_failure;
				} else {
					if (err == -EMSGSIZE)
						goto nla_put_failure;
					error = err;
				}
			}
		} else
#endif
			if (nla_put_u32(skb, RTA_IIF, skb->dev->ifindex))
				goto nla_put_failure;
	}

	if (rtnl_put_cacheinfo(skb, &rt->dst, 0, expires, error) < 0)
		goto nla_put_failure;

	nlmsg_end(skb, nlh);
	return 0;

nla_put_failure:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}

static int inet_rtm_getroute(struct sk_buff *in_skb, struct nlmsghdr *nlh)
{
	struct net *net = sock_net(in_skb->sk);
	struct rtmsg *rtm;
	struct nlattr *tb[RTA_MAX+1];
	struct rtable *rt = NULL;
	struct flowi4 fl4;
	__be32 dst = 0;
	__be32 src = 0;
	u32 iif;
	int err;
	int mark;
	struct sk_buff *skb;

	err = nlmsg_parse(nlh, sizeof(*rtm), tb, RTA_MAX, rtm_ipv4_policy);
	if (err < 0)
		goto errout;

	rtm = nlmsg_data(nlh);

	skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb) {
		err = -ENOBUFS;
		goto errout;
	}

	/* Reserve room for dummy headers, this skb can pass
	   through good chunk of routing engine.
	 */
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);

	/* Bugfix: need to give ip_route_input enough of an IP header to not gag. */
	ip_hdr(skb)->protocol = IPPROTO_UDP;
	skb_reserve(skb, MAX_HEADER + sizeof(struct iphdr));

	src = tb[RTA_SRC] ? nla_get_in_addr(tb[RTA_SRC]) : 0;
	dst = tb[RTA_DST] ? nla_get_in_addr(tb[RTA_DST]) : 0;
	iif = tb[RTA_IIF] ? nla_get_u32(tb[RTA_IIF]) : 0;
	mark = tb[RTA_MARK] ? nla_get_u32(tb[RTA_MARK]) : 0;

	memset(&fl4, 0, sizeof(fl4));
	fl4.daddr = dst;
	fl4.saddr = src;
	fl4.flowi4_tos = rtm->rtm_tos;
	fl4.flowi4_oif = tb[RTA_OIF] ? nla_get_u32(tb[RTA_OIF]) : 0;
	fl4.flowi4_mark = mark;

	if (iif) {
		struct net_device *dev;

		dev = __dev_get_by_index(net, iif);
		if (!dev) {
			err = -ENODEV;
			goto errout_free;
		}

		skb->protocol	= htons(ETH_P_IP);
		skb->dev	= dev;
		skb->mark	= mark;
		local_bh_disable();
		err = ip_route_input(skb, dst, src, rtm->rtm_tos, dev);
		local_bh_enable();

		rt = skb_rtable(skb);
		if (err == 0 && rt->dst.error)
			err = -rt->dst.error;
	} else {
		rt = ip_route_output_key(net, &fl4);

		err = 0;
		if (IS_ERR(rt))
			err = PTR_ERR(rt);
	}

	if (err)
		goto errout_free;

	skb_dst_set(skb, &rt->dst);
	if (rtm->rtm_flags & RTM_F_NOTIFY)
		rt->rt_flags |= RTCF_NOTIFY;

	err = rt_fill_info(net, dst, src, &fl4, skb,
			   NETLINK_CB(in_skb).portid, nlh->nlmsg_seq,
			   RTM_NEWROUTE, 0, 0);
	if (err < 0)
		goto errout_free;

	err = rtnl_unicast(skb, net, NETLINK_CB(in_skb).portid);
errout:
	return err;

errout_free:
	kfree_skb(skb);
	goto errout;
}

void ip_rt_multicast_event(struct in_device *in_dev)
{
	rt_cache_flush(dev_net(in_dev->dev));
}

#ifdef CONFIG_SYSCTL
static int ip_rt_gc_interval __read_mostly  = 60 * HZ;
static int ip_rt_gc_min_interval __read_mostly	= HZ / 2;
static int ip_rt_gc_elasticity __read_mostly	= 8;

static int ipv4_sysctl_rtcache_flush(struct ctl_table *__ctl, int write,
					void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	struct net *net = (struct net *)__ctl->extra1;

	if (write) {
		rt_cache_flush(net);
		fnhe_genid_bump(net);
		return 0;
	}

	return -EINVAL;
}

static struct ctl_table ipv4_route_table[] = {
	{
		.procname	= "gc_thresh",
		.data		= &ipv4_dst_ops.gc_thresh,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "max_size",
		.data		= &ip_rt_max_size,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		/*  Deprecated. Use gc_min_interval_ms */

		.procname	= "gc_min_interval",
		.data		= &ip_rt_gc_min_interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "gc_min_interval_ms",
		.data		= &ip_rt_gc_min_interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_ms_jiffies,
	},
	{
		.procname	= "gc_timeout",
		.data		= &ip_rt_gc_timeout,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "gc_interval",
		.data		= &ip_rt_gc_interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "redirect_load",
		.data		= &ip_rt_redirect_load,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "redirect_number",
		.data		= &ip_rt_redirect_number,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "redirect_silence",
		.data		= &ip_rt_redirect_silence,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "error_cost",
		.data		= &ip_rt_error_cost,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "error_burst",
		.data		= &ip_rt_error_burst,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "gc_elasticity",
		.data		= &ip_rt_gc_elasticity,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "mtu_expires",
		.data		= &ip_rt_mtu_expires,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "min_pmtu",
		.data		= &ip_rt_min_pmtu,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &ip_min_valid_pmtu,
	},
	{
		.procname	= "min_adv_mss",
		.data		= &ip_rt_min_advmss,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

static struct ctl_table ipv4_route_flush_table[] = {
	{
		.procname	= "flush",
		.maxlen		= sizeof(int),
		.mode		= 0200,
		.proc_handler	= ipv4_sysctl_rtcache_flush,
	},
	{ },
};

static __net_init int sysctl_route_net_init(struct net *net)
{
	struct ctl_table *tbl;

	tbl = ipv4_route_flush_table;
	if (!net_eq(net, &init_net)) {
		tbl = kmemdup(tbl, sizeof(ipv4_route_flush_table), GFP_KERNEL);
		if (!tbl)
			goto err_dup;

		/* Don't export sysctls to unprivileged users */
		if (net->user_ns != &init_user_ns)
			tbl[0].procname = NULL;
	}
	tbl[0].extra1 = net;

	net->ipv4.route_hdr = register_net_sysctl(net, "net/ipv4/route", tbl);
	if (!net->ipv4.route_hdr)
		goto err_reg;
	return 0;

err_reg:
	if (tbl != ipv4_route_flush_table)
		kfree(tbl);
err_dup:
	return -ENOMEM;
}

static __net_exit void sysctl_route_net_exit(struct net *net)
{
	struct ctl_table *tbl;

	tbl = net->ipv4.route_hdr->ctl_table_arg;
	unregister_net_sysctl_table(net->ipv4.route_hdr);
	BUG_ON(tbl == ipv4_route_flush_table);
	kfree(tbl);
}

static __net_initdata struct pernet_operations sysctl_route_ops = {
	.init = sysctl_route_net_init,
	.exit = sysctl_route_net_exit,
};
#endif

static __net_init int rt_genid_init(struct net *net)
{
	atomic_set(&net->ipv4.rt_genid, 0);
	atomic_set(&net->fnhe_genid, 0);
	get_random_bytes(&net->ipv4.dev_addr_genid,
			 sizeof(net->ipv4.dev_addr_genid));
	return 0;
}

static __net_initdata struct pernet_operations rt_genid_ops = {
	.init = rt_genid_init,
};

static int __net_init ipv4_inetpeer_init(struct net *net)
{
	struct inet_peer_base *bp = kmalloc(sizeof(*bp), GFP_KERNEL);

	if (!bp)
		return -ENOMEM;
	inet_peer_base_init(bp);
	net->ipv4.peers = bp;
	return 0;
}

static void __net_exit ipv4_inetpeer_exit(struct net *net)
{
	struct inet_peer_base *bp = net->ipv4.peers;

	net->ipv4.peers = NULL;
	inetpeer_invalidate_tree(bp);
	kfree(bp);
}

static __net_initdata struct pernet_operations ipv4_inetpeer_ops = {
	.init	=	ipv4_inetpeer_init,
	.exit	=	ipv4_inetpeer_exit,
};

#ifdef CONFIG_IP_ROUTE_CLASSID
struct ip_rt_acct __percpu *ip_rt_acct __read_mostly;
#endif /* CONFIG_IP_ROUTE_CLASSID */

int __init ip_rt_init(void)
{
	int rc = 0;
	int cpu;

	ip_idents = kmalloc(IP_IDENTS_SZ * sizeof(*ip_idents), GFP_KERNEL);
	if (!ip_idents)
		panic("IP: failed to allocate ip_idents\n");

	prandom_bytes(ip_idents, IP_IDENTS_SZ * sizeof(*ip_idents));

	for_each_possible_cpu(cpu) {
		struct uncached_list *ul = &per_cpu(rt_uncached_list, cpu);

		INIT_LIST_HEAD(&ul->head);
		spin_lock_init(&ul->lock);
	}
#ifdef CONFIG_IP_ROUTE_CLASSID
	ip_rt_acct = __alloc_percpu(256 * sizeof(struct ip_rt_acct), __alignof__(struct ip_rt_acct));
	if (!ip_rt_acct)
		panic("IP: failed to allocate ip_rt_acct\n");
#endif

	ipv4_dst_ops.kmem_cachep =
		kmem_cache_create("ip_dst_cache", sizeof(struct rtable), 0,
				  SLAB_HWCACHE_ALIGN|SLAB_PANIC, NULL);

	ipv4_dst_blackhole_ops.kmem_cachep = ipv4_dst_ops.kmem_cachep;

	if (dst_entries_init(&ipv4_dst_ops) < 0)
		panic("IP: failed to allocate ipv4_dst_ops counter\n");

	if (dst_entries_init(&ipv4_dst_blackhole_ops) < 0)
		panic("IP: failed to allocate ipv4_dst_blackhole_ops counter\n");

	ipv4_dst_ops.gc_thresh = ~0;
	ip_rt_max_size = INT_MAX;

	devinet_init();
	ip_fib_init();

	if (ip_rt_proc_init())
		pr_err("Unable to create route proc files\n");
#ifdef CONFIG_XFRM
	xfrm_init();
	xfrm4_init();
#endif
	rtnl_register(PF_INET, RTM_GETROUTE, inet_rtm_getroute, NULL, NULL);

#ifdef CONFIG_SYSCTL
	register_pernet_subsys(&sysctl_route_ops);
#endif
	register_pernet_subsys(&rt_genid_ops);
	register_pernet_subsys(&ipv4_inetpeer_ops);
	return rc;
}

#ifdef CONFIG_SYSCTL
/*
 * We really need to sanitize the damn ipv4 init order, then all
 * this nonsense will go away.
 */
void __init ip_static_sysctl_init(void)
{
	register_net_sysctl(&init_net, "net/ipv4/route", ipv4_route_table);
}
#endif
