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
 * Copyright 2014, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ASUSTeK Inc..
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bcmnvram.h>
#include <wlioctl.h>
#include <wlutils.h>

#include <httpd.h>

#include <shutils.h>
#include <shared.h>

#include "web-qsr10g.h"

#define	MAX_RETRY_TIMES	30
#define	MAX_TOTAL_TIME	120

#if 0	/* remove */
static int qtn_qcsapi_init = 0;
#endif
static int qtn_init = 0;

#if 1	/* RT-AC87U, src-rt-6.x.4708 */
extern chanspec_t wf_chspec_aton(char *a);
#endif
extern char *wl_vifname_qtn(int unit, int subunit);

#define SSID_FMT_BUF_LEN 4*32+1	/* Length for SSID format string */
#define	MAX_STA_COUNT	100

#ifdef RTCONFIG_PROXYSTA
#define	NVRAM_BUFSIZE	100
#endif

#define ETHER_ADDR_STR_LEN	18
#define DEV_NUMIFS	16	/* Max. # of devices/interfaces supported */

/* include/bcmwifi.h */
#ifndef ASSERT
#define ASSERT(exp)
#endif

char buf[WLC_IOCTL_MAXLEN];

/* shared/bcmwifi.c */
/* given a chanspec string, convert to a chanspec.
 * On error return 0
 */
chanspec_t
wf_chspec_aton(char *a)
{
	char *endp;
	uint channel, band, bw, ctl_sb;
	char c;

	channel = strtoul(a, &endp, 10);

	/* check for no digits parsed */
	if (endp == a)
		return 0;

	if (channel > MAXCHANNEL)
		return 0;

	band = ((channel <= CH_MAX_2G_CHANNEL) ? WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G);
	bw = WL_CHANSPEC_BW_20;
	ctl_sb = WL_CHANSPEC_CTL_SB_NONE;

	a = endp;

	c = tolower(a[0]);
	if (c == '\0')
		goto done;

	/* parse the optional ['A' | 'B'] band spec */
	if (c == 'a' || c == 'b') {
		band = (c == 'a') ? WL_CHANSPEC_BAND_5G : WL_CHANSPEC_BAND_2G;
		a++;
		c = tolower(a[0]);
		if (c == '\0')
			goto done;
	}

	/* parse bandwidth 'N' (10MHz) or 40MHz ctl sideband ['L' | 'U'] */
	if (c == 'n') {
		bw = WL_CHANSPEC_BW_10;
	} else if (c == 'l') {
		bw = WL_CHANSPEC_BW_40;
		ctl_sb = WL_CHANSPEC_CTL_SB_LOWER;
		/* adjust channel to center of 40MHz band */
		if (channel <= (MAXCHANNEL - CH_20MHZ_APART))
			channel += CH_10MHZ_APART;
		else
			return 0;
	} else if (c == 'u') {
		bw = WL_CHANSPEC_BW_40;
		ctl_sb = WL_CHANSPEC_CTL_SB_UPPER;
		/* adjust channel to center of 40MHz band */
		if (channel > CH_20MHZ_APART)
			channel -= CH_10MHZ_APART;
		else
			return 0;
	} else {
		return 0;
	}

done:
	return (channel | band | bw | ctl_sb);
}

/* src-rt-6.x.4708/wl/exe/wlu.c */
char *
wl_ether_etoa(const struct ether_addr *n)
{
	static char etoa_buf[ETHER_ADDR_LEN * 3];
	char *c = etoa_buf;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", n->ether_addr_octet[i] & 0xff);
	}
	return etoa_buf;
}

/* shared/bcmwifi.c */
/*
 * This function returns the channel number that control traffic is being sent on, for legacy
 * channels this is just the channel number, for 40MHZ channels it is the upper or lowre 20MHZ
 * sideband depending on the chanspec selected
 */
uint8
wf_chspec_ctlchan(chanspec_t chspec)
{
	uint8 ctl_chan;

	/* Is there a sideband ? */
	if (CHSPEC_CTL_SB(chspec) == WL_CHANSPEC_CTL_SB_NONE) {
		return CHSPEC_CHANNEL(chspec);
	} else {
		/* we only support 40MHZ with sidebands */
		ASSERT(CHSPEC_BW(chspec) == WL_CHANSPEC_BW_40);
		/* chanspec channel holds the centre frequency, use that and the
		 * side band information to reconstruct the control channel number
		 */
		if (CHSPEC_CTL_SB(chspec) == WL_CHANSPEC_CTL_SB_UPPER) {
			/* control chan is the upper 20 MHZ SB of the 40MHZ channel */
			ctl_chan = UPPER_20_SB(CHSPEC_CHANNEL(chspec));
		} else {
			ASSERT(CHSPEC_CTL_SB(chspec) == WL_CHANSPEC_CTL_SB_LOWER);
			/* control chan is the lower 20 MHZ SB of the 40MHZ channel */
			ctl_chan = LOWER_20_SB(CHSPEC_CHANNEL(chspec));
		}
	}

	return ctl_chan;
}

/* httpd/sysdeps/web-broadcom-wl6.c */
int
ej_wl_sta_status(int eid, webs_t wp, char *name)
{
	// TODO
	return 0;
}

int rpc_qcsapi_init(int verbose)
{
	// const char *host;
	char host[18];
	CLIENT *clnt;
	int retry = 0;
	time_t start_time = uptime();

	/* pcie */
	snprintf(host, sizeof(host), "localhost");

	/* setup RPC based on udp protocol */
	do {
		if (verbose)
			dbG("#%d attempt to create RPC connection\n", retry + 1);

		clnt = clnt_pci_create(host, QCSAPI_PROG, QCSAPI_VERS, NULL);

		if (clnt == NULL) {
			_dprintf("clnt_pci_create() error\n");
			clnt_pcreateerror(host);
			sleep(1);
			continue;
		} else {
			_dprintf("clnt_pci_create() OK, set_rpcclient()\n");
			client_qcsapi_set_rpcclient(clnt);
#if 0	/* remove */
			qtn_qcsapi_init = 1;
#endif
			return 0;
		}
	} while ((retry++ < MAX_RETRY_TIMES) && ((uptime() - start_time) < MAX_TOTAL_TIME));

	return -1;
}

int rpc_qtn_ready()
{
	int ret, qtn_ready;
	int lock;

	qtn_ready = nvram_get_int("qtn_ready");

	lock = file_lock("qtn");

	if (qtn_ready && !qtn_init)
	{
		ret = rpc_qcsapi_init(0);
		if (ret < 0){
			qtn_ready = 0;
			dbG("rpc_qcsapi_init error, return: %d\n", ret);
		}else
		{
			ret = qcsapi_init();
			if (ret < 0){
				qtn_ready = 0;
				dbG("Qcsapi qcsapi_init error, return: %d\n", ret);
			}else
				qtn_init = 1;
		}
	}

	file_unlock(lock);

	// nvram_set("wl1_country_code", nvram_safe_get("1:ccode"));
	return qtn_ready;
}

int rpc_qcsapi_deny_mac_address(const char *ifname, const char *macaddr)
{
	int ret;
	qcsapi_mac_addr address_to_deny;

	if (!rpc_qtn_ready())
		return -1;

	ether_atoe(macaddr, address_to_deny);
	ret = qcsapi_wifi_deny_mac_address(ifname, address_to_deny);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_deny_mac_address %s error, return: %d\n", ifname, ret);
		return ret;
	}
