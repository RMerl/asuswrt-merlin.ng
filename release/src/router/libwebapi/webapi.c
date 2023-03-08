#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <shared.h>
#include <shutils.h>
#include <stdint.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <json.h>
#include <webapi.h>
#if defined(RTCONFIG_BWDPI)
#include <bwdpi.h>
#endif

void httpd_nvram_commit(void){

	/* 0:nvram 1:openvpn 2:ipsec 3:usericon */
	sync_profile_update_time(0);
}

int get_nvram_dlen(char *name)
{
	struct nvram_tuple *t;
	for (t = router_defaults; t->name; t++)
	{
		if(!strcmp(t->name, name))
			return t->len;
	}
	return 0;
}


int is_port_in_use(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		//dbg("socket error\n");
		return 1;
	}
	//dbg("Opened %d\n", sock);

	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		if(errno == EADDRINUSE) {
			dbg("the port is not available. already to other process\n");
			return 1;
		} else {
			dbg("could not bind to process (%d) %s\n", errno, strerror(errno));
			return 1;
		}
	}

	socklen_t len = sizeof(serv_addr);
	if (getsockname(sock, (struct sockaddr *)&serv_addr, &len) == -1) {
		dbg("getsockname");
		return 1;
	}

	dbg("port number %d\n", ntohs(serv_addr.sin_port));

	if (close (sock) < 0 ) {
	    dbg("did not close: %s\n", strerror(errno));
	    return 1;
	}
	return 0;
}

#ifdef RTCONFIG_OPENVPN

// Using this api need to upload OPENVPN_UPLOAD_FILE first and if upload finish nvram set upload_server_ovpn_cert_temp=1
int upload_server_ovpn_cert_cgi()
{
	int ret = HTTP_FAIL;
	char cmd[1024] = {0};
	if (nvram_get_int("upload_server_ovpn_cert_temp")) {
		if(check_if_file_exist(OPENVPN_UPLOAD_FILE)) {
			int i, unit;
			char file_path[128];
			char *lists[] = {
				"ca",
				"ca_key",
				"client_crt",
				"client_key",
				"crt",
				"dh",
				"key",
				"crl",
				"extra"
				"static",
				NULL
			};
			snprintf(cmd, sizeof(cmd), "tar -xzf %s -C %s", OPENVPN_UPLOAD_FILE, OPENVPN_UPLOAD_FLODER);
			system(cmd);

			unit = nvram_get_int("vpn_server_unit");
			for (i = 0; i < ARRAY_SIZE(lists) && lists[i] != NULL; ++i) {
				memset(file_path, 0, sizeof(file_path));
				snprintf(file_path, sizeof(file_path), "%s/vpn_crt_server%d_%s", OPENVPN_UPLOAD_FLODER, unit, lists[i]);
				if(check_if_file_exist(file_path)) {
					eval("mv", "-f", file_path, JFFS_OPENVPN);
				}
			}
			ret = HTTP_OK;
			notify_rc("restart_openvpnd");
			notify_rc("restart_chpass");
			sync_profile_update_time(1); /* 0:nvram 1:openvpn 2:ipsec 3:usericon */
		}
	}
	nvram_unset("upload_server_ovpn_cert_temp");
	doSystem("rm -rf %s", OPENVPN_UPLOAD_FLODER);
	return ret;
}

int gen_server_ovpn_file()
{
	char cmd[1024];
	memset(cmd, 0, sizeof(cmd));
	if (check_if_dir_exist(JFFS_OPENVPN)) {
		int i, unit;
		char file_path[128];
		char filename[32];
		snprintf(cmd, sizeof(cmd), "tar czf %s -C %s", OPENVPN_EXPORT_FILE, JFFS_OPENVPN);
		char *lists[] = {
			"ca",
			"ca_key",
			"client_crt",
			"client_key",
			"crt",
			"dh",
			"key",
			"crl",
			"extra",
			"static",
			NULL
		};
		unit = nvram_get_int("vpn_server_unit");
		for (i = 0; i < ARRAY_SIZE(lists) && lists[i] != NULL; ++i) {
			memset(file_path, 0, sizeof(file_path));
			snprintf(filename, sizeof(filename), "vpn_crt_server%d_%s", unit, lists[i]);
			snprintf(file_path, sizeof(file_path), "%s%s", JFFS_OPENVPN, filename);
			if(check_if_file_exist(file_path)) {
				strlcat(cmd, " ", sizeof(cmd));
				strlcat(cmd, filename, sizeof(cmd));
			}
		}
		system(cmd);
		return HTTP_OK;
	}else
		return HTTP_FAIL;
}

