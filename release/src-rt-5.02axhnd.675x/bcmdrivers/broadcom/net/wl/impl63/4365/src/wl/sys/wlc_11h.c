/*
 * 802.11h module source file (top level and spectrum management, radar avoidance)
 * Broadcom 802.11abgn Networking Device Driver
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
 * $Id: wlc_11h.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * The basics of 802.11H are a set of new IEs and Management Frames to allow for
 * "Spectrum Management" and "Dynamic Frequency Selection". Much of the stuff deals with handling
 * radar conflicts in the 5GHz band, and "local" (AP-based) control of transmit power.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlDriver11DH]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WL11H

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_11h.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_reg.h>
#include <wlc_pcb.h>
#include <wlc_dfs.h>

/* IOVar table */
enum {
	IOV_MEASURE,
	IOV_LAST
};

static const bcm_iovar_t wlc_11h_iovars[] = {
	{"measure", IOV_MEASURE, (0), IOVT_BUFFER, sizeof(uint32)+sizeof(struct ether_addr)},
	{NULL, 0, 0, 0, 0}
};

/* ioctl table */
static const wlc_ioctl_cmd_t wlc_11h_ioctls[] = {
	{WLC_SET_SPECT_MANAGMENT, WLC_IOCF_DRIVER_DOWN, sizeof(int)},
	{WLC_GET_SPECT_MANAGMENT, 0, sizeof(int)},
	{WLC_MEASURE_REQUEST, 0, sizeof(uint32)+sizeof(struct ether_addr)}
};

/* 11h module info */
struct wlc_11h_info {
	wlc_info_t *wlc;
	int cfgh;			/* bsscfg cubby handle */
	uint _spect_management;
};

/* local functions */
/* module */
static int wlc_11h_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
#ifdef BCMDBG
static int wlc_11h_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
static int wlc_11h_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

/* cubby */
static int wlc_11h_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_11h_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
#ifdef BCMDBG
static void wlc_11h_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_11h_bsscfg_dump NULL
#endif // endif

