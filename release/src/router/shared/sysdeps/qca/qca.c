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
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <shutils.h>
#include <shared.h>
#include <qca.h>
extern int get_ap_mac(const char *ifname, struct iwreq *pwrq);
extern int diff_current_bssid(int unit, char bssid_str[]);

#if defined(RTCONFIG_SOC_IPQ8074)
/* Return value of /sys/firmware/devicetree/base/soc_version_major.
 * If it's not readable, return fixed value based on model.
 * @return:	value of /sys/firmware/devicetree/base/soc_version_major
 */
unsigned char get_soc_version_major(void)
{
#if defined(RTAX89U) || defined(GTAXY16000)
	const unsigned char sver = 2;
#else
	const unsigned char sver = 0;
#endif
	unsigned char v = sver;

	if (f_read("/sys/firmware/devicetree/base/soc_version_major", &v, 1) <= 0) {	/* 1 byte */
		dbg("%s: can't read soc_version_major, assume it's %d\n", __func__, sver);
		v = sver;
	}

	return v;
}
#endif

#if defined(RTCONFIG_GLOBAL_INI)
#if defined(RTCONFIG_SOC_IPQ8074)
/* Return path and filename of internal ini file.
 * e.g. /etc/Wireless/ini/internal/QCA8074V2_i.ini
 * @ini_fn:		.ini filename buffer
 * @ini_fn_size:	size of @ini_fn
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int get_internal_ini_filename(char *ini_fn, size_t ini_fn_size)
{
	const char *ini_fn_tbl[] = { "QCA8074_i.ini", "QCA8074V2_i.ini" };
	unsigned char v;

	if (!ini_fn || !ini_fn_size)
		return -1;

	*ini_fn = '\0';

	v = get_soc_version_major();
	if (v <= 0 || v > ARRAY_SIZE(ini_fn_tbl)) {
		dbg("%s: unknown SoC version number [%d]\n", __func__, v);
		return -2;
	}

	snprintf(ini_fn, ini_fn_size, "%s/internal/%s", GLOBAL_INI_TOPDIR, ini_fn_tbl[v - 1]);

	return 0;
}
#elif defined(RTCONFIG_SOC_IPQ40XX)
int get_internal_ini_filename(char *ini_fn, size_t ini_fn_size)
{
	return 0;
}
#elif defined(RTCONFIG_SOC_IPQ60XX)
int get_internal_ini_filename(char *ini_fn, size_t ini_fn_size)
{
	return 0;
}
#else
#error FIXME
#endif

/* Get one parameter from .ini file and return it's value in string format.
 * If @param_name is replicated multi-times, first one is returned.
 * @param_name:
 * @param_val:
 * @param_val_size:
 * @ini_fn:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int get_parameter_from_ini_file(const char *param_name, char *param_val, size_t param_val_size, const char *ini_fn)
{
	FILE *fp;
	int found = 0, key_len;
	char *p, line[MAX_INI_PARM_LINE_LEN];

	if (!param_name || !param_val || !param_val_size || !ini_fn)
		return -1;

	*param_val = '\0';
	if (!(fp = fopen(ini_fn, "r")))
		return -2;

	key_len = strlen(param_name);
	while (fgets(line, sizeof(line), fp)) {
		if (*line == '#' || *line == '\0' || *line == '\r' || *line == '\n' || *line == '=')
			continue;

		if (!strchr(line, '=')) {
			dbg("%s: unknown format [%s] in %s.\n", __func__, line, ini_fn);
			continue;
		}
		if (strncmp(line, param_name, key_len) || *(line + key_len) != '=')
			continue;

		/* replace '\n' with '\0' temporary. */
		if ((p = strchr(line, '\n')) != NULL)
			*p = '\0';

		if (!found) {
			strlcpy(param_val, line + key_len + 1, param_val_size);
			found++;
		}
	}
	fclose(fp);

	return 0;
}

#if defined(RTCONFIG_SPF11_QSDK) || defined(RTCONFIG_SPF11_1_QSDK)
/* Get one board/default parameter from .ini file and return it's value in string format.
 * If board-specific parameter absent, return default parameter instead.
 * If @param_name is replicated multi-times, first one is returned.
 * @board_name: e.g. ap-hk_v1
 * @param_name:
 * @param_val:
 * @param_val_size:
 * @ini_fn:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int get_board_or_default_parameter_from_ini_file(const char *board_name, const char *param_name, char *param_val, size_t param_val_size, const char *ini_fn)
{
	int c, r;
	char key_name[256];

	if (!board_name || !param_name || !param_val || !param_val_size || !ini_fn)
		return -1;

	for (c = 0; c <= 1; ++c) {
		/* e.g.
		 * ap-hk06_v2_enable_daemon_support=0
		 * ap-hk_v1_default_enable_daemon_support=1
		 */
		snprintf(key_name, sizeof(key_name), "%s_%s%s", board_name, (c == 0)? "" : "default_", param_name);

		*param_val = '\0';
		r = get_parameter_from_ini_file(key_name, param_val, param_val_size, ini_fn);
		if (!r && *param_val != '\0')
			break;
	}

	return 0;
}
#endif	/* RTCONFIG_SPF11_QSDK || RTCONFIG_SPF11_1_QSDK */

/* Get one parameter from .ini file and return it's value in integer format.
 * If @param_name is replicated multi-times, first one is returned.
 * @param_name:
 * @param_val:
 * @ini_fn:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int get_integer_parameter_from_ini_file(const char *param_name, int *param_val, const char *ini_fn)
{
	int ret = 0;
	char intbuf[11] = { 0 };

	if (!param_val)
		return -1;

	if (!(ret = get_parameter_from_ini_file(param_name, intbuf, sizeof(intbuf), ini_fn)))
		*param_val = safe_atoi(intbuf);

	return ret;
}
#endif	/* RTCONFIG_GLOBAL_INI */

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) \
 || defined(RTCONFIG_WIFI_QCA9994_QCA9994) \
 || defined(RTCONFIG_WIFI_QCN5024_QCN5054)
int nss_wifi_offloading(void)
{
	int band, shift, olcfg = 0;
	char vphy[IFNAMSIZ] = { 0 }, prefix[sizeof("wlXXXXX_")];

	for (band = 0, olcfg = 0; band < MAX_NR_WL_IF; ++band) {
		SKIP_ABSENT_BAND(band);
		snprintf(prefix, sizeof(prefix), "wl%d_", band);
		strlcpy(vphy, get_vphyifname(band), sizeof(vphy));
		shift = safe_atoi(vphy + strlen(vphy) - 1);
		olcfg |= !!nvram_pf_get_int(prefix, "hwol") << shift;
	}
	return olcfg;
}
#endif

