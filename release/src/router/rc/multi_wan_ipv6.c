/*
 * Copyright 2023, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <shared.h>
#include <pthread.h>

#include "rc.h"
#include "multi_wan.h"
#include "sdn.h"

#ifdef RTCONFIG_MULTIWAN_PROFILE
extern MTWAN_PROF_T mtwan_prof[MAX_MTWAN_PROFILE_NUM];
#endif
extern pthread_mutex_t mtwanduck_chkrt, mtwanduck_chkaddr;

void start_mtwan_ipv6_tunnel(int unit)
{
	char ip[INET6_ADDRSTRLEN + 4] = {0};
	struct in_addr addr4;
	struct in6_addr addr;
	char wan_prefix[16] = {0};
	char wanip[INET_ADDRSTRLEN] = {0};
	char gateway[INET6_ADDRSTRLEN] = {0};
	char tun_dev[IFNAMSIZ] = {0};
	char mtu[5] = {0};
	int service;
	int len;
	char table[8] = {0};

	_dprintf("%s(%d)\n", __FUNCTION__, unit);

	service = get_ipv6_service_by_unit(unit);
	strlcpy(tun_dev, get_wan6_ifname(unit), sizeof(tun_dev));
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", unit);
	strlcpy(wanip, nvram_pf_safe_get(wan_prefix, "ipaddr"), sizeof(wanip));
	strlcpy(mtu, nvram_safe_get(ipv6_nvname_by_unit("ipv6_tun_mtu", unit)), sizeof(mtu));
	if (atoi(mtu) <= 0)
		strlcpy(mtu, "1480", sizeof(mtu));

	modprobe("sit");

	eval("ip", "tunnel", "add", tun_dev, "mode", "sit",
		"remote", (service == IPV6_6IN4) ? nvram_safe_get(ipv6_nvname_by_unit("ipv6_tun_v4end", unit)) : "any",
		"local", wanip,
		"ttl", nvram_safe_get(ipv6_nvname_by_unit("ipv6_tun_ttl", unit)));
	eval("ip", "link", "set", tun_dev, "mtu", mtu, "up");

	switch (service) {
		case IPV6_6TO4: {
			int prefixlen = 16;
			int mask4size = 0;

			/* address */
			addr4.s_addr = 0;
			memset(&addr, 0, sizeof(addr));
			inet_aton(wanip, &addr4);
			addr.s6_addr16[0] = htons(0x2002);
			ipv6_mapaddr4(&addr, prefixlen, &addr4, mask4size);
			//addr.s6_addr16[7] |= htons(0x0001);
			ip[0] = '\0';
			inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
			len = strlen(ip);
			snprintf(ip+len, sizeof(ip)-len, "/%d", prefixlen);

			/* gateway */
			snprintf(gateway, sizeof(gateway), "::%s", nvram_safe_get(ipv6_nvname_by_unit("ipv6_relay", unit)));
			nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), gateway);

			add_ip6_lanaddr();
			break;
		}
		case IPV6_6RD: {
			int prefixlen = nvram_get_int(ipv6_nvname_by_unit("ipv6_6rd_prefixlen", unit));
			int mask4size = nvram_get_int(ipv6_nvname_by_unit("ipv6_6rd_ip4size", unit));
			char brprefix[sizeof("255.255.255.255/32")];

			/* 6rd domain */
			addr4.s_addr = 0;
			if (mask4size) {
				inet_aton(wanip, &addr4);
				addr4.s_addr &= htonl(0xffffffffUL << (32 - mask4size));
			} else	addr4.s_addr = 0;
			snprintf(ip, sizeof(ip), "%s/%d", nvram_safe_get(ipv6_nvname_by_unit("ipv6_6rd_prefix", unit)), prefixlen);
			snprintf(brprefix, sizeof(brprefix), "%s/%d", inet_ntoa(addr4), mask4size);
			eval("ip", "tunnel", "6rd", "dev", tun_dev,
				 "6rd-prefix", ip, "6rd-relay_prefix", brprefix);

			/* address */
			addr4.s_addr = 0;
			memset(&addr, 0, sizeof(addr));
			inet_aton(wanip, &addr4);
			inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname_by_unit("ipv6_6rd_prefix", unit)), &addr);
			ipv6_mapaddr4(&addr, prefixlen, &addr4, mask4size);
			//addr.s6_addr16[7] |= htons(0x0001);
			ip[0] = '\0';
			inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
			len = strlen(ip);
			snprintf(ip+len, sizeof(ip)-len, "/%d", prefixlen);

			/* gateway */
			snprintf(gateway, sizeof(gateway), "::%s", nvram_safe_get(ipv6_nvname_by_unit("ipv6_6rd_router", unit)));
			nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), gateway);

			add_ip6_lanaddr();
			break;
		}
		case IPV6_6IN4: {
			char wan_ifname[IFNAMSIZ] = {0};
			char remote[INET_ADDRSTRLEN] = {0};
			/* remote */
			strlcpy(wan_ifname, get_wan_ifname(unit), sizeof(wan_ifname));
			strlcpy(remote, nvram_safe_get(ipv6_nvname_by_unit("ipv6_tun_v4end", unit)), sizeof(remote));
			eval("ip", "route", "add", remote, "dev", wan_ifname);

			/* address */
			snprintf(ip, sizeof(ip), "%s/%d",
				nvram_safe_get(ipv6_nvname_by_unit("ipv6_tun_addr", unit)),
				nvram_get_int(ipv6_nvname_by_unit("ipv6_tun_addrlen", unit)) ? : 64);

			/* gateway */
			strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_tun_peer", unit)), sizeof(gateway));
			nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), gateway);
			if (gateway[0])
				eval("ip", "-6", "route", "add", gateway, "dev", tun_dev, "metric", "1");

			add_ip6_lanaddr();
		}
	}

	eval("ip", "-6", "addr", "add", ip, "dev", tun_dev);

	mtwan_get_route_table_id(unit, table, sizeof(table));
	eval("ip", "-6", "route", "del", "::/0", "table", table);
	if (gateway[0])
		eval("ip", "-6", "route", "add", "::/0", "via", gateway, "dev", tun_dev, "metric", "1", "table", table);
	else
		eval("ip", "-6", "route", "add", "::/0", "dev", tun_dev, "metric", "1", "table", table);

	mtwan6_handle_ip_rule(unit);

