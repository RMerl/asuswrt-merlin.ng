/*
   Copyright (c) 2002 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2002:DUAL/GPL:standard

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


#include <linux/kernel.h>

#include <linux/inetdevice.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/arp.h>
#include <linux/version.h>

#include "DiagDef.h"
#include "BcmDiagDrv.h"
#include "BcmDiag.h"
#include "shared_utils.h"

#define MIN_DAIG_DATA_LEN     28 /* 32 bytes: 28 + sizeof(LogProtoHeader) */
#define DIAG_DATA_ALIGN_MASK  3

static spinlock_t diagLock;
#define	BcmDiagSyncInit()    spin_lock_init(&diagLock)
#define	BcmDiagSyncEnter()   spin_lock_bh(&diagLock)
#define	BcmDiagSyncExit()    spin_unlock_bh(&diagLock)

#define DIAG_MAX_NUM_FRAG     100

struct bcmDiagClients{
   unsigned int   client;
   BcmDiagDrvSkbPool  *diagSkbDev;
   void           (*pCallback)(unsigned char lineId, void *pCmdData);
   BOOL           isRegistered;
   unsigned int  timerInterval;
   unsigned int  timerIntervalInJiffie;
   unsigned int  timerExpireCntMax;
   unsigned int  timerExpireCntCur;
};

struct bcmDiagTimer {
   unsigned int lowestTimerIntervalInJiffie;
   unsigned int timerExpireCntMaxMT;      // MT => maintenance task
   unsigned int timerExpireCntCurMT;
   unsigned int conMonitorPauseTimerCnt;    // Number of seconds to pause connection monitoring
   struct bcmDiagClients *pDiagClientsTbl;
};

static bcmDiagCtrlType  BcmDiagCtrl = {0};
static BOOL          diagLockSession = 0;
static BOOL          diagRegressionLock = 0;
static DiagConnectInfo     diagConnectInfo = {{ 0 }, { 0 }};
static struct bcmDiagClients  diagClientsTbl[DIAG_MAX_CLIENT];

struct bcmDiagTimer gBcmDiagTimer;

static BOOL    gDiagProcessingCmd = 0;
unsigned long	gDiagLastCmdTime = 0;
unsigned long	gDiagLastPingTime = 0;
unsigned long	gDiagSyncTimeMs = 0;
unsigned long	gDiagSyncTimeMsLast = 0;

char  savedStatFileName[]  = "savedStatusFile.bin";
char  successStr[]         = "SUCCESS";
char  failStr[]            = "FAIL";

//*****************
static int obj_pending_skb_pool_en = 0;
static struct sk_buff_head obj_pending_skb_pool;
static struct sk_buff_head* pending_skb_pool = NULL;

#if defined(ADSLDRV_LITTLE_ENDIAN)

/* Diag Driver to/from PHY address and data conversion macros */
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)

static __inline uint brev32(uint x)
{
#if defined(__GNUC__)
#if defined(CONFIG_ARM64)
   asm volatile("rev32 %0, %1" : "=r" (x) : "r" (x));
#else
   asm volatile("rev %0, %1" : "=r" (x) : "r" (x));
#endif
#else
   __asm { rev  x, x };
#endif
   return x;
}

static __inline ushort brev16(ushort x)
{
#if defined(__GNUC__)
#if defined(CONFIG_ARM64)
   asm volatile("rev16 %0, %1" : "=r" (x) : "r" (x));
#else
   asm volatile("revsh %0, %1" : "=r" (x) : "r" (x));
#endif
#else
   __asm { revsh  x, x };
#endif
   return x;
}

#define  DIAG_ENDIAN_CONV_INT32(x)  ( (int)brev32(x) )
#define  DIAG_ENDIAN_CONV_SHORT(x)  ( (short)brev16(x) )

#else

#define  DIAG_ENDIAN_CONV_INT32(x)  ( (int)(((x) << 24) | (((x) << 8) & 0x00FF0000) | (((x) >> 8) & 0x0000FF00) | ((uint)(x) >> 24)) )
#define  DIAG_ENDIAN_CONV_SHORT(x)  ( (short)(((x) << 8) | ((ushort)(x) >> 8)) )

#endif /* defined(__arm) || defined(CONFIG_ARM) */

#else    /* !ADSLDRV_LITTLE_ENDIAN */

#define	DIAG_ENDIAN_CONV_INT32(x)	( (int)(x) )
#define	DIAG_ENDIAN_CONV_SHORT(x)	( (short)(x) )

#endif   /* ADSLDRV_LITTLE_ENDIAN */


int BcmDiagPendingSkbMsgQueueSize(void)
{
   int retval = 0;
   if ( pending_skb_pool )
      retval = skb_queue_len(pending_skb_pool);
   return retval;
}

void BcmDiagPendingSkbMsgQueueEnable(int bEn)
{
   obj_pending_skb_pool_en = bEn;
}

/**
   This method parses the queue and dequeues the element with user count 0
 */
void BcmDiagPendingSkbMsgQueuePurge(int bFreeMem)
{
   if (pending_skb_pool) {
      int qlen = pending_skb_pool->qlen;
      while(qlen>0) {
         struct sk_buff* skb;
         qlen--;
         if((skb=skb_dequeue(pending_skb_pool))) {
            int skb_users;
            skb_users = refcount_read(&skb->users);
            if ( skb_users ) {
               skb_queue_tail(pending_skb_pool, skb);
            }
            else if(bFreeMem) {
               refcount_set(&skb->users, 1);
               kfree_skb(skb);
            }
         }
      }
      //printk("%s: qlen = %d\n", __FUNCTION__, pending_skb_pool->qlen);
   }
}

void BcmDiagPendingSkbMsgQueueClear(void)
{
   if (pending_skb_pool) {
      struct sk_buff* skb = NULL;
      while((skb=skb_dequeue(pending_skb_pool))) {
         int skb_users;
         skb_users = refcount_read(&skb->users);
         if(DIAG_SKB_USERS != skb_users) {
            refcount_set(&skb->users, 1);
            kfree_skb(skb);
         }
         else {
            refcount_set(&skb->users, (DIAG_SKB_USERS-1) );
         }
      }
   }
}

int BcmDiagPendingSkbAdd(struct sk_buff* skb)
{
   static int showErrMsg = 0;
   int retval;

   if ( SKB_REROUTE_FIELD(skb) != 0 )
      return DEV_TRANSMIT_(skb);

   if (pending_skb_pool==NULL) {
      pending_skb_pool = &obj_pending_skb_pool;
      skb_queue_head_init(pending_skb_pool);
   }

   retval = skb->len;

   if ( obj_pending_skb_pool_en )
   {
      unsigned int pool_queue_size = skb_queue_len(pending_skb_pool);
      if ( pool_queue_size < 256 ) {
         showErrMsg = 1;
         //skb->list = pending_skb_pool;
         skb_queue_tail(pending_skb_pool, skb);
         //printk("%s: qlen=%u skb->len=%d skb=%08x\n",__FUNCTION__, pending_skb_pool->qlen, retval, (unsigned int)(uintptr_t)skb);
         BcmDiagProcWakeup(BCM_DIAG_PROC_SKB_MSG);
      }
      else {
         if (showErrMsg) {
            showErrMsg = 0;
            printk("%s: queue error\n",__FUNCTION__);
         }
         retval = -ENOMEM;
      }
   }

   return retval;
}

/**
 * Read only SkbDiagFrameData+diagHdr+msg, ignoring ipHdr+udpHdr
 * (*d)    -
 *     this pointer points to USER memory location, not KERNEL
 * (*dlen) - 
 *     this pointer points to KERNEL memory location
 *     IN:  specifies available number of bytes under d
 *     OUT: specifies number of bytes written to d
 *  Note: it is easier to return copied number of bytes through dlen
 *        this means, the caller HAS to check dlen, 
 *        and update frameLen in SkbDiagFrameData object
 */
void BcmDiagPendingSkbMsgRead(void* d, long* dlen)
{
   int skb_users = 0;
   struct sk_buff* skb = NULL;
   if ( pending_skb_pool )
      skb = skb_dequeue(pending_skb_pool);

   if (skb==NULL) {
      *dlen = 0;
   }
   else
   {
      static const int prefixSize = sizeof(SkbDiagFrameData) - sizeof(DiagProtoFrame);

      diagSockFrame		*dd;
      unsigned short bytesToCopy;

      dd = PTRDIAGSOCKFRAME(skb); //(diagSockFrame *)((uintptr_t) skb->data-DIAG_FRAME_PAD_SIZE);

      bytesToCopy = dd->dsfh.udpHdr.len - sizeof(diagUdpHeader) + (unsigned short)prefixSize; // expecting this to be a valid len

      if ( bytesToCopy > *dlen ) {
         printk("%s: Error: not enough buffer memory(src:%u dst:%u)...trunc\n", __FUNCTION__, bytesToCopy, (unsigned int)*dlen);
         bytesToCopy = *dlen;
      }

      {
         // Ugly hack:
         // copying SkbDiagFrameData struct in one go.
         unsigned int bytes_not_copied;
         SkbDiagFrameData* dfd = (SkbDiagFrameData*)( (uintptr_t)&dd->dsfh.diagHdr - prefixSize );
         dfd->frameLen = (unsigned short)(bytesToCopy - prefixSize);
         dfd->target = SKB_REROUTE_DEST_FIELD(skb);

         bytes_not_copied = copy_to_user(d, dfd, bytesToCopy);
         *dlen = bytesToCopy - bytes_not_copied;
      }

      skb_users = refcount_read(&skb->users);

      if(DIAG_SKB_USERS != skb_users) {
         refcount_set(&skb->users, 1);
         kfree_skb(skb);
      }
      else {
         refcount_set(&skb->users, (DIAG_SKB_USERS-1) );
      }
   }
}
//*****************

void IdleStatusCallback(unsigned char lineId, void *pCmdData)
{
}

static void diagConnectionCheck(void);

static unsigned int BcmDiagValidateTimerInterval(unsigned int timerInterval)
{
   unsigned int nJiffie = 0;
   switch(timerInterval) {
      case TIMER_EXPIRE_10MS:
         nJiffie = (10 * HZ)/1000;
         break;
      case TIMER_EXPIRE_20MS:
         nJiffie = (20 * HZ)/1000;
         break;
      case TIMER_EXPIRE_50MS:
         nJiffie = (50 * HZ)/1000;
         break;
      case TIMER_EXPIRE_100MS:
         nJiffie = (100 * HZ)/1000;
         break;
      case TIMER_EXPIRE_1000MS:
         nJiffie = (1000 * HZ)/1000;
         break;
      default:
         printk("*** %s: Invalid timerInterval(%u) *** !!!\n", __FUNCTION__, timerInterval);
         break;
   }
   return nJiffie;
}

#if 0
static int timerInfoPrintCnt = 101;    // TO DO: Remove after debugging.
unsigned long tPrev = -1;
static void BcmDiagPrintClientTimerInfo(void)
{
   int i;
   
   printk("*** %s: lowestTimerIntervalInJiffie=%u timerExpireCntMaxMT=%u timerExpireCntCurMT=%u ***\n",
      __FUNCTION__, gBcmDiagTimer.lowestTimerIntervalInJiffie,
      gBcmDiagTimer.timerExpireCntMaxMT, gBcmDiagTimer.timerExpireCntCurMT);
   
   for (i = 0; i < DIAG_MAX_CLIENT; i++) {
      if(diagClientsTbl[i].timerIntervalInJiffie) {
         printk("  clientType %d: timerIntervalInJiffie=%u timerExpireCntMax=%u timerExpireCntCur=%u\n",
            i, diagClientsTbl[i].timerIntervalInJiffie, diagClientsTbl[i].timerExpireCntMax, diagClientsTbl[i].timerExpireCntCur);
      }
   }
}
#endif

