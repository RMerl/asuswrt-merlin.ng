/*
*
*  Filename: dal_voice.h
*
****************************************************************************
*  Description:
*
*
****************************************************************************/

#ifndef __DAL_VOICE_H__
#define __DAL_VOICE_H__

#include <cms.h>
#include <mdm_object.h>

/* Defines and Data structures */
#define ANY_INSTANCE             (-1)
#define DAL_MAX_INSTANCE_IN_LIST  100
#define DAL_VOICE_MAX_VOIP_ARGS 6             /* Maximum number of voip arguments */
#define DAL_VOICE_MAXLEVELS 10                 /* Maximum levels allowed in TR-104 tree */
#define STRMAXSIZE 512              /* Maximum string size for firewall filter fields */
/* Size of the maximum TR104 string is 389 characters (for URI).
** We add some margin if we need to allocate memory for any TR104 object
** without knowing which object we deal with. */
#define MAX_TR104_OBJ_SIZE                512


#ifdef DMP_X_BROADCOM_COM_DECTENDPOINT_1
/* The string that defines an invalid handset unique identifier.
*/
#   define DAL_VOICE_DECT_INVALID_HSET      "0000000000"
#endif /* DMP_X_BROADCOM_COM_DECTENDPOINT_1 */

/* Argument block to be passed to DAL functions */
typedef struct {                          /* Voice arguments should include all heirarchical instance numbers */
    void * args[DAL_VOICE_MAX_VOIP_ARGS];  /* from VoiceProfile onwards then the value that needs to be written*/
} VOICE_ARG_BLK;                              /* Instance num for voiceService is not reqd                        */

/* Argument block to be passed to DAL functions from provisioning interface */
typedef struct {
    int    op[DAL_VOICE_MAX_VOIP_ARGS];
    void*  msgHandle;
} DAL_VOICE_PARMS;

/* Enum to determine the type of call feature operation */
typedef enum {
   CFCALLTYPE_ENABLED,
   CFCALLTYPE_STARTED,
} CFCALLTYPE;

typedef enum
{
   TERMTYPE_DECT,
   TERMTYPE_FXS,
   TERMTYPE_NOSIG,
   TERMTYPE_FXO,
#if defined( DMP_VOICE_SERVICE_2 )
   TERMTYPE_SIP,
#endif
   TERMTYPE_UNKNOWN
} TERMTYPE;

/* Enum for callfeature codes */
typedef enum
{
   DAL_VOICE_FEATURE_CODE_CALLWAIT,        /* Enable CallWaiting feature */
   DAL_VOICE_FEATURE_CODE_FWDNUMBER,       /* Set CallForward dialstring */
   DAL_VOICE_FEATURE_CODE_FWD_OFF,         /* Disable all call forwarding */
   DAL_VOICE_FEATURE_CODE_FWD_NOANS,       /* Forward calls on NoAnswer */
   DAL_VOICE_FEATURE_CODE_FWD_BUSY,        /* Forward calls if Busy */
   DAL_VOICE_FEATURE_CODE_FWD_ALL,         /* Forward all calls */
   DAL_VOICE_FEATURE_CODE_CALLRETURN,      /* Return call */
   DAL_VOICE_FEATURE_CODE_CALLREDIAL,      /* Redial call */
   DAL_VOICE_FEATURE_CODE_CCBS,            /* Activate CCBS Feature */
   DAL_VOICE_FEATURE_CODE_SPEEDDIAL,       /* Speed-dial call */
   DAL_VOICE_FEATURE_CODE_WARM_LINE,       /* Activate warm line */
   DAL_VOICE_FEATURE_CODE_ANON_REJECT,     /* Activate anonymous call rejection */
   DAL_VOICE_FEATURE_CODE_ANON_CALL,       /* Activate permanent CID blocking */
   DAL_VOICE_FEATURE_CODE_CALL_BARRING,    /* Outgoing call barring feature */
   DAL_VOICE_FEATURE_CODE_DND,             /* Activate do-not-disturb */
   DAL_VOICE_FEATURE_CODE_NET_PRIV,        /* Activate network privacy */
   DAL_VOICE_FEATURE_CODE_EUROFLASH,       /* European flash */
   /*
   ** The following features are not controlled via feature code per
   ** say, but they are part of the overall feature framework, so define
   ** them here.
   */
   DAL_VOICE_FEATURE_CODE_TRANSFER,        /* Call transfer */
   DAL_VOICE_FEATURE_CODE_CONFERENCING,    /* Call conferencing (3-ways calling in this case) */
   DAL_VOICE_FEATURE_CODE_ENUM,            /* Enum lookup support */
   DAL_VOICE_FEATURE_CODE_MWI,            /* message waiting indication */
   DAL_VOICE_FEATURE_CODE_VMWI,            /* visual message waiting indication */
   DAL_VOICE_FEATURE_CODE_CID,            /* display CID on local line */
   DAL_VOICE_FEATURE_CODE_CID_NAME,       /* display CID name on local line */
   /* 
    * Emergency Calling handle differently than regular call,
    * never the less, we treat emergency service flags as special call 
    * features as they control how the call been handled
    */
   DAL_VOICE_FEATURE_ESVC_NETHOLDTIME,    /* Network holding time in Emergency Service */
   DAL_VOICE_FEATURE_ESVC_DSCP,           /* DSCP mark for Emergency Service */
   DAL_VOICE_FEATURE_ESVC_END_CALL,       /* End all non-emergency calls, accept incoming emergency call */
   DAL_VOICE_FEATURE_ESVC_NO_LOC_INFO,    /* Not include location info in Emergency Service */
   DAL_VOICE_FEATURE_ESVC_ENABLE_3WAY,    /* Enable 3way calling in Emergency Service */
   DAL_VOICE_FEATURE_ESVC_NETHOLDDISABLE, /* disable network hold in Emergency Service */
   DAL_VOICE_FEATURE_ESVC_NETHOLDBYPASS,  /* bypass network hold check in Emergency Service */

   MAX_DAL_VOICE_FEATURE_CODE

} DAL_VOICE_FEATURE_CODE;

