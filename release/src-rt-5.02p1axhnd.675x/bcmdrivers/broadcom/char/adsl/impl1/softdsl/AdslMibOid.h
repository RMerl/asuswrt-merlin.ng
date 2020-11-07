/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/****************************************************************************
 *
 * AdslMibOid.h 
 *
 * Description:
 *	SNMP object identifiers for ADSL MIB and other related MIBs
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.9 $
 *
 * $Id: AdslMibOid.h,v 1.9 2007/11/20 19:58:31 dadityan Exp $
 *
 * $Log: AdslMibOid.h,v $
 * Revision 1.9  2007/11/20 19:58:31  dadityan
 * Added OIDs for VDSL MIB Band Plan
 *
 * Revision 1.8  2007/09/04 07:21:15  tonytran
 * PR31097: 1_28_rc8
 *
 * Revision 1.7  2006/09/13 22:07:11  dadityan
 * Added Mib Oid for Adsl Phy Cfg
 *
 * Revision 1.5  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.4  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.3  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.2  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#ifndef	AdslMibOidHeader
#define	AdslMibOidHeader

#define kOidAdsl							94
#define kOidAdslInterleave					124
#define kOidAdslFast						125
#define kOidAtm								37
#define kOidAdslPhyCfg						95

#define kOidAdslLine						1
#define kOidAdslMibObjects					1

#define kOidAdslLineTable					1
#define kOidAdslLineEntry					1
#define kOidAdslLineCoding					1
#define kOidAdslLineType					2
#define kOidAdslLineSpecific			    3
#define kOidAdslLineConfProfile				4
#define kOidAdslLineAlarmConfProfile		5

#define kOidAdslAtucPhysTable				2
#define kOidAdslAturPhysTable				3
#define kOidAdslPhysEntry					1
#define kOidAdslPhysInvSerialNumber     	1
#define kOidAdslPhysInvVendorID             2
#define kOidAdslPhysInvVersionNumber    	3
#define kOidAdslPhysCurrSnrMgn          	4
#define kOidAdslPhysCurrAtn             	5
#define kOidAdslPhysCurrStatus          	6
#define kOidAdslPhysCurrOutputPwr       	7
#define kOidAdslPhysCurrAttainableRate  	8

#define kOidAdslAtucChanTable				4
#define kOidAdslAturChanTable				5
#define kOidAdslChanEntry					1
#define kOidAdslChanInterleaveDelay			1
#define kOidAdslChanCurrTxRate				2
#define kOidAdslChanPrevTxRate          	3
#define kOidAdslChanCrcBlockLength      	4

#define kOidAdslAtucPerfDataTable			6
#define kOidAdslAturPerfDataTable			7
#define kOidAdslPerfDataEntry				1
#define kOidAdslPerfLofs                 	1
#define kOidAdslPerfLoss                 	2
#define kOidAdslPerfLprs                 	3
#define kOidAdslPerfESs                  	4
#define kOidAdslPerfValidIntervals          5
#define kOidAdslPerfInvalidIntervals     	6
#define kOidAdslPerfCurr15MinTimeElapsed 	7
#define kOidAdslPerfCurr15MinLofs        	8
#define kOidAdslPerfCurr15MinLoss        	9
#define kOidAdslPerfCurr15MinLprs        	10
#define kOidAdslPerfCurr15MinESs         	11
#define kOidAdslPerfCurr1DayTimeElapsed     12
#define kOidAdslPerfCurr1DayLofs         	13
#define kOidAdslPerfCurr1DayLoss         	14
#define kOidAdslPerfCurr1DayLprs         	15
#define kOidAdslPerfCurr1DayESs          	16
#define kOidAdslPerfPrev1DayMoniSecs     	17
#define kOidAdslPerfPrev1DayLofs         	18
#define kOidAdslPerfPrev1DayLoss            19
#define kOidAdslPerfPrev1DayLprs         	20
#define kOidAdslPerfPrev1DayESs          	21

#define kOidAdslAtucPerfIntervalTable		8
#define kOidAdslAturPerfIntervalTable		9
#define kOidAdslPerfIntervalEntry			1
#define kOidAdslIntervalNumber				1
#define kOidAdslIntervalLofs				2
#define kOidAdslIntervalLoss				3
#define kOidAdslIntervalLprs				4
#define kOidAdslIntervalESs					5
#define kOidAdslIntervalValidData			6

#define kOidAdslAtucChanPerfTable					10
#define kOidAdslAturChanPerfTable					11
#define kOidAdslChanPerfEntry						1
#define kOidAdslChanReceivedBlks                 	1
#define kOidAdslChanTransmittedBlks              	2
#define kOidAdslChanCorrectedBlks                	3
#define kOidAdslChanUncorrectBlks                	4
#define kOidAdslChanPerfValidIntervals           	5
#define kOidAdslChanPerfInvalidIntervals         	6
#define kOidAdslChanPerfCurr15MinTimeElapsed     	7
#define kOidAdslChanPerfCurr15MinReceivedBlks    	8
#define kOidAdslChanPerfCurr15MinTransmittedBlks 	9
#define kOidAdslChanPerfCurr15MinCorrectedBlks   	10
#define kOidAdslChanPerfCurr15MinUncorrectBlks   	11
#define kOidAdslChanPerfCurr1DayTimeElapsed      	12
#define kOidAdslChanPerfCurr1DayReceivedBlks     	13
#define kOidAdslChanPerfCurr1DayTransmittedBlks  	14
#define kOidAdslChanPerfCurr1DayCorrectedBlks    	15
#define kOidAdslChanPerfCurr1DayUncorrectBlks    	16
#define kOidAdslChanPerfPrev1DayMoniSecs         	17
#define kOidAdslChanPerfPrev1DayReceivedBlks     	18
#define kOidAdslChanPerfPrev1DayTransmittedBlks  	19
#define kOidAdslChanPerfPrev1DayCorrectedBlks    	20
#define kOidAdslChanPerfPrev1DayUncorrectBlks    	21

#define kOidAdslAtucChanIntervalTable				12
#define kOidAdslAturChanIntervalTable				13
#define kOidAdslChanIntervalEntry					1
#define kOidAdslChanIntervalNumber					1
#define kOidAdslChanIntervalReceivedBlks        	2
#define kOidAdslChanIntervalTransmittedBlks     	3
#define kOidAdslChanIntervalCorrectedBlks       	4
#define kOidAdslChanIntervalUncorrectBlks       	5
#define kOidAdslChanIntervalValidData           	6

/* AdslExtra OIDs for kOidAdslPrivate -> kOidAdslPrivExtraInfo (defined in AdslMibDef.h) */

