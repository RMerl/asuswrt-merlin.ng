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
/** \file apsta_info.h
 *
 * \brief APSTA_INFO primitive
 *
 **************************************************/
#ifndef APSTA_INFO_H_
#define APSTA_INFO_H_

/***************************************************
 *                 Include section
 ***************************************************/
#include "../base_types.h"
#include "definitions.h"

/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/** \brief UCode */
typedef struct st_Ucode tS_Ucode;
struct st_Ucode
{
   struct st_Ucode*  next;       //!< next Ucode
   TU8               modified;   //!< 0x00: Not modified, 0x01 Modified.
   TU32              version;    //!< ucode version
};

/** \brief APSTA_INFO.CNF */
typedef struct
{
   TU32  ChipVersion;   //!< 32 bit encoded chip version
   TU32  HWVersion;  //!< 32 bit encoded version HW Refdesign Version
   TU32  FWVersion;  //!< 32 bit encoded version Major.minor.build bitmap
   TU32  RomVersion; //!< 32 bit encoded rom version
   TU32  ParamConfigBuiltinVersion; //!< 32 bit encoded paramconfig builtin version
   TU32  ParamConfigNvmVersion;     //!< 32 bit encoded paramconfig nvm version
   TU8   Num_Ucodes; //!< Number of ucode version elements that will follow
   tS_Ucode *Ucodes; //!< Ucodes
   TU32  Uptime;  //!< Time in seconds since the STA was last restarted
   TU8   Fw_boot_msg_len;  //!< Length of the fw boot message
   TChar *Fw_boot_msg;  //!< Fw boot message.
   TU32  fw_version;
   TU32  fw_features;
   TU32  flash_model;
   TU8   HPAV_Version; //!< HomePlug version
   TU8   MaxBitRate; //!< The maximum PHY bit rate supported by this interface (expressed in Mbps).
} tS_APSTA_INFO_CNF;


/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult   result;   //!< Transaction result
    tS_APL2C_ERROR_CNF     err;      //!< APL2C_ERROR_CNF
    tS_APSTA_INFO_CNF      cnf;      //!< APSTA_INFO.CNF
} tS_APSTA_INFO_Result;

/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief            Execute APSTA_INFO transaction
 *
 * This message is a request from the HLE to obtain basic information on the
 * firmware/hardware/ref design running on the STA
 *
 * \param p_result   (out) Transaction result
*/
void Exec_APSTA_INFO(tS_APSTA_INFO_Result* p_result);

/**
 * \brief         Deallocate resources from a APSTA_INFO.CNF
 *
 * \param p_cnf   (in) APSTA_INFO.CNF
*/
void Free_APSTA_INFO_CNF(tS_APSTA_INFO_CNF* p_cnf);

#endif /*APSTA_INFO_H_*/