static DIAGDRV_DIAG gBcmDiag;
static DiagDebugData gDbgCmd;

long BcmDiagProcessTimer(void)
{
   int i;
   long timeOutInJiffies;
   unsigned long t0 = jiffies;

   for (i = 0; i < DIAG_MAX_CLIENT; i++) {
      if(0 == diagClientsTbl[i].timerIntervalInJiffie)
         continue;
      if(++diagClientsTbl[i].timerExpireCntCur < diagClientsTbl[i].timerExpireCntMax)
         continue;
      diagClientsTbl[i].timerExpireCntCur = 0;
      gDbgCmd.param1 = diagClientsTbl[i].timerInterval;
      diagClientsTbl[i].pCallback(0, (void *)&gBcmDiag);
   }
   // Check for the Maintenance task timer which expires every 1 second.
   if(++gBcmDiagTimer.timerExpireCntCurMT == gBcmDiagTimer.timerExpireCntMaxMT) {
      gBcmDiagTimer.timerExpireCntCurMT = 0;
      BcmDiagSyncEnter();
      if(!gBcmDiagTimer.conMonitorPauseTimerCnt) {
        BcmDiagSyncExit();
        diagConnectionCheck();
      }
      else {
        gBcmDiagTimer.conMonitorPauseTimerCnt--;
        BcmDiagSyncExit();
      }
   }

   timeOutInJiffies = gBcmDiagTimer.lowestTimerIntervalInJiffie - (unsigned int)(jiffies - t0);
#if 0
   if(timerInfoPrintCnt) {
      printk("%s -> printCnt=%d t0=%lu(%lu) timeOutInJiffies=%ld\n",__FUNCTION__, timerInfoPrintCnt, t0, t0-tPrev, timeOutInJiffies);
      tPrev = t0;
      BcmDiagPrintClientTimerInfo();
      timerInfoPrintCnt--;
   }
#endif
   return timeOutInJiffies;
}

void BcmDiagRegisterClient(uint clientType, void *pCallback, unsigned int timerInterval)
{
   int i;
   unsigned int nJiffie;

   if(clientType >= DIAG_MAX_CLIENT)
      return;
   
   diagClientsTbl[clientType].isRegistered = TRUE;
   if (NULL == pCallback)
      diagClientsTbl[clientType].pCallback = IdleStatusCallback;
   else {
      diagClientsTbl[clientType].pCallback = pCallback;
      if( 0 != (nJiffie = BcmDiagValidateTimerInterval(timerInterval)) ) {
         diagClientsTbl[clientType].timerInterval = timerInterval;
         diagClientsTbl[clientType].timerIntervalInJiffie = nJiffie;
         diagClientsTbl[clientType].timerExpireCntCur = 0;
         if(diagClientsTbl[clientType].timerIntervalInJiffie < gBcmDiagTimer.lowestTimerIntervalInJiffie) {
            diagClientsTbl[clientType].timerExpireCntMax = 1;
            gBcmDiagTimer.lowestTimerIntervalInJiffie = diagClientsTbl[clientType].timerIntervalInJiffie;
            // Adjust timer for Maintenance task
            gBcmDiagTimer.timerExpireCntMaxMT = HZ/gBcmDiagTimer.lowestTimerIntervalInJiffie;
            gBcmDiagTimer.timerExpireCntCurMT = 0;
            // Adjust other client's timer.
            for (i = 0; i < DIAG_MAX_CLIENT; i++) {
               if(i == clientType)
                  continue;
               if(diagClientsTbl[i].timerIntervalInJiffie) {
                  diagClientsTbl[i].timerExpireCntMax = diagClientsTbl[i].timerIntervalInJiffie/gBcmDiagTimer.lowestTimerIntervalInJiffie;
                  diagClientsTbl[i].timerExpireCntCur = 0;
               }
            }
         }
         else {
            diagClientsTbl[clientType].timerExpireCntMax = diagClientsTbl[clientType].timerIntervalInJiffie/gBcmDiagTimer.lowestTimerIntervalInJiffie;
         }
      }
   }
   
   printk("*** %s: %d ***\n", __FUNCTION__, (int)clientType);
}

void BcmDiagDeRegisterClient(uint clientType)
{
   if(clientType >= DIAG_MAX_CLIENT)
      return;
   diagClientsTbl[clientType].isRegistered = FALSE;
   diagClientsTbl[clientType].pCallback = NULL;
   printk("*** %s: %d ***\n", __FUNCTION__, (int)clientType);
}

static struct sk_buff * BcmDiagGetSkbForReroute(int len);
static ushort BcmDiagIpComputeChecksum(diagIpHeader *ipHdr);

static void BcmDiagCmdNotify(void)
{
   gDiagLastCmdTime = jiffies_to_msecs(jiffies);
}

void BcmDiagSetSyncTime(unsigned long syncTime)
{
	BcmDiagSyncEnter();
	gDiagSyncTimeMsLast = jiffies;
	gDiagSyncTimeMs = syncTime+1;
	BcmDiagSyncExit();
	printk ("%s: syncTime=%lu ms %lu sec\n", __FUNCTION__, syncTime, syncTime/1000);
}

unsigned long BcmDiagGetSyncTime(void)
{
	unsigned long	curSyncTime;
	
	BcmDiagSyncEnter();
	curSyncTime = jiffies;
	gDiagSyncTimeMs += jiffies_to_msecs(curSyncTime - gDiagSyncTimeMsLast);
	gDiagSyncTimeMsLast = curSyncTime;
	BcmDiagSyncExit();
	
	return (gDiagSyncTimeMs+500)/1000;
}

static void BcmDiagClearStats(uint clientType)
{
   if(clientType >= DIAG_MAX_CLIENT)
      return;
   if (diagClientsTbl[clientType].diagSkbDev != NULL)
      diagClientsTbl[clientType].diagSkbDev->extraSkb = 0;
}

static void BcmDiagPrintStats(uint clientType)
{
   int extraSkbCount = 0, numberOfSkbs = 0, numberOfShortSkbs = 0;
   
   if(clientType >= DIAG_MAX_CLIENT)
      return;
   if (diagClientsTbl[clientType].diagSkbDev != NULL) {
      extraSkbCount = diagClientsTbl[clientType].diagSkbDev->extraSkb;
      numberOfSkbs = diagClientsTbl[clientType].diagSkbDev->numOfSkbs;
      numberOfShortSkbs = diagClientsTbl[clientType].diagSkbDev->numOfShortSkbs;
   }
   DiagStrPrintf(0, clientType,
      "DiagLinux Statistics:\n"
      "   numberOfSkbs	= %d\n"
      "   numberOfShortSkbs	= %d\n"
      "   extraSkbCount	= %d\n",
      numberOfSkbs,
      numberOfShortSkbs,
      extraSkbCount);
#if 0 && defined(SUPPORT_VECTORING)
   BcmXdslDumpVectSkbDevInfo(0);
#endif
}

static char *ConvertToDottedIpAddr(uint ipAddr, char *buf)
{
   if(NULL != buf) {
      sprintf(buf,"%d.%d.%d.%d",
         (int)((ipAddr>>24)&0xFF),
         (int)((ipAddr>>16)&0xFF),
         (int)((ipAddr>>8)&0xFF),
         (int)(ipAddr&0xFF));
   }
   return buf;
}

static struct in_ifaddr * BcmDiagGetIntf(struct net_device	*dev, char *pDevName, uint ipAddr)
{
	struct in_ifaddr		*ifa, **ifap;
	struct in_device	*in_dev;
	char			buf[16], tmpBuf[16];

	/* get device IP address */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
	in_dev = __in_dev_get_rtnl(dev);
#else
	in_dev = __in_dev_get(dev);
#endif
	if (NULL == in_dev)
		return NULL;

	for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next) {
		printk ("\tifa_label = %s, ipAddr = %s, mask = %s\n",
			ifa->ifa_label, ConvertToDottedIpAddr(ntohl(ifa->ifa_local), buf), ConvertToDottedIpAddr(ntohl(ifa->ifa_mask),tmpBuf));
		sprintf(tmpBuf, "%s:0", pDevName);	/* Construct the secondary interface name */
		if ((0 == strcmp(pDevName, ifa->ifa_label)) ||
			(0 == strcmp(tmpBuf, ifa->ifa_label))) {
			if ((ifa->ifa_local & ifa->ifa_mask) == (ipAddr & ifa->ifa_mask)) {
				printk("\tMatched network address found!\n");
				return ifa;
			}
		}
	}
	return NULL;
}


