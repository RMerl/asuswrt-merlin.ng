/*
 * Linux cfg80211 driver - Dongle Host Driver (DHD) related
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: wl_cfg80211.c,v 1.1.4.1.2.14 2011/02/09 01:40:07 Exp $
 */

#ifndef __DHD_CFG80211__
#define __DHD_CFG80211__

#include <wl_cfg80211.h>
#include <wl_cfgp2p.h>
#include <dhd.h>
#include <dhd_linux.h>
#include <brcm_nl80211.h>

#ifndef WL_ERR
#define WL_ERR CFG80211_ERR
#endif
#ifndef WL_TRACE
#define WL_TRACE CFG80211_TRACE
#endif

s32 dhd_cfg80211_init(struct bcm_cfg80211 *cfg);
s32 dhd_cfg80211_deinit(struct bcm_cfg80211 *cfg);
s32 dhd_cfg80211_down(struct bcm_cfg80211 *cfg);
s32 dhd_cfg80211_set_p2p_info(struct bcm_cfg80211 *cfg, int val);
s32 dhd_cfg80211_clean_p2p_info(struct bcm_cfg80211 *cfg);
s32 dhd_config_dongle(struct bcm_cfg80211 *cfg);
#ifdef WL_VENDOR_EXT_SUPPORT
int dhd_cfgvendor_priv_string_handler(struct bcm_cfg80211 *cfg,
struct wireless_dev *wdev, const struct bcm_nlmsg_hdr *nlioc, void  *data);
#endif /* WL_VENDOR_EXT_SUPPORT */
int wl_get_busstate(struct bcm_cfg80211 *cfg);
#define WL_IS_DRV_UP(dhd) (((dhd_pub_t *)dhd)->up)
#if defined(STB) && !defined(STBAP)
#if defined(BCMDBUS) && !defined(USB_SUSPEND_AVAILABLE)
void dhd_cfg80211_set_wowl(struct bcm_cfg80211 *cfg, int state);
#endif /* BCMDBUS && USB_SUSPEND_AVAILABLE */
#endif /* STB && STBAP */
#endif /* __DHD_CFG80211__ */
