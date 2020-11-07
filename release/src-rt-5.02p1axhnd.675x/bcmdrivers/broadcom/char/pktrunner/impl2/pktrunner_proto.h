#ifndef __PKT_RUNNER_PROTO_H_INCLUDED__
#define __PKT_RUNNER_PROTO_H_INCLUDED__

/*
  <:copyright-BRCM:2013:proprietary:standard

  Copyright (c) 2013 Broadcom 
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

#if !defined(RDP_SIM)
#define CC_PKTRUNNER_PROCFS
#include <linux/seq_file.h>
#endif
#if defined(CONFIG_BLOG_IPV6)
#define CC_PKTRUNNER_IPV6
#endif


#if defined(CONFIG_BCM_RDPA_MCAST) || defined(RDP_SIM) || defined(CONFIG_BCM963158)
#define CC_PKTRUNNER_MCAST
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
#define CC_PKTRUNNER_BRCM_TAG
#endif

#if defined(RDP_SIM)
#define PKTRUNNER_BRCM_TAG_MODE  CMDLIST_BRCM_TAG_NONE
#else
#define PKTRUNNER_BRCM_TAG_MODE  CMDLIST_BRCM_TAG_RX_TX
#endif

/*******************************************************************************
 *
 * Debugging
 *
 *******************************************************************************/

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_PKTRUNNER, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)

#if !defined(__debug)
#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            bcm_print(fmt, ##arg); )
#endif

#define __dumpCmdList(_cmdList)                                         \
    BCM_LOGCODE(                                                        \
        if(isLogDebug)                                                  \
            cmdlist_dump((_cmdList), RDPA_CMD_LIST_UCAST_LIST_SIZE_32); )

#define __dumpPartialCmdList()                  \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            cmdlist_dump_partial();             \
            bcm_print("\n");                    \
        } )

#define __dumpBlog(_blog_p)                     \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            blog_dump((_blog_p));               \
            bcm_print("\n");                    \
        } )

#define __dumpBlogRule(_blogRule_p)             \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            blog_rule_dump((_blogRule_p));      \
            bcm_print("\n");                    \
        } )

/*******************************************************************************
 *
 * Functions
 *
 *******************************************************************************/

#ifndef BRCM_TAG_TYPE2_LEN
#define BRCM_TAG_TYPE2_LEN  4
#endif

#define PKTRUNNER_ACCEL_FLOW    (0)
#ifdef XRDP
#define PKTRNR_MAX_FHW_ACCEL    (2)  /* Max number of HW accelerators towards FHW */
#define PKTRUNNER_ACCEL_MCAST_WHITELIST (1)
#else
#define PKTRNR_MAX_FHW_ACCEL    (1)  /* Max number of HW accelerators towards FHW */
#endif

#ifndef CONFIG_BCM_RUNNER_MAX_FLOWS
#define PKTRUNNER_MAX_L2L3_FLOWS    (16384)
#else
#define PKTRUNNER_MAX_L2L3_FLOWS    (CONFIG_BCM_RUNNER_MAX_FLOWS) 
#endif

#ifdef XRDP
#define PKTRUNNER_MAX_MCAST_FLOWS   1024
#else
#define PKTRUNNER_MAX_MCAST_FLOWS   128
#endif

typedef enum {
    PKTRNR_FLOW_TYPE_INV    = 0,
    PKTRNR_FLOW_TYPE_L3     = 1,
    PKTRNR_FLOW_TYPE_L2     = 2,
    PKTRNR_FLOW_TYPE_MC     = 3,
}e_PKTRNR_FLOW_TYPE;


rdpa_mcast_flow_t *__mcastFlowMalloc(void);
void __mcastFlowFree(rdpa_mcast_flow_t *mcastFlow_p);
uint32_t __enetLogicalPortToPhysicalPort(uint32_t logicalPort);
uint32_t __skbMarkToQueuePriority(uint32_t skbMark);
uint32_t __skbMarkToTrafficClass(uint32_t skbMark);
int __isEnetWanPort(uint32_t logicalPort);
int __lagPortGet(Blog_t *blog_p);
int __isEnetBondedLanWanPort(uint32_t logicalPort);
int __isWlanPhy(Blog_t *blog_p);
int __isTxWlanPhy(Blog_t *blog_p);
int __isTxEthPhy(Blog_t *blog_p);
int __ucastSetFwdAndFilters(Blog_t *blog_p, rdpa_ip_flow_info_t *ip_flow_p);
int __l2ucastSetFwdAndFilters(Blog_t *blog_p, rdpa_l2_flow_info_t *l2_flow_p);

int __init runnerProto_construct(void);
void __exit runnerProto_destruct(void);

#if defined(CC_PKTRUNNER_PROCFS)
uint32_t pktRunnerGetState(struct seq_file *, uint32_t accel);
#endif

#endif  /* defined(__PKT_RUNNER_PROTO_H_INCLUDED__) */
