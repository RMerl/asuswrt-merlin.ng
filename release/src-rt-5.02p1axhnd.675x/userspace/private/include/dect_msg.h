/***********************************************************************
 *
 *  Copyright (c) 2006-2009  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

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

#ifndef __DECT_MSG_H__
#define __DECT_MSG_H__



/*!\file dect_msg.h
 * \brief header file that defines messages used to communicate between dectd
 * and voice.
 */

#define DECT_MAX_HANDSET 6
#define DECT_IPUI_LEN    5

#define DECT_UNKNOWN_ID (-1)  /* ID for Line, handset, dspChannel, and connection is not specified */
#define DECT_WILDCARD_ID (0x7fff) /* wild card ID */

/* DECT events sent from the dectd to voice*/
typedef enum
{
   MSG_DECT_DIGIT0,
   MSG_DECT_DIGIT1,
   MSG_DECT_DIGIT2,
   MSG_DECT_DIGIT3,
   MSG_DECT_DIGIT4,
   MSG_DECT_DIGIT5,
   MSG_DECT_DIGIT6,
   MSG_DECT_DIGIT7,
   MSG_DECT_DIGIT8,
   MSG_DECT_DIGIT9,
   MSG_DECT_DIGITSTR,
   MSG_DECT_DIGITPND,
   MSG_DECT_FLASH,
   MSG_DECT_ONHOOK,
   MSG_DECT_OFFHOOK,
   MSG_DECT_HANDSETID,
   MSG_DECT_UNKNOWN
} MSG_DECT_EVENTS;


/** Data body for DECT event */
typedef struct
{
   MSG_DECT_EVENTS event;    /* DECT events */
   int lineId;               /* The line Id that the event should be associated*/
   int dstHandsetId;          /* The destination handset id for internal call */
   int handsetId;            /* The handset id that issues the event*/
}DectEventMsgBody;

/* The call states/events from the CM including following catagories:
   signals     ( handset[, call][, line]) 
   call states ( handset, call, line )
   media info  ( handset, call )
   event notification */
typedef enum
{
   /* signals destinated to handset */
   MSG_DECT_CM_SIG_RING,             /* Ring on/off */
   MSG_DECT_CM_SIG_DIALTONE,         /* Dial tone on/off */
   MSG_DECT_CM_SIG_RINGBACK,         /* Ringback tone on/off */
   MSG_DECT_CM_SIG_CWI,              /* Call Waiting tone on/off */
   MSG_DECT_CM_SIG_BUSY,             /* Busy tone on/off */   
   MSG_DECT_CM_SIG_INTERCEPT,        /* Intercept tone */
   MSG_DECT_CM_SIG_CONGESTION,       /* network congestion tone */
   MSG_DECT_CM_SIG_CONFIRM,          /* Confirmation tone */
   MSG_DECT_CM_SIG_ANSWER,           /* Answer tone */
   MSG_DECT_CM_SIG_WARNING,          /* hookoff warning tone */
   MSG_DECT_CM_SIG_CLID,             /* Caller ID, per call */
   MSG_DECT_CM_SIG_MWI,              /* Message Waiting Indication, per line */ 
 
   /* call states destinated to a call on a handset. Line ID is also relevant for external calls.
      line id will be invalid for internal call.  DECT_CSSR_REASONS are used for any reason of failures  */
   MSG_DECT_CM_CSS_IDLE,             /* Call released. DECT_CSSR_REASONS are used for release reasons */
   MSG_DECT_CM_CSS_SETUP,            /* new incoming call. Indicating an incoming external/internal call. 
                                        When dataLength is set to 1, the data[0] will be used to pass src handset id for internal call.
                                        When lineId is DECT_UNKNOWN_ID, this indicates an internal call*/
   MSG_DECT_CM_CSS_SETUP_ACK,        /* This is for CM to ack the outgoing call setup, when resource is available for the call requested*/
   MSG_DECT_CM_CSS_ALERTING,         /* For outgoing call, when remote received the call */
   MSG_DECT_CM_CSS_CONNECT,          /* media is connected */
   MSG_DECT_CM_CSS_HOLD,             /* call is on hold */
   MSG_DECT_CM_CSS_DISCONNECTING,    /* disconnecting the call */
   MSG_DECT_CM_CSS_CONF,             /* Conference established */
   MSG_DECT_CM_CSS_TRANSFER,         /* Call transferred */
   MSG_DECT_CM_CSS_INTERCEPT,        /* Call intercepted */
   MSG_DECT_CM_CSS_INTRUSION,        /* Intrusion call request indication for internal call only*/

   /* media information including DSP channel numbers and codec */
   MSG_DECT_CM_MEDIA_DSP_CHANNEL,    /* provide DSP channel number (0 based ) and ACK for call setup. 
                                        If the dataLength = 0, this is a NACK, and no DSP channel is available. */
   MSG_DECT_CM_MEDIA_CODEC,          /* codec selected by CM */ 

   /* Event notification for calls */
   MSG_DECT_CM_LIST_UPD,             /* LAS List update indication */
   MSG_DECT_CM_LIST_NEW,             /* LAS List update indication, for new entry */
   
   /* following events are for legacy code (call control 1.10), and should be removed after legacy code is obsoleted. */
   MSG_DECT_CM_RING_ON,
   MSG_DECT_CM_RING_OFF,
   MSG_DECT_CM_CLID,
   /* end of legacy events */
   
   MSG_DECT_CM_UNKNOWN,
   
} MSG_DECT_CM_EVENTS;

