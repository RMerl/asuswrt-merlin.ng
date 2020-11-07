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
// File Name  : bcm_vlan_user.c
//
// Description: Broadcom VLAN Interface Driver
//               
//**************************************************************************

#include "bcm_vlan_local.h"
#include "bcm_vlan_dev.h"
#include "bcm_vlan_flows.h"
#include <board.h>
#include <linux/version.h>
#include <linux/ethtool.h>
#include <linux/vlanctl_bind.h>
#include <net/net_namespace.h>


#define BCM_VLAN_DEVICE_NAME  "bcmvlan"

/* defined in CommEngine/targets/makeDevs */
#define BCM_VLAN_DEVICE_MAJOR 323


/*
 * vlan network devices have devices nesting below it, and are a special
 * "super class" of normal network devices; split their locks off into a
 * separate class since they always nest.
 */
static int vlanMajor = BCM_VLAN_DEVICE_MAJOR;

static char bcmVlan_ifNameSuffix[BCM_VLAN_IF_SUFFIX_SIZE];


/*
 * Local functions
 */

#if defined(CONFIG_BCM_VLAN_ISOLATION)
int vlanctl_notify_vlan_set_iso(struct net_device *vlan_dev, int enable)
{
    vlanctl_vlan_t vlanctl_vlan;

    vlanctl_vlan.vlan_dev = vlan_dev;
    vlanctl_vlan.vid = BCM_VLAN_DONT_CARE;
    vlanctl_vlan.enable = enable;

#if defined(CONFIG_BLOG)
    vlanctl_notify(VLANCTL_BIND_NOTIFY_VLAN, &vlanctl_vlan, VLANCTL_BIND_CLIENT_RUNNER);
#endif

    return 0;
}
#endif
int vlanctl_notify_route_mac(unsigned char * mac, int enable)
{
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
    vlanctl_route_mac_t vlanctl_route_mac = {};
    
    memcpy(vlanctl_route_mac.mac, mac, BCM_ETH_ADDR_LEN);
    vlanctl_route_mac.enable = enable;
#if defined(CONFIG_BLOG)
    vlanctl_notify(VLANCTL_BIND_NOTIFY_ROUTE_MAC, &vlanctl_route_mac, VLANCTL_BIND_CLIENT_RUNNER);
#endif

#endif
#endif
    return 0;
}

static int unregisterVlanDevice(const char *vlanDevName)
{
    struct net_device *vlanDev;
    struct net_device *realDev;
    int ret = 0;

    rtnl_lock();

    vlanDev = dev_get_by_name(&init_net, vlanDevName);
    if (vlanDev != NULL)
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Found %s (%p)", vlanDev->name, (void *)vlanDev);

        if(!netdev_path_is_leaf(vlanDev))
        {
            /* device referenced by one or more interfaces, fail */
			dev_put(vlanDev);
            ret = -EBUSY;
			goto out;
        }

        if(vlanDev->priv_flags & IFF_BCM_VLAN)
        {
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "%p -> %p -> %p -> %p -> %p",
                          (void *)vlanDev,
                          (void *)(netdev_priv(vlanDev)),
                          (void *)(((bcmVlan_devInfo_t *)(netdev_priv(vlanDev)))->vlanDevCtrl),
                          (void *)(((bcmVlan_devInfo_t *)(netdev_priv(vlanDev)))->vlanDevCtrl->realDevCtrl),
                          (void *)(((bcmVlan_devInfo_t *)(netdev_priv(vlanDev)))->vlanDevCtrl->realDevCtrl->realDev));

            realDev = BCM_VLAN_REAL_DEV(vlanDev);
            bcmVlan_cleanupRxDefaultActions(realDev, vlanDev);

            bcmVlan_freeVlanDevice(vlanDev, 1);
        }
        else
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN,
                          "Tried to remove the non-VLAN device %s with priv_flags=0x%04X",
                          vlanDev->name, vlanDev->priv_flags);

            /* remove the reference to the vlan device added by dev_get_by_name() */
            dev_put(vlanDev);

            ret = -EPERM;
        }
    }
    else
    {
        BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Could not find VLAN Device %s", vlanDevName);

        ret = -EINVAL;
    }

out:
    rtnl_unlock();

    return ret;
}

#define BCM_VLAN_DRV_VERSION "1.0"

const char bcmVlan_fullname[] = "Broadcom VLAN Interface";
const char bcmVlan_version[] = BCM_VLAN_DRV_VERSION;

static int bcmVlan_ethtoolGetSettings(struct net_device *vlanDev,
                                      struct ethtool_cmd *cmd)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);

    if (!realDev->ethtool_ops ||
        !realDev->ethtool_ops->get_settings)
        return -EOPNOTSUPP;

    return realDev->ethtool_ops->get_settings(realDev, cmd);
}