/* Helper of __get_qca_sta_info_by_ifname()
 * @src:	pointer to WLANCONFIG_LIST
 * @arg:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int handler_qca_sta_info(const WLANCONFIG_LIST *src, void *arg)
{
	WIFI_STA_TABLE *sta_info = arg;
	WLANCONFIG_LIST *dst;

	if (!src || !arg)
		return -1;

	dst = &sta_info->Entry[sta_info->Num++];
	*dst = *src;
#if 0
	dbg("[%s][%u][%u][%s][%s][%d][%s][%s]\n", dst->addr, dst->aid, dst->chan,
		dst->txrate, dst->rxrate, dst->rssi, dst->mode, dst->conn_time);
#endif


	return 0;
}

#define MAX_NR_STA_ITEMS	(30)
struct sta_info_item_s {
	int idx;		/* < 0: doesn't exist; >= 0: v[] index */
	const char *key;
	const char *fmt;	/* format string that is used to convert v[idx] */
	void *var;		/* target address that is used to store convertion result. */
};

/* Helper of __get_qca_sta_info_by_ifname() that is used to initialize struct sta_info_item_s array,
 * according to header line of output of "wlanconfig athX list".
 * header line maybe truncated due to:
 * 1. ACAPS never has data.
 * 2. IEs maybe empty string, RSN, WME, or RSN WME.
 * @line:
 * @sta_info_items:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int init_sta_info_item(const char *line, struct sta_info_item_s *sta_info_items)
{
	const char f[] = "%s";
	int i, n;
	char fmt[MAX_NR_STA_ITEMS * sizeof(f)];
	char v[MAX_NR_STA_ITEMS][sizeof("MAXRATE(DOT11)XXXXX")];
	struct sta_info_item_s *p;

	if (!line || !sta_info_items)
		return -1;

	for (p = sta_info_items; p->key != NULL; ++p)
		p->idx = -1;

	for (i = 0, *fmt = '\0'; i < MAX_NR_STA_ITEMS; ++i)
		strlcat(fmt, f, sizeof(fmt));

	n = sscanf(line, fmt, v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6, v + 7, v + 8, v + 9,
		v + 10, v + 11, v + 12, v + 13, v + 14, v + 15, v + 16, v + 17, v + 18, v + 19,
		v + 20, v + 21, v + 22, v + 23, v + 24, v + 25, v + 26, v + 27, v + 28, v + 29);
	for (i = 0; i < n; ++i) {
		for (p = &sta_info_items[0]; p->key != NULL; ++p) {
			if (strcmp(v[i], p->key))
				continue;

			p->idx = i;
			break;
		}
	}

	return 0;
}

/* Helper of __get_qca_sta_info_by_ifname() that is used to fill data to WLANCONFIG_LIST,
 * according to header line of output of "wlanconfig athX list".
 * header line maybe truncated due to:
 * 1. ACAPS never has data.
 * 2. IEs maybe empty string, RSN, WME, or RSN WME.
 * @line:
 * @sta_info_items:
 * @return:
 * 	0:	success
 *     <0:	error
 *     >0:	number of items can't be parsed.
 */
static int fill_sta_info_item(const char *line, const struct sta_info_item_s *sta_info_items)
{
	const char f[] = "%s";
	int i, n, ret = 0;
	char fmt[MAX_NR_STA_ITEMS * sizeof(f)];
	char v[MAX_NR_STA_ITEMS][sizeof("IEEE80211_MODE_11AXA_HE40MINUSXXXXX")];
	const struct sta_info_item_s *p;

	if (!line || !sta_info_items)
		return -1;

	for (i = 0, *fmt = '\0'; i < MAX_NR_STA_ITEMS; ++i)
		strlcat(fmt, f, sizeof(fmt));

	n = sscanf(line, fmt, v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6, v + 7, v + 8, v + 9,
		v + 10, v + 11, v + 12, v + 13, v + 14, v + 15, v + 16, v + 17, v + 18, v + 19,
		v + 20, v + 21, v + 22, v + 23, v + 24, v + 25, v + 26, v + 27, v + 28, v + 29);
	for (p = sta_info_items; n > 0 && p->key != NULL; ++p) {
		if (p->idx < 0)
			continue;

		if (sscanf(v[p->idx], p->fmt, p->var) == 1)
			continue;

		ret++;
		dbg("%s: can't parse. argv[%d] = [%s] key [%s] fmt [%s] var [%p]\n",
			__func__, p->idx, v[p->idx]? : "<NULL>", p->key, p->fmt, p->var);
	}

	return ret;
}

/* Return channel noise floor.
 * If @band or @ifname is valid and corresponding interface does exist,
 * adjust channel noise floor based on bandwidth.
 * @band:	enum wl_band_id, can be invalid value.
 * @ifname:	VAP interface name.
 * @return:	channel noise floor.
 */
int get_channf(int band, const char *ifname)
{
	int channf = -96;	/* QCA_DEFAULT_NOISE_FLOOR, SF case#03626623. */
	int nf_offset = 0, bw = 40, nctrlsb;
	char vap[IFNAMSIZ] = { 0 };

	if (!absent_band(band))
		__get_wlifname(band, 0, vap);
	else if (ifname)
		strlcpy(vap, ifname, sizeof(vap));

	if (!iface_exist(vap) || get_bw_nctrlsb(vap, &bw, &nctrlsb) < 0)
		return channf;

	/* SF case#03945423, bandwidth offset:
	 * 20MHz: -3
	 * 40MHz:  0
	 * 80MHz:  3
	 * 160MHz: 6
	 */
	if (bw == 20)
		nf_offset = -3;
	else if (bw == 80)
		nf_offset = 3;
	else if (bw == 160)
		nf_offset = 6;

	return channf + nf_offset;
}

