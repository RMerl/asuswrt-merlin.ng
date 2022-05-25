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
 * File Name  : bcmxtmrtbond.c
 *
 * Description: This file implements BCM6368 ATM/PTM bonding network device driver
 *              runtime processing - sending and receiving data.
 *              Current implementation pertains to PTM bonding. Broadcom ITU G.998.2 
 *              solution.
 ***************************************************************************/


/* Includes. */
//#define DUMP_DATA
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
#include <linux/nbuff.h>
#include <bcm_map_part.h>
#include "bcmxtmrtimpl.h"

#if (defined(CONFIG_BCM963178)) && defined(CONFIG_BCM_ARCHER)
#include "xtmrt_archer.h"
#else
#include "xtmrt_runner.h"
#endif


#ifdef PERF_MON_BONDING_US
#include <asm/pmonapi.h>
#endif

#ifdef PERF_MON_BONDING_US
#define PMON_US_LOG(x)        pmon_log(x)
#define PMON_US_CLR(x)        pmon_clr(x)
#define PMON_US_BGN           pmon_bgn()
#define PMON_US_END(x)        pmon_end(x)
#define PMON_US_REG(x,y)      pmon_reg(x,y)
#else
#define PMON_US_LOG(x)        
#define PMON_US_CLR(x)        
#define PMON_US_BGN
#define PMON_US_END(x)        
#define PMON_US_REG(x,y)
#endif

/* Function Prototypes */
static int constructPtmBondHdr(XtmRtPtmTxBondHeader *ptmBondHdr_p, UINT32 ulPtmPrioIdx, UINT32 len) ;
static inline int getPtmFragmentLen(UINT32 len, int* pilCredit) ;

#if defined(HOST_PATH_ADD_BOND_HDR)
static int constructPtmBondHdrTxPAF(XtmRtPtmTxBondHeader *ptmBondHdr_p, UINT32 ulPtmPrioIdx, UINT32 len) ;
static inline int getPtmFragmentLenTxPAF(UINT32 len) ;
#endif

/***************************************************************************
 * Function Name: getPtmFragmentLen
 * Description  : Compute next fragment length
 * Returns      : fragment length.
 ***************************************************************************/
static inline int getPtmFragmentLen(UINT32 origLen, int* pilCredit)
{
	int fragmentLen ;
	int leftOver ;
   int credit = *((int *) pilCredit), availCredit ;

   if (credit >= XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE) {

      if (credit > XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE) {
         availCredit = XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE ;
         credit -= availCredit ;
      }
      else {
         availCredit = credit ;
         credit = 0 ;
      }
	}
	else {
		availCredit = XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE ;
		credit -= availCredit ;
	}

	if (origLen <= availCredit) {
		fragmentLen = origLen;
	}
	else
	{
		/* send as much as possible unless we don't have
			enough data left anymore for a full fragment */
		fragmentLen = availCredit ;
		leftOver    = origLen - fragmentLen;
		if (leftOver < XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE)
		{
			fragmentLen -= (XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE + leftOver) ;
			if (fragmentLen < XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE) 
				fragmentLen = origLen ;
		}
	}

	availCredit -= fragmentLen ;
	credit += availCredit ;

   if (fragmentLen != origLen) { /* Not the last fragment of the packet */
      credit += (fragmentLen & 0x3) ;
      fragmentLen &= ~0x3; /* make it a multiple of 4 bytes */
   }

   if ((fragmentLen < XTMRT_PTM_BOND_PROTO_TX_MIN_FRAGMENT_SIZE) ||
       (fragmentLen > XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE)) {
      BCM_XTM_ERROR ("ALERT------------------------------fragmentLen %d is out of bound \n", fragmentLen) ;
   }

   *pilCredit = credit ;

   return fragmentLen;

}  /* getPtmFragmentLen() */

/***************************************************************************
 * Function Name: constructPtmBondHdr
 * Description  : Calculates the PTM bonding hdr information and fills it up
 *                in the global buffer to be used for the packet.
 * Returns      : NoofFragments.
 ***************************************************************************/