static struct net_device * 
BcmDiagInitNetDev(PDIAGDRV_DIAG pBcmDiag, int port, struct sk_buff **ppskb, char *devname)
{
	char				*diagDevNames[] = {"eth0", "br1", "br0"};
	struct net_device	*dev = NULL;
	struct in_ifaddr	*ifa;
	struct rtable		rtbl;
	diagSockFrame		*dd;
	int			n, dstPort;
	struct sk_buff	*pskb = *ppskb;
	char			buf[16], tmpBuf[16];
	int				nDevNames = sizeof(diagDevNames)/sizeof(diagDevNames[0]);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	struct net		*net;
#if defined(LINUX_FW_EXTRAVERSION) && (LINUX_FW_EXTRAVERSION >= 50204)
	struct nsproxy	*nsproxy;
#endif
#endif

	dstPort = ((uint) pBcmDiag->diagMap) >> 16;
	if (0 == dstPort)
		dstPort = port;
	
	printk ("Drv%sCmd: CONNECT map=0x%X, logTime=%d, gwIpAddr=%s, dstPort=%d\n",
		devname, (uint)pBcmDiag->diagMap&0xffff, pBcmDiag->logTime,
		ConvertToDottedIpAddr(ntohl(pBcmDiag->gwIpAddr),buf), dstPort);
	
	/* find DIAG interface device */
	ifa = NULL;
	printk ("Searching for interface that has matching network address w/ srvIpaddr %s\n", ConvertToDottedIpAddr(ntohl(pBcmDiag->srvIpAddr), buf));
	
	if(0 != diagConnectInfo.devName[0]) {
		diagDevNames[0] = diagConnectInfo.devName;
		nDevNames = 1;
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#if defined(LINUX_FW_EXTRAVERSION) && (LINUX_FW_EXTRAVERSION >= 50204)
	/* get net device list by namespace */
	nsproxy = current->nsproxy;
	if (nsproxy)
		net = nsproxy->net_ns;
	else
#endif
		net = &init_net;
#endif

	for (n = 0; n < nDevNames; n++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
		dev = __dev_get_by_name(net, diagDevNames[n]);
#else
		dev = __dev_get_by_name(diagDevNames[n]);
#endif
		if (NULL == dev)
			continue;
		printk ("\tdev = %s(0x%px) hwAddr=%X:%X:%X:%X:%X:%X\n",
			diagDevNames[n], dev,
			dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
			dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
		if(NULL != (ifa = BcmDiagGetIntf(dev, diagDevNames[n], (uint)pBcmDiag->srvIpAddr)))
			break;
	}

	if (NULL == ifa) {
		if(0 == pBcmDiag->gwIpAddr) {
			printk ("No interface with matching network address with the srvIpAddr. No gateway specified, Diag support is impossible\n");
			return NULL;
		}
		printk ("Searching for interface that has matching network address w/ gwIpAddr %s\n",
			ConvertToDottedIpAddr(ntohl(pBcmDiag->gwIpAddr), buf));
		for (n = 0; n < nDevNames; n++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
			dev = __dev_get_by_name(net, diagDevNames[n]);
#else
			dev = __dev_get_by_name(diagDevNames[n]);
#endif
			if (NULL == dev)
				continue;
			if(NULL != (ifa = BcmDiagGetIntf(dev, diagDevNames[n], (uint)pBcmDiag->gwIpAddr)))
				break;
		}
	}
	
	if (NULL == ifa) {
		printk ("No interface with matching network address with the gwIpAddr. Diag support is impossible\n");
		return NULL;
	}

	/* get remote MAC address for Diag srvIpAddr */
	if (NULL == pskb) {
		pskb = alloc_skb (DIAG_FRAME_HEADER_LEN + 32, GFP_ATOMIC);
		if (pskb == NULL)
			return NULL;
	}
	pskb->dev = dev;
	pskb->protocol = htons(eth_type_trans (pskb, dev));
	dd = (diagSockFrame *) pskb->head;
	dd->dsfh.etype = htons(ETH_P_IP);

	memcpy(dd->dsfh.srcMacAddr, dev->dev_addr, ETH_ALEN);
	memset(dd->dsfh.dstMacAddr, 0, ETH_ALEN);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
	skb_dst_set(pskb, (void *) &rtbl);
#else
	pskb->dst = (void *) &rtbl;
#endif

	if ((ifa->ifa_local & ifa->ifa_mask) == (pBcmDiag->srvIpAddr & ifa->ifa_mask)) {
		printk ("Diag server is on the same subnet\n");
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0))
		rtbl.rt_gw4 = pBcmDiag->srvIpAddr;
#else
		rtbl.rt_gateway = pBcmDiag->srvIpAddr;
#endif
		if((0 != diagConnectInfo.devName[0]) &&
			(0 != (diagConnectInfo.macAddr[0] | diagConnectInfo.macAddr[1] | diagConnectInfo.macAddr[2] | 
			diagConnectInfo.macAddr[3] | diagConnectInfo.macAddr[4] | diagConnectInfo.macAddr[5]))) {
			memcpy(dd->dsfh.dstMacAddr, diagConnectInfo.macAddr, 6);
			n = 0;
		}
		else
#ifdef CONFIG_BRCM_IKOS
			n = -1;
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
			n = arp_find(dd->dsfh.dstMacAddr, pskb);
#else
			n = -1;
#endif
#endif
	}
	else {
		printk ("Diag server is outside subnet, using local IP address %s and gateway IP address %s\n",
			ConvertToDottedIpAddr(ntohl(ifa->ifa_local), buf),
			ConvertToDottedIpAddr(ntohl(pBcmDiag->gwIpAddr), tmpBuf));
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0))
		rtbl.rt_gw4 = pBcmDiag->gwIpAddr;
#else
		rtbl.rt_gateway = pBcmDiag->gwIpAddr;
#endif
		if((0 != diagConnectInfo.devName[0]) &&
			(0 != (diagConnectInfo.macAddr[0] | diagConnectInfo.macAddr[1] | diagConnectInfo.macAddr[2] | 
			diagConnectInfo.macAddr[3] | diagConnectInfo.macAddr[4] | diagConnectInfo.macAddr[5]))) {
			memcpy(dd->dsfh.dstMacAddr, diagConnectInfo.macAddr, 6);
			n = 0;
		}
		else
#ifdef CONFIG_BRCM_IKOS
			n = -1;
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
			n = arp_find(dd->dsfh.dstMacAddr, pskb);
#else
			n = -1;
#endif
#endif
	}

	printk ("dstMACAddr = %02X:%02X:%02X:%02X:%02X:%02X\n", 
		dd->dsfh.dstMacAddr[0], dd->dsfh.dstMacAddr[1], dd->dsfh.dstMacAddr[2],
		dd->dsfh.dstMacAddr[3], dd->dsfh.dstMacAddr[4], dd->dsfh.dstMacAddr[5]);

	if ((n != 0) ||
		(0 == (dd->dsfh.dstMacAddr[0] | dd->dsfh.dstMacAddr[1] | dd->dsfh.dstMacAddr[2] | dd->dsfh.dstMacAddr[3] | dd->dsfh.dstMacAddr[4] | dd->dsfh.dstMacAddr[5]))) {
		*ppskb = pskb;
		return NULL;
	}

	dd->dsfh.ipHdr.ver_hl = 0x45;
	dd->dsfh.ipHdr.tos = 0;
	dd->dsfh.ipHdr.len = 0;			/* changes for frames */
	dd->dsfh.ipHdr.id = htons(0x2000);
	dd->dsfh.ipHdr.off = 0;
	dd->dsfh.ipHdr.ttl =128;
	dd->dsfh.ipHdr.proto = 0x11;		/* always UDP */
	dd->dsfh.ipHdr.checksum = 0;		/* changes for frames */
	dd->dsfh.ipHdr.srcAddr = ifa->ifa_local;
	dd->dsfh.ipHdr.dstAddr = pBcmDiag->srvIpAddr;
	dd->dsfh.ipHdr.checksum = BcmDiagIpComputeChecksum(&dd->dsfh.ipHdr);
	dd->dsfh.udpHdr.srcPort = htons(port);
	dd->dsfh.udpHdr.dstPort = htons(dstPort);
	dd->dsfh.udpHdr.len = 0;			/* changes for frames */
	dd->dsfh.udpHdr.checksum = 0;	/* not used */

	/* to prevent skb from deallocation */
	pskb->data = pskb->head + DIAG_FRAME_PAD_SIZE;
	refcount_set(&pskb->users, DIAG_SKB_USERS);

	*ppskb = pskb;

	return dev;
}

/* len is total length not including frame and pad */
struct sk_buff * BcmDiagGetSkb(BcmDiagDrvSkbPool *skbDev, int len)
{
	struct sk_buff	*skbDiag = NULL;
	struct sk_buff	*skbRef = NULL;
	struct sk_buff	** ppskb;
	int	totalLen;

	if(BcmDiagCtrl.skbModelReroute)
		return BcmDiagGetSkbForReroute(len);
	else {
		ppskb = skbDev->skbPool;
		totalLen = (len + skbDev->frameHeaderLen);
	}
	
	//BcmCoreDpcSyncEnter();	// Caller should have called BcmCoreDpcSyncEnter(&gXdslGlobalInfo.xdslLockDiags);
	if(totalLen < skbDev->shortSkbLengh) {
		if (DIAG_SKB_USERS != refcount_read((&ppskb[skbDev->shortSkbBufIndex]->users))) {
			skbDiag = ppskb[skbDev->shortSkbBufIndex];
			skbDev->shortSkbBufIndex++;
			if (skbDev->shortSkbBufIndex == (skbDev->numOfSkbs + skbDev->numOfShortSkbs))
				skbDev->shortSkbBufIndex -= skbDev->numOfShortSkbs;
		}
	} else if (DIAG_SKB_USERS != refcount_read((&ppskb[skbDev->skbBufIndex]->users))) {
		skbDiag = ppskb[skbDev->skbBufIndex];
		skbDev->skbBufIndex++;
		if (skbDev->skbBufIndex == skbDev->numOfSkbs)
			skbDev->skbBufIndex -= skbDev->numOfSkbs;
	}
	
	if (skbDiag != NULL) {
		refcount_set(&skbDiag->users, DIAG_SKB_USERS);
		skbDiag->data = skbDiag->head;
		if(skbDev->skbHeadRoomReserve)
			skb_reserve (skbDiag, skbDev->skbHeadRoomReserve);
		skbDiag->data = DIAG_DATA_ALIGN(skbDiag->data, skbDev->dataAlignMask);
		skbDiag->len = skbDev->frameHeaderLen;
		skbDiag->dev = skbDev->skbModel->dev;
	}
	else {
		if(len < MIN_DAIG_DATA_LEN)
			totalLen += (MIN_DAIG_DATA_LEN - len);
		if( NULL != (skbDiag = alloc_skb (totalLen+skbDev->dataAlignMask + skbDev->skbHeadRoomReserve, GFP_ATOMIC)) ) {
			int skb_reroute;
			skbRef = skbDev->skbModel;
			skb_reroute = (SKB_REROUTE_FIELD(skbRef)==0);
			if(skbDev->skbHeadRoomReserve)
				skb_reserve (skbDiag, skbDev->skbHeadRoomReserve);
			skbDev->extraSkb++;
			skbDiag->data = DIAG_DATA_ALIGN(skbDiag->data, skbDev->dataAlignMask);
			skbDiag->dev = skbRef->dev;
			skbDiag->protocol = (skb_reroute)?skbRef->protocol:htons(eth_type_trans (skbDiag, skbRef->dev));
			memcpy(skbDiag->data, skbRef->data, skbDev->frameHeaderLen);
			skbDiag->len = skbDev->frameHeaderLen;
		}
	}
	//BcmCoreDpcSyncExit();
	
	return skbDiag;
}

struct sk_buff * BcmDiagGetClientSkb(unsigned int clientType, int len)
{
	BcmDiagDrvSkbPool	*diagSkbDev;
	struct sk_buff	*skbDiag = NULL;
	
	if(clientType >= DIAG_MAX_CLIENT)
		return skbDiag;
	
	BcmDiagSyncEnter();
	diagSkbDev = diagClientsTbl[clientType].diagSkbDev;
	if (diagSkbDev != NULL)
		skbDiag = BcmDiagGetSkb(diagSkbDev, len);
	BcmDiagSyncExit();
	
	return skbDiag;
}

