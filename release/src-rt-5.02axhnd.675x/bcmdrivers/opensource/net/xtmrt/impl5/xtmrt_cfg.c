/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
/**************************************************************************
 * File Name  : xtmrt_cfg.c
 *
 * Description: This file implements BCM63x68 ATM/PTM network device driver
 *              runtime processing - sending and receiving data.
 ***************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/blog.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcmnet.h"
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include "bcmxtmrtimpl.h"
#if (defined(CONFIG_BCM963268) || defined(CONFIG_BCM963178)) && defined(CONFIG_BCM_ARCHER)
#include "xtmrt_archer.h"
#elif defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
#include "bcmPktDma.h"
#include "xtmrt_dma.h"
#else
#include "xtmrt_runner.h"
#endif


/**** Externs ****/

extern int kerSysGetMacAddress(UINT8 *pucaMacAddr, unsigned long ulId);

/**** Globals ****/

int bcmxtmrt_in_init_dev = 0;


/**** Prototypes ****/

static int bcmxtmrt_open(struct net_device *dev);
static int bcmxtmrt_close(struct net_device *dev);
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd);
static void bcmxtmrt_timeout(struct net_device *dev);
static struct rtnl_link_stats64 *bcmxtmrt_query(struct net_device *dev, 
                                                struct rtnl_link_stats64 *pStats);
static void bcmxtmrt_clrStats(struct net_device *dev);
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *useraddr);

static int bcmxtmrt_change_mtu(struct net_device *dev, int new_mtu);
static int bcmxtmrt_set_MacAddress (struct net_device *dev, void *p);
static int DoGlobReInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip);
static int DoCreateDeviceReq(PXTMRT_CREATE_NETWORK_DEVICE pCnd);
static int DoRegCellHdlrReq(PXTMRT_CELL_HDLR pCh);
static int DoUnregCellHdlrReq(PXTMRT_CELL_HDLR pCh);
static int DoLinkStsChangedReq(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                               PXTMRT_LINK_STATUS_CHANGE pLsc);
static int DoLinkUp(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                    PXTMRT_LINK_STATUS_CHANGE pLsc,
                    UINT32 ulDevId);
static int DoLinkDownRx(UINT32 ulPortId);
static int DoLinkDownTx(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                        PXTMRT_LINK_STATUS_CHANGE pLsc);
static int DoUnsetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                          PXTMRT_TRANSMIT_QUEUE_ID pTxQId);
static int DoStartTxQueues(PBCMXTMRT_DEV_CONTEXT pDevCtx);
static int DoStopTxQueues(PBCMXTMRT_DEV_CONTEXT pDevCtx);
static int DoSendCellReq(PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC);
static int DoDeleteDeviceReq(PBCMXTMRT_DEV_CONTEXT pDevCtx);
static int DoGetNetDevTxChannel(PXTMRT_NETDEV_TXCHANNEL pParm);
static int DoTogglePortDataStatusReq(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                                     PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE pParm);

static int DoSetTxPortShaperInfo (PXTMRT_PORT_SHAPER_INFO pShaperInfo) ;

/**** Statics ****/

static const struct header_ops bcmXtmRt_headerOps =
   {
      .parse = NULL
   };

static const struct net_device_ops bcmXtmRt_netdevops =
   {
      .ndo_open            = bcmxtmrt_open,
      .ndo_stop            = bcmxtmrt_close,
      .ndo_start_xmit      = (HardStartXmitFuncP)bcmxtmrt_xmit,
      .ndo_do_ioctl        = bcmxtmrt_ioctl,
      .ndo_set_mac_address = bcmxtmrt_set_MacAddress,
      .ndo_tx_timeout      = bcmxtmrt_timeout,
      .ndo_get_stats64       = bcmxtmrt_query,
      .ndo_change_mtu      = bcmxtmrt_change_mtu
   };


/*---------------------------------------------------------------------------
 * int bcmxtmrt_open(struct net_device *dev)
 * Description:
 *    Called to make the device operational.  Called due to shell command,
 *    "ifconfig <device_name> up".
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_open(struct net_device *dev)
{
   int rc = 0;
   PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);

   BCM_XTM_DEBUG("bcmxtmrt_open\n");

   /* Must set the state to open before enabling rx interrupt.
    * Otherwise, packet received from dqm right after rx interrupt is enable,
    * will not be processed and so dqm rx interrupt will not be re-enabled.
    */ 
   if (pDevCtx->ulAdminStatus == ADMSTS_UP)
      pDevCtx->ulOpenState = XTMRT_DEV_OPENED;
   else
      return -EIO;
         

   netif_start_queue(dev);

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
   printk(CARDNAME ": E-RXIntr\n");
   bcmxapi_enable_rx_interrupt();
#endif

   return rc;
    
}  /* bcmxtmrt_open() */


/*---------------------------------------------------------------------------
 * int bcmxtmrt_close(struct net_device *dev)
 * Description:
 *    Called to stop the device.  Called due to shell command,
 *    "ifconfig <device_name> down".
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_close(struct net_device *dev)
{
   PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);

   if (pDevCtx->ulOpenState != XTMRT_DEV_CLOSED)
   {
      BCM_XTM_DEBUG("bcmxtmrt_close\n");

      pDevCtx->ulOpenState = XTMRT_DEV_CLOSED;
      netif_stop_queue(dev);
   }

   return 0;
    
} /* bcmxtmrt_close */


/*---------------------------------------------------------------------------
 * int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd)
 * Description:
 *    Driver IOCTL entry point.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd)
{
   PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
   MirrorCfg mirrorCfg;
   int *data = (int*)Req->ifr_data;
   int status;
   int nRet = 0;

   switch (nCmd)
   {
   case SIOCGLINKSTATE:
      if (pDevCtx->ulLinkState == LINK_UP)
         status = LINKSTATE_UP;
      else
         status = LINKSTATE_DOWN;
      if (copy_to_user((void*)data, (void*)&status, sizeof(int)))
         nRet = -EFAULT;
      break;

   case SIOCSCLEARMIBCNTR:
      bcmxtmrt_clrStats(dev);
      break;

   case SIOCMIBINFO:
      if (copy_to_user((void*)data, (void*)&pDevCtx->MibInfo,
                       sizeof(pDevCtx->MibInfo)))
         nRet = -EFAULT;
      break;

   case SIOCPORTMIRROR:
      if (copy_from_user((void*)&mirrorCfg,data,sizeof(MirrorCfg)))
         nRet=-EFAULT;
      else
      {
         if (mirrorCfg.nDirection == MIRROR_DIR_IN)
         {
            if (mirrorCfg.nStatus == MIRROR_ENABLED)
               strcpy(pDevCtx->szMirrorIntfIn, mirrorCfg.szMirrorInterface);
            else
               memset(pDevCtx->szMirrorIntfIn, 0x00, MIRROR_INTF_SIZE);
         }
         else /* MIRROR_DIR_OUT */
         {
            if (mirrorCfg.nStatus == MIRROR_ENABLED)
               strcpy(pDevCtx->szMirrorIntfOut, mirrorCfg.szMirrorInterface);
            else
               memset(pDevCtx->szMirrorIntfOut, 0x00, MIRROR_INTF_SIZE);
         }
      }
      break;

   case SIOCETHTOOL:
      nRet = bcmxtmrt_ethtool_ioctl(pDevCtx, (void *)Req->ifr_data);
      break;

   default:
      nRet = -EOPNOTSUPP;
      break;
   }

   return (nRet);
    
}  /* bcmxtmrt_ioctl() */

