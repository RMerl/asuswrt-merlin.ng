/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef __OMCI_PM_H__
#define __OMCI_PM_H__

/*!\file omci_pm.h
 * Performance Monitoring definitions for generic GPON functionality.
 * Definitions and APIs are used by omcipmd app and library
 */



#include "cms_core.h"
#include "cms_msg.h"

#include "omci_api.h"
#include "omcipm_api.h"
#include "bcm_common_llist.h"

/**
 * Parameter Tags indicating whether the parameter is an input, output, or input/output argument
 **/

#ifndef IN
#define IN
#endif /*IN*/

#ifndef OUT
#define OUT
#endif /*OUT*/

#ifndef INOUT
#define INOUT
#endif /*INOUT*/

/* #define ENABLE_PM_DEBUG 1 */

#define MSECS_PER_SEC 1000
#define SECS_PER_MINUTE 60

#define MAX_THRESHOLD_VALUE 0xFFFFFFFF

#ifdef ENABLE_PM_DEBUG
#define WAIT_ALARM_TEST_TIME (MSECS_PER_SEC * 1)
#define WAIT_ALARM_ARC_TIME (MSECS_PER_SEC * 8)
#define WAIT_MULTICAST_TIME (MSECS_PER_SEC * 8)
#define WAIT_GPON_TIME (MSECS_PER_SEC * 1)
#define WAIT_ENET_TIME (MSECS_PER_SEC * 1)
#define WAIT_ENET2_TIME (MSECS_PER_SEC * 1)
#define WAIT_ENET3_TIME (MSECS_PER_SEC * 1)
#define WAIT_ENETDN_TIME (MSECS_PER_SEC * 1)
#define WAIT_ENETUP_TIME (MSECS_PER_SEC * 1)
#define WAIT_FEC_TIME (MSECS_PER_SEC * 1)
#define WAIT_GAL_ENET_TIME (MSECS_PER_SEC * 1)
#define WAIT_VOIP_TIME (MSECS_PER_SEC * 1)
#define WAIT_MOCA_ENET_TIME (MSECS_PER_SEC * 1)
#define WAIT_MOCA_INTF_TIME (MSECS_PER_SEC * 1)
#define WAIT_IP_HOST_TIME (MSECS_PER_SEC * 1)
#define WAIT_TEST_TIME (MSECS_PER_SEC * 1)
#define WAIT_15_MIN (MSECS_PER_SEC * 15)
#define WAIT_MAC_BRIDGE_TIME (MSECS_PER_SEC * 1)
#define WAIT_EXTPM_TIME (MSECS_PER_SEC * 1)
#else /* ENABLE_PM_DEBUG */
#define WAIT_ALARM_TEST_TIME (MSECS_PER_SEC * 1)                  // 1 second
#define WAIT_ALARM_ARC_TIME (MSECS_PER_SEC * 8)                   // 8 second
#define WAIT_MULTICAST_TIME (MSECS_PER_SEC * 8)                   // 8 second
#define WAIT_GPON_TIME (MSECS_PER_SEC * 15)                       // 15 seconds
#define WAIT_ENET_TIME (MSECS_PER_SEC * 15)                       // 15 seconds
#define WAIT_ENET2_TIME (MSECS_PER_SEC * SECS_PER_MINUTE * 1)     // 1 minute
#define WAIT_ENET3_TIME (MSECS_PER_SEC * 30)                      // 30 seconds
#define WAIT_ENETDN_TIME (MSECS_PER_SEC * 30)                     // 30 seconds
#define WAIT_ENETUP_TIME (MSECS_PER_SEC * 30)                     // 30 seconds
#define WAIT_FEC_TIME (MSECS_PER_SEC * 30)                        // 30 seconds
#define WAIT_GAL_ENET_TIME (MSECS_PER_SEC * SECS_PER_MINUTE * 1)  // 1 minute
#define WAIT_VOIP_TIME (MSECS_PER_SEC * SECS_PER_MINUTE * 1)      // 1 minute
#define WAIT_MOCA_ENET_TIME (MSECS_PER_SEC * 30)                  // 30 seconds
#define WAIT_MOCA_INTF_TIME (MSECS_PER_SEC * SECS_PER_MINUTE * 1) // 1 minute
#define WAIT_IP_HOST_TIME (MSECS_PER_SEC * SECS_PER_MINUTE * 1)   // 1 minute
#define WAIT_TEST_TIME (MSECS_PER_SEC * 1)                        // 1 second
#define WAIT_15_MIN (MSECS_PER_SEC * SECS_PER_MINUTE * 15)        // 15 minutes (correct OMCI spec interval duration)
#define WAIT_MAC_BRIDGE_TIME (MSECS_PER_SEC * SECS_PER_MINUTE * 1)// 1 minute
#define WAIT_EXTPM_TIME (MSECS_PER_SEC * 30)                      // 30 seconds
#endif /* ENABLE_PM_DEBUG */

