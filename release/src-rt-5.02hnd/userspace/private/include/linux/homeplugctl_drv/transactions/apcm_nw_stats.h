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
/** \file apcm_nw_stats.h
 *
 * \brief APCM_NW_STATS primitive
 *
 **************************************************/
#ifndef APCM_NW_STATS_
#define APCM_NW_STATS_

/***************************************************
 *                 Include section
 ***************************************************/
#include "../base_types.h"
#include "definitions.h"

/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/**
 * \brief APCM_NW_STATS.REQ: This primitive implements a request to the STA to
 *        provide the Network stats
*/
typedef struct
{
   tE_BandId BandIdentifier; //!< Band to which the primitive refers
   t_NetId NID;  /*!< Network identifier (the least-significant 54 bits contains
                     the NID of the AVLN */
} tS_APCM_NW_STATS_REQ;

/** \brief Item in a list of STA's stats */
typedef struct st_PHYRates tS_PHYRates;
struct st_PHYRates
{
   
/*  AvgPHYDR_TX/RX:
			[10:00] Average PHY Data Rate in Mega Bits per second from STA to DA[0].
      [15:14]: Device type: 0: HPAV1.1, 1: HPAV2, 2: IEEE1901
      [13:12]: TX mode: 0:SISO1, 1:SISO2, 2:MIMO, 3:SISO_ONLY 
      [11]: beamformed */

   struct st_PHYRates *next; //!< next STA's stats.
   t_MACaddr   DA;   //!< MAC Address of the STA.
   
   TU8 txDevType;  
   TU8 txMode;
   TBool txBeamformed;
   TU16 AvgPHYDR_TX; //!< Average PHY Data Rate in MBits/sec from STA to DA
   
   TU8 rxDevType;
   TU8 rxMode;
   TBool rxBeamformed;
   TU16 AvgPHYDR_RX; //!< Average PHY Data Rate in MBits/sec from DA to STA
};

/**
 * \brief APCM_NW_STATS.CNF
 *        This message contains the list of all associated and authenticated
 *        STAs in the AVLN and the physical layer data rates to all those
 *        stations
*/
typedef struct
{
   TU8 NumSTAs;         //!< Number of AV STAs in the AVLN
   tS_PHYRates *first;  //!< first item in the list of AV STAs.
} tS_APCM_NW_STATS_CNF;

/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult  result;   //!< Transaction result
    tS_APL2C_ERROR_CNF    err;      //!< APL2C_ERROR_CNF
    tS_APCM_NW_STATS_CNF  cnf;      //!< APCM_NW_STATS.CNF
} tS_APCM_NW_STATS_Result;

/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief            Execute APCM_NW_STATS
 *
 * \param req        (in)  APCM_NW_STATS.REQ primitive
 * \param p_result   (out) Transaction result
*/
void Exec_APCM_NW_STATS(
   const tS_APCM_NW_STATS_REQ req,
   tS_APCM_NW_STATS_Result* p_result);

/**
 * \brief         Deallocate resources from a APCM_NW_STATS.CNF
 *
 * \param p_cnf   (in)  APCM_NW_STATS.CNF
*/
void Free_APCM_NW_STATS_CNF(tS_APCM_NW_STATS_CNF* p_cnf);


#endif /*APCM_NW_STATS_*/
