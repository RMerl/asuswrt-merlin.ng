/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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

#include "bcm_mcast_priv.h"
#include <linux/blog_rule.h>
#include "bcm_mcast_blogrule.h"
#include "bcm_mcast_whitelist.h"

#if defined(CC_MCAST_WHITELIST_SUPPORT)
#include <rdpa_api.h>

static struct kmem_cache *whitelist_cache = NULL;
bcm_mcast_whitelist_add_hook_t bcm_mcast_whitelist_add_fn = bcm_mcast_whitelist_add;
bcm_mcast_whitelist_delete_hook_t bcm_mcast_whitelist_delete_fn = bcm_mcast_whitelist_delete;
static bdmf_object_handle mcast_whitelist_class = NULL;
struct list_head mcast_whitelist;
struct list_head *whitelist_head_p = &mcast_whitelist;
#ifdef CONFIG_BCM_RUNNER_IPTV
static bdmf_object_handle iptv = NULL;
#endif
void bcm_mcast_dump_whitelist(struct seq_file *seq)
{
    bcm_mcast_whitelist_node_t *node_p;

    seq_printf(seq, "Whitelist entries:\n");
    list_for_each_entry(node_p, whitelist_head_p, list)
    {
        if (node_p->grp.is_ipv4)
        {
            seq_printf(seq, "\toutervlanid 0x%x grp %pI4 src %pI4 refcnt %d\n", 
                       node_p->outer_vlanid, &node_p->grp.ipv4_addr, &node_p->src.ipv4_addr, node_p->refcnt);
        }
        else
        {
            seq_printf(seq, "\toutervlanid 0x%x grp %pI6 src %pI6 refcnt %d\n", 
                       node_p->outer_vlanid, &node_p->grp.ipv6_addr, &node_p->src.ipv6_addr, node_p->refcnt);
        }
    }
}

static rdpa_iptv_lookup_method bcm_mcast_whitelist_get_key(void)
{
    int rc = 0;
    rdpa_iptv_lookup_method lookup_method = iptv_lookup_method_group_ip_src_ip_vid;

#ifdef CONFIG_BCM_RUNNER_IPTV
    if (unlikely(iptv == NULL))
    {
        __logError("iptv NULL");
        return lookup_method;
    }
    rc = rdpa_iptv_lookup_method_get(iptv, &lookup_method);
    if (rc)
    {
        __logError("rdpa_iptv_lookup_method_get returns rc = %d\n", rc);
    }
#else
    if (unlikely(mcast_whitelist_class == NULL))
    {
        __logError("mcast_whitelist_class NULL");
        return lookup_method;
    }
    rc = rdpa_mcast_whitelist_lookup_method_get(mcast_whitelist_class, &lookup_method);
    if ((rc) && (rc != BDMF_ERR_NOT_SUPPORTED))
    {
        __logError("rdpa_mcast_whitelist_lookup_method_get returns rc = %d\n", rc);
    }
#endif

    return (rc)?iptv_lookup_method_group_ip_src_ip_vid:lookup_method;

}

static inline int bcm_mcast_whitelist_is_vlan_key(rdpa_iptv_lookup_method lookup_method)
{
    return (lookup_method == iptv_lookup_method_group_ip_src_ip_vid);
}

static void bcm_mcast_build_whitelist_key(bcm_mcast_whitelist_node_t *node_p, 
                                          rdpa_mcast_whitelist_t *mcast_wlist)
{
    rdpa_iptv_lookup_method lookup_method;
    uint16_t vlan_id;

    if ( node_p->grp.is_ipv4 )
    {
        mcast_wlist->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
        mcast_wlist->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
        if (node_p->is_ssm) 
        {
            /* Use source to lookup only for SSM flows */
            mcast_wlist->mcast_group.l3.src_ip.addr.ipv4 = ntohl(node_p->src.ipv4_addr.s_addr);
        }
        else
        {
            mcast_wlist->mcast_group.l3.src_ip.addr.ipv4 = 0;
        }
        mcast_wlist->mcast_group.l3.gr_ip.addr.ipv4 = ntohl(node_p->grp.ipv4_addr.s_addr);
    }
    else
    {
        mcast_wlist->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
        mcast_wlist->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
        if (node_p->is_ssm) 
        {
            /* Use source to lookup only for SSM flows */
            memcpy(mcast_wlist->mcast_group.l3.src_ip.addr.ipv6.data, node_p->src.ipv6_addr.s6_addr, 16);
        }
        else
        {
            memset(mcast_wlist->mcast_group.l3.src_ip.addr.ipv6.data, 0, 16); 
        }
        memcpy(mcast_wlist->mcast_group.l3.gr_ip.addr.ipv6.data, node_p->grp.ipv6_addr.s6_addr, 16);
    }

    lookup_method = bcm_mcast_whitelist_get_key();
    if (bcm_mcast_whitelist_is_vlan_key(lookup_method))
    {
        vlan_id = node_p->outer_vlanid;
        mcast_wlist->vid = vlan_id & RDPA_VID_MASK;
    }
}

