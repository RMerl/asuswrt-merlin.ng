/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <qca.h>
#include <iwlib.h>
//#include <stapriv.h>
#include <ethutils.h>
#include <shared.h>
#include <sys/mman.h>
#ifndef O_BINARY
#define O_BINARY 	0
#endif
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#define wan_prefix(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)
//static char * rfctime(const time_t *timep);
//static char * reltime(unsigned int seconds);
void reltime(unsigned int seconds, char *buf);
static int wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit);

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <net/if_arp.h>

#include <dirent.h>

const char *get_wifname(int band)
{
	return get_wififname(band);
}

typedef struct _WPS_CONFIGURED_VALUE {
	unsigned short 	Configured;	// 1:un-configured/2:configured
	char		BSSID[18];
	char 		SSID[32 + 1];
	char		AuthMode[16];	// Open System/Shared Key/WPA-Personal/WPA2-Personal/WPA-Enterprise/WPA2-Enterprise
	char 		Encryp[8];	// None/WEP/TKIP/AES
	char 		DefaultKeyIdx;
	char 		WPAKey[64 + 1];
} WPS_CONFIGURED_VALUE;

/* shared/sysdeps/api-qca.c */
extern u_int ieee80211_mhz2ieee(u_int freq);
extern int get_channel_list_via_driver(int unit, char *buffer, int len);
extern int get_channel_list_via_country(int unit, const char *country_code, char *buffer, int len);

#define WL_A		(1U << 0)
#define WL_B		(1U << 1)
#define WL_G		(1U << 2)
#define WL_N		(1U << 3)
#define WL_AC		(1U << 4)
#define WL_AD		(1U << 5)
#define WL_AX		(1U << 6)
#define WL_AY		(1U << 7)

static const struct mode_s {
	unsigned int mask;
	char *mode;
} mode_tbl[] = {
	{ WL_A,	"a" },
	{ WL_B,	"b" },
	{ WL_G, "g" },
	{ WL_N, "n" },
	{ WL_AC, "ac" },
	{ WL_AD, "ad" },
	{ WL_AX, "ax" },
	{ WL_AY, "ay" },
	{ 0, NULL },
};

static void getWPSConfig(int unit, WPS_CONFIGURED_VALUE *result)
{
	char buf[128];
	FILE *fp;

	memset(result, 0, sizeof(*result));

	snprintf(buf, sizeof(buf), "hostapd_cli -i%s get_config", get_wifname(unit));
	fp = popen(buf, "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			char *pt1, *pt2;

			chomp(buf);
			//BSSID
			if ((pt1 = strstr(buf, "bssid="))) {
				pt2 = pt1 + strlen("bssid=");
				strlcpy(result->BSSID, pt2, sizeof(result->BSSID));
			}
			//SSID
			if ((pt1 = strstr(buf, "ssid="))) {
				pt2 = pt1 + strlen("ssid=");
				strlcpy(result->SSID, pt2, sizeof(result->SSID));
			}
			//Configured
			else if ((pt1 = strstr(buf, "wps_state="))) {
				pt2 = pt1 + strlen("wps_state=");
				if (!strcmp(pt2, "configured") ||
				    (!strcmp(pt2, "disabled") && nvram_get_int("w_Setting"))
				   )
					result->Configured = 2;
				else
					result->Configured = 1;
			}
			//WPAKey
			else if ((pt1 = strstr(buf, "passphrase="))) {
				pt2 = pt1 + strlen("passphrase=");
				strlcpy(result->WPAKey, pt2, sizeof(result->WPAKey));
			}
			//AuthMode
			else if ((pt1 = strstr(buf, "key_mgmt="))) {
				pt2 = pt1 + strlen("key_mgmt=");
				strlcpy(result->AuthMode, pt2, sizeof(result->AuthMode));/* FIXME: NEED TRANSFORM CONTENT */
			}
			//Encryp
			else if ((pt1 = strstr(buf, "rsn_pairwise_cipher="))) {
				pt2 = pt1 + strlen("rsn_pairwise_cipher=");
				if (!strcmp(pt2, "NONE"))
					strlcpy(result->Encryp, "None", sizeof(result->Encryp));
				else if (!strncmp(pt2, "WEP", 3))
					strlcpy(result->Encryp, "WEP", sizeof(result->Encryp));
				else if (!strcmp(pt2, "TKIP"))
					strlcpy(result->Encryp, "TKIP", sizeof(result->Encryp));
				else if (!strncmp(pt2, "CCMP", 4))
					strlcpy(result->Encryp, "AES", sizeof(result->Encryp));
				else if (unit == WL_60G_BAND && !strncmp(pt2, "GCMP", 4))
					strlcpy(result->Encryp, "AES", sizeof(result->Encryp));
			}
		}
		pclose(fp);
	}
	//dbg("%s: SSID[%s], Configured[%d], WPAKey[%s], AuthMode[%s], Encryp[%s]\n", __FUNCTION__, result->SSID, result->Configured, result->WPAKey, result->AuthMode, result->Encryp);
}

/**
 * Convert WL_XXX bit masks to string via mode_tbl[]
 * @mask:
 * @return:
 */
static char *mode_mask_to_str(unsigned int mask)
{
	static char result[sizeof("11b/g/nXXXXXXXXXXXX")] = "";
	const struct mode_s *q;
	char *p, *sep;
	size_t len, l;
	int noax = 0;

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	if (!find_word(nvram_safe_get("rc_support"), "11AX"))
		noax = 1;
#endif

	p = result;
	len = sizeof(result);
	*p = '\0';
	sep = "11";
	for (q = &mode_tbl[0]; len > 0 && mask > 0 && q->mask; ++q) {
		if (!(mask & q->mask))
			continue;

		if (q->mask == WL_AX && noax)
			continue;

		mask &= ~q->mask;
		strlcat(p, sep, len);
		l = strlen(sep);
		p += l;
		len -= l;
		strlcat(p, q->mode, len);
		l = strlen(q->mode);
		p += l;
		len -= l;
		sep = "/";
	}

	return result;
}

/**
 * Get phy mode via Wireless Extension ioctl of QCA WiFi 10.2/10.4 driver.
 * @iface:
 * @return:
 */
char *getAPPhyModebyIface(const char *iface)
{
	char *mode, *puren;
	unsigned int m = 0;
	int sta = 0;

	if (!iface)
		return "";
	mode = iwpriv_get(iface, "get_mode");
	if (!mode)
		return "";

	/* Ref to phymode_strings of qca-wifi driver. */
	if (!strcmp(mode, "11A") || !strcmp(mode, "TA"))
		m = WL_A;
	else if (!strcmp(mode, "11G") || !strcmp(mode, "TG"))
		m = WL_G | WL_B;
	else if (!strcmp(mode, "11B"))
		m = WL_B;
	else if (!strncmp(mode, "11NA", 4))
		m = WL_N | WL_A;
	else if (!strncmp(mode, "11NG", 4))
		m = WL_N | WL_G | WL_B;
	else if (!strncmp(mode, "11ACVHT", 7))
		m = WL_AC | WL_N | WL_A;
	else if (!strncmp(mode, "11AHE", 5))
		m = WL_AX | WL_AC | WL_N | WL_A;
	else if (!strncmp(mode, "11GHE", 5))
		m = WL_AX | WL_N | WL_G | WL_B;
	else if (!strncmp(mode, "AUTO", 4)) {
		if (!strcmp(iface, get_staifname(WL_2G_BAND))) {
			sta = 1;
			m = WL_N | WL_G | WL_B;
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
			m |= WL_AX;
#endif
		}
		else if (!strcmp(iface, get_staifname(WL_5G_BAND))
#if defined(RTCONFIG_HAS_5G_2)
			 || !strcmp(iface, get_staifname(WL_5G_2_BAND))	/* for 2-nd 5GHz */
#endif
#if defined(RTCONFIG_WIGIG)
			 || !strcmp(iface, get_staifname(WL_60G_BAND))	/* for 802.11ad Wigig */
#endif
			) {
			sta = 1;
			m = WL_AC | WL_N | WL_A;
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
			m |= WL_AX;
#endif
		}
		else
			dbg("%s: Unknown interface [%s] in AUTO mode\n", __func__, iface);
	}
	else {
		dbg("%s: Unknown mode [%s]\n", __func__, mode);
	}

	/* If puren is enabled, remove a/g/b. */
	puren = iwpriv_get(iface, "get_puren");
	if (!sta && safe_atoi(puren))
		m &= ~(WL_A | WL_B | WL_G);

	return mode_mask_to_str(m);
}

/**
 * Get phy mode via nl80211 of 802.11ad Wigig driver.
 * @ifname:
 * @return:
 */
