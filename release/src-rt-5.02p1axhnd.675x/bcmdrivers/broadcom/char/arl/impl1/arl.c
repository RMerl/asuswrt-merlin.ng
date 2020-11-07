/*
* <:copyright-BRCM:2009:proprietary:standard
* 
*    Copyright (c) 2009 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

/*
 *******************************************************************************
 * File Name  : arl.c
 *
 * Description: This implementation supports the arl table management
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/ppp_defs.h>
#include <linux/version.h>
#include <net/ip.h>
#include <linux/bcm_log.h>
#include "bcmtypes.h"
#include "fcachehw.h"
#include "fap.h"
#include <bcm/bcmswapitypes.h>
#include "arl.h"
#include <linux/blog_rule.h>


#define arlLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_ARL, BCM_LOG_LEVEL_DEBUG)
#define arlDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_ARL, fmt, ##arg)
#define arlInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_ARL, fmt, ##arg)
#define arlNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_ARL, fmt, ##arg)
#define arlError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_ARL, fmt, ##arg)
#define arlPrint(fmt, arg...)   printk(fmt, ##arg)
#define ARLDBG(prt, fmt,arg...) BCM_LOGCODE(if(arlLogDebug)         \
                                                  prt(fmt, ##arg);)

#define ARL_PKT_MAX_FLOWS 128   /* should be power of 2 */

/*
 *------------------------------------------------------------------------------
 * ARL layer global statistics and active flow list
 *------------------------------------------------------------------------------
 */
typedef struct {
    uint32_t status;        /* status: Enable=1 or Disable=0 */

    uint32_t activates;     /* number of activate (downcalls)   */
    uint32_t failures;      /* number of activate failures      */
    uint32_t deactivates;   /* number of deactivate (downcalls) */
    uint32_t flushes;       /* number of clear (upcalls)        */
    uint32_t active;
} arlState_t;

static arlState_t arlState_g;   /* arl layer global context */

#if defined(CONFIG_BCM_FHW)
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
static FAP_CLEAR_HOOK fap_clear_hook_fp = (FAP_CLEAR_HOOK)NULL;
#else
static FC_CLEAR_HOOK fc_clear_hook_fp = (FC_CLEAR_HOOK)NULL;
#endif
#endif

/*
 *------------------------------------------------------------------------------
 *
 * Function   : arlConvIpToMac
 * Description: This function is converts ip addr to mac
 *
 * Parameters :
 *    ipa     : pointer to IP address
 *    maca    : pointer to MAC address
 *
 * Returns    : None
 *------------------------------------------------------------------------------
 */
static void arlConvIpToMac(const UINT8 *ipa, UINT8 *maca)
{
   maca[5] = 0x01;
   maca[4] = 0x00;
   maca[3] = 0x5e;
   maca[2] = 0x7F & ipa[1];
   maca[1] = ipa[2];
   maca[0] = ipa[3];

   return;
}

/*
 *------------------------------------------------------------------------------
 *
 * Function   : arlConvIp6ToMac
 * Description: This function converts ip6 addr to multicast mac
 *
 * Parameters :
 *    ipa6    : pointer to IP6 address
 *    maca    : pointer to MAC address
 *
 * Returns    : None
 *------------------------------------------------------------------------------
 */
static void arlConvIp6ToMac(const ip6_addr_t *ipa6, UINT8 *maca)
{
   maca[5] = 0x33;
   maca[4] = 0x33;
   maca[3] = ipa6->p8[12];
   maca[2] = ipa6->p8[13];
   maca[1] = ipa6->p8[14];
   maca[0] = ipa6->p8[15];

   return;
}

/*
 *------------------------------------------------------------------------------
 *
 * Function   : arlActivate
 * Description: This function is bound to the Flow Cache subsytem for the
 *              configuration of arl flows
 *              When a new flow is added to the flow cache hash table a
 *              request to configure this flow in hardware is made.
 *
 * Parameters :
 *    blog_p  : Pointer to a Blog_t object.
 *
 * Returns    : 16bit FHW_TUPLE (i.e. FlowIx) or ~0.
 *------------------------------------------------------------------------------
 */
