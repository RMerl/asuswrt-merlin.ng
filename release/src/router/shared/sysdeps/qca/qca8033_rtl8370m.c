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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <bcmnvram.h>

#include <utils.h>
#include <shutils.h>
#include <shared.h>
#ifdef RTCONFIG_QCA
#include <net/if.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#endif

#define RTKSWITCH_DEV	"/dev/rtkswitch"

struct trafficCount_t
{
	long long rxByteCount;
	long long txByteCount;
};

int rtkswitch_ioctl(int val, int *val2)
{
	int fd;
	int value = 0;
	int *p = NULL, invalid_ioctl_cmd = 0;
	struct trafficCount_t portTraffic;

	fd = open(RTKSWITCH_DEV, O_RDONLY);
	if (fd < 0) {
		perror(RTKSWITCH_DEV);
		return -1;
	}

	switch (val) {
	case 0:		/* check WAN port phy status */
	case 3:		/* check LAN ports phy status */
	case 6:
	case 8:
	case 9:
	case 10:
	case 19:	/* Set WAN port txDelay */
	case 20:	/* Set WAN port rxDelay */
	case 22:
	case 23:
	case 24:
	case 25:
	case 36:	/* Set Vlan VID. */
	case 37:	/* Set Vlan PRIO. */
	case 38:	/* Initialize VLAN. Cherry Cho added in 2011/7/15. */
	case 39:	/* Create VLAN. Cherry Cho added in 2011/7/15. */
        case 40:
	case 44:	/* set WAN port */
	case 45:	/* set LAN trunk group */
	case 46:	/* set LAN trunk port mask */
	case 47:	/* set LAN hash algorithm */
	case 99:
	case 100:
	case 109:	/* Set specific ext port txDelay */
	case 110:	/* Set specific ext port rxDelay */
//#ifdef RTCONFIG_RTK_SWITCH_VLAN
	case 298:	/* set some ports tagged only */
	case 299:	/* set some ports untagged only */
	case 300:	/* set all ports accept tagged and untagged */
	case 301:	/* config vlan id */
	case 302:	/* config vlan prio */
	case 303:	/* config portlist and set to switch */
	case 304:	/* get vlan info */
	case 305:	/* set port pvid related[ VLAN id ] */
	case 306:	/* set port pvid related[ Priority ] */
	case 307:	/* set port pvid related[ Port Id and set to driver ] */
//#endif			
		p = val2;
		break;
	case 29:/* Set VoIP port. Cherry Cho added in 2011/6/30. */
		val = 34;	/* call ioctl 34 instead. */
		p = val2;
		break;
	case 2:		/* show state of RTL8367RB GMAC1 */
	case 7:
	case 11:
	case 14:	/* power up LAN port(s) */
	case 15:	/* power down LAN port(s) */
	case 16:	/* power up all ports */
	case 17:	/* power down all ports */
	case 21:
	case 27:
	case 41:	/* check realtek switch normal */
	case 90:	/* Reset SGMII link */
	case 91:	/* Read EXT port status */
	case 114:	/* power up WAN port(s) */
	case 115:	/* power down WAN port(s) */
		p = NULL;
		break;
	case 42:
	case 43:
		p = (int*) &portTraffic;
		break;

	default:
		invalid_ioctl_cmd = 1;
		break;
	}

	if (invalid_ioctl_cmd) {
		printf("wrong ioctl cmd: %d\n", val);
		close(fd);
		return 0;
	}

	if (ioctl(fd, val, p) < 0) {
		perror("rtkswitch ioctl");
		close(fd);
		return -1;
	}

	if (val == 0 || val == 3)
		printf("return: %x\n", value);
	else if(val == 42 || val == 43)
		printf("rx/tx: %lld/%lld\n", portTraffic.rxByteCount, portTraffic.txByteCount);

	close(fd);
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

	return rtkswitch_ioctl(val, &val2);
}

