/*
<:copyright-BRCM:2004:DUAL/GPL:standard

   Copyright (c) 2004 Broadcom 
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
 * AdslMibOid.h 
 *
 * Description:
 *  SNMP object identifiers for ADSL MIB and other related MIBs
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.35 $
 *
 * $Id: AdslMibDef.h,v 1.35 2008/10/03 22:49:02 tonytran Exp $
 *
 * $Log: AdslMibDef.h,v $
 * Revision 1.35  2008/10/03 22:49:02  tonytran
 * Added the display of Annex type, selected profile name, and connection type to State Windows
 *
 * Revision 1.34  2008/09/07 15:48:45  ovandewi
 * define mask1 for 30a
 *
 * Revision 1.33  2007/12/05 06:10:31  dadityan
 * Tx Output Pwr obj VDSL
 *
 * Revision 1.29  2007/09/04 07:21:15  tonytran
 * PR31097: 1_28_rc8
 *
 * Revision 1.28  2007/08/20 18:21:42  tonytran
 * Added the display of Vdsl2 modulation string in Diags State Windows
 *
 * Revision 1.27  2007/04/11 23:46:13  tonytran
 * Changed the way DS of N, INP and Delay are displayed when phyR is enabled; Also, the rate will be displayed from MIB var
 *
 * Revision 1.26  2007/02/15 23:27:19  tonytran
 * Fixed output log file access problem after log=0 or completed. Added profstart/profstop commands and display FIRE counters for US
 *
 * Revision 1.24  2007/01/11 09:14:04  tonytran
 * Fixed the set phy cfg and bitswap counter problems; Removed FAST_DEC_DISABLE from phy cfg
 *
 * Revision 1.22  2006/04/03 15:30:27  ovandewi
 * fix structure name and types
 *
 * Revision 1.20  2006/03/31 20:26:36  dadityan
 * PLN and NL Data
 *
 * Revision 1.19  2005/08/11 15:08:38  ilyas
 * Added string scrambling checking and AnnexL/M submodes display
 *
 * Revision 1.18  2005/04/01 21:57:42  ilyas
 * Added definition for PwmClock (ADSL driver)
 *
 * Revision 1.17  2004/07/27 19:24:40  ilyas
 * Added AnnexM configuration option
 *
 * Revision 1.16  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.15  2004/05/25 16:15:04  ilyas
 * Added ADSL2 framing status
 *
 * Revision 1.14  2004/03/31 19:09:48  ilyas
 * Added ADSL2+ modulation control
 *
 * Revision 1.13  2004/03/03 20:14:05  ilyas
 * Merged changes for ADSL2+ from ADSL driver
 *
 * Revision 1.12  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.11  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.10  2003/09/29 18:39:51  ilyas
 * Added new definitions for AnnexI
 *
 * Revision 1.9  2003/07/18 19:14:34  ilyas
 * Merged with ADSL driver
 *
 * Revision 1.8  2003/07/08 18:34:16  ilyas
 * Added fields to adsl configuration structure
 *
 * Revision 1.7  2003/03/25 00:07:00  ilyas
 * Added "long" BERT supprt
 *
 * Revision 1.6  2003/02/27 07:10:52  ilyas
 * Added more configuration and status parameters (for EFNT)
 *
 * Revision 1.5  2003/01/23 20:29:37  ilyas
 * Added structure for ADSL PHY configuration command
 *
 * Revision 1.4  2002/11/13 21:32:49  ilyas
 * Added adjustK support for Centillium non-standard framing mode
 *
 * Revision 1.3  2002/10/31 01:35:50  ilyas
 * Fixed size of K for S=1/2
 *
 * Revision 1.2  2002/10/05 03:28:31  ilyas
 * Added extra definitions for Linux and VxWorks drivers.
 * Added definitions for SelfTest support
 *
 * Revision 1.1  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/
#if !defined(_CFE_) && !defined(_NOOS)
#include <linux/version.h>
#endif

#ifndef AdslMibDefHeader
#define AdslMibDefHeader

#include "SeltDef.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define  ADSLMIBDEF_H_VER  3

#if !defined( __GNUC__) && defined(__arm)
#pragma anon_unions
#endif

//#define SAVE_CRATESRA_MSG
//#define CO_G994_NSIF
//#define SUPPORT_24HR_CNT_STAT

#ifndef CONFIG_VDSL_SUPPORTED
#if defined(CONFIG_BCM963268) || defined(CHIP_63268) || defined(CONFIG_BCM963138) || defined(CHIP_63138) ||  \
	  defined(CONFIG_BCM963148) || defined(CHIP_63148) || defined(CONFIG_BCM963381) || defined(CHIP_63381) || \
	  defined(CONFIG_BCM963158) || defined(CHIP_63158) || \
	  defined(DMP_VDSL2WAN_1)
#define CONFIG_VDSL_SUPPORTED
#endif
#endif

#if defined(CONFIG_VDSL_SUPPORTED)
#define SUPPORT_VECTORING
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
#define US_GFAST_TONE_OFFSET    2048
#endif
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963158)
#ifndef CONFIG_VDSLBRCMPRIV1_SUPPORT
#define CONFIG_VDSLBRCMPRIV1_SUPPORT
#endif
#ifdef CONFIG_BCM963158
#ifndef CONFIG_VDSLBRCMPRIV2_SUPPORT
#define CONFIG_VDSLBRCMPRIV2_SUPPORT
#endif
#endif
#endif /* defined(CONFIG_BCM963138) || defined(CONFIG_BCM963158) */

#ifndef CONFIG_TOD_SUPPORTED
#if defined(CONFIG_BCM963138) || defined(CHIP_63138) || defined(CONFIG_BCM963148) || defined(CHIP_63148) || \
	defined(CONFIG_BCM963381) || defined(CHIP_63381) || defined(CONFIG_BCM963158) || defined(CHIP_63158)
#define CONFIG_TOD_SUPPORTED
#endif
#endif

#ifndef CONFIG_RNC_SUPPORT
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
#define CONFIG_RNC_SUPPORT
#endif
#endif

#if defined(CONFIG_BCM963158)
#ifndef SECONDARY_AFEID_FN
#define SECONDARY_AFEID_FN	"/data/afeid_override"
#endif
#endif

/*
**
**      ADSL configuration parameters 
**
*/

#define kAdslCfgModMask                     (0x00000007 | 0x0000F000)
#define kAdslCfgModAny                      0x00000000

#define kAdslCfgModGdmtOnly                 0x00000001
#define kAdslCfgModGliteOnly                0x00000002
#define kAdslCfgModT1413Only                0x00000004
#define kAdslCfgModAnnexIOnly               0x00000004
#define kAdslCfgModAdsl2Only                0x00001000
#define kAdslCfgModAdsl2pOnly               0x00002000
#define kDslCfgModVdsl2Only                 0x00004000
#define kDslCfgModGfastOnly                 0x00008000


#define kAdslCfgBitmapMask                  0x00000018
#define kAdslCfgDBM                               0x00000000
#define kAdslCfgFBM                               0x00000008
#define kAdslCfgFBMSoL                         0x00000010

#define kAdslCfgLinePairMask                0x00000020
#define kAdslCfgLineInnerPair               0x00000000
#define kAdslCfgLineOuterPair               0x00000020

#define kAdslCfgCentilliumCRCWorkAroundMask         0x00000040
#define kAdslCfgCentilliumCRCWorkAroundDisabled     0x00000000
#define kAdslCfgCentilliumCRCWorkAroundEnabled      0x00000040

#define kAdslCfgExtraData                   0x00000080
#define kAdslCfgTrellisMask                 (0x00000100 | kAdslCfgExtraData)
#define kAdslCfgTrellisOn                   (0x00000100 | kAdslCfgExtraData)
#define kAdslCfgTrellisOff                  (0 | kAdslCfgExtraData)
#define kAdslCfgExtraMask                   0xFFFFFF80

#define kAdslCfgLOSMonitoringMask           0x00000200
#define kAdslCfgLOSMonitoringOff            0x00000200
#define kAdslCfgLOSMonitoringOn             0x00000000

#define kAdslCfgMarginMonitoringMask        0x00000400
#define kAdslCfgMarginMonitoringOn          0x00000400
#define kAdslCfgMarginMonitoringOff         0x00000000

#define kAdslCfgDemodCapMask                0x00000800
#define kAdslCfgDemodCapOn                  0x00000800
#define kAdslCfgDemodCapOff                 0x00000000

/* Flags 0x00001000 - 0x00008000 are reserved for modulation (see above) */

/* Upstream mode flags 0x00010000 - 0x00030000 */

#define kAdslCfgUpstreamModeMask            0x00030000
#define kAdslCfgUpstreamMax                 0x00000000
#define kAdslCfgUpstreamSingle              0x00010000
#define kAdslCfgUpstreamDouble              0x00020000
#define kAdslCfgUpstreamTriple              0x00030000

#define kAdslCfgNoSpectrumOverlap           0x00040000

/* Pwm sync clock configuration */

#define kAdslCfgPwmSyncClockMask            0x00080000
#define kAdslCfgPwmSyncClockOn              0x00080000
#define kAdslCfgPwmSyncClockOff             0x00000000

#define kAdslCfgDemodCap2Mask               0x00100000
#define kAdslCfgDemodCap2On                 0x00100000
#define kAdslCfgDemodCap2Off                0x00000000

#define kAdslCfgTpsTcShift					21
#define kAdslCfgTpsTcMask					0x01E00000
#define kAdslCfgTpsTcPtmAdsl				0x00200000
#define kAdslCfgTpsTcAtmAdsl				0x00400000
#define kAdslCfgTpsTcPtmVdsl				0x00800000
#define kAdslCfgTpsTcAtmVdsl				0x01000000

#define kAdslCfgTpsTcPtmPreShift			25
#define kAdslCfgTpsTcPtmPreMask				0x02000000
#define kAdslCfgTpsTcPtmPreEnabled			0x00000000
#define kAdslCfgTpsTcPtmPreDisabled			0x02000000

#define kAdslCfgDefaultTrainingMargin       -1
#define kAdslCfgDefaultShowtimeMargin       -1
#define kAdslCfgDefaultLOMTimeThld          -1

