/*
  <:copyright-BRCM:2019:proprietary:standard

  Copyright (c) 2019 Broadcom 
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
* File Name  : archer_iq.c
*
* Description: This file implements Ingress QoS for Archer.
*
*******************************************************************************
*/

/* -----------------------------------------------------------------------------
 *                      Ingress QoS (IQ)
 * -----------------------------------------------------------------------------
 * Ingress QoS feature defines a flow has low or high ingress priority at an 
 * ingress interface based on a pre-defined criterion or the layer4 (TCP, UDP)
 * destination port. Under normal load conditions all received packets are
 * accepted and forwarded. But under CPU congestion high ingress priority are
 * accepted whereas low ingress priority packets are dropped.
 *
 * CPU congestion detection:
 * -------------------------
 * Ingress QoS constantly monitors the queue depth for an interface to detect
 * CPU congestion. If the queue depth is greater than the IQ high threshold,
 * CPU congestion has set in. When the queue depth is less than IQ low
 * threshold, then CPU congestion has abated.  
 *
 * CPU Congestion Set:
 * -------------------
 * When the CPU is congested only high ingress priority are accepted 
 * whereas low ingress priority packets are dropped.
 *
 * These are some of the pre-defined criterion for a flow to be 
 * high ingress priority:
 * a) High Priority Interface
 *    - Any packet received from XTM 
 *    - Any packet received from or sent to WLAN 
 * b) Multicast
 * c) Flows configured by default through the following ALGs
 *    - SIP (default SIP UDP port = 5060), RTSP ports for data
 *    - RTSP (default UDP port = 554), RTSP ports for data
 *    - H.323 (default UDP port = 1719, 1720)
 *    - MGCP (default UDP ports 2427 and 2727)
 *    Note:- The above ALGs are not invoked for bridging mode. Therefore 
 *           in bridging mode, user code needs to call APIs such as
 *           fap_iqos_add_L4port() to classify a L4 port as high.
 *      
 * d) Other flows configured through Ingress QoS APIs:
 *    - DNS  UDP port = 53  
 *    - DHCP UDP port = 67  
 *    - DHCP UDP port = 68  
 *    - HTTP TCP port = 80, and 8080  
 *    Note:- If required user code can add more flows in the iq_init() or
 *           use the fap_iqos_add_L4port().
 *
 * Note: API prototypes are given in iqos.h file.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>

#include <linux/iqos.h>
#include <ingqos.h>

#include "sysport_rsb.h"
#include "sysport_classifier.h"
#include "archer.h"
#include "archer_driver.h"

#include <archer_cpu_queues.h>

//#define CC_ARCHER_IQ_DEBUG

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))

/*******************************************************************
 *
 * Internal prototypes
 *
 *******************************************************************/

#define ARCHER_IQ_CAPABILITY    (IQ_KEY_MASK_IP_PROTO +         \
                                 IQ_KEY_MASK_DST_PORT +         \
                                 IQ_KEY_MASK_DSCP +             \
                                 IQ_KEY_MASK_ETHER_TYPE +       \
                                 IQ_KEY_MASK_L3_PROTO)

/* 64 bit key mask */
#define ARCHER_IQ_DSCP_SHIFT    0
#define ARCHER_IQ_DSCP_WIDTH    6

#define ARCHER_IQ_L4DST_PORT_SHIFT  6
#define ARCHER_IQ_L4DST_PORT_WIDTH  16

#define ARCHER_IQ_IPPROTO_SHIFT     22
#define ARCHER_IQ_IPPROTO_WIDTH     8

#define ARCHER_IQ_ETHTYPE_SHIFT     30
#define ARCHER_IQ_ETHTYPE_WIDTH     16

#define ARCHER_IQ_L3_PROTO_SHIFT    46
#define ARCHER_IQ_L3_PROTO_WIDTH    16

#define FIELD_MASK(bits, shift)  ( ( (1ULL<<(bits)) - 1 ) << shift )