int is_wan_unit_in_switch(int wan_unit)
{
	char word[32], *next, wans_dualwan[128];
	int unit = 0;

	strlcpy(wans_dualwan, nvram_safe_get("wans_dualwan"), sizeof(wans_dualwan));
	foreach (word, wans_dualwan, next) {
		if(strcmp(word, "lan") == 0)
		{
			if(unit == wan_unit)
				return 1;
			break;
		}
		unit++;
	}
	return 0;
}

/**
 * @wan_unit:	wan_unit at run-time.
 * 		If dual-wan is disabled, wan1_ifname may be empty string.
 * 		If dual-wan is enabled, wan0_ifname and wan1_ifname may be LAN or USB modem.
 */
unsigned int rtkswitch_wanPort_phyStatus(int wan_unit)
{
	int sw_mode = sw_mode();
	unsigned int value;
	char prefix[16], tmp[100], *ifname = NULL;

	if (sw_mode == SW_MODE_REPEATER)
		return 0;

	switch (sw_mode) {
	case SW_MODE_ROUTER:
		if(is_wan_unit_in_switch(wan_unit))
		{
			value = 0;
			if (rtkswitch_ioctl(0, &value) < 0)
				return -1;
			return value;
		}
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
		ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		break;
	case SW_MODE_AP:
#if defined(BRTAC828) || defined(RTAD7200)
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
		/* BRT-AC828 SR1~SR3, REV 1.00 ~ 1.20 */
		ifname = (!wan_unit)? "eth2" : "eth3";
#else
		/* BRT-AC828 SR4 or above, REV 1.30+ */
		ifname = (!wan_unit)? "eth0" : "eth3";
#endif
#endif
		break;
	default:
		return 0;
	}

	if (!ifname || *ifname == '\0')
		return 0;
	value = !!(mdio_read(ifname, MII_BMSR) & BMSR_LSTATUS);

	return value;
}

unsigned int rtkswitch_lanPorts_phyStatus(void)
{
	unsigned int value = 0;
	if(rtkswitch_ioctl(3, &value) < 0)
		return -1;

	return !!value;

}

unsigned int __rtkswitch_WanPort_phySpeed(int wan_unit)
{
	int v, sw_mode = sw_mode();
	unsigned int value = 0;
	char prefix[16], tmp[100], *ifname;

	if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
		return 0;

	/* FIXME:
	 * if dual-wan is disabled, wan1_ifname may be empty string.
	 * if dual-wan is enabled, wan0_ifname and wan1_ifname may be LAN or USB modem.
	 */
	if(is_wan_unit_in_switch(wan_unit))
	{
		value = 0;
		if (rtkswitch_ioctl(13, &value) < 0)
			return -1;
		return value;
	}

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
	if (!ifname || *ifname == '\0')
		return 10;
	v = mdio_read(ifname, MII_BMCR);
	if (v & BMCR_SPEED1000)
		value = 1000;
	else if (v & BMCR_SPEED100)
		value = 100;
	else
		value = 10;

	return value;
}

unsigned int rtkswitch_WanPort_phySpeed(void)
{
	return __rtkswitch_WanPort_phySpeed(0);
}

static void phy_link_ctrl(char *ifname, int onoff)
{
	int fd, v;
	struct ifreq ifr;

	if (!ifname || *ifname == '\0')
		return;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return;

	strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		_dprintf("%s: ifname %s SIOCGMIIPHY failed.\n",
			__func__, ifname);
		close(fd);
		return;
	}
	v = mdio_read(ifname, MII_BMCR);
	if (onoff)
		v &= ~BMCR_PDOWN;
	else
		v |= BMCR_PDOWN;
	mdio_write(ifr.ifr_name, MII_BMCR, v);
	close(fd);
}

int rtkswitch_WanPort_linkUp(void)
{
	phy_link_ctrl(nvram_get("wan0_ifname"), 1);

	return 0;
}

