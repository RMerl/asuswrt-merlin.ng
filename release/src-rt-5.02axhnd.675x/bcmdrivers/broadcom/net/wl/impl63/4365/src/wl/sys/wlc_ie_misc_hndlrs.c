/**
 * @file
 * Miscellaneous IE handlers
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
 * $Id: wlc_ie_misc_hndlrs.c 787563 2020-06-03 17:06:49Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <proto/bcmip.h>
#include <proto/wpa.h>
#include <proto/vlan.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#include <wlioctl.h>
#include <epivers.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX) || defined(EXT_STA) || defined(STA) || \
	defined(LINUX_CRYPTO)
#include <proto/eapol.h>
#endif // endif
#include <bcmwpa.h>
#ifdef BCMCCX
#include <bcmcrypto/ccx.h>
#endif /* BCMCCX */
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <d11.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_cca.h>
#include <wlc_interfere.h>
#include <wlc_bsscfg.h>
#include <wlc_vndr_ie_list.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_led.h>
#include <wlc_frmutil.h>
#include <wlc_stf.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#include <wlc_tbtt.h>
#endif // endif
#ifdef WLP2P
#include <wlc_p2p.h>
#endif // endif
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#include <wlc_scb_ratesel.h>
#ifdef WL_LPC
#include <wlc_scb_powersel.h>
#endif /* WL_LPC */
#include <wlc_event.h>
#include <wlc_seq_cmds.h>
#include <wl_export.h>
#include "d11ucode.h"
#if defined(BCMSUP_PSK) || defined(BCMCCX)
#include <wlc_sup.h>
#endif // endif
#include <wlc_pmkid.h>
#if defined(BCMAUTH_PSK)
#include <wlc_auth.h>
#endif // endif
#ifdef BCMSDIO
#include <bcmsdh.h>
#endif // endif
#ifdef WET
#include <wlc_wet.h>
#endif // endif
#ifdef WMF
#include <wlc_wmf.h>
#endif // endif
#ifdef PSTA
#include <wlc_psta.h>
#endif /* PSTA */
#if defined(BCMNVRAMW) || defined(WLTEST)
#include <bcmotp.h>
#endif // endif
#ifdef BCMCCMP
#include <bcmcrypto/aes.h>
#endif // endif
#include <wlc_rm.h>
#ifdef BCMCCX
#include <wlc_ccx.h>
#endif // endif
#include "wlc_cac.h"
#include <wlc_ap.h>
#ifdef AP
#include <wlc_apcs.h>
#endif // endif
#include <wlc_scan.h>
#ifdef WLWNM
#include <wlc_wnm.h>
#endif // endif
#include <wlc_extlog.h>
#include <wlc_assoc.h>
#ifdef STA
#include <wlc_wpa.h>
#endif /* STA */
#include <wlc_lq.h>
#include <wlc_11h.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_dfs.h>
#include <wlc_11d.h>
#include <wlc_cntry.h>
#include <bcm_mpool_pub.h>
#include <wlc_utils.h>
#include <wlc_hrt.h>
#include <wlc_prot.h>
#include <wlc_prot_g.h>
#define _inc_wlc_prot_n_preamble_	/* include static INLINE uint8 wlc_prot_n_preamble() */
#include <wlc_prot_n.h>
#include <wlc_11u.h>
#include <wlc_probresp.h>
#ifdef WL11AC
#include <wlc_vht.h>
#endif // endif
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)
#include <wlc_wapi.h>
#endif // endif
#include <wlc_pcb.h>
#include <wlc_txc.h>
#ifdef MFP
#include <wlc_mfp.h>
#endif // endif
#include <wlc_macfltr.h>
#include <wlc_addrmatch.h>
#include <wlc_bmon.h>
#ifdef WL_RELMCAST
#include "wlc_relmcast.h"
#endif // endif
#include <wlc_btcx.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#include <wlc_ie_helper.h>
#include <wlc_akm_ie.h>
#include <wlc_ht.h>
#ifdef ANQPO
#include <wl_anqpo.h>
#endif // endif
#include <wlc_hs20.h>
#ifdef STA
#include <wlc_pm.h>
#endif /* STA */
#ifdef WLFBT
#include <wlc_fbt.h>
#endif // endif
#if defined(BCMCCX) && defined(BCMINTSUP)
#include <wlc_sup_ccx.h>
#endif // endif

#ifdef WLOLPC
#include <wlc_olpc_engine.h>
#endif /* OPEN LOOP POWER CAL */

#ifdef WL_STAPRIO
#include <wlc_staprio.h>
#endif /* WL_STAPRIO */

#include <wlc_monitor.h>
#include <wlc_stamon.h>
#include <wlc_ie_misc_hndlrs.h>
#include <wlc_ht.h>

static void
_wlc_process_extcap_ie(wlc_info_t *wlc,  bcm_tlv_t *ie, struct scb *scb)
{
	dot11_extcap_ie_t *extcap_ie_tlv = (dot11_extcap_ie_t *)ie;
	dot11_extcap_t *cap;

	ASSERT(ie != NULL);
	ASSERT(scb != NULL);

	scb->flags &= ~SCB_COEX_MGMT;
	scb->flags2 &= ~SCB2_TDLS_MASK;

	cap = (dot11_extcap_t*)extcap_ie_tlv->cap;
	if (extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_COEX) {
		if (isset(cap->extcap, DOT11_EXT_CAP_OBSS_COEX_MGMT))
			scb->flags |= SCB_COEX_MGMT;
	}
	if (extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_TDLS) {
		if (isset(cap->extcap, DOT11_TDLS_CAP_PROH))
			scb->flags2 |= SCB2_TDLS_PROHIBIT;
		if (isset(cap->extcap, DOT11_TDLS_CAP_CH_SW_PROH))
			scb->flags2 |= SCB2_TDLS_CHSW_PROHIBIT;
		if (isset(cap->extcap, DOT11_TDLS_CAP_TDLS))
			scb->flags2 |= SCB2_TDLS_SUPPORT;
		if (isset(cap->extcap, DOT11_TDLS_CAP_PU_BUFFER_STA))
			scb->flags2 |= SCB2_TDLS_PU_BUFFER_STA;
		if (isset(cap->extcap, DOT11_TDLS_CAP_PEER_PSM))
			scb->flags2 |= SCB2_TDLS_PEER_PSM;
		if (isset(cap->extcap, DOT11_TDLS_CAP_CH_SW))
			scb->flags2 |= SCB2_TDLS_CHSW_SUPPORT;
	}
	if (extcap_ie_tlv->len >= CEIL(DOT11_EXT_CAP_OPER_MODE_NOTIF, 8)) {
		if (isset(cap->extcap, DOT11_EXT_CAP_OPER_MODE_NOTIF)) {
			scb->flags3 |= SCB3_OPER_MODE_NOTIF;
		}
	}

#ifdef WLWNM
	if (WLWNM_ENAB(wlc->pub)) {
		uint32 wnmcap = 0;
		/* ext_cap 11-th bit for FMS service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_FMS) &&
		    isset(cap->extcap, DOT11_EXT_CAP_FMS))
			wnmcap |= WL_WNM_FMS;

		/* ext_cap 12-th bit for PROXYARP service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_PROXY_ARP) &&
		    isset(cap->extcap, DOT11_EXT_CAP_PROXY_ARP))
			wnmcap |= WL_WNM_PROXYARP;

		/* ext_cap 16-th bit for TFS service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_TFS) &&
		    isset(cap->extcap, DOT11_EXT_CAP_TFS))
			wnmcap |= WL_WNM_TFS;

		/* ext_cap 17-th bit for WNM-Sleep service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_WNM_SLEEP) &&
		    isset(cap->extcap, DOT11_EXT_CAP_WNM_SLEEP))
			wnmcap |= WL_WNM_SLEEP;

		/* ext_cap 18-th bit for TIM Broadcast service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_TIMBC) &&
		    isset(cap->extcap, DOT11_EXT_CAP_TIMBC))
			wnmcap |= WL_WNM_TIMBC;

		/* ext_cap 19-th bit for BSS-Transition service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_BSSTRANS) &&
		    isset(cap->extcap, DOT11_EXT_CAP_BSSTRANS_MGMT))
			wnmcap |= WL_WNM_BSSTRANS;

		/* ext_cap 26-th bit for DMS service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_DMS) &&
		    isset(cap->extcap, DOT11_EXT_CAP_DMS))
			wnmcap |= WL_WNM_DMS;

		/* ext_cap 46-th bit for DMS service */
		if ((extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_WNM_NOTIFICATION) &&
		    isset(cap->extcap, DOT11_EXT_CAP_WNM_NOTIF))
			wnmcap |= WL_WNM_NOTIF;

		/* saved to WNM scb cubby */
		wlc_wnm_set_scbcap(wlc, scb, wnmcap);
	}
#endif /* WLWNM */
}