int bcm_mcast_whitelist_add(bcm_mcast_whitelist_node_t *node_p)
{
    rdpa_mcast_whitelist_t mcast_wlist;
    int rc = 0;
    bdmf_index rdpa_index = 0;

    if ( node_p->grp.is_ipv4 )
    {
        __logDebug("is_ssm %d grp %pI4 src %pI4 vid %u",
                   node_p->is_ssm,
                   &node_p->grp.ipv4_addr, 
                   &node_p->src.ipv4_addr,
                   node_p->outer_vlanid );
    }
    else
    {
        __logDebug("is_ssm %d grp %pI6 src %pI6 vid %u",
                   node_p->is_ssm,
                   &node_p->grp.ipv6_addr,
                   &node_p->src.ipv6_addr,
                   node_p->outer_vlanid );
    }

    if (mcast_whitelist_class == NULL)
    {
        __logError("mcast_whitelist_class NULL");
        return -1;
    }

    memset(&mcast_wlist, 0, sizeof(mcast_wlist));
    bcm_mcast_build_whitelist_key(node_p, &mcast_wlist);

    bdmf_lock();

    rc = rdpa_mcast_whitelist_entry_add(mcast_whitelist_class, &rdpa_index, &mcast_wlist);
    if (rc)
    {
        bdmf_unlock();
        __logError("Could not rdpa_mcast_whitelist_entry_add, rc = %d", rc);
        return -1;
    }

    bdmf_unlock();
    __logDebug("Successfully added whitelist entry rdpa_index 0x%x", rdpa_index);
    node_p->whitelist_key = (whitelist_key_t)rdpa_index;

    return 0;
}

int bcm_mcast_whitelist_delete(whitelist_key_t whitelist_hdl)
{
    int rc = 0;
    bdmf_index rdpa_index = (bdmf_index)whitelist_hdl;

    if (mcast_whitelist_class == NULL)
        return -1;

    __logDebug("whitelist hdl = %u", whitelist_hdl);
    bdmf_lock();

    rc = rdpa_mcast_whitelist_entry_delete(mcast_whitelist_class, rdpa_index);
    if (rc)
    {
        __logError("Cannot rdpa_mcast_whitelist_entry_delete, rc = %d\n", rc);
    }

    bdmf_unlock();

    return rc;
}

static inline bcm_mcast_whitelist_node_t* bcm_mcast_whitelist_node_alloc(void)
{
    bcm_mcast_whitelist_node_t *whitelist_node_p = NULL;

    whitelist_node_p = kmem_cache_alloc(whitelist_cache, GFP_ATOMIC);
    if ( !whitelist_node_p )
    {
        __logError("Unable to allocate whitelist node");
        return NULL;
    }
    return whitelist_node_p;
}

static inline void bcm_mcast_whitelist_node_free(bcm_mcast_whitelist_node_t *whitelist_node_p)
{
    kmem_cache_free(whitelist_cache, whitelist_node_p);
}

static inline bcm_mcast_whitelist_node_t* bcm_mcast_whitelist_node_lookup(uint32_t vid,
                                                                          bcm_mcast_ipaddr_t *grp,
                                                                          bcm_mcast_ipaddr_t *src)
{
    bcm_mcast_whitelist_node_t *node_p;
    rdpa_iptv_lookup_method lookup_method;
    int is_vlan_key = 0;

    lookup_method = bcm_mcast_whitelist_get_key();
    is_vlan_key = bcm_mcast_whitelist_is_vlan_key(lookup_method);

    __logDebug("is_vlan_key %d, vid = %u whitelist_head_p = 0x%px",
        is_vlan_key, vid, whitelist_head_p);

    list_for_each_entry(node_p, whitelist_head_p, list)
    {
        if ( node_p && 
             ((is_vlan_key &&
             (node_p->outer_vlanid == vid)) ||
             (!is_vlan_key)) &&
             bcm_mcast_match_ipaddr(grp, &node_p->grp) &&
             bcm_mcast_match_ipaddr(src, &node_p->src) )
        {
            __logDebug("FOUND : vid = %u node_p = 0x%px",vid, node_p);
            return node_p;
        }
    }
    __logDebug("Not FOUND : vid = %u whitelist_head_p = 0x%px",vid, whitelist_head_p);
    return NULL;
}

