#include <obd.h>
#if defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X)
#include <linux/compiler.h>
#endif
#include <wireless.h>
#include <netinet/ether.h>

#if defined(RTCONFIG_AMAS)

static void obd_clear_all_probe_req_vsie(int unit);

int obd_init()
{
	if(!nvram_match("x_Setting", "1")) { /* default and need sta */
		const char *staifname;
		staifname = get_staifname(0);
		create_tmp_sta(0, staifname);
	}
	return 0;
}

void obd_final()
{
#if defined(RTCONFIG_LP5523)
	lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(MAPAC1750)
#endif
	nvram_set("prelink_pap_status", "-1");
	nvram_unset("wps_enrollee");
	nvram_unset("wps_e_success");
	nvram_unset("wps_success");
	nvram_commit();
	obd_clear_all_probe_req_vsie(0);
}

void obd_save_para()
{
	nvram_set("sw_mode", "3");
	nvram_set("wlc_psta", "2");
	nvram_set("wlc_dpsta", "2");
	nvram_set("lan_proto", "dhcp");
	nvram_set("lan_dnsenable_x", "1");
	nvram_set("x_Setting", "1");
	nvram_set("w_Setting", "1");
	nvram_set("re_mode", "1");
	nvram_unset("cfg_group");

#ifdef RTCONFIG_WIFI_SON
	nvram_set("wifison_ready", "0");
#endif	/* RTCONFIG_WIFI_SON */

	nvram_unset("wps_e_success");
	nvram_unset("wps_success");
	nvram_unset("wps_enrollee");

	nvram_commit();
}

void obd_start_active_scan()
{
	OBD_DBG("Send probe-req \n\n");
	set_wpa_cli_cmd(0, "scan", 1);
}

#define SCAN_WLIST "/tmp/scanned_iwlist"
typedef void (* iwlist_item_cb_func) (void *, int, char *);

const char *key_str[]={" - Address: ", "  Signal level=", "Extra:aimesh="};
struct odb_iwlist_cb_data {
	int got[ARRAY_SIZE(key_str)];
	int item_cnt;
	struct scanned_bss *bss_tail;
	struct scanned_bss *bss_temp;
	int bss_cnt;
};

void obd_iwlist_cb(void *priv, int index, char *str)
{
	struct odb_iwlist_cb_data *cb_data = priv;
	struct scanned_bss *bss = cb_data->bss_temp;

	if(bss == NULL)
		return;

	switch(index) {
		case -1:	/* finish one */
			if(cb_data->item_cnt == ARRAY_SIZE(key_str)) {
				if(cb_data->bss_tail)
					cb_data->bss_tail->next = bss;
				cb_data->bss_tail = bss;

				if((bss = malloc(sizeof(struct scanned_bss))) == NULL) {
					cprintf("%s ERROR malloc !!\n", __func__);
				}
				cb_data->bss_temp = bss;
				cb_data->bss_cnt++;
			}
			/* reset */
			memset(cb_data->got, 0, sizeof(cb_data->got));
			cb_data->item_cnt = 0;
			if(bss)
				memset((void *)bss, 0, sizeof(struct scanned_bss));
			return;

		case 0:	/* " - Address: " */
			if(cb_data->got[index])
				return;
			ether_aton_r(str, &bss->BSSID);
			break;

		case 1: /* "Signel level=" */
			if(cb_data->got[index])
				return;
			bss->RSSI = atoi(str);
			break;

		case 2: /* "Extra:aimesh=" */
			if(cb_data->got[index])
				return;
			{
				int ret;
				int vsie_len = strlen(str);
#define VSIE_ASUS_LEN	(2+3)	// dd25 f843e4 ...
				if((vsie_len & 1) || (vsie_len <= (VSIE_ASUS_LEN << 1))) {
					OBD_DBG("invalid hex str(%s)\n", str);
					return;
				}
				str      += (VSIE_ASUS_LEN << 1);	/* skip vsie and ASUS OUI */
				vsie_len -= (VSIE_ASUS_LEN << 1);
				vsie_len /= 2;				/* str len -> hex len */

				extern int str2hex(const char *str, unsigned char *data, size_t size);
				ret = str2hex(str, bss->vsie, vsie_len);
				if(ret != vsie_len) {
					OBD_DBG("str2hex fail ret(%d) vsie_len(%d) str(%s)\n", ret, vsie_len, str);
					return;
				}
				bss->vsie_len = vsie_len;
			}
			break;

		default:
			OBD_DBG("unhandle data (%d)\n", index);
			return;
	}
	cb_data->got[index] = 1;
	cb_data->item_cnt++;
}

