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
 * AdslCore.c -- Bcm ADSL core driver
 *
 * Description:
 *	This file contains BCM ADSL core driver 
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.7 $
 *
 * $Id: AdslCore.c,v 1.7 2004/07/20 23:45:48 ilyas Exp $
 *
 * $Log: AdslCore.c,v $
 * Revision 1.7  2004/07/20 23:45:48  ilyas
 * Added driver version info, SoftDslPrintf support. Fixed G.997 related issues
 *
 * Revision 1.6  2004/06/10 00:20:33  ilyas
 * Added L2/L3 and SRA
 *
 * Revision 1.5  2004/05/06 20:03:51  ilyas
 * Removed debug printf
 *
 * Revision 1.4  2004/05/06 03:24:02  ilyas
 * Added power management commands
 *
 * Revision 1.3  2004/04/30 17:58:01  ilyas
 * Added framework for GDB communication with ADSL PHY
 *
 * Revision 1.2  2004/04/27 00:33:38  ilyas
 * Fixed buffer in shared SDRAM checking for EOC messages
 *
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/

#if defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#endif

#include "AdslCoreMap.h"

#include "softdsl/SoftDsl.h"
#include "softdsl/CircBuf.h"
#include "softdsl/BlankList.h"
#include "softdsl/BlockUtil.h"
#include "softdsl/Flatten.h"
#include "softdsl/AdslXfaceData.h"

#if defined(_NOOS)

#elif defined(__KERNEL__)

#include <linux/version.h>
#include <linux/kernel.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include <linux/bottom_half.h>	/* for local_bh_disable/local_bh_enable */
#else
#include <linux/interrupt.h>
#endif
#include <linux/semaphore.h>
#include "bcm_map.h"
#include "bcm_intr.h"
#include "boardparms.h"
#include "board.h"
#if defined(BOARD_H_API_VER) && BOARD_H_API_VER > 14
#include "bcm_rsvmem.h"
#endif

#elif defined(_CFE_)

#include "bcm_map.h"
#include "bcm_intr.h"
#include "boardparms.h"
#include "board.h"

#elif defined(__ECOS)

#include "bcm_map.h"

#elif defined(VXWORKS)

#if defined(CONFIG_BCM96362)
#include "6362_common.h"
#elif defined(CONFIG_BCM96328)
#include "6328_common.h"
#elif defined(CONFIG_BCM963268)
#include "63268_common.h"
#elif defined(CONFIG_BCM96318)
#include "6318_common.h"
#elif defined(CONFIG_BCM963138)
#include "63138_common.h"
#elif defined(CONFIG_BCM963381)
#include "63381_common.h"
#elif defined(CONFIG_BCM963148)
#include "63148_common.h"
#elif defined(CONFIG_BCM963158)
#include "63158_common.h"
#elif defined(CONFIG_BCM963178)
#include "63178_common.h"
#elif defined(CONFIG_BCM963146)
#include "63146_common.h"
#else
#error	"Unknown 963x8 chip"
#endif

#endif /* VXWORKS */

#ifndef NO_XTM_MODULE
#include <bcmxtmcfg.h>
#endif

#ifdef ADSL_SELF_TEST
#include "AdslSelfTest.h"
#endif

#include "AdslCore.h"

#include "AdslCoreFrame.h"

#if defined(ADSL_PHY_FILE) || defined(ADSL_PHY_FILE2)
#include "AdslFile.h"
#else

#if defined(CONFIG_BCM963x8)

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
#ifdef ADSL_ANNEXC
#include "adslcore6362C/adsl_lmem.h"
#include "adslcore6362C/adsl_sdram.h"
#elif defined(ADSL_ANNEXB)
#include "adslcore6362B/adsl_lmem.h"
#include "adslcore6362B/adsl_sdram.h"
#elif defined(ADSL_SADSL)
#include "adslcore6362SA/adsl_lmem.h"
#include "adslcore6362SA/adsl_sdram.h"
#else
#include "adslcore6362/adsl_lmem.h"
#include "adslcore6362/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM963268)
#ifdef ADSL_ANNEXC
#include "adslcore63268C/adsl_lmem.h"
#include "adslcore63268C/adsl_sdram.h"
#elif defined(ADSL_ANNEXB)
#include "adslcore63268B/adsl_lmem.h"
#include "adslcore63268B/adsl_sdram.h"
#elif defined(ADSL_SADSL)
#include "adslcore63268SA/adsl_lmem.h"
#include "adslcore63268SA/adsl_sdram.h"
#elif defined(SUPPORT_DSL_BONDING) && !defined(SUPPORT_2CHIP_BONDING)
#include "adslcore63268bnd/adsl_lmem.h"
#include "adslcore63268bnd/adsl_sdram.h"
#else
#include "adslcore63268/adsl_lmem.h"
#include "adslcore63268/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM96318)
#ifdef ADSL_ANNEXC
#include "adslcore6318C/adsl_lmem.h"
#include "adslcore6318C/adsl_sdram.h"
#elif defined(ADSL_ANNEXB)
#include "adslcore6318B/adsl_lmem.h"
#include "adslcore6318B/adsl_sdram.h"
#elif defined(ADSL_SADSL)
#include "adslcore6318SA/adsl_lmem.h"
#include "adslcore6318SA/adsl_sdram.h"
#else
#include "adslcore6318/adsl_lmem.h"
#include "adslcore6318/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM963138)
#if defined(ADSL_ANNEXB)
#include "adslcore63138B/adsl_lmem.h"
#include "adslcore63138B/adsl_sdram.h"
#else
#include "adslcore63138/adsl_lmem.h"
#include "adslcore63138/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM963381)
#if defined(ADSL_ANNEXB)
#include "adslcore63381B/adsl_lmem.h"
#include "adslcore63381B/adsl_sdram.h"
#else
#include "adslcore63381/adsl_lmem.h"
#include "adslcore63381/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM963148)
#if defined(ADSL_ANNEXB)
#include "adslcore63148B/adsl_lmem.h"
#include "adslcore63148B/adsl_sdram.h"
#else
#include "adslcore63148/adsl_lmem.h"
#include "adslcore63148/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM963158)
#if defined(ADSL_ANNEXB)
#include "adslcore63158B/adsl_lmem.h"
#include "adslcore63158B/adsl_sdram.h"
#else
#include "adslcore63158/adsl_lmem.h"
#include "adslcore63158/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM963178)
#if defined(ADSL_ANNEXB)
#include "adslcore63178B/adsl_lmem.h"
#include "adslcore63178B/adsl_sdram.h"
#else
#include "adslcore63178/adsl_lmem.h"
#include "adslcore63178/adsl_sdram.h"
#endif

#elif defined(CONFIG_BCM963146)
#if defined(ADSL_ANNEXB)
#include "adslcore63146B/adsl_lmem.h"
#include "adslcore63146B/adsl_sdram.h"
#else
#include "adslcore63146/adsl_lmem.h"
#include "adslcore63146/adsl_sdram.h"
#endif

#else   /* 48 platform */
#ifdef ADSL_ANNEXC
#include "adslcore6348C/adsl_lmem.h"
#include "adslcore6348C/adsl_sdram.h"
#elif defined(ADSL_ANNEXB)
#include "adslcore6348B/adsl_lmem.h"
#include "adslcore6348B/adsl_sdram.h"
#elif defined(ADSL_SADSL)
#include "adslcore6348SA/adsl_lmem.h"
#include "adslcore6348SA/adsl_sdram.h"
#else
#include "adslcore6348/adsl_lmem.h"
#include "adslcore6348/adsl_sdram.h"
#endif  /* 48 platform */

#endif

#endif /* of CONFIG_BCM963x8 */

#endif /* ADSL_PHY_FILE */

#ifdef G997_1_FRAMER
#include "softdsl/G997.h"
#endif

#ifdef ADSL_MIB
#include "softdsl/AdslMib.h"
#endif

#ifdef G992P3
#include "softdsl/G992p3OvhMsg.h"
#undef	G992P3_DEBUG
#define	G992P3_DEBUG
#endif

#include "softdsl/SoftDsl.gh"
#include "BcmOs.h"
#include "BcmAdslDiag.h"
#include "DiagDef.h"
#include "AdslDrvVersion.h"
#include "bcmadsl.h"
#include "BcmAdslCore.h"
#include <stdarg.h>
#if (defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)) && !defined(_NOOS)
#include <linux/dma-mapping.h>
#endif
#ifdef USE_PMC_API
#include "pmc_drv.h"
#include "pmc_dsl.h"
#endif

#ifdef SUPPORT_HMI
#include "hmimsg/bcm_DrvXface.h"
#endif

#undef	SDRAM_HOLD_COUNTERS
#undef	BBF_IDENTIFICATION

#ifdef CONFIG_BCM963x8

#ifdef DEBUG_L2_RET_L0
static ulong	gL2Start				= 0;
#endif

#define	gTimeInL2Ms(x)		(((dslVarsStruct *)(x))->timeInL2Ms)

static ulong	gL2SwitchL0TimeMs	= (ulong)-1;
#define		kMemPrtyWarnTime	(5 * 1000)	/* 5 Seconds */

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)

volatile uint *pXdslControlReg = (uint *) ADSL_CTRL_BASE;
#define	ADSL_CLK_RATIO_SHIFT		8
#define	ADSL_CLK_RATIO_MASK		(0x1F << ADSL_CLK_RATIO_SHIFT)
#define	ADSL_CLK_RATIO		(0x0 << ADSL_CLK_RATIO_SHIFT)

#define	XDSL_ANALOG_RESET		(1 << 3)
#define	XDSL_PHY_RESET		(1 << 2)
#define	XDSL_MIPS_RESET		(1 << 1)
#define	XDSL_MIPS_POR_RESET		(1 << 0)

#elif defined(CONFIG_BCM963268)

volatile uint *pXdslControlReg = (uint *)0xb0001800;
#define	XDSL_ANALOG_RESET		0	/* Does not exist */
#define	XDSL_PHY_RESET		(1 << 2)
#define	XDSL_MIPS_RESET		(1 << 1)
#define	XDSL_MIPS_POR_RESET		(1 << 0)

#define	XDSL_AFE_GLOBAL_PWR_DOWN	(1 << 5)

#ifndef MISC_IDDQ_CTRL_VDSL_MIPS
#define	MISC_IDDQ_CTRL_VDSL_MIPS	(1 << 10)
#endif
#ifndef MISC_IDDQ_CTRL_VDSL_PHY
#define  MISC_IDDQ_CTRL_VDSL_PHY    MISC_IDDQ_CTRL_ADSL_PHY
#endif

#define	GPIO_PAD_CTRL_AFE_SHIFT	8
#define	GPIO_PAD_CTRL_AFE_MASK	(0xF << GPIO_PAD_CTRL_AFE_SHIFT)
#define	GPIO_PAD_CTRL_AFE_VALUE	(0x9 << GPIO_PAD_CTRL_AFE_SHIFT)

#elif defined(CONFIG_BCM96318)

volatile uint *pXdslControlReg = (uint *)0xb0000340;
#define	XDSL_ANALOG_RESET		(1 << 3)
#define	XDSL_PHY_RESET		(1 << 2)
#define	XDSL_MIPS_RESET		(1 << 1)
#define	XDSL_MIPS_POR_RESET		(1 << 0)

#elif defined(USE_PMC_API)

#else

#error	"Unknown 963x8 chip"

#endif

#endif /* CONFIG_BCM963x8 */

void XdslCoreProcessDrvConfigRequest(int lineId, volatile uint *pDrvConfigAddr);
#if defined(USE_6306_CHIP)
void XdslCoreHwReset6306(void);
void XdslCoreSetExtAFELDMode(int ldMode);
#endif	/* USE_6306_CHIP */
void XdslCoreFlushAllG997ReceivedFrame(void *gDslVars);

typedef ulong (*adslCoreStatusCallback) (dslStatusStruct *status, char *pBuf, int len);

/* Local vars */

void * AdslCoreGetOemParameterData (int paramId, int **ppLen, int *pMaxLen);
static void AdslCoreSetSdramTrueSize(void);

static ulong AdslCoreIdleStatusCallback (dslStatusStruct *status, char *pBuf, int len)
{
	return 0;
}

adslPhyInfo	adslCorePhyDesc = {
	0xA0000000 | (ADSL_SDRAM_TOTAL_SIZE - ADSL_PHY_SDRAM_PAGE_SIZE), 0, 0, ADSL_PHY_SDRAM_START_4,
	0, 0, 0, 0,
	0, {0,0,0,0},
	ADSL_PHY_SDRAM_PAGE_SIZE,
#ifdef SDRAM4G_SUPPORT1
	0xA0000000, 0,
#endif
	0xA0000000 | (ADSL_SDRAM_TOTAL_SIZE - ADSL_PHY_SDRAM_PAGE_SIZE), 0, NULL
};

#ifdef SUPPORT_STATUS_BACKUP
#define	STAT_BKUP_BUF_SIZE	16384
#define	STAT_BKUP_THRESHOLD	3072
#define	MIN(a,b) (((a)<(b))?(a):(b))
static int gStatusBackupThreshold = STAT_BKUP_THRESHOLD;
static stretchHostBufferStruct *pLocSbSta = NULL;
static stretchBufferStruct *pCurSbSta = NULL;
#endif

AdslXfaceData		* volatile pAdslXface	= NULL;
AdslOemSharedData	* volatile pAdslOemData = NULL;
adslCoreStatusCallback	pAdslCoreStatusCallback = AdslCoreIdleStatusCallback;
int			adslCoreSelfTestMode = kAdslSelfTestLMEM;
uint	adslPhyXfaceOffset = ADSL_PHY_XFACE_OFFSET;

stretchBufferStruct * volatile pPhySbSta = NULL;
stretchBufferStruct * volatile pPhySbCmd = NULL;

#define	gXdslCoreShMarginMonEnabled(x)		(((dslVarsStruct *)(x))->xdslCoreShMarginMonEnabled)
#define	gXdslCoreLOMTimeout(x)				(((dslVarsStruct *)(x))->xdslCoreLOMTimeout)
#define	gXdslCoreLOMTime(x)				(((dslVarsStruct *)(x))->xdslCoreLOMTime)
#define	gXdslCoreOvhMsgPrintEnabled(x)		(((dslVarsStruct *)(x))->xdslCoreOvhMsgPrintEnabled)


dslVarsStruct	acDslVars[MAX_DSL_LINE];
dslVarsStruct	*gCurDslVars = &acDslVars[0];
#ifdef SUPPORT_DSL_BONDING
volatile uint	*acAhifStatePtr[MAX_DSL_LINE] = { NULL, NULL };
#else
volatile uint	*acAhifStatePtr[MAX_DSL_LINE] = { NULL };
#endif
uint	acAhifStateMode = 0; /* 0 - old sync on XMT only, 1 -  +RCV showtime */ 

Boolean	acBlockStatusRead = AC_FALSE;
#define	ADSL_SDRAM_RESERVED			32
#define	ADSL_INIT_MARK				0xDEADBEEF
#define	ADSL_INIT_TIME				(30*60*1000)

typedef  struct {
	uint	initMark;
	uint	timeCnt;
	/* add more fields here */
} sdramReservedAreaStruct;
sdramReservedAreaStruct	*pSdramReserved = NULL;
ulong	adslCoreEcUpdateMask = 0;

#define AC_BOOTFLAGS_INIT	0x80000000
ulong	adslCoreBootFlags   = 0;

Boolean	acCmdWakeup = AC_FALSE;
#ifdef SUPPORT_MULTI_PHY
unsigned char acLineReset[2] = { 0, 0 };
#endif

#ifdef CONFIG_BCM_DSL_GFAST
unsigned short bpGpioAFEPwrBoost[2] = {0xFFFF, 0xFFFF};
unsigned char  acPwrBoostLineMask = 0;
#endif

#ifdef VXWORKS
int	ejtagEnable = 0;
#endif

#define gTimeUpdate(x)		((x)->timeUpdate)
#define gPendingFrFlag(x)	((x)->pendingFrFlag)

#define	MAX(a,b) (((a)>(b))?(a):(b))

/*
** PHY reserved memory definitions and functions
*/

typedef struct {
	uchar  *pStart;
	uchar  *pCurrent;
	int    size;
} MemLifoCtrl;
MemLifoCtrl		memBelowPhy, memAbovePhy;

void MemLifoInit(MemLifoCtrl *mCtrl, void *p, int size)
{
	mCtrl->pStart    = p;
	mCtrl->pCurrent  = p;
	mCtrl->size = size;
}

int MemLifoAvail(MemLifoCtrl *mCtrl)
{
	return (mCtrl->pStart + mCtrl->size - mCtrl->pCurrent);
}

void MemLifoPrint(MemLifoCtrl *mCtrl, char *hdr)
{
	printk("%s: start=0x%px cur=0x%px, size=%d avail=%d\n", hdr, mCtrl->pStart, mCtrl->pCurrent, mCtrl->size, MemLifoAvail(mCtrl));
}

void *MemLifoAlloc(MemLifoCtrl *mCtrl, int size)
{
	uchar *p = mCtrl->pCurrent;

	size = (size + 3) & ~3;
	if ((p + size) > (mCtrl->pStart + mCtrl->size))
	  return NULL;
	mCtrl->pCurrent += size;
	MemLifoPrint(mCtrl, "MemAlloc");
	return p;
}

AC_BOOL MemLifoFree(MemLifoCtrl *mCtrl, void *p)
{
	if ((p >=  (void*)mCtrl->pStart) && (p <=  (void *)mCtrl->pCurrent)) {
	  mCtrl->pCurrent  = p;
	  MemLifoPrint(mCtrl, "MemFree");
	  return AC_TRUE;
	}
	else
	  return AC_FALSE;
}

/*
** PHY reserved memory definitions and functions
*/

#ifdef ADSL_SDRAM_RESERVE_SIZE

char *pResVirt = NULL;
char *pResPhys = NULL;
uint resTotalSize = 0;
uint resAlloc = 0;

void	AdslCoreSystemReservedMemInit(void *p, void *physAddr, unsigned int size)
{
	pResVirt = p;
	pResPhys = physAddr;
	resTotalSize = size;
	resAlloc = 0;
	AdslDrvPrintf(TEXT("%s: VA=0x%px PA=0x%px size=0x%X\n"), __FUNCTION__, p, physAddr, size);
}

void	*AdslCoreSystemReservedMemAlloc(unsigned int size, void **ppPhysAddr)
{
	char *pAlloc = pResVirt;

	if (size > (resTotalSize - resAlloc))
		return NULL;

	*ppPhysAddr = pResPhys;
	pResVirt += size;
	pResPhys += size;
	resAlloc += size;

	AdslDrvPrintf(TEXT("%s: VA=0x%px PA=0x%px alloc=0x%X\n"), __FUNCTION__, pAlloc, *ppPhysAddr, resAlloc);
	return pAlloc;
}

void	AdslCoreSystemReservedMemFree(void *p, void *physAddr, unsigned int size)
{
	if (size > resAlloc)
		return;

	pResVirt = p;
	pResPhys = physAddr;
	resAlloc -= size;
	AdslDrvPrintf(TEXT("%s: VA=0x%px PA=0x%px alloc=0x%X\n"), __FUNCTION__, p, physAddr, resAlloc);
}

unsigned int AdslCoreSystemReservedMemAvail(void)
{
	return (resTotalSize - resAlloc);
}
#endif /* ADSL_SDRAM_RESERVE_SIZE */

#ifdef SUPPORT_DSL_BONDING
static void *XdslCoreSetCurDslVars(uchar lineId)
{
	if(lineId < MAX_DSL_LINE)
		gCurDslVars = &acDslVars[lineId];
	return((void *)gCurDslVars);
}

unsigned char XdslCoreGetCurLineId(void)
{
	return(gLineId(gCurDslVars));
}
#endif

void * XdslCoreGetCurDslVars(void)
{
	return((void *)gCurDslVars);
}

void *XdslCoreGetDslVars(unsigned char lineId)
{
	return((lineId < MAX_DSL_LINE)? (void*)&acDslVars[lineId]: (void*)&acDslVars[0]);
}

void XdslCoreSetOvhMsgPrintFlag(uchar lineId, Boolean flag)
{
	void *pDslVars = XdslCoreGetDslVars(lineId);
	gXdslCoreOvhMsgPrintEnabled(pDslVars) = flag;
}

void XdslCoreSetAhifState(uchar lineId, uint state, uint event)
{
	volatile uint	*p = acAhifStatePtr[lineId];
	if ( (NULL != p)  && ((acAhifStateMode != 0) || (0 == event)) )
		*p = ADSL_ENDIAN_CONV_INT32(state);
}

unsigned long AdslCoreGetBootFlags(void)
{
	return adslCoreBootFlags & (~AC_BOOTFLAGS_INIT);
}

/*
**
**		ADSL Core SDRAM memory functions
**
*/

void *AdslCoreGetPhyInfo(void)
{
	return &adslCorePhyDesc;
}

void *AdslCoreGetSdramPageStart(void)
{
	return (void *) adslCorePhyDesc.sdramPageAddr;
}

void *AdslCoreGetSdramImageStart(void)
{
	return (void *) adslCorePhyDesc.sdramImageAddr;
}

uint AdslCoreGetSdramImageSize(void)
{
	return adslCorePhyDesc.sdramImageSize;
}

static void AdslCoreSetSdramTrueSize(void)
{
	uint data[2];
	int	reservedSdramSize = (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE - adslCorePhyDesc.sdramImageAddr);

	BlockByteMove(8, (void*)(adslCorePhyDesc.sdramImageAddr+adslCorePhyDesc.sdramImageSize-8), (void*)&data[0]);
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	AdslDrvPrintf(TEXT("*** %s: data[0]=0x%X data[1]=0x%X ***\n"), __FUNCTION__, data[0], data[1]);
	data[0] = ADSL_SWAP_UINT32(data[0]);
	data[1] = ADSL_SWAP_UINT32(data[1]);
	AdslDrvPrintf(TEXT("*** %s: data[0]=0x%X data[1]=0x%X ***\n"), __FUNCTION__, data[0], data[1]);
#endif
	if( ((uint)-1 == (data[0] + data[1])) &&
		(data[1] > adslCorePhyDesc.sdramImageSize) &&
		(data[1] < reservedSdramSize)) {
		AdslDrvPrintf(TEXT("*** PhySdramSize got adjusted: 0x%X => 0x%X ***\n"), adslCorePhyDesc.sdramImageSize, data[1]);
		adslCorePhyDesc.sdramImageSize = data[1];
	}
	adslCorePhyDesc.sdramImageSize = (adslCorePhyDesc.sdramImageSize+0xF) & ~0xF;
}

void	AdslCoreSetXfaceOffset(void *lmemAddr, uint lmemSize)
{
	uint	xfaceAddr;
	
#if defined(CONFIG_BCM963146)
	xfaceAddr = ((uint*)lmemAddr)[4];
	AdslDrvPrintf(TEXT("*** %s: XfaceAddr=0x%X ***\n"), __FUNCTION__, xfaceAddr);
#else
	uint	data[2];
	BlockByteMove(8, (void*)((uintptr_t)lmemAddr+lmemSize-8), (void*)&data[0]);
	data[0] = ADSL_ENDIAN_CONV_INT32(data[0]);
	data[1] = ADSL_ENDIAN_CONV_INT32(data[1]);
	AdslDrvPrintf(TEXT("*** %s: data[0]=0x%X data[1]=0x%X ***\n"), __FUNCTION__, data[0], data[1]);
	xfaceAddr = ADSL_ENDIAN_CONV_INT32(((uint*)lmemAddr)[4]);
#endif
	if (0 != xfaceAddr) {
		AdslDrvPrintf(TEXT("*** XfaceOffset4: 0x%X => 0x%X ***\n"), adslPhyXfaceOffset, xfaceAddr & 0xFFFFF);
		adslPhyXfaceOffset = xfaceAddr & 0xFFFFF;
	}
#ifndef CONFIG_BCM963146
	else if( ((uint)-1 == (data[0] + data[1])) 
#if defined(CONFIG_BCM96362)
		&& (data[1] <= 0x27F90)
#elif defined(CONFIG_BCM96328)
		&& (data[1] <= 0x21F90)
#elif defined(CONFIG_BCM963268)
		&& (data[1] <= 0x6FF90)
#elif defined(CONFIG_BCM96318)
		&& (data[1] <= 0x1C790)
#elif defined(CONFIG_BCM963138)
		&& (data[1] <= 0x9FF90)
#elif defined(CONFIG_BCM963381)
		&& (data[1] <= 0x5FF90)
#elif defined(CONFIG_BCM963148)
		&& (data[1] <= 0x83F90)
#elif defined(CONFIG_BCM963158)
		&& (data[1] <= 0xAFF90)
#elif defined(CONFIG_BCM963178)
		&& (data[1] <= 0x6FF90)
#endif
	)
	{
		AdslDrvPrintf(TEXT("*** XfaceOffset: 0x%X => 0x%X ***\n"), adslPhyXfaceOffset, data[1]);
		adslPhyXfaceOffset = data[1];
	}
#endif
}

void * AdslCoreSetSdramImageAddr(uint lmem2, uint pgSize, uint sdramSize)
{
	uint sdramPhyImageAddr;
	uintptr_t origSdramAddr, newSdramAddr;
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	AdslDrvPrintf(TEXT("%s: lmem2=0x%x, pgSize=0x%X sdramSize=0x%X\n"), __FUNCTION__, lmem2, pgSize, sdramSize);
#endif
	if (0 == pgSize)
	  pgSize = 0x200000;

	if (0 == lmem2) {
		lmem2 = (sdramSize > 0x40000) ? 0x20000 : 0x40000;
		sdramPhyImageAddr = ADSL_PHY_SDRAM_START + lmem2;
	}
	else {
		sdramPhyImageAddr = lmem2;
	}
	
	lmem2 &= (pgSize-1);

	origSdramAddr = adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_BIAS;
	newSdramAddr  = adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE - pgSize + lmem2;
	AdslDrvPrintf(TEXT("%s: lmem2(0x%x) vs ADSL_PHY_SDRAM_BIAS(0x%x); origAddr=0x%lX newAddr=0x%lX\n"), __FUNCTION__, 
		lmem2, ADSL_PHY_SDRAM_BIAS, origSdramAddr, newSdramAddr);

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)
	if (newSdramAddr < (origSdramAddr & ~0xFFFFF))
#else
	if(lmem2 < ADSL_PHY_SDRAM_BIAS)
#endif
	{
		return NULL;
	}

	adslCorePhyDesc.sdramPhyImageAddr = sdramPhyImageAddr;
	adslCorePhyDesc.sdramPageSize = pgSize;
	adslCorePhyDesc.sdramImageAddr = newSdramAddr;
	adslCorePhyDesc.sdramImageAddr0 = adslCorePhyDesc.sdramPageAddr0 + ADSL_PHY_SDRAM_PAGE_SIZE - pgSize + lmem2;
	adslCorePhyDesc.sdramImageSize = sdramSize;
	pSdramReserved = (void *) (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE - sizeof(sdramReservedAreaStruct));
	AdslDrvPrintf(TEXT("pSdramPHY=0x%px, 0x%X 0x%X\n"), pSdramReserved, pSdramReserved->timeCnt, pSdramReserved->initMark);
	
	adslCoreEcUpdateMask = 0;

#if defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 13)
	if ((kerSysGetResetStatus() & SW_RESET_STATUS) && (0 == adslCoreBootFlags))
		adslCoreBootFlags |= AC_SOFTBOOT;
	if (kerSysGetResetStatus() & (HW_RESET_STATUS | POR_RESET_STATUS))
#else
	if (ADSL_INIT_MARK != pSdramReserved->initMark)
