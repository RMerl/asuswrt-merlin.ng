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
 :>
*/

#ifdef RDP_SIM /* Don't include this file if not RDP_SIM */
 
#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_int.h"
#include <rdd.h>
#include "rdpa_rdd_inline.h"
#include "rdpa_blog.h"
#include "pktrunner_rdpa_sim.h"

#ifdef __KERNEL__
  #error "This file is not intended to be compiled into Kernel space.  It is for UT simulation only\n"
#endif

#define LOGICAL_PORT_MAX_VALUE 31

/* ############################################################### */
/* Code called from pktrunner to retrieve info                     */
/* ############################################################### */

uint16_t rdpa_blog_mtu_list[LOGICAL_PORT_MAX_VALUE+1] = {0};
uint32_t rdpa_blog_log_port_wan_flags = 0;

uint16_t blog_getTxMtu(Blog_t *blog_p)
{
    return rdpa_blog_mtu_list[blog_p->tx.info.channel];
}

int rdpa_blog_is_wan_port(uint32_t logical_port)
{
    if (logical_port > LOGICAL_PORT_MAX_VALUE)
        return 0;
    return (rdpa_blog_log_port_wan_flags & (1 << logical_port)) != 0;
}

const uint8_t rfc2684HdrLength[RFC2684_MAX] =
{
     0, /* header was already stripped. :                               */
    10, /* LLC_SNAP_ETHERNET            : AA AA 03 00 80 C2 00 07 00 00 */
     8, /* LLC_SNAP_ROUTE_IP            : AA AA 03 00 00 00 08 00       */
     4, /* LLC_ENCAPS_PPP               : FE FE 03 CF                   */
     2, /* VC_MUX_ETHERNET              : 00 00                         */
     0, /* VC_MUX_IPOA                  :                               */
     0, /* VC_MUX_PPPOA                 :                               */
     0, /* PTM                          :                               */
};


/* ############################################################### */
/* PktRunner and Blog Support Code For Adding a Flow               */
/* ############################################################### */

int fhwPktRunnerActivate(Blog_t *blog_p, uint32_t key_in);


/* ############################################################### */
/* Logical Port support types, functions and data                  */
/* ############################################################### */

static int rdpa_blog_logical_port_add(struct bdmf_object *mo,
                                      struct bdmf_attr *ad,
                                      bdmf_index *index,
                                      const void *val,
                                      uint32_t size)
{
    rdpa_blog_logical_port_t *port_info = (rdpa_blog_logical_port_t *)val;
    bdmf_index portidx = *index;

    if (port_info->tx_mtu > 0)
    {
        rdpa_blog_mtu_list[portidx] = port_info->tx_mtu;
        if (port_info->is_wan_port)
            rdpa_blog_log_port_wan_flags |= (1 << portidx);
        else
            rdpa_blog_log_port_wan_flags &= ~(1 << portidx);
    }
    else
    {
        return BDMF_ERR_ALREADY;
    }
    
    return BDMF_ERR_OK;
}

static int rdpa_blog_logical_port_delete(struct bdmf_object *mo,
                                         struct bdmf_attr *ad,
                                         bdmf_index index)
{
    if (rdpa_blog_mtu_list[index] > 0)
    {
        rdpa_blog_mtu_list[index] = 0;
        rdpa_blog_log_port_wan_flags &= ~(1 << index);
    }
    else
    {
        return BDMF_ERR_NOENT;
    }
    
    return BDMF_ERR_OK;
}


static int rdpa_blog_logical_port_read(struct bdmf_object *mo,
                                       struct bdmf_attr *ad,
                                       bdmf_index index,
                                       void *val,
                                       uint32_t size)
{
    rdpa_blog_logical_port_t *port_info = (rdpa_blog_logical_port_t *)val;
    
    if (rdpa_blog_mtu_list[index] > 0)
    {
        port_info->tx_mtu = rdpa_blog_mtu_list[index];
        port_info->is_wan_port = (rdpa_blog_log_port_wan_flags & (1 << index)) != 0;
    }
    else
    {
        return BDMF_ERR_NOENT;
    }
    
    return BDMF_ERR_OK;
}



/* Blog data type support types, functions and data */

/* If addr is ipv6 then copy address to dst.
   otherwise set dst to zeros. */
static void v6_address_from_bdmf(uint8_t *dst, bdmf_ip_t *addr)
{
    if (addr->family == bdmf_ip_family_ipv6)
        memcpy(dst, &(addr->addr.ipv6.data[0]), 16);
    else
        memset(dst, 0, 16);
}


/* If addr is ipv4 then copy address to dst.
   otherwise set dst to zeros. */
static void v4_address_from_bdmf(uint32_t *dst, bdmf_ip_t *addr)
{
    if (addr->family == bdmf_ip_family_ipv4)
        *dst = htonl(addr->addr.ipv4);
    else
        *dst = 0;
}


/* If addr is ipv6 then copy address to dst.
   otherwise set dst to zeros. */
static void v6_bdmf_from_address(bdmf_ip_t *addr, uint8_t *src)
{
    addr->family = bdmf_ip_family_ipv6;
    memcpy(&(addr->addr.ipv6.data[0]), src, 16);
}


/* If addr is ipv4 then copy address to dst.
   otherwise set dst to zeros. */
static void v4_bdmf_from_address(bdmf_ip_t *addr, uint32_t *src)
{
    addr->family = bdmf_ip_family_ipv4;
    addr->addr.ipv4 = *src;
}


/* ############################################################### */
/* BDMF Blog Object Private Data & Structures                      */
/* ############################################################### */

static struct bdmf_object *blog_object;

typedef struct blog_list_s blog_list_t; 
typedef struct blog_rule_list_s blog_rule_list_t;

/* blog object private data */
typedef struct {
    blog_list_t *blog_list_head;
    blog_rule_list_t *blog_rule_list_head;
} blog_drv_priv_t;


/* ############################################################### */
/* Sparse List Support for Blog List                               */
/* ############################################################### */

struct blog_list_s {
    Blog_t *blog_p;
    blog_list_t *next;
    uint32_t index;
    int flow_idx;
};

static blog_list_t **find_ptr_to_blog_entry_by_index(blog_drv_priv_t *context, uint32_t index)
{
    blog_list_t **p = &(context->blog_list_head);
    while (*p)
    {
        if ((*p)->index == index) 
            return p;

        p = &((*p)->next);
    }
    return NULL;
}


/* ############################################################### */
/* Sparse List Support for Blog Rules                              */
/* ############################################################### */


struct blog_rule_list_s {
    blogRule_t       *blog_rule_p;
    blog_rule_list_t *next;
    uint32_t          index;
};

static blog_rule_list_t **find_ptr_to_blog_rule_entry_by_index(blog_drv_priv_t *context, uint32_t index)
{
    blog_rule_list_t **p = &(context->blog_rule_list_head);
    while (*p)
    {
        if ((*p)->index == index) 
            return p;

        p = &((*p)->next);
    }
    return NULL;
}

static blog_rule_list_t **find_ptr_to_blog_rule_entry_by_rule_p(blog_drv_priv_t *context, blogRule_t * blog_rule_p)
{
    blog_rule_list_t **p = &(context->blog_rule_list_head);
    while (*p)
    {
        if ((*p)->blog_rule_p == blog_rule_p) 
            return p;

        p = &((*p)->next);
    }
    return NULL;
}


/* ############################################################### */
/* Blog Attribute                                                  */
/* ############################################################### */