/* Parsing "wlanconfig athX list" result, fill WLANCONFIG_LIST, and then pass it to @handler() with @arg which is provided by caller.
 * @ifname:	VAP interface name that is used to execute "wlanconfig @ifname list" command.
 * @subunit_id:	if non-zero, copied to WLANCONFIG_LIST.subunit
 * @handler:	handler function that will be execute for each client.
 * return:
 * 	0:	success
 *  otherwise:	error
 *
 * ILQ3.1 example:
 * wlanconfig ath1 list
 * ADDR               AID CHAN TXRATE RXRATE RSSI IDLE  TXSEQ  RXSEQ  CAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS ASSOCTIME    IEs   MODE PSMODE
 * 00:10:18:55:cc:08    1  149  55M   1299M   63    0      0   65535               0        807              0              Q 00:10:33 IEEE80211_MODE_11A  0
 * 08:60:6e:8f:1e:e6    2  149 159M    866M   44    0      0   65535     E         0          b              0           WPSM 00:13:32 WME IEEE80211_MODE_11AC_VHT80  0
 * 08:60:6e:8f:1e:e8    1  157 526M    526M   51 4320      0   65535    EP         0          b              0          AWPSM 00:00:10 RSN WME IEEE80211_MODE_11AC_VHT80 0
 *
 * SPF8 CSU2 QSDK example:
 * admin@RT-AX89U:/tmp/home/root# wlanconfig ath0 list
 * ADDR               AID CHAN TXRATE RXRATE RSSI MINRSSI MAXRSSI IDLE  TXSEQ  RXSEQ  CAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS ASSOCTIME    IEs   MODE                   PSMODE
 * 12:9d:92:4e:85:bc    1  104 2882M   3026M   73       0      74    0      0   65535   EPs         0          b              0           AWPSM 00:00:35     RSN WME IEEE80211_MODE_11AXA_HE80   0
 * 14:dd:a9:3d:68:65    2  104 433M    433M   69       0      79    0      0   65535    EP         0          b              0            AWPS 00:00:35     RSN WME IEEE80211_MODE_11AC_VHT80   1
 *
 * SPF10 ES QSDK example:
 * admin@GT-AXY16000:/tmp/home/root# wlanconfig ath0 list
 * ADDR               AID CHAN TXRATE RXRATE RSSI MINRSSI MAXRSSI IDLE  TXSEQ  RXSEQ  CAPS XCAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS   VHTCAPS ASSOCTIME    IEs   MODE RXNSS TXNSS                   PSMODE
 * 14:dd:a9:3d:68:65    1   60 433M      6M   36      22      40    0      0   65535    EP    OI         0          b              0            AWPS             gGR 00:00:09     RSN WME IEEE80211_MODE_11AC_VHT80  1 1   1
 *  Minimum Tx Power             : 0
 *  Maximum Tx Power             : 0
 *  HT Capability                        : Yes
 *  VHT Capability                       : Yes
 *  MU capable                   : No
 *  SNR                          : 36
 *  Operating band                       : 5GHz
 *  Current Operating class      : 0
 *  Supported Rates              : 12  18  24  36  48  72  96  108
 *
 * SPF11 CSU1 QSDK example:
 * admin@RT-AX89U-4988:/tmp# wlanconfig ath0 list
 * ADDR               AID CHAN TXRATE RXRATE RSSI MINRSSI MAXRSSI IDLE  TXSEQ  RXSEQ  CAPS XCAPS ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS   VHTCAPS ASSOCTIME    IEs   MODE RXNSS TXNSS                   PSMODE
 * 14:dd:a9:3d:68:65    1   40 325M    433M  -61     -79     -53   24      0   65535    EP    OI NULL    0          b         541666            AWPS             gGR 00:19:28     RSN WME IEEE80211_MODE_11AC_VHT80  1 1   0  
 *  Minimum Tx Power             : 0
 *  Maximum Tx Power             : 0
 *  HT Capability                        : Yes
 *  VHT Capability                       : Yes
 *  MU capable                   : No
 *  SNR                          : 32
 *  Operating band                       : 5GHz
 *  Current Operating class      : 0
 *  Supported Rates              : 12  18  24  36  48  72  96  108 
 *  Max STA phymode              : IEEE80211_MODE_11AC_VHT80 
 */
