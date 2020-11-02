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
 * $Id: wlc_mux.h 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief Transmit Path MUX layer for Broadcom 802.11 Networking Driver
 */

#ifndef _wlc_mux_h_
#define _wlc_mux_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>

/**
 * @brief State structure for the MUX module created by wlc_mux_module_attach()
 */
typedef struct wlc_mux_info wlc_mux_info_t;

/**
 * @brief State structure for an individual MUX created by wlc_mux_alloc()
 */
typedef struct wlc_mux wlc_mux_t;

/**
 * @brief The callback function a MUX source registers in wlc_mux_add_source()
 *	to provide data to the MUX
 */
typedef uint (*mux_output_fn_t)(void *ctx, uint ac, uint request_time, struct spktq *output_q);

/**
 * @brief Handle for a MUX source created by wlc_mux_add_source()
 */
typedef struct mux_source* mux_source_handle_t;

/*
 * Module Attach/Detach
 */

/**
 * @brief Allocate and initialize the MUX module.
 */
wlc_mux_info_t *wlc_mux_module_attach(wlc_info_t *wlc);

/**
 * @brief Free all resources of the MUX module
 */
void wlc_mux_module_detach(wlc_mux_info_t *muxi);

/*
 * MUX alloc/free
 */

/**
 * @brief Allocate and initialize a MUX
 */
wlc_mux_t *wlc_mux_alloc(wlc_mux_info_t *muxi, uint ac, uint quanta);

/**
 * @brief Free all resources of a MUX
 */
void wlc_mux_free(wlc_mux_t *mux);

/*
 * Configuration
 */

/**
 * @brief Set the default source output quanta in microseconds
 */
int wlc_mux_set_quanta(wlc_mux_t *mux, uint quanta);

/**
 * @brief Get the default source output quanta in microseconds
 */
uint wlc_mux_get_quanta(wlc_mux_t *mux);

/*
 * Output
 */

/**
 * @brief Request output packets from a MUX
 */
uint wlc_mux_output(wlc_mux_t *mux, uint request_time, struct spktq *output_q);

/*
 * Registration
 */

/**
 * @brief Add a new source to a MUX
 */
int wlc_mux_add_source(wlc_mux_t *mux, void *ctx, mux_output_fn_t output_fn,
                       mux_source_handle_t *phdl);

/**
 * @brief Delete a source from a MUX
 */
void wlc_mux_del_source(wlc_mux_t *mux, mux_source_handle_t hndl);

/**
 * @brief Update the output fn for a MUX source
 */
void wlc_mux_source_set_output_fn(wlc_mux_t *mux, mux_source_handle_t hndl,
                                  void *ctx, mux_output_fn_t output_fn);

/**
 * @brief Mark the source as no longer stalled
 */
void wlc_mux_source_wake(wlc_mux_t *mux, mux_source_handle_t hndl);

/**
 * @brief Mark the source as enabled so that its MUX will call the source's output function
 */
void wlc_mux_source_start(wlc_mux_t *mux, mux_source_handle_t hndl);

/**
 * @brief Mark the source as disabled so that its MUX will not call the source's output funciton
 */
void wlc_mux_source_stop(wlc_mux_t *mux, mux_source_handle_t hndl);

#endif /* _wlc_mux_h_ */
