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
 * File Name  : bcmxtmrt.c
 *
 * Description: This file implements BCM63x68 ATM/PTM network device driver
 *              runtime processing - sending and receiving data.
 ***************************************************************************/

/* Includes. */

//#define DUMP_DATA

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
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
#include <linux/if_vlan.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/blog.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ip.h>
#include <linux/if_pppox.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include "bcmnet.h"
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include <linux/kthread.h>
#include <linux/bcm_realtime.h>
#include <linux/bcm_atmdev.h>
#include "bcmxtmrtimpl.h"
#if (defined(CONFIG_BCM963178)) && defined(CONFIG_BCM_ARCHER)
#include "xtmrt_archer.h"
#else
#include "xtmrt_runner.h"
#endif

static UINT32 bcmxtmrt_pad (pNBuff_t pNBuf, UINT8 **ppData, UINT32 len, UINT32 isAtmCell, UINT32 hdrType, UINT32 ulTrafficType) ;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include <linux/bcm_log.h>
#include "spdsvc_defs.h"
static bcmFun_t *xtmrt_spdsvc_transmit = NULL;
static int speed_service_check (pNBuff_t pNBuf, UINT32 phyOverhead, struct net_device *dev, uint32_t *spdsvc_ctx) ;
#endif
#include "board.h"
#include <linux/bcm_colors.h>

/* VLAN TPIDs that need to be checked
   ETH_P_8021Q  0x8100
   ETH_P_8021AD 0x88A8
   ETH_P_QINQ1  0x9100
   ETH_P_QINQ2  0x9200
 */
#define BCM_VLAN_TPID_CHECK(x) ( (x) == htons(ETH_P_8021Q) \
                                || (x) == htons(ETH_P_8021AD)  \
                             /* || (x) == htons(ETH_P_QINQ1) */\
                             /* || (x) == htons(ETH_P_QINQ2) */)

#define VER_STR   "v" XTMRT_VERSION


/**** Externs ****/



/**** Globals ****/
BCMXTMRT_GLOBAL_INFO g_GlobalInfo;


/**** Prototypes ****/
static int __init bcmxtmrt_init(void);
static void bcmxtmrt_cleanup(void);
#ifdef CONFIG_PPP
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg);
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan,struct sk_buff *skb);
#endif
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc);
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc);
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb);
static PBCMXTMRT_DEV_CONTEXT FindDevCtx(short vpi, int vci);

static pNBuff_t AddRfc2684Hdr(pNBuff_t pNBuf, UINT32 hdrType);
static void MirrorPacket(char *mirrorIntf, pNBuff_t pNBuf);

#ifdef CONFIG_BLOG
extern int bcm_tcp_v4_recv(pNBuff_t pNBuff, BlogFcArgs_t *fc_args);
#endif

static int bcm63xx_xtmrt_rx_thread(void *arg);

//Proc files related definitions.
struct proc_dir_entry *xtm_dir;
static int add_proc_dir(void);
static int del_proc_dir(void);

/**** Statics ****/

#ifdef CONFIG_PPP
static struct atm_ioctl g_PppoAtmIoctlOps =
   {
      .ioctl      = bcmxtmrt_atm_ioctl,
   };
static struct ppp_channel_ops g_PppoAtmOps =
   {
      .start_xmit = bcmxtmrt_pppoatm_send
   };
#endif
static const struct atmdev_ops g_AtmDevOps =
   {
      .open       = bcmxtmrt_atmdev_open,
      .close      = bcmxtmrt_atmdev_close,
      .send       = bcmxtmrt_atmdev_send,
   };
    
static UINT32 gs_ulLogPorts[] = {0, 1, 2, 3};

#define PORT_PHYS_TO_LOG(PP) gs_ulLogPorts[PP]

#if defined(BCM_XTM_LOG)
static unsigned int data_path_dbg = 0;
unsigned int rx_path_debug   = 0;
unsigned int tx_path_debug   = 0;
unsigned int rx_data_dump    = 0;
unsigned int tx_data_dump    = 0;
unsigned int xtm_link_debug  = 0;
static int data_path_dbg_show(struct seq_file *s, void *v)
{
   
   BCM_XTM_NOTICE("RX path debug mask 0x0001"); 
   BCM_XTM_NOTICE("TX path debug mask 0x0002"); 
   BCM_XTM_NOTICE("RX data dump mask 0x0004"); 
   BCM_XTM_NOTICE("TX data dump mask 0x0008"); 
   BCM_XTM_NOTICE("Link debug mask 0x0010"); 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   seq_printf(s, "0x%08x\n", data_path_dbg);
   return 0;
#else
   return seq_printf(s, "0x%08x\n", data_path_dbg);
#endif
}

static int data_path_dbg_open(struct inode *inode, struct file *file)
{
   return single_open(file, data_path_dbg_show, NULL);
}

static ssize_t data_path_dbg_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
   unsigned int val = 0;
   ssize_t ret;

   ret = kstrtouint_from_user(buf, len, 16, &val);
   if(ret < 0)
      return ret;

   data_path_dbg = val;

   BCM_XTM_NOTICE("data_path_dbg:%08x",data_path_dbg); 
   rx_path_debug  = (data_path_dbg & RX_PATH_DBG_BITMASK)?1:0;
   tx_path_debug  = (data_path_dbg & TX_PATH_DBG_BITMASK)?1:0;
   rx_data_dump   = (data_path_dbg & RX_DATA_DUMP_BITMASK)?1:0;
   tx_data_dump   = (data_path_dbg & TX_DATA_DUMP_BITMASK)?1:0;
   xtm_link_debug = (data_path_dbg & XTM_LINK_DBG_BITMASK)?1:0;
   return len;
}

static const struct file_operations data_path_dbg_proc_fops = {
   .open = data_path_dbg_open,
   .read = seq_read,
   .llseek = seq_lseek,
   .write = data_path_dbg_write,
   .release = single_release
};

#endif
/*---------------------------------------------------------------------------
 * int add_proc_dir()
 * Description:
 *    Adds xtm directory in proc file system for further entries.
 * Returns:
 *    0 if successful or error status
 *--------------------------------------------------------------------------
 */
static int add_proc_dir()
{
   struct proc_dir_entry *entry;
   xtm_dir = proc_mkdir("driver/xtm", NULL);
#if defined(BCM_XTM_LOG)
   //Create proc entries for RX  & TX debug enables and data dump
   //functionality.
   entry = proc_create("data_path_dbg", 0440, xtm_dir, &data_path_dbg_proc_fops);
   if(!entry) {
      pr_err("%s: could not create proc entry for data_path_dbg",__func__);
      return -1;
   }
#endif
   return 0;
}

static int del_proc_dir()
{
#if defined(BCM_XTM_LOG)
   remove_proc_entry("data_path_dbg",xtm_dir);
#endif
   proc_remove(xtm_dir);
   xtm_dir = NULL;
   return 0;
}
/*---------------------------------------------------------------------------
 * int bcmxtmrt_init(void)
 * Description:
 *    Called when the driver is loaded.
 * Returns:
 *    0 or error status
 *---------------------------------------------------------------------------
 */
