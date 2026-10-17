/*
 * <:copyright-BRCM:2024:DUAL/GPL:standard
 *
 *    Copyright (c) 2024 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */
#include <linux/netfilter_bridge.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <net/ip.h>
#include <net/netlink.h>
#include <net/netfilter/nf_conntrack.h>
#include <uapi/linux/netfilter/nfnetlink_conntrack.h>
#include <linux/kqoe.h>
#include "kqoe_local.h"

/* --- variables and functions --- */
static unsigned int kq_dscp_nf_pre_hook(void *priv, struct sk_buff *skb,
					const struct nf_hook_state *state);
static unsigned int kq_dscp_nf_post_hook(void *priv, struct sk_buff *skb,
					 const struct nf_hook_state *state);

#define DECLARE_HOOK(_fun, _pf, _hooknum, _priority) \
	{ \
		.hook		= _fun, \
		.pf		= _pf, \
		.hooknum	= _hooknum, \
		.priority	= _priority, \
	}

static struct nf_hook_ops hooks[] __read_mostly = {
	DECLARE_HOOK(kq_dscp_nf_pre_hook,	NFPROTO_IPV4,	NF_INET_PRE_ROUTING,	NF_IP_PRI_FILTER + 1),
	DECLARE_HOOK(kq_dscp_nf_post_hook,	NFPROTO_IPV4,	NF_INET_POST_ROUTING,	NF_IP_PRI_LAST),
	DECLARE_HOOK(kq_dscp_nf_pre_hook,	NFPROTO_BRIDGE,	NF_BR_PRE_ROUTING,	NF_BR_PRI_FILTER_BRIDGED + 1),
	DECLARE_HOOK(kq_dscp_nf_post_hook,	NFPROTO_BRIDGE,	NF_BR_POST_ROUTING,	NF_BR_PRI_LAST)
};

/* --- functions --- */
static unsigned int
kq_dscp_nf_pre_hook(void *priv, struct sk_buff *skb,
		    const struct nf_hook_state *state)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;

	if (unlikely(!skb || !skb->dev))
		goto out;

	ct = nf_ct_get(skb, &ctinfo);
	if (!ct)
		goto out;

#if IS_ENABLED(CONFIG_NF_MSCS)
	if (test_bit(CTA_QOS_F_MSCS_BIT, &ct->bcm_ext.qos.flags))
		goto out;
#endif

out:
	return NF_ACCEPT;
}

static unsigned int
kq_dscp_nf_post_hook(void *priv, struct sk_buff *skb,
		     const struct nf_hook_state *state)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	struct iphdr *iph;
	u8 dscp;

	if (unlikely(!skb || !skb->dev))
		goto out;

	ct = nf_ct_get(skb, &ctinfo);
	if (!ct)
		goto out;

#if IS_ENABLED(CONFIG_NF_MSCS)
	if (!test_bit(CTA_QOS_F_MSCS_BIT, &ct->bcm_ext.qos.flags) &&
	    !is_netdev_wan(skb->dev) && GET_WLAN_PRIORITY_OVRD(skb)) {
		set_bit(CTA_QOS_F_MSCS_BIT, &ct->bcm_ext.qos.flags);
		ct->bcm_ext.qos.in_tid = ct->bcm_ext.mscs.priority;
		ct->bcm_ext.qos.eg_tid = ct->bcm_ext.mscs.priority;
	}

	if (test_bit(CTA_QOS_F_MSCS_BIT, &ct->bcm_ext.qos.flags))
		goto out;
#endif

	iph = ip_hdr(skb);

	/* rewrite dscp */
	if (test_bit(CTA_QOS_F_DSCP_SET_BIT, &ct->bcm_ext.qos.flags)) {
		dscp = ct->bcm_ext.qos.dscp;
		if (dscp != iph->tos) {
			iph->tos = dscp;
			ip_send_check(iph);
		}
	} else {
		ct->bcm_ext.qos.dscp = iph->tos;
	}

out:
	return NF_ACCEPT;
}

int __init kq_dscp_init(void)
{
	int ret;

	ret = nf_register_net_hooks(&init_net, hooks, ARRAY_SIZE(hooks));
	if (ret < 0) {
		pr_err("cannot register netfilter hooks\n");
		goto err;
	}

	return 0;

err:
	return ret;
}

void kq_dscp_exit(void)
{
	nf_unregister_net_hooks(&init_net, hooks, ARRAY_SIZE(hooks));
}
