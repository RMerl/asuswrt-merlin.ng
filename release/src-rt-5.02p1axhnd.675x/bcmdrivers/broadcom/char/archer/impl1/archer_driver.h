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

#ifndef __ARCHER_DRIVER_H_INCLUDED__
#define __ARCHER_DRIVER_H_INCLUDED__

#if !defined(CC_ARCHER_PERFORMANCE)
#define CC_ARCHER_SIM_MLT
#endif

/*******************************************************************************
 *
 * Debugging
 *
 *******************************************************************************/
#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_ARCHER, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_ARCHER, fmt, ##arg)

#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            bcm_print(fmt, ##arg); )

extern int archer_packet_length_max_g;
extern int archer_packet_headroom_g;
extern int archer_skb_tailroom_g;

long archer_driver_get_time_ns(void);

#if !defined(CC_ARCHER_DRIVER_BASIC)

#define __dump_cmdlist(_cmdList)                                        \
    BCM_LOGCODE(                                                        \
        if(isLogDebug)                                                  \
        {                                                               \
            bcm_print("\n********** Command List **********\n");        \
            cmdlist_dump((_cmdList), CMDLIST_CMD_LIST_SIZE_MAX_32);     \
        } )

#define __dump_partial_cmdlist()                \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            cmdlist_dump_partial();             \
            bcm_print("\n");                    \
        } )

#define __dump_blog(_blog_p)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            blog_dump((_blog_p));               \
            bcm_print("\n");                    \
        } )

#define __dump_blog_rule(_blogRule_p)           \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            blog_rule_dump((_blogRule_p));      \
            bcm_print("\n");                    \
        } )

#define ARCHER_RX_ENET_LAN(_blog_p)                                     \
    ( (_blog_p)->rx.info.phyHdrType == BLOG_ENETPHY && !archer_is_enet_wan_port((_blog_p)->rx.info.channel) )

#define ARCHER_TX_ENET_LAN(_blog_p)                                     \
    ( (_blog_p)->tx.info.phyHdrType == BLOG_ENETPHY && !archer_is_enet_wan_port((_blog_p)->tx.info.channel) )

#define ARCHER_RX_ENET_WAN(_blog_p)                                     \
    ( (_blog_p)->rx.info.phyHdrType == BLOG_ENETPHY && archer_is_enet_wan_port((_blog_p)->rx.info.channel) )

#define ARCHER_TX_ENET_WAN(_blog_p)                                     \
    ( (_blog_p)->tx.info.phyHdrType == BLOG_ENETPHY && archer_is_enet_wan_port((_blog_p)->tx.info.channel) )

static inline int archer_is_enet_wan_port(uint32_t logicalPort)
{
#if defined(CONFIG_BCM_ENET_SYSPORT)
    return 0;
#else
    bcmFun_t *enetIsWanPortFun = bcmFun_get(BCM_FUN_ID_ENET_IS_WAN_PORT);

    BCM_ASSERT(enetIsWanPortFun);

    return enetIsWanPortFun(&logicalPort);
#endif
}

int archer_ucast_ipv4_addresses_table_add(Blog_t *blog_p, uint32_t *table_sram_address_p);

int archer_ucast_common_flow_set(Blog_t *blog_p, sysport_classifier_flow_t *flow_p,
                                 sysport_rsb_flow_type_t flow_type,
                                 cmdlist_brcm_tag_t *brcm_tag_p,
                                 sysport_classifier_rsb_overwrite_t *rsb_overwrite_p);

int archer_ucast_activate(Blog_t *blog_p, sysport_flow_key_t *flow_key_p,
                          uint8_t *prependData_p, int prependSize);
int archer_ucast_deactivate(sysport_flow_key_t flow_key);

int archer_ucast_l2_activate(Blog_t *blog_p, sysport_flow_key_t *flow_key_p,
                             uint8_t *prependData_p, int prependSize);
int archer_ucast_l2_deactivate(sysport_flow_key_t flow_key);

int archer_mcast_activate(Blog_t *blog_p, sysport_flow_key_t *flow_key_p, int *is_activation_p);
int archer_mcast_deactivate(Blog_t *blog_p, int *is_deactivation_p);

#if (defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE)) && !defined(CC_ARCHER_SIM_MLT)
int archer_host_mac_address_match(uint8_t *packet_p);
#endif
int archer_host_mode_set(archer_mode_t mode);
void archer_host_info_dump(void);
int __init archer_host_construct(void);
void __exit archer_host_destruct(void);

void *archer_coherent_mem_alloc(int size, void **phys_addr_pp);
void archer_coherent_mem_free(int size, void *phys_addr_p, void *p);
void *archer_mem_kzalloc(int size);
void *archer_mem_kalloc(int size);
void archer_mem_kfree(void *p);
void *archer_mem_valloc(int size);
void archer_mem_vfree(void *p);

void archer_driver_nbuff_params(pNBuff_t pNBuff, uint8_t **data_p, uint32_t *length_p, int *tc_p);
void *archer_driver_skb_params(void *buf_p, uint8_t **data_p, uint32_t *length_p, void **fkbInSkb_p);

#if defined(CC_SYSPORT_DRIVER_TM)
void archer_driver_enet_tm_enable(int enable);
#endif

#if defined(CONFIG_BCM_ARCHER_GSO)
void archer_gso(pNBuff_t pNBuff, int nbuff_max, pNBuff_t *pNBuff_list, int *nbuff_count_p);
#else
static inline void archer_gso(pNBuff_t pNBuff, int nbuff_max, pNBuff_t *pNBuff_list, int *nbuff_count_p)
{
    pNBuff_list[0] = pNBuff;

    *nbuff_count_p = 1;
}
#endif

#if defined(CONFIG_BCM_ARCHER_SIM)
Blog_t *archer_sim_blog_get(Blog_t *blog_orig_p);
void archer_sim_blog_put(Blog_t *blog_orig_p, Blog_t *blog_p);
int archer_sim_init(void);
int archer_sim_enable(void);
int archer_sim_disable(void);
#if defined(CC_ARCHER_SIM_FC_HOOK)
void archer_sim_fc_hook_set(Blog_t *blog_p);
#endif
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
int __init archer_iq_register(void);
void __exit archer_iq_deregister(void);
#endif
uint8_t archer_iq_sort (sysport_rsb_t *rsb_p, uint8_t *packet_p);

#endif /* !CC_ARCHER_DRIVER_BASIC */

#endif  /* defined(__ARCHER_DRIVER_H_INCLUDED__) */
