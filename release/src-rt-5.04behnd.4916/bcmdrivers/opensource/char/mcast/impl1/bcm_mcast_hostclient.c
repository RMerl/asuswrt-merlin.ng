/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

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

#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#if defined(CONFIG_IPV6)
#include <linux/netfilter_ipv6.h>
#include <linux/ipv6.h>
#endif
#include "bcm_mcast_priv.h"
int hostclient_accel_enabled = 0;

/* This function is called from bcm_mcast_nf_hostclient_in()/bcm_mcast_nf_hostclient_in6()
   This hook gets invoked for the locally delivered packets coming in from both the
   bridge ports and routed ports. */
void bcm_mcast_nf_hook_processing(bcm_mcast_ipaddr_t *ipaddr,
                                  struct sk_buff *skb)
{
    bcm_mcast_ipaddr_t srcaddr_zero = {0};

    if (!skb->blog_p)
    {
        /* No updates to blog shared info needed */
        return;
    }

    /* Check if host client present */
    if (bcm_mcast_hostclient_entry_lookup(ipaddr, &srcaddr_zero))
    {
        blog_inc_client_count(skb);
        blog_inc_host_client_count(skb);
        if (!hostclient_accel_enabled)
        {
            /* Host client acceleration not enabled, free blog.
               Learning will not complete since acceleration is
               not enabled. Packets to host client will take
               slow path */
            blog_skip(skb, blog_skip_reason_known_exception_client);
        }
        /* else do nothing. Acceleration is enabled. blog_emit()
           will be called later in the udp layer */
    }
    else
    {
        /* No host client present, complete learning */
        blog_free(skb, blog_free_reason_known_exception_client);
    }
}

static unsigned int bcm_mcast_nf_hostclient_in(void *priv,
			struct sk_buff *skb,
			const struct nf_hook_state *state)
{
    struct iphdr *pip = ip_hdr(skb);
    bcm_mcast_ipaddr_t ipaddr;

    if (skb->pkt_type == PACKET_MULTICAST)
    {
        //printk("%s invoked skb->pkt_type %d ip dst %pI4\n", __func__, skb->pkt_type, &pip->daddr);
        ipaddr.is_ipv4 = 1;
        ipaddr.ipv4_addr.s_addr = pip->daddr;
        bcm_mcast_nf_hook_processing(&ipaddr, skb);
    }
    return NF_ACCEPT;
}

#if defined(CONFIG_IPV6)
static unsigned int bcm_mcast_nf_hostclient_in6(void *priv,
			struct sk_buff *skb,
			const struct nf_hook_state *state)
{
    struct ipv6hdr *pip6 = ipv6_hdr(skb);
    bcm_mcast_ipaddr_t ipaddr;

    if (skb->pkt_type == PACKET_MULTICAST)
    {
        //printk("%s invoked skb->pkt_type %d ip dst %pI6\n", __func__, skb->pkt_type, &pip6->daddr);
        ipaddr.is_ipv4 = 0;
        BCM_MCAST_IN6_ASSIGN_ADDR(&ipaddr.ipv6_addr, &pip6->daddr);
        bcm_mcast_nf_hook_processing(&ipaddr, skb);
    }
    return NF_ACCEPT;
}
#endif

static struct nf_hook_ops bcm_mcast_nf_hostclient_ops[] = {
    {
        .hook = bcm_mcast_nf_hostclient_in,
        .pf = PF_INET,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP_PRI_MANGLE + 10,/* do it between mangle and filter table */
    },
#if defined(CONFIG_IPV6)
    {
        .hook = bcm_mcast_nf_hostclient_in6,
        .pf = PF_INET6,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP6_PRI_MANGLE + 10,/* do it between mangle and filter table */
    },
#endif
};