#ifdef RTCONFIG_MULTIWAN_PROFILE
	if (is_mtwan_lb(unit)) {
		mtwan6_update_lb_route(unit, MTWAN_CONNECTED);
		mtwan_update_lb_iptables(unit, MTWAN_CONNECTED, MTWAN_HANDLE_V6);
	}
	else if (is_mtwan6_primary(unit))
#else
	if (!is_mtwan_unit(unit))
#endif
	{
#ifdef RTCONFIG_MULTIWAN_PROFILE
		mtwan6_update_main_default_route();
#endif
		eval("ip", "-6", "route", "del", "::/0");
		if (gateway[0])
			eval("ip", "-6", "route", "add", "::/0", "via", gateway, "dev", tun_dev, "metric", "1");
		else
			eval("ip", "-6", "route", "add", "::/0", "dev", tun_dev, "metric", "1");
	}

	eval("ip", "-6", "route", "flush", "cache");
}

void stop_mtwan_ipv6_tunnel(int unit)
{
	int service = get_ipv6_service_by_unit(unit);

	if (service == IPV6_6TO4 || service == IPV6_6RD || service == IPV6_6IN4) {
		eval("ip", "tunnel", "del", get_wan6_ifname(unit));
	}
	if (service == IPV6_6TO4 || service == IPV6_6RD) {
		// TODO: flush lan/sdn global address
		nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
	}
	else if (service == IPV6_6IN4) {
		char remote[INET_ADDRSTRLEN] = {0};
		strlcpy(remote, nvram_safe_get(ipv6_nvname_by_unit("ipv6_tun_v4end", unit)), sizeof(remote));
		eval("ip", "route", "del", remote);
	}

	modprobe_r("sit");
}

void start_mtwan_rdisc6(int unit)
{
	int service;
	pid_t pid;
	char wan6_ifname[IFNAMSIZ] = {0};
	char *rdisc6_argv[] = { "rdisc6", "-r", RDISC6_RETRY_MAX, wan6_ifname, NULL };
	char path[64], buf[16];

	service = get_ipv6_service_by_unit(unit);
	if (
#ifdef RTCONFIG_6RELAYD
		service != IPV6_PASSTHROUGH &&
#endif
		service != IPV6_NATIVE_DHCP)
		return;

	if (nvram_get_int("ipv6_no_rdisc6"))
		return;

	stop_mtwan_rdisc6(unit);

	strlcpy(wan6_ifname, get_wan6_ifname(unit), sizeof(wan6_ifname));
	_eval(rdisc6_argv, NULL, 0, &pid);
	snprintf(buf, sizeof(buf), "%d", pid);
	snprintf(path, sizeof(path), "/var/run/rdisc6.%d.pid", unit);
	f_write_string(path, buf, 0, 0);
}

void stop_mtwan_rdisc6(int unit)
{
	char path[64];
	snprintf(path, sizeof(path), "/var/run/rdisc6.%d.pid", unit);
	kill_pidfile_tk(path);
	unlink(path);
}

void start_mtwan_ipv6(int unit)
{
	int service = get_ipv6_service_by_unit(unit);
	char buf[128] = {0};

	// Check if turned on
	switch (service) {
	case IPV6_NATIVE_DHCP:
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
		if (nvram_get_int(ipv6_nvname_by_unit("ipv6_dhcp_pd", unit))) {
			nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), "");
			nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
		} else
			add_ip6_lanaddr();
		nvram_set(ipv6_nvname_by_unit("ipv6_llremote", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_dns", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_domain", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_ra_routes", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_wan_addr", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), "");
		break;
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_llremote", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_dns", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_domain", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_ra_routes", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_wan_addr", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), "");
		break;
#endif
	case IPV6_MANUAL:
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
		nvram_safe_get_r(ipv6_nvname_by_unit("ipv6_prefix_length_s", unit), buf, sizeof(buf));
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), buf);
		nvram_safe_get_r(ipv6_nvname_by_unit("ipv6_rtr_addr_s", unit), buf, sizeof(buf));
		nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), buf);
		add_ip6_lanaddr();
		nvram_set(ipv6_nvname_by_unit("ipv6_llremote", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_dnsenable", unit), "0");	//FIXME: UI should disable it.
		break;
	case IPV6_6IN4:
		nvram_safe_get_r(ipv6_nvname_by_unit("ipv6_prefix_s", unit), buf, sizeof(buf));
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), buf);
		nvram_safe_get_r(ipv6_nvname_by_unit("ipv6_prefix_length_s", unit), buf, sizeof(buf));
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), buf);
		nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
		add_ip6_lanaddr();
		break;
	case IPV6_6TO4:
	case IPV6_6RD:
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
		break;
	}
}

