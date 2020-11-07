/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

/***************************************************************************
****************************************************************************
** File Name  : BcmAdslCore.h
**
** Description: This file contains the definitions, structures and function
**              prototypes for Bcm Core ADSL PHY interface
**
***************************************************************************/
#if !defined(_BcmAdslCore_H)
#define _BcmAdslCore_H

#include "AdslMibDef.h"

/* 
**	Internal ADSL driver events handled by the ADSL driver
**  defined not to intersect with ADSL_LINK_STATE events (in vcmadsl.h)
*/

#define	ADSL_SWITCH_RJ11_PAIR	0x80000001

#ifdef SUPPORT_MULTI_PHY
#define	MS_SWITCH_SEARCH_SHIFT	0
#define	MS_SWITCH_SEARCH_MSK	(3 << MS_SWITCH_SEARCH_SHIFT)
#define	MS_PHYSWITCH_DISABLED	(1 << MS_SWITCH_SEARCH_SHIFT)
#define	MS_SEARCH_DISABLED		(2 << MS_SWITCH_SEARCH_SHIFT)
#define	MS_USE_NEW_CFG_SHIFT	2
#define	MS_USE_NEW_CFG			(1 << MS_USE_NEW_CFG_SHIFT)
#define	MS_USE_IMG_SHIFT		3
#define	MS_USE_IMG_MSK			(1 << MS_USE_IMG_SHIFT)
#define	MS_USE_IMG_BONDING		(0 << MS_USE_IMG_SHIFT)
#define	MS_USE_IMG_SINGLELINE	(1 << MS_USE_IMG_SHIFT)
#ifndef CONFIG_BCM963268
#define	MS_USE_ONLY_SINGLELINE	MS_USE_IMG_SINGLELINE	/* 63138/63148 */
#endif
#define	MS_USE_AFE_SHIFT		4
#define	MS_USE_AFE_MSK			(3 << MS_USE_AFE_SHIFT)
#define	MS_USE_AFE_INTERNAL		(0 << MS_USE_AFE_SHIFT)
#define	MS_USE_AFE_EXTERNAL		(1 << MS_USE_AFE_SHIFT)
#if 0
#define	MS_SAVE_PREFERMEDIA_SHIFT	6
#define	MS_SAVE_PREFERMEDIA_MSK	(1 << MS_SAVE_PREFERMEDIA_SHIFT)
#define	MS_SAVE_PREFERMEDIA_DISABLED	(1 << MS_SAVE_PREFERMEDIA_SHIFT)
#endif
#if defined(CONFIG_BCM_DSL_GFAST)
#define	PHY_SWITCH_SHIFT		MS_SWITCH_SEARCH_SHIFT
#define	PHY_SWITCH_MSK			MS_PHYSWITCH_DISABLED
#define	PHY_SWITCH_DISABLED		MS_PHYSWITCH_DISABLED
#if 0
#define	PHY_TYPE_PREFER_SAVE_SHIFT	MS_SAVE_PREFERMEDIA_SHIFT
#define	PHY_TYPE_PREFER_SAVE_MSK	MS_SAVE_PREFERMEDIA_MSK
#define	PHY_TYPE_PREFER_SAVE_DISABLED	MS_SAVE_PREFERMEDIA_DISABLED
#endif
#define	PHY_TYPE_SHIFT			6
#define	PHY_TYPE_MSK			(1 << PHY_TYPE_SHIFT)
#define	PHY_TYPE_NON_GFAST		(0 << PHY_TYPE_SHIFT)
#define	PHY_TYPE_GFAST			(1 << PHY_TYPE_SHIFT)
#endif
#endif /* SUPPORT_MULTI_PHY */
#if defined(CONFIG_BCM_DSL_GFAST)
#define	PHY_SWITCH_SHIFT1		0
#define	PHY_SWITCH_MSK1			(1 << PHY_SWITCH_SHIFT1)
#define	PHY_SWITCH_DISABLED1	(1 << PHY_SWITCH_SHIFT1)
#define	PHY_TYPE_SHIFT1			1
#define	PHY_TYPE_MSK1			(1 << PHY_TYPE_SHIFT1)
#define	PHY_TYPE_NON_GFAST1		(0 << PHY_TYPE_SHIFT1)
#define	PHY_TYPE_GFAST1			(1 << PHY_TYPE_SHIFT1)
#if 0
#define	PHY_TYPE_PREFER_SAVE_SHIFT	2
#define	PHY_TYPE_PREFER_SAVE_MSK	(1 << PHY_TYPE_PREFER_SAVE_SHIFT)
#define	PHY_TYPE_PREFER_SAVE_DISABLED	(1 << PHY_TYPE_PREFER_SAVE_SHIFT)
#endif
#endif /* CONFIG_BCM_DSL_GFAST */

