/***********************************************************************
 *
 * Copyright (c) 2006-2007  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2006-2007:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#ifndef __CMS_MSG_MODSW_H__
#define __CMS_MSG_MODSW_H__

#include "cms.h"
#include "cms_msg.h"
#include "mdm_params.h"


/*!\file cms_msg_modsw.h
 * \brief defines for Modular Sofware Functionality.
 *
 * Modular Software messages are from             0x10002500-0x100025ff
 *
 * Should there be two more specific header files for OSGI EE ?
 * And break up the ranges for common ModSW messages, OSGI specific messages.
 */


/*!\enum CmsModSwMsgType
 * \brief  Enumeration of possible message types
 *
 */
typedef enum 
{
   CMS_MSG_REQUEST_DU_STATE_CHANGE              = 0x10002500, /**< request Deployment Unit state change */
   CMS_MSG_REQUEST_EU_STATE_CHANGE              = 0x10002501, /**< request Execution Unit state change */
   CMS_MSG_GET_FILE_FROM_SERVER                 = 0x10002502, /**< get file from file server */
   CMS_MSG_PUT_FILE_TO_SERVER                   = 0x10002503, /**< put file on file server */
   CMS_MSG_OSGI_EXE_ENV_STATUS_CHANGE           = 0x10002504, /**< OSGI execution environment state change */
   CMS_MSG_REQUEST_BUNDLE_LIST                  = 0x10002505, /**< request for bundle list */
   CMS_MSG_RESPONSE_DU_STATE_CHANGE             = 0x10002507, /**< response Deployment Unit state change */
   CMS_MSG_EU_BOOTUP_SETUP                      = 0x10002508, /**< EU bootup notification */

   CMS_MSG_START_EE                             = 0x10002510, /**< start this execution env */
   CMS_MSG_STOP_EE                              = 0x10002511, /**< stop this execution env */
   CMS_MSG_REQUEST_EE_SHUTDOWN                  = 0x10002514, /**< request Execution Environment shutdown */
   CMS_MSG_RESPONSE_EE_SHUTDOWN                 = 0x10002515, /**< response Execution Environment shutdown */
   CMS_MSG_REQUEST_PREINSTALL_EE_CHANGE         = 0x10002516, /**< request change operation to preinstall Execution Environment */

   CMS_MSG_EXT_EU_CLIENT_PRIVILEGE_CHANGE       = 0x10002520, /**< tell pmd EU extension client privilege is changed */
   CMS_MSG_GET_EU_DISK_SPACE_IN_USE             = 0x10002521, /**< ask pmd to return the disk space in use by an EU */
   CMS_MSG_GET_EE_AVAILABLE_DISK_SPACE          = 0x10002522, /**< ask pmd to return the EE available disk space */

   CMS_MSG_OSGID_PRINT                          = 0x10002530, /**< OSGID command to print debug info to console */

   CMS_MSG_LINMOSD_NEWFILE                      = 0x10002560, /**< tell linmosd to process a new file */

   CMS_MSG_CONTAINER_STATUS_REFRESH             = 0x10002570, /**< request to refresh container status */
} CmsModSwMsgType;



/** Data body for CMS_MSG_REQUEST_DU_STATE_CHANGE message type.
 *
 */
typedef struct
{
   char operation[BUFLEN_32];   /**< Required: Install, Update, Uninstall, install_at_bootup */
   char URL[BUFLEN_1024]; /**< Optional: Location of DU to be installed/update */
   char UUID[BUFLEN_40];    /**< Required: Unique ID to describe DU,36 bytes */
   char username[BUFLEN_256]; /**< Optional username for file server */
   char password[BUFLEN_256]; /**< Optional password for file server */
   char execEnvFullPath[BUFLEN_256]; /**< Required for install: FullPath to the Exec Env */
   char version[BUFLEN_32]; /**< Optional: Version of DU */
   UINT16 reqId;               /**< Optional: track req number */
} DUrequestStateChangedMsgBody;



/*!\Software modules operation defines
 */
#define SW_MODULES_OPERATION_INSTALL              "install"
#define SW_MODULES_OPERATION_UNINSTALL            "uninstall"
#define SW_MODULES_OPERATION_UPDATE               "update"
#define SW_MODULES_OPERATION_START                "start"
#define SW_MODULES_OPERATION_STOP                 "stop"
#define SW_MODULES_OPERATION_INSTALL_AT_BOOTUP    "installBootup"
#define SW_MODULES_OPERATION_START_AT_BOOTUP      "startBootup"
#define SW_MODULES_OPERATION_LB                   "LB"
#define SW_MODULES_OPERATION_UPDATE_PRIVILEGE     "updatePrivilege"
#define SW_MODULES_OPERATION_DELETE_PRIVILEGE     "deletePrivilege"


/** Data body for CMS_MSG_RESPONSE_DU_STATE_CHANGE message type.
 *
 */