/*---------------------------------------------------------------------------
 * int bcmxtmrt_set_MacAddress (struct net_device *dev, void *addr)
 * Description:
 *    Called to set the hardware mac address. Called due to shell command,
 *    "ifconfig <device_name> hw ether".
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_set_MacAddress(struct net_device *dev, void *p)
{
   struct sockaddr *addr = p;

   if (netif_running(dev))
      return -EBUSY;
   if (!is_valid_ether_addr(addr->sa_data))
      return -EADDRNOTAVAIL;
   memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

   return 0;
}
 
/*---------------------------------------------------------------------------
 * void bcmxtmrt_timeout(struct net_device *dev)
 * Description:
 *    Called when there is a transmit timeout.
 * Returns: void.
 *---------------------------------------------------------------------------
 */
static void bcmxtmrt_timeout(struct net_device *dev)
{
   dev->trans_start = jiffies;
   netif_wake_queue(dev);
    
} /* bcmxtmrt_timeout() */


/*---------------------------------------------------------------------------
 * struct rtnl_link_stats64 *bcmxtmrt_query(struct net_device *dev,
 *                                          struct rtnl_link_stats64 *pStats64)
 * Description:
 *    Called to return device statistics.
 * Returns:
 *    rtnl_link_stats64 *
 *---------------------------------------------------------------------------
 */
static struct rtnl_link_stats64 *bcmxtmrt_query(struct net_device *dev, 
                                                struct rtnl_link_stats64 *pStats)
{
   PBCMXTMRT_DEV_CONTEXT pDevCtx    = netdev_priv(dev);
   PBCMXTMRT_GLOBAL_INFO pGi        = &g_GlobalInfo;

   /* Do not grab statistics from MIB hardware but instead simply return the
      pStats structure, which is constantly updated in software instead to
      support extended statistics (i.e. multicast, broadcast, unicast 
      packets and other data). */
   UINT32 i;
   UINT32 found      = 0;
   UINT32 rxDropped  = 0;
   UINT32 txDropped  = 0;
   UINT64 rxTotalDropped = 0;
   UINT64 txTotalDropped = 0;

   /* Copy the current driver stats to local copy */
   memcpy(pStats, &pDevCtx->DevStats, sizeof(*pStats));

   for (i = 0; i < MAX_DEFAULT_MATCH_IDS; i++)
   {
      if (pGi->pDevCtxsByMatchId[i] == pDevCtx)
      {
         bcmxapi_XtmGetStats(i, &rxDropped, &txDropped); 
         rxTotalDropped += rxDropped;
         txTotalDropped += txDropped;
         found = 1;
      }
   }

   if (found)
   {
      pStats->rx_dropped += rxTotalDropped;
      pStats->tx_dropped += txTotalDropped;
   }

   return (pStats);
    
}  /* bcmxtmrt_query() */

/*---------------------------------------------------------------------------
 * int bcmxtmrt_change_mtu(struct net_device *dev, int new_mtu)
 * Description: Called to change device mtu.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_change_mtu(struct net_device *dev, int new_mtu)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = (PBCMXTMRT_DEV_CONTEXT)netdev_priv(dev);

    switch (pDevCtx->ulEncapType)
    {
    case TYPE_IP:
    case TYPE_PPP:
        break;

    case TYPE_ETH:
        {
            int max_mtu = (pDevCtx->ulHdrType == HT_PTM) ? PTM_MAX_MTU_PAYLOAD_SIZE : XTM_MAX_MTU_PAYLOAD_SIZE;

            if (new_mtu < ETH_ZLEN || new_mtu > max_mtu)
                return -EINVAL;
        }
        break;

    default:
        return -EPROTONOSUPPORT;
    }

    dev->mtu = new_mtu;
    return 0;
}


/*---------------------------------------------------------------------------
 * void bcmxtmrt_clrStats(struct net_device *dev)
 * Description:
 *    Called to clear device statistics.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void bcmxtmrt_clrStats(struct net_device *dev)
{
   PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
   PBCMXTMRT_GLOBAL_INFO pGi     = &g_GlobalInfo;
   UINT32 i;

   /* 
   *pGi->pulMibRxCtrl |= pGi->ulMibRxClrOnRead;
   bcmxtmrt_query(dev);
   *pGi->pulMibRxCtrl &= ~pGi->ulMibRxClrOnRead; 
   */ 

   for (i = 0; i < MAX_DEFAULT_MATCH_IDS; i++)
   {
      if (pGi->pDevCtxsByMatchId[i] == pDevCtx)
      {
         bcmxapi_XtmResetStats(i); 
      }
   }

   memset(&pDevCtx->DevStats, 0, sizeof(pDevCtx->DevStats));
    
}  /* bcmxtmrt_clrStats() */


/*---------------------------------------------------------------------------
 * int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *useraddr)
 * Description:
 *    Driver ethtool IOCTL entry point.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *useraddr)
{
   struct ethtool_drvinfo info;
   struct ethtool_cmd ecmd;
   UINT32 ethcmd;
   int nRet = 0;

   if (copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)) == 0)
   {
      switch (ethcmd)
      {
      case ETHTOOL_GDRVINFO:
         info.cmd = ETHTOOL_GDRVINFO;
         strncpy(info.driver, CARDNAME, sizeof(info.driver)-1);
         strncpy(info.version, XTMRT_VERSION, sizeof(info.version)-1);
         if (copy_to_user(useraddr, &info, sizeof(info)))
            nRet = -EFAULT;
         break;

      case ETHTOOL_GSET:
         ecmd.cmd = ETHTOOL_GSET;
         ecmd.speed = pDevCtx->MibInfo.ulIfSpeed / (1024 * 1024);
         if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
            nRet = -EFAULT;
         break;

      default:
         nRet = -EOPNOTSUPP;
         break;
      }
   }
   else
      nRet = -EFAULT;

   return (nRet);
    
}  /* bcmxtmrt_ethtool_ioctl() */


