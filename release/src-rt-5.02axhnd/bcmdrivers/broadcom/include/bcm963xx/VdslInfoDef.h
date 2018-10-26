/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 *
 * Description:
 *	Vdsl info defs
 *
 * Authors: Ben Bradshaw
 *
 *****************************************************************************/

#ifndef	VdslInfoDefHeader
#define	VdslInfoDefHeader

#include "AdslMibDef.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* 
**
**		VDSL configuration parameters 
**
*/


/* Bandplan parameters */

#define kVdslCfgBandplanMask 			    0x00000003
#define kVdslCfg3Band					    0x00000000
#define kVdslCfg4Band				        0x00000001
#define kVdslCfg5Band				        0x00000002
#define kVdslCfgUS0 					    0x00000003

/* Single/dual latency parameters */

#define kVdslCfgLatencyMask 			    0x00000030
#define kVdslCfgSingleLatency		        0x00000010
#define kVdslCfgDualLatency				    0x00000020

typedef struct _vdslCfgProfile {
	long		vdslParam;
} vdslCfgProfile;

/* 
**
**		VDSL PHY configuration
**
*/

typedef struct _vdslPhyCfg {
	long		demodCapMask;
	long		demodCap;
} vdslPhyCfg;

/* 
**
**		VDSL version info parameters 
**
*/

#define	kVdslVersionStringSize				64

#define	kVdslTypeUnknown					0
#define	kAdslTypeVDSL						1

typedef struct _vdslVersionInfo {
	unsigned short	phyType;
	unsigned short	phyMjVerNum;
	unsigned short	phyMnVerNum;
	char			phyVerStr[kVdslVersionStringSize];
	unsigned short	drvMjVerNum;
	unsigned short	drvMnVerNum;
	char			drvVerStr[kVdslVersionStringSize];
} vdslVersionInfo;


/* For PTM mode vp/vc configuration */
typedef struct 
{
  int portId ;
  int vpi;
  int vci;
} vdslVpiVciInfo;


/* 
**
**		VDSL self-test parameters 
**
*/

#define kVdslSelfTestInProgress				0x40000000
#define kVdslSelfTestCompleted				0x80000000

/* MIB OID's for VDSL objects */

#define kOidMaxObjLen						80

#define kOidVdsl							94
#define kOidVdslInterleave					124
#define kOidVdslFast						125
#define kOidAtm								37
#define kOidVdslPrivate						255
#define kOidVdslPrivatePartial				254

#define kOidVdslPrivSNR						1
#define kOidVdslPrivBitAlloc				2
#define kOidVdslPrivGain					3
#define kOidVdslPrivShowtimeMargin			4
#define kOidVdslPrivChanCharLin				5
#define kOidVdslPrivChanCharLog				6
#define kOidVdslPrivQuietLineNoise			7
#define kOidVdslPrivExtraInfo				255

#define kOidVdslLine						1
#define kOidVdslMibObjects					1

#define kOidVdslLineTable					1
#define kOidVdslLineEntry					1
#define kOidVdslLineCoding					1
#define kOidVdslLineType					2
#define kOidVdslLineSpecific			    3
#define kOidVdslLineConfProfile				4
#define kOidVdslLineAlarmConfProfile		5

#define kOidVdslAtucPhysTable				2
#define kOidVdslAturPhysTable				3
#define kOidVdslPhysEntry					1
#define kOidVdslPhysInvSerialNumber     	1
#define kOidVdslPhysInvVendorID             2
#define kOidVdslPhysInvVersionNumber    	3
#define kOidVdslPhysCurrSnrMgn          	4
#define kOidVdslPhysCurrAtn             	5
#define kOidVdslPhysCurrStatus          	6
#define kOidVdslPhysCurrOutputPwr       	7
#define kOidVdslPhysCurrAttainableRate  	8

#define kOidVdslAtucChanTable				4
#define kOidVdslAturChanTable				5
#define kOidVdslChanEntry					1
#define kOidVdslChanInterleaveDelay			1
#define kOidVdslChanCurrTxRate				2
#define kOidVdslChanPrevTxRate          	3
#define kOidVdslChanCrcBlockLength      	4