typedef struct
{
   char operation[BUFLEN_32];     /**< Install, Update, Uninstall, install_at_bootup */
   char URL[BUFLEN_1024];         /**< Location of DU to be installed/update */
   char UUID[BUFLEN_40];          /**< Unique ID to describe DU,36 bytes */
   char DUlist[BUFLEN_256];       /**< list of comma seperated DU bundle ID */
   char version[BUFLEN_32];       /**< Version of DU */
   char currentState[BUFLEN_32];  /**< Current state (Installed, Uninstalled, Failed) of DU */
   UBOOL8 resolved;               /**< Resolved */
   char EUlist[BUFLEN_1024];      /**< list of comma seperated EU bundle ID */
   char startTime[BUFLEN_64];	  /**< X_BROADCOM_COM_startTime */
   char completeTime[BUFLEN_64];  /**< X_BROADCOM_COM_completeTime */
   int faultCode;
   UINT16 reqId;
} DUresponseStateChangedMsgBody;


/** Data body for CMS_MSG_REQUEST_EU_STATE_CHANGE message type.
 *
 */
typedef struct
{
   char operation[BUFLEN_32];   /**< Install, Update, Uninstall */
   char name[BUFLEN_32];        /**< Name of EU */
   char euid[BUFLEN_64];        /**< EUID of EU */
   char version[BUFLEN_64];     /**< EU version */
} EUrequestStateChangedMsgBody;


/** Data body for CMS_MSG_EXT_EU_CLIENT_PRIVILEGE_CHANGE message type.
 *
 */
typedef struct
{
   char operation[BUFLEN_32];                   /**< Install, Update, Uninstall */
   char username[BUFLEN_32+1];                  /**< username of EU */
   char fullPath[MDM_SINGLE_FULLPATH_BUFLEN];   /**< CMS full path of EU */
} ExtEuClientPrivilegeChangedMsgBody;


/** Data body for CMS_MSG_REQUEST_EU_STATE_CHANGE message type.
 *
 */
typedef struct
{
   char euName[BUFLEN_32];        /**< Name of EU */
   char euid[BUFLEN_64];          /**< EUID of EU */
   char username[BUFLEN_32+1];    /**< username of EU */
   char duName[BUFLEN_32];        /**< Name of DU */
} EUBootupNotificationMsgBody;


/** Data body for CMS_MSG_REQUEST_EE_SHUTDOWN message type.
 *
 */
typedef struct
{
   char operation[BUFLEN_32];   /**< Update, Uninstall */
   char fullPath[BUFLEN_256];   /**< Required for update or uninstall: FullPath to the Exec Env */
   char appName[BUFLEN_32];     /**< Required for update: EE application name to be updated */
   int faultCode;               /**< Return error code */
} EErequestShutdownMsgBody;


/** Data body for CMS_MSG_START_EE message type.
 *
 */
typedef struct
{
   char fullPath[BUFLEN_256];   /**< FullPath to the Exec Env */
} EErequestStartMsgBody;


/** Data body for CMS_MSG_STOP_EE message type.
 *
 */
typedef struct
{
   char fullPath[BUFLEN_256];   /**< FullPath to the Exec Env */
} EErequestStopMsgBody;


/** Data body for CMS_MSG_GET_EU_DISK_SPACE_IN_USE message type.
 *
 */
typedef struct
{
   char euDir[BUFLEN_128];    /**< EU directory. e.g. "/du/cwmp-1/app_cwmpd" */
} EUdiskSpaceInUseMsgBody;

/** Data body for CMS_MSG_GET_EU_DISK_SPACE_IN_USE message reply.
 *
 */
typedef struct
{
   SINT32 diskSpaceInUse;  /**< disk space in use by EU */
} EUdiskSpaceInUseReplyBody;

/** Data body for CMS_MSG_GET_EE_AVAILABLE_DISK_SPACE message reply.
 *
 */
typedef struct
{
   SINT32 availableDiskSpace; /**< EE available disk space */
} EEavailableDiskSpaceReplyBody;



/** Preinstall message defines. 
 *
 */
typedef enum 
{
   // last one from above CmsModSwMsgType 
  //  CMS_MSG_PREINSTALLED_DU                      = 0x10002560
    
   CMS_MSG_PREINSTALLED_DU       = 0x10002590,  /**< tell EE to preinstall the DUs */
} CmsModSwMsgPreinstallType;

/** Data body for CMS_MSG_PREINSTALL_DU message type.
 *
 */
typedef struct
{
   char preinstalTarballDir[BUFLEN_1024]; /**< Required for preinstall: FullPath to the DU tarball */
} PreinstallDuMsgBody;


typedef struct {
   char id[BUFLEN_64];
   char name[BUFLEN_64];
   char status[BUFLEN_32];
   char pid[BUFLEN_32];
   char cpu_use[BUFLEN_32];
   char mem_use[BUFLEN_32];
   char interface[BUFLEN_32];
   char ipv4_addrs[BUFLEN_256+BUFLEN_4];
   char ports[BUFLEN_64];
   char byte_sent[BUFLEN_32];
   char byte_received[BUFLEN_32];
} CONTAINER_INFO, *PCONTAINER_INFO;

typedef struct
{
   char eeContainer[BUFLEN_64];
   char euContainer[BUFLEN_64];
} ContainerStatusMsgBody;

typedef struct
{
   CmsRet ret;
   CONTAINER_INFO info;
} ContainerStatusReplyBody;

#endif /* __CMS_MSG_MODSW_H__ */
