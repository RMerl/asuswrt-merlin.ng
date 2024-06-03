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

#include <shutils.h>
#include <shared.h>
#include <utils.h>
#include <ralink.h>
#include <mii_mgr.h>		/* $(SRCBASE)/$(PLATFORM_ROUTER)/mii_mgr/source */

/* Comment out this definition, and get the WAN port link status by mdio */
#if !defined(PANTHERB) \
 && !defined(RTAX59U) \
 && !defined(CHEETAH)
#define REDUCE_DUPLICATED_MDIO_QUERY
#endif

/* for vport_to_phy_addr, distinguish whether PHY is on MT7531. */
#define NOT_MT7531_PHY		100

/* MT7531 Register */
#define REG_MAC_PMSR_P0		0x3008
#define REG_SSC_PHY_IAC		0x701c

#define ETH_DEVNAME	"eth0"
#if defined(PANTHERA)
#define NR_WANLAN_PORT	7
#elif defined(TUFAX4200) || defined(TUFAX6000)
#define NR_WANLAN_PORT	6
#elif defined(RTAX59U) || defined(RTAX52)
#define NR_WANLAN_PORT	4
#elif defined(PRTAX57_GO)
#define NR_WANLAN_PORT	2
#else /* PANTHERB, CHEETAH */
#define NR_WANLAN_PORT	5
#endif
#define DBGOUT		NULL	/* "/dev/console" */

enum {
	VLAN_TYPE_STB_VOIP = 0,
	VLAN_TYPE_WAN,
	VLAN_TYPE_WAN_NO_VLAN,	/* Used to bridge WAN/STB for Hinet MOD. */

	VLAN_TYPE_MAX
};

enum {
#if defined(PANTHERA)
	LAN6_PORT=0,
	LAN5_PORT,
	LAN4_PORT,
	LAN3_PORT,
	LAN2_PORT,
	LAN1_PORT,
	WAN_PORT,
#elif defined(TUFAX4200) || defined(TUFAX6000)
	LAN5_PORT=0,
	LAN4_PORT,
	LAN3_PORT,
	LAN2_PORT,
	LAN1_PORT,
	WAN_PORT,
#elif defined(RTAX59U) || defined(RTAX52)
	LAN3_PORT=0,
	LAN2_PORT,
	LAN1_PORT,
	WAN_PORT,
#elif defined(PRTAX57_GO)
	LAN1_PORT=0,
	WAN_PORT,
#else /* PANTHERB, CHEETAH */
	LAN4_PORT=0,
	LAN3_PORT,
	LAN2_PORT,
	LAN1_PORT,
	WAN_PORT,
#endif
	MAX_WANLAN_PORT
};

static const char *upstream_iptv_ifaces[16] = {
	[WANS_DUALWAN_IF_WAN] = "eth1",
};

/* 0:WAN, 1:LAN first index is switch_stb_x nvram variable.
 * lan_wan_partition[switch_stb_x][0] is virtual port0,
 * lan_wan_partition[switch_stb_x][1] is virtual port1, etc.
 */
static const int lan_wan_partition[9][NR_WANLAN_PORT] = {
#if defined(PANTHERA)
	/* L1, L2, L3, L4, L5, L6, W1G */
	{1,1,1,1,1,1,0}, // Normal
	{0,1,1,1,1,1,0}, // IPTV STB port = LAN1
	{1,0,1,1,1,1,0}, // IPTV STB port = LAN2
	{1,1,0,1,1,1,0}, // IPTV STB port = LAN3
	{1,1,1,0,1,1,0}, // IPTV STB port = LAN4
	{0,0,1,1,1,1,0}, // IPTV STB port = LAN1 & LAN2
	{1,1,0,0,1,1,0}, // IPTV STB port = LAN3 & LAN4
	{1,1,1,1,1,1,1}  // ALL
#elif defined(TUFAX4200) || defined(TUFAX6000)
	/* L1, L2, L3, L4, 2.5G LAN, W1G */
	{1,1,1,1,1,0}, // Normal
	{0,1,1,1,1,0}, // IPTV STB port = LAN1
	{1,0,1,1,1,0}, // IPTV STB port = LAN2
	{1,1,0,1,1,0}, // IPTV STB port = LAN3
	{1,1,1,0,1,0}, // IPTV STB port = LAN4
	{0,0,1,1,1,0}, // IPTV STB port = LAN1 & LAN2
	{1,1,0,0,1,0}, // IPTV STB port = LAN3 & LAN4
	{1,1,1,1,1,1}  // ALL
#elif defined(RTAX59U) || defined(RTAX52)
	/* L1, L2, L3, W1G */
	{1,1,1,0}, // Normal
	{0,1,1,0}, // IPTV STB port = LAN1
	{1,0,1,0}, // IPTV STB port = LAN2
	{1,0,1,0}, // IPTV STB port = LAN2 (unused)
	{1,1,0,0}, // IPTV STB port = LAN3
	{0,0,1,0}, // IPTV STB port = LAN1 & LAN2
	{1,0,0,0}, // IPTV STB port = LAN2 & LAN3
	{1,1,1,1}  // ALL
#elif defined(PRTAX57_GO)
	/* L1, W1G */
	{1,0}, // Normal
	{1,0}, // no use
	{1,0}, // no use
	{1,0}, // no use
	{1,0}, // no use
	{1,0}, // no use
	{1,0}, // no use
	{1,1}  // ALL
#else /* PANTHERB, CHEETAH */
	/* L1, L2, L3, L4, W1G */
	{1,1,1,1,0}, // Normal
	{0,1,1,1,0}, // IPTV STB port = LAN1
	{1,0,1,1,0}, // IPTV STB port = LAN2
	{1,1,0,1,0}, // IPTV STB port = LAN3
	{1,1,1,0,0}, // IPTV STB port = LAN4
	{0,0,1,1,0}, // IPTV STB port = LAN1 & LAN2
	{1,1,0,0,0}, // IPTV STB port = LAN3 & LAN4
	{1,1,1,1,1}  // ALL
#endif
};

#if defined(RTCONFIG_BONDING_WAN) || defined(RTCONFIG_LACP)
/* array index:		port number used in wanports_bond, enum bs_port_id.
 * 			0: WAN, 1~6: LAN1~6
 * array element:	virtual port
 * 			e.g. LAN1_PORT ~ LAN6_PORT, WAN_PORT, etc.
 */