char *getAPPhyModebyIfaceIW(const char *ifname)
{
	static char result[sizeof("11a/b/g/n/ac/adXXXXXX")] = "";
	unsigned int m = 0;
	char cmd[sizeof("iwinfo wlan0 info") + IFNAMSIZ];
#if defined(RTAD7200)
	const unsigned int default_11ad_mask = WL_AD;
#else
	const unsigned int default_11ad_mask = WL_AD | WL_AY;
#endif

	if (!ifname || *ifname == '\0') {
		dbg("%s: got invalid ifname %p\n", __func__, ifname);
		return 0;
	}

	/* Example:
	 * wlan0     ESSID: "OpenWrt_11ad"
	 *           Access Point: 04:CE:14:0A:21:17
	 *           Mode: Master  Channel: 3 (62.640 GH)
	 *           Tx-Power: unknown  Link Quality: 0/100
	 *           Signal: unknown  Noise: unknown
	 *           Bit Rate: unknown
	 *           Encryption: none
	 *           Type: nl80211  HW Mode(s): 802.11/ad
	 *           Hardware: 1AE9:0310 1AE9:0000 [Generic MAC80211]
	 *           TX power offset: unknown
	 *           Frequency offset: unknown
	 *           Supports VAPs: no
	 *           Beacon Interval: 100
	 */
	snprintf(cmd, sizeof(cmd), "iwinfo %s info", ifname);
	if (exec_and_parse(cmd, "HW Mode", "%*[^:]:%*[^:]: %s", 1, result))
		*result = '\0';

	if (!strcmp(result, "802.11ad"))
		m = default_11ad_mask;
	else
		dbg("%s: unknown phy mode string [%s]\n", __func__, result);

	return mode_mask_to_str(m);
}

char *getAPPhyMode(int unit)
{
	char *r = "";

	if (unit < 0 || unit >= MAX_NR_WL_IF)
		return "";

	switch (unit) {
	case WL_2G_BAND:	/* fall-through */
	case WL_5G_BAND:	/* fall-through */
	case WL_5G_2_BAND:
		r = getAPPhyModebyIface(get_wifname(unit));
		break;
	case WL_60G_BAND:
		r = getAPPhyModebyIfaceIW(get_wifname(unit));
		break;
	default:
		dbg("%s: unknown wl%d band!\n", __func__, unit);
	}

	return r;
}

static unsigned int getAPChannelbyIface(const char *ifname)
{
	char buf[8192];
	FILE *fp;
	int len, i = 0;
	char *pt1, *pt2, ch_mhz[10], ch_mhz_t[10];

	if (!ifname || *ifname == '\0') {
		dbg("%S: got invalid ifname %p\n", __func__, ifname);
		return 0;
	}

	snprintf(buf, sizeof(buf), "iwconfig %s", ifname);
	fp = popen(buf, "r");
	if (fp) {
		memset(buf, 0, sizeof(buf));
		len = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if (len > 1) {
			buf[len-1] = '\0';
			pt1 = strstr(buf, "Frequency:");
			if (pt1) {
				pt2 = pt1 + strlen("Frequency:");
				pt1 = strstr(pt2, " GHz");
				if (pt1) {
					*pt1 = '\0';
					memset(ch_mhz, 0, sizeof(ch_mhz));
					len = strlen(pt2);
					for (i = 0; i < 5; i++) {
						if (i < len) {
							if (pt2[i] == '.')
								continue;
							snprintf(ch_mhz_t, sizeof(ch_mhz), "%s%c", ch_mhz, pt2[i]);
							strlcpy(ch_mhz, ch_mhz_t, sizeof(ch_mhz));
						}
						else{
							snprintf(ch_mhz_t, sizeof(ch_mhz), "%s0", ch_mhz);
							strlcpy(ch_mhz, ch_mhz_t, sizeof(ch_mhz));
						}
					}
					//dbg("Frequency:%s MHz\n", ch_mhz);
					return ieee80211_mhz2ieee((unsigned int)safe_atoi(ch_mhz));
				}
			}
		}
	}
	return 0;
}

static unsigned int getAPChannelbyIWInfo(const char *ifname)
{
	int r;
	unsigned int freq = 0, ch = 0;
	char buf[256];

	if (!ifname || *ifname == '\0') {
		dbg("%s: got invalid ifname %p\n", __func__, ifname);
		return 0;
	}

	/* FIXME:
	 * I can't find any nl80211 based command that can be used to get channel from 11ad interface.
	 */
	/* Example: /sys/kernel/debug/ieee80211/phy0/wil6210/freq
	 * Freq = 60480
	 */
	r = f_read_string("/sys/kernel/debug/ieee80211/phy2/wil6210/freq", buf, sizeof(buf));
	if (r < strlen("Freq = xxxxx"))
		return 0;

	r = sscanf(buf, "Freq = %u", &freq);
	if (r != 1)
		return 0;

	ch = ieee80211_mhz2ieee(freq);

	return ch;
}

unsigned int getAPChannel(int unit)
{
	int r = 0;

	switch (unit) {
	case WL_2G_BAND:	/* fall-through */
	case WL_5G_BAND:	/* fall-through */
	case WL_5G_2_BAND:
		r = getAPChannelbyIface(get_wifname(unit));
		break;
	case WL_60G_BAND:
		/* FIXME */
		r = getAPChannelbyIWInfo(get_wifname(unit));
		break;
	default:
		dbg("%s: Unknown wl%d band!\n", __func__, unit);
	}

	return r;
}

#if defined(GTAXY16000)
unsigned int getEDMGChannel(void)
{
	int edmg_channel = 0;
	char cmd[sizeof("hostapd_cli -i XXXX status") + IFNAMSIZ];

	/* Example:
	 * / # hostapd_cli -i wlan0 status
	 * state=ENABLED
	 * phy=wlan0
	 * freq=60480
	 * ......
	 * channel=2
	 * edmg_enable=1
	 * edmg_channel=10
	 * ......
	 */
	snprintf(cmd, sizeof(cmd), "hostapd_cli -i %s status", get_wififname(WL_60G_BAND));
	if (exec_and_parse(cmd, "edmg_channel", "%*[^=]=%d", 1, &edmg_channel))
		edmg_channel = 0;

	return edmg_channel;
}
#endif

static int __getAPBitRate(const char *ifname, char *buf, size_t buf_len)
{
	int r;
	FILE *fp;
	char cmd[sizeof("iwconfig athXYYYYYY")], line[256], *rate, *unit;

	if (!ifname || *ifname == '\0' || !buf || !buf_len) {
		dbg("%s: got invalid ifname,buf,buf_len %p,%p,%d\n",
			__func__, ifname, buf, buf_len);
		return 0;
	}

	snprintf(cmd, sizeof(cmd), "iwconfig %s", ifname);
	fp = popen(cmd, "r");
	if (!fp)
		return 0;

	/* Example:
	 * / # iwconfig ath0
	 * ath0      IEEE 802.11axa  ESSID:"ASUS_00_5G"
	 *           Mode:Master  Frequency:5.745 GHz  Access Point: 00:03:7F:12:B3:B3
	 *           Bit Rate:4.8039 Gb/s   Tx-Power:40 dBm
	 *           RTS thr:off   Fragment thr:off
	 *           Encryption key:BEA0-1288-6B95-B8A6-BDB4-4AF3-3248-2244   Security mode:restricted
	 *           Power Management:off
	 *           Link Quality=94/94  Signal level=-107 dBm  Noise level=-105 dBm
	 *           Rx invalid nwid:3287  Rx invalid crypt:0  Rx invalid frag:0
	 *           Tx excessive retries:0  Invalid misc:0   Missed beacon:0
	 */
	while (fgets(line, sizeof(line), fp)) {
		if (!strstr(line, "Bit Rate"))
			continue;
		if ((r = sscanf(line, "%*[^:]:%ms %ms", &rate, &unit)) != 2) {
			_dprintf("%s: Unknown bit rate of ifname [%s]: [%s]\n",
				__func__, ifname, line);
			continue;
		}
		break;
	}
	pclose(fp);

	if (!rate || !unit) {
		*buf = '\0';
	} else {
		snprintf(buf, buf_len, "%s %s", rate, unit);
	}

	if (rate)
		free(rate);
	if (unit)
		free(unit);

	return 0;
}

#if defined(RTCONFIG_WIGIG)
static int __getAPBitRateIW(int band, const char *ifname, char *buf, size_t buf_len)
{
	int ratio = 1, mcs, edmg_enable, edmg_channel;
	char cmd[sizeof("hostapd_cli -i XXXX status") + IFNAMSIZ];
	float rate[] = {
		27.5, 385, 770, 962.5,						/* MCS0~3  Mb/s */
		1.155, 1.25125, 1.54, 1.925, 2.31, 2.5025, 3.08, 3.85, 4.62	/* MCS4~12 Gb/s */
	};

	if (band < 0 || band >= WL_NR_BANDS || !ifname || *ifname == '\0' || !buf || !buf_len) {
		dbg("%s: got invalid ifname,buf,buf_len %p,%p,%d\n",
			__func__, ifname, buf, buf_len);
		return 0;
	}

	/* Example:
	 * / # iw phy phy0 info
	 * Wiphy phy0
	 *       max # scan SSIDs: 1
	 *       max scan IEs length: 1024 bytes
	 *       max # sched scan SSIDs: 0
	 *       max # match sets: 0
	 *       max # scan plans: 1
	 *       max scan plan interval: -1
	 *       max scan plan iterations: 0
	 *       Retry short limit: 7
	 *       Retry long limit: 4
	 *       Coverage class: 0 (up to 0m)
	 *       Available Antennas: TX 0 RX 0
	 *       Supported interface modes:
	 *                * managed
	 *                * AP
	 *                * monitor
	 *                * P2P-client
	 *                * P2P-GO
	 *                * P2P-device
	 *       Band 3:
	 *               Capabilities: 0x00
	 *                       HT20
	 *                       Static SM Power Save
	 *                       No RX STBC
	 *                       Max AMSDU length: 3839 bytes
	 *                       No DSSS/CCK HT40
	 *               Maximum RX AMPDU length 65535 bytes (exponent: 0x003)
	 *               Minimum RX AMPDU time spacing: 8 usec (0x06)
	 *               HT TX/RX MCS rate indexes supported: 1-12
	 *               Frequencies:
	 *                       * 58320 MHz [1] (40.0 dBm)
	 *                       * 60480 MHz [2] (40.0 dBm)
	 *                       * 62640 MHz [3] (40.0 dBm)
	 *                       * 64800 MHz [4] (40.0 dBm)
	 *       interface combinations are not supported
	 */
	snprintf(cmd, sizeof(cmd), "iw phy %s info", get_vphyifname(band));
	if (exec_and_parse(cmd, "MCS", "%*[^-]-%d", 1, &mcs))
		mcs = 12;

	if (mcs >= 12) {
		/* Example:
		 * /# hostapd_cli -i wlan0 status
		 * state=ENABLED
		 * phy=wlan0
		 * freq=60480
		 * ......
		 * channel=2
		 * edmg_enable=0
		 * edmg_channel=0
		 * ......
		 */
		snprintf(cmd, sizeof(cmd), "hostapd_cli -i %s status", get_wififname(WL_60G_BAND));
		if (exec_and_parse(cmd, "edmg_enable", "%*[^=]=%d", 1, &edmg_enable))
			edmg_enable = 0;
		if (edmg_enable) {
			if (exec_and_parse(cmd, "edmg_channel", "%*[^=]=%d", 1, &edmg_channel))
				edmg_channel = 0;
			if (edmg_channel >= 9 && edmg_channel <= 13)
				ratio = 2;
			else if (edmg_channel >= 17 && edmg_channel <= 20)
				ratio = 3;
			else if (edmg_channel >= 25 && edmg_channel <= 27)
				ratio = 4;
		}
	}

	*buf = '\0';
	if (mcs >= 0 && mcs < ARRAY_SIZE(rate))
		snprintf(buf, buf_len, "%.2f %s", ratio * rate[mcs], (mcs >= 4)? "Gb/s" : "Mb/s");

	return 0;
}
#endif

