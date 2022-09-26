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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bcmnvram.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/if_ether.h>	//have in front of <linux/mii.h> to avoid redefinition of 'struct ethhdr'
#include <linux/mii.h>
#include <dirent.h>

#include <shutils.h>
#include <shared.h>
#include <utils.h>
#include <qca.h>
#include <flash_mtd.h>

#define NR_WANLAN_PORT	11
#define DBGOUT		NULL			/* "/dev/console" */
#define QCA8337_IFACE	"eth0"
#define QCA8337_CPUPORT	"0"

enum {
	VLAN_TYPE_STB_VOIP = 0,
	VLAN_TYPE_WAN,
	VLAN_TYPE_WAN_NO_VLAN,	/* Used to bridge WAN/STB for Hinet MOD. */

	VLAN_TYPE_MAX
};

#if defined(GTAXY16000) || defined(RTAX89U)
/* GT-AXY16000 virtual port mapping
 * Assume LAN port closed to 1G WAN port is LAN1.
 */
enum {
	LAN1_PORT=0,
	LAN2_PORT,
	LAN3_PORT,
	LAN4_PORT,
	LAN5_PORT,
	LAN6_PORT,	/* 5 */
	LAN7_PORT,
	LAN8_PORT,
	WAN_PORT=8,
	WAN10GR_PORT,
	WAN10GS_PORT,	/* 10 */

	MAX_WANLAN_PORT
};
#else
#error Define WAN/LAN ports!
#endif

static const char *upstream_iptv_ifaces[16] = {
#if defined(GTAXY16000) || defined(RTAX89U)
	[WANS_DUALWAN_IF_WAN] = "eth3",
	[WANS_DUALWAN_IF_SFPP] = "eth4",
	[WANS_DUALWAN_IF_WAN2] = "eth5",
#else
#error Define WAN interfaces that can be used as upstream port of IPTV.
#endif
};

/* 0:WAN, 1:LAN, 2:WAN2(10G RJ-45), 3: SFP+, first index is switch_stb_x nvram variable.
 * lan_wan_partition[switch_stb_x][0] is virtual port0,
 * lan_wan_partition[switch_stb_x][1] is virtual port1, etc.
 * If it's 2, check it with is_aqr_phy_exist() before using it on GT-AXY16000/GX-AC5400.
 */
static const int lan_wan_partition[9][NR_WANLAN_PORT] = {
	/* L1, L2, L3, L4, L5, L6, L7, L8, W1G, W10GR, W10GS */
	{1,1,1,1,1,1,1,1,0,2,3}, // Normal
	{0,1,1,1,1,1,1,1,0,2,3}, // IPTV STB port = LAN1
	{1,0,1,1,1,1,1,1,0,2,3}, // IPTV STB port = LAN2
	{1,1,0,1,1,1,1,1,0,2,3}, // IPTV STB port = LAN3
	{1,1,1,0,1,1,1,1,0,2,3}, // IPTV STB port = LAN4
	{0,0,1,1,1,1,1,1,0,2,3}, // IPTV STB port = LAN1 & LAN2
	{1,1,0,0,1,1,1,1,0,2,3}, // IPTV STB port = LAN3 & LAN4
	{1,1,1,1,1,1,1,1,1,2,3}  // ALL
};

#if defined(RTCONFIG_BONDING_WAN) || defined(RTCONFIG_LACP)
/* array index:		port number used in wanports_bond, enum bs_port_id.
 * 			0: WAN, 1~8: LAN1~8, 30: 10G base-T (RJ-45), 31: 10G SFP+
 * array element:	virtual port
 * 			e.g. LAN1_PORT ~ LAN8_PORT, WAN_PORT, etc.
 */
static const int bsport_to_vport[32] = {
	WAN_PORT, LAN1_PORT, LAN2_PORT, LAN3_PORT, LAN4_PORT, LAN5_PORT, LAN6_PORT, LAN7_PORT, LAN8_PORT,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	WAN10GR_PORT, WAN10GS_PORT
};
#endif

/* array index:		virtual port mapping enumeration.
 * 			e.g. LAN1_PORT, LAN2_PORT, etc.
 * array element:	PHY address, negative value means absent PHY.
 * 			QCA8337 port X uses PHY X-1
 * 			0x10? means Aquantia AQR107,
 * 			      check it with is_aqr_phy_exist before using it on GT-AXY16000/GX-AC5400.
 * 			0x20? means ethX, ethtool ioctl
 */
#if defined(RTAX89U)
/* HwId: A ~ B, AQR107, AQR113 A1/B0 */
static const int vport_to_phy_addr_hwid_a[MAX_WANLAN_PORT] = {
	10, 9, 4, 3, 2, 1, 0, 6, 11, 0x107, 0x204
};

/* HwId: C, AQR113C */
static const int vport_to_phy_addr_hwid_c[MAX_WANLAN_PORT] = {
	10, 9, 4, 3, 2, 1, 0, 6, 11, 0x108, 0x204
};

static const int *vport_to_phy_addr = vport_to_phy_addr_hwid_a;
#elif defined(GTAXY16000)
/* HwId: A ~ B, AQR107, AQR113 A1/B0 */
static const int vport_to_phy_addr_hwid_a[MAX_WANLAN_PORT] = {
	10, 9, 4, 3, 2, 1, 0, 6, 11, 0x107, 0x204
};

/* HwId: C, exchange LAN3/LAN4, LAN5/LAN6 */
static const int vport_to_phy_addr_hwid_c[MAX_WANLAN_PORT] = {
	10, 9, 3, 4, 1, 2, 0, 6, 11, 0x107, 0x204
};

static const int *vport_to_phy_addr = vport_to_phy_addr_hwid_c;
#else
#error FIXME
#endif

/**
 * The vport_to_iface array is used to get interface name of each virtual
 * port.  If bled need to know TX/RX statistics of LAN1~2, WAN1, WAN2 (AQR107),
 * and 10G SFP+, bled has to find this information from netdev.  So, define
 * this array and implement vport_to_iface_name() function which is used by
 * bled in update_swports_bled().
 *
 * array index:		virtual port mapping enumeration.
 * 			e.g. LAN1_PORT, LAN2_PORT, etc.
 * array element:	Interface name of specific virtual port.
 */
static const char *vport_to_iface[MAX_WANLAN_PORT] = {
	"eth2", "eth1", NULL, NULL, NULL, NULL, NULL, NULL,	/* LAN1~8 */
	"eth3", "eth5", "eth4"					/* WAN1, WAN2 10G RJ-45, 10G SFP+ */
};

/**
 * The vport_to_8337port array is used to convert between virtual port mask
 * and real (QCA8337) port mask.  Because GT-AXY16000 use virtual port mask to
 * generate lanports_mask/wanports_mask and bled need to know TX/RX statistice
 * of some ports which are connected to QCA8337 due to LAN3~7 are connected to
 * QCA8337 directly and LAN8 is connected to QCA8337 indirectly, via AR8033 PHY.
 * So, define this array and implement vportmask_to_rportmask() and
 * rportmask_to_vportmask() functions which are used by bled in
 * update_swports_bled().
 *
 * array index:		virtual port mapping enumeration.
 * 			e.g. LAN1_PORT, LAN2_PORT, etc.
 * array element:	Port mask of QCA8337
 */
static const int vport_to_8337port_hwid_a[MAX_WANLAN_PORT] = {
	-1, -1, 5, 4, 3, 2, 1, 6,				/* LAN1~8, LAN8 is connected to P6 of QCA8337 indirectly. */
	-1, -1, -1						/* WAN1, WAN2 10G RJ-45, 10G SFP+ */
};

#if defined(GTAXY16000)
static const int vport_to_8337port_hwid_c[MAX_WANLAN_PORT] = {
	-1, -1, 4, 5, 2, 3, 1, 6,				/* LAN1~8, LAN8 is connected to P6 of QCA8337 indirectly. */
	-1, -1, -1						/* WAN1, WAN2 10G RJ-45, 10G SFP+ */
};

static const int *vport_to_8337port = vport_to_8337port_hwid_c;
#else
static const int *vport_to_8337port = vport_to_8337port_hwid_a;
#endif

static const unsigned int qca8337_vportmask =
	(1U << LAN3_PORT) | (1U << LAN4_PORT) | (1U << LAN5_PORT) |
	(1U << LAN6_PORT) | (1U << LAN7_PORT) | (1U << LAN8_PORT);

/* array index:		switch_stb_x nvram variable.
 * array element:	platform specific VoIP/STB virtual port bitmask.
 */
static const unsigned int stb_to_mask[7] = { 0,
	(1U << LAN1_PORT),
	(1U << LAN2_PORT),
	(1U << LAN3_PORT),
	(1U << LAN4_PORT),
	(1U << LAN1_PORT) | (1U << LAN2_PORT),
	(1U << LAN3_PORT) | (1U << LAN4_PORT)
};

void reset_qca_switch(void);

#define	CPU_PORT_WAN_MASK	(1U << WAN_PORT)
#define CPU_PORT_LAN_MASK	(0xF)

/* ALL WAN/LAN virtual port bit-mask */
static unsigned int wanlanports_mask = ((1U << WAN_PORT) | (1U << WAN10GR_PORT) | (1U << WAN10GS_PORT) | \
	(1U << LAN1_PORT) | (1U << LAN2_PORT) | (1U << LAN3_PORT) | (1U << LAN4_PORT) | \
	(1U << LAN5_PORT) | (1U << LAN6_PORT) | (1U << LAN7_PORT) | (1U << LAN8_PORT));

/* Final model-specific LAN/WAN/WANS_LAN partition definitions.
 * Because LAN/WAN ports of GT-AXY16000 are distributed on several switches and phys.
 * These ports bitmask below are virtual port bitmask for a pseudo switch covers
 * QCA8075, AR8035, QCA8033, AR8033, AQR107, and SFP+.
 * bit0: VP0, bit1: VP1, bit2: VP2, bit3: VP3, bit4: VP4, bit5: VP5
 */
static unsigned int lan_mask = 0;	/* LAN only. Exclude WAN, WANS_LAN, and generic IPTV port. */
static unsigned int wan_mask = 0;	/* wan_type = WANS_DUALWAN_IF_WAN. Include generic IPTV port. */
static unsigned int wan2_mask = 0;	/* wan_type = WANS_DUALWAN_IF_WAN2. Include generic IPTV port. */
static unsigned int sfpp_mask = 0;	/* wan_type = WANS_DUALWAN_IF_SFPP. Include generic IPTV port. */
static unsigned int wans_lan_mask = 0;	/* wan_type = WANS_DUALWAN_IF_LAN. */

/* RT-N56U's P0, P1, P2, P3, P4 = LAN4, LAN3, LAN2, LAN1, WAN
 * ==> Model-specific virtual port number.
 * array inex:	RT-N56U's port number.
 * array value:	Model-specific virtual port number
 */
static int n56u_to_model_port_mapping[] = {
	LAN4_PORT,	//0000 0000 0001 LAN4
	LAN3_PORT,	//0000 0000 0010 LAN3
	LAN2_PORT,	//0000 0000 0100 LAN2
	LAN1_PORT,	//0000 0000 1000 LAN1
	WAN_PORT,	//0000 0001 0000 WAN
};

#define RTN56U_WAN_GMAC	(1U << 9)

int esw_stb;

/* Model-specific LANx ==> Model-specific virtual PortX.
 * array index:	Model-specific LANx (started from 0).
 * array value:	Model-specific virtual port number.
 */
const int lan_id_to_vport[NR_WANLAN_PORT] = {
	LAN1_PORT,
	LAN2_PORT,
	LAN3_PORT,
	LAN4_PORT,
	LAN5_PORT,
	LAN6_PORT,
	LAN7_PORT,
	LAN8_PORT,
	WAN_PORT,
	WAN10GR_PORT,
	WAN10GS_PORT,
};

void reset_qca_switch(void);

/* Model-specific LANx (started from 0) ==> Model-specific virtual PortX */
static inline int lan_id_to_vport_nr(int id)
{
	return lan_id_to_vport[id];
}

static inline void get_qca8337_port_definition(void)
{
	static char g_HwId[HWID_LENGTH] = { 0 };

	if (*g_HwId == '\0')
		strlcpy(g_HwId, nvram_safe_get("HwId"), sizeof(g_HwId));

#if defined(RTAX89U)
	if (!strcmp(g_HwId, "B") || !strcmp(g_HwId, "A")) {
		vport_to_phy_addr = vport_to_phy_addr_hwid_a;
	} else if (!strcmp(g_HwId, "C")) {
		vport_to_phy_addr = vport_to_phy_addr_hwid_c;
	}
#elif defined(GTAXY16000)
	if (!strcmp(g_HwId, "B") || !strcmp(g_HwId, "A")) {
		vport_to_phy_addr = vport_to_phy_addr_hwid_a;
		vport_to_8337port = vport_to_8337port_hwid_a;
	} else if (!strcmp(g_HwId, "C")) {
		vport_to_phy_addr = vport_to_phy_addr_hwid_c;
		vport_to_8337port = vport_to_8337port_hwid_c;
	}
#endif
}

/**
 * Get WAN port mask
 * @wan_unit:	wan_unit, if negative, select WANS_DUALWAN_IF_WAN
 * @return:	port bitmask
 */
static unsigned int get_wan_port_mask(int wan_unit)
{
	int sw_mode = sw_mode();
	char nv[] = "wanXXXports_maskXXXXXX";

	if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
#if defined(RTCONFIG_AMAS)
		if (!nvram_match("re_mode", "1"))
#endif	/* RTCONFIG_AMAS */
		return 0;

	if (wan_unit <= 0 || wan_unit >= WAN_UNIT_MAX)
		strlcpy(nv, "wanports_mask", sizeof(nv));
	else
		snprintf(nv, sizeof(nv), "wan%dports_mask", wan_unit);
	return nvram_get_int(nv);
}

/**
 * Get LAN port mask
 * @return:	port bitmask
 */
static unsigned int get_lan_port_mask(void)
{
	int sw_mode = sw_mode();
	unsigned int m = nvram_get_int("lanports_mask");

	if (sw_mode == SW_MODE_AP || __mediabridge_mode(sw_mode))
		m = wanlanports_mask;

	return m;
}

#define IPQ807X_MDIO_SYS_DIR	"/sys/devices/platform/soc/90000.mdio/mdio_bus/90000.mdio/"
#define SW_MDIO1_SYS_DIR	"/sys/devices/platform/soc/soc:mdio1/mdio_bus/gpio-0/"
#define AQR_PHY_SYS_DIR		IPQ807X_MDIO_SYS_DIR "90000.mdio:07"
#define AQR113C_PHY_SYS_DIR	SW_MDIO1_SYS_DIR "gpio-0:08"
int is_aqr_phy_exist(void)
{
	static int aqr_phy_absent = -1;

	if (aqr_phy_absent >= 0)
		return !aqr_phy_absent;

	if (!d_exists(IPQ807X_MDIO_SYS_DIR)) {
		dbg("%s hasn't been created, assume AQR PHY exist!\n", IPQ807X_MDIO_SYS_DIR);
		return 1;
	}

	if (d_exists(AQR_PHY_SYS_DIR) || d_exists(AQR113C_PHY_SYS_DIR)) {
		aqr_phy_absent = 0;
	} else {
		aqr_phy_absent = 1;
	}

	return !aqr_phy_absent;
}

/* Return PHY address of AQR107/113/113C based on HwId
 * @return:
 * 	-1: error
 *  0 ~ 31: PHY address
 */
int aqr_phy_addr(void)
{
	static int phy_addr = -1;
	char hwid = '\0';

#if defined(RTAX89U)
        char min_aqr113c_hwid = 'C';
#elif defined(GTAXY16000)
#warning FIXME
        char min_aqr113c_hwid = 'D';
#else
        char min_aqr113c_hwid = 'A';
#endif

	if (phy_addr >= 0)
		return phy_addr;

	memcpy(&hwid, nvram_safe_get("HwId"), 1);
	if (hwid >= min_aqr113c_hwid)
		phy_addr = 8;
	else
		phy_addr = 7;

	return phy_addr;
}

/**
 * Convert (v)port to interface name.
 * @vport:	(virtual) port number
 * @return:
 * 	NULL:	@vport doesn't map to specific interface name.
 *  otherwise:	@vport do map to a specific interface name.
 */
const char *vport_to_iface_name(unsigned int vport)
{
	if (vport >= ARRAY_SIZE(vport_to_iface)) {
		dbg("%s: don't know vport %d\n", __func__, vport);
		return NULL;
	}

	return vport_to_iface[vport];
}

/**
 * Convert interface name to (v)port.
 * NOTE:	LAN3~8 of GT-AXY16000 are not supported due to they share same interface!
 * @iface:	interface name
 * @return:	(virtual) port number
 *  >=0:	(virtual) port number
 *  < 0:	can't find (virtual) port number for @iface.
 */
int iface_name_to_vport(const char *iface)
{
	int ret = -2, i;

	if (!iface)
		return -1;

	for (i = 0; ret < 0 && i < ARRAY_SIZE(vport_to_iface); ++i) {
		if (!vport_to_iface[i] || strcmp(vport_to_iface[i], iface))
			continue;
		ret = i;
	}

	return ret;
}

/**
 * Convert vportmask to real portmask (QCA8337 only).
 * In most platform, vportmask = rportmask.
 * @vportmask:	virtual port mask
 * @return:	real portmask (QCA8337 only)
 */
unsigned int vportmask_to_rportmask(unsigned int vportmask)
{
	int vport;
	unsigned int rportmask = 0, m;

	get_qca8337_port_definition();
	for (vport = 0, m = vportmask;
	     vport < MAX_WANLAN_PORT && m != 0;
	     m >>= 1, vport++)
	{
		if (!(m & 1) || *(vport_to_8337port + vport) < 0)
			continue;

		rportmask |= (1U << *(vport_to_8337port + vport));
	}

	return rportmask;
}