int rtkswitch_WanPort_linkDown(void)
{
	phy_link_ctrl(nvram_get("wan0_ifname"), 0);

	return 0;
}

int rtkswitch_LanPort_linkUp(void)
{
	eval("rtkswitch", "14");

	return 0;
}

int rtkswitch_LanPort_linkDown(void)
{
	eval("rtkswitch", "15");

	return 0;
}

int rtkswitch_AllPort_linkUp(void)
{
	phy_link_ctrl(nvram_get("wan0_ifname"), 1);
	eval("rtkswitch", "16");

	return 0;
}

int rtkswitch_AllPort_linkDown(void)
{
	phy_link_ctrl(nvram_get("wan0_ifname"), 0);
	eval("rtkswitch", "17");

	return 0;
}

int rtkswitch_Reset_Storm_Control(void)
{
	eval("rtkswitch", "21");

	return 0;
}

typedef struct {
	unsigned int link[5];
	unsigned int speed[5];
} phyState;

int rtkswitch_AllPort_phyState(void)
{
	char buf[32];
	int porder_56u[5] = {4,3,2,1,0};
	int *o = porder_56u;
	const char *portMark = "W0=%C;L1=%C;L2=%C;L3=%C;L4=%C;";
	phyState pS;

	pS.link[0] = pS.link[1] = pS.link[2] = pS.link[3] = pS.link[4] = 0;
	pS.speed[0] = pS.speed[1] = pS.speed[2] = pS.speed[3] = pS.speed[4] = 0;

	if (rtkswitch_ioctl(18, (int*) &pS) < 0)
		return -1;

	if (get_model() == MODEL_RTN65U)
		portMark = "W0=%C;L4=%C;L3=%C;L2=%C;L1=%C;";

	snprintf(buf, sizeof(buf), portMark,
		(pS.link[o[0]] == 1) ? (pS.speed[o[0]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[1]] == 1) ? (pS.speed[o[1]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[2]] == 1) ? (pS.speed[o[2]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[3]] == 1) ? (pS.speed[o[3]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[4]] == 1) ? (pS.speed[o[4]] == 2) ? 'G' : 'M': 'X');

	puts(buf);

	return 0;
}

#if 0
void usage(char *cmd)
{
	printf("Usage: %s 1 - set (SCK, SD) as (0, 1)\n", cmd);
	printf("       %s 2 - set (SCK, SD) as (1, 0)\n", cmd);
	printf("       %s 3 - set (SCK, SD) as (1, 1)\n", cmd);
	printf("       %s 0 - set (SCK, SD) as (0, 0)\n", cmd);
	printf("       %s 4 - init vlan\n", cmd);
	printf("       %s 5 - set cpu port 0 0\n", cmd);
	printf("       %s 6 - set cpu port 0 1\n", cmd);
	printf("       %s 7 - set cpu port 1 0\n", cmd);
	printf("       %s 8 - set cpu port 1 1\n", cmd);
	printf("       %s 10 - set vlan entry, no cpu port, but mbr\n", cmd);
	printf("       %s 11 - set vlan entry, no cpu port, no mbr\n", cmd);
	printf("       %s 15 - set vlan PVID, no cpu port\n", cmd);
	printf("       %s 20 - set vlan entry, with cpu port\n", cmd);
	printf("       %s 21 - set vlan entry, with cpu port and no cpu port in untag sets\n", cmd);
	printf("       %s 25 - set vlan PVID, with cpu port\n", cmd);
	printf("       %s 26 - set vlan PVID, not set cpu port\n", cmd);
	printf("       %s 90 - accept all frmaes\n", cmd);
	printf("       %s 66 - setup default vlan\n", cmd);
	printf("       %s 61 - setup vlan type1\n", cmd);
	printf("       %s 62 - setup vlan type2\n", cmd);
	printf("       %s 63 - setup vlan type3\n", cmd);
	printf("       %s 64 - setup vlan type4\n", cmd);
	printf("       %s 65 - setup vlan type34\n", cmd);
	printf("       %s 70 - disable multicast snooping\n", cmd);
	printf("       %s 81 - setRtctTesting on port x\n", cmd);
	printf("       %s 82 - getRtctResult on port x\n", cmd);
	printf("       %s 83 - setGreenEthernet x(green, powsav)\n", cmd);
	printf("       %s 84 - setAsicGreenFeature x(txGreen, rxGreen)\n", cmd);
	printf("       %s 85 - getAsicGreenFeature\n", cmd);
	printf("       %s 86 - enable GreenEthernet on port x\n", cmd);
	printf("       %s 87 - disable GreenEthernet on port x\n", cmd);
	printf("       %s 88 - getAsicPowerSaving x\n", cmd);
	printf("       %s 50 - getPortLinkStatus x\n", cmd);
	exit(0);
}
#endif

