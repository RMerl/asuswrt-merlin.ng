/*
 * local defines for wpapsk supplicant and authenticator
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
 * $Id: wlc_wpa.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_wpa_h_
#define _wlc_wpa_h_

#include <bcmcrypto/passhash.h>
#include <proto/eapol.h>

/* WPA key_info flag sets */
#define PMSG1_REQUIRED	  (WPA_KEY_PAIRWISE | WPA_KEY_ACK)
#define PMSG1_PROHIBITED  (WPA_KEY_SECURE | WPA_KEY_MIC | WPA_KEY_INSTALL)
#define PMSG2_REQUIRED	  (WPA_KEY_PAIRWISE | WPA_KEY_MIC)
#define PMSG2_PROHIBITED  (WPA_KEY_ACK | WPA_KEY_INDEX_MASK)
#define PMSG3_REQUIRED	  (WPA_KEY_PAIRWISE | WPA_KEY_MIC | WPA_KEY_ACK | WPA_KEY_INSTALL)
#define PMSG3_BRCM_REQUIRED	(PMSG3_REQUIRED | WPA_KEY_SECURE)
#define PMSG3_WPA2_REQUIRED	(PMSG3_REQUIRED | WPA_KEY_SECURE | WPA_KEY_ENCRYPTED_DATA)
#define PMSG3_PROHIBITED  (WPA_KEY_SECURE)
#define PMSG4_REQUIRED	  (WPA_KEY_MIC)
#define PMSG4_PROHIBITED  (WPA_KEY_ACK | WPA_KEY_INDEX_MASK)
#define GMSG1_REQUIRED	  (WPA_KEY_SECURE | WPA_KEY_MIC | WPA_KEY_ACK)
#define GMSG2_REQUIRED	  (WPA_KEY_MIC | WPA_KEY_SECURE)
#define MIC_ERROR_REQUIRED (WPA_KEY_MIC | WPA_KEY_ERROR | WPA_KEY_REQ)

/* Spec says some key_info flags in supplicant response should match what
 * authenticator had in previous message.  Define masks to copy those.
 */
#define PMSG2_MATCH_FLAGS (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2 |		\
			   WPA_KEY_PAIRWISE | WPA_KEY_INDEX_MASK |	\
			   WPA_KEY_SECURE | WPA_KEY_ERROR | WPA_KEY_REQ)
#define PMSG4_MATCH_FLAGS (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2 |		\
			   WPA_KEY_PAIRWISE | WPA_KEY_SECURE)
#define GMSG2_MATCH_FLAGS (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2 | WPA_KEY_PAIRWISE)

typedef enum {
	/* Supplicant States */
	WPA_SUP_DISCONNECTED,
	WPA_SUP_INITIALIZE,
	WPA_SUP_AUTHENTICATION,
	WPA_SUP_STAKEYSTARTP_WAIT_M1 = WPA_SUP_AUTHENTICATION,
	                                /* 4-way handshake: waiting for msg M1 */
	WPA_SUP_STAKEYSTARTP_PREP_M2,	/* 4-way handshake: preparing to send M2 */
	WPA_SUP_STAKEYSTARTP_WAIT_M3,	/* 4-way handshake: waiting for M3 */
	WPA_SUP_STAKEYSTARTP_PREP_M4,	/* 4-way handshake: preparing to send M4 */
	WPA_SUP_STAKEYSTARTG_WAIT_G1,	/* group handshake: waiting for G1 */
	WPA_SUP_STAKEYSTARTG_PREP_G2,	/* group handshake: preparing to send G2 */
	WPA_SUP_KEYUPDATE,		/* handshake complete, keys updated */

	/* Authenticator States */
	WPA_AUTH_INITIALIZE,
	WPA_AUTH_PTKSTART,
	WPA_AUTH_PTKINITNEGOTIATING,
	WPA_AUTH_PTKINITDONE,
	/* for WPA1 group key state machine */
	WPA_AUTH_REKEYNEGOTIATING,
	WPA_AUTH_KEYERROR,
	WPA_AUTH_REKEYESTABLISHED,
	WPA_AUTH_KEYUPDATE		/* handshake complete, keys updated */
} wpapsk_state_t;

