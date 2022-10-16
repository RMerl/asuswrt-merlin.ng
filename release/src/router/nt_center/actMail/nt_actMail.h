 /*
 * Copyright 2016, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef __nt_actMail_h__
#define __nt_actMail_h__


#include <libnt.h>
#include <nt_eInfo.h>
#include <nt_actMail_common.h>
#include <json.h>

#define MyDBG(fmt,args...) \
	if(isFileExist(NOTIFY_ACTION_MAIL_DEBUG) > 0) { \
		Debug2Console("[ACTMAIL][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}
#define ErrorMsg(fmt,args...) \
	Debug2Console("[ACTMAIL][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args);

/* Define a shuffle function. e.g. DECL_SHUFFLE(double). */
#define DECL_SHUFFLE(type)				\
void shuffle_##type(type *list, size_t len) {		\
	int j;						\
	type tmp;					\
	while(len) {					\
		j = irand(len);				\
		if (j != len - 1) {			\
			tmp = list[j];			\
			list[j] = list[len - 1];	\
			list[len - 1] = tmp;		\
		}					\
		len--;					\
	}						\
}							\


/* SEND MAIL INFO STRUCTURE
---------------------------------*/
typedef struct __mail_info__t_
{
	char   modelName[16];
	char   subject[64];
	char   toMail[256];
	char   msg[MAX_EVENT_INFO_LEN];
	time_t tstamp;
	int    MsendId;
	int    event;

}MAIL_INFO_T;

/* NOTIFY MAIL CONTENT INFO STRUCTURE
---------------------------------*/
struct MAIL_CONTENT_T
{
	char lang[4];
	int  event;
	MAIL_INFO_T *MailContentFunc;
};

/* NOTIFY MAIL CONTENT CALLBACK FUNCTION DECLARE
NAMING RULE: [EventName]_[Language]_FUNC
---------------------------------
   ### Reservation ###
---------------------------------*/
MAIL_INFO_T *RESERVATION_MAIL_CONFIRM_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *RESERVATION_MAIL_CONFIRM_TW_FUNC(MAIL_INFO_T *mInfo);
/* ------------------------------
   ### System ###
---------------------------------*/
MAIL_INFO_T *SYS_WAN_DISCONN_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_BLOCK_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_NEW_DEVICE_WIFI_CONNECTED_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_NEW_DEVICE_ETH_CONNECTED_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_EXISTED_DEVICE_WIFI_CONNECTED_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_FW_NWE_VERSION_AVAILABLE_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_CABLE_UNPLUGGED_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_PPPOE_AUTH_FAILURE_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_USB_MODEM_UNREADY_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_IP_CONFLICT_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_UNABLE_CONNECT_PARENT_AP_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_MODEM_OFFLINE_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_GOT_PROBLEMS_FROM_ISP_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_WAN_UNPUBLIC_IP_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *SYS_ALL_WIFI_TURN_OFF_EN_FUNC(MAIL_INFO_T *mInfo);
/* ------------------------------
   ### Administration ###
---------------------------------*/
MAIL_INFO_T *ADMIN_LOGIN_FAIL_LAN_WEB_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *ADMIN_LOGIN_FAIL_SSH_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *ADMIN_LOGIN_FAIL_TELNET_EN_FUNC(MAIL_INFO_T *mInfo);
/* ------------------------------
   ### Security ###
---------------------------------*/
MAIL_INFO_T *PROTECTION_VULNERABILITY_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *PROTECTION_CC_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *PROTECTION_MALICIOUS_SITE_EN_FUNC(MAIL_INFO_T *mInfo);
MAIL_INFO_T *PROTECTION_INFECTED_DEVICE_EN_FUNC(MAIL_INFO_T *mInfo);
/* ------------------------------
   ### USB Function ###
---------------------------------*/
MAIL_INFO_T *USB_TETHERING_EN_FUNC(MAIL_INFO_T *mInfo);