void __pre_config_switch(void)
{
	int wanscap_wanlan = get_wans_dualwan() & (WANSCAP_WAN | WANSCAP_LAN);
	int wans_lanport = nvram_get_int("wans_lanport");
	int wan_mask, unit, type, vid;
	char cmd[64], vid_str[6];
	char prefix[8], nvram_ports[20], bif[IFNAMSIZ], vif[IFNAMSIZ];
	char *add_upst_viface[] = { "ip", "link", "add", "link", bif,
		"name", vif, "type", "vlan", "id", vid_str, NULL };
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
	char *wan_if[2] = { "eth2", "eth3" };
#elif defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	char *wan_if[2] = { "eth0", "eth3" };
#endif

	if (!is_routing_enabled())
		return;

	if ((wanscap_wanlan & WANSCAP_LAN) && (wans_lanport < 0 || wans_lanport > 8)) {
		_dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
		wanscap_wanlan &= ~WANSCAP_LAN;
	}

	if ((strlen(nvram_safe_get("switch_wantag")) > 0 && !nvram_match("switch_wantag", "none")) ||
	    (nvram_match("switch_wantag", "none") && (nvram_get_int("switch_stb_x") > 0 && nvram_get_int("switch_stb_x") <= 6))) {
		/* IPTV enabled */
		wan_mask = 0;
		if(wanscap_wanlan & WANSCAP_WAN)
			wan_mask |= 0x1 << 0;
		if(wanscap_wanlan & WANSCAP_LAN)
			wan_mask |= 0x1 << wans_lanport;
		snprintf(cmd, sizeof(cmd), "rtkswitch 44 0x%08x", wan_mask);
		system(cmd);

		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			snprintf(prefix, sizeof(prefix), "%d", unit);
			snprintf(nvram_ports, sizeof(nvram_ports), "wan%sports_mask", (unit == WAN_UNIT_FIRST)? "" : prefix);
			nvram_unset(nvram_ports);
			if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
				/* BRT-AC828 LAN1 = P0, LAN2 = P1, etc */
				nvram_set_int(nvram_ports, (1 << (wans_lanport - 1)));
			}
		}
	} else {
		/* IPTV disabled, if VLAN is enabled on WANx ports, create ethX.VLAN interfaces. */
		for (unit = 0; unit < 2; ++unit) {
			type = get_dualwan_by_unit(unit);
			if (type != WANS_DUALWAN_IF_WAN && type != WANS_DUALWAN_IF_WAN2)
				continue;
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if (!nvram_pf_match(prefix, "dot1q", "1"))
				continue;
			vid = nvram_pf_get_int(prefix, "vid");
			if (vid < 2 || vid > 4094)
				continue;
			strlcpy(bif, wan_if[unit], sizeof(bif));
			snprintf(vif, sizeof(vif), "%s.%d", bif, vid);
			snprintf(vid_str, sizeof(vid_str), "%d", vid);
			_eval(add_upst_viface, NULL, 0, NULL);
			eval("ifconfig", bif, "up");
		}
	}
}
