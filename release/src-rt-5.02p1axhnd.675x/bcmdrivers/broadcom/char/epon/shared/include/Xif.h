/*
*  Copyright 2015, Broadcom Corporation
*
* <:copyright-BRCM:2015:proprietary:standard
* 
*    Copyright (c) 2015 Broadcom 
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

/*                        Copyright(c) 2015 Broadcom                          */

#if !defined(Xif_h)
#define Xif_h
////////////////////////////////////////////////////////////////////////////////
/// \file Xif.h
/// \brief Routines to manipulate Xif module
///
////////////////////////////////////////////////////////////////////////////////

#include "Teknovus.h"
#ifdef EPON_NORMAL_MODE
#include "PonConfigDb.h"
#endif
#include "Laser.h"
#include "drv_epon_xif_ag.h"
#include "drv_epon_xpcstx_ag.h"

typedef enum
    {
    XifStatFirst,
    XifFrameRx = XifStatFirst,
    XifBytesRx,
    XifRuntsRx,
    XifCodewordErrorRx,
    XifCrc8ErrorRx,

    XifXpnDataFrame,
    XifXpnDataByte,
    XifXpnMpcpFrame,
    XifXpnOamFrame,
    XifXpnOamByte,
    XifXpnOversizeFrame,
    XifXpnSecAbortFrame,
    XifStatCount
    } XifStatId ;

typedef enum
    {
    Xpcs32RxStatFirst,
    Xpcs32RxFecDecFailCnt = Xpcs32RxStatFirst,
    Xpcs32RxFecDecTotalCnt,
    Xpcs32Rx64b66bDecErrCnt,
    Xpcs32RxFramerBadShCnt,
    Xpcs32RxTestPseudoErrCnt,
    Xpcs32RxTestPrbsErrCnt,
    Xpcs32RxStatCount
    } Xpcs32RxStatId ;

typedef enum
    {
    Xpcs40RxStatFirst,
    Xpcs40RxFecDecErrCorCnt = Xpcs40RxStatFirst,
    Xpcs40RxStatCount
    } Xpcs40RxStatId ;


#define XifLlidEn              0x00010000UL    
#define XifLlidCount           0x00000020UL
#define XifXpcsRxCwLoss        0x00000040UL

// maximum number of 66b blocks before idle insert is forced
// must be large enough to accomodate 2000 byte frame plus overhead
#define XpcsRxMaxBlocks        0x11BU
#define XifIpgInsertionCfgWordMsk            0x0000007FUL // [6:0]


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set/Clear encryption for a given link/key
///
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
void XifKeySet (EncryptMode mode,
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
BOOL XifEncryptEnGet(LinkIndex link, Direction dir);


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
U8 XifKeyInUse (LinkIndex link, Direction dir);



////////////////////////////////////////////////////////////////////////////////
/// XifEncryptUpDisable - Disable upstream encryption global configuration
///
 // Parameters:
/// \none
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
extern
void XifEncryptUpDisable (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief Reads XIF statistic
///
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
extern
U32 XifReadStat (XifStatId stat);


////////////////////////////////////////////////////////////////////////////////
/// \brief Reads XPCS 32 bits width statistic
///
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
extern
U32 XifXpcsRead32Stat (Xpcs32RxStatId statId);


////////////////////////////////////////////////////////////////////////////////
/// \brief Reads XPCS402 bits width statistic
///
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
extern
U64 XifXpcsRead40Stat (Xpcs40RxStatId statId);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set 10G FEC Rx 
///
/// \param linkMap Bitmap of links to enable FEC rx
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void XifFecRxSet (LinkMap linkMap);
   

////////////////////////////////////////////////////////////////////////////////
/// \brief  Set 10G FEC Tx 
///
/// \param linkMap Bitmap of links to enable FEC tx
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void XifFecTxSet (LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
extern
void XifSetDiscIdleTime (U16 front, U16 back);


////////////////////////////////////////////////////////////////////////////////
//extern
void XifSetNonDiscIdleTime (U16 front, U16 back);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get all link state from the lookup table
///
/// Logical links are numbered from 0;
///
/// \param none
///
/// \return
/// link bit map
////////////////////////////////////////////////////////////////////////////////
extern
U32 XifAllLinkState (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable a link from the lookup table
///
/// Logical links are numbered from 0;
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void XifDisableLink (LinkIndex link);


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
void XifEnableLink (LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// \brief Delete physical LLID value associated with a link
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifDeleteLink (LinkIndex link);


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
void XifCreateLink (LinkIndex link, PhyLlid phy);


////////////////////////////////////////////////////////////////////////////////
/// \brief Return a PhyLlid associated to a link index
///
/// \param link     Logical link number
///
/// \return Physical Llid
////////////////////////////////////////////////////////////////////////////////
extern
PhyLlid XifGetPhyLlid (LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Return a link index associated to a phy llid
///
/// \param phy     Phy LLID
///
/// \return Link index
////////////////////////////////////////////////////////////////////////////////
extern
LinkIndex XifPhyLlidToIndex (PhyLlid phy);


////////////////////////////////////////////////////////////////////////////////
/// \brief Set transmit laser Mode
///
/// \param mode new tx laser mode
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void XifLaserTxModeSet (LaserTxMode mode);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Does the XIF have code word lock?
///
/// \return
/// TRUE if XIF has code word lock, FALSE otherwise
////////////////////////////////////////////////////////////////////////////////
extern
BOOL XifLocked(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable XPCS framer
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void XifRxDisable(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Enable XPCS framer
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void XifRxEnable(void);


////////////////////////////////////////////////////////////////////////////////
extern
void XifRxClk161Disable(void);


////////////////////////////////////////////////////////////////////////////////
extern
void XifRxClk161Enable(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Setup XIF IPG insertion
///
/// \param bytesToInsert      Number of bytes to insert in IPG or 0 to disable
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void XifIpgInsertionSet (U8 bytesToInsert);


////////////////////////////////////////////////////////////////////////////////
extern
void XifTransportTimeSet(U32 time);


////////////////////////////////////////////////////////////////////////////////
extern
U32 XifMpcpTimeGet(void);


////////////////////////////////////////////////////////////////////////////////
extern
void XifLaserMonSet(BOOL enable, U32 threshold);


////////////////////////////////////////////////////////////////////////////////
extern
U32 XifLaserMonLaserOnMaxIntGet(void);


////////////////////////////////////////////////////////////////////////////////
extern
void XifLaserMonClrLaserOnMaxInt(void);
    

////////////////////////////////////////////////////////////////////////////////
extern
void XifInrpStatusGet(xif_int_status *int_status);

////////////////////////////////////////////////////////////////////////////////
extern
void XifInrpMaskSet(const xif_int_mask *int_mask);

////////////////////////////////////////////////////////////////////////////////
extern
void XifXpcsInrpStatusGet(xpcstx_tx_int_stat *tx_int_stat);

////////////////////////////////////////////////////////////////////////////////
extern
void XifXpcsInrpMaskSet(const xpcstx_tx_int_mask *tx_int_mask);

////////////////////////////////////////////////////////////////////////////////
extern
void XifXpcsLbePolaritySet(Polarity polarity);

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
void XifInit (U8 txOffTimeOffset, BOOL txOffIdle,
              Polarity polarity, LaserRate upRate, LaserRate dnRate);

#if defined(__cplusplus)
}
#endif

#endif // Xif.h
