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
*      BEEP header file.
*
*****************************************************************************/
#ifndef BEEP_H
#define BEEP_H

/* ---- Include Files ----------------------------------------------------- */

#include <stdbool.h>
#include "beep_container.h"

/* ---- Constants and Types ----------------------------------------------- */

#define BEEP_USERNAME_LEN_MAX    32
#define BEEP_APPNAME_LEN_MAX     32
#define BEEP_CONTNAME_LEN_MAX    64
#define BEEP_NETWORKMODE_LEN_MAX 16
#define BEEP_ADDRESS_LEN_MAX     64
#define BEEP_EXPOSEDPORT_LEN_MAX 128
#define BEEP_NTWK_IFNAME_LEN_MAX 32
#define BEEP_BUSNAME_LEN_MAX     255
#define BEEP_OBJPATH_LEN_MAX     255
#define BEEP_INTERFACE_LEN_MAX   255
#define BEEP_MEMBER_LEN_MAX      255
#define BEEP_FULLPATH_LEN_MAX    1024

#define BEEP_PKG_NAME_LEN_MAX     32
#define BEEP_PKG_DESCRIPTION_MAX  128
#define BEEP_PKG_URL_MAX          1024
#define BEEP_PKG_USERNAME_MAX     256
#define BEEP_PKG_PASSWORD_MAX     256
#define BEEP_PKG_BEEP_VER_LEN_MAX 16
#define BEEP_PKG_VENDOR_LEN_MAX   32
#define BEEP_PKG_APP_LIST_MAX     8
#define BEEP_PKG_DEPENDENCY_MAX   1024

#define BEEP_PKG_TYPE_DU          0
#define BEEP_PKG_TYPE_EE          1

#define BEEP_ADMIN_USER    "admin"
#define SUPER_USER_STRING  "Super User"

#define BEEP_EE_CONTAINERNAME_PREFIX    "BEEP_"

#define BEEP_EE_STOP_TIMEOUT_INTERVAL 3000 //3 secs
#define BEEP_EE_STOP_TIMEOUT_TRY_MAX  10
#define BEEP_EE_UNINSTALL_TIMEOUT_INTERVAL 3000 //3 secs
#define BEEP_EE_UNINSTALL_TIMEOUT_TRY_MAX  10

#define BEEP_HOSTEE_NAME      "BEEP_HOSTEE"
#define BEEP_VERSION          "5.0"
#define BEEP_HOSTEE_VERSION   BEEP_VERSION
#define BEEP_HOSTEE_VENDOR    "Broadcom"
#define BEEP_HOSTEE_TYPE      "Host EE"
#define BEEP_APP_CONTAINERNAME_PREFIX    "BEEP_HOSTEE_"
#define BEEP_HOSTEE_DU_DIR    "Broadcom/"BEEP_HOSTEE_NAME"/du"

#define BEEP_VERSION_5_0  "5.0"
/* Features in BEEP Version 5.0:
 * - Support EE manifest "process" object.
 */

#define BEEP_HOSTEE_VERSION_5_0  "5.0"
/* Features in BEEP HOSTEE Version 5.0:
 * - Support EU manifest "process" object.
 */

#define BEEP_EE_PREINSTALL_MAX     8

#define BEEP_EU_STOP_TIMEOUT_INTERVAL 1000 //1 secs
#define BEEP_EU_STOP_TIMEOUT_TRY_MAX  2

/* ---- DBUS defines ---- */

#define DBUS_BUS_NAME            "org.freedesktop.DBus"
#define DBUS_OBJECT_PATH         "/org/freedesktop/DBus"
#define DBUS_INTERFACE           "org.freedesktop.DBus"

/* ---- SPD defines ---- */

#define SPD_BUS_NAME             "com.broadcom.spd"
#define SPD_OBJECT_PATH          "/com/broadcom/spd"
#define SPD_INTERFACE            "com.broadcom.spd"
#define SPD_BUSGATE_OBJECT_PATH  "/com/broadcom/busgate"
#define SPD_BUSGATE_INTERFACE    "com.broadcom.busgate"
#define SPD_USER_MANAGEMENT_OBJECT_PATH  "/com/broadcom/usermanagement"
#define SPD_USER_MANAGEMENT_INTERFACE    "com.broadcom.usermanagement"

