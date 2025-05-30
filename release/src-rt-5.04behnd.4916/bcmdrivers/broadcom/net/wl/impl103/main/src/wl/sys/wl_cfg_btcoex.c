/*
 * Linux cfg80211 driver - Dongle Host Driver (DHD) related
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: wl_cfg_btcoex.c 821234 2023-02-06 14:16:52Z $
 */

#include <net/rtnetlink.h>

#include <bcmutils.h>
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <dhd_cfg80211.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhdioctl.h>
#include <wlioctl.h>
#include <wl_core.h>

#ifdef PKT_FILTER_SUPPORT
extern uint dhd_pkt_filter_enable;
extern uint dhd_master_mode;
extern void dhd_pktfilter_offload_enable(dhd_pub_t * dhd, char *arg, int enable, int master_mode);
#endif

struct btcoex_info {
	struct timer_list timer;
	u32 timer_ms;
	u32 timer_on;
	u32 ts_dhcp_start;	/* ms ts ecord time stats */
	u32 ts_dhcp_ok;		/* ms ts ecord time stats */
	bool dhcp_done;	/* flag, indicates that host done with
					 * dhcp before t1/t2 expiration
					 */
	s32 bt_state;
	struct work_struct work;
	struct net_device *dev;
};

#if defined(OEM_ANDROID)
static struct btcoex_info *btcoex_info_loc = NULL;

/* TODO: clean up the BT-Coex code, it still have some legacy ioctl/iovar functions */

/* use New SCO/eSCO smart YG suppression */
#define BT_DHCP_eSCO_FIX
/* this flag boost wifi pkt priority to max, caution: -not fair to sco */
#define BT_DHCP_USE_FLAGS
/* T1 start SCO/ESCo priority suppression */
#define BT_DHCP_OPPR_WIN_TIME	2500
/* T2 turn off SCO/SCO supperesion is (timeout) */
#define BT_DHCP_FLAG_FORCE_TIME 5500

enum wl_cfg80211_btcoex_status {
	BT_DHCP_IDLE,
	BT_DHCP_START,
	BT_DHCP_OPPR_WIN,
	BT_DHCP_FLAG_FORCE_TIMEOUT
};

/*
 * get named driver variable to uint register value and return error indication
 * calling example: dev_wlc_intvar_get_reg(dev, "btc_params",66, &reg_value)
 */
static int
dev_wlc_intvar_get_reg(struct net_device *dev, char *name,
	uint reg, int *retval)
{
	union {
		char buf[WLC_IOCTL_SMLEN];
		int val;
	} var;
	int error;

	bcm_mkiovar(name, (char *)(&reg), sizeof(reg),
		(char *)(&var), sizeof(var.buf));
	error = wldev_ioctl_get(dev, WLC_GET_VAR, (char *)(&var), sizeof(var.buf));

	*retval = dtoh32(var.val);
	return (error);
}

static int
dev_wlc_bufvar_set(struct net_device *dev, char *name, char *buf, int len)
{
	char ioctlbuf_local[WLC_IOCTL_SMLEN];

	bcm_mkiovar(name, buf, len, ioctlbuf_local, sizeof(ioctlbuf_local));

	return (wldev_ioctl_set(dev, WLC_SET_VAR, ioctlbuf_local, sizeof(ioctlbuf_local)));
}

/*
get named driver variable to uint register value and return error indication
calling example: dev_wlc_intvar_set_reg(dev, "btc_params",66, value)
*/
static int
dev_wlc_intvar_set_reg(struct net_device *dev, char *name, char *addr, char * val)
{
	char reg_addr[8];

	memset(reg_addr, 0, sizeof(reg_addr));
	memcpy((char *)&reg_addr[0], (char *)addr, 4);
	memcpy((char *)&reg_addr[4], (char *)val, 4);

	return (dev_wlc_bufvar_set(dev, name, (char *)&reg_addr[0], sizeof(reg_addr)));
}

