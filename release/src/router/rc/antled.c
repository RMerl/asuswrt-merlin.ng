/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2020, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <shared.h>
#include <shutils.h>
#include <rc.h>
#include <wlutils.h>
#include <bcmendian.h>
#include <json.h>

#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))

#define ETHER_ADDR_STR_LEN	18
static struct itimerval itv;
static int unit_g = 0;
int antled_debug = 0;
int ledg_ant_mode = 0;

#define ANTLED_DEBUG_ALL	0x0001
#define ANTLED_DEBUG_NOTICE	0x0002
#define ANTLED_DEBUG_INFO	0x0004
#define ANTLED_DEBUG_SYSLOG	0x0008

#define MESH_INFO_FILE		"/tmp/cfgmnt_log.txt"

#define ANTLED_print(fmt, arg...) \
	do { \
		if (antled_debug & ANTLED_DEBUG_ALL) { \
			dbg(fmt, ##arg); \
		} \
	} while (0)

#define ANTLED_PRINT(fmt, arg...) \
	do { \
		if (antled_debug & ANTLED_DEBUG_ALL) { \
			dbg("antled >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			if (antled_debug & ANTLED_DEBUG_SYSLOG) \
				logmessage("antled", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
		} \
	} while (0)

#define ANTLED_NOTICE(fmt, arg...) \
	do { \
		if (antled_debug & PSTA_DEBUG_NOTICE) { \
			dbg("antled >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			if (antled_debug & ANTLED_DEBUG_SYSLOG) \
				logmessage("antled", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
		} \
	} while (0)

#define ANTLED_INFO(fmt, arg...) \
	do { \
		if (antled_debug & ANTLED_DEBUG_INFO) { \
			dbg("antled >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
			if (antled_debug & ANTLED_DEBUG_SYSLOG) \
				logmessage("antled", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
		} \
	} while (0)

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void antled_init()
{
	int i;
	int cled_gpio[4] = { 18, 19, 20, 21 };

	for (i = 0; i < 4; i++)
		cled_set(cled_gpio[i], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
}

static void antled_alarmtimer()
{
	int period = nvram_get_int("antled_period");
	period = (period > 0) ? period : 2;
	ANTLED_PRINT("period: %d\n", period);
	alarmtimer(period, 0);	/* default: 2 seconds */
}

extern sta_info_t *wl_sta_info(char *ifname, struct ether_addr *ea);

static int wl_rssi(int *rssi_export)
{
	char word[256], *next;
	char wl_ifnames[32] = { 0 };
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	struct maclist *auth = NULL;
	sta_info_t *sta = NULL;
	int mac_list_size;
	int i, ii, unit, rssi = -99, rssi_sta, rssi_org;
	char ea[ETHER_ADDR_STR_LEN];
	scb_val_t scb_val;
	json_object *pobj = NULL;
	json_object *reMacObj = NULL, *clientListObj = NULL;
	json_object *clientInList = NULL;
	json_object *onlineObj = NULL, *macObj = NULL;
	int clientListLen = 0;
	unsigned char *macp;
	unsigned char mac_binary[6];
	char *macaddr_strp;
	char macbuf2[13]; // For Store Original Mac Value (means without :)
	char apmacbuf[18];
	char apmac[18];
	unsigned long long macorigvalue;


	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);

	memset(mac_binary, 0x0, 6);

	if (!auth) return 0;

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		if (wl_ioctl(word, WLC_GET_INSTANCE, &unit, sizeof(unit)))
			continue;

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		if (!nvram_match(strlcat_r(prefix, "radio", tmp, sizeof(tmp)), "1"))
			continue;

		/* query wl for authenticated sta list */
		memset(auth, 0, mac_list_size);
		strlcpy((char*)auth, "authe_sta_list", mac_list_size);
		if (wl_ioctl(word, WLC_GET_VAR, auth, mac_list_size))
			continue;

		/* build authenticated sta list */
		ANTLED_print("\tAUTH COUNT: %d\n",auth->count);
		for (i = 0; i < auth->count; i ++) {
			sta = wl_sta_info(word, &auth->ea[i]);
			if (!sta) continue;
			if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

			/*  Check If Mesh RE Exist, If Exist, use REs As Target  */
			if (nvram_get_int("cfg_rejoin")){

			    	//Generate Mesh Infomation File
				eval("cfg_reportstatus");


				// Analysis Mesh Infomation Content
				pobj = json_object_from_file(MESH_INFO_FILE);
				json_object_object_get_ex(pobj, "mac", &reMacObj);
				json_object_object_get_ex(pobj, "client_list", &clientListObj);	
				clientListLen = json_object_array_length(clientListObj);

				/* Check Every REs Information */
				for (int j=0;j<clientListLen;j++){

					clientInList = json_object_array_get_idx(clientListObj,j);
					json_object_object_get_ex(clientInList, "online", &onlineObj);

					if(!json_object_get_int(onlineObj))
						continue;
					

					memset(&scb_val, 0, sizeof(scb_val));
					memcpy(&scb_val.ea, &auth->ea[i], ETHER_ADDR_LEN);
					wl_ioctl(word, WLC_GET_RSSI, &scb_val, sizeof(scb_val));
					rssi_sta = scb_val.val;
					rssi_org = rssi;
					rssi = max(rssi, rssi_sta);


					char macbuf[32];
					ether_etoa((void *)&auth->ea[i], macbuf);

					ANTLED_print("\t Current Check RE MAC: %s : %d\n",ether_etoa((void *)&auth->ea[i], ea),scb_val.val);

					memset(macbuf2, 0x0, 13);
					/* At Most 4 Band 2G, 5G, 5G-2, 6G   Only RE Online Need To Be Check*/
					for(int k=0;k<8;k++){

						//memset(macbuf3, 0x0, 24);

						if(k==0){
							json_object_object_get_ex(clientInList, "ap2g", &macObj);
							strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
						}
						else if(k==1){
							json_object_object_get_ex(clientInList, "ap5g", &macObj);
							strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
						}
						else if(k==2){
							json_object_object_get_ex(clientInList, "ap5g1", &macObj);
							strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
						}
						else if(k==3){
							json_object_object_get_ex(clientInList, "ap6g", &macObj);
							strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
						}
						else if(k==4){
							json_object_object_get_ex(clientInList, "ap2g", &macObj);
							macaddr_strp=json_object_get_string(macObj);
						}
						else if(k==5){
							json_object_object_get_ex(clientInList, "ap5g", &macObj);
							macaddr_strp=json_object_get_string(macObj);

						}
						else if(k==6){
							json_object_object_get_ex(clientInList, "ap5g1", &macObj);
							macaddr_strp=json_object_get_string(macObj);

						}
						else{
							json_object_object_get_ex(clientInList, "ap6g", &macObj);
							macaddr_strp=json_object_get_string(macObj);
						}

						if(k==4 || k==5 || k==6 || k==7){
							/* original mac copy without : */
							ether_atoe(macaddr_strp, mac_binary);

							/* Hex value mac without : */
							sprintf(macbuf2, "%02X%02X%02X%02X%02X%02X",
							mac_binary[0],
							mac_binary[1],
							mac_binary[2],
							mac_binary[3],
							mac_binary[4],
							mac_binary[5]);

							/* Last Hex value Mac - 1*/
							macorigvalue = strtoll(macbuf2, (char **) NULL, 16);
							macp = (unsigned char*) &macorigvalue;
							*(macp+5) = mac_binary[0];
							*(macp+0) = mac_binary[5] - 1;

							/* Combine To New Mac Value */
							sprintf(apmacbuf, "%02X:%02X:%02X:%02X:%02X:%02X",
							*(macp+5),
							*(macp+4),
							*(macp+3),
							*(macp+2),
							*(macp+1),
							*(macp+0));
							strncpy(apmac,apmacbuf,sizeof(apmac));
						}

						ANTLED_print("\t Current Check apXg MAC: %s : rssi_sta %d : rssi_orig %d : macp \n",apmac,rssi_sta,rssi_org);
						if (rssi_sta >= rssi_org && !strcmp(apmac,ether_etoa((void *)&auth->ea[i], ea))) {
							unit_g = unit;	
							ANTLED_PRINT("UNIT: %d, MAC: %s, RSSI: %d\n", unit, ether_etoa((void *)&auth->ea[i], ea), rssi_sta);
							if (sta->flags & WL_STA_SCBSTATS) {
								for (ii = WL_ANT_IDX_1; ii < WL_RSSI_ANT_MAX; ii++) {
									if (!unit)
										rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[ii]);
									else
										rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[3 - ii]);

									if (ii == WL_ANT_IDX_1)
										ANTLED_print("\t per antenna last rssi of rx data frames:");
										ANTLED_print(" %d", dtoh32(sta->rx_lastpkt_rssi[ii]));
									if (ii == WL_RSSI_ANT_MAX-1)
										ANTLED_print("\n");
								}
							}
						}
					}

					
				}
				
				/* Free Memory */
				json_object_put(onlineObj);
				json_object_put(macObj);
				json_object_put(clientInList);
				json_object_put(clientListObj);
				json_object_put(pobj);
			}
			else{
				memset(&scb_val, 0, sizeof(scb_val));
				memcpy(&scb_val.ea, &auth->ea[i], ETHER_ADDR_LEN);
				wl_ioctl(word, WLC_GET_RSSI, &scb_val, sizeof(scb_val));
				rssi_sta = scb_val.val;
				rssi_org = rssi;
				rssi = max(rssi, rssi_sta);

				if (rssi_sta > rssi_org) {
					unit_g = unit;
					ANTLED_PRINT("UNIT: %d, MAC: %s, RSSI: %d\n", unit, ether_etoa((void *)&auth->ea[i], ea), rssi_sta);
					if (sta->flags & WL_STA_SCBSTATS) {
						for (ii = WL_ANT_IDX_1; ii < WL_RSSI_ANT_MAX; ii++) {
							if (!unit)
								rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[ii]);
							else
								rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[3 - ii]);

							if (ii == WL_ANT_IDX_1)
								ANTLED_print("\t per antenna last rssi of rx data frames:");
							ANTLED_print(" %d", dtoh32(sta->rx_lastpkt_rssi[ii]));
							if (ii == WL_RSSI_ANT_MAX-1)
								ANTLED_print("\n");
						}
					}
				}
			}
		}

		for (i = 1; i < wl_max_no_vifs(unit); i++) {
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, i);
			if (!nvram_match(strlcat_r(prefix, "bss_enabled", tmp, sizeof(tmp)), "1"))
				continue;

			snprintf(name_vif, sizeof(name_vif), "wl%d.%d", unit, i);

			/* query wl for authenticated sta list */
			memset(auth, 0, mac_list_size);
			strlcpy((char*)auth, "authe_sta_list", mac_list_size);
			if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
				continue;

			ANTLED_print("\t[VIF] AUTH COUNT: %d\n",auth->count);
			for (ii = 0; ii < auth->count; ii++) {
				sta = wl_sta_info(name_vif, &auth->ea[ii]);
				if (!sta) continue;
				if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

				/*  Check If Mesh RE Exist, If Exist, use REs As Target  */
				if (nvram_get_int("cfg_rejoin")){


				    	//rssi = -99;

					//Generate Mesh Infomation File
					eval("cfg_reportstatus");


					// Analysis Mesh Infomation Content
					pobj = json_object_from_file(MESH_INFO_FILE);
					json_object_object_get_ex(pobj, "mac", &reMacObj);
					json_object_object_get_ex(pobj, "client_list", &clientListObj);	
					clientListLen = json_object_array_length(clientListObj);

					/* Check Every REs Information */
					for (int j=0;j<clientListLen;j++){

						clientInList = json_object_array_get_idx(clientListObj,j);
						json_object_object_get_ex(clientInList, "online", &onlineObj);

						if(!json_object_get_int(onlineObj))
							continue;
					

						memset(&scb_val, 0, sizeof(scb_val));
						memcpy(&scb_val.ea, &auth->ea[i], ETHER_ADDR_LEN);
						wl_ioctl(word, WLC_GET_RSSI, &scb_val, sizeof(scb_val));
						rssi_sta = scb_val.val;
						rssi_org = rssi;
						rssi = max(rssi, rssi_sta);


						char macbuf[32];
						ether_etoa((void *)&auth->ea[i], macbuf);

						memset(macbuf2, 0x0, 13);

						ANTLED_print("\t[VIF] Current Check RE MAC: %s : %d\n",ether_etoa((void *)&auth->ea[i], ea),scb_val.val);
						/* At Most 4 Band 2G, 5G, 5G-2, 6G   Only RE Online Need To Be Check*/
						for(int k=0;k<4;k++){

							if(k==0){
								json_object_object_get_ex(clientInList, "ap2g", &macObj);
								strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
							}
							else if(k==1){
								json_object_object_get_ex(clientInList, "ap5g", &macObj);
								strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
							}
							else if(k==2){
								json_object_object_get_ex(clientInList, "ap5g1", &macObj);
								strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
							}
							else if(k==3){
								json_object_object_get_ex(clientInList, "ap6g", &macObj);
								strncpy(apmac,json_object_get_string(macObj),sizeof(apmac));
							}
							else if(k==4){
								json_object_object_get_ex(clientInList, "ap2g", &macObj);
								macaddr_strp=json_object_get_string(macObj);
							}
							else if(k==5){
								json_object_object_get_ex(clientInList, "ap5g", &macObj);
								macaddr_strp=json_object_get_string(macObj);
							}
							else if(k==6){
								json_object_object_get_ex(clientInList, "ap5g1", &macObj);
								macaddr_strp=json_object_get_string(macObj);
							}
							else{
								json_object_object_get_ex(clientInList, "ap6g", &macObj);
								macaddr_strp=json_object_get_string(macObj);
							}

							if(k==4 || k==5 || k==6 || k==7){

								/* original mac copy without : */
								ether_atoe(macaddr_strp, mac_binary);

								/* Hex value mac without : */
								sprintf(macbuf2, "%02X%02X%02X%02X%02X%02X",
								mac_binary[0],
								mac_binary[1],
								mac_binary[2],
								mac_binary[3],
								mac_binary[4],
								mac_binary[5]);

								/* Last Hex value Mac - 1*/
								macorigvalue = strtoll(macbuf2, (char **) NULL, 16);
								macp = (unsigned char*) &macorigvalue;
								*(macp+5) = mac_binary[0];
								*(macp+0) = mac_binary[5] - 1;

								/* Combine To New Mac Value */
								sprintf(apmacbuf, "%02X:%02X:%02X:%02X:%02X:%02X",
								*(macp+5),
								*(macp+4),
								*(macp+3),
								*(macp+2),
								*(macp+1),
								*(macp+0));
								strncpy(apmac,apmacbuf,sizeof(apmac));
							}
							
							if (rssi_sta >= rssi_org &&  !strcmp(apmac,ether_etoa((void *)&auth->ea[i], ea))) {
								unit_g = unit;	
								ANTLED_PRINT("UNIT: %d, MAC: %s, RSSI: %d\n", unit, ether_etoa((void *)&auth->ea[i], ea), rssi_sta);
								if (sta->flags & WL_STA_SCBSTATS) {
									for (ii = WL_ANT_IDX_1; ii < WL_RSSI_ANT_MAX; ii++) {
										if (!unit)
											rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[ii]);
										else
											rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[3 - ii]);

										if (ii == WL_ANT_IDX_1)
											ANTLED_print("\t per antenna last rssi of rx data frames:");
											ANTLED_print(" %d", dtoh32(sta->rx_lastpkt_rssi[ii]));
										if (ii == WL_RSSI_ANT_MAX-1)
											ANTLED_print(" %d", dtoh32(sta->rx_lastpkt_rssi[ii]));
									}
								}
							}
						}

					
					}
				
					/* Free Memory */
					json_object_put(onlineObj);
					json_object_put(macObj);
					json_object_put(clientInList);
					json_object_put(clientListObj);
					json_object_put(pobj);
				}
				else{
					memset(&scb_val, 0, sizeof(scb_val));
					memcpy(&scb_val.ea, &auth->ea[ii], ETHER_ADDR_LEN);
					wl_ioctl(name_vif, WLC_GET_RSSI, &scb_val, sizeof(scb_val));
					rssi_sta = scb_val.val;
					rssi_org = rssi;
					rssi = max(rssi, rssi_sta);

					if (rssi_sta > rssi_org) {
						unit_g = unit;
						ANTLED_PRINT("UNIT: d, MAC: %s, RSSI: %d\n", unit, ether_etoa((void *)&auth->ea[ii], ea), rssi_sta);
						if (sta->flags & WL_STA_SCBSTATS) {
							for (ii = WL_ANT_IDX_1; ii < WL_RSSI_ANT_MAX; ii++) {
								if (!unit)
									rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[ii]);
								else
									rssi_export[ii] = dtoh32(sta->rx_lastpkt_rssi[3 - ii]);

								if (ii == WL_ANT_IDX_1)
									ANTLED_print("\t per antenna last rssi of rx data frames:");
								ANTLED_print(" %d", dtoh32(sta->rx_lastpkt_rssi[ii]));
								if (ii == WL_RSSI_ANT_MAX-1)
									ANTLED_print("\n");
							}
						}
					}
				}
			}
		}
	}

	if (auth) free(auth);

	return rssi;
}