#if defined(DESKTOP_LINUX) && !defined(BUILD_DESKTOP_BEEP)
#define SPD_MESSAGE_ADDR         "/var/tmp/spd_messaging_server_addr"
#else
#define SPD_MESSAGE_ADDR         "/var/spd_messaging_server_addr"
#endif
#define SPD_MESSAGE_BACKLOG      3

typedef enum
{
   SPDRET_SUCCESS = 0,                  /**<Success. */
   SPDRET_DB_EE_DUPLICATE,
   SPDRET_DB_UNKNOWN_EE,
   SPDRET_FILE_TRANSFER_UNABLE_CONTACT_FILE_SERVER,
   SPDRET_FILE_TRANSFER_UNABLE_ACCESS_FILE,
   SPDRET_FILE_TRANSFER_FILE_TIMEOUT,
   SPDRET_FILE_TRANSFER_UNABLE_COMPLETE,
   SPDRET_FILE_TRANSFER_AUTH_FAILURE,
   SPDRET_FILE_TRANSFER_FILE_AUTHENTICATION_ERROR,
   SPDRET_FILE_OPEN_ERROR,
   SPDRET_FILE_INVALID,
   SPDRET_UNINSTALL_IN_PROCESS,
   SPDRET_UPGRADE_IN_PROCESS,
   SPDRET_EE_UPDATE_VERSION_EXISTED,
   SPDRET_EE_UPDATE_DOWNGRADE_NOT_ALLOWED,
   SPDRET_UNSUPPORTED_FILE_TRANSFER_PROTOCOL,
   SPDRET_OPERATION_NOT_PERMITTED,
   SPDRET_SW_MODULE_SYSTEM_RESOURCE_EXCEEDED,
   SPDRET_SYSTEM_RESOURCE_EXCEEDED,
   SPDRET_EE_START_ERROR,
   SPDRET_INVALID_ARGUMENT,             /**<Invalid argument. */
   SPDRET_MANIFEST_PARSE_ERROR,         /**<Error during parsing manifest. */
   SPDRET_INTERNAL_ERROR,               /**<Error intenally. */
   SPDRET_USERNAME_IN_USE,              /**<Username in use. */
   SPDRET_ADD_USER_ERROR,               /**<Add user error. */
   SPDRET_DELETE_USER_ERROR,            /**<Delete user error. */
   SPDRET_USER_NOT_FOUND,               /**<User not found. */
   SPDRET_SET_BUSGATE_POLICY_ERROR,     /**<Set busgate policy error. */
   SPDRET_ACCESS_DENIED,                /**<Access denied. */
   SPDRET_DB_DU_DUPLICATE,
   SPDRET_DB_UNKNOWN_DU,
   SPDRET_DU_EE_MISMATCH,
   SPDRET_DU_UPDATE_VERSION_EXISTED,
   SPDRET_DU_UPDATE_DOWNGRADE_NOT_ALLOWED,
   SPDRET_EU_START_ERROR,
   SPDRET_BEEP_EE_VERSION_MISMATCH,      
   SPDRET_HOSTEE_EE_VERSION_MISMATCH,
   SPDRET_INVALID_URL_FORMAT,
   SPDRET_EE_MANIFEST,                  /**<Package manifest is for EE. */
   SPDRET_MANIFEST_NO_PRIVILEGE_INFO,
} SpdRet;

typedef enum
{
   LXC_STATE_STOPPED = 0,
   LXC_STATE_STARTING,
   LXC_STATE_RUNNING,
   LXC_STATE_STOPPING,
   LXC_STATE_ABORTING,
   LXC_STATE_FREEZING,
   LXC_STATE_FROZEN,
   LXC_STATE_THAWED,
   LXC_STATE_MAX
} LxcState;