int arlActivate(Blog_t *blog_p, uint32_t key_in)
{
    int ret = ARL_SUCCESS;
    struct ethswctl_data e;
    unsigned char mcMac[6];
    int rc;
    int i;
    bcmFun_t *enetIsSwSwitchPortFun;
    int isSwSwitched;

    BCM_ASSERT((blog_p!=BLOG_NULL));

    BCM_LOGCODE(if(arlLogDebug)
        { arlPrint("\n::: arlActivate :::\n"); blog_dump(blog_p); });

    if ((! blog_p->rx.multicast ) ||
        (NULL == bcm_arl_process_hook_g) ||
        (blog_p->tx.info.phyHdrType != BLOG_ENETPHY) ||
        (blog_p->key.l1_tuple.channel != 0xFF))
    {
        arlState_g.failures++;
        return ARL_ERROR;
    }

    /* cannot support ssm through arl */
    if (1 == blog_p->rx.info.bmap.PLD_IPv6)
    {
        if((blog_p->tupleV6.daddr.p32[0] & htonl(0xFFF0FFFF)) == htonl(0xFF300000))
        {
            arlState_g.failures++;
            return ARL_ERROR;
        }
        arlConvIp6ToMac(&blog_p->tupleV6.daddr, mcMac);
    }
    else
    {
        /* cannot support ssm through arl */
        if((blog_p->rx.tuple.daddr & htonl(0xFF000000)) == htonl(0xE8000000))
        {
            arlState_g.failures++;
            return ARL_ERROR;
        }
        arlConvIpToMac((UINT8 *)&blog_p->rx.tuple.daddr, mcMac);
    }

    memset(&e, 0, sizeof(e));
    e.type = TYPE_GET;
    e.op   = ETHSWARLACCESS;
    e.vid  = 0;
    e.unit = -1;
    for (i = 0; i < ETH_ALEN; i++) {
       e.mac[i] = mcMac[5-i];
    }

    /* returns MAC, unit, vid and val */
    rc = bcm_arl_process_hook_g((void *)&e);

    /* if entry already exists we need to remove entry and let fcache 
       handle it. Multiple ports for ARL leads to report flooding and 
       subsequently report suppression */
    if (rc == BCM_E_NONE)
    {
       ARLDBG(arlPrint,"ARL entry present - deactivate\n");
       
       /* remove entry from ARL */
       e.type = TYPE_SET;
       e.op   = ETHSWARLACCESS;
       e.val  = 0;
       /* clear ifname - e->mac, e->vid, and e->unit are preserved from GET access */ 
       memset(e.ifname, 0, IFNAMSIZ);
       bcm_arl_process_hook_g((void *)&e);

       arlState_g.deactivates++;
       arlState_g.active--;
       ret = ARL_ERROR;
    }
    else
    {
       /* cannot support soft-switched ports */
       enetIsSwSwitchPortFun = bcmFun_get(BCM_FUN_ID_ENET_IS_SWSWITCH_PORT);
       if ( enetIsSwSwitchPortFun )
       {
          isSwSwitched = enetIsSwSwitchPortFun(((struct net_device *)blog_p->tx_dev_p)->name);
          if ( isSwSwitched )
          {
             arlState_g.failures++;
             return ARL_ERROR;
          }
       }

       ARLDBG(arlPrint,"Activating entry\n");

       /* add entry to ARL */
       e.type = TYPE_SET;
       e.op   = ETHSWARLACCESS;
       e.vid  = 0;
       /* ifname is used to fill in the unit and port */
       memcpy(e.ifname, ((struct net_device *)blog_p->tx_dev_p)->name, IFNAMSIZ);
       for (i = 0; i < ETH_ALEN; i++) {
          e.mac[i] = mcMac[5-i];
       }
       bcm_arl_process_hook_g((void *)&e);

       arlState_g.activates++;
       arlState_g.active++;
    }

    ARLDBG(arlPrint, "::: arlActivate cumm_activates<%u> :::\n\n",
             arlState_g.activates);

    return ret;
}
/*
 *------------------------------------------------------------------------------
 * Function   : arlRefresh
 * Description: This function is invoked to check activity for a NATed flow
 * Parameters :
 *  hwTuple : 16bit index to refer to a ARL entry
 * Returns    : Total hits on this connection.
 *------------------------------------------------------------------------------
 */