/* spectrum management */
static void wlc_recv_measure_request(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
static void wlc_recv_measure_report(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
static void wlc_send_measure_request(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 measure_type);
static void wlc_send_measure_report(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 token, uint8 *report, uint report_len);
static void _wlc_11h_build_basic_report_radar(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	void (*func_ptr)(wlc_info_t *, uint, void *));
#ifdef BCMDBG
static void wlc_print_measure_req_rep(wlc_info_t *wlc, struct dot11_management_header *hdr,
	uint8 *body, int body_len);
#endif // endif

/* IE mgmt */
#ifdef STA
static uint wlc_11h_calc_pwr_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_11h_write_pwr_cap_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_11h_calc_sup_chan_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_11h_write_sup_chan_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#ifdef WLTDLS
static uint wlc_11h_tdls_calc_sup_chan_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_11h_tdls_write_sup_chan_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_11h_disc_calc_sup_chan_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_11h_disc_write_sup_chan_ie(void *ctx, wlc_iem_build_data_t *build);
#endif // endif

/* XXX allocate the struct and reserve a pointer to the struct in the bsscfg
 * as the bsscfg cubby when this structure grows larger than a pointer...
 */
typedef struct {
	uint8 spect_state;
} wlc_11h_t;
#define IEEE11H_BSSCFG_CUBBY(m11h, cfg) ((wlc_11h_t *)BSSCFG_CUBBY(cfg, (m11h)->cfgh))

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_11h_info_t *
BCMATTACHFN(wlc_11h_attach)(wlc_info_t *wlc)
{
	wlc_11h_info_t *m11h;
#ifdef STA
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
#endif // endif

	if ((m11h = MALLOCZ(wlc->osh, sizeof(wlc_11h_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	m11h->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((m11h->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(wlc_11h_t),
	                wlc_11h_bsscfg_init, wlc_11h_bsscfg_deinit, wlc_11h_bsscfg_dump,
	                m11h)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register IE mgmt calc/build callbacks */
	/* calc/build */
#ifdef STA
	/* assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_PWR_CAP_ID,
	      wlc_11h_calc_pwr_cap_ie_len, wlc_11h_write_pwr_cap_ie, m11h) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, pwr cap in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_SUPP_CHANNELS_ID,
	      wlc_11h_calc_sup_chan_ie_len, wlc_11h_write_sup_chan_ie, m11h) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, sup chan in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */
#ifdef WLTDLS
	/* setupreq */
	if (TDLS_SUPPORT(wlc->pub)) {
		if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_SUPP_CHANNELS_ID,
			wlc_11h_tdls_calc_sup_chan_ie_len, wlc_11h_tdls_write_sup_chan_ie, m11h)
				!= BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, sup chan in setupreq\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* setupresp */
		if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_SUPP_CHANNELS_ID,
			wlc_11h_tdls_calc_sup_chan_ie_len, wlc_11h_tdls_write_sup_chan_ie, m11h)
				!= BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, sup chan in setupresp\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* discresp */
		if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_SUPP_CHANNELS_ID,
			wlc_11h_disc_calc_sup_chan_ie_len, wlc_11h_disc_write_sup_chan_ie, m11h)
				!= BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, sup chan in discresp\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
#endif /* WLTDLS */

#ifdef BCMDBG
	if (wlc_dump_register(wlc->pub, "11h", wlc_11h_dump, m11h) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	if (wlc_module_register(wlc->pub, wlc_11h_iovars, "11h", m11h, wlc_11h_doiovar,
	                        NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	if (wlc_module_add_ioctl_fn(wlc->pub, m11h, wlc_11h_doioctl,
	                            ARRAYSIZE(wlc_11h_ioctls), wlc_11h_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	m11h->_spect_management = SPECT_MNGMT_LOOSE_11H;	/* 802.11h */
	wlc->pub->_11h = TRUE;

	return m11h;

	/* error handling */
fail:
	wlc_11h_detach(m11h);
	return NULL;
}

void
BCMATTACHFN(wlc_11h_detach)(wlc_11h_info_t *m11h)
{
	wlc_info_t *wlc;

	if (m11h == NULL)
		return;

	wlc = m11h->wlc;

	wlc_module_remove_ioctl_fn(wlc->pub, m11h);
	wlc_module_unregister(wlc->pub, "11h", m11h);

	MFREE(wlc->osh, m11h, sizeof(wlc_11h_info_t));
}

static int
wlc_11h_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_info_t *wlc = m11h->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	BCM_REFERENCE(ret_int_ptr);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_SVAL(IOV_MEASURE): {
		struct ether_addr *ea = (struct ether_addr *)((uint32 *)arg + 1);
		switch (int_val) {
		case WLC_MEASURE_TPC:
			wlc_send_tpc_request(wlc->tpc, bsscfg, ea);
			break;

		case WLC_MEASURE_CHANNEL_BASIC:
			wlc_send_measure_request(wlc, bsscfg, ea, DOT11_MEASURE_TYPE_BASIC);
			break;

		case WLC_MEASURE_CHANNEL_CCA:
			wlc_send_measure_request(wlc, bsscfg, ea, DOT11_MEASURE_TYPE_CCA);
			break;

		case WLC_MEASURE_CHANNEL_RPI:
			wlc_send_measure_request(wlc, bsscfg, ea, DOT11_MEASURE_TYPE_RPI);
			break;

		default:
			err = BCME_RANGE; /* unknown measurement type */
			break;
		}
		break;
	}

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wlc_11h_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_info_t *wlc = m11h->wlc;
	int val = 0, *pval;
	int err = BCME_OK;

	/* default argument is generic integer */
	pval = (int *)arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));

	switch (cmd) {
	case WLC_SET_SPECT_MANAGMENT:
		err = wlc_11h_set_spect(wlc->m11h, (uint)val);
		break;

	case WLC_GET_SPECT_MANAGMENT: {
		uint spect = SPECT_MNGMT_OFF;
		spect = wlc_11h_get_spect(wlc->m11h);
		if (spect == SPECT_MNGMT_OFF &&
		    WL11D_ENAB(wlc))
			spect = SPECT_MNGMT_STRICT_11D;
		*pval = (int)spect;
		break;
	}

	case WLC_MEASURE_REQUEST:
		err = wlc_iovar_op(wlc, "measure", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#ifdef BCMDBG
static int
wlc_11h_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;

	bcm_bprintf(b, "spect_mngmt:%d\n", m11h->_spect_management);

	return BCME_OK;
}
#endif /* BCMDBG */

/* bsscfg cubby */
static int
wlc_11h_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	return BCME_OK;
}

static void
wlc_11h_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
}

#ifdef BCMDBG
static void
wlc_11h_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_11h_t *p11h = IEEE11H_BSSCFG_CUBBY(m11h, cfg);

	ASSERT(p11h != NULL);

	bcm_bprintf(b, "\tspect_state: %x\n", p11h->spect_state);
}
#endif // endif

void
wlc_11h_tbtt(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = m11h->wlc;
	wlc_11h_t *p11h = IEEE11H_BSSCFG_CUBBY(m11h, cfg);

	ASSERT(p11h != NULL);

	/* If we have previously detected radar and scheduled a channel switch in
	 * the future, then every TBTT interval we come here and decrement the
	 * 'TBTT time to chan switch' field in the outgoing beacons.
	 */
	if (BSSCFG_AP(cfg) &&
	    (p11h->spect_state & NEED_TO_UPDATE_BCN)) {
		/* If all counts are 0, no further need to update outgoing beacons */
		if (wlc_quiet_get_quiet_count(wlc->quiet, cfg) == 0 &&
		    wlc_csa_get_csa_count(wlc->csa, cfg) == 0) {
			p11h->spect_state &= ~NEED_TO_UPDATE_BCN;
		}

		/* Count down for switch channels... */
		wlc_csa_count_down(wlc->csa, cfg);
		wlc_quiet_count_down(wlc->quiet, cfg);

		WL_APSTA_BCN(("wl%d: wlc_tbtt 11h -> wlc_update_beacon()\n", wlc->pub->unit));
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	}
}

#if defined(STA) || defined(WLTDLS)
/* Supported Channels IE */
static uint8 *
_wlc_11h_write_sup_chan_ie(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg, uint8 *cp, int buflen)
{
	wlc_info_t *wlc = m11h->wlc;
	uint8 run_count = 0, len = 0;
	bcm_tlv_t *sup_channel_ie = (bcm_tlv_t*)cp;
	uint8 cur_chan, first_chan = 0, seen_valid = 0, max_channel, ch_sep;
	bool valid_channel;
	uint subband_idx = 0, end_subband, j;

	/* 1-14, 34-46, 36-48, 52-64, 100-144, 149-161, 165, 184-196 */
	const struct {
		uint8 start;
		uint8 cnt;
	} subbands[] = {
		{1, 14}, {34, 4}, {36, 4}, {52, 4},
		{100, 12}, {149, 4}, {165, 1}, {184, 4}
	};

	sup_channel_ie->id = DOT11_MNG_SUPP_CHANNELS_ID;
	cp += 2; /* Skip over ID and Len */

	if (wlc->band->bandtype == WLC_BAND_2G) {
		subband_idx = 0;
		end_subband = 1;
		max_channel = CH_MAX_2G_CHANNEL + 1;
		ch_sep = CH_5MHZ_APART;
	} else {
		end_subband = ARRAYSIZE(subbands);
		max_channel = MAXCHANNEL;
		ch_sep = CH_20MHZ_APART;

		/* Handle special case for JP where subband could be 34-46 or 36-48 */
		/* Arbitrarily decided to give 36-48 priority as 34-46 is legacy passive anyway */
		if (VALID_CHANNEL20_IN_BAND(wlc, BAND_5G_INDEX, 34) &&
		    !VALID_CHANNEL20_IN_BAND(wlc, BAND_5G_INDEX, 36))
			subband_idx = 1;
		else
			subband_idx = 2;
	}

	for (; subband_idx < end_subband; subband_idx++) {
		run_count = 0;
		seen_valid = 0;

		for (cur_chan = subbands[subband_idx].start, j = 0;
		     (j < subbands[subband_idx].cnt) && (cur_chan < max_channel);
		     j++, cur_chan += ch_sep) {

			valid_channel = VALID_CHANNEL20_IN_BAND(wlc,
			                                        wlc->band->bandunit, cur_chan);
			if (valid_channel) {
				if (!seen_valid) {
					first_chan = cur_chan;
					seen_valid = 1;
				}
				run_count++;
			}
		}

		if (seen_valid) {
			*cp++ = first_chan;
			*cp++ = run_count;
			len += 2;
			run_count = 0;
			seen_valid = 0;
		}

		/* If subband 34-46 was present then skip over 36-48 */
		if (subband_idx == 1)
			subband_idx = 2;
	}

	sup_channel_ie->len = len;
	return cp;
}
#endif /* STA || WLTDLS */

#ifdef STA
/* Supported Channels IE */
static uint
wlc_11h_calc_sup_chan_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_info_t *wlc = m11h->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	uint8 buf[257];

	/* TODO: needs a better way to calculate the IE length */

	if (BSS_WL11H_ENAB(wlc, cfg))
		return (uint)(_wlc_11h_write_sup_chan_ie(m11h, cfg, buf, sizeof(buf)) - buf);

	return 0;
}

static int
wlc_11h_write_sup_chan_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_info_t *wlc = m11h->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_WL11H_ENAB(wlc, cfg))
		_wlc_11h_write_sup_chan_ie(m11h, cfg, data->buf, data->buf_len);

	return BCME_OK;
}

/* Power Cap IE */
static uint
wlc_11h_calc_pwr_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_info_t *wlc = m11h->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_WL11H_ENAB(wlc, cfg))
		return TLV_HDR_LEN + sizeof(dot11_power_cap_t);

	return 0;
}

static int
wlc_11h_write_pwr_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_info_t *wlc = m11h->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;
	dot11_power_cap_t pwr_cap;
	uint8 min_pwr = 0, max_pwr = 0;
	ppr_t *srommax;
	uint8 txpwr_backoff = 0;

	if (!BSS_WL11H_ENAB(wlc, cfg))
		return BCME_OK;

	if ((srommax = ppr_create(wlc->osh, WL_TX_BW_20)) != NULL) {
		wlc_phy_txpower_sromlimit(WLC_PI(wlc), bi->chanspec, &min_pwr, srommax, 0);
		max_pwr = ppr_get_max(srommax);
		ppr_delete(wlc->osh, srommax);
	}

	/* Fix for Voice Enterprise power constraint test failing as TxPwr in TPC
	 * report is below TxPwrMin advertised in association request when target
	 * power is dropped to the minimum.
	 * For ACPHY, advertise tssivisible as TxPwrMin which can be different from
	 * SROM min limit and this is now updated in pi->min_txpower itself.
	 */

	 /* Increase the TxPwrMin advertised by values below:
	 * - backoff: To account for further reduction in target power by 1.5 dbm backoff.
	 * - WLC_MINTXPWR_OFFSET: All rates get disabled if target power dropped
	 *   to the srom minimum. Add a small offset to the TxPwrMin to always maintain it
	 *   slightly above the srom minimum. This also takes care of round off.
	 */
	txpwr_backoff = wlc_phy_get_txpwr_backoff(WLC_PI(wlc));

	/* Include antenna gain in order to use EIRP values only for EIRP locales */
	if (wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP) {
		pwr_cap.min = (int8)(min_pwr + txpwr_backoff +
			WLC_MINTXPWR_OFFSET + wlc->band->antgain) /
			WLC_TXPWR_DB_FACTOR; /* convert qdBm to dBm */
		pwr_cap.max = (int8)(max_pwr + wlc->band->antgain) /
			WLC_TXPWR_DB_FACTOR; /* convert qdBm to dBm */
	} else {
		pwr_cap.min = (int8)(min_pwr + txpwr_backoff +
			WLC_MINTXPWR_OFFSET) / WLC_TXPWR_DB_FACTOR;
		pwr_cap.max = (int8)(max_pwr) /	WLC_TXPWR_DB_FACTOR;
	}

	bcm_write_tlv(DOT11_MNG_PWR_CAP_ID, &pwr_cap, sizeof(pwr_cap), data->buf);

	return BCME_OK;
}
#endif /* STA */

