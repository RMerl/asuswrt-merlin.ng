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
#include <lantiq.h>
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


/* shared/sysdeps/api-qca.c */
extern u_int ieee80211_mhz2ieee(u_int freq);

#define WL_A		(1U << 0)
#define WL_B		(1U << 1)
#define WL_G		(1U << 2)
#define WL_N		(1U << 3)
#define WL_AC		(1U << 4)

static const struct mode_s {
	unsigned int mask;
	char *mode;
} mode_tbl[] = {
	{ WL_A,	"a" },
	{ WL_B,	"b" },
	{ WL_G, "g" },
	{ WL_N, "n" },
	{ WL_AC, "ac" },
	{ 0, NULL },
};

/* protect bit rate error code */
int isnumber(const char*s, float *fRate) {
	char* e = NULL;
	*fRate = strtof(s, &e);
	*fRate *= 10;
	return e != NULL && *e == (char)0;
}

/**
 * Run "iwpriv XXX get_XXX" and return string behind colon.
 * Expected result is shown below:
 * ath1      get_mode:11ACVHT40
 *                    ^^^^^^^^^
 * @iface:	interface name
 * @cmd:	get cmd
 * @buf:	pointer to memory area which is used to keep result.
 * 		it is guarantee as valid/empty string, if it is valid pointer
 * @buf_len:	length of @buf
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	run iwpriv command fail
 *     -3:	read result strin fail
 */
static int __iwpriv_get(const char *iface, char *cmd, char *buf, unsigned int buf_len)
{
	int len;
	FILE *fp;
	char *p = NULL, iwpriv_cmd[64], tmp[128];

	if (!iface || *iface == '\0' || !cmd || *cmd == '\0' || !buf || buf_len <= 1)
		return -1;

	if (strncmp(cmd, "get_", 4) && strncmp(cmd, "g_", 2)) {
		dbg("%s: iface [%s] cmd [%s] may not be supported!\n",
			__func__, iface, cmd);
	}

	*buf = '\0';
	sprintf(iwpriv_cmd, "iwpriv %s %s", iface, cmd);
	if (!(fp = popen(iwpriv_cmd, "r")))
		return -2;

	len = fread(tmp, 1, sizeof(tmp), fp);
	pclose(fp);
	if (len < 1)
		return -3;

	tmp[len-1] = '\0';
	if (!(p = strchr(tmp, ':'))) {
		dbg("%s: parsing [%s] of cmd [%s] error!", __func__, tmp, cmd);
		return -4;
	}
	p++;
	chomp(p);
	strlcpy(buf, p, buf_len);

	return 0;
}

/**
 * Run "iwpriv XXX get_XXX" and return string behind colon.
 * Expected result is shown below:
 * ath1      get_mode:11ACVHT40
 *                    ^^^^^^^^^ result
 * @iface:	interface name
 * @cmd:	get cmd
 * @return:
 * 	NULL	invalid parameter or error.
 *  otherwise:	success
 */
static char *iwpriv_get(const char *iface, char *cmd)
{
	static char result[256];

	if (__iwpriv_get(iface, cmd, result, sizeof(result)))
		return NULL;

	return result;
}

static int get_wlsubnet(int band, const char *ifname)
{
	int subnet, sidx;

	for (subnet = 0, sidx = 0; subnet < MAX_NO_MSSID; subnet++)
	{
		if(!nvram_match(wl_nvname("bss_enabled", band, subnet), "1")) {
			if (!subnet)
				sidx++;
			continue;
		}

		if(strcmp(ifname, wl_vifname_wave(band, sidx)) == 0)
			return subnet;

		sidx++;
	}
	return -1;
}

unsigned int getAPChannelbyIface(const char *ifname)
{
	int skfd;
	struct iwreq		wrq;
	struct iw_range	range;
	double		freq;
	int			channel;
	char			buffer[128];	/* Temporary buffer */
	char	vbuf[16];

  /* Create a channel to the NET kernel. */
  if((skfd = iw_sockets_open()) < 0){
      perror("socket");
      return -1;
	}

  /* Get list of frequencies / channels */
  if(iw_get_range_info(skfd, ifname, &range) < 0){
      fprintf(stderr, "%-8.16s  no frequency information.\n\n", ifname);
	}

	/* Get current frequency / channel and display it */
	if(iw_get_ext(skfd, ifname, SIOCGIWFREQ, &wrq) >= 0){
		/* remove IW_FREQ_FIXED */
		wrq.u.freq.flags = wrq.u.freq.flags & 0xFE;
	  freq = iw_freq2float(&(wrq.u.freq));
		if(freq > KILO)
			freq = iw_freq_to_channel(freq, &range);
		/* vbuf is channel */
		iw_print_freq_value(vbuf, sizeof(vbuf), freq);
	}
	/* Close the socket. */
	iw_sockets_close(skfd);

	return atoi(vbuf);
}

unsigned int getAPChannel(int unit)
{
	return getAPChannelbyIface(get_wififname(unit));
}

/**
 * Return SSID of a interface.
 * @return:	Don't return NULL even interface name is invalid or interface absent.
 */
char* getSSIDbyIFace(const char *ifname)
{
	static char ssid[33] = "";
	char buf[8192] = "";
	FILE *fp;
	int len;
	char *pt1, *pt2, *pt3;

	if (!ifname || *ifname == '\0') {
		dbg("%S: got invalid ifname %p\n", __func__, ifname);
		return ssid;
	}

	sprintf(buf, "iwconfig %s", ifname);
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

	return ssid;
}