#endif
	{
		pSdramReserved->timeCnt = 0;
		pSdramReserved->initMark = ADSL_INIT_MARK;
		if (0 == adslCoreBootFlags)
		  adslCoreBootFlags |= AC_HARDBOOT;
	}
	if (pSdramReserved->timeCnt >= ADSL_INIT_TIME)
		adslCoreEcUpdateMask |= kDigEcShowtimeFastUpdateDisabled;
	
	adslCoreBootFlags |= AC_BOOTFLAGS_INIT;
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	AdslDrvPrintf(TEXT("%s: sdramPageAddr=0x%lx, sdramImageAddr=0x%lx, sdramPhyImageAddr=0x%x\n"), __FUNCTION__,
		adslCorePhyDesc.sdramPageAddr,
		adslCorePhyDesc.sdramImageAddr,
		adslCorePhyDesc.sdramPhyImageAddr);
#endif
	return (void *) adslCorePhyDesc.sdramImageAddr;
}

static Boolean AdslCoreIsPhySdramAddr(void *ptr)
{
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	uintptr_t	addr = (uintptr_t) ptr;
#else
	uintptr_t	addr = ((uintptr_t) ptr) | 0xA0000000;
#endif
	return (addr >= adslCorePhyDesc.sdramImageAddr) && (addr < (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE));
}

#define ADSL_PHY_SDRAM_SHARED_START		(adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE - ADSL_SDRAM_RESERVED)
#if defined(SUPPORT_DSL_BONDING) || defined(PHY_CO)
/* 2328/2296(ARM/MIPS) bytes worst case: kDslAfeInfoCmd(180*2) + kDslExtraPhyCfgCmd(16*2) + kDslStartPhysicalLayerCmd(968*2) */
#define SHARE_MEM_REQUIRE			4096
/* PHY_CO target */
/* 2014 bytes worst case: kDslAfeInfoCmd(180) + kXDslSetHmiCoreConfig(6) +kXDslSetConfigGfast(452) + kXDslSetRfiConfigGfast(392) + kDslExtraPhyCfgCmd(16) + kDslStartPhysicalLayerCmd(968) */
#else
#define SHARE_MEM_REQUIRE			2048
#endif
static int	adslPhyShareMemSizeAllow	= 0;
static void	* adslPhyShareMemStart		= NULL;
static void	*pAdslSharedMemAlloc		= NULL;
static void	*adslPhyShareMemCalloc		= NULL;
#if (defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)) && !defined(_NOOS)
static dma_addr_t	_adslPhyShareMemCalloc	= 0;
#endif

void AdslCoreSharedMemInit(void)
{
	int	reservedSdramSize = (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE - adslCorePhyDesc.sdramImageAddr);
	int shareMemAvailable = reservedSdramSize - ADSL_SDRAM_RESERVED - AdslCoreGetSdramImageSize();
	
	if( NULL == adslPhyShareMemCalloc ) {
		if( shareMemAvailable < SHARE_MEM_REQUIRE ) {
#if (defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)) && !defined(_NOOS)
			adslPhyShareMemCalloc = dma_alloc_coherent(NULL, SHARE_MEM_REQUIRE, &_adslPhyShareMemCalloc, GFP_KERNEL);
			adslPhyShareMemStart = (void *)((uintptr_t)adslPhyShareMemCalloc + SHARE_MEM_REQUIRE);
#else
			adslPhyShareMemCalloc = calloc(1,SHARE_MEM_REQUIRE);
			adslPhyShareMemStart = (void *)(0xA0000000 | ((uintptr_t)adslPhyShareMemCalloc + SHARE_MEM_REQUIRE));
#endif
			adslPhyShareMemSizeAllow = SHARE_MEM_REQUIRE;
		}
		else {
			adslPhyShareMemStart = (void *) ADSL_PHY_SDRAM_SHARED_START;
			adslPhyShareMemSizeAllow = shareMemAvailable & ~0xf;
		}
	}
	else if(shareMemAvailable > SHARE_MEM_REQUIRE) {
#if (defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)) && !defined(_NOOS)
		dma_free_coherent(NULL, SHARE_MEM_REQUIRE, adslPhyShareMemCalloc, _adslPhyShareMemCalloc);
#else
		free(adslPhyShareMemCalloc);
#endif
		adslPhyShareMemCalloc = NULL;
		adslPhyShareMemStart = (void *) ADSL_PHY_SDRAM_SHARED_START;
		adslPhyShareMemSizeAllow = shareMemAvailable & ~0xf;
	}
	
	MemLifoInit(&memAbovePhy, (void *) (adslCorePhyDesc.sdramImageAddr + adslCorePhyDesc.sdramImageSize), adslPhyShareMemSizeAllow - SHARE_MEM_REQUIRE);
	MemLifoInit(&memBelowPhy, (void *) (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_BIAS), adslCorePhyDesc.sdramImageAddr - (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_BIAS));

	adslPhyShareMemSizeAllow = SHARE_MEM_REQUIRE;
	pAdslSharedMemAlloc = adslPhyShareMemStart;
	AdslDrvPrintf(TEXT("%s: shareMemStart=%px shareMemSize=%d\n"), __FUNCTION__, adslPhyShareMemStart, adslPhyShareMemSizeAllow);

	MemLifoPrint(&memAbovePhy, "AbovePHY");
	MemLifoPrint(&memBelowPhy, "BelowPHY");
}

void XdslCoreReservedSharedMemFree(uchar lineId, void *p)
{
	AdslCorePhyReservedMemFree(p);
}

void *XdslCoreReservedSharedMemAlloc(uchar lineId, long size)
{
	return AdslCorePhyReservedMemAlloc(size);
}

void AdslCoreSharedMemFree(void *p)
{
	BcmCoreDpcSyncEnter(SYNC_SHAREMEM);
	pAdslSharedMemAlloc = adslPhyShareMemStart;
	BcmCoreDpcSyncExit(SYNC_SHAREMEM);
}

void *AdslCoreSharedMemAlloc(long size)
{
	uintptr_t	addr;
	
	BcmCoreDpcSyncEnter(SYNC_SHAREMEM);
	addr = ((uintptr_t)pAdslSharedMemAlloc - (uintptr_t)size) & ~0xF;
	if (addr <= ((uintptr_t)adslPhyShareMemStart - adslPhyShareMemSizeAllow)) {
		BcmAdslCoreDiagWriteStatusString(0, "***No shared SDRAM!!! ptr=0x%px size=%d\n", pAdslSharedMemAlloc, size);
		pAdslSharedMemAlloc = adslPhyShareMemStart;
		addr = ((uintptr_t)pAdslSharedMemAlloc - (uintptr_t)size) & ~0xF;
	}

	pAdslSharedMemAlloc = (void *) addr;
	BcmCoreDpcSyncExit(SYNC_SHAREMEM);
	
	return pAdslSharedMemAlloc;
}

void *AdslCorePhyReservedMemAlloc(long size)
{
	uchar	*p;

	if (NULL != (p = MemLifoAlloc(&memAbovePhy, size)))
	  return p;
	return MemLifoAlloc(&memBelowPhy, size);
}

AC_BOOL AdslCorePhyReservedMemFree(void *p)
{
	if (MemLifoFree(&memAbovePhy, p))
	  return AC_TRUE;
	if (MemLifoFree(&memBelowPhy, p))
	  return AC_TRUE;

	return AC_FALSE;
}

unsigned int AdslCorePhyReservedMemAvail()
{
	int	memAvail0 = MemLifoAvail(&memBelowPhy);
	int	memAvail1 = MemLifoAvail(&memAbovePhy);

	return MAX(memAvail0, memAvail1);
}


/*
**
**		ADSL Core Status/Command functions 
**
*/

void AdslCoreSetL2Timeout(ulong val)
{
	if( 0 == val)
		gL2SwitchL0TimeMs = (ulong)-1;
	else
		gL2SwitchL0TimeMs = val * 1000;	/* Convert # of Sec to Ms */
}
void AdslCoreIndicateLinkPowerStateL2(uchar lineId)
{
	void *pDslVars = XdslCoreGetDslVars(lineId);
	
	gTimeInL2Ms(pDslVars) = 0;
#ifdef DEBUG_L2_RET_L0
	bcmOsGetTime(&gL2Start);
#endif
}

void AdslCoreIndicateLinkDown(uchar lineId)
{
	dslStatusStruct status;
	void *pDslVars = XdslCoreGetDslVars(lineId);
#ifdef SUPPORT_DSL_BONDING
	status.code = (lineId << DSL_LINE_SHIFT) | kDslEscapeToG994p1Status;
#else
	status.code = kDslEscapeToG994p1Status;
#endif
	AdslMibStatusSnooper(pDslVars, &status);
}

void AdslCoreIndicateLinkUp(uchar lineId)
{
	dslStatusStruct status;
#ifdef SUPPORT_DSL_BONDING
	status.code = kDslTrainingStatus | (lineId << DSL_LINE_SHIFT);
#else
	status.code = kDslTrainingStatus;
#endif
	status.param.dslTrainingInfo.code = kDslG992p2RxShowtimeActive;
	status.param.dslTrainingInfo.value= 0;
	AdslMibStatusSnooper(XdslCoreGetDslVars(lineId), &status);
	
#ifdef SUPPORT_DSL_BONDING
	status.code = kDslTrainingStatus | (lineId << DSL_LINE_SHIFT);
#else
	status.code = kDslTrainingStatus;
#endif
	status.param.dslTrainingInfo.code = kDslG992p2TxShowtimeActive;
	status.param.dslTrainingInfo.value= 0;

	AdslMibStatusSnooper(XdslCoreGetDslVars(lineId), &status);
}

void AdslCoreIndicateLinkPowerState(uchar lineId, int pwrState)
{
	dslStatusStruct status;
#ifdef SUPPORT_DSL_BONDING
	status.code = kDslConnectInfoStatus | (lineId << DSL_LINE_SHIFT);
#else
	status.code = kDslConnectInfoStatus;
#endif
	status.param.dslConnectInfo.code = kG992p3PwrStateInfo;
	status.param.dslConnectInfo.value= pwrState;
	AdslMibStatusSnooper(XdslCoreGetDslVars(lineId), &status);
}

static AC_BOOL AdslCoreIsWakeupCommand(dslCommandStruct *cmdPtr)
{
	dslCommandCode  cmd = DSL_COMMAND_CODE(cmdPtr->command);

	return (kDslStartPhysicalLayerCmd == cmd) || (kDslPingCmd == cmd) || (kDslTestCmd == cmd);
}

AC_BOOL AdslCoreCommandWrite(dslCommandStruct *cmdPtr)
{
	int				n;
	
	n = FlattenBufferCommandWrite(pPhySbCmd, cmdPtr);
	if (n > 0) {
#if !defined(BCM_CORE_NO_HARDWARE)
	  if (acCmdWakeup || AdslCoreIsWakeupCommand(cmdPtr)) {
		volatile uint	*pAdslEnum;

		pAdslEnum = (uint *) XDSL_ENUM_BASE;
		pAdslEnum[ADSL_HOSTMESSAGE] = 1;
	  }
#endif
		return AC_TRUE;
	}
	return AC_FALSE;
}

int AdslCoreFlattenCommand(void *cmdPtr, void *dstPtr, uint nAvail)
{
	return FlattenCommand (cmdPtr, dstPtr, nAvail);
}

AC_BOOL AdslCoreCommandIsPending(void)
{
	return StretchBufferGetReadAvail(pPhySbCmd) ? AC_TRUE : AC_FALSE;
}

#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
extern void BcmXdslCoreSendCmdToExtBondDev(dslCommandStruct *cmdPtr);
#endif

#ifdef ADSLDRV_LITTLE_ENDIAN
void G992p3DataPumpCapabilitiesCopy(g992p3DataPumpCapabilities *pDst, g992p3DataPumpCapabilities *pSrc)
{
	*pDst = *pSrc;
	pDst->rcvNOMPSDus = ADSL_ENDIAN_CONV_SHORT(pSrc->rcvNOMPSDus);
	pDst->rcvMAXNOMPSDus = ADSL_ENDIAN_CONV_SHORT(pSrc->rcvMAXNOMPSDus);
	pDst->rcvMAXNOMATPus = ADSL_ENDIAN_CONV_SHORT(pSrc->rcvMAXNOMATPus);
	BlockShortMoveReverse(kG992p3MaxSpectBoundsUpSize, pSrc->usSubcarrierIndex, pDst->usSubcarrierIndex);
	BlockShortMoveReverse(kG992p3MaxSpectBoundsUpSize, pSrc->usLog_tss, pDst->usLog_tss);
	pDst->numUsSubcarrier = ADSL_ENDIAN_CONV_SHORT(pSrc->numUsSubcarrier);
	pDst->rcvNOMPSDds = ADSL_ENDIAN_CONV_SHORT(pSrc->rcvNOMPSDds);
	pDst->rcvMAXNOMPSDds = ADSL_ENDIAN_CONV_SHORT(pSrc->rcvMAXNOMPSDds);
	pDst->rcvMAXNOMATPds = ADSL_ENDIAN_CONV_SHORT(pSrc->rcvMAXNOMATPds);
	BlockShortMoveReverse(kG992p3MaxSpectBoundsDownSize, pSrc->dsSubcarrierIndex, pDst->dsSubcarrierIndex);
	BlockShortMoveReverse(kG992p3MaxSpectBoundsDownSize, pSrc->dsLog_tss, pDst->dsLog_tss);
	pDst->numDsSubcarrier = ADSL_ENDIAN_CONV_SHORT(pSrc->numDsSubcarrier);
	BlockShortMoveReverse(4, pSrc->minDownSTM_TPS_TC, pDst->minDownSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDownSTM_TPS_TC, pDst->maxDownSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minRevDownSTM_TPS_TC, pDst->minRevDownSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDelayDownSTM_TPS_TC, pDst->maxDelayDownSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minUpSTM_TPS_TC, pDst->minUpSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxUpSTM_TPS_TC, pDst->maxUpSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minRevUpSTM_TPS_TC, pDst->minRevUpSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDelayUpSTM_TPS_TC, pDst->maxDelayUpSTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDownPMS_TC_Latency, pDst->maxDownPMS_TC_Latency);
	BlockShortMoveReverse(4, pSrc->maxUpPMS_TC_Latency, pDst->maxUpPMS_TC_Latency);
	BlockShortMoveReverse(4, pSrc->maxDownR_PMS_TC_Latency, pDst->maxDownR_PMS_TC_Latency);
	BlockShortMoveReverse(4, pSrc->maxDownD_PMS_TC_Latency, pDst->maxDownD_PMS_TC_Latency);
	BlockShortMoveReverse(4, pSrc->maxUpR_PMS_TC_Latency, pDst->maxUpR_PMS_TC_Latency);
	BlockShortMoveReverse(4, pSrc->maxUpD_PMS_TC_Latency, pDst->maxUpD_PMS_TC_Latency);
	BlockShortMoveReverse(4, pSrc->minDownATM_TPS_TC, pDst->minDownATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDownATM_TPS_TC, pDst->maxDownATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minRevDownATM_TPS_TC, pDst->minRevDownATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDelayDownATM_TPS_TC, pDst->maxDelayDownATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minUpATM_TPS_TC, pDst->minUpATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxUpATM_TPS_TC, pDst->maxUpATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minRevUpATM_TPS_TC, pDst->minRevUpATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDelayUpATM_TPS_TC, pDst->maxDelayUpATM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minDownPTM_TPS_TC, pDst->minDownPTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDownPTM_TPS_TC, pDst->maxDownPTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minRevDownPTM_TPS_TC, pDst->minRevDownPTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDelayDownPTM_TPS_TC, pDst->maxDelayDownPTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minUpPTM_TPS_TC, pDst->minUpPTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxUpPTM_TPS_TC, pDst->maxUpPTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->minRevUpPTM_TPS_TC, pDst->minRevUpPTM_TPS_TC);
	BlockShortMoveReverse(4, pSrc->maxDelayUpPTM_TPS_TC, pDst->maxDelayUpPTM_TPS_TC);
	pDst->subModePSDMasks = ADSL_ENDIAN_CONV_SHORT(pSrc->subModePSDMasks);
}

void G993p2DataPumpCapabilitiesCopy(g993p2DataPumpCapabilities *pDst, g993p2DataPumpCapabilities *pSrc)
{
	pDst->verId = ADSL_ENDIAN_CONV_SHORT(pSrc->verId);
	pDst->size = ADSL_ENDIAN_CONV_SHORT(pSrc->size);
	
	pDst->profileSel = ADSL_ENDIAN_CONV_INT32(pSrc->profileSel);
	pDst->maskUS0 = ADSL_ENDIAN_CONV_INT32(pSrc->maskUS0);
	pDst->cfgFlags = ADSL_ENDIAN_CONV_INT32(pSrc->cfgFlags);

	pDst->maskEU = ADSL_ENDIAN_CONV_INT32(pSrc->maskEU);
	pDst->maskADLU = ADSL_ENDIAN_CONV_INT32(pSrc->maskADLU);
	pDst->annex = ADSL_ENDIAN_CONV_INT32(pSrc->annex);
}

#endif

extern Bool	gSharedMemAllocFromUserContext;

AC_BOOL AdslCoreCommandHandler(void *cmdPtr)
{
	dslCommandStruct	*cmd = (dslCommandStruct *) cmdPtr;
#ifdef G992P3
	adslMibInfo		*pMibInfo;
	long			mibLen;
	g992p3DataPumpCapabilities	*pG992p3Cap = cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA;
	g992p3DataPumpCapabilities	*pTmpG992p3Cap;
	int							n;
#ifdef G992P5
	g992p3DataPumpCapabilities	*pG992p5Cap = cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA;
#endif
#ifdef G993
	g993p2DataPumpCapabilities	*pG993p2Cap = cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA;
#endif
#endif
	AC_BOOL	bRes;
	void	*pDslVars;
	uchar	lineId = DSL_LINE_ID(cmd->command);
	pDslVars = XdslCoreGetDslVars(lineId);
	
#ifdef SUPPORT_DSL_BONDING
	if((1 == lineId) && !ADSL_PHY_SUPPORT(kAdslPhyBonding))
		return AC_FALSE;
#endif

	switch (DSL_COMMAND_CODE(cmd->command)) {
#ifdef ADSL_MIB
		case kDslDiagStartBERT:
			BcmCoreDpcSyncEnter(SYNC_RX);
			AdslMibClearBertResults(pDslVars);
			BcmCoreDpcSyncExit(SYNC_RX);
			break;
		case kDslDiagStopBERT:
		{
			adslMibInfo			*pMibInfo;
			long				mibLen;

			mibLen = sizeof(adslMibInfo);
			
			BcmCoreDpcSyncEnter(SYNC_RX);
			pMibInfo = (void *) AdslMibGetObjectValue (pDslVars, NULL, 0, NULL, &mibLen);
			DiagWriteString(lineId, DIAG_DSL_CLIENT, "BERT Status = %s\n"
					"BERT Total Time	= %u sec\n"
					"BERT Elapsed Time = %u sec\n",
					pMibInfo->adslBertStatus.bertSecCur != 0 ? "RUNNING" : "NOT RUNNING",
					pMibInfo->adslBertStatus.bertSecTotal, pMibInfo->adslBertStatus.bertSecElapsed);
			if (pMibInfo->adslBertStatus.bertSecCur != 0)
				AdslMibBertStopEx(pDslVars);
			else
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "BERT_EX results: totalBits=0x%08X%08X, errBits=0x%08X%08X\n",
					pMibInfo->adslBertStatus.bertTotalBits.cntHi, pMibInfo->adslBertStatus.bertTotalBits.cntLo,
					pMibInfo->adslBertStatus.bertErrBits.cntHi, pMibInfo->adslBertStatus.bertErrBits.cntLo);
			BcmCoreDpcSyncExit(SYNC_RX);
		}
			break;
		case kDslDyingGaspCmd:
			AdslMibSetLPR(pDslVars);
			break;
#endif
#ifdef G992P3
		case kDslStartPhysicalLayerCmd:
			BcmCoreDpcSyncEnter(SYNC_RX);
			pMibInfo = (void *) AdslMibGetObjectValue (pDslVars, NULL, 0, NULL, &mibLen);
			if(1 == pG992p3Cap->diagnosticsModeEnabled)
				pMibInfo->adslPhys.adslLDCompleted = 1;	/* Loop Diagnostic is in progress */
			else
				pMibInfo->adslPhys.adslLDCompleted = 0;
			
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2)) {
				n = sizeof(g992p3DataPumpCapabilities);
#ifdef G992P5
				if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p))
					n += sizeof(g992p3DataPumpCapabilities);
#endif
#ifdef G993
				if (ADSL_PHY_SUPPORT(kAdslPhyVdslG993p2))
					n += sizeof(g993p2DataPumpCapabilities);
#endif
				gSharedMemAllocFromUserContext=1;
				pTmpG992p3Cap = AdslCoreSharedMemAlloc(n+0x10);
				if (NULL != pTmpG992p3Cap) {
#ifdef ADSLDRV_LITTLE_ENDIAN
					G992p3DataPumpCapabilitiesCopy(pTmpG992p3Cap, pG992p3Cap);
#else
					*pTmpG992p3Cap = *pG992p3Cap;
#endif
#ifdef G992P5
					if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p)) {
#ifdef ADSLDRV_LITTLE_ENDIAN
						G992p3DataPumpCapabilitiesCopy(pTmpG992p3Cap+1, pG992p5Cap);
#elif defined(CONFIG_ARM64)
						BlockByteMoveDstAlign(sizeof(g992p3DataPumpCapabilities),(uchar*)pG992p5Cap, (uchar*)(pTmpG992p3Cap+1));
#else
						*(pTmpG992p3Cap+1) = *pG992p5Cap;
#endif
					}
#endif
#ifdef G993
					if (ADSL_PHY_SUPPORT(kAdslPhyVdslG993p2)) {
#ifdef ADSLDRV_LITTLE_ENDIAN
						G993p2DataPumpCapabilitiesCopy((g993p2DataPumpCapabilities *)(pTmpG992p3Cap+2), pG993p2Cap);
#elif defined(CONFIG_ARM64)
						BlockByteMoveDstAlign(sizeof(g993p2DataPumpCapabilities),(uchar*)pG993p2Cap, (uchar*)(pTmpG992p3Cap+2));
#else
						*((g993p2DataPumpCapabilities *)(pTmpG992p3Cap+2)) = *pG993p2Cap;
#endif
					}
#endif
				}
				else
					return AC_FALSE;
				cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA = pTmpG992p3Cap;
#ifdef G992P5
				cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA = pTmpG992p3Cap+1;
#endif
#ifdef G993
				if (ADSL_PHY_SUPPORT(kAdslPhyVdslG993p2)) {
					cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA = (g993p2DataPumpCapabilities *)(pTmpG992p3Cap+2);
					if (kGfastAnnexA == (cmd->param.dslModeSpec.capabilities.modulations & (kG993p2AnnexA | kGfastAnnexA))) {
						ulong				cmdBuf[4];
						dslCommandStruct	*cmd1    = (dslCommandStruct*) cmdBuf;
						cmd1->command = kDslSendEocCommand;
						cmd1->param.dslClearEocMsg.msgId = kDslVdslCaps;
						cmd1->param.dslClearEocMsg.dataPtr = (void *) cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA;
						cmd1->param.dslClearEocMsg.msgType = sizeof(g993p2DataPumpCapabilities);
						AdslCoreCommandHandler(cmd1);
					}
				}
				else
					cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA = NULL;
#endif

			}
			XdslMibCmdSnooper(pDslVars, cmd);
			AdslCoreIndicateLinkPowerState(lineId, 0);
			BcmCoreDpcSyncExit(SYNC_RX);
			break;
#ifdef  ADSLDRV_LITTLE_ENDIAN
		case kDslSendEocCommand:
			if(kDslExtraPhyCfgCmd == cmd->param.dslClearEocMsg.msgId) {
				/* Re-using pG992p3Cap and pTmpG992p3Cap variables */
				gSharedMemAllocFromUserContext=1;
				pTmpG992p3Cap = AdslCoreSharedMemAlloc(sizeof(dslExtraCfgCommand));
				BlockLongMoveReverse(sizeof(dslExtraCfgCommand)>>2, (int *)cmd->param.dslClearEocMsg.dataPtr, (int *)pTmpG992p3Cap);
				pG992p3Cap = (void *)cmd->param.dslClearEocMsg.dataPtr;
				cmd->param.dslClearEocMsg.dataPtr = (char *)pTmpG992p3Cap;
			}
			break;
#endif
		case kDslTestCmd:
			BcmCoreDpcSyncEnter(SYNC_RX);
			AdslCoreIndicateLinkPowerState(lineId, 0);
			BcmCoreDpcSyncExit(SYNC_RX);
			break;

		case kDslOLRRequestCmd:
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2) && !AdslCoreIsPhySdramAddr(cmd->param.dslOLRRequest.carrParamPtr)) {
				if ((kAdslModAdsl2p == AdslMibGetModulationType(pDslVars)) ||
					(kVdslModVdsl2 == AdslMibGetModulationType(pDslVars)))
					n = cmd->param.dslOLRRequest.nCarrs * sizeof(dslOLRCarrParam2p);
				else
					n = cmd->param.dslOLRRequest.nCarrs * sizeof(dslOLRCarrParam);
				gSharedMemAllocFromUserContext=1;
				pTmpG992p3Cap = AdslCoreSharedMemAlloc(n);
				BlockByteMove (n, cmd->param.dslOLRRequest.carrParamPtr, (void*)pTmpG992p3Cap);
				cmd->param.dslOLRRequest.carrParamPtr = pTmpG992p3Cap;
			}
			break;
#endif

		case kDslIdleCmd:
			BcmCoreDpcSyncEnter(SYNC_RX);
			XdslMibCmdSnooper(pDslVars, cmd);
			BcmCoreDpcSyncExit(SYNC_RX);
			break;

#ifdef G992P3
		case kDslDownCmd:
			BcmCoreDpcSyncEnter(SYNC_RX);
			pMibInfo = (void *) AdslMibGetObjectValue (pDslVars, NULL, 0, NULL, &mibLen);
			if(2 != pMibInfo->adslPhys.adslLDCompleted)
				pMibInfo->adslPhys.adslLDCompleted = 0;	/* Clear the state as Loop Diagnostic is not in successfully completed state */
			XdslMibCmdSnooper(pDslVars, cmd);
#ifdef SUPPORT_MULTI_PHY
			acLineReset[lineId] = 1;
#endif
			BcmCoreDpcSyncExit(SYNC_RX);
			break;
#endif

		case kDslLoopbackCmd:
			BcmCoreDpcSyncEnter(SYNC_RX);
			AdslCoreIndicateLinkUp(lineId);
			BcmCoreDpcSyncExit(SYNC_RX);
			break;

		default:
			break;
	}

	{
	ulong	irqFlags;
	BcmCoreDpcSyncIntrDisableEnter(SYNC_TX, irqFlags);
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
	if(0 == lineId)
		bRes = AdslCoreCommandWrite(cmd);
	else {
		BcmXdslCoreSendCmdToExtBondDev(cmd);
		bRes = AC_TRUE;
	}
#else
	bRes = AdslCoreCommandWrite(cmd);
#endif
	BcmCoreDpcSyncIntrDisableExit(SYNC_TX, irqFlags);
	}
	gSharedMemAllocFromUserContext=0;
#ifdef DIAG_DBG
	if(bRes == AC_FALSE)
		printk("%s: AdslCoreCommandWrite failed!", __FUNCTION__);
#endif