#define INIT_ALARM_SEQ_VALUE 1

#define MOCA_PORT_MAX 1
#define GPON_PORT_MAX CONFIG_BCM_MAX_GEM_PORTS

#define INVALID_GPON_PORT 0xFFFF

#define CMS_LOCK_TIMEOUT_MSEC 300   // Wait 1/3 sec
#define CMS_LOCK_TIMEOUT_6SEC 6000  // Wait 6 secs


#define OMCI_MESSAGE_TYPE_ALARM     0x10

#define INVALID_OBJ_ID              0x80000000

#define OMCI_LINE_SENSE_UNK   0x00
#define OMCI_LINE_SENSE_10    0x01
#define OMCI_LINE_SENSE_100   0x02
#define OMCI_LINE_SENSE_1000  0x03
#define OMCI_LINE_FULL_DUPLEX 0x10
#define OMCI_LINE_HALF_DUPLEX 0x10  // USED only on Ethernet PPTP Config Ind

#define ENET_ADMIN_STATE_UNLOCKED 0
#define ENET_ADMIN_STATE_LOCKED 1
#define VOIP_ADMIN_STATE_UNLOCKED 0
#define VOIP_ADMIN_STATE_LOCKED 1

#define PORT_VEIP 0xfff

#ifdef BRCM_FTTDP
#define ETHERNET_PORT_MAX 24
#else
#define ETHERNET_PORT_MAX 8
#endif

#define EXT_PM_CONTROL_BLOCK_SIZE 16

#define MAC_BRIDGE_ETHERNET_PPTP_TYPE    1
#define MAC_BRIDGE_1P_MAPPER_TYPE        3
#define MAC_BRIDGE_IWTP_TYPE             5
#define MAC_BRIDGE_MCAST_IWTP_TYPE       6
#define MAC_BRIDGE_VEIP_TYPE             11
#define MAC_BRIDGE_MOCA_TYPE             12

enum
{
  GEM_INTERWORK_PORT_0 = 0,
  GEM_INTERWORK_1,
  GEM_INTERWORK_2,
  GEM_INTERWORK_3,
  GEM_INTERWORK_MAX
};


enum
{
  POTS_PORT_0 = 0,
  POTS_PORT_1,
  POTS_PORT_2,
  POTS_PORT_3,
  POTS_PORT_MAX
};


enum
{
  VOIP_PORT_0 = 0,
  VOIP_PORT_1,
  VOIP_PORT_MAX
};


enum
{
  INDEX_MDMOID_SERVICES = 0,
  INDEX_MDMOID_VOICE,
  INDEX_MDMOID_VOICE_LINE,
  INDEX_MDMOID_VOICE_LINE_SIP
};


enum
{
  OMCIPMD_CMD_MSG_CREATE = 0,
  OMCIPMD_CMD_MSG_GET,
  OMCIPMD_CMD_MSG_GET_NEXT,
  OMCIPMD_CMD_MSG_SET,
  OMCIPMD_CMD_MSG_DELETE,
  OMCIPMD_CMD_MSG_MAX
};