int arlRefresh(uint16_t hwTuple, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
    int ret = ARL_SUCCESS;

    return ret; 
}

/*
 *------------------------------------------------------------------------------
 * Function   : arlDeactivate
 * Description: This function is invoked when a entry in the ARL needs to be
 *              removed.
 * Parameters :
 *  flowIx    : 16bit index to refer to a NATed flow in FAP
 *  blog_p    : pointer to a blog object (for multicast only)
 * Returns    : Remaining number of active port (for multicast only)
 *------------------------------------------------------------------------------
 */
int arlDeactivate(uint16_t hwTuple, uint32_t *pktsCnt_p,
                  uint32_t *octetsCnt_p, struct blog_t * blog_p)
{
    struct ethswctl_data e;
    unsigned char mcMac[6];
    int rc;
    int i;

    BCM_ASSERT((blog_p!=BLOG_NULL));

    BCM_LOGCODE(if(arlLogDebug)
        { arlPrint("\n::: arlDeactivate:::\n"); blog_dump(blog_p); });

    if ((! blog_p->rx.multicast ) ||
        (NULL == bcm_arl_process_hook_g))
    {
        arlState_g.failures++;
        return 0;
    }

    /* cannot support ssm through arl */
    if (1 == blog_p->rx.info.bmap.PLD_IPv6)
    {
        if((blog_p->tupleV6.daddr.p32[0] & htonl(0xFFF0FFFF)) == htonl(0xFF300000))
        {
            arlState_g.failures++;
            return 0;
        }
        arlConvIp6ToMac(&blog_p->tupleV6.daddr, mcMac);
    }
    else
    {
        /* cannot support ssm through arl */
        if((blog_p->rx.tuple.daddr & htonl(0xFF000000)) == htonl(0xE8000000))
        {
            arlState_g.failures++;
            return 0;
        }
        arlConvIpToMac((UINT8 *)&blog_p->rx.tuple.daddr, mcMac);
    }

    memset(&e, 0, sizeof(e));
    e.type = TYPE_GET;
    e.op   = ETHSWARLACCESS;
    e.vid  = 0;
    e.unit = -1;
    for (i = 0; i < ETH_ALEN; i++) {
       e.mac[i] = mcMac[5-i];
    }
    rc = bcm_arl_process_hook_g((void *)&e);
    if (rc == BCM_E_NONE)
    {
       /* remove ARL entry */
       e.type  = TYPE_SET;
       e.op    = ETHSWARLACCESS;
       e.val   = 0;
       memset(e.ifname, 0, IFNAMSIZ);
       /* e->mac, e->vid and e->unit preserved from GET access */ 
       bcm_arl_process_hook_g((void *)&e);

       arlState_g.deactivates++;
       arlState_g.active--;
    }
    else
    {
        arlState_g.failures++;
    }

    ARLDBG(arlPrint,
             "::: arlDeactivate cumm_deactivates<%u> :::\n", arlState_g.deactivates);

    /* the return value is the active port count */
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function   : arlResetStats
 * Description: This function is invoked to reset stats
 * Parameters :
 *     hwTuple: HW Engine instance and match index
 * Returns    : success
 *------------------------------------------------------------------------------
 */
int arlResetStats(uint16_t hwTuple)
{
    return ARL_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : arlClear
 * Description: This function is invoked when all entries pertaining to
 *              a matchIx in HW device (6368) need to be cleared.
 * Parameters :
 *  hwTuple  : HW Engine instance and match index
 * Returns    : success
 *------------------------------------------------------------------------------
 */
int arlClear(uint16_t hwTuple)
{
    return ARL_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : arlFlush
 * Description: Flushes all the ARL table entries 
 *             
 * Parameters : None
 *  
 * Returns    : success
 *------------------------------------------------------------------------------
 */
int arlFlush(void)
{
    struct ethswctl_data e;

    if (NULL == bcm_arl_process_hook_g)
    {
        arlState_g.failures++;
        return ARL_ERROR;
    }

    memset(&e, 0, sizeof(e));
    e.type = TYPE_FLUSH;
    e.op   = ETHSWARLACCESS;
    e.unit = -1;
    bcm_arl_process_hook_g((void *)&e);

    arlState_g.deactivates = arlState_g.activates;

    arlState_g.active = 0;

    ARLDBG(arlPrint,
             "::: arlDeactivate cumm_deactivates<%u> :::\n", arlState_g.deactivates);

    return ARL_SUCCESS;
}

/* 
 *------------------------------------------------------------------------------
 * Function   : arlStatus()
 * Description: Display ARL Tables status, summary
 *------------------------------------------------------------------------------
 */
void arlStatus(void)
{
    struct ethswctl_data e;

    if (NULL == bcm_arl_process_hook_g)
    {
        arlState_g.failures++;
        return;
    }

    memset(&e, 0, sizeof(e));
    e.type = TYPE_DUMP;
    e.op   = ETHSWARLACCESS;
    e.unit = -1;
    bcm_arl_process_hook_g((void *)&e);

    arlPrint("ARL:\n"
               "\tAcceleration %s, Active <%u>, Max <%u>\n"
               "\tActivates   : %u\n"
               "\tFailures    : %u\n"
               "\tDeactivates : %u\n"
               "\tFlushes     : %u\n\n",
               (arlState_g.status == 1) ?  "Enabled" : "Disabled",
               arlState_g.active, ARL_PKT_MAX_FLOWS,
               arlState_g.activates, arlState_g.failures,
               arlState_g.deactivates, arlState_g.flushes);

}

/*
 *------------------------------------------------------------------------------
 * Function   : arlEnable
 * Description: Binds the ARL config handler functions to Flow Cache hooks.
 *------------------------------------------------------------------------------
 */
void arlEnable(void)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    fap_bind_arl((HOOKP)arlActivate, (HOOK4PARM)arlDeactivate,
                (HOOK3PARM)arlRefresh, (HOOK32)arlResetStats, (HOOK32)arlClear,
                &fap_clear_hook_fp);

    BCM_ASSERT((fap_clear_hook_fp != (FAP_CLEAR_HOOK)NULL));

    arlState_g.status = 1;

    if(bcmLog_getLogLevel(BCM_LOG_ID_ARL) >= BCM_LOG_LEVEL_INFO)
    {
        arlStatus();
    }

    /* FIXME: waiting for FAP registration API*/
    arlNotice("Enabled ARL binding to FAP");

#elif (defined(CONFIG_BCM_PKTFLOW) || defined(CONFIG_BCM_PKTFLOW_MODULE)) && defined(CONFIG_BCM_FHW)
    FhwBindHwHooks_t hwHooks = {};
    FhwHwAccPrio_t prioIx = FHW_PRIO_0;

    hwHooks.activate_fn = (HOOKP32)arlActivate;
    hwHooks.deactivate_fn = (HOOK4PARM)arlDeactivate;
    hwHooks.update_fn = (HOOK3PARM)NULL;
    hwHooks.refresh_fn = (HOOK3PARM)arlRefresh;
    hwHooks.reset_stats_fn = (HOOK32)arlResetStats;
    hwHooks.fhw_clear_fn = &fc_clear_hook_fp;
    hwHooks.cap = ( (1<<HW_CAP_IPV4_MCAST) | (1<<HW_CAP_IPV6_MCAST) );

    hwHooks.max_ent = ARL_PKT_MAX_FLOWS;

    /* Bind to Flow Cache for learning connection configurations dynamically */
    hwHooks.clear_fn = (HOOK32)arlClear;
    fhw_bind_hw(prioIx, &hwHooks);

    BCM_ASSERT((fc_clear_hook_fp != (FC_CLEAR_HOOK)NULL));

    arlState_g.status = 1;

    if(bcmLog_getLogLevel(BCM_LOG_ID_ARL) >= BCM_LOG_LEVEL_INFO)
    {
        arlStatus();
    }

    arlNotice("Enabled ARL binding to Flow Cache");
#else
    arlNotice("FAP/Flow Cache not built.");
#endif
}


/*
 *------------------------------------------------------------------------------
 * Function   : arlDisable
 * Description: Clears all active Flow Cache associations with ARL.
 *              Unbind all flow cache to ARL hooks.
 *------------------------------------------------------------------------------
 */
void arlDisable(void)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))

    fap_bind_arl((HOOKP)NULL, (HOOK4PARM)NULL, (HOOK3PARM)NULL, (HOOK32)NULL,
                  (HOOK32)NULL, &fap_clear_hook_fp);

    fap_clear_hook_fp = (FAP_CLEAR_HOOK)NULL;

    arlState_g.status = 0;

    if(bcmLog_getLogLevel(BCM_LOG_ID_ARL) >= BCM_LOG_LEVEL_INFO)
    {
        arlStatus();
    }

    /* FIXME: waiting for FAP registration API*/
    arlNotice("Disaabled ARL binding to FAP");