#define ARCHER_IQ_TBL_SIZE          64

/* 62 bit out of 64 bit used */
typedef struct {
    uint64_t    keymask;
    uint64_t    keyvalue;
    uint32_t    prio;
    uint8_t     action;
    uint8_t     next_idx;

} archer_iq_entry_t;

typedef struct {

    int enable;

    uint8_t  valid_list;
    uint8_t  free_list;
    archer_iq_entry_t   ent[ARCHER_IQ_TBL_SIZE];

} archer_iq_t;

static archer_iq_t  archer_iq_g;

/*******************************************************************
 *
 * Local Functions
 *
 *******************************************************************/

static void archer_iq_form_entry (void *iq_param, uint64_t *entMask, uint64_t *entValue)
{
    iq_param_t *param = (iq_param_t *)iq_param;

    uint32_t key_mask = param->key_mask;
    iq_key_data_t *key_data = &param->key_data;
    uint8_t ipProto = key_data->ip_proto;
    uint16_t dport  = key_data->l4_dst_port;
    uint8_t dscp  = key_data->dscp;
    uint16_t ethType = key_data->eth_type;
    uint16_t l3_proto = key_data->l3_proto;

    uint64_t    entry_mask = 0;
    uint64_t    entry_value = 0;

    /* form the keymask */
    if (key_mask & IQ_KEY_MASK_DSCP)
    {
        entry_mask |= FIELD_MASK(ARCHER_IQ_DSCP_WIDTH, ARCHER_IQ_DSCP_SHIFT);
        entry_value |= ((uint64_t)dscp << ARCHER_IQ_DSCP_SHIFT);
    }
    if (key_mask & IQ_KEY_MASK_DST_PORT)
    {
        entry_mask |= FIELD_MASK(ARCHER_IQ_L4DST_PORT_WIDTH, ARCHER_IQ_L4DST_PORT_SHIFT);
        entry_value |= ((uint64_t)dport << ARCHER_IQ_L4DST_PORT_SHIFT);
    }
    if (key_mask & IQ_KEY_MASK_IP_PROTO)
    {
        entry_mask |= FIELD_MASK(ARCHER_IQ_IPPROTO_WIDTH, ARCHER_IQ_IPPROTO_SHIFT);
        entry_value |= ((uint64_t)ipProto << ARCHER_IQ_IPPROTO_SHIFT);
    }
    if (key_mask & IQ_KEY_MASK_ETHER_TYPE)
    {
        entry_mask |= FIELD_MASK(ARCHER_IQ_ETHTYPE_WIDTH, ARCHER_IQ_ETHTYPE_SHIFT);
        entry_value |= ((uint64_t)ethType << ARCHER_IQ_ETHTYPE_SHIFT);
    }
    if (key_mask & IQ_KEY_MASK_L3_PROTO)
    {
        entry_mask |= FIELD_MASK(ARCHER_IQ_L3_PROTO_WIDTH, ARCHER_IQ_L3_PROTO_SHIFT);
        entry_value |= ((uint64_t)l3_proto << ARCHER_IQ_L3_PROTO_SHIFT);
    }

    *entMask = entry_mask;
    *entValue = entry_value;
}

static int archer_iq_get_entry (uint64_t entry_mask, uint64_t entry_value, uint8_t **pre_idxp)
{
    uint8_t ret = 0;
    uint8_t valid_idx;
    archer_iq_entry_t  *entp;
    
    valid_idx = archer_iq_g.valid_list;
    *pre_idxp = &archer_iq_g.valid_list;

    while (valid_idx)
    {
        entp = &archer_iq_g.ent[valid_idx];
        if ((entp->keymask == entry_mask) &&
            (entp->keyvalue == entry_value))
        {
            ret = valid_idx;
            break;
        }
        valid_idx = entp->next_idx;
        *pre_idxp = &entp->next_idx;
    }

    return ret;
}

