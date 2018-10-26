//****************************************************************************
//
// (c) 2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
#ifndef _BMU_API_H_
#define _BMU_API_H_

#include "bcmtypes.h"

// This is the maximum number of batteries that the code can support
// The system may support less (kBatteryNumberOfCells)
#define MAX_BATTERIES 2

#define CMS_MSG_BMU_CLI                   ((CmsMsgType)(CMS_MSG_BMU_BEGIN+0))
#define CMS_MSG_BMU_STATUS_GET            ((CmsMsgType)(CMS_MSG_BMU_BEGIN+1))
#define CMS_MSG_BMU_CONTROLLER_STATUS_GET ((CmsMsgType)(CMS_MSG_BMU_BEGIN+2))
#define CMS_MSG_BMU_BATTERY_STATUS_GET    ((CmsMsgType)(CMS_MSG_BMU_BEGIN+3))
#define CMS_MSG_BMU_CONFIG_CHANGE_NOTIFY  ((CmsMsgType)(CMS_MSG_BMU_BEGIN+4))

typedef enum
{
  kBmuInit = 0,
  kBmuIdle,
  kBmuSleep,
  kBmuChargeInit,
  kBmuPrecharge,
  kBmuFastCharge,
  kBmuTopoff,
  kBmuChargeSuspended,
  kBmuDischarge,
  kBmuEternalSleep,
  kBmuForcedDischarge,
} BmuControllerState;

typedef enum {
  kLTStateIdle = 0,      
  kLTStateBegin,     
  kLTStatePTD,       
  kLTStateStartCharge,    
  kLTStateHWIMP,     
  kLTStateMonitorCharge,    
  kLTStateStartSWIMP,
  kLTStateComplSWIMP,
  kLTStateDischarge, 
  kLTStateComplete
} BmuLifeTestState;

typedef struct
{
	BOOL BatteryPresent;
	BOOL BatteryValid;
	BOOL BatteryBad;
	BOOL BatterySelected;
	BOOL BatteryFullyCharged;
	BOOL BatteryChargeLow;
	BOOL BatteryChargeLowPercent;
	BOOL BatteryChargeDepleted;
	BOOL BatteryChargeStateUnknown;
	int BatteryChargeCapacity;
	int BatteryActualCapacity;
	int BatteryFullChargeVoltage;
	int BatteryDepletedVoltage;
	int BatteryMeasuredVoltage;
	int BatteryPercentCharge;
	int BatteryEstimatedMinutesRemaining;
	int BatteryTemperature;
	unsigned int BatteryLifeTestCount;
	time_t BatteryLastLifeTest;
	time_t BatteryNextLifeTest;
	BmuLifeTestState BatteryLifeTestState;
	int BatteryStateofHealth; // 0-100 => 0%-100%
} BatteryStatus_type;

typedef struct
{
    char   Version[128];
    time_t BuildDateTime;
    BOOL   OperatingOnBattery;
    BmuControllerState State;
    int    NumberOfPresentBatteries;
    int    InputVoltage;
    int    Temperature;
    int    EstimatedMinutesRemaining;
    int    BatteryCurrent;
    int    UpsSecondsOnBattery;
    BatteryStatus_type BatteryStatus[MAX_BATTERIES];
} BmuStatus_type;

/* Example on how to get the bmu status
{
    void *msgHandle
    BmuStatus_type bmuStatus;

    if (cmsMsg_init(EID_BMUCTL, &msgHandle) == CMSRET_SUCCESS) {
        BmuMsg_send(msgHandle, EID_BMUCTL, CMS_MSG_BMU_GET_STATUS, 0, &bmuStatus);
        printf("Number of Batteries %d\n", bmuStatus.NumberOfPresentBatteries);
    }
}
*/

static inline CmsRet BmuMsg_send(void *msgHandle, CmsEntityId srcEid, CmsMsgType msgType, UINT32 arg, void *pBmuRsp)
{
   CmsMsgHeader *pMsgIn = NULL;
   uint8_t buf[sizeof(CmsMsgHeader)];
   CmsMsgHeader *pMsgOut = (CmsMsgHeader *)&buf[0];
   CmsRet ret = CMSRET_SUCCESS;

   memset(&buf[0], 0x0, sizeof(buf));
   pMsgOut->type        = msgType;
   pMsgOut->src         = srcEid;
   pMsgOut->dst         = EID_BMUD;
   pMsgOut->flags_event = 1;
   pMsgOut->flags_bounceIfNotRunning = 1;   // Do not let CMS try to start BMUD
   pMsgOut->dataLength  = 0;
   pMsgOut->wordData    = arg;

   if ((ret = cmsMsg_send(msgHandle, pMsgOut)) == CMSRET_SUCCESS)
   {
      if ((pBmuRsp != NULL) &&
          (ret = cmsMsg_receiveWithTimeout(msgHandle, &pMsgIn, 2000)) == CMSRET_SUCCESS)
      {
         memcpy(pBmuRsp, pMsgIn+1, pMsgIn->dataLength);
         CMSMEM_FREE_BUF_AND_NULL_PTR(pMsgIn);
      }
   }
   return ret;
}                                                                            

#endif