#elif (defined(CONFIG_BCM_PKTFLOW) || defined(CONFIG_BCM_PKTFLOW_MODULE)) && defined(CONFIG_BCM_FHW)
    FhwBindHwHooks_t hwHooks = {};
    FhwHwAccPrio_t prioIx = FHW_PRIO_0;

    hwHooks.activate_fn = (HOOKP32)NULL;
    hwHooks.deactivate_fn = (HOOK4PARM)NULL;
    hwHooks.update_fn = (HOOK3PARM)NULL;
    hwHooks.refresh_fn = (HOOK3PARM)NULL;
    hwHooks.reset_stats_fn = (HOOK32)NULL;
    hwHooks.fhw_clear_fn = &fc_clear_hook_fp;
    hwHooks.cap = 0;
    hwHooks.max_ent = 0;

    hwHooks.clear_fn = (HOOK32)NULL;
    fhw_bind_hw(prioIx, &hwHooks);

    fc_clear_hook_fp = (FC_CLEAR_HOOK)NULL;

    arlState_g.status = 0;

    if(bcmLog_getLogLevel(BCM_LOG_ID_ARL) >= BCM_LOG_LEVEL_INFO)
    {
        arlStatus();
    }

    arlNotice("Disabled ARL binding to Flow Cache");
#else
    arlNotice("FAP/Flow Cache not built.");
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function   : arlReset
 * Description: Resets Flow Cache to have no reference to ARL flows
 *              and pre-initializes statistics and global state.
 *------------------------------------------------------------------------------
 */
