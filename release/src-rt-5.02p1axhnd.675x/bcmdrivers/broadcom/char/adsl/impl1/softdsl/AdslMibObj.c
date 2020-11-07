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
 * AdslMibObj.c -- Adsl MIB object access functions
 *
 * Description:
 *	This file contains functions for access to ADSL MIB (RFC 2662) data
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.15 $
 *
 * $Id: AdslMibObj.c,v 1.15 2007/09/04 07:21:15 tonytran Exp $
 *
 * $Log: AdslMibObj.c,v $
 * Revision 1.15  2007/09/04 07:21:15  tonytran
 * PR31097: 1_28_rc8
 *
 * Revision 1.14  2007/01/11 09:14:04  tonytran
 * Fixed the set phy cfg and bitswap counter problems; Removed FAST_DEC_DISABLE from phy cfg
 *
 * Revision 1.13  2006/09/13 22:07:06  dadityan
 * Added Mib Oid for Adsl Phy Cfg
 *
 * Revision 1.8  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.7  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.6  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.5  2002/10/31 20:27:13  ilyas
 * Merged with the latest changes for VxWorks/Linux driver
 *
 * Revision 1.4  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.3  2002/01/03 06:03:36  ilyas
 * Handle byte moves tha are not multiple of 2
 *
 * Revision 1.2  2002/01/02 19:13:19  liang
 * Change memcpy to BlockByteMove.
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#include "SoftDsl.gh"

#include "AdslMib.h"
#include "AdslMibOid.h"
#include "BlockUtil.h"

#include "SoftDsl.h"
#include "../AdslCore.h"

#define	globalVar	gAdslMibVars
#undef  G992P3_DBG_PRINT
#ifndef _M_IX86
extern adslCfgProfile	adslCoreCfgProfile[];
#endif
#define RestrictValue(n,l,r)        ((n) < (l) ? (l) : (n) > (r) ? (r) : (n))

#if defined(SUPPORT_VECTORING)
#if ((MIB_PILOT_SEQUENCE_LEN) != (MIB_PILOT_SEQUENCE_LEN))
#error Inconsistent PILOT_SEQUENCE_LEN definition
#endif
extern unsigned int BcmXdslGetVectExtraSkbCnt(void);
#endif /* SUPPORT_VECTORING */

/*
**
**		ATM TC (Transmission Convergence aka PHY) MIB data oblects
**
*/

extern Boolean gSharedMemAllocFromUserContext;

Private void * MibGetAtmTcObjPtr(void *gDslVars, uchar *objId, int objIdLen, long *objLen)
{
	atmPhyDataEntrty	*pAtmData;
	void				*pObj = NULL;

	if ((objIdLen < 2) || (objId[0] != kOidAtmMibObjects) || (objId[1] != kOidAtmTcTable))
		return NULL;

	pAtmData = (kAdslIntlChannel == AdslMibGetActiveChannel(gDslVars) ?
				&globalVar.adslMib.adslChanIntlAtmPhyData : &globalVar.adslMib.adslChanFastAtmPhyData);
	if (objIdLen == 2) {
		*objLen = sizeof(atmPhyDataEntrty);
		return pAtmData;
	}
	if (objId[2] != kOidAtmTcEntry)
		return NULL;
		
	if (objIdLen == 3) {
		*objLen = sizeof(atmPhyDataEntrty);
		return pAtmData;
	}

	if (objId[3] > kOidAtmAlarmState)
		return NULL;

	if (kOidAtmOcdEvents == objId[3]) {
		*objLen = sizeof(pAtmData->atmInterfaceOCDEvents);
		pObj = &pAtmData->atmInterfaceOCDEvents;
	}
	else if (kOidAtmAlarmState == objId[3]) {
		*objLen = sizeof(pAtmData->atmInterfaceTCAlarmState);
		pObj = &pAtmData->atmInterfaceTCAlarmState;
	}

	return pObj;
}


/*
**
**		ADSL Line MIB data oblects
**
*/

Private void * MibGetAdslLineTableObjPtr (void *gDslVars, uchar *objId, int objIdLen, long *objLen)
{
	void	*pObj = NULL;	

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslLine);
		return &globalVar.adslMib.adslLine;
	}
	if (objId[1] != kOidAdslLineEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslLine);
		return &globalVar.adslMib.adslLine;
	}

	switch (objId[2]) {
		case kOidAdslLineCoding:
			*objLen = sizeof(globalVar.adslMib.adslLine.adslLineCoding);
			pObj = &globalVar.adslMib.adslLine.adslLineCoding;
			break;
		case kOidAdslLineType:
			*objLen = sizeof(globalVar.adslMib.adslLine.adslLineType);
			pObj = &globalVar.adslMib.adslLine.adslLineType;
			break;
		case kOidAdslLineSpecific:
		case kOidAdslLineConfProfile:
		case kOidAdslLineAlarmConfProfile:
		default:
			pObj = NULL;
			break;
	}

	return pObj;
}

