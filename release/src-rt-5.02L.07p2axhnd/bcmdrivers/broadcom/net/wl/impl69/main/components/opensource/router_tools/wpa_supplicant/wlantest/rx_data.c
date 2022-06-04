/*
 * Received Data frame processing
 * Copyright (c) 2010-2015, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "common/defs.h"
#include "common/ieee802_11_defs.h"
#include "wlantest.h"

static const char * data_stype(u16 stype)
{
	switch (stype) {
	case WLAN_FC_STYPE_DATA:
		return "DATA";
	case WLAN_FC_STYPE_DATA_CFACK:
		return "DATA-CFACK";
	case WLAN_FC_STYPE_DATA_CFPOLL:
		return "DATA-CFPOLL";
	case WLAN_FC_STYPE_DATA_CFACKPOLL:
		return "DATA-CFACKPOLL";
	case WLAN_FC_STYPE_NULLFUNC:
		return "NULLFUNC";
	case WLAN_FC_STYPE_CFACK:
		return "CFACK";
	case WLAN_FC_STYPE_CFPOLL:
		return "CFPOLL";
	case WLAN_FC_STYPE_CFACKPOLL:
		return "CFACKPOLL";
	case WLAN_FC_STYPE_QOS_DATA:
		return "QOSDATA";
	case WLAN_FC_STYPE_QOS_DATA_CFACK:
		return "QOSDATA-CFACK";
	case WLAN_FC_STYPE_QOS_DATA_CFPOLL:
		return "QOSDATA-CFPOLL";
	case WLAN_FC_STYPE_QOS_DATA_CFACKPOLL:
		return "QOSDATA-CFACKPOLL";
	case WLAN_FC_STYPE_QOS_NULL:
		return "QOS-NULL";
	case WLAN_FC_STYPE_QOS_CFPOLL:
		return "QOS-CFPOLL";
	case WLAN_FC_STYPE_QOS_CFACKPOLL:
		return "QOS-CFACKPOLL";
	}
	return "??";
}

static void rx_data_eth(struct wlantest *wt, const u8 *bssid,
			const u8 *sta_addr, const u8 *dst, const u8 *src,
			u16 ethertype, const u8 *data, size_t len, int prot,
			const u8 *peer_addr);

static void rx_data_vlan(struct wlantest *wt, const u8 *bssid,
			 const u8 *sta_addr, const u8 *dst, const u8 *src,
			 const u8 *data, size_t len, int prot,
			 const u8 *peer_addr)
{
	u16 tag;

	if (len < 4)
		return;
	tag = WPA_GET_BE16(data);
	wpa_printf(MSG_MSGDUMP, "VLAN tag: Priority=%u ID=%u",
		   tag >> 12, tag & 0x0ffff);
	/* ignore VLAN information and process the original frame */
	rx_data_eth(wt, bssid, sta_addr, dst, src, WPA_GET_BE16(data + 2),
		    data + 4, len - 4, prot, peer_addr);
}

static void rx_data_eth(struct wlantest *wt, const u8 *bssid,
			const u8 *sta_addr, const u8 *dst, const u8 *src,
			u16 ethertype, const u8 *data, size_t len, int prot,
			const u8 *peer_addr)
{
	switch (ethertype) {
	case ETH_P_PAE:
		rx_data_eapol(wt, bssid, sta_addr, dst, src, data, len, prot);
		break;
	case ETH_P_IP:
		rx_data_ip(wt, bssid, sta_addr, dst, src, data, len,
			   peer_addr);
		break;
	case 0x890d:
		rx_data_80211_encap(wt, bssid, sta_addr, dst, src, data, len);
		break;
	case ETH_P_8021Q:
		rx_data_vlan(wt, bssid, sta_addr, dst, src, data, len, prot,
			     peer_addr);
		break;
	}
}

static void rx_data_process(struct wlantest *wt, const u8 *bssid,
			    const u8 *sta_addr,
			    const u8 *dst, const u8 *src,
			    const u8 *data, size_t len, int prot,
			    const u8 *peer_addr)
{
	if (len == 0)
		return;

	if (len >= 8 && os_memcmp(data, "\xaa\xaa\x03\x00\x00\x00", 6) == 0) {
		rx_data_eth(wt, bssid, sta_addr, dst, src,
			    WPA_GET_BE16(data + 6), data + 8, len - 8, prot,
			    peer_addr);
		return;
	}

	wpa_hexdump(MSG_DEBUG, "Unrecognized LLC", data, len > 8 ? 8 : len);
}

