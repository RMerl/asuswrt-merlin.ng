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
 * AdslMib.c -- Adsl MIB data manager
 *
 * Description:
 *  This file contains functions for ADSL MIB (RFC 2662) data management
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.22 $
 *
 * $Id: AdslMib.c,v 1.22 2007/09/04 07:21:15 tonytran Exp $
 *
 * $Log: AdslMib.c,v $
 * Revision 1.22  2007/09/04 07:21:15  tonytran
 * PR31097: 1_28_rc8
 *
 * Revision 1.21  2006/04/15 11:54:24  ovandewi
 * fix trellis status snooping
 *
 * Revision 1.20  2006/04/05 05:06:46  dadityan
 * Fix for wron ATU-C vendor Id
 *
 * Revision 1.16  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.15  2004/04/12 23:34:52  ilyas
 * Merged the latest ADSL driver chnages for ADSL2+
 *
 * Revision 1.14  2004/03/03 20:14:05  ilyas
 * Merged changes for ADSL2+ from ADSL driver
 *
 * Revision 1.13  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.12  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.10  2003/07/18 19:07:15  ilyas
 * Merged with ADSL driver
 *
 * Revision 1.9  2002/11/13 21:32:49  ilyas
 * Added adjustK support for Centillium non-standard framing mode
 *
 * Revision 1.8  2002/10/31 20:27:13  ilyas
 * Merged with the latest changes for VxWorks/Linux driver
 *
 * Revision 1.7  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.6  2002/02/01 06:42:48  ilyas
 * Ignore ASx chaanels for transmit coding parameters
 *
 * Revision 1.5  2002/01/13 22:25:40  ilyas
 * Added functions to get channels rate
 *
 * Revision 1.4  2002/01/03 06:03:36  ilyas
 * Handle byte moves tha are not multiple of 2
 *
 * Revision 1.3  2002/01/02 19:13:57  liang
 * Fix compiler warning.
 *
 * Revision 1.2  2001/12/22 02:37:30  ilyas
 * Changed memset,memcpy function to BlockByte functions
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#include "../AdslCoreMap.h"
#include "SoftDsl.gh"
#include "AdslMib.h"
#include "G997.h"
#include "BlockUtil.h"
#if defined(G993) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
/* Targets w/o G993 need SoftDslG993p2.h for kDsl993p2FramerAdslDs/kDsl993p2FramerAdslUs, NTR defines */
#include "SoftDslG993p2.h"
#endif
#ifndef CONFIG_ARM64
#include <asm/div64.h>
#endif
#include "G992p3OvhMsg.h"

#define globalVar   gAdslMibVars

#define k15MinInSeconds             (15*60)
#define k1HourInSeconds             (60*60)
#define k1DayInSeconds              (24*60*60)

#define Q4ToTenth(num)              ((((num) * 10) + 8) >> 4)
#define Q8ToTenth(num)              ((((num) * 10) + 128) >> 8)
#define RestrictValue(n,l,r)        ((n) < (l) ? (l) : (n) > (r) ? (r) : (n))

#define NitroRate(rate)             ((((rate)*53)+48) / 49)
#define ActualRate(rate)    (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression ? NitroRate(rate) : rate)

#define ReadCnt16(pData)  \
  (((uint) ((uchar *)(pData))[0] << 8) + ((uchar *)(pData))[1])
#define ReadCnt32(pData)          \
  (((uint) ((uchar *)(pData))[0] << 24) + \
  ((uint) ((uchar *)(pData))[1] << 16) + \
  ((uint) ((uchar *)(pData))[2] << 8)  + \
  ((uchar *)(pData))[3])

int    secElapsedInDay;

#define     MIN(a,b) (((a)<(b))?(a):(b))
#define     MAX(a,b) (((a)>(b))?(a):(b))

#define     BITSWAP_REQ_TIMEOUT (10 * 1000)  /* 10 seconds in ms */

#ifdef CONFIG_VDSL_SUPPORTED
#if defined(SUPPORT_DSL_BONDING) || defined(CONFIG_VDSLBRCMPRIV1_SUPPORT)
#define MAX_BITSWAP_TONES    256
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
#define MAX_TONE_NUM    (kVdslMibMaxToneNum << 2)       /* V++ or 35b bonding */
#else
#define MAX_TONE_NUM    (kVdslMibMaxToneNum << 1)       /* 5-band bonding */
#endif
#else /* No bonding and no 35b support */
#define MAX_BITSWAP_TONES    128
#define MAX_TONE_NUM    kVdslMibMaxToneNum
#endif
#else   /* !CONFIG_VDSL_SUPPORTED */
#define MAX_BITSWAP_TONES    6
#define MAX_TONE_NUM    kAdslMibMaxToneNum
#endif
static short                   gSnr[MAX_TONE_NUM];
static short                   gShowtimeMargin[MAX_TONE_NUM];
static uchar                   gBitAlloc[MAX_TONE_NUM];
static short                   gGain[MAX_TONE_NUM];
static short                   gBitSwapTones[MAX_BITSWAP_TONES];
static ComplexShort      gChanCharLin[MAX_TONE_NUM];
static short                   gChanCharLog[MAX_TONE_NUM];
static short                   gQuietLineNoise[MAX_TONE_NUM];
#ifdef CONFIG_BCM_DSL_GFAST
#if defined(CONFIG_BCM963138)
#define MAX_GFAST_TONE_NUM     kVdslMibMaxToneNum                   /* 63138 supports kGfastProfile106a/b w/o bonding */
#else
#define MAX_GFAST_TONE_NUM     (kVdslMibMaxToneNum << 1)            /* 63158 supports single-line kGfastProfile212a and kGfastProfile106a/b bonding*/
#endif
static short                   gActiveLineNoise[MAX_GFAST_TONE_NUM];
static uchar                   gDoiBitAlloc[MAX_GFAST_TONE_NUM];
static short                   gDoiGain[MAX_GFAST_TONE_NUM];
#endif /* CONFIG_BCM_DSL_GFAST */
#if defined(CONFIG_RNC_SUPPORT)
static short                   gQuietLineNoiseRnc[MAX_TONE_NUM];
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
static ComplexLong             gUER[MAX_TONE_NUM];
#ifdef SUPPORT_SELT
static unsigned short          gEchoVariance[MAX_TONE_NUM];
#endif
#endif

static char                    gVendorTbl[][6] =
{
   {0x0, 0x0, '?', '?', '?', '?'},         /* Unknown */
   {0xB5, 0, 'B', 'D', 'C', 'M'},   /* Broadcom */
   {0xB5, 0, 'G', 'S', 'P', 'N'},   /* Globespan */
   {0xB5, 0, 'A', 'N', 'D', 'V'},   /* ADI */
   {0xB5, 0, 'T', 'S', 'T', 'C'},   /* TI */
   {0xB5, 0, 'C', 'E', 'N', 'T'},   /* Centillium */
   {0xB5, 0, 'A', 'L', 'C', 'B'},   /* Alcatel */
   {0xB5, 0, 'I', 'F', 'T', 'N'},   /* Infineon */
   {0xB5, 0, 'I', 'K', 'N', 'S'},   /* Ikanos */
   {0xB5, 0, 'C', 'T', 'N', 'W'},   /* Catena */
   {0xB5, 0, 'A', 'L', 'C', 'B'}, /* AlcatelLSpan */
   {0xB5, 0, 'C', 'X', 'S', 'Y'},   /* Conexant */
   {0xB5, 0, 'C', 'E', 'N', 'T'}    /* Centillium */
};
#define  VENDOR_TBL_SIZE   (sizeof(gVendorTbl)/sizeof(gVendorTbl[0]))

#define IncPerfCounterVar(perfEntry, varname) do {                      \
    (*(adslPerfDataEntry*)(perfEntry)).perfTotal.varname++;             \
    (*(adslPerfDataEntry*)(perfEntry)).perfSinceShowTime.varname++;     \
    (*(adslPerfDataEntry*)(perfEntry)).perfCurr15Min.varname++;         \
    (*(adslPerfDataEntry*)(perfEntry)).perfCurr1Day.varname++;          \
} while (0)

#define AddPerfCounterVar(perfEntry, varname, ct) do {                  \
    (*(adslPerfDataEntry*)(perfEntry)).perfTotal.varname += ct;         \
    (*(adslPerfDataEntry*)(perfEntry)).perfSinceShowTime.varname += ct; \
    (*(adslPerfDataEntry*)(perfEntry)).perfCurr15Min.varname += ct;     \
    (*(adslPerfDataEntry*)(perfEntry)).perfCurr1Day.varname += ct;      \
} while (0)

#define AddRtxCounterVar(perfEntry, varname, ct) do {              \
    (*(rtxCounterInfo*)(perfEntry)).perfTotal.varname += ct;         \
    (*(rtxCounterInfo*)(perfEntry)).perfSinceShowTime.varname += ct; \
    (*(rtxCounterInfo*)(perfEntry)).perfCurr15Min.varname += ct;     \
    (*(rtxCounterInfo*)(perfEntry)).perfCurr1Day.varname += ct;      \
} while (0)

#define AddGinpUsCounterVar(mib, varname, ct) do {                   \
    (*(adslMibInfo*)(mib)).adslStat.ginpStat.cntUS.varname += ct;              \
    (*(adslMibInfo*)(mib)).adslStatSincePowerOn.ginpStat.cntUS.varname += ct;  \
} while (0)

#define AddGinpDsCounterVar(mib, varname, ct) do {                   \
    (*(adslMibInfo*)(mib)).adslStat.ginpStat.cntDS.varname += ct;              \
    (*(adslMibInfo*)(mib)).adslStatSincePowerOn.ginpStat.cntDS.varname += ct;  \
} while (0)

#ifdef CONFIG_BCM_DSL_GFAST
#define AddGfastOlrCounterVar(perfEntry, varname, ct) do {              \
    (*(gfastOlrCounterInfo*)(perfEntry)).perfTotal.varname += ct;         \
    (*(gfastOlrCounterInfo*)(perfEntry)).perfSinceShowTime.varname += ct; \
    (*(gfastOlrCounterInfo*)(perfEntry)).perfCurr15Min.varname += ct;     \
    (*(gfastOlrCounterInfo*)(perfEntry)).perfCurr1Day.varname += ct;      \
} while (0)

#define AddGfastEocfCounterVar(mib, varname, ct) do {                   \
    (*(adslMibInfo*)(mib)).adslStat.eocStat.varname += ct;              \
    (*(adslMibInfo*)(mib)).adslStatSincePowerOn.eocStat.varname += ct;  \
} while (0)

#define AddGfastCounterVar(mib, varname, ct) do {                        \
    (*(adslMibInfo*)(mib)).adslStat.gfastStat.varname += ct;             \
    (*(adslMibInfo*)(mib)).adslStatSincePowerOn.gfastStat.varname += ct; \
} while (0)
#endif

#ifndef SUPPORT_24HR_CNT_STAT
#define INC_STAT_HIST_24HR_CNT(var)
#define ADD_STAT_HIST_24HR_CNT(var,cnt)
#else
#define INC_STAT_HIST_24HR_CNT(var)      IncStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, var)
#define ADD_STAT_HIST_24HR_CNT(var,cnt)  AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, var, cnt)
#endif

#ifdef SUPPORT_24HR_CNT_STAT
#define IncStatHist24HrCounterVar(statHist24HrEntry, varname)                      \
    statHist24HrEntry.statHistHrCounters[statHist24HrEntry.curHourIndex].varname++

#define AddStatHist24HrCounterVar(statHist24HrEntry, varname, n)                      \
    statHist24HrEntry.statHistHrCounters[statHist24HrEntry.curHourIndex].varname += n
#else
#define IncStatHist24HrCounterVar(statHist24HrEntry, varname)
#define AddStatHist24HrCounterVar(statHist24HrEntry, varname, n)
#endif

#define AddBlockCounterVar(chEntry, varname, inc) do {                  \
    (*(adslChanPerfDataEntry *)(chEntry)).perfTotal.varname += inc;     \
    (*(adslChanPerfDataEntry *)(chEntry)).perfCurr15Min.varname += inc; \
    (*(adslChanPerfDataEntry *)(chEntry)).perfCurr1Day.varname += inc;  \
} while (0)

#define IncFailureCounterVar(perfEntry, varname) do {                   \
    (*(adslPerfDataEntry*)(perfEntry)).failTotal.varname++;             \
    (*(adslPerfDataEntry*)(perfEntry)).failSinceShowTime.varname++;     \
    (*(adslPerfDataEntry*)(perfEntry)).failSinceLastShowTime.varname++; \
    (*(adslPerfDataEntry*)(perfEntry)).failCurDay.varname++;            \
    (*(adslPerfDataEntry*)(perfEntry)).failCur15Min.varname++;          \
} while (0)

#define IncAtmRcvCounterVar(mib, pathId, varname, n) do {                       \
    (*(adslMibInfo*)(mib)).atmStat2lp[pathId].rcvStat.varname += n;             \
    (*(adslMibInfo*)(mib)).atmStatSincePowerOn2lp[pathId].rcvStat.varname += n; \
    (*(adslMibInfo*)(mib)).atmStatCurDay2lp[pathId].rcvStat.varname += n;       \
    (*(adslMibInfo*)(mib)).atmStatCur15Min2lp[pathId].rcvStat.varname += n;     \
} while (0)

#define IncAtmXmtCounterVar(mib, pathId, varname, n) do {                       \
    (*(adslMibInfo*)(mib)).atmStat2lp[pathId].xmtStat.varname += n;             \
    (*(adslMibInfo*)(mib)).atmStatSincePowerOn2lp[pathId].xmtStat.varname += n; \
    (*(adslMibInfo*)(mib)).atmStatCurDay2lp[pathId].xmtStat.varname += n;       \
    (*(adslMibInfo*)(mib)).atmStatCur15Min2lp[pathId].xmtStat.varname += n;     \
} while (0)

extern uint BcmAdslCoreGetConfiguredMod(uchar lineId);
extern void AdslCoreIndicateLinkPowerStateL2(uchar lineId);
extern int sprintf( char *, const char *, ... );
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
extern int AdslDrvIsFakeLinkUp(unsigned char lineId);
#endif
#ifdef SUPPORT_DSL_BONDING
extern void XdslCoreSetMaxBondingDelay(void);
extern void *XdslCoreGetDslVars(unsigned char lineId);
#endif
#if defined(CONFIG_BCM_DSL_GFAST) || defined(CONFIG_VDSL_SUPPORTED)
extern void * XdslCoreGetCurDslVars(void);
#endif

Private void AdslSetBandPlan(void *gDslVars);

Private int SM_DECL AdslMibNotifyIdle (void *gDslVars, uint event)
{
    return 0;
}

#if defined(CONFIG_BCM_DSL_GFAST)
Public Boolean XdslMibIsFastRetrain(void *gDslVars)
{
    return globalVar.adslMib.fastRetrainActive;
}

Public unsigned short XdslMibGetToneNumGfast(void * gDslVars)
{
    return globalVar.nTonesGfast;
}

Public Boolean XdslMibReportAmd4GfastCounters(void *gDslVars)
{
    return (0 != (globalVar.adslMib.gfastSupportedOptions & GFAST_SUPPORTEDOPTIONS_ANDEFTR));
}
#endif

Public Boolean AdslMibIsAdsl2Mod(void *gDslVars)
{
    int     modType  = AdslMibGetModulationType(gDslVars);

    return ((kAdslModAdsl2 == modType) || (kAdslModAdsl2p == modType) ||(kAdslModReAdsl2 == modType));
}

Public Boolean XdslMibIsGfastMod(void *gDslVars)
{
    return (kXdslModGfast == AdslMibGetModulationType(gDslVars));
}

Public Boolean XdslMibIsVdsl2Mod(void *gDslVars)
{
    int     modType  = AdslMibGetModulationType(gDslVars);

    return ((kVdslModVdsl2 == modType) || (kXdslModGfast == modType));
}

Public Boolean XdslMibIsXdsl2Mod(void *gDslVars)
{
    int     modType  = AdslMibGetModulationType(gDslVars);

    return ((kAdslModAdsl2 == modType) || (kAdslModAdsl2p == modType) ||
    (kAdslModReAdsl2 == modType) || (kVdslModVdsl2 == modType) || (kXdslModGfast == modType));
}

Public Boolean XdslMibIsAtmConnectionType(void *gDslVars)
{
    unsigned char tmType;
    tmType = globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[0].tmType[0];
    return ((kXdslDataAtm == tmType) || (kXdslDataNitro == tmType));
}

Public Boolean XdslMibIsPtmConnectionType(void *gDslVars)
{
    unsigned char tmType;
    tmType = globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[0].tmType[0];
    return(kXdslDataPtm == tmType);
}

#if defined(CONFIG_VDSL_SUPPORTED)
Private uint UpdateAttnEtr(void *gDslVars, uint attRate, uint ndr, uint etr)
{
	uint attnEtr = ndr != 0 ? (((etr << 8)/ndr)*attRate) >> 8 : attRate;
     //__SoftDslPrintf(gDslVars, "UpdateAttnEtr: attnEtr=%d attnRate=%d ndr=%d etr=%d", 0, attnEtr, attRate, ndr, etr);
	return attnEtr;
}
#endif

Public void XdslMibUpdateAttnEtr(void *gDslVars, int dir)
{
#if defined(CONFIG_VDSL_SUPPORTED)
     //__SoftDslPrintf(gDslVars, "XdslMibUpdateAttnEtr: dir=%d", 0, dir);
	if (RX_DIRECTION == dir)
	  globalVar.adslMib.xdslPhys.attnETR = UpdateAttnEtr(gDslVars, globalVar.adslMib.xdslPhys.adslCurrAttainableRate/1000,
	    globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[0].dataRate, globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[0].etrRate);
	else
	  globalVar.adslMib.xdslAtucPhys.attnETR = UpdateAttnEtr(gDslVars, globalVar.adslMib.xdslAtucPhys.adslCurrAttainableRate/1000,
	    globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[0].dataRate, globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[0].etrRate);
#endif
}

#define kTwoThirdQ15      ((int)21845)   /* 2/3 in Q15 */
#define kFourFifthQ15     ((int)26214)   /* 4/5 in Q15 */
#define kTenLog10TwoQ12     ((int)12330)   /* 10*log10(2) in Q12 */
#define kTenLog10EQ12     ((int)17789)   /* 10*log10(e) in Q12 */
Public short
UtilQ0LinearToQ4dB (uint x)
  {
  int   k;
  int  y, z, v2, v3, v4, u;
  /*long  v5;*/
  
  /* (1) speical cases */
  if (x <= 1) return 0;
  
  /* (2) normalize x */
  y = (int)x;
  k = 32;
  while (y > 0) { y <<= 1; k--;}
  
  /* (3) approximate ln(y) */
  z  = ((-y) >> 17) & 0x7FFF;           /* z  = (1-y) in Q15 */
  v2 = (z * z) >> 16;               /* v2 = z**2 / 2 in Q15 */
  v3 = (v2 * ((kTwoThirdQ15 * z) >> 15)) >> 15; /* v3 = z**3 / 3 in Q15 */
  v4 = (v2 * v2) >> 15;             /* v4 = z**4 / 4 in Q15 */
  /*v5 = (v4 * ((kFourFifthQ15 * z) >> 15)) >> 15;*/  /* v5 = z**5 / 5 in Q15 */
  
  u = z + v2 + v3 + v4;
  
  /* (4) calculate output */
  return ((kTenLog10TwoQ12 * k - ((kTenLog10EQ12 * u) >> 15) + 0x80) >> 8);
  
  }
#ifdef G993
Public short log2Int (uint x)
  {
    short k=0;
    uint y=1;
    while(y<x) {y<<=1;k++;}
    return (1<<k);
  }

Public void CreateNegBandPlan(void *gDslVars, bandPlanDescriptor* NegtBandPlan, bandPlanDescriptor32* NegtBandPlan32, bandPlanDescriptor* PhysBandPlan)
{
    int n,j=0,i=0;
    
    AdslMibByteClear(sizeof(bandPlanDescriptor), (void*)NegtBandPlan);
    NegtBandPlan->reserved=NegtBandPlan32->reserved;

    for(n=0; n<PhysBandPlan->noOfToneGroups;n++)
    {
        for (; j<NegtBandPlan32->noOfToneGroups;j++)
        {
            if ((NegtBandPlan32->toneGroups[j].startTone>=PhysBandPlan->toneGroups[n].startTone) &&
                (NegtBandPlan32->toneGroups[j].startTone<=PhysBandPlan->toneGroups[n].endTone))
            {
                NegtBandPlan->noOfToneGroups++;
                NegtBandPlan->toneGroups[i].startTone=NegtBandPlan32->toneGroups[j].startTone;
                for (; j<NegtBandPlan32->noOfToneGroups;j++)
                {
                    if (NegtBandPlan32->toneGroups[j].endTone<=PhysBandPlan->toneGroups[n].endTone)
                        NegtBandPlan->toneGroups[i].endTone=NegtBandPlan32->toneGroups[j].endTone;
                    else {
                        if(0 == NegtBandPlan->toneGroups[i].endTone)
                            NegtBandPlan->toneGroups[i].endTone = PhysBandPlan->toneGroups[n].endTone;
                        break;
                    }
                }
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
                if(0 == gLineId(gDslVars))
#endif
                __SoftDslPrintf(gDslVars, "NegtBP start=%d end=%d\n", 0,NegtBandPlan->toneGroups[i].startTone,NegtBandPlan->toneGroups[i].endTone);
                i++;
                break;
            }
            if (NegtBandPlan32->toneGroups[j].startTone>PhysBandPlan->toneGroups[n].endTone)
                break;
        }
    }
}

Public short calcOamGfactor(unsigned short lastTone)
{
#if 1 /* This agrees with the VDSL standard (and the DSLAM), changing PHY to match (43k) */
  return log2Int((lastTone+511)>>9);
#else
  return log2Int((lastTone+512)>>9); /* Change this to match the PHY calculation and CO GUI calculation */
#endif
}
#endif

Public int AdslMibStrCopy(char *srcPtr, char *dstPtr)
{
    char    *sPtr = srcPtr;

    do {
        *dstPtr = *srcPtr++;
    } while (*dstPtr++ != 0);

    return srcPtr - sPtr;
}

Public int AdslMibGetActiveChannel(void *gDslVars)
{
    return (globalVar.adslMib.adslConnection.chType);

}

Public int AdslMibGetModulationType(void *gDslVars)
{
    return globalVar.adslMib.adslConnection.modType & kAdslModMask;
}

#ifndef _M_IX86
Private int AdslMibGetFastBytesInFrame(G992CodingParams *param)
{
    int     n = 0;

#ifdef  G992P1_NEWFRAME
    if (param->NF >= 0x0200)    /* old PHY (A023a or earlier) */
        n = param->RSF - param->AS3BF;

    n +=  param->AS0BF;
    n += param->AS1BF;
    n += param->AS2BF;
    n += param->AS3BF;
    n += param->LS0CF;
    n += param->LS1BF;
    n += param->LS2BF;
#else
    n = 0;
#endif
    return n;
}

Private int AdslMibGetIntlBytesInFrame(G992CodingParams *param)
{
    int     n;

#ifdef  G992P1_NEWFRAME
    n =  param->AS0BI;
    n += param->AS1BI;
    n += param->AS2BI;
    n += param->AS3BI;
    n += param->LS0CI;
    n += param->LS1BI;
    n += param->LS2BI;
#else
    n = (param->K - 1);
#endif
    return n;
}

Public int AdslMibGetGetChannelRate(void *gDslVars, int dir, int channel)
{
  G992CodingParams *param;
  int               n=0;
  if ( (0 != channel ) && (1 != channel))
    return n;
  param = (kAdslRcvDir == dir) ? &globalVar.rcvParams : &globalVar.xmtParams;
#ifdef G992P3
#ifdef CONFIG_VDSL_SUPPORTED
  if(XdslMibIsVdsl2Mod(gDslVars))
    n = 1000 * ((kAdslRcvDir == dir) ? globalVar.adslMib.vdslInfo[channel].rcvRate : globalVar.adslMib.vdslInfo[channel].xmtRate);
  else
#endif /* CONFIG_VDSL_SUPPORTED */
  if (AdslMibIsAdsl2Mod(gDslVars)) {
#if (MAX_LP_NUM == 2)
    n = 1000 * ((kAdslRcvDir == dir) ? globalVar.adslMib.adsl2Info2lp[channel].rcvRate : globalVar.adslMib.adsl2Info2lp[channel].xmtRate);
#else
    if (kAdslIntlChannel == channel)
      n = 1000 * ((kAdslRcvDir == dir) ? globalVar.adslMib.adsl2Info.rcvRate : globalVar.adslMib.adsl2Info.xmtRate);
#endif /* #if (MAX_LP_NUM == 2) */
  }
  else
#endif /* G992P3 */
  {
    if (kAdslIntlChannel == channel)
      n = AdslMibGetIntlBytesInFrame(param);
    else 
      n = AdslMibGetFastBytesInFrame(param);
    
    n *= 4000 * 8;

    if (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression)
      n = NitroRate(n);
  }

  return n;
}

Private void AdslMibSetChanEntry(G992CodingParams *param, adslChanEntry *pChFast, adslChanEntry *pChIntl)
{
    int     n;

    pChIntl->adslChanIntlDelay = 4 + (param->S*param->D >> 2) + ((param->S - 1) >> 2);
    pChFast->adslChanPrevTxRate = pChFast->adslChanCurrTxRate;
    pChIntl->adslChanPrevTxRate = pChIntl->adslChanCurrTxRate;
    n = AdslMibGetFastBytesInFrame(param);
    pChFast->adslChanCrcBlockLength = n * 68;
    pChFast->adslChanCurrTxRate = n * 4000 * 8;

    n = AdslMibGetIntlBytesInFrame(param);
    pChIntl->adslChanCrcBlockLength = n * 68;
    pChIntl->adslChanCurrTxRate = n * 4000 * 8;
}

Private void AdslMibSetConnectionInfo(void *gDslVars, G992CodingParams *param, int code, int val, adslConnectionInfo *pConInfo)
{
    adslDataConnectionInfo *pInfo;

    pInfo = (kG992p2XmtCodingParamsInfo == code) ? &pConInfo->xmtInfo : &pConInfo->rcvInfo;
    pInfo->K = param->K;
    pInfo->S = param->S;
    pInfo->R = param->R;
    pInfo->D = param->D;

    pConInfo->trellisCoding = (0 == val) ? kAdslTrellisOff : kAdslTrellisOn;
    if(!XdslMibIsXdsl2Mod(gDslVars))
        pConInfo->chType = AdslMibGetFastBytesInFrame(param) ? kAdslFastChannel : kAdslIntlChannel;
    if (kG992p2XmtCodingParamsInfo == code) 
        if (val != 0)
            pConInfo->trellisCoding2 |= kAdsl2TrellisTxEnabled;
        else
            pConInfo->trellisCoding2 &= ~kAdsl2TrellisTxEnabled;
    else
        if (val != 0)
            pConInfo->trellisCoding2 |= kAdsl2TrellisRxEnabled;
        else
            pConInfo->trellisCoding2 &= ~kAdsl2TrellisRxEnabled;

    /* Centillium NS framing for S = 1/2 */
    if (kG992p2RcvCodingParamsInfo == code) {
        if (((pInfo->K + pInfo->R) > 255) && (6 == globalVar.rsOption[0]))
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 6;
        else 
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 0;
    }
}

Private void Adsl2MibSetConnectionInfo(void *gDslVars, G992p3CodingParams *param, int code, int val, adsl2ConnectionInfo *pConInfo)
{
    adsl2DataConnectionInfo *pInfo;

    pInfo = (kG992p3XmtCodingParamsInfo == code) ? &pConInfo->xmt2Info : &pConInfo->rcv2Info;

    pInfo->Nlp  = param->Nlp;
    pInfo->Nbc  = param->Nbc;
    pInfo->MSGlp= param->MSGlp;
    pInfo->MSGc = ADSL_ENDIAN_CONV_USHORT(param->MSGc);

    pInfo->L = ADSL_ENDIAN_CONV_UINT32(param->L);
    pInfo->M = ADSL_ENDIAN_CONV_USHORT(param->M);
    pInfo->T = ADSL_ENDIAN_CONV_USHORT(param->T);
    pInfo->D = ADSL_ENDIAN_CONV_USHORT(param->D);
    pInfo->R = ADSL_ENDIAN_CONV_USHORT(param->R);
    pInfo->B = ADSL_ENDIAN_CONV_USHORT(param->B);

    if(kG992p3RcvCodingParamsInfo == code )
        globalVar.adslMib.xdslConnection[(int)globalVar.pathId].chType = (pInfo->D > 1) ? kAdslIntlChannel: kAdslFastChannel;
}

Private void Adsl2MibSetInfoFromGdmt1(void *gDslVars, adsl2DataConnectionInfo *pInfo2, adslDataConnectionInfo *pInfo)
{
    if (pInfo2->L != 0)
        return;

    pInfo2->Nlp = 1;
    pInfo2->Nbc = 1;
    pInfo2->MSGlp= 0;
    pInfo2->MSGc = 1;

    pInfo2->D = pInfo->D;
    pInfo2->R = pInfo->R;
    pInfo2->M = pInfo->S;
    pInfo2->B = pInfo->K - 1;
    pInfo2->L = (pInfo->K + pInfo->R/pInfo->S) * 8;
    pInfo2->T = 1;
}



Private int GetAdsl2Inpq(xdslFramingInfo *p2, int q)
{
   return (0 == p2->L) ? 0 : (4*p2->R*p2->D*q)/p2->L;
}

Private int GetAdsl2Delayq(xdslFramingInfo *p2, int q)
{
   return (0 == p2->L) ? 0 : (q*2*(p2->M*(p2->B[0]+1)+p2->R)*p2->D/p2->L) + (q >> 1);
}

#ifdef CONFIG_VDSL_SUPPORTED
Private void Xdsl2MibConvertVdslFramerParam(vdslMuxFramerParamType *pVdslFramingParam, xdslFramingInfo * pXdslParam)
{
   pVdslFramingParam->N = pXdslParam->N;
   pVdslFramingParam->D = pXdslParam->D;
   pVdslFramingParam->L = pXdslParam->L;
   pVdslFramingParam->B[0] = (unsigned char)pXdslParam->B[0];
   pVdslFramingParam->B[1] = (unsigned char)pXdslParam->B[1];
   pVdslFramingParam->I = pXdslParam->I;
   pVdslFramingParam->M = pXdslParam->M;
   pVdslFramingParam->T = pXdslParam->T;
   pVdslFramingParam->G = pXdslParam->G;
   pVdslFramingParam->U = pXdslParam->U;
   pVdslFramingParam->F = pXdslParam->F;
   pVdslFramingParam->R = pXdslParam->R;
   pVdslFramingParam->ovhType = pXdslParam->ovhType;
   pVdslFramingParam->ahifChanId[0] = pXdslParam->ahifChanId[0];
   pVdslFramingParam->ahifChanId[1] = pXdslParam->ahifChanId[1] ;
   pVdslFramingParam->tmType[0] = pXdslParam->tmType[0];
   pVdslFramingParam->tmType[1] = pXdslParam->tmType[1];
   pVdslFramingParam->pathId = pXdslParam->pathId;
}
#endif

Private void XdslMibConvertToAdslFramerParam(void *gDslVars, int dir,
   adslDataConnectionInfo *pAdslFramingParam,
   adsl2DataConnectionInfo   *pAdsl2FramingParam,
   xdslDirFramingInfo      *pDirFramingInfo,
   xdslFramingInfo         *pXdslFramingParam)
{
   pAdsl2FramingParam->Nlp = pDirFramingInfo->Nlp;
   pAdsl2FramingParam->Nbc = pDirFramingInfo->Nbc;
   pAdsl2FramingParam->MSGlp = pDirFramingInfo->MSGlp;
   pAdsl2FramingParam->MSGc = pXdslFramingParam->U*pXdslFramingParam->G - 6;

   pAdsl2FramingParam->L = pXdslFramingParam->L;
   pAdsl2FramingParam->M = pXdslFramingParam->M;
   pAdsl2FramingParam->T = pXdslFramingParam->T;
   pAdsl2FramingParam->D = pXdslFramingParam->D;
   pAdsl2FramingParam->R = pXdslFramingParam->R;
   pAdsl2FramingParam->B = pXdslFramingParam->B[0];
   pAdslFramingParam->K = pXdslFramingParam->B[0]+pXdslFramingParam->B[1]+1;

   if(!AdslMibIsAdsl2Mod(gDslVars)) {
      pAdslFramingParam->S = (uchar)pXdslFramingParam->S.num;
      /* Centillium NS framing for S = 1/2 */
      if (RX_DIRECTION == dir) {
        if (((pAdslFramingParam->K + pAdslFramingParam->R) > 255) && (6 == globalVar.rsOption[0]))
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 6;
        else 
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 0;
      }
   }
}

Private void XdslMibConvertFromAdslFramerParam(void *gDslVars,
   adslDataConnectionInfo *pAdslFramingParam,
   adsl2DataConnectionInfo   *pAdsl2FramingParam,
   xdslDirFramingInfo      *pDirFramingInfo,
   xdslFramingInfo         *pXdslFramingParam)
{
   pDirFramingInfo->Nlp = pAdsl2FramingParam->Nlp;
   pDirFramingInfo->Nbc = pAdsl2FramingParam->Nbc;
   pDirFramingInfo->MSGlp = pAdsl2FramingParam->MSGlp;
   
   pXdslFramingParam->L = (ushort)pAdsl2FramingParam->L;
   pXdslFramingParam->M = (uchar)pAdsl2FramingParam->M;
   pXdslFramingParam->T = (uchar)pAdsl2FramingParam->T;
   pXdslFramingParam->D = pAdsl2FramingParam->D;
   pXdslFramingParam->R = (uchar)pAdsl2FramingParam->R;
   pXdslFramingParam->B[0] = (uchar)pAdsl2FramingParam->B;
   pXdslFramingParam->N = (pAdsl2FramingParam->B+1)*pAdsl2FramingParam->M + pAdsl2FramingParam->R;
   
   if(AdslMibIsAdsl2Mod(gDslVars)) {
      pXdslFramingParam->G = 1;
      pXdslFramingParam->U = pAdsl2FramingParam->MSGc + 6;
      pXdslFramingParam->S.num = 8*pXdslFramingParam->N;
      pXdslFramingParam->S.denom = pXdslFramingParam->L;
   }
   else {
      pXdslFramingParam->S.num = pAdslFramingParam->S;
      pXdslFramingParam->S.denom = 1;
   }
}

#if defined(SUPPORT_HMI)
extern void LineUpdateBearerLatencyInfo(unsigned char lineId, FramerDeframerOptions *param, int dir);
#endif

Private void Xdsl2MibConvertConnectionInfo(void *gDslVars, int pathId, int dir)
{
#ifdef CONFIG_VDSL_SUPPORTED
   vdslMuxFramerParamType   *pVdslFramingParam;
#endif
   adsl2DelayInp                  *pDelayInp;
   adslDataConnectionInfo     *pAdslFramingParam;
   adsl2DataConnectionInfo   *pAdsl2FramingParam;
   adsl2ChanInfo                 *pAdsl2ChanInfo;
   xdslConnectionInfo     *pXdslInfo = &globalVar.adslMib.xdslInfo;
   xdslDirFramingInfo      *pDirFramingInfo = &globalVar.adslMib.xdslInfo.dirInfo[dir];
   xdslFramingInfo         *pXdslFramingParam = &globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[pathId];
   Boolean                    newStat = true;

#ifdef CONFIG_VDSL_SUPPORTED
   if(XdslMibIsVdsl2Mod(gDslVars)) {
      globalVar.adslMib.vdslInfo[0].vdsl2Mode = pXdslInfo->xdslMode;
      globalVar.adslMib.vdslInfo[0].pwrState = pXdslInfo->pwrState;
      globalVar.adslMib.vdslInfo[0].vdsl2Profile = pXdslInfo->vdsl2Profile;
      if(RX_DIRECTION == dir ) {
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
         if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
         globalVar.adslMib.vdslInfo[pathId].rcvRate = pXdslFramingParam->dataRate;
         pDelayInp = &globalVar.adslMib.vdslInfo[pathId].rcv2DelayInp;
         pVdslFramingParam = &globalVar.adslMib.vdslInfo[pathId].rcv2Info;
      }
      else {
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
         if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
         globalVar.adslMib.vdslInfo[pathId].xmtRate = pXdslFramingParam->dataRate;
         pDelayInp = &globalVar.adslMib.vdslInfo[pathId].xmt2DelayInp;
         pVdslFramingParam = &globalVar.adslMib.vdslInfo[pathId].xmt2Info;
      }
      pDelayInp->delay = pXdslFramingParam->delay;
      pDelayInp->inp = pXdslFramingParam->INP;
      Xdsl2MibConvertVdslFramerParam(pVdslFramingParam, pXdslFramingParam);
      if (kXdslModGfast == AdslMibGetModulationType(gDslVars)) {
	    globalVar.adslMib.adsl2Info2lp[0].rcvChanInfo.connectionType = globalVar.adslMib.xdslInfo.dirInfo[0].lpInfo[0].tmType[0];
	    globalVar.adslMib.adsl2Info2lp[0].xmtChanInfo.connectionType = globalVar.adslMib.xdslInfo.dirInfo[1].lpInfo[0].tmType[0];
	  }
      return;
   }
   else
#endif
   {   /* ADSL */
      globalVar.adslMib.adsl2Info2lp[0].adsl2Mode = pXdslInfo->xdslMode;
      globalVar.adslMib.adsl2Info2lp[0].pwrState = pXdslInfo->pwrState;
      if(RX_DIRECTION == dir ) {
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
         if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
         globalVar.adslMib.adsl2Info2lp[pathId].rcvRate = pXdslFramingParam->dataRate;
         pDelayInp = &globalVar.adslMib.adsl2Info2lp[pathId].rcv2DelayInp;
         pAdsl2FramingParam = &globalVar.adslMib.adsl2Info2lp[pathId].rcv2Info;
         pAdslFramingParam = &globalVar.adslMib.xdslConnection[pathId].rcvInfo;
         pAdsl2ChanInfo = &globalVar.adslMib.adsl2Info2lp[pathId].rcvChanInfo;
      }
      else {
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
         if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
         globalVar.adslMib.adsl2Info2lp[pathId].xmtRate = pXdslFramingParam->dataRate;
         pDelayInp = &globalVar.adslMib.adsl2Info2lp[pathId].xmt2DelayInp;
         pAdsl2FramingParam = &globalVar.adslMib.adsl2Info2lp[pathId].xmt2Info;
         pAdslFramingParam = &globalVar.adslMib.xdslConnection[pathId].xmtInfo;
         pAdsl2ChanInfo = &globalVar.adslMib.adsl2Info2lp[pathId].xmtChanInfo;
      }
   }
   
   if(0 == pDirFramingInfo->NlpData)
      newStat = false;
   
   if(newStat) {
      /* Old delay/inp were sent and stored in Q8; new delay is sent from PHY as an integer and INP in Q1 */
      pDelayInp->delay = pXdslFramingParam->delay << 8;
      pDelayInp->inp = pXdslFramingParam->INP << 7;
      pAdsl2ChanInfo->ahifChanId = pXdslFramingParam->ahifChanId[0];
      pAdsl2ChanInfo->connectionType = pXdslFramingParam->tmType[0];
      XdslMibConvertToAdslFramerParam(gDslVars, dir, pAdslFramingParam, pAdsl2FramingParam, pDirFramingInfo, pXdslFramingParam);
   }
   else {
      /* For old framing parameters status */
      pXdslFramingParam->ahifChanId[0] = pAdsl2ChanInfo->ahifChanId;
      pXdslFramingParam->tmType[0] = pAdsl2ChanInfo->connectionType;
      XdslMibConvertFromAdslFramerParam(gDslVars, pAdslFramingParam, pAdsl2FramingParam, pDirFramingInfo, pXdslFramingParam);

      if(TX_DIRECTION == dir) {
         /* PHY doesn't send delay/inp for US direction */
         /* Calculate and store in Q8, the same format as sent by PHY through kG992RcvDelay/kG992RcvInp */
         /* status for DS direction */
         pDelayInp->delay = (ushort)GetAdsl2Delayq(pXdslFramingParam, (1 << 8));
         pDelayInp->inp = (ushort)GetAdsl2Inpq(pXdslFramingParam, (1 << 8));
      }
      else {
         if(!AdslMibIsAdsl2Mod(gDslVars)) {
            /* ADSL1, check for S = 0.05 */
            if((pAdslFramingParam->K > 256) && (1 == pXdslFramingParam->S.num)) {
               pXdslFramingParam->B[0] = pAdslFramingParam->K/2;
               pXdslFramingParam->R /= 2;
               pXdslFramingParam->S.denom = 2;
            }
         }
         if (!XdslMibIsPhyRActive(gDslVars, RX_DIRECTION)) {
            /* In PhyR mode, PHY already send these values */
            pDelayInp->delay = (ushort)GetAdsl2Delayq(pXdslFramingParam, (1 << 8));
            pDelayInp->inp = (ushort)GetAdsl2Inpq(pXdslFramingParam, (1 << 8));
         }
      }

      /* Unified framing param store delay as integer and INP in Q1 as sent from new PHY */
      pXdslFramingParam->delay = pDelayInp->delay >> 8;
      pXdslFramingParam->INP = pDelayInp->inp >> 7;
   }
}