int bcm_mcast_hostclient_init(void)
{
    int ret = 0;
#if defined(CC_MCAST_HOST_CLIENT_SUPPORT)
    int i;
#endif
    ret = nf_register_net_hooks(&init_net, bcm_mcast_nf_hostclient_ops,
                                ARRAY_SIZE(bcm_mcast_nf_hostclient_ops));
    if (ret < 0) 
    {
        printk("bcm_mcast_hostclient_init: can't register hooks.\n");
        return ret;
    }

#if defined(CC_MCAST_HOST_CLIENT_SUPPORT)
    mcast_ctrl->hostclient_cache = kmem_cache_create("bcm_mcast_host_client_cache",
                                                     sizeof(t_hostclient_entry),
                                                     0,
                                                     SLAB_HWCACHE_ALIGN, NULL);
    if (NULL == mcast_ctrl->hostclient_cache)
    {
        printk("failed to allocate hostclient_cache\n");
        return -ENOMEM;
    }

    spin_lock_init(&mcast_ctrl->hostclient_lock);
    for (i = 0; i < BCM_MCAST_HASH_SIZE; i++ )
    {
        INIT_HLIST_HEAD(&mcast_ctrl->hostclient_hash[i]);
    }
#endif
    return ret;
}

void bcm_mcast_hostclient_exit(void)
{
    nf_unregister_net_hooks(&init_net, bcm_mcast_nf_hostclient_ops, ARRAY_SIZE(bcm_mcast_nf_hostclient_ops));
}

#if defined(CC_MCAST_HOST_CLIENT_SUPPORT)
static inline t_hostclient_entry *bcm_mcast_hostclient_entry_alloc(void)
{
    t_hostclient_entry *hc_entry_p = NULL;

    hc_entry_p = kmem_cache_alloc(mcast_ctrl->hostclient_cache, GFP_ATOMIC);
    if ( !hc_entry_p )
    {
        __logError("Unable to allocate hostclient entry");
        return NULL;
    }
    memset(hc_entry_p, 0, sizeof(*hc_entry_p));
    return hc_entry_p;
}

static inline void bcm_mcast_hostclient_entry_free(t_hostclient_entry *hc_entry_p)
{
    kmem_cache_free(mcast_ctrl->hostclient_cache, hc_entry_p);
}

t_hostclient_entry *bcm_mcast_hostclient_entry_lookup(bcm_mcast_ipaddr_t *grp, 
                                                      bcm_mcast_ipaddr_t *src)
{
    struct hlist_head *head;
    t_hostclient_entry *hc_entry_p;

    spin_lock_bh(&mcast_ctrl->hostclient_lock);
    head = &mcast_ctrl->hostclient_hash[bcm_mcast_ip_hash(grp)];
    hlist_for_each_entry(hc_entry_p, head, hlist)
    {
        if (bcm_mcast_match_ipaddr(&hc_entry_p->grp, grp) &&
            bcm_mcast_match_ipaddr(&hc_entry_p->src, src))
        {
            spin_unlock_bh(&mcast_ctrl->hostclient_lock);
            return hc_entry_p;
        }
    }
    spin_unlock_bh(&mcast_ctrl->hostclient_lock);
    return NULL;
}

void bcm_mcast_hostclient_entry_dump(t_hostclient_entry *hc_entry_p)
{
#if defined(CC_MCAST_WHITELIST_SUPPORT)
    int i;
#endif

    if (hc_entry_p->grp.is_ipv4)
    {
        printk("grp %pI4 src %pI4 refcnt %d\n",
               &hc_entry_p->grp.ipv4_addr, &hc_entry_p->src.ipv4_addr, hc_entry_p->refcnt);
    }
    else
    {
        printk("grp %pI6 src %pI6 refcnt %d\n",
               &hc_entry_p->grp.ipv6_addr, &hc_entry_p->src.ipv6_addr, hc_entry_p->refcnt);
    }

#if defined(CC_MCAST_WHITELIST_SUPPORT)
    for (i = 0; i < hc_entry_p->hc_wl_info.ifidx_cnt; i++)
    {
        if (i == 0)
        {
            printk("ifidx :");
        }
        printk(" %d", hc_entry_p->hc_wl_info.ifidx[i]);
        if (i == (hc_entry_p->hc_wl_info.ifidx_cnt - 1))
        {
            printk("\n");
        }
    }
#endif
}

