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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bcmnvram.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <shutils.h>
#include <shared.h>
#include <utils.h>
#include <qca.h>

#define NR_WANLAN_PORT	5
#define MAX_WANLAN_PORT	5

enum {
#if defined(RTAC58U) || defined(RTAC95U)
	CPU_PORT=0,
	LAN1_PORT=4,
	LAN2_PORT=3,
	LAN3_PORT=2,
	LAN4_PORT=1,
	WAN_PORT=5,
	P6_PORT=5,
#elif defined(RT4GAC53U)
	CPU_PORT=0,
	LAN1_PORT=3,
	LAN2_PORT=4,
	LAN3_PORT=2,	/* unused */
	LAN4_PORT=1,	/* unused */
	WAN_PORT=5,	/* unused */
	P6_PORT=5,
#elif defined(RTAC82U)
	CPU_PORT=0,
	LAN1_PORT=1,
	LAN2_PORT=2,
	LAN3_PORT=3,
	LAN4_PORT=4,
	WAN_PORT=5,
	P6_PORT=5,
#elif defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300)
	CPU_PORT=0,
	LAN1_PORT=1,
	LAN2_PORT=2,
	LAN3_PORT=3,
	LAN4_PORT=4,
	WAN_PORT=5,
	P6_PORT=5,
#else
#error Define WAN/LAN ports!
#endif

};

//0:WAN, 1:LAN, lan_wan_partition[][0] is port0
static const int lan_wan_partition[9][NR_WANLAN_PORT] = {
#if defined(RTAC95U)
	/* W, L1, L2, L3, X */
	{0,1,1,1,1}, //WLLLL
	{0,0,1,1,1}, //WWLLL
	{0,1,0,1,1}, //WLWLL
	{0,1,1,0,1}, //WLLWL
	{0,1,1,0,1}, //WLLWL
	{0,0,0,1,1}, //WWWLL
	{0,1,0,0,1}, //WLWWL
	{1,1,1,1,1}  //ALL
#else
	/* W, L1, L2, L3, L4 */
	{0,1,1,1,1}, //WLLLL
	{0,0,1,1,1}, //WWLLL
	{0,1,0,1,1}, //WLWLL
	{0,1,1,0,1}, //WLLWL
	{0,1,1,1,0}, //WLLLW
	{0,0,0,1,1}, //WWWLL
	{0,1,1,0,0}, //WLLWW
	{1,1,1,1,1}  //ALL
#endif
};

#define	CPU_PORT_WAN_MASK	(1U << CPU_PORT)
#define CPU_PORT_LAN_MASK	(1U << CPU_PORT)

#define WANPORTS_MASK	((1U << WAN_PORT))	/* ALL WAN port bit-mask */
#define LANPORTS_MASK	((1U << LAN1_PORT) | (1U << LAN2_PORT) | (1U << LAN3_PORT) | (1U << LAN4_PORT))	/* ALL LAN port bit-mask */
#define WANLANPORTS_MASK	(WANPORTS_MASK | LANPORTS_MASK)	/* ALL WAN/LAN port bit-mask */

/* Final model-specific LAN/WAN/WANS_LAN partition definitions.
 * bit0: P0, bit1: P1, bit2: P2, bit3: P3, bit4: P4, bit5: P5
 * ^^^^^^^^ P0 is not used by LAN/WAN port.
 */
static unsigned int lan_mask = 0;	/* LAN only. Exclude WAN, WANS_LAN, and generic IPTV port. */
static unsigned int wan_mask = 0;	/* wan_type = WANS_DUALWAN_IF_WAN. Include generic IPTV port. */
static unsigned int wans_lan_mask = 0;	/* wan_type = WANS_DUALWAN_IF_LAN. */

/* RT-N56U's P0, P1, P2, P3, P4 = LAN4, LAN3, LAN2, LAN1, WAN
 * ==> Model-specific port number.
 */
static int switch_port_mapping[] = {
#if defined(RTAC95U)
	LAN3_PORT,	//0000 0000 0001 LAN4 (convert to LAN3)
	LAN2_PORT,	//0000 0000 0010 LAN3 (convert to LAN2)
	LAN2_PORT,	//0000 0000 0100 LAN2
	LAN1_PORT,	//0000 0000 1000 LAN1
	WAN_PORT,	//0000 0001 0000 WAN
	P6_PORT,	//0000 0010 0000 -
	P6_PORT,	//0000 0100 0000 -
	P6_PORT,	//0000 1000 0000 -
	P6_PORT,	//0001 0000 0000 -
	CPU_PORT,	//0010 0000 0000 CPU port
#else
	LAN4_PORT,	//0000 0000 0001 LAN4
	LAN3_PORT,	//0000 0000 0010 LAN3
	LAN2_PORT,	//0000 0000 0100 LAN2
	LAN1_PORT,	//0000 0000 1000 LAN1
	WAN_PORT,	//0000 0001 0000 WAN
	P6_PORT,	//0000 0010 0000 -
	P6_PORT,	//0000 0100 0000 -
	P6_PORT,	//0000 1000 0000 -
	P6_PORT,	//0001 0000 0000 -
	CPU_PORT,	//0010 0000 0000 CPU port
#endif
};

/* Model-specific LANx ==> Model-specific PortX mapping */
const int lan_id_to_port_mapping[NR_WANLAN_PORT] = {
	WAN_PORT,
	LAN1_PORT,
	LAN2_PORT,
	LAN3_PORT,
	LAN4_PORT,
};

#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) /* for Lyra */
/* this table is mapping to lan_id_to_port_mapping */
static const int skip_ports[NR_WANLAN_PORT] = {
	0,  /* WAN_PORT */
	1,
	1,
	1,
	0,  /* LAN4_PORT */
};

int is_skip_port(int port)
{
	int i;
	for( i = 0; i < NR_WANLAN_PORT; i++) {
		if(lan_id_to_port_mapping[i] == port)
			return skip_ports[i];
	}
	return 1;
}
#else
static const int skip_ports[NR_WANLAN_PORT] = { 0 };
#define is_skip_port(p) (0)
#endif

