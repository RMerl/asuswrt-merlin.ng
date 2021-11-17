/***********************************************************************
 *
 * Copyright (c) 2006-2007  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2006-2007:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ************************************************************************/

#ifndef __CMS_MSG_MODSW_H__
#define __CMS_MSG_MODSW_H__

#include "cms.h"
#include "cms_msg.h"


/*!\file cms_msg_modsw.h
 * \brief defines for Modular Sofware Functionality.
 *
 * Modular Software messages are from             0x10002500-0x100025ff
 *
 * Should there be two more specific header files for OSGI EE and linux EE?
 * And break up the ranges for common ModSW messages, OSGI specific messages,
 * and LinuxEE specific messages.
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

   CMS_MSG_START_EE    = 0x10002510, /**< start this execution env */
   CMS_MSG_STOP_EE     = 0x10002511, /**< stop this execution env */

   CMS_MSG_OSGID_PRINT                          = 0x10002530, /**< OSGID command to print debug info to console */

   CMS_MSG_LINMOSD_NEWFILE = 0x10002560, /**< tell linmosd to process a new file */

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


/** Data body for CMS_MSG_RESPONSE_DU_STATE_CHANGE message type.
 *
 */
typedef struct
{
   char operation[BUFLEN_32];   /**< Install, Update, Uninstall, install_at_bootup */
   char URL[BUFLEN_1024]; /**< Location of DU to be installed/update */
   char UUID[BUFLEN_40];    /**< Unique ID to describe DU,36 bytes */
   char version[BUFLEN_32]; /**< Version of DU */
   char EUlist[BUFLEN_1024]; /**< list of comma seperated EU bundle ID */
   char DUlist[BUFLEN_256];  /**< list of comma seperated DU bundle ID */
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
} EUrequestStateChangedMsgBody;



#endif /* __CMS_MSG_MODSW_H__ */