/* NOTIFY MAIL CONTENT CALLBACK FUNCTION TABLE
NAMING RULE: [EventName]_[Language]_FUNC
---------------------------------*/
struct MAIL_CONTENT_T CONTENT_TABLE[] =
{
	/* ------------------------------
	   ### Reservation ###
	---------------------------------*/
	{"EN", RESERVATION_MAIL_CONFIRM_EVENT,            (MAIL_INFO_T *) RESERVATION_MAIL_CONFIRM_EN_FUNC},
//	{"TW", RESERVATION_MAIL_CONFIRM_EVENT,            (MAIL_INFO_T *) RESERVATION_MAIL_CONFIRM_TW_FUNC},
	/* ------------------------------
	   ### System ###
	---------------------------------*/
	{"EN", SYS_WAN_DISCONN_EVENT,                     (MAIL_INFO_T *) SYS_WAN_DISCONN_EN_FUNC},
	{"EN", SYS_WAN_BLOCK_EVENT,                       (MAIL_INFO_T *) SYS_WAN_BLOCK_EN_FUNC},
	{"EN", SYS_NEW_DEVICE_WIFI_CONNECTED_EVENT,       (MAIL_INFO_T *) SYS_NEW_DEVICE_WIFI_CONNECTED_EN_FUNC},
	{"EN", SYS_NEW_DEVICE_ETH_CONNECTED_EVENT,        (MAIL_INFO_T *) SYS_NEW_DEVICE_ETH_CONNECTED_EN_FUNC},
	{"EN", SYS_FW_NWE_VERSION_AVAILABLE_EVENT,        (MAIL_INFO_T *) SYS_FW_NWE_VERSION_AVAILABLE_EN_FUNC},
	{"EN", SYS_EXISTED_DEVICE_WIFI_CONNECTED_EVENT,   (MAIL_INFO_T *) SYS_EXISTED_DEVICE_WIFI_CONNECTED_EN_FUNC},
	{"EN", SYS_WAN_CABLE_UNPLUGGED_EVENT,             (MAIL_INFO_T *) SYS_WAN_CABLE_UNPLUGGED_EN_FUNC},
	{"EN", SYS_WAN_PPPOE_AUTH_FAILURE_EVENT,          (MAIL_INFO_T *) SYS_WAN_PPPOE_AUTH_FAILURE_EN_FUNC},
	{"EN", SYS_WAN_USB_MODEM_UNREADY_EVENT,           (MAIL_INFO_T *) SYS_WAN_USB_MODEM_UNREADY_EN_FUNC},
	{"EN", SYS_WAN_IP_CONFLICT_EVENT,                 (MAIL_INFO_T *) SYS_WAN_IP_CONFLICT_EN_FUNC},
	{"EN", SYS_WAN_UNABLE_CONNECT_PARENT_AP_EVENT,    (MAIL_INFO_T *) SYS_WAN_UNABLE_CONNECT_PARENT_AP_EN_FUNC},
	{"EN", SYS_WAN_MODEM_OFFLINE_EVENT,               (MAIL_INFO_T *) SYS_WAN_MODEM_OFFLINE_EN_FUNC},
	{"EN", SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT,       (MAIL_INFO_T *) SYS_WAN_GOT_PROBLEMS_FROM_ISP_EN_FUNC},
	{"EN", SYS_WAN_UNPUBLIC_IP_EVENT,                 (MAIL_INFO_T *) SYS_WAN_UNPUBLIC_IP_EN_FUNC},
	{"EN", SYS_ALL_WIFI_TURN_OFF_EVENT,               (MAIL_INFO_T *) SYS_ALL_WIFI_TURN_OFF_EN_FUNC},
	/* ------------------------------
	   ### Administration ###
	---------------------------------*/
	{"EN", ADMIN_LOGIN_FAIL_LAN_WEB_EVENT,            (MAIL_INFO_T *) ADMIN_LOGIN_FAIL_LAN_WEB_EN_FUNC},
	{"EN", ADMIN_LOGIN_FAIL_SSH_EVENT,                (MAIL_INFO_T *) ADMIN_LOGIN_FAIL_SSH_EN_FUNC},
	{"EN", ADMIN_LOGIN_FAIL_TELNET_EVENT,             (MAIL_INFO_T *) ADMIN_LOGIN_FAIL_TELNET_EN_FUNC},
	/* ------------------------------
	   ### Security ###
	---------------------------------*/
	{"EN", PROTECTION_VULNERABILITY_EVENT,            (MAIL_INFO_T *) PROTECTION_VULNERABILITY_EN_FUNC},
	{"EN", PROTECTION_CC_EVENT,                       (MAIL_INFO_T *) PROTECTION_CC_EN_FUNC},
	{"EN", PROTECTION_MALICIOUS_SITE_EVENT,           (MAIL_INFO_T *) PROTECTION_MALICIOUS_SITE_EN_FUNC},
	{"EN", PROTECTION_INFECTED_DEVICE_EVENT,          (MAIL_INFO_T *) PROTECTION_INFECTED_DEVICE_EN_FUNC},
	/* ------------------------------
	   ### USB Function ###
	---------------------------------*/
	{"EN", USB_TETHERING_EVENT,                       (MAIL_INFO_T *) USB_TETHERING_EN_FUNC},
	/* The End */
	{"",0,NULL}
};

/* define */
#define NT_TLD_PATH    "/jffs/.sys/tld/"

/* mailutility.cc */
static void  erase_symbol(char *old, char *sym);
static void  get_hostname_from_NMP(char *mac, char *hostname);
static void  print_mail_list(mail_s *head);
static void  free_mail_list(mail_s *head);
static void  extract_data(const char *path, FILE *new_f);

#endif