int
ej_wl_control_channel(int eid, webs_t wp, int argc, char_t **argv)
{
        int ret = 0;
        int channel_24 = 0, channel_50 = 0;
        
	channel_24 = getAPChannel(0);

        if (!(channel_50 = getAPChannel(1)))
                ret = websWrite(wp, "[\"%d\", \"%d\"]", channel_24, 0);
        else
                ret = websWrite(wp, "[\"%d\", \"%d\"]", channel_24, channel_50);
	
        return ret;
}

typedef struct _WLANCONFIG_LIST {
	char addr[18];
	unsigned int aid;
	unsigned int chan;
	char txrate[7];
	char rxrate[10];
	unsigned int rssi;
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
	int subunit;
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

void
convert_mac_string(char *mac)
{
	int i;
	char mac_str[18], mac_str2[18];
	memset(mac_str,0,sizeof(mac_str));
	memset(mac_str2,0,sizeof(mac_str2));

	for(i=0;i<strlen(mac);i++)
        {
                if(*(mac+i)>0x60 && *(mac+i)<0x67) {
			snprintf(mac_str2, sizeof(mac_str2), "%s%c",mac_str,*(mac+i)-0x20);
			strlcpy(mac_str, mac_str2, sizeof(mac_str2));
                } else {
			snprintf(mac_str2, sizeof(mac_str2), "%s%c",mac_str,*(mac+i));
			strlcpy(mac_str, mac_str2, sizeof(mac_str2));
		}
        }
	strlcpy(mac, mac_str, strlen(mac_str) + 1);
}

static int getSTAInfo(int unit, WIFI_STA_TABLE *sta_info)
{
	#define STA_INFO_PATH "/tmp/iw_wlanX_list"
	FILE *fp;
	int ret = 0, subunit;
	char *unit_name;
	char *p, *ifname;
	char *wl_ifnames;
	char line_buf[300];
	char rssi_char[5];
	float tTx, tRx;

	memset(sta_info, 0, sizeof(*sta_info));
	unit_name = strdup(get_wififname(unit));
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
		if(strncmp(ifname,unit_name,strlen(unit_name)))
			continue;

		subunit = get_wlsubnet(unit, ifname);
		if (subunit < 0)
			subunit = 0;

		doSystem("iw dev %s station dump > %s", ifname, STA_INFO_PATH);
		fp = fopen(STA_INFO_PATH, "r");
		if (fp) {
/* iw dev wlan0 station dump
	Station b0:48:1a:ce:f2:13 (on wlan0)
		inactive time:	9310 ms
		rx bytes:	13170
		rx packets:	187
		tx bytes:	30080
		tx packets:	53
		signal:		-35 dBm
		tx bitrate:	11.5 MBit/s
		rx bitrate:	13.0 MBit/s
*/
			while ( fgets(line_buf, sizeof(line_buf), fp) ) {
				if(strstr(line_buf, "Station")) {
					WLANCONFIG_LIST *r = &sta_info->Entry[sta_info->Num++];
					sscanf(line_buf, "%*s%s", r->addr);
					r->subunit = subunit;
					while ( fgets(line_buf, sizeof(line_buf), fp) ) {
						if(strstr(line_buf, "signal")) {
							sscanf(line_buf, "%*s%s", rssi_char);
							r->rssi = atoi(rssi_char);
						}
						else if(strstr(line_buf, "tx bitrate")) {
							sscanf(line_buf, "%*s%*s%s", r->txrate);
							if(!isnumber(r->txrate, &tTx))
								memset(r->txrate, 0x00, sizeof(r->txrate));
							else
								snprintf(r->txrate, sizeof(r->txrate), "%0.f", tTx);
						}
						else if(strstr(line_buf, "rx bitrate")) {
							sscanf(line_buf, "%*s%*s%s", r->rxrate);
							if(!isnumber(r->rxrate, &tRx))
								memset(r->rxrate, 0x00, sizeof(r->rxrate));
							else
								snprintf(r->rxrate, sizeof(r->rxrate), "%0.f", tRx);
							break;
						}
					}
					convert_mac_string(r->addr);
#if 0
					dbg("[%s][%s][%s][%d]\n",
						r->addr, r->txrate, r->rxrate, r->rssi);
#endif
				}
			}

			fclose(fp);
			unlink(STA_INFO_PATH);
		}
	}
	free(wl_ifnames);
	free(unit_name);
	return ret;
}

char* GetBW(int BW)
{
	switch(BW)
	{
		case BW_10:
			return "10M";

		case BW_20:
			return "20M";

		case BW_40:
			return "40M";

#if defined(RTAC52U) || defined(RTAC51U)
		case BW_80:
			return "80M";
#endif

		default:
			return "N/A";
	}
}