#endif /* RTCONFIG_OPENVPN */

#ifdef RTCONFIG_IPSEC
int gen_server_ipsec_file()
{
	char cmd[1024];
	memset(cmd, 0, sizeof(cmd));
	if (check_if_dir_exist(JFFS_CA_FILES)) {
		int i;
		char file_path[128];
		snprintf(cmd, sizeof(cmd), "tar czf %s -C %s", IPSEC_EXPORT_FILE, JFFS_CA_FILES);
		char *lists[] = {
				"asusCert.der",
				"asusCert.pem",
				"ca.pem",
				"ca_init.sh",
				"generate.sh",
				"svrCert.pem",
				"svrKey.pem",
				NULL
			};
		for (i = 0; i < ARRAY_SIZE(lists) && lists[i] != NULL; ++i) {
			memset(file_path, 0, sizeof(file_path));
			snprintf(file_path, sizeof(file_path), "%s%s", JFFS_CA_FILES, lists[i]);
			if(check_if_file_exist(file_path)) {
				strlcat(cmd, " ", sizeof(cmd));
				strlcat(cmd, lists[i], sizeof(cmd));
			}
		}
		system(cmd);
		return HTTP_OK;
	}else
		return HTTP_FAIL;
}

// Using this api need to upload IPSEC_UPLOAD_FILE first and if upload finish nvram set upload_server_ovpn_cert_temp=1
int upload_server_ipsec_cert_cgi()
{
	int ret = HTTP_FAIL;
	if (nvram_get_int("upload_server_ipsec_cert_temp")) {
		char cmd[1024] = {0};
		if(check_if_file_exist(IPSEC_UPLOAD_FILE)) {
			int i = 0;
			char file_path[128] = {0};
			char *lists[] = {
				"asusCert.der",
				"asusCert.pem",
				"ca.pem",
				"ca_init.sh",
				"generate.sh",
				"svrCert.pem",
				"svrKey.pem",
				NULL
			};
			snprintf(cmd, sizeof(cmd), "tar -xzf %s -C %s", IPSEC_UPLOAD_FILE, IPSEC_UPLOAD_FLODER);
			system(cmd);

			for (i = 0; i < ARRAY_SIZE(lists) && lists[i] != NULL; ++i) {
				memset(file_path, 0, sizeof(file_path));
				snprintf(file_path, sizeof(file_path), "%s/%s", IPSEC_UPLOAD_FLODER, lists[i]);
				if(check_if_file_exist(file_path)) {
					eval("mv", "-f", file_path, JFFS_CA_FILES);
				}
			}
			ret = HTTP_OK;
			notify_rc("ipsec_restart");
			sync_profile_update_time(2);
		}
	}

	nvram_unset("upload_server_ipsec_cert_temp");
	doSystem("rm -rf %s", IPSEC_UPLOAD_FLODER);
	return ret;
}
#endif /* RTCONFIG_IPSEC */

int get_wl_nband_list()
{
	int unit = 0, ret = 0;
	int band = 0, count2g = 0, count5g = 0, count6g = 0;
	char band_str[8] = {0}, word[256] = {0}, *next = NULL;
	char tmp[128] = {0}, prefix[] = "wlXXXXXXXXXX_";
	char wlnband_list[64] = {0};

	foreach (word, nvram_safe_get("wl_ifnames"), next) {

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		band = nvram_get_int(strlcat_r(prefix, "nband", tmp, sizeof(tmp)));

		switch (band){
			case 1:
				snprintf(band_str, sizeof(band_str), "5g%d", ++count5g);
				break;
			case 2:
				snprintf(band_str, sizeof(band_str), "2g%d", ++count2g);
				break;
			case 4:
				snprintf(band_str, sizeof(band_str), "6g%d", ++count6g);
				break;
		}

		if(unit != 0)
			strlcat(wlnband_list, "<", sizeof(wlnband_list));

		strlcat(wlnband_list, band_str, sizeof(wlnband_list));

		unit++;
	}

	nvram_set("wlnband_list", wlnband_list);

	return ret;
}