#ifdef WLTDLS
/* Supported Channels IE in Setup frames */
static uint
wlc_11h_tdls_calc_sup_chan_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;
	uint8 buf[257];

	if (!isset(ftcbparm->tdls.cap, DOT11_TDLS_CAP_CH_SW))
		return 0;

	/* TODO: needs a better way to calculate the IE length */

	return (uint)(_wlc_11h_write_sup_chan_ie(m11h, calc->cfg, buf, sizeof(buf)) - buf);
}

static int
wlc_11h_tdls_write_sup_chan_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	if (!isset(ftcbparm->tdls.cap, DOT11_TDLS_CAP_CH_SW))
		return BCME_OK;

	_wlc_11h_write_sup_chan_ie(m11h, build->cfg, build->buf, build->buf_len);

	return BCME_OK;
}

/* Supported Channels IE in Discovery frames */
static uint
wlc_11h_disc_calc_sup_chan_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;
	uint8 buf[257];

	if (!isset(ftcbparm->disc.cap, DOT11_TDLS_CAP_CH_SW))
		return 0;

	/* TODO: needs a better way to calculate the IE length */

	return (uint)(_wlc_11h_write_sup_chan_ie(m11h, calc->cfg, buf, sizeof(buf)) - buf);
}

static int
wlc_11h_disc_write_sup_chan_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_11h_info_t *m11h = (wlc_11h_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	if (!isset(ftcbparm->disc.cap, DOT11_TDLS_CAP_CH_SW))
		return BCME_OK;

	_wlc_11h_write_sup_chan_ie(m11h, build->cfg, build->buf, build->buf_len);

	return BCME_OK;
}
#endif /* WLTDLS */

