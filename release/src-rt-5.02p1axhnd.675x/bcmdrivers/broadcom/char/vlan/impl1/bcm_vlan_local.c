/*
<:copyright-BRCM:2011:proprietary:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/
//**************************************************************************
// File Name  : bcm_vlan_local.c
//
// Description: Broadcom VLAN Interface Driver
//
//**************************************************************************

#include "bcm_vlan_local.h"
#include "bcm_vlan_flows.h"
#include <linux/version.h>
#include "skb_defines.h"
#include <board.h>


/*
 * Global variables
 */

/* global BCM VLAN lock */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
spinlock_t bcmVlan_dp_lock_g;
spinlock_t bcmVlan_rx_lock_g;
spinlock_t bcmVlan_tx_lock_g;
#else
spinlock_t bcmVlan_dp_lock_g = SPIN_LOCK_UNLOCKED;
spinlock_t bcmVlan_rx_lock_g = SPIN_LOCK_UNLOCKED;
spinlock_t bcmVlan_tx_lock_g = SPIN_LOCK_UNLOCKED;
#endif
#endif


/*
 * Local variables
 */

static BCM_VLAN_DECLARE_LL(realDevCtrlLL);

static struct kmem_cache *vlanDevCtrlCache;
static struct kmem_cache *realDevCtrlCache;


/*
 * Local Functions
 */

static inline struct vlanDeviceControl *allocVlanDevCtrl(void)
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    return kmem_cache_alloc(vlanDevCtrlCache, GFP_ATOMIC);
#else
    return kmem_cache_alloc(vlanDevCtrlCache, GFP_KERNEL);
#endif
}

static inline void freeVlanDevCtrl(struct vlanDeviceControl *vlanDevCtrl)
{
    kmem_cache_free(vlanDevCtrlCache, vlanDevCtrl);
}

static inline struct realDeviceControl *allocRealDevCtrl(void)
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    return kmem_cache_alloc(realDevCtrlCache, GFP_ATOMIC);
#else
    return kmem_cache_alloc(realDevCtrlCache, GFP_KERNEL);
#endif
}

static inline void freeRealDevCtrl(struct realDeviceControl *realDevCtrl)
{
    kmem_cache_free(realDevCtrlCache, realDevCtrl);
}

/*
 * Free all VLAN Devices of a given Real Device
 *
 * Entry points: NOTIFIER, CLEANUP
 */
static int freeVlanDevices(struct realDeviceControl *realDevCtrl)
{
    int count = 0;
    struct vlanDeviceControl *vlanDevCtrl, *nextVlanDevCtrl;

    BCM_ASSERT(realDevCtrl);

    /* Make sure the caller has the RTNL lock */
    ASSERT_RTNL();

    vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);

    while(vlanDevCtrl)
    {
        BCM_LOG_INFO(BCM_LOG_ID_VLAN, "De-allocated VLAN Device %s", vlanDevCtrl->vlanDev->name);

        count++;

        nextVlanDevCtrl = vlanDevCtrl;

        nextVlanDevCtrl = BCM_VLAN_LL_GET_NEXT(nextVlanDevCtrl);

        bcmVlan_freeVlanDevice(vlanDevCtrl->vlanDev, 0);

        vlanDevCtrl = nextVlanDevCtrl;
    };

    return count;
}

/*
 * Free the VLAN Devices of ALL Real Devices
 *
 * Entry points: CLEANUP
 */
static void freeAllVlanDevices(void)
{
    struct realDeviceControl *realDevCtrl, *nextRealDevCtrl;

    rtnl_lock();

    realDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrlLL);

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Cleaning-up Real Devices (%p)", (void *)realDevCtrl);

    while(realDevCtrl)
    {
        BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Cleaning-up Real Device %s", realDevCtrl->realDev->name);

        nextRealDevCtrl = realDevCtrl;

        nextRealDevCtrl = BCM_VLAN_LL_GET_NEXT(nextRealDevCtrl);

        freeVlanDevices(realDevCtrl);
        realDevCtrl = nextRealDevCtrl;
    };

    rtnl_unlock();
}


/*
 * Global Functions
 */

