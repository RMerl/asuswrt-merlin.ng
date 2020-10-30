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

#include "realtek.h"

#define B1_G1	40
#define B1_G2	48

#define B2_G1	56
#define B2_G2	64

#define B3_G1	104
#define B3_G2	112
#define B3_G3	120
#define B3_G4	128
#define B3_G5	136
#define B3_G6	144

#define B4_G1	153
#define B4_G2	161
#define B4_G3	169
#define B4_G4	177

void assign_diff_AC(unsigned char* pMib, unsigned char* pVal)
{
	memset((pMib+35), pVal[0], (B1_G1-35));
	memset((pMib+B1_G1), pVal[1], (B1_G2-B1_G1));
	memset((pMib+B1_G2), pVal[2], (B2_G1-B1_G2));
	memset((pMib+B2_G1), pVal[3], (B2_G2-B2_G1));
	memset((pMib+B2_G2), pVal[4], (B3_G1-B2_G2));
	memset((pMib+B3_G1), pVal[5], (B3_G2-B3_G1));
	memset((pMib+B3_G2), pVal[6], (B3_G3-B3_G2));
	memset((pMib+B3_G3), pVal[7], (B3_G4-B3_G3));
	memset((pMib+B3_G4), pVal[8], (B3_G5-B3_G4));
	memset((pMib+B3_G5), pVal[9], (B3_G6-B3_G5));
	memset((pMib+B3_G6), pVal[10], (B4_G1-B3_G6));
	memset((pMib+B4_G1), pVal[11], (B4_G2-B4_G1));
	memset((pMib+B4_G2), pVal[12], (B4_G3-B4_G2));
	memset((pMib+B4_G3), pVal[13], (B4_G4-B4_G3));

}
#ifdef RTCONFIG_RTL8198D
void assign_diff_AC_hex_to_string(unsigned char* pmib,char* str)
#else
void assign_diff_AC_hex_to_string(unsigned char* pmib,char* str,int len)
#endif
{
	char mib_buf[MAX_5G_CHANNEL_NUM_MIB];
	memset(mib_buf, 0, sizeof(mib_buf));
	assign_diff_AC(mib_buf, pmib);
	hex_to_string(mib_buf, str, MAX_5G_CHANNEL_NUM_MIB);
}

int hex_to_string(unsigned char *hex,char *str,int len)
{
	int i;
	char *d,*s;
	const static char hexdig[] = "0123456789abcdef";
	if(hex == NULL||str == NULL)
		return -1;
	d = str;
	s = hex;

	for(i = 0;i < len;i++,s++){
		*d++ = hexdig[(*s >> 4) & 0xf];
		*d++ = hexdig[*s & 0xf];
	}
	*d = 0;
	return 0;
}

#ifdef RTCONFIG_AMAS 
char *get_pap_bssid(int unit)
{
	unsigned char bssid[6] = "\0";
#ifdef RPAC92
	char interface[] = "wlanxxx-vxd";
	static char bssid_str[sizeof("00:00:00:00:00:00XXX")];

	if(unit==2)
		snprintf(interface, sizeof(interface), "wlan%d-vxd", 0);
	else if(unit==0)
		snprintf(interface, sizeof(interface), "wlan%d-vxd", 2);
	else if(unit==1)
		snprintf(interface, sizeof(interface), "wlan%d-vxd", 1);
#else
	char interface[] = "wlxxx-vxd";
	static char bssid_str[sizeof("00:00:00:00:00:00XXX")];

	snprintf(interface, sizeof(interface), "wl%d-vxd", unit);
#endif
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
#ifdef RPAC92
	char interface[] = "wlanxxx-vxd";
	if(unit==2)
		snprintf(interface, sizeof(interface), "wlan%d-vxd", 0);
	else if(unit==0)
		snprintf(interface, sizeof(interface), "wlan%d-vxd", 2);
	else if(unit==1)
		snprintf(interface, sizeof(interface), "wlan%d-vxd", 1);
#else
	char interface[] = "wlxxx-vxd";
	snprintf(interface, sizeof(interface), "wl%d-vxd", unit);
#endif
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

	str2hex(hexdata, &Set_USERIE->USERIE[5], strlen(hexdata));

	Set_USERIE->USERIELen = len;

	update_vsie(get_wififname(0), (void *)Set_USERIE);
}

