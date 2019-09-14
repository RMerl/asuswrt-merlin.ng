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
#include <linux/mii.h>
#include <dirent.h>

#include <shutils.h>
#include <shared.h>
#include <utils.h>
#include <qca.h>

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

/* array index:		virtual port mapping enumeration.
 * 			e.g. LAN1_PORT, LAN2_PORT, etc.
 * array element:	PHY address, negative value means absent PHY.
 * 			QCA8337 port X uses PHY X-1
 * 			0x10? means Aquentia AQR107,
 * 			      check it with is_aqr_phy_exist before using it on GT-AXY16000/GX-AC5400.
 * 			0x20? means ethX, ethtool ioctl
 */
static const int vport_to_phy_addr[MAX_WANLAN_PORT] = {
	10, 9, 4, 3, 2, 1, 0, 6, 11, 0x107, 0x204
};

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
static const int vport_to_8337port[MAX_WANLAN_PORT] = {
	-1, -1, 5, 4, 3, 2, 1, 6,				/* LAN1~8, LAN8 is connected to P6 of QCA8337 indirectly. */
	-1, -1, -1						/* WAN1, WAN2 10G RJ-45, 10G SFP+ */
};

static const unsigned int qca8337_vportmask =
	(1U << LAN3_PORT) | (1U << LAN4_PORT) | (1U << LAN5_PORT) |
	(1U << LAN6_PORT) | (1U << LAN7_PORT) | (1U << LAN8_PORT);

/* array index:		switch_stb_x nvram variable.
 * array element:	platform specific VoIP/STB port bitmask.
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

/* ALL WAN/LAN port bit-mask */
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