#define kOidAdslExtraConnectionInfo					1
#define kOidAdslExtraConnectionStat					2
#define kOidAdslExtraFramingMode					3
#define kOidAdslExtraTrainingState					4
#define kOidAdslExtraNonStdFramingAdjustK			5
#define kOidAdslExtraAtmStat						6
#define kOidAdslExtraDiagModeData					7
#define kOidAdslExtraAdsl2Info						8
#define kOidAdslExtraTxPerfCounterInfo				9
#define kOidAdslExtraVdsl2Info						13		/* 10, 11 and 12 are already being defined for kOidAdslExtraNLInfo, */
#define kOidAdslExtraDiagModeDataVdsl				14		/* kOidAdslExtraPLNInfo and kOidAdslExtraPLNData in AdslMibDef.h */
#define kOidAdslPrivBandPlanUSNeg       15
#define kOidAdslPrivBandPlanUSPhy       16
#define kOidAdslPrivBandPlanDSNeg       17
#define kOidAdslPrivBandPlanDSPhy       18
#define kOidAdslPrivBandPlanUSNegDiscovery	19
#define kOidAdslPrivBandPlanUSPhyDiscovery	20
#define kOidAdslPrivBandPlanDSNegDiscovery	21
#define kOidAdslPrivBandPlanDSPhyDiscovery	22
#define kOidAdslExtraXdslInfo					23

#define kOidAtmMibObjects		1
#define kOidAtmTcTable			4
#define kOidAtmTcEntry			1
#define kOidAtmOcdEvents		1
#define kOidAtmAlarmState		2

#endif	/* AdslMibOidHeader */