/* ADSL2 parameters */

#define kAdsl2CfgReachExOn                  0x00000001
#define kAdsl2CfgAnnexMEnabled              0x00000002

#define kAdsl2CfgAnnexMPsdShift             2
#define kAdsl2CfgAnnexMPsdBits              12
#define kAdsl2CfgAnnexMPsdMask              (0xFFF << kAdsl2CfgAnnexMPsdShift)

#if (((1 << kAdsl2CfgAnnexMPsdBits) - 1) != (kAdsl2CfgAnnexMPsdMask >> kAdsl2CfgAnnexMPsdShift))
#error Inconsistent kAdsl2CfgAnnexM definitions
#endif

#define kAdsl2CfgAnnexMUp32                 (0x00000001 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp36                 (0x00000002 << kAdsl2CfgAnnexMPsdShift) 
#define kAdsl2CfgAnnexMUp40                 (0x00000004 << kAdsl2CfgAnnexMPsdShift) 
#define kAdsl2CfgAnnexMUp44                 (0x00000008 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp48                 (0x00000010 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp52                 (0x00000020 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp56                 (0x00000040 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp60                 (0x00000080 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp64                 (0x00000100 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMCustomPsd            (0x00000200 << kAdsl2CfgAnnexMPsdShift)

#define kAdsl2CfgAnnexLShift                (2 + kAdsl2CfgAnnexMPsdBits)
#define kAdsl2CfgAnnexLMask                 (0x7 << kAdsl2CfgAnnexLShift)

#define kAdsl2CfgAnnexLUpWide               (0x00000001 << kAdsl2CfgAnnexLShift)
#define kAdsl2CfgAnnexLUpNarrow             (0x00000002 << kAdsl2CfgAnnexLShift)
#define kAdsl2CfgAnnexLDnOvlap              (0x00000004 << kAdsl2CfgAnnexLShift)

#define kAdsl2CfgAnnexMShift                (3+kAdsl2CfgAnnexLShift)
#define kAdsl2CfgAnnexMpXMask               (0x6 << kAdsl2CfgAnnexMShift)
#define kAdsl2CfgAnnexMOnly                 (0x00000001 << kAdsl2CfgAnnexMShift)
#define kAdsl2CfgAnnexMp3                   (0x00000002 << kAdsl2CfgAnnexMShift)
#define kAdsl2CfgAnnexMp5                   (0x00000004 << kAdsl2CfgAnnexMShift)

/* PHY Capabilities bitmap */
#define kXdslBitSwapEnabled				0x00004000
#define kXdslSRAEnabled				0x00800000
#define kXdslTrellisEnabled				0x00000008

/* PHY Capabilities bitmap2 */
#define kXdslRetrainOnSesEnabled			0x00080000
#define kXdsl24kbyteInterleavingEnabled		0x00100000
#define kXdslRetrainOnDslamMinMargin		0x00200000
#define kXdslFireDsSupported				0x00400000
#define kXdslFireUsSupported				0x00800000

/* PHY AuxFeature bitmap  */
#define kXdslMonitorToneDisable		0x00004000
#define kXdslGinpDsSupported        0x00020000
#define kXdslGinpUsSupported        0x00040000

/* VDSL2 parameters */
#define		kVdslProfile8a		0x00000001
#define		kVdslProfile8b 		0x00000002
#define		kVdslProfile8c 		0x00000004
#define		kVdslProfile8d 		0x00000008
#define		kVdslProfile12a		0x00000010
#define		kVdslProfile12b		0x00000020
#define		kVdslProfile17a		0x00000040
#define		kVdslProfile30a		0x00000080
#define		kVdslProfileBrcmPriv1		0x00000100
#define		kVdslProfileBrcmPriv2		0x00000200

#define		kGfastProfile106a		0x00001000
#define		kGfastProfile212a		0x00002000
#define		kGfastProfile106b		0x00004000
#define		kGfastProfile106c		0x00008000
#define		kGfastProfile212c		0x00000800
#define		kGfastProfile106aDisable	kGfastProfile106a
#define		kGfastProfile212aDisable	kGfastProfile212a
#define		kGfastProfile106bDisable	kGfastProfile106b
#define		kGfastProfile106cDisable	kGfastProfile106c
#define		kGfastProfile212cDisable	kGfastProfile212c

#define		kVdslProfileMask	(kVdslProfile8a | kVdslProfile8b | kVdslProfile8c |kVdslProfile8d |\
								kVdslProfile12a | kVdslProfile12b | kVdslProfile17a)
#define		kVdslProfileMask1	(kVdslProfileMask | kVdslProfile30a)
#define		kVdslProfileMask2	(kVdslProfileMask1 | kVdslProfileBrcmPriv1)
#define		kVdslProfileMask3	(kVdslProfileMask2 | kVdslProfileBrcmPriv2)

#define		kGfastProfileMask	(kGfastProfile106aDisable | kGfastProfile106bDisable)
#define		kGfastProfileMask1	(kGfastProfileMask | kGfastProfile212aDisable | kGfastProfile106cDisable | kGfastProfile212cDisable)

#define		kVdslUS0MaskShift	16
#define		kVdsl8aUS0Mask	(0x00000001 << kVdslUS0MaskShift)
#define		kVdsl8bUS0Mask 	(0x00000002 << kVdslUS0MaskShift)
#define		kVdsl8cUS0Mask 	(0x00000004 << kVdslUS0MaskShift)
#define		kVdsl8dUS0Mask 	(0x00000008 << kVdslUS0MaskShift)
#define		kVdsl12aUS0Mask 	(0x00000010 << kVdslUS0MaskShift)
#define		kVdsl12bUS0Mask 	(0x00000020 << kVdslUS0MaskShift)
#define		kVdsl17aUS0Mask 	(0x00000040 << kVdslUS0MaskShift)
#define		kVdsl30aUS0Mask 	(0x00000080 << kVdslUS0MaskShift)
#define		kVdslUS0Mask 		(kVdsl8aUS0Mask | kVdsl8bUS0Mask | kVdsl8cUS0Mask | kVdsl8dUS0Mask |\
								kVdsl12aUS0Mask | kVdsl12bUS0Mask | kVdsl17aUS0Mask)

/* VDSL cfgFlag bitmap */
#define		kVdslDynamicDDisable	0x00000200
#define		kVdslDynamicFDisable	0x00000400
#define		kVdslSOSDisable		0x00000800


#define		SUPPORT_CFG_PROFILE

typedef struct _adslCfgProfile {
    int        adslAnnexCParam;
    int        adslAnnexAParam;
    int        adslTrainingMarginQ4;
    int        adslShowtimeMarginQ4;
    int        adslLOMTimeThldSec;
    int        adslDemodCapMask;
    int        adslDemodCapValue;
    int        adsl2Param;
    int        adslPwmSyncClockFreq;
    int        adslHsModeSwitchTime;
    int        adslDemodCap2Mask;
    int        adslDemodCap2Value;
    int        vdslParam;
    int        vdslParam1;
    int        xdslAuxFeaturesMask;
    int        xdslAuxFeaturesValue;
    int        vdslCfgFlagsMask;
    int        vdslCfgFlagsValue;
    int        xdslCfg1Mask;
    int        xdslCfg1Value;
    int        xdslCfg2Mask;
    int        xdslCfg2Value;
    int        xdslCfg3Mask;
    int        xdslCfg3Value;
    int        xdslCfg4Mask;
    int        xdslCfg4Value;
    int        maxUsDataRateKbps;
    int        maxDsDataRateKbps;
    int        maxAggrDataRateKbps;
    int        xdslMiscCfgParam;
    int        minINP;
    int        maxDelay;
} adslCfgProfile;

/* 
**
**      ADSL PHY configuration
**
*/

typedef struct _adslPhyCfg {
    int        demodCapMask;
    int        demodCap;
} adslPhyCfg;

/* 
**
**      ADSL version info parameters 
**
*/

#define kAdslVersionStringSize              64

#define kAdslTypeUnknown                    0
#define kAdslTypeAnnexA                     1
#define kAdslTypeAnnexB                     2
#define kAdslTypeAnnexC                     3
#define kAdslTypeSADSL                      4
#define kAdslTypeAnnexM                     5
#define kAdslTypeAnnexJ                     6
#define kAdslTypeAnnexI                     7
#define kAdslTypeAnnexAB                    8
#define kAdslTypeAnnexL                     9

typedef struct _adslVersionInfo {
    unsigned short  phyType;
    unsigned short  phyMjVerNum;
    unsigned short  phyMnVerNum;
    char            phyVerStr[kAdslVersionStringSize];
    unsigned short  drvMjVerNum;
    unsigned short  drvMnVerNum;
    char            drvVerStr[kAdslVersionStringSize];
} adslVersionInfo;

/* 
**
**      ADSL self-test parameters 
**
*/

#define kAdslSelfTestLMEM                   0x00000001
#define kAdslSelfTestSDRAM                  0x00000002
#define kAdslSelfTestAFE                    0x00000004
#define kAdslSelfTestQproc                  0x00000008
#define kAdslSelfTestRS                     0x00000010
#define kAdslSelfTestHostDma                0x00000020

#define kAdslSelfTestAll                    ((kAdslSelfTestHostDma - 1) | kAdslSelfTestHostDma)

#define kAdslSelfTestInProgress             0x40000000
#define kAdslSelfTestCompleted              0x80000000

/* MIB OID's for ADSL objects */

#define kOidMaxObjLen                       80

#define kOidAdsl                            94
#define kOidAdslInterleave                  124
#define kOidAdslFast                        125
#define kOidAtm                             37
#define kOidAdslPrivate                     255
#define kOidAdslPrivatePartial              254
#define kOidAdslPrivateSysCtl               253
#define kOidAdslPrivateSysMediaCfg    0

#define kAdslMibAnnexAToneNum               256
#define kAdslMibToneNum                     kAdslMibAnnexAToneNum
#ifdef G992_ANNEXC
#define kAdslMibMaxToneNum                  kAdslMibAnnexAToneNum*2*2
#else
#define kAdslMibMaxToneNum                  kAdslMibAnnexAToneNum*2
#endif