#ifdef G992P3
	if (kDslStartPhysicalLayerCmd == DSL_COMMAND_CODE(cmd->command)) {
		cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA = pG992p3Cap;
#ifdef G992P5
		cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA = pG992p5Cap;
#endif
#ifdef G993
		cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA = pG993p2Cap;
#endif

	}
#ifdef  ADSLDRV_LITTLE_ENDIAN
	if ((kDslSendEocCommand == DSL_COMMAND_CODE(cmd->command)) && (kDslExtraPhyCfgCmd == cmd->param.dslClearEocMsg.msgId))
		cmd->param.dslClearEocMsg.dataPtr = (char *)pG992p3Cap;
#endif
#endif	/* G992P3 */

	return bRes;
}

#ifdef SUPPORT_STATUS_BACKUP
extern Public int	BackUpFlattenBufferStatusRead(stretchHostBufferStruct *fBuf, dslStatusStruct *status);
#endif

int AdslCoreStatusRead (dslStatusStruct *status)
{
	int		n;

	if (acBlockStatusRead)
		return 0;
#ifdef SUPPORT_STATUS_BACKUP
	n = (pCurSbSta==pPhySbSta)? FlattenBufferStatusRead(pPhySbSta, status): BackUpFlattenBufferStatusRead(pLocSbSta, status);
#else
	n = FlattenBufferStatusRead(pPhySbSta, status);
#endif
	if (n < 0) {
		BcmAdslCoreDiagWriteStatusString (0, "Status read failure: len=%d, st.code=%u, st.value=%d\n",
			-n, (uint) status->code, status->param.value);
		n = 0;
	}

	return n;
}

void AdslCoreStatusReadComplete (int nBytes)
{
#ifdef SUPPORT_STATUS_BACKUP
	if(pCurSbSta==pPhySbSta)
		StretchBufferReadUpdate (pPhySbSta, nBytes);
	else
		HostStretchBufferReadUpdate (pLocSbSta, nBytes);
#else
	StretchBufferReadUpdate (pPhySbSta, nBytes);
#endif
}

AC_BOOL AdslCoreStatusAvail (void)
{
	return StretchBufferGetReadAvail(pPhySbSta) ? AC_TRUE : AC_FALSE;
}

void *AdslCoreStatusReadPtr (void)
{
	uintptr_t p;
#ifdef SUPPORT_STATUS_BACKUP
	p = (pCurSbSta==pPhySbSta)? StretchBufferGetReadPtr(pPhySbSta): (uintptr_t)HostStretchBufferGetReadPtr(pLocSbSta);
#else
	p = StretchBufferGetReadPtr(pPhySbSta);
#endif

#ifdef FLATTEN_ADDR_ADJUST
#ifdef SUPPORT_STATUS_BACKUP
	if(pCurSbSta == pPhySbSta)
		p = (uintptr_t)(ADSL_ADDR_TO_HOST( p));
#else
	p = (uintptr_t)(ADSL_ADDR_TO_HOST( p));
#endif
#endif

	return (void *)p;
}

int AdslCoreStatusAvailTotal (void)
{
#ifdef SUPPORT_STATUS_BACKUP
	return ( (pCurSbSta==pPhySbSta)? StretchBufferGetReadAvailTotal(pPhySbSta): HostStretchBufferGetReadAvailTotal(pLocSbSta) );
#else
	return StretchBufferGetReadAvailTotal(pPhySbSta);
#endif
}

#ifdef SUPPORT_STATUS_BACKUP
AC_BOOL AdslCoreSwitchCurSbStatToSharedSbStat(void)
{
	AC_BOOL res = FALSE;
	if(pCurSbSta != pPhySbSta) {
		pCurSbSta = pPhySbSta;
		/* Need to re-init to avoid the potential of wrap-around when backing up statuses */
		FlattenHostBufferInit(pLocSbSta, pLocSbSta+1, STAT_BKUP_BUF_SIZE, kMaxFlattenedStatusSize);
		res = TRUE;
	}
	return res;
}


#ifdef ADSLDRV_STATUS_PROFILING
extern ulong gBkupStartAtIntrCnt;
extern ulong adslCoreIntrCnt;
extern ulong gBkupStartAtdpcCnt;
extern ulong adslCoreIsrTaskCnt;
#endif
/* Called from interrupt, the bottom half is scheduled but have not started processing statuses yet */
void AdslCoreBkupSbStat(void)
{
	int byteAvail;
	void *wrPtr;
	uintptr_t rdPtr;
	
	if(pCurSbSta == pPhySbSta) {
		if((StretchBufferGetReadAvailTotal(pPhySbSta) >= gStatusBackupThreshold) && (NULL != pLocSbSta)) {
#ifdef ADSLDRV_STATUS_PROFILING
			gBkupStartAtIntrCnt=adslCoreIntrCnt;
			gBkupStartAtdpcCnt=adslCoreIsrTaskCnt;
#endif
			byteAvail = StretchBufferGetReadAvail(pPhySbSta);
			/* Loop 2 times max when the shared SB byteAvailTotal includes wrap-around part */
			while((byteAvail > 0) && (byteAvail <= (HostStretchBufferGetWriteAvail(pLocSbSta) - 12))) {
				wrPtr = HostStretchBufferGetWritePtr(pLocSbSta);
				rdPtr = StretchBufferGetReadPtr(pPhySbSta);
#ifdef FLATTEN_ADDR_ADJUST
				rdPtr = (uintptr_t)(ADSL_ADDR_TO_HOST( rdPtr));
#endif
				BlockLongMove((((byteAvail>>2)+3)&~3), (int *)rdPtr, (int *)wrPtr);
				StretchBufferReadUpdate(pPhySbSta, byteAvail);
				HostStretchBufferWriteUpdate(pLocSbSta, byteAvail);
				pCurSbSta = (stretchBufferStruct *)pLocSbSta;
				byteAvail = StretchBufferGetReadAvail(pPhySbSta);
			}
		}
	}
	else {
		/* Backup has been started and the bottom half is still not running, continue backing up statuses */
		/* Loop once most of the time as this is processed right after PHY raised the interrupt for a status written */
		byteAvail = StretchBufferGetReadAvail(pPhySbSta);
		while((byteAvail > 0) && (byteAvail <= (HostStretchBufferGetWriteAvail(pLocSbSta) - 12))) {
			wrPtr = HostStretchBufferGetWritePtr(pLocSbSta);
			rdPtr = StretchBufferGetReadPtr(pPhySbSta);
#ifdef FLATTEN_ADDR_ADJUST
			rdPtr = (uintptr_t)(ADSL_ADDR_TO_HOST( rdPtr));
#endif
			BlockLongMove((((byteAvail>>2)+3)&~3), (int *)rdPtr, (int *)wrPtr);
			StretchBufferReadUpdate(pPhySbSta, byteAvail);
			HostStretchBufferWriteUpdate(pLocSbSta, byteAvail);
			byteAvail = StretchBufferGetReadAvail(pPhySbSta);
		}
	}
}
#endif

static AC_BOOL AdsCoreStatBufInitialized(void *bufPtr)
{
	volatile int	cnt;

	if (bufPtr != (void *)StretchBufferGetStartPtr(pPhySbSta))
		return AC_FALSE;

	if (bufPtr != (void *)StretchBufferGetReadPtr(pPhySbSta))
		return AC_FALSE;

	cnt = 20;
	do {
	} while (--cnt != 0);

	return AC_TRUE;
}

#define AdsCoreStatBufAssigned()	((pPhySbSta->pExtraEnd != NULL) && (NULL != pPhySbSta->pEnd) && (NULL != pPhySbSta->pStart))
#define TmElapsedUs(cnt0)			BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cnt0)
#define OsTimeElapsedMs(osTime0)	BcmAdslCoreOsTimeElapsedMs(osTime0)

AC_BOOL AdslCoreSetStatusBuffer(void *bufPtr, int bufSize)
{
	dslCommandStruct	cmd;
	
	if (NULL == bufPtr)
		bufPtr = (void *) (adslCorePhyDesc.sdramImageAddr + adslCorePhyDesc.sdramImageSize);
	cmd.command = kDslSetStatusBufferCmd;
	cmd.param.dslStatusBufSpec.pBuf = bufPtr;
	cmd.param.dslStatusBufSpec.bufSize = bufSize;

	acBlockStatusRead = AC_TRUE;

	if (!AdslCoreCommandWrite(&cmd)) {
		acBlockStatusRead = AC_FALSE;
		return AC_FALSE;
	}

	do {
	} while (AdsCoreStatBufInitialized(bufPtr));

	acBlockStatusRead = AC_FALSE;
	return AC_TRUE;
}

/*
**
**		G.997 callback and interface functions
**
*/

#ifdef G997_1_FRAMER

#define AdslCoreOvhMsgSupported(gDslV)		XdslMibIsXdsl2Mod(gDslV)

#define	gG997ClEocCtrl(x)	(((dslVarsStruct *)(x))->clEocCtrl)
#define	gG997NsfCtrl(x)		(((dslVarsStruct *)(x))->nsfCtrl)
#define	gG997DatagramCtrl(x)	(((dslVarsStruct *)(x))->datagramCtrl)
#ifdef SUPPORT_HMI
#define	gG997HmiEocCtrl(x)	(((dslVarsStruct *)(x))->hmiEocCtrl)
#endif

#define	gG997RxFrCB(x)		((x)->g997RxFrCB)
#define	gG997RxFrBuf(x)		((x)->g997RxFrBuf)
#define	gG997RxCurBuf(x)	((x)->g997RxCurBuf)
#define	gG997TxFrBufPool(x)	((x)->g997TxFrBufPool)
#define	gG997TxFrList(x)	((x)->g997TxFrList)

Boolean AdslCoreCommonCommandHandler(void *gDslV, dslCommandStruct *cmdPtr)
{
	Boolean res;
#ifdef SUPPORT_DSL_BONDING
	cmdPtr->command |= (gLineId(gDslV) << DSL_LINE_SHIFT);
#endif
	{
	ulong	irqFlags;
	BcmCoreDpcSyncIntrDisableEnter(SYNC_TX, irqFlags);
	res = AdslCoreCommandWrite(cmdPtr);
	BcmCoreDpcSyncIntrDisableExit(SYNC_TX, irqFlags);
	}
#ifdef SUPPORT_DSL_BONDING
	cmdPtr->command &= (gLineId(gDslV) << DSL_LINE_SHIFT);
#endif
	return res;
}

void AdslCoreCommonStatusHandler(void *gDslV, dslStatusStruct *status)
{
	uchar	lineId = gLineId(gDslV);
		
	switch (DSL_STATUS_CODE(status->code)) {
		case kDslConnectInfoStatus:
			BcmAdslCoreDiagWriteStatusString(lineId, "AdslCoreCommonSH (ConnInfo): code=%d, val=%d", 
				status->param.dslConnectInfo.code, status->param.dslConnectInfo.value);
			if ((kG992p3PwrStateInfo == status->param.dslConnectInfo.code) && 
				(3 == status->param.dslConnectInfo.value)) {
				AdslCoreIndicateLinkPowerState(lineId, 3);
				BcmAdslCoreNotify(lineId, ACEV_LINK_POWER_L3);
			}	
			break;
		case kDslGetOemParameter:
		{
			void	*pOemData;
			int		maxLen, *pLen;

			pOemData = AdslCoreGetOemParameterData (status->param.dslOemParameter.paramId, &pLen, &maxLen);
			status->param.dslOemParameter.dataLen = ADSL_ENDIAN_CONV_UINT32(*pLen);
			status->param.dslOemParameter.dataPtr = pOemData;
			if (0 == status->param.dslOemParameter.dataLen)
				status->param.dslOemParameter.dataPtr = NULL;
			else if(kDslOemG994VendorId == status->param.dslOemParameter.paramId) {
				int	i;
				char	*pData = pOemData;
				__SoftDslPrintf(XdslCoreGetCurDslVars(),"AdslCoreGetOemParameter paramId:%d pLen:%d",0,
					kDslOemG994VendorId, status->param.dslOemParameter.dataLen);
				for(i=0;i<status->param.dslOemParameter.dataLen;i++)
					__SoftDslPrintf(XdslCoreGetCurDslVars(),"AdslCoreGetOemParameter pData[i]=%c",0,pData[i]);
			}
		}
			break;
		default:
			break;
	}
}

void AdslCoreStatusHandler(void *gDslV, dslStatusStruct *status)
{
	(*pAdslCoreStatusCallback) (status, NULL, 0);
}

int DslGetLineId(void *gDslVars)
{
	return gLineId(gDslVars);
}

#ifndef __SoftDslPrintf
void __SoftDslPrintf(void *gDslV, char *fmt, int argNum, ...)
{
	dslStatusStruct		status;
	va_list				ap;

	va_start(ap, argNum);
#ifdef SUPPORT_DSL_BONDING
	status.code = kDslPrintfStatus | (gLineId(gDslV) << DSL_LINE_SHIFT);
#else
	status.code = kDslPrintfStatus;
#endif
	status.param.dslPrintfMsg.fmt = fmt;
	status.param.dslPrintfMsg.argNum = 0;
	status.param.dslPrintfMsg.argPtr = (void *)&ap;
	va_end(ap);

	(*pAdslCoreStatusCallback) (&status, NULL, 0);
}
#endif
 
Boolean AdslCoreG997CommandHandler(void *gDslV, dslCommandStruct *cmdPtr)
{
	Boolean res;

	if ((kDslSendEocCommand == DSL_COMMAND_CODE(cmdPtr->command)) &&
		(kDslClearEocSendFrame == cmdPtr->param.dslClearEocMsg.msgId)) {
		if (AdslCoreIsPhySdramAddr(cmdPtr->param.dslClearEocMsg.dataPtr))
			cmdPtr->param.dslClearEocMsg.msgType &= ~kDslClearEocMsgDataVolatileMask;
		else 
			cmdPtr->param.dslClearEocMsg.msgType |= kDslClearEocMsgDataVolatileMask;
	}
#ifdef SUPPORT_DSL_BONDING
	cmdPtr->command |= (gLineId(gDslV) << DSL_LINE_SHIFT);
#endif
	{
	ulong	irqFlags;
	BcmCoreDpcSyncIntrDisableEnter(SYNC_TX, irqFlags);
	res = AdslCoreCommandWrite(cmdPtr);
	BcmCoreDpcSyncIntrDisableExit(SYNC_TX, irqFlags);
	}
#ifdef SUPPORT_DSL_BONDING
	cmdPtr->command &= (gLineId(gDslV) << DSL_LINE_SHIFT);
#endif
	return res;
}

void AdslCoreG997StatusHandler(void *gDslV, dslStatusStruct *status)
{
}

#ifdef G992P3_DEBUG
void AdslCorePrintDebugData(void *gDslVars)
{
	BcmAdslCoreDiagWriteStatusString(gLineId(gDslVars), "gDslVars=0x%X, gG997Vars=0x%X, gG992p3OvhMsgVars=0x%X, gAdslMibVars=0x%X", 
				gDslVars, &gG997Vars, &gG992p3OvhMsgVars, &gAdslMibVars);
}

void AdslCoreG997PrintFrame(void *gDslVars, char *hdr, dslFrame *pFrame)
{
	BcmAdslCoreWriteOvhMsg(gDslVars, hdr, pFrame);
}
#endif

static ac997FrameCtrl *XdslCoreG997GetEocMsgCtrl(void *gDslVars, int eocMsgType)
{
	ac997FrameCtrl *g997FrCtrl;
	
	if(BCM_XDSL_NSF_EOC_MSG == eocMsgType)
		g997FrCtrl = &gG997NsfCtrl(gDslVars);
	else if(BCM_XDSL_DATAGRAM_EOC_MSG == eocMsgType)
		g997FrCtrl = &gG997DatagramCtrl(gDslVars);
	else if(BCM_XDSL_CLEAR_EOC_MSG == eocMsgType)
		g997FrCtrl = &gG997ClEocCtrl(gDslVars);
	else
		g997FrCtrl = NULL;
	
	return g997FrCtrl;
}

static void XdslCoreG997InitEocMsgCtrl(ac997FrameCtrl *g997FrCtrl)
{
	uchar	*p;
	int	i;
	
	CircBufferInit (&g997FrCtrl->g997RxFrCB, g997FrCtrl->g997RxFrBuf, sizeof(g997FrCtrl->g997RxFrBuf));
	g997FrCtrl->g997RxCurBuf = NULL;
	g997FrCtrl->timeUpdate = 0;
	g997FrCtrl->pendingFrFlag = 0;

	g997FrCtrl->g997TxFrList = NULL;
	p = (void *) &g997FrCtrl->g997TxFrBufPool;
	for (i = 0; i < kG997MaxTxPendingFrames; i++) {
		ac997FramePoolItem *pG997Fr = (ac997FramePoolItem *)p;
		memset(pG997Fr->eocHdr, 0, sizeof(pG997Fr->eocHdr));
		BlankListAdd(&g997FrCtrl->g997TxFrList, (void*) p);
		p += sizeof(g997FrCtrl->g997TxFrBufPool[0]);
	}
}

void AdslCoreG997Init(uchar lineId)
{
	dslVarsStruct	*pDslVars = (dslVarsStruct	*)XdslCoreGetDslVars(lineId);
	XdslCoreG997InitEocMsgCtrl(&gG997NsfCtrl(pDslVars));
	XdslCoreG997InitEocMsgCtrl(&gG997ClEocCtrl(pDslVars));
	XdslCoreG997InitEocMsgCtrl(&gG997DatagramCtrl(pDslVars));
#ifdef SUPPORT_HMI
	XdslCoreG997InitEocMsgCtrl(&gG997HmiEocCtrl(pDslVars));
#endif
}

#ifdef SUPPORT_HMI
extern int FrameDataCopy(void *gDslVars, dslFrameBuffer *pBuf, int firstBufOffset, uint destLen, uchar *pDest);
#endif

int __AdslCoreG997SendComplete(void *gDslVars, void *userVc, ulong mid, dslFrame *pFrame)
{
	ac997FrameCtrl *g997FrCtrl;
	long acEvent;
	ac997FramePoolItem	*pFB = (ac997FramePoolItem	*)pFrame;
	char * pBuf = DslFrameBufferGetAddress(gDslVars, &pFB->frBuf);
	
#ifdef SUPPORT_HMI
	if(NULL != userVc) {
		OverheadClient *pOverHeadClient;
		ErrorType      errorType;
		dslFrame       *pRxFrame;
		dslFrameBuffer *pBuf;
		uchar          *pData;
		int            len;
		
		g997FrCtrl = &gG997HmiEocCtrl(gDslVars);
		pOverHeadClient = (OverheadClient *)(gG997RxFrBuf(g997FrCtrl)[0]);
		if(0 == mid) {
			pRxFrame = (dslFrame *)userVc;
			pBuf = DslFrameGetFirstBuffer(gDslVars, pRxFrame);
			len = DslFrameBufferGetLength(gDslVars, pBuf);
			pData = DslFrameBufferGetAddress(gDslVars, pBuf);
			if(pRxFrame->totalLength <= pOverHeadClient->responseBufferSize) {
				if(XdslMibIsGfastMod(gDslVars) && (pData[2] == 0x81)) {
					/* kG992p3OvhMsgCmdPMDRead command, remove SC byte */
					memcpy(pOverHeadClient->bufferPair.buffer2.data, pData, 4);
					len = FrameDataCopy(gDslVars, pBuf, 5, pRxFrame->totalLength-5, pOverHeadClient->bufferPair.buffer2.data+4);
					len += 4;
				}
				else
					len = FrameDataCopy(gDslVars, pBuf, 0, pRxFrame->totalLength, pOverHeadClient->bufferPair.buffer2.data);
				BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "__AdslCoreG997SendComplete: totalLength=%d cpyLen=%d", pRxFrame->totalLength, len);
				pOverHeadClient->bufferPair.buffer2.length = len;
				errorType = NO_ERROR;
			}
			else
				errorType = TIMEOUT;
		}
		else
			errorType = TIMEOUT;
		BlankListAdd(&gG997TxFrList(g997FrCtrl), pFrame);
		(*pOverHeadClient->messageReceived)(pOverHeadClient, errorType);
		return 1;
	}
#endif
	if( NULL != pBuf )
		free(pBuf);
	/* 2 - kG992p3CmdCode */
	if(BCM_XDSL_CLEAR_EOC_MSG == pFB->eocHdr[2]) {
		g997FrCtrl = &gG997ClEocCtrl(gDslVars);
		acEvent = ACEV_G997_FRAME_SENT;
	}
	else if(BCM_XDSL_NSF_EOC_MSG == pFB->eocHdr[2]) {
		g997FrCtrl = &gG997NsfCtrl(gDslVars);
		acEvent = ACEV_G997_NSF_FRAME_SENT;
	}
	else if(BCM_XDSL_DATAGRAM_EOC_MSG == pFB->eocHdr[2]) {
		g997FrCtrl = &gG997DatagramCtrl(gDslVars);
		acEvent = ACEV_G997_DATAGRAM_FRAME_SENT;
	}
	else {
		BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "__AdslCoreG997SendComplete: Unkown frame sent pFrame=0x%08X\n", (uint)(ulong)pFrame);
		return 1;
	}
#if 0 && defined(G992P3_DEBUG)
	BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "__AdslCoreG997SendComplete: acEvent=0x%08X pFrame=0x%08X\n", (uint)acEvent, (uint)(ulong)pFrame);
#endif
	BlankListAdd(&gG997TxFrList(g997FrCtrl), pFrame);
	BcmAdslCoreNotify(gLineId(gDslVars), acEvent);
	
	return 1;
}

int AdslCoreG997SendComplete(void *gDslVars, void *userVc, ulong mid, dslFrame *pFrame)
{
#ifdef G992P3
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2) && (kG992p3OvhMsgFrameBufCnt == pFrame->bufCnt)) {
		G992p3OvhMsgSendCompleteFrame(gDslVars, userVc, mid, pFrame);
#if 0 && defined(G992P3_DEBUG)
		BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "AdslCoreG997SendComplete: Frame sent (from G992p3OvhMsg) pFrame=0x%08X\n", (uint)(ulong)pFrame);
#endif
		return 1;
	}
#endif
#if 0 && defined(G992P3_DEBUG)
	BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "AdslCoreG997SendComplete: Frame sent (NOT from G992p3OvhMsg) gDslVars = 0x%08X, pFrame = 0x%08X, pBuf = 0x%08X, bufCnt = %d\n",
		(uint)(ulong)gDslVars, (uint)(ulong)pFrame, (uint)(ulong)DslFrameGetFirstBuffer(gDslVars, pFrame), (int)pFrame->bufCnt);
#endif
	return __AdslCoreG997SendComplete(gDslVars, userVc, mid, pFrame);
}

#ifdef G992P3_DEBUG
int TstG997SendFrame (void *gDslV, void *userVc, ulong mid, dslFrame *pFrame)
{
#if defined(SUPPORT_EXT_DSL_BONDING_MASTER)
	if(0 != gLineId(gDslV))
		return 1;
#endif
	AdslCoreG997PrintFrame(gDslV, "TX", pFrame);
	return G997SendFrame(gDslV, userVc, mid, pFrame);
}
#endif

extern int BcmXdslIsEocIntfOpen(unsigned lineId, int eocMsgType);

int XdslCoreIsEocIntfOpen(void *gDslVars, int eocMsgType)
{
	return BcmXdslIsEocIntfOpen(gLineId(gDslVars), eocMsgType);
}

int AdslCoreG997IndicateRecevice (void *gDslVars, void *userVc, ulong mid, dslFrame *pFrame)
{
	void	**p;
	int res = -1;
#ifdef G992P3
#ifdef G992P3_DEBUG
	AdslCoreG997PrintFrame(gDslVars, "RX", pFrame);
#endif
	if (AdslCoreOvhMsgSupported(gDslVars)) {
		res = G992p3OvhMsgIndicateRcvFrame(gDslVars, NULL, 0, pFrame);
		if (res) {
			if( 1 == res )
				G997ReturnFrame(gDslVars, NULL, 0, pFrame);
			return 1;
		}
	}
	else
		*(((ulong*) DslFrameGetLinkFieldAddress(gDslVars,pFrame)) + 2) = 0;
#endif
	if((0 == res) && AdslCoreOvhMsgSupported(gDslVars)) {
		int len;
		uchar *pData;
		long acEvent;
		dslFrameBuffer	*pBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
		
		if(NULL == pBuf) {
			G997ReturnFrame(gDslVars, NULL, 0, pFrame);
			return 1;
		}
		else {
			ac997FrameCtrl *g997FrCtrl;
			len = DslFrameBufferGetLength(gDslVars, pBuf);
			pData = DslFrameBufferGetAddress(gDslVars, pBuf);
			/* 2 - kG992p3CmdCode, 3 - kG992p3CmdSubCode */
			if(((BCM_XDSL_NSF_EOC_MSG == pData[2]) ||
				(BCM_XDSL_DATAGRAM_EOC_MSG == pData[2]) ||
				(BCM_XDSL_CLEAR_EOC_MSG == pData[2])) && (1 == pData[3])) {
				
				if(BCM_XDSL_NSF_EOC_MSG == pData[2]) {
					acEvent = ACEV_G997_NSF_FRAME_RCV;
					g997FrCtrl = &gG997NsfCtrl(gDslVars);
				}
				else if(BCM_XDSL_DATAGRAM_EOC_MSG == pData[2]) {
					acEvent = ACEV_G997_DATAGRAM_FRAME_RCV;
					g997FrCtrl = &gG997DatagramCtrl(gDslVars);
				}
				else {
					acEvent = ACEV_G997_FRAME_RCV;
					g997FrCtrl = &gG997ClEocCtrl(gDslVars);
				}
				
				if( XdslCoreIsEocIntfOpen(gDslVars, pData[2]) ) {
					DslFrameBufferSetLength(gDslVars, pBuf, len-4);
					DslFrameBufferSetAddress(gDslVars, pBuf, pData+4);
					*(((ulong*) DslFrameGetLinkFieldAddress(gDslVars,pFrame)) + 2) = 4;
				}
				else {
					BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "AdslCoreG997IndicateRecevice: Discarding frame, eocIntf(0x%02X) is not opened", pData[2]);
					G997ReturnFrame(gDslVars, NULL, 0, pFrame);
					return 1;
				}
			}
			else {
				BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "AdslCoreG997IndicateRecevice: Unknown frame received pData[2]=0x%02X", pData[2]);
				G997ReturnFrame(gDslVars, NULL, 0, pFrame);
				return 1;
			}
			if (CircBufferGetWriteAvail(&gG997RxFrCB(g997FrCtrl)) > 0) {
				if( gPendingFrFlag(g997FrCtrl) == 0 ) {
					gPendingFrFlag(g997FrCtrl) = 1;
					gTimeUpdate(g997FrCtrl) = 0;
				}
				p = CircBufferGetWritePtr(&gG997RxFrCB(g997FrCtrl));
				*p = pFrame;
				CircBufferWriteUpdate(&gG997RxFrCB(g997FrCtrl), sizeof(void *));
				BcmAdslCoreNotify(gLineId(gDslVars), acEvent);
			}
			else {
				BcmAdslCoreDiagWriteStatusString (gLineId(gDslVars), "AdslCoreG997IndicateRecevice:Frame received(0x%08X) but cannot be read as buffer is full", acEvent);
				G992p3OvhMsgReturnFrame(gDslVars, NULL, 0, pFrame);
				G997ReturnFrame(gDslVars, NULL, 0, pFrame);
			}
		}
	}
	
	return 1;
}

