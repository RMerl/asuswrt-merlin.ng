#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <shared.h>

#include "rc.h"
#include "multi_wan.h"

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

	update_mtwan_ip_rule_ipv6(unit);
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

	service = get_ipv6_service_by_unit(unit);
	if (
#ifdef RTCONFIG_6RELAYD
		service != IPV6_PASSTHROUGH &&
#endif
		service != IPV6_NATIVE_DHCP)
		return;

	if (nvram_get_int("ipv6_no_rdisc6"))
		return;

	stop_rdisc6();

	strlcpy(wan6_ifname, get_wan6_ifname(unit), sizeof(wan6_ifname));
	_eval(rdisc6_argv, NULL, 0, &pid);
}

void stop_mtwan_rdisc6(int unit)
{
	stop_rdisc6();
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
		break;
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_llremote", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_dns", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_domain", unit), "");
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

	if (is_mtwan_unit(unit)) {
		mtwan_get_route_table_id(unit, table, sizeof(table));
		eval("ip", "-6", "route", "flush", "table", table);
	}

	nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_llremote", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_get_dns", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_get_domain", unit), "");
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
		accept_defrtr = //(service == IPV6_NATIVE_DHCP)
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
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan_by_idx(SDNFT_TYPE_WAN6, wan_unit, pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i) {
			if (pmtl[i].enable)
			{
				set_intf_ipv6_dad(pmtl[i].nw_t.ifname, 0, 1);
				enable_ipv6(pmtl[i].nw_t.ifname);
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
#else
	set_intf_ipv6_dad(nvram_safe_get("lan_ifname"), 0, 1);
	enable_ipv6(nvram_safe_get("lan_ifname"));
#endif
	config_mtwan_ipv6(wan_unit, 1);
	start_mtwan_ipv6(wan_unit);
}

void stop_mtwan_lan_ipv6(int wan_unit)
{
#ifdef RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan_by_idx(SDNFT_TYPE_WAN6, wan_unit, pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i) {
			if (pmtl[i].enable)
			{
				set_intf_ipv6_dad(pmtl[i].nw_t.ifname, 0, 0);
				disable_ipv6(pmtl[i].nw_t.ifname);
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
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan_by_idx(SDNFT_TYPE_WAN6, wan_unit, pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i) {
			if (pmtl[i].enable)
				start_dnsmasq(pmtl[i].nw_t.idx);
		}
		FREE_MTLAN((void *)pmtl);
	}
#else
	start_dnsmasq();
#endif
}

#ifdef RTCONFIG_SOFTWIRE46
void mtwan_append_s46_resolvconf_ipv6(int wan_unit)
{
	char wan_prefix[16];
	int wan_proto;
	char wan_dns_buf[256];
	char wan_dns[64], *next_dns;
	FILE *fp;

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);

	wan_proto = get_wan_proto(wan_prefix);
	switch(wan_proto) {
	case WAN_V6PLUS:
	case WAN_OCNVC:
		strlcpy(wan_dns_buf, nvram_safe_get(ipv6_nvname_by_unit("ipv6_get_dns", wan_unit)), sizeof(wan_dns_buf));
		foreach (wan_dns, wan_dns_buf, next_dns) {
			if (doSystem("cat /tmp/resolv.conf | grep %s", wan_dns)) {
				fp = fopen("/tmp/resolv.conf", "a");
				if(fp) {
					fprintf(fp, "nameserver %s\n", wan_dns);
					fclose(fp);
				}
			}
		}
		break;
	}
}
#endif

static void _remove_ip_rules_ipv6_dns(int pref)
{
	char buf[256], pref_str[8];
	FILE *fp;
	char *ip6rule_tmp = "/tmp/ip6rule_tmp";

	snprintf(pref_str, sizeof(pref_str), "%d", pref);
	snprintf(buf, sizeof(buf), "ip -6 rule show | grep %d | grep \"iif lo\" > %s", pref, ip6rule_tmp);
	system(buf);
	fp = fopen(ip6rule_tmp, "r");
	if(fp) {
		while(fgets(buf, sizeof(buf), fp)) {
			eval("ip", "-6", "rule", "del", "iif", "lo", "pref", pref_str);
		}
		fclose(fp);
	}
	unlink(ip6rule_tmp);
}

void update_mtwan_ip_rule_ipv6(int unit)
{
	int pref_num;
	char table[8], pref[8];
	char wan_dns_buf[256];
	char wan_dns[64], *next_dns;
	int n;
	char nvname[16];

	pref_num = mtwan_get_route_rule_pref(unit);
	snprintf(pref, sizeof(pref), "%d", pref_num);
	mtwan_get_route_table_id(unit, table, sizeof(table));

	_remove_ip_rules_ipv6_dns(pref_num);

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
}

static void _add_mtwan_ipv6_def_route(const char* ifname, const char* table)
{
	FILE *fp;
	char cmd[256];
	char buf[256];
	char *p;

	snprintf(cmd, sizeof(cmd), "ip -6 route show dev %s", ifname);
	fp = popen(cmd, "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			trimNL(buf);
			if (strstr(buf, "default") && (p = strstr(buf, "proto"))) {
				*p = '\0';
				doSystem("ip -6 route del default dev %s table %s", buf, ifname, table);
				doSystem("ip -6 route add %s dev %s table %s", buf, ifname, table);
			}
		}
		pclose(fp);
	}
}

