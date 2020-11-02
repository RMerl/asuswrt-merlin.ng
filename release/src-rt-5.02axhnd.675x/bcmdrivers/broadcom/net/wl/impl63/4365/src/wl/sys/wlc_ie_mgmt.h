/*
 * IE management module interface - this is a top-level function
 * to manage building and/or parsing of IEs in the following
 * management frame types:
 *
 *	FC_ASSOC_REQ
 *	FC_ASSOC_RESP
 *	FC_REASSOC_REQ
 *	FC_REASSOC_RESP
 *	FC_PROBE_REQ
 *	FC_PROBE_RESP
 *	FC_BEACON
 *	FC_DISASSOC
 *	FC_AUTH
 *	FC_DEAUTH
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
 * $Id: wlc_ie_mgmt.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_ie_mgmt_h_
#define _wlc_ie_mgmt_h_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>

/*
 * special Frame Types
 */
#ifdef IEM_TEST
/* Fake Frame Type for test */
#define WLC_IEM_FC_TEST 0x00D0
#endif // endif
/* Fake Frame Type for parsing PrbRsp frame during scan */
#define WLC_IEM_FC_SCAN_PRBRSP 0x0060
/* Fake Frame Type for parsing Beacon frame during scan */
#define WLC_IEM_FC_SCAN_BCN 0x0070
/* Fake Frame Type for parsing Beacon frame received by AP */
#define WLC_IEM_FC_AP_BCN 0x0090

/*
 * module attach/detach
 */
extern wlc_iem_info_t *wlc_iem_attach(wlc_info_t *wlc);
extern void wlc_iem_detach(wlc_iem_info_t *iem);

/*
 * Register a pair of 'calc_len'/'build' callbacks for an non Vendor Specific IE
 * in one frame type.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - tag: non Vendor Specific IE tag DOT11_MNG_XXXX as defined in 802.11.h
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_add_build_fn(wlc_iem_info_t *iem, uint16 ft, uint8 tag,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx);

/*
 * Register a pair of 'calc_len'/'build' callbacks for an non Vendor Specific IE
 * in multiple frame types.
 *
 * Inputs:
 * - fstbmp: Bitmap of frame subtype(s) FC_SUBTYPE_XXXX as defined in 802.11.h
 *           (see also WLC_IEM_FC_SCAN_XXXX)
 * - tag: non Vendor Specific IE tag DOT11_MNG_XXXX as defined in 802.11.h
 *
 * Bit index in frame subtype bitmap is the frame subtype FC_SUBTYPE_XXXX as defined
 * in 802.11.h. Use FT2BMP() macro to convert a Frame Type to Frame Subtype Bitmap.
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_add_build_fn_mft(wlc_iem_info_t *iem, uint16 fstbmp, uint8 tag,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx);

/*
 * Register a pair of 'calc_len'/build_frame' callbacks for an Vendor Specific IE
 * in one frame type.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - prio: Relative priority/position of Vendor Specific IE
 *
 * Known priorities are defined by WLC_IEM_VS_IE_PRIO_XXXX (0 at the first in the frame
 * while 249 at the last) in wlc_iem_vs.h.
 *
 * Positions of Vendor Specific IEs are undefined when the same 'prio' value is
 * used for multiple Vendor Specific IEs.
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_vs_add_build_fn(wlc_iem_info_t *iem, uint16 ft, uint8 prio,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx);

/*
 * Register a pair of 'calc_len'/build_frame' callbacks for an Vendor Specific IE
 * in multiple frame types.
 *
 * Inputs:
 * - fstbmp: Bitmap of frame subtype(s) FC_SUBTYPE_XXXX as defined in 802.11.h
 *           (see also WLC_IEM_FC_SCAN_XXXX)
 * - prio: Relative priority/position of Vendor Specific IE
 *
 * Bit index in frame subtype bitmap is the frame subtype FC_SUBTYPE_XXXX as defined
 * in 802.11.h. Use FT2BMP macro to convert a Frame Type to Frame Subtype Bitmap.
 *
 * Known priorities are defined by WLC_IEM_VS_IE_PRIO_XXXX (0 at the first in the frame
 * while 249 at the last) in wlc_iem_vs.h.
 *
 * Positions of Vendor Specific IEs are undefined when the same 'prio' value is
 * used for multiple Vendor Specific IEs.
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_vs_add_build_fn_mft(wlc_iem_info_t *iem, uint16 fstbmp, uint8 prio,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx);

/*
 * Calculate all IEs' length in a frame.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - uiel: User supplied IEs list (IEs to be merged into the frame)
 * - cbparm: Parameters passed to callbacks
 *
 * Return:
 * - IEs' total length in bytes
 */
extern uint wlc_iem_calc_len(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm);

/*
 * Calculate an non Vendor Specific IE's length.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - tag: non Vendor Specific IE's tag DOT11_MNG_XXXX as defined in 802.11.h
 * - uiel: User supplied IEs list (IEs to be merged into the frame)
 * - cbparm: Parameters passed to the callback
 *
 * Return:
 * - IE's length in bytes
 */
extern uint wlc_iem_calc_ie_len(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm);

/*
 * Calculate a Vendor Specific IE's length.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - tag: non Vendor Specific IE's tag DOT11_MNG_XXXX as defined in 802.11.h
 * - uiel: User supplied IEs list (IEs to be merged into the frame)
 * - cbparm: Parameters passed to the callback
 *
 * Return:
 * - IE's length in bytes
 */
