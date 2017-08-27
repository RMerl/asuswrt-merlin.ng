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
/** \file apfw_upgrade.h
 *
 * \brief APFW_UPGRADE primitive
 *
 * These primitives are used by the HLE to perform  FW persistent data transfers
 * (configuration files, running firmware image) to the STA.
 *
 **************************************************/

#ifndef APFW_UPGRADE_H_
#define APFW_UPGRADE_H_

/***************************************************
 *                 Include section
 ***************************************************/
#include "../base_types.h"
#include "definitions.h"


/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/** \brief Types of operation code */
typedef enum
{
   START_UPGRADE = 0x02,
   ACK_PACKET = 0x03,
   DATA_PACKET = 0x04,
   ERROR_PACKET = 0x05
} tE_UpgradeOpCode;

/** \brief APFW_UPGRADE.REQ encapsulates messages flowing from HLE to the STA */
typedef struct
{
   TU8               version;    //!< Upgrade protocol version
   TU16              length;     //!< Number of bytes in Data
   tE_UpgradeOpCode  Opcode;     //!< Packet opcode
   TU16              Block;      //!< block number (start with 1, only present in data pkts)
   TU8               Data[512];  /*!< file name if opcode=0x02, transferred data
                                      if opcode=0x04 */
} tS_APFW_UPGRADE_REQ;


/** \brief APFW_UPGRADE.CNF encapsulates the reverse path */
typedef struct
{
   TU8               version; //!< Upgrade protocol version
   tE_UpgradeOpCode  Opcode;  //!< Packet opcode
   TU16              Block;   //!< block number in data ACK pkts
   TChar*            Data;    //!< error msg string in error pkts (not present in ACK's)
} tS_APFW_UPGRADE_CNF;


/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult   result;     //!< Transaction result
    tS_APL2C_ERROR_CNF     err;        //!< APL2C_ERROR_CNF
    tS_APFW_UPGRADE_CNF    cnf;        //!< APFW_UPGRADE.CNF
} tS_APFW_UPGRADE_Result;

/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief            Execute APFW_UPGRADE
 *
 * \param req        (in)  APFW_UPGRADE.REQ
 * \param p_result   (out) Transaction result
 */
void Exec_APFW_UPGRADE(
   const tS_APFW_UPGRADE_REQ req,
   tS_APFW_UPGRADE_Result* p_result);

/**
 * \brief         Deallocate resources from a APFW_UPGRADE.CNF
 *
 * \param p_cnf   (in)  APFW_UPGRADE.CNF
*/
void Free_APFW_UPGRADE_CNF(tS_APFW_UPGRADE_CNF* p_cnf);


#endif // APFW_UPGRADE_H_