void bcm_mcast_hostclient_entries_dump(void)
{
    t_hostclient_entry *hc_entry_p;
    int i = 0;
    int hc_cnt=0;

    spin_lock_bh(&mcast_ctrl->hostclient_lock);
    for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
    {
        hlist_for_each_entry(hc_entry_p, &mcast_ctrl->hostclient_hash[i], hlist) 
        {
            bcm_mcast_hostclient_entry_dump(hc_entry_p);
            hc_cnt++;
        }
    }
    if (hc_cnt == 0)
    {
        printk("No Host Client Entries found\n");
    }
    spin_unlock_bh(&mcast_ctrl->hostclient_lock);
}

void bcm_mcast_hostclient_fill_blog(Blog_t *blog_p, 
                                    BlogTraffic_t proto, 
                                    t_hostclient_entry *hc_entry_p)
{
    blog_p->is_ssm = hc_entry_p->is_ssm;

    if ( BlogTraffic_IPV4_MCAST == proto )
    {
        blog_p->rx.tuple.saddr = hc_entry_p->src.ipv4_addr.s_addr;
        blog_p->rx.tuple.daddr = hc_entry_p->grp.ipv4_addr.s_addr;
    }

    if ( BlogTraffic_IPV6_MCAST == proto )
    {
        BCM_MCAST_IN6_ASSIGN_ADDR(&blog_p->tupleV6.saddr, &hc_entry_p->src.ipv6_addr);
        BCM_MCAST_IN6_ASSIGN_ADDR(&blog_p->tupleV6.daddr, &hc_entry_p->grp.ipv6_addr);
    }
}

#if defined(CC_MCAST_WHITELIST_SUPPORT)
int bcm_mcast_hostclient_check_if_whitelist_exists_for_ifidx(t_hostclient_entry *hc_entry_p,
                                                             unsigned int ifidx)
{
    int i = 0;

    for (i = 0; i < hc_entry_p->hc_wl_info.ifidx_cnt; i++)
    {
        if (hc_entry_p->hc_wl_info.ifidx[i] == ifidx)
        {
            __logInfo("whitelist already exists for ifidx %d", ifidx);
            return 1;
        }
    }
    return 0;
}

int bcm_mcast_hostclient_whitelist_entry_add(bcm_mcast_ipaddr_t *grp,
                                             bcm_mcast_ipaddr_t *src,
                                             t_BCM_MCAST_WAN_INFO *waninfo_p,
                                             t_hostclient_entry *hc_entry_p)
{
    struct net_device *from_dev = NULL;
    int dbgVar=0;
    int idx = 0;

    for(idx = 0; idx < BCM_MCAST_MAX_SRC_IF; idx++)
    {
        if(waninfo_p[idx].if_ops)
        {
            dbgVar = 1;

            if (bcm_mcast_hostclient_check_if_whitelist_exists_for_ifidx(hc_entry_p,
                                                                         waninfo_p[idx].ifi))
            {
                continue;
            }

            from_dev = dev_get_by_index(&init_net, 
                                        waninfo_p[idx].ifi);

            if (NULL == from_dev)
                continue;

            if (0 == (from_dev->flags & IFF_UP))
            {
                dev_put(from_dev);
                continue;
            }

            __logInfo("Adding whitelist nodes for from_dev %s ifidx %d", 
                      from_dev->name, waninfo_p[idx].ifi);
            if (bcm_mcast_whitelist_nodes_add(from_dev, 
                                              waninfo_p[idx].mcast_vlan,
                                              grp, 
                                              src, 
                                              &(hc_entry_p->hc_wl_info.whitelist_info)) != 0)
            {
                __logError("Error adding whitelist nodes");
                dev_put(from_dev);
                return -1;
            }
            hc_entry_p->hc_wl_info.ifidx[hc_entry_p->hc_wl_info.ifidx_cnt++] = waninfo_p[idx].ifi;
       
            dev_put(from_dev);
        }
    }

    if (!dbgVar) 
    {
        __logInfo("No entries in WAN info array");
    }
    return 0;
}
#endif

