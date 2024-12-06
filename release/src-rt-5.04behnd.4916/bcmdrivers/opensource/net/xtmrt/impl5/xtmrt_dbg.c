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
 * File Name  : xtmrt_dbg.c
 *
 * Description: This file implements XTMRT driver generic debug API or feature
 * specific debug API.
 ***************************************************************************/

/* Includes. */


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
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ip.h>
#include <linux/if_pppox.h>
#include <bcmtypes.h>
#include <shared_utils.h>
#include "bcmnet.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include <linux/kthread.h>
#include "bcmxtmrtimpl.h"
#include <linux/bcm_colors.h>

#if defined(XTM_DEBUG_PHY_VECTOR_PKTS)

#define DIAG_SKB_USERS        0x3FFFFFFF
#define SYMB_VECT_SEGMENT		(1019)
#define ETH_HEADER_ITU_LENGTH	(22)
#define ETH_MAC_ADDR_LEN		(6)

#define LINEID_OFFSET			(0)
#define SYNCCOUNTER_OFFSET	(2)
#define PARTID_OFFSET			(4)
uint32_t segmentidx_offset  = ETH_HEADER_ITU_LENGTH + PARTID_OFFSET;
uint32_t synccounter_offset = ETH_HEADER_ITU_LENGTH + SYNCCOUNTER_OFFSET;
typedef struct {
   union {
      struct {
         uint32_t synccount   :16;
         uint32_t segmentid  :16;
      };
      uint32_t frame_info;
   };
   uint32_t timestamp;
}vect_err_dbg_info_t;
#define MAX_VECT_INFO_DUMP 256
uint32_t curr_tx_info_idx   = 0;
uint32_t curr_cmpl_info_idx = 0;

vect_err_dbg_info_t tx_info[MAX_VECT_INFO_DUMP];
vect_err_dbg_info_t cmpl_info[MAX_VECT_INFO_DUMP];
static inline uint32_t xtm_gettimestamp(void)
{
    struct timeval tv;

    do_gettimeofday(&tv);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

static void skb_users_set(struct sk_buff *skb, unsigned int n)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   refcount_set(&skb->users,n);
#else
   atomic_set(&skb->users,n);
#endif
}

static unsigned int skb_users_read(struct sk_buff *skb)
{
   unsigned int count;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   count = refcount_read(&skb->users);
#else
   count = atomic_read(&skb->users);
#endif
   return count;
}

static inline void print_vector_info(vect_err_dbg_info_t *vect_info,uint32_t *idx, int completion)
{
   int i = 0;
   if(*idx == MAX_VECT_INFO_DUMP)
   {
      if(completion)
         printk("Completion:\n");
      printk("SynCnt:\t SegmId:\t tmpStm:\n");
      for(i = 0; i < MAX_VECT_INFO_DUMP; i++)
      {
         printk("%u\t %u\t %u\n",vect_info[i].synccount,
                                   vect_info[i].segmentid,
                                   vect_info[i].timestamp);
      }
      *idx = 0;
   }
}
void vector_frame_recycle_handler(pNBuff_t pNbuff, unsigned long context, uint32_t flags)
{
   //skb
   struct sk_buff *skb = PNBUFF_2_SKBUFF(pNbuff);
   //printk("Completion: len:%u segmentIdx:%u syncCounter:%u timestamp:%u\n",skb->len,
   //                                                            *((unsigned char *)skb->data+segmentidx_offset),
   //                                                            htons(*((uint16_t *)((unsigned char *)skb->data+synccounter_offset))),
   //                                                            xtm_gettimestamp());
   cmpl_info[curr_cmpl_info_idx].synccount   = htons(*((uint16_t *)((unsigned char *)skb->data+synccounter_offset)));
   cmpl_info[curr_cmpl_info_idx].segmentid  = *((unsigned char *)skb->data+segmentidx_offset);
   cmpl_info[curr_cmpl_info_idx++].timestamp = xtm_gettimestamp();
   print_vector_info(cmpl_info, &curr_cmpl_info_idx, 1);
   skb->recycle_flags &= ~SKB_RECYCLE_NOFREE;
   skb->recycle_hook = NULL;

   skb_users_set(skb,DIAG_SKB_USERS-1);
   return;
}

void dump_phy_error_vector_frame_info(struct sk_buff *skb)
{
   unsigned int count = 0;
   count = skb_users_read(skb);

   //printk("len:%u users:%u\n",skb->len,atomic_read(&skb->users));
   if ((count == (DIAG_SKB_USERS)))
   {
       //printk("len:%u segmentIdx:%u syncCounter:%u timestamp:%u\n",skb->len,
       //                                                            *((unsigned char *)skb->data+segmentidx_offset),
       //                                                            htons(*((uint16_t *)((unsigned char *)skb->data+synccounter_offset))),
       //                                                            xtm_gettimestamp());
       tx_info[curr_tx_info_idx].synccount   = htons(*((uint16_t *)((unsigned char *)skb->data+synccounter_offset)));
       tx_info[curr_tx_info_idx].segmentid  = *((unsigned char *)skb->data+segmentidx_offset);
       tx_info[curr_tx_info_idx++].timestamp = xtm_gettimestamp();
       print_vector_info(tx_info, &curr_tx_info_idx, 0);
       // Now register the recycle handler
       skb->recycle_hook = (RecycleFuncP)vector_frame_recycle_handler;
       // Set the recycle flags
       skb->recycle_flags |= SKB_RECYCLE_NOFREE;
       // Set the Users for recycle to be called
       skb_users_set(skb,1);
   }

}

#endif //XTM_DEBUG_PHY_VECTOR_PKTS