/*---------------------------------------------------------------------------
 * int bcmxtmrt_request(XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParm)
 * Description:
 *    Request from the bcmxtmcfg driver.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxtmrt_request(XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParm)
{
   PBCMXTMRT_DEV_CONTEXT pDevCtx = (PBCMXTMRT_DEV_CONTEXT)hDev;
   int nRet = 0;

   switch (ulCommand)
   {
      case XTMRT_CMD_GLOBAL_INITIALIZATION:
         nRet = bcmxapi_DoGlobInitReq((PXTMRT_GLOBAL_INIT_PARMS)pParm);
         break;

      case XTMRT_CMD_GLOBAL_REINITIALIZATION:
         nRet = DoGlobReInitReq((PXTMRT_GLOBAL_INIT_PARMS)pParm);
         break;

      case XTMRT_CMD_GLOBAL_UNINITIALIZATION:
         nRet = bcmxapi_DoGlobUninitReq();
         break;

      case XTMRT_CMD_CREATE_DEVICE:
         nRet = DoCreateDeviceReq((PXTMRT_CREATE_NETWORK_DEVICE)pParm);
         break;

      case XTMRT_CMD_GET_DEVICE_STATE:
         *(UINT32 *)pParm = pDevCtx->ulOpenState;
         break;

      case XTMRT_CMD_SET_ADMIN_STATUS:
         pDevCtx->ulAdminStatus = (*((UINT32 *)pParm));
         break;

      case XTMRT_CMD_REGISTER_CELL_HANDLER:
         nRet = DoRegCellHdlrReq((PXTMRT_CELL_HDLR)pParm);
         break;

      case XTMRT_CMD_UNREGISTER_CELL_HANDLER:
         nRet = DoUnregCellHdlrReq((PXTMRT_CELL_HDLR)pParm);
         break;

      case XTMRT_CMD_LINK_STATUS_CHANGED:
         nRet = DoLinkStsChangedReq(pDevCtx, (PXTMRT_LINK_STATUS_CHANGE)pParm);
         break;

      case XTMRT_CMD_SEND_CELL:
         nRet = DoSendCellReq(pDevCtx, (PXTMRT_CELL)pParm);
         break;

      case XTMRT_CMD_DELETE_DEVICE:
         nRet = DoDeleteDeviceReq(pDevCtx);
         break;

      case XTMRT_CMD_SET_TX_QUEUE:
         nRet = bcmxapi_DoSetTxQueue(pDevCtx, (PXTMRT_TRANSMIT_QUEUE_ID)pParm);
         break;

      case XTMRT_CMD_UNSET_TX_QUEUE:
         nRet = DoUnsetTxQueue(pDevCtx, (PXTMRT_TRANSMIT_QUEUE_ID)pParm);
         break;

      case XTMRT_CMD_GET_NETDEV_TXCHANNEL:
         nRet = DoGetNetDevTxChannel((PXTMRT_NETDEV_TXCHANNEL) pParm);
         break;

      case XTMRT_CMD_TOGGLE_PORT_DATA_STATUS_CHANGE:
         nRet = DoTogglePortDataStatusReq(pDevCtx, (PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE)pParm);
         break;

      case XTMRT_CMD_SET_TEQ_DEVCTX:
         g_GlobalInfo.pTeqNetDev = (struct net_device *)pParm;

         /* If receive interrupts are not enabled, enable them. */
         if (g_GlobalInfo.ulDrvState == XTMRT_INITIALIZED)
         {
            g_GlobalInfo.ulDrvState = XTMRT_RUNNING;

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963178)
            /* Enable receive interrupts and start a timer. */
            printk(CARDNAME ": E-RXIntr2\n");
            bcmxapi_enable_rx_interrupt();
#endif

            g_GlobalInfo.Timer.expires = jiffies + SAR_TIMEOUT;
            add_timer(&g_GlobalInfo.Timer);
         }
         break;

      case XTMRT_CMD_SET_ATMBOND_SID_MODE:
         g_GlobalInfo.atmBondSidMode = (*((UINT32 *)pParm));
         printk(CARDNAME ": ATM Bonding SID mode - %u \n", g_GlobalInfo.atmBondSidMode);
         break;

      case XTMRT_CMD_STOP_ALL_TX_QUEUE:
         nRet = DoStopTxQueues(pDevCtx);
         break;

      case XTMRT_CMD_START_ALL_TX_QUEUE:
         nRet = DoStartTxQueues(pDevCtx);
         break;

      case XTMRT_CMD_SET_TX_PORT_SHAPER_INFO:
         nRet = DoSetTxPortShaperInfo((PXTMRT_PORT_SHAPER_INFO)pParm);
         break;

      default:
         nRet = -EINVAL;
         break;

   } /* switch (ulCommand) */

   return (nRet);
   
}  /* bcmxtmrt_request() */


/*---------------------------------------------------------------------------
 * int DoGlobReInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip)
 * Description:
 *    Processes an XTMRT_CMD_GLOBAL_REINITIALIZATION command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoGlobReInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   if (pGi->ulDrvState == XTMRT_UNINITIALIZED)
      return -EPERM;

   pGi->bondConfig.uConfig = pGip->bondConfig.uConfig;
   if ((pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
       (pGi->bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE))
      printk (CARDNAME ": PTM/ATM Bonding Mode configured in system \n");
   else
      printk (CARDNAME ": PTM/ATM Non-Bonding Mode configured in system \n");

   return 0;
    
}  /* DoGlobReInitReq() */


/*---------------------------------------------------------------------------
 * int DoCreateDeviceReq(PXTMRT_CREATE_NETWORK_DEVICE pCnd)
 * Description:
 *    Processes an XTMRT_CMD_CREATE_DEVICE command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoCreateDeviceReq(PXTMRT_CREATE_NETWORK_DEVICE pCnd)
{
   int nRet = 0;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
   struct net_device *dev = NULL;
//    int i;
   UINT32 unit = 0;
   UINT32 macId = 0;
   UINT32 blogPhyType;
   UINT32 ulRfc2684_type = RFC2684_NONE;
   UINT32 hwAction;

   BCM_XTM_DEBUG("DoCreateDeviceReq\n");

   if (pGi->ulDrvState != XTMRT_UNINITIALIZED &&
       (dev = alloc_netdev(sizeof(BCMXTMRT_DEV_CONTEXT),
                           pCnd->szNetworkDeviceName, 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
                           NET_NAME_UNKNOWN,
#endif
ether_setup)) != NULL)
   {
      dev_alloc_name(dev, dev->name);

      pDevCtx = (PBCMXTMRT_DEV_CONTEXT)netdev_priv(dev);

      memset(pDevCtx, 0x00, sizeof(BCMXTMRT_DEV_CONTEXT));
      memcpy(&pDevCtx->Addr, &pCnd->ConnAddr, sizeof(XTM_ADDR));
      if ((pCnd->ConnAddr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM)
         pDevCtx->ulHdrType = pCnd->ulHeaderType;
      else
         pDevCtx->ulHdrType = HT_PTM;

      if (pDevCtx->ulHdrType == HT_PTM)
      {
         if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
            pDevCtx->ulTrafficType = TRAFFIC_TYPE_PTM_BONDED;
         else
            pDevCtx->ulTrafficType = TRAFFIC_TYPE_PTM;
      }
      else
      {
         if (pGi->bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)
            pDevCtx->ulTrafficType = TRAFFIC_TYPE_ATM_BONDED;
         else
            pDevCtx->ulTrafficType = TRAFFIC_TYPE_ATM;
      }

      pDevCtx->ulTxPafEnabled = pCnd->ulTxPafEnabled ;
      printk (CARDNAME ": TxPAF Status = %s \n", (pDevCtx->ulTxPafEnabled==1) ? "Enabled" : "Disabled") ;

      pDevCtx->ulFlags        = pCnd->ulFlags;
      pDevCtx->pDev           = dev;
      pDevCtx->ulAdminStatus  = ADMSTS_UP;
      pDevCtx->ucTxVcid       = INVALID_VCID;

      /* Read and display the MAC address. */
      dev->dev_addr[0] = 0xff;

      /* format the mac id */

      /* There is no need to include the unit number
      * in the mac id because all xtm interfaces can
      * share the same mac address.
      */
