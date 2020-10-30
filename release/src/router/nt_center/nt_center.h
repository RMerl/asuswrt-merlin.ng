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

#ifndef __nt_center_h__
#define __nt_center_h__

#include <libnt.h>
#ifdef SUPPORT_PUSH_MSG
#include <wb.h>
#include <nt_pushmsg.h>
#endif

/* NOTIFY CENTER CHECK EVENT ACTION STRUCTURE
---------------------------------*/

typedef struct __nt_check_action__t_
{
	int             event;
	int             action;
	char            msg[MAX_EVENT_INFO_LEN];

}NT_CHECK_ACTION_T;

#ifdef SUPPORT_PUSH_MSG
typedef struct __push_msg_conf__t_
{
	char            status[MAX_STATUS_LEN];
	char            server[MAX_URL_LEN];
	char            psr_server[MAX_URL_LEN];
	char            cusid[MAX_CUSID_LEN];
	char            deviceid[MAX_DEVICEID_LEN];
	char            deviceticket[MAX_DEVTICKET_LEN];
	char            devicetype[4];
	char            fwver[MAX_DESC_LEN];
	char            apilevel[4];
	char            modelname[MAX_STATUS_LEN];

}PUSHMSG_CONFIG_T;

PUSHMSG_CONFIG_T PushConf;

#define NC_VERSION           0
#define PUSH_MSG_MAX_LEN     4096
#define IFTTT_HOOK_MAX_LEN   512

#define IFTTT_PUSH_SERVER    "nwep.asus.com/router/ifttt/v1/triggers/"

/* PUSH MESSAGE RETURN CODE DEFINE
---------------------------------*/
#define PSM_SUCCESS          0   /* Success */
#define PSM_AUTH_FAIL        1   /* Authentication Fail */
#define PSM_NO_DEVICE        3   /* No device exist */
#define PSM_NO_USER          5   /* No user exist */
#define PSM_XML_IVLD         7   /* Invalid xml document */
#define PSM_DB_ERR           8   /* Database error */
#define PSM_NT_SERVER_ERR    11  /* Apple/Google notification service fail */
#define PSM_MATCH_RETRY      98  /* Reached the max retry */
#define PSM_PASER_ERR        99  /* Push Info parser error */

#endif

#endif