Private void Xdsl2MibSetConnectionInfo(void *gDslVars, FramerDeframerOptions *param, int dir, int msgLen)
{
   xdslDirFramingInfo   *pDirFramingInfo;
   xdslFramingInfo      *pFramingInfo;
   uint                       ginpStat, fireStat;
   adslConnectionInfo    *pXdslInfo;
   adslConnectionStat    *pXdslStat;
   uchar                       pathId = param->path;
   Boolean                    swapPath = (TX_DIRECTION == dir) ? globalVar.swapTxPath: globalVar.swapRxPath;
   int                         dataRate;
   
   if(swapPath)
      pathId =param->path ^ 1;

   pXdslInfo = &globalVar.adslMib.xdslConnection[pathId];
   pXdslStat = &globalVar.adslMib.xdslStat[pathId];
   pDirFramingInfo = &globalVar.adslMib.xdslInfo.dirInfo[dir];
   pFramingInfo = &pDirFramingInfo->lpInfo[pathId];
   dataRate = pFramingInfo->dataRate;
   AdslMibByteClear(sizeof(xdslFramingInfo), pFramingInfo);
   pFramingInfo->dataRate = dataRate;

   if(DS_DIRECTION == dir) {
      ginpStat = kGinpDsEnabled;
      fireStat = kFireDsEnabled;
   }
   else {
      ginpStat = kGinpUsEnabled;
      fireStat = kFireUsEnabled;
   }
   
   if(TX_DIRECTION == dir) {
      if(param->codingType & kAdslTrellisOn)
         pXdslInfo->trellisCoding2 |= kAdsl2TrellisTxEnabled;
      else
         pXdslInfo->trellisCoding2 &= ~kAdsl2TrellisTxEnabled;
      if(!XdslMibIsGfastMod(gDslVars) && (param->fireEnabled & fireStat)) {
         pXdslStat->fireStat.status |= fireStat;
         pFramingInfo->rtxMode = 1;
      }
   }
   else {
      if(param->codingType & kAdslTrellisOn)
         pXdslInfo->trellisCoding2 |= kAdsl2TrellisRxEnabled;
      else
         pXdslInfo->trellisCoding2 &= ~kAdsl2TrellisRxEnabled;
      if(!XdslMibIsGfastMod(gDslVars) && (param->fireEnabled & fireStat)){
         pXdslStat->fireStat.status |= fireStat;
         pFramingInfo->rtxMode = 1;
      }
      pXdslInfo->chType = (param->D > 1) ? kAdslIntlChannel: kAdslFastChannel;
   }
    
   pFramingInfo->N = param->N;
#ifdef CONFIG_BCM_DSL_GFAST
   if(XdslMibIsGfastMod(gDslVars)) {
      pFramingInfo->Lrmc = param->B;
      pFramingInfo->MNDSNOI = param->MNDSNOI;
   }
   else
#endif
   {
      pFramingInfo->B[0] = (unsigned char)param->B;
      pFramingInfo->B[1] = (unsigned char)param->b1;
   }
   pFramingInfo->I = param->I;
   pFramingInfo->G = param->G;
   pFramingInfo->U = param->U;
   pFramingInfo->F = param->F;
   pFramingInfo->ovhType = param->ovhType;
   pFramingInfo->ahifChanId[0] = param->ahifChanId[0];
   pFramingInfo->ahifChanId[1] = param->ahifChanId[1] ;
   pFramingInfo->tmType[0] = param->tmType[0];
   pFramingInfo->tmType[1] = param->tmType[1];
   pFramingInfo->pathId = param->path;
   pFramingInfo->L = param->L;
   pFramingInfo->M = param->M;
   pFramingInfo->T = param->T;
   pFramingInfo->D = param->D;
   pFramingInfo->R = param->R;
   pFramingInfo->delay = param->delay;
   pFramingInfo->INP = param->INP;
   pFramingInfo->S.num = param->S.num;
   pFramingInfo->S.denom = param->S.denom;
   pFramingInfo->tpsTcOptions = param->tpsTcOptions;
   
   if(msgLen >= GINP_FRAMER_STRUCT_SIZE) {
      if((param->path) && (0 != (param->ginpFraming&7))) {
         pXdslStat->ginpStat.status |= ginpStat;
         pFramingInfo->ginpLookBack = param->ginpFraming >> 3;
         pFramingInfo->rtxMode = 17 + (param->ginpFraming&7);
         if(msgLen >= GINP_FRAMER_INPSHINE_STRUCT_SIZE)
           pFramingInfo->INP = param->INPshine;
      }
#ifdef CONFIG_BCM_DSL_GFAST
      else if((msgLen >= GINP_FRAMER_INPSHINE_STRUCT_SIZE) && XdslMibIsGfastMod(gDslVars))
         pFramingInfo->INP = param->INPshine;
#endif
      pFramingInfo->INPrein = param->INPrein;
      pFramingInfo->rrcBits = param->phyRrrcBits;
      pFramingInfo->rxQueue = param->fireRxQueue;
      pFramingInfo->txQueue = param->fireTxQueue;
      pFramingInfo->Q = param->Q;
      pFramingInfo->V = param->V;
      if(msgLen >= GINP_FRAMER_ETR_STRUCT_SIZE)
         pFramingInfo->etrRate = param->ETR_kbps;
      else
         pFramingInfo->etrRate = 0;
   }

#ifdef CONFIG_BCM_DSL_GFAST
   if(XdslMibIsGfastMod(gDslVars) && (msgLen >= ETR_MIN_EOC_FRAMER_STRUCT_SIZE)) {
      pFramingInfo->etru = param->etru;
      pFramingInfo->ETRminEoc = param->ETRminEoc;
      pFramingInfo->Ldr = param->Ldr;
   }
#endif

   if( XdslMibIs2lpActive(gDslVars, dir))
      pDirFramingInfo->Nlp = 2;
   else
      pDirFramingInfo->Nlp = 1;
   
   pDirFramingInfo->NlpData = 1;
   pDirFramingInfo->Nbc = 1;
   pDirFramingInfo->MSGlp= 0;
#if defined(SUPPORT_HMI)
  LineUpdateBearerLatencyInfo(gLineId(gDslVars), param, dir);
#endif
}

Private uint AdslMibShowtimeSFErrors(uint *curCnts, uint *oldCnts)
{
    return (curCnts[kG992ShowtimeSuperFramesRcvdWrong] - oldCnts[kG992ShowtimeSuperFramesRcvdWrong]);
}

Private uint AdslMibShowtimeRSErrors(uint *curCnts, uint *oldCnts)
{
    return (curCnts[kG992ShowtimeRSCodewordsRcvedCorrectable] - oldCnts[kG992ShowtimeRSCodewordsRcvedCorrectable]);
}

Private Boolean AdslMibShowtimeDataError(uint *curCnts, uint *oldCnts)
{
    return 
        (curCnts[kG992ShowtimeRSCodewordsRcvedUncorrectable] != 
         oldCnts[kG992ShowtimeRSCodewordsRcvedUncorrectable])   ||
         (AdslMibShowtimeSFErrors(curCnts, oldCnts) != 0);
}

Private void AdslMibNotify(void *gDslVars, int event)
{
    globalVar.notifyHandlerPtr (gDslVars, event);
}

#ifdef CONFIG_BCM_DSL_GFAST
#define CAPTURE_CNTRS_ONSES   1
#define CAPTURE_CNTRS_ONLOR   2
#define CAPTURE_CNTRS_ONLOS   3

#define XDSL_CLR_CAPTURE_FLAGS  {globalVar.inhibCntrsOnSES.inhibitCntrCapture = 0; globalVar.inhibCntrsOnLOR.inhibitCntrCapture= 0; globalVar.inhibCntrsOnLOS.inhibitCntrCapture= 0;}
#define XDSL_MIB_CAPTURE_CNTRS(gDV, captureSrc)  XdslMibCaptureCntrs(gDV, captureSrc)
#define XDSL_MIB_RESTORE_CNTRS(gDV)              XdslMibRestoreCntrs(gDV)

Private void XdslMibCaptureCntrs(void *gDslVars, int captureSrc)
{
  InhibitCounters *pInhibitCntrs;
  int pathId;
  
  if(G992p3OvhMsgIsL3RspPending(gDslVars))
    return;
  
  pathId = XdslMibGetCurrentMappedPathId(gDslVars, RX_DIRECTION);
  
  if(CAPTURE_CNTRS_ONSES == captureSrc)
    pInhibitCntrs = &globalVar.inhibCntrsOnSES;
  else if(CAPTURE_CNTRS_ONLOR == captureSrc)
    pInhibitCntrs = &globalVar.inhibCntrsOnLOR;
  else
    pInhibitCntrs = &globalVar.inhibCntrsOnLOS;

  __SoftDslPrintf(gDslVars, "XdslMibCaptureCntrs(%d): inhibitCntrCapture=%d, fastRetrainActive=%d, uasOnSes=%d sesContCnt=%d pathId=%d",0,
    captureSrc, pInhibitCntrs->inhibitCntrCapture,globalVar.adslMib.fastRetrainActive,globalVar.uasOnSes,globalVar.sesContCnt,pathId);
  
  if(!pInhibitCntrs->inhibitCntrCapture) { /* Do for all modulations now */
    int pathId = XdslMibGetCurrentMappedPathId(gDslVars, RX_DIRECTION);
    pInhibitCntrs->inhibitCntrCapture = true;
    pInhibitCntrs->xdslFECs = globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanCorrectedBlks;
    pInhibitCntrs->xdslCRCs = globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanUncorrectBlks;
#if 0
    pInhibitCntrs->xdslRtxUc = globalVar.adslMib.adslStatSincePowerOn.ginpStat.cntDS.rtx_uc;
    pInhibitCntrs->xdslRtxTx = globalVar.adslMib.adslStatSincePowerOn.ginpStat.cntUS.rtx_tx;
#endif
    pInhibitCntrs->xdslESs = globalVar.adslMib.adslPerfData.perfTotal.adslESs;
    pInhibitCntrs->xdslSES = globalVar.adslMib.adslPerfData.perfTotal.adslSES;
    pInhibitCntrs->xdslLOSS = globalVar.adslMib.adslPerfData.perfTotal.adslLOSS;
    pInhibitCntrs->xdslLORS = globalVar.adslMib.adslPerfData.perfTotal.xdslLORS;
    __SoftDslPrintf(gDslVars, "ESs = %u SES = %u LOSS = %u LORS = %u fec = %u crc = %u AS = %u UAS = %u\n", 0,
      pInhibitCntrs->xdslESs, pInhibitCntrs->xdslSES, pInhibitCntrs->xdslLOSS, pInhibitCntrs->xdslLORS,
      pInhibitCntrs->xdslFECs, pInhibitCntrs->xdslCRCs,
      globalVar.adslMib.adslPerfData.perfTotal.adslAS, globalVar.adslMib.adslPerfData.perfTotal.adslUAS);
    if(CAPTURE_CNTRS_ONSES != captureSrc) {
      if(false == globalVar.inhibCntrsOnSES.inhibitCntrCapture)
        globalVar.inhibCntrsOnSES = *pInhibitCntrs;
    }
  }
}

Private void XdslMibRestoreCntrs(void *gDslVars)
{
  InhibitCounters *pInhibitCntrs;
  uint cnt;
  int retrainReason = globalVar.adslMib.adslPerfData.lastRetrainReason;
  int pathId = XdslMibGetCurrentMappedPathId(gDslVars, RX_DIRECTION);
  
  if(((retrainReason >> kRetrainReasonSes) & 0x1) || (globalVar.reInitTimeThld == globalVar.sesContCnt))
    pInhibitCntrs = &globalVar.inhibCntrsOnSES;
  else if(((retrainReason >> kRetrainReasonLosDetector) & 0x1) && ((retrainReason >> kRetrainReasonRdiDetector) & 0x1)) {
    if(globalVar.inhibCntrsOnLOS.inhibitCntrCapture && globalVar.inhibCntrsOnLOR.inhibitCntrCapture) {
      cnt = globalVar.adslMib.adslPerfData.perfTotal.adslSES;
      if((cnt - globalVar.inhibCntrsOnLOS.xdslSES) > (cnt - globalVar.inhibCntrsOnLOR.xdslSES))
        pInhibitCntrs = &globalVar.inhibCntrsOnLOS;
      else
        pInhibitCntrs = &globalVar.inhibCntrsOnLOR;
    }
    else if(globalVar.inhibCntrsOnLOS.inhibitCntrCapture)
      pInhibitCntrs = &globalVar.inhibCntrsOnLOS;
    else
      pInhibitCntrs = &globalVar.inhibCntrsOnLOR;
  }
  else if((retrainReason >> kRetrainReasonLosDetector) & 0x1)
    pInhibitCntrs = &globalVar.inhibCntrsOnLOS;
  else if((retrainReason >> kRetrainReasonRdiDetector) & 0x1)
    pInhibitCntrs = &globalVar.inhibCntrsOnLOR;
  else
    pInhibitCntrs = NULL; /* Not likely going here */

  __SoftDslPrintf(gDslVars, "XdslMibRestoreCntrs: retrainReason=0x%x, captureOnSES=%d, captureOnLOR=%d, captureOnLOS=%d, fastRetrainActive=%d, uasOnSes=%d sesContCnt=%d pathId=%d",0,
    globalVar.adslMib.adslPerfData.lastRetrainReason, globalVar.inhibCntrsOnSES.inhibitCntrCapture,
    globalVar.inhibCntrsOnLOR.inhibitCntrCapture, globalVar.inhibCntrsOnLOS.inhibitCntrCapture,
    globalVar.adslMib.fastRetrainActive, globalVar.uasOnSes, globalVar.sesContCnt, pathId);

  if(pInhibitCntrs && pInhibitCntrs->inhibitCntrCapture) {
    __SoftDslPrintf(gDslVars, "ESs = %u(%u) SES = %u(%u) LOSS = %u(%u) LORS = %u(%u) fec = %u(%u) crc = %u(%u) AS = %u UAS = %u reInitTimeThld = %u\n", 0,
      globalVar.adslMib.adslPerfData.perfTotal.adslESs, pInhibitCntrs->xdslESs,
      globalVar.adslMib.adslPerfData.perfTotal.adslSES, pInhibitCntrs->xdslSES,
      globalVar.adslMib.adslPerfData.perfTotal.adslLOSS, pInhibitCntrs->xdslLOSS,
      globalVar.adslMib.adslPerfData.perfTotal.xdslLORS, pInhibitCntrs->xdslLORS,
      globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanCorrectedBlks, pInhibitCntrs->xdslFECs,
      globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanUncorrectBlks, pInhibitCntrs->xdslCRCs,
      globalVar.adslMib.adslPerfData.perfTotal.adslAS, globalVar.adslMib.adslPerfData.perfTotal.adslUAS, globalVar.reInitTimeThld);
    cnt = globalVar.adslMib.adslPerfData.perfTotal.adslSES - pInhibitCntrs->xdslSES;
    AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslSES, -cnt);
    ADD_STAT_HIST_24HR_CNT(asdlSES, -cnt);
    if(false == globalVar.uasOnSes) {
      AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslUAS, cnt);
      AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslAS, -cnt);
    }
    cnt = globalVar.adslMib.adslPerfData.perfTotal.adslESs - pInhibitCntrs->xdslESs;
    AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslESs, -cnt);
    ADD_STAT_HIST_24HR_CNT(adslESs, -cnt);
    cnt = globalVar.adslMib.adslPerfData.perfTotal.adslLOSS - pInhibitCntrs->xdslLOSS;
    AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslLOSS, -cnt);
    ADD_STAT_HIST_24HR_CNT(adslLOSS, -cnt);
    cnt = globalVar.adslMib.adslPerfData.perfTotal.xdslLORS - pInhibitCntrs->xdslLORS;
    AddPerfCounterVar(&globalVar.adslMib.adslPerfData, xdslLORS, -cnt);
    ADD_STAT_HIST_24HR_CNT(xdslLORS, -cnt);

    cnt = globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanCorrectedBlks - pInhibitCntrs->xdslFECs;
    AddBlockCounterVar(&globalVar.adslMib.xdslChanPerfData[pathId], adslChanCorrectedBlks, -cnt);
    AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, FECErrors, -cnt);
    cnt = globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanUncorrectBlks - pInhibitCntrs->xdslCRCs;
    AddBlockCounterVar(&globalVar.adslMib.xdslChanPerfData[pathId], adslChanUncorrectBlks, -cnt);
    AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, CRCErrors, -cnt);
#if 0 /* CO team is not sure what to do with these counters yet, so leave them as is for now */
    cnt = globalVar.adslMib.adslStatSincePowerOn.ginpStat.cntUS.rtx_tx - pInhibitCntrs->xdslRtxTx;
    AddGinpUsCounterVar(&globalVar.adslMib, rtx_tx, -cnt);
    AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntUS, rtx_tx, -cnt);
    cnt = globalVar.adslMib.adslStatSincePowerOn.ginpStat.cntDS.rtx_uc - pInhibitCntrs->xdslRtxUc;
    AddGinpDsCounterVar(&globalVar.adslMib, rtx_uc, -cnt);
    AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntDS, rtx_uc, -cnt);
#endif
    __SoftDslPrintf(gDslVars, "ESs = %u SES = %u LOSS = %u LORS = %u fec = %u crc = %u AS = %u UAS = %u\n", 0,
      globalVar.adslMib.adslPerfData.perfTotal.adslESs,
      globalVar.adslMib.adslPerfData.perfTotal.adslSES,
      globalVar.adslMib.adslPerfData.perfTotal.adslLOSS,
      globalVar.adslMib.adslPerfData.perfTotal.xdslLORS,
      globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanCorrectedBlks,
      globalVar.adslMib.xdslChanPerfData[pathId].perfTotal.adslChanUncorrectBlks,
      globalVar.adslMib.adslPerfData.perfTotal.adslAS,
      globalVar.adslMib.adslPerfData.perfTotal.adslUAS);
  }
  
  XDSL_CLR_CAPTURE_FLAGS;
}

#else
#define XDSL_CLR_CAPTURE_FLAGS
#define XDSL_MIB_CAPTURE_CNTRS(gDV, captureSrc)
#define XDSL_MIB_RESTORE_CNTRS(gDV)
#endif /* CONFIG_BCM_DSL_GFAST */

#define AdslMibES(state,cnt)                                        \
do {                                                                \
    if (!state) {                                                   \
      state = true;                                               \
      if(!globalVar.uasOnSes) {  \
        IncPerfCounterVar(&globalVar.adslMib.adslPerfData, cnt);    \
        INC_STAT_HIST_24HR_CNT(cnt); \
      }                                                           \
    }                                                               \
} while (0)

#define AdslMibSES(state,cnt)                                       \
do {                                                                \
    if (!state) {                                                   \
        state = true;                                               \
        if(!globalVar.uasOnSes) { /* inhibit for all modulations now */ \
          XDSL_MIB_CAPTURE_CNTRS(gDslVars,CAPTURE_CNTRS_ONSES);     \
          IncPerfCounterVar(&globalVar.adslMib.adslPerfData, cnt);  \
          INC_STAT_HIST_24HR_CNT(cnt);                              \
        }                                                           \
        if (globalVar.uasOnSes) {                                   \
          IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslUAS); \
          globalVar.sesContCnt = 0;                                 \
        }                                                           \
        else if (++globalVar.sesContCnt == globalVar.reInitTimeThld) {  \
          __SoftDslPrintf(gDslVars, "UAS enter: fastRetrainActive=%d sesContCnt=%d UAS=%u AS=%u",0, \
            globalVar.adslMib.fastRetrainActive, globalVar.sesContCnt, \
            globalVar.adslMib.adslPerfData.perfTotal.adslUAS, globalVar.adslMib.adslPerfData.perfTotal.adslAS); \
          AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslUAS, globalVar.reInitTimeThld); \
          AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslAS, -globalVar.reInitTimeThld); \
          __SoftDslPrintf(gDslVars, "UAS=%u AS=%u after adjustment",0,globalVar.adslMib.adslPerfData.perfTotal.adslUAS, globalVar.adslMib.adslPerfData.perfTotal.adslAS); \
          /* AddPerfCounterVar(&globalVar.adslMib.adslPerfData, cnt, -10); */   \
          /* ADD_STAT_HIST_24HR_CNT(asdlSES, -9); */           \
          globalVar.uasOnSes = true;                          \
          XDSL_MIB_RESTORE_CNTRS(gDslVars);                   \
          if (XdslMibIsGfastMod(gDslVars)) {                  \
            AdslMibNotify(gDslVars, kXdslEventContSESThresh); \
          }                                                   \
        }                                                     \
    }                                                         \
} while (0)

#define AdslMibNoSES(state,cnt)                                     \
do {                                                                \
    if (!state) {                                                   \
        if (0 == globalVar.uasOnSes) {                              \
          globalVar.sesContCnt = 0;                                 \
          XDSL_CLR_CAPTURE_FLAGS;                                   \
        }                                                           \
        else if (++globalVar.sesContCnt == 10) {                    \
          __SoftDslPrintf(gDslVars, "UAS exit: fastRetrainActive=%d sesContCnt=%d UAS=%u AS=%u",0, \
            globalVar.adslMib.fastRetrainActive, globalVar.sesContCnt, \
            globalVar.adslMib.adslPerfData.perfTotal.adslUAS, globalVar.adslMib.adslPerfData.perfTotal.adslAS); \
          XDSL_CLR_CAPTURE_FLAGS;                                   \
          AddPerfCounterVar(&globalVar.adslMib.adslPerfData, adslUAS, -9); \
          __SoftDslPrintf(gDslVars, "UAS=%u AS=%u after adjustment",0,globalVar.adslMib.adslPerfData.perfTotal.adslUAS, globalVar.adslMib.adslPerfData.perfTotal.adslAS); \
          globalVar.uasOnSes = false;                               \
        }                                                           \
        else                                                        \
          IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslUAS); \
    }                                                               \
} while (0)

Private void AdslMibConnectionStatUpdate (void *gDslVars, uint *cntOld, uint *cntNew, int pathId)
{
    adslConnectionStat *s1, *s1Tx;
    adslConnectionStat *s2, *s2Tx;
    int n;

    s1 = &globalVar.adslMib.xdslStat[pathId];
    s2 = &globalVar.adslMib.xdslStatSincePowerOn[pathId];

    n = cntNew[kG992ShowtimeSuperFramesRcvd] - cntOld[kG992ShowtimeSuperFramesRcvd];
    s1->rcvStat.cntSF += n;
    s2->rcvStat.cntSF += n;

    if((false == (globalVar.adslMib.fastRetrainActive || globalVar.uasOnSes || globalVar.currSecondSES))
#ifdef CONFIG_BCM_DSL_GFAST
       && !G992p3OvhMsgIsL3RspPending(gDslVars)
#endif
      ) {
      n = cntNew[kG992ShowtimeSuperFramesRcvdWrong] - cntOld[kG992ShowtimeSuperFramesRcvdWrong];
      s1->rcvStat.cntSFErr += n;
      s2->rcvStat.cntSFErr += n;
      n = cntNew[kG992ShowtimeRSCodewordsRcvedCorrectable] - cntOld[kG992ShowtimeRSCodewordsRcvedCorrectable];
      s1->rcvStat.cntRSCor += n;
      s2->rcvStat.cntRSCor += n;
    }

    n = cntNew[kG992ShowtimeRSCodewordsRcved] - cntOld[kG992ShowtimeRSCodewordsRcved];
    s1->rcvStat.cntRS += n;
    s2->rcvStat.cntRS += n;

    n = cntNew[kG992ShowtimeRSCodewordsRcvedUncorrectable] - cntOld[kG992ShowtimeRSCodewordsRcvedUncorrectable];
    s1->rcvStat.cntRSUncor += n;
    s2->rcvStat.cntRSUncor += n;
    if(!XdslMibIsXdsl2Mod(gDslVars)) {
        /* for ADSL2 and above, these are done in AdslMibUpdateTxStat() */
        s1Tx = &globalVar.adslMib.xdslStat[0];
        s2Tx = &globalVar.adslMib.xdslStatSincePowerOn[0];
        n = cntNew[kG992ShowtimeNumOfFEBE] - cntOld[kG992ShowtimeNumOfFEBE];
        s1Tx->xmtStat.cntSFErr += n;
        s2Tx->xmtStat.cntSFErr += n;
        n = cntNew[kG992ShowtimeNumOfFECC] - cntOld[kG992ShowtimeNumOfFECC];
        s1Tx->xmtStat.cntRSCor += n;
        s2Tx->xmtStat.cntRSCor += n;

        if ((globalVar.adslMib.adslConnection.xmtInfo.R != 0) && globalVar.adslMib.adslConnection.xmtInfo.S)
            s1Tx->xmtStat.cntRS = (s1->xmtStat.cntSF * 68) / globalVar.adslMib.adslConnection.xmtInfo.S;
        else
            s1Tx->xmtStat.cntRS = 0;
    }

#ifdef DSL_REPORT_ALL_COUNTERS
    n = cntNew[kG992ShowtimeNumOfHEC] - cntOld[kG992ShowtimeNumOfHEC];
    IncAtmRcvCounterVar(&globalVar.adslMib, pathId, cntHEC, n);
#ifdef SUPPORT_24HR_CNT_STAT
    AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, HECErrors, n);
#endif
    n = cntNew[kG992ShowtimeNumOfOCD] - cntOld[kG992ShowtimeNumOfOCD];
    IncAtmRcvCounterVar(&globalVar.adslMib, pathId, cntOCD, n);

    n = cntNew[kG992ShowtimeNumOfLCD] - cntOld[kG992ShowtimeNumOfLCD];
    IncAtmRcvCounterVar(&globalVar.adslMib, pathId, cntLCD, n);
#ifdef SUPPORT_24HR_CNT_STAT
    AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, CellDelin, n);
#endif
    if (n != 0)
        AdslMibES(globalVar.currSecondLCD, adslLCDS);
    if(!XdslMibIsXdsl2Mod(gDslVars)) {
        /* for ADSL2 and above, these are done in AdslMibUpdateTxStat() */
        n = cntNew[kG992ShowtimeNumOfFHEC] - cntOld[kG992ShowtimeNumOfFHEC];
        IncAtmXmtCounterVar(&globalVar.adslMib, 0, cntHEC, n);
#ifdef SUPPORT_24HR_CNT_STAT
        AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, ATUCHECErrors, n);
#endif
    }
    n = cntNew[kG992ShowtimeNumOfFOCD] - cntOld[kG992ShowtimeNumOfFOCD];
    IncAtmXmtCounterVar(&globalVar.adslMib, 0, cntOCD, n);

    n = cntNew[kG992ShowtimeNumOfFLCD] - cntOld[kG992ShowtimeNumOfFLCD];
    IncAtmXmtCounterVar(&globalVar.adslMib, 0, cntLCD, n);
#endif
}

Private void AdslMibUpdateShowtimeErrors(void *gDslVars, uint nErr, int pathId)
{

    int Limit = 18;

    if(XdslMibIsXdsl2Mod(gDslVars) && !XdslMibIsGinpActive(gDslVars, RX_DIRECTION)) {
        if (XdslMibIsGfastMod(gDslVars))
            Limit = 18;
        else if (globalVar.PERpDS[pathId] < (15 << 8))
            Limit=((18*15) << 8)/globalVar.PERpDS[pathId];
        else if(globalVar.PERpDS[pathId] > (20 << 8))
            Limit=((18*20) << 8)/globalVar.PERpDS[pathId];
    }
    if (nErr > Limit)
        AdslMibSES(globalVar.currSecondSES, adslSES);
    else
        AdslMibNoSES(globalVar.currSecondSES, adslSES);

    AdslMibES(globalVar.currSecondErrored, adslESs);
}

Private void AdslMibUpdateLOM(void *gDslVars)
{
    AdslMibES(globalVar.currSecondLOM, adslLOMS);
}

Private void AdslMibUpdateLOS(void *gDslVars)
{
    AdslMibES(globalVar.currSecondErrored, adslESs);
    AdslMibES(globalVar.currSecondLOS, adslLOSS);
    AdslMibSES(globalVar.currSecondSES, adslSES);
}

#ifdef CONFIG_BCM_DSL_GFAST
Private void AdslMibUpdateLOR(void *gDslVars)
{
    AdslMibES(globalVar.currSecondErrored, adslESs);
    AdslMibES(globalVar.currSecondLOR, xdslLORS);
    AdslMibSES(globalVar.currSecondSES, adslSES);
}
#endif

Private void AdslMibUpdateLOF(void *gDslVars)
{
    AdslMibES(globalVar.currSecondErrored, adslESs);
    AdslMibSES(globalVar.currSecondSES, adslSES);
}

Private void AdslMibUpdateShowtimeRSErrors(void *gDslVars, uint nErr)
{
    AdslMibES(globalVar.currSecondFEC, adslFECs);
}

Private void DiffTxPerfData(
    adslPerfCounters *pPerf0, 
    adslPerfCounters *pPerf1, 
    adslPerfCounters *pPerfRes)
{
    uint n;
    n=pPerf0->adslESs - pPerf1->adslESs;
    pPerfRes->adslESs  = n<=0x1000000?n:0;
    n=pPerf0->adslSES - pPerf1->adslSES;
    pPerfRes->adslSES  = n<=0x1000000?n:0;
    n= pPerf0->adslFECs- pPerf1->adslFECs;
    pPerfRes->adslFECs = n<=0x1000000?n:0;
    n= pPerf0->adslLOSS- pPerf1->adslLOSS;
    pPerfRes->adslLOSS = n<=0x1000000?n:0;
    n=pPerf0->adslUAS - pPerf1->adslUAS;
    pPerfRes->adslUAS  = n<=0x1000000?n:0;
#ifdef CONFIG_BCM_DSL_GFAST
    if (XdslMibIsGfastMod(XdslCoreGetCurDslVars())) {
       n = pPerf0->xdslLORS - pPerf1->xdslLORS;
       pPerfRes->xdslLORS = n <= 0x1000000? n: 0;
    }
#endif
}


Private void AddTxPerfData(
    adslPerfCounters *pPerf0, 
    adslPerfCounters *pPerf1, 
    adslPerfCounters *pPerfRes)
{
    pPerfRes->adslESs  = pPerf0->adslESs + pPerf1->adslESs;
    pPerfRes->adslSES  = pPerf0->adslSES + pPerf1->adslSES;
    pPerfRes->adslFECs = pPerf0->adslFECs+ pPerf1->adslFECs;
    pPerfRes->adslLOSS = pPerf0->adslLOSS+ pPerf1->adslLOSS;
    pPerfRes->adslUAS  = pPerf0->adslUAS + pPerf1->adslUAS;
#ifdef CONFIG_BCM_DSL_GFAST
    if (XdslMibIsGfastMod(XdslCoreGetCurDslVars()))
        pPerfRes->xdslLORS = pPerf0->xdslLORS + pPerf1->xdslLORS;
#endif

}

Private void AdslMibUpdateIntervalCounters (void *gDslVars, uint sec)
{
    adslMibInfo     *pMib = &globalVar.adslMib;
    int    secElapsed, i, n;

    secElapsed  = sec + globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed;
    if (secElapsed >= k15MinInSeconds) {
        n = globalVar.adslMib.adslPerfData.adslPerfValidIntervals;
        if (n < kAdslMibPerfIntervals)
            n++;
        for (i = (n-1); i > 0; i--)
            AdslMibByteMove(
                sizeof(adslPerfCounters), 
                &globalVar.adslMib.adslPerfIntervals[i-1], 
                &globalVar.adslMib.adslPerfIntervals[i]);
        AdslMibByteMove(
            sizeof(adslPerfCounters), 
            &globalVar.adslMib.adslPerfData.perfCurr15Min, 
            &globalVar.adslMib.adslPerfIntervals[0]);
        BlockByteClear(sizeof(adslPerfCounters), (void*)&globalVar.adslMib.adslPerfData.perfCurr15Min);
        globalVar.adslMib.adslPerfData.adslPerfValidIntervals = n;
        /* ATM/failure Cur15 */
        BlockByteClear(sizeof(globalVar.adslMib.atmStatCur15Min2lp), (void*)&globalVar.adslMib.atmStatCur15Min2lp);
        BlockByteClear(sizeof(adslFailureCounters), (void*)&globalVar.adslMib.adslPerfData.failCur15Min);
        /* TX Cur15 */
        BlockByteMove(sizeof(adslPerfCounters), (void*)&pMib->adslTxPerfCur15Min, (void*)&pMib->adslTxPerfLast15Min);
        BlockByteClear(sizeof(adslPerfCounters), (void*)&pMib->adslTxPerfCur15Min);
        /* rtx Cur15 */
        BlockByteClear(sizeof(rtxCounters), (void*)&pMib->rtxCounterData.cntDS.perfCurr15Min);
        BlockByteClear(sizeof(rtxCounters), (void*)&pMib->rtxCounterData.cntUS.perfCurr15Min);
#ifdef CONFIG_BCM_DSL_GFAST
        /* gfastOlrCounters Cur15 */
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[0].cntDS.perfCurr15Min);
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[0].cntUS.perfCurr15Min);
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[1].cntDS.perfCurr15Min);
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[1].cntUS.perfCurr15Min);
#endif
        n = globalVar.adslMib.adslChanIntlPerfData.adslChanPerfValidIntervals;
        if (n < kAdslMibChanPerfIntervals)
            n++;
        for (i = (n-1); i > 0; i--) {
            AdslMibByteMove(
                sizeof(adslChanCounters),
                &globalVar.adslMib.adslChanIntlPerfIntervals[i-1], 
                &globalVar.adslMib.adslChanIntlPerfIntervals[i]);
            AdslMibByteMove(
                sizeof(adslChanCounters),
                &globalVar.adslMib.adslChanFastPerfIntervals[i-1], 
                &globalVar.adslMib.adslChanFastPerfIntervals[i]);
        }
        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanIntlPerfData.perfCurr15Min, 
            &globalVar.adslMib.adslChanIntlPerfIntervals[0]);
        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanFastPerfData.perfCurr15Min, 
            &globalVar.adslMib.adslChanFastPerfIntervals[0]);

        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanIntlPerfData.perfCurr15Min);
        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanFastPerfData.perfCurr15Min);
        globalVar.adslMib.adslChanIntlPerfData.adslChanPerfValidIntervals = n;
        globalVar.adslMib.adslChanFastPerfData.adslChanPerfValidIntervals = n;

        secElapsed -= k15MinInSeconds;
    }
    globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanFastPerfData.adslPerfCurr15MinTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanIntlPerfData.adslPerfCurr15MinTimeElapsed = secElapsed;
#ifdef SUPPORT_24HR_CNT_STAT
    secElapsed = sec + globalVar.adslMib.statHist24HrCounters.cur1HourTimeElapsed;
    if(secElapsed >= k1HourInSeconds) {
        i = globalVar.adslMib.statHist24HrCounters.curHourIndex+1;
        if( i > 23 )
            i = 0;
        BlockByteClear(sizeof(StatHistHrCounters), (void*)&globalVar.adslMib.statHist24HrCounters.statHistHrCounters[i]);
        globalVar.adslMib.statHist24HrCounters.curHourIndex = i;
        secElapsed -= k1HourInSeconds;
    }
    globalVar.adslMib.statHist24HrCounters.cur1HourTimeElapsed = secElapsed;
#endif
    secElapsed = sec + globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed;
    if (secElapsed >= k1DayInSeconds) {
        AdslMibByteMove(
            sizeof(adslPerfCounters), 
            &globalVar.adslMib.adslPerfData.perfCurr1Day, 
            &globalVar.adslMib.adslPerfData.perfPrev1Day);
        BlockByteClear(sizeof(adslPerfCounters), (void*)&globalVar.adslMib.adslPerfData.perfCurr1Day);
        AdslMibByteMove(
            sizeof(adslFailureCounters), 
            &globalVar.adslMib.adslPerfData.failCurDay, 
            &globalVar.adslMib.adslPerfData.failPrevDay);
        BlockByteClear(sizeof(adslFailureCounters), (void*)&globalVar.adslMib.adslPerfData.failCurDay);
        /* ATM 1Day */
        AdslMibByteMove(
            sizeof(globalVar.adslMib.atmStatCurDay2lp), 
            &globalVar.adslMib.atmStatCurDay2lp, 
            &globalVar.adslMib.atmStatPrevDay2lp);
        BlockByteClear(sizeof(globalVar.adslMib.atmStatCurDay2lp), (void*)&globalVar.adslMib.atmStatCurDay2lp);
        /* TX 1Day */
        BlockByteMove(sizeof(adslPerfCounters), (void*)&pMib->adslTxPerfCur1Day, (void*)&pMib->adslTxPerfLast1Day);
        BlockByteClear(sizeof(adslPerfCounters), (void*)&pMib->adslTxPerfCur1Day);
        /* rtx 1Day */
        BlockByteClear(sizeof(rtxCounters), (void*)&pMib->rtxCounterData.cntDS.perfCurr1Day);
        BlockByteClear(sizeof(rtxCounters), (void*)&pMib->rtxCounterData.cntUS.perfCurr1Day);
#ifdef CONFIG_BCM_DSL_GFAST
        /* gfastOlrCounters 1Day */
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[0].cntDS.perfCurr1Day);
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[0].cntUS.perfCurr1Day);
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[1].cntDS.perfCurr1Day);
        BlockByteClear(sizeof(gfastEocCounters), (void*)&pMib->gfastOlrXoiCounterData[1].cntUS.perfCurr1Day);
#endif
        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanIntlPerfData.perfCurr1Day, 
            &globalVar.adslMib.adslChanIntlPerfData.perfPrev1Day);
        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanFastPerfData.perfCurr1Day,
            &globalVar.adslMib.adslChanFastPerfData.perfPrev1Day);
        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanIntlPerfData.perfCurr1Day);
        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanFastPerfData.perfCurr1Day);

        globalVar.adslMib.adslPerfData.adslAturPerfPrev1DayMoniSecs = k1DayInSeconds;
        secElapsed -= k1DayInSeconds;
    }
    globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanFastPerfData.adslPerfCurr1DayTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanIntlPerfData.adslPerfCurr1DayTimeElapsed = secElapsed;
    secElapsedInDay=secElapsed;
}

