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
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/vfs.h>
#include <rc.h>
#include <shared.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <security_ipc.h>
#include <bcmutils.h>
#include <wlutils.h>
#include <wlscan.h>


#include "../../shared/sysdeps/realtek/realtek.h"


/*
 * input variables:
 * 	nvram: wps_band:
 * 	nvram: wps_action: WPS_UI_ACT_ADDENROLLEE/WPS_UI_ACT_CONFIGAe
 *	       (wps_method: (according to wps_sta_pin)
 *	nvram: wps_sta_pin:
 *	nvram: wps_version2:
 *	nvram: wps_autho_sta_mac:
 *
 * output variables:
 * 	wps_proc_status
 */

//#define PBC_STAT_FILE "/var/pbc_stat"
//#define PBC_STAT_LOCK_FILE "/var/pbc_btn_flock"
//#define WSCD_STAT_LOCK_FILE "/tmp/wscd_st_lock"
#define WSCD_CONFIG_STATUS "/tmp/wscd_status"

typedef enum WPS_MODE{WPS_MODE_AP=0,WPS_MODE_CLIENT}WPS_MODE;
static int write_pbc_state_file(int pid,char* state_file)
{
	FILE* fp;
	int fd;
	int res;
	char lock_file[64];
	rtklog("%s\n",__FUNCTION__);
	if(pid < 0 || state_file == NULL)
	{
		rtkerr("%s invalid parameter!!\n",__FUNCTION__);
		return -1;
	}
	sprintf(lock_file,"%s_lock",state_file);
	fd = open(lock_file, O_RDWR|O_CREAT|O_TRUNC);
	if(fd < 0)
	{
		rtkerr("%s failed to open lock file!\n",__FUNCTION__);
		return -1;
	}
	rtklog("%s before lock\n",__FUNCTION__);
	res  = rtk_file_lock(fd);
	if(res == -1)
	{
		rtkerr("%s failed to lock file!\n",__FUNCTION__);
		rtk_file_unlock(fd);
		return -1;
	}
	fp = fopen(state_file,"w");
	if(fp != NULL)
	{
		fprintf(fp,"%d",pid);
		fclose(fp);
	}
	rtk_file_unlock(fd);
	rtklog("%s unlock\n",__FUNCTION__);

}
int start_wps_pin_method(WPS_MODE mode,char* pin)
{
	int multi_band = -1;
	int wps_band = -1;
	if(pin == NULL)
	{
		return -1;
	}
	if(mode == WPS_MODE_CLIENT)
	{
		if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) { // media bridge mode.
#ifdef RTCONFIG_CONCURRENTREPEATER
			doSystem("iwpriv wl0 set_mib pin=%s",pin);
			doSystem("iwpriv wl1 set_mib pin=%s",pin);
#else
			wps_band = nvram_get_int("wlc_band");
			doSystem("iwpriv wl%d set_mib pin=%s",wps_band,pin);
#endif
		}
		else {
#ifdef RTCONFIG_CONCURRENTREPEATER
			doSystem("iwpriv wl0-vxd set_mib pin=%s",pin);
			doSystem("iwpriv wl1-vxd set_mib pin=%s",pin);
#else
			wps_band = nvram_get_int("wlc_band");
			doSystem("iwpriv wl%d-vxd set_mib pin=%s",wps_band,pin);
#endif
		}
	}
	else
	{
		multi_band = nvram_get_int("wps_multiband");
		wps_band = nvram_get_int("wps_band_x");
		if(multi_band == 1)
		{
			doSystem("iwpriv wl0 set_mib pin=%s",pin);
			doSystem("iwpriv wl1 set_mib pin=%s",pin);
		}
		else
		{
			doSystem("iwpriv wl%d set_mib pin=%s",wps_band,pin);
		}
	}
	return 0;
}
int start_wps_pbc_method(WPS_MODE mode)
{
	FILE* fp;
	int pid = -1;
	int multi_band;
	int wps_band = -1;
	char pid_file[64] = {0};
	char pbc_stat_file[64] = {0};
	if(mode == WPS_MODE_CLIENT)
	{
#ifdef RTCONFIG_AMAS
	/*	nvram wps_amas_enrollee indicate that the wps request send
	 *	from "obd". And it will set wps as enrollee.
	 */
		if(nvram_get_int("wps_amas_enrollee") == 1) {
		/*	AiMesh use 2G for connect with CAP in onboarding */
			fp = fopen("/var/run/wscd-wl0-vxd.pid","r");
			if(fp != NULL)
			{
				fscanf(fp,"%d",&pid);
				fclose(fp);
			}
			write_pbc_state_file(pid,"/var/pbc_state1");

		} else
#endif
#ifdef RTCONFIG_CONCURRENTREPEATER
		if (sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_express") != 0) { // Express mode
			if (nvram_get_int("wlc_express") == 1) { // 2.4G
				fp = fopen("/var/run/wscd-wl0.pid","r");
				if(fp != NULL)
				{
					fscanf(fp,"%d",&pid);
					fclose(fp);
				}
				write_pbc_state_file(pid,"/var/pbc_state1");
			}
			else { // 5G
				fp = fopen("/var/run/wscd-wl1.pid","r");
				if(fp != NULL)
				{
					fscanf(fp,"%d",&pid);
					fclose(fp);
				}
				write_pbc_state_file(pid,"/var/pbc_state2");
			}
		}
		else { // Repeater and media bridge
			if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) // media bridge
				fp = fopen("/var/run/wscd-wl0.pid","r");
			else
				fp = fopen("/var/run/wscd-wl0-vxd.pid","r");
			if(fp != NULL)
			{
				fscanf(fp,"%d",&pid);
				fclose(fp);
			}
			write_pbc_state_file(pid,"/var/pbc_state1");
			pid = -1;
			if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) // media bridge
				fp = fopen("/var/run/wscd-wl1.pid","r");
			else
				fp = fopen("/var/run/wscd-wl1-vxd.pid","r");

			if(fp != NULL)
			{
				fscanf(fp,"%d",&pid);
				fclose(fp);
			}
			sleep(1);
			write_pbc_state_file(pid,"/var/pbc_state2");
		}