static int blog_attr_entry_add(struct bdmf_object *mo,
                               struct bdmf_attr *ad,
                               bdmf_index *index,
                               const void *val,
                               uint32_t size)
{
    blog_drv_priv_t *context = (blog_drv_priv_t *)bdmf_obj_data(blog_object);
    rdpa_blog_entry_t *blog_val = (rdpa_blog_entry_t *)val;
    Blog_t *blog_p;
    uint32_t myIndex = (uint32_t) *index;
    blog_list_t *blog_list_p;
    int flow_idx;
    int rval = BDMF_ERR_OK;

    /* if the index already exists then don't continue */
    if (find_ptr_to_blog_entry_by_index(context, myIndex))
    {
        return BDMF_ERR_ALREADY;
    }

    /* allocate necessary structures */
    blog_p = (Blog_t *)calloc(1, sizeof(Blog_t));
    if (!blog_p) 
        return BDMF_ERR_NORES;

    blog_list_p = (blog_list_t *)calloc(1, sizeof(blog_list_t));
    if (!blog_list_p)
    {
        free(blog_p);
        return BDMF_ERR_NORES;
    }

    /* Copy all of the blog data: */
    blog_p->key.protocol    = blog_val->rx.tuple.header.prot;
    blog_p->wfd.u32         = blog_val->wfd_rnr;
    blog_p->vtag_num        = blog_val->vtag_num;
    blog_p->vtag[0]         = swap4bytes(blog_val->vtag0);
    blog_p->vtag[1]         = swap4bytes(blog_val->vtag1);
    blog_p->eth_type        = blog_val->eth_type;
    blog_p->lag_port        = blog_val->lag_port;
    blog_p->ack_cnt         = blog_val->ack_cnt;
    blog_p->mark            = blog_val->mark;
    blog_p->priority        = blog_val->priority;
    blog_p->mcast_learn     = blog_val->mcast_learn;
    blog_p->priority        = blog_val->priority;

    if (blog_val->blog_rule_count)
    {
        blog_rule_list_t **blog_rule_list_pp;
        blog_rule_list_pp = find_ptr_to_blog_rule_entry_by_index(context, blog_val->blog_rule_index);

        if (blog_rule_list_pp)
        {
            blog_p->blogRule_p  = (*blog_rule_list_pp)->blog_rule_p;
        }
        else
        {
            rval = BDMF_ERR_STATE;
        }
    }
    else
    {
        blog_p->blogRule_p  = 0;
    }

    blog_p->ptm_us_bond     = (blog_val->ptm_us_bond != 0);
    blog_p->tos_mode_us     = (blog_val->tos_mode_us != 0);
    blog_p->tos_mode_ds     = (blog_val->tos_mode_ds != 0);
    blog_p->has_pppoe       = (blog_val->has_pppoe != 0);
    blog_p->ack_done        = (blog_val->ack_done != 0);
    blog_p->nf_dir_pld      = (blog_val->nf_dir_pld != 0);
    blog_p->nf_dir_del      = (blog_val->nf_dir_del != 0);
    blog_p->pop_pppoa       = (blog_val->pop_pppoa != 0);
    blog_p->insert_eth      = (blog_val->insert_eth != 0);
    blog_p->iq_prio         = (blog_val->iq_prio != 0);
    blog_p->mc_sync         = (blog_val->mc_sync != 0);
    blog_p->rtp_seq_chk     = (blog_val->rtp_seq_chk != 0);
    blog_p->incomplete      = (blog_val->incomplete != 0);

    v6_address_from_bdmf(&(blog_p->tupleV6.saddr.p8[0]), &(blog_val->tupleV6.header.src_ip));
    v6_address_from_bdmf(&(blog_p->tupleV6.daddr.p8[0]), &(blog_val->tupleV6.header.dst_ip));

    blog_p->tupleV6.port.source   = htons(blog_val->tupleV6.header.src_port);
    blog_p->tupleV6.port.dest     = htons(blog_val->tupleV6.header.dst_port);
    blog_p->tupleV6.next_hdr      = blog_val->tupleV6.header.prot;
    blog_p->tupleV6.rx_hop_limit  = blog_val->tupleV6.header.ttl;

    blog_p->tupleV6.word0         = htonl(blog_val->tupleV6.word0);
    blog_p->tupleV6.length        = blog_val->tupleV6.length;
    blog_p->tupleV6.exthdrs       = blog_val->tupleV6.exthdrs;
    blog_p->tupleV6.fragflag      = (blog_val->tupleV6.fragflag != 0);
    blog_p->tupleV6.tunnel        = (blog_val->tupleV6.tunnel != 0);
    blog_p->tupleV6.tx_hop_limit  = blog_val->tupleV6.tx_hop_limit;
    blog_p->tupleV6.ipid          = blog_val->tupleV6.ipid;


    v4_address_from_bdmf(&(blog_p->rx.tuple.saddr), &(blog_val->rx.tuple.header.src_ip));
    v4_address_from_bdmf(&(blog_p->rx.tuple.daddr), &(blog_val->rx.tuple.header.dst_ip));
    blog_p->rx.tuple.port.source  = htons(blog_val->rx.tuple.header.src_port);
    blog_p->rx.tuple.port.dest    = htons(blog_val->rx.tuple.header.dst_port);
    blog_p->rx.tuple.ttl          = blog_val->rx.tuple.header.ttl;
    blog_p->rx.tuple.tos          = blog_val->rx.tuple.tos;
    blog_p->rx.tuple.check        = htons(blog_val->rx.tuple.check);
    blog_p->rx.info.channel       = blog_val->rx.info.channel;
    blog_p->rx.info.phyHdrLen     = blog_val->rx.info.phy_hdr_len;
    blog_p->rx.info.phyHdrType    = blog_val->rx.info.phy_hdr_type;
    blog_p->rx.info.bmap.GRE_ETH       = (blog_val->rx.info.GRE_ETH != 0);
    blog_p->rx.info.bmap.BCM_XPHY      = (blog_val->rx.info.BCM_XPHY != 0);
    blog_p->rx.info.bmap.BCM_SWC       = (blog_val->rx.info.BCM_SWC != 0);
    blog_p->rx.info.bmap.ETH_802x      = (blog_val->rx.info.ETH_802x != 0);
    blog_p->rx.info.bmap.VLAN_8021Q    = (blog_val->rx.info.VLAN_8021Q != 0);
    blog_p->rx.info.bmap.PPPoE_2516    = (blog_val->rx.info.PPPoE_2516 != 0);
    blog_p->rx.info.bmap.PPP_1661      = (blog_val->rx.info.PPP_1661 != 0);
    blog_p->rx.info.bmap.PLD_IPv4      = (blog_val->rx.info.PLD_IPv4 != 0);
    blog_p->rx.info.bmap.PLD_IPv6      = (blog_val->rx.info.PLD_IPv6 != 0);
    blog_p->rx.info.bmap.PPTP          = (blog_val->rx.info.PPTP != 0);
    blog_p->rx.info.bmap.L2TP          = (blog_val->rx.info.L2TP != 0);
    blog_p->rx.info.bmap.GRE           = (blog_val->rx.info.GRE != 0);
    blog_p->rx.info.bmap.ESP           = (blog_val->rx.info.ESP != 0);
    blog_p->rx.info.bmap.DEL_IPv4      = (blog_val->rx.info.DEL_IPv4 != 0);
    blog_p->rx.info.bmap.DEL_IPv6      = (blog_val->rx.info.DEL_IPv6 != 0);
    blog_p->rx.info.bmap.PLD_L2        = (blog_val->rx.info.PLD_L2 != 0);
    blog_p->rx.multicast          = (blog_val->rx.multicast != 0);
    blog_p->rx.count              = blog_val->rx.count;
    blog_p->rx.length             = blog_val->rx.length;
    memcpy(&(blog_p->rx.encap[0]), &(blog_val->rx.encap[0]), (BLOG_ENCAP_MAX * sizeof(uint32_t)));
    memcpy(&(blog_p->rx.l2hdr[0]), &(blog_val->rx.l2hdr[0]), (BLOG_HDRSZ_MAX * sizeof(uint8_t)));

    v4_address_from_bdmf(&(blog_p->tx.tuple.saddr), &(blog_val->tx.tuple.header.src_ip));
    v4_address_from_bdmf(&(blog_p->tx.tuple.daddr), &(blog_val->tx.tuple.header.dst_ip));
    blog_p->tx.tuple.port.source  = htons(blog_val->tx.tuple.header.src_port);
    blog_p->tx.tuple.port.dest    = htons(blog_val->tx.tuple.header.dst_port);
    blog_p->tx.tuple.ttl          = blog_val->tx.tuple.header.ttl;
    blog_p->tx.tuple.tos          = blog_val->tx.tuple.tos;
    blog_p->tx.tuple.check        = htons(blog_val->tx.tuple.check);
    blog_p->tx.info.channel       = blog_val->tx.info.channel;
    blog_p->tx.info.phyHdrLen     = blog_val->tx.info.phy_hdr_len;
    blog_p->tx.info.phyHdrType    = blog_val->tx.info.phy_hdr_type;
    blog_p->tx.info.bmap.GRE_ETH       = (blog_val->tx.info.GRE_ETH != 0);
    blog_p->tx.info.bmap.BCM_XPHY      = (blog_val->tx.info.BCM_XPHY != 0);
    blog_p->tx.info.bmap.BCM_SWC       = (blog_val->tx.info.BCM_SWC != 0);
    blog_p->tx.info.bmap.ETH_802x      = (blog_val->tx.info.ETH_802x != 0);
    blog_p->tx.info.bmap.VLAN_8021Q    = (blog_val->tx.info.VLAN_8021Q != 0);
    blog_p->tx.info.bmap.PPPoE_2516    = (blog_val->tx.info.PPPoE_2516 != 0);
    blog_p->tx.info.bmap.PPP_1661      = (blog_val->tx.info.PPP_1661 != 0);
    blog_p->tx.info.bmap.PLD_IPv4      = (blog_val->tx.info.PLD_IPv4 != 0);
    blog_p->tx.info.bmap.PLD_IPv6      = (blog_val->tx.info.PLD_IPv6 != 0);
    blog_p->tx.info.bmap.PPTP          = (blog_val->tx.info.PPTP != 0);
    blog_p->tx.info.bmap.L2TP          = (blog_val->tx.info.L2TP != 0);
    blog_p->tx.info.bmap.GRE           = (blog_val->tx.info.GRE != 0);
    blog_p->tx.info.bmap.ESP           = (blog_val->tx.info.ESP != 0);
    blog_p->tx.info.bmap.DEL_IPv4      = (blog_val->tx.info.DEL_IPv4 != 0);
    blog_p->tx.info.bmap.DEL_IPv6      = (blog_val->tx.info.DEL_IPv6 != 0);
    blog_p->tx.info.bmap.PLD_L2        = (blog_val->tx.info.PLD_L2 != 0);
    blog_p->tx.multicast          = (blog_val->tx.multicast != 0);
    blog_p->tx.count              = blog_val->tx.count;
    blog_p->tx.length             = blog_val->tx.length;
    memcpy(&(blog_p->tx.encap[0]), &(blog_val->tx.encap[0]), (BLOG_ENCAP_MAX * sizeof(uint32_t)));
    memcpy(&(blog_p->tx.l2hdr[0]), &(blog_val->tx.l2hdr[0]), (BLOG_HDRSZ_MAX * sizeof(uint8_t)));

    blog_p->is_ssm = 0;
    if (ntohs(blog_p->eth_type) == 0x86DD) /* IPv6? */
    {
        if (!( ( blog_p->tupleV6.saddr.p32[0] == 0 ) &&
               ( blog_p->tupleV6.saddr.p32[1] == 0 ) &&
               ( blog_p->tupleV6.saddr.p32[2] == 0 ) &&
               ( blog_p->tupleV6.saddr.p32[3] == 0 ) ))
        {
            blog_p->is_ssm = 1;
        }
    }
    else
    {
        if (blog_p->rx.tuple.saddr != 0) 
        {
            blog_p->is_ssm = 1;
        }
    }
    
    blog_p->rx.dev_p   = NULL;
    blog_p->tx.dev_p   = NULL;

    if (!rval)
    {
        /* Activate the flow */
        if (context->blog_list_head && context->blog_list_head->blog_p->rx.multicast)
        {
            /* blog_list_head is multicast, use its flow index */
            flow_idx = fhwPktRunnerActivate(blog_p, context->blog_list_head->flow_idx);
        }
        else
        {
            /* blog_list_head is not multicast */
            flow_idx = fhwPktRunnerActivate(blog_p, FHW_TUPLE_INVALID);
        }

        if (flow_idx == FHW_TUPLE_INVALID)
        {
            free(blog_list_p);
            free(blog_p);
            return BDMF_ERR_STATE;
        }

        /* Entry was activated, let's add it to our tracking list: */
        blog_list_p->index      = myIndex;
        blog_list_p->flow_idx   = flow_idx;
        blog_list_p->blog_p     = blog_p;
        blog_list_p->next       = context->blog_list_head;
        context->blog_list_head = blog_list_p;
    }
    
    return rval;
}