void bcm_mcast_hostclient_update_flow(t_hostclient_entry *hc_entry_p,
                                      int isAdd)
{
    Blog_t blog;
    Blog_t *blog_p = &blog;
    BlogTraffic_t proto;

    /* Set fwd and trap flag for this group in fcache */
    if (hc_entry_p->grp.is_ipv4)
    {
        proto = BlogTraffic_IPV4_MCAST;
    }
    else
    {
        proto = BlogTraffic_IPV6_MCAST;
    }

    bcm_mcast_hostclient_fill_blog(blog_p, proto, hc_entry_p);

    /* Set/Reset host_client flag in fcache so that packets
       destined for this group are trapped/not trapped to host */
    blog_p->host_client_add = isAdd;
    blog_host_client_config(blog_p, proto);
}

int bcm_mcast_hostclient_entry_add(bcm_mcast_ipaddr_t *grp,
                                   bcm_mcast_ipaddr_t *src,
                                   t_BCM_MCAST_WAN_INFO *waninfo_p)
{
    struct hlist_head *head;
    t_hostclient_entry *hc_entry_p;

    bcm_mcast_log_ipaddr_info(grp);
    bcm_mcast_log_ipaddr_info(src);

    /* lookup function already has a lock inside */
    hc_entry_p = bcm_mcast_hostclient_entry_lookup(grp, src);

    spin_lock_bh(&mcast_ctrl->hostclient_lock);
    if (hc_entry_p == NULL)
    {
        head = &(mcast_ctrl->hostclient_hash[bcm_mcast_ip_hash(grp)]);

        hc_entry_p = bcm_mcast_hostclient_entry_alloc();
        if (hc_entry_p == NULL)
        {
            __logError("Unable to allocate hostclient entry");
            spin_unlock_bh(&mcast_ctrl->hostclient_lock);
            return -1;
        }

        bcm_mcast_assign_ipaddr(&hc_entry_p->grp, grp);
        bcm_mcast_assign_ipaddr(&hc_entry_p->src, src);
        if (bcm_mcast_is_ipaddr_zero(&hc_entry_p->src))
        {
            hc_entry_p->is_ssm = 1;
        }
        hc_entry_p->refcnt = 1;
        __logInfo("host client entry not found, creating... refcnt %d", hc_entry_p->refcnt);

        hlist_add_head(&hc_entry_p->hlist, head);

        bcm_mcast_hostclient_update_flow(hc_entry_p, 1 /*Add membership*/);
    }
    else
    {
        hc_entry_p->refcnt++;

        __logInfo("host client entry found refcnt %d", hc_entry_p->refcnt);
    }

#if defined(CC_MCAST_WHITELIST_SUPPORT)
    if ( bcm_mcast_whitelist_add_fn ) 
    {
        bcm_mcast_hostclient_whitelist_entry_add(grp,
                                                 src,
                                                 waninfo_p,
                                                 hc_entry_p);
    }
#endif
    spin_unlock_bh(&mcast_ctrl->hostclient_lock);
    return 0;
}

void bcm_mcast_hostclient_lkup_and_update_flow_igmp(struct in_addr *grp,
                                                    struct in_addr *src)
{
    bcm_mcast_ipaddr_t grp_addr;
    bcm_mcast_ipaddr_t src_addr;
    t_hostclient_entry *hc_entry_p;

    grp_addr.is_ipv4 = 1;
    grp_addr.ipv4_addr.s_addr = grp->s_addr;
    src_addr.ipv4_addr.s_addr = src->s_addr;

    /* lookup function already has a lock inside */
    hc_entry_p = bcm_mcast_hostclient_entry_lookup(&grp_addr, &src_addr);