static bool btcoex_is_sco_active(struct net_device *dev)
{
	int ioc_res = 0;
	bool res = FALSE;
	int sco_id_cnt = 0;
	int param27;
	int i;

	for (i = 0; i < 12; i++) {

		ioc_res = dev_wlc_intvar_get_reg(dev, "btc_params", 27, &param27);

		WL_TRACE(("sample[%d], btc params: 27:%x\n", i, param27));

		if (ioc_res < 0) {
			WL_ERR(("ioc read btc params error\n"));
			break;
		}

		if ((param27 & 0x6) == 2) { /* count both sco & esco  */
			sco_id_cnt++;
		}

		if (sco_id_cnt > 2) {
			WL_TRACE(("sco/esco detected, pkt id_cnt:%d  samples:%d\n",
				sco_id_cnt, i));
			res = TRUE;
			break;
		}

		OSL_SLEEP(5);
	}

	return res;
}

#if defined(BT_DHCP_eSCO_FIX)
/* Enhanced BT COEX settings for eSCO compatibility during DHCP window */
static int set_btc_esco_params(struct net_device *dev, bool trump_sco)
{
	static bool saved_status = FALSE;

	char buf_reg50va_dhcp_on[8] =
		{ 50, 00, 00, 00, 0x22, 0x80, 0x00, 0x00 };
	char buf_reg51va_dhcp_on[8] =
		{ 51, 00, 00, 00, 0x00, 0x00, 0x00, 0x00 };
	char buf_reg64va_dhcp_on[8] =
		{ 64, 00, 00, 00, 0x00, 0x00, 0x00, 0x00 };
	char buf_reg65va_dhcp_on[8] =
		{ 65, 00, 00, 00, 0x00, 0x00, 0x00, 0x00 };
	char buf_reg71va_dhcp_on[8] =
		{ 71, 00, 00, 00, 0x00, 0x00, 0x00, 0x00 };
	uint32 regaddr;
	static uint32 saved_reg50;
	static uint32 saved_reg51;
	static uint32 saved_reg64;
	static uint32 saved_reg65;
	static uint32 saved_reg71;

	if (trump_sco) {
		/* this should reduce eSCO agressive retransmit
		 * w/o breaking it
		 */

		/* 1st save current */
		WL_TRACE(("Do new SCO/eSCO coex algo {save &"
			  "override}\n"));
		if ((!dev_wlc_intvar_get_reg(dev, "btc_params", 50, &saved_reg50)) &&
			(!dev_wlc_intvar_get_reg(dev, "btc_params", 51, &saved_reg51)) &&
			(!dev_wlc_intvar_get_reg(dev, "btc_params", 64, &saved_reg64)) &&
			(!dev_wlc_intvar_get_reg(dev, "btc_params", 65, &saved_reg65)) &&
			(!dev_wlc_intvar_get_reg(dev, "btc_params", 71, &saved_reg71))) {
			saved_status = TRUE;
			WL_TRACE(("saved bt_params[50,51,64,65,71]:"
				  "0x%x 0x%x 0x%x 0x%x 0x%x\n",
				  saved_reg50, saved_reg51,
				  saved_reg64, saved_reg65, saved_reg71));
		} else {
			WL_ERR((":%s: save btc_params failed\n",
				__FUNCTION__));
			saved_status = FALSE;
			return -1;
		}

		WL_TRACE(("override with [50,51,64,65,71]:"
			  "0x%x 0x%x 0x%x 0x%x 0x%x\n",
			  *(u32 *)(buf_reg50va_dhcp_on+4),
			  *(u32 *)(buf_reg51va_dhcp_on+4),
			  *(u32 *)(buf_reg64va_dhcp_on+4),
			  *(u32 *)(buf_reg65va_dhcp_on+4),
			  *(u32 *)(buf_reg71va_dhcp_on+4)));

		dev_wlc_bufvar_set(dev, "btc_params",
			(char *)&buf_reg50va_dhcp_on[0], 8);
		dev_wlc_bufvar_set(dev, "btc_params",
			(char *)&buf_reg51va_dhcp_on[0], 8);
		dev_wlc_bufvar_set(dev, "btc_params",
			(char *)&buf_reg64va_dhcp_on[0], 8);
		dev_wlc_bufvar_set(dev, "btc_params",
			(char *)&buf_reg65va_dhcp_on[0], 8);
		dev_wlc_bufvar_set(dev, "btc_params",
			(char *)&buf_reg71va_dhcp_on[0], 8);

		saved_status = TRUE;
	} else if (saved_status) {
		/* restore previously saved bt params */
		WL_TRACE(("Do new SCO/eSCO coex algo {save &"
			  "override}\n"));

		regaddr = 50;
		dev_wlc_intvar_set_reg(dev, "btc_params",
			(char *)&regaddr, (char *)&saved_reg50);
		regaddr = 51;
		dev_wlc_intvar_set_reg(dev, "btc_params",
			(char *)&regaddr, (char *)&saved_reg51);
		regaddr = 64;
		dev_wlc_intvar_set_reg(dev, "btc_params",
			(char *)&regaddr, (char *)&saved_reg64);
		regaddr = 65;
		dev_wlc_intvar_set_reg(dev, "btc_params",
			(char *)&regaddr, (char *)&saved_reg65);
		regaddr = 71;
		dev_wlc_intvar_set_reg(dev, "btc_params",
			(char *)&regaddr, (char *)&saved_reg71);

		WL_TRACE(("restore bt_params[50,51,64,65,71]:"
			"0x%x 0x%x 0x%x 0x%x 0x%x\n",
			saved_reg50, saved_reg51, saved_reg64,
			saved_reg65, saved_reg71));

		saved_status = FALSE;
	} else {
		WL_ERR((":%s att to restore not saved BTCOEX params\n",
			__FUNCTION__));
		return -1;
	}
	return 0;
}
#endif /* BT_DHCP_eSCO_FIX */