void arlReset(void)
{

    if(arlState_g.status == 1)
    {
        arlDisable();
    }

    arlState_g.activates = arlState_g.failures
    = arlState_g.deactivates = arlState_g.flushes
    = arlState_g.active = 0;

    arlNotice("Reset ARL Tables");

}

/*------------------------------------------------------------------------------
 * Function   : arlDebug
 * Description: Sets the ARL log level
 *------------------------------------------------------------------------------
 */
int arlDebugConf(int logLevel)
{
    if(logLevel >= 0 && logLevel < BCM_LOG_LEVEL_MAX)
    {
        bcmLog_setLogLevel(BCM_LOG_ID_ARL, logLevel);
    }
    else
    {
        arlError("Invalid Log level %d (max %d)",
                   logLevel, BCM_LOG_LEVEL_MAX);

        return ARL_ERROR;
    }

    return ARL_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: arlIoctl
 * Description  : Main entry point to handle user applications IOCTL requests.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
static int arlIoctl(struct inode *inode, struct file *filep,
                    unsigned int command, unsigned long arg)
{
    arlIoctl_t cmd;
    int ret = ARL_SUCCESS;

    if (command >= ARL_IOC_MAX)
        cmd = ARL_IOC_MAX;
    else
        cmd = (arlIoctl_t)command;

    BCM_LOG_INFO(BCM_LOG_ID_ARL, "cmd<%d> arg<0x%08lX>", command, arg);

    switch( cmd )
    {
        case ARL_IOC_INIT:
            break;

        case ARL_IOC_SHOW:
            arlStatus();
            break;

        case ARL_IOC_FLUSH:
            ret = arlFlush();
            break;

        case ARL_IOC_DEBUG:
            arlDebugConf((int)arg);
            break;

        default:
            BCM_LOG_ERROR(BCM_LOG_ID_ARL, "Invalid Command [%u]", command);
            ret = ARL_ERROR;
            break;
    }

    return ret;

} /* arlIoctl */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(arlIoctlMutex);

static long unlocked_arlIoctl(struct file *filep, unsigned int cmd, 
                              unsigned long arg )
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)    
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif

    mutex_lock(&arlIoctlMutex);
    rt = arlIoctl( inode, filep, cmd, arg );
    mutex_unlock(&arlIoctlMutex);
    
    return rt;
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function Name: arlOpen
 * Description  : Called when an user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int arlOpen(struct inode *inode, struct file *filp)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_ARL, "Access ARL Char Device");
    return ARL_SUCCESS;
}

