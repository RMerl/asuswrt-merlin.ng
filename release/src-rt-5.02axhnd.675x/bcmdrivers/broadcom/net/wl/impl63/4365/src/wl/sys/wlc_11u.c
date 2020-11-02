/*
 * 802.11u module source file (interworking protocol)
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
 * $Id: wlc_11u.c 708017 2017-06-29 14:11:45Z $
 */

#include <wlc_cfg.h>

#ifdef WL11U

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
#include <wlc_11u.h>
#include <wlc_probresp.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_helper.h>

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_INTERWORKING, /* 802.11u enable/disable */
	IOV_LAST
};

const bcm_iovar_t wlc_11u_iovars[] = {
	{"interworking", IOV_INTERWORKING, (0), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

/* 11h module info */
struct wlc_11u_info {
	wlc_info_t *wlc;
	int cfgh;			/* bsscfg cubby handle */
};

/* local functions */
/* module */
static void wlc_11u_update_ext_cap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool enable);
static int wlc_11u_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);

/* cubby */
static int wlc_11u_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_11u_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
#ifdef BCMDBG
static void wlc_11u_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_11u_bsscfg_dump NULL
#endif // endif

/* IE mgmt */
static uint wlc_11u_calc_iw_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_11u_write_iw_ie(void *ctx, wlc_iem_build_data_t *data);
#ifdef STA
static uint wlc_11u_calc_assoc_iw_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_11u_write_assoc_iw_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#ifdef AP
static uint wlc_11u_calc_iwap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_11u_write_iwap_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_11u_calc_iwrc_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_11u_write_iwrc_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_11u_calc_qos_map_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_11u_write_qos_map_ie(void *ctx, wlc_iem_build_data_t *data);
#endif /* AP */
static int wlc_11u_assocresp_parse_qos_map_ie(void *ctx, wlc_iem_parse_data_t *data);

typedef struct {
	uint8		*iw_ie; 	/* 802.11u interworking(IW) IE */
	uint8		*qos_ie;	/* configured QOS map IE */
	uint8		*up_table;	/* user priority table */
	bool		interworking;	/* interworking enabled */
} wlc_11u_bsscfg_cubby_t;
#define WLC_11U_BSSCFG_CUBBY(m11u, cfg) \
	((wlc_11u_bsscfg_cubby_t *)BSSCFG_CUBBY((cfg), (m11u)->cfgh))

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

#define UP_TABLE_MAX	((IPV4_TOS_DSCP_MASK >> IPV4_TOS_DSCP_SHIFT) + 1)	/* 64 max */

/* set user priority table range from low to high with up value */
static bool
up_table_set(uint8 *up_table, uint8 up, uint8 low, uint8 high)
{
	int i;

	if (up > 7 || low > high || low >= UP_TABLE_MAX || high >= UP_TABLE_MAX)
		return FALSE;

	for (i = low; i <= high; i++)
		up_table[i] = up;

	return TRUE;
}

/* convert QoS Map IE to user priority table indexed by DSCP for faster lookup
 * as per IEEE 802.11-2012, sec 8.4.2.97 QoS Map Set element
 */
static bool
qos_map_set(uint8 *up_table, bcm_tlv_t *qos_map_ie)
{
	bool rc = FALSE;
	uint8 len;

	if (up_table == NULL)
		return FALSE;

	/* clear table */
	memset(up_table, 0xff, UP_TABLE_MAX);

	/* length of QoS Map IE must be 16+n*2, n is number of exceptions */
	if (qos_map_ie != NULL && qos_map_ie->id == DOT11_MNG_QOS_MAP_ID &&
		(len = qos_map_ie->len) >= QOS_MAP_FIXED_LENGTH &&
		(len % 2) == 0) {
		uint8 *except_ptr = qos_map_ie->data;
		uint8 except_len = len - QOS_MAP_FIXED_LENGTH;
		uint8 *range_ptr = except_ptr + except_len;
		int i;

		/* fill in ranges */
		for (i = 0; i < QOS_MAP_FIXED_LENGTH; i += 2) {
			uint8 low = range_ptr[i];
			uint8 high = range_ptr[i + 1];
			if (low == 255 && high == 255)
				continue;
			if (!up_table_set(up_table, i / 2, low, high)) {
				/* clear the table on failure */
				memset(up_table, 0xff, UP_TABLE_MAX);
				goto done;
			}
		}

		/* update exceptions */
		for (i = 0; i < except_len; i += 2) {
			uint8 dscp = except_ptr[i];
			uint8 up = except_ptr[i + 1];
			/* exceptions with invalid dscp/up are ignored */
			up_table_set(up_table, up, dscp, dscp);
		}

		rc = TRUE;
	}
done:
	return rc;
}