#define kVdslMibToneNum                     4096
#define kVdslMibMaxToneNum                  4096

/* OIDs for kOidAdslPrivate ->objId[1] */
#define kOidAdslPrivSNR                     1
#define kOidAdslPrivBitAlloc                2
#define kOidAdslPrivGain                    3
#define kOidAdslPrivShowtimeMargin          4
#define kOidAdslPrivChanCharLin             5
#define kOidAdslPrivChanCharLog             6
#define kOidAdslPrivQuietLineNoise          7
#define kOidAdslPrivPLNDurationBins         8
#define kOidAdslPrivPLNIntrArvlBins         9
#ifdef ADSL_MIBOBJ_PLN
#define kOidAdslPrivPLNValueps              10
#define kOidAdslPrivPLNThldCntps            11
#endif
#define kOidAdslPrivPLNDurationHist         12
#define kOidAdslPrivPLNIntrArvlHist         13
#define kOidAdslPrivNLDistNoise             14
#define kOidAdslPrivChanCharLinDsPerToneGroup     18
#define kOidAdslPrivChanCharLinUsPerToneGroup     19
#define kOidAdslPrivQuietLineNoiseDsPerToneGroup  20
#define kOidAdslPrivQuietLineNoiseUsPerToneGroup  21
#define kOidAdslPrivSNRDsPerToneGroup       22
#define kOidAdslPrivSNRUsPerToneGroup       23
#define kOidAdslPrivBitAllocDsPerToneGroup  24
#define kOidAdslPrivBitAllocUsPerToneGroup  25
#define kOidAdslPrivGainDsPerToneGroup      26
#define kOidAdslPrivGainUsPerToneGroup      27
#define kOidAdslPrivChanCharLogDsPerToneGroup 28
#define kOidAdslPrivChanCharLogUsPerToneGroup 29
#define kOidAdslPrivLATNdsperband             30
#define kOidAdslPrivLATNusperband             31
#define kOidAdslPrivSATNdsperband             32
#define kOidAdslPrivSATNusperband             33
#define kOidAdslPrivSNRMdsperband             34
#define kOidAdslPrivSNRMusperband             35
#define kOIdAdslPrivGetVectState              37
#define kOIdAdslPrivGetVectSM                 38
#define kOIdAdslPrivGetVceMacAddress          39
#define kOIdAdslPrivGetVectPhyData            40
#define kOidAdslPrivTxPwrusperband            41
#define kOidAdslPrivTxPwrdsperband            42
#define kOidAdslPrivUER                     44
#define kOidAdslPrivEchoVariance            45
#define kOidAdslPrivQuietLineNoiseRnc       48
#define kOidAdslPrivActiveLineNoise         49
#define kOidAdslPrivDoiBitAlloc             51
#define kOidAdslPrivDoiGain                 52
#define kOidAdslPrivGetChallenge            53
#define kOidAdslPrivChallengeResp           54
#define kOidAdslPrivGetTodInfo              55


#define kOidAdslExtraPLNInfo						11
#define kOidAdslExtraPLNData						12


#define kOidAdslExtraPLNDataThldBB						1
#define kOidAdslExtraPLNDataThldPerTone					2
#define kOidAdslExtraPLNDataPLNState					3
#define kOidAdslExtraPLNDataNbDurBins					4
#define kOidAdslExtraPLNDataNbIntArrBins				5
#define kOidAdslExtraPLNDataUpdate						6
#define kOidAdslPrivINMConfigFormat         10
#define kOidAdslPrivINMControlParams        11
#define kOidAdslPrivNonLinThldNumBins     15
#define kOidAdslPrivSetSnrClampShape      16
#define kOidAdslPrivSetNtrCfg                      17

#define kOidAdslPrivSetFlagActualGFactor      36
#define kOidAdslPrivGetNtrCnt        43
#define kOidAdslPrivLATNperband        1
#define kOidAdslPrivSATNperband        2
#define kOidAdslPrivSNRMperband        3
#define kOidAdslPrivTxPwrperband       4

#define kOidAdslPrivSetSeltData        46
#define kOidAdslPrivGetSeltData        47

#define kOidAdslPrivGetGfastOlrCnt     50


#define kPlnNumberOfDurationBins       32
#define kPlnNumberOfInterArrivalBins   16

#define kOidAdslPrivExtraInfo               255
#define kOidAdslExtraNLInfo                  10
#define kOidAdslPrivBandPlanUSNegPresentation       24
#define kOidAdslPrivBandPlanUSPhyPresentation       25
#define kOidAdslPrivBandPlanDSNegPresentation       26
#define kOidAdslPrivBandPlanDSPhyPresentation       27
#define kOidAdslPrivBandPlanUSNegDiscoveryPresentation	28
#define kOidAdslPrivBandPlanUSPhyDiscoveryPresentation	29
#define kOidAdslPrivBandPlanDSNegDiscoveryPresentation	30
#define kOidAdslPrivBandPlanDSPhyDiscoveryPresentation	31
#define kOidAdslPrivINMConfigParameters	32

#define kOidAdslLine                        1
#define kOidAdslMibObjects                  1

#define kOidAdslLineTable                   1
#define kOidAdslLineEntry                   1
#define kOidAdslLineCoding                  1
#define kOidAdslLineType                    2
#define kOidAdslLineSpecific                3
#define kOidAdslLineConfProfile             4
#define kOidAdslLineAlarmConfProfile        5

#define kOidAdslAtucPhysTable               2
#define kOidAdslAturPhysTable               3
#define kOidAdslPhysEntry                   1
#define kOidAdslPhysInvSerialNumber         1
#define kOidAdslPhysInvVendorID             2
#define kOidAdslPhysInvVersionNumber        3
#define kOidAdslPhysCurrSnrMgn              4
#define kOidAdslPhysCurrAtn                 5
#define kOidAdslPhysCurrStatus              6
#define kOidAdslPhysCurrOutputPwr           7
#define kOidAdslPhysCurrAttainableRate      8

#define kOidAdslAtucChanTable               4
#define kOidAdslAturChanTable               5
#define kOidAdslChanEntry                   1
#define kOidAdslChanInterleaveDelay         1
#define kOidAdslChanCurrTxRate              2
#define kOidAdslChanPrevTxRate              3
#define kOidAdslChanCrcBlockLength          4

#define kOidAdslAtucPerfDataTable           6
#define kOidAdslAturPerfDataTable           7
#define kOidAdslPerfDataEntry               1
#define kOidAdslPerfLofs                    1
#define kOidAdslPerfLoss                    2
#define kOidAdslPerfLprs                    3
#define kOidAdslPerfESs                     4
#define kOidAdslPerfValidIntervals          5
#define kOidAdslPerfInvalidIntervals        6
#define kOidAdslPerfCurr15MinTimeElapsed    7
#define kOidAdslPerfCurr15MinLofs           8
#define kOidAdslPerfCurr15MinLoss           9
#define kOidAdslPerfCurr15MinLprs           10
#define kOidAdslPerfCurr15MinESs            11
#define kOidAdslPerfCurr1DayTimeElapsed     12
#define kOidAdslPerfCurr1DayLofs            13
#define kOidAdslPerfCurr1DayLoss            14
#define kOidAdslPerfCurr1DayLprs            15
#define kOidAdslPerfCurr1DayESs             16
#define kOidAdslPerfPrev1DayMoniSecs        17
#define kOidAdslPerfPrev1DayLofs            18
#define kOidAdslPerfPrev1DayLoss            19
#define kOidAdslPerfPrev1DayLprs            20
#define kOidAdslPerfPrev1DayESs             21

#define kOidAdslAtucPerfIntervalTable       8
#define kOidAdslAturPerfIntervalTable       9
#define kOidAdslPerfIntervalEntry           1
#define kOidAdslIntervalNumber              1
#define kOidAdslIntervalLofs                2
#define kOidAdslIntervalLoss                3
#define kOidAdslIntervalLprs                4
#define kOidAdslIntervalESs                 5
#define kOidAdslIntervalValidData           6

#define kOidAdslAtucChanPerfTable                   10
#define kOidAdslAturChanPerfTable                   11
#define kOidAdslChanPerfEntry                       1
#define kOidAdslChanReceivedBlks                    1
#define kOidAdslChanTransmittedBlks                 2
#define kOidAdslChanCorrectedBlks                   3
#define kOidAdslChanUncorrectBlks                   4
#define kOidAdslChanPerfValidIntervals              5
#define kOidAdslChanPerfInvalidIntervals            6
#define kOidAdslChanPerfCurr15MinTimeElapsed        7
#define kOidAdslChanPerfCurr15MinReceivedBlks       8
#define kOidAdslChanPerfCurr15MinTransmittedBlks    9
#define kOidAdslChanPerfCurr15MinCorrectedBlks      10
#define kOidAdslChanPerfCurr15MinUncorrectBlks      11
#define kOidAdslChanPerfCurr1DayTimeElapsed         12
#define kOidAdslChanPerfCurr1DayReceivedBlks        13
#define kOidAdslChanPerfCurr1DayTransmittedBlks     14
#define kOidAdslChanPerfCurr1DayCorrectedBlks       15
#define kOidAdslChanPerfCurr1DayUncorrectBlks       16
#define kOidAdslChanPerfPrev1DayMoniSecs            17
#define kOidAdslChanPerfPrev1DayReceivedBlks        18
#define kOidAdslChanPerfPrev1DayTransmittedBlks     19
#define kOidAdslChanPerfPrev1DayCorrectedBlks       20
#define kOidAdslChanPerfPrev1DayUncorrectBlks       21

#define kOidAdslAtucChanIntervalTable               12
#define kOidAdslAturChanIntervalTable               13
#define kOidAdslChanIntervalEntry                   1
#define kOidAdslChanIntervalNumber                  1
#define kOidAdslChanIntervalReceivedBlks            2
#define kOidAdslChanIntervalTransmittedBlks         3
#define kOidAdslChanIntervalCorrectedBlks           4
#define kOidAdslChanIntervalUncorrectBlks           5
#define kOidAdslChanIntervalValidData               6