void reset_qca_switch(void);

/* Model-specific LANx ==> Model-specific PortX */
static inline int lan_id_to_port_nr(int id)
{
	return lan_id_to_port_mapping[id];
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

#if !defined(RTCONFIG_AMAS)
	if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
		return 0;
#endif

	if (wan_unit <= 0 || wan_unit >= WAN_UNIT_MAX)
		strcpy(nv, "wanports_mask");
	else
		sprintf(nv, "wan%dports_mask", wan_unit);

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
		m = WANLANPORTS_MASK;

	return m;
}

int ipq40xx_subvlan_set(int vid, int prio, int mbr)
{
	int i;
	unsigned int m;

	if (vid > 4095 || prio > 7)
		return -1;

	doSystem("ssdk_sh vlan entry create %d", vid);
	for (i = 0, m = mbr; i < 6; ++i, m >>= 1) {
		if (m & 1)
			doSystem("ssdk_sh vlan member add %d %d unmodified", vid, i);
	}

	return 0;
}

/**
 * Set a VLAN
 * @vid:	VLAN ID
 * @prio:	VLAN Priority
 * @mbr:	VLAN members
 * @untag:	VLAN members that need untag
 *
 * @return
 * 	0:	success
 *     -1:	invalid parameter
 */
int ipq40xx_vlan_set(int vid, int prio, int mbr, int untag)
{
	int i;
	unsigned int m, u;

	if (vid > 4095 || prio > 7)
		return -1;

	doSystem("ssdk_sh vlan entry create %d", vid);
	for (i = 0, m = mbr, u = untag; i < 6; ++i, m >>= 1, u >>=1) {
		if (m & 1) {
			if (u & 1) {
				doSystem("ssdk_sh vlan member add %d %d untagged", vid, i);
				doSystem("ssdk_sh portVlan defaultCVid set %d %d", i, vid);
				doSystem("ssdk_sh qos ptDefaultCpri set %d %d", i, prio);
			}
			else {
				doSystem("ssdk_sh vlan member add %d %d tagged", vid, i);
			}
			doSystem("ssdk_sh portVlan ingress set %d secure", i);
			if (nvram_match("wifison_ready", "1"))
			{
#ifdef RTCONFIG_WIFI_SON
				doSystem("ssdk_sh portVlan egress set %d unmodified", i);
#else
				 _dprintf("no wifison feature\n");
#endif
			}
			else
				doSystem("ssdk_sh portVlan egress set %d untagged", i);
		}
	}

	return 0;
}

void ipq40xx_vlan_unset(int vid)
{
	if(vid > 0 && vid < 4096) {
		doSystem("ssdk_sh vlan entry del %d", vid);
	}
}

void vlan_remove(int vid)
{
	ipq40xx_vlan_unset(vid);
}

void ipq40xx_portVlan_accept(int accept, int port)
{
	const char *method;

	if(port <0 || port > 5)
		return;

	if(is_skip_port(port))
		return;

	if(accept)
		method = "fallback";
	else
		method = "secure";

	doSystem("ssdk_sh portVlan ingress set %d %s", port, method);
}

void vlan_accept_vid_via_switch(int accept, int wan, int lan)
{
	unsigned int mask = 0;
	int i;
	if(wan)
		mask |= WANPORTS_MASK;
	if(lan)
		mask |= LANPORTS_MASK;

	for(i = 0; mask; mask >>= 1, i++) {
		if(mask & 1) {
			ipq40xx_portVlan_accept(accept, i);
		}
	}
}


/**
 * Get link status and/or phy speed of a port.
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
static int get_ipq40xx_port_info(unsigned int port, unsigned int *link, unsigned int *speed)
{
        FILE *fp;
	size_t rlen;
	unsigned int l = 0, s = 0;
	char buf[512], *pt;

	if (port < 1 || port > 5 || (!link && !speed))
		return -1;

	if (link)
		*link = 0;
	if (speed)
		*speed = 0;

	sprintf(buf, "ssdk_sh port linkstatus get %d", port);
	if ((fp = popen(buf, "r")) == NULL) {
		_dprintf("%s: Run [%s] fail!\n", __func__, buf);
		return -2;
	}
	rlen = fread(buf, 1, sizeof(buf), fp);
	pclose(fp);
	if (rlen <= 1)
		return -3;

	buf[rlen-1] = '\0';
	if ((pt = strstr(buf, "[Status]:")) == NULL)
	{
#if defined(RTAC82U) //workaround for MALIBU
		if(link!=NULL)
			*link=1;  //linak up
		if(speed!=NULL)
			*speed=2; //1000M
		return 0;
#endif
		return -4;
	}
	pt += 9; // strlen of "[Status]:"
	if (!strncmp(pt, "ENABLE", 6)) {
		l = 1;

		sprintf(buf, "ssdk_sh port speed get %d", port);
		if ((fp = popen(buf, "r")) == NULL) {
			_dprintf("%s: Run [%s] fail!\n", __func__, buf);
			return -5;
		}
		rlen = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if (rlen <= 1)
			return -6;

		buf[rlen-1] = '\0';
		if ((pt = strstr(buf, "[speed]:")) == NULL)
			return -7;

		pt += 8; // strlen of "[speed]:"
		if (!strncmp(pt, "1000", 4))
			s = 2;
		else
			s = 1;
	}

	if (link)
		*link = l;
	if (speed)
		*speed = s;

	return 0;
}

/**
 * Get linkstatus in accordance with port bit-mask.
 * @mask:	port bit-mask.
 * 		bit0 = P0, bit1 = P1, etc.
 * @linkStatus:	link status of all ports that is defined by mask.
 * 		If one port of mask is linked-up, linkStatus is true.
 */
static void get_ipq40xx_phy_linkStatus(unsigned int mask, unsigned int *linkStatus)
{
	int i;
	unsigned int value = 0, m;

	m = mask & WANLANPORTS_MASK;
	for (i = 0; m > 0 && !value; ++i, m >>= 1) {
		if (!(m & 1))
			continue;

		get_ipq40xx_port_info(i, &value, NULL);
		value &= 0x1;
	}
	*linkStatus = value;
}