typedef enum
{
  OMCI_PM_GET_15_MINUTES = 0,
  OMCI_PM_GET_CURRENT
} BCM_OMCI_PM_GET_TYPE;


typedef struct
{
  INOUT UINT8 state;
  INOUT UINT8 classID;
  INOUT UINT16 portID;
} BCM_OMCI_PM_DEBUG, *PBCM_OMCI_PM_DEBUG;


typedef struct
{
  INOUT UINT16 objType;
  INOUT UINT16 objID;
  INOUT UINT16 portID;
  INOUT UINT16 thresholdID;
  INOUT UINT8  interval_End;
  INOUT UINT8  tpType;
  INOUT UINT16 parentMeClass;
  INOUT UINT16 parentMeInstance;
  INOUT UINT16 accumulationDisable;
  INOUT UINT16 tcaDisable;
  INOUT UINT16 controlFields;
  INOUT UINT16 tci;
} BCM_OMCI_PM, *PBCM_OMCI_PM;


typedef struct
{
  INOUT UINT32 msgData_1;
  INOUT UINT32 msgData_2;
  INOUT UINT32 msgData_3;
  INOUT UINT32 msgData_4;
  INOUT UINT32 msgData_5;
  INOUT UINT32 msgData_6;
  INOUT UINT32 msgData_7;
  INOUT UINT32 msgData_8;
  INOUT UINT32 msgData_9;
  INOUT UINT32 msgData_10;
  INOUT UINT32 msgData_11;
  INOUT UINT32 msgData_12;
  INOUT UINT32 msgData_13;
  INOUT UINT32 msgData_14;
  INOUT UINT32 msgData_15;
  INOUT UINT32 msgData_16;
  INOUT UINT32 msgData_17;
  INOUT UINT32 msgData_18;
} BCM_OMCI_RSP_DATA, *PBCM_OMCI_RSP_DATA;


typedef struct
{
  CmsMsgHeader cmsMsgHdr;
  BCM_OMCI_PM omcipm;
  UINT32 result;
  UINT32 newMeID;
  BCM_OMCI_RSP_DATA msgData;
} BCM_OMCI_PM_MSG_DATA;


typedef struct
{
  BCM_OMCI_PM omcipm;
} BCM_OMCI_PM_ID_ONLY, *PBCM_OMCI_PM_ID_ONLY;


typedef enum
{
  OMCIPM_COUNTER_TYPE_A = 0,
  OMCIPM_COUNTER_TYPE_B,
  OMCIPM_COUNTER_TYPE_BASE,
  OMCIPM_COUNTER_TYPE_MAX
} BCM_OMCIPM_COUNTER_TYPE;


typedef struct
{
  BCM_COMMON_DECLARE_LL_ENTRY ();
  BCM_OMCI_PM omcipm;
  UINT16 reportedAlarmBits;
  UINT8 initBaseFlag;
  UINT8 debugFlag;
  void* pm[OMCIPM_COUNTER_TYPE_MAX];
} BCM_OMCIPM_ENTRY, *PBCM_OMCIPM_ENTRY;


typedef struct
{
  BCM_COMMON_DECLARE_LL_ENTRY ();
  UINT16 objectID;
  UINT32 thresholdValue1;
  UINT32 thresholdValue2;
  UINT32 thresholdValue3;
  UINT32 thresholdValue4;
  UINT32 thresholdValue5;
  UINT32 thresholdValue6;
  UINT32 thresholdValue7;
  UINT32 thresholdValue8;
  UINT32 thresholdValue9;
  UINT32 thresholdValue10;
  UINT32 thresholdValue11;
  UINT32 thresholdValue12;
  UINT32 thresholdValue13;
  UINT32 thresholdValue14;
} BCM_OMCIPM_THRESHOLD_ENTRY, *PBCM_OMCIPM_THRESHOLD_ENTRY;


