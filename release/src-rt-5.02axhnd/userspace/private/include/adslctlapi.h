/***********************************************************************
 *
 *  Copyright (c) 2000-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/


#ifndef __ADSL_CTL_API_H__
#define __ADSL_CTL_API_H__
#ifdef BRCM_CMS_BUILD
#include "cms.h"
#else
#include "devctl_adsl.h"
#endif
#include "AdslMibDef.h"

#define ADSL_LINE_INNER_PAIR 1
#define ADSL_LINE_OUTER_PAIR 2
#define ADSL_BERT_STATE_STOP 0
#define ADSL_BERT_STATE_RUN 1

#define DIAG_DSL_LOOPBACK_TIMEOUT_PERIOD 150 /* maximum 2.5 minutes */
#define DIAG_DSL_LOOPBACK_ERROR    -1  /* test complete but failed */
#define DIAG_DSL_LOOPBACK_ERROR_BUT_RETRY    -2  /* test complete but failed, new session in progress */
#define DIAG_DSL_LOOPBACK_TIMEOUT   0  /* test not complete, time out */
#define DIAG_DSL_LOOPBACK_IN_PROGRESS  1  
#define DIAG_DSL_LOOPBACK_SUCCESS   2  /* test complete and successful */

#define DIAG_DSL_SELT_TIMEOUT_PERIOD         50
#define DIAG_DSL_SELT_STATE_IN_PROGRESS      0
#define DIAG_DSL_SELT_STATE_TEST_COMPLETE    2
#define DIAG_DSL_SELT_STATE_POST_PROCESS_COMPLETE    1
#define DIAG_DSL_SELT_CALIBRATION_NO         0
#define DIAG_DSL_SELT_CALIBRATION_YES        1
#define DIAG_DSL_SELT_RESULT_PROCESS_YES     1
#define DIAG_DSL_SELT_RESULT_PROCESS_NO      0
#define DIAG_DSL_SELT_MAX_TESTS_SAVED        5 /* for one line */
#define DIAG_DSL_SELT_PERSISTENT_DIR_PATH    "/data/selt"
#define DIAG_DSL_SELT_RESULT_PERSISTENT_DIR_PATH    "/data/selt/result"
#define DIAG_DSL_SELT_LOG_PERSISTENT_DIR_PATH       "/data/selt/log"
#define DIAG_DSL_SELT_RESULT_TMP_DIR_PATH    "/var"
#define DIAG_DSL_SELT_RESULT_FILE_NAME_PREFIX  "dataOut"  /* it's dataOut_lineId.txt */
#define DIAG_DSL_SELT_MEASUREMENT_STEPS      0x18


// modulation mode -- up to last 6 bytes
#define ANNEX_A_MODE_GDMT        0x00000001
#define ANNEX_A_MODE_GLITE       0x00000002
#define ANNEX_A_MODE_T1413       0x00000004
#define ANNEX_A_MODE_ADSL2       0x00000008
#define ANNEX_A_MODE_ANNEXL      0x00000010
#define ANNEX_A_MODE_ADSL2PLUS   0x00000020
#define ANNEX_A_MODE_ANNEXM      0x00000040        // default ANNEXM is DISABLED. (bit clear)
#define ANNEX_A_MODE_VDSL2       0x00000080
#define ANNEX_A_MODE_GFAST       0x10000000

#define ANNEX_A_MODE_ALL_MOD     (ANNEX_A_MODE_GDMT | ANNEX_A_MODE_GLITE | ANNEX_A_MODE_T1413 | \
                                 ANNEX_A_MODE_ADSL2 | ANNEX_A_MODE_ANNEXL)

// 6358 has the adsl-plus
#define ANNEX_A_MODE_ALL_MOD_48  (ANNEX_A_MODE_ALL_MOD | ANNEX_A_MODE_ADSL2PLUS)

