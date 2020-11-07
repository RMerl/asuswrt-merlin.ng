/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
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
* :>
*/

/***************************************************************************
 * File Name  : AdslDrv.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the ATM API driver.
 *
 * Updates    : 08/24/2001  lat.  Created.
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/module.h>
#ifdef CONFIG_BCM963158
#include <linux/notifier.h>
#include <linux/reboot.h>
#endif
#include <linux/signal.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/if_arp.h>
#include <asm/uaccess.h>

#include <bcmtypes.h>
#include <adsldrv.h>
#include <bcmadsl.h>
#include <bcmatmapi.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include <bcmxtmcfg.h>
#endif
#include <board.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#include <linux/kthread.h>
#include <bcmnetlink.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
#include <linux/sched.h>
#endif

#include <DiagDef.h>
#include "BcmOs.h"
#include "softdsl/SoftDsl.h"
#include "AdslCore.h"
#include "BcmAdslCore.h"
#include "BcmAdslDiag.h"
#if defined(CONFIG_BRCM_IKOS)
#include "AdslCoreMap.h"
#endif
#include "linux/poll.h"

#ifndef DSL_IFNAME
#define	DSL_IFNAME "dsl0"
#endif

#if defined(BCM_XTM_API_MAJ_VERSION) && defined(BCM_XTM_API_MIN_VERSION)
#if defined(CONFIG_BCM963381) && (BCM_XTM_API_VERSION(BCM_XTM_API_MAJ_VERSION,BCM_XTM_API_MIN_VERSION) < BCM_XTM_API_VERSION(2,5))
#define	NO_RATE_CHANGE_SUPPORT_IN_XTM_API
#endif
#else
#define	NO_RATE_CHANGE_SUPPORT_IN_XTM_API
#endif

#if defined(CONFIG_BCM_ATM_BONDING_ETH) || defined(CONFIG_BCM_ATM_BONDING_ETH_MODULE)
extern int atmbonding_enabled;
extern BCMADSL_STATUS BcmAdsl_SetGfc2VcMapping(int bOn) ;
#endif

#ifndef TRAFFIC_TYPE_PTM_RAW
#define	TRAFFIC_TYPE_PTM_RAW		3
#endif
#ifndef	TRAFFIC_TYPE_PTM_BONDED
#define	TRAFFIC_TYPE_PTM_BONDED	4
#endif
#ifndef	TRAFFIC_TYPE_ATM_BONDED
#define	TRAFFIC_TYPE_ATM_BONDED	0	/* The actual defined value in the new Bcmxtmcfg.h is 3, but since it is
												contradicted with TRAFFIC_TYPE_PTM_RAW, 0 is defined. This should cause
												 no harm b/c if Linux supports it, it would have been defined */
#endif

#ifdef CONFIG_BCM963158
extern void BcmXdslCoreNotifyPhyReset(void);

static int BcmXdsl_RebootCallback(struct notifier_block *self, unsigned long val,void *data)
{
	if (val == SYS_RESTART) {
		printk("%s: Notifying PHY reset\n", __FUNCTION__);
		BcmXdslCoreNotifyPhyReset();
	}
	return NOTIFY_DONE;
}

static struct notifier_block BcmXdsl_RebootNotifier = {
	.notifier_call = BcmXdsl_RebootCallback,
};
#endif

/*
* Send Msg to the user mode application to request ioctl call back so the driver
* can perform needed task that can not be done in softirq context
*/
void BcmXdsl_RequestIoCtlCallBack(void)
{
#ifdef DYING_GASP_API

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef MSG_NETLINK_BRCM_CALLBACK_DSL_DRV
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_CALLBACK_DSL_DRV,NULL,0);
#else
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,NULL,0);
#endif
#else
	kerSysWakeupMonitorTask();
#endif

#endif
}

#if defined(SUPPORT_MULTI_PHY) || defined(CONFIG_BCM_DSL_GFAST)
void BcmXdsl_RequestSavePreferredLine(void)
{
#if defined(DYING_GASP_API) && defined(MSG_NETLINK_BRCM_SAVE_DSL_CFG)
#ifndef MSG_ID_BRCM_SAVE_DSL_PREFERRED_LINE
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_SAVE_DSL_CFG, NULL, 0);
#else
	unsigned int msgData = MSG_ID_BRCM_SAVE_DSL_PREFERRED_LINE;
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_SAVE_DSL_CFG, (void *)&msgData, sizeof(msgData));
#endif
#endif
}
#endif

#ifdef SUPPORT_MULTI_PHY
void BcmXdsl_NotifyMisMatchTrafficType(void)
{
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_TRAFFIC_TYPE_MISMATCH, NULL, 0);
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
struct net_device_stats *dsl_get_stats(struct net_device *dev);
static const struct net_device_ops dsl_netdev_ops = {
    .ndo_get_stats      = dsl_get_stats,
 };
#endif


/* typedefs. */
typedef void (*ADSL_FN_IOCTL) (unsigned char lineId, unsigned long arg);

/* Externs. */
BCMADSL_STATUS BcmAdsl_SetVcEntry (int gfc, int port, int vpi, int vci);
BCMADSL_STATUS BcmAdsl_SetVcEntryEx (int gfc, int port, int vpi, int vci, int pti);
void BcmAdsl_AtmSetPortId(int path, int portId);
void BcmAdsl_AtmClearVcTable(void);
void BcmAdsl_AtmAddVc(int vpi, int vci);
void BcmAdsl_AtmDeleteVc(int vpi, int vci);
void BcmAdsl_AtmSetMaxSdu(unsigned short maxsdu);
static void BcmXdslDrvInitEocCtrl(void);
static BCMADSL_STATUS BcmXdslDrvOpenEocIf(unsigned lineId, int eocMsgType, char *pIfName);
static BCMADSL_STATUS BcmXdslDrvCloseEocIf(unsigned lineId, int eocMsgType, char *pIfName);

#if defined(NO_XTM_MODULE)	/* defined(_NOOS) || defined(CONFIG_BRCM_IKOS) */

#ifdef XTM_USE_DSL_MIB
void *g_pfnAdslGetObjValue;
#if defined(XTM_USE_DSL_SYSCTL)
void *g_pfnAdslSetObjValue;
#endif
#if defined(XTM_USE_DSL_WAN_NOTIFY)
void *g_pfnAdslWanDevState;
#endif
#endif /* XTM_USE_DSL_MIB */

BCMXTM_STATUS BcmXtm_SetInterfaceLinkInfo( uint ulPortId, PXTM_INTERFACE_LINK_INFO pLinkInfo )
{
	return XTMSTS_STATE_ERROR;
}

#ifdef CONFIG_BRCM_IKOS
void kerSysDisableDyingGaspInterrupt(void)
{
}
void kerSysEnableDyingGaspInterrupt(void)
{
}
#if 0
void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
}
void kerSysSendtoMonitorTask(int msgType, char *msgData, int msgDataLen)
{
}
int kerSysGetSdramSize(void)
{
	return 0;
}
int BpGetIntAFELDPwrDslCtl( unsigned short *pusValue )
{
	return 0;
}
int BpGetExtAFELDPwrDslCtl( unsigned short *pusValue )
{
	return 0;
}
#endif
#endif

#else /* !defined(NO_XTM_MODULE) */

#ifdef XTM_USE_DSL_MIB
extern void *g_pfnAdslGetObjValue;
#if defined(XTM_USE_DSL_SYSCTL)
extern void *g_pfnAdslSetObjValue;
#endif
#if defined(XTM_USE_DSL_WAN_NOTIFY)
extern void *g_pfnAdslWanDevState;
#endif
#endif /* XTM_USE_DSL_MIB */

#endif /* defined(NO_XTM_MODULE) */

/* Prototypes. */
static int __init adsl_init( void );
static void __exit adsl_cleanup( void );
static int adsl_open( struct inode *inode, struct file *filp );
int adsl_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg );
static void DoCheck( unsigned char lineId, unsigned long arg );
static void DoInitialize( unsigned char lineId, unsigned long arg );
static void DoUninitialize( unsigned char lineId, unsigned long arg );
static void DoConnectionStart( unsigned char lineId, unsigned long arg );
static void DoConnectionStop( unsigned char lineId, unsigned long arg );
static void DoGetPhyAddresses( unsigned char lineId, unsigned long arg );
static void DoSetPhyAddresses( unsigned char lineId, unsigned long arg );
static void DoMapATMPortIDs( unsigned char lineId, unsigned long arg );
static void DoGetConnectionInfo( unsigned char lineId, unsigned long arg );
static void DoDiagCommand( unsigned char lineId, unsigned long arg );
static void DoGetObjValue( unsigned char lineId, unsigned long arg );
static void DoSetObjValue( unsigned char lineId, unsigned long arg );
static void DoStartBERT( unsigned char lineId, unsigned long arg );
static void DoStopBERT( unsigned char lineId, unsigned long arg );
static void DoConfigure( unsigned char lineId, unsigned long arg );
static void DoTest( unsigned char lineId, unsigned long arg );
static void DoGetConstelPoints( unsigned char lineId, unsigned long arg );
static void DoGetVersion( unsigned char lineId, unsigned long arg );
static void DoSetSdramBase( unsigned char lineId, unsigned long arg );
static void DoResetStatCounters( unsigned char lineId, unsigned long arg );
static void DoSetOemParameter( unsigned char lineId, unsigned long arg );
static void DoBertStartEx( unsigned char lineId, unsigned long arg );
static void DoBertStopEx( unsigned char lineId, unsigned long arg );
static void AdslConnectCb(unsigned char lineId, ADSL_LINK_STATE dslLinkState, ADSL_LINK_STATE dslPrevLinkState, void *pParm );
static void DoCallBackDrv( unsigned char lineId, unsigned long arg );
static void DoOpenEocIf( unsigned char lineId, unsigned long arg );
static void DoCloseEocIf( unsigned char lineId, unsigned long arg );
#if defined(SUPPORT_HMI)
static void DoHmiCommand( unsigned char lineId, unsigned long arg );
#endif

#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
static ssize_t adsl_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos);
static ssize_t adsl_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos);
static unsigned int adsl_poll(struct file *file, poll_table *wait);
extern void dumpaddr( unsigned char *pAddr, int nLen );
#endif /* defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO) */

#if defined(XTM_USE_DSL_WAN_NOTIFY)
static void BcmXdslWanDevNotify(unsigned char bDevUp);  /* 1/0 - up/down */
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#define ARPHRD_CPCS     28              /* CPCS                         */
#define ARPHRD_DSL      29              /* ADSL                         */
#endif

typedef struct
{
  int readEventMask;    
  int writeEventMask;    
  char *buffer;
} ADSL_IO, *pADSL_IO;

static DEFINE_MUTEX(ioctlMutex);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))

static long adsl_unlocked_ioctl(struct file *filep,
   unsigned int cmd, unsigned long arg )
{
   struct inode *inode;
   long rt;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
   inode = filep->f_dentry->d_inode;
#else
   inode = file_inode(filep);
#endif
   mutex_lock(&ioctlMutex);
   rt = adsl_ioctl( inode, filep, cmd, arg );
   mutex_unlock(&ioctlMutex);

   return rt;
}

#else

static int BcmXdsl_IoctlWrapper( struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg )
{
	int rt;
	mutex_lock(&ioctlMutex);
	rt = adsl_ioctl(inode, flip, command, arg);
	mutex_unlock(&ioctlMutex);

	return rt;
}

#endif // >= KERNEL_VERSION(3,2,0)

/* Globals. */
static struct file_operations adsl_fops =
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
    .unlocked_ioctl = adsl_unlocked_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = adsl_unlocked_ioctl,
#endif
#else
    .ioctl      = BcmXdsl_IoctlWrapper,
#endif
    .open       = adsl_open,
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
    .read       = adsl_read,
    .write      = adsl_write,
    .poll       = adsl_poll,
#endif 
#else
    ioctl:      BcmXdsl_IoctlWrapper,
    open:       adsl_open,
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
    read:       adsl_read,
    write:      adsl_write,
    poll:       adsl_poll,
