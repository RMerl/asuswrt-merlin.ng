/***********************************************************************
 *
 *  Copyright (c) 2016  Broadcom Corporation
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

/*****************************************************************************
*    Description:
*
*      BEEP common header file included by various server.
*
*****************************************************************************/
#ifndef BEEP_COMMON_H
#define BEEP_COMMON_H

/* ---- Include Files ----------------------------------------------------- */
#include "beep_log.h"
#include "beep.h"
#include "cms_params_modsw.h"

/* ---- Constants and Types ----------------------------------------------- */

#define DBUS_DIR              "/dbus-1"
#define BEEP_DBUS_DIR         CMS_DATA_STORAGE_DIR DBUS_DIR

#define DBUS_POLICY_DIR       "/dbus-1/system.d"
#define BEEP_DBUS_POLICY_DIR  CMS_DATA_STORAGE_DIR DBUS_POLICY_DIR

#define BUSGATE_DIR           "/busgate"
#define BEEP_BUSGATE_DIR      CMS_DATA_STORAGE_DIR BUSGATE_DIR

#define BUSGATE_MANIFEST_FILE "busgateconf"

#define BUSGATE_PASSWD_FILE   BUSGATE_DIR"/passwd"
#define BUSGATE_GROUP_FILE    BUSGATE_DIR"/group"
#define BEEP_PASSWD_FILE      BEEP_BUSGATE_DIR"/passwd"
#define BEEP_GROUP_FILE       BEEP_BUSGATE_DIR"/group"

#define BBCD_DIR              "/bbcd"
#define BEEP_BBCD_DIR         CMS_DATA_STORAGE_DIR BBCD_DIR

#define DB_FILE_NAME          "/beepdb"
#define BEEP_DB_FILE          BEEP_PERSISTENT_STORAGE_DIR DB_FILE_NAME


#define BEEPLIB_DEBUG_ENABLE 0

#if BEEPLIB_DEBUG_ENABLE
#define beepLog_error(fmt, arg...) \
  fprintf(stderr, "ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define beepLog_debug(fmt, arg...) \
  fprintf(stderr, "DEBUG[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
#define beepLog_error(fmt, arg...) \
  fprintf(stderr, "ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define beepLog_debug(args...)
#endif


SpdRet beepUtil_errcodeToSpdRet(int errcode);
int beepUtil_strverscmp(const char *v1, const char *v2);
void beepParam_getDataDirName(const char **dirName);
void beepParam_getPersistentDirName(const char **dirName);

#endif /* BEEP_COMMON_H */