#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif /* WL_PWRSTATS */

void
wlc_process_extcap_ie(wlc_info_t *wlc, uint8 *tlvs, int len, struct scb *scb)
{
	bcm_tlv_t *extcap_ie_tlv;

	extcap_ie_tlv = bcm_parse_tlvs(tlvs, len, DOT11_MNG_EXT_CAP_ID);
	if (!extcap_ie_tlv)
		return;

	_wlc_process_extcap_ie(wlc, extcap_ie_tlv, scb);
}

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * TODO: Move these functions to their own modules when possible.
 * Leave them here for now.
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */
/* Extended Capabilities IE */

/* encodes the ext cap ie based on intersection of target AP and self ext cap
 * output buffer size must be DOT11_EXTCAP_LEN_MAX, encoded length is returned
 */
static int
encode_ext_cap_ie(wlc_bsscfg_t *cfg, uint8 *buffer)
{
	uint32 result_len = 0;
	wlc_bss_info_t *bi = cfg->target_bss;
	uint bcn_parse_len = bi->bcn_prb_len - sizeof(struct dot11_bcn_prb);
	uint8 *bcn_parse = (uint8*)bi->bcn_prb + sizeof(struct dot11_bcn_prb);
	bcm_tlv_t *bcn_ext_cap;

	bcn_ext_cap = bcm_parse_tlvs(bcn_parse, bcn_parse_len, DOT11_MNG_EXT_CAP_ID);

	if (bcn_ext_cap && cfg->ext_cap_len) {
		uint32 i;
		uint32 ext_cap_len = MIN(bcn_ext_cap->len, cfg->ext_cap_len);

		for (i = 0; i < ext_cap_len; i++) {
			/* Take the intersection of AP and self ext cap */
			buffer[i] = bcn_ext_cap->data[i] & cfg->ext_cap[i];
			/* is intersection non-zero */
			if (buffer[i])
				result_len = i + 1;
		}
	}

	return result_len;
}

static uint
wlc_bss_calc_conditional_ext_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	uint8 buffer[DOT11_EXTCAP_LEN_MAX];
	int len;

	if ((len = encode_ext_cap_ie(data->cfg,	buffer)) > 0) {
		len += TLV_HDR_LEN;
	}

	return len;
}

static int
wlc_bss_write_conditional_ext_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	uint8 buffer[DOT11_EXTCAP_LEN_MAX];
	int len;

	len = encode_ext_cap_ie(data->cfg, buffer);

	/* add extended capabilities */
	if (len) {
		if (bcm_write_tlv_safe(DOT11_MNG_EXT_CAP_ID, buffer,
			len, data->buf, data->buf_len) == data->buf) {
			return BCME_BUFTOOSHORT;
		}
	}

	return BCME_OK;
}

static uint
wlc_bss_calc_ext_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;

	if (cfg->ext_cap_len == 0) {
		return 0;
	}

	if (data->ft == FC_ASSOC_REQ ||
		data->ft == FC_REASSOC_REQ ||
		data->ft == FC_AUTH) {
		/* ext cap IE added only if target AP supports it */
		return wlc_bss_calc_conditional_ext_cap_ie_len(ctx, data);
	}

	return TLV_HDR_LEN + cfg->ext_cap_len;
}

static int
wlc_bss_write_ext_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;

	if (cfg->ext_cap_len == 0) {
		return BCME_OK;
	}

	if (data->ft == FC_ASSOC_REQ ||
		data->ft == FC_REASSOC_REQ ||
		data->ft == FC_AUTH) {
		/* ext cap IE added only if target AP supports it */
		return wlc_bss_write_conditional_ext_cap_ie(ctx, data);
	}

	if (bcm_write_tlv_safe(DOT11_MNG_EXT_CAP_ID, cfg->ext_cap,
		cfg->ext_cap_len, data->buf, data->buf_len) == data->buf) {
		return BCME_BUFTOOSHORT;
	}

	return BCME_OK;
}

static int
wlc_bss_parse_ext_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm;
	struct scb *scb = NULL;

	switch (data->ft) {
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		ASSERT(data->pparm != NULL);
		ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);
		scb = ftpparm->assocreq.scb;
		ASSERT(scb != NULL);
		break;
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP:
		ASSERT(data->pparm != NULL);
		ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);
		scb = ftpparm->assocresp.scb;
		ASSERT(scb != NULL);
		break;
	case FC_BEACON:
		ASSERT(data->pparm != NULL);
		ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);
		if (ftpparm->bcn.scb != NULL && SCB_WDS(ftpparm->bcn.scb))
			scb = ftpparm->bcn.scb;
		else if (BSSCFG_STA(data->cfg))
			scb = ftpparm->bcn.scb;
		break;
	}

	if (scb != NULL) {
		if (data->ie != NULL) {
			_wlc_process_extcap_ie(wlc, (bcm_tlv_t *)data->ie, scb);
		}
	}

	return BCME_OK;
}

#ifdef WLP2P
static bool
wlc_vndr_non_p2p_ie_filter(void *arg, const vndr_ie_t *ie)
{
	uint8 *parse;
	uint parse_len;

	ASSERT(ie != NULL);

	if (ie->id != DOT11_MNG_VS_ID)
		return FALSE;

	parse = (uint8 *)ie;
	parse_len = TLV_HDR_LEN + ie->len;

	return !bcm_is_p2p_ie((uint8 *)ie, &parse, &parse_len);
}

static int
wlc_vndr_non_p2p_ie_getlen(wlc_bsscfg_t *cfg, uint32 pktflag, int *totie)
{
	return wlc_vndr_ie_getlen_ext(cfg, wlc_vndr_non_p2p_ie_filter, pktflag, totie);
}

static bool
wlc_vndr_non_p2p_ie_write_filter(void *arg, uint type, const vndr_ie_t *ie)
{
	return wlc_vndr_non_p2p_ie_filter(arg, ie);
}

static uint8 *
wlc_vndr_non_p2p_ie_write(wlc_bsscfg_t *cfg, uint type, uint32 pktflag, uint8 *cp, int buflen)
{
	return wlc_vndr_ie_write_ext(cfg, wlc_vndr_non_p2p_ie_write_filter, type,
	                             cp, buflen, pktflag);
}
#endif /* WLP2P */

/* Common: Vendor added IEs */
static uint
wlc_bss_calc_vndr_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
#ifdef WLP2P
	wlc_info_t *wlc = (wlc_info_t *)ctx;
#endif // endif
	wlc_bsscfg_t *cfg = data->cfg;
	uint16 type = data->ft;
	uint32 flag;

	if (type == FC_AUTH) {
		wlc_iem_ft_cbparm_t *ftcbparm;

		ASSERT(data->cbparm != NULL);
		ftcbparm = data->cbparm->ft;
		ASSERT(ftcbparm != NULL);

		flag = wlc_auth2vieflag(ftcbparm->auth.seq);
	}
	else
		flag = wlc_ft2vieflag(type);

#ifdef WLP2P
	/* leave all p2p IEs up to p2p IE handling */
	if (type == FC_ASSOC_RESP || type == FC_REASSOC_RESP) {
		wlc_iem_ft_cbparm_t *ftcbparm;

		ASSERT(data->cbparm != NULL);
		ftcbparm = data->cbparm->ft;
		ASSERT(ftcbparm != NULL);

		if (SCB_P2P(ftcbparm->assocresp.scb))
			return wlc_vndr_non_p2p_ie_getlen(cfg, flag, NULL);
	}
	else if (BSS_P2P_ENAB(wlc, cfg)) {
		return wlc_vndr_non_p2p_ie_getlen(cfg, flag, NULL);
	}
#endif /* WLP2P */

	if (flag != 0)
		return wlc_vndr_ie_getlen(cfg, flag, NULL);

	return 0;
}