#endif 
#endif
};

static ushort g_usAtmFastPortId;
static ushort g_usAtmInterleavedPortId;
static struct net_device_stats g_dsl_if_stats;
static struct net_device *g_dslNetDev;
static int    drvInit = 0;
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
static int g_eoc_hdr_offset = 0;
static int g_eoc_hdr_len = 0;
#endif

#ifdef DSL_KTHREAD
struct task_struct * dslThreadTask = NULL;
static wait_queue_head_t dslThreadWaitQue;
extern void BcmXdslStatusPolling(void);
extern void XdslCoreSetAhifState(unsigned char lineId, uint state, uint event);
extern void *XdslCoreGetDslVars(unsigned char lineId);
extern Bool XdslMibIsGinpActive(void *gDslVars, unsigned char direction);
#if defined(SUPPORT_PROCESS_STAT_IN_THREAD)
extern void BcmCoreDpc(void);
extern Bool BcmXdslStatusDataAvail(void);
#else
extern Bool BcmXdslDrvReqEventPending(void);
extern void BcmXdslProcessDrvPendingReq(void);
#endif
void BcmXdslThreadWakeup(void);
void dslThreadCreate(void);
#ifdef STAT_HANDLING_PRINT
ulong thrRunCnt = 0;
#endif

static int dslThread(void * data)
{
    OS_TICKS osTime, osTime1;
    long timeOutInJiffies;

    init_waitqueue_head(&dslThreadWaitQue);
    timeOutInJiffies = HZ;
#ifdef STAT_HANDLING_PRINT
    thrRunCnt = 0;
    printk(KERN_INFO "***%s: timeOutInJiffies = %ld\n", __FUNCTION__, timeOutInJiffies);
#endif

    while (1) {
        timeOutInJiffies = wait_event_interruptible_timeout(
            dslThreadWaitQue,
#ifdef SUPPORT_PROCESS_STAT_IN_THREAD
            (false != BcmXdslStatusDataAvail()),
#else
            (false != BcmXdslDrvReqEventPending()),
#endif
            timeOutInJiffies);
        if (kthread_should_stop()) {
            printk(KERN_INFO "*** %s: kthread_should_stop detected ***\n", __FUNCTION__);
            break;
        }
#ifdef STAT_HANDLING_PRINT
        if(thrRunCnt < STAT_HANDLING_PRINT_MAX) {
            thrRunCnt++;
            bcmOsGetTime(&osTime);
            printk(KERN_INFO "Thr %lu: ot=%lX to=%ld da=%d\n",
                thrRunCnt, osTime, timeOutInJiffies,
#ifdef SUPPORT_PROCESS_STAT_IN_THREAD
                BcmXdslStatusDataAvail()
#else
                BcmXdslDrvReqEventPending()
#endif
                );
        }
#endif
        bcmOsGetTime(&osTime);
        if(timeOutInJiffies > 0) {
#ifdef SUPPORT_PROCESS_STAT_IN_THREAD
            BcmCoreDpc();
#else
            BcmXdslProcessDrvPendingReq();
#endif
            bcmOsGetTime(&osTime1);
            timeOutInJiffies -= (osTime1 - osTime);
            if(timeOutInJiffies < 1) {
                timeOutInJiffies = HZ;
                BcmXdslStatusPolling();
            }
        }
        else {
            timeOutInJiffies = HZ;
            BcmXdslStatusPolling();
#ifdef SUPPORT_PROCESS_STAT_IN_THREAD
            if(BcmXdslStatusDataAvail()) {
                BcmCoreDpc();
                bcmOsGetTime(&osTime1);
                timeOutInJiffies -= (osTime1 - osTime);
                if(timeOutInJiffies < 1)
                   timeOutInJiffies = 1;
            }
#endif
        }
    }
    
    printk(KERN_INFO "*** %s: Exiting ***\n", __FUNCTION__);
    
    return 0;
}

void BcmXdslThreadWakeup(void)
{
    wake_up_interruptible(&dslThreadWaitQue);
}

void dslThreadTerminate(void)
{
   if(dslThreadTask != NULL) {
      printk("*** %s: Stopping dslThread pid=%d ***\n", __FUNCTION__,  dslThreadTask->pid);
      kthread_stop(dslThreadTask);
      dslThreadTask = NULL;
   }
   printk("*** %s: Exiting ***\n", __FUNCTION__);
}

void dslThreadCreate(void)
{
   OS_STATUS sts = OS_STATUS_OK;
   
   if (NULL == dslThreadTask) {
      sts = bcmOsTaskCreate(DSL_IFNAME, 20*1024, 200, dslThread, NULL, (OS_TASKID *)&dslThreadTask);
      if (sts == OS_STATUS_OK) {
#ifdef SUPPORT_PROCESS_STAT_IN_THREAD
         printk("*** dslThread pid=%d, SSP ***\n", dslThreadTask->pid);
#else
         printk("*** dslThread pid=%d ***\n", dslThreadTask->pid);
#endif
      }
      else
         printk("*** FATAL: Create dslThread failed! ***\n");
   }
}
#endif  /* DSL_KTHREAD */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#if defined(MODULE)
/***************************************************************************
 * Function Name: init_module
 * Description  : Initial function that is called if this driver is compiled
 *                as a module.  If it is not, adsl_init is called in
 *                chr_dev_init() in drivers/char/mem.c.
 * Returns      : None.
 ***************************************************************************/
int init_module(void)
{
    return( adsl_init() );
}


/***************************************************************************
 * Function Name: cleanup_module
 * Description  : Final function that is called if this driver is compiled
 *                as a module.
 * Returns      : None.
 ***************************************************************************/
void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("adsl: cleanup_module failed because module is in use\n");
    else {
        adsl_cleanup();
    }
}
#endif //MODULE
#endif //LINUX_VERSION_CODE

/***************************************************************************
 * Function Name: dsl_config_netdev
 * Description  : Configure a network device for DSL interface
 * Returns      : None.
 ***************************************************************************/
struct net_device_stats *dsl_get_stats(struct net_device *dev)
{
/*
        uint interfaceId;
        ATM_INTERFACE_CFG Cfg;
        ATM_INTERFACE_STATS KStats;
        PATM_INTF_ATM_STATS pAtmStats = &KStats.AtmIntfStats;

        BcmAtm_GetInterfaceId(0, &interfaceId);
        Cfg.ulStructureId = ID_ATM_INTERFACE_CFG;
        KStats.ulStructureId = ID_ATM_INTERFACE_STATS;
        if ( BcmAtm_GetInterfaceCfg(interfaceId, &Cfg)==STS_SUCCESS &&
            BcmAtm_GetInterfaceStatistics(interfaceId, &KStats, 0)==STS_SUCCESS)
        {
            atm_if_stats.rx_bytes = (unsigned long) pAtmStats->ulIfInOctets;
            atm_if_stats.tx_bytes = (unsigned long) pAtmStats->ulIfOutOctets;
            atm_if_stats.rx_errors = (unsigned long) pAtmStats->ulIfInErrors;
            atm_if_stats.tx_errors = (unsigned long) pAtmStats->ulIfOutErrors;  
        }
*/
        return &g_dsl_if_stats;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
static const struct net_device_ops dsl_netdevOps =
{
    .ndo_get_stats = dsl_get_stats,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static void dsl_setup(struct net_device *dev) {
        
    dev->type = ARPHRD_DSL;
    dev->mtu = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
    dev->netdev_ops = &dsl_netdevOps;
#else
    dev->get_stats = dsl_get_stats;
#endif
}

static int dsl_config_netdev(void) {

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    g_dslNetDev = alloc_netdev(0, DSL_IFNAME, dsl_setup);
#else
    g_dslNetDev = alloc_netdev(0, DSL_IFNAME, NET_NAME_UNKNOWN, dsl_setup);
#endif
    if ( g_dslNetDev )
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
         g_dslNetDev->netdev_ops = &dsl_netdev_ops;
#endif
         return register_netdev(g_dslNetDev);
    }
    else    
         return -ENOMEM;
}
#else
static int dsl_config_netdev(void) {
    int status = 0;
    char name[16]="";

    sprintf(name, "dsl0");
    g_dslNetDev = dev_alloc(name,  &status);
    g_dslNetDev->type = ARPHRD_DSL;
    g_dslNetDev->mtu = 0;
    g_dslNetDev->get_stats = dsl_get_stats;
    register_netdev(g_dslNetDev);    
    return status;
}
#endif

/***************************************************************************
 * Function Name: adsl_init
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 ***************************************************************************/
static int __init adsl_init( void )
{
#if defined(CONFIG_BRCM_IKOS) && defined(DSL_KTHREAD)
    OS_STATUS sts = OS_STATUS_OK;
#endif
#if defined(BOARD_H_API_VER) && BOARD_H_API_VER > 0 && !defined(CONFIG_BRCM_IKOS)
    if (kerSysGetDslPhyEnable())
         printk( "adsl: adsl_init entry\n" );
    else
         return( -1 );
#else
    printk( "adsl: adsl_init entry\n" );
#endif

    BcmXdslDrvInitEocCtrl();
    if ( register_chrdev( ADSLDRV_MAJOR, "adsl", &adsl_fops ) )
    {
         printk("adsldd: failed to create adsl\n");
         return( -1 );
    }            
    if ( dsl_config_netdev() )
    {
         printk("adsldd: failed to create %s\n", DSL_IFNAME);
         return( -1 );
    }
#ifdef CONFIG_BCM963158
    register_reboot_notifier(&BcmXdsl_RebootNotifier);
#endif
    {
#ifdef CONFIG_BRCM_IKOS
    adslCfgProfile adslCfg;
#ifdef DSL_KTHREAD
   dslThreadCreate();
#endif
    memset((void *)&adslCfg, 0, sizeof(adslCfg));
    BcmAdsl_Initialize(0, AdslConnectCb, NULL, &adslCfg);
#ifdef XDSL_DRV_STATUS_POLLING
    BcmXdslStatusPolling();
#else
    {
    dslCommandStruct	cmd;
    volatile uint	*pAdslEnum = (uint *) XDSL_ENUM_BASE;
    cmd.command = 401;	/* Initiate playback status */
    printk("*** Before initiating playback status intStatus=0x%08X\n", (uint)pAdslEnum[ADSL_INTMASK_I]);
    AdslCoreCommandHandler(&cmd);
    printk("*** After initiating playback status intStatus=0x%08X\n", (uint)pAdslEnum[ADSL_INTMASK_I]);
    }
#endif
#endif	/* CONFIG_BRCM_IKOS */
    }
    return 0;
} /* adsl_init */


/***************************************************************************
 * Function Name: adsldrv_cleanup
 * Description  : clean up function for ADSL driver
 * Returns      : None.
 ***************************************************************************/
static BCMADSL_STATUS adsldrv_cleanup( unsigned char lineId )
{
    BCMADSL_STATUS bvStatus;

#ifdef DSL_KTHREAD
    dslThreadTerminate();
#endif
#if defined(XTM_USE_DSL_MIB)
    g_pfnAdslGetObjValue = NULL;
#if defined(XTM_USE_DSL_SYSCTL)
    g_pfnAdslSetObjValue = NULL;
#endif
#if defined(XTM_USE_DSL_WAN_NOTIFY)
    g_pfnAdslWanDevState = NULL;
#endif
#endif

    bvStatus = BcmAdsl_Uninitialize(lineId);
#if defined(DYING_GASP_API) && !defined(CONFIG_BRCM_IKOS)
    kerSysDeregisterDyingGaspHandler(DSL_IFNAME);
#endif
#ifdef CONFIG_BCM963158
    unregister_reboot_notifier(&BcmXdsl_RebootNotifier);
#endif
    drvInit = 0;

    return bvStatus;
}


/***************************************************************************
 * Function Name: adsl_cleanup
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void __exit adsl_cleanup( void )
{
    printk( "adsl: adsl_cleanup entry\n" );

    adsldrv_cleanup(0);

    unregister_chrdev( ADSLDRV_MAJOR, "adsl" );
    if(g_dslNetDev) {
        unregister_netdev(g_dslNetDev);
	 free_netdev(g_dslNetDev);
	 g_dslNetDev = NULL;
    }
} /* adsl_cleanup */


/***************************************************************************
 * Function Name: adsl_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int adsl_open( struct inode *inode, struct file *filp )
{
    unsigned char lineId = MINOR(inode->i_rdev);
#ifdef SUPPORT_DSL_BONDING
    if( lineId > 1 ) {
        printk("%s: Invalid lineId(%d)!\n", __FUNCTION__, lineId);
        return -EINVAL;
    }
#else
    if( lineId > 0 ) {
        printk("%s: Invalid lineId(%d)!\n", __FUNCTION__, lineId);
        return -EINVAL;
    }
#endif
    filp->private_data = (void *)(long)lineId;
    return( 0 );
} /* adsl_open */


/***************************************************************************
 * Function Name: adsl_ioctl
 * Description  : Main entry point for an application send issue ATM API
 *                requests.
 * Returns      : 0 - success or error
 ***************************************************************************/
int adsl_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg )
{
    int ret = 0;
    unsigned int cmdnr = _IOC_NR(command);
    unsigned char lineId = MINOR(inode->i_rdev);

    ADSL_FN_IOCTL IoctlFuncs[] = {DoCheck, DoInitialize, DoUninitialize,
                DoConnectionStart, DoConnectionStop, DoGetPhyAddresses, DoSetPhyAddresses,
                DoMapATMPortIDs,DoGetConnectionInfo, DoDiagCommand, DoGetObjValue, 
                DoStartBERT, DoStopBERT, DoConfigure, DoTest, DoGetConstelPoints,
                DoGetVersion, DoSetSdramBase, DoResetStatCounters, DoSetOemParameter, 
                DoBertStartEx, DoBertStopEx , DoSetObjValue, DoCallBackDrv,DoOpenEocIf,DoCloseEocIf
#if defined(SUPPORT_HMI)
                ,DoHmiCommand
#endif
                };
#ifdef SUPPORT_DSL_BONDING
    if( lineId > 1 ) {
        printk("%s: Invalid lineId(%d)!\n", __FUNCTION__, lineId);
        return -EINVAL;
    }
#else
    if( lineId > 0 ) {
       printk("%s: Invalid lineId(%d)!\n", __FUNCTION__, lineId);
        return -EINVAL;
    }
#endif

    if( cmdnr >= 0 && cmdnr < MAX_ADSLDRV_IOCTL_COMMANDS ) {
        if (drvInit || (cmdnr <= _IOC_NR(ADSLIOCTL_INITIALIZE)) ||
            (cmdnr == _IOC_NR(ADSLIOCTL_DIAG_COMMAND)) ||
            (cmdnr == _IOC_NR(ADSLIOCTL_GET_CONNECTION_INFO))) /* allow only DoInitialize and DoGetConnectionInfo if not initialized */
            (*IoctlFuncs[cmdnr]) (lineId, arg);
        else
            ret = -EINVAL;
    }
    else
        ret = -EINVAL;

    return( ret );
} /* adsl_ioctl */