static u8 * try_all_ptk(struct wlantest *wt, int pairwise_cipher,
			const struct ieee80211_hdr *hdr,
			const u8 *data, size_t data_len, size_t *decrypted_len)
{
	struct wlantest_ptk *ptk;
	u8 *decrypted;
	int prev_level = wpa_debug_level;

	wpa_debug_level = MSG_WARNING;
	dl_list_for_each(ptk, &wt->ptk, struct wlantest_ptk, list) {
		unsigned int tk_len = ptk->ptk_len - 32;
		decrypted = NULL;
		if ((pairwise_cipher == WPA_CIPHER_CCMP ||
		     pairwise_cipher == 0) && tk_len == 16) {
			decrypted = ccmp_decrypt(ptk->ptk.tk, hdr, data,
						 data_len, decrypted_len);
		} else if ((pairwise_cipher == WPA_CIPHER_CCMP_256 ||
			    pairwise_cipher == 0) && tk_len == 32) {
			decrypted = ccmp_256_decrypt(ptk->ptk.tk, hdr, data,
						     data_len, decrypted_len);
		} else if ((pairwise_cipher == WPA_CIPHER_GCMP ||
			    pairwise_cipher == WPA_CIPHER_GCMP_256 ||
			    pairwise_cipher == 0) &&
			   (tk_len == 16 || tk_len == 32)) {
			decrypted = gcmp_decrypt(ptk->ptk.tk, tk_len, hdr,
						 data, data_len, decrypted_len);
		} else if ((pairwise_cipher == WPA_CIPHER_TKIP ||
			    pairwise_cipher == 0) && tk_len == 32) {
			decrypted = tkip_decrypt(ptk->ptk.tk, hdr, data,
						 data_len, decrypted_len);
		}
		if (decrypted) {
			wpa_debug_level = prev_level;
			add_note(wt, MSG_DEBUG, "Found PTK match from list of all known PTKs");
			return decrypted;
		}
	}
	wpa_debug_level = prev_level;

	return NULL;
}