void stop_mtwan_ipv6(int unit)
{
	char wan_ifname[IFNAMSIZ] = {0};
	char table[8] = {0};

#ifdef RTCONFIG_6RELAYD
	stop_mtwan_6relayd(unit);
#endif
	stop_mtwan_dhcp6c(unit);
	stop_mtwan_ipv6_tunnel(unit);

	strlcpy(wan_ifname, get_wan6_ifname(unit), sizeof(wan_ifname));
	eval("ip", "-6", "addr", "flush", "scope", "global", "dev", wan_ifname);

#if !defined(RTCONFIG_MULTIWAN_PROFILE)
	if (is_mtwan_unit(unit))
#endif
	{
		mtwan_get_route_table_id(unit, table, sizeof(table));
		eval("ip", "-6", "route", "flush", "table", table);
	}

	if (nvram_get(ipv6_nvname_by_unit("ipv6_service", unit)))
	{
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_llremote", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_dns", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_domain", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_ra_routes", unit), "");
	}
}

void config_mtwan_ipv6(int wan_unit, int enable)
{
	char *wan6_ifname = get_wan6_ifname(wan_unit);
	int service = get_ipv6_service_by_unit(wan_unit);
	int wan_proto;
	char wan_prefix[16];
	int accept_defrtr;

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);

	if (enable) {
		enable_ipv6(wan6_ifname);
		if (!with_ipv6_linklocal_addr(wan6_ifname)) {
			reset_ipv6_linklocal_addr(wan6_ifname, 0);
			enable_ipv6(wan6_ifname);
		}
		else {
dprintf("%s: %s link: %s\n", __FUNCTION__, wan6_ifname, getifaddr(wan6_ifname, AF_INET6, GIF_LINKLOCAL));
		}
	}
	else {
		disable_ipv6(wan6_ifname);
	}

	switch (service) {
	case IPV6_NATIVE_DHCP:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		wan_proto = get_wan_proto(wan_prefix);
		accept_defrtr = (service == IPV6_NATIVE_DHCP) &&
				(wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) &&
				nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp") ?
				nvram_get_int(ipv6_nvname_by_unit("ipv6_accept_defrtr", wan_unit)) : 1;
		ipv6_sysconf(wan6_ifname, "accept_ra", 1);
		ipv6_sysconf(wan6_ifname, "accept_ra_defrtr", accept_defrtr);
		break;
	case IPV6_6IN4:
	case IPV6_6TO4:
	case IPV6_6RD:
	case IPV6_MANUAL:
	default:
		ipv6_sysconf(wan6_ifname, "accept_ra", 0);
		break;
	}
}

void start_mtwan_lan_ipv6(int wan_unit)
{
	if (!ipv6x_enabled(wan_unit))
		return;

#ifdef RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	int act;
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i) {
			if (pmtl[i].enable && pmtl[i].nw_t.v6_enable) {
				act = 1;
#ifdef RTCONFIG_MULTIWAN_PROFILE
				if (pmtl[i].sdn_t.mtwan_idx && !is_mtwan_unit_in_active_group(pmtl[i].sdn_t.mtwan_idx, wan_unit))
					act = 0;
				else
#endif
				if (pmtl[i].sdn_t.wan6_idx && mtwan_get_mapped_unit(pmtl[i].sdn_t.wan6_idx) != wan_unit)
					act = 0;
				if (act) {
					set_intf_ipv6_dad(pmtl[i].nw_t.ifname, 0, 1);
					enable_ipv6(pmtl[i].nw_t.ifname);
				}
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
#else
	set_intf_ipv6_dad(nvram_safe_get("lan_ifname"), 0, 1);
	enable_ipv6(nvram_safe_get("lan_ifname"));
#endif
	config_ipv6(1, 0);
	start_mtwan_ipv6(wan_unit);
}

void stop_mtwan_lan_ipv6(int wan_unit)
{
#ifdef RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	int act;
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i) {
			if (pmtl[i].enable) {
				act = 1;
#ifdef RTCONFIG_MULTIWAN_PROFILE
				if (pmtl[i].sdn_t.mtwan_idx && !is_mtwan_unit_in_active_group(pmtl[i].sdn_t.mtwan_idx, wan_unit))
					act = 0;
				else
#endif
				if (pmtl[i].sdn_t.wan6_idx && mtwan_get_mapped_unit(pmtl[i].sdn_t.wan6_idx) != wan_unit)
					act = 0;
				if (act) {
					set_intf_ipv6_dad(pmtl[i].nw_t.ifname, 0, 0);
					disable_ipv6(pmtl[i].nw_t.ifname);
				}
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
#else
	set_intf_ipv6_dad(nvram_safe_get("lan_ifname"), 0, 0);
	disable_ipv6(nvram_safe_get("lan_ifname"));
#endif
	stop_mtwan_ipv6(wan_unit);
	config_mtwan_ipv6(wan_unit, 0);
}

void restart_mtwan_dnsmasq_ipv6(int wan_unit)
{
	if (!ipv6x_enabled(wan_unit))
		return;

#ifdef RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	int act = 0;

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i) {
			if (pmtl[i].enable && pmtl[i].nw_t.v6_enable) {
#ifdef RTCONFIG_MULTIWAN_PROFILE
				if (pmtl[i].sdn_t.mtwan_idx && is_mtwan_unit_in_active_group(pmtl[i].sdn_t.mtwan_idx, wan_unit))
					act = 1;
				else
#endif
				if (pmtl[i].sdn_t.wan6_idx && mtwan_get_mapped_unit(pmtl[i].sdn_t.wan6_idx) == wan_unit)
					act = 1;
				else if (is_mtwan6_primary(wan_unit))
					act = 1;
				if (act) {
					start_dnsmasq(pmtl[i].nw_t.idx);
				}
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
#else
	start_dnsmasq();
#endif
}

#ifdef RTCONFIG_MULTIWAN_PROFILE
int is_mtwan6_primary(int wan_unit)
{
	return (nvram_get_int(ipv6_nvname_by_unit("ipv6_primary", wan_unit))) ? 1 : 0;
}

int is_mtwan6_used(int wan_unit)
{
	int i, j;
	int ret = 0;
#ifdef RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
#endif

	if (is_mtwan6_primary(wan_unit))
		return 1;

#ifdef RTCONFIG_SOFTWIRE46
	switch(get_ipv4_service_by_unit(wan_unit)) {
		case WAN_LW4O6:
		case WAN_MAPE:
		case WAN_V6PLUS:
		case WAN_OCNVC:
		case WAN_DSLITE:
			return 1;
	}
#endif

	// Multi-WAN profile
	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++) {
		if (mtwan_prof[i].enable == 0)
			continue;
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++) {
			if (mtwan_prof[i].wan_units[j] != wan_unit)
				continue;
			if (mtwan_prof[i].mt_group[j] == mtwan_prof[i].group)
				return 1;
		}
	}

#ifdef RTCONFIG_MULTILAN_CFG
	// SDN
	if (wan_unit) {
		pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
		if (pmtl) {
			get_mtlan_by_idx(SDNFT_TYPE_WAN6, wan_unit, pmtl, &mtl_sz);
			if (mtl_sz)
				ret = 1;
			FREE_MTLAN((void *)pmtl);
		}
	}
#endif

	return (ret);
}

int mtwan6_neighbor_main(int argc, char *argv[])
{
	char *wan6_ifname = safe_getenv("MASTER_INTERFACE");
	char *neighbor_file = safe_getenv("NEIGHBOR_FILE");
	int wan6_unit = get_wan6_unit(wan6_ifname);

	mtwan_init_profile();

#ifdef RTCONFIG_MULTILAN_CFG
	handle_sdn_routing_ipv6_neigh(wan6_unit, neighbor_file);
#else
	/// TODO
#endif

	return 0;
}

static void _set_mtwan6_def_gateway_by_route(int unit, const char *ifname, const char *table)
{
	FILE *fp;
	char cmd[256];
	char buf[256];
	char *p, *pend;

	snprintf(cmd, sizeof(cmd), "ip -6 route show default dev %s table %s", ifname, table);
	fp = popen(cmd, "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			trimNL(buf);
			if ((p = strstr(buf, "via")) != NULL) {
				if ((pend = strchr(p+4, ' ')) != NULL) {
					*pend = '\0';
					nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), p+4);
					break;
				}
			}
		}
		pclose(fp);
	}
}