static void build_wan_lan_mask(int stb)
{
	int i, unit;
	int wanscap_lan = get_wans_dualwan() & WANSCAP_LAN;
	int wans_lanport = nvram_get_int("wans_lanport");
	int sw_mode = nvram_get_int("sw_mode");
	char prefix[8], nvram_ports[20];

	if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
		wanscap_lan = 0;

#ifdef RTCONFIG_ETHBACKHAUL
	f_write_string("/proc/sys/net/edma/merge_wan_into_lan", "0", 0, 0);
#else
	if (stb == 100 && (sw_mode == SW_MODE_AP || __mediabridge_mode(sw_mode))) {
		stb = 7;	/* Don't create WAN port. */
		f_write_string("/proc/sys/net/edma/merge_wan_into_lan", "1", 0, 0);
	}
	else
		f_write_string("/proc/sys/net/edma/merge_wan_into_lan", "0", 0, 0);
#endif
#if defined(MAPAC1300) /* Lyra mini WAN/LAN port exchanged. */
	if (!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", "")) {
		if (stb==4 || stb==6) // All WAN.
			stb = 7;
		else if (stb == 7) // All LAN.
			stb = 4;
	}
#endif

#if 0	/* TODO: no WAN port */
	if ((get_wans_dualwan() & (WANSCAP_LAN | WANSCAP_WAN)) == 0)
		stb = 7; // no WAN?
#endif

	if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 4)) {
		_dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
		wanscap_lan = 0;
	}

	lan_mask = wan_mask = wans_lan_mask = 0;
	for (i = 0; i < NR_WANLAN_PORT; ++i) {
		if (skip_ports[i]) continue;
		switch (lan_wan_partition[stb][i]) {
		case 0:
			wan_mask |= 1U << lan_id_to_port_nr(i);
			break;
		case 1:
			lan_mask |= 1U << lan_id_to_port_nr(i);
			break;
		default:
			_dprintf("%s: Unknown LAN/WAN port definition. (stb %d i %d val %d)\n",
				__func__, stb, i, lan_wan_partition[stb][i]);
		}
	}

	//DUALWAN
	if (wanscap_lan) {
		wans_lan_mask = 1U << lan_id_to_port_nr(wans_lanport);
		lan_mask &= ~wans_lan_mask;
	}

	if(!nvram_match("wifison_ready", "1")) {
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			sprintf(prefix, "%d", unit);
			sprintf(nvram_ports, "wan%sports_mask", (unit == WAN_UNIT_FIRST)?"":prefix);

			if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
				nvram_set_int(nvram_ports, wan_mask);
			}
			else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
				nvram_set_int(nvram_ports, wans_lan_mask);
			}
			else
				nvram_unset(nvram_ports);
		}
		nvram_set_int("lanports_mask", lan_mask);
#if defined(MAPAC1300) /* Lyra mini fixed the WAN / LAN port mask. */
		nvram_set_int(nvram_ports, lan_mask);
		nvram_set_int("lanports_mask", wan_mask);
#endif
	}
}

static void edma_group_mask_to_bmp(int groupid, unsigned int mask)
{
        char path[50], buf[10];

        snprintf(path, sizeof(path)-1, "/proc/sys/net/edma/default_group%d_bmp", groupid);
	snprintf(buf, sizeof(buf)-1, "%d", mask);
	f_write_string(path, buf, 0, 0);
}

#ifdef RTCONFIG_ETHBACKHAUL
#define	ISOLATED_VLAN_OFFSET	10
static unsigned int edma_default_tag(int groupid)
{
        char *path, value[10];
	if (groupid==1) /* eth0 */
	        path="/proc/sys/net/edma/default_wan_tag";
	else if (groupid==2) /* eth1 */
	        path="/proc/sys/net/edma/default_lan_tag";
	else {
		_dprintf("%s: BUG!!! invalid groupid[%d]\n", __func__, groupid);
		return 0;
	}
        f_read_string(path, value, sizeof(value));
        return atoi(value);
}

static unsigned int edma_group_bmp_to_mask(int groupid)
{
        char path[50], value[10];

        snprintf(path, sizeof(path)-1, "/proc/sys/net/edma/default_group%d_bmp", groupid);
        f_read_string(path, value, sizeof(value));
        return atoi(value);
}

/* mask 0: automatic, else isolated the specific port */
unsigned int isolated_vlan_create(unsigned int mask, char *lan_nic)
{
	int i;
	unsigned int isolated_vlan_id, port_mask, all_mask, inv_mask;
	unsigned char buf[30];
	unsigned int group1_tag, group2_tag, wan_tag;

	/* keep compatibility with IPTV & Auto WAN/LAN */
	group1_tag = edma_default_tag(1);  /* eth0 */
	group2_tag = edma_default_tag(2);  /* eth1 */

	all_mask = inv_mask = 0;
	/* create isolated vlan */
	system("ifconfig eth2 up 0.0.0.0");

	if (!strcmp(lan_nic, "eth1"))
		wan_tag = group1_tag;
	else
		wan_tag = group2_tag;

	for ( i=0; i< NR_WANLAN_PORT; i++ ) {
		if ( skip_ports[i] ) continue;
		port_mask = 1 << lan_id_to_port_mapping[i];
		if ( mask && !(port_mask & mask)) {
			inv_mask |= port_mask;
			continue;
		}
		isolated_vlan_id = ISOLATED_VLAN_OFFSET + lan_id_to_port_mapping[i]; /* same number as QCA's design */
		doSystem("ssdk_sh vlan entry del %d", isolated_vlan_id);
		all_mask |= port_mask;
		ipq40xx_vlan_set(isolated_vlan_id, 0, port_mask | CPU_PORT_LAN_MASK, port_mask);
		snprintf(buf, sizeof(buf)-1, "%d", isolated_vlan_id);
		eval("vconfig", "add", "eth2", buf); /* set all isolated port to lan */
		doSystem("ssdk_sh vlan member del %d %d", wan_tag, lan_id_to_port_mapping[i]);
	}

	if (!strcmp(lan_nic, "eth1")) {
		edma_group_mask_to_bmp(1, inv_mask | CPU_PORT_WAN_MASK);
		edma_group_mask_to_bmp(2, CPU_PORT_LAN_MASK);
		edma_group_mask_to_bmp(3, all_mask | CPU_PORT_LAN_MASK); /* set all isolated port to eth2 */
		ipq40xx_vlan_set(group2_tag, 0, CPU_PORT_LAN_MASK, 0); /* for eth1 */
	} else if (!strcmp(lan_nic, "eth0")) {
		edma_group_mask_to_bmp(1, CPU_PORT_WAN_MASK);
		edma_group_mask_to_bmp(2, inv_mask | CPU_PORT_LAN_MASK); 
		edma_group_mask_to_bmp(3, all_mask | CPU_PORT_LAN_MASK); /* set all isolated port to eth2 */
		ipq40xx_vlan_set(group1_tag, 0, CPU_PORT_WAN_MASK, 0); /* for eth0 */
	} else
		_dprintf("%s: BUG!!! invalid lan nic name [%s]\n", __func__, lan_nic);

	/* for vlan-based guest network */
	ipq40xx_subvlan_set(55, 0, all_mask | CPU_PORT_LAN_MASK);

	return all_mask;
}

