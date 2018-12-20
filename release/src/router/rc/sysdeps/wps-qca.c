/*
 * Miscellaneous services
 *
 * Copyright (C) 2009, Broadcom Corporation. All Rights Reserved.
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
 * $Id: services.c,v 1.100 2010/03/04 09:39:18 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <shared.h>

/*
 * input variables:
 *	nvram: wps_sta_pin:
 *
 */

int 
start_wps_method(void)
{
	if(getpid()!=1) {
		notify_rc("start_wps_method");
		return 0;
	}
#ifdef RTCONFIG_CONCURRENTREPEATER
	int sw_mode = sw_mode();

	if (sw_mode == SW_MODE_REPEATER) {// Repeater mode

		start_wps_cli();
		return 0;
	}
#endif

#ifdef RTCONFIG_WPS_ENROLLEE
	if (nvram_match("wps_enrollee", "1"))
		start_wsc_enrollee();
	else
#endif
		start_wsc();

	return 0;
}

int 
stop_wps_method(void)
{
	if(getpid()!=1) {
		notify_rc("stop_wps_method");
		return 0;
	}

#ifdef RTCONFIG_CONCURRENTREPEATER
	int sw_mode = sw_mode;

	if (sw_mode == SW_MODE_REPEATER) {// Repeater mode

		stop_wps_cli();
		return 0;
	}
#endif
#ifdef RTCONFIG_WPS_ENROLLEE
	if (nvram_match("wps_enrollee", "1")) {
		stop_wsc_enrollee();
	}
	else
#endif
		stop_wsc();

	return 0;
}

extern int g_isEnrollee[MAX_NR_WL_IF];

int is_wps_stopped(void)
{
	int i, ret = 1;
	char status[16], tmp[128], prefix[] = "wlXXXXXXXXXX_", word[256], *next, ifnames[128];
	int wps_band = nvram_get_int("wps_band_x"), multiband = get_wps_multiband();
	char tmpbuf[512];

	i = 0;
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach (word, ifnames, next) {
		if (i >= MAX_NR_WL_IF)
			break;
		if (!multiband && wps_band != i) {
			++i;
			continue;
		}
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if (!__need_to_start_wps_band(prefix) || nvram_match(strcat_r(prefix, "radio", tmp), "0")) {
			ret = 0;
			++i;
			continue;
		}

#ifdef RTCONFIG_WIFI_SON
		if(sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
			if(i==0 || i==2)
			{	
				++i;
				continue;
			}
		}
#ifdef RTCONFIG_AMAS
		else if(sw_mode() == SW_MODE_ROUTER && !nvram_match("wifison_ready", "1") && (
			(nvram_match("x_Setting", "1")) ||
			(nvram_match("x_Setting", "0") && nvram_match("wps_enrollee", "1"))
		)) {	/* only check 2G in AiMesh mode */
			if (i != 0) {
				++i;
				continue;
			}
		}
#endif	/* RTCONFIG_AMAS */
#endif

#ifdef RTCONFIG_WPS_ENROLLEE
		if (nvram_match("wps_enrollee", "1"))
			strcpy(status, getWscStatus_enrollee(i, tmpbuf, sizeof(tmpbuf)));
		else
#endif
			strcpy(status, getWscStatus(i, tmpbuf, sizeof(tmpbuf)));

		//dbG("band %d wps status: %s\n", i, status);
		if (!strcmp(status, "Success") 
#ifdef RTCONFIG_WPS_ENROLLEE
				|| !strcmp(status, "COMPLETED")
#endif
		) {
			dbG("\nWPS %s\n", status);
#ifdef RTCONFIG_WPS_LED
			nvram_set("wps_success", "1");
#endif
#if defined(RTCONFIG_LP5523)
//			lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_WPS_SUCCESS);
//			usleep(3990 * 1000); // flashing 4 times is about 3990 ms
#endif
#if (defined(RTCONFIG_WPS_ENROLLEE))
			if (nvram_match("wps_enrollee", "1")) {
				nvram_set("wps_e_success", "1");
#if (defined(PLN12) || defined(PLAC56))
				set_wifiled(4);
#endif
#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_WIFI_SON) && defined(RTCONFIG_AMAS)
				if(!nvram_match("wifison_ready", "1"))
#endif
				amas_save_wifi_para();
#endif	/* RTCONFIG_AMAS */
#if defined(RTCONFIG_WIFI_CLONE)
				wifi_clone(i);
#endif
			}
#endif
			ret = 1;
		}
		else if (!strcmp(status, "Failed") 
#ifdef RTCONFIG_WPS_ENROLLEE
				|| !strcmp(status, "INACTIVE")
#endif
		) {
			dbG("\nWPS %s\n", status);
			ret = 1;
		}
		else
			ret = 0;

		if (ret)
			break;

		++i;
	}

	return ret;
}