void mtwan6_update_lb_route(int wan_unit, int up)
{
	int i, j;
	int mtwan_idx, mtwan_group;
	char lb_table[16] = {0}, mt_table[16] = {0};
	char cmd[512] = {0};
	int real_unit, wan_proto;
	char ifname[IFNAMSIZ] = {0};
	char network[INET6_ADDRSTRLEN+sizeof("/128")] = {0};
	char gateway[INET6_ADDRSTRLEN] = {0};

	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++)
	{
		if (mtwan_prof[i].enable == 0)
			continue;

		mtwan_idx = i + 1;

		if (is_mtwan_lb_in_profile(mtwan_idx, wan_unit) == 0)
			continue;

		mtwan_group = 0;
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].wan_units[j] == wan_unit)
			{
				mtwan_group = mtwan_prof[i].mt_group[j];
				break;
			}
		}
		if (mtwan_group == 0)
			continue;

		/// flush
		mtwan_get_lb_route_table_id(mtwan_idx, mtwan_group, lb_table, sizeof(lb_table));
		eval("ip", "-6", "route", "flush", "table", lb_table);

		snprintf(cmd, sizeof(cmd), "ip -6 route replace default table %s", lb_table);
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] == mtwan_group)
			{
				real_unit = mtwan_prof[i].wan_units[j];
				mtwan_get_route_table_id(real_unit, mt_table, sizeof(mt_table));
				strlcpy(ifname, get_wan6_ifname(real_unit), sizeof(ifname));
				if ((wan_unit != real_unit) ? (is_mtwan6_def_route_exist(ifname, mt_table)) : ((up) ? 1 : 0))
				{
					/// wan route
					strlcpy(network, getifaddr(ifname, AF_INET6, GIF_PREFIXLEN), sizeof(network));
					eval("ip", "-6", "route", "add", network, "dev", ifname, "table", lb_table);

					/// default route
					wan_proto = get_ipv4_service_by_unit(real_unit);
					if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
					 && nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp"))
						strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_llremote", real_unit)), sizeof(gateway));
					else
						strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", real_unit)), sizeof(gateway));

					snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via %s weight %d dev %s"
						, (*gateway) ? gateway : "::\0"
						, mtwan_prof[i].mt_weight[j]
						, ifname);
				}
			}
		}
		/// default route
		_dprintf("cmd: %s\n", cmd);
		system(cmd);

		/// main table default route
		if (mtwan_idx == 1 && mtwan_group == mtwan_prof[i].group)	// default Multi-WAN profile using same group
		{
			char new_cmd[512] = {0};
			char *p;
			p = strstr(cmd, "default");
			if (p) {
				*p = '\0';
				strlcat(new_cmd, cmd, sizeof(new_cmd));
				strlcat(new_cmd, "::/1 ", sizeof(new_cmd));
				p = strstr(p + 1, "nexthop");
				if (p) {
					strlcat(new_cmd, p, sizeof(new_cmd));
					_dprintf("cmd: %s\n", new_cmd);
					system(new_cmd);

					memset(new_cmd, 0, sizeof(new_cmd));
					strlcat(new_cmd, cmd, sizeof(new_cmd));
					strlcat(new_cmd, "8000::/1 ", sizeof(new_cmd));
					strlcat(new_cmd, p, sizeof(new_cmd));
					_dprintf("cmd: %s\n", new_cmd);
					system(new_cmd);
				}
			}
		}

		eval("ip", "-6", "route", "flush", "cache");
	}
}

