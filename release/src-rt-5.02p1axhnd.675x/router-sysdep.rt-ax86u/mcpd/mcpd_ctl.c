/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2016:proprietary:standard

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
#include "mcpd_timer.h"
#include <mcpctl.h>

extern t_MCPD_ROUTER mcpd_router;

int mcpd_control_socket_init(void)
{
    struct sockaddr_in sa;
    int             sd;
    int             flags;
    int             optval = 1;
    socklen_t       optlen = sizeof(optval);
  
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "socket() error, %s", strerror(errno));
        return -1;
    }

    /* Allow reusing the socket immediately when application is restarted */
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen))
    {
        MCPD_TRACE(MCPD_TRC_INFO, "setsockopt error %s", strerror(errno));
    }

    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sa.sin_port        = htons( (unsigned short)MCPD_CONTROL_SOCK_PORT);
    if((bind(sd, (struct sockaddr *)&sa, sizeof(sa))) == -1)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "bind() to port %d error, %s", 
                   MCPD_CONTROL_SOCK_PORT, strerror(errno));
        close(sd);
        return -1;
    }

    if ((listen(sd, 3)) == -1)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "listen() to port %d error, %s", 
                   MCPD_CONTROL_SOCK_PORT, strerror(errno));
        close(sd);
        return -1;
    }

    flags = fcntl(sd, F_GETFL, 0);
    if(flags < 0)
    {
        MCPD_TRACE(MCPD_TRC_INFO, "cannot retrieve socket flags. error=%s", 
                   strerror(errno));
    }
    if ( fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0 )
    {
        MCPD_TRACE(MCPD_TRC_INFO, "cannot set socket to non-blocking. error=%s", 
                   strerror(errno));
    }

    mcpd_router.sock_ctl = sd;
    return(0);
}

int mcpd_control_socket_accept(void)
{
    struct sockaddr_un clientAddr;
    unsigned int sockAddrSize;
    int sd;
    int flags;

    if ( mcpd_router.sock_ctl_con != -1 )
    {
        MCPD_TRACE(MCPD_TRC_INFO, "Only one connection available");
        return -1;
    }

    sockAddrSize = sizeof(clientAddr);
    if ((sd = accept(mcpd_router.sock_ctl, (struct sockaddr *)&clientAddr, &sockAddrSize)) < 0) {
        MCPD_TRACE(MCPD_TRC_ERR, "accept connection failed. errno=%d", errno);
        return -1;
    }
  
    flags = fcntl(sd, F_GETFL, 0);
    if(flags < 0) {
        MCPD_TRACE(MCPD_TRC_INFO, "cannot retrieve socket flags. errno=%d", errno);
    }
    if ( fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
        MCPD_TRACE(MCPD_TRC_INFO, "cannot set socket to non-blocking. errno=%d", errno);
    }

    mcpd_router.sock_ctl_con = sd;
    return 0;
}

#if 0
void mcpd_handle_ctl_msg(void *msg)
{
    t_MCPD_CTL_MSG *mcpd_ctl_msg = (t_MCPD_CTL_MSG *)msg;
    switch (mcpd_ctl_msg->cmd)
    {
        case MCPD_CTL_CMD_OBJINFO:
            mcpd_dump_obj_tree();
            break;

        case MCPD_CTL_CMD_MEMINFO:
            mcpd_display_mem_usage();
            break;

        case MCPD_CTL_CMD_ALLINFO:
            mcpd_display_mem_usage();
#ifdef SUPPORT_MLD
            mcpd_config_display(&mcpd_router.igmp_config, &mcpd_router.mld_config);
#else
            mcpd_config_display(&mcpd_router.igmp_config, NULL);
#endif
            //mcpd_display_group_mode();
            mcpd_dump_obj_tree();
            break;
#if 0
        case MCPD_CTL_CMD_MCGRPMODE:
            mcpd_setMcastGroupMode ( (mcpd_ctl_msg->data[0] == MCPD_CTL_CMD_MCGRPMODE_IANA) ?
                                     MULTICAST_MODE_IANA : MULTICAST_MODE_FIRST_IN);
            break;

        case MCPD_CTL_CMD_ADMISSION:
            
            mcpd_router.igmp_config.admission_required = mcpd_ctl_msg->data[0];
            mcpd_router.igmp_config.admission_bridging_filter= mcpd_ctl_msg->data[0];
#ifdef SUPPORT_MLD
            mcpd_router.mld_config.admission_required = mcpd_ctl_msg->data[0];
            mcpd_router.mld_config.admission_bridging_filter= mcpd_ctl_msg->data[0];
#endif
            mcpd_update_multicast_info();
            break;

        case MCPD_CTL_CMD_CONFIGINFO:
#ifdef SUPPORT_MLD
            mcpd_config_display(&mcpd_router.igmp_config, &mcpd_router.mld_config);
#else
            mcpd_config_display(&mcpd_router.igmp_config, NULL);
#endif
            break;

#endif

        case MCPD_CTL_CMD_RELOAD:
#ifdef SUPPORT_MLD
            mcpd_config_read(&mcpd_router.igmp_config,
                             &mcpd_router.mld_config);
#else
            mcpd_config_read(&mcpd_router.igmp_config, NULL);
#endif
            mcpd_update_interface_info();
            mcpd_process_query_timer(NULL); /* start general query timer */
            mcpd_update_multicast_info();
            break;

        default:
            MCPD_TRACE(MCPD_TRC_ERR, "unrecognized control command 0x%x", mcpd_ctl_msg->cmd);
            break;
    }
}
#endif

