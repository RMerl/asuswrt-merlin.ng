/***********************************************************************
 *
 *  Copyright (c) 2016  Broadcom
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
*      BEEP  manifest header file.
*
*****************************************************************************/
#ifndef BEEP_MANIFEST_H
#define BEEP_MANIFEST_H

/* ---- Include Files ----------------------------------------------------- */

#include <stdbool.h>
#include "cms_params_modsw.h"
#include "beep.h"

/* download_pkginfo.h contains bcm generated hamc for hmac sha256 digest
 * and default pkg directory name and prefix and suffix
 */
#include "download_pkginfo.h"

/* ---- Constants and Types ----------------------------------------------- */

#define MANIFEST_NET_MODE_DISCONNECT    "Disconnect"
#define MANIFEST_NET_MODE_PRIMARY       "Primary"
#define MANIFEST_NET_MODE_SECONDARY     "Secondary"
#define MANIFEST_NET_MODE_WANONLY       "WanOnly"
#define MANIFEST_NET_MODE_LANONLY       "LanOnly"

#define EU_RUNLEVEL_MIN                0
#define AUTOSTART_ORDER_MIN            0
#define AUTOSTART_ORDER_MAX            1
#define RUNLEVEL_MIN                   -1 
#define RUNLEVEL_MAX                   65535
#define MAX_RESTARTS_MIN               0
#define MAX_RESTARTS_MAX               65535

#define DEFAULT_ENABLE_AFTER_INSTALL   TRUE
#define DEFAULT_AUTOSTART              FALSE
#define DEFAULT_AUTOSTART_ORDER        AUTOSTART_ORDER_MIN
#define DEFAULT_EU_RUNLEVEL            EU_RUNLEVEL_MIN
#define DEFAULT_RUNLEVEL               RUNLEVEL_MIN
#define DEFAULT_AUTO_RELAUNCH          FALSE
#define DEFAULT_MAX_RESTARTS           5
#define DEFAULT_RESTART_INTERVAL       2000
#define DEFAULT_SUCCESSFUL_START_PERIOD   3000


typedef enum
{
   APP_MEDIA_TYPE_EXECUTABLE  = 0,
   APP_MEDIA_TYPE_TARBALL,
   APP_MEDIA_TYPE_LAST,
} appMediaType;

typedef struct
{
  char appName[BEEP_APPNAME_LEN_MAX+1];       /**< app name */
  char appMediaType[BEEP_APPNAME_LEN_MAX+1];  /**< app media type (executiable or tarball) */
  char mngrAppName[BEEP_APPNAME_LEN_MAX+1];
  char appDigest[BEEP_KEY_LEN_MAX+1];         /**< application hmac sha256 digest */
  char maniDigest[BEEP_KEY_LEN_MAX+1];        /**< application manifest hmac sha256 digest */
} appInfo, *pAppInfo;

typedef struct
{
  char appName[BEEP_APPNAME_LEN_MAX+1];   /**< app name */
  char appStatus[32];              /**< app status */
  char appFullPath[BEEP_FULLPATH_LEN_MAX+1];         /**< app MDM full path */
  char appEuidBuf[64+1];           /**< Euid buffer */
  char mngrAppName[BEEP_APPNAME_LEN_MAX+1];  /**< For mediaType tarball, this is manager app instead of appName*/
  char username[BEEP_USERNAME_LEN_MAX+1]; /**< username of this app */
} updateAppInfo, *pUpdateAppInfo;

typedef struct
{
   char appStatus[32];              /**< app status */
   char appEuidBuf[64+1];           /**< Euid buffer */
} restoreAppInfo, *pRestoreAppInfo;

typedef enum
{
   ACT_BASIC,
   ACT_DEPENDENCY,
   ACT_CONTAINER,
   ACT_PRIVILEGE,
   ACT_MANIFEST_TO_MDM,
   ACT_DM_ACCESS
} ParseAppMan_t;