static int __get_QCA_sta_info_by_ifname(const char *ifname, char subunit_id, int (*handler)(const WLANCONFIG_LIST *rptr, void *arg), void *arg)
{
#if defined(RTCONFIG_SOC_IPQ8074)
	const int l2_offset = 91;
#elif defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_)
	const int l2_offset = 85;
#else
	const int l2_offset = 79;
#endif
	FILE *fp;
	int channf, ret = 0, ax2he = 0;
	unsigned char tmac[6], *tm = &tmac[0];
	char cmd[sizeof("wlanconfig XXX list") + IFNAMSIZ];
	char *q, line_buf[300], *l2 = line_buf + l2_offset;
	WLANCONFIG_LIST result, *r = &result;
	struct sta_info_item_s part1_tbl[] = {
		/* Parse ADDR ~ XCAPS. */
		{ .key = "ADDR",	.fmt = "%s",	.var = &r->addr },
		{ .key = "AID",		.fmt = "%u",	.var = &r->aid },
		{ .key = "CHAN",	.fmt = "%u",	.var = &r->chan },
		{ .key = "TXRATE",	.fmt = "%s",	.var = &r->txrate },
		{ .key = "RXRATE",	.fmt = "%s",	.var = &r->rxrate },
		{ .key = "RSSI",	.fmt = "%d",	.var = &r->rssi },

		{ .key = NULL, .fmt = NULL, .var = NULL },
	}, part2_tbl[] = {
		/* Parse ACAPS ~ IEs (maybe empty string, RSN, WME, or both).
		 * ACAPS is empty on ILQ2.x ~ SPF10, is "NULL" on SPF11
		 */
		{ .key = "ASSOCTIME",	.fmt = "%s",	.var = &r->conn_time },

		{ .key = NULL, .fmt = NULL, .var = NULL },
	}, part3_tbl[] = {
		/* Parse MODE ~ PSMODE */
		{ .key = "MODE",	.fmt = "IEEE80211_MODE_%s", .var = r->mode },

		{ .key = NULL, .fmt = NULL, .var = NULL },
	};

	if (!ifname || !handler)
		return -1;

	snprintf(cmd, sizeof(cmd), "wlanconfig %s list", ifname);
	if (!(fp = popen(cmd, "r")))
		return -2;

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	if (!find_word(nvram_safe_get("rc_support"), "11AX"))
		ax2he = 1;
#endif
	channf = get_channf(-1, ifname);

	/* Parsing header and initialize related data structure */
	if (!fgets(line_buf, sizeof(line_buf), fp))
		goto leave;

	if ((q = strstr(line_buf, "MODE")) != NULL) {
		*(q - 1) = '\0';
		init_sta_info_item(q, part3_tbl);
	}
	if ((q = strstr(line_buf, "ACAPS")) != NULL) {
		*(q - 1) = '\0';
		l2 = q;
#if defined(RTCONFIG_SPF11_QSDK) || defined(RTCONFIG_SPF11_1_QSDK)
		init_sta_info_item(q, part2_tbl);
#else
		/* ILQ2.x ~ SPF10 */
		init_sta_info_item(q + strlen("ACAPS"), part2_tbl);	/* skip ACAPS due to it doesn't have data. */
#endif
	}
	init_sta_info_item(line_buf, part1_tbl);

	/* Parsing client list */
	while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
		if (sscanf(line_buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx %*[^\n]", tm, tm + 1, tm + 2, tm + 3, tm + 4, tm + 5) != 6)
			continue;

		memset(r, 0, sizeof(*r));
		/* Parsing part3, all data behind IEs (started from IEEE802...) */
		if ((q = strstr(line_buf, "IEEE80211_MODE_")) != NULL) {
			*(q - 1) = '\0';
			fill_sta_info_item(q, part3_tbl);
		}

		/* Parsing part2, ACAPS (omit) ~ IEs */
		*(l2 - 1) = '\0';
		fill_sta_info_item(l2, part2_tbl);

		/* Parsing part1, ADDR ~ IEs */
		fill_sta_info_item(line_buf, part1_tbl);

		/* Post adjustment */
		if (ax2he) {
			if ((q = strstr(r->mode, "11AXA")) != NULL)
				memcpy(q, "11AHE", 5);
			else if ((q = strstr(r->mode, "11AXG")) != NULL)
				memcpy(q, "11GHE", 5);
		}
		if (subunit_id)
			r->subunit_id = subunit_id;
		if (strlen(r->rxrate) >= 6)
			strcpy(r->rxrate, "0M");
		convert_mac_string(r->addr);

		/* If wlanconfig reports QCA_RSSI (0 ~ 115), adjust it with channf.
		 * But it's not accurate as long as client bandwidth different.
		 * If wlanconfig reports normal RSSI, negative value, don't adjust it again.
		 */
		if (r->rssi > 0) {
			r->rssi += channf;
			if (r->rssi >= 0)
				r->rssi = -1;
		}

		handler(r, arg);
	}
leave:
	pclose(fp);
	return ret;
}

#if defined(RTCONFIG_WIGIG)
/* Parsing "iw wlan0 station dump" result, fill WLANCONFIG_LIST, and then pass it to @handler() with @arg which is provided by caller.
 * @ifname:	VAP interface name that is used to execute "iw @ifname station dump" command.
 * @subunit_id:	if non-zero, copied to WLANCONFIG_LIST.subunit
 * @handler:	handler function that will be execute for each client.
 * return:
 * 	0:	success
 *  otherwise:	error
 *
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
static int __get_IW_sta_info_by_ifname(const char *ifname, char subunit_id, int (*handler)(const WLANCONFIG_LIST *rptr, void *arg), void *arg)
{
	FILE *fp;
	int c, time_val, hr, min, sec, rssi;
	char rate[6], line_buf[300];
	char cmd[sizeof("iw wlan0 station dump XXXXXX")];
	WLANCONFIG_LIST result, *r = &result;

	if (!ifname || !handler)
		return -1;

	snprintf(cmd, sizeof(cmd), "iw %s station dump", get_wififname(WL_60G_BAND));
	fp = popen(cmd, "r");
	if (!fp)
		return -2;

	/* /sys/kernel/debug/ieee80211/phy0/wil6210/stations has client list too.
	 * But I guess none of any another attributes exist, e.g., connection time, exist.
	 * Thus, parsing result of "iw wlan0 station dump" instead.
	 */
	while (fgets(line_buf, sizeof(line_buf), fp)) {
		if (strncmp(line_buf, "Station", 7)) {
			continue;
		}

next_sta:
		memset(r, 0, sizeof(*r));
		c = sscanf(line_buf, "Station %17[0-9a-f:] %*[^\n]", r->addr);
		if (c != 1) {
			continue;
		}
		convert_mac_string(r->addr);
		if (subunit_id)
			r->subunit_id = subunit_id;
		strlcpy(r->mode, "11ad", sizeof(r->mode));	/* FIXME */
		while (fgets(line_buf, sizeof(line_buf), fp)) {
			if (!strncmp(line_buf, "Station", 7)) {

#if 0
				dbg("[%s][%u][%u][%s][%s][%u][%s]\n",
					r->addr, r->aid, r->chan, r->txrate, r->rxrate, r->rssi, r->mode);
#endif
				handler(r, arg);
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
#endif	/* RTCONFIG_WIGIG */

/* Wrapper function of QCA/IW Wireless client list parser.
 * @ifname:	VAP interface name
 * @subunit_id:
 * @sta_info:	pointer to WIFI_STA_TABLE
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int __get_qca_sta_info_by_ifname(const char *ifname, char subunit_id, int (*handler)(const WLANCONFIG_LIST *rptr, void *arg), void *arg)
{
#if defined(RTCONFIG_WIGIG)
	char *vap_60g = get_wififname(WL_60G_BAND);

	if (!ifname)
		return -1;

	if (!strncmp(ifname, vap_60g, strlen(vap_60g))) {
		return __get_IW_sta_info_by_ifname(ifname, subunit_id, handler, arg);
	} else
#endif
		return __get_QCA_sta_info_by_ifname(ifname, subunit_id, handler, arg);
}

/* Wrapper function of QCA Wireless client list parser.
 * @ifname:	VAP interface name
 * @subunit_id:
 * @sta_info:	pointer to WIFI_STA_TABLE
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int get_qca_sta_info_by_ifname(const char *ifname, char subunit_id, WIFI_STA_TABLE *sta_info)
{
#if defined(RTCONFIG_WIGIG)
	char *vap_60g = get_wififname(WL_60G_BAND);

	if (!ifname)
		return -1;

	if (!strncmp(ifname, vap_60g, strlen(vap_60g))) {
		return __get_IW_sta_info_by_ifname(ifname, subunit_id, handler_qca_sta_info, sta_info);
	} else
#endif
		return __get_QCA_sta_info_by_ifname(ifname, subunit_id, handler_qca_sta_info, sta_info);
}

#ifdef RTCONFIG_AMAS
/**
 * @brief add beacon vise by unit and subunit
 *
 * @param unit band index
 * @param subunit mssid index
 * @param hexdata vise string
 */
void add_beacon_vsie_by_unit(int unit, int subunit, char *hexdata)
{
	// 0: Beacon
	// 1: ProbeRequest
	// 2: ProbeResponse
	// 3: AuthenticationRequest
	// 4: AuthenticationRespnse
	// 5: AssocationRequest
	// 6: AssociationResponse
	// 7: ReassociationRequest
	// 8: ReassociationResponse
	char cmd[300];
	int pktflag = 0x0;
	int len = 0;
	char *ifname = NULL;
	char buf[50] = "wlXX.XX_ifname";
#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		return;
#endif
	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	if(subunit<=0)
		ifname = get_wififname(unit);	// TODO: Should we get the band from nvram?
	else
	{
		memset(buf, 0, sizeof(buf));
                snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
		ifname=nvram_safe_get(buf);
		if(!guest_wlif(ifname)) //not guestnetwork
			return;
	}

	//_dprintf("%s: ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli -i%s set_vsie %d DD%02X%02X%02X%02X%s",
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0], (uint8_t)OUI_ASUS[1], (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
	}
}


/**
 * @brief add guest vsie
 *
 * @param hexdata vsie string
 */
void add_beacon_vsie_guest(char *hexdata)
{
	int unit = 0, subunit = 0;
    	char word[100], *next;

    	foreach (word, nvram_safe_get("wl_ifnames"), next) {
        	if (nvram_get_int("re_mode") == 1)  // RE
            		subunit = 2;
        	else  // CAP/Router
       		     	subunit = 1;
        	for (; subunit <=  num_of_mssid_support(unit); subunit++) 
		{
                        char buf[] = "wlXX.XX_ifname";
                        memset(buf, 0, sizeof(buf));
                        snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
            		if (is_intf_up(nvram_safe_get(buf)) != -1)  // interface exist
                		add_beacon_vsie_by_unit(unit,subunit, hexdata);
        	}
        	unit++;
    	}
}

void add_beacon_vsie(char *hexdata)
{
	// 0: Beacon
	// 1: ProbeRequest
	// 2: ProbeResponse
	// 3: AuthenticationRequest
	// 4: AuthenticationRespnse
	// 5: AssocationRequest
	// 6: AssociationResponse
	// 7: ReassociationRequest
	// 8: ReassociationResponse
	char cmd[300];
	int pktflag = 0x0;
	int len = 0;
	char *ifname = NULL;
#ifdef RTCONFIG_BHCOST_OPT
        int unit = 0;
        char word[100], *next;
#endif



#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		return;
#endif
	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */
#ifdef RTCONFIG_BHCOST_OPT
	unit=0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) 
	{
		ifname=get_wififname(unit);	
	//	_dprintf("%s: wl%d_ifname=%s\n", __func__,unit, ifname);
#else
	ifname = get_wififname(0);	// TODO: Should we get the band from nvram?
	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);
#endif

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli -i%s set_vsie %d DD%02X%02X%02X%02X%s",
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0], (uint8_t)OUI_ASUS[1], (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
	}

#ifdef RTCONFIG_BHCOST_OPT
		unit++;
	}
#endif	
}

