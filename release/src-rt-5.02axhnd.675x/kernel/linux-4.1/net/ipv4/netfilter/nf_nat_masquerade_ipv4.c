/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/atomic.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <net/route.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/ipv4/nf_nat_masquerade.h>

#if defined(CONFIG_BCM_KF_NETFILTER)
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_core.h>
#endif

#if defined(CONFIG_BCM_KF_NETFILTER)
/****************************************************************************/
static void bcm_nat_expect(struct nf_conn *ct,
			   struct nf_conntrack_expect *exp)
{
	struct nf_nat_range range;

	/* This must be a fresh one. */
	BUG_ON(ct->status & IPS_NAT_DONE_MASK);

	/* Change src to where new ct comes from */
	range.flags = NF_NAT_RANGE_MAP_IPS;
	range.min_addr = range.max_addr =
		ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3;
	nf_nat_setup_info(ct, &range, NF_NAT_MANIP_SRC);
	 
	/* For DST manip, map port here to where it's expected. */
	range.flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
	range.min_proto = range.max_proto = exp->saved_proto;
	range.min_addr = range.max_addr = exp->saved_addr;
	nf_nat_setup_info(ct, &range, NF_NAT_MANIP_DST);
}

/****************************************************************************/
static int bcm_nat_help(struct sk_buff *skb, unsigned int protoff,
			struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	struct nf_conn_help *help = nfct_help(ct);
	struct nf_conntrack_expect *exp;
	
	if (dir != IP_CT_DIR_ORIGINAL ||
	    help->expecting[NF_CT_EXPECT_CLASS_DEFAULT])
		return NF_ACCEPT;

	pr_debug("bcm_nat: packet[%d bytes] ", skb->len);
	nf_ct_dump_tuple(&ct->tuplehash[dir].tuple);
	pr_debug("reply: ");
	nf_ct_dump_tuple(&ct->tuplehash[!dir].tuple);
	
	/* Create expect */
	if ((exp = nf_ct_expect_alloc(ct)) == NULL)
		return NF_ACCEPT;

	nf_ct_expect_init(exp, NF_CT_EXPECT_CLASS_DEFAULT, AF_INET, NULL,
			  &ct->tuplehash[!dir].tuple.dst.u3, IPPROTO_UDP,
			  NULL, &ct->tuplehash[!dir].tuple.dst.u.udp.port);
	exp->flags = NF_CT_EXPECT_PERMANENT;
	exp->saved_addr = ct->tuplehash[dir].tuple.src.u3;
	exp->saved_proto.udp.port = ct->tuplehash[dir].tuple.src.u.udp.port;
	exp->dir = !dir;
	exp->expectfn = bcm_nat_expect;

	/* Setup expect */
	nf_ct_expect_related(exp);
	nf_ct_expect_put(exp);
	pr_debug("bcm_nat: expect setup\n");

	return NF_ACCEPT;
}

/****************************************************************************/
static struct nf_conntrack_expect_policy bcm_nat_exp_policy __read_mostly = {
	.max_expected 	= 1000,
	.timeout	= 240,
};

/****************************************************************************/
static struct nf_conntrack_helper nf_conntrack_helper_bcm_nat __read_mostly = {
	.name = "BCM-NAT",
	.me = THIS_MODULE,
	.tuple.src.l3num = AF_INET,
	.tuple.dst.protonum = IPPROTO_UDP,
	.expect_policy = &bcm_nat_exp_policy,
	.expect_class_max = 1,
	.help = bcm_nat_help,
};

/****************************************************************************/
static inline int find_exp(__be32 ip, __be16 port, struct nf_conn *ct)
{
	struct nf_conntrack_tuple tuple;
	struct nf_conntrack_expect *i = NULL;

	
	memset(&tuple, 0, sizeof(tuple));
	tuple.src.l3num = AF_INET;
	tuple.dst.protonum = IPPROTO_UDP;
	tuple.dst.u3.ip = ip;
	tuple.dst.u.udp.port = port;

	rcu_read_lock();
	i = __nf_ct_expect_find(nf_ct_net(ct), nf_ct_zone(ct), &tuple);
	rcu_read_unlock();

	return i != NULL;
}