static int
wlc_bss_write_vndr_ie(void *ctx, wlc_iem_build_data_t *data)
{
#ifdef WLP2P
	wlc_info_t *wlc = (wlc_info_t *)ctx;
#endif // endif
	wlc_bsscfg_t *cfg = data->cfg;
	uint16 type = data->ft;
	uint32 flag;

	if (type == FC_AUTH) {
		wlc_iem_ft_cbparm_t *ftcbparm;

		ASSERT(data->cbparm != NULL);
		ftcbparm = data->cbparm->ft;
		ASSERT(ftcbparm != NULL);

		flag = wlc_auth2vieflag(ftcbparm->auth.seq);
	}
	else
		flag = wlc_ft2vieflag(type);

#ifdef WLP2P
	/* leave all p2p IEs up to p2p IE handling */
	if (type == FC_ASSOC_RESP || type == FC_REASSOC_RESP) {
		wlc_iem_ft_cbparm_t *ftcbparm;

		ASSERT(data->cbparm != NULL);
		ftcbparm = data->cbparm->ft;
		ASSERT(ftcbparm != NULL);

		if (SCB_P2P(ftcbparm->assocresp.scb)) {
			wlc_vndr_non_p2p_ie_write(cfg, type, flag, data->buf, data->buf_len);
			return BCME_OK;
		}
	}
	else if (BSS_P2P_ENAB(wlc, cfg)) {
		wlc_vndr_non_p2p_ie_write(cfg, type, flag, data->buf, data->buf_len);
		return BCME_OK;
	}
#endif /* WLP2P */

	if (flag != 0) {
		wlc_vndr_ie_write(cfg, data->buf, data->buf_len, flag);
	}

	return BCME_OK;
}

#ifdef MULTIAP
/* VS: Multi-AP IE */
static uint
wlc_bss_calc_multiap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm = NULL;
	struct scb *scb = NULL;
	uint len = 0;

	if (!MAP_ENAB(cfg)) {
		return len;
	}

	switch (data->ft) {
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			ASSERT(data->cbparm != NULL);
			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);
			scb = ftcbparm->assocresp.scb;
			ASSERT(scb != NULL);
			break;
	}

	if (scb && !SCB_MAP_CAP(scb)) {
		return len;
	}

	if (cfg->multiap_ie[TLV_LEN_OFF] > 0) {
		len = TLV_HDR_LEN + cfg->multiap_ie[TLV_LEN_OFF];
	}

	return len;
}

/* VS: Multi-AP IE */
static int
wlc_bss_write_multiap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm = NULL;
	struct scb *scb = NULL;

	if (!MAP_ENAB(cfg)) {
		return BCME_OK;
	}

	switch (data->ft) {
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			ASSERT(data->cbparm != NULL);
			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);
			scb = ftcbparm->assocresp.scb;
			ASSERT(scb != NULL);
			break;
	}

	if (scb && !SCB_MAP_CAP(scb)) {
		return BCME_OK;
	}

	if (cfg->multiap_ie[TLV_LEN_OFF] > 0) {
		bcm_copy_tlv(cfg->multiap_ie, data->buf);
	}

	return BCME_OK;
}

/* VS: Multi-AP IE */
static int
wlc_bss_parse_multiap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm;
	struct scb *scb = NULL;

	switch (data->ft) {
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			ASSERT(data->pparm != NULL);
			ftpparm = data->pparm->ft;
			ASSERT(ftpparm != NULL);
			scb = ftpparm->assocreq.scb;
			ASSERT(scb != NULL);
			break;
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			ASSERT(data->pparm != NULL);
			ftpparm = data->pparm->ft;
			ASSERT(ftpparm != NULL);
			scb = ftpparm->assocresp.scb;
			ASSERT(scb != NULL);
			break;
	}

	if (scb != NULL && data->ie != NULL) {
		wlc_process_multiap_ie(wlc, scb, (multiap_ie_t *)data->ie);
	}

	return BCME_OK;
}
#endif	/* MULTIAP */

/* Common: BRCM IE */
static uint
wlc_bss_calc_brcm_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if ((wlc->brcm_ie) && (cfg->brcm_ie[TLV_LEN_OFF] > 0))
		return TLV_HDR_LEN + cfg->brcm_ie[TLV_LEN_OFF];

	return 0;
}

static int
wlc_bss_write_brcm_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if ((wlc->brcm_ie) && (cfg->brcm_ie[TLV_LEN_OFF] > 0)) {
		bcm_copy_tlv(cfg->brcm_ie, data->buf);
	}

	return BCME_OK;
}

static int
wlc_bss_parse_brcm_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm;
	struct scb *scb = NULL;

	switch (data->ft) {
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		ASSERT(data->pparm != NULL);
		ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);
		scb = ftpparm->assocreq.scb;
		ASSERT(scb != NULL);
		break;
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP:
		ASSERT(data->pparm != NULL);
		ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);
		scb = ftpparm->assocresp.scb;
		ASSERT(scb != NULL);
		break;
	case FC_BEACON:
		ASSERT(data->pparm != NULL);
		ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);
		if (ftpparm->bcn.scb != NULL && SCB_WDS(ftpparm->bcn.scb))
			scb = ftpparm->bcn.scb;
		else if (BSSCFG_STA(data->cfg))
			scb = ftpparm->bcn.scb;
		break;
	}

	if (scb != NULL) {
		if (data->ie != NULL) {
			wlc_process_brcm_ie(wlc, scb, (brcm_ie_t *)data->ie);
		}
	}

	return BCME_OK;
}

/* Common: WME parameters IE */
static uint
wlc_bss_calc_wme_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	bool iswme = FALSE;

	if (!BSS_WME_ENAB(wlc, cfg))
		return 0;

	switch (data->ft) {
	case FC_BEACON:
	case FC_PROBE_RESP:
		/* WME IE for beacon responses in IBSS when 11n enabled */
		if (BSSCFG_AP(cfg) || (!cfg->BSS && data->cbparm->ht))
			iswme = TRUE;
		break;
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		/* Include a WME info element if the AP supports WME */
		if (ftcbparm->assocreq.target->flags & WLC_BSS_WME)
			iswme = TRUE;
		break;
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP:
		if (SCB_WME(ftcbparm->assocresp.scb))
			iswme = TRUE;
		break;
	}
	if (!iswme)
		return 0;

	if (BSSCFG_AP(cfg))
		return TLV_HDR_LEN + sizeof(wme_param_ie_t);
	else
		return TLV_HDR_LEN + sizeof(wme_ie_t);

	return 0;
}

static int
wlc_bss_write_wme_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_wme_t *wme = cfg->wme;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	bool iswme = FALSE;
	bool apapsd = FALSE;

	if (!BSS_WME_ENAB(wlc, cfg))
		return BCME_OK;

	switch (data->ft) {
	case FC_BEACON:
	case FC_PROBE_RESP:
		if (BSSCFG_AP(cfg))
			iswme = TRUE;
		/* WME IE for beacon responses in IBSS when 11n enabled */
		else if (!cfg->BSS && data->cbparm->ht) {
			apapsd = (cfg->current_bss->wme_qosinfo & WME_QI_AP_APSD_MASK) ?
			        TRUE : FALSE;
			iswme = TRUE;
		}
		break;
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		/* Include a WME info element if the AP supports WME */
		if (ftcbparm->assocreq.target->flags & WLC_BSS_WME) {
			apapsd = (ftcbparm->assocreq.target->wme_qosinfo & WME_QI_AP_APSD_MASK) ?
			        TRUE : FALSE;
			iswme = TRUE;
		}
		break;
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP:
		if (SCB_WME(ftcbparm->assocresp.scb))
			iswme = TRUE;
		break;
	}
	if (!iswme)
		return BCME_OK;

	/* WME parameter info element in infrastructure beacons responses only */
	if (BSSCFG_AP(cfg)) {
		edcf_acparam_t *acp_ie = wme->wme_param_ie.acparam;
		wme_param_ie_t *ad = wme->wme_param_ie_ad;
		uint8 i = 0;

		if (wme->wme_apsd)
			ad->qosinfo |= WME_QI_AP_APSD_MASK;
		else
			ad->qosinfo &= ~WME_QI_AP_APSD_MASK;

		/* update the ACM value in WME IE */
		for (i = 0; i < AC_COUNT; i++, acp_ie++) {
			if (acp_ie->ACI & EDCF_ACM_MASK)
				ad->acparam[i].ACI |= EDCF_ACM_MASK;
			else
				ad->acparam[i].ACI &= ~EDCF_ACM_MASK;
		}

		bcm_write_tlv(DOT11_MNG_VS_ID, (uint8 *)ad, sizeof(wme_param_ie_t), data->buf);
	}
	else {
		wme_ie_t wme_ie;

		ASSERT(sizeof(wme_ie) == WME_IE_LEN);
		bcopy(WME_OUI, wme_ie.oui, WME_OUI_LEN);
		wme_ie.type = WME_OUI_TYPE;
		wme_ie.subtype = WME_SUBTYPE_IE;
		wme_ie.version = WME_VER;
		if (!wme->wme_apsd || !apapsd) {
			wme_ie.qosinfo = 0;
		} else {
			wme_ie.qosinfo = wme->apsd_sta_qosinfo;
		}

		bcm_write_tlv(DOT11_MNG_VS_ID, (uint8 *)&wme_ie, sizeof(wme_ie), data->buf);
	}

	return BCME_OK;
}