void mtwan6_update_profile_lb_route(int mtwan_idx, int mtwan_group, int unit, int up)
{
	char table[8];
	char cmd[256];
	int i = mtwan_idx -1;
	int j;
	char network[INET6_ADDRSTRLEN+sizeof("/128")] = {0};
	char gateway[INET6_ADDRSTRLEN] = {0};
	char ifname[IFNAMSIZ] = {0};
	int cnt = 0;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || mtwan_group < 1 || mtwan_group > MAX_MULTI_WAN_NUM)
		return;

	/// flush
	mtwan_get_lb_route_table_id(mtwan_idx, mtwan_group, table, sizeof(table));
	eval("ip", "-6", "route", "flush", "table", table);

	snprintf(cmd, sizeof(cmd), "ip -6 route replace default table %s", table);

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].mt_group[j] == mtwan_group)
		{
			/// wan route
			strlcpy(ifname, get_wan6_ifname(mtwan_prof[i].wan_units[j]), sizeof(ifname));
			strlcpy(network, getifaddr(ifname, AF_INET6, GIF_PREFIXLEN), sizeof(network));
			eval("ip", "-6", "route", "add", network, "dev", ifname, "table", table);

			/// default route
			strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", mtwan_prof[i].wan_units[j])), sizeof(gateway));
			snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via %s weight %d dev %s"
				, (*gateway) ? gateway : "::\0"
				, mtwan_prof[i].mt_weight[j]
				, ifname);
			cnt++;
		}
	}
	/// default route
	_dprintf("cmd: %s\n", cmd);
	system(cmd);

	eval("ip", "-6", "route", "flush", "cache");
}

void mtwan6_update_main_default_route()
{
	char cmd[256];
	int i;
	int wan_unit;
	char ifname[IFNAMSIZ] = {0};
	char table[8] = {0};
	char new_cmd[512] = {0};
	char *p;

	if (mtwan_prof[0].enable == 0)
	{
		wan_unit = wan_primary_ifunit_ipv6();
		snprintf(cmd, sizeof(cmd), "ip -6 route replace ::/1 via %s dev %s"
			, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", wan_unit))
			, get_wan6_ifname(wan_unit));
		_dprintf("cmd: %s\n", cmd);
		system(cmd);
		snprintf(cmd, sizeof(cmd), "ip -6 route replace 8000::/1 via %s dev %s"
			, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", wan_unit))
			, get_wan6_ifname(wan_unit));
		_dprintf("cmd: %s\n", cmd);
		system(cmd);
		eval("ip", "-6", "route", "flush", "cache");
		return;
	}

	snprintf(cmd, sizeof(cmd), "ip -6 route replace default");
	for(i = 0; i < MAX_MULTI_WAN_NUM; i++)
	{
		if (mtwan_prof[0].mt_group[i] == mtwan_prof[0].group)
		{
			wan_unit = mtwan_prof[0].wan_units[i];
			mtwan_get_route_table_id(wan_unit, table, sizeof(table));
			strlcpy(ifname, get_wan6_ifname(wan_unit), sizeof(ifname));

			if (if_nametoindex(ifname) && is_mtwan6_def_route_exist(ifname, table))
			{
				snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via %s weight %d dev %s"
					, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", wan_unit))
					, mtwan_prof[0].mt_weight[i]
					, ifname
					);
			}
		}
	}

	// To avoid the error "ICMPv6: RA: ndisc_router_discovery failed to add default route",
	// Leave default route added by kernel, use "::/1", "8000::/1" instead.
	p = strstr(cmd, "default");
	if (p)
	{
		*p = '\0';
		strlcat(new_cmd, cmd, sizeof(new_cmd));
		strlcat(new_cmd, "::/1 ", sizeof(new_cmd));
		p = strstr(p + 1, "nexthop");
		if (p) {
			strlcat(new_cmd, p, sizeof(new_cmd));
			_dprintf("cmd: %s\n", new_cmd);
			system(new_cmd);

			memset(new_cmd, 0, sizeof(new_cmd));
			strlcat(new_cmd, cmd, sizeof(new_cmd));
			strlcat(new_cmd, "8000::/1 ", sizeof(new_cmd));
			strlcat(new_cmd, p, sizeof(new_cmd));
			_dprintf("cmd: %s\n", new_cmd);
			system(new_cmd);
		}
	}

	eval("ip", "-6", "route", "flush", "cache");
}

int mtwan6_get_active_wan_unit(int mtwan_idx)
{
	int i = mtwan_idx - 1, j;
	int unit;
	char table[8];

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM)
		return WAN_UNIT_NONE;

	mtwan_check_and_init_profile();

	if (mtwan_prof[i].enable == 0)
		return WAN_UNIT_NONE;

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++) {
		if (mtwan_prof[i].mt_group[j] == mtwan_prof[i].group) {
			unit = mtwan_prof[i].wan_units[j];
			mtwan_get_route_table_id(unit, table, sizeof(table));
			if (ipv6x_enabled(unit)
			 && is_mtwan6_def_route_exist(get_wan6_ifname(unit), table)
			)
				return unit;
		}
	}

	return WAN_UNIT_NONE;
}

