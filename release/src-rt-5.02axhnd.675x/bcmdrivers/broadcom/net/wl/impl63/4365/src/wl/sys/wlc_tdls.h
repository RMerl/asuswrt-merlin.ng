/*
 * TDLS(Tunnel Direct Link Setup) related header file
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
 * $Id: wlc_tdls.h 708017 2017-06-29 14:11:45Z $
*/

#ifndef _wlc_tdls_h_
#define _wlc_tdls_h_

#define TDLS_PAYLOAD_TYPE		2
#define TDLS_PAYLOAD_TYPE_LEN	1

/* TDLS Action Category code */
#define TDLS_ACTION_CATEGORY_CODE		12
/* Wi-Fi Display (WFD) Vendor Specific Category */
/* used for WFD Tunneled Probe Request and Response */
#define TDLS_VENDOR_SPECIFIC				127

/* Default Listen Interval for Peer UAPSD SLEEP STA */
#define TDLS_DEFAULT_PEER_LISTEN_INTERVAL		10

/* Default FW credit available for Peer UAPSD SLEEP STA */
#define TDLS_WLFC_DEFAULT_FWQ_DEPTH			6

/* IE Mgnt defines */
#define IEM_TDLS_SRQ_BUILD_CB_MAX		16
#define IEM_TDLS_SRQ_PARSE_CB_MAX		9
#define IEM_TDLS_SRS_BUILD_CB_MAX		17
#define IEM_TDLS_SRS_PARSE_CB_MAX		10
#define IEM_TDLS_SCF_BUILD_CB_MAX		9
#define IEM_TDLS_SCF_PARSE_CB_MAX		9
#define IEM_TDLS_DRS_BUILD_CB_MAX		9
#define IEM_TDLS_DRS_PARSE_CB_MAX		4

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* 802.11z TDLS Public Action Frame */
BWL_PRE_PACKED_STRUCT struct tdls_pub_act_frame {
	uint8	category;	/* DOT11_ACTION_CAT_PUBLIC */
	uint8	action;		/* TDLS_DISCOVERY_RESP */
	uint8	dialog_token;
	uint16	cap;		/* TDLS capabilities */
	uint8	elts[1];	/* Variable length information elements.  Max size =
				* ACTION_FRAME_SIZE - sizeof(this structure) - 1
				*/
} BWL_POST_PACKED_STRUCT;
typedef struct tdls_pub_act_frame tdls_pub_act_frame_t;
#define TDLS_PUB_AF_FIXED_LEN	4
/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#ifdef WLTDLS
#define TDLS_PRHEX(m, b, n)	do {if (WL_TDLS_ON()) prhex(m, b, n);} while (0)

extern tdls_info_t *wlc_tdls_attach(wlc_info_t *wlc);
extern void wlc_tdls_detach(tdls_info_t *tdls);
extern bool wlc_tdls_cap(tdls_info_t *tdls);

extern bool wlc_tdls_buffer_sta_enable(tdls_info_t *tdls);
extern bool wlc_tdls_sleep_sta_enable(tdls_info_t *tdls);
extern void wlc_tdls_update_tid_seq(tdls_info_t *tdls, struct scb *scb, uint8 tid, uint16 seq);
extern void wlc_tdls_return_to_base_ch_on_eosp(tdls_info_t *tdls, struct scb *scb);
extern void wlc_tdls_rcv_data_frame(tdls_info_t *tdls, struct scb *scb, d11rxhdr_t *rxhdr);
extern void wlc_tdls_rcv_action_frame(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint pdata_offset);
extern bool wlc_tdls_recvfilter(tdls_info_t *tdls, struct scb *scb);
extern void wlc_tdls_process_discovery_resp(tdls_info_t *tdls, struct dot11_management_header *hdr,
	uint8 *body, int body_len, int8 rssi);
extern int wlc_tdls_set(tdls_info_t *tdls, bool on);
extern void wlc_tdls_cleanup(tdls_info_t *tdls, wlc_bsscfg_t *parent);
extern void wlc_tdls_free_scb(tdls_info_t *tdls, struct scb *scb);
extern struct scb *wlc_tdls_query(tdls_info_t *tdls, wlc_bsscfg_t *parent, void *p,
	struct ether_addr *ea);