unsigned int get_all_portmask(void)
{
	int i;
	unsigned int port_mask, all_mask;
	all_mask = 0;
	for ( i=0; i< NR_WANLAN_PORT; i++ ) {
		if ( skip_ports[i] ) continue;
		port_mask = 1 << lan_id_to_port_mapping[i];
		all_mask |= port_mask;
	}
	return all_mask;
}

unsigned int get_portlink_bymask(unsigned int portmask)
{
	int i;
	unsigned int value = 0, m, orbit, islink;

	m = portmask & WANLANPORTS_MASK;
	orbit = 1;
	for (i = 0; m > 0 ; ++i, m >>= 1, orbit <<= 1) {
		if (!(m & 1))
			continue;

		get_ipq40xx_port_info(i, &islink, NULL);
		if (islink)
			value |= orbit;
	}
	return value;
}

void power_onoff_port(int portno, int state)
{
	doSystem("ssdk_sh port %s set %d", state==1?"poweron":"poweroff", portno);
}

void move_port_to(int portno, char *nic)
{
	unsigned int group1_mask = edma_group_bmp_to_mask(1); /* eth0 */
	unsigned int group2_mask = edma_group_bmp_to_mask(2); /* eth1 */
	unsigned int group3_mask = edma_group_bmp_to_mask(3); /* eth2 */
	unsigned int portmask;
	int group, to_vlan;

	if(strcmp(nic,"eth0")==0)
		group=1;
	else if(strcmp(nic,"eth1")==0)
		group=2;
	else {
		_dprintf("%s: Invalid nic:%s\n", __func__, nic);
		return;
	}

	if (( portno > MAX_WANLAN_PORT ) || ( portno == 0)) {
		_dprintf("%s: Invalid portno:%d\n", __func__, portno);
		return; 
	}
	portmask = 1 << portno;

	if (group3_mask & portmask)
		edma_group_mask_to_bmp(3 , group3_mask & ~portmask);

	if ( group == 1 ) { /* move to group1, eth0 */
		to_vlan = edma_default_tag(1);  /* eth0 */

		if (group2_mask & portmask)
			edma_group_mask_to_bmp(2 , group2_mask & ~portmask);
		if (!(group1_mask & portmask)) /* check if isolated port is in group1 */
			edma_group_mask_to_bmp(1 , group1_mask | portmask);
	} else if ( group == 2 ) { /* move to group2, eth1 */
		to_vlan = edma_default_tag(2);  /* eth1 */

		if (group1_mask & portmask)
			edma_group_mask_to_bmp(1 , group1_mask & ~portmask);
		if (!(group2_mask & portmask)) /* check if isolated port is in group2 */
			edma_group_mask_to_bmp(2 , group2_mask | portmask);
	}

	doSystem("ssdk_sh vlan member add %d %d untagged", to_vlan, portno);
	doSystem("ssdk_sh portVlan defaultCVid set %d %d", portno, to_vlan);
	/* remove from isolated VLAN */
	doSystem("ssdk_sh vlan member del %d %d", portno + ISOLATED_VLAN_OFFSET, portno);
}

void isolate_port(int portno)
{
	unsigned int group1_mask = edma_group_bmp_to_mask(1); /* eth0 */
	unsigned int group2_mask = edma_group_bmp_to_mask(2); /* eth1 */
	unsigned int group3_mask = edma_group_bmp_to_mask(3); /* eth2 */
	unsigned int portmask;
	int from_vlan=0;

	if (( portno > MAX_WANLAN_PORT ) || ( portno == 0)) {
		_dprintf("%s: Invalid portno:%d\n", __func__, portno);
		return; 
	}

	portmask = 1 << portno;

	if (group2_mask & portmask) {
		from_vlan = edma_default_tag(2);  /* eth1 */
		edma_group_mask_to_bmp(2 , group2_mask & ~portmask);
	}
	else if (group1_mask & portmask) {
		from_vlan = edma_default_tag(1); /* eth0 */
		edma_group_mask_to_bmp(1 , group1_mask & ~portmask);
	}

	edma_group_mask_to_bmp(3 , group3_mask | portmask);

	if (from_vlan)
		doSystem("ssdk_sh vlan member del %d %d", from_vlan, portno);
	/* add to isolated VLAN */
	doSystem("ssdk_sh vlan member add %d %d untagged", portno + ISOLATED_VLAN_OFFSET, portno);
	doSystem("ssdk_sh portVlan defaultCVid set %d %d", portno, portno + ISOLATED_VLAN_OFFSET);
}
#endif

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
static void config_ipq40xx_LANWANPartition(int type)
{
	int wanscap_wanlan = get_wans_dualwan() & (WANSCAP_WAN | WANSCAP_LAN);
	int wanscap_lan = wanscap_wanlan & WANSCAP_LAN;
	int wans_lanport = nvram_get_int("wans_lanport");
	int sw_mode = nvram_get_int("sw_mode");

	if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
		wanscap_lan = 0;

	if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 4)) {
		_dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
		wanscap_lan = 0;
		wanscap_wanlan &= ~WANSCAP_LAN;
	}

	build_wan_lan_mask(type);
	_dprintf("%s: LAN/WAN/WANS_LAN portmask %08x/%08x/%08x\n", __func__, lan_mask, wan_mask, wans_lan_mask);
	reset_qca_switch();

