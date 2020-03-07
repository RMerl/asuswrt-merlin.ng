/*
* <:copyright-BRCM:2015:proprietary:standard
* 
*    Copyright (c) 2015 Broadcom 
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
* :>
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <sys/types.h>
#include <sys/stat.h>
#include "devctl_adsl.h"
#include "SeltBldProfileDep.h"

#ifdef SUPPORT_SELT
#define SELT_FILE_SYSTEM_SUPPORT

#ifdef SUPPORT_DSL_BONDING
#define MAX_DSL_LINE    2
#else
#define MAX_DSL_LINE    1
#endif

#define POSTPROCESSING_INPUT_CHIP_6368         1
#define POSTPROCESSING_INPUT_CHIP_63268        2
#define POSTPROCESSING_INPUT_CHIP_63138        3

int xdslSeltGetMaxLine(void)
{
   return MAX_DSL_LINE;
}

char xdslSeltGetPostProcessingChip(void)
{
   char chip = 0;
#if defined(CHIP_63138)  || defined(CHIP_63148)
   chip = POSTPROCESSING_INPUT_CHIP_63138;
#endif
#ifdef CHIP_63268
   chip = POSTPROCESSING_INPUT_CHIP_63268;
#endif
   return chip;
}

CmsRet xdslSeltGetAfeIdAndSeltData(unsigned char lineId, unsigned int *pAfeId, SeltData *pSeltData, long *pSeltDataLen)
{
   CmsRet      cmsRet;
   adslMibInfo adslMib;
   long        size = sizeof(adslMib);
   
   cmsRet = xdslCtl_GetObjectValue(lineId, NULL, 0, (char *)&adslMib, &size);
   if (cmsRet != CMSRET_SUCCESS) {
      //cmsLog_error("%s: xdslCtl_GetObjectValue error\n", __FUNCTION__ );
      return (CMSRET_INTERNAL_ERROR);
   }
   memcpy((void *)pSeltData, (void *)&adslMib.selt, *pSeltDataLen);
#if (ADSLMIBDEF_H_VER <= 2)
   /* With older ADSLMIBDEF, afeId of line 1 needs to be retrieved line 0 */
   if (1 == lineId) {
      size = sizeof(adslMib);
      cmsRet = xdslCtl_GetObjectValue(0, NULL, 0, (char *)&adslMib, &size);
      if (cmsRet != CMSRET_SUCCESS) {
         //cmsLog_error("%s: xdslCtl_GetObjectValue error\n", __FUNCTION__ );
         return (CMSRET_INTERNAL_ERROR);
      }
      *pAfeId = adslMib.afeId[lineId];
   }
   else
      *pAfeId = adslMib.afeId[lineId];
#else
   *pAfeId = adslMib.afeId[lineId];
#endif

   return cmsRet;
}

CmsRet xdslSeltGetSeltData(unsigned char lineId, SeltData *pSeltData, long *pSeltDataLen)
{
   char  oid[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslPrivGetSeltData};
   
   return xdslCtl_GetObjectValue(lineId, oid, sizeof(oid), (char*)pSeltData, pSeltDataLen);
}

CmsRet xdslSeltGetUER(unsigned char lineId, int *pData, long *pDataLen)
{
   char  oidStr[]  = { kOidAdslPrivate, kOidAdslPrivUER };
   
   return xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)pData, pDataLen);
}

CmsRet xdslSeltUpdateSeltData(unsigned char lineId, SeltData *pSeltData, long *pSeltDataLen)
{
   char  oid[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslPrivSetSeltData};
   
   //cmsLog_debug("%s: new state = 0x%lx, steps=0x%x\n",__FUNCTION__,pSeltData->seltState, pSeltData->seltSteps);
   return xdslCtl_SetObjectValue(lineId, oid, sizeof(oid), (char*)pSeltData, pSeltDataLen);
}

CmsRet xdslSeltSetSeltNextMode(unsigned char lineId)
{
   return xdslCtl_SetTestMode(lineId, ADSL_TEST_NEXT_SELT);
}

#endif   /* SUPPORT_SELT */