#if defined(SUPPORT_DSL_BONDING)

#if defined(CONFIG_VDSLBRCMPRIV1_SUPPORT)
Private void XdslMibAdjustMibDataPtr(void *gDslVars, int toLineNum)
{
    __SoftDslPrintf(gDslVars, "*** XdslMibAdjustMibDataPtr: toLineNum %d nTones = %d\n", toLineNum, globalVar.nTones);
    if(0 == toLineNum) {
        globalVar.snr = &gSnr[0];
        globalVar.showtimeMargin = &gShowtimeMargin[0];
        globalVar.bitAlloc = &gBitAlloc[0];
        globalVar.gain = &gGain[0];
        globalVar.bitSwapTones = &gBitSwapTones[0];
        globalVar.chanCharLin = &gChanCharLin[0];
        globalVar.chanCharLog = &gChanCharLog[0];
        globalVar.quietLineNoise = &gQuietLineNoise[0];
#if defined(CONFIG_RNC_SUPPORT)
        globalVar.quietLineNoiseRnc = &gQuietLineNoiseRnc[0];
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
        globalVar.uer = &gUER[0];
#ifdef SUPPORT_SELT
        globalVar.echoVariance = &gEchoVariance[0];
#endif
#endif
    }
    else {
        globalVar.snr = &gSnr[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
        globalVar.showtimeMargin = &gShowtimeMargin[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
        globalVar.bitAlloc = &gBitAlloc[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
        globalVar.gain = &gGain[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
        globalVar.bitSwapTones = &gBitSwapTones[0] + gLineId(gDslVars) * (MAX_BITSWAP_TONES/2);
        globalVar.chanCharLin = &gChanCharLin[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
        globalVar.chanCharLog = &gChanCharLog[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
        globalVar.quietLineNoise = &gQuietLineNoise[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#if defined(CONFIG_RNC_SUPPORT)
        globalVar.quietLineNoiseRnc = &gQuietLineNoiseRnc[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
        globalVar.uer = &gUER[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#ifdef SUPPORT_SELT
        globalVar.echoVariance = &gEchoVariance[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#endif
#endif
    }
}
#endif

#ifdef CONFIG_BCM_DSL_GFAST
Private void XdslMibAdjustGfastMibDataPtr(void *gDslVars, int toLineNum)
{
    __SoftDslPrintf(gDslVars, "*** XdslMibAdjustGfastMibDataPtr: toLineNum %d nTonesGfast = %d\n", toLineNum, globalVar.nTonesGfast);
    if(0 == toLineNum) {
        globalVar.activeLineNoise = &gActiveLineNoise[0];
        globalVar.doiBitAlloc = &gDoiBitAlloc[0];
        globalVar.doiGain = &gDoiGain[0];
    }
    else {
        globalVar.activeLineNoise = &gActiveLineNoise[0] + gLineId(gDslVars) * (MAX_GFAST_TONE_NUM/2);
        globalVar.doiBitAlloc = &gDoiBitAlloc[0] + gLineId(gDslVars) * (MAX_GFAST_TONE_NUM/2);
        globalVar.doiGain = &gDoiGain[0] + gLineId(gDslVars) * (MAX_GFAST_TONE_NUM/2);
    }
}
#endif

#endif  /* SUPPORT_DSL_BONDING */

Public void AdslMibClearData(void *gDslVars)
{
    BlockByteClear (sizeof(adslMibVarsStruct), (void*)&globalVar);

    globalVar.notifyHandlerPtr = AdslMibNotifyIdle;

    globalVar.adslMib.adslLine.adslLineCoding = kAdslLineCodingDMT;
    globalVar.adslMib.adslLine.adslLineType  = kAdslLineTypeFastOrIntl;

    globalVar.adslMib.adslPhys.adslCurrOutputPwr = 130;
    globalVar.adslMib.adslChanIntlAtmPhyData.atmInterfaceOCDEvents = 0;
    globalVar.adslMib.adslChanIntlAtmPhyData.atmInterfaceTCAlarmState = kAtmPhyStateLcdFailure;
    globalVar.adslMib.adslChanFastAtmPhyData.atmInterfaceOCDEvents = 0;
    globalVar.adslMib.adslChanFastAtmPhyData.atmInterfaceTCAlarmState = kAtmPhyStateLcdFailure;
    globalVar.adslMib.adslFramingMode = 3;
    globalVar.showtimeMarginThld = -1;
}
Public void AdslMibResetConectionStatCounters(void *gDslVars)
{
#ifdef CONFIG_VDSL_SUPPORTED
    adslBondingStat bondingStat;
#endif
    uint n = globalVar.adslMib.xdslStat[0].fireStat.status;
    uint n0 = globalVar.adslMib.xdslStat[0].ginpStat.status;
    uint n1 = globalVar.adslMib.xdslStat[MAX_LP_NUM-1].ginpStat.status;
#ifdef CONFIG_VDSL_SUPPORTED
    BlockByteMove(sizeof(adslBondingStat), (uchar *)&globalVar.adslMib.xdslStat[0].bondingStat, (uchar *)&bondingStat);  /* No dual latency in bonding */
#endif
    BlockByteClear (sizeof(globalVar.adslMib.xdslStat), (void *) &globalVar.adslMib.xdslStat);
    
    globalVar.adslMib.xdslStat[0].fireStat.status=n;
    globalVar.adslMib.xdslStat[0].ginpStat.status=n0;
    globalVar.adslMib.xdslStat[MAX_LP_NUM-1].ginpStat.status=n1;
#ifdef CONFIG_VDSL_SUPPORTED
    BlockByteMove(sizeof(adslBondingStat), (uchar *)&bondingStat, (uchar *)&globalVar.adslMib.xdslStat[0].bondingStat);
#endif
    BlockByteClear (sizeof(globalVar.adslMib.atmStat2lp), (void *) &globalVar.adslMib.atmStat2lp);
    BlockByteClear (sizeof(globalVar.adslMib.adslPerfData), (void *) &globalVar.adslMib.adslPerfData);
    BlockByteClear (sizeof(globalVar.adslMib.rtxCounterData), (void *) &globalVar.adslMib.rtxCounterData);
#ifdef CONFIG_BCM_DSL_GFAST
    BlockByteClear (sizeof(globalVar.adslMib.gfastOlrXoiCounterData), (void *) &globalVar.adslMib.gfastOlrXoiCounterData[0]);
#endif
    BlockByteClear (sizeof(globalVar.adslMib.adslChanIntlPerfData), (void *) &globalVar.adslMib.adslChanIntlPerfData);
    BlockByteClear (sizeof(globalVar.adslMib.adslChanFastPerfData), (void *) &globalVar.adslMib.adslChanFastPerfData);
    BlockByteClear (sizeof(globalVar.adslMib.adslPerfIntervals), (void *) &globalVar.adslMib.adslPerfIntervals);
    BlockByteClear (sizeof(globalVar.adslMib.xdslChanPerfIntervals), (void *) &globalVar.adslMib.xdslChanPerfIntervals);
    BlockByteClear (sizeof(globalVar.adslMib.xdslStatSincePowerOn), (void *) &globalVar.adslMib.xdslStatSincePowerOn);
    BlockByteClear (sizeof(globalVar.adslMib.atmStatSincePowerOn2lp), (void *) &globalVar.adslMib.atmStatSincePowerOn2lp);
    BlockByteClear (sizeof(globalVar.adslMib.atmStatCurDay2lp), (void *) &globalVar.adslMib.atmStatCurDay2lp);
    BlockByteClear (sizeof(globalVar.adslMib.atmStatPrevDay2lp), (void *) &globalVar.adslMib.atmStatPrevDay2lp);
    BlockByteClear (sizeof(globalVar.adslMib.atmStatCur15Min2lp), (void *) &globalVar.adslMib.atmStatCur15Min2lp);
    BlockByteClear (sizeof(globalVar.adslMib.adslTxPerfTotal), (void *) &globalVar.adslMib.adslTxPerfTotal);
    BlockByteClear (sizeof(globalVar.adslMib.adslTxPerfCur15Min), (void *) &globalVar.adslMib.adslTxPerfCur15Min);
    BlockByteClear (sizeof(globalVar.adslMib.adslTxPerfCur1Day), (void *) &globalVar.adslMib.adslTxPerfCur1Day);
    BlockByteClear (sizeof(globalVar.adslMib.adslTxPerfLast15Min), (void *) &globalVar.adslMib.adslTxPerfLast15Min);
    BlockByteClear (sizeof(globalVar.adslMib.adslTxPerfLast1Day), (void *) &globalVar.adslMib.adslTxPerfLast1Day);
    BlockByteClear (sizeof(globalVar.adslMib.adslTxPerfSinceShowTime), (void *) &globalVar.adslMib.adslTxPerfSinceShowTime);
    globalVar.txShowtimeTime=0;
    globalVar.secElapsedShTm=0;
}

#define kAdslConnected  (kAdslXmtActive | kAdslRcvActive) 

Public Boolean AdslMibIsLinkActive(void *gDslVars)
{
    return (kAdslConnected == (globalVar.linkStatus & kAdslConnected));
}

Public int AdslMibTrainingState (void *gDslVars)
{
    return (globalVar.adslMib.adslTrainingState);
}

Public int AdslMibPowerState(void *gDslVars)
{
    return (globalVar.adslMib.xdslInfo.pwrState);
}

Public void AdslMibSetNotifyHandler(void *gDslVars, adslMibNotifyHandlerType notifyHandlerPtr)
{
    globalVar.notifyHandlerPtr = notifyHandlerPtr ? notifyHandlerPtr : AdslMibNotifyIdle;
}

Public Boolean XdslMibIsPhyRActive(void *gDslVars, unsigned char dir)
{
    Boolean res = false;
    
#ifdef CONFIG_VDSL_SUPPORTED
    if(XdslMibIsVdsl2Mod(gDslVars)) {
        uint fireStat = 0;
        if(DS_DIRECTION == dir)
            fireStat = kFireDsEnabled;
        else if(US_DIRECTION == dir)
            fireStat = kFireUsEnabled;
        res = (globalVar.adslMib.xdslStat[0].fireStat.status & fireStat)? true: false;
    }
    else
#endif
    {
        /* No US PhyR for ADSL2/2+ */
        if(DS_DIRECTION == dir)
            res = (globalVar.adslMib.xdslStat[0].fireStat.status & kFireDsEnabled)? true:false;
    }
    return res;
}

Public Boolean XdslMibIsDataCarryingPath(void *gDslVars, unsigned char pathId, unsigned char dir)
{
    unsigned char ahifChanId = globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[pathId].ahifChanId[0];
    return ((unsigned char)-1 != ahifChanId)? 1:0;
}

Public Boolean XdslMibIsGinpActive(void *gDslVars, unsigned char dir)
{
    Boolean res = false;
    uint ginpStat = 0;
#ifdef CONFIG_VDSL_SUPPORTED
    if(XdslMibIsVdsl2Mod(gDslVars)) {
        if(DS_DIRECTION == dir)
            ginpStat = kGinpDsEnabled;
        else if(US_DIRECTION == dir)
            ginpStat = kGinpUsEnabled;
        res = (globalVar.adslMib.xdslStat[0].ginpStat.status & ginpStat)? true: false;
    }
    else
#endif
    {
        /* No US G.inp for ADSL2/2+ */
        if(DS_DIRECTION == dir)
            res = (globalVar.adslMib.xdslStat[0].ginpStat.status & kGinpDsEnabled)? true:false;
    }
    return res;
}

Public Boolean XdslMibIs2lpActive(void *gDslVars, unsigned char dir)
{
    Boolean res = false;

    if(RX_DIRECTION == dir)
        res = globalVar.adslMib.lp2Active;
    else if(TX_DIRECTION == dir)
        res = globalVar.adslMib.lp2TxActive;
    
    return res;
}

Public Boolean  AdslMibInit(void *gDslVars, dslCommandHandlerType commandHandler)
{
    int i;
    
    AdslMibClearData(gDslVars);
    globalVar.nTones = kAdslMibToneNum;
    globalVar.cmdHandlerPtr = commandHandler;
#ifdef G992_ANNEXC
    globalVar.nTones = kAdslMibToneNum*2;
#endif
    AdslMibSetNotifyHandler(gDslVars, NULL);
    globalVar.xdslInitIdleStateTime = 0;
    globalVar.txUpdateStatFlag = 0;
    globalVar.txCntReceived = 0;
    globalVar.secElapsedShTm = 0;
    globalVar.pathId = 0;
    globalVar.pathActive = 0;
    globalVar.swapRxPath = false;
    globalVar.swapTxPath = false;
    globalVar.minEFTRReported = false;
    globalVar.ginpExtEnabled = false;
    globalVar.adslMib.lp2Active = false;
    globalVar.adslMib.lp2TxActive = false;
    globalVar.adslMib.IkanosCO4Detected = false;
#ifdef SUPPORT_VECTORING
    globalVar.adslMib.reportVectoringCounter = false;
    globalVar.adslMib.fdps_us = false;
#endif
    globalVar.adslMib.adslNonLinData.NonLinThldNumAffectedBins=kDslNonLinearityDefaultThreshold;
    globalVar.adslMib.adslNonLinData.NonLinNumAffectedBins=-1;
    globalVar.adslMib.adslTrainingState = kAdslTrainingIdle;
    globalVar.adslMib.xdslInitializationCause = kXdslInitConfigSuccess;
    globalVar.lastInitState = kXdslLastInitStateStart;
    globalVar.adslTpsTcOptions = 0;
    globalVar.reInitTimeThld = 10;

    for(i=0; i<MAX_LP_NUM; i++) {
        globalVar.per2lp[i]=1;
        globalVar.PERpDS[i]=1;
    }
    
    globalVar.waitBandPlan=0;
    globalVar.waitfirstQLNstatusLD=0;
    globalVar.waitfirstHLOGstatusLD=0;
    globalVar.waitfirstSNRstatusLD=0;
    globalVar.waitfirstSNRstatusMedley=0;
    globalVar.bandPlanType=0;

    globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds =
    globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus =
    globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds =
    globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus =
    globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSds =
    globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSus =
    globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETds =
    globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETus = 1;
#ifdef CONFIG_BCM_DSL_GFAST
    globalVar.adslMib.gFactors.Gfactor_Gfast_mode = 0;
    XDSL_CLR_CAPTURE_FLAGS;
    globalVar.adslMib.adslStatSincePowerOn.gfastStat.txANDEFTRmin = (uint)-1;
    globalVar.adslMib.gfastSupportedOptions = 0;
#endif
    globalVar.adslMib.fastRetrainActive = false;

    AdslSetBandPlan(gDslVars);

    globalVar.snr = &gSnr[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
    globalVar.showtimeMargin = &gShowtimeMargin[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
    globalVar.bitAlloc = &gBitAlloc[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
    globalVar.gain = &gGain[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
    globalVar.bitSwapTones = &gBitSwapTones[0] + gLineId(gDslVars) * (MAX_BITSWAP_TONES/2);
    globalVar.chanCharLin = &gChanCharLin[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
    globalVar.chanCharLog = &gChanCharLog[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
    globalVar.quietLineNoise = &gQuietLineNoise[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#ifdef CONFIG_BCM_DSL_GFAST
    globalVar.activeLineNoise = &gActiveLineNoise[0] + gLineId(gDslVars) * (MAX_GFAST_TONE_NUM/2);
    globalVar.doiBitAlloc = &gDoiBitAlloc[0] + gLineId(gDslVars) * (MAX_GFAST_TONE_NUM/2);
    globalVar.doiGain = &gDoiGain[0] + gLineId(gDslVars) * (MAX_GFAST_TONE_NUM/2);
    globalVar.nTonesGfast = kVdslMibMaxToneNum;
#endif /* CONFIG_BCM_DSL_GFAST */
#if defined(CONFIG_RNC_SUPPORT)
    globalVar.quietLineNoiseRnc = &gQuietLineNoiseRnc[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
    globalVar.uer = &gUER[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#ifdef SUPPORT_SELT
    globalVar.echoVariance = &gEchoVariance[0] + gLineId(gDslVars) * (MAX_TONE_NUM/2);
#endif
#endif
#if defined(SUPPORT_VECTORING)
    globalVar.vectSM.state = VECT_DISABLED;
#endif
#ifdef SUPPORT_DSL_BONDING
    globalVar.adslMib.maxBondingDelay = 0;
#endif
   globalVar.adslMib.adslPerfData.adslSinceDrvStartedTimeElapsed=0L; /*reset the counters to 0 during MIB init*/
    return true;
}

Public int PHYSupportedNTones(void *gDslVars)
{
#ifdef  G993
  if ( (ADSL_PHY_SUPPORT(kAdslPhyVdslG993p2)) && (BcmAdslCoreGetConfiguredMod(gLineId(gDslVars)) & kG993p2AnnexA) ) {
#ifdef SUPPORT_DSL_BONDING
    if(ADSL_PHY_SUPPORT(kAdslPhyBonding))
      return MAX_TONE_NUM/2;
    else
#endif
      return  kVdslMibMaxToneNum;
  }
#endif
  if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p) || ADSL_PHY_SUPPORT(kAdslPhyAdsl2))
    return  kAdslMibToneNum*2;
  return kAdslMibToneNum;
}

Public void AdslMibTimer(void *gDslVars, uint timeMs)
{
    uint   sec;
    int path0, path1;
    path0 = XdslMibGetPath0MappedPathId(gDslVars, TX_DIRECTION);
    path1 = XdslMibGetPath1MappedPathId(gDslVars, TX_DIRECTION);

    if (globalVar.txUpdateStatFlag==1) {
        globalVar.txShowtimeTime+=timeMs;
    }
    
    if(globalVar.txShowtimeTime>3000 && globalVar.txUpdateStatFlag==1 )
    {
        ulonglong var64;
        
        var64 = globalVar.txShowtimeTime;
#ifndef CONFIG_ARM64
        do_div(var64, 1000);
#else
        var64 /= 1000;
#endif
        sec = (uint)var64;
        if(sec > globalVar.secElapsedShTm+1)
        {
            globalVar.secElapsedShTm=sec;
            globalVar.adslMib.adslPerfData.adslSincePrevLinkTimeElapsed += sec - globalVar.adslMib.adslPerfData.adslSinceLinkTimeElapsed;
            globalVar.adslMib.adslPerfData.adslSinceLinkTimeElapsed = sec;
            if(XdslMibIsXdsl2Mod(gDslVars))
            {
                var64 = globalVar.txShowtimeTime * globalVar.rsRate2lp[path0];
#ifndef CONFIG_ARM64
                do_div(var64, 1000);
#else
                var64 /= 1000;
#endif
                globalVar.adslMib.xdslStat[path0].xmtStat.cntRS = (uint)var64;
                if(!XdslMibIsGinpActive(gDslVars, TX_DIRECTION) ||
                    !XdslMibIsDataCarryingPath(gDslVars, path0, TX_DIRECTION)) {    /* OHM Channel */
                    var64 = globalVar.txShowtimeTime << 8;
#ifndef CONFIG_ARM64
                    do_div(var64, globalVar.per2lp[path0]);
#else
                    var64 /= globalVar.per2lp[path0];
#endif
                    globalVar.adslMib.xdslStatSincePowerOn[path0].xmtStat.cntSF += (var64 - globalVar.adslMib.xdslStat[path0].xmtStat.cntSF);
                    globalVar.adslMib.xdslStat[path0].xmtStat.cntSF = (uint)var64;
                }
                
                if(globalVar.adslMib.lp2TxActive)
                {
                    var64 = globalVar.txShowtimeTime*globalVar.rsRate2lp[path1];
#ifndef CONFIG_ARM64
                    do_div(var64, 1000);
#else
                    var64 /= 1000;
#endif
                    globalVar.adslMib.xdslStat[path1].xmtStat.cntRS = (uint)var64;
                    if(!XdslMibIsGinpActive(gDslVars, TX_DIRECTION) ||
                        !XdslMibIsDataCarryingPath(gDslVars, path1, TX_DIRECTION)) {    /* OHM Channel */
                        var64 = globalVar.txShowtimeTime << 8;
#ifndef CONFIG_ARM64
                        do_div(var64, globalVar.per2lp[path1]);
#else
                        var64 /= globalVar.per2lp[path1];
#endif
                        globalVar.adslMib.xdslStatSincePowerOn[path1].xmtStat.cntSF += (var64 - globalVar.adslMib.xdslStat[path1].xmtStat.cntSF);
                        globalVar.adslMib.xdslStat[path1].xmtStat.cntSF = (uint)var64;
                    }
                }
            }
            else
            {
                var64 = globalVar.txShowtimeTime;
#ifndef CONFIG_ARM64
                do_div(var64, 17);
#else
                var64 /= 17;
#endif
                globalVar.adslMib.adslStatSincePowerOn.xmtStat.cntSF += (var64 - globalVar.adslMib.adslStat.xmtStat.cntSF);
                globalVar.adslMib.adslStat.xmtStat.cntSF = (uint)var64;
            }
        }
    }

    timeMs += globalVar.timeMs;
    sec = timeMs / 1000;
    globalVar.timeSec += sec;
    globalVar.timeMs  = timeMs - sec * 1000;
    globalVar.adslMib.adslPerfData.adslSinceDrvStartedTimeElapsed+=sec; /*accumulate the total time*/

    if (sec != 0) {
        globalVar.currSecondErrored = false;
        globalVar.currSecondSES     = false;
        globalVar.currSecondLOS     = false;
        globalVar.currSecondFEC     = false;
        globalVar.currSecondLCD     = false;
        globalVar.currSecondLOM     = false;
#ifdef CONFIG_BCM_DSL_GFAST
        globalVar.currSecondLOR     = false;
#endif
        if (AdslMibIsLinkActive(gDslVars)
#ifdef CONFIG_BCM_DSL_GFAST
            && !globalVar.adslMib.fastRetrainActive && !G992p3OvhMsgIsL3RspPending(gDslVars)
#endif
            )
        {
            if(globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOM)
                AdslMibUpdateLOM(gDslVars);
            if ((globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS)
#ifdef CONFIG_BCM_DSL_GFAST
                && !(XdslMibIsGfastMod(gDslVars) && globalVar.uasOnSes)
#endif
              )
            {
                globalVar.losEventOccurred = true;
            }
            if (!globalVar.uasOnSes) {
               IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslAS);
#ifdef CONFIG_BCM_DSL_GFAST
               if (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOR)
                  //AdslMibUpdateLOR(gDslVars);
                  globalVar.lorEventOccurred = true;
#endif
            }
        }
        else {
            int i;  /* When link is down, the frequency of status arrival sometimes more than 1 sec */
            for(i = 0; i < sec; i++)
                IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslUAS);
            globalVar.adslMib.adslTxPerfTotal.adslUAS += sec;
            globalVar.adslMib.adslTxPerfCur15Min.adslUAS += sec;
            globalVar.adslMib.adslTxPerfCur1Day.adslUAS += sec;
        }

        AdslMibUpdateIntervalCounters (gDslVars, sec);
    }

    if(kAdslTrainingIdle == globalVar.adslMib.adslTrainingState) {
        if(0 == globalVar.xdslInitIdleStateTime)
            globalVar.xdslInitIdleStateTime = globalVar.txShowtimeTime;
        else if((globalVar.txShowtimeTime - globalVar.xdslInitIdleStateTime) > NO_PEER_DETECT_TIMEOUT)
            globalVar.adslMib.xdslInitializationCause = kXdslInitConfigNoXTUDetected;
    }
    else
        globalVar.xdslInitIdleStateTime = 0;

    if( (0 != globalVar.bitSwapReqTime[RX_DIRECTION]) &&
        ((globalVar.txShowtimeTime - globalVar.bitSwapReqTime[RX_DIRECTION]) > BITSWAP_REQ_TIMEOUT) )
        globalVar.bitSwapReqTime[RX_DIRECTION] = 0;
    
    if( (0 != globalVar.bitSwapReqTime[TX_DIRECTION]) &&
        ((globalVar.txShowtimeTime - globalVar.bitSwapReqTime[TX_DIRECTION]) > BITSWAP_REQ_TIMEOUT) )
        globalVar.bitSwapReqTime[TX_DIRECTION] = 0;
}

Private void AdslMibSetBitAndGain(void *gDslVars, int tblIdx, void *buf, int bufBgSize, Boolean bShared)
{
    int     i;
    uchar   *bufPtr = buf;
    short valGi;
    short sub=UtilQ0LinearToQ4dB(512);

    if ((tblIdx + bufBgSize) > kAdslMibMaxToneNum)
        bufBgSize = kAdslMibMaxToneNum - tblIdx;

    for (i = 0; i < bufBgSize; i++) {
        if (!bShared || (0 == (globalVar.bitAlloc[tblIdx + i] | globalVar.gain[tblIdx + i]))) {
            globalVar.bitAlloc[tblIdx + i] = bufPtr[i << 1] & 0xF;
            valGi=(bufPtr[i << 1] >> 4) | ((uint)bufPtr[(i << 1) + 1] << 4);
            globalVar.gain[tblIdx + i] =(short)(2*(UtilQ0LinearToQ4dB(valGi)-sub));
        }
    }
}

Private void AdslMibSetGain(void *gDslVars, int tblIdx, void *buf, int bufBgSize, Boolean bShared)
{
    int     i;
    short   *bufPtr = buf;
    if ((tblIdx + bufBgSize) > kAdslMibMaxToneNum)
        bufBgSize = kAdslMibMaxToneNum - tblIdx;
    for (i = 0; i < bufBgSize; i++) {
        if (!bShared || (0 ==  globalVar.gain[tblIdx + i])) 
            globalVar.gain[tblIdx + i] = ADSL_ENDIAN_CONV_SHORT(bufPtr[i])>>3;
    }
}

#define AdslMibParseBitGain(p,inx,b,g)  do {        \
    inx = p->ind;                                   \
    b   = p->gb & 0xF;                              \
    g   = ((int) p->gain << 4) | (p->gb >> 4);     \
} while (0)

Private void *AdslMibUpdateBitGain(void *gDslVars, void *pOlrEntry, Boolean bAdsl2)
{
    int     index, gain;
    uchar   bits;

    if (bAdsl2) {
        dslOLRCarrParam2p   *p = pOlrEntry;
        AdslMibParseBitGain(p, index, bits, gain);
#ifdef ADSLDRV_LITTLE_ENDIAN
        index = ADSL_ENDIAN_CONV_USHORT((ushort)index);
#endif
        pOlrEntry = p + 1;
    }
    else {
        dslOLRCarrParam     *p = pOlrEntry;
        AdslMibParseBitGain(p, index, bits, gain);
        pOlrEntry = p + 1;
    }

    globalVar.bitAlloc[index] = bits;
    globalVar.gain[index]     = gain;

    return pOlrEntry;
}


Private void AdslMibSetBitAllocation(void *gDslVars, int tblIdx, void *buf, int bufSize, Boolean bShared)
{
    int     i;
    uchar   *bufPtr = buf;


    if ((tblIdx + bufSize) > kAdslMibMaxToneNum)
        bufSize = kAdslMibMaxToneNum - tblIdx;

    for (i = 0; i < bufSize; i++) {
        if (!bShared || (0 == (globalVar.bitAlloc[tblIdx + i]))) {
            if(bufPtr[i] <= 0xF)
                globalVar.bitAlloc[tblIdx + i] = bufPtr[i];
            else
                globalVar.bitAlloc[tblIdx + i] = 0;
        }
    }
}

Private void AdslMibSetModulationType(void *gDslVars, int mod)
{
    globalVar.adslMib.adslConnection.modType = 
        (globalVar.adslMib.adslConnection.modType & ~kAdslModMask) | mod;
}

Private int AdslMibGetAdsl2Mode(void *gDslVars, uint mask)
{
    return globalVar.adslMib.xdslInfo.xdslMode & mask;
}

Private void AdslMibSetAdsl2Mode(void *gDslVars, uint mask, int mod)
{
    globalVar.adslMib.xdslInfo.xdslMode = (globalVar.adslMib.xdslInfo.xdslMode & ~mask) | mod;
}

Private void XdslMibSetXdslMode(void *gDslVars, uint mask, int mod)
{
    globalVar.adslMib.xdslInfo.xdslMode = (globalVar.adslMib.xdslInfo.xdslMode & ~mask) | mod;
}

#define XdslMibSetAnnexType(gv,mod)    XdslMibSetXdslMode(gv,kXdslModeAnnexMask,mod)

#define AdslMibGetAnnexMType(gv)        AdslMibGetAdsl2Mode(gv,kAdsl2ModeAnnexMask)
#define AdslMibSetAnnexMType(gv,mod)    AdslMibSetAdsl2Mode(gv,kAdsl2ModeAnnexMask,mod)

#define AdslMibGetAnnexLUpType(gv)      AdslMibGetAdsl2Mode(gv,kAdsl2ModeAnnexLUpMask)
#define AdslMibSetAnnexLUpType(gv,mod)  AdslMibSetAdsl2Mode(gv,kAdsl2ModeAnnexLUpMask,mod)
#define AdslMibGetAnnexLDnType(gv)      AdslMibGetAdsl2Mode(gv,kAdsl2ModeAnnexLDnMask)
#define AdslMibSetAnnexLDnType(gv,mod)  AdslMibSetAdsl2Mode(gv,kAdsl2ModeAnnexLDnMask,mod)

Private Boolean AdslMibTone32_64(void *gDslVars)
{
#ifdef G992P1_ANNEX_B
    if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB))
        return true;
#endif
    if (AdslMibGetAnnexMType(gDslVars) != 0)
        return true;

    return false;
}


/********* ADSL bandplans *******
Modulation:       US              DS
G.lite:               0-31           32-255
ADSL1:
ADSL2:
  AnnexA:          0-31           32-255
  AnnexM:          0-63           64-255
  AnnexB:          32-63          64-255
  AnnexL(w/n):     0-31           32-255
ADSL2+:
  AnnexA:          0-31           32-511
  AnnexB:          32-63          64-511
  AnnexM:          0-63           32-511
********************************/
Private void AdslSetBandPlan(void *gDslVars)
{
    unsigned short startToneUS=0, endToneUS=0, startToneDS=0, endToneDS=0;
    
    globalVar.adslMib.usNegBandPlan.noOfToneGroups=1;
    globalVar.adslMib.dsNegBandPlan.noOfToneGroups=1;
    globalVar.adslMib.usNegBandPlan32.noOfToneGroups=1;
    globalVar.adslMib.dsNegBandPlan32.noOfToneGroups=1;
    if(AdslMibGetModulationType(gDslVars)!=kAdslModAdsl2p){
        endToneDS=255;
        if(ADSL_PHY_SUPPORT(kAdslPhyAnnexA)){
            startToneUS = 0;
            if(AdslMibGetAnnexMType(gDslVars) != 0) {
                endToneUS = 63;
                startToneDS=64;
            }
            else{
                endToneUS = 31;
                startToneDS=32;
            }            
        }
        else{
                startToneUS = 32;
                endToneUS = 63;
                startToneDS=64;
            }
    }           

    else {
        endToneDS=511;
        if(ADSL_PHY_SUPPORT(kAdslPhyAnnexA)){
            startToneUS = 0;
            if(AdslMibGetAnnexMType(gDslVars) != 0) {
                endToneUS = 63;
                startToneDS=64;
            }
            else{
                endToneUS = 31;
                startToneDS=32;
            }
        }
        else{
                startToneUS = 32;
                endToneUS = 63;
                startToneDS=64;
            }
    }

    globalVar.adslMib.usNegBandPlan.toneGroups[0].startTone=startToneUS;
    globalVar.adslMib.usNegBandPlan.toneGroups[0].endTone=endToneUS;
    globalVar.adslMib.dsNegBandPlan.toneGroups[0].startTone=startToneDS;
    globalVar.adslMib.dsNegBandPlan.toneGroups[0].endTone=endToneDS;
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
    if(0 == gLineId(gDslVars))
#endif
    __SoftDslPrintf(gDslVars, "AdslMibGetModulationType= %d startToneUS: %d, startToneDS: %d endToneUS %d endToneDS = %d\n", 0,AdslMibGetModulationType(gDslVars), globalVar.adslMib.usNegBandPlan.toneGroups[0].startTone, globalVar.adslMib.dsNegBandPlan.toneGroups[0].startTone, globalVar.adslMib.usNegBandPlan.toneGroups[0].endTone,globalVar.adslMib.dsNegBandPlan.toneGroups[0].endTone); 
    AdslMibByteMove(sizeof(globalVar.adslMib.usPhyBandPlan), &globalVar.adslMib.usNegBandPlan, &globalVar.adslMib.usPhyBandPlan);
    AdslMibByteMove(sizeof(globalVar.adslMib.usNegBandPlanDiscovery), &globalVar.adslMib.usNegBandPlan, &globalVar.adslMib.usNegBandPlanDiscovery);
    AdslMibByteMove(sizeof(globalVar.adslMib.usPhyBandPlanDiscovery), &globalVar.adslMib.usNegBandPlan, &globalVar.adslMib.usPhyBandPlanDiscovery);
    
    AdslMibByteMove(sizeof(globalVar.adslMib.dsPhyBandPlan), &globalVar.adslMib.dsNegBandPlan, &globalVar.adslMib.dsPhyBandPlan);
    AdslMibByteMove(sizeof(globalVar.adslMib.dsNegBandPlanDiscovery), &globalVar.adslMib.dsNegBandPlan, &globalVar.adslMib.dsNegBandPlanDiscovery);
    AdslMibByteMove(sizeof(globalVar.adslMib.dsPhyBandPlanDiscovery), &globalVar.adslMib.dsNegBandPlan, &globalVar.adslMib.dsPhyBandPlanDiscovery); 
    
    globalVar.adslMib.usNegBandPlan32.toneGroups[0].startTone=startToneUS;
    globalVar.adslMib.usNegBandPlan32.toneGroups[0].endTone=endToneUS;
    globalVar.adslMib.dsNegBandPlan32.toneGroups[0].startTone=startToneDS;
    globalVar.adslMib.dsNegBandPlan32.toneGroups[0].endTone=endToneDS;
    
    AdslMibByteMove(sizeof(globalVar.adslMib.usNegBandPlanDiscovery32), &globalVar.adslMib.usNegBandPlan32, &globalVar.adslMib.usNegBandPlanDiscovery32);
    AdslMibByteMove(sizeof(globalVar.adslMib.dsNegBandPlanDiscovery32), &globalVar.adslMib.dsNegBandPlan32, &globalVar.adslMib.dsNegBandPlanDiscovery32);
    
    
    globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds =
    globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus =
    globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds =
    globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus =
    globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSds =
    globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSus =
    globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETds =
    globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETus = 1;
#ifdef CONFIG_BCM_DSL_GFAST
    globalVar.adslMib.gFactors.Gfactor_Gfast_mode = 0;
#endif
    globalVar.dsBpQLNForReport=
    globalVar.dsBpHlogForReport=
    globalVar.dsBpSNRForReport=
    globalVar.dsBpSATNpbForReport=
    globalVar.dsBpSNRpbForReport=&globalVar.adslMib.dsNegBandPlan;
    globalVar.usBpSNRpbForReport=
    globalVar.usBpSATNpbForReport=&globalVar.adslMib.usNegBandPlan;
    globalVar.dsGfactorForSNRReport =
    globalVar.dsPhysGfactor = &globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSds;
}
Private int AdslMibGetTotalNumOfLoadedTones(void *gDslVars, unsigned char dir)
{
    int totalNumOfLoadedTones;
    
    if( TX_DIRECTION == dir ) {
        if(AdslMibIsAdsl2Mod(gDslVars) && ADSL_PHY_SUPPORT(kAdslPhyAnnexA) && (0 != AdslMibGetAnnexMType(gDslVars))) {
            totalNumOfLoadedTones = 64;   /* AnnexM: 63-0+1 */
        }
        else {
            totalNumOfLoadedTones = 32;   /* AnnexA: 31-0+1; AnnexB:63-32+1; AnnexL:31-0+1 */
        }
    }
    else {
        if(kAdslModAdsl2p == AdslMibGetModulationType(gDslVars)) {
            if(ADSL_PHY_SUPPORT(kAdslPhyAnnexA))
                totalNumOfLoadedTones = 480;  /* 511-32+1 */
            else
                totalNumOfLoadedTones = 448;  /* AnnexB: 511-64+1 */
        }
        else {
            if(ADSL_PHY_SUPPORT(kAdslPhyAnnexA) && (0 == AdslMibGetAnnexMType(gDslVars)))
                totalNumOfLoadedTones = 224;  /* 255-32+1 */
            else
                totalNumOfLoadedTones = 192;   /* AnnexM/B: 255-64+1 */
        }
    }

    return totalNumOfLoadedTones;
}

Public void AdslMibUpdateACTPSD(void *gDslVars, int ACTATP, unsigned char dir)
{
    int *pActPsd, nLoadedTones;
    
    if(!XdslMibIsVdsl2Mod(gDslVars)) {
#if defined(CONFIG_VDSL_SUPPORTED)
        pActPsd = (TX_DIRECTION==dir)? &globalVar.adslMib.xdslPhys.actualPSD: &globalVar.adslMib.xdslAtucPhys.actualPSD;
#else
        pActPsd = (TX_DIRECTION==dir)? &globalVar.adslMib.adslPhys.actualPSD: &globalVar.adslMib.adslAtucPhys.actualPSD;
#endif
        nLoadedTones = AdslMibGetTotalNumOfLoadedTones(gDslVars, dir);
        /* ACTPSD(dBm/Hz) = ACTATP(dBm) - (10log10(tone_sapcing) + 10log10(number_of_loaded tones)) */
        *pActPsd = ACTATP - (UtilQ0LinearToQ4dB(4312) + UtilQ0LinearToQ4dB(nLoadedTones));
        *pActPsd = Q4ToTenth(*pActPsd);
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
         if(0 == gLineId(gDslVars))
#endif
        DiagWriteString(gLineId(gDslVars), DIAG_DSL_CLIENT, "%s ACTPSD: %d/10 dBm/Hz, ACTATP: %d/10 dBm, nLoadedTones: %d\n",
            (TX_DIRECTION==dir)? "US": "DS", *pActPsd, Q4ToTenth(ACTATP), nLoadedTones);
    }
}

Private void XdslMibSetActivePath(void *gDslVars, unsigned char dir, int pathId)
{
    if(TX_DIRECTION == dir )
      globalVar.pathActive |= (1 << (2+pathId));
    else if(RX_DIRECTION == dir )
      globalVar.pathActive |= (1 << pathId);
}


Private Boolean XdslMibIsAllPathActive(void *gDslVars, unsigned char dir)
{
    uint mask = 0;
    
    if(TX_DIRECTION == dir ) {
       if(globalVar.adslMib.lp2TxActive)
           mask = 3 << 2;
       else
           mask = 1 << 2;
    }
    else if(RX_DIRECTION == dir ) {
        if(globalVar.adslMib.lp2Active)
            mask = 3;
        else
            mask = 1;
    }
    else
        return false;

    return ( (globalVar.pathActive & mask) == mask );

}

Private void AdslMibCalcAdslPERp(void *gDslVars, int pathId, int dir)
{
    uint per, rsRate, den, UG;
    
    xdslFramingInfo   *p = &globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[pathId];
    UG = p->U*p->G;
    den = (p->M*p->L);
    per =(den) ? (((2*p->T*((uint)p->M*(p->B[0]+1)+p->R)*UG) << 8)/den) : 1;
    if(0 == per)
        per = 1;

    den = (8*p->N);
    rsRate = (den) ? ((uint)p->L*4000)/den : 0;

    if(TX_DIRECTION == dir) {
        globalVar.rsRate2lp[pathId] = rsRate;
        globalVar.per2lp[pathId] = per;
    }
    else
        globalVar.PERpDS[pathId] = per;
}

#ifdef CONFIG_VDSL_SUPPORTED
Private void AdslMibCalcVdslPERp(void *gDslVars, int pathId, int dir)
{
   uint             per, rsRate, den;
   xdslFramingInfo   *pXdslFramingParam = &globalVar.adslMib.xdslInfo.dirInfo[dir].lpInfo[pathId];

   den = (8*pXdslFramingParam->N);
   rsRate = (den) ? (pXdslFramingParam->L * 4000)/den : 0;
   if (kVdslProfile30a == globalVar.adslMib.xdslInfo.vdsl2Profile)
       rsRate <<= 1;

   den = (pXdslFramingParam->M*pXdslFramingParam->L);
   if (kVdslProfile30a == globalVar.adslMib.xdslInfo.vdsl2Profile)
       den <<= 1;
   per = (den) ? (((uint)(2*pXdslFramingParam->T * pXdslFramingParam->U * pXdslFramingParam->N) << 8)/den) : 1; /* Q8 */
   if(0 == per)
   per = 1;

   if(TX_DIRECTION == dir) {
     globalVar.rsRate2lp[pathId] = rsRate;
     globalVar.per2lp[pathId] = per;
   }
   else
     globalVar.PERpDS[pathId] = per;
}
#endif

void AdslMibInitTxStat(
  void          *gDslVars, 
  adslConnectionDataStat  *adslTxData, 
  adslPerfCounters    *adslTxPerf, 
  ginpCounters          *pGinpCounters)
{
  adslMibInfo               *pMib = &globalVar.adslMib;

  BlockByteClear(sizeof(adslPerfCounters), (void*)&pMib->adslTxPerfSinceShowTime);
  BlockByteMove(sizeof(adslPerfCounters), (void *)adslTxPerf, (void*)&pMib->adslTxPerfLast);
  BlockByteMove(sizeof(ginpCounters), (void *)pGinpCounters, (void*)&globalVar.lastTxGinpCounters);
}

Public int XdslMibGetCurrentMappedPathId(void *gDslVars, unsigned char dir)
{
    int pathId = globalVar.pathId;
    
    if(TX_DIRECTION == dir ) {
      if(true == globalVar.swapTxPath)
          pathId ^= 1;
    }
    else if(RX_DIRECTION == dir ) {
        if(true == globalVar.swapRxPath)
            pathId ^= 1;
    }
    
    return pathId;
}

Public int XdslMibGetPath0MappedPathId(void *gDslVars, unsigned char dir)
{
    int pathId = 0;
    
    if(TX_DIRECTION == dir ) {
      if(true == globalVar.swapTxPath)
          pathId = 1;
    }
    else if(RX_DIRECTION == dir ) {
        if(true == globalVar.swapRxPath)
            pathId = 1;
    }
    
    return pathId;
}
Public int XdslMibGetPath1MappedPathId(void *gDslVars, unsigned char dir)
{
    int pathId = 1;
    
    if(TX_DIRECTION == dir ) {
      if(true == globalVar.swapTxPath)
          pathId = 0;
    }
    else if(RX_DIRECTION == dir ) {
        if(true == globalVar.swapRxPath)
            pathId = 0;
    }
    
    return pathId;
}

Public void XdslMibGinpEFTRminReported(void *gDslVars)
{
    globalVar.minEFTRReported = true;
}

Public void XdslMibNotifyBitSwapReq(void *gDslVars, unsigned char dir)
{
    globalVar.bitSwapReqTime[dir] = globalVar.txShowtimeTime;
}

Public void XdslMibNotifyBitSwapRej(void *gDslVars, unsigned char dir)
{
    globalVar.bitSwapReqTime[dir] = 0;
}

Public void AdslMibUpdateTxStat(
  void          *gDslVars, 
  adslConnectionDataStat  *adslTxData, 
  adslPerfCounters    *adslTxPerf, 
  atmConnectionDataStat *atmTxData,
  ginpCounters          *pGinpCounters)
{
  adslMibInfo       *pMib = &globalVar.adslMib;
  adslPerfCounters    txPerfTemp;
  uint       n, updateTimeDiff;
  int       i;
  adslChanPerfDataEntry     *pChanPerfData;
  
  if(!globalVar.adslMib.lp2Active && !globalVar.adslMib.lp2TxActive )
    pChanPerfData = (kAdslIntlChannel == globalVar.adslMib.adslConnection.chType) ? &globalVar.adslMib.adslChanIntlPerfData : &globalVar.adslMib.adslChanFastPerfData;
  else
    pChanPerfData = &globalVar.adslMib.xdslChanPerfData[0];

  if(!globalVar.txCntReceived) {
    globalVar.txCntReceived++;
    globalVar.txCountersReportedTime = globalVar.txShowtimeTime;
    AdslMibInitTxStat(gDslVars, adslTxData, adslTxPerf, pGinpCounters);
    for(i=0; i<MAX_LP_NUM; i++) {
      if( (0==i) || XdslMibIs2lpActive(gDslVars, TX_DIRECTION) ) {
        globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFEBE] = adslTxData[i].cntSFErr;
        globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFECC] = adslTxData[i].cntRSCor;
        if( XdslMibIsAtmConnectionType(gDslVars) ) {
          globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFHEC] = atmTxData[i].cntHEC;
          globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfCellTotal]=atmTxData[i].cntCellTotal;
          globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfCellData]= atmTxData[i].cntCellData;
          globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfBitErrs]= atmTxData[i].cntBitErrs;
        }
      }
    }
  }
  updateTimeDiff = globalVar.txShowtimeTime - globalVar.txCountersReportedTime;
  for(i=0; i<MAX_LP_NUM; i++) {
    if( (0 ==i ) || XdslMibIs2lpActive(gDslVars, TX_DIRECTION) ) {
      n = adslTxData[i].cntSFErr - globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFEBE];
#ifdef BOGUS_CRC_DEBUG
    __SoftDslPrintf(gDslVars, "diffCRC: %ld < ((%ld << 8)/%ld): %ld;  txShowtimeTime=%ld, cnSFErr=%ld\n", 0,
                         n, updateTimeDiff, globalVar.per2lp[i], ((updateTimeDiff<<8)/globalVar.per2lp[i]), globalVar.txShowtimeTime, adslTxData[i].cntSFErr);
#endif
      if ( n < ((updateTimeDiff<<8)/globalVar.per2lp[i]) )   /* per2lp was stored in Q8 */
      {
        pMib->xdslStat[i].xmtStat.cntSFErr += n;
        pMib->xdslStatSincePowerOn[i].xmtStat.cntSFErr += n;
        pChanPerfData[i].perfTotal.adslChanTxCRC += n;
        pChanPerfData[i].perfCurr15Min.adslChanTxCRC += n;
        pChanPerfData[i].perfCurr1Day.adslChanTxCRC  += n;
#ifdef SUPPORT_24HR_CNT_STAT
        AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, ATUCCRCErrors, n);
#endif
      }
      n = adslTxData[i].cntRSCor - globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFECC];
      if(n < ((updateTimeDiff*globalVar.rsRate2lp[i])/1000))   /* rsRate is per sec */
      {
        pMib->xdslStat[i].xmtStat.cntRSCor += n;
        pMib->xdslStatSincePowerOn[i].xmtStat.cntRSCor += n;
        pChanPerfData[i].perfTotal.adslChanTxFEC += n;
        pChanPerfData[i].perfCurr15Min.adslChanTxFEC += n;
        pChanPerfData[i].perfCurr1Day.adslChanTxFEC  += n;
#ifdef SUPPORT_24HR_CNT_STAT
        AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, ATUCFECErrors, n);
#endif
      }

      if( XdslMibIsAtmConnectionType(gDslVars) ) {
     #ifdef DSL_REPORT_ALL_COUNTERS
        n = atmTxData[i].cntHEC - globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFHEC];
        if(n<=0x1000000)
        {
          IncAtmXmtCounterVar(&globalVar.adslMib, 0, cntHEC, n);
#ifdef SUPPORT_24HR_CNT_STAT
          AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, ATUCHECErrors, n);
#endif
        }
     #endif
        n = atmTxData[i].cntCellTotal - globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfCellTotal];
        if(n<=0x1000000)
          IncAtmXmtCounterVar(&globalVar.adslMib, i, cntCellTotal, n);
        n = atmTxData[i].cntCellData - globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfCellData];
        if(n<=0x1000000)
          IncAtmXmtCounterVar(&globalVar.adslMib, i, cntCellData, n);
        n = atmTxData[i].cntBitErrs - globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfBitErrs];
        if(n<=0x1000000)
          IncAtmXmtCounterVar(&globalVar.adslMib, i, cntBitErrs, n);
      }
    }
  }
  
  for(i=0; i<MAX_LP_NUM; i++) {
    if( (0==i) || XdslMibIs2lpActive(gDslVars, TX_DIRECTION) ) {
      globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFEBE] = adslTxData[i].cntSFErr;
      globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFECC] = adslTxData[i].cntRSCor;
      if( XdslMibIsAtmConnectionType(gDslVars) ) {
        globalVar.shtCounters2lp[i][kG992ShowtimeNumOfFHEC] = atmTxData[i].cntHEC;
        globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfCellTotal] = atmTxData[i].cntCellTotal;
        globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfCellData]= atmTxData[i].cntCellData;
        globalVar.shtExtCounters2lp[i][kG992ShowtimeNumOfBitErrs]= atmTxData[i].cntBitErrs;
      }
    }
  }
  
  BlockByteMove(sizeof(adslPerfCounters), (void *)adslTxPerf, (void*)&txPerfTemp);
  DiffTxPerfData(adslTxPerf, &pMib->adslTxPerfLast, adslTxPerf);
  AddTxPerfData(adslTxPerf, &pMib->adslTxPerfTotal, &pMib->adslTxPerfTotal);
  AddTxPerfData(adslTxPerf, &pMib->adslTxPerfCur15Min, &pMib->adslTxPerfCur15Min);
  AddTxPerfData(adslTxPerf, &pMib->adslTxPerfCur1Day, &pMib->adslTxPerfCur1Day);
  AddTxPerfData(adslTxPerf, &pMib->adslTxPerfSinceShowTime, &pMib->adslTxPerfSinceShowTime);

  if( XdslMibIsGinpActive(gDslVars, RX_DIRECTION)
      || XdslMibIsGinpActive(gDslVars, TX_DIRECTION)
#ifdef CONFIG_BCM_DSL_GFAST
      || XdslMibIsGfastMod(gDslVars)
#endif
    )
  {
    if( XdslMibIsGinpActive(gDslVars, RX_DIRECTION)
#ifdef CONFIG_BCM_DSL_GFAST
        || XdslMibIsGfastMod(gDslVars)
#endif
      )
    {
      n = pGinpCounters->rtx_tx - globalVar.lastTxGinpCounters.rtx_tx;
      AddGinpDsCounterVar(&globalVar.adslMib, rtx_tx, n);
      AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntDS, rtx_tx, n);
    }
    if( XdslMibIsGinpActive(gDslVars, TX_DIRECTION)
#ifdef CONFIG_BCM_DSL_GFAST
        || XdslMibIsGfastMod(gDslVars)
#endif
      )
    {
      n = pGinpCounters->rtx_uc - globalVar.lastTxGinpCounters.rtx_uc;
      AddGinpUsCounterVar(&globalVar.adslMib, rtx_uc, n);
      AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntUS, rtx_uc, n);
      
      globalVar.adslMib.adslStat.ginpStat.cntUS.minEFTR = pGinpCounters->minEFTR;
      globalVar.adslMib.adslStatSincePowerOn.ginpStat.cntUS.minEFTR = pGinpCounters->minEFTR;
      n = pGinpCounters->errFreeBits - globalVar.lastTxGinpCounters.errFreeBits;
      AddGinpUsCounterVar(&globalVar.adslMib, errFreeBits, n);
      if( XdslMibIsGinpActive(gDslVars, TX_DIRECTION) )
      {
        n = pGinpCounters->rtx_c - globalVar.lastTxGinpCounters.rtx_c;
        AddGinpUsCounterVar(&globalVar.adslMib, rtx_c, n);
        AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntUS, rtx_c, n);
        
        n = pGinpCounters->LEFTRS - globalVar.lastTxGinpCounters.LEFTRS;
        AddGinpUsCounterVar(&globalVar.adslMib, LEFTRS, n);
      }
    }
  }
  BlockByteMove(sizeof(adslPerfCounters), (void *)&txPerfTemp, (void*)&pMib->adslTxPerfLast);
  BlockByteMove(sizeof(ginpCounters), (void *)pGinpCounters, (void*)&globalVar.lastTxGinpCounters);
  globalVar.txCountersReportedTime = globalVar.txShowtimeTime;
}