static int __init bcmxtmrt_init(void)
{
   UINT16 usChipId  = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
   UINT16 usChipRev = (PERF->RevID & REV_ID_MASK);

#if !defined(NO_XDSL_MODULE)
   if (!kerSysGetDslPhyEnable())
   {
       printk(CLRyr "bcmxtmrt : DSL PHY disabled, ATM/PTM Network Device not created\n" CLRnl);
       return -1;
   }
#endif

   printk(CARDNAME ": Broadcom BCM%X%X ATM/PTM Network Device ",
          usChipId, usChipRev);
   printk(VER_STR "\n");

   memset(&g_GlobalInfo, 0x00, sizeof(g_GlobalInfo));

   g_GlobalInfo.ulChipRev = PERF->RevID;

   //Set Loglevel for XTM (RT & CFG drivers)
   bcmLog_setLogLevel(BCM_LOG_ID_XTM, BCM_LOG_LEVEL_NOTICE);

#ifdef CONFIG_PPP
   register_atm_ioctl(&g_PppoAtmIoctlOps);
#endif
   
   g_GlobalInfo.pAtmDev = atm_dev_register("bcmxtmrt_atmdev", NULL,
                                           &g_AtmDevOps, -1, NULL);
   if (g_GlobalInfo.pAtmDev)
   {
      g_GlobalInfo.pAtmDev->ci_range.vpi_bits = 12;
      g_GlobalInfo.pAtmDev->ci_range.vci_bits = 16;
   }
  
   if(add_proc_dir() != 0)
   {
      BCM_XTM_ERROR("Failed to create XTM proc dir");
      return -1;
   }
   bcmxapi_add_proc_files(xtm_dir);
   bcmxapi_module_init();

   /* Create a thread to do the rx processing work. */
   g_GlobalInfo.rx_work_avail = 0;
   init_waitqueue_head(&g_GlobalInfo.rx_thread_wqh);
   g_GlobalInfo.rx_thread = kthread_create(bcm63xx_xtmrt_rx_thread, NULL, "bcmxtm_rx");

   wake_up_process(g_GlobalInfo.rx_thread);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
   xtmrt_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
   BCM_ASSERT(xtmrt_spdsvc_transmit != NULL);
#endif

   return 0;
    
}  /* bcmxtmrt_init() */


/*---------------------------------------------------------------------------
 * void bcmxtmrt_cleanup(void)
 * Description:
 *    Called when the driver is unloaded.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void bcmxtmrt_cleanup(void)
{
   /* Stop RX thread first so it won't touch anything being deallocated */
   kthread_stop(g_GlobalInfo.rx_thread);

   /* Continue cleanup - ATM stuff */
#ifdef CONFIG_PPP
   deregister_atm_ioctl(&g_PppoAtmIoctlOps);
#endif
   if (g_GlobalInfo.pAtmDev)
   {
      atm_dev_deregister(g_GlobalInfo.pAtmDev);
      g_GlobalInfo.pAtmDev = NULL;
   }
    
   /* Finally, get rid of temp files and do API cleanup */
   bcmxapi_del_proc_files(xtm_dir);
   del_proc_dir();
   bcmxapi_module_cleanup();
    

}  /* bcmxtmrt_cleanup() */