//	dbG("deny MAC addresss of interface %s: %s\n", ifname, macaddr);

	return 0;
}

int rpc_qcsapi_get_SSID(const char *ifname, qcsapi_SSID *ssid)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_SSID(ifname, (char *) ssid);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_SSID %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_SSID_broadcast(const char *ifname, int *p_current_option)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_option(ifname, qcsapi_SSID_broadcast, p_current_option);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_option::SSID_broadcast %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_vht(qcsapi_unsigned_int *vht)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_vht(WIFINAME, vht);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_vht %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_bw(qcsapi_unsigned_int *p_bw)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_bw(WIFINAME, p_bw);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_bw %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_channel(qcsapi_unsigned_int *p_channel, int unit)
{
	int ret;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	if (!rpc_qtn_ready())
		return -1;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	if(unit == 0)
		ret = qcsapi_wifi_get_channel(WIFINAME2G, p_channel);
	else
		ret = qcsapi_wifi_get_channel(WIFINAME, p_channel);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_channel %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_snr(void)
{
	int ret;
	int snr;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_avg_snr(WIFINAME, &snr);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_avg_snr %s error, return: %d\n", WIFINAME, ret);
		return 0;
	}

	return snr;
}

int rpc_qcsapi_get_channel_list(string_1024* list_of_channels)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_list_channels(WIFINAME, *list_of_channels);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_list_channels %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_beacon_type(const char *ifname, char *p_current_beacon)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_beacon_type(ifname, p_current_beacon);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_beacon_type %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_WPA_encryption_modes(const char *ifname, char *p_current_encryption_mode)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_WPA_encryption_modes(ifname, p_current_encryption_mode);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_WPA_encryption_modes %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_key_passphrase(const char *ifname, char *p_current_key_passphrase)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_key_passphrase(ifname, 0, p_current_key_passphrase);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_key_passphrase %s error, return: %d\n", ifname, ret);

		ret = qcsapi_wifi_get_pre_shared_key(ifname, 0, p_current_key_passphrase);
		if (ret < 0)
			dbG("Qcsapi qcsapi_wifi_get_pre_shared_key %s error, return: %d\n", ifname, ret);

		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_dtim(qcsapi_unsigned_int *p_dtim)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_dtim(WIFINAME, p_dtim);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_dtim %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_denied_mac_addresses(const char *ifname, char *list_mac_addresses, const unsigned int sizeof_list)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_denied_mac_addresses(ifname, list_mac_addresses, sizeof_list);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_denied_mac_addresses %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_interface_get_mac_addr(const char *ifname, qcsapi_mac_addr *current_mac_addr)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_interface_get_mac_addr(ifname, (uint8_t *) current_mac_addr);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_interface_get_mac_addr %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_get_state(const char *ifname, char *wps_state, const qcsapi_unsigned_int max_len)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_get_state(ifname, wps_state, max_len);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_get_state %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wifi_disable_wps(const char *ifname, int disable_wps)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_disable_wps(ifname, disable_wps);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_disable_wps %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_get_ap_pin(const char *ifname, char *wps_pin, int force_regenerate)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_get_ap_pin(ifname, wps_pin, force_regenerate);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_get_ap_pin %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_get_configured_state(const char *ifname, char *wps_state, const qcsapi_unsigned_int max_len)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_get_configured_state(ifname, wps_state, max_len);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_get_configured_state %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_cancel(const char *ifname)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_cancel(ifname);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_cancel %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_registrar_report_button_press(const char *ifname)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_registrar_report_button_press(ifname);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_registrar_report_button_press %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_registrar_report_pin(const char *ifname, const char *wps_pin)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_registrar_report_pin(ifname, wps_pin);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_registrar_report_pin %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_bootcfg_commit(void)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_bootcfg_commit();
	if (ret < 0) {
		dbG("Qcsapi qcsapi_bootcfg_commit error, return: %d\n", ret);
		return ret;
	}
	dbG("QTN commit bootcfg successfully\n");

	return 0;
}

int rpc_update_ap_isolate(const char *ifname, const int isolate)
{
	int ret;

	if(!rpc_qtn_ready())
		return -1;

	qcsapi_wifi_rfenable((qcsapi_unsigned_int) 0 /* off */);
	ret = qcsapi_wifi_set_ap_isolate(ifname, isolate);
	if(ret < 0){
		dbG("qcsapi_wifi_set_ap_isolate %s error, return: %d\n", ifname, ret);
		return ret;
	}else{
		dbG("qcsapi_wifi_set_ap_isolate OK\n");
	}
	if(nvram_get_int("wl1_radio") == 1)
		qcsapi_wifi_rfenable((qcsapi_unsigned_int) 1 /* on */);

	return 0;
}

void rpc_parse_nvram(const char *name, const char *value)
{
	return;	/* move to restart_wireless */
}

#define SET_SSID	0x01
#define SET_CLOSED	0x02
#define SET_AUTH	0x04
#define	SET_CRYPTO	0x08
#define	SET_WPAPSK	0x10
#define	SET_MACMODE	0x20
#define SET_ALL		0x3F

/* differ from wl_vifname_qtn in shared */
char *wl_ifname_qtn_by_unit(int unit)
{
	static char tmp[128];
	int qtn_unit;

	if(unit == 0)
		qtn_unit=2;	/* 2G */
	else
		qtn_unit=0;	/* 5G */

	sprintf(tmp, "wifi%d_0", qtn_unit);
	return strdup(tmp);
}

int wl_wps_info(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char tmpstr[256];
	int retval = 0;
	char tmp[128], prefix[]="wlXXXXXXX_";
	char *wps_sta_pin;
	int ret;
	qcsapi_SSID ssid;
	string_64 key_passphrase;
//	char wps_pin[16];


	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	retval += websWrite(wp, "<wps>\n");

	//0. WSC Status
	if (!strcmp(nvram_safe_get(strcat_r(prefix, "wps_mode", tmp)), "enabled"))
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s radio is not ready</wps_info>\n", wl_ifname_qtn_by_unit(unit));
		else
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", getWscStatusStr_qtn(unit));
	}
	else
		retval += websWrite(wp, "<wps_info>%s Not used</wps_info>\n", wl_ifname_qtn_by_unit(unit));

	//1. WPSConfigured
	if (!rpc_qtn_ready())
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", get_WPSConfiguredStr_qtn(unit));

	//2. WPSSSID
	if (unit == 0 || unit == 1)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
		{
			memset(&ssid, 0, sizeof(qcsapi_SSID));
			ret = rpc_qcsapi_get_SSID(wl_ifname_qtn_by_unit(unit), &ssid);
			if (ret < 0)
				dbG("rpc_qcsapi_get_SSID %s error, return: %d\n", wl_ifname_qtn_by_unit(unit), ret);

			memset(tmpstr, 0, sizeof(tmpstr));
			char_to_ascii(tmpstr, ssid);
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
		}
	}
	else{
		_dprintf("(%s)(%d) no such unit\n", __func__, __LINE__);
	}

	//3. WPSAuthMode
	if (unit == 0 || unit == 1)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", getWPSAuthMode_qtn(unit));
	}
	else
	{
		_dprintf("(%s)(%d) no such unit\n", __func__, __LINE__);
	}

	//4. EncrypType
	if (unit == 0 || unit == 1)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", getWPSEncrypType_qtn(unit));
	}
	else
	{
		_dprintf("(%s)(%d) no such unit\n", __func__, __LINE__);
	}

	//5. DefaultKeyIdx
	if (unit == 0 || unit == 1)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
			retval += websWrite(wp, "<wps_info>%d</wps_info>\n", 1);
	}
	else
	{
		_dprintf("(%s)(%d) no such unit\n", __func__, __LINE__);
	}

	//6. WPAKey
