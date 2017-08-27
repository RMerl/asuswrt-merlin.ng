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
/** \file apcm_get_newsta.h
 *
 * \brief APCM_GET_NEWSTA primitive
 *
 **************************************************/

#ifndef APCM_GET_NEWSTA_H_
#define APCM_GET_NEWSTA_H_

/***************************************************
 *                 Include section
 ***************************************************/
#include "../base_types.h"
#include "definitions.h"

/** \brief STA visibility */
typedef enum
{
   NO_VISIBLE_SAME_AVLN = 0,
   VISIBLE_NOT_IN_OUR_AVLN,
   SAME_AVLN_AND_VISIBLE
} tE_Visibility;


/** \brief Item in a list of visibles STA */
typedef struct st_VisibleSTA tS_VisibleSTA;
struct st_VisibleSTA
{
   struct st_VisibleSTA *next;         //!< next new STA
   t_MACaddr            MAC;           //!< MAC address of new STA
   t_TEI                TEI;           //!< TEI of this STA
   tE_BandId            BandId;        //!< Band supported by the STA
   TU8                  HFID_MFG_len;  //!< Manufacturer-set HFID len
   TChar                HFID_MFG[64];  //!< Manufacturer-set HFID of new STA
   TU8                  HFID_USER_len; //!< User-set HFID len
   TChar                HFID_USER[64]; //!< User-set HFID of new STA
   tE_Visibility        visibility;    //!< new STA visibility
};


/** \brief Response with the num of visible STA along with their band info */
typedef struct
{
   TU8            num_stas;   //!< Number of STAs
   tS_VisibleSTA* first;      //!< first item in the list of visibles STA.
} tS_APCM_GET_NEWSTA_CNF;


/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult   result;  //!< Transaction result
    tS_APL2C_ERROR_CNF     err;     //!< APL2C_ERROR_CNF
    tS_APCM_GET_NEWSTA_CNF cnf;     //!< APCM_GET_NEWSTA_CNF
} tS_APCM_GET_NEWSTA_Result;


/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief            Execute APCM_GET_NEWSTA transaction
 *                   Request to the STA from the HLE, to provide all Visible
 *                   STAs, along with the band they belong to
 *
 * \param p_result   (out) Transaction result
*/
void Exec_APCM_GET_NEWSTA(tS_APCM_GET_NEWSTA_Result* p_result);


/**
 * \brief         Deallocate resources from a APCM_GET_NEWSTA.CNF
 *
 * \param p_cnf   (in) APCM_GET_NEWSTA.CNF
*/
void Free_APCM_GET_NEWSTA_CNF(tS_APCM_GET_NEWSTA_CNF* p_cnf);


#endif /*APCM_GET_NEWSTA_H_*/
