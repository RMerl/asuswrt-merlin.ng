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
#else
#include <signal.h>
#include <time.h>
#include <httpd.h>
#endif
#ifdef RTCONFIG_CFGSYNC
#include <cfg_capability.h>
#include <cfg_param.h>
#include <cfg_slavelist.h>
#endif
#if defined(RTCONFIG_AMAS_NEWOB) || defined(RTCONFIG_CFGSYNC)
#include <cfg_event.h>
#include <cfg_onboarding.h>
#include <cfg_lib.h>
#endif
#ifdef RTCONFIG_HND_ROUTER_AX
#include <wlioctl.h>
#endif

#ifdef RTCONFIG_CFGSYNC
#define CFG_JSON_FILE           "/tmp/cfg.json"
#endif

#define sys_upload(image) eval("nvram", "restore", image)

void httpd_nvram_commit(void){

	/* 0:nvram 1:openvpn 2:ipsec 3:usericon */
	sync_profile_update_time(0);
}

/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
int
b64_decode( const char* str, unsigned char* space, int size )
{
    const char* cp;
    int space_idx, phase;
    int d, prev_d=0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[(int)*cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size )
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
}

/*
 * Get the value of an NVRAM variable
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
char*
nvram_get_x(const char *sid, const char *name)
{
	return (nvram_safe_get(name));
}


#ifndef RTCONFIG_BWDPI
int check_tcode_blacklist()
{
	return 0;
}

int dump_dpi_support(int index)
{
	return 0;
}
#endif

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

#ifdef RTCONFIG_CFGSYNC
#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
#ifdef RTCONFIG_AMAS_CAP_CONFIG
char *get_ft_name_by_ft_index(int index)
{
	struct subfeature_mapping_s *pFeature = NULL;
	char *ftName = NULL;

	for (pFeature = &subfeature_mapping_list[0]; pFeature->index != 0; pFeature++) {
		if (index == pFeature->index) {
			ftName = pFeature->name;
			break;
		}
	}

	return ftName;
}

int is_cap_private_cfg(char *param)
{
	struct param_mapping_s *pParam = &param_mapping_list[0];
	json_object *privFtArray = json_object_from_file(CAP_PRIVATE_FEATURE_FILE);
	json_object *ftEntry = NULL;
	int ret = 0, privFtLen = 0, i = 0;
	char *ftName = NULL;

	if (privFtArray) {
		privFtLen = json_object_array_length(privFtArray);

		for (pParam = &param_mapping_list[0]; pParam->param != NULL; pParam++) {
			if (strcmp(param, pParam->param) == 0) {
				ftName = get_ft_name_by_ft_index(pParam->subfeature);
				break;
			}
		}

		if (ftName) {
			for (i = 0; i < privFtLen; i++) {
				if ((ftEntry = json_object_array_get_idx(privFtArray, i))) {
					if (strcmp(ftName, json_object_get_string(ftEntry)) == 0) {
						ret = 1;
						break;
					}
				}
			}
		}

		json_object_put(privFtArray);
	}

	return ret;
}
#endif	/* RTCONFIG_AMAS_CAP_CONFIG */
#endif	/* RTCONFIG_AMAS_CENTRAL_CONTROL */

int is_cfg_server_ready()
{
	if (nvram_match("x_Setting", "1") &&
		pids("cfg_server") && check_if_file_exist(CFG_SERVER_PID))
		return 1;

	return 0;
}

int check_cfg_changed(json_object *root)
{
	json_object *paramObj = NULL;
	struct param_mapping_s *pParam = &param_mapping_list[0];

	if (!root)
		return 0;

	for (pParam = &param_mapping_list[0]; pParam->param != NULL; pParam++) {
		json_object_object_get_ex(root, pParam->param, &paramObj);
		if (paramObj)
			return 1;
	}

	return 0;
}