#if 0	//hide for security
	if (unit == 0 || unit == 1)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
		{
			memset(&key_passphrase, 0, sizeof(key_passphrase));
			ret = rpc_qcsapi_get_key_passphrase(WIFINAME, (char *) &key_passphrase);
			if (ret < 0)
				dbG("rpc_qcsapi_get_key_passphrase %s error, return: %d\n", WIFINAME, ret);

			if (!strlen(key_passphrase))
				retval += websWrite(wp, "<wps_info>None</wps_info>\n");
			else
			{
				memset(tmpstr, 0, sizeof(tmpstr));
				char_to_ascii(tmpstr, key_passphrase);
				retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
			}
		}
	}
	else{
		_dprintf("(%s)(%d) no such unit\n", __func__, __LINE__);
	}
#else
	retval += websWrite(wp, "<wps_info></wps_info>\n");
#endif
	//7. AP PIN Code
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "%s", nvram_safe_get("wps_device_pin"));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}

	//8. Saved WPAKey
#if 0	//hide for security
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
		{
			if (!strlen(key_passphrase))
				retval += websWrite(wp, "<wps_info>None</wps_info>\n");
			else
			{
				memset(tmpstr, 0, sizeof(tmpstr));
				char_to_ascii(tmpstr, key_passphrase);
				retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
			}
		}
	}
	else
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
	if (!strcmp(nvram_safe_get(strcat_r(prefix, "wps_mode", tmp)), "enabled"))
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "1");
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "0");

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

/* 5G */
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

int get_wl_channel_list_by_bw_core(int unit, string_1024 list_of_channels, int bw)
{
	int ret;
	int retval = 0;
	char tmp[256], tmp_t[256];
	char *p;
	int i = 0;;
	char cur_ccode[20] = {0};
	char ifname[WIFINAME_MAX_LEN] = {0};

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	sprintf(tmp, "[\"%d\"]", 0);

	if (!rpc_qtn_ready()){
		snprintf(tmp, sizeof(tmp), "");
		goto ERROR;
	}

	// ret = qcsapi_wifi_get_list_channels(WIFINAME2G, (char *) &list_of_channels);
	ret = qcsapi_wifi_get_regulatory_region(ifname, cur_ccode);
	if (ret < 0) {
		dbG("get_regulatory_region %s error[%d]\n", ifname, ret);
		goto ERROR;
	}

	if(strcmp(cur_ccode, "eu")==0 || strcmp(cur_ccode, "jp")==0){
		ret = qcsapi_regulatory_get_list_regulatory_channels_if(ifname, cur_ccode, 40 /* bw */, -1, list_of_channels);
	}else{
		ret = qcsapi_regulatory_get_list_regulatory_channels_if(ifname, cur_ccode, bw, -1, list_of_channels);
	}
	if (ret < 0) {
		dbG("get_list_regulatory_channels %s error[%d]\n", ifname, ret);
		goto ERROR;
	}

	p = strtok((char *) list_of_channels, ",");
	while (p)
	{
		if (i == 0)
			sprintf(tmp, "[\"%s\"", (char *) p);
		else{
			sprintf(tmp_t,  "%s, \"%s\"", tmp, (char *) p);
			strlcpy(tmp, tmp_t, sizeof(tmp));
		}

		p = strtok(NULL, ",");
		i++;
	}

	if (i){
		sprintf(tmp_t,  "%s]", tmp);
		strlcpy(tmp, tmp_t, sizeof(tmp));
	}

ERROR:
	/* list_of_channels = 1024, tmp = 256 */
	sprintf(list_of_channels, "%s", tmp);
	return 0;
}

int ej_wl_channel_list_2g_20m(int eid, webs_t wp, int argc, char_t **argv)
{
	static char list_20m[1024] = {0};
	int retval;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	if(strlen(list_20m) == 0) retval = get_wl_channel_list_by_bw_core(0, list_20m, 20);
	retval = websWrite(wp, "%s", list_20m);
	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	return retval;
}

int ej_wl_channel_list_2g_40m(int eid, webs_t wp, int argc, char_t **argv)
{
	static char list_40m[1024] = {0};
	int retval;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	if(strlen(list_40m) == 0) get_wl_channel_list_by_bw_core(0, list_40m, 40);
	retval = websWrite(wp, "%s", list_40m);
	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	return retval;
}

int
ej_wl_channel_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	static char list_40m[1024] = {0};
	int retval;

	if(strlen(list_40m) == 0) get_wl_channel_list_by_bw_core(0, list_40m, 40);
	retval = websWrite(wp, "%s", list_40m);
	return retval;
}

int ej_wl_channel_list_5g_20m(int eid, webs_t wp, int argc, char_t **argv)
{
	static char list_20m[1024] = {0};
	int retval;

	if(strlen(list_20m) == 0) retval = get_wl_channel_list_by_bw_core(1, list_20m, 20);
	retval = websWrite(wp, "%s", list_20m);
	return retval;
}

int ej_wl_channel_list_5g_40m(int eid, webs_t wp, int argc, char_t **argv)
{
	static char list_40m[1024] = {0};
	int retval;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	if(strlen(list_40m) == 0) get_wl_channel_list_by_bw_core(1, list_40m, 40);
	retval = websWrite(wp, "%s", list_40m);
	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	return retval;
}

int ej_wl_channel_list_5g_80m(int eid, webs_t wp, int argc, char_t **argv)
{
	static char list_80m[1024] = {0};
	int retval;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	if(strlen(list_80m) == 0) get_wl_channel_list_by_bw_core(1, list_80m, 80);
	retval = websWrite(wp, "%s", list_80m);
	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	return retval;
}

int
ej_wl_channel_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	static char list_80m[1024] = {0};
	int retval;

	if(strlen(list_80m) == 0) get_wl_channel_list_by_bw_core(1, list_80m, 80);
	retval = websWrite(wp, "%s", list_80m);
	return retval;
}

int
wl_control_channel(int unit)
{
	_dprintf("Raymond: wl_control_channel()-02\n");
	return 6;
}

int
ej_wl_control_channel(int eid, webs_t wp, int argc, char_t **argv)
{
	_dprintf("Raymond: ej_wl_control_channel-03()\n");
}