extern uint wlc_iem_vs_calc_ie_len(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 prio, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm);

/*
 * Write all IEs in a frame into buffer.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - uiel: User supplied IEs list (IEs to be merged into the frame)
 * - cbparm: Parameters passed to callbacks
 * - buf: buffer pointer
 * - buf_len: buffer length
 *
 * Outputs:
 * - buf: IEs including User Supplied IEs if any all written into the buffer
 *
 * Return:
 * - a negative return value indicates an error (BCME_XXXX).
 */
extern int wlc_iem_build_frame(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len);

/*
 * Write an non Vendor Specific IE into a buffer.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - tag: non Vendor Specific IE tag DOT11_MNG_XXXX as defined in 802.11.h
 * - uiel: User supplied IEs list (IEs to be merged into the frame)
 * - cbparm: Parameters passed to the callback
 * - buf: buffer pointer
 * - buf_len: buffer length
 *
 * Outputs:
 * - buf: IE written into the buffer
 *
 * Return:
 * - a negative return value indicates an error (BCME_XXXX).
 */
extern int wlc_iem_build_ie(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len);

/*
 * Write a Vendor Specific IE into a buffer.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - prio: Vendor Specific IE prio WLC_IEM_VS_IE_PRIO as defined in wlc_ie_mgmt_vs.h
 * - uiel: User supplied IEs list (IEs to be merged into the frame)
 * - cbparm: Parameters passed to the callback
 * - buf: buffer pointer
 * - buf_len: buffer length
 *
 * Outputs:
 * - buf: IE written into the buffer
 *
 * Return:
 * - a negative return value indicates an error (BCME_XXXX).
 */
extern int wlc_iem_vs_build_ie(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 prio, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len);

/*
 * Register 'parse' callback for an non Vendor Specific IE 'tag'
 * in one frame type.
 *
 * The callback is invoked when parsing a frame with the IE.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - tag: non Vendor Specific IE tag DOT11_MNG_XXXX as defined in 802.11.h
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_add_parse_fn(wlc_iem_info_t *iem, uint16 ft, uint8 tag,
	wlc_iem_parse_fn_t parse_fn, void *ctx);

/*
 * Register 'parse' callback for an non Vendor Specific IE 'tag'
 * in multiple frame types.
 *
 * The callback is invoked when parsing a frame with the IE.
 *
 * Inputs:
 * - fstbmp: Bitmap of frame subtype(s) FC_SUBTYPE_XXXX as defined in 802.11.h
 *           (see also WLC_IEM_FC_SCAN_XXXX)
 * - tag: non Vendor Specific IE tag DOT11_MNG_XXXX as defined in 802.11.h
 *
 * Bit index in frame subtype bitmap is the frame subtype FC_SUBTYPE_XXXX as defined
 * in 802.11.h. Use FT2BMP() macro to convert a Frame Type to Frame Subtype Bitmap.
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_add_parse_fn_mft(wlc_iem_info_t *iem, uint16 fstbmp, uint8 tag,
	wlc_iem_parse_fn_t parse_fn, void *ctx);

/*
 * Register 'parse' callback for an Vendor Specific IE 'id'
 * in one frame type 'ft'.
 *
 * The callback is invoked when parsing a frame with the IE.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - id: Vendor Specific IE ID
 *
 * Known IDs are defined by WLC_IEM_VS_IE_PRIO_XXXX in wlc_iem_vs.h.
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_vs_add_parse_fn(wlc_iem_info_t *iem, uint16 ft, uint8 id,
	wlc_iem_parse_fn_t parse_fn, void *ctx);

/*
 * Register 'parse' callback for an Vendor Specific IE 'id'
 * in multiple frame types.
 *
 * The callback is invoked when parsing a frame with the IE.
 *
 * Inputs:
 * - fstbmp: Bitmap of frame subtype(s) FC_SUBTYPE_XXXX as defined in 802.11.h
 *           (see also WLC_IEM_FC_SCAN_XXXX)
 * - id: Vendor Specific IE ID
 *
 * Bit index in frame subtype bitmap is the frame subtype FC_SUBTYPE_XXXX as defined
 * in 802.11.h. Use FT2BMP() macro to convert a Frame Type to Frame Subtype Bitmap.
 *
 * Known IDs are defined by WLC_IEM_VS_IE_PRIO_XXXX in wlc_iem_vs.h.
 *
 * Return:
 * - a negative return value indicates an error.
 */
extern int wlc_iem_vs_add_parse_fn_mft(wlc_iem_info_t *iem, uint16 fstbmp, uint8 id,
	wlc_iem_parse_fn_t parse_fn, void *ctx);

/*
 * Parse all IEs in a frame.
 *
 * Inputs:
 * - ft: Frame type FC_XXXX as defined in 802.11.h (see also WLC_IEM_FC_SCAN_XXXX)
 * - upp: User parse parameters passed to IE mgmt. module
 * - pparm: Parameters passed to callbacks
 * - buf: IEs' buffer pointer
 * - buf_len: IEs' buffer length
 *
 * A classification callback must be provided through upp parameter
 * to map the variable length OUI (and TYPE in most cases) in Vendor Specific IEs
 * to IDs used at callbacks' registrations.
 *
 * Return:
 * - a negative return value indicates an error (BCME_XXXX).
 */
extern int wlc_iem_parse_frame(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_upp_t *upp, wlc_iem_pparm_t *pparm, uint8 *buf, uint buf_len);

#endif /* _wlc_ie_mgmt_h_ */
