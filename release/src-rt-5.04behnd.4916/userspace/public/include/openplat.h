/***********************************************************************
 *
 * Copyright (c) 2017  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2017:DUAL/GPL:standard
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


/*****************************************************************************
*    Description:
*
*      OpenPlatform header file.
*
*****************************************************************************/
#ifndef OPENPLAT_H
#define OPENPLAT_H

/* ---- Include Files ----------------------------------------------------- */

#include "beep.h"
#include "bcm_retcodes.h"

/* ---- Constants and Types ----------------------------------------------- */

#define OPS_EE_CONTAINERNAME_PREFIX    "OPS_"

#define OPS_BOOTUP_TIMEOUT_INTERVAL 6000 //6 secs
#define OPS_BOOTUP_TIMEOUT_TRY_MAX  10
#define OPS_EE_STOP_TIMEOUT_INTERVAL 3000 //3 secs
#define OPS_EE_STOP_TIMEOUT_TRY_MAX  10
#define OPS_EE_RESTART_TIMEOUT_TRY_MAX  11
#define OPS_EE_UNINSTALL_TIMEOUT_INTERVAL 3000 //3 secs
#define OPS_EE_UNINSTALL_TIMEOUT_TRY_MAX  10
#define OPS_PREINSTALL_CLEAN_TIMEOUT_INTERVAL 3000 //3 secs
#define OPS_PREINSTALL_CLEAN_TIMEOUT_TRY_MAX  10

#define OPS_HOSTEE_TYPE_PREFIX   "ETSI OpenPlatform 2.0, HG_Core, Broadcom Ref SW Linux"
#define OPS_EE_TYPE_PREFIX       "ETSI OpenPlatform 2.0, HG_EE:"

#define OPS_HOSTEE_NAME      "OPS_HOSTEE"
#define OPS_HOSTEE_ALIAS     "HostEE"
/* OPS_VERSION 3.0 has changes in BDK ZBUS API */
/* OPS_VERSION 3.1 has changes in container lib (hardcode bind mount removal) */
/* OPS_VERSION 4.0 
 * 1. Added required period parameter in cpu section in manifest
 * 2. after switching to kernel 5.15, glibc used by toolchain
 * removes libpthread and librt. So dependency section in container manifest
 * needs to be updated
 * 3. we will always bump up the major number whenever a new toolchain is
 * updated or a major feature is introduced
 * 4. Our convention of a DU version that uses HostEE version 4 would have its
 * major version 4 as well. The minor version is bumped whenever there is a
 * modification in the DU's EU */
/* OPS_VERSION 4.1 
 * The version is bumped because the changeDuStateCompleteResponse API
 * changed to retrieve the allocatedEUUID, allocatedEUGID,
 * allocatedHostUID, allocatedHostGID, and updateUID */
#define OPS_VERSION          "4.1"
#define OPS_HOSTEE_VERSION   OPS_VERSION
#define OPS_HOSTEE_VENDOR    "Broadcom"
#define OPS_HOSTEE_TYPE      "Host EE"
#define OPS_APP_CONTAINERNAME_PREFIX    "OPS_HOSTEE_"
#define OPS_HOSTEE_DU_DIR    "Broadcom/"OPS_HOSTEE_NAME"/du"
#define OPS_HOSTEE_LIB_DIR   "Broadcom/"OPS_HOSTEE_NAME"/lib"
#define OPS_PKG_APPARMOR_DIR "etc/apparmor.d"
#define OPS_APPARMOR_DIR     "/etc/apparmor.d/lxc"

#define OPS_EE_PREINSTALL_MAX     8
#define OPS_EE_RESTART_MAX        8
#define OPS_DU_PREINSTALL_MAX     16
#define OPS_DU_PER_UNINSTALL_MAX  8
#define OPS_DU_PER_UPDATE_MAX     16

#define OPS_EU_STOP_TIMEOUT_INTERVAL 1000 //1 secs
#define OPS_EU_STOP_TIMEOUT_TRY_MAX  2
#define OPS_EU_RESTART_TIMEOUT_TRY_MAX  4
#define OPS_EU_RESTART_MAX  8

#define OPS_UID_BASE_HOSTEE      8000
#define OPS_UID_BASE_BGN         10000
#define OPS_UID_MAX_NUM          1000

#define OPS_LOCK_TIMEOUT         (8*1000)

#define OPS_UNKNOWN_EE_NAME      "EE"
#define OPS_UNKNOWN_DU_NAME      "DU"