//        i = strcspn(dev->name, "0123456789");
//        if (i > 0)
//           unit = simple_strtoul(&(dev->name[i]), (char **)NULL, 10);

      if (pDevCtx->ulHdrType == HT_PTM)
         macId = MAC_ADDRESS_PTM;
      else
         macId = MAC_ADDRESS_ATM;
      /* set unit number to bit 20-27 */
      macId |= ((unit & 0xff) << 20);

      kerSysGetMacAddress(dev->dev_addr, macId);

      if ((dev->dev_addr[0] & 0x01) == 0x01)
      {
         printk( KERN_ERR CARDNAME": Unable to read MAC address from "
             "persistent storage.  Using default address.\n" );
         memcpy( dev->dev_addr, "\x02\x10\x18\x02\x00\x01", 6 );
      }

      printk(CARDNAME": MAC address: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n",
             dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
             dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
      dev->netdev_ops = &bcmXtmRt_netdevops;

#if defined(CONFIG_BCM_KF_EXTSTATS)
      /* Indicate we're supporting extended statistics */
      dev->features |= NETIF_F_EXTSTATS;
#endif
        
#if defined(CONFIG_BLOG)
      dev->clr_stats      = bcmxtmrt_clrStats;
      /* XTM interface need following stats for accelerated flows */
      dev->blog_stats_flags |= BLOG_DEV_STAT_FLAG_INCLUDE_HW ;
#endif
      dev->watchdog_timeo = SAR_TIMEOUT;

      //In case of DPU mode we shouldn't set the IFF_WANDEV
#if !defined(CONFIG_BCM_55153_DPU)
      /* identify as a WAN interface to block WAN-WAN traffic */
      dev->priv_flags |= IFF_WANDEV;
#endif
      switch( pDevCtx->ulHdrType )
      {
      case HT_LLC_SNAP_ROUTE_IP:
      case HT_VC_MUX_IPOA:
         pDevCtx->ulEncapType = TYPE_IP;     /* IPoA */

         /* Since IPoA does not need a Ethernet header,
          * set the pointers below to NULL. Refer to kernel rt2684.c.
          */
         dev->header_ops = &bcmXtmRt_headerOps;

         dev->type = ARPHRD_PPP;
         dev->hard_header_len = HT_LEN_LLC_SNAP_ROUTE_IP;
         dev->mtu = RFC1626_MTU;
         dev->addr_len = 0;
         dev->tx_queue_len = 100;
         dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
         break;

      case HT_LLC_ENCAPS_PPP:
      case HT_VC_MUX_PPPOA:
         pDevCtx->ulEncapType = TYPE_PPP;    /*PPPoA*/
         break;

      default:
         pDevCtx->ulEncapType = TYPE_ETH;    /* bridge, MER, PPPoE, PTM */
         dev->flags = IFF_BROADCAST | IFF_MULTICAST;
         if (pDevCtx->ulHdrType == HT_PTM)
             dev->mtu = BCM_PTM_DEFAULT_MTU_SIZE;
         else
             dev->mtu = BCM_XTM_DEFAULT_MTU_SIZE;
         break;
      }

      if ((pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0)
      {
        if (HT_LEN(pDevCtx->ulHdrType) > 0)
           ulRfc2684_type = HT_TYPE(pDevCtx->ulHdrType);
      }

      hwAction    = HT_TYPE(pDevCtx->ulHdrType);
      blogPhyType = BLOG_SET_PHYHDR(ulRfc2684_type, BLOG_XTMPHY);

      /* Embed HT_TYPE() info for provisioned mcast case */
      blogPhyType |= BLOG_SET_HW_ACT(hwAction);
      netdev_path_set_hw_port(dev, 0, blogPhyType);

      /* Don't reset or enable the device yet. "Open" does that. */
      printk(CARDNAME ": register_netdev\n");
      nRet = register_netdev(dev);
      printk(CARDNAME ": register_netdev done\n");
      if (nRet == 0)
      {
         UINT32 i;
         netif_carrier_off(dev);
         printk (CARDNAME ": netif_carrier_off \n") ;
         for (i = 0; i < MAX_DEV_CTXS; i++)
         {
            if (pGi->pDevCtxs[i] == NULL)
            {
               UINT32 trailerDelLen = 0;
               UINT32 delLen = 0;

               pGi->pDevCtxs[i] = pDevCtx;

               if (pDevCtx->ulHdrType == HT_PTM &&
                   (pDevCtx->ulFlags & CNI_HW_REMOVE_TRAILER) == 0)
                  trailerDelLen = (ETH_FCS_LEN + XTMRT_PTM_CRC_SIZE);

               if ((pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0)
                  delLen = HT_LEN(pDevCtx->ulHdrType);

               bcmxapi_XtmCreateDevice(i, pDevCtx->ulEncapType, delLen, trailerDelLen);

               break;
            }
         }
         pCnd->hDev = (XTMRT_HANDLE)pDevCtx;
#if defined(CONFIG_BCM_55153_DPU)
         //For DPU its statically created netdevice, we don't normally have any
         //specific Open IOCTL coming in, here we are making sure that this
         //device is in ready state. 
         if (pDevCtx->ulAdminStatus == ADMSTS_UP)
            pDevCtx->ulOpenState = XTMRT_DEV_OPENED;
               
       
         netif_start_queue(dev);
#endif
      }
      else
      {
         printk(KERN_ERR CARDNAME": register_netdev failed\n");
         free_netdev(dev);
      }
   }
   else
   {
      printk(KERN_ERR CARDNAME": alloc_netdev failed\n");
      nRet = -ENOMEM;
   }

   return (nRet);
    
}  /* DoCreateDeviceReq() */


/*---------------------------------------------------------------------------
 * int DoRegCellHdlrReq(PXTMRT_CELL_HDLR pCh)
 * Description:
 *    Processes an XTMRT_CMD_REGISTER_CELL_HANDLER command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoRegCellHdlrReq(PXTMRT_CELL_HDLR pCh)
{
   int nRet = 0;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   switch (pCh->ulCellHandlerType)
   {
   case CELL_HDLR_OAM:
      if (pGi->pfnOamHandler == NULL)
      {
         pGi->pfnOamHandler = pCh->pfnCellHandler;
         pGi->pOamContext   = pCh->pContext;
      }
      else
         nRet = -EEXIST;
      break;

   case CELL_HDLR_ASM:
      if (pGi->pfnAsmHandler == NULL)
      {
         pGi->pfnAsmHandler = pCh->pfnCellHandler;
         pGi->pAsmContext   = pCh->pContext;
      }
      else
         nRet = -EEXIST;
      break;
   }

   return (nRet);

}  /* DoRegCellHdlrReq() */


/*---------------------------------------------------------------------------
 * int DoUnregCellHdlrReq(PXTMRT_CELL_HDLR pCh)
 * Description:
 *    Processes an XTMRT_CMD_UNREGISTER_CELL_HANDLER command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoUnregCellHdlrReq(PXTMRT_CELL_HDLR pCh)
{
   int nRet = 0;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   switch (pCh->ulCellHandlerType)
   {
   case CELL_HDLR_OAM:
      if (pGi->pfnOamHandler == pCh->pfnCellHandler)
      {
         pGi->pfnOamHandler = NULL;
         pGi->pOamContext   = NULL;
      }
      else
         nRet = -EPERM;
      break;

   case CELL_HDLR_ASM:
      if (pGi->pfnAsmHandler == pCh->pfnCellHandler)
      {
         pGi->pfnAsmHandler = NULL;
         pGi->pAsmContext   = NULL;
      }
      else
         nRet = -EPERM;
      break;
   }

   return (nRet);
    
}  /* DoUnregCellHdlrReq() */


/*---------------------------------------------------------------------------
 * int DoLinkStsChangedReq(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                         PXTMRT_LINK_STATUS_CHANGE pLsc)
 * Description:
 *    Processes an XTMRT_CMD_LINK_STATUS_CHANGED command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoLinkStsChangedReq(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                               PXTMRT_LINK_STATUS_CHANGE pLsc)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   int nRet = -EPERM;

   local_bh_disable();

   if (pDevCtx)
   {
      if (pDevCtx->pDev) {

         UINT32 i;

#if 0  /* debug code */
         {
            int j;
            printk("ulLinkState: %ld ulLinkUsRate: %ld ulLinkDsRate: %ld ulLinkDataMask: %ld ulTransmitQueueIdsSize: %ld ucTxVcid: %d ulRxVcidsSize: %ld\n\n",
                  pLsc->ulLinkState, pLsc->ulLinkUsRate, pLsc->ulLinkDsRate, pLsc->ulLinkDataMask, pLsc->ulTransmitQueueIdsSize, pLsc->ucTxVcid, pLsc->ulRxVcidsSize);

            for (j = 0; j < MAX_TRANSMIT_QUEUES; j++)
               printk("%d: ulPortId: %ld PtmPriority: %ld WeightAlg: %ld WeightValue: %ld SubPriority: %ld QueueSize: %ld QueueIndex: %ld BondingPortId: %ld\n",
                     j, pLsc->TransmitQueueIds[j].ulPortId, pLsc->TransmitQueueIds[j].ulPtmPriority, pLsc->TransmitQueueIds[j].ucWeightAlg, pLsc->TransmitQueueIds[j].ulWeightValue,
                     pLsc->TransmitQueueIds[j].ucSubPriority, pLsc->TransmitQueueIds[j].usQueueSize, pLsc->TransmitQueueIds[j].ulQueueIndex, pLsc->TransmitQueueIds[j].ulBondingPortId);
         }
#endif

         for (i = 0; i < MAX_DEV_CTXS; i++)
         {
            if (pGi->pDevCtxs[i] == pDevCtx)
            {
               UINT32 ulMibOldSpeed;
               UINT32 ulLinkUsRate[MAX_BOND_PORTS], ulLinkDsRate;

               pDevCtx->ulFlags |= pLsc->ulLinkState & LSC_RAW_ENET_MODE;
               pLsc->ulLinkState &= ~LSC_RAW_ENET_MODE;
               pDevCtx->MibInfo.ulIfLastChange = (jiffies * 100) / HZ;
               ulMibOldSpeed = pDevCtx->MibInfo.ulIfSpeed ;
               pDevCtx->MibInfo.ulIfSpeed = pLsc->ulLinkUsRate+pLsc->ulOtherLinkUsRate;

               ulLinkUsRate[0] = pDevCtx->ulLinkUsRate[0] ;
               ulLinkUsRate[1] = pDevCtx->ulLinkUsRate[1] ;
               ulLinkDsRate    = pDevCtx->ulLinkDsRate ;
               pDevCtx->ulLinkUsRate[0] = pLsc->ulLinkUsRate ;
               pDevCtx->ulLinkUsRate[1] = pLsc->ulOtherLinkUsRate ;
               pDevCtx->ulLinkDsRate    = pLsc->ulLinkDsRate + pLsc->ulOtherLinkDsRate;

               /* compute the weights */
               if (pLsc->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)
               {
                  bcmxtmrt_ptmbond_calculate_link_parameters(&pDevCtx->ulLinkUsRate[0],
                        pLsc->ulLinkDataMask, 0);
               }
               else
               {
                  memset(&(pGi->ptmBondInfo), 0x00, sizeof(XtmRtPtmBondInfo));
               }

               if (pLsc->ulLinkState == LINK_UP)
                  nRet = DoLinkUp( pDevCtx, pLsc , i);
               else
               {
                  if(pLsc->ulLinkDataMask == 0) {
                     printk (CARDNAME ": netif_carrier_off \n") ;
                     netif_carrier_off(pDevCtx->pDev);
                  }
                  spin_lock_bh(&pGi->xtmlock_tx);
                  nRet = DoLinkDownTx(pDevCtx, pLsc);
                  spin_unlock_bh(&pGi->xtmlock_tx);
               }
               break;
            } /* if( pGi->pDevCtxs[i] == pDevCtx ) */
         } /* for (i) */
      } /* pDev */
   }
   else
   {
      /* No device context indicates that the link is down.  Do global link
       * down processing.  pLsc is really an unsigned long containing the
       * port id.
       */
      spin_lock(&pGi->xtmlock_rx);
      nRet = DoLinkDownRx(*((UINT32 *)pLsc));
      spin_unlock(&pGi->xtmlock_rx);
   }

   local_bh_enable();


