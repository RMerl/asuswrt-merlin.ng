/*
* <:copyright-BRCM:2016:proprietary:standard
* 
*    Copyright (c) 2016 Broadcom 
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
#include <cms.h>
#include <cms_msg.h>
#include <cms_log.h>
#include <cms_mem.h>
#include <cms_tmr.h>
#include <cms_tms.h>
#include <cms_fil.h>
#include <prctl.h>

#include "mcpd.h"
#include "mcpd_main.h"
#include "mcpd_omci.h"
#include "mcpd_config.h"

extern t_MCPD_ROUTER mcpd_router;
extern int mcpd_main_quit;

void *mcpdMsgHandle = NULL; /* CMS message handle */
static int cms_msg_fd;

int mcpd_cms_init( void )
{
    CmsLogLevel logLevel=DEFAULT_LOG_LEVEL;

    /* initialize CMS logging.
     * TODO: parse args and accept -v to set log level from cmdline
     */
    cmsLog_initWithName(EID_MCPD, "mcpd");
    cmsLog_setLevel(logLevel);

    /* Connect to smd */
    if(cmsMsg_initWithFlags(EID_MCPD, 0, &mcpdMsgHandle) != CMSRET_SUCCESS)
    {
        mcpdMsgHandle = NULL;
        return -1;
    }
    cmsMsg_getEventHandle(mcpdMsgHandle, &cms_msg_fd);

    return 0;
}

void mcpd_cms_cleanup(void)
{
    cmsMsg_cleanup(&mcpdMsgHandle);
}

int mcpd_cms_get_fd( void )
{
    return cms_msg_fd;   
}

CmsRet mcpd_cms_send_reply(const CmsMsgHeader *msgReq)
{
    CmsRet ret = CMSRET_SUCCESS;
    CmsMsgHeader msgRes;

    memset(&msgRes, 0, sizeof(CmsMsgHeader));
    msgRes.type = msgReq->type;
    msgRes.src = msgReq->dst;
    msgRes.dst = msgReq->src;
    msgRes.flags_response = 1;
    msgRes.dataLength = 0;
    msgRes.sequenceNumber = msgReq->sequenceNumber;
    msgRes.wordData = ret;

    if ((ret = cmsMsg_send(mcpdMsgHandle, &msgRes)) != CMSRET_SUCCESS)
    {
        cmsLog_error("could not send out reply, ret=%d", ret);
    }
    else
    {
        cmsLog_notice("sent reply successfully");
    }
    return ret;
} /* mcpd_omci_mib_reset_send_reply */

void mcpd_cms_process_msg(void)
{
    CmsMsgHeader *msg;
    CmsRet ret;
    int pre_igmp_flood;
#ifdef SUPPORT_MLD
    int pre_mld_flood;
#endif

    ret = cmsMsg_receiveWithTimeout(mcpdMsgHandle, &msg, 0);
    while(ret == CMSRET_SUCCESS )
    {
        switch(msg->type)
        {
            case CMS_MSG_MCPD_RELOAD:
                MCPD_TRACE(MCPD_TRC_LOG, "received CMS_MSG_MCPD_RELOAD");

                /* Reload config file */
                pre_igmp_flood = mcpd_router.igmp_config.flood_enable;
#ifdef SUPPORT_MLD
                pre_mld_flood  = mcpd_router.mld_config.flood_enable;
                mcpd_config_read(&mcpd_router.igmp_config,
                                 &mcpd_router.mld_config);
#else
                mcpd_config_read(&mcpd_router.igmp_config, NULL);
#endif

#ifdef MCPD_DEBUG
                mcpd_config_display(&mcpd_router.igmp_config, NULL);
#endif
                mcpd_update_interface_info(msg->wordData);
                mcpd_process_query_timer(NULL); /* start general query timer */
                mcpd_update_multicast_info();

#ifdef SUPPORT_MLD
                mcpd_update_flooding_status(&pre_igmp_flood, &pre_mld_flood);                    
#else
                mcpd_update_flooding_status(&pre_igmp_flood, NULL);
#endif

            break;

            case CMS_MSG_MCPD_RESET:
                mcpd_reset_handler();
                break;

#if defined(DMP_X_BROADCOM_COM_GPON_1)
            case CMS_MSG_OMCI_MCPD_MIB_RESET:
                mcpd_reset_handler();
                mcpd_cms_send_reply(msg);
                break;
#endif

            case CMS_MSG_SET_LOG_LEVEL:
                cmsLog_setLevel(msg->wordData);
                if ((ret = cmsMsg_sendReply(mcpdMsgHandle, msg, CMSRET_SUCCESS)) != CMSRET_SUCCESS)
                {
                    cmsLog_error("send response for msg 0x%x failed, ret=%d", msg->type, ret);
                }
                break;

            case CMS_MSG_SET_LOG_DESTINATION:
                cmsLog_setDestination(msg->wordData);
                if ((ret = cmsMsg_sendReply(mcpdMsgHandle, msg, CMSRET_SUCCESS)) != CMSRET_SUCCESS)
                {
                    cmsLog_error("send response for msg 0x%x failed, ret=%d", msg->type, ret);
                }
                break;

            case CMS_MSG_TERMINATE:
            {
               CmsRet r2;
               r2 = cmsMsg_sendReply(mcpdMsgHandle, msg, CMSRET_SUCCESS);
               if (r2 != CMSRET_SUCCESS)
               {
                  cmsLog_error("MSG_TERMINATE response failed, ret=%d", r2);
               }
               mcpd_main_quit = 1;
               break;
            }

            default:
                MCPD_TRACE(MCPD_TRC_ERR, "unrecognized msg 0x%x", msg->type);
            break;
        }
        CMSMEM_FREE_BUF_AND_NULL_PTR(msg);
        ret = cmsMsg_receiveWithTimeout(mcpdMsgHandle, &msg, 0);
    }

    if(ret == CMSRET_DISCONNECTED)
    {
        if (!cmsFil_isFilePresent(SMD_SHUTDOWN_IN_PROGRESS))
        {
            MCPD_TRACE(MCPD_TRC_ERR, "lost connection to smd, exiting now.");
        }
        mcpd_main_quit = 1;
    }
    return;
} /* mcpd_cms_process_msg */


