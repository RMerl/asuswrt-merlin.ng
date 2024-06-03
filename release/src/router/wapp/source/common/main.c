/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "wapp_cmm.h"
#ifdef DPP_SUPPORT
#include "dpp_wdev.h"
#endif /*DPP_SUPPORT*/

/*
========================================================================
IAPP
========================================================================
*/

#include "rt_config.h"
#include "rtmpiapp.h"
#include "iappdefs.h"

VOID IAPP_Usage(
	VOID)
{
	printf("\tUSAGE:\t\tralinkiappd <-e eth_if_name> <-w wireless_if_name>\n");
	printf("\t\t\t\t<-k security_key> <-d debug level>\n");
	printf("\tDefault:\tralinkiappd -e br0 -w ra0 -k 12345678 -d 3\n");
}

/*
========================================================================
HOTSPOT
========================================================================
*/

#include "hotspot.h"
extern struct hotspot_event_ops hs_event_ops;
extern void wapp_iface_deinit(struct wifi_app *wapp);

int hs_usage()
{

	DBGPRINT(RT_DEBUG_OFF, "hotspot [-f <hotspot configuration file>] [-m <hotspot mode>] [-i <hotspot ipc type>] [-d <debug level>] [-I <ineterface name>]\n");
	DBGPRINT(RT_DEBUG_OFF, "-f <hotspot configuration file>\n");
	DBGPRINT(RT_DEBUG_OFF, "-m <hotspot mode> (OPMODE_STA, OPMODE_AP)\n");
	DBGPRINT(RT_DEBUG_OFF, "-i <hotspot ipc type> (RA_WEXT, RA_NETLINK)\n");
	DBGPRINT(RT_DEBUG_OFF, "-d <hotspot debug level>\n");
	DBGPRINT(RT_DEBUG_OFF, "-h help\n");
	return 0;
}

/*
========================================================================
MBO
========================================================================
*/
#ifdef MAP_SUPPORT
int process_options(int argc, char *argv[], char *filename,
					int *opmode, int *drv_mode, int *debug_level, int *version, char *iface
					,char *map_cfg, char *map_user_cfg
					, RTMP_IAPP *pCtrlBK
					)
#else
int process_options(int argc, char *argv[], char *filename,
					int *opmode, int *drv_mode, int *debug_level, int *version, char *iface
					,RTMP_IAPP *pCtrlBK
					)
