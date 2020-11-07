#ifndef __CMDLIST_H_INCLUDED__
#define __CMDLIST_H_INCLUDED__

/*
  <:copyright-BRCM:2017:proprietary:standard

  Copyright (c) 2017 Broadcom 
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
#include <linux/ip.h>
#endif
#include "cmdlist_api.h"

#if defined(CONFIG_BLOG_IPV6)
#define CC_CMDLIST_IPV6
#endif

#if defined(CONFIG_BCM_RDPA_MCAST) || defined(RDP_SIM) || defined(CONFIG_BCM963158)
#define CC_CMDLIST_MCAST
#endif

/*******************************************************************************
 *
 * Debugging
 *
 *******************************************************************************/

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_CMDLIST, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_CMDLIST, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_CMDLIST, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_CMDLIST, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_CMDLIST, fmt, ##arg)

#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            bcm_print(fmt, ##arg); )

#define __dumpPartialCmdList()                  \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            cmdlist_dump_partial();             \
            bcm_print("\n");                    \
        } )

#define CMDLIST_CMD_CHECK(_ret)                         \
    do {                                                \
        if((_ret) == CMDLIST_RET_ERR_OVERFLOW)          \
        {                                               \
            __logInfo("Overflow, Could not Add Command");        \
            return (_ret);                              \
        }                                               \
        else if((_ret) != CMDLIST_RET_OK)               \
        {                                               \
            __logError("Could not Add Command");        \
            return (_ret);                              \
        }                                               \
        __dumpPartialCmdList();                         \
    } while(0)

/*******************************************************************************
 *
 * Functions
 *
 *******************************************************************************/

#define CMDLIST_BUFFER_SIZE  64

#ifndef BRCM_TAG_TYPE2_LEN
#define BRCM_TAG_TYPE2_LEN  4
#endif

#define BRCM_TAG_TYPE2_OFFSET 12

#define CMDLIST_UCAST_WLAN_ETH_HEADER_SIZE 16

extern cmdlist_hooks_t cmdlist_hooks_g;

int cmdlist_ucast_create_bin(Blog_t *blog_p, cmdlist_cmd_target_t target,
                             uint8_t *prependData_p, int prependSize, void **buffer_pp,
                             cmdlist_brcm_tag_t brcm_tag);
int cmdlist_l2_ucast_create_bin(Blog_t *blog_p, cmdlist_cmd_target_t target,
                                uint8_t *prependData_p, int prependSize, void **buffer_pp,
                                cmdlist_brcm_tag_t brcm_tag);
int cmdlist_ucast_fc_context_add_bin(Blog_t *blog_p, cmdlist_cmd_target_t target);
void cmdlist_mcast_parse_bin(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p,
                             cmdlist_brcm_tag_t brcm_tag);
int cmdlist_l2_mcast_create_bin(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p);
int cmdlist_l3_mcast_create_bin(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p, uint8_t *isRouted_p);
int cmdlist_pre_mcast_create_bin(cmdlist_mcast_parse_t *parse_p);
int cmdlist_post_mcast_create_bin(cmdlist_mcast_parse_t *parse_p);

void *cmdlist_buffer_alloc(void **phys_addr_pp);
void cmdlist_buffer_free(void *buffer_p);
void cmdlist_buffer_flush(void *buffer_p);

int __isEnetWanPort(uint32_t logicalPort);
int __isTxWlanPhy(Blog_t *blog_p);
uint32_t __buildBrcmTagType2(Blog_t *blog_p);

#if defined(CONFIG_CMDLIST_GPE)
void gpe_config(int ih_offset, int ih_headroom, int headroom, int target_ddr_supported);
void gpe_init(uint32_t *cmd_list_p, uint32_t cmd_list_length_max, uint32_t cmd_list_start_offset);
uint32_t gpe_get_length(void);
void gpe_dump(uint32_t *cmd_list_p, int length32);
void gpe_dump_partial(void);
int gpe_cmd_end(void);
#define cmdlist_config_bin gpe_config
#define cmdlist_init_bin gpe_init
#define cmdlist_get_length_bin gpe_get_length
#define cmdlist_dump_bin gpe_dump
#define cmdlist_dump_partial_bin gpe_dump_partial
#define cmdlist_cmd_end_bin gpe_cmd_end
#else /* SPE */
void spe_init(uint32_t *cmd_list_p, uint32_t cmd_list_length_max, uint32_t cmd_list_start_offset);
uint32_t spe_get_length(void);
void spe_dump(uint32_t *cmd_list_p, int length32);
void spe_dump_partial(void);
int spe_cmd_end(void);
#define cmdlist_config_bin(_ih_offset, _ih_headroom, _headroom, _target_ddr_supported)
#define cmdlist_init_bin spe_init
#define cmdlist_get_length_bin spe_get_length
#define cmdlist_dump_bin spe_dump
#define cmdlist_dump_partial_bin spe_dump_partial
#define cmdlist_cmd_end_bin spe_cmd_end
#endif

static inline void __copy16_ntoh(uint16_t *to_p, uint16_t *from_p, int words)
{
    int i;

    for(i=0; i<words; ++i)
    {
        to_p[i] = ntohs(from_p[i]);
    }
}

static inline void __copy32_ntoh(uint32_t *to_p, uint32_t *from_p, int words)
{
    int i;

    for(i=0; i<words; ++i)
    {
        to_p[i] = ntohl(from_p[i]);
    }
}

#endif  /* defined(__CMDLIST_H_INCLUDED__) */
