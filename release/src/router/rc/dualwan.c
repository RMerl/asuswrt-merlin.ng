
#include <string.h>
#include "rc.h"

#ifdef RTCONFIG_DUALWAN

#define FLUSH_INTERVAL 30

#if defined(RTAC88U)
#define MODEL_PROTECT "RT-AC88U"
#endif

#if defined(RTAC3100)
#define MODEL_PROTECT "RT-AC3100"
#endif

#if defined(RTAC5300)
#define MODEL_PROTECT "RT-AC5300"
#endif

#if defined(RTAC87U)
#define MODEL_PROTECT "RT-AC87U"
// #define WANDEV_SING "vlan2 vlan3"	not supported now
#define WANDEVS_DUAL "vlan3"
#define WANDEVS_SING "et1"

#define LANIF_ETH	"vlan1"
#define LANIF_2G	"eth1"
#define LANIF_5G	"wifi0"

#define WANIF_ETH	"eth0"
#define WANIF_WAN0	"vlan2"
#define WANIF_WAN1	"vlan3"
#define WANIF_USB	"usb"
#ifdef RTCONFIG_USB_MULTIMODEM
#define WANIF_USB2	"usb2"
#endif
#define WANIF_ETH_USB	"eth0 usb"
#endif

#if defined(RTAC3200)
#define MODEL_PROTECT "RT-AC3200"
#endif

#if defined(RTN18U)
#define MODEL_PROTECT "RT-N18U"
#endif

#ifndef MODEL_PROTECT
#define MODEL_PROTECT "NOT_SUPPORT"
#endif

#ifdef RTCONFIG_HW_DUALWAN
void init_dualwan(int argc, char *argv[])
{
	int unit = 0;
	int caps;
	int if_id;
	char wan_if[10];

	caps = get_wans_dualwan();

	if ((caps & WANSCAP_WAN) && (caps & WANSCAP_LAN))
		nvram_set("wandevs", WANDEVS_DUAL);
	else
		nvram_set("wandevs", WANDEVS_SING);

	set_lan_phy(LANIF_ETH);

	if (!(caps & WANSCAP_2G))
		add_lan_phy(LANIF_2G);
#ifdef RTCONFIG_HAS_5G
	if (!(caps & WANSCAP_5G))
		add_lan_phy(LANIF_5G);
#endif

	if (nvram_get("wans_dualwan")) {
		set_wan_phy("");
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			if_id = get_dualwan_by_unit(unit);
			if (if_id == WANS_DUALWAN_IF_LAN) {
				if (caps & WANSCAP_WAN)
					add_wan_phy(WANIF_WAN1);
				else
					add_wan_phy(WANIF_ETH);
			}
			else if (if_id == WANS_DUALWAN_IF_2G)
				add_wan_phy(LANIF_2G);
#ifdef RTCONFIG_HAS_5G
			else if (if_id == WANS_DUALWAN_IF_5G)
				add_wan_phy(LANIF_5G);
#endif
			else if (if_id == WANS_DUALWAN_IF_WAN) {
				/* tag by IPTV */
				if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
					int wan_vid = nvram_get_int("switch_wan0tagid");
					if (wan_vid) {
						snprintf(wan_if, sizeof(wan_if), "vlan%d", wan_vid);
						add_wan_phy(wan_if);
					}
					else
						add_wan_phy(WANIF_ETH);
				}
				else if (caps &WANSCAP_LAN)
					add_wan_phy(WANIF_WAN0);
				else
					add_wan_phy(WANIF_ETH);
			}
#ifdef RTCONFIG_USB_MODEM
			else if (if_id == WANS_DUALWAN_IF_USB)
				add_wan_phy(WANIF_USB);
#ifdef RTCONFIG_USB_MULTIMODEM
			else if (if_id == WANS_DUALWAN_IF_USB2)
				add_wan_phy(WANIF_USB2);
#endif
#endif
#ifdef RTCONFIG_USB_MULTIMODEM
			else if (if_id == WANS_DUALWAN_IF_USB2)
				add_wan_phy(WANIF_USB2);
#endif
		}
	}
#ifdef RTCONFIG_USB_MODEM
	else
		nvram_set("wan_ifnames", WANIF_ETH_USB);
#endif
}
#endif

int dualwan_control(int argc, char *argv[])
{
	char dualwan_mode[8];
	char dualwan_wans[16];
	char wan0_proto[10];
	char wan1_proto[10];

#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
	if (strcmp(nvram_safe_get("model"), MODEL_PROTECT) != 0){
#else
	if (strcmp(cfe_nvram_safe_get("model"), MODEL_PROTECT) != 0){
#endif
		_dprintf("illegal, cannot enable DualWAN\n");
		return -1;
	}

	strlcpy(dualwan_mode, nvram_safe_get("wans_mode"), sizeof(dualwan_mode));
	strlcpy(dualwan_wans, nvram_safe_get("wans_dualwan"), sizeof(dualwan_wans));
	strlcpy(wan0_proto, nvram_safe_get("wan0_proto"), sizeof(wan0_proto));
	strlcpy(wan1_proto, nvram_safe_get("wan1_proto"), sizeof(wan1_proto));

	if(strcmp(dualwan_mode, "lb") != 0) goto EXIT;
	if(strcmp(dualwan_wans, "wan lan") != 0) goto EXIT;
	if(strcmp(wan0_proto, "pptp") == 0 ||
		strcmp(wan1_proto, "l2tp") == 0)
		goto EXIT;
	if (!is_router_mode()) goto EXIT;

	while(1){
		f_write_string("/proc/sys/net/ipv4/route/flush", "1", 0, 0);
		f_write_string("/proc/sys/net/ipv4/route/flush", "1", 0, 0);
		f_write_string("/proc/sys/net/ipv4/route/flush", "1", 0, 0);
		sleep(FLUSH_INTERVAL);
	}

EXIT:
	return 0;
}

#endif