#define kOidAtmMibObjects       1
#define kOidAtmTcTable          4
#define kOidAtmTcEntry          1
#define kOidAtmOcdEvents        1
#define kOidAtmAlarmState       2

/* Adsl Channel coding */

#define kAdslRcvDir         0
#define kAdslXmtDir         1

#define kAdslRcvActive      (1 << kAdslRcvDir)
#define kAdslXmtActive      (1 << kAdslXmtDir)

#define kAdslIntlChannel    0
#define kAdslFastChannel    1

#define kAdslTrellisOff     0
#define kAdslTrellisOn      1
#define kAdsl2TrellisRxEnabled        0x1
#define kAdsl2TrellisTxEnabled        0x2

/* AnnexC modulation and bitmap types for the field (adslConnection.modType) */

#define kAdslModMask        0xF

#define kAdslModGdmt        0
#define kAdslModT1413       1
#define kAdslModGlite       2
#define kAdslModAnnexI      3
#define kAdslModAdsl2       4
#define kAdslModAdsl2p      5
#define kAdslModReAdsl2     6
#define kVdslModVdsl2       7
#define kXdslModGfast       8

#define kAdslBitmapShift    3
#define kAdslBitmapMask     kAdslCfgBitmapMask
#define kAdslDBM            (0 << kAdslBitmapShift)
#define kAdslFBM            (1 << kAdslBitmapShift)
#define kAdslFBMSoL         (2 << kAdslBitmapShift)

#define kAdslUpstreamModeShift      5
#define kAdslUpstreamModeMask       (3 << kAdslUpstreamModeShift)
#define kAdslUpstreamModeSingle     (0 << kAdslUpstreamModeShift)
#define kAdslUpstreamModeDouble     (1 << kAdslUpstreamModeShift)
#define kAdslUpstreamModeTriple     (2 << kAdslUpstreamModeShift)

/* AdslLineCodingType definitions */

#define kAdslLineCodingOther        1
#define kAdslLineCodingDMT          2
#define kAdslLineCodingCAP          3
#define kAdslLineCodingQAM          4

/* AdslLineType definitions */

#define kAdslLineTypeNoChannel      1
#define kAdslLineTypeFastOnly       2
#define kAdslLineTypeIntlOnly       3
#define kAdslLineTypeFastOrIntl     4
#define kAdslLineTypeFastAndIntl    5

typedef struct _adslLineEntry {
    unsigned char   adslLineCoding;
    unsigned char   adslLineType;
} adslLineEntry;


/* AdslPhys status definitions */

#define kAdslPhysStatusNoDefect     (1 << 0)
#define kAdslPhysStatusLOF          (1 << 1)    /* lossOfFraming (not receiving valid frame) */
#define kAdslPhysStatusLOS          (1 << 2)    /* lossOfSignal (not receiving signal) */
#define kAdslPhysStatusLPR          (1 << 3)    /* lossOfPower */
#define kAdslPhysStatusLOSQ         (1 << 4)    /* lossOfSignalQuality */
#define kAdslPhysStatusLOM          (1 << 5)    /* lossOfMargin */
#define kAdslPhysStatusLOR          (1 << 6)    /* lossOfRMC */


#define kAdslPhysVendorIdLen        8
#define kAdslPhysSerialNumLen       32
#define kAdslPhysVersionNumLen    32
#define MAX_NUM_BANDS               4
#define MAX_NUM_SMALL_BANDS    32
#define MAX_LP_NUM                      2

typedef struct _adslPhysEntry {
    int        adslCurrSnrMgn;
    int        adslCurrAtn;
    int        adslCurrStatus;
    int        adslCurrOutputPwr;
    int        adslCurrAttainableRate;
    int        adslSignalAttn;
    int        adslHlinScaleFactor;
    int        adslLDCompleted;
    char        adslVendorID[kAdslPhysVendorIdLen];
    int        actualPSD;
    int        SNRmode;
    int        QLNMT;
    int        HLOGMT;
    int        SNRMT;
    int        actualCE;
    unsigned short	UPBOkle;
} adslPhysEntry;

#if defined(CONFIG_VDSL_SUPPORTED)
typedef struct _xdslPhysEntry {
    int        adslCurrSnrMgn;
    int        adslCurrAtn;
    int        adslCurrStatus;
    int        adslCurrOutputPwr;
    int        adslCurrAttainableRate;
    int        adslSignalAttn;
    int        adslHlinScaleFactor;
    int        adslLDCompleted;
    char        adslVendorID[kAdslPhysVendorIdLen];
    int        actualPSD;
    int        SNRmode;
    int        QLNMT;
    int        HLOGMT;
    int        SNRMT;
    int        actualCE;
    unsigned short      UPBOkle;
    int        perBandCurrOutputPwr[MAX_NUM_BANDS];
    unsigned char  attnDrMethod;
    unsigned char  attnDrInp;
    unsigned char  attnDrDelay;
#ifdef USE_TRAINING_ATTNDR
    int        adslShowAttainableRate;
    int        adslTrainAttainableRate;
#endif
    int        snrmRoc; /* SNRM ROC/SNRM RMC --> VDSL2/G.fast */
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
    short       rxPower;
#endif
    int        xdslSignalAttnAt300kHz;
    int         numKl0BandReported;
    short       kl0PerBand[MAX_NUM_BANDS];
    unsigned short      UPBOkleCpe;
    unsigned int        attnETR;
} xdslPhysEntry;
#endif

typedef struct _adslFullPhysEntry {
    char        adslSerialNumber[kAdslPhysSerialNumLen];
    char        adslVendorID[kAdslPhysVendorIdLen];
    char        adslVersionNumber[kAdslPhysVersionNumLen];
    int        adslCurrSnrMgn;
    int        adslCurrAtn;
    int        adslCurrStatus;
    int        adslCurrOutputPwr;
    int        adslCurrAttainableRate;
    int        adslSignalAttn;
    int        adslHlinScaleFactor;
    char        adslSysVendorID[kAdslPhysVendorIdLen];
    char        adslSysVersionNumber[kAdslPhysVersionNumLen];
    int        actualPSD;
    int        SNRmode;
    int        QLNMT;
    int        HLOGMT;
    int        SNRMT;
#ifdef CO_G994_NSIF
    int        nsifLen;
    char        adslNsif[64];
#endif
} adslFullPhysEntry;

#if defined(CONFIG_VDSL_SUPPORTED)
typedef struct _xdslFullPhysEntry {
    char        adslSerialNumber[kAdslPhysSerialNumLen];
    char        adslVendorID[kAdslPhysVendorIdLen];
    char        adslVersionNumber[kAdslPhysVersionNumLen];
    int        adslCurrSnrMgn;
    int        adslCurrAtn;
    int        adslCurrStatus;
    int        adslCurrOutputPwr;
    int        adslCurrAttainableRate;
    int        adslSignalAttn;
    int        adslHlinScaleFactor;
    char        adslSysVendorID[kAdslPhysVendorIdLen];
    char        adslSysVersionNumber[kAdslPhysVersionNumLen];
    int        actualPSD;
    int        SNRmode;
    int        QLNMT;
    int        HLOGMT;
    int        SNRMT;
#ifdef CO_G994_NSIF
    int        nsifLen;
    char        adslNsif[64];
#endif
    int        perBandCurrOutputPwr[MAX_NUM_BANDS];
    unsigned char  attnDrMethod;
    unsigned char  attnDrInp;
    unsigned char  attnDrDelay;
    int        snrmRoc; /* SNRM ROC/SNRM RMC --> VDSL2/G.fast */
    int         numKl0BandReported;
    short       kl0PerBand[MAX_NUM_BANDS];
    unsigned int       attnETR;
} xdslFullPhysEntry;
#endif

/* Adsl channel entry definitions */

typedef struct _adslChanEntry {
    unsigned int       adslChanIntlDelay;
    unsigned int       adslChanCurrTxRate;
    unsigned int       adslChanPrevTxRate;
    unsigned int       adslChanCrcBlockLength;
} adslChanEntry;

/* Adsl performance data definitions */

typedef struct _adslPerfCounters {
   union {
    unsigned int       adslLofs;
    unsigned int       xdslLors;   /* Count of LOR */
    };
    unsigned int       adslLoss;
    union {
    unsigned int       adslLols;   /* Loss of Link failures (ATUC only) */
    unsigned int       xdslLoms;   /* Count of LOM */
    };
    unsigned int       adslLprs;
    unsigned int       adslESs;    /* Count of Errored Seconds */
    union {
    unsigned int       adslInits;  /* Count of Line initialization attempts (ATUC only) */
    unsigned int       adslLOMS;  /* Count of LOM Seconds */
    };
    unsigned int       adslUAS;    /* Count of Unavailable Seconds */
    unsigned int       adslSES;    /* Count of Severely Errored Seconds */
    unsigned int       adslLOSS;   /* Count of LOS seconds */
    unsigned int       adslFECs;   /* Count of FEC seconds  */
    union {
    unsigned int       adslLCDS;   /* Count of LCD Errored Seconds */
    unsigned int       xdslLORS;   /* Count of LOR Seconds */
    };
    unsigned int       adslAS;         /* Count of Availabe in second */ 
} adslPerfCounters;

typedef struct _adslFailureCounters {
    unsigned int       adslRetr;       /* Count of total retrains */
    unsigned int       adslRetrLof;    /* Count of retrains due to LOF */
    unsigned int       adslRetrLos;    /* Count of retrains due to LOS */
    unsigned int       adslRetrLpr;    /* Count of retrains due to LPR */
    unsigned int       adslRetrLom;    /* Count of retrains due to LOM */
    unsigned int       adslInitErr;    /* Count of training failures */
    unsigned int       adslInitTo;     /* Count of training timeouts */
    unsigned int       adslLineSearch; /* Count of line search inits */
    unsigned int       xdslFastRetr;   /* Count of fast retrains */
    unsigned int       xdslFastInitErr;    /* Count of fastRetrain failures */
    unsigned int       xdslHostInitRetr;   /* Count of Host initiated retrains */
} adslFailureCounters;