#if 0 /* debug code */
   printk("\n");
   printk("GLOBAL: ulNumTxQs %ld\n", pGi->ulNumTxQs);

   if (pDevCtx != NULL)
   {
      printk("DEV PTR: %p VPI: %d VCI: %d\n", pDevCtx, pDevCtx->Addr.u.Vcc.usVpi, pDevCtx->Addr.u.Vcc.usVci);
      printk("DEV ulLinkState: %ld ulPortDataMask: %ld ulOpenState: %ld ulAdminStatus: %ld \n",
            pDevCtx->ulLinkState, pDevCtx->ulPortDataMask, pDevCtx->ulOpenState, pDevCtx->ulAdminStatus);
      printk("DEV ulHdrType: %ld ulEncapType: %ld ulFlags: %ld ucTxVcid: %d ulTxQInfosSize: %ld\n",
            pDevCtx->ulHdrType, pDevCtx->ulEncapType, pDevCtx->ulFlags, pDevCtx->ucTxVcid, pDevCtx->ulTxQInfosSize);
   }
   else
      printk("StsChangedReq called with NULL pDevCtx\n");
#endif

   return (nRet);

}  /* DoLinkStsChangedReq() */


/*---------------------------------------------------------------------------
 * int DoLinkUp(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *              PXTMRT_LINK_STATUS_CHANGE pLsc, UINT32 ulDevId)
 * Description:
 *    Processes a "link up" condition.
 *    In bonding case, successive links may be coming UP one after
 *    another, accordingly the processing differs.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoLinkUp(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                    PXTMRT_LINK_STATUS_CHANGE pLsc, UINT32 ulDevId)
{
   BCM_XTM_DEBUG("DoLinkUp\n");

   if (pDevCtx->ulLinkState != pLsc->ulLinkState)
   {
      PBCMXTMRT_GLOBAL_INFO      pGi = &g_GlobalInfo;
      PXTMRT_TRANSMIT_QUEUE_ID   pTxQId;
      UINT32 ulChannel;
      UINT16 bufStatus = 0;
      int i;
      
      /* Initialize transmit DMA channel information. */
      pDevCtx->ucTxVcid       = pLsc->ucTxVcid;
      pDevCtx->ulLinkState    = pLsc->ulLinkState;
      pDevCtx->ulTxQInfosSize = 0;

      /* Mcast driver requires to obtain tx_wan_flow_id from
       * netdev->hw_tx_port for creating proper TX info */
      bcmxtmrt_get_bufStatus(pDevCtx, 0, &bufStatus);
      ulChannel = ((UINT32)bufStatus & FSTAT_CT_MASK) >> FSTAT_CT_SHIFT;
      ulChannel += MAX_TRANSMIT_QUEUES * pDevCtx->ucTxVcid;
      netdev_path_set_hw_port_only(pDevCtx->pDev, ulDevId);
      netdev_path_set_hw_tx_port_only(pDevCtx->pDev, ulChannel);

      /* Use each Rx vcid as an index into an array of bcmxtmrt devices
       * context structures.
       */
      for (i = 0; i < pLsc->ulRxVcidsSize; i++)
      {
         pGi->pDevCtxsByMatchId[pLsc->ucRxVcids[i]] = pDevCtx;
         pGi->ulDevCtxMask |= (1 << pLsc->ucRxVcids[i]);
         bcmxapi_XtmLinkUp(ulDevId, pLsc->ucRxVcids[i]);
      }

      for (i = 0, pTxQId = pLsc->TransmitQueueIds;
           i < pLsc->ulTransmitQueueIdsSize;
           i++, pTxQId++)
      {
         if (bcmxapi_DoSetTxQueue(pDevCtx, pTxQId) != 0)
         {
            pDevCtx->ulTxQInfosSize = 0;
            return -ENOMEM;
         }
      } /* for i */

      /* If it is not already there, put the driver into a "ready to send and
       * receive state".
       */
      printk (CARDNAME ": netif_carrier_on \n") ;
      netif_carrier_on(pDevCtx->pDev);

      if (pGi->ulDrvState == XTMRT_INITIALIZED)
      {
         //printk (CARDNAME ": add_timer \n") ;
         pGi->ulDrvState = XTMRT_RUNNING;

         pGi->Timer.expires = jiffies + SAR_TIMEOUT;
         add_timer(&pGi->Timer);

         if (pDevCtx->ulOpenState == XTMRT_DEV_OPENED)
            netif_start_queue(pDevCtx->pDev);
      }
   }
   pDevCtx->ulPortDataMask = pLsc->ulLinkDataMask;

   return 0;
    
}  /* DoLinkUp() */


