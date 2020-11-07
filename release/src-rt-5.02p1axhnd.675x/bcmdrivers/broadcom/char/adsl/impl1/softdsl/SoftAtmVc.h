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
 * SoftAtmVc.h 
 *
 * Description:
 *	This file contains ATM VC definitions
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.27 $
 *
 * $Id: SoftAtmVc.h,v 1.27 2004/06/02 22:26:17 ilyas Exp $
 *
 * $Log: SoftAtmVc.h,v $
 * Revision 1.27  2004/06/02 22:26:17  ilyas
 * Added ATM counters for G.992.3
 *
 * Revision 1.26  2004/03/10 22:57:20  ilyas
 * Added I.432 scramling control
 *
 * Revision 1.25  2003/09/23 00:21:59  ilyas
 * Added status to indicate ATM header compression
 *
 * Revision 1.24  2003/08/27 02:00:50  ilyas
 * Original implementation of ATM header compression
 *
 * Revision 1.23  2003/02/25 04:13:15  ilyas
 * Added standard Broadcom header
 *
 * Revision 1.22  2003/01/10 23:25:48  ilyas
 * Added ATM status definition
 *
 * Revision 1.21  2002/09/12 21:07:19  ilyas
 * Added HEC, OCD and LCD counters
 *
 * Revision 1.20  2002/04/02 09:58:00  ilyas
 * Initial implementatoin of BERT
 *
 * Revision 1.19  2001/10/09 22:35:14  ilyas
 * Added more ATM statistics and OAM support
 *
 * Revision 1.18  2001/06/18 19:49:36  ilyas
 * Changes to include support for HOST_ONLY mode
 *
 * Revision 1.17  2001/02/23 05:49:57  ilyas
 * Added routed 1483 encapsulation
 *
 * Revision 1.16  2001/02/09 04:18:18  ilyas
 * Added framer for bridged ethernet PDUs
 *
 * Revision 1.15  2001/02/09 01:55:27  ilyas
 * Added status codes and macros to support printing of AAL packets
 *
 * Revision 1.14  2000/09/21 17:28:35  ilyas
 * Added VBR support to traffic management code, separated UBR to a different
 * Tx list, changed some of the algorithms
 *
 * Revision 1.13  2000/08/23 18:42:13  ilyas
 * Added AAL2, added VcConfigure functions, moved commonly used look-up
 * tables for CRC calculation to AtmLayer
 *
 * Revision 1.12  2000/08/02 03:06:22  ilyas
 * Added support for reserving space in RX packets for ATm protocols
 *
 * Revision 1.11  2000/07/28 17:23:39  ilyas
 * Added ATM connect/disconnect statuses
 *
 * Revision 1.10  2000/07/25 02:16:12  ilyas
 * Added EClip (with Eth to ATM ARP translation) implementation
 *
 * Revision 1.9  2000/07/23 20:57:14  ilyas
 * Added ATM framer and protocol layers
 *
 * Revision 1.8  2000/07/17 21:08:16  lkaplan
 * removed global pointer
 *
 * Revision 1.7  2000/06/09 18:33:04  liang
 * Fixed Irix compiler warnings.
 *
 * Revision 1.6  2000/05/18 21:47:31  ilyas
 * Added detection of preassigned cells such as OAM F4, F5
 *
 * Revision 1.5  2000/05/14 01:50:11  ilyas
 * Added more statuses to ATM code
 *
 * Revision 1.4  2000/05/10 02:41:28  liang
 * Added status report for no cell memory
 *
 * Revision 1.3  2000/05/09 23:00:27  ilyas
 * Added ATM status messages, ATM timer, Tx frames flush on timeout
 * Fixed a bug - adding flushed Tx frames to the list of free Rx frames
 *
 * Revision 1.2  2000/05/03 03:53:00  ilyas
 * Added support for pVc to vcID translation needed for LOG file and other
 * definitions for ATM data in LOG file
 *
 * Revision 1.1  2000/04/19 00:21:35  ilyas
 * Fixed some problems and added Out Of Band (OOB) support to ATM packets
 *
 *
 *****************************************************************************/

#ifndef	SoftAtmVcHeader
#define	SoftAtmVcHeader

/*
**
**		ATM UNI types
**
*/

#define AtmLinkFlags(bMap,name)		(((bMap) >> name##Shift) & name##Mask)

/* ATM service category types */

#define	kAtmSrvcCBR		1	/* Constant Bit Rate	*/
#define	kAtmSrvcVBR		2	/* Variable Bit Rate	*/
#define	kAtmSrvcUBR		4	/* Unspecified Bit Rate	*/
#define	kAtmSrvcABR		8	/* Available Bit Rate	*/
#define	kAtmSrvcUnknown	0xFF

/* ATM AAL types (as encoded at UNI) */

#define	kAtmAalIE		0x58