int bcmVlan_initVlanDevices(void)
{
    /* create a slab cache for device descriptors */
    vlanDevCtrlCache = kmem_cache_create("bcmvlan_vlanDev",
                                         sizeof(struct vlanDeviceControl),
                                         0, /* align */
                                         0, /* flags */
                                         NULL); /* ctor */
    if(vlanDevCtrlCache == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to create VLAN Devices Cache");

        return -ENOMEM;
    }

    /* create a slab cache for device descriptors */
    realDevCtrlCache = kmem_cache_create("bcmvlan_realDev",
                                         sizeof(struct realDeviceControl),
                                         0, /* align */
                                         0, /* flags */
                                         NULL); /* ctor */
    if(realDevCtrlCache == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to create Real Devices Cache");

        kmem_cache_destroy(vlanDevCtrlCache);

        return -ENOMEM;
    }

    BCM_VLAN_LL_INIT(&realDevCtrlLL);

    return 0;
}

void bcmVlan_cleanupVlanDevices(void)
{
    freeAllVlanDevices();
    kmem_cache_destroy(vlanDevCtrlCache);
    kmem_cache_destroy(realDevCtrlCache);
}

/*
 * IMPORTANT : This function MUST only be called within critical regions
 */
struct realDeviceControl *bcmVlan_getRealDevCtrl(struct net_device *realDev)
{
    struct realDeviceControl *realDevCtrl;

    BCM_ASSERT(realDev);

    realDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrlLL);

    /* find the device control structure associated to the real device */
    while(realDevCtrl)
    {
        if(realDevCtrl->realDev == realDev)
        {
            break;
        }

        realDevCtrl = BCM_VLAN_LL_GET_NEXT(realDevCtrl);
    };

/*     if(realDevCtrl == NULL) */
/*     { */
/*         BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "%s has no VLAN Interfaces", realDev->name); */
/*     } */

    return realDevCtrl;
}

/*
 * Free all VLAN Devices of a specific Real Device
 *
 * Entry points: NOTIFIER
 */
void bcmVlan_freeRealDeviceVlans(struct net_device *realDev)
{
    struct realDeviceControl *realDevCtrl;

    BCM_ASSERT(realDev);

    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);

    if(realDevCtrl != NULL)
    {
        BCM_LOG_INFO(BCM_LOG_ID_VLAN, "De-allocating VLAN Devices of %s", realDev->name);

        freeVlanDevices(realDevCtrl);
    }
}

/*
 * Free a specific VLAN Device
 *
 * Entry points: IOCTL
 */
void bcmVlan_freeVlanDevice(struct net_device *vlanDev, int unregisterFlag)
{
    struct vlanDeviceControl *vlanDevCtrl;
    struct realDeviceControl *realDevCtrl;
    struct net_device *realDev;
    int ret = 0;

    BCM_ASSERT(vlanDev);

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    vlanDevCtrl = BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl;
    realDevCtrl = vlanDevCtrl->realDevCtrl;
    realDev = realDevCtrl->realDev;

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "De-allocating %s", vlanDevCtrl->vlanDev->name);

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "head %p, tail %p",
                  (void *)(&realDevCtrl->vlanDevCtrlLL)->head,
                  (void *)realDevCtrl->vlanDevCtrlLL.tail);

#if !defined(CONFIG_BCM_VLAN_ROUTED_WAN_USES_ROOT_DEV_MAC)
    if(BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl->flags.routed)
    {
        kerSysReleaseMacAddress(vlanDev->dev_addr);
        vlanctl_notify_route_mac(vlanDev->dev_addr, 0);
    }
#endif

#if defined(CONFIG_BCM_VLAN_ISOLATION)
    if (!BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl->flags.swOnly)
    {
        vlanctl_notify_vlan_set_iso(vlanDev, 0);
    }
#endif

    if(unregisterFlag)
    {
        /* Delete all VLAN flows associated with this device */
        bcmVlan_flowDev_delete(vlanDev);
    }

    BCM_VLAN_LL_REMOVE(&realDevCtrl->vlanDevCtrlLL, vlanDevCtrl);

    bcmVlan_removeTagRulesByVlanDev(vlanDevCtrl);

    /* FIXME: Do we need to remove both pointers from the Info structure here ? */
    BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl = NULL;
