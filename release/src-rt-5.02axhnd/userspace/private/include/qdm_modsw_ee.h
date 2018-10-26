/***********************************************************************
 *
 *  Copyright (c) 2012  Broadcom Corporation
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

#ifndef _QDM_MODSW_EE_H_
#define _QDM_MODSW_EE_H_


/*!\file qdm_modsw_ee.h
 * \brief This file contains declarations for Modular Software
 *        Execution Environment query functions.
 *
 */

#include "cms.h"
#include "cms_eid.h"
#include "cms_obj.h"

/* Execution Environment query functions */

CmsRet qdmModsw_getExecEnvFullPathByNameLocked(const char *name,
                                        char *fullPathBuf, UINT32 bufLen);

CmsRet qdmModsw_getExecEnvFullPathByUuidLocked(const char *uuid,
                                        char *fullPathBuf, UINT32 bufLen);

CmsRet qdmModsw_getExecEnvFullPathByContainerNameLocked(const char *containerName,
                                        char *fullPathBuf, UINT32 bufLen);

CmsRet qdmModsw_getExecEnvFullPathByEidLocked(CmsEntityId mngrEid,
                                        char *fullPathBuf, UINT32 bufLen);

CmsRet qdmModsw_getContainerNameByEidLocked(CmsEntityId mngrEid,
                                        char *containerName, UINT32 len);

CmsRet qdmModsw_getMngrEidByExecEnvFullPathLocked(const char *fullPath,
                                              CmsEntityId *mngrEid);

CmsRet qdmModsw_getExecEnvNameByFullPathLocked(const char *fullPath,
                                        char *nameBuf, UINT32 nameBufLen);

CmsRet qdmModsw_getExecEnvNameByMngrEidLocked(CmsEntityId mngrEid,
                                              char *nameBuf, UINT32 nameBufLen);

CmsRet qdmModsw_getExecEnvEnableByFullPathLocked(const char *fullPath,
                                                 UBOOL8 *enable);

CmsRet qdmModsw_getExecEnvStatusByFullPathLocked(const char *fullPath,
                                                 char *statusBuf,
                                                 UINT32 statusBufLen);

CmsRet qdmModsw_getExecEnvObjectByFullPathLocked(const char *fullPath,
                                                 ExecEnvObject **eeObject,
                                                 InstanceIdStack *iidStack);


/* deployment unit query functions */

CmsRet qdmModsw_getDeployUnitFullPathByDuidLocked(const char *duid,
                                        char *fullPathBuf, UINT32 bufLen);

CmsRet qdmModsw_getDeployUnitNameByEuFullPathLocked(const char *euFullPath,
                                           char *duName, UINT32 duNameLen);

CmsRet qdmModsw_getDeployUnitStatusByUuidEeFullPathLocked(const char *uuid,
                                                          const char *eeFullPath,
                                                          char *status,
                                                          UINT32 statusLen);


/* Execution Unit query functions */

CmsRet qdmModsw_getExecUnitFullPathByEuidLocked(const char *euid,
                                        char *fullPathBuf, UINT32 bufLen);
CmsRet qdmModsw_getExecutionUnitParamsByEuFullPathLocked(const char *euFullPath,
                    char *euName, UINT32 euNameLen,
                    char *euid, UINT32 euidLen,
                    char *username, UINT32 usernameLen,
                    char *status, UINT32 statusLen,
                    char *mngrAppName, UINT32 mngrAppNameLen);
UBOOL8 qdmModsw_isDuResolvedByEuFullPathLocked(const char *euFullPath);
UBOOL8 qdmModsw_isDuResolvedByEuIidStackLocked(const InstanceIdStack *euIidStack);

CmsRet qdmModsw_getBeepPkgAppNamesLocked(const char *pkgName, char *pkgAppNames, UINT32 pkgAppNamesLen);

CmsRet qdmModsw_getBeepEuManifestInfoByEuidLocked(const char *euid,
                                                  char *network, UINT32 ntwkLen,
                                                  char *ports, int portsLen,
                                                  UINT32 *intfidx,
                                                  UINT32 *realtimeRuntime);
CmsRet qdmModsw_getBeepEuMacIdxLocked(const char *euid, UINT32 *idx);
CmsRet qdmModsw_getBeepEuNetworkInfoByEuidLocked(const char *euid, 
                      char *ntwkMode, int modeLen, char *ports, int portLen,
                      char *ipaddr, int addrLen);


CmsRet qdmModsw_getDuidByEuFullPathLocked (const char *euFullPath, 
                                           char *duid, 
                                           UINT32 duidLen);

CmsRet qdmModsw_getDuNameDuInstanceByEuFullPathLocked(const char *euFullPath,
                                                      char *duName,
                                                      UINT32 duNameLen,
                                                      UINT32 *duInstance);

CmsRet qdmModsw_getDuNameDuInstanceFromDuid(const char *duid,
                                            char *duName,
                                            UINT32 duNameLen,
                                            char *duInstString,
                                            UINT32 duInstStringLen);

CmsRet qdmModsw_getDuNameDuInstanceFromUuidVersionEeFullPathLocked(const char *uuid,
                                                                   const char *version,
                                                                   const char *eeFullPath,
                                                                   char *duName,
                                                                   UINT32 duNameLen,
                                                                   char *duInst,
                                                                   UINT32 duInstLen);
CmsRet qdmModsw_getEeDirByEuFullPathLocked(const char *euFullPath, int *flash,
                                           char *eeDirName, 
                                           UINT32 eeDirNameLen);

CmsRet qdmModsw_getEeDirByDuidLocked(const char *duid, int *flash,
                                     char *eeDirName, UINT32 eeDirNameLen);
                               
CmsRet qdmModsw_getEeNameByDuUuidLocked(const char *duUuid, 
                                        char *eeName, 
                                        UINT32 eeNameLen);

CmsRet qdmModsw_getEeNameByEuFullPathLocked(const char *euFullPath, 
                                            char *eeName,
                                            UINT32 eeNameLen);

CmsRet qdmModsw_getEeNameFlashByEuFullPathLocked(const char *euFullPath, 
                                                 int *flash,
                                                 char *eeName, 
                                                 UINT32 eeNameLen);

CmsRet qdmModsw_getEeDirByEeFullPathLocked(const char *eeFullPath, 
                                           char *eeDirName, 
                                           UINT32 eeDirNameLen);

CmsRet qdmModsw_getDuVendorStringByEuNameLocked(const char *euName, 
                                                char *duVendor, 
                                                UINT32 duVendorLen);
                                                    
CmsRet qdmModsw_getDuVendorStringFromDuid(const char *duUuid, char *duVendor, UINT32 duVendorLen);

CmsRet qdmModsw_getEeRunLevelByFullPathLocked(const char *eeFullPath,
                                              SINT32 *currentRunLevel,
                                              SINT32 *initialEURunLevel);

CmsRet qdmModsw_getEeRunLevelByEuidLocked(const char *euid,
                                          SINT32 *currentRunLevel,
                                          SINT32 *initialEURunLevel);

#endif /* _QDM_MODSW_EE_H_ */