Private int XdslMibGetMaxToneNum(void *gDslVars)
{
    int n;
#if defined(SUPPORT_DSL_BONDING)
#if defined(CONFIG_VDSLBRCMPRIV1_SUPPORT)
    unsigned short vdsl2Profile;
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
    vdsl2Profile = kVdslProfileBrcmPriv2;
#else
    vdsl2Profile = kVdslProfile35b;
#endif
    if(vdsl2Profile == globalVar.adslMib.xdslInfo.vdsl2Profile)
        n = MAX_TONE_NUM;
    else
#endif
        n = MAX_TONE_NUM/2;
#else /* !SUPPORT_DSL_BONDING */
    n = MAX_TONE_NUM;
#endif
    return n;
}

#ifdef CONFIG_VDSL_SUPPORTED
Private void copyBandPlanData(short *pSrcBuf, short *pDstBuf, bandPlanDescriptor *pBandPlan, int nMaxTone)
{
   int i, len = 0;
   for(i = 0; i < pBandPlan->noOfToneGroups; i++) {
      if(pBandPlan->toneGroups[i].endTone >= nMaxTone)
         break;
      pSrcBuf += len;
      len = pBandPlan->toneGroups[i].endTone - pBandPlan->toneGroups[i].startTone + 1;
      BlockShortMoveReverse(len, pSrcBuf, pDstBuf + pBandPlan->toneGroups[i].startTone);
   }
}

Private VdslToneGroup *getCorrespondingNegToneGroup(VdslToneGroup *pPhysToneGroup, bandPlanDescriptor *pNegBandPlanDesc)
{
   int i;
   VdslToneGroup *pNegToneGroup = (VdslToneGroup *)NULL;
   
   for(i = 0; i < pNegBandPlanDesc->noOfToneGroups; i++) {
      if(pPhysToneGroup->startTone <= pNegBandPlanDesc->toneGroups[i].startTone) {
         if(pPhysToneGroup->endTone >= pNegBandPlanDesc->toneGroups[i].endTone)
            pNegToneGroup = &pNegBandPlanDesc->toneGroups[i];
         else
            __SoftDslPrintf(XdslCoreGetCurDslVars(), "DRV MIB: No corresponding NEG toneGroup for PHYS toneGroup(%d, %d)", 0, pPhysToneGroup->startTone, pPhysToneGroup->endTone);
         break;
      }
   }
   
   return pNegToneGroup;
}

#endif

#ifdef CONFIG_BCM_DSL_GFAST
Private void updateGfastOlrCompletedCounter(int cntType, gfastOlrCounterInfo *pGfastOlrCounterInfo)
{
   switch(cntType) {
      case kOlrBSW:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, bswCompleted, 1);
         break;
      case kOlrSRA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, sraCompleted, 1);
         break;
      case kOlrTIGA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, tigaCompleted, 1);
         break;
      case kOlrRPA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, rpaCompleted, 1);
         break;
      case kOlrFRA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, fraCompleted, 1);
         break;
      default:
         break;
   }
}

Private void updateGfastOlrStartedCounter(int cntType, gfastOlrCounterInfo *pGfastOlrCounterInfo)
{
   switch(cntType) {
      case kOlrBSW:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, bswStarted, 1);
         break;
      case kOlrSRA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, sraStarted, 1);
         break;
      case kOlrTIGA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, tigaStarted, 1);
         break;
      case kOlrRPA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, rpaStarted, 1);
         break;
      case kOlrFRA:
         AddGfastOlrCounterVar(pGfastOlrCounterInfo, fraStarted, 1);
         break;
      default:
         break;
   }
}
#endif

Public void XdslMibCmdSnooper (void *gDslVars, dslCommandStruct *cmd)
{
  switch (DSL_COMMAND_CODE(cmd->command)) {
    case kDslStartPhysicalLayerCmd:
    case kDslIdleCmd:
    case kDslDownCmd:
      if(globalVar.adslMib.adslTrainingState == kAdslTrainingConnected) {
        if (globalVar.adslMib.xdslInfo.pwrState != 3) {
          gG992p3OvhMsgVars.phriFlag = true;
          IncFailureCounterVar(&globalVar.adslMib.adslPerfData, xdslHostInitRetr);
        }
      }
      break;
    default:
      break;
  }
}

Public int AdslMibStatusSnooper (void *gDslVars, dslStatusStruct *status)
{
  atmPhyDataEntrty          *pAtmData;
  adslChanPerfDataEntry     *pChanPerfData;
  int                     val, n, nb;
  uchar                     *pVendorId, *pSysVendorId;
  int                       pathId = globalVar.pathId;
  int                       retCode = 0;
  int                       tmpInt;
  int                       nMaxToneAllow = XdslMibGetMaxToneNum(gDslVars);

  switch (DSL_STATUS_CODE(status->code)) {
#ifdef CONFIG_TOD_SUPPORTED
    case kDslTODactiveLine:
      globalVar.todInfo.todStatus = status->param.value;
      break;
#endif
#if defined(CONFIG_BCM_DSL_GFAST)
    case kDslFastRetrain:
        globalVar.adslMib.fastRetrainActive = (0 == status->param.value)? false: true;
        if(globalVar.adslMib.fastRetrainActive) {
          if(globalVar.lorEventOccurred) {
            if(globalVar.losEventOccurred) {
              AdslMibES(globalVar.currSecondLOS, adslLOSS);
              globalVar.losEventOccurred = false;
            }
            AdslMibUpdateLOR(gDslVars);
            globalVar.lorEventOccurred = false;
          }
          else if(globalVar.losEventOccurred) {
            AdslMibUpdateLOS(gDslVars);
            globalVar.losEventOccurred = false;
          }
          XDSL_MIB_RESTORE_CNTRS(gDslVars);
          IncFailureCounterVar(&globalVar.adslMib.adslPerfData, xdslFastRetr);
        }
        AdslMibNotify(gDslVars, kXdslEventFastRetrain);
        break;
#endif
    case kDslOLRResponseStatus:
        val = status->param.value & 0xffff;
        if((kOLRDeferType1 == val) || (kOLRRejectType2 == val) || (kOLRRejectType3 == val))
            globalVar.adslMib.adslStat.bitswapStat.xmtCntRej++;
        break;
    case kDslDspControlStatus:
      switch(status->param.dslConnectInfo.code)
      {
        case kDslATURClockErrorInfo:
           globalVar.adslMib.transceiverClkError = status->param.dslConnectInfo.value;
           break;
        case kFireMonitoringCounters:
        {
          uint *pLong = (uint *)status->param.dslConnectInfo.buffPtr;
          adslFireStat *pFireStat = &globalVar.adslMib.adslStat.fireStat;
          val = status->param.dslConnectInfo.value;
          pFireStat->reXmtRSCodewordsRcved = ADSL_ENDIAN_CONV_INT32(pLong[kFireReXmtRSCodewordsRcved]);
          pFireStat->reXmtUncorrectedRSCodewords = ADSL_ENDIAN_CONV_INT32(pLong[kFireReXmtUncorrectedRSCodewords]);
          pFireStat->reXmtCorrectedRSCodewords = ADSL_ENDIAN_CONV_INT32(pLong[kFireReXmtCorrectedRSCodewords]);
          break;
        }
        case kDslG992RcvShowtimeUpdateGainPtr:
        {
          ushort   *buffPtr = status->param.dslConnectInfo.buffPtr;
          val = status->param.dslConnectInfo.value;
          if (AdslMibTone32_64(gDslVars)) {
            AdslMibSetGain(gDslVars, 32, buffPtr + 32, 32, true);
            AdslMibSetGain(gDslVars, 64, buffPtr + 64, val - 32, false);
          }
          else
            AdslMibSetGain(gDslVars, 32, buffPtr+32, val - 32, false);
#ifdef G992_ANNEXC
          if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
            AdslMibSetGain(gDslVars, kAdslMibToneNum, buffPtr+kAdslMibToneNum, 256, false);
            AdslMibSetGain(gDslVars, 2*kAdslMibToneNum+32, buffPtr+2*kAdslMibToneNum+32, 224+256, false);
          }
          else
            AdslMibSetGain(gDslVars, kAdslMibToneNum+32, buffPtr+kAdslMibToneNum+32, 224, false);
#endif
        }
          break;
        case kDslNLNoise:
          n = (status->param.dslConnectInfo.value <kAdslMibMaxToneNum ? status->param.dslConnectInfo.value : kAdslMibMaxToneNum);
          //AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.distNoisedB);
          BlockShortMoveReverse(n>>1, status->param.dslConnectInfo.buffPtr, globalVar.distNoisedB);
#ifdef SUPPORT_SELT
          AdslMibNotify(gDslVars, kXdslEventSeltNext);
#endif
          break;
#ifdef ADSL_MIBOBJ_PLN
        case kDslPLNPeakNoiseTablePtr:
          n=status->param.dslConnectInfo.value;
          //AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNValueps);
          BlockShortMoveReverse(n>>1, status->param.dslConnectInfo.buffPtr, globalVar.PLNValueps);
          break;
        case kDslPerBinThldViolationTablePtr:
          n=status->param.dslConnectInfo.value;
          //AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNThldCntps);
          BlockShortMoveReverse(n>>1, status->param.dslConnectInfo.buffPtr, globalVar.PLNThldCntps);
          break;
#endif
        case kDslImpulseNoiseDurationTablePtr:
        {
            int i;
            short *pShort = (short *)status->param.dslConnectInfo.buffPtr;
            n=status->param.dslConnectInfo.value;
            for(i=0;i<n/sizeof(short);i++)
               globalVar.PLNDurationHist[i]=ADSL_ENDIAN_CONV_SHORT(pShort[i]);
            break;
        }
        case kDslImpulseNoiseDurationTableLongPtr:
            n=status->param.dslConnectInfo.value;
            //AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNDurationHist);
            BlockLongMoveReverse(n>>2, status->param.dslConnectInfo.buffPtr, globalVar.PLNDurationHist);
            break;
        case kDslImpulseNoiseTimeTablePtr:
        {
            int i;
            short *pShort = (short *)status->param.dslConnectInfo.buffPtr;
            n=status->param.dslConnectInfo.value;
            for(i=0;i<n/sizeof(short);i++)
               globalVar.PLNIntrArvlHist[i]=ADSL_ENDIAN_CONV_SHORT(pShort[i]);
            if(globalVar.adslMib.adslPLNData.PLNUpdateData==0)
               globalVar.adslMib.adslPLNData.PLNUpdateData=1;
            break;
        }
        case kDslImpulseNoiseTimeTableLongPtr:
            n=status->param.dslConnectInfo.value;
          //AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNIntrArvlHist);
          BlockLongMoveReverse(n>>2, status->param.dslConnectInfo.buffPtr, globalVar.PLNIntrArvlHist);
          if(globalVar.adslMib.adslPLNData.PLNUpdateData==0)
              globalVar.adslMib.adslPLNData.PLNUpdateData=1;
          break;
        case kDslINMControTotalSymbolCount:
          globalVar.adslMib.adslPLNData.PLNBBCounter=status->param.dslConnectInfo.value;
          break;
        case kDslPLNMarginPerBin:
          globalVar.adslMib.adslPLNData.PLNThldPerTone=status->param.dslConnectInfo.value;
          break;
        case kDslPLNMarginBroadband:
          globalVar.adslMib.adslPLNData.PLNThldBB=status->param.dslConnectInfo.value;
          break;
        case kDslPerBinMsrCounter:
          globalVar.adslMib.adslPLNData.PLNPerToneCounter=status->param.dslConnectInfo.value;
          break;
        case kDslBroadbandMsrCounter:
          globalVar.adslMib.adslPLNData.PLNBBCounter=status->param.dslConnectInfo.value;
          break;
        case kDslInpBinTablePtr:
          n=status->param.dslConnectInfo.value;
          globalVar.adslMib.adslPLNData.PLNNbDurBins=n/2;
          //AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNDurationBins);
          BlockShortMoveReverse(n>>1,status->param.dslConnectInfo.buffPtr,globalVar.PLNDurationBins);
          break;
        case kDslItaBinTablePtr:
          n=status->param.dslConnectInfo.value;
          globalVar.adslMib.adslPLNData.PLNNbIntArrBins=n/2;
          //AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNIntrArvlBins);
          BlockShortMoveReverse(n>>1,status->param.dslConnectInfo.buffPtr,globalVar.PLNIntrArvlBins);
          break;
        case kDslPlnState:
          globalVar.adslMib.adslPLNData.PLNState=status->param.dslConnectInfo.value;
          break;
      }
      break; /* kDslDspControlStatus */
    case kDslTrainingStatus:
      val = status->param.dslTrainingInfo.value;
      switch (status->param.dslTrainingInfo.code) {
#ifdef SUPPORT_VECTORING
         case kDslVectoringReportErrorSampleCounters:
            globalVar.adslMib.reportVectoringCounter = true;
            break;
         case kDslVectoringFdpsUs:
            globalVar.adslMib.fdps_us = true;
            break;
         case kDslVectoringUseEoc:
            globalVar.adslMib.vectData.macAddress.addressType = 2;
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "DRV MIB: Use EOC for error samples",0);
           break;
#endif
         case kDslIkanosCO4Detected:
            globalVar.adslMib.IkanosCO4Detected = true;
            break;
         case kG992LDLastStateDs:
            globalVar.adslMib.adslDiag.ldLastStateDS = (unsigned short)val;
            break;
         case kG992LDLastStateUs:
            globalVar.adslMib.adslDiag.ldLastStateUS = (unsigned short)val;
            break;
         case kG992ControlAllRateOptionsFailedErr:
         case kDslG992BitAndGainVerifyFailed:
            globalVar.lastInitState = kXdslLastInitStateNotFeasible;
            globalVar.adslMib.xdslInitializationCause = kXdslInitConfigNotFeasible;
            break;
#ifdef CONFIG_VDSL_SUPPORTED
        case kDsl993p2Profile:
#if defined(CONFIG_BCM_DSL_GFAST)
            if(ADSL_PHY_SUPPORT(kAdslPhyGfast212a) && XdslMibIsGfastMod(gDslVars)) {
                val &= 0x1FFFF;
                if(PROFILEGFAST212C == val)
                   globalVar.adslMib.xdslInfo.vdsl2Profile = kGfastProfile212c;
                else
                   globalVar.adslMib.xdslInfo.vdsl2Profile = val;
            }
            else
#endif
            globalVar.adslMib.xdslInfo.vdsl2Profile = (ushort)(val & 0xFFFF);
#if defined(CONFIG_VDSLBRCMPRIV1_SUPPORT)
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
            n = kVdslProfileBrcmPriv2;
#else
            n = kVdslProfile35b;
#endif
            if(n == globalVar.adslMib.xdslInfo.vdsl2Profile) {
                globalVar.nTones = MAX_TONE_NUM;
#if defined(SUPPORT_DSL_BONDING)
                if(1 == gLineId(gDslVars))
                    XdslMibAdjustMibDataPtr(gDslVars, 0);
#endif
            }
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
            else if(kVdslProfile35b == globalVar.adslMib.xdslInfo.vdsl2Profile)
                globalVar.nTones = MAX_TONE_NUM/2;
#endif
#endif /* defined(CONFIG_VDSLBRCMPRIV1_SUPPORT) */

#if defined(CONFIG_BCM_DSL_GFAST)
            if(XdslMibIsGfastMod(gDslVars)) {
                if(ADSL_PHY_SUPPORT(kAdslPhyGfast212a) &&
                  ((kGfastProfile212a == globalVar.adslMib.xdslInfo.vdsl2Profile) ||
                   (kGfastProfile212c == globalVar.adslMib.xdslInfo.vdsl2Profile))) {
                    globalVar.nTonesGfast = MAX_GFAST_TONE_NUM;
#if defined(SUPPORT_DSL_BONDING)
                    if(1 == gLineId(gDslVars))
                        XdslMibAdjustGfastMibDataPtr(gDslVars, 0);
#endif
                }
            }
#endif
            globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds =
            globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus =
            globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds =
            globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus =
            globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSds =
            globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSus =
            globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETds =
            globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETus = 8;
          break;
        case kDsl993p2Annex:
            if( VDSL2_ANNEX_A == val )
                XdslMibSetAnnexType(gDslVars, (kAdslTypeAnnexA << kXdslModeAnnexShift));
            else if( VDSL2_ANNEX_B == val )
                XdslMibSetAnnexType(gDslVars, (kAdslTypeAnnexB << kXdslModeAnnexShift));
            else if( VDSL2_ANNEX_C == val )
                XdslMibSetAnnexType(gDslVars, (kAdslTypeAnnexC << kXdslModeAnnexShift));
          break;
#if defined(CONFIG_VDSL_SUPPORTED)
        case kDsl993p2LRActOpType:
          globalVar.adslMib.xdslInfo.vdslLRmode = val;
          break;
#endif
#ifdef CONFIG_BCM_DSL_GFAST
        case kG992DataRcvDetectLOR:
          if (0 == (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS)) {
            globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOR;
            globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusNoDefect;
            XDSL_MIB_CAPTURE_CNTRS(gDslVars, CAPTURE_CNTRS_ONLOR);
            //IncPerfCounterVar(&globalVar.adslMib.adslPerfData, xdslLors);
            //AdslMibUpdateLOR(gDslVars);
            if((false == (globalVar.adslMib.fastRetrainActive || globalVar.uasOnSes)) && !G992p3OvhMsgIsL3RspPending(gDslVars))
              globalVar.lorEventOccurred = true;
          }
          break;
        case kG992DataRcvDetectLORRecovery:
          globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOR;
          if(0 == globalVar.adslMib.adslPhys.adslCurrStatus)
            globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
          globalVar.inhibCntrsOnLOR.inhibitCntrCapture= 0;
          break;
        case kG992ReinitTimeThld:
          globalVar.reInitTimeThld = val;
          break;
        case kG992DataRcvDetectLOM:
          if (0 == (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS)) {
            globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOM;
            globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusNoDefect;
            if(!XdslMibIsGfastMod(gDslVars))
              IncPerfCounterVar(&globalVar.adslMib.adslPerfData, xdslLoms);
          }
          break;
        case kG992DataRcvDetectLOMRecovery:
          globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOM;
            if(0 == globalVar.adslMib.adslPhys.adslCurrStatus)
              globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
          break;
#endif
#endif
        case kDslG992LatencyPathId:
          globalVar.pathId = ((unsigned char)val < MAX_LP_NUM)? (unsigned char)val: 0;
          break;
        case kDslG992RxLatencyPathCount:
          globalVar.adslMib.lp2Active = (2 == val);
          break;
        case kDslG992TxLatencyPathCount:
          globalVar.adslMib.lp2TxActive = (2 == val);
          break;
        case kG992RcvDelay:
          globalVar.adslMib.adsl2Info2lp[pathId].rcv2DelayInp.delay = (unsigned short)val;
          break;
        case kG992RcvInp:
          globalVar.adslMib.adsl2Info2lp[pathId].rcv2DelayInp.inp = (unsigned short)val;
          break;
        case kG992FireState:
          globalVar.adslMib.xdslStat[0].fireStat.status = val;
          if (0 == (val & ~(kFireDsEnabled | kFireUsEnabled))) {
            globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].lpInfo[0].rtxMode = (val & kFireDsEnabled) ? 1 : 0;
            globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[0].rtxMode = (val & kFireUsEnabled) ? 1 : 0;
          }
          break;
        case kDslExtendedRateStatSupport:
          globalVar.ginpExtEnabled = true;
          break;
#ifdef CONFIG_BCM_DSL_GFAST
        case kDslGfastGfactor:
          globalVar.adslMib.gFactors.Gfactor_Gfast_mode = 1;
          break;
#endif
#if defined(SUPPORT_VECTORING)
        case kDslVectoringEnabled:
        case kDslVectoringFriendlyEnabled:
          globalVar.vectSM.state = VECT_WAIT_FOR_CONFIG;
          BlockByteClear(sizeof(VceMacAddress),(char *)&globalVar.adslMib.vectData.macAddress);
          break;
        case kDslVectoringState:
          globalVar.vectSM.state = val;
          break;
        case kDslVectoringLineId:
          globalVar.vectSM.lineId = val;
          break;
        case kDslVectoringPilotSequenceLength:
          globalVar.vectPhyData.pilotSequence.pilotSeqLengthInBytes = val;
          break;
#endif
        case kDslPtmOptionsDs:
          globalVar.adslTpsTcOptions |= (val & 1);
          break;
        case kDslPtmOptionsUs:
          globalVar.adslTpsTcOptions |= ((val & 1) << 1);
          break;
        case kDslBondingState:
          globalVar.adslMib.xdslStat[0].bondingStat.status = val;
          break;
        case kDslStartedG994p1:
        case kG994p1EventToneDetected:
        case kDslStartedT1p413HS:
        case kDslT1p413ReturntoStartup:
#if defined(SUPPORT_DSL_BONDING) && (defined(CONFIG_VDSLBRCMPRIV1_SUPPORT) || defined(CONFIG_BCM_DSL_GFAST))
            if(1 == gLineId(gDslVars)) {
#ifdef CONFIG_VDSLBRCMPRIV1_SUPPORT
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
                n = kVdslProfileBrcmPriv2;
#else
                n = kVdslProfile35b;
#endif
                if(n == globalVar.adslMib.xdslInfo.vdsl2Profile) {
                    XdslMibAdjustMibDataPtr(gDslVars, 1);
                    globalVar.nTones = kVdslMibMaxToneNum;
                }
#endif /* CONFIG_VDSLBRCMPRIV1_SUPPORT */
#if defined(CONFIG_BCM_DSL_GFAST)
                if(ADSL_PHY_SUPPORT(kAdslPhyGfast212a) &&
                   ((kGfastProfile212a == globalVar.adslMib.xdslInfo.vdsl2Profile) ||
                   (kGfastProfile212c == globalVar.adslMib.xdslInfo.vdsl2Profile))) {
                    XdslMibAdjustGfastMibDataPtr(gDslVars, 1);
                    globalVar.nTonesGfast = kVdslMibMaxToneNum;
                }
#endif
            }
#endif
            BlockByteClear(sizeof(globalVar.adslMib.xdslInfo), (void*)&globalVar.adslMib.xdslInfo);
#if defined(CONFIG_VDSL_SUPPORTED)
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
           if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            BlockByteClear(sizeof(globalVar.adslMib.vdslInfo), (void*)&globalVar.adslMib.vdslInfo[0]);
#endif
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
           if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            BlockByteClear(sizeof(globalVar.adslMib.adsl2Info2lp), (void*)&globalVar.adslMib.adsl2Info2lp[0]);
            globalVar.pathId = 0;
            globalVar.swapRxPath = false;
            globalVar.swapTxPath = false;
            globalVar.minEFTRReported = true;
            globalVar.adslMib.ginpExtRateStat.maxEFTRds = 0;
            globalVar.adslMib.ginpExtRateStat.maxEFTRus = 0;
            globalVar.ginpExtEnabled = false;
            globalVar.adslMib.lp2Active = false;
            globalVar.adslMib.lp2TxActive = false;
            globalVar.adslMib.IkanosCO4Detected = false;
#ifdef SUPPORT_VECTORING
            globalVar.adslMib.reportVectoringCounter = false;
            globalVar.adslMib.fdps_us = false;
#endif
#if defined(CONFIG_BCM_DSL_GFAST)
            globalVar.adslMib.gfastSupportedOptions = 0;
            globalVar.txANDEFTRacc = 0;
            globalVar.adslMib.adslStat.gfastStat.txANDEFTRsum = 0;
            globalVar.adslMib.adslStat.gfastStat.txANDEFTRDS = 0;
            globalVar.adslMib.adslStat.gfastStat.txLANDEFTRS = 0;
#endif
            globalVar.adslMib.xdslStat[0].fireStat.status=0;
            globalVar.adslMib.xdslStat[0].ginpStat.status=0;
            globalVar.adslMib.xdslStat[0].bondingStat.status=0;
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 0;
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            globalVar.adslMib.adslTrainingState = kAdslTrainingIdle;
            globalVar.vendorIdReceived = false;
            globalVar.rsOptionValid = false;
            globalVar.rsOption[0] = 0;
            globalVar.nMsgCnt = 0;
            globalVar.txUpdateStatFlag=0;
            n = globalVar.linkStatus;
            globalVar.linkStatus = 0;
            globalVar.pathActive = 0;
            globalVar.adslMib.adslNonLinData.NonLinNumAffectedBins=-1;
            globalVar.adslMib.adslNonLinData.NonLinearityFlag=0;
#if defined(SUPPORT_VECTORING)
            globalVar.vectSM.state = VECT_DISABLED;
#endif
#ifdef CO_G994_NSIF
            globalVar.adslMib.adslAtucPhys.nsifLen = 0;
#endif
            globalVar.reInitTimeThld = 10;
#ifdef CONFIG_TOD_SUPPORTED
            globalVar.todInfo.todStatus = 0;
#endif
            if (n != 0)
                AdslMibNotify(gDslVars, kAdslEventLinkChange);
            break;
#ifdef CO_G994_NSIF
        case kDslG994p1RcvNonStandardInfo:
        {
          int  AdslCoreGetOemParameter (int paramId, void *buf, int len);
          
          globalVar.adslMib.adslAtucPhys.nsifLen = AdslCoreGetOemParameter(
            kDslOemG994RcvNSInfo, globalVar.adslMib.adslAtucPhys.adslNsif, sizeof(globalVar.adslMib.adslAtucPhys.adslNsif));
        }
            break;
#endif
        case kDslG992p2RcvVerifiedBitAndGain:
            val = Q4ToTenth(val);
            globalVar.adslMib.adslPhys.adslCurrSnrMgn = RestrictValue(val, -640, 640);
            break;
        case kDslG992p2TxShowtimeActive:
            pathId = XdslMibGetCurrentMappedPathId(gDslVars, TX_DIRECTION);

            globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[pathId].dataRate = val;
            if (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression)
               globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[pathId].dataRate = NitroRate(val);

            Xdsl2MibConvertConnectionInfo(gDslVars, pathId, TX_DIRECTION);
#ifdef CONFIG_VDSL_SUPPORTED
            if(XdslMibIsVdsl2Mod(gDslVars))
                AdslMibCalcVdslPERp(gDslVars, pathId, TX_DIRECTION);
            else
#endif
                AdslMibCalcAdslPERp(gDslVars, pathId, TX_DIRECTION);

            globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].aggrDataRate =
            globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[0].dataRate + globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[1].dataRate;

            if(XdslMibIsGinpActive(gDslVars, TX_DIRECTION)) {
                /* For Ginp, INPrein is not computed in PHY for non-RTX latency path, and INPreinLp0 = INPLp0, so assign it here */
                tmpInt = XdslMibGetPath0MappedPathId(gDslVars, TX_DIRECTION);
                globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[tmpInt].INPrein = globalVar.adslMib.xdslInfo.dirInfo[TX_DIRECTION].lpInfo[tmpInt].INP;
            }
            