typedef struct
{
   char name[BEEP_CONTNAME_LEN_MAX];
   LxcState state;
} LxcStatusMsgBody;

typedef enum
{
   EE_TO_HOST_MSG_DEFAULT  = 0,
   EE_TO_HOST_MSG_BEE      = 1,
   EE_TO_HOST_MSG_OPENWRT  = 2,
   EE_TO_HOST_MSG_OSGI     = 3,
   EE_TO_HOST_MSG_DOCKERMD = 4,
   EE_TO_HOST_MSG_LAST,
} eeToHostMessageType;

typedef enum
{
   BEEP_UNIT_STATUS_DOWN  = 0,
   BEEP_UNIT_STATUS_UP,
   BEEP_UNIT_STATUS_ERR,
   BEEP_UNIT_STATUS_STARTING,
   BEEP_UNIT_STATUS_MAX,
} BeepUnitStatus_t;

typedef enum
{
   BEEP_EVENT_EE_UNINSTALL_COMPLETED  = 0,
   BEEP_EVENT_EE_UPGRADE_COMPLETED,
   BEEP_EVENT_CONTSTATUS,
   BEEP_EVENT_HOSTINFO,
   BEEP_EVENT_DU_UNINSTALL_COMPLETED,
   BEEP_EVENT_DU_UPGRADE_COMPLETED,
   BEEP_EVENT_MAX,
} BeepEvent_t;

typedef enum
{
   BEEP_HOST_EVENT_TYPE_WAN_CONNECTION_UP  = 0,
   BEEP_HOST_EVENT_TYPE_WAN_CONNECTION_DOWN,
   BEEP_HOST_EVENT_TYPE_ETH_LINK_UP,
   BEEP_HOST_EVENT_TYPE_ETH_LINK_DOWN,
   BEEP_HOST_EVENT_TYPE_USB_LINK_UP,
   BEEP_HOST_EVENT_TYPE_USB_LINK_DOWN,
   BEEP_HOST_EVENT_TYPE_WIFI_LINK_UP,
   BEEP_HOST_EVENT_TYPE_WIFI_LINK_DOWN,
   BEEP_HOST_EVENT_TYPE_MOCA_LAN_LINK_UP,
   BEEP_HOST_EVENT_TYPE_MOCA_LAN_LINK_DOWN,
   BEEP_HOST_EVENT_TYPE_HOMEPLUG_LINK_UP,
   BEEP_HOST_EVENT_TYPE_HOMEPLUG_LINK_DOWN,
   BEEP_HOST_EVENT_TYPE_WAN_LINK_UP,
   BEEP_HOST_EVENT_TYPE_WAN_LINK_DOWN,
   BEEP_HOST_EVENT_TYPE_MAX,
} BeepHostEventType_t;

typedef struct
{
   BeepHostEventType_t type;
   char                arg[32];
} BeepHostEvent_t;

typedef struct
{
   BeepUnitStatus_t status;
   char             contName[BEEP_CONTNAME_LEN_MAX];
} BeepContStatusEvent_t;

typedef struct
{
   char    name[BEEP_PKG_NAME_LEN_MAX];
   char    version[BEEP_PKG_BEEP_VER_LEN_MAX];
   char    vendor[BEEP_PKG_VENDOR_LEN_MAX];
   SpdRet  retCode;
} BeepEeShutdownEvent_t;

typedef struct
{
   char    name[BEEP_PKG_NAME_LEN_MAX];
   char    version[BEEP_PKG_BEEP_VER_LEN_MAX];
   char    vendor[BEEP_PKG_VENDOR_LEN_MAX];
   SpdRet  retCode;
} BeepEeUpgradeEvent_t;

typedef struct
{
   char                url[BEEP_PKG_URL_MAX];
   char                username[BEEP_PKG_USERNAME_MAX];
   char                password[BEEP_PKG_PASSWORD_MAX];
   char                name[BEEP_PKG_NAME_LEN_MAX];
   char                vendor[BEEP_PKG_VENDOR_LEN_MAX];
   char                version[BEEP_PKG_BEEP_VER_LEN_MAX];
   char                description[BEEP_PKG_DESCRIPTION_MAX];
   int                 resolved;
   int                 reqId;
   int                 type;   /* 0: EU, 1: EE */
} BeepDuInfo_t;

