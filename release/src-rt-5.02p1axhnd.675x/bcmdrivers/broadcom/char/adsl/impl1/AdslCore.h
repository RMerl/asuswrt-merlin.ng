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

/****************************************************************************
 *
 * AdslCore.h -- Internal definitions for ADSL core driver
 *
 * Description:
 *	Internal definitions for ADSL core driver
 *
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.4 $
 *
 * $Id: AdslCore.h,v 1.4 2004/05/06 03:24:02 ilyas Exp $
 *
 * $Log: AdslCore.h,v $
 * Revision 1.4  2004/05/06 03:24:02  ilyas
 * Added power management commands
 *
 * Revision 1.3  2004/04/30 17:58:01  ilyas
 * Added framework for GDB communication with ADSL PHY
 *
 * Revision 1.2  2004/04/13 01:07:19  ilyas
 * Merged the latest ADSL driver changes for RTEMS
 *
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/
#if !defined(_AdslCore_H)
#define _AdslCore_H

#undef	PHY_BLOCK_TEST
//#define	PHY_BLOCK_TEST
#undef	ADSLDRV_STATUS_PROFILING
//#define	ADSLDRV_STATUS_PROFILING
//#define	ADSLDRV_SILENT_MODE

//#define STAT_HANDLING_PRINT
#ifdef STAT_HANDLING_PRINT
#define	STAT_HANDLING_PRINT_MAX	256
#endif
#ifndef SUPPORT_PROCESS_STAT_IN_THREAD
#if defined(CONFIG_BCM963158) || (defined(LINUX_FW_EXTRAVERSION) && (LINUX_FW_EXTRAVERSION >= 50206))
#define SUPPORT_PROCESS_STAT_IN_THREAD
#endif
#endif

#if defined(PHY_LOOPBACK) || defined(SUPPORT_TEQ_FAKE_LINKUP)
#ifdef PHY_LOOPBACK
#define	PHY_BLOCK_TEST
#endif
#if defined(CONFIG_BCM963138)
#define	PHY_LOOPBACK_US_RATE	400000
#define	PHY_LOOPBACK_DS_RATE	400000
#else
#define	PHY_LOOPBACK_US_RATE	50000
#define	PHY_LOOPBACK_DS_RATE	99000
#endif
#endif

#define	LD_MODE_VDSL	0
#define	LD_MODE_ADSL	1

typedef unsigned char		AC_BOOL;
#define	AC_TRUE				1
#define	AC_FALSE			0

/* Boot flags */

#define AC_HARDBOOT			0x01
#define AC_SOFTBOOT			0x02

/* Adsl Core events */

#define	ACEV_LINK_UP					1
#define	ACEV_LINK_DOWN					2
#define	ACEV_G997_FRAME_RCV				4
#define	ACEV_G997_FRAME_SENT			8
#define	ACEV_SWITCH_RJ11_PAIR			0x10
#define ACEV_G994_NONSTDINFO_RECEIVED	0x20
#define ACEV_LINK_POWER_L3				0x40
#define ACEV_RATE_CHANGE				0x80
#define ACEV_REINIT_PHY					0x100
#define ACEV_FAST_RETRAIN				0x200
#define ACEV_G997_NSF_FRAME_RCV			0x0400
#define ACEV_G997_NSF_FRAME_SENT		0x0800
#define ACEV_G997_DATAGRAM_FRAME_RCV		0x1000
#define ACEV_G997_DATAGRAM_FRAME_SENT		0x2000

typedef struct _AdslCoreConnectionRates {
    int		fastUpRate;
    int		fastDnRate;
    int		intlUpRate;
    int		intlDnRate;
} AdslCoreConnectionRates;

void AdslCoreSetL2Timeout(unsigned long val);
void AdslCoreIndicateLinkPowerStateL2(unsigned char lineId);
AC_BOOL AdslCoreSetSDRAMBaseAddr(void *pAddr);
void	*AdslCoreGetSDRAMBaseAddr(void);
AC_BOOL AdslCoreSetVcEntry (int gfc, int port, int vpi, int vci, int pti_clp);
void	AdslCoreSetStatusCallback(void *pCallback);
void	*AdslCoreGetStatusCallback(void);
void	AdslCoreSetShowtimeMarginMonitoring(unsigned char lineId, unsigned char monEnabled, int showtimeMargin, int lomTimeSec);
AC_BOOL AdslCoreSetStatusBuffer(void *bufPtr, int bufSize);
AC_BOOL XdslCoreIs2lpActive(unsigned char lineId, unsigned char direction);
AC_BOOL XdslCoreIsGfastMod(unsigned char lineId);
AC_BOOL XdslCoreIsVdsl2Mod(unsigned char lineId);
AC_BOOL XdslCoreIsXdsl2Mod(unsigned char lineId);
AC_BOOL AdslCoreLinkState (unsigned char lineId);
int		AdslCoreLinkStateEx (unsigned char lineId);
void	AdslCoreGetConnectionRates (unsigned char lineId, AdslCoreConnectionRates *pAdslRates);