void add_mtwan_ipv6_route(int unit, char *ifname, int service)
{
	char gateway[INET6_ADDRSTRLEN] = {0};
	char table[8] = {0};
	char wan_prefix[16] = {0};
	int wan_proto;
	char *server;

	_dprintf("\n%s: %d,%s,%d\n", __FUNCTION__, unit, ifname, service);

	mtwan_get_route_table_id(unit, table, sizeof(table));
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", unit);
	wan_proto = get_wan_proto(wan_prefix);

	switch(service) {
		case IPV6_PASSTHROUGH:
		case IPV6_NATIVE_DHCP: {
			if (is_mtwan_unit(unit)) {
				if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
				 && nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")
				) {
					strlcpy(gateway, nvram_safe_get(ipv6_nvname_by_unit("ipv6_llremote", unit)), sizeof(gateway));
					if (*gateway) {
						eval("ip", "-6", "route", "add", "default", "via", gateway, "dev", ifname, "table", table);
					}
				}
				else if ((server = safe_getenv("SERVER")) != NULL) {
					eval("ip", "-6", "route", "add", "default", "via", server, "dev", ifname, "table", table);
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
}

static int _have_mtwan_ipv6_def_route(const char *ifname, const char *table)
{
	FILE *fp;
	char cmd[256];
	char buf[256];
	int ret = 0;

	snprintf(cmd, sizeof(cmd), "ip -6 route show dev %s table %s", ifname, table);
	fp = popen(cmd, "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			if (strstr(buf, "default")) {
				ret = 1;
			}
		}
		pclose(fp);
	}

	return (ret);
}

void check_mtwan_ipv6_route()
{
	int unit, wan_unit;
	char table[8] = {0};
	char ifname[IFNAMSIZ] = {0};

	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++) {
		wan_unit = mtwan_get_mapped_unit(unit);
		if (is_mtwan_unit(wan_unit)) {
			if (nvram_get_int(ipv6_nvname_by_unit("ipv6_accept_defrtr", wan_unit))) {
				strlcpy(ifname, get_wan6_ifname(wan_unit), sizeof(ifname));
				if (_have_mtwan_ipv6_def_route(ifname, "main")) {
					mtwan_get_route_table_id(wan_unit, table, sizeof(table));
					_add_mtwan_ipv6_def_route(ifname, table);
					eval("ip", "-6", "route", "del", "default", "dev", ifname);
				}
			}
		}
	}
}
