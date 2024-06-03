/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifdef CONFIG_NETFILTER_FAMILY_BRIDGE

#include <linux/netfilter_bridge.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/if_bridge.h>

#include "br_private.h"

#if defined(CONFIG_BLOG)
#include <linux/bcm_br_fdbid.h>
#endif
#include "bcm_br_hooks_mcast.h"
#include "bcm_br_hooks_local_switching.h"
#include "bcm_br_hooks_maclimit.h"

static inline int bcm_br_pkt_type(struct sk_buff *skb)
{
    if (is_multicast_ether_addr(eth_hdr(skb)->h_dest)) {
        /* by definition the broadcast is also a multicast address */
        if (is_broadcast_ether_addr(eth_hdr(skb)->h_dest))
            return BR_PKT_BROADCAST;
        else
            return BR_PKT_MULTICAST;
    }

    return BR_PKT_UNICAST;
}

static unsigned int bcm_br_nf_pre_routing(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    unsigned int ret;

    ret = mcast_receive(skb);

    return ret;
}


static inline int bcm_br_blog_hook(struct sk_buff *skb, int hooknum)
{
#if defined(CONFIG_BLOG)
    struct net_device *brdev;
    struct net_bridge *br;
    struct net_bridge_fdb_entry *src = NULL;
    struct net_bridge_fdb_entry *dst = NULL;
    u16 vid = 0;
    enum br_pkt_type pkt_type = bcm_br_pkt_type(skb);

    struct net_bridge_vlan_group *vg;
    struct net_bridge_port *to;
    struct net_bridge_vlan *v;
    u16 blog_vid = 0;

    if (netif_is_bridge_port(skb->dev)) {
        struct net_bridge_port *p = br_port_get_rcu(skb->dev);
        br = p->br;
    } else if (netif_is_bridge_master(skb->dev)) {
        br = netdev_priv(skb->dev);
    } else {
        if (printk_ratelimit())
            printk("Broadcom Bridge Hook: unexpected device %s\n", skb->dev->name);
        return 0;
    }

    brdev = br->dev;

    br_vlan_get_tag(skb, &vid);

    /* link FDB */
    if (pkt_type == BR_PKT_UNICAST || pkt_type == BR_PKT_MULTICAST) {

        src = br_fdb_find_rcu(br, eth_hdr(skb)->h_source, vid);
        /* unlikely case: src is NULL in NF_BR_FORWARD/NF_BR_LOCAL_IN/NF_BR_LOCAL_OUT hooks.
           Note when we get here, br_handle_vlan has been done.
           In br_handle_vlan, skb->vlan_tci may be set as 0, which cause vid here is not same as vid in FDB.
           Fix this situation by using frame's vlan to search FDB table.
        */
        if (unlikely(!src)) {
            if (br_vlan_enabled(br->dev) && blog_ptr(skb) && (blog_ptr(skb)->vtag_num) && (skb->dev)) {
                blog_vid = ntohl(blog_ptr(skb)->vtag[0]) & VLAN_VID_MASK;

                if ((hooknum == NF_BR_FORWARD) || (hooknum == NF_BR_LOCAL_OUT)){
                    to = br_port_get_rcu(skb->dev);
                    if (to)
                        vg = nbp_vlan_group_rcu(to);
                }
                else if (hooknum == NF_BR_LOCAL_IN)
                    vg = br_vlan_group_rcu(br);

                v = br_vlan_find(vg, blog_vid);
                if (v && (v->flags & BRIDGE_VLAN_INFO_UNTAGGED)) {
                    vid = blog_vid;
                    src = br_fdb_find_rcu(br, eth_hdr(skb)->h_source, vid);
                }
            }
        }

        dst = br_fdb_find_rcu(br, eth_hdr(skb)->h_dest, vid);

        if ((src) && ((hooknum == NF_BR_FORWARD) || (hooknum == NF_BR_LOCAL_IN)))
            blog_link(BRIDGEFDB, blog_ptr(skb), (void *)src, BLOG_PARAM1_SRCFDB, br->dev->ifindex);

        /* LOCAL_IN, LOCAL_OUT, FORWARD
           Dst MAC learning does not apply for Multicast traffic */
        if (dst && pkt_type == BR_PKT_UNICAST)
            blog_link(BRIDGEFDB, blog_ptr(skb), (void *)dst, BLOG_PARAM1_DSTFDB, br->dev->ifindex);
    }

    /* TODO check if anything needs to be done(ex: link vlan dev) for CONFIG_BRIDGE_VLAN_FILTERING */
    if (hooknum == NF_BR_LOCAL_OUT)
        /* add ETH_HLEN as its stripped */
        blog_link(IF_DEVICE, blog_ptr(skb), (void *)brdev, DIR_TX, skb->len + ETH_HLEN);

    if (hooknum == NF_BR_LOCAL_IN)
        blog_link(IF_DEVICE, blog_ptr(skb), (void *)brdev, DIR_RX, skb->len);

    if (blog_ptr(skb) && BLOG_RX_UNKNOWN_UCAST(blog_ptr(skb)))
    {
        /* The bridge calls should_deliver() prior to cloning the skb/blog. So shared_info will
           be null when should_deliver is called for the first 2 ports. Check br_flood() logic
           for reference */
        blog_alloc_shared_info_if_null(skb);
        blog_inc_client_count(skb);
    }
#endif
    return 0;
}

