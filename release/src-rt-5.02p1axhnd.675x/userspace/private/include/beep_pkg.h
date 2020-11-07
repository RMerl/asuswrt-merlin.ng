/***********************************************************************
 *
 *  Copyright (c) 2017  Broadcom
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

#ifndef BEEP_PKG_H
#define BEEP_PKG_H

#include <stdbool.h>
#include "beep.h"
#include "download_pkginfo.h"

/* len_of_prefix + hash_string_size
 * example:
 * hmac-sha256:2a038fd09e138096b1eb19ea8bceee1df82829cdbe3f09a4c907baa62ac5f89b
 */
 
#define BEEP_BDDT_VER                "1.0" /* Brcm Digital Digest Tag version */
#define BEEP_BDDT_VER_LEN            8
#define BEEP_BDDT_PAYLOAD_LEN        32
#define BEEP_BDDT_ALGORITHM_LEN      20 
#define BEEP_BDDT_VENDOR_LEN         32
#define BEEP_BDDT_TAG_LEN            128   /* Total BDDT Length */
#define BEEP_BDDT_TOKEN_LEN          20
#define BEEP_BDDT_RESERVED_LEN       (BEEP_BDDT_TAG_LEN - BEEP_BDDT_VER_LEN - BEEP_BDDT_PAYLOAD_LEN - BEEP_BDDT_ALGORITHM_LEN - BEEP_BDDT_VENDOR_LEN - BEEP_BDDT_TOKEN_LEN)

/* Broadcom Digital Digest Tag structure */
typedef struct
{
   char bddtVersion[BEEP_BDDT_VER_LEN];    /* Brcm Digital Digest Tag version */
   char bddtPayloadLen[BEEP_BDDT_PAYLOAD_LEN];  /* payload(tarball mostly)len */
   char bddtAlgorithm[BEEP_BDDT_ALGORITHM_LEN]; /* Brcm Digital Digest algorithm used for payload */
   char bddtVendor[BEEP_BDDT_VENDOR_LEN];       /* Vendor name string */
   char reserved[BEEP_BDDT_RESERVED_LEN];       /* reserved for later use */
   char tagValidationToken[BEEP_BDDT_TOKEN_LEN];/* validation token for tag(from bddtVersion to the end of reserved)*/
} BDDT, *pBDDT;


/* 
 * spd package download timeout
 * Note: The values assume DBus call timeout is set with default 25 sec
 */
#define BEEP_DOWNLOAD_ESTABLISH_TIMEOUT 5L
#define BEEP_DOWNLOAD_CONNECTION_TIMEOUT 15L

SpdRet beepPkg_startDownload(const char *url, const char *user,
                             const char *passwd, const char *destDir);

SpdRet beepPkg_getTmpPkgManifestAndTarBall(const char *fullpath,
                                      char *tmpTarballDir, int tmpTarballDirLen,
                                      char *tmpPkgManifest, int maniBufLen);
bool beepPkg_isBeepFileValid(const char *inFile, const char *vendorString,
                             algType aType, const char *inDigestString);
bool beepPkg_getVallidAlgTypeFromDigest(const char *inDigestString,
                                        algType *aType);
#endif /* #ifndef BEEP_PKG_H */