/****************************************************************************/
static inline struct nf_conntrack_expect *find_fullcone_exp(struct nf_conn *ct)
{
	struct nf_conntrack_tuple * tp =
		&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	struct net *net = nf_ct_net(ct);
	struct nf_conntrack_expect * exp = NULL;
	struct nf_conntrack_expect * i;
	unsigned int h;

	rcu_read_lock();
	for (h = 0; h < nf_ct_expect_hsize; h++) {
		hlist_for_each_entry_rcu(i, &net->ct.expect_hash[h], hnode) {
			if (nf_inet_addr_cmp(&i->saved_addr, &tp->src.u3) &&
		    	    i->saved_proto.all == tp->src.u.all &&
		    	    i->tuple.dst.protonum == tp->dst.protonum &&
		    	    i->tuple.src.u3.ip == 0 &&
		    	    i->tuple.src.u.udp.port == 0) {
				exp = i;
				break;
			}
		}
	}
	rcu_read_unlock();

	return exp;
}
#endif /* CONFIG_KF_NETFILTER */


unsigned int
nf_nat_masquerade_ipv4(struct sk_buff *skb, unsigned int hooknum,
		       const struct nf_nat_range *range,
		       const struct net_device *out)
{
	struct nf_conn *ct;
	struct nf_conn_nat *nat;
	enum ip_conntrack_info ctinfo;
	struct nf_nat_range newrange;
	const struct rtable *rt;
	__be32 newsrc, nh;

	NF_CT_ASSERT(hooknum == NF_INET_POST_ROUTING);

	ct = nf_ct_get(skb, &ctinfo);
	nat = nfct_nat(ct);

	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED ||
			    ctinfo == IP_CT_RELATED_REPLY));

	/* Source address is 0.0.0.0 - locally generated packet that is
	 * probably not supposed to be masqueraded.
	 */
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == 0)
		return NF_ACCEPT;

	rt = skb_rtable(skb);
	nh = rt_nexthop(rt, ip_hdr(skb)->daddr);
	newsrc = inet_select_addr(out, nh, RT_SCOPE_UNIVERSE);
	if (!newsrc) {
		pr_info("%s ate my IP address\n", out->name);
		return NF_DROP;
	}

	nat->masq_index = out->ifindex;

#if defined(CONFIG_BCM_KF_NETFILTER)

/* RFC 4787 - 4.2.2.  Port Parity
   i.e., an even port will be mapped to an even port, and an odd port will be mapped to an odd port.
*/
#define CHECK_PORT_PARITY(a, b) ((a%2)==(b%2))

	if (range->min_addr.ip != 0 /* nat_mode == full cone */
	    && (nfct_help(ct) == NULL || nfct_help(ct)->helper == NULL)
	    && nf_ct_protonum(ct) == IPPROTO_UDP) {
		unsigned int ret;
		u_int16_t minport;
		u_int16_t maxport;
		struct nf_conntrack_expect *exp;

		pr_debug("bcm_nat: need full cone NAT\n");

		/* Choose port */
		spin_lock_bh(&nf_conntrack_expect_lock);
		/* Look for existing expectation */
		exp = find_fullcone_exp(ct);
		if (exp) {
			minport = maxport = exp->tuple.dst.u.udp.port;
			pr_debug("bcm_nat: existing mapped port = %hu\n",
			       	 ntohs(minport));
		} else { /* no previous expect */
			u_int16_t newport, tmpport, orgport;
			
			minport = range->min_proto.all == 0? 
				ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.
				u.udp.port : range->min_proto.all;
			maxport = range->max_proto.all == 0? 
				htons(65535) : range->max_proto.all;
                        orgport = ntohs(minport);
			for (newport = ntohs(minport),tmpport = ntohs(maxport); 
			     newport <= tmpport; newport++) {
			     	if (CHECK_PORT_PARITY(orgport, newport) && !find_exp(newsrc, htons(newport), ct)) {
                                        pr_debug("bcm_nat: new mapped port = "
					       	 "%hu\n", newport);
					minport = maxport = htons(newport);
					break;
				}
			}
		}
		spin_unlock_bh(&nf_conntrack_expect_lock);


	memset(&newrange.min_addr, 0, sizeof(newrange.min_addr));
	memset(&newrange.max_addr, 0, sizeof(newrange.max_addr));

		newrange.flags = range->flags | NF_NAT_RANGE_MAP_IPS |
			NF_NAT_RANGE_PROTO_SPECIFIED;
		newrange.max_addr.ip = newrange.min_addr.ip = newsrc;
		newrange.min_proto.udp.port = newrange.max_proto.udp.port = minport;
	
		/* Set ct helper */
		ret = nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_SRC);
		if (ret == NF_ACCEPT) {
			struct nf_conn_help *help = nfct_help(ct);
			if (help == NULL)
				help = nf_ct_helper_ext_add(ct, &nf_conntrack_helper_bcm_nat, GFP_ATOMIC);
			if (help != NULL) {
				help->helper = &nf_conntrack_helper_bcm_nat;
				pr_debug("bcm_nat: helper set\n");
			}
		}
		return ret;
	}
