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
* File Name  : sysport_rsb.c
*
* Description: This file contains the System Port RSB support functions
*
*******************************************************************************
*/

#if defined(__KERNEL__)
#include <linux/module.h>
#include <linux/blog_net.h>
#else
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define CONFIG_BCM_KF_BLOG
#include "blog_net.h"
#undef CONFIG_BCM_KF_BLOG
#endif

#include "sysport_print.h"
#include "sysport_rsb.h"

const char *sysport_rsb_flow_type_name[SYSPORT_RSB_FLOW_TYPE_MAX] =
{
    "UNKNOWN",
    "UCAST_L3",
    "UCAST_L2",
    "MCAST"
};

const char *sysport_rsb_phy_name[SYSPORT_RSB_PHY_MAX] =
{
    "ETH_LAN",
    "ETH_WAN",
    "DSL",
    "WLAN"
};

const char *sysport_rsb_l3_type_name[SYSPORT_RSB_L3_TYPE_MAX] =
{
    "UNKNOWN",
    "IPV4",
    "IPV6",
    "4IN6",
    "6IN4"
};

static char *__rsb_l3_type_name(int l3_type)
{
    char *l3_type_name;

    if(l3_type < SYSPORT_RSB_L3_TYPE_MAX)
    {
        l3_type_name = (char *)sysport_rsb_l3_type_name[l3_type];
    }
    else
    {
        l3_type_name = "OUT_OF_RANGE";
    }

    return l3_type_name;
}

void sysport_rsb_mem_dump(void *p, int length)
{
    uint8_t *mem_p = (uint8_t *)p;
    int i;

    bcm_print("addr <0x%px>, length <%d>\n", mem_p, length);

    for(i=0; i<length; ++i)
    {
        if((i != 0) && ((i % 16) == 0))
        {
            bcm_print("\n");
        }
        bcm_print("%02X ", *(mem_p + i));
    }

    bcm_print("\n");
}