void AdslCoreSetL3(uchar lineId)
{
	void *pDslVars = XdslCoreGetDslVars(lineId);

	if (!AdslCoreLinkState(lineId)) {
		BcmCoreDpcSyncEnter(SYNC_RX);
		AdslCoreIndicateLinkPowerState(lineId, 3);
		BcmAdslCoreNotify(lineId, ACEV_LINK_POWER_L3);
		BcmCoreDpcSyncExit(SYNC_RX);
	}
	if (AdslCoreOvhMsgSupported(pDslVars))
		G992p3OvhMsgSetL3(pDslVars);
}

void AdslCoreSetL0(uchar lineId)
{
	void *pDslVars = XdslCoreGetDslVars(lineId);
	
	if (AdslCoreOvhMsgSupported(pDslVars))
		G992p3OvhMsgSetL0(pDslVars);
}

AC_BOOL AdslCoreG997ReturnFrame (void *pDslVars, ac997FrameCtrl *g997FrCtrl)
{
	dslFrame **p;
	AC_BOOL	 res = AC_FALSE;

	if (CircBufferGetReadAvail(&gG997RxFrCB(g997FrCtrl)) > 0) {
		p = CircBufferGetReadPtr(&gG997RxFrCB(g997FrCtrl));
#ifdef G992P3
		G992p3OvhMsgReturnFrame(pDslVars, NULL, 0, *p);
#endif
		res = G997ReturnFrame(pDslVars, NULL, 0, *p);
		CircBufferReadUpdate(&gG997RxFrCB(g997FrCtrl), sizeof(void *));
	}
	return res;
}

#ifdef SUPPORT_HMI

int XdslCoreG997SendHmiEocData(unsigned char lineId, OverheadClient* pOverHeadClient)
{
	ac997FrameCtrl *g997FrCtrl;
	ac997FramePoolItem	*pFB;
	int	 res = 0;
	void *gDslVars = XdslCoreGetDslVars(lineId);
	
	if(NULL == (g997FrCtrl = &gG997HmiEocCtrl(gDslVars)))
		return res;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	pFB = BlankListGet(&gG997TxFrList(g997FrCtrl));
	if (NULL == pFB) {
		BcmCoreDpcSyncExit(SYNC_RX);
		return res;
	}

	DslFrameInit (gDslVars, &pFB->fr);
	DslFrameBufferSetAddress (gDslVars, &pFB->frBuf, pOverHeadClient->bufferPair.buffer1.data);
	DslFrameBufferSetLength (gDslVars, &pFB->frBuf, pOverHeadClient->bufferPair.buffer1.length);
	DslFrameEnqueBufferAtBack (gDslVars, &pFB->fr, &pFB->frBuf);
	res = G992p3OvhMsgSendHmiEocFrame(gDslVars, &pFB->fr);
	
	if( 0 == res )
		BlankListAdd(&gG997TxFrList(g997FrCtrl), &pFB->fr);
	else
		gG997RxFrBuf(g997FrCtrl)[0] = (void *)pOverHeadClient;
	
	BcmCoreDpcSyncExit(SYNC_RX);
	
	return res;
}

#endif

AC_BOOL AdslCoreG997SendData(unsigned char lineId, int eocMsgType, void *buf, int len)
{
	ac997FrameCtrl *g997FrCtrl;
	ac997FramePoolItem	*pFB;
	AC_BOOL	 res = AC_FALSE;
	void *gDslVars = XdslCoreGetDslVars(lineId);
	
	if(NULL == (g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, eocMsgType)))
		return res;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	pFB = BlankListGet(&gG997TxFrList(g997FrCtrl));
	if (NULL == pFB) {
		BcmCoreDpcSyncExit(SYNC_RX);
		return res;
	}

	DslFrameInit (gDslVars, &pFB->fr);
	DslFrameBufferSetAddress (gDslVars, &pFB->frBuf, buf);
	DslFrameBufferSetLength (gDslVars, &pFB->frBuf, len);
#ifdef G992P3
	if (AdslCoreOvhMsgSupported(gDslVars)) {
		DslFrameBufferSetAddress (gDslVars, &pFB->frBufHdr, pFB->eocHdr);
		DslFrameBufferSetLength (gDslVars, &pFB->frBufHdr, 4);
		DslFrameEnqueBufferAtBack (gDslVars, &pFB->fr, &pFB->frBufHdr);
		DslFrameEnqueBufferAtBack (gDslVars, &pFB->fr, &pFB->frBuf);
#if 0 && defined(G992P3_DEBUG)
		__SoftDslPrintf(gDslVars, "AdslCoreG997SendData: pFrame=0x%X frameLen=%d frBufHdr=0x%X frBuf=0x%X", 0,
			(uint)&pFB->fr, DslFrameGetLength(gDslVars, &pFB->fr), (uint)&pFB->frBufHdr, (uint)&pFB->frBuf);
#endif
		if(BCM_XDSL_NSF_EOC_MSG == eocMsgType)
			res = G992p3OvhMsgSendNonStdFacFrame(gDslVars, &pFB->fr);
		else if(BCM_XDSL_DATAGRAM_EOC_MSG == eocMsgType)
			res = G992p3OvhMsgSendDatagramEocFrame(gDslVars, &pFB->fr);
		else if(BCM_XDSL_CLEAR_EOC_MSG == eocMsgType)
			res = G992p3OvhMsgSendClearEocFrame(gDslVars, &pFB->fr);
		else {
			BlankListAdd(&gG997TxFrList(g997FrCtrl), &pFB->fr);
			BcmCoreDpcSyncExit(SYNC_RX);
			return res;
		}
	}
	else
#endif
	{
		DslFrameEnqueBufferAtBack (gDslVars, &pFB->fr, &pFB->frBuf);
		AdslCoreG997PrintFrame(gDslVars,"TX",&pFB->fr);
		res = G997SendFrame(gDslVars, NULL, 0, &pFB->fr);
	}

	if( AC_FALSE == res )
		BlankListAdd(&gG997TxFrList(g997FrCtrl), &pFB->fr);
	
	BcmCoreDpcSyncExit(SYNC_RX);
	return res;
}


int AdslCoreG997FrameReceived(unsigned char lineId, int eocMsgType)
{
	ac997FrameCtrl *g997FrCtrl;
	void *gDslVars = XdslCoreGetDslVars(lineId);
	if(NULL == (g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, eocMsgType)))
		return 0;
	return(CircBufferGetReadAvail(&gG997RxFrCB(g997FrCtrl)) > 0);
}

void *AdslCoreG997BufGet(unsigned char lineId, dslFrameBuffer *pBuf, int *pLen)
{
	void *gDslVars = XdslCoreGetDslVars(lineId);

	if (NULL == pBuf)
		return NULL;

	*pLen = DslFrameBufferGetLength(gDslVars, pBuf);
	return DslFrameBufferGetAddress(gDslVars, pBuf);
}

void *AdslCoreG997FrameGet(unsigned char lineId, int eocMsgType, int *pLen)
{
	ac997FrameCtrl *g997FrCtrl;
	dslFrame *pFrame;
	void *gDslVars = XdslCoreGetDslVars(lineId);
	
	if(NULL == (g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, eocMsgType)))
		return NULL;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	if (CircBufferGetReadAvail(&gG997RxFrCB(g997FrCtrl)) == 0) {
		BcmCoreDpcSyncExit(SYNC_RX);
		return NULL;
	}
	pFrame = *(dslFrame **)CircBufferGetReadPtr(&gG997RxFrCB(g997FrCtrl));
	gG997RxCurBuf(g997FrCtrl) = DslFrameGetFirstBuffer(gDslVars, pFrame);
	gPendingFrFlag(g997FrCtrl) = 0;
	gTimeUpdate(g997FrCtrl) = 0;
	BcmCoreDpcSyncExit(SYNC_RX);
	
	return AdslCoreG997BufGet(lineId, gG997RxCurBuf(g997FrCtrl), pLen);
}

void *AdslCoreG997FrameGetNext(unsigned char lineId, int eocMsgType, int *pLen)
{
	ac997FrameCtrl *g997FrCtrl;
	void *gDslVars = XdslCoreGetDslVars(lineId);
	
	if(NULL == (g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, eocMsgType)))
		return NULL;

	if (NULL == gG997RxCurBuf(g997FrCtrl))
		return NULL;

	gG997RxCurBuf(g997FrCtrl) = DslFrameGetNextBuffer(gDslVars, gG997RxCurBuf(g997FrCtrl));
	return AdslCoreG997BufGet(lineId, gG997RxCurBuf(g997FrCtrl), pLen);
}

void AdslCoreG997FrameFinished(unsigned char lineId, int eocMsgType)
{
	ac997FrameCtrl *g997FrCtrl;
	void *pDslVars = XdslCoreGetDslVars(lineId);
	
	if(NULL == (g997FrCtrl = XdslCoreG997GetEocMsgCtrl(pDslVars, eocMsgType)))
		return;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	AdslCoreG997ReturnFrame (pDslVars, g997FrCtrl);
	BcmCoreDpcSyncExit(SYNC_RX);
}


#endif /* G997_1_FRAMER */

/*
**
**		ADSL MIB functions
**
*/

#ifdef ADSL_MIB

int AdslCoreMibNotify(void *gDslV, uint event)
{
	uchar lineId = gLineId(gDslV);

	BcmCoreDpcSyncExit(SYNC_RX);
	if (event & kAdslEventRateChange){
		BcmXdslNotifyRateChange(lineId);
		G992p3OvhMsgSetRateChangeFlag(gDslV);
	}
	else if (event & kXdslEventContSESThresh) {
		if ((XdslMibIsGfastMod(gDslV)) && ADSL_PHY_SUPPORT(kAdslPhyGfastRetrainCmd))
			BcmXdslCoreSendCmd(lineId, kDslGfastRetrainCmd, 0);
		else {
			DiagWriteString(lineId, DIAG_DSL_CLIENT, "AdslCoreMibNotify: Re-init PHY");
			BcmAdslCoreConnectionStart(lineId);
		}
	}
	else if (event & kXdslEventFastRetrain) {
		AdslDrvPrintf(TEXT("Line %d: fast retrain started\n"), lineId);
	}
#ifdef SUPPORT_SELT
	else if (event & kXdslEventSeltNext) {
		BcmAdslCoreSetSeltNextMode(lineId);
	}
#endif
	else if (event & kAdslEventLinkChange) {
		BcmCoreDpcSyncEnter(SYNC_RX);
		BcmAdslCoreNotify (lineId, AdslCoreLinkState(lineId) ? ACEV_LINK_UP : ACEV_LINK_DOWN);
		BcmCoreDpcSyncExit(SYNC_RX);
	}
	BcmCoreDpcSyncEnter(SYNC_RX);
	
	return 0;
}
#endif

/*
**
**		Interface functions
**
*/

void setParam (uint *addr, uint val)
{
	*addr = val;
}

uint getParam (uint *addr)
{
	return *addr;
} 

#if 0 && defined(CONFIG_BCM963x8)

#define AFE_REG_BASE		0xFFF58000
#define	AFE_REG_DATA		(AFE_REG_BASE + 0xC)
#define	AFE_REG_CTRL		(AFE_REG_BASE + 0x8)

void writeAFE(uint reg, uint val)
{
	uint	cycleCnt0;

	setParam((uint *) AFE_REG_DATA, val & 0xff);
	/* need to wait 16 usecs here */
	cycleCnt0 = BcmAdslCoreGetCycleCount();
	while (TmElapsedUs(cycleCnt0) < 16)	;
	setParam((uint *) AFE_REG_CTRL, (reg << 8) | 0x05);
	while (getParam((uint *) AFE_REG_CTRL) & 0x01) ;
	/* need to wait 16 usecs here */
	cycleCnt0 = BcmAdslCoreGetCycleCount();
	while (TmElapsedUs(cycleCnt0) < 16)	;
	setParam((uint *) AFE_REG_CTRL, (reg << 8) | 0x01);
	while (getParam((uint *) AFE_REG_CTRL) & 0x01) ;
}

uint readAFE(uint reg)
{
	setParam((uint *) AFE_REG_CTRL, (reg << 8) | 0x01);
	while (getParam((uint *) AFE_REG_CTRL) & 0x01) ;
	return getParam((uint *) AFE_REG_DATA);
}

#endif /* 0 && CONFIG_BCM963x8 */

static __inline void writeAdslEnum(int offset, int value)
{
    volatile unsigned int *penum = ((unsigned int *)XDSL_ENUM_BASE);
    penum[offset] = value;
    return;
}

#define RESET_STALL \
    do { int _stall_count = 20; while (_stall_count--) ; } while (0)

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM96318)

#define RESET_ADSL_CORE \
	pXdslControlReg[0] &= ~(XDSL_PHY_RESET | XDSL_MIPS_POR_RESET | XDSL_ANALOG_RESET); \
	RESET_STALL; \
	pXdslControlReg[0] |= XDSL_PHY_RESET | XDSL_MIPS_POR_RESET | XDSL_ANALOG_RESET

#define ENABLE_ADSL_CORE \
	pXdslControlReg[0] |= XDSL_PHY_RESET | XDSL_MIPS_POR_RESET | XDSL_ANALOG_RESET

#define DISABLE_ADSL_CORE \
	pXdslControlReg[0] &= ~(XDSL_PHY_RESET | XDSL_MIPS_POR_RESET | XDSL_ANALOG_RESET)

#define ENABLE_ADSL_MIPS \
	pXdslControlReg[0] |= XDSL_PHY_RESET | \
						XDSL_MIPS_RESET | \
						XDSL_MIPS_POR_RESET | \
						XDSL_ANALOG_RESET

#define DISABLE_ADSL_MIPS \
	pXdslControlReg[0] &= ~XDSL_MIPS_RESET
	
#elif defined(USE_PMC_API)

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)	// TO DO: Tony - Double check for 63146
#define BG_BIAS0_OFFSET (0xf000/4)
#define AFE_REG0_OFFSET (0xf094/4)
#elif defined(CONFIG_BCM963178)
#define BG_BIAS0_OFFSET (0x7350/4)
#define AFE_REG0_OFFSET (0x7300/4)
#endif

#ifdef _NOOS
extern int start_MIPS(unsigned int core_id);
extern int stop_MIPS(unsigned int core_id);
extern int reset_DSLCORE(unsigned int core_id);
extern AC_BOOL load_MIPS_image(unsigned int core_id, unsigned long src, unsigned int len, unsigned long dst);
#define RESET_ADSL_CORE		reset_DSLCORE(get_cpuid())
#define ENABLE_ADSL_CORE
#define DISABLE_ADSL_CORE
#define ENABLE_ADSL_MIPS		start_MIPS(get_cpuid())
#define DISABLE_ADSL_MIPS		stop_MIPS(get_cpuid())
#else
#if defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 9)

#if defined(CONFIG_BRCM_IKOS)
int kerSysGetResetStatus(void)
{
	return 0;
}
#endif

static int XdslCoreReset(void)
{
	int ret;
	
	kerSysDisableDyingGaspInterrupt();
	ret = pmc_dsl_core_reset();
#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 12)
	{
	uint afe_reg0, bg_bias0;
	volatile uint *pPhyRegAddr = (volatile uint *)DSLPHY_BASE;
	kerSysGetDyingGaspConfig(&afe_reg0, &bg_bias0);
#if defined(CONFIG_BCM963178)
	afe_reg0 &= ~0x8;
#endif
	pPhyRegAddr[BG_BIAS0_OFFSET] = bg_bias0;
	pPhyRegAddr[AFE_REG0_OFFSET] = afe_reg0;
	}
#endif
	kerSysEnableDyingGaspInterrupt();

	return ret;
}
#define RESET_ADSL_CORE		XdslCoreReset()
#else
#define RESET_ADSL_CORE		pmc_dsl_core_reset()
#endif /* defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 9) */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
#define ENABLE_ADSL_CORE		pmc_dsl_clock_set(1)
#define DISABLE_ADSL_CORE		pmc_dsl_clock_set(0)
#else
#define ENABLE_ADSL_CORE
#define DISABLE_ADSL_CORE
#endif /* defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) */
#define ENABLE_ADSL_MIPS		pmc_dsl_mips_enable(1)
#ifndef CONFIG_BCM963138
#define DISABLE_ADSL_MIPS		pmc_dsl_mips_enable(0)
#else
static int XdslCoreDisableMipsCore(void)
{
	int ret;
	
	ret = pmc_dsl_mips_enable(0);
#if defined(BOARD_H_API_VER) &&  (BOARD_H_API_VER > 8)
	if(0xb0 == (PERF->RevID & REV_ID_MASK))
		ret |= pmc_dsl_mipscore_enable(0, 1);
#endif
	return ret;
}
#define DISABLE_ADSL_MIPS		XdslCoreDisableMipsCore()
#endif	/* !CONFIG_BCM963138 */
#endif	/* _NOOS */

#else /* !USE_PMC_API */

#define RESET_ADSL_CORE \
    writeAdslEnum(ADSL_CORE_RESET, 0x1); \
    RESET_STALL;						 \
    writeAdslEnum(ADSL_CORE_RESET, 0x0); \
    RESET_STALL

#define ENABLE_ADSL_CORE \
    writeAdslEnum(ADSL_CORE_RESET, 0x0); \
    RESET_STALL
    
#define DISABLE_ADSL_CORE \
    writeAdslEnum(ADSL_CORE_RESET, 0x1)

#define ENABLE_ADSL_MIPS \
    writeAdslEnum(ADSL_MIPS_RESET, 0x2)
    
#define DISABLE_ADSL_MIPS \
    writeAdslEnum(ADSL_MIPS_RESET, 0x3)

#endif

#ifdef CONFIG_BCM_DSL_GFASTCOMBO

#if defined(SUPPORT_ANNEXAB_COMBO)
char * const pPhyImageName[2] = {"/etc/adsl/adsl_phy.bin", "/etc/adsl/adsl_phy0.bin"};	/* 1 - AnnexB PHY */
static int curPhyImageIndex =0;
#define	PHY_IMAGE_NAME	pPhyImageName[curPhyImageIndex]
#else
#define	PHY_IMAGE_NAME	"/etc/adsl/adsl_phy.bin"
#endif

#ifdef SUPPORT_MULTI_PHY
extern adslCfgProfile adslCoreCfgProfile[];
#endif

#elif defined(SUPPORT_DSL_BONDING)

#ifdef SUPPORT_MULTI_PHY

#ifdef CONFIG_BCM963268
#if defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0)
char * const pPhyImageName[3] = {"/etc/adsl/adsl_phy.bin", "/etc/adsl/adsl_phy0.bin", "/etc/adsl/adsl_phy1.bin"};/* 0-bonding5B; 1-bonding4B, 2-non bonding PHY */
#else
char * const pPhyImageName[2] = {"/etc/adsl/adsl_phy.bin", "/etc/adsl/adsl_phy0.bin"};	/* 0-bonding5B/4B; 1-non bonding PHY */
#endif
static int curPhyImageIndex =0;
#define	PHY_IMAGE_NAME	pPhyImageName[curPhyImageIndex]
#else	/* 63138/63148 */
#if defined(SUPPORT_ANNEXAB_COMBO) || defined(CONFIG_BCM_DSL_GFAST)
char * const pPhyImageName[2] = {"/etc/adsl/adsl_phy.bin", "/etc/adsl/adsl_phy0.bin"};	/* 1 - G.fast PHY */
static int curPhyImageIndex =0;
#define	PHY_IMAGE_NAME	pPhyImageName[curPhyImageIndex]
#else
#define	PHY_IMAGE_NAME	"/etc/adsl/adsl_phy.bin"
#endif
#endif /* CONFIG_BCM963268 */
extern adslCfgProfile adslCoreCfgProfile[];

#else /* !SUPPORT_MULTI_PHY */

#if defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0)
char * const pPhyImageName[2] = {"/etc/adsl/adsl_phy.bin", "/etc/adsl/adsl_phy0.bin"};	/* 0-bonding5B; 1-bonding4B */
static int curPhyImageIndex =0;
#define	PHY_IMAGE_NAME	pPhyImageName[curPhyImageIndex]
#elif defined(CONFIG_BCM_DSL_GFAST)
char * const pPhyImageName[2] = {"/etc/adsl/adsl_phy.bin", "/etc/adsl/adsl_phy0.bin"};	/* 1 - G.fast PHY */
static int curPhyImageIndex =0;
#define	PHY_IMAGE_NAME	pPhyImageName[curPhyImageIndex]
#else
#define	PHY_IMAGE_NAME	"/etc/adsl/adsl_phy.bin"
#endif
#endif /* SUPPORT_MULTI_PHY */

#elif defined(SUPPORT_ANNEXAB_COMBO) || defined(CONFIG_BCM_DSL_GFAST)	/* !SUPPORT_DSL_BONDING */

char * const pPhyImageName[2] = {"/etc/adsl/adsl_phy.bin", "/etc/adsl/adsl_phy0.bin"};	/* 1 - AnnexB/G.fast PHY */
static int curPhyImageIndex =0;
#define	PHY_IMAGE_NAME	pPhyImageName[curPhyImageIndex]

#else /* !SUPPORT_DSL_BONDING && !SUPPORT_ANNEXAB_COMBO && !CONFIG_BCM_DSL_GFAST */

#define	PHY_IMAGE_NAME	"/etc/adsl/adsl_phy.bin"

#endif /* CONFIG_BCM_DSL_GFASTCOMBO */

#ifdef SUPPORT_MULTI_PHY
void XdslCoreProcessPrivateSysMediaCfg(uint mediaSrchCfg)
{
	char str[80];
	int newVal = (mediaSrchCfg & MS_USE_IMG_MSK) >> MS_USE_IMG_SHIFT;
	
	sprintf(str, "XdslMediaSearch: mediaSrchCfg(0x%08x), XTM commands %s PHY\n",
		(uint)mediaSrchCfg, (BCM_IMAGETYPE_SINGLELINE == newVal)? "Single Line": "Bonding");
	DiagWriteString(0, DIAG_DSL_CLIENT, str);
	AdslDrvPrintf(TEXT("%s"), str);
#ifdef CONFIG_BCM963268

	if( ADSL_PHY_SUPPORT(kAdslPhyBonding) && (BCM_IMAGETYPE_SINGLELINE == newVal) && !(adslCoreCfgProfile[0].xdslMiscCfgParam & BCM_SWITCHPHY_DISABLED) ) {
		if(!(adslCoreCfgProfile[0].xdslMiscCfgParam & BCM_PREFERREDTYPE_FOUND)) {
			if((unsigned char)-1 != BcmXdslCoreSelectAndReturnPreferredMedia()) {
				XdslCoreSetPhyImageType(newVal);
				BcmXdslCoreMediaSearchReStartPhy();
			}
			else {
				sprintf(str, "XdslMediaSearch: Failed to select preferred media!\n");
				DiagWriteString(0, DIAG_DSL_CLIENT, str);
				AdslDrvPrintf(TEXT("%s"), str);
			}
		}
	}
	else
	if( TRUE == BcmXdslCoreProcessMediaSearchCfgCmd(mediaSrchCfg, AC_TRUE) )
		BcmXdslCoreMediaSearchReStartPhy();
	
#else
	if (!(adslCoreCfgProfile[0].xdslMiscCfgParam & BCM_SWITCHPHY_DISABLED)) {
		if(BCM_IMAGETYPE_SINGLELINE == newVal) {
			unsigned char lineId = BcmXdslCoreSelectAndReturnPreferredMedia();
			if((unsigned char)-1 != lineId) {
				int  lineReset = 0;  /* 0 - suspend another line; 1 - reset this line */
#ifndef XTM_SUPPORT_PORT1_NONBONDCONN
				if (0 != lineId) {
					adslMibInfo	*pMibInfo;
					long	mibLen;
					mibLen = sizeof(adslMibInfo);
					pMibInfo = (void *) AdslCoreGetObjectValue (lineId, NULL, 0, NULL, &mibLen);
					if (!(AdslCoreLinkState(lineId) && (0 == pMibInfo->xdslInfo.dirInfo[TX_DIRECTION].lpInfo[0].ahifChanId[0])))
						lineReset = 1;
				}
#endif
				if ( (adslCoreCfgProfile[0].xdslCfg2Mask & adslCoreCfgProfile[0].xdslCfg2Value & kPhyCfg2PreferBondingOverPhyR) 
					 && (0 == acLineReset[lineId ^ 1]) )
					lineReset = 1;

				if (0 == lineReset) {
					if (BcmXdslCoreGetLineActive(lineId ^ 1))
						BcmXdslCoreSendCmd(lineId ^ 1, kDslDownCmd, kDslIdleSuspend);
				}
				else {
					sprintf(str, "XdslMediaSearch: Line%d reset with another line down\n", lineId);
					DiagWriteString(0, DIAG_DSL_CLIENT, str);
					AdslDrvPrintf(TEXT("%s"), str);
					BcmXdslCoreSendCmd(lineId, kDslTestCmd, 0);
					BcmXdslCoreSendCmd(lineId ^ 1, kDslDownCmd, 0);
					BcmXdslCoreSendCmd(lineId, kDslSetLineResumeDelay, 120); /* resume timeout in seconds */
				}
			}
			else {
				sprintf(str, "XdslMediaSearch: Failed to select preferred line!\n");
				DiagWriteString(0, DIAG_DSL_CLIENT, str);
				AdslDrvPrintf(TEXT("%s"), str);
			}
		}
		else
			BcmXdslCoreBringUpLineInDownState();
	}
#endif
}

#endif	/* SUPPORT_MULTI_PHY */

#if defined(CONFIG_BCM_DSL_GFAST)

#ifndef CONFIG_BCM_DSL_GFASTCOMBO
int XdslCoreGetPhyImageType(void)
{
	return curPhyImageIndex;	/* 1 - Gfast PHY */
}

void XdslCoreSetPhyImageType(int imageType)
{
	if(imageType > 1)
		return;
	curPhyImageIndex = imageType;
}
#endif

#elif defined(CONFIG_BCM963268)

#if defined(SUPPORT_MULTI_PHY) || (defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0))
void XdslCoreSetPhyImageType(int imageType)
{
#if defined(CONFIG_BCM963268) && defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0)
	int chipRevId = (PERF->RevID & REV_ID_MASK);
#endif

	if(imageType > 1)
		return;
	
#if defined(CONFIG_BCM963268) && defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0)
	if(1 == imageType)
		curPhyImageIndex = 2;	/* Single line PHY */
	else if( chipRevId <= 0xC0 )
		curPhyImageIndex = 1;	/* 4B bonding PHY */
	else
		curPhyImageIndex = 0;	/* 5B bonding PHY */
#else
	if(1 == imageType)
		curPhyImageIndex = 1;	/* Single line PHY */
	else
		curPhyImageIndex = 0;	/* 5B/4B bonding PHY */
#endif
}

#if defined(SUPPORT_MULTI_PHY)
extern void BcmXdslCorePrintCurrentMedia(void);

int XdslCoreGetPhyImageType(void)
{
	int imageType;
#if defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0)
	if (2 == curPhyImageIndex)
		imageType = 1; /* Single line PHY */
	else
		imageType = 0; /* Bonding PHY */
#else
	if (1 == curPhyImageIndex)
		imageType = 1; /* Single line PHY */
	else
		imageType = 0; /* Bonding PHY */
#endif
	return imageType;
}
#endif	/* SUPPORT_MULTI_PHY */
#endif /* defined(SUPPORT_MULTI_PHY) || (defined(SUPPORT_DSL_BONDING5B) && defined(SUPPORT_DSL_BONDING_C0)) */

#endif	/* CONFIG_BCM963268 */