void notify_cfg_server(json_object *cfg_root, int check)
{
	char cfg_ver[9];
	int apply_lock = 0;

	if (is_cfg_server_ready()) {
		if ((check && check_cfg_changed(cfg_root)) || !check) {
			/* save the changed nvram parameters */
			apply_lock = file_lock(CFG_APPLY_LOCK);
			json_object_to_file(CFG_JSON_FILE, cfg_root);
			file_unlock(apply_lock);

			/* change cfg_ver when setting changed */
			srand(time(NULL));
			snprintf(cfg_ver, sizeof(cfg_ver), "%d%d", rand(), rand());
			nvram_set("cfg_ver", cfg_ver);

			/* trigger cfg_server to send notification */
			kill_pidfile_s(CFG_SERVER_PID, SIGUSR2);
		}
	}
}

int save_changed_param(json_object *cfg_root, char *param, const char *value)
{
	int ret = 0;
#if defined(RTCONFIG_AMAS_CENTRAL_CONTROL) && defined(RTCONFIG_AMAS_CAP_CONFIG)
	int param_is_private = is_cap_private_cfg(param);
#endif

	if (is_cfg_server_ready()){
		json_object *tmp = NULL;
		struct param_mapping_s *pParam = &param_mapping_list[0];

		json_object_object_get_ex(cfg_root, param, &tmp);
		if (tmp == NULL) {
			for (pParam = &param_mapping_list[0]; pParam->param != NULL; pParam++) {
				if (!strcmp(param, pParam->param)) {
#if defined(RTCONFIG_AMAS_CENTRAL_CONTROL) && defined(RTCONFIG_AMAS_CAP_CONFIG)
					if (param_is_private && value)
						json_object_object_add(cfg_root, param,
							json_object_new_string(value));
					else
#endif
					json_object_object_add(cfg_root, param,
						json_object_new_string(""));
					ret = 1;
					break;
				}
			}
		}
	}

	return ret;
}
#endif	/* RTCONFIG_CFGSYNC */

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
	char nv[64] = {0};
	char band_str[8] = {0}, word[256] = {0}, *next = NULL;
	char tmp[128] = {0}, prefix[] = "wlXXXXXXXXXX_";
	char wlnband_list[64] = {0};
#ifdef RTCONFIG_UAWIFI
	char wlnband_list_conv[64] = {0}, band_str_conv[8] = {0};
	int count2g_conv = 0, count5g_conv = 0, count6g_conv = 0;
#endif
#ifdef RTCONFIG_HND_ROUTER_AX
	int unit_real;
#endif

	foreach (word, nvram_safe_get("wl_ifnames"), next) {

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		snprintf(nv, sizeof(nv), "wl%d_nband_type", unit);
		band = nvram_get_int(strlcat_r(prefix, "nband", tmp, sizeof(tmp)));

		switch (band){
			case 1:
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
				if(nvram_get_int(nv) == 1 || nvram_get_int(nv) == 2)
					snprintf(band_str, sizeof(band_str), "5g1");
				else if(nvram_get_int(nv) == 3)
					snprintf(band_str, sizeof(band_str), "5g2");
				else
#endif
				{
#ifdef RTCONFIG_HND_ROUTER_AX
					if (!wl_ioctl(word, WLC_GET_INSTANCE, &unit_real, sizeof(unit_real))) {
						if (unit_real == WL_5G_BAND) {
							snprintf(band_str, sizeof(band_str), "5g1");
							count5g++;
						} else if (unit_real == WL_5G_2_BAND) {
							snprintf(band_str, sizeof(band_str), "5g2");
							count5g++;
						} else
							snprintf(band_str, sizeof(band_str), "5g%d", ++count5g);
					} else
#endif
					snprintf(band_str, sizeof(band_str), "5g%d", ++count5g);
				}
#ifdef RTCONFIG_UAWIFI
				snprintf(band_str_conv, sizeof(band_str_conv), "5g%d", ++count5g_conv);
#endif
				break;
			case 2:
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
				if(nvram_get_int(nv) == 0)
					snprintf(band_str, sizeof(band_str), "2g1");
				else
#endif
					snprintf(band_str, sizeof(band_str), "2g%d", ++count2g);
#ifdef RTCONFIG_UAWIFI
				snprintf(band_str_conv, sizeof(band_str_conv), "2g%d", ++count2g_conv);
#endif
				break;
			case 4:
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
				if(nvram_get_int(nv) == 4 || nvram_get_int(nv) == 5)
					snprintf(band_str, sizeof(band_str), "6g1");
				else if(nvram_get_int(nv) == 6)
					snprintf(band_str, sizeof(band_str), "6g2", ++count6g);
				else
#endif
				{
#if defined(RTCONFIG_HND_ROUTER_AX) && (defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_HAS_6G))
					if (!wl_ioctl(word, WLC_GET_INSTANCE, &unit_real, sizeof(unit_real))) {
						if (unit_real == WL_6G_BAND) {
							snprintf(band_str, sizeof(band_str), "6g1");
							count6g++;
#ifdef WL_6G_2_BAND
						} else if (unit_real == WL_6G_2_BAND) {
							snprintf(band_str, sizeof(band_str), "6g2");
							count6g++;
#endif
						} else
							snprintf(band_str, sizeof(band_str), "6g%d", ++count6g);
					} else
#endif

					snprintf(band_str, sizeof(band_str), "6g%d", ++count6g);
				}
#ifdef RTCONFIG_UAWIFI
				snprintf(band_str_conv, sizeof(band_str_conv), "6g%d", ++count6g_conv);
#endif
				break;
		}

#ifdef RTCONFIG_UAWIFI
		if(strncmp(band_str, "6g", 2)){
#endif

		if(wlnband_list[0] != '\0')
			strlcat(wlnband_list, "<", sizeof(wlnband_list));

		strlcat(wlnband_list, band_str, sizeof(wlnband_list));
#ifdef RTCONFIG_UAWIFI
		}
#endif


#ifdef RTCONFIG_UAWIFI
		if(wlnband_list_conv[0] != '\0')
			strlcat(wlnband_list_conv, "<", sizeof(wlnband_list_conv));
		strlcat(wlnband_list_conv, band_str_conv, sizeof(wlnband_list_conv));
#endif

		unit++;
	}

	nvram_set("wlnband_list", wlnband_list);
