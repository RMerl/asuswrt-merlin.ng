/***********************************************************************
 * <:copyright-BRCM:2007-2013:proprietary:standard
 * 
 *    Copyright (c) 2007-2013 Broadcom 
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

/** @ingroup N3
 * \file apcm_uke_cmd.h
 * \brief APCM_UKE_CMD primitive
 */

#ifndef APCM_UKE_CMD_H_
#define APCM_UKE_CMD_H_

#include "../base_types.h"
#include "definitions.h"

/** @ingroup N3
   \brief UKE command's action
*/
typedef enum
{
   UKE_START = 0,
   UKE_RANDOMIZE_KEY,
   UKE_GET_STATUS,
} tE_UKECmdAction;

/** @ingroup N3
   \brief result when UKE_GET_STATUS has been asked
*/
typedef enum
{
  L2CONFIG_CMD_STATUS_SUCCESS = 0,
  L2CONFIG_CMD_STATUS_FAILED,
  L2CONFIG_CMD_STATUS_TIMEOUT,
  L2CONFIG_CMD_STATUS_ONGOING,
  L2CONFIG_CMD_STATUS_NA,
}t_l2config_uke_cmd_status;

/** @ingroup N3
   \brief This is the struct to hold the transaction response.
*/
typedef struct
{
    tE_TransactionResult  result;   //!< Transaction result
    tS_APL2C_ERROR_CNF    err;      //!< APL2C_ERROR_CNF
    unsigned char cnf;              //!< APCM_UKE_CMD.CNF
} tS_APCM_UKE_CMD_Result;

/** @ingroup N3
   \brief Execute APCM_UKE_CMD.

   \param req  REQ primitive.
   \param result  Transaction result.
*/
void Exec_APCM_UKE_CMD(
   const tE_UKECmdAction action,
   tS_APCM_UKE_CMD_Result *result);

#endif // APCM_UKE_CMD_H_