static int blog_attr_entry_delete(struct bdmf_object *mo,
                                  struct bdmf_attr *ad,
                                  bdmf_index index)
{
    return BDMF_ERR_OK;
}

static int blog_attr_entry_read(struct bdmf_object *mo,
                                struct bdmf_attr *ad,
                                bdmf_index index,
                                void *val,
                                uint32_t size)
{
    blog_drv_priv_t *context = (blog_drv_priv_t *)bdmf_obj_data(blog_object);
    rdpa_blog_entry_t *blog_val = (rdpa_blog_entry_t *)val;
    Blog_t *blog_p;
    uint32_t myIndex = (uint32_t) index;
    blog_list_t **blog_list_pp;

    /* if the index doesn't exist then don't continue */
    blog_list_pp = find_ptr_to_blog_entry_by_index(context, myIndex);

    if (!blog_list_pp)
    {
        return BDMF_ERR_NOENT;
    }

    /* allocate necessary structures */
    blog_p = (Blog_t *)(*blog_list_pp)->blog_p;
    if (!blog_p) 
        return BDMF_ERR_NOENT;

    /* Copy all of the blog data: */
    blog_val->rx.tuple.header.prot      = blog_p->key.protocol;
    blog_val->wfd_rnr                   = blog_p->wfd.u32;
    blog_val->vtag_num                  = blog_p->vtag_num;
    blog_val->vtag0                     = swap4bytes(blog_p->vtag[0]);
    blog_val->vtag1                     = swap4bytes(blog_p->vtag[1]);
    blog_val->eth_type                  = blog_p->eth_type;
    blog_val->lag_port                  = blog_p->lag_port;
    blog_val->ack_cnt                   = blog_p->ack_cnt;
    blog_val->mark                      = blog_p->mark;
    blog_val->priority                  = blog_p->priority;
    blog_val->mcast_learn               = blog_p->mcast_learn;
    blog_val->priority                  = blog_p->priority;

    blog_val->blog_rule_index = 0;
    blog_val->blog_rule_count = 0;
    if (blog_p->blogRule_p)
    {
        blog_rule_list_t **blog_rule_list_pp;
        blog_rule_list_pp = find_ptr_to_blog_rule_entry_by_rule_p(context, blog_p->blogRule_p);
        if (blog_rule_list_pp)
        {
            blog_val->blog_rule_index = (*blog_rule_list_pp)->index;
            blog_val->blog_rule_count = 1;
        }
    }

    blog_val->ptm_us_bond               = blog_p->ptm_us_bond;
    blog_val->tos_mode_us               = blog_p->tos_mode_us;
    blog_val->tos_mode_ds               = blog_p->tos_mode_ds;
    blog_val->has_pppoe                 = blog_p->has_pppoe;
    blog_val->ack_done                  = blog_p->ack_done;
    blog_val->nf_dir_pld                = blog_p->nf_dir_pld;
    blog_val->nf_dir_del                = blog_p->nf_dir_del;
    blog_val->pop_pppoa                 = blog_p->pop_pppoa;
    blog_val->insert_eth                = blog_p->insert_eth;
    blog_val->iq_prio                   = blog_p->iq_prio;
    blog_val->mc_sync                   = blog_p->mc_sync;
    blog_val->rtp_seq_chk               = blog_p->rtp_seq_chk;
    blog_val->incomplete                = blog_p->incomplete;

    v6_bdmf_from_address(&(blog_val->tupleV6.header.src_ip), &(blog_p->tupleV6.saddr.p8[0]));
    v6_bdmf_from_address(&(blog_val->tupleV6.header.dst_ip), &(blog_p->tupleV6.daddr.p8[0]));

    blog_val->tupleV6.header.src_port   = blog_p->tupleV6.port.source;
    blog_val->tupleV6.header.dst_port   = blog_p->tupleV6.port.dest;
    blog_val->tupleV6.header.prot       = blog_p->tupleV6.next_hdr;
    blog_val->tupleV6.header.ttl        = blog_p->tupleV6.rx_hop_limit;

    blog_val->tupleV6.word0             = blog_p->tupleV6.word0;
    blog_val->tupleV6.length            = blog_p->tupleV6.length;
    blog_val->tupleV6.exthdrs           = blog_p->tupleV6.exthdrs;
    blog_val->tupleV6.fragflag          = blog_p->tupleV6.fragflag;
    blog_val->tupleV6.tunnel            = blog_p->tupleV6.tunnel;
    blog_val->tupleV6.tx_hop_limit      = blog_p->tupleV6.tx_hop_limit;
    blog_val->tupleV6.ipid              = blog_p->tupleV6.ipid;


    v4_bdmf_from_address(&(blog_val->rx.tuple.header.src_ip), &(blog_p->rx.tuple.saddr));
    v4_bdmf_from_address(&(blog_val->rx.tuple.header.dst_ip), &(blog_p->rx.tuple.daddr));

    blog_val->rx.tuple.header.src_port = blog_p->rx.tuple.port.source;
    blog_val->rx.tuple.header.dst_port = blog_p->rx.tuple.port.dest;
    blog_val->rx.tuple.header.ttl      = blog_p->rx.tuple.ttl;
    blog_val->rx.tuple.tos             = blog_p->rx.tuple.tos;
    blog_val->rx.tuple.check           = blog_p->rx.tuple.check;
    blog_val->rx.info.channel          = blog_p->rx.info.channel;
    blog_val->rx.info.phy_hdr_len      = blog_p->rx.info.phyHdrLen;
    blog_val->rx.info.phy_hdr_type     = blog_p->rx.info.phyHdrType;

    blog_val->rx.info.GRE_ETH        = blog_p->rx.info.bmap.GRE_ETH;
    blog_val->rx.info.BCM_XPHY       = blog_p->rx.info.bmap.BCM_XPHY;
    blog_val->rx.info.BCM_SWC        = blog_p->rx.info.bmap.BCM_SWC;
    blog_val->rx.info.ETH_802x       = blog_p->rx.info.bmap.ETH_802x;
    blog_val->rx.info.VLAN_8021Q     = blog_p->rx.info.bmap.VLAN_8021Q;
    blog_val->rx.info.PPPoE_2516     = blog_p->rx.info.bmap.PPPoE_2516;
    blog_val->rx.info.PPP_1661       = blog_p->rx.info.bmap.PPP_1661;
    blog_val->rx.info.PLD_IPv4       = blog_p->rx.info.bmap.PLD_IPv4;
    blog_val->rx.info.PLD_IPv6       = blog_p->rx.info.bmap.PLD_IPv6;
    blog_val->rx.info.PPTP           = blog_p->rx.info.bmap.PPTP;
    blog_val->rx.info.L2TP           = blog_p->rx.info.bmap.L2TP;
    blog_val->rx.info.GRE            = blog_p->rx.info.bmap.GRE;
    blog_val->rx.info.ESP            = blog_p->rx.info.bmap.ESP;
    blog_val->rx.info.DEL_IPv4       = blog_p->rx.info.bmap.DEL_IPv4;
    blog_val->rx.info.DEL_IPv6       = blog_p->rx.info.bmap.DEL_IPv6;
    blog_val->rx.info.PLD_L2         = blog_p->rx.info.bmap.PLD_L2;
    blog_val->rx.multicast           = blog_p->rx.multicast;
    blog_val->rx.count                 = blog_p->rx.count;
    blog_val->rx.length                = blog_p->rx.length;
    memcpy(&(blog_val->rx.encap[0]), &(blog_p->rx.encap[0]), (BLOG_ENCAP_MAX * sizeof(uint32_t)));
    memcpy(&(blog_val->rx.l2hdr[0]), &(blog_p->rx.l2hdr[0]), (BLOG_HDRSZ_MAX * sizeof(uint8_t)));


    v4_bdmf_from_address(&(blog_val->tx.tuple.header.src_ip), &(blog_p->tx.tuple.saddr));
    v4_bdmf_from_address(&(blog_val->tx.tuple.header.dst_ip), &(blog_p->tx.tuple.daddr));
    blog_val->tx.tuple.header.src_port = blog_p->tx.tuple.port.source;
    blog_val->tx.tuple.header.dst_port = blog_p->tx.tuple.port.dest;
    blog_val->tx.tuple.header.ttl      = blog_p->tx.tuple.ttl;
    blog_val->tx.tuple.tos             = blog_p->tx.tuple.tos;
    blog_val->tx.tuple.check           = blog_p->tx.tuple.check;
    blog_val->tx.info.channel          = blog_p->tx.info.channel;
    blog_val->tx.info.phy_hdr_len      = blog_p->tx.info.phyHdrLen;
    blog_val->tx.info.phy_hdr_type     = blog_p->tx.info.phyHdrType;
    blog_val->tx.info.GRE_ETH          = blog_p->tx.info.bmap.GRE_ETH;
    blog_val->tx.info.BCM_XPHY         = blog_p->tx.info.bmap.BCM_XPHY;
    blog_val->tx.info.BCM_SWC          = blog_p->tx.info.bmap.BCM_SWC;
    blog_val->tx.info.ETH_802x         = blog_p->tx.info.bmap.ETH_802x;
    blog_val->tx.info.VLAN_8021Q       = blog_p->tx.info.bmap.VLAN_8021Q;
    blog_val->tx.info.PPPoE_2516       = blog_p->tx.info.bmap.PPPoE_2516;
    blog_val->tx.info.PPP_1661         = blog_p->tx.info.bmap.PPP_1661;
    blog_val->tx.info.PLD_IPv4         = blog_p->tx.info.bmap.PLD_IPv4;
    blog_val->tx.info.PLD_IPv6         = blog_p->tx.info.bmap.PLD_IPv6;
    blog_val->tx.info.PPTP             = blog_p->tx.info.bmap.PPTP;
    blog_val->tx.info.L2TP             = blog_p->tx.info.bmap.L2TP;
    blog_val->tx.info.GRE              = blog_p->tx.info.bmap.GRE;
    blog_val->tx.info.ESP              = blog_p->tx.info.bmap.ESP;
    blog_val->tx.info.DEL_IPv4         = blog_p->tx.info.bmap.DEL_IPv4;
    blog_val->tx.info.DEL_IPv6         = blog_p->tx.info.bmap.DEL_IPv6;
    blog_val->tx.info.PLD_L2           = blog_p->tx.info.bmap.PLD_L2;
    blog_val->tx.multicast             = blog_p->tx.multicast;
    blog_val->tx.count                 = blog_p->tx.count;
    blog_val->tx.length                = blog_p->tx.length;
    memcpy(&(blog_val->tx.encap[0]), &(blog_p->tx.encap[0]), (BLOG_ENCAP_MAX * sizeof(uint32_t)));
    memcpy(&(blog_val->tx.l2hdr[0]), &(blog_p->tx.l2hdr[0]), (BLOG_HDRSZ_MAX * sizeof(uint8_t)));

    return BDMF_ERR_OK;
}