char *wl_nband_to_wlx(char *nv_name, char *wl_name, size_t len){

	int i = 0;
	char prefix[8] = {0}, tmp[32] = {0};
	char wlnband_list[64] = {0};
	char word[64] = {0}, *next = NULL;

	strlcpy(wlnband_list, nvram_safe_get("wlnband_list"), sizeof(wlnband_list));

	//for(i=0; i<(sizeof(wl_band_list)/sizeof(wl_band_list[0])) && wl_band_list[i][0] != '\0'; i++){
	foreach_60(word, wlnband_list, next){
		if(strncmp(nv_name, word, 3) == 0){
			snprintf(prefix, sizeof(prefix), "wl%d", i);
			break;
		}
		i++;
	}

	if(prefix[0] == '\0')
		strlcpy(wl_name, nv_name, len);
	else
		strlcpy(wl_name, strlcat_r(prefix, nv_name+3, tmp, sizeof(tmp)), len);

	return wl_name;
}

#ifdef RTCONFIG_MULTILAN_CFG
static int get_sdn_rwd_cap_array(struct json_object *sdn_rwd_cap_array){

	const char *sdn_rwd_cap[] = {
#ifdef RTCONFIG_BUSINESS
		"Employee", "Portal", "Sched", "Customized",
#else
		 "Kids",
#endif
		"Guest", "IoT", "VPN",
		NULL};

	int i = 0;

	for(i=0; sdn_rwd_cap[i]; i++)
		json_object_array_add(sdn_rwd_cap_array,json_object_new_string(sdn_rwd_cap[i]));

	return i;
}
#endif

struct RWD_MAPPING_TABLE rwd_mapping_t[] =
{
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GTAXE11000) || defined(GTAC2900) || defined(GTAX11000) || defined(GSAX3000) || defined(GSAX5400) || defined(GTAX11000_PRO) || defined(TUFAX5400) || defined(GTAXE16000) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2)
	{"AuraRGB", "light_effect/light_effect.html", "light_effect/light_effect_white.css"},
	{"AuraRGB_preview", "light_effect/light_effect_pre.html", NULL},
#endif
	{"Tencent", "game_accelerator_tencent.html", NULL},
#if defined(RTCONFIG_OOKLA) || defined(RTCONFIG_OOKLA_LITE)
	{"SpeedTest", "internet_speed.html", "css/internetSpeed_white_theme.css"},
#endif
#if defined(RTCONFIG_BWDPI)
	{"AiProtection_MALS", "AiProtection_MaliciousSitesBlocking_m.asp", NULL},
	{"AiProtection_VP", "AiProtection_IntrusionPreventionSystem_m.asp", NULL},
	{"AiProtection_CC", "AiProtection_InfectedDevicePreventBlock_m.asp", NULL},
#endif
	{"VPN_Fusion", "VPN/vpnc.html", "VPN/vpncWHITE.css"},
	{"VPN_Server", "VPN/vpns.html", "VPN/vpnsWHITE.css"},
#ifdef RTCONFIG_MULTILAN_CFG
	{"SDN", "SDN/sdn.html", "SDN/sdn_WHITE.css"},
#endif
#ifdef RTCONFIG_DASHBOARD
	{"Dashboard", "index.html?url=dashboard", NULL},
#endif
	{NULL, NULL, NULL}
};

int get_rwd_table(struct json_object *rwd_mapping)
{
	int white_theme_status = 0;
	char *url = NULL;
	char url_file[128] = {0}, check_path[128] = {0};
	struct RWD_MAPPING_TABLE *p;
	struct json_object *function_obj = NULL;

	for(p = rwd_mapping_t; p->name; p++){
		white_theme_status = 0;
#if defined(RTCONFIG_BWDPI)
		if(!strcmp("AiProtection_MALS", p->name)){
			if(!dump_dpi_support(INDEX_MALS) || check_tcode_blacklist())
				continue;
		}
		if(!strcmp("AiProtection_VP", p->name)){
			if(!dump_dpi_support(INDEX_VP) || check_tcode_blacklist())
				continue;
		}
		if(!strcmp("AiProtection_CC", p->name)){
			if(!dump_dpi_support(INDEX_CC) || check_tcode_blacklist())
				continue;
		}
#endif
		strlcpy(url_file, p->path, sizeof(url_file));
		url = strtok(url_file, "?");
		snprintf(check_path, sizeof(check_path), "/www/%s", url);

		if(!check_if_file_exist(check_path))
			continue;

		if(p->white_theme){
			snprintf(check_path, sizeof(check_path), "/www/%s", p->white_theme);
			if(check_if_file_exist(check_path))
				white_theme_status = 1;
		}

		function_obj = json_object_new_object();

		json_object_object_add(function_obj, "path", json_object_new_string(p->path));
		json_object_object_add(function_obj, "white_theme", json_object_new_string((white_theme_status==1)?"1":"0"));
#if defined(RTCONFIG_MULTILAN_CFG)
		if(!strcmp("SDN", p->name)){
			json_object *sdn_rwd_cap_array = json_object_new_array();
			if(get_sdn_rwd_cap_array(sdn_rwd_cap_array))
				json_object_object_add(function_obj, "wizard", sdn_rwd_cap_array);
		}
#endif
		json_object_object_add(rwd_mapping, p->name, function_obj);
	}
	return 1;
}


