/*
*    Copyright (c) 2003-2020 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2020:DUAL/GPL:standard

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
#include "bcm_br_hooks_maclimit.h"

#define NORMAL_DROP_PACKET(m) ((m)->max && (((m)->learning_count+(m)->reserve) >= (m)->max))
#define ZERO_DROP_PACKET(m)   ((!(m)->max) && (m)->max_zero_drop)
#define NEED_DROP_PACKET(m)   (NORMAL_DROP_PACKET(m) || ZERO_DROP_PACKET(m))
#define HAVE_RESERVE_ENTRY(m) ((m)->learning_count < (m)->min)

mac_limit_rcv_hook_t _rcv_ex = NULL;
mac_learning_notify_hook_t  _notify_ex = NULL;
mac_learning_update_hook_t _update_ex = NULL;
extern spinlock_t mac_limit_spinlock;

int bcm_mac_limit_en = 0;       /* mac limit default disabled */
EXPORT_SYMBOL(bcm_mac_limit_en);

void bcm_mac_limit_learning_notify(struct net_bridge *br, const struct net_bridge_fdb_entry *fdb, int type)
{
    struct net_device *dev;
    struct mac_limit *dev_mac_limit;
    struct mac_limit *br_mac_limit;
    int br_update = 0;
    
    if (!fdb || !fdb->dst || fdb->is_local)
        return;
    
    dev = fdb->dst->dev;
    dev_mac_limit = &dev->bcm_nd_ext.mac_limit;
    br_mac_limit = &br->dev->bcm_nd_ext.mac_limit;

    spin_lock(&mac_limit_spinlock);
    br_update = dev_mac_limit->enable;
    if (type == RTM_DELNEIGH)
    {
       dev_mac_limit->learning_count--;
       if (br_update)
       {
          if (HAVE_RESERVE_ENTRY(dev_mac_limit))
          {
              br_mac_limit->reserve++;
          }
          br_mac_limit->learning_count--;
       }
    }
    else if (type == RTM_NEWNEIGH)
    {
        if (br_update)
        {
            if (HAVE_RESERVE_ENTRY(dev_mac_limit))
            {
                br_mac_limit->reserve--;
            }
            br_mac_limit->learning_count++;
        }
        dev_mac_limit->learning_count++;
    }
    
    if (_notify_ex)
        _notify_ex(dev, type);
    
    spin_unlock(&mac_limit_spinlock);
}

void bcm_mac_limit_learning_update(struct net_bridge_fdb_entry *fdb,struct net_bridge_port *source)
{
    struct net_device *dev, *br_dev;
    struct mac_limit *dev_mac_limit;
    struct mac_limit *br_mac_limit;
    int br_update = 0;
    
    if (!fdb || !fdb->dst || !source)
        return;
    
    dev = fdb->dst->dev;
    br_dev = source->br->dev;
    dev_mac_limit = &dev->bcm_nd_ext.mac_limit;
    br_mac_limit = &br_dev->bcm_nd_ext.mac_limit;

    spin_lock(&mac_limit_spinlock);
    br_update = dev_mac_limit->enable;
    /* mac move case, delete obselete then notify new */
    if (unlikely(source != fdb->dst))
    {
       dev_mac_limit->learning_count--;
       if (br_update)
       {
          if (HAVE_RESERVE_ENTRY(dev_mac_limit))
          {
              br_mac_limit->reserve++;
          }
          br_mac_limit->learning_count--;
       }
           
       if (_update_ex)
           _update_ex(dev, 0);
    }
    spin_unlock(&mac_limit_spinlock);
}

static unsigned int _bcm_mac_limit_rcv(struct sk_buff *skb)
{
    struct net_device *dev = skb->dev;
    struct net_bridge_port *p = br_port_get_rcu(skb->dev);
    struct mac_limit *dev_mac_limit = &dev->bcm_nd_ext.mac_limit;
    struct net_device *br_dev = p->br->dev;
    struct mac_limit *br_mac_limit;
    struct net_bridge_fdb_entry *fdb;
    u16 vid = 0;
    bool entry_exist = false;
        
    if (!br_port_exists(dev) || !dev_mac_limit->enable)
        return NF_ACCEPT;
    
    br_vlan_get_tag(skb, &vid);
    rcu_read_lock();
    fdb = br_fdb_find_rcu(p->br, eth_hdr(skb)->h_source, vid);
    if (fdb && fdb->dst)
        entry_exist = (dev == fdb->dst->dev);
    rcu_read_unlock();
    
    if (unlikely(!entry_exist))
    {
        br_mac_limit = &br_dev->bcm_nd_ext.mac_limit;
        if (NEED_DROP_PACKET(dev_mac_limit))
        {
            dev_mac_limit->drop_count++;
            return NF_DROP;
        }

        if (NEED_DROP_PACKET(br_mac_limit) && !HAVE_RESERVE_ENTRY(dev_mac_limit))
        {
            br_mac_limit->drop_count++;
            return NF_DROP;
        }
        
        if (_rcv_ex)
            return _rcv_ex(dev);
        else
            return NF_ACCEPT;
    }
    else
        return NF_ACCEPT;
}

unsigned int bcm_br_fdb_mac_limit(struct sk_buff *skb)
{
    if (!bcm_mac_limit_en)
        return 0;
    else
        return (_bcm_mac_limit_rcv(skb) == NF_DROP);
}