int parse_iwlist_scan(const char *filename, int num, const char **key_str, iwlist_item_cb_func iwlist_cb, void *priv)
{
	FILE *fp;
	int item_cnt;
	int apCount;
	int i;
	char prefix_header[32];
	char line[1024];
	int size = sizeof(line)-1;
	char *key;

	if (!(fp= fopen(filename, "r")))
		return -1;

	line[size] = '\0';	// string end
	if(fgets(line, size, fp) && strstr(line, "Scan completed") == NULL) {
		fclose(fp);
		return -1;
	}
	/* init */
	item_cnt=0;
	apCount=0;
	snprintf(prefix_header, sizeof(prefix_header), "Cell %02d - Address:",apCount+1);

	while(fgets(line, size, fp)) {
		int len = strlen(line);

		while(len > 0 && (line[len -1] == '\r' || line[len -1] == '\n'))
			line[--len] = '\0';
		if(len <= 0)
			continue;

		if(strstr(line, prefix_header)) { /* Got new one */
			if(item_cnt > 0) {
				/* handle */
				iwlist_cb(priv, -1, NULL);
			}

			/* reset */
			item_cnt=0;
			apCount++;
			snprintf(prefix_header, sizeof(prefix_header), "Cell %02d - Address:",apCount+1);
		}

		for(i=0; i<num; i++) {
			if((key = strstr(line, key_str[i]))) {
				iwlist_cb(priv, i, key + strlen(key_str[i]));
				item_cnt++;
				break;
			}
		}
	}
	/* handle */
	if(item_cnt > 0) {
		iwlist_cb(priv, -1, NULL);
	}

	fclose(fp);
	return apCount;
}

#if 0
struct scanned_bss *obd_get_bss_scan_result()
{
	char *iwlist_argv[] = { "iwlist", NULL, "scanning", NULL };
	int ret;
	struct odb_iwlist_cb_data cb_data;
	struct scanned_bss *bss_list;
	int try;

	bss_list = malloc(sizeof(struct scanned_bss));
	if(bss_list == NULL)
		return NULL;

	memset(&cb_data, 0, sizeof(cb_data));
	cb_data.bss_temp = bss_list;
	iwlist_argv[1] = get_staifname(0);	// sta0

	for(try = 3; try > 0 ; try--) {
		_eval(iwlist_argv, ">"SCAN_WLIST, 0, NULL);
		ret = parse_iwlist_scan(SCAN_WLIST, ARRAY_SIZE(key_str), key_str, obd_iwlist_cb, (void *) &cb_data);
		if(ret > 0)
			break;
		sleep(1);
	}

	OBD_DBG("# bss_cnt(%d) #\n", cb_data.bss_cnt);
	free(cb_data.bss_temp);
	if(bss_list == cb_data.bss_temp) {
		return NULL;
	}

	return bss_list;
}
#else
struct scanned_bss *obd_get_bss_scan_result()
{
	char line[1024];
	char ctrl_sk[32];
	int unit;
	char *staifname;
	FILE *fp;
	struct scanned_bss *bss_list = NULL;
	int ret;

	obd_start_active_scan();	/* scan before get scan_results */

	unit = 0;
	get_wpa_ctrl_sk(unit, ctrl_sk, sizeof(ctrl_sk));
	staifname = get_staifname(unit);

	snprintf(line, sizeof(line), "/usr/bin/wpa_cli -p %s -i %s scan_results", ctrl_sk, staifname);
	if ((fp = popen(line, "r")) != NULL) {
		struct scanned_bss *bss;
		char *mac, *freq, *signal, *proto, *ssid, *aimesh;
		char *p;
		int RSSI;
		struct ether_addr *BSSID;
		int vsie_len;
		unsigned char vsie[512];

		freq = NULL;
		while(fgets(line, sizeof(line)-1, fp) != NULL) {
			strip_new_line(line);

			p = line;
			if((mac = strsep(&p, "\t")) == NULL || *mac == '\0' || (BSSID = ether_aton(mac)) == NULL ) {
				if(freq != NULL)
					_dprintf("# INVALID MAC #\n");
				continue;
			}
			if((freq = strsep(&p, "\t")) == NULL) {
				_dprintf("# NO FREQ #\n");
				continue;
			}
			if((signal = strsep(&p, "\t")) == NULL || !isdigit(*signal) || (RSSI = atoi(signal)) < 0) {
				_dprintf("# INVALID SIGNAL #\n");
				continue;
			}
			if((proto = strsep(&p, "\t")) == NULL) {
				_dprintf("# NO PROTO #\n");
				continue;
			}
			if((ssid = strsep(&p, "\t")) == NULL) {
				_dprintf("# NO SSID #\n");
				continue;
			}
			if((aimesh = strsep(&p, "\t")) == NULL || *aimesh == '\0' || strncmp(aimesh, "#f832e4", 7) != NULL) {
				//_dprintf("# NO AIMESH #\n");
				continue;
			}

			aimesh += 7;
			vsie_len = strlen(aimesh);
			vsie_len >>= 1;

			extern int str2hex(const char *str, unsigned char *data, size_t size);
			ret = str2hex(aimesh, vsie, vsie_len);
			if(ret != vsie_len) {
				OBD_DBG("str2hex fail ret(%d) vsie_len(%d) aimesh(%s)\n", ret, vsie_len, aimesh);
				continue;
			}

			if((bss = malloc(sizeof(struct scanned_bss))) == NULL)
				continue;
			memset(bss, 0, sizeof(struct scanned_bss));
			bss->vsie_len = vsie_len;
			memcpy(bss->vsie, vsie, bss->vsie_len);
			bss->RSSI = (unsigned char) RSSI;
			memcpy(&bss->BSSID, BSSID, sizeof(struct ether_addr));
			bss->next = bss_list;
			bss_list = bss;
		}
		pclose(fp);
	}
	return bss_list;
}
#endif