int
update_string_in_62(char *out, int idx, char *update_str, int out_len)
{
	int ret = 0, find_idx = 0;
	char *buf, *g, *p;
	g = buf = strdup(out);

	while ((p = strchr(g, '>')) != NULL) {
		if(find_idx == idx)
		{
			if((int)(p-buf)-(int)(p-g) < out_len){
				out[(int)(p-buf)-(int)(p-g)] = '\0';
				strlcat(out, update_str, out_len);
				strlcat(out, p, out_len);
			}
			ret = find_idx;
			find_idx++;
			break;
		}
		else{
			g = p + 1;
			find_idx++;
		}
	}

	if(find_idx == idx){ // Consider the field is at the end of in_list (no '>' occur anymore).
		if((int)(g-buf) < out_len){
			strncpy(out, buf, (g-buf));
			out[(int)(g-buf)] = '\0';
			strlcat(out, update_str, out_len);
			out[(int)(g-buf)+strlen(update_str)] = '\0';
			ret = find_idx;
		}
	}
	free(buf);

	return ret;

}

int enable_wireguard_client(int wgc_index, char *vpnc_enable)
{
#ifdef RTCONFIG_WIREGUARD
	int ret = HTTP_NO_CHANGE, find_idx = 0;
	char vpnc_index[2] = {0}, vpnc_name[16] = {0}, vpnc_enable_org[8] = {0};
	char word[1024]={0}, word_tmp[1024] = {0}, *next = NULL;
	char vpnc_clientlist[CKN_STR8192] = {0}, vpnc_clientlist_tmp[CKN_STR8192] = {0};

	strlcpy(vpnc_clientlist, nvram_safe_get("vpnc_clientlist"), sizeof(vpnc_clientlist));

	foreach_60(word, vpnc_clientlist, next){
		strlcpy(word_tmp, word, sizeof(word_tmp));
		get_string_in_62(word_tmp, 2, vpnc_index, sizeof(vpnc_index));
		get_string_in_62(word_tmp, 1, vpnc_name, sizeof(vpnc_name));
		get_string_in_62(word_tmp, 5, vpnc_enable_org, sizeof(vpnc_enable_org));

		if(!strcmp(vpnc_name, "WireGuard") && wgc_index == safe_atoi(vpnc_index) && strcmp(vpnc_enable_org, vpnc_enable)!=0){
			if(update_string_in_62(word_tmp, 5, vpnc_enable, sizeof(word_tmp))){
				if(vpnc_clientlist_tmp[0] != '\0')
					strlcat(vpnc_clientlist_tmp, "<", sizeof(vpnc_clientlist_tmp));
				strlcat(vpnc_clientlist_tmp, word_tmp, sizeof(vpnc_clientlist_tmp));
				if(next)
					strlcat(vpnc_clientlist_tmp, next, sizeof(vpnc_clientlist_tmp));
				ret = HTTP_OK;
				break;
			}
			else
			{
				ret = HTTP_INVALID_INPUT;
				goto FINISH;
			}
			break;
		}
		if(vpnc_clientlist_tmp[0] != '\0')
			strlcat(vpnc_clientlist_tmp, "<", sizeof(vpnc_clientlist_tmp));
		strlcat(vpnc_clientlist_tmp, word_tmp, sizeof(vpnc_clientlist_tmp));
		find_idx++;
	}

	if(ret == HTTP_OK){
		nvram_set_int("vpnc_unit", find_idx);
		nvram_set("vpnc_clientlist", vpnc_clientlist_tmp);
		if(!strcmp(vpnc_enable, "0"))
			notify_rc("stop_vpnc");
		else
			notify_rc("restart_vpnc");
	}

FINISH:
	return ret;
#else
	return ASUSAPI_NOT_SUPPORT;
#endif
}