typedef struct
{
    OUT   UINT64 receivedPayloadBytes_64;
    OUT   UINT64 transmittedPayloadBytes_64;
    OUT   UINT32 transmittedGEMFrames;
    OUT   UINT32 receivedGEMFrames;
    OUT   UINT32 receivedPayloadBytes_32;
    OUT   UINT32 transmittedPayloadBytes_32;
} BCM_OMCI_PM_GEM_PORT_COUNTER_64, *PBCM_OMCI_PM_GEM_PORT_COUNTER_64;


/*========================== ALARM Definitions ================*/


extern UINT32 ethLanNum;
extern int rgPortsMask;

typedef void (*ALARM_CB_FUNC)(void);
typedef UINT8 (*ARC_CB_FUNC)(UINT32 cbParam);

#define BCM_OMCI_FILE_NAME          "/dev/bcm_omci"

// NOTE Alarm bit positions start at 0.
// AniG Alarms
#define BCM_ALARM_ID_ANIG_LO_RX_OPTICAL_PWR        (0x8000 >> 0)
#define BCM_ALARM_ID_ANIG_HI_RX_OPTICAL_PWR        (0x8000 >> 1)
#define BCM_ALARM_ID_ANIG_SF_VAL                   (0x8000 >> 2)
#define BCM_ALARM_ID_ANIG_SD_VAL                   (0x8000 >> 3)
#define BCM_ALARM_ID_ANIG_LO_TX_OPTICAL_PWR        (0x8000 >> 4)
#define BCM_ALARM_ID_ANIG_HI_TX_OPTICAL_PWR        (0x8000 >> 5)
#define BCM_ALARM_ID_ANIG_LASER_BIAS_CURRENT       (0x8000 >> 6)

// Ethenet UNI Alarm
#define BCM_ALARM_ID_LOS_VAL                  (0x8000)
// OntG Alarms
#define BCM_ALARM_ID_ONTG_POWERING_VAL        (0x8000 >> 1)
#define BCM_ALARM_ID_ONTG_BATTERY_MISSING_VAL (0x8000 >> 2)
#define BCM_ALARM_ID_ONTG_BATTERY_FAILURE_VAL (0x8000 >> 3)
#define BCM_ALARM_ID_ONTG_BATTERY_LOW_VAL     (0x8000 >> 4)

// NOTE AVC alarm bit positions start at 1, not 0, so shifts need to be adjusted.
#define BCM_AVC_ANIG_ARC_VAL  (0x8000 >> (8 - 1))
#define BCM_AVC_ENET_ARC_VAL  (0x8000 >> (12 - 1))
#define BCM_AVC_ENET_LINE_VAL (0x8000 >> (2 - 1))
#define BCM_AVC_POTS_OP_VAL   (0x8000 >> (9 - 1))

#define BCM_SOAK_DECLARE_SECS 0   // NOTE: Change this value to allow an alarm declaration soak time in seconds (OMCI spec recommends 2.5 +/- 0.5 seconds)
#define BCM_SOAK_RETIRE_SECS 0    // NOTE: Change this value to allow an alarm retire soak time in seconds (OMCI spec recommends 10.5 +/- 0.5 seconds)

#define ARC_INTERVAL_SELFTEST 0
#define ARC_INTERVAL_INDEF 255

#define LINE_SENSE_10_100_1000 47


#define DEFAULT_RX_THRESHOLD_VAL_NP (0xFF)
#define DEFAULT_TX_THRESHOLD_VAL_NP (0x81)

enum
{
    ALARM_SOAK_GPON_SF = 0,
    ALARM_SOAK_GPON_SD,
    ALARM_SOAK_ANIG_LO_RX_PWR,
    ALARM_SOAK_ANIG_HI_RX_PWR,
    ALARM_SOAK_ANIG_LO_TX_PWR,
    ALARM_SOAK_ANIG_HI_TX_PWR,
    ALARM_SOAK_ENET_LOS_PORT0,
    ALARM_SOAK_ENET_LOS_PORT1,
    ALARM_SOAK_ENET_LOS_PORT2,
    ALARM_SOAK_ENET_LOS_PORT3,
    ALARM_SOAK_ONTG_POWERING,
    ALARM_SOAK_ONTG_BATTERY_MISSING,
    ALARM_SOAK_ONTG_BATTERY_FAILURE,
    ALARM_SOAK_ONTG_BATTERY_LOW,
    MAX_ALARM_SOAK_LIST
};

