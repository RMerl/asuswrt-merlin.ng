 /*
 * Copyright 2015, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef __nt_common_h__
#define __nt_common_h__

#define NC_VERSION 2

/* SOCKET SERVER DEFINE SETTING 
---------------------------------*/
#define NOTIFY_CENTER_SOCKET_PATH               "/var/run/nt_center_socket"
#define NOTIFY_MAIL_SERVICE_SOCKET_PATH         "/var/run/nt_actMail_socket"

#define MAX_NOTIFY_SOCKET_CLIENT    5
#define PTHREAD_STACK_SIZE          0x100000

#define NOTIFY_CENTER_PID_PATH                  "/var/run/nt_center.pid"
#define NOTIFY_CENTER_MONITOR_PID_PATH          "/var/run/nt_monitor.pid"
#define NOTIFY_ACTION_MAIL_PID_PATH             "/var/run/nt_actMail.pid"

#define NOTIFY_CENTER_LOG_FILE                  "/tmp/NTC.log"
#define NOTIFY_CENTER_MONITOR_LOG_FILE          "/tmp/NTM.log"
#define NOTIFY_ACTION_MAIL_LOG_FILE             "/tmp/NAM.log"

#define NOTIFY_CENTER_DEBUG                     "/tmp/NTC_DEBUG"
#define NOTIFY_CENTER_MONITOR_DEBUG             "/tmp/NTM_DEBUG"
#define NOTIFY_ACTION_MAIL_DEBUG                "/tmp/NAM_DEBUG"

#define NOTIFY_CENTER_TEMP_DIR                  "/tmp/nc"
#define NOTIFY_SETTING_CONF                     "/tmp/nc/nc.conf"
#define EVENTID_SCRIPT_DEFINE_PATH              "/tmp/nc/event.conf"

#define PUSH_MAC_PATH                           "/tmp/nc/mac"
#define PUSH_CONF_PATH                          "/tmp/nc/pns_info"
#define APP_API_LEVEL_PATH                      "/tmp/nc/appver"

/* NOTIFICATION DATABASE DEFINE SETTING 
----------------------------------------*/
#define TYPE_SHIFT              16
#define NOTIFY_DB_QLEN          400
#define NOTIFY_DB_DEBUG         "/tmp/NTD_DEBUG"

#ifdef ASUSWRT_SDK /* ASUSWRT SDK */

#include <rtconfig.h>

#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS))
#define NOTIFY_DB_FOLDER        "/jffs/.sys/nc/"
#else
#define NOTIFY_DB_FOLDER        "/tmp/nc/"
#endif

#else /* DSL_ASUSWRT_SDK */

#if defined(TCSUPPORT_ADD_JFFS) || defined(TCSUPPORT_SQUASHFS_ADD_YAFFS)
#define NOTIFY_DB_FOLDER        "/jffs/nc/"
#else
#define NOTIFY_DB_FOLDER        "/tmp/nc/"
#endif

#endif

#define NTDB_V1        NOTIFY_DB_FOLDER"nt_center.db"
#define NTDB_V2        NOTIFY_DB_FOLDER"nt_db.db"

/* ACTION SERVICE EVENT DEFINE 
---------------------------------*/
#define ACT_NOTIFY_RSV       0
#define ACT_NOTIFY_DB        0x01
#define ACT_NOTIFY_EMAIL     0x02
#define ACT_NOTIFY_APPUSH    0x04
#define ACT_NOTIFY_IFTTT     0x08
#define ACT_NOTIFY_ALEXA     0x10
#define ACT_NOTIFY_GENERAL   0x20

#define ACT_MULTI_ALL        ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH | ACT_NOTIFY_EMAIL | ACT_NOTIFY_IFTTT | ACT_NOTIFY_ALEXA

/* ACTION BIT OPERATION */
#define NC_ACT_DB_BIT           0
#define NC_ACT_EMAIL_BIT        1
#define NC_ACT_APPUSH_BIT       2
#define NC_ACT_IFTTT_BIT        3
#define NC_ACT_ALEXA_BIT        4
#define NC_ACT_GENERAL_BIT      5

/* DB STATUS BIT OPERATION */
#define NC_DB_APPUSH_BIT        1
#define NC_DB_IFTTT_BIT         3
#define NC_DB_ALEXA_BIT         5

#define NC_ACTION_SET(value,x) ( (value) |=  (0x1 << x))
#define NC_ACTION_CLR(value,x) ( (value) &= ~(0x1 << x))

#define MAX_NOTIFY_EVENT_NUM    128

