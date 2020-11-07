/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
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


#ifndef Epon_Stat_Id_h
#define Epon_Stat_Id_h


typedef enum
    {
    StatStr64,
    StatStr0_127,
    StatStr65_127,
    StatStr128_255,
    StatStr256_383,
    StatStr384_511,
    StatStr512_639,
    StatStr640_767,
    StatStr768_895,
    StatStr896_1023,
    StatStr1024_1151,
    StatStr1152_1279,
    StatStr1280_1407,
    StatStr1408_1535,
    StatStr1536_1663,
    StatStr1664_1791,
    StatStr1792_1919,
    StatStr1920_2047,
    StatStr64_511,
    StatStr256_511,
    StatStr512_1023,
    StatStr1024_1518,
    StatStr1519Plus,
#if defined(CONFIG_BCM96858) || defined (CONFIG_BCM96856) || defined(CONFIG_BCM963158)
    StatStr2048_4095,
    StatStr4096_9216,
    StatStr9216Plus, 
#endif
    StatStr64b66bDecErr,
    StatStrAlignErr,
    StatStrBcastFrmRx,
    StatStrBcastFrmTx,
    StatStrByteDlyd,
    StatStrByteDrop,
    StatStrByteRx,
    StatStrByteTx,
    StatStrCodeErr,
    StatStrCol1,
    StatStrCol2,
    StatStrCol3,
    StatStrCol4,
    StatStrCol5,
    StatStrCol6,
    StatStrCol7,
    StatStrCol8,
    StatStrCol9,
    StatStrCol10,
    StatStrCol11,
    StatStrCol12_15,
    StatStrColExc,
    StatStrColLate,
    StatStrColTot,
    StatStrCrc32Err,
    StatStrCrc8Err,
    StatStrErrByte,
    StatStrFcsErr,
    StatStrFecBlock,
    StatStrFecByte,
    StatStrFecCorBlk,
    StatStrFecCorByte,
    StatStrFecCorOne,
    StatStrFecCorZero,
    StatStrFecDecErrCor,
    StatStrFecDecFail,
    StatStrFecDecPass,
    StatStrFecExceedErr,
    StatStrFecFrm,
    StatStrFecUnCorBlk,
    StatStrFrm,
    StatStrFrmRx,
    StatStrFrmTx,
    StatStrFrmBadSh,
    StatStrFrmDrop,
    StatStrGateRx,
    StatStrGenCrc32,
    StatStrLineCodeErr,
    StatStrMaxLenErr,
    StatStrMaxPktDly,
    StatStrMcastFrmRx,
    StatStrMcastFrmTx,
    StatStrMpcpByte,
    StatStrMpcpFrm,
    StatStrNonFecBytes,
    StatStrNonFecFrm,
    StatStrOamBytesRx,
    StatStrOamBytesTx,
    StatStrOamFrm,
    StatStrOamFrmRx,
    StatStrOamFrmTx,
    StatStrOversized,
    StatStrPauseRx,
    StatStrPauseTx,
    StatStrRegFrmRx,
    StatStrReports,
    StatStrSecGoodByte,
    StatStrSecGoodFrm,
    StatStrSecRxBadFrm,
    StatStrTestPrbsErr,
    StatStrTestPseudoErr,
    StatStrUcastFrmRx,
    StatStrUcastFrmTx,
    StatStrUndersized,
    StatStrUnusedWordsTx,
    StatStrBufUnd,
    StatStrRxAbrt,
    StatStrNumStrings
    } StatStrId_e;

typedef U8 StatStrId;

// EPON Link statistics ID
typedef enum
    {
    // Bidirectional stats
    EponBiDnTotalBytesRx,
    EponBiDnFcsErrors,
    EponBiDnOamFramesRx,
    EponBiDnGateFramesRx,
    EponBiDn64ByteFramesRx,
    EponBiDn65to127ByteFramesRx,
    EponBiDn128to255ByteFramesRx,
    EponBiDn256to511ByteFramesRx,
    EponBiDn512to1023ByteFramesRx,
    EponBiDn1024to1518ByteFramesRx,
    EponBiDn1518PlusByteFramesRx,
#if defined(CONFIG_BCM96858) || defined (CONFIG_BCM96856) || defined(CONFIG_BCM963158)
    EponBiDn2048to4095ByteFramesRx,
    EponBiDn4096to9216ByteFramesRx,
    EponBiDnGrant9216ByteFramesRx,
#endif
    EponBiDnOversizedFramesRx,
    EponBiDnBroadcastFramesRx,
    EponBiDnMulticastFramesRx,
    EponBiDnUnicastFramesRx,
    EponBiDnUndersizedFramesRx,
    EponBiDnOamBytesRx,
    EponBiDnRegisterFramesRx,

    // Downstream only stats
    EponDnTotalBytesRx,
    EponDnFCSErrors,
    EponDnOamFramesRx,
    EponDnGateFramesRx,
    EponDnBroadcastFramesRx,
    EponDnMulticastFramesRx,
    EponDnUnicastFramesRx,
    EponDn64to511ByteFramesRx,
    EponDn512to1023ByteFramesRx,
    EponDn1024to1518ByteFramesRx,
    EponDn1518PlusByteFramesRx,
    EponDnOversizedFramesRx,
    EponDnUndersizedFramesRx,
    EponDnOamBytesRx,

    // Upstream only stats
    EponBiUpTotalBytesTx,
    EponBiUpReserved,
    EponBiUpOamFramesTx,
    EponBiUpReportFramesTx,
    EponBiUp64ByteFramesTx,
    EponBiUp65To127ByteFramesTx,
    EponBiUp128To255ByteFramesTx,
    EponBiUp256To511ByteFramesTx,
    EponBiUp512To1023ByteFamesTx,
    EponBiUp1024To1518ByteFramesTx,
    EponBiUp1518PlusByteFramesTx,
#if defined(CONFIG_BCM96858) || defined (CONFIG_BCM96856) || defined(CONFIG_BCM963158)
    EponBiUp2048to4095ByteFramesTx,
    EponBiUp4096to9216ByteFramesTx,
    EponBiUpGrant9216ByteFramesTx,
#endif
    EponBiUpOamBytesTx,
    EponBiUpBroadcastFramesTx,
    EponBiUpMulticastFramesTx,
    EponBiUpUnicastFramesTx,
 
    //special       
    EponLinkMPCPRegReq,
    EponLinkMPCPRegAck,

    //for l2cp
    EponBiDnL2cpFramesRx,
    EponBiDnL2cpOctetsRx,
    EponBiDnL2cpFramesDiscarded,
    EponBiDnL2cpOctetsDiscarded,
    EponBiDnL2ErrorsRx,

    EponBiUpL2cpFramesTx,
    EponBiUpL2cpOctetsTx,
    EponBiUpL2ErrorsTx,
    
    EponLinkHwStatCount,
    EponLinkStatNotSupport = 0xFF
    } EponLinkStatId;

#define FirstLinkStat       EponBiDnTotalBytesRx
#define LastLinkStat        EponLinkHwStatCount

#endif