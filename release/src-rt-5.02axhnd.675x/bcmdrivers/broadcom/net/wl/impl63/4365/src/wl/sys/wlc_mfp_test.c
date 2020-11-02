/*
 * Broadcom 802.11 Networking Device Driver
 * Management frame protection
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
 * $Id: wlc_mfp_test.c 708017 2017-06-29 14:11:45Z $
 */

#if defined(MFP_TEST)

#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>

#include <bcmcrypto/prf.h>
#include <proto/eap.h>
#include <proto/eapol.h>
#include <proto/wpa.h>

#include <wlc_types.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_assoc.h>
#include <wlc_frmutil.h>

#include <wl_export.h>

#include <wlc_mfp.h>
#include <wlc_mfp_test.h>

/* iovar support */

static void* mfp_bypass_send_sa_query(wlc_info_t *wlc,
	struct scb *scb, uint8 action, uint16 id);
static void* mfp_send_disassoc_deauth(wlc_info_t *wlc,
	const struct ether_addr *da, const struct ether_addr *bssid,
	const struct ether_addr *sa, struct scb *scb,
	uint16 fc, uint16 reason_code, int flag);
static void mfp_disassoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	int flag, int flag2);
static void mfp_deauth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	int flag, int flag2);
static void mfp_assoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag);
static void mfp_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag);
static void mfp_reassoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag);
static void mfp_bip_test(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag);

#ifdef STA
#define mfp_sendassocreq wlc_sendassocreq
#else
#define mfp_sendassocreq(wlc, bss_info, scb, reassoc) WL_WSEC((\
	"wl%d: %s: supported  only when STA is configured for the driver\n", \
	WLCWLUNIT(wlc), __FUNCTION__));
#endif // endif

/* end prototypes */

/* iovar callback */
int
mfp_test_doiovar(wlc_info_t *wlc, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *params, uint p_len, void *arg,
	int len, int val_size, struct wlc_if *wlcif)
{
	int err = BCME_OK;
	wlc_bsscfg_t *bsscfg;
	int32 *ret_int_ptr;
	int32 flag;

	if (wlc->mfp == NULL) {
		err =  BCME_UNSUPPORTED;
		goto done;
	}

	ret_int_ptr = (int32*)arg;
	flag = *ret_int_ptr;
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {
	case IOV_SVAL(IOV_MFP_TEST_SA_QUERY): {
		uint8 action;
		uint16 id;
		struct scb *scb;
		struct scb_iter scbiter;

		char *inbuf = (char *) arg;
		memcpy(&action, inbuf + OFFSETOF(wl_sa_query_t, action), sizeof(action));
		memcpy(&id, inbuf + OFFSETOF(wl_sa_query_t, id), sizeof(id));
		memcpy(&flag, inbuf + OFFSETOF(wl_sa_query_t, flag), sizeof(flag));

		/* Token needs to be non-zero, so burn the high bit */
		if (id == 0)
			id = (uint16)(wlc->counter | 0x8000);

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if (SCB_ASSOCIATED(scb)) {
				if (flag != MFP_TEST_FLAG_NORMAL) {
					if (flag == MFP_TEST_FLAG_ANY_KEY) {
						wlc_key_t *key;
						wlc_key_info_t key_info;
						key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
							WLC_KEY_ID_PAIRWISE,
							WLC_KEY_FLAG_NONE, &key_info);
						err = wlc_key_set_flags(key,
							key_info.flags |
							WLC_KEY_FLAG_GEN_MFP_ACT_ERR);
					}
					mfp_test_send_sa_query(wlc, scb, action, id);
				} else {
					mfp_bypass_send_sa_query(wlc, scb, action, id);
				}
			}
		}
		break;
	}
	case IOV_SVAL(IOV_MFP_TEST_DISASSOC): {
		int32 flag2;
		char *inbuf = (char *) arg;
		memcpy(&flag2, inbuf+sizeof(int32), sizeof(flag2));
		mfp_disassoc(wlc, bsscfg, flag, flag2);
		break;
	}
	case IOV_SVAL(IOV_MFP_TEST_DEAUTH): {
		int32 flag2;
		char *inbuf = (char *) arg;
		memcpy(&flag2, inbuf+sizeof(int32), sizeof(flag2));
		mfp_deauth(wlc, bsscfg, flag, flag2);
		break;
	}
	case IOV_SVAL(IOV_MFP_TEST_ASSOC):
		mfp_assoc(wlc, bsscfg, flag);
		break;
	case IOV_SVAL(IOV_MFP_TEST_AUTH):
		mfp_auth(wlc, bsscfg, flag);
		break;
	case IOV_SVAL(IOV_MFP_TEST_REASSOC):
		mfp_reassoc(wlc, bsscfg, flag);
		break;
	case IOV_SVAL(IOV_MFP_TEST_BIP):
		mfp_bip_test(wlc, bsscfg, flag);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
