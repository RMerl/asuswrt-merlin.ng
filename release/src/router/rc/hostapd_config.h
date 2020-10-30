/*
 * Hostapd config header file
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hostapd_config.h 789711 2020-08-05 18:04:52Z $
 */

#ifndef _hostapd_config_h_
#define _hostapd_config_h_
#include <bcmendian.h>

extern bool pbcd_swap;

#define htod32(i) (pbcd_swap ? bcmswap32(i) : (uint32)(i))
#define dtoh32(i) (pbcd_swap ? bcmswap32(i) : (uint32)(i))

extern int start_hapd_dpp_self_provision();
#ifdef HAPD_WDS
extern void hapd_wpasupp_wds_hndlr(const char *pkt);
#endif /* HAPD_WDS */
extern char *hapd_wpasupp_strncpy(char *dest, const char *src, size_t n);

extern int hapd_wpasupp_get_all_lanifname_sz(void);
extern int hapd_wpasupp_get_all_lanifname(char *ifnames, int ifnames_sz);
extern int hapd_wpasupp_get_all_lanifnames_listsz(void);
extern int hapd_wpasupp_get_all_lanifnames_list(char *ifnames_list, int ifnames_listsz);

#endif /* _hostapd_config_h_ */