void	AdslCoreSetSupportedNumTones(unsigned char lineId);
int		AdslCoreSetObjectValue (unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
long		AdslCoreGetObjectValue (unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
AC_BOOL AdslCoreBertStart(unsigned char lineId, unsigned int totalBits);
AC_BOOL AdslCoreBertStop(unsigned char lineId);
void	AdslCoreBertStartEx(unsigned char lineId, unsigned int bertSec);
void	AdslCoreBertStopEx(unsigned char lineId);
void	AdslCoreResetStatCounters(unsigned char lineId);

int		AdslCoreGetOemParameter (int paramId, void *buf, int len);
int		AdslCoreSetOemParameter (int paramId, void *buf, int len);
char	*AdslCoreGetVersionString(void);
char	*AdslCoreGetBuildInfoString(void);
int		AdslCoreGetSelfTestMode(void);
void	AdslCoreSetSelfTestMode(int stMode);
int		AdslCoreGetSelfTestResults(void);
void	*AdslCoreSetSdramImageAddr(unsigned int lmem2, unsigned int pgSize, unsigned int sdramSize);
int		AdslCoreFlattenCommand(void *cmdPtr, void *dstPtr, unsigned int nAvail);
void	AdslCoreSetL3(unsigned char lineId);
void	AdslCoreSetL0(unsigned char lineId);
void	AdslCoreSetXfaceOffset(void *lmemAddr, unsigned int lmemSize);

void	*AdslCoreGetPhyInfo(void);
void	*AdslCoreGetSdramPageStart(void);
void	*AdslCoreGetSdramImageStart(void);
unsigned int AdslCoreGetSdramImageSize(void);
void	AdslCoreSharedMemInit(void);
void	*AdslCoreSharedMemAlloc(long size);
void	AdslCoreSharedMemFree(void *p);
void	*AdslCorePhyReservedMemAlloc(long size);
AC_BOOL	AdslCorePhyReservedMemFree(void *p);
unsigned int AdslCorePhyReservedMemAvail(void);
#if defined(USE_RESERVE_SHARE_MEM)
void XdslCoreReservedSharedMemFree(unsigned char lineId, void *p);
void *XdslCoreReservedSharedMemAlloc(unsigned char lineId, long size);
#endif
#ifdef ADSL_SDRAM_RESERVE_SIZE
void	*AdslCoreSystemReservedMemAlloc(unsigned int size, void **ppPhysAddr);
void	AdslCoreSystemReservedMemFree(void *p, void *physAddr, unsigned int size);
unsigned int AdslCoreSystemReservedMemAvail(void);
#endif
AC_BOOL AdslCoreInit(void);
void	AdslCoreUninit(void);
AC_BOOL AdslCoreHwReset(AC_BOOL bCoreReset);
void AdslCoreStop(void);

AC_BOOL AdslCoreIntHandler(void);
int	AdslCoreIntTaskHandler(int nMaxStatus);
void	AdslCoreTimer (unsigned long tMs);
AC_BOOL AdslCoreCommandHandler(void *cmdPtr);
AC_BOOL AdslCoreCommandIsPending(void);

AC_BOOL AdslCoreG997SendData(unsigned char lineId, int eocMsgType, void *buf, int len);

int AdslCoreG997FrameReceived(unsigned char lineId, int eocMsgType);
void	*AdslCoreG997FrameGet(unsigned char lineId, int eocMsgType, int *pLen);
void	*AdslCoreG997FrameGetNext(unsigned char lineId, int eocMsgType, int *pLen);
void	AdslCoreG997FrameFinished(unsigned char lineId, int eocMsgType);

void	BcmAdslCoreNotify(unsigned char lineId, int acEvent);
AC_BOOL BcmAdslCoreLogEnabled(void);
int		BcmAdslCoreDiagWrite(void *pBuf, int len);
void	BcmAdslCoreDiagClose(void);

#if (defined(CONFIG_BCM_DSL_GFAST) && !defined(CONFIG_BCM_DSL_GFASTCOMBO)) ||\
    (defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0)) ||\
    (defined(SUPPORT_MULTI_PHY) && defined(CONFIG_BCM963268))
void XdslCoreSetPhyImageType(int imageType);
int XdslCoreGetPhyImageType(void);
#endif

#if defined(SUPPORT_MULTI_PHY) || (defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0))
#ifdef SUPPORT_MULTI_PHY
typedef enum {
	MEDIASEARCH_START_E = 0,
	MEDIASEARCH_TIMER_E,
	MEDIASEARCH_SIGNAL_E,
	MEDIASEARCH_RXLNATTNAVG_E,
	MEDIASEARCH_LINKUP_E,
	MEDIASEARCH_LINKDOWN_E
} xdslMediaSearchEvent;

void BcmXdslCoreMediaSearchInit(void);
AC_BOOL BcmXdslCoreMediaSearchInInitState(void);
void BcmXdslCoreMediaSearchSM(xdslMediaSearchEvent mediaSearchEvent, unsigned int param);
AC_BOOL BcmXdslCoreProcessMediaSearchCfgCmd(unsigned int mediaSrchCfg, AC_BOOL phySwitchOnly);
void BcmXdslCoreMediaSearchReStartPhy(void);
unsigned char BcmXdslCoreSelectAndReturnPreferredMedia(void);
void XdslCoreProcessPrivateSysMediaCfg(unsigned int mediaSrchCfg);
#ifndef CONFIG_BCM963268
void BcmXdslCoreBringUpLineInDownState(void);
#endif
#endif /* SUPPORT_MULTI_PHY */
#endif

unsigned int BcmXdslCoreGetPhyExtraCfg(unsigned char lineId, unsigned char phyExtraCfgNum);
unsigned long AdslCoreGetBootFlags(void);
void BcmXdslCoreSendCmd(unsigned char lineId, unsigned int cmd, unsigned int value);

#ifdef SUPPORT_DSL_BONDING
unsigned char XdslCoreGetCurLineId(void);
#endif

/* synchronization with ADSL PHY DPC/bottom halves */
#ifdef __KERNEL__

#include <linux/spinlock.h>
typedef struct _BcmXdslGlobalInfo
{
	spinlock_t xdslLockTx;
	spinlock_t xdslLockRx;
	spinlock_t xdslLockRegs;
	spinlock_t xdslLockDiags;
	spinlock_t xdslLockSharedMem;
} BcmXdslGlobalInfo;

extern BcmXdslGlobalInfo gXdslGlobalInfo;
#define	SYNC_RX			&gXdslGlobalInfo.xdslLockRx
#define	SYNC_TX			&gXdslGlobalInfo.xdslLockTx
#define	SYNC_REGS		&gXdslGlobalInfo.xdslLockRegs
#define	SYNC_SHAREMEM	&gXdslGlobalInfo.xdslLockSharedMem
#define	SYNC_DIAGS		&gXdslGlobalInfo.xdslLockDiags

#define	BcmCoreDpcSyncInit(x)		spin_lock_init(x)
#define	BcmCoreDpcSyncEnter(x)	spin_lock_bh(x)
#define	BcmCoreDpcSyncExit(x)	spin_unlock_bh(x)
#define	BcmCoreDpcSyncIntrDisableEnter(x,y)	spin_lock_irqsave(x,y)
#define	BcmCoreDpcSyncIntrDisableExit(x,y)	spin_unlock_irqrestore(x,y)
#if 0
#ifdef CONFIG_SMP
void SMP_DpcSyncEnter(void);
void SMP_DpcSyncExit(void);
#define	BcmCoreDpcSyncEnter()		SMP_DpcSyncEnter()
#define	BcmCoreDpcSyncExit()		SMP_DpcSyncExit()
#else
#define	BcmCoreDpcSyncEnter()		local_bh_disable()
#define	BcmCoreDpcSyncExit()		local_bh_enable()
#endif
#endif
#elif defined(VXWORKS)
#define	BcmCoreDpcSyncEnter()		taskLock()
#define	BcmCoreDpcSyncExit()		taskUnlock()
#else
#define	BcmCoreDpcSyncEnter()
#define	BcmCoreDpcSyncExit()
#endif

#endif	/* _AdslCore_H */