typedef struct _adslPerfDataEntry {
    adslPerfCounters    perfTotal;
    adslPerfCounters    perfSinceShowTime;
    unsigned int               adslPerfValidIntervals;
    unsigned int               adslPerfInvalidIntervals;
    adslPerfCounters    perfCurr15Min;
    unsigned int               adslPerfCurr15MinTimeElapsed;
    adslPerfCounters    perfCurr1Day;
    unsigned int               adslPerfCurr1DayTimeElapsed;
    adslPerfCounters    perfPrev1Day;
    unsigned int               adslAturPerfPrev1DayMoniSecs;
    adslFailureCounters failTotal;
    adslFailureCounters failSinceShowTime;
    adslFailureCounters failSinceLastShowTime;
    adslFailureCounters failCurDay;
    adslFailureCounters failPrevDay;
    adslFailureCounters failCur15Min;
    int                    lastRetrainReason;
    int                    lastShowtimeDropReason;
    unsigned int       adslSinceLinkTimeElapsed;
    unsigned int       adslSincePrevLinkTimeElapsed;
    unsigned int       adslSinceDrvStartedTimeElapsed; /*total time since driver started, reset when driver restarted*/
} adslPerfDataEntry;

#define kAdslMibPerfIntervals       4

typedef struct {
    unsigned int    rtx_c;
    unsigned int    rtx_uc;
    unsigned int    rtx_tx;
} rtxCounters;

typedef struct {
    rtxCounters     perfTotal;
    rtxCounters     perfSinceShowTime;
    rtxCounters     perfCurr15Min;
    rtxCounters     perfCurr1Day;
} rtxCounterInfo;

typedef struct {
    rtxCounterInfo  cntDS;
    rtxCounterInfo  cntUS;
} rtxCounterInfoEntry;

#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
typedef struct {
    unsigned int    bswStarted;
    unsigned int    bswCompleted;
    unsigned int    sraStarted;
    unsigned int    sraCompleted;
    unsigned int    fraStarted;
    unsigned int    fraCompleted;
    unsigned int    rpaStarted;
    unsigned int    rpaCompleted;
    unsigned int    tigaStarted;
    unsigned int    tigaCompleted;
} gfastOlrCounters;

typedef struct {
    gfastOlrCounters   perfTotal;
    gfastOlrCounters   perfSinceShowTime;
    gfastOlrCounters   perfCurr15Min;
    gfastOlrCounters   perfCurr1Day;
} gfastOlrCounterInfo;

typedef struct {
    gfastOlrCounterInfo    cntDS;
    gfastOlrCounterInfo    cntUS;
} gfastOlrCounterInfoEntry;
#endif

/* Adsl channel performance data definitions */

typedef struct _adslChanCounters {
    unsigned int       adslChanReceivedBlks;
    unsigned int       adslChanTransmittedBlks;
    unsigned int       adslChanCorrectedBlks;
    unsigned int       adslChanUncorrectBlks;
    unsigned int       adslChanTxFEC;
    unsigned int       adslChanTxCRC;
} adslChanCounters;

typedef struct _adslChanPerfDataEntry {
    adslChanCounters    perfTotal;
    unsigned int               adslChanPerfValidIntervals;
    unsigned int               adslChanPerfInvalidIntervals;
    adslChanCounters    perfCurr15Min;
    unsigned int               adslPerfCurr15MinTimeElapsed;
    adslChanCounters    perfCurr1Day;
    unsigned int               adslPerfCurr1DayTimeElapsed;
    adslChanCounters    perfPrev1Day;
    unsigned int               adslAturPerfPrev1DayMoniSecs;
} adslChanPerfDataEntry;

#define kDslNonLinearityDefaultThreshold 60

typedef struct _adslNonLinearityData{
  unsigned short                maxCriticalDistNoise;
  unsigned short                distAffectedBits;
  short                             NonLinNumAffectedBins;
  unsigned short                NonLinThldNumAffectedBins;
  char                          NonLinearityFlag;
  short                             NonLinDbEcho;
} adslNonLinearityData;

typedef struct _adslPLNDataEntry {
    char PLNState;
    unsigned int PLNThldBB;
    unsigned int PLNThldPerTone;
    char PLNNbDurBins;
    char PLNNbIntArrBins;
    unsigned int PLNPerToneCounter;
    unsigned int PLNBBCounter;
    char  PLNUpdateData;
} adslPLNDataEntry;
	
typedef struct _adslINMConfiguration {
    unsigned short        INMCC;
    unsigned short        INM_INPEQ_MODE;
    unsigned short        INMIATS;
    unsigned short        INMIATO;
    unsigned char         INMDF;
    unsigned short        INM_INPEQ_FORMAT;
    unsigned short        INM_INPEQ_SCALE;
    unsigned short        INM_IAT_SCALE;
    unsigned short        INM_BRGN;
    unsigned short        INM_GFAST_CO_CPE_SUPPORT;
    unsigned char         newConfig;
}adslINMConfiguration;

#define kAdslMibChanPerfIntervals   4

/* Adsl trap threshold definitions */

#define kAdslEventLinkChange        0x001
#define kAdslEventRateChange        0x002
#define kAdslEventLofThresh         0x004
#define kAdslEventLosThresh         0x008
#define kAdslEventLprThresh         0x010
#define kAdslEventESThresh          0x020
#define kAdslEventFastUpThresh      0x040
#define kAdslEventIntlUpThresh      0x080
#define kAdslEventFastDownThresh    0x100
#define kAdslEventIntlDwonThresh    0x200

#define kXdslEventContSESThresh     0x400
#define kXdslEventFastRetrain       0x800
#define kXdslEventKeyChal          0x1000


typedef struct _adslThreshCounters {
    unsigned int       adslThreshLofs;
    unsigned int       adslThreshLoss;
    unsigned int       adslThreshLols; /* Loss of Link failures (ATUC only) */
    unsigned int       adslThreshLprs;
    unsigned int       adslThreshESs;
    unsigned int       adslThreshFastRateUp;
    unsigned int       adslThreshIntlRateUp;
    unsigned int       adslThreshFastRateDown;
    unsigned int       adslThreshIntlRateDown;
} adslThreshCounters;


/* Atm PHY performance data definitions */

#define kAtmPhyStateNoAlarm         1
#define kAtmPhyStateLcdFailure      2

typedef struct _atmPhyDataEntrty {
    unsigned int       atmInterfaceOCDEvents;
    unsigned int       atmInterfaceTCAlarmState;
} atmPhyDataEntrty;

typedef struct _adslBertResults {
    unsigned int       bertTotalBits;
    unsigned int       bertErrBits;
} adslBertResults;

typedef struct {
    unsigned int       cntHi;
    unsigned int       cntLo;
} cnt64;

typedef struct _adslBertStatusEx {
    unsigned int       bertSecTotal;
    unsigned int       bertSecElapsed;
    unsigned int       bertSecCur;
    cnt64               bertTotalBits;
    cnt64               bertErrBits;
} adslBertStatusEx;


typedef struct _adslDataConnectionInfo {
    unsigned short      K;
    unsigned char       S, R, D;
} adslDataConnectionInfo;

typedef struct _adslConnectionInfo {
    unsigned char           chType;             /* fast or interleaved */
    unsigned char           modType;            /* modulation type: G.DMT or T1.413 */
    unsigned char           trellisCoding;      /* off(0) or on(1) */
    adslDataConnectionInfo  rcvInfo;
    adslDataConnectionInfo  xmtInfo;
    unsigned char           trellisCoding2;
} adslConnectionInfo;

typedef struct _adsl2DataConnectionInfo {
    unsigned char       Nlp;
    unsigned char       Nbc;
    unsigned char       MSGlp;
    unsigned short      MSGc;

    unsigned int        L;
    unsigned short      M;
    unsigned short      T;
    unsigned short      D;
    unsigned short      R;
    unsigned short      B;
} adsl2DataConnectionInfo;

/* ADSL2 connection parameters */

#define kAdsl2ModeAnnexMask                 0xF

#define kAdsl2ModeAnnexMUp32                1
#define kAdsl2ModeAnnexMUp36                2 
#define kAdsl2ModeAnnexMUp40                3 
#define kAdsl2ModeAnnexMUp44                4
#define kAdsl2ModeAnnexMUp48                5
#define kAdsl2ModeAnnexMUp52                6
#define kAdsl2ModeAnnexMUp56                7
#define kAdsl2ModeAnnexMUp60                8
#define kAdsl2ModeAnnexMUp64                9

#define kAdsl2ModeAnnexLShift               4
#define kAdsl2ModeAnnexLMask                (0x3 << kAdsl2ModeAnnexLShift)

#define kAdsl2ModeAnnexLUpMask              (0x00000001 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLUpWide              (0x00000000 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLUpNarrow            (0x00000001 << kAdsl2ModeAnnexLShift)

#define kAdsl2ModeAnnexLDnMask              (0x00000002 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLDnNonOvlap          (0x00000000 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLDnOvlap             (0x00000002 << kAdsl2ModeAnnexLShift)

#define kXdslModeAnnexShift                     8
#define kXdslModeAnnexMask                      (0xF << kXdslModeAnnexShift)

/* TPS-TC options */

#define kXdslTpsTcPtmPreemption				0x01
#define kXdslTpsTcPtmShortPackets			0x02

typedef struct UDenomNum16 UDenomNum16;
struct UDenomNum16
{
   unsigned short num; /*numerator*/
   unsigned short denom; /*denominator*/
};