//    BCM_VLAN_DEV_INFO(vlanDev)->realDev = NULL;

    freeVlanDevCtrl(vlanDevCtrl);

    if(BCM_VLAN_LL_IS_EMPTY(&realDevCtrl->vlanDevCtrlLL))
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Real Device has no more VLANs: De-allocate");

        /* there are no more VLAN interfaces associated to the real device */
        BCM_VLAN_LL_REMOVE(&realDevCtrlLL, realDevCtrl);

        bcmVlan_cleanupRuleTables(realDevCtrl);

        freeRealDevCtrl(realDevCtrl);
    }

    if(unregisterFlag)
    {
        /* remove the reference to the vlan device added by dev_get_by_name() */
        dev_put(vlanDev);
        ret = netdev_path_remove(vlanDev);
        if(ret)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to remove %s from Interface path (%d)",
                          vlanDev->name, ret);

            netdev_path_dump(vlanDev);
        }
    }

    BCM_LOG_INFO(BCM_LOG_ID_VLAN, "De-allocated VLAN Device %s", vlanDev->name);

    BCM_VLAN_GLOBAL_UNLOCK();

    unregister_netdevice(vlanDev);

    /* decrement reference counter of the real device, since we
       added a reference in register_vlan_device() when the VLAN
       device was created */
    dev_put(realDev);


    /******** CRITICAL REGION END ********/
}

/*
 * IMPORTANT : This function MUST only be called within critical regions
 *
 * Entry points: IOCTL indirect
 */
struct net_device *bcmVlan_getRealDeviceByName(char *realDevName, struct realDeviceControl **pRealDevCtrl)
{
    struct realDeviceControl *realDevCtrl;
    struct net_device *realDev = NULL;

    /* NULL terminate name, just in case */
    realDevName[IFNAMSIZ-1] = '\0';

    realDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrlLL);

    while(realDevCtrl)
    {
        if(!strcmp(realDevCtrl->realDev->name, realDevName))
        {
            realDev = realDevCtrl->realDev;
            break;
        }

        realDevCtrl = BCM_VLAN_LL_GET_NEXT(realDevCtrl);
    };

    if(pRealDevCtrl != NULL)
    {
        *pRealDevCtrl = realDevCtrl;
    }

    return realDev;
}

/*
 * Entry points: IOCTL indirect
 */
struct net_device *bcmVlan_getVlanDeviceByName(struct realDeviceControl *realDevCtrl, char *vlanDevName)
{
    struct vlanDeviceControl *vlanDevCtrl;
    struct net_device *vlanDev = NULL;

    BCM_ASSERT(realDevCtrl);

    /* NULL terminate name, just in case */
    vlanDevName[IFNAMSIZ-1] = '\0';

    vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);

    while(vlanDevCtrl)
    {
        if(!strcmp(vlanDevCtrl->vlanDev->name, vlanDevName))
        {
            vlanDev = vlanDevCtrl->vlanDev;
            break;
        }

        vlanDevCtrl = BCM_VLAN_LL_GET_NEXT(vlanDevCtrl);
    };

    return vlanDev;
}

/*
 * IMPORTANT : This function MUST only be called within critical regions
 */
bool bcmVlan_isIptvOnlyVlanDevice(struct net_device *vlanDev)
{
    struct vlanDeviceControl *vlanDevCtrl;

    BCM_ASSERT(vlanDev);
    vlanDevCtrl = BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl;
    
    return vlanDevCtrl->iptvOnly != 0;
}

/*
 * IMPORTANT : This function MUST only be called within critical regions
 *
 * Entry points: IOCTL indirect
 */
int bcmVlan_setIptvOnlyVlanDevice(struct net_device *vlanDev)
{
    int ret = 0;
    struct vlanDeviceControl *vlanDevCtrl;

    BCM_ASSERT(vlanDev);
    vlanDevCtrl = BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl;

    if(vlanDevCtrl->iptvOnly >= BCM_VLAN_IPTV_REF_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLAN Device %s IPTV only refer beyond limit", vlanDev->name);
        ret = -EINVAL;
        goto out;
    }
    else
    {
        vlanDevCtrl->iptvOnly++;
    }
out:
    return ret;
}

/*
 * IMPORTANT : This function MUST only be called within critical regions
 *
 * Entry points: IOCTL indirect
 */