#define kOidVdslAtucPerfDataTable			6
#define kOidVdslAturPerfDataTable			7
#define kOidVdslPerfDataEntry				1
#define kOidVdslPerfLofs                 	1
#define kOidVdslPerfLoss                 	2
#define kOidVdslPerfLprs                 	3
#define kOidVdslPerfESs                  	4
#define kOidVdslPerfValidIntervals          5
#define kOidVdslPerfInvalidIntervals     	6
#define kOidVdslPerfCurr15MinTimeElapsed 	7
#define kOidVdslPerfCurr15MinLofs        	8
#define kOidVdslPerfCurr15MinLoss        	9
#define kOidVdslPerfCurr15MinLprs        	10
#define kOidVdslPerfCurr15MinESs         	11
#define kOidVdslPerfCurr1DayTimeElapsed     12
#define kOidVdslPerfCurr1DayLofs         	13
#define kOidVdslPerfCurr1DayLoss         	14
#define kOidVdslPerfCurr1DayLprs         	15
#define kOidVdslPerfCurr1DayESs          	16
#define kOidVdslPerfPrev1DayMoniSecs     	17
#define kOidVdslPerfPrev1DayLofs         	18
#define kOidVdslPerfPrev1DayLoss            19
#define kOidVdslPerfPrev1DayLprs         	20
#define kOidVdslPerfPrev1DayESs          	21

#define kOidVdslAtucPerfIntervalTable		8
#define kOidVdslAturPerfIntervalTable		9
#define kOidVdslPerfIntervalEntry			1
#define kOidVdslIntervalNumber				1
#define kOidVdslIntervalLofs				2
#define kOidVdslIntervalLoss				3
#define kOidVdslIntervalLprs				4
#define kOidVdslIntervalESs					5
#define kOidVdslIntervalValidData			6

#define kOidVdslAtucChanPerfTable					10
#define kOidVdslAturChanPerfTable					11
#define kOidVdslChanPerfEntry						1
#define kOidVdslChanReceivedBlks                 	1
#define kOidVdslChanTransmittedBlks              	2
#define kOidVdslChanCorrectedBlks                	3
#define kOidVdslChanUncorrectBlks                	4
#define kOidVdslChanPerfValidIntervals           	5
#define kOidVdslChanPerfInvalidIntervals         	6
#define kOidVdslChanPerfCurr15MinTimeElapsed     	7
#define kOidVdslChanPerfCurr15MinReceivedBlks    	8
#define kOidVdslChanPerfCurr15MinTransmittedBlks 	9
#define kOidVdslChanPerfCurr15MinCorrectedBlks   	10
#define kOidVdslChanPerfCurr15MinUncorrectBlks   	11
#define kOidVdslChanPerfCurr1DayTimeElapsed      	12
#define kOidVdslChanPerfCurr1DayReceivedBlks     	13
#define kOidVdslChanPerfCurr1DayTransmittedBlks  	14
#define kOidVdslChanPerfCurr1DayCorrectedBlks    	15
#define kOidVdslChanPerfCurr1DayUncorrectBlks    	16
#define kOidVdslChanPerfPrev1DayMoniSecs         	17
#define kOidVdslChanPerfPrev1DayReceivedBlks     	18
#define kOidVdslChanPerfPrev1DayTransmittedBlks  	19
#define kOidVdslChanPerfPrev1DayCorrectedBlks    	20
#define kOidVdslChanPerfPrev1DayUncorrectBlks    	21

#define kOidVdslAtucChanIntervalTable				12
#define kOidVdslAturChanIntervalTable				13
#define kOidVdslChanIntervalEntry					1
#define kOidVdslChanIntervalNumber					1
#define kOidVdslChanIntervalReceivedBlks        	2
#define kOidVdslChanIntervalTransmittedBlks     	3
#define kOidVdslChanIntervalCorrectedBlks       	4
#define kOidVdslChanIntervalUncorrectBlks       	5
#define kOidVdslChanIntervalValidData           	6

#define kOidAtmMibObjects		1
#define kOidAtmTcTable			4
#define kOidAtmTcEntry			1
#define kOidAtmOcdEvents		1
#define kOidAtmAlarmState		2

/* Vdsl Channel coding */

#define	kVdslRcvDir			0
#define	kVdslXmtDir			1