#define IPQ807X_MDIO_SYS_DIR	"/sys/devices/platform/soc/90000.mdio/mdio_bus/90000.mdio"
#define AQR_PHY_SYS_DIR		"/sys/devices/platform/soc/90000.mdio/mdio_bus/90000.mdio/90000.mdio:07"
int is_aqr_phy_exist(void)
{
	static int aqr_phy_absent = -1;

	if (aqr_phy_absent >= 0)
		return !aqr_phy_absent;

	if (!d_exists(IPQ807X_MDIO_SYS_DIR)) {
		dbg("%s hasn't been created, assume AQR PHY exist!\n", IPQ807X_MDIO_SYS_DIR);
		return 1;
	}

	if (d_exists(AQR_PHY_SYS_DIR)) {
		aqr_phy_absent = 0;
	} else {
		aqr_phy_absent = 1;
	}

	return !aqr_phy_absent;
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

	for (vport = 0, m = vportmask;
	     vport < ARRAY_SIZE(vport_to_8337port) && m != 0;
	     m >>= 1, vport++)
	{
		if (!(m & 1) || vport_to_8337port[vport] < 0)
			continue;

		rportmask |= (1U << vport_to_8337port[vport]);
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
	unsigned int m = mbr, u = untag;
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

	upstream_vport = iface_name_to_vport(upstream_if);
	if (upstream_vport < 0 || upstream_vport >= MAX_WANLAN_PORT) {
		dbg("%s: Can't find vport for upstream iface [%s]\n", __func__, upstream_if);
		return -1;
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
		mbr |= (1U << upstream_vport);
	}
	if (untag & (1U << WAN_PORT)) {
		untag &= ~(1U << WAN_PORT);
		untag |= (1U << upstream_vport);
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

		for (vport = 0, m = mbr & ~((1U << upstream_vport)), u = untag;
		     vport < MAX_WANLAN_PORT && m > 0;
		     vport++, m >>= 1, u >>= 1)
		{
			if (!(m & 1))
				continue;

			if ((port = vport_to_8337port[vport]) < 0) {
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
 * @return
 * 	0:	success
 *     -1:	invalid parameter.
 *     -2:	miireg_read() MII_BMSR failed
 */
static int get_phy_info(unsigned int phy, unsigned int *link, unsigned int *speed)
{
	int r = 0, l = 1, s = 0;
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
		l = 0;
		break;
	}

	if (link)
		*link = l;
	if (speed) {
		*speed = s;
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
static int get_qca8075_8337_8035_8033_aqr107_vport_info(unsigned int vport, unsigned int *link, unsigned int *speed)
{
	int phy;

	if (vport >= ARRAY_SIZE(vport_to_phy_addr) || (!link && !speed))
		return -1;

	if (link)
		*link = 0;
	if (speed)
		*speed = 0;

	if (vport == WAN10GR_PORT && !is_aqr_phy_exist())
		return 0;

	phy = vport_to_phy_addr[vport];
	if (phy < 0) {
		dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
		return -1;
	}

	get_phy_info(phy, link, speed);

	return 0;
}

/**
 * Get linkstatus in accordance with port bit-mask.
 * @mask:	port bit-mask.
 * 		bit0 = P0, bit1 = P1, etc.
 * @linkStatus:	link status of all ports that is defined by mask.
 * 		If one port of mask is linked-up, linkStatus is true.
 */
static void get_qca8075_8337_8035_8033_aqr107_phy_linkStatus(unsigned int mask, unsigned int *linkStatus)
{
	int i;
	unsigned int value = 0, m;

	m = mask & wanlanports_mask;
	for (i = 0; m > 0 && !value; ++i, m >>= 1) {
		if (!(m & 1))
			continue;

		get_qca8075_8337_8035_8033_aqr107_vport_info(i, &value, NULL);
		value &= 0x1;
	}
	*linkStatus = value;
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

	if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 4)) {
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
		wans_lan_mask = 1U << lan_id_to_vport_nr(wans_lanport);
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

		get_qca8075_8337_8035_8033_aqr107_vport_info(i, NULL, (unsigned int*) &t);
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
	default:
		_dprintf("%s: invalid speed!\n", __func__);
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
		if (vport >= ARRAY_SIZE(vport_to_phy_addr)) {
			dbg("%s: PHY address is not defined for vport %d\n", __func__, vport);
			continue;
		}

		if (vport == WAN10GR_PORT && !is_aqr_phy_exist())
			return;
		phy = vport_to_phy_addr[vport];
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
			/* Aquentia PHY */
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
rtkswitch_wanPort_phyStatus(int wan_unit)
{
	unsigned int status = 0;

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

void ATE_port_status(void)
{
	int i;
	char buf[6 * 11], wbuf[6 * 3], lbuf[6 * 8];
	phyState pS;
#if defined(RTAC89U)
	const int wan1g_sfp10g = 1;	/* 10G base-T absent. */
#else
	const int wan1g_sfp10g = 0;
#endif

	memset(&pS, 0, sizeof(pS));
	for (i = 0; i < NR_WANLAN_PORT; i++) {
		get_qca8075_8337_8035_8033_aqr107_vport_info(lan_id_to_vport_nr(i), &pS.link[i], &pS.speed[i]);
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

	strlcpy(buf, wbuf, sizeof(buf));
	strlcat(buf, lbuf, sizeof(buf));

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

	/* If AQR PHY doesn't exist, remove related settings. */
	if (!is_aqr_phy_exist()) {
		upstream_iptv_ifaces[WANS_DUALWAN_IF_WAN2] = NULL;
		vport_to_iface[WAN10GR_PORT] = NULL;
		wanlanports_mask &= ~(1U << WAN10GR_PORT);
		logmessage("system", "AQR PHY firmware: N/A");

	} else {
		/* 10G link status is shown on LED2 (GREEN, closed to WAN) of AQR107.
		 * No need to show 10G link statis on LED1 (YELLOW, closed to WAN) too.
		 */
		if ((r1 = read_phy_reg(7, 0x401EC431)) < 0)
			r1 = 0;
		write_phy_reg(7, 0x401EC431, r1 & ~(1U << 7));

		/* Print AQR firmware version. */
		for (i = 0, paddr = &vport_to_phy_addr[0]; i < ARRAY_SIZE(vport_to_phy_addr); ++i, ++paddr) {
			int fw, build;

			/* only accept AQR phy */
			if (*paddr < 0x100 || *paddr >= 0x120)
				continue;

			fw = read_phy_reg(*paddr & 0xFF, 0x401e0020);
			build = read_phy_reg(*paddr & 0xFF, 0x401ec885);
			if (fw < 0  || build < 0) {
				dbg("Can't get AQR PHY firmware version.\n");
			} else {
				dbg("AQR PHY firmware %d.%d build %d.%d\n",
					(fw >> 8) & 0xFF, fw & 0xFF, (build >> 4) & 0xF, build & 0xF);
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

		strlcpy(adm_type, (i == 0)? "admit_all" : "admit_untagged", sizeof(adm_type));
		_eval(vadmit, DBGOUT, 0, NULL);
		_eval(defcvid, DBGOUT, 0, NULL);

		strlcpy(tag_type, (i == 0)? "untouched" : "untagged", sizeof(tag_type));
		_eval(vegress, DBGOUT, 0, NULL);
		_eval(vingress, DBGOUT, 0, NULL);
	}
}

void __post_config_switch(void)
{
	char *p1_8023az[] = { "ssdk_sh", SWID_IPQ807X, "port", "ieee8023az", "set", "1", "disable", NULL };
	char *p0_vegress[] = { "ssdk_sh", SWID_QCA8337, "portVlan", "egress", "set", QCA8337_CPUPORT, "untouched", NULL };
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

	/* Always turn off IEEE 802.3az support on IPQ8074 port 1,
	 * which connects to QCA8035 and then QCA8337.
	 */
	_eval(p1_8023az, DBGOUT, 0, NULL);

	if (sw_mode() != SW_MODE_ROUTER || !iptv_enabled())
		return;

	_eval(p0_vegress, DBGOUT, 0, NULL);
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
	const int stb_x = nvram_get_int("switch_stb_x");
	const int upstream_wanif = get_dualwan_by_unit(get_upstream_wan_unit()); /* WANS_DUALWAN_IF_XXX */
	const char *no_lan_port_profiles[] = { "spark", "2degrees", "slingshot", "orcon", "voda_nz",
		"tpg", "iinet", "aapt", "intronode", "amaysim", "dodo", "iprimus", NULL
	}, **p;
	char switch_wantag[65];

	strlcpy(switch_wantag, nvram_safe_get("switch_wantag"), sizeof(switch_wantag));
	if (*switch_wantag == '\0')
		return 0;
	else if (stb_x < 0 || stb_x > 6) {
		dbg("%s: unknown switch_stb_x %d\n", __func__, stb_x);
		return 0;
	}

	/* If ISP IPTV profile doesn't need any LAN port, return 0. */
	for (p = &no_lan_port_profiles[0]; *p != NULL; ++p) {
		if (!strcmp(switch_wantag, *p))
			return 0;
	}

	if (upstream_wanif == WANS_DUALWAN_IF_WAN || upstream_wanif == WANS_DUALWAN_IF_WAN2) {
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
		if (wanif_type != WANS_DUALWAN_IF_WAN &&
		    wanif_type != WANS_DUALWAN_IF_WAN2)
			continue;

		unit = i;
	}

	return unit;
}

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