/*
 * Frame received, frame type FC_ACTION,
 *  action_category DOT11_ACTION_CAT_SPECT_MNG
 */
void
wlc_recv_frameaction_specmgmt(wlc_11h_info_t *m11h, struct dot11_management_header *hdr,
	uint8 *body, int body_len, int8 rssi, ratespec_t rspec)
{
	wlc_info_t *wlc = m11h->wlc;
	wlc_bsscfg_t *cfg;

	ASSERT(WL11H_ENAB(wlc));
	ASSERT(body_len >= DOT11_ACTION_HDR_LEN);
	ASSERT(body[DOT11_ACTION_CAT_OFF] == DOT11_ACTION_CAT_SPECT_MNG);

	if ((cfg = wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid)) == NULL) {
#if defined(BCMDBG) || defined(WLMSG_INFORM)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

		WL_INFORM(("wl%d: %s: ignoring request from %s since we are not in a BSS or IBSS\n",
		           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));
		return;
	}

	/* Spectrum Management action_id's */
	switch (body[DOT11_ACTION_ACT_OFF]) {
	case DOT11_SM_ACTION_M_REQ:
		if (wlc_validate_measure_req(m11h, cfg, hdr) == FALSE)
			return;
		wlc_recv_measure_request(wlc, cfg, hdr, body, body_len);
		break;
	case DOT11_SM_ACTION_M_REP:
		wlc_recv_measure_report(wlc, cfg, hdr, body, body_len);
		break;
	case DOT11_SM_ACTION_TPC_REQ:
		wlc_recv_tpc_request(wlc->tpc, cfg, hdr, body, body_len, rssi, rspec);
		break;
	case DOT11_SM_ACTION_TPC_REP:
		wlc_recv_tpc_report(wlc->tpc, cfg, hdr, body, body_len, rssi, rspec);
		break;
	case DOT11_SM_ACTION_CHANNEL_SWITCH:
		wlc_recv_csa_action(wlc->csa, cfg, hdr, body, body_len);
		break;
	default:
		wlc_send_action_err(wlc, hdr, body, body_len);
		break;
	}
}