#if defined(SUPPORT_ANNEXAB_COMBO)
int IsAnnexBBoard(void)
{
    unsigned int annex;
    unsigned int afeIds[2];
    BcmXdslCoreGetAfeBoardId(&afeIds[0]);
    annex = ((afeIds[0] & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT);
    return (((AFE_FE_ANNEXB == annex) || (AFE_FE_ANNEXJ == annex) ||(AFE_FE_ANNEXBJ == annex)));
}
#endif

#ifdef SUPPORT_PHY_BIN_FROM_TMP
char loadImageFromTmp = 0;
#endif

#ifdef SUPPORT_PHY_BIN_FROM_SDRAM
int loadImageFromSdram = 0;
char phyBinAddr[0x200000];

int AdslFileInSdramLoadImage(void *pAdslLMem, void *pAdslSDRAM);

int AdslFileInSdramLoadImage(void *pAdslLMem, void *pAdslSDRAM)
{
	adslPhyImageHdr	phyHdr;
	char	* pFileInMem = &phyBinAddr[0];
	
	memcpy((void *)&phyHdr, pFileInMem, sizeof(phyHdr));
#ifdef ADSLDRV_LITTLE_ENDIAN
	phyHdr.lmemOffset = ADSL_ENDIAN_CONV_INT32(phyHdr.lmemOffset);
	phyHdr.lmemSize = ADSL_ENDIAN_CONV_INT32(phyHdr.lmemSize);
	phyHdr.sdramOffset = ADSL_ENDIAN_CONV_INT32(phyHdr.sdramOffset);
	phyHdr.sdramSize = ADSL_ENDIAN_CONV_INT32(phyHdr.sdramSize);
#endif
	AdslDrvPrintf(TEXT("%s: lmemOffset=%ld, lmemSize=%ld, sdramOffset=%ld, sdramSize=%ld\n"),
		__FUNCTION__, phyHdr.lmemOffset, phyHdr.lmemSize, phyHdr.sdramOffset, phyHdr.sdramSize);
	memcpy(pAdslLMem, pFileInMem+phyHdr.lmemOffset, phyHdr.lmemSize);
	pAdslSDRAM = AdslCoreSetSdramImageAddr(ADSL_ENDIAN_CONV_INT32(((uint*)pAdslLMem)[2]), ADSL_ENDIAN_CONV_INT32(((uint*)pAdslLMem)[3]), phyHdr.sdramSize);
	AdslCoreSetXfaceOffset(pAdslLMem, phyHdr.lmemSize);
	memcpy(pAdslSDRAM, pFileInMem+phyHdr.sdramOffset, phyHdr.sdramSize);

	return phyHdr.lmemSize + phyHdr.sdramSize;
}
#endif

static AC_BOOL AdslCoreLoadImage(void)
{
#ifndef _NOOS
	volatile uint	*pAdslLMem = (uint *) HOST_LMEM_BASE;
#endif
#ifdef SUPPORT_PHY_BIN_FROM_SDRAM
	AdslDrvPrintf(TEXT("*** AdslCoreLoadImage: phyBinAddr = 0x%08X\n"), (uint)&phyBinAddr[0]);
#endif

#ifdef ADSL_PHY_FILE2

	if (!AdslFileReadFile("/etc/adsl/adsl_lmem.bin", pAdslLMem))
		return AC_FALSE;
	{
	void *pSdramImage;

	pSdramImage = AdslCoreSetSdramImageAddr(pAdslLMem[2], pAdslLMem[3], 0);
	return (AdslFileReadFile("/etc/adsl/adsl_sdram.bin", pSdramImage) != 0);
	}
	
#elif defined(ADSL_PHY_FILE)

#if defined(SUPPORT_MULTI_PHY) && defined(CONFIG_BCM963268)
	AdslDrvPrintf(TEXT("AdslCoreLoadImage: %s PHY\n"), (BCM_IMAGETYPE_SINGLELINE==XdslCoreGetPhyImageType()) ? "Single line": "Bonding" );
#elif defined(SUPPORT_ANNEXAB_COMBO)
	curPhyImageIndex = IsAnnexBBoard() ? 1 : 0;
#elif defined(CONFIG_BCM_DSL_GFAST) && !defined(CONFIG_BCM_DSL_GFASTCOMBO)
	AdslDrvPrintf(TEXT("AdslCoreLoadImage: %s PHY\n"), (1==curPhyImageIndex) ? "Gfast": "Non-Gfast" );
#endif
#ifdef SUPPORT_PHY_BIN_FROM_SDRAM
	if(loadImageFromSdram)
		return (AdslFileInSdramLoadImage((void*)pAdslLMem, NULL) != 0);
	else
#endif
#ifdef SUPPORT_PHY_BIN_FROM_TMP
	if(loadImageFromTmp)
		return (AdslFileLoadImage("/tmp/adsl_phy.bin", (void*)pAdslLMem, NULL) != 0);
	else
#endif
	return (AdslFileLoadImage(PHY_IMAGE_NAME, (void*)pAdslLMem, NULL) != 0);

#else	/* !ADSL_PHY_FILE */

	AC_BOOL	res = TRUE;

	/* Copying ADSL core program to LMEM and SDRAM */
	AdslDrvPrintf(TEXT("AdslCoreLoadImage: adsl_lmem(%p) size = 0x%x adsl_sdram(%p) size = 0x%x\n"),
		adsl_lmem, (uint)sizeof(adsl_lmem), adsl_sdram, (uint)sizeof(adsl_sdram));
#ifdef _NOOS
	res = load_MIPS_image(get_cpuid(), (unsigned long)&adsl_lmem[0], (sizeof(adsl_lmem)+0xF) & ~0xF, HOST_LMEM_BASE);
	if(TRUE != res)
		return res;
	AdslCoreSetSdramImageAddr(0x10000000, 0, sizeof(adsl_sdram));
	res = load_MIPS_image(get_cpuid(), (unsigned long)&adsl_sdram[0], (sizeof(adsl_sdram)+0xF) & ~0xF, (unsigned long)AdslCoreGetSdramImageStart());
#else
{
#ifdef CONFIG_BRCM_IKOS
	void *pSrcAddr;
	volatile void *pDstAddr;

	pSrcAddr = (void *)adsl_lmem;
	pDstAddr = (volatile void *)pAdslLMem;
	printk("%s: Copying LMEM image: pLmemImageAddr = 0x%px, adsl_lmem = 0x%px\n", __FUNCTION__, pDstAddr, pSrcAddr);
	memcpy(pDstAddr, pSrcAddr, sizeof(adsl_lmem));
#else
	BlockByteMove ((sizeof(adsl_lmem)+0xF) & ~0xF, (void *)adsl_lmem, (uchar *) pAdslLMem);
#endif
	AdslCoreSetSdramImageAddr(ADSL_ENDIAN_CONV_INT32(((uint *) adsl_lmem)[2]), ADSL_ENDIAN_CONV_INT32(((uint *) adsl_lmem)[3]), sizeof(adsl_sdram));
	AdslCoreSetXfaceOffset(pAdslLMem, sizeof(adsl_lmem));
#ifdef CONFIG_BRCM_IKOS
	pSrcAddr = (void *)adsl_sdram;
	pDstAddr = (volatile void *)AdslCoreGetSdramImageStart();
	printk("%s: Copying SDRAM image: pSdramImageAddr = 0x%px, adsl_sdram = 0x%px\n", __FUNCTION__, pDstAddr, pSrcAddr);
	memcpy(pDstAddr, pSrcAddr, sizeof(adsl_sdram));
#else
	BlockByteMove (AdslCoreGetSdramImageSize(), (void *)adsl_sdram, AdslCoreGetSdramImageStart());
#endif
}
#endif

	return res;
#endif	/* ADSL_PHY_FILE2 */
}

#ifdef CONFIG_PHY_PARAM
static AC_BOOL AdslCoreProcessPhyParams(void)
{
	volatile DslParamData *pDslParam = (void *) ((uint *) HOST_LMEM_BASE + 0x388/4);
	uint    size;
	ushort   len;
	AC_BOOL  res = AC_TRUE;

	size = ADSL_ENDIAN_CONV_INT32(pDslParam->lowSdramSize);
	len  = ADSL_ENDIAN_CONV_SHORT(pDslParam->len);
	DiagStrPrintf(0,DIAG_DSL_CLIENT, "AdslCoreProcessPhyParams: len=%d ver=%d sz=%d\n", len, pDslParam->verId, size);
	if ((len >= 8) && (0 != size)) {
	  size = (uint) BcmCoreAllocLowMem(size);
	  if (size != 0)
	    pDslParam->lowSdramSize = ADSL_ENDIAN_CONV_INT32(size | 0xA0000000);
	  DiagStrPrintf(0,DIAG_DSL_CLIENT, "AdslCoreProcessPhyParams: pPHY=0x%08X\n", pDslParam->lowSdramSize);
	}

	return res;
}
#endif

AdslOemSharedData		adslOemDataSave;
Boolean					adslOemDataSaveFlag = AC_FALSE;
Boolean					adslOemDataModified = AC_FALSE;

static int StrCmp(char *s1, char *s2, int n)
{
	while (n > 0) {
		if (*s1++ != *s2++)
			return 1;
		n--;
	}
	return 0;
}

static int Str2Num(char *s, char **psEnd)
{
	int		n = 0;

	while ((*s >= '0') && (*s <= '9'))
		n = n*10 + (*s++ - '0');

	*psEnd = s;
	return n;
}

static char *StrSkip(char *pVer)
{
	while ((*pVer == ' ') || (*pVer == '\t') || (*pVer == '_'))
		pVer++;
	return pVer;
}

static char *AdslCoreParseVersionString(char *sVer, adslPhyInfo *pInfo)
{
	static char adslVerStrAnchor[] = "Version";
	char  *pVer = sVer;
	
	while (StrCmp(pVer, adslVerStrAnchor, sizeof(adslVerStrAnchor)-1) != 0) {
		pVer++;
		if (0 == *pVer)
			return pVer;
	}

	pVer += sizeof(adslVerStrAnchor)-1;
	pVer = StrSkip(pVer);
	if (0 == *pVer)
		return pVer;

	pInfo->pVerStr0 = (uint)(uintptr_t)pVer;
	pInfo->pVerStr = pVer;
	pInfo->chipType = kAdslPhyChipUnknown;
	switch (*pVer) {
		case 'A':
		case 'B':
			pVer++;	/* Annex A or B PHY type */
			break;
		default:
			printk("%s: Unknown version string (%s)\n", __FUNCTION__, pVer);
			return pVer;
	}
	
	while((kAdslPhyChipUnknown == pInfo->chipType) && (0 != *pVer)) {
		switch(*pVer) {
			case 'D':
				pInfo->chipType = kAdslPhyChip6328;
				pVer++;
				break;
			case 'E':
				pInfo->chipType = kAdslPhyChip6362;
				pVer++;
				break;
			case 'F':
				pInfo->chipType = kAdslPhyChip63268;
				pVer++;
				break;
			case 'G':
				pInfo->chipType = kAdslPhyChip6318;
				pVer++;
				break;
			case 'H':
				pInfo->chipType = kAdslPhyChip63138;
				pVer++;
				break;
			case 'I':
				pInfo->chipType = kAdslPhyChip63381;
				pVer++;
				break;
			case 'J':
				pInfo->chipType = kAdslPhyChip63148;
				pVer++;
				break;
			case 'K':
				pInfo->chipType = kAdslPhyChip63158;
				pVer++;
				break;
			case 'L':
				pInfo->chipType = kAdslPhyChip63178;
				pVer++;
				break;
			case 'M':
				pInfo->chipType = kAdslPhyChip63146;
				pVer++;
				break;
			default:
				pVer++;
				break;
		}
	}
	if (0 == *pVer) {
		printk("%s: Missing PHY mj/mn version number\n", __FUNCTION__);
		return pVer;
	}
	
	pVer = StrSkip(pVer);
	if (0 == *pVer)
		return pVer;

	pInfo->mjVerNum = Str2Num(pVer, &pVer);
	if (0 == *pVer)
		return pVer;

	if ((*pVer >= 'a') && (*pVer <= 'z')) {
		int		n;

		pInfo->mnVerNum = (*pVer - 'a' + 1) * 100;
		n = Str2Num(pVer+1, &pVer);
		pInfo->mnVerNum += n;
	}
	return pVer;
}

#if defined(BBF_IDENTIFICATION)
static char XdslCoreGetVersionChipType(ushort chipType)
{
	char verChipType;
	
	switch(chipType) {
		case kAdslPhyChip6328:
			verChipType = 'D';
			break;
		case kAdslPhyChip6362:
			verChipType = 'E';
			break;
		case kAdslPhyChip63268:
			verChipType = 'F';
			break;
		case kAdslPhyChip6318:
			verChipType = 'G';
			break;
		case kAdslPhyChip63138:
			verChipType = 'H';
			break;
		case kAdslPhyChip63381:
			verChipType = 'I';
			break;
		case kAdslPhyChip63148:
			verChipType = 'J';
			break;
		case kAdslPhyChip63158:
			verChipType = 'K';
			break;
		case kAdslPhyChip63178:
			verChipType = 'L';
			break;
		case kAdslPhyChip63146:
			verChipType = 'M';
			break;
		default:
			verChipType = '?';
			break;
	}
	
	return verChipType;
}
#endif

void AdslCoreExtractPhyInfo(AdslOemSharedData *pOemData, adslPhyInfo *pInfo)
{
	char				*pVer;
	int					i;
	Boolean				bPhyFeatureSet;
	pInfo->fwType		= 0;
	pInfo->chipType		= kAdslPhyChipUnknown;
	pInfo->mjVerNum		= 0;
	pInfo->mnVerNum		= 0;
	pInfo->pVerStr		= NULL;
	for (i = 0; i < sizeof(pInfo->features)/sizeof(pInfo->features[0]);i++)
		pInfo->features[i]	= 0;

	if (NULL == pAdslOemData)
		return;

	if (NULL == (pVer = AdslCoreGetVersionString()))
		return;

	pVer = AdslCoreParseVersionString(pVer, pInfo);

	bPhyFeatureSet = false;
	for (i = 0; i < sizeof(pInfo->features)/sizeof(pInfo->features[0]); i++)
		if (ADSL_ENDIAN_CONV_INT32(pAdslXface->gfcTable[i]) != 0) {
			bPhyFeatureSet = true;
			break;
		}

	if (bPhyFeatureSet) {
		for (i = 0; i < sizeof(pInfo->features)/sizeof(pInfo->features[0]); i++) {
			pInfo->features[i] = ADSL_ENDIAN_CONV_INT32(pAdslXface->gfcTable[i]);
			pAdslXface->gfcTable[i] = 0;
		}
	}

#if defined(BBF_IDENTIFICATION)
	{
	char	*p, *pEocVer = pOemData->eocVersion;
	int		n;

	*pEocVer++ = AdslFeatureSupported(pInfo->features, kAdslPhyAnnexB) ? 'B' : 'A';
	if (AdslFeatureSupported(pInfo->features, kAdslPhyAdslReAdsl2))
		*pEocVer++ = 'p';
	if (AdslFeatureSupported(pInfo->features, kAdslPhyVdslG993p2)) {
		if (AdslFeatureSupported(pInfo->features, kAdslPhyVdsl30a))
			*pEocVer++ = '6';
		else if (AdslFeatureSupported(pInfo->features, kAdslPhyBonding))
			*pEocVer++ = '3';
		*pEocVer++ = 'v';
	}
	if (AdslFeatureSupported(pInfo->features, kAdslPhyGfast))
		*pEocVer++ = 'f';
	if (AdslFeatureSupported(pInfo->features, kAdslPhyBonding))
		*pEocVer++ = 'b';
	
	*pEocVer++ = XdslCoreGetVersionChipType(pInfo->chipType);

	*pEocVer++ = '0' + pInfo->mjVerNum/10;
	*pEocVer++ = '0' + pInfo->mjVerNum%10;
	if (pInfo->mnVerNum != 0) {
		n = pInfo->mnVerNum/100;
		*pEocVer++ = 'a' - 1 + n;
		n = pInfo->mnVerNum - n * 100;
		if (n != 0)
			*pEocVer++ = '0' + n;
	}
	*pEocVer++  = ('_' == *pVer) ? '!' : '.';

	p = ADSL_DRV_VER_STR;
	while ((*p != ' ') && (*p != 0) && (*p != '_'))
		*pEocVer++ = *p++;
	*pEocVer++  = ('_' == *p) ? '!' : ' ';

	*pEocVer++ = '0' + ((PERF->RevID >> 20) & 0xF);
	*pEocVer++ = '0' + ((PERF->RevID >> 16) & 0xF);

	p = pEocVer;
	while (p < (char *) pOemData->eocVersion + 16)
		*p ++ = 0;
	pOemData->eocVersionLen = ADSL_ENDIAN_CONV_INT32(pEocVer - (char *) pOemData->eocVersion);
	}
#endif
}

void AdslCoreSaveOemData(void)
{
	if ((NULL != pAdslOemData) && adslOemDataModified) {
		BlockByteMove ((sizeof(adslOemDataSave) + 3) & ~0x3, (void *)pAdslOemData, (void *) &adslOemDataSave);
		adslOemDataSaveFlag = true;
		AdslDrvPrintf(TEXT("Saving OEM data from 0x%px\n"), pAdslOemData);
	}
}

void AdslCoreRestoreOemData(void)
{
	if ((NULL != pAdslOemData) && adslOemDataSaveFlag) {
		BlockByteMove ((FLD_OFFSET(AdslOemSharedData,g994RcvNonStdInfo) + 3) & ~0x3, (void *) &adslOemDataSave, (void *)pAdslOemData);
		BcmAdslCoreDiagWriteStatusString(0, "Restoring OEM data from 0x%px\n", pAdslOemData);
	}
}

void AdslCoreProcessOemDataAddrMsg(dslStatusStruct *status)
{
#ifdef FLATTEN_ADDR_ADJUST
	pAdslOemData = ADSL_ADDR_TO_HOST(status->param.value);
#else
	pAdslOemData = (void *)(uintptr_t)status->param.value;
#endif
	AdslCoreRestoreOemData();
	AdslCoreExtractPhyInfo(pAdslOemData, &adslCorePhyDesc);

#ifdef G992P3
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2) && (NULL != pAdslOemData->clEocBufPtr)) {
		int clEocBufLen =ADSL_ENDIAN_CONV_INT32(pAdslOemData->clEocBufLen);
		uchar *clEocBufPtr = ADSL_ADDR_TO_HOST(ADSL_ENDIAN_CONV_INT32((uint)pAdslOemData->clEocBufPtr));
#ifdef SUPPORT_DSL_BONDING
		if (ADSL_PHY_SUPPORT(kAdslPhyBonding)) {
			clEocBufLen = clEocBufLen/2;
			G997SetTxBuffer(XdslCoreGetDslVars(0), clEocBufLen, clEocBufPtr);
			G997SetTxBuffer(XdslCoreGetDslVars(1), clEocBufLen, clEocBufPtr+clEocBufLen);
		}
		else {
			G997SetTxBuffer(XdslCoreGetDslVars(0), clEocBufLen, clEocBufPtr);
		}
#else
		G997SetTxBuffer(XdslCoreGetDslVars(0), clEocBufLen, clEocBufPtr);
#endif
	}
#endif
}

void AdslCoreSendFilterSNRMarginCmd(void)
{
	dslCommandStruct	cmd;
	
	cmd.command = kDslFilterSNRMarginCmd;
	cmd.param.value = 0xFF;
	AdslCoreCommandHandler(&cmd);
#ifdef SUPPORT_DSL_BONDING
	/* Do we have to send this per line for bonding PHY ??? */
	if (ADSL_PHY_SUPPORT(kAdslPhyBonding)) {
		cmd.command = kDslFilterSNRMarginCmd | (1 << DSL_LINE_SHIFT);
		cmd.param.value = 0xFF;
		AdslCoreCommandHandler(&cmd);
	}
#endif
}

void AdslCoreStop(void)
{
	writeAdslEnum(ADSL_INTMASK_I, 0);
	BcmCoreDpcSyncEnter(SYNC_RX);
	AdslCoreIndicateLinkDown(0);
#ifdef SUPPORT_DSL_BONDING
	if (ADSL_PHY_SUPPORT(kAdslPhyBonding))
		AdslCoreIndicateLinkDown(1);
#endif
	BcmCoreDpcSyncExit(SYNC_RX);
	AdslCoreSaveOemData();
	DISABLE_ADSL_MIPS;
	bcmOsDelay(1);	/* Delay 1ms */
	RESET_ADSL_CORE;
}

//#define BCM6348_ADSL_MIPS_213MHz
#define BCM6348_ADSL_MIPS_240MHz


void AdslCoreSetPllClock(void)
{

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)

	pXdslControlReg[0] = (pXdslControlReg[0] & ~ADSL_CLK_RATIO_MASK) | ADSL_CLK_RATIO;
#elif defined(CONFIG_BCM963268)

#if defined(BOARD_H_API_VER) && BOARD_H_API_VER > 0
        if (kerSysGetDslPhyEnable()) {
	    MISC->miscIddqCtrl &= ~(MISC_IDDQ_CTRL_VDSL_PHY | MISC_IDDQ_CTRL_VDSL_MIPS);
        }
#else
	MISC->miscIddqCtrl &= ~(MISC_IDDQ_CTRL_VDSL_PHY | MISC_IDDQ_CTRL_VDSL_MIPS);
#endif
	if(IsAfe6306ChipUsed()) {
		unsigned int afeIds[2];
		BcmXdslCoreGetAfeBoardId(&afeIds[0]);
		if((afeIds[0] & AFE_CHIP_MASK) == AFE_CHIP_6306_BITMAP) {
#ifdef BP_GET_INT_AFE_DEFINED
			unsigned short bpGpio;
			if(BP_SUCCESS == BpGetAFELDRelayGpio(&bpGpio)) {
				if((afeIds[0] & AFE_LD_MASK) == AFE_LD_ISIL1556_BITMAP) 
					kerSysSetGpio(bpGpio, kGpioActive);	/* Activate the switch relay to use the Intersil LD */
				else
					kerSysSetGpio(bpGpio, kGpioInactive);	/* De-activate the switch relay */
			}
#endif
			/* Power off the internal AFE */
			pXdslControlReg[0] |= XDSL_AFE_GLOBAL_PWR_DOWN;
		}
		else {
			/* Most likely bonding target, power up the internal AFE */
			pXdslControlReg[0] &= ~XDSL_AFE_GLOBAL_PWR_DOWN;
		}
		GPIO->GPIOMode |= GPIO_MODE_ADSL_SPI_MOSI | GPIO_MODE_ADSL_SPI_MISO;
		GPIO->PadControl = (GPIO->PadControl & ~GPIO_PAD_CTRL_AFE_MASK) | GPIO_PAD_CTRL_AFE_VALUE;
	}
	else
		pXdslControlReg[0] &= ~XDSL_AFE_GLOBAL_PWR_DOWN;	/* Power up the internal AFE */
#endif
}

#ifdef	SDRAM_HOLD_COUNTERS
#define	rd_shift			11
#define	rt_shift			16
#define	rs_shift			21

#define INSTR_CODE(code)	#code
#define GEN_MFC_INSTR(code,rd)	__asm__ __volatile__( ".word\t" INSTR_CODE(code) "\n\t" : : : "$" #rd)
#define GEN_MTC_INSTR(code)		__asm__ __volatile__( ".word\t" INSTR_CODE(code) "\n\t")

#define opcode_MTC0			0x40800000
#define opcode_MFC0			0x40000000

#define MTC0_SEL(rd,rt,sel)	GEN_MTC_INSTR(opcode_MTC0 | (rd << rd_shift) | (rt << rt_shift) | (sel))
#define MFC0_SEL(rd,rt,sel)	GEN_MFC_INSTR(opcode_MFC0 | (rd << rd_shift) | (rt << rt_shift) | (sel), rt)


void AdslCoreStartSdramHoldCounters(void)
{
	__asm__ __volatile__("li\t$6,0x80000238\n\t" : :: "$6");
	MTC0_SEL(25,6, 6);

	__asm__ __volatile__("li\t  $7,0x7\n\t" : :: "$7");
	/* __asm__ __volatile__("li\t  $7,0x8\n\t" : :: "$7"); */
	__asm__ __volatile__("sll\t $8, $7, 2\n\t" : :: "$8");
	__asm__ __volatile__("sll\t $9, $7, 18\n\t" : :: "$9");
	__asm__ __volatile__("li\t  $10, 0x80008100\n\t" : :: "$10");
	__asm__ __volatile__("or\t  $10, $10, $8\n\t" : :: "$10");
	__asm__ __volatile__("or\t  $10, $10, $9\n\t" : :: "$10");

	MTC0_SEL(25,0, 0);
	MTC0_SEL(25,0, 1);
	MTC0_SEL(25,0, 2);
	MTC0_SEL(25,0, 3);

	MTC0_SEL(25,10,4);

/*
li    $6, 0x8000_0238
mtc0    $6, $25, 6        // select sys_events to test_mux

li    $7, 0x7                // pick a value between [15:6], that corresponds to sys_event[9:0]
sll    $8, $7, 2
sll    $9, $7, 18
li    $10, 0x8000_8100     // counter1 counts active HIGH, counter0 counts positive edge
or    $10, $10, $8
or    $10, $10, $9

mtc0    $0, $25, 0        // initialize counters to ZEROs
mtc0    $0, $25, 1
mtc0    $0, $25, 2
mtc0    $0, $25, 3
 
mtc0    $10, $25, 4    // start counting
*/
}
 
uint AdslCoreReadtSdramHoldEvents(void)
{
	MFC0_SEL(25,2,0);
}

uint AdslCoreReadtSdramHoldTime(void)
{
	MFC0_SEL(25,2,1);
}

void AdslCorePrintSdramCounters(void)
{
	BcmAdslCoreDiagWriteStatusString(0, "SDRAM Hold: Events = %d, Total Time = %d", 
		AdslCoreReadtSdramHoldEvents(),
		AdslCoreReadtSdramHoldTime());
}
#endif /* SDRAM_HOLD_COUNTERS */

#ifndef MEM_MONITOR_ID
#define MEM_MONITOR_ID	0
#endif

#ifdef CONFIG_BRCM_IKOS
#define	HW_RESET_DELAY_MS	100	/* 100 ms */
#else
#define	HW_RESET_DELAY_MS	500	/* 500 ms */
#endif

#if defined(CONFIG_BCM96328)
#define PI_PHASE_OFFSET                     0x29
#define PI_PHASE_READ_CNT                   200
#define PI_PHASE_SEARCH_RANGE               (416*4)

static void shiftPiPhase(int shiftCnt, int isInc)
{
    uint tmpValue;
    uint phase;
    int   phaseDiff;
    int   i;

    tmpValue = DDR->PI_DSL_PHY_CTL;
    phase = tmpValue & 0x3fff;
    tmpValue &= ~0x3fff;    /* mask out phase */
    tmpValue &= ~0x4000;    /* mask out inc/dec */
    phaseDiff = -1;         /* default is decrease */
    if (isInc) {
        tmpValue |= 0x4000;
        phaseDiff = 1;
    }

    for (i=0; i<shiftCnt; i++) {
        phase += phaseDiff;
        DDR->PI_DSL_PHY_CTL = tmpValue|phase;
    }
} 

static int getPiPhase(int readCnt)
{
    uint   regValue;
    int     zeroCnt = 0;
    int     oneCnt = 0;
    int     loopCnt;

    for (loopCnt = 0; loopCnt < readCnt; loopCnt++) {
        regValue = *((uint volatile *) 0xb0d5f0c4); /* RBUS_QPROC_CLK_SYN_STATUS */
        if (regValue & 1)
            oneCnt++;
        else 
            zeroCnt++;
    }

    if (oneCnt != 0 && zeroCnt != 0) 
        return -1;      /* un-reliable */ 
    else if (oneCnt != 0) 
        return 1; 
    else 
        return 0; 
} 