/* NOTIFY CLIENT EVENT DEFINE 
---------------------------------*/
/* RESERVATION EVENT */
#define RESERVATION_EVENT_PREFIX                   0xF000
#define RESERVATION_MAIL_REPORT_EVENT              0xF001           /* Used in email daemon status report */
#define RESERVATION_MAIL_CONFIRM_EVENT             0xF002
/* ------------------------------
    ### System ###
---------------------------------*/
#define SYS_WAN_DISCONN_EVENT                      0x10001
#define SYS_WAN_BLOCK_EVENT                        0x10002
#define SYS_NEW_DEVICE_WIFI_CONNECTED_EVENT        0x10010
#define SYS_NEW_DEVICE_ETH_CONNECTED_EVENT         0x10011
#define SYS_FW_NWE_VERSION_AVAILABLE_EVENT         0x10012
#define SYS_EXISTED_DEVICE_WIFI_CONNECTED_EVENT    0x10014
#define SYS_WAN_CABLE_UNPLUGGED_EVENT              0x10015
#define SYS_WAN_PPPOE_AUTH_FAILURE_EVENT           0x10016
#define SYS_WAN_USB_MODEM_UNREADY_EVENT            0x10017
#define SYS_WAN_IP_CONFLICT_EVENT                  0x10018
#define SYS_WAN_UNABLE_CONNECT_PARENT_AP_EVENT     0x10019
#define SYS_WAN_MODEM_OFFLINE_EVENT                0x1001A
#define SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT        0x1001B
#define SYS_WAN_UNPUBLIC_IP_EVENT                  0x1001C
#define SYS_ALL_WIFI_TURN_OFF_EVENT                0x1001F /* <-- last */
/* ------------------------------
    ### Administration ###
---------------------------------*/
#define ADMIN_LOGIN_FAIL_LAN_WEB_EVENT             0x20001
#define ADMIN_LOGIN_FAIL_SSH_EVENT                 0x20003
#define ADMIN_LOGIN_FAIL_TELNET_EVENT              0x20004
/* ------------------------------
    ### Security ###
---------------------------------*/
#define PROTECTION_VULNERABILITY_EVENT             0x30002
#define PROTECTION_CC_EVENT                        0x30003
#define PROTECTION_MALICIOUS_SITE_EVENT            0x30008
#define PROTECTION_INFECTED_DEVICE_EVENT           0x30010
/* ------------------------------
    ### USB Function ###
---------------------------------*/
#define USB_TETHERING_EVENT                        0x60006
/* ------------------------------
    ### General Event  ###
---------------------------------*/
#define GENERAL_EVENT_PREFIX                       0x70000
#define GENERAL_WIFI_DEV_ONLINE                    0x70001
#define GENERAL_WIFI_DEV_OFFLINE                   0x70002
#define GENERAL_ETH_DEV_ONLINE                     0x70003
#define GENERAL_ETH_DEV_OFFLINE                    0x70004
#define GENERAL_ETH_DEV_REFUSED                    0x70005
#define GENERAL_SYS_STATES                         0x70006
#define GENERAL_DEV_UPDATE                         0x70007
#define GENERAL_DEV_DELETED                        0x70008
#define GENERAL_DEV_ACCESS_CHANGE                  0x70009
#define GENERAL_QOS_UPDATE                         0x7000A
#define GENERAL_TOGGLE_STATES_UPDATE               0x7000B
/* EVENT DEFINE END
---------------------------------*/

#define MAX_EVENT_INFO_LEN    512

/* EVENT PRIORITY
---------------------------------*/
typedef enum {
	PRI_RSV    = 0,
	PRI_UHIGH  = 10,
	PRI_HIGH   = 20,
	PRI_LOW    = 90,
}EVENT_PRI_T;

/* SEND MAIL STATUS
---------------------------------*/
typedef enum {
	MAIL_WAIT = 0,
	MAIL_SENDING,
	MAIL_SUCCESS,
	MAIL_FAILED,
	MAIL_FATAL_ERROR                /* E-Mail Config error */
}MAIL_STATUS_T;

/* ACTION MAIL EVENT STRUCTURE
---------------------------------*/
typedef struct __act_mail_event__t_
{
	int             MsendId;        /* Random ID to record send mail index */
	int             Mretry;         /* Record retry times when send mail fail */
	time_t          MsendTime;      /* Record send time */
	MAIL_STATUS_T   MsendStatus;    /* Record send mail status */
}ACTMAIL_EVENT_T;

/* NOTIFY CLIENT EVENT STRUCTURE
---------------------------------*/
typedef struct __notify_event__t_
{
	int             event;          /* Refer NOTIFY CLIENT EVENT DEFINE */
	char            msg[MAX_EVENT_INFO_LEN];       /* Info */

/*  ### NT_CENTER HANDLER Info ### */
	time_t          tstamp;         /* Receive time */
	int             action;
	int             period;         /* Period time(sec) */
	int             gntee;          /* Action Guarantee */
	ACTMAIL_EVENT_T mail_t;         /* Action Mail Data Info*/

}NOTIFY_EVENT_T;

/* NOTIFY DATABASE STRUCTURE */
typedef struct __notify_database__t_
{
	time_t          tstamp;         /* Receive time */
	int             event;          /* Refer NOTIFY CLIENT EVENT DEFINE */
	int             status;         /* store the action / read status of different behavior */
	char            msg[MAX_EVENT_INFO_LEN];       /* Info */

}NOTIFY_DATABASE_T;

#endif