#if defined(CONFIG_PPP)
/*---------------------------------------------------------------------------
 * int bcmxtmrt_atm_ioctl(struct socket *sock, UINT32 cmd, UINT32 arg)
 * Description:
 *    Driver atm ioctl entry point.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
    int nRet = -ENOIOCTLCMD;
   PBCMXTMRT_DEV_CONTEXT pDevCtx;
   struct atm_vcc *pAtmVcc = ATM_SD(sock);
   void __user *argp = (void __user *)arg;
   atm_backend_t b;

   BCM_XTM_DEBUG("XTM: vpi:%u vci:%u",pAtmVcc->vpi, pAtmVcc->vci);
   switch (cmd)
   {
   case ATM_SETBACKEND:
      if (get_user(b, (atm_backend_t __user *) argp) == 0)
      {
         switch (b)
         {
         case ATM_BACKEND_PPP_BCM:
            if ((pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL &&
                pDevCtx->Chan.private == NULL)
            {
               pDevCtx->Chan.private = pDevCtx->pDev;
               pDevCtx->Chan.ops     = &g_PppoAtmOps;
               pDevCtx->Chan.mtu     = 1500; /* TBD. Calc value. */
               pAtmVcc->user_back    = pDevCtx;
               if (ppp_register_channel(&pDevCtx->Chan) == 0)
                  nRet = 0;
               else
               {
                  BCM_XTM_ERROR("XTM:Failed to register PPP channel");
                  nRet = -EFAULT;
               }
            }
            else
            {
               nRet = (pDevCtx) ? 0 : -EFAULT;
               if(nRet != 0)
                  BCM_XTM_ERROR("XTM: Failed to find device context for this PVC");
            }
            break;

         case ATM_BACKEND_PPP_BCM_DISCONN:
            /* This is a patch for PPP reconnection.
             * ppp daemon wants us to send out an LCP termination request
             * to let the BRAS ppp server terminate the old ppp connection.
             */
            if ((pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
            {
               struct sk_buff *skb;
               int size = 6;
               int eff  = (size+3) & ~3; /* align to word boundary */

               while (!(skb = alloc_skb(eff, GFP_ATOMIC)))
                  schedule();

               skb->dev = NULL; /* for paths shared with net_device interfaces */
               skb_put(skb, size);

               skb->data[0] = 0xc0;  /* PPP_LCP == 0xc021 */
               skb->data[1] = 0x21;
               skb->data[2] = 0x05;  /* TERMREQ == 5 */
               skb->data[3] = 0x02;  /* id == 2 */
               skb->data[4] = 0x00;  /* HEADERLEN == 4 */
               skb->data[5] = 0x04;

               if (eff > size)
                  memset(skb->data+size,0,eff-size);

               nRet = bcmxtmrt_xmit(SKBUFF_2_PNBUFF(skb), pDevCtx->pDev);
            }
            else
               nRet = -EFAULT;
            break;

         case ATM_BACKEND_PPP_BCM_CLOSE_DEV:
            if ((pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
            {
               bcmxtmrt_pppoatm_send(&pDevCtx->Chan, NULL);
               ppp_unregister_channel(&pDevCtx->Chan);
               pDevCtx->Chan.private = NULL;
            }
            nRet = 0;
            break;

         default:
            break;
         }
      }
      else
         nRet = -EFAULT;
      break;

   case PPPIOCGCHAN:
      if ((pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
      {
         nRet = put_user(ppp_channel_index(&pDevCtx->Chan),
                         (int __user *)argp) ? -EFAULT : 0;
      }
      else
      {
         BCM_XTM_ERROR("XTM:PPPIOCGCHAN:Failed to find device context");
         nRet = -EFAULT;
      }
      break;

   case PPPIOCGUNIT:
      if ((pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
      {
         nRet = put_user(ppp_unit_number(&pDevCtx->Chan),
                         (int __user *)argp) ? -EFAULT : 0;
      }
      else
      {
         BCM_XTM_ERROR("XTM:PPPIOCGUNIT:Failed to find device context");
         nRet = -EFAULT;
      }
      break;
   default:
      break;
   }

   return (nRet);
    
}  /* bcmxtmrt_atm_ioctl() */
#endif     


/*---------------------------------------------------------------------------
 * int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc)
 * Description:
 *    ATM device open
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc)
{
   set_bit(ATM_VF_READY,&pVcc->flags);
   return 0;
   
}  /* bcmxtmrt_atmdev_open() */


/*---------------------------------------------------------------------------
 * void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc)
 * Description:
 *    ATM device open
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc)
{
   clear_bit(ATM_VF_READY,&pVcc->flags);
   clear_bit(ATM_VF_ADDR,&pVcc->flags);
   
}  /* bcmxtmrt_atmdev_close() */


/*---------------------------------------------------------------------------
 * int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb)
 * Description:
 *    send data
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb)
{
   int nRet;
   PBCMXTMRT_DEV_CONTEXT pDevCtx = FindDevCtx(pVcc->vpi, pVcc->vci);

   if (pDevCtx)
      nRet = bcmxtmrt_xmit(SKBUFF_2_PNBUFF(skb), pDevCtx->pDev);
   else
      nRet = -EIO;

    return (nRet);
    
}  /* bcmxtmrt_atmdev_send() */

#ifdef CONFIG_PPP
/*---------------------------------------------------------------------------
 * int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan, struct sk_buff *skb)
 * Description:
 *    Called by the PPP driver to send data.
 * Returns:
 *    1 if successful or error status
 *---------------------------------------------------------------------------
 */
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan, struct sk_buff *skb)
{
   if (skb)
      skb->dev = (struct net_device *)pChan->private;
   bcmxtmrt_xmit(SKBUFF_2_PNBUFF(skb), (struct net_device *)pChan->private);
   
   return 1;   /* Note: must return 1 for success */
   
}  /* bcmxtmrt_pppoatm_send() */
#endif

/*---------------------------------------------------------------------------
 * PBCMXTMRT_DEV_CONTEXT FindDevCtx(short vpi, int vci)
 * Description:
 *    Finds a device context structure for a VCC.
 * Returns:
 *    Pointer to a device context structure or NULL.
 *---------------------------------------------------------------------------
 */
static PBCMXTMRT_DEV_CONTEXT FindDevCtx(short vpi, int vci)
{
   PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
   PBCMXTMRT_GLOBAL_INFO pGi     = &g_GlobalInfo;
   UINT32 i;

   for (i = 0; i < MAX_DEV_CTXS; i++)
   {
      if ((pDevCtx = pGi->pDevCtxs[i]) != NULL)
      {
         if (pDevCtx->Addr.u.Vcc.usVpi == vpi &&
             pDevCtx->Addr.u.Vcc.usVci == vci)
         {
            break;
         }
         pDevCtx = NULL;
      }
   }
   BCM_XTM_DEBUG("vpi:%u, vci:%u, pDevCtx:%pk",vpi,vci,pDevCtx);
   return (pDevCtx);
    
}  /* FindDevCtx() */


/*---------------------------------------------------------------------------
 * BcmPktDma_XtmTxDma *find_xmit_channel(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                                       UINT32 uMark)
 * Description:
 *    Finds the XTM TX iudma channel based on the skb->mark.
 * Returns:
 *    pointer to txdma structure
 *---------------------------------------------------------------------------
 */
static inline BcmPktDma_XtmTxDma *find_xmit_channel(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                                                    UINT32 uMark)
{
   int   qid;
   BcmPktDma_XtmTxDma *txdma = NULL;
   qid = 0;
#ifdef CONFIG_NETFILTER
   /* See if this is a classified flow */
   if (SKBMARK_GET_FLOW_ID(uMark))
   {
      /* Non-zero flow id implies classified packet.
       * Find tx queue based on its qid.
       */
      /* For ATM classified packet,
       *   bit 3-0 of nfmark is the queue id (0 to 15).
       *   bit 4   of nfmark is the DSL latency, 0=PATH0, 1=PATH1
       *
       * For PTM classified packet,
       *   bit 2-0 of nfmark is the queue id (0 to 7).
       *   bit 3   of nfmark is the PTM priority, 0=LOW, 1=HIGH
       *   bit 4   of nfmark is the DSL latency, 0=PATH0, 1=PATH1
       */
      /* Classified packet. Find tx queue based on its queue id. */
      if ((pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM) ||
          (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))
      {
         /* For PTM, bit 2-0 of the 32-bit nfmark is the queue id. */
         qid = uMark & 0x7;
      }
      else
      {
         /* For ATM, bit 3-0 of the 32-bit nfmark is the queue id. */
         qid = uMark & 0xf;
      }
      txdma = pDevCtx->pTxQids[qid];
   }
   else
   {
      /* Flow id 0 implies unclassified packet.
       * Find tx queue based on its subpriority.
       */
      /* There are 2 types of unclassified packet flow.
       *   1) Protocol control packets originated locally from CPE.
       *      Such packets are marked the highest subpriority (7),
       *      and will be sent to the highest subpriority queue of
       *      the connection.
       *   2) Packets that do not match any QoS classification rule.
       *      Such packets will be sent to the first queue of matching
       *      subpriority.
       */
      /* For unclassified packet,
       *   bit 2-0 of nfmark is the subpriority (0 to 7).
       *   bit 3   of nfmark is the PTM priority, 0=LOW, 1=HIGH
       *   bit 4   of nfmark is the DSL latency, 0=PATH0, 1=PATH1
       */

      UINT32 subprio = (uMark & 0x7);
      
      /* If the subpriority is the highest (7), use the existing
       * highest priority queue.
       */
      if (subprio == 0x7)
         txdma = pDevCtx->pHighestPrio;
      else
      {
        
         /* Find the txdma channel configured with the subpriority */
         for (qid = 0; qid < MAX_TRANSMIT_QUEUES; qid++)
         {
            if (pDevCtx->pTxQids[qid] && (subprio == pDevCtx->pTxQids[qid]->ulSubPriority))
            {
               txdma = pDevCtx->pTxQids[qid];
               break;
            }
         }
      }
   }
#endif

   BCM_XTM_TX_DEBUG("uMark:%08x qid:%u ",uMark,qid);
   /* If a transmit queue was not found or the queue was disabled,
    * use the default queue (qid 0).
    */
   if (txdma == NULL || txdma->txEnabled == 0) {
      BCM_XTM_TX_DEBUG("Queue not available or enabled. Sending in default Queue");
      txdma = pDevCtx->txdma[0]; /* the default queue */
   }

   return txdma;
    
}  /* find_xmit_channel() */


#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
static int speed_service_check (pNBuff_t pNBuf, UINT32 phyOverhead, struct net_device *dev, uint32_t *spdsvc_ctx)
{
   spdsvcHook_transmit_t spdsvc_transmit = {};
   int rc;

   spdsvc_transmit.pNBuff = pNBuf;
#if defined(CONFIG_BCM_ARCHER)
   spdsvc_transmit.dev = dev;
#else
   spdsvc_transmit.dev = NULL;
#endif
   spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
   spdsvc_transmit.phy_overhead = phyOverhead;
   spdsvc_transmit.so_mark = 0;

   rc = xtmrt_spdsvc_transmit(&spdsvc_transmit);
   *spdsvc_ctx = spdsvc_transmit.so_mark;

   return rc;
}

#else

#define speed_service_check(_pNBuf, _phyOverhead, _dev, _ctx)      0

#endif

static UINT32 bcmxtmrt_pad (pNBuff_t pNBuf, UINT8 **ppData, UINT32 len, UINT32 isAtmCell, UINT32 hdrType, UINT32 trafficType)
{

/* Skip padding for 138/148 platforms as a whole except for when the traffic type is PTM bonded.
** Pad packets for other platforms for shorter packets which are ethernet
** compatible ones.
**/

// #if !defined (CONFIG_BCM963158) /* No padding required for 158 onwards platforms due to short packet support. Currently DEFERRED. */

#if !defined (CONFIG_BCM963138) && !defined (CONFIG_BCM963148) /* RDP FW pads for these platform so we save extra CPU cycles for this path */
   if (len < ETH_ZLEN &&
       !isAtmCell &&
       (hdrType == HT_PTM ||
        hdrType == HT_LLC_SNAP_ETHERNET ||
        hdrType == HT_VC_MUX_ETHERNET))
      goto _doPad ;
   else
      goto _doNoPad ;
#else
      /* PTM bonded fragments cant go less than MinFragSize which is 64 per
      ** standards. */
   if ( (trafficType == TRAFFIC_TYPE_PTM_BONDED) &&
        (len < ETH_ZLEN) )
      goto _doPad ;
   else 
      goto _doNoPad ;
#endif

_doPad :
   nbuff_pad(pNBuf, ETH_ZLEN - len) ; /* Pad the buffer at the end, as SAR packet mode can not handle packets of lesser
                                      * size than the MinSize */
   nbuff_set_len (pNBuf, ETH_ZLEN) ;
   nbuff_get_context(pNBuf, ppData, (uint32_t *)&len) ;

_doNoPad :
   return (len) ;
}

/*---------------------------------------------------------------------------
 * int bcmxtmrt_xmit(pNBuff_t pNBuf, struct net_device *dev)
 * Description:
 *    Check for transmitted packets to free and, if skb is
 *    non-NULL, transmit a packet. Transmit may be invoked for
 *    a packet originating from the network stack or from a
 *    packet received from another interface.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxtmrt_xmit(pNBuff_t pNBuf, struct net_device *dev)
{
   int rc = -EIO;
   
   PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
   spinlock_t           *xtmlock = &g_GlobalInfo.xtmlock_tx;
   UINT16 bufStatus;
   UINT8  *pData;
   UINT32 len, skbMark, skbPrio;
   UINT32 hdrType             = pDevCtx->ulHdrType;
   UINT32 rfc2684Type         = RFC2684_NONE;
   UINT32 isAtmCell           = 0;
   UINT32 txAvailable         = 0;
   struct sk_buff     *skb    = NULL; /* If pNBuf is sk_buff: protocol access */
   BcmPktDma_XtmTxDma *txdma  = NULL;
   int is_spdsvc_setup_packet = 0 ;
   uint32_t spdsvc_so_mark = 0;
   UINT32 *pMark = NULL;
   spin_lock_bh(xtmlock);

   if (pDevCtx->ulLinkState != LINK_UP)
   {
      BCM_XTM_TX_DEBUG("LinkState not up Dropping");
      goto tx_drop;
   }
   
   /* Free packets that have been transmitted. */
   bcmxapi_free_xmit_packets(pDevCtx);

   if (nbuff_get_params(pNBuf, &pData, (uint32_t *)&len, (uint32_t *)&skbMark,
                        (uint32_t *)&skbPrio) == (void *)NULL)
   {
      /* pNBuf must be NULL. This call is for freeing the xmit packets. */
      goto unlock_done_xmit;
   }

   if (pDevCtx->ulTxQInfosSize == 0)
   {
      BCM_XTM_INFO("TxQInfoSize is 0 Dropping");
      goto tx_drop;
   }
   
   if (IS_SKBUFF_PTR(pNBuf))
   {
      skb = PNBUFF_2_SKBUFF(pNBuf);
      pMark = (UINT32 *)&(skb->mark);

      if ((skb->protocol & htons(~FSTAT_CT_MASK)) == htons(SKB_PROTO_ATM_CELL) &&
          (pDevCtx->Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM)
      {
         isAtmCell = 1;
         
         /* Give the highest possible priority to oam cells */
         skbMark |= 0x7;
      }
      else 
      {
          int hiPrioFlag=0;
          check_arp_lcp_pkt(skb->data, hiPrioFlag);
          if (hiPrioFlag)
          {
              /* Give the highest possible priority to ARP and LCP packets */
              skbMark |= 0x7;
          }
      }
#ifdef CONFIG_BLOG
      if (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)
         bcmxapi_blog_ptm_us_bonding (pDevCtx->ulTxPafEnabled, skb) ;
#endif

   }

   BCM_XTM_TX_DEBUG("XTM TX: pNBuf<0x%08x> skb<0x%08x> pData<0x%08x>", pNBuf,skb,pData);

   /* Find the transmit queue to send on. */
   txdma = find_xmit_channel(pDevCtx, skbMark);

   if (txdma && txdma->txEnabled == 1)
   {
       txAvailable = bcmxapi_xmit_available(txdma, skbMark);
   }

   if (!txAvailable || !bcmxapi_queue_packet(txdma, isAtmCell))
   {
      /* Transmit queue is full.  Free the socket buffer.  Don't call
       * netif_stop_queue because this device may use more than one
       * queue.
       */
      BCM_XTM_INFO("TxQueue Full Dropping");
      goto tx_drop;
   }
   
   if (!(pDevCtx->ulFlags & CNI_HW_ADD_HEADER) && HT_LEN(hdrType) && !isAtmCell)
   {
      rfc2684Type = HT_TYPE(hdrType);
   }
   
   /* Calculate bufStatus for the packet */
   if (((pDevCtx->Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM) &&
       (isAtmCell))
   {
      bufStatus = (htons(skb->protocol) & FSTAT_CT_MASK);
   }
   else
   {
      bufStatus = 0;
   }

   bcmxtmrt_get_bufStatus(pDevCtx, isAtmCell, &bufStatus);
   BCM_XTM_TX_DEBUG("isAtmCell:%u, bufStatus:%08x",isAtmCell,bufStatus);

#ifdef CONFIG_BLOG
   {
       BlogAction_t blog_action;
       spin_unlock_bh(xtmlock);
       blog_action = bcmxapi_blog_emit (pNBuf, dev, pDevCtx, txdma, rfc2684Type, bufStatus) ;
       spin_lock_bh(xtmlock);

       if (blog_action == PKT_DROP)
            goto tx_drop;
   }
#endif

   if (pDevCtx->szMirrorIntfOut[0] != '\0' &&
       !isAtmCell &&
       (hdrType == HT_PTM ||
        hdrType == HT_LLC_SNAP_ETHERNET ||
        hdrType == HT_VC_MUX_ETHERNET))
   {
      spin_unlock_bh(xtmlock);
      MirrorPacket(pDevCtx->szMirrorIntfOut, pNBuf);
      spin_lock_bh(xtmlock);
   }

   if (rfc2684Type)
   {
      pNBuf = AddRfc2684Hdr(pNBuf, hdrType);
      nbuff_get_context(pNBuf, &pData, (uint32_t *)&len);
   }
   
   /* pNBuf may have been changed by AddRfc2684Hdr. Update pData and len.  */
   // For Archer platforms its already taken care in its IUDMA driver.
   if (!(pDevCtx->ulFlags & CNI_HW_PAD))
      len = bcmxtmrt_pad (pNBuf, &pData, len, isAtmCell, hdrType, pDevCtx->ulTrafficType) ;
   else {
#if 0 /* Test */
   if (!isAtmCell) {
      len = (len <=62) ? len-20 : len;
      nbuff_set_len (pNBuf, len) ;
   }
#endif
   }

   if(unlikely(tx_data_dump == 1))
      DUMP_PKT(pData, len);
   /* Check Speed Service */

   if (!isAtmCell) {
      /* Speed test is applicable for when there is data traffic & not for cell traffic */
      is_spdsvc_setup_packet = speed_service_check (pNBuf, bcmxtmrt_xtmOverhead(hdrType, len, rfc2684Type, pDevCtx),
          dev, &spdsvc_so_mark) ;
   }

#if defined(CONFIG_BCM_ARCHER)
   if (is_spdsvc_setup_packet) {
      pDevCtx->DevStats.tx_dropped++;
      goto unlock_done_xmit ;
   }
#endif

   if ((pDevCtx->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)
                   &&
       (!(pDevCtx->ulFlags & CNI_HW_PTM_BONDING)))
   {
      /* bit 3 of the 32-bit nfmark is the PTM priority, 0=LOW, 1=HIGH */
      UINT32 ptmPrioIdx = (skbMark >> 3) & 0x1;

      BCM_XTM_DEBUG ("XTM TX: PTM bonding hdr addition Flags=%x \n", pDevCtx->ulFlags) ;

      if (pDevCtx->ulPortDataMask == 0 ||
          !bcmxtmrt_ptmbond_add_hdr(pDevCtx->ulTxPafEnabled, &pNBuf, ptmPrioIdx))
      {
         goto tx_drop;
      }

      nbuff_get_context(pNBuf, &pData, (uint32_t *)&len);

   } /* pNBuf may have been changed by ptm bonding add header. Update pData and len.  */

   BCM_XTM_DEBUG  ("XTM TX: Len=%d \n", len) ;
   rc = bcmxapi_xmit_packet(&pNBuf, &pData, &len, txdma, txdma->ulDmaIndex,
                            bufStatus, skbMark, is_spdsvc_setup_packet, &spdsvc_so_mark);
   if (rc)
   {
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148) || defined(CONFIG_BCM963158)
      pDevCtx->DevStats.tx_dropped++;
      goto unlock_done_xmit ;
#else
      goto tx_drop;
#endif
   }

   /* Gather statistics.  */
   pDevCtx->DevStats.tx_packets++;
   pDevCtx->DevStats.tx_bytes += len;

   /* Now, determine extended statistics.  Is this an Ethernet packet? */
   if (hdrType == HT_PTM ||
       hdrType == HT_LLC_SNAP_ETHERNET || 
       hdrType == HT_VC_MUX_ETHERNET)
   {
      /* Yes, this is Ethernet.  Test for multicast packet */
      if (pData[0]  == 0x01)
      {
         /* Multicast packet - record statistics */
         pDevCtx->DevStats.tx_multicast_packets++;
         pDevCtx->DevStats.tx_multicast_bytes += len;                    
      }

      /* Test for broadcast packet */
      if (pData[0] == 0xFF)
      {
         /* Constant value to test for Ethernet broadcast address */
         const unsigned char pucEtherBroadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      
         /* Low byte indicates we might be broadcast - check against the rest */
         if (memcmp(pData, pucEtherBroadcastAddr, 5) == 0)
         {
            /* Broadcast packet - record statistics */
            pDevCtx->DevStats.rx_broadcast_packets++;
         }
      }
   }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
   pDevCtx->pDev->trans_start = jiffies;
#endif
   goto unlock_done_xmit;

tx_drop:
   if (pNBuf)
   {
      nbuff_flushfree(pNBuf);
      pDevCtx->DevStats.tx_dropped++;
   }
      
unlock_done_xmit:
   spin_unlock_bh(xtmlock);

   return rc;
    
}  /* bcmxtmrt_xmit() */


/*---------------------------------------------------------------------------
 * pNBuff_t AddRfc2684Hdr(pNBuff_t pNBuf, UINT32 hdrType)
 * Description:
 *    Inserts the RFC2684 header to an ATM packet before transmitting it.
 *    Note that NBuf pointer may be modified by this function.
 * Returns:
 *    Modified NBuf pointer
 *---------------------------------------------------------------------------
 */
static pNBuff_t AddRfc2684Hdr(pNBuff_t pNBuf, UINT32 hdrType)
{
   UINT8 ucHdrs[][16] =
         {{},
         {0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00},
         {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00},
         {0xFE, 0xFE, 0x03, 0xCF},
         {0x00, 0x00}};
   int minheadroom = HT_LEN(hdrType);

   if (IS_SKBUFF_PTR(pNBuf))
   {
      struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuf);

      if (skb_headroom(skb) < minheadroom)
      {
         struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);

         if (skb2)
         {
            dev_kfree_skb_any(skb);
            skb = skb2;
         }
      }
      
      if (skb_headroom(skb) >= minheadroom)
      {
         UINT8 *pData = skb_push(skb, minheadroom);
         u16cpy(pData, ucHdrs[HT_TYPE(hdrType)], minheadroom);
      }
      else
      {
         BCM_XTM_ERROR(CARDNAME ": Failed to allocate SKB with enough headroom.");
      }
      pNBuf = SKBUFF_2_PNBUFF(skb);
   }
   else
   {
      struct fkbuff *fkb = PNBUFF_2_FKBUFF(pNBuf);
        
      if (fkb_headroom(fkb) >= minheadroom)
      {
         UINT8 *pData = fkb_push(fkb, minheadroom);
         u16cpy(pData, ucHdrs[HT_TYPE(hdrType)], minheadroom);
      }
      else
      {
         BCM_XTM_ERROR(CARDNAME ": FKB not enough headroom.");
      }
   }
   
   return pNBuf;
   
}  /* AddRfc2684Hdr() */


/*---------------------------------------------------------------------------
 * UINT32 bcmxtmrt_processRxPkt(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *rxdma,
 *                              pNBuff_t pNBuf, UINT16 bufStatus, UINT32 len, UINT32 flow_key)
 * Description:
 *    Processes a received packet.
 *    Responsible for sending the packet up to the blog and network stack.
 * Returns:
 *    Status as the packet thro BLOG/NORMAL path.
 *---------------------------------------------------------------------------
 */
UINT32 bcmxtmrt_processRxPkt(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *rxdma,
                             UINT8  *pData, UINT16 bufStatus, UINT32 len, UINT32 flow_key,
                             UINT8 is_mcast_fwd_exp)
{
   struct sk_buff *skb  = NULL;
   FkBuff_t       *pFkb = NULL;
   UINT32 hdrType       = pDevCtx->ulHdrType;
   UINT32 flags         = pDevCtx->ulFlags;
   UINT32 rfc2684_type  = RFC2684_NONE; /* blog.h: Rfc2684_t */
   UINT32 retStatus;
   UINT8  *pBuf;
   pNBuff_t pNBuf = NULL;
   int    delLen        = 0;
   int    trailerDelLen = 0;
#ifdef CONFIG_BLOG
   BlogFcArgs_t fcArgs;
   BlogAction_t blogAction;
#endif
#ifdef PHY_LOOPBACK
   char mac[6];
   char Ip[4];
   char port[2];
#endif

   pBuf  = pData;
   
   BCM_XTM_RX_DEBUG ("xtm rx");
   if(unlikely(rx_data_dump == 1))
      DUMP_PKT(pData, len);

#ifdef PHY_LOOPBACK
   memcpy(mac, pData, 6);
   memcpy(pData, pData+6, 6);
   memcpy(pData+6, mac, 6);
   memcpy(Ip, pData+30, 4);
   memcpy(pData+30, pData+26, 4);
   memcpy(pData+26, Ip, 4);
   memcpy(port, pData+36, 2);
   memcpy(pData+36, pData+34, 2);
   memcpy(pData+34, port, 2);
#endif

   BCM_XTM_RX_DEBUG("XTM_RX: bufStatus:%04x len:%u",bufStatus,len);
   if ((flags & LSC_RAW_ENET_MODE) != 0)
      len -= ETH_FCS_LEN;

   if (hdrType == HT_PTM && (flags & CNI_HW_REMOVE_TRAILER) == 0)
   {
      if (len > (ETH_FCS_LEN+XTMRT_PTM_CRC_SIZE))
      {
         trailerDelLen = (ETH_FCS_LEN+XTMRT_PTM_CRC_SIZE);
         len          -= trailerDelLen;
      }
   }

   if ((flags & CNI_HW_REMOVE_HEADER) == 0)
   {
      delLen = HT_LEN(hdrType);

      /* For PTM flow, this will not take effect and hence so, for
       * bonding flow as well. So we do not need checks here to not
       * make it happen.
       */
      if (delLen > 0)
      {
         pData += delLen;
         len   -= delLen;
      }
      
      /* cannot be an AtmCell, also do not use delLen (bonding), recompute */
      if (HT_LEN(hdrType) > 0)
         rfc2684_type = HT_TYPE(hdrType); /* blog.h: Rfc2684_t */
   }

   BCM_XTM_RX_DEBUG ("bcmxtmrt : Len = %d flags = %x ", (int) len, (unsigned int) flags ) ;

   if (len < ETH_ZLEN)
      len = ETH_ZLEN;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
   /* Record time of this RX */
   pDevCtx->pDev->last_rx = jiffies;
#endif
    
   /* Calculate total RX packets received */
   pDevCtx->DevStats.rx_packets++;
   pDevCtx->DevStats.rx_bytes += len;

   /* Now, determine extended statistics.  Is this an Ethernet packet? */
   if (hdrType == HT_PTM ||
       hdrType == HT_LLC_SNAP_ETHERNET ||
       hdrType == HT_VC_MUX_ETHERNET)
   {
      UINT8  bIsBcast = 0;  /* Flags a broadcast frame or packet */
      UINT8  bIsMcast = 0;  /* Flags a multicast frame or packet */        
      UINT16 *pusSessEthertypeId = (UINT16 *)(pData + ETH_ALEN + ETH_ALEN);
        
      /* Check for a multicast MAC address and increment multicast counters if so */
      if (is_multicast_ether_addr(pData))
      {
         /* Multicast Ethernet frame -- set flag */
         bIsMcast = 1;
      }         
        
      /* Test for broadcast MAC address */
      else if (pData[0] == 0xFF)
      {
         /* Low byte indicates we might be broadcast - check against the rest */
         if (is_broadcast_ether_addr(pData))
         {
            /* Broadcast Ethernet frame - set flag */
            bIsBcast = 1;
         }
      }      

      /* If the packet does not have a multicast or broadcast Ethernet address,  
         check PPPoE packets for the IP address as well.  */
      if (bIsBcast == 0 && bIsMcast == 0)
      {
         /* Now, examine the ethertype protocol ID and skip past any 802.1Q VLAN header
            tags (i.e. ethertype ETH_P_8021Q) and TCI tags (32 bits total) until
            we reach the inner packet */
         while (*pusSessEthertypeId == htons(ETH_P_8021Q))
             pusSessEthertypeId = pusSessEthertypeId + (VLAN_HLEN/sizeof(unsigned short));  

         /* Check to see if this is a packet encapsulated within PPPoE and a multicast
            by examining the ethertype of the packet (i.e. ethertype ETH_P_PPP_SES or 0x8864) */
         if (*pusSessEthertypeId == htons(ETH_P_PPP_SES))
         {
            /* Yes, this is PPPoE.  Get the destination IP address, which is 
               in an IP header that follows the dest MAC address and the src MAC address */
            struct iphdr *pPPPoEIpHdr = (struct iphdr *)(pusSessEthertypeId + HT_LEN(hdrType));
             
            /* Is destination IP address is multicast? */
            if (ipv4_is_multicast(pPPPoEIpHdr->daddr))
            {
               /* Multicast IP packet -- set flag */
               bIsMcast = 1;
            }
             
            /* Is IP address broadcast? */
            if (ipv4_is_lbcast(pPPPoEIpHdr->daddr))
            {
               /* Broadcast IP packet - set flag */
               bIsBcast = 1;
            }            
         }
      }

      /* Record statistics for multicast or broadcast packets */
      if (bIsMcast != 0)
      {
         /* Multicast packet - record statistics */
         pDevCtx->DevStats.multicast++;
         pDevCtx->DevStats.rx_multicast_bytes += len ;
      }   
      else if (bIsBcast != 0)
      {
         /* Broadcast packet - record statistics */
         pDevCtx->DevStats.rx_broadcast_packets++;
      }
   }

   /* Initialize the FKB context for the received packet. */
   pFkb = bcmxapi_fkb_qinit(pBuf, pData, len, rxdma);

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
   if (flow_key) 
        pFkb->fc_ctxt = flow_key;   
#endif

   /* convert pFkb to pNBuf */
   pNBuf = FKBUFF_2_PNBUFF(pFkb);

   if (pDevCtx->szMirrorIntfIn[0] != '\0' &&
       (hdrType == HT_PTM ||
        hdrType == HT_LLC_SNAP_ETHERNET ||
        hdrType == HT_VC_MUX_ETHERNET))
   {
      MirrorPacket(pDevCtx->szMirrorIntfIn, pNBuf);
   }

#ifdef CONFIG_BLOG
   fcArgs.mcast_fwd_exception = is_mcast_fwd_exp;
   blogAction = blog_finit_args(pFkb, pDevCtx->pDev, pDevCtx->ulEncapType,
                           (bufStatus & FSTAT_MATCH_ID_MASK),
                           BLOG_SET_PHYHDR(rfc2684_type, BLOG_XTMPHY),&fcArgs);
   
   if (blogAction == PKT_DONE)
   {
      retStatus = PACKET_BLOG ;
   }
   else if (blogAction == PKT_DROP)
   {
      if (rxdma)
         bcmxapi_rx_pkt_drop(rxdma, pBuf, len);
      else
         fkb_free(pFkb);

      pDevCtx->DevStats.rx_dropped++;
      retStatus = PACKET_NORMAL;
   }
   else if (blogAction == PKT_TCP4_LOCAL)
   {
      bcm_tcp_v4_recv((void *)CAST_REAL_TO_VIRT_PNBUFF(pFkb,FKBUFF_PTR),&fcArgs);
      retStatus = PACKET_BLOG;
   }
   else
#endif
   {
      /* Get an skb to return to the network stack. */
      skb = bcmxapi_skb_alloc(rxdma, pNBuf, delLen, trailerDelLen);
      if (skb == NULL)
      {
         fkb_free(pFkb);
         pDevCtx->DevStats.rx_dropped++;
      }
      if (skb)
      {
         skb->dev = pDevCtx->pDev;
         BCM_XTM_RX_DEBUG ("skb forward to net stack ") ;
         //DUMP_PKT(skb->data, skb->len);

         switch (hdrType)
         {
         case HT_LLC_SNAP_ROUTE_IP:
         case HT_VC_MUX_IPOA:
            /* IPoA */
            skb->protocol = htons(ETH_P_IP);
            skb_reset_mac_header(skb);
            /* Give the received packet to the network stack. */
            local_bh_disable();
            netif_receive_skb(skb);
            local_bh_enable();
            break;
#ifdef CONFIG_PPP
         case HT_LLC_ENCAPS_PPP:
         case HT_VC_MUX_PPPOA:
            /*PPPoA*/
            BCM_XTM_RX_DEBUG("XTM_RX:PPPoA Chan:%x",pDevCtx->Chan);
            ppp_input(&pDevCtx->Chan, skb);
            break;
#endif
         default:
            /* bridge, MER, PPPoE */
            skb->protocol = eth_type_trans(skb,pDevCtx->pDev);
            /* Give the received packet to the network stack. */
            local_bh_disable();
            netif_receive_skb(skb);
            local_bh_enable();
            break;
         }
      }

      retStatus = PACKET_NORMAL;
      BCM_XTM_RX_DEBUG("XTM_RX: retStatus:%u",retStatus);
   }
   return (retStatus);
   
}  /* bcmxtmrt_processRxPkt() */


/*---------------------------------------------------------------------------
 * void bcmxtmrt_processRxCell(UINT8 *pData)
 * Description:
 *    Processes a received cell.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxtmrt_processRxCell(UINT8 *pData)
{
   PBCMXTMRT_GLOBAL_INFO   pGi     = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT   pDevCtx = NULL;
   XTMRT_CELL              Cell;
   
   const UINT16 usOamF4VciSeg = 3;
   const UINT16 usOamF4VciEnd = 4;
   
   UINT8 ucCts[] = {0, 0, 0, 0, CTYPE_OAM_F5_SEGMENT, CTYPE_OAM_F5_END_TO_END,
                    0, 0, CTYPE_ASM_P0, CTYPE_ASM_P1, CTYPE_ASM_P2, CTYPE_ASM_P3,
                    CTYPE_OAM_F4_SEGMENT, CTYPE_OAM_F4_END_TO_END};
                    
   UINT8 ucCHdr   = *pData;
   UINT32 atmHdr  = ntohl(*((UINT32 *)(pData + sizeof(char))));
   if(unlikely(rx_data_dump == 1))
      DUMP_PKT(pData, CELL_SIZE) ;

   /* Fill in the XTMRT_CELL structure */
   Cell.ConnAddr.u.Vcc.usVpi = (UINT16)((atmHdr & ATM_CELL_HDR_VPI_MASK) >>
                                         ATM_CELL_HDR_VPI_SHIFT);

   if (pGi->atmBondSidMode == ATMBOND_ASM_MESSAGE_TYPE_NOSID)
   {
      Cell.ConnAddr.u.Vcc.usVci = (UINT16)((atmHdr & ATM_NON_BONDED_CELL_HDR_VCI_MASK) >>
                                            ATM_CELL_HDR_VCI_SHIFT);
   }
   else if ((pGi->atmBondSidMode == ATMBOND_ASM_MESSAGE_TYPE_12BITSID) ||
            (pGi->atmBondSidMode == ATMBOND_ASM_MESSAGE_TYPE_8BITSID))
   {
      Cell.ConnAddr.u.Vcc.usVci = (UINT16)((atmHdr & ATM_BONDED_CELL_HDR_VCI_MASK) >>
                                            ATM_CELL_HDR_VCI_SHIFT);
   }

   if ((Cell.ConnAddr.u.Vcc.usVpi == XTMRT_ATM_BOND_ASM_VPI) &&
       (Cell.ConnAddr.u.Vcc.usVci == XTMRT_ATM_BOND_ASM_VCI))
   {
      pDevCtx = pGi->pDevCtxs[0];
      memcpy(Cell.ucData, pData + sizeof(char), sizeof(Cell.ucData));
   }
   else
   {
      UINT8 ucLogPort;
   
      /* Possibly OAM Cell type */
      Cell.ConnAddr.ulTrafficType = TRAFFIC_TYPE_ATM;

      {
         /* Read port from hardware in nobonded mode */
         ucLogPort = PORT_PHYS_TO_LOG((ucCHdr & CHDR_PORT_MASK) >> CHDR_PORT_SHIFT);

         /* Get port mask form port number */
         Cell.ConnAddr.u.Vcc.ulPortMask = PORT_TO_PORTID(ucLogPort);

         /* Act depending on type of OAM */
         if (Cell.ConnAddr.u.Vcc.usVci == usOamF4VciSeg)
         {
            /* F4 segment OAM */
            ucCHdr  = CHDR_CT_OAM_F4_SEG;
            pDevCtx = pGi->pDevCtxs[0];
         }
         else
         {
            if (Cell.ConnAddr.u.Vcc.usVci == usOamF4VciEnd)
            {
               /* F4 end to end OAM */
               ucCHdr  = CHDR_CT_OAM_F4_E2E;
               pDevCtx = pGi->pDevCtxs[0];
            }
            else
            {
               /* Other OAM */
               pDevCtx = FindDevCtx((short)Cell.ConnAddr.u.Vcc.usVpi,
                                    (int)Cell.ConnAddr.u.Vcc.usVci);
            }
         }
         memcpy(Cell.ucData, pData + sizeof(char), sizeof(Cell.ucData));
      }
   }

   Cell.ucCircuitType = ucCts[(ucCHdr & CHDR_CT_MASK) >> CHDR_CT_SHIFT];

   if ((ucCHdr & CHDR_ERROR) == 0)
   {
      /* Call the registered OAM or ASM callback function. */
      switch (ucCHdr & CHDR_CT_MASK)
      {
      case CHDR_CT_OAM_F5_SEG:
      case CHDR_CT_OAM_F5_E2E:
      case CHDR_CT_OAM_F4_SEG:
      case CHDR_CT_OAM_F4_E2E:
         if (pGi->pfnOamHandler && pDevCtx)
         {
            BCM_XTM_INFO("bcmxtmrt : Rx OAM Cell %d ", (ucCHdr&CHDR_CT_MASK)) ;
            (*pGi->pfnOamHandler)((XTMRT_HANDLE)pDevCtx,
                                  XTMRTCB_CMD_CELL_RECEIVED, &Cell,
                                  pGi->pOamContext);
         }
         break;

      case CHDR_CT_ASM_P0:
      case CHDR_CT_ASM_P1:
      case CHDR_CT_ASM_P2:
      case CHDR_CT_ASM_P3:
         if (pGi->pfnAsmHandler && pDevCtx)
         {
            (*pGi->pfnAsmHandler)((XTMRT_HANDLE)pDevCtx,
                                  XTMRTCB_CMD_CELL_RECEIVED, &Cell,
                                  pGi->pAsmContext);
         }
         break;

      default:
         BCM_XTM_ERROR("bcmxtmrt : unknown cell type %x ", ucCHdr & CHDR_CT_MASK);
         break;
      }
   }
   else
   {
      if (pDevCtx)
         pDevCtx->DevStats.rx_errors++;
      BCM_XTM_ERROR("bcmxtmcfg : Cell Received in Error ");
   }
}  /* bcmxtmrt_processRxCell() */