int
remove_client_in_group_list(int check_idx, char *pattern, char *group_list, int out_len){

	int ret = 0, group_idx = 0, group_next = 0;
	char *buf, *g;
	g = buf = strdup(group_list);
	char group_word[1024]={0}, word[1024]={0}, *word_next=NULL;

	foreach_60(word, g, word_next){
		get_string_in_62(word, check_idx, group_word, sizeof(group_word));
		if(!strcmp(group_word, pattern)){
			if(word_next){
				memmove(group_list+group_next, word_next, strlen(word_next));
					group_list[group_next+strlen(word_next)] = '\0';
			}else
				group_list[group_next] = '\0';

			ret = group_idx;
			break;
		}
		group_idx++;
		group_next = word_next-buf;
	}
	free(buf);
	return ret;
}

int
remove_client_in_list_by_idx(char *out, int idx, int out_len)
{
	int ret = -1, find_idx = 0, len = 0;
	char *buf, *g, *p;
	g = buf = strdup(out);

    len = strlen(out);

	while ((p = strchr(g, '<')) != NULL) {
		if(find_idx == idx)
		{
			if((int)(p-buf)-(int)(p-g) < out_len){
				out[(int)(p-buf)-(int)(p-g)-1] = '\0';
				strlcat(out+1, p, out_len);
				out[len - (p-g)-1] = '\0';
			}
			ret = find_idx;
			find_idx++;
			break;
		}
		else{
			g = p + 1;
			find_idx++;
		}
	}

	if(find_idx == idx){ // Consider the field is at the end of in_list (no '>' occur anymore).
        if((int)(g-buf) < out_len){
            out[(int)(g-buf)-1] = '\0';
        }
        ret = find_idx;
	}
	free(buf);

	return ret;
}

int delete_wireguard_client(int wgc_index)
{
#ifdef RTCONFIG_WIREGUARD
	int ret = HTTP_NO_CHANGE, del_vpnc_clientlist_idx = -1;
	char vpnc_clientlist[CKN_STR8192] = {0}, vpnc_pptp_options_x_list[CKN_STR2048] = {0}, wgc_index_str[8] = {0}, prefix[16] = {0};
	strlcpy(vpnc_clientlist, nvram_safe_get("vpnc_clientlist"), sizeof(vpnc_clientlist));
	strlcpy(vpnc_pptp_options_x_list, nvram_safe_get("vpnc_pptp_options_x_list"), sizeof(vpnc_pptp_options_x_list));
	snprintf(wgc_index_str, sizeof(wgc_index_str), "%d", wgc_index);

	if(enable_wireguard_client(wgc_index, "0") == HTTP_OK)
		sleep(3);

	del_vpnc_clientlist_idx = remove_client_in_group_list(2, wgc_index_str, vpnc_clientlist, sizeof(vpnc_clientlist));
	if(del_vpnc_clientlist_idx){
		remove_client_in_list_by_idx(vpnc_pptp_options_x_list, del_vpnc_clientlist_idx, sizeof(vpnc_pptp_options_x_list));
		nvram_set("vpnc_clientlist", vpnc_clientlist);
		nvram_set("vpnc_pptp_options_x_list", vpnc_pptp_options_x_list);

		snprintf(prefix, sizeof(prefix), "wgc%d_", wgc_index);
		nvram_pf_set(prefix, "use_tnl", "");
		nvram_pf_set(prefix, "ep_device_id", "");
		nvram_pf_set(prefix, "ep_area", "");

		httpd_nvram_commit();
		notify_rc("restart_vpnc");
		ret = HTTP_OK;
	}

	return ret;
#else
	return ASUSAPI_NOT_SUPPORT;
#endif
}