done:
	return err;
}

static void*
mfp_test_frame_get_mgmt_dbg(wlc_info_t *wlc, uint16 fc, uint8 cat,
	const struct ether_addr *da, const struct ether_addr *sa,
	const struct ether_addr *bssid, uint body_len, uint8 **pbody)
{
	void *p = wlc_mfp_frame_get_mgmt(wlc->mfp, fc, cat, da, sa,
		bssid, body_len, pbody);
	if (p != NULL)
		WLPKTTAG(p)->flags &= ~WLF_MFP; /* bypass encryption */
	return p;
}

static void *
mfp_send_disassoc_deauth(wlc_info_t *wlc, const struct ether_addr *da,
	const struct ether_addr *bssid, const struct ether_addr *sa,
	struct scb *scb, uint16 fc, uint16 reason_code, int flag)
{
	void *p = NULL;
	uint8 *pbody;
	uint16 *reason;
	wlc_bsscfg_t *bsscfg;
	wlc_key_t *key;
	wlc_key_info_t key_info;

	ASSERT(fc == FC_DISASSOC || fc == FC_DEAUTH);

	WL_ASSOC(("wl%d: %s ...\n",  WLCWLUNIT(wlc), __FUNCTION__));
	bsscfg = wlc_bsscfg_find_by_bssid(wlc, bssid);

	/*
	 * get a packet - disassoc pkt has the following contents:
	 * 2 bytes Reason Code
	 */
	if (BSSCFG_AP(bsscfg) && ETHER_ISMULTI(da)) {
		uint body_len = 2;
		if (flag)
			p = wlc_mfp_frame_get_mgmt(wlc->mfp, fc, 0, da, sa,
				bssid, body_len, &pbody);
		else
			p = mfp_test_frame_get_mgmt_dbg(wlc, fc, 0, da, sa,
				bssid, 2, &pbody);
	}
	else if (flag) {
		if (flag == 1) {
			int err;
			do {
				key = wlc_keymgmt_get_key_by_addr(wlc->keymgmt, bsscfg, da,
					WLC_KEY_FLAG_NONE, &key_info);
				err = wlc_key_set_flags(key, key_info.flags | (fc == FC_DISASSOC ?
					WLC_KEY_FLAG_GEN_MFP_DISASSOC_ERR :
					WLC_KEY_FLAG_GEN_MFP_DEAUTH_ERR));
				if (err != BCME_OK)
					break;
			} while (0);
		}
		p = wlc_mfp_frame_get_mgmt(wlc->mfp, fc, 0, da, sa, bssid, 2, &pbody);
	}
	else {
		p = mfp_test_frame_get_mgmt_dbg(wlc, fc, 0, da, sa, bssid, 2, &pbody);
	}

	if (p == NULL)
		return p;

	/* fill out the disassociation reason code */
	reason = (uint16 *) pbody;
	reason[0] = htol16(reason_code);

	if (flag && BSSCFG_AP(bsscfg) && ETHER_ISMULTI(da)) {
		if (flag == 1) {
			/* send frame with bad mic */
			key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, TRUE, &key_info);
			wlc_key_set_flags(key, key_info.flags | WLC_KEY_FLAG_GEN_ICV_ERR);
		}
	}
	if (wlc_sendmgmt(wlc, p, bsscfg->wlcif->qi, scb))
		return p;

	WL_ASSOC(("wl%d: %s exit\n",  WLCWLUNIT(wlc), __FUNCTION__));
	return NULL;
}

