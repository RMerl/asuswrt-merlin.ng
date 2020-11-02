/*
 * This file defines a collection of low-level IE management functions to:
 *
 * 1. manipulate IE management related callback tables
 * 2. register callbacks
 * 3. invoke callbacks
 *
 * The user provides callbacks table's storage.
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
 * $Id: wlc_ie_mgmt_lib.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_ie_mgmt_lib_h_
#define _wlc_ie_mgmt_lib_h_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>

/*
 * 'calc_len'/'build' callback pair callback table entry
 */
typedef struct {
	wlc_iem_calc_fn_t calc;
	wlc_iem_build_fn_t build;
	void *ctx;
} wlc_iem_cbe_t;

/*
 * Register 'calc_len'/'build' callback pair
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build callback table and size.
 * 'calc_fn', 'build_fn', and 'ctx' are the callback functions and context.
 * 'tag' is a value (IE tag or VS IE prio) that the callback pair is registered for.
 */
extern void wlc_ieml_add_build_fn(uint8 *build_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx,
	uint8 tag);

/*
 * Invoke to calculate all IEs' length.
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'calc_len' callbacks as is.
 */
extern uint wlc_ieml_calc_len(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm);

/*
 * Invoke to calculate a specific non Vendor Specific IE's length.
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'calc_len' callbacks as is.
 * 'tag' is the specific IE's tag.
 */
extern uint wlc_ieml_calc_ie_len(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm);

/*
 * Invoke to write all IEs into buffer
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'build' callbacks as is.
 * 'buf' and 'buf_len' are the buffer and its length the IEs are written to.
 */
extern int wlc_ieml_build_frame(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm,
	uint8 *buf, uint buf_len);

/*
 * Invoke to write a specific IE into buffer
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'build' callbacks as is.
 * 'buf' and 'buf_len' are the buffer and its length the IEs are written to.
 * 'tag' is the specific IE's tag.
 */
extern int wlc_ieml_build_ie(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len);

/*
 * Sort calc_len/build callback table (build_tag + build_cb) entries based on
 * the tags order in the tag table 'tag'.
 */
extern void wlc_ieml_sort_cbtbl(uint8 *build_tag, wlc_iem_cbe_t *buidd_cb, uint16 build_cnt,
	const uint8 *tag, uint16 cnt);

/*
 * 'parse' callback table entry
 */
typedef struct {
	wlc_iem_parse_fn_t parse;
	void *ctx;
} wlc_iem_pe_t;

/*
 * Register 'parse' callback
 *
 * 'parse_tag', 'parse_cb', and 'parse_cnt' are the callback registration table and size.
 * 'parse_fn' and ctx are callback function and context.
 * 'tag' is a value (IE tag or VS IE id) that the callback is registered for.
 */
extern void wlc_ieml_add_parse_fn(uint8 *parse_tag, wlc_iem_pe_t *parse_cb, uint16 parse_cnt,
	wlc_iem_parse_fn_t parse_fn, void *ctx,
	uint8 tag);

/*
 * Invoke to parse all IEs in buffer.
 *
 * 'parse_tag', 'parse_cb', and 'parse_cnt' are callback registration table and size.
 * 'is_tag' indicates if the tag table 'parse_tag' contains IE tags or VS IE IDs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the parse callbacks as is.
 * 'buf' and 'buf_len' are the buffer containing the IEs and their length in bytes.
 */
extern int wlc_ieml_parse_frame(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *parse_tag, bool is_tag, wlc_iem_pe_t *parse_cb, uint16 parse_cnt,
	wlc_iem_upp_t *upp, wlc_iem_pparm_t *pparm,
	uint8 *buf, uint buf_len);

#endif /* _wlc_ie_mgmt_lib_h_ */
