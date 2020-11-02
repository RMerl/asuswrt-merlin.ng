/*
 * IE management data structures and types
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
 * $Id: wlc_ie_mgmt_types.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_ie_mgmt_types_h_
#define _wlc_ie_mgmt_types_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>
#include <proto/ethernet.h>	/* to get wlc_wrxh_t in d11.h */
#include <d11.h>
#include <wlc_rate.h>		/* to get ratespec_t */

/*
 * unknown frame type
 */
#define WLC_IEM_FC_UNK 0xffff

/*
 * unknown frame type
 */
#define NARROW_BW_NONE 0
#define NARROW_BW_20 1
#define NARROW_BW_40 2

/* ******** 'calc_len'/'build' callback pair & registration ******** */

/*
 * forward declarations
 */
typedef struct wlc_iem_cbparm wlc_iem_cbparm_t;
typedef union wlc_iem_ft_cbparm wlc_iem_ft_cbparm_t;

/*
 * 'calc_len' callback - invoked to calculate the length of an IE.
 *
 * This callback is invoked from either wlc_iem_build_frame or
 * wlc_iem_calc_len APIs.
 *
 * - 'cbparm' points to the user supplied calc_len/build parameters
 *   structure from the wlc_iem_calc_len()/wlc_iem_build_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie/ie_len' points to the user supplied IE which the callback
 *   can use as a template to calculate the length of/build an new IE.
 *   (the 'ie''s tag is of course the same as the 'tag')
 * - 'tag' is that the callback was registered for.
 *
 * The caller and the callbacks must agree on 'cbparm' and 'cfg' which may
 * be set to NULL by the caller.
 *
 * The 'ie/ie_len' is NULL/0 when there is no user supplied IE list to merge.
 *
 * It should return the length of the IE in bytes.
 */
typedef struct {
	wlc_iem_cbparm_t *cbparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;	/* Frame type */
	uint8 *ie;	/* user supplied IE */
	uint ie_len;
	uint8 tag;	/* IE tag */
} wlc_iem_calc_data_t;
typedef uint (*wlc_iem_calc_fn_t)(void *ctx, wlc_iem_calc_data_t *data);

/*
 * 'build' callback - invoked to write the IE into a buffer.
 *
 * This callback is invoked from wlc_iem_build_frame API.
 *
 * - 'cbparm' points to the user supplied calc_len/build parameters
 *   structure from the wlc_iem_calc_len()/wlc_iem_build_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie/ie_len' points to the user supplied IE which the callback can use
 *   as a template to calculate the length of/build an new IE.
 *   (the 'ie''s tag is of course the same as the 'tag')
 * - 'tag' is that the callback was registered for.
 * - 'buf'/'buf_len' is the buffer pointer/length into which the callback
 *   writes the IE.
 *
 * The caller and the callbacks must agree on 'cbparm' and 'cfg' which may
 * be set to NULL by the caller.
 *
 * The 'ie/ie_len' is NULL/0 when there is no user supplied IE list to merge.
 *
 * The callback can use 'buf_len' to verify if the buffer is big enough
 * to hold the IE it is going to write.
 *
 * A negative return value indicates an error (BCME_XXXX), and will stop
 * wlc_iem_build_frame API immediately.
 */
typedef struct {
	wlc_iem_cbparm_t *cbparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;	/* Frame type */
	uint8 *ie;	/* user supplied IE */
	uint ie_len;
	uint8 tag;	/* IE tag */
	uint8 *buf;	/* IE buffer pointer */
	uint buf_len;
} wlc_iem_build_data_t;
typedef int (*wlc_iem_build_fn_t)(void *ctx, wlc_iem_build_data_t *data);

/* ******** 'calc_len'/'build' callback invocation ******** */

/* 'insert ie' query callback - invoked to query if the user supplied IE 'ie'
 * can be inserted into the frame at between tags 'prev' and 'tag' when there
 * is no 'calc_len/build' callback pair registered for the tag of the user
 * supplied IE 'ie'.
 *
 * - 'cbparm' points to the user supplied calc_len/build parameters
 *   structure from the wlc_iem_calc_len()/wlc_iem_build_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'prev'/'tag' - 'prev' is the last processed registered callback's tag and
 *   'tag' is the current being processed registered callback's tag. 'prev' and
 *   'tag' are IE tags when the user supplied IE 'ie' is an non Vendor Specific
 *   IE and they are the priorities otherwise.
 * - 'ie/ie_len' points to the user supplied IE which the callback decides
 *   if it can be inserted at between tags 'prev' and 'tag'.
 *
 * It should return TRUE if the user supplied IE 'ie' can be inserted into the frame.
 *
 * It is used to merge non Vendor Specific IEs in the user supplied IEs list
 * with the driver generated non Vendor Specific IEs.
 *
 * Note: Please design and implement this callback carefully. It will be invoked
 * many times for the same IE in the user supplied IEs list, but the decision
 * to insert the IE somewhere in the frame should be made only once for any
 * given IE otherwise the same IE will be inserted and duplicated many times.
 */