BcmDiagDrvSkbPool * BcmDiagDevSkbAllocate(struct sk_buff *model,
		int skbMaxBufSize, int numOfSkbsInPool,
		int shortSkbMaxBufSize, int numOfShortSkbsInPool,
		int dataAlignMask, int frameHeaderLength, int dmaZone, int skbHeadRoomReserve)
{
	int			 n;
	struct sk_buff	*skb = NULL;
	BcmDiagDrvSkbPool	* skbDev = NULL;
	struct sk_buff ** ppskb;
	gfp_t	gfp_mask;

	int skb_reroute = (SKB_REROUTE_FIELD(model)==0);

	if(dmaZone)
		gfp_mask = (GFP_ATOMIC | GFP_DMA);
	else
		gfp_mask = GFP_ATOMIC;
		
	skbDev = (BcmDiagDrvSkbPool *) kmalloc(sizeof(*skbDev) + sizeof(struct sk_buff **) * (numOfSkbsInPool+numOfShortSkbsInPool), GFP_ATOMIC);
	if (skbDev != NULL) {
		skbDev->skbPool = (struct sk_buff **)(skbDev + 1);
		ppskb = skbDev->skbPool;
		for (n = 0; n < numOfSkbsInPool; n++) {
			skb = alloc_skb (skbMaxBufSize+dataAlignMask + skbHeadRoomReserve, gfp_mask);
			if(skb != NULL) {
				if(skbHeadRoomReserve)
					skb_reserve (skb, skbHeadRoomReserve);
				skb->dev = model->dev;
				skb->protocol = (skb_reroute)?model->protocol:htons(eth_type_trans (skb, model->dev));
				skb->data = DIAG_DATA_ALIGN(skb->data, dataAlignMask);
				memcpy(skb->data, model->data, frameHeaderLength);
#ifdef DIAG_ALLOC_SKB_DYNAMICALLY
				refcount_set(&skb->users, DIAG_SKB_USERS);
#else
				refcount_set(&skb->users, DIAG_SKB_USERS-1);
#endif
				ppskb[n] = skb;
			} else {
				n--;
				break;
			}
		}
		skbDev->numOfSkbs = n;
		for (; n < (skbDev->numOfSkbs+numOfShortSkbsInPool); n++) {
			skb = alloc_skb (shortSkbMaxBufSize+dataAlignMask + skbHeadRoomReserve, gfp_mask);
			if(skb != NULL) {
				if(skbHeadRoomReserve)
					skb_reserve (skb, skbHeadRoomReserve);
				skb->dev = model->dev;
				skb->protocol = (skb_reroute)?model->protocol:htons(eth_type_trans (skb, model->dev));
				skb->data = DIAG_DATA_ALIGN(skb->data, dataAlignMask);
				memcpy(skb->data, model->data, frameHeaderLength);
#ifdef DIAG_ALLOC_SKB_DYNAMICALLY
				refcount_set(&skb->users, DIAG_SKB_USERS);
#else
				refcount_set(&skb->users, DIAG_SKB_USERS-1);
#endif
				ppskb[n] = skb;
			} else {
				n--;
				break;
			}
		}
		skbDev->skbModel = model;
		skbDev->numOfShortSkbs = n - skbDev->numOfSkbs;
		skbDev->skbLengh = skbMaxBufSize;
		skbDev->shortSkbLengh = shortSkbMaxBufSize;
		skbDev->skbBufIndex = 0;
		skbDev->shortSkbBufIndex = skbDev->numOfSkbs;
		skbDev->frameHeaderLen = frameHeaderLength;
		skbDev->dataAlignMask = dataAlignMask;
		skbDev->extraSkb = 0;
		skbDev->skbHeadRoomReserve = skbHeadRoomReserve;
	}
	return skbDev;
}

static void BcmDiagAllocateBlankSkb( struct sk_buff **ppskb  )
{
	struct sk_buff *pskb = *ppskb;
	
	if ( pskb )
		printk("%s: Warning: skb pointer is not null\n",__FUNCTION__);
	
	pskb = alloc_skb (DIAG_FRAME_HEADER_LEN + 32, GFP_ATOMIC);
	if(NULL != pskb) {
		pskb->data = pskb->head + DIAG_FRAME_PAD_SIZE;
		refcount_set(&pskb->users, DIAG_SKB_USERS);
	}
	
	*ppskb = pskb;
}

static struct net_device * BcmDiagInit(PDIAGDRV_DIAG pBcmDiag)
{
   struct net_device *dev = NULL;
   int allocSkbPool = 0;

   if ( pBcmDiag->srvIpAddr == 0 ) {
      if ( ! BcmDiagCtrl.skbModel )
         BcmDiagAllocateBlankSkb(&BcmDiagCtrl.skbModel);
      BcmDiagCtrl.skbModel->dev = NULL;
      // Indicating the corresponding skb pool will be re-routed
      SKB_REROUTE_FIELD(BcmDiagCtrl.skbModel) = 0;
      // Preserving client's ip, which is expected to be passed in gwIpAddr field
      SKB_REROUTE_CLIENT_ADDR_FIELD(BcmDiagCtrl.skbModel) = pBcmDiag->gwIpAddr;
      // Indicating the corresponding skb pool is for Diag
      SKB_REROUTE_DEST_FIELD(BcmDiagCtrl.skbModel) = DIAG_SKB_REROUTE_DATA;
      BcmDiagCtrl.skbModelReroute = 1;
   }
   else {
      BcmDiagCtrl.skbModelReroute = 0;
      dev = BcmDiagInitNetDev(pBcmDiag, LOG_FILE_PORT, &BcmDiagCtrl.skbModel, "Diag");
      allocSkbPool = (dev != NULL) && (BcmDiagCtrl.skbModel != NULL) && diagClientsTbl[DIAG_DSL_CLIENT].isRegistered;
   }

   if (allocSkbPool) {
      diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev =
         BcmDiagDevSkbAllocate(BcmDiagCtrl.skbModel, SKB_PRE_ALLOC_SIZE, NUM_OF_SKBS_IN_POOL,
            SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
            DIAG_DATA_ALIGN_MASK, DIAG_FRAME_HEADER_LEN, 0, 0);
   }
   else
      diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev = NULL;
   return dev;
}

struct net_device *BcmDiagGdbInit(PDIAGDRV_DIAG pBcmDiag)
{
   struct net_device *dev = NULL;

   printk("%s: skb reroute is used = %d\n",__FUNCTION__, (pBcmDiag->srvIpAddr == 0) );

   //GdbStubInit();  // This will be handled in the client's callback

   if ( pBcmDiag->srvIpAddr == 0 ) {
      BcmDiagCtrl.skbGdbReroute = 1;
      if ( ! BcmDiagCtrl.skbGdb )
         BcmDiagAllocateBlankSkb(&BcmDiagCtrl.skbGdb);
      BcmDiagCtrl.skbGdb->dev = NULL;
      // Indicating the corresponding skb pool will be re-routed
      SKB_REROUTE_FIELD(BcmDiagCtrl.skbGdb) = 0;
      // Preserving client's ip, which is expected to be passed in gwIpAddr field
      SKB_REROUTE_CLIENT_ADDR_FIELD(BcmDiagCtrl.skbGdb) = pBcmDiag->gwIpAddr;
      // Indicating the corresponding skb pool is for GDB
      SKB_REROUTE_DEST_FIELD(BcmDiagCtrl.skbGdb) = GDB_SKB_REROUTE_DATA;
   }
   else {
      BcmDiagCtrl.skbGdbReroute = 0;
      dev = BcmDiagInitNetDev(pBcmDiag, GDB_PORT, &BcmDiagCtrl.skbGdb, "Gdb");
      if(NULL != dev)
        BcmDiagCtrl.skbGdb->dev = dev;
   }

   return dev;
}

int BcmDiagGdbIsConnected(void)
{
	return BcmDiagCtrl.skbGdb && ( (BcmDiagCtrl.skbGdbReroute) || (NULL != BcmDiagCtrl.gdbDev) );
}

int BcmDiagIsActive(void)
{
	return BcmDiagCtrl.skbModel && ( (BcmDiagCtrl.skbModelReroute) || (NULL != BcmDiagCtrl.dbgDev) );
}

unsigned int BcmDiagGetSrvIpAddr(void)
{
   uint32 res = 0;
   if(NULL != BcmDiagCtrl.skbModel) {
      if ( BcmDiagCtrl.skbModelReroute ) {
         res = SKB_REROUTE_CLIENT_ADDR_FIELD(BcmDiagCtrl.skbModel);
      }
      else {
         diagSockFrame *dd = (diagSockFrame *) DiagPacketStart(BcmDiagCtrl.skbModel);
         res = dd->dsfh.ipHdr.dstAddr;
      }
         res = htonl(res);
   }
   return res;
}

void BcmDiagDisconnect(int keepDiagConInfo, PDIAGDRV_DIAG pBcmDiag)
{
	BcmDiagDrvSkbPool * pSkbDev;
	int clientType;

	if(LOG_CMD_DISCONNECT != pBcmDiag->diagCmd)
		return;

	BcmDiagSyncEnter();
	BcmDiagCtrl.dbgDev = NULL;
	BcmDiagCtrl.skbModelReroute = 0;
	if(!keepDiagConInfo)
		memset((void*)&diagConnectInfo, 0, sizeof(diagConnectInfo));
	diagLockSession = 0;
	diagRegressionLock = 0;
	gBcmDiagTimer.conMonitorPauseTimerCnt = 0;
	BcmDiagSyncExit();

	for(clientType = 0; clientType < DIAG_MAX_CLIENT; clientType++) {
		BcmDiagSyncEnter();
		pSkbDev = diagClientsTbl[clientType].diagSkbDev;
		diagClientsTbl[clientType].diagSkbDev = NULL;
		BcmDiagSyncExit();
		if(diagClientsTbl[clientType].isRegistered && (NULL != diagClientsTbl[clientType].pCallback))
			diagClientsTbl[clientType].pCallback(0, (void*)pBcmDiag);
		if (pSkbDev)
			BcmDiagDevSkbFree(pSkbDev, 1);
	}
}

static uint BcmDiagGetAnyRegisteredClientType(void)
{
   uint clientType;
   
   for (clientType = 0; clientType < DIAG_MAX_CLIENT; clientType++) {
      if (diagClientsTbl[clientType].diagSkbDev)
         break;
   }
   
   return clientType;
}

void BcmDiagCallBackClient(unsigned char lineId, uint clientType, PDIAGDRV_DIAG pBcmDiag)
{
	if(clientType >= DIAG_MAX_CLIENT)
		return;
	
	if (diagClientsTbl[clientType].isRegistered)
		diagClientsTbl[clientType].pCallback(lineId, (void *)pBcmDiag);
	else
		BcmDiagWriteString(lineId, BcmDiagGetAnyRegisteredClientType(), "Client(%d) is not registered! Ignoring its command\n", clientType);
}