static const int bsport_to_vport[MAX_WANLAN_PORT] = {
	WAN_PORT, LAN1_PORT, LAN2_PORT, LAN3_PORT
#if !defined(RTAX59U) && !defined(RTAX52)
	, LAN4_PORT
#endif
#if defined(PANTHERA)
	, LAN5_PORT, LAN6_PORT
#elif defined(TUFAX4200) || defined(TUFAX6000)
	, LAN5_PORT
#endif
};
#endif

/* array index:		virtual port mapping enumeration.
 * 			e.g. LAN1_PORT, LAN2_PORT, etc.
 * array element:	PHY address, negative value means absent PHY.
 *
 * reference from
 * arch/arm64/boot/dts/mediatek/mt7986a-PANTHERA.dts
 *
 *********************************************************
 * Notice: If PHY is not on MT7531, phy_addr needs to be *
 * 	   increased by NOT_MT7531_PHY (100).		 *
 *********************************************************
 */
static const int vport_to_phy_addr[MAX_WANLAN_PORT] = {
#if defined(PANTHERA)
	4, 3, 2, 1, 0, 105, 106				/* LAN6~1, WAN */
#elif defined(TUFAX4200)
	105, 4, 3, 2, 1, 106				/* LAN5~1, WAN */
#elif defined(TUFAX6000)
	105, 1, 2, 3, 4, 106				/* LAN5~1, WAN */
#elif defined(RTAX59U)
	4, 3, 2, 1					/* LAN3~1, WAN */
#elif defined(CHEETAH)
	3, 2, 1, 0, 105					/* LAN4~1, WAN */
#elif defined(PRTAX57_GO)
	124, 100					/* LAN1, WAN */
#elif defined(RTAX52)
	0, 1, 2, 100					/* LAN3~1, WAN */
#else /* PANTHERB */
	0, 1, 2, 3, 4					/* LAN4~1, WAN */
#endif
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
#if defined(PANTHERA)
	"lan4", "lan3", "lan2", "lan1", "lan0", "lan5",		/* LAN6~1 */
#elif defined(TUFAX4200) || defined(TUFAX6000)
	"lan5", "lan4", "lan3", "lan2", "lan1",			/* LAN5~1 */
#elif defined(RTAX59U)
	"lan3", "lan2", "lan1",					/* LAN3~1 */
#elif defined(CHEETAH)
	"lan4", "lan3", "lan2", "lan1",				/* LAN4~1 */
#elif defined(PRTAX57_GO)
	"eth0",							/* LAN1 */
#elif defined(RTAX52)
	"lan1", "lan2", "lan3",					/* LAN3~1 */
#else /* PANTHERB */
	"lan0", "lan1", "lan2", "lan3",				/* LAN4~1 */
#endif
	"eth1"							/* WAN */
};

/* array index:		switch_stb_x nvram variable.
 * array element:	platform specific VoIP/STB virtual port bitmask.
 */
static const unsigned int stb_to_mask[7] = { 0,
#if defined(RTAX59U) || defined(RTAX52)
	(1U << LAN1_PORT),
	(1U << LAN2_PORT),
	(1U << LAN2_PORT), /* unused */
	(1U << LAN3_PORT),
	(1U << LAN1_PORT) | (1U << LAN2_PORT),
	(1U << LAN2_PORT) | (1U << LAN3_PORT)
#elif defined(PRTAX57_GO)
	(1U << LAN1_PORT)
#else
	(1U << LAN1_PORT),
	(1U << LAN2_PORT),
	(1U << LAN3_PORT),
	(1U << LAN4_PORT),
	(1U << LAN1_PORT) | (1U << LAN2_PORT),
	(1U << LAN3_PORT) | (1U << LAN4_PORT)
#endif
};

/* ALL WAN/LAN virtual port bit-mask */
static unsigned int wanlanports_mask =
#if defined(PANTHERA)
					(1U << WAN_PORT) | (1U << LAN1_PORT) | (1U << LAN2_PORT) | (1U << LAN3_PORT) | (1U << LAN4_PORT) | (1U << LAN5_PORT) | (1U << LAN6_PORT);
#elif defined(TUFAX4200) || defined(TUFAX6000)
					(1U << WAN_PORT) | (1U << LAN1_PORT) | (1U << LAN2_PORT) | (1U << LAN3_PORT) | (1U << LAN4_PORT) | (1U << LAN5_PORT);
#elif defined(RTAX59U) || defined(RTAX52)
					(1U << WAN_PORT) | (1U << LAN1_PORT) | (1U << LAN2_PORT) | (1U << LAN3_PORT);
#elif defined(PRTAX57_GO)
					(1U << WAN_PORT) | (1U << LAN1_PORT);
#else /* PANTHERB, CHEETAH */
					(1U << WAN_PORT) | (1U << LAN1_PORT) | (1U << LAN2_PORT) | (1U << LAN3_PORT) | (1U << LAN4_PORT);
#endif

/* Final model-specific LAN/WAN/WANS_LAN partition definitions.
 * bit0: P0, bit1: P1, bit2: P2, bit3: P3, bit4: P4
 */
static unsigned int lan_mask = 0;	/* LAN only. Exclude WAN, WANS_LAN, and generic IPTV port. */
static unsigned int wan_mask = 0;	/* wan_type = WANS_DUALWAN_IF_WAN. Include generic IPTV port. */
static unsigned int wans_lan_mask = 0;	/* wan_type = WANS_DUALWAN_IF_LAN. */

int esw_fd;

/* RT-N56U's P0, P1, P2, P3, P4 = LAN4, LAN3, LAN2, LAN1, WAN
 * ==> Model-specific virtual port number.
 * array inex:	RT-N56U's port number.
 * array value:	Model-specific virtual port number
 */
static int n56u_to_model_port_mapping[] = {
#if defined(RTAX59U) || defined(RTAX52)
	LAN3_PORT,	//0000 0000 0001 LAN4 (convert to LAN3)
	LAN2_PORT,	//0000 0000 0010 LAN3 (convert to LAN2)
	LAN2_PORT,	//0000 0000 0100 LAN2
	LAN1_PORT,	//0000 0000 1000 LAN1
	WAN_PORT,	//0000 0001 0000 WAN
#elif defined(PRTAX57_GO)
	LAN1_PORT,	//0000 0000 1000 LAN1
	WAN_PORT,	//0000 0001 0000 WAN
#else
	LAN4_PORT,	//0000 0000 0001 LAN4
	LAN3_PORT,	//0000 0000 0010 LAN3
	LAN2_PORT,	//0000 0000 0100 LAN2
	LAN1_PORT,	//0000 0000 1000 LAN1
	WAN_PORT,	//0000 0001 0000 WAN
#endif
};

#define RTN56U_WAN_GMAC	(1U << 9)