int MCSMappingRateTable[] =
	{2,  4,   11,  22, // CCK
	12, 18,   24,  36, 48, 72, 96, 108, // OFDM
	13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260, // 20MHz, 800ns GI, MCS: 0 ~ 15
	39, 78,  117, 156, 234, 312, 351, 390,										  // 20MHz, 800ns GI, MCS: 16 ~ 23
	27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540, // 40MHz, 800ns GI, MCS: 0 ~ 15
	81, 162, 243, 324, 486, 648, 729, 810,										  // 40MHz, 800ns GI, MCS: 16 ~ 23
	14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288, // 20MHz, 400ns GI, MCS: 0 ~ 15
	43, 87,  130, 173, 260, 317, 390, 433,										  // 20MHz, 400ns GI, MCS: 16 ~ 23
	30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600, // 40MHz, 400ns GI, MCS: 0 ~ 15
	90, 180, 270, 360, 540, 720, 810, 900,
	13, 26,   39,  52,  78, 104, 117, 130, 156, /* 11ac: 20Mhz, 800ns GI, MCS: 0~8 */
	27, 54,   81, 108, 162, 216, 243, 270, 324, 360, /*11ac: 40Mhz, 800ns GI, MCS: 0~9 */
	59, 117, 176, 234, 351, 468, 527, 585, 702, 780, /*11ac: 80Mhz, 800ns GI, MCS: 0~9 */
	14, 29,   43,  57,  87, 115, 130, 144, 173, /* 11ac: 20Mhz, 400ns GI, MCS: 0~8 */
	30, 60,   90, 120, 180, 240, 270, 300, 360, 400, /*11ac: 40Mhz, 400ns GI, MCS: 0~9 */
	65, 130, 195, 260, 390, 520, 585, 650, 780, 867 /*11ac: 80Mhz, 400ns GI, MCS: 0~9 */
	};