typedef struct {
	wlc_iem_cbparm_t *cbparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;	/* Frame type */
	int16 prev;	/* Previous tag */
	uint8 tag;	/* Current tag */
	bool is_tag;	/* TRUE indicates 'tag' field is IE tag; otherwise prio */
	uint8 *ie;	/* User supplied IE */
	uint ie_len;
} wlc_iem_ins_data_t;
typedef bool (*wlc_iem_ins_cb_t)(void *ctx, wlc_iem_ins_data_t *data);

/* 'modify ie' query callback - invoked to query if the user supplied IE 'ie'
 * needs to be modified by the registered 'calc_len/build' callback before being
 * written to the frame.
 *
 * - 'cbparm' points to the user supplied calc_len/build parameters
 *   structure from the wlc_iem_calc_len()/wlc_iem_build_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie/ie_len' points to the user supplied IE which the callback decides
 *   if it will be modified by a registered callback.
 *
 * It should return TRUE if the 'calc_len/build' callback wants to modify
 * the user supplied IE 'ie'.
 *
 * It is used to merge non Vendor Specific IEs in the user supplied IEs list
 * (with modification) with the driver generated non Vendor Specific IEs.
 */
typedef struct {
	wlc_iem_cbparm_t *cbparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;	/* Frame type */
	uint8 *ie;	/* User supplied IE */
	uint ie_len;
} wlc_iem_mod_data_t;
typedef bool (*wlc_iem_mod_cb_t)(void *ctx, wlc_iem_mod_data_t *data);

/* 'Vendor Specific IE Prio Classification' callback - invoked to return a priority for
 * the user supplied Vendor Specific IE 'ie'.
 *
 * The priority is an integer value ranging from 0 to 249.
 *
 * The IE will be queried for modification via 'modify ie' query callback as is done to
 * non Vendor Specific IEs if the returned priority matches that of any registered
 * Vendor Specific IE's; the IE will be queried for insertion via 'insert ie' query
 * callback as is done to non Vendor Specific IEs if the returned priority isn't
 * among that of the registered Vendor Specific IE's.
 *
 * It is used to merge Vendor Specific IEs in the user supplied IEs list with the driver
 * generated Vendor Specific IEs.
 */
typedef struct {
	wlc_iem_cbparm_t *cbparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;	/* Frame type */
	uint8 *ie;	/* User supplied Vendor Specific IE */
	uint ie_len;
} wlc_iem_cbvsie_data_t;
typedef uint8 (*wlc_iem_cbvsie_cb_t)(void *ctx, wlc_iem_cbvsie_data_t *data);

/* User supplied IEs list and handling information used by IE management.
 *
 * A NULL pointer to this structure or to the 'ies' field of this structure
 * indicates there is no user supplied IEs list information; when the 'ies'
 * field of this structure is non NULL all other information in the structure
 * must be valid.
 *
 * 'ies/ies_len' - user supplied IEs list (tlvs)
 * 'mod_fn' - "modify ie" query callback and invoked for IEs with registered callbacks.
 * 'ins_fn' - "insert ie" query callback and invoked for IEs without registered callbacks.
 * 'vsie_fn' - "Vendor Specific IE Prio Classification" callback and invoked
 *             for Vendor Specific IEs.
 * See wlc_iem_ins_cb_t/wlc_iem_mod_cb_t/wlc_iem_cbvsie_cb_t for details.
 */
typedef struct {
	/* user supplied IEs list */
	uint8 *ies;	/* IEs list */
	uint ies_len;	/* IEs length */
	/* query/classification callbacks */
	wlc_iem_mod_cb_t mod_fn;	/* "Modify IE" query callback */
	wlc_iem_ins_cb_t ins_fn;	/* "Insert IE" query callback */
	wlc_iem_cbvsie_cb_t vsie_fn;	/* 'VS IE Prio" classification callback */
	void *ctx;	/* Callbacks context */
} wlc_iem_uiel_t;

/*
 * 'calc_len' and 'build' callback parameters.
 *
 * Passed from the caller of either wlc_iem_calc_len or wlc_iem_build_frame
 * APIs to the registered 'calc_len' or 'build' callbacks.
 */
/* Common parameters */
struct wlc_iem_cbparm {
	bool ht;	/* include ht components */
	bool vht;	/* include vht components */
	wlc_iem_ft_cbparm_t *ft;	/* frame type specific, may be NULL if n/a */
};

/* ******** 'parse' callback & registration ******** */

/*
 * forward declarations
 */
typedef struct wlc_iem_pparm wlc_iem_pparm_t;
typedef union wlc_iem_ft_pparm wlc_iem_ft_pparm_t;

