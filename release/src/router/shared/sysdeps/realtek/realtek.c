#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <wlutils.h>
#include "utils.h"
#include "shutils.h"
#include "shared.h"


#ifdef RTCONFIG_AMAS 
char *get_pap_bssid(int unit)
{
	unsigned char bssid[6] = "\0";
	char interface[] = "wlxxx-vxd";
	static char bssid_str[sizeof("00:00:00:00:00:00XXX")];

	snprintf(interface, sizeof(interface), "wl%d-vxd", unit);
	if (wl_ioctl(interface, WLC_GET_BSSID, bssid, sizeof(bssid)) == 0) {
		if ( !(!bssid[0] && !bssid[1] && !bssid[2] && !bssid[3] && !bssid[4] && !bssid[5]) ) {
			snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", 
				(unsigned char)bssid[0], (unsigned char)bssid[1],
				(unsigned char)bssid[2], (unsigned char)bssid[3],
				(unsigned char)bssid[4], (unsigned char)bssid[5]);
		}
	}

	return bssid_str;
}

int wl_get_bw(int unit)
{
    int bw, nctrlsb;

    get_bw_nctrlsb(get_staifname(unit), &bw, &nctrlsb);
    return bw;
}

int get_psta_status(int unit)
{
	bss_info bss_buf;
	char interface[] = "wlxxx-vxd";
	snprintf(interface, sizeof(interface), "wl%d-vxd", unit);
	memset(&bss_buf,0,sizeof(bss_info));

	if (wl_ioctl(interface, WLC_GET_BSS_INFO, &bss_buf, sizeof(bss_buf)) < 0)
		return 0;
	else {
		if (bss_buf.state == 4)
			return WLC_STATE_CONNECTED;
		else if (bss_buf.state == 2)
			return WLC_STATE_CONNECTING;
		else if (bss_buf.state == 0)
			return WLC_STATE_STOPPED;
		else
			return WLC_STATE_INITIALIZING;	
	}
}

void set_beacon_vsie(DOT11_SET_USERIE* Set_USERIE, char* hexdata)
{
	int len = 0, i = 0;
	char *p;

	Set_USERIE->EventId = DOT11_EVENT_USER_SETIE;

	memset(Set_USERIE->USERIE,0 ,sizeof(Set_USERIE->USERIE));
	Set_USERIE->USERIE[0] = 221;
	Set_USERIE->USERIE[1] = 3 + strlen(hexdata)/2;
	Set_USERIE->USERIE[2] = OUI_ASUS[0];
	Set_USERIE->USERIE[3] = OUI_ASUS[1];
	Set_USERIE->USERIE[4] = OUI_ASUS[2];

	len = strlen(hexdata)/2+5;
	p = hexdata;

	if(len > sizeof(Set_USERIE->USERIE))
		return;

	for(i = 5; i < len; i++) {
		sscanf(p, "%02x", &(Set_USERIE->USERIE[i]));
		p = p+2;
	}

	Set_USERIE->USERIELen = len;

	update_vsie(get_wififname(0), (void *)Set_USERIE);
}

void add_beacon_vsie(char *hexdata)
{
	if(hexdata == NULL)
		return;

	DOT11_SET_USERIE Set_USERIE;

	Set_USERIE.Flag = SET_IE_FLAG_INSERT;
	set_beacon_vsie(&Set_USERIE, hexdata);	
}

void del_beacon_vsie(char *hexdata)
{
	if(hexdata == NULL)
		return;

	DOT11_SET_USERIE Set_USERIE;

	Set_USERIE.Flag = SET_IE_FLAG_DELETE;
	set_beacon_vsie(&Set_USERIE, hexdata);	

}

void wait_connection_finished(char* ifname) {
    _dprintf("%s %s\n", __FUNCTION__, ifname);
    int wait_time = 0;
    bss_info bss;
    while (wait_time++ < 20) {
        getWlBssInfo(ifname, &bss);
        if (bss.state == 4)// connectted
            break;
        sleep(1);
    }
}
#endif
#ifdef RTCONFIG_CFGSYNC

void update_macfilter_relist(void)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char word[256], *next;
	char mac2g[32], mac5g[32], *next_mac;
	int unit = 0;
	char *wlif_name = NULL;
	char *nv, *nvp, *b;
	char *reMac, *maclist2g, *maclist5g, *timestamp;
	char stamac2g[18] = {0};
	char stamac5g[18] = {0};

	if (nvram_get("cfg_relist"))
	{
#ifdef RTCONFIG_AMAS
		if (nvram_get_int("re_mode") == 1) {
			nv = nvp = strdup(nvram_safe_get("cfg_relist"));
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

					if (strcmp(reMac, get_lan_hwaddr()) == 0) {
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

		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

#ifdef RTCONFIG_AMAS
			if (nvram_get_int("re_mode") == 1)
				snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
			else
#endif
				snprintf(prefix, sizeof(prefix), "wl%d_", unit);

			wlif_name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

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
								dbg("relist sta (%s) in %s\n", mac2g, wlif_name);
								set_mib_acladdr(wlif_name, mac2g);
							}
						}
						else
						{
							foreach_44 (mac5g, maclist5g, next_mac) {
								if (check_re_in_macfilter(unit, mac5g))
									continue;
								dbg("relist sta (%s) in %s\n", mac5g, wlif_name);
								set_mib_acladdr(wlif_name, mac5g);
							}
						}
					}
					free(nv);
				}
			}

			unit++;
		}
	}
}
#endif