static int constructPtmBondHdr(XtmRtPtmTxBondHeader *pPtmBondHdr, UINT32 ptmPrioIdx, UINT32 len)
{
   int reloadCount, serviced ;
   int fragNo, fragLen, fragLenWithOvh ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   volatile XtmRtPtmBondInfo     *pBondInfo = &pGi->ptmBondInfo ;
   XtmRtPtmTxBondHeader  *pPtmHeader = pPtmBondHdr ;
   volatile UINT16       *pPortDataMask = (UINT16 *) &pBondInfo->portMask ;
   UINT32       *pulLinkWts    = (UINT32 *) &pBondInfo->ulLinkUsWt[0] ;
   int        *pilLinkCredit = (int *) &pBondInfo->ilLinkUsCredit[0] ;
   UINT16       *pu16ConfWtPortDist = (UINT16 *) &pBondInfo->u16ConfWtPortDist[0] ;
   UINT16       *pulCurrWtPortDistRunIndex   = (UINT16 *) &pBondInfo->ulCurrWtPortDistRunIndex ;
   UINT8        portCount ;

   fragNo = 0 ;
   len += ETH_FCS_LEN ;    /* Original Packet Len + 4 bytes of Eth FCS Len */
   
   while (len != 0) {

#ifndef PTMBOND_US_PRIO_TRAFFIC_SPLIT

      serviced = reloadCount = 0 ;

      do {

         portCount = pu16ConfWtPortDist [*pulCurrWtPortDistRunIndex] ;

         if (((*pPortDataMask >> portCount) & 0x1) != 0) {

            fragLen = getPtmFragmentLen (len, (int*)&pilLinkCredit[portCount]) ;

            fragLenWithOvh = fragLen + XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE ;

            if (pulLinkWts[portCount] >= fragLenWithOvh) { /* port is enabled with valid weights */

               len -= fragLen ;
               pPtmHeader = &pPtmBondHdr [fragNo] ;
               pPtmHeader->sVal.FragSize = fragLenWithOvh ; /* With actual hdrs/trailers */
#ifdef BONDING_DEBUG
               if (pBondInfo->printEnable == 1) {
                  if (portCount == 1) {
                     BCM_XTM_NOTICE("ALARM=================== portCount=1, Mask=%x", (unsigned int) *pPortDataMask) ;
                  }
               }
#endif
               pPtmHeader->sVal.portSel = portCount ;
               pulLinkWts[portCount] -= fragLenWithOvh ;
               pilLinkCredit[portCount] -= XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE ;
               serviced = 1 ;
            }
            else {
               //fap4kePrt_Print ("P %d, W %lu W1 %lu \n", portCount, pulLinkWts[portCount],
                                 //pulLinkWts [portCount^1]) ;
               pilLinkCredit[portCount] += (fragLenWithOvh+pulLinkWts[portCount]) ;
					pulLinkWts[portCount] = 0 ;
            }

         if (serviced == 1) {
            if (pilLinkCredit [portCount] > 0)
               break ;
            else
               pilLinkCredit[portCount] += pBondInfo->ilConfLinkUsCredit[portCount] ;
         }
			} /* if (*pPortDataMask) */

			*pulCurrWtPortDistRunIndex = ((*pulCurrWtPortDistRunIndex+1) >= pBondInfo->totalWtPortDist) ? 0 :
				*pulCurrWtPortDistRunIndex+1 ;

         if (serviced == 1)
            break ;
         else { /* No port selection happened  for this fragment, reload the weights with unused credits */

				if (*pPortDataMask == 0) {
					return 0 ;
            }
				else if ((pulLinkWts[0] == 0) && (pulLinkWts[1] == 0)) {
               pulLinkWts[0] = pBondInfo->ulConfLinkUsWt[0] ;
               pulLinkWts[1] = pBondInfo->ulConfLinkUsWt[1] ;

               pilLinkCredit[0] = pBondInfo->ilConfLinkUsCredit [0] ;
               pilLinkCredit[1] = pBondInfo->ilConfLinkUsCredit [1] ;
               reloadCount++ ;
               if (reloadCount >= 2) {
                  return 0 ;
					}
            }
         }
      } while (1) ;
#else

      fragLen = getPtmFragmentLen (len, NULL) ;
      len -= fragLen ;
      pPtmHeader = &pPtmBondHdr [fragNo] ;

      fragLen += XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE ;  /* With actual hdrs/trailers */
      pPtmHeader->sVal.FragSize = fragLen ;

      pPtmHeader->sVal.portSel = (ulPtmPrioIdx == PTM_FLOW_PRI_LOW) ? PHY_PORTID_0 : PHY_PORTID_1 ;

#endif /* #ifndef PTMBOND_US_PRIO_TRAFFIC_SPLIT */

      pPtmHeader->sVal.PktEop = XTMRT_PTM_BOND_HDR_NON_EOP ;
      fragNo++ ;

   } /* while (len != 0) */

   pPtmHeader->sVal.PktEop = XTMRT_PTM_BOND_FRAG_HDR_EOP ;

   return (fragNo) ;
   
}  /* constructPtmBondHdr() */