#ifdef CONFIG_BCM_DSL_GFAST
            if(globalVar.adslMib.fastRetrainActive) {
                globalVar.txUpdateStatFlag = 1;
                globalVar.adslMib.fastRetrainActive = false;
                globalVar.adslMib.adslPhys.adslCurrStatus &= ~(kAdslPhysStatusLOR | kAdslPhysStatusLOS);
                BlockByteClear(sizeof(globalVar.shtCounters2lp), (void*)&globalVar.shtCounters2lp[0]);
                BlockByteClear(sizeof(globalVar.shtExtCounters2lp), (void*)&globalVar.shtExtCounters2lp[0]);
                BlockByteClear(sizeof(globalVar.ginpCounters), (void*)&globalVar.ginpCounters);
                BlockByteClear(sizeof(globalVar.eocCounters), (void*)&globalVar.eocCounters);
                BlockByteClear(sizeof(globalVar.vectorFBCounters), (void*)&globalVar.vectorFBCounters);
                globalVar.uasOnSes = false;
                globalVar.sesContCnt = 0;
            }
#endif
            if (AdslMibIsLinkActive(gDslVars)) {
                AdslMibNotify(gDslVars, kAdslEventRateChange);
                break;
            }
            
            globalVar.txCntReceived=0;
            globalVar.txShowtimeTime=0;
            globalVar.secElapsedShTm=0;
            globalVar.adslMib.adslPerfData.adslSincePrevLinkTimeElapsed = globalVar.adslMib.adslPerfData.adslSinceLinkTimeElapsed;
            globalVar.adslMib.adslPerfData.adslSinceLinkTimeElapsed = 0;
            
            XdslMibSetActivePath(gDslVars, TX_DIRECTION, pathId);
            
            if(XdslMibIsAllPathActive(gDslVars, TX_DIRECTION)) {
                globalVar.txCntReceived=0;
                globalVar.txShowtimeTime=0;
                globalVar.secElapsedShTm=0;
                globalVar.txUpdateStatFlag=1;
                globalVar.linkStatus |= kAdslXmtActive;
            }
            
            BlockByteClear(sizeof(globalVar.shtCounters2lp), (void*)&globalVar.shtCounters2lp[0]);
            BlockByteClear(sizeof(globalVar.shtExtCounters2lp), (void*)&globalVar.shtExtCounters2lp[0]);
            BlockByteClear(sizeof(globalVar.ginpCounters), (void*)&globalVar.ginpCounters);
#ifdef CONFIG_BCM_DSL_GFAST
            BlockByteClear(sizeof(globalVar.eocCounters), (void*)&globalVar.eocCounters);
            BlockByteClear(sizeof(globalVar.vectorFBCounters), (void*)&globalVar.vectorFBCounters);
#endif
#ifndef ADSL_DRV_NO_STAT_RESET
            n = globalVar.adslMib.xdslStat[0].fireStat.status;
            nb = globalVar.adslMib.xdslStat[0].bondingStat.status;
#if 0
            /* Need to preserve Ginp counters which was stored on path 0 */
            BlockByteMove(sizeof(globalVar.adslMib.xdslStat[0].ginpStat),
                                    (uchar *)&globalVar.adslMib.xdslStat[0].ginpStat,
                                    (uchar *)&globalVar.adslMib.xdslStat[1].ginpStat);
#endif
            /* Preserve Ginp status which was stored on path 0 */
            globalVar.adslMib.xdslStat[1].ginpStat.status = globalVar.adslMib.xdslStat[0].ginpStat.status;
            BlockByteClear(sizeof(globalVar.adslMib.xdslStat[0]), (void*)&globalVar.adslMib.xdslStat[0]);
#if 0
            BlockByteMove(sizeof(globalVar.adslMib.xdslStat[0].ginpStat),
                                    (uchar *)&globalVar.adslMib.xdslStat[1].ginpStat,
                                    (uchar *)&globalVar.adslMib.xdslStat[0].ginpStat);
#endif
            globalVar.adslMib.xdslStat[0].ginpStat.status = globalVar.adslMib.xdslStat[1].ginpStat.status;
            globalVar.adslMib.xdslStat[0].fireStat.status = n;
            globalVar.adslMib.xdslStat[0].bondingStat.status = nb;
            n = globalVar.adslMib.xdslStat[1].fireStat.status;
            nb = globalVar.adslMib.xdslStat[1].bondingStat.status;
            BlockByteClear(sizeof(globalVar.adslMib.xdslStat[1]), (void*)&globalVar.adslMib.xdslStat[1]);
            globalVar.adslMib.xdslStat[1].fireStat.status = n;
            globalVar.adslMib.xdslStat[1].bondingStat.status = nb;
            BlockByteClear(sizeof(globalVar.adslMib.atmStat2lp), (void*)&globalVar.adslMib.atmStat2lp[0]);

            BlockByteClear(
                sizeof(globalVar.adslMib.adslPerfData.perfSinceShowTime), 
                (void*)&globalVar.adslMib.adslPerfData.perfSinceShowTime
                );

            BlockByteMove(
                sizeof(globalVar.adslMib.adslPerfData.failSinceShowTime), 
                (void*)&globalVar.adslMib.adslPerfData.failSinceShowTime,
                (void*)&globalVar.adslMib.adslPerfData.failSinceLastShowTime
                );

            BlockByteClear(
                sizeof(globalVar.adslMib.adslPerfData.failSinceShowTime), 
                (void*)&globalVar.adslMib.adslPerfData.failSinceShowTime
                );

            BlockByteClear(
                sizeof(globalVar.adslMib.rtxCounterData.cntDS.perfSinceShowTime),
                (void*)&globalVar.adslMib.rtxCounterData.cntDS.perfSinceShowTime
                );
            BlockByteClear(
                sizeof(globalVar.adslMib.rtxCounterData.cntUS.perfSinceShowTime),
                (void*)&globalVar.adslMib.rtxCounterData.cntUS.perfSinceShowTime
                );
#ifdef CONFIG_BCM_DSL_GFAST
            BlockByteClear(
                sizeof(globalVar.adslMib.gfastOlrXoiCounterData[0].cntDS.perfSinceShowTime),
                (void*)&globalVar.adslMib.gfastOlrXoiCounterData[0].cntDS.perfSinceShowTime
                );
            BlockByteClear(
                sizeof(globalVar.adslMib.gfastOlrXoiCounterData[0].cntUS.perfSinceShowTime),
                (void*)&globalVar.adslMib.gfastOlrXoiCounterData[0].cntUS.perfSinceShowTime
                );
            BlockByteClear(
                sizeof(globalVar.adslMib.gfastOlrXoiCounterData[1].cntDS.perfSinceShowTime),
                (void*)&globalVar.adslMib.gfastOlrXoiCounterData[1].cntDS.perfSinceShowTime
                );
            BlockByteClear(
                sizeof(globalVar.adslMib.gfastOlrXoiCounterData[1].cntUS.perfSinceShowTime),
                (void*)&globalVar.adslMib.gfastOlrXoiCounterData[1].cntUS.perfSinceShowTime
                );
#endif
#endif
            globalVar.adslMib.adslTrainingState = kAdslTrainingConnected;
            globalVar.timeConStarted = 0;
            globalVar.uasOnSes = false;
            globalVar.sesContCnt = 0;
#ifdef CONFIG_BCM_DSL_GFAST
            XDSL_CLR_CAPTURE_FLAGS;
#endif
            if (AdslMibIsLinkActive(gDslVars)) {
                globalVar.lastInitState = kXdslLastInitStateShowtime;
                globalVar.adslMib.xdslInitializationCause = kXdslInitConfigSuccess;
                globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
                globalVar.adslMib.adslAtucPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
                globalVar.losEventOccurred = false;
#ifdef SUPPORT_DSL_BONDING
                if(AdslMibIsLinkActive(XdslCoreGetDslVars(1^gLineId(gDslVars))))
                    XdslCoreSetMaxBondingDelay();
#endif
                AdslMibNotify(gDslVars, kAdslEventLinkChange);
#ifdef SUPPORT_24HR_CNT_STAT
                IncStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, Syncs);
#endif
            }
            break;
        case kDslG992p2RxShowtimeActive:
            pathId = XdslMibGetCurrentMappedPathId(gDslVars, RX_DIRECTION);
            globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].lpInfo[pathId].dataRate = val;
            if (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression)
               globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].lpInfo[pathId].dataRate = NitroRate(val);
            
            Xdsl2MibConvertConnectionInfo(gDslVars, pathId, RX_DIRECTION);
#ifdef CONFIG_VDSL_SUPPORTED
            if(XdslMibIsVdsl2Mod(gDslVars))
               AdslMibCalcVdslPERp(gDslVars, pathId, RX_DIRECTION);
            else
#endif
               AdslMibCalcAdslPERp(gDslVars, pathId, RX_DIRECTION);

            globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].aggrDataRate =
            globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].lpInfo[0].dataRate + globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].lpInfo[1].dataRate;

            if(XdslMibIsGinpActive(gDslVars, RX_DIRECTION)) {
                /* For Ginp, INPrein is not computed in PHY for non-RTX latency path, and INPreinLp0 = INPLp0, so assign it here */
                tmpInt = XdslMibGetPath0MappedPathId(gDslVars, RX_DIRECTION);
                globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].lpInfo[tmpInt].INPrein = globalVar.adslMib.xdslInfo.dirInfo[RX_DIRECTION].lpInfo[tmpInt].INP;
            }
            
            if (AdslMibIsLinkActive(gDslVars)) {
                AdslMibNotify(gDslVars, kAdslEventRateChange);
                break;
            }
            
            XdslMibSetActivePath(gDslVars, RX_DIRECTION, pathId);
            
            if(XdslMibIsAllPathActive(gDslVars, RX_DIRECTION))
                globalVar.linkStatus |= kAdslRcvActive;
            
            if (AdslMibIsLinkActive(gDslVars)) {
                globalVar.lastInitState = kXdslLastInitStateShowtime;
                globalVar.adslMib.xdslInitializationCause = kXdslInitConfigSuccess;
                globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
                globalVar.losEventOccurred = false;
#ifdef SUPPORT_DSL_BONDING
                if(AdslMibIsLinkActive(XdslCoreGetDslVars(1^gLineId(gDslVars))))
                    XdslCoreSetMaxBondingDelay();
#endif
                AdslMibNotify(gDslVars, kAdslEventLinkChange);
#ifdef SUPPORT_24HR_CNT_STAT
                IncStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, Syncs);
#endif
            }
            
            if(XdslMibIsGinpActive(gDslVars, RX_DIRECTION) && (0 == pathId))
                globalVar.adslMib.xdslStat[0].ginpStat.cntDS.minEFTR = val; /* Initialize minEFTR to NDR */
            
            break;
        case kDslRetrainReason:
            globalVar.txUpdateStatFlag=0;
            if(globalVar.adslMib.adslTrainingState == kAdslTrainingConnected) {
                if(!XdslMibIsGfastMod(gDslVars) && globalVar.losEventOccurred) {
                  AdslMibUpdateLOS(gDslVars);
                  globalVar.losEventOccurred = false;
                }
                globalVar.adslMib.adslPerfData.lastShowtimeDropReason=val;
#ifdef CONFIG_BCM_DSL_GFAST
                if(XdslMibIsGfastMod(gDslVars)&& (0 != val)) {
                  if((val>>kRetrainReasonLosDetector)&0x1)
                    IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslLoss);
                  if ((val>>kRetrainReasonRdiDetector)&0x1)
                    IncPerfCounterVar(&globalVar.adslMib.adslPerfData, xdslLors);
                  if ((val>>kRetrainReasonCoMinMargin)&0x1)
                    IncPerfCounterVar(&globalVar.adslMib.adslPerfData, xdslLoms);
                }
#endif
            }
            if(globalVar.adslMib.adslTrainingState > kAdslTrainingG994)
                globalVar.adslMib.adslPerfData.lastRetrainReason=val;
            break;
        case kDslFinishedG994p1:
            if(-1 == globalVar.adslMib.adslPhys.adslLDCompleted)
                globalVar.adslMib.adslPhys.adslLDCompleted = -2;
            BlockByteClear(sizeof(globalVar.adslMib.adslPhys.adslVendorID), (uchar*)&globalVar.adslMib.adslPhys.adslVendorID[0]);
#ifdef CONFIG_VDSL_SUPPORTED
            globalVar.adslMib.xdslPhys.attnDrMethod     = 0;
            globalVar.adslMib.xdslAtucPhys.attnDrMethod = 0;
            globalVar.adslMib.xdslPhys.numKl0BandReported = 0;
            BlockByteClear(sizeof(globalVar.adslMib.xdslPhys.kl0PerBand), (uchar*)&globalVar.adslMib.xdslPhys.kl0PerBand[0]);
            globalVar.adslMib.xdslAtucPhys.numKl0BandReported = 0;
            BlockByteClear(sizeof(globalVar.adslMib.xdslAtucPhys.kl0PerBand), (uchar*)&globalVar.adslMib.xdslAtucPhys.kl0PerBand[0]);
#endif
            globalVar.adslMib.adslPhys.adslVendorID[0] = 0xb5;
            globalVar.adslMib.adslPhys.adslVendorID[1] = 0x00;
            globalVar.adslMib.adslPhys.adslVendorID[2] = 'B';
            globalVar.adslMib.adslPhys.adslVendorID[3] = 'D';
            globalVar.adslMib.adslPhys.adslVendorID[4] = 'C';
            globalVar.adslMib.adslPhys.adslVendorID[5] = 'M';
            globalVar.adslMib.adslPhys.adslVendorID[6] = 0x00;
            globalVar.adslMib.adslPhys.adslVendorID[7] = 0x00;
            
            gG997Vars.setup = 0;
            switch (val) {
                case kG992p2AnnexAB:
                case kG992p2AnnexC:
                    AdslMibSetModulationType(gDslVars, kAdslModGlite);
                    break;
#ifdef G992P1_ANNEX_I
                case (kG992p1AnnexI>>4):
                    AdslMibSetModulationType(gDslVars, kAdslModAnnexI);
                    break;
#endif
#ifdef G992P3
                case kG992p3AnnexA:
                case kG992p3AnnexB:
                case kG992p3AnnexM:
                case kG992p3AnnexJ:
                    AdslMibSetModulationType(gDslVars, kAdslModAdsl2);
                    break;
#endif
#ifdef G992P5
                case kG992p5AnnexA:
                case kG992p5AnnexB:
                case kG992p5AnnexM:
                case kG992p5AnnexJ:
                    AdslMibSetModulationType(gDslVars, kAdslModAdsl2p);
                    break;
#endif
#ifdef G993
               case kG993p2AnnexA:
                   globalVar.waitBandPlan=1;
                   AdslMibSetModulationType(gDslVars, kVdslModVdsl2);
                   break;
               case kGfastAnnexA:
                   globalVar.waitBandPlan=1;
                   AdslMibSetModulationType(gDslVars, kXdslModGfast);
                   gG997Vars.setup = kG997NoHdlc;
                   break;
#endif
                case kG992p1AnnexA:
                case kG992p1AnnexB:
                case kG992p1AnnexC:
                default:
                    AdslMibSetModulationType(gDslVars, kAdslModGdmt);
                    break;
            }
            switch (val) {
                case kG992p2AnnexAB:
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexAB << kXdslModeAnnexShift));
                    break;
                case kG992p2AnnexC:
                case kG992p1AnnexC:
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexC << kXdslModeAnnexShift));
                    break;
                case (kG992p1AnnexI>>4):
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexI << kXdslModeAnnexShift));
                    break;
                case kG992p3AnnexA:
                case kG992p5AnnexA:
                case kG992p1AnnexA:
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexA << kXdslModeAnnexShift));
                    break;
                case kG992p3AnnexB:
                case kG992p5AnnexB:
                case kG992p1AnnexB:
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexB << kXdslModeAnnexShift));
                    break;
                case kG992p3AnnexM:
                case kG992p5AnnexM:
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexM << kXdslModeAnnexShift));
                    break;
                case kG992p3AnnexJ:
                case kG992p5AnnexJ:
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexJ << kXdslModeAnnexShift));
                    break;
                default:
                    XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexA << kXdslModeAnnexShift));
                    break;
            }
            globalVar.nTones = kAdslMibToneNum;
#ifdef G992P5
            if (kAdslModAdsl2p == AdslMibGetModulationType(gDslVars))
                globalVar.nTones = kAdslMibToneNum * 2;
#endif
#ifdef G992_ANNEXC
            globalVar.nTones = kAdslMibToneNum * 
                ((kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) ? 4 : 2);
#endif
#ifdef G993
            if (XdslMibIsVdsl2Mod(gDslVars)) {
                globalVar.nTones = kVdslMibMaxToneNum;
#ifdef CONFIG_BCM_DSL_GFAST
                globalVar.nTonesGfast = kVdslMibMaxToneNum;
                globalVar.adslMib.gfastDta.dtaFlags = 0;
#endif
                if( !globalVar.adslMib.fastRetrainActive
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
                    && !AdslDrvIsFakeLinkUp(gLineId(gDslVars))
#endif
                  )
                    globalVar.adslMib.adslTrainingState = kAdslTrainingG993Started;
            }
            else
#endif
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            globalVar.adslMib.adslTrainingState = kAdslTrainingG992Started;

            if (!XdslMibIsVdsl2Mod(gDslVars)) {
                 /* Will get updated when framing parameters statuses arrived on the 6368/6362 in ADSL mode */
                globalVar.adslMib.adsl2Info2lp[0].rcvChanInfo.ahifChanId = 0;
                globalVar.adslMib.adsl2Info2lp[0].rcvChanInfo.connectionType = kXdslDataAtm;
                globalVar.adslMib.adsl2Info2lp[0].xmtChanInfo.ahifChanId = 0;
                globalVar.adslMib.adsl2Info2lp[0].xmtChanInfo.connectionType = kXdslDataAtm;
                if( 2 == MAX_LP_NUM) {
                    globalVar.adslMib.adsl2Info2lp[1].rcvChanInfo.ahifChanId = 2;
                    globalVar.adslMib.adsl2Info2lp[1].rcvChanInfo.connectionType = kXdslDataAtm;
                    globalVar.adslMib.adsl2Info2lp[1].xmtChanInfo.ahifChanId = 2;
                    globalVar.adslMib.adsl2Info2lp[1].xmtChanInfo.connectionType = kXdslDataAtm;
                }
            }
            AdslSetBandPlan(gDslVars);
            break;
        case kDslG992p3AnnexLMode:
            if ((val != 0) && (kAdslModAdsl2 == AdslMibGetModulationType(gDslVars))) {
                AdslMibSetModulationType(gDslVars, kAdslModReAdsl2);
                XdslMibSetAnnexType(gDslVars,(kAdslTypeAnnexL << kXdslModeAnnexShift));
                if (kG994p1G992p3AnnexLUpNarrowband == (val & 0xFF))
                    AdslMibSetAnnexLUpType(gDslVars, kAdsl2ModeAnnexLUpNarrow);
                else
                    AdslMibSetAnnexLUpType(gDslVars, kAdsl2ModeAnnexLUpWide);
                if (kG994p1G992p3AnnexLDownNonoverlap == ((val >> 8) & 0xFF))
                    AdslMibSetAnnexLDnType(gDslVars, kAdsl2ModeAnnexLDnNonOvlap);
                else
                    AdslMibSetAnnexLDnType(gDslVars, kAdsl2ModeAnnexLDnOvlap);
            }
            AdslSetBandPlan(gDslVars);
            break;
        case kG992EnableAnnexM:
            AdslMibSetAnnexMType(gDslVars, val+1);
            AdslSetBandPlan(gDslVars);
            break;
        case kDslFinishedT1p413:
            AdslMibSetModulationType(gDslVars, kAdslModT1413);
            XdslMibSetAnnexType(gDslVars,(kAdslTypeUnknown << kXdslModeAnnexShift));
            globalVar.nTones = kAdslMibToneNum;
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            globalVar.adslMib.adslTrainingState = kAdslTrainingG992Started;
            BlockByteClear(sizeof(globalVar.adslMib.adslPhys.adslVendorID), &globalVar.adslMib.adslPhys.adslVendorID[0]);
#ifdef CONFIG_VDSL_SUPPORTED
            globalVar.adslMib.xdslPhys.attnDrMethod     = 0;
            globalVar.adslMib.xdslAtucPhys.attnDrMethod = 0;
#endif
            globalVar.adslMib.adslPhys.adslVendorID[0] = 0x54;
            globalVar.adslMib.adslPhys.adslVendorID[1] = 0x4d;
            AdslSetBandPlan(gDslVars);
            break;
        case kDslStartedG992p2Training:
            AdslSetBandPlan(gDslVars);
            break;
        case kG992DataRcvDetectLOS:
            globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOS;
            globalVar.adslMib.adslPhys.adslCurrStatus &= ~(kAdslPhysStatusLOF | kAdslPhysStatusLOM | kAdslPhysStatusNoDefect);
            XDSL_MIB_CAPTURE_CNTRS(gDslVars, CAPTURE_CNTRS_ONLOS);
            if(!XdslMibIsGfastMod(gDslVars)) {
              IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslLoss);
              globalVar.losEventOccurred = true;
            }
#if defined(CONFIG_BCM_DSL_GFAST)
            else if((false == (globalVar.adslMib.fastRetrainActive || globalVar.uasOnSes)) && !G992p3OvhMsgIsL3RspPending(gDslVars))
              globalVar.losEventOccurred = true;
#endif
            //AdslMibUpdateLOS(gDslVars);
            
            break;
        case kG992DataRcvDetectLOSRecovery:
            globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOS;
            if(0 == globalVar.adslMib.adslPhys.adslCurrStatus)
                globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
#if defined(CONFIG_BCM_DSL_GFAST)
            globalVar.inhibCntrsOnLOS.inhibitCntrCapture= 0;
#endif
            break;
        case kG992DecoderDetectRemoteLOS:
            globalVar.adslMib.adslAtucPhys.adslCurrStatus |= kAdslPhysStatusLOS;
            globalVar.adslMib.adslAtucPhys.adslCurrStatus &= ~kAdslPhysStatusNoDefect;
            break;
        case kG992DecoderDetectRemoteLOSRecovery:
            globalVar.adslMib.adslAtucPhys.adslCurrStatus &= ~kAdslPhysStatusLOS;
            if(0 == globalVar.adslMib.adslAtucPhys.adslCurrStatus)
                globalVar.adslMib.adslAtucPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
            break;
        case kG992DecoderDetectRDI:
            if (0 == (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS)) {
                globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOF;
                globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusNoDefect;
            }
            IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslLofs);
            AdslMibUpdateLOF(gDslVars);
#ifdef SUPPORT_24HR_CNT_STAT
            IncStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, LossOfFraming);
#endif
            break;
        case kG992DecoderDetectRDIRecovery:
            globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOF;
            if(0 == globalVar.adslMib.adslPhys.adslCurrStatus)
                globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
            break;
        case kG992LDCompleted:
            globalVar.adslMib.adslPhys.adslLDCompleted = (val != 0)? 2: -1;
#ifdef SUPPORT_SELT
            if (val==255)
               AdslMibNotify(gDslVars, kXdslEventSeltNext);
#endif
            break;
        case kDslG994p1ReturntoStartup:
            globalVar.adslMib.adslTrainingState = kAdslTrainingIdle;
            break;
        case kDslG994p1StartupFinished:
#if 0 /* Wait until first message received before proceeding to Training State
           This was causing false Galf detection to trigger the transition even
           when there was no DSL input. It was confusing customer */
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            globalVar.adslMib.adslTrainingState = kAdslTrainingG994;
#endif
            break;
        case kDslG992p2Phase3Started:
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            globalVar.adslMib.adslTrainingState = kAdslTrainingG992ChanAnalysis;
            break;
        case kDslG992p2Phase4Started:
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            globalVar.adslMib.adslTrainingState = kAdslTrainingG992Exchange;
            break;
        case kDslG992p2ReceivedBitGainTable:
            globalVar.nMsgCnt = 0;
            /* fall through */
        case kDslG992p2ReceivedMsgLD:
        case kDslG992p2ReceivedRates1:
        case kDslG992p2ReceivedMsg1:
        case kDslG992p2ReceivedRatesRA:
        case kDslG992p2ReceivedMsgRA:
        case kDslG992p2ReceivedRates2:
        case kDslG992p2ReceivedMsg2:
            globalVar.g992MsgType = status->param.dslTrainingInfo.code;
            break;
        case kDslG992Timeout:
            IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslInitTo);
#ifdef SUPPORT_24HR_CNT_STAT
            IncStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, InitTimeouts);
#endif

            break;
        default:
            break;
      }
      break; /* kDslTrainingStatus */
    
    case kDslConnectInfoStatus:
      val = status->param.dslConnectInfo.value;
      switch (status->param.dslConnectInfo.code) {
        case kDslNLdbEcho:
            globalVar.adslMib.adslNonLinData.NonLinDbEcho = (short)val;
            break;
        case kDslG992VendorID:
            val = (val < VENDOR_TBL_SIZE) ? val: 0;
#ifdef CONFIG_VDSL_SUPPORTED
            pVendorId = &globalVar.adslMib.xdslAtucPhys.adslVendorID[0];
            pSysVendorId = &globalVar.adslMib.xdslAtucPhys.adslSysVendorID[0];
#else
            pVendorId = &globalVar.adslMib.adslAtucPhys.adslVendorID[0];
            pSysVendorId = &globalVar.adslMib.adslAtucPhys.adslSysVendorID[0];
#endif
            AdslMibByteMove(6, (void *)&gVendorTbl[val][0], pVendorId);
            AdslMibByteMove(kAdslPhysVendorIdLen, pVendorId, pSysVendorId);
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "kDslG992VendorID: %c%c%c%c\n",
                         0, pVendorId[2],pVendorId[3],pVendorId[4],pVendorId[5]);
            break;
#ifdef G994P1
        case    kG994MessageExchangeRcvInfo:
        {
            uchar   *msgPtr = ((unsigned char*)status->param.dslConnectInfo.buffPtr);
            ushort  v;
            if ((msgPtr != NULL) && ((msgPtr[0] == 2) || (msgPtr[0] == 3)) &&   /* CL or CLR message */
                (val >= (2 + kAdslPhysVendorIdLen)) && !globalVar.vendorIdReceived) {
                globalVar.vendorIdReceived=true;
#ifdef CONFIG_VDSL_SUPPORTED
                AdslMibByteMove(kAdslPhysVendorIdLen, msgPtr+2, globalVar.adslMib.xdslAtucPhys.adslVendorID);
                AdslMibByteMove(kAdslPhysVendorIdLen, msgPtr+2, globalVar.adslMib.xdslAtucPhys.adslSysVendorID);
                v=((uchar)globalVar.adslMib.xdslAtucPhys.adslVendorID[6]<<8)+(uchar)globalVar.adslMib.xdslAtucPhys.adslVendorID[7];
                sprintf(globalVar.adslMib.xdslAtucPhys.adslVersionNumber,"0x%04x",v);
#else
                AdslMibByteMove(kAdslPhysVendorIdLen, msgPtr+2, globalVar.adslMib.adslAtucPhys.adslVendorID);
                AdslMibByteMove(kAdslPhysVendorIdLen, msgPtr+2, globalVar.adslMib.adslAtucPhys.adslSysVendorID);
                v=((uchar)globalVar.adslMib.adslAtucPhys.adslVendorID[6]<<8)+(uchar)globalVar.adslMib.adslAtucPhys.adslVendorID[7];
                sprintf(globalVar.adslMib.adslAtucPhys.adslVersionNumber,"0x%04x",v);
#endif
            }
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
            /* Put the transition to training state here to make sure it is not false detection.
                 kDslG994p1StartupFinished can be triggered by false Galf even if no DSL input */
            {
              if (globalVar.adslMib.adslTrainingState == kAdslTrainingIdle) {
                globalVar.adslMib.adslTrainingState = kAdslTrainingG994;
                __SoftDslPrintf(gDslVars, "Driver Transitioning to kAdslTrainingG994 state\n", 0);
              }
            }
        }
            break;
#endif
        case kG992p2XmtCodingParamsInfo:
            {
            G992CodingParams *codingParam = (G992CodingParams*) status->param.dslConnectInfo.buffPtr;

            codingParam->AS0BF = codingParam->AS1BF = codingParam->AS2BF = codingParam->AS3BF = codingParam->AEXAF = 0;
            codingParam->AS0BI = codingParam->AS1BI = codingParam->AS2BI = codingParam->AS3BI = codingParam->AEXAI = 0;
            AdslMibByteMove(sizeof(G992CodingParams), codingParam, &globalVar.xmtParams2lp[pathId]);
#ifdef  ADSLDRV_LITTLE_ENDIAN
            codingParam = &globalVar.xmtParams2lp[pathId];
            codingParam->K = ADSL_ENDIAN_CONV_USHORT(codingParam->K);
            codingParam->direction = ADSL_ENDIAN_CONV_INT32(codingParam->direction);
            codingParam->N = ADSL_ENDIAN_CONV_USHORT(codingParam->N);
            codingParam->NF = ADSL_ENDIAN_CONV_USHORT(codingParam->NF);
            codingParam->AS0BI = ADSL_ENDIAN_CONV_USHORT(codingParam->AS0BI);
#endif
            AdslMibSetChanEntry(codingParam, &globalVar.adslMib.adslChanFast, &globalVar.adslMib.adslChanIntl);
            AdslMibSetConnectionInfo(gDslVars, codingParam, status->param.dslConnectInfo.code, val, &globalVar.adslMib.xdslConnection[pathId]);
            Adsl2MibSetInfoFromGdmt1(gDslVars, &globalVar.adslMib.adsl2Info2lp[pathId].xmt2Info, &globalVar.adslMib.xdslConnection[pathId].xmtInfo);
            }
            break;
        case kG992p2RcvCodingParamsInfo:
            {
            G992CodingParams *codingParam = (G992CodingParams*) status->param.dslConnectInfo.buffPtr;
            AdslMibByteMove(sizeof(G992CodingParams), codingParam, &globalVar.rcvParams2lp[pathId]);
#ifdef  ADSLDRV_LITTLE_ENDIAN
            codingParam = &globalVar.rcvParams2lp[pathId];
            codingParam->K = ADSL_ENDIAN_CONV_USHORT(codingParam->K);
            codingParam->direction = ADSL_ENDIAN_CONV_INT32(codingParam->direction);
            codingParam->N = ADSL_ENDIAN_CONV_USHORT(codingParam->N);
            codingParam->NF = ADSL_ENDIAN_CONV_USHORT(codingParam->NF);
            codingParam->AS0BI = ADSL_ENDIAN_CONV_USHORT(codingParam->AS0BI);
#endif
            AdslMibSetChanEntry(codingParam, &globalVar.adslMib.adslChanFast, &globalVar.adslMib.adslChanIntl);
            AdslMibSetConnectionInfo(gDslVars, codingParam, status->param.dslConnectInfo.code, val, &globalVar.adslMib.xdslConnection[pathId]);
            Adsl2MibSetInfoFromGdmt1(gDslVars, &globalVar.adslMib.adsl2Info2lp[pathId].rcv2Info, &globalVar.adslMib.xdslConnection[pathId].rcvInfo);
            }
            break;

        case kG992p3XmtCodingParamsInfo:
        case kG992p3RcvCodingParamsInfo:
            {
            G992p3CodingParams *codingParam = (G992p3CodingParams *) status->param.dslConnectInfo.buffPtr;

            Adsl2MibSetConnectionInfo(gDslVars, codingParam, status->param.dslConnectInfo.code, val, &globalVar.adslMib.adsl2Info);
            }
            break;

        case kG992p3PwrStateInfo:
            globalVar.adslMib.adsl2Info.pwrState = val; /* For DslDiag backward compatible */
            globalVar.adslMib.xdslInfo.pwrState = val;
            if( 2 == val )
                AdslCoreIndicateLinkPowerStateL2(gLineId(gDslVars));
#ifdef DEBUG_L2_RET_L0
            printk("%s: L%d\n", __FUNCTION__, val);
#endif
            break;

        case kDslATUAvgLoopAttenuationInfo:
            val = Q4ToTenth(val);
            globalVar.adslMib.adslPhys.adslCurrAtn = RestrictValue(val, 0, 1630);
            globalVar.adslMib.adslPhys.adslSignalAttn = globalVar.adslMib.adslPhys.adslCurrAtn;
            break;
#if defined(CONFIG_VDSL_SUPPORTED)
        case kDslATUAvgLoopAttenuationInfoAt300kHz:
            val = Q4ToTenth(val);
            globalVar.adslMib.xdslPhys.xdslSignalAttnAt300kHz = RestrictValue(val, 0, 1630);
            break;
#endif
        case kDslSignalAttenuation:
            val = Q4ToTenth(val);
            globalVar.adslMib.adslDiag.signalAttn = val;
            globalVar.adslMib.adslPhys.adslSignalAttn = val;
            break;

        case kDslAttainableNetDataRate:
            globalVar.adslMib.adslDiag.attnDataRate = val;
            globalVar.adslMib.adslPhys.adslCurrAttainableRate = ActualRate(val);
            break;

        case kDslHLinScale:
            globalVar.adslMib.adslDiag.hlinScaleFactor = val;
            globalVar.adslMib.adslPhys.adslHlinScaleFactor = val;
            break;

        case kDslATURcvPowerInfo:
            globalVar.rcvPower = val;
            break;

        case kDslMaxReceivableBitRateInfo:
            globalVar.adslMib.adslPhys.adslCurrAttainableRate = ActualRate(val * 1000);
#ifdef USE_TRAINING_ATTNDR
            globalVar.adslMib.xdslPhys.adslTrainAttainableRate = ActualRate(val * 1000);
            globalVar.adslMib.xdslPhys.adslShowAttainableRate = 0;
#endif
            break;

        case kDslRcvCarrierSNRInfo:
#ifdef CONFIG_VDSL_SUPPORTED
            if((BcmAdslCoreGetConfiguredMod(gLineId(gDslVars)) & (kG993p2AnnexA | kGfastAnnexA)) && XdslMibIsVdsl2Mod(gDslVars))
               copyBandPlanData((short *)status->param.dslConnectInfo.buffPtr, globalVar.snr, &globalVar.adslMib.dsPhyBandPlan, globalVar.nTones);
            else
#endif
            {
               n = globalVar.nTones;
               if (val > nMaxToneAllow)
                   val = nMaxToneAllow;
               BlockShortMoveReverse(val, (short *)status->param.dslConnectInfo.buffPtr, globalVar.snr + globalVar.nMsgCnt * n);
#ifdef G992_ANNEXC
               globalVar.nMsgCnt ^= 1;
#endif
            }
            break;
        case kG992p2XmtToneOrderingInfo:
            {
            uchar   *buffPtr = status->param.dslConnectInfo.buffPtr;

            if (AdslMibTone32_64(gDslVars)) {
                AdslMibSetBitAllocation(gDslVars, 0, buffPtr + 0, 32, false);
                AdslMibSetBitAllocation(gDslVars, 32, buffPtr + 32, 32, true);
            }
            else
                AdslMibSetBitAllocation(gDslVars, 0, buffPtr, val, false);
            
            if(AdslMibIsLinkActive(gDslVars)) {
                if ( XdslMibIsXdsl2Mod(gDslVars) ) {
                    if(globalVar.bitSwapReqTime[TX_DIRECTION]) {
                        globalVar.adslMib.adslStat.bitswapStat.xmtCnt++;
                        globalVar.bitSwapReqTime[TX_DIRECTION] = 0;
                    }
                }
            }
#ifdef G992_ANNEXC
            n = kAdslMibToneNum;
            if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars))
                n <<= 1;
            AdslMibSetBitAllocation(gDslVars, n, buffPtr + 32, 32, false);
#endif
            }
            break;

        case kG992p2RcvToneOrderingInfo:
            {
            uchar   *buffPtr = status->param.dslConnectInfo.buffPtr;

            if (AdslMibTone32_64(gDslVars)) {
                AdslMibSetBitAllocation(gDslVars, 32, buffPtr + 32, 32, true);
                AdslMibSetBitAllocation(gDslVars, 64, buffPtr + 64, val - 32, false);
            }
            else
                AdslMibSetBitAllocation(gDslVars, 32, buffPtr+32, val - 32, false);
            
            if(AdslMibIsLinkActive(gDslVars)) {
                if ( XdslMibIsXdsl2Mod(gDslVars) ) {
                    if(globalVar.bitSwapReqTime[RX_DIRECTION]) {
                        globalVar.adslMib.adslStat.bitswapStat.rcvCnt++;
                        globalVar.bitSwapReqTime[RX_DIRECTION] = 0;
                    }
                }
            }
#ifdef G992_ANNEXC
            if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
                AdslMibSetBitAllocation(gDslVars, kAdslMibToneNum, buffPtr+kAdslMibToneNum, 256, false);
                AdslMibSetBitAllocation(gDslVars, 2*kAdslMibToneNum+32, buffPtr+2*kAdslMibToneNum+32, 224+256, false);
            }
            else
                AdslMibSetBitAllocation(gDslVars, kAdslMibToneNum+32, buffPtr+kAdslMibToneNum+32, 224, false);
#endif
            }
            break;
            
        case kG992MessageExchangeRcvInfo:
            if (kDslG992p2ReceivedMsg2 == globalVar.g992MsgType) {
                uchar   *msgPtr = (uchar *)status->param.dslConnectInfo.buffPtr;
                int     n;

                n = (*msgPtr) | ((*(msgPtr+1) & 1) << 8);
                globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate = ActualRate(n * 4000);
                globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn = (*(msgPtr+2) & 0x1F) * 10;
                globalVar.adslMib.adslAtucPhys.adslCurrAtn = (*(msgPtr+3) >> 2) * 5;
            }
            else if (kDslG992p2ReceivedBitGainTable == globalVar.g992MsgType) {
                short   *buffPtr = status->param.dslConnectInfo.buffPtr;
                int     n;

                globalVar.bitAlloc[0] = 0;
                globalVar.gain[0] = 0;
                val = status->param.dslConnectInfo.value >> 1;
                n   = 0;
#ifdef G992P3
                if (XdslMibIsXdsl2Mod(gDslVars)) {
                    uchar   *p = (uchar *) buffPtr;
                    int     rate;

                    val -= 7;
                    n    = 7;
                    globalVar.adslMib.adslAtucPhys.adslCurrAtn = (((uint) p[1] & 0x3) << 8) | p[0];
                    globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn = ((int) p[5] << 8) | p[4];
                    rate = ((uint) p[9] << 24) | ((uint) p[8] << 16) | ((uint) p[7] << 8) | p[6];
                    globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate = ActualRate(rate);

                    rate = ((int) p[11] << 8) | p[10];
                    globalVar.adslMib.adslPhys.adslCurrOutputPwr = (rate << (32-10)) >> (32-10);
                }
#endif

#ifdef G992_ANNEXC
                if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
                    AdslMibSetBitAndGain(gDslVars, 1+globalVar.nMsgCnt*2*kAdslMibToneNum, buffPtr + 0, 31, false);
                }
                else {
                    AdslMibSetBitAndGain(gDslVars, 1, buffPtr + 0, 31, false);
                    globalVar.bitAlloc[kAdslMibToneNum] = 0;
                    globalVar.gain[kAdslMibToneNum] = 0;
                    AdslMibSetBitAndGain(gDslVars, kAdslMibToneNum + 1, buffPtr + 31, 31, false);
                }
