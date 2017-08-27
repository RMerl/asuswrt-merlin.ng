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
/** \file apfw_set_param.h
 *
 * \brief APFW_SET_PARAM primitive
 *
 * This primitive is used to write an specified ParamConfig persistent parameter
 * on the STA NVM.
 * Regarding the parameters themselves, we will support 8-bit. 16-bit and 32-bit
 * parameters, and they can be scalar or an array of N elements.  Parameters
 * will be refered  by their numerical id in the paramconfig table.
 **************************************************/

#ifndef APFW_SET_PARAM_H_
#define APFW_SET_PARAM_H_

/***************************************************
 *                 Include section
 ***************************************************/
#include "../base_types.h"
#include "definitions.h"

/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/** \brief APFW_SET_PARAM.REQ common data */
typedef struct
{
   TU16  parameter_id; //!< parameter identifier (index in the parameter table)
   tE_ParamSize   Size; //!< Size of one element
   TU16  NumElements;   //!< Length of the Array (1 in case of scalar values)
} tS_APFW_SET_COMMON_PARAM_REQ;


/** \brief APFW_SET_PARAM.REQ for 8-bit parameter */
typedef struct
{
   tS_APFW_SET_COMMON_PARAM_REQ  base_req;   //!< common data
   TU8*                          val;        //!< 8-bit value
} tS_APFW_SET_U8_PARAM_REQ;

/** \brief APFW_SET_PARAM.REQ for 16-bit parameter */
typedef struct
{
   tS_APFW_SET_COMMON_PARAM_REQ  base_req;   //!< common data
   TU16*                         val;        //!< 16-bit value
} tS_APFW_SET_U16_PARAM_REQ;

/** \brief APFW_SET_PARAM.REQ for 24-bit parameter */
typedef struct
{
   tS_APFW_SET_COMMON_PARAM_REQ  base_req;   //!< common data
   TU32*                         val;        //!< 24-bit value
} tS_APFW_SET_U24_PARAM_REQ;

/** \brief APFW_SET_PARAM.REQ for 32-bit parameter */
typedef struct
{
   tS_APFW_SET_COMMON_PARAM_REQ  base_req;   //!< common data
   TU32*                         val;        //!< 32-bit value
} tS_APFW_SET_U32_PARAM_REQ;


/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult   result;     //!< Transaction result
    tS_APL2C_ERROR_CNF     err;        //!< APL2C_ERROR_CNF
    tE_OpResult            cnf;        //!< APFW_SET_PARAM.CNF
} tS_APFW_SET_PARAM_Result;


/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief            Execute APFW_SET_PARAM for 8-bit parameter
 *
 * \param req        (in)  REQ primitive
 * \param p_result   (out) Transaction result
*/
void Exec_APFW_SET_U8_PARAM(
   const tS_APFW_SET_U8_PARAM_REQ req,
   tS_APFW_SET_PARAM_Result* p_result);

/**
 * \brief            Execute APFW_SET_PARAM for 16-bit parameter
 *
 * \param req        (in)  REQ primitive
 * \param p_result   (out) Transaction result
*/
void Exec_APFW_SET_U16_PARAM(
   const tS_APFW_SET_U16_PARAM_REQ req,
   tS_APFW_SET_PARAM_Result* p_result);

/**
 * \brief            Execute APFW_SET_PARAM for 24-bit parameter
 *
 * \param req        (in)  REQ primitive
 * \param p_result   (out) Transaction result
*/
void Exec_APFW_SET_U24_PARAM(
   const tS_APFW_SET_U24_PARAM_REQ req,
   tS_APFW_SET_PARAM_Result* p_result);

/**
 * \brief            Execute APFW_SET_PARAM for 32-bit parameter
 *
 * \param req        (in)  REQ primitive
 * \param p_result   (out) Transaction result
*/
void Exec_APFW_SET_U32_PARAM(
   const tS_APFW_SET_U32_PARAM_REQ req,
   tS_APFW_SET_PARAM_Result* p_result);


#endif // APFW_SET_PARAM_H_