#define FN_GETRATE(_fn_, _st_)						\
_fn_(_st_ HTSetting)							\
{									\
	int rate_count = sizeof(MCSMappingRateTable)/sizeof(int);	\
	int rate_index = 0;						\
									\
	if (HTSetting.field.MODE >= MODE_VHT)				\
	{								\
		if (HTSetting.field.BW == BW_20) {			\
			rate_index = 108 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
		else if (HTSetting.field.BW == BW_40) {			\
			rate_index = 117 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
		else if (HTSetting.field.BW == BW_80) {			\
			rate_index = 127 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
	}								\
	else								\
	if (HTSetting.field.MODE >= MODE_HTMIX)				\
	{								\
		rate_index = 12 + ((unsigned char)HTSetting.field.BW *24) + ((unsigned char)HTSetting.field.ShortGI *48) + ((unsigned char)HTSetting.field.MCS);	\
	}								\
	else								\
	if (HTSetting.field.MODE == MODE_OFDM)				\
		rate_index = (unsigned char)(HTSetting.field.MCS) + 4;	\
	else if (HTSetting.field.MODE == MODE_CCK)			\
		rate_index = (unsigned char)(HTSetting.field.MCS);	\
									\
	if (rate_index < 0)						\
		rate_index = 0;						\
									\
	if (rate_index >= rate_count)					\
		rate_index = rate_count-1;				\
									\
	return (MCSMappingRateTable[rate_index] * 5)/10;		\
}

int FN_GETRATE(getRate,      MACHTTRANSMIT_SETTING)		//getRate(MACHTTRANSMIT_SETTING)
int FN_GETRATE(getRate_2g,   MACHTTRANSMIT_SETTING_2G)		//getRate_2g(MACHTTRANSMIT_SETTING_2G)
#if defined(RTAC52U) || defined(RTAC51U)
int FN_GETRATE(getRate_11ac, MACHTTRANSMIT_SETTING_11AC)	//getRate_11ac(MACHTTRANSMIT_SETTING_11AC)
#endif



int
ej_wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	int ii = 0;
	char word[256], *next;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
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
	struct iwreq		wrq;
	struct iw_range	range;
	int skfd;
	double	freq;
	int		channel;
	char	buffer[128];	/* Temporary buffer */
	char	vbuf[16];
	struct ifreq ifr;

	if (unit < 0 || !ifname || !op_mode)
		return 0;

	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0){
		perror("socket");
		return -1;
	}

	memset(&mac_addr, 0, sizeof(mac_addr));
	get_iface_hwaddr(ifname, mac_addr);
	ret += websWrite(wp, "=======================================================================================\n"); // separator
	ret += websWrite(wp, "OP Mode		: %s\n", op_mode);
	ret += websWrite(wp, "SSID		: %s\n", getSSIDbyIFace(ifname));
	sprintf(cmd, "iwconfig %s", ifname);
	if ((fp = popen(cmd, "r")) != NULL && fread(tmpstr, 1, sizeof(tmpstr), fp) > 1) {
		pclose(fp);
		*(tmpstr + sizeof(tmpstr) - 1) = '\0';
		*ap_bssid = '\0';
		if ((p = strstr(tmpstr, "Access Point: ")) != NULL) {
			strncpy(ap_bssid, p + 14, 17);
			ap_bssid[17] = '\0';
		}
		ret += websWrite(wp, "BSSID		: %s\n", ap_bssid);
	}
	ret += websWrite(wp, "MAC address	: %02X:%02X:%02X:%02X:%02X:%02X\n",
			mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	/* Get list of frequencies / channels */
	if(iw_get_range_info(skfd, ifname, &range) < 0){
		fprintf(stderr, "%-8.16s  no frequency information.\n\n", ifname);
	}

	/* Get current frequency / channel and display it */
	if(iw_get_ext(skfd, ifname, SIOCGIWFREQ, &wrq) >= 0){
		/* remove IW_FREQ_FIXED */
		wrq.u.freq.flags = wrq.u.freq.flags & 0xFE;
		freq = iw_freq2float(&(wrq.u.freq));
		if(freq > KILO)
			freq = iw_freq_to_channel(freq, &range);
		/* vbuf is channel */
		iw_print_freq_value(vbuf, sizeof(vbuf), freq);
	 //  iw_print_freq(buffer, sizeof(buffer),
//			freq, channel, wrq.u.freq.flags);
	}
	/* Close the socket. */
	iw_sockets_close(skfd);
	/* current channel */
	ret += websWrite(wp, "Channel		: %s\n", vbuf);
	return ret;
}

static int
wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret = 0, wl_mode_x, i;
	WIFI_STA_TABLE *sta_info;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_", *ifname, *op_mode;
	char subunit_str[8];

#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_PROXYSTA)
	if (mediabridge_mode()) {
		/* Media bridge mode */
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		if (unit != nvram_get_int("wlc_band")) {
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			ret += websWrite(wp, "%s radio is disabled\n",
				nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz");
			return ret;
		}
		ret += show_wliface_info(wp, unit, ifname, "Media Bridge");
	} else {
#endif
		/* Router mode, Repeater and AP mode */
#if defined(RTCONFIG_WIRELESSREPEATER)
		if (!unit && repeater_mode()) {
			/* Show P-AP information first, if we are about to show 2.4G information in repeater mode. */
			ret += show_wliface_info(wp, unit, get_staifname(nvram_get_int("wlc_band")), "Repeater");
			ret += websWrite(wp, "\n");
		}
#endif
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		if (!get_radio_status(ifname)) {
#if defined(BAND_2G_ONLY)
			ret += websWrite(wp, "2.4 GHz radio is disabled\n");
#else
			ret += websWrite(wp, "%s radio is disabled\n",
				nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz");
#endif
			return ret;
		}
		if (nvram_get_int("wave_action") != 0) {
			ret += websWrite(wp, "%s radio is not ready\n",
				nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz");
			return ret;
		}
		if( strcmp(getSSIDbyIFace("wlan0"), "test_ssid") == 0 ||
			strlen(getSSIDbyIFace("wlan0")) == 0 ||
			strcmp(getSSIDbyIFace("wlan2"), "LSDK_5G") == 0 ||
			strlen(getSSIDbyIFace("wlan2")) == 0){
			ret += websWrite(wp, "%s radio is not ready\n",
				nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz");
			return ret;
		}

		wl_mode_x = nvram_get_int(strcat_r(prefix, "mode_x", tmp));
		op_mode = "AP";
		if (wl_mode_x == 1)
			op_mode = "WDS Only";
		else if (wl_mode_x == 2)
			op_mode = "Hybrid";
		ret += show_wliface_info(wp, unit, ifname, op_mode);
		ret += websWrite(wp, "\nStations List\n");
		ret += websWrite(wp, "----------------------------------------------------------------\n");
#if 0 //barton++
		ret += websWrite(wp, "%-18s%-4s%-8s%-4s%-4s%-4s%-5s%-5s%-12s\n",
				   "MAC", "PSM", "PhyMode", "BW", "MCS", "SGI", "STBC", "Rate", "Connect Time");
#else
		ret += websWrite(wp, "%-3s %-17s %-6s %-6s\n",
			"idx", "MAC", "TXRATE", "RXRATE");
#endif

		if ((sta_info = malloc(sizeof(*sta_info))) != NULL) {
			getSTAInfo(unit, sta_info);
			for(i = 0; i < sta_info->Num; i++) {
				*subunit_str = '\0';
				if (sta_info->Entry[i].subunit)
					snprintf(subunit_str, sizeof(subunit_str), "%d", sta_info->Entry[i].subunit);
				ret += websWrite(wp, "%3s %-17s %6s %6s\n",
					subunit_str,
					sta_info->Entry[i].addr,
					sta_info->Entry[i].txrate,
					sta_info->Entry[i].rxrate
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

int ej_get_wlstainfo_list(int eid, webs_t wp, int argc, char_t **argv)
{
	// TODO
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
	char idx_str[8];
	int i, s, firstRow = 1;

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
		//websWrite(wp, ", \"%s\"", r->conn_time);
		s = r->subunit;
		if (s < 0 || s > 7)
			s = 0;
		if (!s)
			strcpy(idx_str, "main");
		else
			sprintf(idx_str, "GN%d", s);
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
#endif	/* RTCONFIG_STAINFO */

int getWscStatusStr(int unit, char *buf, int buf_size){
	int retVal = wlan_getWpsStatus(unit, buf);

	memset(buf, 0, buf_size);
	retVal = wlan_getWpsStatus(unit, buf);

	if(retVal)
		return -1;

	return retVal;
}

void getWPSAuthMode(int unit, char *ret_str){
	char tmp[128], prefix[]="wlXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "shared"))
		strcpy(ret_str, "Shared Key");
	else if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "psk"))
		strcpy(ret_str, "WPA-Personal");
	else if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "psk2"))
		strcpy(ret_str, "WPA2-Personal");
	else if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "pskpsk2"))
		strcpy(ret_str, "WPA-Auto-Personal");
	else if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "wpa"))
		strcpy(ret_str, "WPA-Enterprise");
	else if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "wpa2"))
		strcpy(ret_str, "WPA2-Enterprise");
	else if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "wpawpa2"))
		strcpy(ret_str, "WPA-Auto-Enterprise");
	else if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "radius"))
		strcpy(ret_str, "802.1X");
	else
		strcpy(ret_str, "Open System");
}