typedef struct _xdslFramingInfo {
    unsigned short          N;          /* RS codeword size*/
    unsigned short          D;          /* interleaving depth: =1 for fast path */
    union {
    unsigned short          L16;
    unsigned short          Lrmc;       /* Gfast: RMC bits in RC symbol */
    };
    union {
    unsigned char           B[2];       /* nominal total of each bearer's octets per Mux Data Frame (Slightly redundant)*/
    struct {
        unsigned char       m;          /* Length of cyclic extension */
        unsigned char       MNDSNOI;    /* Min Data Symbols in NOI */
    };
    };
    union {
    unsigned char           I;          /* VDSL2: Interleaver block length */
    unsigned char           Rrmc;       /* Gfast: R in RMC symbol */
    };
    unsigned char           M;          /* ADSL2: Number of Mux frames per FEC Data frame; G.fast: Mds/Mus.*/
    union {
    unsigned char           T;          /* ADSL2: Number of Mux frames per sync octet*/
                                        /* VDSL2: Number of Mux data frames in an OH sub-frame */
    unsigned char           Mf;         /* Gfast: Common for DS and US */
    };
    union {
    unsigned char           G;          /* VDSL2: Notional number of OH bytes in an OH sub-frame - actual number of bytes in any sub-frame may 1 be greater than this */
    unsigned char           Msf;        /* Gfast: Common for DS and US */
    };
    union {
    unsigned short          U;
    unsigned short          Ldoi;       /* L in DOI; Bdoi = floor(Ldoi/8) */
    };
    union {
    unsigned char           F;          /* VDSL2: Number of OH frames in an OH superframe */
    unsigned char           Drmc;       /* Gfast: RMC symbol offset  */
    };
    unsigned char           R;          /* RS codeword overhead bytes */
    unsigned char           Q;          /* G.Inp: Number of RS CW per DTU (PhyR & G.Inp) */
    unsigned char           V;          /* G.Inp: Number of padding bytes per DTU */
    unsigned char           ovhType;
    unsigned char           ahifChanId[2];
    unsigned char           tmType[2];   /* ATM or ATM_Nitro or PTM */
    unsigned char           pathId;       /* "physical" LP number */

    unsigned char           rtxMode;        /* 0 - none, 1 - phyR, 2-16 (reserved fro phyR); 17 - Ginp modes */
    union {
    unsigned char           rxQueue;       /* length of the retransmission queue in Rx direction */
    unsigned char           ackWindowShift; /*Gfast */
    };
    unsigned char           txQueue;       /* length of the retransmission queue in Tx direction */
    unsigned char           rrcBits;     /* number of bits in the retransmission return channel */
    unsigned char           ginpLookBack;

    /* derived parameters */
    UDenomNum16          S;          /* S== number of PMD symbols over which the FEC data frame spans (=1 for G.dmt fast path, <=1 for ADSL2 fast path) */
    unsigned short          delay;      /* Q1 */
    unsigned short          INP;        /* Q1 */
    unsigned short          INPrein;    /* actual INP guaranteed on this latency path */
    unsigned char           A;            /* is the integer number of ATM cells or PTM codwords per DTU */
    int                        dataRate;
    int                        etrRate;
    unsigned char           tpsTcOptions;
    unsigned int            L;
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
    unsigned int            etru;
    unsigned int            ETRminEoc;
    unsigned short          Ldr;
#endif
} xdslFramingInfo;

// ADSL.K = B[0] + B[1] + 1
// VDSL.K = B[0] + B[1] + roundup(G/T); for ADSL2 G=1, so the formula works for ADSL2
// SEQp   = U*G
// ADSL2.MSGc = U*G - 6

typedef struct _xdslDirFramingInfo {
    int                       aggrDataRate;  /* aggregate date rate (sum for dual latency) */
    unsigned char          Nlp;                 /* # of "physical" latency paths == # of entries in lpInfo */
    unsigned char          NlpData;          /* # of data carrying latency paths */
    unsigned char          Nbc;                /* # of bearers = NlpData + #(of non zero B1) */
    unsigned char          MSGlp;            /* # of LP carrying OH data */
    xdslFramingInfo        lpInfo[2];
} xdslDirFramingInfo;

// xdslMode, pwrState, vdsl2Profile  -- logically don't belong in framing - left to be like before
typedef struct _xdslConnectionInfo {
    int                       xdslMode;
    unsigned char          pwrState;           /* Lx state: x = 0..3 */
    unsigned short         vdsl2Profile;
    xdslDirFramingInfo    dirInfo[2];          /* 0 - Rx, 1 - Tx */
} xdslConnectionInfo;


typedef struct _adsl2DelayInp{
    unsigned short delay;
    unsigned short inp;
} adsl2DelayInp;

typedef struct _adsl2ChanInfo {
    unsigned char ahifChanId;
    unsigned char connectionType;
} adsl2ChanInfo;

typedef struct _adsl2ConnectionInfo {
    int                    adsl2Mode;
    int                    rcvRate;
    int                    xmtRate;
    unsigned char           pwrState;           /* Lx state: x = 0..3 */
    adsl2DataConnectionInfo rcv2Info;
    adsl2DataConnectionInfo xmt2Info;
    adsl2DelayInp   rcv2DelayInp;
    adsl2DelayInp   xmt2DelayInp;
    adsl2ChanInfo   rcvChanInfo;
    adsl2ChanInfo   xmtChanInfo;
} adsl2ConnectionInfo;

typedef struct _adslConnectionDataStat {
    unsigned int           cntRS;  
    unsigned int           cntRSCor;   
    unsigned int           cntRSUncor; 
    unsigned int           cntSF;  
    unsigned int           cntSFErr;
} adslConnectionDataStat;

typedef struct _adslConnectionBitswapStat {
    unsigned int	rcvCnt;
    unsigned int	xmtCnt;
    unsigned int	rcvCntReq;
    unsigned int	xmtCntReq;
    unsigned int	rcvCntRej;
    unsigned int	xmtCntRej;
    unsigned int	status;         /* bit0/bit1 - Rx/Tx bitswap state */
    unsigned int	reserved;
} adslConnectionBitswapStat;

typedef struct _adslFireStat {
    unsigned int	status;
    unsigned int	reXmtRSCodewordsRcved;
    unsigned int	reXmtUncorrectedRSCodewords;
    unsigned int	reXmtCorrectedRSCodewords;
    unsigned int	reXmtRSCodewordsRcvedUS;
    unsigned int	reXmtUncorrectedRSCodewordsUS;
    unsigned int	reXmtCorrectedRSCodewordsUS;
} adslFireStat;

typedef struct _adslBondingStat {
    unsigned int   status;
    unsigned int   remoteDiscoveryRegisterLow;         /* 32 LSB of Remote_discovery_register */
    unsigned int   capabilityAndRemoteDiscoveryHigh;   /* 16 of capability register and 16 MSB of Remote_discovery_register */
    unsigned int   PMI_Aggregate_Register;
    unsigned int   PMI_Available_Register;
    unsigned int   Aggregation_Link_Register;
} adslBondingStat;

typedef struct
{
    unsigned int        rtx_tx;         /* Counter of retransmitted DTUs by the transmitter */
    unsigned int        rtx_c;          /* Counter of corrected DTUs at receiver */
    unsigned int        rtx_uc;         /* Counter of uncorrected DTUs at receiver */
    unsigned int        LEFTRS;         /* Low Error-Free Troughtput Rate Second */
    unsigned int        errFreeBits;    /* #bits belonging to correct DTU's leaving the Rx PMS-TC * 2^(-16)*/
    unsigned int        minEFTR;        /* Lowest EFTR observed in the current interval */
} ginpCounters;     /* No longer the same as GinpCounters in SoftDsl.h; SEFTR was added for driver's internal use */

#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
/* supportedOptions[0] */
#define GFAST_SUPPORTEDOPTIONS_RPFIB     0x01
#define GFAST_SUPPORTEDOPTIONS_INM       0x02
/* supportedOptions[1] */
#define GFAST_SUPPORTEDOPTIONS_DATAGRAM  (0x01 << 8)
#define GFAST_SUPPORTEDOPTIONS_RMCR      (0x02 << 8)

typedef struct
{
    unsigned int   bytesSent;
    unsigned int   bytesReceived;
    unsigned int   packetsSent;
    unsigned int   packetsReceived;
    unsigned int   messagesSent;
    unsigned int   messagesReceived;
} gfastEocCounters;

typedef struct
{
    unsigned int   txANDEFTRmin;
    unsigned int   txANDEFTRmax;
    unsigned int   txANDEFTRsum;
    unsigned int   txANDEFTRDS;
    unsigned int   txLANDEFTRS;
    unsigned int   rxANDEFTRmin;
    unsigned int   rxANDEFTRmax;
    unsigned int   rxANDEFTRsum;
    unsigned int   rxANDEFTRDS;
    unsigned int   rxLANDEFTRS;
} gfastCounters;
#endif

#define kXdslFireDsEnabled            0x1
#define kXdslFireUsEnabled            0x2
#define kXdslGinpDsEnabled            0x4
#define kXdslGinpUsEnabled            0x8

typedef struct _xdslGinpStat {
    unsigned int    status;
    ginpCounters    cntDS;
    ginpCounters    cntUS;
} xdslGinpStat;

typedef struct _adslConnectionStat {
    adslConnectionDataStat  rcvStat;
    adslConnectionDataStat  xmtStat;
    adslConnectionBitswapStat bitswapStat;
    adslFireStat                 fireStat;
    adslBondingStat              bondingStat;
    xdslGinpStat                 ginpStat;
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
    gfastEocCounters             eocStat;
    gfastCounters                gfastStat;
#endif
} adslConnectionStat;

typedef struct _atmConnectionDataStat {
    unsigned int           cntHEC;
    unsigned int           cntOCD;
    unsigned int           cntLCD;
    unsigned int           cntES;
    unsigned int           cntCellTotal;
    unsigned int           cntCellData;
    unsigned int           cntCellDrop;
    unsigned int           cntBitErrs;
} atmConnectionDataStat;

typedef struct _atmConnectionStat {
    atmConnectionDataStat   rcvStat;
    atmConnectionDataStat   xmtStat;
} atmConnectionStat;

#define kAdslFramingModeMask            0x0F
#define kAtmFramingModeMask             0xF0
#define kAtmHeaderCompression           0x80

/* ADSL2 data */

typedef struct _adslDiagModeData {
    int                    loopAttn;
    int                    signalAttn;
    int                    snrMargin;
    int                    attnDataRate;
    int                    actXmtPower;
    int                    hlinScaleFactor;
    unsigned short    ldLastStateDS;    /* DS LD last state transmitted */
    unsigned short    ldLastStateUS;    /* US LD last state transmitted */
} adslDiagModeData;

