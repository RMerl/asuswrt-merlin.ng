/**
 * @file
 * IE management TEST
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
 * $Id: wlc_ie_mgmt_test.c 708017 2017-06-29 14:11:45Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <wl_dbg.h>
#include <osl.h>
#include <siutils.h>
#include <d11.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_mgmt_test.h>

static uint
wlc_iem_test_calc_ssid_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	return 9;
}

static int
wlc_iem_test_write_ssid_ie(void *ctx, wlc_iem_build_data_t *data)
{
	data->buf[TLV_TAG_OFF] = DOT11_MNG_SSID_ID;
	data->buf[TLV_LEN_OFF] = 7;
	bcopy("iemtest", &data->buf[TLV_BODY_OFF], 7);
	return BCME_OK;
}

static uint
wlc_iem_test_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	return 10;
}

static int
wlc_iem_test_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	uint8 rates[] = {0x82, 0x84, 0x0b, 0x8c, 0x98, 0x24, 0xb0, 0x48};
	data->buf[TLV_TAG_OFF] = DOT11_MNG_RATES_ID;
	data->buf[TLV_LEN_OFF] = 8;
	bcopy(rates, &data->buf[TLV_BODY_OFF], 8);
	return BCME_OK;
}

static uint
wlc_iem_test_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	return 4;
}

static int
wlc_iem_test_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	uint8 rates[] = {0x60, 0x6c};
	data->buf[TLV_TAG_OFF] = DOT11_MNG_EXT_RATES_ID;
	data->buf[TLV_LEN_OFF] = 2;
	bcopy(rates, &data->buf[TLV_BODY_OFF], 2);
	return BCME_OK;
}

static uint
wlc_iem_test_calc_vs111_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	if (data->ie != NULL)
		return 0x0a;
	return 0;
}

static int
wlc_iem_test_write_vs111_ie(void *ctx, wlc_iem_build_data_t *data)
{
	uint8 vs111[] = {DOT11_MNG_VS_ID, 8, 0x00, 0x50, 0xf2, 0xff, 0x1, 0x2, 0x3, 0x4};
	bcopy(vs111, &data->buf[TLV_TAG_OFF], sizeof(vs111));
	return BCME_OK;
}

static uint
wlc_iem_test_calc_wpa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	if (data->ie != NULL)
		return 0x14;
	return 0x1a;
}

static int
wlc_iem_test_write_wpa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	uint8 wpa_mod[] = {
		DOT11_MNG_WPA_ID, 0x12,
		0x00, 0x50, 0xf2, 0x01,
		0x01, 0x00, 0x00, 0x50, 0xf2, 0xff,
		0x01, 0x00, 0x00, 0x50, 0xf2, 0xff,
		0x02, 0x01
	};
	uint8 wpa[] = {
		DOT11_MNG_WPA_ID, 0x18,
		0x00, 0x50, 0xf2, 0x01, 0x01, 0x00,
		0x00, 0x50, 0xf2, 0xff,
		0x01, 0x00, 0x00, 0x50, 0xf2, 0xff,
		0x01, 0x00, 0x00, 0x50, 0xf2, 0xff,
		0x00, 0x00
	};

	if (data->ie != NULL)
		bcopy(wpa_mod, &data->buf[TLV_TAG_OFF], sizeof(wpa_mod));
	else
		bcopy(wpa, &data->buf[TLV_TAG_OFF], sizeof(wpa));
	return BCME_OK;
}

static int
wlc_iem_test_parse_ssid_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	if (data->ie != NULL)
		prhex("ssid", data->ie, TLV_HDR_LEN + data->ie[TLV_LEN_OFF]);
	return BCME_OK;
}

static int
wlc_iem_test_parse_sup_rates_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	if (data->ie != NULL)
		prhex("sup", data->ie, TLV_HDR_LEN + data->ie[TLV_LEN_OFF]);
	else
		printf("sup rates IE not present\n");
	return BCME_OK;
}

static int
wlc_iem_test_parse_ext_rates_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	if (data->ie != NULL)
		prhex("ext", data->ie, TLV_HDR_LEN + data->ie[TLV_LEN_OFF]);
	else
		printf("ext rates IE not present\n");
	return BCME_OK;
}

static int
wlc_iem_test_parse_wpa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	if (data->ie != NULL)
		prhex("wpa", data->ie, TLV_HDR_LEN + data->ie[TLV_LEN_OFF]);
	return BCME_OK;
}

int
BCMATTACHFN(wlc_iem_test_register_fns)(wlc_iem_info_t *iem)
{
	int err;

	if ((err = wlc_iem_add_build_fn(iem, WLC_IEM_FC_TEST, DOT11_MNG_SSID_ID,
	    wlc_iem_test_calc_ssid_ie_len, wlc_iem_test_write_ssid_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_build_fn failed, err %d, ssid in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}
	if ((err = wlc_iem_add_build_fn(iem, WLC_IEM_FC_TEST, DOT11_MNG_RATES_ID,
	    wlc_iem_test_calc_sup_rates_ie_len, wlc_iem_test_write_sup_rates_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_build_fn failed, err %d, sup in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}
	if ((err = wlc_iem_add_build_fn(iem, WLC_IEM_FC_TEST, DOT11_MNG_EXT_RATES_ID,
	    wlc_iem_test_calc_ext_rates_ie_len, wlc_iem_test_write_ext_rates_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_build_fn failed, err %d, ext in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}
	if ((err = wlc_iem_vs_add_build_fn(iem, WLC_IEM_FC_TEST, WLC_IEM_VS_IE_PRIO_WPA,
	    wlc_iem_test_calc_wpa_ie_len, wlc_iem_test_write_wpa_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_build_fn failed, err %d, sup in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}
	if ((err = wlc_iem_vs_add_build_fn(iem, WLC_IEM_FC_TEST, 111,
	    wlc_iem_test_calc_vs111_ie_len, wlc_iem_test_write_vs111_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_build_fn failed, err %d, vs111 in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}

	if ((err = wlc_iem_add_parse_fn(iem, WLC_IEM_FC_TEST, DOT11_MNG_SSID_ID,
	                                wlc_iem_test_parse_ssid_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_parse_fn failed, err %d, ssid in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}
	if ((err = wlc_iem_add_parse_fn(iem, WLC_IEM_FC_TEST, DOT11_MNG_RATES_ID,
	                                wlc_iem_test_parse_sup_rates_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_parse_fn failed, err %d, sup in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}
	if ((err = wlc_iem_add_parse_fn(iem, WLC_IEM_FC_TEST, DOT11_MNG_EXT_RATES_ID,
	                                wlc_iem_test_parse_ext_rates_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_parse_fn failed, err %d, ext in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}
	if ((err = wlc_iem_vs_add_parse_fn(iem, WLC_IEM_FC_TEST, WLC_IEM_VS_IE_PRIO_WPA,
	                                wlc_iem_test_parse_wpa_ie, iem)) != BCME_OK) {
		WL_ERROR(("%s: wlc_iem_add_parse_fn failed, err %d, wpa in assoc req\n",
		          __FUNCTION__, err));
		return err;
	}

	return err;
}

static bool
wlc_assoc_req_ins_cb(void *ctx, wlc_iem_ins_data_t *data)
{
	if (data->is_tag &&
	    data->tag == DOT11_MNG_VS_ID &&
	    data->ie[TLV_TAG_OFF] == DOT11_MNG_EXT_CAP_ID)
		return TRUE;
	return FALSE;
}

static bool
wlc_assoc_req_mod_cb(void *ctx, wlc_iem_mod_data_t *data)
{
	uint8 uid[] = {0x00, 0x50, 0xf2, 0x01};
	if (TLV_HDR_LEN + data->ie[TLV_LEN_OFF] >= sizeof(uid) &&
	    data->ie[TLV_TAG_OFF] == DOT11_MNG_VS_ID &&
	    bcmp(&data->ie[TLV_BODY_OFF], uid, sizeof(uid)) == 0)
		return TRUE;
	return FALSE;
}

static uint8
wlc_assoc_req_vsie_cb(void *ctx, wlc_iem_cbvsie_data_t *data)
{
	uint8 vs111[] = {0x00, 0x50, 0xf2, 0xff};
	if (TLV_HDR_LEN + data->ie[TLV_LEN_OFF] >= sizeof(vs111) &&
	    data->ie[TLV_TAG_OFF] == DOT11_MNG_VS_ID &&
	    bcmp(&data->ie[TLV_BODY_OFF], vs111, sizeof(vs111)) == 0)
		return 111;
	return wlc_iem_vs_get_id((wlc_iem_info_t *)ctx, data->ie);
}

int
wlc_iem_test_build_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_iem_uiel_t uiel;
	wlc_iem_cbparm_t cbparm;
	uint8 uie[] = {
		DOT11_MNG_SSID_ID, 3, 'u', 'i', 'e',
		DOT11_MNG_EXT_RATES_ID, 4, 0x8c, 0x98, 0xa4, 0xc0,
		DOT11_MNG_EXT_CAP_ID, 3, 0xaa, 0xaa, 0xaa,
		DOT11_MNG_RATES_ID, 6, 0x82, 0x84, 0x0b, 0x8c, 0x98, 0x24,
		DOT11_MNG_VS_ID, 4, 0x00, 0x50, 0xf2, 0xff,
		DOT11_MNG_VS_ID, 6, 0x00, 0x50, 0xf2, 0x01, 0x00, 0x01
	};
	uint8 *buf;
	uint buf_len;
	int err;

	bzero(&uiel, sizeof(uiel));
	uiel.ies = uie;
	uiel.ies_len = sizeof(uie);
	uiel.ins_fn = wlc_assoc_req_ins_cb;
	uiel.mod_fn = wlc_assoc_req_mod_cb;
	uiel.vsie_fn = wlc_assoc_req_vsie_cb;
	uiel.ctx = wlc->iemi;
	bzero(&cbparm, sizeof(cbparm));

	printf("==========wlc_iem_calc_len==========\n");
	buf_len = wlc_iem_calc_len(wlc->iemi, cfg, WLC_IEM_FC_TEST, &uiel, &cbparm);
	printf("LEGNTH: %u\n", buf_len);
	if (buf_len != 0) {
		if ((buf = MALLOC(wlc->osh, buf_len)) == NULL) {
			return BCME_NOMEM;
		}
		printf("==========wlc_iem_build_frame==========\n");
		err = wlc_iem_build_frame(wlc->iemi, cfg, WLC_IEM_FC_TEST,
		                          &uiel, &cbparm, buf, buf_len);
		if (err != BCME_OK)
			printf("ERROR: %d\n", err);
		MFREE(wlc->osh, buf, buf_len);
	}

	printf("==========wlc_iem_calc_ie_len==========\n");
	buf_len = wlc_iem_calc_ie_len(wlc->iemi, cfg, WLC_IEM_FC_TEST, DOT11_MNG_EXT_CAP_ID,
	                              &uiel, &cbparm);
	printf("LEGNTH: %u\n", buf_len);
	if (buf_len != 0) {
		if ((buf = MALLOC(wlc->osh, buf_len)) == NULL) {
			return BCME_NOMEM;
		}
		printf("==========wlc_iem_build_ie==========\n");
		err = wlc_iem_build_ie(wlc->iemi, cfg, WLC_IEM_FC_TEST, DOT11_MNG_EXT_CAP_ID,
		                       &uiel, &cbparm, buf, buf_len);
		if (err != BCME_OK)
			printf("ERROR: %d\n", err);
		MFREE(wlc->osh, buf, buf_len);
	}

	printf("==========wlc_iem_calc_ie_len==========\n");
	buf_len = wlc_iem_calc_ie_len(wlc->iemi, cfg, WLC_IEM_FC_TEST, DOT11_MNG_EXT_RATES_ID,
	                              &uiel, &cbparm);
	printf("LEGNTH: %u\n", buf_len);
	if (buf_len != 0) {
		if ((buf = MALLOC(wlc->osh, buf_len)) == NULL) {
			return BCME_NOMEM;
		}
		printf("==========wlc_iem_build_ie==========\n");
		err = wlc_iem_build_ie(wlc->iemi, cfg, WLC_IEM_FC_TEST, DOT11_MNG_EXT_RATES_ID,
		                       &uiel, &cbparm, buf, buf_len);
		if (err != BCME_OK)
			printf("ERROR: %d\n", err);
		MFREE(wlc->osh, buf, buf_len);
	}

	printf("==========wlc_iem_vs_calc_ie_len==========\n");
	buf_len = wlc_iem_vs_calc_ie_len(wlc->iemi, cfg, WLC_IEM_FC_TEST, WLC_IEM_VS_IE_PRIO_WPA,
	                                 &uiel, &cbparm);
	printf("LEGNTH: %u\n", buf_len);
	if (buf_len != 0) {
		if ((buf = MALLOC(wlc->osh, buf_len)) == NULL) {
			return BCME_NOMEM;
		}
		printf("==========wlc_iem_vs_build_ie==========\n");
		err = wlc_iem_vs_build_ie(wlc->iemi, cfg, WLC_IEM_FC_TEST, WLC_IEM_VS_IE_PRIO_WPA,
		                          &uiel, &cbparm, buf, buf_len);
		if (err != BCME_OK)
			printf("ERROR: %d\n", err);
		MFREE(wlc->osh, buf, buf_len);
	}

	return BCME_OK;
}

static void
wlc_assoc_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	printf("%s: tag %u\n", __FUNCTION__, data->ie[TLV_TAG_OFF]);
	prhex("unk", data->ie, TLV_HDR_LEN + data->ie[TLV_LEN_OFF]);
}

static uint8
wlc_assoc_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	return wlc_iem_vs_get_id((wlc_iem_info_t *)ctx, data->ie);
}

int
wlc_iem_test_parse_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_iem_upp_t upp;
	wlc_iem_pparm_t pparm;
	uint8 buf[] = {
		DOT11_MNG_SSID_ID, 3, 'u', 'i', 'e',
		DOT11_MNG_EXT_CAP_ID, 3, 0xaa, 0xaa, 0xaa,
		DOT11_MNG_VS_ID, 4, 0x00, 0x50, 0xf2, 0xff,
		DOT11_MNG_VS_ID, 6, 0x00, 0x50, 0xf2, 0x01, 0x00, 0x01
	};
	uint buf_len = sizeof(buf);

	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_assoc_nhdlr_cb;
	upp.vsie_fn = wlc_assoc_vsie_cb;
	upp.ctx = wlc->iemi;
	bzero(&pparm, sizeof(pparm));

	return wlc_iem_parse_frame(wlc->iemi, cfg, WLC_IEM_FC_TEST, &upp, &pparm, buf, buf_len);
}