/* ############################################################### */
/* Blog Rule Attribute                                             */
/* ############################################################### */

static int blog_rule_attr_entry_add(struct bdmf_object *mo,
                               struct bdmf_attr *ad,
                               bdmf_index *index,
                               const void *val,
                               uint32_t size)
{
    blog_drv_priv_t *context = (blog_drv_priv_t *)bdmf_obj_data(blog_object);
    rdpa_blog_rule_t *blog_rule_val = (rdpa_blog_rule_t *)val;
    blogRule_t *blog_rule_p;
    uint32_t myIndex = (uint32_t) *index;
    blog_rule_list_t *blog_rule_list_p;

    /* if the index already exists then don't continue */
    if (find_ptr_to_blog_rule_entry_by_index(context, myIndex))
    {
        return BDMF_ERR_ALREADY;
    }

    /* allocate necessary structures */
    blog_rule_p = (blogRule_t *)calloc(1, sizeof(blogRule_t));
    if (!blog_rule_p) 
        return BDMF_ERR_NORES;

    blog_rule_list_p = (blog_rule_list_t *)calloc(1, sizeof(blog_rule_list_t));
    if (!blog_rule_list_p)
    {
        free(blog_rule_p);
        return BDMF_ERR_NORES;
    }

    /* Copy all of the blog rule data: */
    blog_rule_p->filter.nbrOfVlanTags        = blog_rule_val->filter.nbr_of_vlan_tags;
    blog_rule_p->filter.hasPppoeHeader       = blog_rule_val->filter.has_pppoe_header;
    blog_rule_p->filter.ipv4.value.tos       = blog_rule_val->filter.ipv4_tos_value;
    blog_rule_p->filter.ipv4.mask.tos        = blog_rule_val->filter.ipv4_tos_mask;
    blog_rule_p->filter.ipv4.value.ip_proto  = blog_rule_val->filter.ipv4_ip_proto_value;
    blog_rule_p->filter.ipv4.mask.ip_proto   = blog_rule_val->filter.ipv4_ip_proto_mask;
    blog_rule_p->filter.ipv6.value.tclass    = blog_rule_val->filter.ipv6_tclass_value;
    blog_rule_p->filter.ipv6.mask.tclass     = blog_rule_val->filter.ipv6_tclass_mask;
    blog_rule_p->filter.ipv6.value.nxtHdr    = blog_rule_val->filter.ipv6_next_hdr_value;
    blog_rule_p->filter.ipv6.mask.nxtHdr     = blog_rule_val->filter.ipv6_next_hdr_mask;
    blog_rule_p->filter.skb.priority         = blog_rule_val->filter.skb_priority;
    blog_rule_p->filter.skb.markFlowId       = blog_rule_val->filter.skb_mark_flow_id;
    blog_rule_p->filter.skb.markPort         = blog_rule_val->filter.skb_mark_port;
    blog_rule_p->filter.flags                = blog_rule_val->filter.flags;
    
    blog_rule_p->actionCount      = blog_rule_val->action_count;
    
    /* Blog Rule Action is a special case.  Normally it is an array of structures,
       but because rdpa can not handle embedded arrays, it is specified as a buffer.
       As such we need to 'intellegently' extract the fields from the buffer.
       
       The buffer length of each command is variable depending on the command
       given in the 'cmd' field.  The next command follows immediately in the next
       byte.
       
       The structure is given in uint8_t bytes where the index 'n' is the index
       of the current 'cmd' byte.

       buffer[n+0] = blogRuleAction_t.cmd
       buffer[n+1] = blogRuleAction_t.toTag

       The next 6 bytes of buffer are dependend on the value of 'cmd':

           if cmd == BLOG_RULE_CMD_NOP
           or cmd == BLOG_RULE_CMD_POP_PPPOE_HDR
           or cmd == BLOG_RULE_CMD_DECR_TTL

               No parameters

           if cmd == BLOG_RULE_CMD_SET_MAC_DA
           or cmd == BLOG_RULE_CMD_SET_MAC_SA
           or cmd == BLOG_RULE_CMD_SET_STA_MAC_ADDRESS

               buffer[n+2] == MSB of MAC address
               buffer[...]
               buffer[n+7] == LSB of MAC address

           if cmd == BLOG_RULE_CMD_SET_VID
           
               buffer[n+2] == MSB of VID
               buffer[n+3] == LSB of VID
               
           ALL Other 'cmd' values are not supported.
    */
       
    {
        int n = 0;
        int count = 0;
        
        while (count < blog_rule_val->action_count)
        {
            blog_rule_p->action[count].cmd   = blog_rule_val->action[n];
            blog_rule_p->action[count].toTag = blog_rule_val->action[n+1];
            switch(blog_rule_p->action[count].cmd)
            {
                case BLOG_RULE_CMD_NOP:
                case BLOG_RULE_CMD_DROP:
                case BLOG_RULE_CMD_DECR_HOP_LIMIT:
                case BLOG_RULE_CMD_PUSH_VLAN_HDR:
                case BLOG_RULE_CMD_POP_VLAN_HDR:
                case BLOG_RULE_CMD_POP_PPPOE_HDR:
                case BLOG_RULE_CMD_DECR_TTL:
                    n += 2;
                    break;

                case BLOG_RULE_CMD_SET_MAC_DA:
                case BLOG_RULE_CMD_SET_MAC_SA:
                case BLOG_RULE_CMD_SET_STA_MAC_ADDRESS:
                    blog_rule_p->action[count].macAddr[0] = blog_rule_val->action[n+2];
                    blog_rule_p->action[count].macAddr[1] = blog_rule_val->action[n+3];
                    blog_rule_p->action[count].macAddr[2] = blog_rule_val->action[n+4];
                    blog_rule_p->action[count].macAddr[3] = blog_rule_val->action[n+5];
                    blog_rule_p->action[count].macAddr[4] = blog_rule_val->action[n+6];
                    blog_rule_p->action[count].macAddr[5] = blog_rule_val->action[n+7];
                    n += 8;
                    break;
                
                case BLOG_RULE_CMD_SET_VID:
                case BLOG_RULE_CMD_COPY_VID:
                case BLOG_RULE_CMD_SET_ETHERTYPE:
                case BLOG_RULE_CMD_SET_PBITS:
                case BLOG_RULE_CMD_SET_DEI:
                case BLOG_RULE_CMD_SET_VLAN_PROTO:
                case BLOG_RULE_CMD_SET_DSCP:
                case BLOG_RULE_CMD_COPY_PBITS:
                case BLOG_RULE_CMD_COPY_DEI:
                case BLOG_RULE_CMD_COPY_VLAN_PROTO:
                case BLOG_RULE_CMD_SET_SKB_MARK_QUEUE:
                    blog_rule_p->action[count].vid = ((uint16_t) blog_rule_val->action[n+3] << 8) | (uint16_t) blog_rule_val->action[n+2];
                    n += 4;
                    break;
                    
                default:
               
                    return BDMF_ERR_STATE;
            }
            ++count;
        }
    }

    /* We don't (currently) support the linked list of multiple blog rules, so next is null */
    blog_rule_p->next_p = 0;

    /* Let's add it to our tracking list: */
    blog_rule_list_p->index         = myIndex;
    blog_rule_list_p->blog_rule_p   = blog_rule_p;
    blog_rule_list_p->next          = context->blog_rule_list_head;
    context->blog_rule_list_head    = blog_rule_list_p;

    return BDMF_ERR_OK;
}


