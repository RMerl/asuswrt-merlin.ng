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

#include <shutils.h>
#include <shared.h>
#include <qca.h>

#ifdef RTCONFIG_AMAS
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

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		return;
#endif
	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	ifname = get_wififname(0);	// TODO: Should we get the band from nvram?

	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli set_vsie -i%s %d DD%02X%02X%02X%02X%s",
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0], (uint8_t)OUI_ASUS[1], (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
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

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		return;
#endif
	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	ifname = get_wififname(0);	// TODO: Should we get the band from nvram?

	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli del_vsie -i%s %d DD%02X%02X%02X%02X%s",
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0], (uint8_t)OUI_ASUS[1], (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
	}
}

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
	int pid;
	pid = getpid();

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

void Pty_start_wlc_connect(int band)
{
	band = swap_5g_band(band);
	set_wpa_cli_cmd(band, "reconnect", 0);
}

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
	char athfix[8];

	if(bssidx < 0 || bssidx >= MAX_NR_WL_IF || vifidx < 0 || vifidx >= MAX_NO_MSSID)
		return -1;
	if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
		if(vifidx == 0)
			strcpy(athfix, get_staifname(swap_5g_band(bssidx)));
		else
			__get_wlifname(swap_5g_band(bssidx), 0, athfix);
	}
	else {
		__get_wlifname(swap_5g_band(bssidx), vifidx, athfix);
	}
	ret = is_intf_up(athfix);
	return ret;
}

void set_wlan_service_status(int bssidx, int vifidx, int enabled)
{
	char athfix[8];
	if(bssidx < 0 || bssidx >= MAX_NR_WL_IF || vifidx < 0 || vifidx >= MAX_NO_MSSID)
		return;
	if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
		if(vifidx <= 0) {
			strcpy(athfix, get_staifname(swap_5g_band(bssidx)));
			doSystem("ifconfig %s %s", athfix, enabled?"up":"down");
			return;
		}
		vifidx--;
	}
	set_radio(enabled, swap_5g_band(bssidx), vifidx);
}

/*
 * set_pre_sysdep_config()
 * set_post_sysdep_config()
 *
 * The two function is called before and after adding a interface to brdige.
 * For handling some parameters or procrss of the interface.
 *
 */
void set_pre_sysdep_config(int iftype)
{
}

void set_post_sysdep_config(int iftype)
{
}

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

int wl_get_bw(int unit)
{
	int athfix[8];
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
	int ret = 0;
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
#endif // WIFI_SON
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
								fprintf(fp, "iwpriv %s %s %s\n", athfix, qca_mac, mac2g);
							}
						}
						else {
							foreach_44 (mac5g, maclist5g, next_mac) {
								if (check_re_in_macfilter(unit, mac5g))
									continue;
								printf("relist sta (%s) in %s\n", mac5g, athfix);
								fprintf(fp, "iwpriv %s %s %s\n", athfix, qca_mac, mac5g);
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