bcm_mcast_whitelist_node_t* bcm_mcast_whitelist_node_add(uint32_t outervlanid, 
                                                         bcm_mcast_ipaddr_t *grp_ip,
                                                         bcm_mcast_ipaddr_t *src_ip)
{
    bcm_mcast_whitelist_node_t *node_p;

    __logDebug("ENTER");
    node_p = bcm_mcast_whitelist_node_lookup(outervlanid, grp_ip, src_ip);

    if ( node_p == NULL )
    {
        __logDebug("Allocate new whitelist node");
        node_p = bcm_mcast_whitelist_node_alloc();
        if ( node_p == NULL )
        {
            __logError("Whitelist node allocation failure");
            return NULL;
        }

        memset(node_p, 0, sizeof(*node_p));
        memcpy(&node_p->grp, grp_ip, sizeof(bcm_mcast_ipaddr_t));
        memcpy(&node_p->src, src_ip, sizeof(bcm_mcast_ipaddr_t));
        node_p->outer_vlanid = outervlanid;
        node_p->refcnt = 1;

        if ( node_p->grp.is_ipv4 )
        {
            node_p->is_ssm = (node_p->src.ipv4_addr.s_addr != 0);
            __logInfo("vid = %u grp %pI4 src %pI4",
                      outervlanid, &node_p->grp.ipv4_addr, &node_p->src.ipv4_addr);
        }
        else
        {
            if (!(BCM_IN6_IS_ADDR_ZERO(&node_p->src.ipv6_addr)))
            {
                node_p->is_ssm = 1;
            }
            __logInfo("vid = %u grp %pI6 src %pI6", outervlanid, &node_p->grp.ipv6_addr, &node_p->src.ipv6_addr);
        }

        if ( bcm_mcast_whitelist_add(node_p) )
        {
            __logError("Whitelist node add failure");
            bcm_mcast_whitelist_node_free(node_p);
            return NULL;
        }
        list_add(&node_p->list, whitelist_head_p);
    }
    else
    {
        node_p->refcnt++;
        if (node_p->grp.is_ipv4)
        {
            __logInfo("whitelist node already exists for vid = %u grp %pI4 src %pI4 refcnt %d",
                      outervlanid, &node_p->grp.ipv4_addr, &node_p->src.ipv4_addr, node_p->refcnt);
        }
        else
        {
            __logInfo("whitelist node already exists for vid = %u grp %pI6 src %pI6 refcnt %d", 
                      outervlanid, &node_p->grp.ipv6_addr, &node_p->src.ipv6_addr, node_p->refcnt);
        }
    }

    __logDebug("EXIT");
    return node_p;
}

int bcm_mcast_whitelist_node_del(bcm_mcast_whitelist_node_t *node_p)
{
    __logDebug("ENTER");

    if (!node_p)
    {
        return -1;
    }

    if (node_p->grp.is_ipv4)
    {
        __logInfo("Deleting whitelist node for vid = %u grp %pI4 src %pI4 refcnt %d",
                  node_p->outer_vlanid, &node_p->grp.ipv4_addr, &node_p->src.ipv4_addr, node_p->refcnt);
    }
    else
    {
        __logInfo("Deleting whitelist node for vid = %u grp %pI6 src %pI6 refcnt %d", 
                  node_p->outer_vlanid, &node_p->grp.ipv6_addr, &node_p->src.ipv6_addr, node_p->refcnt);
    }

    if (node_p->refcnt == 1)
    {
        if ( bcm_mcast_whitelist_delete(node_p->whitelist_key) )
        {
            /* Error handling*/
            __logError("Error while deleting whitelist entry");
            return  -1;
        }
        node_p->refcnt = 0;
        list_del(&node_p->list);
        bcm_mcast_whitelist_node_free(node_p);
    }
    else
    {
        node_p->refcnt--;
    }

    __logDebug("EXIT");
    return 0;
}