#endif
{
	int c;
	char *cvalue = NULL;
	int i = 0;

	/* IAPP init */
	strcpy(pCtrlBK->IfNameEth, FT_KDP_DEFAULT_IF_ETH);
	strcpy(pCtrlBK->IfNameWlan, FT_KDP_DEFAULT_IF_WLAN);
	strcpy(pCtrlBK->IfNameWlanIoctl[0], FT_KDP_DEFAULT_IF_WLAN_IOCTL);

	strcpy(pCtrlBK->CommonKey, FT_KDP_DEFAULT_PTK);


	opterr = 0;

	while ((c = getopt(argc, argv, "m:f:i:d:v:e:w:k:c:F:u:")) != -1) {
		switch (c) {
		case 'd':
			cvalue = optarg;
			if (os_strcmp(cvalue, "0") == 0)
				*debug_level = RT_DEBUG_OFF;
			else if (os_strcmp(cvalue, "1") == 0)
				*debug_level = RT_DEBUG_ERROR;
			else if (os_strcmp(cvalue, "2") == 0)
				*debug_level = RT_DEBUG_WARN;
			else if (os_strcmp(cvalue, "3") == 0)
				*debug_level = RT_DEBUG_TRACE;
			else if (os_strcmp(cvalue, "4") == 0)
				*debug_level = RT_DEBUG_INFO;
			else {
				DBGPRINT(RT_DEBUG_ERROR, "-d option does not have this debug_level %s\n", cvalue);
				return - 1;
			}
			break;
		case 'f':
			cvalue = optarg;
			os_strcpy(filename, cvalue);
			break;
		case 'm':
			cvalue = optarg;
			if (os_strcmp(cvalue, "OPMODE_STA") == 0)
				*opmode = OPMODE_STA;
			else if (os_strcmp(cvalue, "OPMODE_AP") == 0)
				*opmode = OPMODE_AP;
			else {
				DBGPRINT(RT_DEBUG_ERROR, "-m option does not have this mode %s\n", cvalue);
				return -1;
			}
			break;
		case 'i':
			cvalue = optarg;
			if (os_strcmp(cvalue, "RA_WEXT") == 0)
				*drv_mode = RA_WEXT;
			else if (os_strcmp(cvalue, "RA_NETLINK") == 0)
				*drv_mode = RA_NETLINK;
			else {
				DBGPRINT(RT_DEBUG_OFF, "-i option does not have this type %s\n", cvalue);
				return -1;
			}
			break;
		case 'v':
			cvalue = optarg;
           	*version = atoi(cvalue);
 			break;
		case 'h':
			cvalue = optarg;
			hs_usage();
			IAPP_Usage();
			break;
		case 'e':
			cvalue = optarg;
			os_strcpy(pCtrlBK->IfNameEth, cvalue);
			break;
		case 'w':
			cvalue = optarg;
			os_strcpy(pCtrlBK->IfNameWlan, cvalue);
			break;
		case 'k':
			cvalue = optarg;
			if (strlen(cvalue) > IAPP_ENCRYPT_KEY_MAX_SIZE)
			{
				cvalue[IAPP_ENCRYPT_KEY_MAX_SIZE] = 0x00;

				DBGPRINT(RT_DEBUG_TRACE, "iapp> key length can not be larger than %d!",
						IAPP_ENCRYPT_KEY_MAX_SIZE);
			}
			strcpy(pCtrlBK->CommonKey, cvalue);
			break;
		case 'c': //original IAPP -wi : wlan ioctl interface
			cvalue = optarg;
			strcpy(pCtrlBK->IfNameWlanIoctl[pCtrlBK->IfNameWlanCount++], cvalue);
			break;
		case '?':
			if (optopt == 'f') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'm') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'd') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'i') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (isprint(optopt)) {
				DBGPRINT(RT_DEBUG_OFF, "Unknow options -%c\n", optopt);
			} else {

			}
			return -1;
			break;
#ifdef MAP_SUPPORT
		case 'F':
			cvalue = optarg;
			if (strlen(cvalue) != 0) {
				strcpy(map_cfg, cvalue);
			}
			break;
		case 'u':
			cvalue = optarg;
			if (strlen(cvalue) != 0) {
				strcpy(map_user_cfg, cvalue);
			}
			break;
#endif
		}
	}

	if (strlen(iface) == 0)
	{
		os_strcpy(iface, DEFAULT_IFNAME);
		DBGPRINT(RT_DEBUG_OFF, "Default interface: %s\n", iface);
	} else {
		DBGPRINT(RT_DEBUG_OFF, "Interface: %s\n", iface);
	}

//label_exit:
	if (pCtrlBK->IfNameWlanCount == 0)
		pCtrlBK->IfNameWlanCount = 1;
	DBGPRINT(RT_DEBUG_TRACE, "iapp> -e=%s, -w=%s",
			pCtrlBK->IfNameEth, pCtrlBK->IfNameWlan);

	for (i = 0; i < pCtrlBK->IfNameWlanCount; i++) {
		DBGPRINT(RT_DEBUG_TRACE, ", -wi=%s",
			pCtrlBK->IfNameWlanIoctl[i]);
	}
	DBGPRINT(RT_DEBUG_TRACE, ", IfNameWlanCount = %d\n", pCtrlBK->IfNameWlanCount);
	return 0;

}