#ifdef RTCONFIG_DETWAN
	if(nvram_match("wifison_ready", "1")) {
		if(lan_mask == 0 && nvram_match("detwan_phy", "eth1"))
		{
			int mask = lan_mask;
			lan_mask = wan_mask;
			wan_mask = mask;
		}
	}
#endif	/* RTCONFIG_DETWAN */

	// LAN 
	if(lan_mask)
	ipq40xx_vlan_set(1, 0, (lan_mask | CPU_PORT_LAN_MASK), lan_mask);
	edma_group_mask_to_bmp(2, lan_mask);

	// WAN & DUALWAN
	{
		int vlan = 2;
		if (wan_mask)
			ipq40xx_vlan_set(vlan++, 0, (wan_mask      | CPU_PORT_WAN_MASK), wan_mask);
		if (wans_lan_mask)
			ipq40xx_vlan_set(vlan++, 0, (wans_lan_mask | CPU_PORT_WAN_MASK), wans_lan_mask);
		edma_group_mask_to_bmp(1, wan_mask);
	}
}

static void get_ipq40xx_Port_Speed(unsigned int port_mask, unsigned int *speed)
{
	int i, v = -1, t;
	unsigned int m;

	if(speed == NULL)
		return;

	m = port_mask & WANLANPORTS_MASK;
	for (i = 0; m; ++i, m >>= 1) {
		if (!(m & 1))
			continue;

		get_ipq40xx_port_info(i, NULL, (unsigned int*) &t);
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
		*speed = 0;
		_dprintf("%s: invalid speed!\n", __func__);
	}
}

static void get_ipq40xx_WAN_Speed(unsigned int *speed)
{
	get_ipq40xx_Port_Speed(get_wan_port_mask(0) | get_wan_port_mask(1), speed);
}

static void link_down_up_ipq40xx_PHY(unsigned int mask, int status)
{
	int i;
	char idx[2], action[9];
	unsigned int m;

	if (!status)		//power down PHY
		sprintf(action, "poweroff");
	else
		sprintf(action, "poweron");

	mask &= WANLANPORTS_MASK;
	for (i = 0, m = mask; m; ++i, m >>= 1) {
		if (!(m & 1))
			continue;
		sprintf(idx, "%d", i);
		eval("ssdk_sh", "port", action, "set", idx);
	}
}

void set_ipq40xx_broadcast_rate(int bsr)
{
#if 0
	if ((bsr < 0) || (bsr > 255))
		return;

	if (switch_init() < 0)
		return;

	printf("set broadcast strom control rate as: %d\n", bsr);
	switch_fini();
#endif
}