int archerIq_add_entry(void *iq_param)
{
    iq_param_t *param = (iq_param_t *)iq_param;

    int ret = -1;
    uint8_t add_idx = 0;
    archer_iq_entry_t  *entp = NULL;

    uint8_t *pre_idxp;
    uint64_t    entry_mask;
    uint64_t    entry_value;

    archer_iq_form_entry(iq_param, &entry_mask, &entry_value);

    add_idx = archer_iq_get_entry (entry_mask, entry_value, &pre_idxp);
    /* check if the entry already exist */
    if (add_idx)
    {
        entp = &archer_iq_g.ent[add_idx];
        if (entp->prio == param->prio)
        {
            __logError("entry mask 0x%llx value 0x%llx already exist\n", entry_mask, entry_value);
            add_idx = 0;
        }
        else
        {
            __logError("entry mask 0x%llx value 0x%llx re-add with different prio prep %u\n", entry_mask, entry_value, *pre_idxp);
            /* take out the entry from the list for resorting on priority */
            *pre_idxp = entp->next_idx;
            entp->next_idx = 0;
        }
        if (entp->action != param->action.value)
        {
            /* action changed, take the new one */
            entp->action = param->action.value;
        }
    }
    else
    {
        /* entry does not exist, add it to the table */
        if (archer_iq_g.free_list)
        {
            /* allocate 1 free entry */
            add_idx = archer_iq_g.free_list;
            entp = &archer_iq_g.ent[add_idx];

            archer_iq_g.free_list = entp->next_idx;

            entp->keymask = entry_mask;
            entp->keyvalue = entry_value;
            entp->prio = param->prio;
            entp->action = param->action.value;
        }
    }
    if (add_idx)
    {
        archer_iq_entry_t *idxp;
        uint8_t valid_idx;

        valid_idx = archer_iq_g.valid_list;
        pre_idxp = &archer_iq_g.valid_list;

        /* sort the entry based on priority */
        while(valid_idx)
        {
            idxp = &archer_iq_g.ent[valid_idx];

            if (idxp->prio > entp->prio)
            {
                valid_idx = idxp->next_idx;
                pre_idxp = &idxp->next_idx;
            }
            else
            {
                /* indexed entry is of equal or lower priority, add it */
                *pre_idxp = add_idx;
                entp->next_idx = valid_idx;
                break;
            }
        }
        if (valid_idx == 0)
        {
            /* either table is empty of it only contains higher priority entries, add it to end of list */
            *pre_idxp = add_idx;
            entp->next_idx = 0;
        }
        ret = add_idx;
    }

    return 0;
}

int archerIq_delete_entry(void *iq_param)
{
    uint64_t    entry_mask;
    uint64_t    entry_value;
    uint8_t add_idx = 0;
    archer_iq_entry_t  *entp = NULL;
    uint8_t *pre_idxp;

    archer_iq_form_entry(iq_param, &entry_mask, &entry_value);

    add_idx = archer_iq_get_entry (entry_mask, entry_value, &pre_idxp);

    if (add_idx)
    {
        entp = &archer_iq_g.ent[add_idx];
        /* take out the entry from the list for resorting on priority */
        *pre_idxp = entp->next_idx;
        entp->next_idx = archer_iq_g.free_list;
        entp->keymask = 0;
        entp->keyvalue = 0;
        archer_iq_g.free_list = add_idx;
    }
    else
    {
        __logError("entry mask 0x%llx value 0x%llx not found\n", entry_mask, entry_value);
    }

    return 0;
}

int archerIq_setStatus(void *iq_param)
{
    iq_param_t *param = (iq_param_t *)iq_param;
    archer_iq_t *archer_iq = &archer_iq_g;
    uint32_t status = param->status;
    
    bcm_print ("Archer IQ status changed from %d to %d\n", archer_iq->enable, status);
    archer_iq->enable = status;

    return 0;
}

int archerIq_dumpStatus(void *iq_param)
{
    archer_iq_t *archer_iq = &archer_iq_g;
    bcm_print("Archer IQ status %d\n", archer_iq->enable);

    return 0;
}

