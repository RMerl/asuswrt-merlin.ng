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

/*                        Copyright(c) 2010 Broadcom                          */

#if !defined(Lif_h)
#define Lif_h
////////////////////////////////////////////////////////////////////////////////
/// \file Lif.h
/// \brief Routines to manipulate Lif module
///
////////////////////////////////////////////////////////////////////////////////

#include "Teknovus.h"
#ifdef EPON_NORMAL_MODE
#include "PonConfigDb.h"
#endif
#include "Laser.h"
#include <rdpa_api.h>
#include "rdpa_epon.h"
#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
    {
    LifStatFirst = 0,
    // Tx Stats
    LifTxFrames = LifStatFirst,
    LifTxBytes,
    LifTxNonFecFrames,
    LifTxNonFecBytes,
    LifTxFecFrames,
    LifTxFecBytes,
    LifTxFecBlocks,
    LifTxReportFrames,

    // Rx Stats
    LifFirstRxStat,
    LifRxLineCodeErrors = LifFirstRxStat,
    LifRxGateFrames,
    LifRxGoodFrames,
    LifRxGoodBytes,
    LifRxUndersizedFrames,
    LifRxOversizeFrames,
    LifRxCrc8ErrorsFrames,
    LifRxFecFrames,
    LifRxFecBytes,
    LifRxFecExceedErrorsFrames,
    LifRxNonFecFrames,
    LifRxNonFecBytes,
    LifRxFecErrorBytes,
    LifRxFecErrorZeroes,
    LifRxFecNoErrorBlocks,
    LifRxFecCorrBlocks,
    LifRxFecUnCorrBlocks,
    LifRxFecCorrOnes,
    LifRxErroredFrames,

    LifStatCount
    } LifStatId_e;

typedef U8 LifStatId;
////////////////////////////////////////////////////////////////////////////////
/// LifReadStat:  Reads LIF statistic
///
 // Parameters:
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
extern
U32 LifReadStat (LifStatId stat);


////////////////////////////////////////////////////////////////////////////////
/// LifKeySet:  Set/Clear encryption for a given link/key
///
 // Parameters:
/// \param mode security decrytion mode
/// \param direction direction of interest
/// \param link link of interest
/// \param keyIdx index of the key
/// \param key pointer to the key to set or NULL if encryption is disabled
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifKeySet (EncryptMode mode,
                Direction direction,
                LinkIndex link,
                U8 keyIdx,
                U32 const BULK *key,
                const U32 BULK* sci,
                U8 tci,
                U32 pn);


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the current security enable status for link
///
/// This function gets the security enable status for link
///
/// \param  None
///
/// \return
/// the current security encrypt enable status
////////////////////////////////////////////////////////////////////////////////
extern
BOOL LifEncryptEnGet(LinkIndex link, Direction dir);


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the current security key for a link
///
/// This function gets the active key for a given link.  It is used to detect a
/// key switch over.
///
/// \param  link Link index to check
///
/// \return
/// Active security key index
////////////////////////////////////////////////////////////////////////////////
extern
U8 LifKeyInUse (LinkIndex link, Direction dir);


////////////////////////////////////////////////////////////////////////////////
/// LifEncryptUpDisable - Disable upstream encryption global configuration
///
 // Parameters:
/// \none
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
extern
void LifEncryptUpDisable (void);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Set FEC Tx for links
///
/// \param  linkMap     Bitmap of links to set FEC tx
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void LifFecTxSet(LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set FEC Rx
///
/// \param mode rx fec mode to set
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifFecRxSet (BOOL mode);



////////////////////////////////////////////////////////////////////////////////
/// \brief Sets idle time for LIF
///
/// \param  front   Time to idle before burst,
///                 measured in 62.5M clocks (16 bit times)
/// \param  back    Time to idle after burst, 62.5M clocks
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifSetIdleTime (U16 front, U16 back);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Remove a link form the lookup table
///
/// Logical links are numbered from 0;
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifDisableLink (LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// \brief Enable lookup for incoming traffic
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
/// LLID value within it that is not necessarily the same.
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifEnableLink (LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// \brief delete physical LLID value associated with a link
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifDeleteLink (LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// \brief sets physical LLID value associated with a link
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
/// LLID value within it that is not necessarily the same.
///
/// \param link     Logical link number
/// \param phy      Physical LLID value for this link
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifCreateLink (LinkIndex link, PhyLlid phy);


////////////////////////////////////////////////////////////////////////////////
/// \brief Return a PhyLlid associated to a link index
///
/// \param link     Logical link number
///
/// \return Physical Llid
/// None
////////////////////////////////////////////////////////////////////////////////
extern
PhyLlid LifGetPhyLlid (LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// LifPhyLlidToIndex:  Return a link index associated to a phy llid
///
 // Parameters:
/// \param link     Phy LLID
///
/// \return Link index
/// None
////////////////////////////////////////////////////////////////////////////////
extern
LinkIndex LifPhyLlidToIndex (PhyLlid phy);


/* Set Laser Tx mode. Never fails */
void LifLaserTxModeSet (rdpa_epon_laser_tx_mode mode);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable Lif Rx module
///
/// Only Disable the Lif Rx module once it has properly been configured
///
/// \param None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifDisableRx (void);


////////////////////////////////////////////////////////////////////////////////
/// LifDisableTx:  Disable Lif Tx module
///
/// Only Disable the Lif Tx module once it has properly been configured
///
/// \param None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifDisableTx (void);


////////////////////////////////////////////////////////////////////////////////
/// LifEnableTx:  Enable Lif module Tx
///
/// Only enable the Lif module Tx once it has properly been configured
///
/// \param None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifEnableTx (void);


////////////////////////////////////////////////////////////////////////////////
/// LifEnableRx:  Enable Lif module Rx
///
/// Only enable the Lif module Rx once it has properly been configured
///
/// \param None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifEnableRx (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Does the LIF have code word lock?
///
/// \return
/// TRUE if LIF has code word lock, FALSE otherwise
////////////////////////////////////////////////////////////////////////////////
extern
BOOL LifLocked(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable LIF RX
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifRxDisable(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Enable LIF RX
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifRxEnable(void);




////////////////////////////////////////////////////////////////////////////////
/// \brief  Setup TX-to-RX loopback
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void LifTxToRxLoopback (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set the MPCP time at which the 1PPS signal should be asserted
///
/// \param  time    MPCP time to asserted 1PPS signal
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void LifTransportTimeSet(U32 time);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get the current MPCP time
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
U32 LifMpcpTimeGet(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set Lif Laser Monitor Configuration
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void LifLaserMonSet(BOOL enable, U32 threshold);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get Lif Laser Monitor LaserOnMaxInt interrrupt
///
/// \return interrupt bit value
////////////////////////////////////////////////////////////////////////////////
extern
U32 LifLaserMonLaserOnMaxIntGet(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set Lif Laser Monitor Configuration
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void LifLaserMonClrLaserOnMaxInt(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Lif lbe polarity Configuration
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void LifLbePolaritySet(Polarity polarity);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Initialize module
///
/// \param  txOffTimeOffset     Delay normal laser Off (in TQ)
/// \param  txOffIdle           If TRUE we insert idle when laser is OFF
/// \param  upRate              Upstream laser rate
/// \param  dnRate              Downstream laser rate
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void LifInit (U8 txOffTimeOffset, BOOL txOffIdle,
              Polarity polarity, LaserRate upRate, LaserRate dnRate);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Set pon speed
///
/// \param  dnRate              Downstream laser rate
///
/// \return None
//////////////////////////////////////////////////////////////////////////
extern 
void LifSetPonSpeed(LaserRate dnRate);

#if defined(__cplusplus)
}
#endif

#endif // Lif.h
