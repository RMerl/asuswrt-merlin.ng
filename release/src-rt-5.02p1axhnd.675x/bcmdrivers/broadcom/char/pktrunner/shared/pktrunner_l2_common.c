/*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
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
#include "pktrunner_l2_common.h"
#include "rdpa_mw_blog_parse.h"

#define protoLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_PKTRUNNER, BCM_LOG_LEVEL_DEBUG)
#define protoDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoPrint(fmt, arg...)   bcm_print(fmt, ##arg)

uint32_t pktrunner_l2flow_key_construct(Blog_t *blog_p, rdpa_l2_flow_key_t* l2_key)
{
    /*Start of key build */
    memcpy(&l2_key->src_mac, &blog_p->rx.l2hdr[6], sizeof(BlogEthAddr_t));
    memcpy(&l2_key->dst_mac, &blog_p->rx.l2hdr[0], sizeof(BlogEthAddr_t));

    l2_key->vtag_num = blog_p->vtag_num;

    if (l2_key->vtag_num > 0)
       l2_key->vtag0 = htonl(blog_p->vtag[0]);
    
    if (l2_key->vtag_num > 1)
        l2_key->vtag1 = htonl(blog_p->vtag[1]);

    l2_key->eth_type = ntohs(blog_p->eth_type);

    if (blog_p->rx.info.phyHdrType == BLOG_GPONPHY)    
    {
        protoDebug("source.phy GPON\n");
        l2_key->ingress_if = rdpa_wan_type_to_if(rdpa_wan_gpon);
        l2_key->dir = rdpa_dir_ds;
        l2_key->wan_flow = blog_p->rx.info.channel;
    }
    else if (blog_p->rx.info.phyHdrType == BLOG_EPONPHY)
    {
        protoDebug("source.phy EPON\n");
        l2_key->ingress_if = rdpa_wan_type_to_if(rdpa_wan_epon);
        l2_key->dir = rdpa_dir_ds;
        l2_key->wan_flow = blog_p->rx.info.channel;
    }
    else if (((struct net_device *)blog_p->rx_dev_p)->priv_flags & IFF_WANDEV)
    {
        protoDebug("source.phy ETH WAN\n");
        l2_key->ingress_if = rdpa_wan_type_to_if(rdpa_wan_gbe);
        l2_key->dir = rdpa_dir_ds;
    }
    else
    {
        protoDebug("source.phy ETH/WLAN\n");
        l2_key->ingress_if = rdpa_mw_root_dev2rdpa_if((struct net_device *)blog_p->rx_dev_p);
        l2_key->dir = rdpa_dir_us;
    }

    /*set the tos/tc */
    l2_key->tos = blog_p->rx.tuple.tos;

    l2_key->tcp_pure_ack = blog_p->key.tcp_pure_ack;

    /* End of key build */

    return 0;
}