/* lookup user priority for specified DSCP */
static uint8
dscp2up(uint8 *up_table, uint8 dscp)
{
	uint8 up = 255;

	/* lookup up from table if parameters valid */
	if (up_table != NULL && dscp < UP_TABLE_MAX)
		up = up_table[dscp];

	/* 255 is unused value so return up from dscp */
	if (up == 255)
		up = dscp >> (IPV4_TOS_PREC_SHIFT - IPV4_TOS_DSCP_SHIFT);

	return up;
}

wlc_11u_info_t *
BCMATTACHFN(wlc_11u_attach)(wlc_info_t *wlc)
{
	wlc_11u_info_t *m11u;
	uint16 iwfstbmp =
#ifdef STA
	        FT2BMP(FC_PROBE_REQ) |
#endif // endif
#ifdef AP
	        FT2BMP(FC_BEACON) |
	        FT2BMP(FC_PROBE_RESP) |
#endif // endif
	        0;
#ifdef STA
	uint16 assocfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
#endif // endif
#ifdef AP
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
#endif // endif
	uint16 arsfstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP);

	if (!wlc)
		return NULL;

	if ((m11u = MALLOCZ(wlc->osh, sizeof(wlc_11u_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	m11u->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((m11u->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(wlc_11u_bsscfg_cubby_t),
		wlc_11u_bsscfg_init, wlc_11u_bsscfg_deinit, wlc_11u_bsscfg_dump,
		m11u)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register IE mgmt callback */
	/* bcn/prbrsp/prbreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, iwfstbmp, DOT11_MNG_INTERWORKING_ID,
	      wlc_11u_calc_iw_ie_len, wlc_11u_write_iw_ie, m11u) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, iw ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef STA
	/* assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, assocfstbmp, DOT11_MNG_INTERWORKING_ID,
	      wlc_11u_calc_assoc_iw_ie_len, wlc_11u_write_assoc_iw_ie, m11u) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, iw ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif	/* STA */
#ifdef AP
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_ADVERTISEMENT_ID,
	      wlc_11u_calc_iwap_ie_len, wlc_11u_write_iwap_ie, m11u) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, ad in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_ROAM_CONSORT_ID,
	      wlc_11u_calc_iwrc_ie_len, wlc_11u_write_iwrc_ie, m11u) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, rc in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arsfstbmp, DOT11_MNG_QOS_MAP_ID,
	      wlc_11u_calc_qos_map_ie_len, wlc_11u_write_qos_map_ie, m11u) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, rc in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */

	/* parse */
	/* assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arsfstbmp, DOT11_MNG_QOS_MAP_ID,
		wlc_11u_assocresp_parse_qos_map_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, qos map in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* keep the module registration the last other add module unregistration
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, wlc_11u_iovars, "11u", m11u, wlc_11u_doiovar,
	                        NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	wlc->pub->_11u = TRUE;
	wlc_probresp_register(wlc->mprobresp, m11u, wlc_11u_check_probe_req_iw, FALSE);
	return m11u;

	/* error handling */
fail:
	if (m11u != NULL)
		MFREE(wlc->osh, m11u, sizeof(wlc_11u_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_11u_detach)(wlc_11u_info_t *m11u)
{
	wlc_info_t *wlc;

	if (m11u) {
		wlc = m11u->wlc;
		wlc_probresp_unregister(wlc->mprobresp, m11u);
		wlc_module_unregister(wlc->pub, "11u", m11u);

		MFREE(wlc->osh, m11u, sizeof(wlc_11u_info_t));
	}
}

/* update extend capabilities */
static void
wlc_11u_update_ext_cap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool enable)
{
	wlc_bsscfg_set_ext_cap(bsscfg, DOT11_EXT_CAP_IW, enable);
	wlc_bsscfg_set_ext_cap(bsscfg, DOT11_EXT_CAP_QOS_MAP, enable);
	if (bsscfg->up && (BSSCFG_AP(bsscfg) ||	BSSCFG_IBSS(bsscfg))) {
		/* update AP or IBSS beacons */
		wlc_bss_update_beacon(wlc, bsscfg);
		/* update AP or IBSS probe responses */
		wlc_bss_update_probe_resp(wlc, bsscfg, TRUE);
	}
}

static int
wlc_11u_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *bsscfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;

	/* check if 11u supported */
	if (!WL11U_ENAB(wlc)) {
		return BCME_UNSUPPORTED;
	}

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* get the cubby */
	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, bsscfg);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_INTERWORKING):
		*ret_int_ptr = cubby_11u->interworking;
		break;
	case IOV_SVAL(IOV_INTERWORKING):
		/* if bss configuration is down then return error */
		if (BSSCFG_AP(bsscfg) && (!bsscfg->up)) {
			err = BCME_NOTUP;
			break;
		}
		if (cubby_11u->interworking != bool_val) {
			cubby_11u->interworking = bool_val;
			wlc_11u_update_ext_cap(wlc, bsscfg, bool_val);
		}
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* bsscfg cubby */
static int
wlc_11u_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	return BCME_OK;
}