#ifdef RTCONFIG_UAWIFI
	nvram_set("wlnband_list_conv", wlnband_list_conv);
#endif

	return ret;
}


char *wl_nband_to_wlx(char *nv_name, char *wl_name, size_t len){

	int i = 0;
	char prefix[8] = {0}, tmp[32] = {0};
	char wlnband_list[64] = {0};
	char word[64] = {0}, *next = NULL;

#ifdef RTCONFIG_UAWIFI
	strlcpy(wlnband_list, nvram_safe_get("wlnband_list_conv"), sizeof(wlnband_list));
#else
	strlcpy(wlnband_list, nvram_safe_get("wlnband_list"), sizeof(wlnband_list));
#endif

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
		"Employee", "Sched",
#else
		"Kids",
#endif
#if !defined(RTCONFIG_SMART_HOME_MASTER_UI)
		"Customized",
#if defined(RTCONFIG_CAPTIVE_PORTAL) && defined(RTCONFIG_CP_FREEWIFI)
		"Portal",
#endif
#endif
#if defined(RTCONFIG_MLO)
		"MLO",
#endif
#if defined(RTCONFIG_OPEN_NAT) && defined(RTCONFIG_SDN_PRIORITY)
		"Gaming",
#endif
#if defined(RTCONFIG_MULTILAN_MWL)
		"MAINFH",
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
#if defined(GTAXE11000) || defined(GTAC2900) || defined(GTAX11000) \
	|| defined(RTCONFIG_BCMLEDG) \
	|| defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX6000) || defined(GTAXE16000) \
	|| defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX11000_PRO) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(TUFAX6000) \
	|| defined(GS7) || defined(GTBE96) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(GTBE96_AI)
	{"AuraRGB", "light_effect/light_effect.html", "<rt><white>light_effect/light_effect_white.css"},
	{"AuraRGB_preview", "light_effect/light_effect_pre.html", "<rt>"},
#endif
	{"Tencent", "game_accelerator_tencent.html", "<rt>"},
#if defined(RTCONFIG_OOKLA) || defined(RTCONFIG_OOKLA_LITE)
	{"SpeedTest", "internet_speed.html", "<default><white>css/internetSpeed_white_theme.css"},
#endif
#if defined(RTCONFIG_BWDPI) || defined(RTCONFIG_HNS)
	{"AiProtection_MALS", "AiProtection_MaliciousSitesBlocking_m.asp", "<rt>"},
	{"AiProtection_VP", "AiProtection_IntrusionPreventionSystem_m.asp", "<rt>"},
	{"AiProtection_CC", "AiProtection_InfectedDevicePreventBlock_m.asp", "<rt>"},
