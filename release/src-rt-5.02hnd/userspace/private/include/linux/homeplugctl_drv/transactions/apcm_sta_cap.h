/***********************************************************************
 * <:copyright-BRCM:2008-2013:proprietary:standard
 * 
 *    Copyright (c) 2008-2013 Broadcom 
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
 * :> *
 * $Change: 121810 $
 ***********************************************************************/
/** \file apcm_sta_cap.h
 *
 * \brief APCM_STA_CAP primitive
 *
 **************************************************/
#ifndef APCM_STA_CAP_
#define APCM_STA_CAP_

/***************************************************
 *                 Include section
 ***************************************************/
#include "../base_types.h"
#include "definitions.h"


/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/** \brief CCo Capabilities */
typedef enum
{
   LEVEL_0 = 0,//!< does not support QoS and TDMA
   LEVEL_1,    //!< support QoS and TDMA but only in Uncoordinated Mode
   LEVEL_2,    //!< support QoS and TDMA but only in Coordinated Mode
   LEVEL_3     //!< future CCo capabilities
} tE_CCoCap;

/** \brief STA general profile */
typedef struct
{
   TBool AutoConnect;         //!< Auto Connect Capability
   TBool Smoothing;           //!< Smoothing Capability
   tE_CCoCap CCoCapability;   //!< CCo Capability
   TBool ProxyCapable;        //!< Proxy Capability
   TBool BackupCCo;           //!< Backup CCo-capable
   TBool SoftHandOver;        //!< Soft Hand Over Support
   TBool TwoSymFC;            //!< Two Symbol Frame Control
   TU16 MaxFL_AV;             /*!< Maximum value of FL_AV that the STA is
                                   capable supporting in multiples of 1.28 microsec */
   TU8 RegulatoryCap;         /*!< Capability of operating in various regulatory domains
                                 0: north america only
                                 1-255: reserved */
   TU8 BidirectionalBursting; /*!< 0: not capable
                                 1: only supports CFP bidirectional burst
                                    ending with SACK
                                 2: supports CFP bidirectional burst that
                                    either end with a SACK or a reverse SOF
                                 3-255: reserved */
   TU16  ImplVer;             //!< Implementation Version.
} tS_STAProfile;


/** \brief STA low band profile */
typedef struct
{
   tS_STAProfile general_profile;   /*!< STA general profile */
   TBool Homeplug_1_1_Cap;          /*!< Ability to support Enhanced Coexistence
                                          with Homeplug 1.1 */
   TBool Homeplug_1_0_Interop;      //!< Homeplug 1.0.1 interoperability
} tS_STALowBandProfile;


/** \brief APCM_STA_CAP.CNF */
typedef struct
{
   tE_BandId            BandCapability;      //!< Band capability
   TU8                  HPAVVersion;         //!< HomePlug AV version
   TU8                  MXVersion;           //!< MediaXtream version
   t_MACaddr            MACAddr;             //!< MAC address
   TU32                 OUI;                 //!< Organizationally Unique Id
   tS_STALowBandProfile low_band_profile;    //!< Low Band Station Profile
   tS_STAProfile        high_band_profile;   //!< High Band Station Profile
} tS_APCM_STA_CAP_CNF;



/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult   result;   //!< Transaction result
    tS_APL2C_ERROR_CNF     err;      //!< APL2C_ERROR_CNF
    tS_APCM_STA_CAP_CNF    cnf;      //!< APCM_STA_CAP.CNF
} tS_APCM_STA_CAP_Result;


/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief Execute APCM_STA_CAP
 *
 *  The APCM_STA_CAP.REQ primitive is a request from HLE to provide the station
 *  capabilities.
 *
 * \param p_result   (out) Transaction result
*/
void Exec_APCM_STA_CAP(tS_APCM_STA_CAP_Result* p_result);


#endif /*APCM_STA_CAP_*/