int wapp_read_wts_map_config(struct wifi_app *wapp, char *name,
		struct set_config_bss_info bss_info[], unsigned char max_bss_num,
		unsigned char *config_bss_num)
{
	FILE* f;
	int index, resv1, resv2, i, j;
	unsigned int mac_int[6] = {0};
	unsigned char mac[6] = {0};
	char op_class[4] = {0};
	unsigned char ssid[33] = {0};
	int authmode = 0, encrytype = 0;
	unsigned char key[65] = {0};
	char content[4096] = {0};
	char *pos1 = NULL, *pos2 = NULL;
	signed char ch = 0, sch = 0;
	unsigned char hidden_ssid = 0;
	unsigned char hidden_ssid_exist = 0;
	unsigned char fh_bss_unhidden = 0;
	unsigned char op_8x = 0, op_11x = 0, op_12x = 0;
	char backslash = 0x5C;
	char space = 0x20;
	char SUB = 0x1A;

	f = fopen(name, "r");
	if (f == NULL) {
		DBGPRINT(RT_DEBUG_TRACE, "open file %s fail\n", name);
		return -1;
	}

	for (i = 0; i < max_bss_num; i++) {
		memset(&bss_info[i], 0, sizeof(struct set_config_bss_info));
	}

	i = 0;
	do {
		ch = fgetc(f);
	/* If ssid is a space, need add \ as ESC infront of it in wts file. As space is regard as
	* a separator. If ssid is a \, need add \ as ESC in front of it. when parsing wts file,
	"\space"need remove \ and replace space to SUB. "\\"need remove \*/
		if (ch == backslash) {
			sch = fgetc(f);
			if (sch == space) {
				ch = SUB;
			} else if (sch == backslash) {
				ch = sch;
			} else {
				content[i++] = ch;
				ch = sch;
			}
		}
		content[i++] = ch;
	}while (EOF != ch);

	i = 0;
	pos1 = content;
	while (1) {
		/*index*/
		pos2 = strchr(pos1, ',');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "index not found\n");
			break;
		}
		*pos2 = '\0';
		sscanf(pos1, "%d", &index);
		pos2++;
		pos1 = pos2;

		/*mac*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "mac not found\n");
			break;
		}
		*pos2 = '\0';
		sscanf(pos1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_int,
				(mac_int + 1), (mac_int + 2), (mac_int + 3),
				(mac_int + 4), (mac_int + 5));
		for (j = 0; j < 6; j++)
			mac[j] = (unsigned char)mac_int[j];

		pos2++;
		pos1 = pos2;

		/*opclass*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "opclass not found\n");
			break;
		}
		*pos2 = '\0';

		sscanf(pos1, "%s", op_class);
		DBGPRINT(RT_DEBUG_TRACE, "opclass %s\n", op_class);

		if (0 == memcmp(op_class, "8", 1)) {
			op_8x ++;
			bss_info[i].operating_chan = RADIO_24G;
		}
		else if (0 == memcmp(op_class, "11", 2)) {
			op_11x ++;
			bss_info[i].operating_chan = RADIO_5GL;
		}
		else if (0 == memcmp(op_class, "12", 2)) {
			op_12x ++;
			bss_info[i].operating_chan = RADIO_5GH;
		}

		pos2++;
		pos1 = pos2;

		/*ssid*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "ssid not found\n");
			break;
		}
		*pos2 = '\0';
		sscanf(pos1, "%s", ssid);
		for (j = 0; j < 32; j++) {
			if (ssid[j] == SUB)
				ssid[j] = space;
		}
		pos2++;
		pos1 = pos2;

		/*authmode*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "authmode not found\n");
			break;
		}
		*pos2 = '\0';
		sscanf(pos1, "0x%04x", &authmode);
		pos2++;
		pos1 = pos2;

		/*encrytype*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "encrytype not found\n");
			break;
		}
		*pos2 = '\0';
		sscanf(pos1, "0x%04x", &encrytype);
		pos2++;
		pos1 = pos2;

		/*key*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "key not found\n");
			break;
		}
		*pos2 = '\0';
		sscanf(pos1, "%s", key);
		pos2++;
		pos1 = pos2;

		/*resv1*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "resv1 not found\n");
			break;
		}
		*pos2 = '\0';
		sscanf(pos1, "%d", &resv1);
		pos2++;
		pos1 = pos2;
		/*resv2*/
		pos2 = strstr(pos1, "hidden-");
		if (pos2 != NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "hidden SSID exist\n");
			hidden_ssid_exist = 1;
		} else {
			DBGPRINT(RT_DEBUG_TRACE, "no hidden SSID\n");
			hidden_ssid_exist = 0;
			hidden_ssid = 'N';
		}

		if (!hidden_ssid_exist) {
			/*resv2*/
			pos2 = strchr(pos1, '\n');
			if (pos2 == NULL) {
				DBGPRINT(RT_DEBUG_TRACE, "resv2 not found\n");
				break;
			}
			*pos2 = '\0';
			sscanf(pos1, "%d", &resv2);
			pos2++;
			pos1 = pos2;
		} else {
			/*resv2*/
			pos2 = strchr(pos1, ' ');
			if (pos2 == NULL) {
				DBGPRINT(RT_DEBUG_TRACE, "resv2 not found\n");
				break;
			}
			*pos2 = '\0';
			sscanf(pos1, "%d", &resv2);
			pos2++;
			pos1 = pos2;

			/*hidden ssid*/
			pos2 = strchr(pos1, '\n');
			if (pos2 == NULL) {
				DBGPRINT(RT_DEBUG_TRACE, "hidden ssid not found\n");
				break;
			}
			*pos2 = '\0';
			if (sscanf(pos1, "hidden-%c", &hidden_ssid) == 1) {
				pos2++;
				pos1 = pos2;
			} else {
				DBGPRINT(RT_DEBUG_TRACE, "wrong format hidden ssid\n");
				break;
			}
		}
		bss_info[i].authmode = authmode;
		bss_info[i].encryptype = encrytype;
		memcpy(bss_info[i].mac, mac, 6);
		memcpy(bss_info[i].key, key, sizeof(key));
		memcpy(bss_info[i].oper_class, op_class, sizeof(op_class));
		memcpy(bss_info[i].ssid, ssid, sizeof(ssid));
		bss_info[i].wfa_vendor_extension |= (resv1 & 0x01) << 6;
		bss_info[i].wfa_vendor_extension |= (resv2 & 0x01) << 5;
		if (hidden_ssid == 'Y')
			bss_info[i].hidden_ssid = 1;
		else if (hidden_ssid == 'N')
			bss_info[i].hidden_ssid = 0;
		else
			bss_info[i].hidden_ssid = 0;
		DBGPRINT(RT_DEBUG_OFF, "set bss index=%d, mac=%02x:%02x:%02x:%02x:%02x:%02x,"
				" opclass=%s, ssid=%s, authmode=%04x, encrytype=%04x, key=%s, "
				"bh_bss=%s, fh_bss=%s hidden_ssid=%d\n",
				index, PRINT_MAC(mac), op_class, ssid, authmode, encrytype, key,
				bss_info[i].wfa_vendor_extension & BIT_BH_BSS ? "1" : "0",
				bss_info[i].wfa_vendor_extension & BIT_FH_BSS ? "1" : "0", bss_info[i].hidden_ssid);
		i++;
		if (i >= max_bss_num) {
			DBGPRINT(RT_DEBUG_TRACE, "too much bss wireless setting info\n");
			i--;
			break;
		}
	}

	*config_bss_num = i;
	DBGPRINT(RT_DEBUG_OFF, "config_bss_num=%d\n", *config_bss_num);
	fclose(f);

	if (0 == op_8x) {
		DBGPRINT(RT_DEBUG_TRACE, RED("!!!2G Band BSS Configuration is missing!!!\n"));
	}
	if (0 == op_11x) {
		DBGPRINT(RT_DEBUG_TRACE, RED("!!!5G Low Band BSS Configuration is missing!!!\n"));
	}
	if (0 == op_12x) {
		DBGPRINT(RT_DEBUG_TRACE, RED("!!!5G High Band BSS Configuration is missing!!!\n"));
	}

	fh_bss_unhidden = 0;
	for (i = 0; i < *config_bss_num; i++) {
		if ((bss_info[i].oper_class[0] == '8') &&
				(bss_info[i].wfa_vendor_extension & BIT_FH_BSS) &&
				(bss_info[i].hidden_ssid == 0)) {
			fh_bss_unhidden = 1;
			break;
		}
	}
	if (fh_bss_unhidden == 0) {
		DBGPRINT(RT_DEBUG_TRACE, "all fronthaul bss of radio 24G is hidden! wps may fail!\n");
	}

	fh_bss_unhidden = 0;
	for (i = 0; i < *config_bss_num; i++) {
		if ((bss_info[i].oper_class[1] == '1') &&
				(bss_info[i].wfa_vendor_extension & BIT_FH_BSS) &&
				(bss_info[i].hidden_ssid == 0)) {
			fh_bss_unhidden = 1;
			break;
		}
	}
	if (fh_bss_unhidden == 0) {
		DBGPRINT(RT_DEBUG_TRACE, "all fronthaul bss of radio 5GL is hidden! wps may fail!\n");
	}

	fh_bss_unhidden = 0;
	for (i = 0; i < *config_bss_num; i++) {
		if ((bss_info[i].oper_class[1] == '2') &&
				(bss_info[i].wfa_vendor_extension & BIT_FH_BSS) &&
				(bss_info[i].hidden_ssid == 0)) {
			fh_bss_unhidden = 1;
			break;
		}
	}
	if (fh_bss_unhidden == 0) {
		DBGPRINT(RT_DEBUG_TRACE, "all fronthaul bss of radio 5GH is hidden! wps may fail!\n");
	}
	return 0;
}