int bcmVlan_unsetIptvOnlyVlanDevice(struct net_device *vlanDev, bool clearAll)
{
    int ret = 0;
    struct vlanDeviceControl *vlanDevCtrl;

    BCM_ASSERT(vlanDev);

    vlanDevCtrl = BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl;

    if(clearAll)
    {
        vlanDevCtrl->iptvOnly = 0;
    }
    else if(vlanDevCtrl->iptvOnly == 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLAN Device %s IPTV only refer underflow", vlanDev->name);
        ret = -EINVAL;
        goto out;
    }
    else
    {
        vlanDevCtrl->iptvOnly--;
    }
out:
    return ret;
}

/*
 * This function must be called after the new vlan device has been registered
 in Linux because we do not check for pre-existing devices here. We rely on
 Linux to do it
 *
 * Entry points: IOCTL indirect
 */
int bcmVlan_createVlanDevice(struct net_device *realDev, struct net_device *vlanDev,
                             bcmVlan_vlanDevFlags_t flags)
{
    int ret = 0;
    struct vlanDeviceControl *vlanDevCtrl;
    struct realDeviceControl *realDevCtrl;

    BCM_ASSERT(realDev);
    BCM_ASSERT(vlanDev);

    /* Make sure the caller has the RTNL lock */
    ASSERT_RTNL();

    vlanDevCtrl = allocVlanDevCtrl();
    if(vlanDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate Device Control memory for %s",
                      vlanDev->name);

        ret = -ENOMEM;
        goto out;
    }

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
    if(realDevCtrl == NULL)
    {
        /* allocate device control */
        realDevCtrl = allocRealDevCtrl();
        if(realDevCtrl == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate Device Control memory for %s",
                          realDev->name);

            freeVlanDevCtrl(vlanDevCtrl);

            ret = -ENOMEM;
            goto out;
        }

        /* initialize real device */
        realDevCtrl->realDev = realDev;

        BCM_VLAN_LL_INIT(&realDevCtrl->vlanDevCtrlLL);

        bcmVlan_initTpidTable(realDevCtrl);

        bcmVlan_initRuleTables(realDevCtrl);

        /* initialize local stats */
        memset(&realDevCtrl->localStats, 0, sizeof(bcmVlan_localStats_t));

        realDevCtrl->mode = BCM_VLAN_MODE_ONT;

        /* insert real device into the main linked list */
        BCM_VLAN_LL_APPEND(&realDevCtrlLL, realDevCtrl);
    }

    /* initialize VLAN Device */
    vlanDevCtrl->vlanDev = vlanDev;
    vlanDevCtrl->realDevCtrl = realDevCtrl;
    vlanDevCtrl->flags = flags;
	vlanDevCtrl->iptvOnly = 0;

    /* insert vlan device into the real device's linked list */
    BCM_VLAN_LL_APPEND(&realDevCtrl->vlanDevCtrlLL, vlanDevCtrl);

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "head %p, tail %p",
                  (void *)realDevCtrl->vlanDevCtrlLL.head,
                  (void *)realDevCtrl->vlanDevCtrlLL.tail);

    /* update pointer to the new vlanDevCtrl in the VLAN device structure */
    BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl = vlanDevCtrl;
    BCM_VLAN_DEV_INFO(vlanDev)->realDev = realDev;

    BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Created VLAN Device %s", vlanDev->name);

out:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL, NOTIFIER
 */
void bcmVlan_transferOperstate(struct net_device *realDev)
{
    struct realDeviceControl *realDevCtrl;
    struct vlanDeviceControl *vlanDevCtrl;

    BCM_ASSERT(realDev);

    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
    if(realDevCtrl == NULL)
    {
        return;
    }

    vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);

    while(vlanDevCtrl)
    {
        BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Updating Operation State of %s",
                     vlanDevCtrl->vlanDev->name);

        /* Have to respect userspace enforced dormant state
         * of real device, also must allow supplicant running
         * on VLAN device
         */
        if(realDev->operstate == IF_OPER_DORMANT)
        {
            netif_dormant_on(vlanDevCtrl->vlanDev);
        }
        else
        {
            netif_dormant_off(vlanDevCtrl->vlanDev);
        }

        if(netif_carrier_ok(realDev))
        {
            if(!netif_carrier_ok(vlanDevCtrl->vlanDev))
            {
                netif_carrier_on(vlanDevCtrl->vlanDev);
            }
        }
        else
        {
            if(netif_carrier_ok(vlanDevCtrl->vlanDev))
            {
                netif_carrier_off(vlanDevCtrl->vlanDev);
            }
        }

        vlanDevCtrl = BCM_VLAN_LL_GET_NEXT(vlanDevCtrl);
    };
}

