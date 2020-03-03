/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
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
 *
 * $Change: 111969 $
 ***********************************************************************/

/*
 * CMS Utils
 */
#if defined(BRCM_CMS_BUILD)

#include "cms.h"
#include "cms_mdm.h"
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"
#include "cms_mem.h"
#include "cms_msg.h"
#include "cms_util.h"
#include "board.h"
#include "ieee1905_socket.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_trace.h"
#include "ieee1905_cmsutil.h"
#include "ieee1905_cmsmdm.h"

#define I5_TRACE_MODULE    i5TraceCmsUtil

void *cmsMsgHandle=NULL; /* handle to communications link to cms */

int i5CmsutilGet1901MacAddress(unsigned char *MACAddress)
{
  if (devCtl_getMacAddress(MACAddress, MAC_ADDRESS_PLC) != CMSRET_SUCCESS) {
     printf("%s() devCtl_getMacAddress failed\n", __FUNCTION__);
     return -1;
  }
  return 0;
}

int i5CmsutilGet1905MacAddress(unsigned char *MACAddress)
{
  if (devCtl_getMacAddress(MACAddress, MAC_ADDRESS_1905) != CMSRET_SUCCESS) {
     printf("%s() devCtl_getMacAddress failed\n", __FUNCTION__);
     return -1;
  }
  return 0;
}

int i5CmsUtilGetFriendlyName( unsigned char *deviceId, char *pFriendlyName, int maxLen )
{
#if defined(SUPPORT_IEEE1905_GOLDENNODE)
   snprintf(pFriendlyName, maxLen, "REG-%02X%02X%02X", deviceId[3], deviceId[4], deviceId[5]);
#else
   char boardId[BUFLEN_64] = {0};

   if ( devCtl_boardIoctl(BOARD_IOCTL_GET_ID, 0, boardId, maxLen, 0, NULL) == CMSRET_SUCCESS )
   {
       if ((strcmp(boardId, "96319PLC") == 0) ||
           (strncmp(boardId, "960333", 6) == 0) ||
           (strncmp(boardId, "960500", 6) == 0) )
       {
           snprintf(pFriendlyName, maxLen, "WRE-%02X%02X%02X", deviceId[3], deviceId[4], deviceId[5]);
       }
       else
       {
           snprintf(pFriendlyName, maxLen, "GW-%02X%02X%02X", deviceId[3], deviceId[4], deviceId[5]);
       }
   }
   else
   {
       snprintf(pFriendlyName, maxLen, "%02X%02X%02X%02X%02X%02X",
                    deviceId[0], deviceId[1], deviceId[2], deviceId[3], deviceId[4], deviceId[5]);
   }
#endif // endif
   return 0;
}

int i5CmsutilInit()
{
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
    SINT32       shmId = UNINITIALIZED_SHM_ID;
    CmsMsgHeader msg = { 0 };
#endif // endif
    CmsRet       ret;

    i5Trace("\n");
    ret = cmsMsg_initWithFlags(EID_1905, 0, &cmsMsgHandle);
    if (ret != CMSRET_SUCCESS) {
        i5TraceError("%s() cmsMsg_initWithFlags failed\n", __FUNCTION__);
        return -1;
    }

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
    msg.src = EID_1905;
    msg.dst = EID_SMD;
    msg.type = CMS_MSG_GET_SHMID;
    msg.flags_request = 1;
    msg.dataLength = 0;
    shmId = (SINT32)cmsMsg_sendAndGetReplyWithTimeout(cmsMsgHandle, &msg, 6000);
    if (shmId == (SINT32)CMSRET_TIMED_OUT)  /* assumes shmId is never 9809, which is value of CMSRET_TIMED_OUT */
    {
        i5TraceError("unable to read shmId from smd (shmId=%d)\n", shmId);
        cmsMsg_cleanup(&cmsMsgHandle);
        cmsMsgHandle = NULL;
        return -1;
    }

    if (cmsMdm_initWithAcc(EID_BMUD, 0, cmsMsgHandle, &shmId) != CMSRET_SUCCESS)
    {
        printf("cmsMdm_init failed");
        cmsMsg_cleanup(&cmsMsgHandle);
        return -1;
    }

    /* initialize TR181 data model objects */
    return i5CmsMdmInit();
#else
    return 0;
#endif // endif

}

void i5CmsutilDeinit()
{
    if ( cmsMsgHandle )
    {
        cmsMsg_cleanup(&cmsMsgHandle);
    }

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
    cmsMdm_cleanup();
#endif // endif
}
#endif // endif