static void
wl_cfg80211_bt_setflag(struct net_device *dev, bool set)
{
#if defined(BT_DHCP_USE_FLAGS)
	char buf_flag7_dhcp_on[8] = { 7, 00, 00, 00, 0x1, 0x0, 0x00, 0x00 };
	char buf_flag7_default[8]   = { 7, 00, 00, 00, 0x0, 0x00, 0x00, 0x00};
#endif

#if defined(BT_DHCP_eSCO_FIX)
	/* set = 1, save & turn on  0 - off & restore prev settings */
	set_btc_esco_params(dev, set);
#endif

#if defined(BT_DHCP_USE_FLAGS)
	WL_TRACE(("WI-FI priority boost via bt flags, set:%d\n", set));
	if (set == TRUE)
		/* Forcing bt_flag7  */
		dev_wlc_bufvar_set(dev, "btc_flags",
			(char *)&buf_flag7_dhcp_on[0],
			sizeof(buf_flag7_dhcp_on));
	else
		/* Restoring default bt flag7 */
		dev_wlc_bufvar_set(dev, "btc_flags",
			(char *)&buf_flag7_default[0],
			sizeof(buf_flag7_default));
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 85)
static void wl_cfg80211_bt_timerfunc(ulong data)
#else
static void wl_cfg80211_bt_timerfunc(struct timer_list *data)
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 85 */
{
	struct btcoex_info *bt_local = (struct btcoex_info *)data;
	WL_TRACE(("Enter\n"));
	bt_local->timer_on = 0;
	schedule_work(&bt_local->work);
}