int
ej_wl_scan_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	char ssid_str[128];
	char macstr[18];
	int retval = 0;
	int ret, i, j;
	unsigned int ap_count = 0;
	qcsapi_ap_properties ap_current;

	if (!rpc_qtn_ready())
	{
		retval += websWrite(wp, "[]");
		return retval;
	}

	ret = qcsapi_wifi_start_scan_ext(WIFINAME2G, IEEE80211_PICK_ALL | IEEE80211_PICK_NOPICK_BG);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_start_scan %s error, return: %d\n", WIFINAME2G, ret);
		retval += websWrite(wp, "[]");
		return retval;
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < i+1; j++)
			dbg(".");
		sleep(1);
		dbg("\n");
	}

	//Get the scaned AP count
	ret = qcsapi_wifi_get_results_AP_scan(WIFINAME2G, &ap_count);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_results_AP_scan %s error, return: %d\n", WIFINAME2G, ret);
		retval += websWrite(wp, "[]");
		return retval;
	}
	if (ap_count == 0) {
		dbG("Scaned ap count is 0\n");
		retval += websWrite(wp, "[]");
		return retval;
	}

	dbg("%-4s%-33s%-18s\n\n", "Ch", "SSID", "BSSID");

	retval += websWrite(wp, "[");

	for (i = 0; i < ap_count; i++) {
		ret = qcsapi_wifi_get_properties_AP(WIFINAME2G, i, &ap_current);
		if (ret < 0) {
			dbG("Qcsapi qcsapi_wifi_get_properties_AP %s error, return: %d\n", WIFINAME2G, ret);
			goto END;
		}

		memset(ssid_str, 0, sizeof(ssid_str));
		char_to_ascii(ssid_str, ap_current.ap_name_SSID);

		sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
			ap_current.ap_mac_addr[0],
			ap_current.ap_mac_addr[1],
			ap_current.ap_mac_addr[2],
			ap_current.ap_mac_addr[3],
			ap_current.ap_mac_addr[4],
			ap_current.ap_mac_addr[5]);

		dbg("%-4d%-33s%-18s\n",
			ap_current.ap_channel,
			ap_current.ap_name_SSID,
			macstr
		);

		if (!i)
			retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, macstr);
		else
			retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, macstr);
	}

	dbg("\n");
END:
	retval += websWrite(wp, "]");
	return retval;
}

int
ej_wl_scan_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	char ssid_str[128];
	char macstr[18];
	int retval = 0;
	int ret, i, j;
	unsigned int ap_count = 0;
	qcsapi_ap_properties ap_current;

	if (!rpc_qtn_ready())
	{
		retval += websWrite(wp, "[]");
		return retval;
	}

	ret = qcsapi_wifi_start_scan_ext(WIFINAME, IEEE80211_PICK_ALL | IEEE80211_PICK_NOPICK_BG);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_start_scan %s error, return: %d\n", WIFINAME, ret);
		retval += websWrite(wp, "[]");
		return retval;
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < i+1; j++)
			dbg(".");
		sleep(1);
		dbg("\n");
	}

	//Get the scaned AP count
	ret = qcsapi_wifi_get_results_AP_scan(WIFINAME, &ap_count);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_results_AP_scan %s error, return: %d\n", WIFINAME, ret);
		retval += websWrite(wp, "[]");
		return retval;
	}
	if (ap_count == 0) {
		dbG("Scaned ap count is 0\n");
		retval += websWrite(wp, "[]");
		return retval;
	}

	dbg("%-4s%-33s%-18s\n\n", "Ch", "SSID", "BSSID");

	retval += websWrite(wp, "[");

	for (i = 0; i < ap_count; i++) {
		ret = qcsapi_wifi_get_properties_AP(WIFINAME, i, &ap_current);
		if (ret < 0) {
			dbG("Qcsapi qcsapi_wifi_get_properties_AP %s error, return: %d\n", WIFINAME, ret);
			goto END;
		}

		memset(ssid_str, 0, sizeof(ssid_str));
		char_to_ascii(ssid_str, ap_current.ap_name_SSID);

		sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
			ap_current.ap_mac_addr[0],
			ap_current.ap_mac_addr[1],
			ap_current.ap_mac_addr[2],
			ap_current.ap_mac_addr[3],
			ap_current.ap_mac_addr[4],
			ap_current.ap_mac_addr[5]);

		dbg("%-4d%-33s%-18s\n",
			ap_current.ap_channel,
			ap_current.ap_name_SSID,
			macstr
		);

		if (!i)
			retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, macstr);
		else
			retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, macstr);
	}

	dbg("\n");
END:
	retval += websWrite(wp, "]");
	return retval;
}

static int
ej_wl_sta_list_qtn_core(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret, retval = 0, firstRow = 1;;
	qcsapi_unsigned_int association_count = 0;
	qcsapi_mac_addr sta_address;
	int i, rssi, index = -1;
	char prefix[] = "wlXXXXXXXXXX_", tmp[128];
	char ifname[WIFINAME_MAX_LEN] = {0};

	if (!rpc_qtn_ready())
		return retval;

	int from_app = 0;
	char *name_t = NULL;

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	if (strArgs(argc, argv, "%s", &name_t) < 1) {
		//_dprintf("name_t = NULL\n");
	} else if (!strncmp(name_t, "appobj", 6))
		from_app = 1;

	sscanf(ifname, "wifi%d_0", &index);
#if 0
	if (index == -1) return retval;
	else if (index == 0)
		sprintf(prefix, "wl%d_", unit);
	else
		sprintf(prefix, "wl%d.%d_", unit, index);
#else
		sprintf(prefix, "wl%d_", unit);
#endif

	// ret = qcsapi_wifi_get_count_associations((const char *)ifname, &association_count);
	ret = qcsapi_wifi_get_count_associations(ifname, &association_count);
	// _dprintf("[%s][%s], unit[%d], association_count:[%d]\n",
//			__func__, __LINE__, unit, association_count);
	// ret = 0;
	// association_count = 0;
	// ret = qcsapi_wifi_get_count_associations("wifi2_0", &association_count);
	if (ret < 0) {
		_dprintf("wifi_get_count_associations %s error[%d]\n", ifname, ret);
		return retval;
	} else {
		for (i = 0; i < association_count; ++i) {
			rssi = 0;
			ret = qcsapi_wifi_get_associated_device_mac_addr(ifname, i, (uint8_t *) &sta_address);
			if (ret < 0) {
				dbG("get_associated_device_mac_addr %s error[%d]\n",
						ifname, ret);
				return retval;
			} else {
				if (firstRow == 1)
					firstRow = 0;
				else
				retval += websWrite(wp, ", ");
				if(from_app == 0)
					retval += websWrite(wp, "[");

				retval += websWrite(wp, "\"%s\"", wl_ether_etoa((struct ether_addr *) &sta_address));
				if(from_app == 1){
					retval += websWrite(wp, ":{");
					retval += websWrite(wp, "\"isWL\":");
				}
				if(from_app == 0)
					retval += websWrite(wp, ", \"%s\"", "Yes");
				else
					retval += websWrite(wp, "\"%s\"", "Yes");
				if(from_app == 0)
					retval += websWrite(wp, ", \"%s\"", !(nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "open")) ? "Yes" : "No");
				if(from_app == 1){
					ret += websWrite(wp, ",\"rssi\":");
				}
				ret= qcsapi_wifi_get_rssi_in_dbm_per_association(ifname, i, &rssi);
				if (ret < 0)
					dbG("Qcsapi qcsapi_wifi_get_rssi_in_dbm_per_association %s error, return: %d\n", ifname, ret);
				if(from_app == 0)
					retval += websWrite(wp, ", \"%d\"", rssi);
				else
					retval += websWrite(wp, "\"%d\"", rssi);
				if(from_app == 0)
					retval += websWrite(wp, "]");
				else
					retval += websWrite(wp, "}");
			}
		}
	}

	return retval;
}