static void getVAPBitRate(int unit, char *ifname, char *buf, size_t buf_len)
{
	char *rate = "N/A";

	if (!buf || !buf_len)
		return;

	strlcpy(buf, rate, buf_len);
	switch (unit) {
	case WL_2G_BAND:	/* fall-through */
	case WL_5G_BAND:	/* fall-through */
	case WL_5G_2_BAND:
		__getAPBitRate(ifname, buf, buf_len);
		break;
#if defined(RTCONFIG_WIGIG)
	case WL_60G_BAND:
		/* FIXME */
		__getAPBitRateIW(unit, ifname, buf, buf_len);
		break;
#endif
	default:
		dbg("%s: Unknown wl%d band!\n", __func__, unit);
	}
}

/**
 * Return SSID of a interface.
 * @return:	Don't return NULL even interface name is invalid or interface absent.
 */
char* getSSIDbyIFace(int unit, const char *ifname)
{
	static char ssid[33] = "";
	char buf[8192] = "";
	FILE *fp;
	int len;
	char *pt1, *pt2, *pt3;

	if (unit < 0 || unit >= MAX_NR_WL_IF)
		return ssid;

	if (!ifname || *ifname == '\0') {
		dbg("%S: got invalid ifname %p\n", __func__, ifname);
		return ssid;
	}

#if defined(RTCONFIG_WIGIG)
	if (unit == WL_60G_BAND) {
#if defined(RTAD7200)
		snprintf(buf, sizeof(buf), "/sys/kernel/debug/ieee80211/%s/wil6210/ssid", get_vphyifname(unit));
		f_read_string(buf, ssid, sizeof(ssid));
#elif defined(GTAXY16000)
		char cmd[sizeof("iw wlan0 info") + IFNAMSIZ];

		snprintf(cmd, sizeof(cmd), "iw %s info", get_wififname(unit));
		if (exec_and_parse(cmd, "ssid", "%*s %[^\n]", 1, ssid))
			*ssid = '\0';
#else
#error FIXME: Get SSID
#endif
	} else
#endif	/* RTCONFIG_WIGIG */
	{
		snprintf(buf, sizeof(buf), "iwconfig %s", ifname);
		if (!(fp = popen(buf, "r")))
			return ssid;

		len = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if (len <= 0)
			return ssid;

		buf[len] = '\0';
		pt1 = strstr(buf, "ESSID:");
		if (!pt1)
			return ssid;

		pt2 = pt1 + strlen("ESSID:") + 1;	/* skip leading " */
		pt1 = strchr(pt2, '\n');
		if (!pt1 || (pt1 - pt2) <= 1)
			return ssid;

		/* Remove trailing " */
		*pt1 = '\0';
		pt3 = strrchr(pt2, '"');
		if (pt3)
			*pt3 = '\0';

		strlcpy(ssid, pt2, sizeof(ssid));
	}

	return ssid;
}

int
ej_wl_control_channel(int eid, webs_t wp, int argc, char_t **argv)
{
        int ret = 0;
        int channel_24 = 0, channel_50 = 0;
	int channel_5G2, channel_60G;
        
	channel_24 = getAPChannel(0);
#if defined(RTCONFIG_LYRA_5G_SWAP)
#if defined(RTCONFIG_WIFI_SON)
	if(nvram_match("wifison_ready","1"))
	{
		channel_50 = getAPChannel(1);
		channel_5G2 = getAPChannel(2);
	}
	else
#endif
	{
		channel_50 = getAPChannel(2);
		channel_5G2 = getAPChannel(1);
	}
#else
	channel_50 = getAPChannel(1);
	channel_5G2 = getAPChannel(2);
#endif
	channel_60G = getAPChannel(3);

	ret = websWrite(wp, "[\"%d\", \"%d\", \"%d\", \"%d\"]",
		channel_24, channel_50, channel_5G2, channel_60G);
	
        return ret;
}

#if defined(GTAXY16000)
int
ej_wl_edmg_channel(int eid, webs_t wp, int argc, char_t **argv)
{
        int ret = 0, edmg_channel;

	edmg_channel = getEDMGChannel();
	ret = websWrite(wp, "[\"%d\", \"%d\", \"%d\", \"%d\"]", 0, 0, 0, edmg_channel);

        return ret;
}
#endif

long getSTAConnTime(char *ifname, char *bssid)
{
	char buf[8192];
	FILE *fp;
	int len;
	char *pt1,*pt2;

	snprintf(buf, sizeof(buf), "hostapd_cli -i%s sta %s", ifname, bssid);
	fp = popen(buf, "r");
	if (fp) {
		memset(buf, 0, sizeof(buf));
		len = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if (len > 1) {
			buf[len-1] = '\0';
			pt1 = strstr(buf, "connected_time=");
			if (pt1) {
				pt2 = pt1 + strlen("connected_time=");
				chomp(pt2);
				return atol(pt2);
			}
		}
	}
	return 0;
}

/** Get client list via wlanconfig utility.
 * @unit:
 * @sta_info:
 * @ifname:
 * @subunit_id:
 * 	'B':	Facebook Wi-Fi
 * 	'F':	Free Wi-Fi
 * 	'C':	Captive Portal
 * otherwise:	Main or guest network.
 * @return:
 */
static int __getSTAInfo(int unit, WIFI_STA_TABLE *sta_info, char *ifname, char id)
{
	int subunit;
	char subunit_str[4] = "0", wlif[sizeof("wlX.Yxxx")];

	if (absent_band(unit))
		return -1;
	if (!ifname || *ifname == '\0')
		return -1;

	subunit = get_wlsubnet(unit, ifname);
	if (subunit < 0)
		subunit = 0;
	if (subunit >= 10) {
		dbg("%s: invalid subunit %d\n", __func__, subunit);
		return -2;
	}

	snprintf(wlif, sizeof(wlif), "wl%d.%d", unit, subunit);
	if (subunit >= 0 && subunit < MAX_NO_MSSID)
		snprintf(subunit_str, sizeof(subunit_str), "%d", subunit);
	if (id == 'B' || id == 'F' || id == 'C')
		snprintf(subunit_str, sizeof(subunit_str), "%c", id);

	return get_qca_sta_info_by_ifname(ifname, subunit_str[0], sta_info);
}

/** Get client list via iw utility.
 * @unit:
 * @sta_info:
 * @ifname:
 * @subunit_id:
 * 	'B':	Facebook Wi-Fi
 * 	'F':	Free Wi-Fi
 * 	'C':	Captive Portal
 * otherwise:	Main or guest network.
 * @return:
 */
