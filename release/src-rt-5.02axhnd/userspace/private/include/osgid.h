/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2011:proprietary:standard

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

#ifndef _OSGID_H_
#define _OSGID_H_

#include "cms_msg.h"
#include "cms_eid.h"

#define OSGI_NAME   "osgi4_2"
#define OSGI_TYPE   "OSGi Release 4.2"
#define OSGI_FELIX  "Apache Felix"
#define OSGI_FELIX_VERSION "Framework 3.0.1"

/*
 * OSGID_BUNDLE_LOCATION has been moved from "/usr/local/data"
 * to CMS_MODSW_OSGIEE_DU_DIR
 * see userspace/public/include/cms_params_modsw.h
 */

#define OSGID_WORK_DIR                "/tmp/osgi"
#define OSGID_FELIX_DIR               "/usr/local/felix"
#define OSGID_FELIX_EXEC              "bin/felix.jar"
#define OSGID_UNIX_SOCK "/tmp/osgi/interfacesock"  /* osgid unix domain name */
#define MAX_MSG_LEN  1024  /*max msg buf length between osgid and felix*/
#define TIMEOUT_RECV_FROM_FELIX   3000 /*timeout value for receive msg from felix*/
#define MAX_LINE_LEN 256
#define TIME_WAIT_FOR_FELIX_UP    60  /* 60 seconds */
#define RETRY_WAIT_FOR_FELIX_UP   4   
#define OSGID_LOCK_TIMEOUT  (60 * MSECS_IN_SEC)
#define MAX_UUID_LEN        36
#define MAX_ERROR_MSG_PRINTOUT 5
#define MAX_EU_LIST_FULL_PATH_LEN     1024
#define OSGID_MAX_LOG_MSG_LEN         128
#define JVM_DEFAULT_MIN_HEAP          4000  /* 4M */
#define JVM_DEFAULT_MAX_HEAP          10000  /* 10M */
#define JVM_DEFAULT_STACK_SIZE        256  /* 256k */
#define OSGID_FELIX_ACTIVATOR         "org.apache.felix.main.Main"
#define OSGID_FELIX_CP                "/usr/local/classpath/lib/classpath:/usr/local/classpath/share/classpath:/lib"

/* request list's individual request status */
#define REQUEST_ENTRY_PENDING        1
#define REQUEST_ENTRY_PROCESSING     2

/* request ID range for each application */
#define DU_CHANGE_REQ_ID_WEBUI_BASE           0        /* 1-100000 */
#define DU_CHANGE_REQ_ID_TR69C_BASE           100000   /* 100001-200000 */

typedef enum
{
   OSGID_UPDATE_ALL_DU=0,
   OSGID_UPDATE_ALL_MATCH_URL=1,
   OSGID_UPDATE_ONLY_UUID=2
} OSGI_UPDATE_ACTION;

typedef enum
{
   OSGID_REQUEST_NOT_START=0,
   OSGID_REQUEST_PROCESSING=1,
   OSGID_REQUEST_DONE=2
} OSGI_REQUEST_STATUS;

typedef struct lb_data
{
   int bundleId;
   char status[BUFLEN_16];
   int startLevel;
   char description[BUFLEN_64];
   char alias[BUFLEN_32];
   char version[BUFLEN_32];
   int faultCode;
   char errMsg[BUFLEN_256];
} *pLB_DATA, LB_DATA;

typedef struct request_data
{
   char operation[BUFLEN_32];   /**< Install, Update, Uninstall, install_at_bootup */
   char URL[BUFLEN_1024]; /**< Location of DU to be installed/update */
   char UUID[BUFLEN_40];    /**< Unique ID to describe DU,36 bytes */
   char username[BUFLEN_256]; /**< Optional username for file server */
   char password[BUFLEN_256]; /**< Optional password for file server */
   char executionEnv[BUFLEN_256]; /**< Environment to execute EU */
   char version[BUFLEN_32]; /**< Version of DU */
   char commandKey[BUFLEN_32]; /**< Command Key of op request */
   UINT16 reqId;
   char name[BUFLEN_32]; /**< Name of EU */
   char alias[BUFLEN_32]; /**< Alias Name of EU */
   char euid[BUFLEN_64]; /**< EUID of EU */
   OSGI_REQUEST_STATUS  requestStatus;
   CmsMsgType  type;  /**< specifies what message this is. */
   CmsEntityId src;  /**< CmsEntityId of the sender; so we can sent reply back to src*/ 
   int FaultCode;   /**< send back to src if exists */
   char ErrMsg[BUFLEN_256];
   char EUlist[BUFLEN_1024]; /**< list of comma seperated EU bundle ID */
   char DUlist[BUFLEN_256];  /**< list of comma seperated DU bundle ID */
} *pREQUEST_DATA, REQUEST_DATA;


#endif /* _OSGID_H_ */