/*
 * Entry points: NOTIFIER
 */
void bcmVlan_updateInterfaceState(struct net_device *realDev, int state)
{
    int flags;
    struct realDeviceControl *realDevCtrl;
    struct vlanDeviceControl *vlanDevCtrl;

    BCM_ASSERT(realDev);

    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
    if(realDevCtrl == NULL)
    {
        return;
    }

    vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);

    while(vlanDevCtrl)
    {
        flags = vlanDevCtrl->vlanDev->flags;

        if(state == NETDEV_UP)
        {
            if(!(flags & IFF_UP))
            {
                BCM_LOG_INFO(BCM_LOG_ID_VLAN, "%s is UP", vlanDevCtrl->vlanDev->name);
                dev_change_flags(vlanDevCtrl->vlanDev, flags | IFF_UP);
            }
        }
        else
        {
            if(flags & IFF_UP)
            {
                BCM_LOG_INFO(BCM_LOG_ID_VLAN, "%s is DOWN", vlanDevCtrl->vlanDev->name);
                dev_change_flags(vlanDevCtrl->vlanDev, flags & ~IFF_UP);
            }
        }

        vlanDevCtrl = BCM_VLAN_LL_GET_NEXT(vlanDevCtrl);
    };
}


/*
 * Entry points: IOCTL
 */
int bcmVlan_dumpAllTagRules(void)
{
    int ret = 0;
    struct realDeviceControl *realDevCtrl = NULL;
    UINT8 nbrOfTags;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrlLL);
    while(realDevCtrl)
    {
        for (nbrOfTags = 0; nbrOfTags < BCM_VLAN_MAX_RULE_TABLES;  nbrOfTags++)
        {            
            bcmVlan_dumpTagRulesByTable(realDevCtrl, nbrOfTags, BCM_VLAN_TABLE_DIR_RX);
            bcmVlan_dumpTagRulesByTable(realDevCtrl, nbrOfTags, BCM_VLAN_TABLE_DIR_TX);
        }
        realDevCtrl = BCM_VLAN_LL_GET_NEXT(realDevCtrl);
    }

    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