#else
                AdslMibSetBitAndGain(gDslVars, 1, buffPtr + n, val, false);
#endif
            }
            else if (kDslG992p2ReceivedRatesRA == globalVar.g992MsgType) {
                uchar   *msgPtr = (uchar *)status->param.dslConnectInfo.buffPtr;
#ifdef SAVE_CRATESRA_MSG
                val = (status->param.dslConnectInfo.value > sizeof(globalVar.adslMib.adslCRatesRAMsg)) ? sizeof(globalVar.adslMib.adslCRatesRAMsg): status->param.dslConnectInfo.value;
                AdslMibByteMove(val, msgPtr, globalVar.adslMib.adslCRatesRAMsg);
#endif
                for (n = 0; n < 4; n++)
                    globalVar.rsOption[1+n] = msgPtr[n*30 + 21] & 0x3F;
                globalVar.rsOptionValid = true;
            }
            else if (kDslG992p2ReceivedMsgLD == globalVar.g992MsgType) {
                uchar   *p = (uchar *)status->param.dslConnectInfo.buffPtr;
                char    *ps = (char *) p;
                uint   msgLen;
                int     i, j;

                msgLen = status->param.dslConnectInfo.value;
                switch (*p) {
                  case 0x11:
                    {
                    uint   n;

                    globalVar.adslMib.adslAtucPhys.adslHlinScaleFactor = ((int) p[3] << 8) + p[2];
                    globalVar.adslMib.adslAtucPhys.adslCurrAtn = ((int) p[5] << 8) + p[4];
                    globalVar.adslMib.adslAtucPhys.adslSignalAttn = ((int) p[7] << 8) + p[6];
                    globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn = ((int) ps[9] << 8) + p[8];
                    globalVar.adslMib.adslPhys.adslCurrOutputPwr = ((int) ps[15] << 8) + p[14];
                    n = ((uint) p[13] << 24) | ((uint) p[12] << 16) | ((uint) p[11] << 8) | p[10];
                    globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate = n;
                    }
                    break;

                  case 0x22:
                    j = 0;
                    for (i = 2; i < msgLen; i += 4) {
                        globalVar.chanCharLin[j].x = ((int) ps[i+1] << 8) + p[i+0];
                        globalVar.chanCharLin[j].y = ((int) ps[i+3] << 8) + p[i+2];
                        j++;
                    }
                    break;
                  case 0x33:
                    j = 0;
                    for (i = 2; i < msgLen; i += 2) {
                        int     n;

                        n = ((int) (p[i+1] & 0x3) << 12) + (p[i] << 4);
                        n = -(n/10) + 6*16;
                        globalVar.chanCharLog[j] = n;
                        j++;
                    }
                    break;
                  case 0x44:
                    for (i = 2; i < msgLen; i++)
                        globalVar.quietLineNoise[i-2] = -(((short) p[i]) << 3) - 23*16;
                    break;
                  case 0x55:
                    for (i = 2; i < msgLen; i++)
                        globalVar.snr[i-2] = (((short) p[i]) << 3) - 32*16;
                    break;

                }
            }
            else if ((kAdslModT1413 == globalVar.adslMib.adslConnection.modType) &&
                        (kDslG992p2ReceivedMsg1 == globalVar.g992MsgType)){
                uchar   vendorVer;
                ushort  vendorId;
                uchar   *msgPtr = (uchar *)status->param.dslConnectInfo.buffPtr;
                globalVar.vendorIdReceived=true;
                 vendorId = ((ushort)msgPtr[3] >> 4) + ((ushort)msgPtr[4] << 4) + (((ushort)msgPtr[5] & 0x0F) << 12);
                vendorVer = (msgPtr[2]  >> 2) & 0x1F;
#ifdef CONFIG_VDSL_SUPPORTED
                pVendorId = &globalVar.adslMib.xdslAtucPhys.adslVendorID[0];
                pSysVendorId = &globalVar.adslMib.xdslAtucPhys.adslSysVendorID[0];
                sprintf(globalVar.adslMib.xdslAtucPhys.adslVersionNumber,"0x%04x",vendorVer);
#else
                pVendorId = &globalVar.adslMib.adslAtucPhys.adslVendorID[0];
                pSysVendorId = &globalVar.adslMib.adslAtucPhys.adslSysVendorID[0];
                sprintf(globalVar.adslMib.adslAtucPhys.adslVersionNumber,"0x%04x",vendorVer);
#endif
                pVendorId[0] = 0xB5;
                pVendorId[1] = 0x00;
                pVendorId[6] = 0x00;
                pVendorId[7] = vendorVer;
                switch (vendorId)
                {
                    case 0x4D54:
                        AdslMibByteMove(4, "BDCM", pVendorId+2);
                        break;
                    case 0x001C:
                        AdslMibByteMove(4, "ANDV", pVendorId+2);
                        break;
                    case 0x0039:
                        AdslMibByteMove(4, "GSPN", pVendorId+2);
                        break;
                    case 0x0022:
                        AdslMibByteMove(4, "ALCB", pVendorId+2);
                        break;
                    case 0x0004:
                        AdslMibByteMove(4, "TSTC", pVendorId+2);
                        break;
                    case 0x0050:
                    case 0xB6DB:
                        AdslMibByteMove(4, "CENT", pVendorId+2);
                        break;
                    default:
                        AdslMibByteMove(4, "UNKN", pVendorId+2);
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
                        if(0 == gLineId(gDslVars))
#endif
                        __SoftDslPrintf(gDslVars, "kDslG992p2ReceivedMsg1: Unknown chipset vendorId =%x version=0x%4x\n",
                            0, vendorId, vendorVer);
                        break;
                }
                AdslMibByteMove(kAdslPhysVendorIdLen, pVendorId, pSysVendorId);
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
                    if(0 == gLineId(gDslVars))
#endif
                __SoftDslPrintf(gDslVars, "kDslG992p2ReceivedMsg1: vendorId =%x, %c%c%c%c:0x%04x\n",
                            0, vendorId, pVendorId[2],pVendorId[3],pVendorId[4],pVendorId[5], vendorVer);
            }
            break;

        case kG992MessageExchangeXmtInfo:
            if ((1 == status->param.dslConnectInfo.value) && globalVar.rsOptionValid) {
                static  uchar optNum[16] = { 0, 1, 2, 0, 3, 0,0,0, 4,  0,0,0,0,0,0,0 };
                uchar   *msgPtr = (uchar *)status->param.dslConnectInfo.buffPtr;

                globalVar.rsOption[0] = globalVar.rsOption[optNum[*msgPtr & 0xF]];
                break;
            }

            if (kDslG992p2ReceivedBitGainTable == globalVar.g992MsgType) {
                short * buffPtr = status->param.dslConnectInfo.buffPtr;
                int     n;

                val = status->param.dslConnectInfo.value >> 1;
                n   = 0;
#ifdef G992P3
                if (XdslMibIsXdsl2Mod(gDslVars)) {
                    val -= 7;
                    n    = 7;
                }
#endif

#ifdef G992_ANNEXC
                if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
                    AdslMibSetBitAndGain(gDslVars, 32+globalVar.nMsgCnt*2*kAdslMibToneNum, buffPtr + 31, 224+256, false);
                }
                else {
                    AdslMibSetBitAndGain(gDslVars, 32, buffPtr + 31, 224, false);
                    AdslMibSetBitAndGain(gDslVars, kAdslMibToneNum + 32, buffPtr + 255 + 31, 224, false);
                }
#else
                if (AdslMibTone32_64(gDslVars)) {
                    AdslMibSetBitAndGain(gDslVars, 32, buffPtr + 31 + n, 32, true);
                    AdslMibSetBitAndGain(gDslVars, 64, buffPtr + 63 + n, val - 32, false);
                }
                else
#endif
                    AdslMibSetBitAndGain(gDslVars, 32, buffPtr + 31 + n, val > 300 ? 480 : 224, false);
            }
            break;

        case kG992ShowtimeMonitoringStatus:
          {
            uint   *counters = (uint*) (status->param.dslConnectInfo.buffPtr);
#ifdef  ADSLDRV_LITTLE_ENDIAN
            uint dstCounters[DSL_COUNTERS_MAX];
            BlockLongMoveReverse(DSL_COUNTERS_MAX, counters, &dstCounters[0]);
            counters = &dstCounters[0];
#endif
            pathId = XdslMibGetCurrentMappedPathId(gDslVars, RX_DIRECTION);
            
            if(!globalVar.adslMib.lp2Active && !globalVar.adslMib.lp2TxActive ) {
                pChanPerfData = (kAdslIntlChannel == globalVar.adslMib.adslConnection.chType) ?
                   &globalVar.adslMib.adslChanIntlPerfData : &globalVar.adslMib.adslChanFastPerfData;
            }
            else
                pChanPerfData = &globalVar.adslMib.xdslChanPerfData[pathId];
            
            if (XdslMibIsDataCarryingPath(gDslVars, pathId, RX_DIRECTION)) {
#ifdef CONFIG_BCM_DSL_GFAST
               if(globalVar.lorEventOccurred) {
                   if(globalVar.losEventOccurred) {
                      AdslMibES(globalVar.currSecondLOS, adslLOSS);
                      globalVar.losEventOccurred = false;
                   }
                   AdslMibUpdateLOR(gDslVars);
                   globalVar.lorEventOccurred = false;
               }
#endif
               if(globalVar.losEventOccurred) {
                   AdslMibUpdateLOS(gDslVars);
                   globalVar.losEventOccurred = false;
               }

               if (AdslMibShowtimeDataError(counters, (uint *)&globalVar.shtCounters2lp[pathId])) {
#ifdef CONFIG_BCM_DSL_GFAST
                   if(!globalVar.adslMib.fastRetrainActive && !G992p3OvhMsgIsL3RspPending(gDslVars))
#endif
                   {
                   uint   nErr = AdslMibShowtimeSFErrors(counters, (uint *)&globalVar.shtCounters2lp[pathId]);
                   AdslMibUpdateShowtimeErrors(gDslVars, nErr, pathId);
                   }
               }
               else
                   AdslMibNoSES(globalVar.currSecondSES, adslSES);

               n = AdslMibShowtimeRSErrors(counters, (uint *)&globalVar.shtCounters2lp[pathId]);
               if (n != 0)
                   AdslMibUpdateShowtimeRSErrors(gDslVars, n);
            }
            
            val = counters[kG992ShowtimeSuperFramesRcvd] - globalVar.shtCounters2lp[pathId][kG992ShowtimeSuperFramesRcvd];
            AddBlockCounterVar(pChanPerfData, adslChanReceivedBlks, val);
            AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, ReceiveBlocks, val);

            if(XdslMibIsXdsl2Mod(gDslVars))
              val=val*globalVar.PERpDS[pathId]/globalVar.per2lp[pathId];
            AddBlockCounterVar(pChanPerfData, adslChanTransmittedBlks, val);  /* TBD */
            AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, TransmitBlocks, val);
            
            if((false == (globalVar.adslMib.fastRetrainActive || globalVar.uasOnSes || globalVar.currSecondSES))
#ifdef CONFIG_BCM_DSL_GFAST
               && !G992p3OvhMsgIsL3RspPending(gDslVars)
#endif
              ) {
              val = counters[kG992ShowtimeSuperFramesRcvdWrong] - globalVar.shtCounters2lp[pathId][kG992ShowtimeSuperFramesRcvdWrong];
              AddBlockCounterVar(pChanPerfData, adslChanUncorrectBlks, val);
              AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, CRCErrors, val);

              val = counters[kG992ShowtimeRSCodewordsRcvedCorrectable] - globalVar.shtCounters2lp[pathId][kG992ShowtimeRSCodewordsRcvedCorrectable];
              AddBlockCounterVar(pChanPerfData, adslChanCorrectedBlks, val);
              AddStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, FECErrors, val);
            }
            
            if(XdslMibIsXdsl2Mod(gDslVars)) {
                counters[kG992ShowtimeNumOfFEBE] = globalVar.shtCounters2lp[pathId][kG992ShowtimeNumOfFEBE];
                counters[kG992ShowtimeNumOfFECC] = globalVar.shtCounters2lp[pathId][kG992ShowtimeNumOfFECC];
                counters[kG992ShowtimeNumOfFHEC] = globalVar.shtCounters2lp[pathId][kG992ShowtimeNumOfFHEC];
            }
            
            AdslMibConnectionStatUpdate (gDslVars, (uint *)&globalVar.shtCounters2lp[pathId], counters, pathId);
            AdslMibByteMove(sizeof(globalVar.shtCounters), counters, &globalVar.shtCounters2lp[pathId]);
            
            if((XdslMibIsGinpActive(gDslVars, RX_DIRECTION) || XdslMibIsGinpActive(gDslVars, TX_DIRECTION)) &&
              !(XdslMibIsGinpActive(gDslVars, RX_DIRECTION) && XdslMibIsGinpActive(gDslVars, TX_DIRECTION))) {
              /* Insert the US counters from the same latency path as the DS counters; For Diags print purposes only */
              pathId ^= 1;
              counters[kG992ShowtimeNumOfFEBE] = globalVar.shtCounters2lp[pathId][kG992ShowtimeNumOfFEBE];
              counters[kG992ShowtimeNumOfFECC] = globalVar.shtCounters2lp[pathId][kG992ShowtimeNumOfFECC];
              counters[kG992ShowtimeNumOfFHEC] = globalVar.shtCounters2lp[pathId][kG992ShowtimeNumOfFHEC];
            }
          }
            break;
        case kG992BitswapState:
            globalVar.adslMib.adslStat.bitswapStat.status = val;
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "kG992BitswapState status=%d",0,globalVar.adslMib.adslStat.bitswapStat.status);
            break;
        case kG992AocBitswapTxStarted:
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "kG992AocBitswapTxStarted xmtCntReq=%d",0,globalVar.adslMib.adslStat.bitswapStat.xmtCntReq);
#ifdef CONFIG_BCM_DSL_GFAST
            if(XdslMibIsGfastMod(gDslVars)) {
               int xoiType = (int)(kOlrXOIMask & val);
               int cntType = (int)(~kOlrXOIMask & val);
               if(kOlrNoi == xoiType)
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntUS);
               else if(kOlrDoi == xoiType)
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntUS);
               else if(kOlrNoiDoi == xoiType){
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntUS);
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntUS);
               }
            }
            else
#endif
               globalVar.adslMib.adslStat.bitswapStat.xmtCntReq++;
            break;
        case kG992AocBitswapRxStarted:
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "kG992AocBitswapRxStarted rcvCntReq=%d",0,globalVar.adslMib.adslStat.bitswapStat.rcvCntReq);
#ifdef CONFIG_BCM_DSL_GFAST
            if(XdslMibIsGfastMod(gDslVars)) {
               int xoiType = (int)(kOlrXOIMask & val);
               int cntType = (int)(~kOlrXOIMask & val);
               if(kOlrNoi == xoiType)
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntDS);
               else if(kOlrDoi == xoiType)
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntDS);
               else if(kOlrNoiDoi == xoiType){
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntDS);
                  updateGfastOlrStartedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntDS);
               }
            }
            else
#endif
               globalVar.adslMib.adslStat.bitswapStat.rcvCntReq++;
            break;
        case kG992AocBitswapTxCompleted:
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "kG992AocBitswapTxCompleted xmtCnt=%d",0,globalVar.adslMib.adslStat.bitswapStat.xmtCnt);
#ifdef CONFIG_BCM_DSL_GFAST
            if(XdslMibIsGfastMod(gDslVars)) {
               int xoiType = (int)(kOlrXOIMask & val);
               int cntType = (int)(~kOlrXOIMask & val);
               if(kOlrNoi == xoiType)
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntUS);
               else if(kOlrDoi == xoiType)
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntUS);
               else if(kOlrNoiDoi == xoiType) {
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntUS);
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntUS);
               }
            }
            else
#endif
               globalVar.adslMib.adslStat.bitswapStat.xmtCnt++;
            break;
        case kG992AocBitswapRxCompleted:
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "kG992AocBitswapRxCompleted rcvCnt=%d",0,globalVar.adslMib.adslStat.bitswapStat.rcvCnt);
#ifdef CONFIG_BCM_DSL_GFAST
            if(XdslMibIsGfastMod(gDslVars)) {
               int xoiType = (int)(kOlrXOIMask & val);
               int cntType = (int)(~kOlrXOIMask & val);
               if(kOlrNoi == xoiType)
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntDS);
               else if(kOlrDoi == xoiType)
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntDS);
               else if(kOlrNoiDoi == xoiType) {
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[0].cntDS);
                  updateGfastOlrCompletedCounter(cntType, &globalVar.adslMib.gfastOlrXoiCounterData[1].cntDS);
               }
            }
            else
#endif
               globalVar.adslMib.adslStat.bitswapStat.rcvCnt++;
            break;
        case kG992AocBitswapTxDenied:
            if(0 == val) {  /* 0 - rejected */
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
                if(0 == gLineId(gDslVars))
#endif
                __SoftDslPrintf(gDslVars, "kG992AocBitswapTxDenied xmtCntRej=%d",0,globalVar.adslMib.adslStat.bitswapStat.xmtCntRej);
                globalVar.adslMib.adslStat.bitswapStat.xmtCntRej++;
            }
            break;
        case kG992AocBitswapRxDenied:
            if(0 == val) {
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
                if(0 == gLineId(gDslVars))
#endif
                __SoftDslPrintf(gDslVars, "kG992AocBitswapRxDenied rcvCntRej=%d",0,globalVar.adslMib.adslStat.bitswapStat.rcvCntRej);
                globalVar.adslMib.adslStat.bitswapStat.rcvCntRej++;
            }
            break;
        case kDslChannelResponseLog:
            n = val << 1;
            nb = nMaxToneAllow * sizeof(globalVar.chanCharLog[0]);
            if (n > nb)
                n = nb;
            BlockShortMoveReverse(n>>1, status->param.dslConnectInfo.buffPtr, globalVar.chanCharLog);
            break;

        case kDslChannelResponseLinear:
            n = val << 1;
            nb = nMaxToneAllow * sizeof(globalVar.chanCharLin[0]);
            if (n > nb)
                n = nb;
            BlockShortMoveReverse(n>>1, status->param.dslConnectInfo.buffPtr, (short *)globalVar.chanCharLin);
            break;

        case kDslChannelQuietLineNoise:
            {
            int     i;
            uchar   *pNoiseBuf;
            
            n = (val < nMaxToneAllow) ? val : nMaxToneAllow;
            pNoiseBuf = (uchar *) status->param.dslConnectInfo.buffPtr;

            for (i = 0; i < n; i++)
                globalVar.quietLineNoise[i] = (-23 << 4) - (pNoiseBuf[i] << 3);
            }
            break;

        case kDslNLMaxCritNoise:
            globalVar.adslMib.adslNonLinData.maxCriticalDistNoise= val;
            break;

        case kDslNLAffectedBits:
            globalVar.adslMib.adslNonLinData.distAffectedBits=val;
            break;

        case kDslNLAffectedBins:
                globalVar.adslMib.adslNonLinData.NonLinNumAffectedBins=val;
                if(val>=globalVar.adslMib.adslNonLinData.NonLinThldNumAffectedBins)
                    globalVar.adslMib.adslNonLinData.NonLinearityFlag=1;
                else globalVar.adslMib.adslNonLinData.NonLinearityFlag=0;
                break;
                
        case    kDslATURXmtPowerInfo:
            globalVar.adslMib.adslPhys.adslCurrOutputPwr = Q4ToTenth(val);
            AdslMibUpdateACTPSD(gDslVars, val, TX_DIRECTION);
            break;

        case    kDslATUCXmtPowerInfo:
#ifndef USE_LOCAL_DS_POWER
            if(!AdslMibIsLinkActive(gDslVars) || !XdslMibIsXdsl2Mod(gDslVars)) {
                globalVar.adslMib.adslAtucPhys.adslCurrOutputPwr = Q4ToTenth(val);
                AdslMibUpdateACTPSD(gDslVars, val, RX_DIRECTION);
            }
#else
            globalVar.adslMib.adslAtucPhys.adslCurrOutputPwr = Q4ToTenth(val);
            AdslMibUpdateACTPSD(gDslVars, val, RX_DIRECTION);
#endif
            break;
#ifdef USE_LOCAL_DS_POWER
        case kDslATUCShowtimeXmtPowerInfo:
            globalVar.adslMib.adslAtucPhys.adslCurrOutputPwr = Q4ToTenth(val);
            AdslMibUpdateACTPSD(gDslVars, val, RX_DIRECTION);
            break;
#endif
        case    kDslFramingModeInfo:
            globalVar.adslMib.adslFramingMode = 
                (globalVar.adslMib.adslFramingMode & ~kAdslFramingModeMask) | (val & kAdslFramingModeMask);
            break;

        case    kDslATUHardwareAGCObtained:
            globalVar.adslMib.afeRxPgaGainQ1 = val >> 3;
            break;

        case    kDslAutoINPInUse:
            globalVar.adslMib.adsl2Info.autoINPInUse = val;
            break;

        default:
            break;
    }
    break;  /* kDslConnectInfoStatus */
    
    case kDslShowtimeSNRMarginInfo:
       if (status->param.dslShowtimeSNRMarginInfo.avgSNRMargin >= globalVar.showtimeMarginThld) {
           globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOM;
           if(0 == globalVar.adslMib.adslPhys.adslCurrStatus)
               globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusNoDefect;
       }
       else if (0 == (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS)) {
           globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOM;
           globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusNoDefect;
       }
       globalVar.adslMib.adslPhys.adslCurrSnrMgn = Q4ToTenth(status->param.dslShowtimeSNRMarginInfo.avgSNRMargin);
#ifdef G993
      if (XdslMibIsVdsl2Mod(gDslVars))
         copyBandPlanData((short *)status->param.dslShowtimeSNRMarginInfo.buffPtr, globalVar.showtimeMargin, &globalVar.adslMib.dsPhyBandPlan, globalVar.nTones);
      else
#endif
      {
         val = status->param.dslShowtimeSNRMarginInfo.nCarriers;
         if (val > nMaxToneAllow)
             val = nMaxToneAllow;
         if (val != 0)
             BlockShortMoveReverse(
                 val,
                 status->param.dslShowtimeSNRMarginInfo.buffPtr,
                 globalVar.showtimeMargin);
      }
      break;
    
    case kDslEscapeToG994p1Status:
      
      if(globalVar.adslMib.adslTrainingState != kAdslTrainingIdle) {
         if(kXdslLastInitStateStart == globalVar.lastInitState)
            globalVar.adslMib.xdslInitializationCause = kXdslInitConfigCommProblem;
      }
      globalVar.lastInitState = kXdslLastInitStateStart;
      globalVar.adslTpsTcOptions = 0;

      switch (globalVar.adslMib.adslTrainingState) {
        case kAdslTrainingConnected:
#if defined(CONFIG_BCM_DSL_GFAST)
          if(globalVar.adslMib.fastRetrainActive)
            IncFailureCounterVar(&globalVar.adslMib.adslPerfData, xdslFastInitErr);
#endif
          IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslRetr);
#ifdef SUPPORT_24HR_CNT_STAT
          IncStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, LinkRetrain);
#endif
          n = globalVar.adslMib.adslPhys.adslCurrStatus;
          if (n & kAdslPhysStatusLOS)
          {
              IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslRetrLos);
          }
          else if (n & kAdslPhysStatusLOF)
          {
              IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslRetrLof);
          }
          else if (n & kAdslPhysStatusLOM)
          {
              IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslRetrLom);
          }
          else if (n & kAdslPhysStatusLPR)
          {
              IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslRetrLpr);
          }
          break;

        case kAdslTrainingG992Started:
        case kAdslTrainingG992ChanAnalysis:
        case kAdslTrainingG992Exchange:
        case kAdslTrainingG993Started:
        case kAdslTrainingG993ChanAnalysis:
        case kAdslTrainingG993Exchange:
          IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslInitErr);
#ifdef SUPPORT_24HR_CNT_STAT
          IncStatHist24HrCounterVar(globalVar.adslMib.statHist24HrCounters, InitErrors);
#endif
          break;
        case kAdslTrainingG994:
          IncFailureCounterVar(&globalVar.adslMib.adslPerfData, adslLineSearch);
          break;  
      }
      if (0 == globalVar.timeConStarted)
          globalVar.timeConStarted = globalVar.timeSec;
#if defined(SUPPORT_TEQ_FAKE_LINKUP)
            if(!AdslDrvIsFakeLinkUp(gLineId(gDslVars)))
#endif
      globalVar.adslMib.adslTrainingState = kAdslTrainingIdle;
      globalVar.adslMib.adslRxNonStdFramingAdjustK = 0;
      globalVar.rsOptionValid = false;
      globalVar.rsOption[0] = 0;

      globalVar.nMsgCnt = 0;
      n = globalVar.linkStatus;
      globalVar.linkStatus = 0;
      globalVar.pathActive = 0;
#if defined(CONFIG_BCM_DSL_GFAST)
      globalVar.adslMib.fastRetrainActive = false;
      XDSL_CLR_CAPTURE_FLAGS;
#endif
      if (n != 0)
          AdslMibNotify(gDslVars, kAdslEventLinkChange);
      break; /* kDslEscapeToG994p1Status */

    case kDslOLRBitGainUpdateStatus:
        {
        void        *p = status->param.dslOLRRequest.carrParamPtr;
        Boolean     bAdsl2 = (status->param.dslOLRRequest.msgType >= kOLRRequestType4);

        for (n = 0; n < status->param.dslOLRRequest.nCarrs; n++)
            p = AdslMibUpdateBitGain(gDslVars, p, bAdsl2);  
        }   
        break;

    case kAtmStatus:
      pAtmData = (kAdslIntlChannel == AdslMibGetActiveChannel(gDslVars) ?
                  &globalVar.adslMib.adslChanIntlAtmPhyData : &globalVar.adslMib.adslChanFastAtmPhyData);
      switch (status->param.atmStatus.code) {
          case kAtmStatRxHunt:
          case kAtmStatRxPreSync:
              if (kAtmPhyStateNoAlarm == pAtmData->atmInterfaceTCAlarmState)
                  pAtmData->atmInterfaceOCDEvents++;
              pAtmData->atmInterfaceTCAlarmState = kAtmPhyStateLcdFailure;
              break;
          case kAtmStatRxSync:
              pAtmData->atmInterfaceTCAlarmState = kAtmPhyStateNoAlarm;
              break;
          case kAtmStatBertResult:
              globalVar.adslMib.adslBertRes.bertTotalBits = 
                  status->param.atmStatus.param.bertInfo.totalBits;
              globalVar.adslMib.adslBertRes.bertErrBits = 
                  status->param.atmStatus.param.bertInfo.errBits;
              break;
          case kAtmStatHdrCompr:
              if (status->param.atmStatus.param.value)
                  globalVar.adslMib.adslFramingMode |= kAtmHeaderCompression;
              else
                  globalVar.adslMib.adslFramingMode &= ~kAtmHeaderCompression;
              break;
          case kAtmStatCounters:
              {
              int n, m;
#ifdef  ADSLDRV_LITTLE_ENDIAN
              atmPhyCounters counters;
              atmPhyCounters *p = &counters;
              int *pSrc = (int *)(ADSL_ADDR_TO_HOST(status->param.atmStatus.param.value));
              BlockShortMoveReverse(2, (short *)pSrc, (short *)p);	/* id and bertStatus(first 2 ushort) */
              BlockLongMoveReverse((sizeof(atmPhyCounters)>>2)-1, pSrc+1, (int *)p+1);
#else
              atmPhyCounters  *p = (void *)(ADSL_ADDR_TO_HOST(status->param.atmStatus.param.value));
#endif
              pathId = XdslMibGetCurrentMappedPathId(gDslVars, RX_DIRECTION);
#if defined(CONFIG_BCM_DSL_GFAST)
              if ((0 == globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellTotal]) &&
                  (0 == globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellData]) && XdslMibIsGfastMod(gDslVars)) {
                /* A new G.fast showtime session */
                if((p->rxCellTotal > 6) || (p->txCellTotal > 6)) {
                  /* Not the first G.fast showtime session */
                  __SoftDslPrintf(gDslVars, "*** Not the first G.fast showtime session ***",0);
                  globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellData] = p->txCellData;
                  globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellTotal] = p->txCellTotal;
                  globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellDataRcved] = p->rxCellData;
                  globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellTotalRcved] = p->rxCellTotal;
                }
              }
#endif
              n = p->rxCellData - globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellDataRcved];
              IncAtmRcvCounterVar(&globalVar.adslMib, pathId, cntCellData, n);
              m = p->rxCellTotal - globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellTotalRcved];
#if defined(CONFIG_BCM_DSL_GFAST)
              if (XdslMibIsGfastMod(gDslVars))
                n += m;
              else
#endif
                n = m;
              IncAtmRcvCounterVar(&globalVar.adslMib, pathId, cntCellTotal, n);
              n = p->rxCellDrop - globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellDropRcved];
              IncAtmRcvCounterVar(&globalVar.adslMib, pathId, cntCellDrop, n);
              n = p->bertBitErrors - globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfBitErrsRcved];
              IncAtmRcvCounterVar(&globalVar.adslMib, pathId, cntBitErrs, n);
              globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellTotalRcved]= p->rxCellTotal;
              globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellDataRcved]= p->rxCellData;
              globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellDropRcved]= p->rxCellDrop;
              globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfBitErrsRcved]= p->bertBitErrors;
#if defined(CONFIG_BCM_DSL_GFAST)
              if (XdslMibIsGfastMod(gDslVars)) {
                n = p->txCellData - globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellData];
                if (n <= 0x1000000)
                  IncAtmXmtCounterVar(&globalVar.adslMib, 0, cntCellData, n);
                n += p->txCellTotal - globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellTotal];
                if (n <= 0x1000000)
                  IncAtmXmtCounterVar(&globalVar.adslMib, 0, cntCellTotal, n);
                globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellData] = p->txCellData;
                globalVar.shtExtCounters2lp[pathId][kG992ShowtimeNumOfCellTotal] = p->txCellTotal;
              }
#endif
              }
              break;

          default:
              break;
      }
      break; /* kAtmStatus */
    case kDslReceivedEocCommand:
      switch(status->param.dslClearEocMsg.msgId) {
#if defined(CONFIG_BCM_DSL_GFAST)
        case kDslGfastSupportedOption:
        {
          uchar *pData8 = (uchar *)status->param.dslClearEocMsg.dataPtr;
          globalVar.adslMib.gfastSupportedOptions = (pData8[1] << 8) | pData8[0];
          break;
        }

        case    kDslGfastDtaInfo:
        {
          GfastDtaOptions *pDta = (GfastDtaOptions *) status->param.dslClearEocMsg.dataPtr;
          globalVar.adslMib.gfastDta.dsCurRate = ADSL_ENDIAN_CONV_INT32(pDta->ndrDS);
          globalVar.adslMib.gfastDta.usCurRate = ADSL_ENDIAN_CONV_INT32(pDta->ndrUS);
          globalVar.adslMib.gfastDta.dtaFlags  = pDta->dtaFlags;
          globalVar.adslMib.gfastDta.curMds = pDta->currentMds;
          globalVar.adslMib.gfastDta.maxMds = pDta->maxMds;
          globalVar.adslMib.gfastDta.hsMds  = pDta->hsMds;

          globalVar.adslMib.xdslInfo.dirInfo[0].lpInfo[0].M = pDta->currentMds;
          globalVar.adslMib.xdslInfo.dirInfo[1].lpInfo[0].M = globalVar.adslMib.xdslInfo.dirInfo[1].lpInfo[0].Mf - 1 - pDta->currentMds;
          break;
        }
#endif
#ifdef CONFIG_TOD_SUPPORTED
        case kDslTodTimeStamp:
        {
          TodTimeStampRpt *pTodTimeStampRpt = (TodTimeStampRpt *)status->param.dslClearEocMsg.dataPtr;
#ifdef  ADSLDRV_LITTLE_ENDIAN
          TodTimeStampRpt tod;
          BlockLongMoveReverse(sizeof(TodTimeStampRpt) >> 2, (int *)pTodTimeStampRpt, (int *)&tod);
          pTodTimeStampRpt = &tod;
          pTodTimeStampRpt->secondsSinceEpoch = (pTodTimeStampRpt->secondsSinceEpoch << 32) | (pTodTimeStampRpt->secondsSinceEpoch >> 32);
#endif
          globalVar.todInfo.todTimeStampRpt = *pTodTimeStampRpt;
          break;
        }
#endif
#ifdef CONFIG_VDSL_SUPPORTED
        case kDsl993p2SnrROC:
        {
          short *pBuf =(short *)(status->param.dslClearEocMsg.dataPtr);
          globalVar.adslMib.xdslPhys.snrmRoc = Q8ToTenth(ADSL_ENDIAN_CONV_SHORT(pBuf[0]));
          break;
        }
        case kDslShowtimeSnrMarginHdrQ8:
        {
          dslShowtimeSNRMarginInfoType *pSnrMarginInfoType = (dslShowtimeSNRMarginInfoType *)status->param.dslClearEocMsg.dataPtr;
          int avgSNRMargin = ADSL_ENDIAN_CONV_INT32(pSnrMarginInfoType->avgSNRMargin) >> 4;  /* Q8  -->  Q4 */

          if (avgSNRMargin >= globalVar.showtimeMarginThld) {
              globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOM;
              if(0 == globalVar.adslMib.adslPhys.adslCurrStatus)
                  globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusNoDefect;
          }
          else if (0 == (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS)) {
              globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOM;
              globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusNoDefect;
          }
          globalVar.adslMib.adslPhys.adslCurrSnrMgn = Q4ToTenth(avgSNRMargin);
          break;
        }
        case kDslShowtimeSnrMarginDataQ8:
        {
          int k,n,i;short *SnrBuff;
          SnrBuff = (short *)status->param.dslClearEocMsg.dataPtr;
          if (XdslMibIsVdsl2Mod(gDslVars)) {
            k = 0;
            for(n = 0; n < globalVar.adslMib.dsPhyBandPlan.noOfToneGroups; n++)
            {
              if(globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone < globalVar.nTones)
                for(i = globalVar.adslMib.dsPhyBandPlan.toneGroups[n].startTone; i <= globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone; i++)
                  globalVar.showtimeMargin[i] = ADSL_ENDIAN_CONV_SHORT(SnrBuff[k++]) >> 4;   /* Q8  ==>  Q4 */
            }
          }
          else {
            val = (status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask) >> 1;
            if (val > nMaxToneAllow)
              val = nMaxToneAllow;
            for (i = 0; i < val; i++)
              globalVar.showtimeMargin[i] = ADSL_ENDIAN_CONV_SHORT(SnrBuff[i]) >> 4;   /* Q8  ==>  Q4 */
          }
          break;
        }
#endif
        case kGinpMonitoringCounters:
        {
          unsigned int x;
#ifdef  ADSLDRV_LITTLE_ENDIAN
          GinpCounters ginpCounter;
#endif
          GinpCounters *pGinpCounter = (GinpCounters *)status->param.dslClearEocMsg.dataPtr;
          n = status->param.dslClearEocMsg.msgType & 0xFFFF;
          if(n > sizeof(GinpCounters))
            n = sizeof(GinpCounters);
#ifdef  ADSLDRV_LITTLE_ENDIAN
          BlockLongMoveReverse(n >> 2, (int *)pGinpCounter, (int *)&ginpCounter);
          pGinpCounter = &ginpCounter;
#endif
#if 0 && defined(CONFIG_BCM_DSL_GFAST)
          if(!XdslMibIsGfastMod(gDslVars) ||
            (false == (globalVar.adslMib.fastRetrainActive || globalVar.uasOnSes || globalVar.currSecondSES)))
#endif
          {
          x = (pGinpCounter->rtx_tx -globalVar.ginpCounters.rtx_tx);
          AddGinpUsCounterVar(&globalVar.adslMib, rtx_tx, x);
          AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntUS, rtx_tx, x);
          x = (pGinpCounter->rtx_uc - globalVar.ginpCounters.rtx_uc);
          AddGinpDsCounterVar(&globalVar.adslMib, rtx_uc, x);
          AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntDS, rtx_uc, x);
          }
          x = (pGinpCounter->rtx_c - globalVar.ginpCounters.rtx_c);
          AddGinpDsCounterVar(&globalVar.adslMib, rtx_c, x);
          AddRtxCounterVar(&globalVar.adslMib.rtxCounterData.cntDS, rtx_c, x);
          AddGinpDsCounterVar(&globalVar.adslMib, LEFTRS, pGinpCounter->LEFTRS);
          x = pGinpCounter->errFreeBits - globalVar.ginpCounters.errFreeBits;
          AddGinpDsCounterVar(&globalVar.adslMib, errFreeBits, x);
          if(globalVar.minEFTRReported) {
            globalVar.adslMib.adslStat.ginpStat.cntDS.minEFTR = pGinpCounter->minEFTR;
            globalVar.adslMib.ginpExtRateStat.maxEFTRds = pGinpCounter->minEFTR;
            globalVar.minEFTRReported = false;
#if defined(CONFIG_BCM_DSL_GFAST)
            if((n >= sizeof(GinpCounters)) && pGinpCounter->ANDEFTRDS) {
               // Reset min/max ANDEFTR
              globalVar.adslMib.adslStat.gfastStat.txANDEFTRmin = pGinpCounter->ANDEFTR;
              globalVar.adslMib.adslStat.gfastStat.txANDEFTRmax = pGinpCounter->ANDEFTR;
            }
#endif
          }
          else {
            globalVar.adslMib.adslStat.ginpStat.cntDS.minEFTR = MIN(globalVar.adslMib.adslStat.ginpStat.cntDS.minEFTR, pGinpCounter->minEFTR);
            globalVar.adslMib.ginpExtRateStat.maxEFTRds = MAX(globalVar.adslMib.ginpExtRateStat.maxEFTRds, pGinpCounter->minEFTR);
#if defined(CONFIG_BCM_DSL_GFAST)
            if((n >= sizeof(GinpCounters)) && pGinpCounter->ANDEFTRDS) {
              // Evaluate min/max ANDEFTR
              globalVar.adslMib.adslStat.gfastStat.txANDEFTRmin = MIN(globalVar.adslMib.adslStatSincePowerOn.gfastStat.txANDEFTRmin, pGinpCounter->ANDEFTR);
              globalVar.adslMib.adslStat.gfastStat.txANDEFTRmax = MAX(globalVar.adslMib.adslStatSincePowerOn.gfastStat.txANDEFTRmax, pGinpCounter->ANDEFTR);
            }
#endif
          }
          globalVar.adslMib.adslStatSincePowerOn.ginpStat.cntDS.minEFTR = globalVar.adslMib.adslStat.ginpStat.cntDS.minEFTR;
#if defined(CONFIG_BCM_DSL_GFAST)
          if(n >= sizeof(GinpCounters)) {
            if(pGinpCounter->ANDEFTRDS) {
               // Accumulate ANDEFTRDS/LANDEFTRS/ANDEFTR
               AddGfastCounterVar(&globalVar.adslMib, txANDEFTRDS, pGinpCounter->ANDEFTRDS);
               AddGfastCounterVar(&globalVar.adslMib, txLANDEFTRS, pGinpCounter->LANDEFTRS);
               globalVar.txANDEFTRacc = (globalVar.txANDEFTRacc + pGinpCounter->ANDEFTR) & ((ulonglong)-1 >> 16);   // 48 bits wraparound counter;
               x = (globalVar.txANDEFTRacc + ((1 << 16)-1))/(1 << 16);  // Ceiling of txANDEFTRacc/(1<<16) = txANDEFTRsum
               globalVar.adslMib.adslStat.gfastStat.txANDEFTRsum = x;
               // Update StatSincePowerOn min/max ANDEFTR
               globalVar.adslMib.adslStatSincePowerOn.gfastStat.txANDEFTRmin = globalVar.adslMib.adslStat.gfastStat.txANDEFTRmin;
               globalVar.adslMib.adslStatSincePowerOn.gfastStat.txANDEFTRmax = globalVar.adslMib.adslStat.gfastStat.txANDEFTRmax;
            }
          }