/**
 * @brief remove beacon vsie by unit and subunit
 *
 * @param unit band index
 * @param subunit mssid index
 * @param hexdata vsie string
 */
void del_beacon_vsie_by_unit(int unit, int subunit, char *hexdata)
{
	// 0: Beacon
	// 1: ProbeRequest
	// 2: ProbeResponse
	// 3: AuthenticationRequest
	// 4: AuthenticationRespnse
	// 5: AssocationRequest
	// 6: AssociationResponse
	// 7: ReassociationRequest
	// 8: ReassociationResponse
	char cmd[300] = {0};
	int pktflag = 0x0;
	int len = 0;
	char *ifname = NULL;
	char buf[50] = "wlXX.XX_ifname";

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		return;
#endif
	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	if(subunit<=0)
		ifname = get_wififname(unit);	// TODO: Should we get the band from nvram?
	else
	{
		memset(buf, 0, sizeof(buf));
                snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
		ifname=nvram_safe_get(buf);
		if(!guest_wlif(ifname)) //not guestnetwork
			return;
	}

	//_dprintf("%s: ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli -i%s del_vsie %d DD%02X%02X%02X%02X%s",
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0], (uint8_t)OUI_ASUS[1], (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
	}
}


/**
 * @brief remove guest beacon vsie
 *
 * @param hexdata vsie string
 */
void del_beacon_vsie_guest(char *hexdata)
{
    	int unit = 0, subunit = 0;
    	char word[100], *next;

    	foreach (word, nvram_safe_get("wl_ifnames"), next) {
        	if (nvram_get_int("re_mode") == 1)  // RE
            		subunit = 2;
        	else  // CAP/Router
            		subunit = 1;
        	for (; subunit <= num_of_mssid_support(unit); subunit++) {
                        char buf[] = "wlXX.XX_ifname";
                        memset(buf, 0, sizeof(buf));
                        snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
            		if (is_intf_up(nvram_safe_get(buf)) != -1)  // interface exist
                		del_beacon_vsie_by_unit(unit, subunit, hexdata);
        	}
        	unit++;
    	}
}

void del_beacon_vsie(char *hexdata)
{
	// 0: Beacon
	// 1: ProbeRequest
	// 2: ProbeResponse
	// 3: AuthenticationRequest
	// 4: AuthenticationRespnse
	// 5: AssocationRequest
	// 6: AssociationResponse
	// 7: ReassociationRequest
	// 8: ReassociationResponse
	char cmd[300] = {0};
	int pktflag = 0x0;
	int len = 0;
	char *ifname = NULL;
#ifdef RTCONFIG_BHCOST_OPT
        int unit = 0;
        char word[100], *next;
#endif

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		return;
#endif
	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

#ifdef RTCONFIG_BHCOST_OPT
	unit=0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) 
	{
		ifname=get_wififname(unit);	
		//_dprintf("%s: wl%d_ifname=%s\n", __func__,unit, ifname);
#else
	ifname = get_wififname(0);	// TODO: Should we get the band from nvram?
	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);
#endif

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli -i%s del_vsie %d DD%02X%02X%02X%02X%s",
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0], (uint8_t)OUI_ASUS[1], (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
	}

#ifdef RTCONFIG_BHCOST_OPT
		unit++;
	}
#endif	
}

#if defined(RTCONFIG_AMAS_WGN)
char* get_all_lan_ifnames(void)
{
	char *wgn_ifnames = NULL, *lan_ifnames=NULL;
        char word[64], *next = NULL;
        char ath[64], *tmp = NULL;
	char br[20],result[200];
        if (!(wgn_ifnames = nvram_safe_get("wgn_ifnames")))
                return NULL;
	memset(result,0,sizeof(result));
	strlcat(result,nvram_safe_get("lan_ifnames"),sizeof(result));
	strlcat(result," ",sizeof(result));
        foreach (word, wgn_ifnames, next)
        {
		memset(br,0,sizeof(br));
		snprintf(br, sizeof(br), "%s_ifnames", word);
		if((lan_ifnames = nvram_safe_get(br)) != NULL)
		{
        		foreach (ath, lan_ifnames, tmp)
			{
				if(strstr(ath,"ath"))
				{		
				 	if(!strncmp(ath, "ath0.", 5) || !strncmp(ath, "ath1.", 5) || !strncmp(ath, "ath2.", 5))
						continue;
					strlcat(result,ath,sizeof(result));
					strlcat(result," ",sizeof(result));
				}	
			}	
		}	
        }
	result[strlen(result)-1] = '\0';
	return strdup(result);
}	
#endif

