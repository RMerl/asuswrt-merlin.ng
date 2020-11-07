/*
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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
/******************************************************************************
 Filename: cmdlist_driver.c
           This file implements the Command List Driver
******************************************************************************/

#if defined(RDP_SIM)
#include "pktrunner_rdpa_sim.h"
#else
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/nbuff.h>
#include "bcmenet.h"
#endif

#if defined(CONFIG_CMDLIST_GPE)
#include "rdpa_api.h"
#endif

#include "cmdlist.h"

cmdlist_hooks_t cmdlist_hooks_g;
uint32_t cmdlist_err_stats[CMDLIST_MAX_RET_CODES];

#define CMDLIST_RETURN(_ret)                            \
    do {                                                \
        int idx = -(_ret);                              \
        if(idx >= 0 && idx < CMDLIST_MAX_RET_CODES)     \
        {                                               \
            cmdlist_err_stats[idx]++;                   \
        }                                               \
        return (_ret);                                  \
    } while(0)

#define CMDLIST_RET_CODE_STATS(_ret)  cmdlist_err_stats[-(_ret)]

#define CMDLIST_STATS_PRINT(sf, bytes, fmt, arg...)     \
    do {                                                \
        if(sf) bytes += seq_printf(sf, fmt, ##arg);     \
        else bytes += bcm_print(fmt,##arg);             \
    } while(0);


/*******************************************************************************
 *
 * Helper Functions
 *
 *******************************************************************************/

#if !defined(RDP_SIM)

static struct kmem_cache *cmdlist_cache_p = NULL;

void *cmdlist_buffer_alloc(void **phys_addr_pp)
{
    void *buffer_p = kmem_cache_alloc(cmdlist_cache_p, GFP_ATOMIC);

    if(buffer_p)
    {
        *phys_addr_pp = (void *)VIRT_TO_PHYS(buffer_p);
    }

    __logDebug("buffer_p %p, phys_addr_p %p", buffer_p, *phys_addr_pp);

    return buffer_p;
}

void cmdlist_buffer_free(void *buffer_p)
{
    __logDebug("buffer_p %p", buffer_p);

    kmem_cache_free(cmdlist_cache_p, buffer_p);
}

static int __init cmdlist_buffer_construct(void)
{
    cmdlist_cache_p = kmem_cache_create("cmdlist_buffer",
                                        CMDLIST_BUFFER_SIZE,
                                        0, /* align */
                                        SLAB_HWCACHE_ALIGN, /* flags */
                                        NULL); /* ctor */
    if(cmdlist_cache_p == NULL)
    {
        __logError("Unable to create Buffer Cache\n");

        return -ENOMEM;
    }

    return 0;
}

static void __exit cmdlist_buffer_destruct(void)
{
    kmem_cache_destroy(cmdlist_cache_p);
}

void cmdlist_buffer_flush(void *buffer_p)
{
    cache_flush_len(buffer_p, CMDLIST_BUFFER_SIZE);
}

int __isEnetWanPort(uint32_t logicalPort)
{
#if defined (CONFIG_BCM_ENET_SYSPORT)
    return 0;
#else
    bcmFun_t *enetIsWanPortFun = bcmFun_get(BCM_FUN_ID_ENET_IS_WAN_PORT);
    int isWanPort = 0;

    BCM_ASSERT(enetIsWanPortFun != NULL);

    isWanPort = enetIsWanPortFun(&logicalPort);

    return (isWanPort);
#endif
}

int __isTxWlanPhy(Blog_t *blog_p)
{
    return (blog_p->tx.info.phyHdrType == BLOG_WLANPHY);
}

#else /* RDP_SIM */

#include "rdp_mm.h"

void *cmdlist_buffer_alloc(void **phys_addr_pp)
{
    bdmf_phys_addr_t phys_addr_p;
    void *buffer_p = rdp_mm_aligned_alloc_atomic(CMDLIST_BUFFER_SIZE, &phys_addr_p);

    if(buffer_p)
    {
        *phys_addr_pp = (void *)phys_addr_p;
    }

    __logDebug("buffer_p %p, phys_addr_p %p", buffer_p, *phys_addr_pp);

    return buffer_p;
}

void cmdlist_buffer_free(void *buffer_p)
{
    __logDebug("buffer_p %p", buffer_p);

    rdp_mm_aligned_free(buffer_p, CMDLIST_BUFFER_SIZE);
}

static int __init cmdlist_buffer_construct(void)
{
    return 0;
}

static void __exit cmdlist_buffer_destruct(void)
{
}

void cmdlist_buffer_flush(void *buffer_p)
{
}

#endif /* RDP_SIM */

uint32_t __buildBrcmTagType2(Blog_t *blog_p)
{
    int switch_port;
    int switch_queue;
    uint32_t tag;

    if(cmdlist_hooks_g.brcm_tag_info)
    {
        cmdlist_hooks_g.brcm_tag_info(blog_p, &switch_port, &switch_queue);
    }
    else
    {
        switch_port = LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel);
        switch_queue = SKBMARK_GET_Q_PRIO(blog_p->mark);
    }

    tag = (BRCM_TAG2_EGRESS | (switch_queue << 10)) << 16;
    tag |= (uint16_t)(1 << switch_port);

    return tag;
}

/*******************************************************************************
 *
 * API
 *
 *******************************************************************************/

int cmdlist_bind(cmdlist_hooks_t *hooks_p)
{
    cmdlist_hooks_g.ipv6_addresses_table_add = hooks_p->ipv6_addresses_table_add;
    cmdlist_hooks_g.ipv4_addresses_table_add = hooks_p->ipv4_addresses_table_add;
    cmdlist_hooks_g.brcm_tag_info = hooks_p->brcm_tag_info;

    CMDLIST_RETURN(CMDLIST_RET_OK);
}

void cmdlist_unbind(void)
{
    memset(&cmdlist_hooks_g, 0, sizeof(cmdlist_hooks_t));
}

void cmdlist_init(uint32_t *cmd_list_p, uint32_t cmd_list_length_max, uint32_t cmd_list_start_offset)
{
    cmdlist_init_bin(cmd_list_p, cmd_list_length_max, cmd_list_start_offset);
}

int cmdlist_ucast_create(Blog_t *blog_p, cmdlist_cmd_target_t target,
                         uint8_t *prependData_p, int prependSize, void **buffer_pp,
                         cmdlist_brcm_tag_t brcm_tag)
{
    int ret;

    if(blog_p->fc_hybrid)
        ret = cmdlist_ucast_fc_context_add_bin(blog_p, target);
    else
        ret = cmdlist_ucast_create_bin(blog_p, target, prependData_p, prependSize, buffer_pp, brcm_tag);

    if(!ret)
    {
        ret = cmdlist_cmd_end_bin();
        if(ret != 0)
        {
            __logInfo("Could not cmdlist_cmd_end_bin");
        }
    }

    CMDLIST_RETURN(ret);
}

int cmdlist_l2_ucast_create(Blog_t *blog_p, cmdlist_cmd_target_t target,
                            uint8_t *prependData_p, int prependSize, void **buffer_pp,
                            cmdlist_brcm_tag_t brcm_tag)
{
    int ret;

    if(blog_p->fc_hybrid)
        ret = cmdlist_ucast_fc_context_add_bin(blog_p, target);
    /* For L2GRE_terminated_flow, use L3_ucast cmdlist */
    else if((TG2in4UP(blog_p) || TG2in6UP(blog_p)) && !PT(blog_p))
    {
        __logInfo("cmdlist_l2_ucast: L2GRE terminated, use L3_ucast cmdlist");
        ret = cmdlist_ucast_create_bin(blog_p, target, prependData_p, prependSize, buffer_pp, brcm_tag);
    }
    else  
        ret = cmdlist_l2_ucast_create_bin(blog_p, target, prependData_p, prependSize, buffer_pp, brcm_tag);

    if(!ret)
    {
        ret = cmdlist_cmd_end_bin();
        if(ret != 0)
        {
            __logInfo("Could not cmdlist_cmd_end_bin");
        }
    }

    CMDLIST_RETURN(ret);
}

void cmdlist_mcast_parse(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p,
                         cmdlist_brcm_tag_t brcm_tag)
{
    cmdlist_mcast_parse_bin(blog_p, blogRule_p, parse_p, brcm_tag);
}

int cmdlist_l2_mcast_create(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p)
{
    int ret = cmdlist_l2_mcast_create_bin(blog_p, blogRule_p, parse_p);

    if(!ret)
    {
        ret = cmdlist_cmd_end_bin();
        if(ret != 0)
        {
            __logInfo("Could not cmdlist_cmd_end_bin");
        }
    }

    CMDLIST_RETURN(ret);
}

int cmdlist_l3_mcast_create(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p, uint8_t *isRouted_p)
{
    int ret = cmdlist_l3_mcast_create_bin(blog_p, blogRule_p, parse_p, isRouted_p);

    if(!ret)
    {
        ret = cmdlist_cmd_end_bin();
        if(ret != 0)
        {
            __logInfo("Could not cmdlist_cmd_end_bin");
        }
    }

    CMDLIST_RETURN(ret);
}

int cmdlist_l2_l3_mcast_create(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p, uint8_t *isRouted_p)
{
    int ret;

    ret = cmdlist_pre_mcast_create_bin(parse_p);
    if(!ret)
    {
        ret = cmdlist_l3_mcast_create_bin(blog_p, blogRule_p, parse_p, isRouted_p);
        if(!ret)
        {
            ret = cmdlist_l2_mcast_create_bin(blog_p, blogRule_p, parse_p);
            if(!ret)
            {
                ret = cmdlist_post_mcast_create_bin(parse_p);
                if(!ret)
                {
                    ret = cmdlist_cmd_end_bin();
                    if(ret)
                    {
                        __logInfo("Could not cmdlist_cmd_end_bin");
                    }
                }
            }
        }
    }

    CMDLIST_RETURN(ret);
}

uint32_t cmdlist_get_length(void)
{
    return cmdlist_get_length_bin();
}

void cmdlist_dump(uint32_t *cmd_list_p, int length32)
{
    cmdlist_dump_bin(cmd_list_p, length32);
}

void cmdlist_dump_partial(void)
{
    cmdlist_dump_partial_bin();
}

#if !defined(RDP_SIM)
uint32_t cmdlist_print_stats(void *sf)
{
    uint32_t bytes = 0;

    CMDLIST_STATS_PRINT(sf, bytes, "CMDLIST Stats:\n");
    CMDLIST_STATS_PRINT(sf, bytes, "Success        : %u\n",CMDLIST_RET_CODE_STATS(CMDLIST_RET_OK));
    CMDLIST_STATS_PRINT(sf, bytes, "Errors         : %u\n",CMDLIST_RET_CODE_STATS(CMDLIST_RET_ERR));
    CMDLIST_STATS_PRINT(sf, bytes, "Unsupported    : %u\n",CMDLIST_RET_CODE_STATS(CMDLIST_RET_ERR_UNSUPPORTED));
    CMDLIST_STATS_PRINT(sf, bytes, "Overflow       : %u\n",CMDLIST_RET_CODE_STATS(CMDLIST_RET_ERR_OVERFLOW));
    CMDLIST_STATS_PRINT(sf, bytes, "Invalid Target : %u\n",CMDLIST_RET_CODE_STATS(CMDLIST_RET_ERR_INV_TARGET));

    return bytes;
}
#endif
/*******************************************************************************
 *
 * Driver
 *
 *******************************************************************************/
void __init cmdlist_err_stats_init(void)
{
    memset(cmdlist_err_stats, 0, sizeof(cmdlist_err_stats));
}

int __init cmdlist_construct(void)
{
    int ret;

#if defined(CONFIG_BCM_CMDLIST_SIM)
    bcmLog_setLogLevel(BCM_LOG_ID_CMDLIST, BCM_LOG_LEVEL_DEBUG);
#else
    bcmLog_setLogLevel(BCM_LOG_ID_CMDLIST, BCM_LOG_LEVEL_ERROR);
#endif

    cmdlist_unbind();

#if defined(CONFIG_CMDLIST_GPE)
    {
#if defined(BCM63158) && !defined(CONFIG_BCM963146_EMULATION)
        int target_ddr_supported = 0;
#else
        int target_ddr_supported = 1;
#endif
        cmdlist_config_bin(RDPA_CMD_LIST_PACKET_BUFFER_OFFSET,
                           RDPA_CMD_LIST_PACKET_HEADER_OFFSET,
                           RDPA_CMD_LIST_HEADROOM,
                           target_ddr_supported);
    }
#else // SPE
    cmdlist_config_bin(0, 0, 0, 0);
#endif

    ret = cmdlist_buffer_construct();

    cmdlist_err_stats_init();

    bcm_print("Broadcom Command List Driver v1.0\n");

    return ret;
}

void __exit cmdlist_destruct(void)
{
    cmdlist_buffer_destruct();
}

#if !defined(RDP_SIM)
module_init(cmdlist_construct);
module_exit(cmdlist_destruct);

EXPORT_SYMBOL(cmdlist_init);
EXPORT_SYMBOL(cmdlist_ucast_create);
EXPORT_SYMBOL(cmdlist_l2_ucast_create);
EXPORT_SYMBOL(cmdlist_mcast_parse);
EXPORT_SYMBOL(cmdlist_l2_mcast_create);
EXPORT_SYMBOL(cmdlist_l3_mcast_create);
EXPORT_SYMBOL(cmdlist_l2_l3_mcast_create);
EXPORT_SYMBOL(cmdlist_get_length);
EXPORT_SYMBOL(cmdlist_bind);
EXPORT_SYMBOL(cmdlist_unbind);
EXPORT_SYMBOL(cmdlist_dump);
EXPORT_SYMBOL(cmdlist_dump_partial);
EXPORT_SYMBOL(cmdlist_buffer_free);
EXPORT_SYMBOL(cmdlist_print_stats);

MODULE_DESCRIPTION("Broadcom Command List Driver");
MODULE_VERSION("v1.0");

MODULE_LICENSE("Proprietary");
#endif
