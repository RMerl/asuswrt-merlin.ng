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
/** \file apcm_nw_info.h
 *
 * \brief APCM_NW_INFO primitive
 *
 **************************************************/

#ifndef APCM_NW_INFO_H_
#define APCM_NW_INFO_H_

/***************************************************
 *                 Include section
 ***************************************************/
#include "../base_types.h"
#include "definitions.h"


/***************************************************
 *                 Public Typedefs Section
 ***************************************************/
/**
 * \brief The APCM_NW_INFO.REQ primitive is a request from HLE to provide the
 *        list of AVLNs to which the STA is a member and the relevant
 *        information about the AVLN.
*/
typedef struct
{
   tE_BandId   band; //!< Band
   TU8 command;      /*!< 0x00 Query info  on  Network(s) this sta belongs to
                          0x01 Query info  on  All Visible Networks. */
} tS_APCM_NW_INFO_REQ;

/** \brief Station roles */
typedef enum
{
   PLC_NODE_UNASSOC_STA = 0,
   PLC_NODE_UNASSOC_CCO,
   PLC_NODE_STA,
   PLC_NODE_CCO
} tE_StationRole;

/** \brief Types of network access */
typedef enum
{
   IN_HOME = 0,
   ACCESS
} tE_Access;

/** \brief AVLN stats */
typedef enum
{
   JOINED = 0,
   NOT_JOINED_HAVE_NMK,
   NOT_JOINED_NO_NMK
} tE_AVLNStat;

/** \brief Item in a list of AVLNs */
typedef struct st_AVLN tS_AVLN;
struct st_AVLN
{
   struct st_AVLN *next; //!< next AVLN
   t_NetId NID;  /*!< Network identifier (the least-significant 54 bits contains
                     the NID of the AVLN. */
   TU8   SNID; /*!< Short Network Identifier (the least-significant 4 bits of
                    this field contains the Short Network Identifier. */
   t_TEI   TEI;  //!< Terminal Equipment Identifier of the STA in the AVLN.
   tE_StationRole   StationRole;   //!< Role of the station in the AVLN.
   t_MACaddr   CCo_MACAddr;   //!< MAC Address of the CCo of the network.
   tE_Access   Access;  //!< Access network
   TU8   NumCordNWs; /*!< Number of Neighbor Networks that are coordinating with
                        the AVLN */
   tE_AVLNStat STATUS;  //!< Status of AVLN
};

/** \brief APCM_NW_INFO.CNF */
typedef struct
{
   TU8 NumNWs; /*!< Number of AVLNs that the STA is a member. If STA is member
                    of multiple networks, tS_AVLN contains the information about
                    the AVLN whose PHY Clock the STA is tracking.
               */
   tS_AVLN *first; //!< first item in the list of AVLNs.
} tS_APCM_NW_INFO_CNF;

/** \brief This is the struct to hold the transaction response */
typedef struct
{
    tE_TransactionResult   result;   //!< Transaction result
    tS_APL2C_ERROR_CNF     err;      //!< APL2C_ERROR_CNF
    tS_APCM_NW_INFO_CNF    cnf;      //!< APCM_NW_INFO.CNF
} tS_APCM_NW_INFO_Result;

/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
 * \brief            Execute APCM_NW_INFO
 *
 * \param req        (in)  APCM_NW_INFO.REQ primitive
 * \param p_result   (out) Transaction result
*/
void Exec_APCM_NW_INFO(
   tS_APCM_NW_INFO_REQ req,
   tS_APCM_NW_INFO_Result* p_result);

/**
 * \brief         Deallocate resources from a APCM_NW_INFO.CNF
 *
 * \param p_cnf   (in) APCM_NW_INFO.CNF
*/
void Free_APCM_NW_INFO_CNF(tS_APCM_NW_INFO_CNF* p_cnf);

#endif /*APCM_NW_INFO_H_*/