/*
 * int get_psta_status(int unit)
 *
 * return value
 * 	0: init
 * 	1:
 * 	2: connect and auth
 * 	3: stop
 */
int get_psta_status(int unit)
{
	int ret;
	const char *sta;

	unit = swap_5g_band(unit);

	sta = get_staifname(unit);
	ret = chk_assoc(sta);
	if (ret < 0) return WLC_STATE_STOPPED;
	if (ret > 0) return WLC_STATE_CONNECTED;
	return ret;
}

void Pty_stop_wlc_connect(int band)
{
	band = swap_5g_band(band);
	set_wpa_cli_cmd(band, "disconnect", 0);
}

#ifdef RTCONFIG_BHCOST_OPT
#if defined(RTAX89U)
#define PORT_UNITS 11
#elif defined(RTAC59_CD6R) || defined(RTAC59_CD6N)
#define PORT_UNITS 6
#else
#define PORT_UNITS 6
#endif

//Aimesh RE: vport to eth name
static const char *query_ifname[PORT_UNITS] = { //Aimesh RE
#if defined(RTAX89U)
//	P0	P1	P2	P3	P4	P5	P6	P7	P8	P9	P10
	"eth2", "eth1", "eth0", "eth0", "eth0", "eth0", "eth0", "eth0", "eth3", "eth5", "eth4"
#elif defined(RTAC59_CD6R) || defined(RTAC59_CD6N)
//	P0	P1	P2	P3	P4	P5
	NULL,   "vlan1",NULL,   NULL,   "vlan4",NULL
#elif defined(PLAX56_XP4)
//	P0	P1	P2	P3	P4	P5
	"eth2",	"eth3",	"eth1",	NULL,	"eth4",	NULL
#else
//	P0	P1	P2	P3	P4	P5
	NULL,   NULL,   NULL,   NULL,   NULL,   NULL
#endif
};

void Pty_start_wlc_connect(int band, char *bssid)
{
	char *sta;

	band = swap_5g_band(band);

	if (bssid != NULL) {
		sta = get_staifname(band);
		if (chk_assoc(sta)==0 || diff_current_bssid(band, bssid)) {	//Restart the network with configured BSSID
			doSystem("wpa_cli -p /var/run/wpa_supplicant-%s disable_network 0", sta);
			doSystem("wpa_cli -p /var/run/wpa_supplicant-%s set_network 0 bssid %s", sta, bssid);
			doSystem("wpa_cli -p /var/run/wpa_supplicant-%s enable_network 0", sta);
			logmessage("AMAS RE", "RE: wpacli set %s's bssid as %s\n", sta, bssid);
		}
	}

	set_wpa_cli_cmd(band, "reconnect", 0);

	return;
}

/**
 * @brief Get DFS status
 *
 * @param band Band
 * @return int Status. 1: CAC 0: Idle
 */
int amas_dfs_status(int band)
{
	return 0;
}

#ifdef RTCONFIG_AMAS_ETHDETECT
unsigned int get_uplinkports_linkrate(char *ifname)
{
	unsigned int link_rate = 0;
	int vport = 0;

	for (vport = 0; vport < PORT_UNITS; vport++) {
		if (vport >= ARRAY_SIZE(query_ifname)) {
			dbg("%s: don't know vport %d\n", __func__, vport);
			return 0;
		}
		if (query_ifname[vport] != NULL && strstr(query_ifname[vport],ifname)) {
			if (rtkswitch_Port_phyStatus(1 << vport)) //connect
			{	
				link_rate = rtkswitch_Port_phyLinkRate(1 << vport);
				break;
			}
		}
	}
	
	return link_rate;
}
/**
 * @brief Get the uplinkports status
 *
 * @param ifname ethernet uplink ifname
 * @return int connnected(1) or not(0)
 */

int get_uplinkports_status(char *ifname)
{
	int vport = 0;

	for (vport = 0; vport < PORT_UNITS; vport++) {
		if (vport >= ARRAY_SIZE(query_ifname)) {
			dbg("%s: don't know vport %d\n", __func__, vport);
			return 0;
		}
		if (query_ifname[vport] != NULL && strstr(query_ifname[vport],ifname)) {
			if (rtkswitch_Port_phyStatus(1 << vport))
				return 1;
		}
	}
	return 0;
}
#else
/**
 * @brief Get the uplinkports status
 *
 * @param ifname ethernet uplink ifname
 * @return int connnected(1) or not(0)
 */
int get_uplinkports_status(char *ifname)
{
	int wan_unit = wan_primary_ifunit();

	return get_wanports_status(wan_unit);
}

unsigned int get_uplinkports_linkrate(char *ifname)
{
	int speed;
	char *eth=NULL;
	speed=0;
	eth=nvram_safe_get("eth_ifnames");
	if(eth && strstr(eth,ifname))
		speed=rtkswitch_WanPort_phySpeed();
	return speed;
}	

#endif
#else
void Pty_start_wlc_connect(int band)
{
	band = swap_5g_band(band);
	set_wpa_cli_cmd(band, "reconnect", 0);
}
#endif

/*
 * int Pty_get_upstream_rssi(int band)
 *
 * return value
 * 	a rssi value which is a native value. The lower the smaller signal.
 *
 * 	-91: RSSI_NO_SIGNAL
 */
int Pty_get_upstream_rssi(int band)
{
	char *sta;
	int status, quality, signal, noise, update;
	int ret;

	if (band < 0 || band > MAX_NR_WL_IF)
		return 0;

	sta = get_staifname(swap_5g_band(band));
	extern int get_wl_status(const char *ifname, int *status, int *quality, int *signal, int *noise, unsigned int *update);
	ret = get_wl_status(sta, &status, &quality, &signal, &noise, &update);
	if (ret > 0 && status)
		return signal;

	return 0;
}

/*
 * int get_wlan_service_status(int bssidx, int vifidx)
 *
 * Get the status of interface that indicate by bssidx and vifidx.
 *
 * return
 * 	-1: invalid
 * 	 0: inactive
 * 	 1: active
 */