static int
wlc_bss_parse_wme_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	struct scb *scb;
	wlc_iem_ft_pparm_t *ftpparm;
	bcm_tlv_t *wme_ie = (bcm_tlv_t *)data->ie;
	wlc_wme_t *wme = cfg->wme;

	scb = wlc_iem_parse_get_assoc_bcn_scb(data);
	ASSERT(scb != NULL);

	/* Do not parse IE if TLV length doesn't matches the size of the structure */
	if (wme_ie != NULL &&
		(BSSCFG_STA(cfg) ?
		(wme_ie->len != sizeof(wme_param_ie_t)) : (wme_ie->len != sizeof(wme_ie_t)))) {
		WL_ERROR(("%s Incorrect TLV - IE len: %d size of WME data: %d\n",
			__FUNCTION__, wme_ie->len,
			BSSCFG_STA(cfg) ? (uint)sizeof(wme_param_ie_t) : (uint)sizeof(wme_ie_t)));
		if (BSS_WME_ENAB(wlc, cfg)) {
			/* clear WME flags */
			scb->flags &= ~(SCB_WMECAP | SCB_APSDCAP);
			cfg->flags &= ~WLC_BSSCFG_WME_ASSOC;

			/* Clear Qos Info by default */
			wlc_qosinfo_update(scb, 0, TRUE);
		}
		return BCME_OK;
	}

	switch (data->ft) {
#ifdef AP
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:

		/* Handle WME association */
		scb->flags &= ~(SCB_WMECAP | SCB_APSDCAP);

		if (!BSS_WME_ENAB(wlc, cfg))
			break;

		wlc_qosinfo_update(scb, 0, TRUE);     /* Clear Qos Info by default */

		if (wme_ie == NULL)
			break;

		scb->flags |= SCB_WMECAP;

		/* Note requested APSD parameters if AP supporting APSD */
		if (!wme->wme_apsd)
			break;

		wlc_qosinfo_update(scb, ((wme_ie_t *)wme_ie->data)->qosinfo, TRUE);

		if (scb->apsd.ac_trig & AC_BITMAP_ALL)
			scb->flags |= SCB_APSDCAP;
		break;
#endif /* AP */
#ifdef STA
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP: {
		wlc_pm_st_t *pm = cfg->pm;
		bool upd_trig_delv;

		/* If WME is enabled, check if response indicates WME association */
		scb->flags &= ~SCB_WMECAP;
		cfg->flags &= ~WLC_BSSCFG_WME_ASSOC;

		if (!BSS_WME_ENAB(wlc, cfg))
			break;

		/* Do not update ac_delv and ac for ReassocResp with same AP */
		/* upd_trig_delv is FALSE for ReassocResp with same AP, TRUE otherwise */
		upd_trig_delv = !((data->ft == FC_REASSOC_RESP) &&
			(!bcmp((char *)&cfg->prev_BSSID,
			(char *)&cfg->target_bss->BSSID, ETHER_ADDR_LEN)));
		wlc_qosinfo_update(scb, 0, upd_trig_delv);

		if (wme_ie == NULL)
			break;

		scb->flags |= SCB_WMECAP;
		cfg->flags |= WLC_BSSCFG_WME_ASSOC;

		/* save the new IE, or params IE which is superset of IE */
		bcopy(wme_ie->data, &wme->wme_param_ie, wme_ie->len);
		/* Apply the STA AC params sent by AP,
		 * will be done in wlc_join_adopt_bss()
		 */
		/* wlc_edcf_acp_apply(wlc, cfg, TRUE); */
		/* Use locally-requested APSD config if AP advertised APSD */
		/* STA is in AUTO WME mode,
		 *     AP has UAPSD enabled, then allow STA to use wlc->PM
		 *            else, don't allow STA to sleep based on wlc->PM only
		 *                  if it's BRCM AP not capable of handling
		 *                                  WME STAs in PS,
		 *                  and leave PM mode if already set
		 */
		if ((wme->wme_param_ie.qosinfo & WME_QI_AP_APSD_MASK) && (wme->wme_apsd)) {
			wlc_qosinfo_update(scb, wme->apsd_sta_qosinfo, upd_trig_delv);
			pm->WME_PM_blocked = FALSE;
			if (pm->PM == PM_MAX)
				wlc_set_pmstate(cfg, TRUE);
		}
		else if (WME_AUTO(wlc) &&
		         (scb->flags & SCB_BRCM)) {
			if (!(scb->flags & SCB_WMEPS)) {
				pm->WME_PM_blocked = TRUE;
				WL_RTDC(wlc, "wlc_recvctl: exit PS", 0, 0);
				wlc_set_pmstate(cfg, FALSE);
			}
			else {
				pm->WME_PM_blocked = FALSE;
				if (pm->PM == PM_MAX)
					wlc_set_pmstate(cfg, TRUE);
			}
		}
		break;
	}
	case FC_BEACON:
		/* WME: check if the AP has supplied new acparams */
		/* WME: check if IBSS WME_IE is present */
		if (!BSS_WME_AS(wlc, cfg))
			break;

		if (scb && BSSCFG_IBSS(cfg)) {
			if (wme_ie != NULL) {
				scb->flags |= SCB_WMECAP;
			}
			break;
		}

		if (wme_ie == NULL) {
			WL_ERROR(("wl%d: %s: wme params ie missing\n",
			          wlc->pub->unit, __FUNCTION__));
			/* for non-wme association, BE ACI is 2 */
			wme->wme_param_ie.acparam[0].ACI = NON_EDCF_AC_BE_ACI_STA;
			wlc_edcf_acp_apply(wlc, cfg, TRUE);
			break;
		}

		if ((((wme_ie_t *)wme_ie->data)->qosinfo & WME_QI_AP_COUNT_MASK) !=
		    (wme->wme_param_ie.qosinfo & WME_QI_AP_COUNT_MASK)) {
			/* save and apply new params ie */
			bcopy(wme_ie->data, &wme->wme_param_ie,	sizeof(wme_param_ie_t));
			/* Apply the STA AC params sent by AP */
			wlc_edcf_acp_apply(wlc, cfg, TRUE);
		}
		break;
#endif /* STA */
	default:
		(void)wlc;
		(void)scb;
		(void)ftpparm;
		(void)wme_ie;
		(void)wme;
		break;
	}

	return BCME_OK;
}

/* register common IE mgmt handlers */
int
BCMATTACHFN(wlc_register_iem_fns)(wlc_info_t *wlc)
{
	wlc_iem_info_t *iemi = wlc->iemi;
	/* Extended Capabilities IE */
	uint16 ext_cap_build_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_REQ) |
	        FT2BMP(FC_REASSOC_RESP) |
	        FT2BMP(FC_PROBE_REQ) |
	        FT2BMP(FC_PROBE_RESP) |
	        FT2BMP(FC_BEACON) |
	        0;
	uint16 ext_cap_parse_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_REQ) |
	        FT2BMP(FC_REASSOC_RESP) |
	        FT2BMP(FC_BEACON) |
	        0;
	/* Vendor added Vendor Specific IEs */
	uint16 vndr_build_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_REQ) |
	        FT2BMP(FC_REASSOC_RESP) |
	        FT2BMP(FC_PROBE_REQ) |
	        FT2BMP(FC_PROBE_RESP) |
	        FT2BMP(FC_BEACON) |
	        FT2BMP(FC_DISASSOC) |
	        FT2BMP(FC_AUTH) |
	        FT2BMP(FC_DEAUTH) |
	        0;
	/* Brcm Vendor Specific IE */
	uint16 brcm_build_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_REQ) |
	        FT2BMP(FC_REASSOC_RESP) |
	        FT2BMP(FC_PROBE_REQ) |
	        FT2BMP(FC_PROBE_RESP) |
	        FT2BMP(FC_BEACON) |
	        FT2BMP(FC_AUTH) |
	        0;
	uint16 brcm_parse_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_REQ) |
	        FT2BMP(FC_REASSOC_RESP) |
	        FT2BMP(FC_BEACON) |
	        0;
	/* WME Vendor Specific IE */
	uint16 wme_build_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_REQ) |
	        FT2BMP(FC_REASSOC_RESP) |
	        FT2BMP(FC_PROBE_RESP) |
	        FT2BMP(FC_BEACON) |
	        0;
	uint16 wme_parse_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_REQ) |
	        FT2BMP(FC_REASSOC_RESP) |
	        FT2BMP(FC_BEACON) |
	        0;
#ifdef	MULTIAP
	uint16 map_fstbmp = FT2BMP(FC_ASSOC_RESP) |  FT2BMP(FC_REASSOC_RESP) |
		FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
#endif	/* MULTIAP */
	int err = BCME_OK;

	/* calc/build */