//#ifdef NTR_SUPPORT
#define kNtrOperMode6368            (0)
#define kNtrOperModeInt                (1)
#define kNtrOperModeExtDriver    (2)

#define kNtrStop                (18)
#define kNtrStart               (19)

typedef struct dslNtrCfg {
   unsigned int   intModeDivRatio;     /* NTR output freq = 17.664e6/intModeDivRatio */
   unsigned int   extModePhaseScale;   /* scale factor used to convert external clock ticks to 17.664 MHz clock ticks */
                                /* extModePhaseScale = 17.664/(external clock freq), 16.16 format */
   unsigned int   updatePeriod;        /* PLL update period in ms */
   int    b0;                  /* loop filter, b0       */
   int    b1;                  /* loop filter, b1       */
   unsigned char   operMode;
   unsigned char   b0_scale;            /* loop filter, b0 scale */
   unsigned char   b1_scale;            /* loop filter, b1 scale */
} dslNtrCfg;

typedef struct dslNtrData {
    unsigned int   mipsCntAtDmt;
    unsigned int   mipsCntAtNtr;
    unsigned int   lcoCntAtDmt;
    unsigned int   lcoCntAtNtr;
    unsigned int   ncoCntAtDmt;
    unsigned int   ncoCntAtNtr;
    /* 6362/6328 */
#if defined(CONFIG_BCM963268) || defined(CHIP_63268) ||\
    defined(CONFIG_BCM963138) || defined(CHIP_63138) || defined(CONFIG_BCM963381) || defined(CHIP_63381) ||\
    defined(CONFIG_BCM963148) || defined(CHIP_63148) || defined(CONFIG_BCM963158) || defined(CHIP_63158)
    int    phaseError;       /* 32.0 format */
    int    VCOAdjInfo;
#endif
} dslNtrData;

#ifndef VDSLTONEGROUP
#define VDSLTONEGROUP
typedef struct _VdslToneGroup {
   unsigned short            endTone;
   unsigned short            startTone;
} VdslToneGroup;
#endif
typedef struct _bandPlanDescriptor {
   unsigned char             noOfToneGroups;
   unsigned char             reserved;
   VdslToneGroup             toneGroups[MAX_NUM_BANDS];
} bandPlanDescriptor;

typedef struct _bandPlanDescriptor32 {
   unsigned char             noOfToneGroups;
   unsigned char             reserved;
   VdslToneGroup             toneGroups[MAX_NUM_SMALL_BANDS];
} bandPlanDescriptor32;

typedef struct _gFactorsEntry {
  short                      Gfactor_SUPPORTERCARRIERSds;
  short                      Gfactor_SUPPORTERCARRIERSus;
  short                      Gfactor_MEDLEYSETds;
  short                      Gfactor_MEDLEYSETus;
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
  short                      Gfactor_Gfast_mode;
#endif
} gFactorsEntry;

typedef struct _vdslperbandPMDdata {
    int        adslCurrSnrMgn;
    int        adslCurrAtn;
    int        adslSignalAttn;
} vdslperbandPMDdata;

#if defined(CONFIG_VDSL_SUPPORTED)

typedef struct {
    unsigned short                  N;
    unsigned short                  D;
    unsigned short                  L;
    unsigned char                   B[2];
    unsigned char                   I;
    unsigned char                   M;
    unsigned char                   T;
    unsigned char                   G;
    unsigned short                  U;
    unsigned char                   F;
    unsigned char                   R;
    unsigned char                   ovhType;
    unsigned char                   ahifChanId[2];
    unsigned char                   tmType[2];   // ATM or ATM_Nitro or PTM
    unsigned char                   pathId;
} vdslMuxFramerParamType;

typedef struct _vdsl2ConnectionInfo {
    int                    vdsl2Mode;
    int                    rcvRate;
    int                    xmtRate;
    unsigned char           pwrState;           /* Lx state: x = 0..3 */
    vdslMuxFramerParamType  rcv2Info;
    vdslMuxFramerParamType  xmt2Info;
    adsl2DelayInp   rcv2DelayInp;
    adsl2DelayInp   xmt2DelayInp;
    unsigned short  vdsl2Profile;
} vdsl2ConnectionInfo;


#if defined(SUPPORT_VECTORING)

#define MIB_LOG_PILOT_SEQUENCE_LEN 10
#define MIB_PILOT_SEQUENCE_LEN     (1<<MIB_LOG_PILOT_SEQUENCE_LEN)

typedef struct _MibPilotSequence {
    unsigned short firstSync;                          /* Position in the sequence of
                                                 first sync symbol in showtime
                                                 (Unused at the CO side) */
    unsigned char bitsPattern[MIB_PILOT_SEQUENCE_LEN>>3]; /* One sign bit per modulation */
    unsigned short pilotSeqLengthInBytes;
} MibPilotSequence;

typedef struct _FourBandsDescriptor{
  unsigned char  noOfToneGroups;
  unsigned char  noCheck; /* bypass validation logic (should only be used in lineTest and loopback modes) */
  VdslToneGroup toneGroups[4];
} MibFourBandsDescriptor;

typedef struct _VectDataPhy {
    MibPilotSequence pilotSequence;
    MibFourBandsDescriptor vectoringBandPlan;
    unsigned short syncOffset;
} MibVectDataPhy;

#ifndef VECTORING_STRUCT_H
#define VECTORING_STRUCT_H

/* Storage for Vectoring values */
#define MAX_VECT_TONES_DS (47*32)
/* Maximal overhead of error samples report     */
/*  - ERB byte :  1 byte                        */
/*  - overhead per band VBB+Bm = 3 bytes        */
/* Those bytes shall be added by the function   */
/* generateErrorReport                          */
/* Maximal number of bands = 8                  */
#define ERB_OVERHEAD  (8*3+1)

#define MAX_VECT_DUMP_SEG   16
#define MAX_VECT_DUMP_TONES MAX_VECT_DUMP_SEG*256
#define MAX_VECT_TONES MAX_VECT_TONES_DS

#define ROUNDUP32(a) ((((a)+31)>>5)<<5)

/* Tones and memory constant relative to VECTORING */
/* Minimal downsampling of the SYNC symbols */
#define MIN_LOG2_SUB_SYNC_US 2
#define MIN_LOG2_SUB_SYNC_DS 1

#define MIN_LOG2_SUB_SYNC_RX MIN_LOG2_SUB_SYNC_DS

#define NO_RX_TONES_5_BAND 1100
#define MAX_VDSL_RX_TONES NO_RX_TONES_5_BAND

typedef struct _ConversionInfoBand {
  unsigned short startRx;
  unsigned short nSkipped;
  unsigned short nTonesInBand;
  unsigned short nSamples;
} ConversionInfoBand;

typedef struct ConversionInfo ConversionInfo;
struct ConversionInfo
{
  unsigned char  nBands;
  ConversionInfoBand infoBand[4];
};

typedef struct _VectorErrorSample
{
  unsigned short lineId;
  unsigned short syncCounter;
  unsigned short nERBbytes;      /* Number of ERB bytes in the buffer */
  /* Error in interleaved format */
  unsigned char  errorMsg[MAX_VECT_TONES*2+ERB_OVERHEAD];
} VectorErrorSample;

typedef struct StartVectorDump StartVectorDump;
struct StartVectorDump
{
  unsigned char  direction; /* 0: upstream 1: downstream                     */
  unsigned char  reservedA;
  unsigned short startSync; /* Index of the first dump, startSync            */
                    /* if set to 0xFFFF, dump as soon as possible    */
  unsigned short numSymbol; /* Maximal number of symbol to dump              */
  unsigned short startTone; /* Start frequency to dump, must be a multiple of 8         */
  unsigned short numTones;  /* Number of samples to be dumped, must be a multiple of 8
                     * */
  unsigned short lineId;    /* Index of the line */
  unsigned char  log2M;     /* Only used at the VTU-R side */
  unsigned char  reservedB;
};

typedef struct VectorMode VectorMode;
struct VectorMode
{
  unsigned char direction;  /* 0 upstream 1: downstream */
  unsigned char disableRxBitSwap;
  unsigned char disableVN;
  unsigned char reservedA;
};

/* States */
#define VECT_WAIT_FOR_CONFIG  0
#define VECT_FULL             1
#define VECT_WAIT_FOR_TRIGGER 2
#define VECT_RUNNING          3
#define VECT_DISABLED         4
#define VECT_UNCONFIGURED     5

typedef struct _VectoringStateMachine {
  unsigned char       state;
  unsigned char       mode;
  unsigned short      syncCounter;
  
  /* Field modified by HMI messages */
  unsigned short      startSync; /* First tone that is dumped */
  unsigned char       lastReportedBand;
  unsigned short      numERBbytes;
  ConversionInfo      convInfo;     /* info to make the conversion between the
                                       rx bandplan and the vectoring bandplan */
  unsigned char               log2M;
  unsigned short              lineId;
} VectoringStateMachine;
#endif /* VECTORING_STRUCT_H */

typedef struct _VceMacAddress { 
  unsigned char  macAddress[6];
  unsigned char  addressType;   /* 0: MAC address */
                        /* 1: IP address  */
} VceMacAddress;

typedef struct _VectStat {
    unsigned int   cntESPktSend;	/* Number of packets sent to the WAN device */
    unsigned int   cntESPktDrop;
    unsigned int   cntESStatSend;	/* Number of error samples statuses sent */
    unsigned int   cntESStatDrop;
} VectStat;

typedef struct _VectData {
    VectorMode vectorMode;
    VceMacAddress macAddress;
    VectStat vectStat;
} VectData;

#endif /* SUPPORT_VECTORING */
#endif /* CONFIG_VDSL_SUPPORTED */