static int __getSTAInfoIW(int unit, WIFI_STA_TABLE *sta_info, char *ifname, char id)
{
	FILE *fp;
	int c, subunit, time_val, hr, min, sec, rssi;
	char rate[6], line_buf[300];
	char subunit_str[4] = "0", wlif[sizeof("wlX.Yxxx")];
	char cmd[sizeof("iw wlan0 station dump XXXXXX")];
	WLANCONFIG_LIST *r;


	if (absent_band(unit))
		return -1;
	if (!ifname || *ifname == '\0')
		return -1;

	subunit = get_wlsubnet(unit, ifname);
	if (subunit < 0)
		subunit = 0;
	if (subunit >= 10) {
		dbg("%s: invalid subunit %d\n", __func__, subunit);
		return -2;
	}

	snprintf(wlif, sizeof(wlif), "wl%d.%d", unit, subunit);
	if (subunit >= 0 && subunit < MAX_NO_MSSID)
		snprintf(subunit_str, sizeof(subunit_str), "%d", subunit);
	if (id == 'B' || id == 'F' || id == 'C')
		snprintf(subunit_str, sizeof(subunit_str), "%c", id);

	snprintf(cmd, sizeof(cmd), "iw %s station dump", get_wififname(unit));
	fp = popen(cmd, "r");
	if (!fp)
		return -2;

	/* /sys/kernel/debug/ieee80211/phy0/wil6210/stations has client list too.
	 * But I guess none of any another attributes exist, e.g., connection time, exist.
	 * ILQ1.3.7 Example: iw wlan0 station dump
	 * Station 04:ce:14:0a:21:17 (on wlan0)
	 *       rx bytes:       0
	 *       rx packets:     0
	 *       tx bytes:       0
	 *       tx packets:     0
	 *       tx failed:      0
	 *       tx bitrate:     27.5 MBit/s MCS 0
	 *       rx bitrate:     27.5 MBit/s MCS 0
	 *       connected time: 292 seconds
	 * SPF10.0 FC Example: iw wlan0 station dump
	 *  Station 04:ce:14:0b:46:12 (on wlan0)
	 *        rx bytes:       0
	 *        rx packets:     0
	 *        tx bytes:       0
	 *        tx packets:     0
	 *        tx failed:      0
	 *        rx drop misc:   0
	 *        signal:         -55 dBm
	 *        tx bitrate:     27.5 MBit/s MCS 0
	 *        rx bitrate:     27.5 MBit/s MCS 0
	 */
	while (fgets(line_buf, sizeof(line_buf), fp)) {
		if (strncmp(line_buf, "Station", 7)) {
			continue;
		}

next_sta:
		r = &sta_info->Entry[sta_info->Num++];
		c = sscanf(line_buf, "Station %17[0-9a-f:] %*[^\n]", r->addr);
		if (c != 1) {
			continue;
		}
		convert_mac_string(r->addr);
		r->subunit_id = subunit_str[0];
		strlcpy(r->mode, "11ad", sizeof(r->mode));
		while (fgets(line_buf, sizeof(line_buf), fp)) {
			if (!strncmp(line_buf, "Station", 7)) {

#if 0
				dbg("[%s][%u][%u][%s][%s][%u][%s]\n",
					r->addr, r->aid, r->chan, r->txrate, r->rxrate, r->rssi, r->mode);
#endif
				goto next_sta;
			} else if (strstr(line_buf, "tx bitrate:")) {
				c = sscanf(line_buf, "%*[ \t]tx bitrate:%*[ \t]%6[0-9.]", rate);
				if (c != 1) {
					continue;
				}
				snprintf(r->txrate, sizeof(r->txrate), "%sM", rate);
			} else if (strstr(line_buf, "rx bitrate:")) {
				c = sscanf(line_buf, "%*[ \t]rx bitrate:%*[ \t]%6[0-9.]", rate);
				if (c != 1) {
					continue;
				}
				snprintf(r->rxrate, sizeof(r->rxrate), "%sM", rate);
			} else if (strstr(line_buf, "connected time:")) {
				c = sscanf(line_buf, "%*[ \t]connected time:%*[ \t]%d seconds", &time_val);
				if (c != 1) {
					continue;
				}
				hr = time_val / 3600;
				time_val %= 3600;
				min = time_val / 60;
				sec = time_val % 60;
				snprintf(r->conn_time, sizeof(r->conn_time), "%02d:%02d:%02d", hr, min, sec);
			} else if (strstr(line_buf, "signal:")) {
				c = sscanf(line_buf, "%*[ \t]signal:%*[ \t]%d dBm", &rssi);
				r->rssi = rssi;
			} else {
				//dbg("%s: skip [%s]\n", __func__, line_buf);
			}
		}
	}
	pclose(fp);

	return 0;
}

#if defined(RTCONFIG_CAPTIVE_PORTAL)
/**
 * List non standard guest network clients, e.g., Free Wi-Fi and Captive portal
 * @unit:
 * @ifnames:	6-th parameter of captive_portal or captive_portal_adv_profile
 * 		e.g.: "wl0.6wl1.6" minus double quotes
 * @sta_info:
 * @id:		The @id will be passed to __getSTAInfo() function.
 * 	'B':	Facebook Wi-Fi
 * 	'F':	Free Wi-Fi
 * 	'C':	Captive Portal
 *  otherwise:	Main 2G/5G network or guest network.
 * @return:
 */
static int getNonStdGuestSTAInfo(int unit, char *ifnames,
					WIFI_STA_TABLE * sta_info, char id)
{
	int i, u, s;
	char *p, *q, ifname[IFNAMSIZ];

	if (absent_band(unit) || !ifnames || strncmp(ifnames, "wl", 2) ||
	    strlen(ifnames) < 5 || !sta_info)
		return -1;

	/* ifnames example: "wl0.6wl1.6", minus double quotes */
	for (u = -1, p = q = ifnames; u != unit && p != NULL; p = q) {
		q = strstr(p + 1, "wl");
		if (sscanf(p, "wl%d.%d", &u, &s) != 2 || u != unit)
			continue;
		break;
	}

	if (u != unit || p == NULL)
		return -2;
	for (i = 1, *ifname = '\0'; i < MAX_NO_MSSID; ++i) {
		__get_wlifname(unit, i, ifname);
		if (get_wlsubnet(unit, ifname) != s)
			continue;

		__getSTAInfo(unit, sta_info, ifname, id);
		break;
	}

	return 0;
}

/**
 * List Captive Portal clients.
 * @unit:
 * @sta_info:
 * @return:
 */
static int getCPortalSTAInfo(int unit, WIFI_STA_TABLE * sta_info)
{
	char *a[12], *nv, *nvp, *b;

	if (absent_band(unit) || !sta_info)
		return -1;

	/* Captive Portal */
	if (!nvram_match("captive_portal_adv_enable", "on"))
		return 0;

	nv = nvp = strdup(nvram_safe_get("captive_portal_adv_profile"));
	if (!nv)
		return 0;

	while ((b = strsep(&nvp, "<")) != NULL) {
		memset(a, 0, sizeof(a));
		if ((vstrsep
		     (b, ">", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7],
		      &a[8], &a[9], &a[10], &a[11]) != 12))
			continue;

		getNonStdGuestSTAInfo(unit, a[5], sta_info, 'C');
	}
	free(nv);

	return 0;
}

/**
 * List Free Wi-Fi clients.
 * @unit:
 * @sta_info:
 * @return:
 */
static int getFreeWiFiSTAInfo(int unit, WIFI_STA_TABLE *sta_info)
{
	char *a[7], *nv, *nvp, *b;

	if (absent_band(unit) || !sta_info)
		return -1;

	/* Free Wi-Fi */
	if (!nvram_match("captive_portal_enable", "on"))
		return 0;

	nv = nvp = strdup(nvram_safe_get("captive_portal"));
	if (!nv)
		return 0;

	while ((b = strsep(&nvp, "<")) != NULL) {
		memset(a, 0, sizeof(a));
		if ((vstrsep
		     (b, ">", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6]) != 7))
			continue;

		getNonStdGuestSTAInfo(unit, a[5], sta_info, 'F');
	}
	free(nv);

	return 0;
}
#else
static inline int getCPortalSTAInfo(int unit, WIFI_STA_TABLE *sta_info) { return 0; }
static inline int getFreeWiFiSTAInfo(int unit, WIFI_STA_TABLE *sta_info) { return 0; }
#endif

#if defined(RTCONFIG_FBWIFI)
/**
 * List Facebook Wi-Fi clients.
 * @unit:
 * @sta_info:
 * @return:
 */
static int getFacebookWiFiSTAInfo(int unit, WIFI_STA_TABLE *sta_info)
{
	char *fbwifi_iface[] = { "fbwifi_2g", "fbwifi_5g", "fbwifi_5g_2" };

	if (absent_band(unit) || unit >= ARRAY_SIZE(fbwifi_iface)|| !sta_info)
		return -1;

	/* Facebook Wi-Fi */
	if (!nvram_match("fbwifi_enable", "on"))
		return 0;

	getNonStdGuestSTAInfo(unit, nvram_safe_get(fbwifi_iface[unit]), sta_info, 'B');

	return 0;
}
#else
static inline int getFacebookWiFiSTAInfo(int unit, WIFI_STA_TABLE *sta_info) { return 0; }
#endif

static int getSTAInfo(int unit, WIFI_STA_TABLE *sta_info)
{
	int ret = 0;
	char *unit_name;
	char *p, *ifname;
	char *wl_ifnames;
#if defined(RTCONFIG_FBWIFI)
	char *fbwifi_iface[] = { "fbwifi_2g", "fbwifi_5g", "fbwifi_5g_2" };
	char wl_ifname[IFNAMSIZ] = "", *wl_if = wl_ifname;
#endif

	if (absent_band(unit))
		return 0;

#if defined(RTCONFIG_FBWIFI)
	if (nvram_match("fbwifi_enable", "on") &&
	    unit >= 0 && unit < min(MAX_NR_WL_IF, ARRAY_SIZE(fbwifi_iface)))
	{
		int j;

		if (sscanf(nvram_safe_get(fbwifi_iface[unit]), "wl%*d.%d", &j) == 1)
			wl_if = get_wlxy_ifname(unit, j, wl_ifname);
	}
#endif
	memset(sta_info, 0, sizeof(*sta_info));
	unit_name = strdup(get_wifname(unit));
	if (!unit_name)
		return ret;
#if defined(RTCONFIG_AMAS_WGN)  
        wl_ifnames = strdup(get_all_lan_ifnames());
#else
	wl_ifnames = strdup(nvram_safe_get("lan_ifnames"));
#endif	
	if (!wl_ifnames) {
		free(unit_name);
		return ret;
	}
	p = wl_ifnames;
	while ((ifname = strsep(&p, " ")) != NULL) {
		while (*ifname == ' ') ++ifname;
		if (*ifname == 0) break;
		SKIP_ABSENT_FAKE_IFACE(ifname);
		if (strncmp(ifname, unit_name, strlen(unit_name)))
			continue;

#if defined(RTCONFIG_FBWIFI)
		if (!strcmp(ifname, wl_if))
			continue;
#endif

		switch (unit) {
		case WL_2G_BAND:	/* fall-through */
		case WL_5G_BAND:	/* fall-through */
		case WL_5G_2_BAND:
			__getSTAInfo(unit, sta_info, ifname, 0);
			break;
		case WL_60G_BAND:
			__getSTAInfoIW(unit, sta_info, ifname, 0);
			break;
		default:
			dbg("%s: unknown wl%d band!\n", __func__, unit);
		}
	}
	free(wl_ifnames);
	free(unit_name);

	getFacebookWiFiSTAInfo(unit, sta_info);
	getCPortalSTAInfo(unit, sta_info);
	getFreeWiFiSTAInfo(unit, sta_info);

	return ret;
}