/*---------------------------------------------------------------------------
 * int DoLinkDownRx(UINT32 ulPortId)
 * Description:
 *    Processes a "link down" condition for receive only.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoLinkDownRx(UINT32 ulPortId)
{
   int nRet = 0;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 i, ulStopRunning;

   BCM_XTM_DEBUG("DoLinkDownRx\n");

   /* If all links are down, put the driver into an "initialized" state. */
   for (i = 0, ulStopRunning = 1; i < MAX_DEV_CTXS; i++)
   {
      if (pGi->pDevCtxs[i])
      {
         PBCMXTMRT_DEV_CONTEXT pDevCtx = pGi->pDevCtxs[i];
         UINT32 ulDevPortId = pDevCtx->ulPortDataMask;
         if ((ulDevPortId & ~ulPortId) != 0)
         {
            /* At least one link that uses a different port is up.
             * For Ex., in bonding case, one of the links can be up
             */
            ulStopRunning = 0;
            break;
         }
      }
   }

   if (ulStopRunning)
   {
      pGi->ulDrvState = XTMRT_INITIALIZED;

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
      /* Disable receive interrupts and stop the timer. */
      printk(CARDNAME ": D-RXIntr\n");
      bcmxapi_disable_rx_interrupt();
#endif

      /* Stop the timer. */
      del_timer_sync(&pGi->Timer);
   }

   return (nRet);
    
}  /* DoLinkDownRx() */


/*---------------------------------------------------------------------------
 * int DoLinkDownTx(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                  PXTMRT_LINK_STATUS_CHANGE pLsc)
 * Description:
 *    Processes a "link down" condition for transmit only.
 *    In bonding case, one of the links could still be UP, in which
 *    case only the link data status is updated.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoLinkDownTx(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                        PXTMRT_LINK_STATUS_CHANGE pLsc)
{
   int nRet = 0;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 i;

   BCM_XTM_DEBUG("DoLinkDownTx\n");

   if (pLsc->ulLinkDataMask == 0)
   {
      /* Disable transmit DMA. */
      pDevCtx->ulLinkState = LINK_DOWN;

#if defined(CONFIG_BLOG)
      /* Flush flows associated with the device */
      blog_notify_async_wait(UPDATE_NETDEVICE, pDevCtx->pDev, 0, 0);