static void bcmVlan_ethtoolGetDrvinfo(struct net_device *vlanDev,
                                      struct ethtool_drvinfo *info)
{
    strcpy(info->driver, bcmVlan_fullname);
    strcpy(info->version, bcmVlan_version);
    strcpy(info->fw_version, "N/A");
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0))
static u32 bcmVlan_ethtoolGetRxCsum(struct net_device *vlanDev)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);
    if (realDev->ethtool_ops == NULL ||
        realDev->ethtool_ops->get_rx_csum == NULL)
        return 0;

    return realDev->ethtool_ops->get_rx_csum(realDev);
}

static u32 bcmVlan_ethtoolGetFlags(struct net_device *vlanDev)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);

    if (!(realDev->features & NETIF_F_HW_VLAN_RX) ||
        realDev->ethtool_ops == NULL ||
        realDev->ethtool_ops->get_flags == NULL)
        return 0;

    return realDev->ethtool_ops->get_flags(realDev);
}
#endif

static const struct ethtool_ops bcmVlan_ethtool_ops = {
    .get_settings = bcmVlan_ethtoolGetSettings,
    .get_drvinfo  = bcmVlan_ethtoolGetDrvinfo,
    .get_link     = ethtool_op_get_link,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0))
    .get_rx_csum  = bcmVlan_ethtoolGetRxCsum,
    .get_flags    = bcmVlan_ethtoolGetFlags,
#endif
};

const struct header_ops bcmVlan_header_ops = {
    .create  = bcmVlan_devHardHeader,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    .rebuild = bcmVlan_devRebuildHeader,
#endif
    .parse   = eth_header_parse,
};

const struct net_device_ops bcmVlan_netdev_ops = {
    .ndo_change_mtu         = bcmVlan_devChangeMtu,
    .ndo_init		    = bcmVlan_devInit,
    .ndo_uninit		    = bcmVlan_devUninit,
    .ndo_open		    = bcmVlan_devOpen,
    .ndo_stop		    = bcmVlan_devStop,
    .ndo_start_xmit         = bcmVlan_devHardStartXmit,
    .ndo_validate_addr	    = eth_validate_addr,
    .ndo_set_mac_address    = bcmVlan_devSetMacAddress,
    .ndo_set_rx_mode        = bcmVlan_devSetRxMode,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
#else
    .ndo_set_multicast_list = bcmVlan_devSetRxMode,
#endif
    .ndo_change_rx_flags    = bcmVlan_devChangeRxFlags,
    .ndo_do_ioctl	        = bcmVlan_devIoctl,
    .ndo_neigh_setup	    = bcmVlan_devNeighSetup,
    .ndo_get_stats64        = bcmVlan_devCollectStats,
};

static void vlanSetup(struct net_device *vlanDev)
{
    ether_setup(vlanDev);

    vlanDev->priv_flags |= IFF_BCM_VLAN;
    vlanDev->tx_queue_len = 0;

    vlanDev->netdev_ops  = &bcmVlan_netdev_ops;
    vlanDev->destructor	 = free_netdev;
    vlanDev->ethtool_ops = &bcmVlan_ethtool_ops;

#if defined(CONFIG_BLOG)
    vlanDev->blog_stats_flags |= BLOG_DEV_STAT_FLAG_INCLUDE_ALL;
    vlanDev->clr_stats = bcmVlan_devClearStats;
#endif

    memset(vlanDev->broadcast, 0, ETH_ALEN);
}

#if !defined(CONFIG_BCM_VLAN_ROUTED_WAN_USES_ROOT_DEV_MAC)
static int allocMacAddress(struct net_device *vlanDev)
{
    int ret = 0;
    struct sockaddr addr;
    unsigned long unit = 0, connId = 0, macId = 0;
    char *p;
    int i;
          
    addr.sa_data[0] = 0xFF;

    /* Format the mac id */
    i = strcspn(vlanDev->name, "0123456789");
    if(i > 0)
    {
        unit = simple_strtoul(&(vlanDev->name[i]), NULL, 10);
    }

    p = strstr(vlanDev->name, bcmVlan_ifNameSuffix);
    if(p != NULL)
    {
        connId = simple_strtoul(p + strlen(bcmVlan_ifNameSuffix), NULL, 10);
    }

    macId = kerSysGetMacAddressType(vlanDev->name);

    /* set unit number to bit 20-27, connId to bit 12-19. */
    macId |= ((unit & 0xFF) << 20) | ((connId & 0xFF) << 12);

    kerSysGetMacAddress(addr.sa_data, macId);

    if((addr.sa_data[0] & 0x01) == 0x01)
    {
        printk("Unable to get MAC address from persistent storage\n");

        ret = -EADDRNOTAVAIL;
        goto out;
    }

    ret = bcmVlan_devSetMacAddress(vlanDev, &addr);
    if(ret)
    {
        kerSysReleaseMacAddress(addr.sa_data);
    }

out:
    return ret;
}
#endif