#else
		{
			wps_band = nvram_get_int("wlc_band");

			if (sw_mdoe() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) // media bridge
				sprintf(pid_file,"/var/run/wscd-wl%d.pid",wps_band);
			else
				sprintf(pid_file,"/var/run/wscd-wl%d-vxd.pid",wps_band);

			sprintf(pbc_stat_file,"/var/pbc_state%d",wps_band+1);
			fp = fopen(pid_file,"r");
			if(fp != NULL)
			{
				fscanf(fp,"%d",&pid);
				fclose(fp);
				write_pbc_state_file(pid,pbc_stat_file);
			}
		}
#endif
#ifdef RPAC68U
		set_led(LED_BLINK_NORNAL, LED_BLINK_NORNAL);
#else
		nvram_set_int("led_status", LED_WPS_START);
#endif
	}
	else
	{
		multi_band = nvram_get_int("wps_multiband");
		wps_band = nvram_get_int("wps_band_x");
		if(multi_band == 0)
		{
			sprintf(pid_file,"/var/run/wscd-wl%d.pid",wps_band);
		}
		else
		{
			strcpy(pid_file,"/var/run/wscd-wl0-wl1.pid");
		}
		rtklog("%s pid_file:%s\n",__FUNCTION__,pid_file);
		fp = fopen(pid_file,"r");
		if(fp != NULL)
		{
			fscanf(fp,"%d",&pid);
			fclose(fp);
		}
		write_pbc_state_file(pid,"/var/pbc_state1");
#ifdef RPAC68U
		set_led(LED_BLINK_QUICK, LED_BLINK_QUICK);
#else
		nvram_set_int("led_status", LED_AP_WPS_START);
#endif
	}

	return 0;
}
int
start_wps_method(void)
{

	if(getpid()!=1) {
		notify_rc("start_wps_method");
                return 0;
	}

	rtklog("%s \n",__FUNCTION__);
	char *wps_sta_pin = nvram_safe_get("wps_sta_pin");
	int sw_mode = sw_mode();
	int wlc_express = 0;
	int multi_band = 0;
	int wps_band = 0;
	
	wlc_express = nvram_get_int("wlc_express");
#ifdef RTCONFIG_AMAS
	/*	nvram wps_amas_enrollee indicate that the wps request send
	 *	from "obd". And it will set wps as enrollee.
	 */
	if(nvram_get_int("wps_amas_enrollee") == 1) {
	/*	AiMesh use 2G for connect with CAP in onboarding */
		eval("iwpriv", "wl0-vxd", "set_mib","wsc_enable=1");
		eval("ifconfig", "wl0-vxd", "down");
		eval("ifconfig", "wl0-vxd", "up");

		start_wps_pbc_method(WPS_MODE_CLIENT);

	} else
#endif
	if(sw_mode == SW_MODE_REPEATER && wlc_express == 0)
	{
#ifdef RTCONFIG_CONCURRENTREPEATER
		eval("iwpriv", "wl0-vxd", "set_mib","wsc_enable=1");
		eval("ifconfig", "wl0-vxd", "down");
		eval("ifconfig", "wl0-vxd", "up");

		eval("iwpriv", "wl1-vxd", "set_mib","wsc_enable=1");
		eval("ifconfig", "wl1-vxd", "down");
		eval("ifconfig", "wl1-vxd", "up");
#else
		wps_band = nvram_get_int("wlc_band");
		if(wps_band == 0)
		{
			eval("iwpriv", "wl0-vxd", "set_mib","wsc_enable=1");
			eval("ifconfig", "wl0-vxd", "down");
			eval("ifconfig", "wl0-vxd", "up");
		}
		else if(wps_band == 1)
		{
			eval("iwpriv", "wl1-vxd", "set_mib","wsc_enable=1");
			eval("ifconfig", "wl1-vxd", "down");
			eval("ifconfig", "wl1-vxd", "up");
		}
#endif
		if(strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000") && wl_wpsPincheck(wps_sta_pin) == 0)
		{
			rtklog("%s PIN method\n",__FUNCTION__);
			start_wps_pin_method(WPS_MODE_CLIENT,wps_sta_pin);
		}
		else
		{
			rtklog("%s PBC method\n",__FUNCTION__);	
			start_wps_pbc_method(WPS_MODE_CLIENT);
		}
	}
#ifdef RTCONFIG_CONCURRENTREPEATER
	else if (sw_mode == SW_MODE_REPEATER && wlc_express != 0) {
		if (wlc_express == 1) {
			eval("iwpriv", "wl0", "set_mib","wsc_enable=1");
			eval("ifconfig", "wl0", "down");
			eval("ifconfig", "wl0", "up");
		}
		else {
			eval("iwpriv", "wl1", "set_mib","wsc_enable=1");
			eval("ifconfig", "wl1", "down");
			eval("ifconfig", "wl1", "up");
		}
		if(strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000") && wl_wpsPincheck(wps_sta_pin) == 0)
		{
			rtklog("%s PIN method\n",__FUNCTION__);
			start_wps_pin_method(WPS_MODE_CLIENT,wps_sta_pin);
		}
		else
		{
			rtklog("%s PBC method\n",__FUNCTION__);	
			start_wps_pbc_method(WPS_MODE_CLIENT);
		}
	}
#endif
	else if (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) {
#ifdef RTCONFIG_CONCURRENTREPEATER
		eval("iwpriv", "wl0", "set_mib","wsc_enable=1");
		eval("ifconfig", "wl0", "down");
		eval("ifconfig", "wl0", "up");

		eval("iwpriv", "wl1", "set_mib","wsc_enable=1");
		eval("ifconfig", "wl1", "down");
		eval("ifconfig", "wl1", "up");
#else
		wps_band = nvram_get_int("wlc_band");
		if(wps_band == 0)
		{
			eval("iwpriv", "wl0", "set_mib","wsc_enable=1");
			eval("ifconfig", "wl0", "down");
			eval("ifconfig", "wl0", "up");
		}
		else if(wps_band == 1)
		{
			eval("iwpriv", "wl1", "set_mib","wsc_enable=1");
			eval("ifconfig", "wl1", "down");
			eval("ifconfig", "wl1", "up");
		}
#endif
		if(strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000") && wl_wpsPincheck(wps_sta_pin) == 0)
		{
			rtklog("%s PIN method\n",__FUNCTION__);
			start_wps_pin_method(WPS_MODE_CLIENT,wps_sta_pin);
		}
		else
		{
			rtklog("%s PBC method\n",__FUNCTION__);	
			start_wps_pbc_method(WPS_MODE_CLIENT);
		}
	}
	else
	{
		if(strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000") && wl_wpsPincheck(wps_sta_pin) == 0)
		{
			rtklog("%s PIN method\n",__FUNCTION__);
			start_wps_pin_method(WPS_MODE_AP,wps_sta_pin);
		}
		else
		{
			rtklog("%s PBC method\n",__FUNCTION__);	
			start_wps_pbc_method(WPS_MODE_AP);
		}
	}
	sleep(3); //Wait for update tmp/wscd_status
	if (nvram_match("wps_restart", "1")) {
		sleep(2); // Because wscd restart, so need more wait time.
		nvram_set("wps_restart", "0");
	}

	return 0;
}

#define WSCD_BEFORE_STOP_STATUS "/tmp/wscd_before_stop_status"

int
stop_wps_method(void)
{
	if(getpid()!=1) {
		notify_rc("stop_wps_method");
                return 0;
	}

	rtklog("%s\n",__FUNCTION__);

	char cmd[64] = {0};
	sprintf(cmd, "cat %s > %s", WSCD_CONFIG_STATUS, WSCD_BEFORE_STOP_STATUS);
	doSystem(cmd);

#ifdef RTCONFIG_CONCURRENTREPEATER
	doSystem("echo 1 > /var/pbc_state1_cancel");
	doSystem("echo 1 > /var/pbc_state2_cancel");
#else
	int wlc_band = nvram_get_int("wlc_band");
	if(wlc_band == 0)
	{
		doSystem("echo 1 > /var/pbc_state1_cancel");
	}
	else if(wlc_band == 1)
	{
		doSystem("echo 1 > /var/pbc_state2_cancel");
	}
#endif
	if (sw_mode() == SW_MODE_REPEATER || (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1))
#ifdef RPAC68U
		set_led(LED_OFF_ALL, LED_OFF_ALL);
#else
		nvram_set_int("led_status", LED_WPS_FAIL);
#endif
	else if (sw_mode() == SW_MODE_AP) {
		char word[8], *next, tmp[16];
		int wl0_stage = LED_OFF_ALL, wl1_stage = LED_OFF_ALL;
		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			if (nvram_match(strcat_r(word, "_radio", tmp), "1")) {
				if (strstr(word, "0"))
					wl0_stage = LED_ON_ALL;
				else if (strstr(word, "1"))
					wl1_stage = LED_ON_ALL;
			}
		}
#ifdef RPAC68U
		set_led(wl0_stage, wl1_stage);
#else
		nvram_set_int("led_status", LED_WPS_FAIL);
#endif
	}

#ifdef RTCONFIG_CONCURRENTREPEATER
	/* If wlc ssid is not be set, don't set up it in repeater mode. */
	if (repeater_mode() && nvram_get_int("wlc_express") == 0) {
		if (!rtk_chk_wlc_ssid(VXD_2G))
			eval("ifconfig", "wl0-vxd", "down");
		if (!rtk_chk_wlc_ssid(VXD_5G))
			eval("ifconfig", "wl1-vxd", "down");
	}
#endif

	return 0;
}

int is_wps_stopped(void)
{
	FILE *fp;
	int status = 0;
	fp = fopen(WSCD_CONFIG_STATUS,"r");
	if(fp != NULL)
	{
		fscanf(fp,"%d",&status);
		fclose(fp);
	}
	nvram_set_int("wps_status", status);
	if(status == -1 || status == 2) {
		return 1;
	}
	return 0;
}

int is_wps_success(void)
{
	nvram_unset("wps_amas_enrollee");
	nvram_commit();
	if(nvram_get_int("wps_e_success") == 1)
		return 1;
	else
		return 0;
}

void
restart_wps_monitor(void)
{
	rtklog("%s\n",__FUNCTION__);
	rtk_start_wsc();
}