static void rx_data_bss_prot_group(struct wlantest *wt,
				   const struct ieee80211_hdr *hdr,
				   size_t hdrlen,
				   const u8 *qos, const u8 *dst, const u8 *src,
				   const u8 *data, size_t len)
{
	struct wlantest_bss *bss;
	int keyid;
	u8 *decrypted = NULL;
	size_t dlen;
	u8 pn[6];
	int replay = 0;

	bss = bss_get(wt, hdr->addr2);
	if (bss == NULL)
		return;
	if (len < 4) {
		add_note(wt, MSG_INFO, "Too short group addressed data frame");
		return;
	}

	if (bss->group_cipher & (WPA_CIPHER_TKIP | WPA_CIPHER_CCMP) &&
	    !(data[3] & 0x20)) {
		add_note(wt, MSG_INFO, "Expected TKIP/CCMP frame from "
			 MACSTR " did not have ExtIV bit set to 1",
			 MAC2STR(bss->bssid));
		return;
	}

	if (bss->group_cipher == WPA_CIPHER_TKIP) {
		if (data[3] & 0x1f) {
			add_note(wt, MSG_INFO, "TKIP frame from " MACSTR
				 " used non-zero reserved bit",
				 MAC2STR(bss->bssid));
		}
		if (data[1] != ((data[0] | 0x20) & 0x7f)) {
			add_note(wt, MSG_INFO, "TKIP frame from " MACSTR
				 " used incorrect WEPSeed[1] (was 0x%x, "
				 "expected 0x%x)",
				 MAC2STR(bss->bssid), data[1],
				 (data[0] | 0x20) & 0x7f);
		}
	} else if (bss->group_cipher == WPA_CIPHER_CCMP) {
		if (data[2] != 0 || (data[3] & 0x1f) != 0) {
			add_note(wt, MSG_INFO, "CCMP frame from " MACSTR
				 " used non-zero reserved bit",
				 MAC2STR(bss->bssid));
		}
	}

	keyid = data[3] >> 6;
	if (bss->gtk_len[keyid] == 0 && bss->group_cipher != WPA_CIPHER_WEP40)
	{
		add_note(wt, MSG_MSGDUMP, "No GTK known to decrypt the frame "
			 "(A2=" MACSTR " KeyID=%d)",
			 MAC2STR(hdr->addr2), keyid);
		return;
	}

	if (bss->group_cipher == WPA_CIPHER_TKIP)
		tkip_get_pn(pn, data);
	else if (bss->group_cipher == WPA_CIPHER_WEP40)
		goto skip_replay_det;
	else
		ccmp_get_pn(pn, data);
	if (os_memcmp(pn, bss->rsc[keyid], 6) <= 0) {
		u16 seq_ctrl = le_to_host16(hdr->seq_ctrl);
		add_note(wt, MSG_INFO, "CCMP/TKIP replay detected: A1=" MACSTR
			 " A2=" MACSTR " A3=" MACSTR " seq=%u frag=%u%s",
			 MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			 MAC2STR(hdr->addr3),
			 WLAN_GET_SEQ_SEQ(seq_ctrl),
			 WLAN_GET_SEQ_FRAG(seq_ctrl),
			 (le_to_host16(hdr->frame_control) & WLAN_FC_RETRY) ?
			 " Retry" : "");
		wpa_hexdump(MSG_INFO, "RX PN", pn, 6);
		wpa_hexdump(MSG_INFO, "RSC", bss->rsc[keyid], 6);
		replay = 1;
	}

skip_replay_det:
	if (bss->group_cipher == WPA_CIPHER_TKIP)
		decrypted = tkip_decrypt(bss->gtk[keyid], hdr, data, len,
					 &dlen);
	else if (bss->group_cipher == WPA_CIPHER_WEP40)
		decrypted = wep_decrypt(wt, hdr, data, len, &dlen);
	else if (bss->group_cipher == WPA_CIPHER_CCMP)
		decrypted = ccmp_decrypt(bss->gtk[keyid], hdr, data, len,
					 &dlen);
	else if (bss->group_cipher == WPA_CIPHER_CCMP_256)
		decrypted = ccmp_256_decrypt(bss->gtk[keyid], hdr, data, len,
					     &dlen);
	else if (bss->group_cipher == WPA_CIPHER_GCMP ||
		 bss->group_cipher == WPA_CIPHER_GCMP_256)
		decrypted = gcmp_decrypt(bss->gtk[keyid], bss->gtk_len[keyid],
					 hdr, data, len, &dlen);

	if (decrypted) {
		rx_data_process(wt, bss->bssid, NULL, dst, src, decrypted,
				dlen, 1, NULL);
		if (!replay)
			os_memcpy(bss->rsc[keyid], pn, 6);
		write_pcap_decrypted(wt, (const u8 *) hdr, hdrlen,
				     decrypted, dlen);
	} else
		add_note(wt, MSG_DEBUG, "Failed to decrypt frame");
	os_free(decrypted);
}