/***************************************************************************
 * Function Name: DoCheck
 * Description  : Calls BcmAdsl_Check on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoCheck( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_Check();
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoCheck */

/***************************************************************************
 * Function Name: DoInitialize
 * Description  : Calls BcmAdsl_Initialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoInitialize( unsigned char lineId, unsigned long arg )
{
    ADSLDRV_INITIALIZE      KArg;
    adslCfgProfile          adslCfg, *pAdslCfg;
    /* For now, the user callback function is not used.  Rather,
     * this module handles the connection up/down callback.  The
     * user application will need to call BcmAdsl_GetConnectionStatus
     * in order to determine the state of the connection.
     */
    pAdslCfg = NULL;
    do {
        if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
            KArg.bvStatus = BCMADSL_STATUS_ERROR;
            break;
        }
        KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
        if (is_compat_task()) {
            BCM_IOC_PTR_ZERO_EXT(KArg.pFnNotifyCb);
            BCM_IOC_PTR_ZERO_EXT(KArg.pParm);
            BCM_IOC_PTR_ZERO_EXT(KArg.pAdslCfg);
        }
#endif
        if (NULL != KArg.pAdslCfg) {
            if( copy_from_user( &adslCfg, KArg.pAdslCfg, sizeof(adslCfg)) != 0 )
                break;
            pAdslCfg = &adslCfg;
        }
    } while (0);

    KArg.bvStatus = BcmAdsl_Initialize(lineId, AdslConnectCb, NULL, pAdslCfg);
#if defined(DYING_GASP_API) && !defined(CONFIG_BRCM_IKOS)
#if defined(BOARD_H_API_VER) && BOARD_H_API_VER > 15
    kerSysRegisterDyingGaspHandlerV2(DSL_IFNAME, &BcmAdsl_DyingGaspHandler, 0);
#else
    kerSysRegisterDyingGaspHandler(DSL_IFNAME, &BcmAdsl_DyingGaspHandler, 0);
#endif
#endif
    put_user( KArg.bvStatus, &((PADSLDRV_INITIALIZE) arg)->bvStatus );

    /* This ADSL module is loaded after the ATM API module so the ATM API
     * cannot call the functions below directly.  This modules sets the
     * function address to a global variable which is used by the ATM API
     * module.
     */

#if defined(XTM_USE_DSL_MIB)
    g_pfnAdslGetObjValue = (void *) BcmAdsl_GetObjectValue;
#if defined(XTM_USE_DSL_SYSCTL)
    g_pfnAdslSetObjValue = (void *) BcmAdsl_SetObjectValue;
#endif
#if defined(XTM_USE_DSL_WAN_NOTIFY)
	g_pfnAdslWanDevState = (void *) BcmXdslWanDevNotify;
#endif
#endif

    /* Set LED into mode that is looking for carrier. */
    kerSysLedCtrl(kLedAdsl, kLedStateFail);
#if defined(SUPPORT_DSL_BONDING) && !defined(SUPPORT_2CHIP_BONDING)
    kerSysLedCtrl(kLedSecAdsl, kLedStateFail);
#endif
	drvInit = 1;
} /* DoInitialize */


/***************************************************************************
 * Function Name: DoUninitialize
 * Description  : Calls BcmAdsl_Uninitialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoUninitialize( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus;

    bvStatus = adsldrv_cleanup(lineId);
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoUninitialize */


/***************************************************************************
 * Function Name: DoConnectionStart
 * Description  : Calls BcmAdsl_ConnectionStart on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoConnectionStart( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_ConnectionStart(lineId);
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoConnectionStart */


/***************************************************************************
 * Function Name: DoConnectionStop
 * Description  : Calls BcmAdsl_ConnectionStop on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoConnectionStop( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_ConnectionStop(lineId);
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoConnectionStop */


/***************************************************************************
 * Function Name: DoGetPhyAddresses
 * Description  : Calls BcmAdsl_GetPhyAddresses on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetPhyAddresses( unsigned char lineId, unsigned long arg )
{
    ADSLDRV_PHY_ADDR KArg;

    KArg.bvStatus = BcmAdsl_GetPhyAddresses( &KArg.ChannelAddr );
    copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
} /* DoGetPhyAddresses */


/***************************************************************************
 * Function Name: DoSetPhyAddresses
 * Description  : Calls BcmAdsl_SetPhyAddresses on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetPhyAddresses( unsigned char lineId, unsigned long arg )
{
    ADSLDRV_PHY_ADDR KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bvStatus = BcmAdsl_SetPhyAddresses( &KArg.ChannelAddr );
        put_user( KArg.bvStatus, &((PADSLDRV_PHY_ADDR) arg)->bvStatus );
    }
} /* DoSetPhyAddresses */


/***************************************************************************
 * Function Name: DoMapATMPortIDs
 * Description  : Maps ATM Port IDs to DSL PHY Utopia Addresses.
 * Returns      : None.
 ***************************************************************************/
static void DoMapATMPortIDs( unsigned char lineId, unsigned long arg )
{
    ADSLDRV_MAP_ATM_PORT KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
                g_usAtmFastPortId = KArg.usAtmFastPortId;
                g_usAtmInterleavedPortId = KArg.usAtmInterleavedPortId;
            put_user( BCMADSL_STATUS_SUCCESS, &((PADSLDRV_MAP_ATM_PORT) arg)->bvStatus );
        }
} /* DoMapATMPortIDs */


/***************************************************************************
 * Function Name: DoGetConnectionInfo
 * Description  : Calls BcmAdsl_GetConnectionInfo on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/

static void DoGetConnectionInfo( unsigned char lineId, unsigned long arg )
{
    ADSLDRV_CONNECTION_INFO KArg;
	
    KArg.bvStatus = BcmAdsl_GetConnectionInfo(lineId, &KArg.ConnectionInfo );
	
    copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
} /* DoGetConnectionInfo */


/***************************************************************************
 * Function Name: DoDiagCommand
 * Description  : Calls BcmAdsl_DiagCommand on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoDiagCommand ( unsigned char lineId, unsigned long arg )
{
	ADSLDRV_DIAG        KArg;
#ifdef SUPPORT_DSL_BONDING
	static char		*pCmdData[MAX_DSL_LINE] = {NULL, NULL};
#else
	static char		*pCmdData[MAX_DSL_LINE] = {NULL};
#endif
	KArg.bvStatus = BCMADSL_STATUS_ERROR;

	do {
		if (copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0)
			break;
#if defined(CONFIG_COMPAT)
		if (is_compat_task())
			BCM_IOC_PTR_ZERO_EXT(KArg.diagMap);
#endif
		if ((LOG_CMD_CONNECT != KArg.diagCmd) && (LOG_CMD_DISCONNECT != KArg.diagCmd) && (0 != KArg.diagMap)) {
			if(BcmAdslDiagIsActive() && (0 != KArg.srvIpAddr) && ((KArg.srvIpAddr&0xFFFF) != (BcmXdslDiagGetSrvIpAddr()&0xFFFF)))
				break;
			
			if(NULL == pCmdData[lineId]) {
				if(NULL == (pCmdData[lineId] = kmalloc(LOG_MAX_DATA_SIZE, GFP_ATOMIC)))
					break;
			}
			if (KArg.logTime > LOG_MAX_DATA_SIZE)
				break;
			if (copy_from_user(pCmdData[lineId], (char*)KArg.diagMap, KArg.logTime) != 0)
				break;
			if(KArg.logTime < LOG_MAX_DATA_SIZE)
				memset((void*)(pCmdData[lineId]+KArg.logTime), 0, LOG_MAX_DATA_SIZE-KArg.logTime);
			KArg.diagMap = (unsigned long) pCmdData[lineId];
		}
		else if(LOG_CMD_DISCONNECT == KArg.diagCmd) {
#ifdef SUPPORT_DSL_BONDING
			unsigned char otherLineId = lineId ^ 1;
			if(NULL != pCmdData[otherLineId]) {
				kfree(pCmdData[otherLineId]);
				pCmdData[otherLineId] = NULL;
			}
#endif
			if(NULL != pCmdData[lineId]) {
				kfree(pCmdData[lineId]);
				pCmdData[lineId] = NULL;
			}
		}
		else if((LOG_CMD_CONNECT != KArg.diagCmd) && BcmAdslDiagIsActive() &&
			(0 != KArg.srvIpAddr) && ((KArg.srvIpAddr&0xFFFF) != (BcmXdslDiagGetSrvIpAddr()&0xFFFF)))
			break;
		
		KArg.bvStatus = BcmAdsl_DiagCommand (lineId, (PADSL_DIAG) &KArg );
	} while (0);
	put_user( KArg.bvStatus, &((PADSLDRV_DIAG) arg)->bvStatus );
} /* DoDiagCommand */

