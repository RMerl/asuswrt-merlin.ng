/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2011:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
 ************************************************************************/

#ifndef __CELLULAR_MSG_H_
#define __CELLULAR_MSG_H_

#include "cms_msg.h"
#include "cms_eid.h"
#include "cms.h"


/*************************************
*Below structure match from ril.h.
************************************/

/*From RIL_RadioState */
typedef enum
{
    DEVCELL_RADIO_STATE_OFF = 0,                   /* Radio explictly powered off (eg CFUN=0) */
    DEVCELL_RADIO_STATE_UNAVAILABLE = 1,           /* Radio unavailable (eg, resetting or not booted) */
    DEVCELL_RADIO_STATE_ON = 10                    /* Radio is on */
} DevCellularRadioState;


/*************************************
*Message type and body used by CellularApp
*and CMS.
************************************/
typedef enum
{
/* For CMS_MSG_CELLULARAPP_GET_REQUEST */
   DEVCELL_ROAMINGSTATUS,                              /* Device.Cellular.RoamingStatus, response is char*  ---- Phase 2 */
   DEVCELL_INT_IMEI,                                   /* Device.Cellular.Interface.{i}.IMEI, response is char* */
   DEVCELL_INT_CURRACCESSTECH,                         /* Device.Cellular.Interface.{i}.CurrentAccessTechnology, response is char* */
   DEVCELL_INT_AVAILNETWORKS,                          /* Device.Cellular.Interface.{i}.AvailableNetworks, response is char* ---- Phase 2 */
   DEVCELL_INT_NETWORKINUSE,                           /* Device.Cellular.Interface.{i}.NetworkInUse, response is char* */
   DEVCELL_INT_RSSI,                                   /* Device.Cellular.Interface.{i}.RSSI,  response is int* */
   DEVCELL_INT_UPMAXBITRATE,                           /* Device.Cellular.Interface.{i}.UpstreamMaxBitRate, response is unsignedInt* */
   DEVCELL_INT_DOWNMAXBITRATE,                         /* Device.Cellular.Interface.{i}.DownstreamMaxBitRate, response is unsignedInt* */
   DEVCELL_INT_USIMSTATUS,                             /* Device.Cellular.Interface.{i}.USIM.Status,  response is char* */
   DEVCELL_INT_USIMIMSI,                               /* Device.Cellular.Interface.{i}.USIM.IMSI, response is char* */
   DEVCELL_INT_USIMICCID,                              /* Device.Cellular.Interface.{i}.USIM.ICCID, response is char* */
   DEVCELL_INT_USIMMSISDN,                             /* Device.Cellular.Interface.{i}.USIM.MSISDN, response is char* */
   DEVCELL_INT_USIMMCCMNC,                             /* SIM MCC MNC, response is char* */
   DEVCELL_CALL_GET_CURRENT,
   
/* For CMS_MSG_CELLULARAPP_SET_REQUEST */
   DEVCELL_ROAMINGENABLED,                             /* Device.Cellular.RoamingEnabled, request data is ubool8* ---- Phase 2 */
   DEVCELL_INT_ENABLE,                                 /* Device.Cellular.Interface.{i}.Enable, request data is ubool8* */
   DEVCELL_INT_PREFERREDACCESSTECH,                    /* Device.Cellular.Interface.{i}.PreferredAccessTechnology, request data is char* */
   DEVCELL_INT_NETWORKREQUESTED,                       /* Device.Cellular.Interface.{i}.NetworkRequested, request data is char* ---- Phase 2 */
   DEVCELL_INT_USIMPINCHECK,                           /* Device.Cellular.Interface.{i}.USIM.PINCheck ---- Phase 2 */
   DEVCELL_INT_USIMPIN,                                /* Device.Cellular.Interface.{i}.USIM.PIN ---- Phase 2 */
   DEVCELL_ACCESSPOINTENABLE,                          /* Device.Cellular.AccessPoint.{i}.Enable, requets data is DevCellularAccessPointObjBody* */
   DEVCELL_CALL_DIAL,
   DEVCELL_CALL_HANGUP,
   DEVCELL_CALL_ANSWER,
   DEVCELL_SMS_DELETE_SMS_ON_SIM,                      /* Delete a SMS on SIM, request data is int* */

/* For CMS_MSG_CELLULARAPP_GET_REQUEST and CMS_MSG_CELLULARAPP_SET_REQUEST */
   DEVCELL_SMS_SMSC_ADDRESS,                           /* get or set SMSC address, request data is char *, response is char* */

/* For CMS_MSG_CELLULARAPP_NOTIFY_EVENT */
   DEVCELL_EVENT_DATACALLSTATECHANGED,                 /* Data call state change, event data is DevCellularDataCallStateChangedBody* */
   DEVCELL_EVENT_RADIOSTATECHANGED,                    /* Radio state change, event data is DevCellularRadioStateChangedBody* */
   DEVCELL_EVENT_CALLSTATECHANGED,
   DEVCELL_EVENT_RINGBACKTONE,
   DEVCELL_EVENT_SMS_STORAGE_FULL,                     /*  Indicates that SMS storage on the SIM is full, event data is NULL */
   DEVCELL_EVENT_SMS_NEW_SMS,                          /* Indicate when new SMS is received, event data is char* */
   
   DEVCELLNOSUPPORT,
}CellularMsgObj;