int compare(const void *a, const void *b)
{
	int c = *(int *)a;
	int d = *(int *)b;

	if (c < d)
		return -1;	// a < b
	else if (c == d)
		return 0;	// a = b
	else
		return 1;	// a > b
}

static void antled(int sig)
{
	struct cled_config0 cc0;
	int cled_gpio[4] = { 21, 20, 19, 18 };
	int rssi[4], rssi_sort[4], i;
	int bright;
	int idx_highest_rssi = -1;

	/* Copy From ledg.c, Aura Mode Check Part*/
	if (nvram_get("antled_scheme_tmp")) {
		ledg_ant_mode = nvram_get_int("antled_scheme_tmp");
		nvram_unset("antled_scheme_tmp");
	} else
		ledg_ant_mode = nvram_get_int("antled_scheme");


	if(!nvram_get_int("AllLED")){
		AntennaGroupReset(LED_OFF);
		return;
	}


	/* Turn Off Ant LED IF WEB TAB Was Disabled */
	if(ledg_ant_mode == ANTLED_SCHEME_OFF){
		AntennaGroupReset(LED_OFF);
		alarmtimer(0, 0);
		return;
	}


	/* ANT LED Static Mode */
	if(ledg_ant_mode == ANTLED_SCHEME_STATIC){

		for (i = 0; i < 4; i++){

			memset(&cc0, 0, sizeof(struct cled_config0));
			cc0.bright_change_dir = 1;
			cc0.bright_ctl = 60;
			cled_set(cled_gpio[i], *(uint32_t *)&cc0, 0x0, 0x0, 0x0);
			ANTLED_PRINT("Static Mode idx: %d, bright: %d\n", i, 60);

		}
		alarmtimer(0, 0);
		return;
	}


	for (i = 0; i < 4; i++)
		rssi[i] = 0;

	wl_rssi(rssi);
	memcpy(rssi_sort, rssi, sizeof(rssi));

	if (!rssi[0] && !rssi[1] && !rssi[2] && !rssi[3])
		goto no_adjust;

	qsort((void *)rssi_sort, 4, sizeof(rssi_sort[0]), compare);
	for (i = 0; i < 4; i++) {
		if ((rssi[i] == rssi_sort[3])) {
			idx_highest_rssi = i;
			break;
		}
	}
no_adjust:
	for (i = 0; i < 4; i++) {

		//RSSI Brightness Mapping
		if (-39 <= rssi[i] && rssi[i] <= -1)
			bright = 128;
		else if (rssi[i] == 0)
			bright = 3;
		else if (-41 <= rssi[i] && rssi[i] <= -40)
			bright = 110;
		else if (-43 <=rssi[i] && rssi[i] <= -42)
			bright = 100;
		else if (-45 <= rssi[i] && rssi[i] <= -44)
			bright = 90;
		else if (-47 <=rssi[i] && rssi[i] <= -46)
			bright = 80;
		else if (-49 <=rssi[i] && rssi[i] <= -48)
			bright = 70;
		else if (-54 <= rssi[i] && rssi[i] <= -50)
			bright = 60;
		else if (-59 <= rssi[i] && rssi[i] <= -55)
			bright = 50;
		else if (-64 <= rssi[i] && rssi[i] <= -60)
			bright = 40;
		else if (-69 <= rssi[i] && rssi[i] <= -65)
			bright = 30;
		else if (-75 <= rssi[i] && rssi[i] <= -70)
			bright = 20;
		else
			bright = 3;

		memset(&cc0, 0, sizeof(struct cled_config0));
		cc0.bright_change_dir = 1;
		cc0.bright_ctl = bright;
		cled_set(cled_gpio[i], *(uint32_t *)&cc0, 0x0, 0x0, 0x0);
		ANTLED_PRINT("idx: %d, rssi: %d, bright: %d\n", i, rssi[i], bright);
	}
}

static void antled_exit(int sig)
{
	alarmtimer(0, 0);

	remove("/var/run/antled.pid");
	exit(0);
}

static void antled_reinit(int sig)
{

	//alarmtimer(0, 0);
	antled(SIGALRM);

	if(ledg_ant_mode == ANTLED_SCHEME_RSSI){
		antled_alarmtimer();
	}	

}

int
antled_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;


	if (!nvram_get_int("AllLED")){
		return 0;
	}

	/* write pid */
	if ((fp = fopen("/var/run/antled.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	antled_debug = nvram_get_int("antled_debug");

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGTSTP);

	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGALRM, antled);
	signal(SIGTERM, antled_exit);
	signal(SIGTSTP, antled_reinit);


	antled_init();
	antled_alarmtimer();

	/* Most of time it goes to sleep */
	while (1)
	{
		pause();
	}

	return 0;
}