extern void wlc_tdls_port_open(tdls_info_t *tdls, struct ether_addr *ea);
extern wlc_bsscfg_t *wlc_tdls_get_parent_bsscfg(wlc_info_t *wlc, struct scb *scb);
extern void wlc_tdls_update_pm(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg, uint txstatus);
extern void wlc_tdls_notify_pm_state(tdls_info_t *tdls, wlc_bsscfg_t *parent, bool state);
extern void wlc_tdls_send_pti(tdls_info_t *tdls, struct scb *scb);
extern int wlc_tdls_down(void *hdl);
extern bool wlc_tdls_in_pti_interval(tdls_info_t *tdls, struct scb *scb);
extern void wlc_tdls_apsd_usp_end(tdls_info_t *tdls, struct scb *scb);
extern uint wlc_tdls_apsd_usp_interval(tdls_info_t *tdls, struct scb *scb);
extern bool wlc_tdls_stay_awake(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_tdls_do_chsw(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg, bool off_ch);
extern bool wlc_tdls_is_chsw_enabled(tdls_info_t *tdls);
extern uint16 wlc_tdls_get_pretbtt_time(tdls_info_t *tdls);
extern bool wlc_tdls_chk_switch_allowed(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg);
extern bool wlc_tdls_is_active(wlc_info_t *wlc);
extern bool wlc_tdls_quiet_down(tdls_info_t *tdls);
extern bool wlc_tdls_cert_test_enabled(wlc_info_t *wlc);
extern struct scb * wlc_tdls_scbfind_all_ex(wlc_info_t *wlc, const struct ether_addr *ea);
#else	/* stubs */
#define TDLS_PRHEX(m, b, n)	do {} while (0)

#define wlc_tdls_attach(a) (dpt_info_t *)0x0dadbeef
#define	wlc_tdls_detach(a) do {} while (0)
#define	wlc_tdls_cap(a) FALSE
#define wlc_tdls_buffer_sta_enable(a)	FALSE
#define wlc_tdls_sleep_sta_enable(a)	FALSE
#define wlc_tdls_update_tid_seq(a, b, c, d) do {} while (0)
#define wlc_tdls_return_to_base_ch_on_eosp(a, b) do {} while (0)
#define wlc_tdls_rcv_data_frame(a, b, c) do {} while (0)
#define wlc_tdls_recvfilter(a, b) FALSE
#define	wlc_tdls_rcv_action_frame(a, b, c, d) do {} while (0)
#define wlc_tdls_process_discovery_resp(a, b, c, d, e) do {} while (0)
#define wlc_tdls_set(a, b) do {} while (0)
#define wlc_tdls_cleanup(a, b) do {} while (0)
#define wlc_tdls_free_scb(a, b) do {} while (0)
#define wlc_tdls_query(a, b, c, d) NULL
#define wlc_tdls_port_open(a, b) NULL
#define wlc_tdls_get_parent_bsscfg(a, b) NULL
#define wlc_tdls_update_pm(a, b, c) do {} while (0)
#define wlc_tdls_notify_pm_state(a, b, c) do {} while (0)
#define wlc_tdls_on(a) do {} while (0)
#define wlc_tdls_send_pti(a, b) do {} while (0)
#define wlc_tdls_down(a) do {} while (0)
#define wlc_tdls_in_pti_interval(a, b) FALSE
#define wlc_tdls_apsd_usp_end(a, b) do {} while (0)
#define wlc_tdls_apsd_usp_interval(a, b) 0
#define wlc_tdls_stay_awake(a, b) FALSE
#define wlc_tdls_do_chsw(a, b, c) do {} while (0)
#define wlc_tdls_get_pretbtt_time(a) 0
#define wlc_tdls_quiet_down(a) FALSE
#define wlc_tdls_cert_test_enabled(a) FALSE
#endif /* WLTDLS */

#endif /* _wlc_tdls_h_ */