#define	kAtmRaw			0
#define	kAtmAal1		1
#define	kAtmAal2		2
#define	kAtmAal34		3
#define	kAtmAal5		5
#define	kAtmAalUser		16
#define	kAtmAalUnknown	0xFF

/* ATM AAL1 parameters  */

#define	kAal1SubTypeId			0x85

#define	kAal1TransportShift		0
#define	kAal1TransportMask		0x7

#define	kAal1NullTransport		0
#define	kAal1VoiceTransport		1
#define	kAal1CircuitTransport	2
#define	kAal1AudioTransport		4
#define	kAal1VideoTransport		5


#define	kAal1CBRId				0x86

#define	kAal1CBRShift			24
#define	kAal1CBRMask			0xFF

#define	kAal1CBR64				1
#define	kAal1CBR1544			4		/* DS1 */
#define	kAal1CBR6312			5		/* DS2 */
#define	kAal1CBR32064			6
#define	kAal1CBR44736			7		/* DS3 */
#define	kAal1CBR97728			8
#define	kAal1CBR2048			0x10	/* E1 */
#define	kAal1CBR8448			0x11	/* E2 */
#define	kAal1CBR34368			0x12	/* E3 */
#define	kAal1CBR139264			0x13
#define	kAal1CBR64xN			0x40
#define	kAal1CBR8xN				0x41


#define	kAal1MultiplierId		0x87

#define	kAal1ClockRecoveryId	0x88

#define	kAal1ClockRecoveryShift	3
#define	kAal1ClockRecoveryMask	0x3

#define	kAal1ClockRecoveryNull	1		/* synchronous transport */
#define	kAal1ClockRecoverySRTS	1		/* asynchronous transport */
#define	kAal1ClockRecoveryAdaptive	2


#define	kAal1ECMId				0x89	/* Error correction method */

#define	kAal1ECMShift			(kAal1ClockRecoveryShift + 2)
#define	kAal1ECMMask			0x3

#define	kAal1ECMNull			0
#define	kAal1ECMLossSensitive	1
#define	kAal1ECMDelaySensitive	2


#define	kAal1SDTBlockSizeId		0x8A

#define	kAal1CellFillId			0x8B

/* ATM AAL34 and AAL5 parameters  */

#define	kAalFwdMaxSDUSizeId		0x8C
#define	kAalBacMaxkSDUSizeId	0x81

#define	kAal34MidRangeId		0x82

#define	kAalSSCSTypeId			0x84

#define	kAalSSCSAssured			1
#define	kAalSSCSNonAssured		2
#define	kAalSSCSFrameRelay		4

/* ATM AAL2 parameters  */

#define	kAal2SSNone				0
#define	kAal2SSSAR				1
#define	kAal2SSTED				2
#define	kAal2SSSARMask			3
#define	kAal2SSType1			4
#define	kAal2SSType3			5

typedef struct {
  uchar			aalType;
  union {
	struct {
	  bitMap	aal1Flags;
	  uint	cbrRate;
	  ushort	blkSize;
	  uchar		sarUsed;
	} aal1Params;
	struct {
	  ushort	fwdMaxCpSize;			/* Max "common part" packet size */
	  ushort	backMaxCpSize;
	  ushort	cidLow;
	  ushort	cidHigh;
	  ushort	fwdMaxSsSize;			/* Max "service specific" packet size */
	  ushort	backMaxSsSize;
	  uchar		sscsType;
	} aal2Params;
	struct {
	  ushort	fwdMaxSDUSize;
	  ushort	backMaxSDUSize;
	  ushort	midLow;
	  ushort	midHigh;
	  uchar		sscsType;
	} aal34Params;
	struct {
	  ushort	fwdMaxSDUSize;
	  ushort	backMaxSDUSize;
	  uchar		sscsType;
	} aal5Params;
  } param;
} atmAalParams;

/* ATM Traffic Descriptor types (as encoded at UNI) */

#define	kAtmTrafficIE		0x59

#define	kTrafficFwdPeakCellRateId0	0x82
#define	kTrafficBackPeakCellRateId0	0x83
#define	kTrafficFwdPeakCellRateId	0x84
#define	kTrafficBackPeakCellRateId	0x85

#define	kTrafficFwdSustainCellRateId0	0x88
#define	kTrafficBackSustainCellRateId0	0x89
#define	kTrafficFwdSustainCellRateId	0x90
#define	kTrafficBackSustainCellRateId	0x91

#define	kTrafficFwdMaxBurstSizeId0	0xA0
#define	kTrafficBackMaxBurstSizeId0	0xA1
#define	kTrafficFwdMaxBurstSizeId	0xB0
#define	kTrafficBackMaxBurstSizeId	0xB1

#define	kTrafficBestEffortId		0xBE
#define	kTrafficMgrOptionsId		0xBF