#endif

      for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
         bcmxapi_ShutdownTxQueue(pDevCtx, pDevCtx->txdma[i]);
   
      /* Free memory used for txdma info - Apr 2010 */
      for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
      {
         if (pDevCtx->txdma[i])
         {
            kfree((void*)pDevCtx->txdma[i]);
            pDevCtx->txdma[i] = NULL;
         }
      }
      pDevCtx->ulTxQInfosSize = 0;

      if (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)
      {
         bcmxapi_SetPtmBonding(0);  /* clear bonding flag */
      }

      /* Zero out list of priorities - Apr 2010 */
      memset(pDevCtx->pTxPriorities, 0x00, sizeof(pDevCtx->pTxPriorities));
      /* Zero out pTxQids pointer array */
      memset(pDevCtx->pTxQids, 0x00, sizeof(pDevCtx->pTxQids));

      pDevCtx->pHighestPrio = NULL;
      pDevCtx->ucTxVcid     = INVALID_VCID;
//      pGi->ulNumTxBufsQdAll = 0;

      /* Zero receive vcids. */
      for (i = 0; i < MAX_MATCH_IDS; i++)
         if (pGi->pDevCtxsByMatchId[i] == pDevCtx)
         {
            pGi->pDevCtxsByMatchId[i] = NULL;
            pGi->ulDevCtxMask &= ~(1 << i);
         }
   }                                          
   else
   {
      /* flush out all the queues, as one of the ports, particularly in
       * bonding, could be down and all the data in the queues need to be
       * flushed out, as the data fragments might be destined for this down
       * port.
       */
      if ((pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) ||
          (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_ATM_BONDED))
      {
         for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
            bcmxapi_FlushdownTxQueue(pDevCtx, pDevCtx->txdma[i]);
      }
   }

   pDevCtx->ulPortDataMask = pLsc->ulLinkDataMask;

   return (nRet);
    
}  /* DoLinkDownTx() */


/*---------------------------------------------------------------------------
 * int DoUnsetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                    PXTMRT_TRANSMIT_QUEUE_ID pTxQId)
 * Description:
 *    Frees memory for a transmit queue.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoUnsetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                          PXTMRT_TRANSMIT_QUEUE_ID pTxQId)
{
   int nRet = -EINVAL;
   UINT32 i, j;
   BcmPktDma_XtmTxDma  *txdma;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   BCM_XTM_DEBUG("DoUnsetTxQueue\n");

   spin_lock_bh(&pGi->xtmlock_tx);

   for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
   {
      txdma = pDevCtx->txdma[i];

      if (txdma && pTxQId->ulQueueIndex == txdma->ulDmaIndex)
      {
         UINT32 ulPort = PORTID_TO_PORT(pTxQId->ulPortId);
         UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;

         bcmxapi_ShutdownTxQueue(pDevCtx, txdma);
   
         if ((pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM) ||
             (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))
            ulPtmPrioIdx = (txdma->ulPtmPriority == PTM_PRI_HIGH)? PTM_FLOW_PRI_HIGH : PTM_FLOW_PRI_LOW;

         pDevCtx->pTxPriorities[ulPtmPrioIdx][ulPort][txdma->ulSubPriority] = NULL;
         pDevCtx->pTxQids[pTxQId->ucQosQId] = NULL;

         if (pDevCtx->pHighestPrio == txdma)
            pDevCtx->pHighestPrio = NULL;

         /* Shift remaining array elements down by one element. */
         memmove(&pDevCtx->txdma[i], &pDevCtx->txdma[i + 1],
                 (pDevCtx->ulTxQInfosSize - i - 1) * sizeof(txdma));
         pDevCtx->ulTxQInfosSize--;

         kfree((void*)txdma);

         /* Find the highest subpriority dma */
         for (j = 0; j < pDevCtx->ulTxQInfosSize; j++)
         {
            txdma = pDevCtx->txdma[j];
            if (pDevCtx->pHighestPrio == NULL ||
                pDevCtx->pHighestPrio->ulSubPriority < txdma->ulSubPriority)
               pDevCtx->pHighestPrio = txdma;
         }

         nRet = 0 ;
         break;
      }
   }

   wmb() ;
   spin_unlock_bh(&pGi->xtmlock_tx);
   return (nRet);
    
}  /* DoUnsetTxQueue() */


/*---------------------------------------------------------------------------
 * int DoStopTxQueues(PBCMXTMRT_DEV_CONTEXT pDevCtx)
 * Description:
 *    Stop all transmit queues.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoStopTxQueues(PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
   int nRet = 0 ;
   UINT32 i;
   BcmPktDma_XtmTxDma  *txdma;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   BCM_XTM_DEBUG("DoStopTxQueues\n");

   spin_lock_bh(&pGi->xtmlock_tx);

   for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
   {
      txdma = pDevCtx->txdma[i];
      if (txdma)
         txdma->txEnabled = 0;
   }

   for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
   {
      txdma = pDevCtx->txdma[i];
      if (txdma)
      {
         //printk ("Stop Q %d \n", (unsigned int) txdma->ulDmaIndex);
         bcmxapi_StopTxQueue(pDevCtx, txdma);
      }
   } /* for i */

   spin_unlock_bh(&pGi->xtmlock_tx);
   return (nRet);
    
}  /* DoStopTxQueues() */

/*---------------------------------------------------------------------------
 * int DoStartTxQueues(PBCMXTMRT_DEV_CONTEXT pDevCtx)
 * Description:
 *    Start all transmit queues.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoStartTxQueues(PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
   int nRet = 0 ;
   UINT32 i;
   BcmPktDma_XtmTxDma  *txdma;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   BCM_XTM_DEBUG("DoStartTxQueues\n");

   spin_lock_bh(&pGi->xtmlock_tx);

   for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
   {
      txdma = pDevCtx->txdma[i];
      if (txdma)
      {
         //printk ("Start Q %d \n", (unsigned int) txdma->ulDmaIndex);
         bcmxapi_StartTxQueue(pDevCtx, txdma);
      }
   } /* for i */

   for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
   {
      txdma = pDevCtx->txdma[i];
      if (txdma)
         txdma->txEnabled = 1;
   }

   spin_unlock_bh(&pGi->xtmlock_tx);
   return (nRet);
    
}  /* DoStartTxQueues() */