static void wl_cfg80211_bt_handler(struct work_struct *work)
{
	struct btcoex_info *btcx_inf;

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	btcx_inf = container_of(work, struct btcoex_info, work);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

	if (btcx_inf->timer_on) {
		btcx_inf->timer_on = 0;
		del_timer_sync(&btcx_inf->timer);
	}

	switch (btcx_inf->bt_state) {
		case BT_DHCP_START:
			/* DHCP started
			 * provide OPPORTUNITY window to get DHCP address
			 */
			WL_TRACE(("bt_dhcp stm: started \n"));

			btcx_inf->bt_state = BT_DHCP_OPPR_WIN;
			mod_timer(&btcx_inf->timer,
				jiffies + msecs_to_jiffies(BT_DHCP_OPPR_WIN_TIME));
			btcx_inf->timer_on = 1;
			break;

		case BT_DHCP_OPPR_WIN:
			if (btcx_inf->dhcp_done) {
				WL_TRACE(("DHCP Done before T1 expiration\n"));
				goto btc_coex_idle;
			}

			/* DHCP is not over yet, start lowering BT priority
			 * enforce btc_params + flags if necessary
			 */
			WL_TRACE(("DHCP T1:%d expired\n", BT_DHCP_OPPR_WIN_TIME));
			if (btcx_inf->dev)
				wl_cfg80211_bt_setflag(btcx_inf->dev, TRUE);
			btcx_inf->bt_state = BT_DHCP_FLAG_FORCE_TIMEOUT;
			mod_timer(&btcx_inf->timer,
				jiffies + msecs_to_jiffies(BT_DHCP_FLAG_FORCE_TIME));
			btcx_inf->timer_on = 1;
			break;

		case BT_DHCP_FLAG_FORCE_TIMEOUT:
			if (btcx_inf->dhcp_done) {
				WL_TRACE(("DHCP Done before T2 expiration\n"));
			} else {
				/* Noo dhcp during T1+T2, restore BT priority */
				WL_TRACE(("DHCP wait interval T2:%d msec expired\n",
					BT_DHCP_FLAG_FORCE_TIME));
			}

			/* Restoring default bt priority */
			if (btcx_inf->dev)
				wl_cfg80211_bt_setflag(btcx_inf->dev, FALSE);
btc_coex_idle:
			btcx_inf->bt_state = BT_DHCP_IDLE;
			btcx_inf->timer_on = 0;
			break;

		default:
			WL_ERR(("error g_status=%d !!!\n",	btcx_inf->bt_state));
			if (btcx_inf->dev)
				wl_cfg80211_bt_setflag(btcx_inf->dev, FALSE);
			btcx_inf->bt_state = BT_DHCP_IDLE;
			btcx_inf->timer_on = 0;
			break;
	}

	net_os_wake_unlock(btcx_inf->dev);
}

void* wl_cfg80211_btcoex_init(struct net_device *ndev)
{
	struct btcoex_info *btco_inf = NULL;

	btco_inf = kmalloc(sizeof(struct btcoex_info), GFP_KERNEL);
	if (!btco_inf)
		return NULL;

	btco_inf->bt_state = BT_DHCP_IDLE;
	btco_inf->ts_dhcp_start = 0;
	btco_inf->ts_dhcp_ok = 0;
	/* Set up timer for BT  */
	btco_inf->timer_ms = 10;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 85)
	init_timer(&btco_inf->timer);
	btco_inf->timer.data = (ulong)btco_inf;
	btco_inf->timer.function = wl_cfg80211_bt_timerfunc;
#else
	timer_setup(&btco_inf->timer, wl_cfg80211_bt_timerfunc, 0);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 85 */

	btco_inf->dev = ndev;

	INIT_WORK(&btco_inf->work, wl_cfg80211_bt_handler);

	btcoex_info_loc = btco_inf;
	return btco_inf;
}

void wl_cfg80211_btcoex_deinit()
{
	if (!btcoex_info_loc)
		return;

	if (btcoex_info_loc->timer_on) {
		btcoex_info_loc->timer_on = 0;
		del_timer_sync(&btcoex_info_loc->timer);
	}

	cancel_work_sync(&btcoex_info_loc->work);

	kfree(btcoex_info_loc);
}