static int blog_rule_attr_entry_delete(struct bdmf_object *mo,
                                  struct bdmf_attr *ad,
                                  bdmf_index index)
{
    return BDMF_ERR_OK;

    }

static int blog_rule_attr_entry_read(struct bdmf_object *mo,
                                struct bdmf_attr *ad,
                                bdmf_index index,
                                void *val,
                                uint32_t size)
{
    blog_drv_priv_t *context = (blog_drv_priv_t *)bdmf_obj_data(blog_object);
    rdpa_blog_rule_t *blog_rule_val = (rdpa_blog_rule_t *)val;
    blogRule_t *blog_rule_p;
    uint32_t myIndex = index;
    blog_rule_list_t **blog_rule_list_pp;

    /* if the index doesn't exist then don't continue */
    blog_rule_list_pp = find_ptr_to_blog_rule_entry_by_index(context, myIndex);

    if (!blog_rule_list_pp)
    {
        return BDMF_ERR_NOENT;
    }

    /* allocate necessary structures */
    blog_rule_p = (blogRule_t *)(*blog_rule_list_pp)->blog_rule_p;
    if (!blog_rule_p) 
    {
        return BDMF_ERR_NOENT;
    }

    /* Copy all of the blog rule data: */
    blog_rule_val->filter.nbr_of_vlan_tags      = blog_rule_p->filter.nbrOfVlanTags;
    blog_rule_val->filter.has_pppoe_header      = blog_rule_p->filter.hasPppoeHeader;
    blog_rule_val->filter.ipv4_tos_value        = blog_rule_p->filter.ipv4.value.tos;
    blog_rule_val->filter.ipv4_tos_mask         = blog_rule_p->filter.ipv4.mask.tos;
    blog_rule_val->filter.ipv4_ip_proto_value   = blog_rule_p->filter.ipv4.value.ip_proto;
    blog_rule_val->filter.ipv4_ip_proto_mask    = blog_rule_p->filter.ipv4.mask.ip_proto;
    blog_rule_val->filter.ipv6_tclass_value     = blog_rule_p->filter.ipv6.value.tclass;
    blog_rule_val->filter.ipv6_tclass_mask      = blog_rule_p->filter.ipv6.mask.tclass;
    blog_rule_val->filter.ipv6_next_hdr_value   = blog_rule_p->filter.ipv6.value.nxtHdr;
    blog_rule_val->filter.ipv6_next_hdr_mask    = blog_rule_p->filter.ipv6.mask.nxtHdr;
    blog_rule_val->filter.skb_priority          = blog_rule_p->filter.skb.priority;
    blog_rule_val->filter.skb_mark_flow_id      = blog_rule_p->filter.skb.markFlowId;
    blog_rule_val->filter.skb_mark_port         = blog_rule_p->filter.skb.markPort;
    blog_rule_val->filter.flags                 = blog_rule_p->filter.flags;
    
    blog_rule_val->action_count                 = blog_rule_p->actionCount;

    /* NOTE:
       See comments above in blog_rule_attr_entry_add for this structure:
    */
    {
        int n = 0;
        int count = 0;
        
        while (count < blog_rule_val->action_count)
        {
            blog_rule_val->action[n]   = blog_rule_p->action[count].cmd;
            blog_rule_val->action[n+1] = blog_rule_p->action[count].toTag;
            switch(blog_rule_p->action[count].cmd)
            {
                case BLOG_RULE_CMD_NOP:
                case BLOG_RULE_CMD_POP_PPPOE_HDR:
                case BLOG_RULE_CMD_DECR_TTL:
                    n += 2;
                    break;

                case BLOG_RULE_CMD_SET_MAC_DA:
                case BLOG_RULE_CMD_SET_MAC_SA:
                case BLOG_RULE_CMD_SET_STA_MAC_ADDRESS:
                    blog_rule_val->action[n+2] = blog_rule_p->action[count].macAddr[0];
                    blog_rule_val->action[n+3] = blog_rule_p->action[count].macAddr[1];
                    blog_rule_val->action[n+4] = blog_rule_p->action[count].macAddr[2];
                    blog_rule_val->action[n+5] = blog_rule_p->action[count].macAddr[3];
                    blog_rule_val->action[n+6] = blog_rule_p->action[count].macAddr[4];
                    blog_rule_val->action[n+7] = blog_rule_p->action[count].macAddr[5];
                    n += 8;
                    break;
                
                case BLOG_RULE_CMD_SET_VID:
                case BLOG_RULE_CMD_COPY_VID:
                    blog_rule_val->action[n+2] = (uint8_t)(blog_rule_p->action[count].vid >> 8);
                    blog_rule_val->action[n+3] = (uint8_t)(blog_rule_p->action[count].vid & 0xFF);
                    n += 4;
                    break;
                    
                default:
                    return BDMF_ERR_STATE;
            }
            ++count;
        }
    }

    return BDMF_ERR_OK;
}