#endif
          globalVar.ginpCounters.rtx_tx = pGinpCounter->rtx_tx;
          globalVar.ginpCounters.rtx_c = pGinpCounter->rtx_c;
          globalVar.ginpCounters.rtx_uc = pGinpCounter->rtx_uc;
          globalVar.ginpCounters.errFreeBits = pGinpCounter->errFreeBits;
          if((n >= GINP_COUNTERS_SEFTR_STRUCT_SIZE) && (0 != pGinpCounter->SEFTR)
#ifdef CONFIG_BCM_DSL_GFAST
              && !globalVar.adslMib.fastRetrainActive && !G992p3OvhMsgIsL3RspPending(gDslVars)
#endif
            )
            AdslMibSES(globalVar.currSecondSES, adslSES);
          break;
        }
#ifdef CONFIG_BCM_DSL_GFAST
        case kGfastEocMonitoringCounters:
        {
          uint x;
          gfastEocCounters *pEocCounters = (gfastEocCounters *)status->param.dslClearEocMsg.dataPtr;
#ifdef ADSLDRV_LITTLE_ENDIAN
          gfastEocCounters eocCounters;
          BlockLongMoveReverse(sizeof(gfastEocCounters) >> 2, (int *)pEocCounters, (int *)&eocCounters);
          pEocCounters = &eocCounters;
#endif
          x = pEocCounters->bytesSent - globalVar.eocCounters.bytesSent;
          AddGfastEocfCounterVar(&globalVar.adslMib, bytesSent, x);
          x = pEocCounters->bytesReceived - globalVar.eocCounters.bytesReceived;
          AddGfastEocfCounterVar(&globalVar.adslMib, bytesReceived, x);
          x = pEocCounters->packetsSent - globalVar.eocCounters.packetsSent;
          AddGfastEocfCounterVar(&globalVar.adslMib, packetsSent, x);
          x = pEocCounters->packetsReceived - globalVar.eocCounters.packetsReceived;
          AddGfastEocfCounterVar(&globalVar.adslMib, packetsReceived, x);
          x = pEocCounters->messagesSent - globalVar.eocCounters.messagesSent;
          AddGfastEocfCounterVar(&globalVar.adslMib, messagesSent, x);
          x = pEocCounters->messagesReceived - globalVar.eocCounters.messagesReceived;
          AddGfastEocfCounterVar(&globalVar.adslMib, messagesReceived, x);
          globalVar.eocCounters = *pEocCounters;
        }
          break;
#ifdef SUPPORT_VECTORING
        case kDslGfastVectoringEocSegments:
        {
          GfastTxVectorFBEocSegment *pVectorFBCounters = (GfastTxVectorFBEocSegment *)status->param.dslClearEocMsg.dataPtr;
#ifdef ADSLDRV_LITTLE_ENDIAN
          GfastTxVectorFBEocSegment vectorFBCounters;
          BlockLongMoveReverse(sizeof(GfastTxVectorFBEocSegment) >> 2, (int *)pVectorFBCounters, (int *)&vectorFBCounters);
          pVectorFBCounters = &vectorFBCounters;
#endif
          globalVar.adslMib.vectData.vectStat.cntESPktSend += (pVectorFBCounters->cntVecFBSegmentSend - globalVar.vectorFBCounters.cntVecFBSegmentSend);
          globalVar.adslMib.vectData.vectStat.cntESPktDrop += (pVectorFBCounters->cntVecFBSegmentDrop - globalVar.vectorFBCounters.cntVecFBSegmentDrop);
          globalVar.adslMib.vectData.vectStat.cntESStatSend += (pVectorFBCounters->cntVecFBMessageSend - globalVar.vectorFBCounters.cntVecFBMessageSend);
          globalVar.adslMib.vectData.vectStat.cntESStatDrop += (pVectorFBCounters->cntVecFBMessageDrop - globalVar.vectorFBCounters.cntVecFBMessageDrop);
          globalVar.vectorFBCounters = *pVectorFBCounters;
          break;
        }
#endif  /* SUPPORT_VECTORING */
#endif
        case kDsl993p2FramerDeframerUs:
        case kDsl993p2FramerDeframerDs:
        {
          int dir, cpyLen;
          Boolean *pSwapPath;
          FramerDeframerOptions framerParam;
          FramerDeframerOptions *pFramerParam = &framerParam;
          int msgId = status->param.dslClearEocMsg.msgId;
          
          n = status->param.dslClearEocMsg.msgType & 0xFFFF;
          cpyLen = MIN(n, sizeof(FramerDeframerOptions));
          BlockByteMove(cpyLen, (unsigned char *)status->param.dslClearEocMsg.dataPtr, (unsigned char *)pFramerParam);
#ifdef  ADSLDRV_LITTLE_ENDIAN
          /* 1st 7 fields are short, the rest are char so just convert the 1st 7 */
          BlockShortMoveReverse(7, (short *)pFramerParam, (short *)pFramerParam);
          /* a uint, ETR_kbps was added */
          if(n >= GINP_FRAMER_ETR_STRUCT_SIZE)
            pFramerParam->ETR_kbps = ADSL_ENDIAN_CONV_INT32(pFramerParam->ETR_kbps);
          /* a ushort, inpShine was added to allow reporting of INP > 127 */
          if(n >= GINP_FRAMER_INPSHINE_STRUCT_SIZE)
            pFramerParam->INPshine = ADSL_ENDIAN_CONV_SHORT(pFramerParam->INPshine);
          /* 32bits L */
          if(n >= L32_FRAMER_STRUCT_SIZE)
            pFramerParam->L = ADSL_ENDIAN_CONV_INT32(pFramerParam->L);
          /* GFAST: maxMemory, ndr, Ldr, Nret */
          if(n >= Nret_FRAMER_STRUCT_SIZE) {
            pFramerParam->maxMemory = ADSL_ENDIAN_CONV_INT32(pFramerParam->maxMemory);
            pFramerParam->ndr = ADSL_ENDIAN_CONV_INT32(pFramerParam->ndr);
            pFramerParam->Ldr = ADSL_ENDIAN_CONV_SHORT(pFramerParam->Ldr);
          }
          if(n >= ETR_MIN_EOC_FRAMER_STRUCT_SIZE) {
            pFramerParam->etru = ADSL_ENDIAN_CONV_INT32(pFramerParam->etru);
            pFramerParam->Lmax = ADSL_ENDIAN_CONV_INT32(pFramerParam->Lmax);
            pFramerParam->Lmin = ADSL_ENDIAN_CONV_INT32(pFramerParam->Lmin);
            pFramerParam->ETRminEoc = ADSL_ENDIAN_CONV_INT32(pFramerParam->ETRminEoc);
          }
          if(n >= RX_QUEUE16_FRAMER_STRUCT_SIZE) {
            pFramerParam->fireRxQueue = ADSL_ENDIAN_CONV_SHORT(pFramerParam->fireRxQueue);
          }
#endif
          if(n < L32_FRAMER_STRUCT_SIZE)
            pFramerParam->L = pFramerParam->L16;

          if(n < RX_QUEUE16_FRAMER_STRUCT_SIZE)
            pFramerParam->fireRxQueue = pFramerParam->fireRxQueueOld;
          
          if(kDsl993p2FramerDeframerUs == msgId) {
#ifdef PHY_CO
            pSwapPath = &globalVar.swapRxPath;
#else
            pSwapPath = &globalVar.swapTxPath;
#endif
            dir = US_DIRECTION;
          }
          else {
#ifdef PHY_CO
            pSwapPath = &globalVar.swapTxPath;
#else
            pSwapPath = &globalVar.swapRxPath;
#endif
            dir = DS_DIRECTION;
          }

          pathId = (pFramerParam->path < MAX_LP_NUM) ? pFramerParam->path: 0;
          if((n >= GINP_FRAMER_STRUCT_SIZE) && (0 == pathId) && ((uchar)-1 == pFramerParam->ahifChanId[0]))
            *pSwapPath = true;
          
          Xdsl2MibSetConnectionInfo(gDslVars, pFramerParam, dir, n);
          if(*pSwapPath)
            pathId ^= 1;
          Xdsl2MibConvertConnectionInfo(gDslVars, pathId, dir);
 #ifdef CONFIG_VDSL_SUPPORTED
          if(XdslMibIsVdsl2Mod(gDslVars))
              AdslMibCalcVdslPERp(gDslVars, pathId, dir);
          else
 #endif
              AdslMibCalcAdslPERp(gDslVars, pathId, dir);
          break;
        }
        case kDsl993p2dsATTNDR:
        {
          int val = ADSL_ENDIAN_CONV_INT32(*(int *)status->param.dslClearEocMsg.dataPtr);
#ifdef USE_TRAINING_ATTNDR
          int newAttndr = ActualRate(val*1000);
          int halfLastAttndr = ((globalVar.adslMib.xdslPhys.adslCurrAttainableRate)>>11); /* Hz to kHz and divide by 2 (div by 2048) */
          pathId = XdslMibGetCurrentMappedPathId(gDslVars, DS_DIRECTION);

          /* Set adslShowAttainableRate only once the first time in showtime */
          if (globalVar.adslMib.xdslPhys.adslShowAttainableRate == 0)
            globalVar.adslMib.xdslPhys.adslShowAttainableRate = newAttndr;

          /* If GINP or if the DS rate is at least half the training ATTNDR then just use the PHY
             ATTNDR value, else compute it based on training ATTNDR. The idea is that if the 
             DS rate is less than half the training ATTNDR then showtime is using a reduced tone
             set (NULL loop capped rate) and the customer wants to see an ATTNDR relative to the
             much larger training ATTNDR */
          if (AdslMibIsLinkActive(gDslVars) && (XdslMibIsGinpActive(gDslVars,DS_DIRECTION) ||
             (globalVar.adslMib.xdslInfo.dirInfo[DS_DIRECTION].lpInfo[pathId].dataRate > halfLastAttndr)))
          {
            globalVar.adslMib.xdslPhys.adslCurrAttainableRate = newAttndr;
          }
          else
          {
            globalVar.adslMib.xdslPhys.adslCurrAttainableRate = globalVar.adslMib.xdslPhys.adslTrainAttainableRate + (newAttndr - globalVar.adslMib.xdslPhys.adslShowAttainableRate);
          }
#else
          globalVar.adslMib.adslPhys.adslCurrAttainableRate=ActualRate(val*1000);
#endif
          XdslMibUpdateAttnEtr(gDslVars, DS_DIRECTION);
        }
        break;

        case kDsl993p2FramerAdslUs:
        case kDsl993p2FramerAdslDs:
        {
          adsl2ChanInfo *pChanInfo;
          xdslFramingInfo  *pFramingInfo;
          FramerDeframerOptions *pFramerParam = (FramerDeframerOptions *) status->param.dslClearEocMsg.dataPtr;
          int msgId = status->param.dslClearEocMsg.msgId;
          pathId = (pFramerParam->path < MAX_LP_NUM) ? pFramerParam->path: 0;

          if(kDsl993p2FramerAdslUs == msgId) {
              pChanInfo = &globalVar.adslMib.adsl2Info2lp[pathId].xmtChanInfo;
              pFramingInfo = &globalVar.adslMib.xdslInfo.dirInfo[US_DIRECTION].lpInfo[pathId];
              pFramingInfo->tpsTcOptions = (globalVar.adslTpsTcOptions & 2) ? kXdslTpsTcPtmPreemption : 0;
          }
          else {
              pChanInfo = &globalVar.adslMib.adsl2Info2lp[pathId].rcvChanInfo;
              pFramingInfo = &globalVar.adslMib.xdslInfo.dirInfo[DS_DIRECTION].lpInfo[pathId];
              pFramingInfo->tpsTcOptions = (globalVar.adslTpsTcOptions & 1) ? kXdslTpsTcPtmPreemption : 0;
          }
          pChanInfo->ahifChanId = pFramerParam->ahifChanId[0];
          pChanInfo->connectionType = pFramerParam->tmType[0];
          break;
        }
#ifdef NTR_SUPPORT
      case kDslNtrCounters:
      {
         dslNtrData* pNtr =(dslNtrData*) status->param.dslClearEocMsg.dataPtr;
         int   ntrCntLen = status->param.dslClearEocMsg.msgType & 0xFFFF;

         if (ntrCntLen > sizeof(dslNtrData))
            ntrCntLen = sizeof(dslNtrData);
         //AdslMibByteMove (ntrCntLen, pNtr, &globalVar.adslMib.ntrCnt);
         BlockLongMoveReverse(ntrCntLen>>2, (int *)pNtr, (int *)&globalVar.adslMib.ntrCnt);
      }
         break;
#endif
#ifdef CONFIG_VDSL_SUPPORTED
      case kDsl993p2USkl0perBand:
      case kDsl993p2DSkl0perBand:
      {
        int i;
        short *pDst;
#ifdef ADSLDRV_LITTLE_ENDIAN
        short kl0PerBand[MAX_NUM_BANDS];
        short *pSrc = &kl0PerBand[0];
#else
        short *pSrc = (short *)status->param.dslClearEocMsg.dataPtr;
#endif
        int msgLen = status->param.dslClearEocMsg.msgType & 0xFFFF;
        int numKl0BandReported = MIN((msgLen>>1), MAX_NUM_BANDS);
        if(kDsl993p2DSkl0perBand == status->param.dslClearEocMsg.msgId) {
          pDst = &globalVar.adslMib.xdslPhys.kl0PerBand[0];
          globalVar.adslMib.xdslPhys.numKl0BandReported = numKl0BandReported;
        }
        else {
          pDst = &globalVar.adslMib.xdslAtucPhys.kl0PerBand[0];
          globalVar.adslMib.xdslAtucPhys.numKl0BandReported = numKl0BandReported;
        }
#ifdef ADSLDRV_LITTLE_ENDIAN
        BlockShortMoveReverse(numKl0BandReported, (short *)status->param.dslClearEocMsg.dataPtr, &kl0PerBand[0]);
#endif
        for(i = 0; i < numKl0BandReported; i++)
          pDst[i] = (short)(((int)pSrc[i] * 100) >> 8);
      }
        break;
      case kDsl993p2LnAttnAvg:
        val = ADSL_ENDIAN_CONV_SHORT(*(short *)status->param.dslClearEocMsg.dataPtr);
        val = Q8ToTenth(val);
        globalVar.adslMib.xdslPhys.adslCurrAtn = RestrictValue(val, 0, 1630);
        break;
      case kDsl993p2dsATTNDRmethod:
        globalVar.adslMib.xdslPhys.attnDrMethod = *(uchar *)status->param.dslClearEocMsg.dataPtr;
        globalVar.adslMib.xdslAtucPhys.attnDrMethod = *(uchar *)status->param.dslClearEocMsg.dataPtr;
        break;
      case kDsl993p2dsATTNDRinp:
        globalVar.adslMib.xdslPhys.attnDrInp = *(uchar *)status->param.dslClearEocMsg.dataPtr;
        break;
      case kDsl993p2dsATTNDRdel:
        globalVar.adslMib.xdslPhys.attnDrDelay = *(uchar *)status->param.dslClearEocMsg.dataPtr;
        break;

      case kDsl993p2BandPlanDsDump:
      {
        unsigned short lastTone;
#ifdef ADSLDRV_LITTLE_ENDIAN
        bandPlanDescriptor32 bpData;
#endif
        bandPlanDescriptor32 *bp;
        int i;
        if(globalVar.waitBandPlan)
        {
          char vendorId[kAdslPhysVendorIdLen];
          char adslVersionNumber[kAdslPhysVersionNumLen];

          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.usNegBandPlan);
          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.dsNegBandPlan);
          BlockByteClear(sizeof(bandPlanDescriptor32), (void*)&globalVar.adslMib.usNegBandPlan32);
          BlockByteClear(sizeof(bandPlanDescriptor32), (void*)&globalVar.adslMib.dsNegBandPlan32);
          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.usPhyBandPlan);
          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.dsPhyBandPlan);
          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.usNegBandPlanDiscovery);
          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.dsNegBandPlanDiscovery);
          BlockByteClear(sizeof(bandPlanDescriptor32), (void*)&globalVar.adslMib.usNegBandPlanDiscovery32);
          BlockByteClear(sizeof(bandPlanDescriptor32), (void*)&globalVar.adslMib.dsNegBandPlanDiscovery32);
          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.usPhyBandPlanDiscovery);
          BlockByteClear(sizeof(bandPlanDescriptor), (void*)&globalVar.adslMib.dsPhyBandPlanDiscovery);
          BlockByteMove(kAdslPhysVendorIdLen, &globalVar.adslMib.xdslAtucPhys.adslVendorID[0], vendorId);
          BlockByteMove(kAdslPhysVersionNumLen, &globalVar.adslMib.xdslAtucPhys.adslVersionNumber[0], adslVersionNumber);
          BlockByteClear(sizeof(globalVar.adslMib.xdslAtucPhys), (void*)&globalVar.adslMib.xdslAtucPhys);
          BlockByteMove(kAdslPhysVendorIdLen, vendorId, &globalVar.adslMib.xdslAtucPhys.adslVendorID[0]);
          BlockByteMove(kAdslPhysVersionNumLen, adslVersionNumber, &globalVar.adslMib.xdslAtucPhys.adslVersionNumber[0]);

          BlockByteClear(sizeof(globalVar.adslMib.vdslDiag), (void*)&globalVar.adslMib.vdslDiag[0]);
          BlockByteClear(sizeof(globalVar.adslMib.perbandDataUs), (void*)&globalVar.adslMib.perbandDataUs[0]);
          BlockByteClear(sizeof(globalVar.adslMib.perbandDataDs), (void*)&globalVar.adslMib.perbandDataDs[0]);
          i = XdslMibGetMaxToneNum(gDslVars);
          BlockByteClear(i, (void*)globalVar.snr);
          BlockByteClear(i, (void*)globalVar.showtimeMargin);
          BlockByteClear(i, (void*)globalVar.bitAlloc);
          BlockByteClear(i, (void*)globalVar.gain);
          BlockByteClear(i, (void*)globalVar.chanCharLin);
          for(i=0; i<globalVar.nTones; i++)
          {
            globalVar.quietLineNoise[i]=(-160)<<4;
#if defined(CONFIG_RNC_SUPPORT)
            globalVar.quietLineNoiseRnc[i]=(-160)<<4;
#endif
            globalVar.chanCharLog[i]=(-96)<<4;
          }
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars)) {
            for(i = 0; i < globalVar.nTonesGfast; i++)
              globalVar.activeLineNoise[i] = (-160) << 4;
            BlockByteClear(sizeof(globalVar.doiBitAlloc[0])*globalVar.nTonesGfast, (void*)globalVar.doiBitAlloc);
            BlockByteClear(sizeof(globalVar.doiGain[0])*globalVar.nTonesGfast, (void*)globalVar.doiGain);
          }
#endif
          globalVar.waitfirstQLNstatusLD=1;
          globalVar.waitfirstHLOGstatusLD=1;
          globalVar.waitfirstSNRstatusLD=1;
          globalVar.waitBandPlan=0;
        }
        bp=(bandPlanDescriptor32*)(status->param.dslClearEocMsg.dataPtr);
#ifdef ADSLDRV_LITTLE_ENDIAN
        bpData.noOfToneGroups = bp->noOfToneGroups;
        bpData.reserved = bp->reserved;
        BlockShortMoveReverse(bp->noOfToneGroups << 1, (short *)bp->toneGroups, (short *)bpData.toneGroups);
        bp=&bpData;
#endif
        lastTone = bp->toneGroups[bp->noOfToneGroups-1].endTone;
        if(globalVar.bandPlanType==MedleyPhase)
        {
          if(bp->reserved==PhyBandPlan)
          {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.dsPhyBandPlan);
            globalVar.bpSNR=&globalVar.adslMib.dsPhyBandPlan;
            globalVar.waitfirstSNRstatusMedley=1;
            globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETds = calcOamGfactor(lastTone);
            globalVar.dsPhysGfactor = &globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETds;
            __SoftDslPrintf(gDslVars, "Physical Gfactor_MEDLEYSETds=%d\n",0,globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETds);
          }
          else if (bp->reserved==NegBandPlan)
          {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.dsNegBandPlan32);
            globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds=calcOamGfactor(lastTone);
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "Gfactor_MEDLEYSETds=%d\n",0,globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds);
          }
          if (globalVar.adslMib.dsPhyBandPlan.noOfToneGroups>0 && globalVar.adslMib.dsNegBandPlan32.noOfToneGroups>0)
            CreateNegBandPlan(gDslVars,&globalVar.adslMib.dsNegBandPlan, &globalVar.adslMib.dsNegBandPlan32, &globalVar.adslMib.dsPhyBandPlan);
        }
        else
        {
          if(bp->reserved==PhyBandPlan)
          {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.dsPhyBandPlanDiscovery);
            globalVar.bpSNR=&globalVar.adslMib.dsPhyBandPlanDiscovery;
            globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSds = calcOamGfactor(lastTone);
            globalVar.dsPhysGfactor = &globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSds;
            __SoftDslPrintf(gDslVars, "Physical Gfactor_SUPPORTERCARRIERSds=%d\n",0,globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSds);
          }
          else if (bp->reserved==NegBandPlan)
          {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.dsNegBandPlanDiscovery32);
            globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds=calcOamGfactor(lastTone);
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
            __SoftDslPrintf(gDslVars, "Gfactor_SUPPORTERCARRIERSds=%d\n",0,globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds);
          }
          if (globalVar.adslMib.dsPhyBandPlanDiscovery.noOfToneGroups>0 && globalVar.adslMib.dsNegBandPlanDiscovery32.noOfToneGroups>0)
            CreateNegBandPlan(gDslVars,&globalVar.adslMib.dsNegBandPlanDiscovery, &globalVar.adslMib.dsNegBandPlanDiscovery32, &globalVar.adslMib.dsPhyBandPlanDiscovery);
          }
      }
        break;
      case kDsl993p2BandPlanUsDump:
      {
        unsigned short lastTone;
        bandPlanDescriptor32 *bp;
#ifdef ADSLDRV_LITTLE_ENDIAN
        bandPlanDescriptor32 bpData;
#endif
        bp=(bandPlanDescriptor32*)(status->param.dslClearEocMsg.dataPtr);
#ifdef ADSLDRV_LITTLE_ENDIAN
        bpData.noOfToneGroups = bp->noOfToneGroups;
        bpData.reserved = bp->reserved;
        BlockShortMoveReverse(bp->noOfToneGroups << 1, (short *)bp->toneGroups, (short *)bpData.toneGroups);
        bp=&bpData;
#endif
        lastTone = bp->toneGroups[bp->noOfToneGroups-1].endTone;
        if(globalVar.bandPlanType==MedleyPhase)
        {
          if(bp->reserved==PhyBandPlan) {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.usPhyBandPlan);
            globalVar.adslMib.physGfactors.Gfactor_MEDLEYSETus = calcOamGfactor(lastTone);
          }
          else if (bp->reserved==NegBandPlan)
          {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.usNegBandPlan32);
            globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus=calcOamGfactor(lastTone);
            __SoftDslPrintf(gDslVars, "Gfactor_MEDLEYSETus=%d\n",0,globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus);
          }
          if (globalVar.adslMib.usPhyBandPlan.noOfToneGroups>0 && globalVar.adslMib.usNegBandPlan32.noOfToneGroups>0)
            CreateNegBandPlan(gDslVars,&globalVar.adslMib.usNegBandPlan, &globalVar.adslMib.usNegBandPlan32, &globalVar.adslMib.usPhyBandPlan);
        }
        else
        {
          if(bp->reserved==PhyBandPlan) {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.usPhyBandPlanDiscovery);
            globalVar.adslMib.physGfactors.Gfactor_SUPPORTERCARRIERSus = calcOamGfactor(lastTone);
          }
          else if (bp->reserved==NegBandPlan)
          {
            AdslMibByteMove((bp->noOfToneGroups << 2)+2, bp, &globalVar.adslMib.usNegBandPlanDiscovery32);
            globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus=calcOamGfactor(lastTone);
            __SoftDslPrintf(gDslVars, "Gfactor_SUPPORTERCARRIERSus=%d\n",0,globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus);
          }
          if (globalVar.adslMib.usPhyBandPlanDiscovery.noOfToneGroups>0 && globalVar.adslMib.usNegBandPlanDiscovery32.noOfToneGroups>0)
            CreateNegBandPlan(gDslVars,&globalVar.adslMib.usNegBandPlanDiscovery, &globalVar.adslMib.usNegBandPlanDiscovery32, &globalVar.adslMib.usPhyBandPlanDiscovery);
        }
      }
        break;
      case kDsl993p2QlnRaw:
#if defined(CONFIG_RNC_SUPPORT)
      case kDsl993p2QlnRawRnc:
#endif
      {
        int k,n,i,j; short *QLNBuff,*pQuietLineNoise;
        QLNBuff=(short *)(status->param.dslClearEocMsg.dataPtr);
        globalVar.dsBpQLNForReport=&globalVar.adslMib.dsPhyBandPlanDiscovery;
        k=0;
        if(globalVar.waitfirstQLNstatusLD==1)
            globalVar.waitfirstQLNstatusLD=0;
#ifdef CONFIG_BCM_DSL_GFAST
        if(ADSL_PHY_SUPPORT(kAdslPhyGfast) && XdslMibIsGfastMod(gDslVars))
            j = 754;
        else
#endif
        if (globalVar.adslMib.xdslInfo.vdsl2Profile!=kVdslProfile30a)
            j = 582;
        else
            j = 630;
#if defined(CONFIG_RNC_SUPPORT)
        if(kDsl993p2QlnRawRnc == status->param.dslClearEocMsg.msgId)
            pQuietLineNoise = globalVar.quietLineNoiseRnc;
        else
#endif
        pQuietLineNoise = globalVar.quietLineNoise;
        for(n=0;n<globalVar.adslMib.dsPhyBandPlanDiscovery.noOfToneGroups;n++)
        {
            if(globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].endTone < globalVar.nTones)
                for(i=globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].startTone;i<=globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].endTone;i++)
                    pQuietLineNoise[i]=(short)(ADSL_ENDIAN_CONV_SHORT(QLNBuff[k++])/16-j); //convert dBm per tone to dBm per Hz
        }
      }
#ifdef SUPPORT_SELT
        AdslMibNotify(gDslVars, kXdslEventSeltNext);
#endif
        break;
#ifdef CONFIG_BCM_DSL_GFAST
      case kDsl993p2AlnRaw:
      {
        int k,n,i;
        short *ALNBuff;
        
        ALNBuff = (short *)(status->param.dslClearEocMsg.dataPtr);
        k=0;
        for(n = 0; n < globalVar.adslMib.dsPhyBandPlanDiscovery.noOfToneGroups; n++)
        {
            if(globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].endTone >= globalVar.nTonesGfast)
                break;
            for(i = globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].startTone; i <= globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].endTone; i++)
                globalVar.activeLineNoise[i] = (short)((ADSL_ENDIAN_CONV_SHORT(ALNBuff[k++]) >> 4) - 754); //convert dBm per tone to dBm per Hz
        }
      }
        break;
#ifdef SUPPORT_HMI
      case kDsl993p2MrefPSDus:
      case kDsl993p2MrefPSDds:
      {
        
        short *pDst, *pSrc = (short *)status->param.dslClearEocMsg.dataPtr;
        n = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
        n = (n < sizeof(UsMrefPsdDescriptor)) ? n: sizeof(UsMrefPsdDescriptor);
        if(kDsl993p2MrefPSDus == status->param.dslClearEocMsg.msgId)
          pDst = (short *)&globalVar.gMrefPsd.usDescriptor;
        else
          pDst = (short *)&globalVar.gMrefPsd.dsDescriptor;
        *pDst = *pSrc;
        n = (n >> 1)-1;
        BlockShortMoveReverse(n, (pSrc+1), (pDst+1));
        __SoftDslPrintf(gDslVars, "reserved = %02x, n = %02x", 0, ((uchar *)pDst)[0], ((uchar *)pDst)[1]);
      }
        break;
#endif
#endif
#if defined(CONFIG_RNC_SUPPORT)
      case kDslChannelQlnRnc:
      {
        int     i;
        uchar   *pNoiseBuf;
        n = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
        if(n > nMaxToneAllow)
            n = nMaxToneAllow;
        pNoiseBuf = (uchar *)status->param.dslClearEocMsg.dataPtr;
        for (i = 0; i < n; i++)
            globalVar.quietLineNoiseRnc[i] = (-23 << 4) - (pNoiseBuf[i] << 3);
      }
        break;
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
      case kDsl993p2TestHlin:
          n = (status->param.dslClearEocMsg.msgType & 0x1FFFF) >> 3; /* ComplexLong */
          if(n > nMaxToneAllow)
             n = nMaxToneAllow;
          BlockLongMoveReverse(n<<1,(int *)(status->param.dslClearEocMsg.dataPtr),(int *)globalVar.uer);
      break;
#ifdef SUPPORT_SELT
      case kDslEchoVariance:
          BlockShortMoveReverse(kVdslMibMaxToneNum,(short *)(status->param.dslClearEocMsg.dataPtr),(short *)globalVar.echoVariance);
        break;
#endif
#endif
      case kDsl993p2HlogRaw:
      {
          int k,n,i;short *HlogBuff;
          HlogBuff=(short *)(status->param.dslClearEocMsg.dataPtr); 
          k=0;
          globalVar.dsBpHlogForReport=&globalVar.adslMib.dsPhyBandPlanDiscovery;
          for(n=0;n<globalVar.adslMib.dsPhyBandPlanDiscovery.noOfToneGroups;n++)
            if(globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].endTone < globalVar.nTones)
                for(i=globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].startTone;i<=globalVar.adslMib.dsPhyBandPlanDiscovery.toneGroups[n].endTone;i++)
                  globalVar.chanCharLog[i]=(short)ADSL_ENDIAN_CONV_SHORT(HlogBuff[k++])/16;
      }
        break;
      case kDsl993p2SnrRaw:
      {
          int k,n,i;short *SnrBuff;
          globalVar.dsBpSNRForReport=globalVar.bpSNR;
          globalVar.dsGfactorForSNRReport = globalVar.dsPhysGfactor;
          if(globalVar.waitfirstSNRstatusMedley==1)
          {
            BlockByteClear(nMaxToneAllow, (void*)globalVar.snr);
            globalVar.waitfirstSNRstatusMedley=0;
          }
          SnrBuff=(short *)(status->param.dslClearEocMsg.dataPtr);
          k=0;
          for(n=0;n<globalVar.bpSNR->noOfToneGroups;n++)
          {
            if(globalVar.bpSNR->toneGroups[n].endTone < globalVar.nTones)
                for(i=globalVar.bpSNR->toneGroups[n].startTone;i<=globalVar.bpSNR->toneGroups[n].endTone;i++)
                  globalVar.snr[i]=(short)ADSL_ENDIAN_CONV_SHORT(SnrBuff[k++])/16;
          }
      }
        break;
      case kDsl993p2NeBi:
#ifdef CONFIG_BCM_DSL_GFAST
      case kDsl993p2DOINeBi:
#endif
      {
          int i,k,n, nTones; uchar *pBuf;
          uchar *bitAlloc;
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars)) {
              if(kDsl993p2DOINeBi == status->param.dslClearEocMsg.msgId)
                 bitAlloc = globalVar.doiBitAlloc;
              else
                 bitAlloc = globalVar.bitAlloc;
              nTones = globalVar.nTonesGfast;
          }
          else
#endif
          {
              bitAlloc = globalVar.bitAlloc;
              nTones = globalVar.nTones;
          }
          pBuf =(uchar *)(status->param.dslClearEocMsg.dataPtr); 
          k=0;
          for(n=0;n<globalVar.adslMib.dsNegBandPlan32.noOfToneGroups;n++)
          {
              if(globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone >= nTones)
                  break;
              for(i=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone;i++)
              {
                  bitAlloc[i] = (pBuf[k] > 0xF) ? 0: pBuf[k];
                  k++;
              }
          }
      }
        break;
      case kDsl993p2NeBiPhy: /* VDSL Only */
      {
          int i,k,n;
          uchar *pBuf;
          VdslToneGroup *pNegToneGroup;
          pBuf =(uchar *)(status->param.dslClearEocMsg.dataPtr);
          k=0;
          
          for(n = 0; n < globalVar.adslMib.dsPhyBandPlan.noOfToneGroups; n++) {
              if(globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone >= globalVar.nTones)
                  break;
              if(NULL == (pNegToneGroup = getCorrespondingNegToneGroup(&globalVar.adslMib.dsPhyBandPlan.toneGroups[n], &globalVar.adslMib.dsNegBandPlan))) {
                  k += globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone - globalVar.adslMib.dsPhyBandPlan.toneGroups[n].startTone + 1;
                  continue;
              }
              k += pNegToneGroup->startTone - globalVar.adslMib.dsPhyBandPlan.toneGroups[n].startTone;
              for(i = pNegToneGroup->startTone; i <= pNegToneGroup->endTone; i++) {
                  globalVar.bitAlloc[i] = (pBuf[k] > 0xF) ? 0 : pBuf[k];
                  k++;
              }
              k += globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone - pNegToneGroup->endTone;
          }
          
          if(AdslMibIsLinkActive(gDslVars)) {
              if(globalVar.bitSwapReqTime[RX_DIRECTION]) {
                  globalVar.adslMib.adslStat.bitswapStat.rcvCnt++;
                  globalVar.bitSwapReqTime[RX_DIRECTION] = 0;
              }
          }
      }
      break;
      case kDsl993p2FeBi:
#ifdef CONFIG_BCM_DSL_GFAST
      case kDsl993p2DOIFeBi:
#endif
      {
        int i,k,n, nTones; uchar *pBuf, *pBitAlloc;
        pBuf =(uchar *)(status->param.dslClearEocMsg.dataPtr); 
        k=0;
#ifdef CONFIG_BCM_DSL_GFAST
        if(XdslMibIsGfastMod(gDslVars)) {
            if(kDsl993p2DOIFeBi == status->param.dslClearEocMsg.msgId)
                pBitAlloc = globalVar.doiBitAlloc + globalVar.nTonesGfast/2;
            else
                pBitAlloc = globalVar.bitAlloc + globalVar.nTonesGfast/2;
            nTones = globalVar.nTonesGfast;
        }
        else
#endif
        {
            pBitAlloc = globalVar.bitAlloc;
            nTones = globalVar.nTones;
        }
        for(n=0;n<globalVar.adslMib.usNegBandPlan32.noOfToneGroups;n++)
        {
            if(globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone >= nTones)
                break;
            for(i=globalVar.adslMib.usNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone;i++)
            {
                pBitAlloc[i] = (pBuf[k] > 0xF)? 0: pBuf[k];
                k++;
            }
        }
      }
        break;
      case kDsl993p2FeBiPhy: /* VDSL Only */
      {
          int i,k,n,nTones;
          uchar *pBuf, *pBitAlloc;
          VdslToneGroup *pNegToneGroup;
          pBuf =(uchar *)(status->param.dslClearEocMsg.dataPtr);
          k=0;
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars)) {
              pBitAlloc = globalVar.bitAlloc + globalVar.nTonesGfast/2;
              nTones = globalVar.nTonesGfast;
          }
          else
#endif
          {
              pBitAlloc = globalVar.bitAlloc;
              nTones = globalVar.nTones;
          }
          for(n = 0; n < globalVar.adslMib.usPhyBandPlan.noOfToneGroups; n++) {
              if(globalVar.adslMib.usPhyBandPlan.toneGroups[n].endTone >= nTones)
                  break;
              if(NULL == (pNegToneGroup = getCorrespondingNegToneGroup(&globalVar.adslMib.usPhyBandPlan.toneGroups[n], &globalVar.adslMib.usNegBandPlan))) {
                  k += globalVar.adslMib.usPhyBandPlan.toneGroups[n].endTone - globalVar.adslMib.usPhyBandPlan.toneGroups[n].startTone + 1;
                  continue;
              }
              k += pNegToneGroup->startTone - globalVar.adslMib.usPhyBandPlan.toneGroups[n].startTone;
              for(i = pNegToneGroup->startTone; i <= pNegToneGroup->endTone; i++) {
                  pBitAlloc[i] = (pBuf[k] > 0xF) ? 0 : pBuf[k];
                  k++;
              }
              k += globalVar.adslMib.usPhyBandPlan.toneGroups[n].endTone - pNegToneGroup->endTone;
          }
          
          if(AdslMibIsLinkActive(gDslVars)) {
              if(globalVar.bitSwapReqTime[TX_DIRECTION]) {
                  globalVar.adslMib.adslStat.bitswapStat.xmtCnt++;
                  globalVar.bitSwapReqTime[TX_DIRECTION] = 0;
              }
          }
      }
          break;
      case kDsl993p2NeGi:
#ifdef CONFIG_BCM_DSL_GFAST
      case kDsl993p2DOINeGi:
#endif
      {
        int k,n,i, nTones; short *GiBuff;
        GiBuff=(short *)(status->param.dslClearEocMsg.dataPtr); 
        k=0;
        if( XdslMibIsVdsl2Mod(gDslVars) ) {
            short *gain;
#ifdef CONFIG_BCM_DSL_GFAST
            if( XdslMibIsGfastMod(gDslVars) ) {
                if(kDsl993p2DOINeGi == status->param.dslClearEocMsg.msgId)
                    gain = globalVar.doiGain;
                else
                    gain = globalVar.gain;
                nTones = globalVar.nTonesGfast;
            }
            else
#endif
            {
                gain = globalVar.gain;
                nTones = globalVar.nTones;
            }
            for(n=0;n<globalVar.adslMib.dsNegBandPlan32.noOfToneGroups;n++) {
                if(globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone >= nTones)
                    break;
                for(i=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone;i++)
                {
                  gain[i]=ADSL_ENDIAN_CONV_SHORT(GiBuff[k])/16;
                  k++;
                }
            }
        }
        else {
            int msgLen = (status->param.dslClearEocMsg.msgType & 0xFFFF) / sizeof(short);
            if(msgLen > nMaxToneAllow)
                msgLen = nMaxToneAllow;
            if (AdslMibTone32_64(gDslVars))
                n=64;
            else n=32;
            for(;n<msgLen;n++)
            {
                globalVar.gain[n]=ADSL_ENDIAN_CONV_SHORT(GiBuff[n])/16;
            }
        }
      }
      break;
      case kDsl993p2NeGiPhy:
      {
          int k,n,i;
          short *GiBuff;
          VdslToneGroup *pNegToneGroup;
          GiBuff=(short *)(status->param.dslClearEocMsg.dataPtr);
          k=0;
          
          if( XdslMibIsVdsl2Mod(gDslVars) ) {
              for(n = 0; n < globalVar.adslMib.dsPhyBandPlan.noOfToneGroups; n++) {
                  if(globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone >= globalVar.nTones)
                      break;
                  if(NULL == (pNegToneGroup = getCorrespondingNegToneGroup(&globalVar.adslMib.dsPhyBandPlan.toneGroups[n], &globalVar.adslMib.dsNegBandPlan))) {
                      k += globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone - globalVar.adslMib.dsPhyBandPlan.toneGroups[n].startTone + 1;
                      continue;
                  }
                  k += pNegToneGroup->startTone - globalVar.adslMib.dsPhyBandPlan.toneGroups[n].startTone;
                  for(i = pNegToneGroup->startTone; i <= pNegToneGroup->endTone; i++) {
                      globalVar.gain[i] = ADSL_ENDIAN_CONV_SHORT(GiBuff[k]) >> 4;
                      k++;
                  }
                  k += globalVar.adslMib.dsPhyBandPlan.toneGroups[n].endTone - pNegToneGroup->endTone;
              }
          }
          else {
              int msgLen = (status->param.dslClearEocMsg.msgType & 0xFFFF) / sizeof(short);
              if(msgLen > nMaxToneAllow)
                  msgLen = nMaxToneAllow;
              if (AdslMibTone32_64(gDslVars))
                  n=64;
              else
                  n=32;
              for(; n < msgLen; n++)
                  globalVar.gain[n] = ADSL_ENDIAN_CONV_SHORT(GiBuff[n]) >> 4;
          }
      }
      break;
      case kDsl993p2BitSwapTones :
      {
        int i,k,n,numTonesInGroup;
#ifdef SUPPORT_DSL_BONDING
        int maxBitswapTones = MAX_BITSWAP_TONES/2;
#else
        int maxBitswapTones = MAX_BITSWAP_TONES;
#endif
        short tone, *pTone =(short *)(status->param.dslClearEocMsg.dataPtr); 
        int msgLen = (status->param.dslClearEocMsg.msgType & 0xFFFF) / sizeof(short);
        
        if(msgLen > maxBitswapTones)
            msgLen = maxBitswapTones;
        for(i=0;i<msgLen;i++)
        {
            k = 0;
            tone = ADSL_ENDIAN_CONV_SHORT(pTone[i]);
            for(n=0;n<globalVar.adslMib.usPhyBandPlan.noOfToneGroups;n++)
            {
                numTonesInGroup = (globalVar.adslMib.usPhyBandPlan.toneGroups[n].endTone -
                                   globalVar.adslMib.usPhyBandPlan.toneGroups[n].startTone + 1);
                if(tone < (k + numTonesInGroup)) {
                    k = globalVar.adslMib.usPhyBandPlan.toneGroups[n].startTone + tone - k; 
                    break;
                }
                else
                    k += numTonesInGroup;
            }
            if(k > globalVar.adslMib.usPhyBandPlan.toneGroups[globalVar.adslMib.usPhyBandPlan.noOfToneGroups-1].endTone)
                __SoftDslPrintf(gDslVars, "*** bitSwapTone is out of range(%d)!!! ***",0, k);
            else
                globalVar.bitSwapTones[i] = k;
        }
      }
      break;
      case kDsl993p2UsGi :
      {
        int i;
        short sub;
        int msgLen = (status->param.dslClearEocMsg.msgType & 0xFFFF) / sizeof(short);
        short *GiBuff=(short *)(status->param.dslClearEocMsg.dataPtr); 
        short *pGain = globalVar.gain;
#ifdef SUPPORT_DSL_BONDING
        int maxBitswapTones = MAX_BITSWAP_TONES/2;
#else
        int maxBitswapTones = MAX_BITSWAP_TONES;
#endif
        if( XdslMibIsVdsl2Mod(gDslVars) ) { /* VDSL Update only Gi changed by BitSwap (S6.9) */
            sub = UtilQ0LinearToQ4dB(512);
            if(msgLen > maxBitswapTones)
                msgLen = maxBitswapTones;
#ifdef CONFIG_BCM_DSL_GFAST
            if(XdslMibIsGfastMod(gDslVars))
                pGain += globalVar.nTonesGfast/2;
#endif
            for(i=0;i<msgLen;i++)
            {
                pGain[globalVar.bitSwapTones[i]] =(short)(2*(UtilQ0LinearToQ4dB(ADSL_ENDIAN_CONV_SHORT(GiBuff[i]))-sub));
            }
        }
        else { /* Adsl */
            sub = UtilQ0LinearToQ4dB(4096); /* ADSL Upstream Gi are S3.12 */ 
            for(i=0;i<msgLen;i++)
            {
                pGain[i] =(short)(2*(UtilQ0LinearToQ4dB(ADSL_ENDIAN_CONV_SHORT(GiBuff[i]))-sub));
            }
        }
      }
      break;
      case kDsl993p2FeGi:
#ifdef CONFIG_BCM_DSL_GFAST
      case kDsl993p2DOIFeGi:
#endif
      {
          int k,n,i,nTones;
          short sub;
          short *GiBuff, *pGain;
          GiBuff=(short *)(status->param.dslClearEocMsg.dataPtr);
          k=0;
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars)) {
              if(kDsl993p2DOIFeGi == status->param.dslClearEocMsg.msgId)
                  pGain = globalVar.doiGain + globalVar.nTonesGfast/2;
              else
                  pGain = globalVar.gain + globalVar.nTonesGfast/2;
              nTones = globalVar.nTonesGfast;
          }
          else
#endif
          {
              pGain = globalVar.gain;
              nTones = globalVar.nTones;
          }
          if( XdslMibIsVdsl2Mod(gDslVars) ) {
#ifdef CONFIG_BCM_DSL_GFAST
              if(XdslMibIsGfastMod(gDslVars)) {
                for(n=0;n<globalVar.adslMib.usNegBandPlan32.noOfToneGroups;n++)
                {
                    if(globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone >= nTones)
                        break;
                    for(i=globalVar.adslMib.usNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone;i++)
                    {
                        pGain[i] = ADSL_ENDIAN_CONV_SHORT(GiBuff[k]) >> 4;
                        k++;
                    }
                }
              }
              else
#endif
              {
                sub=UtilQ0LinearToQ4dB(512); /* Upstream Gi are S6.9 for Vdsl */
                for(n=0;n<globalVar.adslMib.usNegBandPlan32.noOfToneGroups;n++)
                {
                    if(globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone >= nTones)
                        break;
                    for(i=globalVar.adslMib.usNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone;i++)
                    {
                        pGain[i] =(short)(2*(UtilQ0LinearToQ4dB(ADSL_ENDIAN_CONV_SHORT(GiBuff[k]))-sub));
                        k++;
                    }
                }
              }
          }
          else { /* Adsl */
              int msgLen = (status->param.dslClearEocMsg.msgType & 0xFFFF) / sizeof(short);
              if(msgLen > nMaxToneAllow)
                  msgLen = nMaxToneAllow;
              sub=UtilQ0LinearToQ4dB(4096); /* Upstream Gi are S3.12, NOT S6.9 for Adsl */
              for(i=0;i<msgLen;i++)
              {
                  pGain[i] =(short)(2*(UtilQ0LinearToQ4dB(ADSL_ENDIAN_CONV_SHORT(GiBuff[i]))-sub));
              }
          }
      }
      break;
      case kDsl993p2SNRM:
      {
        short* buff;short val;
        buff=(short *)(status->param.dslClearEocMsg.dataPtr);
        val = Q4ToTenth(ADSL_ENDIAN_CONV_SHORT(buff[0]));
        globalVar.adslMib.adslPhys.adslCurrSnrMgn= val;
#ifdef DBG_PRINT_PMD_PARAMS
        __SoftDslPrintf(gDslVars, " Avg SNRM =%d",0,val);
#endif
      }
        break;
      case kDsl993p2SNRMpb:
      {
        short *buff;int n;short val;
        globalVar.dsBpSNRpbForReport=&globalVar.adslMib.dsNegBandPlan;
        globalVar.usBpSNRpbForReport=&globalVar.adslMib.usNegBandPlan;
        buff=(short *)(status->param.dslClearEocMsg.dataPtr);
        for(n=0;n<globalVar.adslMib.dsNegBandPlan.noOfToneGroups;n++)
        {
          val=Q4ToTenth(ADSL_ENDIAN_CONV_SHORT(buff[n]));
          globalVar.adslMib.perbandDataDs[n].adslCurrSnrMgn=val;
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, " PB SNRMPB =%d",0,val);
#endif
        }
      }
        break;
      case kDsl993p2SATNpbRaw:
      {
        short *buff;int n;short val;
        globalVar.dsBpSATNpbForReport=&globalVar.adslMib.dsNegBandPlan;
        globalVar.usBpSATNpbForReport=&globalVar.adslMib.usNegBandPlan;
        buff=(short *)(status->param.dslClearEocMsg.dataPtr);
        for(n=0;n<globalVar.adslMib.dsNegBandPlan.noOfToneGroups;n++)
        {
          val=Q8ToTenth(ADSL_ENDIAN_CONV_SHORT(buff[n]));
          globalVar.adslMib.perbandDataDs[n].adslSignalAttn=val;
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, " PB SATN Attn =%d",0,val);
#endif
        }
      }
        break;
      case kDsl993p2LnAttnRaw:
      {
          short *buff;int n;short val;
        buff=(short *)(status->param.dslClearEocMsg.dataPtr);
         for(n=0;n<globalVar.adslMib.dsPhyBandPlanDiscovery.noOfToneGroups;n++)
        {
          val=Q8ToTenth(ADSL_ENDIAN_CONV_SHORT(buff[n]));
          globalVar.adslMib.perbandDataDs[n].adslCurrAtn=val;
          globalVar.adslMib.perbandDataDs[n].adslSignalAttn=val;
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, " PB LATN Attn =%d",0,val);
#endif
        }
#ifdef CONFIG_BCM_DSL_GFAST
        if(XdslMibIsGfastMod(gDslVars)) {
          globalVar.adslMib.adslPhys.adslCurrAtn = globalVar.adslMib.perbandDataDs[0].adslCurrAtn;
          globalVar.adslMib.adslPhys.adslSignalAttn = globalVar.adslMib.perbandDataDs[0].adslSignalAttn;
        }
#endif
      }
        break;
      case kDsl993p2BpType:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.bandPlanType= val;
        break;
      }
      case kDsl993p2MaxRate:
      {
        uint *pUlong = (uint *) status->param.dslClearEocMsg.dataPtr;
        
        globalVar.adslMib.adslPhys.adslCurrAttainableRate = ActualRate(ADSL_ENDIAN_CONV_INT32(*pUlong)) * 1000;
        globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate = ActualRate(ADSL_ENDIAN_CONV_INT32(*(pUlong+1))) * 1000;
#ifdef USE_TRAINING_ATTNDR
        globalVar.adslMib.xdslPhys.adslTrainAttainableRate = ActualRate(ADSL_ENDIAN_CONV_INT32(*pUlong)) * 1000;
        globalVar.adslMib.xdslPhys.adslShowAttainableRate = 0;
#endif
        XdslMibUpdateAttnEtr(gDslVars, DS_DIRECTION);
        XdslMibUpdateAttnEtr(gDslVars, US_DIRECTION);
      }
        break;
      case kDsl993p2PowerNeTxTot:
      {
        short val = ADSL_ENDIAN_CONV_SHORT(*(short *)status->param.dslClearEocMsg.dataPtr);
        val = Q8ToTenth(val);
        globalVar.adslMib.xdslPhys.adslCurrOutputPwr=val;
      }
      break;
      case kDsl993p2PowerFeTxTot:
      {
        short val = ADSL_ENDIAN_CONV_SHORT(*(short *)status->param.dslClearEocMsg.dataPtr);
#ifdef USE_LOCAL_DS_POWER
        short valQ4 = val >> 4;
#endif
        val = Q8ToTenth(val);
        globalVar.adslMib.xdslAtucPhys.adslCurrOutputPwr=val;
#ifdef USE_LOCAL_DS_POWER
        AdslMibUpdateACTPSD(gDslVars, valQ4, RX_DIRECTION);
#endif
        globalVar.adslMib.adslTrainingState = kAdslTrainingG993ChanAnalysis;
      }
        break;
#ifdef CONFIG_BCM_DSL_GFAST
      case kDsl993p2NeRxPower:
      {
          short val = ADSL_ENDIAN_CONV_SHORT(*(short *)status->param.dslClearEocMsg.dataPtr);
          val = Q8ToTenth(val);
          globalVar.adslMib.xdslPhys.rxPower = val;
      }
        break;
#endif
      case kDslActualPSDUs:
      {
        int val = ADSL_ENDIAN_CONV_INT32(*(int *)status->param.dslClearEocMsg.dataPtr);
        if (val<0)
        val = -Q8ToTenth(-val);
        else 
        val = Q8ToTenth(val);
        globalVar.adslMib.xdslPhys.actualPSD=val;
      }
        break;
      case kDslActualPSDDs:
      {
        int val = ADSL_ENDIAN_CONV_INT32(*(int *)status->param.dslClearEocMsg.dataPtr);
        if (val<0)
          val = -Q8ToTenth(-val);
        else 
          val = Q8ToTenth(val);
        globalVar.adslMib.xdslAtucPhys.actualPSD=val;
      }
        break;
      case kDslSNRModeUs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslPhys.SNRmode= val;
        break;
      }
      case kDslSNRModeDs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslAtucPhys.SNRmode= val;
        break;
      }
      case kDslActualCE:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslPhys.actualCE = val;
        break;
      }
      case kDslUPBOkle:
      {
        unsigned short val = ADSL_ENDIAN_CONV_SHORT(*(unsigned short *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslPhys.UPBOkle= val;
        break;
      }
      case kDslUPBOkleCpe:
      {
        unsigned short val = ADSL_ENDIAN_CONV_SHORT(*(unsigned short *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslPhys.UPBOkleCpe = val;
        break;
      }
      case kDslQLNmtUs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslAtucPhys.QLNMT=val;
          break;
      }
      case kDslQLNmtDs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslPhys.QLNMT=val;
        break;
      }
      case kDslSNRmtUs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslAtucPhys.SNRMT=val;
        break;
      }
      case kDslSNRmtDs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslPhys.SNRMT=val;
        break;
      }
      case kDslHLOGmtUs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslAtucPhys.HLOGMT=val;
        break;
      }
      case kDslHLOGmtDs:
      {
        uint val = ADSL_ENDIAN_CONV_INT32(*(uint *)status->param.dslClearEocMsg.dataPtr);
        globalVar.adslMib.xdslPhys.HLOGMT=val;
        break;
      }
      case kDsl993p2PowerNeTxPb:
      {
        short val;
        short *pBuf =(short *)(status->param.dslClearEocMsg.dataPtr); 
        for(n=0;n<globalVar.adslMib.usPhyBandPlanDiscovery.noOfToneGroups;n++)
        {
          val=Q8ToTenth(ADSL_ENDIAN_CONV_SHORT(pBuf[n]));
          globalVar.adslMib.xdslPhys.perBandCurrOutputPwr[n]=val;
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, " PB NE Cur Output Pwr =%d",0,val);
#endif
        }
      }
        break;
      case kDsl993p2PowerFeTxPb:
      {
        short val;
        short *pBuf =(short *)(status->param.dslClearEocMsg.dataPtr); 
        for(n=0;n<globalVar.adslMib.dsPhyBandPlanDiscovery.noOfToneGroups;n++)
        {
          val=Q8ToTenth(ADSL_ENDIAN_CONV_SHORT(pBuf[n]));
          globalVar.adslMib.xdslAtucPhys.perBandCurrOutputPwr[n]=val;
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, " PB FE Cur Output Pwr =%d",0,val);
#endif
        }
      }
        break;
      case kDsl993p2FeQlnLD:
      {
          int n,i,grpIndex,nTones; uchar *pMsg; short val=0, *pQln = globalVar.quietLineNoise;
          int g=globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus;
          if(0 == g)
              g=1;
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars))
              nTones = globalVar.nTonesGfast;
          else
#endif
          nTones = globalVar.nTones;

          if(globalVar.waitfirstQLNstatusLD==1)
          {
              for(i = 0; i < nTones; i++)
                  pQln[i]=(-160)<<4;
              globalVar.waitfirstQLNstatusLD=0;
          }
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr);
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars))
              pQln += globalVar.nTonesGfast/2;
#endif

          for(n=0;n<globalVar.adslMib.usNegBandPlanDiscovery32.noOfToneGroups;n++)
          {
              grpIndex=-1;
              if(globalVar.adslMib.usNegBandPlanDiscovery32.toneGroups[n].endTone >= nTones)
                  break;
              for(i=globalVar.adslMib.usNegBandPlanDiscovery32.toneGroups[n].startTone;i<=globalVar.adslMib.usNegBandPlanDiscovery32.toneGroups[n].endTone;i++)
              {
                  if(i/g > grpIndex)
                  {
                      grpIndex=i/g;
                      if(pMsg[grpIndex]!=255)
                          val = -(((short) pMsg[grpIndex]) << 3) - 23*16;
                      else 
                          val = -160*16;
                  }
                  pQln[i]=val;
              }
          }
      }
        break;
      case kDsl993p2FeHlogLD:
      {
          int n,i,grpIndex,nTones; uchar *pMsg; short val=0, *pChanCharLog = globalVar.chanCharLog;
          int g=globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus;
          if(0 == g)
              g=1;
          if(globalVar.waitfirstHLOGstatusLD==1)
          {
              for(i=0;i<globalVar.nTones;i++)
                  pChanCharLog[i]=(-96)<<4;
              globalVar.waitfirstHLOGstatusLD=0;
          }
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr);
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars)) {
              pChanCharLog += globalVar.nTonesGfast/2;
              nTones = globalVar.nTonesGfast;
          }
          else
#endif
          nTones = globalVar.nTones;

          for(n=0;n<globalVar.adslMib.usNegBandPlanDiscovery32.noOfToneGroups;n++)
          {
              grpIndex=-1;
              if(globalVar.adslMib.usNegBandPlanDiscovery32.toneGroups[n].endTone >= nTones)
                  break;
              for(i=globalVar.adslMib.usNegBandPlanDiscovery32.toneGroups[n].startTone;i<=globalVar.adslMib.usNegBandPlanDiscovery32.toneGroups[n].endTone;i++)
              {
                  if(i/g>grpIndex)
                  {
                      grpIndex=i/g;
                      val = ((int) (pMsg[grpIndex*2] & 0x3) << 8) + (pMsg[grpIndex*2+1]);
                          val = -(val*16/10) + 6*16;
                  }
                  pChanCharLog[i]=val;
              }
          }
      }
        break;
      case kDsl993p2NeHlogLD:
      {
          int n,i,grpIndex;uchar *pMsg;short val=0;
          int g=globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds;
          globalVar.dsBpHlogForReport=&globalVar.adslMib.dsNegBandPlanDiscovery;
          if(0==g)
              g=1;
          if(globalVar.waitfirstHLOGstatusLD==1)
          {
              for(i=0;i<globalVar.nTones;i++)
                  globalVar.chanCharLog[i]=(-96)<<4;
              globalVar.waitfirstHLOGstatusLD=0;
          }
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr); 
          for(n=0;n<globalVar.adslMib.dsNegBandPlanDiscovery32.noOfToneGroups;n++)
          {
              grpIndex=-1;
              if(globalVar.adslMib.dsNegBandPlanDiscovery32.toneGroups[n].endTone < globalVar.nTones)
                  for(i=globalVar.adslMib.dsNegBandPlanDiscovery32.toneGroups[n].startTone;i<=globalVar.adslMib.dsNegBandPlanDiscovery32.toneGroups[n].endTone;i++)
                  {
                      if(i/g > grpIndex)
                      {
                          grpIndex=i/g;
                          val = ((int) (pMsg[grpIndex*2] & 0x3) << 8) + (pMsg[grpIndex*2+1]);
                              val = -(val*16/10) + 6*16;
                      }
                      globalVar.chanCharLog[i]=val;
                  }
          }
      }
        break;
      case kDsl993p2FePbLatnLD:
      {
          int n,i;uchar *pMsg;
          ulong dataLen;
          bandPlanDescriptor32    usNegBandPlanPresentation;
          uchar  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanUSNegDiscoveryPresentation};
          dataLen = sizeof(bandPlanDescriptor32);
          AdslMibGetObjectValue(gDslVars,oidStr, sizeof(oidStr), (char *)(&usNegBandPlanPresentation), &dataLen);
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr); 
          for(n=0,i=0;n<usNegBandPlanPresentation.noOfToneGroups;n++)
          {
              if (usNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF)
                  globalVar.adslMib.perbandDataUs[i++].adslCurrAtn=ReadCnt16(pMsg);
              pMsg+=2;
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, "US PB LATN Attn[%d] =%d",0,i-1,globalVar.adslMib.perbandDataUs[i-1].adslCurrAtn);
#endif
          }
      }
        break;
      case kDsl993p2NePbLatnLD:
      {
          int n,i;uchar *pMsg;
          ulong dataLen;
          bandPlanDescriptor32    dsNegBandPlanPresentation;
          uchar  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanDSNegDiscoveryPresentation};
          dataLen = sizeof(bandPlanDescriptor32);
          AdslMibGetObjectValue(gDslVars,oidStr, sizeof(oidStr), (char *)(&dsNegBandPlanPresentation), &dataLen);
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr);
          for(n=0,i=0;n<dsNegBandPlanPresentation.noOfToneGroups;n++)
          {
              if (dsNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF)
              {
                  globalVar.adslMib.perbandDataDs[i].adslCurrAtn=ReadCnt16(pMsg);
                  globalVar.adslMib.vdslDiag[i].loopAttn=globalVar.adslMib.perbandDataDs[i].adslCurrAtn;
                  i++;
#ifdef DBG_PRINT_PMD_PARAMS
              __SoftDslPrintf(gDslVars, "DS PB LATN Attn[%d] =%d",0,i-1,globalVar.adslMib.perbandDataDs[i-1].adslCurrAtn);
#endif
              }
              pMsg+=2;
          }
      }
        break;
      case kDsl993p2FePbSatnLD:
      {
          int n,i;uchar *pMsg;
          ulong dataLen;
          bandPlanDescriptor32    usNegBandPlanPresentation;
          uchar  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanUSNegDiscoveryPresentation};
          globalVar.usBpSATNpbForReport=&globalVar.adslMib.usNegBandPlanDiscovery;
          dataLen = sizeof(bandPlanDescriptor32);
          AdslMibGetObjectValue(gDslVars,oidStr, sizeof(oidStr), (char *)(&usNegBandPlanPresentation), &dataLen);  
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr);
          for(n=0,i=0;n<usNegBandPlanPresentation.noOfToneGroups;n++)
          {
              if (usNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF)
                  globalVar.adslMib.perbandDataUs[i++].adslSignalAttn=ReadCnt16(pMsg);
#ifdef DBG_PRINT_PMD_PARAMS
              __SoftDslPrintf(gDslVars, "US PB SATN Attn[%d] =%d",0,i-1,globalVar.adslMib.perbandDataUs[i-1].adslSignalAttn);
#endif
              pMsg+=2;
          }
      }
        break;
      case kDsl993p2NePbSatnLD:
      {
          int n,i;uchar *pMsg;
          ulong dataLen;
          bandPlanDescriptor32    dsNegBandPlanPresentation;
          uchar  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanDSNegDiscoveryPresentation};
          globalVar.dsBpSATNpbForReport=&globalVar.adslMib.dsNegBandPlanDiscovery;
          dataLen = sizeof(bandPlanDescriptor32);
          AdslMibGetObjectValue(gDslVars,oidStr, sizeof(oidStr), (char *)(&dsNegBandPlanPresentation), &dataLen); 
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr);
          for(n=0,i=0;n<dsNegBandPlanPresentation.noOfToneGroups;n++)
          {
              if (dsNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF)
              {
              globalVar.adslMib.perbandDataDs[i].adslSignalAttn=ReadCnt16(pMsg);
              globalVar.adslMib.vdslDiag[i].signalAttn=globalVar.adslMib.perbandDataDs[i].adslSignalAttn;
              i++;
#ifdef DBG_PRINT_PMD_PARAMS
              __SoftDslPrintf(gDslVars, "DS PB SATN Attn[%d] =%d",0,i-1,globalVar.adslMib.perbandDataDs[i-1].adslSignalAttn);
#endif
              }

              pMsg+=2;
          }
      }
          break;
      case kDsl993p2FePbSnrLD:
      {
          int n,i;uchar *pMsg;
          ulong dataLen;
          bandPlanDescriptor32    usNegBandPlanPresentation;
          uchar  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanUSNegDiscoveryPresentation};
          globalVar.usBpSNRpbForReport=&globalVar.adslMib.usNegBandPlanDiscovery;
          dataLen = sizeof(bandPlanDescriptor32);
          AdslMibGetObjectValue(gDslVars,oidStr, sizeof(oidStr), (char *)(&usNegBandPlanPresentation), &dataLen);
          pMsg=(uchar *)(status->param.dslClearEocMsg.dataPtr);
          globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn = ReadCnt16(pMsg);
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, " Avg SNRM =%d",0,globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn);
#endif
          pMsg+=2;

          for(n=0,i=0;n<usNegBandPlanPresentation.noOfToneGroups;n++)
          {
              if (usNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF)
                  globalVar.adslMib.perbandDataUs[i++].adslCurrSnrMgn=ReadCnt16(pMsg);
#ifdef DBG_PRINT_PMD_PARAMS
              __SoftDslPrintf(gDslVars, "US PB SNRM [%d] =%d",0,i-1,globalVar.adslMib.perbandDataUs[i-1].adslCurrSnrMgn);
#endif
              pMsg+=2;
          }
      }
        break;
      case kDsl993p2NePbSnrLD:
      {
          int n,i;uchar *pMsg;
          ulong dataLen;
          bandPlanDescriptor32    dsNegBandPlanPresentation;
          uchar  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanDSNegDiscoveryPresentation};
          dataLen = sizeof(bandPlanDescriptor32);
          AdslMibGetObjectValue(gDslVars,oidStr, sizeof(oidStr), (char *)(&dsNegBandPlanPresentation), &dataLen);
          globalVar.dsBpSNRpbForReport=&globalVar.adslMib.dsNegBandPlanDiscovery;
          globalVar.dsBpSNRForReport=&globalVar.adslMib.dsNegBandPlanDiscovery;
          pMsg=(uchar *)(status->param.dslClearEocMsg.dataPtr);
          globalVar.adslMib.adslPhys.adslCurrSnrMgn = ReadCnt16(pMsg);
#ifdef DBG_PRINT_PMD_PARAMS
          __SoftDslPrintf(gDslVars, " Avg SNRM =%d",0,globalVar.adslMib.adslPhys.adslCurrSnrMgn);
#endif
          pMsg+=2;
          for(n=0,i=0;n<dsNegBandPlanPresentation.noOfToneGroups;n++)
          {
              if (dsNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF)
              {
                  globalVar.adslMib.perbandDataDs[i].adslCurrSnrMgn=ReadCnt16(pMsg);
                  globalVar.adslMib.vdslDiag[i].snrMargin=globalVar.adslMib.perbandDataDs[i].adslCurrSnrMgn;
                  i++;
#ifdef DBG_PRINT_PMD_PARAMS
              __SoftDslPrintf(gDslVars, "DS PB SNRM [%d] =%d",0,i-1,globalVar.adslMib.perbandDataDs[i-1].adslCurrSnrMgn);
#endif
              }

              pMsg+=2;
          }
      }
        break;
      case kDsl993p2FeAttnLD:
      {
          uchar *pMsg;
          pMsg=(uchar *)(status->param.dslClearEocMsg.dataPtr);
          globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate=ReadCnt32(pMsg);
      }
        break;
      case kDsl993p2NeAttnLD:
      {
          uchar *pMsg;
          pMsg=(uchar *)(status->param.dslClearEocMsg.dataPtr);
          globalVar.adslMib.adslPhys.adslCurrAttainableRate=ReadCnt32(pMsg);
      }
        break;
      case kDsl993p2FeTxPwrLD:
      {
        uchar *pMsg;
		int   n;
        pMsg=(uchar *)(status->param.dslClearEocMsg.dataPtr);
		n = ReadCnt16(pMsg);
		n = (n << (32 - 10)) >> (32 - 10);
        globalVar.adslMib.adslAtucPhys.adslCurrOutputPwr = n;
      }
        break;
      case kDsl993p2NeTxPwrLD:
      {
        uchar *pMsg;
		int   n;
        pMsg=(uchar *)(status->param.dslClearEocMsg.dataPtr);
		n = ReadCnt16(pMsg);
		n = (n << (32 - 10)) >> (32 - 10);
        globalVar.adslMib.adslPhys.adslCurrOutputPwr = n;
      }
        break;
      case kDsl993p2FeSnrLD:
      {
          int n,i,grpIndex,nTones; uchar *pMsg; short val=0, *pSnr = globalVar.snr;
          int g=globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus;
          if(0==g)
              g=1;
          if(globalVar.waitfirstSNRstatusLD==1)
          {
              BlockByteClear(nMaxToneAllow, (void*)pSnr);
              globalVar.waitfirstSNRstatusLD=0;
          }
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr);
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars)) {
              pSnr += globalVar.nTonesGfast/2;
              nTones = globalVar.nTonesGfast;
          }
          else
#endif
          nTones = globalVar.nTones;

          for(n=0;n<globalVar.adslMib.usNegBandPlan32.noOfToneGroups;n++)
          {
              grpIndex=-1;
              if(globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone >= nTones)
                  break;
              for(i=globalVar.adslMib.usNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone;i++)
              {
                  if(i/g>grpIndex)
                  {
                      grpIndex=i/g;
                      if(pMsg[grpIndex]!=255)
                          val= (((short) pMsg[grpIndex]) << 3) - 32*16;
                      else
                          val = 0;
                  }
                  pSnr[i]=val;
              }
          }
      }
        break;
      case kDsl993p2NeSnrLD:
      {
          int n,i,grpIndex;uchar *pMsg;short val=0;
          int g=globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds;
          if(0==g)
              g=1;
          if(globalVar.waitfirstSNRstatusLD==1)
          {
              BlockByteClear(nMaxToneAllow, (void*)globalVar.snr);
              globalVar.waitfirstSNRstatusLD=0;
          }
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr); 
          for(n=0;n<globalVar.adslMib.dsNegBandPlan32.noOfToneGroups;n++)
          {
              grpIndex=-1;
              if(globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone < globalVar.nTones)
                  for(i=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone;i++)
                  {
                      if(i/g>grpIndex)
                      {
                          grpIndex=i/g;
                          if(pMsg[grpIndex]!=255)
                              val= (((short) pMsg[grpIndex]) << 3) - 32*16;
                          else 
                              val = 0;
                      }
                      globalVar.snr[i]=val;
                  }
          }
      }
        break;
      case kDsl993p2NeHlinLD:
      {
          int n,i,k=0,grpIndex;uchar *pMsg;
          int g=globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds;
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr); 
          for(n=0;n<globalVar.adslMib.dsNegBandPlan32.noOfToneGroups;n++)
          {
              grpIndex=-1;
              if(globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone < globalVar.nTones)
                  for(i=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.dsNegBandPlan32.toneGroups[n].endTone;i++)
                  {
                      if(i/g>grpIndex)
                      {
                          grpIndex=i/g;
                          if(k==0 && i>globalVar.adslMib.dsNegBandPlan32.toneGroups[n].startTone+10)
                          {
                              globalVar.adslMib.adslPhys.adslHlinScaleFactor = ReadCnt16(pMsg+6*grpIndex);
                              globalVar.adslMib.adslDiag.hlinScaleFactor = globalVar.adslMib.adslPhys.adslHlinScaleFactor;
                              k=1;
                          }
                      }
                      globalVar.chanCharLin[i].x = ReadCnt16(pMsg+6*grpIndex+2);
                      globalVar.chanCharLin[i].y = ReadCnt16(pMsg+6*grpIndex+4);
                  }
          }
      }
        break;
      case kDsl993p2FeHlinLD:
      {
          int n,i,k=0,grpIndex,nTones; uchar *pMsg;
          ComplexShort *pChanCharLin;
          int g=globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus;
          pMsg=(uchar*)(status->param.dslClearEocMsg.dataPtr);
#ifdef CONFIG_BCM_DSL_GFAST
          if(XdslMibIsGfastMod(gDslVars)) {
              pChanCharLin = globalVar.chanCharLin + globalVar.nTonesGfast/2;
              nTones = globalVar.nTonesGfast;
          }
          else
#endif
          {
              pChanCharLin = globalVar.chanCharLin;
              nTones = globalVar.nTones;
          }
          for(n=0;n<globalVar.adslMib.usNegBandPlan32.noOfToneGroups;n++)
          {
              grpIndex=-1;
              if(globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone >= nTones)
                  break;
              for(i=globalVar.adslMib.usNegBandPlan32.toneGroups[n].startTone;i<=globalVar.adslMib.usNegBandPlan32.toneGroups[n].endTone;i++)
              {
                  if(i/g>grpIndex)
                  {
                      grpIndex=i/g;
                      if(k==0 && i>globalVar.adslMib.usNegBandPlan32.toneGroups[n].startTone+10)
                      {
                          globalVar.adslMib.adslAtucPhys.adslHlinScaleFactor = ReadCnt16(pMsg+6*grpIndex);
                          k=1;
                      }
                  }
                  pChanCharLin[i].x = ReadCnt16(pMsg+6*grpIndex+2);
                  pChanCharLin[i].y = ReadCnt16(pMsg+6*grpIndex+4);
              }
          }
      }
        break;
#if defined(SUPPORT_VECTORING)
#ifndef G993P5_OVERHEAD_MESSAGING
#error This driver requires to be compiled with a PHY using G993P5_OVERHEAD_MESSAGING when vectoring is enabled
#endif
      case kVectoringMacAddress:
      {
          BlockByteMove(6,(char *)status->param.dslClearEocMsg.dataPtr,(char *)&globalVar.adslMib.vectData.macAddress.macAddress[0]);
          globalVar.adslMib.vectData.macAddress.addressType=0;
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
          __SoftDslPrintf(gDslVars, "DRV MIB: kVectoringMacAddress: %x:%x:%x:%x:%x:%x, addressType=%d",0,
              globalVar.adslMib.vectData.macAddress.macAddress[0], globalVar.adslMib.vectData.macAddress.macAddress[1],
              globalVar.adslMib.vectData.macAddress.macAddress[2], globalVar.adslMib.vectData.macAddress.macAddress[3],
              globalVar.adslMib.vectData.macAddress.macAddress[4], globalVar.adslMib.vectData.macAddress.macAddress[5],
              globalVar.adslMib.vectData.macAddress.addressType);
      }
      break;
      case kDslVectoringPilotSequence:
      {
           int msgLen = status->param.dslClearEocMsg.msgType & 0xFFFF;
#ifdef SUPPORT_EXT_DSL_BONDING_MASTER
            if(0 == gLineId(gDslVars))
#endif
          __SoftDslPrintf(gDslVars, "DRV MIB: kDslVectoringPilotSequence: 0x%x   length=%d  L=%d",0,*((unsigned int*)status->param.dslClearEocMsg.dataPtr), msgLen, globalVar.vectPhyData.pilotSequence.pilotSeqLengthInBytes);
          if( msgLen > sizeof(globalVar.vectPhyData.pilotSequence.bitsPattern)) {
              msgLen = sizeof(globalVar.vectPhyData.pilotSequence.bitsPattern);
              __SoftDslPrintf(gDslVars, "DRV MIB: Errored kDslVectoringPilotSequence: msgLen adjust to =%d", 0, msgLen);
          }
          BlockByteMove(msgLen, (char *)status->param.dslClearEocMsg.dataPtr, (char *)&globalVar.vectPhyData.pilotSequence.bitsPattern[0]);
      break;
      }
#endif /* defined(SUPPORT_VECTORING) */
#endif /* CONFIG_VDSL_SUPPORTED */
      }
      break; /* kDslReceivedEocCommand */
    default:
      break;
  }
  return retCode;
}

Public void AdslMibClearBertResults(void *gDslVars)
{
    globalVar.adslMib.adslBertRes.bertTotalBits = 0;
    globalVar.adslMib.adslBertRes.bertErrBits = 0;
}

Public void AdslMibBertStartEx(void *gDslVars, uint bertSec)
{
    AdslMibClearBertResults(gDslVars);

    globalVar.adslMib.adslBertStatus.bertTotalBits.cntHi = 0;
    globalVar.adslMib.adslBertStatus.bertTotalBits.cntLo = 0;
    globalVar.adslMib.adslBertStatus.bertErrBits.cntHi = 0;
    globalVar.adslMib.adslBertStatus.bertErrBits.cntLo = 0;

    globalVar.adslMib.adslBertStatus.bertSecTotal = bertSec;
    globalVar.adslMib.adslBertStatus.bertSecElapsed = 1;
    globalVar.adslMib.adslBertStatus.bertSecCur = (uint)-1;
}

Public void AdslMibBertStopEx(void *gDslVars)
{
    globalVar.adslMib.adslBertStatus.bertSecCur = 0;
}

Private void AdslMibAdd64 (cnt64 *pCnt64, uint num)
{
    uint   n;

    n = pCnt64->cntLo + num;
    if ((n < pCnt64->cntLo) || (n < num))
        pCnt64->cntHi++;

    pCnt64->cntLo = n;
}

Public uint AdslMibBertContinueEx(void *gDslVars, uint totalBits, uint errBits)
{
    if (0 == globalVar.adslMib.adslBertStatus.bertSecCur)
        return 0;

    AdslMibAdd64(&globalVar.adslMib.adslBertStatus.bertTotalBits, totalBits);
    AdslMibAdd64(&globalVar.adslMib.adslBertStatus.bertErrBits, errBits);

    globalVar.adslMib.adslBertStatus.bertSecElapsed += globalVar.adslMib.adslBertStatus.bertSecCur;
    if (globalVar.adslMib.adslBertStatus.bertSecElapsed >= globalVar.adslMib.adslBertStatus.bertSecTotal) {
        globalVar.adslMib.adslBertStatus.bertSecCur = 0;
        return 0;
    }
        
    if (AdslMibIsLinkActive(gDslVars)) {
        uint    nBits, nSec, nSecLeft;
        
        nBits = AdslMibGetGetChannelRate(gDslVars, kAdslRcvDir, kAdslIntlChannel);
        nBits += AdslMibGetGetChannelRate(gDslVars, kAdslRcvDir, kAdslFastChannel);
        if(nBits <= 0x05555555)  /* Avoid overflow */
          nBits = (nBits * 48) / 53;
        else
          nBits = (nBits / 53) * 48;
        nSec = 0xFFFFFFF0 / nBits;
        nSecLeft = globalVar.adslMib.adslBertStatus.bertSecTotal - globalVar.adslMib.adslBertStatus.bertSecElapsed;
        if (nSec > nSecLeft)
            nSec = nSecLeft;
        if (nSec > 20)
            nSec = 20;
        
        globalVar.adslMib.adslBertStatus.bertSecCur = nSec;
        return nSec * nBits;
    }
    else {
        globalVar.adslMib.adslBertStatus.bertSecCur = 1;
        return 8000000;
    }
}

Public void AdslMibSetLPR(void *gDslVars)
{
    globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLPR;
    globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusNoDefect;
}

Public void AdslMibSetShowtimeMargin(void *gDslVars, int showtimeMargin)
{
    globalVar.showtimeMarginThld = showtimeMargin;
}

Public void AdslMibSetNumTones (void *gDslVars, int numTones)
{
    globalVar.nTones=numTones;
}

Public void  *AdslMibGetData (void *gDslVars, int dataId, void *pAdslMibData)
{
    switch (dataId) {
        case kAdslMibDataAll:
            if (NULL != pAdslMibData)
                AdslMibByteMove (sizeof(adslMibVarsStruct), &globalVar, pAdslMibData);
            else
                pAdslMibData = &globalVar;
            break;

        default:
            pAdslMibData = NULL;
    }   
    return pAdslMibData;
}

#endif /* _M_IX86 */