int archerIq_dump_porttbl(void *iq_param)
{
    int idx;
    archer_iq_t *archer_iq = &archer_iq_g;

    idx = archer_iq->valid_list;

    while (idx)
    {
        bcm_print("idx %d keymask 0x%llx keyvalue 0x%llx prio %d\n", idx, 
                  archer_iq->ent[idx].keymask, archer_iq->ent[idx].keyvalue,
                  archer_iq->ent[idx].prio);

        idx = archer_iq->ent[idx].next_idx;
    }

#if defined (CC_ARCHER_IQ_DEBUG)
    idx = archer_iq->free_list;
    bcm_print("archer iq free list -> %d", idx);
    while (idx)
    {
        idx = archer_iq->ent[idx].next_idx;
        bcm_print(" -> %d", idx);
    }
    bcm_print("\n");
#endif

    return 0;
}


static const iq_hw_info_t archerIq_info_db = {
    .mask_capability = ARCHER_IQ_CAPABILITY,
    .add_entry = archerIq_add_entry,
    .delete_entry = archerIq_delete_entry,
    .set_status = archerIq_setStatus,
    .get_status = archerIq_dumpStatus,
    .dump_table = archerIq_dump_porttbl,
};
/*
*******************************************************************************
* Function   : archer_iq_register
* Description: register Archer (HW accelerator) with IngQos Driver
*******************************************************************************
*/
int __init archer_iq_register(void)
{
    int idx;
    archer_iq_t *archer_iq = &archer_iq_g;
    for (idx = 1; idx < ARCHER_IQ_TBL_SIZE; idx++)
    {
        archer_iq->ent[idx].next_idx = idx+1;
        archer_iq->ent[idx].keymask = 0;
        archer_iq->ent[idx].keyvalue = 0;
        archer_iq->ent[idx].prio = 0;
    }
    archer_iq->ent[ARCHER_IQ_TBL_SIZE-1].next_idx = 0;
    archer_iq->free_list = 1;
    archer_iq->valid_list = 0;

    if (bcm_iq_register_hw ((iq_hw_info_t *)&archerIq_info_db))
    {
        __logError("failed to register Archer to Ingress QoS\n");
    }
    else
    {
        __logInfo("Complete registering Archer to Ingress QoS\n");
    }
    return 0;
}

/*
*******************************************************************************
* Function   : archer_iq_deregister
* Description: unregister Archer (HW accelerator) with IngQos Driver
*******************************************************************************
*/
void __exit archer_iq_deregister(void)
{
    bcm_iq_unregister_hw ((iq_hw_info_t *)&archerIq_info_db);
}
/*
*******************************************************************************
* Function   : archer_iq_get_l4dport_ipproto
* Description: extra L4 dst port and IP protocol from data packet
*******************************************************************************
*/
static void archer_iq_get_l4dport_ipproto(sysport_rsb_t *rsb_p, uint8_t *packet_p, uint16_t *dport, uint8_t *ipproto)
{
    /* L4 ports are in the same location for both TCP and UDP packets so the TCP pachet header is used */
    BlogTcpHdr_t *th_p;

    if(SYSPORT_RSB_L3_TYPE_IPV4 == rsb_p->l3_type ||
       SYSPORT_RSB_L3_TYPE_4IN6 == rsb_p->l3_type)
    {
        /* IPv4 packet */
        BlogIpv4Hdr_t *ip_p = (BlogIpv4Hdr_t *)(packet_p + rsb_p->ip_offset);
        int ihl = ip_p->ihl << 2;
        th_p = ((void *)ip_p) + ihl;
        *ipproto = ip_p->proto;
    }
    else
    {
        /* IPv6 packet */
        BlogIpv6Hdr_t *ip6_p = (BlogIpv6Hdr_t *)(packet_p + rsb_p->ip_offset);
        th_p = (BlogTcpHdr_t *)(ip6_p + 1);
        *ipproto = ip6_p->nextHdr;
    }
    *dport = ntohs(th_p->dPort);
}
#endif //(defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))

