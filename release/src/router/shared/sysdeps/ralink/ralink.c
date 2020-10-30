#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ralink.h>
#include "shutils.h"
#include "shared.h"

#ifdef RTCONFIG_AMAS 
char *get_pap_bssid(int unit)
{
	//TODO
        return "";
}

int wl_get_bw(int unit)
{
	//TODO
	return 20;
}

int get_psta_status(int unit)
{
	const char *ifname;
	char data[32];
	struct iwreq wrq;
	int status;

	ifname = get_staifname(unit);

	memset(data, 0x00, sizeof(data));
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = ASUS_SUBCMD_CONN_STATUS;

	if (wl_ioctl(ifname, RTPRIV_IOCTL_ASUSCMD, &wrq) < 0) {
		dbg("errors in getting %s CONN_STATUS result\n", ifname);
		return -1;
	}

	status = *(int*)wrq.u.data.pointer;

	if (status == 6)        // APCLI_CTRL_CONNECTED
		return WLC_STATE_CONNECTED;
	else if (status == 4)   // APCLI_CTRL_ASSOC
		return WLC_STATE_CONNECTING;

	return WLC_STATE_INITIALIZING;
}

enum {
    VSIE_BEACON = 0x1,
    VSIE_PROBE_REQ = 0x2,
    VSIE_PROBE_RESP = 0x4,
    VSIE_ASSOC_REQ = 0x8,
    VSIE_ASSOC_RESP = 0x10,
    VSIE_AUTH_REQ = 0x20,
    VSIE_AUTH_RESP = 0x40
};

void vsie_operation(int unit, int subunit, int flag, int opt, char *hexdata)
{
	struct iwreq wrq;
	char cmd_data[512], ifname[16];

	int len = 0;

	len = 3 + strlen(hexdata)/2;    /* 3 is oui's len */

	__get_wlifname(unit, subunit, ifname);

	snprintf(cmd_data, sizeof(cmd_data), "vie_op=%d-frm_map:%d-oui:%02X%02X%02X-length:%d-ctnt:%s",
		opt, flag, (uint8_t)OUI_ASUS[0], (uint8_t)OUI_ASUS[1], (uint8_t)OUI_ASUS[2], len, hexdata);

	wrq.u.data.length = strlen(cmd_data) + 1;
	wrq.u.data.pointer = cmd_data;
	wrq.u.data.flags = 0;

        if (wl_ioctl(ifname, RTPRIV_IOCTL_SET, &wrq) < 0)
                dbg("wl_ioctl failed on %s (%d)\n", __FUNCTION__, __LINE__);

    return;
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
    vsie_operation(unit, subunit, VSIE_BEACON | VSIE_PROBE_RESP, 1, hexdata);
}

/**
 * @brief add guest vsie
 *
 * @param hexdata vsie string
 */
void add_beacon_vsie_guest(char *hexdata)
{
    int unit = 0, subunit = 0;
    char word[100], *next;

    foreach (word, nvram_safe_get("wl_ifnames"), next) {
        if (nvram_get_int("re_mode") == 1)  // RE
            subunit = 3;
        else  // CAP/Router
            subunit = 2;
        for (; subunit <= num_of_mssid_support(unit); subunit++) {
            char buf[] = "wlXX.XX_ifname";
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
            if (is_intf_up(nvram_safe_get(buf)) != -1)  // interface exist
		    	vsie_operation(unit, subunit, VSIE_BEACON | VSIE_PROBE_RESP, 1, hexdata);
        }
        unit++;
    }
}

void add_beacon_vsie(char *hexdata)
{
#ifdef RTCONFIG_BHCOST_OPT
    int unit = 0;
    char word[100], *next;

    foreach (word, nvram_safe_get("wl_ifnames"), next) {
    	vsie_operation(unit, 0, VSIE_BEACON | VSIE_PROBE_RESP, 1, hexdata);
        unit++;
    }
#else
    vsie_operation(0, 0, VSIE_BEACON | VSIE_PROBE_RESP, 1, hexdata);
#endif
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
	vsie_operation(unit, subunit, VSIE_BEACON | VSIE_PROBE_RESP, 3, hexdata);
}

/**
 * @brief remove guest beacon vsie
 *
 * @param hexdata vsie string
 */
void del_beacon_vsie_guest(char *hexdata)
{
    int unit = 0, subunit = 0;
    char word[100], *next;

    foreach (word, nvram_safe_get("wl_ifnames"), next) {
        if (nvram_get_int("re_mode") == 1)  // RE
            subunit = 3;
        else  // CAP/Router
            subunit = 2;
        for (; subunit <= num_of_mssid_support(unit); subunit++) {
            char buf[] = "wlXX.XX_ifname";
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
            if (is_intf_up(nvram_safe_get(buf)) != -1)  // interface exist
		    	vsie_operation(unit, subunit, VSIE_BEACON | VSIE_PROBE_RESP, 3, hexdata);
        }
        unit++;
    }
}

void del_beacon_vsie(char *hexdata)
{
#ifdef RTCONFIG_BHCOST_OPT
    int unit = 0;
    char word[100], *next;

    foreach (word, nvram_safe_get("wl_ifnames"), next) {
    	vsie_operation(unit, 0, VSIE_BEACON | VSIE_PROBE_RESP, 3, hexdata);
        unit++;
    }
#else
    vsie_operation(0, 0, VSIE_BEACON | VSIE_PROBE_RESP, 3, hexdata);
#endif
}

void add_probe_req_vsie(char *hexdata)
{
    vsie_operation(0, 0, VSIE_PROBE_REQ, 1, hexdata);
}

void del_probe_req_vsie(char *hexdata)
{
    vsie_operation(0, 0, VSIE_PROBE_REQ, 3, hexdata);
}

void wait_connection_finished(int band)
{
    int wait_time = 0;
    int conn_stat = 0;
    int wlc_conn_time = nvram_get_int("wlc_conn_time") ?: 10;

    while (wait_time++ < wlc_conn_time) {
        conn_stat = get_psta_status(band);
        if (conn_stat == WLC_STATE_CONNECTED) break;
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
								set_acl_entry(wlif_name, mac2g);
							}
						}
						else
						{
							foreach_44 (mac5g, maclist5g, next_mac) {
								if (check_re_in_macfilter(unit, mac5g))
									continue;
								dbg("relist sta (%s) in %s\n", mac5g, wlif_name);
								set_acl_entry(wlif_name, mac5g);
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