/***************************************************************************
** Function Prototypes
***************************************************************************/

void BcmAdslCoreInit(void);
void BcmAdslCoreUninit(void);
Bool BcmAdslCoreCheckBoard(void);
void BcmAdslCoreConnectionStart(unsigned char lineId);
void BcmAdslCoreConnectionStop(unsigned char lineId);
void BcmAdslCoreConnectionReset(unsigned char lineId);

void BcmAdslCoreGetConnectionInfo(unsigned char lineId, PADSL_CONNECTION_INFO pConnectionInfo);
void BcmAdslCoreDiagCmd(unsigned char lineId, PADSL_DIAG pAdslDiag);
void BcmAdslCoreDiagCmdAdsl(unsigned char lineId, int diagCmd, int len, void *pCmdData);
void BcmAdslCoreDiagCmdCommon(unsigned char lineId, int diagCmd, int len, void *pCmdData);
void BcmAdslCoreDiagCmdNotify(void);
void BcmAdslCoreDiagSetSyncTime(unsigned long syncTime);
void BcmAdslCoreDiagConnectionCheck(void);
unsigned long BcmAdslCoreDiagGetSyncTime(void);
char * BcmAdslCoreDiagScrambleString(char *s);
int  BcmAdslCoreSetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
long BcmAdslCoreGetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
void BcmAdslCoreStartBERT(unsigned char lineId, uint totalBits);
void BcmAdslCoreStopBERT(unsigned char lineId);
void BcmAdslCoreBertStartEx(unsigned char lineId, uint bertSec);
void BcmAdslCoreBertStopEx(unsigned char lineId);
#ifndef DYING_GASP_API
Bool BcmAdslCoreCheckPowerLoss(void);
#endif
void BcmAdslCoreSendDyingGasp(int powerCtl);
void BcmAdslCoreConfigure(unsigned char lineId, adslCfgProfile *pAdslCfg);
void BcmAdslCoreGetVersion(adslVersionInfo *pAdslVer);
void BcmAdslCoreSetTestMode(unsigned char lineId, int testMode);
void BcmAdslCoreSetTestExecutionDelay(unsigned char lineId, int testMode, uint value);
void BcmAdslCoreSelectTones(
	unsigned char lineId,
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap);
void BcmAdslCoreDiagSelectTones(
	unsigned char lineId,
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap);
void BcmAdslCoreSetAdslDiagMode(unsigned char lineId, int diagMode);
void BcmAdslCoreSetSeltNextMode(unsigned char lineId);
int BcmAdslCoreGetConstellationPoints (int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints);
int BcmAdslCoreGetOemParameter (int paramId, void *buf, int len);
int BcmAdslCoreSetOemParameter (int paramId, void *buf, int len);
int BcmAdslCoreSetXmtGain(unsigned char lineId, int gain);
int  BcmAdslCoreGetSelfTestMode(void);
void BcmAdslCoreSetSelfTestMode(int stMode);
int  BcmAdslCoreGetSelfTestResults(void);
void BcmAdslCoreAfeTestMsg(void *pMsg);
void BcmAdslCoreDebugCmd(unsigned char lineId, void *pMsg);
#ifdef SUPPORT_XDSLDRV_GDB
void GdbStubInit(void);
void setGdbMboxAddr(void);
void setGdbOn(void);
char isGdbOn(void);
void BcmAdslCoreGdbCmd(void *pCmd, int cmdLen);
void BcmAdslCoreGdbTask(void);
void BcmAdslCoreGdbSetMemDumpState(int bOn);
#endif