int get_wlan_service_status(int bssidx, int vifidx)
{
	int ret;
	char athfix[8],ifname[20];
	if (nvram_get_int("wlready") == 0)
		return -1;
	if(bssidx < 0 || bssidx >= MAX_NR_WL_IF || vifidx < 0 || vifidx >= MAX_NO_MSSID)
		return -1;

	if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
		if(vifidx == 0) //sta
			strcpy(athfix, get_staifname(swap_5g_band(bssidx)));
		else if(vifidx == 1) //main ap
			__get_wlifname(swap_5g_band(bssidx), 0, athfix);
		else //guestnetwork or vif
		{	
			snprintf(ifname,sizeof(ifname), "wl%d.%d_ifname", swap_5g_band(bssidx), vifidx);
			if(strlen(nvram_safe_get(ifname)))
				strcpy(athfix,nvram_get(ifname));
			else
				return -1;
		}			
	}
	else {
		__get_wlifname(swap_5g_band(bssidx), vifidx, athfix);
	}

	ret = is_intf_up(athfix);
	return ret;
}

void set_wlan_service_status(int bssidx, int vifidx, int enabled)
{
	int cfg_stat;
	char athfix[8];
        char tmp[20],tmp2[20];
	if (nvram_get_int("wlready") == 0)
                return;
	if(bssidx < 0 || bssidx >= MAX_NR_WL_IF || vifidx < 0 || vifidx >= MAX_NO_MSSID)
		return;

	if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
		if(vifidx <= 0) {  //for sta
			strcpy(athfix, get_staifname(swap_5g_band(bssidx)));
			doSystem("ifconfig %s %s", athfix, enabled?"up":"down");
			return;
		}
		if(vifidx==1)
			vifidx--;
	}
	cfg_stat = nvram_get_int("cfg_alive");

	if(cfg_stat)
	{	
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			snprintf(tmp, sizeof(tmp), "wl%d_qca_sched", bssidx);
			snprintf(tmp2, sizeof(tmp2), "wl%d_timesched", bssidx);
			if(nvram_get_int(tmp2)==1 ) //wifi sched is enabled
			{
				if(nvram_get_int(tmp)==0) //sched is radio-off
				{
					//_dprintf("radio[%d] should be left to wifi-sched\n",bssidx);	
					return; 
				}	
			}	
		}	
	}
	set_radio(enabled, swap_5g_band(bssidx), vifidx);
}

/*
 * get_radar_status()
 *
 * Emily:
 * If platform in RE mode can re-associate to PAP after all DFS channels are blocked by radar 30 minutes, it don't need this function.
 *
 * Lencer:
 * In fact, this is not called in asuswrt-router-3004 branch in a 10 hour test.
 *
 */

int get_radar_status(int bssidx)
{
	return 0;
}

/* Pty_procedure_check()
 *
 * a wordaround in DFS environment
 *
 * On dualband platform: RE would not limite DFS channel
 * On triband  platform: RE would use +112/80 +136u +108l +100l +140 +132 +116" with higher pritority when radar is detected.
 *
 */
int Pty_procedure_check(int unit, int wlif_count)
{
	return 0;
}

char *get_pap_bssid(int unit, char bssid_str[])
{
	struct iwreq wrq;
	char *sta;
	int ret;
	unsigned char *mac;

	*bssid_str = '\0';
	sta = get_staifname(swap_5g_band(unit));
	if ((ret = get_ap_mac(sta, &wrq)) >= 0) {
		mac = wrq.u.ap_addr.sa_data;
		snprintf(bssid_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	return bssid_str;
}

int diff_current_bssid(int unit, char bssid_str[])
{
	char cur_bssid[18];
	int i,diff;

	get_pap_bssid(unit, cur_bssid);
	if (strcmp(cur_bssid, "00:00:00:00:00:00") != 0) {
		for(i=0; i<17; i++) {
			diff = abs((int)(*(cur_bssid+i)-*(bssid_str+i)));
			if(diff==0 || diff==32)
				continue;
			else {
				logmessage("AMAS RE", "Change %s's bssid from %s to %s\n", unit?"5G":"2G", cur_bssid, bssid_str);
				return 1;
			}

		}
		logmessage("AMAS RE", "Current serving-ap and the best serving-ap are the same, no restart required.\n");
		return 0;
	}
	else
		logmessage("AMAS RE", "Can not get %s's current pap bssid!!\n", unit?"5G":"2G");

	return 1;
}

int wl_get_bw(int unit)
{
	char athfix[IFNAMSIZ];
	int bw, nctrlsb;

	__get_wlifname(swap_5g_band(unit), 0, athfix);
	get_bw_nctrlsb(athfix, &bw, &nctrlsb);
	return bw;
}
#endif

#ifdef RTCONFIG_CFGSYNC

#define check_re_in_macfilter(...) (0)

#define TMP_UPDATE_MACFILTER "/tmp/update_macfilter.sh"
void update_macfilter_relist(void)
{
	char word[16], *next;
	char mac2g[32], mac5g[32], *next_mac;
	int unit = 0;
	char *nv, *nvp, *b;
	char *reMac, *maclist2g, *maclist5g, *timestamp;
	char stamac2g[18] = {0};
	char stamac5g[18] = {0};
	char *cfg_relist;
	FILE *fp;
	char qca_mac[32];
	char *sec = "";

	nvram_unset("relist_ready");
	while (nvram_get_int("wlready") != 1)
		sleep(1);

	unlink(TMP_UPDATE_MACFILTER);
	if ((fp = fopen(TMP_UPDATE_MACFILTER, "w")) == NULL) {
		perror(TMP_UPDATE_MACFILTER);
		return;
	}

	fprintf(fp, "#!/bin/sh\n");
	set_macfilter_all(fp);
	if ((cfg_relist = nvram_get("cfg_relist")) && *cfg_relist != '\0') {
#ifdef RTCONFIG_AMAS
		if (nvram_get_int("re_mode") == 1) {
			nv = nvp = strdup(cfg_relist);
			if (nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &reMac, &maclist2g, &maclist5g, &timestamp) != 4))
						continue;
					/* first mac for sta 2g of dut */
					foreach_44 (mac2g, maclist2g, next_mac)
						break;
					/* first mac for sta 5g of dut */
					foreach_44 (mac5g, maclist5g, next_mac)
						break;

					if (strcmp(reMac, get_2g_hwaddr()) == 0) {
						snprintf(stamac2g, sizeof(stamac2g), "%s", mac2g);
						dbg("dut 2g sta (%s)\n", stamac2g);
						snprintf(stamac5g, sizeof(stamac5g), "%s", mac5g);
						dbg("dut 5g sta (%s)\n", stamac5g);
						break;
					}
				}
				free(nv);
			}
		}
