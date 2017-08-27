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
/** \file definitions.h
 *
 * \brief General defs for all the transactions
 *
 **************************************************/

#ifndef DEFS_H_
#define DEFS_H_

/***************************************************
*                 Include section
***************************************************/
#include "../base_types.h"

/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/** \brief Connection ID. */
typedef TU16 t_CID;

/** \brief Connection Info id */
typedef TU32 t_CIID;

/** \brief Link ID */
typedef TU8 t_LID;

/** \brief Terminal Equipment id */
typedef TU8 t_TEI;

/** \brief MAC address */
typedef TU8 t_MACaddr[6];

/** \brief IPv6 address */
typedef TU8 t_IPv6addr[16];

/** \brief IPv4 address */
typedef TU8 t_IPv4addr[4];

/** \brief AES key */
typedef TU8 t_Key[16];

/** \brief Network Id */
typedef TU8 t_NetId[7];

/** \brief Band identifier */
typedef enum
{
    LOW_BAND = 0, //!< low band
    HIGH_BAND = 1, //!< high band
    BOTH = 2
} tE_BandId;

/** \brief Authorization information */
typedef enum
{
   SIMPLE_CONNECT = 0,
   SECURE
} tE_SecurityLevel;

/** \brief Transaction result */
typedef enum
{
   LINK_DOWN = 0,    //!< no response from the STA (Link is down)
   CNF_ARRIVED = 1,  //!< REQ ACK by its response (*.CNF) counterpart
   ERR_ARRIVED = 2,   //!< REQ ACK by and error (APL2CME _ERROR.CNF)
   UNDEF_ERROR = 3  /*! undefined error due to a fatal condition (this errors
                        raises an assertion on the earlier versions) */
} tE_TransactionResult;

/** \brief L2 primitive's operation result */
typedef enum
{
    SUCCESS = 0,
    FAILURE = 1
} tE_OpResult;


/** \brief Parameters size in bits */
typedef enum
{
   SIZE_8 = 1,
   SIZE_16,
   SIZE_24,
   SIZE_32
} tE_ParamSize;


/** \brief This Message is for indications flowing from the STA to the HLE */
typedef struct
{
    TU8 reason_code; //!< Reason
    TU8 RX_L2CMW;    //!< L2 Config Message Version Received
    TU16 RX_MTYPE;   //!< L2 Config MTYPE Received
} tS_APL2C_ERROR_CNF;

/**
 * \brief This is the struct to hold the transaction response when *.CNF
 *        primitive has no additional data fields
 *
 * \note if an ACK is received from the STA the field result is "CNF_ARRIVED"
*/
typedef struct
{
    tE_TransactionResult  result;   //!< Transaction result
    tS_APL2C_ERROR_CNF    err;      //!< APL2C_ERROR_CNF
} tS_NoCNFData_Result;

#endif