int main(int argc, char *argv[])
{
	int ret;
	int opmode;
	int drv_mode;
	int debug_level;
	int version = 2;
	char filename[256] = {0};
	struct wifi_app wapp_cfg;
	struct hotspot hs;
	struct mbo_cfg mbo;
	struct oce_cfg oce;
#ifdef MAP_SUPPORT
	struct map_info map;
	char value[10] = {0};
	char map_cfg[128]= {0};
	char map_user_cfg[128]= {0};
#endif
	struct _RTMP_IAPP IAPP_Ctrl_Block;
	struct wifi_app *wapp = &wapp_cfg;
	pid_t child_pid_hs,child_pid_iapp;

	/* default setting */
	opmode = OPMODE_AP;
	drv_mode = RA_WEXT;

	memset(wapp,0,sizeof(struct wifi_app));
	memset(&hs,0,sizeof(struct hotspot));
	memset(&mbo,0,sizeof(struct mbo_cfg));
	memset(&oce,0,sizeof(struct oce_cfg));
#ifdef MAP_SUPPORT
    memset(&map, 0, sizeof(struct map_info));
#endif
	memset(&IAPP_Ctrl_Block,0,sizeof(struct _RTMP_IAPP));

#ifdef MAP_SUPPORT
	ret = process_options(argc, argv, filename, &opmode, &drv_mode, &debug_level, &version, &wapp_cfg.iface[0], map_cfg, map_user_cfg, &IAPP_Ctrl_Block);
#else
	ret = process_options(argc, argv, filename, &opmode, &drv_mode, &debug_level, &version, &wapp_cfg.iface[0],&IAPP_Ctrl_Block);

#endif

	if(ret){
		hs_usage();
		return -1;
	}

	RTDebugLevel = debug_level;

	ret = wapp_cmm_init(wapp,drv_mode,opmode,version,&hs,&mbo, &oce, &map, &IAPP_Ctrl_Block);
	if (ret)
		goto error;

#ifdef MAP_SUPPORT
	if(strlen(map_cfg) == 0)
	{
		snprintf(wapp->map->map_cfg, 128, "/etc/map/mapd_cfg");
	}
	else
	{
		snprintf(wapp->map->map_cfg, 128, "%s", map_cfg);
	}

	if(strlen(map_user_cfg) == 0)
	{
		snprintf(wapp->map->map_user_cfg, 128, "/etc/map/mapd_user.cfg");
	}
	else
	{
		snprintf(wapp->map->map_user_cfg, 128, "%s", map_user_cfg);
	}
#endif

	DBGPRINT(RT_DEBUG_OFF, "Map cfg file: %s Map user cfg: %s\n", wapp->map->map_cfg, wapp->map->map_user_cfg);
	DBGPRINT(RT_DEBUG_OFF, "WAPP version: %s\nwapp cmm type version: %s\n", WAPP_VERSION, VERSION_WAPP_CMM);

#ifdef MAP_SUPPORT
		dl_list_init(&wapp->air_monitor_query_list);
		dl_list_init(&wapp->sta_mntr_list);
#endif
	setsid();
	child_pid_hs = fork();
	if (child_pid_hs == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Initialize socket\n");
		ret = wapp_socket_and_ctrl_inf_init(wapp,drv_mode,opmode);
		ret += wapp_get_wireless_interfaces(wapp);
#ifdef MAP_SUPPORT
	get_map_parameters(wapp->map, "Enable_WPS_toggle_5GL_5GH", value, NON_DRIVER_PARAM, sizeof(value));
	if (!strcmp(value,"1"))
		wapp->map->enable_wps_toggle_5GL_5GH = 1;
	else
		wapp->map->enable_wps_toggle_5GL_5GH = 0;
	get_map_parameters(wapp->map, "MAP_QuickChChange", value, NON_DRIVER_PARAM, sizeof(value));
	if (!strcmp(value,"1"))
		wapp->map->quick_ch_change = 1;
	else
		wapp->map->quick_ch_change = 0;
#ifdef MAP_R2
	get_map_parameters(wapp->map, "MetricRepIntv", value, NON_DRIVER_PARAM, sizeof(value));
	wapp->map->metric_rep_intv = atoi(value);
	get_map_parameters(wapp->map, "MaxStaAllowed", value, NON_DRIVER_PARAM, sizeof(value));
	wapp->map->max_client_cnt = atoi(value);
	if (wapp->map->max_client_cnt <= 0 || wapp->map->max_client_cnt > MAX_NUM_OF_CLIENT)
		wapp->map->max_client_cnt = MAX_NUM_OF_CLIENT;
#endif
		/*if MAP enable, create ARP socket for br & wireless interface*/
		get_map_parameters(wapp->map, "MapMode", value, DRIVER_PARAM, sizeof(value));
                if (!strcmp(value,"1"))
			ret += wapp_create_arp_socket(wapp);
#endif
		if (ret)
			goto error;

		if (os_strlen(filename) == 0) {
				DBGPRINT(RT_DEBUG_TRACE, "Use default configuration file /etc/wapp_ap.conf");
				snprintf(filename, 256, WAPP_CONF_PATH"/wapp_ap.conf");
		}

		ret = wapp_init_all_config(wapp, filename);

		if (ret) {
			DBGPRINT(RT_DEBUG_OFF, "Initial hotspot configuration file(%s) fail\n", filename);
			goto error0;
		}

		ret = hotspot_set_ap_ifaces_all_ies(wapp);

		if (ret)
			goto error1;

		/* Enable hotspot feature for all interfaces */
		ret = hotspot_onoff_all(wapp, 1);

//		if (ret)
//			goto error2;
#ifdef DPP_SUPPORT
		if (wapp_dpp_init(wapp) < 0) {
			DBGPRINT(RT_DEBUG_OFF, "failed to init dpp\n");
		}
		if (wapp_dpp_gas_server_init(wapp) < 0) {
			DBGPRINT(RT_DEBUG_OFF, "failed to init hostapd dpp\n");
		}
		DBGPRINT(RT_DEBUG_OFF, "initialized dpp\n");
		dpp_read_config_file(wapp->dpp);
		if (wapp->dpp->dpp_configurator_supported)
			wapp_dpp_configurator_add(wapp->dpp);

		DBGPRINT(RT_DEBUG_OFF, "key %s\n", wapp->dpp->dpp_private_key);
		dpp_bootstrap_gen_at_bootup(wapp->dpp, (char *)wapp->dpp->dpp_private_key, NULL, NULL);
		if (wapp->dpp->is_map) {
			wapp_read_wts_map_config(wapp, "/etc/wts_bss_info_config",
				wapp->dpp->bss_config, MAX_SET_BSS_INFO_NUM, &wapp->dpp->bss_config_num);

			DBGPRINT(RT_DEBUG_OFF, "Setting configurator\n");
			// TODO pass key here
		}
		dl_list_init(&wapp->scan_results_list);
#endif /*DPP_SUPPORT*/
		hotspot_run(wapp);

		/* Disable hotspot feature for all interfaces */
		hotspot_onoff_all(wapp, 0);
	}
	else{
		child_pid_iapp = fork();
		if (child_pid_iapp == 0) {
			DBGPRINT(RT_DEBUG_OFF, "Initialize IAPP\n");
#ifdef IAPP_OS_LINUX
			IAPP_Task((VOID *)wapp->IAPP_Ctrl_Block);
#endif /* IAPP_OS_LINUX */
		}
		else{
			return 0;
		}

		return 0;
	}

// TODO: WAPP deinit

//error2:
//	hotspot_reset_all_ap_resource(wapp);
error1:
	wapp_deinit_all_config(wapp);
error0:
	hotspot_deinit(wapp);
#ifdef MAP_SUPPORT
	map_bss_table_release(wapp->map);
	wapp_iface_deinit(wapp);
#endif
error:
	exit(-1);
}

