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

#ifndef _MODSW_H_
#define _MODSW_H_


/*!\file modsw.h
 * \brief This file contains declarations and definitions associated with
 * libmodsw.so in userspace/private/lib/modsw
 *
 */

#include "cms.h"
#include "cms_image.h"


/** Make the directories needed for modular software operations.
 *  It is ok if the directories already exist.
 *
 *  @return CmsRet
 */
CmsRet modsw_makeRequiredDirs(void);


/*
 * Function get file from server using transport type http, https, ftp
 * url: point to server address, URL contains name of file
 * username: optional username for server authentication. NULL is none.
 * password: optional password for server authentication. NULL is none.
 * destdir: place to store retrieved file, i.e. /usr/local/data.
 * XXX miwang: Should use a structure to track the download state.  Good enough for now.
 */
CmsRet modswDld_startDownload(const char *url,
                              const char *username, const char *password,
                              char *destdir);


/** Unpack a LinuxEE or LinuxPFP package.
 *
 * @param fullpath (IN) fullpathname to the linux package.
 * @param imageFormat (OUT) image format of the linux package.
 * @param header2 (OUT) string buffer of length CMS_MODSW_LINUX_HEADER2_LEN+1
 *         The contents of header2 in the bcmpkg will be stored here.
 *         If imageFormat is MODSW_LINUXEE_DU, this is the Deployment Unit
 *         (DU) name.  If imageFormat is MODSW_LINUXPFP, this is the patch name.
 * @param pkgVersion (OUT) string buffer of TR157_VERSION_STR_LENGTH+1 bytes.
 *          This is the version of the DU or patch.
 * @param pkgDesc (OUT) string buffer of TR157_DESCRIPTION_STR_LENGTH+1 bytes.
 *          This is the description of the DU or patch.
 * @param innerFilename (OUT) string buffer of  BCMPKG_INNERFILENAME_MAX_LEN+1
 *          bytes to hold the inner filename.
 *          This is only used if the imageFormat is MODSW_LINUXEE_DU, in which
 *          case this inner filename is the Execution Unit (EU) name.
 * @param innerVersion (OUT) string buffer of TR157_VERSION_STR_LENGTH+1 bytes.
 *          This is only used if the imageFormat is MODSW_LINUXEE_DU, in which
 *          case this is the EU version.
 * @param innerVersion (OUT) string buffer of TR157_DESCRIPTION_STR_LENGTH+1 bytes.
 *          This is only used if the imageFormat is MODSW_LINUXEE_DU, in which
 *          case this is the EU description.
 *
 * @return CmsRet
 */
CmsRet modsw_unpackLinuxPkg(const char *fullpath,
                      CmsImageFormat *imageFormat,
                      char *header2, char *pkgVersion, char *pkgDesc,
                      char *innerName, char *innerVersion, char *innerDesc);

CmsRet modsw_unpackOuterTarball(CmsImageFormat imageFormat,
          const char *header1, const char *pkgName, const char *fullpath,
          char *pkgVersion, char *pkgDesc,
          char *innerName, char *innerVersion, char *innerDesc);

CmsRet modsw_processOuter(const char *destDir, const char *tmpDir,
                   const char *header1, const char *header2,
                   char *pkgVersion, char *pkgDesc,
                   char *innerFilename, char *innerVersion, char *innerDesc);

CmsRet modsw_processOuterHeader(const char *destDir, const char *header1,
                                const char *header2, char *innerName);

CmsRet modsw_processInnerHeader(const char *destDir,
                                char *pkgVersion, char *pkgDesc,
                                char *innerVersion, char *innerDesc);

CmsRet modsw_createInnerChkSumFile(const char *header1, const char *header2,
        const char *filename, const char *chkSumFilename);

CmsRet modsw_calculateSha1Checksum(const char *filename, char *chkSumBuf);

void modsw_deleteLinuxPkg(const char *topDir, const char *innerName);


/** Enable/disable an Exec Env */
void modsw_setExecEnvEnableLocked(const char *name, UBOOL8 enable);

/** Set Exec Env Status */
void modsw_setExecEnvStatusLocked(const char *name, const char *status);
void modsw_setExecEnvStatus(const char *name, const char *status);


/** Add a Deployment Unit object in the Data Model.
 *
 */
CmsRet modsw_addDuEntry(const char *uuid, const char *duid, const char *url,
                        const char *execEnvPath);


void modsw_deleteDuEntryLocked(const char *uuid, const char *version);

/** Set the status of the specified DU
 *
 */
void modsw_setDuStatusLocked(const char *uuid, const char *versionStr, const char *duStatus);

void modsw_setDuStatus(const char *uuid, const char *versionStr, const char *duStatus);


void modsw_setDuInfo(const char *uuid, const char *versionStr,
              const char *name, const char *alias, const char *version, const char *description);

void modsw_getDuNameLocked(const char *uuid, const char *versionStr,
                           char *name, UINT32 nameLen);

void modsw_setDuResolved(const char *uuid, const char *versionStr, UBOOL8 resolved);

void modsw_setDuResolvedLocked(const char *uuid, const char *versionStr, UBOOL8 resolved);

void modsw_addEuPathToDuLocked(const char *uuid, const char *version, const char *euFullPath);


CmsRet modsw_getEuPathFromDuLocked(const char *uuid, const char *version, char *euFullPath, UINT32 euFullPathLen);


/*** EU related functions */

CmsRet modsw_addEuEntryLocked(const char *euId, const char *alias, const char *name,
                        const char *execEnvLabel, const char *execEnvRef,
                        const char *vendor, const char *version,
                        const char *description, UINT32 bundleId,
                        char *euFullPath, UINT32 euFullPathLen);

void modsw_getEuNameLocked(const char *euFullPath, char *name, UINT32 nameLen);

UBOOL8 modsw_isEuAutoStartLocked(const char *euFullPath);

void modsw_setEuAutoStartLocked(const char *euFullPath, UBOOL8 autoStart);

void modsw_setEuStatusLocked(const char *euFullPath, const char *status);

void modsw_setEuRequestedStateLocked(const char *euFullPath, const char *requestedState);

void modsw_deleteEuEntryLocked(const char *euFullPath);

#endif /* _MODSW_H_ */