#ifdef RTCONFIG_STAINFO
static int
ej_wl_stainfo_list_qtn(int eid, webs_t wp, int argc, char_t **argv, const char *ifname)
{
	int ret, retval = 0, firstRow = 1;;
	qcsapi_unsigned_int association_count = 0;
	qcsapi_mac_addr sta_address;
	int i, rssi, index = -1, unit = 1;
	char prefix[] = "wlXXXXXXXXXX_";
	qcsapi_unsigned_int tx_phy_rate, rx_phy_rate, time_associated;
	int hr, min, sec;

	if (!rpc_qtn_ready())
		return retval;

	sscanf(ifname, "wifi%d", &index);
	if (index == -1) return retval;
	else if (index == 0)
		sprintf(prefix, "wl%d_", unit);
	else
		sprintf(prefix, "wl%d.%d_", unit, index);

	ret = qcsapi_wifi_get_count_associations(ifname, &association_count);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_count_associations %s error, return: %d\n", ifname, ret);
		return retval;
	} else {
		for (i = 0; i < association_count; ++i) {
			rssi = 0;
			ret = qcsapi_wifi_get_associated_device_mac_addr(ifname, i, (uint8_t *) &sta_address);
			if (ret < 0) {
				dbG("Qcsapi qcsapi_wifi_get_associated_device_mac_addr %s error, return: %d\n", ifname, ret);
				return retval;
			} else {
				if (firstRow == 1)
					firstRow = 0;
				else
				retval += websWrite(wp, ", ");
				retval += websWrite(wp, "[");

				retval += websWrite(wp, "\"%s\"", wl_ether_etoa((struct ether_addr *) &sta_address));

				tx_phy_rate = rx_phy_rate = time_associated = 0;

				ret = qcsapi_wifi_get_tx_phy_rate_per_association(ifname, i, &tx_phy_rate);
				if (ret < 0)
					dbG("Qcsapi qcsapi_wifi_get_tx_phy_rate_per_association %s error, return: %d\n", ifname, ret);

				ret = qcsapi_wifi_get_rx_phy_rate_per_association(ifname, i, &rx_phy_rate);
				if (ret < 0)
					dbG("Qcsapi qcsapi_wifi_get_rx_phy_rate_per_association %s error, return: %d\n", ifname, ret);

				ret = qcsapi_wifi_get_time_associated_per_association(ifname, i, &time_associated);
				if (ret < 0)
					dbG("Qcsapi qcsapi_wifi_get_time_associated_per_association %s error, return: %d\n", ifname, ret);

				hr = time_associated / 3600;
				min = (time_associated % 3600) / 60;
				sec = time_associated - hr * 3600 - min * 60;

				retval += websWrite(wp, ", \"%d\"", tx_phy_rate);
				retval += websWrite(wp, ", \"%d\"", rx_phy_rate);
				retval += websWrite(wp, ", \"%02d:%02d:%02d\"", hr, min, sec);

				retval += websWrite(wp, "]");
			}
		}
	}

	return retval;
}
#endif

int
ej_wl_sta_list_qtn(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret = 0, retval = 0, ret_t = 0;
	int i;
	char prefix[] = "wlXXXXXXXXXX_", tmp[128];

	if (!rpc_qtn_ready())
		return retval;

	ret += ej_wl_sta_list_qtn_core(eid, wp, argc, argv, unit);

	if (nvram_get_int("sw_mode") == SW_MODE_REPEATER && nvram_get_int("wlc_band"))
		return ret;

	for (i = 1; i < 4; i++) {
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")){
			if (ret_t != ret)
				retval += websWrite(wp, ", ");
			ret += ej_wl_sta_list_qtn_core(eid, wp, argc, argv, unit);
			ret_t = ret;
		}
	}

	return retval;
}

#ifdef RTCONFIG_STAINFO
int
ej_wl_stainfo_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0, retval = 0, ret_t = 0;
	int i, unit = 1;
	char prefix[] = "wlXXXXXXXXXX_", tmp[128];

	if (!rpc_qtn_ready())
		return retval;

	ret += ej_wl_stainfo_list_qtn(eid, wp, argc, argv, WIFINAME2G);

        if (nvram_get_int("sw_mode") == SW_MODE_REPEATER && nvram_get_int("wlc_band"))
                return ret;

	for (i = 1; i < 4; i++) {
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")){
			if (ret_t != ret)
				retval += websWrite(wp, ", ");
			ret += ej_wl_stainfo_list_qtn(eid, wp, argc, argv, wl_vifname_qtn(unit, i));
			ret_t = ret;
		}
	}

	return retval;
}

int
ej_wl_stainfo_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0, retval = 0, ret_t = 0;
	int i, unit = 1;
	char prefix[] = "wlXXXXXXXXXX_", tmp[128];

	if (!rpc_qtn_ready())
		return retval;

	ret += ej_wl_stainfo_list_qtn(eid, wp, argc, argv, WIFINAME);

        if (nvram_get_int("sw_mode") == SW_MODE_REPEATER && nvram_get_int("wlc_band"))
                return ret;

	for (i = 1; i < 4; i++) {
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")){
			if (ret_t != ret)
				retval += websWrite(wp, ", ");
			ret += ej_wl_stainfo_list_qtn(eid, wp, argc, argv, wl_vifname_qtn(unit, i));
			ret_t = ret;
		}
	}

	return retval;
}
#endif

int
wl_status_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret, retval = 0;
	qcsapi_SSID ssid;
	qcsapi_unsigned_int channel;
	qcsapi_unsigned_int bw;
	char chspec_str[8];
	int is_nctrlsb_lower;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	if (!rpc_qtn_ready())
		return 0;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	memset(&ssid, 0, sizeof(qcsapi_SSID));
	ret = rpc_qcsapi_get_SSID(WIFINAME2G, &ssid);
	if (ret < 0)
		dbG("rpc_qcsapi_get_SSID %s error, return: %d\n", WIFINAME2G, ret);
	retval += websWrite(wp, "SSID: \"%s\"\n", ssid);

	int rssi_by_chain[4], rssi;
	qcsapi_wifi_get_rssi_by_chain(WIFINAME2G, 0, &rssi_by_chain[0]);
	qcsapi_wifi_get_rssi_by_chain(WIFINAME2G, 1, &rssi_by_chain[1]);
	qcsapi_wifi_get_rssi_by_chain(WIFINAME2G, 2, &rssi_by_chain[2]);
	qcsapi_wifi_get_rssi_by_chain(WIFINAME2G, 3, &rssi_by_chain[3]);
	rssi = (rssi_by_chain[0] + rssi_by_chain[1] + rssi_by_chain[2] + rssi_by_chain[3]) / 4;

	retval += websWrite(wp, "RSSI: %d dBm\t", 0);

	retval += websWrite(wp, "SNR: %d dB\t", rpc_qcsapi_get_snr());

	retval += websWrite(wp, "noise: %d dBm\t", rssi);

	ret = rpc_qcsapi_get_channel(&channel, 0);
	if (ret < 0)
		dbG("rpc_qcsapi_get_channel error, return: %d\n", ret);

	ret = rpc_qcsapi_get_bw(&bw);
	if (ret < 0)
		dbG("rpc_qcsapi_get_bw error, return: %d\n", ret);

	if (bw == 80)
		sprintf(chspec_str, "%d/%d", channel, bw);
	else if (bw == 40)
	{
		is_nctrlsb_lower = ((channel == 36) || (channel == 44) || (channel == 52) || (channel == 60) || (channel == 100) || (channel == 108) || (channel == 116) || (channel == 124) || (channel == 132) || (channel == 149) || (channel == 157));
		sprintf(chspec_str, "%d%c", channel, is_nctrlsb_lower ? 'l': 'u');
	}
	else
		sprintf(chspec_str, "%d", channel);

	retval += websWrite(wp, "Channel: %s\n", chspec_str);

	qcsapi_mac_addr wl_mac_addr;
	ret = rpc_qcsapi_interface_get_mac_addr(WIFINAME2G, &wl_mac_addr);
	if (ret < 0)
		dbG("rpc_qcsapi_interface_get_mac_addr %s error, return: %d\n", WIFINAME2G, ret);

	retval += websWrite(wp, "BSSID: %s\t", wl_ether_etoa((struct ether_addr *) &wl_mac_addr));

	retval += websWrite(wp, "Supported Rates: [ 6(b) 9 12(b) 18 24(b) 36 48 54 ]\n");

	retval += websWrite(wp, "\n");

	return retval;
}