void BcmXdslEocWakeup(unsigned lineId, int eocMsgType);

void BcmXdslCoreGetCurrentMedia(unsigned int *pMediaInUse);
int  BcmXdslCoreGetLineActive(int lineId);

#ifdef __KERNEL__
int BcmXdslCoreDiagProcFileCreate(void);
void BcmXdslCoreDiagProcFileRemove(void);
void BcmAdslCoreResetPhy(int copyImage);
#endif /* __KERNEL__ */
ADSL_LINK_STATE BcmAdslCoreGetEvent (unsigned char lineId);
Bool BcmAdslCoreSetSDRAMBaseAddr(void *pAddr);
Bool BcmAdslCoreSetVcEntry (int gfc, int port, int vpi, int vci, int pti_clp);
Bool BcmAdslCoreSetGfc2VcMapping(Bool bOn);
Bool BcmAdslCoreSetAtmLoopbackMode(void);
void BcmAdslCoreResetStatCounters(unsigned char lineId);

Bool BcmAdslCoreG997SendData(unsigned char lineId, int eocMsgType, void *buf, int len);

void *BcmAdslCoreG997FrameGet(unsigned char lineId, int eocMsgType, int *pLen);
void *BcmAdslCoreG997FrameGetNext(unsigned char lineId, int eocMsgType, int *pLen);
void BcmAdslCoreG997FrameFinished(unsigned char lineId, int eocMsgType);

void BcmAdslCoreAtmSetPortId(int path, int portId);
void BcmAdslCoreAtmClearVcTable(void);
void BcmAdslCoreAtmAddVc(int vpi, int vci);
void BcmAdslCoreAtmDeleteVc(int vpi, int vci);
void BcmAdslCoreAtmSetMaxSdu(unsigned short maxsdu);

void BcmAdsl_Notify(unsigned char lineId);
void BcmAdsl_ConfigureRj11Pair(int pair);
void BcmAdslCoreDelay(unsigned long timeMs);
void BcmXdslCoreSendAfeInfo(int toPhy);
void BcmXdslNotifyRateChange(unsigned char lineId);
void BcmXdslCoreMiscIoCtlFunc(unsigned char lineId);
void BcmXdslCoreMaintenanceTask(void);
#if defined(SUPPORT_HMI)
void BcmXdslCoreSendHmiConfig(unsigned char lineId, int configId, void *data, int dataLen);
#endif

int BcmXdslCoreGetAfeBoardId(unsigned int *pAfeIds);
#if defined(USE_6306_CHIP)
int spiTo6306Read(int address);
void spiTo6306Write(int address, unsigned int data);
int spiTo6306IndirectRead(int address);
void spiTo6306IndirectWrite(int address, unsigned int data);
void SetupReferenceClockTo6306(void);
void DisableReferenceClockTo6306(void);
int IsAfe6306ChipUsed(void);
#ifdef CONFIG_BCM96368
void PLLPowerUpSequence6306(void);
#endif
#endif	/* USE_6306_CHIP */

#if defined(CONFIG_VDSL_SUPPORTED) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 6)
int IsLD6303VR5p3Used(void);
#endif

unsigned long BcmAdslCoreGetCycleCount(void);
unsigned long BcmAdslCoreCycleTimeElapsedUs(unsigned long cnt1, unsigned long cnt0);
unsigned long BcmAdslCoreOsTimeElapsedMs(OS_TICKS osTime0);

void *BcmCoreAllocLowMem(uint size);

#define	BcmCoreCommandHandler(cmd)	do {	\
	AdslCoreCommandHandler(cmd);				\
} while (0)

#endif /* _BcmAdslCore_H */

