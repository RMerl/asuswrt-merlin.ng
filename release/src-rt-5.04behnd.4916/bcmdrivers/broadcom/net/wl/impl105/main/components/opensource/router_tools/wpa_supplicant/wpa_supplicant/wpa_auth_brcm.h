/*
 * wpa_supplicant / iovar setting to driver
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 */
#ifndef _wpa_auth_brcm_h_
#define _wpa_auth_brcm_h_

#include "includes.h"
#include "common.h"

#define ETHER_ADDR_LEN	6
#ifdef CONFIG_PASN
int brcm_set_secure_ranging_ctx(struct wpa_supplicant *wpa_s,
				const u8 *own_addr, const u8 *peer_addr,
				u32 cipher, u8 tk_len, const u8 *tk,
				u8 ltf_keyseed_len,
				const u8 *ltf_keyseed,
				u8 kdk_len, const u8 *kdk);
#endif /* CONFIG_PASN */
#endif /* _wpa_auth_brcm_h_ */