/***************************************************************************
 * Function Name: DoGetObjValue
 * Description  : Calls BcmAdsl_GetObjectValue on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetObjValue( unsigned char lineId, unsigned long arg )
{
	ADSLDRV_GET_OBJ	KArg;
	char				objId[kOidMaxObjLen];
	long				userObjLen, retObj;
	
	do {
		if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
			KArg.bvStatus = BCMADSL_STATUS_ERROR;
			break;
		}
#if defined(CONFIG_COMPAT)
		if (is_compat_task()) {
			BCM_IOC_PTR_ZERO_EXT(KArg.objId);
			BCM_IOC_PTR_ZERO_EXT(KArg.dataBuf);
			BCM_IOC_PTR_ZERO_EXT(KArg.dataBufLen);
		}
#endif
		KArg.bvStatus = BCMADSL_STATUS_ERROR;
		userObjLen = KArg.dataBufLen;

		if ((NULL != KArg.objId) && (KArg.objIdLen)) {
			if (KArg.objIdLen > kOidMaxObjLen)
				break;
			if( copy_from_user( objId, KArg.objId, KArg.objIdLen ) != 0 )
				break;
			if ( objId[0] == kOidAdslDiagdSkb ) {
				BcmAdslPendingSkbMsgRead(KArg.dataBuf, &KArg.dataBufLen);
				KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
				break;
			}
			else
				retObj = BcmAdsl_GetObjectValue (lineId, objId, KArg.objIdLen, NULL, &KArg.dataBufLen);
		}
		else
			retObj = BcmAdsl_GetObjectValue (lineId, NULL, 0, NULL, &KArg.dataBufLen);

		if ((retObj >= kAdslMibStatusLastError) && (retObj < 0))
			break;

		if( KArg.dataBufLen > userObjLen )
			KArg.dataBufLen = userObjLen;

		copy_to_user( KArg.dataBuf, (void *) retObj, KArg.dataBufLen );
		KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
	} while (0);

	if (BCMADSL_STATUS_ERROR == KArg.bvStatus)
		KArg.dataBufLen = 0;
	copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
}


/***************************************************************************
 * Function Name: DoSetObjValue
 * Description  : Calls BcmAdsl_SetObjectValue on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetObjValue( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_GET_OBJ KArg;
        char                    objId[kOidMaxObjLen];
        char                    *dataBuf = NULL;
        int                     retObj = kAdslMibStatusFailure;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
                if (is_compat_task()) {
                    BCM_IOC_PTR_ZERO_EXT(KArg.objId);
                    BCM_IOC_PTR_ZERO_EXT(KArg.dataBuf);
                    BCM_IOC_PTR_ZERO_EXT(KArg.dataBufLen);
                }
#endif
                if ((NULL != KArg.objId) && (KArg.objIdLen)) {
                        if (KArg.objIdLen > kOidMaxObjLen)
                                break;
                        if( copy_from_user( objId, KArg.objId, KArg.objIdLen ) != 0 )
                                break;
                        dataBuf = kmalloc(KArg.dataBufLen, GFP_ATOMIC);
                        if(NULL == dataBuf)
                            break;
                        else if (copy_from_user( dataBuf, KArg.dataBuf, KArg.dataBufLen ) != 0 )
                            break;
                        retObj = BcmAdsl_SetObjectValue (lineId, objId, KArg.objIdLen, dataBuf, &KArg.dataBufLen);
                }

                if ((retObj > kAdslMibStatusSuccess) && (retObj <= kAdslMibStatusLastError))
                        break;
                KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        if (BCMADSL_STATUS_ERROR == KArg.bvStatus)
                KArg.dataBufLen = 0;
        copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
        
        if(dataBuf)
            free(dataBuf);
}

/***************************************************************************
 * Function Name: DoStartBERT
 * Description  : Calls BcmAdsl_StartBERT on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoStartBERT( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_BERT    KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
        KArg.bvStatus = BcmAdsl_StartBERT (lineId, KArg.totalBits);
        put_user( KArg.bvStatus, &((PADSLDRV_BERT) arg)->bvStatus );
    }
}

/***************************************************************************
 * Function Name: DoStopBERT
 * Description  : Calls BcmAdsl_StopBERT on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoStopBERT( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_StopBERT(lineId);
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoConfigure
 * Description  : Calls BcmAdsl_Configureon behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoConfigure( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_CONFIGURE       KArg;
        adslCfgProfile          adslCfg;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
                if (is_compat_task())
                    BCM_IOC_PTR_ZERO_EXT(KArg.pAdslCfg);
#endif
                if (NULL != KArg.pAdslCfg) {
                        if( copy_from_user( &adslCfg, KArg.pAdslCfg, sizeof(adslCfg)) != 0 )
                                break;
                        KArg.bvStatus = BcmAdsl_Configure(lineId, &adslCfg);
                }
                else
                        KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        put_user( KArg.bvStatus, &((PADSLDRV_CONFIGURE) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoTest
 * Description  : Controls ADSl driver/PHY special test modes
 * Returns      : None.
 ***************************************************************************/
static void DoTest( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_TEST            KArg;
        char                            rcvToneMap[512/8];
        char                            xmtToneMap[512/8];
        int                                     nToneBytes;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
                if (is_compat_task()) {
                    BCM_IOC_PTR_ZERO_EXT(KArg.xmtToneMap);
                    BCM_IOC_PTR_ZERO_EXT(KArg.rcvToneMap);
                }
#endif
                if ((NULL != KArg.xmtToneMap) && (0 != KArg.xmtNumTones)) {
                        nToneBytes = (KArg.xmtNumTones + 7) >> 3;
                        if (nToneBytes > sizeof(xmtToneMap)) {
                                nToneBytes = sizeof(xmtToneMap);
                                KArg.xmtNumTones = nToneBytes << 3;
                        }
                        if( copy_from_user( xmtToneMap, KArg.xmtToneMap, nToneBytes) != 0 ) {
                                KArg.xmtNumTones = 0;
                                break;
                        }
                        KArg.xmtToneMap = xmtToneMap;
                }

                if ((NULL != KArg.rcvToneMap) && (0 != KArg.rcvNumTones)) {
                        nToneBytes = (KArg.rcvNumTones + 7) >> 3;
                        if (nToneBytes > sizeof(rcvToneMap)) {
                                nToneBytes = sizeof(rcvToneMap);
                                KArg.rcvNumTones = nToneBytes << 3;
                        }
                        if( copy_from_user( rcvToneMap, KArg.rcvToneMap, nToneBytes) != 0 ) {
                                KArg.rcvNumTones = 0;
                                break;
                        }
                        KArg.rcvToneMap = rcvToneMap;
                }
                
                if (ADSL_TEST_SELECT_TONES != KArg.testCmd)
                        KArg.bvStatus = BcmAdsl_SetTestMode(lineId, KArg.testCmd);
                else
                        KArg.bvStatus = BcmAdsl_SelectTones(
                                lineId,
                                KArg.xmtStartTone, KArg.xmtNumTones,
                                KArg.rcvStartTone, KArg.rcvNumTones,
                                xmtToneMap, rcvToneMap);
        } while (0);

        put_user( KArg.bvStatus, &((PADSLDRV_TEST) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoGetConstelPoints
 * Description  : Calls BcmAdsl_GetObjectValue on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetConstelPoints( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_GET_CONSTEL_POINTS      KArg;
        ADSL_CONSTELLATION_POINT        pointBuf[64];
        int                                                     numPoints;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
                if (is_compat_task())
                    BCM_IOC_PTR_ZERO_EXT(KArg.pointBuf);
#endif
                numPoints = (KArg.numPoints > (sizeof(pointBuf)/sizeof(pointBuf[0])) ? sizeof(pointBuf)/sizeof(pointBuf[0]) : KArg.numPoints);
                numPoints = BcmAdsl_GetConstellationPoints (KArg.toneId, pointBuf, KArg.numPoints);
                if (numPoints > 0)
                        copy_to_user( KArg.pointBuf, (void *) pointBuf, numPoints * sizeof(ADSL_CONSTELLATION_POINT) );
                KArg.numPoints = numPoints;
                KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        if (BCMADSL_STATUS_ERROR == KArg.bvStatus)
                KArg.numPoints = 0;
        copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
}

static void DoGetVersion( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_GET_VERSION             KArg;
        adslVersionInfo                 adslVer;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
#if defined(CONFIG_COMPAT)
                if (is_compat_task())
                    BCM_IOC_PTR_ZERO_EXT(KArg.pAdslVer);
#endif
                BcmAdsl_GetVersion(&adslVer);
                copy_to_user( KArg.pAdslVer, (void *) &adslVer, sizeof(adslVersionInfo) );
                KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        put_user( KArg.bvStatus , &((PADSLDRV_GET_VERSION) arg)->bvStatus );
}

static void DoSetSdramBase( unsigned char lineId, unsigned long arg )
{
    ADSLDRV_SET_SDRAM_BASE          KArg;
    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
#if defined(CONFIG_COMPAT)
        if (is_compat_task())
            BCM_IOC_PTR_ZERO_EXT(KArg.sdramBaseAddr);
#endif
        KArg.bvStatus = BcmAdsl_SetSDRAMBaseAddr((void *)KArg.sdramBaseAddr);
        put_user( KArg.bvStatus, &((PADSLDRV_SET_SDRAM_BASE) arg)->bvStatus );
    }
}

static void DoResetStatCounters( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_ResetStatCounters(lineId);
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
}

static void DoSetOemParameter( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_SET_OEM_PARAM   KArg;
        char                                    dataBuf[256];

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
                if (is_compat_task())
                    BCM_IOC_PTR_ZERO_EXT(KArg.buf);
#endif
                if ((NULL != KArg.buf) && (KArg.len > 0)) {
                        if (KArg.len > sizeof(dataBuf))
                                KArg.len = sizeof(dataBuf);
                        if( copy_from_user( dataBuf, (void *)KArg.buf, KArg.len) != 0 )
                                break;
                        KArg.bvStatus = BcmAdsl_SetOemParameter (KArg.paramId, dataBuf, KArg.len);
                }
                else
                        KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        put_user( KArg.bvStatus, &((PADSLDRV_SET_OEM_PARAM) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoBertStartEx
 * Description  : Calls BcmAdsl_BertStartEx on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoBertStartEx( unsigned char lineId, unsigned long arg )
{
        ADSLDRV_BERT_EX         KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
        KArg.bvStatus = BcmAdsl_BertStartEx (lineId, KArg.totalSec);
        put_user( KArg.bvStatus, &((PADSLDRV_BERT_EX) arg)->bvStatus );
    }
}

/***************************************************************************
 * Function Name: DoBertStopEx
 * Description  : Calls BcmAdsl_BertStopEx on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoBertStopEx( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_BertStopEx(lineId);
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoCallBackDrv
 * Description  : Calls BcmXdsl_CallBackDrv on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoCallBackDrv ( unsigned char lineId, unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmXdsl_CallBackDrv(lineId);
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoOpenEocIf
 * Description  : Calls BcmXdslDrvOpenEocIf on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoOpenEocIf( unsigned char lineId, unsigned long arg )
{
	XDSLDRV_EOC_IFACE	KArg;
	char	ifName[BCM_XDSL_MAX_EOC_IFNAME_LEN];

	do {
		if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
			KArg.bvStatus = BCMADSL_STATUS_ERROR;
			break;
		}

		KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
		if (is_compat_task())
			BCM_IOC_PTR_ZERO_EXT(KArg.ifName);
#endif
		if ((NULL != KArg.ifName) && (KArg.ifNameLen)) {
			if (KArg.ifNameLen >= BCM_XDSL_MAX_EOC_IFNAME_LEN)
				break;
			if( copy_from_user( ifName, KArg.ifName, KArg.ifNameLen ) != 0 )
				break;
			ifName[KArg.ifNameLen] = 0;
			KArg.bvStatus = BcmXdslDrvOpenEocIf(lineId, KArg.eocMsgType, ifName);
		}
	} while (0);
	put_user( KArg.bvStatus, &((PXDSLDRV_EOC_IFACE) arg)->bvStatus );
}
/***************************************************************************
 * Function Name: DoCloseEocIf
 * Description  : Calls BcmXdslDrvCloseEocIf on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoCloseEocIf( unsigned char lineId, unsigned long arg )
{
	XDSLDRV_EOC_IFACE	KArg;
	char	ifName[BCM_XDSL_MAX_EOC_IFNAME_LEN];

	do {
		if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
			KArg.bvStatus = BCMADSL_STATUS_ERROR;
			break;
		}

		KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
		if (is_compat_task())
			BCM_IOC_PTR_ZERO_EXT(KArg.ifName);
#endif
		if ((NULL != KArg.ifName) && (KArg.ifNameLen)) {
			if (KArg.ifNameLen >= BCM_XDSL_MAX_EOC_IFNAME_LEN)
				break;
			if( copy_from_user( ifName, KArg.ifName, KArg.ifNameLen ) != 0 )
				break;
			ifName[KArg.ifNameLen] = 0;
			KArg.bvStatus = BcmXdslDrvCloseEocIf(lineId, KArg.eocMsgType, ifName);
		}
	} while (0);
	put_user( KArg.bvStatus, &((PXDSLDRV_EOC_IFACE) arg)->bvStatus );
}

#if defined(SUPPORT_HMI)

#include "hmimsg/HpiMessage.h"

extern void XdslDrvProcessHmiMsg(void *hmiMsgp, uint16 size);

#define MAX_HMI_PACKET_SIZE 1224
XDSLDRV_HMI_MSG KArg;
void XdslDrvHpiPost(void *replyPtr, uint16 usedSize, int result);

/***************************************************************************
 * Function Name: DoHmiCommand
 * Description  : Calls BcmXdslDrv... on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoHmiCommand( unsigned char lineId, unsigned long arg )
{
	unsigned char	hmiMsg[MAX_HMI_PACKET_SIZE];

	do {
		if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
			KArg.bvStatus = BCMADSL_STATUS_ERROR;
			break;
		}

		KArg.bvStatus = BCMADSL_STATUS_ERROR;
#if defined(CONFIG_COMPAT)
		if (is_compat_task()) {
			BCM_IOC_PTR_ZERO_EXT(KArg.header);
			BCM_IOC_PTR_ZERO_EXT(KArg.payload);
			BCM_IOC_PTR_ZERO_EXT(KArg.replyMessage);
		}
#endif
#if 0
		printk("%s: header %p headerSize %d payload %p payloadSize %d replyMessage %p replyMaxMessageSize %d\n",
			__FUNCTION__, KArg.header, KArg.headerSize,
			KArg.payload, KArg.payloadSize,
			KArg.replyMessage, KArg.replyMaxMessageSize);
#endif
		if ((NULL != KArg.header) && (NULL != KArg.replyMessage) &&
			(KArg.headerSize) && (KArg.replyMaxMessageSize)) {
			if ((KArg.headerSize + KArg.payloadSize) > MAX_HMI_PACKET_SIZE)
				break;
			if( copy_from_user( hmiMsg, KArg.header, KArg.headerSize ) != 0 )
				break;
			if(KArg.payloadSize) {
				if(NULL == KArg.payload)
					break;
				else if( copy_from_user( hmiMsg+KArg.headerSize, KArg.payload, KArg.payloadSize ) != 0 )
					break;
			}
			XdslDrvProcessHmiMsg(hmiMsg, KArg.payloadSize);
			KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
		}
	} while (0);
	put_user( KArg.bvStatus, &((PXDSLDRV_HMI_MSG) arg)->bvStatus );
}

void XdslDrvHpiPost(void *replyPtr, uint16 usedSize, int result)
{
  HpiMsgHeader *ptr = replyPtr;
  
  ptr --;

  /* check msg size */
  if (usedSize > (MAX_HMI_PACKET_SIZE - sizeof(HpiMsgHeader)))
  {
    printk("%s: application post a msg where used size(%d) is too big",__FUNCTION__, usedSize);
    usedSize = 0;
    result = HPI_INVALID_REQUEST_DATA;
  }

  ptr->len = usedSize;
  ptr->buildId = 0;
  ptr->result = (int8)result;
  ptr->errorCode = (int16)(result>>8);
  copy_to_user( KArg.replyMessage, (void *)ptr, sizeof(HpiMsgHeader)+usedSize );
}

#endif

#if defined(PHY_LOOPBACK) || defined(SUPPORT_TEQ_FAKE_LINKUP)

void AdslDrvXtmLinkDown(unsigned char lineId)
{
	ulong	mibLen;
	adslMibInfo	*pMib;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	int	linkState = (int)BCM_ADSL_LINK_DOWN;
#endif

	printk("AdslDrvXtmLinkDown: lineId %d\n", lineId);

	mibLen = sizeof(adslMibInfo);
	pMib = (void *) AdslCoreGetObjectValue (lineId, NULL, 0, NULL, &mibLen);

	pMib->adslTrainingState = kAdslTrainingIdle;

	{
	XTM_INTERFACE_LINK_INFO	linkInfo;
	uint					ahifChanId = 0;

	linkInfo.ulLinkState = LINK_DOWN;
	linkInfo.ulLinkUsRate =	0;
	linkInfo.ulLinkDsRate =	0;
	linkInfo.ulLinkTrafficType = TRAFFIC_TYPE_ATM;
	BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID(ahifChanId+lineId), &linkInfo);
	}

#ifdef DYING_GASP_API
	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
	kerSysWakeupMonitorTask();
#endif

#endif
}

#if defined(SUPPORT_TEQ_FAKE_LINKUP)
int AdslDrvIsFakeLinkUp(unsigned char lineId)
{
	ulong	mibLen;
	adslMibInfo	*pMib;
	int	res = 0;
	
	mibLen = sizeof(adslMibInfo);
	pMib = (void *) AdslCoreGetObjectValue (lineId, NULL, 0, NULL, &mibLen);
	
	if((kAdslTrainingConnected == pMib->adslTrainingState) &&
#ifdef CONFIG_VDSL_SUPPORTED
		(PHY_LOOPBACK_DS_RATE == pMib->vdslInfo[0].rcvRate) &&
		(PHY_LOOPBACK_US_RATE == pMib->vdslInfo[0].xmtRate) )
#else
	(PHY_LOOPBACK_DS_RATE == pMib->adsl2Info2lp[0].rcvRate) &&
	(PHY_LOOPBACK_US_RATE == pMib->adsl2Info2lp[0].xmtRate) )
#endif
		res = 1;
	return res;
}
#endif

