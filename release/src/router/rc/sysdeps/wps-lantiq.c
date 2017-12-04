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
#include <fapi_wlan_private.h>
#include <fapi_wlan.h>
#include <help_objlist.h>
#include <wlan_config_api.h>


//#define NOT_SHELL_FAPI

int 
start_wps_method(void)
{
	if(getpid()!=1) {
		notify_rc("start_wps_method");
		return 0;
	}

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

#ifdef RTCONFIG_WPS_ENROLLEE
	if (nvram_match("wps_enrollee", "1")) {
		stop_wsc_enrollee();
	}
	else
#endif
		stop_wsc();

	return 0;
}

int is_wps_stopped(void){
	int i, ret = 1;
	char status[16], tmp[128], prefix[] = "wlXXXXXXXXXX_", word[256], *next, ifnames[128];
	int wps_band = nvram_get_int("wps_band_x");

	i = 0;
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach(word, ifnames, next){
		if(i >= MAX_NR_WL_IF)
			break;
		if(wps_band != i){
			++i;
			continue;
		}
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if(!__need_to_start_wps_band(prefix) || nvram_get_int(strcat_r(prefix, "radio", tmp)) != 1){
			ret = 0;
			++i;
			continue;
		}

#ifdef RTCONFIG_WPS_ENROLLEE
		if(nvram_match("wps_enrollee", "1"))
			strcpy(status, getWscStatus_enrollee(i));
		else
#endif
			getWscStatusStr(wl_wave_unit(i), status, sizeof(status));

		dbG("band %d(wlan%d) wps status: %s\n", i, wl_wave_unit(i), status);
		if(!strcmp(status, "Success")
#ifdef RTCONFIG_WPS_ENROLLEE
				|| !strcmp(status, "COMPLETED")
#endif
				){
			dbG("\nWPS %s\n", status);
#if defined(RTCONFIG_WPS_LED) || defined(BLUECAVE)
			nvram_set("wps_success", "1");
#endif
#if (defined(RTCONFIG_WPS_ENROLLEE) && defined(RTCONFIG_WIFI_CLONE))
			if(nvram_match("wps_enrollee", "1")){
				nvram_set("wps_e_success", "1");
#if (defined(PLN12) || defined(PLAC56))
				set_wifiled(4);
#endif
			}
#endif
			ret = 1;
		}
		else if(!strcmp(status, "Failed")
				|| !strcmp(status, "Timeout")
#ifdef RTCONFIG_WPS_ENROLLEE
				|| !strcmp(status, "INACTIVE")
#endif
				){
			dbG("\nWPS %s\n", status);
			ret = 1;
		}
		else
			ret = 0;

		if(ret)
			break;

		++i;
	}

	return ret;
}

int __need_to_start_wps_band(char *prefix){
	char *p, tmp[128];

	if(!prefix || *prefix == '\0')
		return 0;

	p = nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp));
	if((!strcmp(p, "open") && !nvram_match(strcat_r(prefix, "wep_x", tmp), "0"))
			|| !strcmp(p, "shared") || !strcmp(p, "psk") || !strcmp(p, "wpa")
			|| !strcmp(p, "wpa2") || !strcmp(p, "wpawpa2")
			|| !strcmp(p, "radius")
			|| nvram_match(strcat_r(prefix, "radio", tmp), "0")
			|| !((sw_mode() == SW_MODE_ROUTER) || (sw_mode() == SW_MODE_AP))
			)
		return 0;

	return 1;
}

int need_to_start_wps_band(int wps_band){
	int ret = 1;
	char prefix[] = "wlXXXXXXXXXX_";

	switch(wps_band){
		case 0:		/* fall through */
		case 1:
			snprintf(prefix, sizeof(prefix), "wl%d_", wps_band);
			ret = __need_to_start_wps_band(prefix);
			break;
		default:
			ret = 0;
	}

	return ret;
}