int
wl_status_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret, retval = 0;
	qcsapi_SSID ssid;
	qcsapi_unsigned_int channel;
	qcsapi_unsigned_int bw;
	char chspec_str[8];
	int is_nctrlsb_lower;

	if (!rpc_qtn_ready())
		return 0;

	memset(&ssid, 0, sizeof(qcsapi_SSID));
	ret = rpc_qcsapi_get_SSID(WIFINAME, &ssid);
	if (ret < 0)
		dbG("rpc_qcsapi_get_SSID %s error, return: %d\n", WIFINAME, ret);
	retval += websWrite(wp, "SSID: \"%s\"\n", ssid);

	int rssi_by_chain[4], rssi;
	qcsapi_wifi_get_rssi_by_chain(WIFINAME, 0, &rssi_by_chain[0]);
	qcsapi_wifi_get_rssi_by_chain(WIFINAME, 1, &rssi_by_chain[1]);
	qcsapi_wifi_get_rssi_by_chain(WIFINAME, 2, &rssi_by_chain[2]);
	qcsapi_wifi_get_rssi_by_chain(WIFINAME, 3, &rssi_by_chain[3]);
	rssi = (rssi_by_chain[0] + rssi_by_chain[1] + rssi_by_chain[2] + rssi_by_chain[3]) / 4;

	retval += websWrite(wp, "RSSI: %d dBm\t", 0);

	retval += websWrite(wp, "SNR: %d dB\t", rpc_qcsapi_get_snr());

	retval += websWrite(wp, "noise: %d dBm\t", rssi);

	ret = rpc_qcsapi_get_channel(&channel, 1);
	if (ret < 0)
		dbG("rpc_qcsapi_get_channel error, return: %d\n", ret);

	ret = rpc_qcsapi_get_bw(&bw);
	if (ret < 0)
		dbG("rpc_qcsapi_get_bw error, return: %d\n", ret);

	if (bw == 80)
		sprintf(chspec_str, "%d/%d", channel, bw);
	else if (bw == 40)
	{
		is_nctrlsb_lower = ((channel == 36) || (channel == 44) || (channel == 52) || (channel == 60) || (channel == 100) || (channel == 108) || (channel == 116) || (channel == 124) || (channel == 132) || (channel == 149) || (channel == 157));
		sprintf(chspec_str, "%d%c", channel, is_nctrlsb_lower ? 'l': 'u');
	}
	else
		sprintf(chspec_str, "%d", channel);

	retval += websWrite(wp, "Channel: %s\n", chspec_str);

	qcsapi_mac_addr wl_mac_addr;
	ret = rpc_qcsapi_interface_get_mac_addr(WIFINAME, &wl_mac_addr);
	if (ret < 0)
		dbG("rpc_qcsapi_interface_get_mac_addr %s error, return: %d\n", WIFINAME, ret);

	retval += websWrite(wp, "BSSID: %s\t", wl_ether_etoa((struct ether_addr *) &wl_mac_addr));

	retval += websWrite(wp, "Supported Rates: [ 6(b) 9 12(b) 18 24(b) 36 48 54 ]\n");

	retval += websWrite(wp, "\n");

	return retval;
}

/* core of 2G and 5G wl_status */
static int
ej_wl_status_qtn_core(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret, retval = 0, i, rssi;
	qcsapi_unsigned_int association_count = 0, tx_phy_rate, rx_phy_rate, time_associated;
	qcsapi_mac_addr sta_address;
	int hr, min, sec;
	char ifname[WIFINAME_MAX_LEN] = {0};

	if (!rpc_qtn_ready())
		return retval;

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	ret = qcsapi_wifi_get_count_associations(ifname, &association_count);
	if (ret >= 0) {
		for (i = 0; i < association_count; ++i) {
			rssi = tx_phy_rate = rx_phy_rate = time_associated = 0;

			ret = qcsapi_wifi_get_associated_device_mac_addr(ifname, i, (uint8_t *) &sta_address);
			if (ret < 0)
				dbG("Qcsapi qcsapi_wifi_get_associated_device_mac_addr %s error, return: %d\n", ifname, ret);

			ret= qcsapi_wifi_get_rssi_in_dbm_per_association(ifname, i, &rssi);
			if (ret < 0)
				dbG("Qcsapi qcsapi_wifi_get_rssi_in_dbm_per_association %s error, return: %d\n", ifname, ret);

			ret = qcsapi_wifi_get_tx_phy_rate_per_association(ifname, i, &tx_phy_rate);
			if (ret < 0)
				dbG("Qcsapi qcsapi_wifi_get_tx_phy_rate_per_association %s error, return: %d\n", ifname, ret);

			ret = qcsapi_wifi_get_rx_phy_rate_per_association(ifname, i, &rx_phy_rate);
			if (ret < 0)
				dbG("Qcsapi qcsapi_wifi_get_rx_phy_rate_per_association %s error, return: %d\n", ifname, ret);

			ret = qcsapi_wifi_get_time_associated_per_association(ifname, i, &time_associated);
			if (ret < 0)
				dbG("Qcsapi qcsapi_wifi_get_time_associated_per_association %s error, return: %d\n", ifname, ret);

			hr = time_associated / 3600;
			min = (time_associated % 3600) / 60;
			sec = time_associated - hr * 3600 - min * 60;

			retval += websWrite(wp, "%s ", wl_ether_etoa((struct ether_addr *) &sta_address));

			retval += websWrite(wp, "%-11s%-11s", "Yes", !nvram_match("wl1_auth_mode_x", "open") ? "Yes" : "No");

			retval += websWrite(wp, "%4ddBm ", rssi);

			retval += websWrite(wp, "%6dM ", tx_phy_rate);

			retval += websWrite(wp, "%6dM ", rx_phy_rate);

			retval += websWrite(wp, "%02d:%02d:%02d", hr, min, sec);

			retval += websWrite(wp, "\n");
		}
	}

	return retval;
}