void obd_start_wps_enrollee()
{
	nvram_set("wps_enrollee", "1");
	nvram_unset("wps_e_success");
	nvram_unset("wps_success");

#if defined(RTCONFIG_LP5523)
	lp55xx_leds_proc(LP55XX_GREENERY_LEDS, LP55XX_WPS_PARAM_SYNC);
#elif defined(MAPAC1750)
	set_rgbled(RGBLED_BLUE_3ON1OFF);
#endif
	void start_wsc_enrollee_band(int band);
	start_wsc_enrollee_band(0);	/* Only active WPS on band 0 */
}

void obd_clear_all_probe_req_vsie(int unit)
{
	char cmd[300];
	int pktflag = 0xE;
	char *ifname;
	FILE *fp;
	char ctrl_sk[32];
	
	ifname = get_staifname(unit);
	get_wpa_ctrl_sk(unit, ctrl_sk, sizeof(ctrl_sk));

	snprintf(cmd, sizeof(cmd), "/usr/bin/wpa_cli -p %s -i %s vendor_elem_get %d", ctrl_sk, ifname, pktflag);
	if ((fp = popen(cmd, "r")) != NULL) {
		char vendor_elem[MAX_VSIE_LEN];
		vendor_elem[sizeof(vendor_elem)-1] = '\0';
		while (fgets(vendor_elem , sizeof(vendor_elem)-1 , fp) != NULL) {
			if(strlen(vendor_elem) > 0) {
				snprintf(cmd, sizeof(cmd), "/usr/bin/wpa_cli -p %s -i %s vendor_elem_remove %d %s", ctrl_sk,
					ifname, pktflag, vendor_elem);
				OBD_DBG("cmd=%s\n", cmd);
				system(cmd);
			}
		}
		pclose(fp);
	}
}

void obd_add_probe_req_vsie(int unit, int len, unsigned char *ie_data)
{
	// 13 : Associatino Request
	// 14 : Probe Reqeust
	// 15 : Authentication Request
	char cmd[300];
	char hexdata[256];
	int pktflag = 0xE;
	int i, ie_len = (len - OUI_LEN);
	char *ifname;
	char ctrl_sk[32];

	cprintf("## %s unit(%d) len(%d) ##\n", __func__, unit, len);
	ifname = get_staifname(unit);
	get_wpa_ctrl_sk(unit, ctrl_sk, sizeof(ctrl_sk));

	for (i = 0; i < ie_len; i++)
		sprintf(&hexdata[2 * i], "%02x", ie_data[i]);
	hexdata[2 * ie_len] = 0;

	obd_clear_all_probe_req_vsie(unit);

	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "/usr/bin/wpa_cli -p %s -i %s vendor_elem_add %d DD%02X%02X%02X%02X%s > /tmp/cmd_add", ctrl_sk, 
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0],  (uint8_t)OUI_ASUS[1],  (uint8_t)OUI_ASUS[2], hexdata);
		OBD_DBG("cmd=%s\n", cmd);
		system(cmd);
	}
}

void obd_del_probe_req_vsie(int unit, int len, unsigned char *ie_data)
{
	char cmd[300];
	char hexdata[256];
	int pktflag = 0xE;
	int i, ie_len = (len - OUI_LEN);
	char *ifname;
	char ctrl_sk[32];

	cprintf("## %s unit(%d) len(%d) ##\n", __func__, unit, len);
	ifname = get_staifname(unit);
	get_wpa_ctrl_sk(unit, ctrl_sk, sizeof(ctrl_sk));

	for (i = 0; i < ie_len; i++)
		sprintf(&hexdata[2 * i], "%02x", ie_data[i]);
	hexdata[2 * ie_len] = 0;

	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "/usr/bin/wpa_cli -p %s -i %s vendor_elem_remove %d DD%02X%02X%02X%02X%s", ctrl_sk, 
			ifname, pktflag, (uint8_t)len,  (uint8_t)OUI_ASUS[0],  (uint8_t)OUI_ASUS[1],  (uint8_t)OUI_ASUS[2], hexdata);
		OBD_DBG("cmd=%s\n", cmd);
		system(cmd);
	}
}

void obd_led_blink()
{
#if defined(RTCONFIG_LP5523)
	lp55xx_leds_proc(LP55XX_GREENERY_LEDS, LP55XX_WPS_PARAM_SYNC);
#elif defined(MAPAC1750)
	set_rgbled(RGBLED_BLUE_3ON1OFF);
#endif
}

void obd_led_off()
{
#if defined(RTCONFIG_LP5523)
	lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(MAPAC1750)
	nvram_set("prelink_pap_status", "-1");
#endif
}
#endif //#if defined(RTCONFIG_AMAS)
