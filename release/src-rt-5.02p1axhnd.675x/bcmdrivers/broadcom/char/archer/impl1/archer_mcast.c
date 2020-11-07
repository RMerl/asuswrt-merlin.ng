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
* File Name  : archer_mcast.c
*
* Description: Translation of Multicast Blogs into Multicast Archer Flows.
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
#include "archer_drop.h"

#include "cmdlist_api.h"

/*******************************************************************************
 *
 * Local Functions
 *
 *******************************************************************************/

static blogRuleAction_t *__find_blog_rule_command(blogRule_t *blog_rule_p,
                                                  blogRuleCommand_t blog_rule_command,
                                                  uint32 *cmdIndex_p)
{
    blogRuleAction_t *action_p;
    int i;

    for(i=*cmdIndex_p; i<blog_rule_p->actionCount; ++i)
    {
        action_p = &blog_rule_p->action[i];

        if(action_p->cmd == blog_rule_command)
        {
            *cmdIndex_p = i;

            return action_p;
        }
    }

    return NULL;
}

static inline blogRuleAction_t *find_blog_rule_command(blogRule_t *blog_rule_p,
                                                       blogRuleCommand_t blog_rule_command)
{
    uint32 cmdIndex = 0;

    return __find_blog_rule_command(blog_rule_p, blog_rule_command, &cmdIndex);
}

static int archer_mcast_flow_tuple_set(Blog_t *blog_p, sysport_classifier_flow_t *flow_p,
                                       sysport_rsb_flow_tuple_info_t *flow_info_p,
                                       sysport_classifier_rsb_overwrite_t *rsb_overwrite_p)
{
    sysport_rsb_flow_mcast_t *mcast_p = &flow_p->tuple.mcast;
    int vlan_id;

    mcast_p->header.valid = 1;
    mcast_p->header.flow_type = SYSPORT_RSB_FLOW_TYPE_MCAST;

    if(blog_p->rx.info.bmap.BCM_XPHY)
    {
        mcast_p->header.ingress_phy = SYSPORT_RSB_PHY_DSL;
        mcast_p->header.ingress_port = blog_p->rx.info.channel;
    }
    else if(ARCHER_RX_ENET_WAN(blog_p))
    {
        int enet_ingress_intf_index;
        int enet_ingress_phys_port;

        sysport_driver_logical_port_to_phys_port(blog_p->rx.info.channel,
                                                 &enet_ingress_intf_index,
                                                 &enet_ingress_phys_port);

        mcast_p->header.ingress_phy = SYSPORT_RSB_PHY_ETH_WAN;
        mcast_p->header.ingress_port = blog_p->rx.info.channel;
        rsb_overwrite_p->parser.header.ingress_port = enet_ingress_phys_port;
    }
    else
    {
        __logError("Invalid Multicast Source PHY: %d", blog_p->rx.info.phyHdrType);

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }

    mcast_p->ip_protocol = blog_p->key.protocol;

#if defined(CONFIG_BLOG_IPV6)
    if(blog_p->rx.info.bmap.PLD_IPv6)
    {
        if(blog_p->is_ssm)
        {
            /* Use source to lookup only for SSM flows */
            mcast_p->ip_src_addr = sysport_parser_crc32(blog_p->tupleV6.saddr.p8,
                                                        BLOG_IPV6_ADDR_LEN);
        }
        else
        {
            mcast_p->ip_src_addr = 0;
        }

        mcast_p->ip_dst_addr = sysport_parser_crc32(blog_p->tupleV6.daddr.p8,
                                                    BLOG_IPV6_ADDR_LEN);

        flow_info_p->l3.is_ipv6 = 1;
        if(blog_p->is_ssm)
        {
            memcpy(flow_info_p->l3.ipv6.src_addr.u8, blog_p->tupleV6.saddr.p8, BLOG_IPV6_ADDR_LEN);
        }
        else
        {
            memset(flow_info_p->l3.ipv6.src_addr.u8, 0, BLOG_IPV6_ADDR_LEN);
        }
        memcpy(flow_info_p->l3.ipv6.dst_addr.u8, blog_p->tupleV6.daddr.p8, BLOG_IPV6_ADDR_LEN);
    }
    else
#endif
    {
        if (blog_p->is_ssm) 
        {
            /* Use source to lookup only for SSM flows */
            mcast_p->ip_src_addr = ntohl(blog_p->rx.tuple.saddr);
        }
        else
        {
            mcast_p->ip_src_addr = 0;
        }

        mcast_p->ip_dst_addr = ntohl(blog_p->rx.tuple.daddr);

        memset(flow_info_p, 0, sizeof(sysport_rsb_flow_tuple_info_t));
    }

    mcast_p->nbr_of_vlans = blog_p->vtag_num;

    vlan_id = ntohl(blog_p->vtag[0]);
    mcast_p->vlan.outer_vlan_id = vlan_id & 0xFFF;

    vlan_id = ntohl(blog_p->vtag[1]);
    mcast_p->vlan.inner_vlan_id = vlan_id & 0xFFF;

    return 0;
}