static void rx_data_bss_prot(struct wlantest *wt,
			     const struct ieee80211_hdr *hdr, size_t hdrlen,
			     const u8 *qos, const u8 *dst, const u8 *src,
			     const u8 *data, size_t len)
{
	struct wlantest_bss *bss, *bss2;
	struct wlantest_sta *sta, *sta2;
	int keyid;
	u16 fc = le_to_host16(hdr->frame_control);
	u8 *decrypted;
	size_t dlen;
	int tid;
	u8 pn[6], *rsc;
	struct wlantest_tdls *tdls = NULL, *found;
	const u8 *tk = NULL;
	int ptk_iter_done = 0;
	int try_ptk_iter = 0;
	int replay = 0;

	if (hdr->addr1[0] & 0x01) {
		rx_data_bss_prot_group(wt, hdr, hdrlen, qos, dst, src,
				       data, len);
		return;
	}

	if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) ==
	    (WLAN_FC_TODS | WLAN_FC_FROMDS)) {
		bss = bss_find(wt, hdr->addr1);
		if (bss) {
			sta = sta_find(bss, hdr->addr2);
			if (sta) {
				sta->counters[
					WLANTEST_STA_COUNTER_PROT_DATA_TX]++;
			}
			if (!sta || !sta->ptk_set) {
				bss2 = bss_find(wt, hdr->addr2);
				if (bss2) {
					sta2 = sta_find(bss2, hdr->addr1);
					if (sta2 && (!sta || sta2->ptk_set)) {
						bss = bss2;
						sta = sta2;
					}
				}
			}
		} else {
			bss = bss_find(wt, hdr->addr2);
			if (!bss)
				return;
			sta = sta_find(bss, hdr->addr1);
		}
	} else if (fc & WLAN_FC_TODS) {
		bss = bss_get(wt, hdr->addr1);
		if (bss == NULL)
			return;
		sta = sta_get(bss, hdr->addr2);
		if (sta)
			sta->counters[WLANTEST_STA_COUNTER_PROT_DATA_TX]++;
	} else if (fc & WLAN_FC_FROMDS) {
		bss = bss_get(wt, hdr->addr2);
		if (bss == NULL)
			return;
		sta = sta_get(bss, hdr->addr1);
	} else {
		bss = bss_get(wt, hdr->addr3);
		if (bss == NULL)
			return;
		sta = sta_find(bss, hdr->addr2);
		sta2 = sta_find(bss, hdr->addr1);
		if (sta == NULL || sta2 == NULL)
			return;
		found = NULL;
		dl_list_for_each(tdls, &bss->tdls, struct wlantest_tdls, list)
		{
			if ((tdls->init == sta && tdls->resp == sta2) ||
			    (tdls->init == sta2 && tdls->resp == sta)) {
				found = tdls;
				if (tdls->link_up)
					break;
			}
		}
		if (found) {
			if (!found->link_up)
				add_note(wt, MSG_DEBUG,
					 "TDLS: Link not up, but Data "
					 "frame seen");
			tk = found->tpk.tk;
			tdls = found;
		}
	}
	if ((sta == NULL ||
	     (!sta->ptk_set && sta->pairwise_cipher != WPA_CIPHER_WEP40)) &&
	    tk == NULL) {
		add_note(wt, MSG_MSGDUMP, "No PTK known to decrypt the frame");
		if (dl_list_empty(&wt->ptk))
			return;
		try_ptk_iter = 1;
	}

	if (len < 4) {
		add_note(wt, MSG_INFO, "Too short encrypted data frame");
		return;
	}

	if (sta == NULL)
		return;
	if (sta->pairwise_cipher & (WPA_CIPHER_TKIP | WPA_CIPHER_CCMP) &&
	    !(data[3] & 0x20)) {
		add_note(wt, MSG_INFO, "Expected TKIP/CCMP frame from "
			 MACSTR " did not have ExtIV bit set to 1",
			 MAC2STR(src));
		return;
	}

	if (tk == NULL && sta->pairwise_cipher == WPA_CIPHER_TKIP) {
		if (data[3] & 0x1f) {
			add_note(wt, MSG_INFO, "TKIP frame from " MACSTR
				 " used non-zero reserved bit",
				 MAC2STR(hdr->addr2));
		}
		if (data[1] != ((data[0] | 0x20) & 0x7f)) {
			add_note(wt, MSG_INFO, "TKIP frame from " MACSTR
				 " used incorrect WEPSeed[1] (was 0x%x, "
				 "expected 0x%x)",
				 MAC2STR(hdr->addr2), data[1],
				 (data[0] | 0x20) & 0x7f);
		}
	} else if (tk || sta->pairwise_cipher == WPA_CIPHER_CCMP) {
		if (data[2] != 0 || (data[3] & 0x1f) != 0) {
			add_note(wt, MSG_INFO, "CCMP frame from " MACSTR
				 " used non-zero reserved bit",
				 MAC2STR(hdr->addr2));
		}
	}

	keyid = data[3] >> 6;
	if (keyid != 0) {
		add_note(wt, MSG_INFO, "Unexpected non-zero KeyID %d in "
			 "individually addressed Data frame from " MACSTR,
			 keyid, MAC2STR(hdr->addr2));
	}

	if (qos) {
		tid = qos[0] & 0x0f;
		if (fc & WLAN_FC_TODS)
			sta->tx_tid[tid]++;
		else
			sta->rx_tid[tid]++;
	} else {
		tid = 0;
		if (fc & WLAN_FC_TODS)
			sta->tx_tid[16]++;
		else
			sta->rx_tid[16]++;
	}
	if (tk) {
		if (os_memcmp(hdr->addr2, tdls->init->addr, ETH_ALEN) == 0)
			rsc = tdls->rsc_init[tid];
		else
			rsc = tdls->rsc_resp[tid];
	} else if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) ==
		   (WLAN_FC_TODS | WLAN_FC_FROMDS)) {
		if (os_memcmp(sta->addr, hdr->addr2, ETH_ALEN) == 0)
			rsc = sta->rsc_tods[tid];
		else
			rsc = sta->rsc_fromds[tid];
	} else if (fc & WLAN_FC_TODS)
		rsc = sta->rsc_tods[tid];
	else
		rsc = sta->rsc_fromds[tid];

	if (tk == NULL && sta->pairwise_cipher == WPA_CIPHER_TKIP)
		tkip_get_pn(pn, data);
	else if (sta->pairwise_cipher == WPA_CIPHER_WEP40)
		goto skip_replay_det;
	else
		ccmp_get_pn(pn, data);
	if (os_memcmp(pn, rsc, 6) <= 0) {
		u16 seq_ctrl = le_to_host16(hdr->seq_ctrl);
		add_note(wt, MSG_INFO, "CCMP/TKIP replay detected: A1=" MACSTR
			 " A2=" MACSTR " A3=" MACSTR " seq=%u frag=%u%s",
			 MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			 MAC2STR(hdr->addr3),
			 WLAN_GET_SEQ_SEQ(seq_ctrl),
			 WLAN_GET_SEQ_FRAG(seq_ctrl),
			 (le_to_host16(hdr->frame_control) &  WLAN_FC_RETRY) ?
			 " Retry" : "");
		wpa_hexdump(MSG_INFO, "RX PN", pn, 6);
		wpa_hexdump(MSG_INFO, "RSC", rsc, 6);
		replay = 1;
	}