/**
 * Set VLAN to a QCA8337 switch port.
 * @port:	port number of QCA8337 switch.
 * @vid:	VLAN ID
 * @prio:	VLAN PRIO
 * @untag:	if true, untag frame before it egress from @port.
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 */
int set_qca8337_port_vlan(int port, int vid, int prio, int untag)
{
	char *tag_type = untag? "untagged" : "tagged";
	char vid_str[6], port_str[4], prio_str[4];
	char *vmbr_add[] = { "ssdk_sh", SWID_QCA8337, "vlan", "member", "add", vid_str, port_str, tag_type, NULL };
	char *v1mbr_del[] = { "ssdk_sh", SWID_QCA8337, "vlan", "member", "del", "1", port_str, NULL };
	char *defcvid[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "defaultCVid", "set", port_str, vid_str, NULL };
	char *clrdefcvid[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "defaultCVid", "set", port_str, "0", NULL };
	char *defcpri[] = { "ssdk_sh", SWID_QCA8337, "qos", "ptDefaultCpri", "set", port_str, prio_str, NULL };
	char *vegress[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "egress", "set", port_str, tag_type, NULL };

	if (port < 0 || port > 6 || vid < 0 || vid >= 4096 || prio < 0 || prio > 7)
		return -1;

	snprintf(vid_str, sizeof(vid_str), "%d", vid);
	snprintf(port_str, sizeof(port_str), "%d", port);
	snprintf(prio_str, sizeof(prio_str), "%d", prio);

	/* Remove port from default VLAN1 and add to VLAN @vid. */
	_eval(v1mbr_del, DBGOUT, 0, NULL);
	_eval(vmbr_add, DBGOUT, 0, NULL);
	if (untag) {
		_eval(defcvid, DBGOUT, 0, NULL);
		_eval(defcpri, DBGOUT, 0, NULL);
	} else {
		_eval(clrdefcvid, DBGOUT, 0, NULL);
	}
	_eval(vegress, DBGOUT, 0, NULL);

	return 0;
}

/**
 * Set a VLAN
 * @vtype:	VLAN type
 * 	0:	VLAN for STB/VoIP
 * 	1:	VLAN for WAN
 * @upstream_if:upstream interface name
 * @vid:	VLAN ID, 0 ~ 4095.
 * @prio:	VLAN Priority
 * @mbr:	VLAN members
 * @untag:	VLAN members that need untag
 *
 * @return
 * 	0:	success
 *     -1:	invalid parameter
 */
int qca8075_8337_8035_8033_aqr107_vlan_set(int vtype, char *upstream_if, int vid, int prio, unsigned int mbr, unsigned int untag)
{
	unsigned int m = mbr, u = untag, upstream_mask = 0;
	int vport, port, upstream_vport, wan_vlan_br = 0, wan_br = 0;
	char wvlan_if[IFNAMSIZ], qvlan_if[IFNAMSIZ], vid_str[6], prio_str[4], brv_if[IFNAMSIZ];
	char *ventry_create[] = { "ssdk_sh", SWID_QCA8337, "vlan", "entry", "create", vid_str, NULL };
	char *add_upst_viface[] = { "ip", "link", "add", "link", upstream_if, wvlan_if, "type", "vlan", "id", vid_str, NULL };
	char *add_8337_viface[] = { "ip", "link", "add", "link", QCA8337_IFACE, qvlan_if, "type", "vlan", "id", vid_str, NULL };
	char *p0vmbr_add[] = { "ssdk_sh", SWID_QCA8337, "vlan", "member", "add", vid_str, "0", "unmodified", NULL };
	char *set_upst_viface_egress_map[] = { "vconfig", "set_egress_map", wvlan_if, "0", prio_str, NULL };

	dbg("%s: vtype %d upstream_if %s vid %d prio %d mbr 0x%x untag 0x%x\n",
		__func__, vtype, upstream_if, vid, prio, mbr, untag);
	if (!upstream_if || vtype < 0 || vtype >= VLAN_TYPE_MAX) {
		dbg("%s: invalid parameter\n", __func__);
		return -1;
	}

	if (bond_wan_enabled() && !strcmp(upstream_if, "bond1")) {
		uint32_t wmask = nums_str_to_u32_mask(nvram_safe_get("wanports_bond"));
		int b;
		const char *p;

		while ((b = ffs(wmask)) > 0) {
			b--;
			if ((p = bs_port_id_to_iface(b)) != NULL) {
				upstream_vport = iface_name_to_vport(p);
				if (upstream_vport < 0 || upstream_vport >= MAX_WANLAN_PORT) {
					dbg("%s: Can't find vport for upstream iface [%s] of bond1\n", __func__, upstream_if);
					return -1;
				}
				upstream_mask |= 1U << upstream_vport;
			}
			wmask &= ~(1U << b);
		}
	} else {
		upstream_vport = iface_name_to_vport(upstream_if);
		if (upstream_vport < 0 || upstream_vport >= MAX_WANLAN_PORT) {
			dbg("%s: Can't find vport for upstream iface [%s]\n", __func__, upstream_if);
			return -1;
		}
		upstream_mask = 1U << upstream_vport;
	}

	if (vtype == VLAN_TYPE_WAN_NO_VLAN) {
		wan_br = 1;
		vtype = VLAN_TYPE_WAN;
	}
	if (vtype == VLAN_TYPE_WAN && (mbr & ~(1U << WAN_PORT)) != 0)
		wan_vlan_br = 1;

	/* Replace WAN port as selected upstream port. */
	if (mbr & (1U << WAN_PORT)) {
		mbr &= ~(1U << WAN_PORT);
		mbr |= upstream_mask;
	}
	if (untag & (1U << WAN_PORT)) {
		untag &= ~(1U << WAN_PORT);
		untag |= upstream_mask;
	}

	snprintf(vid_str, sizeof(vid_str), "%d", vid);
	snprintf(prio_str, sizeof(prio_str), "%d", prio);
	snprintf(brv_if, sizeof(brv_if), "brv%d", vid);

	if ((vtype == VLAN_TYPE_WAN && wan_vlan_br) || vtype == VLAN_TYPE_STB_VOIP) {
		/* Use bridge to connect WAN and STB/VoIP. */
		eval("brctl", "addbr", brv_if);
		eval("ifconfig", brv_if, "0.0.0.0", "up");

		set_netdev_sysfs_param(brv_if, "bridge/multicast_querier", "1");
		set_netdev_sysfs_param(brv_if, "bridge/multicast_snooping",
			nvram_match("switch_br_no_snooping", "1")? "0" : "1");
	}

	if (vtype == VLAN_TYPE_WAN) {
		if (wan_br) {
			/* In this case, no VLAN on WAN port. */
			strlcpy(wvlan_if, upstream_if, sizeof(wvlan_if));
		} else {
			/* Follow naming rule in set_basic_ifname_vars() on upstream interface. */
			snprintf(wvlan_if, sizeof(wvlan_if), "vlan%d", vid);
			_eval(add_upst_viface, DBGOUT, 0, NULL);
			_eval(set_upst_viface_egress_map, DBGOUT, 0, NULL);
			eval("ifconfig", upstream_if, "0.0.0.0", "up");
		}
		if (wan_vlan_br) {
			eval("brctl", "addif", brv_if, wvlan_if);
		}
		eval("ifconfig", wvlan_if, "0.0.0.0", "up");
	} else if (vtype == VLAN_TYPE_STB_VOIP) {
		snprintf(wvlan_if, sizeof(wvlan_if), "%s.%d", upstream_if, vid);
		_eval(add_upst_viface, DBGOUT, 0, NULL);
		_eval(set_upst_viface_egress_map, DBGOUT, 0, NULL);
		eval("brctl", "addif", brv_if, wvlan_if);
		eval("ifconfig", wvlan_if, "0.0.0.0", "up");
	}

	get_qca8337_port_definition();
	if ((vtype == VLAN_TYPE_WAN && wan_vlan_br) || vtype == VLAN_TYPE_STB_VOIP) {
		if (mbr & qca8337_vportmask) {
			snprintf(qvlan_if, sizeof(qvlan_if), "%s.%d", QCA8337_IFACE, vid);
			_eval(add_8337_viface, DBGOUT, 0, NULL);
			eval("brctl", "addif", brv_if, qvlan_if);
			eval("ifconfig", qvlan_if, "0.0.0.0", "up");

			_eval(ventry_create, DBGOUT, 0, NULL);
			if (vid != 1)
				_eval(p0vmbr_add, DBGOUT, 0, NULL);
		}

		for (vport = 0, m = mbr & ~upstream_mask, u = untag;
		     vport < MAX_WANLAN_PORT && m > 0;
		     vport++, m >>= 1, u >>= 1)
		{
			if (!(m & 1))
				continue;

			if ((port = *(vport_to_8337port + vport)) < 0) {
				/* IPQ807x ports */
				dbg("%s: vport %d iface %s vid %d prio %d u %d\n",
					__func__, vport, vport_to_iface[vport], vid, prio, u & 1);
				eval("brctl", "addif", brv_if, (char*) vport_to_iface[vport]);
			} else {
				/* QCA8337 ports */
				dbg("%s: vport %d port %d vid %d prio %d u %d\n",
					__func__, vport, port, vid, prio, u & 1);
				set_qca8337_port_vlan(port, vid, prio, u & 1);
			}
		}
	}

	return 0;
}

static void is_singtel_mio(int is)
{
	int i;
	char port_str[4], *admit_type = is? "admit_all" : "admit_untagged";
	char *vadmit[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "invlan", "set", port_str, admit_type, NULL };

	/* admit all/untagged frames if @is is true/false, except P0. */
	for (i = 1; i <= 6; ++i) {
		snprintf(port_str, sizeof(port_str), "%d", i);
		_eval(vadmit, DBGOUT, 0, NULL);
	}
}

/**
 * Get link status and/or phy speed of a traditional PHY.
 * @phy:	PHY address
 * @link:	pointer to unsigned integer.
 * @speed:	pointer to unsigned integer.
 * 		If speed != NULL,
 * 			*speed = 1 means 100Mbps
 * 			*speed = 2 means 1000Mbps
 * 			*speed = 3 means 10Gbps
 * 			*speed = 4 means 2.5Gbps
 * 			*speed = 5 means 5Gbps
 * @link_speed:	pointer to unsigned integer which is used to store normal link speed.
 * 		e.g. 100 = 100Mbps, 10000 = 10Gbps, etc.
 * @return
 * 	0:	success
 *     -1:	invalid parameter.
 *     -2:	miireg_read() MII_BMSR failed
 */
static int get_phy_info(unsigned int phy, unsigned int *link, unsigned int *speed, unsigned int *link_speed, phy_info *info)
{
	int r = 0, l = 1, s = 0, lspd = 0;
	char iface[IFNAMSIZ];
	uint32_t uspd;

#if defined(GTAXY16000) || defined(RTAX89U)
	/* Use ssdk_sh port linkstatus/speed get command to minimize number of executed ssdk_sh
	 * and number of accessing MDIO bus.
	 */
	if (phy >= 0 && phy <= 4) {
		/* Exclude LAN8, AR8033, which is connected to P6 of QCA8337.
		 * Because it always has 1G link with switch whether cable is attached or not.
		 */
		r = qca8337_port_speed(phy + 1);
	}
	else if (phy >= 9 && phy <= 11) {
		r = ipq8074_port_speed(phy - 7);
	}
#endif
	else if (phy < 0x20) {
		r = mdio_phy_speed(phy);
	}
	else if (phy >= 0x100 && phy < 0x120) {
		r = aqr_phy_speed(phy & 0x1F);
	}
	else if (phy >= 0x200 && phy < 0x210) {
		snprintf(iface, sizeof(iface), "eth%d", phy & 0xF);
		if (!ethtool_gset(iface, &uspd, NULL))
			r = uspd;
	} else {
		dbg("%s: Unknown PHY address 0x%x\n", __func__, phy);
	}

	lspd = r;
	switch (r) {
	case 5000:
		s = 5;
		break;
	case 2500:
		s = 4;
		break;
	case 10000:
		s = 3;
		break;
	case 1000:
		s = 2;
		break;
	case 100:
		s = 1;
		break;
	case 10:
		s = 0;
		break;
	default:
		l  = lspd = 0;
		break;
	}

	if (link)
		*link = l;
	if (speed)
		*speed = s;
	if (link_speed)
		*link_speed = lspd;
	if (info) {
		if (l) {
			info->link_rate = lspd;
			snprintf(info->state, sizeof(info->state), "up");
			// TODO: need to retreive duplex from driver
			//snprintf(info->duplex, sizeof(info->duplex), "none"); 
		} else {
			snprintf(info->state, sizeof(info->state), "down");
			snprintf(info->duplex, sizeof(info->duplex), "none");
		}
	}

	return 0;
}

/**
 * Get link status and/or phy speed of a virtual port.
 * @vport:	virtual port number
 * @link:	pointer to unsigned integer.
 * 		If link != NULL,
 * 			*link = 0 means link-down
 * 			*link = 1 means link-up.
 * @speed:	pointer to unsigned integer.
 * 		If speed != NULL,
 * 			*speed = 1 means 100Mbps
 * 			*speed = 2 means 1000Mbps
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *  otherwise:	fail
 */
static int get_qca8075_8337_8035_8033_aqr107_vport_info(unsigned int vport, unsigned int *link, unsigned int *speed, phy_info *info)
{
	int phy;

	if (vport >= MAX_WANLAN_PORT || (!link && !speed))
		return -1;

	if (link)
		*link = 0;
	if (speed)
		*speed = 0;

	if (vport == WAN10GR_PORT && !is_aqr_phy_exist())
		return 0;

	get_qca8337_port_definition();
	phy = *(vport_to_phy_addr + vport);
	if (phy < 0) {
		dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
		return -1;
	}

	get_phy_info(phy, link, speed, NULL, info);

	return 0;
}

/**
 * Get linkstatus in accordance with port bit-mask.
 * @mask:	port bit-mask.
 * 		bit0 = VP0, bit1 = VP1, etc.
 * @linkStatus:	link status of all ports that is defined by mask.
 * 		If one port of mask is linked-up, linkStatus is true.
 * @return:	speed.
 * 	0:	not connected.
 *  non-zero:	linkrate.
 */
static int get_qca8075_8337_8035_8033_aqr107_phy_linkStatus(unsigned int mask, unsigned int *linkStatus)
{
	int i,t,speed=0;
	unsigned int value = 0, m;

	m = mask & wanlanports_mask;
	for (i = 0; m > 0 && !value; ++i, m >>= 1) {
		if (!(m & 1))
			continue;

		get_qca8075_8337_8035_8033_aqr107_vport_info(i, &value, (unsigned int*) &t, NULL);
		value &= 0x1;
	}
	*linkStatus = value;

	switch (t) {
	case 0x0:
		speed = 10;
		break;
	case 0x1:
		speed = 100;
		break;
	case 0x2:
		speed = 1000;
		break;
	case 0x3:
		speed = 10000;
		break;
	case 0x4:
		speed = 2500;
		break;
	case 0x5:
		speed = 5000;
		break;
	default:
		speed=0;
		_dprintf("%s: mask %8x t %8x invalid speed!\n", __func__, mask, t);
	}
	return speed;
}


/**
 * Set wanports_mask, wanXports_mask, and lanports_mask based on
 * nvram configuration, @stb, and @stb_bitmask parameters.
 * @stb_bitmask should be platform-specific (v)port bitmask.
 */
static void build_wan_lan_mask(int stb, int stb_bitmask)
{
	int i, unit, type, m, upstream_unit = __get_upstream_wan_unit();
	int wanscap_lan = get_wans_dualwan() & WANSCAP_LAN;
	int wans_lanport = nvram_get_int("wans_lanport");
	int sw_mode = sw_mode();
	char prefix[8], nvram_ports[20];
	unsigned int unused_wan_mask = 0, iptv_mask = 0;

	if (stb < 0 || stb >= ARRAY_SIZE(stb_to_mask)) {
		return;
	}
	if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
		wanscap_lan = 0;

	if (wanscap_lan && (wans_lanport < 1 || wans_lanport > 4)) {
		_dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
		wanscap_lan = 0;
	}

	/* To compatible to original architecture, @stb and @stb_bitmask are exclusive.
	 * @stb is assumed as zero if @stb_bitmask is non-zero value.  Because
	 * "rtkswitch  8 X" specifies STB port configuration via 0 ~ 6 and
	 * "rtkswitch 38 X" specifies VoIP/STB port via RT-N56U's port bitmask.
	 */
	if (stb_bitmask != 0)
		stb = 0;
	if (sw_mode == SW_MODE_ROUTER) {
		if (stb_bitmask != 0)
			iptv_mask = stb_bitmask;
		else
			iptv_mask = stb_to_mask[stb];
	}

	lan_mask = wan_mask = wans_lan_mask = 0;
	for (i = 0; i < NR_WANLAN_PORT; ++i) {
		switch (lan_wan_partition[stb][i]) {
		case 0:
			wan_mask |= 1U << lan_id_to_vport_nr(i);
			break;
		case 1:
			lan_mask |= 1U << lan_id_to_vport_nr(i);
			break;
		case 2:
			if (is_aqr_phy_exist())
				wan2_mask |= 1U << lan_id_to_vport_nr(i);
			break;
		case 3:
			sfpp_mask |= 1U << lan_id_to_vport_nr(i);
			break;
		default:
			_dprintf("%s: Unknown LAN/WAN port definition. (stb %d i %d val %d)\n",
				__func__, stb, i, lan_wan_partition[stb][i]);
		}
	}

	/* One of LAN port is acting as WAN. */
	if (wanscap_lan) {
		wans_lan_mask = 1U << lan_id_to_vport_nr(wans_lanport -1);
		lan_mask &= ~wans_lan_mask;
	}

	unused_wan_mask = wan_mask | wan2_mask | sfpp_mask;
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		snprintf(prefix, sizeof(prefix), "%d", unit);
		snprintf(nvram_ports, sizeof(nvram_ports), "wan%sports_mask", (unit == WAN_UNIT_FIRST)?"":prefix);
		m = 0;	/* In AP/RP/MB mode, all WAN ports are bridged to LAN. */

		if (sw_mode == SW_MODE_ROUTER
#ifdef RTCONFIG_AMAS
		 ||(sw_mode == SW_MODE_AP && nvram_match("re_mode", "1"))
#endif	/* RTCONFIG_AMAS */
		 ) {
			type = get_dualwan_by_unit(unit);

			switch (type) {
			case WANS_DUALWAN_IF_WAN:
				m = wan_mask;
				unused_wan_mask &= ~wan_mask;
				break;
			case WANS_DUALWAN_IF_WAN2:
				m = wan2_mask;
				unused_wan_mask &= ~wan2_mask;
				break;
			case WANS_DUALWAN_IF_SFPP:
				m = sfpp_mask;
				unused_wan_mask &= ~sfpp_mask;
				break;
			case WANS_DUALWAN_IF_LAN:
				m = wans_lan_mask;
				break;
			default:
				nvram_unset(nvram_ports);
				break;
			}

			if (m == 0)
				continue;

			if (unit == upstream_unit)
				m |= iptv_mask;
		}
		nvram_set_int(nvram_ports, m);
	}

	/* Let all unused WAN ports become LAN ports.
	 * 1. 10G RJ-45 and 10G SFP+ can be WAN or LAN, depends on dualwan configuration.
	 * 2. 1G WAN can aggreate with LAN1/LAN2, creates 3Gbps bandwidth. (TODO)
	 */
	lan_mask = (lan_mask | unused_wan_mask) & ~iptv_mask;
	nvram_set_int("lanports_mask", lan_mask);
}

/**
 * Configure LAN/WAN partition base on generic IPTV type.
 * @type:
 * 	0:	Default.
 * 	1:	LAN1
 * 	2:	LAN2
 * 	3:	LAN3
 * 	4:	LAN4
 * 	5:	LAN1+LAN2
 * 	6:	LAN3+LAN4
 */
static void config_qca8075_8337_8035_8033_aqr107_LANWANPartition(int type)
{
	build_wan_lan_mask(type, 0);
	reset_qca_switch();
	dbg("%s: LAN/P.WAN/S.WAN portmask %08x/%08x/%08x Upstream %s (unit %d)\n",
		__func__, lan_mask, nvram_get_int("wanports_mask"), nvram_get_int("wan1ports_mask"),
		get_wan_base_if(), __get_upstream_wan_unit());
}

static void get_qca8075_8337_8035_8033_aqr107_WAN_Speed(unsigned int *speed)
{
	int i, v = -1, t;
	unsigned int m;

	m = (get_wan_port_mask(0) | get_wan_port_mask(1)) & wanlanports_mask;
	for (i = 0; m; ++i, m >>= 1) {
		if (!(m & 1))
			continue;

		get_qca8075_8337_8035_8033_aqr107_vport_info(i, NULL, (unsigned int*) &t, NULL);
		t &= 0x3;
		if (t > v)
			v = t;
	}

	switch (v) {
	case 0x0:
		*speed = 10;
		break;
	case 0x1:
		*speed = 100;
		break;
	case 0x2:
		*speed = 1000;
		break;
	case 0x3:
		*speed = 10000;
		break;
	case 0x4:
		*speed = 2500;
		break;
	case 0x5:
		*speed = 5000;
		break;
	default:
		_dprintf("%s: invalid speed! (%d)\n", __func__, v);
	}
}

/**
 * @vpmask:	Virtual port mask
 * @status:	0: power down PHY; otherwise: power up PHY
 */
static void link_down_up_qca8075_8337_8035_8033_aqr107_PHY(unsigned int vpmask, int status)
{
	int vport, phy, r;
	unsigned int m;
	char iface[IFNAMSIZ];

	vpmask &= wanlanports_mask;
	for (vport = 0, m = vpmask; m; ++vport, m >>= 1) {
		if (!(m & 1))
			continue;
		if (vport >= MAX_WANLAN_PORT) {
			dbg("%s: PHY address is not defined for vport %d\n", __func__, vport);
			continue;
		}

		if (vport == WAN10GR_PORT && !is_aqr_phy_exist())
			return;
		get_qca8337_port_definition();
		phy = *(vport_to_phy_addr + vport);
		if (phy < 0) {
			dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
			return;
		}

		if (phy < 0x20) {
			/* Legacy PHY and QCA8337 PHY */
			if ((r = read_phy_reg(phy, MII_BMCR)) < 0)
				r = 0;
			if (!status)	/* power down PHY */
				r |= BMCR_PDOWN;
			else
				r &= ~(BMCR_PDOWN);
			write_phy_reg(phy, MII_BMCR, r);
		} else if (phy >= 0x100 && phy < 0x120) {
			/* Aquantia PHY */
			if ((r = read_phy_reg(phy & 0x1F, 0x40010009)) < 0)
				r = 0;
			if (!status)	/* power down PHY */
				r |= 1;
			else
				r &= ~(1);
			write_phy_reg(phy & 0x1F, 0x40010009, r);
		} else if (phy >= 0x200 && phy < 0x210) {
			snprintf(iface, sizeof(iface), "eth%d", phy & 0xF);
			eval("ifconfig", iface, (!status)? "down" : "up");
		} else {
			dbg("%s: Unknown PHY address 0x%x\n", __func__, phy);
		}
	}
}

#if 0
/*define structure for software with 64bit*/
typedef struct
{
	uint64_t RxBroad;
	uint64_t RxPause;
	uint64_t RxMulti;
	uint64_t RxFcsErr;
	uint64_t RxAllignErr;
	uint64_t RxRunt;
	uint64_t RxFragment;
	uint64_t Rx64Byte;
	uint64_t Rx128Byte;
	uint64_t Rx256Byte;
	uint64_t Rx512Byte;
	uint64_t Rx1024Byte;
	uint64_t Rx1518Byte;
	uint64_t RxMaxByte;
	uint64_t RxTooLong;
	uint64_t RxGoodByte;
	uint64_t RxBadByte;
	uint64_t RxOverFlow;		/* no this counter for Hawkeye*/
	uint64_t Filtered;			/*no this counter for Hawkeye*/
	uint64_t TxBroad;
	uint64_t TxPause;
	uint64_t TxMulti;
	uint64_t TxUnderRun;
	uint64_t Tx64Byte;
	uint64_t Tx128Byte;
	uint64_t Tx256Byte;
	uint64_t Tx512Byte;
	uint64_t Tx1024Byte;
	uint64_t Tx1518Byte;
	uint64_t TxMaxByte;
	uint64_t TxOverSize;	/*no this counter for Hawkeye*/
	uint64_t TxByte;
	uint64_t TxCollision;
	uint64_t TxAbortCol;
	uint64_t TxMultiCol;
	uint64_t TxSingalCol;
	uint64_t TxExcDefer;
	uint64_t TxDefer;
	uint64_t TxLateCol;
	uint64_t RxUniCast;
	uint64_t TxUniCast;
	uint64_t RxJumboFcsErr;	/* add for  Hawkeye*/
	uint64_t RxJumboAligenErr;	/* add for Hawkeye*/
} fal_mib_counter_t;

#define MISC_CHR_DEV           10
#define UK_MINOR_DEV           254
#define DEV_SWITH_SSDK_PATH    "/dev/switch_ssdk"
#define SW_MAX_API_BUF         2048
#define SW_MAX_API_PARAM       12 /* cmd type + return value + ten parameters */
#define SW_API_PT_MIB_COUNTER_GET        1107
#endif


/**
 * Return MiB(tx_bytes/rx_bytes/tx_pakcets/rx_packets/crc_errors) of @phy.
 * @port:	port id
 * @info:	sruct phy_info to store MiB(tx_bytes/rx_bytes/tx_pakcets/rx_packets/crc_errors)
 * @return:	0 for success
     non-zero for fail
 */
static int get_qca8075_8337_8035_8033_aqr107_port_mib(unsigned int port, phy_info *info)
{
#if 0
	int fd;
	unsigned long arg_val[SW_MAX_API_PARAM] = {0};
	unsigned long rtn = 0;
	fal_mib_counter_t mib_counter = {0};

	if (!info)
		return -1;

	/* even mknod fail we not quit, perhaps the device node exist already */
	mknod(DEV_SWITH_SSDK_PATH, S_IFCHR, makedev(MISC_CHR_DEV, UK_MINOR_DEV));
	if ((fd = open(DEV_SWITH_SSDK_PATH, O_RDWR)) < 0) {
		return -1;
	}
	arg_val[0] = (unsigned long)SW_API_PT_MIB_COUNTER_GET;  // API
	arg_val[1] = (unsigned long)&rtn;
	arg_val[2] = (unsigned long)0;    //device id
	arg_val[3] = (unsigned long)port; //port
	arg_val[4] = (unsigned long)(&mib_counter);

	ioctl(fd, SIOCDEVPRIVATE, arg_val);
	//fprintf(stderr, "rtn=%d, port=%d\n", rtn, port);
	if (rtn == 0) {
		info->tx_bytes = mib_counter.TxByte;
		info->rx_bytes = mib_counter.RxGoodByte;
		info->tx_packets = mib_counter.TxBroad + mib_counter.TxMulti + mib_counter.TxUniCast;
		info->rx_packets = mib_counter.RxBroad + mib_counter.RxMulti + mib_counter.RxUniCast;
		info->crc_errors = mib_counter.RxFcsErr;
		//fprintf(stderr, "tx_bytes=%llu, rx_bytes=%llu, tx_packets=%lu, rx_packets=%lu, crc_errors=%lu\n", 
		//	info->tx_bytes, info->rx_bytes, info->tx_packets, info->rx_packets, info->crc_errors);
	}
	close(fd);
#endif
	return 0;
}

void reset_qca_switch(void)
{
	nvram_unset("vlan_idx");
}

static void set_Vlan_VID(int vid)
{
	char tmp[8];

	snprintf(tmp, sizeof(tmp), "%d", vid);
	nvram_set("vlan_vid", tmp);
}

static void set_Vlan_PRIO(int prio)
{
	char tmp[2];

	snprintf(tmp, sizeof(tmp), "%d", prio);
	nvram_set("vlan_prio", tmp);
}

static int convert_n56u_portmask_to_model_portmask(unsigned int orig)
{
	int i, bit, bitmask;
	bitmask = 0;
	for(i = 0; i < ARRAY_SIZE(n56u_to_model_port_mapping); i++) {
		bit = (1 << i);
		if (orig & bit)
			bitmask |= (1 << n56u_to_model_port_mapping[i]);
	}
	return bitmask;
}

/**
 * @stb_bitmask:	bitmask of STB port(s)
 * 			e.g. bit0 = P0, bit1 = P1, etc.
 */
static void initialize_Vlan(int stb_bitmask)
{
	char *p, wan_base_if[IFNAMSIZ] = "N/A";

	stb_bitmask = convert_n56u_portmask_to_model_portmask(stb_bitmask);
	build_wan_lan_mask(0, stb_bitmask);
	if ((p = get_wan_base_if()) != NULL)
		strlcpy(wan_base_if, p, sizeof(wan_base_if));

	dbg("%s: LAN/P.WAN/S.WAN portmask %08x/%08x/%08x Upstream %s (unit %d)\n",
		__func__, lan_mask, nvram_get_int("wanports_mask"), nvram_get_int("wan1ports_mask"),
		get_wan_base_if(), __get_upstream_wan_unit());
	reset_qca_switch();
}

/**
 * Create VLAN for LAN and/or WAN in accordance with bitmask parameter.
 * @bitmask:
 *  bit15~bit0:		member port bitmask.
 * 	bit0:		RT-N56U port0, LAN4
 * 	bit1:		RT-N56U port1, LAN3
 * 	bit2:		RT-N56U port2, LAN2
 * 	bit3:		RT-N56U port3, LAN1
 * 	bit4:		RT-N56U port4, WAN
 * 	bit8:		RT-N56U port8, LAN_CPU port
 * 	bit9:		RT-N56U port9, WAN_CPU port
 *  bit31~bit16:	untag port bitmask.
 * 	bit16:		RT-N56U port0, LAN4
 * 	bit17:		RT-N56U port1, LAN3
 * 	bit18:		RT-N56U port2, LAN2
 * 	bit19:		RT-N56U port3, LAN1
 * 	bit20:		RT-N56U port4, WAN
 * 	bit24:		RT-N56U port8, LAN_CPU port
 * 	bit25:		RT-N56U port9, WAN_CPU port
 * First Ralink-based model is RT-N56U.
 * Convert RT-N56U-specific bitmask to physical port of your model,
 * base on relationship between physical port and visual WAN/LAN1~4 of that model first.
 */
static void create_Vlan(int bitmask)
{
	const int vid = nvram_get_int("vlan_vid");
	const int prio = nvram_get_int("vlan_prio") & 0x7;
	const int stb_x = nvram_get_int("switch_stb_x");
	unsigned int mbr = bitmask & 0xffff;
	unsigned int untag = (bitmask >> 16) & 0xffff;
	unsigned int mbr_qca, untag_qca;
	int vtype = VLAN_TYPE_STB_VOIP;
	char upstream_if[IFNAMSIZ];

	//convert port mapping
	mbr_qca   = convert_n56u_portmask_to_model_portmask(mbr);
	untag_qca = convert_n56u_portmask_to_model_portmask(untag);
	if ((nvram_match("switch_wantag", "none") && stb_x > 0) ||
	    nvram_match("switch_wantag", "hinet")) {
		vtype = VLAN_TYPE_WAN_NO_VLAN;
	} else if (mbr & RTN56U_WAN_GMAC) {
		/* setup VLAN for WAN (WAN1 or WAN2), not VoIP/STB */
		vtype = VLAN_TYPE_WAN;
	}

	/* selecet upstream port for IPTV port. */
	strlcpy(upstream_if, get_wan_base_if(), sizeof(upstream_if));
	qca8075_8337_8035_8033_aqr107_vlan_set(vtype, upstream_if, vid, prio, mbr_qca, untag_qca);
}

int qca8075_8337_8035_8033_aqr107_ioctl(int val, int val2)
{
	unsigned int value2 = 0;
	int i, max_wan_unit = 0;

#if defined(RTCONFIG_DUALWAN)
	max_wan_unit = 1;
#endif

	switch (val) {
	case 0:
		value2 = rtkswitch_wanPort_phyStatus(-1);
		printf("WAN link status : %u\n", value2);
		break;
	case 3:
		value2 = rtkswitch_lanPorts_phyStatus();
		printf("LAN link status : %u\n", value2);
		break;
	case 8:
		config_qca8075_8337_8035_8033_aqr107_LANWANPartition(val2);
		break;
	case 13:
		get_qca8075_8337_8035_8033_aqr107_WAN_Speed(&value2);
		printf("WAN speed : %u Mbps\n", value2);
		break;
	case 14: // Link up LAN ports
		link_down_up_qca8075_8337_8035_8033_aqr107_PHY(get_lan_port_mask(), 1);
		break;
	case 15: // Link down LAN ports
		link_down_up_qca8075_8337_8035_8033_aqr107_PHY(get_lan_port_mask(), 0);
		break;
	case 16: // Link up ALL ports
		link_down_up_qca8075_8337_8035_8033_aqr107_PHY(wanlanports_mask, 1);
		break;
	case 17: // Link down ALL ports
		link_down_up_qca8075_8337_8035_8033_aqr107_PHY(wanlanports_mask, 0);
		break;
	case 27:
		reset_qca_switch();
		break;
	case 36:
		set_Vlan_VID(val2);
		break;
	case 37:
		set_Vlan_PRIO(val2);
		break;
	case 38:
		initialize_Vlan(val2);
		break;
	case 39:
		create_Vlan(val2);
		break;
	case 40:
		is_singtel_mio(val2);
		break;
	case 114: // link up WAN ports
		for (i = WAN_UNIT_FIRST; i <= max_wan_unit; ++i)
			link_down_up_qca8075_8337_8035_8033_aqr107_PHY(get_wan_port_mask(i), 1);
		break;
	case 115: // link down WAN ports
		for (i = WAN_UNIT_FIRST; i <= max_wan_unit; ++i)
			link_down_up_qca8075_8337_8035_8033_aqr107_PHY(get_wan_port_mask(i), 0);
		break;
	case 200:	/* set LAN port number that is used as WAN port */
		/* Nothing to do, nvram_get_int("wans_lanport ") is enough. */
		break;

	/* unused ioctl command. */
	case 21:	/* reset storm control rate, only Realtek switch platform need. */
	case 22:	/* set unknown unicast storm control rate. RTK switch only. */
	case 23:	/* set unknown multicast storm control rate. RTK switch only. */
	case 24:	/* set multicast storm control rate. RTK switch only. */
	case 25:	/* set broadcast storm rate. RTK switch only. */
	case 29:	/* Set VoIP port.  Not using any more. */
	case 50:	/* Fix-up hwnat for WiFi interface on MTK platform. */
		break;
	default:
		printf("wrong ioctl cmd: %d\n", val);
	}

	return 0;
}

int config_rtkswitch(int argc, char *argv[])
{
	int val;
	int val2 = 0;
	char *cmd = NULL;
	char *cmd2 = NULL;

	if (argc >= 2)
		cmd = argv[1];
	else
		return -1;
	if (argc >= 3)
		cmd2 = argv[2];

	val = (int) strtol(cmd, NULL, 0);
	if (cmd2)
		val2 = (int) strtol(cmd2, NULL, 0);

	return qca8075_8337_8035_8033_aqr107_ioctl(val, val2);
}

unsigned int
rtkswitch_Port_phyStatus(unsigned int port_mask)
{
        unsigned int status = 0;

	get_qca8075_8337_8035_8033_aqr107_phy_linkStatus(port_mask, &status);

        return status;
}

unsigned int
rtkswitch_Port_phyLinkRate(unsigned int port_mask)
{
        unsigned int speed = 0 ,status = 0;

	speed=get_qca8075_8337_8035_8033_aqr107_phy_linkStatus(port_mask, &status);

        return speed;
}


unsigned int
rtkswitch_wanPort_phyStatus(int wan_unit)
{
	unsigned int status = 0;

#if defined(RTCONFIG_BONDING_WAN)
	if (bond_wan_enabled() && sw_mode() == SW_MODE_ROUTER
	 && get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_WAN)
	{
		int r = ethtool_glink("bond1");
		if (r >= 0)
			status = r;
		return status;
	}
#endif

	get_qca8075_8337_8035_8033_aqr107_phy_linkStatus(get_wan_port_mask(wan_unit), &status);

	return status;
}

unsigned int
rtkswitch_lanPorts_phyStatus(void)
{
	unsigned int status = 0;

	get_qca8075_8337_8035_8033_aqr107_phy_linkStatus(get_lan_port_mask(), &status);

	return status;
}

unsigned int
rtkswitch_WanPort_phySpeed(void)
{
	unsigned int speed;

	get_qca8075_8337_8035_8033_aqr107_WAN_Speed(&speed);

	return speed;
}

int rtkswitch_WanPort_linkUp(void)
{
	eval("rtkswitch", "114");

	return 0;
}

int rtkswitch_WanPort_linkDown(void)
{
	eval("rtkswitch", "115");

	return 0;
}

int
rtkswitch_LanPort_linkUp(void)
{
	system("rtkswitch 14");

	return 0;
}

int
rtkswitch_LanPort_linkDown(void)
{
	system("rtkswitch 15");

	return 0;
}

int
rtkswitch_AllPort_linkUp(void)
{
	system("rtkswitch 16");

	return 0;
}

int
rtkswitch_AllPort_linkDown(void)
{
	system("rtkswitch 17");

	return 0;
}

int
rtkswitch_Reset_Storm_Control(void)
{
	system("rtkswitch 21");

	return 0;
}

/**
 * @link:
 * 	0:	no-link
 * 	1:	link-up
 * @speed:
 * 	0,10:		10Mbps		==> 'M'
 * 	1,100:		100Mbps		==> 'M'
 * 	2,1000:		1000Mbps	==> 'G'
 * 	3,10000:	10Gbps		==> 'T'
 * 	4,2500:		2.5Gbps		==> 'Q'
 * 	5,5000:		5Gbps		==> 'F'
 */
static char conv_speed(unsigned int link, unsigned int speed)
{
	char ret = 'X';

	if (link != 1)
		return ret;

	if (speed == 2 || speed == 1000)
		ret = 'G';
	else if (speed == 3 || speed == 10000)
		ret = 'T';
	else if (speed == 4 || speed == 2500)
		ret = 'Q';
	else if (speed == 5 || speed == 5000)
		ret = 'F';
	else
		ret = 'M';

	return ret;
}

void ATE_port_status(int verbose, phy_info_list *list)
{
	int i, len;
	char buf[6 * 11], wbuf[6 * 3], lbuf[6 * 8];
#ifdef RTCONFIG_NEW_PHYMAP
	char cap_buf[64] = {0};
	char wlen = 0, llen = 0;
#endif
	phyState pS;
#if defined(RTAC89U)
	const int wan1g_sfp10g = 1;	/* 10G base-T absent. */
#else
	const int wan1g_sfp10g = 0;
#endif

#ifdef RTCONFIG_NEW_PHYMAP
	phy_port_mapping port_mapping;
	get_phy_port_mapping(&port_mapping);

	for (i = 0; i < port_mapping.count; i++) {
		// Only handle WAN/LAN ports
		if (((port_mapping.port[i].cap & PHY_PORT_CAP_WAN) == 0) && ((port_mapping.port[i].cap & PHY_PORT_CAP_LAN) == 0))
			continue;
		pS.link[i] = 0;
		pS.speed[i] = 0;
		get_qca8075_8337_8035_8033_aqr107_vport_info(lan_id_to_vport_nr(i), &pS.link[i], &pS.speed[i], list ? &list->phy_info[i] : NULL);

		if (list) {
			list->phy_info[i].phy_port_id = port_mapping.port[i].phy_port_id;
			snprintf(list->phy_info[i].label_name, sizeof(list->phy_info[i].label_name), "%s", 
				port_mapping.port[i].label_name);
			list->phy_info[i].cap = port_mapping.port[i].cap;
			snprintf(list->phy_info[i].cap_name, sizeof(list->phy_info[i].cap_name), "%s", 
				get_phy_port_cap_name(port_mapping.port[i].cap, cap_buf, sizeof(cap_buf)));
			/*if (pS.link[i] == 1 && !list->status_and_speed_only)
				get_qca8075_8337_8035_8033_aqr107_port_mib(port_mapping.port[i].phy_port_id, &list->phy_info[i]);*/

			list->count++;
		}
		if (port_mapping.port[i].cap & PHY_PORT_CAP_WAN > 0)
			wlen += sprintf(wbuf+wlen, "%s=%C;", port_mapping.port[i].label_name,
						conv_speed(pS.link[i], pS.speed[i]));
		else
			llen += sprintf(lbuf+llen, "%s=%C;", port_mapping.port[i].label_name,
						conv_speed(pS.link[i], pS.speed[i]));
	}

#else
	memset(&pS, 0, sizeof(pS));
	for (i = 0; i < NR_WANLAN_PORT; i++) {
		get_qca8075_8337_8035_8033_aqr107_vport_info(lan_id_to_vport_nr(i), &pS.link[i], &pS.speed[i], list ? &list->phy_info[i] : NULL);

		if (list) {
			list->phy_info[list->count].phy_port_id = lan_id_to_vport_nr(i);
			if (!list->count) {
				list->phy_info[list->count].cap = PHY_PORT_CAP_WAN;
				snprintf(list->phy_info[list->count].cap_name, sizeof(list->phy_info[i].cap_name), "wan");
				snprintf(list->phy_info[list->count].label_name, sizeof(list->phy_info[list->count].label_name), "W0");
			}
			else {
				list->phy_info[list->count].cap = PHY_PORT_CAP_LAN;
				snprintf(list->phy_info[list->count].cap_name, sizeof(list->phy_info[i].cap_name), "lan");
				snprintf(list->phy_info[list->count].label_name, sizeof(list->phy_info[list->count].label_name), "L%d", 
					list->count);
			}
			/*if (pS.link[i] == 1 && !list->status_and_speed_only)
				get_ipq40xx_port_mib(lan_id_to_port_nr(i), &list->phy_info[i]);*/

			list->count++;
		}
	}

	snprintf(lbuf, sizeof(lbuf), "L1=%C;L2=%C;L3=%C;L4=%C;L5=%C;L6=%C;L7=%C;L8=%C;",
		conv_speed(pS.link[LAN1_PORT], pS.speed[LAN1_PORT]),
		conv_speed(pS.link[LAN2_PORT], pS.speed[LAN2_PORT]),
		conv_speed(pS.link[LAN3_PORT], pS.speed[LAN3_PORT]),
		conv_speed(pS.link[LAN4_PORT], pS.speed[LAN4_PORT]),
		conv_speed(pS.link[LAN5_PORT], pS.speed[LAN5_PORT]),
		conv_speed(pS.link[LAN6_PORT], pS.speed[LAN6_PORT]),
		conv_speed(pS.link[LAN7_PORT], pS.speed[LAN7_PORT]),
		conv_speed(pS.link[LAN8_PORT], pS.speed[LAN8_PORT]));
	if (wan1g_sfp10g) {
		/* RT-AC89U */
		snprintf(wbuf, sizeof(wbuf), "W0=%C;W2=%C;",
			conv_speed(pS.link[WAN_PORT], pS.speed[WAN_PORT]),
			conv_speed(pS.link[WAN10GS_PORT], pS.speed[WAN10GS_PORT]));
	} else {
		/* GT-AXY16000/RT-AX89U */
		snprintf(wbuf, sizeof(wbuf), "W0=%C;W1=%C;W2=%C;",
			conv_speed(pS.link[WAN_PORT], pS.speed[WAN_PORT]),
			conv_speed(pS.link[WAN10GR_PORT], pS.speed[WAN10GR_PORT]),
			conv_speed(pS.link[WAN10GS_PORT], pS.speed[WAN10GS_PORT]));
	}
#endif // #ifdef RTCONFIG_NEW_PHYMAP

	strlcpy(buf, wbuf, sizeof(buf));
	strlcat(buf, lbuf, sizeof(buf));
	if (verbose)
		puts(buf);
}

/* Callback function which is used to fin brvX interface, X must be number.
 * @return:
 * 	0:	d->d_name is not brvX interface.
 *  non-zero:	d->d_name is brvx interface.
 */
static int brvx_filter(const struct dirent *d)
{
	const char *p;

	if (!d || strncmp(d->d_name, "brv", 3))
		return 0;

	p = d->d_name + 3;
	while (*p != '\0') {
		if (!isdigit(*p))
			return 0;
		p++;
	}

	return 1;
}

void upgrade_aqr113c_fw(void)
{
#if defined(RTAX89U)
	char *fw_name = "/lib/firmware/RT-AX89U_aqr113c.cld";
#elif defined(GTAXY16000)
	char *fw_name = "/lib/firmware/GT-AXY16000_aqr113c.cld";
#endif
	int id1, id2, fw, build, r, aqr_addr = aqr_phy_addr();
	char addr[sizeof("32XX")], ver[sizeof("255.255.15XXX")];
	time_t t1;

	if (!is_aqr_phy_exist() || aqr_addr < 0 || pidof("aq-fw-download") > 0)
		return;

	id1 = read_phy_reg(aqr_addr, 0x40070002);
	id2 = read_phy_reg(aqr_addr, 0x40070003);
	if (id1 != 0x31c3 || (id2 & 0xFFF0) != 0x1c10)
		return;

	nvram_set("aqr_act_swver", "");
	snprintf(addr, sizeof(addr), "%d", aqr_addr);
	eval("aq-fw-download", "-w", fw_name, "miireg", addr);

	t1 = uptime();
	do {
		fw = read_phy_reg(aqr_addr, 0x401e0020);
		build = read_phy_reg(aqr_addr, 0x401ec885);
		if (fw < 0  || build < 0) {
			dbg("%s: Can't get AQR PHY firmware version.\n", __func__);
			sleep(1);
			continue;
		} else if (fw == 0xFFFF || build == 0xFFFF) {
			dbg("wait 1s\n");
			sleep(1);
			continue;
		}
		break;

	} while ((uptime() - t1) < 5);

	r = read_phy_reg(aqr_addr, 0x40010009);
	if (r >= 0 && (r & 1)) {
		r &= ~(1U);
		write_phy_reg(aqr_addr, 0x40010009, r);
	}

	dbg("AQR PHY @ %d firmware %d.%d build %X.%X\n", aqr_addr,
		(fw >> 8) & 0xFF, fw & 0xFF, (build >> 4) & 0xF, build & 0xF);
	snprintf(ver, sizeof(ver), "%d.%d.%X", (fw >> 8) & 0xFF, fw & 0xFF, (build >> 4) & 0xF);
	nvram_set("aqr_act_swver", ver);
}

void __pre_config_switch(void)
{
	const int *paddr;
	int i, j, r1, nr_brvx, nr_brif;
	struct dirent **brvx = NULL, **brif = NULL;
	char vlan1_if[IFNAMSIZ], port_str[4], vid_str[6] = "1", tag_type[16], adm_type[16];
	char brif_path[sizeof("/sys/class/net/X/brifXXXXX") + IFNAMSIZ];
	char *aqr_ssdk_port = "6"; /* GMAC5, AQR107 */
	char *autoneg[] = { "ssdk_sh", SWID_IPQ807X, "port", "autoNeg", "restart", aqr_ssdk_port, NULL };
	char *vlan_flush[] = { "ssdk_sh", SWID_QCA8337, "vlan", "entry", "flush", NULL };
	char *create_vlan1[] = { "ssdk_sh", SWID_QCA8337, "vlan", "entry", "create", vid_str, NULL };
	char *vmbr_add[] = { "ssdk_sh", SWID_QCA8337, "vlan", "member", "add", vid_str, port_str, tag_type, NULL };
	char *vadmit[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "invlan", "set", port_str, adm_type, NULL };
	char *defcvid[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "defaultCVid", "set", port_str, vid_str, NULL };
	char *vegress[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "egress", "set", port_str, tag_type, NULL };
	char *vingress[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "ingress", "set", port_str, "check", NULL };

	_eval(autoneg, DBGOUT, 0, NULL);

	get_qca8337_port_definition();

	/* If AQR PHY doesn't exist, remove related settings. */
	if (!is_aqr_phy_exist()) {
		upstream_iptv_ifaces[WANS_DUALWAN_IF_WAN2] = NULL;
		vport_to_iface[WAN10GR_PORT] = NULL;
		wanlanports_mask &= ~(1U << WAN10GR_PORT);
		logmessage("system", "AQR PHY firmware: N/A");

	} else {
		int id1, id2, aqr_addr = aqr_phy_addr();
		char id_str[sizeof("0xXXXXXXXXYYY")];

		/* 10G link status is shown on LED2 (GREEN, closed to WAN) of AQR107.
		 * Don't show 10G link status on same set of LED, LED1 (YELLOW, closed to WAN),
		 * due to YELLOW and GREEN are mixed and GREEN basically disappear.
		 * Fix setAllLedNormal() and __handle_led_onoff_button() too if definition changed.
		 */
		if ((r1 = read_phy_reg(aqr_addr, 0x401EC431)) < 0)
			r1 = 0;
		write_phy_reg(aqr_addr, 0x401EC431, r1 & ~(1U << 7));

		/* If LED are inhibited, turn off embedded LED. */
		if (inhibit_led_on() && aqr_addr >= 0) {
			write_phy_reg(aqr_addr, 0x401EC430, 0x00);
			write_phy_reg(aqr_addr, 0x401EC431, 0x00);
			write_phy_reg(aqr_addr, 0x401EC432, 0x00);
		}

		id1 = read_phy_reg(aqr_addr, 0x40070002);
		id2 = read_phy_reg(aqr_addr, 0x40070003);
		if (id1 >= 0 && id2 >= 0) {
			snprintf(id_str, sizeof(id_str), "0x%08x", (id1 & 0xFFFF) << 16 | (id2 & 0xFFF0));
			nvram_set("aqr_chip_id", id_str);
		}

		/* Print AQR firmware version. */
		for (i = 0, paddr = vport_to_phy_addr; i < MAX_WANLAN_PORT; ++i, ++paddr) {
			int fw, build;

			/* only accept AQR phy */
			if (*paddr < 0x100 || *paddr >= 0x120)
				continue;

			fw = read_phy_reg(*paddr & 0xFF, 0x401e0020);
			build = read_phy_reg(*paddr & 0xFF, 0x401ec885);
			if (fw < 0  || build < 0) {
				dbg("%s: Can't get AQR PHY firmware version.\n", __func__);
			} else {
				char ver[sizeof("255.255.15XXX")];
				dbg("AQR PHY @ %d firmware %d.%d build %X.%X\n", *paddr & 0xFF,
					(fw >> 8) & 0xFF, fw & 0xFF, (build >> 4) & 0xF, build & 0xF);
				snprintf(ver, sizeof(ver), "%d.%d.%X",
					(fw >> 8) & 0xFF, fw & 0xFF, (build >> 4) & 0xF);
				nvram_set("aqr_act_swver", ver);
			}
		}
	}

	/* Remove all brvXXX bridge interfaces that are used to bridge WAN and STB/VoIP. */
	nr_brvx = scandir(SYS_CLASS_NET, &brvx, brvx_filter, alphasort);
	for (i = 0; i < nr_brvx; ++i) {
		snprintf(brif_path, sizeof(brif_path), "%s/%s/brif", SYS_CLASS_NET, brvx[i]->d_name);
		nr_brif = scandir(brif_path, &brif, NULL, alphasort);
		if (nr_brif <= 0) {
			free(brvx[i]);
			continue;
		}

		for (j = 0; j < nr_brif; ++j) {
			eval("brctl", "delif", brvx[i]->d_name, brif[j]->d_name);
		}
		free(brif);
		free(brvx[i]);
	}
	free(brvx);

#if defined(RTCONFIG_AMAS_WGN)
	/* Set P0 ~ P6 of QCA8337 as untouched, otherwise, RE's guest client can't get IP address
	 * due to VLAN 501/502 DHCP request/reply frames are untagged by QCA8337.
	 */
	for (i = 0; i <= 6; ++i) {
		snprintf(port_str, sizeof(port_str), "%d", i);
		strlcpy(tag_type, "untouched", sizeof(tag_type));
		_eval(vegress, DBGOUT, 0, NULL);
	}
#endif

	if (sw_mode() != SW_MODE_ROUTER || !iptv_enabled() || !sw_bridge_iptv_different_switches())
		return;

	snprintf(vlan1_if, sizeof(vlan1_if), "%s.%s", QCA8337_IFACE, vid_str);
	eval("ip", "link", "add", "link", QCA8337_IFACE, "name", vlan1_if, "type", "vlan", "id", vid_str);
	eval("ifconfig", QCA8337_IFACE, "0.0.0.0", "up");
	eval("ifconfig", vlan1_if, "0.0.0.0", "up");

	/* Flush VLAN entry and create VLAN1 for LAN. */
	_eval(vlan_flush, DBGOUT, 0, NULL);
	_eval(create_vlan1, DBGOUT, 0, NULL);

	/* Add all ports to VLAN1, accept untagged frames only, and egress untagged. */
	for (i = 0; i <= 6; ++i) {
		snprintf(port_str, sizeof(port_str), "%d", i);
		strlcpy(tag_type, (i == 0)? "tagged" : "untagged", sizeof(tag_type));
		_eval(vmbr_add, DBGOUT, 0, NULL);

#if defined(RTCONFIG_AMAS_WGN)
		/* Accept tagged/untagged frames both, otherwise, RE's guest client can't get IP address
		 * if IPTV feature is enabled. Because tagged DHCP request frames are blocked by QCA8337.
		 */
		strlcpy(adm_type, "admit_all", sizeof(adm_type));
		_eval(vadmit, DBGOUT, 0, NULL);
		_eval(defcvid, DBGOUT, 0, NULL);
#else
		strlcpy(adm_type, (i == 0)? "admit_all" : "admit_untagged", sizeof(adm_type));
		_eval(vadmit, DBGOUT, 0, NULL);
		_eval(defcvid, DBGOUT, 0, NULL);

		strlcpy(tag_type, (i == 0)? "untouched" : "untagged", sizeof(tag_type));
		_eval(vegress, DBGOUT, 0, NULL);
#endif
		_eval(vingress, DBGOUT, 0, NULL);
	}
}

void __post_config_switch(void)
{
	int i, speed, r, ipg = 0;
	char *aqr_ssdk_port = "6"; /* GMAC5, AQR107 */
	char port[sizeof("1XX")], speed_str[sizeof("10000XXX")], adv_str[sizeof("0xVVVVXXX")];
	char *ipq807x_p1_8023az[] = { "ssdk_sh", SWID_IPQ807X, "port", "ieee8023az", "set", "1", "disable", NULL };
	char *qca8337_px_8023az[] = { "ssdk_sh", SWID_QCA8337, "port", "ieee8023az", "set", port, "disable", NULL };
	char *p0_vegress[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "egress", "set", QCA8337_CPUPORT, "untouched", NULL };
	char *autoadv[] = { "ssdk_sh", SWID_IPQ807X, "port", "autoAdv", "set", aqr_ssdk_port, adv_str, NULL };
	char *fix_aqr_speed[] = { "ssdk_sh", SWID_IPQ807X, "port", "speed", "set", aqr_ssdk_port, speed_str, NULL };
	char read_ipg_cmd[] = "ssdk_sh " SWID_IPQ807X " debug reg get 0x7000 4";
	char write_ipg_cmd[sizeof("ssdk_sh sw0 debug reg set 0x7000 0xXXXXXXXX 4XXX")];
#if defined(GTAXY16000) || defined(RTAX89U)
	unsigned int rx_afe_2 = 0xf67;
	char rx_afe_2_str[sizeof("0x00000000XXX")] = "0xf67";
	char *adj_uniphy2_rx_afe_2[] = { "devmem", "0x07a107c4", "w", rx_afe_2_str, NULL };
#endif

#if defined(GTAXY16000) || defined(RTAX89U)
	/* Improve GT-AXY16000 SFP+ DAC compatability. */
	if (sscanf(nvram_safe_get("qca_uniphy2_rx_afe_2"), "%x", &rx_afe_2) == 1)
		snprintf(rx_afe_2_str, sizeof(rx_afe_2_str), "0x%x", rx_afe_2 & 0x1FFF);
	_eval(adj_uniphy2_rx_afe_2, DBGOUT, 0, NULL);
#endif

	/* Always turn off IEEE 802.3az support on IPQ8074 port 1 and QCA8337 port 1~5. */
	_eval(ipq807x_p1_8023az, DBGOUT, 0, NULL);
	for (i = 1; i <= 5; ++i) {
		snprintf(port, sizeof(port), "%d", i);
		_eval(qca8337_px_8023az, DBGOUT, 0, NULL);
	}

	/* Fixed 10G base-T link-speed. */
	speed = nvram_get_int("aqr_link_speed");
	if (speed == 1000)
		snprintf(adv_str, sizeof(adv_str), "0x%x", 1U << 9);
	else if (speed == 2500)
		snprintf(adv_str, sizeof(adv_str), "0x%x", 1U << 12);
	else if (speed == 5000)
		snprintf(adv_str, sizeof(adv_str), "0x%x", 1U << 13);
	else if (speed == 10000)
		snprintf(adv_str, sizeof(adv_str), "0x%x", 1U << 14);
	else
		*adv_str = '\0';

	if (*adv_str) {
		_eval(autoadv, DBGOUT, 0, NULL);

		snprintf(speed_str, sizeof(speed_str), "%d", speed);
		_eval(fix_aqr_speed, DBGOUT, 0, NULL);
		dbg("Fixed 10G base-T link-speed as %dMbps, advertise mask %s!\n", speed, adv_str);
	}

	/* Set IPG of port 6 that is wired to Aquantia 10G. */
	if (!(r = parse_ssdk_sh(read_ipg_cmd, "%*[^:]:%x", 1, &ipg))) {
		if (nvram_match("aqr_ipg", "128"))
			ipg = (ipg & 0xFFFFF0FF) | 0x900;	/* 128 bit times */
		else {
			ipg &= 0xFFFFF0FF;			/*  96 bit times */
			if (!nvram_match("aqr_ipg", "96"))
				nvram_set("aqr_ipg", "96");
		}

		snprintf(write_ipg_cmd, sizeof(write_ipg_cmd), "ssdk_sh " SWID_IPQ807X " debug reg set 0x7000 0x%x 4", ipg);
		if ((r = parse_ssdk_sh(write_ipg_cmd, NULL, 0)) != 0) {
			dbg("%s: write IPG failed, cmd [%s], return %d\n", __func__, write_ipg_cmd, r);
		}
	}

	if (sw_mode() != SW_MODE_ROUTER || !iptv_enabled())
		return;

	_eval(p0_vegress, DBGOUT, 0, NULL);
}

/* Set acceleration type of 10G base-T/10G SFP+ as "PPE + NSS" or "NSS only"
 * by setting MAC learning/packet action as "ENABLE"/"FORWARD" or "DISABLE"/"RDTCPU".
 * When hardware NAT ON, 1Gbps LAN may get unstable download throughput, about 600Mbps,
 * upload throughput is stable at 940Mbps at same time. Wireless TPT seems okay at same time.
 * Some environment get stable 940Mbps download throughput after enable flowcontrol (pause-frame)
 * manually after negotiation done. Another environment do need to change acceleration type of
 * XGMAC from "PPE + NSS" to "NSS only".
 */
void __post_ecm(void)
{
	const char *enable_ppe_str[] = { "ENABLE", "FORWARD" };
	const char *disable_ppe_str[] = { "DISABLE", "RDTCPU" };
	int flush = 0, act, enable_ppe;
	char xgmac_port[4], mac_learn[sizeof("DISABLEXXX")], pkt_act[sizeof("FORWARDXXX")];
	char mac_learn_result[sizeof("DISABLEXXX")], pkt_act_result[sizeof("FORWARDXXX")];
	char *set_cmd[] = { "ssdk_sh", SWID_IPQ807X, "fdb", "ptLearnCtrl", "set", xgmac_port, mac_learn, pkt_act, NULL };
	char *flush_cmd[] = { "ssdk_sh", SWID_IPQ807X, "fdb", "entry", "flush", "1", NULL };
	char get_cmd[sizeof("ssdk_sh " SWID_IPQ807X " fdb ptLearnCtrl get XXX")];
	struct xgmac_defs_s {
		int xgmac_port;
		int wanscap;
		char *nv;
		int (*exist_func)(void);
	} xgmac_defs_tbl[] = {
		{ 6, WANSCAP_WAN2, "aqr_hwnat_type", is_aqr_phy_exist },
		{ 5, WANSCAP_SFPP, "sfpp_hwnat_type", NULL },

		{ -1, 0, NULL, NULL }
	}, *p;

	if (nvram_match("qca_sfe", "0"))
		return;

	for (p = &xgmac_defs_tbl[0]; p->nv != NULL; ++p) {
		if (p->exist_func && !p->exist_func())
			continue;

		enable_ppe = act = nvram_get_int(p->nv);
		if (act < 0 || act > 2) {
			act = 0;
			nvram_set_int(p->nv, act);
		}
		if (act == 2)
			enable_ppe = 0;

		if (!act) {
			/* If 10G base-T/10G SFP+ is one of WAN port and acceleration type is auto, select "NSS only". */
			if ((get_wans_dualwan() & p->wanscap) != 0)
				enable_ppe = 0;
			else
				enable_ppe = 1;
		}

		/* Example:
		 * SSDK Init OK![Learn Ctrl]:ENABLE[Action]:FORWARD
		 *operation done.
		 *
		 * SSDK Init OK![Learn Ctrl]:DISABLE[Action]:RDTCPU
		 *operation done.
		 */
		*mac_learn_result = *pkt_act_result = '\0';
		snprintf(get_cmd, sizeof(get_cmd), "ssdk_sh " SWID_IPQ807X " fdb ptLearnCtrl get %d", p->xgmac_port);
		if (parse_ssdk_sh(get_cmd, "%*[^:]:%[^[]%*[^:]:%s", 2, mac_learn_result, pkt_act_result)) {
			dbg("%s: Execute and parse [%s] failed. (mac_learn_result [%s] pkt_act_result [%s])\n",
				__func__, mac_learn_result, pkt_act_result);
			continue;
		}

		snprintf(xgmac_port, sizeof(xgmac_port), "%d", p->xgmac_port);
		if (enable_ppe) {
			strlcpy(mac_learn, enable_ppe_str[0], sizeof(mac_learn));
			strlcpy(pkt_act, enable_ppe_str[1], sizeof(pkt_act));
		} else {
			strlcpy(mac_learn, disable_ppe_str[0], sizeof(mac_learn));
			strlcpy(pkt_act, disable_ppe_str[1], sizeof(pkt_act));
		}

		/* Don't set setting if it same as current. */
		if (!strcmp(mac_learn, mac_learn_result) && !strcmp(pkt_act, pkt_act_result))
			continue;

		_eval(set_cmd, DBGOUT, 0, NULL);
		flush++;
	}

	if (flush > 0)
		_eval(flush_cmd, DBGOUT, 0, NULL);
}

void __post_start_lan(void)
{
	char br_if[IFNAMSIZ];

	strlcpy(br_if, nvram_get("lan_ifname")? : nvram_default_get("lan_ifname"), sizeof(br_if));
	set_netdev_sysfs_param(br_if, "bridge/multicast_querier", "1");
	set_netdev_sysfs_param(br_if, "bridge/multicast_snooping",
		nvram_match("switch_br0_no_snooping", "1")? "0" : "1");
}

void __post_start_lan_wl(void)
{
	__post_start_lan();
}

int __sw_based_iptv(void)
{
	/* GT-AXY16000 always use software bridge to implement IPTV feature.
	 * If we support LAN3~8 as upstream port of IPTV and all VoIP/STB
	 * ports on LAN3~8, we can support IPTV by just configure QCA8337
	 * switch.  Return zero if all above conditions true.
	 */
	return 1;
}

/**
 * GT-AXY16000's WAN1~2/LAN1~2 are IPQ807x ports and LAN3~8 are QCA8337 ports.
 * If IPTV upstream port is WAN1~2 and QCA8337 port is selected as STB port,
 * e.g. all ISP IPTV profiles or pure STB port(s) on LAN3/4, or IPTV upstream
 * port is LAN5~8 (not supported yet) and IPQ807X port is selected as STB
 * port, we need to use VLAN to seperate ingress frames of STB port.  VLAN ID
 * is returned by get_sw_bridge_iptv_vid().
 * @return:
 * 	0:	WAN/IPTV ports on same switch or IPTV is not enabled.
 * 	1:	IPTV is enabled, WAN and IPTV port are different switch.
 */
int __sw_bridge_iptv_different_switches(void)
{
	int stb_x = nvram_get_int("switch_stb_x");
	const int upstream_wanif = get_dualwan_by_unit(get_upstream_wan_unit()); /* WANS_DUALWAN_IF_XXX */
	const char *no_lan_port_profiles[] = { "spark", "2degrees", "slingshot", "orcon", "voda_nz",
		"tpg", "iinet", "aapt", "intronode", "amaysim", "dodo", "iprimus", NULL
	}, **p;
	char switch_wantag[65];

	strlcpy(switch_wantag, nvram_safe_get("switch_wantag"), sizeof(switch_wantag));
	if (*switch_wantag == '\0')
		return 0;

	if (stb_x < 0 || stb_x > 6)
		stb_x = 0;

	/* If ISP IPTV profile doesn't need any LAN port, return 0. */
	for (p = &no_lan_port_profiles[0]; *p != NULL; ++p) {
		if (!strcmp(switch_wantag, *p))
			return 0;
	}

	if (upstream_wanif == WANS_DUALWAN_IF_WAN
	 || upstream_wanif == WANS_DUALWAN_IF_WAN2
	 || upstream_wanif == WANS_DUALWAN_IF_SFPP
	 ) {
		if (!strcmp(switch_wantag, "none") &&
		    (stb_x == 1 || stb_x == 2 || stb_x == 5))	/* LAN1,2,1+2 */
			return 0;
		return 1;					/* LAN3,4,3+4 and all ISP IPTV profiles */
	} else if (upstream_wanif == WANS_DUALWAN_IF_LAN) {
		if (!strcmp(switch_wantag, "none") &&
		    (stb_x == 1 || stb_x == 2 || stb_x == 5))	/* LAN1,2,1+2 */
			return 1;
		return 0;					/* LAN3,4,3+4 and all ISP IPTV profiles */
	}

	dbg("%s: unknown iptv upstream wanif type [%d]\n", __func__, upstream_wanif);

	return 0;
}

/* Return wan_base_if for start_vlan() and selectable upstream port for IPTV.
 * @wan_base_if:	pointer to buffer, minimal length is IFNAMSIZ.
 * @return:		pointer to base interface name for start_vlan().
 */
char *__get_wan_base_if(char *wan_base_if)
{
	int unit, wanif_type;

	if (!wan_base_if)
		return NULL;

	/* Select upstream port of IPTV profile based on configuration at run-time. */
	*wan_base_if = '\0';
	for (unit = WAN_UNIT_FIRST; *wan_base_if == '\0' && unit < WAN_UNIT_MAX; ++unit) {
		wanif_type = get_dualwan_by_unit(unit);
		if (!upstream_iptv_ifaces[wanif_type] || *upstream_iptv_ifaces[wanif_type] == '\0')
			continue;

#if defined(RTCONFIG_BONDING_WAN)
		if (wanif_type == WANS_DUALWAN_IF_WAN && sw_mode() == SW_MODE_ROUTER && bond_wan_enabled()) {
			strlcpy(wan_base_if, "bond1", IFNAMSIZ);
		} else
#endif
			strlcpy(wan_base_if, upstream_iptv_ifaces[wanif_type], IFNAMSIZ);
	}

	return wan_base_if;
}

/* Return wan unit of upstream port for IPTV.
 * This function must same wan unit that returned by get_wan_base_if()!
 * @return:	wan unit of upstream port of IPTV.
 */
int __get_upstream_wan_unit(void)
{
	int i, wanif_type, unit = -1;

	for (i = WAN_UNIT_FIRST; unit < 0 && i < WAN_UNIT_MAX; ++i) {
		wanif_type = get_dualwan_by_unit(i);
		if (wanif_type != WANS_DUALWAN_IF_WAN
		 && wanif_type != WANS_DUALWAN_IF_WAN2
		 && wanif_type != WANS_DUALWAN_IF_SFPP)
			continue;

		unit = i;
	}

	return unit;
}

#if defined(RTCONFIG_BONDING_WAN)
/** Helper function of get_bonding_port_status().
 * Convert bonding slave port definition that is used in wanports_bond to our virtual port definition
 * and get link status/speed of it.
 * @bs_port:	bonding slave port number, 0: WAN, 1~8: LAN1~8, 30: 10G base-T (RJ-45), 31: 10G SFP+
 * @return:
 *  <= 0:	disconnected
 *  otherwise:	link speed
 */
int __get_bonding_port_status(enum bs_port_id bs_port)
{
	int vport, phy, link = 0, speed = 0;

	if (bs_port < 0 || bs_port >= ARRAY_SIZE(bsport_to_vport))
		return 0;

	vport = bsport_to_vport[bs_port];
	if (vport == WAN10GR_PORT && !is_aqr_phy_exist())
		return 0;

	get_qca8337_port_definition();
	phy = vport_to_phy_addr[vport];
	if (phy < 0) {
		dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
		return 0;
	}
	get_phy_info(phy, &link, NULL, &speed, NULL);

	return link? speed : 0;
}
#endif

#if defined(RTCONFIG_BONDING_WAN) || defined(RTCONFIG_LACP)
/** Convert bs_port_id to interface name.
 * @bs_port:	enum bs_port_id
 * @return:	pointer to interface name or NULL.
 *  NULL:	@bs_port doesn't have interface name or error.
 *  otherwise:	interface name.
 */
const char *bs_port_id_to_iface(enum bs_port_id bs_port)
{
	int vport;

	if (bs_port < 0 || bs_port >= ARRAY_SIZE(bsport_to_vport))
		return NULL;

	vport = bsport_to_vport[bs_port];
	if (vport == WAN10GR_PORT && !is_aqr_phy_exist())
		return NULL;

	return vport_to_iface_name(vport);
}
#endif

void set_jumbo_frame(void)
{
	unsigned int m, vport, mtu = 1500;
	int vlan1_enabled;
	const char *p;
	char ifname[IFNAMSIZ], mtu_iface[sizeof("9000XXX")], mtu_frame[sizeof("9000XX")];
	char *ifconfig_argv[] = { "ifconfig", ifname, "mtu", mtu_iface, NULL };
	char *qca8337_framesize[] = { "ssdk_sh", SWID_QCA8337, "misc", "frameMaxSize", "set", mtu_frame, NULL };

	if (!nvram_contains_word("rc_support", "switchctrl"))
		return;

	if (nvram_get_int("jumbo_frame_enable"))
		mtu = 9000;

	snprintf(mtu_iface, sizeof(mtu_iface), "%d", mtu);
	snprintf(mtu_frame, sizeof(mtu_frame), "%d", mtu + 18);
	vlan1_enabled = (sw_mode() == SW_MODE_ROUTER) && iptv_enabled() && sw_bridge_iptv_different_switches();

	/* Enable jumbo frame of QCA8337 switch. */
	_eval(qca8337_framesize, DBGOUT, 0, NULL);
	/* If IPTV is enabled and VLAN1 (eth0.1) is created on QCA8337,
	 * we have to set MTU of base interface prior to set MTU of VLAN interface on it.
	 */
	strlcpy(ifname, "eth0", sizeof(ifname));
	_eval(ifconfig_argv, DBGOUT, 0, NULL);
	if (vlan1_enabled) {
		strlcpy(ifname, "eth0.1", sizeof(ifname));
		_eval(ifconfig_argv, DBGOUT, 0, NULL);
	}

	m = get_lan_port_mask();
	for (vport = 0; m > 0 && vport < MAX_WANLAN_PORT ; vport++, m >>= 1) {
		if (!(m & 1) || !(p = vport_to_iface_name(vport)))
			continue;
		strlcpy(ifname, p, sizeof(ifname));
		_eval(ifconfig_argv, NULL, 0, NULL);
	}
}

/* Platform-specific function of wgn_sysdep_swtich_unset()
 * Unconfigure VLAN settings that is used to connect AiMesh guest network.
 * @vid:	VLAN ID
 */
void __wgn_sysdep_swtich_unset(int vid)
{
	char vid_str[6];
	char *delete_vlan[] = { "ssdk_sh", SWID_QCA8337, "vlan", "entry", "del", vid_str, NULL };

	snprintf(vid_str, sizeof(vid_str), "%d", vid);
	_eval(delete_vlan, DBGOUT, 0, NULL);
}

/* Platform-specific function of wgn_sysdep_swtich_set()
 * Unconfigure VLAN settings that is used to connect AiMesh guest network.
 * @vid:	VLAN ID
 */
void __wgn_sysdep_swtich_set(int vid)
{
	int i;
	char port_str[4], vid_str[6];
	char *create_vlan[] = { "ssdk_sh", SWID_QCA8337, "vlan", "entry", "create", vid_str, NULL };
	char *vmbr_add[] = { "ssdk_sh", SWID_QCA8337, "vlan", "member", "add", vid_str, port_str, "unmodified", NULL };

	snprintf(vid_str, sizeof(vid_str), "%d", vid);
	_eval(create_vlan, DBGOUT, 0, NULL);
	for (i = 0; i <= 6; ++i) {
		snprintf(port_str, sizeof(port_str), "%d", i);
		_eval(vmbr_add, DBGOUT, 0, NULL);
	}
}

#ifdef RTCONFIG_NEW_PHYMAP
extern int get_trunk_port_mapping(int trunk_port_value)
{
	return trunk_port_value;
}

/* phy port related start */
void get_phy_port_mapping(phy_port_mapping *port_mapping)
{
	static phy_port_mapping port_mapping_static = {
#if defined(RTAC89U)
		.count = 10,
		.port[0] = { .phy_port_id = LAN1_PORT, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[1] = { .phy_port_id = LAN2_PORT, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = LAN3_PORT, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = LAN4_PORT, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = LAN5_PORT, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[5] = { .phy_port_id = LAN6_PORT, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[6] = { .phy_port_id = LAN7_PORT, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[7] = { .phy_port_id = LAN8_PORT, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[8] = { .phy_port_id = WAN_PORT, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = NULL },
		.port[9] = { .phy_port_id = WAN10GS_PORT, .label_name = "W2", .cap = (PHY_PORT_CAP_WAN2 | PHY_PORT_CAP_SFPP), .max_rate = 10000, .ifname = NULL }
#elif defined(RTAX89U) || defined(GTAXY16000)
		.count = 13,
		.port[0] = { .phy_port_id = LAN1_PORT, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[1] = { .phy_port_id = LAN2_PORT, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = LAN3_PORT, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = LAN4_PORT, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = LAN5_PORT, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[5] = { .phy_port_id = LAN6_PORT, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[6] = { .phy_port_id = LAN7_PORT, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[7] = { .phy_port_id = LAN8_PORT, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[8] = { .phy_port_id = WAN_PORT, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = NULL },
		.port[9] = { .phy_port_id = WAN10GR_PORT, .label_name = "W1", .cap = (PHY_PORT_CAP_WAN | PHY_PORT_CAP_WAN2), .max_rate = 10000, .ifname = NULL },
		.port[10] = { .phy_port_id = WAN10GS_PORT, .label_name = "W2", .cap = (PHY_PORT_CAP_WAN | PHY_PORT_CAP_WAN3 | PHY_PORT_CAP_SFPP), .max_rate = 10000, .ifname = NULL },
		.port[11] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[12] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#else
		#error "port_mapping is not defined."
#endif
	};

	if (!port_mapping)
		return;

	memcpy(port_mapping, &port_mapping_static, sizeof(phy_port_mapping));

	add_sw_cap(port_mapping);
	swap_wanlan(port_mapping);
	return;
}

/* phy port related end.*/
#endif

#if defined(RTCONFIG_FRS_FEEDBACK)
static const struct sfpp_eeprom_value_defs {
	int byte;
	unsigned char value;
	char *description;
} g_sfpp_eeprom_value_defs[] = {
	/* Byte 0, Description */
	{  0, 0x01, "GBIC" },
	{  0, 0x02, "Module soldered to motherboard" },
	{  0, 0x03, "SFP/SFP+" },

	{ -1,    0, NULL }
};

static const struct sfpp_eeprom_bit_defs {
	int byte;
	int bit;
	char *description;
} g_sfpp_eeprom_bit_defs[] = {
	/* Byte 3, 10G Ethernet Compliance Codes */
	{  3, 7, "10G Base-ER" },
	{  3, 6, "10G Base-LRM" },
	{  3, 5, "10G Base-LR" },
	{  3, 4, "10G base-SR" },
	{  3, 3, "1X SX" },
	{  3, 2, "1X LX" },
	{  3, 1, "1X Copper Active" },
	{  3, 0, "1X Copper Passive" },

	/* Byte 6, Ethernet Compliance Codes */
	{  6, 7, "BASE-PX" },
	{  6, 6, "BASE-BX10" },
	{  6, 5, "100BASE-FX" },
	{  6, 4, "100BASE-LX/LX10" },
	{  6, 3, "1000BASE-T" },
	{  6, 2, "1000BASE-CX" },
	{  6, 1, "1000BASE-LX" },
	{  6, 0, "1000BASE-SX" },

	/* Byte 7, Fiber Channel Link Length */
	{  7, 7, "very long distance" },
	{  7, 6, "short distance" },
	{  7, 5, "intermediate distance" },
	{  7, 4, "long distance" },
	{  7, 3, "medium distance" },

	/* Byte 8, SFP+ Cable Technology */
	{  8, 3, "Active Cable" },
	{  8, 2, "Passive Cable" },

	/* Byte 9, Fiber Channel Transmission Media */
	{  9, 7, "Twin Axial Pair" },
	{  9, 6, "Twisted Pair" },
	{  9, 5, "Miniature Coax" },
	{  9, 4, "Video Coax" },
	{  9, 3, "Multimode, 62.5um" },
	{  9, 2, "Multimode, 50um" },
	{  9, 0, "Single mode" },

	/* Byte 10, Fiber Channel Speed */
	{ 10, 7, "1200 MBytes/s" },
	{ 10, 6, "800 MBytes/s" },
	{ 10, 5, "1600 MBytes/s" },
	{ 10, 4, "400 MBytes/s" },
	{ 10, 3, "3200 MBytes/s" },
	{ 10, 2, "200 MBytes/s" },
	{ 10, 0, "100 MBytes/s" },

	/* Byte 93, Enhanced Options */
	{ 93, 7, "Opt. Alarm/warning flags implemented for all monitored quantities" },
	{ 93, 6, "Opt. soft TX_DISABLE control and monitoring implemented" },
	{ 93, 5, "Opt. soft TX_FAULT monitoring implemented" },
	{ 93, 4, "Opt. soft RX_LOS monitoring implemented" },
	{ 93, 3, "Opt. soft RATE_SELECT control and monitoring implemented" },
	{ 93, 2, "Opt. Application Select control implemented per SFF-8079" },
	{ 93, 1, "Opt. soft Rate Select control implemented per SFF-8431" },

	{ -1, 0, NULL }
};

static void trim(char *str)
{
	char *end;

	if (!str)
		return;

	end = str + strlen(str) - 1;
	while (end > str && isspace(*end)) {
		*end-- = '\0';
	}
}

/* Parse specific byte value in EEPROM A0 of SFP+ module */
static int parse_sfpp_eeprom_a0_val(FILE *fp, int byte, unsigned char val)
{
	const struct sfpp_eeprom_value_defs *p;

	if (!fp || byte < 0 || byte >= 256)
		return -1;

	fprintf(fp, "Byte %2X: %2X", byte, val);
	for (p = &g_sfpp_eeprom_value_defs[0]; p->byte >= 0 && p->byte < 256 && p->description; ++p) {
		if (p->byte != byte || val != p->value)
			continue;

		fprintf(fp, ", %s", p->description);
	}
	fprintf(fp, "\n");

	return 0;
}

/* Parse specific byte bits in EEPROM A0 of SFP+ module */
static int parse_sfpp_eeprom_bits(FILE *fp, int byte, unsigned char val)
{
	const struct sfpp_eeprom_bit_defs *p;

	if (!fp || byte < 0 || byte >= 256 || !val)
		return -1;

	fprintf(fp, "Byte %2X: %2X", byte, val);
	for (p = &g_sfpp_eeprom_bit_defs[0]; p->byte >= 0 && p->byte < 256 && p->description; ++p) {
		if (p->byte != byte || !(val & (1U << p->bit)))
			continue;

		fprintf(fp, ", %s", p->description);
	}
	fprintf(fp, "\n");

	return 0;
}

static int parse_sfpp_eeprom_a0(FILE *fp, unsigned char a0[256])
{
	char v_name[16 + 1] = { 0 }, v_pn[16 + 1] = { 0 }, v_rev[4 + 1] = { 0 };
	char v_sn[16 + 1] = { 0 }, v_dcode[8 + 1] = { 0 };

	if (!fp || !a0)
		return -1;

	/* Parsing most interesting registers. See SFF-8472 */
	memcpy(v_name, a0 + 20, 16);
	trim(v_name);
	memcpy(v_pn, a0 + 40, 16);
	trim(v_pn);
	memcpy(v_rev, a0 + 56, 4);
	trim(v_rev);
	memcpy(v_sn, a0 + 68, 16);
	trim(v_sn);
	memcpy(v_dcode, a0 + 84, 8);
	trim(v_dcode);
	fprintf(fp, "VENDOR [%s] OUI %02X-%02X-%02X PN [%s] REV [%s] SN [%s] DATE CODE [%s]\n",
		v_name, *(a0 + 37), *(a0 + 38), *(a0 + 39), v_pn, v_rev, v_sn, v_dcode);
	parse_sfpp_eeprom_a0_val(fp, 0, *a0);
	parse_sfpp_eeprom_bits(fp, 3, *(a0 + 3));
	parse_sfpp_eeprom_bits(fp, 6, *(a0 + 6));
	fprintf(fp, "Byte %2X: %2X, %d Mbps\n", 12, *(a0 + 12), *(a0 + 12) * 100);
	parse_sfpp_eeprom_bits(fp, 7, *(a0 + 7));
	parse_sfpp_eeprom_bits(fp, 8, *(a0 + 8));
	parse_sfpp_eeprom_bits(fp, 9, *(a0 + 9));
	parse_sfpp_eeprom_bits(fp, 10, *(a0 + 10));

	/* Maximum link length of each type cable */
	if (*(a0 + 14))
		fprintf(fp, "Byte %2X: %2X, single-mode cable max. length %dkm\n", 14, *(a0 + 14), *(a0 + 14));
	if (*(a0 + 15))
		fprintf(fp, "Byte %2X: %2X, single-mode cable max. length %dm\n", 15, *(a0 + 15), *(a0 + 15) * 100);
	if (*(a0 + 17))
		fprintf(fp, "Byte %2X: %2X, 62.5um OM1 cable max. length %dm\n", 17, *(a0 + 17), *(a0 + 17) * 10);
	if (*(a0 + 16))
		fprintf(fp, "Byte %2X: %2X, 50um OM2 cable max. length %dm\n", 16, *(a0 + 16), *(a0 + 16) * 10);
	if (*(a0 + 19))
		fprintf(fp, "Byte %2X: %2X, 50um OM3 cable max. length %dm\n", 19, *(a0 + 19), *(a0 + 19) * 10);
	if (*(a0 + 18))
		fprintf(fp, "Byte %2X: %2X, 50um OM4 cable max. length %dm\n", 18, *(a0 + 18), *(a0 + 18) * 10);

	parse_sfpp_eeprom_bits(fp, 93, *(a0 + 93));

	return 0;
}

/* Detect I2C slave devices, dump slave address 0x50 and d ump slave address 0x51. */
static int dump_sfpp_eeprom(FILE *fp)
{
	int bus = 0, addr = 0x50;
	int r, rlen = 0, found = 0;
	FILE *fp_res;
	char cmd[sizeof("i2cdump -y XXX 0xXXXYYY")];
	char line[4 + 3 * 16 + 4 + 16 + 10];
	unsigned char tmp[16], a0[256] = { 0 }, a2[256] = { 0 }, *p;

	if (!fp)
		return -1;

	fprintf(fp, "\n\n######## Dump I2C bus & EEPROM in SFP+ module ########\n\n");
	/* Example: i2cdetect -y -r 0
	 *      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	 * 00:          -- -- -- -- -- -- -- -- -- -- -- -- --
	 * 10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	 * 20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	 * 30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	 * 40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	 * 50: 50 51 -- -- -- -- -- -- -- -- -- -- -- -- -- --
	 * 60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	 * 70: -- -- -- -- -- -- -- --
	 */
	snprintf(cmd, sizeof(cmd), "i2cdetect -y -r %d", bus);
	if (!(fp_res = popen(cmd, "r"))) {
		fprintf(fp, "cmd [%s] failed. errno %d (%s)\n", cmd, errno, strerror(errno));
		return -2;
	}
	fprintf(fp, "Detect I2C bus %d:\n", bus);
	while (fgets(line, sizeof(line), fp_res) != NULL) {
		fprintf(fp, "%s", line);
	}
	pclose(fp_res);
	fprintf(fp, "\n\n");

	/****** Address A0 ******/
	/* Example: i2cdump -y 0 0x50
	 *      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
	 * 00: 03 04 21 01 00 00 00 00 04 00 00 00 67 00 00 00    ??!?....?...g...
	 * 10: 00 00 01 00 4f 45 4d 20 20 20 20 20 20 20 20 20    ..?.OEM
	 * 20: 20 20 20 20 00 00 40 20 53 46 50 2d 44 41 43 2d        ..@ SFP-DAC-
	 * 30: 31 4d 20 20 20 20 20 20 30 33 20 20 01 00 00 63    1M      03  ?..c
	 * 40: 00 00 00 00 54 4f 32 30 30 39 31 36 30 32 30 20    ....TO200916020
	 * 50: 20 20 20 20 32 30 30 39 31 36 20 20 00 00 00 79        200916  ...y
	 * 60: 80 00 11 b2 73 06 49 0f 47 61 3b 52 ee a3 2b 51    ?.??s?I?Ga;R??+Q
	 * 70: b1 92 e9 53 4e 00 00 00 00 00 88 97 92 8e 85 0b    ???SN.....??????
	 * 80: 43 4f 50 51 41 41 36 4a 41 42 33 37 2d 30 39 36    COPQAA6JAB37-096
	 * 90: 31 2d 30 33 56 30 33 20 01 00 46 00 00 00 00 cf    1-03V03 ?.F....?
	 * a0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
	 * b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
	 * c0: 53 46 50 2d 48 31 30 47 42 2d 43 55 31 4d 20 20    SFP-H10GB-CU1M
	 * d0: 20 20 20 20 30 39 00 00 00 00 00 00 00 00 00 b6        09.........?
	 * e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
	 * f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
	 */
	snprintf(cmd, sizeof(cmd), "i2cdump -y %d 0x%x", bus, addr);
	if (!(fp_res = popen(cmd, "r"))) {
		fprintf(fp, "cmd [%s] failed. errno %d (%s)\n", cmd, errno, strerror(errno));
		return -3;
	}

	fprintf(fp, "Hexdump I2C bus %d addr 0x%x:\n", bus, addr);
	p = a0;
	while (fgets(line, sizeof(line), fp_res) != NULL) {
		fprintf(fp, "%s", line);
		if (!strncmp(line, "00: ", 4))
			found = 1;
		if (!found)
			continue;
		*(line + 3 + 3 * 16) = '\0';	/* remove ascii character */

		/* EEPROM doesn't exist. */
		if (strstr(line + 4, "XX")) {
			found = -1;
			break;
		}
		r = sscanf(line + 4, "%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx "
			"%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx",
			tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5,
			tmp + 6, tmp + 7, tmp + 8, tmp + 9, tmp + 10, tmp + 11,
			tmp + 12, tmp + 13, tmp + 14, tmp + 15);
		if (r > 0) {
			if ((rlen + r) > sizeof(a0)) {
				fprintf(fp, "Size of a0 (%d) is not enough (%d)!\n", sizeof(a0), rlen + r);
				rlen = sizeof(a0) - rlen;
			}
			memcpy(p, tmp, r);
			p += r;
			rlen += r;
		}

		if (r < 16)
			break;
	}
	pclose(fp_res);
	if (found < 0) {
		fprintf(fp, "EEPROM %2X doesn't exist, maybe SFP+ module doesn't exist.\n", addr * 2);
		return 0;
	}
	fprintf(fp, "\n");

	fprintf(fp, "Readed %d bytes from EEPROM of SFP+ module\n", rlen);
	parse_sfpp_eeprom_a0(fp, a0);
	fprintf(fp, "\n\n");

	/****** Address A2 ******/

	addr = 0x51;
	snprintf(cmd, sizeof(cmd), "i2cdump -y %d 0x%x", bus, addr);
	if (!(fp_res = popen(cmd, "r"))) {
		fprintf(fp, "cmd [%s] failed. errno %d (%s)\n", cmd, errno, strerror(errno));
		return -4;
	}

	fprintf(fp, "Hexdump I2C bus %d addr 0x%x:\n", bus, addr);
	rlen = found = 0;
	p = a2;
	while (fgets(line, sizeof(line), fp_res) != NULL) {
		fprintf(fp, "%s", line);
		if (!strncmp(line, "00: ", 4))
			found = 1;
		if (!found)
			continue;
		*(line + 3 + 3 * 16) = '\0';	/* remove ascii character */

		/* EEPROM doesn't exist. */
		if (strstr(line + 4, "XX")) {
			found = -1;
			break;
		}
		r = sscanf(line + 4, "%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx "
			"%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %*[^\n]",
			tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5,
			tmp + 6, tmp + 7, tmp + 8, tmp + 9, tmp + 10, tmp + 11,
			tmp + 12, tmp + 13, tmp + 14, tmp + 15);

		if (r > 0) {
			if ((rlen + r) > sizeof(a2)) {
				fprintf(fp, "Size of a2 (%d) is not enough (%d)!\n", sizeof(a2), rlen + r);
				rlen = sizeof(a2) - rlen;
			}
			memcpy(p, tmp, r);
			p += r;
			rlen += r;
		}

		if (r < 16)
			break;
	}
	pclose(fp_res);
	if (found < 0) {
		fprintf(fp, "EEPROM %2X doesn't exist, maybe SFP+ module doesn't exist.\n", addr * 2);
		return 0;
	}

	return 0;
}

static char *aqr_id_to_name(int id)
{
	/* Reference to drivers/net/phy/aquantia.c */
	if ((id & 0xFFFFFFF0) == (0x03a1b4e2 & 0xFFFFFFF0))
		return "AQR107";
	else if ((id & 0xFFFFFFF0) == (0x31c31c41 & 0xFFFFFFF0))
		return "AQR113";
	else if ((id & 0xFFFFFFF0) == (0x31c31C10 & 0xFFFFFFF0))
		return "AQR113C";

	return 0;
}

/* Dump important AQR chip registers. */
static int dump_aqr_regs(FILE *fp)
{
	int fw, build, id2, id3, v, val, phy = aqr_phy_addr();
	char *dstr;
	unsigned char factory_hwid[HWID_LENGTH] = { 0 };

	if (!fp)
		return -1;

	fprintf(fp, "\n\n######## Dump AQR chip registers ########\n");
	FRead(factory_hwid, OFFSET_HWID, sizeof(factory_hwid));
	fprintf(fp, "SoC [%d], HwID: factory [%s] nvram [%s], HwVer [%s]\n",
		get_soc_version_major(), factory_hwid, nvram_get("HwId")? : "NULL", nvram_get("HwVer")? : "NULL");
	fprintf(fp, "AQR chip exist [%d] PHY addr [%d] blver [%s]\n", is_aqr_phy_exist(), phy, nvram_safe_get("blver"));

	fw = read_phy_reg(phy, 0x401e0020);
	build = read_phy_reg(phy, 0x401ec885);
	if (fw < 0  || build < 0) {
		fprintf(fp, "%s: Can't get AQR PHY firmware version.\n", __func__);
	} else {
		fprintf(fp, "AQR PHY @ %d firmware %d.%d build %X.%X\n", phy,
			(fw >> 8) & 0xFF, fw & 0xFF, (build >> 4) & 0xF, build & 0xF);
	}

	id2 = read_phy_reg(phy, 0x40070002);
	id3 = read_phy_reg(phy, 0x40070003);
	if (id2 < 0 || id3 < 0) {
		fprintf(fp, "Can't get AQR chip ID (%04hu%04hu).\n", (unsigned short)id2, (unsigned short)id3);
	} else {
		fprintf(fp, "AQR chip ID %04hx%04hx", (unsigned short)id2, (unsigned short)id3);
		dstr = aqr_id_to_name((id2 & 0xFFFF) << 16 | (id3 & 0xFFFF));
		if (dstr)
			fprintf(fp, " (%s)", dstr);
		fprintf(fp, "\n");
	}

	val = read_phy_reg(phy, 0x4004e812);
	if (val < 0) {
		fprintf(fp, "Can't get XGMAC status.\n");
	} else {
		fprintf(fp, "Register 4.e812: %04hx", (unsigned short)val);
		/* bit F:E */
		v = (val >> 14) & 3;
		if (v == 0) {
			dstr = "N/A";
		} else if (v == 1) {
			dstr = "Not complete (okay if no cable)";
		} else if (v == 2) {
			dstr = "Complete";
		} else {
			dstr = NULL;
		}
		if (dstr)
			fprintf(fp, ", Autoneg status:%s", dstr);

		/* bit D, RX Link UP */
		fprintf(fp, ", RX link %s", (val & (1U << 13))? "up" : "down");

		/* bit C, TX Ready */
		fprintf(fp, ", TX %s", (val & (1U << 12))? "Ready" : "Not ready");

		/* bit B:8, System Interface rate */
		v = (val >> 8) & 0xF;
		if (v == 0) {
			dstr = "Power down";
		} else if (v == 1) {
			dstr = "100M";
		} else if (v == 2) {
			dstr = "1G";
		} else if (v == 3) {
			dstr = "10G";
		} else if (v == 4) {
			dstr = "2.5G";
		} else if (v == 5) {
			dstr = "5G";
		} else if (v == 6) {
			dstr = "10M";
		} else {
			dstr = NULL;
		}
		if (dstr)
			fprintf(fp, ", System Interface Rate %s", dstr);

		/* bit 0, Read for Data (PHY link status) */
		fprintf(fp, ", PHY link %s", (val & (1U))? "up" : "down");

		fprintf(fp, "\n");
	}

	return 0;
}

static const struct reg_bit_defs_s {
	int sbit;	/* lowest start bit */
	int blen;	/* bit-length of the field */
	char *description;
} g_8337_reg_4[] = {
	{ 30,  1, "RMII master" },
	{ 29,  1, "RMII slave" },
	{ 28,  1, "Inverse RMII clock" },
	{ 27,  1, "RMII clock edge" },
	{ 26,  1, "RGMII EN" },
	{ 25,  1, "MAC0 RGMII TXCLK delay EN" },
	{ 22,  2, "MAC0 RGMII TXCLK delay" },
	{ 20,  2, "MAC0 RGMII RXCLK delay" },
	{ 19,  1, "SGMII CLK125M RX edge" },
	{ 18,  1, "SGMII CLK125M TX edge" },
	{ 17,  1, "FX100_EN" },
	{ 14,  1, "MAC0_PHY_GMII_EN" },
	{ 13,  1, "MAC0_PHY_GMII_TXCLK inv." },
	{ 12,  1, "MAC0_PHY_GMII_RXCLK inv." },
	{ 11,  1, "MAC0_PHY_MII_PIPE_RXCLK inv." },
	{ 10,  1, "MAC0_PHY_MII_EN" },
	{  9,  1, "MAC0_PHY_MII_TXCLK inv." },
	{  8,  1, "MAC0_PHY_MII_RXCLK inv." },
	{  7,  1, "MAC0_SGMII_EN" },
	{  6,  1, "MAC0_GMII_EN" },
	{  5,  1, "MAC0_MAC_TXCLK inv." },
	{  4,  1, "MAC0_MAC_RXCLK inv." },
	{  2,  1, "MAC0_MAC_MII_EN" },
	{  1,  1, "MAC0_MAC_MII_TXCLK inv." },
	{  0,  1, "MAc0_MAC_MII_RXCLK inv." },
	{ -1, -1, NULL }
}, g_8337_reg_8[] = {
	{ 26,  1, "MAC5_RGMII_EN" },
	{ 25,  1, "MAC5_RGMII_TXCLK_DELAY_EN" },
	{ 24,  1, "MAC5_RGMII_RXCLK_DELAY_EN" },
	{ 22,  2, "MAC5_RGMII_TXCLK_DELAY" },
	{ 20,  2, "MAC5_RGMII_RXCLK_DELAY" },
	{ 11,  1, "MAC5_PHY_MII_PIPE_RXCLK edge" },
	{ 10,  1, "MAC5_PHY_MII_EN" },
	{  9,  1, "MAC5_PHY_MII_TXCLK inv." },
	{  8,  1, "MAC5_PHY_MII_RXCLK inv." },
	{  2,  1, "MAC5_MAC_MII_EN" },
	{  1,  1, "MAC5_MAC_MII_TXCLK inv." },
	{  0,  1, "MAC5_MAC_MII_TXCLK inv." },
	{ -1, -1, NULL }
}, g_8337_reg_c[] = {
	{ 26,  1, "MAC6_RGMII_EN" },
	{ 25,  1, "MAC6_RGMII_TXCLK_DELAY_EN" },
	{ 22,  2, "MAC6_RGMII_TXCLK_DELAY" },
	{ 20,  2, "MAC6_RGMII_RXCLK_DELAY" },
	{ 17,  1, "PHY4_RGMII_EN edge" },
	{ 11,  1, "MAC6_PHY_MII_PIPE_RXCLK edge" },
	{ 10,  1, "MAC6_PHY_MII_EN" },
	{  9,  1, "MAC6_PHY_MII_TXCLK inv." },
	{  8,  1, "MAC6_PHY_MII_RXCLK inv." },
	{  7,  1, "MAC6_SGMII_EN" },
	{  2,  1, "MAC6_MAC_MII_EN" },
	{  1,  1, "MAC6_MAC_MII_TXCLK inv." },
	{  0,  1, "MAC6_MAC_MII_TXCLK inv." },
	{ -1, -1, NULL }
}, g_8337_reg_10[] = {
	{  31, 1, "Power-ON strap" },
	{  28, 1, "PACKAGEMIN_EN" },
	{  27, 1, "INPUT_MODE" },
	{  25, 1, "SPI_EEPROM_EN" },
	{  24, 1, "LED pad mode" },
	{  17, 1, "Hibernate" },
	{   7, 1, "SerDes autoneg EN" },
	{ -1, -1, NULL }
}, g_8337_reg_28[] = {
	{ 29,  1, "ACL" },
	{ 28,  1, "LOOKUP" },
	{ 27,  1, "QM" },
	{ 26,  1, "MIB" },
	{ 25,  1, "OFFLOAD" },
	{ 24,  1, "HARDWARE INT DONE EN" },
	{ 23,  1, "ACL MATCH" },
	{ 22,  1, "ARL DONE" },
	{ 21,  1, "ARL CPU FULL" },
	{ 20,  1, "VT DONE" },
	{ 19,  1, "MIB DONE" },
	{ 18,  1, "ACL DONE" },
	{ 17,  1, "OFFLOAD DONE" },
	{ 16,  1, "OFFLOAD CPU FULL DONE" },
	{ 11,  1, "ARL LEARN CREATE" },
	{ 10,  1, "ARL LEARN CHANGE" },
	{  9,  1, "ARL DELETE" },
	{  8,  1, "ARL LEARN FULL" },
	{  5,  1, "ARP LEARN CREATE" },
	{  4,  1, "ARP LEARN CHANGE" },
	{  3,  1, "ARP AGE DELETE" },
	{  2,  1, "ARP LEARN FULL" },
	{  1,  1, "VT MISS VIO" },
	{  0,  1, "VT MEM VIO" },
	{ -1, -1, NULL }
}, g_8337_reg_2c[] = {
	{ 19,  1, "THERM" },
	{ 18,  1, "EEPROM ERR" },
	{ 17,  1, "EEPROM" },
	{ 16,  1, "MDIO DONE" },
	{ 15,  1, "PHY" },
	{ 14,  1, "QM ERR" },
	{ 13,  1, "LOOKUP ERR" },
	{ 12,  1, "LOOP CHECK" },
	{  7,  7, "LINK_CHG" },
	{  0,  1, "BIST_DONE" },
	{ -1, -1, NULL }
}, g_8337_reg_30[] = {
	{ 10,  1, "SPECIAL DIP EN" },
	{  1,  1, "ACL_EN" },
	{  0,  1, "MIB_EN" },
	{ -1, -1, NULL }
}, g_8337_reg_38[] = {
	{ 24,  4, "RELOAD_TIMER" },
	{ 19,  1, "SGMII_CLK125M_RX inv." },
	{ 18,  1, "SGMII_CLK125M_TX inv." },
	{ -1, -1, NULL }
}, g_8337_reg_3c[] = {
	{ 26,  1, "MDIO_SUP_PRE" },
	{ 16,  5, "REG_ADDR" },
	{  0, 16, "MDIO_DATA" },
	{ -1, -1, NULL }
}, g_8337_reg_40[] = {
	{ 31,  1, "BIST_BUSY" },
	{ 29,  1, "BIST_PASS" },
	{ 23,  1, "BIST_CRITICAL" },
	{ 22,  1, "BIST_PTN_EN_2" },
	{ 21,  1, "BIST_PTN_EN_1" },
	{ 20,  1, "BIST_PTN_EN_0" },
	{ -1, -1, NULL }
}, g_8337_reg_7c[] = {
	{ 12,  1, "FLOW_LINK_EN" },
	{ 11,  1, "AUTO RX FLOW EN" },
	{ 10,  1, "AUTO TX FLOW EN" },
	{  9,  1, "LINK_EN" },
	{  8,  1, "LINK" },
	{  7,  1, "TX_HALF_FLOW_EN" },
	{  6,  1, "DUPLEX" },
	{  5,  1, "RX_FLOW_EN" },
	{  4,  1, "TX_FLOW_EN" },
	{  3,  1, "RXMAC_EN" },
	{  2,  1, "TXMAC_EN" },
	{  0,  2, "SPEED" },
	{ -1, -1, NULL }
}, g_8337_reg_100[] = {
	{ 12,  1, "LPI_EN_5" },
	{ 10,  1, "LPI_EN_4" },
	{  8,  1, "LPI_EN_3" },
	{  6,  1, "LPI_EN_2" },
	{  4,  1, "LPI_EN_1" },
	{  3,  1, "EEE_CPU_CHANGE_EN" },
	{  2,  1, "EEE_LLDP_TO_CPU_EN" },
	{  1,  1, "EEE_EN" },
	{ -1, -1, NULL }
}, g_8337_reg_400[] = {
	{ 31,  1, "ACL_BUSY" },
	{  8,  2, "ACL_RULE_SEL" },
	{  0,  7, "ACL rule index" },
	{ -1, -1, NULL }
}, g_8337_reg_420[] = {
	{ 29,  1, "CVLAN priority" },
	{ 16, 12, "Default CVID" },
	{ 13,  3, "SVLAN priority" },
	{  0, 12, "Default SVID" },
	{ -1, -1, NULL }
}, g_8337_reg_424[] = {
	{ 14,  1, "EG_VLAN_TYPE" },
	{ 12,  2, "EG_VLAN_MODE" },
	{ 10,  1, "L3 src port check" },
	{  9,  1, "CORE_PORT_EN" },
	{  8,  1, "FORCE_DEF_VID_EN" },
	{  7,  1, "PORT_TLS_MODE" },
	{  6,  1, "PORT_VLAN_PROP_EN" },
	{  5,  1, "PORT_CLONE_EN" },
	{  4,  1, "VLAN_PRI_PRO_EN" },
	{  2,  2, "ING_VLAN_MODE" },
	{ -1, -1, NULL }
}, g_8337_reg_660[] = {
	{ 31,  1, "MCAST_DROP_EN" },
	{ 28,  1, "UNI_LEAKY_EN" },
	{ 27,  1, "MULTI_LEAKY_EN" },
	{ 26,  1, "ARP_LEAKY_EN" },
	{ 25,  1, "NG_MIRROR_EN" },
	{ 21,  1, "PORT_LOOPBACK_EN" },
	{ 20,  1, "LEARN_EN" },
	{ 16,  3, "PORT_STATE" },
	{ 10,  1, "FORCE_PORT_VLAN_EN" },
	{  8,  2, "VLAN_MODE" },
	{  0,  7, "PORT_VID_MEM" },
	{ -1, -1, NULL }
};

static const struct qca8337_regs_def_s {
	int offset;	/* aligned to 4-bytes */
	char *name;
	const struct reg_bit_defs_s *bdef;
} g_qca8337_regs_def[] = {
	/* 0.Global control registers */
	{    4, "PORT0_PAD_CTRL", g_8337_reg_4 },
	{    8, "PORT5_PAD_CTRL", g_8337_reg_8 },
	{  0xC, "PORT6_PAD_CTLR", g_8337_reg_c },
	{ 0x10, "PWS_REG", g_8337_reg_10 },
	{ 0x28, "GLOBAL_INT0_MASK", g_8337_reg_28 },
	{ 0x2C, "GLOBAL_INT1_MASK", g_8337_reg_2c },
	{ 0x30, "MODULE_EN", g_8337_reg_30 },
	{ 0x38, "INTERFACE_HIGH_ADDR", g_8337_reg_38 },
	{ 0x3C, "MDIO master control", g_8337_reg_3c },
	{ 0x40, "BIST_CTRL", g_8337_reg_40 },
	{ 0x7C, "PORT0_STATUS", g_8337_reg_7c },
	{ 0x80, "PORT1_STATUS", g_8337_reg_7c },	/* same as PORT0_STATUS */
	{ 0x84, "PORT2_STATUS", g_8337_reg_7c },	/* same as PORT0_STATUS */
	{ 0x88, "PORT3_STATUS", g_8337_reg_7c },	/* same as PORT0_STATUS */
	{ 0x8C, "PORT4_STATUS", g_8337_reg_7c },	/* same as PORT0_STATUS */
	{ 0x90, "PORT5_STATUS", g_8337_reg_7c },	/* same as PORT0_STATUS */
	{ 0x94, "PORT6_STATUS", g_8337_reg_7c },	/* same as PORT0_STATUS */

	/* 1.EEE control registers */
	{ 0x100, "EEE_CTRL", g_8337_reg_100 },

	/* 2.Parser control registers */

	/* 3.ACL control registers */
	{ 0x400, "ACL_FUNC0", g_8337_reg_400 },
	{ 0x420, "PORT0_VLAN_CTRL0", g_8337_reg_420 },
	{ 0x424, "PORT0_VLAN_CTRL1", g_8337_reg_424 },
	{ 0x428, "PORT1_VLAN_CTRL0", g_8337_reg_420 },	/* same as PORT0_VLAN_CTRL0 */
	{ 0x42C, "PORT1_VLAN_CTRL1", g_8337_reg_424 },	/* same as PORT0_VLAN_CTRL1 */
	{ 0x430, "PORT2_VLAN_CTRL0", g_8337_reg_420 },	/* same as PORT0_VLAN_CTRL0 */
	{ 0x434, "PORT2_VLAN_CTRL1", g_8337_reg_424 },	/* same as PORT0_VLAN_CTRL1 */
	{ 0x438, "PORT3_VLAN_CTRL0", g_8337_reg_420 },	/* same as PORT0_VLAN_CTRL0 */
	{ 0x43C, "PORT3_VLAN_CTRL1", g_8337_reg_424 },	/* same as PORT0_VLAN_CTRL1 */
	{ 0x440, "PORT4_VLAN_CTRL0", g_8337_reg_420 },	/* same as PORT0_VLAN_CTRL0 */
	{ 0x444, "PORT4_VLAN_CTRL1", g_8337_reg_424 },	/* same as PORT0_VLAN_CTRL1 */
	{ 0x448, "PORT5_VLAN_CTRL0", g_8337_reg_420 },	/* same as PORT0_VLAN_CTRL0 */
	{ 0x44C, "PORT5_VLAN_CTRL1", g_8337_reg_424 },	/* same as PORT0_VLAN_CTRL1 */
	{ 0x450, "PORT6_VLAN_CTRL0", g_8337_reg_420 },	/* same as PORT0_VLAN_CTRL0 */
	{ 0x454, "PORT7_VLAN_CTRL1", g_8337_reg_424 },	/* same as PORT0_VLAN_CTRL1 */

	/* 4.Lookup control registers */
	{ 0x660, "PORT0_LOOKUP_CTRL", g_8337_reg_660 },
	{ 0x66C, "PORT1_LOOKUP_CTRL", g_8337_reg_660 },	/* same as PORT0_LOOKUP_CTRL */
	{ 0x678, "PORT2_LOOKUP_CTRL", g_8337_reg_660 },	/* same as PORT0_LOOKUP_CTRL */
	{ 0x684, "PORT3_LOOKUP_CTRL", g_8337_reg_660 },	/* same as PORT0_LOOKUP_CTRL */
	{ 0x690, "PORT4_LOOKUP_CTRL", g_8337_reg_660 },	/* same as PORT0_LOOKUP_CTRL */
	{ 0x69C, "PORT5_LOOKUP_CTRL", g_8337_reg_660 },	/* same as PORT0_LOOKUP_CTRL */
	{ 0x6A8, "PORT6_LOOKUP_CTRL", g_8337_reg_660 },	/* same as PORT0_LOOKUP_CTRL */

	/* 5.QM control registers */

	/* 6.PKT edit control registers */

	{ -1, NULL, NULL }
};

/* Dump important QCA8337 switch registers. */
static int dump_qca8337_regs(FILE *fp)
{
	int i, j, rlen, r, found, s_offset;
	FILE *fp_res;
	unsigned int tmp[8], reg[240], *p, m, v;
	char line[128], cmd[sizeof("ssdk_sh " SWID_QCA8337 " debug reg dump 0XXX")];
	const struct reg_bit_defs_s *b;
	const struct qca8337_regs_def_s *q;

	if (!fp)
		return -1;

	fprintf(fp, "\n\n######## Dump QCA8337 switch registers ########\n");
	/* Example: ssdk_sh sw1 debug reg dump 0
	 *
	 *  SSDK Init OK!
	 * [Register dump]
	 * 0.Global control registers.
	 *              0        4        8        c       10       14       18       1c
	 *  [0000] 00001302 05600000 01000000 00000080 00261320 f0107650 00004d86 00003f1f
	 *  [0020] 3f000800 00010000 00000000 00008000 80000401 00000000 0f000000 00000000
	 *  [0040] 00700000 00000000 000088a8 00000000 cc35cc35 0000007e c935c935 03ffff00
	 *  [0060] 00000001 00000000 00000000 00000000 b00ee060 03707f07 000005ee 0000004e
	 *  [0080] 000010c2 000010c2 000010c2 00001ffe 000010c2 00000200 00000000 00000000
	 *  [00a0] 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	 *  [00c0] 00000000 00000000 80901040 00000000 fffbff7e 00000001 00000100 000303ff
	 *  [00e0] c78164de 000aa545
	 *
	 *
	 *
	 * operation done.
	 */
	for (i = 0; i <= 6; ++i) {
		snprintf(cmd, sizeof(cmd), "ssdk_sh %s debug reg dump %d", SWID_QCA8337, i);
		if (!(fp_res = popen(cmd, "r"))) {
			fprintf(fp, "cmd [%s] failed. errno %d (%s)\n", cmd, errno, strerror(errno));
			continue;
		}
		p = &reg[0];
		rlen = 0;
		found = s_offset = 0;
		while (fgets(line, sizeof(line), fp_res) != NULL) {
			fprintf(fp, "%s", line);
			if (strncmp(line, " [0", 3))
				continue;

			if (!found) {
				if (sscanf(line + 2, "%x", &s_offset) != 1) {
					fprintf(fp, "Looking up start offset failed\n");
				}
				found = 1;
			}

			r = sscanf(line + 8, "%x %x %x %x %x %x %x %x", tmp, tmp + 1,
				tmp + 2, tmp + 3, tmp + 4, tmp + 5, tmp + 6, tmp + 7);
			if (r > 0) {
				if ((rlen + r) * 4 > sizeof(reg)) {
					fprintf(fp, "Size of reg (%d) is not enough (%d)!\n", sizeof(reg), (rlen + r) * 4);
					rlen = sizeof(reg) - rlen;
				}
				memcpy(p, tmp, r * 4);
				p += r;
				rlen += r;
			}

			if (r < 8)
				break;
		}
		fprintf(fp, "\n");

		/* Parse interesting registers. */
		for (q = &g_qca8337_regs_def[0]; q->offset >= 0 && q->name && q->bdef; ++q) {
			if (q->offset < s_offset || q->offset >= (s_offset + rlen * 4))
				continue;

			j = (q->offset - s_offset) >> 2;
			if (!*(reg + j))
				continue;
			fprintf(fp, "%20s(%04x): %08x", q->name, q->offset, *(reg + j));
			for (b = q->bdef; b->sbit >= 0 && b->blen > 0 && b->description; ++b) {
				if (b->sbit >= 32 || b->blen >= 32)
					break;
				m = (1U << b->blen) - 1;
				v = (*(reg + j) >> b->sbit) & m;
				if (!v)
					continue;
				fprintf(fp, ", %s %x", b->description, v);
			}
			fprintf(fp, "\n");
		}

		pclose(fp_res);
		fprintf(fp, "\n");
	}

	return 0;
}

static const struct reg_bit_defs_s g_phy_reg_0[] = {
	{ 15,  1, "RESET" },
	{ 14,  1, "LOOPBACK" },
	{ 13,  1, "SPEED SEL." },
	{ 12,  1, "AUTONEG" },
	{ 11,  1, "POWER DOWN" },
	{  9,  1, "RESTART AUTONEG" },
	{  8,  1, "DUPLEX MODE" },
	{  6,  1, "SPEED SEL (MSB)" },
	{ -1, -1, NULL }
}, g_phy_reg_1[] = {
	{ 14,  1, "100BASE-X full" },
	{ 13,  1, "100BASE-X half" },
	{ 12,  1, "10M FDUPX" },
	{ 11,  1, "10M HDUPX" },
	{  5,  1, "AUTONEG COMPLETE" },
	{  3,  1, "AUTONEG ABILITY" },
	{  2,  1, "LINK STATUS" },
	{  0,  1, "EXTENDED CAP.." },
	{ -1, -1, NULL }
}, g_phy_reg_5[] = {
	{  8,  1, "100BASE-TX FDUP" },
	{  7,  1, "100BASE-TX HDUP" },
	{ -1, -1, NULL }
}, g_phy_reg_8[] = {
	{  9,  1, "1000BASE-T FDUP" },
	{  8,  1, "1000BASE-T HDUP" },
}, g_phy_reg_9[] = {
	{ 11,  1, "L.PARTNER 1G FDUP CAP" },
	{ 10,  1, "L.PARTNER 1G HDUP CAP" },
	{ -1, -1, NULL }
}, g_phy_reg_a[] = {
	{ 15,  1, "Master/Slave conf. fault" },
	{ 14,  1, "Master/Slave conf. resolution" },
	{ 13,  1, "Local receiver status" },
	{ 12,  1, "Remote receiver status" },
	{ 11,  1, "L.PARTNER 1G FDUP CAP." },
	{ 10,  1, "L.PARTNER 1G HDUP CAP." },
	{ -1, -1, NULL }
}, g_phy_reg_f[] = {
	{ 13,  1, "1000BASE-T FDUP" },
	{ 12,  1, "1000BASE-T HDUP" },
};

static const struct phy_regs_def_s {
	int addr;	/* 0 ~ 0x1F */
	char *name;
	const struct reg_bit_defs_s *bdef;
} g_phy_regs_def[] = {
	{    0, "Control", g_phy_reg_0 },
	{    1, "Status", g_phy_reg_1 },
	{    5, "Link partner ability", g_phy_reg_5 },
	{    8, "Link partner next page", g_phy_reg_8 },
	{    9, "1000BASE-T control", g_phy_reg_9 },
	{  0xA, "1000BASE-T status", g_phy_reg_a },
	{  0xF, "Extended status", g_phy_reg_f },

	{ -1, NULL, NULL }
};

/* Dump PHY registers */
static int dump_phy_regs(FILE *fp)
{
	int phy, i, v;
	unsigned short reg[64], m;
	const struct vports_s {
		int vport;
		char *name;
	} vports_tbl[] = {
		{ WAN_PORT, "WAN port" },
		{ LAN1_PORT, "LAN1 port" },
		{ LAN2_PORT, "LAN2 port" },
		{ LAN3_PORT, "LAN3 port" },
		{ LAN4_PORT, "LAN4 port" },
		{ LAN5_PORT, "LAN5 port" },
		{ LAN6_PORT, "LAN6 port" },
		{ LAN7_PORT, "LAN7 port" },
		{ LAN8_PORT, "LAN8 port" },

		{ -1, NULL }
	}, *p;
	const struct reg_bit_defs_s *b;
	const struct phy_regs_def_s *q;

	if (!fp)
		return -1;

	/* enumerate W1, L1 ~ L8 */
	fprintf(fp, "\n\n######## Dump PHY/DBG registers of each port ########\n");
	get_qca8337_port_definition();
	for (p = &vports_tbl[0]; p->vport >= 0 && p->name; ++p) {
		phy = *(vport_to_phy_addr + p->vport);
		if (phy < 0 || phy > 0x20)
			continue;

		/* PHY registers, 0 ~ 0x1F */
		fprintf(fp, "\nDump %s (PHY %d)'s phy registers:", p->name, phy);
		memset(reg, 0, sizeof(reg));
		for (i = 0; i <= 0x1F; ++i) {
			if ((v = read_phy_reg(phy, i)) < 0) {
				fprintf(fp, "Failed to read register 0x%x from phy %d\n", i, phy);
				continue;
			}
			if (!(i % 16))
				fprintf(fp, "\n%4x:", i);
			fprintf(fp, " %04x", v);
			reg[i] = v;
		}
		fprintf(fp, "\n\n");

		/* Parse interesting phy registers. */
		for (q = &g_phy_regs_def[0]; q->addr >= 0 && q->name && q->bdef; ++q) {
			if (!*(reg + q->addr))
				continue;
			fprintf(fp, "%20s(%02x): %04x", q->name, q->addr, *(reg + q->addr));
			for (b = q->bdef; b->sbit >= 0 && b->blen > 0 && b->description; ++b) {
				if (b->sbit >= 16 || b->blen >= 16)
					break;
				m = (1U << b->blen) - 1;
				v = (*(reg + q->addr) >> b->sbit) & m;
				if (!v)
					continue;
				fprintf(fp, ", %s %x", b->description, v);
			}
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");

		/* dbg registers, 0 ~ 0x3F */
		fprintf(fp, "\nDump %s (PHY %d)'s debug registers:", p->name, phy);
		memset(reg, 0, sizeof(reg));
		for (i = 0; i <= 0x3F; ++i) {
			if (!write_phy_reg(phy, 0x1d, i)) {
				fprintf(fp, "Failed to write 0x%x to reg 0x1d of PHY %d\n", i, phy);
				continue;
			}
			if ((v = read_phy_reg(phy, 0x1e)) < 0) {
				fprintf(fp, "Failed to read register 0x1d from phy %d\n", phy);
				continue;
			}
			if (!(i % 16))
				fprintf(fp, "\n%4x:", i);
			fprintf(fp, " %04x", v);
			reg[i] = v;
		}
	}

	return 0;
}

static const char *g_qca_nss_drv_stats_attrs[] = {
	"pppoe", "gre", "pptp", "l2tpv2", "n2h", "ipv4", "ipv6",
	"gmac", "drv", "wifi", "wifili", "eth_rx", "lso_rx",
	NULL
};

static int dump_qca_nss_drv_stats(FILE *fp)
{
	const char **p;
	FILE *fp_res;
	char line[512], path[sizeof("/sys/kernel/debug/qca-nss-drv/stats/XXX") + 16];

	if (!fp)
		return -1;

	fprintf(fp, "\n");
	for (p = &g_qca_nss_drv_stats_attrs[0]; *p != NULL; ++p) {
		snprintf(path, sizeof(path), "/sys/kernel/debug/qca-nss-drv/stats/%s", *p);
		if (!f_exists(path)) {
			fprintf(fp, "%s doesn't exist, skip\n", path);
			continue;
		}

		if (!(fp_res = fopen(path, "r"))) {
			fprintf(fp, "Open [%s] failed. errno %d (%s)\n", path, errno, strerror(errno));
			return -2;
		}
		fprintf(fp, "\n\n######## Dump %s ########", path);
		while (fgets(line, sizeof(line), fp_res) != NULL) {
			fprintf(fp, "%s", line);
		}
		fclose(fp_res);
	}

	return 0;
}

/* Dump @fn content to @fp.
 * @fp:	FILE pointer
 * @fn: filename
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int dump_file(FILE *fp, const char *fn)
{
	FILE *fp_res;
	char line[512];

	if (!fp || !fn)
		return -1;

	if (!(fp_res = fopen(fn, "r")))
		return -2;

	while (fgets(line, sizeof(line), fp_res) != NULL) {
		fprintf(fp, "%s", line);
	}
	fprintf(fp, "\n");
	fclose(fp_res);

	return 0;
}

/* Exec @cmd and dump output to @fp
 * @fp:	FILE pointer
 * @cmd: command
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int exec_and_dump(FILE *fp, const char *cmd)
{
	FILE *fp_res;
	char line[512];

	if (!fp || !cmd || *cmd == '\0')
		return -1;

	if (!(fp_res = popen(cmd, "r")))
		return -2;

	while (fgets(line, sizeof(line), fp_res) != NULL) {
		fprintf(fp, "%s", line);
	}
	fprintf(fp, "\n");
	pclose(fp_res);

	return 0;
}

struct sw_port_name_s {
	const char *sw;
	int port;
	const char *name;
};

static struct sw_port_name_s sw_port_name[] = {
	{ SWID_IPQ807X, 4, "WAN" },
	{ SWID_IPQ807X, 6, "10G base-T" },
	{ SWID_IPQ807X, 5, "10G SFP+" },
	{ SWID_IPQ807X, 3, "LAN1" },
	{ SWID_IPQ807X, 2, "LAN2" },
	{ SWID_QCA8337, 4, "LAN3" },
	{ SWID_QCA8337, 3, "LAN4" },
	{ SWID_QCA8337, 2, "LAN5" },
	{ SWID_QCA8337, 1, "LAN6" },
	{ SWID_QCA8337, 0, "LAN7" },
	{ SWID_QCA8337, 6, "LAN8" },

	{ NULL, -1, NULL }
};

void __gen_switch_log(char *fn)
{
#if defined(RTCONFIG_SOC_IPQ8064)
	const char *v4_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv4/stop", *v6_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv6/stop";
#elif defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ60XX) || defined(RTCONFIG_SOC_IPQ50XX)
	const char *v4_stop_fn = "/sys/kernel/debug/ecm/front_end_ipv4_stop", *v6_stop_fn = "/sys/kernel/debug/ecm/front_end_ipv6_stop";
#endif
	int r, ipg = 0;
	char *ipg_str, read_ipg_cmd[] = "ssdk_sh " SWID_IPQ807X " debug reg get 0x7000 4";
	char fc_state[sizeof("DISABLEXXXXXX")], fc_cmd[sizeof("ssdk_sh sw0 port flowctrl get XYYY")];
	char path[64], *ate_cmd[] = { "ATE", "Get_WanLanStatus", NULL };
	FILE *fp;
	struct sw_port_name_s *swp;
	struct xgmac_def_s {
		char *name;
		int xgmac_port;
		char *nv;		/* nvram that is used to specify acceleration type. */
	} xgmac_def_tbl[] = {
		{ "10G base-T", 6, "aqr_hwnat_type" },
		{ "10G SFP+", 5, "sfpp_hwnat_type" },

		{ NULL, -1, NULL }
	}, *pxgmac;

	if (!fn || *fn == '\0')
		return;

	snprintf(path, sizeof(path), ">%s", fn);
	_eval(ate_cmd, path, 0, NULL);

	if (!(fp = fopen(fn, "a")))
		return;

	fprintf(fp, "\n\n######## IPG of 10G base-T port ########\n");
	if (!(r = parse_ssdk_sh(read_ipg_cmd, "%*[^:]:%x", 1, &ipg))) {
		if ((ipg & 0xF00) == 0x900)
			ipg_str = "128";
		else if ((ipg & 0xF00) == 0)
			ipg_str = "96";
		else
			ipg_str = "unknown";
		fprintf(fp, "Reg 0x7000 = 0x%08x, IPG %s bit times\n", ipg, ipg_str);
	} else {
		fprintf(fp, "Read IPG failed, cmd [%s], return %d\n", read_ipg_cmd, r);
	}

	/* ssdk_sh sw0 port flowctrl get 6
	 *
	 *   SSDK Init OK![Flow control]:ENABLE
	 *  operation done.
	 */
	fprintf(fp, "\n\n######## Flow control ########\n");
	for (swp = &sw_port_name[0]; swp->sw != NULL; ++swp) {
		snprintf(fc_cmd, sizeof(fc_cmd), "ssdk_sh %s port flowctrl get %d", swp->sw, swp->port);
		strlcpy(fc_state, "READ FAIL", sizeof(fc_state));
		parse_ssdk_sh(fc_cmd, "%*[^:]:%13s", 1, fc_state);
		fprintf(fp, "%10s: %7s (%s port %d)\n", swp->name, fc_state, swp->sw, swp->port);
	}

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	/* Check run-time status of ecm. */
	fprintf(fp, "\n\n######## ecm stopped? ########\n");
	fprintf(fp, "IPv4: ");
	dump_file(fp, v4_stop_fn);
	fprintf(fp, "IPv6: ");
	dump_file(fp, v6_stop_fn);
#endif

	/* Check hardware NAT acceleration type of XGMAC
	 *          MAC learning	Packet action
	 * PPE+NSS: ENABLE		FORWARD
	 * NSS:     DISABLE		RDTCPU
	 */
	fprintf(fp, "\n\n######## acceleration type of XGMAC ports? ########\n");
	fprintf(fp, "PPE + NSS: ENABLE, FORWARD\n");
	fprintf(fp, "NSS      : DISABLE, RDTCPU\n");
	fprintf(fp, "wans_dualwan: [%s]\n", nvram_get("wans_dualwan")? : "NULL");
	for (pxgmac = &xgmac_def_tbl[0]; pxgmac->name != NULL; ++pxgmac) {
		char mac_learn_result[sizeof("DISABLEXXX")], pkt_act_result[sizeof("FORWARDXXX")];
		char get_cmd[sizeof("ssdk_sh " SWID_IPQ807X " fdb ptLearnCtrl get XXX")];

		strlcpy(mac_learn_result, "READ FAIL", sizeof(mac_learn_result));
		strlcpy(pkt_act_result, "READ FAIL", sizeof(pkt_act_result));
		snprintf(get_cmd, sizeof(get_cmd), "ssdk_sh " SWID_IPQ807X " fdb ptLearnCtrl get %d", pxgmac->xgmac_port);
		parse_ssdk_sh(get_cmd, "%*[^:]:%[^[]%*[^:]:%s", 2, mac_learn_result, pkt_act_result);
		fprintf(fp, "%10s: MAC learning: [%s], Packet action: [%s], %s=%s\n",
			pxgmac->name, mac_learn_result, pkt_act_result, pxgmac->nv, nvram_get(pxgmac->nv)? : "NULL");
	}

	/* Check ebtables* kernel modules is loaded or not. */
	fprintf(fp, "\n\n######## ebtables status ########\n");
	exec_and_dump(fp, "lsmod|grep ebtable");

	/* EEPROM in SFP+ module */
	dump_sfpp_eeprom(fp);

	/* Important AQR PHY registers */
	dump_aqr_regs(fp);

	/* QCA8337 registers */
	dump_qca8337_regs(fp);

	/* Registers of each PHY */
	dump_phy_regs(fp);

	/* qca-nss-drv stats */
	dump_qca_nss_drv_stats(fp);

	fclose(fp);
}
#endif