static void
wlc_11u_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_11u_bsscfg_cubby_t *cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->up_table != NULL) {
		MFREE(m11u->wlc->osh, cubby_11u->up_table, UP_TABLE_MAX);
		cubby_11u->up_table = NULL;
	}
}

#ifdef BCMDBG
static void
wlc_11u_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_11u_bsscfg_cubby_t *cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);
	int i;
	ASSERT(cubby_11u != NULL);

	if (cubby_11u->iw_ie) {
		bcm_bprintf(b, "IW IE len: %d\n", cubby_11u->iw_ie[1]);
		for (i = 0; i < cubby_11u->iw_ie[1]; i++) {
			bcm_bprintf(b, "IW data[%d]: 0x%x\n", i, cubby_11u->iw_ie[i]);
		}
	}
}
#endif /* BCMDBG */

/* check interworking IE in probe request */
bool
wlc_11u_check_probe_req_iw(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len, bool *psendProbeResp)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)handle;
	wlc_11u_bsscfg_cubby_t *cubby_11u;
	wlc_info_t *wlc;
	bcm_tlv_t *iw;
	bool sendProbeResp = TRUE;
	uint8 ap_iw_len;

	if (!m11u)
		return TRUE;

	wlc = m11u->wlc;
	if (!WL11U_ENAB(wlc))
		return TRUE;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->iw_ie == NULL)
		return sendProbeResp;

	iw = bcm_parse_tlvs(body, body_len, DOT11_MNG_INTERWORKING_ID);
	ap_iw_len = cubby_11u->iw_ie[TLV_LEN_OFF];
	if (iw && iw->len && ap_iw_len) {
		uint8 sta_ant = (iw->data[IW_ANT_OFFSET] & IW_ANT_MASK);
		uint8 ap_ant = (cubby_11u->iw_ie[TLV_HDR_LEN+IW_ANT_OFFSET] & IW_ANT_MASK);
		if ((sta_ant != IW_ANT_WILDCARD) && (ap_ant != IW_ANT_WILDCARD) &&
		            (sta_ant != ap_ant)) {
			sendProbeResp = FALSE;
		} else if ((iw->len > ETHER_ADDR_LEN) && (ap_iw_len > ETHER_ADDR_LEN)) {
			uint8 *hessid = (iw->len >= IW_LEN) ?
				(&iw->data[IW_HESSID_OFFSET]) : (&iw->data[SHORT_IW_HESSID_OFFSET]);
			uint8 *hessid_ap = (ap_iw_len >= IW_LEN) ?
				(&cubby_11u->iw_ie[TLV_HDR_LEN+IW_HESSID_OFFSET]) :
				(&cubby_11u->iw_ie[TLV_HDR_LEN+SHORT_IW_HESSID_OFFSET]);
			if ((!ETHER_ISBCAST(hessid)) && (!ETHER_ISBCAST(hessid_ap)) &&
				bcmp(hessid, hessid_ap, ETHER_ADDR_LEN))
					sendProbeResp = FALSE;
		}
	}
	return sendProbeResp;
}