void getWPSEncrypType(int unit, char *ret_str){
	char tmp[128], prefix[]="wlXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "open") && nvram_match(strcat_r(prefix, "wep_x", tmp), "0"))
		strcpy(ret_str, "None");
	else if((nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "open") && !nvram_match(strcat_r(prefix, "wep_x", tmp), "0"))
			|| nvram_match("wl_auth_mode", "shared") || nvram_match("wl_auth_mode", "radius"))
		strcpy(ret_str, "WEP");

	if(nvram_match(strcat_r(prefix, "crypto", tmp), "tkip"))
		strcpy(ret_str, "TKIP");
	else if(nvram_match(strcat_r(prefix, "crypto", tmp), "aes"))
		strcpy(ret_str, "AES");
	else if(nvram_match(strcat_r(prefix, "crypto", tmp), "tkip+aes"))
		strcpy(ret_str, "TKIP+AES");
}

int wl_wps_info(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char tmpstr[256];
	int retval = 0;
	char tmp[128], prefix[]="wlXXXXXXX_";
	char *wps_sta_pin;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	retval += websWrite(wp, "<wps>\n");

	//0. WSC Status
	if(nvram_get_int("wps_enable") != 0){
		getWscStatusStr(unit, tmpstr, sizeof(tmpstr));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "Not used");

	//1. WPSConfigured
#if 0
	// TODO: temporarily cannot reset to OOB.
	if (!wps_is_oob())
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "Yes");
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "No");
#else
	memset(tmpstr, 0, sizeof(tmpstr));
	retval += wlan_getWpsConfigurationState(unit, tmpstr);
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", (!strcmp(tmpstr, "Configured")?"Yes":"No"));
#endif

	//2. WPSSSID
	memset(tmpstr, 0, sizeof(tmpstr));
	char_to_ascii(tmpstr, nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);

	//3. WPSAuthMode
	memset(tmpstr, 0, sizeof(tmpstr));
	getWPSAuthMode(unit, tmpstr);
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);

	//4. EncrypType
	memset(tmpstr, 0, sizeof(tmpstr));
	getWPSEncrypType(unit, tmpstr);
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);

	//5. DefaultKeyIdx
	snprintf(tmpstr, sizeof(tmpstr), "%s", nvram_safe_get(strcat_r(prefix, "key", tmp)));
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);

	//6. WPAKey
#if 0	//hide for security
	if (!strlen(nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp))))
		retval += websWrite(wp, "<wps_info>None</wps_info>\n");
	else
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		char_to_ascii(tmpstr, nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}
#else
	retval += websWrite(wp, "<wps_info></wps_info>\n");
#endif

	//7. AP PIN Code
	snprintf(tmpstr, sizeof(tmpstr), "%s", nvram_safe_get("secret_code"));
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);

	//8. Saved WPAKey
#if 0	//hide for security
	if (!strlen(nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp))))
	{
		retval += websWrite(wp, "<wps_info>None</wps_info>\n");
	}
	else
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		char_to_ascii(tmpstr, nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}
#else
	retval += websWrite(wp, "<wps_info></wps_info>\n");