/* Blog Rule Filter type */
struct bdmf_aggr_type rdpa_blog_rule_filter_type = {
    .name = "rdpa_blog_rule_filter", .struct_name = "rdpa_blog_rule_filter_t",
    .help = "Blog Rule Filter Structure",
    .fields = (struct bdmf_attr[])
    {
        { .name = "nbr_of_vlan_tags", .help = "Count of VLAN Tags", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, nbr_of_vlan_tags),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "has_pppoe_header", .help = "Has PPPOE Header (1/0)", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, has_pppoe_header),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv4_tos_mask", .help = "v4 tos filter mask", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv4_tos_mask),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv4_tos_value", .help = "v4 tos filter value", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv4_tos_value),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv4_ip_proto_mask", .help = "v4 ip protocol filter mask", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv4_ip_proto_mask),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv4_ip_proto_value", .help = "v4 ip protocol filter value", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv4_ip_proto_value),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv6_tclass_mask", .help = "v6 traffic class filter mask", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv6_tclass_mask),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv6_tclass_value", .help = "v6 traffic class filter value", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv6_tclass_value),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv6_next_hdr_mask", .help = "v6 next header filter mask", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv6_next_hdr_mask),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ipv6_next_hdr_value", .help = "v6 next header filter value", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, ipv6_next_hdr_value),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "skb_priority", .help = "skb priority filter", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, skb_priority),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "skb_mark_flow_id", .help = "skb mark flow id filter", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, skb_mark_flow_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "skb_mark_port", .help = "skb mark port filter", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, skb_mark_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "flags", .help = "skb mark port filter", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_filter_t, flags),
            .flags = BDMF_ATTR_UNSIGNED
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_blog_rule_filter_type);


