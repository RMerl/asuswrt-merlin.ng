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


#if !defined(PonMgrFec_h)
#define PonMgrFec_h
////////////////////////////////////////////////////////////////////////////////
/// \file PonMgrFec.h
/// \brief Forward Error Correction configuration module
/// \author Jason Armstrong
/// \date December 8, 2006
///
/// This modules wraps the low level of the ONUs Forward Error Correction.  It
/// provided two interfaces, an advanced independent RX/TX configuration, and
/// an IEEE compliant simple interface.  The IEEE interface implements
/// functionality to support the following attributes:
///
///   - aFECAbility
///   - aFECmode
///   - aFECCorrectedBlocks
///   - aFECUncorrectableBlocks
///
////////////////////////////////////////////////////////////////////////////////
#include "Teknovus.h"
#include "Lif.h"
#ifdef CONFIG_EPON_10G_SUPPORT
#include "Xif.h"
#endif
#include "EponMac.h"


typedef enum
    {
    FecApp1G    = 1U<<0,
    FecApp10G   = 1U<<1,
    FecAppBoth  = (FecApp1G | FecApp10G)
    } FecAppDirection_e;

typedef enum
    {
    FecRxLinkAllEn = 0,
    FecRxLinkBoth,
    FecRxLinkAllDis,
    FecRxLinkNum
    }FecRxLinkMode;

typedef U8 FecAppDirection;



////////////////////////////////////////////////////////////////////////////////
/// \brief  Get the current FEC Tx state for the EPON port
///
/// \return
/// TRUE if upstream FEC is enabled
////////////////////////////////////////////////////////////////////////////////
extern
BOOL FecTxState (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get the current FEC Tx state for the given link
///
/// \param  link    Index of link
///
/// \return TRUE if upstream FEC is enabled on the given link
////////////////////////////////////////////////////////////////////////////////
extern
BOOL FecTxLinkState(LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get the current FEC Rx state for the EPON port
///
/// \return
/// TRUE if downstream FEC is enabled
////////////////////////////////////////////////////////////////////////////////
extern
BOOL FecRxState (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get the current FEC Rx state for the given link
///
/// \param  link    Index of link
///
/// \return TRUE if downstream FEC is enabled on the given link
////////////////////////////////////////////////////////////////////////////////
extern
BOOL FecRxLinkState(LinkIndex link);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get the current FEC state for the EPON port
///
/// \return TRUE if FEC is enabled on the EPON port
////////////////////////////////////////////////////////////////////////////////
extern
BOOL FecState (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set the current FEC mode
///
/// \param  link    Index of link to set FEC mode
/// \param  rxState FEC Rx enable/disable
/// \param  txState FEC Tx enable/disable
/// \param  path    Which speeds to apply the settings to
///
/// \return True if set sucessful.
////////////////////////////////////////////////////////////////////////////////
extern
BOOL FecModeSet (LinkIndex link,
                 BOOL rxState,
                 BOOL txState,
                 FecAppDirection path);


////////////////////////////////////////////////////////////////////////////////
/// FecCorrectedBlocks - Number of blocks that FEC has fixed
///
/// This function returns the number of downstream blocks that the FEC has
/// repaired since the last time this count was read.  The count clears on read.
///
/// \return
/// Number of corrected blocks
////////////////////////////////////////////////////////////////////////////////
extern
U32 FecCorrectedBlocks (void);


////////////////////////////////////////////////////////////////////////////////
/// FecUncorrectableBlocks - Number of blocks that FEC has not fixed
///
/// This function returns the number of downstream blocks that the FEC could not
/// repair since the last time this count was read.  The count clears on read.
///
/// \return
/// Number of uncorrected blocks
////////////////////////////////////////////////////////////////////////////////
extern
U32 FecUncorrectableBlocks (void);


////////////////////////////////////////////////////////////////////////////////
/// FecInit - Initialize FEC
///
/// This function initializes FEC, restoring the previous configuration if it is
/// present or using the FEC configuration from personality if it is not.
///
/// \param up10G is the upstream 10G?
///
/// \return
/// Nothing
////////////////////////////////////////////////////////////////////////////////
extern
void FecInit (void);


#endif // Fec_h
