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
 * $Id$
 */

/**
 * @file
 * @brief Transmit Path MUX layer helpers/utilities
 */

#ifndef _wlc_mux_utils_h_
#define _wlc_mux_utils_h_

#define MUX_GRP_BITMAP(fifo)	((uint)(1 << (fifo)))
#define MUX_GRP_AC_BK	(MUX_GRP_BITMAP(TX_AC_BK_FIFO))
#define MUX_GRP_AC_BE	(MUX_GRP_BITMAP(TX_AC_BE_FIFO))
#define MUX_GRP_AC_VI	(MUX_GRP_BITMAP(TX_AC_VI_FIFO))
#define MUX_GRP_AC_VO	(MUX_GRP_BITMAP(TX_AC_VO_FIFO))
#define MUX_GRP_BCMC	(MUX_GRP_BITMAP(TX_BCMC_FIFO))
#define MUX_GRP_ATIM	(MUX_GRP_BITMAP(TX_ATIM_FIFO))
#define MUX_GRP_DATA	\
	((MUX_GRP_AC_BK)|(MUX_GRP_AC_BE)|(MUX_GRP_AC_VI)|(MUX_GRP_AC_VO))

#define MUX_SRC_AC_BK	TX_AC_BK_FIFO
#define MUX_SRC_AC_BE	TX_AC_BE_FIFO
#define MUX_SRC_AC_VI	TX_AC_VI_FIFO
#define MUX_SRC_AC_VO	TX_AC_VO_FIFO
#define MUX_SRC_BCMC	TX_BCMC_FIFO
#define MUX_SRC_ATIM	TX_ATIM_FIFO

/**
 * @brief Start source attached to a mux queue
 *
 * @param msrcs          pointer to mux sources
 * @param src            source to start
 */
void wlc_msrc_start(mux_srcs_t *msrcs, uint src);

/**
 * @brief Wake source attached to a mux queue
 *
 * @param msrcs          pointer to mux sources
 * @param src            source to wake
 */
void wlc_msrc_wake(mux_srcs_t *msrcs, uint src);

/**
 * @brief Stop source attached to a mux queue
 *
 * @param msrcs          pointer to mux sources
 * @param src            source to stop
 */
void wlc_msrc_stop(mux_srcs_t *msrcs, uint src);

/**
 * @brief Start a group of sources attached to a mux queue
 *
 * @param msrcs           pointer to mux sources
 * @param grp             bitmap of mux srcs, if set sr,c is started
 */
void wlc_msrc_group_start(mux_srcs_t *msrcs, uint mux_grp_map);

/**
 * @brief Wake a group of sources attached to a mux queue
 *
 * @param msrcs           pointer to mux sources
 * @param grp             bitmap of mux srcs, if set, src is woken
 */
void wlc_msrc_group_wake(mux_srcs_t *msrcs, uint mux_grp_map);

/**
 * @brief Stop a group of sources attached to a mux queue
 *
 * @param msrcs          pointer to mux sources
 * @param grp            bitmap of mux srcs, if set, src is stopped
 */
void wlc_msrc_group_stop(mux_srcs_t *msrcs, uint mux_grp_map);

/**
 * @brief Allocate and initialize mux sources
 *
 * @param wlc            pointer to wlc_info_t
 * @param mux            pointer to list of muxes to attach sources
 * @param nfifo          number of fifos in low txq
 * @param ctx            pointer to context for mux source output function
 * @param output_fn      mux output function
 * @param mux_grp_map    bitmap of mux srcs, if set, src is alloc/init
 *
 * @return               mux_srcs_t containing mux sources
 */
mux_srcs_t *wlc_msrc_alloc(wlc_info_t *wlc, wlc_mux_t** mux, uint nfifo,
	void *ctx, mux_output_fn_t output_fn, uint mux_grp_map);

/**
 * @brief Free all resources of the mux sources
 *
 * @param wlc            pointer to wlc_info_t
 * @param osh            pointer portlayer osl routines
 * @param srcs           pointer to structure of mux sources
 */
void wlc_msrc_free(wlc_info_t *wlc, osl_t *osh, mux_srcs_t *srcs);

#endif /* _wlc_mux_utils_h_ */
