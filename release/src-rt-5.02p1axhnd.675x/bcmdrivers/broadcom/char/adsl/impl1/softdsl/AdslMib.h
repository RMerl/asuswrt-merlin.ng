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
 * AdslMib.h 
 *
 * Description:
 *	This file contains the exported functions and definitions for AdslMib
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.9 $
 *
 * $Id: AdslMib.h,v 1.9 2004/04/12 23:34:52 ilyas Exp $
 *
 * $Log: AdslMib.h,v $
 * Revision 1.9  2004/04/12 23:34:52  ilyas
 * Merged the latest ADSL driver chnages for ADSL2+
 *
 * Revision 1.8  2004/03/03 20:14:05  ilyas
 * Merged changes for ADSL2+ from ADSL driver
 *
 * Revision 1.7  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.6  2003/07/18 19:07:15  ilyas
 * Merged with ADSL driver
 *
 * Revision 1.5  2002/10/31 20:27:13  ilyas
 * Merged with the latest changes for VxWorks/Linux driver
 *
 * Revision 1.4  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.3  2002/01/13 22:25:40  ilyas
 * Added functions to get channels rate
 *
 * Revision 1.2  2002/01/03 06:03:36  ilyas
 * Handle byte moves tha are not multiple of 2
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#ifndef	AdslMibHeader
#define	AdslMibHeader

#if defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#endif

#include "AdslMibDef.h"
#include "BlockUtil.h"

/* Extended showtime monitor counters */
#define kG992ShowtimeNumOfCellTotal				0	/* number of Tx Cell */
#define kG992ShowtimeNumOfCellData				1	/* number of Tx Data Cells */
#define kG992ShowtimeNumOfBitErrs					2	/* number of Tx Bit Errors */
#define kG992ShowtimeNumOfCellTotalRcved			3	/* number of Rx Cell */
#define kG992ShowtimeNumOfCellDataRcved			4	/* number of Rx Data Cells */
#define kG992ShowtimeNumOfCellDropRcved			5	/* number of Rx Cell Drops */
#define kG992ShowtimeNumOfBitErrsRcved			6	/* number of Rx Bit Errors */

#define kG992ShowtimeNumOfExtendedMonitorCounters	(kG992ShowtimeNumOfBitErrsRcved+1)	/* always last number + 1 */

/* Interface functions */

typedef	int	(SM_DECL *adslMibNotifyHandlerType)	(void *gDslVars, uint event);

extern Boolean	AdslMibInit(void *gDslVars, dslCommandHandlerType	commandHandler);
extern void		AdslMibTimer(void *gDslVars, uint timeMs);
extern int		AdslMibStatusSnooper (void *gDslVars, dslStatusStruct *status);
extern void		AdslMibSetNotifyHandler(void *gDslVars, adslMibNotifyHandlerType notifyHandlerPtr);
extern int		AdslMibGetModulationType(void *gDslVars);
extern Boolean	AdslMibIsAdsl2Mod(void *gDslVars);
extern Boolean	XdslMibIsVdsl2Mod(void *gDslVars);
extern Boolean	XdslMibIsXdsl2Mod(void *gDslVars);
extern Boolean	XdslMibIsGfastMod(void *gDslVars);
extern int		AdslMibGetActiveChannel(void *gDslVars);
extern int		AdslMibGetGetChannelRate(void *gDslVars, int dir, int channel);
extern Boolean	AdslMibIsLinkActive(void *gDslVars);
extern int		AdslMibPowerState(void *gDslVars);
extern int		AdslMibTrainingState (void *gDslVars);
extern void		AdslMibClearData(void *gDslVars);
extern void		AdslMibClearBertResults(void *gDslVars);
extern void		AdslMibBertStartEx(void *gDslVars, uint bertSec);
extern void		AdslMibBertStopEx(void *gDslVars);
extern uint	AdslMibBertContinueEx(void *gDslVars, uint totalBits, uint errBits);
extern void		AdslMibSetLPR(void *gDslVars);
extern void		AdslMibSetShowtimeMargin(void *gDslVars, int showtimeMargin);
extern void		AdslMibSetNumTones (void *gDslVars, int numTones);
extern void		AdslMibResetConectionStatCounters(void *gDslVars);
extern void		AdslMibUpdateTxStat(
					void					*gDslVars,
					adslConnectionDataStat	*adslTxData,
					adslPerfCounters		*adslTxPerf,
					atmConnectionDataStat	*atmTxData,
					ginpCounters			*pGinpCounters);
extern void XdslMibCmdSnooper (void *gDslVars, dslCommandStruct *cmd);


#ifdef _NOOS
#define AdslMibByteMove(sz,src,dst)		BlockByteMove(sz, (uchar *)(src), (uchar *)(dst))
#define AdslMibByteClear(sz,dst)		BlockByteClear(sz, (uchar *)(dst))
#else
#define AdslMibByteMove(sz,src,dst)		BlockByteMove(sz,(void*) src, (void*) dst)
#define AdslMibByteClear(sz,dst)		BlockByteClear(sz,(void*) dst)
#endif
extern int		AdslMibStrCopy(char *srcPtr, char *dstPtr);
extern int	secElapsedInDay;
/* AdslMibGetData dataId codes */

#define	kAdslMibDataAll					0

extern void		*AdslMibGetData (void *gDslVars, int dataId, void *pAdslMibData);

extern int		AdslMibSetObjectValue (
					void	*gDslVars, 
					uchar	*objId, 
					int		objIdLen,
					uchar	*dataBuf,
					long	*dataBufLen);
extern long		AdslMibGetObjectValue (
					void	*gDslVars, 
					uchar	*objId, 
					int		objIdLen,
					uchar	*dataBuf,
					long	*dataBufLen);

extern Boolean XdslMibIsDataCarryingPath(void *gDslVars, unsigned char pathId, unsigned char dir);
extern Boolean XdslMibIsGinpActive(void *gDslVars, unsigned char dir);
extern Boolean XdslMibIsPhyRActive(void *gDslVars, unsigned char dir);
extern Boolean XdslMibIs2lpActive(void *gDslVars, unsigned char dir);
extern Boolean XdslMibIsAtmConnectionType(void *gDslVars);
extern Boolean XdslMibIsPtmConnectionType(void *gDslVars);

extern int XdslMibGetCurrentMappedPathId(void *gDslVars, unsigned char dir);
extern int XdslMibGetPath0MappedPathId(void *gDslVars, unsigned char dir);
extern int XdslMibGetPath1MappedPathId(void *gDslVars, unsigned char dir);
extern void XdslMibGinpEFTRminReported(void *gDslVars);
extern void XdslMibNotifyBitSwapReq(void *gDslVars, unsigned char dir);
extern void XdslMibNotifyBitSwapRej(void *gDslVars, unsigned char dir);
extern void AdslMibUpdateACTPSD(void *gDslVars, int ACTATP, unsigned char dir);
extern void XdslMibUpdateAttnEtr(void *gDslVars, int dir);
#if defined(CONFIG_BCM_DSL_GFAST)
extern Boolean XdslMibIsFastRetrain(void *gDslVars);
extern unsigned short XdslMibGetToneNumGfast(void * gDslVars);
Public Boolean XdslMibReportAmd4GfastCounters(void *gDslVars);
#endif
#endif	/* AdslMibHeader */