int set_wps_enable(int unit){
	int retVal = 0;
	bool orig_enable = 0;
	int enable = nvram_get_int("wps_enable");
	int band = nvram_get_int("wps_band_x");

	_dprintf("%s(%d): enable=%d, band=%d.\n", __func__, unit, enable, band);

	if(unit != band){
		_dprintf("%s(%d): skip.\n", __func__, unit);
		return 0;
	}

	if(enable != 0 && enable != 1)
		enable = 0;

	retVal = wlan_getWpsEnable(wl_wave_unit(unit), &orig_enable);
	_dprintf("%s(%d): orig_enable=%d.\n", __func__, unit, orig_enable);
	if(orig_enable == enable){
		_dprintf("%s(%d): enable state was not changed.\n", __func__, unit);
		return retVal;
	}

	retVal = wlan_setWpsEnable(wl_wave_unit(unit), enable);
	_dprintf("%s(%d): done. %d.\n", __func__, unit, retVal);

	return retVal;
}

#define WPS_CONFIG_FILE "/tmp/wps_config.xml"

int write_wps_config(int configured){
	FILE *fp = fopen(WPS_CONFIG_FILE, "w+");

	if(!fp){
		_dprintf("%s: cannot open the WPS config file.\n", __func__);
		return -1;
	}

	fprintf(fp, "Object_0=Device.WiFi.Radio.X_LANTIQ_COM_Vendor.WPS\n");
	fprintf(fp, "ConfigState_0=%s\n", (configured == 1)?"Configured":"Unconfigured");
	if(configured == 0)
		fprintf(fp, "WPSAction_0=ResetWPS\n");

	fclose(fp);

	return 0;
}

int set_wps_config(int unit, int configured){
	int retVal;
#ifdef NOT_SHELL_FAPI
	char buf[256];
	ObjList *dbObjPtr = NULL;
#else
	char cmd[256];
	FILE *fp;
	char *pt1, *pt2, *pt3;
	int orig_config = 0;
#endif

	_dprintf("%s(%d): configured=%d.\n", __func__, unit, configured);

#ifdef NOT_SHELL_FAPI
	retVal = wlan_getWpsConfigurationState(wl_wave_unit(unit), buf);
	_dprintf("%s(%d): buf=%s.\n", __func__, unit, buf);
	if((!strcmp(buf, "Configured") && configured == 1)
			|| (!strcmp(buf, "Unconfigured") && configured == 0)
			){
		_dprintf("%s(%d): config state was not changed.\n", __func__, unit);
		return retVal;
	}

	dbObjPtr = HELP_CREATE_OBJ(SOPT_OBJVALUE);
#else
	snprintf(cmd, sizeof(cmd), "/usr/sbin/fapi_wlan_cli getWpsConfigurationState -i%d", wl_wave_unit(unit));
	fp = popen(cmd, "r");
	if(fp){
		memset(cmd, 0, sizeof(cmd));
		while(fgets(cmd, sizeof(cmd), fp)){
			pt1 = strstr(cmd, "ConfigState= '");
			if(pt1){
				pt2 = pt1+strlen("ConfigState= '");
				pt3 = strchr(pt2, '\'');
				pt3[0] = '\0';

				if(!strcmp(pt2, "Configured"))
					orig_config = 1;
				else
					orig_config = 0;

				if(orig_config == configured){
					_dprintf("%s(%d): config state was not changed.\n", __func__, unit);
					pclose(fp);
					return 0;
				}
			}
		}
		pclose(fp);
	}
#endif

	write_wps_config(configured);

#ifdef NOT_SHELL_FAPI
	if((retVal = wlanLoadFromDB(WPS_CONFIG_FILE, "", dbObjPtr)) != UGW_SUCCESS){
		_dprintf("%s(%d): wlanLoadFromDB return with error\n", __func__, unit);
		HELP_DELETE_OBJ(dbObjPtr, SOPT_OBJVALUE, FREE_OBJLIST);
		return retVal;
	}

	if((retVal = fapi_wlan_wps_set_native(wl_wave_unit(unit), dbObjPtr, 0)) != UGW_SUCCESS){
		_dprintf("%s(%d): fapi_wlan_wps_set_native return with error\n", __func__, unit);
		HELP_DELETE_OBJ(dbObjPtr, SOPT_OBJVALUE, FREE_OBJLIST);
		return retVal;
	}

	HELP_DELETE_OBJ(dbObjPtr, SOPT_OBJVALUE, FREE_OBJLIST);
#else
	snprintf(cmd, sizeof(cmd), "/usr/sbin/fapi_wlan_cli setWps -i%d -f%s", wl_wave_unit(unit), WPS_CONFIG_FILE);
	system(cmd);
#endif

	_dprintf("%s(%d): done.\n", __func__, unit);

	return retVal;
}