typedef struct
{
   char vendor[BEEP_PKG_VENDOR_LEN_MAX+1];
   char version[BEEP_PKG_BEEP_VER_LEN_MAX+1];
   char description[BEEP_PKG_DESCRIPTION_MAX+1];
   bool autoStart;
   int autoStartOrder;
   int runLevel;
   bool autoRelaunch;
   int maxRestarts;
   int restartInterval;
   int successfulStartPeriod;
} AppManBasic_t;

typedef struct
{
   int privileged;
   bool enableAfterInstall;
   int runLevel;
   char user[32];
   char appName[64]; //application name to start the service
   char library[1024];
   char networkMode[BEEP_NETWORKMODE_LEN_MAX];
   char networkBound[16];
   char exposedPorts[BEEP_EXPOSEDPORT_LEN_MAX];
   char wellknownName[BEEP_BUSNAME_LEN_MAX+1];
   ContProcess_t process;
   ContResource_t resource;
   ContDevicesList_t devices[CONT_DEVICES_MAX_ENTRIES];
   ContHooks_t hooks;
   ContSeccomp_t scmp;
} AppManContainer_t;

typedef enum
{
   BUSGATE_MEMBER_TYPE_METHOD  = 0,
   BUSGATE_MEMBER_TYPE_SIGNAL,
   BUSGATE_MEMBER_TYPE_PROPERTY,
} busgateMemberType;

/* ---- Function Prototypes ----------------------------------------------- */


/*****************************************************************************
*  FUNCTION:  beepDu_pkgManifestParse
*  DESCRIPTION:
*     Parse DU package manifest and return package information.
*  PARAMETERS:
*     @param pkgManifest   (IN) fullpathname to the package manifest.
*     @param eeName        (OUT) ee ame.
*     @param beepVersion   (OUT) ee version.
*     @param duName        (OUT) package name.
*     @param duVendor      (OUT) package vendor.
*     @param duVersion     (OUT) package version.
*     @param duDesc        (OUT) package description.
*     @param duAlias       (OUT) package alias.
*     @param duDependency  (OUT) package dpendency.
*     @param appInfoArray  (OUT) array of applications in package.
*     @param eeEid         (OUT) EID of application.
*     @param beepVersion   (OUT) beep version.
*  RETURNS:
*     SpRet: SPDRET_SUCCESS.
******************************************************************************
*/

SpdRet beepDu_pkgManifestParse(const char *pkgManifest,
                               char *eeName,
                               char *eeVersion,
                               char *duName,
                               char *duVendor,
                               char *duVersion,
                               char *duDesc,
                               char *duAlias,
                               char *duDependency,
                               pAppInfo appInfoArray,
                               int *eeEid,
                               char *beepVersion);


/*****************************************************************************
*  FUNCTION:  beepApp_manifestParse
*  DESCRIPTION:
*     Parse application manifest, and return application information.
*     Generate service keys and store them to database.
*  PARAMETERS:
*     @param manifest    (IN)  fullpathname to the application manifest.
*     @param type        (IN)  Parsing type
*     @param ptr         (OUT) Output structure
*  RETURNS:
*     SpdRet: SPDRET_SUCCESS.
******************************************************************************
*/
SpdRet beepApp_manifestParse(const char *manifest,
                             ParseAppMan_t type, void *ptr);


/*****************************************************************************
*  FUNCTION:  beepDu_getPkgManifestMediaType
*  DESCRIPTION:
*     Parse package manifest, and return appMediaType,
*  PARAMETERS:
*     @param manifest    (IN)  fullpathname to the package manifest.
*     @param type        (OUT) package manifest media type.
*     @param typeLen     (IN)  length of package manifest media type.
*  RETURNS:
*     SpdRet: SPDRET_SUCCESS.
******************************************************************************
*/
SpdRet beepDu_getPkgManifestMediaType(const char *pkgManifest,
                                      char *type, int typeLen);


#endif /* BEEP_MANIFEST_H */