/* ---- ops defines ---- */
#define OPENPLAT_BUS_TYPENAME_DBUS       "dbus"
#define OPENPLAT_BUS_TYPENAME_UBUS       "ubus"
#define OPENPLAT_MD_MODSW_INTERFACE      "com.broadcom.openplat_md_modsw"
#define OPENPLAT_MD_BUSGATE_OBJECT_PATH  "/com/broadcom/busgate"
#define OPENPLAT_MD_BUSGATE_INTERFACE    "com.broadcom.busgate"
#define OPENPLAT_MD_USER_MANAGEMENT_OBJECT_PATH  "/com/broadcom/usermanagement"
#define OPENPLAT_MD_USER_MANAGEMENT_INTERFACE    "com.broadcom.usermanagement"

#if defined(DESKTOP_LINUX)
#define OPS_OOM_MESSAGE_ADDR         "/var/tmp/ops_oom_messaging_server_addr"
#else
#define OPS_OOM_MESSAGE_ADDR         "/var/ops_oom_messaging_server_addr"
#endif
#define OPS_OOM_MESSAGE_BACKLOG      3


/* ---- modswd defines ---- */
#define MODSWD_BUS_NAME              "com.broadcom.modswd"
#define MODSWD_MODSW_OBJECT_PATH           "/com/broadcom/modsw"
#define MODSWD_MODSW_INTERFACE_NAME        "com.broadcom.modsw"
#define MODSWD_OBJECT_PATH     "/com/broadcom/modswd"
#define MODSWD_INTERFACE_NAME  "com.broadcom.modswd"

#define TR69_PROXY_BUS_NAME          "com.broadcom.tr69_proxy"
#define TR69_PROXY_PROXY_OBJECT_PATH       "/com/broadcom/tr69_proxyd"
#define TR69_PROXY_PROXY_INTERFACE_NAME    "com.broadcom.tr69_proxyd"
#define TR69_PROXY_OBJECT_PATH       "/com/broadcom/tr69_proxy"
#define TR69_PROXY_INTERFACE_NAME    "com.broadcom.tr69_proxy"

#define USP_MD_MODSW_OBJECT_PATH     "/com/broadcom/usp_md_modsw"
#define USP_MD_MODSW_INTERFACE_NAME  "com.broadcom.usp_md_modsw"

/* Package state change operation */
#define OPERATION_INSTALL     "install"
#define OPERATION_UPDATE      "update"
#define OPERATION_UNINSTALL   "uninstall"

typedef enum
{
   DU_INSTALL,
   DU_UPDATE,
   DU_UNINSTALL
} OpenplatDuOperation;

/* Package state change status */
typedef enum
{
   DU_STATUS_INSTALLING,
   DU_STATUS_INSTALLED,
   DU_STATUS_UPDATING,
   DU_STATUS_UNINSTALLING,
   DU_STATUS_UNINSTALLED
} OpenplatDuStatus;

/* Application state change execution */
typedef enum
{
   EU_START,
   EU_STOP
} OpenplatEuExecution;

/* Application state change status */
typedef enum
{
   EU_STATUS_IDLE,
   EU_STATUS_STARTING,
   EU_STATUS_ACTIVE,
   EU_STATUS_STOPPING
} OpenplatEuStatus;

/* Application state change fault code */
typedef enum
{
   EU_NO_FAULT,
   EU_FAILURE_ON_START,
   EU_FAILURE_ON_AUTO_START,
   EU_FAILURE_ON_STOP,
   EU_FAILURE_WHILE_ACTIVE,
   EU_DEPENDENCY_FAILURE,
   EU_UNSTARTABLE
} OpenplatEuFaultCode;


/* ChangeDuStateCompleteResponse Identifiers */
#define PACKAGE_ID      "PKGID"
#define NAME_ID         "name"
#define VENDOR_ID       "vendor"
#define VERSION_ID      "version"
#define DESCRIPTION_ID  "description"
#define STATUS_ID       "status"
#define RESOLVED_ID     "resolved"

#define APPLICATION_ID  "APPID"
#define AUTOSTART_ID    "autoStart"
#define RUNLEVEL_ID     "runLevel"
#define EXEFAULTCODE_ID "exeFaultCode"
#define EXEFAULTMSG_ID  "exeFaultMessage"
#define CONTAINERNAME_ID "containerName"
/* UPDATE_USER_ID must be set to 1 for openplat_md to pick up the
 * allocatedEUID, allocatedEUGID, allocatedHostID, and allocatedGroupID.
 * When 0, openplat will assume these ID have not been assigned */
#define UPDATE_USER_ID   "updateUID"
#define USER_ID          "allocatedEUUID"
#define GROUP_ID         "allocatedEUGID"
#define HOST_USER_ID     "allocatedHostUID"
#define HOST_GROUP_ID    "allocatedHostGID"


#endif /* OPENPLAT_H */
