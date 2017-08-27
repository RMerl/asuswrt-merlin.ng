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

#ifndef APCM_AUTHORIZE_H_
#define APCM_AUTHORIZE_H_

/***************************************************
*                 Include section
***************************************************/
#include "../base_types.h"
#include "definitions.h"


/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/** \brief     The APCM_AUTHORIZE.REQ message is used to instruct a STA to
 *             authorize another STA to join its AVLN using DAK-based
 *             distribution of the NMK
 */
typedef struct
{
   t_Key DAK;           //!< DAK to be used for NMK Provisioning.
   t_MACaddr  ODA;      //!< MAC address of STA to authenticate, if known.
} tS_APCM_AUTHORIZE_BASE_REQ;

/** \brief     If the type of the authorization is uses current NID and NMK then
 *             a security level will be provided.
 */
typedef struct
{
   tS_APCM_AUTHORIZE_BASE_REQ Req;  //!< Request common info.
   tE_SecurityLevel SecLevel;       //!< Security Level.
} tS_APCM_AUTHORIZE_USING_CURRENT_AUTH_INFO_REQ;

/** \brief     If the type of the authorization is user provided NID and NMK
 *             then a NMK & NID will be provided.
 */
typedef struct
{
   tS_APCM_AUTHORIZE_BASE_REQ Req;  //!< Request common info
   t_Key  NMK;                      //!< Network Management Key to distribute
   t_NetId   NID;                   //!< Net Id to associate with the new NMK
} tS_APCM_AUTHORIZE_USER_PROVIDED_AUTH_INFO_REQ;

/** \brief If the type of the authorization is user provided NID and NMK then
           a NMK & NID will be provided.
*/
typedef struct
{
   tS_APCM_AUTHORIZE_BASE_REQ Req; //!< Request common info.
   t_Key  NMK; //!< Network Management Key to distribute.
   tE_SecurityLevel SecLevel; //!< Security Level.
} tS_APCM_AUTHORIZE_USER_PROVIDED_NMK_REQ;

/** \brief The APCM_AUTHORIZE.CNF message is used by STA to inform the HLE of
 *         the results of its request to authorize another STA to join its AVLN
 *         using DAK-based distribution of the NMK. This confirmation shall only
 *         be sent when the protocol terminates due to successful completion,
 *         timeout, or abort. Timeout may be due to TEK lifetime expiration, or
 *         it may be due to a persistent lack of response to the DAK-encrypted
 *         initial message.
 */
typedef enum
{
   AUTHORIZATION_COMPLETE = 0,
   NO_RESPONSE,
   PROTOCOL_ABORTED,
   AUTHORIZATION_STARTED,
   AUTHORIZATION_BUSY,
   AUTHORIZATION_FAILED
} tE_APCM_AUTHORIZE_CNF;

/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult  result;      //!< Transaction result
    tS_APL2C_ERROR_CNF    err;         //!< APL2C_ERROR_CNF
    tE_APCM_AUTHORIZE_CNF  cnf;        //!< tE_APCM_AUTHORIZE_CNF
} tS_APCM_AUTHORIZE_Result;

/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief            Execute APCM_AUTHORIZE using current auth info transaction
 *
 * \param req        (in)  REQ primitive
 * \param p_result   (out) Transaction result
 */
void Exec_APCM_AUTHORIZE_USING_CURRENT_AUTH_INFO(
   const tS_APCM_AUTHORIZE_USING_CURRENT_AUTH_INFO_REQ req,
   tS_APCM_AUTHORIZE_Result* p_result);

/**
 * \brief         Execute APCM_AUTHORIZE user provided auth info transaction
 *
 * \param req     (in) REQ primitive
 * \param result  (out) Transaction result
*/
void Exec_APCM_AUTHORIZE_USER_PROVIDED_AUTH_INFO(
   const tS_APCM_AUTHORIZE_USER_PROVIDED_AUTH_INFO_REQ req,
   tS_APCM_AUTHORIZE_Result* p_result);


/** 
 * \brief Execute APCM_AUTHORIZE user provide NMK default NID generated from
 *         NMK and SL
 *
 * \param req  REQ primitive.
 * \param result  Transaction result.
*/
void Exec_APCM_AUTHORIZE_USER_PROVIDED_NMK_REQ(
   const tS_APCM_AUTHORIZE_USER_PROVIDED_NMK_REQ req,
   tS_APCM_AUTHORIZE_Result* p_result);
   
/** 
 * \brief   Register a callback notification function
 *
 * In order to receive APCM_AUTHORIZE.IND from the STA, the client must to
 * register a callback function with this prototype:
 *
 *    void callback(TU8 *data,TU16 size)
 *
 * Then, when the library receives an APCM_AUTHORIZE.IND from the STA it will
 * notify to the clients registered before.
 *
 *
 * \param callback  function pointer
*/
void Suscribe_APCM_AUTHORIZE_IND(void (*callback)(TU8*,TU16));


/** 
 * \brief   Unregister a callback notification function
 * \param callback  function pointer
*/
void Unsubscribe_APCM_AUTHORIZE_IND(void (*callback)(TU8*,TU16));

#endif /*APCM_AUTHORIZE_H_*/