void mcpd_control_socket_receive(void)
{
    int length;
    int pre_igmp_flood;
#ifdef SUPPORT_MLD
    int pre_mld_flood;
#endif

    length = recv(mcpd_router.sock_ctl_con, &mcpd_router.sock_buff[0], 
                  BCM_MCAST_NL_RX_BUF_SIZE, 0);
    if (length < 0) {
       MCPD_TRACE(MCPD_TRC_ERR, "recvfrom() error %d: %s", errno, strerror(errno));
    }
    else if (length == 0) {
        /* socket has been closed */
        close(mcpd_router.sock_ctl_con);
        mcpd_router.sock_ctl_con = -1;
    }
    else {
        t_MCPD_CTL_MSG *pMsg;
        union {
            t_MCPD_CTL_GRP_MODE *pGrpMsg;
            t_MCPD_CTL_ADMISSION *pAdmis;
        }u;

        while (length)
        {
            int msg_len = sizeof(t_MCPD_CTL_MSG);
            if ( length < msg_len )
            {
                /* truncated message - ignore */
                break;
            }

            pMsg = (t_MCPD_CTL_MSG *)&mcpd_router.sock_buff[0];
            switch (pMsg->cmd)
            {
                case MCPD_CTL_CMD_MCGRPMODE:
                    msg_len += sizeof(t_MCPD_CTL_GRP_MODE);
                    break;

                case MCPD_CTL_CMD_ADMISSION:
                    msg_len += sizeof(t_MCPD_CTL_ADMISSION);
                    break;

                default:
                    break;
            }

            if ( length < msg_len )
            {
                /* truncated message - ignore */
                break;
            }
            length -= msg_len;

            switch (pMsg->cmd)
            {
                case MCPD_CTL_CMD_RELOAD:
                    pre_igmp_flood = mcpd_router.igmp_config.flood_enable;
#ifdef SUPPORT_MLD
                    pre_mld_flood  = mcpd_router.mld_config.flood_enable;
                    mcpd_config_read(&mcpd_router.igmp_config,
                                     &mcpd_router.mld_config);
#else
                    mcpd_config_read(&mcpd_router.igmp_config, NULL);
#endif
                    mcpd_update_interface_info(BCM_MCAST_PROTO_ALL);
                    mcpd_process_query_timer(NULL); /* start general query timer */
                    mcpd_update_multicast_info();

#ifdef SUPPORT_MLD
                    mcpd_update_flooding_status(&pre_igmp_flood, &pre_mld_flood);                    
#else
                    mcpd_update_flooding_status(&pre_igmp_flood, NULL);
#endif

                    break;
    
                case MCPD_CTL_CMD_OBJINFO:
                    mcpd_dump_obj_tree();
                    break;
    
                case MCPD_CTL_CMD_MEMINFO:
                    mcpd_display_mem_usage();
                    break;
    
                case MCPD_CTL_CMD_CONFIGINFO:
#ifdef SUPPORT_MLD
                    mcpd_config_display(&mcpd_router.igmp_config, &mcpd_router.mld_config);
#else
                    mcpd_config_display(&mcpd_router.igmp_config, NULL);
#endif
                    break;
    
                case MCPD_CTL_CMD_ALLINFO:
                    mcpd_display_mem_usage();
#ifdef SUPPORT_MLD
                    mcpd_config_display(&mcpd_router.igmp_config, &mcpd_router.mld_config);
#else
                    mcpd_config_display(&mcpd_router.igmp_config, NULL);
#endif
                    mcpd_display_group_mode();
                    mcpd_dump_obj_tree();
                    mcpd_timer_dumpEvents();
                    break;
    
                case MCPD_CTL_CMD_MCGRPMODE:
                    u.pGrpMsg = (t_MCPD_CTL_GRP_MODE *)(pMsg + 1);
                    mcpd_setMcastGroupMode( (u.pGrpMsg->mode == MCPD_CTL_CMD_MCGRPMODE_IANA) ?
                                             MULTICAST_MODE_IANA : MULTICAST_MODE_FIRST_IN);
                    break;
    
                case MCPD_CTL_CMD_ADMISSION:
                    u.pAdmis = (t_MCPD_CTL_ADMISSION*)(pMsg + 1);
                    mcpd_router.igmp_config.admission_required = u.pAdmis->enable;
                    mcpd_router.igmp_config.admission_bridging_filter = u.pAdmis->enable;
#ifdef SUPPORT_MLD
                    mcpd_router.mld_config.admission_required = u.pAdmis->enable;
                    mcpd_router.mld_config.admission_bridging_filter = u.pAdmis->enable;
#endif
                    mcpd_update_multicast_info();
                    break;

                default:
                    MCPD_TRACE(MCPD_TRC_INFO, "unknown message (%d)", pMsg->cmd);
                    break;
            }
        }
    }
}