/*
*******************************************************************************
* Function   : archer_iq_sort
* Description: sort incoming data to high / lo priority
*******************************************************************************
*/
uint8_t archer_iq_sort (sysport_rsb_t *rsb_p, uint8_t *packet_p)
{
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    archer_iq_t *archer_iq = &archer_iq_g;
    archer_iq_entry_t *entp;
    uint8_t ipProto = 0, dscp = 0;
    uint16_t dport = 0, ethType = 0, l3_proto = 0;
    volatile uint16_t *ethType_p;
    uint8_t valid_idx;
    uint8_t retPrio = 0;

    uint64_t pktKey = 0;

    if (!archer_iq->enable)
    {
        return CPU_RX_LO;
    }
    else if (rsb_p->tuple.header.valid &&
             (rsb_p->tuple.header.flow_type == SYSPORT_RSB_FLOW_TYPE_MCAST))
    {
        return CPU_RX_HI;
    }
    if (rsb_p->l3_type != SYSPORT_RSB_L3_TYPE_UNKNOWN)
    {
        /* extract the l4 dport and ip protocol from packet */
        archer_iq_get_l4dport_ipproto (rsb_p, packet_p, &dport, &ipProto);

        /* check filtering on dscp */
        dscp = (rsb_p->ip_tos >> 2);
    }
    /* get EthType from packet */
    ethType_p = (volatile uint16_t *)&packet_p[2*BLOG_ETH_ADDR_LEN];
    ethType = ntohs(*ethType_p);

    if (rsb_p->nbr_of_vlans)
    {
        /* VLAN packet, get the actual EthType */
        ethType_p = (volatile uint16_t *)&packet_p[(2*BLOG_ETH_ADDR_LEN) + (BLOG_VLAN_HDR_LEN * rsb_p->nbr_of_vlans)];
        ethType = ntohs(*ethType_p);
    }

    /* L3 protocol is applicable to PPPoE packet only */
    if (rsb_p->pppoe)
    {
        BlogPppoeHdr_t *pppoe_p = (BlogPppoeHdr_t *)&packet_p[sizeof(BlogEthHdr_t) + (BLOG_VLAN_HDR_LEN * rsb_p->nbr_of_vlans)];
        l3_proto = ntohs(pppoe_p->pppType);
    }

    pktKey |= ((uint64_t)dscp << ARCHER_IQ_DSCP_SHIFT);
    pktKey |= ((uint64_t)dport << ARCHER_IQ_L4DST_PORT_SHIFT);
    pktKey |= ((uint64_t)ipProto << ARCHER_IQ_IPPROTO_SHIFT);
    pktKey |= ((uint64_t)ethType << ARCHER_IQ_ETHTYPE_SHIFT);
    pktKey |= ((uint64_t)l3_proto << ARCHER_IQ_L3_PROTO_SHIFT);

    valid_idx = archer_iq->valid_list;
    while(valid_idx)
    {
        entp = &archer_iq->ent[valid_idx];

        if ( (pktKey & entp->keymask) == entp->keyvalue)
        {
            /* match found */
            retPrio = entp->action;
            break;
        }
        valid_idx = entp->next_idx;
    }

#if defined (CC_ARCHER_IQ_DEBUG)
    if (retPrio)
    {
        bcm_print("Archer IQ: multicast %d proto %d dport %d dscp %d  ethType 0x%x l3_proto 0x%x\n", 
                  (rsb_p->tuple.header.flow_type == SYSPORT_RSB_FLOW_TYPE_MCAST), ipProto, dport, dscp, ethType, l3_proto);
    }
#endif
    /* map priority to CPU queue ID fo network drivers */
    return ((retPrio > 0) ? CPU_RX_HI : CPU_RX_LO);
#else
    return CPU_RX_HI; /* default to high priority queue if ING_QOS not supported */
#endif
}