void AdslDrvXtmLinkUp(unsigned char lineId, unsigned char tpsTc)
{
	ulong	mibLen;
	adslMibInfo	*pMib;
	XTM_INTERFACE_LINK_INFO	linkInfo;
	uint		ahifChanId = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	int linkState = (int)BCM_ADSL_LINK_UP;
#endif

	printk("AdslDrvXtmLinkUp: lineId %d, tpsTc %d\n", lineId, tpsTc);
	
	mibLen = sizeof(adslMibInfo);
	pMib = (void *) AdslCoreGetObjectValue (lineId, NULL, 0, NULL, &mibLen);

	pMib->adsl2Info2lp[0].rcvChanInfo.connectionType = tpsTc;
	pMib->adsl2Info2lp[0].xmtChanInfo.connectionType = tpsTc;

	pMib->adslTrainingState = kAdslTrainingConnected;
	pMib->xdslInfo.dirInfo[0].lpInfo[0].tmType[0] = tpsTc;
	pMib->xdslInfo.dirInfo[1].lpInfo[0].tmType[0] = tpsTc;
#ifdef CONFIG_VDSL_SUPPORTED
	pMib->adslConnection.modType = kVdslModVdsl2;
	pMib->vdslInfo[0].vdsl2Profile = kVdslProfile17a;
	pMib->vdslInfo[0].rcvRate = PHY_LOOPBACK_DS_RATE;
	pMib->vdslInfo[0].xmtRate = PHY_LOOPBACK_US_RATE;
	pMib->vdslInfo[1].rcvRate = PHY_LOOPBACK_DS_RATE;
	pMib->vdslInfo[1].xmtRate = PHY_LOOPBACK_US_RATE;

    pMib->vdslInfo[0].xmt2Info.tmType[0] = tpsTc ;
    pMib->vdslInfo[1].xmt2Info.tmType[0] = tpsTc ;
#else
	pMib->adslConnection.modType = kAdslModAdsl2p;
	pMib->adsl2Info2lp[0].rcvRate = PHY_LOOPBACK_DS_RATE;
	pMib->adsl2Info2lp[0].xmtRate = PHY_LOOPBACK_US_RATE;
	pMib->adsl2Info2lp[1].rcvRate = PHY_LOOPBACK_DS_RATE;
	pMib->adsl2Info2lp[1].xmtRate = PHY_LOOPBACK_US_RATE;
#endif

#ifdef DYING_GASP_API
	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
	kerSysWakeupMonitorTask();
#endif

#endif

	linkInfo.ulLinkState = LINK_UP;
	linkInfo.ulLinkUsRate =	1000 * PHY_LOOPBACK_US_RATE;
	linkInfo.ulLinkDsRate =	1000 * PHY_LOOPBACK_DS_RATE;
	linkInfo.ulLinkTrafficType = ((kXdslDataPtm == tpsTc ) ? TRAFFIC_TYPE_PTM :
							(kXdslDataAtm == tpsTc )? TRAFFIC_TYPE_ATM:
							( kXdslDataNitro == tpsTc) ? TRAFFIC_TYPE_ATM:
							(kXdslDataRaw == tpsTc) ? TRAFFIC_TYPE_PTM_RAW:TRAFFIC_TYPE_NOT_CONNECTED);
#if defined(SUPPORT_DSL_BONDING)
	if (kXdslDataPtm == tpsTc)
	   linkInfo.ulLinkTrafficType = TRAFFIC_TYPE_PTM_BONDED;
#endif
	BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID(ahifChanId+lineId), &linkInfo);
	XdslCoreSetAhifState(lineId, 1, 1);
//	BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID(ahifChanId+1), &linkInfo);
}
#endif /* PHY_LOOPBACK */

#if defined(SUPPORT_2CHIP_BONDING)
extern Bool BcmXdslCoreExtBondEnable(unsigned char lineId);
#endif

#ifdef SUPPORT_DSL_BONDING
extern Bool BcmXdslCorePtmBondingEnable(unsigned char lineId);
extern Bool BcmXdslCoreAtmBondingEnable(unsigned char lineId);
#endif

#if defined(SUPPORT_VECTORING)
extern void BcmXdslDiscardWanDev(unsigned char lineId, unsigned char bDevDown);
#endif
#ifdef PHY_BLOCK_TEST
extern int BcmCoreDiagTeqLogOrPlaybackActive(void);
#endif

#ifdef ADSLDRV_SILENT_MODE
int gXtmNotify = 1;
#endif

/***************************************************************************
 * Function Name: AdslConnectCb
 * Description  : Callback function that is called when by the ADSL driver
 *                when there is a change in status.
 * Returns      : None.
 ***************************************************************************/
static void AdslConnectCb(unsigned char lineId, ADSL_LINK_STATE dslLinkState, ADSL_LINK_STATE dslPrevLinkState, void *pParm )
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
	XTM_INTERFACE_LINK_INFO	linkInfo;
#else
	ATM_INTERFACE_LINK_INFO	linkInfo;
#endif
	adslMibInfo		*adslMib = NULL;
	long				size = sizeof(adslMibInfo);
	uint			ahifChanId;
	int				ledType ;
	int				connectionType, i;
	xdslFramingInfo *pXdslFramingParam;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	int				linkState = (int)dslLinkState;
#endif
	
	adslMib = (void *) BcmAdsl_GetObjectValue (lineId, NULL, 0, NULL, &size);
	if( NULL == adslMib) {
		BcmAdslCoreDiagWriteStatusString(lineId, "AdslConnectCb: BcmAdsl_GetObjectValue() failed!!!\n" );
		return;
	}