#endif
	{"VPN_Fusion", "VPN/vpnc.html", "<rt><gt>VPN/vpncGT.css<tuf>VPN/vpncTUF.css<white>VPN/vpncWHITE.css"},
	{"VPN_Server", "VPN/vpns.html", "<rt><gt>VPN/vpnsGT.css<tuf>VPN/vpnsTUF.css<white>VPN/vpnsWHITE.css"},
#ifdef RTCONFIG_MULTILAN_CFG
	{"SDN", "SDN/sdn.html", "<rt><gt>SDN/sdn_ROG.css<tuf>SDN/sdn_TUF.css<white>SDN/sdn_WHITE.css"},
#endif
#ifdef RTCONFIG_DASHBOARD
	{"Dashboard", "index.html?url=dashboard", "<white>css/business-white.css"},
#endif
#ifdef RTCONFIG_SW_BTN
	{"MultiFuncBtn", "multifuncbtn/mfb.html", "<rt><white>multifuncbtn/mfb_WHITE.css"},
#endif
#ifdef RTCONFIG_ADGUARDDNS
	{"AdGuard DNS", "adguard_dns.html", "<rt><gt>css/adguard_customize_ROG.css<white>css/adguard_dns_WHITE.css"},
#endif
	{NULL, NULL, NULL}
};

