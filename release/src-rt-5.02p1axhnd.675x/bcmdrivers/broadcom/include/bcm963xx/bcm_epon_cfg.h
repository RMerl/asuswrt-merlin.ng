/*
<:copyright-BRCM:2007:proprietary:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

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
*/

#ifndef _BCM_EPON_CFG_H_
#define _BCM_EPON_CFG_H_

#include "bcmtypes.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define EPON_OAM_IF_NAME "oam" //OAM interface name

#define GetEponDefActivetime()        0  
#define GetEponDefStandbytime()       0

#define GetEponDefPsConfig()          PsConfigNone
#define GetEponDefPsMode()            PsModeInternal

#define GetEponDefShaperIpg()         FALSE

#define GetEponDefLinkNum()           1

#define GetEponDefFec10gRxCfg()       TkOnuTxLlidsBitMap
#define GetEponDefFec10gTxCfg()       TkOnuTxLlidsBitMap


#ifdef CONFIG_EPON_10G_SUPPORT 
#define GetEponDefDnRateMap(a)        (PonRateTenG | PonRateTwoG | PonRateOneG)
#define GetEponDefUpRateMap(a)        (PonRateTenG | PonRateOneG)
#else
#define GetEponDefDnRateMap(a)        (PonRateOneG |PonRateTwoG)
#define GetEponDefUpRateMap(a)        PonRateOneG
#endif
#define GetEponDefLaserOn(a)          0x0020
#define GetEponDefLaserOff(a)         0x0020
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158)
#define GetEponDefTxPolarity(a)       ActiveLo
#else
#define GetEponDefTxPolarity(a)       ActiveHi
#endif
#define GetEponDefOffTimeOffset(a)    0x0  //xnue:0x5
#define GetEponDefMaxFrameSize(a)     EponMaxFrameSize
#define GetEponDefPreDraft2dot1(a)    0
#define GetEponDefPowerupTime(a)      0
#define GetEponDefTxOffIdle(a)        1
#define GetEponDefNttReporting(a)     0
#define GetEponDefSchMode()           0
#define GetEponDefIdleTimeOffset()    0

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158)
#define EponMaxFrameSize10G    10000

#define TkOnuFullFeatureLlids   32
#define TkOnuNumTxLlids         TkOnuFullFeatureLlids
#define TkOnuNumRxLlids         TkOnuFullFeatureLlids
#define TkOnuNumTotalLlids      TkOnuFullFeatureLlids
#define TkOnuMaxRxOnlyLlids     TkOnuFullFeatureLlids
#define TkOnuInvalidLlid        TkOnuFullFeatureLlids
#define TkOnuRsvNumRxOnlyLlids  1

#if defined(__KERNEL__)
extern unsigned int rtMaxRxOnlyLlids;
extern unsigned int rtFirstRxOnlyLlid;
#define TkOnuFirstRxOnlyLlid    rtFirstRxOnlyLlid
#define TkOnuNumRxOnlyLlids     (rtMaxRxOnlyLlids-TkOnuFirstRxOnlyLlid)
#define TkOnuLastRxOnlyLlid     (rtMaxRxOnlyLlids - 1)
#define TkOnuNumBiDirLlids      TkOnuFirstRxOnlyLlid
#define TkOnuDefaultRxOnlyLlid   (rtMaxRxOnlyLlids - TkOnuRsvNumRxOnlyLlids)
#define TkOnuDynStartRxOnlyLlid  TkOnuFirstRxOnlyLlid
#define TkOnuDynEndRxOnlyLlid    (TkOnuDefaultRxOnlyLlid - 1)
#endif

#define EponNumL2Queues         32
#define EponAllL2Qs             0xFFFFFFFFUL
#define EponUpstreamShaperCount 32
#define TkOnuTxLlidsBitMap      0xFFFFFFFFU
#define AllLinks                0xFFFFFFFFUL

#define Bcm10gOnuNumBiDirLlids   TkOnuNumTxLlids
#define Bcm1gOnuNumBiDirLlids    16
#define Ctc8pOnuNumBiDirLlids    3
#define Ctc4pOnuNumBiDirLlids    6
#define Ctc3pOnuNumBiDirLlids    8

#else /* !defined(CONFIG_BCM96858) &&  !defined(CONFIG_BCM96856) */

#define TkOnuNumTxLlids        8
#define TkOnuNumRxLlids        16
#define TkOnuNumTotalLlids     24

#define TkOnuFirstRxOnlyLlid   16
#define TkOnuNumRxOnlyLlids    8
#define TkOnuLastRxOnlyLlid    (TkOnuNumTotalLlids - 1)
#define TkOnuInvalidLlid       TkOnuNumTotalLlids
#define TkOnuMaxRxOnlyLlids    TkOnuNumTotalLlids
#define TkOnuDefaultRxOnlyLlid   TkOnuFirstRxOnlyLlid
#define TkOnuDynStartRxOnlyLlid  (TkOnuDefaultRxOnlyLlid + 1)
#define TkOnuDynEndRxOnlyLlid    TkOnuLastRxOnlyLlid

#define Bcm1gOnuNumBiDirLlids   8
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
#define Ctc8pOnuNumBiDirLlids    1
#define Ctc4pOnuNumBiDirLlids    2
#define Ctc3pOnuNumBiDirLlids    2
#endif

#define TkOnuRsvNumRxOnlyLlids  1


#define TkOnuNumBiDirLlids     TkOnuNumTxLlids

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
#define EponNumL2Queues        8
#else
#define EponNumL2Queues        24
#endif
#define EponAllL2Qs            0xFFFFFFUL
#define EponUpstreamShaperCount 8
#define TkOnuTxLlidsBitMap     0x00FFU
#define AllLinks               0x00FFUL
#endif

#define TkOnuMaxBiDirLlids     TkOnuNumTxLlids
#define TkOnuFirstRxLlid       0
#define TkOnuFirstTxLlid       0

// total number of RAM Blocks
#define TkOnuNumUpQueues       32
#define TkOnuNumDnQueues       20
#define EponNumL1Queues        32
#define EponAllL1Qs            0xFFFFFFFFUL
#define EponMaxL1AccNum 32
#define EponMaxPri 8
#define EponMaxFrameSize       2000

#define ActDelAllMcastLlid      0xFEU
#define InvalidPhyLlid          0x0000
#define InvalidMcastIdx         0xFFU
typedef enum
{
    ActGetMcastLlidAndIdxByUcastIdx = 0x00, /* associated mode ONLY */
    ActGetMcastIdxByMcastLlid,
    ActGetMcastLlidByMcastIdx
} ActGetMcastFlag;



#if defined(__cplusplus)
}
#endif

#endif //_BCM_EPON_CFG_H_