/* Validate the source of a measurement request */
bool
wlc_validate_measure_req(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr)
{
	wlc_info_t *wlc = m11h->wlc;
	struct scb *scb;
	char eabuf[ETHER_ADDR_STR_LEN];
	char eabuf1[ETHER_ADDR_STR_LEN];

	BCM_REFERENCE(eabuf);
	BCM_REFERENCE(eabuf1);

	ASSERT(cfg != NULL);

	/* is this a request from someone we should care about? */
	if (!cfg->associated) {
		WL_INFORM(("wl%d: %s: ignoring request from %s since we are not in a BSS or IBSS\n",
		           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));
		return FALSE;
	} else if (BSSCFG_AP(cfg)) {
		if (ETHER_ISMULTI(&hdr->sa) ||
		    (scb = wlc_scbfind(wlc, cfg, &hdr->sa)) == NULL ||
		    !SCB_ASSOCIATED(scb)) {
			/* AP only accepts reqs from associated STAs */
			WL_INFORM(("wl%d: %s: ignoring request from unassociated STA %s\n",
			           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));
			return FALSE;
		} else if (ETHER_ISMULTI(&hdr->da)) {
			/* AP only accepts unicast reqs */
			WL_INFORM(("wl%d: %s: ignoring bc/mct request %s from associated STA %s\n",
			           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->da, eabuf),
			           bcm_ether_ntoa(&hdr->sa, eabuf1)));
			return FALSE;
		}
		/* XXX APSTA: could confirm here that BSSIDs match, but this is good enough already?
		 */
	} else if (cfg->BSS) {
		if (bcmp(hdr->sa.octet, cfg->BSSID.octet, ETHER_ADDR_LEN)) {
			/* STAs should only get requests from the AP */
			WL_INFORM(("wl%d: %s: ignoring request from %s since it is not our AP\n",
			           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));
			return FALSE;
		}
	} else {
		/* IBSS STAs should only get requests from other IBSS members */
		WL_INFORM(("wl%d: %s: ignoring request from %s since it is in a foreign IBSS %s\n",
		           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf),
		           bcm_ether_ntoa(&hdr->bssid, eabuf1)));
			return FALSE;
	}

	return TRUE;
}

void wlc_11h_send_basic_report_radar(wlc_info_t *wlc,
	wlc_bsscfg_t *cfg, void (*func_ptr)(wlc_info_t *, uint, void *))
{
	_wlc_11h_build_basic_report_radar(wlc, cfg, func_ptr);
}