#ifdef RTCONFIG_MULTILAN_CFG
int mtwan6_get_sdn_wan6_unit(int sdn_idx)
{
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int wan6_unit = 0;

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan_by_idx(SDNFT_TYPE_SDN, sdn_idx, pmtl, &mtl_sz);
#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (pmtl->sdn_t.mtwan_idx)
			wan6_unit = mtwan6_get_active_wan_unit(pmtl->sdn_t.mtwan_idx);
		else
#endif
		if (pmtl->sdn_t.wan6_idx)
			wan6_unit = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx);
		else
			wan6_unit = wan_primary_ifunit_ipv6();

		FREE_MTLAN((void *)pmtl);
	}
	return wan6_unit;
}
#endif

void mtwan6_restart_6relayd(int mtwan_idx, int old_unit, int new_unit)
{
	_dprintf("%s: MTWAN(%d), unit: %d->%d\n", __FUNCTION__, mtwan_idx, old_unit, new_unit);

	if (old_unit == WAN_UNIT_NONE || new_unit == WAN_UNIT_NONE)
		return;

	if (old_unit != new_unit
	 && get_ipv6_service_by_unit(old_unit) == IPV6_PASSTHROUGH
	 && get_ipv6_service_by_unit(new_unit) != IPV6_DISABLED
	 && (!is_mtwan6_used(old_unit) || !mtwanduck_get_mtwan_status(old_unit ?: MULTI_WAN_START_IDX + old_unit))
	) {
		stop_mtwan_6relayd(old_unit);
	}

	if (get_ipv6_service_by_unit(new_unit) == IPV6_PASSTHROUGH
	 && is_mtwan6_used(new_unit)
	) {
		start_mtwan_6relayd(new_unit);
	}
}
#endif	//RTCONFIG_MULTIWAN_PROFILE

void mtwan6_handle_ip_rule(int unit)
{
	int pref_num;
	char table[8], pref[8];
	char wan_dns_buf[256];
	char wan_dns[64], *next_dns;
	int n;
	char nvname[16];
	int mtwan_unit = (unit < MULTI_WAN_START_IDX) ? mtwan_get_unit_by_dualwan_unit(unit) : unit;

	mtwan_get_route_table_id(unit, table, sizeof(table));

	pref_num = mtwan_get_dns_rule_pref(mtwan_unit);
	snprintf(pref, sizeof(pref), "%d", pref_num);
	remove_ip_rules(pref_num, MTWAN_HANDLE_V6);

	if (nvram_get_int(ipv6_nvname_by_unit("ipv6_dnsenable", unit))) {
		strlcpy(wan_dns_buf, nvram_safe_get(ipv6_nvname_by_unit("ipv6_get_dns", unit)), sizeof(wan_dns_buf));
		if (wan_dns_buf[0]) {
			foreach(wan_dns, wan_dns_buf, next_dns) {
				eval("ip", "-6", "rule", "add", "iif", "lo", "to", wan_dns, "table", table, "pref", pref);
			}
		}
	}
	else {
		for (n = 1; n <= 3; n++) {
			snprintf(nvname, sizeof(nvname), "ipv6_dns%d", n);
			strlcpy(wan_dns, nvram_safe_get(ipv6_nvname_by_unit(nvname, unit)), sizeof(wan_dns));
			if (wan_dns[0]) {
				eval("ip", "-6", "rule", "add", "iif", "lo", "to", wan_dns, "table", table, "pref", pref);
			}
		}
	}

#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_by_wan_ipv6(unit);
#else
	if(mtwan_get_default_wan() == unit)
	{
		/// rule for default wan
		pref_num = IP_RULE_PREF_MTWAN_ROUTE;
		snprintf(pref, sizeof(pref), "%d", pref_num);
		remove_ip_rules(pref_num, MTWAN_HANDLE_V6);
		eval("ip", "-6", "rule", "add", "iif", nvram_safe_get("lan_ifname"), "table", table, "perf", pref);
	}
#endif

#ifdef RTCONFIG_MULTIWAN_PROFILE
	/// rule for MARK
	pref_num = mtwan_get_mark_rule_pref(mtwan_unit);
	remove_ip_rules(pref_num, MTWAN_HANDLE_V6);
	doSystem("ip -6 rule add fwmark 0x%08x/0x%08x pref %d lookup %s"
		, IPTABLES_MARK_MTWAN_SET(mtwan_unit - MULTI_WAN_START_IDX, 0), IPTABLES_MARK_MTWAN_MASK
		, pref_num, table);
#endif
}

int is_mtwan6_def_route_exist(const char *ifname, const char *table)
{
	FILE *fp;
	char cmd[256];
	char buf[256];
	int ret = 0;

	if (!ifname || !table)
		return 0;

	snprintf(cmd, sizeof(cmd), "ip -6 route show default table %s", table);
	fp = popen(cmd, "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			if (strstr(buf, ifname)) {
				ret = 1;
				break;
			}
		}
		pclose(fp);
	}

	return (ret);
}
/*
//exclude nexthop case
static int is_mtwan6_def_route_exist_ex(const char *ifname, const char *table)
{
	FILE *fp;
	char cmd[256];
	char buf[256];
	int ret = 0;

	if (!ifname || !table)
		return 0;

	snprintf(cmd, sizeof(cmd), "ip -6 route show default dev %s table %s", ifname, table);
	fp = popen(cmd, "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			if (strlen(buf)) {
				ret = 1;
				break;
			}
		}
		pclose(fp);
	}

	return (ret);
}
*/
void mtwan6_handle_ip_route(int unit)
{
	char gateway[INET6_ADDRSTRLEN] = {0};
	char table[8] = {0};
	char wan_prefix[16] = {0};
	int wan_proto;
	char ifname[IFNAMSIZ] = {0};
	int service;
	char *server;
	char ra_route[128] = {0}, *ra_routes = NULL, *next = NULL;
	char *ra_network, *ra_gateway, *ra_valid, *ra_metric;

	mtwan_get_route_table_id(unit, table, sizeof(table));
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", unit);
	wan_proto = get_wan_proto(wan_prefix);
	strlcpy(ifname, get_wan6_ifname(unit), sizeof(ifname));
	service = get_ipv6_service_by_unit(unit);

	_dprintf("\n%s: %d,%s,%d\n", __FUNCTION__, unit, ifname, service);

	switch(service) {
		case IPV6_PASSTHROUGH:
		case IPV6_NATIVE_DHCP: {
#if !defined(RTCONFIG_MULTIWAN_PROFILE)
			if (is_mtwan_unit(unit))
#endif
			{
				// RA_ROUTES
				ra_routes = safe_getenv("RA_ROUTES");
				if (*ra_routes) {
					// called by odhcp6c
					foreach(ra_route, ra_routes, next) {
						if ((vstrsep(ra_route, ",", &ra_network, &ra_gateway, &ra_valid, &ra_metric) < 3))
							continue;
						if (*ra_gateway) {
							nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), ra_gateway);
							eval("ip", "-6", "route", "add", ra_network, "via", ra_gateway, "dev", ifname, "table", table, "expires", ra_valid, "metric", ra_metric);
						}
						else
							eval("ip", "-6", "route", "add", ra_network, "dev", ifname, "table", table, "expires", ra_valid, "metric", ra_metric);
					}
				}

				// default route
				if (!is_mtwan6_def_route_exist(ifname, table)) {
					if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
					 && nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")
					) {
						strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_llremote", unit)), sizeof(gateway));
						if (*gateway) {
							nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), gateway);
							eval("ip", "-6", "route", "add", "default", "via", gateway, "dev", ifname, "table", table);
						}
					}
					else if ((server = safe_getenv("SERVER")) && *server) {
						eval("ip", "-6", "route", "add", "default", "via", server, "dev", ifname, "table", table);
						nvram_set(ipv6_nvname_by_unit("ipv6_gateway", unit), server);
					}
					else if (is_mtwan6_def_route_exist(ifname, "main")) {
						strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", unit)), sizeof(gateway));
						if (*gateway) {
							eval("ip", "-6", "route", "add", "default", "via", gateway, "dev", ifname, "table", table);
						}
					}
				}
			}
			break;
		}

		case IPV6_MANUAL: {
			eval("ip", "-6", "route", "del", "default", "table", table);
			strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_gateway", unit)), sizeof(gateway));
			if (*gateway) {
				eval("ip", "-6", "route", "add", "default", "via", gateway, "dev", ifname, "table", table);
			}
			else if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
				 && nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")
			) {
				strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_llremote", unit)), sizeof(gateway));
				if (*gateway) {
					eval("ip", "-6", "route", "add", "default", "via", gateway, "dev", ifname, "table", table);
				}
			}
			break;
		}
	}

	eval("ip", "-6", "route", "flush", "cache");
}

void mtwan6_handle_new_default_route(const char* ifname, const char* gateway)
{
	int wan_unit;
	char table[8] = {0};

	if (!ifname || !*ifname || !gateway || !*gateway)
		return;

	// _dprintf("%s: %s\n", __FUNCTION__, ifname);

	pthread_mutex_lock(&mtwanduck_chkrt);

	wan_unit = get_wan6_unit((char *)ifname);
	mtwan_get_route_table_id(wan_unit, table, sizeof(table));
	if (!is_mtwan6_def_route_exist(ifname, table))
	{
		if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_gateway", wan_unit), gateway))
			nvram_set(ipv6_nvname_by_unit("ipv6_gateway", wan_unit), gateway);

		mtwan6_handle_ip_route(wan_unit);
		mtwan6_handle_ip_rule(wan_unit);

#ifdef RTCONFIG_MULTIWAN_PROFILE
		add_mtwan_ipv6_mangle_rules(wan_unit);
		if (is_mtwan_lb(wan_unit))
		{
			mtwan6_update_lb_route(wan_unit, MTWAN_CONNECTED);
			mtwan_update_lb_iptables(wan_unit, MTWAN_CONNECTED, MTWAN_HANDLE_V6);
		}
#endif
	}

#ifdef RTCONFIG_MULTIWAN_PROFILE
	if (is_mtwan6_primary(wan_unit))
		mtwan6_update_main_default_route();
#endif
/*
#ifdef RTCONFIG_MULTIWAN_PROFILE
	if (!is_mtwan6_primary(wan_unit) && !is_mtwan_primary_group(wan_unit))
#else
	if (is_mtwan_unit(wan_unit))
#endif
	{
		// _dprintf("Delete %s default route\n", ifname);
		eval("ip", "-6", "route", "del", "default", "dev", (char *)ifname);
		eval("ip", "-6", "route", "flush", "cache");
	}
*/
	pthread_mutex_unlock(&mtwanduck_chkrt);
}

