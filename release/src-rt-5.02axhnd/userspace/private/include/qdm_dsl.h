/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:proprietary:standard

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
#ifndef _QDM_DSL_H_
#define _QDM_DSL_H_


/*!\file qdm_dsl.h
 * \brief This file contains declarations for Modular Software
 *        Execution Environment query functions.
 *
 */

#include "cms.h"


#include "cms.h"
#include "cms_mdm.h"
#include "cms_obj.h"
#include "cms_phl.h"
#include "cms_qdm.h"
#include "cms_util.h"

/* return true is this channel is trained for ATM mode.  The lowerLayers is the lower layer of this channel */
UBOOL8 qdmDsl_isAtmConnectionLocked(const char *lowerLayers);

/* given a dsl line full path, find the active channel above it */
UBOOL8 qdmDsl_getChannelFullPathFromLineFullPathLocked(const char *lineFullPath, char **channelFullPath);
/* given a fast line full path, find the PTM channel above the equivalent dsl line */
UBOOL8 qdmDsl_getChannelFullPathFromFastLineFullPathLocked(const char *lineFullPath, char **channelFullPath);
/* get the modulation string from AdslMib.  Modulation is not available from the data model */
void qdmDsl_getModulationTypeStrLocked(int lineNumber, char *modeStr);

void qdmDsl_getPath1LineRateLocked_dev2(int *lineRate);
void qdmDsl_getPath1LineRateLocked_igd(int *lineRate);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmDsl_getPath1LineRateLocked(p)  qdmDsl_getPath1LineRateLocked_igd((p))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmDsl_getPath1LineRateLocked(p)  qdmDsl_getPath1LineRateLocked_igd((p))
#elif defined(SUPPORT_DM_PURE181)
#define qdmDsl_getPath1LineRateLocked(p)  qdmDsl_getPath1LineRateLocked_dev2((p))
#elif defined(SUPPORT_DM_DETECT)
#define qdmDsl_getPath1LineRateLocked(p)   (cmsMdm_isDataModelDevice2() ? \
                                            qdmDsl_getPath1LineRateLocked_dev2((p)) : \
                                            qdmDsl_getPath1LineRateLocked_igd((p)))
#endif

void qdmDsl_getDSLTrainedModeLocked(UBOOL8 *isVdsl, UBOOL8 *isAtm);
void qdmDsl_getDSLTrainedModeLocked_igd(UBOOL8 *isVdsl, UBOOL8 *isAtm);
void qdmDsl_getDSLTrainedModeLocked_dev2(UBOOL8 *isVdsl, UBOOL8 *isAtm);
#if defined(SUPPORT_DM_LEGACY98)
#define qdmDsl_getDSLTrainedModeLocked(a,b)  qdmDsl_getDSLTrainedModeLocked_igd((a),(b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmDsl_getDSLTrainedModeLocked(a,b)  qdmDsl_getDSLTrainedModeLocked_igd((a),(b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmDsl_getDSLTrainedModeLocked(a,b)  qdmDsl_getDSLTrainedModeLocked_dev2((a),(b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmDsl_getDSLTrainedModeLocked(a,b)  (cmsMdm_isDataModelDevice2() ? \
                                              qdmDsl_getDSLTrainedModeLocked_dev2((a),(b)) : \
                                              qdmDsl_getDSLTrainedModeLocked_igd((a),(b)))
#endif

UBOOL8 qdmDsl_isXdslLinkUpLocked(void);
UBOOL8 qdmDsl_isXdslLinkUpLocked_igd(void);
UBOOL8 qdmDsl_isXdslLinkUpLocked_dev2(void);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmDsl_isXdslLinkUpLocked()  qdmDsl_isXdslLinkUpLocked_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmDsl_isXdslLinkUpLocked()  qdmDsl_isXdslLinkUpLocked_igd()
#elif defined(SUPPORT_DM_PURE181)
#define qdmDsl_isXdslLinkUpLocked()  qdmDsl_isXdslLinkUpLocked_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmDsl_isXdslLinkUpLocked()  (cmsMdm_isDataModelDevice2() ? \
                                      qdmDsl_isXdslLinkUpLocked_dev2() : \
                                      qdmDsl_isXdslLinkUpLocked_igd())
#endif

void qdmDsl_isDslBondingEnabled(UBOOL8 *enabled);
void qdmDsl_isDslBondingEnabled_igd(UBOOL8 *enabled);
void qdmDsl_isDslBondingEnabled_dev2(UBOOL8 *enabled);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmDsl_isDslBondingEnabled(e)  qdmDsl_isDslBondingEnabled_igd((e))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmDsl_isDslBondingEnabled(e)  qdmDsl_isDslBondingEnabled_igd((e))
#elif defined(SUPPORT_DM_PURE181)
#define qdmDsl_isDslBondingEnabled(e)  qdmDsl_isDslBondingEnabled_dev2((e))
#elif defined(SUPPORT_DM_DETECT)
#define qdmDsl_isDslBondingEnabled(e)  (cmsMdm_isDataModelDevice2() ? \
                                        qdmDsl_isDslBondingEnabled_dev2((e)) : \
                                        qdmDsl_isDslBondingEnabled_igd((e)))
#endif

void qdmDsl_isDslBondingGroupStatusOperational_dev2(char *scheme,UBOOL8 *operational);
UBOOL8 qdmDsl_isAnyLowerLayerChannelUpLocked_dev2(char *lowerLayers);
CmsRet qdmDsl_getLineIdFromChannelFullPathLocked_dev2(const char *fullPath,UINT32 *lineId);

#endif /* _QDM_DSL_H_ */