BCMDIAG_STATUS BcmDiagProcessCmd(PDIAGDRV_DIAG pBcmDiag)
{
   uint  map, clientType, tmpBuf[32];  // 128 bytes
   int      dstPort, i, srvIpAddr;
   diagSockFrame  *dd;
   unsigned char lineId;
   struct net_device *pDev = NULL;
   char     buf[16], *tmpStr;
   BCMDIAG_STATUS res = BCMDIAG_STATUS_SUCCESS;

   gDiagProcessingCmd = 1;
   BcmDiagCmdNotify();
   dstPort  = ((uint) pBcmDiag->diagMap) >> 16;
   map      = (uint) pBcmDiag->diagMap & 0xFFFF;
   lineId   = (unsigned char) (((uint)pBcmDiag->diagCmd & DIAGS_LINE_MASK) >> DIAGS_LINE_SHIFT);
   clientType           = (uint) ((pBcmDiag->diagCmd & DIAG_TYPE_MASK) >> DIAG_TYPE_SHIFT);
   pBcmDiag->diagCmd   &= ~(DIAGS_LINE_MASK | DIAG_TYPE_MASK);
   tmpStr = (char *)&tmpBuf[0];

   switch (pBcmDiag->diagCmd) {
      case LOG_CMD_DIAG_CONNECT_INFO:
         memcpy((void *)&diagConnectInfo, (void *)pBcmDiag->diagMap, sizeof(diagConnectInfo));
         printk ("From DslDiagd: dstMACAddr = %02X:%02X:%02X:%02X:%02X:%02X, devName = %s\n",
            diagConnectInfo.macAddr[0], diagConnectInfo.macAddr[1], diagConnectInfo.macAddr[2],
            diagConnectInfo.macAddr[3], diagConnectInfo.macAddr[4], diagConnectInfo.macAddr[5],
            diagConnectInfo.devName);
         break;
      case LOG_CMD_CONNECT:
      {
         int skb_reroute = (pBcmDiag->srvIpAddr == 0);
         srvIpAddr = skb_reroute ? pBcmDiag->gwIpAddr: pBcmDiag->srvIpAddr;

         printk("CONNECT request from srvIpAddr=%s, dstPort=%d, diagMap=0x%08X\n",
            ConvertToDottedIpAddr(ntohl(srvIpAddr), buf), dstPort, (uint)map);
         if((0 != diagConnectInfo.devName[0]) &&
            (0 == (diagConnectInfo.macAddr[0] | diagConnectInfo.macAddr[1] | diagConnectInfo.macAddr[2] | diagConnectInfo.macAddr[3] | diagConnectInfo.macAddr[4] | diagConnectInfo.macAddr[5]))) {
            printk("*** Skip processing CONNECT request due to invalid dstMACAddr! ***\n");
            break;
         }
         
         printk ("***CONNECT: DSL:%d, SRDS_ETH:%d, XTM:%d, SRDS_PCIE:%d\n",
               diagClientsTbl[0].isRegistered,
               diagClientsTbl[1].isRegistered,
               diagClientsTbl[2].isRegistered,
               diagClientsTbl[3].isRegistered);

         if (pBcmDiag->diagMap & DIAG_DATA_GDB_ID) {
            BcmDiagSyncEnter();
            pDev = BcmDiagGdbInit(pBcmDiag);
            if( skb_reroute || (NULL != pDev) )
               BcmDiagCtrl.gdbDev = pDev;
            BcmDiagSyncExit();
            if( ((NULL != BcmDiagCtrl.gdbDev) || (skb_reroute && BcmDiagCtrl.skbGdb)) &&
               diagClientsTbl[clientType].isRegistered && (NULL != diagClientsTbl[clientType].pCallback) ) {
               pBcmDiag->skbModel = (void *)BcmDiagCtrl.skbGdb;
               diagClientsTbl[clientType].pCallback(lineId, (void *)pBcmDiag);
            }
            gDiagProcessingCmd=0;
            return res;
         }
         else {
            dd = (NULL != BcmDiagCtrl.skbModel) ? (diagSockFrame *) DiagPacketStart(BcmDiagCtrl.skbModel): NULL;
            if ( (0 == diagLockSession) ||
               ( BcmDiagIsActive() && (srvIpAddr == dd->dsfh.ipHdr.dstAddr) ) ) {
               pBcmDiag->diagMap = (int)((uint)pBcmDiag->diagMap & ~((uint)DIAG_LOCK_SESSION));
               if(BcmDiagIsActive() && (srvIpAddr == dd->dsfh.ipHdr.dstAddr)) {
                  printk("Re-connecting to srvIpAddr %s\n", ConvertToDottedIpAddr(ntohl(srvIpAddr),buf));
                  BcmDiagDisconnect(1, pBcmDiag);
               }
               else
                  printk("Connecting to srvIpAddr %s\n", ConvertToDottedIpAddr(ntohl(srvIpAddr),buf));
               
               BcmDiagSyncEnter();
               pDev = BcmDiagInit(pBcmDiag);
               if( (skb_reroute || (NULL != pDev)) && (NULL != BcmDiagCtrl.skbModel) ) {
                  diagLockSession = 1;
                  BcmDiagCtrl.dbgDev = pDev;
                  for(i = 0; i < DIAG_MAX_CLIENT; i++) {
                     if(DIAG_DSL_CLIENT == i)
                        continue;   // Already done in BcmDiagInit()
                     if( diagClientsTbl[i].isRegistered && !skb_reroute )
                        diagClientsTbl[i].diagSkbDev = BcmDiagDevSkbAllocate(BcmDiagCtrl.skbModel,
                          SKB_PRE_ALLOC_SIZE, NUM_OF_SKBS_IN_POOL,
                          SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
                          DIAG_DATA_ALIGN_MASK, DIAG_FRAME_HEADER_LEN, 0, 0);
                     else
                        diagClientsTbl[i].diagSkbDev = NULL;
                  }
               }
               BcmDiagSyncExit();
               
               sprintf(tmpStr, "%s: pDev=0x%08x diagCtrl.skbModel=0x%08x\n", __FUNCTION__, (unsigned int)(uintptr_t)pDev, (unsigned int)(uintptr_t)BcmDiagCtrl.skbModel);
               printk("%s", tmpStr);
               clientType = BcmDiagGetAnyRegisteredClientType();
               BcmDiagWriteString(lineId, clientType, tmpStr);
            }
            else {
               static struct sk_buff *skbModelNew = NULL;
               struct sk_buff *skbModelTmp = BcmDiagCtrl.skbModel;
               BcmDiagDrvSkbPool *pDrvSkbPoolTmp = diagClientsTbl[clientType].diagSkbDev;
               BcmDiagDrvSkbPool *pDrvSkbPoolTmp1 = NULL;
               
               pDev = BcmDiagCtrl.dbgDev;
               if(NULL == pDrvSkbPoolTmp) {
                  clientType = BcmDiagGetAnyRegisteredClientType();
                  pDrvSkbPoolTmp = diagClientsTbl[clientType].diagSkbDev;
               }
#if 1
              if ( skb_reroute )
                 printk("%s: skb_reroute not handling this yet(reconn)\n",__FUNCTION__);
#endif
               sprintf(tmpStr, "Reject connection request from srvIpAddr %s\n", ConvertToDottedIpAddr(ntohl(pBcmDiag->srvIpAddr),buf));
               printk("%s", tmpStr);
               BcmDiagWriteString(lineId, clientType, tmpStr);
               
               sprintf(tmpStr, "Connection is being locked by srvIpAddr %s\n", (dd) ? ConvertToDottedIpAddr(ntohl(dd->dsfh.ipHdr.dstAddr),buf): "Unknown" );
               printk("%s", tmpStr);
               BcmDiagWriteString(lineId, clientType, tmpStr);
               
               sprintf(tmpStr,"Temporarily connect to rejected srvIpAddr %s to deliver unlock instructions\n", ConvertToDottedIpAddr(ntohl(pBcmDiag->srvIpAddr),buf));
               printk("%s", tmpStr);
               BcmDiagWriteString(lineId, clientType, tmpStr);
               
               //BcmDiagSyncEnter();
               if((NULL != pDrvSkbPoolTmp) && (NULL != (BcmDiagCtrl.dbgDev = BcmDiagInitNetDev(pBcmDiag, LOG_FILE_PORT, &skbModelNew, "Diag")))) {
                  BcmDiagCtrl.skbModel = skbModelNew;
                  diagClientsTbl[clientType].diagSkbDev = BcmDiagDevSkbAllocate(BcmDiagCtrl.skbModel, 0, 0,
                     SHORT_SKB_PRE_ALLOC_SIZE, 5, pDrvSkbPoolTmp->dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
                  BcmDiagWriteString(lineId, clientType, "Connection request is rejected!!!\n");
                  BcmDiagWriteString(lineId, clientType, "Connection is being locked by srvIpAddr %s\n", (dd) ? ConvertToDottedIpAddr(ntohl(dd->dsfh.ipHdr.dstAddr),buf): "Unkown");
                  if(!diagRegressionLock)
                     BcmDiagWriteString(lineId, clientType, "To establish a new connection, unlock the current connection(File Menu)\n");
                  else
                     BcmDiagWriteString(lineId, clientType, "The target is being locked for regression testing, please contact the owner\n");
                  
                  pDrvSkbPoolTmp1 = diagClientsTbl[clientType].diagSkbDev;
                  
                  diagClientsTbl[clientType].diagSkbDev = pDrvSkbPoolTmp;
                  BcmDiagCtrl.skbModel = skbModelTmp;
                  BcmDiagCtrl.dbgDev = pDev;
               }
               //BcmDiagSyncExit();
               
               if(NULL != pDrvSkbPoolTmp1)
                  BcmDiagDevSkbFree(pDrvSkbPoolTmp1, 1);
               gDiagProcessingCmd=0;
               return res;
            }
         }
         if( (skb_reroute || (NULL != BcmDiagCtrl.dbgDev)) && (NULL != BcmDiagCtrl.skbModel) ) {
            pBcmDiag->skbModel = (void *)BcmDiagCtrl.skbModel;
            for(i = 0; i < DIAG_MAX_CLIENT; i++) {
               if( diagClientsTbl[i].isRegistered && (NULL != diagClientsTbl[i].pCallback) )
                  diagClientsTbl[i].pCallback(lineId, (void *)pBcmDiag);
            }
         }
      } /* end of case LOG_CMD_CONNECT */
         break;
      case LOG_CMD_ENABLE_CLIENT: /* sent after connect command */
      {
         char   clientId;
         int    dataAlignMask = DIAG_DATA_ALIGN_MASK;
         
         if (BcmDiagCtrl.skbModel != NULL) {
            clientId = *((char *)pBcmDiag->diagMap);
            if ((clientId & DIAG_DATA_DSL_ID) && diagClientsTbl[DIAG_DSL_CLIENT].isRegistered &&
                  !diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev) {
               printk("DSL Diag Enable Request Received\n");
               diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev = BcmDiagDevSkbAllocate(BcmDiagCtrl.skbModel, SKB_PRE_ALLOC_SIZE,
                     NUM_OF_SKBS_IN_POOL, SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
                     dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
               diagClientsTbl[DIAG_DSL_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            }
            if ((clientId & DIAG_DATA_XTM_ID) && diagClientsTbl[DIAG_XTM_CLIENT].isRegistered &&
                  !diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev) {
               printk("XTM Diag Enable Request Received\n");
               diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev = BcmDiagDevSkbAllocate(BcmDiagCtrl.skbModel, SKB_PRE_ALLOC_SIZE,
                     NUM_OF_SKBS_IN_POOL, SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
                     dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
               diagClientsTbl[DIAG_XTM_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            }
            if ((clientId & DIAG_DATA_SRDS_ETH_ID) && diagClientsTbl[DIAG_SRDS_ETH_CLIENT].isRegistered &&
                  !diagClientsTbl[DIAG_SRDS_ETH_CLIENT].diagSkbDev) {
               printk("SRDS_ETH Diag Enable Request Received\n");
               diagClientsTbl[DIAG_SRDS_ETH_CLIENT].diagSkbDev = BcmDiagDevSkbAllocate(BcmDiagCtrl.skbModel, SKB_PRE_ALLOC_SIZE,
                     NUM_OF_SKBS_IN_POOL, SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
                     dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
               diagClientsTbl[DIAG_SRDS_ETH_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            }
            if ((clientId & DIAG_DATA_SRDS_PCIE_ID) && diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].isRegistered &&
                  !diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].diagSkbDev) {
               printk("SRDS_PCIE Diag Enable Request Received\n");
               diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].diagSkbDev = BcmDiagDevSkbAllocate(BcmDiagCtrl.skbModel, SKB_PRE_ALLOC_SIZE,
                     NUM_OF_SKBS_IN_POOL, SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
                     dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
               diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            }
         }
      }
         break;
      case LOG_CMD_DISABLE_CLIENT:
      {
         BcmDiagDrvSkbPool * pSkbDev;
         char clientId;

         clientId = *((char *)pBcmDiag->diagMap);
         if ((clientId & DIAG_DATA_DSL_ID) && (diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev)) {
            printk("DSL Diag Disable Request Received\n");
            diagClientsTbl[DIAG_DSL_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            pSkbDev = diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev;
            diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev = NULL;
            BcmDiagDevSkbFree(pSkbDev,1);
         }
         if ((clientId & DIAG_DATA_SRDS_ETH_ID) && (diagClientsTbl[DIAG_SRDS_ETH_CLIENT].diagSkbDev)) {
            printk("SRDS_ETH Diag Disable Request Received\n");
            diagClientsTbl[DIAG_SRDS_ETH_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            pSkbDev = diagClientsTbl[DIAG_SRDS_ETH_CLIENT].diagSkbDev;
            diagClientsTbl[DIAG_SRDS_ETH_CLIENT].diagSkbDev = NULL;
            BcmDiagDevSkbFree(pSkbDev,1);
         }
         if ((clientId & DIAG_DATA_XTM_ID) && (diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev)) {
            printk("XTM Diag Disable Request Received\n");
            diagClientsTbl[DIAG_XTM_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            pSkbDev = diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev;
            diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev = NULL;
            BcmDiagDevSkbFree(pSkbDev,1);
         }
         if ((clientId & DIAG_DATA_SRDS_PCIE_ID) && (diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].diagSkbDev)) {
            printk("SRDS_PCIE Diag Disable Request Received\n");
            diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].pCallback(lineId, (void *)pBcmDiag);
            pSkbDev = diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].diagSkbDev;
            diagClientsTbl[DIAG_SRDS_PCIE_CLIENT].diagSkbDev = NULL;
            BcmDiagDevSkbFree(pSkbDev,1);
         }
      }
         break;
      case LOG_CMD_PING_REQ:
         if (NULL == diagClientsTbl[clientType].diagSkbDev)
            clientType = BcmDiagGetAnyRegisteredClientType();
         BcmDiagWriteData(((clientType << DIAG_DRV_TYPE_SHIFT) | LOG_CMD_PING_RSP), buf, 2, NULL, 0);
         break;
      case LOG_CMD_DISCONNECT:
      {
         int ip_mismatch = 0;

         srvIpAddr = (BcmDiagIsActive() && BcmDiagCtrl.skbModelReroute) ? SKB_REROUTE_CLIENT_ADDR_FIELD(BcmDiagCtrl.skbModel): pBcmDiag->srvIpAddr;

         printk ("DISCONNECT Request(%s) from srvIpAddr=%s diagLockSession=%d regressionLock=%d\n",
            (map & DIAG_FORCE_DISCONNECT) ? "forced": "not-forced",
            ConvertToDottedIpAddr(ntohl(srvIpAddr), buf),
            diagLockSession, diagRegressionLock);
         
         if( !BcmDiagIsActive() ) {
            printk ("CPE is already in Diags disconnected state!\n");
            break;
         }
         
         if(NULL != BcmDiagCtrl.skbModel) {
            dd = (diagSockFrame *) DiagPacketStart(BcmDiagCtrl.skbModel);
            if ( BcmDiagCtrl.skbModelReroute )
               ip_mismatch = (srvIpAddr != SKB_REROUTE_CLIENT_ADDR_FIELD(BcmDiagCtrl.skbModel));
            else
               ip_mismatch = (srvIpAddr != dd->dsfh.ipHdr.dstAddr);
         }
         else
            dd = NULL;
         
         if( (NULL != dd) && ip_mismatch )
            BcmDiagWriteString(lineId, clientType, "DISCONNECT Request(%s) from srvIpAddr = %s diagLockSession=%d regressionLock=%d\n",
               (map & DIAG_FORCE_DISCONNECT) ? "forced": "not-forced",
               ConvertToDottedIpAddr(ntohl(srvIpAddr), buf), diagLockSession, diagRegressionLock);
         
         if( 0 == (map & DIAG_FORCE_DISCONNECT) ) {
            if( diagLockSession && ip_mismatch )
                break;   /* Reject DISCONNECT */
         }
         else if( diagRegressionLock && ip_mismatch )
               break;   /* Reject DISCONNECT */
         
         BcmDiagDisconnect(0, pBcmDiag);
      } // end of case LOG_CMD_DISCONNECT
         break;
      case LOG_CMD_DEBUG:
      {
         DiagDebugData  *pDbgCmd = (void *)pBcmDiag->diagMap;
         
         //printk("%s: Executing dbgcmd=%d cmdId=%d param1=0x%08x param2=0x%08x\n", __FUNCTION__, pDbgCmd->cmd, pDbgCmd->cmdId, pDbgCmd->param1, pDbgCmd->param2);
         
         switch (pDbgCmd->cmd) {
            case DIAG_DEBUG_CMD_PAUSE_CON_MONITOR:
                BcmDiagSyncEnter();
                gBcmDiagTimer.conMonitorPauseTimerCnt = pDbgCmd->param1;
                BcmDiagSyncExit();
                break;
            case DIAG_DEBUG_CMD_SYNC_CPE_TIME:
               BcmDiagSetSyncTime(pDbgCmd->param1);
               break;
            case DIAG_DEBUG_CMD_CLEAR_STAT:
               BcmDiagClearStats(clientType);
               break;
            case DIAG_DEBUG_CMD_PRINT_STAT:
               BcmDiagPrintStats(clientType);
               break;
            case DIAG_DEBUG_CMD_SET_REGRESSION_LOCK:
               diagRegressionLock = 1;
               break;
            case DIAG_DEBUG_CMD_CLR_REGRESSION_LOCK:
               diagRegressionLock = 0;
               diagLockSession = 0;
               break;
            case DIAG_DEBUG_CMD_EXEC_FUNC:
            {
               int res, (*pExecFunc) (uint param) = (void *)(uintptr_t)(pDbgCmd->param1);
            
               res = (*pExecFunc) (pDbgCmd->param2);
               BcmDiagWriteString(lineId, DIAG_DSL_CLIENT, "CMD_EXEC_FUNC at 0x%px param=%d: result=%d", pExecFunc, pDbgCmd->param2, res);
            }
               break;
            default:
               if((DIAG_DEBUG_CMD_READ_MEM == pDbgCmd->cmd) && (CMDID_READ_MEM_CHIPID == pDbgCmd->cmdId)) {
                  DiagDebugData  *pDbgRsp = (DiagDebugData *)&tmpBuf[0];
                  pDbgRsp->cmd = DIAG_ENDIAN_CONV_SHORT(pDbgCmd->cmd);
                  pDbgRsp->cmdId = DIAG_ENDIAN_CONV_SHORT(pDbgCmd->cmdId);
                  pDbgRsp->param1 = DIAG_ENDIAN_CONV_INT32(pDbgCmd->param1);
                  pDbgRsp->param2 = DIAG_ENDIAN_CONV_INT32(8);
                  tmpBuf[3] = DIAG_ENDIAN_CONV_INT32(UtilGetChipIdRaw());
                  tmpBuf[4] = DIAG_ENDIAN_CONV_INT32(UtilGetChipId());
                  clientType = BcmDiagGetAnyRegisteredClientType();
                  BcmDiagWriteData(((lineId << DIAG_DRV_LINE_SHIFT) | (clientType << DIAG_DRV_TYPE_SHIFT) | LOG_CMD_DEBUG), (char *)&tmpBuf[0], 20, NULL, 0);
               }
               else
                  BcmDiagCallBackClient(lineId, clientType, pBcmDiag);
               break;
         }
      } // end of case LOG_CMD_DEBUG
         break;
      default:
         BcmDiagCallBackClient(lineId, clientType, pBcmDiag);
         break;
   }
   gDiagProcessingCmd=0;
   return res;
}

void BcmDiagDrvInit(void)
{
   BcmDiagSyncInit();
   memset((void *)&gBcmDiagTimer, 0, sizeof(gBcmDiagTimer));
   memset((void *)&diagClientsTbl, 0, sizeof(diagClientsTbl));
   memset((void *)&gBcmDiag, 0, sizeof(gBcmDiag));
   memset((void *)&gDbgCmd, 0, sizeof(gDbgCmd));
   gBcmDiag.diagCmd  = LOG_CMD_DEBUG;
   gBcmDiag.diagMap  = (unsigned long) &gDbgCmd;
   gDbgCmd.cmd    = DIAG_DEBUG_CMD_TIMER_EXPIRE;
   gDbgCmd.param1 = TIMER_EXPIRE_1000MS;
   gBcmDiagTimer.pDiagClientsTbl = diagClientsTbl;
   gBcmDiagTimer.lowestTimerIntervalInJiffie = HZ;    // One second in jiffie unit.
   gBcmDiagTimer.timerExpireCntMaxMT = 1;
}

#define DIAG_NOCMD_TO	5000	// # of milliseconds
#define DIAG_PING_TIME	1000
#define DIAG_DISC_TIME	10000

static void diagConnectionCheck(void)
{
	char	buf[4];
	unsigned int	timeElapseMs;
	unsigned long	curTime, syncTimeSec;

	if(gDiagProcessingCmd) {
		BcmDiagCmdNotify();
		return;
	}

	if(BcmDiagIsActive()) {
		curTime = jiffies;
		syncTimeSec = BcmDiagGetSyncTime();
		timeElapseMs = jiffies_to_msecs(curTime - gDiagLastCmdTime);
		if (timeElapseMs > DIAG_NOCMD_TO) {
			if (timeElapseMs > DIAG_DISC_TIME) {
				DIAGDRV_DIAG	bcmDiag;
				bcmDiag.diagCmd = LOG_CMD_DISCONNECT;
				printk ("BcmDiag timeout disconnect: syncTime= %lu sec, to=%u ms, currTick=%lu , gDiagLastCmdTime=%lu\n",
					syncTimeSec, timeElapseMs, curTime, gDiagLastCmdTime);
				BcmDiagDisconnect(0, &bcmDiag);
			}
			else if (jiffies_to_msecs(curTime - gDiagLastPingTime) > DIAG_PING_TIME) {
				printk ("BcmDiag ping request: syncTime= %lu sec, currTick=%lu, gDiagLastPingTime=%lu, gDiagLastCmdTime=%lu\n",
					syncTimeSec, curTime, gDiagLastPingTime, gDiagLastCmdTime);
				gDiagLastPingTime = curTime;
				BcmDiagWriteData(LOG_CMD_PING_REQ, buf, 2, NULL, 0);
			}
		}
	}
}


#if defined(CONFIG_ARM64)

#define BlockByteMove(len,src,dst)	BlockMemCopy(dst,src,len, (-((long) (src))) & 0x7)

#define CPY1(d,s) do {												\
	register int tmp;												\
	__asm volatile("ldrb  %w0, [%x1], #1" : "=r" (tmp), "+r" (s));	\
	__asm volatile("strb  %w1, [%x0], #1" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY2(d,s) do {												\
	register int  tmp;												\
	__asm volatile("ldrh  %w0, [%x1], #2" : "=r" (tmp), "+r" (s));	\
	__asm volatile("strh  %w1, [%x0], #2" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY4(d,s) do {												\
	register int  tmp;												\
	__asm volatile("ldr  %w0, [%x1], #4" : "=r" (tmp), "+r" (s));	\
	__asm volatile("str  %w1, [%x0], #4" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY8(d,s) do {												\
	register long  tmp;												\
	__asm volatile("ldr  %x0, [%x1], #8" : "=r" (tmp), "+r" (s));	\
	__asm volatile("str  %x1, [%x0], #8" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY16(d,s) do {																	\
	register long  tmp1, tmp2;															\
	__asm volatile("ldp  %x0, %x1, [%x2], #16" : "=r" (tmp1), "=r" (tmp2), "+r" (s));	\
	__asm volatile("stp  %x1, %x2, [%x0], #16" : "+r" (d) : "r" (tmp1), "r" (tmp2));	\
} while (0)

static void *BlockMemCopy(void *dst, void *src, int size, int nAlign)
{
	char *dstPtr = dst, *srcPtr = src;
	char *src64End;

	if (size < nAlign) {
		char *srcEndPtr = srcPtr + size;
		while (srcPtr != srcEndPtr)
		  CPY1(dstPtr, srcPtr);
		return dst;
	}

	if (nAlign) {
	  if (nAlign & 1)
		CPY1(dstPtr, srcPtr);
	  if (nAlign & 2)
		CPY2(dstPtr, srcPtr);
	  if (nAlign & 4)
		CPY4(dstPtr, srcPtr);
	  size -= nAlign;
	}

	if (size >=64) {
		/* copy by 64 bytes */
	    src64End = srcPtr + (size & ~0x3F);
		do {
			CPY16(dstPtr, srcPtr);
			CPY16(dstPtr, srcPtr);
			CPY16(dstPtr, srcPtr);
			CPY16(dstPtr, srcPtr);
		} while (srcPtr != src64End);
	}
	/* copy remaining by 16 bytes */
	if (size & 0x20) {
		CPY16(dstPtr, srcPtr);
		CPY16(dstPtr, srcPtr);
	}
	if (size & 0x10)
		CPY16(dstPtr, srcPtr);

	/* copy remaining */
	if (size & 8)    // >= 8
		CPY8(dstPtr, srcPtr);
	if (size & 4)    // >= 4
		CPY4(dstPtr, srcPtr);
	if (size & 2)    // >= 2
		CPY2(dstPtr, srcPtr);
	if (size & 1)    // >= 1
		CPY1(dstPtr, srcPtr);

	return dst;
}

#endif

static ushort BcmDiagIpComputeChecksum(diagIpHeader *ipHdr)
{
	ushort	*pHdr = (ushort	*) ipHdr;
	ushort	*pHdrEnd = pHdr + 10;
	uint	sum = 0;

	do {
		sum += pHdr[0];
		sum += pHdr[1];
		pHdr += 2;
	} while (pHdr != pHdrEnd);

	while (sum > 0xFFFF)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return sum ^ 0xFFFF;
}

static ushort BcmDiagIpUpdateChecksum(int sum, ushort oldv, ushort newv)
{
	ushort	tmp;

	tmp = (newv - oldv);
	tmp -= (tmp >> 15);
	sum = (sum ^ 0xFFFF) + tmp;
	sum = (sum & 0xFFFF) + (sum >> 16);
	return sum ^ 0xFFFF;
}

static void BcmDiagUpdateDataLen(diagSockFrame *diagFr, int dataLen)
{
	int	newIpHdrLen = dataLen + sizeof(LogProtoHeader) + sizeof(diagUdpHeader) + sizeof(diagIpHeader);
	
	diagFr->dsfh.ipHdr.checksum = htons(BcmDiagIpUpdateChecksum(htons(diagFr->dsfh.ipHdr.checksum), htons((ushort)diagFr->dsfh.ipHdr.len), newIpHdrLen));
	diagFr->dsfh.ipHdr.len = htons(newIpHdrLen);
	diagFr->dsfh.udpHdr.len = htons(newIpHdrLen - sizeof(diagIpHeader));
}

static int __BcmDiagGdbWriteData(struct sk_buff *skb, char *buf0, int len0)
{
	diagSockFrame	*dd;
	int				n;

	dd = (diagSockFrame *)((uintptr_t) skb->data-DIAG_FRAME_PAD_SIZE);
	if(BcmDiagCtrl.skbGdbReroute)
		dd->dsfh.udpHdr.len = len0 + sizeof(diagUdpHeader);
	else
		BcmDiagUpdateDataLen(dd, len0 - sizeof(LogProtoHeader));

	memcpy (dd->diagData-sizeof(LogProtoHeader), buf0, len0);
	if(!BcmDiagCtrl.skbGdbReroute) {
		skb->len = DIAG_FRAME_HEADER_LEN + len0 - sizeof(LogProtoHeader);
		skb_set_tail_pointer(skb, skb->len);
	}
	n = DEV_TRANSMIT(skb);

	return n;
}

int BcmDiagGdbWriteData(char *buf0, int len0)
{
	struct sk_buff *skb;
#ifdef DEBUG_GDB_STUB
	int i;
#endif
	int dataAlignMask = DIAG_DATA_ALIGN_MASK;
	
	BcmDiagSyncEnter();
	if ( ! BcmDiagGdbIsConnected() ) {
		BcmDiagSyncExit();
		return 0;
	}
#ifdef DEBUG_GDB_STUB
	printk("GdbWriteData::'");
	for (i = 0; i < len0; i++)
		printk("%c", buf0[i]);
	printk("' len=%d\n", len0);
#endif

	skb = alloc_skb (dataAlignMask + DIAG_FRAME_HEADER_LEN + len0 + 16, GFP_ATOMIC);
	if (NULL == skb) {
		BcmDiagSyncExit();
		return -ENOMEM;
	}
	
	skb->dev = BcmDiagCtrl.gdbDev;
	skb->protocol = (BcmDiagCtrl.skbGdbReroute) ? BcmDiagCtrl.skbGdb->protocol: htons(eth_type_trans (skb, BcmDiagCtrl.gdbDev));
	skb->data = DIAG_DATA_ALIGN(skb->head, dataAlignMask);
	memcpy(skb->data, BcmDiagCtrl.skbGdb->data, DIAG_FRAME_HEADER_LEN);
	BcmDiagSyncExit();
	return __BcmDiagGdbWriteData(skb, buf0, len0);

}

static int __BcmDiagWriteData(struct sk_buff *skbDiag, uint cmd, char *buf0, int len0, char *buf1, int len1)
{
	diagSockFrame		*dd;
	int					n,totalBufLen;
	uint				lineId, clientType;
	
	if (skbDiag == NULL)
		return 0;
	dd = (diagSockFrame *) ((uintptr_t)skbDiag->data-DIAG_FRAME_PAD_SIZE);
	totalBufLen = len0+len1;
	if(BcmDiagCtrl.skbModelReroute)
		dd->dsfh.udpHdr.len = totalBufLen + sizeof(LogProtoHeader) + sizeof(diagUdpHeader);
	else
		BcmDiagUpdateDataLen(dd, totalBufLen);

	if (cmd & DIAG_DRV_MSG_NUM) {
		*(short *)dd->dsfh.diagHdr.logProtoId = (ushort)BcmDiagCtrl.diagDmaLogBlockNum;
		BcmDiagCtrl.diagDmaLogBlockNum++;
	}
	else {
		*(short *)dd->dsfh.diagHdr.logProtoId = *(short *) LOG_PROTO_ID;
		if (cmd & DIAG_DRV_SPLIT_MSG)
			dd->dsfh.diagHdr.logProtoId[1] += 1;
	}
	lineId = (cmd & DIAG_DRV_LINE_MASK) >> DIAG_DRV_LINE_SHIFT;
	clientType = (cmd & DIAG_DRV_TYPE_MASK) >> DIAG_DRV_TYPE_SHIFT;
	dd->dsfh.diagHdr.logPartyId = LOG_PARTY_CLIENT | (lineId << DIAG_PARTY_LINEID_SHIFT) | (clientType << DIAG_PARTY_TYPE_SHIFT);
	dd->dsfh.diagHdr.logCommmand = cmd & 0xFF;
#ifdef CONFIG_ARM64
	BlockByteMove(len0, buf0, dd->diagData);
#else
	memcpy (dd->diagData, buf0, len0);
#endif
	if (NULL != buf1) {
#ifdef CONFIG_ARM64
		BlockByteMove(len1, buf1, dd->diagData+len0);
#else
		memcpy (dd->diagData+len0, buf1, len1);
#endif
	}
	
	if(BcmDiagCtrl.skbModelReroute)
		skbDiag->len += totalBufLen;
	else {
		if(totalBufLen < MIN_DAIG_DATA_LEN) {
			int padSize = MIN_DAIG_DATA_LEN - totalBufLen;
			char *pDiagDataEnd = dd->diagData + totalBufLen;
			memset(pDiagDataEnd, 0, padSize);
			totalBufLen += padSize;
		}
		skbDiag->len += totalBufLen;
		skb_set_tail_pointer(skbDiag, skbDiag->len);
#if defined(VECTORING)
		/* Assumming VECTORING is built based on new Linux where this function is available in skbuff.h */
		/* This is to prevent "protocol x is buggy" prints from the kernel */
		skb_set_network_header(skbDiag, sizeof(struct ethhdr));
#endif
	}
	/* DiagPrintData(skbDiag); */
	n = DEV_TRANSMIT(skbDiag);
	return n;
}

/* len is total length not including frame and pad */
static struct sk_buff * BcmDiagGetSkbForReroute(int len)
{
	struct sk_buff *skb;

	int dataAlignMask = DIAG_DATA_ALIGN_MASK;

	if (NULL == (skb = alloc_skb (dataAlignMask + DIAG_FRAME_HEADER_LEN + len + 16, GFP_ATOMIC)))
		return skb;
	
	skb->dev = BcmDiagCtrl.dbgDev;
	skb->protocol = BcmDiagCtrl.skbModel->protocol;
	skb->data = DIAG_DATA_ALIGN(skb->head, dataAlignMask);
	skb->len = DIAG_FRAME_HEADER_LEN;
	memcpy(skb->data, BcmDiagCtrl.skbModel->data, DIAG_FRAME_HEADER_LEN);
	return skb;
}

static int DevSkbSendDiagsPacket(BcmDiagDrvSkbPool *skbDev, uint cmd, diagDrvFragment *pFrag, int nFrag)
{
	struct sk_buff  *skbDiag = NULL;
	int n = 0;
	int k = 0;

	if ((((skbDev == NULL) && !BcmDiagCtrl.skbModelReroute)) || (pFrag == NULL) || (nFrag == 0))
		return 0;
	
	if (nFrag == 1) {
		if( NULL != (skbDiag = BcmDiagGetSkb(skbDev, pFrag[0].len)) )
			n = __BcmDiagWriteData(skbDiag, cmd, pFrag[0].pBuf, pFrag[0].len, NULL, 0);
		return n;
	}

	if ((nFrag == 2) &&  ((pFrag[0].len + pFrag[1].len) <= LOG_MAX_DATA_SIZE)) {
		if( NULL != (skbDiag = BcmDiagGetSkb(skbDev, (pFrag[0].len + pFrag[1].len))) )
			n = __BcmDiagWriteData(skbDiag, cmd, pFrag[0].pBuf, pFrag[0].len, pFrag[1].pBuf, pFrag[1].len);
		return n;
	}
	
	if( NULL != (skbDiag = BcmDiagGetSkb(skbDev, pFrag[0].len)) )
		n = __BcmDiagWriteData(skbDiag, cmd | DIAG_DRV_SPLIT_MSG, pFrag[0].pBuf, pFrag[0].len, NULL, 0);
	else {
		return 0;
	}
	
	for (k = 1; k < (nFrag-1); k++){
		skbDiag = BcmDiagGetSkb(skbDev, pFrag[k].len);
		if (NULL != skbDiag)
			n += __BcmDiagWriteData(skbDiag, (cmd & (DIAG_DRV_LINE_MASK | DIAG_DRV_TYPE_MASK)) | (diagStatusInfoData-2), pFrag[k].pBuf, pFrag[k].len, NULL, 0);
		else {
			return n;
		}
	}
	skbDiag = BcmDiagGetSkb(skbDev, pFrag[k].len);
	if (NULL != skbDiag)
		n += __BcmDiagWriteData(skbDiag, (cmd & (DIAG_DRV_LINE_MASK | DIAG_DRV_TYPE_MASK)) | (diagStatusInfoData-3), pFrag[k].pBuf, pFrag[k].len, NULL, 0);
	
	return n;
}

static int BcmDiagBuildFragTbl(diagDrvFragment *frag, char *p0, int len0, char *p1,  int len1)
{
	int d, r, nFrag  = 0;

	if (((len0 + len1)) > (DIAG_MAX_NUM_FRAG*LOG_MAX_DATA_SIZE)) {
		return nFrag;
	}

	if (len0 > 0) {
		d = len0/LOG_MAX_DATA_SIZE;
		r = len0 - d * LOG_MAX_DATA_SIZE;
		frag[nFrag].pBuf = p0;
		frag[nFrag].len = (d > 0) ? LOG_MAX_DATA_SIZE:r;
		nFrag++;
		if (d > 0){
			for(; nFrag < d ; nFrag++){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = LOG_MAX_DATA_SIZE;
			}
			if (r > 0){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = r;
				nFrag++;
			}
		}
	}

	if (len1 > 0) {
		d = len1/LOG_MAX_DATA_SIZE;
		r = len1 - d * LOG_MAX_DATA_SIZE;
		frag[nFrag].pBuf = p1;
		frag[nFrag].len = (d > 0) ? LOG_MAX_DATA_SIZE:r;
		nFrag++;
		if (d > 0){
			int d1 = nFrag+d-1;
			for(; nFrag < d1 ; nFrag++){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = LOG_MAX_DATA_SIZE;
			}
			if (r > 0){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = r;
				nFrag++;
			}
		}
	}

	return nFrag;
}

int BcmDiagWriteData(uint  cmd, char *buf0, int len0, char *buf1, int len1)
{
	struct sk_buff  *skbDiag = NULL;
	int n = 0;
	BcmDiagDrvSkbPool	*diagSkbDev = NULL;
	uint clientType;

	clientType = (cmd & DIAG_DRV_TYPE_MASK) >> DIAG_DRV_TYPE_SHIFT;
	if(clientType >= DIAG_MAX_CLIENT)
		return n;
	
	BcmDiagSyncEnter();
	diagSkbDev = diagClientsTbl[clientType].diagSkbDev;
	if ((diagSkbDev != NULL) || (0 != BcmDiagCtrl.skbModelReroute))
		skbDiag = BcmDiagGetSkb(diagSkbDev, (len0 + len1));

	if (skbDiag != NULL)
		n = __BcmDiagWriteData(skbDiag, cmd, buf0, len0, buf1, len1);
	BcmDiagSyncExit();
	
	return n;
}

void BcmDiagWriteStatusInfo(uint cmd, char *p, int n, char *p1, int n1)
{
	diagDrvFragment frag[DIAG_MAX_NUM_FRAG];
	int nFrag;
	BcmDiagDrvSkbPool	*diagSkbDev = NULL;
	uint clientType;
	
	clientType = (cmd & DIAG_DRV_TYPE_MASK) >> DIAG_DRV_TYPE_SHIFT;
	if(clientType >= DIAG_MAX_CLIENT)
		return;
	
	BcmDiagSyncEnter();
	diagSkbDev = diagClientsTbl[clientType].diagSkbDev;
	if((NULL == diagSkbDev) && !BcmDiagCtrl.skbModelReroute) {
		BcmDiagSyncExit();
		return;
	}
	nFrag = BcmDiagBuildFragTbl(frag, p, n, p1, n1);
	DevSkbSendDiagsPacket(diagSkbDev, cmd, frag, nFrag);
	BcmDiagSyncExit();
}

int BcmDiagDevSkbFree(BcmDiagDrvSkbPool *skbDev, int enableWA){
	unsigned long	osTime0;
	int			n;
	struct sk_buff	*skb;
	struct sk_buff	*skbRef;
	struct sk_buff ** ppskb = skbDev->skbPool;
	BOOL freeSkbDev = 0;
	int totalSkbs = skbDev->numOfSkbs + skbDev->numOfShortSkbs;
	int skb_reroute = (SKB_REROUTE_FIELD(skbDev->skbModel)==0);
	
	do {
		for (n = 0; n < totalSkbs; n++) {
			skb = ppskb[n];
			if ( DIAG_SKB_USERS == refcount_read(&skb->users) )
				break;
		}
		if ( n >= totalSkbs ){
			freeSkbDev = 1;
			break;
		}
		
#ifdef DIAG_ALLOC_SKB_DYNAMICALLY
		printk("** %s: n = %d totalSkbs = %d\n", __FUNCTION__, n, totalSkbs);
		enableWA = 0;
		freeSkbDev = 1;
#endif
		if(enableWA) {
			/* Work-around: send a dummy (DIAG PING) packet so the system will release the skbs */
			skb = alloc_skb (skbDev->dataAlignMask + DIAG_FRAME_HEADER_LEN + 32, GFP_ATOMIC);
			if(skb != NULL) {
				char buf[4] = {0};
				skbRef = skbDev->skbModel;
				skb->dev = skbRef->dev;
				skb->protocol = (skb_reroute)?skbRef->protocol:htons(eth_type_trans(skb, skbRef->dev));
				skb->data = DIAG_DATA_ALIGN(skb->head, skbDev->dataAlignMask);
				memcpy(skb->data, skbRef->data, skbDev->frameHeaderLen);
				skb->len = skbDev->frameHeaderLen;
				__BcmDiagWriteData(skb, LOG_CMD_PING_REQ, buf, sizeof(buf), NULL, 0);
			}
			/* Loop 10 Ms */
			osTime0 = jiffies;
			while (jiffies_to_msecs(jiffies - osTime0) < 10);
		}
	} while (enableWA);

	if(freeSkbDev)
	{
		for (n = 0; n < totalSkbs; n++) {
			skb = ppskb[n];
			refcount_set(&skb->users, 1);
			kfree_skb(skb);
		}
		kfree(skbDev);
	}
	
	return(freeSkbDev);
}

void BcmDiagWriteString(uint lineId, uint clientType,  char *fmt, ...)
{
	uint	cmd;
	int	statStrBuf[1000/4];
	char	*statStr;
	int	n;
	va_list	ap;

	va_start(ap, fmt);
	cmd = diagStatusInfoData | (clientType << DIAG_DRV_TYPE_SHIFT) | (lineId << DIAG_DRV_LINE_SHIFT);
	statStr = (char *)&statStrBuf[4];
	n = vsprintf (statStr, fmt, ap) + 1;
	n += 4;	// argNum lenghth
	statStrBuf[0] = DIAG_ENDIAN_CONV_INT32(kDiagReceivedEocCommand | (lineId << DIAGS_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT));
	statStrBuf[1] = DIAG_ENDIAN_CONV_INT32(kDiagStrPrintf);
	statStrBuf[2] = DIAG_ENDIAN_CONV_INT32(n | kDiagClearEocMsgDataVolatile);
	statStrBuf[3] = 0;	// argNum
	n += 12;	// code/msgId/msgType length
	BcmDiagWriteStatusInfo(cmd, (char *)statStrBuf, n, NULL, 0);
	va_end(ap);
}

void BcmDiagReportEocData(uint lineId, uint clientType, int eocId, void *ptr, uint len, uint  flags)
{
	int	stat[3];
	uint	cmd = diagStatusInfoData | (clientType << DIAG_DRV_TYPE_SHIFT) | (lineId << DIAG_DRV_LINE_SHIFT);

	stat[0] = DIAG_ENDIAN_CONV_INT32(kDiagReceivedEocCommand | (lineId << DIAGS_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT));
	stat[1] = DIAG_ENDIAN_CONV_INT32(eocId);
	stat[2] = DIAG_ENDIAN_CONV_INT32(flags | len | kDiagClearEocMsgDataVolatile);
	BcmDiagWriteStatusInfo(cmd, (char *)&stat[0], sizeof(stat), ptr, len);
}

void BcmDiagOpenFile(uint lineId, uint clientType, char *fname)
{
	BcmDiagReportEocData(lineId, clientType, kDiagGeneralMsgDbgFileName, fname, strlen(fname) + 1, kDiagDbgFileNameDelete);
}

void BcmDiagWriteFile(uint lineId, uint clientType, char *fname, void *ptr, uint len)
{
	BcmDiagReportEocData(lineId, clientType, kDiagGeneralMsgDbgFileName, fname, strlen(fname) + 1, 0);
	BcmDiagReportEocData(lineId, clientType, kDiagGeneralMsgDbgWriteFile, ptr, len, 0);
}

void BcmDiagDumpData(uint lineId, uint clientType, void *ptr, uint len, uint  flags)
{
	BcmDiagReportEocData(lineId, clientType, kDiagGeneralMsgDbgDataPrint, ptr, len, flags);
}

void BcmDiagEventData(uint lineId, uint clientType, void *ptr, uint len, uint  flags)
{
	BcmDiagReportEocData(lineId, clientType, kDiagSerdesEventData, ptr, len, flags);
}

void BcmDiagImageTestAck (unsigned int clientType, diagImageTestData *pImageData, uint32_t frNum, uint32_t frRcv)
{
  int   stat[3];
  uint  cmd = diagStatusInfoData | (clientType << DIAG_DRV_TYPE_SHIFT);
  stat[0] = DIAG_ENDIAN_CONV_INT32(kDiagDataAvailStatus | (clientType << DIAG_TYPE_SHIFT));
  stat[1] = DIAG_ENDIAN_CONV_INT32(frNum | (frRcv << 8) | DIAG_ACK_FRAME_RCV_PRESENT);
  stat[2] = DIAG_ENDIAN_CONV_INT32(DIAG_ACK_LEN_INDICATION);
  BcmDiagWriteData(cmd, (char *)&stat[0], sizeof(stat), NULL, 0);
}

EXPORT_SYMBOL(BcmDiagRegisterClient);
EXPORT_SYMBOL(BcmDiagDeRegisterClient);
EXPORT_SYMBOL(BcmDiagWriteString);
EXPORT_SYMBOL(BcmDiagWriteData);
EXPORT_SYMBOL(BcmDiagWriteStatusInfo);
EXPORT_SYMBOL(BcmDiagGdbWriteData);
EXPORT_SYMBOL(BcmDiagPendingSkbAdd);
EXPORT_SYMBOL(BcmDiagGetSrvIpAddr);
EXPORT_SYMBOL(BcmDiagDevSkbAllocate);
EXPORT_SYMBOL(BcmDiagDevSkbFree);
EXPORT_SYMBOL(BcmDiagGetSkb);
EXPORT_SYMBOL(BcmDiagGetClientSkb);
EXPORT_SYMBOL(BcmDiagReportEocData);
EXPORT_SYMBOL(BcmDiagOpenFile);
EXPORT_SYMBOL(BcmDiagWriteFile);
EXPORT_SYMBOL(BcmDiagDumpData);
EXPORT_SYMBOL(BcmDiagEventData);
EXPORT_SYMBOL(BcmDiagImageTestAck);