int
ej_wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	int ii = 0;
	char word[256], *next;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(ii);
		retval += wl_status(eid, wp, argc, argv, ii);
		retval += websWrite(wp, "\n");

		ii++;
	}

	return retval;
}

int
ej_wl_status_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_status(eid, wp, argc, argv, 0);
}

static int
show_wliface_info(webs_t wp, int unit, char *ifname, char *op_mode)
{
#if defined(GTAXY16000)
	unsigned int edmg_channel;
#endif
	int ret = 0;
	FILE *fp;
	unsigned char mac_addr[ETHER_ADDR_LEN];
	char tmpstr[1024], cmd[] = "iwconfig staXYYYYYY";
	char *p, ap_bssid[] = "00:00:00:00:00:00XXX";

	if (unit < 0 || !ifname || !op_mode)
		return 0;

	memset(&mac_addr, 0, sizeof(mac_addr));
	get_iface_hwaddr(ifname, mac_addr);
	ret += websWrite(wp, "=======================================================================================\n"); // separator
	ret += websWrite(wp, "OP Mode		: %s\n", op_mode);
	ret += websWrite(wp, "SSID		: %s\n", getSSIDbyIFace(unit, ifname));

	if (unit == WL_60G_BAND) {
		/* Example: iw wlan0 info
		 * Interface wlan0
		 *         ifindex 24
		 *         wdev 0x1
		 *         addr 04:ce:14:0b:46:12
		 *         type AP
		 *         wiphy 0
		 */
		snprintf(cmd, sizeof(cmd), "iw %s info", ifname);
		if (exec_and_parse(cmd, "addr", "%*s %[^\n]", 1, ap_bssid))
			*ap_bssid = '\0';
		convert_mac_string(ap_bssid);
	} else {
		snprintf(cmd, sizeof(cmd), "iwconfig %s", ifname);
		if ((fp = popen(cmd, "r")) != NULL && fread(tmpstr, 1, sizeof(tmpstr), fp) > 1) {
			pclose(fp);
			*(tmpstr + sizeof(tmpstr) - 1) = '\0';
			*ap_bssid = '\0';
			if ((p = strstr(tmpstr, "Access Point: ")) != NULL) {
				strlcpy(ap_bssid, p + 14, sizeof(ap_bssid));
				ap_bssid[17] = '\0';
			}
		}
	}
	ret += websWrite(wp, "BSSID		: %s\n", ap_bssid);
	ret += websWrite(wp, "MAC address	: %02X:%02X:%02X:%02X:%02X:%02X\n",
		mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	*tmpstr = '\0';
	strlcpy(tmpstr, getAPPhyMode(unit), sizeof(tmpstr));
	ret += websWrite(wp, "Phy Mode	: %s\n", tmpstr);
	getVAPBitRate(unit, ifname, tmpstr, sizeof(tmpstr));
	ret += websWrite(wp, "Bit Rate	: %s\n", tmpstr);
	ret += websWrite(wp, "Channel		: %u\n", getAPChannel(unit));
#if defined(GTAXY16000)
	if (unit == WL_60G_BAND) {
		edmg_channel = getEDMGChannel();
		if (edmg_channel != 0) {
			ret += websWrite(wp, "EDMG Channel	: %u\n", edmg_channel);
		}
	}
#endif

	return ret;
}

static int
wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret = 0, wl_mode_x, i;
	WIFI_STA_TABLE *sta_info;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_", *ifname, *op_mode;
	char subunit_str[20];
#if defined(RTCONFIG_CONCURRENTREPEATER)
	char wlc_prefix[] = "wlcXXXXXXXXXX_";
#endif

#if defined(RTCONFIG_LYRA_5G_SWAP)
       unit=swap_5g_band(unit);
#endif

#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_PROXYSTA)
	if (mediabridge_mode()) {
#if !defined(RTCONFIG_CONCURRENTREPEATER)
		/* Media bridge mode */
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		if (unit != nvram_get_int("wlc_band")) {
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			ret += websWrite(wp, "%s radio is disabled\n",
				wl_nband_name(nvram_pf_get(prefix, "nband")));
			return ret;
		}
		ret += show_wliface_info(wp, unit, ifname, "Media Bridge");
#else
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		snprintf(wlc_prefix, sizeof(wlc_prefix), "wlc%d_", unit);
		ret += show_wliface_info(wp, unit, ifname, "Media Bridge");
		ret += websWrite(wp, "\n");
#endif	/* #if !defined(RTCONFIG_CONCURRENTREPEATER) */
	} else {
#endif
		/* Router mode, Repeater and AP mode */
#if defined(RTCONFIG_WIRELESSREPEATER)
#if !defined(RTCONFIG_CONCURRENTREPEATER)
		if (!unit && repeater_mode()) {
			/* Show P-AP information first, if we are about to show 2.4G information in repeater mode. */
#if defined(RTCONFIG_REPEATER_STAALLBAND)
			unit = nvram_get_int("wlc_triBand");
			snprintf(prefix, sizeof(prefix), "sta%d", unit);
			ret += show_wliface_info(wp, unit, prefix, "Repeater");
			ret += websWrite(wp, "\n");
			unit = 0;
#else
			snprintf(prefix, sizeof(prefix), "wl%d.1_", nvram_get_int("wlc_band"));
			ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			ret += show_wliface_info(wp, nvram_get_int("wlc_band"), ifname, "Repeater");
			ret += websWrite(wp, "\n");
#endif
		}
#else
		if (repeater_mode()) {
			if (!unit) {
				if (nvram_get_int("wlc_express") == 0) {	/* concurrent repeater */
					for (i = 0; i <= 1; i++) {
						snprintf(prefix, sizeof(prefix), "wl%d.1_", i);
						ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
						snprintf(wlc_prefix, sizeof(wlc_prefix), "wlc%d_", i);
						ret += show_wliface_info(wp, i, ifname, "Repeater");
						ret += websWrite(wp, "\n");
					}
				}
				else {	/* express way (2G or 5G) */
					snprintf(prefix, sizeof(prefix), "wl%d.1_", nvram_get_int("wlc_express") - 1);
					ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
					snprintf(wlc_prefix, sizeof(wlc_prefix), "wlc%d_", nvram_get_int("wlc_express") - 1);
					ret += show_wliface_info(wp, nvram_get_int("wlc_express") - 1, ifname, nvram_get_int("wlc_express") == 1 ? "Express Way 2.4 GHz" : "Express Way 5 GHz");
					ret += websWrite(wp, "\n");
					//return ret;
				}
			}

			if (nvram_get_int("wlc_express") > 0) {
				if (unit == (nvram_get_int("wlc_express") - 1))
					return ret;
			}
		}
#endif	/* #if !defined(RTCONFIG_CONCURRENTREPEATER) */
#endif

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		if (!get_radio_status(ifname)) {
#if defined(BAND_2G_ONLY)
			ret += websWrite(wp, "2.4 GHz radio is disabled\n");
#else

#if defined(RTCONFIG_HIDDEN_BACKHAUL)
#if defined(MAPAC2200)
		if(strcmp(ifname,"ath1"))
#endif
#endif

			ret += websWrite(wp, "%s radio is disabled\n",
				wl_nband_name(nvram_pf_get(prefix, "nband")));
#endif
			return ret;
		}

		wl_mode_x = nvram_get_int(strcat_r(prefix, "mode_x", tmp));
		op_mode = "AP";
		if (wl_mode_x == 1)
			op_mode = "WDS Only";
		else if (wl_mode_x == 2)
			op_mode = "Hybrid";
#if defined(RTCONFIG_CONCURRENTREPEATER)
		if (repeater_mode()) {
			if (nvram_get_int("wlc_express") == 0) {	/* concurrent repeater */
				snprintf(wlc_prefix, sizeof(wlc_prefix), "wl%d.1_", unit);
				ret += show_wliface_info(wp, unit, ifname, op_mode);
			}
			else
			{
				snprintf(wlc_prefix, sizeof(wlc_prefix), "wl%d.1_", nvram_get_int("wlc_express") == 1 ? 1 : 0);
				ret += show_wliface_info(wp, nvram_get_int("wlc_express") == 1 ? 1 : 0, ifname, op_mode);
			}
		}
		else
#endif
		ret += show_wliface_info(wp, unit, ifname, op_mode);
		ret += websWrite(wp, "\nStations List\n");
		ret += websWrite(wp, "------------------------------------------------------------------------------------\n");
		ret += websWrite(wp, "%-16s %-17s %-15s %-4s %-7s %-7s %-12s\n",
			"idx", "MAC", "PhyMode", "RSSI", "TX_RATE", "RX_RATE", "Connect Time");

		if ((sta_info = malloc(sizeof(*sta_info))) != NULL) {
			getSTAInfo(unit, sta_info);
			for(i = 0; i < sta_info->Num; i++) {
				*subunit_str = '\0';
				if (sta_info->Entry[i].subunit_id == '0')
					strlcpy(subunit_str, "Main", sizeof(subunit_str));
				else if (isdigit(sta_info->Entry[i].subunit_id))
					snprintf(subunit_str, sizeof(subunit_str), "Guest Network-%c", sta_info->Entry[i].subunit_id);
				else if (sta_info->Entry[i].subunit_id == 'B')
					strlcpy(subunit_str, "Facebook Wi-Fi", sizeof(subunit_str));
				else if (sta_info->Entry[i].subunit_id == 'F')
					strlcpy(subunit_str, "Free Wi-Fi", sizeof(subunit_str));
				else if (sta_info->Entry[i].subunit_id == 'C')
					strlcpy(subunit_str, "Captive Portal", sizeof(subunit_str));
				else {
					dbg("%s: Unknown subunit_id [%c]\n", sta_info->Entry[i].subunit_id);
				}
				ret += websWrite(wp, "%-16s %-17s %-15s %4d %7s %7s %12s\n",
					subunit_str,
					sta_info->Entry[i].addr,
					sta_info->Entry[i].mode,
					sta_info->Entry[i].rssi,
					sta_info->Entry[i].txrate,
					sta_info->Entry[i].rxrate,
					sta_info->Entry[i].conn_time
					);
			}
			free(sta_info);
		}
#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_PROXYSTA)
	}
#endif

	return ret;
}