/* get 802.11u IE */
uint8 *
wlc_11u_get_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, uint8 ie_type)
{
	uint8 *ie_data = NULL;
	wlc_11u_bsscfg_cubby_t *cubby_11u;
	wlc_info_t *wlc;

	if (!m11u)
		return NULL;

	wlc = m11u->wlc;
	if (!WL11U_ENAB(wlc))
		return NULL;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	switch (ie_type) {
	case DOT11_MNG_INTERWORKING_ID:
		ie_data = cubby_11u->iw_ie;
		break;
	case DOT11_MNG_QOS_MAP_ID:
		ie_data = cubby_11u->qos_ie;
		break;
	default:
		ie_data = wlc_vndr_ie_find_by_type(cfg, ie_type);
		break;
	}
	return ie_data;
}

/* set 802.11u IE */
int
wlc_11u_set_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, uint8 *ie_data,
	bool *bcn_upd, bool *prbresp_upd)
{
	wlc_11u_bsscfg_cubby_t *cubby_11u;
	wlc_info_t *wlc;
	int err = BCME_OK;
	int ie_len;
	uint8 ie_type;

	if (!m11u)
		return BCME_UNSUPPORTED;

	wlc = m11u->wlc;
	if (!WL11U_ENAB(wlc))
		return BCME_UNSUPPORTED;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	ie_type = ie_data[TLV_TAG_OFF];
	ie_len = ie_data[TLV_LEN_OFF] + TLV_HDR_LEN;

	switch (ie_type) {
		case DOT11_MNG_INTERWORKING_ID:
			if (BSSCFG_AP(cfg)) {
				*bcn_upd = TRUE;
				*prbresp_upd = TRUE;
			}
			break;
#ifdef AP
		case DOT11_MNG_QOS_MAP_ID:
			if (BSSCFG_STA(cfg)) {
				err = BCME_UNSUPPORTED;
				return err;
			}
			/* fall through */
		case DOT11_MNG_ADVERTISEMENT_ID:
		case DOT11_MNG_ROAM_CONSORT_ID:
			if ((cubby_11u->iw_ie != NULL) && BSSCFG_AP(cfg)) {
				*bcn_upd = TRUE;
				*prbresp_upd = TRUE;
			}
			break;
#endif /* AP */
		default:
			err = BCME_UNSUPPORTED;
			return err;
	}

	if (ie_len == TLV_HDR_LEN) {
		/* delete the IE if len is zero */
		wlc_vndr_ie_del_by_type(cfg, ie_type);
	} else {
		/* update the IE */
		err = wlc_vndr_ie_mod_elem_by_type(cfg, ie_type,
			VNDR_IE_CUSTOM_FLAG, (vndr_ie_t *)ie_data);
	}

	/* update the pointer to the TLV field in the list for quick access */
	switch (ie_type) {
	case DOT11_MNG_INTERWORKING_ID:
		cubby_11u->iw_ie = wlc_vndr_ie_find_by_type(cfg, DOT11_MNG_INTERWORKING_ID);
		wlc_bsscfg_set_ext_cap(cfg, DOT11_EXT_CAP_IW, (cubby_11u->iw_ie != NULL));
		break;
	case DOT11_MNG_QOS_MAP_ID:
		cubby_11u->qos_ie = wlc_vndr_ie_find_by_type(cfg,
			DOT11_MNG_QOS_MAP_ID);
		if (cubby_11u->qos_ie != NULL) {
			wlc_11u_set_rx_qos_map_ie(m11u, cfg, (bcm_tlv_t *)cubby_11u->qos_ie,
				cubby_11u->qos_ie[TLV_LEN_OFF] + TLV_HDR_LEN);
		}
		wlc_bsscfg_set_ext_cap(cfg, DOT11_EXT_CAP_QOS_MAP,
			(cubby_11u->qos_ie != NULL));
		break;
	default:
		break;
	}

	return err;
}

/* 802.11u IW IE */
static uint
wlc_11u_calc_iw_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (WL11U_ENAB(wlc)) {

		cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);
		ASSERT(cubby_11u != NULL);

		if (cubby_11u->iw_ie != NULL)
			return TLV_HDR_LEN + cubby_11u->iw_ie[TLV_LEN_OFF];
	}

	return 0;
}