/**
 * @brief add beacon vise by unit and subunit
 *
 * @param unit band index
 * @param subunit mssid index
 * @param hexdata vise string
 */
void add_beacon_vsie_by_unit(int unit, int subunit, char *hexdata)
{
	; // TODO
}

/**
 * @brief add guest vsie
 *
 * @param hexdata vsie string
 */
void add_beacon_vsie_guest(char *hexdata)
{
	; // TODO
}

void add_beacon_vsie(char *hexdata)
{
	if(hexdata == NULL)
		return;

	DOT11_SET_USERIE Set_USERIE;

	Set_USERIE.Flag = SET_IE_FLAG_INSERT;
	set_beacon_vsie(&Set_USERIE, hexdata);	
}

/**
 * @brief remove beacon vsie by unit and subunit
 *
 * @param unit band index
 * @param subunit mssid index
 * @param hexdata vsie string
 */
void del_beacon_vsie_by_unit(int unit, int subunit, char *hexdata)
{
	; // TODO
}

/**
 * @brief remove guest beacon vsie
 *
 * @param hexdata vsie string
 */
void del_beacon_vsie_guest(char *hexdata)
{
	; // TODO
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

#ifdef RTCONFIG_BHCOST_OPT
unsigned int get_uplinkports_linkrate(char *ifname)
{
	unsigned int link_rate = 1000;

	//TODO for getting link rate

	return link_rate;
}
#endif	/* RTCONFIG_BHCOST_OPT */
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

void stop_wlc_connect(int band) {
	char *ifname = NULL;

	if(repeater_mode())
		ifname = get_staifname(band);
	else if(mediabridge_mode())
		ifname = get_wififname(band);

	if (is_intf_up(ifname))
		eval("ifconfig", ifname, "down");
}

void start_wlc_connect(int band) {	

	char *ifname = NULL;

	if(repeater_mode())
		ifname = get_staifname(band);
	else if(mediabridge_mode())
		ifname = get_wififname(band);

	if(!is_intf_up(ifname))
		eval("ifconfig", ifname, "up");
}

int get_wlan_status(int band) {
	char tmp[32]="func_off";
	int length =sizeof(tmp);

	char* ifname = get_wififname(band);
	if(is_intf_up(ifname)){
		getmibInfo(ifname, tmp, &length);
		//*tmp = 0 normal mode 1 is wlan off
#if defined(RPAC92)
		if(!(int)(*(int*)tmp))
#else
		if(!(int)(*tmp))
#endif
			return 1;
	}
	return 0;
}

/*
	use func off to enable or disable
	enable = 1 equal of func_off =0
*/
void set_wlan_status(int band, int enable) {
	doSystem("iwpriv %s set_mib func_off=%d", get_wififname(band), enable?0:1);
}

#ifdef RTCONFIG_NEW_PHYMAP
/* phy port related start */
phy_port_mapping get_phy_port_mapping(void)
{
	static const phy_port_mapping port_mapping = {
#if 0
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) || defined(SHAC1300) /* for Lyra */
		.count = 2, 
		.port[0] = { .phy_port_id = WAN_PORT, .cap = PHY_PORT_CAP_WAN, .max_rate = 1000 }, 
		.port[1] = { .phy_port_id = LAN4_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }
#elif defined(RTAC95U)
		.count = 4, 
		.port[0] = { .phy_port_id = WAN_PORT, .cap = PHY_PORT_CAP_WAN, .max_rate = 1000 }, 
		.port[1] = { .phy_port_id = LAN1_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }, 
		.port[2] = { .phy_port_id = LAN2_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }, 
		.port[3] = { .phy_port_id = LAN3_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }
#else
		.count = 5, 
		.port[0] = { .phy_port_id = WAN_PORT, .cap = PHY_PORT_CAP_WAN, .max_rate = 1000 }, 
		.port[1] = { .phy_port_id = LAN1_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }, 
		.port[2] = { .phy_port_id = LAN2_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }, 
		.port[3] = { .phy_port_id = LAN3_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }, 
		.port[4] = { .phy_port_id = LAN4_PORT, .cap = PHY_PORT_CAP_LAN, .max_rate = 1000 }
#endif
#endif
	};
	return port_mapping;
}
#endif