typedef struct {
	wpapsk_state_t state;	/* state of WPA PSK key msg exchanges */
	uint16 auth_wpaie_len;	/* length of authenticator's WPA info element */
	uint16 sup_wpaie_len;	/* length of supplicant's WPA info element */
	uchar *auth_wpaie;	/* authenticator's WPA info element */
	uchar *sup_wpaie;	/* supplicant's WPA info element */
	ushort ucipher;		/* negotiated unicast cipher */
	ushort mcipher;		/* negotiated multicast cipher */
	ushort ptk_len;		/* PTK len, used in PRF calculation */
	ushort gtk_len;		/* Group (mcast) key length */
	ushort tk_len;		/* TK len, used when loading key into driver */
	ushort desc;		/* key descriptor type */
	uint8 anonce[EAPOL_WPA_KEY_NONCE_LEN];	/* AP's nonce */
	uint8 snonce[EAPOL_WPA_KEY_NONCE_LEN];	/* STA's nonce */
	uint8 replay[EAPOL_KEY_REPLAY_LEN];	/* AP's replay counter */
	uint8 last_replay[EAPOL_KEY_REPLAY_LEN]; /* AP's last replay counter (for WOWL) */
	uint8 gtk[TKIP_KEY_SIZE];		/* group transient key */
	/* fields of WPA key hierarchy (together forming the PTK) */
	uint8 eapol_mic_key[WPA_MIC_KEY_LEN];
	uint8 eapol_encr_key[WPA_ENCR_KEY_LEN];
	uint8 temp_encr_key[WPA_TEMP_ENCR_KEY_LEN];
	uint8 temp_tx_key[WPA_TEMP_TX_KEY_LEN];
	uint8 temp_rx_key[WPA_TEMP_RX_KEY_LEN];
	uint8 gtk_index;
	uint32 WPA_auth;
#ifdef BCMAUTH_PSK
	uint8 retries;		/* retry count */
#endif /* BCMAUTH_PSK */
} wpapsk_t;

/* persistent WPA stuff (survives set_sup initialization ) */
typedef struct wpapsk_info {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	/* break lengthy passhash() calculation into smaller chunks */
	struct wl_timer *passhash_timer; /* timer for passhash */
	passhash_t passhash_states;	/* states for passhash */
#ifdef BCMAUTH_PSK
	struct wl_timer *retry_timer;	/* auth retry timer */
#endif /* BCMAUTH_PSK */
	/* IMPORTANT : keep config info below used; in config get/set */
	ushort psk_len;			/* len of pre-shared key */
	ushort pmk_len;			/* len of pairwise master key */
	ushort pmk_psk_len; 		/* len of pairwise master key */
	uchar  psk[WSEC_MAX_PSK_LEN];	/* saved pre-shared key */
	uint8 pmk[PMK_LEN];		/* saved pairwise master key */
	uint8 pmk_psk[PMK_LEN]; 	/* PMK derived from PSK, used for driver based roaming */
} wpapsk_info_t;

typedef enum {
	PMSG1, PMSG2, PMSG3, PMSG4, GMSG1, GMSG2, GMSG_REKEY, MIC_FAILURE
} wpa_msg_t;

extern void wlc_wpapsk_free(wlc_info_t *wlc, wpapsk_t *wpa);
extern bool wlc_wpapsk_start(wlc_info_t *wlc, wpapsk_t *wpa, uint8 *sup_ies,
	uint sup_ies_len, uint8 *auth_ies, uint auth_ies_len);
extern int wlc_wpa_cobble_pmk(wpapsk_info_t *info, char *psk, size_t psk_len,
	uchar *ssid, uint ssid_len);
extern bool wlc_wpa_set_ucipher(wpapsk_t *wpa, ushort ucipher, bool wep_ok);
extern int wlc_wpa_set_pmk(wlc_bsscfg_t *bsscfg, wpapsk_info_t *info,
	wpapsk_t *wpa, wsec_pmk_t *pmk, bool assoc);
extern void wlc_wpa_plumb_tk(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *tk,
	uint32 tk_len, uint32 cipher, struct ether_addr *ea);
extern void *wlc_eapol_pktget(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct ether_addr *da, uint len);
extern uint32 wlc_wpa_plumb_gtk(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *gtk, uint32 gtk_len,
	uint32 key_index, uint32 cipher, uint8 *rsc, bool primary_key);

/* send deauthentication */
extern void wlc_wpa_senddeauth(wlc_bsscfg_t *bsscfg, char *da, int reason);

typedef struct _sup_pmkid {
	struct ether_addr	BSSID;
	uint8			PMKID[WPA2_PMKID_LEN];
	uint8			PMK[PMK_LEN];
	bool			opportunistic;
} sup_pmkid_t;

#define SUP_MAXPMKID	16 /* Supplementary Max PMK ID */

#endif	/* _wlc_wpa_h_ */