#if defined(SUPPORT_DSL_BONDING) && !defined(SUPPORT_2CHIP_BONDING)
#if defined(SUPPORT_MULTI_PHY) && defined(CONFIG_BCM963268)
	if (!ADSL_PHY_SUPPORT(kAdslPhyBonding)) {
		uint	afeIds[2];
		uint	ledOther;

		BcmXdslCoreGetCurrentMedia(afeIds);
		afeIds[0] >>= AFE_CHIP_SHIFT;
		if ((AFE_CHIP_6306 == afeIds[0]) || (AFE_CHIP_CH1 == afeIds[0])) {
			ledType  = kLedSecAdsl;
			ledOther = kLedAdsl;
		}
		else {
			ledType  = kLedAdsl;
			ledOther = kLedSecAdsl;
		}
		kerSysLedCtrl(ledOther, kLedStateOff);
	}
	else
#endif
		ledType = (lineId == 0) ? kLedAdsl : kLedSecAdsl ;
#else
	ledType = kLedAdsl;
#endif
	linkInfo.ulLinkState = LINK_DOWN;

	switch (dslLinkState) {
		case ADSL_LINK_UP:
			linkInfo.ulLinkState = LINK_UP;
			kerSysLedCtrl(ledType, kLedStateOn);
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
			snmp_adsl_eoc_event(lineId);
#endif
			break;
		case ADSL_LINK_DOWN:
		case BCM_ADSL_ATM_IDLE:
			if (dslLinkState == ADSL_LINK_DOWN)
				kerSysLedCtrl(ledType, BcmXdslCoreGetLineActive(lineId) ? kLedStateFail : kLedStateOff);
			break;
		case BCM_ADSL_TRAINING_G994:
				kerSysLedCtrl(ledType, kLedStateFail);
			break;
		case BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS:
		case BCM_ADSL_TRAINING_G992_STARTED:
		case BCM_ADSL_TRAINING_G993_CHANNEL_ANALYSIS:
		case BCM_ADSL_TRAINING_G993_STARTED:
#if defined(LINK_TRAINING1) && defined(SUPPORT_DSL_BONDING)
			memset((void *)&linkInfo, 0, sizeof(linkInfo));
			if((BCM_ADSL_TRAINING_G992_STARTED==dslLinkState) || (BCM_ADSL_TRAINING_G993_STARTED==dslLinkState))
				linkInfo.ulLinkState = LINK_TRAINING1;
			else
				linkInfo.ulLinkState = LINK_TRAINING2;
			pXdslFramingParam = &adslMib->xdslInfo.dirInfo[TX_DIRECTION].lpInfo[0];
#ifdef ADSLDRV_SILENT_MODE
			if (gXtmNotify)
#endif
			//BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID(pXdslFramingParam->ahifChanId[0]), &linkInfo);
			BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID(lineId), &linkInfo);
#endif
			kerSysLedCtrl(ledType, kLedStateSlowBlinkContinues);
			break;
#ifdef CONFIG_BCM_DSL_GFAST
		case BCM_ADSL_TRAINING_GFAST_STARTED:
#endif
		case BCM_ADSL_TRAINING_G992_EXCHANGE:
		case BCM_ADSL_TRAINING_G993_EXCHANGE:
			kerSysLedCtrl(ledType, kLedStateFastBlinkContinues);
			break;
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
		case BCM_ADSL_G997_FRAME_RECEIVED:
#if defined(NOTIFY_INTERMEDIATE_LINKSTATUS_CHANGE) && defined(DYING_GASP_API)	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
			kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
			kerSysWakeupMonitorTask();
#endif
#endif
			return;
		case BCM_ADSL_G997_FRAME_SENT:
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
			printk("BCM_ADSL_G997_FRAME_SENT \n");
#endif
#if defined(NOTIFY_INTERMEDIATE_LINKSTATUS_CHANGE) && defined(DYING_GASP_API)	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
			kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
			kerSysWakeupMonitorTask();
#endif
#endif
			return;
#endif /* defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO) */
		default:
#if defined(NOTIFY_INTERMEDIATE_LINKSTATUS_CHANGE) && defined(DYING_GASP_API)	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef ADSLDRV_SILENT_MODE
			if (gXtmNotify)
#endif
			kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
			kerSysWakeupMonitorTask();
#endif
#endif
			return;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
	/* record time stamp of link state change (in 1/100ths of a second unit per MIB spec.) */
	if (g_dslNetDev != NULL) 
		g_dslNetDev->trans_start = jiffies * 100 / HZ;
#endif
	
	if( ((ADSL_LINK_DOWN == dslLinkState) && (ADSL_LINK_UP != dslPrevLinkState)) ||
		((ADSL_LINK_UP != dslLinkState) && (ADSL_LINK_DOWN != dslLinkState)) ) {
#if defined(NOTIFY_INTERMEDIATE_LINKSTATUS_CHANGE) && defined(DYING_GASP_API)	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef ADSLDRV_SILENT_MODE
		if (gXtmNotify)
#endif
		kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
		kerSysWakeupMonitorTask();
#endif
#endif /* DYING_GASP_API */
		return;
	}

	for(i=0; i<MAX_LP_NUM; i++) {
		if( (0==i) || (adslMib->lp2Active || adslMib->lp2TxActive) ) {
#if defined(SUPPORT_DSL_BONDING) || defined(SUPPORT_2CHIP_BONDING)
#if defined(SUPPORT_2CHIP_BONDING)
			Bool bondingStat = (BcmXdslCoreExtBondEnable(lineId) && ((kDslBondingPTM==adslMib->xdslStat[0].bondingStat.status) || (kDslBondingATM==adslMib->xdslStat[0].bondingStat.status)));
#else
			Bool bondingStat = ((BcmXdslCorePtmBondingEnable(lineId) && (kDslBondingPTM==adslMib->xdslStat[0].bondingStat.status)) ||
				(BcmXdslCoreAtmBondingEnable(lineId) && (kDslBondingATM==adslMib->xdslStat[0].bondingStat.status)));
#endif
#else
			Bool bondingStat = AC_FALSE;
#endif
			pXdslFramingParam = &adslMib->xdslInfo.dirInfo[TX_DIRECTION].lpInfo[i];
			ahifChanId = pXdslFramingParam->ahifChanId[0];
			connectionType = pXdslFramingParam->tmType[0];
			linkInfo.ulLinkUsRate = 1000 * pXdslFramingParam->dataRate;
			linkInfo.ulLinkDsRate = 1000 * adslMib->xdslInfo.dirInfo[RX_DIRECTION].lpInfo[i].dataRate;
			
			if((0 == linkInfo.ulLinkUsRate) && (0 == linkInfo.ulLinkDsRate))
				continue;	/* Skip notifying event for non-data path */
			
			linkInfo.ulLinkTrafficType = ((kXdslDataPtm == connectionType ) ? (bondingStat? TRAFFIC_TYPE_PTM_BONDED: TRAFFIC_TYPE_PTM) :
								(kXdslDataAtm == connectionType )? (bondingStat? TRAFFIC_TYPE_ATM_BONDED: TRAFFIC_TYPE_ATM):
								( kXdslDataNitro == connectionType) ? TRAFFIC_TYPE_ATM:
								(kXdslDataRaw == connectionType) ? TRAFFIC_TYPE_PTM_RAW:TRAFFIC_TYPE_NOT_CONNECTED);
			
			if(ADSL_LINK_UP == dslLinkState)
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "Line: %d Path: %d  Link %s UsRate %u DsRate %u Connection type %s ahifChanId %d\n",
					lineId, i, (dslPrevLinkState != dslLinkState) ? "Up": "Rate Change",
					linkInfo.ulLinkUsRate, linkInfo.ulLinkDsRate,
					(TRAFFIC_TYPE_PTM == linkInfo.ulLinkTrafficType) ? "PTM":
					(TRAFFIC_TYPE_PTM_BONDED == linkInfo.ulLinkTrafficType) ? "PTM_BONDED":
					(TRAFFIC_TYPE_ATM == linkInfo.ulLinkTrafficType) ? "ATM":
					(TRAFFIC_TYPE_ATM_BONDED == linkInfo.ulLinkTrafficType) ? "ATM_BONDED":
					(TRAFFIC_TYPE_PTM_RAW == linkInfo.ulLinkTrafficType) ? "RAW ETHERNET":"Not connected", ahifChanId);
			else {
				BcmAdslCoreDiagWriteStatusString(lineId, "Line: %d Path: %d  Link Down\n", lineId, i);
			}
#ifdef PHY_BLOCK_TEST
			/* Skip notifying the XTM driver to prevent unexpected Tx data */
			if(BcmCoreDiagTeqLogOrPlaybackActive()) {
				BcmAdslCoreDiagWriteStatusString(lineId, "Line: %d Path: %d  in TEQ data logging mode. Skip notifying XTM driver\n", lineId, i);
#if defined(NOTIFY_INTERMEDIATE_LINKSTATUS_CHANGE) && defined(DYING_GASP_API)	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef ADSLDRV_SILENT_MODE
				if (gXtmNotify)
#endif
				kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
				kerSysWakeupMonitorTask();
#endif
#endif
				return;
			}
#endif
#ifndef PHY_LOOPBACK
#if !defined(XTM_SUPPORT_DSL_SRA) || defined(NO_RATE_CHANGE_SUPPORT_IN_XTM_API)
			if((ADSL_LINK_UP == dslLinkState) && (dslPrevLinkState == dslLinkState)) {
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "Line: %d Path: %d  Link Rate Change.  Skip reporting to XTM driver\n", lineId, i);
#if defined(NOTIFY_INTERMEDIATE_LINKSTATUS_CHANGE) && defined(DYING_GASP_API)	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef ADSLDRV_SILENT_MODE
				if (gXtmNotify)
#endif
				kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
				kerSysWakeupMonitorTask();
#endif
#endif
				return;
			}
#endif
#ifdef ADSLDRV_SILENT_MODE
			if (gXtmNotify)
#endif
			BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID(ahifChanId), &linkInfo);
			XdslCoreSetAhifState(lineId, 1, LINK_DOWN == linkInfo.ulLinkState ? 0 : 1);
#ifdef SUPPORT_MULTI_PHY
			if(ADSL_LINK_UP == dslLinkState)
				BcmXdslCoreMediaSearchSM(MEDIASEARCH_LINKUP_E, 0);
			else if(ADSL_LINK_DOWN == dslLinkState)
				BcmXdslCoreMediaSearchSM(MEDIASEARCH_LINKDOWN_E, 0);
#endif
#if defined(SUPPORT_VECTORING)
			if(ADSL_LINK_DOWN == dslLinkState)
				BcmXdslDiscardWanDev(lineId, 0);
#endif

#endif
		}
		if(XdslMibIsGinpActive(XdslCoreGetDslVars(lineId), TX_DIRECTION) ||
			XdslMibIsGinpActive(XdslCoreGetDslVars(lineId), RX_DIRECTION))
			break;
	}	

#ifdef DYING_GASP_API	/* Send Msg to the user mode application that monitors link status. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef ADSLDRV_SILENT_MODE
	if (gXtmNotify)
#endif
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,(char *)&linkState,sizeof(int));
#else
	kerSysWakeupMonitorTask();
#endif
#endif
} /* AdslConnectCb */

#if defined(XTM_USE_DSL_WAN_NOTIFY)
extern void BcmXdslWanDevIsUp(void);

static void BcmXdslWanDevNotify(unsigned char bDevUp)  /* 1/0 - up/down */
{
#if defined(SUPPORT_VECTORING)
	BcmCoreDpcSyncEnter(SYNC_RX);	// Avoid BcmXdslSendErrorSamples() from sending out Error samples in the middle of bringing down the WAN device
	if(!bDevUp)
		BcmXdslDiscardWanDev(0, 1);
	else
		BcmXdslWanDevIsUp();
	BcmCoreDpcSyncExit(SYNC_RX);
#endif
}
#endif