/* Model-specific LANx ==> Model-specific virtual PortX.
 * array index:	Model-specific LANx (started from 0).
 * array value:	Model-specific virtual port number.
 */
const int lan_id_to_vport[NR_WANLAN_PORT] = {
	LAN1_PORT,
#if !defined(PRTAX57_GO)
	LAN2_PORT,
	LAN3_PORT,
#if !defined(RTAX59U) && !defined(RTAX52)
	LAN4_PORT,
#endif
#if defined(PANTHERA)
	LAN5_PORT,
	LAN6_PORT,
#elif defined(TUFAX4200) || defined(TUFAX6000)
	LAN5_PORT,
#endif
#endif
	WAN_PORT,
};

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
	char nv[] = "wanXXXports_maskXXXXXX";

	if (sw_mode() == SW_MODE_REPEATER)
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

/* HwId A: Single 2.5G PHY, two WiFi LED.
 * HwId B: Two 2.5G PHY, single WiFi LED.
 */
int is_2500m_lan_exist(void)
{
	if (!iface_exist("lan5") || nvram_match("HwId", "A"))
		return 0;
	return 1;
}

int switch_init(void)
{
	esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (esw_fd < 0) {
		perror("socket");
		return -1;
	}
	return 0;
}

void switch_fini(void)
{
	close(esw_fd);
}

