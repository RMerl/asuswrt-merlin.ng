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

#ifndef _QDM_TR69C_H_
#define _QDM_TR69C_H_


/*!\file qdm_tr69c.h
 * \brief This file contains declarations for Modular Software
 *        Execution Environment query functions.
 *
 */

#include "cms.h"

typedef struct
{
   UBOOL8 enable;
   char *URL;
   char *lastConnectedURL;
   char *username;
   char *password;
   UBOOL8 periodicInformEnable;
   UINT32 periodicInformInterval;
   char *parameterKey;
   char *connectionRequestURL;
   char *connectionRequestUsername;
   char *connectionRequestPassword;
   UINT32 CWMPRetryMinimumWaitInterval;
   UINT32 CWMPRetryIntervalMultiplier;
} Tr69cCfg;


#if defined(DMP_BASELINE_1) && defined(DMP_DEVICE2_BASELINE_1)

#define qdmtr69c_getManagementServerCfg(p)   do { \
   if (mdmLibCtx.dataModel == 181)                \
      qdmtr69c_getManagementServerCfg_dev2(p);    \
   else                                           \
      qdmtr69c_getManagementServerCfg_igd(p);     \
   }while (0);

#elif defined(DMP_DEVICE2_BASELINE_1)

#define qdmtr69c_getManagementServerCfg(p) qdmtr69c_getManagementServerCfg_dev2(p)

#else
/* tr98 */
#define qdmtr69c_getManagementServerCfg(p) qdmtr69c_getManagementServerCfg_igd(p)

#endif /* hybrid, TR181 or TR98 */



/** Get the boundIfName of the TR69 Management server object.
 *
 * @param ifName  (OUT)  A buffer of at least CMS_IFNAME_LENGTH bytes long.
 *                       On successful return, will contain the bound ifName.
 *                       Note it could also be MDMVS_ANYWAN or MDMVS_ANYLAN.
 *
 * @return CmsRet enum.
 */
CmsRet qdmTr69c_getBoundIfNameLocked(char *ifName);

CmsRet qdmTr69c_getBoundIfNameLocked_igd(char *ifName);

CmsRet qdmTr69c_getBoundIfNameLocked_dev2(char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmTr69c_getBoundIfNameLocked(i)   qdmTr69c_getBoundIfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmTr69c_getBoundIfNameLocked(i)   qdmTr69c_getBoundIfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmTr69c_getBoundIfNameLocked(i)   qdmTr69c_getBoundIfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmTr69c_getBoundIfNameLocked(i)       (cmsMdm_isDataModelDevice2() ? \
                                     qdmTr69c_getBoundIfNameLocked_dev2((i)) : \
                                     qdmTr69c_getBoundIfNameLocked_igd((i)))
#endif

/** Get the autonomous transfer policy of the TR69 Management server object.
 *
 * @param enable  (OUT)  A pointer to boolean variable in which the policy mode is returned (enable=TRUE/disable=FALSE)
 * @param fileTypeFilter (OUT)  A pointer of string buffer large enough to hold returned file type string of either "Upload", "Download" or "Both"
 * @param resultTypeFilter (OUT)  A pointer of string buffer large enough to hold returned file type string of either "Success", "Failure" or "Both"
 * @param transferTypeFilter (OUT)  A pointer of string buffer 1024 bytes long to hold returned returned file type filter (comma separated file type string)
 * @return CmsRet enum.
 */
CmsRet qdmTr69c_getAutonXferCompletePolicyLocked(UBOOL8 *enable, char *fileTypeFilter, char *resultTypeFilter, 
                                           char *transferTypeFilter);
CmsRet qdmTr69c_getAutonXferCompletePolicyLocked_igd(UBOOL8 *enable, char *fileTypeFilter, char *resultTypeFilter, 
                                               char *transferTypeFilter);
CmsRet qdmTr69c_getAutonXferCompletePolicyLocked_dev2(UBOOL8 *enable, char *fileTypeFilter, char *resultTypeFilter, 
                                                char *transferTypeFilter);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmTr69c_getAutonXferCompletePolicyLocked(a,b,c,d) qdmTr69c_getAutonXferCompletePolicyLocked_igd((a),(b),(c),(d))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmTr69c_getAutonXferCompletePolicyLocked(a,b,c,d) qdmTr69c_getAutonXferCompletePolicyLocked_igd((a),(b),(c),(d))
#elif defined(SUPPORT_DM_PURE181)
#define qdmTr69c_getAutonXferCompletePolicyLocked(a,b,c,d) qdmTr69c_getAutonXferCompletePolicyLocked_dev2((a),(b),(c),(d))
#elif defined(SUPPORT_DM_DETECT)
#define qdmTr69c_getAutonXferCompletePolicyLocked(a,b,c,d) (cmsMdm_isDataModelDevice2() ? \
                                                      qdmTr69c_getAutonXferCompletePolicyLocked_dev2((a),(b),(c),(d)) : \
                                                      qdmTr69c_getAutonXferCompletePolicyLocked_igd((a),(b),(c),(d)))
#endif

/** Get the DU State Change policy of the TR69 Management server object.
 *
 * @param enable  (OUT)  A pointer to boolean variable in which the policy mode is returned (enable=TRUE/disable=FALSE)
 * @param opTypeFilter (OUT)  A pointer of string buffer large enough to hold returned comma separate list of string "Install", "Update" or "Uninstall"
 * @param resultTypeFilter (OUT)  A pointer of string buffer large enough to hold returned file type string of either "Success", "Failure" or "Both"
 * @param FaultCodeFilter (OUT)  A pointer of string buffer 256 bytes long to hold returned returned fault code filter list (comma separated error code strings, i.e. 9001, 9012)
 * @return CmsRet enum.
 */