static int ej_wl_sta_list(int unit, webs_t wp)
{
	WIFI_STA_TABLE *sta_info;
	char *value;
	int firstRow = 1;
	int i;
	int from_app = 0;

	from_app = check_user_agent(user_agent);

	if ((sta_info = malloc(sizeof(*sta_info))) != NULL)
	{
		getSTAInfo(unit, sta_info);
		for(i = 0; i < sta_info->Num; i++)
		{
			if (firstRow == 1)
				firstRow = 0;
			else
				websWrite(wp, ", ");

			if (from_app == 0)
				websWrite(wp, "[");

			websWrite(wp, "\"%s\"", sta_info->Entry[i].addr);

			if (from_app != 0) {
				websWrite(wp, ":{");
				websWrite(wp, "\"isWL\":");
			}

			value = "Yes";
			if (from_app == 0)
				websWrite(wp, ", \"%s\"", value);
			else
				websWrite(wp, "\"%s\"", value);

			value = "";

			if (from_app == 0)
				websWrite(wp, ", \"%s\"", value);
	
			if (from_app != 0) {
				websWrite(wp, ",\"rssi\":");
			}

			if (from_app == 0)
				websWrite(wp, ", \"%d\"", sta_info->Entry[i].rssi);
			else
				websWrite(wp, "\"%d\"", sta_info->Entry[i].rssi);

			if (from_app == 0)
				websWrite(wp, "]");
			else
				websWrite(wp, "}");
		}
		free(sta_info);
	}
	return 0;
}

int ej_wl_sta_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	ej_wl_sta_list(0, wp);
	return 0;
}

int ej_wl_sta_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	ej_wl_sta_list(1, wp);
	return 0;
}

int ej_wl_sta_list_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
#if defined(RTCONFIG_HAS_5G_2)
	/* FIXME: I think it's not good to report 2-nd 5G station list in 1-st 5G station list. */
	ej_wl_sta_list(2, wp);
#endif
	return 0;
}

#if defined(RTCONFIG_STAINFO)
/**
 * Format:
 * 	[ MAC, TX_RATE, RX_RATE, CONNECT_TIME, IDX ]
 * IDX:	main/GN1/GN2/GN3
 */
static int wl_stainfo_list(int unit, webs_t wp)
{
	WIFI_STA_TABLE *sta_info;
	WLANCONFIG_LIST *r;
	char idx_str[8], s;
	int i, firstRow = 1;

	if ((sta_info = malloc(sizeof(*sta_info))) == NULL)
		return 0 ;

	getSTAInfo(unit, sta_info);
	for(i = 0, r = &sta_info->Entry[0]; i < sta_info->Num; i++, r++) {
		if (firstRow == 1)
			firstRow = 0;
		else
			websWrite(wp, ", ");

		websWrite(wp, "[");
		websWrite(wp, "\"%s\"", r->addr);
		websWrite(wp, ", \"%s\"", r->txrate);
		websWrite(wp, ", \"%s\"", r->rxrate);
		websWrite(wp, ", \"%s\"", r->conn_time);
		s = r->subunit_id;
		if (s < '0' || s  >= ('0' + MAX_NO_MSSID - 1))
			s = '0';
		if (s == '0')
			strlcpy(idx_str, "main", sizeof(idx_str));
		else if (isdigit(s))
			snprintf(idx_str, sizeof(idx_str), "GN%c", s);
		else
			snprintf(idx_str, sizeof(idx_str), "%c", toupper(s));
		websWrite(wp, ", \"%s\"", idx_str);
		websWrite(wp, "]");
	}
	free(sta_info);
	return 0;
}

int
ej_wl_stainfo_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_stainfo_list(0, wp);
}

int
ej_wl_stainfo_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_stainfo_list(1, wp);
}

int
ej_wl_stainfo_list_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_stainfo_list(2, wp);
}
#endif  /* RTCONFIG_STAINFO */

int ej_get_wlstainfo_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char word[64], *next;
	int unit = 0;
	int haveInfo = 0;

	websWrite(wp, "{");

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		WIFI_STA_TABLE *sta_info;
		WLANCONFIG_LIST *r;
		int i, j, s;
		char alias[16];

		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		if ((sta_info = malloc(sizeof(*sta_info))) == NULL)
			return 0;

		getSTAInfo(unit, sta_info);
		for (i = 0; i < MAX_NO_MSSID; i++) {
			int firstRow = 1;

			memset(alias, 0, sizeof(alias));
			switch (unit) {
				case WL_2G_BAND:
					if (i == 0)
						strlcpy(alias, "2G", sizeof(alias));
					else
						snprintf(alias, sizeof(alias), "%s_%d", "2G", i);
					break;
				case WL_5G_BAND:	/* fall-through */
#if defined(RTCONFIG_HAS_5G_2)
				case WL_5G_2_BAND:
#endif
					if (i == 0)
						snprintf(alias, sizeof(alias), "%s", unit == 2 ? "5G1" : "5G");
					else
						snprintf(alias, sizeof(alias), "%s_%d", unit == 2 ? "5G1" : "5G", i);
					break;
#if defined(RTCONFIG_WIGIG)
				case WL_60G_BAND:
					if (i == 0)
						strlcpy(alias, "60G", sizeof(alias));
					else
						snprintf(alias, sizeof(alias), "%s_%d", "60G", i);
					break;
#endif
				default:
					dbg("%s():%d: Unknown unit %d (MAX_NR_WL_IF %d)\n", __func__, __LINE__, unit, MAX_NR_WL_IF);
			}

			for(j = 0, r = &sta_info->Entry[0]; j < sta_info->Num; j++, r++) {
				s = r->subunit_id;
				if (s < 0 || s > 3)
					s = 0;

				if (i != s)
					continue;

				if (firstRow == 1) {
					if (haveInfo)
						websWrite(wp, ",");
					websWrite(wp, "\"%s\":[", alias);
					firstRow = 0;
					haveInfo = 1;
				}
				else
					websWrite(wp, ",");
				websWrite(wp, "{\"mac\":\"%s\",\"rssi\":%d}", r->addr, r->rssi);
			}

			if (!firstRow)
				websWrite(wp, "]");
		}
		free(sta_info);
		unit++;
	}
	websWrite(wp, "}");

	return 0;
}

char *getWscStatus(int unit)
{
	char buf[512];
	FILE *fp;
	int len;
	char *pt1,*pt2;

	snprintf(buf, sizeof(buf), "hostapd_cli -i%s wps_get_status", get_wifname(unit));
	fp = popen(buf, "r");
	if (fp) {
		memset(buf, 0, sizeof(buf));
		len = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if (len > 1) {
			buf[len-1] = '\0';
			pt1 = strstr(buf, "Last WPS result: ");
			if (pt1) {
				pt2 = pt1 + strlen("Last WPS result: ");
				pt1 = strstr(pt2, "Peer Address: ");
				if (pt1) {
					*pt1 = '\0';
					chomp(pt2);
				}
				return pt2;
			}
		}
	}
	return "";
}

char *getAPPIN(int unit)
{
	static char buffer[128];
#if 0
	char cmd[64];
	FILE *fp;
	int len;

	buffer[0] = '\0';
	snprintf(cmd, sizeof(cmd), "hostapd_cli -i%s wps_ap_pin get", get_wifname(unit));
	fp = popen(cmd, "r");
	if (fp) {
		len = fread(buffer, 1, sizeof(buffer), fp);
		pclose(fp);
		if (len > 1) {
			buffer[len] = '\0';
			//dbg("%s: AP PIN[%s]\n", __FUNCTION__, buffer);
			if(!strncmp(buffer,"FAIL",4))
			   strlcpy(buffer,nvram_get("secret_code"), sizeof(buffer));
			return buffer;
		}
	}
	return "";
#else
	snprintf(buffer, sizeof(buffer), "%s", nvram_safe_get("secret_code"));
	return buffer;
#endif
}