#define	kVdslRcvActive		(1 << kVdslRcvDir)
#define	kVdslXmtActive		(1 << kVdslXmtDir)

#define	kVdslIntlChannel	0
#define	kVdslFastChannel	1

#define	kVdslTrellisOff		0
#define	kVdslTrellisOn		1

/* AnnexC modulation and bitmap types for the field (vdslConnection.modType) */

#define kVdslModMask		0x7

#define	kVdslModGdmt		0

/* VdslLineCodingType definitions */

#define kVdslLineCodingOther		1
#define kVdslLineCodingDMT			2
#define kVdslLineCodingCAP			3
#define kVdslLineCodingQAM			4

/* VdslLineType definitions */

#define kVdslLineTypeNoChannel		1
#define kVdslLineTypeFastOnly		2
#define kVdslLineTypeIntlOnly		3
#define kVdslLineTypeFastOrIntl		4
#define kVdslLineTypeFastAndIntl	5

typedef struct _vdslLineEntry {
	unsigned char	vdslLineCoding;
	unsigned char	vdslLineType;
} vdslLineEntry;


/* VdslPhys status definitions */

#define kVdslPhysStatusNoDefect		(1 << 0)
#define kVdslPhysStatusLOF			(1 << 1)	/* lossOfFraming (not receiving valid frame) */
#define kVdslPhysStatusLOS			(1 << 2)	/* lossOfSignal (not receiving signal) */
#define kVdslPhysStatusLPR			(1 << 3)	/* lossOfPower */
#define kVdslPhysStatusLOSQ			(1 << 4)	/* lossOfSignalQuality */
#define kVdslPhysStatusLOM			(1 << 5)	/* lossOfMargin */

typedef struct _vdslPhysEntry {
	long		vdslCurrSnrMgn;
	long		vdslCurrAtn;
	long		vdslCurrStatus;
	long		vdslCurrOutputPwr;
	long		vdslCurrAttainableRate;
} vdslPhysEntry;

#define kVdslPhysVendorIdLen		8
#define kVdslPhysSerialNumLen		32
#define kVdslPhysVersionNumLen		32

typedef struct _vdslFullPhysEntry {
	char		vdslSerialNumber[kVdslPhysSerialNumLen];
	char		vdslVendorID[kVdslPhysVendorIdLen];
	char		vdslVersionNumber[kVdslPhysVersionNumLen];
	long		vdslCurrSnrMgn;
	long		vdslCurrAtn;
	long		vdslCurrStatus;
	long		vdslCurrOutputPwr;
	long		vdslCurrAttainableRate;
} vdslFullPhysEntry;

/* Vdsl channel entry definitions */

typedef struct _vdslChanEntry {
    unsigned long		vdslChanIntlDelay;
	unsigned long		vdslChanCurrTxRate;
	unsigned long		vdslChanPrevTxRate;
	unsigned long		vdslChanCrcBlockLength;
} vdslChanEntry;

/* Vdsl performance data definitions */
typedef struct _vdslAnomCounters {
	unsigned long		vdsl_NE_CV;                    /* Near end superframe CRC-8 anomaly count */
	unsigned long		vdsl_NE_FEC;                   /* Near end Codeword correction count */ 
	unsigned long		vdsl_NE_uncorrectableCodeword; /* Uncorrectable codeword count */ 
	unsigned long		vdsl_FE_CV;                    /* FEBE count. Far end superframe CRC-8 anomaly count */
	unsigned long		vdsl_FE_FEC;                   /* Number of super-frames during which at least one codeword correction 
                                                          occurred */ 
} vdslAnomalyCounters;

typedef struct _vdslDerivedCounters {
	unsigned long		vdslLofs;
	unsigned long		vdslLoss;
	unsigned long		vdslLols;	/* Loss of Link failures (ATUC only) */
	unsigned long		vdslLprs;
	unsigned long		vdslESs;	/* Count of Errored Seconds */
	unsigned long		vdslInits;	/* Count of Line initialization attempts (ATUC only) */
	unsigned long		vdslUAS;	/* Count of Unavailable Seconds */
	unsigned long		vdslSES;	/* Count of Severely Errored Seconds */
	unsigned long		vdslLOSS;	/* Count of LOS seconds */
	unsigned long		vdslFECs;	/* Count of FEC seconds  */
} vdslDerivedCounters;