#ifdef MULTIAP
	if ((err = wlc_iem_vs_add_build_fn_mft(iemi, map_fstbmp, WLC_IEM_VS_IE_PRIO_MULTIAP,
	      wlc_bss_calc_multiap_ie_len, wlc_bss_write_multiap_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, err %d, multiap ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#endif	/* MULTIAP */
	if ((err = wlc_iem_add_build_fn_mft(iemi, ext_cap_build_fstbmp, DOT11_MNG_EXT_CAP_ID,
	      wlc_bss_calc_ext_cap_ie_len, wlc_bss_write_ext_cap_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, ext cap ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_vs_add_build_fn_mft(iemi, vndr_build_fstbmp, WLC_IEM_VS_IE_PRIO_VNDR,
	      wlc_bss_calc_vndr_ie_len, wlc_bss_write_vndr_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, err %d, vndr ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_vs_add_build_fn_mft(iemi, brcm_build_fstbmp, WLC_IEM_VS_IE_PRIO_BRCM,
	      wlc_bss_calc_brcm_ie_len, wlc_bss_write_brcm_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, err %d, brcm ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_vs_add_build_fn_mft(iemi, wme_build_fstbmp, WLC_IEM_VS_IE_PRIO_WME,
	      wlc_bss_calc_wme_ie_len, wlc_bss_write_wme_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, err %d, wme ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	/* parse */
#ifdef MULTIAP
	if ((err = wlc_iem_vs_add_parse_fn_mft(iemi, map_fstbmp, WLC_IEM_VS_IE_PRIO_MULTIAP,
	                                    wlc_bss_parse_multiap_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, err %d, multiap ie in assocreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#endif	/* MULTIAP */
	if ((err = wlc_iem_add_parse_fn_mft(iemi, ext_cap_parse_fstbmp, DOT11_MNG_EXT_CAP_ID,
	                                    wlc_bss_parse_ext_cap_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, err %d, ext cap ie in assocreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_vs_add_parse_fn_mft(iemi, brcm_parse_fstbmp, WLC_IEM_VS_IE_PRIO_BRCM,
	                                       wlc_bss_parse_brcm_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, err %d, brcm ie in assocreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_vs_add_parse_fn_mft(iemi, wme_parse_fstbmp, WLC_IEM_VS_IE_PRIO_WME,
	                                       wlc_bss_parse_wme_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, err %d, wme ie in assocreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	return BCME_OK;
fail:

	return err;
}

#ifdef STA
/* AssocReq: SSID IE */
static uint
wlc_assoc_calc_ssid_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;

	return TLV_HDR_LEN + bi->SSID_len;
}

static int
wlc_assoc_write_ssid_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;

	bcm_write_tlv(DOT11_MNG_SSID_ID, bi->SSID, bi->SSID_len, data->buf);

	return BCME_OK;
}

/* AssocReq: Supported Rates IE */
static uint
wlc_assoc_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->assocreq.sup->count == 0)
		return 0;

	return TLV_HDR_LEN + ftcbparm->assocreq.sup->count;
}

static int
wlc_assoc_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->assocreq.sup->count == 0)
		return BCME_OK;

	bcm_write_tlv(DOT11_MNG_RATES_ID, ftcbparm->assocreq.sup->rates,
		ftcbparm->assocreq.sup->count, data->buf);

	return BCME_OK;
}

/* AssocReq: Extended Supported Rates IE */
static uint
wlc_assoc_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->assocreq.ext->count == 0)
		return 0;

	return TLV_HDR_LEN + ftcbparm->assocreq.ext->count;
}

static int
wlc_assoc_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->assocreq.ext->count == 0)
		return BCME_OK;

	bcm_write_tlv(DOT11_MNG_EXT_RATES_ID, ftcbparm->assocreq.ext->rates,
		ftcbparm->assocreq.ext->count, data->buf);

	return BCME_OK;
}

#if defined(WL_GLOBAL_RCLASS)
static int
wlc_assoc_write_sup_opclass_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	chanspec_t chanspec;
	uint8 rclen;                       /* regulatory class length */
	uint8 rclist[MAXRCLISTSIZE];       /* regulatory class list */

	ASSERT(data != NULL && data->cfg != NULL && data->cfg->target_bss);
	chanspec = data->cfg->target_bss->chanspec;
	rclen = wlc_get_regclass_list(wlc->cmi, rclist, MAXRCLISTSIZE, chanspec, TRUE);
	if (rclen <= data->buf_len) {
		bcm_write_tlv_safe(DOT11_MNG_REGCLASS_ID, rclist, rclen, data->buf, data->buf_len);
	} else {
		ASSERT(0);
	}

	return BCME_OK;
}

static uint
wlc_assoc_calc_sup_opclass_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	chanspec_t chanspec;
	uint8 rclen;                       /* regulatory class length */
	uint8 rclist[MAXRCLISTSIZE];       /* regulatory class list */

	ASSERT(data != NULL && data->cfg != NULL && data->cfg->target_bss);
	chanspec = data->cfg->target_bss->chanspec;
	rclen = wlc_get_regclass_list(wlc->cmi, rclist, MAXRCLISTSIZE, chanspec, TRUE);
	return TLV_HDR_LEN + rclen;
}
#endif /* WL_GLOBAL_RCLASS */

/* register AssocReq/ReassocReq IE mgmt handlers */
int
BCMATTACHFN(wlc_assoc_register_iem_fns)(wlc_info_t *wlc)
{
	wlc_iem_info_t *iemi = wlc->iemi;
	int err = BCME_OK;
	uint16 fstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);

	/* calc/build */
	/* assocreq/reassocreq */
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_SSID_ID,
	      wlc_assoc_calc_ssid_ie_len, wlc_assoc_write_ssid_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, ssid ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_RATES_ID,
	      wlc_assoc_calc_sup_rates_ie_len, wlc_assoc_write_sup_rates_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, sup rates ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_EXT_RATES_ID,
	    wlc_assoc_calc_ext_rates_ie_len, wlc_assoc_write_ext_rates_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, ext rates ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#if defined(WL_GLOBAL_RCLASS)
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_REGCLASS_ID,
		wlc_assoc_calc_sup_opclass_len, wlc_assoc_write_sup_opclass_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, err %d, sup opclass ie\n",
			wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#endif // endif

	return BCME_OK;
fail:

	return err;
}
#endif /* STA */

/* SSID IE */
static uint
wlc_bcn_calc_ssid_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->ft == FC_BEACON && cfg->closednet_nobcnssid) {
	}

	return TLV_HDR_LEN + cfg->SSID_len;
}

static int
wlc_bcn_write_ssid_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;
	uint8 ssid[DOT11_MAX_SSID_LEN];

	if ((data->ft == FC_BEACON) && cfg->closednet_nobcnssid) {
	}

	/* in the closed net case the ssid is faked to be all zero */
	if ((data->ft == FC_BEACON) && cfg->closednet_nobcnssid) {
		bzero(ssid, cfg->SSID_len);
	} else {
		bcopy(cfg->SSID, ssid, cfg->SSID_len);
	}

	bcm_write_tlv(DOT11_MNG_SSID_ID, ssid, cfg->SSID_len, data->buf);

	return BCME_OK;
}

/* Supported Rates IE */
static uint
wlc_bcn_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	return TLV_HDR_LEN + ftcbparm->bcn.sup->count;
}

static int
wlc_bcn_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	bcm_write_tlv(DOT11_MNG_RATES_ID, ftcbparm->bcn.sup->rates,
		ftcbparm->bcn.sup->count, data->buf);

	return BCME_OK;
}

/* Extended Supported Rates IE */
static uint
wlc_bcn_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (wlc->band->gmode || wlc_ht_get_phy_membership(wlc->hti)) {
		if (ftcbparm->bcn.ext->count == 0)
			return 0;
		return TLV_HDR_LEN + ftcbparm->bcn.ext->count;
	}

	return 0;
}

static int
wlc_bcn_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (wlc->band->gmode || wlc_ht_get_phy_membership(wlc->hti)) {
		if (ftcbparm->bcn.ext->count == 0) {
			return BCME_OK;

		}

		bcm_write_tlv(DOT11_MNG_EXT_RATES_ID, ftcbparm->bcn.ext->rates,
			ftcbparm->bcn.ext->count, data->buf);
	}

	return BCME_OK;
}

/* DS Parameters */
static uint
wlc_bcn_calc_ds_parms_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_bss_info_t *current_bss = cfg->current_bss;

	if (!CHSPEC_IS2G(current_bss->chanspec))
		return 0;

	return TLV_HDR_LEN + 1;
}

static int
wlc_bcn_write_ds_parms_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	uint8 chan;

	if (!CHSPEC_IS2G(current_bss->chanspec))
		return BCME_OK;

	chan = wf_chspec_ctlchan(current_bss->chanspec);

	bcm_write_tlv(DOT11_MNG_DS_PARMS_ID, &chan, 1, data->buf);

	return BCME_OK;
}