int
ej_wl_status_qtn(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret = 0;
	int i;
	char prefix[] = "wlXXXXXXXXXX_", tmp[128];

	if (!rpc_qtn_ready())
		return ret;

	ret += websWrite(wp, "\n");
	ret += websWrite(wp, "Stations List                           \n");
	ret += websWrite(wp, "----------------------------------------\n");

	ret += websWrite(wp, "%-18s%-11s%-11s%-8s%-8s%-8s%-12s\n",
			     "MAC", "Associated", "Authorized", "   RSSI", "Tx rate", "Rx rate", "Connect Time");

	ret += ej_wl_status_qtn_core(eid, wp, argc, argv, unit);

        if (nvram_get_int("sw_mode") == SW_MODE_REPEATER && nvram_get_int("wlc_band"))
                return ret;

	for (i = 1; i < 4; i++) {
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
			ret += ej_wl_status_qtn_core(eid, wp, argc, argv, unit);
	}

	return ret;
}


int
ej_wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	int val = 0, ret = 0;
	qcsapi_unsigned_int channel;

	_dprintf("Raymond: [%s][%d], unit=[%d]\n", __func__, __LINE__, unit);
	ret = rpc_qcsapi_get_channel(&channel, unit);
	if (ret < 0)
		dbG("rpc_qcsapi_get_channel error, return: %d\n", ret);

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	if (psta_exist_except(unit))
	{
		ret += websWrite(wp, "%s radio is disabled\n",
			(channel > 0) ?
			((channel > 14 /* CH_MAX_2G_CHANNEL */) ? "5 GHz" : "2.4 GHz") :
			(nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz"));
		return ret;
	}
#endif
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
	if (unit && rpc_qtn_ready())
	{
		ret = qcsapi_wifi_rfstatus((qcsapi_unsigned_int *) &val);
		if (ret < 0)
			dbG("qcsapi_wifi_rfstatus error, return: %d\n", ret);
		else
			val = !val;
	}

	if (unit && !rpc_qtn_ready())
	{
		ret += websWrite(wp, "5 GHz radio is not ready\n");
		return ret;
	}
	else
	if (val)
	{
		ret += websWrite(wp, "%s radio is disabled\n",
			(channel > 0) ?
			((channel > 14 /* CH_MAX_2G_CHANNEL */) ? "5 GHz" : "2.4 GHz") :
			(nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz"));
		return ret;
	}

	if (nvram_match(strcat_r(prefix, "mode", tmp), "wds")) {
		// dump static info only for wds mode:
		// ret += websWrite(wp, "SSID: %s\n", nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
		ret += websWrite(wp, "Channel: %d\n", channel);
	}
	else {
		if (unit)
			ret += wl_status_5g(eid, wp, argc, argv);
		else
			ret += wl_status_2g(eid, wp, argc, argv);
	}

	if (nvram_match(strcat_r(prefix, "mode", tmp), "ap"))
	{
		if (nvram_match(strcat_r(prefix, "lazywds", tmp), "1") ||
			nvram_invmatch(strcat_r(prefix, "wds", tmp), ""))
			ret += websWrite(wp, "Mode	: Hybrid\n");
		else	ret += websWrite(wp, "Mode	: AP Only\n");
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wds"))
	{
		ret += websWrite(wp, "Mode	: WDS Only\n");
		return ret;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "sta"))
	{
		ret += websWrite(wp, "Mode	: Stations\n");
		ret += ej_wl_sta_status(eid, wp, name);
		return ret;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wet"))
	{
//		ret += websWrite(wp, "Mode	: Ethernet Bridge\n");
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER)
			&& (nvram_get_int("wlc_band") == unit))
			sprintf(prefix, "wl%d.%d_", unit, 1);
#endif
		ret += websWrite(wp, "Mode	: Repeater [ SSID local: \"%s\" ]\n", nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
//		ret += ej_wl_sta_status(eid, wp, name);
//		return ret;
	}
#ifdef RTCONFIG_PROXYSTA
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "psta"))
	{
		if ((nvram_get_int("sw_mode") == SW_MODE_AP) &&
			(nvram_get_int("wlc_psta") == 1) &&
			(nvram_get_int("wlc_band") == unit))
		ret += websWrite(wp, "Mode	: Media Bridge\n");
	}
#endif

	if (rpc_qtn_ready())
		ret += ej_wl_status_qtn(eid, wp, argc, argv, unit);

	return ret;
}

int
ej_wl_status_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	int ii = 0;
	char nv_param[NVRAM_MAX_PARAM_LEN];
	char *temp;

	_dprintf("Raymond: [%s][%d]\n", __func__, __LINE__);
	for (ii = 0; ii < DEV_NUMIFS; ii++) {
		sprintf(nv_param, "wl%d_unit", ii);
		_dprintf("Raymond: [%s][%d], nv_param=[%s]\n", __func__, __LINE__, nv_param);
		temp = nvram_get(nv_param);

		if (temp && strlen(temp) > 0)
		{
			retval += ej_wl_status(eid, wp, argc, argv, ii);
			retval += websWrite(wp, "\n");
		}
	}

	return retval;
}

/* httpd/sysdeps/web-broadcom-wl6.c */
int
wps_is_oob()
{
	char tmp[16];

	snprintf(tmp, sizeof(tmp), "lan_wps_oob");

	/*
	 * OOB: enabled
	 * Configured: disabled
	 */
	if (nvram_match(tmp, "disabled"))
		return 0;

	return 1;
}

char *
getWscStatusStr_qtn(int unit)
{
	int ret, state = -1;
	char wps_state[32], state_str[32];
	char ifname[WIFINAME_MAX_LEN] = {0};

	if (!rpc_qtn_ready())
		return "5 GHz radio is not ready";

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	ret = rpc_qcsapi_wps_get_state(WIFINAME, wps_state, sizeof(wps_state));
	if (ret < 0)
		dbG("rpc_qcsapi_wps_get_state %s error, return: %d\n", WIFINAME, ret);
	else if (sscanf(wps_state, "%d %s", &state, state_str) != 2)
		dbG("prase wps state error!\n");

	switch (state) {
	case 0: /* WPS_INITIAL */
		return "initialization";
		break;
	case 1: /* WPS_START */
		return "Start WPS Process";
		break;
	case 2: /* WPS_SUCCESS */
		return "Success";
		break;
	case 3: /* WPS_ERROR */
		return "Fail due to WPS message exchange error!";
		break;
	case 4: /* WPS_TIMEOUT */
		return "Fail due to WPS time out!";
		break;
	case 5: /* WPS_OVERLAP */
		return "Fail due to PBC session overlap!";
		break;
	default:
		if (nvram_match("wps_enable", "1"))
			return "Idle";
		else
			return "Not used";
		break;
	}
}

char *
get_WPSConfiguredStr_qtn(int unit)
{
	int ret;
	char wps_configured_state[32];
	char ifname[WIFINAME_MAX_LEN] = {0};

	if (!rpc_qtn_ready())
		return "";

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	wps_configured_state[0] = 0;
	ret = rpc_qcsapi_wps_get_configured_state(ifname, wps_configured_state, sizeof(wps_configured_state));
	if (ret < 0)
		dbG("rpc_qcsapi_wps_get_configured_state %s error, return: %d\n", ifname, ret);

	if (!strcmp(wps_configured_state, "configured"))
		return "Yes";
	else
		return "No";
}