static void _wlc_11h_build_basic_report_radar(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	void (*func_ptr)(wlc_info_t *, uint, void *))
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_action_measure * action_hdr;
	int report_len;
	uint8 *report;
	dot11_meas_rep_t* report_ie;
	struct ether_addr *da;
	uint32 rand_tsf = 0;

	ASSERT(cfg != NULL);

	/* calculate the length of the report */
	report_len = 0;

	/* Basic report with Unmeasured set */
	report_len += TLV_HDR_LEN + DOT11_MNG_IE_MREP_FIXED_LEN +
		DOT11_MEASURE_BASIC_REP_LEN;

	/* allocate space and create the basic report */
	report = (uint8*)MALLOC(wlc->osh, report_len);
	if (report == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return;
	}

	report_ie = (dot11_meas_rep_t*)report;

	/* Basic report with Unmeasured set */
	report_ie->id = DOT11_MNG_MEASURE_REPORT_ID;
	report_ie->len = DOT11_MNG_IE_MREP_FIXED_LEN +
		DOT11_MEASURE_BASIC_REP_LEN;
	bzero((uint8*)&report_ie->token, report_ie->len);
	report_ie->token = 0;
	report_ie->mode = 0;
	report_ie->type = DOT11_MEASURE_TYPE_BASIC;
	report_ie->rep.basic.channel = CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC);
	rand_tsf = wlc->clk ? R_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_random) : 0;
	memcpy(report_ie->rep.basic.start_time, (uint32 *)&rand_tsf, 8);
	bzero((uint8*)&report_ie->rep.basic.duration, 2);
	report_ie->rep.basic.map = DOT11_MEASURE_BASIC_MAP_RADAR;
	report_ie = (dot11_meas_rep_t*)((int8*)report_ie + TLV_HDR_LEN +
		report_ie->len);

	ASSERT(((uint8*)report_ie - report) == (int)report_len);

	body_len = DOT11_ACTION_MEASURE_LEN + report_len;
	da = &cfg->BSSID;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &cfg->cur_etheraddr, &cfg->BSSID,
		body_len, &pbody);
	if (p == NULL) {
		WL_INFORM(("wl%d: %s: no memory for Measure Report\n",
		wlc->pub->unit, __FUNCTION__));
		MFREE(wlc->osh, report, report_len);
		return;
	}

	action_hdr = (struct dot11_action_measure *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = DOT11_SM_ACTION_M_REP;
	action_hdr->token = 0;

	memcpy(action_hdr->data, report, report_len);

	wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL);
	MFREE(wlc->osh, report, report_len);
	wlc_pcb_fn_register(wlc->pcb, func_ptr, (void*)wlc->dfs, p);
}
static void
wlc_recv_measure_request(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	struct dot11_action_measure * action_hdr;
	int len;
	int ie_tot_len;
	int report_len;
	uint8 *report;
	dot11_meas_req_t* ie;
	dot11_meas_rep_t* report_ie;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

#ifdef BCMDBG
	if (WL_INFORM_ON())
		wlc_print_measure_req_rep(wlc, hdr, body, body_len);
#endif /* BCMDBG */

	ASSERT(cfg != NULL);

	if (body_len < 3) {
		WL_ERROR(("wl%d: %s: got Measure Request from %s, "
			  "but frame body len was %d, expected > 3\n",
			  wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf), body_len));
		return;
	}

	action_hdr = (struct dot11_action_measure *)body;

	/* calculate the length of the report */
	report_len = 0;
	ie = (dot11_meas_req_t*)action_hdr->data;
	len = body_len - DOT11_ACTION_MEASURE_LEN;

	/* for each measurement request, calc the length of the report in the response */
	while (len > 2) {
		ie_tot_len = TLV_HDR_LEN + ie->len;

		if (ie->id != DOT11_MNG_MEASURE_REQUEST_ID ||
		    ie->len < DOT11_MNG_IE_MREQ_LEN ||
		    (ie->len >= 3 && (ie->mode & DOT11_MEASURE_MODE_ENABLE))) {
			/* ignore non-measure ie, short, or Mode == ENABLED requests */
		} else if (ie->type == DOT11_MEASURE_TYPE_BASIC) {
			/* Basic report with Unmeasured set */
			report_len += TLV_HDR_LEN + DOT11_MNG_IE_MREP_FIXED_LEN +
				DOT11_MEASURE_BASIC_REP_LEN;
		} else {
			/* CCA, RPI, or other req: Measure report with Incapable */
			report_len += TLV_HDR_LEN + DOT11_MNG_IE_MREP_FIXED_LEN;
		}

		ie = (dot11_meas_req_t*)((int8*)ie + ie_tot_len);
		len -= ie_tot_len;
	}

	/* if nothing to send inform and return */
	if (report_len == 0) {
		WL_INFORM(("wl%d: %s: got unexpected IE or Measure Request mode Enable"
			"or short Measure Request IE len - nothing to send:ignoring\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* allocate space and create the report */
	report = (uint8*)MALLOC(wlc->osh, report_len);
	if (report == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return;
	}

	report_ie = (dot11_meas_rep_t*)report;
	ie = (dot11_meas_req_t*)action_hdr->data;
	len = body_len - DOT11_ACTION_MEASURE_LEN;

	/* for each measurement request, generate the report for the response */
	while (len > 2) {
		ie_tot_len = TLV_HDR_LEN + ie->len;
		if (ie->id != DOT11_MNG_MEASURE_REQUEST_ID) {
			WL_INFORM(("wl%d: %s: got unexpected IE (id %d len"
				" %d), ignoring\n",
				wlc->pub->unit, __FUNCTION__, ie->id, ie->len));
		} else if (ie->len >= 3 && (ie->mode & DOT11_MEASURE_MODE_ENABLE)) {
			WL_INFORM(("wl%d: %s: got Measure Request mode Enable"
				" bit, ignoring\n",
				wlc->pub->unit, __FUNCTION__));
		} else if (ie->len < DOT11_MNG_IE_MREQ_LEN) {
			WL_ERROR(("wl%d: %s: got short Measure Request IE len"
				" %d, ignoring\n",
				wlc->pub->unit, __FUNCTION__, ie->len));
		} else {
			if (ie->type == DOT11_MEASURE_TYPE_BASIC) {
				/* Basic report with Unmeasured set */
				report_ie->id = DOT11_MNG_MEASURE_REPORT_ID;
				report_ie->len = DOT11_MNG_IE_MREP_FIXED_LEN +
					DOT11_MEASURE_BASIC_REP_LEN;
				bzero((uint8*)&report_ie->token, report_ie->len);
				report_ie->token = ie->token;
				report_ie->mode = 0;
				report_ie->type = ie->type;
				report_ie->rep.basic.channel = ie->channel;
				bcopy(ie->start_time, report_ie->rep.basic.start_time, 8);
				bcopy(&ie->duration, &report_ie->rep.basic.duration, 2);
				report_ie->rep.basic.map = DOT11_MEASURE_BASIC_MAP_UNMEAS;
			} else {
				/* CCA, RPI, or other req: Measure report with Incapable */
				report_ie->id = DOT11_MNG_MEASURE_REPORT_ID;
				report_ie->len = DOT11_MNG_IE_MREP_FIXED_LEN;
				bzero((uint8*)&report_ie->token, report_ie->len);
				report_ie->token = ie->token;
				report_ie->mode = DOT11_MEASURE_MODE_INCAPABLE;
				report_ie->type = ie->type;
			}
			report_ie = (dot11_meas_rep_t*)((int8*)report_ie + TLV_HDR_LEN +
				report_ie->len);
		}

		ie = (dot11_meas_req_t*)((int8*)ie + ie_tot_len);
		len -= ie_tot_len;
	}

	ASSERT(((uint8*)report_ie - report) == (int)report_len);

	wlc_send_measure_report(wlc, cfg, &hdr->sa, action_hdr->token, report, report_len);
	MFREE(wlc->osh, report, report_len);
}

static void
wlc_recv_measure_report(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
#ifdef BCMDBG
	if (WL_INFORM_ON())
		wlc_print_measure_req_rep(wlc, hdr, body, body_len);
#endif /* BCMDBG */

	ASSERT(cfg != NULL);
}

#ifdef BCMDBG
static void
wlc_print_measure_req_rep(wlc_info_t *wlc, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
	struct dot11_action_measure * action_hdr;
	int len;
	int ie_tot_len;
	dot11_meas_req_t* req_ie;
	uint32 start_h, start_l;
	uint16 dur;
	const char *action_name;
	uint8 legal_id;
	bool is_request;
	char da[ETHER_ADDR_STR_LEN];
	char sa[ETHER_ADDR_STR_LEN];
	char bssid[ETHER_ADDR_STR_LEN];

	printf("Action Frame: DA %s SA %s BSSID %s\n",
	       bcm_ether_ntoa(&hdr->da, da), bcm_ether_ntoa(&hdr->sa, sa),
	       bcm_ether_ntoa(&hdr->bssid, bssid));

	if (body_len < 3) {
		printf("Action frame body len was %d, expected > 3\n", body_len);
		return;
	}

	action_hdr = (struct dot11_action_measure *)body;
	req_ie = (dot11_meas_req_t*)action_hdr->data;
	len = body_len - DOT11_ACTION_MEASURE_LEN;

	printf("Action Frame: category %d action %d dialog token %d\n",
	       action_hdr->category, action_hdr->action, action_hdr->token);

	if (action_hdr->category != DOT11_ACTION_CAT_SPECT_MNG) {
		printf("Unexpected category, expected Spectrum Management %d\n",
			DOT11_ACTION_CAT_SPECT_MNG);
		return;
	}

	if (action_hdr->action == DOT11_SM_ACTION_M_REQ) {
		action_name = "Measurement Request";
		legal_id = DOT11_MNG_MEASURE_REQUEST_ID;
		is_request = TRUE;
	} else if (action_hdr->action == DOT11_SM_ACTION_M_REP) {
		action_name = "Measurement Report";
		legal_id = DOT11_MNG_MEASURE_REPORT_ID;
		is_request = FALSE;
	} else {
		printf("Unexpected action type, expected Measurement Request (%d) or Report (%d)\n",
		       DOT11_MNG_MEASURE_REQUEST_ID, DOT11_MNG_MEASURE_REPORT_ID);
		return;
	}

	while (len > 0) {
		if (len < 2) {
			printf("Malformed Action frame, less that an IE header length (2 bytes)"
				" remaining in buffer\n");
			break;
		}
		if (req_ie->id != legal_id) {
			printf("Unexpected IE (id %d len %d):\n", req_ie->id, req_ie->len);
			prhex(NULL, (uint8*)req_ie + TLV_HDR_LEN, req_ie->len);
			goto next_ie;
		}
		if (req_ie->len < DOT11_MNG_IE_MREQ_FIXED_LEN) {
			printf("%s (id %d len %d): len less than minimum of %d\n",
			       action_name, req_ie->id, req_ie->len, DOT11_MNG_IE_MREQ_FIXED_LEN);
			prhex("IE data", (uint8*)req_ie + TLV_HDR_LEN, req_ie->len);
			goto next_ie;
		}
		printf("%s (id %d len %d): measure token %d mode 0x%02x type %d%s\n",
		       action_name, req_ie->id, req_ie->len, req_ie->token, req_ie->mode,
		       req_ie->type,
		       (req_ie->type == DOT11_MEASURE_TYPE_BASIC) ? " \"Basic\"" :
		       ((req_ie->type == DOT11_MEASURE_TYPE_CCA) ? " \"CCA\"" :
			((req_ie->type == DOT11_MEASURE_TYPE_RPI) ? " \"RPI Histogram\"" : "")));

		/* more data past fixed length portion of request/report? */

		if (req_ie->len <= DOT11_MNG_IE_MREP_FIXED_LEN) {
			/* just the fixed bytes of request/report present */
			goto next_ie;
		}

		/* here if more than fixed length portion of request/report */

		if (is_request && (req_ie->mode & DOT11_MEASURE_MODE_ENABLE)) {
			prhex("Measurement Request variable data (should be null since mode Enable"
				" is set)",
				&req_ie->channel, req_ie->len - 3);
			goto next_ie;
		}

		if (!is_request &&
		    (req_ie->mode & (DOT11_MEASURE_MODE_LATE |
			DOT11_MEASURE_MODE_INCAPABLE |
			DOT11_MEASURE_MODE_REFUSED))) {
			prhex("Measurement Report variable data (should be null since mode"
				" Late|Incapable|Refused is set)",
				&req_ie->channel, req_ie->len - DOT11_MNG_IE_MREP_FIXED_LEN);
			goto next_ie;
		}

		if (req_ie->type != DOT11_MEASURE_TYPE_BASIC &&
		    req_ie->type != DOT11_MEASURE_TYPE_CCA &&
		    req_ie->type != DOT11_MEASURE_TYPE_RPI) {
			prhex("variable data", &req_ie->channel, req_ie->len -
				DOT11_MNG_IE_MREP_FIXED_LEN);
			goto next_ie;
		}

		bcopy(req_ie->start_time, &start_l, 4);
		bcopy(&req_ie->start_time[4], &start_h, 4);
		bcopy(&req_ie->duration, &dur, 2);
		start_l = ltoh32(start_l);
		start_h = ltoh32(start_h);
		dur = ltoh16(dur);

		printf("%s variable data: channel %d start time %08X:%08X dur %d TU\n",
		       action_name, req_ie->channel, start_h, start_l, dur);

		if (req_ie->len > DOT11_MNG_IE_MREQ_LEN) {
			prhex("additional data", (uint8*)req_ie + TLV_HDR_LEN +
				DOT11_MNG_IE_MREQ_LEN, req_ie->len - DOT11_MNG_IE_MREQ_LEN);
		}

	next_ie:
		ie_tot_len = TLV_HDR_LEN + req_ie->len;
		req_ie = (dot11_meas_req_t*)((int8*)req_ie + ie_tot_len);
		len -= ie_tot_len;
	}
}
#endif /* BCMDBG */

static void
wlc_send_measure_request(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 measure_type)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_action_measure * action_hdr;
	dot11_meas_req_t *req;
	uint32 tsf_l, tsf_h;
	uint32 measure_tsf_l, measure_tsf_h;
	uint16 duration;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	WL_INFORM(("wl%d: %s: sending Measure Request type %d to %s\n",
	           wlc->pub->unit, __FUNCTION__, measure_type, bcm_ether_ntoa(da, eabuf)));

	ASSERT(cfg != NULL);

	/* Channel Measure Request frame is
	 * 3 bytes Action Measure Req frame
	 * 16 bytes Measure Request IE
	 */
	body_len = DOT11_ACTION_MEASURE_LEN + TLV_HDR_LEN + DOT11_MNG_IE_MREQ_LEN;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &cfg->cur_etheraddr, &cfg->BSSID,
	                       body_len, &pbody);
	if (p == NULL) {
		WL_INFORM(("wl%d: %s: no memory for Measure Request\n",
		           wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* read the tsf from our chip */
	wlc_read_tsf(wlc, &tsf_l, &tsf_h);
	/* set the measure time to now + 100ms */
	measure_tsf_l = tsf_l + 100 * 1000;
	measure_tsf_h = tsf_h;
	if (measure_tsf_l < tsf_l)
		measure_tsf_h++; /* carry from addition */

	action_hdr = (struct dot11_action_measure *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = DOT11_SM_ACTION_M_REQ;
	/* Token needs to be non-zero, so burn the high bit */
	action_hdr->token = (uint8)(wlc->counter | 0x80);
	req = (dot11_meas_req_t *)action_hdr->data;
	req->id = DOT11_MNG_MEASURE_REQUEST_ID;
	req->len = DOT11_MNG_IE_MREQ_LEN;
	req->token = (uint8)(action_hdr->token + 1);
	req->mode = 0;
	req->type = measure_type;
	req->channel = wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC);
	measure_tsf_l = htol32(measure_tsf_l);
	measure_tsf_h = htol32(measure_tsf_h);
	bcopy(&measure_tsf_l, req->start_time, 4);
	bcopy(&measure_tsf_h, &req->start_time[4], 4);
	duration = htol16(50);
	bcopy(&duration, &req->duration, 2);

	wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL);
}

static void
wlc_send_measure_report(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 token, uint8 *report, uint report_len)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_action_measure * action_hdr;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	WL_INFORM(("wl%d: %s: sending Measure Report (token %d) to %s\n",
		wlc->pub->unit, __FUNCTION__, token, bcm_ether_ntoa(da, eabuf)));

	ASSERT(cfg != NULL);

	/* Measure Report frame is
	 * 3 bytes Action Measure Req frame
	 * variable len report
	 */
	body_len = DOT11_ACTION_MEASURE_LEN + report_len;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &cfg->cur_etheraddr, &cfg->BSSID,
	                       body_len, &pbody);
	if (p == NULL) {
		WL_INFORM(("wl%d: %s: no memory for Measure Report\n",
		           wlc->pub->unit, __FUNCTION__));
		return;
	}

	action_hdr = (struct dot11_action_measure *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = DOT11_SM_ACTION_M_REP;
	action_hdr->token = token;

	bcopy(report, action_hdr->data, report_len);

	wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL);
}

