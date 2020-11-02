/*
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
 * $Id: wlc_scbq.h 708017 2017-06-29 14:11:45Z $
 */
/**
 * @file
 * @brief Per-SCB Tx Queuing modulue for Broadcom 802.11 Networking Driver
 */

#ifndef _wlc_scbq_h_
#define _wlc_scbq_h_

#include <typedefs.h>
#include "wlc_types.h"
#include "wlc_mux.h"
#include "wlc_txtime.h"

/**
 * @brief State structure for the SCBQ module created by wlc_scbq_module_attach()
 */
typedef struct wlc_scbq_info wlc_scbq_info_t;

/* Flow Control stop flags for an SCB */
typedef enum scbq_stop_flag {
	SCBQ_FC_BLOCK_DATA = 0x01,
	SCBQ_FC_ASSOC      = 0x02,
	SCBQ_FC_PS         = 0x04
} scbq_stop_flag_t;

/*
 * Module Attach/Detach
 */

/**
 * @brief Allocate and initialize the SCBQ module.
 */
wlc_scbq_info_t *wlc_scbq_module_attach(wlc_info_t *wlc);

/**
 * @brief Free all resources of the SCBQ module
 */
void wlc_scbq_module_detach(wlc_scbq_info_t *scbq_info);

/**
 * @brief Update the output fn an SCBQ
 */
void wlc_scbq_set_output_fn(wlc_scbq_info_t *scbq_info, struct scb *scb,
                            void *ctx, mux_output_fn_t output_fn);

/**
 * @brief Reset the output fn for an SCBQ to the default output fn
 */
void wlc_scbq_reset_output_fn(wlc_scbq_info_t *scbq_info, struct scb *scb);

/**
 * @brief Return the given scb's tx queue
 */
struct pktq* wlc_scbq_txq(wlc_scbq_info_t *scbq_info, struct scb *scb);

/**
 * @brief Set a Stop Flag to prevent tx output from all SCBs
 */
void wlc_scbq_global_stop_flag_set(wlc_scbq_info_t *scbq_info, scbq_stop_flag_t flag);

/**
 * @brief Clear a Stop Flag preventing tx output from all SCBs
 */
void wlc_scbq_global_stop_flag_clear(wlc_scbq_info_t *scbq_info, scbq_stop_flag_t flag);

/**
 * @brief Set a Stop Flag to prevent tx output from the given scb
 */
void wlc_scbq_scb_stop_flag_set(wlc_scbq_info_t *scbq_info, struct scb *scb, scbq_stop_flag_t flag);

/**
 * @brief Clear a Stop Flag preventing tx output from the given scb
 */
void wlc_scbq_scb_stop_flag_clear(wlc_scbq_info_t *scbq_info, struct scb *scb,
                                  scbq_stop_flag_t flag);

/**
 * @brief Update the scbq state to indicate that the mux source is stalled.
 */
void wlc_scbq_scb_stall_set(wlc_scbq_info_t *scbq_info, struct scb *scb, uint ac);

/**
 * @brief Clear a mux source stall condition for the given scb
 */
void wlc_scbq_scb_stall_clear(wlc_scbq_info_t *scbq_info, struct scb *scb, uint ac);

/**
 * @brief Calculate a TX airtime estimate using a previously initialized timecalc_t struct
 */
uint wlc_scbq_timecalc(timecalc_t *tc, uint mpdu_len);

/**
 * @brief Packet drop policy handler
 */
typedef void (*scbq_overflow_fn_t)(wlc_scbq_info_t *scbq_info,
	struct scb *scb, struct pktq *q, void *pkt, uint prec);
/**
 *  Set the packet overflow handler for SCBQ
 */
void wlc_scbq_register_overflow_fn(wlc_scbq_info_t *scbq_info,
	struct scb *scb, scbq_overflow_fn_t overflow_fn);
#endif /* _wlc_scbq_h_ */