void reset_qca_switch(void)
{
	int i;
	for(i=0; i<6; i++) {
		doSystem("ssdk_sh portVlan defaultCVid set %d 0", i);
	}
	eval("ssdk_sh", "vlan", "entry", "flush"); // clear
	nvram_unset("vlan_idx");

	/* TX interrupt infinity */
	f_write_string("/proc/irq/97/smp_affinity", "4", 0, 0);
	f_write_string("/proc/irq/98/smp_affinity", "4", 0, 0);
	f_write_string("/proc/irq/99/smp_affinity", "4", 0, 0);
	f_write_string("/proc/irq/100/smp_affinity", "4", 0, 0);
	f_write_string("/proc/irq/101/smp_affinity", "8", 0, 0);
	f_write_string("/proc/irq/102/smp_affinity", "8", 0, 0);
	f_write_string("/proc/irq/103/smp_affinity", "8", 0, 0);
	f_write_string("/proc/irq/104/smp_affinity", "8", 0, 0);
	f_write_string("/proc/irq/105/smp_affinity", "1", 0, 0);
	f_write_string("/proc/irq/106/smp_affinity", "1", 0, 0);
	f_write_string("/proc/irq/107/smp_affinity", "1", 0, 0);
	f_write_string("/proc/irq/108/smp_affinity", "1", 0, 0);
	f_write_string("/proc/irq/109/smp_affinity", "2", 0, 0);
	f_write_string("/proc/irq/110/smp_affinity", "2", 0, 0);
	f_write_string("/proc/irq/111/smp_affinity", "2", 0, 0);
	f_write_string("/proc/irq/112/smp_affinity", "2", 0, 0);
	/* RX interrupt infinity */
	f_write_string("/proc/irq/272/smp_affinity", "1", 0, 0);
	f_write_string("/proc/irq/274/smp_affinity", "2", 0, 0);
	f_write_string("/proc/irq/276/smp_affinity", "4", 0, 0);
	f_write_string("/proc/irq/278/smp_affinity", "8", 0, 0);
	f_write_string("/proc/irq/273/smp_affinity", "1", 0, 0);
	f_write_string("/proc/irq/275/smp_affinity", "2", 0, 0);
	f_write_string("/proc/irq/277/smp_affinity", "4", 0, 0);
	f_write_string("/proc/irq/279/smp_affinity", "8", 0, 0);
	/* XPS/RFS configuration */
	f_write_string("/sys/class/net/eth0/queues/tx-0/xps_cpus", "1", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/tx-1/xps_cpus", "2", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/tx-2/xps_cpus", "4", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/tx-3/xps_cpus", "8", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-0/rps_cpus", "1", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-1/rps_cpus", "2", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-2/rps_cpus", "4", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-3/rps_cpus", "8", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/tx-0/xps_cpus", "1", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/tx-1/xps_cpus", "2", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/tx-2/xps_cpus", "4", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/tx-3/xps_cpus", "8", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-0/rps_cpus", "1", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-1/rps_cpus", "2", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-2/rps_cpus", "4", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-3/rps_cpus", "8", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-0/rps_flow_cnt", "256", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-1/rps_flow_cnt", "256", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-2/rps_flow_cnt", "256", 0, 0);
	f_write_string("/sys/class/net/eth0/queues/rx-3/rps_flow_cnt", "256", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-0/rps_flow_cnt", "256", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-1/rps_flow_cnt", "256", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-2/rps_flow_cnt", "256", 0, 0);
	f_write_string("/sys/class/net/eth1/queues/rx-3/rps_flow_cnt", "256", 0, 0);
	f_write_string("/proc/sys/net/core/rps_sock_flow_entries", "1024", 0, 0);
#if defined(VZWAC1300)
	eval("ssdk_sh", "debug", "phy", "set", "5", "0xB", "0x000a"); // decrease PHY NOISE, Arcadyan request
#endif
}

static void set_Vlan_VID(int vid)
{
	char tmp[8];

	sprintf(tmp, "%d", vid);
	nvram_set("vlan_vid", tmp);
}

static void set_Vlan_PRIO(int prio)
{
	char tmp[2];

	sprintf(tmp, "%d", prio);
	nvram_set("vlan_prio", tmp);
}

static int convert_n56u_to_qca_bitmask(int orig)
{
	int i, bit, bitmask;
	bitmask = 0;
	for(i = 0; i < ARRAY_SIZE(switch_port_mapping); i++) {
		bit = (1 << i);
		if (orig & bit)
			bitmask |= (1 << switch_port_mapping[i]);
	}
#if defined(RTCONFIG_PORT2_DEVICE)
#if defined(RTCONFIG_DETWAN)
	if(nvram_match("wifison_ready", "1")) {
		if(nvram_safe_get("detwan_phy")[0] != '\0')
		{
			int mask = 0;
			//iptv (VoIP & STB) use LAN3/LAN4 but only LAN1 OR WAN to be real port in LYRA
			if((bitmask & (1 << LAN4_PORT)) || (bitmask & (1 << LAN3_PORT))) {
				bitmask &= ~(1 << LAN4_PORT);
				bitmask &= ~(1 << LAN3_PORT);
				mask |= nvram_get_int("detwan_lan_mask");
			}
			if(bitmask & (1<<WAN_PORT)) {
				bitmask &= ~(1 << WAN_PORT);
				mask |= nvram_get_int("detwan_wan_mask");
			}
			bitmask |= mask;
		}
	}
	else
#endif	/* RTCONFIG_DETWAN */
	{
		int mask = 0;
		//iptv (VoIP & STB) use LAN3/LAN4 but only LAN1 OR WAN to be real port in LYRA
		if((bitmask & (1 << LAN4_PORT)) || (bitmask & (1 << LAN3_PORT))) {
			bitmask &= ~(1 << LAN4_PORT);
			bitmask &= ~(1 << LAN3_PORT);
			mask |= (1 << LAN4_PORT);
		}
		bitmask |= mask;
	}
#endif	/* RTCONFIG_PORT2_DEVICE */

	return bitmask;
}

/**
 * @stb_bitmask:	bitmask of STB port(s)
 * 			e.g. bit0 = P0, bit1 = P1, etc.
 */
static void initialize_Vlan(int stb_bitmask)
{
	int wans_lan_vid = 3, wanscap_wanlan = get_wans_dualwan() & (WANSCAP_WAN | WANSCAP_LAN);
	int wanscap_lan = get_wans_dualwan() & WANSCAP_LAN;
	int wans_lanport = nvram_get_int("wans_lanport");
	int sw_mode = nvram_get_int("sw_mode");

	if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
		wanscap_lan = 0;

	if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 4)) {
		_dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
		wanscap_lan = 0;
		wanscap_wanlan &= ~WANSCAP_LAN;
	}

	build_wan_lan_mask(0);
	stb_bitmask = convert_n56u_to_qca_bitmask(stb_bitmask);
#if defined(RTCONFIG_DETWAN)
	if(nvram_match("wifison_ready", "1")) {
		lan_mask = get_lan_port_mask();
		wan_mask = get_wan_port_mask(0);
	}
#endif
	lan_mask &= ~stb_bitmask;
	wan_mask |= stb_bitmask;
	_dprintf("%s: LAN/WAN/WANS_LAN portmask %08x/%08x/%08x\n", __func__, lan_mask, wan_mask, wans_lan_mask);

	if(wans_lan_mask & stb_bitmask)	// avoid using a port for two functions.
	{
		wanscap_lan = 0;
		wanscap_wanlan &= ~WANSCAP_LAN;
	}

	if (wanscap_lan && (!(get_wans_dualwan() & WANSCAP_WAN)) && !(!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", "")))
	{
		wans_lan_vid = 2;
	}

	reset_qca_switch();

	// LAN
	if(lan_mask)
	ipq40xx_vlan_set(1, 0, (lan_mask | CPU_PORT_LAN_MASK), lan_mask);

	// DUALWAN
	if (wanscap_lan) {
		ipq40xx_vlan_set(wans_lan_vid, 0, (wans_lan_mask | CPU_PORT_WAN_MASK), wans_lan_mask);
	}
}


static int wanmii_need_vlan_tag(void)
{
	int dualwan = get_wans_dualwan();

	if(!(dualwan & WANSCAP_WAN))								// none or one port for WAN
		return 0;
	if(dualwan & WANSCAP_LAN)								// dual port for WAN
		return 1;
	if(!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", ""))		// has IPTV ISP setting (which has vlan id)
		return 1;

	return 0;
}

static int handle_wanmii_untag(int mask)
{
	if((mask & CPU_PORT_WAN_MASK) && wanmii_need_vlan_tag())
		mask &= ~CPU_PORT_WAN_MASK;

	return mask;
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
	int vid = safe_atoi(nvram_safe_get("vlan_vid")) & 0xFFF;
	int prio = safe_atoi(nvram_safe_get("vlan_prio")) & 0x7;
	int mbr = bitmask & 0xffff;
	int untag = (bitmask >> 16) & 0xffff;
	int mbr_qca, untag_qca;

	//convert port mapping
	mbr_qca   = convert_n56u_to_qca_bitmask(mbr);
	untag_qca = convert_n56u_to_qca_bitmask(untag);
	untag_qca = handle_wanmii_untag(untag_qca);

	ipq40xx_vlan_set(vid, prio, mbr_qca, untag_qca);
}

#if 0
static void is_singtel_mio(int is)
{
}
#endif

int ipq40xx_ioctl(int val, int val2)
{
//	int value = 0;
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
		config_ipq40xx_LANWANPartition(val2);
		break;
	case 13:
		get_ipq40xx_WAN_Speed(&value2);
		printf("WAN speed : %u Mbps\n", value2);
		break;
	case 14: // Link up LAN ports
		link_down_up_ipq40xx_PHY(get_lan_port_mask(), 1);
		break;
	case 15: // Link down LAN ports
		link_down_up_ipq40xx_PHY(get_lan_port_mask(), 0);
		break;
	case 16: // Link up ALL ports
		link_down_up_ipq40xx_PHY(WANLANPORTS_MASK, 1);
		break;
	case 17: // Link down ALL ports
		link_down_up_ipq40xx_PHY(WANLANPORTS_MASK, 0);
		break;
	case 21:
		break;
	case 22:
		break;
	case 23:
		break;
	case 24:
		break;
#if 0
	case 25:
		set_ipq40xx_broadcast_rate(val2);
		break;
#endif
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
#if 0
	case 40:
		is_singtel_mio(val2);
		break;
	case 50:
		fix_up_hwnat_for_wifi();
		break;
#endif
	case 100:
		break;
	case 114: // link up WAN ports
		for (i = WAN_UNIT_FIRST; i <= max_wan_unit; ++i)
			link_down_up_ipq40xx_PHY(get_wan_port_mask(i), 1);
		break;
	case 115: // link down WAN ports
		for (i = WAN_UNIT_FIRST; i <= max_wan_unit; ++i)
			link_down_up_ipq40xx_PHY(get_wan_port_mask(i), 0);
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

	return ipq40xx_ioctl(val, val2);
}

unsigned int
rtkswitch_Port_phyStatus(unsigned int port_mask)
{
	unsigned int status = 0;

	get_ipq40xx_phy_linkStatus(port_mask, &status);

	return status;
}

unsigned int
rtkswitch_wanPort_phyStatus(int wan_unit)
{
	unsigned int status = 0;

	get_ipq40xx_phy_linkStatus(get_wan_port_mask(wan_unit), &status);

	return status;
}

unsigned int
rtkswitch_lanPorts_phyStatus(void)
{
	unsigned int status = 0;

	get_ipq40xx_phy_linkStatus(get_lan_port_mask(), &status);

	return status;
}

unsigned int
rtkswitch_WanPort_phySpeed(void)
{
	unsigned int speed;

	get_ipq40xx_WAN_Speed(&speed);

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

#if 0 /*defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) obsolete  */
int read_vlan_bmp(int iface)
{
	FILE *fp;
        char cmd[100], buf[128];
        int rlen,uport,dport;
	
        sprintf(cmd, "cat /proc/sys/net/edma/default_group%d_bmp",iface);

        if ((fp = popen(cmd, "r")) == NULL) {
                return 0;
        }
        rlen = fread(buf, 1, sizeof(buf), fp);
        pclose(fp);
        if (rlen <= 1)
                return 0;

        buf[rlen-1] = '\0';
        //_dprintf("=> vlan1 group=%d\n",atoi(buf));

	rlen=0;
	if(0x20 & atoi(buf))
		rlen|=0x2;  //port5		
	if(0x16 & atoi(buf))
		rlen|=0x1;  //port4
	uport=nvram_get_int("upstream_port");
	dport=nvram_get_int("downstream_port");
	if(iface==1)//eth0
	{
		uport|= rlen;
		dport&=!rlen;
	}		
	else
	{
		dport|= rlen;
		uport&=!rlen;
	}
	sprintf(cmd,"%d",uport);
	nvram_set("upstream_port",cmd);
	sprintf(cmd,"%d",dport);
	nvram_set("downstream_port",cmd);
	return rlen;
}

int cap_lanport_status(void)
{
	return rtkswitch_lanPorts_phyStatus();
}

//upstram and downstream port
int re_port_status(int re_iface)
{
	int i,res,member;
	phyState pS;

	for (i = 0; i < NR_WANLAN_PORT; i++) {
		pS.link[i] = 0;
		pS.speed[i] = 0;
		if (!skip_ports[i])
			get_ipq40xx_port_info(lan_id_to_port_nr(i), &pS.link[i], &pS.speed[i]);
	}
	res=0;member=0;
	if(re_iface==0)
		member=read_vlan_bmp(1); //for members of eth0(vlan1) 
	else
		member=read_vlan_bmp(2); //for members of eth1(vlan2) 
	if(member != 0)
	{
		if(pS.link[0]==1 && member&0x2) 
			res|=0x2;

		if(pS.link[1]==1 && member&0x1) 
			res|=0x1;
	}
	return res;	 
}
#else
int cap_lanport_status(void)
{
	return 0;
}
int re_port_status(int re_iface)
{
	return 0;
}
#endif

void ATE_port_status(void)
{
	int i;
	char buf[32];
	phyState pS;

	for (i = 0; i < NR_WANLAN_PORT; i++) {
		pS.link[i] = 0;
		pS.speed[i] = 0;
		if (!skip_ports[i])
			get_ipq40xx_port_info(lan_id_to_port_nr(i), &pS.link[i], &pS.speed[i]);
	}

#if defined(RT4GAC53U)
	sprintf(buf, "L1=%C;L2=%C;",
		(pS.link[1] == 1) ? (pS.speed[1] == 2) ? 'G' : 'M': 'X',
		(pS.link[2] == 1) ? (pS.speed[2] == 2) ? 'G' : 'M': 'X');
#elif defined(RTAC95U)
	sprintf(buf, "W0=%C;L1=%C;L2=%C;L3=%C;",
		(pS.link[0] == 1) ? (pS.speed[0] == 2) ? 'G' : 'M': 'X',
		(pS.link[1] == 1) ? (pS.speed[1] == 2) ? 'G' : 'M': 'X',
		(pS.link[2] == 1) ? (pS.speed[2] == 2) ? 'G' : 'M': 'X',
		(pS.link[3] == 1) ? (pS.speed[3] == 2) ? 'G' : 'M': 'X');
#else
	sprintf(buf, "W0=%C;L1=%C;L2=%C;L3=%C;L4=%C;",
		(pS.link[0] == 1) ? (pS.speed[0] == 2) ? 'G' : 'M': 'X',
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300)
		(pS.link[4] == 1) ? (pS.speed[4] == 2) ? 'G' : 'M': 'X',
		'X',
		'X',
		'X'
#else
		(pS.link[1] == 1) ? (pS.speed[1] == 2) ? 'G' : 'M': 'X',
		(pS.link[2] == 1) ? (pS.speed[2] == 2) ? 'G' : 'M': 'X',
		(pS.link[3] == 1) ? (pS.speed[3] == 2) ? 'G' : 'M': 'X',
		(pS.link[4] == 1) ? (pS.speed[4] == 2) ? 'G' : 'M': 'X'
#endif	/* MAPAC1300 || MAPAC2200 || VZWAC1300 */
		);
#endif
	puts(buf);
}

#ifdef RTCONFIG_LAN4WAN_LED
int led_ctrl(void)
{
        phyState pS;
	int led,i;
        for (i = 1; i < NR_WANLAN_PORT; i++) {
		if (skip_ports[i]) continue;
                led=LED_ID_MAX;
		pS.link[i] = 0;
                pS.speed[i] = 0;
                get_ipq40xx_port_info(lan_id_to_port_nr(i), &pS.link[i], &pS.speed[i]);
		switch(i)
		{
			case 1: led=LED_LAN1;
				break;
			case 2: led=LED_LAN2;
				break;
			case 3: led=LED_LAN3;
				break;
			case 4: led=LED_LAN4;
				break;
			default: break;		
		}
		if(pS.link[i]==1)
			led_control(led, LED_ON);
		else
			led_control(led, LED_OFF);
        }

	return 1;
}
#endif	/* LAN4WAN_LED*/


int detwan_set_def_vid(const char *ifname, int setVid, int needTagged, int avoidVid)
{
	struct {
		const char *ifname;
		const char *path;
		int port_id;
		int def_vid;
	} const ipq40xx_net [] = {
		{ "eth0", "default_wan_tag", 5, 2 },
		{ "eth1", "default_lan_tag", 4, 1 },
	};
	int i;
	char fullpath[64];
	char value[16];
	int ret = -1;
	const char *tagged = "tagged";
	const char *untagged = "untagged";
	char *type = tagged;
	int cvid = 0;
	int vid = setVid;

	if(setVid) {
		for(i = 0; i < ARRAY_SIZE(ipq40xx_net); i++) {
			if(setVid == ipq40xx_net[i].def_vid && strcmp(ipq40xx_net[i].ifname, ifname)) {
				int new_vid;
				new_vid = detwan_set_def_vid(ipq40xx_net[i].ifname, 0, 0, setVid);

#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300)
				if (IPTV_ports_cnt() >= 2)
				{
					doSystem("ssdk_sh vlan entry del %d", new_vid);	/* remove new vid (NOT USED) */
				}
#endif	/* MAPAC1300 || MAPAC2200 || VZWAC1300 */
			}
		}
	}

	for(i = 0; i < ARRAY_SIZE(ipq40xx_net); i++) {
		if(strcmp(ipq40xx_net[i].ifname, ifname))
			continue;

		if(needTagged == 0)
			type = untagged;

		if(vid <= 0 || vid >= 4096) {
			vid = ipq40xx_net[i].def_vid;
			type = untagged;
		}

		snprintf(fullpath, sizeof(fullpath), "/proc/sys/net/edma/%s", ipq40xx_net[i].path);
		if(f_read_string(fullpath, value, sizeof(value)) > 0) {
			int o_vid = atoi(value);
			if(o_vid > 0)
				doSystem("ssdk_sh vlan entry del %d", o_vid);	/* remove old (unrequired) vid */
		}

		if(setVid == 0 && vid == avoidVid) {
			while(1) {
				//vid = (srand ( time(NULL) ) % 4094)+1;
				vid++;
				if(vid == nvram_get_int("switch_wan1tagid") || vid == nvram_get_int("switch_wan2tagid"))
					continue;
				if(doSystem("ssdk_sh vlan entry find %d | grep -q member", vid))
					break;	//find an idle vlan id
			}
		}
		if(type == untagged)
			cvid = vid;

		snprintf(value, sizeof(value), "%d", vid);
		f_write_string(fullpath, value, 0, 0);

		doSystem("ssdk_sh vlan entry create %d", vid);
		doSystem("ssdk_sh vlan member add %d %d %s", vid, 0, tagged);			//CPU port
		doSystem("ssdk_sh vlan member add %d %d %s", vid, ipq40xx_net[i].port_id, type);	//WAN port
		doSystem("ssdk_sh portVlan defaultCVid set %d %d", ipq40xx_net[i].port_id, cvid);
		ret = 0;
		break;
	}

	logmessage(__func__, "set ifname(%s) vlan id(%d/%d) type(%s) avoid(%d) %s", ifname, setVid, vid, type, avoidVid, (ret<0)?"fail !!!":"ok");

	return vid;
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