typedef enum
{
    ALARM_FREQ_1_SEC = 0,
    ALARM_FREQ_2_SEC,
    ALARM_FREQ_4_SEC,
    ALARM_FREQ_8_SEC,
    MAX_ALARM_LIST
} ALARM_TIMER_FREQ;

typedef enum
{
    ALARM_SOAK_IDLE = 0,
    ALARM_SOAK_ACTIVE,
    ALARM_SOAK_DECLARE,
    ALARM_SOAK_RETIRE
} ALARM_SOAK_TYPE;

typedef enum
{
    ALARM_POLL_TIMER_ENET = 0,
    ALARM_POLL_TIMER_ANI_G,
    ALARM_POLL_TIMER_ARC,
    ALARM_POLL_TIMER_BMU,
    ALARM_POLL_TIMER_MAX
} ALARM_TIMER_TYPE;

// struct to encapsulate all the optical parameters for managing ani g optical alarms..
typedef struct
{
    short   RxOpticalPwr;           //Optical Signal Level
    short   RxPwrLowThreshold;      //Lower Optical Threshold
    short   RxPwrHighThreshold;     //Upper Optical Threshold
    short   TxOpticalPwr;           //Transmit Optical Signal
    short   TxPwrLowThreshold;      //Lower transmit power threshold
    short   TxPwrHighThreshold;     //Upper transmit power threshold

} AnigOpticalParms_t;

typedef struct
{
    UINT8 initFlag;
    UINT32 lineSpeed;
} BCM_OMCIPM_LINE_SENSE_REC;

typedef struct
{
    BCM_COMMON_DECLARE_LL_ENTRY ();
    UINT16 objectType;
    UINT16 objectID;
    UINT16 arcInterval;
    time_t expireTime;
    ARC_CB_FUNC cbFunction;
} BCM_OMCIPM_ARC_ENTRY, *PBCM_OMCIPM_ARC_ENTRY;

typedef struct
{
    BCM_COMMON_DECLARE_LL_ENTRY ();
    UINT16 objectType;
    UINT16 objectID;
    UINT16 alarmBitmap;
} BCM_OMCIPM_ALARM_ENTRY, *PBCM_OMCIPM_ALARM_ENTRY;

typedef struct
{
    BCM_COMMON_DECLARE_LL_ENTRY ();
    ALARM_TIMER_TYPE timerType;
    ALARM_CB_FUNC cbFunction;
} BCM_OMCIPM_ALARM_TIMER_ENTRY, *PBCM_OMCIPM_ALARM_TIMER_ENTRY;

typedef struct
{
    UINT16 objectType;
    UINT16 objectID;
    time_t soakStartTime;
    UINT8 soakState;
} BCM_ALARM_SOAK_REC, *PBCM_ALARM_SOAK_REC;


/*========================== APIs for omcipmd ================*/


void omci_pm_pollByObjectClass(BCM_OMCI_PM_CLASS_ID obj_Class);

void omci_pm_increaseIntervalCounter(void);

void omci_pm_resetIntervalCounter(void);

UINT8 omci_pm_getIntervalCounter(void);

void omci_pm_setIntervalFlag(UBOOL8 intervalEndFlag);

UBOOL8 omci_pm_getIntervalFlag(void);

void omci_pm_initStatBuffers(UINT8 intervalCounter);

void omci_pm_releaseAll(void);

void omci_threshold_releaseAll(void);

UINT8 omci_alarm_getSequenceNumber(void);

void omci_alarm_setSequenceNumber(UINT8 seqNumber);

UINT16 omci_alarm_getFrozenAll(UINT8 arcFlag);