#ifdef STA
/* IBSS Params */
static uint
wlc_bcn_calc_ibss_parms_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
#ifdef WLP2P
	wlc_info_t *wlc = (wlc_info_t *)ctx;
#endif // endif
	wlc_bsscfg_t *cfg = data->cfg;

	if (!cfg->BSS &&
#ifdef WLP2P
	    !BSS_P2P_DISC_ENAB(wlc, cfg) &&
#endif // endif
	    TRUE) {
		return TLV_HDR_LEN + sizeof(uint16);
	}

	return 0;
}

static int
wlc_bcn_write_ibss_parms_ie(void *ctx, wlc_iem_build_data_t *data)
{
#ifdef WLP2P
	wlc_info_t *wlc = (wlc_info_t *)ctx;
#endif // endif
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_bss_info_t *current_bss = cfg->current_bss;

	if (!cfg->BSS &&
#ifdef WLP2P
	    !BSS_P2P_DISC_ENAB(wlc, cfg) &&
#endif // endif
	    TRUE) {
		uint16 atim = htol16(current_bss->atim_window);

		bcm_write_tlv(DOT11_MNG_IBSS_PARMS_ID, &atim, sizeof(uint16), data->buf);
	}

	return BCME_OK;
}

/*
 * Check if STA's bit is set in TIM
 * See Tality MS/futils.c: boFUTILS_InTIM()
 */