int get_wgc_connect_status(struct json_object *wgc_connect_status_obj){
#ifdef RTCONFIG_WIREGUARD
	char vpnc_clientlist[CKN_STR8192] = {0};
	char word[1024]={0}, word_tmp[1024] = {0}, *next = NULL;
	char vpn_name[16] = {0}, wgc_index[8] = {0}, ifname[8] = {0};

	strlcpy(vpnc_clientlist, nvram_safe_get("vpnc_clientlist"), sizeof(vpnc_clientlist));

	foreach_60(word, vpnc_clientlist, next){
		strlcpy(word_tmp, word, sizeof(word_tmp));
		get_string_in_62(word_tmp, 1, vpn_name, sizeof(vpn_name));
		get_string_in_62(word_tmp, 2, wgc_index, sizeof(wgc_index));
		if(!strcmp(vpn_name, "WireGuard")){
			snprintf(ifname, sizeof(ifname), "%s%s", WG_CLIENT_IF_PREFIX, wgc_index);
			if(is_wgc_connected(safe_atoi(wgc_index)))
				json_object_object_add(wgc_connect_status_obj, ifname, json_object_new_string("1"));
			else
				json_object_object_add(wgc_connect_status_obj, ifname, json_object_new_string("0"));
		}
	}

	return 0;
#else
	return ASUSAPI_NOT_SUPPORT;
#endif
}

int del_wgsc_list(int s_unit, int c_unit){
#ifdef RTCONFIG_WIREGUARD
	int ret = HTTP_NORULE_DEL, i = 0;
	char *nv = NULL;
	char prefix[16] = {0}, c_prefix[16] = {0}, tmp[16] = {0}, rc_command[64] = {0};

	const char *wgsc_nv_list[] = {"addr", "aips", "caips", "priv", "pub", "psk", "name", "caller", "extinfo", NULL};

	if((s_unit < 1 || s_unit > 2) || (c_unit < 1 || c_unit > 10))
		return HTTP_INVALID_INPUT;

	snprintf(prefix, sizeof(prefix), "%s%d_", WG_SERVER_NVRAM_PREFIX, s_unit);
	snprintf(c_prefix, sizeof(c_prefix), "%sc%d_", prefix, c_unit);

	nv = nvram_pf_safe_get(c_prefix, "enable");

	if(*nv == '1'){

		nvram_pf_set(c_prefix, "enable", "0");
		for(i=0; wgsc_nv_list[i]; i++){
			snprintf(tmp, sizeof(tmp), "%s%s", c_prefix, wgsc_nv_list[i]);
			nvram_unset(tmp);
		}

		snprintf(rc_command, sizeof(rc_command), "restart_wgsc %d %d", s_unit, c_unit);
		httpd_nvram_commit();
		notify_rc(rc_command);
		ret = HTTP_OK;
	}

	return ret;
#else
	return ASUSAPI_NOT_SUPPORT;
#endif
}

int get_wgsc_list(int s_unit, struct json_object *wgsc_list_array) {
#ifdef RTCONFIG_WIREGUARD
	int i = 0, c_unit = 0;
	char *nv = NULL, *nv_enable = NULL, *nv_name = NULL;
	char prefix[16] = {0}, c_prefix[16] = {0}, tmp[8] = {0};
	struct json_object *wgsc_nv_obj = NULL;

	const char *wgsc_nv_list[] = {"name", "enable", "addr", "aips", "caips", "caller", "extinfo", NULL};

	snprintf(prefix, sizeof(prefix), "%s%d_", WG_SERVER_NVRAM_PREFIX, s_unit);
	for (c_unit = 1; c_unit <= WG_SERVER_CLIENT_MAX; c_unit++)
	{
		wgsc_nv_obj = json_object_new_object();
		snprintf(c_prefix, sizeof(c_prefix), "%sc%d_", prefix, c_unit);
		snprintf(tmp, sizeof(tmp), "%d", c_unit);

		nv_enable = nvram_pf_safe_get(c_prefix, "enable");
		nv_name = nvram_pf_safe_get(c_prefix, "name");

		if(*nv_enable == '\0') continue;
		if(*nv_enable == '0' && *nv_name == '\0') continue;

		for(i=0; wgsc_nv_list[i]; i++){
			nv = nvram_pf_safe_get(c_prefix, wgsc_nv_list[i]);
			json_object_object_add(wgsc_nv_obj, wgsc_nv_list[i], json_object_new_string(nv));
		}
		json_object_object_add(wgsc_nv_obj, "index", json_object_new_string(tmp));
		json_object_array_add(wgsc_list_array, wgsc_nv_obj);
	}
#else
	return ASUSAPI_NOT_SUPPORT;
#endif
}
