/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/
/***************************************************************************
 * File Name  : mcpd_nl.c
 *
 * Description: API for netlink communication with kernel
 *              
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/netlink.h>
#include <linux/un.h>
#include "mcpd.h"
#include "common.h"
#include "mcpd_nl.h"
#include "igmp.h"
#include "mld.h"
#include "igmp_main.h"
#include "mld_main.h"
#include "obj_hndlr.h"
#include "mcpd_config.h"
#include "mcpd_main.h"

extern t_MCPD_ROUTER mcpd_router;

int mcpd_netlink_init(void)
{
    /* create socket with portid "MCPD" */
    if ( bcm_mcast_api_socket_create(&mcpd_router.sock_nl, 0x4D435044) < 0 )
    {
        mcpd_router.sock_nl = -1;
        return MCPD_RET_GENERR;
    }

    /* register for notifications */
    bcm_mcast_api_register(mcpd_router.sock_nl, 0);

    return MCPD_RET_OK;
} /* mcpd_netlink_init */

int mcpd_netlink_shutdown(void)
{
    close(mcpd_router.sock_nl);
  
    return MCPD_RET_OK;
} /* mcpd_netlink_shutdown */

void mcpd_netlink_process_msg(int type, unsigned char *pdata, int data_len)
{
    t_BCM_MCAST_PKT_INFO *pkt_info = NULL;
    t_BCM_MCAST_IGMP_PURGE_ENTRY *purge_entry;
    t_BCM_MCAST_REGISTER *preg;
    t_MCPD_RET_CODE reply = 0;

    MCPD_TRACE(MCPD_TRC_LOG, "Received message of type %d", type);
    mcpd_dump_buf((char *)pdata, data_len);

    switch(type)
    {
        case BCM_MCAST_MSG_UNREGISTER:
            /* no action required */
            break;

        case BCM_MCAST_MSG_REGISTER:
            MCPD_TRACE(MCPD_TRC_LOG, "BCM_MCAST_MSG_REGISTER");
            preg = (t_BCM_MCAST_REGISTER *)pdata;
            if ( preg->primary )
            {
               /* another application has registered for exclusive access to notifications
                  perform cleanup */
               MCPD_TRACE(MCPD_TRC_LOG, "MCPD will no longer receive notifications");

               mcpd_interface_cleanup(NULL, 0, 1, MCPD_PROTO_MAX);

               /* cleanup all mcast forwarding entries*/
               if(bcm_mcast_api_fdb_cleanup(mcpd_router.sock_nl) < 0)
               {
                   MCPD_TRACE(MCPD_TRC_ERR, "Error while sending clean up msg");
               }
            }
            break;

        case BCM_MCAST_MSG_IGMP_PKT:
            MCPD_TRACE(MCPD_TRC_LOG, "BCM_MCAST_MSG_IGMP_PKT");
            pkt_info = (t_BCM_MCAST_PKT_INFO *)pdata;

            /* Set OVS response pointer to null for IGMP pkt
               from linux bridge */
            reply = mcpd_igmp_process_input(pkt_info);
            if (pkt_info->packetIndex != -1) {
                MCPD_TRACE(MCPD_TRC_INFO, "BCM_MCAST_MSG_IGMP_PKT.  Admitted = %d.", (reply == MCPD_RET_OK) ? TRUE : FALSE);
                bcm_mcast_api_admission_result(mcpd_router.sock_nl, 
                                               pkt_info->parent_ifi, 
                                               (reply == MCPD_RET_OK) ? TRUE : FALSE,
                                               pkt_info->packetIndex,
                                               BCM_MCAST_PROTO_IPV4);
            }
            break;

#ifdef SUPPORT_MLD
        case BCM_MCAST_MSG_MLD_PKT:
            MCPD_TRACE(MCPD_TRC_LOG, "BCM_MCAST_MSG_MLD_PKT");
            pkt_info = (t_BCM_MCAST_PKT_INFO *)pdata;

            /* Set OVS response pointer to null for MLD pkt
               from linux bridge */
            mcpd_mld_process_input(pkt_info);
            if (pkt_info->packetIndex != -1) {
                MCPD_TRACE(MCPD_TRC_INFO, "BCM_MCAST_MSG_MLD_PKT.  Admitted = %d.", (reply == MCPD_RET_OK) ? TRUE : FALSE);
                bcm_mcast_api_admission_result(mcpd_router.sock_nl, 
                                               pkt_info->parent_ifi, 
                                               TRUE,
                                               pkt_info->packetIndex,
                                               BCM_MCAST_PROTO_IPV6);
            }
            break; 
#endif

        case BCM_MCAST_MSG_IGMP_PURGE_ENTRY:
            MCPD_TRACE(MCPD_TRC_LOG, "BCM_MCAST_MSG_IGMP_PURGE_ENTRY");
            purge_entry = (t_BCM_MCAST_IGMP_PURGE_ENTRY *)pdata;
            mcpd_igmp_admission_control(MCPD_ADMISSION_LEAVE,
                                        purge_entry->rep_ifi,
                                        (UINT8 *)&purge_entry->grp,
                                        (UINT8 *)&purge_entry->src,
                                        (UINT8 *)&purge_entry->rep,
                                        purge_entry->tci,
                                        purge_entry->rep_proto_ver);
            break;

        case BCM_MCAST_MSG_QUERY_TRIGGER:
            MCPD_TRACE(MCPD_TRC_LOG, "BCM_MCAST_MSG_QUERY_TRIGGER");
            mcpd_process_query_timer(NULL);
            break;

        default:
           MCPD_TRACE(MCPD_TRC_ERR, "mcpd_nl msg %d not supported", type);
           break;
    }

    return;
} /* mcpd_netlink_process_msg */

int mcpd_netlink_recv_mesg(void)
{
    if ( bcm_mcast_api_nl_recv(mcpd_router.sock_nl, mcpd_router.sock_buff, BCM_MCAST_NL_RX_BUF_SIZE, mcpd_netlink_process_msg) < 0 )
    {
        MCPD_TRACE(MCPD_TRC_ERR,"Error receiving message\n");
        return MCPD_RET_GENERR;
    }

    return MCPD_RET_OK;
} /* mcpd_netlink_recv_mesg */