/* Enum for dtmf relay types */
typedef enum
{
   DAL_VOICE_DTMF_RELAY_NONE, /* i.e InBand */
   DAL_VOICE_DTMF_RELAY_RTP,  /* i.e rfc2833 */
   DAL_VOICE_DTMF_RELAY_SIPINFO

}  DAL_VOICE_DTMF_RELAY_SERVICE;

/* Enum for sip transports */
typedef enum
{
   DAL_VOICE_SIP_TRANSPORT_UDP,
   DAL_VOICE_SIP_TRANSPORT_TCP,
   DAL_VOICE_SIP_TRANSPORT_TLS,
   DAL_VOICE_SIP_TRANSPORT_SCTP
} DAL_VOICE_SIP_TRANSPORTS;

/* Enum for SRTP usage options */
typedef enum
{
   DAL_VOICE_SRTP_OPTIONAL,
   DAL_VOICE_SRTP_MANDATORY,
   DAL_VOICE_SRTP_DISABLED,
   DAL_VOICE_SRTP_MAX
} DAL_VOICE_SRTP_OPTIONS;

/* Enum for back-to-primary options */
typedef enum
{
   DAL_VOICE_BACKTOPRIM_DISABLED,
   DAL_VOICE_BACKTOPRIM_SILENT,
   DAL_VOICE_BACKTOPRIM_DEREG,
   DAL_VOICE_BACKTOPRIM_SILENT_DEREG,
   DAL_VOICE_BACKTOPRIM_MAX
} DAL_VOICE_BACKTOPRIM_OPTIONS;

/* Firewall control block */
typedef struct
{
   UINT32   enable;                           /* Enable filter */
   char     filterName[STRMAXSIZE];              /* Filter Name */
   char     protocol[STRMAXSIZE];              /* Transport Protocol */
   UINT32   sourcePort;                          /* Source Port */
   UINT32   destinationPort;                  /* Destination Port */
   char     sourceIPAddress[STRMAXSIZE];      /* Source IP Address */
   char     sourceNetMask[STRMAXSIZE];          /* Source Subnet Mask */
   char     destinationIPAddress[STRMAXSIZE]; /* Destination IP Address */
   char     destinationNetMask[STRMAXSIZE];      /* Destination Subnet Mask */
} DAL_VOICE_FIREWALL_CTL_BLK;

/* call types */
typedef enum
{
   INCOMING = 0,
   OUTGOING,
   MISSED,
   VOICE_CALL_TYPE_MAX
} VOICECALLTYPE;

/* call list stats */
typedef struct
{
   UINT32   numMissed;
   UINT32   numUnreadMissed;
   UINT32   numIncoming;
   UINT32   numOutgoing;
} DAL_VOICE_DECT_CALL_LIST_STATS;

/* call list stats */
typedef struct
{
    char      linkDate[STRMAXSIZE];	/**< X_BROADCOM_COM_LinkDate */
    UINT32    type;	                /**< X_BROADCOM_COM_Type */
    char      dectId[STRMAXSIZE];	/**< X_BROADCOM_COM_DectId */
    UINT32    manic;	            /**< X_BROADCOM_COM_MANIC */
    UINT32    modic;	            /**< X_BROADCOM_COM_MODIC */
    UINT32    numberOfHandsets;	    /**< NumberOfHandsets */
    UINT32    maxNumberOfHandsets;	/**< X_BROADCOM_COM_MaxNumberOfHandsets */
    UBOOL8    waitingSubscription;	/**< WaitingSubscription */
    UBOOL8    serviceEnabled;	    /**< X_BROADCOM_COM_ServiceEnabled */
    char      accessCode[STRMAXSIZE];	/**< X_BROADCOM_COM_AccessCode */
} DAL_VOICE_DECT_INFO;

#ifdef DMP_VOICE_SERVICE_1
#include <dal_voice_v1.h>
#endif

#ifdef DMP_VOICE_SERVICE_2
#include <dal_voice_v2.h>
#endif

#endif /* __DAL_VOICE_H__ */