#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
/* routine copied from tiny bridge written by lt */
void snmp_adsl_eoc_event(unsigned char lineId)
{
	adslMibInfo *adslMib;
	long size = sizeof(adslMibInfo);

	adslMib = (void *) BcmAdsl_GetObjectValue (lineId, NULL, 0, NULL, &size);

	if( NULL != adslMib ) {
		if( (kAdslModAdsl2p == adslMib->adslConnection.modType) ||
			(kAdslModAdsl2  == adslMib->adslConnection.modType) ||
			(kVdslModVdsl2  == adslMib->adslConnection.modType) ) {
			if(kVdslModVdsl2  == adslMib->adslConnection.modType)
				BcmAdslCoreDiagWriteStatusString(lineId, "Line %d: VDSL2 connection\n", lineId);
			else
				BcmAdslCoreDiagWriteStatusString(lineId, "Line %d: ADSL2/ADSL2+ connection\n", lineId);
			g_eoc_hdr_offset = ADSL_2P_HDR_OFFSET;
			g_eoc_hdr_len    = ADSL_2P_EOC_HDR_LEN; 
		}
		else {
			BcmAdslCoreDiagWriteStatusString(lineId, "Line %d: ADSL1 connection\n", lineId);
			g_eoc_hdr_offset = ADSL_HDR_OFFSET;
			g_eoc_hdr_len    = ADSL_EOC_HDR_LEN;
		}
	}
}

/***************************************************************************
 * Function Name: adsl_read
 * The EOC read code (slightly modified) is copied from cfeBridge written by lat
 ***************************************************************************/
static ssize_t adsl_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos)
{
	int ret = 0, len, request_length=0;
	char *p;
	unsigned char adsl_eoc_hdr[] = ADSL_EOC_HDR;
	unsigned char adsl_eoc_enable[] = ADSL_EOC_ENABLE;
	char inputBuffer[520];
	unsigned char lineId = (unsigned char)(unsigned long)file->private_data;
	
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
	printk("adsl_read entry\n");
#endif

	/* Receive SNMP request. */
	p = BcmAdsl_G997FrameGet(lineId, BCM_XDSL_CLEAR_EOC_MSG, &len );
	if( p && len > 0 )
	{
		if( memcmp(p, adsl_eoc_enable, sizeof(adsl_eoc_enable)) == 0 ) {
			/* return now and let the upper layer know that link up is received */
			/* don't clear the FRAME_RCVD bit, just in case there are things to read */
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
			printk("*** Got message up on ADSL EOC connection, len %d ***\n",len);
			dumpaddr(p,len);
#endif
			BcmAdsl_G997FrameFinished(lineId, BCM_XDSL_CLEAR_EOC_MSG);
			return len;
		}

		if (memcmp(p, adsl_eoc_hdr+g_eoc_hdr_offset, g_eoc_hdr_len) == 0) {
			  p += g_eoc_hdr_len;
			  len -= g_eoc_hdr_len;
		}
		else {
			  BcmAdslCoreDiagWriteStatusString(lineId, "something is wrong, len %d, no adsl_eoc_hdr found\n",len);
			  dumpaddr(p,len);
		}

		do
		{
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
			printk("***count %d, processing rx from EOC link, len %d/requestLen %d ***\n",
			count,len,request_length);
			dumpaddr(p,len);
#endif
			if( len + request_length < count ) {
				memcpy(inputBuffer + request_length, p, len);
				request_length += len;
			}
			p = BcmAdsl_G997FrameGetNext(lineId, BCM_XDSL_CLEAR_EOC_MSG, &len );
		  } while( p && len > 0 );

		BcmAdsl_G997FrameFinished(lineId, BCM_XDSL_CLEAR_EOC_MSG);
		if (!(copy_to_user(buf, inputBuffer, request_length)))
			ret = request_length;

#ifdef BUILD_SNMP_TRANSPORT_DEBUG
		printk("adsl_read(): end, request_length %d\n", request_length);
		printk("inputBuffer:\n");
		dumpaddr(inputBuffer,request_length);
#endif
	} /* (p  & len > 0) */
	
	return( ret );
} /* adsl_read */

/***************************************************************************
 * Function Name: adsl_write
 * The EOC write code is copied from cfeBridge written by lat
 ***************************************************************************/
static ssize_t adsl_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos)
{
	unsigned char adsl_eoc_hdr[] = ADSL_EOC_HDR;
	int ret = 0, len;
	char *pBuf;
	unsigned char lineId = (unsigned char)(unsigned long)file->private_data;
	
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
	printk("adsl_write(entry): count %d\n",count);
#endif

	/* Need to include adsl_eoc_hdr */
	len = count + g_eoc_hdr_len;

	if( (pBuf = calloc(len, 1)) == NULL ) {
		BcmAdslCoreDiagWriteStatusString(lineId, "ADSL send failed, out of memory");
		return ret;
	}
	
	memcpy(pBuf, adsl_eoc_hdr+g_eoc_hdr_offset, g_eoc_hdr_len);

	if (copy_from_user((pBuf+g_eoc_hdr_len), buf, count) == 0)	{
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
		printk("adsl_write ready to send data over EOC, len %d\n", len);
		dumpaddr(pBuf, len);
#endif
		
		if( BCMADSL_STATUS_SUCCESS == BcmAdsl_G997SendData(lineId, BCM_XDSL_CLEAR_EOC_MSG, pBuf, len) ) {
			ret = len;
		}
		else {
			free(pBuf);
			BcmAdslCoreDiagWriteStatusString(lineId, "ADSL send failed, len %d",   len);
		}
	}
	else {
		free(pBuf);
		BcmAdslCoreDiagWriteStatusString(lineId, "ADSL send copy_from_user failed, len %d",   len);
	}		
  
	return ret;
	
} 

static unsigned int adsl_poll(struct file *file, poll_table *wait)
{
	return AdslCoreG997FrameReceived((unsigned char)(unsigned long)file->private_data, BCM_XDSL_CLEAR_EOC_MSG);
}
#endif /* defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO) */


//#define XDSL_EOC_DBG
typedef struct {
	char ifName[BCM_XDSL_MAX_EOC_IFNAME_LEN];
	int eocMsgType;
	unsigned char lineId;
	Bool fileOpened;
	wait_queue_head_t waitQue;
	struct proc_dir_entry *pProcEntry;
	struct file_operations *pFileOps;
} EocIfCtrl;

EocIfCtrl	nsfEocCtrl[2];
EocIfCtrl	dataGramEocCtrl[2];
EocIfCtrl	clearEocCtrl[2];
EocIfCtrl	skbEocCtrl[2];

static Bool BcmXdslEocXmtReady(EocIfCtrl *pEocIfCtrl)
{
	return(AdslCoreLinkState(pEocIfCtrl->lineId) && XdslCoreIsXdsl2Mod(pEocIfCtrl->lineId));
}

static EocIfCtrl *BcmXdslGetEocIfCtrl(unsigned char lineId, int eocMsgType)
{
	EocIfCtrl *pEocIfCtrl = NULL;

	switch(eocMsgType) {
		case BCM_XDSL_NSF_EOC_MSG:
			pEocIfCtrl = &nsfEocCtrl[lineId];
			break;
		case BCM_XDSL_DATAGRAM_EOC_MSG:
			pEocIfCtrl = &dataGramEocCtrl[lineId];
			break;
		case BCM_XDSL_CLEAR_EOC_MSG:
			pEocIfCtrl = &clearEocCtrl[lineId];
			break;
		case BCM_XDSL_SKB_EOC_MSG:
			pEocIfCtrl = &skbEocCtrl[lineId];
			break;
		default:
			break;
	}
	
	return pEocIfCtrl;
}