void sysport_rsb_tuple_dump(sysport_rsb_flow_tuple_t *tuple_p,
                            sysport_rsb_flow_tuple_info_t *tuple_info_p)
{
    sysport_rsb_flow_header_t *header_p = &tuple_p->header;

    bcm_print("valid %d, flow_type %s (%d), ingress_phy %s (%d), ingress_port %d\n",
              header_p->valid, sysport_rsb_flow_type_name[header_p->flow_type], header_p->flow_type,
              sysport_rsb_phy_name[header_p->ingress_phy], header_p->ingress_phy, header_p->ingress_port);

    switch(header_p->flow_type)
    {
        case SYSPORT_RSB_FLOW_TYPE_UNKNOWN:
            break;

        case SYSPORT_RSB_FLOW_TYPE_UCAST_L3:
        {
            sysport_rsb_flow_ucast_l3_t *ucast_l3_p = &tuple_p->ucast_l3;

            if(tuple_info_p)
            {
                if(tuple_info_p->l3.is_ipv6)
                {
                    bcm_print("ip_protocol %d, ip_tos %d\n"
                              "IPv6 src_addr " BLOG_IPV6_ADDR_FMT " (0x%08x)\n"
                              "IPv6 dst_addr " BLOG_IPV6_ADDR_FMT " (0x%08x)\n"
                              "src_port 0x%04x, dst_port 0x%04x\n",
                              ucast_l3_p->ip_protocol, ucast_l3_p->ip_tos,
                              BLOG_IPV6_ADDR(tuple_info_p->l3.ipv6.src_addr.u8), ucast_l3_p->ip_src_addr,
                              BLOG_IPV6_ADDR(tuple_info_p->l3.ipv6.dst_addr.u8), ucast_l3_p->ip_dst_addr,
                              ucast_l3_p->l4_ports.src_port, ucast_l3_p->l4_ports.dst_port);
                }
                else
                {
                    bcm_print("ip_protocol %d, ip_tos %d\n"
                              "IPv4 src_addr " BLOG_IPV4_ADDR_FMT "\n"
                              "IPv4 dst_addr " BLOG_IPV4_ADDR_FMT "\n"
                              "src_port 0x%04x, dst_port 0x%04x\n",
                              ucast_l3_p->ip_protocol, ucast_l3_p->ip_tos,
                              BLOG_IPV4_ADDR_HOST(ucast_l3_p->ip_src_addr),
                              BLOG_IPV4_ADDR_HOST(ucast_l3_p->ip_dst_addr),
                              ucast_l3_p->l4_ports.src_port, ucast_l3_p->l4_ports.dst_port);
                }
            }
            else
            {
                bcm_print("ip_protocol %d, ip_tos %d\n"
                          "ip_src_addr " BLOG_IPV4_ADDR_FMT " (0x%08x)\n"
                          "ip_dst_addr " BLOG_IPV4_ADDR_FMT " (0x%08x)\n"
                          "src_port 0x%04x, dst_port 0x%04x\n",
                          ucast_l3_p->ip_protocol, ucast_l3_p->ip_tos,
                          BLOG_IPV4_ADDR_HOST(ucast_l3_p->ip_src_addr), ucast_l3_p->ip_src_addr,
                          BLOG_IPV4_ADDR_HOST(ucast_l3_p->ip_dst_addr), ucast_l3_p->ip_dst_addr,
                          ucast_l3_p->l4_ports.src_port, ucast_l3_p->l4_ports.dst_port);
            }

            break;
        }

        case SYSPORT_RSB_FLOW_TYPE_UCAST_L2:
        {
            sysport_rsb_flow_ucast_l2_t *ucast_l2_p = &tuple_p->ucast_l2;

            if(tuple_info_p)
            {
                int i;

                bcm_print("ip_tos 0x%02x, ethertype 0x%04x\n"
                          "dst_mac " BLOG_ETH_ADDR_FMT " (0x%08x)\n"
                          "src_mac " BLOG_ETH_ADDR_FMT " (0x%08x)\n"
                          "nbr_of_vlans %d (0x%08x)\n",
                          ucast_l2_p->ip_tos, ucast_l2_p->ethertype,
                          BLOG_ETH_ADDR(tuple_info_p->l2.dst_mac), ucast_l2_p->dst_mac_crc32,
                          BLOG_ETH_ADDR(tuple_info_p->l2.src_mac), ucast_l2_p->src_mac_crc32,
                          tuple_info_p->l2.nbr_of_vlans, ucast_l2_p->vlan_tag_crc32);

                for(i=0; i<tuple_info_p->l2.nbr_of_vlans; ++i)
                {
                    BlogVlanHdr_t vlan;

                    vlan.u16[0] = ntohs(tuple_info_p->l2.vlan[i].u16[0]);
                    vlan.u16[1] = ntohs(tuple_info_p->l2.vlan[i].u16[1]);

                    bcm_print("VLAN[%d] " BLOG_VLAN_HDR_FMT "\n", i, BLOG_VLAN_HDR(vlan));
                }
            }
            else
            {
                bcm_print("ip_tos 0x%02x, ethertype 0x%04x\n"
                          "dst_mac_crc32 0x%08x, src_mac_crc32 0x%08x\n"
                          "vlan_tag_crc32 0x%08x\n",
                          ucast_l2_p->ip_tos, ucast_l2_p->ethertype, ucast_l2_p->dst_mac_crc32,
                          ucast_l2_p->src_mac_crc32, ucast_l2_p->vlan_tag_crc32);
            }

            break;
        }

        case SYSPORT_RSB_FLOW_TYPE_MCAST:
        {
            sysport_rsb_flow_mcast_t *mcast_p = &tuple_p->mcast;

            if(tuple_info_p)
            {
                if(tuple_info_p->l3.is_ipv6)
                {
                    bcm_print("ip_protocol %d\n"
                              "IPv6 src_addr " BLOG_IPV6_ADDR_FMT " (0x%08x)\n"
                              "IPv6 dst_addr " BLOG_IPV6_ADDR_FMT " (0x%08x)\n"
                              "nbr_of_vlans %d, outer_vlan_id %d, inner_vlan_id %d\n",
                              mcast_p->ip_protocol,
                              BLOG_IPV6_ADDR(tuple_info_p->l3.ipv6.src_addr.u8), mcast_p->ip_src_addr,
                              BLOG_IPV6_ADDR(tuple_info_p->l3.ipv6.dst_addr.u8), mcast_p->ip_dst_addr,
                              mcast_p->nbr_of_vlans,
                              mcast_p->vlan.outer_vlan_id, mcast_p->vlan.inner_vlan_id);
                }
                else
                {
                    bcm_print("ip_protocol %d\n"
                              "IPv4 src_addr " BLOG_IPV4_ADDR_FMT "\n"
                              "IPv4 dst_addr " BLOG_IPV4_ADDR_FMT "\n"
                              "nbr_of_vlans %d, outer_vlan_id %d, inner_vlan_id %d\n",
                              mcast_p->ip_protocol,
                              BLOG_IPV4_ADDR_HOST(mcast_p->ip_src_addr),
                              BLOG_IPV4_ADDR_HOST(mcast_p->ip_dst_addr),
                              mcast_p->nbr_of_vlans,
                              mcast_p->vlan.outer_vlan_id, mcast_p->vlan.inner_vlan_id);
                }
            }
            else
            {
                bcm_print("ip_protocol %d\n"
                          "ip_src_addr " BLOG_IPV4_ADDR_FMT " (0x%08x)\n"
                          "ip_dst_addr " BLOG_IPV4_ADDR_FMT " (0x%08x)\n"
                          "nbr_of_vlans %d, outer_vlan_id %d, inner_vlan_id %d\n",
                          mcast_p->ip_protocol, BLOG_IPV4_ADDR_HOST(mcast_p->ip_src_addr),
                          mcast_p->ip_src_addr, BLOG_IPV4_ADDR_HOST(mcast_p->ip_dst_addr),
                          mcast_p->ip_dst_addr, mcast_p->nbr_of_vlans,
                          mcast_p->vlan.outer_vlan_id, mcast_p->vlan.inner_vlan_id);
            }

            break;
        }

        default:
            /* We should never get here */
            bcm_print("Internal Error\n");
    }
}