#endif /* CONFIG_KF_NETFILTER */

	/* Transfer from original range. */
	memset(&newrange.min_addr, 0, sizeof(newrange.min_addr));
	memset(&newrange.max_addr, 0, sizeof(newrange.max_addr));
	newrange.flags       = range->flags | NF_NAT_RANGE_MAP_IPS;
	newrange.min_addr.ip = newsrc;
	newrange.max_addr.ip = newsrc;
	newrange.min_proto   = range->min_proto;
	newrange.max_proto   = range->max_proto;

	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_SRC);
}
EXPORT_SYMBOL_GPL(nf_nat_masquerade_ipv4);

static int device_cmp(struct nf_conn *i, void *ifindex)
{
	const struct nf_conn_nat *nat = nfct_nat(i);

	if (!nat)
		return 0;
	if (nf_ct_l3num(i) != NFPROTO_IPV4)
		return 0;
	return nat->masq_index == (int)(long)ifindex;
}

static int masq_device_event(struct notifier_block *this,
			     unsigned long event,
			     void *ptr)
{
	const struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net *net = dev_net(dev);

	if (event == NETDEV_DOWN) {
		/* Device was downed.  Search entire table for
		 * conntracks which were associated with that device,
		 * and forget them.
		 */
		NF_CT_ASSERT(dev->ifindex != 0);

		nf_ct_iterate_cleanup(net, device_cmp,
				      (void *)(long)dev->ifindex, 0, 0);
	}

	return NOTIFY_DONE;
}

static int masq_inet_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr)
{
	struct in_device *idev = ((struct in_ifaddr *)ptr)->ifa_dev;
	struct netdev_notifier_info info;

	/* The masq_dev_notifier will catch the case of the device going
	 * down.  So if the inetdev is dead and being destroyed we have
	 * no work to do.  Otherwise this is an individual address removal
	 * and we have to perform the flush.
	 */
	if (idev->dead)
		return NOTIFY_DONE;

	netdev_notifier_info_init(&info, idev->dev);
	return masq_device_event(this, event, &info);
}

static struct notifier_block masq_dev_notifier = {
	.notifier_call	= masq_device_event,
};

static struct notifier_block masq_inet_notifier = {
	.notifier_call	= masq_inet_event,
};

static atomic_t masquerade_notifier_refcount = ATOMIC_INIT(0);

void nf_nat_masquerade_ipv4_register_notifier(void)
{
	/* check if the notifier was already set */
	if (atomic_inc_return(&masquerade_notifier_refcount) > 1)
		return;

	/* Register for device down reports */
	register_netdevice_notifier(&masq_dev_notifier);
	/* Register IP address change reports */
	register_inetaddr_notifier(&masq_inet_notifier);
}
EXPORT_SYMBOL_GPL(nf_nat_masquerade_ipv4_register_notifier);

void nf_nat_masquerade_ipv4_unregister_notifier(void)
{
#if defined(CONFIG_BCM_KF_NETFILTER)
	nf_conntrack_helper_unregister(&nf_conntrack_helper_bcm_nat);
#endif
	/* check if the notifier still has clients */
	if (atomic_dec_return(&masquerade_notifier_refcount) > 0)
		return;

	unregister_netdevice_notifier(&masq_dev_notifier);
	unregister_inetaddr_notifier(&masq_inet_notifier);
}
EXPORT_SYMBOL_GPL(nf_nat_masquerade_ipv4_unregister_notifier);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rusty Russell <rusty@rustcorp.com.au>");