/* Blog Rule Type */
struct bdmf_aggr_type rdpa_blog_rule_type = {
    .name = "rdpa_blog_rule", .struct_name = "rdpa_blog_rule_t",
    .help = "Blog Rule Structure",
    .fields = (struct bdmf_attr[])
    {
        { .name = "filter", .help = "Blog Rule Filter", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "rdpa_blog_rule_filter", .offset = offsetof(rdpa_blog_rule_t, filter)
        },
        { .name = "action_count", .help = "Count of actions in the list", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_rule_t, action_count),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "action", .help = "Blog rule actions", .size = (BLOG_RULE_ACTION_MAX * BLOG_RULE_ACTION_SIZE * sizeof(uint8_t)),
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_blog_rule_t, action),
            .flags = BDMF_ATTR_HEX_FORMAT
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_blog_rule_type);


/*  Logical port setup structure */
struct bdmf_aggr_type blog_logical_port_type = {
    .name = "blog_logical_port", .struct_name = "rdpa_blog_logical_port_t",
    .help = "Blog Logical Port Setup",
    .fields = (struct bdmf_attr[])
    {
        { .name = "is_wan_port", .help = "True if this logical port is a wan port", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_logical_port_t, is_wan_port),
        },
        { .name = "tx_mtu", .help = "Packets transmitted on this port must be <= this mtu", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_logical_port_t, tx_mtu),
            .flags = BDMF_ATTR_UNSIGNED
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(blog_logical_port_type);



struct bdmf_aggr_type rdpa_blog_info_type = {
    .name = "rdpa_blog_info", .struct_name = "rdpa_blog_info_t",
    .help = "Blog Info Structure",
    .fields = (struct bdmf_attr[])
    {
        { .name = "channel", .help = "Port Number / Tx Channel", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_info_t, channel),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "phyHdrLen", .help = "Phy Header Length", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_info_t, phy_hdr_len),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "phyHdrType", .help = "Phy Header Type", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_info_t, phy_hdr_type),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "GRE_ETH", .help = "GRE_ETH Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, GRE_ETH),
        },
        { .name = "BCM_XPHY", .help = "BCM_XPHY Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, BCM_XPHY),
        },
        { .name = "BCM_SWC", .help = "BCM_SWC Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, BCM_SWC),
        },
        { .name = "ETH_802x", .help = "ETH_802x Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, ETH_802x),
        },
        { .name = "VLAN_8021Q", .help = "VLAN_8021Q Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, VLAN_8021Q),
        },
        { .name = "PPPoE_2516", .help = "PPPoE_2516 Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, PPPoE_2516),
        },
        { .name = "PPP_1661", .help = "PPP_1661 Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, PPP_1661),
        },
        { .name = "PLD_IPv4", .help = "PLD_IPv4 Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, PLD_IPv4),
        },
        { .name = "PLD_IPv6", .help = "PLD_IPv6 Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, PLD_IPv6),
        },
        { .name = "PPTP", .help = "PPTP Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, PPTP),
        },
        { .name = "L2TP", .help = "L2TP Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, L2TP),
        },
        { .name = "GRE", .help = "GRE Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, GRE),
        },
        { .name = "ESP", .help = "ESP Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, ESP),
        },
        { .name = "DEL_IPv4", .help = "DEL_IPv4 Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, DEL_IPv4),
        },
        { .name = "DEL_IPv6", .help = "DEL_IPv6 Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, DEL_IPv6),
        },
        { .name = "PLD_L2", .help = "PLD_L2 Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_info_t, PLD_L2),
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_blog_info_type);