static struct net_device *registerVlanDeviceByName(const char *realDevName, char *name,
                                                   int isRouted, int isMulticast,
                                                   int isSwOnly)
{
    struct net_device *newDev = NULL;
    struct net_device *realDev; /* the ethernet device */
    struct realDeviceControl *realDevCtrl;
    struct net *net;
    bcmVlan_vlanDevFlags_t vlanDevFlags;
    int ret;

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Creating VLAN Interface on %s", realDevName);

    realDev = dev_get_by_name(&init_net, realDevName);
    if(!realDev)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not find Real Device %s", realDevName);
        return NULL;
    }

    if(realDev->features & NETIF_F_VLAN_CHALLENGED)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLANs not supported by %s", realDev->name);
        goto out_put_dev;
    }

    /* The real device must be up and operating in order to
       associate a VLAN device with it */
    if (!(realDev->flags & IFF_UP))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Device %s is not UP", realDev->name);
        goto out_put_dev;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    newDev = alloc_netdev(sizeof(bcmVlan_devInfo_t), name, vlanSetup);
#else
    newDev = alloc_netdev(sizeof(bcmVlan_devInfo_t), name, NET_NAME_UNKNOWN, vlanSetup);
#endif

    if (newDev == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate %s for %s", name, realDev->name);
        goto out_put_dev;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Allocated new VLAN Interface: %s", newDev->name);

    /* Inherit the realDev flags as long as the real device is not a bridge */
    if(!(realDev->priv_flags & IFF_EBRIDGE))
    {
        newDev->priv_flags |= (realDev->priv_flags & ~IFF_BRIDGE_PORT);
    }

    /* Clear the bonding flags for the newly created device */
    newDev->priv_flags &= ~(IFF_BONDING);
    newDev->flags &= ~(IFF_MASTER | IFF_SLAVE);

    net = dev_net(realDev);
    dev_net_set(newDev, net);

    /* FIXME: need N*4 bytes for extra VLAN header info, hope the underlying
       device can handle it */
    newDev->mtu = realDev->mtu;
    newDev->path.hw_port_type = realDev->path.hw_port_type;

    /* Initialize some VLAN dev info members */
    BCM_VLAN_DEV_INFO(newDev)->realDev = realDev;

#if defined(CONFIG_BCM_VLAN_ISOLATION)
    memset(BCM_VLAN_DEV_INFO(newDev)->vids, 0, sizeof(BCM_VLAN_DEV_INFO(newDev)->vids));
#endif

    /* FIXME: Do we need to support Netlink ops??? */
//    newDev->rtnl_link_ops = ;

    /* We will make changes to a net_device structure, so get the
       routing netlink semaphore */
    rtnl_lock();

    if (register_netdevice(newDev))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to REGISTER VLAN Device %s", newDev->name);
        goto out_free_newdev;
    }

/*     BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "After register_netdevice(): realDev->refcnt %d, vlanDev->refcnt %d", */
/*                   realDev->refcnt.counter, vlanDev->refcnt.counter); */

//    dev_hold(realDev);  --> dev_get_by_name() already calls dev_hold() for the real device

    /* register_netdevice() checks for uniqueness of device name,
       so we don't have to worry about checking if the new device
       already exists */

    vlanDevFlags.u32 = 0;
    vlanDevFlags.routed = (isRouted) ? 1 : 0;
    vlanDevFlags.multicast = (isMulticast) ? 1 : 0;
    vlanDevFlags.swOnly = (isSwOnly) ? 1 : 0;

    ret = bcmVlan_createVlanDevice(realDev, newDev, vlanDevFlags);
    if(ret)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to ADD VLAN Device %s", newDev->name);

        goto out_unregister_free;
    }

    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
    if(realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Internal Error: Could not get realDevCtrl for %s",
                      newDev->name);

        goto out_unregister_free;
    }

    if(realDevCtrl->mode == BCM_VLAN_MODE_ONT)
    {
        newDev->priv_flags &= ~IFF_HW_SWITCH;
    }

/*     BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "After bcmVlan_createVlanDevice(): realDev->refcnt %d, vlanDev->refcnt %d", */
/*                   realDev->refcnt.counter, vlanDev->refcnt.counter); */

    bcmVlan_transferOperstate(realDev);

    /* FIXME: What is this ??? Increment vlanDev refcounter! */
    linkwatch_fire_event(newDev); /* _MUST_ call rfc2863_policy() */

#if !defined(CONFIG_BCM_VLAN_ROUTED_WAN_USES_ROOT_DEV_MAC)
    if(isRouted) 
    {
        ret = allocMacAddress(newDev);
        if(ret)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate %s MAC Address", newDev->name);

            goto out_unregister_free;
        }
        vlanctl_notify_route_mac(newDev->dev_addr, 1);
    }
#endif

    ret = netdev_path_add(newDev, realDev);
    if(ret)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to add %s to Interface path (%d)",
                      newDev->name, ret);

        goto out_unregister_free;
    }

    netdev_path_dump(newDev);

#if defined(CONFIG_BCM_VLAN_ISOLATION)
    if (!isSwOnly)
    {
        vlanctl_notify_vlan_set_iso(newDev, 1);
    }
#endif

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Registered new VLAN Interface: %s", newDev->name);

    goto out_unlock;

out_unregister_free:
    unregister_netdevice(newDev);

out_free_newdev:
    rtnl_unlock();
    free_netdev(newDev);

out_put_dev:
    dev_put(realDev);
    return NULL; 

out_unlock:
    rtnl_unlock();

    return newDev;
}

static struct net_device *registerVlanDevice(const char *realDevName, int vlanDevId,
                                             int isRouted, int isMulticast,
                                             int isSwOnly)
{
    char name[IFNAMSIZ];

    /* allocate net_device structure for the new vlan interface */
    snprintf(name, IFNAMSIZ, "%s%s%d", realDevName, bcmVlan_ifNameSuffix, vlanDevId);

    return registerVlanDeviceByName(realDevName,name,isRouted,isMulticast,isSwOnly);
}

static int vlanOpen(struct inode *ip, struct file *fp)
{
//    BCM_LOG_FUNC(BCM_LOG_ID_VLAN);

    return 0;
}

static int vlanClose(struct inode *ip, struct file *fp)
{
//    BCM_LOG_FUNC(BCM_LOG_ID_VLAN);

    return 0;
}

static int dumpLocalStats(char *realDevName)
{
    int ret = 0;
    struct net_device *realDev;
    
    rtnl_lock();

    realDev = dev_get_by_name(&init_net, realDevName);

    if(realDev != NULL)
    {
        struct realDeviceControl *realDevCtrl;

        /******** CRITICAL REGION BEGIN ********/
        BCM_VLAN_GLOBAL_LOCK();

        realDevCtrl = bcmVlan_getRealDevCtrl(realDev);

        if(realDevCtrl != NULL)
        {
            bcmVlan_localStats_t *localStats = &realDevCtrl->localStats;

            printk("*** %s Local Stats ***\n", realDev->name);

            printk("rx_Misses             : %d\n", localStats->rx_Misses);
            printk("tx_Misses             : %d\n", localStats->tx_Misses);
            printk("error_PopUntagged     : %d\n", localStats->error_PopUntagged);
            printk("error_PopNoMem        : %d\n", localStats->error_PopNoMem);
            printk("error_PushTooManyTags : %d\n", localStats->error_PushTooManyTags);
            printk("error_PushNoMem       : %d\n", localStats->error_PushNoMem);
            printk("error_SetEtherType    : %d\n", localStats->error_SetEtherType);
            printk("error_SetTagEtherType : %d\n", localStats->error_SetTagEtherType);
            printk("error_InvalidTagNbr   : %d\n", localStats->error_InvalidTagNbr);
            printk("error_UnknownL3Header : %d\n", localStats->error_UnknownL3Header);
        }
        else
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "%s has no BCM VLAN Interfaces", realDev->name);

            ret = -EINVAL;
        }

        BCM_VLAN_GLOBAL_UNLOCK();
        /******** CRITICAL REGION END ********/

        dev_put(realDev);
    }

    rtnl_unlock();

    return ret;
}

static int vlanIoctl(struct inode *ip, struct file *fp, unsigned int cmd, 
                     unsigned long arg)
{
    int ret = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Command = %d", cmd);

    switch((bcmVlan_ioctlCmd_t)(cmd))
    {
        case BCM_VLAN_IOC_CREATE_VLAN:
        {
            struct net_device *vlanDev;
            bcmVlan_iocCreateVlan_t iocCreateVlan;

            copy_from_user(&iocCreateVlan, (bcmVlan_iocCreateVlan_t *)arg,
                           sizeof(bcmVlan_iocCreateVlan_t));

            vlanDev = registerVlanDevice(iocCreateVlan.realDevName, iocCreateVlan.vlanDevId,
                                         iocCreateVlan.isRouted, iocCreateVlan.isMulticast,
                                         iocCreateVlan.isSwOnly);
            if(vlanDev == NULL)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to create VLAN device (%s, %d)",
                              iocCreateVlan.realDevName, iocCreateVlan.vlanDevId);
                ret = -ENODEV;
            }

            break;
        }

        case BCM_VLAN_IOC_CREATE_VLAN_BY_NAME:
        {
            struct net_device *vlanDev;
            bcmVlan_iocCreateVlanByName_t iocCreateVlan;
        
            copy_from_user(&iocCreateVlan, (bcmVlan_iocCreateVlanByName_t *)arg,
                           sizeof(bcmVlan_iocCreateVlanByName_t));
        
            vlanDev = registerVlanDeviceByName(iocCreateVlan.realDevName, iocCreateVlan.vlanDevName,
                                               iocCreateVlan.isRouted, iocCreateVlan.isMulticast,
                                               iocCreateVlan.isSwOnly);
            if(vlanDev == NULL)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to create VLAN device (%s, %s)",
                              iocCreateVlan.realDevName, iocCreateVlan.vlanDevName);
                ret = -ENODEV;
            }
        
            break;
        }

        case BCM_VLAN_IOC_DELETE_VLAN:
        {
            bcmVlan_iocDeleteVlan_t iocDeleteVlan;

            copy_from_user(&iocDeleteVlan, (bcmVlan_iocDeleteVlan_t *)arg,
                           sizeof(bcmVlan_iocDeleteVlan_t));

            ret = unregisterVlanDevice(iocDeleteVlan.vlanDevName);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to delete VLAN device %s",
                              iocDeleteVlan.vlanDevName);
            }

            break;
        }

        case BCM_VLAN_IOC_CREATE_VLAN_FLOWS:
        {
	         struct net_device *rxdev, *txdev;
            bcmVlan_iocVlanFlows_t iocVlanFlows;
        
            copy_from_user(&iocVlanFlows, (bcmVlan_iocVlanFlows_t *)arg,
                           sizeof(bcmVlan_iocVlanFlows_t));
                           
            rxdev = dev_get_by_name(&init_net, iocVlanFlows.rxVlanDevName);
            if(rxdev == NULL)
            {
                BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Could not find VLAN Device %s",
                             iocVlanFlows.rxVlanDevName);
                ret = -EINVAL;
                break;
            }
            
            txdev = dev_get_by_name(&init_net, iocVlanFlows.txVlanDevName);
            if(txdev == NULL)
            {
                BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Could not find VLAN Device %s", 
                             iocVlanFlows.txVlanDevName);
                ret = -EINVAL;
                dev_put(rxdev);
                break;
            }
            
            ret = bcmVlan_flowPath_create(rxdev, txdev);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to create VLAN flows for path (%s, %s)",
                              iocVlanFlows.rxVlanDevName, iocVlanFlows.txVlanDevName);
            }

            /* remove the reference to the vlan device added by dev_get_by_name() */
            dev_put(rxdev);
            dev_put(txdev);

            break;
        }

        case BCM_VLAN_IOC_DELETE_VLAN_FLOWS:
        {
	         struct net_device *rxdev=NULL, *txdev=NULL;
            bcmVlan_iocVlanFlows_t iocVlanFlows;
        
            copy_from_user(&iocVlanFlows, (bcmVlan_iocVlanFlows_t *)arg,
                           sizeof(bcmVlan_iocVlanFlows_t));
                           
            if (strlen(iocVlanFlows.rxVlanDevName))
            {
                rxdev = dev_get_by_name(&init_net, iocVlanFlows.rxVlanDevName);
                if(rxdev == NULL)
                {
                    BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Could not find VLAN Device %s",
                                 iocVlanFlows.rxVlanDevName);
                    ret = -EINVAL;
                    break;
                }
            }

            if (strlen(iocVlanFlows.txVlanDevName))
            {
                txdev = dev_get_by_name(&init_net, iocVlanFlows.txVlanDevName);
                if(txdev == NULL)
                {
                    BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Could not find VLAN Device %s", 
                                 iocVlanFlows.txVlanDevName);
                    ret = -EINVAL;
                    if (rxdev)
                       dev_put(rxdev);
                    break;
                }
            }
            
            ret = bcmVlan_flowPath_delete(rxdev, txdev);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to delete VLAN flows for path (%s, %s)",
                              iocVlanFlows.rxVlanDevName, iocVlanFlows.txVlanDevName);
            }

            /* remove the reference to the vlan device added by dev_get_by_name() */
            if (rxdev)
                dev_put(rxdev);
            if (txdev)
                dev_put(txdev);

            break;
        }

        case BCM_VLAN_IOC_INSERT_TAG_RULE:
        {
            bcmVlan_iocInsertTagRule_t iocInsertTagRule;

            copy_from_user(&iocInsertTagRule, (bcmVlan_iocInsertTagRule_t *)arg,
                           sizeof(bcmVlan_iocInsertTagRule_t));

            ret = bcmVlan_insertTagRule(iocInsertTagRule.ruleTableId.realDevName,
                                        iocInsertTagRule.ruleTableId.nbrOfTags,
                                        iocInsertTagRule.ruleTableId.tableDir,
                                        &iocInsertTagRule.tagRule,
                                        iocInsertTagRule.position,
                                        iocInsertTagRule.posTagRuleId);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to Insert Tag Rule in %s (tags=%d, dir=%d)",
                              iocInsertTagRule.ruleTableId.realDevName, iocInsertTagRule.ruleTableId.nbrOfTags,
                              (int)iocInsertTagRule.ruleTableId.tableDir);
            }
            else
            {
                BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Inserted Tag Rule in %s (tags=%d, dir=%d, id=%d)",
                             iocInsertTagRule.ruleTableId.realDevName, iocInsertTagRule.ruleTableId.nbrOfTags,
                             (int)iocInsertTagRule.ruleTableId.tableDir, (int)iocInsertTagRule.tagRule.id);

                /* send argument back, containing the assigned rule tag Id */
                copy_to_user((bcmVlan_iocInsertTagRule_t *)arg, &iocInsertTagRule,
                             sizeof(bcmVlan_iocInsertTagRule_t));
            }

            break;
        }

        case BCM_VLAN_IOC_REMOVE_TAG_RULE:
        {
            bcmVlan_iocRemoveTagRule_t iocRemoveTagRule;

            copy_from_user(&iocRemoveTagRule, (bcmVlan_iocRemoveTagRule_t *)arg,
                           sizeof(bcmVlan_iocRemoveTagRule_t));

            if(iocRemoveTagRule.tagRuleId == BCM_VLAN_DONT_CARE)
            {
                ret = bcmVlan_removeTagRuleByFilter(iocRemoveTagRule.ruleTableId.realDevName,
                                                    iocRemoveTagRule.ruleTableId.nbrOfTags,
                                                    iocRemoveTagRule.ruleTableId.tableDir,
                                                    &iocRemoveTagRule.tagRule);
            }
            else
            {
                ret = bcmVlan_removeTagRuleById(iocRemoveTagRule.ruleTableId.realDevName,
                                                iocRemoveTagRule.ruleTableId.nbrOfTags,
                                                iocRemoveTagRule.ruleTableId.tableDir,
                                                iocRemoveTagRule.tagRuleId);
            }

            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to removed Tag Rule from %s (tags=%d, dir=%d, id=%d)",
                              iocRemoveTagRule.ruleTableId.realDevName, iocRemoveTagRule.ruleTableId.nbrOfTags,
                              (int)iocRemoveTagRule.ruleTableId.tableDir, (int)iocRemoveTagRule.tagRuleId);
            }
            else
            {
                BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Removed Tag Rule from %s (tags=%d, dir=%d, id=%d)",
                             iocRemoveTagRule.ruleTableId.realDevName, iocRemoveTagRule.ruleTableId.nbrOfTags,
                             (int)iocRemoveTagRule.ruleTableId.tableDir, (int)iocRemoveTagRule.tagRuleId);
            }

            break;
        }

        case BCM_VLAN_IOC_REMOVE_ALL_TAG_RULE:
        {
            bcmVlan_iocRemoveAllTagRule_t iocRemoveAllTagRule;
    
            copy_from_user(&iocRemoveAllTagRule, (bcmVlan_iocRemoveAllTagRule_t *)arg,
                           sizeof(bcmVlan_iocRemoveAllTagRule_t));
    
            ret = bcmVlan_removeAllTagRulesByDev(iocRemoveAllTagRule.vlanDevName);
    
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to removed ALL Tag Rule for Dev <%s>",
                              iocRemoveAllTagRule.vlanDevName);
            }
            else
            {
                BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Removed ALL Tag Rule for Dev <%s>",
                             iocRemoveAllTagRule.vlanDevName);
            }
    
            break;
        }

        case BCM_VLAN_IOC_DUMP_RULE_TABLE:
        {
            bcmVlan_iocDumpRuleTable_t iocDumpRuleTable;

            copy_from_user(&iocDumpRuleTable, (bcmVlan_iocDumpRuleTable_t *)arg,
                           sizeof(bcmVlan_iocDumpRuleTable_t));

            ret = bcmVlan_dumpTagRules(iocDumpRuleTable.ruleTableId.realDevName,
                                              iocDumpRuleTable.ruleTableId.nbrOfTags,
                                              iocDumpRuleTable.ruleTableId.tableDir);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to dump Rule Table from %s (tags=%d, dir=%d)",
                              iocDumpRuleTable.ruleTableId.realDevName, iocDumpRuleTable.ruleTableId.nbrOfTags,
                              (int)iocDumpRuleTable.ruleTableId.tableDir);
            }

            break;
        }

        case BCM_VLAN_IOC_DUMP_ALL_RULES:
        {

            ret = bcmVlan_dumpAllTagRules();
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to dump all rules");
            }

            break;
        }

        case BCM_VLAN_IOC_GET_NBR_OF_RULES_IN_TABLE:
        {
            bcmVlan_iocGetNbrOfRulesInTable_t iocGetNbrOfRulesInTable;

            copy_from_user(&iocGetNbrOfRulesInTable, (bcmVlan_iocGetNbrOfRulesInTable_t *)arg,
                           sizeof(bcmVlan_iocGetNbrOfRulesInTable_t));

            ret = bcmVlan_getNbrOfTagRulesByTable(iocGetNbrOfRulesInTable.ruleTableId.realDevName,
                                              iocGetNbrOfRulesInTable.ruleTableId.nbrOfTags,
                                              iocGetNbrOfRulesInTable.ruleTableId.tableDir,
                                              &(iocGetNbrOfRulesInTable.nbrOfRules));
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to get number of rules in table from %s (tags=%d, dir=%d)",
                              iocGetNbrOfRulesInTable.ruleTableId.realDevName, iocGetNbrOfRulesInTable.ruleTableId.nbrOfTags,
                              (int)iocGetNbrOfRulesInTable.ruleTableId.tableDir);
            }
            else
                copy_to_user((bcmVlan_iocGetNbrOfRulesInTable_t *)arg, &iocGetNbrOfRulesInTable,
                               sizeof(bcmVlan_iocGetNbrOfRulesInTable_t));

            break;
        }

        case BCM_VLAN_IOC_SET_DEFAULT_TAG:
        {
            bcmVlan_iocSetDefaultVlanTag_t iocSetDefaultVlanTag;

            copy_from_user(&iocSetDefaultVlanTag, (bcmVlan_iocSetDefaultVlanTag_t *)arg,
                           sizeof(bcmVlan_iocSetDefaultVlanTag_t));

            ret = bcmVlan_setDefaultVlanTag(iocSetDefaultVlanTag.ruleTableId.realDevName,
                                            iocSetDefaultVlanTag.ruleTableId.nbrOfTags,
                                            iocSetDefaultVlanTag.ruleTableId.tableDir,
                                            iocSetDefaultVlanTag.defaultTpid,
                                            iocSetDefaultVlanTag.defaultPbits,
                                            iocSetDefaultVlanTag.defaultCfi,
                                            iocSetDefaultVlanTag.defaultVid);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to set default VLAN Tag of %s (tags=%d, dir=%d)",
                              iocSetDefaultVlanTag.ruleTableId.realDevName, iocSetDefaultVlanTag.ruleTableId.nbrOfTags,
                              (int)iocSetDefaultVlanTag.ruleTableId.tableDir);
            }

            break;
        }

        case BCM_VLAN_IOC_SET_DSCP_TO_PBITS:
        {
            bcmVlan_iocDscpToPbits_t iocDscpToPbits;

            copy_from_user(&iocDscpToPbits, (bcmVlan_iocDscpToPbits_t *)arg,
                           sizeof(bcmVlan_iocDscpToPbits_t));

            ret = bcmVlan_setDscpToPbitsTable(iocDscpToPbits.realDevName,
                                              iocDscpToPbits.dscp,
                                              iocDscpToPbits.pbits);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to set DSCP to PBITS table entry: %s, dscp %d, pbits %d",
                              iocDscpToPbits.realDevName, iocDscpToPbits.dscp, iocDscpToPbits.pbits);
            }

            break;
        }

        case BCM_VLAN_IOC_DUMP_DSCP_TO_PBITS:
        {
            bcmVlan_iocDscpToPbits_t iocDscpToPbits;

            copy_from_user(&iocDscpToPbits, (bcmVlan_iocDscpToPbits_t *)arg,
                           sizeof(bcmVlan_iocDscpToPbits_t));

            ret = bcmVlan_dumpDscpToPbitsTable(iocDscpToPbits.realDevName, iocDscpToPbits.dscp);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to dump DSCP to PBITS table: %s, dscp %d",
                              iocDscpToPbits.realDevName, iocDscpToPbits.dscp);
            }

            break;
        }

        case BCM_VLAN_IOC_DUMP_LOCAL_STATS:
        {
            bcmVlan_iocDumpLocalStats_t iocDumpLocalStats;

            copy_from_user(&iocDumpLocalStats, (bcmVlan_iocDumpLocalStats_t *)arg,
                           sizeof(bcmVlan_iocDumpLocalStats_t));

            ret = dumpLocalStats(iocDumpLocalStats.realDevName);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to dump Local Stats of %s", iocDumpLocalStats.realDevName);
            }

            break;
        }

        case BCM_VLAN_IOC_SET_TPID_TABLE:
        {
            bcmVlan_iocSetTpidTable_t iocSetTpidTable;

            copy_from_user(&iocSetTpidTable, (bcmVlan_iocSetTpidTable_t *)arg,
                           sizeof(bcmVlan_iocSetTpidTable_t));

            ret = bcmVlan_setTpidTable(iocSetTpidTable.realDevName, iocSetTpidTable.tpidTable);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to Set TPID Table in %s",
                              iocSetTpidTable.realDevName);
            }

            break;
        }

        case BCM_VLAN_IOC_DUMP_TPID_TABLE:
        {
            bcmVlan_iocDumpTpidTable_t iocDumpTpidTable;

            copy_from_user(&iocDumpTpidTable, (bcmVlan_iocDumpTpidTable_t *)arg,
                           sizeof(bcmVlan_iocDumpTpidTable_t));

            ret = bcmVlan_dumpTpidTable(iocDumpTpidTable.realDevName);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to Dump TPID Table for %s",
                              iocDumpTpidTable.realDevName);
            }

            break;
        }

        case BCM_VLAN_IOC_SET_IF_SUFFIX:
        {
            bcmVlan_iocSetIfSuffix_t iocSetIfSuffix;

            copy_from_user(&iocSetIfSuffix, (bcmVlan_iocSetIfSuffix_t *)arg,
                           sizeof(bcmVlan_iocSetIfSuffix_t));

            snprintf(bcmVlan_ifNameSuffix, BCM_VLAN_IF_SUFFIX_SIZE,
                     "%s", iocSetIfSuffix.suffix);

            break;
        }

        case BCM_VLAN_IOC_SET_DEFAULT_ACTION:
        {
            bcmVlan_iocSetDefaultAction_t iocSetDefaultAction;

            copy_from_user(&iocSetDefaultAction, (bcmVlan_iocSetDefaultAction_t *)arg,
                           sizeof(bcmVlan_iocSetDefaultAction_t));

            ret = bcmVlan_setDefaultAction(iocSetDefaultAction.ruleTableId.realDevName,
                                           iocSetDefaultAction.ruleTableId.nbrOfTags,
                                           iocSetDefaultAction.ruleTableId.tableDir,
                                           iocSetDefaultAction.defaultAction,
                                           iocSetDefaultAction.defaultRxVlanDevName);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to set default action: %s, "
                              "tags=%d, dir=%d, defaultAction=%d, defaultRxVlanDevName %s",
                              iocSetDefaultAction.ruleTableId.realDevName,
                              iocSetDefaultAction.ruleTableId.nbrOfTags,
                              (int)iocSetDefaultAction.ruleTableId.tableDir,
                              iocSetDefaultAction.defaultAction,
                              iocSetDefaultAction.defaultRxVlanDevName);
            }

            break;
        }

        case BCM_VLAN_IOC_SET_REAL_DEV_MODE:
        {
            bcmVlan_iocSetRealDevMode_t iocSetRealDevMode;

            copy_from_user(&iocSetRealDevMode, (bcmVlan_iocSetRealDevMode_t *)arg,
                           sizeof(bcmVlan_iocSetRealDevMode_t));

            ret = bcmVlan_setRealDevMode(iocSetRealDevMode.realDevName,
                                         iocSetRealDevMode.mode);

            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to set Real Device mode: %s (mode=%d)",
                              iocSetRealDevMode.realDevName,
                              iocSetRealDevMode.mode);
            }

            break;
        }

        case BCM_VLAN_IOC_RUN_TEST:
        {
            bcmVlan_iocRunTest_t iocRunTest;

            copy_from_user(&iocRunTest, (bcmVlan_iocRunTest_t *)arg,
                           sizeof(bcmVlan_iocRunTest_t));

            bcmVlan_runTest(&iocRunTest);

            break;
        }

        case BCM_VLAN_IOC_SET_DP:
        {
            bcmVlan_iocSetDropPrecedence_t iocSetDp;

            copy_from_user(&iocSetDp, (bcmVlan_iocSetDropPrecedence_t *)arg,
                           sizeof(bcmVlan_iocSetDropPrecedence_t));
            bcmVlan_setDp(&iocSetDp);

            break;
        }

        default:
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid command, %d", cmd);
            ret = -EINVAL;
    }

    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(vlanIoctlMutex);

static long vlanIoctl_unlocked(struct file *filep, unsigned int cmd, 
                               unsigned long arg)
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)    
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif

    mutex_lock(&vlanIoctlMutex);
    rt = vlanIoctl(inode, filep, cmd, arg);
    mutex_unlock(&vlanIoctlMutex);

    return rt;
}
#endif

static struct file_operations vlanFops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
    .unlocked_ioctl = vlanIoctl_unlocked,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = vlanIoctl_unlocked,
#endif
#else
    .ioctl   = vlanIoctl,
#endif
    .open    = vlanOpen,
    .release = vlanClose,
    .owner   = THIS_MODULE
};

int bcmVlan_userInit(void)
{
    int ret;

    /* Register the driver and link it to our fops */
    ret = register_chrdev(vlanMajor, BCM_VLAN_DEVICE_NAME, &vlanFops);
    if(ret < 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN,
                      "Unable to register %s char device (major=%d), error:%d\n",
                      BCM_VLAN_DEVICE_NAME, vlanMajor, ret);

        return ret;
    }

    if(vlanMajor == 0)
    {
        vlanMajor = ret;
    }

    snprintf(bcmVlan_ifNameSuffix, BCM_VLAN_IF_SUFFIX_SIZE,
             "%s", BCM_VLAN_IF_SUFFIX_DEFAULT);

    return 0;
}

void bcmVlan_userCleanup(void)
{
    unregister_chrdev(vlanMajor, BCM_VLAN_DEVICE_NAME);

    if(vlanMajor != BCM_VLAN_DEVICE_MAJOR)
    {
        vlanMajor = 0;
    }
}
