/******************************************************************************
 *
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
 *
******************************************************************************/
#include <stdio.h>
#include "cms.h"

#include "xtm_trace.h"
#include "cms_actionlog.h"

#define __print(fmt, arg...)                                            \
    do {                                                                \
        calLog_library(fmt, ##arg);                                     \
    } while(0)

#define __trace(cmd, fmt, arg...)                                       \
    do {                                                                \
        if(cmsActionTraceEnable_g)                                      \
            __print(#cmd " " fmt, ##arg);                               \
        else                                                            \
            return CMSRET_SUCCESS;                                      \
    } while(0)


#ifdef CMS_ACTION_LOG
static int cmsActionTraceEnable_g = 1;
#else
static int cmsActionTraceEnable_g = 0;
#endif

CmsRet devCtl_xtmInitializeTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmInitialize");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmUninitializeTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmUninitialize");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmReInitializeTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmReInitialize");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmGetTrafficDescrTableTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmGetTrafficDescrTable");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmConfigTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmConfig");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmManageThresholdTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmManageThreshold");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmSetTrafficDescrTableTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmSetTrafficDescrTable");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmGetInterfaceCfgTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmGetInterfaceCfg");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmSetInterfaceCfgTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmSetInterfaceCfg");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmSetDsPtmBondingDeviationTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmSetDsPtmBondingDeviation");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmGetConnCfgTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmGetConnCfg");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmSetConnCfgTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmSetConnCfg");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmGetConnAddrsTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmGetConnAddrs");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmGetInterfaceStatisticsTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmGetInterfaceStatistics");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmGetErrorStatisticsTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmGetErrorStatistics");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmSetInterfaceLinkInfoTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmSetInterfaceLinkInfo");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmSendOamCellTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmSendOamCell");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmCreateNetworkDeviceTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmCreateNetworkDevice");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmDeleteNetworkDeviceTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmDeleteNetworkDevice");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_xtmGetBondingInfoTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_xtmGetBondingInfo");

    return CMSRET_SUCCESS;
}

CmsRet devCtl_atmSendOamLoopbackTestTrace(void)
{
    __trace(xtm_api, "%s\n", "devCtl_atmSendOamLoopbackTest");

    return CMSRET_SUCCESS;
}

