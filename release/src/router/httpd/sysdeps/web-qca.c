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
void convert_mac_string(char *mac);

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
	{ 0, NULL },
};

static void getWPSConfig(int unit, WPS_CONFIGURED_VALUE *result)
{
	char buf[128];
	FILE *fp;

	memset(result, 0, sizeof(result));

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

	p = result;
	len = sizeof(result);
	*p = '\0';
	sep = "11";
	for (q = &mode_tbl[0]; len > 0 && mask > 0 && q->mask; ++q) {
		if (!(mask & q->mask))
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
	else if (!strncmp(mode, "AUTO", 4)) {
		if (!strcmp(iface, get_staifname(0))) {
			sta = 1;
			m = WL_N | WL_G | WL_B;
		}
		else if (!strcmp(iface, get_staifname(1))
#if defined(RTCONFIG_HAS_5G_2)
			 || !strcmp(iface, get_staifname(2))	/* for 2-nd 5GHz */
#endif
#if defined(RTCONFIG_WIGIG)
			 || !strcmp(iface, get_staifname(3))	/* for 802.11ad Wigig */
#endif
			) {
			sta = 1;
			m = WL_AC | WL_N | WL_A;
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
	static char result[sizeof("11b/g/nXXXXXX")] = "";
	int r;
	FILE *fp;
	unsigned int m = 0;
	char cmd[sizeof("iwinfo wlan0 infoXXXXXX")], line[256], *pmode;

	if (!ifname || *ifname == '\0') {
		dbg("%s: got invalid ifname %p\n", __func__, ifname);
		return 0;
	}

	snprintf(cmd, sizeof(cmd), "iwinfo %s info", ifname);
	fp = popen(cmd, "r");
	if (!fp)
		return 0;

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
	while (fgets(line, sizeof(line), fp)) {
		if (!strstr(line, "HW Mode"))
			continue;
		if ((r = sscanf(line, "%*[^:]:%*[^:]: %ms", &pmode)) != 1) {
			_dprintf("%s: Unknown phy mode of ifname [%s]: [%s]\n",
				__func__, ifname, line);
			continue;
		}
		break;
	}
	pclose(fp);

	if (!pmode)
		return result;

	if (!strcmp(pmode, "802.11/ad"))
		m = WL_AD;
	else
		dbg("%s: unknown phy mode string [%s]\n", __func__, pmode);

	free(pmode);

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
	char *pt1, *pt2, ch_mhz[5], ch_mhz_t[5];

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
	r = f_read_string("/sys/kernel/debug/ieee80211/phy0/wil6210/freq", buf, sizeof(buf));
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

	if (unit == WL_60G_BAND) {
		snprintf(buf, sizeof(buf), "/sys/kernel/debug/ieee80211/%s/wil6210/ssid", get_vphyifname(unit));
		f_read_string(buf, ssid, sizeof(ssid));
	} else {
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

typedef struct _WLANCONFIG_LIST {
	char addr[18];
	unsigned int aid;
	unsigned int chan;
	char txrate[10];
	char rxrate[10];
	int rssi;
	unsigned int idle;
	unsigned int txseq;
	unsigned int rxseq;
	char caps[12];
	char acaps[10];
	char erp[7];
	char state_maxrate[20];
	char wps[4];
	char conn_time[12];
	char rsn[4];
	char wme[4];
	char mode[31];
	char ie[32];
	char htcaps[10];
	unsigned int u_acaps;
	unsigned int u_erp;
	unsigned int u_state_maxrate;
	unsigned int u_psmode;
	char subunit;	/* '0': main 2G/5G network, '1' ~ '7': Guest network (MAX_NO_MSSID = 8), 'B': Facebook Wi-Fi, 'F': Free Wi-Fi, 'C': Captive Portal */
} WLANCONFIG_LIST;

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || defined(RTCONFIG_WIFI_QCA9994_QCA9994)
#define MAX_STA_NUM 512
#else
#define MAX_STA_NUM 256
#endif

typedef struct _WIFI_STA_TABLE {
	int Num;
	WLANCONFIG_LIST Entry[ MAX_STA_NUM ];
} WIFI_STA_TABLE;

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
#define STA_INFO_PATH "/tmp/wlanconfig_athX_list"
static int __getSTAInfo(int unit, WIFI_STA_TABLE *sta_info, char *ifname, char id)
{
	FILE *fp;
	int l2_offset, subunit, channf, ax2he = 0;
	char *l2, *l3, *p;
	char line_buf[300]; // max 14x
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

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	if (!find_word(nvram_safe_get("rc_support"), "11AX"))
		ax2he = 1;
#endif
	channf = QCA_DEFAULT_NOISE_FLOOR;

	snprintf(wlif, sizeof(wlif), "wl%d.%d", unit, subunit);
	if (subunit >= 0 && subunit < MAX_NO_MSSID)
		snprintf(subunit_str, sizeof(subunit_str), "%d", subunit);
	if (id == 'B' || id == 'F' || id == 'C')
		snprintf(subunit_str, sizeof(subunit_str), "%c", id);

	doSystem("wlanconfig %s list > %s", ifname, STA_INFO_PATH);
	fp = fopen(STA_INFO_PATH, "r");
	if (fp) {
/* wlanconfig ath1 list
ADDR               AID CHAN TXRATE RXRATE RSSI IDLE  TXSEQ  RXSEQ  CAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS ASSOCTIME    IEs   MODE PSMODE
00:10:18:55:cc:08    1  149  55M   1299M   63    0      0   65535               0        807              0              Q 00:10:33 IEEE80211_MODE_11A  0
08:60:6e:8f:1e:e6    2  149 159M    866M   44    0      0   65535     E         0          b              0           WPSM 00:13:32 WME IEEE80211_MODE_11AC_VHT80  0
08:60:6e:8f:1e:e8    1  157 526M    526M   51 4320      0   65535    EP         0          b              0          AWPSM 00:00:10 RSN WME IEEE80211_MODE_11AC_VHT80 0
*/
		//fseek(fp, 131, SEEK_SET);	// ignore header
		fgets(line_buf, sizeof(line_buf), fp); // ignore header
		l2 = strstr(line_buf, "ACAPS");
		if (l2 != NULL)
			l2_offset = (int)(l2 - line_buf);
		else {
			l2_offset = 79;
			l2 = line_buf + l2_offset;
		}
		while ( fgets(line_buf, sizeof(line_buf), fp) ) {
			WLANCONFIG_LIST *r = &sta_info->Entry[sta_info->Num++];

			r->subunit = subunit_str[0];
			/* IEs may be empty string, find IEEE80211_MODE_ before parsing mode and psmode. */
			l3 = strstr(line_buf, "IEEE80211_MODE_");
			if (l3) {
				*(l3 - 1) = '\0';
				sscanf(l3, "IEEE80211_MODE_%s %d", r->mode, &r->u_psmode);
				if (ax2he) {
					if ((p = strstr(r->mode, "11AXA")) != NULL)
						memcpy(p, "11AHE", 5);
					else if ((p = strstr(r->mode, "11AXG")) != NULL)
						memcpy(p, "11GHE", 5);
				}
			}
			*(l2 - 1) = '\0';
			sscanf(line_buf, "%s%u%u%s%s%u%u%u%u%[^\n]",
				r->addr, &r->aid, &r->chan, r->txrate,
				r->rxrate, &r->rssi, &r->idle, &r->txseq,
				&r->rxseq, r->caps);
			sscanf(l2, "%u%x%u%s%s%[^\n]",
				&r->u_acaps, &r->u_erp, &r->u_state_maxrate, r->htcaps, r->conn_time, r->ie);
			if (strlen(r->rxrate) >= 6)
				strcpy(r->rxrate, "0M");
			convert_mac_string(r->addr);
			r->rssi += channf;
#if 0
			dbg("[%s][%u][%u][%s][%s][%u][%u][%u][%u][%s]"
				"[%u][%u][%x][%s][%s][%s][%d]\n",
				r->addr, r->aid, r->chan, r->txrate, r->rxrate,
				r->rssi, r->idle, r->txseq, r->rxseq, r->caps,
				r->u_acaps, r->u_erp, r->u_state_maxrate, r->htcaps, r->ie,
				r->mode, r->u_psmode);
#endif
		}

		fclose(fp);
		unlink(STA_INFO_PATH);
	}
	return 0;
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
	int c, subunit, time_val, hr, min, sec;
	char rate[6], line_buf[300];
	char subunit_str[4] = "0", wlif[sizeof("wlX.Yxxx")];
	char cmd[sizeof("iw wlan0 station dump XXXXXX")];
	WLANCONFIG_LIST *r;


	if (unit < 0 || unit >= MAX_NR_WL_IF)
		return -1;
	if (!ifname || *ifname == '\0')
		return -1;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		return -1;
#endif
#if !defined(RTCONFIG_WIGIG)
	if (unit == 3)
		return -1;
#endif

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
	 * Example: iw wlan0 station dump
	 * Station 04:ce:14:0a:21:17 (on wlan0)
	 *       rx bytes:       0
	 *       rx packets:     0
	 *       tx bytes:       0
	 *       tx packets:     0
	 *       tx failed:      0
	 *       tx bitrate:     27.5 MBit/s MCS 0
	 *       rx bitrate:     27.5 MBit/s MCS 0
	 *       connected time: 292 seconds
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
		r->subunit = subunit_str[0];
		strlcpy(r->mode, "11ad", sizeof(r->mode));
		while (fgets(line_buf, sizeof(line_buf), fp)) {
			if (!strncmp(line_buf, "Station", 7)) {

#if 0
				dbg("[%s][%u][%u][%s][%s][%u][%u][%u][%u][%s]"
					"[%u][%u][%x][%s][%s][%s][%d]\n",
					r->addr, r->aid, r->chan, r->txrate, r->rxrate,
					r->rssi, r->idle, r->txseq, r->rxseq, r->caps,
					r->u_acaps, r->u_erp, r->u_state_maxrate, r->htcaps, r->ie,
					r->mode, r->u_psmode);
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

	if (unit < 0 || unit >= MAX_NR_WL_IF || !ifnames
	    || strncmp(ifnames, "wl", 2) || strlen(ifnames) < 5 || !sta_info)
		return -1;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		return -1;
#endif
#if !defined(RTCONFIG_WIGIG)
	if (unit == 3)
		return -1;
#endif

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

	if (unit < 0 || unit >= MAX_NR_WL_IF || !sta_info)
		return -1;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		return -1;
#endif
#if !defined(RTCONFIG_WIGIG)
	if (unit == 3)
		return -1;
#endif

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

	if (unit < 0 || unit >= MAX_NR_WL_IF || !sta_info)
		return -1;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		return -1;
#endif
#if !defined(RTCONFIG_WIGIG)
	if (unit == 3)
		return -1;
#endif

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

	if (unit < 0 || unit >= MAX_NR_WL_IF || unit >= ARRAY_SIZE(fbwifi_iface)|| !sta_info)
		return -1;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		return -1;
#endif
#if !defined(RTCONFIG_WIGIG)
	if (unit == 3)
		return -1;
#endif

	/* Facebook Wi-Fi */
	if (!nvram_match("fbwifi_enable", "on"))
		return 0;

	getNonStdGuestSTAInfo(unit, nvram_safe_get(fbwifi_iface[unit]), sta_info, 'B');

	return 0;
}
#else
static inline int getFacebookWiFiSTAInfo(int unit, WIFI_STA_TABLE *sta_info) { return 0; }
#endif

void
convert_mac_string(char *mac)
{
	int i;
	char mac_str[18], mac_str_t[18];
	memset(mac_str,0,sizeof(mac_str));

	for(i=0;i<strlen(mac);i++)
	{
		if(*(mac+i)>0x60 && *(mac+i)<0x67){
			snprintf(mac_str_t, sizeof(mac_str), "%s%c",mac_str,*(mac+i)-0x20);
			strlcpy(mac_str, mac_str_t, sizeof(mac_str));
		}
		else{
			snprintf(mac_str_t, sizeof(mac_str), "%s%c",mac_str,*(mac+i));
			strlcpy(mac_str, mac_str_t, sizeof(mac_str));
		}

	}
	strlcpy(mac, mac_str, strlen(mac_str) + 1);
}

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

	if (unit < 0 || unit >= MAX_NR_WL_IF)
		return 0;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		return 0;
#endif
#if !defined(RTCONFIG_WIGIG)
	if (unit == 3)
		return 0;
#endif
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
	wl_ifnames = strdup(nvram_safe_get("lan_ifnames"));
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
		if ((fp = popen(cmd, "r")) != NULL && fread(tmpstr, 1, sizeof(tmpstr), fp) > 1) {
			pclose(fp);
			*(tmpstr + sizeof(tmpstr) - 1) = '\0';
			*ap_bssid = '\0';
			if ((p = strstr(tmpstr, "addr ")) != NULL) {
				strlcpy(ap_bssid, p + 5, sizeof(ap_bssid));
				ap_bssid[17] = '\0';
			}
			convert_mac_string(ap_bssid);
		}
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
	ret += websWrite(wp, "Channel		: %u\n", getAPChannel(unit));

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
			ret += show_wliface_info(wp, unit, ifname, "Repeater");
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
#if defined(MAPAC2200) || defined(RTAC92U)
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
				if (sta_info->Entry[i].subunit == '0')
					strlcpy(subunit_str, "Main", sizeof(subunit_str));
				else if (isdigit(sta_info->Entry[i].subunit))
					snprintf(subunit_str, sizeof(subunit_str), "Guest Network-%c", sta_info->Entry[i].subunit);
				else if (sta_info->Entry[i].subunit == 'B')
					strlcpy(subunit_str, "Facebook Wi-Fi", sizeof(subunit_str));
				else if (sta_info->Entry[i].subunit == 'F')
					strlcpy(subunit_str, "Free Wi-Fi", sizeof(subunit_str));
				else if (sta_info->Entry[i].subunit == 'C')
					strlcpy(subunit_str, "Captive Portal", sizeof(subunit_str));
				else {
					dbg("%s: Unknown subunit [%s]\n", sta_info->Entry[i].subunit);
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
#if defined(MAPAC2200) || defined(RTAC92U)
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
		s = r->subunit;
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
				case 0:
					if (i == 0)
						strlcpy(alias, "2G", sizeof(alias));
					else
						snprintf(alias, sizeof(alias), "%s_%d", "2G", i);
					break;
				case 1:	/* fall-through */
#if defined(RTCONFIG_HAS_5G_2)
				case 2:
#endif
					if (i == 0)
						snprintf(alias, sizeof(alias), "%s", unit == 2 ? "5G1" : "5G");
					else
						snprintf(alias, sizeof(alias), "%s_%d", unit == 2 ? "5G1" : "5G", i);
					break;
#if defined(RTCONFIG_WIGIG)
				case 3:
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
				s = r->subunit;
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
#if !defined(RTCONFIG_HAS_5G)
		if (unit == 1)
			continue;
#endif
#if !defined(RTCONFIG_HAS_5G_2)
		if (unit == 2)
			continue;
#endif
#if !defined(RTCONFIG_WIGIG)
		if (unit == 3)
			continue;
#endif
#endif
		switch (j) {
		case 0: /* fall through */
		case 1: /* fall through */
		case 2: /* fall through */
		case 3: /* fall through */
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

int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv)
{
//only for ath0 & ath1
        WLANCONFIG_LIST *result;
	#define AUTH_INFO_PATH "/tmp/auth_athX_list"
	FILE *fp;
	int unit, firstRow, ret = 0;
	char line_buf[300]; // max 14x
	char *value;
	char ifname[100], *next;
	result = (WLANCONFIG_LIST *)malloc(sizeof(WLANCONFIG_LIST));
	memset(result, 0, sizeof(result));

	unit = 0;
	firstRow = 1;
	foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
		if (unit >= MAX_NR_WL_IF)
			break;
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

		doSystem("wlanconfig %s list > %s", ifname, AUTH_INFO_PATH);
		fp = fopen(AUTH_INFO_PATH, "r");
		if (!fp) {
			++unit;
			continue;
		}
		fgets(line_buf, sizeof(line_buf), fp); // ignore header
		while ( fgets(line_buf, sizeof(line_buf), fp) ) {
			sscanf(line_buf, "%s%u%u%s%s%u%u%u%u%s%s%s%s%s%s%s%s%s",
				result->addr, &result->aid, &result->chan,
				result->txrate, result->rxrate, &result->rssi,
				&result->idle, &result->txseq, &result->rxseq,
				result->caps, result->acaps, result->erp,
				result->state_maxrate, result->wps,
				result->conn_time, result->rsn, result->wme,
				result->mode);
			
			if (firstRow == 1)
				firstRow = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "[");

			websWrite(wp, "\"%s\"", result->addr);
			value = "YES";
			websWrite(wp, ", \"%s\"", value);
			value = "";
			websWrite(wp, ", \"%s\"", value);
			websWrite(wp, "]");
		
		}

		fclose(fp);
		unlink(AUTH_INFO_PATH);
		++unit;
	}
	
	free(result);

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

	dbg("Please wait...");
	lock = file_lock("nvramcommit");
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	snprintf(cmd, sizeof(cmd), "iwlist %s scanning", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	fp = popen(cmd, "r");
	file_unlock(lock);
	
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

	if (unit < 0 || unit >= MAX_NR_WL_IF)
		return 0;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		return 0;
#endif
#if !defined(RTCONFIG_WIGIG)
	if (unit == 3)
		return 0;
#endif

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
#if defined(RTAC58U)
	if (unit == 0 && !strncmp(nvram_safe_get("territory_code"), "CX", 2))
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
	int unit_max = MAX_NR_WL_IF;
	unsigned int rate[2];
	char rate_buf[32] = "0 Mbps";
	int sw_mode = sw_mode();
	int wlc_band = nvram_get_int("wlc_band");
	int from_app = 0;

	if (sw_mode != SW_MODE_REPEATER && sw_mode != SW_MODE_HOTSPOT)
		goto ERROR;

	from_app = check_user_agent(user_agent);

	if (unit > (unit_max - 1))
		goto ERROR;
#if !defined(RTCONFIG_HAS_5G_2)
	if (unit == 2)
		goto ERROR;
#endif
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
#if defined(RTCONFIG_SOC_IPQ8064)
	{ "ecm" },
#elif defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_SOC_IPQ40XX)
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

#if defined(RTCONFIG_SOC_IPQ8064)
	/* Hardware NAT can be stopped via set non-zero value to below files.
	 * Don't claim hardware NAT is enabled if one of them is non-zero value.
	 */
	if (status) {
		const char *v4_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv4/stop", *v6_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv6/stop";
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