typedef struct
{
   UBOOL8 enable;
   char ifname[BUFLEN_8];
   char APN[BUFLEN_64];
   WanConnL3Type ipProtocalMode;
//   char username[BUFLEN_256];   //for PPP, not support now
//   char password[BUFLEN_256];   //for PPP, not support now
//   char authType[BUFLEN_8];       //for PPP, not support now
//   char proxy[BUFLEN_48];          // not support now
//   UINT32 proxyport;                  // not support now
//   char interface[BUFLEN_256];   // not support now
} DevCellularAccessPointMsgBody;

typedef struct
{
   UINT8 active;  /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
   char ifname[BUFLEN_8];  /* The network interface name */
   char addresses[BUFLEN_128]; /* A space-delimited list of addresses with optional "/" prefix length,
                                   e.g., "192.0.1.3" or "192.0.1.11/16 2001:db8::1/64".
                                   May not be empty, typically 1 IPv4 or 1 IPv6 or
                                   one of each. If the prefix length is absent the addresses
                                   are assumed to be point to point with IPv4 having a prefix
                                   length of 32 and IPv6 128. */
   char dnses[BUFLEN_128];  /* A space-delimited list of DNS server addresses,
                                   e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
                                   May be empty. */
   char gateways[BUFLEN_64];  /* A space-delimited list of default gateway addresses,
                                   e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
                                   May be empty in which case the addresses represent point
                                   to point connections. */
} DevCellularDataCallStateChangedBody;

typedef struct
{
   char address[BUFLEN_256];
   SINT32 clirMode;
   UINT32 uusType;
   UINT32 uusDcs;
   UINT32 uusLength;
   char uusData[BUFLEN_256];
} DevCellularCallDialMsgBody;

typedef struct
{
   UINT32 state;
   SINT32 index;
   SINT32 toa;
   UBOOL8 isMpty;
   UBOOL8 isMT;
   char als;

   UBOOL8 isVoice;
   UBOOL8 isVoicePrivacy;
   char number[BUFLEN_128];
   SINT32 numberPresentation;
   char name[BUFLEN_128];
   SINT32 namePresentation;
   
   UINT32 uusType;
   UINT32 uusDcs;
   UINT32 uusLength;
   char uusData[BUFLEN_256];
   
   char isVideoCall;    /* nonzero if this is is a video call vt added VideoPhone */	    
   char callType;   /* CS Call over UTRAN/GERAN : 0, IMS VoLTE over EUTRAN : 1, UNDEFINED : 0xFF */
} DevCellularCallListMsgBody;

typedef struct
{
    UBOOL8 isOnSim;
    SINT32 index;
    UINT32 length;
    UINT8 pdu[BUFLEN_256];
} DevCellularNewSmsMsgBody;

typedef struct
{
    SINT32 gsmIndex;
} DevCellularCallHangupMsgBody;

typedef struct
{
//   char interface[BUFLEN_256];   // not support now
   DevCellularRadioState state;
} DevCellularRadioStateChangedBody;

/*************************************
 *Below structure are defined for CMS_MSG_CELLULARAPP_GET_RIL_REQUEST
 *Should be comply with structure defined in ril.h
************************************/

/*ril.h->RIL_REQUEST_VOICE_REGISTRATION_STATE*/
typedef struct
{
   char regState[BUFLEN_64];
   char regDenyReason[BUFLEN_64];
} GetRilReqVoiceRegStateBody;

/*ril.h->RIL_REQUEST_DATA_REGISTRATION_STATE*/
typedef struct
{
   char regState[BUFLEN_64];
   char regDenyReason[BUFLEN_64];
   char availDataRadioTech[BUFLEN_64];
} GetRilReqDataRegStateBody;

/*ril.h->RIL_REQUEST_DATA_REGISTRATION_STATE*/
typedef struct
{
   char regState[BUFLEN_64];
   char radioTechnology[BUFLEN_64];
} GetRilReqIMSRegStateBody;

/*************************************
*Below structure is for furture use to support mutiple celluar interface.
************************************/
typedef struct
{
    UINT32 instance_id : 16;
    UINT32 req_cellular_obj : 16;
} CelluluarMsgWordData;


#endif // __CELLULAR_MSG_H_