int is_wps_success(void)
{
#if (defined(RTCONFIG_WPS_ENROLLEE))
	if (nvram_match("wps_enrollee", "1"))
		return nvram_get_int("wps_e_success");
#endif
	return nvram_get_int("wps_success");
}

/*
 * save new AP configuration from WPS external registrar
 */
int get_wps_er_main(int argc, char *argv[])
{
	int i;
	char word[32], *next, ifnames[64];
	char wps_ifname[32], _ssid[33], _auth[8], _encr[8], _key[65];

	if (nvram_invmatch("w_Setting", "0"))
		return 0;

	sleep(2);

	memset(wps_ifname, 0x0, sizeof(wps_ifname));
	memset(_ssid, 0x0, sizeof(_ssid));
	memset(_auth, 0x0, sizeof(_auth));
	memset(_encr, 0x0, sizeof(_encr));
	memset(_key, 0x0, sizeof(_key));
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));

	while (nvram_match("w_Setting", "0")) {
		foreach (word, ifnames, next) {
			char buf[1024];
			FILE *fp;
			int len;
			char *pt1,*pt2 = NULL;

			SKIP_ABSENT_FAKE_IFACE(word);
			snprintf(buf, sizeof(buf), "hostapd_cli -i%s get_config", word);
			fp = popen(buf, "r");
			if (fp) {
				memset(buf, 0, sizeof(buf));
				len = fread(buf, 1, sizeof(buf), fp);
				pclose(fp);

				if (len > 1) {
					buf[len-1] = '\0';
					pt1 = strstr(buf, "wps_state=configured");
					if (pt1) {
						strcpy(wps_ifname, word);
						//BSSID
						if ((pt1 = strstr(buf, "bssid=")))
							pt2 = pt1 + strlen("bssid=");
						//SSID
						if ((pt1 = strstr(pt2, "ssid="))) {
							pt1 += strlen("ssid=");
							if ((pt2 = strstr(pt1, "\n")))
								*pt2 = '\0';
							//dbG("ssid=%s\n", pt1);
							nvram_set("wl0_ssid", pt1);
							nvram_set("wl1_ssid", pt1);
							strcpy(_ssid, pt1);
							pt2++;
						}
						//Encryp
						if (strstr(pt2, "wpa=2")) {
							nvram_set("wl0_auth_mode_x", "pskpsk2");
							nvram_set("wl0_crypto", "tkip+aes");
							nvram_set("wl1_auth_mode_x", "pskpsk2");
							nvram_set("wl1_crypto", "tkip+aes");
							strcpy(_auth, "WPA2PSK");
							if (strstr(pt2, "group_cipher=TKIP"))
								strcpy(_encr, "TKIP");
							else
								strcpy(_encr, "CCMP");
						}
						else if (strstr(pt2, "wpa=3")) {
							nvram_set("wl0_auth_mode_x", "pskpsk2");
							nvram_set("wl0_crypto", "tkip+aes");
							nvram_set("wl1_auth_mode_x", "pskpsk2");
							nvram_set("wl1_crypto", "tkip+aes");
							strcpy(_auth, "WPAPSK");
							if (strstr(pt2, "group_cipher=TKIP"))
								strcpy(_encr, "TKIP");
							else
								strcpy(_encr, "CCMP");
						}
						else {
							nvram_set("wl0_auth_mode_x", "open");
							nvram_set("wl0_crypto", "tkip+aes");
							nvram_set("wl1_auth_mode_x", "open");
							nvram_set("wl1_crypto", "tkip+aes");
							strcpy(_auth, "OPEN");
							strcpy(_encr, "NONE");
						}
						//WPAKey
						if ((pt1 = strstr(pt2, "passphrase="))) {
							pt1 += strlen("passphrase=");
							if ((pt2 = strstr(pt1, "\n")))
								*pt2 = '\0';
							//dbG("passphrase=%s\n", pt1);
							nvram_set("wl0_wpa_psk", pt1);
							nvram_set("wl1_wpa_psk", pt1);
							strcpy(_key, pt1);
							pt2++;
						}

						nvram_set("w_Setting", "1");
						nvram_commit();
					}
				}
			}
		}

		sleep(1);
	}

	i = 0;
	foreach (word, ifnames, next) {
		if (i >= MAX_NR_WL_IF)
			break;
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
		eval("hostapd_cli", "-i", word, "wps_ap_pin", "disable");
#endif
		if (!strcmp(word, wps_ifname)) {
			++i;
			continue;
		}
		eval("hostapd_cli", "-i", word, "wps_config", _ssid, _auth, _encr, _key);
		++i;
	}

	return 0;
}