int
wl_wps_info(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int j = -1, u = unit;
	char tmpstr[128];
	WPS_CONFIGURED_VALUE result;
	int retval=0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *wps_sta_pin;
	char tag1[] = "<wps_infoXXXXXX>", tag2[] = "</wps_infoXXXXXX>";

#if defined(RTCONFIG_WPSMULTIBAND)
	for (j = -1; j < MAX_NR_WL_IF; ++j) {
#endif
		switch (j) {
		case WL_2G_BAND:	/* fall through */
		case WL_5G_BAND:	/* fall through */
		case WL_5G_2_BAND:	/* fall through */
		case WL_60G_BAND:	/* fall through */
			u = j;
			snprintf(tag1, sizeof(tag1), "<wps_info%d>", j);
			snprintf(tag2, sizeof(tag2), "</wps_info%d>", j);
			break;
		case -1: /* fall through */
		default:
			u = unit;
			strlcpy(tag1, "<wps_info>", sizeof(tag1));
			strlcpy(tag2, "</wps_info>", sizeof(tag2));
		}

		snprintf(prefix, sizeof(prefix), "wl%d_", u);

#if defined(RTCONFIG_WPSMULTIBAND)
		SKIP_ABSENT_BAND(u);
		if (!nvram_get(strcat_r(prefix, "ifname", tmp)))
			continue;
#endif

		memset(&result, 0, sizeof(result));
		getWPSConfig(u, &result);

		if (j == -1)
			retval += websWrite(wp, "<wps>\n");

		//0. WSC Status
		memset(tmpstr, 0, sizeof(tmpstr));
		strlcpy(tmpstr, getWscStatus(u), sizeof(tmpstr));
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//1. WPS Configured
		if (result.Configured==2)
			retval += websWrite(wp, "%s%s%s\n", tag1, "Yes", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, "No", tag2);

		//2. WPS SSID
		memset(tmpstr, 0, sizeof(tmpstr));
		char_to_ascii(tmpstr, result.SSID);
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//3. WPS AuthMode
		retval += websWrite(wp, "%s%s%s\n", tag1, result.AuthMode, tag2);

		//4. WPS Encryp
		retval += websWrite(wp, "%s%s%s\n", tag1, result.Encryp, tag2);

		//5. WPS DefaultKeyIdx
		memset(tmpstr, 0, sizeof(tmpstr));
		snprintf(tmpstr, sizeof(tmpstr), "%d", result.DefaultKeyIdx);/* FIXME: TBD */
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//6. WPS WPAKey
#if 0	//hide for security
		if (!strlen(result.WPAKey))
			retval += websWrite(wp, "%sNone%s\n", tag1, tag2);
		else
		{
			memset(tmpstr, 0, sizeof(tmpstr));
			char_to_ascii(tmpstr, result.WPAKey);
			retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);
		}
#else
		retval += websWrite(wp, "%s%s\n", tag1, tag2);
#endif
		//7. AP PIN Code
		memset(tmpstr, 0, sizeof(tmpstr));
		strlcpy(tmpstr, getAPPIN(u), sizeof(tmpstr));
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//8. Saved WPAKey
#if 0	//hide for security
		if (!strlen(nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp))))
			retval += websWrite(wp, "%s%s%s\n", tag1, "None", tag2);
		else
		{
			char_to_ascii(tmpstr, nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));
			retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);
		}
#else
		retval += websWrite(wp, "%s%s\n", tag1, tag2);
#endif
		//9. WPS enable?
		if (!strcmp(nvram_safe_get(strcat_r(prefix, "wps_mode", tmp)), "enabled"))
			retval += websWrite(wp, "%s%s%s\n", tag1, "None", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, nvram_safe_get("wps_enable"), tag2);

		//A. WPS mode
		wps_sta_pin = nvram_safe_get("wps_sta_pin");
		if (strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000"))
			retval += websWrite(wp, "%s%s%s\n", tag1, "1", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, "2", tag2);

		//B. current auth mode
		if (!strlen(nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp))))
			retval += websWrite(wp, "%s%s%s\n", tag1, "None", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp)), tag2);

		//C. WPS band
		retval += websWrite(wp, "%s%d%s\n", tag1, u, tag2);
#if defined(RTCONFIG_WPSMULTIBAND)
	}
#endif

	retval += websWrite(wp, "</wps>");

	return retval;
}

int
ej_wps_info(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_wps_info(eid, wp, argc, argv, 1);
}

int
ej_wps_info_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_wps_info(eid, wp, argc, argv, 0);
}

// Wireless Client List		 /* Start --Alicia, 08.09.23 */

struct ej_wl_auth_list_priv_s {
	int firstRow;
	webs_t wp;
};

/* Helper of ej_wl_auth_list()
 * @src:	pointer to WLANCONFIG_LIST
 * @arg:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int handle_ej_wl_auth_list(const WLANCONFIG_LIST *src, void *arg)
{
	struct ej_wl_auth_list_priv_s *priv = arg;
	char *value;

	if (!src || !arg)
		return -1;

	if (priv->firstRow == 1)
		priv->firstRow = 0;
	else
		websWrite(priv->wp, ", ");

	websWrite(priv->wp, "[");

	websWrite(priv->wp, "\"%s\"", src->addr);
	value = "YES";
	websWrite(priv->wp, ", \"%s\"", value);
	value = "";
	websWrite(priv->wp, ", \"%s\"", value);
	websWrite(priv->wp, "]");

	return 0;
}

int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv)
{
//only for ath0 & ath1
	int unit, ret = 0;
	char ifname[IFNAMSIZ], *next;
	struct ej_wl_auth_list_priv_s priv = { .firstRow = 1, .wp = wp };

	unit = 0;
	foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
		if (unit >= MAX_NR_WL_IF)
			break;
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

		__get_qca_sta_info_by_ifname(ifname, 0, handle_ej_wl_auth_list, &priv);
		++unit;
	}
	
	return ret;
}

#define target 7
char str[target][40]={"Address:","ESSID:","Frequency:","Quality=","Encryption key:","IE:","Authentication Suites"};
static int wl_scan(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
   	int apCount=0,retval=0;
	char header[128];
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char cmd[300];
	FILE *fp;
	char buf[target][200];
	int i,fp_len;
	char *pt1,*pt2;
	char a1[10],a2[10];
	char ssid_str[256];
	char ch[4] = "", ssid[33] = "", address[18] = "", enc[9] = "";
	char auth[16] = "", sig[9] = "", wmode[8] = "";
	int  lock;
#if defined(RTCONFIG_QCA_LBD)
	int restart_lbd = 0;
#endif

	dbg("Please wait...");
#if defined(RTCONFIG_QCA_LBD)
	if (nvram_match("smart_connect_x", "1") && pids("lbd")) {
		eval("rc", "rc_service", "stop_qca_lbd");
		restart_lbd = 1;
	}
#endif
	lock = file_lock("nvramcommit");
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	snprintf(cmd, sizeof(cmd), "iwlist %s scanning", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	fp = popen(cmd, "r");
	file_unlock(lock);
#if defined(RTCONFIG_QCA_LBD)
	if (restart_lbd) {
		eval("rc", "rc_service", "start_qca_lbd");
	}
#endif
	
	if (fp == NULL)
		return -1;
	
	memset(header, 0, sizeof(header));
	snprintf(header, sizeof(header), "%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode");

	dbg("\n%s", header);

	retval += websWrite(wp, "[");
	while(1)
	{
		memset(buf,0,sizeof(buf));
		fp_len=0;
		for(i=0;i<target;i++)
		{
		   	while(fgets(buf[i], sizeof(buf[i]), fp))
			{
				fp_len += strlen(buf[i]);  	
				if(i!=0 && strstr(buf[i],"Cell") && strstr(buf[i],"Address"))
				{
					fseek(fp,-fp_len, SEEK_CUR);
					fp_len=0;
					break;
				}
				else
			  	{ 	   
					if(strstr(buf[i],str[i]))
					{
					 	fp_len =0;  	
						break;
					}	
					else
						memset(buf[i],0,sizeof(buf[i]));
				}	

			}
		        	
	      		//dbg("buf[%d]=%s\n",i,buf[i]);
		}

  		if(feof(fp)) 
		   break;

		apCount++;

		dbg("\napCount=%d\n",apCount);
		//ch
	        pt1 = strstr(buf[2], "Channel ");	
		if(pt1)
		{

			pt2 = strstr(pt1,")");
		   	memset(ch,0,sizeof(ch));
			strlcpy(ch, pt1+strlen("Channel "), min(sizeof(ch), pt2-pt1-strlen("Channel ")+1));
		}   

		//ssid
	        pt1 = strstr(buf[1], "ESSID:");	
		if(pt1)
		{
		   	memset(ssid,0,sizeof(ssid));
			strlcpy(ssid, pt1+strlen("ESSID:")+1, min(sizeof(ssid), strlen(buf[1])-2-(pt1+strlen("ESSID:")+1-buf[1])+1));
		}   


		//bssid
	        pt1 = strstr(buf[0], "Address: ");	
		if(pt1)
		{
		   	memset(address,0,sizeof(address));
			strlcpy(address, pt1+strlen("Address: "), min(sizeof(address), strlen(buf[0])-(pt1+strlen("Address: ")-buf[0])-1+1));
		}   
	

		//enc
		pt1=strstr(buf[4],"Encryption key:");
		if(pt1)
		{   
			if(strstr(pt1+strlen("Encryption key:"),"on"))
			{  	
				strlcpy(enc, "ENC", sizeof(enc));
		
			} 
			else
				strlcpy(enc, "NONE", sizeof(enc));
		}

		//auth
		memset(auth,0,sizeof(auth));
		strlcpy(auth, "N/A", sizeof(auth));

		//sig
	        pt1 = strstr(buf[3], "Quality=");	
		pt2 = NULL;
		if (pt1 != NULL)
			pt2 = strstr(pt1,"/");
		if(pt1 && pt2)
		{
			memset(sig,0,sizeof(sig));
			memset(a1,0,sizeof(a1));
			memset(a2,0,sizeof(a2));
			strlcpy(a1, pt1+strlen("Quality="), min(sizeof(a1), pt2-pt1-strlen("Quality=")+1));
			strlcpy(a2, pt2+1, min(sizeof(a2), strstr(pt2," ")-(pt2+1)+1));
			snprintf(sig, sizeof(sig), "%d",safe_atoi(a1)/safe_atoi(a2));

		}   

		//wmode
		memset(wmode,0,sizeof(wmode));
		strlcpy(wmode, "11b/g/n", sizeof(wmode));


#if 1
		dbg("%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n",ch,ssid,address,enc,auth,sig,wmode);
#endif	


		memset(ssid_str, 0, sizeof(ssid_str));
		char_to_ascii(ssid_str, trim_r(ssid));
		if (apCount==1)
			retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, address);
		else
			retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, address);

	}

	retval += websWrite(wp, "]");
	pclose(fp);
	return 0;
}   

int
ej_wl_scan(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 0);
}

int
ej_wl_scan_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 0);
}

int
ej_wl_scan_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 1);
}

int
ej_wl_scan_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 2);
}


static int ej_wl_channel_list(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *country_code;
	char chList[256];

	if (absent_band(unit))
		return 0;

#if defined(RTCONFIG_LYRA_5G_SWAP)
#if defined(RTCONFIG_WIFI_SON)
	if(nvram_match("wifison_ready","1"))
		goto bypass;
	else
#endif
	{
		if(unit==1)
			unit=2;
		else if(unit==2)
			unit=1;
	}
bypass:
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	country_code = nvram_get(strcat_r(prefix, "country_code", tmp));

	if (country_code == NULL || strlen(country_code) != 2) return retval;

	//try getting channel list via wifi driver first
#if defined(RTAC58U) || defined(RTAC59U)
	if (unit == 0 && (!strncmp(nvram_safe_get("territory_code"), "CX/01", 5)
		       || !strncmp(nvram_safe_get("territory_code"), "CX/05", 5)))
		retval += websWrite(wp, "[1,2,3,4,5,6,7,8,9,10,11]");
	else
#endif
	if(get_channel_list_via_driver(unit, chList, sizeof(chList)) > 0)
	{
		retval += websWrite(wp, "[%s]", chList);
	}
	else if(get_channel_list_via_country(unit, country_code, chList, sizeof(chList)) > 0)
	{
		retval += websWrite(wp, "[%s]", chList);
	}
	return retval;
}


int
ej_wl_channel_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 0);
}

int
ej_wl_channel_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 1);
}

int
ej_wl_channel_list_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 2);
}

int
ej_wl_channel_list_60g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 3);
}


static int ej_wl_rate(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
#define ASUS_IOCTL_GET_STA_DATARATE (SIOCDEVPRIVATE+15) /* from qca-wifi/os/linux/include/ieee80211_ioctl.h */
        struct iwreq wrq;
	int retval = 0;
	char tmp[256], prefix[sizeof("wlXXXXXXXXXX_")];
	char *name;
	unsigned int rate[2];
	char rate_buf[32] = "0 Mbps";
	int sw_mode = sw_mode();
	int wlc_band = nvram_get_int("wlc_band");
	int from_app = 0;

	if (sw_mode != SW_MODE_REPEATER && sw_mode != SW_MODE_HOTSPOT)
		goto ERROR;
	if (absent_band(unit))
		goto ERROR;

	from_app = check_user_agent(user_agent);

	if (wlc_band < 0 || !nvram_match("wlc_state", "2"))
		goto ERROR;