typedef struct
{
   char                url[BEEP_PKG_URL_MAX];
   char                name[BEEP_PKG_NAME_LEN_MAX];
   char                vendor[BEEP_PKG_VENDOR_LEN_MAX];
   char                version[BEEP_PKG_BEEP_VER_LEN_MAX];
   char                description[BEEP_PKG_DESCRIPTION_MAX];
   char                mngrAppName[BEEP_APPNAME_LEN_MAX];
   eeToHostMessageType type;
   int                 enable;
   BeepUnitStatus_t    status;

   int                 privileged;
   char                username[BEEP_USERNAME_LEN_MAX+1];
   char                containerName[BEEP_CONTNAME_LEN_MAX];
   char                networkMode[BEEP_NETWORKMODE_LEN_MAX];
   char                ipAddress[BEEP_ADDRESS_LEN_MAX];
   char                exposedPorts[BEEP_EXPOSEDPORT_LEN_MAX];
   int                 runLevel;
   ContResource_t      resource;
   bool                enableAfterInstall;
} BeepEeInfo_t;

typedef struct
{
   char                name[BEEP_PKG_NAME_LEN_MAX];
   char                vendor[BEEP_PKG_VENDOR_LEN_MAX];
   char                version[BEEP_PKG_BEEP_VER_LEN_MAX];
   char                description[BEEP_PKG_DESCRIPTION_MAX];
   char                mngrAppName[BEEP_APPNAME_LEN_MAX];
   char                containerName[BEEP_CONTNAME_LEN_MAX];
   int                 privileged;
   int                 runLevel;
   int                 autoStart;
   int                 autoStartOrder;
   int                 autoRelaunch;
   int                 maxRestarts;
   int                 restartInterval;
   int                 successfulStartPeriod;
   ContResource_t      resource;
} BeepEuInfo_t;

typedef struct
{
   char    name[BEEP_PKG_NAME_LEN_MAX];
   char    version[BEEP_PKG_BEEP_VER_LEN_MAX];
   char    vendor[BEEP_PKG_VENDOR_LEN_MAX];
   SpdRet  retCode;
} BeepDuUninstallEvent_t;

typedef struct
{
   char    name[BEEP_PKG_NAME_LEN_MAX];
   char    version[BEEP_PKG_BEEP_VER_LEN_MAX];
   char    vendor[BEEP_PKG_VENDOR_LEN_MAX];
   char    description[BEEP_PKG_DESCRIPTION_MAX];
   char    url[BEEP_PKG_URL_MAX];
   int     resolved;
   unsigned int     euCnt;
   SpdRet  retCode;
   BeepEuInfo_t eu[BEEP_PKG_APP_LIST_MAX];
} BeepDuUpgradeEvent_t;

/** Debug log for storing various ModSw operations. */
#define BEEP_LOG_DIR             "/var/log/beep"
#define BEEP_DEBUG_EE_LOG        BEEP_LOG_DIR"/EeLog"
#define BEEP_DEBUG_DU_LOG        BEEP_LOG_DIR"/DuLog"
#define BEEP_DEBUG_EU_LOG        BEEP_LOG_DIR"/EuLog"

typedef enum
{
   BEEP_LOG_MODE_EE = 0,
   BEEP_LOG_MODE_DU,
   BEEP_LOG_MODE_EU
} BeepLogModeType_t;