#endif

#ifdef RTCONFIG_WIFI_SON
		if (nvram_match("wifison_ready", "1"))
			sec = "_sec";
#endif
#ifdef RTCONFIG_QCA_LBD
		if (nvram_match("smart_connect_x", "1"))
			sec = "_sec";
#endif
		sprintf(qca_mac, "%s%s", QCA_ADDMAC, sec);

		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			char tmp[32], prefix[16] = "wlXXXXXXXXXX_";
			char athfix[8];
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

#ifdef RTCONFIG_AMAS
			if (nvram_get_int("re_mode") == 1)
				snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
			else
#endif
				snprintf(prefix, sizeof(prefix), "wl%d_", unit);

			__get_wlifname(swap_5g_band(unit), 0, athfix);

			if (nvram_match(strcat_r(prefix, "macmode", tmp), "allow")) {
				nv = nvp = strdup(nvram_safe_get("cfg_relist"));
				if (nv) {
					while ((b = strsep(&nvp, "<")) != NULL) {
						if ((vstrsep(b, ">", &reMac, &maclist2g, &maclist5g, &timestamp) != 4))
							continue;

						if (strcmp(reMac, get_lan_hwaddr()) == 0)
							continue;

						if (unit == 0) {
							foreach_44 (mac2g, maclist2g, next_mac) {
								if (check_re_in_macfilter(unit, mac2g))
									continue;
								printf("relist sta (%s) in %s\n", mac2g, athfix);
								fprintf(fp, IWPRIV " %s %s %s\n", athfix, qca_mac, mac2g);
							}
						}
						else {
							foreach_44 (mac5g, maclist5g, next_mac) {
								if (check_re_in_macfilter(unit, mac5g))
									continue;
								printf("relist sta (%s) in %s\n", mac5g, athfix);
								fprintf(fp, IWPRIV " %s %s %s\n", athfix, qca_mac, mac5g);
							}
						}
					}
					free(nv);
				}
			}

			unit++;
		}
	}
	fflush(fp);
	fclose(fp);
	chmod(TMP_UPDATE_MACFILTER, 0777);
	eval(TMP_UPDATE_MACFILTER);
	nvram_set("relist_ready", "1");
}
#endif

#if defined(RTCONFIG_LYRA_5G_SWAP)
int swap_5g_band(int band)
{
#if defined(RTCONFIG_WIFI_SON)
	if (nvram_match("wifison_ready", "1"))
		return band;
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
	if (sw_mode() == SW_MODE_REPEATER)
		return band;
#endif
	switch(band)
	{
		case 1:
			return 2;
			break;
		case 2:
			return 1;
			break;
		default:
			return band;
			break;
	}
	return 0;
}
#endif

#if defined(RTCONFIG_QCA_LBD)
static int make_lbd_conn(char *ip, int port)
{
	int sfd, flags;
	struct sockaddr_in rsock;
	fd_set wmask;
	struct timeval select_timeout;

	sfd = socket(AF_INET,SOCK_STREAM,0);
	if ( sfd == -1 )
		return -1;

	flags = fcntl(sfd, F_GETFL, 0);
	fcntl(sfd, F_SETFL, flags | O_NONBLOCK);

	memset ((char *)&rsock,0,sizeof(rsock));
	rsock.sin_addr.s_addr = inet_addr(ip);
	rsock.sin_family = AF_INET;
	rsock.sin_port = htons(port);
	if ( connect(sfd,(struct sockaddr *)(&rsock),sizeof(rsock)) == -1 ) {
		if ( errno != EINPROGRESS )
			return -2;
	}

	FD_ZERO(&wmask);
	FD_SET(sfd, &wmask);
	select_timeout.tv_sec = 1;
	select_timeout.tv_usec= 500000;

	if (select(sfd+1, NULL, &wmask, NULL, &select_timeout) <= 0) {
		close(sfd);
		return -3;
	}

	fcntl(sfd, F_SETFL, flags&~O_NONBLOCK);
	return sfd;
}

static char *get_lbd_data(int sfd, int *rlen, char *terminate)
{
	char *recv_buf;
	int block_len, buf_len, recv_len;
	int ok = 0;
	int terminate_len;
	fd_set rmask;
	struct timeval select_timeout;

	block_len = 500;
	buf_len = recv_len = 0;
	recv_buf = NULL;
	if (terminate)
		terminate_len = strlen(terminate);
	else
		terminate_len = 0;
	while (1) {
		int len;
		char *tmp;

		if (recv_len + block_len >= buf_len) {
			buf_len += block_len;
			tmp = realloc(recv_buf, buf_len + 1);
			if (!tmp)
				break;
			recv_buf = tmp;
		}

		FD_ZERO(&rmask);
		FD_SET(sfd, &rmask);
		select_timeout.tv_sec = 1;
		select_timeout.tv_usec= 0;
		if  (select(sfd+1, &rmask, NULL, NULL, &select_timeout) <= 0)
			break;

		len = read(sfd, recv_buf+recv_len, block_len);
		if (len < 0)
			break;
		recv_len += len;
		if ((terminate_len) && (recv_len >= terminate_len)) {
			if (memcmp(recv_buf+recv_len-terminate_len, terminate, terminate_len) == 0)
				ok = 1;
		}
		if (ok) {
			recv_buf[recv_len] = '\0';
			break;
		}
	};
	if (!ok) {
		if (recv_buf)
			free(recv_buf);
		return NULL;
	}
	if (rlen)
		*rlen = recv_len;
	return recv_buf;
}

/* nosteer */
char *set_steer(const char *mac,int val)
{
	int sock;
	char outbuf[100];
	char *got_buf=NULL;
	int len;

	sock = make_lbd_conn("127.0.0.1", 7787);
	if (sock < 0)
		return NULL;

	got_buf = get_lbd_data(sock, &len, "@ ");
	if (!got_buf)
		goto fail;
	free(got_buf);

	if (mac==NULL)
		goto fail;
	else
		len = sprintf(outbuf, "stadb nosteer %s %d\r\n", mac,val);
	write(sock, outbuf, len);
	got_buf = get_lbd_data(sock, &len, "@ ");
	if (!got_buf)
		goto fail;

	len = sprintf(outbuf, "q q\r\n");
	write(sock, outbuf, len);
fail:
	close(sock);
	return got_buf;
}
#endif