int bcm_mcast_whitelist_nodes_add(struct net_device *from_dev,
                                  uint32_t grpVid,
                                  bcm_mcast_ipaddr_t *grp,
                                  bcm_mcast_ipaddr_t *src,
                                  bcm_mcast_whitelist_info_t *info_p)
{
    int idx;
    int ret = 0;

    /*
     * If grpVid is not valid and this is bcmvlan device, merge the vlanctl rules to find out the vlan;
     * else take use of grpVid which is from management.
     */
    if ((BCM_MCAST_INVALID_VID == grpVid) && (is_netdev_vlan(from_dev)))
    {
        ret = bcm_mcast_blogrule_get_whitelist_vlan_info(from_dev, info_p);    
    }
    else
    {
        info_p->num_entries = 1;
        info_p->outervlan[0] = grpVid;        
    }

    if ( ret == 0 )
    {
        for (idx = 0; idx < info_p->num_entries; idx++)
        {
            if (grp->is_ipv4)
            {
                __logInfo("Adding whitelist node num_entries %d idx %d grp %pI4 src %pI4 vlan 0x%x",
                          info_p->num_entries,
                          idx,
                          &grp->ipv4_addr,
                          &src->ipv4_addr,
                          info_p->outervlan[idx]);
            }
            else
            {
               __logInfo("Adding whitelist node num_entries %d idx %d grp %pI6 src %pI6 vlan 0x%x",
                         info_p->num_entries,
                         idx,
                         &grp->ipv6_addr,
                         &src->ipv6_addr,
                         info_p->outervlan[idx]);
            }

            if ((info_p->node_p[idx] = bcm_mcast_whitelist_node_add(info_p->outervlan[idx], 
                                                                    grp, src)) == NULL)
            {
                __logError("Error adding whitelist node");
                return -1;
            }
        }
    }
    return 0;
}

int bcm_mcast_whitelist_nodes_del(bcm_mcast_whitelist_info_t *info_p)
{
    int idx = 0;

    __logInfo("%d whitelist nodes to delete\n", info_p->num_entries);
    for (idx = 0; idx < info_p->num_entries; idx++)
    {
        if (info_p->node_p[idx])
        {
            if ( bcm_mcast_whitelist_node_del(info_p->node_p[idx]) )
            {
                __logError("Whitelist delete failed");
                return -1;
            }
            info_p->node_p[idx] = NULL;
        }
        else
        {
            /* No more whitelist nodes */
            break;
        }
    }
    info_p->num_entries = 0;
    return 0;
}

int bcm_mcast_whitelist_init()
{
   int rc;
   BDMF_MATTR_ALLOC(mcast_whitelist_attrs, rdpa_mcast_whitelist_drv());

   whitelist_cache = kmem_cache_create("bcm_mcast_whitelist_node_cache", 
                                       sizeof(bcm_mcast_whitelist_node_t), 
                                       0, 
                                       SLAB_HWCACHE_ALIGN, NULL);

   if ( NULL == whitelist_cache )
   {
       __logError("failed to allocate whitelist_cache\n");
       rc = -ENOMEM;
       goto exit;
   }

   rc = rdpa_mcast_whitelist_get(&mcast_whitelist_class);
   if (rc)
   {
       rc = bdmf_new_and_set(rdpa_mcast_whitelist_drv(), NULL, mcast_whitelist_attrs, &mcast_whitelist_class);
       if (rc)
       {
           __logError("rdpa mcast_whitelist class object does not exist and can't be created.\n");
           goto exit;
       }
   }

   INIT_LIST_HEAD(whitelist_head_p);

#ifdef CONFIG_BCM_RUNNER_IPTV
   rc = rdpa_iptv_get(&iptv);
   if (rc)
   {
       __logError("rdpa rdpa_iptv_get return rc %d.\n", rc);
       goto exit;
   }
#endif

exit:
   BDMF_MATTR_FREE(mcast_whitelist_attrs);
   return rc;
}

void bcm_mcast_whitelist_exit()
{
    if (mcast_whitelist_class)
    {
        bdmf_destroy(mcast_whitelist_class);
    }

    if ( whitelist_cache )
    {
        kmem_cache_destroy(whitelist_cache);
    }

#ifdef CONFIG_BCM_RUNNER_IPTV
    bdmf_put(iptv);
#endif

}

#else

bcm_mcast_whitelist_add_hook_t bcm_mcast_whitelist_add_fn = NULL;
bcm_mcast_whitelist_delete_hook_t bcm_mcast_whitelist_delete_fn = NULL;

int bcm_mcast_whitelist_add(bcm_mcast_whitelist_node_t *node_p)
{
    return 0;
}

int bcm_mcast_whitelist_delete(whitelist_key_t whitelist_hdl)
{
    return 0;
}

int bcm_mcast_whitelist_init()
{
   return 0;
}

void bcm_mcast_whitelist_exit()
{
}
#endif