#define BEEP_SIGNAL_ETH_LINK_UP    "EthLinkUp"
#define BEEP_SIGNAL_ETH_LINK_DOWN  "EthLinkDown"
#define BEEP_SIGNAL_USB_LINK_UP    "UsbLinkUp"
#define BEEP_SIGNAL_USB_LINK_DOWN  "UsbLinkDown"
#define BEEP_SIGNAL_WIFI_LINK_UP    "WifiLinkUp"
#define BEEP_SIGNAL_WIFI_LINK_DOWN  "WifiLinkDown"
#define BEEP_SIGNAL_MOCA_LAN_LINK_UP     "MocaLanLinkUp"
#define BEEP_SIGNAL_MOCA_LAN_LINK_DOWN   "MocaLanLinkDown"
#define BEEP_SIGNAL_HOMEPLUG_LINK_UP     "HomePlugLinkUp"
#define BEEP_SIGNAL_HOMEPLUG_LINK_DOWN   "HomePlugLinkDown"
#define BEEP_SIGNAL_WAN_LINK_UP    "WanLinkUp"
#define BEEP_SIGNAL_WAN_LINK_DOWN  "WanLinkDown"
#define BEEP_SIGNAL_WAN_CONNECTION_UP    "WanConnectionUp"
#define BEEP_SIGNAL_WAN_CONNECTION_DOWN  "WanConnectionDown"

#define BEEP_SIGNAL_CONTAINER_STATUS_CHANGE  "ContainerStatusChange"

#define BEEP_UNINSTALL_EE_COMPLETED  "EeUninstallCompleted"
#define BEEP_UPGRADE_EE_COMPLETED  "EeUpgradeCompleted"

#define BEEP_UNINSTALL_DU_COMPLETED  "DuUninstallCompleted"
#define BEEP_UPGRADE_DU_COMPLETED  "DuUpgradeCompleted"

/* ---- PMD defines ---- */

#define PMD_BUS_NAME             "com.broadcom.pmd"
#define PMD_OBJECT_PATH          "/com/broadcom/pmd"
#define PMD_INTERFACE            "com.broadcom.pmd"

/* ---- CWMPD defines ---- */

#define CWMPD_BUS_NAME           "com.broadcom.cwmp"
#define CWMPD_OBJECT_PATH        "/com/broadcom/cwmp"
#define CWMPD_INTERFACE          "com.broadcom.cwmp"
#define SIGNAL_CWMPD_TERMINATION "CwmpdTermination"
#define CWMPUTL_OBJECT_PATH      "/com/broadcom/cwmputl"
#define CWMPUTL_INTERFACE        "com.broadcom.cwmputl"

/* ---- DAD defines ---- */

#define DAD_BUS_NAME             "com.broadcom.DataAdaptation"
#define DAD_OBJECT_PATH          "/com/broadcom/DataAdaptation"
#define DAD_LAYER2_INTERFACE     "com.broadcom.DataAdaptation.Layer2"
#define DAD_CWMPCLIENT_INTERFACE "com.broadcom.DataAdaptation.CwmpClient"
#define DAD_WIFI_INTERFACE       "com.broadcom.DataAdaptation.Wifi"

/* ---- Data Model Access Daemon defines ---- */

#define DMAD_BUS_NAME            "com.broadcom.dmad"
#define DMAD_OBJECT_PATH         "/com/broadcom/dmad"
#define DMAD_INTERFACE           "com.broadcom.dmad"

/* Openwrt names */
#define OPENWRT_BUS_NAME         "com.broadcom.openwrt"
#define OPENWRTD_BUS_NAME        "com.broadcom.openwrtd"

/* ---- BBCD defines ---- */

#define BBCD_BUS_NAME            "com.broadcom.bbcd"
#define BBCD_OBJECT_PATH         "/com/broadcom/bbcd"
#define BBCD_INTERFACE           "com.broadcom.bbcd"

/* -------------------------------- */
/* ---- Signals emitted by PMD ---- */
/* -------------------------------- */

/* Signal Name: DuStateChangeComplete
 * Description:
 *    DU operation has been completed.
 * Parameters: array of structs. Each contains the following fields:
 *    char *operation  - Install, Update, Uninstall, install_at_bootup
 *    char *URL  - Location of DU to be installed/update
 *    char *UUID  - Unique ID to describe DU,36 bytes
 *    char *DUlist  - list of comma seperated DU bundle ID
 *    char *version  - Version of DU
 *    char *currentState - Current state (Installed, Uninstalled, Failed) of DU
 *    UBOOL8 resolved - Resolved of DU
 *    char *EUlist  - list of comma seperated EU bundle ID
 *    char *startTime - time operation is started
 *    char *completeTime - time operation is completed
 *    gint32   faultCode  - operation error code
 *    guint16  reqId  - operation request ID
 */