#ifdef BCM_VLAN_DATAPATH_DEBUG
void __bcmVlan_dumpPacket(unsigned int *tpidTable, struct sk_buff *skb)
{
    int i;
    bcmVlan_ethHeader_t *ethHeader = BCM_VLAN_SKB_ETH_HEADER(skb);
    bcmVlan_vlanHeader_t *vlanHeader = &ethHeader->vlanHeader;
    bcmVlan_vlanHeader_t *prevVlanHeader = vlanHeader - 1;
    bcmVlan_ipHeader_t *ipHeader;

    if(bcmLog_getLogLevel(BCM_LOG_ID_VLAN) != BCM_LOG_LEVEL_DEBUG)
    {
        return;
    }

    printk("skb         : 0x%p, ", (void *)(skb));
    printk("data 0x%p, ", (void *)skb->data);
    printk("skb_len %d, ", (unsigned int)skb->len);
    printk("mac_len %d, ", (unsigned int)skb->mac_len);
    printk("mac_header 0x%08X, ", (unsigned int)skb->mac_header);
    printk("network_header 0x%08X\n", (unsigned int)skb->network_header);
    printk("Priority    : %d\n", skb->priority);
    printk("Mark.FlowId : %d\n", SKBMARK_GET_FLOW_ID(skb->mark));
    printk("Mark.Port   : %d\n", SKBMARK_GET_PORT(skb->mark));
    printk("Mark.Queue  : %d\n", SKBMARK_GET_Q(skb->mark));
    if(skb->vlan_count)
    {
        printk("skb vlan    : count %d, ", skb->vlan_count);
        printk("tpid 0x%x, ", skb->vlan_tpid);
        printk("headers 0x%x 0x%x\n", skb->vlan_header[0], skb->vlan_header[1]);
    }
    else
    {
        printk("skb vlan    : NONE\n");
    }
    printk("\n");

    printk("-> L2 Header (0x%p)\n", (void *)(ethHeader));

    printk("DA    : ");
    for(i=0; i<ETH_ALEN; ++i)
    {
        printk("%02X ", ethHeader->macDest[i]);
    }
    printk("\n");

    printk("SA    : ");
    for(i=0; i<ETH_ALEN; ++i)
    {
        printk("%02X ", ethHeader->macSrc[i]);
    }
    printk("\n");

    printk("ETHER : %04X\n", ntohs(ethHeader->etherType));

    if(BCM_VLAN_TPID_MATCH(tpidTable, (ntohs(ethHeader->etherType))))
    {
        printk("\n");

        do {
            printk("-> VLAN Header (0x%p)\n", (void *)(vlanHeader));
            printk("PBITS : %d\n", BCM_VLAN_GET_TCI_PBITS(vlanHeader));
            printk("CFI   : %d\n", BCM_VLAN_GET_TCI_CFI(vlanHeader));
            printk("VID   : %d\n", BCM_VLAN_GET_TCI_VID(vlanHeader));
            printk("ETHER : %04X\n", ntohs(vlanHeader->etherType));
            printk("\n");
            prevVlanHeader = vlanHeader;
        } while((vlanHeader = BCM_VLAN_GET_NEXT_VLAN_HEADER(tpidTable, vlanHeader)) != NULL);
    }

    if(ntohs(prevVlanHeader->etherType) == 0x0800)
    {
        ipHeader = (bcmVlan_ipHeader_t *)(prevVlanHeader + 1);
    }
    else if(ntohs(prevVlanHeader->etherType) == 0x8864)
    {
        bcmVlan_pppoeSessionHeader_t *pppoeSessionHeader = (bcmVlan_pppoeSessionHeader_t *)(prevVlanHeader + 1);

        printk("-> PPPoE Header (0x%p)\n", (void *)pppoeSessionHeader);

        printk("version_type : 0x%02X\n", pppoeSessionHeader->version_type);
        printk("code         : 0x%02X\n", pppoeSessionHeader->code);
        printk("sessionId    : 0x%04X\n", ntohs(pppoeSessionHeader->sessionId));
        printk("pppHeader    : 0x%04X\n", ntohs(pppoeSessionHeader->pppHeader));

        ipHeader = &pppoeSessionHeader->ipHeader;
    }
    else
    {
        ipHeader = NULL;

        printk("Unknown L3 Header: 0x%04X\n", ntohs(prevVlanHeader->etherType));
    }

    if(ipHeader)
    {
        printk("\n");

        printk("-> IP Header (0x%p)\n", (void *)(ipHeader));

        if(BCM_VLAN_GET_IP_VERSION(ipHeader) == 4)
        {
#if 0
            UINT32 *p = (UINT32*)ipHeader;
            for(i=0; i<5; ++i)
            {
                printk("0x%08lX\n", p[i]);
            }
#endif
            printk("Type   : IPv4\n");
            printk("DSCP   : %d\n", BCM_VLAN_GET_IP_DSCP(ipHeader));
            printk("Proto  : %d\n", ipHeader->protocol);
            printk("IP Src : %d.%d.%d.%d\n",
                   (ntohl(ipHeader->ipSrc) & 0xFF000000) >> 24,
                   (ntohl(ipHeader->ipSrc) & 0x00FF0000) >> 16,
                   (ntohl(ipHeader->ipSrc) & 0x0000FF00) >> 8,
                   (ntohl(ipHeader->ipSrc) & 0x000000FF));
            printk("IP Dst : %d.%d.%d.%d\n",
                   (ntohl(ipHeader->ipDest) & 0xFF000000) >> 24,
                   (ntohl(ipHeader->ipDest) & 0x00FF0000) >> 16,
                   (ntohl(ipHeader->ipDest) & 0x0000FF00) >> 8,
                   (ntohl(ipHeader->ipDest) & 0x000000FF));
        }
        else
        {
            printk("Not IPv4\n");
        }
    }

    printk("\n");

//    dumpHexData(ethHeader, skb->len);
}
#endif