void sysport_rsb_dump(sysport_rsb_t *rsb_p, sysport_rsb_flow_tuple_info_t *tuple_info_p, int mlt_enable)
{
    sysport_rsb_flow_tuple_t *tuple_p = &rsb_p->tuple;

    bcm_print("\nRSB\n");

    sysport_rsb_mem_dump(rsb_p, sizeof(sysport_rsb_t));

    bcm_print("\nRSB RX_INFO\n");

    if(mlt_enable)
    {
        bcm_print("mlt_index 0x%x, ", rsb_p->mlt_index);
    }
    else
    {
        bcm_print("traffic_class %d, reason_code %d, pkt_type %d, ",
                  rsb_p->traffic_class, rsb_p->reason_code, rsb_p->pkt_type);
    }

    bcm_print("error %d, overflow %d\n"
              "parse_fail %d, l4_checksum %d, sop %d, eop %d, data_length %d\n",
              rsb_p->error, rsb_p->overflow,
              rsb_p->parse_fail, rsb_p->l4_checksum, rsb_p->sop, rsb_p->eop, rsb_p->data_length);

    bcm_print("\nRSB PARSING\n");

    bcm_print("tunnel_ip_src_addr " BLOG_IPV4_ADDR_FMT " (0x%08x)\n",
              BLOG_IPV4_ADDR_HOST(rsb_p->tunnel_ip_src_addr), rsb_p->tunnel_ip_src_addr);

    bcm_print("ip_offset %d, ip_tos 0x%02x, ip_length %d\n",
              rsb_p->ip_offset, rsb_p->ip_tos, rsb_p->ip_length);

    bcm_print("nbr_of_vlans %d, pppoe %d, llc_snap %d, l3_type %s (%d)\n"
              "tcp %d, udp %d, ip_version_mismatch %d, ipv4_options %d, ttl_expired %d\n"
              "ipv4_df %d, ipv4_frag %d, tcp_rst_syn_fin %d, tuple_crc16 0x%04x\n",
              rsb_p->nbr_of_vlans, rsb_p->pppoe, rsb_p->llc_snap, __rsb_l3_type_name(rsb_p->l3_type),
              rsb_p->l3_type, rsb_p->tcp, rsb_p->udp, rsb_p->ip_version_mismatch,
              rsb_p->ipv4_options, rsb_p->ttl_expired, rsb_p->ipv4_df, rsb_p->ipv4_frag,
              rsb_p->tcp_rst_syn_fin, rsb_p->tuple_crc16);

    bcm_print("\nRSB TUPLE\n");

    sysport_rsb_tuple_dump(tuple_p, tuple_info_p);
}