#if defined(HOST_PATH_ADD_BOND_HDR)

/***************************************************************************
 * Function Name: getPtmFragmentLenTxPAF
 * Description  : Compute next fragment length
 * Returns      : fragment length.
 ***************************************************************************/
static inline int getPtmFragmentLenTxPAF(UINT32 origLen)
{
   int fragmentLen ;
   int leftOver ;

   if (origLen <= XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE) {
      fragmentLen = origLen ;
   }
   else {
      /* send as much as possible unless we don't have
         enough data left anymore for a full fragment */
      fragmentLen = XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE ;
      leftOver = origLen-fragmentLen ;
      if (leftOver < XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE) {
         fragmentLen -= (XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE + leftOver) ;
         fragmentLen &= ~0x3 ; /* make it a multiple of 4 bytes */
      }
   }

   if ((fragmentLen < XTMRT_PTM_BOND_PROTO_TX_MIN_FRAGMENT_SIZE) ||
       (fragmentLen > XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE)) {
      BCM_XTM_ERROR("ALERT------------------------------fragmentLen %d is out of bound \n", fragmentLen) ;
   }

   return fragmentLen ;
}  /* getPtmFragmentLenTxPAF() */

/***************************************************************************
 * Function Name: constructPtmBondHdrTxPAF
 * Description  : Calculates the PTM bonding hdr information and fills it up
 *                in the global buffer to be used for the packet.
 * Returns      : NoofFragments.
 ***************************************************************************/
static int constructPtmBondHdrTxPAF(XtmRtPtmTxBondHeader *pPtmBondHdr, UINT32 ptmPrioIdx, UINT32 len)
{
   int fragNo, fragLen, fragLenWithOvh ;
   XtmRtPtmTxBondHeader  *pPtmHeader = pPtmBondHdr ;

   fragNo = 0 ;
   len += ETH_FCS_LEN ;    /* Original Packet Len + 4 bytes of Eth FCS Len */
   
   while (len != 0) {

      fragLen = getPtmFragmentLenTxPAF (len) ;
      fragLenWithOvh = fragLen + XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE ;
      len -= fragLen ;

      pPtmHeader = &pPtmBondHdr [fragNo] ;
      pPtmHeader->sVal.FragSize = fragLenWithOvh ; /* With actual hdrs/trailers */

#ifndef PTMBOND_US_PRIO_TRAFFIC_SPLIT
      pPtmHeader->sVal.portSel = PHY_PORTID_0 ;   /* Over US0, for single latency. */
#else
      pPtmHeader->sVal.portSel = (ulPtmPrioIdx == PTM_FLOW_PRI_LOW) ? PHY_PORTID_0 : PHY_PORTID_1 ;
#endif
      pPtmHeader->sVal.PktEop = XTMRT_PTM_BOND_HDR_NON_EOP ;
      fragNo++ ;

   } /* while (len != 0) */

   pPtmHeader->sVal.PktEop = XTMRT_PTM_BOND_FRAG_HDR_EOP ;

   return (fragNo) ;
   
}  /* constructPtmBondHdrTxPAF() */
#endif

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_calculate_link_parameters
 * Description  : Calculates the ptm bonding link parameters (weights), based on the
 *                link availability. For host execution, also forwards the
 *                the upstream rate of each link and the portMask to 4KE
 *                with a DQM message.
 * Returns      : 0
 * Note: Apply the FAP_4KE compiler flag to allow this function source shared
 * by Host and 4KE.
 ***************************************************************************/