static bool
wlc_InTIM(wlc_bsscfg_t *cfg, bcm_tlv_t *tim_ie, uint tim_ie_len)
{
	uint pvboff, AIDbyte;
	uint pvblen;

	if (tim_ie == NULL ||
	    tim_ie->len < DOT11_MNG_TIM_FIXED_LEN ||
	    tim_ie->data[DOT11_MNG_TIM_DTIM_COUNT] >= tim_ie->data[DOT11_MNG_TIM_DTIM_PERIOD]) {
		/* sick AP, prevent going to power-save mode */
#ifdef BCMDBG
		if (tim_ie == NULL) {
			WL_PRINT(("wl%d.%d: %s: no TIM\n",
			          cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		}
		else if (tim_ie->len < DOT11_MNG_TIM_FIXED_LEN) {
			WL_PRINT(("wl%d.%d: %s: short TIM %d\n",
			          cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
			          tim_ie->len));
		}
		else {
			WL_PRINT(("wl%d.%d: %s: bad DTIM count %d\n",
			          cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
			          tim_ie->data[DOT11_MNG_TIM_DTIM_COUNT]));
		}
#endif // endif
		wlc_set_pmoverride(cfg, TRUE);
		return FALSE;
	}

	if (cfg->pm->PM_override) {
#ifdef BCMDBG
		WL_PRINT(("wl%d.%d: %s: good TIM is restored\n",
		          cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
#endif // endif
		wlc_set_pmoverride(cfg, FALSE);
	}
	if (cfg->dtim_programmed == 0) {
		wlc_adopt_dtim_period(cfg, tim_ie->data[DOT11_MNG_TIM_DTIM_PERIOD]);
	}

#if defined(DEBUG_TIM)
	if (WL_INFORM_ON()) {
		prhex("BCN_INFO: TIM", tim_ie, TLV_HDR_LEN + tim_ie->len);
	}
#endif /* defined(DEBUG_TIM) */

	/* extract bitmap offset (N1) from bitmap control field */
	pvboff = tim_ie->data[DOT11_MNG_TIM_BITMAP_CTL] & 0xfe;

	/* compute bitmap length (N2 - N1) from info element length */
	pvblen = tim_ie->len - DOT11_MNG_TIM_FIXED_LEN;

	/* bail early if our AID precedes the TIM */
	AIDbyte = (cfg->AID & DOT11_AID_MASK) >> 3;
	if (AIDbyte < pvboff || AIDbyte >= pvboff + pvblen)
		return FALSE;

	/* check our AID in bitmap */
	return (tim_ie->data[DOT11_MNG_TIM_PVB + AIDbyte - pvboff] &
	        (1 << (cfg->AID & 0x7))) ? TRUE : FALSE;
}
/*
 * tell the AP that STA is ready to
 * receive traffic if it is there.
 */
void
wlc_bcn_tim_ie_pm2_action(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	UNUSED_PARAMETER(wlc);

	WL_RTDC(wlc, "wlc_proc_bcn: inTIM PMep=%02u AW=%02u xPS",
		(cfg->pm->PMenabled ? 10 : 0) | cfg->pm->PMpending,
		(PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));
	wlc_set_pmstate(cfg, FALSE);

	/* Reset PM2 Rx Pkts Count */
	cfg->pm->pm2_rx_pkts_since_bcn = 0;
	/* use pm2_bcn_sleep_ret_time instead
	 * of pm2_sleep_ret_time
	 */
	if (cfg->pm->pm2_bcn_sleep_ret_time)
		cfg->pm->pm2_sleep_ret_time_left =
			cfg->pm->pm2_bcn_sleep_ret_time;

#ifdef WL_PWRSTATS
	if (PWRSTATS_ENAB(wlc->pub))
		wlc_pwrstats_set_frts_data(wlc->pwrstats, TRUE);
#endif /* WL_PWRSTATS */

	wlc_pm2_sleep_ret_timer_start(cfg);
	/* Start the receive throttle timer to limit how
	 * long we can receive data before returning to
	 * PS mode.
	 */
	if (PM2_RCV_DUR_ENAB(cfg)) {
		wlc_pm2_rcv_timer_start(cfg);
	}
}

/* TIM */
static int
wlc_bcn_parse_tim_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_pm_st_t *pm = cfg->pm;
	wlc_wme_t *wme = cfg->wme;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->bcn.scb;
	bool intim;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;

	if (!cfg->BSS)
		return BCME_OK;

	if (pm->PMblocked)
		return BCME_OK;

	intim = wlc_InTIM(cfg, (bcm_tlv_t *)data->ie, data->ie_len);

	/* Check the TIM to see if data is buffered for us...
	 * else if PSpoll was sent, then cancel it
	 */
	if (intim) {
		/* Fast PM mode: leave PS mode */
		if (pm->PM == PM_FAST) {
			wlc_bcn_tim_ie_pm2_action(ctx, cfg);
		}
		/* WMM/APSD 3.6.2.3: Don't send PS poll if all ACs are
		 * delivery-enabled.
		 * wlc->PMenabled? Probably not due to 1->0 transition
		 */
		else if (!WME_ENAB(wlc->pub) || !wme->wme_apsd ||
		         (scb != NULL && scb->apsd.ac_delv != AC_BITMAP_ALL)) {
			if (pm->PMenabled) {
				if (wlc_sendpspoll(wlc, cfg) == FALSE) {
					WL_ERROR(("wl%d: %s: sendpspoll() failed\n",
					          wlc->pub->unit, __FUNCTION__));
				}
			} else {
				/* we do not have PS enabled but AP thinks
				 * we do (it has the tim bit set),
				 * so send a null frame to bring in sync
				 */
				wlc_sendnulldata(wlc, cfg, &cfg->BSSID, 0, 0, -1,
					NULL, NULL);
				WL_PS(("wl%d.%d: unexpected tim bit set, send null frame\n",
				       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
			}
		}
		/* Send APSD trigger frames to get the buffered
		 * packets
		 */
		else if (wme->apsd_auto_trigger) {

			/* clear the scb flag to force transmit a trigger frame */
			if (pm->PMenabled) {
				if (scb != NULL)
					scb->flags &= ~SCB_SENT_APSD_TRIG;
				wlc_apsd_trigger_timeout(cfg);
			}

			/* force core awake if above trigger frame succeeded */
			wlc_set_ps_ctrl(cfg);
		}
	}
	else {
		/* If periodic ps-poll is enabled, ignore beacon with no TIM
		 * bit set since the pspoll and beacon may cross paths and the
		 * AP will attempt to send a data frame after we go to sleep.
		 */
		if (pm->PSpoll && pm->pspoll_prd == 0) {
			WL_PS(("wl%d.%d: PS-Poll timeout...go to sleep (%d)\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
			       STAY_AWAKE(wlc)));
			wlc_set_pspoll(cfg, FALSE);
		}

		if (PM2_RCV_DUR_ENAB(cfg) && pm->PM == PM_FAST &&
		    pm->pm2_rcv_state == PM2RD_WAIT_BCN) {
			WL_RTDC(wlc, "wlc_proc_bcn: !inTIM WBCN PMep=%02u AW=%02u",
			        (pm->PMenabled ? 10 : 0) | pm->PMpending,
			        (PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));
			wlc_pm2_rcv_reset(cfg);
		}
	}

	/* Forcing quick return to sleep if no data buffered in AP */
	if (SCAN_IN_PROGRESS(wlc->scan) &&
	    (pm->PM == PM_FAST) &&
	    wlc_roam_scan_islazy(wlc, cfg, TRUE)) {
		pm->pm2_sleep_ret_time_left = MAX(pm->pm2_bcn_sleep_ret_time,
			cfg->current_bss->beacon_period / 8);
		wlc_pm2_sleep_ret_timer_start(cfg);
	}

	return BCME_OK;
}

/* Parse TIM IE for bcast/mcast to defer watch dog */
static int
wlc_bcn_parse_tim_ie_wd(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_pm_st_t *pm = cfg->pm;
	bcm_tlv_t *tim_ie = (bcm_tlv_t *)data->ie;

	if (!BSSCFG_STA(cfg) || !cfg->BSS)
		return BCME_OK;

	if (cfg->associated && pm->PM != PM_OFF &&
		wlc->pub->align_wd_tbtt) {

		wlc->wd_run_flag = TRUE;

		if (tim_ie == NULL ||
			tim_ie->len < DOT11_MNG_TIM_FIXED_LEN ||
			(tim_ie->data[DOT11_MNG_TIM_DTIM_COUNT] >=
			tim_ie->data[DOT11_MNG_TIM_DTIM_PERIOD])) {
			/* AP has problem; don't defer WD in that case */
			return BCME_ERROR;
		}

		/* bcast/mcast traffic indication is set in beacon; defer WD */
		if (!tim_ie->data[DOT11_MNG_TIM_DTIM_COUNT] &&
			tim_ie->data[DOT11_MNG_TIM_BITMAP_CTL] & 0x1) {

			 wlc->wd_run_flag = FALSE;
		}
	}
	return BCME_OK;
}
#endif /* STA */

/* register Bcn/PrbRsp IE mgmt handlers */
int
BCMATTACHFN(wlc_bcn_register_iem_fns)(wlc_info_t *wlc)
{
	wlc_iem_info_t *iemi = wlc->iemi;
	int err = BCME_OK;
	uint16 fstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);

	/* calc/build */
	/* bcn/prbrsp */
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_SSID_ID,
	      wlc_bcn_calc_ssid_ie_len, wlc_bcn_write_ssid_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, ssid ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_RATES_ID,
	      wlc_bcn_calc_sup_rates_ie_len, wlc_bcn_write_sup_rates_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, sup rates ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_EXT_RATES_ID,
	      wlc_bcn_calc_ext_rates_ie_len, wlc_bcn_write_ext_rates_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, ext rates ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_DS_PARMS_ID,
	      wlc_bcn_calc_ds_parms_ie_len, wlc_bcn_write_ds_parms_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, err %d, ds parms ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#ifdef STA
	if ((err = wlc_iem_add_build_fn_mft(iemi, fstbmp, DOT11_MNG_IBSS_PARMS_ID,
	      wlc_bcn_calc_ibss_parms_ie_len, wlc_bcn_write_ibss_parms_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, err %d, ibss parms ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#endif // endif
	/* parse */
#ifdef STA
	if ((err = wlc_iem_add_parse_fn(iemi, FC_BEACON, DOT11_MNG_TIM_ID,
	                                wlc_bcn_parse_tim_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, err %d, tim ie in bcn\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	/* Register call back for bcast/mcast to defer watch dog */
	if ((err = wlc_iem_add_parse_fn(iemi, FC_BEACON, DOT11_MNG_TIM_ID,
	                                wlc_bcn_parse_tim_ie_wd, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, err %d, tim ie in bcn for wd\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#endif /* STA */

	return BCME_OK;

fail:
	return err;
}

/* SSID IE */
static uint
wlc_prq_calc_ssid_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	return TLV_HDR_LEN + ftcbparm->prbreq.ssid_len;
}

static int
wlc_prq_write_ssid_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	bcm_write_tlv(DOT11_MNG_SSID_ID, ftcbparm->prbreq.ssid,
		ftcbparm->prbreq.ssid_len, data->buf);

	return BCME_OK;
}

/* Supported Rates IE */
static uint
wlc_prq_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	return TLV_HDR_LEN + ftcbparm->prbreq.sup->count;
}

static int
wlc_prq_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	bcm_write_tlv(DOT11_MNG_RATES_ID, ftcbparm->prbreq.sup->rates,
		ftcbparm->prbreq.sup->count, data->buf);

	return BCME_OK;
}

/* Extended Supported Rates IE */
static uint
wlc_prq_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->prbreq.ext->count == 0) {
		return 0;
	}

	return TLV_HDR_LEN + ftcbparm->prbreq.ext->count;
}

static int
wlc_prq_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	bcm_write_tlv(DOT11_MNG_EXT_RATES_ID, ftcbparm->prbreq.ext->rates,
		ftcbparm->prbreq.ext->count, data->buf);

	return BCME_OK;
}

/* DS Parameters */
static uint
wlc_prq_calc_ds_parms_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	if (!CHSPEC_IS2G(wlc->chanspec))
		return 0;

	return TLV_HDR_LEN + 1;
}

static int
wlc_prq_write_ds_parms_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	uint8 chan;

	if (!CHSPEC_IS2G(wlc->chanspec))
		return BCME_OK;

	chan = wf_chspec_ctlchan(wlc->chanspec);

	bcm_write_tlv(DOT11_MNG_DS_PARMS_ID, &chan, 1, data->buf);

	return BCME_OK;
}

/* register PrbReq IE mgmt handlers */
int
BCMATTACHFN(wlc_prq_register_iem_fns)(wlc_info_t *wlc)
{
	wlc_iem_info_t *iemi = wlc->iemi;
	int err = BCME_OK;

	/* calc/build */
	/* prbreq */
	if ((err = wlc_iem_add_build_fn(iemi, FC_PROBE_REQ, DOT11_MNG_SSID_ID,
	      wlc_prq_calc_ssid_ie_len, wlc_prq_write_ssid_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, ssid in prbreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn(iemi, FC_PROBE_REQ, DOT11_MNG_RATES_ID,
	      wlc_prq_calc_sup_rates_ie_len, wlc_prq_write_sup_rates_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, sup rates in prbreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn(iemi, FC_PROBE_REQ, DOT11_MNG_EXT_RATES_ID,
	      wlc_prq_calc_ext_rates_ie_len, wlc_prq_write_ext_rates_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, ext rates in prbreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_build_fn(iemi, FC_PROBE_REQ, DOT11_MNG_DS_PARMS_ID,
	      wlc_prq_calc_ds_parms_ie_len, wlc_prq_write_ds_parms_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, err %d, ds parms in prbreq\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

/* Challenge Text */
static uint
wlc_auth_calc_chlng_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->auth.alg == DOT11_SHARED_KEY) {
		uint8 *chlng;

		switch (ftcbparm->auth.seq) {
#ifdef AP
		case 2:
			return TLV_HDR_LEN + DOT11_CHALLENGE_LEN;
#endif // endif
		case 3:
			chlng = ftcbparm->auth.challenge;
			ASSERT(chlng != NULL);

			return TLV_HDR_LEN + chlng[TLV_LEN_OFF];
		}
	}

	return 0;
}

static int
wlc_auth_write_chlng_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (ftcbparm->auth.alg == DOT11_SHARED_KEY) {
		uint8 *chlng;
		uint16 status;

		status = DOT11_SC_SUCCESS;

		switch (ftcbparm->auth.seq) {
#ifdef AP
		case 2: {
			wlc_info_t *wlc = (wlc_info_t *)ctx;
			wlc_bsscfg_t *cfg = data->cfg;
			struct scb *scb;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
			uint i;

			scb = ftcbparm->auth.scb;
			ASSERT(scb != NULL);

			if (cfg->WPA_auth != WPA_AUTH_DISABLED) {
				WL_ERROR(("wl%d: %s: unhandled algo Shared Key from %s\n",
				          wlc->pub->unit, __FUNCTION__,
				          bcm_ether_ntoa(&scb->ea, eabuf)));
				status = DOT11_SC_AUTH_MISMATCH;
				break;
			}

			if (scb->challenge != NULL) {
				MFREE(wlc->osh, scb->challenge, 2 + scb->challenge[1]);
				scb->challenge = NULL;
			}

			/* create the challenge text */
			if ((chlng = MALLOC(wlc->osh, 2 + DOT11_CHALLENGE_LEN)) == NULL) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				status = DOT11_SC_FAILURE;
				break;
			}
			chlng[0] = DOT11_MNG_CHALLENGE_ID;
			chlng[1] = DOT11_CHALLENGE_LEN;
			for (i = 0; i < DOT11_CHALLENGE_LEN; i++) {
				uint16 rand = R_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_random);
				chlng[i+2] = (uint8)rand;
			}

			/* write to frame */
			bcopy(chlng, data->buf, 2 + DOT11_CHALLENGE_LEN);
#ifdef BCMDBG
			if (WL_ASSOC_ON()) {
				prhex("Auth challenge text #2", chlng, 2 + DOT11_CHALLENGE_LEN);
			}
#endif // endif
			scb->challenge = chlng;
			break;
		}
#endif /* AP */
#ifdef STA
		case 3:
			chlng = ftcbparm->auth.challenge;
			ASSERT(chlng != NULL);

			/* write to frame */
			bcopy(chlng, data->buf, 2 + chlng[1]);
#ifdef BCMDBG
			if (WL_ASSOC_ON()) {
				prhex("Auth challenge text #3", chlng, 2 + chlng[1]);
			}
#endif // endif
			break;
#endif /* STA */
		default:
			(void)chlng;
			break;
		}

		ftcbparm->auth.status = status;
	}

	return BCME_OK;
}

static int
wlc_auth_parse_chlng_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (ftpparm->auth.alg == DOT11_SHARED_KEY) {
		wlc_info_t *wlc = (wlc_info_t *)ctx;
		uint8 *chlng = data->ie;
		uint16 status;

		status = DOT11_SC_SUCCESS;

		switch (ftpparm->auth.seq) {
#ifdef STA
		case 2:
			/* What else we need to do in addition to passing it out? */
			if (chlng == NULL) {
				WL_ASSOC(("wl%d: no WEP Auth Challenge\n", wlc->pub->unit));
				status = DOT11_SC_AUTH_CHALLENGE_FAIL;
				break;
			}
			ftpparm->auth.challenge = chlng;
			break;
#endif /* STA */
#ifdef AP
		case 3: {
			wlc_bsscfg_t *cfg = data->cfg;
			int idx = WLC_BSSCFG_IDX(cfg);
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
			struct scb *scb;

			scb = ftpparm->auth.scb;
			ASSERT(scb != NULL);

			/* Check length */
			if (data->ie_len != TLV_HDR_LEN + DOT11_CHALLENGE_LEN) {
				WL_ASSOC(("wl%d: wrong length WEP Auth Challenge from %s\n",
				          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
				status = DOT11_SC_AUTH_CHALLENGE_FAIL;
				goto cleanup;
			}

			/* No Challenge Text */
			if (chlng == NULL) {
				WL_ASSOC(("wl%d: no WEP Auth Challenge from %s\n",
				          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
				status = DOT11_SC_AUTH_CHALLENGE_FAIL;
				goto cleanup;
			}

			/* Failed Challenge Text comparison */
			if (bcmp(&scb->challenge[2], &chlng[2], chlng[1]) != 0) {
				WL_ERROR(("wl%d: failed verify WEP Auth Challenge from %s\n",
				          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
				wlc_scb_clearstatebit_bsscfg(scb, AUTHENTICATED, idx);
				status = DOT11_SC_AUTH_CHALLENGE_FAIL;
				goto cleanup;
			}

			WL_ASSOC(("wl%d: WEP Auth Challenge success from %s\n",
			          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
			wlc_scb_setstatebit_bsscfg(scb, AUTHENTICATED, idx);
			scb->auth_alg = DOT11_SHARED_KEY;

		cleanup:
			/* free the challenge text */
			MFREE(wlc->osh, scb->challenge, 2 + scb->challenge[1]);
			scb->challenge = NULL;
			break;
		}
#endif /* AP */
		default:
			(void)wlc;
			(void)chlng;
			break;
		}

		ftpparm->auth.status = status;
		if (status != DOT11_SC_SUCCESS) {
			WL_INFORM(("wl%d: %s: signal to stop parsing IEs, status %u\n",
			           wlc->pub->unit, __FUNCTION__, status));
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}

/* register Auth IE mgmt handlers */
int
BCMATTACHFN(wlc_auth_register_iem_fns)(wlc_info_t *wlc)
{
	wlc_iem_info_t *iemi = wlc->iemi;
	int err = BCME_OK;

	/* calc/build */
	if ((err = wlc_iem_add_build_fn(iemi, FC_AUTH, DOT11_MNG_CHALLENGE_ID,
	      wlc_auth_calc_chlng_ie_len, wlc_auth_write_chlng_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, err %d, chlng in auth\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	/* parse */
	if ((err = wlc_iem_add_parse_fn(iemi, FC_AUTH, DOT11_MNG_CHALLENGE_ID,
	      wlc_auth_parse_chlng_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, err %d, chlng in auth\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

static int
wlc_scan_parse_ssid_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (data->ie == NULL) {
		WL_INFORM(("Missing SSID info in beacon\n"));
		bi->SSID_len = 0;
	}
	else if (data->ie[TLV_LEN_OFF] > DOT11_MAX_SSID_LEN) {
		WL_INFORM(("long SSID in beacon\n"));
		bi->SSID_len = 0;
	}
	else {
		bi->SSID_len = data->ie[TLV_LEN_OFF];
		bcopy(&data->ie[TLV_BODY_OFF], bi->SSID, bi->SSID_len);
	}

	return BCME_OK;
}

static int
wlc_scan_parse_sup_rates_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	/* Check for a legacy 54G bcn/proberesp by looking for more than 8 rates
	 * in the Supported Rates elt
	 */
	if (data->ie != NULL &&
	    data->ie[TLV_LEN_OFF] > 8)
		bi->flags |= WLC_BSS_54G;

	return BCME_OK;
}

static int
wlc_scan_parse_ibss_parms_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	/* IBSS parameters */
	if (data->ie != NULL &&
	    data->ie[TLV_LEN_OFF] >= DOT11_MNG_IBSS_PARAM_LEN)
		bi->atim_window = ltoh16_ua(&data->ie[TLV_BODY_OFF]);

	return BCME_OK;
}

static int
wlc_scan_parse_wme_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	/* WME parameters */
	if (WME_ENAB(wlc->pub)) {
		bcm_tlv_t *wme_ie = (bcm_tlv_t *)data->ie;

		if (wme_ie != NULL) {
			bi->flags |= WLC_BSS_WME;
			bi->wme_qosinfo = ((wme_ie_t *)wme_ie->data)->qosinfo;
		}
	}

	return BCME_OK;
}

static int
wlc_scan_parse_brcm_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	/* Is the AP from Broadcom */
	if (data->ie != NULL)
		bi->flags |= WLC_BSS_BRCM;

	return BCME_OK;
}

static int
wlc_scan_parse_ext_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (data->ie != NULL) {
		dot11_extcap_ie_t *extcap_ie_tlv = (dot11_extcap_ie_t *)data->ie;
		dot11_extcap_t *cap;
		cap = (dot11_extcap_t*)extcap_ie_tlv->cap;
		if (extcap_ie_tlv->len >= CEIL(DOT11_EXT_CAP_OPER_MODE_NOTIF, 8)) {
			if (isset(cap->extcap, DOT11_EXT_CAP_OPER_MODE_NOTIF)) {
				bi->flags2 |= WLC_BSS_OPER_MODE;
			}
		}
	}
	return BCME_OK;
}

/* register Bcn/Prbrsp IE mgmt handlers */
int
BCMATTACHFN(wlc_scan_register_iem_fns)(wlc_info_t *wlc)
{
	wlc_iem_info_t *iemi = wlc->iemi;
	int err = BCME_OK;
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);

	/* parse */
	if ((err = wlc_iem_add_parse_fn_mft(iemi, scanfstbmp, DOT11_MNG_EXT_CAP_ID,
	      wlc_scan_parse_ext_cap_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, err %d, ext cap ie\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_parse_fn_mft(iemi, scanfstbmp, DOT11_MNG_SSID_ID,
	                                    wlc_scan_parse_ssid_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, err %d, ssid in scan\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_parse_fn_mft(iemi, scanfstbmp, DOT11_MNG_RATES_ID,
	                                    wlc_scan_parse_sup_rates_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, err %d, sup rates in scan\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_add_parse_fn_mft(iemi, scanfstbmp, DOT11_MNG_IBSS_PARMS_ID,
	                                    wlc_scan_parse_ibss_parms_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, err %d, ibss parms in scan\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_vs_add_parse_fn_mft(iemi, scanfstbmp, WLC_IEM_VS_IE_PRIO_WME,
	                                    wlc_scan_parse_wme_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, err %d, wme in scan\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
	if ((err = wlc_iem_vs_add_parse_fn_mft(iemi, scanfstbmp, WLC_IEM_VS_IE_PRIO_BRCM,
	                                    wlc_scan_parse_brcm_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, err %d, brcm in scan\n",
		          wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * TODO: Move above functions to their own modules when possible.
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */
