#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "rc.h"

#if defined(RTBE82M) || defined(GS7_PRO_MAX) || defined(GSBE18000) || defined(GS7_PRO)
#define IS_MXL_MACRO 1
#else
#define IS_MXL_MACRO 0
#endif

/* =========================================================================
 * Logical port roles and macro 
 * ========================================================================= */
#define IPTV_BR_INDEX	  101

#define PORT_WAN          (1 << 0)
#define PORT_LAN1         (1 << 1)
#define PORT_LAN2         (1 << 2)
#define PORT_LAN3         (1 << 3)
#define PORT_LAN4         (1 << 4)
#define PORT_LAN5         (1 << 5)
#define PORT_LAN6         (1 << 6)
#define PORT_LAN7         (1 << 7)
#define PORT_LAN8         (1 << 8)
#define PORT_CPU_WAN      (1 << 9)  
#define PORT_CPU_LAN      (1 << 10) 
#define PORT_UNTAGGED_WAN (1 << 11) /* virtual flag for no-tag wan */
#define PORT_STB_X        (1 << 12) /* virtual flag for dynamic stb port */
#define MASK_ALL_LAN_EXC_STB (1 << 13) /* virtual flag for remaining lan ports */

#define MASK_ALL_LAN      (PORT_LAN1 | PORT_LAN2 | PORT_LAN3 | PORT_LAN4 | \
                           PORT_LAN5 | PORT_LAN6 | PORT_LAN7 | PORT_LAN8)

#if defined(RTCONFIG_EXT_SWITCH_ALL)
#define UPLINK_WAN    (PORT_WAN | PORT_CPU_WAN)
#define UPLINK_BRIDGE (PORT_WAN | PORT_CPU_WAN | PORT_CPU_LAN)
#else
#define UPLINK_WAN    (PORT_CPU_WAN) 
#define UPLINK_BRIDGE (PORT_CPU_WAN | PORT_CPU_LAN)
#endif

static int is_netdev_per_port_arch(void) {
#if defined(RTCONFIG_NETDEV_PER_PORT)
	return 1;
#else
	return 0;
#endif
}

static int is_interface_exists(const char *ifname) {
	char path[128];
	snprintf(path, sizeof(path), "/sys/class/net/%s", ifname);
	return (access(path, F_OK) == 0);
}

static const char* get_cpu_lan_if(void) {
	return "eth1"; 	/* external switch common lan ifname; create case if not */
}

static const char* get_netdev_if(unsigned int logical_port) {
	if (logical_port == PORT_LAN1) return "eth1";
	if (logical_port == PORT_LAN2) return "eth2";
	if (logical_port == PORT_LAN3) return "eth3";
	if (logical_port == PORT_LAN4) return "eth4";
	if (logical_port == PORT_LAN5) return "eth5";
	if (logical_port == PORT_LAN6) return "eth6";
	if (logical_port == PORT_LAN7) return "eth7";
	if (logical_port == PORT_LAN8) return "eth8";
	return NULL;
}

/* =========================================================================
 * Hardware Layer (for case-2 / case-3)
 * ========================================================================= */
static int g_rtk = 0;
#if !defined(RTCONFIG_NETDEV_PER_PORT)

#if IS_MXL_MACRO
static int g_mxls_inited = 0;
#endif

struct switch_phy_map {
	unsigned int logical_port;
	unsigned int phy_bitmask;
};

static const struct switch_phy_map dut_phy_map[] = {
#if defined(RTBE82M)
	{ PORT_WAN,     (1 << 1) }, { PORT_LAN1,    (1 << 2) }, { PORT_LAN2,    (1 << 3) }, 
	{ PORT_LAN3,    (1 << 4) }, { PORT_LAN4,    (1 << 5) }, { PORT_CPU_WAN, (1 << 6) }, 
	{ PORT_CPU_LAN, (1 << 7) }, 
#elif defined(GS7_PRO_MAX)
	{ PORT_CPU_LAN, (1 << 6) }, { PORT_LAN1,    (1 << 2) }, { PORT_LAN2,    (1 << 1) },  
	{ PORT_LAN3,    (1 << 4) }, { PORT_LAN4,    (1 << 3) }, { PORT_LAN5,    (1 << 5) },  
	{ PORT_LAN6,    (1 << 13) }, 
#elif defined(GSBE18000) || defined(GS7_PRO)
	{ PORT_WAN,     (1 << 4) }, { PORT_CPU_WAN, (1 << 9) }, { PORT_CPU_LAN, (1 << 10) }, 
	{ PORT_LAN1,    (1 << 5) }, { PORT_LAN2,    (1 << 6) }, { PORT_LAN3,    (1 << 7) },  
	{ PORT_LAN4,    (1 << 8) }, { PORT_LAN5,    (1 << 1) }, { PORT_LAN6,    (1 << 2) },  
	{ PORT_LAN7,    (1 << 3) },  
/* ================= Realtek Switch (Ext_LAN) Models ================= */
/* 8372: LAN (4,5,6,7), CPU (8) */
#elif defined(RTBE82U) || defined(TUFBE82)
	{ PORT_LAN1, (1 << 4) }, { PORT_LAN2, (1 << 5) }, { PORT_LAN3, (1 << 6) }, { PORT_LAN4, (1 << 7) },
	{ PORT_CPU_LAN, (1 << 8) },
/* 8372: LAN (4,5,6,7), CPU (3) */
#elif defined(RTBE92U) || defined(RTBE95U) || defined(RTBE58U_PRO)
	{ PORT_LAN1, (1 << 4) }, { PORT_LAN2, (1 << 5) }, { PORT_LAN3, (1 << 6) }, { PORT_LAN4, (1 << 7) },
	{ PORT_CPU_LAN, (1 << 3) },
/* 8372: LAN (5,6,7,4), CPU (3) */
#elif defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GTBE96_AI)
	{ PORT_LAN1, (1 << 5) }, { PORT_LAN2, (1 << 6) }, { PORT_LAN3, (1 << 7) }, { PORT_LAN4, (1 << 4) },
	{ PORT_CPU_LAN, (1 << 3) },
/* 8367s: LAN (4,3,2,1), CPU (16) */
#elif defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55)
	{ PORT_LAN1, (1 << 4) }, { PORT_LAN2, (1 << 3) }, { PORT_LAN3, (1 << 2) }, { PORT_LAN4, (1 << 1) },
	{ PORT_CPU_LAN, (1 << 16) },
#endif
};

static unsigned int get_phy_vlan_mask(unsigned int logical_member_mask, unsigned int logical_untag_mask) {
	unsigned int phy_member = 0, phy_untag = 0;
	int i, map_size = sizeof(dut_phy_map) / sizeof(dut_phy_map[0]);
	for (i = 0; i < map_size; i++) {
		if (logical_member_mask & dut_phy_map[i].logical_port) phy_member |= dut_phy_map[i].phy_bitmask;
			if (logical_untag_mask & dut_phy_map[i].logical_port) phy_untag |= dut_phy_map[i].phy_bitmask;
	}
	return ((phy_untag & 0xffff) << 16) | (phy_member & 0xffff);
}

static int setup_vlan_logical(int vid, int prio, unsigned int logical_member, unsigned int logical_untag) {
	unsigned int hw_mask = get_phy_vlan_mask(logical_member, logical_untag);
	if (vid > 0 && hw_mask == 0) return 0; 

#if IS_MXL_MACRO
	if (!g_mxls_inited) return 0;
#endif

	return __setup_vlan(vid, prio, hw_mask);
}
#endif

/* =========================================================================
 * ISP Profile Table 
 * ========================================================================= */
#define VID_DEFAULT		 0
#define VID_NVRAM_INTERNET	-1 
#define VID_NVRAM_IPTV		-2 
#define VID_NVRAM_VOIP		-3 

struct iptv_vlan_rule {
	int vid;                      
	unsigned int logical_members; 
	unsigned int logical_untags;  
	int prio;                     
	int is_internet;              
	int use_snooper; 
	int is_iptv;
};

struct iptv_profile {
	const char *profile_name;
	int rule_count;           
	struct iptv_vlan_rule rules[6]; 
};

static const struct iptv_profile iptv_profile_table[] = {
	{ "2degrees", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "aapt", 2, { 
		{ 2, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "actrix", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "amaysim", 2, { 
		{ 100, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "celcom", 2, { 
		{ 101, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "celcom_tm", 2, { 
		{ 623, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "centurylink", 2, { 
		{ 201, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "digi1_tm", 2, { 
		{ 629, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "digi2_time", 2, { 
		{ 221, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "digi3_digi", 2, { 
		{ 1001, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "digi4_cts", 2, { 
		{ 150, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "digi5_allo", 2, { 
		{ 1600, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "digi6_sacofa", 2, { 
		{ 1000, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "dodo", 2, { 
		{ 100, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "free", 3, { 
		{ VID_DEFAULT, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN4) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN4), -1, 0, 0, 0 }, 
		{ 100, UPLINK_BRIDGE | PORT_LAN4, 0, 0, 0, 0, 1 } 
	}},
	{ "google_fiber", 2, { 
		{ 2, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "hinet", 2, {
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN4) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN4), -1, 0, 0, 0 },
		{ 1, PORT_UNTAGGED_WAN | PORT_LAN4 | PORT_CPU_LAN, PORT_LAN4, 0, 1, 0, 1 }
	}},
	{ "hinet_mesh", 2, {
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 },
		{ 1, PORT_UNTAGGED_WAN | PORT_CPU_LAN, 0, 0, 1, 1, 1 }
	}},
	{ "iinet", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "intronode", 2, { 
		{ 2, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "iprimus", 2, { 
		{ 100, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "jastel", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "kpn_nl", 3, {
		{ 6, UPLINK_WAN, 0, 0, 1, 0, 0 },
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN4) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN4), -1, 0, 0, 0 },
		{ 4, UPLINK_BRIDGE | PORT_LAN4, PORT_LAN4, 0, 0, 0, 1 } 
	}},
	{ "m1_fiber", 3, { 
		{ 1103, UPLINK_WAN, 0, 1, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN3) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN3), -1, 0, 0, 0 }, 
		{ 1107, UPLINK_BRIDGE | PORT_LAN3, 0, 1, 0, 0, 0 } 
	}},
	{ "manual", 4, {
		{ VID_NVRAM_INTERNET, UPLINK_WAN, 0, -1, 1, 0, 0 },
		{ VID_DEFAULT, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)) | PORT_CPU_LAN, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)), -1, 0, 0, 0 },
		{ VID_NVRAM_IPTV, UPLINK_BRIDGE | PORT_LAN4, PORT_LAN4, -1, 0, 0, 1 }, /* is_iptv = 1 */
		{ VID_NVRAM_VOIP, UPLINK_BRIDGE | PORT_LAN3, PORT_LAN3, -1, 0, 0, 1 }
	}},
	{ "maxis_cts", 3, { 
		{ 41, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN3) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN3), -1, 0, 0, 0 }, 
		{ 44, UPLINK_BRIDGE | PORT_LAN3, 0, 0, 0, 0, 0 } 
	}},
	{ "maxis_fiber", 4, { 
		{ 621, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN3) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN3), -1, 0, 0, 0 }, 
		{ 821, UPLINK_BRIDGE | PORT_LAN3, 0, 0, 0, 0, 0 }, 
		{ 822, UPLINK_BRIDGE | PORT_LAN3, 0, 0, 0, 0, 0 } 
	}},
	{ "maxis_fiber_sp", 3, { 
		{ 11, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN3) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN3), -1, 0, 0, 0 }, 
		{ 14, UPLINK_BRIDGE | PORT_LAN3, 0, 0, 0, 0, 0 } 
	}},
	{ "maxis_sacofa", 3, { 
		{ 31, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN3) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN3), -1, 0, 0, 0 }, 
		{ 34, UPLINK_BRIDGE | PORT_LAN3, 0, 0, 0, 0, 0 } 
	}},
	{ "maxis_tnb", 3, { 
		{ 51, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN3) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN3), -1, 0, 0, 0 }, 
		{ 54, UPLINK_BRIDGE | PORT_LAN3, 0, 0, 0, 0, 0 } 
	}},
	{ "meo_br", 4, { 
		{ 12, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)) | PORT_CPU_LAN, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)), -1, 0, 0, 0 }, 
		{ 12, UPLINK_BRIDGE | PORT_LAN3, PORT_LAN3, 0, 0, 0, 1 }, 
		{ 12, UPLINK_BRIDGE | PORT_LAN4, 0, 0, 0, 0, 0 } 
	}},
	{ "meo_iptv", 3, { 
		{ 12, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)) | PORT_CPU_LAN, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)), -1, 0, 0, 0 }, 
		{ 12, UPLINK_BRIDGE | PORT_LAN3 | PORT_LAN4, PORT_LAN3 | PORT_LAN4, 0, 0, 0, 1 } 
	}},
	{ "movistar", 4, {
		{ 6, UPLINK_WAN, 0, 1, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 }, /* All LANs stay in br0 */
		{ 2, UPLINK_WAN, 0, 4, 0, 0, 1 }, /* is_iptv = 1 */
		{ 3, UPLINK_WAN, 0, 4, 0, 0, 0 }  
	}},
	{ "none", 2, {
		{ VID_DEFAULT, MASK_ALL_LAN_EXC_STB | PORT_CPU_LAN, MASK_ALL_LAN_EXC_STB, -1, 0, 0, 0 },
		{ 1, PORT_UNTAGGED_WAN | PORT_STB_X | PORT_CPU_LAN, PORT_STB_X, 0, 1, 0, 1 }
	}}, 
	{ "nowtv", 2, { 
		{ VID_DEFAULT, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "orcon", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "singtel_mio", 4, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)) | PORT_CPU_LAN, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)), -1, 0, 0, 0 }, 
		{ 20, UPLINK_BRIDGE | PORT_LAN4, PORT_LAN4, 4, 0, 0, 1 }, 
		{ 30, UPLINK_BRIDGE | PORT_LAN3, 0, 4, 0, 0, 0 } 
	}},
	{ "singtel_mstb", 3, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)) | PORT_CPU_LAN, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)), -1, 0, 0, 0 }, 
		{ 20, UPLINK_BRIDGE | PORT_LAN3 | PORT_LAN4, PORT_LAN3 | PORT_LAN4, 4, 0, 0, 1 } 
	}},
	{ "singtel_others", 3, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN4) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN4), -1, 0, 0, 0 }, 
		{ 20, UPLINK_BRIDGE | PORT_LAN4, PORT_LAN4, 4, 0, 0, 1 } 
	}},
	{ "slingshot", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "spark", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "stuff_fibre", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "superonline", 3, { 
		{ VID_DEFAULT, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN4) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN4), -1, 0, 0, 0 }, 
		{ 103, UPLINK_BRIDGE | PORT_LAN4, PORT_LAN4, 4, 0, 0, 1 } 
	}},
	{ "tpg", 2, { 
		{ 2, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "unifi_biz", 2, { 
		{ 500, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "unifi_biz_voip", 3, { 
		{ 500, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN3) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN3), -1, 0, 0, 0 }, 
		{ 400, UPLINK_BRIDGE | PORT_LAN3, PORT_LAN3, 0, 0, 0, 0 } 
	}},
	{ "unifi_home", 3, { 
		{ 500, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~PORT_LAN4) | PORT_CPU_LAN, (MASK_ALL_LAN & ~PORT_LAN4), -1, 0, 0, 0 }, 
		{ 600, UPLINK_BRIDGE | PORT_LAN4, PORT_LAN4, 0, 0, 0, 1 } 
	}},
	{ "voda_nz", 2, { 
		{ 10, UPLINK_WAN, 0, 0, 1, 0, 0 }, 
		{ VID_DEFAULT, MASK_ALL_LAN | PORT_CPU_LAN, MASK_ALL_LAN, -1, 0, 0, 0 } 
	}},
	{ "vodafone", 4, {
		{ 100, UPLINK_BRIDGE | PORT_LAN4, 0, 1, 1, 0, 0 }, 
		{ VID_DEFAULT, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)) | PORT_CPU_LAN, (MASK_ALL_LAN & ~(PORT_LAN3 | PORT_LAN4)), -1, 0, 0, 0 },
		{ 101, UPLINK_BRIDGE | PORT_LAN4, 0, 1, 0, 0, 0 }, 
		{ 105, UPLINK_BRIDGE | PORT_LAN3 | PORT_LAN4, PORT_LAN3, 1, 0, 0, 1 } 
	}}   
};

/* =========================================================================
 * Core engine 
 * ========================================================================= */
void apply_iptv_profile() 
{
	int i, j;
	const struct iptv_profile *prof = NULL;
	const char *profile_name = nvram_safe_get("switch_wantag");
	int is_netdev_arch = is_netdev_per_port_arch();
    
	char wan_if[IFNAMSIZ] = { 0 };
#ifdef HND_ROUTER
	get_wan0_ifname_phymap(wan_if, sizeof(wan_if));
#else
	snprintf(wan_if, sizeof(wan_if), "%s", nvram_safe_get("wan_ifname"));
#endif
    
	int br_idx = IPTV_BR_INDEX; 

#ifdef RTCONFIG_HINET_MESH_BRIDGE
	if (strcmp(profile_name, "hinet_mesh") == 0) {
		config_hinet_mesh(wan_if);
		return;
	}
#endif

	unsigned int stb_ports = 0;
	if (strcmp(profile_name, "none") == 0) {
		int stb_val = nvram_get_int("switch_stb_x");
		if (stb_val == 1) stb_ports = PORT_LAN1;
		else if (stb_val == 2) stb_ports = PORT_LAN2;
		else if (stb_val == 3) stb_ports = PORT_LAN3;
		else if (stb_val == 4) stb_ports = PORT_LAN4;
		else if (stb_val == 5) stb_ports = PORT_LAN1 | PORT_LAN2;
		else if (stb_val == 6) stb_ports = PORT_LAN3 | PORT_LAN4;
	}

	/* mxl init (for case-2/3) */
#if !defined(RTCONFIG_NETDEV_PER_PORT)
#if IS_MXL_MACRO
	int switch_stb = nvram_get_int("switch_stb_x");
	if (switch_stb==0 || switch_stb>=7) {
		g_mxls_inited = 0; 
	} else {
		for (i=0; i<4; ++i) {
			if (mxls_init() == 0) { g_mxls_inited = 1; break; }
				sleep(1);
		}
	}
#endif
	/* set access_type_all */
	if (access("/sbin/rtkswitch", F_OK) == 0 || access("/usr/sbin/rtkswitch", F_OK) == 0) {
		g_rtk = 1;
		eval("rtkswitch", "395");
	}
#endif

	/* find match profile */
	int table_size = sizeof(iptv_profile_table) / sizeof(iptv_profile_table[0]);
	for (i = 0; i < table_size; i++) {
		if (strcmp(iptv_profile_table[i].profile_name, profile_name) == 0) {
			prof = &iptv_profile_table[i];
			break;
		}
	}
	if (!prof) return; 

	/* ============== step A: merge rules ============== */
	struct iptv_vlan_rule exec_rules[8];
	int exec_count = 0;

	for (j = 0; j < prof->rule_count; j++) {
		struct iptv_vlan_rule r = prof->rules[j];
        
		if (strcmp(profile_name, "none") == 0) {
			if (r.logical_members & PORT_STB_X) {
				r.logical_members = (r.logical_members & ~PORT_STB_X) | stb_ports;
			}
			if (r.logical_untags & PORT_STB_X) {
				r.logical_untags = (r.logical_untags & ~PORT_STB_X) | stb_ports;
			}
			if (r.logical_members & MASK_ALL_LAN_EXC_STB) {
				r.logical_members = (r.logical_members & ~MASK_ALL_LAN_EXC_STB) | (MASK_ALL_LAN & ~stb_ports);
			}
			if (r.logical_untags & MASK_ALL_LAN_EXC_STB) {
				r.logical_untags = (r.logical_untags & ~MASK_ALL_LAN_EXC_STB) | (MASK_ALL_LAN & ~stb_ports);
			}
			if (r.is_iptv && stb_ports == 0) {
				continue;
			}
		}

		int current_vid = r.vid;
        
		if (current_vid == VID_NVRAM_INTERNET) current_vid = nvram_get_int("switch_wan0tagid") & 0x0fff;
		if (current_vid == VID_NVRAM_IPTV)     current_vid = nvram_get_int("switch_wan1tagid") & 0x0fff;
		if (current_vid == VID_NVRAM_VOIP)     current_vid = nvram_get_int("switch_wan2tagid") & 0x0fff;
        
		int current_prio = r.prio;
		if (current_prio < 0) {
			if (r.vid == VID_NVRAM_INTERNET || r.is_internet) current_prio = nvram_get_int("switch_wan0prio") & 0x7;
			else if (r.vid == VID_NVRAM_IPTV) current_prio = nvram_get_int("switch_wan1prio") & 0x7;
			else if (r.vid == VID_NVRAM_VOIP) current_prio = nvram_get_int("switch_wan2prio") & 0x7;
			else current_prio = 0;
		}

		if (current_vid == 0 && r.vid != VID_DEFAULT && !r.is_internet && !r.use_snooper) {
			for (int k = 0; k < exec_count; k++) {
				if (exec_rules[k].vid == VID_DEFAULT && !exec_rules[k].is_internet) {
					unsigned int lan_port = r.logical_members & MASK_ALL_LAN;
					exec_rules[k].logical_members |= lan_port;
					exec_rules[k].logical_untags  |= lan_port;
					break;
				}
			}
			continue; 
		}

		int found = 0;
		for (int k = 0; k < exec_count; k++) {
			if (exec_rules[k].vid == current_vid && current_vid != VID_DEFAULT) {
				exec_rules[k].logical_members |= r.logical_members;
				exec_rules[k].logical_untags  |= r.logical_untags;
				exec_rules[k].is_internet     |= r.is_internet;
				exec_rules[k].use_snooper     |= r.use_snooper;
				exec_rules[k].is_iptv         |= r.is_iptv;
				if (exec_rules[k].prio == 0 && current_prio > 0) exec_rules[k].prio = current_prio;
					found = 1;
				break;
			}
		}
		if (!found) {
			exec_rules[exec_count] = r;
			exec_rules[exec_count].vid = current_vid;
			exec_rules[exec_count].prio = current_prio;
			exec_count++;
		}
	}

	/* run vlan0 at the end for rtk due cpu port pvid issue; run vlan0 entry first, mxl setup_vlan must */
	if (g_rtk) {
		for (int k = 0; k < exec_count - 1; k++) {
			if (exec_rules[k].vid == 0) {
				struct iptv_vlan_rule temp = exec_rules[exec_count - 1];
				exec_rules[exec_count - 1] = exec_rules[k];
				exec_rules[k] = temp;
				break;
			}
		}
	} else {
		for (int k = 1; k < exec_count; k++) {
			if (exec_rules[k].vid == 0) {
				struct iptv_vlan_rule temp = exec_rules[0];
				exec_rules[0] = exec_rules[k];
				exec_rules[k] = temp;
				break;
			}
		}
	}

	/* reset frame type of utp ports of vlan host */
	if (g_rtk) {
		unsigned int any_untag_logical = 0;
		unsigned int tag_only_logical = 0;
        
		for (int k = 0; k < exec_count; k++) {
			any_untag_logical |= (exec_rules[k].logical_untags & MASK_ALL_LAN);
		}
		for (int k = 0; k < exec_count; k++) {
			unsigned int members = exec_rules[k].logical_members & MASK_ALL_LAN;
			unsigned int untags = exec_rules[k].logical_untags & MASK_ALL_LAN;
			tag_only_logical |= (members & ~untags);
		}
		/* make sure the port is w/o untag attribute in any profile rule, then can be judged as tag-only */
		tag_only_logical &= ~any_untag_logical; 
        
		if (tag_only_logical > 0) {
			unsigned int tag_only_phy = get_phy_vlan_mask(tag_only_logical, 0) & 0xFFFF;
			if (tag_only_phy > 0) {
				char mask_str[32];
				snprintf(mask_str, sizeof(mask_str), "0x%x", tag_only_phy);
				eval("rtkswitch", "3952", mask_str);
			}
		}
	}

	/* Clear iptv_ifname before loop to avoid overwrite bugs */
	nvram_set("iptv_ifname", "");
	nvram_set("wan10_enable", "0");
	nvram_set("wan10_ifname", "");
	nvram_set("wan10_proto", "");
	nvram_set("wan11_enable", "0");
	nvram_set("wan11_ifname", "");
	nvram_set("wan11_proto", "");

	/* ============== step B: setup vlans and bridge ============== */
	for (j = 0; j < exec_count; j++) {
		struct iptv_vlan_rule *rule = &exec_rules[j];
		int current_vid = rule->vid;
		int current_prio = rule->prio; 
		char vid_str[16], wan_vif[16], egr_map[16];
        
		sprintf(vid_str, "%d", current_vid);

		int is_hw_bridged = 0;
#if defined(RTCONFIG_EXT_SWITCH_ALL)
		if ((rule->logical_members & MASK_ALL_LAN) && !rule->is_internet) {
			is_hw_bridged = 1;
		}
#endif

		if (current_vid > 0 && !is_hw_bridged) {
			sprintf(wan_vif, "%s.v%d", wan_if, current_vid);
		} else {
			sprintf(wan_vif, "%s", wan_if);
		}

		int has_wan = (rule->logical_members & PORT_CPU_WAN) ? 1 : 0;
		int has_untagged_wan = (rule->logical_members & PORT_UNTAGGED_WAN) ? 1 : 0;
		int has_lan = (rule->logical_members & MASK_ALL_LAN) ? 1 : 0;
        	
		int need_software_bridge = ((current_vid > 0 && has_lan) || has_untagged_wan) ? 1 : 0;
#if defined(RTCONFIG_EXT_SWITCH_ALL)
		if (has_untagged_wan && has_lan) need_software_bridge = 1;
		else need_software_bridge = 0;

		if (strcmp(profile_name, "hinet_mesh") == 0 && current_vid == 1) {
			need_software_bridge = 1;
		}
#endif

		/* 1. hardware configuration (case-2/3) */
#if !defined(RTCONFIG_NETDEV_PER_PORT)
		unsigned int hw_mask = get_phy_vlan_mask(rule->logical_members, rule->logical_untags);

		if (!is_netdev_arch && !(current_vid == 0 && rule->is_internet)) {
			setup_vlan_logical(current_vid, current_prio, rule->logical_members, rule->logical_untags);
		}

		if (strcmp(profile_name, "hinet_mesh") == 0) {
			char nv_name[64], nv_val[32];
			snprintf(nv_name, sizeof(nv_name), "hinet_mesh_vlan%d_mask", current_vid);
			snprintf(nv_val, sizeof(nv_val), "0x%x", hw_mask);
			nvram_set(nv_name, nv_val);

			snprintf(nv_name, sizeof(nv_name), "hinet_mesh_vlan%d_prio", current_vid);
			snprintf(nv_val, sizeof(nv_val), "%d", current_prio);
			nvram_set(nv_name, nv_val);
		}
#endif

		/* 2. Fix race condition: link up wan_if before adding vlan */
		eval("ifconfig", wan_if, "up");

		/* 3. add wan vlan dev */
		if (has_wan && current_vid > 0 && (rule->is_internet || need_software_bridge || rule->is_iptv || strcmp(profile_name, "movistar") == 0)) {
			if (!is_interface_exists(wan_vif)) {
				eval("ip", "link", "add", "link", wan_if, "name", wan_vif, "type", "vlan", "id", vid_str);
				if (current_prio > 0) {
					snprintf(egr_map, sizeof(egr_map), "0:%d", current_prio);
					eval("ip", "link", "set", "dev", wan_vif, "type", "vlan", "egress", egr_map);
				}
				eval("ifconfig", wan_vif, "allmulti", "up");
			}
		}	

		/* 4. create br and add port */
		char br_dev[16] = "";
		if (need_software_bridge && current_vid > 0) {
			sprintf(br_dev, "br%d", br_idx);
            
			if (!is_interface_exists(br_dev)) {
				eval("brctl", "addbr", br_dev);
				/* stp off for dhcp speedup */
				eval("brctl", "stp", br_dev, "off"); 
			}
            
			if (has_wan) eval("brctl", "addif", br_dev, wan_vif);
			if (has_untagged_wan) eval("brctl", "addif", br_dev, wan_if);
            
			eval("bcmmcastctl", "mode", "-i", br_dev, "-p", "1", "-m", "1");
			eval("bcmmcastctl", "mode", "-i", br_dev, "-p", "2", "-m", "1");

			/* --- lan port logic --- */
			if (is_netdev_arch) {
				/* Case-1: pure netdev arch */
				for (int p = 1; p <= 8; p++) {
					unsigned int lan_p = (1 << p);
					if (rule->logical_members & lan_p) {
						const char *lan_if = get_netdev_if(lan_p);
						if (!lan_if) continue;
                      
						eval("ifconfig", (char*)lan_if, "up"); 
						eval("brctl", "delif", "br0", (char*)lan_if); 
                        
						if (rule->logical_untags & lan_p) {
							/* untag host */
							eval("brctl", "addif", br_dev, (char*)lan_if);
						} else {
							/* tagged host */
							char lan_vif[16];
							sprintf(lan_vif, "%s.v%d", lan_if, current_vid);
							if (!is_interface_exists(lan_vif)) {
								eval("ip", "link", "add", "link", (char*)lan_if, "name", lan_vif, "type", "vlan", "id", vid_str);
								eval("ifconfig", lan_vif, "allmulti", "up");
							}
							eval("brctl", "addif", br_dev, lan_vif);
						}
					}
				}
			} else {
			/* Case-2/3: L2 Switch arch */
				if (rule->logical_members & PORT_CPU_LAN) {
					const char *cpu_lan_if = get_cpu_lan_if();
					char lan_vif[16];
					sprintf(lan_vif, "%s.v%d", cpu_lan_if, current_vid);
					if (!is_interface_exists(lan_vif)) {
						eval("ip", "link", "add", "link", (char*)cpu_lan_if, "name", lan_vif, "type", "vlan", "id", vid_str);
						eval("brctl", "addif", br_dev, lan_vif);
						eval("ifconfig", lan_vif, "up");
					}
				}
			}
            
			eval("ifconfig", br_dev, "allmulti", "up");
				br_idx++;
		}

#ifdef RTCONFIG_MULTISERVICE_WAN
		if (strcmp(profile_name, "none") == 0)
			continue;
#endif

		/* ============== step C: config explicit nvram ============== */
        
		/* 1. Setup WAN internet interfaces */
		if (rule->is_internet) {
			char target_wan_if[16];
			if (current_vid > 0) {
				if (need_software_bridge) sprintf(target_wan_if, "br%d", br_idx - 1);
					else strncpy(target_wan_if, wan_vif, sizeof(target_wan_if)); 
				} else {
					strncpy(target_wan_if, wan_if, sizeof(target_wan_if));
			}

			set_wan_phy("");
			add_wan_phy(target_wan_if);
			nvram_set("wan0_ifname", target_wan_if);
			nvram_set("wan0_gw_ifname", target_wan_if);
			nvram_set("switch_wantag_ifname", target_wan_if);
		}

		/* 2. Setup IPTV interface for mcpd (IGMP Proxy) */
		if (rule->is_iptv) {
			if (strcmp(nvram_safe_get("iptv_ifname"), "") == 0) {
				char target_iptv_if[16];
				if (need_software_bridge) sprintf(target_iptv_if, "br%d", br_idx - 1);
				else strncpy(target_iptv_if, wan_vif, sizeof(target_iptv_if)); 
                
				nvram_set("iptv_ifname", target_iptv_if);

				if (rule->use_snooper) {
					nvram_set("wan10_ifname", target_iptv_if);
					nvram_set("wan10_proto", "bridge");
					nvram_set("wan10_enable", "1");
				}
			}
		}

		/* 3. Setup Movistar L3 custom route config */
		if (strcmp(profile_name, "movistar") == 0) {
			char target_movistar_if[16];
			if (need_software_bridge) sprintf(target_movistar_if, "br%d", br_idx - 1);
			else strncpy(target_movistar_if, wan_vif, sizeof(target_movistar_if)); 

			if (current_vid == 2) {
				nvram_set("wan10_ifname", target_movistar_if);
				nvram_set("wan10_proto", "dhcp");
				nvram_set("wan10_enable", "1");
			} else if (current_vid == 3) {
				nvram_set("wan11_ifname", target_movistar_if);
				nvram_set("wan11_proto", "dhcp");
				nvram_set("wan11_enable", "1");
			}
		}
	}
    
	/* 4. Fallback for iptv_ifname (If empty, fallback to wan0_ifname) */
	if (strcmp(nvram_safe_get("iptv_ifname"), "") == 0) {
		nvram_set("iptv_ifname", nvram_safe_get("wan0_ifname"));
	}

	/* ============== step D: flush hw mac table ============== */
	eval("ethctl", wan_if, "phy-power", "down");
	sleep(1);
	eval("ethctl", wan_if, "phy-power", "up");
}

