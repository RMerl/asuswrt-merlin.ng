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

/*
*******************************************************************************
*
* File Name  : archer_ucast_l2.c
*
* Description: Translation of Unicast Blogs into L2 Unicast Archer Flows.
*
*******************************************************************************
*/
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include "bcmenet.h"
#include "pktHdr.h"

#include "sysport_rsb.h"
#include "sysport_parser.h"
#include "sysport_classifier.h"
#include "sysport_driver.h"

#include "archer.h"
#include "archer_driver.h"

#include "cmdlist_api.h"

static int archer_ucast_l2_flow_set(Blog_t *blog_p, sysport_classifier_flow_t *flow_p,
                                    sysport_rsb_flow_tuple_info_t *flow_info_p,
                                    sysport_classifier_rsb_overwrite_t *rsb_overwrite_p)
{
    sysport_rsb_flow_ucast_l2_t *ucast_l2_p = &flow_p->tuple.ucast_l2;
    sysport_classifier_flow_context_t *context_p = &flow_p->context;
//    BlogVlanHdr_t *vlan_p = (BlogVlanHdr_t *)&blog_p->rx.l2hdr[BLOG_ETH_ADDR_LEN * 2];
    uint32_t *vlan_p = blog_p->vtag;

    if(blog_p->l2_ipv4 || blog_p->l2_ipv6)
    {
        ucast_l2_p->ip_tos = blog_p->rx.tuple.tos;
    }
    else if(blog_p->rx.tuple.tos == SYSPORT_CLASSIFIER_UCAST_L2_IP_TOS_UNKNOWN)
    {
        ucast_l2_p->ip_tos = SYSPORT_CLASSIFIER_UCAST_L2_IP_TOS_UNKNOWN;

        rsb_overwrite_p->parser.ucast_l2.ip_tos = 0;
    }
    else
    {
        __logError("Invalid blog_p->rx.tuple.tos 0x%02x", blog_p->rx.tuple.tos);

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }

    ucast_l2_p->ethertype = ntohs(blog_p->eth_type);

    ucast_l2_p->dst_mac_crc32 =
        sysport_parser_crc32(&blog_p->rx.l2hdr[0], BLOG_ETH_ADDR_LEN);

    ucast_l2_p->src_mac_crc32 =
        sysport_parser_crc32(&blog_p->rx.l2hdr[6], BLOG_ETH_ADDR_LEN);

    if(blog_p->vtag_num)
    {
        ucast_l2_p->vlan_tag_crc32 =
            sysport_parser_crc32(vlan_p, (blog_p->vtag_num * sizeof(BlogVlanHdr_t)));
    }
    else
    {
        ucast_l2_p->vlan_tag_crc32 = 0;
    }

    memcpy(flow_info_p->l2.dst_mac.u8, &blog_p->rx.l2hdr[0], BLOG_ETH_ADDR_LEN);
    memcpy(flow_info_p->l2.src_mac.u8, &blog_p->rx.l2hdr[6], BLOG_ETH_ADDR_LEN);
    flow_info_p->l2.nbr_of_vlans = blog_p->vtag_num;
    if(blog_p->vtag_num)
    {
        memcpy(&flow_info_p->l2.vlan[0].u8, vlan_p, (blog_p->vtag_num * sizeof(BlogVlanHdr_t)));
    }

    context_p->mtu = blog_getTxMtu(blog_p);

    __debug("Dst MAC <%02x:%02x:%02x:%02x:%02x:%02x>\n", 
            blog_p->rx.l2hdr[0], blog_p->rx.l2hdr[1], blog_p->rx.l2hdr[2],
            blog_p->rx.l2hdr[3], blog_p->rx.l2hdr[4], blog_p->rx.l2hdr[5] );
    __debug("Src MAC <%02x:%02x:%02x:%02x:%02x:%02x>\n", 
            blog_p->rx.l2hdr[6], blog_p->rx.l2hdr[7], blog_p->rx.l2hdr[8],
            blog_p->rx.l2hdr[9], blog_p->rx.l2hdr[10], blog_p->rx.l2hdr[11] );
    __debug("vtag_num = %u, vtag[0] = 0x%08x, vtag[1] = 0x%08x, eth_type = 0x%04x tos= 0x%02x\n", 
            blog_p->vtag_num, ntohl(blog_p->vtag[0]), ntohl(blog_p->vtag[1]), ntohs(blog_p->eth_type), 
            blog_p->rx.tuple.tos) ;
    __debug("\n");

#if 0
    if(blog_p->rx.tuple.tos != blog_p->tx.tuple.tos)
    {
        l2_flow_p->result.is_tos_mangle = 1;
        l2_flow_p->result.tos = blog_p->tx.tuple.tos;
    }
#endif

    return 0;
}

int archer_ucast_l2_activate(Blog_t *blog_p, sysport_flow_key_t *flow_key_p,
                             uint8_t *prependData_p, int prependSize)
{
    sysport_classifier_flow_t flow;
    sysport_rsb_flow_tuple_info_t flow_info;
    sysport_classifier_rsb_overwrite_t rsb_overwrite;
    sysport_cmdlist_table_t cmdlist;
    cmdlist_brcm_tag_t brcm_tag;
    void *buffer_p = NULL;
    int cmdlist_length;
    int ret;

    sysport_classifier_rsb_overwrite_init(&rsb_overwrite);

    ret = archer_ucast_common_flow_set(blog_p, &flow,
                                       SYSPORT_RSB_FLOW_TYPE_UCAST_L2,
                                       &brcm_tag, &rsb_overwrite);
    if(ret)
    {
        __logInfo("Could not archer_ucast_common_flow_set");

        goto abort_activate;
    }

    ret = archer_ucast_l2_flow_set(blog_p, &flow, &flow_info, &rsb_overwrite);
    if(ret)
    {
        __logInfo("Could not archer_ucast_l2_flow_set");

        goto abort_activate;
    }

    cmdlist_init(cmdlist.cmdlist, CMDLIST_CMD_LIST_SIZE_MAX, 0);

    ret = cmdlist_l2_ucast_create(blog_p, CMDLIST_CMD_TARGET_SRAM,
                                  prependData_p, prependSize, &buffer_p, brcm_tag);
    if(ret)
    {
        __logInfo("Could not cmdlist_l2_ucast_create");

        goto abort_activate;
    }

    flow.context.ip_addr_index = SYSPORT_IP_ADDR_TABLE_INVALID;

    cmdlist_length = cmdlist_get_length();

    __debug("cmdlist_length = %u\n", cmdlist_length);
    if(cmdlist_length)
    {
        __dump_cmdlist(cmdlist.cmdlist);
    }

    ret = sysport_classifier_flow_create(&flow, &flow_info, cmdlist.cmdlist,
                                         cmdlist_length, flow_key_p, &rsb_overwrite);
    if(ret)
    {
        __logInfo("Could not sysport_classifier_flow_create");

        goto abort_activate;
    }

#if defined(CC_ARCHER_SIM_FC_HOOK)
    archer_sim_fc_hook_set(blog_p);
#endif

    return 0;

abort_activate:
    return ret;
}

int archer_ucast_l2_deactivate(sysport_flow_key_t flow_key)
{
    int ret;

    ret = sysport_classifier_flow_delete(flow_key);
    if(ret)
    {
        __logError("Could not sysport_classifier_flow_delete (flow_key 0x%08X)",
                   flow_key.u32);
        return ret;
    }

    return 0;
}