static int
wlc_11u_write_iw_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (WL11U_ENAB(wlc)) {

		cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);
		ASSERT(cubby_11u != NULL);

		if (cubby_11u->iw_ie != NULL) {
			bcm_copy_tlv(cubby_11u->iw_ie, data->buf);
		}
	}

	return BCME_OK;
}

#ifdef STA
/* return TRUE if wlc_bss_info_t contains IW IE, else FALSE */
static bool
wlc_11u_is_iw_ie(wlc_bss_info_t *bi)
{
	uint bcn_parse_len = bi->bcn_prb_len - sizeof(struct dot11_bcn_prb);
	uint8 *bcn_parse = (uint8*)bi->bcn_prb + sizeof(struct dot11_bcn_prb);

	if (bcm_parse_tlvs(bcn_parse, bcn_parse_len, DOT11_MNG_INTERWORKING_ID))
		return TRUE;

	return FALSE;
}

/* 802.11u IW IE */
static uint
wlc_11u_calc_assoc_iw_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	/* include IW IE only if target AP supports IW */
	if (wlc_11u_is_iw_ie(wlc_iem_calc_get_assocreq_target(data)))
		return wlc_11u_calc_iw_ie_len(ctx, data);

	return 0;
}

static int
wlc_11u_write_assoc_iw_ie(void *ctx, wlc_iem_build_data_t *data)
{
	/* include IW IE only if target AP supports IW */
	if (wlc_11u_is_iw_ie(wlc_iem_build_get_assocreq_target(data)))
		return wlc_11u_write_iw_ie(ctx, data);

	return BCME_OK;
}
#endif	/* STA */

#ifdef AP
/* 802.11u IWAP IE */
static uint
wlc_11u_calc_iwap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (!WL11U_ENAB(wlc))
		return 0;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->iw_ie != NULL) {
		uint8 *iwap_ie = wlc_vndr_ie_find_by_type(cfg, DOT11_MNG_ADVERTISEMENT_ID);

		if (iwap_ie != NULL) {
			if (iwap_ie[TLV_BODY_OFF+1] != DOT11_MNG_VS_ID)
				return TLV_HDR_LEN + iwap_ie[TLV_LEN_OFF];
			else {
				uint len = wlc_vndr_ie_getlen(cfg, VNDR_IE_IWAPID_FLAG, NULL);

				if (1 + len != IWAP_QUERY_INFO_SIZE)
					return TLV_HDR_LEN + 1 + len;
			}
		}
	}

	return 0;
}

static int
wlc_11u_write_iwap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (!WL11U_ENAB(wlc))
		return BCME_OK;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->iw_ie != NULL) {
		uint8 *iwap_ie = wlc_vndr_ie_find_by_type(cfg,
			DOT11_MNG_ADVERTISEMENT_ID);

		if (iwap_ie != NULL) {
			if (iwap_ie[TLV_BODY_OFF+1] != DOT11_MNG_VS_ID) {
				bcm_copy_tlv_safe(iwap_ie, data->buf, data->buf_len);
			}
			else {
				uint len = wlc_vndr_ie_getlen(cfg, VNDR_IE_IWAPID_FLAG, NULL);

				if (1 + len != IWAP_QUERY_INFO_SIZE) {
					uint8 *cp = data->buf;
					uint l = data->buf_len;

					cp[TLV_TAG_OFF] = DOT11_MNG_ADVERTISEMENT_ID;
					cp[TLV_LEN_OFF] = 1 + len;
					cp[TLV_BODY_OFF] = iwap_ie[TLV_BODY_OFF];
					cp += TLV_BODY_OFF + 1;
					l -= TLV_BODY_OFF + 1;

					wlc_vndr_ie_write(cfg, cp, l, VNDR_IE_IWAPID_FLAG);
				}
			}
		}
	}

	return BCME_OK;
}

/* 802.11u IWRC IE */
static uint
wlc_11u_calc_iwrc_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (!WL11U_ENAB(wlc))
		return 0;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->iw_ie != NULL) {
		uint8 *iwrc_ie = wlc_vndr_ie_find_by_type(cfg, DOT11_MNG_ROAM_CONSORT_ID);

		if (iwrc_ie != NULL)
			return TLV_HDR_LEN + iwrc_ie[TLV_LEN_OFF];
	}

	return 0;
}

