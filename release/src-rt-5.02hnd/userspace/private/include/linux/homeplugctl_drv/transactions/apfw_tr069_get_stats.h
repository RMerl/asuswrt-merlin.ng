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

/** @ingroup N3
 * \file apfw_tr06_get_stats.h
 * \brief APFW_TR069_GET_STATS primitive
 *
 * This message is a request from the HLE to obtain statistics from STA as
 * defined by HomePlug data model in TR-181 specification [14]; it does not
 * require any additional parameter.
 */

#ifndef APFW_TR069_GET_STATS_
#define APFW_TR069_GET_STATS_

#include "../base_types.h"
#include "definitions.h"

/** @ingroup N3
   \brief APFW_TR069_GET_STATS.CNF
*/
typedef struct
{
  T64 BytesSent; //!< number of bytes transmitted out of the interface
  T64 BytesReceived; //!<	number of bytes received on the interface
  T64 PacketsSent; //!< number of packets transmitted out of the interface
  T64 PacketsReceived; //!< number of packets received on the interface
  TU32 ErrorsSent; //!< number of packets that could not be transmitted because of errors
  TU32 ErrorsReceived; //!< number of packets containing errors preventing them from being delivered to HLE protocols
  T64 UnicastPacketsSent; //!< number of unicast packets requested for transmission, including those that were discarded or not sent
  T64 UnicastPacketsReceived; //!< number of unicast packets received
  TU32 DiscardPacketsSent; //!< number of packets which were chosen to be discarded
  TU32 DiscardPacketsReceived; //!< number of received packets which were chosen to be discarded
  T64 MulticastPacketsSent; //!< number of multicast packets that HLE protocols requested for transmission, including those that were discarded or not sent
  T64 MulticastPacketsReceived; //!< number of multicast packets received
  T64 BroadcastPacketsSent; //!< number of broadcast packets that HLE protocols requested for transmission, including those that were discarded or not sent
  T64 BroadcastPacketsReceived; //!< number of broadcast packets received
  TU32 UnknownProtoPacketsReceived; //!< number of received packets discarded because of an unknown or unsupported protocol
  T64 MPDUTxAck; //!< number of MPDUs transmitted and acknowledged
  T64 MPDUTxCol; //!< number of MPDUs transmitted and collided
  T64 MPDUTxFailed; //!< number of MPDUs transmitted and failed
  T64 MPDURxAck; //!< number of MPDUs received and acknowledged
  T64 MPDURxFailed; //!< number of MPDUs received and failed
} tS_APFW_TR069_GET_STATS_CNF;

/** @ingroup N3
   \brief This is the struct to hold the APFW_TR069_GET_STATS transaction
          response.
*/
typedef struct
{
  tE_TransactionResult        result;   //!< Transaction result
  tS_APL2C_ERROR_CNF          err;      //!< APL2C_ERROR_CNF
  tS_APFW_TR069_GET_STATS_CNF cnf;      //!< APFW_TR069_GET_STATS.CNF
} tS_APFW_TR069_GET_STATS_Result;

/** @ingroup N3
   \brief Execute APFW_TR069_GET_STATS

   \param result  Transaction result.
*/
void Exec_APFW_TR069_GET_STATS(tS_APFW_TR069_GET_STATS_Result *result);

#endif /*APFW_TR069_GET_STATS_*/


