/*
 * The WIFI HAL lib for MLO/MLD.
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */
#ifndef _NL_MLD_H_
#define _NL_MLD_H_

#ifdef MODULE
#define WIFI_ERR(fmt, arg...) \
do { \
	if (wl_dbg_level & WL_DBG_ERR) { \
		printf(fmt, ##arg); \
	} \
}	while (0)

#define WIFI_INFO(fmt, arg...) \
do { \
	if (wl_dbg_level & WL_DBG_INFO) { \
		printf(fmt, ##arg); \
	} \
}	while (0)

#define WIFI_DBG(fmt, arg...) \
do { \
	if (wl_dbg_level & WL_DBG_DBG) { \
		printf(fmt, ##arg); \
	} \
}	while (0)
#endif /* MODULE */

/* 802.11be MLO/MLD support */
/*
 * In wlconf, max radio is WL_MAX_RADIO and max mld_unit is WL_MAXMLDUNIT(== WL_MAXBSSCFG).
 * In wlc_mlo, max mld unit is MLO_MAX_MLD(== WLC_MAXBSSCFG).
 *
 * CMWIFI: the max radio is MAX_WLAN_ADAPTER and max bsscfg is WL_MAX_NUM_SSID(See cmwifi.mk).
 */
#define WL_MAXMLDUNIT				(WL_MAX_NUM_SSID)
#define WL_MAX_MLO_LINKS			(3)		/* cf. wlconf.c */

typedef struct _mld_config {
	int flags;
	int enable;
	int mld_unit;
}	mld_config_t;
#define MLD_CFG_FLAG_ERR			(0x02)		/* Error */
#define MLD_CFG_FLAG_SET			(0x01)		/* Changed and correct */

#ifdef MODULE
extern int wlhal_mlo_init(void);
extern int wlhal_mld_ndev_reg(struct net_device *ndev, int wl_unit, int wl_subunit);
extern int wlhal_mld_cfg_set(int wl_unit, int wl_subunit, int mld_enable, int mld_unit);
extern int wlhal_mld_mac_set(int mld_unit, char *mld_mac);
extern int wlhal_mlo_apply(void);
extern void wlhal_mlo_dump(void);
#endif /* MODULE */

#endif /* _NL_MLD_H_ */