#endif

	//9. WPS enable?
	retval += websWrite(wp, "<wps_info>%d</wps_info>\n", nvram_get_int("wps_enable"));

	//A. WPS mode
	wps_sta_pin = nvram_safe_get("wps_sta_pin");
	if (strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000"))
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "1");
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "2");

	//B. current auth mode
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp)));

	//C. WPS band
	retval += websWrite(wp, "<wps_info>%d</wps_info>\n", nvram_get_int("wps_band_x"));

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
	int ret = 0;
	char *p, *ifname;
	char *wl_ifnames;
	char line_buf[300]; // max 14x
	char *value;
	int firstRow;
	result = (WLANCONFIG_LIST *)malloc(sizeof(WLANCONFIG_LIST));
	memset(result, 0, sizeof(WLANCONFIG_LIST));

	wl_ifnames = strdup(nvram_safe_get("wl_ifnames"));
	if (!wl_ifnames) 
		return ret;
	
	p = wl_ifnames;

	firstRow = 1;
	while ((ifname = strsep(&p, " ")) != NULL) {
		while (*ifname == ' ') ++ifname;
		if (*ifname == 0) break;
		doSystem("wlanconfig %s list > %s", ifname, AUTH_INFO_PATH);
		fp = fopen(AUTH_INFO_PATH, "r");
		if (fp) {
			//fseek(fp, 131, SEEK_SET);	// ignore header
			fgets(line_buf, sizeof(line_buf), fp); // ignore header
			while ( fgets(line_buf, sizeof(line_buf), fp) ) {
				sscanf(line_buf, "%s%u%u%s%s%u%u%u%u%s%s%s%s%s%s%s%s", 
							result->addr, 
							&result->aid, 
							&result->chan, 
							result->txrate, 
							result->rxrate, 
							&result->rssi, 
							&result->idle, 
							&result->txseq, 
							&result->rxseq,
							result->caps, 
							result->acaps, 
							result->erp, 
							result->state_maxrate, 
							result->wps, 
							result->rsn, 
							result->wme,
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
		}
	}
	free(result);
	free(wl_ifnames);
	return ret;
}

#if 0
static void convertToUpper(char *str)
{
	if(str == NULL)
		return;
	while(*str)
	{
		if(*str >= 'a' && *str <= 'z')
		{
			*str &= (unsigned char)~0x20;
		}
		str++;
	}
}
#endif

#if 1
#define target 7
char str[target][40]={"Address:","Frequency:","Quality=","Encryption key:","ESSID:","IE:","Authentication Suites"};
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
	char ch[4],ssid[33],address[18],enc[9],auth[16],sig[9],wmode[8];
	int  lock;

	dbg("Please wait...");
	lock = file_lock("nvramcommit");
	system("rm -f /tmp/wlist");
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	sprintf(cmd,"iwlist %s scanning >> /tmp/wlist",nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	system(cmd);
	file_unlock(lock);
	
	if((fp= fopen("/tmp/wlist", "r"))==NULL) 
	   return -1;
	
	memset(header, 0, sizeof(header));
	sprintf(header, "%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode");

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
	        pt1 = strstr(buf[1], "Channel ");
		if(pt1)
		{
			pt2 = strstr(pt1,")");
		   	memset(ch,0,sizeof(ch));
			strncpy(ch,pt1+strlen("Channel "),pt2-pt1-strlen("Channel "));
		}   

		//ssid
	        pt1 = strstr(buf[4], "ESSID:");	
		if(pt1)
		{
		   	memset(ssid,0,sizeof(ssid));
			strncpy(ssid,pt1+strlen("ESSID:")+1,strlen(buf[4])-2-(pt1+strlen("ESSID:")+1-buf[4]));
		}   


		//bssid
	        pt1 = strstr(buf[0], "Address: ");	
		if(pt1)
		{
		   	memset(address,0,sizeof(address));
			strncpy(address,pt1+strlen("Address: "),strlen(buf[0])-(pt1+strlen("Address: ")-buf[0])-1);
		}   
	

		//enc
		pt1=strstr(buf[3],"Encryption key:");
		if(pt1)
		{   
			if(strstr(pt1+strlen("Encryption key:"),"on"))
			{  	
				sprintf(enc,"ENC");
		
			} 
			else
				sprintf(enc,"NONE");
		}

		//auth
		memset(auth,0,sizeof(auth));
		sprintf(auth,"N/A");    

		//sig
	        pt1 = strstr(buf[2], "Quality=");	
		pt2 = strstr(pt1,"/");
		if(pt1 && pt2)
		{
			memset(sig,0,sizeof(sig));
			memset(a1,0,sizeof(a1));
			memset(a2,0,sizeof(a2));
			strncpy(a1,pt1+strlen("Quality="),pt2-pt1-strlen("Quality="));
			strncpy(a2,pt2+1,strstr(pt2," ")-(pt2+1));
			sprintf(sig,"%d",atoi(a1)/atoi(a2));
		}   

		//wmode
		memset(wmode,0,sizeof(wmode));
		sprintf(wmode,"11b/g/n");   


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
	fclose(fp);
	return 0;
}   
#else
static int wl_scan(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0, i = 0, apCount = 0;
	char data[8192];
	char ssid_str[256];
	char header[128];
	struct iwreq wrq;
	SSA *ssap;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	int lock;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	memset(data, 0x00, 255);
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data; 
	wrq.u.data.flags = 0; 

	lock = file_lock("nvramcommit");
	if (wl_ioctl(nvram_safe_get(strcat_r(prefix, "ifname", tmp)), RTPRIV_IOCTL_SET, &wrq) < 0)
	{
		file_unlock(lock);
		dbg("Site Survey fails\n");
		return 0;
	}
	file_unlock(lock);
	dbg("Please wait");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".\n\n");
	memset(data, 0, 8192);
	strcpy(data, "");
	wrq.u.data.length = 8192;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	if (wl_ioctl(nvram_safe_get(strcat_r(prefix, "ifname", tmp)), RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return 0;
	}
	memset(header, 0, sizeof(header));
	//sprintf(header, "%-3s%-33s%-18s%-8s%-15s%-9s%-8s%-2s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT");
#if 0// defined(RTN14U)
	sprintf(header, "%-4s%-33s%-18s%-9s%-16s%-9s%-8s%-4s%-5s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode"," WPS", " DPID");
#else
	sprintf(header, "%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode");
#endif
	dbg("\n%s", header);
	if (wrq.u.data.length > 0)
	{
#if defined(RTN65U)
		if (unit == 0 && get_model() == MODEL_RTN65U)
		{
			char *encryption;
			SITE_SURVEY_RT3352_iNIC *pSsap, *ssAP;

			pSsap = ssAP = (SITE_SURVEY_RT3352_iNIC *) (1 /* '\n' */ + wrq.u.data.pointer +  sizeof(SITE_SURVEY_RT3352_iNIC) /* header */);
			while(((unsigned int)wrq.u.data.pointer + wrq.u.data.length) > (unsigned int) ssAP)
			{
				ssAP->channel   [sizeof(ssAP->channel)    -1] = '\0';
				ssAP->ssid      [32                         ] = '\0';
				ssAP->bssid     [17                         ] = '\0';
				ssAP->encryption[sizeof(ssAP->encryption) -1] = '\0';
				if((encryption = strchr(ssAP->authmode, '/')) != NULL)
				{
					memmove(ssAP->encryption, encryption +1, sizeof(ssAP->encryption) -1);
					memset(encryption, ' ', sizeof(ssAP->authmode) - (encryption - ssAP->authmode));
					*encryption = '\0';
				}
				ssAP->authmode  [sizeof(ssAP->authmode)   -1] = '\0';
				ssAP->signal    [sizeof(ssAP->signal)     -1] = '\0';
				ssAP->wmode     [sizeof(ssAP->wmode)      -1] = '\0';
				ssAP->extch     [sizeof(ssAP->extch)      -1] = '\0';
				ssAP->nt        [sizeof(ssAP->nt)         -1] = '\0';
				ssAP->wps       [sizeof(ssAP->wps)        -1] = '\0';
				ssAP->dpid      [sizeof(ssAP->dpid)       -1] = '\0';

				convertToUpper(ssAP->bssid);
				ssAP++;
				apCount++;
			}

			if (apCount)
			{
				retval += websWrite(wp, "[");
				for (i = 0; i < apCount; i++)
				{
					dbg("%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n",
						pSsap[i].channel,
						pSsap[i].ssid,
						pSsap[i].bssid,
						pSsap[i].encryption,
						pSsap[i].authmode,
						pSsap[i].signal,
						pSsap[i].wmode
					);

					memset(ssid_str, 0, sizeof(ssid_str));
					char_to_ascii(ssid_str, trim_r(pSsap[i].ssid));

					if (!i)
						retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, pSsap[i].bssid);
					else
						retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, pSsap[i].bssid);
				}
				retval += websWrite(wp, "]");
				dbg("\n");
			}
			else
				retval += websWrite(wp, "[]");
			return retval;
		}
#endif
		ssap=(SSA *)(wrq.u.data.pointer+strlen(header)+1);
		int len = strlen(wrq.u.data.pointer+strlen(header))-1;
		char *sp, *op;
 		op = sp = wrq.u.data.pointer+strlen(header)+1;
		while (*sp && ((len - (sp-op)) >= 0))
		{
			ssap->SiteSurvey[i].channel[3] = '\0';
			ssap->SiteSurvey[i].ssid[32] = '\0';
			ssap->SiteSurvey[i].bssid[17] = '\0';
			ssap->SiteSurvey[i].encryption[8] = '\0';
			ssap->SiteSurvey[i].authmode[15] = '\0';
			ssap->SiteSurvey[i].signal[8] = '\0';
			ssap->SiteSurvey[i].wmode[7] = '\0';
#if 0//defined(RTN14U)
			ssap->SiteSurvey[i].wps[3] = '\0';
			ssap->SiteSurvey[i].dpid[4] = '\0';
#endif
			sp+=strlen(header);
			apCount=++i;
		}
		if (apCount)
		{
			retval += websWrite(wp, "[");
			for (i = 0; i < apCount; i++)
			{
			   	dbg("\napCount=%d\n",i);
				dbg(
#if 0//defined(RTN14U)
				"%-4s%-33s%-18s%-9s%-16s%-9s%-8s%-4s%-5s\n",
#else
				"%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n",
#endif
					ssap->SiteSurvey[i].channel,
					(char*)ssap->SiteSurvey[i].ssid,
					ssap->SiteSurvey[i].bssid,
					ssap->SiteSurvey[i].encryption,
					ssap->SiteSurvey[i].authmode,
					ssap->SiteSurvey[i].signal,
					ssap->SiteSurvey[i].wmode
#if 0//defined(RTN14U)
					, ssap->SiteSurvey[i].wps
					, ssap->SiteSurvey[i].dpid
#endif
				);

				memset(ssid_str, 0, sizeof(ssid_str));
				char_to_ascii(ssid_str, trim_r(ssap->SiteSurvey[i].ssid));

				if (!i)
//					retval += websWrite(wp, "\"%s\"", ssap->SiteSurvey[i].bssid);
					retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, ssap->SiteSurvey[i].bssid);
				else
//					retval += websWrite(wp, ", \"%s\"", ssap->SiteSurvey[i].bssid);
					retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, ssap->SiteSurvey[i].bssid);
			}
			retval += websWrite(wp, "]");
			dbg("\n");
		}
		else
			retval += websWrite(wp, "[]");
	}
	return retval;
}