/*---------------------------------------------------------------------------
 * int DoSendCellReq(PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC)
 * Description:
 *    Processes an XTMRT_CMD_SEND_CELL command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoSendCellReq(PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC)
{
   int nRet = 0;

   if (pDevCtx->ulLinkState == LINK_UP)
   {
      struct sk_buff *skb = dev_alloc_skb(CELL_PAYLOAD_SIZE);

      if (skb)
      {
         UINT32 i;
         UINT32 ulPort = PORTID_TO_PORT(pC->ConnAddr.u.Conn.ulPortMask) ;
         UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;

         /* A network device instance can potentially have transmit queues
          * on different ports. Find a transmit queue for the port specified
          * in the cell structure.  The cell structure should only specify
          * one port.
          */
         for (i = 0; i < MAX_SUB_PRIORITIES; i++)
         {
            if (pDevCtx->pTxPriorities[ulPtmPrioIdx][ulPort][i])
            {
               skb->mark = i;
               break;
            }
         }

         skb->dev = pDevCtx->pDev;
         __skb_put(skb, CELL_PAYLOAD_SIZE);
         memcpy(skb->data, pC->ucData, CELL_PAYLOAD_SIZE);

         switch (pC->ucCircuitType)
         {
         case CTYPE_OAM_F5_SEGMENT:
            skb->protocol = htons(FSTAT_CT_OAM_F5_SEG);
            break;

         case CTYPE_OAM_F5_END_TO_END:
            skb->protocol = htons(FSTAT_CT_OAM_F5_E2E);
            break;

         case CTYPE_OAM_F4_SEGMENT:
            skb->protocol = htons(FSTAT_CT_OAM_F4_SEG);
            break;

         case CTYPE_OAM_F4_END_TO_END:
            skb->protocol = htons(FSTAT_CT_OAM_F4_E2E);
            break;

         case CTYPE_ASM_P0:
            skb->protocol = htons(FSTAT_CT_ASM_P0);
            break;

         case CTYPE_ASM_P1:
            skb->protocol = htons(FSTAT_CT_ASM_P1);
            break;

         case CTYPE_ASM_P2:
            skb->protocol = htons(FSTAT_CT_ASM_P2);
            break;

         case CTYPE_ASM_P3:
            skb->protocol = htons(FSTAT_CT_ASM_P3);
            break;
         }

         skb->protocol |= htons(SKB_PROTO_ATM_CELL);

         bcmxtmrt_xmit(SKBUFF_2_PNBUFF(skb), pDevCtx->pDev);
      }
      else
        nRet = -ENOMEM;
   }
   else
      nRet = -EPERM;

   return (nRet);
    
}  /* DoSendCellReq() */


/*---------------------------------------------------------------------------
 * int DoDeleteDeviceReq(PBCMXTMRT_DEV_CONTEXT pDevCtx)
 * Description:
 *    Processes an XTMRT_CMD_DELETE_DEVICE command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoDeleteDeviceReq(PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
   int nRet = -EPERM;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 i;
   struct net_device *pDev = NULL ;

   BCM_XTM_DEBUG("DoDeleteDeviceReq\n");

   for (i = 0; i < MAX_MATCH_IDS; i++)
      if (pGi->pDevCtxsByMatchId[i] == pDevCtx)
         pGi->pDevCtxsByMatchId[i] = NULL;

   for (i = 0; i < MAX_DEV_CTXS; i++) {
      if (pGi->pDevCtxs[i] == pDevCtx)
      {

         //            kerSysReleaseMacAddress( pDevCtx->pDev->dev_addr );

         pDev = pDevCtx->pDev ;
         pDevCtx->pDev = NULL ;
         pGi->pDevCtxs[i] = NULL;
         nRet = 0;
         break;
      }
   }

   if (pDev) {
      unregister_netdev(pDev);
      free_netdev(pDev);
   }

   return (nRet);

}  /* DoDeleteDeviceReq() */


/*---------------------------------------------------------------------------
 * int DoGetNetDevTxChannel(PXTMRT_NETDEV_TXCHANNEL pParm)
 * Description:
 *    Processes an XTMRT_CMD_GET_NETDEV_TXCHANNEL command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoGetNetDevTxChannel(PXTMRT_NETDEV_TXCHANNEL pParm)
{
   int nRet = 0;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT pDevCtx;
   BcmPktDma_XtmTxDma   *txdma;
   UINT32 i, j;

   for (i = 0; i < MAX_DEV_CTXS; i++)
   {
      pDevCtx = pGi->pDevCtxs[i];
      if (pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL)
      {
         if (pDevCtx->ulOpenState == XTMRT_DEV_OPENED)
         {
            for (j = 0; j < pDevCtx->ulTxQInfosSize; j++)
            {
               txdma = pDevCtx->txdma[j];

               if (txdma->ulDmaIndex == pParm->txChannel)
               {
                  pParm->pDev = (void*)pDevCtx->pDev;
                  return nRet;
               }
            }
         }
      }
   }
   return -EEXIST;
    
}  /* DoGetNetDevTxChannel() */


/*---------------------------------------------------------------------------
 * int DoTogglePortDataStatusReq(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                               PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE pTpdsc)
 * Description:
 *    Processes an XTMRT_CMD_TOGGLE_PORT_DATA_STATUS_CHANGE command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoTogglePortDataStatusReq(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                                     PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE pTpdsc)
{
   UINT32 i ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;

   local_bh_disable();
   for( i = 0; i < MAX_DEV_CTXS; i++ )
   {
      pDevCtx  = pGi->pDevCtxs [i] ;

      if ((pDevCtx != NULL) && (pDevCtx->ulHdrType == HT_PTM)) {

         spin_lock(&pGi->xtmlock_tx);
         /* For the US direction */
         if ((pTpdsc->ulPortDataUsStatus == XTMRT_CMD_PORT_DATA_STATUS_DISABLED)
					 ||
             (pTpdsc->ulPortDataDsStatus == XTMRT_CMD_PORT_DATA_STATUS_DISABLED))
            pDevCtx->ulPortDataMask &= ~(0x1 << pTpdsc->ulPortId) ;
         else
            pDevCtx->ulPortDataMask |= (0x1 << pTpdsc->ulPortId) ;
			bcmxtmrt_ptmbond_calculate_link_parameters(&pDevCtx->ulLinkUsRate[0],
					                                  pDevCtx->ulPortDataMask, 1);
         spin_unlock(&pGi->xtmlock_tx);
         break ;
      }
   }

   local_bh_enable();
   return (0) ;

}  /* DoTogglePortDataStatusReq() */


/*---------------------------------------------------------------------------
 * int DoSetTxPortShaperInfo (PXTMRT_PORT_SHAPER_INFO pShaperInfo)
 * Description:
 *    Processes an XTMRT_CMD_SET_TX_PORT_SHAPER_INFO command.
 *    This can be called to set as well as unset the overall port shaping
 *    feature on the tx side.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int DoSetTxPortShaperInfo (PXTMRT_PORT_SHAPER_INFO pShaperInfo)
{
   int nRet = 0;

   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   nRet = bcmxapi_SetTxPortShaperInfo (pGi, pShaperInfo) ;

   return (nRet);

}  /* DoSetTxPortShaperInfo() */


EXPORT_SYMBOL(bcmxtmrt_request);