#define	kTrafficMaxTolerance		0x7FFFFFFF

/*	trafficFlags coding */

#define	kTrafficTagFwd				1
#define	kTrafficTagBack				2
#define	kTrafficBestEffort			4

typedef struct {
  uint		tPCR0;					/* CLP = 0, time between cells in us */
  uint		tPCR;					/* CLP = 0+1 */
  uint		tolPCR;					/* tolerance for PCR in us */

  uint		tSCR0;					/* CLP = 0 */
  uint		tSCR;					/* CLP = 0+1 */
  uint		tolSCR;					/* tolerance for SCR in us */

  uchar			atmServiceType;			/* CBR, VBR, UBR, etc. */
  uchar			trafficFlags;
} atmTrafficParams;

/* ATM Broadband Bearer Capabilty (BBC) types (as encoded at UNI) */

#define	kAtmBBCIE			0x5E

#define	kBBCClassShift		0
#define	kBBCClassMask		0x1F

#define	kBBCClassA			0x1
#define	kBBCClassC			0x3
#define	kBBCClassX			0x10


#define	kBBCTrafficShift	(kBBCClassShift + 5)
#define	kBBCTrafficMask		0x7

#define	kBBCTrafficNull		0
#define	kBBCTrafficCBR		1
#define	kBBCTrafficVBR		2


#define	kBBCTimingShift		(kBBCTrafficShift + 3)
#define	kBBCTimingMask		0x3

#define	kBBCTimingNull			0
#define	kBBCTimingRequired		1
#define	kBBCTimingNotRequired	2


#define	kBBCClippingShift	(kBBCTimingShift + 2)
#define	kBBCClippingMask	0x3

#define	kBBCNoClipping		0
#define	kBBCClippingOk		1

#define	kBBCConnectionShift	(kBBCClippingShift + 2)
#define	kBBCConnectionMask	0x3

#define	kBBCPoint2Point		0
#define	kBBCPoint2MPoint	1

/* ATM Broadband High/Low Layer Information (BHLI/BLLI) types (as encoded at UNI) */

#define	kAtmBHLIIE			0x5D
#define	kAtmBLLIIE			0x5F

/* ATM QoS types (as encoded at UNI) */

#define	kAtmQoSIE			0x5C

#define	kQoSNull			0
#define	kQoSClass1			1
#define	kQoSClass2			2
#define	kQoSClass3			3
#define	kQoSClass4			4
#define	kQoSReserved		0xFF

typedef struct {
  uchar				fwdQoSClass;
  uchar				backQoSClass;
} atmQoSParams;

/* ATM MID definitions (ConfigureHandler) */

#define	kAtmMidEntireVc		((uint) -1)

typedef struct {
  void				*pUserVc;			/* VC id from the caller: has to be 1st !!! */
  uint			vci;
  uchar				defaultCLP;			/* default CLP for tx packets on this VC */
  uchar				framerId;
  uchar				protoId;
  uchar				protoRxBytesReserved; /* # bytes reserved by protocol in the beginning of Rx packet */
  uchar				protoTxBytesReserved; /* # bytes reserved by protocol in the beginning of Tx packet */

  atmAalParams		aalParams;
  atmTrafficParams	rxTrafficParams;
  atmTrafficParams	txTrafficParams;
  bitMap			bbcFlags;
  atmQoSParams		qosParams;
} atmVcParams;

/*
**
**		ATM Out of Band (OOB) packet information
**
*/

typedef struct {
  Boolean			clp;				/* Cell Loss Prioroty */
  uchar				aalType;
  union {
	struct {
	  uchar			payloadType;
	} aalRawParams;
	struct {
	  uchar			payloadType;
	  ushort		mid;
	} aal34Params;
	struct {
	  uchar			uui;				/* Uses to user indicator */
	  uchar			cpi;				/* common part indicator */
	} aal5Params;
  } aalParam;
} atmOobPacketInfo;

/*
**
**		ATM setup bit definition
**
*/

#define	kAtmCorrectHecErrors		1
#define	kCorrectHecErrors			kAtmCorrectHecErrors
#define	kAtmPhyHeaderCompression	2
#define	kAtmPhyNoDataScrambling		4

#define	kAtmTxIdleTimeoutMask	0x6
#define	kAtmTxIdleNoTimeout		0
#define	kAtmTxIdleTimeout10s	2
#define	kAtmTxIdleTimeout30s	4
#define	kAtmTxIdleTimeout60s	6

/*
**
**		ATM framer modes and protocol definitions
**
*/

#define	kAtmFramerNone			0
#define	kAtmFramerISO			1
#define	kAtmFramerIP			2
#define	kAtmFramerEth			3
#define	kAtmFramerEthWithCRC	4

#define	kAtmProtoNone			0
#define	kAtmProtoEClip			1
#define kAtmProtoERouted1483	2
#define	kAtmProtoPPP			3