int bcmxtmrt_ptmbond_calculate_link_parameters(UINT32 *pulLinkUSRates, UINT32 portMask, UINT32 updateOnly)
{
   int i, max, min, nRet = 0, mod, quot ;
   XtmRtPtmBondInfo      *pBondInfo ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;

   pBondInfo = &pGi->ptmBondInfo ;

   for (i=0; i< MAX_BOND_PORTS; i++) {
      if ((pulLinkUSRates[i] != 0) && (((portMask >> i) & 0x1) != 0))
         pBondInfo->ulConfLinkUsWt [i] = pulLinkUSRates[i]/8 ;  /* bytes/sec */
      else
         pBondInfo->ulConfLinkUsWt [i] = 0 ;
   }

   memcpy (&pBondInfo->ulLinkUsWt[0], &pBondInfo->ulConfLinkUsWt[0], (MAX_BOND_PORTS*sizeof (UINT32))) ;
   pBondInfo->portMask = portMask ;
#ifdef BONDING_DEBUG
   if (portMask == 0x3)
      g_GlobalInfo.ptmBondInfo.printEnable = 0 ;
   else
      g_GlobalInfo.ptmBondInfo.printEnable = 0 ;
#endif
   /* Calculate the wt port distribution */

   if (pBondInfo->ulConfLinkUsWt[0] >= pBondInfo->ulConfLinkUsWt[1]) {
      max = 0; min = 1 ;
   }
   else {
      max = 1; min = 0 ;
   }
            
	if (!updateOnly)
   BCM_XTM_NOTICE("[HOST] link0=%u link1=%u mask=0x%x \n",
                   (unsigned int)pulLinkUSRates[0], (unsigned int)pulLinkUSRates[1], (unsigned int)portMask) ;

   if (pBondInfo->ulConfLinkUsWt [max] != 0) {

      if (pBondInfo->ulConfLinkUsWt [min] != 0) {
         pBondInfo->ilConfLinkUsCredit [min] = XTMRT_PTM_BOND_TX_DEF_CREDIT ;
			quot = ((pBondInfo->ulConfLinkUsWt [max] * pBondInfo->ilConfLinkUsCredit[min])/
			                          pBondInfo->ulConfLinkUsWt [min]) ;
			mod = ((pBondInfo->ulConfLinkUsWt [max] * pBondInfo->ilConfLinkUsCredit[min])%
			                          pBondInfo->ulConfLinkUsWt [min]) ;
			mod = ((mod*10)/pBondInfo->ulConfLinkUsWt[min]) ;
			quot = (mod>0x5) ? quot+1 : quot ;
         pBondInfo->ilConfLinkUsCredit [max] = quot ;
         pBondInfo->totalWtPortDist = 0x2 ;
         pBondInfo->u16ConfWtPortDist[min]    = min ;
         pBondInfo->u16ConfWtPortDist[max]    = max ;
      }
      else {
         pBondInfo->totalWtPortDist = 0x1 ;
         pBondInfo->u16ConfWtPortDist[0]    = max ;
         pBondInfo->ilConfLinkUsCredit[max] = XTMRT_PTM_BOND_TX_DEF_CREDIT ;
         pBondInfo->ilConfLinkUsCredit[min] = 0 ;
      }
   }
   else {
      if (pBondInfo->ulConfLinkUsWt [min] != 0) {
         pBondInfo->totalWtPortDist = 0x1 ;
         pBondInfo->u16ConfWtPortDist[0]    = min ;
         pBondInfo->ilConfLinkUsCredit[min] = XTMRT_PTM_BOND_TX_DEF_CREDIT ;
         pBondInfo->ilConfLinkUsCredit[max] = 0 ;
      }
      else {
         pBondInfo->totalWtPortDist = 0x0 ;
         pBondInfo->u16ConfWtPortDist[0]    = 0xFF ;
         pBondInfo->ilConfLinkUsCredit[0] = 0 ;
         pBondInfo->ilConfLinkUsCredit[1] = 0 ;
      }
   }

   memcpy (&pBondInfo->ilLinkUsCredit[0], &pBondInfo->ilConfLinkUsCredit[0], (MAX_BOND_PORTS*sizeof (INT32))) ;
   pBondInfo->ulCurrWtPortDistRunIndex   = 0 ;

   pBondInfo->bonding   = 1; /* set bonding to true */

	if (!updateOnly)
   BCM_XTM_NOTICE("[HOST] link wt0=%u link wt1=%u mask=0x%x \n",
                   (unsigned int)pBondInfo->ulConfLinkUsWt[0], (unsigned int)pBondInfo->ulConfLinkUsWt[1], 
                   (unsigned int)pBondInfo->portMask) ;


   return (nRet) ;
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_add_hdr (4KE/CPU Paths)
 * Description  : Adds the PTM Bonding Tx header to a packet before transmitting
 *                it. The header is composed of 8 16-bit words of fragment info.
 *                Each fragment info word contains the length of the fragment and
 *                its tx port (link).
 *                The number of fragments is calculated from the packet length.
 *                The end-of-packet bit is set in the last fragment info word.
 *                The tx port for each fragment is selected based on the tx
 *                bandwidth of each link so that both links are load balanced.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
 * Returns      : 0: fail, others: success.
 ***************************************************************************/

int bcmxtmrt_ptmbond_get_hdr(UINT8 *pBondHdr, UINT32 ulTxPafEnabled, UINT32 ulPtmPrioIdx, int len) {
    int frags = 0;
    PBCMXTMRT_GLOBAL_INFO pGi;
    XtmRtPtmTxBondHeader *pPtmSrcBondHdr;

    /* Take lock ? No need as such */

    pGi = &g_GlobalInfo;
    pPtmSrcBondHdr = &pGi->ptmBondHdr[0];

    if (!ulTxPafEnabled)
        frags = constructPtmBondHdr(pPtmSrcBondHdr, ulPtmPrioIdx, len);
    else {
#if defined(HOST_PATH_ADD_BOND_HDR)
        frags = constructPtmBondHdrTxPAF(pPtmSrcBondHdr, ulPtmPrioIdx, len);
#else
        frags = 1;
#endif
    }
    memcpy(pBondHdr, pPtmSrcBondHdr, sizeof(XtmRtPtmTxBondHeader) * frags);

    /* Release lock - if taken above */
    return frags;
}

#define CACHE_FLUSH_PTM_BONDING_HEADER_REGION(data,len) cache_flush_len((data),len)

int bcmxtmrt_ptmbond_add_hdr(UINT32 ulTxPafEnabled, pNBuff_t *ppNBuff, UINT32 ulPtmPrioIdx)
{
   int   frags = 0;
#ifdef BONDING_DEBUG
   int i ;
   volatile unsigned int *pDma = (unsigned int *) 0xb000c444 ;
#endif
   int len = 0;
   UINT8 *pData;
   int   headroom ;
   unsigned long   minheadroom, bondhdr_size ;
   PBCMXTMRT_GLOBAL_INFO pGi ;
   XtmRtPtmBondInfo      *pBondInfo;
   XtmRtPtmTxBondHeader  *pPtmSrcBondHdr;
             
   PMON_US_BGN;

   bondhdr_size    = sizeof(XtmRtPtmTxBondHeader) * XTMRT_PTM_BOND_MAX_FRAG_PER_PKT;
   minheadroom     = bondhdr_size ;
   pGi = &g_GlobalInfo ;
   pBondInfo      = &pGi->ptmBondInfo;
   pPtmSrcBondHdr = &pGi->ptmBondHdr[0];

   PMON_US_LOG(1);

   if (IS_SKBUFF_PTR(*ppNBuff))
   {
      struct sk_buff *skb = PNBUFF_2_SKBUFF(*ppNBuff);
      headroom = skb_headroom (skb) ;

      /*Align required headroom up to the nearest cache line to ensure that we
       *don't have to share this cache line with other SW.*/
      minheadroom += (unsigned long)(skb->data - minheadroom) & (L1_CACHE_BYTES-1);

      BCM_XTM_DEBUG("skb->data=%lx, skb HR = %d \n", (uintptr_t) skb->data, headroom) ;      
      if (headroom < minheadroom) {

         struct sk_buff *skb2 ;

         BCM_XTM_INFO("bcmxtmrt: Warning!!, HR (%d) is < min HR (%d) \n",
                (int) headroom, (int) minheadroom) ;
      
         if (atomic_read(&skb_shinfo(skb)->dataref) == 1) { /* Realloc_headroom requires refcount to be 1, otherwise, do copy/expand */
             skb2 = skb_realloc_headroom (skb, minheadroom) ;
         }
         else {
             skb2 = skb_copy_expand (skb, minheadroom, 0, GFP_ATOMIC) ;
         }

         dev_kfree_skb_any(skb);
         if (skb2)
         {
            skb = skb2;
         }
         else
            skb = NULL ;
      }
      
      if (skb && skb_headroom(skb) >= minheadroom)
      {
//         BCM_XTM_DEBUG("skb->len = %d, Data=%lx\n", skb->len, (uintptr_t) skb->data);
         len = skb->len ;
         if (!ulTxPafEnabled)
            frags = constructPtmBondHdr(pPtmSrcBondHdr, ulPtmPrioIdx, len) ;
         else {
#if defined(HOST_PATH_ADD_BOND_HDR)
            frags = constructPtmBondHdrTxPAF(pPtmSrcBondHdr, ulPtmPrioIdx, len) ;
#else
            /*Cache flush the headroom required for the PTM bonding header now. 
             *This cache line might be dirty. We don't want it to get evicted at some 
             *undetermined later time, after RDP FW has written the PTM bonding header into it.
             */
            CACHE_FLUSH_PTM_BONDING_HEADER_REGION (skb->data - minheadroom, L1_CACHE_BYTES) ;
            frags = 1 ;
            goto _skipFragmentationSkb ;
#endif
         }

			if (frags)
         {
				PMON_US_LOG(2);
				pData = skb_push(skb, bondhdr_size);
				PMON_US_LOG(3);
            if (!((uintptr_t)(pData) & 0x1))  /* 16-bit aligned packet boundary */
				    u16datacpy(pData, pPtmSrcBondHdr, sizeof(XtmRtPtmTxBondHeader) * frags);
            else
				    memcpy(pData, pPtmSrcBondHdr, sizeof(XtmRtPtmTxBondHeader) * frags);
            //DUMP_PKT(pData, 16);
				PMON_US_LOG(4);
			}
      }
      else
      {
         //if (skb) {
            //BCM_XTM_DEBUG("Failed to allocate SKB with enough headroom. RC-%d, len-%d \n", atomic_read(&skb_shinfo(skb)->dataref),
                  //skb->len);
            //DUMP_PKT(skb->data, skb->len);
         //}
         //else
            BCM_XTM_ERROR("Failed to allocate SKB with enough headroom \n");
      }

#if !defined(HOST_PATH_ADD_BOND_HDR)
_skipFragmentationSkb :
#endif
      *ppNBuff = SKBUFF_2_PNBUFF(skb);
   } /* if (SKB) */
   else
   {
      struct fkbuff *fkb = PNBUFF_2_FKBUFF(*ppNBuff);
      headroom = fkb_headroom (fkb) ;

      /*Align required headroom up to the nearest cache line to ensure that we
       *don't have to share this cache line with other SW.*/
      minheadroom += (unsigned long)(fkb_data(fkb) - minheadroom) & (L1_CACHE_BYTES-1);

      BCM_XTM_DEBUG("fkb=%pk, fkb_headroom = %d \n", fkb, headroom) ;
      if (headroom >= minheadroom)
      {
         BCM_XTM_DEBUG("fkb->len = %d\n", fkb->len);
         len = fkb->len ;
         if (!ulTxPafEnabled)
            frags = constructPtmBondHdr(pPtmSrcBondHdr, ulPtmPrioIdx, len) ;
         else {
#if defined(HOST_PATH_ADD_BOND_HDR)
            frags = constructPtmBondHdrTxPAF(pPtmSrcBondHdr, ulPtmPrioIdx, len) ;
#else
            /*Cache flush the headroom required for the PTM bonding header now. 
             *This cache line might be dirty. We don't want it to get evicted at some 
             *undetermined later time, after RDP FW has written the PTM bonding header into it.
             */
            CACHE_FLUSH_PTM_BONDING_HEADER_REGION (fkb_data(fkb) - minheadroom, L1_CACHE_BYTES);
            frags = 1 ;
            goto _skipFragmentationFkb ;
#endif
         }

			if (frags)
         {
				PMON_US_LOG(2);
				pData = fkb_push(fkb, bondhdr_size);
				PMON_US_LOG(3);
            if (!((uintptr_t)(pData) & 0x1))  /* 16-bit aligned packet boundary */
				    u16datacpy(pData, pPtmSrcBondHdr, sizeof(XtmRtPtmTxBondHeader) * frags);
            else
				    memcpy(pData, pPtmSrcBondHdr, sizeof(XtmRtPtmTxBondHeader) * frags);                                    
            //DUMP_PKT(pData, 16);
				PMON_US_LOG(4);
			}
      }
      else
      {
         BCM_XTM_ERROR("FKB not enough headroom.\n");
      }
#if !defined(HOST_PATH_ADD_BOND_HDR)
_skipFragmentationFkb :
#endif
      goto _End ;
   } /* if (FKB) */

_End :
#ifdef FRAGMENT_ALIGNMENT_32BIT_DEBUG
      BCM_XTM_NOTICE("-------------------\nIn Packet Len without FCS =%d frags=%d\n-------------------\n", len, frags) ;
      DUMP_PKT((UINT8 *) pPtmSrcBondHdr, 16) ;
#endif

   PMON_US_LOG(5);
   PMON_US_END(5);

   return frags;

} /* bcmxtmrt_ptmbond_add_hdr() */

static int __ProcTxBondInfo(char *page)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT pDevCtx;
   volatile XtmRtPtmTxBondHeader *bondHdr_p  = &pGi->ptmBondHdr[0];
   volatile XtmRtPtmBondInfo     *bondInfo_p = &pGi->ptmBondInfo;
   UINT32 i;
   int sz = 0, port, eopStatus, fragSize;

   if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
	{
		sz += sprintf(page + sz, "\nPTM Tx Bonding Information \n");
		sz += sprintf(page + sz, "\n========================== \n");

		sz += sprintf(page + sz, "\nPTM Header Information"); 

		for (i = 0; i < XTMRT_PTM_BOND_MAX_FRAG_PER_PKT; i++)
		{
			port       = bondHdr_p[i].sVal.portSel;
			eopStatus  = bondHdr_p[i].sVal.PktEop;
			fragSize   = bondHdr_p[i].sVal.FragSize;
			fragSize   -= (XTMRT_PTM_BOND_FRAG_HDR_SIZE+
					XTMRT_PTM_CRC_SIZE);
			if (eopStatus == 1)
			{
				sz += sprintf(page + sz, "\nFragment[%u]: port<%d> eopStatus<%d> size<%d>\n",
						(unsigned int)i, port, eopStatus, fragSize-ETH_FCS_LEN);
				break;
			}
			else
			{
				sz += sprintf(page + sz, "\nFragment[%u]: port<%u> eopStatus<%u> size<%u>\n",
						(unsigned int)i, port, eopStatus, fragSize);
			}
		} /* for (i) */

		for (i = 0; i < MAX_DEV_CTXS; i++)
		{
			pDevCtx = pGi->pDevCtxs[i];
			if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
			{
				sz += sprintf(page + sz, "dev: %s, ActivePortMask<%u>\n",
						pDevCtx->pDev->name, (unsigned int)pDevCtx->ulPortDataMask);
			}
		}

		for (i = 0; i < MAX_BOND_PORTS; i++ )
		{
			sz += sprintf(page + sz, "Port[%u]: ConfWt<%u> CurrWt<%u> CurrCredit<%i>\n",
					(unsigned int)i, (unsigned int)bondInfo_p->ulConfLinkUsWt[i],
					(unsigned int)bondInfo_p->ulLinkUsWt[i],
					(signed int)bondInfo_p->ilLinkUsCredit[i]);
		}

		sz += sprintf(page + sz, "totalWtPortDist : %u CurrWtPortDistRunIndex : %u \n", 
				(unsigned int) bondInfo_p->totalWtPortDist, 
				(unsigned int) bondInfo_p->ulCurrWtPortDistRunIndex) ;
				

		sz += sprintf(page + sz, "All Wt Port Distributions ==>") ;

		for( i = 0; i < bondInfo_p->totalWtPortDist; i++ ) {
			sz += sprintf(page + sz, " %u,", bondInfo_p->u16ConfWtPortDist[i]) ;
		}

		sz += sprintf(page + sz, "\n");
	}
	else
	{
		sz += sprintf(page + sz, "No Bonding Information \n");
	}

    return( sz );
        
} /* __ProcTxBondInfo() */

/***************************************************************************
 * Function Name: ProcTxBondInfo
 * Description  : Displays information about Bonding Tx side counters.
 *                Currently for PTM bonding.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int ProcTxBondInfo(struct file *file, char __user *buf,
                        size_t len, loff_t *offset)
{
int sz=0;
    if(*offset == 0)
    {
        *offset=sz=__ProcTxBondInfo(buf);
    }
return sz;
}