char *
getWPSAuthMode_qtn(int unit)
{
	string_16 current_beacon_type;
	int ret;
	char ifname[WIFINAME_MAX_LEN] = {0};

	if (!rpc_qtn_ready())
		return "";

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	memset(&current_beacon_type, 0, sizeof(current_beacon_type));
	ret = rpc_qcsapi_get_beacon_type(ifname, (char *) &current_beacon_type);
	if (ret < 0)
		dbG("rpc_qcsapi_get_beacon_type %s error, return: %d\n", ifname, ret);

	if (!strcmp(current_beacon_type, "Basic"))
		return "Open System";
	else if (!strcmp(current_beacon_type, "WPA"))
		return "WPA-Personal";
	else if (!strcmp(current_beacon_type, "11i"))
		return "WPA2-Personal";
	else if (!strcmp(current_beacon_type, "WPAand11i"))
		return "WPA-Auto-Personal";
	else
		return "Open System";
}

char *
getWPSEncrypType_qtn(int unit)
{
	string_32 encryption_mode;
	int ret;
	char ifname[WIFINAME_MAX_LEN] = {0};

	if (!rpc_qtn_ready())
		return "";

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	memset(&encryption_mode, 0, sizeof(encryption_mode));
	ret = rpc_qcsapi_get_WPA_encryption_modes(ifname, (char *) &encryption_mode);
	if (ret < 0)
		dbG("rpc_qcsapi_get_WPA_encryption_modes %s error, return: %d\n", ifname, ret);

	if (!strcmp(encryption_mode, "TKIPEncryption"))
		return "TKIP";
	else if (!strcmp(encryption_mode, "AESEncryption"))
		return "AES";
	else if (!strcmp(encryption_mode, "TKIPandAESEncryption"))
		return "TKIP+AES";
	else
		return "AES";
}

static int psta_auth = 0;

int
ej_wl_auth_psta_qtn(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char ssid[33], auth[33];
	int psta_debug = 0;
	int retval = 0, psta = 0;
	char wlc_ssid[33];
	char ifname[WIFINAME_MAX_LEN] = {0};

	strcpy(wlc_ssid, nvram_safe_get("wlc_ssid"));
	memset(ssid, 0, sizeof(ssid));
	memset(auth, 0, sizeof(auth));

	if (nvram_match("psta_debug", "1"))
		psta_debug = 1;

	if (!rpc_qtn_ready()){
		goto ERROR;
	}

	snprintf(ifname, sizeof(ifname), "%s", wl_vifname_qtn(unit, 0));

	qcsapi_wifi_get_SSID(ifname, ssid);

	if(!strcmp(ssid, wlc_ssid))
	{
		if (psta_debug) dbg("connected: ssid %s\n", ssid);
		psta = 1;
		psta_auth = 0;
	}
	else
	{
		qcsapi_SSID_get_authentication_mode(ifname, wlc_ssid, auth);
		if (psta_debug) dbg("get_auth : ssid=%s, auth=%s\n", wlc_ssid, auth);

		if (!strcmp(auth, "PSKAuthentication") || !strcmp(auth, "EAPAuthentication"))
		{
			if (psta_debug) dbg("authorization failed\n");
			psta = 2;
			psta_auth = 1;
		}
		else
		{
			if (psta_debug) dbg("disconnected\n");
			psta = 0;
			psta_auth = 0;
		}
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

ERROR:
	return retval;
}

/* httpd/sysdeps/web-broadcom-wl6.c */
// no WME in WL500gP V2
// MAC/associated/authorized
int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit = 0;
	struct maclist *auth = NULL;
	int mac_list_size;
	int firstRow = 1;
	char word[256], *next;
	int ret = 0;

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);
	//wme = malloc(mac_list_size);

	//if (!auth || !wme)
	if (!auth)
		goto exit;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		if (rpc_qtn_ready()) {
			if (firstRow == 1){
				firstRow = 0;
			}else{
				ret += websWrite(wp, ", ");
			}
			ret += ej_wl_sta_list_qtn(eid, wp, argc, argv, unit);
		}
		unit++;
	}

exit:
	if (auth) free(auth);
	//if (wme) free(wme);

	return ret;
}

int
ej_wl_sta_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_sta_list_qtn(eid, wp, argc, argv, 0);
}

int
ej_wl_sta_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_sta_list_qtn(eid, wp, argc, argv, 1);
}

int ej_get_wlstainfo_list(int eid, webs_t wp, int argc, char_t **argv)
{
	// TODO
	return 0;
}

#ifdef RTCONFIG_PROXYSTA

/* httpd/sysdeps/web-broadcom-wl6.c */
static int
wl_autho(char *name, struct ether_addr *ea)
{
	char buf[sizeof(sta_info_t)];

	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LEN);

	if (!wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf))) {
		sta_info_t *sta = (sta_info_t *)buf;
		uint32 f = sta->flags;

		if (f & WL_STA_AUTHO)
			return 1;
	}

	return 0;
}

int
ej_wl_auth_psta(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	struct maclist *mac_list = NULL;
	int mac_list_size, i, unit;
	int retval = 0, psta = 0;
	struct ether_addr bssid;
	unsigned char bssid_null[6] = {0x0,0x0,0x0,0x0,0x0,0x0};
	int psta_debug = 0;

	if (nvram_match("psta_debug", "1"))
		psta_debug = 1;

	unit = nvram_get_int("wlc_band");
	return ej_wl_auth_psta_qtn(eid, wp, argc, argv, unit);
}
#endif	/* RTCONFIG_PROXYSTA */

static int ej_wl_rate(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	char tmp[256], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char word[256], *next;
	int unit_max = 0, unit_cur = -1;
	int rate = 0;
	char rate_buf[32];
	struct ether_addr bssid;
	unsigned char bssid_null[6] = {0x0,0x0,0x0,0x0,0x0,0x0};
	int sta_rate;
	int from_app = 0;
	char ifname[10] = "wifi0_0";
	uint32_t count = 0, speed;

	from_app = check_user_agent(user_agent);

	if(unit == 0)	snprintf(ifname, sizeof(ifname), "wifi2_0");
	else if(unit == 1)	snprintf(ifname, sizeof(ifname), "wifi0_0");
	else snprintf(ifname, sizeof(ifname), "wifi0_0");

	sprintf(rate_buf, "0 Mbps");

	if (!rpc_qtn_ready()) {
		goto ERROR;
	}
	// if ssid associated, check associations
	if (qcsapi_wifi_get_link_quality(ifname, count, &speed) < 0) {
		// dbg("fail to get link status index %d\n", (int)count);
	} else {
		speed = speed ;  /* 4 antenna? */
		if ((int)speed < 1) {
			sprintf(rate_buf, "auto");
		} else {
			sprintf(rate_buf, "%d Mbps", (int)speed);
		}
	}

	retval += websWrite(wp, "%s", rate_buf);
	return retval;

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
	return ej_wl_rate(eid, wp, argc, argv, 0);
}

int
ej_wl_rate_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_rate(eid, wp, argc, argv, 1);
}