# define NE 0                   /* Near end */
# define FE 1                   /* Far end */

# define B0 0                   /* Bearer 0 */
# define B1 1                   /* Bearer 1 */

typedef struct _ptmConnectionStat {
	unsigned long			cntCVprio0;
	unsigned long			cntCVprio1;
	unsigned long			cntRxCellDropPrio0;
	unsigned long			cntRxCellDropPrio1;
        unsigned long                   cntRxPktPrio0;
        unsigned long                   cntRxPktPrio1;
        unsigned long                   cntTxPktPrio0;
        unsigned long                   cntTxPktPrio1;
        unsigned long                   cntTxFlushPrio0;
        unsigned long                   cntTxFlushPrio1;
} ptmConnectionStat;

typedef struct _vdslPerfCounters {
    vdslDerivedCounters   vdslDerivCounters[2]; /* NE and FE counts */
    vdslAnomalyCounters   vdslAnomCounters[2]; /* B0 and B1 counts */
	atmConnectionStat	  atmStat[2];          /* B0 and B1 */ /* Used */
	ptmConnectionStat	  ptmStat[2];          /* B0 and B1 */ /* Used */
} vdslPerfCounters;

typedef struct _vdslPerfDataEntry {
	vdslPerfCounters	perfTotal;
	unsigned long				vdslPerfValidIntervals;
	unsigned long				vdslPerfInvalidIntervals;
	vdslPerfCounters	perfCurr15Min;
	unsigned long				vdslPerfCurr15MinTimeElapsed;
	vdslPerfCounters	perfCurr1Day;
	unsigned long				vdslPerfCurr1DayTimeElapsed;
	vdslPerfCounters	perfPrev15Min;
	unsigned long				vdslPerfPrev15MinTimeElapsed;
	vdslPerfCounters	perfPrev1Day;
	unsigned long				vdslAturPerfPrev1DayMoniSecs;
} vdslPerfDataEntry;

#define kVdslMibPerfIntervals		4

/* Vdsl channel performance data definitions */

typedef struct _vdslChanCounters {
	unsigned long		vdslChanReceivedBlks;
	unsigned long		vdslChanTransmittedBlks;
	unsigned long		vdslChanCorrectedBlks;
	unsigned long		vdslChanUncorrectBlks;
} vdslChanCounters;

typedef struct _vdslChanPerfDataEntry {
	vdslChanCounters	perfTotal;
	unsigned long				vdslChanPerfValidIntervals;
	unsigned long				vdslChanPerfInvalidIntervals;
	vdslChanCounters	perfCurr15Min;
	unsigned long				vdslPerfCurr15MinTimeElapsed;
	vdslChanCounters	perfCurr1Day;
	unsigned long				vdslPerfCurr1DayTimeElapsed;
	vdslChanCounters	perfPrev1Day;
	unsigned long				vdslAturPerfPrev1DayMoniSecs;
} vdslChanPerfDataEntry;

#define kVdslMibChanPerfIntervals	4

/* Vdsl trap threshold definitions */

#define	kVdslEventLinkChange		0x001
#define	kVdslEventRateChange		0x002
#define	kVdslEventLofThresh			0x004
#define	kVdslEventLosThresh			0x008
#define	kVdslEventLprThresh			0x010
#define	kVdslEventESThresh			0x020
#define	kVdslEventFastUpThresh		0x040
#define	kVdslEventIntlUpThresh		0x080
#define	kVdslEventFastDownThresh	0x100
#define	kVdslEventIntlDwonThresh	0x200

typedef struct _vdslThreshCounters {
	unsigned long		vdslThreshLofs;
	unsigned long		vdslThreshLoss;
	unsigned long		vdslThreshLols;	/* Loss of Link failures (ATUC only) */
	unsigned long		vdslThreshLprs;
	unsigned long		vdslThreshESs;
	unsigned long		vdslThreshFastRateUp;
	unsigned long		vdslThreshIntlRateUp;
	unsigned long		vdslThreshFastRateDown;
	unsigned long		vdslThreshIntlRateDown;
} vdslThreshCounters;


/* Atm PHY performance data definitions */
#if 0
#define	kAtmPhyStateNoAlarm			1
#define	kAtmPhyStateLcdFailure		2

