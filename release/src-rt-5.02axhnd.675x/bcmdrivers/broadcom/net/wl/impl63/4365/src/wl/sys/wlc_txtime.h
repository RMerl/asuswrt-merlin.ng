/*
 * Broadcom 802.11 Networking Device Driver
 *
 * Functionality relating to calculating the txtime
 * of frames, frame components, and frame exchanges.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_txtime.h 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief TX airtime calculation
 * Functionality relating to calculating the airtime
 * of frames, frame components, and frame exchanges.
 */

#ifndef _wlc_txtime_h_
#define _wlc_txtime_h_

#include "typedefs.h"
#include "wlc_types.h"
#include "wlc_rate.h"

/**
 * @brief a structure to hold parameters for packet tx time calculations
 *
 * This structure holds parameters for the time calculation and packet independent
 * partial results. This allows less work per-packet to calculation tx time.
 */
typedef struct timecalc {
	ratespec_t rspec;       /**< @brief ratespec for time calulations */
	uint fixed_overhead_us; /**< @brief length independent fixed time overhead */
	int is2g;               /**< @brief true for 2GHz band calculation */
	int short_preamble;     /**< @brief true if using short preamble */
} timecalc_t;

/**
 * @brief Calculate the airtime for a frame including preamble
 *
 * Calculate the airtime for a frame including preamble
 *
 * Calculation follows the TXTIME primitive for the PHYs we support where the TXVECTOR parmeters
 * are provided in ratespec, preamble_type, and psdu_len.
 *
 * @param	ratespec	the phy rate and other modulation parameters for frame
 * @param	band2g		true if frame is on 2.4G band
 * @param	short_preamble	For DSSS rates, 1 indicates Short Preamble, 0 indicates
 *                              Long Preamble.
 *				For HT, 1 indicates GreenField preamble, 0 indicates Mixed Mode.
 *                              For VHT, this parameter is 0 since VHT always uses Mixed Mode.
 *                              For OFDM, this parameter is 0 since there is only one preamble type.
 * @param	psdu_len	count of bytes in Phy Service Data Unit. For a simple MSDU frame
 *				this is the length from 802.11 FC field up to and including the FCS.
 *				For A-MPDUs, this is the total length of the A-MPDU adding MPDU
 *				delimiter fields, MPDU sub frame padding, and EOF padding
 *
 * @return	return value is the time in microseconds to transmit the entire frame
 *
 */
uint wlc_txtime(ratespec_t ratespec, bool band2g, int short_preamble, uint psdu_len);

/**
 * @brief Calculate the airtime for just the given byte length of a DATA portion of a PPDU
 *
 * Calculate the airtime for just the given byte length of a DATA portion of a PPDU
 *
 * Calculation follows the TXTIME primitive for the PHYs we support where the relavent TXVECTOR
 * parmeters are provided in ratespec. The time is the time for the number of symbols in the
 * DATA portion of the PPDU where the MSDU frame bytes are encoded.
 *
 * @param	ratespec	the phy rate and other modulation parameters for frame
 * @param	data_len	count of bytes for which to calculate the airtime
 *
 * @return	return value is the time in microseconds to transmit the given data portion
 *              of a frame
 *
 */
uint wlc_txtime_data(ratespec_t ratespec, uint data_len);

/**
 * @brief Calculate Ndbps (Number of Data Bits Per Symbol) value for the given ratespec.
 *
 * Calculate Ndbps (Number of Data Bits Per Symbol) value for the given ratespec.
 *
 * Return Ndbps (Number of Data Bits Per Symbol) value for the given ratespec
 * Note that the symbol time for OFDM, HT, and VHT is 4us, or 3.6us when using SGI,
 * and DSSS symbol time is 1us. Ndbps for DSSS rate 5.5Mbps will evaluate to 5.
 *
 * @param	ratespec	The ratespec specifying the modulation for which Ndbps is desired.
 *
 * @return	return value is the number of data bits per symbol
 *
 */
uint wlc_txtime_Ndbps(ratespec_t ratespec);

/**
 * @brief Calculate Nes (Number of Encoding Streams) value for the given ratespec.
 *
 * Calculate Nes (Number of Encoding Streams) value for the given ratespec.
 *
 * @param	ratespec	The ratespec specifying the modulation for which Nes is desired.
 *
 * @return	return value is the number of encoding streams
 */
uint wlc_txtime_Nes(ratespec_t ratespec);

#endif /* _wlc_txtime_h_ */