Private void * MibGetAdslPhysTableObjPtr (void *gDslVars, uchar *objId, int objIdLen, long *objLen)
{
	void	*pObj = NULL;	

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPhys);
		return &globalVar.adslMib.adslPhys;
	}
	if (objId[0] != kOidAdslPhysEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPhys);
		return &globalVar.adslMib.adslPhys;
	}

	switch (objId[1]) {
		case kOidAdslPhysInvSerialNumber:
		case kOidAdslPhysInvVendorID:
		case kOidAdslPhysInvVersionNumber:
			pObj = NULL;
			break;
			
		case kOidAdslPhysCurrSnrMgn:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrSnrMgn);
			pObj = &globalVar.adslMib.adslPhys.adslCurrSnrMgn;
			break;
		case kOidAdslPhysCurrAtn:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrAtn);
			pObj = &globalVar.adslMib.adslPhys.adslCurrAtn;
			break;
		case kOidAdslPhysCurrStatus:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrStatus);
			pObj = &globalVar.adslMib.adslPhys.adslCurrStatus;
			break;
		case kOidAdslPhysCurrOutputPwr:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrOutputPwr);
			pObj = &globalVar.adslMib.adslPhys.adslCurrOutputPwr;
			break;
		case kOidAdslPhysCurrAttainableRate:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrAttainableRate);
			pObj = &globalVar.adslMib.adslPhys.adslCurrAttainableRate;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslPerfTableCounterPtr (adslPerfCounters *pPerf, uchar cntId, long *objLen)
{
	void	*pObj = NULL;

	switch (cntId) {
		case  kOidAdslPerfLofs:
			*objLen = sizeof(pPerf->adslLofs);
			pObj = &pPerf->adslLofs;
			break;
		case  kOidAdslPerfLoss:
			*objLen = sizeof(pPerf->adslLoss);
			pObj = &pPerf->adslLoss;
			break;
		case  kOidAdslPerfLprs:
			*objLen = sizeof(pPerf->adslLprs);
			pObj = &pPerf->adslLprs;
			break;
		case  kOidAdslPerfESs:
			*objLen = sizeof(pPerf->adslESs);
			pObj = &pPerf->adslESs;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslPerfTableObjPtr (void *gDslVars, uchar *objId, int objIdLen, long *objLen)
{
	void	*pObj = NULL;

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPerfData);
		return &globalVar.adslMib.adslPerfData;
	}
	if (objId[0] != kOidAdslPerfDataEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPerfData);
		return &globalVar.adslMib.adslPerfData;
	}

	switch (objId[1]) {
		case  kOidAdslPerfLofs:
		case  kOidAdslPerfLoss:
		case  kOidAdslPerfLprs:
		case  kOidAdslPerfESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfTotal, objId[1], objLen);
			break;
		case  kOidAdslPerfValidIntervals:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfValidIntervals);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfValidIntervals;
			break;	
		case  kOidAdslPerfInvalidIntervals:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfInvalidIntervals);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfInvalidIntervals;
			break;	
		case  kOidAdslPerfCurr15MinTimeElapsed: 	
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed;
			break;
		case  kOidAdslPerfCurr15MinLofs:
		case  kOidAdslPerfCurr15MinLoss:
		case  kOidAdslPerfCurr15MinLprs:
		case  kOidAdslPerfCurr15MinESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfCurr15Min, objId[1]-kOidAdslPerfCurr15MinLofs+1, objLen);
			break;	
		case  kOidAdslPerfCurr1DayTimeElapsed:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed;
			break;
		case  kOidAdslPerfCurr1DayLofs:
		case  kOidAdslPerfCurr1DayLoss:
		case  kOidAdslPerfCurr1DayLprs:
		case  kOidAdslPerfCurr1DayESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfCurr1Day, objId[1]-kOidAdslPerfCurr1DayLofs+1, objLen);
			break;	
		case  kOidAdslPerfPrev1DayMoniSecs:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslAturPerfPrev1DayMoniSecs);
			pObj = &globalVar.adslMib.adslPerfData.adslAturPerfPrev1DayMoniSecs;
			break;
		case  kOidAdslPerfPrev1DayLofs:
		case  kOidAdslPerfPrev1DayLoss:
		case  kOidAdslPerfPrev1DayLprs:
		case  kOidAdslPerfPrev1DayESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfPrev1Day, objId[1]-kOidAdslPerfPrev1DayLofs+1, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslPerfIntervalObjPtr (void *gDslVars, uchar *objId, int objIdLen, long *objLen)
{
	void	*pObj = NULL;	
	uchar	ind;

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPerfIntervals);
		return &globalVar.adslMib.adslPerfIntervals;
	}
	if ((objId[0] != kOidAdslPerfIntervalEntry) || (objIdLen < 4))
		return NULL;
	ind = objId[3];
	if ((ind == 0) || (ind > globalVar.adslMib.adslPerfData.adslPerfValidIntervals))
		return NULL;

	switch (objId[1]) {
		case kOidAdslIntervalNumber:
			*objLen = sizeof(int);
			globalVar.scratchData = ind;
			pObj = &globalVar.scratchData;
			break;
		case kOidAdslIntervalLofs:
		case kOidAdslIntervalLoss:
		case kOidAdslIntervalLprs:
		case kOidAdslIntervalESs:
			pObj = MibGetAdslPerfTableCounterPtr(&globalVar.adslMib.adslPerfIntervals[ind-1], objId[1]-kOidAdslIntervalLofs+1, objLen);
			break;

		case kOidAdslIntervalValidData:
			*objLen = sizeof(int);
			globalVar.scratchData = true;
			pObj = &globalVar.scratchData;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslObjPtr (void *gDslVars, uchar *objId, int objIdLen, long *objLen)
{
	void	*pObj = NULL;	

	if ((objIdLen < 3) || (objId[0] != kOidAdslLine) || (objId[1] != kOidAdslMibObjects))
		return NULL;

	switch (objId[2]) {
		case kOidAdslLineTable:
			pObj = MibGetAdslLineTableObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturPhysTable:
			pObj = MibGetAdslPhysTableObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAtucPhysTable:
			pObj	= &globalVar.adslMib.adslAtucPhys;
#if defined(CONFIG_VDSL_SUPPORTED)
			*objLen = sizeof(globalVar.adslMib.xdslAtucPhys);
#else
			*objLen = sizeof(globalVar.adslMib.adslAtucPhys);
#endif
			break;
		case kOidAdslAturPerfDataTable:
			pObj = MibGetAdslPerfTableObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturPerfIntervalTable:
			pObj = MibGetAdslPerfIntervalObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}

	return pObj;
}

/*
**
**		ADSL Channel MIB data oblects
**
*/

Private void * MibGetAdslChanTableObjPtr(void *gDslVars, uchar chId, uchar *objId, int objIdLen, long *objLen)
{
	void			*pObj = NULL;
	adslChanEntry	*pChan;

	pChan = (kAdslIntlChannel == chId ? &globalVar.adslMib.adslChanIntl: &globalVar.adslMib.adslChanFast);

	if (0 == objIdLen) {
		*objLen = sizeof(adslChanEntry);
		return pChan;
	}
	if (objId[0] != kOidAdslChanEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(adslChanEntry);
		return pChan;
	}

	switch (objId[1]) {
		case kOidAdslChanInterleaveDelay:
			*objLen = sizeof(pChan->adslChanIntlDelay);
			pObj = &pChan->adslChanIntlDelay;
			break;
		case kOidAdslChanCurrTxRate:
			*objLen = sizeof(pChan->adslChanCurrTxRate);
			pObj = &pChan->adslChanCurrTxRate;
			break;
		case kOidAdslChanPrevTxRate:
			*objLen = sizeof(pChan->adslChanPrevTxRate);
			pObj = &pChan->adslChanPrevTxRate;
			break;
		case kOidAdslChanCrcBlockLength:
			*objLen = sizeof(pChan->adslChanCrcBlockLength);
			pObj = &pChan->adslChanCrcBlockLength;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanPerfTableCounterPtr (adslChanCounters *pPerf, uchar cntId, long *objLen)
{
	void	*pObj = NULL;

	switch (cntId) {
		case kOidAdslChanReceivedBlks:
			*objLen = sizeof(pPerf->adslChanReceivedBlks);
			pObj = &pPerf->adslChanReceivedBlks;
			break;
		case kOidAdslChanTransmittedBlks:
			*objLen = sizeof(pPerf->adslChanTransmittedBlks);
			pObj = &pPerf->adslChanTransmittedBlks;
			break;
		case kOidAdslChanCorrectedBlks:
			*objLen = sizeof(pPerf->adslChanCorrectedBlks);
			pObj = &pPerf->adslChanCorrectedBlks;
			break;
		case kOidAdslChanUncorrectBlks:
			*objLen = sizeof(pPerf->adslChanUncorrectBlks);
			pObj = &pPerf->adslChanUncorrectBlks;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanPerfTableObjPtr(void *gDslVars, uchar chId, uchar *objId, int objIdLen, long *objLen)
{
	void					*pObj = NULL;
	adslChanPerfDataEntry	*pChanPerf;

	pChanPerf = (kAdslIntlChannel == chId ? &globalVar.adslMib.adslChanIntlPerfData : &globalVar.adslMib.adslChanFastPerfData);

	if (0 == objIdLen) {
		*objLen = sizeof(adslChanPerfDataEntry);
		return pChanPerf;
	}
	if (objId[0] != kOidAdslChanPerfEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(adslChanPerfDataEntry);
		return pChanPerf;
	}

	switch (objId[1]) {
		case kOidAdslChanReceivedBlks:
		case kOidAdslChanTransmittedBlks:
		case kOidAdslChanCorrectedBlks:
		case kOidAdslChanUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfTotal, objId[1], objLen);
			break;

		case kOidAdslChanPerfValidIntervals:
			*objLen = sizeof(pChanPerf->adslChanPerfValidIntervals);
			pObj = &pChanPerf->adslChanPerfValidIntervals;
			break;	
		case kOidAdslChanPerfInvalidIntervals:
			*objLen = sizeof(pChanPerf->adslChanPerfInvalidIntervals);
			pObj = &pChanPerf->adslChanPerfInvalidIntervals;
			break;	
		case kOidAdslChanPerfCurr15MinTimeElapsed:
			*objLen = sizeof(pChanPerf->adslPerfCurr15MinTimeElapsed);
			pObj = &pChanPerf->adslPerfCurr15MinTimeElapsed;
			break;	

		case kOidAdslChanPerfCurr15MinReceivedBlks:
		case kOidAdslChanPerfCurr15MinTransmittedBlks:
		case kOidAdslChanPerfCurr15MinCorrectedBlks:
		case kOidAdslChanPerfCurr15MinUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfCurr15Min, objId[1]-kOidAdslChanPerfCurr15MinReceivedBlks+1, objLen);
			break;

		case kOidAdslChanPerfCurr1DayTimeElapsed:
			*objLen = sizeof(pChanPerf->adslPerfCurr1DayTimeElapsed);
			pObj = &pChanPerf->adslPerfCurr1DayTimeElapsed;
			break;	

		case kOidAdslChanPerfCurr1DayReceivedBlks:
		case kOidAdslChanPerfCurr1DayTransmittedBlks:
		case kOidAdslChanPerfCurr1DayCorrectedBlks:
		case kOidAdslChanPerfCurr1DayUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfCurr1Day, objId[1]-kOidAdslChanPerfCurr1DayReceivedBlks+1, objLen);
			break;

		case kOidAdslChanPerfPrev1DayMoniSecs:
			*objLen = sizeof(pChanPerf->adslPerfCurr1DayTimeElapsed);
			pObj = &pChanPerf->adslPerfCurr1DayTimeElapsed;
			break;	

		case kOidAdslChanPerfPrev1DayReceivedBlks:
		case kOidAdslChanPerfPrev1DayTransmittedBlks:
		case kOidAdslChanPerfPrev1DayCorrectedBlks:
		case kOidAdslChanPerfPrev1DayUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfPrev1Day, objId[1]-kOidAdslChanPerfPrev1DayReceivedBlks+1, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanPerfIntervalObjPtr(void *gDslVars, uchar chId, uchar *objId, int objIdLen, long *objLen)
{
	void				*pObj = NULL;
	adslChanCounters	*pIntervals;
	uchar				ind, nInt;

	if (kAdslIntlChannel == chId) {
		pIntervals = globalVar.adslMib.adslChanIntlPerfIntervals;
		nInt = globalVar.adslMib.adslChanIntlPerfData.adslChanPerfValidIntervals;
	}
	else {
		pIntervals = globalVar.adslMib.adslChanFastPerfIntervals;
		nInt = globalVar.adslMib.adslChanFastPerfData.adslChanPerfValidIntervals;
	}

	if (0 == objIdLen) {
		*objLen = sizeof(adslChanCounters) * nInt;
		return pIntervals;
	}
	if ((objId[0] != kOidAdslChanIntervalEntry) || (objIdLen < 4))
		return NULL;
	ind = objId[3];
	if ((ind == 0) || (ind > nInt))
		return NULL;

	switch (objId[1]) {
		case kOidAdslChanIntervalNumber:
			*objLen = sizeof(int);
			globalVar.scratchData = ind;
			pObj = &globalVar.scratchData;
			break;
		case kOidAdslChanIntervalReceivedBlks:
		case kOidAdslChanIntervalTransmittedBlks:
		case kOidAdslChanIntervalCorrectedBlks:
		case kOidAdslChanIntervalUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr(pIntervals+ind-1, objId[1]-kOidAdslChanIntervalReceivedBlks+1, objLen);
			break;
		case kOidAdslChanIntervalValidData:
			*objLen = sizeof(int);
			globalVar.scratchData = true;
			pObj = &globalVar.scratchData;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanObjPtr (void *gDslVars, uchar chId, uchar *objId, int objIdLen, long *objLen)
{
	void	*pObj = NULL;

	if ((objIdLen < 3) || (objId[0] != kOidAdslLine) || (objId[1] != kOidAdslMibObjects))
		return NULL;

	if (chId != AdslMibGetActiveChannel(gDslVars))
		return NULL;

	switch (objId[2]) {
		case kOidAdslAturChanTable:
			pObj = MibGetAdslChanTableObjPtr (gDslVars, chId, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturChanPerfTable:
			pObj = MibGetAdslChanPerfTableObjPtr (gDslVars, chId, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturChanIntervalTable:
			pObj = MibGetAdslChanPerfIntervalObjPtr (gDslVars, chId, objId+3, objIdLen-3, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}

	return pObj;
}

Private uchar * perToneToPerToneGrpArray(void	*gDslVars, uchar *dstPtr, uchar *srcPtr, bandPlanDescriptor *bp, short gFactor, int elemSize, int defVal)
{
#if defined(PSDATA_IMPL_VERSION) && (PSDATA_IMPL_VERSION >= 2)
	int  i, n;
	VdslToneGroup  *pCurToneRange = &bp->toneGroups[0];
	VdslToneGroup  lastRange = { 0x7FFF, 0x7FFF };
	uchar          *pDef = (void *) &defVal;
#if !(defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64))
	/* for big endian MIPS */
	pDef += 4 - elemSize;
#endif
#else
	int   n, gpNum;
#endif

	if (globalVar.actualgFactorForToneGroupObjects==false)
		gFactor=8;
	
	for(n=0;n<bp->noOfToneGroups;n++) {
		if((bp->toneGroups[n].endTone/gFactor) >= 512) {
			__SoftDslPrintf(gDslVars, "*** ALERT (dst %d)! lineId=%d noOfToneGroups=%d gFactor=%d actGfactorFlag=%d elemSize=%d ***", 0,
				(bp->toneGroups[n].endTone/gFactor) * elemSize, DslGetLineId(gDslVars),bp->noOfToneGroups,
				gFactor, globalVar.actualgFactorForToneGroupObjects, elemSize);
			globalVar.actualgFactorForToneGroupObjects=false;
			return NULL;
		}
	}
	
	globalVar.actualgFactorForToneGroupObjects=false;
#if defined(PSDATA_IMPL_VERSION) && (PSDATA_IMPL_VERSION >= 2)
	for (i = 0, n = 0; i < 512; i++, n += gFactor)
	  do {
		if (n <= pCurToneRange->endTone) {
			AdslMibByteMove(elemSize, (n < pCurToneRange->startTone ? pDef : &srcPtr[n*elemSize]), &dstPtr[i*elemSize]);
			break;
		}
		else
			pCurToneRange = (pCurToneRange != &bp->toneGroups[bp->noOfToneGroups-1]) ? pCurToneRange + 1 : &lastRange;
	  } while (1);
#else
	BlockByteClear(elemSize*512, (void*)dstPtr);
	for(n=0;n<bp->noOfToneGroups;n++)
		for(gpNum=bp->toneGroups[n].startTone/gFactor;gpNum<=bp->toneGroups[n].endTone/gFactor;gpNum++)
			AdslMibByteMove(elemSize, &srcPtr[gpNum*gFactor*elemSize], &dstPtr[gpNum*elemSize]);
#endif

	return dstPtr;
}
Private void CreateUserBandPlanUS(void *gDslVars, bandPlanDescriptor32 *usrBp, bandPlanDescriptor *bp)
{
    int n,i=0,US0limit=256,US1limit=1500;
    if (globalVar.adslMib.xdslInfo.vdsl2Profile==kVdslProfile30a)
    {
        US0limit=128;
        US1limit=750;
    }
    
    if(bp->noOfToneGroups>0 && bp->toneGroups[0].startTone>US0limit)
    {
        usrBp->toneGroups[i].startTone=0xFFFF;
        usrBp->toneGroups[i++].endTone=0xFFFF;
        if (bp->toneGroups[0].startTone>US1limit)
        {
            usrBp->toneGroups[i].startTone=0xFFFF;
            usrBp->toneGroups[i++].endTone=0xFFFF;
        }
    }
    for(n=0;n<bp->noOfToneGroups;n++)
    {
        usrBp->toneGroups[i].startTone=bp->toneGroups[n].startTone;
        usrBp->toneGroups[i++].endTone=bp->toneGroups[n].endTone;
    }
    usrBp->noOfToneGroups=i;
}

Private void CreateUserBandPlanDS(void *gDslVars, bandPlanDescriptor32 *usrBp, bandPlanDescriptor *bp)
{
    int n,i=0,DS1limit=1000;
    if (globalVar.adslMib.xdslInfo.vdsl2Profile==kVdslProfile30a)
        DS1limit=500;
    if(bp->noOfToneGroups>0 && bp->toneGroups[0].startTone>DS1limit)
    {
        usrBp->toneGroups[i].startTone=0xFFFF;
        usrBp->toneGroups[i++].endTone=0xFFFF;
    }
    for(n=0;n<bp->noOfToneGroups;n++)
    {
        usrBp->toneGroups[i].startTone=bp->toneGroups[n].startTone;
        usrBp->toneGroups[i++].endTone=bp->toneGroups[n].endTone;
    }
    usrBp->noOfToneGroups=i;
}


Private void PerBandObjectInReportFormat(void	*gDslVars, short *dstPtr, uchar objId, bandPlanDescriptor *bp,int elemSize, int usDir)
{
	int n,j=0;short specialVal,val;
	bandPlanDescriptor32 usrBp;
	void		*pObj;
	BlockByteClear(sizeof(short)*MAX_NUM_BANDS, (void*)dstPtr);
	
	if (usDir==1)
	{
		CreateUserBandPlanUS(gDslVars, &usrBp, bp);
		pObj=(void *)&globalVar.adslMib.perbandDataUs;
	}
	else
	{
		CreateUserBandPlanDS(gDslVars, &usrBp, bp);
		pObj=(void *)&globalVar.adslMib.perbandDataDs;	 
	}
	switch(objId)
	{
		case kOidAdslPrivLATNperband:       
			specialVal=1023;
			for (n=0,j=0;n<5;n++)
			{
				if (n<usrBp.noOfToneGroups && usrBp.toneGroups[n].startTone!=0xFFFF)
					val=((vdslperbandPMDdata *)pObj)[j++].adslCurrAtn;
				else
					val=specialVal;
				dstPtr[n]=val;
			}
			break;
		case kOidAdslPrivSATNperband:
			specialVal=1023;
			for (n=0,j=0;n<5;n++)
			{
				if (n<usrBp.noOfToneGroups && usrBp.toneGroups[n].startTone!=0xFFFF)
					val=((vdslperbandPMDdata *)pObj)[j++].adslSignalAttn;
				else
					val=specialVal;
				dstPtr[n]=val;
			}
			break;
		case kOidAdslPrivSNRMperband:
			specialVal=-512;
			for (n=0,j=0;n<5;n++)
			{
				if (n<usrBp.noOfToneGroups && usrBp.toneGroups[n].startTone!=0xFFFF)
					val=((vdslperbandPMDdata *)pObj)[j++].adslCurrSnrMgn;
				else
					val=specialVal;
				dstPtr[n]=val;
			}
			break;
#if defined(CONFIG_VDSL_SUPPORTED)
		case kOidAdslPrivTxPwrperband:
			if (usDir==1)
				pObj=(void *)&globalVar.adslMib.xdslPhys.perBandCurrOutputPwr;
			else
				pObj=(void *)&globalVar.adslMib.xdslAtucPhys.perBandCurrOutputPwr;
			specialVal=-1281;
			for (n=0,j=0;n<5;n++)
			{
				if (n<usrBp.noOfToneGroups && usrBp.toneGroups[n].startTone!=0xFFFF)
					val=((int *)pObj)[j++];
				else
					val=specialVal;
				dstPtr[n]=val;
			}
			break;
#endif
	}
}

#ifdef CONFIG_TOD_SUPPORTED
extern void *XdslCoreGetDslVars(unsigned char lineId);
Private void MibGetTodInfo(void *gDslVars, uchar *dataBuf)
{
	BlockByteMove(sizeof(TodInfo), (uchar *)&globalVar.todInfo, dataBuf);
}
#endif

#ifndef _M_IX86
Public int AdslMibSetObjectValue(
				void	*gDslVars, 
				uchar	*objId, 
				int		objIdLen,
				uchar	*dataBuf,
				long	*dataBufLen)
{
	int			i,len;
	ushort			*pInterval;
	dslCommandStruct	cmd;
	len = (dataBufLen)? *dataBufLen: 0;
	switch (objId[0])
	{
		case kOidAdslPrivate:
			switch (objId[1]) {
				case kOidAdslPrivExtraInfo:
					switch (objId[2])
					{
#ifdef NTR_SUPPORT
						case kOidAdslPrivSetNtrCfg:
							if(len > sizeof(dslNtrCfg))
								len = sizeof(dslNtrCfg);
							AdslMibByteMove(len, dataBuf,(void*)&globalVar.adslMib.ntrCfg);
								__SoftDslPrintf(gDslVars, "AdslMibSetObjectValue: kOidAdslPrivSetNtrCfg received, operMode=%d", 0, globalVar.adslMib.ntrCfg.operMode);
							break;
#endif
#ifdef SUPPORT_SELT
						case kOidAdslPrivSetSeltData:
							if(len > sizeof(SeltData))
								len = sizeof(SeltData);
							AdslMibByteMove(len, dataBuf,(void*)&globalVar.adslMib.selt);
								__SoftDslPrintf(gDslVars, "AdslMibSetObjectValue: kOidAdslPrivSetSeltData received", 0);
							break;
#endif /* SUPPORT_SELT */
						case kOidAdslPrivSetFlagActualGFactor:
								
								if(*dataBuf==0)
									globalVar.actualgFactorForToneGroupObjects=false;
								else globalVar.actualgFactorForToneGroupObjects=true;
								break;
						case kOidAdslPrivSetSnrClampShape:
							cmd.command = kDslSendEocCommand;
							cmd.param.dslClearEocMsg.msgId = kDslSetSnrClampingMask;
							cmd.param.dslClearEocMsg.msgType = len;
							gSharedMemAllocFromUserContext=1;
							cmd.param.dslClearEocMsg.dataPtr = (char *) AdslCoreSharedMemAlloc(len);
							AdslMibByteMove(len, &dataBuf[0], cmd.param.dslClearEocMsg.dataPtr);
							(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
							gSharedMemAllocFromUserContext=0;
							break;
						case kOidAdslPrivNonLinThldNumBins:
							globalVar.adslMib.adslNonLinData.NonLinThldNumAffectedBins=dataBuf[0];
							globalVar.adslMib.adslNonLinData.NonLinThldNumAffectedBins<<=8;
							globalVar.adslMib.adslNonLinData.NonLinThldNumAffectedBins|=dataBuf[1];
							if (globalVar.adslMib.adslNonLinData.NonLinNumAffectedBins>globalVar.adslMib.adslNonLinData.NonLinThldNumAffectedBins)
								globalVar.adslMib.adslNonLinData.NonLinearityFlag=1;
							else
								globalVar.adslMib.adslNonLinData.NonLinearityFlag=0;
							break;
						case kOidAdslPrivPLNDurationBins:
							if(len > sizeof(globalVar.PLNDurationBins))
								len = sizeof(globalVar.PLNDurationBins);
							
							for(i=0;i<kPlnNumberOfDurationBins;i++)
								globalVar.PLNDurationBins[i]=0;
							AdslMibByteMove(len, dataBuf,globalVar.PLNDurationBins);
							gSharedMemAllocFromUserContext=1;
							if(NULL!=(pInterval=(ushort *)AdslCoreSharedMemAlloc(len))){
								AdslMibByteMove(len, &dataBuf[0], pInterval);
								cmd.command=kDslPLNControlCmd;
								cmd.param.dslPlnSpec.plnCmd=kDslPLNControlDefineInpBinTable;
								cmd.param.dslPlnSpec.nInpBin= globalVar.adslMib.adslPLNData.PLNNbDurBins;
								cmd.param.dslPlnSpec.inpBinPtr=pInterval;
								(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
							}
							else
								__SoftDslPrintf(gDslVars, "No Shared Mem", 0);
							gSharedMemAllocFromUserContext=0;
#ifdef G992P3_DBG_PRINT
							__SoftDslPrintf(gDslVars, "PLNDurationBins=%d cmd.param.dslPlnSpec.nInpBin=%d", 0,
									globalVar.PLNDurationBins,cmd.param.dslPlnSpec.nInpBin);
#endif
							break;
						case kOidAdslPrivPLNIntrArvlBins:
							if(len > sizeof(globalVar.PLNIntrArvlBins))
								len = sizeof(globalVar.PLNIntrArvlBins);
							
							for(i=0;i<kPlnNumberOfInterArrivalBins;i++)
								globalVar.PLNIntrArvlBins[i]=0;
							AdslMibByteMove(len, dataBuf, globalVar.PLNIntrArvlBins);
#ifdef G992P3_DBG_PRINT
							__SoftDslPrintf(gDslVars, "PLNIntrArvlBins=%X %X %X %X", 0,
									globalVar.PLNIntrArvlBins[0],globalVar.PLNIntrArvlBins[1],
									globalVar.PLNIntrArvlBins[2],globalVar.PLNIntrArvlBins[3]);
#endif
							gSharedMemAllocFromUserContext=1;
							if(NULL!=(pInterval=(ushort *)AdslCoreSharedMemAlloc(len))){
								AdslMibByteMove(len, &dataBuf[0], pInterval);
#ifdef G992P3_DBG_PRINT
								__SoftDslPrintf(gDslVars, "pInterval=%X %X %X %X", 0,
										pInterval[0],pInterval[1],pInterval[2],pInterval[3]);
#endif
								cmd.command=kDslPLNControlCmd;
								cmd.param.dslPlnSpec.plnCmd=kDslPLNControlDefineItaBinTable;
								cmd.param.dslPlnSpec.nItaBin=globalVar.adslMib.adslPLNData.PLNNbIntArrBins;
								cmd.param.dslPlnSpec.itaBinPtr= pInterval;
								(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
							}
							gSharedMemAllocFromUserContext=0;
#ifdef G992P3_DBG_PRINT
							__SoftDslPrintf(gDslVars,  "cmd.param.dslPlnSpec.nItaBin=%d", 0, cmd.param.dslPlnSpec.nItaBin);
#endif
							break;
						case kOidAdslPrivINMConfigFormat:
							cmd.command=kDslPLNControlCmd;
							cmd.param.dslPlnSpec.plnCmd=kDslINMConfigInpEqFormat;
							cmd.param.dslPlnSpec.inmInpEqFormat=dataBuf[0];
							(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
							gG992p3OvhMsgVars.inmConfig.INM_INPEQ_FORMAT=dataBuf[0];
							break;
						case kOidAdslPrivINMControlParams:
							cmd.command=kDslPLNControlCmd;
							cmd.param.dslPlnSpec.plnCmd=kDslINMControlParams;
							cmd.param.dslPlnSpec.inmContinueConfig=dataBuf[0];
							cmd.param.dslPlnSpec.inmInpEqMode= dataBuf[1];
							(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
							cmd.command=kDslPLNControlCmd;
							cmd.param.dslPlnSpec.plnCmd=kDslINMConfigParams;
							cmd.param.dslPlnSpec.inmContinueConfig=dataBuf[0];
							cmd.param.dslPlnSpec.inmInpEqMode=dataBuf[1];
							cmd.param.dslPlnSpec.inmIATO=dataBuf[2];
							cmd.param.dslPlnSpec.inmIATS= dataBuf[3];
							(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
							
#ifdef G992P3_DBG_PRINT
							__SoftDslPrintf(gDslVars,  "kOidAdslPrivINMControlParams inmContinueConfig=%d inmInpEqMode=%d inmIATO=%d inmIATS=%d", 0, dataBuf[0], dataBuf[1], dataBuf[2], dataBuf[3]);
#endif 
							break;
						case kOidAdslExtraPLNData:
							switch (objId[3]){
								case kOidAdslExtraPLNDataThldBB:
									globalVar.adslMib.adslPLNData.PLNThldBB=dataBuf[0];
									globalVar.adslMib.adslPLNData.PLNThldBB<<=8;
									globalVar.adslMib.adslPLNData.PLNThldBB|=dataBuf[1];
									break;
								case kOidAdslExtraPLNDataThldPerTone:
									globalVar.adslMib.adslPLNData.PLNThldPerTone=dataBuf[0];
									globalVar.adslMib.adslPLNData.PLNThldPerTone<<=8;
									globalVar.adslMib.adslPLNData.PLNThldPerTone|=dataBuf[1];
									break;
								case kOidAdslExtraPLNDataPLNState:
									if(*dataBuf==2){
#ifdef G992P3_DBG_PRINT
										__SoftDslPrintf(gDslVars, "Start Requested\n", 0);
#endif
										globalVar.adslMib.adslPLNData.PLNState=1;
									}
									else if(*dataBuf==3){
#ifdef G992P3_DBG_PRINT
										__SoftDslPrintf(gDslVars, "Stop Requested\n", 0);
#endif
										globalVar.adslMib.adslPLNData.PLNState=0;
									}
									cmd.command=kDslPLNControlCmd;
									cmd.param.value=kDslPLNControlStop;
									(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
									if(globalVar.adslMib.adslPLNData.PLNState==1) {
										cmd.command=kDslPLNControlCmd;
										cmd.param.dslPlnSpec.plnCmd=kDslPLNControlStart;
										cmd.param.dslPlnSpec.mgnDescreaseLevelPerBin= globalVar.adslMib.adslPLNData.PLNThldPerTone;
										cmd.param.dslPlnSpec.mgnDescreaseLevelBand=globalVar.adslMib.adslPLNData.PLNThldBB;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
									}
									break;
								case kOidAdslExtraPLNDataNbDurBins:
									if(*dataBuf>kPlnNumberOfDurationBins)
										*dataBuf=kPlnNumberOfDurationBins;
									globalVar.adslMib.adslPLNData.PLNNbDurBins=dataBuf[0];
#ifdef G992P3_DBG_PRINT
									__SoftDslPrintf(gDslVars, "PLNNbDurBins = %d", 0, globalVar.adslMib.adslPLNData.PLNNbDurBins);
#endif
									break;
								case kOidAdslExtraPLNDataNbIntArrBins:
									if(*dataBuf>kPlnNumberOfInterArrivalBins)
										*dataBuf=kPlnNumberOfInterArrivalBins;
									globalVar.adslMib.adslPLNData.PLNNbIntArrBins=dataBuf[0];
#ifdef G992P3_DBG_PRINT
									__SoftDslPrintf(gDslVars, "PLNNbIntrArrBins = %d", 0, globalVar.adslMib.adslPLNData.PLNNbIntArrBins);
#endif
									break;
								case kOidAdslExtraPLNDataUpdate:
									if(*dataBuf==1)
									{
#ifdef G992P3_DBG_PRINT
										__SoftDslPrintf(gDslVars, "Update Requested , sending update commands\n", 0);
#endif
										globalVar.adslMib.adslPLNData.PLNUpdateData=0;
										cmd.command=kDslPLNControlCmd;
										cmd.param.value=kDslPLNControlPeakNoiseGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										 cmd.command=kDslPLNControlCmd;
										cmd.param.value=kDslPLNControlThldViolationGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										cmd.command=kDslPLNControlCmd;
										cmd.param.value=kDslPLNControlImpulseNoiseEventGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										cmd.command=kDslPLNControlCmd; 
										cmd.param.value= kDslPLNControlGetStatus;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										cmd.command=kDslPLNControlCmd; 
										cmd.param.value=kDslPLNControlImpulseNoiseTimeGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
									}
									break;
							}	
							break;
					}
					break;
			}
			break;
#ifdef SUPPORT_MULTI_PHY
		case kOidAdslPrivateSysCtl:
			switch (objId[1]) {
				case kOidAdslPrivateSysMediaCfg:
					if(3 == objIdLen) {
						BcmCoreDpcSyncExit(SYNC_RX);
						XdslCoreProcessPrivateSysMediaCfg((uint)objId[2]);
						BcmCoreDpcSyncEnter(SYNC_RX);
					}
					break;
			}
			break;
#endif
	}
	return kAdslMibStatusSuccess;
}
#endif

Public long	AdslMibGetObjectValue (
				void	*gDslVars, 
				uchar	*objId, 
				int		objIdLen,
				uchar	*dataBuf,
				long	*dataBufLen)
{
	uchar		*pObj;
	long		objLen, bufLen;
	
	bufLen = *dataBufLen;
	pObj = NULL;
	if ((NULL == objId) || (0 == objIdLen)) {
		pObj = (void *) &globalVar.adslMib;
		objLen = sizeof (globalVar.adslMib);
	}
	else {
		switch (objId[0]) {
			case kOidAdsl:
				pObj = MibGetAdslObjPtr (gDslVars, objId+1, objIdLen-1, &objLen);
				break;
			case kOidAdslInterleave:
				pObj = MibGetAdslChanObjPtr (gDslVars, kAdslIntlChannel, objId+1, objIdLen-1, &objLen);
				break;
			case kOidAdslFast:
				pObj = MibGetAdslChanObjPtr (gDslVars, kAdslFastChannel, objId+1, objIdLen-1, &objLen);
				break;
			case kOidAtm:
				pObj = MibGetAtmTcObjPtr (gDslVars, objId+1, objIdLen-1, &objLen);
				break;
#ifndef _M_IX86
			case kOidAdslPhyCfg:
				pObj=(void *) &adslCoreCfgProfile[gLineId(gDslVars)];
				objLen=sizeof(adslCfgProfile);
				break;
#endif				
			case kOidAdslPrivate:
				switch (objId[1]) {
					case kOidAdslPrivSNR:
						pObj = (void *) globalVar.snr;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							objLen = sizeof (globalVar.snr[0])*globalVar.nTonesGfast;
						else
#endif
						objLen = sizeof (globalVar.snr[0])*globalVar.nTones;
#ifndef _M_IX86
						__SoftDslPrintf(gDslVars, "kOidAdslPrivSNR Len=%d", 0, globalVar.nTones);
#endif
						break;
					case kOidAdslPrivBitAlloc:
						pObj = (void *) globalVar.bitAlloc;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							objLen = sizeof (globalVar.bitAlloc[0]) * globalVar.nTonesGfast;
						else
#endif
						objLen = sizeof (globalVar.bitAlloc[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivGain:
						pObj = (void *) globalVar.gain;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							objLen = sizeof (globalVar.gain[0]) * globalVar.nTonesGfast;
						else
#endif
						objLen = sizeof (globalVar.gain[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivShowtimeMargin:
						pObj = (void *) globalVar.showtimeMargin;
						objLen = sizeof (globalVar.showtimeMargin[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivChanCharLin:
						pObj = (void *) globalVar.chanCharLin;
						objLen = sizeof (globalVar.chanCharLin[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivChanCharLog:
						pObj = (void *) globalVar.chanCharLog;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							objLen = sizeof (globalVar.chanCharLog[0]) * globalVar.nTonesGfast;
						else
#endif
						objLen = sizeof (globalVar.chanCharLog[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivQuietLineNoise:
						pObj = (void *) globalVar.quietLineNoise;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							objLen = sizeof (globalVar.quietLineNoise[0]) * globalVar.nTonesGfast;
						else
#endif
						objLen = sizeof (globalVar.quietLineNoise[0]) * globalVar.nTones;
						break;
#if defined(CONFIG_BCM_DSL_GFAST)
					case kOidAdslPrivActiveLineNoise:
						pObj = (void *) globalVar.activeLineNoise;
						objLen = sizeof (globalVar.activeLineNoise[0]) * globalVar.nTonesGfast;
						break;
					case kOidAdslPrivDoiBitAlloc:
						pObj = (void *) globalVar.doiBitAlloc;
						objLen = sizeof (globalVar.doiBitAlloc[0]) * globalVar.nTonesGfast;
						break;
					case kOidAdslPrivDoiGain:
						pObj = (void *) globalVar.doiGain;
						objLen = sizeof (globalVar.doiGain[0]) * globalVar.nTonesGfast;
						break;
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
					case kOidAdslPrivUER:
						pObj = (void *) globalVar.uer;
						objLen = sizeof (globalVar.uer[0])*globalVar.nTones;
						break;
#if defined(SUPPORT_SELT)
					case kOidAdslPrivEchoVariance:
						pObj = (void *) globalVar.echoVariance;
						objLen = sizeof (globalVar.echoVariance[0])*kVdslMibMaxToneNum*2;
						break;
#endif
#endif
					case kOidAdslPrivPLNDurationBins:
						pObj = (void *) &globalVar.PLNDurationBins;
						objLen = sizeof (globalVar.PLNDurationBins[0])*kPlnNumberOfDurationBins;
						break;
					case kOidAdslPrivPLNIntrArvlBins:
						pObj = (void *) &globalVar.PLNIntrArvlBins;
						objLen = sizeof (globalVar.PLNIntrArvlBins[0])*kPlnNumberOfInterArrivalBins;
						break;
#ifdef ADSL_MIBOBJ_PLN
					case kOidAdslPrivPLNValueps :
						pObj = (void *) &globalVar.PLNValueps;
						objLen = sizeof (globalVar.PLNValueps[0])*(kAdslMibToneNum*2-32);
						break;
					case kOidAdslPrivPLNThldCntps  :
						pObj = (void *) &globalVar.PLNThldCntps;
						objLen = sizeof (globalVar.PLNThldCntps[0])*(kAdslMibToneNum*2-32);
						break;
#endif
					case kOidAdslPrivPLNDurationHist :
						pObj = (void *) &globalVar.PLNDurationHist;
						objLen = sizeof (globalVar.PLNDurationHist[0])*kPlnNumberOfDurationBins;
						break;
					case kOidAdslPrivPLNIntrArvlHist:
						pObj = (void *) &globalVar.PLNIntrArvlHist;
						objLen = sizeof (globalVar.PLNIntrArvlHist[0])*kPlnNumberOfInterArrivalBins;
						break;
					case kOidAdslPrivNLDistNoise:
						pObj = (void *) &globalVar.distNoisedB;
						objLen = sizeof (globalVar.distNoisedB[0])*kAdslMibToneNum*2;
						break;
					case kOidAdslPrivChanCharLinDsPerToneGroup:
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *) (&globalVar.chanCharLin[0]), &globalVar.adslMib.dsNegBandPlan, globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds,sizeof(ComplexShort),0x8000);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(ComplexShort);
						break;
					case kOidAdslPrivChanCharLinUsPerToneGroup:
					{
						ComplexShort *pChanCharLin;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							pChanCharLin = globalVar.chanCharLin + globalVar.nTonesGfast/2;
						else
#endif
							pChanCharLin = globalVar.chanCharLin;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *)pChanCharLin, &globalVar.adslMib.usNegBandPlan, globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus,sizeof(ComplexShort),0x8000);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(ComplexShort);
					}
						break;
					case kOidAdslPrivChanCharLogDsPerToneGroup:
						if(globalVar.dsBpHlogForReport==NULL)
							break;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *) (&globalVar.chanCharLog[0]), globalVar.dsBpHlogForReport, globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds,sizeof(short),0x8000);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
						break;
					case kOidAdslPrivChanCharLogUsPerToneGroup:
					{
						short *pChanCharLog;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							pChanCharLog = globalVar.chanCharLog + globalVar.nTonesGfast/2;
						else
#endif
							pChanCharLog = globalVar.chanCharLog;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *)pChanCharLog, &globalVar.adslMib.usNegBandPlanDiscovery, globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus,sizeof(short),0x8000);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
					}
						break;
					case kOidAdslPrivQuietLineNoiseDsPerToneGroup:
						if(globalVar.dsBpQLNForReport==NULL)
							break;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *) (&globalVar.quietLineNoise[0]), globalVar.dsBpQLNForReport, globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds,sizeof(short),0);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
						break;
					case kOidAdslPrivQuietLineNoiseUsPerToneGroup:
					{
						short *pQln;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							pQln = globalVar.quietLineNoise + globalVar.nTonesGfast/2;
						else
#endif
							pQln = globalVar.quietLineNoise;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *)pQln, &globalVar.adslMib.usNegBandPlanDiscovery, globalVar.adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus,sizeof(short),0);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
					}
						break;
					case kOidAdslPrivSNRDsPerToneGroup:
						if (globalVar.dsBpSNRForReport==NULL)
							break;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *) (&globalVar.snr[0]), globalVar.dsBpSNRForReport, *globalVar.dsGfactorForSNRReport,sizeof(short),0x8000);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
						break;
					case kOidAdslPrivSNRUsPerToneGroup:
					{
						short *pSnr;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							pSnr = globalVar.snr + globalVar.nTonesGfast/2;
						else
#endif
							pSnr = globalVar.snr;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *)pSnr, &globalVar.adslMib.usNegBandPlan, globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus,sizeof(short),0x8000);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
					}
						break;
					case kOidAdslPrivLATNdsperband:
						pObj=globalVar.scratchObject;
						PerBandObjectInReportFormat(gDslVars,(short *)pObj,kOidAdslPrivLATNperband,&globalVar.adslMib.dsNegBandPlanDiscovery,sizeof(short),0);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivSATNdsperband:
						pObj=globalVar.scratchObject;
						if(globalVar.dsBpSATNpbForReport==NULL)
							break;
						PerBandObjectInReportFormat(gDslVars,(short *)pObj,kOidAdslPrivSATNperband,globalVar.dsBpSATNpbForReport,sizeof(short),0);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivLATNusperband:
						pObj=globalVar.scratchObject;
						PerBandObjectInReportFormat(gDslVars,(short *)pObj,kOidAdslPrivLATNperband,&globalVar.adslMib.usNegBandPlanDiscovery,sizeof(short),1);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivSATNusperband:
						pObj=globalVar.scratchObject;
						if(globalVar.usBpSATNpbForReport==NULL)
							break;
						PerBandObjectInReportFormat(gDslVars,(short*)pObj,kOidAdslPrivSATNperband,globalVar.usBpSATNpbForReport,sizeof(short),1);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivSNRMusperband:
						pObj=globalVar.scratchObject;
						if(globalVar.usBpSNRpbForReport==NULL)
						    break;
						PerBandObjectInReportFormat(gDslVars,(short*)pObj,kOidAdslPrivSNRMperband,globalVar.usBpSNRpbForReport,sizeof(short),1);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivSNRMdsperband:
						pObj=globalVar.scratchObject;
						if(globalVar.dsBpSNRpbForReport==NULL)
							break;
						PerBandObjectInReportFormat(gDslVars,(short*)pObj,kOidAdslPrivSNRMperband,globalVar.dsBpSNRpbForReport,sizeof(short),0);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivTxPwrusperband:
						pObj=globalVar.scratchObject;
						PerBandObjectInReportFormat(gDslVars,(short*)pObj,kOidAdslPrivTxPwrperband,&globalVar.adslMib.usPhyBandPlanDiscovery,sizeof(short),1);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivTxPwrdsperband:
						pObj=globalVar.scratchObject;
						PerBandObjectInReportFormat(gDslVars,(short*)pObj,kOidAdslPrivTxPwrperband,&globalVar.adslMib.dsPhyBandPlanDiscovery,sizeof(short),0);
						objLen=5*sizeof(short);
						break;
					case kOidAdslPrivBitAllocDsPerToneGroup:
						pObj=perToneToPerToneGrpArray(gDslVars, globalVar.scratchObject,(uchar *) (&globalVar.bitAlloc[0]), &globalVar.adslMib.dsNegBandPlan, globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds,sizeof(uchar),0);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(uchar);
						break;
					case kOidAdslPrivBitAllocUsPerToneGroup:
					{
						uchar *pBitAlloc;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							pBitAlloc = globalVar.bitAlloc + globalVar.nTonesGfast/2;
						else
#endif
							pBitAlloc = globalVar.bitAlloc;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *)pBitAlloc, &globalVar.adslMib.usNegBandPlan, globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus,sizeof(uchar),0);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(uchar);
					}
						break;
					case kOidAdslPrivGainDsPerToneGroup:
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *) (&globalVar.gain[0]), &globalVar.adslMib.dsNegBandPlan, globalVar.adslMib.gFactors.Gfactor_MEDLEYSETds,sizeof(short),0);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
						break;
					case kOidAdslPrivGainUsPerToneGroup:
					{
						short *pGain;
#ifdef CONFIG_BCM_DSL_GFAST
						if(XdslMibIsGfastMod(gDslVars))
							pGain = globalVar.gain + globalVar.nTonesGfast/2;
						else
#endif
							pGain = globalVar.gain;
						pObj=perToneToPerToneGrpArray(gDslVars,globalVar.scratchObject,(uchar *)pGain, &globalVar.adslMib.usNegBandPlan, globalVar.adslMib.gFactors.Gfactor_MEDLEYSETus,sizeof(short),0);
						if(NULL == pObj)
							break;
						objLen=512*sizeof(short);
					}
						break;
#if defined(SUPPORT_VECTORING)
					case kOIdAdslPrivGetVectState:
						pObj = (void *)&globalVar.vectSM.state;
						objLen = sizeof(globalVar.vectSM.state);
						break;
					case kOIdAdslPrivGetVectSM:
						objLen = sizeof(VectoringStateMachine);
						pObj = globalVar.scratchObject;
						BlockByteMove(objLen, (unsigned char *)&globalVar.vectSM, pObj);
						break;
					case kOIdAdslPrivGetVectPhyData:
						objLen = sizeof(MibVectDataPhy);
						pObj = globalVar.scratchObject;
						BlockByteMove(objLen, (unsigned char *)&globalVar.vectPhyData, pObj);
						break;
					case kOIdAdslPrivGetVceMacAddress:
						objLen = sizeof(VceMacAddress);
						pObj = globalVar.scratchObject;
						BlockByteMove(objLen, (unsigned char *)&globalVar.adslMib.vectData.macAddress, pObj);
						break;
					case kOidAdslPrivGetExtraSkbCnt:
					{
						unsigned int extraSkb = BcmXdslGetVectExtraSkbCnt();
						objLen = sizeof(unsigned int);
						pObj = globalVar.scratchObject;
						BlockByteMove(objLen, (unsigned char *)&extraSkb, pObj);
						break;
					}
#endif
#if defined(CONFIG_RNC_SUPPORT)
					case kOidAdslPrivQuietLineNoiseRnc:
						pObj = (void *) globalVar.quietLineNoiseRnc;
						objLen = sizeof (globalVar.quietLineNoiseRnc[0]) * globalVar.nTones;
						break;
#endif
#ifdef CONFIG_TOD_SUPPORTED
					case kOidAdslPrivGetTodInfo:
						objLen = sizeof(TodInfo);
						pObj = globalVar.scratchObject;
						MibGetTodInfo(gDslVars, pObj);
						if(((TodInfo *)pObj)->todStatus) {
							((TodInfo *)pObj)->todStatus += gLineId(gDslVars);
						}
#ifdef SUPPORT_DSL_BONDING
						else {
							uchar otherLineId = gLineId(gDslVars) ^ 1;
							MibGetTodInfo(XdslCoreGetDslVars(otherLineId), pObj);
							if(((TodInfo *)pObj)->todStatus)
								((TodInfo *)pObj)->todStatus += otherLineId;
						}
#endif
						break;
#endif
#ifdef SUPPORT_HMI
					case kOidAdslPrivGetMrefPsdInfo:
						pObj = (void *)&globalVar.gMrefPsd;
						objLen = sizeof(globalVar.gMrefPsd);
						break;
#endif
					case kOidAdslPrivExtraInfo:
						switch (objId[2]) {
							case kOidAdslExtraConnectionInfo:
								pObj = (void *) &globalVar.adslMib.xdslConnection;
								objLen = sizeof (globalVar.adslMib.xdslConnection);
								break;
							case kOidAdslExtraConnectionStat:
								pObj = (void *) &globalVar.adslMib.xdslStat;
								objLen = sizeof (globalVar.adslMib.xdslStat);
								break;
							case kOidAdslExtraFramingMode:
								pObj = (void *) &globalVar.adslMib.adslFramingMode;
								objLen = sizeof (globalVar.adslMib.adslFramingMode);
								break;
							case kOidAdslExtraTrainingState:
								pObj = (void *) &globalVar.adslMib.adslTrainingState;
								objLen = sizeof (globalVar.adslMib.adslTrainingState);
								break;
							case kOidAdslExtraNonStdFramingAdjustK:
								pObj = (void *) &globalVar.adslMib.adslRxNonStdFramingAdjustK;
								objLen = sizeof (globalVar.adslMib.adslRxNonStdFramingAdjustK);
								break;
							case kOidAdslExtraAtmStat:
								pObj = (void *) &globalVar.adslMib.atmStat2lp;
								objLen = sizeof (globalVar.adslMib.atmStat2lp);
								break;
							case kOidAdslExtraDiagModeData:
								pObj = (void *) &globalVar.adslMib.adslDiag;
								objLen = sizeof (globalVar.adslMib.adslDiag);
								break;
							case kOidAdslExtraAdsl2Info:
								pObj = (void *) &globalVar.adslMib.adsl2Info2lp;
								objLen = sizeof (globalVar.adslMib.adsl2Info2lp);
								break;
							case kOidAdslExtraTxPerfCounterInfo:
								pObj = (void *) &globalVar.adslMib.adslTxPerfTotal;
								objLen = sizeof (globalVar.adslMib.adslTxPerfTotal);
								break;
							case kOidAdslExtraNLInfo:
								pObj = (void *) &globalVar.adslMib.adslNonLinData;
								objLen= sizeof(globalVar.adslMib.adslNonLinData);
								break;
							case kOidAdslExtraPLNInfo:
								pObj = (void *) &globalVar.adslMib.adslPLNData;
								objLen= sizeof(globalVar.adslMib.adslPLNData);
								break;
#ifdef G993
							case kOidAdslPrivBandPlanUSNeg:
								pObj = (void *) &globalVar.adslMib.usNegBandPlan;
								objLen= sizeof(globalVar.adslMib.usNegBandPlan);
								break;
							case kOidAdslPrivBandPlanUSPhy:
								pObj = (void *) &globalVar.adslMib.usPhyBandPlan;
								objLen= sizeof(globalVar.adslMib.usPhyBandPlan);
								break;
							case kOidAdslPrivBandPlanDSNeg:
								pObj = (void *) &globalVar.adslMib.dsNegBandPlan;
								objLen= sizeof(globalVar.adslMib.dsNegBandPlan);
								break;
							case kOidAdslPrivBandPlanDSPhy:
								pObj = (void *) &globalVar.adslMib.dsPhyBandPlan;
								objLen= sizeof(globalVar.adslMib.dsPhyBandPlan);
								break;
							case kOidAdslPrivBandPlanUSNegDiscovery:
								pObj = (void *) &globalVar.adslMib.usNegBandPlanDiscovery;
								objLen= sizeof(globalVar.adslMib.usNegBandPlanDiscovery);
								break;
							case kOidAdslPrivBandPlanUSPhyDiscovery:
								pObj = (void *) &globalVar.adslMib.usPhyBandPlanDiscovery;
								objLen= sizeof(globalVar.adslMib.usPhyBandPlanDiscovery);
								break;
							case kOidAdslPrivBandPlanDSNegDiscovery:
								pObj = (void *) &globalVar.adslMib.dsNegBandPlanDiscovery;
								objLen= sizeof(globalVar.adslMib.dsNegBandPlanDiscovery);
								break;
							case kOidAdslPrivBandPlanDSPhyDiscovery:
								pObj = (void *) &globalVar.adslMib.dsPhyBandPlanDiscovery;
								objLen= sizeof(globalVar.adslMib.dsPhyBandPlanDiscovery);
								break;
							case kOidAdslPrivBandPlanUSNegPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanUS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.usNegBandPlan);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
							case kOidAdslPrivBandPlanUSPhyPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanUS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.usPhyBandPlan);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
							case kOidAdslPrivBandPlanDSNegPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanDS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.dsNegBandPlan);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
							case kOidAdslPrivBandPlanDSPhyPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanDS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.dsPhyBandPlan);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
							case kOidAdslPrivBandPlanUSNegDiscoveryPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanUS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.usNegBandPlanDiscovery);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
							case kOidAdslPrivBandPlanUSPhyDiscoveryPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanUS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.usPhyBandPlanDiscovery);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
							case kOidAdslPrivBandPlanDSNegDiscoveryPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanDS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.dsNegBandPlanDiscovery);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
							case kOidAdslPrivBandPlanDSPhyDiscoveryPresentation:
								pObj = (void *) &globalVar.scratchBpObject;
								CreateUserBandPlanDS(gDslVars,&globalVar.scratchBpObject,&globalVar.adslMib.dsPhyBandPlanDiscovery);
								objLen= sizeof(globalVar.scratchBpObject);
								break;
#endif						
							case kOidAdslPrivINMConfigParameters:
								pObj = (void *) &gG992p3OvhMsgVars.inmConfig;
								objLen=sizeof(gG992p3OvhMsgVars.inmConfig);
								break;
#if defined(CONFIG_VDSL_SUPPORTED)
							case kOidAdslExtraDiagModeDataVdsl:
								pObj = (void *) &globalVar.adslMib.vdslDiag;
								objLen = sizeof (globalVar.adslMib.vdslDiag);
								break;
							case  kOidAdslExtraVdsl2Info:
								pObj = (void *) &globalVar.adslMib.vdslInfo;
								objLen = sizeof (globalVar.adslMib.vdslInfo);
								break;
#endif	
							case kOidAdslExtraXdslInfo:
								pObj = (void *) &globalVar.adslMib.xdslInfo;
								objLen = sizeof (globalVar.adslMib.xdslInfo);
								break;
#ifdef SUPPORT_SELT
							case kOidAdslPrivGetSeltData:
								pObj = (uchar *)&globalVar.adslMib.selt;
								objLen = sizeof(SeltData);
								break;
#endif /* SUPPORT_SELT */
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
							case kOidAdslPrivGetGfastOlrCnt:
								pObj = (void *) &globalVar.adslMib.gfastOlrXoiCounterData;
								objLen = sizeof (globalVar.adslMib.gfastOlrXoiCounterData);
								break;
							case kOidAdslPrivGetGfastDta:
								pObj = (void *) &globalVar.adslMib.gfastDta;
								objLen = sizeof (globalVar.adslMib.gfastDta);
								break;
#endif
#if defined(NTR_SUPPORT)
							case kOidAdslPrivGetNtrCnt:
								__SoftDslPrintf(gDslVars, "Voip: globalVar.adslMib.ntrCnt.phaseError = 0x%08x", 0, globalVar.adslMib.ntrCnt.phaseError);
								pObj = (void *) &globalVar.adslMib.ntrCnt;
								objLen = sizeof (globalVar.adslMib.ntrCnt);
								break;
#endif
						}
						break;
				}
				break;
		}
	}
	if (NULL == pObj)
		return kAdslMibStatusNoObject;

	*dataBufLen = objLen;
	if (NULL == dataBuf)
		return (long)pObj;
	if (objLen > bufLen)
		return kAdslMibStatusBufferTooSmall;

	AdslMibByteMove(objLen, pObj, dataBuf);

	return kAdslMibStatusSuccess;
}