typedef struct _atmPhyDataEntrty {
	unsigned long		atmInterfaceOCDEvents;
	unsigned long		atmInterfaceTCAlarmState;
} atmPhyDataEntrty;
#endif

typedef struct _vdslBertResults {
	unsigned long		bertTotalBits;
	unsigned long		bertErrBits;
} vdslBertResults;

#if 0
typedef struct {
	unsigned long		cntHi;
	unsigned long		cntLo;
} cnt64;
#endif

typedef struct _vdslBertStatusEx {
	unsigned long		bertSecTotal;
	unsigned long		bertSecElapsed;
	unsigned long		bertSecCur;
	cnt64				bertTotalBits;
	cnt64				bertErrBits;
} vdslBertStatusEx;

typedef struct _vdslDataConnectionInfo {
	unsigned short		K;
	unsigned char		S, R, D;
} vdslDataConnectionInfo;

typedef struct _vdslConnectionInfo {
	unsigned char			chType;				/* fast or interleaved */
	unsigned char			modType;			/* modulation type: G.DMT or T1.413 */
	unsigned char			trellisCoding;		/* off(0) or on(1) */
	vdslDataConnectionInfo	rcvInfo;
	vdslDataConnectionInfo	xmtInfo;
} vdslConnectionInfo;

typedef struct _vdslConnectionDataStat {
	unsigned long			cntRS;	
	unsigned long			cntRSCor;	
	unsigned long			cntRSUncor;	
	unsigned long			cntSF;	
	unsigned long			cntSFErr;	
} vdslConnectionDataStat;

typedef struct _vdslConnectionStat {
	vdslConnectionDataStat	rcvStat;
	vdslConnectionDataStat	xmtStat;
} vdslConnectionStat;

#define	kVdslFramingModeMask			0x0F
#define	kAtmFramingModeMask				0xF0
#define	kAtmHeaderCompression			0x80

/* VdslMibGetObjectValue return codes */

#define	kVdslMibStatusSuccess			0
#define	kVdslMibStatusFailure			-1
#define	kVdslMibStatusNoObject			-2
#define	kVdslMibStatusObjectInvalid		-3
#define	kVdslMibStatusBufferTooSmall	-4
#define	kVdslMibStatusLastError			-4

/* Vdsl training codes */

#define	kVdslTrainingIdle				0
#define	kVdslTrainingG994				1
#define	kVdslTrainingG992Started		2
#define	kVdslTrainingG992ChanAnalysis	3
#define	kVdslTrainingG992Exchange		4
#define	kVdslTrainingConnected			5

/* Global info structure */

typedef struct _vdslInfo {
	vdslLineEntry			vdslLine;
	vdslPhysEntry			vdslPhys;
	vdslChanEntry			vdslChanIntl;
	vdslChanEntry			vdslChanFast;
 	vdslPerfDataEntry		vdslPerfData; /* Used */
	vdslPerfCounters		vdslPerfIntervals[kVdslMibPerfIntervals];
	vdslChanPerfDataEntry	vdslChanIntlPerfData;
	vdslChanPerfDataEntry	vdslChanFastPerfData;
	vdslChanCounters		vdslChanIntlPerfIntervals[kVdslMibChanPerfIntervals];
	vdslChanCounters		vdslChanFastPerfIntervals[kVdslMibChanPerfIntervals];

	vdslThreshCounters		vdslAlarm;

	atmPhyDataEntrty		vdslChanIntlAtmPhyData;
	atmPhyDataEntrty		vdslChanFastAtmPhyData;

	vdslBertResults			vdslBertRes;

	vdslConnectionInfo		vdslConnection;
	vdslConnectionStat		vdslStat;
	unsigned char			vdslTrainingState;

	vdslFullPhysEntry		vdslAtucPhys;
	unsigned char			vdslRxNonStdFramingAdjustK;
	unsigned char			vdslFramingMode;
	vdslBertStatusEx		vdslBertStatus;
	long					afeRxPgaGainQ1;

	vdslPerfCounters		vdslTxPerfTotal;
} vdslInfo;

#if defined(__cplusplus)
}
#endif

#endif	/* VdslInfoDefHeader */