#define SIGNAL_DU_STATE_CHANGE_COMPLETE   "DuStateChangeComplete"

/* Signal Name: EthLinkUp
 * Description:
 *    Eth link is up.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_ETH_LINK_UP    "EthLinkUp"

/* Signal Name: EthLinkDown
 * Description:
 *    Eth link is down.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_ETH_LINK_DOWN  "EthLinkDown"

/* Signal Name: UsbLinkUp
 * Description:
 *    Usb link is up.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_USB_LINK_UP    "UsbLinkUp"

/* Signal Name: UsbLinkDown
 * Description:
 *    Usb link is down.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_USB_LINK_DOWN  "UsbLinkDown"

/* Signal Name: WifiLinkUp
 * Description:
 *    Wifi link is up.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_WIFI_LINK_UP    "WifiLinkUp"

/* Signal Name: WifiLinkDown
 * Description:
 *    Wifi link is down.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_WIFI_LINK_DOWN  "WifiLinkDown"

/* Signal Name: MocaLanLinkUp
 * Description:
 *    Moca LAN link is up.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_MOCA_LAN_LINK_UP     "MocaLanLinkUp"

/* Signal Name: MocaLanLinkDown
 * Description:
 *    Moca LAN link is down.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_MOCA_LAN_LINK_DOWN   "MocaLanLinkDown"

/* Signal Name: HomePlugLinkUp
 * Description:
 *    Homeplug link is up.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_HOMEPLUG_LINK_UP     "HomePlugLinkUp"

/* Signal Name: HomePlugLinkDown
 * Description:
 *    Homeplug link is down.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_HOMEPLUG_LINK_DOWN   "HomePlugLinkDown"

/* Signal Name: WanLinkUp
 * Description:
 *    Wan link is up.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_WAN_LINK_UP    "WanLinkUp"

/* Signal Name: WanLinkDown
 * Description:
 *    Wan link is down.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_WAN_LINK_DOWN  "WanLinkDown"

/* Signal Name: WanConnectionUp
 * Description:
 *    Layer 3 Wan connection is up.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_WAN_CONNECTION_UP    "WanConnectionUp"

/* Signal Name: WanConnectionDown
 * Description:
 *    Layer 3 Wan connection is down.
 * Parameters:
 *    char *ifname  - interface name
 */
#define SIGNAL_WAN_CONNECTION_DOWN  "WanConnectionDown"

/* Signal Name: AcsConfigChanged
 * Description:
 *    ACS configuration has changed.
 * Parameters:
 *    none
 */
#define SIGNAL_ACS_CONFIG_CHANGED   "AcsConfigChanged"

/* Signal Name: TR69cConfigChanged
 * Description:
 *    TR69c configuration has changed.
 * Parameters:
 *    none
 */
#define SIGNAL_TR69C_CONFIG_CHANGED   "TR69cConfigChanged"


/* Signal Name: DeviceNotificationLimitChanged
 * Description:
 *    TR69c manageable device notification limit has changed
 * Parameters:
 *    unsigned int limit
 */
#define SIGNAL_DEVICE_NOTIFICATION_LIMIT_CHANGED "DeviceNotificationLimitChanged"

/* Signal Name: ActiveParameterValueChanged
 * Description:
 *    One or more parameters with active notification attribute set
 *    had their value changed.
 * Parameters:
 *    none
 */
#define SIGNAL_ACTIVE_PARAMETER_VALUE_CHANGED   "ActiveParameterValueChanged"

/* Signal Name: Tr69GetRpcMethodsDiag
 * Description:
 *    request tr69c send out a GetRpcMethods.
 * Parameters:
 *    none
 */
#define SIGNAL_TR69_GETRPCMETHODS_DIAG    "Tr69GetRpcMethodsDiag"