/** Data body for CM DECT event */
typedef struct
{
   MSG_DECT_CM_EVENTS event;     /* CM events or state including Signals, call state, media info, and notification*/
   int lineId;                   /* The line ID of the call. Passed in by CALLSTATE messages, and notifications */
   int handsetId;                /* The handset ID associated with the call. Passed in by all the messages*/
   int connectionId;             /* The connction ID associated with the call. Passed in by CALLSTATE and MEDIA messages, and maybe signals*/
   int dataLength;               /* The length of the data in the next field*/
   char data[1];                 /* customer data such as CLID, MWI, signal on/off, call end reasons, codec ...*/
}DectCMEventMsgBody;

#define DECT_CM_CLID_MAX_NAME_LENGTH 40
#define DECT_CM_CLID_MAX_NUMBER_LENGTH 40
#define DECT_CM_CLID_DATE_LENGTH 16
typedef struct
{
   char  date[DECT_CM_CLID_DATE_LENGTH + 1];         /* date string */
   char  number[DECT_CM_CLID_MAX_NUMBER_LENGTH + 1]; /* phone number string */
   char  reasonNoNumber;                             /* number block character */
   char  name[DECT_CM_CLID_MAX_NAME_LENGTH + 1];     /* name string */
   char  reasonNoName;                               /* name block character */

} DectCMClidType;


#define  DECT_CM_MWI_OFF 0
#define  DECT_CM_MWI_ON  1
#define  DECT_CM_TONE_OFF 0   /* the tone on/off is used for signals including ring, ringback, dial tone, call waiting indication, busy tone, ...*/
#define  DECT_CM_TONE_ON  1