skip_replay_det:
	if (tk) {
		if (sta->pairwise_cipher == WPA_CIPHER_CCMP_256)
			decrypted = ccmp_256_decrypt(tk, hdr, data, len, &dlen);
		else if (sta->pairwise_cipher == WPA_CIPHER_GCMP ||
			 sta->pairwise_cipher == WPA_CIPHER_GCMP_256)
			decrypted = gcmp_decrypt(tk, sta->ptk.tk_len, hdr, data,
						 len, &dlen);
		else
			decrypted = ccmp_decrypt(tk, hdr, data, len, &dlen);
	} else if (sta->pairwise_cipher == WPA_CIPHER_TKIP) {
		decrypted = tkip_decrypt(sta->ptk.tk, hdr, data, len, &dlen);
	} else if (sta->pairwise_cipher == WPA_CIPHER_WEP40) {
		decrypted = wep_decrypt(wt, hdr, data, len, &dlen);
	} else if (sta->ptk_set) {
		if (sta->pairwise_cipher == WPA_CIPHER_CCMP_256)
			decrypted = ccmp_256_decrypt(sta->ptk.tk, hdr, data,
						     len, &dlen);
		else if (sta->pairwise_cipher == WPA_CIPHER_GCMP ||
			 sta->pairwise_cipher == WPA_CIPHER_GCMP_256)
			decrypted = gcmp_decrypt(sta->ptk.tk, sta->ptk.tk_len,
						 hdr, data, len, &dlen);
		else
			decrypted = ccmp_decrypt(sta->ptk.tk, hdr, data, len,
						 &dlen);
	} else {
		decrypted = try_all_ptk(wt, sta->pairwise_cipher, hdr, data,
					len, &dlen);
		ptk_iter_done = 1;
	}
	if (!decrypted && !ptk_iter_done) {
		decrypted = try_all_ptk(wt, sta->pairwise_cipher, hdr, data,
					len, &dlen);
		if (decrypted) {
			add_note(wt, MSG_DEBUG, "Current PTK did not work, but found a match from all known PTKs");
		}
	}
	if (decrypted) {
		u16 fc = le_to_host16(hdr->frame_control);
		const u8 *peer_addr = NULL;
		if (!(fc & (WLAN_FC_FROMDS | WLAN_FC_TODS)))
			peer_addr = hdr->addr1;
		if (!replay)
			os_memcpy(rsc, pn, 6);
		rx_data_process(wt, bss->bssid, sta->addr, dst, src, decrypted,
				dlen, 1, peer_addr);
		write_pcap_decrypted(wt, (const u8 *) hdr, hdrlen,
				     decrypted, dlen);
	} else {
		if (!try_ptk_iter)
			add_note(wt, MSG_DEBUG, "Failed to decrypt frame");

		/* Assume the frame was corrupted and there was no FCS to check.
		 * Allow retry of this particular frame to be processed so that
		 * it could end up getting decrypted if it was received without
		 * corruption. */
		sta->allow_duplicate = 1;
	}
	os_free(decrypted);
}