int wl_cfg80211_set_btcoex_dhcp(struct net_device *dev, char *command)
{

#ifndef OEM_ANDROID
	static int  pm = PM_FAST;
	int  pm_local = PM_OFF;
#endif /* OEM_ANDROID */
	struct bcm_cfg80211 *cfg;
	struct btcoex_info *btco_inf = btcoex_info_loc;
	char powermode_val = 0;
	char buf_reg66va_dhcp_on[8] = { 66, 00, 00, 00, 0x10, 0x27, 0x00, 0x00 };
	char buf_reg41va_dhcp_on[8] = { 41, 00, 00, 00, 0x33, 0x00, 0x00, 0x00 };
	char buf_reg68va_dhcp_on[8] = { 68, 00, 00, 00, 0x90, 0x01, 0x00, 0x00 };

	uint32 regaddr;
	static uint32 saved_reg66;
	static uint32 saved_reg41;
	static uint32 saved_reg68;
	static bool saved_status = FALSE;

	char buf_flag7_default[8] =   { 7, 00, 00, 00, 0x0, 0x00, 0x00, 0x00};

	cfg = wl_get_cfg(dev);
	/* Figure out powermode 1 or o command */
#ifdef  OEM_ANDROID
	strncpy((char *)&powermode_val, command + strlen("BTCOEXMODE") +1, 1);
#else
	strncpy((char *)&powermode_val, command + strlen("POWERMODE") +1, 1);
#endif

	if (strnicmp((char *)&powermode_val, "1", strlen("1")) == 0) {
		WL_TRACE(("DHCP session starts\n"));

#if defined(OEM_ANDROID) && defined(DHCP_SCAN_SUPPRESS)
		/* Suppress scan during the DHCP */
		wl_cfg80211_scan_suppress(dev, 1);
#endif /* OEM_ANDROID && DHCP_SCAN_SUPPRESS */

#ifdef PKT_FILTER_SUPPORT
		cfg->wlcore->dhcp_in_progress = 1;

#if defined(APSTA_BLOCK_ARP_DURING_DHCP)
		if ((cfg->wlcore->op_mode & DHD_FLAG_CONCURR_STA_HOSTAP_MODE) ==
			DHD_FLAG_CONCURR_STA_HOSTAP_MODE) {
			/* Block ARP frames while DHCP of STA interface is in
			 * progress in case of STA/SoftAP concurrent mode
			 */
			wl_cfg80211_block_arp(dev, TRUE);
		} else
#endif /* APSTA_BLOCK_ARP_DURING_DHCP */
		if (cfg->wlcore->early_suspended) {
			WL_TRACE(("DHCP in progressing , disable packet filter!!!\n"));
			wl_cfg80211_enable_packet_filter(cfg, 0);
		}
#endif /* PKT_FILTER_SUPPORT */

		/* Retrieve and saved orig regs value */
		if ((saved_status == FALSE) &&
#ifndef OEM_ANDROID
			(!dev_wlc_ioctl(dev, WLC_GET_PM, &pm, sizeof(pm))) &&
#endif
			(!dev_wlc_intvar_get_reg(dev, "btc_params", 66,  &saved_reg66)) &&
			(!dev_wlc_intvar_get_reg(dev, "btc_params", 41,  &saved_reg41)) &&
			(!dev_wlc_intvar_get_reg(dev, "btc_params", 68,  &saved_reg68)))   {
				saved_status = TRUE;
				WL_TRACE(("Saved 0x%x 0x%x 0x%x\n",
					saved_reg66, saved_reg41, saved_reg68));

				/* Disable PM mode during dhpc session */
#ifndef OEM_ANDROID
				dev_wlc_ioctl(dev, WLC_SET_PM, &pm_local, sizeof(pm_local));
#endif

				/* Disable PM mode during dhpc session */
				/* Start  BT timer only for SCO connection */
				if (btcoex_is_sco_active(dev)) {
					/* btc_params 66 */
					dev_wlc_bufvar_set(dev, "btc_params",
						(char *)&buf_reg66va_dhcp_on[0],
						sizeof(buf_reg66va_dhcp_on));
					/* btc_params 41 0x33 */
					dev_wlc_bufvar_set(dev, "btc_params",
						(char *)&buf_reg41va_dhcp_on[0],
						sizeof(buf_reg41va_dhcp_on));
					/* btc_params 68 0x190 */
					dev_wlc_bufvar_set(dev, "btc_params",
						(char *)&buf_reg68va_dhcp_on[0],
						sizeof(buf_reg68va_dhcp_on));
					saved_status = TRUE;

					btco_inf->bt_state = BT_DHCP_START;
					btco_inf->timer_on = 1;
					mod_timer(&btco_inf->timer, btco_inf->timer.expires);
					WL_TRACE(("enable BT DHCP Timer\n"));
				}
		}
		else if (saved_status == TRUE) {
			WL_ERR(("was called w/o DHCP OFF. Continue\n"));
		}
	}
#ifdef  OEM_ANDROID
	else if (strnicmp((char *)&powermode_val, "2", strlen("2")) == 0) {
#else
	else if (strnicmp((char *)&powermode_val, "0", strlen("0")) == 0) {
#endif

#if defined(OEM_ANDROID) && defined(DHCP_SCAN_SUPPRESS)
		/* Since DHCP is complete, enable the scan back */
		wl_cfg80211_scan_suppress(dev, 0);
#endif /* OEM_ANDROID */

#ifdef PKT_FILTER_SUPPORT
		cfg->wlcore->dhcp_in_progress = 0;
		WL_TRACE(("DHCP is complete \n"));

#if defined(APSTA_BLOCK_ARP_DURING_DHCP)
		if ((cfg->wlcore->op_mode & DHD_FLAG_CONCURR_STA_HOSTAP_MODE) ==
			DHD_FLAG_CONCURR_STA_HOSTAP_MODE) {
			/* Unblock ARP frames */
			wl_cfg80211_block_arp(dev, FALSE);
		} else
#endif /* APSTA_BLOCK_ARP_DURING_DHCP */
		if (cfg->wlcore->early_suspended) {
			/* Enable packet filtering */
			WL_TRACE(("DHCP is complete , enable packet filter!!!\n"));
			wl_cfg80211_enable_packet_filter(cfg, 1);
		}
#endif /* PKT_FILTER_SUPPORT */

		/* Restoring PM mode */
#ifndef OEM_ANDROID
		dev_wlc_ioctl(dev, WLC_SET_PM, &pm, sizeof(pm));
#endif

		/* Stop any bt timer because DHCP session is done */
		WL_TRACE(("disable BT DHCP Timer\n"));
		if (btco_inf->timer_on) {
			btco_inf->timer_on = 0;
			del_timer_sync(&btco_inf->timer);

			if (btco_inf->bt_state != BT_DHCP_IDLE) {
			/* need to restore original btc flags & extra btc params */
				WL_TRACE(("bt->bt_state:%d\n", btco_inf->bt_state));
				/* wake up btcoex thread to restore btlags+params  */
				schedule_work(&btco_inf->work);
			}
		}

		/* Restoring btc_flag paramter anyway */
		if (saved_status == TRUE)
			dev_wlc_bufvar_set(dev, "btc_flags",
				(char *)&buf_flag7_default[0], sizeof(buf_flag7_default));

		/* Restore original values */
		if (saved_status == TRUE) {
			regaddr = 66;
			dev_wlc_intvar_set_reg(dev, "btc_params",
				(char *)&regaddr, (char *)&saved_reg66);
			regaddr = 41;
			dev_wlc_intvar_set_reg(dev, "btc_params",
				(char *)&regaddr, (char *)&saved_reg41);
			regaddr = 68;
			dev_wlc_intvar_set_reg(dev, "btc_params",
				(char *)&regaddr, (char *)&saved_reg68);

			WL_TRACE(("restore regs {66,41,68} <- 0x%x 0x%x 0x%x\n",
				saved_reg66, saved_reg41, saved_reg68));
		}
		saved_status = FALSE;

	}
	else {
		WL_ERR(("Unkwown yet power setting, ignored\n"));
	}

	snprintf(command, 3, "OK");

	return (strlen("OK"));
}
#endif /* defined(OEM_ANDROID) */