static void*
mfp_bypass_send_sa_query(wlc_info_t *wlc, struct scb *scb,
	uint8 action, uint16 id)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_action_sa_query *af;

	body_len = sizeof(struct dot11_action_sa_query);

	/* bypass normal path so no encryption is used */
	if ((p = mfp_test_frame_get_mgmt_dbg(wlc, FC_ACTION,
		DOT11_ACTION_CAT_SA_QUERY, &scb->ea, &scb->bsscfg->cur_etheraddr,
		&scb->bsscfg->BSSID, body_len, &pbody)) == NULL)
		return p;

	af = (struct dot11_action_sa_query *)pbody;
	af->category = DOT11_ACTION_CAT_SA_QUERY;
	af->action = action;
	af->id = id;

	wlc_sendmgmt(wlc, p, scb->bsscfg->wlcif->qi, scb);
	return p;
}

static void
mfp_disassoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag, int flag2)
{
	struct scb *scb;
	if (ETHER_ISNULLADDR(bsscfg->BSSID.octet))
		return;

	if (BSSCFG_AP(bsscfg)) {
		if (flag2) {
			WL_WSEC(("wl: %s: send bcast disassoc\n", __FUNCTION__));
			mfp_send_disassoc_deauth(wlc, (struct ether_addr*)&ether_bcast,
				&bsscfg->BSSID, &bsscfg->cur_etheraddr,
				NULL, FC_DISASSOC, DOT11_RC_DISASSOC_LEAVING, flag);
		}
		else {
			struct scb_iter scbiter;
			WL_WSEC(("wl: %s: send unicast disassoc\n", __FUNCTION__));
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
				if (SCB_ASSOCIATED(scb)) {
					mfp_send_disassoc_deauth(wlc, &scb->ea, &bsscfg->BSSID,
						&bsscfg->cur_etheraddr, scb,
						FC_DISASSOC, DOT11_RC_DISASSOC_LEAVING, flag);
				}
			}
		}
	} else {
		scb = wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID);
		ASSERT(scb != NULL);
		ASSERT(SCB_ASSOCIATED(scb));

		WL_WSEC(("wl: %s: send unicast disassoc\n", __FUNCTION__));
		mfp_send_disassoc_deauth(wlc, &bsscfg->BSSID, &bsscfg->BSSID,
			&bsscfg->cur_etheraddr, scb,
			FC_DISASSOC, DOT11_RC_DISASSOC_LEAVING, flag);
	}
}

static void
mfp_deauth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag, int flag2)
{
	struct scb *scb;
	if (ETHER_ISNULLADDR(bsscfg->BSSID.octet))
		return;

	if (BSSCFG_AP(bsscfg)) {
		if (flag2) {
			WL_WSEC(("wl: %s: send bcast deauth\n", __FUNCTION__));
			mfp_send_disassoc_deauth(wlc, (struct ether_addr*)&ether_bcast,
				&bsscfg->BSSID, &bsscfg->cur_etheraddr,
				NULL, FC_DEAUTH, DOT11_RC_INACTIVITY, flag);
		}
		else {
			struct scb_iter scbiter;
			WL_WSEC(("wl: %s: send unicast deauth\n", __FUNCTION__));
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
				if (SCB_ASSOCIATED(scb)) {
					mfp_send_disassoc_deauth(wlc, &scb->ea, &bsscfg->BSSID,
						&bsscfg->cur_etheraddr, scb,
						FC_DEAUTH, DOT11_RC_INACTIVITY, flag);
				}
			}
		}

		return;
	}

	scb = wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID);
	ASSERT(scb != NULL);
	ASSERT(SCB_ASSOCIATED(scb));

	if (flag == 2)
		wlc_senddeauth(wlc, bsscfg, scb, &bsscfg->BSSID, &bsscfg->BSSID,
		               &bsscfg->cur_etheraddr, DOT11_RC_INACTIVITY);
	else
		mfp_send_disassoc_deauth(wlc, &bsscfg->BSSID, &bsscfg->BSSID,
		                         &bsscfg->cur_etheraddr, scb,
		                         FC_DEAUTH, DOT11_RC_INACTIVITY, flag);
}

