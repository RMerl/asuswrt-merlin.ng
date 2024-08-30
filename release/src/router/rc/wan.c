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
 *
 * Network services
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <rc.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <dirent.h>
#if !defined(__GLIBC__) && !defined(__UCLIBC__) /* musl */
#else
#include <linux/sockios.h>
#endif
#include <bcmnvram.h>
#include <shutils.h>
#include <wlutils.h>
#include <bcmutils.h>
#include <bcmparams.h>
#include <net/route.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <rtstate.h>

#include <linux/types.h>
#if !defined(__GLIBC__) && !defined(__UCLIBC__) /* musl */
#include <netinet/if_ether.h>		//have to in front of <linux/ethtool.h> to avoid redefinition of 'struct ethhdr'
#endif
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX

#ifdef RTCONFIG_USB
#include <disk_io_tools.h>
#endif

#ifdef RTCONFIG_RALINK
#include <ralink.h>
#endif

#ifdef RTCONFIG_QCA
#include <qca.h>
#endif

#ifdef RTCONFIG_ALPINE
#include <alpine.h>
#endif

#ifdef RTCONFIG_LANTIQ
#include <lantiq.h>
#endif

#ifdef RTCONFIG_BCM9
#include <linux/netlink.h>
#include <ctf/ctf_cfg.h>
#endif

#ifdef RTCONFIG_BWDPI
#include <bwdpi_common.h>
#endif

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
#include <plc_utils.h>
#endif

#if defined(RTCONFIG_AMAS)
#include <amas_lib.h>
#endif

#ifdef RTCONFIG_MULTILAN_CFG
#include "sdn.h"
#endif

#ifdef RTCONFIG_MULTIWAN_IF
#include "multi_wan.h"
#endif

#define	MAX_MAC_NUM	16
static int mac_num;
static char mac_clone[MAX_MAC_NUM][18];

void convert_wan_nvram(char *prefix, int unit);

#if defined(DSL_N55U) || defined(DSL_N55U_B)
int classATargetTable[]={
	1,
	14,
	27,
	36,
	39,
	42,
	49,
	58,
	59,
	60,
	61,
	101,
	103,
	106,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121,
	122,
	123,
	124,
	125,
	126,
	175,
	180,
	182,
	183,
	202,
	203,
	210,
	211,
	218,
	219,
	220,
	221,
	222,
	223
};

int isTargetArea()
{
	int i;
	char *ip = get_wanip();
	int prefixA = inet_network(ip) >> 24;
_dprintf("==>%s ip: %s, prefix: %d\n", __func__, ip, prefixA);
	for(i=0; i<sizeof(classATargetTable); i++) {
		if( prefixA == classATargetTable[i] )
			return 1;
	}
	return 0;
}
#endif

#ifdef RTCONFIG_DUALWAN
void check_wan_nvram(void){
	if(nvram_match("wan1_proto", "")) nvram_set("wan1_proto", "dhcp");
}
#else
int add_multi_routes(int check_link, int wan_unit){
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char wan_proto[32];
	char cmd[2048];
	char wan_multi_if[WAN_UNIT_MAX][32], wan_multi_gate[WAN_UNIT_MAX][32];
	int debug = nvram_get_int("routes_debug");
	int lock;
	lock = file_lock("mt_routes");
_dprintf("add_multi_routes: running..., unit=%d\n", wan_unit);

	// clean the rules of routing table and re-build them then.
	system("ip rule flush");
	system("ip rule add from all lookup main pref 32766");
	system("ip rule add from all lookup default pref 32767");

	memset(wan_multi_if, 0, sizeof(char)*WAN_UNIT_MAX*32);
	memset(wan_multi_gate, 0, sizeof(char)*WAN_UNIT_MAX*32);

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){ // Multipath
		if(unit != wan_primary_ifunit())
			continue;

		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix, "proto", tmp)));
		snprintf(wan_multi_if[unit], sizeof(wan_multi_if[unit]), "%s", get_wan_ifname(unit));
		snprintf(wan_multi_gate[unit], sizeof(wan_multi_gate[unit]), "%s", nvram_safe_get(strcat_r(prefix, "gateway", tmp)));

		// when wan_down().
		if(check_link && !is_wan_connect(unit)){
_dprintf("add_multi_routes: skip because of the result of is_wan_connect(%d)...\n", unit);
			continue;
		}

		snprintf(cmd, sizeof(cmd), "ip route replace %s dev %s proto kernel", wan_multi_gate[unit], wan_multi_if[unit]);
if(debug) printf("test 10. cmd=%s.\n", cmd);
		system(cmd);

		// set the default gateway.
		snprintf(cmd, sizeof(cmd), "ip route replace default via %s dev %s", wan_multi_gate[unit], wan_multi_if[unit]);
if(debug) printf("test 11. cmd=%s.\n", cmd);
		system(cmd);

		if(!strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "l2tp")){
			snprintf(cmd, sizeof(cmd), "ip route del %s dev %s 2>/dev/null", wan_multi_gate[unit], wan_multi_if[unit]);
if(debug) printf("test 12. cmd=%s.\n", cmd);
			system(cmd);
		}
	}

if(debug) printf("test 26. route flush cache.\n");
	system("ip route flush cache");

	file_unlock(lock);
	return 0;
}
#endif

int
add_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100], *buf;
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
	if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
		ipv6_enabled() &&
		nvram_match(ipv6_nvname("ipv6_only"), "1"))
		return 0;
#endif
	buf = strdup(nvram_safe_get(strcat_r(prefix, var, tmp)));
	if (buf == NULL)
		return -1;

	foreach(word, buf, next) {
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;

		/* Incorrect, empty and 0.0.0.0
		 * probably need to allow empty gateway to set on-link route */
		if (inet_addr_(gateway) == INADDR_ANY)
			gateway = nvram_safe_get_r(strcat_r(prefix, "xgateway", tmp), tmp, sizeof(tmp));
		route_add(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}
	free(buf);

	return 0;
}

static void
add_dhcp_routes(char *prefix, char *ifname, int metric)
{
	char *routes, *tmp;
	char nvname[sizeof("wanXXXXXXXXXX_routesXXX")];
	char *ipaddr, *gateway;
	char netmask[sizeof("255.255.255.255")];
	struct in_addr mask;
	int netsize;

	if (nvram_get_int("dr_enable_x") == 0)
		return;

	/* classful static routes */
	routes = strdup(nvram_safe_get(strcat_r(prefix, "routes", nvname)));
	for (tmp = routes; tmp && *tmp; ) {
		ipaddr  = strsep(&tmp, "/");
		gateway = strsep(&tmp, " ");
		if (gateway && inet_addr(ipaddr) != INADDR_ANY)
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
	}
	free(routes);

	/* ms claseless static routes */
	routes = strdup(nvram_safe_get(strcat_r(prefix, "routes_ms", nvname)));
	for (tmp = routes; tmp && *tmp; ) {
		ipaddr  = strsep(&tmp, "/");
		netsize = atoi(strsep(&tmp, " "));
		gateway = strsep(&tmp, " ");
		if (gateway && netsize > 0 && netsize <= 32 && inet_addr(ipaddr) != INADDR_ANY) {
			mask.s_addr = htonl(0xffffffff << (32 - netsize));
			strcpy(netmask, inet_ntoa(mask));
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);

	/* rfc3442 classless static routes */
	routes = strdup(nvram_safe_get(strcat_r(prefix, "routes_rfc", nvname)));
	for (tmp = routes; tmp && *tmp; ) {
		ipaddr  = strsep(&tmp, "/");
		netsize = atoi(strsep(&tmp, " "));
		gateway = strsep(&tmp, " ");
		if (gateway && netsize > 0 && netsize <= 32 && inet_addr(ipaddr) != INADDR_ANY) {
			mask.s_addr = htonl(0xffffffff << (32 - netsize));
			strcpy(netmask, inet_ntoa(mask));
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);
}

int
del_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100], *buf;

	buf = strdup(nvram_safe_get(strcat_r(prefix, var, tmp)));
	if (buf == NULL)
		return -1;

	foreach(word, buf, next) {
		_dprintf("%s: %s\n", __FUNCTION__, word);

		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;

		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;

		if (inet_addr_(gateway) == INADDR_ANY)
			gateway = nvram_safe_get_r(strcat_r(prefix, "xgateway", tmp), tmp, sizeof(tmp));

		route_del(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}
	free(buf);

	return 0;
}

#if 0
#ifdef RTCONFIG_IPV6
void
stop_ecmh(void)
{
	if (pids("ecmh"))
	{
		killall_tk("ecmh");
		sleep(1);
	}
}

void
start_ecmh(const char *wan_ifname)
{
	int service = get_ipv6_service();

	stop_ecmh();

	if (!wan_ifname || (strlen(wan_ifname) <= 0))
		return;

	if (!nvram_get_int("mr_enable_x"))
		return;

	switch (service) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		eval("/bin/ecmh", "-u", nvram_safe_get("http_username"), "-i", (char*)wan_ifname);
		break;
	}
}
#endif
#endif

#ifdef RTCONFIG_IMPROXY
static const char *improxy_name(int family)
{
	switch (family) {
	case AF_INET:
		return "improxy-igmp";
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		return "improxy-mld";
#endif
	default:
		return "improxy";
	}
}

void
stop_improxy(int family)
{
	char pidfile[sizeof("/var/run/improxy-xxxx.pid")];

	if (family != AF_UNSPEC) {
		snprintf(pidfile, sizeof(pidfile), "/var/run/%s.pid", improxy_name(family));
		kill_pidfile_tk(pidfile);
	} else
		killall_tk("improxy");
}

int
start_improxy(int family, char *wan_ifname)
{
	char options[sizeof("/etc/improxy-xxxx.conf")];
	char pidfile[sizeof("/var/run/improxy-xxxx.pid")];
	char *improxy_argv[] = { "/usr/sbin/improxy",
		"-c", options,
		"-p", pidfile,
		NULL
	};
	const char *fname;
	FILE *fp;
	pid_t pid;

	fname = improxy_name(family);
	snprintf(options, sizeof(options), "/etc/%s.conf", fname);
	snprintf(pidfile, sizeof(pidfile), "/var/run/%s.pid", fname);

	if ((fp = fopen(options, "w")) == NULL) {
		perror(options);
		return -1;
	}

	fprintf(fp,
		"igmp %s version %d\n"		/* default igmp version */
#ifdef RTCONFIG_IPV6
		"mld %s version %d\n"		/* default mld version */
#endif
		"upstream %s\n"			/* upstream interface */
		"downstream %s\n"		/* downstream interface */
		"quickleave %s\n",		/* fast leave */
		(family == AF_UNSPEC || family == AF_INET) ? "enable" : "disable",
		nvram_get_int("mr_igmp_ver") ? : 3,
#ifdef RTCONFIG_IPV6
		(family == AF_UNSPEC || family == AF_INET6) ? "enable" : "disable",
		nvram_get_int("mr_mld_ver") ? : 2,
#endif
		wan_ifname,
		nvram_get("lan_ifname") ? : "br0",
		nvram_get_int("mr_qleave_x") ? "enable" : "disable");

	fclose(fp);

	return _eval(improxy_argv, NULL, 0, &pid);
}
#endif

void
stop_igmpproxy()
{
	if (pids("udpxy")) {
		_dprintf("stop udpxy [%s]\n");
		killall_tk("udpxy");
	}

	_dprintf("stop igmpproxy\n");

#ifdef RTCONFIG_IMPROXY
	stop_improxy(AF_INET);
#elif defined(BLUECAVE)
	stop_mcast_proxy();
#elif defined(HND_ROUTER) || defined(MCPD_PROXY)
	/* nothing yet */
#else
	if (pids("igmpproxy"))
		killall_tk("igmpproxy");
#endif
}

void
start_igmpproxy(char *wan_ifname)
{
#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_DUALWAN
#if !defined(RTCONFIG_MULTISERVICE_WAN)
	if ( nvram_match("wan0_ifname", wan_ifname)
		&& get_dualwan_primary() == WANS_DUALWAN_IF_DSL) {
		if (nvram_get_int("dslx_config_num") > 1)
			wan_ifname = STB_BR_IF;
	}
#endif
#else
	if (nvram_get_int("dslx_config_num") > 1)
		wan_ifname = STB_BR_IF;
#endif
#endif
#ifdef RTCONFIG_MULTISERVICE_WAN
	if (nvram_match("switch_wantag", "none"))
	{
		char iptv_ifname[16] = {0};
		nvram_safe_get_r("iptv_ifname", iptv_ifname, sizeof(iptv_ifname));
		if (strcmp(wan_ifname, STB_BR_IF))
			wan_ifname = iptv_ifname;
	}
#endif

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6 &&
	    !nvram_match("iptv_ifname", wan_ifname))
		return;
#endif

	stop_igmpproxy();

	if (nvram_get_int("udpxy_enable_x")) {
		_dprintf("start udpxy [%s]\n", nvram_get_int("udpxy_if_alt") ? get_wanface() : wan_ifname);
		eval("/usr/sbin/udpxy",
			"-m", nvram_get_int("udpxy_if_alt") ? (char*) get_wanface() : wan_ifname,
			"-p", nvram_safe_get("udpxy_enable_x"),
			"-B", "65536",
			"-c", nvram_safe_get("udpxy_clients"),
			"-a", nvram_get("lan_ifname") ? : "br0");
	}

#if !defined(HND_ROUTER)
	if (!nvram_get_int("mr_enable_x"))
		return;
#endif

	_dprintf("start igmpproxy [%s]\n", wan_ifname);

#ifdef RTCONFIG_IMPROXY
	start_improxy(AF_INET, wan_ifname);
#elif defined(BLUECAVE)
	nvram_set("igmp_ifname", wan_ifname);
	start_mcast_proxy();
#elif defined(HND_ROUTER) || defined(MCPD_PROXY)
	nvram_set("igmp_ifname", wan_ifname);
	start_mcpd_proxy();
#else
	FILE *fp;
	static char *igmpproxy_conf = "/tmp/igmpproxy.conf";
	char *altnet, buf[32];

	if ((fp = fopen(igmpproxy_conf, "w")) == NULL) {
		perror(igmpproxy_conf);
		return;
	}

	snprintf(buf, sizeof(buf), "%s", nvram_safe_get("mr_altnet_x"));
	altnet = buf;

	if (nvram_get_int("mr_qleave_x"))
		fprintf(fp, "quickleave\n");
	fprintf(fp,
		"phyint %s upstream ratelimit 0 threshold 1 altnet %s\n"
		"phyint %s downstream ratelimit 0 threshold 1\n",
		wan_ifname, *altnet ? altnet : "0.0.0.0/0",
		nvram_get("lan_ifname") ? : "br0");

	append_custom_config("igmpproxy.conf", fp);
	fclose(fp);
	use_custom_config("igmpproxy.conf", igmpproxy_conf);
	run_postconf("igmpproxy", igmpproxy_conf);

	eval("/usr/sbin/igmpproxy", igmpproxy_conf);
#endif
}

#ifdef RTCONFIG_IPV6
void
stop_mldproxy()
{
	_dprintf("stop mldproxy\n");

#ifdef RTCONFIG_IMPROXY
	stop_improxy(AF_INET6);
#endif
}

void
start_mldproxy(char *wan_ifname)
{
	stop_mldproxy();

	if ((nvram_get_int("mr_enable_x") & 2) == 0)
		return;

	_dprintf("start mldproxy [%s]\n", wan_ifname);

#ifdef RTCONFIG_IMPROXY
	start_improxy(AF_INET6, wan_ifname);
#endif
}
#endif

int
wan_prefix(char *ifname, char *prefix)
{
	int unit;

	if ((unit = wan_ifunit(ifname)) < 0 &&
	    (unit = wanx_ifunit(ifname)) < 0) {
#ifdef DEBUG
		if(wan_ifunit(ifname) < 0)
			logmessage("wan", "[%s] exit [%d], ifname:[%s]", __FUNCTION__, __LINE__, ifname);
		if(wanx_ifunit(ifname) < 0)
			logmessage("wan", "[%s] exit [%d], ifname:[%s]", __FUNCTION__, __LINE__, ifname);
#endif
		return -1;
	}

	sprintf(prefix, "wan%d_", unit);

	return unit;
}

static int
add_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return add_routes(prefix, "route", wan_ifname);
}

static int
del_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
#if 0
		return -1;
#else
		snprintf(prefix, sizeof(prefix), "wan%d_", WAN_UNIT_FIRST);
#endif

	return del_routes(prefix, "route", wan_ifname);
}

/*
 * (1) wan[x]_ipaddr_x/wan[x]_netmask_x/wan[x]_gateway_x/...:
 *    static ip or ip get from dhcp
 *
 * (2) wan[x]_xipaddr/wan[x]_xnetmaskwan[x]_xgateway/...:
 *    ip get from dhcp when proto = l2tp/pptp/pppoe
 *
 * (3) wan[x]_ipaddr/wan[x]_netmask/wan[x]_gateway/...:
 *    always keeps the latest updated ip/netmask/gateway in system
 *      static: it is the same as (1)
 *      dhcp:
 *      	- before getting ip from dhcp server, it is 0.0.0.0
 *      	- after getting ip from dhcp server, it is updated
 *      l2tp/pptp/pppoe with static ip:
 *      	- before getting ip from vpn server, it is the same as (1)
 *      	- after getting ip from vpn server, it is the one from vpn server
 *      l2tp/pptp/pppoe with dhcp ip:
 *      	- before getting ip from dhcp server, it is 0.0.0.0
 *      	- before getting ip from vpn server, it is the one from vpn server
 */

void update_wan_state(char *prefix, int state, int reason)
{
	char tmp[100], tmp1[100], *ptr;
	int wan_proto, unit = -1;

	_dprintf("%s(%s, %d, %d)\n", __FUNCTION__, prefix, state, reason);

	if (strncmp(prefix, "wan", 3) == 0)
		unit = atoi(prefix + 3);

	nvram_set_int(strcat_r(prefix, "state_t", tmp), state);
	if(state == WAN_STATE_CONNECTED)
	{
		nvram_set_int(strcat_r(prefix, "sbstate_t", tmp), WAN_STOPPED_REASON_NONE);
		nvram_set_int(strcat_r(prefix, "uptime", tmp), uptime());
	}
	else
	{
		nvram_set_int(strcat_r(prefix, "sbstate_t", tmp), reason);
		if(state == WAN_STATE_DISCONNECTED || 
			state == WAN_STATE_DISABLED ||
			state == WAN_STATE_STOPPED)
			{
				nvram_set_int(strcat_r(prefix, "uptime", tmp), 0);
			}
	}

	// 20110610, reset auxstate each time state is changed
	nvram_set_int(strcat_r(prefix, "auxstate_t", tmp), 0);

	if (state == WAN_STATE_INITIALIZING)
	{
		wan_proto = get_wan_proto(prefix);
		nvram_set(strcat_r(prefix, "proto_t", tmp), nvram_safe_get(strcat_r(prefix, "proto", tmp1)));

		/* reset wanX_* variables */
		if (!nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp))) {
			nvram_set(strcat_r(prefix, "ipaddr", tmp), nvram_safe_get(strcat_r(prefix, "ipaddr_x", tmp1)));
			nvram_set(strcat_r(prefix, "netmask", tmp), nvram_safe_get(strcat_r(prefix, "netmask_x", tmp1)));
			nvram_set(strcat_r(prefix, "gateway", tmp), nvram_safe_get(strcat_r(prefix, "gateway_x", tmp1)));
		}
		else {
			nvram_set(strcat_r(prefix, "ipaddr", tmp), "0.0.0.0");
			nvram_set(strcat_r(prefix, "netmask", tmp), "0.0.0.0");
			nvram_set(strcat_r(prefix, "gateway", tmp), "0.0.0.0");
		}
		nvram_unset(strcat_r(prefix, "domain", tmp));
		nvram_unset(strcat_r(prefix, "lease", tmp));
		nvram_unset(strcat_r(prefix, "expires", tmp));
		nvram_unset(strcat_r(prefix, "routes", tmp));
		nvram_unset(strcat_r(prefix, "routes_ms", tmp));
		nvram_unset(strcat_r(prefix, "routes_rfc", tmp));

		/* reset wanX_x* VPN variables */
		if (wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) {
			nvram_set(strcat_r(prefix, "xipaddr", tmp), nvram_safe_get(strcat_r(prefix, "ipaddr", tmp1)));
			nvram_set(strcat_r(prefix, "xnetmask", tmp), nvram_safe_get(strcat_r(prefix, "netmask", tmp1)));
			nvram_set(strcat_r(prefix, "xgateway", tmp), nvram_safe_get(strcat_r(prefix, "gateway", tmp1)));
		} else {
			nvram_set(strcat_r(prefix, "xipaddr", tmp), "0.0.0.0");
			nvram_set(strcat_r(prefix, "xnetmask", tmp), "0.0.0.0");
			nvram_set(strcat_r(prefix, "xgateway", tmp), "0.0.0.0");
		}
		nvram_unset(strcat_r(prefix, "xdomain", tmp));
		nvram_unset(strcat_r(prefix, "xlease", tmp));
		nvram_unset(strcat_r(prefix, "xexpires", tmp));
		nvram_unset(strcat_r(prefix, "xroutes", tmp));
		nvram_unset(strcat_r(prefix, "xroutes_ms", tmp));
		nvram_unset(strcat_r(prefix, "xroutes_rfc", tmp));

		/* reset wanX_dns && wanX_xdns VPN */
		ptr = nvram_get_int(strcat_r(prefix, "dnsenable_x", tmp)) ? "" :
			get_userdns_r(prefix, tmp1, sizeof(tmp1));
		nvram_set(strcat_r(prefix, "dns", tmp), ptr);
		if (nvram_match(strcat_r(prefix, "proto", tmp), "pppoe") ||
		    nvram_match(strcat_r(prefix, "proto", tmp), "pptp") ||
		    nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
			nvram_set(strcat_r(prefix, "xdns", tmp), ptr);
		else
			nvram_unset(strcat_r(prefix, "xdns", tmp));

#ifdef RTCONFIG_IPV6
		nvram_set(strcat_r(prefix, "6rd_ip4size", tmp), "");
		nvram_set(strcat_r(prefix, "6rd_router", tmp), "");
		nvram_set(strcat_r(prefix, "6rd_prefix", tmp), "");
		nvram_set(strcat_r(prefix, "6rd_prefixlen", tmp), "");
#endif
#ifdef RTCONFIG_TR069
//		nvram_unset(strcat_r(prefix, "tr_acs_url", tmp));
//		nvram_unset(strcat_r(prefix, "tr_pvgcode", tmp));
#endif
	}
#if 0
	else if (state == WAN_STATE_STOPPED) {
		// Save Stopped Reason
		// keep ip info if it is stopped from connected
		nvram_set_int(strcat_r(prefix, "sbstate_t", tmp), reason);
	}
#endif
	else if(state == WAN_STATE_STOPPING) {
		snprintf(tmp, sizeof(tmp), "/var/run/ppp-wan%d.status", unit);
		unlink(tmp);
	}

	sprintf(tmp,"%d", unit);

	switch (state) {
	case WAN_STATE_INITIALIZING:
		strcpy(tmp1, "init");
		break;
	case WAN_STATE_CONNECTING:
		strcpy(tmp1, "connecting");
		break;
	case WAN_STATE_CONNECTED:
		strcpy(tmp1, "connected");
		break;
	case WAN_STATE_DISCONNECTED:
		strcpy(tmp1, "disconnected");
		break;
	case WAN_STATE_STOPPED:
		strcpy(tmp1, "stopped");
		break;
	case WAN_STATE_DISABLED:
		strcpy(tmp1, "disabled");
		break;
	case WAN_STATE_STOPPING:
		strcpy(tmp1, "stopping");
		break;
	default:
		sprintf(tmp1, "state %d", state);
	}

	run_custom_script("wan-event", 0, tmp, tmp1);

	/* For backward/legacy compatibility */
	if (state == WAN_STATE_CONNECTED) {
		sprintf(tmp,"%c",prefix[3]);
		run_custom_script("wan-start", 0, tmp, NULL);
	}
}

#ifdef RTCONFIG_IPV6
// for one ipv6 in current stage
void update_wan6_state(char *prefix, int state, int reason)
{
	char tmp[100];

	_dprintf("%s(%s, %d, %d)\n", __FUNCTION__, prefix, state, reason);

	nvram_set_int(strcat_r(prefix, "state_t", tmp), state);
	nvram_set_int(strcat_r(prefix, "sbstate_t", tmp), 0);

	if (state == WAN_STATE_INITIALIZING)
	{
	}
	else if (state == WAN_STATE_STOPPED) {
		// Save Stopped Reason
		// keep ip info if it is stopped from connected
		nvram_set_int(strcat_r(prefix, "sbstate_t", tmp), reason);
	}
}
#endif

// IPOA test case
// 111.235.232.137 (gateway)
// 111.235.232.138 (ip)
// 255.255.255.252 (netmask)

// cat /proc/net/arp
// arp -na

#ifdef RTCONFIG_DSL_REMOTE
static int start_ipoa()
{
	char tc_mac[32];
	char ip_addr[32];
	char ip_mask[32];
	char ip_gateway[32];
	int try_cnt;
	FILE* fp_dsl_mac;
	FILE* fp_log;

	int NeighborIpNum;
	int i;
	int NeiBaseIpNum;
	int LastIpNum;
	int NetMaskLastIpNum;
	char NeighborIpPrefix[32];
	int ip_addr_dot_cnt;
	char CmdBuf[128];

	// mac address is adsl mac
	for (try_cnt = 0; try_cnt < 10; try_cnt++)
	{
		fp_dsl_mac = fopen("/tmp/adsl/tc_mac.txt","r");
		if (fp_dsl_mac != NULL)
		{
			fgets(tc_mac,sizeof(tc_mac),fp_dsl_mac);
			fclose(fp_dsl_mac);
			break;
		}
		usleep(1000*1000);
	}

#ifdef RTCONFIG_DUALWAN
	if (get_dualwan_secondary()==WANS_DUALWAN_IF_DSL)
	{
		strcpy(ip_gateway, nvram_safe_get("wan1_gateway"));
		strcpy(ip_addr, nvram_safe_get("wan1_ipaddr"));
		strcpy(ip_mask, nvram_safe_get("wan1_netmask"));
	}
	else
	{
		strcpy(ip_gateway, nvram_safe_get("wan0_gateway"));
		strcpy(ip_addr, nvram_safe_get("wan0_ipaddr"));
		strcpy(ip_mask, nvram_safe_get("wan0_netmask"));
	}
#else
	strcpy(ip_gateway, nvram_safe_get("wan0_gateway"));
	strcpy(ip_addr, nvram_safe_get("wan0_ipaddr"));
	strcpy(ip_mask, nvram_safe_get("wan0_netmask"));
#endif

	// we only support maximum 256 neighbor host
	if (strncmp("255.255.255",ip_mask,11) != 0)
	{
		fp_log = fopen("/tmp/adsl/ipoa_too_many_neighbors.txt","w");
		fputs("ErrorMsg",fp_log);
		fclose(fp_log);
		return -1;
	}

//
// do not send arp to neighborhood and gateway
//

	ip_addr_dot_cnt = 0;
	for (i=0; i<sizeof(NeighborIpPrefix); i++)
	{
		if (ip_addr[i] == '.')
		{
			ip_addr_dot_cnt++;
			if (ip_addr_dot_cnt >= 3) break;
		}
		NeighborIpPrefix[i]=ip_addr[i];
	}
	NeighborIpPrefix[i] = 0;

	LastIpNum = atoi(&ip_addr[i+1]);
	NetMaskLastIpNum = atoi(&ip_mask[12]);
	NeighborIpNum = ((~NetMaskLastIpNum) + 1)&0xff;
	NeiBaseIpNum = LastIpNum & NetMaskLastIpNum;

	//
	// add gateway host
	//
#ifdef RTCONFIG_DSL_TCLINUX
	eval("arp","-i",nvram_safe_get("wan0_ifname"),"-a",ip_gateway,"-s",tc_mac);
#else
	eval("arp","-i","br0","-a",ip_gateway,"-s",tc_mac);
#endif

	// add neighbor hosts
	for (i=0; i<NeighborIpNum; i++)
	{
		snprintf(CmdBuf, sizeof(CmdBuf), "%s.%d", NeighborIpPrefix, i+NeiBaseIpNum);
#ifdef RTCONFIG_DSL_TCLINUX
		eval("arp","-i",nvram_safe_get("wan0_ifname"),"-a",CmdBuf,"-s",tc_mac);
#else
		eval("arp","-i","br0","-a",CmdBuf,"-s",tc_mac);
#endif
	}

	return 0;
}

static int stop_ipoa()
{
	char ip_addr[32];
	char ip_mask[32];
	char ip_gateway[32];
	FILE* fp_log;

	int NeighborIpNum;
	int i;
	int NeiBaseIpNum;
	int LastIpNum;
	int NetMaskLastIpNum;
	char NeighborIpPrefix[32];
	int ip_addr_dot_cnt;
	char CmdBuf[128];

#ifdef RTCONFIG_DUALWAN
		if (get_dualwan_secondary()==WANS_DUALWAN_IF_DSL)
		{
			strcpy(ip_gateway, nvram_safe_get("wan1_gateway"));
			strcpy(ip_addr, nvram_safe_get("wan1_ipaddr"));
			strcpy(ip_mask, nvram_safe_get("wan1_netmask"));
		}
		else
		{
			strcpy(ip_gateway, nvram_safe_get("wan0_gateway"));
			strcpy(ip_addr, nvram_safe_get("wan0_ipaddr"));
			strcpy(ip_mask, nvram_safe_get("wan0_netmask"));
		}
#else
		strcpy(ip_gateway, nvram_safe_get("wan0_gateway"));
		strcpy(ip_addr, nvram_safe_get("wan0_ipaddr"));
		strcpy(ip_mask, nvram_safe_get("wan0_netmask"));
#endif

	// we only support maximum 256 neighbor host
	if (strncmp("255.255.255",ip_mask,11) != 0)
	{
		fp_log = fopen("/tmp/adsl/ipoa_too_many_neighbors.txt","w");
		fputs("ErrorMsg",fp_log);
		fclose(fp_log);
		return -1;
	}

	//
	// do not send arp to neighborhood and gateway
	//

	ip_addr_dot_cnt = 0;
	for (i=0; i<sizeof(NeighborIpPrefix); i++)
	{
		if (ip_addr[i] == '.')
		{
			ip_addr_dot_cnt++;
			if (ip_addr_dot_cnt >= 3) break;
		}
		NeighborIpPrefix[i]=ip_addr[i];
	}
	NeighborIpPrefix[i] = 0;

	LastIpNum = atoi(&ip_addr[i+1]);
	NetMaskLastIpNum = atoi(&ip_mask[12]);
	NeighborIpNum = ((~NetMaskLastIpNum) + 1)&0xff;
	NeiBaseIpNum = LastIpNum & NetMaskLastIpNum;

	//
	// delete gateway host
	//
	eval("arp","-d",ip_gateway);

	// delete neighbor hosts
	for (i=0; i<NeighborIpNum; i++)
	{
		snprintf(CmdBuf, sizeof(CmdBuf), "%s.%d", NeighborIpPrefix, i+NeiBaseIpNum);
		eval("arp", "-d", CmdBuf);
	}

	return 0;
}
#endif

#ifdef DSL_AC68U	//Andy Chiu, 2015/09/15
#include "interface.h"
extern int cur_ewan_vid;

int check_wan_if(int unit)
{
	//check wan mode.
	dbG("unit=%d\n", unit);
	if(get_wans_dualwan()&WANSCAP_LAN)
	{
		//check vid
		int new_vid;

		if(nvram_match("ewan_dot1q", "1"))
			new_vid = nvram_get_int("ewan_vid");
		else
			new_vid = 4;
		dbG("cur_vid=%d, new_vid=%d\n", cur_ewan_vid, new_vid);

		if(new_vid != cur_ewan_vid)
		{
			//update port info
			char buf[32], wan[32];
			const int ports[] = { 0, 1, 2, 3, 4, 5 };
			int wan1cfg = nvram_get_int("wans_lanport") + WAN1PORT1 - 1;

			snprintf(buf, sizeof(buf), "vlan%dports", cur_ewan_vid);
			nvram_unset(buf);
			switch_gen_config(wan, ports, wan1cfg, 1, "t");
			snprintf(buf, sizeof(buf), "vlan%dports", new_vid);
			nvram_set(buf, wan);

			//update hwname info
			snprintf(buf, sizeof(buf), "vlan%dhwname", cur_ewan_vid);
			nvram_unset(buf);
			snprintf(buf, sizeof(buf), "vlan%dhwname", new_vid);
			nvram_set(buf, "et0");

			//generate vlan interfaces
			char cur_vif[16], new_vif[16];

			snprintf(cur_vif, sizeof(cur_vif), "vlan%d", cur_ewan_vid);
			snprintf(new_vif, sizeof(new_vif), "vlan%d", new_vid);

			dbG("cur_vif=%s, new_vif=%s\n", cur_vif, new_vif);

			char word[256], *next, tmp[256];
			char *p;

			//replace old vlan interface by new vlan interface
			memset(tmp, 0, 256);
			p = tmp;
			foreach(word, nvram_safe_get("wandevs"), next)
			{
				p += sprintf(p, " %s", (!strcmp(word, cur_vif))? new_vif: word);
			}
			dbG("wandevs=%s\n", tmp);
			nvram_set("wandevs", tmp);

			//replace wan_phy
			memset(tmp, 0, 256);
			p = tmp;
			foreach(word, nvram_safe_get("wan_ifnames"), next)
			{
				p += sprintf(p, " %s", (!strcmp(word, cur_vif))? new_vif: word);
			}
			dbG("wan_ifnames=%s\n", tmp);
			nvram_set("wan_ifnames", tmp);

			//check all wan unit.
			int unit;
			for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
			{
				snprintf(buf, sizeof(buf), "wan%d_ifname", unit);
				if(nvram_match(buf, cur_vif))
				{
					dbG("%s=%s\n", buf, new_vif);
					nvram_set(buf, new_vif);
				}

				snprintf(buf, sizeof(buf), "wan%d_gw_ifname", unit);
				if(nvram_match(buf, cur_vif))
				{
					dbG("%s=%s\n", buf, new_vif);
					nvram_set(buf, new_vif);
				}
			}

			//remove old vlan
			eval("ifconfig", cur_vif, "down");
			eval("vconfig", "rem", cur_vif);

			//set new vlan
			eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
			snprintf(buf, sizeof(buf), "%d", new_vid);
			eval("vconfig", "add", "eth0", buf);
			eval("ifconfig", new_vif, "up");
			cur_ewan_vid = new_vid;
		}
		dbG("Set switch\n");
		config_switch_dsl_set_lan();
	}
	return 0;
}
#endif

void start_dhcpfilter(const char *ifname)
{
	char iface[IFNAMSIZ];

	if (!ifname || !*ifname)
		return;

	strlcpy(iface, ifname, sizeof(iface));
	eval("ebtables", "-A", "INPUT", "-p", "ipv4", "-i", iface, "-d", "broadcast"
		, "--ip-proto", "udp", "--ip-destination-port", "67", "-j", "DROP");
	eval("ebtables", "-A", "FORWARD", "-p", "ipv4", "-i", iface, "-d", "broadcast"
		, "--ip-proto", "udp", "--ip-destination-port", "67", "-j", "DROP");
	eval("ebtables", "-A", "FORWARD", "-p", "ipv4", "-o", iface, "-d", "broadcast"
		, "--ip-proto", "udp", "--ip-destination-port", "67", "-j", "DROP");
#ifdef RTCONFIG_IPV6
	eval("ebtables", "-A", "FORWARD", "-p", "ipv6", "-o", iface,
		"--ip6-proto", "ipv6-icmp", "--ip6-icmp-type", "router-solicitation", "-j", "DROP");
	eval("ebtables", "-A", "FORWARD", "-p", "ipv6", "-i", iface,
		"--ip6-proto", "ipv6-icmp", "--ip6-icmp-type", "router-advertisement", "-j", "DROP");
	eval("ebtables", "-A", "OUTPUT", "-p", "ipv6", "-o", iface,
		"--ip6-proto", "ipv6-icmp", "--ip6-icmp-type", "router-advertisement", "-j", "DROP");
#endif
}
void stop_dhcpfilter(const char *ifname)
{
	char iface[IFNAMSIZ];

	if (!ifname || !*ifname)
		return;

	strlcpy(iface, ifname, sizeof(iface));
	eval("ebtables", "-D", "INPUT", "-p", "ipv4", "-i", iface, "-d", "broadcast"
		, "--ip-proto", "udp", "--ip-destination-port", "67", "-j", "DROP");
	eval("ebtables", "-D", "FORWARD", "-p", "ipv4", "-i", iface, "-d", "broadcast"
		, "--ip-proto", "udp", "--ip-destination-port", "67", "-j", "DROP");
	eval("ebtables", "-D", "FORWARD", "-p", "ipv4", "-o", iface, "-d", "broadcast"
		, "--ip-proto", "udp", "--ip-destination-port", "67", "-j", "DROP");
#ifdef RTCONFIG_IPV6
	eval("ebtables", "-D", "FORWARD", "-p", "ipv6", "-o", iface,
		"--ip6-proto", "ipv6-icmp", "--ip6-icmp-type", "router-solicitation", "-j", "DROP");
	eval("ebtables", "-D", "FORWARD", "-p", "ipv6", "-i", iface,
		"--ip6-proto", "ipv6-icmp", "--ip6-icmp-type", "router-advertisement", "-j", "DROP");
	eval("ebtables", "-D", "OUTPUT", "-p", "ipv6", "-o", iface,
		"--ip6-proto", "ipv6-icmp", "--ip6-icmp-type", "router-advertisement", "-j", "DROP");
#endif
}

void restore_wan_ebtables_rules()
{
#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_match("switch_wantag", "hinet_mesh")) {
		start_dhcpfilter(get_wan_ifname(WAN_UNIT_IPTV));
	}
#endif
#ifdef RTCONFIG_MULTISERVICE_WAN
	int base_wan_unit = wan_primary_ifunit();
	int i, mswan_unit;
	char wan_prefix[16];
	for (i = 1; i < WAN_MULTISRV_MAX; i++) {
		mswan_unit = get_ms_wan_unit(base_wan_unit, i);
		snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mswan_unit);
		if (nvram_pf_get_int(wan_prefix, "enable") == 0)
			continue;
		if (!nvram_pf_match(wan_prefix, "proto", "bridge"))
			continue;
		if (nvram_pf_get_int(wan_prefix, "dhcpfilter_enable"))
			start_dhcpfilter(nvram_pf_safe_get(wan_prefix, "ifname"));
	}
#endif
}

void
start_wan_if(int unit)
{
TRACE_PT("dbg: 1. wan_ifname=%s.\n", nvram_safe_get("wan0_ifname"));
#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
	int wan_type;
#endif
#ifdef RTCONFIG_DUALWAN
	int pppoerelay_unit;
#endif
	char wan_ifname[16];
	char tmp[100], prefix[32];
	char eabuf[32];
	int wan_proto, s;
	struct ifreq ifr;
	int wan_mtu;
	pid_t pid;
	int mtu = 0;
#ifdef RTCONFIG_USB_MODEM
	int flags;
	char usb_node[32], port_path[8];
	char nvram_name[32];
	int i = 0;
	char modem_type[32];
#ifdef RTCONFIG_USB_BECEEM
	unsigned int uvid, upid;
#endif
#ifdef SET_USB_MODEM_MTU_ETH
	int modem_mtu;
#endif
	int modem_unit;
	char tmp2[100], prefix2[32];
	char env_unit[32];
#endif
#ifdef RTCONFIG_DSL_REMOTE
	char dsl_prefix[16] = {0};
#endif
	struct vlan_ioctl_args ifv;
#ifdef RTCONFIG_MULTILAN_CFG
	size_t  mtl_sz = 0;
	MTLAN_T *pmtl;
#endif
#if defined(BCM4912)
	uint phy_pwr_skip = 0;
#endif

#ifdef RTCONFIG_HND_ROUTER_AX
#ifdef RTCONFIG_BONDING_WAN
	if(nvram_get_int("bond_wan") == 1)
		start_wan_bonding();
	else
		stop_wan_bonding();
#endif
#endif

#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
	if(!nvram_get_int("stopwar_correct_wan_if") && is_auto_wanport_enabled() == 2 && strcmp(nvram_safe_get("autowan_detected_ifname"), nvram_safe_get("wan0_ifname"))){
		TRACE_PT("AUTO_WANPORT: set the incorrect wan_ifname to be %s.\n", nvram_safe_get("autowan_detected_ifname"));
		nvram_set("wan0_ifname", nvram_safe_get("autowan_detected_ifname"));
	}
#endif
TRACE_PT("dbg: 2. wan_ifname=%s.\n", nvram_safe_get("wan0_ifname"));

#ifdef RTCONFIG_MULTISERVICE_WAN
	if(dualwan_unit__nonusbif(unit) && nvram_match("switch_wantag", "none"))
	{
		if(unit < WAN_UNIT_MAX && unit > WAN_UNIT_NONE)
		{ //GENERIC WAN
			int i = 1;
			int mswan_unit;
			for(i = 1; i < WAN_MULTISRV_MAX; i++) {
				mswan_unit = get_ms_wan_unit(unit, i);
				snprintf(tmp, sizeof(tmp), "wan%d_enable", mswan_unit);
				if (nvram_get(tmp)) {
					config_mswan(mswan_unit);
					start_wan_if(mswan_unit);
				}
			}
		}
#ifdef RTCONFIG_MULTIWAN_IF
		if(MULTI_WAN_START_IDX <= unit && unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
		{ //GENERIC WAN
			int i = 1;
			int mswan_unit;
			for(i = 1; i < WAN_MULTISRV_MAX; i++) {
				mswan_unit = get_ms_wan_unit(unit, i);
				snprintf(tmp, sizeof(tmp), "wan%d_enable", mswan_unit);
				if (nvram_get(tmp)) {
					config_mswan(mswan_unit);
					start_wan_if(mswan_unit);
				}
			}
		}
#endif
	}
#endif

	TRACE_PT("unit=%d.\n", unit);
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* variable should exist? */
	if (nvram_match(strcat_r(prefix, "enable", tmp), "0")) {
		update_wan_state(prefix, WAN_STATE_DISABLED, 0);
#ifdef RTCONFIG_WIFI_SON
		nvram_set_int("link_internet", 0);
#endif
		return;
	}
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	// had detected the DATA limit before.
	else if(get_wan_sbstate(unit) == WAN_STOPPED_REASON_DATALIMIT){
		TRACE_PT("start_wan_if: Data limit was detected and skip the start_wan_if().\n");
		return;
	}
#endif

	update_wan_state(prefix, WAN_STATE_INITIALIZING, 0);
#if defined(RTCONFIG_MT798X)
	void wan_force_link_sp(int unit);
	wan_force_link_sp(unit);
#endif

	// WAR: fix the Tx disconnection of the external PHY
#if defined(GTAXE16000)
	snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	if(*wan_ifname != '\0' && !strcmp(wan_ifname, "eth0") || !strcmp(wan_ifname, "eth5") || !strcmp(wan_ifname, "eth6")){
		_dprintf("%s(%d): phy-reseting %s...\n", __func__, __LINE__, wan_ifname);
		nvram_set("freeze_duck", "10");
		eval("ethctl", wan_ifname, "phy-reset");
		_dprintf("%s: %s was phy-reseted...\n", __func__, wan_ifname);
	}
#endif

	// workaround internal XPHY no Rx issue
#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000)
	strlcpy(wan_ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(wan_ifname));
	if (!strcmp(wan_ifname, "vlan4094") || ((get_wans_dualwan() & WANSCAP_LAN) && !strcmp(wan_ifname, "vlan2"))) {
		nvram_set("freeze_duck", "5");
		if (!strcmp(wan_ifname, "vlan4094"))
			system("rtkswitch 11");
		else if ((get_wans_dualwan() & WANSCAP_LAN) && !strcmp(wan_ifname, "vlan2") && (nvram_get_int("wans_lanport") >= 2) && (nvram_get_int("wans_lanport") <= 4))
			doSystem("rtkswitch 13 %d", nvram_get_int("wans_lanport"));
		sleep(1);
		if (!strcmp(wan_ifname, "vlan4094"))
			system("rtkswitch 10");
		else if ((get_wans_dualwan() & WANSCAP_LAN) && !strcmp(wan_ifname, "vlan2") && (nvram_get_int("wans_lanport") >= 2) && (nvram_get_int("wans_lanport") <= 4))
			doSystem("rtkswitch 12 %d", nvram_get_int("wans_lanport"));
	}
#endif

#if defined(BCM4912) && !defined(RTAX86U_PRO)
	switch (get_wan_proto(prefix)) {
	case WAN_V6PLUS:
	case WAN_OCNVC:
	case WAN_DSLITE:
		snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		if (strlen(wan_ifname) && strstr(wan_ifname, "eth") != NULL) {
#ifdef RTCONFIG_DUALWAN
			if (!nvram_contains_word("wans_dualwan", "none") &&
			     WAN_STATE_CONNECTED == nvram_get_int(strcat_r(prefix, "state_t", tmp))) {
				phy_pwr_skip = 1;
			}
#endif
			if (!phy_pwr_skip) {
				nvram_set("freeze_duck", "7");
				doSystem("ethctl %s phy-power down", wan_ifname);
				usleep(100*1000);
				doSystem("ethctl %s phy-power up", wan_ifname);
			}
		else
			_dprintf("%s: skip to power recycle %s\n", __func__, wan_ifname);
		}
		break;
	default:
		break;
	}
#endif

#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
	wan_type = get_dualwan_by_unit(unit);
#endif

#ifdef RTCONFIG_DUALWAN
	if (is_router_mode()) {
		if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
			check_wan_nvram();
	}
#endif

#ifdef RTCONFIG_USB_MODEM
	if (dualwan_unit__usbif(unit)) {
		FILE *fp;

#ifdef RTCONFIG_USB_MODEM
		modem_unit = get_modemunit_by_type(get_dualwan_by_unit(unit));
		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
		snprintf(env_unit, sizeof(env_unit), "unit=%d", modem_unit);

		if(nvram_get_int(strcat_r(prefix2, "act_scanning", tmp2)) != 0){
_dprintf("start_wan_if: USB modem is scanning...\n");
			update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_USBSCAN);
			return;
		}
		else
#endif
		if(is_usb_modem_ready(get_dualwan_by_unit(unit)) != 1){
			TRACE_PT("No USB Modem!\n");
			return;
		}

		TRACE_PT("3g begin.\n");
		update_wan_state(prefix, WAN_STATE_CONNECTING, WAN_STOPPED_REASON_NONE);

		putenv(env_unit);
		eval("/usr/sbin/find_modem_type.sh");
		unsetenv("unit");
		snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)));

		if(nvram_match("g3err_pin", "1")
				&& strcmp(modem_type, "rndis")){ // Android phone's shared network don't need to check SIM
			TRACE_PT("3g end: PIN error previously!\n");
			update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_PINCODE_ERR);
			return;
		}

		if(!strcmp(modem_type, "tty") || !strcmp(modem_type, "qmi") || !strcmp(modem_type, "mbim") || !strcmp(modem_type, "gobi")
#if defined(RTCONFIG_FIBOCOM_FG621)
			|| (!strcmp(modem_type, "ncm"))
#endif
			){
			if(!is_builtin_modem(modem_type) && !strcmp(nvram_safe_get(strcat_r(prefix2, "act_int", tmp2)), "")){
				if(!strcmp(modem_type, "qmi")){	// e.q. Huawei E398.
					TRACE_PT("Sleep 3 seconds to wait modem nodes.\n");
					sleep(3);
				}
			}

			// find the modem node at every start_wan_if() to avoid the incorrct one sometimes.
			putenv(env_unit);
			eval("/usr/sbin/find_modem_node.sh");
			unsetenv("unit");
		}

		if(nvram_get_int(strcat_r(prefix2, "act_reset", tmp2)) == 1){
			// need to execute find_modem_xxx.sh again.
			TRACE_PT("3g end: Reseting the modem...\n");
			return;
		}

		/* Stop pppd */
		stop_pppd(unit);

		/* Stop dhcp client */
		stop_udhcpc(unit);

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		unsigned long long rx, tx;
		unsigned long long total, limit;

		rx = strtoull(nvram_safe_get("modem_bytes_rx"), NULL, 10);
		tx = strtoull(nvram_safe_get("modem_bytes_tx"), NULL, 10);
		limit = strtoull(nvram_safe_get("modem_bytes_data_limit"), NULL, 10);

		total = rx+tx;

		if(limit > 0 && total >= limit){
			TRACE_PT("3g end: Data limit was set: limit %llu, now %llu.\n", limit, total);
			update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_DATALIMIT);
			return;
		}
#endif

		if(nvram_get_int("stop_conn_3g") == 1){
			write_3g_ppp_conf(get_modemunit_by_type(wan_type));
		}
		else if(strcmp(modem_type, "wimax")){
			putenv(env_unit);
			char *modem_argv[] = {"/usr/sbin/modem_enable.sh", NULL};
			_eval(modem_argv, ">>/tmp/usb.log", 0, NULL);
			unsetenv("unit");

			if(strcmp(modem_type, "rndis")){ // Android phone's shared network don't need to check SIM
				if(nvram_match("g3err_pin", "1")){
					TRACE_PT("3g end: PIN error!\n");
					update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_PINCODE_ERR);
					return;
				}

				snprintf(tmp, sizeof(tmp), "%s", nvram_safe_get(strcat_r(prefix2, "act_sim", tmp2)));
				if(strlen(tmp) > 0){
					int sim_state = atoi(tmp);
					if(sim_state == 2 || sim_state == 3){
						TRACE_PT("3g end: Need to input PIN or PUK.\n");
						update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_PINCODE_ERR);
						return;
					}
					else if(sim_state != 1){
						TRACE_PT("3g end: SIM isn't ready.\n");
						update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_NONE);
						return;
					}
				}
			}
		}

		if((!strcmp(modem_type, "tty") || !strcmp(modem_type, "mbim"))
				&& write_3g_ppp_conf(get_modemunit_by_type(wan_type))
				&& (fp = fopen(PPP_CONF_FOR_3G, "r")) != NULL){
			fclose(fp);

			// run as ppp proto.
			nvram_set(strcat_r(prefix, "proto", tmp), "pppoe");
#ifndef RTCONFIG_DUALWAN
			nvram_set(strcat_r(prefix, "dhcpenable_x", tmp), "1");
			nvram_set(strcat_r(prefix, "vpndhcp", tmp), "0");
			nvram_set(strcat_r(prefix, "dnsenable_x", tmp), "1");
#endif

			char *pppd_argv[] = { "/usr/sbin/pppd", "call", "3g", NULL};

			if(nvram_get_int("stop_conn_3g") != 1)
				_eval(pppd_argv, NULL, 0, NULL);
			else
				TRACE_PT("stop_conn_3g was set.\n");
		}
		// RNDIS, QMI interface: usbX, Beceem interface: usbbcm -> ethX, gct(mad)wimax: wimaxX.
		else{
			snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
TRACE_PT("3g begin with %s.\n", wan_ifname);

			if(strlen(wan_ifname) <= 0){
#ifdef RTCONFIG_USB_BECEEM
				snprintf(usb_node, sizeof(usb_node), "%s", nvram_safe_get("usb_modem_act_path"));
				if(strlen(usb_node) <= 0)
					return;

				if(get_path_by_node(usb_node, port_path, 8) != NULL){
					snprintf(nvram_name, sizeof(nvram_name), "usb_path%s", port_path);
					TRACE_PT("RNDIS/Beceem: start_wan_if.\n");

					if(!strcmp(nvram_safe_get(nvram_name), "modem")){
						snprintf(nvram_name, sizeof(nvram_name), "usb_path%s_vid", port_path);
						uvid = strtoul(nvram_safe_get(nvram_name), NULL, 16);
						snprintf(nvram_name, sizeof(nvram_name), "usb_path%s_pid", port_path);
						upid = strtoul(nvram_safe_get(nvram_name), NULL, 16);

						if(is_samsung_dongle(1, uvid, upid)){
							modprobe("tun");
							sleep(1);

							xstart("madwimax");
						}
						else if(is_gct_dongle(1, uvid, upid)){
							modprobe("tun");
							sleep(1);

							write_gct_conf();

							xstart("gctwimax", "-C", WIMAX_CONF);
						}
					}
				}
#endif

				return;
			}

#define MAX_TRY_IFUP 3
			for (i = 0; i < MAX_TRY_IFUP; i++) {
				if (_ifconfig_get(wan_ifname, &flags, NULL, NULL, NULL, &mtu) != 0) {
					TRACE_PT("Couldn't read the flags of %s!\n", wan_ifname);
					update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
					return;
				}

#ifdef SET_USB_MODEM_MTU_ETH
				modem_mtu = nvram_get_int("modem_mtu");
				mtu = (modem_mtu >= 576 && modem_mtu < mtu) ? modem_mtu : 0;
				if ((flags & IFF_UP) && !mtu)
					break;
				else if(i == (MAX_TRY_IFUP-1)){
					TRACE_PT("Interface %s couldn't be up!\n", wan_ifname);
					update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
					return;
				}

				_ifconfig(wan_ifname, flags | IFUP, NULL, NULL, NULL, mtu);
#else
				if ((flags & IFF_UP))
					break;
				else if(i == (MAX_TRY_IFUP-1)){
					TRACE_PT("Interface %s couldn't be up!\n", wan_ifname);
					update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
					return;
				}

				ifconfig(wan_ifname, flags | IFUP, NULL, NULL);
#endif
				if (!is_builtin_modem(modem_type))
					continue;

				TRACE_PT("%s: wait %s(%s) be up, %d second...!\n", __FUNCTION__, modem_type, wan_ifname, i+1);
				sleep(1);
			}

			// run as dhcp proto.
			nvram_set(strcat_r(prefix, "proto", tmp), "dhcp");
			nvram_set(strcat_r(prefix, "dhcpenable_x", tmp), "1");
			nvram_set(strcat_r(prefix, "dnsenable_x", tmp), "1");

			// Android phone, RNDIS, QMI interface, Gobi.
			if(!strncmp(wan_ifname, "usb", 3) || !strncmp(wan_ifname, "wwan", 4)
					// RNDIS devices should always be named "lte%d" in LTQ platform
					|| !strncmp(wan_ifname, "lte", 3)
					){
				if(nvram_get_int("stop_conn_3g") != 1){
#ifdef RTCONFIG_TCPDUMP
					char *tcpdump_argv[] = { "/usr/sbin/tcpdump", "-i", wan_ifname, "-nnXw", "/tmp/udhcpc.pcap", NULL};

					if(nvram_get_int("dhcp_dump")){
						_eval(tcpdump_argv, NULL, 0, &pid);
						sleep(2);
					}
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
					/* Skip dhcp for IPv6-only USB modem */
					if (nvram_get_int("modem_pdp") == 2) {
						//wan_ifname = get_wan6face();
						ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
						wan_up(wan_ifname);
					} else
#endif
					{
						dbG("start udhcpc(%d): %s.\n", unit, wan_ifname);
						start_udhcpc(wan_ifname, unit, &pid);
					}
				}
				else
					TRACE_PT("stop_conn_3g was set.\n");
			}
			// Beceem dongle, ASIX USB to RJ45 converter, Huawei E353.
			else if(!strncmp(wan_ifname, "eth", 3)){
#ifdef RTCONFIG_USB_BECEEM
				write_beceem_conf(wan_ifname);
#endif

				if(nvram_get_int("stop_conn_3g") != 1){
					snprintf(usb_node, sizeof(usb_node), "%s", nvram_safe_get(strcat_r(prefix2, "act_path", tmp2)));
					if(strlen(usb_node) <= 0)
						return;

					if(get_path_by_node(usb_node, port_path, 8) == NULL)
						return;

					snprintf(nvram_name, sizeof(nvram_name), "usb_path%s_act", port_path);

					if(!strcmp(nvram_safe_get(nvram_name), wan_ifname))
						start_udhcpc(wan_ifname, unit, &pid);

#ifdef RTCONFIG_USB_BECEEM
					if(strlen(nvram_safe_get(nvram_name)) <= 0){
						char buf[128];

						snprintf(buf, sizeof(buf), "wimaxd -c %s", WIMAX_CONF);
						TRACE_PT("%s: cmd=%s.\n", __FUNCTION__, buf);
						system(buf);
						sleep(3);

						TRACE_PT("%s: cmd=wimaxc search.\n", __FUNCTION__);
						system("wimaxc search");
						TRACE_PT("%s: sleep 10 seconds.\n", __FUNCTION__);
						sleep(10);

						TRACE_PT("%s: cmd=wimaxc connect.\n", __FUNCTION__);
						system("wimaxc connect");
					}
#endif
				}
				else
					TRACE_PT("stop_conn_3g was set.\n");
			}
#ifdef RTCONFIG_USB_BECEEM
			else if(!strncmp(wan_ifname, "wimax", 5)){
				if(nvram_get_int("stop_conn_3g") != 1)
					start_udhcpc(wan_ifname, unit, &pid);
				else
					TRACE_PT("stop_conn_3g was set.\n");
			}
#endif
		}

		TRACE_PT("3g end.\n");
		return;
	}
	else
#endif
	if (dualwan_unit__nonusbif(unit)) {
		convert_wan_nvram(prefix, unit);

		/* make sure the connection exists and is enabled */
		snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		if (*wan_ifname == '\0')
			return;

		wan_proto = get_wan_proto(prefix);
		if (wan_proto == WAN_DISABLED) {
			update_wan_state(prefix, WAN_STATE_DISABLED, 0);
			return;
		}

		/* Set i/f hardware address before bringing it up */
		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
			update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
			return;
		}

		strlcpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);

		/* Since WAN interface may be already turned up (by vlan.c),
			if WAN hardware address is specified (and different than the current one),
			we need to make it down for synchronizing hwaddr. */
		if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
			close(s);
			update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
			return;
		}

#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
		if(is_auto_wanport_enabled() <= 0)
#endif
		{
#if defined(TUFAX3000_V2) || defined(RTAXE7800)
		if (!strcmp(wan_ifname, "eth1") || !strcmp(wan_ifname, "eth2") || !strcmp(wan_ifname, "eth3") || !strcmp(wan_ifname, "eth4"))
			doSystem("ethswctl -c wan -i %s -o %s", wan_ifname, "enable");
#endif
#if defined(XT8PRO) || defined(BM68) || defined(ET8PRO) || defined(ET8_V2) || defined(XT8_V2)
		if (!strcmp(wan_ifname, "eth3")){
			doSystem("ethswctl -c wan -i %s -o %s", wan_ifname, "enable");
		}
#endif
#if defined(BT12)
		if (!strcmp(wan_ifname, "eth4")){
			doSystem("ethswctl -c wan -i %s -o %s", wan_ifname, "enable");
		}
#endif
#if defined(BT10)
		if (!strcmp(wan_ifname, "eth2")){
			doSystem("ethswctl -c wan -i %s -o %s", wan_ifname, "enable");
		}
#endif
#if defined(BQ16) || defined(BQ16_PRO)
		if (!strcmp(wan_ifname, "eth4")){
			doSystem("ethswctl -c wan -i %s -o %s", wan_ifname, "enable");
		}
#endif
		}

#ifdef RTCONFIG_IPV6
#if (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
		if ((wan_proto == WAN_STATIC) &&
			!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
			ipv6_enabled() &&
			nvram_match(ipv6_nvname("ipv6_only"), "1"))
			wan_proto = WAN_DHCP;
#endif
		/* Enable wired IPv6 interface */
		int need_linklocal_addr = 0;
		switch (get_ipv6_service_by_unit(unit)) {
		case IPV6_NATIVE_DHCP:
		case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
		case IPV6_PASSTHROUGH:
#endif
			if (!(wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) ||
			    !nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")) {
				enable_ipv6(wan_ifname);
				need_linklocal_addr = 1;
				break;
			}
			/* fall through */
		default:
			disable_ipv6(wan_ifname);
			break;
		}
#endif

		ether_atoe((const char *) nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), (unsigned char *) eabuf);
		if ((bcmp(eabuf, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
#if defined(RTCONFIG_WISP) \
 && (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA))
		 && !wisp_mode()
#endif
		) {
			/* current hardware address is different than user specified */
			ifconfig(wan_ifname, 0, NULL, NULL);
		}

		/* Configure i/f only once, specially for wireless i/f shared by multiple connections */
		if (ioctl(s, SIOCGIFFLAGS, &ifr)) {
			close(s);
			update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
			return;
		}

		if (!(ifr.ifr_flags & IFF_UP)) {
			/* Sync connection nvram address and i/f hardware address */
			memset(ifr.ifr_hwaddr.sa_data, 0, ETHER_ADDR_LEN);

			if (nvram_match(strcat_r(prefix, "hwaddr", tmp), "") ||
			    !ether_atoe((const char *) nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), (unsigned char *) ifr.ifr_hwaddr.sa_data) ||
			    !memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN)) {
				if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
					fprintf(stderr, "ioctl fail. continue\n");
					close(s);
					update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
					return;
				}
				nvram_set(strcat_r(prefix, "hwaddr", tmp), ether_etoa((unsigned char *) ifr.ifr_hwaddr.sa_data, eabuf));
			}
			else {
#if defined(RTCONFIG_DETWAN)
				unsigned char lan[6], wan[6];

				ether_atoe((const char *) get_lan_hwaddr(), lan);
				ether_atoe((const char *) get_wan_hwaddr(), wan);

				if (nvram_match(strcat_r(prefix, "ifname", tmp), "eth0")) {
					if(memcmp(ifr.ifr_hwaddr.sa_data, lan, 6) == 0)
						memcpy(ifr.ifr_hwaddr.sa_data, wan, 6);	//change to the original mac when same as lan in eth0
				} else if (nvram_match(strcat_r(prefix, "ifname", tmp), "eth1")) {
					if(memcmp(ifr.ifr_hwaddr.sa_data, wan, 6) == 0)
						memcpy(ifr.ifr_hwaddr.sa_data, lan, 6);	//change to the original mac when same as wan in eth1
				}
#endif	/* RTCONFIG_DETWAN */
#if defined(RTCONFIG_BONDING_WAN) && defined(RTCONFIG_QCA)
				if (!strncmp(ifr.ifr_name, "bond", 4)) {
					ether_etoa((unsigned char *) ifr.ifr_hwaddr.sa_data, eabuf);
					set_bonding_iface_hwaddr(ifr.ifr_name, eabuf);
				} else {
#endif
					ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
					ioctl(s, SIOCSIFHWADDR, &ifr);
#if defined(RTCONFIG_BONDING_WAN) && defined(RTCONFIG_QCA)
				}
#endif
			}

			wan_mtu = nvram_get_int(strcat_r(prefix, "mtu", tmp));

			/* Bring up i/f */
			_ifconfig(wan_ifname, IFUP, NULL, NULL, NULL, wan_mtu);
		}
		close(s);

#ifdef RTCONFIG_IPV6
		/* Reset linklocal address if necessary after interface is up */
		if (need_linklocal_addr && !with_ipv6_linklocal_addr(wan_ifname)) {
			reset_ipv6_linklocal_addr(wan_ifname, 0);
			enable_ipv6(wan_ifname);
		}
#endif

		/* Set initial QoS mode again now that WAN port is ready. */
#ifdef CONFIG_BCMWL5
		set_et_qos_mode();
#endif

#ifdef RTCONFIG_DUMP4000
#define	WANCAP_FILE1	"/tmp/wan1.pcap"
#define	WANCAP_FILE2	"/tmp/wan2.pcap"
		if (!f_exists(WANCAP_FILE1)) { /* first time, detection period  */
			char *tcpdump_argv[] = { "/usr/sbin/tcpdump", "-i", wan_ifname, "-c", "4000", "-nn", "-w", WANCAP_FILE1, NULL};
			_dprintf("[DDDDD] run tcpdump 1st on %s!!\n", wan_ifname);
			_eval(tcpdump_argv, NULL, 0, &pid);
			sleep(1);
		} else if (!f_exists(WANCAP_FILE2)) { /* second time, QIS finished */
			char *tcpdump_argv[] = { "/usr/sbin/tcpdump", "-i", wan_ifname, "-c", "4000", "-nn", "-w", WANCAP_FILE2, NULL};
			killall("tcpdump", SIGTERM); // kill first one if still alive
			sleep(1);
			_dprintf("[DDDDD] run tcpdump 2nd on %s!!\n", wan_ifname);
			_eval(tcpdump_argv, NULL, 0, &pid);
			sleep(1);
		}
#endif

#ifdef RTCONFIG_DUALWAN
		pppoerelay_unit = wan_primary_ifunit();
		if (nvram_match("wans_mode", "lb") && get_nr_wan_unit() > 1)
			pppoerelay_unit = nvram_get_int("pppoerelay_unit");
		if (unit == pppoerelay_unit)
			start_pppoe_relay(wan_ifname);
#else
		if (unit == wan_primary_ifunit())
			start_pppoe_relay(wan_ifname);
#endif

		enable_ip_forward();

		update_wan_state(prefix, WAN_STATE_CONNECTING, 0);

#ifdef RTCONFIG_SOFTWIRE46
		if (wan_proto != WAN_V6PLUS ||
		    wan_proto != WAN_OCNVC ||
		    wan_proto != WAN_DSLITE) {
			stop_v6plusd(unit);
			stop_ocnvcd(unit);
			stop_dslited(unit);
			nvram_pf_set_int(prefix, "s46_hgw_case", S46_CASE_INIT);
		}
#if defined(RTCONFIG_RALINK)
		switch (wan_proto) {
		case WAN_MAPE:
		case WAN_V6PLUS:
		case WAN_OCNVC:
			/* Enable mtkhnat to support map-e. */
			system("echo 1 > /sys/kernel/debug/hnat/mape_toggle");
			break;
		default:
			/* Enable mtkhnat to support ds-lite.(As Default) */
			system("echo 0 > /sys/kernel/debug/hnat/mape_toggle");
			break;
		}
#endif
#endif
		/*
		 * Configure PPPoE connection. The PPPoE client will run
		 * ip-up/ip-down scripts upon link's connect/disconnect.
		 */
		switch (wan_proto) {
#ifdef RTCONFIG_SOFTWIRE46
			char tun_dev[IFNAMSIZ];
#endif
		case WAN_PPPOE:
		case WAN_PPTP:
		case WAN_L2TP:
		{
			char ipaddr[sizeof("255.255.255.255")], netmask[sizeof("255.255.255.255")];
			int dhcpenable = nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp));
			int demand = nvram_get_int(strcat_r(prefix, "pppoe_idletime", tmp)) &&
				     (wan_proto != WAN_L2TP); /* L2TP does not support idling */
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
			char ip_mask[sizeof("192.168.100.200/255.255.255.255XXX")];
#endif

			snprintf(ipaddr, sizeof(ipaddr), "%s", nvram_safe_get(strcat_r(prefix, "xipaddr", tmp)));
			snprintf(netmask, sizeof(netmask), "%s", nvram_safe_get(strcat_r(prefix, "xnetmask", tmp)));

			/* update demand option */
			nvram_set_int(strcat_r(prefix, "pppoe_demand", tmp), demand);

#ifdef RTCONFIG_MULTILAN_CFG
			if (dhcpenable == 0) {
				pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
				if (pmtl) {
					int i, conflict = 0;
					get_mtlan(pmtl, &mtl_sz);
					for (i = 0; i < mtl_sz; i++) {
						if (pmtl[i].enable
						 && inet_equal(ipaddr, netmask, pmtl[i].nw_t.addr, pmtl[i].nw_t.netmask)
						) {
							update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
							conflict = 1;
							break;
						}
					}
					FREE_MTLAN((void *)pmtl);
					if (conflict)
						return;
				}
			}
#else
			if (dhcpenable == 0 &&
			    inet_equal(ipaddr, netmask,
				       nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"))) {
				update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
				return;
			}
#endif

#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
			/* If return value of test_and_get_free_char_network() is 1 and
			 * we got different IP/netmask from it, the WAN IP/netmask conflicts with known networks.
			 */
			if (!dhcpenable) {
				snprintf(ip_mask, sizeof(ip_mask), "%s/%s", ipaddr, netmask);
				if (test_and_get_free_char_network(7, ip_mask, EXCLUDE_NET_ALL_EXCEPT_LAN_VLAN) == 1) {
					logmessage("start_wan_if", "%d, %s conflicts with known networks", unit, ip_mask);
					update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
					return;
				}
			}
#endif
#if defined(RTCONFIG_COOVACHILLI)
			if (!dhcpenable) {
				restart_coovachilli_if_conflicts(ipaddr, netmask);
			}
#endif
			/* Bring up WAN interface */
			ifconfig(wan_ifname, IFUP, ipaddr, netmask);

			/* Increase WAN interface's MTU to allow pppoe MTU/MRU over 1492 (with 8 byte overhead) */
			if (wan_proto == WAN_PPPOE) {
				/* Compute maximum required MTU by taking the maximum of the pppoe MRU and MTU values */
				int mru = nvram_get_int(strcat_r(prefix, "pppoe_mru", tmp));
				mtu = nvram_get_int(strcat_r(prefix, "pppoe_mtu", tmp));
				if (mru > mtu)
					mtu = mru;
				if (mtu > 1492) {
					if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
						/* First set parent device if vlan was configured */
						strncpy(ifv.device1, wan_ifname, IFNAMSIZ);
						ifv.cmd = GET_VLAN_REALDEV_NAME_CMD;
						if (ioctl(s, SIOCGIFVLAN, &ifv) >= 0) {
							strncpy(ifr.ifr_name, ifv.u.device2, IFNAMSIZ);
							ifr.ifr_mtu = mtu + 8;
							if (ioctl(s, SIOCSIFMTU, &ifr)) {
								perror(wan_ifname);
								logmessage("start_wan_if()", "Error setting MTU on %s to %d", ifv.u.device2, mtu);
							}
						}

						/* Set WAN device */
						strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
						ifr.ifr_mtu = mtu + 8;
						if (ioctl(s, SIOCSIFMTU, &ifr)) {
							perror(wan_ifname);
							logmessage("start_wan_if()", "Error setting MTU on %s to %d", wan_ifname, mtu);
						}
						close(s);
					}
				}
			}

			/* launch dhcp client and wait for lease forawhile */
			if (dhcpenable) {
#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
				if(is_auto_wanport_enabled() == 1){
					strlcpy(wan_ifname, nvram_safe_get("lan_ifname"), sizeof(wan_ifname));

					dbG("AUTO_WANPORT start PPP's udhcpc: %s, %d\n", wan_ifname, unit);
					start_udhcpc(wan_ifname, unit,
						(wan_proto == WAN_PPPOE) ? &pid : NULL);

					dbG("AUTO_WANPORT Skip the below PPP procedure: %s, %d\n", wan_ifname, unit);
					break;
				}
				else
#endif
				{
					start_udhcpc(wan_ifname, unit,
						(wan_proto == WAN_PPPOE) ? &pid : NULL);
				}
			} else {
				char gateway[16];

				snprintf(gateway, sizeof(gateway), "%s", nvram_safe_get(strcat_r(prefix, "xgateway", tmp)));

				/* start firewall */
// TODO: handle different lan_ifname
				start_firewall(unit, 0);

				/* setup static wan routes via physical device */
				add_routes(prefix, "mroute", wan_ifname);

				/* and set default route if specified with metric 1 */
				if (inet_addr_(gateway) != INADDR_ANY &&
				    !nvram_match(strcat_r(prefix, "heartbeat_x", tmp), "")) {
					in_addr_t mask = inet_addr(netmask);

					/* the gateway is out of the local network */
					if ((inet_addr(gateway) & mask) != (inet_addr(ipaddr) & mask))
						route_add(wan_ifname, 2, gateway, NULL, "255.255.255.255");

					/* default route via default gateway */
					route_add(wan_ifname, 2, "0.0.0.0", gateway, "0.0.0.0");
				}

				/* update resolv.conf */
				update_resolvconf();

				/* start multicast router on Static+VPN physical interface */
				if (unit == wan_primary_ifunit())
					start_igmpproxy(wan_ifname);
			}

#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
			if(is_auto_wanport_enabled() == 1){
				dbG("AUTO_WANPORT Skip the below PPP procedure (no DHCP): %s, %d\n", wan_ifname, unit);
				break;
			}
#endif

#if defined(HND_ROUTER) || defined(RTAC1200V2) || defined(RTACRH18)|| defined(RT4GACRH86U)
			if (wan_proto == WAN_PPTP && !module_loaded("pptp"))
				modprobe("pptp");
#endif

#if defined(RTCONFIG_TCPDUMP) && defined(RTCONFIG_SOC_IPQ40XX) && defined(RTCONFIG_PSISTLOG)
			{
				char *tcpdump_argv[] = { "/usr/sbin/tcpdump", "-i", wan_ifname, "-nnXw", "/jffs/pppoe.pcap", NULL};
				_eval(tcpdump_argv, NULL, 0, &pid);
				sleep(1);
			}
#endif	/* RTCONFIG_TCPDUMP && RTCONFIG_SOC_IPQ40XX && RTCONFIG_PSISTLOG */

			/* launch pppoe client daemon */
			start_pppd(unit);

			/* ppp interface name is referenced from this point
			 * after pppd start before ip-pre-up it will be empty */
			snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp)));

#ifdef RTCONFIG_MULTI_PPP
#ifdef RTCONFIG_DUALWAN
			if (!(!strstr(nvram_safe_get("wans_dualwan"), "none") && nvram_match("wans_mode", "lb")))	//exclude load balance
#endif
			{
				if (is_mtppp_base_unit(unit)) {
					int i = 1;
					int max_mtppp_unit = nvram_get_int(strcat_r(prefix, "ppp_conn", tmp));
					for(i = 1; i < max_mtppp_unit; i++) {
						start_pppd(get_mtppp_wan_unit(unit, i));
					}
				}
			}
#endif

			/* Pretend that the WAN interface is up */
			if (demand) {
				int timeout = 5;

				/* Wait for pppx to be created */
				while (timeout--) {
					/* ppp interface name is re-referenced from this point */
					snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp)));

					if(strlen(wan_ifname) > 0 && ifconfig(wan_ifname, IFUP, NULL, NULL) == 0)
						break;
					_dprintf("%s: wait interface %s up at %d seconds...\n", __FUNCTION__, wan_ifname, timeout);
					sleep(1);
				}

				if(strlen(wan_ifname) <= 0){
					_dprintf("%s: no interface of wan_unit %d.\n", __FUNCTION__, unit);
					return;
				}

				/* Retrieve IP info */
				if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
					update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_SYSTEM_ERR);
					return;
				}
				strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);

				/* Set temporary IP address */
				if (ioctl(s, SIOCGIFADDR, &ifr))
					perror(wan_ifname);
				nvram_set(strcat_r(prefix, "ipaddr", tmp), inet_ntoa(sin_addr(&ifr.ifr_addr)));
				nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.255");

				/* Set temporary P-t-P address */
				if (ioctl(s, SIOCGIFDSTADDR, &ifr))
					perror(wan_ifname);
				nvram_set(strcat_r(prefix, "gateway", tmp), inet_ntoa(sin_addr(&ifr.ifr_dstaddr)));

				close(s);

				/*
				 * Preset routes so that traffic can be sent to proper pppx even before
				 * the link is brought up.
				 */
				preset_wan_routes(wan_ifname);

				/* Trigger it up to obtain PPP DNS early */
				start_demand_ppp(unit, 0);
			}
			break;
		}

		/*
		 * Configure DHCP connection. The DHCP client will run
		 * 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address
		 * renew and release.
		 */
		case WAN_DHCP:
		{
#if defined(RTCONFIG_BCM_7114) && defined(RTCONFIG_AMAS) && defined(RTCONFIG_ETHOBD)
			if (nvram_get_int("x_Setting") == 0) {
				if(strcmp(wan_ifname, nvram_safe_get("eth_ifnames"))) {
					dbG("ifup:%s\n", nvram_safe_get("eth_ifnames"));
					ifconfig(nvram_safe_get("eth_ifnames"), IFUP, NULL, NULL);
				}
			}
#endif

#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
			if(is_auto_wanport_enabled() == 1){
				strlcpy(wan_ifname, nvram_safe_get("lan_ifname"), sizeof(wan_ifname));

				/* Bring up WAN interface */
				dbG("AUTO_WANPORT ifup:%s\n", wan_ifname);
				ifconfig(wan_ifname, IFUP, NULL, NULL);

				/* MTU */
				if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
					mtu = nvram_get_int(strcat_r(prefix, "mtu", tmp));
					if ((mtu < 576) || (mtu > 9000))
						mtu = 1500;     // Set a sane default value

					ifr.ifr_mtu = mtu;
					strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
					if (ioctl(s, SIOCSIFMTU, &ifr)) {
						perror(wan_ifname);
						logmessage("start_wan_if()","Error setting MTU on %s to %d", wan_ifname, mtu);
					}
					close(s);
				}

				/* Start pre-authenticator */
				dbG("AUTO_WANPORT start auth:%d\n", unit);
				start_auth(unit, 0);

				/* Start dhcp daemon */
				dbG("AUTO_WANPORT start udhcpc:%s, %d\n", wan_ifname, unit);
				start_udhcpc(wan_ifname, unit, &pid);
			}
			else
#endif
			{
				/* Bring up WAN interface */
				dbG("ifup:%s\n", wan_ifname);
				ifconfig(wan_ifname, IFUP, NULL, NULL);

				/* MTU */
				if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
					mtu = nvram_get_int(strcat_r(prefix, "mtu", tmp));
					if ((mtu < 576) || (mtu > 9000))
						mtu = 1500;     // Set a sane default value

					ifr.ifr_mtu = mtu;
					strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
					if (ioctl(s, SIOCSIFMTU, &ifr)) {
						perror(wan_ifname);
						logmessage("start_wan_if()","Error setting MTU on %s to %d", wan_ifname, mtu);
					}
					close(s);
				}

				/* Start pre-authenticator */
				dbG("start auth:%d\n", unit);
				start_auth(unit, 0);

				/* Start dhcp daemon */
				dbG("start udhcpc:%s, %d\n", wan_ifname, unit);
				start_udhcpc(wan_ifname, unit, &pid);
			}

			break;
		}

		/* Configure static IP connection. */
		case WAN_STATIC:
		{
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
			char ip_mask[sizeof("192.168.100.200/255.255.255.255XXX")];
#endif

#ifdef RTCONFIG_MULTILAN_CFG
			pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
			if (pmtl) {
				int i, conflict = 0;
				get_mtlan(pmtl, &mtl_sz);
				for (i = 0; i < mtl_sz; i++) {
					if (pmtl[i].enable
					 && inet_equal(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), nvram_safe_get(strcat_r(prefix, "netmask", tmp))
						, pmtl[i].nw_t.addr, pmtl[i].nw_t.netmask)
					) {
						update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
						conflict = 1;
						break;
					}
				}
				FREE_MTLAN((void *)pmtl);
				if (conflict)
					return;
			}
#else
			if (inet_equal(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), nvram_safe_get(strcat_r(prefix, "netmask", tmp)),
				       nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"))) {
				update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
				return;
			}
#endif

#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
			/* If return value of test_and_get_free_char_network() is 1 and
			 * we got different IP/netmask from it, the WAN IP/netmask conflicts with known networks.
			 */
			snprintf(ip_mask, sizeof(ip_mask), "%s/%s",
				nvram_pf_safe_get(prefix, "ipaddr"), nvram_pf_safe_get(prefix, "netmask"));
			if (test_and_get_free_char_network(7, ip_mask, EXCLUDE_NET_ALL_EXCEPT_LAN_VLAN) == 1) {
				logmessage("start_wan_if", "%d, %s conflicts with known networks", unit, ip_mask);
				update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
				return;
			}
#endif
#if defined(RTCONFIG_COOVACHILLI)
			restart_coovachilli_if_conflicts(nvram_pf_get(prefix, "ipaddr"), nvram_pf_get(prefix, "netmask"));
#endif

			/* Assign static IP address to i/f */
			ifconfig(wan_ifname, IFUP,
					nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
					nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

			/* Start pre-authenticator */
			start_auth(unit, 0);

#ifdef RTCONFIG_DSL_REMOTE
			if (get_dsl_prefix_by_wan_unit(unit, dsl_prefix, sizeof(dsl_prefix)) == 0)
			(
				if (nvram_pf_match(dsl_prefix, "proto", "ipoa"))
					start_ipoa();
#endif

			/* MTU */
			if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
				mtu = nvram_get_int(strcat_r(prefix, "mtu", tmp));
				if ((mtu < 576) || (mtu > 9000))
					mtu = 1500;	// Set a sane default value

				ifr.ifr_mtu = mtu;
				strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
				if (ioctl(s, SIOCSIFMTU, &ifr)) {
					perror(wan_ifname);
					logmessage("start_wan_if()","Error setting MTU on %s to %d", wan_ifname, mtu);
				}
				close(s);
			}

			/* We are done configuration */
			wan_up(wan_ifname);
			break;
		}

		case WAN_BRIDGE:
		{
#ifdef RTCONFIG_MULTILAN_CFG
			int i;
			int cnt = 0;
			pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
			if (pmtl) {
				get_mtlan(pmtl, &mtl_sz);
				for (i = 1; i < mtl_sz; i++) {
#ifdef RTCONFIG_MULTISERVICE_WAN
					if (pmtl[i].sdn_t.mswan_idx == get_ms_idx_by_wan_unit(unit))
#else
					if (pmtl[i].sdn_t.wan_idx == unit)
#endif
					{
						cnt++;
						eval("brctl", "addif", pmtl[i].nw_t.br_ifname, wan_ifname);
#ifdef HND_ROUTER
						eval("bcmmcastctl", "mode", "-i", pmtl[i].nw_t.br_ifname, "-p", "1", "-m", "1");
						eval("bcmmcastctl", "mode", "-i", pmtl[i].nw_t.br_ifname, "-p", "2", "-m", "1");
#endif
						if (nvram_get_int(strcat_r(prefix, "dhcpfilter_enable", tmp))) {
							start_dhcpfilter(wan_ifname);
						}
						break;
					}
				}
				FREE_MTLAN((void *)pmtl);
			}
			if (cnt == 0) {
				int switch_stb_x = nvram_get_int("switch_stb_x");
				char br_ifname[IFNAMSIZ] = {0};
				if (switch_stb_x == 0 || switch_stb_x > 6) {
					strlcpy(br_ifname, nvram_safe_get("lan_ifname"), sizeof(br_ifname));
					if (nvram_match("switch_wantag", "hinet_mesh")) {
						start_dhcpfilter(wan_ifname);
						eval("dhcpfwd", "-i", nvram_safe_get("lan_ifname"), "-o", wan_ifname, "-V", "CHT");
					}
					else if (nvram_get_int(strcat_r(prefix, "dhcpfilter_enable", tmp))) {
						start_dhcpfilter(wan_ifname);
					}
				}
				else {
					strlcpy(br_ifname, STB_BR_IF, sizeof(br_ifname));
				}
				eval("brctl", "addif", br_ifname, wan_ifname);
#ifdef HND_ROUTER
				eval("bcmmcastctl", "mode", "-i", br_ifname, "-p", "1", "-m", "1");
				eval("bcmmcastctl", "mode", "-i", br_ifname, "-p", "2", "-m", "1");
#endif
			}
#else
#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_DSL_BCM
			if (nvram_get_int("switch_stb_x")) {
				config_wan_bridge(STB_BR_IF, wan_ifname, 1);
			}
			else {
				char *br_itf = nvram_safe_get("lan_ifname");
				config_wan_bridge(br_itf, wan_ifname, 1);
				if (nvram_get_int(strcat_r(prefix, "dhcpfilter_enable", tmp))) {
					start_dhcpfilter(wan_ifname);
				}
				eval("bcmmcastctl", "mode", "-i", br_itf, "-p", "1", "-m", "1");
				eval("bcmmcastctl", "mode", "-i", br_itf, "-p", "2", "-m", "1");
			}
#else
			if (nvram_get_int("wan2lan")) {
				config_wan_bridge(nvram_safe_get("lan_ifname"), wan_ifname, 1);
			}
			else {
				config_wan_bridge(STB_BR_IF, wan_ifname, 1);
			}
#endif
#else
			eval("brctl", "addif", nvram_get_int("switch_stb_x") ? STB_BR_IF : nvram_safe_get("lan_ifname"), wan_ifname);
			if (nvram_match("switch_wantag", "hinet_mesh")) {
				start_dhcpfilter(wan_ifname);
				eval("dhcpfwd", "-i", nvram_safe_get("lan_ifname"), "-o", wan_ifname, "-V", "CHT");
			}
			else if (nvram_get_int(strcat_r(prefix, "dhcpfilter_enable", tmp))) {
				start_dhcpfilter(wan_ifname);
			}
#ifdef HND_ROUTER
			eval("bcmmcastctl", "mode", "-i", nvram_safe_get("lan_ifname"), "-p", "1", "-m", "1");
			eval("bcmmcastctl", "mode", "-i", nvram_safe_get("lan_ifname"), "-p", "2", "-m", "1");
#endif
#endif
#endif
			wan_up(wan_ifname);
			break;
		}

#ifdef RTCONFIG_SOFTWIRE46
		/* Configure Softwire46 connection */
		case WAN_LW4O6:
		case WAN_MAPE:
		{
			/* Bring up WAN interface */
			dbG("ifup:%s\n", wan_ifname);
			ifconfig(wan_ifname, IFUP, NULL, NULL);

			/* tunnel interface name is referenced from this point */
			snprintf(tun_dev, sizeof(tun_dev), "v4tun%d", unit);
			nvram_set(strcat_r(prefix, "pppoe_ifname", tmp), tun_dev);

			start_firewall(unit, 0);

			/* Postpone configuration */
			wan6_up(wan_ifname);

			break;
		}
		case WAN_V6PLUS:
		case WAN_OCNVC:
		case WAN_DSLITE:
		{
			nvram_pf_set_int(prefix, "s46_hgw_case", S46_CASE_INIT);

			/* start wan46 daemon */
			switch (wan_proto) {
				case WAN_V6PLUS:
					restart_v6plusd(unit);
					break;
				case WAN_OCNVC:
					restart_ocnvcd(unit);
					break;
				case WAN_DSLITE:
					restart_dslited(unit);
					break;
			}

			/* Bring up WAN interface */
			dbG("ifup:%s\n", wan_ifname);
			ifconfig(wan_ifname, IFUP, NULL, NULL);

			/* Start pre-authenticator */
			dbG("start auth:%d\n", unit);
			start_auth(unit, 0);

#if defined(RTCONFIG_RALINK)
			if (wan_proto == WAN_V6PLUS || wan_proto == WAN_OCNVC) {
				/* Enable mtkhnat to support map-e. */
				system("echo 1 > /sys/kernel/debug/hnat/mape_toggle");
			} else {
				/* Enable mtkhnat to support ds-lite.(As Default) */
				system("echo 0 > /sys/kernel/debug/hnat/mape_toggle");
			}
#endif
			/* Start dhcp daemon */
			dbG("start udhcpc:%s, %d\n", wan_ifname, unit);
			start_udhcpc(wan_ifname, unit, &pid);

			/* tunnel interface name is referenced from this point */
			snprintf(tun_dev, sizeof(tun_dev), "v4tun%d", unit);
			nvram_set(strcat_r(prefix, "pppoe_ifname", tmp), tun_dev);

			start_firewall(unit, 0);
			break;
		}
#endif
		}
	} else {
#ifdef RTCONFIG_DUALWAN
		_dprintf("%s(): Cound't find the type(%d) of unit(%d)!!!\n", __FUNCTION__, wan_type, unit);
#else
		_dprintf("%s(): Cound't find the wan(%d)!!!\n", __FUNCTION__, unit);
#endif
	}

	_dprintf("%s(): End.\n", __FUNCTION__);

#ifdef RTCONFIG_IPSEC
	if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
		) {
		wait_ntp_repeat(WAIT_FOR_NTP_READY_TIME, WAIT_FOR_NTP_READY_LOOP); //wait for ntp_ready, for rc_ipsec_config_init().
		rc_ipsec_config_init();
#ifdef RTCONFIG_MULTILAN_CFG
		start_dnsmasq(ALL_SDN);
#else
		start_dnsmasq();
#endif
	}
#endif
}

void
stop_wan_if(int unit)
{
#if defined(RTCONFIG_DSL_REMOTE)
	char dsl_prefix[16] = {0};
#endif
	char wan_ifname[16];
	char tmp[100], prefix[32];
	char wan_proto[16], active_proto[16];
#ifdef RTCONFIG_USB_BECEEM
	int i;
	unsigned int uvid, upid;
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
	int modem_unit;
	char tmp2[100], prefix2[32];
	char env_unit[32];
#endif
	int end_wan_sbstate = WAN_STOPPED_REASON_NONE;

#ifdef RTCONFIG_MULTISERVICE_WAN
	if(dualwan_unit__nonusbif(unit) && nvram_match("switch_wantag", "none"))
	{
		if(unit < WAN_UNIT_MAX && unit > WAN_UNIT_NONE) { //GENERIC WAN
			int i = 1;
			int mswan_unit;
			for(i = 1; i < WAN_MULTISRV_MAX; i++) {
				mswan_unit = get_ms_wan_unit(unit, i);
				snprintf(tmp, sizeof(tmp), "wan%d_enable", mswan_unit);
				if (nvram_get(tmp))
					stop_wan_if(mswan_unit);
			}
		}
#ifdef RTCONFIG_MULTIWAN_IF
		if(MULTI_WAN_START_IDX <= unit && unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM) { //GENERIC WAN
			int i = 1;
			int mswan_unit;
			for(i = 1; i < WAN_MULTISRV_MAX; i++) {
				mswan_unit = get_ms_wan_unit(unit, i);
				snprintf(tmp, sizeof(tmp), "wan%d_enable", mswan_unit);
				if (nvram_get(tmp))
					stop_wan_if(mswan_unit);
			}
		}
#endif
	}
#endif

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	if(get_wan_sbstate(unit) == WAN_STOPPED_REASON_DATALIMIT)
		end_wan_sbstate = WAN_STOPPED_REASON_DATALIMIT;
#endif

	update_wan_state(prefix, WAN_STATE_STOPPING, end_wan_sbstate);

	/* Backup active wan_proto for later restore, if it have been updated by ui */
	snprintf(active_proto, sizeof(active_proto), "%s", nvram_safe_get(strcat_r(prefix, "proto", tmp)));

	/* Set previous wan_proto as active */
	snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix, "proto_t", tmp)));
	if (*wan_proto && strcmp(active_proto, wan_proto) != 0) {
#ifdef RTCONFIG_MULTIWAN_IF
		stop_mtwan_iQos(unit);
#else
		stop_iQos(); // clean all tc rules
#endif
		_dprintf("%s %sproto_t=%s\n", __FUNCTION__, prefix, wan_proto);
		nvram_set(strcat_r(prefix, "proto", tmp), wan_proto);
		nvram_unset(strcat_r(prefix, "proto_t", tmp));
	}

	snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

	// Handel for each interface
	if(unit == wan_primary_ifunit()){
		killall_tk("stats");
#ifndef RTCONFIG_NTPD
		killall_tk("ntpclient");
#endif

		/* Shutdown and kill all possible tasks */
#if 0
		killall_tk("ip-up");
		killall_tk("ip-down");
		killall_tk("ip-pre-up");
#ifdef RTCONFIG_IPV6
		killall_tk("ipv6-up");
		killall_tk("ipv6-down");
#endif
		killall_tk("auth-fail");
#endif

#ifdef RTCONFIG_MULTICAST_IPTV
		if (nvram_get_int("switch_stb_x") > 6 && unit == WAN_UNIT_IPTV)
#endif
		stop_igmpproxy();
	}

#ifdef RTCONFIG_MULTISERVICE_WAN
	if(unit < WAN_UNIT_MAX && unit > WAN_UNIT_NONE) //GENERIC WAN
#endif
	{
#ifdef RTCONFIG_OPENVPN
	stop_ovpn_eas();
#endif

#ifdef RTCONFIG_VPNC
	/* Stop VPN client */
	stop_vpnc();
#endif
	}

	switch (get_wan_proto(prefix)) {
	case WAN_L2TP:
		kill_pidfile_tk("/var/run/l2tpd.pid");
		usleep(1000*1000);
		break;
#ifdef RTCONFIG_MULTIWAN_IF
	case WAN_PPPOE:
		wan6_down(wan_ifname);
		break;
#endif
#ifdef RTCONFIG_SOFTWIRE46
	case WAN_LW4O6:
	case WAN_MAPE:
	case WAN_V6PLUS:
	case WAN_OCNVC:
	case WAN_DSLITE:
		wan6_down(wan_ifname);
		break;
#endif
	}

	/* Stop pppd */
	stop_pppd(unit);

#ifdef RTCONFIG_MULTI_PPP
	if (is_mtppp_base_unit(unit)) {
		int i = 1;
		for(i = 1; i < WAN_MULTIPPP_MAX; i++) {
			stop_pppd(get_mtppp_wan_unit(unit, i));
		}
	}
#endif

	/* Stop post-authenticator */
	stop_auth(unit, 1);

	/* Stop dhcp client */
	stop_udhcpc(unit);

	/* Stop pre-authenticator */
	stop_auth(unit, 0);

#if 1
	/* Clean WAN interface */
	snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	if (*wan_ifname && *wan_ifname != '/') {
#ifdef RTCONFIG_IPV6
		disable_ipv6(wan_ifname);
#endif
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	}
#else
	/* Bring down WAN interfaces */
	// Does it have to?
	snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
#ifdef RTCONFIG_USB_MODEM
	if(strncmp(wan_ifname, "/dev/tty", 8))
#endif
	{
		if(strlen(wan_ifname) > 0){
#ifdef RTCONFIG_SOC_IPQ40XX
			if (strcmp(wan_ifname, "eth0") == 0)
				ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
			else
#elif defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
			if (strcmp(wan_ifname, "eth5") == 0)
				ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
			else
#endif
			ifconfig(wan_ifname, 0, NULL, NULL);
#ifdef RTCONFIG_RALINK
#elif defined(RTCONFIG_QCA)
#else
			if(!strncmp(wan_ifname, "eth", 3) || !strncmp(wan_ifname, "vlan", 4))
				ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
#endif
		}
	}
#endif

#ifdef RTCONFIG_DSL_REMOTE
	if (get_dsl_prefix_by_wan_unit(unit, dsl_prefix, sizeof(dsl_prefix)) == 0)
	{
		if (nvram_pf_match(dsl_prefix, "proto", "ipoa"))
			stop_ipoa();
	}
#endif

	if (!strcmp(wan_proto, "bridge")) {
#ifdef RTCONFIG_MULTILAN_CFG
		if (is_bridged(nvram_safe_get("lan_ifname"), wan_ifname)) {
			if (nvram_match("switch_wantag", "hinet_mesh")) {
				killall_tk("dhcpfwd");
			}
		}
		stop_dhcpfilter(wan_ifname);
		del_from_bridge(wan_ifname);
#else
#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_DSL_BCM
		if (nvram_get_int("switch_stb_x")) {
			config_wan_bridge(STB_BR_IF, wan_ifname, 0);
		}
		else {
			eval("brctl", "delif", nvram_safe_get("lan_ifname"), wan_ifname);
		}
		stop_dhcpfilter(wan_ifname);
#else
		if (nvram_get_int("wan2lan")) {
			config_wan_bridge(nvram_safe_get("lan_ifname"), wan_ifname, 0);
		}
		else {
			config_wan_bridge(STB_BR_IF, wan_ifname, 0);
		}
#endif
#else
		stop_dhcpfilter(wan_ifname);
		eval("brctl", "delif", nvram_safe_get("lan_ifname"), wan_ifname);
		if (nvram_match("switch_wantag", "hinet_mesh")) {
			killall_tk("dhcpfwd");
		}
#endif
#endif
	}

#ifdef RTCONFIG_USB_MODEM
	if (dualwan_unit__usbif(unit)) {
#ifdef RTCONFIG_USB_BECEEM
		if(is_usb_modem_ready(get_dualwan_by_unit(unit)) == 1){
			if(pids("wimaxd"))
				eval("wimaxc", "disconnect");
		}

		if(pids("wimaxd")){
			killall("wimaxd", SIGTERM);
			killall("wimaxd", SIGUSR1);
		}

		uvid = nvram_get_int("usb_modem_act_vid");
		upid = nvram_get_int("usb_modem_act_pid");

		if(is_samsung_dongle(1, uvid, upid)){
			i = 0;
			while(i < 3){
				if(pids("madwimax")){
					killall_tk("madwimax");
					sleep(1);

					++i;
				}
				else
					break;
			}

			modprobe_r("tun");

			nvram_set(strcat_r(prefix, "ifname", tmp), "");
		}
		else if(is_gct_dongle(1, uvid, upid)){
			i = 0;
			while(i < 3){
				if(pids("gctwimax")){
					killall_tk("gctwimax");
					sleep(1);

					++i;
				}
				else
					break;
			}
			unlink(WIMAX_CONF);

			modprobe_r("tun");

			nvram_set(strcat_r(prefix, "ifname", tmp), "");
		}
#endif	/* RTCONFIG_USB_BECEEM */

#ifdef RTCONFIG_INTERNAL_GOBI
		modem_unit = get_modemunit_by_type(get_dualwan_by_unit(unit));
		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
		snprintf(env_unit, sizeof(env_unit), "unit=%d", modem_unit);

		if(is_builtin_modem(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)))){
			putenv(env_unit);
			char *const modem_argv[] = {"/usr/sbin/modem_stop.sh", NULL};
			_eval(modem_argv, ">>/tmp/usb.log", 0, NULL);
			unsetenv("unit");
		}
#endif
	}

#ifdef RTCONFIG_GETREALIP
#ifdef RTCONFIG_DUALWAN
	if(nvram_invmatch("wans_mode", "lb"))
#endif
	{
		nvram_set(strcat_r(prefix, "realip_state", tmp), "0");
		nvram_set(strcat_r(prefix, "realip_ip", tmp), "");
	}
#endif

	if(dualwan_unit__usbif(unit))
		update_wan_state(prefix, WAN_STATE_INITIALIZING, end_wan_sbstate);
	else
#endif // RTCONFIG_USB_MODEM
		update_wan_state(prefix, WAN_STATE_STOPPED, end_wan_sbstate);

	// wait for release finished ?
#ifdef RTCONFIG_MULTISERVICE_WAN
	if(unit < WAN_UNIT_MAX && unit > WAN_UNIT_NONE) //GENERIC WAN
#endif
	if (!g_reboot)
		sleep(2);

	/* Restore active wan_proto value */
	_dprintf("%s %sproto=%s\n", __FUNCTION__, prefix, active_proto);
	nvram_set(strcat_r(prefix, "proto", tmp), active_proto);
}


void wan_add_resolv_conf(FILE* fp, int wan_unit)
{
	char wan_prefix[16] = {0};
	char wan_dns_buf[256];
	char wan_dns[64], *next_dns;

	if (!fp)
		return;
	if (wan_unit < 0)
		return;

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
	strlcpy(wan_dns_buf, nvram_pf_safe_get(wan_prefix, "dns"), sizeof(wan_dns_buf));
	if (wan_dns_buf[0]) {
		foreach (wan_dns, wan_dns_buf, next_dns)
			fprintf(fp, "nameserver %s\n", wan_dns);
	}
	else {
		strlcpy(wan_dns_buf, nvram_pf_safe_get(wan_prefix, "xdns"), sizeof(wan_dns_buf));
		if (wan_dns_buf[0]) {
			foreach (wan_dns, wan_dns_buf, next_dns)
				fprintf(fp, "nameserver %s\n", wan_dns);
		}
	}
}

void wan_add_resolv_dnsmasq(FILE* fp, int wan_unit)
{
	char wan_prefix[16] = {0};
	char wan_dns_buf[256], wan_domain_buf[256];
	char wan_dns[64], wan_domain[64], *next_dns, *next_domain;

	if (!fp)
		return;
	if (wan_unit < 0)
		return;

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
	strlcpy(wan_dns_buf, nvram_pf_safe_get(wan_prefix, "dns"), sizeof(wan_dns_buf));
	if (wan_dns_buf[0]) {
		foreach (wan_dns, wan_dns_buf, next_dns)
		{
			fprintf(fp, "server=%s\n", wan_dns);
#ifdef RTCONFIG_YANDEXDNS
			fprintf(fp, "server=/%s/%s\n", "local", wan_dns);
#endif
		}
	}
	else {
		strlcpy(wan_dns_buf, nvram_pf_safe_get(wan_prefix, "xdns"), sizeof(wan_dns_buf));
		if (wan_dns_buf[0]) {
			foreach (wan_dns, wan_dns_buf, next_dns)
			{
				fprintf(fp, "server=%s\n", wan_dns);
#ifdef RTCONFIG_YANDEXDNS
				fprintf(fp, "server=/%s/%s\n", "local", wan_dns);
#endif
			}
		}
	}
	strlcpy(wan_domain_buf, nvram_pf_safe_get(wan_prefix, "domain"), sizeof(wan_domain_buf));
	if (wan_domain_buf[0]) {
		foreach (wan_dns, wan_dns_buf, next_dns)
			foreach(wan_domain, wan_domain_buf, next_domain)
				fprintf(fp, "server=/%s/%s\n", wan_domain, wan_dns);
	}
	else {
		strlcpy(wan_domain_buf, nvram_pf_safe_get(wan_prefix, "xdomain"), sizeof(wan_domain_buf));
		if (wan_domain_buf[0]) {
			foreach (wan_dns, wan_dns_buf, next_dns)
				foreach(wan_domain, wan_domain_buf, next_domain)
					fprintf(fp, "server=/%s/%s\n", wan_domain, wan_dns);
		}
	}
}

#ifdef RTCONFIG_IPV6
void wan_add_resolv_dnsmasq_ipv6(FILE* fp, int wan6_unit)
{
	char wan_dns_buf[256], wan_domain_buf[256];
	char wan_dns[64], wan_domain[64], *next_dns, *next_domain;

	if (!fp)
		return;
	if (wan6_unit < 0)
		return;

	if (nvram_get_int(ipv6_nvname_by_unit("ipv6_dnsenable", wan6_unit))) {
		strlcpy(wan_dns_buf, nvram_safe_get(ipv6_nvname_by_unit("ipv6_get_dns", wan6_unit)), sizeof(wan_dns_buf));
		foreach (wan_dns, wan_dns_buf, next_dns)
			fprintf(fp, "server=%s\n", wan_dns);
		strlcpy(wan_domain_buf, nvram_safe_get(ipv6_nvname_by_unit("ipv6_get_domain", wan6_unit)), sizeof(wan_domain_buf));
		if (wan_domain_buf[0]) {
			foreach (wan_dns, wan_dns_buf, next_dns)
				foreach(wan_domain, wan_domain_buf, next_domain)
					fprintf(fp, "server=/%s/%s\n", wan_domain, wan_dns);
		}
	}
	else {
		char nvname[16];
		int n;
		for (n = 1; n <= 3; n++) {
			snprintf(nvname, sizeof(nvname), "ipv6_dns%d", n);
			strlcpy(wan_dns, nvram_safe_get(ipv6_nvname_by_unit(nvname, wan6_unit)), sizeof(wan_dns));
			if (wan_dns[0]) {
				fprintf(fp, "server=%s\n", wan_dns);
			}
		}
	}
}
#endif

int update_resolvconf(void)
{
	FILE *fp, *fp_servers;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char *wan_dns, *wan_domain, *next;
	char wan_dns_buf[INET6_ADDRSTRLEN*3 + 3], wan_domain_buf[256];
	char *wan_xdns, *wan_xdomain;
	char wan_xdns_buf[sizeof("255.255.255.255 ")*2], wan_xdomain_buf[256];
	char domain[64], *next_domain;
	int primary_unit = wan_primary_ifunit();
	int unit, lock;
#ifdef RTCONFIG_YANDEXDNS
	int yadns_mode = nvram_get_int("yadns_enable_x") ? nvram_get_int("yadns_mode") : YADNS_DISABLED;
#endif
#ifdef RTCONFIG_DNSPRIVACY
	int dnspriv_enable = nvram_get_int("dnspriv_enable");
#endif

#if defined(RTCONFIG_VPNC) && !defined(RTCONFIG_VPN_FUSION)
	if (is_vpnc_dns_active())
		return 0;
#endif

	lock = file_lock("resolv");

	if (!(fp = fopen("/tmp/resolv.conf", "w+"))) {
		perror("/tmp/resolv.conf");
		goto error;
	}
	if (!(fp_servers = fopen("/tmp/resolv.dnsmasq", "w+"))) {
		perror("/tmp/resolv.dnsmasq");
		fclose(fp);
		goto error;
	}
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
	if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
		ipv6_enabled() &&
		nvram_match(ipv6_nvname("ipv6_only"), "1"))
		goto NOIP;
#endif

#if defined(RTCONFIG_OPENVPN) && (!defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_VPN_FUSION_MERLIN))
	write_ovpn_resolv_dnsmasq(fp_servers);
#endif
	{
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; unit++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			wan_dns = nvram_safe_get_r(strcat_r(prefix, "dns", tmp), wan_dns_buf, sizeof(wan_dns_buf));
			wan_xdns = nvram_safe_get_r(strcat_r(prefix, "xdns", tmp), wan_xdns_buf, sizeof(wan_xdns_buf));

			if (!*wan_dns && !*wan_xdns)
				continue;

#ifdef RTCONFIG_DUALWAN
			/* skip disconnected WANs in LB mode */
			if (nvram_match("wans_mode", "lb")) {
				if (!is_phy_connect2(unit))
					continue;
			} else
			/* skip non-primary WANs except not fully connected in FB mode */
			if (nvram_match("wans_mode", "fb")) {
				if (unit != primary_unit && *wan_dns)
					continue;
			} else
#endif
			/* skip non-primary WANs */
			if (unit != primary_unit)
					continue;

			foreach(tmp, (*wan_dns ? wan_dns : wan_xdns), next)
				fprintf(fp, "nameserver %s\n", tmp);

			do {
#ifdef RTCONFIG_YANDEXDNS
				if (yadns_mode != YADNS_DISABLED)
					break;
#endif
#ifdef RTCONFIG_DNSPRIVACY
				if (dnspriv_enable)
					break;
#endif
//#if defined(RTCONFIG_WIREGUARD) && !defined(RTCONFIG_VPN_FUSION)
//				if (write_wgc_resolv_dnsmasq(fp_servers))
//					break;
//#endif
#if defined(RTCONFIG_IPSEC) && (!defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_VPN_FUSION_MERLIN))
				if (write_ipc_resolv_dnsmasq(fp_servers))
					break;
#endif
#ifdef RTCONFIG_DUALWAN
				/* Skip not fully connected WANs in LB mode */
				if (nvram_match("wans_mode", "lb") && !*wan_dns)
					break;
#endif
#ifdef RTCONFIG_OPENVPN
				/* We have a client with DNS set to Exclusive and routing set to All */
				if (ovpn_skip_dnsmasq())
					break;
#endif
#if defined(RTCONFIG_MULTILAN_CFG) && defined(RTCONFIG_MULTIWAN_IF)
				if (write_sdnlan_resolv_dnsmasq(fp_servers))
					break;
#endif
				foreach(tmp, (*wan_dns ? wan_dns : wan_xdns), next)
				{
					fprintf(fp_servers, "server=%s\n", tmp);
				}
			} while (0);

			wan_domain = nvram_safe_get_r(strcat_r(prefix, "domain", tmp), wan_domain_buf, sizeof(wan_domain_buf));
			foreach (tmp, wan_dns, next) {
				foreach(domain, wan_domain, next_domain)
					fprintf(fp_servers, "server=/%s/%s\n", domain, tmp);
#ifdef RTCONFIG_YANDEXDNS
				if (yadns_mode != YADNS_DISABLED)
					fprintf(fp_servers, "server=/%s/%s\n", "local", tmp);
#endif
			}

			wan_xdomain = nvram_safe_get_r(strcat_r(prefix, "xdomain", tmp), wan_xdomain_buf, sizeof(wan_xdomain_buf));
			foreach (tmp, wan_xdns, next) {
				int new = (find_word(wan_dns, tmp) == NULL);
				foreach (domain, wan_xdomain, next_domain) {
					if (new || find_word(wan_domain, domain) == NULL)
						fprintf(fp_servers, "server=/%s/%s\n", domain, tmp);
				}
#ifdef RTCONFIG_YANDEXDNS
				if (yadns_mode != YADNS_DISABLED && new)
					fprintf(fp_servers, "server=/%s/%s\n", "local", tmp);
#endif
			}
		}

#ifdef RTCONFIG_MULTISERVICE_WAN
		for (unit = 1; unit < WAN_MULTISRV_MAX; unit++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", get_ms_wan_unit(primary_unit, unit));
			if (nvram_get_int(strcat_r(prefix, "enable", tmp)) == 0)
				continue;
			wan_dns = nvram_safe_get_r(strcat_r(prefix, "dns", tmp), wan_dns_buf, sizeof(wan_dns_buf));
			wan_xdns = nvram_safe_get_r(strcat_r(prefix, "xdns", tmp), wan_xdns_buf, sizeof(wan_xdns_buf));

			if (!*wan_dns && !*wan_xdns)
				continue;

			foreach(tmp, (*wan_dns ? wan_dns : wan_xdns), next) {
				fprintf(fp, "nameserver %s\n", tmp);
				fprintf(fp_servers, "server=%s\n", tmp);
#ifdef RTCONFIG_YANDEXDNS
				if (yadns_mode != YADNS_DISABLED)
					fprintf(fp_servers, "server=/%s/%s\n", "local", tmp);
#endif
			}
		}
#endif
	}

#ifdef RTCONFIG_YANDEXDNS
	if (yadns_mode != YADNS_DISABLED) {
		char *server[2];
		int count = get_yandex_dns(AF_INET, yadns_mode, server, sizeof(server)/sizeof(server[0]));
		for (unit = 0; unit < count; unit++) {
			fprintf(fp_servers, "server=%s\n", server[unit]);
			fprintf(fp_servers, "server=%s#%u\n", server[unit], YADNS_DNSPORT);
		}
	}
#endif
#ifdef RTCONFIG_DNSPRIVACY
	if (dnspriv_enable) {
		if (!nvram_get_int("dns_local_cache"))
			fprintf(fp, "nameserver %s\n", "127.0.1.1");
		fprintf(fp_servers, "server=%s\n", "127.0.1.1");
	}
#endif

#if (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
NOIP:
#endif
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && is_routing_enabled()) {
		struct in6_addr addr;

	/* TODO: Skip unconnected wan */

		switch (get_ipv6_service()) {
		case IPV6_NATIVE_DHCP:
#ifdef RTCONFIG_6RELAYD
		case IPV6_PASSTHROUGH:
#endif
			if (nvram_get_int(ipv6_nvname("ipv6_dnsenable"))) {
				wan_dns = nvram_safe_get_r(ipv6_nvname("ipv6_get_dns"), wan_dns_buf, sizeof(wan_dns_buf));
				wan_domain = nvram_safe_get_r(ipv6_nvname("ipv6_get_domain"), wan_domain_buf, sizeof(wan_domain_buf));
#if 0
				//FIXME: workaround, as odhcp6c cannot receive DNS info due to no IA_NA in 'Advertise' packets.
				snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());
				if (get_wan_proto(prefix) == WAN_V6PLUS &&
				    nvram_get_int("s46_hgw_case") == S46_CASE_MAP_HGW_OFF &&
				    !strcmp(wan_dns_buf, "")) {
					wan_dns = strcpy(wan_dns_buf, "2404:1a8:7f01:b::3 2404:1a8:7f01:a::3");
				}
#endif
				break;
			}
			/* fall through */
		default:
			wan_dns = strcpy(wan_dns_buf, "");
			wan_domain = "";
			for (unit = 1; unit <= 3; unit++) {
				snprintf(tmp, sizeof(tmp), "ipv6_dns%d", unit);
				if (*wan_dns_buf)
					strlcat(wan_dns_buf, " ", sizeof(wan_dns_buf));
				strlcat(wan_dns_buf, nvram_safe_get(ipv6_nvname(tmp)), sizeof(wan_dns_buf));
			}
		}

		foreach(tmp, wan_dns, next) {
			if (inet_pton(AF_INET6, tmp, &addr) <= 0)
				continue;
			foreach(domain, wan_domain, next_domain)
				fprintf(fp_servers, "server=/%s/%s\n", domain, tmp);
#ifdef RTCONFIG_YANDEXDNS
			if (yadns_mode != YADNS_DISABLED) {
				fprintf(fp_servers, "server=/%s/%s\n", "local", tmp);
				continue;
			}
#endif
#ifdef RTCONFIG_DNSPRIVACY
			if (dnspriv_enable)
				continue;
#endif
			fprintf(fp, "nameserver %s\n", tmp);
			fprintf(fp_servers, "server=%s\n", tmp);
		}

#ifdef RTCONFIG_YANDEXDNS
		if (yadns_mode != YADNS_DISABLED) {
			char *server[2];
			int count = get_yandex_dns(AF_INET6, yadns_mode, server, sizeof(server)/sizeof(server[0]));
			for (unit = 0; unit < count; unit++) {
				fprintf(fp_servers, "server=%s\n", server[unit]);
				fprintf(fp_servers, "server=%s#%u\n", server[unit], YADNS_DNSPORT);
			}
		}
#endif
	}
#endif

	fclose(fp);
	fclose(fp_servers);

#ifdef RTCONFIG_MULTIWAN_PROFILE
	mtwan_append_group_main_resolvconf(primary_unit);
#endif

#if defined(RTCONFIG_MULTIWAN_IF) && defined(RTCONFIG_SOFTWIRE46)
	mtwan_append_s46_resolvconf();
#endif

	file_unlock(lock);

#if defined(RTCONFIG_VPN_FUSION) && !defined(RTCONFIG_VPN_FUSION_MERLIN)
	if (is_vpnc_dns_active())
		vpnc_update_resolvconf(nvram_get_int("vpnc_default_wan"));
#endif

#ifdef RTCONFIG_OPENVPN
	if (ovpn_need_dnsmasq_restart())
#ifdef RTCONFIG_MULTILAN_CFG
		start_dnsmasq(ALL_SDN);	// add strict-order
#else
		start_dnsmasq();	// add strict-order
#endif
	else
#endif
#ifdef RTCONFIG_MULTILAN_CFG
		reload_dnsmasq(LAN_IN_SDN_IDX);
#else
		reload_dnsmasq();
#endif
	return 0;

error:
	file_unlock(lock);
	return -1;
}

/* List of IP address blocks which are private / reserved and therefore not suitable for public external IP addresses */
/* If interface has IP address from one of this block, then it is either behind NAT or port forwarding is impossible */
#define IP(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define MSK(m) (32-(m))
static const struct { uint32_t address; uint32_t rmask; } reserved[] = {
        { IP(  0,   0,   0, 0), MSK( 8) }, /* RFC1122 "This host on this network" */
        { IP( 10,   0,   0, 0), MSK( 8) }, /* RFC1918 Private-Use */
        { IP(100,  64,   0, 0), MSK(10) }, /* RFC6598 Shared Address Space */
        { IP(127,   0,   0, 0), MSK( 8) }, /* RFC1122 Loopback */
        { IP(169, 254,   0, 0), MSK(16) }, /* RFC3927 Link-Local */
        { IP(172,  16,   0, 0), MSK(12) }, /* RFC1918 Private-Use */
        { IP(192,   0,   0, 0), MSK(24) }, /* RFC6890 IETF Protocol Assignments */
        { IP(192,   0,   2, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-1) */
        { IP(192,  31, 196, 0), MSK(24) }, /* RFC7535 AS112-v4 */
        { IP(192,  52, 193, 0), MSK(24) }, /* RFC7450 AMT */
        { IP(192,  88,  99, 0), MSK(24) }, /* RFC7526 6to4 Relay Anycast */
        { IP(192, 168,   0, 0), MSK(16) }, /* RFC1918 Private-Use */
        { IP(192, 175,  48, 0), MSK(24) }, /* RFC7534 Direct Delegation AS112 Service */
        { IP(198,  18,   0, 0), MSK(15) }, /* RFC2544 Benchmarking */
        { IP(198,  51, 100, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-2) */
        { IP(203,   0, 113, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-3) */
        { IP(224,   0,   0, 0), MSK( 4) }, /* RFC1112 Multicast */
        { IP(240,   0,   0, 0), MSK( 4) }, /* RFC1112 Reserved for Future Use + RFC919 Limited Broadcast */
};
#undef IP
#undef MSK

int
addr_is_reserved(struct in_addr * addr)
{
        uint32_t address = ntohl(addr->s_addr);
        size_t i;

        for (i = 0; i < sizeof(reserved)/sizeof(reserved[0]); ++i) {
                if ((address >> reserved[i].rmask) == (reserved[i].address >> reserved[i].rmask))
                        return 1;
        }

        return 0;
}

#ifdef RTCONFIG_IPV6
void wan6_up(const char *pwan_ifname)
{
	char addr6[INET6_ADDRSTRLEN + 4];
	char gateway[INET6_ADDRSTRLEN];
	char wan_ifname[16];
	char prefix[sizeof("wanXXXXXXXXXX_")];
	struct in_addr addr4;
	struct in6_addr addr;
	int mtu, service, accept_defrtr;
	int wan_unit;

	if (!pwan_ifname || *pwan_ifname == '\0')
		return;

	/* Value of pwan_ifname can be modfied after do_dns_detect */
	strlcpy(wan_ifname, pwan_ifname, sizeof(wan_ifname));
	wan_unit = get_wan6_unit(wan_ifname);
	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

	service = get_ipv6_service_by_unit(wan_unit);
	switch (service) {
	case IPV6_NATIVE_DHCP:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
#ifdef RTCONFIG_MULTIWAN_IF
		accept_defrtr = nvram_get_int(ipv6_nvname_by_unit("ipv6_accept_defrtr", wan_unit));
#else
		accept_defrtr = service == IPV6_NATIVE_DHCP && /* limit to native by now */
				nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp") ?
				nvram_get_int(ipv6_nvname_by_unit("ipv6_accept_defrtr", wan_unit)) : 1;
#endif
		ipv6_sysconf(wan_ifname, "accept_ra", 1);
		ipv6_sysconf(wan_ifname, "accept_ra_defrtr", accept_defrtr);
		ipv6_sysconf(wan_ifname, "forwarding", 0);
		break;
	case IPV6_MANUAL:
		ipv6_sysconf(wan_ifname, "accept_ra", 0);
		ipv6_sysconf(wan_ifname, "forwarding", 1);
		break;
	case IPV6_6RD:
		update_6rd_info_by_unit(wan_unit);
		break;
	case IPV6_DISABLED:
		return;
	}

	set_intf_ipv6_dad(wan_ifname, 0, 1);

	switch (service) {
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#ifdef RTCONFIG_MULTIWAN_IF
		start_mtwan_6relayd(wan_unit);
#else
		start_6relayd();
#endif
		/* fall through */
#endif
	case IPV6_NATIVE_DHCP:
#ifdef RTCONFIG_MULTIWAN_IF
		start_mtwan_rdisc6(wan_unit);
		start_mtwan_dhcp6c(wan_unit);
		add_mtwan_ipv6_route(wan_unit, wan_ifname, service);
#else
		start_rdisc6();
		start_dhcp6c();

		if (nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp")) {
			strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_llremote", wan_unit)), sizeof(gateway));
			if (/* gateway && */ *gateway)
				_ipv6_route_add(wan_ifname, 0, "::/0", gateway, RTF_DEFAULT | RTF_ADDRCONF);
		}
#endif

		/* propagate ipv6 mtu */
		mtu = ipv6_getconf(wan_ifname, "mtu");
		if (mtu) {
			ipv6_sysconf(nvram_safe_get("lan_ifname"), "mtu", mtu);
#ifdef RTCONFIG_MULTILAN_CFG
			set_sdn_ipv6_mtu(wan_unit, mtu);
#endif
		}
		break;

	case IPV6_MANUAL:
		if (nvram_match(ipv6_nvname_by_unit("ipv6_ipaddr", wan_unit), (char*)ipv6_router_address(NULL))) {
			dbG("WAN IPv6 address is the same as LAN IPv6 address!\n");
			break;
		}
		snprintf(addr6, sizeof(addr6), "%s/%d", nvram_safe_get(ipv6_nvname_by_unit("ipv6_ipaddr", wan_unit)), nvram_get_int(ipv6_nvname_by_unit("ipv6_prefix_len_wan", wan_unit)));
		eval("ip", "-6", "addr", "add", addr6, "dev", (char *)wan_ifname);

#ifdef RTCONFIG_MULTIWAN_IF
		add_mtwan_ipv6_route(wan_unit, wan_ifname, service);
		update_mtwan_ip_rule_ipv6(wan_unit);
#else
		eval("ip", "-6", "route", "del", "::/0");

		strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", wan_unit)), sizeof(gateway));
		if (/* gateway && */ *gateway) {
			eval("ip", "-6", "route", "add", gateway, "dev", (char *)wan_ifname, "metric", "1");
			eval("ip", "-6", "route", "add", "::/0", "via", gateway, "dev", (char *)wan_ifname, "metric", "1");
		} else if (nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp")) {
			strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_llremote", wan_unit)), sizeof(gateway));
			if (/* gateway && */ *gateway)
				_ipv6_route_add(wan_ifname, 0, "::/0", gateway, RTF_DEFAULT | RTF_ADDRCONF);
		}
#endif

		/* propagate ipv6 mtu */
		mtu = ipv6_getconf(wan_ifname, "mtu");
		if (mtu) {
			ipv6_sysconf(nvram_safe_get("lan_ifname"), "mtu", mtu);
#ifdef RTCONFIG_MULTILAN_CFG
			set_sdn_ipv6_mtu(wan_unit, mtu);
#endif
		}

#ifdef RTCONFIG_SOFTWIRE46
		int wan_proto = -1;
		long rsp_code = 0;
		switch (wan_proto = get_wan_proto(prefix)) {
			char peerbuf[INET6_ADDRSTRLEN];
			char addr6buf[INET6_ADDRSTRLEN];
			char addr4buf[INET_ADDRSTRLEN + sizeof("/32")];
			char *rules, *type;
			char tmp[100];
			int prefix4len, ealen, offset, psidlen, psid, draft;
		case WAN_LW4O6:
			if (nvram_get_int(strcat_r(prefix, "dhcpenble_x", tmp)))
				break;
			type = "lw4o6";
			prefix4len = 32;
			ealen = -1;
			draft = 0;
			goto s46_maprules;
		case WAN_MAPE:
			if (nvram_get_int(strcat_r(prefix, "dhcpenble_x", tmp)))
				break;
			type = "map-e";
			prefix4len = nvram_get_int(strcat_r(prefix, "s46_prefix4len_x", tmp));
			ealen = nvram_get_int(strcat_r(prefix, "s46_ealen_x", tmp));
			draft = 0;
			goto s46_maprules;
		case WAN_V6PLUS:
			draft = 1;
			if (nvram_get_int(strcat_r(prefix, "dhcpenble_x", tmp))) {
				rules = s46_jpne_maprules(NULL, NULL, 0, &rsp_code);
				goto s46_mapcalc;
			}
			type = "map-e";
			prefix4len = nvram_get_int(strcat_r(prefix, "s46_prefix4len_x", tmp));
			ealen = nvram_get_int(strcat_r(prefix, "s46_ealen_x", tmp));
			goto s46_maprules;
		case WAN_OCNVC:
			draft = 1;
			if (nvram_get_int(strcat_r(prefix, "dhcpenble_x", tmp))) {
				rules = s46_ocn_maprules(nvram_safe_get("ipv6_ra_addr"), nvram_get_int("ipv6_ra_length"), &rsp_code);
				goto s46_mapcalc;
			}
			type = "map-e";
			prefix4len = nvram_get_int(strcat_r(prefix, "s46_prefix4len_x", tmp));
			ealen = nvram_get_int(strcat_r(prefix, "s46_ealen_x", tmp));
			goto s46_maprules;
		s46_maprules:
			if (asprintf(&rules,
				     "type=%s,ipv6prefix=%s,prefix6len=%d,ipv4prefix=%s,prefix4len=%d,"
				     "ealen=%d,offset=%d,psidlen=%d,psid=%d,br=%s",
				     type,
				     nvram_safe_get(strcat_r(prefix, "s46_prefix6_x", tmp)),
				     nvram_get_int(strcat_r(prefix, "s46_prefix6len_x", tmp)),
				     nvram_safe_get(strcat_r(prefix, "s46_prefix4_x", tmp)),
				     prefix4len, ealen,
				     nvram_get_int(strcat_r(prefix, "s46_offset_x", tmp)),
				     nvram_get_int(strcat_r(prefix, "s46_psidlen_x", tmp)),
				     nvram_get_int(strcat_r(prefix, "s46_psid_x", tmp)),
				     nvram_safe_get(strcat_r(prefix, "s46_peer_x", tmp))) < 0) {
				rules = NULL;
			}
		s46_mapcalc:
			if (s46_mapcalc(wan_unit, wan_proto, rules, peerbuf, sizeof(peerbuf), addr6buf, sizeof(addr6buf),
					addr4buf, sizeof(addr4buf), &offset, &psidlen, &psid, NULL, draft) <= 0) {
				peerbuf[0] = addr6buf[0] = addr4buf[0] = '\0';
				offset = 0, psidlen = 0, psid = 0;
			}
			free(rules);
			nvram_set(ipv6_nvname_by_unit("ipv6_s46_peer", wan_unit), peerbuf);
			nvram_set(ipv6_nvname_by_unit("ipv6_s46_addr6", wan_unit), addr6buf);
			nvram_set(ipv6_nvname_by_unit("ipv6_s46_addr4", wan_unit), addr4buf);
			nvram_set(ipv6_nvname_by_unit("ipv6_s46_fmrs", wan_unit), "");
			nvram_set_int(ipv6_nvname_by_unit("ipv6_s46_offset", wan_unit), offset);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_s46_psidlen", wan_unit), psidlen);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_s46_psid", wan_unit), psid);
			stop_s46_tunnel(wan_unit, 0);
			start_s46_tunnel(wan_unit);
			break;
		}
#endif

		/* workaround to update ndp entry for now */
		char *ping6_argv[] = {"ping6", "-c", "2", "-I", (char *)wan_ifname, "ff02::1", NULL};
		char *ping6_argv2[] = {"ping6", "-c", "2", gateway, NULL};
		pid_t pid;
		_eval(ping6_argv, NULL, 0, &pid);
		_eval(ping6_argv2, NULL, 0, &pid);
		break;

	case IPV6_6TO4:
	case IPV6_6IN4:
	case IPV6_6RD:
#ifdef RTCONFIG_MULTIWAN_IF
		stop_mtwan_ipv6_tunnel(wan_unit);
#else
		stop_ipv6_tunnel();
#endif
		if (service == IPV6_6TO4) {
			int prefixlen = 16;
			int mask4size = 0;

			/* prefix */
			addr4.s_addr = 0;
			memset(&addr, 0, sizeof(addr));
			inet_aton(nvram_pf_safe_get(prefix, "ipaddr"), &addr4);
			if (addr_is_reserved(&addr4))
				return;
			addr.s6_addr16[0] = htons(0x2002);
			prefixlen = ipv6_mapaddr4(&addr, prefixlen, &addr4, mask4size);
			//addr4.s_addr = htonl(0x00000001);
			//prefixlen = ipv6_mapaddr4(&addr, prefixlen, &addr4, (32 - 16));
			inet_ntop(AF_INET6, &addr, addr6, sizeof(addr6));
			nvram_set(ipv6_nvname_by_unit("ipv6_prefix", wan_unit), addr6);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan_unit), prefixlen);

			/* address */
			addr.s6_addr16[7] |= htons(0x0001);
			inet_ntop(AF_INET6, &addr, addr6, sizeof(addr6));
			nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", wan_unit), addr6);
		}
		else if (service == IPV6_6RD) {
			int prefixlen = nvram_get_int(ipv6_nvname_by_unit("ipv6_6rd_prefixlen", wan_unit));
			int masklen = nvram_get_int(ipv6_nvname_by_unit("ipv6_6rd_ip4size", wan_unit));

			/* prefix */
			addr4.s_addr = 0;
			memset(&addr, 0, sizeof(addr));
			inet_aton(nvram_pf_safe_get(prefix, "ipaddr"), &addr4);
			inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname_by_unit("ipv6_6rd_prefix", wan_unit)), &addr);
			prefixlen = ipv6_mapaddr4(&addr, prefixlen, &addr4, masklen);
			//addr4.s_addr = htonl(0x00000001);
			//prefixlen = ipv6_mapaddr4(&addr, prefixlen, &addr4, (32 - 1));
			inet_ntop(AF_INET6, &addr, addr6, sizeof(addr6));
			nvram_set(ipv6_nvname_by_unit("ipv6_prefix", wan_unit), addr6);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan_unit), prefixlen);

			/* address */
			addr.s6_addr16[7] |= htons(0x0001);
			inet_ntop(AF_INET6, &addr, addr6, sizeof(addr6));
			nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", wan_unit), addr6);
		}
#ifdef RTCONFIG_MULTIWAN_IF
		start_mtwan_ipv6_tunnel(wan_unit);
#else
		start_ipv6_tunnel();
#endif

		/* propagate ipv6 mtu */
		mtu = ipv6_getconf(wan_ifname, "mtu");
		if (mtu) {
			ipv6_sysconf(nvram_safe_get("lan_ifname"), "mtu", mtu);
#ifdef RTCONFIG_MULTILAN_CFG
			set_sdn_ipv6_mtu(wan_unit, mtu);
#endif
		}
		// FIXME: give it a few seconds for DAD completion
		sleep(2);
		break;
	}
#if (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
	if ((wan_unit = wan_ifunit(wan_ifname)) != -1) {
		if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
			ipv6_enabled() &&
			nvram_match(ipv6_nvname("ipv6_only"), "1")) {
			snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
			update_wan_state(prefix, WAN_STATE_CONNECTED, 0);
		}
	}
#endif
#if 0
	start_ecmh(wan_ifname);
#endif
	switch (service) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		start_mldproxy(wan_ifname);
		break;
	}

#ifdef RTCONFIG_HTTPS
	start_httpd_ipv6();
#endif

#ifdef RTCONFIG_OPENVPN
	stop_ovpn_serverall();
	start_ovpn_serverall();
#endif

#ifdef RTCONFIG_GRE
	switch (service) {
	case IPV6_MANUAL:
	case IPV6_6TO4:
	case IPV6_6IN4:
	case IPV6_6RD:
		restart_gre(1);
		break;
	}
#endif
}

void wan6_down(const char *wan_ifname)
{
	int unit;
	char ifname[IFNAMSIZ];

	strlcpy(ifname, wan_ifname, sizeof(ifname));
	unit = get_wan6_unit(ifname);

	set_intf_ipv6_dad(wan_ifname, 0, 0);
	stop_rdisc6();
#if 0
	stop_ecmh();
#endif
	stop_mldproxy();
#ifdef RTCONFIG_MULTIWAN_IF
#ifdef RTCONFIG_6RELAYD
	stop_mtwan_6relayd(unit);
#endif
	stop_mtwan_dhcp6c(unit);
	stop_mtwan_ipv6_tunnel(unit);
#ifdef RTCONFIG_SOFTWIRE46
	stop_s46_tunnel(unit, 1);
#endif
#else
#ifdef RTCONFIG_6RELAYD
	stop_6relayd();
#endif
	stop_dhcp6c();
	stop_ipv6_tunnel();
#ifdef RTCONFIG_SOFTWIRE46
	stop_s46_tunnel(unit, 1);
#endif
#endif

	update_resolvconf();
#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_resolvconf();
#endif
}

void start_wan6(void)
{
	char prefix[sizeof("wanXXXXXXXXXX_")];
	int wan_proto;

	switch (get_ipv6_service()) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit_ipv6());
		wan_proto = get_wan_proto(prefix);
		if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) &&
		    nvram_match(ipv6_nvname("ipv6_ifdev"), "ppp"))
			break;
		/* fall through */
	default:
		// call wan6_up directly
		wan6_up(get_wan6face());
		break;
	}
}

void stop_wan6(void)
{
	// call wan6_down directly
#ifdef RTCONFIG_MULTIWAN_IF
	mtwan_stop_multi_wan6();
#else
	wan6_down(get_wan6face());
#endif
}

#endif

/**
 * Append netdev to bled or remove netdev from bled.
 * @action:	append or move
 * 	0:	remove
 *  otherwise:	append
 * @wan_unit:
 * @wan_ifname:
 */
#ifdef RTCONFIG_BLINK_LED
static void adjust_netdev_if_of_wan_bled(int action, int wan_unit, char *wan_ifname)
{
	char *wan_gpio = "led_wan_gpio";
	int (*func)(const char *led_gpio, const char *ifname);

	if (wan_unit < 0 || wan_unit >= WAN_UNIT_MAX || !wan_ifname)
		return;

	if (action)
		func = append_netdev_bled_if;
	else
		func = remove_netdev_bled_if;
#if defined(RTCONFIG_WANLEDX2)
	if (wan_unit == 1)
		wan_gpio = "led_wan2_gpio";
#endif
	if (dualwan_unit__usbif(wan_unit)) {
		func(wan_gpio, wan_ifname);
		return;
	}

	if (!(nvram_get_int("boardflags") & 0x100))
		return;

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	/* Nothing to do. */
#else
	if (get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_LAN) {
		func(wan_gpio, wan_ifname);
	}
#endif
}
#endif

#ifdef RTCONFIG_HND_ROUTER_AX
int is_starlink_wan(char *gateway)
{
	unsigned int pkt_size = 79; //odd packet size
	unsigned int ping_cnt = 2;
	unsigned int wait_sec = 1;
	double loss_rate = 40.0;
	int ret = -1;

	if(gateway && (strlen(gateway) > 0))
	{
		//Step 01:ping with odd packet size
		ret = ping_target_with_size(gateway, pkt_size, ping_cnt, wait_sec, loss_rate);
		if(ret == 0)
		{
			//Step 02:ping with even packet size if step 01 is failed.
			ret = ping_target_with_size(gateway, pkt_size + 1, ping_cnt, wait_sec, loss_rate);
			if(ret == 1)
			{
				return 1;
			}
		}
	}
	return 0;
}

void modify_tx_pad(char *func_name, char *wan_ifname, int option)
{
	char cmd[128] = {0};

	if(func_name && (strlen(func_name) > 0) && wan_ifname && (strlen(wan_ifname) > 0))
	{
		if(option == 1)
		{
			logmessage(func_name, "Enable tx_pad for Starlink.\n");
			snprintf(cmd, sizeof(cmd), "ethctl %s tx_pad %d", wan_ifname, option);
			system(cmd);
			snprintf(cmd, sizeof(cmd), "%d>%s", option, wan_ifname);
			nvram_set("ethctl_tx_pad", cmd);
		}
		else if(option == 0)
		{
			logmessage(func_name, "Disable tx_pad feature.\n");
			snprintf(cmd, sizeof(cmd), "ethctl %s tx_pad %d", wan_ifname, option);
			system(cmd);
			snprintf(cmd, sizeof(cmd), "%d>%s", option, wan_ifname);
			nvram_set("ethctl_tx_pad", cmd);
		}
		else
		{
			logmessage(func_name, "Wrong tx_pad option.\n");
		}
	}
}

int has2_5Gport(char *wan_ifname)
{
#ifdef RTCONFIG_NEW_PHYMAP
	int count = 0, i = 0;
	phy_port_mapping port_mapping;

	if(wan_ifname && (strlen(wan_ifname) > 0))
	{
		get_phy_port_mapping(&port_mapping);
		count = port_mapping.count;
		for(i = 0; i < count; i++)
		{
			if((port_mapping.port[i].ext_port_id == -1) &&
			   (port_mapping.port[i].max_rate == 2500))
			{
				if(port_mapping.port[i].ifname)
				{
					if(strcmp(wan_ifname, port_mapping.port[i].ifname) == 0)
					{
						return 1;
					}
				}
				else
				{
					if(port_mapping.port[i].cap & (PHY_PORT_CAP_DUALWAN_PRIMARY_WAN || PHY_PORT_CAP_DUALWAN_SECONDARY_WAN))
					{
						return 1;
					}
				}
			}
		}
	}
	return 0;
#else
	return 0;
#endif /* RTCONFIG_NEW_PHYMAP */
}

void starlink_workaround(char *wan_ifname, char *gateway)
{
	if(wan_ifname && (strlen(wan_ifname) > 0) && gateway && (strlen(gateway) > 0))
	{
		if(is_starlink_wan(gateway) == 1)
		{
			modify_tx_pad(__FUNCTION__, wan_ifname, 1);
		}
	}
}
#endif /* RTCONFIG_HND_ROUTER_AX */

void
wan_up(const char *pwan_ifname)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char prefix_x[sizeof("wanXXXXXXXXX_")];
	char wan_ifname[16];
	char gateway[16], dns[PATH_MAX];
	int wan_unit, wan_proto;
#if defined(RTCONFIG_USB_MODEM) && defined(RTCONFIG_INTERNAL_GOBI)
	int modem_unit;
	char tmp2[100], prefix2[32];
	char env_unit[32];
#endif
#ifdef RTCONFIG_LANTIQ
	char ppa_cmd[255] = {0};
#endif
	// FILE *fp;
	char word[100], *next;
#ifdef RTCONFIG_SOFTWIRE46
	char prc[16] = {0};

	prctl(PR_GET_NAME, prc);

	strlcpy(wan_ifname, pwan_ifname, sizeof(wan_ifname));
	if ((wan_unit = wan_ifunit(wan_ifname)) < 0)
		wan_unit = 0;
	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	wan_proto = get_wan_proto(prefix);

	switch (wan_proto) {
		case WAN_V6PLUS:
		case WAN_OCNVC:
		case WAN_DSLITE:
			S46_DBG("Callby:[%s]\n", prc);
		default:
			break;
	}
#endif
	in_addr_t addr, mask;
	int is_private_dns = 0;
	int i=0;
	int first_ntp_sync = 0;

	/* Value of pwan_ifname can be modfied after do_dns_detect */
	strlcpy(wan_ifname, pwan_ifname, sizeof(wan_ifname));
	wan_unit = wan_ifunit(wan_ifname);

#ifdef RTCONFIG_MULTIWAN_IF
	if(is_mtwan_ifname(wan_ifname)
#ifdef RTCONFIG_MULTIWAN_PROFILE
	 && get_wan_state(wan_unit) != WAN_STATE_CONNECTED
#endif
	){
		mtwan_handle_if_updown(wan_unit, wan_ifname, 1);
#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_by_wan(wan_unit);
#endif
#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (!is_mtwan_primary(wan_unit))
#endif
		return;

	}
#endif
	/* Figure out nvram variable name prefix for this i/f */
	if (wan_unit  < 0
#ifdef RTCONFIG_SOFTWIRE46
	    || (nvram_pf_get_int(prefix, "s46_hgw_case") == S46_CASE_MAP_HGW_OFF && !strcmp(prc, "udhcpc_wan"))
#endif
	)
	{
		/* called for dhcp+ppp */
		if ((wan_unit = wanx_ifunit(wan_ifname)) < 0)
			return;
		_dprintf("%s_x(%s)\n", __FUNCTION__, wan_ifname);

		snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
		snprintf(prefix_x, sizeof(prefix_x), "wan%d_x", wan_unit);

#ifdef RTCONFIG_IPV6
		wan_proto = get_wan_proto(prefix);
		if (wan_unit == wan_primary_ifunit_ipv6()) {
			switch (get_ipv6_service()) {
			case IPV6_NATIVE_DHCP:
			case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
			case IPV6_PASSTHROUGH:
#endif
				if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) &&
				    nvram_match(ipv6_nvname("ipv6_ifdev"), "ppp"))
					break;
#ifdef RTCONFIG_SOFTWIRE46
				if (wan_proto == WAN_LW4O6 || wan_proto == WAN_MAPE ||
				    wan_proto == WAN_V6PLUS || wan_proto == WAN_OCNVC ||
				    wan_proto == WAN_DSLITE)
					break;
#endif
				/* fall through */

			default:
				wan6_up(get_wan6_ifname(wan_unit));
				break;
			}
		}
#endif
#ifdef RTCONFIG_DUALWAN
		if (nvram_match("wans_mode", "lb")) {
			int unit = wan_primary_ifunit();

			if (!is_phy_connect(unit))
				unit = 1 - unit;

			if (unit != nvram_get_int("upnp_unit") ||
				strcmp(wan_ifname, nvram_safe_get("upnp_ifname"))) {
				stop_upnp();
				start_upnp();
			}
		}
#endif

		start_firewall(wan_unit, 0);
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
		if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
			ipv6_enabled() &&
			nvram_match(ipv6_nvname("ipv6_only"), "1"))
			return;
#endif
		/* setup static wan routes via physical device */
		add_routes(prefix, "mroute", wan_ifname);

		/* and one supplied via DHCP */
		add_dhcp_routes(prefix_x, wan_ifname, 1);

		/* and default route with metric 1 */
		snprintf(gateway, sizeof(gateway), "%s", nvram_safe_get(strcat_r(prefix_x, "gateway", tmp)));
		if (inet_addr_(gateway) != INADDR_ANY) {
			addr = inet_addr(nvram_safe_get(strcat_r(prefix_x, "ipaddr", tmp)));
			mask = inet_addr(nvram_safe_get(strcat_r(prefix_x, "netmask", tmp)));

			/* the gateway is out of the local network */
			if ((inet_addr(gateway) & mask) != (addr & mask))
				route_add(wan_ifname, 2, gateway, NULL, "255.255.255.255");

			/* default route via default gateway */
			route_add(wan_ifname, 2, "0.0.0.0", gateway, "0.0.0.0");

			/* ... and to dns servers as well for demand ppp to work */
			if (nvram_get_int(strcat_r(prefix, "dnsenable_x", tmp))) {
				snprintf(dns, sizeof(dns), "%s", nvram_safe_get(strcat_r(prefix_x, "dns", tmp)));
				foreach(word, dns, next) {
					if ((inet_addr(word) != inet_addr(gateway)) &&
					    (inet_addr(word) & mask) != (addr & mask))
						route_add(wan_ifname, 2, word, gateway, "255.255.255.255");
				}
			}
		}

		update_resolvconf();
#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_resolvconf();
#endif

		/* start multicast router on DHCP+VPN physical interface */
#ifdef RTCONFIG_MULTISERVICE_WAN
		if (!nvram_match("switch_wantag", "none"))
#endif
		if (nvram_match("iptv_ifname", wan_ifname)
		 || wan_unit == wan_primary_ifunit()
		)
			start_igmpproxy(wan_ifname);

#ifdef RTCONFIG_LANTIQ
		disable_ppa_wan(wan_ifname);
#endif
#ifdef RTCONFIG_MULTIWAN_IF
		mtwan_handle_ip_rule(wan_unit);
#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_by_wan(wan_unit);
#endif
#endif
		_dprintf("%s_x(%s): done.\n", __FUNCTION__, wan_ifname);

		return;
	}

	_dprintf("%s(%s)\n", __FUNCTION__, wan_ifname);

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	wan_proto = get_wan_proto(prefix);
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
	if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
		ipv6_enabled() &&
		nvram_match(ipv6_nvname("ipv6_only"), "1"))
		goto NOIP;
#endif
	snprintf(gateway, sizeof(gateway), "%s", nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
	if (inet_addr_(gateway) == INADDR_ANY)
		memset(gateway, 0, sizeof(gateway));

	/* Set default route to gateway if specified */
	switch (wan_proto) {
	case WAN_DHCP:
	case WAN_STATIC:
#ifdef RTCONFIG_SOFTWIRE46
	case WAN_LW4O6:
	case WAN_MAPE:
	case WAN_V6PLUS:
	case WAN_OCNVC:
	case WAN_DSLITE:
#endif
		/* the gateway is in the local network */
		if (*gateway &&
		    inet_addr_(gateway) != inet_addr_(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)))) {
#ifdef RTCONFIG_MULTICAST_IPTV
			/* Rawny: delete gateway route in IPTV(movistar) case to enable QUAGGA */
			if (nvram_get_int("switch_stb_x") > 6 &&
			    !nvram_match("switch_wantag", "movistar"))
#endif
			route_add(wan_ifname, 0, gateway, NULL, "255.255.255.255");
		}
		/* replaced with add_multi_routes()
		route_add(wan_ifname, 0, "0.0.0.0", gateway, "0.0.0.0"); */
		break;

	case WAN_PPTP:
	case WAN_L2TP:
		/* hack: avoid routing cycles, when both peer and server has the same IP */
		/* delete gateway route as it's no longer needed */
		if (*gateway)
			route_del(wan_ifname, 0, gateway, "0.0.0.0", "255.255.255.255");
	}

	/* Install interface dependent static routes */
	add_wan_routes(wan_ifname);

	nvram_set(strcat_r(prefix, "gw_ifname", tmp), wan_ifname);

	/* setup static wan routes via physical device */
	switch (wan_proto) {
	case WAN_DHCP:
	case WAN_STATIC:
#ifdef RTCONFIG_SOFTWIRE46
	case WAN_LW4O6:
	case WAN_MAPE:
	case WAN_V6PLUS:
	case WAN_OCNVC:
	case WAN_DSLITE:
#endif
		nvram_set(strcat_r(prefix, "xgateway", tmp), strlen(gateway) > 0 ? gateway : "0.0.0.0");
		add_routes(prefix, "mroute", wan_ifname);
		break;
	}

	/* and one supplied via DHCP */
	switch (wan_proto) {
#ifdef RTCONFIG_SOFTWIRE46
	case WAN_LW4O6:
		if (!nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp)))
			break;
		/* fall through */
#endif
	case WAN_DHCP:
		add_dhcp_routes(prefix, wan_ifname, 0);
		break;
	}

	/* add wan dns route via wan interface */
	addr = inet_addr(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
	mask = inet_addr(nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
	nvram_safe_get_r(strcat_r(prefix, "dns", tmp), dns, sizeof(dns));
	foreach(word, dns, next) {
		is_private_dns = is_private_subnet(word) && strcmp(word, nvram_safe_get("wan0_ipaddr")) && strcmp(word, nvram_safe_get("wan1_ipaddr"));
		// skip if is 1. WAN gateway, 2. in WAN subnet 3. in LAN subnet
		if ((inet_addr(word) != inet_addr(gateway)) &&
			(inet_addr(word) & mask) != (addr & mask) && 
			((inet_addr(word) & inet_addr(nvram_safe_get("lan_netmask")))
				!= (inet_addr(nvram_safe_get("lan_ipaddr")) & inet_addr(nvram_safe_get("lan_netmask")))) &&
			!chk_inlan(word)
		)
			route_add(wan_ifname, is_private_dns?0:2, word, gateway, "255.255.255.255");
	}
#if (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
NOIP:
#endif
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
	{
		switch (get_ipv6_service_by_unit(wan_unit)) {
		case IPV6_NATIVE_DHCP:
		case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
		case IPV6_PASSTHROUGH:
#endif
			if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) &&
			    nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp"))
				break;
#ifdef RTCONFIG_SOFTWIRE46
			if (wan_proto == WAN_LW4O6 || wan_proto == WAN_MAPE ||
			    wan_proto == WAN_V6PLUS || wan_proto == WAN_OCNVC ||
			    wan_proto == WAN_DSLITE)
				break;
#endif
			/* fall through */
		default:
			wan6_up(get_wan6_ifname(wan_unit));
			break;
		}
	}
#else
	if (wan_unit == wan_primary_ifunit_ipv6()) {
		switch (get_ipv6_service()) {
		case IPV6_NATIVE_DHCP:
		case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
		case IPV6_PASSTHROUGH:
#endif
			if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) &&
			    nvram_match(ipv6_nvname("ipv6_ifdev"), "ppp"))
				break;
#ifdef RTCONFIG_SOFTWIRE46
			if (wan_proto == WAN_LW4O6 || wan_proto == WAN_MAPE ||
			    wan_proto == WAN_V6PLUS || wan_proto == WAN_OCNVC ||
			    wan_proto == WAN_DSLITE)
				break;
#endif
			/* fall through */
		default:
			wan6_up(get_wan6face());
			break;
		}
	}
#endif
#endif

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6 &&
	    nvram_match("iptv_ifname", wan_ifname)) {
		if (nvram_match("switch_wantag", "maxis_fiber_iptv"))
			route_add(wan_ifname, 0, "172.17.90.1", NULL, "255.255.255.255");
		start_igmpproxy(wan_ifname);
	}
#ifdef RTCONFIG_QUAGGA
	if (wan_unit == WAN_UNIT_IPTV || wan_unit == WAN_UNIT_VOIP) {
		stop_quagga();
		start_quagga();
	}
#endif
#endif

#if defined(RTCONFIG_MULTISERVICE_WAN)
	if (nvram_match("iptv_ifname", wan_ifname) && nvram_match("switch_wantag", "none"))
	{
		if (wan_proto == WAN_BRIDGE && !is_ms_base_unit(wan_unit))
#if defined(RTCONFIG_DSL_BCM)
			if (nvram_get_int("switch_stb_x") == 0)
				start_igmpproxy(wan_ifname);
			else
#endif
			start_igmpproxy(STB_BR_IF);
		else
			start_igmpproxy(wan_ifname);
	}
#endif

#if defined(DSL_N55U) || defined(DSL_N55U_B)
	if(nvram_match("wl0_country_code", "GB")) {
		if(isTargetArea()) {
			system("ATE Set_RegulationDomain_2G SG");
			//system("ATE Set_RegulationDomain_5G SG");
		}
	}
#endif

	/* Set connected state */
	update_wan_state(prefix, WAN_STATE_CONNECTED, 0);

#if defined(RTCONFIG_QCA) || \
		(defined(RTCONFIG_RALINK) && !defined(RTCONFIG_DSL) && !defined(RTN13U))
	reinit_hwnat(wan_unit);
#endif

	ctrl_wan_gro(wan_unit, 0);

	// TODO: handle different lan_ifname?
	start_firewall(wan_unit, 0);
	//start_firewall(wan_ifname, nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
	//	nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ipaddr"));

	/* Start post-authenticator */
	start_auth(wan_unit, 1);

	/* Add dns servers to resolv.conf */
	update_resolvconf();
#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_resolvconf();
#endif

#ifdef RTCONFIG_SOFTWIRE46
	if (wan_hgw_detect(wan_unit, wan_ifname, prc))
		return;
#endif

	/* default route via default gateway */
	add_multi_routes(0, wan_unit);
#if defined(RTCONFIG_WIREGUARD) || defined(RTCONFIG_OPENVPN)
	amvpn_set_kilswitch_rules_all();
#endif

#ifdef RTCONFIG_MULTIWAN_PROFILE
	mtwan_handle_ip_route(wan_unit);
	if (is_mtwan_lb(wan_unit)) {
		mtwan_update_lb_route(wan_unit, 1);
		mtwan_update_lb_iptables(wan_unit, 1);
	}
	else {
		mtwan_update_main_default_route(wan_unit, 1);
	}
#endif

#ifdef RTCONFIG_MULTI_PPP
	add_multi_ppp_routes(wan_unit);
	if (is_mtppp_unit(wan_unit)) {
		return;
	}
#endif

	/* Kick syslog to re-resolve remote server */
	reload_syslogd();

#if defined(RTCONFIG_USB_MODEM) && defined(RTCONFIG_INTERNAL_GOBI)
	if(dualwan_unit__usbif(wan_unit)){
		modem_unit = get_modemunit_by_type(get_dualwan_by_unit(wan_unit));
		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
		snprintf(env_unit, sizeof(env_unit), "unit=%d", modem_unit);

		putenv(env_unit);
		if(is_builtin_modem(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)))){
			nvram_set("freeze_duck", "5");
			eval("/usr/sbin/modem_status.sh", "rate");
			eval("/usr/sbin/modem_status.sh", "band");
			eval("/usr/sbin/modem_status.sh", "operation");
			eval("/usr/sbin/modem_status.sh", "provider");
		}
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		eval("/usr/sbin/modem_status.sh", "get_dataset");
		eval("/usr/sbin/modem_status.sh", "bytes");
#endif
		unsetenv("unit");

		char start_sec[32], *str = file2str("/proc/uptime");
		unsigned int up = atoi(str);

		free(str);
		snprintf(start_sec, sizeof(start_sec), "%u", up);
		nvram_set(strcat_r(prefix2, "act_startsec", tmp2), start_sec);
	}
#endif

#ifdef RTCONFIG_OPENVPN
	stop_ovpn_eas();
#endif

	/* Sync time if not already set, or not running a daemon */
#ifdef RTCONFIG_NTPD
	if (!nvram_get_int("ntp_ready")) {
		first_ntp_sync = 1;
#endif
		refresh_ntpc();
	}

#if defined(RTCONFIG_VPN_FUSION) && !defined(RTCONFIG_VPN_FUSION_MERLIN)
	vpnc_set_internet_policy(1);
#endif

#ifdef RTCONFIG_GRE
	if (wan_unit == wan_primary_ifunit())
		restart_gre(0);
#endif

#if !defined(RTCONFIG_MULTIWAN_CFG)
	if (wan_unit != wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
			|| nvram_match("wans_mode", "lb")
#endif
			)
	{

		/* ntp is set, but it didn't just get set, so ntp_synced didn't already did these */
		if (nvram_get_int("ntp_ready") && !first_ntp_sync) {
#ifdef RTCONFIG_OPENVPN
//#ifndef RTCONFIG_VPN_FUSION
//	start_ovpn_clientall();
//#endif
//	start_ovpn_serverall();
			start_ovpn_eas();
#endif
			stop_ddns();
			start_ddns(NULL);
		}
#ifdef RTCONFIG_WIREGUARD
		reload_wgs_ip_rule();
#endif
#ifdef RTCONFIG_TR069
               if(wan_unit == 0 ){
                       if(!pids("tr069")){
                               if(nvram_get_int("link_wan")){
                                       start_tr();
                               }
                       }
               }
               else if(wan_unit == 1 ){
                       if(!pids("tr069")){
                                if(nvram_get_int("link_wan1")){
                                        start_tr();
                                }
                        }
               }
#endif
#ifdef RTCONFIG_MULTIWAN_IF
		mtwan_handle_ip_rule(wan_unit);
#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_by_wan(wan_unit);
#endif
#endif
		/* Restart DDNS when WAN was restored */
		logmessage("wan_up", "Restart DDNS in load-balance\n");
		stop_ddns();
		start_ddns(NULL);

		return;
	}
#endif

#ifdef RTCONFIG_MULTISERVICE_WAN
	if (!nvram_match("switch_wantag", "none"))
#endif
{
	/* start multicast router when not VPN */
	if (wan_unit == wan_primary_ifunit() &&
	    (wan_proto == WAN_DHCP || wan_proto == WAN_STATIC))
		start_igmpproxy(wan_ifname);
}

#ifdef RTCONFIG_IPSEC
	if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
		) {
		wait_ntp_repeat(WAIT_FOR_NTP_READY_TIME, WAIT_FOR_NTP_READY_LOOP); //wait for ntp_ready, for rc_ipsec_config_init().
		rc_ipsec_config_init();
#ifdef RTCONFIG_MULTILAN_CFG
		start_dnsmasq(ALL_SDN);
#else
		start_dnsmasq();
#endif
	}
#endif

#ifdef RTCONFIG_VPNC
#if defined(RTCONFIG_VPN_FUSION) && !defined(RTCONFIG_VPN_FUSION_MERLIN)
	start_vpnc();
#else
	if((nvram_match("vpnc_proto", "pptp") || nvram_match("vpnc_proto", "l2tp")) && nvram_match("vpnc_auto_conn", "1"))
		start_vpnc();
#endif
#endif

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
/* TODO: still required? */
	if (nvram_get_int("pptpd_enable")) {
		stop_pptpd();
		start_pptpd();
	}
#endif

#ifdef RTCONFIG_WIREGUARD

#if defined(RTCONFIG_VPN_FUSION_MERLIN) || !defined(RTCONFIG_VPN_FUSION)
	stop_wgcall();
	start_wgcall();
#endif
	stop_wgsall();
	start_wgsall();
#endif

#ifdef RTCONFIG_BLINK_LED
	adjust_netdev_if_of_wan_bled(1, wan_unit, wan_ifname);
#endif

#if !defined(RTCONFIG_MULTIWAN_CFG)
	/* FIXME: Protect below code from 2-nd WAN temporarilly. */
	if(wan_unit == wan_primary_ifunit())
#endif
	{
#ifdef RTCONFIG_TR069
		start_tr();
#endif

#ifdef RTCONFIG_GETREALIP
		char *getip[] = {"getrealip.sh", NULL};
		pid_t pid;

		//_eval(getip, ">>/tmp/log.txt", 0, &pid);
		_eval(getip, ">>/dev/null", 0, &pid);
#endif
	}

/* Need to be done after getrealip */
	stop_upnp();
	start_upnp();

	/* ntp is set, but it didn't just get set, so ntp_synced didn't already did these */
	if (nvram_get_int("ntp_ready") && !first_ntp_sync) {
		stop_ddns();
		start_ddns(NULL);
	}

#ifdef RTCONFIG_LANTIQ
	disable_ppa_wan(wan_ifname);

	if(ppa_support(wan_unit) == 1){
		sleep(1);
		enable_ppa_wan(wan_ifname);
	}
#endif

#if 0
	snprintf(tmp, sizeof(tmp), "arping -w 1 -I %s %s", wan_ifname, gateway);
	if((fp = popen(tmp, "r")) != NULL){
		char wan_mac[18], upper_mac[18];
		int i;
		while(fgets(tmp, sizeof(tmp), fp) != NULL){
			memset(wan_mac, 0, sizeof(wan_mac));
			if(sscanf(tmp, "Unicast reply from %*s [%s] %*s", wan_mac) == 1){
				wan_mac[17] = 0;
				memset(upper_mac, 0, sizeof(upper_mac));
				for(i = 0; wan_mac[i]; ++i)
					upper_mac[i] = toupper(wan_mac[i]);
				nvram_set(strcat_r(prefix, "gw_mac", tmp), upper_mac);
				_dprintf("%s: wan_mac=%s.\n", __func__, upper_mac);
				break;
			}
		}
		pclose(fp);
	}
#else
	char gw_mac[ETHER_ADDR_LEN * 3];

	if(!get_gw_mac(gateway, wan_ifname, gw_mac, sizeof(gw_mac)))
	{
		nvram_set(strcat_r(prefix, "gw_mac", tmp), gw_mac);
		_dprintf("%s: wan_mac=%s.\n", __func__, gw_mac);
	}
	else
	{
		nvram_unset(strcat_r(prefix, "gw_mac", tmp));
		_dprintf("%s: no wan_mac, remove\n", __func__);
	}
#endif

#ifdef RTCONFIG_OPENVPN
	/* ntp is set, but it didn't just get set, so ntp_synced didn't already did these */
	if (nvram_get_int("ntp_ready") && !first_ntp_sync) {
		start_ovpn_eas();
	}
#endif

#ifdef RTCONFIG_BWDPI
	int enabled = check_bwdpi_nvram_setting();
	int changed = tdts_check_wan_changed();

	BWDPI_DBG("enabled = %d, changed = %d\n", enabled, changed);

	if(enabled){
		_dprintf("[%s] do dpi engine service ... \n", __FUNCTION__);
		// if Adaptive QoS or AiProtection is enabled
		int count = 0;
		int val = 0;
		while (count < 5) {
			sleep(1);
			val = found_default_route(0);
			usleep(400*1000);
			count++;
			if ((val == 1) || (count == 5)) break;
		}

		BWDPI_DBG("found_default_route result: %d\n", val);

		if (val) {
			// if restart_wan_if, remove dpi engine related
			if ((f_exists("/dev/detector") || f_exists("/dev/idpfw")) && changed == 0)
			{
				_dprintf("[%s] stop dpi engine service - %d\n", __FUNCTION__, changed);
				stop_dpi_engine_service(0);
			}
			else if ((f_exists("/dev/detector") || f_exists("/dev/idpfw")) && changed == 1)
			{
				_dprintf("[%s] stop dpi engine service - %d\n", __FUNCTION__, changed);
				stop_dpi_engine_service(1);
			}
			_dprintf("[%s] start dpi engine service\n", __FUNCTION__);
			start_dpi_engine_service();
			start_firewall(wan_unit, 0);
		}

		if(IS_NON_AQOS() || IS_ROG_QOS()){
			_dprintf("[wan up] tradtional qos or bandwidth limiter start\n");
#if RTCONFIG_MULTIWAN_IF
			start_mtwan_iQos(wan_unit);
#else
			start_iQos();
#endif
		}
	}
	else{
		if(IS_NON_AQOS() || IS_ROG_QOS()){
			_dprintf("[wan up] tradtional qos or bandwidth limiter start\n");
#if RTCONFIG_MULTIWAN_IF
			start_mtwan_iQos(wan_unit);
#else
			start_iQos();
#endif
		}
	}
#else
	start_iQos();
#endif

#ifdef RTCONFIG_AMAS
	if (is_amaslib_enabled()) {
		// force to trigger amaslib to do static scan
		AMAS_EVENT_TRIGGER(NULL, NULL, 3);
	}
#endif

#ifdef RTCONFIG_AMAS_WGN
	wgn_check_subnet_conflict();
	wgn_check_avalible_brif();
#endif

#ifdef RTCONFIG_FPROBE
	start_fprobe();
#endif

#if defined(RTCONFIG_HND_ROUTER_AX)
	if (wan_proto == WAN_PPTP)
		eval("fc", "config", "--tcp-ack-mflows", "0");
	else
		eval("fc", "config", "--tcp-ack-mflows", nvram_get_int("fc_tcp_ack_mflows_disable_force") ? "0" : "1");
#endif

#if defined(GT10) && defined(RTCONFIG_BWDPI)
	/*
	 * workaround : special case
	 * GT10 + LAN1 as WAN + DPI enabled + WAN protocol is PPTP + fc GRE is enabled, it will effect LAN can't ping/access WAN server
	 * disable gre could solve issue, but the tpt will drop to 170 Mbps from 900 Mbps
	 * */
	if (wan_proto == WAN_PPTP
		&& check_bwdpi_nvram_setting()
		&& nvram_match("wans_dualwan", "wan none")
		&& (nvram_get_int("wans_extwan") == 1)
	)
		eval("fc", "config", "--gre", "0");
	else
		eval("fc", "config", "--gre", "1");
#endif

#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
	if (nvram_match("enable_samba", "1"))
	{
		stop_samba(0);
		start_samba();
	}
#endif

#ifdef RTCONFIG_MULTIWAN_IF
		mtwan_handle_ip_rule(wan_unit);
#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_by_wan(wan_unit);
#endif
#endif

#ifdef RTCONFIG_FBWIFI
	if(sw_mode() == SW_MODE_ROUTER){
		if(nvram_match("fbwifi_dbg", "1"))
			dbg("[%s][%s(%d)] start_fbwifi\n", __FILE__, __FUNCTION__, __LINE__);
		start_fbwifi();
	}
#endif

#ifdef RTCONFIG_HND_ROUTER_AX
	//A workaround for Starlink.
	if(has2_5Gport(wan_ifname))
	{
		logmessage("wan_up", "Find a 2.5G port on [%s]\n", wan_ifname);
		starlink_workaround(wan_ifname, gateway);
	}
#endif /* RTCONFIG_HND_ROUTER_AX */

	/* Restart DDNS when WAN was restored */
	logmessage("wan_up", "Restart DDNS\n");
	stop_ddns();
	start_ddns(NULL);

_dprintf("%s(%s): done.\n", __FUNCTION__, wan_ifname);
}

void
wan_down(char *wan_ifname)
{
	int wan_unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *gateway;
	int wan_proto, end_wan_sbstate = WAN_STOPPED_REASON_NONE;
#ifdef RTCONFIG_INTERNAL_GOBI
	int modem_unit;
	char tmp2[100], prefix2[32];
#endif

	_dprintf("%s(%s)\n", __FUNCTION__, wan_ifname);

#ifdef RTCONFIG_FPROBE
	stop_fprobe();
#endif

#ifdef RTCONFIG_HND_ROUTER_AX
	snprintf(tmp, sizeof(tmp), "%d>%s", 1, wan_ifname);
	if(nvram_match("ethctl_tx_pad", tmp))
	{
		modify_tx_pad(__FUNCTION__, wan_ifname, 0);
	}
#endif /* RTCONFIG_HND_ROUTER_AX */

	/* Skip physical interface of VPN connections */
	if ((wan_unit = wan_ifunit(wan_ifname)) < 0)
		return;

	/* Figure out nvram variable name prefix for this i/f */
	if(wan_prefix(wan_ifname, prefix) < 0)
		return;

	_dprintf("%s(%s): %s.\n", __FUNCTION__, wan_ifname, nvram_safe_get(strcat_r(prefix, "dns", tmp)));

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	if(get_wan_sbstate(wan_unit) == WAN_STOPPED_REASON_DATALIMIT)
		end_wan_sbstate = WAN_STOPPED_REASON_DATALIMIT;
#endif

#ifdef RTCONFIG_INTERNAL_GOBI
	if(dualwan_unit__usbif(wan_unit)){
		modem_unit = get_modemunit_by_type(get_dualwan_by_unit(wan_unit));
		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

		nvram_unset(strcat_r(prefix2, "act_tx", tmp2));
		nvram_unset(strcat_r(prefix2, "act_rx", tmp2));
		nvram_unset(strcat_r(prefix2, "act_band", tmp2));
		if(nvram_get_int("modem_model") == 1){
			nvram_unset(strcat_r(prefix2, "act_pcc", tmp2));
			nvram_unset(strcat_r(prefix2, "act_scc", tmp2));
		}
		nvram_unset(strcat_r(prefix2, "act_operation", tmp2));
		nvram_unset(strcat_r(prefix2, "act_provider", tmp2));
		nvram_unset(strcat_r(prefix2, "act_startsec", tmp2));
	}
#endif

#ifdef RTCONFIG_BLINK_LED
	adjust_netdev_if_of_wan_bled(0, wan_unit, wan_ifname);
#endif

	/* Stop post-authenticator */
	stop_auth(wan_unit, 1);

	wan_proto = get_wan_proto(prefix);

	if (wan_unit == wan_primary_ifunit()) {
		/* Stop multicast router when not VPN */
		if (wan_proto == WAN_DHCP || wan_proto == WAN_STATIC) {
#ifdef RTCONFIG_MULTICAST_IPTV
			if (nvram_get_int("switch_stb_x") > 6 && nvram_match("iptv_ifname", wan_ifname))
#endif
			stop_igmpproxy();
		}

		/* Remove default route to gateway if specified */
		gateway = nvram_safe_get_r(strcat_r(prefix, "gateway", tmp), tmp, sizeof(tmp));
		if (inet_addr_(gateway) == INADDR_ANY)
			gateway = NULL;
		route_del(wan_ifname, 0, "0.0.0.0", gateway, "0.0.0.0");
	}

	/* Remove interface dependent static routes */
	del_wan_routes(wan_ifname);

	switch (wan_proto) {
	case WAN_STATIC:
#ifdef RTCONFIG_SOFTWIRE46
	case WAN_LW4O6:
	case WAN_MAPE:
	case WAN_V6PLUS:
	case WAN_OCNVC:
	case WAN_DSLITE:
#endif
		ifconfig(wan_ifname, IFUP, NULL, NULL);
		break;
	}

	update_wan_state(prefix, WAN_STATE_DISCONNECTED, end_wan_sbstate);

	/* Update resolv.conf
	 * Leave as is if no dns servers left for demand to work */
	if (*nvram_safe_get(strcat_r(prefix, "xdns", tmp)))
		nvram_unset(strcat_r(prefix, "dns", tmp));
	update_resolvconf();
#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_resolvconf();
#endif

#ifdef RTCONFIG_DUALWAN
	if(nvram_match("wans_mode", "lb")) {
		add_multi_routes(1, -1);
#if defined(RTCONFIG_WIREGUARD) || defined(RTCONFIG_OPENVPN)
		amvpn_set_kilswitch_rules_all();
#endif
	}
#endif

#if 0//def RTCONFIG_MULTIWAN_PROFILE
	if (is_mtwan_lb(wan_unit)) {
		mtwan_update_lb_route(wan_unit, 0);
		mtwan_update_lb_iptables(wan_unit, 0);
	}
#endif

#ifdef RTCONFIG_MULTI_PPP
	del_multi_ppp_routes(wan_unit);
#endif

#ifdef RTCONFIG_GETREALIP
#ifdef RTCONFIG_DUALWAN
	if(nvram_invmatch("wans_mode", "lb"))
#endif
	{
		nvram_set(strcat_r(prefix, "realip_state", tmp), "0");
		nvram_set(strcat_r(prefix, "realip_ip", tmp), "");
	}
#endif
#ifdef RTCONFIG_LANTIQ
	disable_ppa_wan(wan_ifname);
#endif
#if defined(RTCONFIG_VPN_FUSION) && !defined(RTCONFIG_VPN_FUSION_MERLIN)
	vpnc_set_internet_policy(0);
#endif
#ifdef RTCONFIG_MULTIWAN_IF
	mtwan_handle_if_updown(wan_unit, wan_ifname, 0);
#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_by_wan(wan_unit);
#endif
#endif
}

int
wan_ifunit(char *wan_ifname)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int unit;

	if ((unit = ppp_ifunit(wan_ifname)) >= 0)
		return unit;

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		switch (get_wan_proto(prefix)) {
		case WAN_DHCP:
		case WAN_STATIC:
#ifdef RTCONFIG_SOFTWIRE46
		case WAN_MAPE:
		case WAN_V6PLUS:
		case WAN_OCNVC:
		case WAN_DSLITE:
#endif
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
				return unit;
			break;
		}
	}
#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6) {
		for (unit = WAN_UNIT_IPTV; unit < WAN_UNIT_MULTICAST_IPTV_MAX; unit++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			switch (get_wan_proto(prefix)) {
			case WAN_DHCP:
			case WAN_STATIC:
			case WAN_BRIDGE:
				if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
					return unit;
				break;
			}
		}
	}
#endif
#ifdef RTCONFIG_MULTISERVICE_WAN
	for (unit = WAN_UNIT_FIRST_MULTISRV_START; unit < WAN_UNIT_MULTISRV_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		switch (get_wan_proto(prefix)) {
		case WAN_DHCP:
		case WAN_STATIC:
		case WAN_BRIDGE:
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
				return unit;
			break;
		}	
	}
#ifdef RTCONFIG_MULTIWAN_IF
	for (unit = WAN_UNIT_MTWAN0_MS_BASE; unit < WAN_UNIT_MTWAN_MS_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		switch (get_wan_proto(prefix)) {
		case WAN_DHCP:
		case WAN_STATIC:
		case WAN_BRIDGE:
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
				return unit;
			break;
		}
	}
#endif
#endif

#ifdef RTCONFIG_MULTIWAN_IF
	unit =  mtwan_ifunit(wan_ifname);
	if(unit != -1)
		return unit;
#endif
	return -1;
}

int
wanx_ifunit(char *wan_ifname)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int unit;

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		switch (get_wan_proto(prefix)) {
		case WAN_PPPOE:
		case WAN_PPTP:
		case WAN_L2TP:
#ifdef RTCONFIG_SOFTWIRE46
		case WAN_MAPE:
		case WAN_V6PLUS:
		case WAN_OCNVC:
		case WAN_DSLITE:
#endif
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
				return unit;
			break;
		}
	}
#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6) {
		for (unit = WAN_UNIT_IPTV; unit < WAN_UNIT_MULTICAST_IPTV_MAX; unit++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			switch (get_wan_proto(prefix)) {
			case WAN_PPPOE:
			case WAN_PPTP:
			case WAN_L2TP:
				if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
					return unit;
				break;
			}
		}
	}
#endif
#ifdef RTCONFIG_MULTIWAN_IF
	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		switch (get_wan_proto(prefix)) {
		case WAN_PPPOE:
		case WAN_PPTP:
		case WAN_L2TP:
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
				return unit;
			break;
		}
	}
#endif
#ifdef RTCONFIG_MULTISERVICE_WAN
	for (unit = WAN_UNIT_FIRST_MULTISRV_START; unit < WAN_UNIT_MULTISRV_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname) &&
			(nvram_match(strcat_r(prefix, "proto", tmp), "pppoe") ||
			 nvram_match(strcat_r(prefix, "proto", tmp), "pptp") ||
			 nvram_match(strcat_r(prefix, "proto", tmp), "l2tp")))
			return unit;
	}
#ifdef RTCONFIG_MULTIWAN_IF
	for (unit = WAN_UNIT_MTWAN0_MS_BASE; unit < WAN_UNIT_MTWAN_MS_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		switch (get_wan_proto(prefix)) {
		case WAN_PPPOE:
		case WAN_PPTP:
		case WAN_L2TP:
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
				return unit;
			break;
		}
	}
#endif
#endif
	return -1;
}

int
preset_wan_routes(char *wan_ifname)
{
	int unit = -1;

	if((unit = wan_ifunit(wan_ifname)) < 0)
		if((unit = wanx_ifunit(wan_ifname)) < 0)
			return -1;

	/* Set default route to gateway if specified */
	if(unit == wan_primary_ifunit())
		route_add(wan_ifname, 0, "0.0.0.0", "0.0.0.0", "0.0.0.0");

	/* Install interface dependent static routes */
	add_wan_routes(wan_ifname);
	return 0;
}

char *
get_lan_ipaddr()
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *inaddr;
	struct in_addr ip_addr;

	/* Retrieve IP info */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
#if 0
		return strdup("0.0.0.0");
#else
	{
		memset(&ip_addr, 0x0, sizeof(ip_addr));
		return inet_ntoa(ip_addr);
	}
#endif

	strncpy(ifr.ifr_name, "br0", IFNAMSIZ);
	inaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	inet_aton("0.0.0.0", &inaddr->sin_addr);

	/* Get IP address */
	ioctl(s, SIOCGIFADDR, &ifr);
	close(s);

	ip_addr = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr;
//	fprintf(stderr, "current LAN IP address: %s\n", inet_ntoa(ip_addr));
	return inet_ntoa(ip_addr);
}

int
ppp0_as_default_route()
{
	int i, n, found;
	FILE *f;
	unsigned int dest, mask;
	char buf[256], device[256];

	n = 0;
	found = 0;
	mask = 0;
	device[0] = '\0';

	if ((f = fopen("/proc/net/route", "r")) != NULL)
	{
		while (fgets(buf, sizeof(buf), f) != NULL)
		{
			if (++n == 1 && strncmp(buf, "Iface", 5) == 0)
				continue;

			i = sscanf(buf, "%255s %x %*s %*s %*s %*s %*s %x",
						device, &dest, &mask);

			if (i != 3)
				break;

			if (device[0] != '\0' && dest == 0 && mask == 0)
			{
				found = 1;
				break;
			}
		}

		fclose(f);

		if (found && !strcmp("ppp0", device))
			return 1;
		else
			return 0;
	}

	return 0;
}

int
found_default_route(int wan_unit)
{
	int i, n, found;
	FILE *f;
	unsigned int dest, mask;
	char buf[256], device[256];
	char *wanif;

	if(wan_unit != wan_primary_ifunit())
		return 1;

	if(dualwan_unit__usbif(wan_unit) && nvram_get_int("modem_pdp") == 2)
		return 1;

	n = 0;
	found = 0;
	mask = 0;
	device[0] = '\0';

	if ((f = fopen("/proc/net/route", "r")) != NULL)
	{
		while (fgets(buf, sizeof(buf), f) != NULL)
		{
			if (++n == 1 && strncmp(buf, "Iface", 5) == 0)
				continue;

			i = sscanf(buf, "%255s %x %*s %*s %*s %*s %*s %x",
				device, &dest, &mask);
			if (i != 3)
			{
				break;
			}

			if (device[0] != '\0' && dest == 0 && mask == 0)
			{
				wanif = get_wan_ifname(wan_unit);
				if (!strcmp(wanif, device))
				{
					found = 1;
					break;
				}
			}
		}

		fclose(f);

		if (found)
		{
			return 1;
		}
	}

	_dprintf("\nNO default route!!!\n");

	return 0;
}

long print_num_of_connections()
{
	char buf[256];
	char entries[16], others[256];
	long num_of_entries;

	FILE *fp = fopen("/proc/net/stat/nf_conntrack", "r");
	if (!fp) {
		fprintf(stderr, "no connection!\n");
		return 0;
	}

	fgets(buf, 256, fp);
	fgets(buf, 256, fp);
	fclose(fp);

	memset(entries, 0x0, 16);
	sscanf(buf, "%15s %s", entries, others);
	num_of_entries = strtoul(entries, NULL, 16);

	fprintf(stderr, "connection count: %ld\n", num_of_entries);
	return num_of_entries;
}

#ifdef RTCONFIG_BCM9
static int
ctf_entry_cleanup(void)
{
	ctf_cfg_request_t req;
	ctf_tuple_t tuple, *tp = NULL;
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct msghdr msg;
	struct iovec iov;
	int sock_fd, ret = SUCCESS;

	memset(&req, '\0', sizeof(req));
	req.command_id = CTFCFG_CMD_IPC_CLEANUP;
	req.size = sizeof(ctf_tuple_t);
	tp = (ctf_tuple_t *)req.arg;
	/* CTFCFG_CMD_IPC_CLEANUP doesn't care about the content of tuple*/
	*tp = tuple;

	/* Create a netlink socket */
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_CTF);

	if (sock_fd < 0) {
		fprintf(stderr, "Netlink socket create failed\n");
		return FAILURE;
	}

	/* Associate a local address with the opened socket */
	memset(&src_addr, 0, sizeof(struct sockaddr_nl));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	src_addr.nl_groups = 0;
	bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

	/* Fill the destination address, pid of 0 indicates kernel */
	memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = 0;

	/* Allocate memory for sending configuration request */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(CTFCFG_MAX_SIZE));

	if (nlh == NULL) {
		fprintf(stderr, "Out of memory allocating cfg buffer\n");
		ret = FAILURE;
		goto out;
	}

	/* Fill the netlink message header. The configuration request
	 * contains netlink header followed by data.
	 */
	nlh->nlmsg_len = NLMSG_SPACE(CTFCFG_MAX_SIZE);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	/* Fill the data part */
	memcpy(NLMSG_DATA(nlh), &req, sizeof(req));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Send request to kernel module */
	ret = sendmsg(sock_fd, &msg, 0);
	if (ret < 0) {
		perror("sendmsg:");
		ret = FAILURE;
		goto out;
	}

	/* Wait for the response */
	memset(nlh, 0, NLMSG_SPACE(CTFCFG_MAX_SIZE));
	ret = recvmsg(sock_fd, &msg, 0);
	if (ret < 0) {
		perror("recvmsg:");
		ret = FAILURE;
		goto out;
	}

	/* Copy data to user buffer */
	memcpy(&req, NLMSG_DATA(nlh), sizeof(req));

out:
	if (nlh) {
		free(nlh);
	}

	if (sock_fd >= 0) {
		close(sock_fd);
	}

	return ret;
}
#endif

void
start_wan(void)
{
#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV)
	int unit;
#endif

#ifdef RTCONFIG_DSL
	dsl_wan_config(1);
#endif

	if (!is_routing_enabled())
	{
#ifdef HND_ROUTER
		eval("iptables", "-t", "filter", "-F");
		/* drop access to envrams service from interfaces other than lo */
		eval("iptables", "-I", "INPUT", "!", "-i", "lo", "-p", "tcp", "--dport", "5152", "-j", "DROP");
#endif
		return;
	}

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		nvram_set("start_wan", "1");
#endif

	/* Create links */
	mkdir("/tmp/ppp", 0777);
	mkdir("/tmp/ppp/peers", 0777);
	symlink("/sbin/rc", "/tmp/ppp/ip-up");
	symlink("/sbin/rc", "/tmp/ppp/ip-down");
	symlink("/sbin/rc", "/tmp/ppp/ip-pre-up");
#ifdef RTCONFIG_IPV6
	symlink("/sbin/rc", "/tmp/ppp/ipv6-up");
	symlink("/sbin/rc", "/tmp/ppp/ipv6-down");
	symlink("/sbin/rc", "/tmp/dhcp6c");
#endif
	symlink("/sbin/rc", "/tmp/ppp/auth-fail");
#ifdef RTCONFIG_VPNC
	symlink("/sbin/rc", "/tmp/ppp/vpnc-ip-up");
	symlink("/sbin/rc", "/tmp/ppp/vpnc-ip-down");
	symlink("/sbin/rc", "/tmp/ppp/vpnc-ip-pre-up");
	symlink("/sbin/rc", "/tmp/ppp/vpnc-auth-fail");
#if defined(RTCONFIG_VPN_FUSION) && !defined(RTCONFIG_VPN_FUSION_MERLIN)
	if(!d_exists("/etc/openvpn")) {
		mkdir("/etc/openvpn", 0700);
	}
	symlink("/sbin/rc", "/etc/openvpn/ovpnc-up");
	symlink("/sbin/rc", "/etc/openvpn/ovpnc-down");
	symlink("/sbin/rc", "/etc/openvpn/ovpnc-route-up");
	symlink("/sbin/rc", "/etc/openvpn/ovpnc-route-pre-down");
#endif
#endif
	symlink("/sbin/rc", "/tmp/udhcpc_wan");
	symlink("/sbin/rc", "/tmp/zcip");
#ifdef RTCONFIG_EAPOL
	symlink("/sbin/rc", "/tmp/wpa_cli");
#endif
//	symlink("/dev/null", "/tmp/ppp/connect-errors");

#if defined(RTCONFIG_QCA) || \
		(defined(RTCONFIG_RALINK) && !defined(RTCONFIG_DSL) && !defined(RTN13U))
	reinit_hwnat(-1);
#endif

#ifdef HND_ROUTER
	fc_init();
#endif

#if defined(RTCONFIG_USB_MODEM) && !defined(RTCONFIG_SOC_IPQ40XX)
	if(sw_mode() == SW_MODE_ROUTER && (get_wans_dualwan()&WANSCAP_USB)){
#if !defined(RTCONFIG_BT_CONN)
		_dprintf("wan: Insert USB modules early.\n");
		add_usb_host_modules();
#endif
		add_usb_modem_modules();
	}
#endif

#if defined(RTCONFIG_BONDING_WAN) && defined(RTCONFIG_RALINK)
	if (sw_mode() == SW_MODE_ROUTER && bond_wan_enabled() && nvram_get("bond1_ifnames")) {
		char nv_mode[] = "bondXXX_mode", nv_policy[] = "bondXXX_policy";
		sprintf(nv_mode, "%s_mode", "bond1");
		sprintf(nv_policy, "%s_policy", "bond1");
		set_bonding("bond1", nvram_get(nv_mode), nvram_get(nv_policy), get_wan_hwaddr());
		ifconfig("bond1", IFUP, NULL, NULL);
	}
#endif

#ifdef RTCONFIG_MULTIWAN_IF
	mtwan_start_multi_wan();
#else

#ifdef RTCONFIG_DUALWAN
	char wans_mode[16];

	snprintf(wans_mode, sizeof(wans_mode), "%s", nvram_safe_get("wans_mode"));

	/* Start each configured and enabled wan connection and its undelying i/f */
	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(dualwan_unit__nonusbif(unit)){
			if(((!strcmp(wans_mode, "fo") || !strcmp(wans_mode, "fb")) && unit == wan_primary_ifunit())
					|| !strcmp(wans_mode, "lb")
					){
#ifdef RTCONFIG_MULTISERVICE_WAN
				config_mswan(unit);
#endif
				_dprintf("%s: start_wan_if(%d)!\n", __FUNCTION__, unit);
				start_wan_if(unit);
			}
#ifdef RTCONFIG_HND_ROUTER
			else if(!strcmp(wans_mode, "fo") || !strcmp(wans_mode, "fb")){
				_dprintf("%s: stop_wan_if(%d) for IFUP only!\n", __func__, unit);
				stop_wan_if(unit);
			}
#endif
		}
	}
#else // RTCONFIG_DUALWAN
#ifdef RTCONFIG_MULTISERVICE_WAN
	config_mswan(WAN_UNIT_FIRST);
#endif
	_dprintf("%s: start_wan_if(%d)!\n", __FUNCTION__, WAN_UNIT_FIRST);
	start_wan_if(WAN_UNIT_FIRST);

#ifdef RTCONFIG_USB_MODEM
	if(is_usb_modem_ready(WANS_DUALWAN_IF_USB) == 1 && nvram_get_int("success_start_service") == 1){
		_dprintf("%s: start_wan_if(%d)!\n", __FUNCTION__, WAN_UNIT_SECOND);
		start_wan_if(WAN_UNIT_SECOND);
	}
#endif
#endif // RTCONFIG_DUALWAN

#endif // RTCONFIG_MULTIWAN_IF

	nvram_set("wanduck_start_detect", "1");

#ifdef RTCONFIG_MULTICAST_IPTV
	/* Start each configured and enabled wan connection and its undelying i/f */
	if (nvram_get_int("switch_stb_x") > 6) {
		for (unit = WAN_UNIT_IPTV; unit < WAN_UNIT_MULTICAST_IPTV_MAX; ++unit) {
			_dprintf("%s(IPTV): start_wan_if(%d)!\n", __FUNCTION__, unit);
			start_wan_if(unit);
		}
	}
#endif

#ifndef RT4GAC68U
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(2,6,36)
	f_write_string("/proc/sys/net/bridge/bridge-nf-call-iptables", "0", 0, 0);
	f_write_string("/proc/sys/net/bridge/bridge-nf-call-ip6tables", "0", 0, 0);
#endif
#endif

	/* Report stats */
	if (*nvram_safe_get("stats_server")) {
		char *stats_argv[] = { "stats", nvram_safe_get("stats_server"), NULL };
		_eval(stats_argv, NULL, 5, NULL);
	}
}

void
stop_wan(void)
{
	int unit;

	if (nvram_match("wifison_ready", "1"))
	{
#ifdef RTCONFIG_WIFI_SON
		if (nvram_get("start_wan") == NULL)
			return;
		nvram_unset("start_wan");
#else
	_dprintf("no wifison feature\n");
#endif
	}
	else
	{
		if (!is_routing_enabled())
			return;
	}

#ifdef RTCONFIG_RALINK
	if (is_hwnat_loaded()) {
		unregister_hnat_wlifaces();
		modprobe_r(MTK_HNAT_MOD);
		if (!g_reboot)
			sleep(1);
	}
#endif

#ifdef RTCONFIG_IPV6
	stop_wan6();
#endif

	/* Start each configured and enabled wan connection and its undelying i/f */
	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
		stop_wan_if(unit);
#ifdef RTCONFIG_MULTIWAN_IF
	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++unit)
		stop_wan_if(unit);
#endif

#ifdef HND_ROUTER
	if (module_loaded("pptp"))
		modprobe_r("pptp");

	fc_fini();
#endif

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	if (nvram_get_int("pptpd_enable"))
		stop_pptpd();
#endif

	/* Remove dynamically created links */
#ifdef RTCONFIG_EAPOL
	unlink("/tmp/wpa_cli");
#endif
	unlink("/tmp/udhcpc_wan");
	unlink("/tmp/zcip");
	unlink("/tmp/ppp/ip-up");
	unlink("/tmp/ppp/ip-down");
	unlink("/tmp/ppp/ip-pre-up");
#ifdef RTCONFIG_IPV6
	unlink("/tmp/ppp/ipv6-up");
	unlink("/tmp/ppp/ipv6-down");
	unlink("/tmp/dhcp6c");
#endif
	unlink("/tmp/ppp/auth-fail");
#ifdef RTCONFIG_VPNC
	unlink("/tmp/ppp/vpnc-ip-up");
	unlink("/tmp/ppp/vpnc-ip-down");
	unlink("/tmp/ppp/vpnc-ip-pre-up");
	unlink("/tmp/ppp/vpnc-auth-fail");
#endif
	rmdir("/tmp/ppp");

#ifdef RTCONFIG_BCM9
	/* Clean up all ipct entry */
	if (!nvram_match("ctf_disable", "1") && !nvram_match("ctf_clean_disable", "1"))
		ctf_entry_cleanup();
#endif
}

void convert_wan_nvram(char *prefix, int unit)
{
#ifdef RTCONFIG_DUALWAN
	int mac_clone = 0;
#endif
	char tmp[100];
	char macbuf[32];
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_RGMII_BRCM5301X)
	char hwaddr_5g[18];
#endif

	_dprintf("%s(%s)\n", __FUNCTION__, prefix);

	// setup hwaddr
	strcpy(macbuf, nvram_safe_get(strcat_r(prefix, "hwaddr_x", tmp)));
	if (strlen(macbuf)!=0 && strcasecmp(macbuf, "FF:FF:FF:FF:FF:FF")) {
#ifdef RTCONFIG_DUALWAN
		mac_clone = 1;
#endif
		nvram_set(strcat_r(prefix, "hwaddr", tmp), macbuf);
		logmessage("wan", "mac clone: [%s] == [%s]\n", tmp, macbuf);
	}
#ifdef CONFIG_BCMWL5
#ifdef RTCONFIG_RGMII_BRCM5301X
	else{
		/* QTN */
		if(strcmp(prefix, "wan1_") == 0) {
			strcpy(hwaddr_5g, get_wan_hwaddr());
			inc_mac(hwaddr_5g, 7);
			nvram_set(strcat_r(prefix, "hwaddr", tmp), hwaddr_5g);
			logmessage("wan", "[%s] == [%s]\n", tmp, hwaddr_5g);
		} else {
			nvram_set(strcat_r(prefix, "hwaddr", tmp), nvram_safe_get("lan_hwaddr"));
			logmessage("wan", "[%s] == [%s]\n", tmp, nvram_safe_get("lan_hwaddr"));
		}
	}
#else
	else nvram_set(strcat_r(prefix, "hwaddr", tmp), nvram_safe_get("et0macaddr"));
#endif	/* RTCONFIG_RGMII_BRCM5301X */
#else
	else nvram_set(strcat_r(prefix, "hwaddr", tmp), get_wan_hwaddr());
#endif	/* CONFIG_BCMWL5 */

#if defined(RTCONFIG_DUALWAN)
	if (!mac_clone && unit > 0) {
		unsigned char eabuf[ETHER_ADDR_LEN];
		char macaddr[32];

		/* Don't use same MAC address on all WANx interfaces. */
		ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), eabuf);
		eabuf[ETHER_ADDR_LEN - 1] += unit;
		ether_etoa(eabuf, macaddr);
		nvram_set(strcat_r(prefix, "hwaddr", tmp), macaddr);
	}
#endif

#if defined(RTCONFIG_MULTISERVICE_WAN) && defined(RTCONFIG_DUALWAN)
	if (!mac_clone) {
		int base_unit = get_ms_base_unit(unit);
		int ms_idx = get_ms_idx_by_wan_unit(unit);
		unsigned char eabuf[ETHER_ADDR_LEN] = {0};
#ifdef DSL_AX82U
		if (is_ax5400_i1()) {
			if (get_dualwan_by_unit(base_unit) == WANS_DUALWAN_IF_WAN)
				ether_atoe(nvram_safe_get("wl1_hwaddr"), eabuf);
			else
				ether_atoe(nvram_safe_get("wl0_hwaddr"), eabuf);
		}
		else
#endif
		{
#ifdef RTCONFIG_MULTIWAN_IF
			ether_atoe(get_wan_hwaddr(), eabuf);
			if (base_unit > MULTI_WAN_START_IDX)
				ether_inc(eabuf, base_unit - MULTI_WAN_START_IDX);
			else if (base_unit)
				ether_inc(eabuf, base_unit);
#else
			if (base_unit)
				ether_atoe(nvram_safe_get("wl1_hwaddr"), eabuf);
			else
				ether_atoe(nvram_safe_get("wl0_hwaddr"), eabuf);
#endif
		}
		ether_inc(eabuf, ms_idx);
		ether_etoa(eabuf, macbuf);
		nvram_set(strcat_r(prefix, "hwaddr", tmp), macbuf);
	}
#endif

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6 &&
	    unit > 9) {
		unsigned char ea[6];
		ether_atoe(get_wan_hwaddr(), ea);
		ea[5] = (ea[5] & 0xf0) | ((ea[5] + unit - 9) & 0x0f);
		ether_etoa(ea, macbuf);
		nvram_set(strcat_r(prefix, "hwaddr", tmp), macbuf);
	}
#endif
	// sync proto
	if (nvram_match(strcat_r(prefix, "proto", tmp), "static"))
		nvram_set_int(strcat_r(prefix, "dhcpenable_x", tmp), 0);
	// backlink unit for ppp
	nvram_set_int(strcat_r(prefix, "unit", tmp), unit);
}

void dumparptable()
{
	char buf[256];
	char ip_entry[32], hw_type[8], flags[8], hw_address[32], mask[32], device[8];
	char macbuf[32];

	FILE *fp = fopen("/proc/net/arp", "r");
	if (!fp) {
		fprintf(stderr, "no proc fs mounted!\n");
		return;
	}

	mac_num = 0;

	while (fgets(buf, 256, fp) && (mac_num < MAX_MAC_NUM - 2)) {
		sscanf(buf, "%s %s %s %s %s %s", ip_entry, hw_type, flags, hw_address, mask, device);

		if (!strcmp(device, "br0") && strlen(hw_address)!=0)
		{
			strcpy(mac_clone[mac_num++], hw_address);
		}
	}
	fclose(fp);

	strcpy(macbuf, nvram_safe_get("wan0_hwaddr_x"));

	// try pre-set mac
	if (strlen(macbuf)!=0 && strcasecmp(macbuf, "FF:FF:FF:FF:FF:FF"))
		strcpy(mac_clone[mac_num++], macbuf);

	// try original mac
	strcpy(mac_clone[mac_num++], get_lan_hwaddr());

	if (mac_num)
	{
		fprintf(stderr, "num of mac: %d\n", mac_num);
		int i;
		for (i = 0; i < mac_num; i++)
			fprintf(stderr, "mac to clone: %s\n", mac_clone[i]);
	}
}

#ifdef RTCONFIG_QCA_PLC_UTILS
int autodet_plc_main(int argc, char *argv[]){
	int cnt;
	cnt = get_connected_plc(NULL);
	nvram_set_int("autodet_plc_state" , cnt);

	return 0;
}
#endif

#ifdef RTCONFIG_SOFTWIRE46
void init_wan46(void) {

	int unit;
	char prefix[]="wan46detXXXXXX_", tmp[100];

	/* JP Sku Only */
	if (!strncmp(nvram_safe_get("territory_code"), "JP", 2)) {
		nvram_unset("wan46det_proceeding");
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			if (unit == WAN_UNIT_FIRST)
				snprintf(prefix, sizeof(prefix), "wan46det_");
			else
				snprintf(prefix, sizeof(prefix), "wan46det%d_", unit);
			nvram_unset(strcat_r(prefix, "state", tmp));
		}
	}
	return;
}

int auto46det_main(int argc, char *argv[]) {

	int opt, unit, re = 0;
	char wired_link_nvram[16];
	char prefix[]="wan46detXXXXXX_", tmp[100];

	while ((opt = getopt(argc, argv, "r")) != -1) {
		switch (opt) {
		case 'r':
			re = 1;
			break;
		default:
			break;
		}
	}

	if (nvram_get_int("wan46det_proceeding"))
		return 0;

	nvram_set("wan46det_proceeding", "1");

	/* JP Sku Only */
	if (!strncmp(nvram_safe_get("territory_code"), "JP", 2)) {
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {

			if (!eth_wantype(unit))
				continue;

			link_wan_nvname(unit, wired_link_nvram, sizeof(wired_link_nvram));
			if (unit == WAN_UNIT_FIRST)
				snprintf(prefix, sizeof(prefix), "wan46det_");
			else
				snprintf(prefix, sizeof(prefix), "wan46det%d_", unit);

			if (nvram_get_int("x_Setting") && !ipv6_enabled()) {
				nvram_set_int(strcat_r(prefix, "state", tmp), WAN46DET_STATE_UNKNOW);
				_dprintf("[%s(%d)] ### IPv6 Disabled. ###\n", __FUNCTION__, __LINE__);
				continue;
			}

			if (re) {
				_dprintf("[%s(%d)] ### Aute Detect RESET ###\n", __FUNCTION__, __LINE__);
				nvram_set_int(strcat_r(prefix, "state", tmp), WAN46DET_STATE_INITIALIZING);
			}

			if (!nvram_get_int(wired_link_nvram)) {
				nvram_set_int(strcat_r(prefix, "state", tmp), WAN46DET_STATE_NOLINK);
				continue;
			}

			if (nvram_get_int(strcat_r(prefix, "state", tmp)) >= WAN46DET_STATE_UNKNOW)
				continue;

			if (!pids("odhcp6c")) {
				nvram_set("ipv6_service", "ipv6pt");
				wan6_up(get_wan6face());
				sleep(5);
			}

			nvram_set_int(strcat_r(prefix, "state", tmp), WAN46DET_STATE_INITIALIZING);
			nvram_set_int(strcat_r(prefix, "state", tmp), wan46det(unit));
		}
	}

	nvram_set("wan46det_proceeding", "0");
	return 0;
}
#endif

int autodet_main(int argc, char *argv[]){
	int unit;
	char wired_link_nvram[16];
	char prefix2[]="autodetXXXXXX_", tmp2[100];
	int status;
#ifdef RTCONFIG_ALPINE
	int i;
#endif
#if 0
	char hwaddr_x[32];
#endif

	if(nvram_get_int("autodet_proceeding"))
		return 0;

	nvram_set("autodet_proceeding", "1");//Cherry Cho added for httpd checking in 2016/4/22.

	f_write_string("/tmp/detect_wrong.log", "", 0, 0);

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(!eth_wantype(unit))
			continue;

		link_wan_nvname(unit, wired_link_nvram, sizeof(wired_link_nvram));
		if(unit == WAN_UNIT_FIRST)
			snprintf(prefix2, sizeof(prefix2), "autodet_");
		else
			snprintf(prefix2, sizeof(prefix2), "autodet%d_", unit);

		//if(!get_wanports_status(unit))
		if(!nvram_get_int(wired_link_nvram))
		{
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_NOLINK);
			nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_OK);
			continue;
		}

		if(nvram_get_int(strcat_r(prefix2, "state", tmp2)) == AUTODET_STATE_FINISHED_WITHPPPOE
				|| nvram_get_int(strcat_r(prefix2, "auxstate", tmp2)) == AUTODET_STATE_FINISHED_WITHPPPOE){
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_OK);
			nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_WITHPPPOE);
			continue;
		}

		nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_INITIALIZING);
		nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_INITIALIZING);

#if 0
		// it shouldnot happen, because it is only called in default mode
		if(!nvram_match(strcat_r(prefix, "proto", tmp), "dhcp")){
			status = discover_all(unit);
			if(status == -1)
				nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_NOLINK);
			else if(status == 2)
				nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_WITHPPPOE);
			else
				nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_OK);

			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_NODHCP);
			continue;
		}

		if(get_wan_state(unit) == WAN_STATE_CONNECTED){
			i = nvram_get_int(strcat_r(prefix, "lease", tmp));

			if(i < 60 && is_private_subnet(strcat_r(prefix, "ipaddr", tmp))){
				sleep(i);
			}
			//else{
			//	nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_OK);
			//	continue;
			//}
		}
#endif

		status = discover_all(unit);

		if(get_wan_state(unit) == WAN_STATE_CONNECTED){
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_OK);
			if(status < 0)
				nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_FAIL);
			else if(status == 2)
				nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_WITHPPPOE);
			else
				nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_OK);
		}
		else if(status < 0){
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_FAIL);
			nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_FAIL);
		}
		else if(status == 2){
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_WITHPPPOE);
			nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_OK);
		}
#if 0
		else if(is_ip_conflict(unit)){
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_OK);
			nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_OK);
			continue;
		}
#endif
		else{
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_OK);
			nvram_set_int(strcat_r(prefix2, "auxstate", tmp2), AUTODET_STATE_FINISHED_OK);
		}

// remove the Auto MAC clone from the decision on 2018/4/11.
#if 0
		dumparptable();

		// backup hwaddr_x
		strcpy(hwaddr_x, nvram_safe_get(strcat_r(prefix, "hwaddr_x", tmp)));
		//nvram_set(strcat_r(prefix, "hwaddr_x", tmp), "");

		int waitsec = nvram_get_int(strcat_r(prefix2, "waitsec", tmp2));

#define DEF_CLONE_WAITSEC 10
		if(waitsec <= 0)
			waitsec = DEF_CLONE_WAITSEC;

		i = 0;
		while(i < mac_num && (!is_wan_connect(unit) && !is_ip_conflict(unit))){
			if(!(nvram_match("wl0_country_code", "SG")) &&
			   strncmp(nvram_safe_get("territory_code"), "SG", 2) != 0){ // Singpore do not auto clone
				_dprintf("try clone %s\n", mac_clone[i]);
				nvram_set(strcat_r(prefix, "hwaddr_x", tmp), mac_clone[i]);
			}
			char buf[32];
			snprintf(buf, sizeof(buf), "restart_wan_if %d", unit);
			notify_rc_and_wait(buf);
			_dprintf("%s: wait a IP during %d seconds...\n", __FUNCTION__, waitsec);
			int count = 0;
			while(count < waitsec && (!is_wan_connect(unit) && !is_ip_conflict(unit))){
				sleep(1);

				++count;
			}
			++i;
		}

		if(i == mac_num){
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_FAIL);
			// restore hwaddr_x
			nvram_set(strcat_r(prefix, "hwaddr_x", tmp), hwaddr_x);
		}
		else if(i == mac_num-1){ // OK in original mac
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_OK);
		}
		else{ // OK in cloned mac
			nvram_set_int(strcat_r(prefix2, "state", tmp2), AUTODET_STATE_FINISHED_OK);
		}
		nvram_commit();
#endif
	}

#ifdef RTCONFIG_ALPINE
	// detect 10G
	const char *intf_10g[] = {"eth0", "eth2", NULL};

	for(i = 0; intf_10g[i] != NULL; ++i){
		snprintf(prefix, sizeof(prefix), "link_10G%d", i+1);
		if(nvram_get_int(prefix) != 1){
			nvram_set_int(strcat_r(prefix, "_state", tmp), AUTODET_STATE_FINISHED_NOLINK);
			continue;
		}

		status = discover_interface(intf_10g[i], 0);
		if(status == -1)
			nvram_set_int(strcat_r(prefix, "_state", tmp), AUTODET_STATE_FINISHED_NOLINK);
		else if(status == 2)
			nvram_set_int(strcat_r(prefix, "_state", tmp), AUTODET_STATE_FINISHED_WITHPPPOE);
		else
			nvram_set_int(strcat_r(prefix, "_state", tmp), AUTODET_STATE_FINISHED_OK);
	}
#endif

	nvram_set("autodet_proceeding", "0");//Cherry Cho added for httpd checking in 2016/4/22.

	return 0;
}

#ifdef RTCONFIG_DETWAN
#define RETRY_COUNT	3
#define MAX_DETWAN 4

int string_remove(char *string, const char *match)
{
	char *p;
	int len;
	if(string == NULL || match == NULL)
		return 0;
	if((p = strstr(string, match)) == NULL)
		return 0;

	len = strlen(match);
	while(isspace(*(p+len)))
		len++;
	memmove(p, p + len, strlen(p)+1 - len); //including '\0'
	return 1;
}

int string_add(char *string, const char *match, int at_head)
{
	char *p;
	int len;
	if(string == NULL || match == NULL)
		return 0;
	if(strstr(string, match) != NULL)
		return 0;

	len = strlen(match);
	if(at_head) {
		memmove(string+len+1, string, strlen(string)+1);
		memcpy(string, match, len);
		string[len] = ' ';
	} else {
		p = string + strlen(string);
		sprintf(p, " %s", match);
	}
	return 1;
}

void detwan_set_net_block(int add)
{
	char *block_dhcp_argv[] = {"ebtables", "-A", "FORWARD", "-p", "IPV4", "--ip-protocol", "UDP", "--ip-dport", "67", "-j", "DROP", NULL};
	char *block_nonarp_argv[] = {"ebtables", "-A", "FORWARD", "-d", "Broadcast", "-p", "!", "ARP", "-i", "eth+", "-j", "DROP", NULL};

	if(add == 0)
	{
		block_dhcp_argv[1] = "-D";
		block_nonarp_argv[1] = "-D";
	}

	_eval(block_dhcp_argv, NULL, 0, NULL);
	_eval(block_nonarp_argv, NULL, 0, NULL);
}

void detwan_apply_wan(const char *wan_ifname, unsigned int wan_mask, unsigned int lan_mask)
{
	char lan[128];
	int max_inf;
	int modify = 0;

	max_inf = nvram_get_int("detwan_max");
	if(max_inf <= 0)
		return;

	if(nvram_match("detwan_apply", "yes"))
	{
		int retry = 20;
		while(retry-- > 0 && nvram_safe_get("rc_service")[0] != '\0')
		{
			sleep(1);
		}
		if(retry == 0)
		{
			logmessage(__func__, "1: SKIP");
			return;	//skip this result
		}
	}
	{
		int idx;
		char var_name[32];
		char ifname[32];

		strcpy(lan, nvram_safe_get("lan_ifnames"));
		for(idx = 0; idx < max_inf; idx++) {
			snprintf(var_name, sizeof(var_name), "detwan_name_%d", idx);
			snprintf(ifname, sizeof(ifname), "%s", nvram_safe_get(var_name));
			if(strlen(ifname) <= 0)
				break;

			if(strcmp(ifname, wan_ifname) == 0) {
				if(string_remove(lan, ifname)) {
					modify++;
					eval("brctl", "delif", "br0", ifname);
					eval("ifconfig", ifname, "down");
					eval("ifconfig", ifname, "hw", "ether", get_wan_hwaddr());
					eval("ifconfig", ifname, "up");
				}
			} else {
				if(string_add(lan, ifname, 1)) {
					modify++;
					eval("ifconfig", ifname, "down");
					eval("ifconfig", ifname, "hw", "ether", get_lan_hwaddr());
					eval("ifconfig", ifname, "0.0.0.0");
					eval("brctl", "addif", "br0", ifname);
					eval("ifconfig", ifname, "up");
				}
			}
		}
	}

	logmessage(__func__, "2: wan(%s) lan(%s) modify(%d)\n", wan_ifname, lan, modify);

	if(modify == 0 && nvram_match("wan0_ifname", wan_ifname))
		return;	// skip, when the same interface

	nvram_set_int("wanports_mask", wan_mask);
	nvram_set_int("lanports_mask", lan_mask);
	nvram_set_int("detwan_wan_mask", wan_mask);
	nvram_set_int("detwan_lan_mask", lan_mask);
	nvram_set("lan_ifnames", lan);
	nvram_set("wan_ifnames", wan_ifname);
	nvram_set("wan0_ifname", wan_ifname);
	nvram_set("detwan_ifname", wan_ifname);
	nvram_set("wan0_gw_ifname", wan_ifname);
	nvram_commit();

	if(nvram_match("detwan_apply", "yes"))
	{
		int retry = RETRY_COUNT;
		char buf[32];
		snprintf(buf, sizeof(buf), "restart_wan_if %d", 0);
		while(retry-- > 0 && notify_rc_and_wait(buf) != 0);
		logmessage(__func__, "3: 'restart_wan_if 0' finish\n");
	}
}

int detwan_allmask(void)
{
	char var_name[32];
	int allmask;
	int idx, max_inf;
	int value;

	max_inf = nvram_get_int("detwan_max");
	allmask = 0;

	for(idx = 0; idx < max_inf; idx++){
		snprintf(var_name, sizeof(var_name), "detwan_mask_%d", idx);
		if((value = nvram_get_int(var_name)) != 0)
			allmask |= value;
		else
			break;
	}

	return allmask;
}

int detwan_check(char *ifname, unsigned int *wan_mask)
{
	int idx;
	int max_inf;
	int state = -1;
	char var_name[32];
	char var_value[PATH_MAX];
	char wan0_ifname[32];
	int value;
	int phy;
	int inf_count;
	char inf_names_buf[MAX_DETWAN][32];
	char *inf_names[MAX_DETWAN];
	int inf_mask[MAX_DETWAN];
	int got_inf;

	if(ifname == NULL || wan_mask == NULL)
		return -1;

	max_inf = nvram_get_int("detwan_max");
	strncpy(wan0_ifname, nvram_safe_get("wan0_ifname"), sizeof(wan0_ifname)-1);
	wan0_ifname[sizeof(wan0_ifname)-1] = '\0';

	logmessage(__func__, "0: max_inf(%d) wan0_ifname(%s)\n", max_inf, wan0_ifname);
	inf_count = 0;
	got_inf = -1;
	for(idx = 0; idx < max_inf; idx++){
		snprintf(var_name, sizeof(var_name), "detwan_mask_%d", idx);
		if((value = nvram_get_int(var_name)) != 0) {
			phy = get_ports_status((unsigned int)value);
			snprintf(var_name, sizeof(var_name), "detwan_name_%d", idx);
			snprintf(var_value, sizeof(var_value), "%s", nvram_safe_get(var_name));

//			if(wan0_ifname == NULL || wan0_ifname[0] == '\0')
			{ //No WAN
				if(phy > 0 && inf_count < MAX_DETWAN && strlen(var_value) > 0) {
					strncpy(inf_names_buf[inf_count], var_value, sizeof(inf_names_buf[0])-1);
					inf_names_buf[inf_count][sizeof(inf_names_buf[0])-1] = '\0';
					inf_names[inf_count] = inf_names_buf[inf_count];
					inf_mask[inf_count] = value;
					inf_count++;
				}
			}
#if 0
			else
			{
				if(strlen(var_value) > 0 && strcmp(wan0_ifname, var_value) == 0)
				{ //Is WAN
				}
				else if(strlen(var_value) > 0)
				{ //Not WAN
				}
			}
#endif
		}
	}
	if(inf_count) {
		time_t now;
		extern int discover_interfaces(int num, const char **current_wan_ifnames, int dhcp_det, int *got_inf);
		state = discover_interfaces(inf_count, (const char **) inf_names, nvram_match("wan0_proto", "dhcp"), &got_inf);
		now = time(NULL);
		logmessage(__func__, "1: wan0_ifname(%s) inf_count(%d) state(%d) got_inf(%d) %s", wan0_ifname, inf_count, state, got_inf, ctime(&now));

		if(state <= 0 && inf_count == 1)	//set to the only phy with cable
		{
			state = 0;
			got_inf = 0;
		}
		nvram_set_int("detwan_proto", state);
		if(got_inf < 0 || got_inf >= inf_count) {
			nvram_unset("detwan_phy");
			ifname[0] = '\0';
			*wan_mask = 0;
		}
		else {
			nvram_set("detwan_phy", inf_names[got_inf]);
			strcpy(ifname, inf_names[got_inf]);
			*wan_mask = inf_mask[got_inf];
		}
	}
	else {
		time_t now = time(NULL);
		logmessage(__func__, "2: NO phy is linked %s", ctime(&now));
		nvram_set_int("detwan_proto", -1);
		nvram_unset("detwan_phy");
		ifname[0] = '\0';
		*wan_mask = 0;
	}
	return state;
}

static void detwan_preinit(void)
{
	char lan[128];
	char defwan[32];

	strcpy(defwan, nvram_safe_get("wan0_ifname"));

	nvram_unset("wan0_gw_ifname");
	nvram_unset("wan0_ifname");
	nvram_unset("detwan_phy");
	nvram_unset("detwan_ifname");
	nvram_unset("wan_ifnames");
	strcpy(lan, nvram_safe_get("lan_ifnames"));
	if (string_add(lan, defwan, 1))
		nvram_set("lan_ifnames", lan);

	stop_wanduck();
	// Only MAP-AC2200 && MAC-AC1300 support DETWAN
	// following configs are same in both products
	nvram_set("detwan_proto", "-1");
	nvram_set("wanports_mask", "0");
	nvram_set("lanports_mask", "48");
	nvram_set("detwan_max", "2");
	nvram_set("detwan_name_0", "eth0");
	nvram_set("detwan_mask_0", "32");
	nvram_set("detwan_name_1", "eth1");
	nvram_set("detwan_mask_1", "16");

	ifconfig(defwan, 0, NULL, NULL);
	eval("ifconfig", defwan, "hw", "ether", get_lan_hwaddr());
	ifconfig(defwan, IFUP | IFF_ALLMULTI, "0.0.0.0", NULL);
	eval("brctl", "addif", nvram_safe_get("lan_ifname"), defwan);
	start_wanduck();
}

extern void set_defwan(char *wan, char *wanmask, char *lanmask);
static void detwan_reset(void)
{
	char lan[128];
	char *defwan;

#if defined(MAPAC1300)
	defwan = "eth1";
	set_defwan(defwan, "16", "32");
#elif defined(MAPAC2200)
	defwan = "eth0";
	set_defwan(defwan, "32", "16");
#else
#error are you DETWAN product?
#endif
	strcpy(lan, nvram_safe_get("lan_ifnames"));
	if (string_remove(lan, defwan))
		nvram_set("lan_ifnames", lan);

	stop_wanduck();

	nvram_unset("detwan_proto");
	nvram_unset("detwan_max");
	nvram_unset("detwan_name_0");
	nvram_unset("detwan_mask_0");
	nvram_unset("detwan_name_1");
	nvram_unset("detwan_mask_1");

	ifconfig(defwan, 0, NULL, NULL);
	eval("ifconfig", defwan, "hw", "ether", get_wan_hwaddr());
	ifconfig(defwan, IFUP | IFF_ALLMULTI, "0.0.0.0", NULL);
	eval("brctl", "delif", nvram_safe_get("lan_ifname"), defwan);
	eval("killall", "-SIGUSR2", "udhcpc");
	sleep(1);
	eval("killall", "-SIGUSR1", "udhcpc");
	start_wanduck();
}

int detwan_main(int argc, char *argv[]){
	int max_inf;
	unsigned int allmask;
	int state = -1;
	int sw_mode;
	char ifname[32];
	unsigned int wan_mask;

	sw_mode = nvram_get_int("sw_mode");

	if (sw_mode != SW_MODE_ROUTER)
		return -1;

	if (argc >=2) {
		if (strncmp(argv[1], "init", 4) == 0)
			detwan_preinit();
		else if (strncmp(argv[1], "reset", 5) == 0)
			detwan_reset();
		return 0;
	}

	allmask = detwan_allmask();

	logmessage(__func__, "0: sw_mode(%d) wan0_ifname(%s) allmask(%08x)", sw_mode, nvram_get("wan0_ifname"), allmask);
	if (allmask == 0)
		return -1;

	if(nvram_match("detwan_apply", "yes"))
	{
		int retry = 20;
		while(retry-- > 0 && nvram_safe_get("rc_service")[0] != '\0')
		{
			sleep(1);
		}
		if(retry <= 0)
		{
			logmessage(__func__, "1: rc_service(%s) block !!", nvram_safe_get("rc_service"));
			return -1;
		}
	}

	state = detwan_check(ifname, &wan_mask);
	if(state >= 0 && ifname[0] != '\0') {
		detwan_apply_wan(ifname, wan_mask, allmask & ~wan_mask);
	}

	logmessage(__func__, "9: finish !!");
	return 0;
}
#endif	/* RTCONFIG_DETWAN */

int get_gw_mac(const char *gateway, const char *ifname, char *mac, const size_t mac_size)
{
	char tmp[512];
	int ret = -1;
	FILE *fp;

	if(!gateway || !mac)
		return ret;

	if(ifname)
		snprintf(tmp, sizeof(tmp), "ip neigh show %s dev %s 2>/dev/null", gateway, ifname);
	else
		snprintf(tmp, sizeof(tmp), "ip neigh show %s 2>/dev/null", gateway);

	if ((fp = popen(tmp, "r")) != NULL)
	{
		char lladdr[18], *ptr;
		if (fscanf(fp, "%*s lladdr %17s", lladdr) == 1)
		{
			for (ptr = lladdr; *ptr; ptr++)
				*ptr = toupper(*ptr);

			strlcpy(mac, lladdr, mac_size);
			ret = 0;
		}
		pclose(fp);
	}
	return ret;
}