static unsigned int bcm_br_nf_forward(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (bcm_br_local_switching_should_deliver(skb, state) == NF_DROP)
        return NF_DROP;

    bcm_br_blog_hook(skb, NF_BR_FORWARD);

    return NF_ACCEPT;
}

static unsigned int bcm_br_nf_local_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    bcm_br_blog_hook(skb, NF_BR_LOCAL_OUT);
    return NF_ACCEPT;
}

static unsigned int bcm_br_nf_local_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    unsigned int ret = NF_ACCEPT;
    
    bcm_br_blog_hook(skb, NF_BR_LOCAL_IN);
    return ret;
}

static struct nf_hook_ops bcm_br_nf_ops[] __read_mostly = {
    {
        .hook = bcm_br_nf_pre_routing,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_PRE_ROUTING,
        .priority = NF_BR_PRI_FIRST
    },
    {
        .hook = bcm_br_nf_forward,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_FORWARD,
        .priority = NF_BR_PRI_FIRST
    },
    {
        .hook = bcm_br_nf_local_out,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_LOCAL_OUT,
        .priority = NF_BR_PRI_FIRST
    },
    {
        .hook = bcm_br_nf_local_in,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_LOCAL_IN,
        .priority = NF_BR_PRI_FIRST
    },
};

static int __net_init bcm_br_nf_init(struct net *net)
{
    return nf_register_net_hooks(net, bcm_br_nf_ops, ARRAY_SIZE(bcm_br_nf_ops));
}

static void __net_exit bcm_br_nf_cleanup(struct net *net)
{
    nf_unregister_net_hooks(net, bcm_br_nf_ops, ARRAY_SIZE(bcm_br_nf_ops));
}

static struct pernet_operations bcm_br_net_ops = {
	.init = bcm_br_nf_init,
	.exit = bcm_br_nf_cleanup,
};

static int __init bcm_br_hooks_init(void)
{
    int ret;

    if ((ret = register_pernet_subsys(&bcm_br_net_ops)))
        return ret;

#if defined(CONFIG_BLOG)
    bcm_fdbid_tbl_init();
#endif
    printk(KERN_NOTICE "Bridge Broadcom hooks registered\n");
    return 0;
}

static void __exit bcm_br_hooks_cleanup(void)
{
    unregister_pernet_subsys(&bcm_br_net_ops);
}

module_init(bcm_br_hooks_init);
module_exit(bcm_br_hooks_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikolai Iosifov <nikolai.iosifov@broadcom.com>");
MODULE_DESCRIPTION("Broadcom bridge hooks");

#endif