int set_all_wps_config(int configured){
	int i;
	char word[256], *next;
	char ifnames[128];

	if(nvram_match("lan_ipaddr", ""))
		return -1;

	i = 0;
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach(word, ifnames, next){
		if(i >= MAX_NR_WL_IF)
			break;

		_dprintf("%s: %s %s...\n", __func__, (configured == 1)?"Configure":"Unconfigure", word);
		set_wps_config(i, configured);

		++i;
	}

	return 0;
}

void wps_oob(void){
	if(nvram_match("lan_ipaddr", ""))
		return;

	nvram_set("wps_reset", "1");
	restart_wireless();

	nvram_commit();
}

void start_wsc(void){
	char *wps_sta_pin = nvram_safe_get("wps_sta_pin");
	int wps_band = nvram_get_int("wps_band_x");
#ifndef NOT_SHELL_FAPI
	char cmd[256];
#endif

	if(nvram_match("lan_ipaddr", ""))
		return;

	if(!need_to_start_wps_band(wps_band))
		return;

	dbg("%s: start wsc(%d)\n", __func__, wps_band);

	_dprintf("%s: cancel wlan%d's WPS...\n", __func__, wl_wave_unit(wps_band));
#ifdef NOT_SHELL_FAPI
	wlan_cancelWps(wl_wave_unit(wps_band));
#else
	snprintf(cmd, sizeof(cmd), "/usr/sbin/fapi_wlan_cli cancelWps -i%d", wl_wave_unit(wps_band));
	system(cmd);
#endif

	if(strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000")
			&& wl_wpsPincheck(wps_sta_pin) == 0){
		dbg("WPS: PIN\n");	// PIN method
#ifdef NOT_SHELL_FAPI
		wlan_setWpsEnrolleePin(wl_wave_unit(wps_band), wps_sta_pin);
#else
		snprintf(cmd, sizeof(cmd), "/usr/sbin/fapi_wlan_cli setWpsEnrolleePin -i%d -p%s", wl_wave_unit(wps_band), wps_sta_pin);
		system(cmd);
#endif
	}
	else{
		dbg("WPS: PBC\n");	// PBC method
#ifdef NOT_SHELL_FAPI
		wlan_setWpsPbcTrigger(wl_wave_unit(wps_band));
#else
		snprintf(cmd, sizeof(cmd), "/usr/sbin/fapi_wlan_cli setWpsPbcTrigger -i%d", wl_wave_unit(wps_band));
		system(cmd);
#endif
	}
}

void stop_wsc(){
	int wps_band = nvram_get_int("wps_band_x");
#ifndef NOT_SHELL_FAPI
	char cmd[256];
#endif

	if(!need_to_start_wps_band(wps_band))
		return;

	dbg("%s: stop wsc(%d)\n", __func__, wps_band);

	_dprintf("%s: cancel wlan%d's WPS...\n", __func__, wl_wave_unit(wps_band));
#ifdef NOT_SHELL_FAPI
	wlan_cancelWps(wl_wave_unit(wps_band));
#else
	snprintf(cmd, sizeof(cmd), "/usr/sbin/fapi_wlan_cli cancelWps -i%d", wl_wave_unit(wps_band));
	system(cmd);
#endif
}

int getWscStatusStr(int unit, char *buf, int buf_size){
#ifdef NOT_SHELL_FAPI
	int retVal = wlan_getWpsStatus(wl_wave_unit(unit), buf);
#else
	FILE *fp;
	char cmd[256];
	char *pt1, *pt2, *pt3;
	int retVal = 0;

	snprintf(cmd, sizeof(cmd), "/usr/sbin/fapi_wlan_cli getWpsStatus -i%d", wl_wave_unit(unit));
	fp = popen(cmd, "r");
	if(fp){
		memset(cmd, 0, sizeof(cmd));
		while(fgets(cmd, sizeof(cmd), fp)){
			pt1 = strstr(cmd, "wlan_getWpsStatus: Status= '");
			if(pt1){
				pt2 = pt1+strlen("wlan_getWpsStatus: Status= '");
				pt3 = strchr(pt2, '\'');
				pt3[0] = '\0';

				snprintf(buf, buf_size, "%s", pt2);
				break;
			}
		}
		pclose(fp);
	}
#endif

	return retVal;
}