void mtwan6_handle_new_addr(const char* ifname, const char* addr, int prefix_length)
{
	int wan_unit;

	if (!ifname || !*ifname || !addr || !*addr)
		return;

	// _dprintf("%s: %s: %s/%d\n", __FUNCTION__, ifname, addr, prefix_length);

	wan_unit = get_wan6_unit((char *)ifname);

	if (wan_unit == WAN_UNIT_NONE)
		return;

	pthread_mutex_lock(&mtwanduck_chkaddr);

	if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit), addr)) {
		nvram_set(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit), addr);
		add_mtwan_ipv6_nat_rules(wan_unit);
	}

	pthread_mutex_unlock(&mtwanduck_chkaddr);
}

void mtwan6_check_route()
{
	int unit, wan_unit;
	char table[8] = {0};
	char ifname[IFNAMSIZ] = {0};
	int accept_defrtr;
	int service;
	int wan_proto;

	pthread_mutex_lock(&mtwanduck_chkrt);

	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++) {
		wan_unit = mtwan_get_mapped_unit(unit);
		service = get_ipv6_service_by_unit(wan_unit);
		if (service == IPV6_DISABLED)
			continue;
#if !defined(RTCONFIG_MULTIWAN_PROFILE)
		if (is_mtwan_unit(wan_unit))
#endif
		{
			wan_proto = get_ipv4_service_by_unit(wan_unit);
			accept_defrtr = (service == IPV6_NATIVE_DHCP) &&
				(wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP) &&
				nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp") ?
				nvram_get_int(ipv6_nvname_by_unit("ipv6_accept_defrtr", wan_unit)) : 1;

			if (accept_defrtr)
			{
				strlcpy(ifname, get_wan6_ifname(wan_unit), sizeof(ifname));
				mtwan_get_route_table_id(wan_unit, table, sizeof(table));
				// check mtwan route
				if (if_nametoindex(ifname) && is_mtwan6_def_route_exist(ifname, "main"))
				{
					if (!is_mtwan6_def_route_exist(ifname, table))
					{
						if (nvram_is_empty(ipv6_nvname_by_unit("ipv6_gateway", wan_unit)))
							_set_mtwan6_def_gateway_by_route(wan_unit, ifname, "main");

						mtwan6_handle_ip_route(wan_unit);
						mtwan6_handle_ip_rule(wan_unit);

#ifdef RTCONFIG_MULTIWAN_PROFILE
						add_mtwan_ipv6_mangle_rules(wan_unit);
						if (is_mtwan_lb(wan_unit))
						{
							mtwan6_update_lb_route(wan_unit, MTWAN_CONNECTED);
							mtwan_update_lb_iptables(wan_unit, MTWAN_CONNECTED, MTWAN_HANDLE_V6);
						}
						if (is_mtwan6_primary(wan_unit))
							mtwan6_update_main_default_route();
#endif
					}
/*
#ifdef RTCONFIG_MULTIWAN_PROFILE
					if (!is_mtwan6_primary(wan_unit) && !is_mtwan_primary_group(wan_unit))
#else
					if (is_mtwan_unit(wan_unit))
#endif
					{
						// _dprintf("Delete %s default route\n", ifname);
						eval("ip", "-6", "route", "del", "default", "dev", ifname);
						eval("ip", "-6", "route", "flush", "cache");
					}
*/
				}
			}
		}
	}

	pthread_mutex_unlock(&mtwanduck_chkrt);
}

void mtwan6_check_addr()
{
	int unit, wan_unit;
	char ifname[IFNAMSIZ] = {0};
	char addr[INET6_ADDRSTRLEN+4];

	pthread_mutex_lock(&mtwanduck_chkaddr);

	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++) {
		wan_unit = mtwan_get_mapped_unit(unit);
		if (!ipv6x_enabled(wan_unit))
			continue;
		if (!nvram_is_empty(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit)))
			continue;
		strlcpy(ifname, get_wan6_ifname(wan_unit), sizeof(ifname));
		memset(addr, 0 , sizeof(addr));
		if (if_nametoindex(ifname))
			strlcpy(addr, getifaddr(ifname, AF_INET6, GIF_PREFIXLEN), sizeof(addr));
		if (*addr) {
			nvram_set(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit), addr);
			add_mtwan_ipv6_nat_rules(wan_unit);
		}
	}

	pthread_mutex_unlock(&mtwanduck_chkaddr);
}

void mtwan6_check_dns()
{
	int unit, wan_unit;
	int service;
	char path1[128], path2[128];
	char wan_ifname[IFNAMSIZ];

	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++) {
		wan_unit = mtwan_get_mapped_unit(unit);
		service = get_ipv6_service_by_unit(wan_unit);
		if (
#ifdef RTCONFIG_6RELAYD
		service != IPV6_PASSTHROUGH &&
#endif
		service != IPV6_NATIVE_DHCP)
			continue;
		if (nvram_get_int(ipv6_nvname_by_unit("ipv6_dnsenable", wan_unit)) == 0)
			continue;
		if (!nvram_is_empty(ipv6_nvname_by_unit("ipv6_get_dns", wan_unit)))
			continue;
		strlcpy(wan_ifname, get_wan6_ifname(wan_unit), sizeof(wan_ifname));
		if (!getifaddr(wan_ifname, AF_INET6, 0))
			continue;
		snprintf(path1, sizeof(path1), "/tmp/wan%d_bound6.env", wan_unit);
		snprintf(path2, sizeof(path2), "/tmp/wan%d_ra.env", wan_unit);
		if (!f_exists(path1) && !f_exists(path2)) {
			snprintf(path1, sizeof(path1), "/var/run/odhcp6c.%s.pid", wan_ifname);
			kill_pidfile_s(path1, SIGPOLL);
			kill_pidfile_s(path1, SIGUSR1);
			usleep(100000);
			eval("rdisc6", wan_ifname);
		}
	}
}