#ifdef SUPPORT_24HR_CNT_STAT
typedef struct _StatHistHrCounters {
    unsigned int    Syncs;
    unsigned int    ReceiveBlocks;
    unsigned int    TransmitBlocks;
    unsigned int    CellDelin;
    unsigned int    LinkRetrain;
    unsigned int    InitErrors;
    unsigned int    InitTimeouts;
    unsigned int    LossOfFraming;
    union {
    unsigned int    ErroredSecs;
    unsigned int    adslESs;
    };
    union {
    unsigned int    SeverelyErroredSecs;
    unsigned int    adslSES;
    };
    unsigned int    FECErrors;
    unsigned int    ATUCFECErrors;
    unsigned int    HECErrors;
    unsigned int    ATUCHECErrors;
    unsigned int    CRCErrors;
    unsigned int    ATUCCRCErrors;
    
/* Not required, but defined for code compatibility */
    unsigned int       adslLOMS;  /* Count of LOM seconds */
    unsigned int       adslLOSS;   /* Count of LOS seconds */
    unsigned int       adslFECs;   /* Count of FEC seconds  */
    unsigned int       adslLCDS;   /* Count of LCD Errored Seconds */
} StatHistHrCounters;

typedef struct _StatHist24HrCounters {
    int                         curHourIndex;
    unsigned int         cur1HourTimeElapsed;
    StatHistHrCounters  statHistHrCounters[24];
} StatHist24HrCounters;
#endif

/* AdslMibGetObjectValue return codes */

#define kAdslMibStatusSuccess           0
#define kAdslMibStatusFailure           -1
#define kAdslMibStatusNoObject          -2
#define kAdslMibStatusObjectInvalid     -3
#define kAdslMibStatusBufferTooSmall    -4
#define kAdslMibStatusLastError         -4

/* Adsl training codes */

#define kAdslTrainingIdle               0
#define kAdslTrainingG994               1
#define kAdslTrainingG992Started        2
#define kAdslTrainingG992ChanAnalysis   3
#define kAdslTrainingG992Exchange       4
#define kAdslTrainingConnected          5
#define kAdslTrainingG993Started        6
#define kAdslTrainingG993ChanAnalysis   7
#define kAdslTrainingG993Exchange       8

/* TPS-TC types define */
#define kXdslDataRaw		0
#define kXdslDataAtm		1
#define kXdslDataPtm1		2
#define kXdslDataPtm2		3
#define kXdslDataPtm		kXdslDataPtm2
#define kXdslDataNitro		4

/* Xdsl Initialization sucess/failure cause */
#define kXdslInitConfigSuccess			0
#define kXdslInitConfigError			1
#define kXdslInitConfigNotFeasible		2
#define kXdslInitConfigCommProblem		3
#define kXdslInitConfigNoXTUDetected	4
#define kXdslInitConfigUnknownFailure	5

#define kXdslLastInitStateStart		0
#define kXdslLastInitStateShowtime	1
#define kXdslLastInitStateNotFeasible	2
#define kXdslLastInitStateCommProblem	3

#define NO_PEER_DETECT_TIMEOUT		(120*1000)	/* Number of ms */

/* Key challenge defines */
#define kKeyChalStrSize		128
#define kRspChalInvalid		0
#define kRspChal		1
#define kRspChalOK		2
#define kRspChalNOK		3
typedef struct _rsaKeyInfo {
	unsigned char	keyChallengeStr[kKeyChalStrSize];
	unsigned char	keyChallengeState;
} RsaKeyInfo;

#ifdef CONFIG_TOD_SUPPORTED
typedef struct {
	unsigned long long	secondsSinceEpoch;
	unsigned int		nanoSecondsSinceEpoch;
} TodTimeStampRpt;

typedef struct {
	TodTimeStampRpt	todTimeStampRpt;
	unsigned char	todStatus;  /* 0 - inactive; 1 - active(for bonding targets, 1 - active on line0, 2 - active on line1 */
} TodInfo;
#endif

/* Global info structure */

typedef struct _adslMibInfo {
	adslLineEntry			adslLine;
	union{
		adslPhysEntry			adslPhys;
#if defined(CONFIG_VDSL_SUPPORTED)
		xdslPhysEntry			xdslPhys;
#endif
	};
	union {
		adslChanEntry		xDslChan[2];
		struct {
		adslChanEntry		adslChanIntl;
		adslChanEntry		adslChanFast;
		};
	};
	adslPerfDataEntry		adslPerfData;
	adslPerfCounters		adslPerfIntervals[kAdslMibPerfIntervals];

	rtxCounterInfoEntry		rtxCounterData;
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST) || defined(WINNT) || defined(LINUX_DRIVER)
	union {
	gfastOlrCounterInfoEntry	gfastOlrCounterData;
	gfastOlrCounterInfoEntry	gfastOlrXoiCounterData[2];	/* 0-NOI, 1-DOI */
	};
#endif

	union {
		adslChanPerfDataEntry	xdslChanPerfData[MAX_LP_NUM];
		struct {
		adslChanPerfDataEntry	adslChanIntlPerfData;
		adslChanPerfDataEntry	adslChanFastPerfData;
		};
	};
	union {
		adslChanCounters		xdslChanPerfIntervals[2][kAdslMibChanPerfIntervals];
		struct {
		adslChanCounters		adslChanIntlPerfIntervals[kAdslMibChanPerfIntervals];
		adslChanCounters		adslChanFastPerfIntervals[kAdslMibChanPerfIntervals];
		};
	};
	adslThreshCounters		adslAlarm;
	union {
		atmPhyDataEntrty		xdslChanAtmPhyData[MAX_LP_NUM];
		struct {
		atmPhyDataEntrty		adslChanIntlAtmPhyData;
		atmPhyDataEntrty		adslChanFastAtmPhyData;
		};
	};
	adslBertResults			adslBertRes;
	union {
		adslConnectionInfo		xdslConnection[MAX_LP_NUM];
		adslConnectionInfo		adslConnection;
	};
	union {
		adslConnectionStat	xdslStat[MAX_LP_NUM];
		adslConnectionStat	adslStat;
	};
	union {
		adslConnectionStat	xdslStatSincePowerOn[MAX_LP_NUM];
		adslConnectionStat	adslStatSincePowerOn;
	};
	unsigned char		adslTrainingState;
	unsigned char		xdslInitializationCause;
	union {
		atmConnectionStat	atmStat2lp[MAX_LP_NUM];
		atmConnectionStat	atmStat;
	};
	union {
		atmConnectionStat	atmStatSincePowerOn2lp[MAX_LP_NUM];
		atmConnectionStat	atmStatSincePowerOn;
	};
	atmConnectionStat	atmStatCurDay2lp[MAX_LP_NUM];
	atmConnectionStat	atmStatPrevDay2lp[MAX_LP_NUM];
	atmConnectionStat	atmStatCur15Min2lp[MAX_LP_NUM];

	union{
	adslFullPhysEntry		adslAtucPhys;
#if defined(CONFIG_VDSL_SUPPORTED)
	xdslFullPhysEntry		xdslAtucPhys;
#endif
	};
	unsigned char			adslRxNonStdFramingAdjustK;
	unsigned char			adslFramingMode;
	adslBertStatusEx		adslBertStatus;
	int					afeRxPgaGainQ1;
	union {
		adslDiagModeData		vdslDiag[MAX_NUM_BANDS];
		adslDiagModeData		adslDiag;
	};
	union {
#if defined(CONFIG_VDSL_SUPPORTED)
		vdsl2ConnectionInfo	vdslInfo[MAX_LP_NUM];
#endif
		adsl2ConnectionInfo	adsl2Info2lp[MAX_LP_NUM];
		adsl2ConnectionInfo	adsl2Info;
	};
	xdslConnectionInfo	xdslInfo;
	
	adslPerfCounters			adslTxPerfTotal;
	adslPerfCounters			adslTxPerfCur15Min;
	adslPerfCounters			adslTxPerfCur1Day;
	adslPerfCounters			adslTxPerfLast15Min;
	adslPerfCounters			adslTxPerfLast1Day;
	adslPerfCounters			adslTxPerfSinceShowTime;
	adslPerfCounters			adslTxPerfLast;
	adslNonLinearityData		adslNonLinData;
	vdslperbandPMDdata		perbandDataUs[MAX_NUM_BANDS];
	vdslperbandPMDdata		perbandDataDs[MAX_NUM_BANDS];
	

	bandPlanDescriptor			usNegBandPlan;
	bandPlanDescriptor			dsNegBandPlan;
	bandPlanDescriptor32		usNegBandPlan32;
	bandPlanDescriptor32		dsNegBandPlan32;
	bandPlanDescriptor			usPhyBandPlan;
	bandPlanDescriptor			dsPhyBandPlan;
	bandPlanDescriptor			usNegBandPlanDiscovery;
	bandPlanDescriptor			dsNegBandPlanDiscovery;
	bandPlanDescriptor32			usNegBandPlanDiscovery32;
	bandPlanDescriptor32			dsNegBandPlanDiscovery32;
	bandPlanDescriptor			usPhyBandPlanDiscovery;
	bandPlanDescriptor			dsPhyBandPlanDiscovery;
	gFactorsEntry				gFactors;
#if defined(SUPPORT_VECTORING)
	VectData				vectData;
#endif
	unsigned char			lp2Active;
	unsigned char			lp2TxActive;
	adslPLNDataEntry			adslPLNData;
//#if defined(NTR_SUPPORT)
	dslNtrData					ntrCnt;
	dslNtrCfg					ntrCfg;
#ifdef SUPPORT_SELT
	SeltData				selt;
#endif
	unsigned int					afeId[2];
	int					transceiverClkError;	/* Q8 format */
#ifdef SUPPORT_24HR_CNT_STAT
	StatHist24HrCounters	statHist24HrCounters;
#endif
#ifdef SAVE_CRATESRA_MSG
	unsigned char		adslCRatesRAMsg[120];
#endif
	unsigned char IkanosCO4Detected;
#ifdef SUPPORT_VECTORING
	unsigned char reportVectoringCounter;
	unsigned char fdps_us;
#endif
	unsigned int maxBondingDelay;
	gFactorsEntry			physGfactors;
	unsigned char			fastRetrainActive;
#if defined(CONFIG_BCM963158)
	unsigned int			xdslSecondaryAfeId[2];
#endif
#if defined(SUPPORT_DSL_GFAST) || defined(CONFIG_BCM_DSL_GFAST)
	unsigned int gfastSupportedOptions;
#endif
} adslMibInfo;

#if defined(__cplusplus)
}
#endif

#endif  /* AdslMibDefHeader */