static int XdslPhyClockAlign(void)
{
   uint regValue, cycleCnt0;
   int   prevPhase, currPhase;
   int   currLoopCnt, prevLoopCnt;
   int   zero2OnePhase = 0;

   /* Disable HW auto PI sync */ 
   printk("Disable HW auto PI ...\n");
   *((uint volatile *) 0xb0d5f0c0) = 0x00700387;
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);
   regValue &= ~0x100000;
   DDR->PI_DSL_PHY_CTL = regValue;
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);

   /* Collect init phase */ 
   printk("Reset PI counter ...\n");
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);
   regValue &= 0x3fff; 
   shiftPiPhase(regValue, 0);  /* shift left to make phase equal to 0 */ 
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);

   /* look for 0->1 or 1->0 transitions */ 
   printk("Look for transitions ...\n");
   prevPhase = getPiPhase(PI_PHASE_READ_CNT);
   currPhase = prevPhase; 
   prevLoopCnt = 0; 

   for (currLoopCnt = 0; currLoopCnt <= PI_PHASE_SEARCH_RANGE; currLoopCnt++) { 
       shiftPiPhase(1, 1);
       /* Need to wait 5 usecs here */
      cycleCnt0 = BcmAdslCoreGetCycleCount();
      while (TmElapsedUs(cycleCnt0) < 5)
       currPhase = getPiPhase(PI_PHASE_READ_CNT); 
       if (currPhase >= 0) { 
           if (currPhase != prevPhase) { 
               printk(
                   "Transition found at (%4d -> %4d): %1d -> %1d\n",
                   prevLoopCnt, currLoopCnt, prevPhase, currPhase);
               if (currPhase == 1 && currLoopCnt > PI_PHASE_OFFSET) {
                   if (zero2OnePhase == 0)
                       zero2OnePhase = currLoopCnt;
                   if ((currLoopCnt - zero2OnePhase) <= 8)
                       zero2OnePhase = currLoopCnt;
              }
           }
           prevPhase = currPhase;
           prevLoopCnt = currLoopCnt;
       }
   } 
   printk("Zero2OnePhase ==> 0x%4d\n", zero2OnePhase);
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);

   /* Align clock phase to be zero2OnePhase - PI_PHASE_OFFSET */ 
   printk("Reset PI counter ...\n");
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);
   regValue &= 0x3fff; 
   shiftPiPhase(regValue, 0);  /* shift left to make phase equal to 0 */ 
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);
   printk("Align PI counter to 1st zero to one minus PI offset\n");
   shiftPiPhase(zero2OnePhase - PI_PHASE_OFFSET, 1);  /* shift right */ 
   regValue = DDR->PI_DSL_PHY_CTL;
   printk("regDdrCtlPiDslPhyCtl ==> 0x%08x\n", regValue);

   return 1;
} 
#endif /* defined(CONFIG_BCM96328) */


#if defined(USE_6306_CHIP)
void XdslCoreHwReset6306(void)
{
#ifdef _NOOS
	return;
#elif defined(BP_GET_EXT_AFE_DEFINED)
	unsigned short bpGpio;

	if(BP_SUCCESS == BpGetExtAFEResetGpio(&bpGpio)) {
		AdslDrvPrintf(TEXT("*** XdslCoreHwReset6306 ***\n"));
		kerSysSetGpio(bpGpio, kGpioInactive);
		kerSysSetGpio(bpGpio, kGpioActive);
		kerSysSetGpio(bpGpio, kGpioInactive);
	}
#else
	BCMOS_DECLARE_IRQFLAGS(flags);

	AdslDrvPrintf(TEXT("*** XdslCoreHwReset6306 ***\n"));
	/* Default to 96368MBG board*/
	/* miwang (7/16/10) should call kerSysSetGpioState to do this,
	 * but kerSysSetGpioState cannot reach these high bits, so
	 * just acquire gpio spinlock and access here.
	 */
	BCMOS_SPIN_LOCK_IRQ(&bcm_gpio_spinlock, flags);
	GPIO->GPIODir_high |= 0x00000008;
	GPIO->GPIOio_high |= 0x00000008;
	GPIO->GPIOio_high &= ~0x00000008;
	GPIO->GPIOio_high |= 0x00000008;
	BCMOS_SPIN_UNLOCK_IRQ(&bcm_gpio_spinlock, flags);
#endif
}

/* Delay in uSecond */
#define WR_LD_REG_DATA_WAIT      (10)
#define WR_LD_REG_CLK_WAIT       (10)

void XdslCoreConfigExtAFELD6303(LONGLONG data)
{
	int shift = 38;
	int bits, d, k;
	unsigned short bpGpioData, bpGpioClk;
	ulong cycleCnt0;
	
	AdslDrvPrintf(TEXT("*** Config data for 6303 = 0x%llX ***\n"), data);
#ifndef _NOOS
#if defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 1)
	if((BP_SUCCESS != BpGetExtAFELDDataGpio(&bpGpioData)) || (BP_SUCCESS != BpGetExtAFELDClkGpio(&bpGpioClk))) {
		AdslDrvPrintf(TEXT("*** BpGetExtAFELDDataGpio()/BpGetExtAFELDClkGpio() failed ***\n"));
		return;
	}
#else
	AdslDrvPrintf(TEXT("*** Using hard-coded GPIO pins 50(clk) and 51(data) ***\n"));
	bpGpioData = 51; bpGpioClk = 50;
#endif
	/* Program line driver register through serial port
	*  Register is 40 bits, programmed on both clock edges
	*/
	for (k=0; k < 20; k++) {
		bits = ((int)(data>>shift))&0x3;
		/* Write bit
		* Generate rising edge 
		*/
		d = (bits & 0x1);
		kerSysSetGpio(bpGpioData, (1==d)? kGpioActive: kGpioInactive);
		cycleCnt0 = BcmAdslCoreGetCycleCount();
		while (BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cycleCnt0) < WR_LD_REG_DATA_WAIT);
		
		kerSysSetGpio(bpGpioClk, kGpioActive);
		cycleCnt0 = BcmAdslCoreGetCycleCount();
		while (BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cycleCnt0) < WR_LD_REG_CLK_WAIT);
		
		/* Write bit
		* Generate falling edge
		*/
		d = (bits>>1)&0x1;
		kerSysSetGpio(bpGpioData, (1==d)? kGpioActive: kGpioInactive);
		cycleCnt0 = BcmAdslCoreGetCycleCount();
		while (BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cycleCnt0) < WR_LD_REG_DATA_WAIT);
		
		kerSysSetGpio(bpGpioClk, kGpioInactive);
		cycleCnt0 = BcmAdslCoreGetCycleCount();
		while (BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cycleCnt0) < WR_LD_REG_CLK_WAIT);
		/* Update mask/shift */
		shift -= 2;
	}
#endif /* _NOOS */
}

void XdslCoreSetExtAFELDMode(int ldMode)
{
#ifdef BP_GET_EXT_AFE_DEFINED
	unsigned short bpGpioPwr, bpGpioMode;

	if((BP_SUCCESS == BpGetExtAFELDPwrGpio(&bpGpioPwr)) &&
		(BP_SUCCESS == BpGetExtAFELDModeGpio(&bpGpioMode))) {
#ifdef CONFIG_BCM963268
		/* By default the CFE has this bit set - control from PHY, but PHY is passing control to the Host, so clear it */
		GPIO->GPIOBaseMode &= ~GPIO_BASE_VDSL_PHY_OVERRIDE_1;
#endif
		kerSysSetGpio(bpGpioPwr, kGpioInactive);	/* Turn Off LD power*/
		kerSysSetGpio(bpGpioMode, (LD_MODE_ADSL==ldMode)? kGpioActive: kGpioInactive);	/* ACTIVE-ADSL/INACTIVE-VDSL */
		kerSysSetGpio(bpGpioPwr, kGpioActive);	/* Turn On LD power*/
		DiagWriteString(0, DIAG_DSL_CLIENT, "*** External AFE LD Mode: %s ***\n", (LD_MODE_ADSL==ldMode)? "ADSL": "VDSL");
	}
#ifdef CONFIG_BCM963268
	else {
		/* Building with old Linux tree, use hard-coded gpio pins */
		bpGpioMode=12;
		bpGpioPwr=13;
		GPIO->GPIOBaseMode &= ~GPIO_BASE_VDSL_PHY_OVERRIDE_1;
		kerSysSetGpio(bpGpioPwr, kGpioInactive);	/* Turn Off LD power*/
		kerSysSetGpio(bpGpioMode, (LD_MODE_ADSL==ldMode)? kGpioActive: kGpioInactive);	/* ACTIVE-ADSL/INACTIVE-VDSL */
		kerSysSetGpio(bpGpioPwr, kGpioActive);	/* Turn On LD power*/
		DiagWriteString(0, DIAG_DSL_CLIENT, "*** External AFE LD Mode: %s ***\n", (LD_MODE_ADSL==ldMode)? "ADSL": "VDSL");
	}
#endif
#endif	/* BP_GET_EXT_AFE_DEFINED */
}
#endif	/* defined(USE_6306_CHIP) */

#if defined(USE_PMC_API) && defined(DSL_KTHREAD)
extern void BcmXdslDrvConfigReqEventNotify(unsigned char lineId, volatile drvRegControl *pDrvRegCtrl);
#endif

void XdslCoreProcessDrvConfigRequest(int lineId, volatile uint	*pDrvConfigAddr)
{
	uint ctrlMsg = ADSL_ENDIAN_CONV_INT32(*pDrvConfigAddr) >> 1;
#ifdef CONFIG_BCM_DSL_GFAST
	int res;
	unsigned short bpGpioAFELDRelay;
#endif

	switch(ctrlMsg) {
#if defined(USE_6306_CHIP)
		case kDslDrvConfigExtLD6303:
		{
			volatile drvRegControl *pDrvRegCtrl = (drvRegControl *)pDrvConfigAddr;
			XdslCoreConfigExtAFELD6303(pDrvRegCtrl->data);
			break;
		}
		case kDslDrvConfigSetLD6302Adsl:
			XdslCoreSetExtAFELDMode(LD_MODE_ADSL);
			break;
		case kDslDrvConfigSetLD6302Vdsl:
			XdslCoreSetExtAFELDMode(LD_MODE_VDSL);
			break;
		case kDslDrvConfigReset6306:
			XdslCoreHwReset6306();
			break;
		case kDslDrvConfigDisable6306RefClk:
			DisableReferenceClockTo6306();
			break;
#endif	/* defined(USE_6306_CHIP) */
		case kDslDrvConfigRegRead:
			((drvRegControl *) pDrvConfigAddr)->regValue = 
				*(uint *) ((uintptr_t)(((drvRegControl *) pDrvConfigAddr)->regAddr));
			break;
		case kDslDrvConfigRegWrite:
			*(uint *) ((uintptr_t)(((drvRegControl *) pDrvConfigAddr)->regAddr)) =
				((drvRegControl *) pDrvConfigAddr)->regValue;
			break;
		case kDslDrvConfigAhifStatePtr:
#ifndef SUPPORT_DSL_BONDING
			if (NULL == acAhifStatePtr[lineId])
#endif
			{
			uint tmp = ADSL_ENDIAN_CONV_INT32(pDrvConfigAddr[1]);
			acAhifStatePtr[lineId] = pDrvConfigAddr + 1;
			acAhifStateMode = (tmp == (uint) -2) ? 1 : 0;
			pDrvConfigAddr[1] = ADSL_ENDIAN_CONV_INT32(acAhifStateMode);
			}
			break;
#if defined(CONFIG_VDSL_SUPPORTED) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 6) && !defined(CONFIG_BRCM_IKOS)
		case kDslDrvConfigLD6303V5p3:
		{
			int res;
			unsigned short bpGpio;
#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 11)
			res = BpGetAFEVR5P3PwrEnGpio(lineId, &bpGpio);
#else
			res = BpGetAFEVR5P3PwrEnGpio(&bpGpio);
#endif
			if(BP_SUCCESS == res) {
				kerSysSetGpioState(bpGpio, kGpioActive);
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "*** Drv: LD6303V5p3 is activated (bpGpio 0x%04X)\n", bpGpio);
			}
			else
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "*** Drv: LD6303V5p3 is not configured!\n");
			break;
		}
#endif
#if defined(USE_PMC_API) && defined(DSL_KTHREAD)
		case kDslDrvConfigRdAfePLLMdiv:
		case kDslDrvConfigWrAfePLLMdiv:
			BcmXdslDrvConfigReqEventNotify(lineId, (drvRegControl *)pDrvConfigAddr);
			return;
#endif
#ifdef CONFIG_BCM_DSL_GFAST
#if defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 10)
		case kDslDrvConfigLDPowerMode:
		{
			if(0xFFFF != bpGpioAFEPwrBoost[lineId]) {
				volatile drvRegControl *pDrvRegCtrl = (drvRegControl *)pDrvConfigAddr;
				uint pwrMode = ADSL_ENDIAN_CONV_INT32(pDrvRegCtrl->regValue);
#ifdef SUPPORT_DSL_BONDING
				if (bpGpioAFEPwrBoost[0] == bpGpioAFEPwrBoost[1]) {
					if (kDslLDPowerMode15V == pwrMode)
					  acPwrBoostLineMask |= 1 << lineId;
					else
					  acPwrBoostLineMask &= ~(1 << lineId);
					kerSysSetGpio(bpGpioAFEPwrBoost[0], acPwrBoostLineMask ? kGpioActive: kGpioInactive);
					DiagWriteString(lineId, DIAG_DSL_CLIENT, "*** Drv: AFE LD power boost single GPIO state=0x%X (bpGpio 0x%04X)\n", acPwrBoostLineMask, bpGpioAFEPwrBoost[0]);
				}
				else
#endif				
				{
					kerSysSetGpio(bpGpioAFEPwrBoost[lineId], (kDslLDPowerMode15V == pwrMode)? kGpioActive: kGpioInactive);
					DiagWriteString(lineId, DIAG_DSL_CLIENT, "*** Drv: %s AFE LD power boost (bpGpio 0x%04X)\n", (kDslLDPowerMode15V==pwrMode)? "Activate": "De-activate", bpGpioAFEPwrBoost[lineId]);
				}
			}
			else
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "*** Drv: AFE LD power boost is not configured!\n");
			break;
		}
#endif
		case kDslDrvConfigAfeRelay:
#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 11)
			res = BpGetAFELDRelayGpio(lineId, &bpGpioAFELDRelay);
#else
			res = BpGetAFELDRelayGpio(&bpGpioAFELDRelay);
#endif
			if(BP_SUCCESS == res) {
				volatile drvRegControl *pDrvRegCtrl = (drvRegControl *)pDrvConfigAddr;
				uint relayMode = ADSL_ENDIAN_CONV_INT32(pDrvRegCtrl->regValue);
				
				kerSysSetGpio(bpGpioAFELDRelay, (kDslRelayModeGfast == relayMode)? kGpioInactive: kGpioActive);
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "*** Drv: %s LD Relay (bpGpio 0x%04X)\n", (kDslRelayModeGfast==relayMode)? "De-activate": "Activate", bpGpioAFELDRelay);
			}
			else
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "*** Drv: AFE relay is not configured!\n");
			break;
#endif
	}
	*pDrvConfigAddr = ADSL_ENDIAN_CONV_INT32((ctrlMsg << 1) | 1);	/* Ack to PHY */
}

#ifdef LMEM_ACCESS_WORKAROUND
extern void BcmAdslCoreTestLMEM(void);
#define	ALT_BOOT_VECTOR_SHIFT	16
#define	ALT_BOOT_VECTOR_MASK		(0xFFF << ALT_BOOT_VECTOR_SHIFT)
#define	ALT_BOOT_VECTOR_LMEM		(0xB90 << ALT_BOOT_VECTOR_SHIFT)
#define	ALT_BOOT_VECTOR_SDRAM	(((adslCorePhyDesc.sdramImageAddr&0xFFF00000) >> 20) << ALT_BOOT_VECTOR_SHIFT)

AC_BOOL AdslCoreIsPhyMipsRunning(void)
{
	return(XDSL_MIPS_RESET == (pXdslControlReg[0] & XDSL_MIPS_RESET));
}

void AdslCoreRunPhyMipsInSDRAM(void)
{
	pXdslControlReg[0] &= ~ALT_BOOT_VECTOR_MASK;
	pXdslControlReg[0] |= ALT_BOOT_VECTOR_SDRAM;
	*(volatile uint *)adslCorePhyDesc.sdramImageAddr = 0x42000020;	/* wait/loop instruction */
	*(volatile uint *)(adslCorePhyDesc.sdramImageAddr+4) = 0x08040000;
	*(volatile uint *)(adslCorePhyDesc.sdramImageAddr+8) = 0;
	ENABLE_ADSL_MIPS;
	printk("*** %s: Started ***\n", __FUNCTION__);
}
#endif

extern OS_SEMID			syncPhyMipsSemId;
extern DiagDebugData	diagDebugCmd;

static AC_BOOL __AdslCoreHwReset(AC_BOOL bCoreReset)
{
#ifndef BCM_CORE_NO_HARDWARE
#ifndef _NOOS
	volatile uint			*pAdslEnum = (uint *) XDSL_ENUM_BASE;
#endif
	volatile AdslXfaceData	*pAdslX = NULL;
	ulong				to;
	uint				tmp;
	int			nRet;
	OS_TICKS	osTime0, osTimePhyInitStart;
#if defined(PHY_BLOCK_TEST) || defined(_NOOS) || defined(SDRAM4G_SUPPORT1)
	volatile uint			*pAdslLmem = (void *) HOST_LMEM_BASE;
#endif
#if defined(PHY_BLOCK_TEST) || defined(_NOOS)
	uint				memMonitorIdIntialVal;
#ifdef CONFIG_BRCM_IKOS
	uint				initialLmemVal;
#endif
#else
	OS_TICKS	osTime1, osTimeOut = (HW_RESET_DELAY_MS/BCMOS_MSEC_PER_TICK);
#endif
#if defined(USE_6306_CHIP)
	int afe6306ChipUsed = IsAfe6306ChipUsed();
#endif
#if defined(CONFIG_BCM963268) || (defined(CONFIG_BCM963138) &&  defined(BOARD_H_API_VER) &&   (BOARD_H_API_VER > 8))
	int chipRevId = (PERF->RevID & REV_ID_MASK);
#endif

	BcmCoreDpcSyncEnter(SYNC_RX);
	AdslCoreIndicateLinkDown(0);
	
#ifdef SUPPORT_DSL_BONDING
	if (ADSL_PHY_SUPPORT(kAdslPhyBonding))
		AdslCoreIndicateLinkDown(1);
#endif
	BcmCoreDpcSyncExit(SYNC_RX);

	if (!adslOemDataSaveFlag)
		AdslCoreSaveOemData();

#if defined(CONFIG_VDSL_SUPPORTED) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 6) && !defined(CONFIG_BRCM_IKOS)
	if(IsLD6303VR5p3Used()) {
		int res;
		unsigned short bpGpio;
#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 11)
		res = BpGetAFEVR5P3PwrEnGpio(0, &bpGpio);
#else
		res = BpGetAFEVR5P3PwrEnGpio(&bpGpio);
#endif
		if(BP_SUCCESS == res)
			kerSysSetGpioState(bpGpio, kGpioInactive);
	}
#endif

	if (bCoreReset) {
		/* take ADSL core out of reset */
		nRet = RESET_ADSL_CORE;
		if(nRet)
			printk(" *** %s: RESET_ADSL_CORE error(%d)! ***\n", __FUNCTION__, nRet);

#if defined(CONFIG_BCM96328)
		if (0x632800a0 == PERF->RevID) {
			if(!XdslPhyClockAlign()) {
				AdslDrvPrintf(TEXT("AdslCoreHwReset:  Failed to align ADSL Phy clock\n"));
				return AC_FALSE;
			}
		}
#endif

#ifdef LMEM_ACCESS_WORKAROUND
		DISABLE_ADSL_MIPS;
		if( (0xa0 == chipRevId) || (0xb0 == chipRevId) ) {
			AdslCoreLoadImage();	/* Needed to get the SDRAM base address */
			AdslCoreRunPhyMipsInSDRAM();
		}
#endif
		if (!(tmp = AdslCoreLoadImage())) {
			AdslDrvPrintf(TEXT("AdslCoreHwReset:  Failed to load ADSL PHY image\n"));
			return AC_FALSE;
		}
	}
	
	{
	extern void FlattenBufferClearStat(void);
	FlattenBufferClearStat();
	}
	
	AdslCoreSetSdramTrueSize();
	AdslCoreSharedMemInit();
#ifdef CONFIG_PHY_PARAM
	AdslCoreProcessPhyParams();
#endif
	pAdslX = (AdslXfaceData *)(uintptr_t)ADSL_LMEM_XFACE_DATA;
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	AdslDrvPrintf(TEXT("%s: pAdslX=0x%px\n"), __FUNCTION__, pAdslX);	//TO DO: Remove after debugging
#endif
	BlockByteClear (sizeof(AdslXfaceData), (void *)pAdslX);
#if defined(SDRAM4G_SUPPORT1)
	tmp = ADSL_ENDIAN_CONV_INT32(pAdslLmem[5]);
	if (tmp & 1) {
	  pAdslLmem[5] = ADSL_ENDIAN_CONV_INT32(tmp | 2);
	  pAdslX->sdramBaseAddr = ADSL_ENDIAN_CONV_UINT32(adslCorePhyDesc.sdramImageAddr0);
	  adslCorePhyDesc.sdramPhyAddrMaskAny  = 0;
	  adslCorePhyDesc.sdramPhyConvAddr = adslCorePhyDesc.sdramPhyImageAddr | 0x20000000;
	}
	else {
	  pAdslX->sdramBaseAddr = ADSL_ENDIAN_CONV_UINT32(adslCorePhyDesc.sdramImageAddr0 | 0xA0000000);
	  adslCorePhyDesc.sdramPhyAddrMaskAny  = 0xA0000000;
	  adslCorePhyDesc.sdramPhyConvAddr = adslCorePhyDesc.sdramImageAddr0 | 0xA0000000;
	}
#else
	pAdslX->sdramBaseAddr = ADSL_ENDIAN_CONV_UINT32(adslCorePhyDesc.sdramImageAddr0 | 0xA0000000);
#endif
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	pAdslX->gfcTable[sizeof(pAdslX->gfcTable)/sizeof(pAdslX->gfcTable[0]) - 1] = ADSL_ENDIAN_CONV_INT32(adslCorePhyDesc.sdramImageAddr);
#if defined(CONFIG_ARM64)
	pAdslX->gfcTable[sizeof(pAdslX->gfcTable)/sizeof(pAdslX->gfcTable[0]) - 2] = ADSL_ENDIAN_CONV_INT32((adslCorePhyDesc.sdramImageAddr>>32));
	AdslDrvPrintf(TEXT("%s: pAdslX->sdramBaseAddr=0x%08x, pAdslX->gfcTable[]=0x%x%08x, adslCorePhyDesc.sdramImageAddr=0x%x%08x\n"), __FUNCTION__,
		(uint)pAdslX->sdramBaseAddr,
		(uint)pAdslX->gfcTable[sizeof(pAdslX->gfcTable)/sizeof(pAdslX->gfcTable[0]) - 2],
		(uint)pAdslX->gfcTable[sizeof(pAdslX->gfcTable)/sizeof(pAdslX->gfcTable[0]) - 1],
		(uint)(adslCorePhyDesc.sdramImageAddr>>32), (uint)adslCorePhyDesc.sdramImageAddr);
#else
	AdslDrvPrintf(TEXT("%s: pAdslX->sdramBaseAddr=0x%08x, pAdslX->gfcTable[]=0x%08x, adslCorePhyDesc.sdramImageAddr=0x%08x\n"), __FUNCTION__,
		(uint)pAdslX->sdramBaseAddr, (uint)pAdslX->gfcTable[sizeof(pAdslX->gfcTable)/sizeof(pAdslX->gfcTable[0]) - 1], (uint)adslCorePhyDesc.sdramImageAddr);	//TO DO: Remove after debugging
#endif
#endif
	for(tmp = 0; tmp < MAX_DSL_LINE; tmp++)
		acAhifStatePtr[tmp] = NULL;

	/* now take ADSL core MIPS out of reset */
#ifdef VXWORKS
	if (ejtagEnable) {
		int *p = (int *)0xfff0001c;
		*p = 1;
	}
#endif

	pAdslXface = (AdslXfaceData *) pAdslX;
	pAdslOemData = NULL;
	pPhySbSta = &pAdslXface->sbSta;
	pPhySbCmd = &pAdslXface->sbCmd;

#ifdef SUPPORT_STATUS_BACKUP
	if(NULL == pLocSbSta) {
		pLocSbSta = calloc(1, sizeof(stretchHostBufferStruct)+STAT_BKUP_BUF_SIZE+kMaxFlattenedStatusSize);
		if(pLocSbSta)
			FlattenHostBufferInit(pLocSbSta, pLocSbSta+1, STAT_BKUP_BUF_SIZE, kMaxFlattenedStatusSize);
		else
			AdslDrvPrintf(TEXT("AdslCoreHwReset:  Failed to calloc for pLocSbSta\n"));	/* No back up status buffer */
	}
	else
		FlattenHostBufferInit(pLocSbSta, pLocSbSta+1, STAT_BKUP_BUF_SIZE, kMaxFlattenedStatusSize);
	pCurSbSta = pPhySbSta;
#endif
	
	/* clear and enable interrupts */
#if !defined(_NOOS)
#if defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158A0)
	tmp = pAdslEnum[ADSL_Core2HostMsg];
#else
	pAdslEnum[ADSL_INTSTATUS_I] = pAdslEnum[ADSL_INTSTATUS_I];
#endif
#ifndef CONFIG_BRCM_IKOS
	tmp = pAdslEnum[ADSL_INTMASK_I];
	pAdslEnum[ADSL_INTMASK_I] = tmp | MSGINT_MASK_BIT;
#endif
#endif

#if defined(PHY_BLOCK_TEST) || defined(_NOOS)
	memMonitorIdIntialVal = pAdslLmem[MEM_MONITOR_ID];
#endif

#ifdef SUPPORT_XDSLDRV_GDB
	setGdbOn();
#endif

#if defined(USE_6306_CHIP)
	if(afe6306ChipUsed) {
		XdslCoreHwReset6306();
		SetupReferenceClockTo6306();
		XdslCoreHwReset6306();
#ifdef CONFIG_BCM96368	/* For the 63268, this will be done in PHY */
		PLLPowerUpSequence6306();
		XdslCoreHwReset6306();
#endif
#if defined(CONFIG_BCM963268) && defined(SUPPORT_DSL_BONDING)
		/* PHY control Line Driver mode internally, except for bonding PHY under 63268A0/63268B0 chip, will pass
		line1 LD control to the Host through a status.  For Diags downloaded PHY, put back the default(control from PHY)
		in case this downloaded PHY will control the LD mode internally, such as single line PHY */
		if( !bCoreReset && ((0xa0 == chipRevId) || (0xb0 == chipRevId)) )
			GPIO->GPIOBaseMode |= GPIO_BASE_VDSL_PHY_OVERRIDE_1;
#endif
	}
#endif /* USE_6306_CHIP */

#if defined(LMEM_ACCESS_WORKAROUND)
	if( (0xa0 == chipRevId) || (0xb0 == chipRevId) ) {
		DISABLE_ADSL_MIPS;
		pXdslControlReg[0] &= ~ALT_BOOT_VECTOR_MASK;
		pXdslControlReg[0] |= ALT_BOOT_VECTOR_LMEM;
	}
#endif
#if defined(PHY_BLOCK_TEST) && defined(CONFIG_BRCM_IKOS)
	initialLmemVal = pAdslLmem[51];
#endif
#if defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158A0)
	/* reset VDSl PMD */
	tmp = pAdslEnum[(0x50 / 4)];
	pAdslEnum[(0x50 / 4)] = tmp & ~0x7E;
	bcmOsGetTime(&osTime0);
	do { } while (0 == OsTimeElapsedMs(osTime0));
	pAdslEnum[(0x50 / 4)] = tmp | 0x7E;
#endif
	nRet = ENABLE_ADSL_MIPS;
	if(nRet)
		printk(" *** %s: ENABLE_ADSL_MIPS error(%d)! ***\n", __FUNCTION__, nRet);
	
#ifdef VXWORKS
	if (ejtagEnable) {
		int read(int fd, void *buf, int n);

		int *p = (int *)0xfff0001c;
		char ch[16];
		printf("Enter any key (for EJTAG)\n");
		read(0, ch, 1);
		*p = 0;
	}
#endif

	/* wait for ADSL core to initialize */
	bcmOsGetTime(&osTime0);
	osTimePhyInitStart = osTime0;
	to = OsTimeElapsedMs(osTime0);
#ifdef STAT_HANDLING_PRINT
	AdslDrvPrintf(TEXT("*** %s: osTimePhyInitStart=0x%lX ***\n"), __FUNCTION__, osTimePhyInitStart);
#endif
#if defined(PHY_BLOCK_TEST) || defined(_NOOS)
	AdslDrvPrintf(TEXT("**** BLOCK TEST BUILD, &adslPhyXfaceOffset=0x%px (0x%X) ***\n"), &adslPhyXfaceOffset, (uint)adslPhyXfaceOffset);
#ifdef CONFIG_BRCM_IKOS
	AdslDrvPrintf(TEXT("*** osTimePhyInitStart=0x%lX &pAdslLmem[51]=%px pAdslLmem[51] = 0x%08x ***\n"),
		osTimePhyInitStart, &pAdslLmem[51], initialLmemVal);
#endif
	while (!AdsCoreStatBufAssigned() && ((to = OsTimeElapsedMs(osTime0)) < HW_RESET_DELAY_MS) &&
		((memMonitorIdIntialVal==pAdslLmem[MEM_MONITOR_ID] ) || (0==pAdslLmem[MEM_MONITOR_ID]))) {
#if 0 && defined(CONFIG_BRCM_IKOS)
		if(pAdslLmem[51] != initialLmemVal)
			AdslDrvPrintf(TEXT("*** pAdslLmem[51] = 0x%08x ***\n"), pAdslLmem[51]);
#endif
	}
	if(to < HW_RESET_DELAY_MS)
		nRet = OS_STATUS_OK;
	else
		nRet = OS_STATUS_TIMEOUT;
#else
	bcmOsGetTime(&osTime1);
	nRet = OS_STATUS_OK;
	do {
		if(AdsCoreStatBufAssigned())
			break;
		if(OS_STATUS_OK == nRet) {
			sema_init((struct semaphore *)syncPhyMipsSemId, 0);
			nRet = bcmOsSemTake(syncPhyMipsSemId, osTimeOut - (osTime1-osTime0));
		}
		bcmOsGetTime(&osTime1);
	} while((osTime1-osTime0) < osTimeOut);
	to = (osTime1 - osTimePhyInitStart) * BCMOS_MSEC_PER_TICK;
#endif
#ifdef STAT_HANDLING_PRINT
	AdslDrvPrintf(TEXT("*** %s: HW reset delayed %lu ms, nRet(%d) ***\n"), __FUNCTION__, to, (int)nRet);
#endif
	DiagWriteString(0, DIAG_DSL_CLIENT, "%s: HW reset delayed %lu ms, nRet(%d)\n", __FUNCTION__, to, (int)nRet);

#ifdef PHY_BLOCK_TEST
	if ((memMonitorIdIntialVal==pAdslLmem[MEM_MONITOR_ID]) || (0==pAdslLmem[MEM_MONITOR_ID]))	/* GLOBAL_EVENT_ZERO_BSS */
#else
	if (!AdsCoreStatBufAssigned())
#endif
	{
#ifdef PHY_BLOCK_TEST
		if(AdsCoreStatBufAssigned())
			AdslDrvPrintf(TEXT("AdslCoreHwReset:  Non-blocktest PHY is running\n"));
		else
			AdslDrvPrintf(TEXT("AdslCoreHwReset:  No PR received from PHY in %lu ms\n"), to);
		AdslDrvPrintf(TEXT("AdslCoreHwReset:  pAdslLmem[MEM_MONITOR_ID]=0x%X\n"), pAdslLmem[MEM_MONITOR_ID]);
#else
		AdslDrvPrintf(TEXT("AdslCoreHwReset:  ADSL PHY initialization timeout %lu ms\n"), to);
		DiagStrPrintf(0,DIAG_DSL_CLIENT, "AdslCoreHwReset:  ADSL PHY initialization timeout %lu ms\n", to);
		
		DISABLE_ADSL_MIPS;
		return AC_FALSE;
#endif
	}

#ifdef SUPPORT_STATUS_BACKUP
#ifdef PHY_BLOCK_TEST
	if(AdsCoreStatBufAssigned())	/* Non-blocktest PHY is running */
		gStatusBackupThreshold = MIN( STAT_BKUP_THRESHOLD, (StretchBufferGetSize(pPhySbSta) >> 1));
#else
	gStatusBackupThreshold = MIN( STAT_BKUP_THRESHOLD, (StretchBufferGetSize(pPhySbSta) >> 1));
#endif
	AdslDrvPrintf(TEXT("AdslCoreHwReset:  pLocSbSta=%px bkupThreshold=%d\n"), pLocSbSta, gStatusBackupThreshold);
#endif

#ifdef PHY_BLOCK_TEST
	if(AdsCoreStatBufAssigned())	/* Non-blocktest PHY is running */
#endif
	{
		dslStatusStruct status;
		dslStatusCode	statusCode;
		
		bcmOsGetTime(&osTime0);
		do {
			status.code = 0;
			statusCode = 0;
			if ((tmp = AdslCoreStatusRead (&status)) != 0) {
#ifdef CONFIG_BRCM_IKOS
				AdslDrvPrintf(TEXT("AdslCoreHwReset:  pStart=0x%lX, pRead=0x%lX, pWrite=0x%lX, readLen=%d\n"),
					StretchBufferGetStartPtr(pPhySbSta), StretchBufferGetReadPtr(pPhySbSta),
					StretchBufferGetWritePtr(pPhySbSta), tmp);
#endif
				statusCode = DSL_STATUS_CODE(status.code);
				AdslCoreStatusReadComplete (tmp);
#ifdef CONFIG_BRCM_IKOS
				AdslDrvPrintf(TEXT("AdslCoreHwReset:  pStart=0x%lX, pRead=0x%lX, pWrite=0x%lX\n"),
					StretchBufferGetStartPtr(pPhySbSta), StretchBufferGetReadPtr(pPhySbSta), StretchBufferGetWritePtr(pPhySbSta));
#endif
				if (kDslOemDataAddrStatus == statusCode) {
					AdslDrvPrintf(TEXT("AdslCoreHwReset:  AdslOemDataAddr = 0x%X, time=%lu ms\n"), (uint)status.param.value, OsTimeElapsedMs(osTimePhyInitStart));
					AdslCoreProcessOemDataAddrMsg(&status);
					break;
				}
			}
		} while (OsTimeElapsedMs(osTime0) < HW_RESET_DELAY_MS);
		
		if (kDslOemDataAddrStatus != statusCode) {
			to = OsTimeElapsedMs(osTimePhyInitStart);
			AdslDrvPrintf(TEXT("AdslCoreHwReset:  ADSL PHY OemData status read timeout %lu ms\n"), to);
			DiagStrPrintf(0,DIAG_DSL_CLIENT, "AdslCoreHwReset:  ADSL PHY OemData status read timeout %lu ms\n", to);
			DISABLE_ADSL_MIPS;
			return AC_FALSE;
		}
#if defined(CONFIG_BCM963138) &&  defined(BOARD_H_API_VER) &&   (BOARD_H_API_VER > 8)
		if((0xb0 == chipRevId) && ADSL_PHY_SUPPORT(kAdslPhyDualMips))
			pmc_dsl_mipscore_enable(1, 1);
#endif
#ifdef SUPPORT_XDSLDRV_GDB
		BcmAdslCoreGdbTask();
#endif
	}

#if !defined(CONFIG_BRCM_IKOS) && !defined(_NOOS)
#ifdef PHY_BLOCK_TEST
	if(AdsCoreStatBufAssigned())	/* Non-blocktest PHY is running */
#endif
	{
#ifdef SUPPORT_MULTI_PHY
		if(BcmXdslCoreMediaSearchInInitState())
			BcmXdslCoreMediaSearchSM(MEDIASEARCH_START_E, 0);
#if defined(CONFIG_BCM963268)
		BcmXdslCorePrintCurrentMedia();
#endif
#endif
		AdslCoreSendFilterSNRMarginCmd();
		BcmXdslCoreSendAfeInfo(1);	/* Send Afe info to PHY */
#if !defined(__ECOS)
		BcmAdslDiagSendHdr();		/* Send DslDiag Hdr to PHY */
#endif /* !defined(__ECOS) */
		if (DIAG_DEBUG_CMD_LOG_DATA == diagDebugCmd.cmd) {
			BcmAdslCoreDiagStartLog_2(0, diagDebugCmd.param1, diagDebugCmd.param2);
			diagDebugCmd.cmd = 0;
		}
	}
	BcmXdslCoreDiagSendPhyInfo();	/* Send PHY info to DslDiag */
#endif /* !defined(CONFIG_BRCM_IKOS) && !defined(_NOOS) */

#endif /* BCM_CORE_NO_HARDWARE */

	return AC_TRUE;
}

