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
 * $Change: 86159 $
 ***********************************************************************/
/** \file apchannel_monitor.h
 *
 * \brief APCHANNEL_MONITOR primitive
 *
 **************************************************/

#ifndef APCHANNEL_MONITOR_H_
#define APCHANNEL_MONITOR_H_

/***************************************************
*                 Include section
***************************************************/
#include "../base_types.h"
#include "definitions.h"

/***************************************************
 *                 Public Typedefs Section
 ***************************************************/
/** \brief Measurement type. */
typedef enum
{
   SNR = 0,                 // SNR
   CFR,                     // CFR
   FFT,                     // FFT
   LLR,                     // LLR Weights. Not available
   COV,                     // Covariance. Not available
   ANG,                     // Precoding Angles measured by Hw
   BIT_LOADING_RX,          // Bit Loading of Rx Tone Map
   BIT_LOADING_TX,          // Bit Loading of Tx Tone Map
   PRECO_TM_RX,             // Precoding Angles of Rx Tone Map
   PRECO_TM_TX,             // Precoding Angles of Tx Tone Map
   SNR_FILT,                // Filtered SNR
   PRECOD_FILT,             // Filtered precoding angles
   BLER,                    // BLER statistics
   DATA,                    // Raw data dump
   OVERVIEW,                // Channel traffic overview.
   SNIFFER,                 // Sniffer messages
   SNR_VAR,                 // SNR Variance
   SNR_ALTER                // SNR filtered for Alternate Beamforming mode
} tE_MeasurementType;

/** \brief Sniffer Operating Mode */
typedef enum
{
  SNIFFER_OP_MODE_BASIC = 0,
  SNIFFER_OP_MODE_EXTENDED,
} tE_Sniffer_Op_Mode;

/**
   \brief APCHANNEL_MONITOR.REQ

   Request an specific channel measure with respect to an endpoint STA, in a
   given band.
*/
typedef struct
{
   tE_BandId   BandIdentifier;   //!< Band to which the primitive refers.
   TU8   Action;  /*!< Action to be done:
                     0: register a new petition
                     1: unregister an existing petition */
   t_MACaddr   MC;   /*!< MAC address of the equipment in the opposite side of
                        the monitored channel. */
   tE_MeasurementType   measure_type;  //!< Measurement type
   TU8                  interval_mask; // bit mask containing requested intervals for measurement.
   TU16                 report_period; // Minimum period between 2 consecutive measurements.
} tS_APCHANNEL_MONITOR_REQ;

/**
   \brief APCHANNEL_MONITOR_SNIFFER.REQ

   Enable/Disable BCM60500 sniffer.
*/
typedef struct
{
  TU8                 Action;  /*!< Action to be done:
                                    0: register a new petition
                                    1: unregister an existing petition */
  TU32                SnifferID; //!< uniquely identifies a Sniffer session
  t_MACaddr           HLE;       /*!< MAC address of the HLE host which is
                                  going to receive Sniffer measurements */
  tE_Sniffer_Op_Mode  operating_mode;
} tS_APCHANNEL_MONITOR_SNIFFER_REQ;

/** \brief APCHANNEL_MONITOR.CNF */
typedef struct
{
   TBool   OK; //!< registration success?
   TU8   Action;  //!< 0: add petition 1: release slot
   t_MACaddr   MC;   /*!< MAC address of the equipment in the opposite side of
                        the monitored channel. */
   tE_MeasurementType   measure_type;  //!< Measurement type
   tE_BandId   measure_band;  //!< Measurement band
} tS_APCHANNEL_MONITOR_CNF;


/** \brief This is the struct to hold the transaction response. */
typedef struct
{
    tE_TransactionResult  result;   //!< Transaction result
    tS_APL2C_ERROR_CNF    err;      //!< APL2C_ERROR_CNF
    tS_APCHANNEL_MONITOR_CNF  cnf; //!< APCHANNEL_MONITOR.CNF
} tS_APCHANNEL_MONITOR_Result;

/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/**
   \brief Execute APCHANNEL_MONITOR

   \param req  APCHANNEL_MONITOR.REQ primitive.
   \param result  Transaction result.
*/
void Exec_APCHANNEL_MONITOR(
   const tS_APCHANNEL_MONITOR_REQ req,
   tS_APCHANNEL_MONITOR_Result *result);

/**
   \brief Execute APCHANNEL_MONITOR_SNIFFER

   \param req  APCHANNEL_MONITOR_SNIFFER_REQ primitive.
   \param result  Transaction result.
*/
void Exec_APCHANNEL_MONITOR_SNIFFER(
   const tS_APCHANNEL_MONITOR_SNIFFER_REQ req,
   tS_APCHANNEL_MONITOR_Result *result);

/**
   \brief   Register a callback notification function

   In order to receive APCHANNEL_MONITOR.IND from the STA, the client must to
   register a callback function with this prototype:

      void callback(TU8 *data,TU16 size)

   Then, when the library receives an APCHANNEL_MONITOR.IND from the STA it will
   notify to the clients registered before.


   \param callback  function pointer
*/
void Suscribe_APCHANNEL_MONITOR_IND(void (*callback)(TU8*,TU16));


/**
   \brief   Unregister a callback notification function
   \param callback  function pointer
*/
void Unsubscribe_APCHANNEL_MONITOR_IND(void (*callback)(TU8*,TU16));

/**
   \brief   Register a callback notification function

   The functionality is the same as for Suscribe_APCHANNEL_MONITOR_IND.
   But the client has to register a callback function with this prototype:

      void callback(TU8 *data,TU16 size, TU8 *authMAC)

  Where:  'data' is a pointer to the received data array
          'size' is the size of the received data array
          'authMAC' is a pointer to the address of the STA
                    that sent this APCHANNEL_MONITOR_IND message.
                    The pointed buffer will contain 6 char values reprenting a MAC address.

   Then, when the library receives an APCHANNEL_MONITOR.IND from the STA it will
   notify to the clients registered before.


   \param callback  function pointer
*/
void Suscribe_Rich_APCHANNEL_MONITOR_IND(void (*callback)(TU8*,TU16,TU8*));


/**
   \brief   Unregister a callback notification function.
            Works with Suscribe_Rich_APCHANNEL_MONITOR_IND
   \param callback  function pointer
*/
void Unsubscribe_Rich_APCHANNEL_MONITOR_IND(void (*callback)(TU8*,TU16,TU8*));


#endif /*APCHANNEL_MONITOR_H_*/