    if (hc_entry_p)
    {
        __logInfo("hostclient entry found for grp %pI4 src %pI4. Setting flw fwdandtrap", 
                  &grp_addr.ipv4_addr, &src_addr.ipv4_addr);
        bcm_mcast_hostclient_update_flow(hc_entry_p, 1/*isAdd*/);
    }
    else
    {
        __logInfo("hostclient entry NOT found for grp %pI4 src %pI4", 
                  &grp_addr.ipv4_addr, &src_addr.ipv4_addr);
    }
}

void bcm_mcast_hostclient_lkup_and_update_flow_mld(struct in6_addr *grp,
                                                   struct in6_addr *src)
{
    bcm_mcast_ipaddr_t grp_addr;
    bcm_mcast_ipaddr_t src_addr;
    t_hostclient_entry *hc_entry_p;

    grp_addr.is_ipv4 = 0;
    BCM_MCAST_IN6_ASSIGN_ADDR(&grp_addr.ipv6_addr, grp);
    BCM_MCAST_IN6_ASSIGN_ADDR(&src_addr.ipv6_addr, src);

    /* lookup function already has a lock inside */
    hc_entry_p = bcm_mcast_hostclient_entry_lookup(&grp_addr, &src_addr);

    if (hc_entry_p)
    {
        __logInfo("hostclient entry found for grp %pI6 src %pI6. Setting flw fwdandtrap", 
                  &grp_addr.ipv6_addr, &src_addr.ipv6_addr);
        bcm_mcast_hostclient_update_flow(hc_entry_p, 1/*isAdd*/);
    }
    else
    {
        __logInfo("hostclient entry NOT found for grp %pI6 src %pI6", 
                  &grp_addr.ipv6_addr, &src_addr.ipv6_addr);
    }
}

int bcm_mcast_hostclient_entry_remove(bcm_mcast_ipaddr_t *grp,
                                      bcm_mcast_ipaddr_t *src)
{
    t_hostclient_entry *hc_entry_p;

    bcm_mcast_log_ipaddr_info(grp);
    bcm_mcast_log_ipaddr_info(src);

    /* lookup function already has a lock inside */
    hc_entry_p = bcm_mcast_hostclient_entry_lookup(grp, src);

    spin_lock_bh(&mcast_ctrl->hostclient_lock);
    if (hc_entry_p == NULL)
    {
        __logInfo("host client entry not found");
        spin_unlock_bh(&mcast_ctrl->hostclient_lock);
        return -1;
    }

    if (hc_entry_p->refcnt <= 0)
    {
        __logError("host entry refcnt invalid %d", hc_entry_p->refcnt);
        spin_unlock_bh(&mcast_ctrl->hostclient_lock);
        return -1;
    }

    if (hc_entry_p->refcnt > 1)
    {
        hc_entry_p->refcnt--;
        __logInfo("host client entry found decremented refcnt %d", hc_entry_p->refcnt);
        spin_unlock_bh(&mcast_ctrl->hostclient_lock);
        return 0;
    }

    /* refcnt = 1. delete whitelist and free host client entry */
#if defined(CC_MCAST_WHITELIST_SUPPORT)
    if ( bcm_mcast_whitelist_delete_fn )
    {
        bcm_mcast_whitelist_nodes_del(&(hc_entry_p->hc_wl_info.whitelist_info));
    }
#endif
    hlist_del(&hc_entry_p->hlist);

    /* Reset fwd and trap flag for this group in fcache */
    bcm_mcast_hostclient_update_flow(hc_entry_p, 0 /*isAdd*/);

    bcm_mcast_hostclient_entry_free(hc_entry_p);
    spin_unlock_bh(&mcast_ctrl->hostclient_lock);
    return 0;
}
#else
t_hostclient_entry *bcm_mcast_hostclient_entry_lookup(bcm_mcast_ipaddr_t *grp, 
                                                      bcm_mcast_ipaddr_t *src)
{
    return 0;
}
#endif