#endif
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


static int ej_wl_channel_list(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	char chList[256];
	char ifname[16] = {0};
	int skfd;
	struct iw_range	range;
	int k;
	char buffer[128];	/* Temporary buffer */

	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0){
		perror("socket");
		return -1;
	}

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_wave(unit, 0));

	/* Get list of frequencies / channels */
	if(iw_get_range_info(skfd, ifname, &range) < 0){
		fprintf(stderr, "%-8.16s  no frequency information.\n\n", ifname);
	}

	memset(chList, 0, sizeof(chList));

	for(k = 0; k < range.num_frequency; k++){
		if(k != 0){
			if(range.freq[k].i != 165 && range.freq[k].i != 140 ){
				strcat(chList, ",");
			}
		}
		snprintf(buffer, sizeof(buffer), "%d", range.freq[k].i);
		if(range.freq[k].i != 165 && range.freq[k].i != 140 ){
			strcat(chList, buffer);
		}
	}
	retval += websWrite(wp, "[%s]", chList);
	/* Close the socket. */
	iw_sockets_close(skfd);

	return retval;
}


int
ej_wl_channel_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 0);
}

int display_channel(int bw, int channel)
{
	char country_code[3];

	snprintf(country_code, sizeof(country_code), "%s",
		nvram_safe_get("wl_country_code"));

	if(bw == 80){
		if(strcmp(country_code, "GB") == 0){
			if(channel == 116 || channel == 132 ||
				channel == 136 || channel == 140 ){
				return 0;
			}
		}else{
			if(channel == 165 || channel == 140 ){
				return 0;
			}
		}
	}else if(bw == 40){
		if(strcmp(country_code, "GB") == 0){
			if(channel == 116 || channel == 140){
				return 0;
			}
		}
	}
	return 1;
}