#ifdef RTCONFIG_CONCURRENTREPEATER
	wlc_band = -1;
	snprintf(prefix, sizeof(prefix), "wlc%d_", unit);
	if (!nvram_pf_match(prefix, "state", "2"))
		goto ERROR;
#endif

	if (wlc_band >= WL_2G_BAND && wlc_band != unit)
		goto ERROR;

#if defined(RTCONFIG_REPEATER_STAALLBAND)
	name = get_staifname(nvram_get_int("wlc_triBand"));
#else
	snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
#endif

	wrq.u.data.pointer = rate;
	wrq.u.data.length = sizeof(rate);

	if (wl_ioctl(name, ASUS_IOCTL_GET_STA_DATARATE, &wrq) < 0)
	{
		dbg("%s: errors in getting %s ASUS_IOCTL_GET_STA_DATARATE result\n", __func__, name);
		goto ERROR;
	}

	if (rate[0] > rate[1])
		snprintf(rate_buf, sizeof(rate_buf), "%d Mbps", rate[0]);
	else
		snprintf(rate_buf, sizeof(rate_buf), "%d Mbps", rate[1]);

ERROR:
	if(from_app == 0)
		retval += websWrite(wp, "%s", rate_buf);
	else
		retval += websWrite(wp, "\"%s\"", rate_buf);
	return retval;
}


int
ej_wl_rate_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	if(sw_mode() == SW_MODE_REPEATER)
		return ej_wl_rate(eid, wp, argc, argv, 0);
	else
	   	return 0;
}

int
ej_wl_rate_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	if(sw_mode() == SW_MODE_REPEATER)
		return ej_wl_rate(eid, wp, argc, argv, 1);
	else
	   	return 0;
}

int
ej_wl_rate_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	if(sw_mode() == SW_MODE_REPEATER)
		return ej_wl_rate(eid, wp, argc, argv, 2);
	else
		return 0;
}

/* Check necessary kernel module only. */
static struct nat_accel_kmod_s {
	char *kmod_name;
} nat_accel_kmod[] = {
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074) || defined (RTCONFIG_SOC_IPQ60XX)
	{ "ecm" },
#elif defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X) || defined(RTCONFIG_SOC_IPQ40XX)
	{ "shortcut_fe" },
#else
#error Implement nat_accel_kmod[]
#endif
};

int
ej_nat_accel_status(int eid, webs_t wp, int argc, char_t **argv)
{
	int i, status = 1, retval = 0;
	struct nat_accel_kmod_s *p = &nat_accel_kmod[0];

	for (i = 0, p = &nat_accel_kmod[i]; status && i < ARRAY_SIZE(nat_accel_kmod); ++i, ++p) {
		if (module_loaded(p->kmod_name))
			continue;

		status = 0;
	}

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	/* Hardware NAT can be stopped via set non-zero value to below files.
	 * Don't claim hardware NAT is enabled if one of them is non-zero value.
	 */
	if (status) {
#if defined(RTCONFIG_SOC_IPQ8064)
		const char *v4_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv4/stop", *v6_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv6/stop";
#elif defined(RTCONFIG_SOC_IPQ8074)
		const char *v4_stop_fn = "/sys/kernel/debug/ecm/front_end_ipv4_stop", *v6_stop_fn = "/sys/kernel/debug/ecm/front_end_ipv6_stop";
#endif
		int s1, s2;
		char *str;

		s1 = s2 = 0;
		if((str = file2str(v4_stop_fn)) != NULL) {
			s1 = safe_atoi(str);
			free(str);
		}
		if((str = file2str(v6_stop_fn)) != NULL) {
			s2 = safe_atoi(str);
			free(str);
		}

		if (s1 != 0 || s2 != 0)
			status = 0;
	}
#endif

	retval += websWrite(wp, "%d", status);

	return retval;
}

#ifdef RTCONFIG_PROXYSTA
int
ej_wl_auth_psta(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	int psta = 0, psta_auth = 0;

	if(nvram_match("wlc_state", "2")){	//connected
		psta = 1;
		psta_auth = 0;
	//else if(?)				//authorization failed
	//	retval += websWrite(wp, "wlc_state=2;wlc_state_auth=1;");
	}else{					//disconnected
		psta = 0;
		psta_auth = 0;
	}

	if(json_support){
		retval += websWrite(wp, "{");
		retval += websWrite(wp, "\"wlc_state\":\"%d\"", psta);
		retval += websWrite(wp, ",\"wlc_state_auth\":\"%d\"", psta_auth);
		retval += websWrite(wp, "}");
	}else{
		retval += websWrite(wp, "wlc_state=%d;", psta);
		retval += websWrite(wp, "wlc_state_auth=%d;", psta_auth);
	}

	return retval;
}
#endif

const char *syslog_msg_filter[] = {
	"net_ratelimit",
#if defined(RTCONFIG_SOC_IPQ8074)
	"[AUTH] vap", "[MLME] vap", "[ASSOC] vap", "[INACT] vap", "LBDR ", "npu_corner", "apc_corner", "Sync active EEPROM set",
#endif
	NULL
};