AC_BOOL AdslCoreHwReset(AC_BOOL bCoreReset)
{
	int		cnt = 0;
#if defined(CONFIG_BCM_DSL_GFAST) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 10)
	int res;
#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)) && (BOARD_H_API_VER > 11)
	res = BpGetAFELDPwrBoostGpio(0, &bpGpioAFEPwrBoost[0]);
#else
	res = BpGetAFELDPwrBoostGpio(&bpGpioAFEPwrBoost[0]);
#endif
	acPwrBoostLineMask = 0;
	if(BP_SUCCESS == res) {
		kerSysSetGpio(bpGpioAFEPwrBoost[0], kGpioInactive);
		DiagWriteString(0, DIAG_DSL_CLIENT, "*** Drv: De-activate AFE LD power boost (bpGpio 0x%04X)\n", bpGpioAFEPwrBoost[0]);
#if defined(CONFIG_BCM963158) && (BOARD_H_API_VER > 11) && defined(SUPPORT_DSL_BONDING)
		res = BpGetAFELDPwrBoostGpio(1, &bpGpioAFEPwrBoost[1]);
		if(BP_SUCCESS == res) {
			if (bpGpioAFEPwrBoost[0] != bpGpioAFEPwrBoost[1])
				kerSysSetGpio(bpGpioAFEPwrBoost[1], kGpioInactive);
		}
		else
			bpGpioAFEPwrBoost[1] = 0xFFFF;
#endif
	}
	else {
		bpGpioAFEPwrBoost[0] = 0xFFFF;
		DiagWriteString(0, DIAG_DSL_CLIENT, "*** Drv: AFE LD power boost GPIO is not configured\n");
	}
#endif

	do {
		if (__AdslCoreHwReset(bCoreReset)) {
#if defined(XTM_NOTIFY_CORE_RESET)
			{
			XTM_INTERFACE_LINK_INFO	linkInfo;

			linkInfo.ulLinkState = LINK_DOWN;
			linkInfo.ulLinkUsRate =	0;
			linkInfo.ulLinkDsRate =	0;
			linkInfo.ulLinkTrafficType = TRAFFIC_TYPE_ATM;
			BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID(0), &linkInfo);
			}
#endif
			return AC_TRUE;
		}
#if defined(_NOOS) || defined(CONFIG_BRCM_IKOS)
		else {
#ifdef _NOOS
			stop_TEST(get_cpuid());
#elif defined(XDSL_DRV_STATUS_POLLING)
			extern void stop_TEST(unsigned int cpuId);
			extern unsigned int get_cpuid(void);
			stop_TEST(get_cpuid());
#endif
			return AC_FALSE;
		}
#endif
		cnt++;
	} while (cnt < 3);

	if(cnt == 3) {
		AdslDrvPrintf(TEXT("Error: DSL PHY initialization failed.\n"));
#if defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158B0)
		AdslDrvPrintf(TEXT("Try Power-On Reset\n"));
#endif
	}
	
	return AC_FALSE;
}

AC_BOOL AdslCoreInit(void)
{
	dslVarsStruct	*pDslVars;
	unsigned char	lineId;
	
#ifndef BCM_CORE_NO_HARDWARE
	if (NULL != pAdslXface)
		return AC_FALSE;
#endif

	for(lineId = 0; lineId < MAX_DSL_LINE; lineId++) {
	pDslVars = (dslVarsStruct *)XdslCoreGetDslVars(lineId);
	pDslVars->lineId = lineId;
	pDslVars->xdslCoreOvhMsgPrintEnabled = AC_FALSE;
	pDslVars->timeInL2Ms = 0;
	pDslVars->xdslCoreShMarginMonEnabled = AC_FALSE;
	pDslVars->xdslCoreLOMTimeout = (uint)-1;
	pDslVars->xdslCoreLOMTime = -1;
	pDslVars->dataPumpCommandHandlerPtr = AdslCoreCommonCommandHandler;
	pDslVars->externalStatusHandlerPtr  = AdslCoreStatusHandler;
	DslFrameAssignFunctions (pDslVars->DslFrameFunctions, DslFrameNative);
	AdslCoreAssignDslFrameFunctions (pDslVars->DslFrameFunctions);
	acAhifStatePtr[lineId] = NULL;
	
#ifdef G997_1_FRAMER
	{
		upperLayerFunctions	g997UpperLayerFunctions;

		AdslCoreG997Init(lineId);
		g997UpperLayerFunctions.rxIndicateHandlerPtr = AdslCoreG997IndicateRecevice;
		g997UpperLayerFunctions.txCompleteHandlerPtr = AdslCoreG997SendComplete;
		g997UpperLayerFunctions.statusHandlerPtr = AdslCoreG997StatusHandler;
#ifdef CONFIG_BCM_DSL_GFAST
		G997Init((void *)pDslVars,0, 64, 128, 16, &g997UpperLayerFunctions, AdslCoreG997CommandHandler);
#else
		G997Init((void *)pDslVars,0, 32, 128, 8, &g997UpperLayerFunctions, AdslCoreG997CommandHandler);
#endif
	}
#endif

#ifdef ADSL_MIB
	AdslMibInit((void *)pDslVars,AdslCoreCommonCommandHandler);
	AdslMibSetNotifyHandler((void *)pDslVars, AdslCoreMibNotify);
#endif

#ifdef G992P3
#ifdef G992P3_DEBUG
	G992p3OvhMsgInit((void *)pDslVars, 0, G997ReturnFrame, TstG997SendFrame, __AdslCoreG997SendComplete, AdslCoreCommonCommandHandler, AdslCoreCommonStatusHandler);
#else
	G992p3OvhMsgInit((void *)pDslVars, 0, G997ReturnFrame, G997SendFrame, __AdslCoreG997SendComplete, AdslCoreCommonCommandHandler, AdslCoreCommonStatusHandler);
#endif
#endif

#ifdef ADSL_SELF_TEST
	{
	uint	stRes;

	AdslSelfTestRun(adslCoreSelfTestMode);
	stRes = AdslCoreGetSelfTestResults();
	AdslDrvPrintf (TEXT("AdslSelfTestResults = 0x%X %s\n"), stRes, 
			(stRes == (kAdslSelfTestCompleted | adslCoreSelfTestMode)) ? "PASSED" : "FAILED");
	}
#endif
	}
	
	AdslCoreSetPllClock();
#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)) && defined(BOARD_H_API_VER) && (BOARD_H_API_VER > 12)
	{
	uint afe_reg0, bg_bias0;
	volatile uint *pPhyRegAddr = (volatile uint *)DSLPHY_BASE;
	kerSysGetDyingGaspConfig(&afe_reg0, &bg_bias0);
#if defined(CONFIG_BCM963178)
	afe_reg0 &= ~0x8;
#endif
	AdslDrvPrintf (TEXT("%s: afe_reg0=0x%X bg_bias0=0x%X\n"), __FUNCTION__, afe_reg0, bg_bias0);
	pPhyRegAddr[BG_BIAS0_OFFSET] = bg_bias0;
	pPhyRegAddr[AFE_REG0_OFFSET] = afe_reg0;
	kerSysDisableDyingGaspInterrupt();
	kerSysDisableDyingGaspOverride();
	kerSysEnableDyingGaspInterrupt();
	}
#endif

	return AdslCoreHwReset(AC_TRUE);
}

void AdslCoreUninit(void)
{
#ifdef G997_1_FRAMER
	void *pDslVars;
	unsigned char lineId;
#endif
#ifndef BCM_CORE_NO_HARDWARE
	DISABLE_ADSL_MIPS;
	DISABLE_ADSL_CORE;
#endif

#ifdef G997_1_FRAMER
	for (lineId = 0; lineId < MAX_DSL_LINE; lineId++) {
		pDslVars = XdslCoreGetDslVars(lineId);
		G997Close(pDslVars);
	}
#endif

#ifdef SUPPORT_STATUS_BACKUP
	if(NULL != pLocSbSta) {
		free(pLocSbSta);
		pLocSbSta = NULL;
	}
#endif
	pAdslXface = NULL;
#if defined(USE_PMC_API) && !defined(CONFIG_BCM963178)
	pmc_dsl_power_down();
#endif
}

AC_BOOL AdslCoreSetSDRAMBaseAddr(void *pAddr)
{
	if (NULL != pAdslXface)
		return AC_FALSE;
#if (defined(CONFIG_ARM) || defined(CONFIG_ARM64)) && !defined(_NOOS)
	adslCorePhyDesc.sdramPageAddr0 = ((phys_addr_t) pAddr) - (ADSL_PHY_SDRAM_BIAS & ~0xFFFFF); /* pAddr is 1MB aligned */
#if defined(BOARD_H_API_VER) && BOARD_H_API_VER > 14
	{
		void* virt_base = NULL;
		phys_addr_t phys_base = (phys_addr_t)pAddr;

		BcmMemReserveGetByName(ADSL_BASE_ADDR_STR, &virt_base, NULL, NULL);
		adslCorePhyDesc.sdramPageAddr = (uintptr_t)BcmMemReservePhysToVirt(virt_base, phys_base, adslCorePhyDesc.sdramPageAddr0);
#ifdef ADSL_SDRAM_RESERVE_SIZE
		AdslCoreSystemReservedMemInit(virt_base, pAddr, ADSL_SDRAM_RESERVE_SIZE - ADSL_SDRAM_IMAGE_SIZE);
		AdslDrvPrintf(TEXT("%s: Orig: pAddr=0x%px vBase=0x%px pg0=0x%X pg1=0x%lX\n"), __FUNCTION__, pAddr, virt_base, 
			adslCorePhyDesc.sdramPageAddr0, adslCorePhyDesc.sdramPageAddr);
		pAddr = ((char *) pAddr) + ADSL_SDRAM_RESERVE_SIZE - ADSL_SDRAM_IMAGE_SIZE;
		adslCorePhyDesc.sdramPageAddr0 += ADSL_SDRAM_RESERVE_SIZE - ADSL_SDRAM_IMAGE_SIZE;
		adslCorePhyDesc.sdramPageAddr  += ADSL_SDRAM_RESERVE_SIZE - ADSL_SDRAM_IMAGE_SIZE;
		AdslDrvPrintf(TEXT("%s: Res: pAddr=0x%px vBase=0x%px pg0=0x%X pg1=0x%lX\n"), __FUNCTION__, pAddr, virt_base, 
			adslCorePhyDesc.sdramPageAddr0, adslCorePhyDesc.sdramPageAddr);
#endif
	}
#else
	adslCorePhyDesc.sdramPageAddr0 = ((phys_addr_t) pAddr) & ~(adslCorePhyDesc.sdramPageSize-1);
	adslCorePhyDesc.sdramPageAddr = (uintptr_t)phys_to_virt(adslCorePhyDesc.sdramPageAddr0);
#endif
	AdslDrvPrintf(TEXT("%s: pAddr=0x%px sdramPageAddr=0x%lX\n"), __FUNCTION__, pAddr, adslCorePhyDesc.sdramPageAddr);
#else
	adslCorePhyDesc.sdramPageAddr = 0xA0000000 | (((uintptr_t) pAddr) - ADSL_PHY_SDRAM_BIAS);
	adslCorePhyDesc.sdramPageAddr0 = (uint)adslCorePhyDesc.sdramPageAddr;
#endif

	return AC_TRUE;
}

void * AdslCoreGetSDRAMBaseAddr()
{
	return (void *) adslCorePhyDesc.sdramPageAddr;
}

AC_BOOL AdslCoreSetVcEntry (int gfc, int port, int vpi, int vci, int pti_clp)
{
	dslCommandStruct	cmd;

	if ((NULL == pAdslXface) || (gfc <= 0) || (gfc > 15))
		return AC_FALSE;

#ifndef  ADSLDRV_LITTLE_ENDIAN
	pAdslXface->gfcTable[gfc-1] = (vpi << 20) | ((vci & 0xFFFF) << 4) | (pti_clp & 0xF);
#else
	{
	uint	tmp = (vpi << 20) | ((vci & 0xFFFF) << 4) | (pti_clp & 0xF);
	pAdslXface->gfcTable[gfc-1] = ADSL_ENDIAN_CONV_INT32(tmp);
	}
#endif

	cmd.command = kDslAtmVcMapTableChanged;
	cmd.param.value = gfc;
	AdslCoreCommandHandler(&cmd);

	return AC_TRUE;
}

void AdslCoreSetStatusCallback(void *pCallback)
{
	if (NULL == pCallback)
		pAdslCoreStatusCallback = AdslCoreIdleStatusCallback;
	else
		pAdslCoreStatusCallback = pCallback;
}

void *AdslCoreGetStatusCallback(void)
{
	return (AdslCoreIdleStatusCallback == pAdslCoreStatusCallback ? NULL : pAdslCoreStatusCallback);
}

void AdslCoreSetShowtimeMarginMonitoring(uchar lineId, uchar monEnabled, int showtimeMargin, int lomTimeSec)
{
	void *pDslVars = XdslCoreGetDslVars(lineId);
	
	AdslMibSetShowtimeMargin(pDslVars, showtimeMargin);
	gXdslCoreShMarginMonEnabled(pDslVars) = (monEnabled != 0);
	gXdslCoreLOMTimeout(pDslVars) = ((uint) lomTimeSec) * 1000;
}

void XdslCoreG997FrameBufferFlush(void *gDslVars, ac997FrameCtrl *g997FrCtrl)
{
#if 0
	char* frameReadPtr;
	int bufLen;
	uchar lineId = gLineId(gDslVars);
	
	frameReadPtr=(char*)AdslCoreG997FrameGet(lineId, &bufLen);
	
	while(frameReadPtr!=NULL){
		AdslCoreG997ReturnFrame(gDslVars);
		frameReadPtr=(char*)AdslCoreG997FrameGet(lineId, &bufLen);
		BcmAdslCoreDiagWriteStatusString(gLineId(gDslVars), "Line %d: G997Frame returned to free-pool as it was not being read", lineId);
	}
#endif
	while(CircBufferGetReadAvail(&gG997RxFrCB(g997FrCtrl)) != NULL) {
		dslFrame *pFrame = *(dslFrame **)CircBufferGetReadPtr(&gG997RxFrCB(g997FrCtrl));
		gG997RxCurBuf(g997FrCtrl) = DslFrameGetFirstBuffer(gDslVars, pFrame);
		AdslCoreG997ReturnFrame(gDslVars, g997FrCtrl);
		BcmAdslCoreDiagWriteStatusString(gLineId(gDslVars), "G997Frame returned to free-pool as it was not being read, g997FrCtrl=0x%08x", (uint)(ulong)g997FrCtrl);
	}
	gTimeUpdate(g997FrCtrl) = 0;
	gPendingFrFlag(g997FrCtrl) = 0;
}


void AdslCoreUpdateG997andOvhMsgTimer(ulong tMs)
{
	void	*pDslVars;
	
#ifdef G997_1_FRAMER
	G997UpdateTimer(XdslCoreGetDslVars(0), tMs);
#ifdef SUPPORT_DSL_BONDING
	if (ADSL_PHY_SUPPORT(kAdslPhyBonding))
		G997UpdateTimer(XdslCoreGetDslVars(1), tMs);
#endif
#endif

#ifdef G992P3
	pDslVars = XdslCoreGetDslVars(0);
	if (AdslCoreOvhMsgSupported(pDslVars))
		G992p3OvhMsgUpdateTimer(pDslVars, tMs);
#ifdef SUPPORT_DSL_BONDING
	if(ADSL_PHY_SUPPORT(kAdslPhyBonding)) {
		pDslVars = XdslCoreGetDslVars(1);
		if (AdslCoreOvhMsgSupported(pDslVars))
			G992p3OvhMsgUpdateTimer(pDslVars, tMs);
	}
#endif
#endif
}

void XdslCoreFlushAllG997ReceivedFrame(void *gDslVars)
{
	ac997FrameCtrl *g997FrCtrl;
	g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, BCM_XDSL_NSF_EOC_MSG);
	if(NULL != g997FrCtrl)
		XdslCoreG997FrameBufferFlush(gDslVars, g997FrCtrl);
	g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, BCM_XDSL_CLEAR_EOC_MSG);
	if(NULL != g997FrCtrl)
		XdslCoreG997FrameBufferFlush(gDslVars, g997FrCtrl);
	g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, BCM_XDSL_DATAGRAM_EOC_MSG);
	if(NULL != g997FrCtrl)
		XdslCoreG997FrameBufferFlush(gDslVars, g997FrCtrl);
}

static void XdslCoreFlushExpiredG997ReceivedFrame(void *gDslVars, ulong tMs)
{
	ac997FrameCtrl *g997FrCtrl;
	g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, BCM_XDSL_NSF_EOC_MSG);
	if((NULL != g997FrCtrl) && gPendingFrFlag(g997FrCtrl)) {
		gTimeUpdate(g997FrCtrl) += tMs;
		if (gTimeUpdate(g997FrCtrl) > 5000)
			XdslCoreG997FrameBufferFlush(gDslVars, g997FrCtrl);
	}
	g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, BCM_XDSL_CLEAR_EOC_MSG);
	if((NULL != g997FrCtrl) && gPendingFrFlag(g997FrCtrl)) {
		gTimeUpdate(g997FrCtrl) += tMs;
		if (gTimeUpdate(g997FrCtrl) > 5000)
			XdslCoreG997FrameBufferFlush(gDslVars, g997FrCtrl);
	}
	g997FrCtrl = XdslCoreG997GetEocMsgCtrl(gDslVars, BCM_XDSL_DATAGRAM_EOC_MSG);
	if((NULL != g997FrCtrl) && gPendingFrFlag(g997FrCtrl)) {
		gTimeUpdate(g997FrCtrl) += tMs;
		if (gTimeUpdate(g997FrCtrl) > 5000)
			XdslCoreG997FrameBufferFlush(gDslVars, g997FrCtrl);
	}
}