static int archer_mcast_flow_context_set(Blog_t *blog_p, sysport_classifier_flow_t *flow_p,
                                         sysport_cmdlist_table_t *cmdlist_p, int *cmdlist_length_p)
{
    sysport_classifier_flow_context_t *context_p = &flow_p->context;
    blogRule_t *blog_rule_p;
    sysport_driver_switch_t switch_mode;
    cmdlist_brcm_tag_t brcm_tag;
    cmdlist_mcast_parse_t parse;
    uint8_t is_routed;
    int egress_port;
    int ret;

    blog_rule_p = blog_p->blogRule_p;
    if(blog_rule_p == NULL)
    {
        __logError("NULL Blog Rule");

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }

    __dump_blog_rule(blog_rule_p);

    if(BLOG_WLANPHY == blog_p->tx.info.phyHdrType)
    {
        if(blog_p->wfd.mcast.is_wfd)
        {
            sysport_classifier_flow_port_t *port_p;

            __logInfo("WLAN Multicast Client\n");

            if(blog_p->wfd.mcast.wfd_idx >= SYSPORT_FLOW_WLAN_PORTS_MAX)
            {
                __logError("Invalid wfd_idx %d (max %d)",
                           blog_p->wfd.mcast.wfd_idx, SYSPORT_FLOW_WLAN_PORTS_MAX);

                return SYSPORT_CLASSIFIER_ERROR_INVALID;
            }

            if(blog_p->wfd.mcast.wfd_prio >= SYSPORT_FLOW_WLAN_QUEUES_MAX)
            {
                __logError("Invalid wfd_prio %d (max %d)",
                           blog_p->wfd.mcast.wfd_prio, SYSPORT_FLOW_WLAN_QUEUES_MAX);

                return SYSPORT_CLASSIFIER_ERROR_INVALID;
            }

            if(blog_p->wfd.mcast.ssid >= SYSPORT_FLOW_WLAN_SSID_MAX)
            {
                __logError("SSID %d exceeds the maximum number of SSIDs (%d)",
                           blog_p->wfd.mcast.ssid, SYSPORT_FLOW_WLAN_SSID_MAX);

                return SYSPORT_CLASSIFIER_ERROR_INVALID;
            }

            egress_port = SYSPORT_FLOW_WLAN_PORT_0 + blog_p->wfd.mcast.wfd_idx;

            port_p = &context_p->port[egress_port];

            port_p->u32 = 0;

            // wfd_queue index = (wfd_idx << 1) + wfd_prio
            port_p->egress_queue = ((blog_p->wfd.mcast.wfd_idx * 2) +
                                    blog_p->wfd.mcast.wfd_prio);

            port_p->wlan_ssid_vector = (1 << blog_p->wfd.mcast.ssid);

            context_p->wlan_priority = SKBMARK_GET_Q_PRIO(blog_p->mark);
        }
        else
        {
            __logError("Only WFD acceleration is supported");

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        switch_mode = SYSPORT_DRIVER_SWITCH_NONE;
    }
    else
    {
        sysport_classifier_flow_port_t *port_p;
        blogRuleAction_t *blog_rule_action_p;
        archer_drop_config_t drop_config;
        int switch_queue;
        int txq_index;

        __logInfo("Ethernet Multicast Client\n");

        egress_port = blog_p->tx.info.channel;

        port_p = &context_p->port[egress_port];

        blog_rule_action_p = find_blog_rule_command(blog_rule_p,
                                                    BLOG_RULE_CMD_SET_SKB_MARK_QUEUE);
        if(blog_rule_action_p)
        {
            switch_queue = blog_rule_action_p->arg;
        }
        else
        {
            switch_queue = SKBMARK_GET_Q_PRIO(blog_p->mark);
        }

        if(sysport_driver_switch_queue_to_txq_index(egress_port, switch_queue, &txq_index))
        {
            __logError("Could not sysport_driver_switch_queue_to_txq_index");

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        port_p->egress_queue = txq_index;

        if(sysport_driver_switch_mode_get(egress_port, port_p->egress_queue, &switch_mode))
        {
            __logError("Could not sysport_driver_switch_mode_get");

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        ret = sysport_driver_drop_config_get(egress_port, switch_queue, &drop_config);
        if(ret)
        {
            __logError("Could not sysport_driver_drop_config_get");

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        context_p->drop_profile =
            archer_drop_profile_by_tc(&drop_config, SKBMARK_GET_TC_ID(blog_p->mark));
    }

    // The Egress PHY is not used for MCAST
    context_p->egress_phy = SYSPORT_RSB_PHY_ETH_LAN;

    context_p->egress_port_or_mask = (1 << egress_port);

    context_p->mtu = blog_getTxMtu(blog_p);

#if 0 // XXX
    if((blog_p->mcast_learn) && (blog_p->rx.info.bmap.PLD_IPv4))
    {
        if(blog_p->rx.tuple.tos != blog_p->tx.tuple.tos)
        {
            result_p->is_tos_mangle = 1;
            result_p->tos = blog_p->rx.tuple.tos;
        }
    }
#endif

#if defined(CONFIG_BCM_ARCHER_SIM)
    {
        struct net_device *dev_p = (struct net_device *)
            blog_p->tx_dev_p;

        context_p->dev_xmit = (sysport_classifier_dev_xmit)
            dev_p->netdev_ops->ndo_start_xmit;

        context_p->tx_dev_p = blog_p->tx_dev_p;
    }
#endif

    /*
     * Create L2/L3 Command List
     */

    __debug("\n*** Command List ***\n\n");

    if(SYSPORT_DRIVER_SWITCH_NONE == switch_mode)
    {
        brcm_tag = CMDLIST_BRCM_TAG_NONE;
    }
    else
    {
        brcm_tag = CMDLIST_BRCM_TAG_PT;
    }

    cmdlist_init(cmdlist_p->cmdlist, CMDLIST_CMD_LIST_SIZE_MAX, 0);

    cmdlist_mcast_parse(blog_p, blog_rule_p, &parse, brcm_tag);

    ret = cmdlist_l2_l3_mcast_create(blog_p, blog_rule_p, &parse, &is_routed);
    if(ret)
    {
        __logError("Could not cmdlist_l2_l3_mcast_create");

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }

    *cmdlist_length_p = cmdlist_get_length();

    __debug("cmdlist_length = %u\n", *cmdlist_length_p);
    __dump_cmdlist(cmdlist_p->cmdlist);

    context_p->ip_addr_index = SYSPORT_IP_ADDR_TABLE_INVALID;

    context_p->is_routed = is_routed;

    return 0;
}

static int archer_mcast_egress_port_count(sysport_classifier_flow_t *flow_p)
{
    int egress_port_mask = flow_p->context.egress_port_or_mask;
    int egress_port_count = 0;
    int egress_port;

    while(egress_port_mask)
    {
        egress_port = sysport_classifier_port_mask_to_port(egress_port_mask);

        egress_port_mask &= ~(1 << egress_port);

        egress_port_count++;
    }

    return egress_port_count;
}

int archer_mcast_activate(Blog_t *blog_p, sysport_flow_key_t *flow_key_p, int *is_activation_p)
{
    sysport_classifier_flow_t new_flow;
    sysport_rsb_flow_tuple_info_t flow_info;
    sysport_classifier_rsb_overwrite_t rsb_overwrite;
    sysport_cmdlist_table_t cmdlist;
    int cmdlist_length;
    int ret;

    BCM_ASSERT(blog_p != BLOG_NULL);

    __logInfo("ACTIVATE");

    if((blog_p->key.protocol != IPPROTO_UDP) &&
       (blog_p->key.protocol != IPPROTO_TCP) &&
       (blog_p->key.protocol != IPPROTO_IPV6) &&
       (blog_p->key.protocol != IPPROTO_IPIP))
    {
        __logInfo("Flow Type proto<%d> is not supported", blog_p->key.protocol);

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }

    if(!SYSPORT_PARSER_IS_MCAST_IPV4(ntohl(blog_p->rx.tuple.daddr))
#if defined(CONFIG_BLOG_IPV6)
       && !SYSPORT_PARSER_IS_MCAST_IPV6(blog_p->tupleV6.daddr.p8[0])
#endif
        )
    {
        __logError("Not IPv4 or IPv6 Multicast : %pI4, %pI6",
                   &blog_p->rx.tuple.daddr, blog_p->tupleV6.daddr.p8);

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }

    if(blog_p->rx.info.channel == 0xFF)
    {
        __logInfo("LAN to LAN Multicast acceleration is not supported\n");

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }

    __debug("\n%s: ************** Multicast Flow **************\n\n", __FUNCTION__);

    memset(&new_flow, 0, sizeof(sysport_classifier_flow_t));

    sysport_classifier_rsb_overwrite_init(&rsb_overwrite);

    ret = archer_mcast_flow_tuple_set(blog_p, &new_flow, &flow_info, &rsb_overwrite);
    if(ret)
    {
        __logError("Could not archer_mcast_flow_tuple_set");

        return ret;
    }

    ret = archer_mcast_flow_context_set(blog_p, &new_flow,
                                        &cmdlist, &cmdlist_length);
    if(ret)
    {
        __logError("Could not archer_mcast_flow_context_set");

        return ret;
    }

    *is_activation_p = sysport_classifier_flow_find(&new_flow, flow_key_p, &rsb_overwrite);

    if(blog_p->mc_sync)
    {
        __logInfo("SYNC: dev=%pS >>>>", blog_p->tx_dev_p);

        if(*is_activation_p)
        {
            __logError("SYNC: Mcast Flow does not exist");

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        __logError("SYNC: Operation not supported");

        return SYSPORT_CLASSIFIER_ERROR_INVALID;
    }
    else if(*is_activation_p)
    {
        __logInfo("CREATE: dev=%pS >>>>", blog_p->tx_dev_p);

        /* Create new Flow */

        ret = sysport_classifier_flow_create(&new_flow, &flow_info, cmdlist.cmdlist,
                                             cmdlist_length, flow_key_p, &rsb_overwrite);
        if(ret)
        {
            __logInfo("Could not sysport_classifier_flow_create");

            return ret;
        }

#if defined(CC_ARCHER_SIM_FC_HOOK)
        archer_sim_fc_hook_set(blog_p);
#endif
        __logInfo("CREATE: SUCCESSFUL <<<<");
    }
    else /* mcast flow already exists */
    {
        int egress_port = sysport_classifier_port_mask_to_port(new_flow.context.egress_port_or_mask);
        sysport_classifier_flow_port_t *new_port_p = &new_flow.context.port[egress_port];
        int port_add = 1;

        __logInfo("ADD_PORT: dev=%pS >>>>", blog_p->tx_dev_p);

        if(BLOG_WLANPHY == blog_p->tx.info.phyHdrType)
        {
            sysport_classifier_flow_port_t port;

            ret = sysport_classifier_flow_port_get(*flow_key_p, egress_port, &port);
            if(ret)
            {
                __logError("ADD_PORT: Could not sysport_classifier_flow_port_get");

                return ret;
            }

            if(port.wlan_ssid_vector)
            {
                // Port has already been added
                port_add = 0;
            }

            if(new_port_p->wlan_ssid_vector & port.wlan_ssid_vector)
            {
                __logError("ADD_PORT: WLAN SSID has already been added: "
                           "egress_port %d, current 0x%04X, new 0x%04X",
                           egress_port, port.wlan_ssid_vector,
                           new_port_p->wlan_ssid_vector);

                return SYSPORT_CLASSIFIER_ERROR_INVALID;
            }

            new_port_p->wlan_ssid_vector |= port.wlan_ssid_vector;
        }

        if(port_add)
        {
            ret = sysport_classifier_flow_port_add(*flow_key_p, egress_port,
                                                   new_port_p, new_flow.context.mtu,
                                                   cmdlist.cmdlist, cmdlist_length);
            if(ret)
            {
                __logError("ADD_PORT: Could not sysport_classifier_flow_port_add");

                return ret;
            }
        }
        else
        {
            ret = sysport_classifier_flow_port_set(*flow_key_p, egress_port, new_port_p);
            if(ret)
            {
                __logError("ADD_PORT: Could not sysport_classifier_flow_port_set");

                return ret;
            }
        }

        __logInfo("ADD_PORT: SUCCESSFUL <<<<");
    }

    return 0;
}

int archer_mcast_deactivate(Blog_t *blog_p, int *is_deactivation_p)
{
    sysport_classifier_flow_t flow;
    sysport_rsb_flow_tuple_info_t flow_info;
    sysport_classifier_rsb_overwrite_t rsb_overwrite;
    sysport_flow_key_t flow_key;
    int egress_port_count;
    int egress_port;
    int port_delete = 1;
    int ret;

    BCM_ASSERT(blog_p != BLOG_NULL);

    __logInfo("DEACTIVATE");

    memset(&flow, 0, sizeof(sysport_classifier_flow_t));

    sysport_classifier_rsb_overwrite_init(&rsb_overwrite);

    ret = archer_mcast_flow_tuple_set(blog_p, &flow, &flow_info, &rsb_overwrite);
    if(ret)
    {
        __logError("Could not archer_mcast_flow_tuple_set");

        return ret;
    }

    ret = sysport_classifier_flow_find(&flow, &flow_key, &rsb_overwrite);
    if(ret)
    {
        __logError("Could not sysport_classifier_flow_find");

        return ret;
    }

    ret = sysport_classifier_flow_get(flow_key, &flow);
    if(ret)
    {
        __logError("Could not sysport_classifier_flow_get");

        return ret;
    }

    if(BLOG_WLANPHY == blog_p->tx.info.phyHdrType)
    {
        sysport_classifier_flow_port_t *port_p;
        uint16_t blog_ssid_mask;

        if(blog_p->wfd.mcast.wfd_idx >= SYSPORT_FLOW_WLAN_PORTS_MAX)
        {
            __logError("Invalid wfd_idx %d (max %d)",
                       blog_p->wfd.mcast.wfd_idx, SYSPORT_FLOW_WLAN_PORTS_MAX);

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        if(blog_p->wfd.mcast.ssid >= SYSPORT_FLOW_WLAN_SSID_MAX)
        {
            __logError("SSID %d exceeds the maximum number of SSIDs (%d)",
                       blog_p->wfd.mcast.ssid, SYSPORT_FLOW_WLAN_SSID_MAX);

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        egress_port = SYSPORT_FLOW_WLAN_PORT_0 + blog_p->wfd.mcast.wfd_idx;

        if(!(flow.context.egress_port_or_mask & (1 << egress_port)))
        {
            __logError("Port <%d:%d> has not joined flow key 0x%08X",
                       blog_p->tx.info.channel, egress_port, flow_key.u32);

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        port_p = &flow.context.port[egress_port];

        blog_ssid_mask = (1 << blog_p->wfd.mcast.ssid);

        if(!(port_p->wlan_ssid_vector & blog_ssid_mask))
        {
            __logError("ADD_PORT: WLAN SSID has not been added: "
                       "egress_port %d, curr 0x%04X, blog 0x%04X",
                       egress_port, port_p->wlan_ssid_vector,
                       blog_ssid_mask);

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }

        port_p->wlan_ssid_vector &= ~blog_ssid_mask;

        if(port_p->wlan_ssid_vector)
        {
            __logInfo("SET_PORT: dev=%pS >>>>", blog_p->tx_dev_p);

            ret = sysport_classifier_flow_port_set(flow_key, egress_port, port_p);
            if(ret)
            {
                __logError("ADD_PORT: Could not sysport_classifier_flow_port_set");

                return ret;
            }

            port_delete = 0;

            __logInfo("SET_PORT: SUCCESSFUL <<<<");
        }
    }
    else
    {
        egress_port = blog_p->tx.info.channel;

        if(!(flow.context.egress_port_or_mask & (1 << egress_port)))
        {
            __logError("Port <%d:%d> has not joined flow key 0x%08X",
                       blog_p->tx.info.channel, egress_port, flow_key.u32);

            return SYSPORT_CLASSIFIER_ERROR_INVALID;
        }
    }

    egress_port_count = archer_mcast_egress_port_count(&flow);

    BCM_ASSERT(egress_port_count && (egress_port_count <= SYSPORT_FLOW_PORTS_MAX));

    *is_deactivation_p = 0;

    if(port_delete)
    {
        /* Delete or remove port from mcast flow */

        if(egress_port_count == 1)
        {
            __logInfo("DELETE: >>>>");

            *is_deactivation_p = 1;

            ret = sysport_classifier_flow_delete(flow_key);
            if(ret)
            {
                __logError("Could not sysport_classifier_flow_delete (flow_key 0x%08X)", flow_key.u32);

                return ret;
            }

            __logInfo("DELETE: SUCCESSFUL <<<<");
        }
        else
        {
            __logInfo("REM_PORT: dev=%pS >>>>", blog_p->tx_dev_p);

            ret = sysport_classifier_flow_port_delete(flow_key, egress_port);
            if(ret)
            {
                __logError("Could not sysport_classifier_flow_port_delete <%d:%d>, flow key 0x%08X",
                           blog_p->tx.info.channel, egress_port, flow_key.u32);

                return ret;
            }

            __logInfo("REM_PORT: SUCCESSFUL <<<<");
        }

        egress_port_count--;
    }

    /* return number of associations to the mcast flow */

    return egress_port_count;
}