/*---------------------------------------------------------------------------
 * void MirrorPacket(char *mirrorIntf, pNBuff_t pNBuf)
 * Description:
 *    This function sends a sent or received packet to a LAN port.
 *    The purpose is to allow packets sent and received on the WAN
 *    to be captured by a protocol analyzer on the Lan for debugging
 *    purposes.
 * Note: must keep pNBuf intact.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void MirrorPacket(char *mirrorIntf, pNBuff_t pNBuf)
{
   /* pNBuf is either SKBUFF or FKBUFF */
   struct net_device *netDev;
   struct sk_buff    *skb2;   /* mirror skb */
   UINT8  *data_p;
   int    len, size;
   unsigned long flags;
    
   if ((netDev = __dev_get_by_name(&init_net, mirrorIntf)) == NULL)
      return;
      
   if (IS_SKBUFF_PTR(pNBuf))
   {
      struct sk_buff *skbO;   /* Original skb */

      skbO = PNBUFF_2_SKBUFF(pNBuf);
      if (skbO == (struct sk_buff *)NULL)
      {
          BCM_XTM_ERROR(CARDNAME ": PNBUFF_2_SKBUFF failure");
          return;
      }

      data_p = skbO->data;
      len    = skbO->len;
   }
   else
   {
      FkBuff_t *fkbO_p;
      
      fkbO_p = PNBUFF_2_FKBUFF(pNBuf);
      if (fkbO_p == FKB_NULL)
      {
          BCM_XTM_ERROR(CARDNAME ": PNBUFF_2_FKBUFF failure");
          return;
      }

      data_p = fkbO_p->data;      
      len    = fkbO_p->len;
   }

   size = (len + 3) & ~3;  /* align to word boundary */
   
   skb2 = alloc_skb(size, GFP_ATOMIC);
   if (skb2 == (struct sk_buff *)NULL)
   {
       BCM_XTM_ERROR(CARDNAME ": alloc_skb failure");
       return;
   }
   skb_put(skb2, len);
   memcpy(skb2->data, data_p, len);

   /* pad with zero */      
   if (size > len)
      memset(skb2->data+len, 0, size-len);
   
   skb2->dev      = netDev;
   skb2->protocol = htons(ETH_P_802_3);
   local_irq_save(flags);
   local_irq_enable();
   dev_queue_xmit(skb2) ;
   local_irq_restore(flags);
   return;

}  /* MirrorPacket() */


