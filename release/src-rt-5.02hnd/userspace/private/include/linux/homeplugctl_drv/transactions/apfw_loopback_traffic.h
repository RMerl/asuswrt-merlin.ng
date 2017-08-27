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
 * $Change: 85905 $
 ***********************************************************************/

/** @ingroup N3
 * \file apfw_loopback_traffic.h
 * \brief APFW_LOOPBACK_TRAFIC primitive
 */

#ifndef APFW_LOOPBACK_TRAFFIC_H_
#define APFW_LOOPBACK_TRAFFIC_H_

#include "../base_types.h"
#include "definitions.h"

/**
 * \brief APFW_LOOPBACK_TRAFFIC.REQ: This primitive implements a request to the STA to
 *        provide the Network stats
*/
typedef struct
{
   TU8 *mac_address; //!< Destination mac address
   TU16 duration; //!< Time duration (in ms)
   TU16 packet_length; //!< Packet length (in bytes). Max value 1518
   TU16 packet_period; //!< Packet period (in ms)
   TU8 is_bidir; //!< 0 indicates unidir, 1 indicates bidir
} tS_APFW_LOOPBACK_TRAFFIC_REQ;



/** @ingroup N3
   \brief This is the struct to hold the transaction response.
*/
typedef struct
{
    tE_TransactionResult  result;   //!< Transaction result
    tS_APL2C_ERROR_CNF    err;      //!< APL2C_ERROR_CNF
    unsigned char cnf;              //!< APFW_LOOPBACK_TRAFFIC.CNF
} tS_APFW_LOOPBACK_TRAFFIC_Result;

/** @ingroup N3
   \brief Execute APFW_LOOPBACK_TRAFFIC.
   \param mac_address Destination mac address of the traffic 
   \param duration Time of traffic generation (in ms)
   \param packet_length Ethernet packet length (in bytes)
   \param inter_packet_gap Inter packet gap time (in ms)
   \param is_bidir Generate unidir or bidir traffic (0 unidir, 1 bidir)
   \param result  Transaction result.
*/
void Exec_APFW_LOOPBACK_TRAFFIC( tS_APFW_LOOPBACK_TRAFFIC_REQ req, tS_APFW_LOOPBACK_TRAFFIC_Result *result);

#endif // APFW_LOOPBACK_TRAFFIC_H_