void AdslCoreTimer (ulong tMs)
{
	int		i;
	void		*pDslVars;
	
	BcmCoreDpcSyncEnter(SYNC_RX);	// Need to synchronize with AdslCoreG997SendData()
	for(i = 0; i < MAX_DSL_LINE; i++) {
#ifdef SUPPORT_DSL_BONDING
		if ((1 == i) && !ADSL_PHY_SUPPORT(kAdslPhyBonding))
			break;
#endif
		pDslVars = XdslCoreGetDslVars((uchar)i);
		XdslCoreFlushExpiredG997ReceivedFrame(pDslVars, tMs);
	}
	
	if (pSdramReserved->timeCnt < ADSL_INIT_TIME) {
		pSdramReserved->timeCnt += tMs;
		if (pSdramReserved->timeCnt >= ADSL_INIT_TIME) {
			dslCommandStruct	cmd;

			adslCoreEcUpdateMask |= kDigEcShowtimeFastUpdateDisabled;
			BcmAdslCoreDiagWriteStatusString (0, "AdslCoreEcUpdTmr: timeMs=%d ecUpdMask=0x%X\n", pSdramReserved->timeCnt, adslCoreEcUpdateMask);

			cmd.command = kDslSetDigEcShowtimeUpdateModeCmd;
			cmd.param.value = kDslSetDigEcShowtimeUpdateModeSlow;
			AdslCoreCommandHandler(&cmd);
		}
	}
	
#ifdef G997_1_FRAMER
	G997Timer(XdslCoreGetDslVars(0));
#ifdef SUPPORT_DSL_BONDING
	if (ADSL_PHY_SUPPORT(kAdslPhyBonding))
		G997Timer(XdslCoreGetDslVars(1));
#endif
#endif

#ifdef ADSL_MIB
	for(i = 0; i < MAX_DSL_LINE; i++) {
#ifdef SUPPORT_DSL_BONDING
		if ((1 == i) && !ADSL_PHY_SUPPORT(kAdslPhyBonding))
			break;
#endif
		pDslVars = XdslCoreGetDslVars((uchar)i);
		AdslMibTimer(pDslVars, tMs);
		if (gXdslCoreLOMTime(pDslVars) >= 0)
			gXdslCoreLOMTime(pDslVars) += tMs;
	}
#endif

#ifdef G992P3
	pDslVars = XdslCoreGetDslVars(0);
	if (AdslCoreOvhMsgSupported(pDslVars))
		G992p3OvhMsgTimer(pDslVars);
#ifdef SUPPORT_DSL_BONDING
	if(ADSL_PHY_SUPPORT(kAdslPhyBonding)) {
		pDslVars = XdslCoreGetDslVars(1);
		if (AdslCoreOvhMsgSupported(pDslVars))
			G992p3OvhMsgTimer(pDslVars);
	}
#endif
#endif

	for(i = 0; i < MAX_DSL_LINE; i++) {
#ifdef SUPPORT_DSL_BONDING
		if ((1 == i) && !ADSL_PHY_SUPPORT(kAdslPhyBonding))
			break;
#endif
		pDslVars = XdslCoreGetDslVars((uchar)i);
		if( 2 == AdslMibPowerState(pDslVars) ) {
			gTimeInL2Ms(pDslVars) += tMs;
			if(gTimeInL2Ms(pDslVars) >= gL2SwitchL0TimeMs) {
#ifdef DEBUG_L2_RET_L0
				ulong cTime;
				bcmOsGetTime(&cTime);
				printk("line: %d L2 -> L0: %d Sec\n", i, ((cTime-gL2Start)*BCMOS_MSEC_PER_TICK)/1000);
#endif			
				AdslCoreSetL0((uchar)i);
				gTimeInL2Ms(pDslVars) = 0;
			}
		}
	}
	
	BcmCoreDpcSyncExit(SYNC_RX);
}

#if 0
AC_BOOL AdslCoreIntHandler(void)
{
#ifndef BCM_CORE_NO_HARDWARE
	{
		volatile uint	*pAdslEnum = (uint *) XDSL_ENUM_BASE;
		uint	tmp;
#ifdef CONFIG_BCM963158
		uint	tmp1;
#endif
		tmp = pAdslEnum[ADSL_INTSTATUS_I];
#ifdef CONFIG_BCM963158
		tmp1 = pAdslEnum[ADSL_Core2HostMsg];
#else
		pAdslEnum[ADSL_INTSTATUS_I] = tmp;
#endif
		if( !(tmp & MSGINT_BIT) )
			return 0;
	}
#endif

	return AdslCoreStatusAvail ();
}
#endif

static ulong	lastStatTimeMs = 0;
#ifdef SDRAM_HOLD_COUNTERS
static ulong	lastSdramHoldTimeMs = 0;
#endif

void AdslCoreSetTime(ulong timeMs)
{
	lastStatTimeMs = timeMs;
}

ulong AdslCoreGetCurrentTime(void)
{
	OS_TICKS curTick;
	bcmOsGetTime(&curTick);
	return curTick * BCMOS_MSEC_PER_TICK;
}

void Adsl2UpdateCOErrCounter(dslStatusStruct *status)
{
  adslMibInfo *pMib;
  long mibLen;
  uint *counters=(uint *)( status->param.dslConnectInfo.buffPtr);

  mibLen=sizeof(adslMibInfo);
  pMib=(void *) AdslMibGetObjectValue(XdslCoreGetCurDslVars(),NULL,0,NULL,&mibLen);
  counters[kG992ShowtimeNumOfFEBE]=ADSL_ENDIAN_CONV_INT32(pMib->adslStat.xmtStat.cntSFErr);
  counters[kG992ShowtimeNumOfFECC]=ADSL_ENDIAN_CONV_INT32(pMib->adslStat.xmtStat.cntRSCor);
  counters[kG992ShowtimeNumOfFHEC]=ADSL_ENDIAN_CONV_INT32(pMib->atmStat.xmtStat.cntHEC);
}

#if defined(SUPPORT_STATUS_BACKUP) || defined(ADSLDRV_STATUS_PROFILING)
extern int	gByteProc;
#endif
#ifdef SUPPORT_VECTORING
extern void BcmXdslSendErrorSamples(unsigned char lineId, VectorErrorSample *pVectErrorSample);
extern unsigned int BcmXdslGetVectExtraSkbCnt(void);
#endif

#ifdef DETECT_EXCESSIVE_INTR
void printLMEMContent(void)
{
	stretchBufferStruct *fBuf = pPhySbSta;
	printk(" pRead=%x pWrite=%x pStretchEnd=%x(%px %px %px)\n",
		ADSL_ENDIAN_CONV_INT32(fBuf->pRead), ADSL_ENDIAN_CONV_INT32(fBuf->pWrite), ADSL_ENDIAN_CONV_INT32(fBuf->pStretchEnd),
		pCurSbSta, pPhySbSta, pLocSbSta);
#if 0
	fBuf = pPhySbCmd;
	printk("Cmd: pStart=0x%08x pEnd=0x%08x pExtraEnd=0x%08x pStretchEnd=0x%08x pRead=0x%08x pWrite=0x%08x\n",
		ADSL_ENDIAN_CONV_INT32(fBuf->pStart), ADSL_ENDIAN_CONV_INT32(fBuf->pEnd),
		ADSL_ENDIAN_CONV_INT32(fBuf->pEnd), ADSL_ENDIAN_CONV_INT32(fBuf->pStretchEnd),
		ADSL_ENDIAN_CONV_INT32(fBuf->pRead), ADSL_ENDIAN_CONV_INT32(fBuf->pWrite));
#endif
}
#endif

int AdslCoreIntTaskHandler(int nMaxStatus)
{
	ulong			tMs, curTimeMs;
	dslStatusStruct	status;
	dslStatusCode	statusCode;
	int				mibRetCode = 0, nStatProcessed = 0, tmp;
	void			*gDslVars;
	uchar			lineId;

	tMs = lastStatTimeMs;
	
	if (!AdslCoreCommandIsPending() && (0 == gSharedMemAllocFromUserContext))
		AdslCoreSharedMemFree(NULL);
	
	curTimeMs = AdslCoreGetCurrentTime();
	if(curTimeMs != tMs)
		AdslCoreUpdateG997andOvhMsgTimer(curTimeMs - tMs);
	
	while ((tmp = AdslCoreStatusRead (&status)) > 0) {
#if defined(SUPPORT_STATUS_BACKUP) || defined(ADSLDRV_STATUS_PROFILING)
		gByteProc += tmp;
#endif
		statusCode = DSL_STATUS_CODE(status.code);
#ifdef SUPPORT_DSL_BONDING
		lineId = DSL_LINE_ID(status.code);
		gDslVars = XdslCoreSetCurDslVars(lineId);
#else
		lineId = 0;
		gDslVars = XdslCoreGetCurDslVars();
#endif
		nStatProcessed++;
#if defined(__KERNEL__)
		if(kDslResetPhyReqStatus == statusCode) {
			BcmAdslCoreResetPhy(status.param.value);
			AdslCoreStatusReadComplete (tmp);
			return nStatProcessed;
		}
		else if( kDslRequestDrvConfig == statusCode) {
			XdslCoreProcessDrvConfigRequest(lineId, ADSL_ADDR_TO_HOST(status.param.value));
			AdslCoreStatusReadComplete (tmp);
			continue;
		}
		else if (kDslEscapeToG994p1Status == statusCode)
			acCmdWakeup = AC_FALSE;
		else if (kDslWakeupRequest == statusCode)
			acCmdWakeup = AC_TRUE;
#ifdef CONFIG_BCM_PWRMNGT_DDR_SR_API
		else if( kDslPwrMgrSrAddrStatus == statusCode )
			BcmPwrMngtRegisterLmemAddr(ADSL_ADDR_TO_HOST(status.param.value));
#endif
#if defined(SUPPORT_VECTORING) && !defined(_NOOS) && !defined(CONFIG_BRCM_IKOS)
		else if( (kDslReceivedEocCommand == statusCode) && (kDslVectoringErrorSamples == status.param.dslClearEocMsg.msgId) ) {
			VectorErrorSample *pES = (VectorErrorSample *)status.param.dslClearEocMsg.dataPtr;
			if (ADSL_ENDIAN_CONV_USHORT(pES->nERBbytes) > 0) {
				adslMibInfo	*pMibInfo;
				long		mibLen;
				unsigned int extraSkb;
				
				BcmXdslSendErrorSamples(lineId, pES);
				
				mibLen = sizeof(adslMibInfo);
				pMibInfo = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
				extraSkb = BcmXdslGetVectExtraSkbCnt();
				BcmAdslCoreDiagWriteStatusString(lineId, "DRV VECT: pkts sent = %u, pkts discarded = %u, statuses sent = %u, statuses discarded = %u, extraSkb = %u",
					pMibInfo->vectData.vectStat.cntESPktSend,
					pMibInfo->vectData.vectStat.cntESPktDrop,
					pMibInfo->vectData.vectStat.cntESStatSend,
					pMibInfo->vectData.vectStat.cntESStatDrop, extraSkb);
			}
		}
#endif
#endif
		if (XdslMibIsXdsl2Mod(gDslVars) && (statusCode == kDslConnectInfoStatus) && (status.param.dslConnectInfo.code == kG992ShowtimeMonitoringStatus))
			Adsl2UpdateCOErrCounter(&status);

		tMs = (*pAdslCoreStatusCallback) (&status, AdslCoreStatusReadPtr(), tmp-4);
		
		BcmCoreDpcSyncEnter(SYNC_RX);
#ifdef ADSL_MIB
		mibRetCode = AdslMibStatusSnooper(gDslVars, &status);
		
		if (gXdslCoreShMarginMonEnabled(gDslVars)) {
			adslMibInfo		*pMibInfo;
			long			mibLen;
			dslCommandStruct	cmd;
			
			mibLen = sizeof(adslMibInfo);
			pMibInfo = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
			
			if (pMibInfo->adslPhys.adslCurrStatus & kAdslPhysStatusLOM) {
				/* AdslDrvPrintf (TEXT("LOM detected = 0x%lX\n"), pMibInfo->adslPhys.adslCurrStatus); */
				if (gXdslCoreLOMTime(gDslVars) < 0)
					gXdslCoreLOMTime(gDslVars) = 0;
				if (gXdslCoreLOMTime(gDslVars) > gXdslCoreLOMTimeout(gDslVars)) {
					gXdslCoreLOMTime(gDslVars) = -1;
#ifdef SUPPORT_DSL_BONDING
					cmd.command = kDslStartRetrainCmd | (lineId << DSL_LINE_SHIFT);
#else
					cmd.command = kDslStartRetrainCmd;
#endif
					AdslCoreCommandHandler(&cmd);
				}
			}
			else
				gXdslCoreLOMTime(gDslVars) = -1;
		}
#endif
#ifdef G997_1_FRAMER
		G997StatusSnooper (gDslVars, &status);
#endif
#ifdef G992P3
		G992p3OvhMsgStatusSnooper (gDslVars, &status);
#endif
		BcmCoreDpcSyncExit(SYNC_RX);

		switch (statusCode) {
			case kDslEscapeToG994p1Status:
				break;

			case kDslOemDataAddrStatus:
				BcmAdslCoreDiagWriteStatusString(0, "Regular AdslOemDataAddr = 0x%lX\n", status.param.value);
				AdslCoreProcessOemDataAddrMsg(&status);
				break;

			case kAtmStatus:
				if (kAtmStatBertResult == status.param.atmStatus.code) {
					uint	nBits;
					BcmCoreDpcSyncEnter(SYNC_RX);
					nBits = AdslMibBertContinueEx(gDslVars, 
						status.param.atmStatus.param.bertInfo.totalBits,
						status.param.atmStatus.param.bertInfo.errBits);
					BcmCoreDpcSyncExit(SYNC_RX);
					if (nBits != 0)
						AdslCoreBertStart(gLineId(gDslVars), nBits);
				}
				break;
#ifdef SUPPORT_MULTI_PHY
			case kDslTrainingStatus:
				if (kDslStartedG994p1 == status.param.dslTrainingInfo.code)
					acLineReset[lineId] = 0;
				break;
#endif
		}
		
		AdslCoreStatusReadComplete (tmp);
		
		if (kDslStatusBufferChange == statusCode)
			pPhySbSta = ADSL_ADDR_TO_HOST(status.param.value);
		else if (kDslCommandBufferChange == statusCode)
			pPhySbCmd = ADSL_ADDR_TO_HOST(status.param.value);
#if defined(_NOOS)
		if((nStatProcessed%10) == 0)
			reset_TEST_timeout(get_cpuid());
#else
		if( nStatProcessed >= nMaxStatus )
			break;
#endif
	}

	if (tMs != lastStatTimeMs) {
		AdslCoreTimer(tMs - lastStatTimeMs);
		lastStatTimeMs = tMs;
	}

#ifdef SDRAM_HOLD_COUNTERS
	if ((tMs - lastSdramHoldTimeMs) > 2000) {
		BcmAdslCoreDiagWriteStatusString(0, "SDRAM Hold: Time=%d ms, HoldEvents=%d, HoldTime=%d\n"), 
			tMs - lastSdramHoldTimeMs,
			AdslCoreReadtSdramHoldEvents(),
			AdslCoreReadtSdramHoldTime());
		AdslCoreStartSdramHoldCounters();
		lastSdramHoldTimeMs = tMs;
	}
#endif
#if 0
	AdslDrvPrintf(TEXT("AdslCoreIntTaskHandler 2:  pStart=0x%lX, pRead=0x%lX, pWrite=0x%lX\n"),
		StretchBufferGetStartPtr(pPhySbSta), StretchBufferGetReadPtr(pPhySbSta), StretchBufferGetWritePtr(pPhySbSta));
#endif
	return nStatProcessed;

}

AC_BOOL XdslCoreIs2lpActive(unsigned char lineId, unsigned char direction)
{
	return XdslMibIs2lpActive(XdslCoreGetDslVars(lineId), direction);
}

AC_BOOL XdslCoreIsGfastMod(unsigned char lineId)
{
	return XdslMibIsGfastMod(XdslCoreGetDslVars(lineId));
}

AC_BOOL XdslCoreIsVdsl2Mod(unsigned char lineId)
{
	return XdslMibIsVdsl2Mod(XdslCoreGetDslVars(lineId));
}

AC_BOOL XdslCoreIsXdsl2Mod(unsigned char lineId)
{
	return XdslMibIsXdsl2Mod(XdslCoreGetDslVars(lineId));
}
AC_BOOL AdslCoreLinkState (unsigned char lineId)
{
	return AdslMibIsLinkActive(XdslCoreGetDslVars(lineId));
}

int	AdslCoreLinkStateEx (unsigned char lineId)
{
	return AdslMibTrainingState(XdslCoreGetDslVars(lineId));
}

void AdslCoreGetConnectionRates (unsigned char lineId, AdslCoreConnectionRates *pAdslRates)
{
    void *pDslVars = XdslCoreGetDslVars(lineId);
    pAdslRates->fastUpRate = AdslMibGetGetChannelRate (pDslVars, kAdslXmtDir, kAdslFastChannel);
    pAdslRates->fastDnRate = AdslMibGetGetChannelRate (pDslVars, kAdslRcvDir, kAdslFastChannel);
    pAdslRates->intlUpRate = AdslMibGetGetChannelRate (pDslVars, kAdslXmtDir, kAdslIntlChannel);
    pAdslRates->intlDnRate = AdslMibGetGetChannelRate (pDslVars, kAdslRcvDir, kAdslIntlChannel);

#if !defined(__KERNEL__) && !defined(TARG_OS_RTEMS) && !defined(_CFE_)
    if (0 == pAdslRates->intlDnRate) {
            pAdslRates->intlDnRate = pAdslRates->fastDnRate;
            pAdslRates->fastDnRate = 0;
    }
    if (0 == pAdslRates->intlUpRate) {
            pAdslRates->intlUpRate = pAdslRates->fastUpRate;
            pAdslRates->fastUpRate = 0;
    }
#endif
}

extern int PHYSupportedNTones(void *gDslVars);
void AdslCoreSetSupportedNumTones (uchar lineId)
{
	void *pDslVars = XdslCoreGetDslVars(lineId);
	AdslMibSetNumTones(pDslVars, PHYSupportedNTones(pDslVars));
}

int	AdslCoreSetObjectValue (uchar lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	int res;
	BcmCoreDpcSyncEnter(SYNC_RX);
	res = AdslMibSetObjectValue (XdslCoreGetDslVars(lineId), objId, objIdLen, dataBuf, dataBufLen);
	BcmCoreDpcSyncExit(SYNC_RX);
	return res;

	
}

long	AdslCoreGetObjectValue (uchar lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	long res;
	BcmCoreDpcSyncEnter(SYNC_RX);
	res = AdslMibGetObjectValue (XdslCoreGetDslVars(lineId), objId, objIdLen, dataBuf, dataBufLen);
	BcmCoreDpcSyncExit(SYNC_RX);
	return res;
}

AC_BOOL AdslCoreBertStart(uchar lineId, uint totalBits)
{
	dslCommandStruct	cmd;
#ifdef SUPPORT_DSL_BONDING
	cmd.command = kDslDiagStartBERT | (lineId << DSL_LINE_SHIFT);
#else
	cmd.command = kDslDiagStartBERT;
#endif
	cmd.param.value = totalBits;
	return AdslCoreCommandHandler(&cmd);
}

AC_BOOL AdslCoreBertStop(uchar lineId)
{
	dslCommandStruct	cmd;
#ifdef SUPPORT_DSL_BONDING
	cmd.command = kDslDiagStopBERT | (lineId << DSL_LINE_SHIFT);
#else
	cmd.command = kDslDiagStopBERT;
#endif
	return AdslCoreCommandHandler(&cmd);
}

void AdslCoreBertStartEx(uchar lineId, uint bertSec)
{
	uint nBits;

	void *pDslVars = XdslCoreGetDslVars(lineId);
	BcmCoreDpcSyncEnter(SYNC_RX);
	AdslMibBertStartEx(pDslVars, bertSec);
	nBits = AdslMibBertContinueEx(pDslVars, 0, 0);
	BcmCoreDpcSyncExit(SYNC_RX);
	if (nBits != 0)
		AdslCoreBertStart(lineId, nBits);
}

void AdslCoreBertStopEx(uchar lineId)
{
	BcmCoreDpcSyncEnter(SYNC_RX);
	AdslMibBertStopEx(XdslCoreGetDslVars(lineId));
	BcmCoreDpcSyncExit(SYNC_RX);
}

void AdslCoreResetStatCounters(uchar lineId)
{
#ifdef ADSL_MIB
	BcmCoreDpcSyncEnter(SYNC_RX);
	AdslMibResetConectionStatCounters(XdslCoreGetDslVars(lineId));
	BcmCoreDpcSyncExit(SYNC_RX);
#endif
}


void * AdslCoreGetOemParameterData (int paramId, int **ppLen, int *pMaxLen)
{
	int		maxLen = 0;
	uint	*pLen = NULL;
	char	*pData = NULL;

	switch (paramId) {
		case kDslOemG994VendorId:
			pLen = &pAdslOemData->g994VendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->g994VendorId;
			break;
		case kDslOemG994XmtNSInfo:
			pLen = &pAdslOemData->g994XmtNonStdInfoLen;
			maxLen = kAdslOemNonStdInfoMaxSize;
			pData = pAdslOemData->g994XmtNonStdInfo;
			break;
		case kDslOemG994RcvNSInfo:
			pLen = &pAdslOemData->g994RcvNonStdInfoLen;
			maxLen = kAdslOemNonStdInfoMaxSize;
			pData = pAdslOemData->g994RcvNonStdInfo;
			break;
		case kDslOemEocVendorId:
			pLen = &pAdslOemData->eocVendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->eocVendorId;
			break;
		case kDslOemEocVersion:
			pLen = &pAdslOemData->eocVersionLen;
			maxLen=  kAdslOemVersionMaxSize;
			pData = pAdslOemData->eocVersion;
			break;
		case kDslOemEocSerNum:
			pLen = &pAdslOemData->eocSerNumLen;
			maxLen= kAdslOemSerNumMaxSize;
			pData = pAdslOemData->eocSerNum;
			break;
		case kDslOemT1413VendorId:
			pLen = &pAdslOemData->t1413VendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->t1413VendorId;
			break;
		case kDslOemT1413EocVendorId:
			pLen = &pAdslOemData->t1413EocVendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->t1413EocVendorId;
			break;

	}

	*ppLen = (int *)pLen;
	*pMaxLen = maxLen;
	return pData;
}

int AdslCoreGetOemParameter (int paramId, void *buf, int len)
{
	int		maxLen, paramLen,i;
	int		*pLen;
	char	*pData;

	if (NULL == pAdslOemData)
		return 0;

	pData = AdslCoreGetOemParameterData (paramId, &pLen, &maxLen);
	paramLen = (NULL != pLen) ? ADSL_ENDIAN_CONV_UINT32(*pLen) : 0;
	
	__SoftDslPrintf(XdslCoreGetCurDslVars(),"AdslCoreGetOemParameter paramId:%d pLen:%d",0,paramId, paramLen);
	for(i=0; i<paramLen; i++)
		__SoftDslPrintf(XdslCoreGetCurDslVars(),"AdslCoreGetOemParameter pData[i]=%c",0,pData[i]);
	
	if (NULL == buf)
		return maxLen;
	
	if (len > paramLen)
		len = paramLen;
	
	if (len > 0)
		BlockByteMove(len, pData, buf);
	
	return len;
}

int AdslCoreSetOemParameter (int paramId, void *buf, int len)
{
	int		maxLen;
	int		*pLen;
	char	*pData;
	char    *str;
	str=(char *)buf;
	__SoftDslPrintf(XdslCoreGetCurDslVars(),"AdslCoreSetOemParameter paramId=%d len=%d buf=%c%c..%c%c",0,paramId,len,*str,*(str+1),*(str+len-2),*(str+len-1));
	if (NULL == pAdslOemData)
		return 0;

	pData = AdslCoreGetOemParameterData (paramId, &pLen, &maxLen);
	if (len > maxLen)
		len = maxLen;
	if (len > 0)
		BlockByteMove (len, buf, pData);
	*pLen = ADSL_ENDIAN_CONV_UINT32(len);
	adslOemDataModified = AC_TRUE;
	return len;
}

char * AdslCoreGetVersionString(void)
{
	return ADSL_ADDR_TO_HOST(ADSL_ENDIAN_CONV_INT32((uint)pAdslOemData->gDslVerionStringPtr));
}

char * AdslCoreGetBuildInfoString(void)
{
	return ADSL_ADDR_TO_HOST(ADSL_ENDIAN_CONV_INT32((uint)pAdslOemData->gDslBuildDataStringPtr));
}

#ifdef SUPPORT_DSL_BONDING

Private uint GetGinpActMaxDelay(adslMibInfo *pMib)
{
	uint	x, res;
	xdslFramingInfo	*pXdslParam = &pMib->xdslInfo.dirInfo[0].lpInfo[0];	/* dirInfo[0].lpInfo[0] -> Rx data carrying path */
	
	if(0 == pXdslParam->L)
		return 0;
	
	/* ceil((N*Q*RxQueue*8)/L) + 1 symbol times */
	x = pXdslParam->N * pXdslParam->Q * pXdslParam->rxQueue * 8;
	res = (0 != (x%pXdslParam->L)) ? (x/pXdslParam->L + 2): (x/pXdslParam->L + 1);
	/* Convert to ms */
	if((kVdslModVdsl2 == (pMib->adslConnection.modType & kAdslModMask)) &&
		(kVdslProfile30a == pMib->xdslInfo.vdsl2Profile))
		res >>= 3;
	else
		res >>= 2;
	
	return res;
}

void XdslCoreSetMaxBondingDelay(void)
{
	adslMibInfo	*pMib, *pMib1;
	long	mibLen;
	uint	delay, delay1;
	
	pMib = (void *) AdslMibGetObjectValue (XdslCoreGetDslVars(0), NULL, 0, NULL, &mibLen);
	pMib1 = (void *) AdslMibGetObjectValue (XdslCoreGetDslVars(1), NULL, 0, NULL, &mibLen);
	
	if(XdslMibIsGinpActive(XdslCoreGetDslVars(0), RX_DIRECTION)) {
		delay = GetGinpActMaxDelay(pMib);
		delay1 = GetGinpActMaxDelay(pMib1);
		pMib->maxBondingDelay = MAX(delay, delay1);
		pMib1->maxBondingDelay = pMib->maxBondingDelay;
	}
	else {
		delay = pMib->xdslInfo.dirInfo[0].lpInfo[0].delay;
		delay1 = pMib1->xdslInfo.dirInfo[0].lpInfo[0].delay;
		pMib->maxBondingDelay = 0;
		pMib1->maxBondingDelay = 0;
		if(delay > delay1)
			pMib->maxBondingDelay = delay-delay1;
		else if(delay1 > delay)
			pMib1->maxBondingDelay = delay1-delay;
	}
	
	__SoftDslPrintf(XdslCoreGetCurDslVars(),"XdslCoreSetMaxBondingDelay: delay %u maxBondingDelay %u, delay1 %u maxBondingDelay1 %u", 0,
		delay, pMib->maxBondingDelay, delay1, pMib1->maxBondingDelay);
}
#endif


/*
**
**		ADSL_SELF_TEST functions
**
*/

int  AdslCoreGetSelfTestMode(void)
{
	return adslCoreSelfTestMode;
}

void AdslCoreSetSelfTestMode(int stMode)
{
	adslCoreSelfTestMode = stMode;
}

#ifdef ADSL_SELF_TEST

int  AdslCoreGetSelfTestResults(void)
{
	return AdslSelfTestGetResults();
}

#else  /* ADSL_SELF_TEST */

int  AdslCoreGetSelfTestResults(void)
{
	return adslCoreSelfTestMode | kAdslSelfTestCompleted;
}

#endif /* ADSL_SELF_TEST */