static void
mfp_assoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag)
{
	struct scb *scb;

	if (ETHER_ISNULLADDR(bsscfg->BSSID.octet))
		return;

	scb = wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID);
	ASSERT(scb != NULL);
	ASSERT(SCB_ASSOCIATED(scb));
	mfp_sendassocreq(wlc, bsscfg->target_bss, scb, FALSE);
}

static void
mfp_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag)
{
	struct scb *scb;

	if (ETHER_ISNULLADDR(bsscfg->BSSID.octet))
		return;

	scb = wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID);
	ASSERT(scb != NULL);
	ASSERT(SCB_ASSOCIATED(scb));

	wlc_sendauth(bsscfg, &scb->ea, &bsscfg->target_bss->BSSID, scb,
		DOT11_OPEN_SYSTEM, 1, DOT11_SC_SUCCESS, NULL, DOT11_CAP_SHORT);
}

static void
mfp_reassoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag)
{
	struct scb *scb;

	if (ETHER_ISNULLADDR(bsscfg->BSSID.octet))
		return;

	scb = wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID);
	ASSERT(scb != NULL);
	ASSERT(SCB_ASSOCIATED(scb));
	mfp_sendassocreq(wlc, bsscfg->target_bss, scb, TRUE);
}

static void
mfp_bip_mic(const struct dot11_management_header *hdr,
    const uint8 *body, int body_len, const uint8 *key, uint8 * mic, size_t mic_len)
{
	uint data_len = 0;
	uchar micdata[256];
	uint16 fc;

	ASSERT(body_len >= BIP_MIC_SIZE);

	/* calc mic */
	fc = htol16(ltoh16(hdr->fc) & ~(FC_RETRY | FC_PM | FC_MOREDATA));
	memcpy((char *)&micdata[data_len], (uint8 *)&fc, 2);
	data_len += 2;
	memcpy((char *)&micdata[data_len], (uint8 *)&hdr->da, ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	memcpy((char *)&micdata[data_len], (uint8 *)&hdr->sa, ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	memcpy((char *)&micdata[data_len], (uint8 *)&hdr->bssid, ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;

	ASSERT((data_len + body_len - BIP_MIC_SIZE) <= sizeof(micdata));

	/* copy body without mic */
	memcpy(&micdata[data_len], body, body_len - BIP_MIC_SIZE);
	data_len += body_len - BIP_MIC_SIZE;
	memset(&micdata[data_len], 0, BIP_MIC_SIZE);
	data_len += BIP_MIC_SIZE;

	aes_cmac_calc(micdata, data_len, key, BIP_KEY_SIZE, mic, mic_len);
}

static void
mfp_bip_test(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int flag)
{
	uint8  mic[AES_BLOCK_SZ];
	uint8 frame[] = {0xc0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 9, 0, 2, 0,
		0x4c, 0x10, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x48, 0xdf, 0xbf, 0xa7, 0xb8, 0x27, 0x88, 0x72};
	uint8 igtk[] = {0x4e, 0xa9, 0x54, 0x3e, 0x09, 0xcf, 0x2b, 0x1e, 0xca, 0x66,
		0xff, 0xc5, 0x8b, 0xde, 0xcb, 0xcf};
	uint8 target_mic[] = {0x48, 0xdf, 0xbf, 0xa7, 0xb8, 0x27, 0x88, 0x72};
	struct dot11_management_header *hdr = (struct dot11_management_header *)frame;

	/* check mic for frame */
	mfp_bip_mic(hdr, (void *)(frame + sizeof(*hdr)), 20, igtk, mic, BIP_MIC_SIZE);
	if (!bcmp(mic, target_mic, BIP_MIC_SIZE))
		printf("bip mic test succcess\n");
	else
		printf("bip mic test failed\n");
}

#endif /* MFP_TEST */
