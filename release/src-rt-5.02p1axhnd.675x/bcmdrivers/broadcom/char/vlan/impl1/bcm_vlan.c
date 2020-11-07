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
// File Name  : bcm_vlan.c
//
// Description: Broadcom VLAN Interface Driver
//               
//**************************************************************************

#include "bcm_vlan_local.h"
#include "bcm_vlan_dev.h"
#include "bcm_vlan_flows.h"


#define BCM_VLAN_MODULE_NAME    "Broadcom 802.1Q VLAN Interface"
#define BCM_VLAN_MODULE_VERSION "0.1"


/*
 * Local variables
 */
static int deviceEventHandler(struct notifier_block *unused, unsigned long event, void *ptr);

static struct notifier_block vlanNotifierBlock = {
    .notifier_call = deviceEventHandler,
};


/*
 * Global variables
 */
extern int (*bcm_vlan_handle_frame_hook)(struct sk_buff **);

/* 
 * Local Functions
 */
static int deviceEventHandler(struct notifier_block *unused, unsigned long event, void *ptr)
{
    struct net_device *dev = NETDEV_NOTIFIER_GET_DEV(ptr);

    /******** CRITICAL REGION BEGIN ********/
    /* Getting BCM_VLAN_LOCK here blocks re-entrance to this function,
     * However, netdevice notification might need to re-enter this function
     * to perform the same event for each vlan devices that are associated
     * with this real device. This lock is not needed.
     */
//    BCM_VLAN_LOCK();

    /* We run under the RTNL lock here */

    switch (event) {

	case NETDEV_CHANGE:
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "NETDEV_CHANGE");
            /* Propagate real device state to vlan devices */
            bcmVlan_transferOperstate(dev);
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Done");
            break;

	case NETDEV_DOWN:
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "NETDEV_DOWN");
            /* Put all VLANs for this dev in the down state too.  */
            bcmVlan_updateInterfaceState(dev, NETDEV_DOWN);
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Done");
            break;

	case NETDEV_UP:
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "NETDEV_UP");
            /* Put all VLANs for this dev in the up state too.  */
            bcmVlan_updateInterfaceState(dev, NETDEV_UP);
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Done");
            break;

	case NETDEV_UNREGISTER:
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "NETDEV_UNREGISTER");
            /* Delete all VLAN interfaces of this real device */
            bcmVlan_freeRealDeviceVlans(dev);
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Done");
            break;
    };

//    BCM_VLAN_UNLOCK();
    /******** CRITICAL REGION END ********/

    return NOTIFY_DONE;
}

static int __init moduleInit(void)
{
    int ret;

    printk(KERN_INFO "%s, v%s\n", BCM_VLAN_MODULE_NAME, BCM_VLAN_MODULE_VERSION);
    
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    spin_lock_init(&bcmVlan_rx_lock_g);
    spin_lock_init(&bcmVlan_tx_lock_g);
#endif

    ret = bcmVlan_initVlanDevices();
    if(ret)
    {
        return ret;
    }

    ret = bcmVlan_initTagRules();
    if(ret)
    {
        bcmVlan_cleanupVlanDevices();

        return ret;
    }

    ret = bcmVlan_userInit();
    if(ret)
    {
        bcmVlan_cleanupTagRules();

        bcmVlan_cleanupVlanDevices();

        return ret;
    }
    
#if defined(CONFIG_BLOG)
    ret = bcmVlan_flows_init();
    if(ret)
    {
        bcmVlan_userCleanup();

        bcmVlan_cleanupTagRules();
        
        bcmVlan_cleanupVlanDevices();

        return ret;
    }
#endif

    bcmFun_reg(BCM_FUN_ID_VLAN_LOOKUP_DP, (bcmFun_t *) bcmVlan_lookupDp);

    /* Register to receive netdevice events */
    ret = register_netdevice_notifier(&vlanNotifierBlock);
    if (ret < 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to register notifier (%d)", ret);

        bcmVlan_flows_uninit();
        
        bcmVlan_userCleanup();

        bcmVlan_cleanupTagRules();
        
        bcmVlan_cleanupVlanDevices();
    }

    /* register our receive packet handler */
    bcm_vlan_handle_frame_hook = bcmVlan_devReceiveSkb;
    return ret;
}

/*
 *     Module 'remove' entry point.
 *     o delete /proc/net/router directory and static entries.
 */
static void __exit moduleCleanup(void)
{
    /* unregister our receive packet handler */
    bcm_vlan_handle_frame_hook = NULL;
    
    /* Unregister from receiving netdevice events */
    unregister_netdevice_notifier(&vlanNotifierBlock);

    bcmVlan_flows_uninit();
   
    bcmVlan_userCleanup();

    bcmVlan_cleanupTagRules();
   
    bcmVlan_cleanupVlanDevices();

}

module_init(moduleInit);
module_exit(moduleCleanup);

MODULE_LICENSE("Proprietary");
MODULE_VERSION(BCM_VLAN_MODULE_VERSION);
