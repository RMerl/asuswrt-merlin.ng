/*
 * wpa_supplicant - WPA/RSN IE and KDE definitions
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WPA_IE_H
#define WPA_IE_H

struct wpa_sm;

struct wpa_eapol_ie_parse {
	const u8 *wpa_ie;
	size_t wpa_ie_len;
	const u8 *rsn_ie;
	size_t rsn_ie_len;
	const u8 *pmkid;
	const u8 *gtk;
	size_t gtk_len;
	const u8 *mac_addr;
	size_t mac_addr_len;
#ifdef CONFIG_IEEE80211W
	const u8 *igtk;
	size_t igtk_len;
#endif /* CONFIG_IEEE80211W */
	const u8 *mdie;
	size_t mdie_len;
	const u8 *ftie;
	size_t ftie_len;
	const u8 *reassoc_deadline;
	const u8 *key_lifetime;
	const u8 *lnkid;
	size_t lnkid_len;
	const u8 *ext_capab;
	size_t ext_capab_len;
	const u8 *supp_rates;
	size_t supp_rates_len;
	const u8 *ext_supp_rates;
	size_t ext_supp_rates_len;
	const u8 *ht_capabilities;
	const u8 *vht_capabilities;
	const u8 *supp_channels;
	size_t supp_channels_len;
	const u8 *supp_oper_classes;
	size_t supp_oper_classes_len;
	u8 qosinfo;
	u16 aid;
	const u8 *wmm;
	size_t wmm_len;
#ifdef CONFIG_P2P
	const u8 *ip_addr_req;
	const u8 *ip_addr_alloc;
#endif /* CONFIG_P2P */
#ifdef CONFIG_OCV
	const u8 *oci;
	size_t oci_len;
#endif /* CONFIG_OCV */
};

int wpa_supplicant_parse_ies(const u8 *buf, size_t len,
			     struct wpa_eapol_ie_parse *ie);
int wpa_gen_wpa_ie(struct wpa_sm *sm, u8 *wpa_ie, size_t wpa_ie_len);

#endif /* WPA_IE_H */