CmsRet qdmTr69c_getDuStateChangePolicyLocked(UBOOL8 *enable, char *opTypeFilter, char *resultTypeFilter, 
                                             char *faultCodeFilter);
CmsRet qdmTr69c_getDuStateChangePolicyLocked_igd(UBOOL8 *enable, char *opTypeFilter, char *resultTypeFilter, 
                                                 char *faultCodeFilter);
CmsRet qdmTr69c_getDuStateChangePolicyLocked_dev2(UBOOL8 *enable, char *opTypeFilter, char *resultTypeFilter, 
                                                  char *faultCodeFilter);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmTr69c_getDuStateChangePolicyLocked(a,b,c,d) qdmTr69c_getDuStateChangePolicyLocked_igd((a),(b),(c),(d))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmTr69c_getDuStateChangePolicyLocked(a,b,c,d) qdmTr69c_getDuStateChangePolicyLocked_igd((a),(b),(c),(d))
#elif defined(SUPPORT_DM_PURE181)
#define qdmTr69c_getDuStateChangePolicyLocked(a,b,c,d) qdmTr69c_getDuStateChangePolicyLocked_dev2((a),(b),(c),(d))
#elif defined(SUPPORT_DM_DETECT)
#define qdmTr69c_getDuStateChangePolicyLocked(a,b,c,d) (cmsMdm_isDataModelDevice2() ? \
                                                        qdmTr69c_getDuStateChangePolicyLocked_dev2((a),(b),(c),(d)) : \
                                                        qdmTr69c_getDuStateChangePolicyLocked_igd((a),(b),(c),(d)))
#endif

/** Get some parameters from Management server configuration object.
 *
 * @param url  (OUT)  A pointer to string buffer (256 characters) to hold the ACS URL
 * @param connReqUser (OUT)  A pointer of string buffer (256 characters) to hold the connection requuest username
 * @param connReqPwd (OUT)  A pointer of string buffer (256 characters) to hold the connection requuest username
 * @param user (OUT)  A pointer of string buffer (256 characters) to hold the connection username
 * @param pwd (OUT)  A pointer of string buffer (256 characters) to hold the connection password
 * @return CmsRet enum.
 */

CmsRet qdmTr69c_getManagementServerCfgLocked(char *url, char *connReqUser, char *connReqPwd, char *user, char *pwd);
CmsRet qdmTr69c_getManagementServerCfgLocked_igd(char *url, char *connReqUser, char *connReqPwd, char *user, char *pwd);
CmsRet qdmTr69c_getManagementServerCfgLocked_dev2(char *url, char *connReqUser, char *connReqPwd, char *user, char *pwd);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmTr69c_getManagementServerCfgLocked(a,b,c,d,e)    qdmTr69c_getManagementServerCfgLocked_igd((a),(b),(c),(d),(e))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmTr69c_getManagementServerCfgLocked(a,b,c,d,e)    qdmTr69c_getManagementServerCfgLocked_igd((a),(b),(c),(d),(e))
#elif defined(SUPPORT_DM_PURE181)
#define qdmTr69c_getManagementServerCfgLocked(a,b,c,d,e)    qdmTr69c_getManagementServerCfgLocked_dev2((a),(b),(c),(d),(e))
#elif defined(SUPPORT_DM_DETECT)
#define qdmTr69c_getManagementServerCfgLocked(a,b,c,d,e)   (cmsMdm_isDataModelDevice2() ? \
                                                            qdmTr69c_getManagementServerCfgLocked_dev2((a),(b),(c),(d),(e)): \
                                                            qdmTr69c_getManagementServerCfgLocked_igd((a),(b),(c),(d),(e)))
#endif

/** Get XMPP related parameters from Management server configuration object.
 *
 * @param connectionPath  (OUT)  A pointer to string buffer (256 characters) to hold the ACS URL
 * @param allowedJabberIds (OUT)  A pointer of string buffer (8192 characters) to hold the connection requuest username
 * @param connReqJabberId (OUT)  A pointer of string buffer (256 characters) to hold the connection requuest username
 * @return CmsRet enum.
 */
CmsRet qdmTr69c_getManagementServerXmppCfgLocked_dev2(char *connectionPath, char *allowedJabberIds, char *connReqJabberId);
CmsRet qdmTr69c_getManagementServerXmppCfgLocked_igd(char *connectionPath, char *allowedJabberIds, char *connReqJabberId);
CmsRet qdmTr69c_getManagementServerXmppCfgLocked(char *connectionPath, char *allowedJabberIds, char *connReqJabberId);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmTr69c_getManagementServerXmppCfgLocked(a,b,c)    qdmTr69c_getManagementServerXmppCfgLocked_igd((a),(b),(c))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmTr69c_getManagementServerXmppCfgLocked(a,b,c)    qdmTr69c_getManagementServerXmppCfgLocked_igd((a),(b),(c))
#elif defined(SUPPORT_DM_PURE181)
#define qdmTr69c_getManagementServerXmppCfgLocked(a,b,c)    qdmTr69c_getManagementServerXmppCfgLocked_dev2((a),(b),(c))
#elif defined(SUPPORT_DM_DETECT)
#define qdmTr69c_getManagementServerXmppCfgLocked(a,b,c)   (cmsMdm_isDataModelDevice2() ? \
                                                            qdmTr69c_getManagementServerXmppCfgLocked_dev2((a),(b),(c)): \
                                                            qdmTr69c_getManagementServerXmppCfgLocked_igd((a),(b),(c)))
#endif

#endif /* _QDM_TR69C_H_ */