/*  blog_ip_tuple_shared aggregate type */
struct bdmf_aggr_type blog_ip_tuple_shared_type = {
    .name = "blog_ip_tuple_shared", .struct_name = "rdpa_blog_ip_tuple_shared_t",
    .help = "Blog IP Header v4/v6 Shared Fields",
    .fields = (struct bdmf_attr[])
    {
        { .name = "src_ip", .help = "Source IPv4/IPv6 Address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_blog_ip_tuple_shared_t, src_ip)
        },
        { .name = "dst_ip", .help = "Destination IPv4/IPv6 Address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_blog_ip_tuple_shared_t, dst_ip)
        },
        { .name = "src_port", .help = "Source Port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ip_tuple_shared_t, src_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "dst_port", .help = "Destination Port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ip_tuple_shared_t, dst_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "prot", .help = "IP Protocol or Next Header",
            .type = bdmf_attr_number, .size = sizeof(uint8_t), .offset = offsetof(rdpa_blog_ip_tuple_shared_t, prot),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ttl", .help = "TTL/Hop Count", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ip_tuple_shared_t, ttl),
            .flags = BDMF_ATTR_UNSIGNED
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(blog_ip_tuple_shared_type);



/*  blog_ipv4_tuple aggregate type */
struct bdmf_aggr_type blog_ipv4_tuple_type = {
    .name = "blog_ipv4_tuple", .struct_name = "rdpa_blog_ipv4_tuple_t",
    .help = "Blog IPv4 Tuple",
    .fields = (struct bdmf_attr[])
    {
        { .name = "header", .help = "Blog IP Header", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "blog_ip_tuple_shared", .offset = offsetof(rdpa_blog_ipv4_tuple_t, header)
        },
        { .name = "tos", .help = "Type of Service / DSCP-ECN", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ipv4_tuple_t, tos),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "check", .help = "Checksum", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ipv4_tuple_t, check),
            .flags = BDMF_ATTR_UNSIGNED
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(blog_ipv4_tuple_type);



/*  blog_ipv6_tuple aggregate type */
struct bdmf_aggr_type blog_ipv6_tuple_type = {
    .name = "blog_ipv6_tuple", .struct_name = "rdpa_blog_ipv6_tuple_t",
    .help = "Blog IPv6 Tuple",
    .fields = (struct bdmf_attr[])
    {
        { .name = "header", .help = "Blog IP Header", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "blog_ip_tuple_shared", .offset = offsetof(rdpa_blog_ipv6_tuple_t, header)
        },
        { .name = "word0", .help = "word0", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ipv6_tuple_t, word0),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "length", .help = "Total Length", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ipv6_tuple_t, length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tx_hop_limit", .help = "Tx Packet Hop Limit", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ipv6_tuple_t, tx_hop_limit),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "exthdrs", .help = "Bit field of IPv6 extension headers", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ipv6_tuple_t, exthdrs),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "fragflag", .help = "6in4 Upstream IPv4 fragmentation flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_ipv6_tuple_t, fragflag),
        },
        { .name = "tunnel", .help = "Indication of IPv6 tunnel", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_ipv6_tuple_t, tunnel),
        },
        { .name = "ipid", .help = "6in4 Upstream IPv4 identification", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_ipv6_tuple_t, ipid),
            .flags = BDMF_ATTR_UNSIGNED
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(blog_ipv6_tuple_type);


/*  blog_header aggregate type */
struct bdmf_aggr_type blog_header_type = {
    .name = "blog_header", .struct_name = "rdpa_blog_header_t",
    .help = "Blog Header",
    .fields = (struct bdmf_attr[])
    {
        { .name = "tuple", .help = "Blog IP v4 Tuple", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "blog_ipv4_tuple", .offset = offsetof(rdpa_blog_header_t, tuple)
        },
        { .name = "info", .help = "Blog Information & Flags", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "rdpa_blog_info", .offset = offsetof(rdpa_blog_header_t, info)
        },
        { .name = "multicast", .help = "GRE Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_header_t, multicast),
        },
        { .name = "count", .help = "Bit field of IPv6 extension headers", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_header_t, count),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "length", .help = "L2 header total length", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_header_t, length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "encap", .help = "All L2 header types", .size = (BLOG_ENCAP_MAX * sizeof(uint32_t)),
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_blog_header_t, encap),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        { .name = "l2hdr", .help = "Data of all L2 headers", .size = (BLOG_HDRSZ_MAX * sizeof(uint8_t)),
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_blog_header_t, l2hdr),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(blog_header_type);


/*  ip_flow_result aggregate type */
struct bdmf_aggr_type blog_entry_type =
{
    .name = "blog_entry", .struct_name = "rdpa_blog_entry_t",
    .help = "Blog entry",
    .fields = (struct bdmf_attr[])
    {
        { .name = "wfd_rnr", .help = "WFD Flags", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, wfd_rnr),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "vtag_num", .help = "VTAG Number", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, vtag_num),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "eth_type", .help = "Ether Type", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, eth_type),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "lag_port", .help = "LAG port when trunking is done by internal switch/runner", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, lag_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ack_cnt", .help = "Back to back TCP ACKs for prio", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, ack_cnt),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ptm_us_bond", .help = "PTM US Bonding Mode", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, ptm_us_bond),
        },
        { .name = "tos_mode_us", .help = "ToS mode for US: fixed, inherit", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, tos_mode_us),
        },
        { .name = "tos_mode_ds", .help = "ToS mode for DS: fixed, inherit", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, tos_mode_ds),
        },
        { .name = "has_pppoe", .help = "PPPoE Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, has_pppoe),
        },
        { .name = "ack_done", .help = "TCP ACK prio decision made", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, ack_done),
        },
        { .name = "nf_dir_pld", .help = "??", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, nf_dir_pld),
        },
        { .name = "nf_dir_del", .help = "??", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, nf_dir_del),
        },
        { .name = "pop_pppoa", .help = "POP PPPoA flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, pop_pppoa),
        },
        { .name = "insert_eth", .help = "Insert ethernet flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, insert_eth),
        },
        { .name = "iq_prio", .help = "IQ priority flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, iq_prio),
        },
        { .name = "mc_sync", .help = "mc sync flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, mc_sync),
        },
        { .name = "rtp_seq_chk", .help = "RTP seq check flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, rtp_seq_chk),
        },
        { .name = "incomplete", .help = "Incomplete flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_blog_entry_t, incomplete),
        },
        { .name = "mark", .help = "NF mark value on tx", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, mark),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "priority", .help = "Tx priority", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, priority),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "blog_rule_count", .help = "Count of blog rules in this blog entry (currently 0 or 1)",
            .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, blog_rule_count),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "blog_rule_index", .help = "Index of the first blog rule for this blog entry",
            .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, blog_rule_index),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "mcast_learn", .help = "Learn MCast", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, mcast_learn),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "vtag0", .help = "Outer VLAN Tag", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, vtag0),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        { .name = "vtag1", .help = "Inner VLAN Tag", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_blog_entry_t, vtag1),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        { .name = "tupleV6", .help = "Blog IP v6 Tuple", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "blog_ipv6_tuple", .offset = offsetof(rdpa_blog_entry_t, tupleV6)
        },
        { .name = "tx", .help = "TX Header Information", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "blog_header", .offset = offsetof(rdpa_blog_entry_t, tx)
        },
        { .name = "rx", .help = "RX Header Information", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "blog_header", .offset = offsetof(rdpa_blog_entry_t, rx)
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(blog_entry_type);


/* Object attribute descriptors */
static struct bdmf_attr blog_attrs[] = {
    { .name = "entry", .help = "Blog entry containing all data needed by pktrunner to create flow.",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "blog_entry", .array_size = 64,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_NO_RANGE_CHECK,
        .add = blog_attr_entry_add, .del = blog_attr_entry_delete,
        .read = blog_attr_entry_read
    },
    { .name = "rule", .help = "Blog rules which are referenced by blogs.",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_blog_rule", .array_size = 64,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_NO_RANGE_CHECK,
        .add = blog_rule_attr_entry_add, .del = blog_rule_attr_entry_delete,
        .read = blog_rule_attr_entry_read
    },
    { .name = "logport", .help = "Sparse list of logical port attributes.",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "blog_logical_port", .array_size = 32,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE,
        .add = rdpa_blog_logical_port_add, .del = rdpa_blog_logical_port_delete,
        .read = rdpa_blog_logical_port_read
    },

    BDMF_ATTR_LAST
};


static int blog_post_init(struct bdmf_object *mo);
static void blog_destroy(struct bdmf_object *mo);
static int blog_drv_init(struct bdmf_type *drv);
static void blog_drv_exit(struct bdmf_type *drv);
int rdpa_blog_get(bdmf_object_handle *_obj_);

struct bdmf_type blog_drv = {
    .name = "blog",
    .parent = "system",
    .description = "UT Simulation Packet Runner Blog Manager",
    .drv_init = blog_drv_init,
    .drv_exit = blog_drv_exit,
    .post_init = blog_post_init,
    .destroy = blog_destroy,
    .extra_size = sizeof(blog_drv_priv_t),
    .aattr = blog_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_blog, blog_drv);

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
 
int runnerProto_construct(void);
int __init cmdlist_construct(void);

static int blog_post_init(struct bdmf_object *mo)
{
    blog_drv_priv_t *context = (blog_drv_priv_t *)bdmf_obj_data(mo);

    /* save pointer to the blog object */
    blog_object = mo;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "blog");

    context->blog_list_head = NULL;
    context->blog_rule_list_head = NULL;

    cmdlist_construct();
    runnerProto_construct();

    return 0;
}

static void blog_destroy(struct bdmf_object *mo)
{
    bdmf_trace_level_set(&blog_drv, bdmf_trace_level_info);
    blog_object = NULL;
    bdmf_trace_level_set(&blog_drv, bdmf_trace_level_error);
}

/* Init/exit module. Cater for GPL layer */
static int blog_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_blog_drv = rdpa_blog_drv;
    f_rdpa_blog_get = rdpa_blog_get;
#endif
    return 0;
}

static void blog_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_blog_drv = NULL;
    f_rdpa_blog_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get blog object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_blog_get(bdmf_object_handle *_obj_)
{
    if (!blog_object)
        return BDMF_ERR_NOENT;
    bdmf_get(blog_object);
    *_obj_ = blog_object;
    return 0;
}

#endif /* RDP_SIM */