static int
wlc_11u_write_iwrc_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (!WL11U_ENAB(wlc))
		return BCME_OK;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->iw_ie != NULL) {
		uint8 *iwrc_ie = wlc_vndr_ie_find_by_type(cfg, DOT11_MNG_ROAM_CONSORT_ID);

		if (iwrc_ie != NULL) {
			bcm_copy_tlv(iwrc_ie, data->buf);
		}
	}

	return BCME_OK;
}
static uint
wlc_11u_calc_qos_map_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (!WL11U_ENAB(wlc))
		return 0;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->iw_ie) {
		uint8 *iwqm_ie = wlc_vndr_ie_find_by_type(cfg,
			DOT11_MNG_QOS_MAP_ID);

		/* 11u IW QoS map IE */
		if (iwqm_ie != NULL) {
			return (iwqm_ie[TLV_LEN_OFF] + TLV_HDR_LEN);
		}
	}
	return 0;
}

static int
wlc_11u_write_qos_map_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_11u_info_t *m11u = (wlc_11u_info_t *)ctx;
	wlc_info_t *wlc = m11u->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	if (!WL11U_ENAB(wlc))
		return BCME_OK;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->iw_ie) {
		uint8 *iwqm_ie = wlc_vndr_ie_find_by_type(cfg, DOT11_MNG_QOS_MAP_ID);

		/* 11u IW QoS map IE */
		if (iwqm_ie != NULL) {
			bcm_copy_tlv(iwqm_ie, data->buf);
		}
	}

	return BCME_OK;
}
#endif /* AP */

/* QOS MAP */
static int
wlc_11u_assocresp_parse_qos_map_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	/* 11u QOS map */
	if (WL11U_ENAB(wlc)) {
		wlc_11u_set_rx_qos_map_ie(wlc->m11u, cfg, (bcm_tlv_t *)data->ie, data->ie_len);
	}

	return BCME_OK;
}

/* set QOS map IE */
/* return 0 if QOS map not set, non-zero if QOS map set */
int
wlc_11u_set_rx_qos_map_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, bcm_tlv_t *ie, int ie_len)
{
	int ret = 0;
	wlc_info_t *wlc;
	wlc_11u_bsscfg_cubby_t *cubby_11u;

	/* invalid parameters and length */
	if (!m11u || !ie || ie->len > ie_len - TLV_HDR_LEN)
		return 0;

	wlc = m11u->wlc;

	cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	if (cubby_11u->up_table == NULL) {
		cubby_11u->up_table = MALLOCZ(wlc->osh,	UP_TABLE_MAX);
		if (cubby_11u->up_table == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return ret;
		}
	}

	/* convert QoS Map to user priority table */
	if (qos_map_set(cubby_11u->up_table, ie)) {
		ret = 1;
	} else {
		MFREE(m11u->wlc->osh, cubby_11u->up_table, UP_TABLE_MAX);
		cubby_11u->up_table = NULL;
	}

	return ret;
}

/* whether it is 11u IE */
bool
wlc_11u_is_11u_ie(wlc_11u_info_t *m11u, uint8 ie_type)
{
	bool result;
	switch (ie_type) {
		case DOT11_MNG_INTERWORKING_ID:
		case DOT11_MNG_ADVERTISEMENT_ID:
		case DOT11_MNG_ROAM_CONSORT_ID:
		case DOT11_MNG_QOS_MAP_ID:
			result = TRUE;
			break;
		default:
			result = FALSE;
			break;
	}
	return result;
}

/* set the packet priority from QoS Map */
void
wlc_11u_set_pkt_prio(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, void *pkt)
{
	wlc_11u_bsscfg_cubby_t *cubby_11u = WLC_11U_BSSCFG_CUBBY(m11u, cfg);

	/* set priority only if interworking enabled and UP table available */
	if (cubby_11u->interworking && cubby_11u->up_table != NULL) {
		osl_t *osh = m11u->wlc->osh;
		uint8 *pktdata;
		uint pktlen;
		uint8 dscp;

		pktdata = (uint8 *)PKTDATA(osh, pkt);
		ASSERT(ISALIGNED((uintptr)pktdata, sizeof(uint16)));
		pktlen = PKTLEN(osh, pkt);

		if (pktgetdscp(pktdata, pktlen, &dscp)) {
			PKTSETPRIO(pkt, dscp2up(cubby_11u->up_table, dscp));
		}
	}
}

#endif /* WL11U */
