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
 * \file apfw_tr06_get_tone_info.h
 * \brief APFW_TR069_GET_TONE_INFO primitive
 *
 * This primitive is used to retrieve tone information between the local node
 * and a peer one, passed as a parameter of the request.
 */

#ifndef APFW_TR069_GET_TONE_INFO_
#define APFW_TR069_GET_TONE_INFO_

#include "../base_types.h"
#include "definitions.h"

#define TONEMAPFILTER_N_BOUNDARIES 19
#define PLC_HPAV_N_CARRIERS        1155
#define PLC_HPAV_FIRST_CARRIER     74

#define TEI_INFO_GRP_IDX_MASK     0x3
#define TEI_INFO_HPAV20_FLAG_MASK 0x4
#define TEI_INFO_MIMO_FLAG_MASK   0x8
#define TEI_INFO_NUM_OF_BOUNDARY_CARRIERS_ON_HEADER (1 << 5)

/* Added 8 extra elements to account for a possible excess filling caused by
 * uneven grouping.
 */
#define SNR_PER_TONE_MAX_NUM_ELEMS (6908 + 8)

typedef struct
{
  TU8  tei_info;
  TU16 n_tones;
  TU16 AvgAttenuation; //!< Average attenuation from this remote device (expressed in 0.1dB).
  TU8  SNRPerTone[SNR_PER_TONE_MAX_NUM_ELEMS]; //!< Signal to Noise Ratio (SNR) from remote device (expressed in 0.25dB).
} tS_APFW_TR069_GET_TONE_INFO_CNF;

/** @ingroup N3
   \brief This is the struct to hold the transaction response.
*/
typedef struct
{
  tE_TransactionResult            result;   //!< Transaction result
  tS_APL2C_ERROR_CNF              err;      //!< APL2C_ERROR_CNF
  tS_APFW_TR069_GET_TONE_INFO_CNF cnf;      //!< APFW_TR069_GET_TONE_INFO.CNF
} tS_APFW_TR069_GET_TONE_INFO_Result;

/** @ingroup N3
   \brief Execute APFW_TR069_GET_TONE_INFO

   \param remote_device  Remote node's MAC Address
   \param result         Transaction result.
*/
void Exec_APFW_TR069_GET_TONE_INFO(t_MACaddr remote_device,
                                   tS_APFW_TR069_GET_TONE_INFO_Result *result);

#endif /*APFW_TR069_GET_TONE_INFO_*/