int
wlc_11h_set_spect(wlc_11h_info_t *m11h, uint spect)
{
	wlc_info_t *wlc = m11h->wlc;

	ASSERT(!wlc->pub->up);

	if ((spect != SPECT_MNGMT_OFF) &&
	    (spect != SPECT_MNGMT_LOOSE_11H) &&
	    (spect != SPECT_MNGMT_STRICT_11H))
		return BCME_RANGE;

	if (m11h->_spect_management == spect)
		return BCME_OK;

	m11h->_spect_management = spect;
	wlc->pub->_11h = spect != SPECT_MNGMT_OFF;
	wlc_quiet_channels_reset(wlc->cmi);

	return BCME_OK;
}

/* accessors */
uint
wlc_11h_get_spect(wlc_11h_info_t *m11h)
{
	return m11h->_spect_management;
}

void
wlc_11h_set_spect_state(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg, uint mask, uint val)
{
	wlc_11h_t *p11h = IEEE11H_BSSCFG_CUBBY(m11h, cfg);
	uint spect_state;

	ASSERT(p11h != NULL);
	ASSERT((~mask & val) == 0);

	spect_state = p11h->spect_state;
	p11h->spect_state = (uint8)((spect_state & ~mask) | val);
}

uint
wlc_11h_get_spect_state(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg)
{
	wlc_11h_t *p11h = IEEE11H_BSSCFG_CUBBY(m11h, cfg);

	ASSERT(p11h != NULL);

	return p11h->spect_state;
}

#endif /* WL11H */