static void rx_data_bss(struct wlantest *wt, const struct ieee80211_hdr *hdr,
			size_t hdrlen, const u8 *qos, const u8 *dst,
			const u8 *src, const u8 *data, size_t len)
{
	u16 fc = le_to_host16(hdr->frame_control);
	int prot = !!(fc & WLAN_FC_ISWEP);

	if (qos) {
		u8 ack = (qos[0] & 0x60) >> 5;
		wpa_printf(MSG_MSGDUMP, "BSS DATA: " MACSTR " -> " MACSTR
			   " len=%u%s tid=%u%s%s",
			   MAC2STR(src), MAC2STR(dst), (unsigned int) len,
			   prot ? " Prot" : "", qos[0] & 0x0f,
			   (qos[0] & 0x10) ? " EOSP" : "",
			   ack == 0 ? "" :
			   (ack == 1 ? " NoAck" :
			    (ack == 2 ? " NoExpAck" : " BA")));
	} else {
		wpa_printf(MSG_MSGDUMP, "BSS DATA: " MACSTR " -> " MACSTR
			   " len=%u%s",
			   MAC2STR(src), MAC2STR(dst), (unsigned int) len,
			   prot ? " Prot" : "");
	}

	if (prot)
		rx_data_bss_prot(wt, hdr, hdrlen, qos, dst, src, data, len);
	else {
		const u8 *bssid, *sta_addr, *peer_addr;
		struct wlantest_bss *bss;

		if (fc & WLAN_FC_TODS) {
			bssid = hdr->addr1;
			sta_addr = hdr->addr2;
			peer_addr = NULL;
		} else if (fc & WLAN_FC_FROMDS) {
			bssid = hdr->addr2;
			sta_addr = hdr->addr1;
			peer_addr = NULL;
		} else {
			bssid = hdr->addr3;
			sta_addr = hdr->addr2;
			peer_addr = hdr->addr1;
		}

		bss = bss_get(wt, bssid);
		if (bss) {
			struct wlantest_sta *sta = sta_get(bss, sta_addr);

			if (sta) {
				if (qos) {
					int tid = qos[0] & 0x0f;
					if (fc & WLAN_FC_TODS)
						sta->tx_tid[tid]++;
					else
						sta->rx_tid[tid]++;
				} else {
					if (fc & WLAN_FC_TODS)
						sta->tx_tid[16]++;
					else
						sta->rx_tid[16]++;
				}
			}
		}

		rx_data_process(wt, bssid, sta_addr, dst, src, data, len, 0,
				peer_addr);
	}
}

static struct wlantest_tdls * get_tdls(struct wlantest *wt, const u8 *bssid,
				       const u8 *sta1_addr,
				       const u8 *sta2_addr)
{
	struct wlantest_bss *bss;
	struct wlantest_sta *sta1, *sta2;
	struct wlantest_tdls *tdls, *found = NULL;

	bss = bss_find(wt, bssid);
	if (bss == NULL)
		return NULL;
	sta1 = sta_find(bss, sta1_addr);
	if (sta1 == NULL)
		return NULL;
	sta2 = sta_find(bss, sta2_addr);
	if (sta2 == NULL)
		return NULL;

	dl_list_for_each(tdls, &bss->tdls, struct wlantest_tdls, list) {
		if ((tdls->init == sta1 && tdls->resp == sta2) ||
		    (tdls->init == sta2 && tdls->resp == sta1)) {
			found = tdls;
			if (tdls->link_up)
				break;
		}
	}

	return found;
}

static void add_direct_link(struct wlantest *wt, const u8 *bssid,
			    const u8 *sta1_addr, const u8 *sta2_addr)
{
	struct wlantest_tdls *tdls;

	tdls = get_tdls(wt, bssid, sta1_addr, sta2_addr);
	if (tdls == NULL)
		return;

	if (tdls->link_up)
		tdls->counters[WLANTEST_TDLS_COUNTER_VALID_DIRECT_LINK]++;
	else
		tdls->counters[WLANTEST_TDLS_COUNTER_INVALID_DIRECT_LINK]++;
}