/*
**
**		ATM status codes
**
*/

typedef	void (*atmStatusHandler) (void *gDslVars, uint statusCode, ...);

/* physical layer I.432 */

#define	kAtmStatRxHunt					1
#define	kAtmStatRxPreSync				2
#define	kAtmStatRxSync					3
#define	kAtmStatRxPlOamCell				4
#define	kAtmStatBertResult				5
#define	kAtmStatHec						6
#define	kAtmStatHdrCompr				7
#define	kAtmStatCounters				8
#define	kAtmStatCounters1				9

/* ATM layer */

#define	kAtmLayerStatFirst				100
#define	kAtmStatRxDiscarded				100
#define	kAtmStatTxDelayed				101

#define	kAtmStatVcCreated				102
#define	kAtmStatVcStarted				103
#define	kAtmStatVcStopped				104
#define	kAtmStatVcDeleted				105

#define	kAtmStatTimeout					106
#define	kAtmStatNoCellMemory			107
#define	kAtmStatPrintCell				108
#define kAtmStatInvalidCell				109
#define kAtmStatUnassignedCell			110
#define kAtmStatOamF4SegmentCell		111
#define kAtmStatOamF4End2EndCell		112
#define kAtmStatOamI371Cell				113
#define kAtmStatOamF5SegmentCell		114
#define kAtmStatOamF5End2EndCell		115
#define kAtmStatReservedCell			116

#define	kAtmStatConnected				117
#define	kAtmStatDisconnected			118

#define	kAtmStatRxPacket				119
#define	kAtmStatTxPacket				120

#define	kAtmStatOamLoopback				121


typedef struct _atmPhyCounters {
	ushort						id;
	ushort						bertStatus;
	uint					bertCellTotal;
	uint					bertCellCnt;
	uint					bertBitErrors;

	uint					rxHecCnt;
	uint					rxCellTotal;
	uint					rxCellData;
	uint					rxCellDrop;
	uint					rxOCD;

	uint					txCellTotal;
	uint					txCellData;
} atmPhyCounters;

/* AAL layer */



/*
**
**		ATM log file definitions
**
*/

/* ATM log file flags */

#define		kAtmLogFrameFlagMask		3		/* mask */

#define		kAtmLogFrameFlagNone		0		/* nothing */
#define		kAtmLogFrameFlagNoData		1		/* no data only frame size */
#define		kAtmLogFrameFlagBinData		2		/* data in binary form */
#define		kAtmLogFrameFlagTextData	3		/* data in text form */

#define		kAtmLogSendFrameShift		0
#define		kAtmLogSendFrameNoData		(kAtmLogFrameFlagNoData << kAtmLogSendFrameShift)
#define		kAtmLogSendFrameBinData		(kAtmLogFrameFlagBinData << kAtmLogSendFrameShift)
#define		kAtmLogSendFrameTextData	(kAtmLogFrameFlagTextData << kAtmLogSendFrameShift)

#define		kAtmLogRcvFrameShift		2
#define		kAtmLogRcvFrameNone			(kAtmLogFrameFlagNone << kAtmLogRcvFrameShift)
#define		kAtmLogRcvFrameNoData		(kAtmLogFrameFlagNoData << kAtmLogRcvFrameShift)
#define		kAtmLogRcvFrameBinData		(kAtmLogFrameFlagBinData << kAtmLogRcvFrameShift)
#define		kAtmLogRcvFrameTextData		(kAtmLogFrameFlagTextData << kAtmLogRcvFrameShift)		

#define		kAtmLogSendCompleteFrameShift	4
#define		kAtmLogSendCompleteFrameNone	(kAtmLogFrameFlagNone << kAtmLogSendCompleteFrameShift)
#define		kAtmLogSendCompleteFrameNoData	(kAtmLogFrameFlagNoData << kAtmLogSendCompleteFrameShift)

#define		kAtmLogReturnFrameShift		6
#define		kAtmLogReturnFrameNoData	(kAtmLogFrameFlagNoData << kAtmLogReturnFrameShift)

#define		kAtmLogCellFlag				(1 << 8)

/* ATM log codes */

#define		kAtmLogSendFrame					1
#define		kAtmLogRcvFrame						2
#define		kAtmLogSendFrameComplete			3
#define		kAtmLogReturnFrame					4
#define		kAtmLogVcAllocate					5
#define		kAtmLogVcFree						6
#define		kAtmLogVcActivate					7
#define		kAtmLogVcDeactivate					8
#define		kAtmLogTimer						9
#define		kAtmLogCell							10
#define		kAtmLogVcConfigure					11

#define		kAtmLogRxCellHeader					12
#define		kAtmLogRxCellData					13
#define		kAtmLogTxCell						14

#endif	/* SoftAtmVcHeader */