#define		VDSL_PROFILE_SHIFT	8
#define		VDSL_PROFILE_8a		0x00000100
#define		VDSL_PROFILE_8b 		0x00000200
#define		VDSL_PROFILE_8c 		0x00000400
#define		VDSL_PROFILE_8d 		0x00000800
#define		VDSL_PROFILE_12a		0x00001000
#define		VDSL_PROFILE_12b		0x00002000
#define		VDSL_PROFILE_17a		0x00004000
#define		VDSL_PROFILE_30a		0x00008000
#define		VDSL_PROFILE_MASK		(VDSL_PROFILE_8a | VDSL_PROFILE_8b | VDSL_PROFILE_8c |VDSL_PROFILE_8d |\
								VDSL_PROFILE_12a | VDSL_PROFILE_12b | VDSL_PROFILE_17a | VDSL_PROFILE_30a)
#define		VDSL_US0_SHIFT		16
#define		VDSL_US0_8a			0x00010000
#define		VDSL_US0_8b			0x00020000
#define		VDSL_US0_8c			0x00040000
#define		VDSL_US0_8d			0x00080000
#define		VDSL_US0_12a			0x00100000
#define		VDSL_US0_12b			0x00200000
#define		VDSL_US0_17a			0x00400000
#define		VDSL_US0_30a			0x00800000
#define		VDSL_US0_MASK		(VDSL_US0_8a | VDSL_US0_8b | VDSL_US0_8c | VDSL_US0_8d |\
								VDSL_US0_12a | VDSL_US0_12b | VDSL_US0_17a)
#ifndef SUPPORT_DSL_GFAST
#define ANNEX_A_MODE_ALL_MOD_68  (ANNEX_A_MODE_ALL_MOD_48 | ANNEX_A_MODE_VDSL2)
#else
#define ANNEX_A_MODE_ALL_MOD_68  (ANNEX_A_MODE_ALL_MOD_48 | ANNEX_A_MODE_VDSL2 | ANNEX_A_MODE_GFAST)
#endif

// line pair define  -- start on 6th byte
#define ANNEX_A_LINE_PAIR_INNER     0x00000000           // default -- INNER PAIR 
#define ANNEX_A_LINE_PAIR_OUTER     0x01000000   
// bitswap bit
#define ANNEX_A_BITSWAP_ENABLE      0x00000000           // default -- enabled 
#define ANNEX_A_BITSWAP_DISENABLE   0x02000000
// SRA bit
#define ANNEX_A_SRA_DISENABLE       0x00000000           // default -- disenabled 
#define ANNEX_A_SRA_ENABLE          0x08000000           // enable

// ANNEX_C mode bit
#define ANNEX_C_BITMAP_DBM          0x00000000           // default
#define ANNEX_C_BITMAP_FBM          0x04000000           // 

#define NUM_TONE_GROUP              512
#define MAX_PS_STRING               61430
#define MAX_LOGARITHMIC_STRING      2559
#define MAX_QLN_STRING              2047
#define NUM_BAND                    5