/*---------------------------------------------------------------------------
 * void bcm63xx_xtmrt_rx_thread(void *arg)
 * Description:
 *    Handle XTM receive processing.
 * Returns: void
 *---------------------------------------------------------------------------
 */

/*
 * bcm63xx_xtmrt_rx_thread: Handle XTM receive processing
 */
static int bcm63xx_xtmrt_rx_thread(void *arg)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 mask;
   uint32 work_done;
   uint32 ret_done;
   uint32 more_to_do;
   int budget = XTMRT_BUDGET;


   /* Main task loop */
   while (1)
   {
      /* Wait to be woken up by received packets */
      wait_event_interruptible(pGi->rx_thread_wqh,
                    pGi->rx_work_avail);

      /* Abort if we were woken up to terminate */
      if (kthread_should_stop())
      {
         BCM_XTM_ERROR("kthread_should_stop detected on bcmxtmrt-rx");
         break;
      }

      /* Do preprocessing if any */
      bcmxapi_preRxProcessing(&mask);

      /* Now, process some received packets */
      work_done  = bcmxapi_rxtask(budget, &more_to_do);
      ret_done   = work_done & XTM_POLL_DONE;            /* Did the poll of channels complete? */
      work_done &= ~XTM_POLL_DONE;                       /* How many packets processed? */

      /* Did we use up our budget?  Do we still have channels with
         unprocessed data left in them (i.e. did we fail to complete
         a poll of all channels)? */
      if (work_done >= budget || ret_done != XTM_POLL_DONE)
      {
         /* We have either exhausted our budget or there are
            more packets on the DMA (or both). */

         /* Do post Rx processing if any */
         bcmxapi_postRxProcessing(mask);

         /* Yield CPU to allow others to have a chance, then continue to
            top of loop for more work.  */
         yield();
      }
      else
      {
         /*
          * No more packets.  Indicate we are done (rx_work_avail=0) and
          * re-enable interrupts (bcmxapi_clear_xtmrxint) and go to top of
          * loop to wait for more work.
          */
         clear_bit(0, &pGi->rx_work_avail);
         bcmxapi_clear_xtmrxint(mask);
      }
   }

   return 0;
}

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init(bcmxtmrt_init);
module_exit(bcmxtmrt_cleanup);
MODULE_LICENSE("GPL");