int get_wl_channel_list_by_bw_core(int unit, char *ch_list, int bw)
{
	int retval = 0;
	char ifname[16] = {0};
	int skfd;
	struct iw_range	range;
	int k;
	char buffer[128];	/* Temporary buffer */

	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0){
		perror("socket");
		return -1;
	}

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_wave(unit, 0));

	/* Get list of frequencies / channels */
	if(iw_get_range_info(skfd, ifname, &range) < 0){
		fprintf(stderr, "%-8.16s  no frequency information.\n\n", ifname);
	}

	strcat(ch_list, "[");
	for(k = 0; k < range.num_frequency; k++){
		if(k != 0){
			if(display_channel(bw, range.freq[k].i) == 1){
				strcat(ch_list, ",");
			}
		}
		snprintf(buffer, sizeof(buffer), "%d", range.freq[k].i);
		if(display_channel(bw, range.freq[k].i) == 1){
			strcat(ch_list, buffer);
		}
	}
	strcat(ch_list, "]");
	/* Close the socket. */
	iw_sockets_close(skfd);

	return retval;
}

int ej_wl_channel_list_5g_20m(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	static char ch_list[256] = {0};
	int check_bit;

	if( (nvram_get_int("wl_country_changed") & 0x1) == 0x1){
		check_bit = nvram_get_int("wl_country_changed") ^ 0x1 ;
		nvram_set_int("wl_country_changed", check_bit);
		memset(ch_list, 0, sizeof(ch_list));
	}

	if(strlen(ch_list) == 0) retval = get_wl_channel_list_by_bw_core(1, ch_list, 20);
	retval += websWrite(wp, ch_list);
	return retval;
}

int ej_wl_channel_list_5g_40m(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	static char ch_list[256] = {0};
	int check_bit;
	int get_list_again;

	if( (nvram_get_int("wl_country_changed") & 0x2) == 0x2){
		check_bit = nvram_get_int("wl_country_changed") ^ 0x2 ;
		nvram_set_int("wl_country_changed", check_bit);
		memset(ch_list, 0, sizeof(ch_list));
	}

	if(strlen(ch_list) == 0) retval = get_wl_channel_list_by_bw_core(1, ch_list, 40);
	retval += websWrite(wp, ch_list);
	return retval;
}

int ej_wl_channel_list_5g_80m(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	static char ch_list[256] = {0};
	int check_bit;

	if( (nvram_get_int("wl_country_changed") & 0x4) == 0x4){
		check_bit = nvram_get_int("wl_country_changed") ^ 0x4;
		nvram_set_int("wl_country_changed", check_bit);
		memset(ch_list, 0, sizeof(ch_list));
	}

	if(strlen(ch_list) == 0) retval = get_wl_channel_list_by_bw_core(1, ch_list, 80);
	retval += websWrite(wp, ch_list);
	return retval;
}

int
ej_wl_channel_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	/* show 80MHz channel list */
	return ej_wl_channel_list(eid, wp, argc, argv, 1);
}


static int ej_wl_rate(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
#define ASUS_IOCTL_GET_STA_DATARATE (SIOCDEVPRIVATE+15) /* from qca-wifi/os/linux/include/ieee80211_ioctl.h */
        struct iwreq wrq;
	int retval = 0;
	char tmp[256], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char word[256], *next;
	int unit_max = 0;
	unsigned int rate[2];
	char rate_buf[32];
	int sw_mode = nvram_get_int("sw_mode");
	int wlc_band = nvram_get_int("wlc_band");

	sprintf(rate_buf, "0 Mbps");

	if (!nvram_match("wlc_state", "2"))
		goto ERROR;

	foreach (word, nvram_safe_get("wl_ifnames"), next)
		unit_max++;

	if (unit > (unit_max - 1))
		goto ERROR;

	if (wlc_band == unit && (sw_mode == SW_MODE_REPEATER || sw_mode == SW_MODE_HOTSPOT))
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	else
		goto ERROR;
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	wrq.u.data.pointer = rate;
	wrq.u.data.length = sizeof(rate);

	if (wl_ioctl(name, ASUS_IOCTL_GET_STA_DATARATE, &wrq) < 0)
	{
		dbg("%s: errors in getting %s ASUS_IOCTL_GET_STA_DATARATE result\n", __func__, name);
		goto ERROR;
	}

	if (rate[0] > rate[1])
		sprintf(rate_buf, "%d Mbps", rate[0]);
	else
		sprintf(rate_buf, "%d Mbps", rate[1]);

ERROR:
	retval += websWrite(wp, "%s", rate_buf);
	return retval;
}


int
ej_wl_rate_2g(int eid, webs_t wp, int argc, char_t **argv)
{
   	if(nvram_match("sw_mode", "2"))
		return ej_wl_rate(eid, wp, argc, argv, 0);
	else
	   	return 0;
}

int
ej_wl_rate_5g(int eid, webs_t wp, int argc, char_t **argv)
{
   	if(nvram_match("sw_mode", "2"))
		return ej_wl_rate(eid, wp, argc, argv, 1);
	else
	   	return 0;
}

int
ej_nat_accel_status(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;

	/* always 1, alpine has no HW NAT */
	retval += websWrite(wp, "1");

	return retval;
}

#ifdef RTCONFIG_PROXYSTA
int
ej_wl_auth_psta(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;

	if(nvram_match("wlc_state", "2"))	//connected
		retval += websWrite(wp, "wlc_state=1;wlc_state_auth=0;");
	//else if(?)				//authorization failed
	//	retval += websWrite(wp, "wlc_state=2;wlc_state_auth=1;");
	else					//disconnected
		retval += websWrite(wp, "wlc_state=0;wlc_state_auth=0;");

	return retval;
}
#endif