/* Signal Name: ConfigUploadComplete
 * Description:
 *    A remote configuration cycle has ended.
 * Parameters:
 *    none
 */
#define SIGNAL_CONFIG_UPLOAD_COMPLETE  "ConfigUploadComplete"

/* Signal Name: ConfigWritten
 * Description:
 *    A config file has been written.
 * Parameters:
 *    guint32 eid  - eid who wrote out the config file.
 */
#define SIGNAL_CONFIG_WRITTEN    "ConfigWritten"

/* Signal Name: TimeStateChanged
 * Description:
 *    TIME state changed.
 * Parameters:
 *    guint32 state  - time state.
 */
#define SIGNAL_TIME_STATE_CHANGED   "TimeStateChanged"

/* Signal Name: LogicalVolumeAdded
 * Description:
 *    USB storage logical volume added.
 * Parameters:
  *    int disk int: partition
 */
#define SIGNAL_LOGICAL_VOLUME_ADDED   "LogicalVolumeAdded"

/* Signal Name: LogicalVolumeRemoved
 * Description:
 *    USB storage logical volume removed.
 * Parameters:
 *    int disk int: partition
 */
#define SIGNAL_LOGICAL_VOLUME_REMOVED   "LogicalVolumeRemoved"


/* Signal Name: PmdTermination
 * Description:
 *    Pmd is terminating.
 * Parameters:
 *    none
 * Note: This is a public signal that reqires no permission to receive.
 *       PMD related applications should subscribe and gracefully shutdown
 *       upon receiving this signal.
 */
#define SIGNAL_PMD_TERMINATION  "PmdTermination"

/* Signal Name: SpdTermination
 * Description:
 *    Spd is terminating.
 * Parameters:
 *    none
 * Note: This is a public signal that reqires no permission to receive.
 *       SPD related applications should subscribe and gracefully shutdown
 *       upon receiving this signal.
 */
#define SIGNAL_SPD_TERMINATION  "SpdTermination"


/* ---------------------------------- */
/* ---- Signals emitted by CWMPD ---- */
/* ---------------------------------- */

/* Signal Name: AcsConfigChanged
 * Description:
 *    ACS configuration has changed.
 * Parameters:
 *    none
 */
#define SIGNAL_ACS_CONFIG_CHANGED   "AcsConfigChanged"

/* Signal Name: PingStateChanged 
 * Description:
 *    Forward ipping diag state change event
 * Parameters:
 *    none
 */
#define SIGNAL_PING_STATE_CHANGED "PingStateChanged"

/* Signal Name: DiagnosticsComplete
 * Description:
 *    Forward tr143 diag state complete event
 * Parameters:
 *    none
 */
#define SIGNAL_DIAG_COMPLETE "DiagnosticsComplete"

/* Signal Name: Tr69ActiveNotification
 * Description:
 *    One or more parameters with active notification attribute set
 *    had their value changed.
 * Parameters:
 *    none
 */
#define SIGNAL_TR69_ACTIVE_NOTIFICATION   "Tr69ActiveNotification"

/* Signal Name: StunConfigChanged
 * Description:
 *    STUN configuration has changed.
 * Parameters:
 *    none
 */
#define SIGNAL_STUN_CONFIG_CHANGED  "StunConfigChanged"


/*****************************************************************************
*  FUNCTION:  beep_debug_write_to_log
*  DESCRIPTION:
*     Write debug message to specific BEEP log.
*  PARAMETERS:
*     log_type (IN) BEEP_LOG_MODE_EE, BEEP_LOG_MODE_DU, or BEEP_LOG_MODE_EU.
*     req_id (IN) Identifer of this log message.
*     name (IN) log message is applied to this EE, DU, or EU name.
*     operation (IN) operation causes log message.
*     msg (IN) log.
*  RETURNS:
*     void
******************************************************************************
*/
void beep_debug_write_to_log(BeepLogModeType_t log_type,
                             int req_id,
                             const char *name,
                             const char *operation,
                             const char *msg);


#endif /* BEEP_H */