int mt7986_mt7531_reg_read(unsigned int phy, unsigned int reg, unsigned int *value)
{
	struct ifreq ifr;
	struct mtk_mii_ioctl_data mii;

	strlcpy(ifr.ifr_name, ETH_DEVNAME, IFNAMSIZ);
	ifr.ifr_data = (char *)&mii;

	mii.phy_id = phy;
	mii.reg_num = reg;

	if (-1 == ioctl(esw_fd, MTKETH_MII_READ, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		return -1;
	}

	*value = mii.val_out;
	//dbg("%s: reg=%4x, value=%8x\n", __func__, reg, *value);

	return 0;
} 

int mt7986_mt7531_reg_write(unsigned int phy, unsigned int reg, unsigned int value)
{
	struct ifreq ifr;
	struct mtk_mii_ioctl_data mii;

	//dbg("%s: reg=%4x, value=%8x\n", __func__, reg, value);
	strlcpy(ifr.ifr_name, ETH_DEVNAME, IFNAMSIZ);
	ifr.ifr_data = (char *)&mii;

	mii.phy_id = phy;
	mii.reg_num = reg;
	mii.val_in = value;

	if (-1 == ioctl(esw_fd, MTKETH_MII_WRITE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		return -1;
	}

	return 0;
}

int mt7986_mt7531_phy_read(unsigned int port_num, unsigned int reg, unsigned int *value)
{
	unsigned int reg_value;
	int loop_cnt;
	int op_busy;

	loop_cnt = 0;

	reg_value = 0x80090000 | (port_num << 20) | (reg << 25);
	mt7986_mt7531_reg_write(0x1f, REG_SSC_PHY_IAC, reg_value);
	while (1) {
		mt7986_mt7531_reg_read(0x1f, REG_SSC_PHY_IAC, &reg_value);
		op_busy = reg_value & (1 << 31);
		if (!op_busy) {
			reg_value = reg_value & 0xffff;
			break;
		}
		else if (loop_cnt < 10)
			loop_cnt++;
		else {
			dbg("%s: MDIO read opeartion timeout\n", __func__);
			reg_value = 0;
			break;
		}
	}

	//dbg("%s: PHY Indirect Access Control(0x701c) register read value = 0x%x\n", __func__, reg_value);
	*value = reg_value;

	return 0;
}

int mt7986_mt7531_phy_write(unsigned int port_num, unsigned int reg, unsigned int value)
{
	unsigned int reg_value;
	int loop_cnt;
	int op_busy;

	loop_cnt = 0;

	reg_value = 0x80050000 | (port_num << 20) | (reg << 25) | value;
	mt7986_mt7531_reg_write(0x1f, REG_SSC_PHY_IAC, reg_value);
	while (1) {
		mt7986_mt7531_reg_read(0x1f, REG_SSC_PHY_IAC, &reg_value);
		op_busy = reg_value & (1 << 31);
		if (!op_busy)
			break;
		else if (loop_cnt < 10)
			loop_cnt++;
		else {
			dbg("%s: MDIO write opeartion timeout\n", __func__);
			break;
		}
	}

	//dbg("%s: PHY Indirect Access Control(0x701c) register write value = 0x%x\n", __func__, reg_value);

	return 0;
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
int mt7986_mt7531_vlan_set(int vtype, char *upstream_if, int vid, int prio, unsigned int mbr, unsigned int untag)
{
	unsigned int m = mbr, u = untag, upstream_mask = 0;
	int vport, upstream_vport, wan_vlan_br = 0, wan_br = 0;
	char wvlan_if[IFNAMSIZ], vid_str[6], prio_str[4], brv_if[IFNAMSIZ];
	char *add_upst_viface[] = { "ip", "link", "add", "link", upstream_if, wvlan_if, "type", "vlan", "id", vid_str, NULL };
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
		if (!iface_exist("bond1")) {
			f_write_string("/sys/class/net/bonding_masters", "+bond1", 0, 0);
			sleep(1);
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

	if ((vtype == VLAN_TYPE_WAN && wan_vlan_br) || vtype == VLAN_TYPE_STB_VOIP) {
		for (vport = 0, m = mbr & ~upstream_mask, u = untag;
		     vport < MAX_WANLAN_PORT && m > 0;
		     vport++, m >>= 1, u >>= 1)
		{
			char *tmp_str;
			if (!(m & 1))
				continue;

			dbg("%s: vport %d iface %s vid %d prio %d u %d\n",
				__func__, vport, vport_to_iface[vport], vid, prio, u & 1);

			if (u & 1) {
				char br_untag_path[] = "/sys/class/net/ethXXXXXXXXXXXX/brport/untagged_vlan_en";
				eval("brctl", "addif", brv_if, (char*) vport_to_iface[vport]);
				snprintf(br_untag_path, sizeof(br_untag_path), "/sys/class/net/%s/brport/untagged_vlan", (char*) vport_to_iface[vport]);
				f_write_string(br_untag_path, vid_str, 0, 0); // vid value, include 0
				snprintf(br_untag_path, sizeof(br_untag_path), "/sys/class/net/%s/brport/untagged_vlan_en", (char*) vport_to_iface[vport]);
				f_write_string(br_untag_path, "1", 0, 0); // enable
			} else {
				char lan_if[IFNAMSIZ], lan_vlan_if[IFNAMSIZ];
				char *add_lan_vlan_viface[] = { "ip", "link", "add", "link", lan_if, lan_vlan_if, "type", "vlan", "id", vid_str, NULL };
				char *set_lan_vlan_egress_map[] = { "vconfig", "set_egress_map", lan_vlan_if, "0", prio_str, NULL };
				snprintf(lan_if, sizeof(lan_if), "%s",  (char *)vport_to_iface[vport]);
				snprintf(lan_vlan_if, sizeof(lan_vlan_if), "%s.%d", (char *)vport_to_iface[vport], vid);
				_eval(add_lan_vlan_viface, DBGOUT, 0, NULL);
				_eval(set_lan_vlan_egress_map, DBGOUT, 0, NULL);
				eval("ifconfig", lan_vlan_if, "0.0.0.0", "up");
				eval("brctl", "addif", brv_if, lan_vlan_if);
			}
			// remove lan_nic from lan_ifnames
			tmp_str = strdup(nvram_safe_get("lan_ifnames"));
			if (tmp_str) {
				if (remove_word(tmp_str, ((char *)vport_to_iface[vport]))) {
					trim_space(tmp_str);
					nvram_set("lan_ifnames", tmp_str);
				}
				free(tmp_str);
				eval("ifconfig", (char *)vport_to_iface[vport], "0.0.0.0", "up");
			}
		}
	}

	return 0;
}

#ifndef REDUCE_DUPLICATED_MDIO_QUERY
/**
 * Get link status and/or phy speed of a port. (by mdio)
 * @link:	pointer to unsigned integer.
 * 		If link != NULL,
 * 			*link = 0 means link-down
 * 			*link = 1 means link-up.
 * @speed:	pointer to unsigned integer.
 * 		If speed != NULL,
 * 			*speed = 10/100/1000/2500 Mbps
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *  otherwise:	fail
 */
static void get_phy_info_by_mdio(unsigned int phy, unsigned int *link, unsigned int *speed, phy_info *info)
{
	unsigned int value;
	unsigned int l = 0, s = 0;

	if (switch_init() < 0)
		return;

	if (phy >= 0 && phy <= 4) {
		mt7986_mt7531_reg_read(0x1f, (REG_MAC_PMSR_P0 + 0x100*phy), &value);
		l = value & 0x1;
		s = (value >> 2) & 0x3;
	}
	else {
		mt7986_mt7531_reg_read((phy - NOT_MT7531_PHY), 0x1, &value);
		l = (value >> 2) & 0x1;
		if (l) {
			mt7986_mt7531_reg_read((phy - NOT_MT7531_PHY), 0x18, &value);
			s = value & 0x7;
		}
	}

	if (link) {
		*link = l;
		if (info) {
			if (l)
				snprintf(info->state, sizeof(info->state), "up");
			else
				snprintf(info->state, sizeof(info->state), "down");
		}
	}
	if (speed) {
		switch (s) {
		case 0x0:
			*speed = 10;
			if (l && info)
				info->link_rate = 10;
			break;
		case 0x1:
			*speed = 100;
			if (l && info)
				info->link_rate = 100;
			break;
		case 0x2:
			*speed = 1000;
			if (l && info)
				info->link_rate = 1000;
			break;
		case 0x4:
			*speed = 2500;
			if (l && info)
				info->link_rate = 2500;
			break;
		default:
			_dprintf("%s: invalid speed!\n", __func__);
		}
	}

	switch_fini();
}
#endif /* REDUCE_DUPLICATED_MDIO_QUERY */

/**
 * Get link status and/or phy speed of a port. (by sysfs)
 * use /sys/class/net/NIC/speed to retrieve information
 * @link:	pointer to unsigned integer.
 * 		If link != NULL,
 * 			*link = 0 means link-down
 * 			*link = 1 means link-up.
 * @speed:	pointer to unsigned integer.
 * 		If speed != NULL,
 * 			*speed = 10/100/1000/2500 Mbps
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *  otherwise:	fail
 */
static void get_phy_info_by_sysfs(unsigned int vport, unsigned int *link, unsigned int *speed, phy_info *info)
{
	unsigned int l = 0, s = 0;
	char path[256];
	char sp_str[10]; // -1, 10, 100, 1000, 2500, 10000

	snprintf(path, sizeof(path), "/sys/class/net/%s/speed", vport_to_iface[vport]);
	if (f_read_string(path, sp_str, sizeof(sp_str)) > 0) {
		int sp_val = atoi(sp_str);
		if (sp_val > 0) {
			l = 1;
			s = sp_val;
		}
	}

	if (link) {
		*link = l;
		if (info) {
			if (l)
				snprintf(info->state, sizeof(info->state), "up");
			else
				snprintf(info->state, sizeof(info->state), "down");
		}
	}
	if (speed) {
		*speed = s;
		if (l && info)
			info->link_rate = s;
	}
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
static int get_mt7986_mt7531_vport_info(unsigned int vport, unsigned int *link, unsigned int *speed, phy_info *info)
{
#ifndef REDUCE_DUPLICATED_MDIO_QUERY
	int phy;
#endif

	if (vport >= MAX_WANLAN_PORT || (!link && !speed))
		return -1;

	if (link)
		*link = 0;
	if (speed)
		*speed = 0;

#ifndef REDUCE_DUPLICATED_MDIO_QUERY
	if (!strcmp(vport_to_iface[vport], "eth1")) {
		phy = *(vport_to_phy_addr + vport);
		if (phy < 0) {
			dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
			return -1;
		}

		get_phy_info_by_mdio(phy, link, speed, info);
	}
	else
#endif
		get_phy_info_by_sysfs(vport, link, speed, info);

	return 0;
}

/**
 * Get linkstatus in accordance with port bit-mask.
 * @mask:	port bit-mask.
 * 		bit0 = P0, bit1 = P1, etc.
 * @linkStatus:	link status of all ports that is defined by mask.
 * 		If one port of mask is linked-up, linkStatus is true.
 */
static void get_mt7986_mt7531_phy_linkStatus(unsigned int mask, unsigned int *linkStatus)
{
	int i;
	unsigned int value = 0, m;

	m = mask & wanlanports_mask;
	for (i = 0; m > 0 && !value; ++i, m >>= 1) {
		if (!(m & 1))
			continue;

		get_mt7986_mt7531_vport_info(i, &value, NULL, NULL);
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
	int i, unit, type, m, upstream_unit = get_upstream_wan_unit();
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

	if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 5)) {
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
		default:
			_dprintf("%s: Unknown LAN/WAN port definition. (stb %d i %d val %d)\n",
				__func__, stb, i, lan_wan_partition[stb][i]);
		}
	}

	/* One of LAN port is acting as WAN. */
	if (wanscap_lan) {
		wans_lan_mask = 1U << lan_id_to_vport_nr(wans_lanport - 1);
		lan_mask &= ~wans_lan_mask;
	}

	unused_wan_mask = wan_mask;
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
static void config_mt7986_mt7531_LANWANPartition(int type)
{
	build_wan_lan_mask(type, 0);
	dbg("%s: LAN/P.WAN/S.WAN portmask %08x/%08x/%08x Upstream %s (unit %d)\n",
		__func__, lan_mask, nvram_get_int("wanports_mask"), nvram_get_int("wan1ports_mask"),
		get_wan_base_if(), get_upstream_wan_unit());
}

static void get_mt7986_mt7531_Port_Speed(unsigned int port_mask, unsigned int *speed)
{
	int i;
	unsigned int value = 0, m;

	if (speed == NULL)
		return;

	m = port_mask & wanlanports_mask;
	for (i = 0; m; ++i, m >>= 1) {
		if (!(m & 1))
			continue;

		get_mt7986_mt7531_vport_info(i, NULL, &value, NULL);
	}
	*speed = value;
}

static void get_mt7986_mt7531_WAN_Speed(unsigned int *speed)
{
	get_mt7986_mt7531_Port_Speed(get_wan_port_mask(0) | get_wan_port_mask(1), speed);
}

static void link_down_up_mt7986_mt7531_PHY(unsigned int vpmask, int status)
{
	int vport, phy;
	unsigned int value = 0, m;

	if (switch_init() < 0)
		return;

	vpmask &= wanlanports_mask;
	for (vport = 0, m = vpmask; m; ++vport, m >>= 1) {
		if (!(m & 1))
			continue;

		phy = *(vport_to_phy_addr + vport);
		if (phy < 0) {
			dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
			return;
		}

		if (phy >= 0 && phy <= 4)
			mt7986_mt7531_phy_read(phy, 0x0, &value);
		else
			mt7986_mt7531_reg_read((phy - NOT_MT7531_PHY), 0x0, &value);
		if (!status)
			value |= 0x0800; /* power down PHY */
		else
			value &= 0xf7ff; /* power up PHY */
		if (phy >= 0 && phy <= 4)
			mt7986_mt7531_phy_write(phy, 0x0, value);
		else
			mt7986_mt7531_reg_write((phy - NOT_MT7531_PHY), 0x0, value);
	}

	switch_fini();
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
	stb_bitmask = convert_n56u_portmask_to_model_portmask(stb_bitmask);
	build_wan_lan_mask(0, stb_bitmask);
	dbg("%s: LAN/P.WAN/S.WAN portmask %08x/%08x/%08x Upstream %s (unit %d)\n",
		__func__, lan_mask, nvram_get_int("wanports_mask"), nvram_get_int("wan1ports_mask"),
		get_wan_base_if(), get_upstream_wan_unit());
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
	unsigned int orig_mbr = bitmask & 0xffff;
	unsigned int orig_untag = (bitmask >> 16) & 0xffff;
	unsigned int conv_mbr, conv_untag;
	int vtype = VLAN_TYPE_STB_VOIP;
	char upstream_if[IFNAMSIZ];

	//convert port mapping
	dbg("%s: bitmask:%08x, mbr:%08x, untag:%08x\n", __func__, bitmask, orig_mbr, orig_untag);
	conv_mbr = convert_n56u_portmask_to_model_portmask(orig_mbr);
	conv_untag = convert_n56u_portmask_to_model_portmask(orig_untag);
	dbg("%s: after conversion mbr:%08x, untag:%08x\n", __func__, conv_mbr, conv_untag);
	if ((nvram_match("switch_wantag", "none") && stb_x > 0) ||
	    nvram_match("switch_wantag", "hinet")) {
		vtype = VLAN_TYPE_WAN_NO_VLAN;
	} else if (orig_mbr & RTN56U_WAN_GMAC) {
		/* setup VLAN for WAN (WAN1 or WAN2), not VoIP/STB */
		vtype = VLAN_TYPE_WAN;
	}

	/* selecet upstream port for IPTV port. */
	strlcpy(upstream_if, get_wan_base_if(), sizeof(upstream_if));
	mt7986_mt7531_vlan_set(vtype, upstream_if, vid, prio, conv_mbr, conv_untag);
}

int mt7986_mt7531_ioctl(int val, int val2)
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
		config_mt7986_mt7531_LANWANPartition(val2);
		break;
	case 13:
		get_mt7986_mt7531_WAN_Speed(&value2);
		printf("WAN speed : %u Mbps\n", value2);
		break;
	case 14: // Link up LAN ports
		link_down_up_mt7986_mt7531_PHY(get_lan_port_mask(), 1);
		break;
	case 15: // Link down LAN ports
		link_down_up_mt7986_mt7531_PHY(get_lan_port_mask(), 0);
		break;
	case 16: // Link up ALL ports
		link_down_up_mt7986_mt7531_PHY(wanlanports_mask, 1);
		break;
	case 17: // Link down ALL ports
		link_down_up_mt7986_mt7531_PHY(wanlanports_mask, 0);
		break;
	case 21:
		break;
	case 22:
		break;
	case 23:
		break;
	case 24:
		break;
	case 25:
		break;
	case 27:
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
		break;
	case 50:
		break;
	case 114: // link up WAN ports
		for (i = WAN_UNIT_FIRST; i <= max_wan_unit; ++i)
			link_down_up_mt7986_mt7531_PHY(get_wan_port_mask(i), 1);
		break;
	case 115: // link down WAN ports
		for (i = WAN_UNIT_FIRST; i <= max_wan_unit; ++i)
			link_down_up_mt7986_mt7531_PHY(get_wan_port_mask(i), 0);
		break;
	case 200:	/* set LAN port number that is used as WAN port */
		/* Nothing to do, nvram_get_int("wans_lanport ") is enough. */
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

	return mt7986_mt7531_ioctl(val, val2);
}

int ralink_gpio_write_bit(int idx, int value)
{
	return 0;
}

int ralink_gpio_read_bit(int idx)
{
	return 0;
}

int ralink_gpio_init(unsigned int idx, int dir)
{
	return 0;
}

unsigned int rtkswitch_wanPort_phyStatus(int wan_unit)
{
	unsigned int status = 0;

#if defined(RTCONFIG_BONDING_WAN)
	if (bond_wan_enabled() && sw_mode() == SW_MODE_ROUTER && get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_WAN) {
		char ifname[8], *next;
		int vport, phy;
		unsigned int link = 0;

		foreach(ifname, nvram_safe_get("bond1_ifnames"), next) {
			vport = iface_name_to_vport(ifname);
			phy = vport_to_phy_addr[vport];
			if (phy < 0) {
				dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
				return 0;
			}

#ifndef REDUCE_DUPLICATED_MDIO_QUERY
			if (!strcmp(vport_to_iface[vport], "eth1"))
				get_phy_info_by_mdio(phy, &link, NULL, NULL);
			else
#endif
				get_phy_info_by_sysfs(vport, &link, NULL, NULL);
			status |= link;
		}

		return status;
	}
#endif

	get_mt7986_mt7531_phy_linkStatus(get_wan_port_mask(wan_unit), &status);

	return status;
}

unsigned int rtkswitch_lanPorts_phyStatus(void)
{
	unsigned int status = 0;

	get_mt7986_mt7531_phy_linkStatus(get_lan_port_mask(), &status);

	return status;
}

unsigned int rtkswitch_WanPort_phySpeed(void)
{
	unsigned int speed;

	get_mt7986_mt7531_WAN_Speed(&speed);

	return speed;
}

unsigned int __rtkswitch_WanPort_phySpeed(int wan_unit)
{
	unsigned int speed;

	if (wan_unit < 0 || wan_unit >= WAN_UNIT_MAX)
		return 0;

	get_mt7986_mt7531_Port_Speed(get_wan_port_mask(wan_unit), &speed);

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

int rtkswitch_LanPort_linkUp(void)
{
	system("rtkswitch 14");

	return 0;
}

int rtkswitch_LanPort_linkDown(void)
{
	system("rtkswitch 15");

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
	char buf[64];
#ifdef RTCONFIG_NEW_PHYMAP
	char cap_buf[64] = {0};
#endif
	phyState pS;

#ifdef RTCONFIG_NEW_PHYMAP
	phy_port_mapping port_mapping;
	get_phy_port_mapping(&port_mapping);

	len = 0;
	for (i = 0; i < port_mapping.count; i++) {
		// Only handle WAN/LAN ports
		if (((port_mapping.port[i].cap & PHY_PORT_CAP_WAN) == 0) && ((port_mapping.port[i].cap & PHY_PORT_CAP_LAN) == 0))
			continue;
		pS.link[i] = 0;
		pS.speed[i] = 0;
		get_mt7986_mt7531_vport_info(port_mapping.port[i].phy_port_id, &pS.link[i], &pS.speed[i], list ? &list->phy_info[i] : NULL);
		if (list) {
			list->phy_info[i].phy_port_id = port_mapping.port[i].phy_port_id;
			snprintf(list->phy_info[i].label_name, sizeof(list->phy_info[i].label_name), "%s", 
				port_mapping.port[i].label_name);
			list->phy_info[i].cap = port_mapping.port[i].cap;
			snprintf(list->phy_info[i].cap_name, sizeof(list->phy_info[i].cap_name), "%s", 
				get_phy_port_cap_name(port_mapping.port[i].cap, cap_buf, sizeof(cap_buf)));
			if (pS.link[i] == 1 && !list->status_and_speed_only) {
				// TODO not complete
				//get_mt7986_mt7531_port_mib(port_mapping.port[i].phy_port_id, &list->phy_info[i]);
			}

			list->count++;
		}
		len += sprintf(buf+len, "%s=%C;", port_mapping.port[i].label_name,
			conv_speed(pS.link[i], pS.speed[i]));
	}

#else
	for (i = 0; i < NR_WANLAN_PORT; i++) {
		get_mt7986_mt7531_vport_info(lan_id_to_vport_nr(i), &pS.link[i], &pS.speed[i], NULL);
	}

	len = 0;
	len += sprintf(buf+len, "W0=%C;", conv_speed(pS.link[WAN_PORT], pS.speed[WAN_PORT]));
	for (i = 0; i < WAN_PORT; i++) {
		len += sprintf(buf+len, "L%d=%C;", i+1, conv_speed(pS.link[i], pS.speed[i]));
	}
#endif

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

void __pre_config_switch(void)
{
	const int stb_x = nvram_get_int("switch_stb_x");
	int i, j, nr_brvx, nr_brif;
	struct dirent **brvx = NULL, **brif = NULL;
	char brif_path[sizeof("/sys/class/net/X/brifXXXXX") + IFNAMSIZ], iface[IFNAMSIZ];
#if defined(TUFAX4200)
	int port;
	char port_str[4];
#endif

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

	/* up lanX before implement STB/VoIP, otherwise lanX will not work */
	if (!nvram_match("switch_wantag", "none") || stb_x > 0) {
		//dbg("%s: up lanX/eth1 before implement STB/VoIP!\n", __func__);
		for (i = 0; i < ARRAY_SIZE(vport_to_iface); ++i) {
			strlcpy(iface, vport_to_iface[i], sizeof(iface));
			eval("ifconfig", iface, "0.0.0.0", "up");
		}
	}

#if defined(TUFAX4200)
	/* Fine-tune MT7531 Ethernet ports rise/fall time. */
	for (port = 1; port <= 4; ++port) {
		snprintf(port_str, sizeof(port_str), "%d", port);
		eval("switch", "phy", "cl45", "w", port_str, "0x1e", "0x1", "0x1b7");
		eval("switch", "phy", "cl45", "w", port_str, "0x1e", "0x7", "0x3ba");
		eval("switch", "phy", "cl45", "w", port_str, "0x1e", "0x4", "0x200");
		eval("switch", "phy", "cl45", "w", port_str, "0x1e", "0xA", "0x0");
	}
#endif
}

void __post_config_switch(void)
{
	return;
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
	return 1;
}

int __sw_bridge_iptv_different_switches(void)
{
	return 1;
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

#if defined(RTCONFIG_BONDING_WAN)
/** Helper function of get_bonding_port_status().
 * Convert bonding slave port definition that is used in wanports_bond to our virtual port definition
 * and get link status/speed of it.
 * @bs_port:	bonding slave port number, 0: WAN, 1~6: LAN1~6
 * @return:
 *  <= 0:	disconnected
 *  otherwise:	link speed
 */
int __get_bonding_port_status(enum bs_port_id bs_port)
{
	int vport, phy;
	unsigned int link = 0, speed = 0;

	if (bs_port < 0 || bs_port >= ARRAY_SIZE(bsport_to_vport))
		return 0;

	vport = bsport_to_vport[bs_port];
	phy = vport_to_phy_addr[vport];
	if (phy < 0) {
		dbg("%s: can't get PHY address of vport %d\n", __func__, vport);
		return 0;
	}
#ifndef REDUCE_DUPLICATED_MDIO_QUERY
	if (!strcmp(vport_to_iface[vport], "eth1"))
		get_phy_info_by_mdio(phy, &link, &speed, NULL);
	else
#endif
		get_phy_info_by_sysfs(vport, &link, &speed, NULL);

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
	return vport_to_iface_name(vport);
}
#endif

#if defined(TUFAX4200) || defined(TUFAX6000)
/* Force GPY211 PHY LED on/off
 * @port:
 * @mode:	0: OFF, otherwise: ON
 */
void force_gpy211_led_onoff(int port, int mode)
{
	int inv = 0;
	pid_t pid;
	char *nv __attribute__((unused)) = NULL, port_str[4], value_str[8];
	char *nv_commit[] = { "nvram", "commit", NULL };

#if defined(TUFAX4200)
	if (port == 5 && !is_2500m_lan_exist())
		return;
#endif
	/* 2.5G WAN LED of TUF-AX4200 is active-low. */
	if (port == 6) {
		inv = 1 << 12;	/* inverse */
		nv = "led_wan_last_state";
	} else if (port == 5) {
		nv = "led_lan_last_state";
	}

	/* GPHY211 LED, Register 0.27
	 * bit0: direct access of LED0
	 * bit8: disable(0)/enable(1) LED0 function
	 */
	snprintf(port_str, sizeof(port_str), "%d", port);
	snprintf(value_str, sizeof(value_str), "0x%x", mode | inv);
	eval("mii_mgr", "-s", "-p", port_str, "-d", "0", "-r", "0x1b", "-v", value_str);
	if (nv) {
		nvram_set_int(nv, !!mode);
		if (!nvram_match("x_Setting", "1"))
			_eval(nv_commit, NULL, 0, &pid);
	}
}

/* Force MT7531 switch LED on/off
 * @mode:	0: OFF, otherwise: ON
 */
void force_mt7531_led_onoff(int mode)
{
	pid_t pid;
	char *nv_commit[] = { "nvram", "commit", NULL };

	if (mode) {
		/* Make sure LEDs are not controlled by LED_MODE,
		 * force on link LED (LED0) and turn off activity LED (LED1).
		 */
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x21", "0x8009");
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x24", "0x8040");
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x26", "0x0");
	} else {
		/* Use LED_MODE to control LEDs and disable all LEDs, clock must be sustain. */
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x21", "0x8");
	}
	nvram_set_int("led_wan_last_state", !!mode);
	if (!nvram_match("x_Setting", "1"))
		_eval(nv_commit, NULL, 0, &pid);
}

/* Enable/turn oFF GPY211 PHY LED
 * @port:
 * @mode:	LED_ON and LED_OFF
 */
void set_gpy211_led_onoff(int port, int mode)
{
	int value, inv = 0;
	pid_t pid;
	char *nv __attribute__((unused)) = NULL, port_str[4], value_str[8];
	char *nv_commit[] = { "nvram", "commit", NULL };

#if defined(TUFAX4200)
	if (port == 5 && !is_2500m_lan_exist())
		return;
#endif
	/* 2.5G WAN LED of TUF-AX4200 is active-low. */
	if (port == 6) {
		inv = 1 << 12;	/* inverse */
		nv = "led_wan_last_state";
	} else if (port == 5) {
		nv = "led_lan_last_state";
	}

	snprintf(port_str, sizeof(port_str), "%d", port);
	if (mode == LED_ON) {
		/* GPHY211 LED, Register 0.27, enabled LED function.
		 * bit0: direct access of LED0
		 * bit8: disable(0)/enable(1) LED0 function
		 */
		value = inv | (1U << 8);
		snprintf(value_str, sizeof(value_str), "0x%x", value);
		eval("mii_mgr", "-s", "-p", port_str, "-d", "0", "-r", "0x1b", "-v", value_str);
	} else {
		/* Disable LED function and use direct-access to turn it off. */
		value = inv;
		snprintf(value_str, sizeof(value_str), "0x%x", value);
		eval("mii_mgr", "-s", "-p", port_str, "-d", "0", "-r", "0x1b", "-v", value_str);
	}

	if (nv) {
		nvram_set_int(nv, (mode == LED_ON)? LED_ON : LED_OFF);
		if (!nvram_match("x_Setting", "1"))
			_eval(nv_commit, NULL, 0, &pid);
	}
}

/* Enable/turn off MT7531 switch LED.
 * @mode:	LED_ON and LED_OFF
 */
void set_mt7531_led_onoff(int mode)
{
	pid_t pid;
	char *nv_commit[] = { "nvram", "commit", NULL };

	if (mode == LED_ON) {
		/* Make sure LEDs are not controlled by LED_MODE, restore LED0 and LED1 settings. */
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x21", "0x8009");
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x24", "0x8000");
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x26", "0xc007");
	} else {
		/* Use LED_MODE to control LEDs and disable all LEDs, clock must be sustain. */
		eval("switch", "phy", "cl45", "w", "0", "0x1f", "0x21", "0x8");
	}
	nvram_set_int("led_lan_last_state", (mode == LED_ON)? LED_ON : LED_OFF);
	if (!nvram_match("x_Setting", "1"))
		_eval(nv_commit, NULL, 0, &pid);
}

/* Platform-specific and managed by hardware.
 * @which:	enum led_id
 * @mode:	LED_ON, LED_OFF
 * @return:
 * 	0:	@which is not processed.
 *  otherwise:	@which is processed.
 */
int __do_led_control(int which, int mode)
{
	int ret = 0;

	switch (which) {
	case LED_WAN:
		if (mode != nvram_get_int("led_wan_last_state")) {
			set_gpy211_led_onoff(6, mode);
		}
		nvram_set_int("led_wan_last_state", mode);
		ret = 1;
		break;
	case LED_LAN:
		if (mode != nvram_get_int("led_lan_last_state")) {
			set_gpy211_led_onoff(5, mode);
			set_mt7531_led_onoff(mode);
		}
		nvram_set_int("led_lan_last_state", mode);
		ret = 1;
		break;
	}

	return ret;
}

#elif defined(RTAX52)
/* Enable/turn off MT7531 switch LED.
 * @mode:	0: GPIO mode
 * 		1: default mode
 * @onoff:	force LED on/off under GPIO mode
 */
void set_mt7531_led(int mode, int onoff)
{
	switch (mode) {
	case 0:		/* GPIO mode */
		eval("switch", "reg", "w", "7c00", "1462000");		/* set LANx_LED0 direction to output */
		if (onoff)
			eval("switch", "reg", "w", "7c04", "0");	/* set LANx_LED0 output to low */
		else
			eval("switch", "reg", "w", "7c04", "1462000");	/* set LANx_LED0 output to high */
		eval("switch", "reg", "w", "7c10", "11011111");		/* set LAN4_LED0 to GPIO mode */
		eval("switch", "reg", "w", "7c14", "10110000");		/* set LAN1_LED0/LAN2_LED0/LAN3_LED0 to GPIO mode */
		eval("switch", "reg", "w", "7c18", "110");		/* set LAN0_LED0 to GPIO mode */
		break;
	default:	/* default mode */
		eval("switch", "reg", "w", "7c10", "11111111");		/* set LAN4_LED0 to default mode */
		eval("switch", "reg", "w", "7c14", "11110110");		/* set LAN1_LED0/LAN2_LED0/LAN3_LED0 to default mode */
		eval("switch", "reg", "w", "7c18", "111");		/* set LAN0_LED0 to default mode */
	}
}
#endif // end of defined(TUFAX4200) || defined(TUFAX6000)

#ifdef RTCONFIG_NEW_PHYMAP
void mt798x_get_phy_port_mapping(phy_port_mapping *port_mapping)
{
	int i, id;
	static phy_port_mapping port_mapping_static = {
#if defined(TUFAX4200) || defined(TUFAX6000)
		.count = NR_WANLAN_PORT,
		.port[0] = { .phy_port_id = WAN_PORT,  .ext_port_id = -1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[1] = { .phy_port_id = LAN1_PORT, .ext_port_id = -1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[2] = { .phy_port_id = LAN2_PORT, .ext_port_id = -1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[3] = { .phy_port_id = LAN3_PORT, .ext_port_id = -1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[4] = { .phy_port_id = LAN4_PORT, .ext_port_id = -1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[5] = { .phy_port_id = LAN5_PORT, .ext_port_id = -1, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
#elif defined(PANTHERA)
		.count = NR_WANLAN_PORT,
		.port[0] = { .phy_port_id = WAN_PORT,  .ext_port_id = -1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[1] = { .phy_port_id = LAN1_PORT, .ext_port_id = -1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[2] = { .phy_port_id = LAN2_PORT, .ext_port_id = -1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[3] = { .phy_port_id = LAN3_PORT, .ext_port_id = -1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[4] = { .phy_port_id = LAN4_PORT, .ext_port_id = -1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[5] = { .phy_port_id = LAN5_PORT, .ext_port_id = -1, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[6] = { .phy_port_id = LAN6_PORT, .ext_port_id = -1, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
#elif defined(PANTHERB)
		.count = NR_WANLAN_PORT,
		.port[0] = { .phy_port_id = WAN_PORT,  .ext_port_id = -1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[1] = { .phy_port_id = LAN1_PORT, .ext_port_id = -1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[2] = { .phy_port_id = LAN2_PORT, .ext_port_id = -1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[3] = { .phy_port_id = LAN3_PORT, .ext_port_id = -1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[4] = { .phy_port_id = LAN4_PORT, .ext_port_id = -1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
#elif defined(RTAX59U) || defined(RTAX52)
		.count = NR_WANLAN_PORT,
		.port[0] = { .phy_port_id = WAN_PORT,  .ext_port_id = -1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[1] = { .phy_port_id = LAN1_PORT, .ext_port_id = -1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[2] = { .phy_port_id = LAN2_PORT, .ext_port_id = -1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[3] = { .phy_port_id = LAN3_PORT, .ext_port_id = -1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
#elif defined(CHEETAH)
		.count = NR_WANLAN_PORT,
		.port[0] = { .phy_port_id = WAN_PORT,  .ext_port_id = -1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[1] = { .phy_port_id = LAN1_PORT, .ext_port_id = -1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[2] = { .phy_port_id = LAN2_PORT, .ext_port_id = -1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[3] = { .phy_port_id = LAN3_PORT, .ext_port_id = -1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[4] = { .phy_port_id = LAN4_PORT, .ext_port_id = -1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
#elif defined(PRTAX57_GO)
		.count = NR_WANLAN_PORT,
		.port[0] = { .phy_port_id = WAN_PORT,  .ext_port_id = -1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
		.port[1] = { .phy_port_id = LAN1_PORT, .ext_port_id = -1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL, .flag = 0, .seq_no = -1, .ui_display = NULL },
#else
		#error "port_mapping is not defined."
#endif
	};
	if (!port_mapping)
		return;

///////////////// copy all Ethernet port here ////////////////////////
	memcpy(port_mapping, &port_mapping_static, sizeof(phy_port_mapping));

///////////////// finetune Ethernet port here ////////////////////////
#if defined(TUFAX4200)
	if (nvram_match("HwId", "A")) { // disable 2.5G LAN
		port_mapping->count = NR_WANLAN_PORT-1;
		port_mapping->port[port_mapping->count].phy_port_id = -1;
	}
#endif

///////////////// Assign Ethernet NIC name here ////////////////////////
	for (i = 0; i < port_mapping->count; i++) {
		id = port_mapping->port[i].phy_port_id;
		if (id != -1)
			port_mapping->port[i].ifname = (char *)vport_to_iface[id];
	}

///////////////// Add USB port define here ////////////////////////
#if defined(PANTHERB) || defined(TUFAX4200) || defined(TUFAX6000) || defined(CHEETAH) || defined(PRTAX57_GO)
////  1 USB3 port device
	i = port_mapping->count++;
	port_mapping->port[i].phy_port_id = -1;
	port_mapping->port[i].label_name = "U1";
	port_mapping->port[i].cap = PHY_PORT_CAP_USB;
	port_mapping->port[i].max_rate = 5000;
	port_mapping->port[i].ifname = NULL;
#elif defined(PANTHERA) || defined(RTAX59U)
////  1 USB3 + 1 USB2 port device
	i = port_mapping->count++;
	port_mapping->port[i].phy_port_id = -1;
	port_mapping->port[i].label_name = "U1";
	port_mapping->port[i].cap = PHY_PORT_CAP_USB;
	port_mapping->port[i].max_rate = 5000;
	port_mapping->port[i].ifname = NULL;
	i = port_mapping->count++;
	port_mapping->port[i].phy_port_id = -1;
	port_mapping->port[i].label_name = "U2";
	port_mapping->port[i].cap = PHY_PORT_CAP_USB;
	port_mapping->port[i].max_rate = 480;
	port_mapping->port[i].ifname = NULL;
#endif

	add_sw_cap(port_mapping);
	swap_wanlan(port_mapping);
	return;
}
#endif // end of RTCONFIG_NEW_PHYMAP