static void add_ap_path(struct wlantest *wt, const u8 *bssid,
			const u8 *sta1_addr, const u8 *sta2_addr)
{
	struct wlantest_tdls *tdls;

	tdls = get_tdls(wt, bssid, sta1_addr, sta2_addr);
	if (tdls == NULL)
		return;

	if (tdls->link_up)
		tdls->counters[WLANTEST_TDLS_COUNTER_INVALID_AP_PATH]++;
	else
		tdls->counters[WLANTEST_TDLS_COUNTER_VALID_AP_PATH]++;
}

void rx_data(struct wlantest *wt, const u8 *data, size_t len)
{
	const struct ieee80211_hdr *hdr;
	u16 fc, stype;
	size_t hdrlen;
	const u8 *qos = NULL;

	if (len < 24)
		return;

	hdr = (const struct ieee80211_hdr *) data;
	fc = le_to_host16(hdr->frame_control);
	stype = WLAN_FC_GET_STYPE(fc);
	hdrlen = 24;
	if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) ==
	    (WLAN_FC_TODS | WLAN_FC_FROMDS))
		hdrlen += ETH_ALEN;
	if (stype & 0x08) {
		qos = data + hdrlen;
		hdrlen += 2;
	}
	if (len < hdrlen)
		return;
	wt->rx_data++;

	switch (fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) {
	case 0:
		wpa_printf(MSG_EXCESSIVE, "DATA %s%s%s IBSS DA=" MACSTR " SA="
			   MACSTR " BSSID=" MACSTR,
			   data_stype(WLAN_FC_GET_STYPE(fc)),
			   fc & WLAN_FC_PWRMGT ? " PwrMgt" : "",
			   fc & WLAN_FC_ISWEP ? " Prot" : "",
			   MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			   MAC2STR(hdr->addr3));
		add_direct_link(wt, hdr->addr3, hdr->addr1, hdr->addr2);
		rx_data_bss(wt, hdr, hdrlen, qos, hdr->addr1, hdr->addr2,
			    data + hdrlen, len - hdrlen);
		break;
	case WLAN_FC_FROMDS:
		wpa_printf(MSG_EXCESSIVE, "DATA %s%s%s FromDS DA=" MACSTR
			   " BSSID=" MACSTR " SA=" MACSTR,
			   data_stype(WLAN_FC_GET_STYPE(fc)),
			   fc & WLAN_FC_PWRMGT ? " PwrMgt" : "",
			   fc & WLAN_FC_ISWEP ? " Prot" : "",
			   MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			   MAC2STR(hdr->addr3));
		add_ap_path(wt, hdr->addr2, hdr->addr1, hdr->addr3);
		rx_data_bss(wt, hdr, hdrlen, qos, hdr->addr1, hdr->addr3,
			    data + hdrlen, len - hdrlen);
		break;
	case WLAN_FC_TODS:
		wpa_printf(MSG_EXCESSIVE, "DATA %s%s%s ToDS BSSID=" MACSTR
			   " SA=" MACSTR " DA=" MACSTR,
			   data_stype(WLAN_FC_GET_STYPE(fc)),
			   fc & WLAN_FC_PWRMGT ? " PwrMgt" : "",
			   fc & WLAN_FC_ISWEP ? " Prot" : "",
			   MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			   MAC2STR(hdr->addr3));
		add_ap_path(wt, hdr->addr1, hdr->addr3, hdr->addr2);
		rx_data_bss(wt, hdr, hdrlen, qos, hdr->addr3, hdr->addr2,
			    data + hdrlen, len - hdrlen);
		break;
	case WLAN_FC_TODS | WLAN_FC_FROMDS:
		wpa_printf(MSG_EXCESSIVE, "DATA %s%s%s WDS RA=" MACSTR " TA="
			   MACSTR " DA=" MACSTR " SA=" MACSTR,
			   data_stype(WLAN_FC_GET_STYPE(fc)),
			   fc & WLAN_FC_PWRMGT ? " PwrMgt" : "",
			   fc & WLAN_FC_ISWEP ? " Prot" : "",
			   MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			   MAC2STR(hdr->addr3),
			   MAC2STR((const u8 *) (hdr + 1)));
		rx_data_bss(wt, hdr, hdrlen, qos, hdr->addr1, hdr->addr2,
			    data + hdrlen, len - hdrlen);
		break;
	}
}