BCM_OMCIPM_ALARM_ENTRY* omci_alarm_getFrozen(UINT16 alarmIndex);

void omci_alarm_releaseAll(void);

CmsRet omci_alarm_init(void);

void omci_alarm_clear(UINT16 alarmSoakId);

void omci_alarm_data_reset(void);

void omci_alarm_poll_enet(void);

void omci_alarm_poll_gpon(void);

void omci_alarm_poll_optical_signals(void);

void omci_arc_poll(void);

void omci_arc_releaseAll(void);

void omci_pm_mcast_poll(void);

UINT8 omci_test_iphost_ping(
    UINT16 tcID,
    UINT16 meID,
    UINT16 objID,
    UINT8 test,
    UINT8 *ipAddr);

void omci_test_iphost_ping_result(
    const CmsMsgHeader *msgPtr);

UINT8 omci_test_iphost_trace_route(
    UINT16 tcID,
    UINT16 meID,
    UINT16 objID,
    UINT8 test,
    UINT8 *ipAddr);

void omci_test_iphost_trace_route_result(
    const CmsMsgHeader* msgPtr);

UINT8 omci_test_aniG(UINT16 tcID, UINT16 meID, UINT16 objID);

void omci_poll_test(void);

void omci_test_releaseAll(void);


/*========================== APIs for RTL/STL ================*/


CmsRet omci_pm_findEntryById(
    UINT16 obj_Type,
    UINT16 obj_ID);


CmsRet omci_pm_createObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    UINT16 port_ID,
    UINT16 threshold_ID);


void omci_pm_deleteObject(
    UINT16 obj_Type,
    UINT16 obj_ID);


void omci_pm_getObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    void **ppData,
    UINT16 *pDataLen);


void omci_pm_getCurrentObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    void **ppData,
    UINT16 *pDataLen);


void omci_pm_setObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    UINT16 threshold_ID);


void omci_th_createObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    const void *pData);


void omci_th_deleteObject(
    UINT16 obj_Type,
    UINT16 obj_ID);


void omci_th_getObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    void *pData,
    UINT16 *pDataLen);


void omci_th_setObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    const void *pData);


UBOOL8 omci_th_isObjectExisted(
    UINT16 obj_Type,
    UINT16 obj_ID);


UINT32 omci_arc_create(
    UINT16 obj_Class,
    UINT16 obj_ID);


UINT32 omci_arc_delete(
    UINT16 obj_Class,
    UINT16 obj_ID);


BCM_OMCIPM_ARC_ENTRY* omci_arc_get(
    UINT16 obj_Class,
    UINT16 obj_ID);


UINT32 omci_arc_set(
    UINT16 class_ID,
    UINT16 obj_ID,
    UINT16 arc_Interval);


UBOOL8 omci_arc_exist(
    UINT16 obj_Class,
    UINT16 obj_ID);


CmsRet omci_pm_mcast_get(
    UINT16 phyId,
    UINT16 portId,
    UINT32 *pCounter);


void omci_pm_syncEnetAdminStates(
    UINT32 portNum,
    UBOOL8 isAdminLocked);


void omci_pm_syncPotsAdminStates(
    UINT32 portNum,
    UBOOL8 isAdminLocked);


void omci_pm_syncUniGAdminStates(
    UINT32 uniGMeId,
    UBOOL8 isAdminLocked);


void omci_pm_initAniGObject(void);


void omci_pm_initEnetPptpObjects(void);


void omci_pm_syncAllAdminStates(void);


void omci_pm_release(void);


#ifdef DMP_X_ITU_ORG_VOICE_1

void omci_pm_initVoipPptpObjects(void);

#endif /* DMP_X_ITU_ORG_VOICE_1 */

CmsRet omci_extPm_createObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    UINT8  *control);

void omci_extPm_setObject(
    UINT16 obj_Type,
    UINT16 obj_ID,
    UINT8  *control);

#endif /* __OMCI_PM_H__ */