typedef struct dslLoopDiagData
{
   UBOOL8   testCompleted;
   /* gfactor for VDSL only */
   UINT16   HLINGds;                           /* gfactorSupporterCarrierDs */
   UINT16   HLINGus;                           /* gfactorSupporterCarrierUs */
   UINT16   HLOGGds;                           /* gfactorSupporterCarrierDs */
   UINT16   HLOGGus;                           /* gfactorSupporterCarrierUs */
   UINT16   QLNGds;                            /* gfactorSupporterCarrierDs */
   UINT16   QLNGus;                            /* gfactorSupporterCarrierUs */
   UINT16   SNRGds;                            /* gfactorMedleySetDs */
   UINT16   SNRGus;                            /* gfactorMedleySetUs */
   /* per-subcarrier group objects */
   SINT32   HLINpsds[NUM_TONE_GROUP];          /* chanCharLinDsPerToneGroup */
   SINT32   HLINpsus[NUM_TONE_GROUP];          /* chanCharLinUsPerToneGroup */
   SINT16   QLNpsds[NUM_TONE_GROUP];           /* quietLineNoiseDsPerToneGroup */
   SINT16   QLNpsus[NUM_TONE_GROUP];           /* quietLineNoiseUsPerToneGroup */
   SINT16   SNRpsds[NUM_TONE_GROUP];           /* SNRDsPerToneGroup */
   SINT16   SNRpsus[NUM_TONE_GROUP];           /* SNRUsPerToneGroup */
   SINT16   HLOGpsds[NUM_TONE_GROUP];          /* chanCharLogDsPerToneGroup */
   SINT16   HLOGpsus[NUM_TONE_GROUP];          /* chanCharLogUsPerToneGroup */
   SINT8    BITSpsds[NUM_TONE_GROUP];          /* bitAllocDsPerToneGroup */
   SINT16   GAINSpsds[NUM_TONE_GROUP];         /* gainDsPerToneGroup */
   /* per-band objects (VDSL) */
   UINT16   LATNds[NUM_BAND];                  /* LATNdsperband */
   UINT16   LATNus[NUM_BAND];                  /* LATNusperband */
   UINT16   SATNds[NUM_BAND];                  /* SATNdsperband */
   UINT16   SATNus[NUM_BAND];                  /* SATNusperband */
   UINT16   SNRMpbds[NUM_BAND];                /* SNRMdsperband */
   UINT16   SNRMpbus[NUM_BAND];                /* SNRMusperband */
   /* */
   SINT32   ACTATPds;                          /* atucPhys curr output power */
   SINT32   ACTATPus;                          /* adslPhys curr output power */
   SINT32   HLINSCds;                          /* adslPhys Hlin Scale factor */
   SINT32   HLINSCus;                          /* adslPhys Hlin Scale factor */
   SINT32   ACTPSDds;                          /* phy */
   SINT32   ACTPSDus;                          /* phy */
   SINT32   HLOGMTds;                          /* phy, 32 */
   SINT32   HLOGMTus;                          /* phy, 32 */
   SINT32   QLNMTds;                           /* phy, 128 */
   SINT32   QLNMTus;                           /* phy, 128 */
   SINT32   SNRMTds;                           /* phy, 128 */
   SINT32   SNRMTus;                           /* phy, 128 */
} DslLoopDiagData, *PDslLoopDiagData;

CmsRet cmsAdsl_getAdslMib(adslMibInfo *adslMib);
CmsRet BcmAdslCtl_GetBertStatus(adslBertStatusEx *bertStatus);
void cmsAdsl_getPhyVersion(char *version, int len);

//CmsRet cmsAdsl_getConnectionInfo( PADSL_CONNECTION_INFO pConnInfo );
CmsRet cmsAdsl_initialize(adslCfgProfile *pAdslCfg);
CmsRet cmsAdsl_uninitialize(void);
CmsRet cmsAdsl_start(void);
CmsRet cmsAdsl_stop(void);
CmsRet cmsAdsl_getAdslMib(adslMibInfo *adslMib);
CmsRet cmsAdsl_ResetStatCounters(void);
CmsRet cmsAdsl_BertTestStart(UINT32  duration);
CmsRet cmsAdsl_BertTestStop(void);
CmsRet cmsAdsl_getAdslMibObject(char *oidStr, int oidLen, char *data, long *dataLen);
CmsRet cmsAdsl_setTestDiagMode(void);

extern void parseAdslInfo(char *info, char *var, char *val, int len);

CmsRet cmsAdsl_setAdslMibObject(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
CmsRet cmsAdsl_formatSubCarrierDataStringU(char **paramValue, SINT32 *data, int maxDataStrLen);
CmsRet cmsAdsl_formatHLINString(char **paramValue, SINT32 *data, int maxDataStrLen);
CmsRet cmsAdsl_formatSubCarrierDataString(char **paramValue, void *data, char *paramName, int maxDataStrLen);
CmsRet cmsAdsl_formatPertoneGroupQ4String(char **paramValue, void *data, int maxDataStrLen);

char* QnToString(SINT32 val, int q);
SINT32 Qn2DecF(SINT32 qnVal, int q);
SINT32 Qn2DecI(SINT32 qnVal, int q);

#endif /*  __ADSL_CTL_API_H__ */