int get_rwd_table(struct json_object *rwd_mapping)
{
	int white_theme_status = 0;
	char *url = NULL;
	char *theme_support = NULL, *theme_url = NULL;
	char url_file[128] = {0}, check_path[128] = {0}, theme_name[32] = {0};
	char theme_list[2048] = {0}, word[256]={0}, *word_next = NULL;
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

		function_obj = json_object_new_object();
		json_object_object_add(function_obj, "path", json_object_new_string(p->path));

		strlcpy(theme_list, p->theme_list, sizeof(theme_list));

		foreach_60(word, theme_list, word_next){

			if ((vstrsep(word, ">", &theme_support, &theme_url) != 2))
				continue;

			if(*theme_url != '\0'){
				snprintf(check_path, sizeof(check_path), "/www/%s", theme_url);
				if(!check_if_file_exist(check_path)){
					dbg("check_path[%s] not found\n", check_path);
					continue;
				}
			}
			snprintf(theme_name, sizeof(theme_name), "%s_theme", theme_support);
			json_object_object_add(function_obj, theme_name, json_object_new_string("1"));
		}

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

				if((int)(p-buf)-(int)(p-g)-1 > 0){
					out[(int)(p-buf)-(int)(p-g)-1] = '\0';
					strlcat(out+1, p, out_len);
				}else
					strlcpy(out, p, out_len);

				if((p-g)!=0)
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

		if((int)(g-buf) < out_len)
			out[(int)(g-buf)-1] = '\0';

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
	if(del_vpnc_clientlist_idx != -1){
		remove_client_in_list_by_idx(vpnc_pptp_options_x_list, del_vpnc_clientlist_idx+1, sizeof(vpnc_pptp_options_x_list));
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
	char vpn_caller[8] = {0}, wgc_enable[2] = {0}, vpn_proto[16] = {0}, wgc_index[8] = {0}, ifname[8] = {0};

	strlcpy(vpnc_clientlist, nvram_safe_get("vpnc_clientlist"), sizeof(vpnc_clientlist));

	foreach_60(word, vpnc_clientlist, next){
		strlcpy(word_tmp, word, sizeof(word_tmp));
		get_string_in_62(word_tmp, 1, vpn_proto, sizeof(vpn_proto));
		get_string_in_62(word_tmp, 2, wgc_index, sizeof(wgc_index));
		get_string_in_62(word_tmp, 5, wgc_enable, sizeof(wgc_enable));
		get_string_in_62(word_tmp, 11, vpn_caller, sizeof(vpn_caller));
		if(!strcmp(vpn_proto, "WireGuard")){
			snprintf(ifname, sizeof(ifname), "%s%s", WG_CLIENT_IF_PREFIX, wgc_index);

			struct json_object *wgc_obj = json_object_new_object();
			json_object_object_add(wgc_obj, "caller", json_object_new_string(vpn_caller));
			json_object_object_add(wgc_obj, "enable", json_object_new_string(wgc_enable));

			if(is_wgc_connected(safe_atoi(wgc_index)))
				json_object_object_add(wgc_obj, "connected", json_object_new_string("1"));
			else
				json_object_object_add(wgc_obj, "connected", json_object_new_string("0"));

			json_object_object_add(wgc_connect_status_obj, ifname, wgc_obj);
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
	return 0;
#else
	return ASUSAPI_NOT_SUPPORT;
#endif
}

int set_app_mnt(char *app_mnt)
{
	int ret = HTTP_OK;
	char *p = NULL;

	time_t now = time(NULL);

	if(app_mnt && *app_mnt != '\0' && strlen(app_mnt) < 9){
		for (p = app_mnt; *p != '\0'; ++p) {
			if(!isdigit(*p)){
				ret = HTTP_INVALID_INPUT;
				goto FINISH;
			}
		}
		nvram_set("app_mnt", app_mnt);
		nvram_set_int("app_mnt_ts", now);
		httpd_nvram_commit();
	}

FINISH:
	return ret;
}

int get_app_mnt(struct json_object *app_mnt_obj)
{
	if(json_object_get_type(app_mnt_obj) == json_type_object){
		json_object_object_add(app_mnt_obj, "app_mnt", json_object_new_string(nvram_safe_get("app_mnt")));
		json_object_object_add(app_mnt_obj, "app_mnt_ts", json_object_new_string(nvram_safe_get("app_mnt_ts")));
	}
	return 0;
}

struct REPLACE_PRODUCTID_S replace_productid_t[] =
{
	{"LYRA_VOICE", "LYRA VOICE", "global"},
	{"RT-AC57U_V2", "RT-AC57U V2", "global"},
	{"RT-AC58U_V2", "RT-AC58U V2", "global"},
	{"RT-AC1300G_PLUS_V2", "RT-AC1300G PLUS V2", "global"},
	{"RT-AC1500G_PLUS", "RT-AC1500G PLUS", "global"},
	{"ZenWiFi_CT8", "ZenWiFi AC", "global"},
	{"ZenWiFi_CT8", "灵耀AC3000", "CN"},
	{"ZenWiFi_XT8", "ZenWiFi AX", "global"},
	{"ZenWiFi_XT8", "灵耀AX6600", "CN"},
	{"ZenWiFi_XD4", "ZenWiFi AX Mini", "global"},
	{"ZenWiFi_XD4", "灵耀AX魔方", "CN"},
	{"ZenWiFi_CD6R", "ZenWiFi AC Mini", "global"},
	{"ZenWiFi_CD6N", "ZenWiFi AC Mini", "global"},
	{"ZenWiFi_XP4", "ZenWiFi AX Hybrid", "global"},
	{"ZenWiFi_XP4", "灵耀AX XP4", "CN"},
	{"ZenWiFi_CV4", "ZenWiFi Voice", "global"},
	{"ZenWiFi_Pro_XT12", "灵耀Pro AX11000", "CN"},
	{"ZenWiFi_XD4_Pro", "灵耀AX魔方Pro", "CN"},
	{"ZenWiFi_XT9", "灵耀AX7800", "CN"},
	{"ZenWiFi_XD6", "灵耀AX5400", "CN"},
	{"TUF-AX3000_V2", "TUF GAMING 小旋风", "CN"},
	{"GT6", "ROG魔方 • 幻", "CN"},
	{"TUF-AX4200Q", "TUF GAMING 小旋风 Pro", "CN"},
	{"TUF-AX4200", 	"TUF GAMING AX4200", "global"},
	{"TX-AX6000", "天选游戏路由", "CN"},
	{"TUF-AX6000",  "TUF GAMING AX6000", "global"},
	{"GT-BE96",  "ROG 八爪鱼7", "CN"},
	{"TUF-BE3600", "TUF GAMING 小旋风 WiFi7", "CN"},
	{"TUF-BE6500", "TUF GAMING 小旋风 Pro WiFi7", "CN"},
	{"TUF_3600", "TUF GAMING 小旋风 WiFi7", "CN"},
	{"TUF_6500", "TUF GAMING 小旋风 Pro WiFi7", "CN"},
	{"ZenWiFi_BD4", "灵耀魔方 WiFi7 BE3600", "CN"},
	{"ZenWiFi_BD4_Outdoor", "灵耀魔方 无界", "CN"},
	{"GS7", "ROG 魔盒", "CN"},
	{"GS7_Pro", "ROG 魔盒 Pro", "CN"},
	{"RP-BE58", "小飞侠组网超人 WiFi7", "CN"},
	{"TUF_3600_V2", "TUF GAMING 小旋风 V2 WiFi7", "CN"},
	{NULL, NULL, NULL}
};

void replace_productid(char *GET_PID_STR, char *RP_PID_STR, int len){

	struct REPLACE_PRODUCTID_S *p;
	char *p_temp;

	for(p = &replace_productid_t[0]; p->org_name; p++){
		if(!strcmp(GET_PID_STR, p->org_name)){
			if(!strncmp(nvram_safe_get("preferred_lang"), p->p_lang, 2))
				strlcpy(RP_PID_STR, p->replace_name, len);

			if(!strcmp("global", p->p_lang) && !strlen(RP_PID_STR))
				strlcpy(RP_PID_STR, p->replace_name, len);
		}
	}

	if(!strcmp(GET_PID_STR,"RT-AX57M") && !strcmp(nvram_safe_get("preferred_lang"), "CN")){
		if(!strcmp(nvram_safe_get("CoBrand"), "5"))
			strlcpy(RP_PID_STR, "RT-AX57 青春版", len);
		else
			strlcpy(RP_PID_STR, "RT-AX57 热血版", len);
	}
	else if(!strcmp(GET_PID_STR, "RT-BE57") && !strncmp(nvram_safe_get("preferred_lang"), "CN", 2) && nvram_get_int("CoBrand") == 12){
		strlcpy(RP_PID_STR, "天选游戏路由2 初音未来版", len);
	}
	else if(!strcmp(GET_PID_STR, "RT-BE57") && !strncmp(nvram_safe_get("preferred_lang"), "CN", 2) && nvram_get_int("CoBrand") == 11){
		strlcpy(RP_PID_STR, "天选游戏路由2", len);
	}
	else if(!strcmp(GET_PID_STR, "GS7") && !strncmp(nvram_safe_get("preferred_lang"), "CN", 2) && nvram_get_int("CoBrand") == 18){
		strlcpy(RP_PID_STR, "ROG 魔盒 初音未来版", len);
	}
	else if(!strcmp(GET_PID_STR, "GS7") && !strncmp(nvram_safe_get("preferred_lang"), "CN", 2) && nvram_get_int("CoBrand") == 17){
		strlcpy(RP_PID_STR, "ROG 魔盒 透视版", len);
	}
	else if(!strlen(RP_PID_STR)){
		if((p_temp = strstr(GET_PID_STR, "ZenWiFi_")) && !strncmp(nvram_safe_get("preferred_lang"), "CN", 2)){
			p_temp += strlen("ZenWiFi_");
			snprintf(RP_PID_STR, len, "灵耀%s", p_temp);
		}
		else
			strlcpy(RP_PID_STR, GET_PID_STR, len);

		/* general replace underscore with space */
		for (; *RP_PID_STR; ++RP_PID_STR)
		{
			if (*RP_PID_STR == '_')
				*RP_PID_STR = ' ';
		}
	}
}

#ifdef RTCONFIG_AMAS
int do_reboot_action(char *device_list)
{
	int ret = HTTP_OK;
	char word[32] = {0}, *next = NULL;
	char event_msg[1024] = {0}, mac_list[512] = {0};
	struct json_object *mac_array = json_object_new_array();

	if(device_list)
		strlcpy(mac_list, device_list, sizeof(mac_list));

	if(!strcmp(mac_list, "DUT")){
		notify_rc("reboot");
		goto FINISH;
	}

	foreach_44(word, mac_list, next) {
		if(!isValidMacAddress(word)){
			ret = HTTP_INVALID_MAC;
			goto FINISH;
		}
		json_object_array_add(mac_array, json_object_new_string(word));
	}

	snprintf(event_msg, sizeof(event_msg), HTTPD_ACTION_MSG,
		EID_HTTPD_REBOOT, json_object_to_json_string(mac_array), "{}");

	dbg("event_msg = [%s]\n", event_msg);

	if (strlen(event_msg))
		send_cfgmnt_event(event_msg);

FINISH:
	if(mac_array)
		json_object_put(mac_array);

	return ret;
}
#endif

/*
 * Check firmware update status
 * @return	0:No need upgrade 1:need upgrade 2:need Force upgrade 3:Fail to retrieve firmware
 */
int check_firmware_update_status(void)
{
	int count = 60;

	while(count-- > 0){
		sleep(1);
#ifdef RTCONFIG_CFGSYNC
		if(is_cfg_server_ready()){
			int cfg_check = nvram_get_int("cfg_check");

			if(cfg_check == 0 || cfg_check == 1 || cfg_check == 5)
				continue;
			else if(cfg_check == 2 || cfg_check == 3)
				return 3;
			else if(cfg_check == 9)
				return 0;
			else if(cfg_check == 7)
				return 1;
		}
		else
#endif
		{
			if(nvram_get_int("webs_state_update") == 0)
				continue;
			else{
				if(nvram_get_int("webs_state_error") == 1)
					return 3;
				else
					return nvram_get_int("webs_state_flag");
			}
		}
	}
	return 3;
}

/*
 * Do firmware update to FRS server
 * @param	from_id		trigger service
 * @param	wait_result	0: do not wait 1:watting
 * @return	0:No need upgrade 1:need upgrade 2:need Force upgrade 3:Fail to retrieve firmware
 */
int do_firmware_check(int from_id, int wait_result)
{
	int ret = 0;

	if(from_id == FROM_BROWSER)
		nvram_set("webs_update_trigger", "GUI_CFG");
	else if(from_id == FROM_ASUSROUTER)
		nvram_set("webs_update_trigger", "APP_CFG");
	else
		nvram_set("webs_update_trigger", "AWSIOT_CFG");

#ifdef RTCONFIG_CFGSYNC
	char event_msg[64] = {0};
	snprintf(event_msg, sizeof(event_msg), HTTPD_GENERIC_MSG, EID_HTTPD_FW_CHECK);
	if(is_cfg_server_ready())
	{
		if (strlen(event_msg))
			send_cfgmnt_event(event_msg);
	}
	else
#endif
		notify_rc("start_webs_update");

	if(wait_result)
		ret = check_firmware_update_status();

	return ret;
}

int do_firmware_upgrade(void)
{
#ifdef RTCONFIG_CFGSYNC
	char event_msg[64] = {0};

	snprintf(event_msg, sizeof(event_msg), HTTPD_GENERIC_MSG, EID_HTTPD_FW_UPGRADE);

	if (strlen(event_msg))
			send_cfgmnt_event(event_msg);
#else
	notify_rc("start_webs_upgrade");
#endif
	return 0;
}

int do_upload_config(void)
{
	int ret = 0;
#if defined(RTCONFIG_NVSW_IN_JFFS)
	int nvswid_bk = nvram_get_int("nvswid");
	int upload_nvswid;
#endif
#if defined(RTCONFIG_SAVEJFFS)
	ret = restore_jffs_cfgs("/tmp/settings_u.prf");
	dbg("Restore jffs cfgs to /jffs, return %d\n", ret);
	logmessage("savejffs", "Restore jffs cfgs to /jffs. (return %d)", ret);
#endif
	sys_upload("/tmp/settings_u.prf");
#if defined(RTCONFIG_NVSW_IN_JFFS)
	upload_nvswid = nvram_get_int("nvswid");
	if (nvswid_bk != upload_nvswid) // won't be changed if not in defaults.c, just in case
		nvram_set_int("nvswid", nvswid_bk);
#endif
#ifdef RTCONFIG_LANTIQ
	system("rm -f /jffs/db_*.tgz");
#endif
#ifdef RTCONFIG_LANTIQ
	system("rm -f /jffs/db_*.tgz");
#endif

#ifdef RTCONFIG_NVRAM_ENCRYPT
	start_enc_nvram();
#endif
	httpd_nvram_commit();

	sys_reboot();

	return ret;
}
