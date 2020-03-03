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

#include "ivi_nf.h"

struct net_device *v4_dev, *v6_dev;

static int running;

static unsigned int nf_hook4(const struct nf_hook_ops *ops,
                            struct sk_buff *skb,
    		                const struct nf_hook_state *state) {
	unsigned int ret;

	if ((!running) || (state->in != v4_dev)) {
		return NF_ACCEPT;
	}

	ret = ivi_v4v6_xmit(skb, v6_dev->mtu, v4_dev->mtu);

	if (ret == 0)
		return NF_DROP;
	else if (ret == NF_STOLEN)
		return NF_STOLEN;
	else
		return NF_ACCEPT;
}

static unsigned int nf_hook6(const struct nf_hook_ops *ops,
                            struct sk_buff *skb,
    		                const struct nf_hook_state *state) {
	unsigned int ret;

	if ((!running) || (state->in != v6_dev)) {
		return NF_ACCEPT;
	}

	ret = ivi_v6v4_xmit(skb);

	if (ret == 0)
		return NF_DROP;
	else if (ret == NF_STOLEN)
		return NF_STOLEN;
	else
		return NF_ACCEPT;
}

struct nf_hook_ops v4_ops = {
	list	:	{ NULL, NULL },
	hook	:	(nf_hookfn *)nf_hook4,
	owner	:	THIS_MODULE,
	pf	:	PF_INET,
	hooknum	:	NF_INET_PRE_ROUTING,
	priority:	NF_IP_PRI_FIRST,
};

struct nf_hook_ops v6_ops = {
	list	:	{ NULL, NULL },
	hook	:	(nf_hookfn *)nf_hook6,
	owner	:	THIS_MODULE,
	pf	:	PF_INET6,
	hooknum	:	NF_INET_PRE_ROUTING,
	priority:	NF_IP6_PRI_FIRST,
};

int nf_running(const int run) {
	running = run;
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "nf_running: set running state to %d.\n", running);
#endif
	return running;
}

int nf_getv4dev(struct net_device *dev) {
	v4_dev = dev;
	return 0;
}

int nf_getv6dev(struct net_device *dev) {
	v6_dev = dev;
	return 0;
}

int ivi_nf_init(void) {
	running = 0;
	v4_dev = NULL;
	v6_dev = NULL;

	nf_register_hook(&v4_ops);
	nf_register_hook(&v6_ops);

#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_nf loaded.\n");
#endif
	return 0;
}

void ivi_nf_exit(void) {
	running = 0;

	nf_unregister_hook(&v4_ops);
	nf_unregister_hook(&v6_ops);
	
	if (v4_dev)
		dev_put(v4_dev);

	if (v6_dev)
		dev_put(v6_dev);

#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_nf unloaded.\n");
#endif
}