/*
 * 'parse' callback - invoked from wlc_iem_parse_frame API.
 *
 * It is invoked when an IE whose tag matches the registered tag (IE tag for
 * non Vendor Specific IEs or IE ID for Vendor Specific IEs) when parsing IEs
 * in a frame. It is also invoked with 'ie' field being NULL when the IE isn't
 * found in the frame.
 *
 * - 'pparm' points to the parse callback parameters structure from the
 *   wlc_iem_parse_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie/ie_len' is the IE for which the callback is registered; it is NULL
 *   if the IE is absent from the frame
 * - 'buf/buf_len' is the buffer starting from the first IE in the frame,
 *   callbacks can search the entire buffer for any particular IEs they
 *   may be interested (but such usage is discouraged).
 *
 * Any non BCME_OK return value signals the IE mgmt to stop parsing the IEs in
 * the frame.
 */
typedef struct {
	wlc_iem_pparm_t *pparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;
	uint8 *ie;		/* IE pointer */
	uint ie_len;
	uint8 *buf;		/* IEs in the frame */
	uint buf_len;
} wlc_iem_parse_data_t;
typedef int (*wlc_iem_parse_fn_t)(void *ctx, wlc_iem_parse_data_t *data);

/* ******** 'parse' callback invocation ******** */

/*
 * 'notif' callback - invoked to notify the user that the IE mgmt can't find
 * a callback for the IE.
 *
 * - 'pparm' points to the parse callback parameters structure from the
 *   wlc_iem_parse_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie/ie_len' is the IE for which no callback is registered
 */
typedef struct {
	wlc_iem_pparm_t *pparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;	/* Frame type */
	uint8 *ie;	/* IE pointer */
	uint ie_len;
} wlc_iem_nhdlr_data_t;
typedef void (*wlc_iem_nhdlr_cb_t)(void *ctx, wlc_iem_nhdlr_data_t *data);

/*
 * 'Vendor Specific IE ID classification' callback - invoked to return an ID for
 * the Vendor Specific IE 'ie'.
 *
 * The ID is an integer value ranging from 0 to 249. It was used for users to
 * register a Vendor Specific IE parser callback through wlc_iem_vs_add_parse_fn.
 *
 * - 'pparm' points to the parse callback parameters structure from the
 *   wlc_iem_parse_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie/ie_len' is the Vendor Specific IE.
 *
 * The callback may call wlc_iem_vs_get_id() for Vendor Specific IEs that wlc_iem_vs.c
 * knows, or in case patching ROM function the callback can return an unique ID directly.
 * The recommendation is to modify wlc_iem_vs.c to add any new IDs if possible.
 */
typedef struct {
	wlc_iem_pparm_t *pparm;	/* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;	/* Frame type */
	uint8 *ie;	/* Vendor Specific IE pointer */
	uint ie_len;
} wlc_iem_pvsie_data_t;
typedef uint8 (*wlc_iem_pvsie_cb_t)(void *ctx, wlc_iem_pvsie_data_t *data);

/*
 * User parse parameters and handling information used by IE management.
 *
 * A NULL pointer to this structure indicates to the IE management there's
 * no User parse parameters and handling information.
 *
 * - 'vsie_fn': 'Vendor Specific IE ID classification" callback is invoked to query the
 *   Vendor Specific IEs' IDs used by the Vendor Specific IE parse callback registration.
 * - 'notif': 'Notif' callback is invoked for IEs that don't have any registered callbacks.
 */
typedef struct {
	/* query/classification callbacks */
	wlc_iem_nhdlr_cb_t notif_fn;	/* no handler notification callback */
	wlc_iem_pvsie_cb_t vsie_fn;	/* VS IE ID classification callback */
	void *ctx;	/* Callbacks context */
} wlc_iem_upp_t;

/*
 * 'parse' callback parameters.
 *
 * Passed from the wlc_iem_parse_frame API to the registered 'parse' callbacks.
 *
 * For meta data (wrxh, plcp, hdr, body, rxchan, rspec, etc), users (caller and
 * callbacks) must be in an agreement as to when and where they are available.
 */
/* Common parameters */
struct wlc_iem_pparm {
	wlc_d11rxhdr_t *wrxh;	/* d11 header */
	uint8 *plcp;	/* phy header */
	uint8 *hdr;	/* dot11 header */
	uint8 *body;	/* fixed frame header */
	uint8 rxchan;	/* Receive channel, calculated from wrxh */
	ratespec_t rspec;	/* Receive rate, calculated from wrxh and plcp */
	bool ht;	/* HT enabled */
	bool vht;	/* VHT enabled */
	wlc_iem_ft_pparm_t *ft;	/* may be NULL if n/a */
};

#endif /* _wlc_ie_mgmt_types_h_ */
