/*************************************************************************
 *
 * ivi_nf.c :
 *
 * MAP-T/MAP-E Packet Processing based on Netfilter 
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 Guoliang Han <bupthgl@gmail.com>
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#include <linux/notifier.h>
#include "ivi_nf.h"

struct net_device *v4_dev, *v6_dev;

int ivi_netdev_event_handler(struct notifier_block *this,
                            unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);

	switch (event) {

	case NETDEV_UNREGISTER:
		if (dev == v4_dev)
		{
			rcu_assign_pointer(v4_dev, NULL);
			dev_put(dev);
		}
		if (dev == v6_dev)
		{
			rcu_assign_pointer(v6_dev, NULL);
			dev_put(dev);
		}
		synchronize_rcu();
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block ivi_netdev_notifier = {
	.notifier_call = ivi_netdev_event_handler,
};

struct arpinfo {
	u_int8_t ar_sha[ETH_ALEN];
	__be32 ar_sip;
	u_int8_t ar_tha[ETH_ALEN];
	__be32 ar_tip;
} __attribute__ ((packed));

static int running;
static int transporting;

static unsigned int nf_hooka(void *priv,
                            struct sk_buff *skb,
    		                const struct nf_hook_state *state) {
	struct arphdr *arph = arp_hdr(skb);
	struct arpinfo *arpi;

	if ((!running) || (state->out != v6_dev)) {
		return NF_ACCEPT;
	}

	if ((arph->ar_hrd != htons(ARPHRD_ETHER)) ||
	    (arph->ar_pro != htons(ETH_P_IP)) ||
	    (arph->ar_pln != 4) || (arph->ar_hln != ETH_ALEN)) {
		return NF_ACCEPT;
	}

	arpi = (void *)(arph + 1);

	if (addr_is_v4address(&(arpi->ar_sip)) && (arph->ar_op == htons(ARPOP_REQUEST))) {
		return NF_DROP;
	}
	else {
		return NF_ACCEPT;
	}
}

static unsigned int nf_hook4(void *priv,
                            struct sk_buff *skb,
    		                const struct nf_hook_state *state) {
	unsigned int ret;
	struct net_device *dev4, *dev6;

	dev4 = rcu_dereference(v4_dev);
	dev6 = rcu_dereference(v6_dev);
	if ((!running) ||
	    (!dev6) || (!dev4) ||
	    ((state->hook == NF_INET_LOCAL_OUT) && (state->out != dev6)) ||
	    ((state->hook == NF_INET_PRE_ROUTING) && (state->in != dev4))) {
		return NF_ACCEPT;
	}

	ret = ivi_v4v6_xmit(skb, state->net, dev6->mtu, dev4->mtu);

	if (ret == 0)
		return NF_DROP;
	else if (ret == NF_STOLEN)
		return NF_STOLEN;
	else
		return NF_ACCEPT;
}

static unsigned int nf_hook6(void *priv,
                            struct sk_buff *skb,
    		                const struct nf_hook_state *state) {
	unsigned int ret;
	struct net_device *dev4, *dev6;

	dev4 = rcu_dereference(v4_dev);
	dev6 = rcu_dereference(v6_dev);
	if ((!running) || (!dev6) || (state->in != dev6)) {
		return NF_ACCEPT;
	}

	ret = ivi_v6v4_xmit(skb, state->net, dev6->mtu, dev4->mtu);

	if (ret == 0)
		return NF_DROP;
	else if (ret == NF_STOLEN)
		return NF_STOLEN;
	else
		return NF_ACCEPT;
}

struct nf_hook_ops ar_ops = {
	hook	:	(nf_hookfn *)nf_hooka,
	pf	:	NFPROTO_ARP,
	hooknum	:	NF_ARP_OUT,
	priority:	NF_IP_PRI_FIRST,
};

// Service for gateway its own needs
struct nf_hook_ops l4_ops = {
	hook	:	(nf_hookfn *)nf_hook4,
	pf	:	PF_INET,
	hooknum	:	NF_INET_LOCAL_OUT,
	priority:	NF_IP_PRI_FIRST,
};

struct nf_hook_ops v4_ops = {
	hook	:	(nf_hookfn *)nf_hook4,
	pf	:	PF_INET,
	hooknum	:	NF_INET_PRE_ROUTING,
	priority:	NF_IP_PRI_FIRST,
};

struct nf_hook_ops v6_ops = {
	hook	:	(nf_hookfn *)nf_hook6,
	pf	:	PF_INET6,
	hooknum	:	NF_INET_PRE_ROUTING,
	priority:	NF_IP6_PRI_FIRST,
};

int nf_running(const int run, const int transport) {
	if (((!running) && run) || (transporting != transport)) {
		u16 ratio, adjacent, psidoff;

		hgw_rand = 0;
		ratio = fls(hgw_ratio) - 1;
		if (ratio > 0) {
			adjacent = fls(hgw_adjacent) - 1;
			psidoff = 16 - ratio - adjacent;
			while (1) { // we want to generate an integer between [1, 2^psidoff - 1]
				get_random_bytes(&hgw_rand, sizeof(int));
				hgw_rand = (hgw_rand >= 0) ? hgw_rand : -hgw_rand;
				hgw_rand -= (hgw_rand >> psidoff) << psidoff;
				if (hgw_rand) break;
			}
		}
	}
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "nf_running: take random # %04x.\n", hgw_rand);
#endif
	transporting = transport;
	running = run;
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "nf_running: set running state to %d.\n", running);
#endif
	return running;
}

int nf_getv4dev(struct net_device *dev) {

	if (v4_dev)
		dev_put(v4_dev);

	rcu_assign_pointer(v4_dev, dev);
	synchronize_rcu();
	return 0;
}

int nf_getv6dev(struct net_device *dev) {

	if (v6_dev)
		dev_put(v6_dev);

	rcu_assign_pointer(v6_dev, dev);
	synchronize_rcu();
	return 0;
}

int ivi_nf_init(void) {
	running = 0;
	transporting = 0;
	v4_dev = NULL;
	v6_dev = NULL;

	nf_register_net_hook(&init_net, &ar_ops);
	nf_register_net_hook(&init_net, &l4_ops);
	nf_register_net_hook(&init_net, &v4_ops);
	nf_register_net_hook(&init_net, &v6_ops);

	register_netdevice_notifier(&ivi_netdev_notifier);

#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_nf loaded.\n");
#endif
	return 0;
}

void ivi_nf_exit(void) {
	running = 0;
	transporting = 0;

	unregister_netdevice_notifier(&ivi_netdev_notifier);

	nf_unregister_net_hook(&init_net, &ar_ops);
	nf_unregister_net_hook(&init_net, &l4_ops);
	nf_unregister_net_hook(&init_net, &v4_ops);
	nf_unregister_net_hook(&init_net, &v6_ops);
	
	if (v4_dev)
		dev_put(v4_dev);

	if (v6_dev)
		dev_put(v6_dev);

#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_nf unloaded.\n");
#endif
}