/*  Call Control messages from the dectd*/
typedef enum
{
   MSG_DECT_CALL_CTL_SETUP,     /* setup an outgoing call, similar to off-hook, may come with dialed digits */
   MSG_DECT_CALL_CTL_SETUP_INT, /* setup an outgoing internal call, may come with handset id */
   MSG_DECT_CALL_CTL_ACCEPT,    /* accept an incoming call, similar to off-hook */
   MSG_DECT_CALL_CTL_RELEASE,   /* release a call, similar to on-hook */
   MSG_DECT_CALL_CTL_PARAL_INT, /* initiate an internal parallel call, like flash + # */
   MSG_DECT_CALL_CTL_PARAL_EXT, /* initiate an external parallel call, like flash + # */
   MSG_DECT_CALL_CTL_TOGGLE,    /* toggle a call */
   MSG_DECT_CALL_CTL_3WAY_CONF, /* 3-party conference call request */
   MSG_DECT_CALL_CTL_TRANSFER,  /* Call transfer request */
   MSG_DECT_CALL_CTL_REL_PARAL, /* Release parallel call */
   MSG_DECT_CALL_CTL_CW_ACCEPT, /* Call waiting acception */
   MSG_DECT_CALL_CTL_CW_REJECT, /* call waiting rejection */
   MSG_DECT_CALL_CTL_REPLACE,   /* active call release with replacement */
   MSG_DECT_CALL_CTL_INTRUSION, /* explicit call instrusion */
   MSG_DECT_CALL_CTL_HOLD,      /* Putting a call on-hold */
   MSG_DECT_CALL_CTL_UNHOLD,    /* resuming a cal put on-hold */
   MSG_DECT_CALL_CTL_INTERCEPT, /* call interception request */
} MSG_DECT_CALL_CTL_CMD;

typedef struct
{
   MSG_DECT_CALL_CTL_CMD cmd;   /* call control command */
   int lineId;                  /* lined Id selected for the call */
   int connectionId;            /* Connection ID selected for the call */
   int dstHandsetId;            /* Handset ID selected for the call */
   int srcHandsetId;            /* Handset id where the command originated from*/
   int dataLength;              /* custom data length */
   char data[1];                /* custom data */
}DectCallCtlCmdBody;

/* List Update indication messages sent from voice to dectd */
typedef enum
{
   MSG_DECT_LIST_MISSED_CALL,
   MSG_DECT_LIST_INCOMING_CALL,
   MSG_DECT_LIST_OUTGOING_CALL,
} MSG_DECT_LIST_TYPE;

/* dect delay message ids */
typedef enum
{
   DECT_DELAY_FLASH_SAVE,           /* delayed save to flash msg */
   DECT_DELAY_LAS_DB_READ,          /* large LAS list read requests are split into
                                     * smaller manageable portions. This message
                                     * restarts the list read operation.
                                     */
   DECT_DELAY_LIST_INCOMING,        /* incoming call list change notification */
   DECT_DELAY_LIST_OUTGOING,        /* outgoing call list change notification */
   DECT_DELAY_LIST_MISSED,          /* missed call list change notification */
   DECT_DELAY_LIST_LINE_SETTINGS,   /* DECT line settings list change notification */
   DECT_DELAY_LIST_INTERNAL_NAMES,  /* internal handset names list change notification */
} MSG_DECT_DELAY_MSG_ID;

/* call status reasons.  We intend to use this enum as byte type instead of int. So make sure not 
   expand the number of items over 255. */
typedef enum
{
   DECT_CSSR_NORMAL = 0, 
   DECT_CSSR_USER_BUSY,
   DECT_CSSR_USER_REJECTION,
   DECT_CSSR_USER_UNKNOWN,
   DECT_CSSR_CALL_RESTRICTION,
   DECT_CSSR_INSUFFICIENT_RESOURCES,
   DECT_CSSR_UNEXPECTED_MESSAGE,              
   DECT_CSSR_SERVICE_NOT_IMPLEMENTED,
   DECT_CSSR_INVALID_IDENTITY,
   DECT_CSSR_TIMER_EXPIRY, 
   DECT_CSSR_NEGOTIATION_FAILED,          
   DECT_CSSR_MAX
} DECT_CSSR_REASONS;

/* dect codec. We intend to use this enum as byte type instead of int. So make sure not expand 
   the number of items over 255.  We will not have that many codec supported by DECT anyway. */
typedef enum
{
   DECT_CODEC_G726 = 0,
   DECT_CODEC_G722
} DECT_CODEC_TYPE;

#endif // __DECT_MSG_H__