/* Global file ops */
static struct file_operations arlFops =
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
    .unlocked_ioctl = unlocked_arlIoctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = unlocked_arlIoctl,
#endif
#else
    .ioctl  = arlIoctl,
#endif
    .open   = arlOpen,
};

/*
 *------------------------------------------------------------------------------
 * Function Name: arlDrv_construct
 * Description  : Initial function that is called at system startup that
 *                registers this device. 
 * Returns      : None.  *------------------------------------------------------------------------------ */

int __init arlDrv_construct(void)
{
    if(register_chrdev(ARLDRV_MAJOR, ARLDRV_NAME, &arlFops))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_ARL, "Unable to get major number <%d>", ARLDRV_MAJOR);

        return ARL_ERROR;
    }

    arlEnable();

    printk(ARL_MODNAME " Char Driver " ARL_VER_STR " Registered <%d>\n", ARLDRV_MAJOR);

    /* debugging only */
    bcmLog_setLogLevel(BCM_LOG_ID_ARL, BCM_LOG_LEVEL_ERROR);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    return 0;
#else
    return ARLDRV_MAJOR;
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function Name: arlDrv_destruct
 * Description  : Final function that is called when the module is unloaded.
 *                
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void __exit arlDrv_destruct(void)
{

    unregister_chrdev(ARLDRV_MAJOR, ARLDRV_NAME);

    arlDisable();

    BCM_LOG_NOTICE(BCM_LOG_ID_ARL, ARL_MODNAME " Char Driver " ARL_VER_STR
                   " Unregistered<%d>", ARLDRV_MAJOR );
}

module_init(arlDrv_construct);
module_exit(arlDrv_destruct);

MODULE_DESCRIPTION(ARL_MODNAME);
MODULE_VERSION(ARL_VERSION);
MODULE_LICENSE("Proprietary");