static int BcmXdsNsfEocMsgOpen0(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &nsfEocCtrl[0];
	pEocIfCtrl->lineId = 0;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_NSF_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdsNsfEocMsgOpen1(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &nsfEocCtrl[1];
	pEocIfCtrl->lineId = 1;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_NSF_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdsDatagramEocMsgOpen0(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &dataGramEocCtrl[0];
	pEocIfCtrl->lineId = 0;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_DATAGRAM_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdslDatagramEocMsgOpen1(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &dataGramEocCtrl[1];
	pEocIfCtrl->lineId = 1;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_DATAGRAM_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdsSkbEocMsgOpen0(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &skbEocCtrl[0];
	pEocIfCtrl->lineId = 0;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_SKB_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	BcmAdslPendingSkbMsgQueueEnable(1);
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdsSkbEocMsgOpen1(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &skbEocCtrl[1];
	pEocIfCtrl->lineId = 1;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_SKB_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	BcmAdslPendingSkbMsgQueueEnable(1);
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdsClearEocMsgOpen0(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &clearEocCtrl[0];
	pEocIfCtrl->lineId = 0;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_CLEAR_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdsClearEocMsgOpen1(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = &clearEocCtrl[1];
	pEocIfCtrl->lineId = 1;
	pEocIfCtrl->fileOpened = TRUE;
	pEocIfCtrl->eocMsgType = BCM_XDSL_CLEAR_EOC_MSG;
	init_waitqueue_head(&pEocIfCtrl->waitQue);
	file->private_data = pEocIfCtrl;
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static int BcmXdslEocMsgRelease(struct inode * inode, struct file * file)
{
	EocIfCtrl *pEocIfCtrl = (EocIfCtrl *)file->private_data;
	pEocIfCtrl->fileOpened = FALSE;
	if (pEocIfCtrl->eocMsgType==BCM_XDSL_SKB_EOC_MSG) {
		BcmAdslPendingSkbMsgQueueEnable(0);
		BcmAdslPendingSkbMsgQueueClear();
	}
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
	return 0;
}

static ssize_t BcmXdslEocMsgRead(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0, len, request_length=0;
	char *p, *inputBuffer;
	EocIfCtrl *pEocIfCtrl = (EocIfCtrl *)file->private_data;
	
	while(NULL == (p = BcmAdsl_G997FrameGet(pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType, &len))) {
		if (file->f_flags & O_NONBLOCK) {
			//printk("%s: No data available\n", __FUNCTION__);
			return -EAGAIN;
		}
		else
			if (wait_event_interruptible(pEocIfCtrl->waitQue, (NULL != p)))
				return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
	}
	
	if( p && len > 0 )
	{
		if( NULL == (inputBuffer = calloc(count, 1)) ) {
			printk("%s: calloc(%d) failed\n", __FUNCTION__, (int)count);
			return -EFAULT;
		}
		
		do
		{
			if( len + request_length < count ) {
				memcpy(inputBuffer + request_length, p, len);
				request_length += len;
			}
			p = BcmAdsl_G997FrameGetNext(pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType, &len );
		} while( p && len > 0 );
		
		BcmAdsl_G997FrameFinished(pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType);
		
		if (!(copy_to_user(buf, inputBuffer, request_length)))
			ret = request_length;
		else
			ret = -EFAULT;
		
		free(inputBuffer);
	} /* (p  & len > 0) */
#ifdef XDSL_EOC_DBG
	printk("%s: pEocIfCtrl=%px read %d/%d\n", __FUNCTION__, pEocIfCtrl, ret, (int)count);
#endif

	return( ret );
}

static ssize_t BcmXdslEocMsgWrite(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char *pBuf;
	size_t ret = 0;
	EocIfCtrl *pEocIfCtrl = (EocIfCtrl *)file->private_data;
	
	if( NULL == (pBuf = calloc(count, 1)) ) {
		BcmAdslCoreDiagWriteStatusString(pEocIfCtrl->lineId, "%s calloc(%d) failed", pEocIfCtrl->ifName, count);
		return ret;
	}
	
	if (copy_from_user(pBuf, buf, count) == 0) {
		while(FALSE == BcmXdslEocXmtReady(pEocIfCtrl)) {
			if (file->f_flags & O_NONBLOCK) {
				free(pBuf);
				return -EAGAIN;
			}
			else
				if (wait_event_interruptible(pEocIfCtrl->waitQue, (FALSE != BcmXdslEocXmtReady(pEocIfCtrl)))) {
					free(pBuf);
					return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
				}
		}
		if( BCMADSL_STATUS_SUCCESS == BcmAdsl_G997SendData(pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType, pBuf, count) ) {
			ret = count;
		}
		else {
			free(pBuf);
			BcmAdslCoreDiagWriteStatusString(pEocIfCtrl->lineId, "%s send(%d) failed", pEocIfCtrl->ifName, count);
		}
	}
	else {
		free(pBuf);
		ret = -EFAULT;
		BcmAdslCoreDiagWriteStatusString(pEocIfCtrl->lineId, "%s copy_from_user(%d) failed", pEocIfCtrl->ifName, count);
	}
#ifdef XDSL_EOC_DBG
	printk("%s: pEocIfCtrl=%px written %d bytes\n", __FUNCTION__, pEocIfCtrl, (int)ret);
#endif

	return ret;
}

static unsigned int BcmXdslEocMsgPoll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	EocIfCtrl *pEocIfCtrl = (EocIfCtrl *)file->private_data;
	
	poll_wait(file, &pEocIfCtrl->waitQue,  wait);
	
	if(AdslCoreG997FrameReceived(pEocIfCtrl->lineId, pEocIfCtrl->eocMsgType))
		mask |= POLLIN | POLLRDNORM;    /* readable */
#if 0
	if(BcmXdslEocXmtReady(pEocIfCtrl))
		mask |= POLLOUT | POLLWRNORM;   /* writable */
#endif

#ifdef XDSL_EOC_DBG
	if(0 != mask)
		printk("%s: pEocIfCtrl=%px mask %08x\n", __FUNCTION__, pEocIfCtrl, mask);
#endif

	return mask;
}

int BcmXdslIsEocIntfOpen(unsigned lineId, int eocMsgType)
{
	int res = 0;
	EocIfCtrl *pEocIfCtrl = BcmXdslGetEocIfCtrl(lineId,eocMsgType);

	if((NULL != pEocIfCtrl) && pEocIfCtrl->fileOpened)
		res = 1;
	
	return res;
}

void BcmXdslEocWakeup(unsigned lineId, int eocMsgType)
{
	EocIfCtrl *pEocIfCtrl = BcmXdslGetEocIfCtrl(lineId,eocMsgType);
#ifdef XDSL_EOC_DBG
	printk("%s: pEocIfCtrl=%px lineId %d eocMsgType 0x%X\n", __FUNCTION__, pEocIfCtrl, lineId, eocMsgType);
#endif
	if(NULL != pEocIfCtrl) { 
		if(pEocIfCtrl->fileOpened) 
			wake_up_interruptible(&pEocIfCtrl->waitQue);
		else if(eocMsgType!=BCM_XDSL_SKB_EOC_MSG) {
			BcmAdsl_G997FrameFinished(lineId, eocMsgType);
			BcmAdslCoreDiagWriteStatusString(lineId, "BcmXdslEocWakeup: Discarding received data");
		}
	}
}

static ssize_t BcmXdslSkbEocMsgRead(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	EocIfCtrl *pEocIfCtrl = (EocIfCtrl*)file->private_data;
	
	while ( !BcmAdslPendingSkbMsgQueueSize() ) {
		if (file->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		}
		else if (wait_event_interruptible(pEocIfCtrl->waitQue, BcmAdslPendingSkbMsgQueueSize() )) {
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		}
	}

	BcmAdslPendingSkbMsgRead( buf , (long*)&count );

#ifdef XDSL_EOC_DBG
	printk("%s: pEocIfCtrl=%px read %d\n", __FUNCTION__, pEocIfCtrl, (int)count);
#endif

	return count;
}

static ssize_t BcmXdslSkbEocMsgWrite(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return -EFAULT;
}

static unsigned int BcmXdslSkbEocMsgPoll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	EocIfCtrl *pEocIfCtrl = (EocIfCtrl*)file->private_data;

	poll_wait(file, &pEocIfCtrl->waitQue,  wait);
	
	if( BcmAdslPendingSkbMsgQueueSize() )
		mask |= POLLIN | POLLRDNORM;    /* readable */

#ifdef XDSL_EOC_DBG
	if(0 != mask)
		printk("%s: pEocIfCtrl=%px mask %08x\n", __FUNCTION__, pEocIfCtrl, mask);
#endif
	
	return mask;
}

static struct file_operations BcmXdslNsfEocFileOps0 = {
	.read = BcmXdslEocMsgRead,
	.write = BcmXdslEocMsgWrite,
	.poll = BcmXdslEocMsgPoll,
	.open = BcmXdsNsfEocMsgOpen0,
	.release = BcmXdslEocMsgRelease,
};
static struct file_operations BcmXdslNsfEocFileOps1 = {
	.read = BcmXdslEocMsgRead,
	.write = BcmXdslEocMsgWrite,
	.poll = BcmXdslEocMsgPoll,
	.open = BcmXdsNsfEocMsgOpen1,
	.release = BcmXdslEocMsgRelease,
};
static struct file_operations BcmXdslDatagramEocFileOps0 = {
	.read = BcmXdslEocMsgRead,
	.write = BcmXdslEocMsgWrite,
	.poll = BcmXdslEocMsgPoll,
	.open = BcmXdsDatagramEocMsgOpen0,
	.release = BcmXdslEocMsgRelease,
};
static struct file_operations BcmXdslDatagramEocFileOps1 = {
	.read = BcmXdslEocMsgRead,
	.write = BcmXdslEocMsgWrite,
	.poll = BcmXdslEocMsgPoll,
	.open = BcmXdslDatagramEocMsgOpen1,
	.release = BcmXdslEocMsgRelease,
};
static struct file_operations BcmXdslClearEocFileOps0 = {
	.read = BcmXdslEocMsgRead,
	.write = BcmXdslEocMsgWrite,
	.poll = BcmXdslEocMsgPoll,
	.open = BcmXdsClearEocMsgOpen0,
	.release = BcmXdslEocMsgRelease,
};
static struct file_operations BcmXdslClearEocFileOps1 = {
	.read = BcmXdslEocMsgRead,
	.write = BcmXdslEocMsgWrite,
	.poll = BcmXdslEocMsgPoll,
	.open = BcmXdsClearEocMsgOpen1,
	.release = BcmXdslEocMsgRelease,
};
static struct file_operations BcmXdslSkbEocFileOps0 = {
    .read = BcmXdslSkbEocMsgRead,
    .write = BcmXdslSkbEocMsgWrite,
    .poll = BcmXdslSkbEocMsgPoll,
    .open = BcmXdsSkbEocMsgOpen0,
    .release = BcmXdslEocMsgRelease,
};
static struct file_operations BcmXdslSkbEocFileOps1 = {
    .read = BcmXdslSkbEocMsgRead,
    .write = BcmXdslSkbEocMsgWrite,
    .poll = BcmXdslSkbEocMsgPoll,
    .open = BcmXdsSkbEocMsgOpen1,
    .release = BcmXdslEocMsgRelease,
};


static void BcmXdslDrvInitEocCtrl(void)
{
	memset((void*)&nsfEocCtrl[0], 0, sizeof(nsfEocCtrl));
	memset((void*)&dataGramEocCtrl[0], 0, sizeof(dataGramEocCtrl));
	memset((void*)&clearEocCtrl[0], 0, sizeof(clearEocCtrl));
	memset((void*)&skbEocCtrl[0], 0, sizeof(skbEocCtrl));
}

static BCMADSL_STATUS BcmXdslDrvOpenEocIf(unsigned lineId, int eocMsgType, char *pIfName)
{
	EocIfCtrl *pEocIfCtrl;
	BCMADSL_STATUS ret = BCMADSL_STATUS_SUCCESS;

	printk("%s: lineId %d ifName %s eocMsgType 0x%X\n", __FUNCTION__, lineId, pIfName, eocMsgType);

	switch(eocMsgType) {
		case BCM_XDSL_NSF_EOC_MSG:
			pEocIfCtrl = &nsfEocCtrl[lineId];
			pEocIfCtrl->eocMsgType = eocMsgType;
			pEocIfCtrl->lineId = lineId;
			pEocIfCtrl->fileOpened = FALSE;
			strcpy(pEocIfCtrl->ifName, pIfName);
			pEocIfCtrl->pFileOps = (0==lineId) ? &BcmXdslNsfEocFileOps0: &BcmXdslNsfEocFileOps1;
			break;
		case BCM_XDSL_DATAGRAM_EOC_MSG:
			pEocIfCtrl = &dataGramEocCtrl[lineId];
			pEocIfCtrl->eocMsgType = eocMsgType;
			pEocIfCtrl->lineId = lineId;
			pEocIfCtrl->fileOpened = FALSE;
			strcpy(pEocIfCtrl->ifName, pIfName);
			pEocIfCtrl->pFileOps = (0==lineId) ? &BcmXdslDatagramEocFileOps0: &BcmXdslDatagramEocFileOps1;
			break;
		case BCM_XDSL_CLEAR_EOC_MSG:
			pEocIfCtrl = &clearEocCtrl[lineId];
			pEocIfCtrl->eocMsgType = eocMsgType;
			pEocIfCtrl->lineId = lineId;
			pEocIfCtrl->fileOpened = FALSE;
			strcpy(pEocIfCtrl->ifName, pIfName);
			pEocIfCtrl->pFileOps = (0==lineId) ? &BcmXdslClearEocFileOps0: &BcmXdslClearEocFileOps1;
			break;
		case BCM_XDSL_SKB_EOC_MSG:
			pEocIfCtrl = &skbEocCtrl[lineId];
			pEocIfCtrl->eocMsgType = eocMsgType;
			pEocIfCtrl->lineId = lineId;
			pEocIfCtrl->fileOpened = FALSE;
			strcpy(pEocIfCtrl->ifName, pIfName);
			pEocIfCtrl->pFileOps = (0==lineId) ? &BcmXdslSkbEocFileOps0: &BcmXdslSkbEocFileOps1;
			break;
		default:
			printk("%s: Unkown eocMsgType 0x%X\n", __FUNCTION__, eocMsgType);
			return BCMADSL_STATUS_ERROR;
	}

	if ( pEocIfCtrl->pProcEntry == NULL )
		pEocIfCtrl->pProcEntry = proc_create(pEocIfCtrl->ifName, S_IRUSR, NULL, pEocIfCtrl->pFileOps);
	else
		printk("%s: pointer to struct proc_dir_entry not empty\n", __FUNCTION__);

	if(NULL == pEocIfCtrl->pProcEntry) {
		ret = BCMADSL_STATUS_ERROR;
		printk("%s: proc_create(%s} failed!\n", __FUNCTION__, pEocIfCtrl->ifName);
	}
	
	return ret;
}

static BCMADSL_STATUS BcmXdslDrvCloseEocIf(unsigned lineId, int eocMsgType, char *pIfName)
{
	EocIfCtrl *pEocIfCtrl;
	BCMADSL_STATUS ret = BCMADSL_STATUS_SUCCESS;

	printk("%s: lineId %d ifName %s eocMsgType 0x%X\n", __FUNCTION__, lineId, pIfName, eocMsgType);
	
	switch(eocMsgType) {
		case BCM_XDSL_NSF_EOC_MSG:
			pEocIfCtrl = &nsfEocCtrl[lineId];
			break;
		case BCM_XDSL_DATAGRAM_EOC_MSG:
			pEocIfCtrl = &dataGramEocCtrl[lineId];
			break;
		case BCM_XDSL_CLEAR_EOC_MSG:
			pEocIfCtrl = &clearEocCtrl[lineId];
			break;
		case BCM_XDSL_SKB_EOC_MSG:
			pEocIfCtrl = &skbEocCtrl[lineId];
			break;
		default:
			printk("%s: Unkown eocMsgType 0x%X\n", __FUNCTION__, eocMsgType);
			return BCMADSL_STATUS_ERROR;
	}

	if(NULL != pEocIfCtrl->pProcEntry) {
		remove_proc_entry(pEocIfCtrl->ifName, NULL);
		pEocIfCtrl->pProcEntry = NULL;
		printk("%s: procEntry for %s is removed\n", __FUNCTION__, pEocIfCtrl->ifName);
	}
	else {
		printk("%s: procEntry for %s doesn't exist!\n", __FUNCTION__, pIfName);
		ret = BCMADSL_STATUS_ERROR;
	}
	
	return ret;
}

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) || !defined(MODULE)
module_init( adsl_init );
module_exit( adsl_cleanup );
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
MODULE_LICENSE("Proprietary");
#endif

