/*
 * Copyright 2023, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include <multi_wan.h>
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
#include <linux/sockios.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlutils.h>
#include <bcmutils.h>
#include <bcmparams.h>
#include <net/route.h>
#include <stdarg.h>
#include <sys/wait.h>

#include <linux/types.h>
#include <shared.h>

#ifdef RTCONFIG_MULTILAN_CFG
#include <mtlan_utils.h>
#include <sdn.h>
#endif

#ifdef RTCONFIG_MULTIWAN_PROFILE
MTWAN_PROF_T mtwan_prof[MAX_MTWAN_PROFILE_NUM] = {0};
#endif

#define IS_MTWAN_UNIT(x) (x >= MULTI_WAN_START_IDX && x < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)

extern int g_upgrade;

char wanX_resolv_path[] = "/tmp/resolv.wan%d";

static int _is_dualwan_enable()
{
	char *dualwan = nvram_safe_get("wans_dualwan");

	return strstr(dualwan, "none")? 0: 1;
}

int mtwan_get_real_wan(const int unit, char *prefix, const size_t len)
{
	char tmp[256], prefix_tmp[] = "wanXXXX_", *dualwan_str;
	int dualwan_idx;

	if (!prefix)
		return -1;

	if (unit < WAN_UNIT_MAX && unit >= WAN_UNIT_FIRST)
	{
		snprintf(prefix, len, "wan%d_", unit);
		return unit;
	}

	snprintf(prefix_tmp, sizeof(prefix_tmp), "wan%d_", unit);

#ifdef RTCONFIG_MULTISERVICE_WAN
	if (is_ms_wan_unit(unit))
	{
		strlcpy(prefix, prefix_tmp, len);
		return unit;
	}
#endif

	dualwan_str = nvram_get(strlcat_r(prefix_tmp, "dualwan_idx", tmp, sizeof(tmp)));
	if(!dualwan_str)
		return -1;

	dualwan_idx = atoi(dualwan_str);

	if (dualwan_idx == WAN_UNIT_NONE)
	{
		strlcpy(prefix, prefix_tmp, len);
		return unit;
	}
	else if (dualwan_idx == WAN_UNIT_FIRST || dualwan_idx == WAN_UNIT_SECOND)
	{
		snprintf(prefix, len, "wan%d_", dualwan_idx);
		return dualwan_idx;
	}
	else
	{
		return -1;
	}
}

int mtwan_get_unit_by_dualwan_unit(const int dualwan_unit)
{
	int i;
	char name[64];

	for(i = 0; i < MAX_MULTI_WAN_NUM; ++ i)
	{
		snprintf(name, sizeof(name), "wan%d_dualwan_idx", MULTI_WAN_START_IDX + i);
		if(nvram_get_int(name) == dualwan_unit)
			return MULTI_WAN_START_IDX + i;
	}
	return -1;
}
/*
static int _remove_mtwan_routing(const int unit)
{
	char table[32];

	//remove ip rule
	remove_ip_rules(mtwan_get_route_rule_pref(unit));

	//flush routing table
	if(mtwan_get_route_table_id(unit, table, sizeof(table)) != -1)
	{
		eval("ip", "route", "flush", "table", table);
	}
	return 0;
}

static int _remove_iptables(const char *ifname)
{
	static const char iptables_tmp[] = "/tmp/iptables_tmp";
	const char *tables[] = {"filter", "nat", "mangle", NULL};
	char tmp[1024], cmd[1024];
	int i;
	FILE *fp;

	if(!ifname)
		return -1;

	for(i = 0; tables[i] != NULL; ++i)
	{
		snprintf(tmp, sizeof(tmp), "iptables-save -t %s | grep %s > %s", tables[i], ifname, iptables_tmp);
		system(tmp);

		eval("sed", "-i", "s/-I/-D/g", (char*)iptables_tmp);
		eval("sed", "-i", "s/-A/-D/g", (char*)iptables_tmp);

		fp = fopen(iptables_tmp, "r");
		if(fp)
		{
			while(fgets(tmp, sizeof(tmp), fp))
			{
				snprintf(cmd, sizeof(cmd), "iptables -t %s %s", tables[i], tmp);
				system(cmd);
			}
			fclose(fp);
		}
		unlink(iptables_tmp);
	}
	return 0;
}

static void _show_routing_table(const int unit)
{
	char buf[2048], table[16];
	FILE *fp;

	mtwan_get_route_table_id(unit, table, sizeof(table));
	snprintf(buf, sizeof(buf), "ip route show table %s > /tmp/rttmp", table);
	system(buf);
	fp = fopen("/tmp/rttmp", "r");

	_dprintf("[%s]wan%d routing table:\n", __FUNCTION__, unit);
	if(fp)
	{
		while(fgets(buf, 2048, fp))
		{
			_dprintf("%s", buf);
		}
		_dprintf("\n");
		fclose(fp);
	}
	unlink("/tmp/rttmp");

}
*/

#ifdef RTCONFIG_MULTIWAN_PROFILE
int is_mtwan_enable()
{
	char *wans_mtwan = nvram_safe_get("wans_mt_ioport");

	return (strchr(wans_mtwan, ' ') && strlen(wans_mtwan) > 4) ? 1 : 0;
}

int is_mtwan_lb_in_profile(int mtwan_idx, int wan_unit)
{
	int i, j, k;
	int cnt;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || wan_unit < WAN_UNIT_FIRST)
		return 0;

	i = mtwan_idx - 1;
	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].wan_units[j] == wan_unit)
		{
			cnt = 0;
			for (k = 0; k < MAX_MULTI_WAN_NUM; k++)
			{
				if (mtwan_prof[i].mt_group[j] == mtwan_prof[i].mt_group[k])
					cnt++;
			}
			if (cnt > 1)
				return 1;
		}
	}

	return 0;
}

/// wan unit is load balance in any Multi-WAN profile
int is_mtwan_lb(int unit)
{
	int i;

	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++)
	{
		if (mtwan_prof[i].enable == 0)
			continue;
		if (is_mtwan_lb_in_profile(i + 1, unit))
			return 1;
	}
	return 0;
}

int is_mtwan_primary(int wan_unit)
{
	char nvname[32];
	snprintf(nvname, sizeof(nvname), "wan%d_primary", wan_unit);
	return (nvram_get_int(nvname)) ? 1 : 0;
}

int is_mtwan_primary_group(int wan_unit)
{
	int primary_unit = nvram_get_int("wan_primary");
	int i;
	int pri_grp_idx = -1, wan_grp_idx = -1;

	if (wan_unit == primary_unit)
		return 1;

	for (i = 0; i < MAX_MULTI_WAN_NUM; i++)
	{
		if (mtwan_prof[0].wan_units[i] == primary_unit)
			pri_grp_idx = i;
	}
	for (i = 0; i < MAX_MULTI_WAN_NUM; i++)
	{
		if (mtwan_prof[0].wan_units[i] == wan_unit)
			wan_grp_idx = i;
	}
	return (mtwan_prof[0].mt_group[pri_grp_idx] == mtwan_prof[0].mt_group[wan_grp_idx]);
}

//handle with nvram
int mtwan_get_num_of_wan_by_group(int mtwan_idx, int mtwan_group)
{
	int cnt = 0;
	char nvname[32] = {0}, mt_group[32] = {0};
	char word[4] = {0}, *next = NULL;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || mtwan_group < 1 || mtwan_group > MAX_MULTI_WAN_NUM)
		return 0;

	snprintf(nvname, sizeof(nvname), "mtwan%d_mt_group", mtwan_idx);
	strlcpy(mt_group, nvram_safe_get(nvname), sizeof(mt_group));
	foreach(word, mt_group, next)
	{
		if (mtwan_group == atoi(word))
			cnt++;
	}
	return (cnt);
}

int is_mtwan_group_lb(int mtwan_idx, int mtwan_group)
{
	return (mtwan_get_num_of_wan_by_group(mtwan_idx, mtwan_group) > 1) ? 1 : 0;
}

int is_mtwan_group_in_profile(int mtwan_idx, int mtwan_group)
{
	int i = mtwan_idx - 1;
	int j;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || mtwan_group < 1 || mtwan_group > MAX_MULTI_WAN_NUM)
		return 0;

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].mt_group[j] == mtwan_group)
			return 1;
	}

	return 0;
}

int is_mtwan_unit_in_profile(int mtwan_idx, int wan_unit)
{
	int i = mtwan_idx - 1;
	int j;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || wan_unit < WAN_UNIT_FIRST)
		return 0;

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].wan_units[j] == wan_unit)
			return 1;
	}

	return 0;
}

int is_mtwan_unit_in_active_group(int mtwan_idx, int wan_unit)
{
	int i = mtwan_idx - 1;
	int j;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || wan_unit < WAN_UNIT_FIRST)
		return 0;

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].wan_units[j] == wan_unit
		 && mtwan_prof[i].mt_group[j] == mtwan_prof[i].group
		)
			return 1;
	}

	return 0;
}

int mtwan_get_lb_route_table_id(int mtwan_idx, int mtwan_group, char *table, size_t table_len)
{
	int ret;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || mtwan_group < 1 || mtwan_group > MAX_MULTI_WAN_NUM)
		return -1;

	ret = IP_ROUTE_TABLE_ID_MTWAN_LB_BASE + mtwan_idx * 10 + mtwan_group;
	if (table)
		snprintf(table, table_len, "%d", ret);
	return (ret);
}

int mtwan_get_lb_rule_pref(int mtwan_idx, char *pref, size_t pref_len)
{
	int ret;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM)
		return -1;

	ret = IP_RULE_PREF_MTWAN_LB_BASE + mtwan_idx;
	if (pref)
		snprintf(pref, pref_len, "%d", ret);
	return (ret);
}

int mtwan_get_mark_rule_pref(int unit)
{
	int mtwan_unit = unit;

	if(unit < MULTI_WAN_START_IDX)
		mtwan_unit = mtwan_get_unit_by_dualwan_unit(unit);

	return IP_RULE_PREF_MTWAN_MARK_BASE + mtwan_unit;
}

void mtwan_update_profile_lb_route(int mtwan_idx, int mtwan_group, int unit, int up)
{
	char table[8];
	char cmd[256];
	int i = mtwan_idx -1;
	int j;
	char wan_prefix[16];
	int wan_proto;
	char network[INET_ADDRSTRLEN+3];
	char *gateway, *gw_ifname, *xgateway, *ifname;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || mtwan_group < 1 || mtwan_group > MAX_MULTI_WAN_NUM)
		return;

	/// flush
	mtwan_get_lb_route_table_id(mtwan_idx, mtwan_group, table, sizeof(table));
	eval("ip", "route", "flush", "table", table);

	snprintf(cmd, sizeof(cmd), "ip route replace default table %s", table);

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].mt_group[j] == mtwan_group)
		{
			/// wan route
			snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mtwan_prof[i].wan_units[j]);
			wan_proto = get_wan_proto(wan_prefix);
			gateway = nvram_pf_get(wan_prefix, "gateway") ?: "0.0.0.0";
			gw_ifname = nvram_pf_safe_get(wan_prefix, "gw_ifname");

			switch(wan_proto)
			{
				case WAN_DHCP:
				case WAN_STATIC:
					get_network_addr_by_ip_prefix(nvram_pf_safe_get(wan_prefix, "ipaddr"), nvram_pf_safe_get(wan_prefix, "netmask"), network, sizeof(network));
					eval("ip", "route", "add", network, "via", gateway, "dev", gw_ifname, "table", table);
					eval("ip", "route", "add", gateway, "dev", gw_ifname, "table", table);
					break;
				case WAN_PPPOE:
				case WAN_PPTP:
				case WAN_L2TP:
				case WAN_V6PLUS:
				case WAN_OCNVC:
					eval("ip", "route", "add", gateway, "dev", gw_ifname, "table", table);
					if (nvram_pf_get_int(wan_prefix, "vpndhcp"))
					{
						xgateway = nvram_pf_get(wan_prefix, "xgateway") ?: "0.0.0.0";
						ifname = nvram_pf_safe_get(wan_prefix, "ifname");
						get_network_addr_by_ip_prefix(nvram_pf_safe_get(wan_prefix, "xipaddr"), nvram_pf_safe_get(wan_prefix, "xnetmask"), network, sizeof(network));
						eval("ip", "route", "add", network, "via", xgateway, "dev", ifname, "table", table);
						eval("ip", "route", "add", xgateway, "dev", ifname, "table", table);
					}
					break;
			}

			/// default route
			if ((unit != mtwan_prof[i].wan_units[j]) ? (nvram_pf_get_int(wan_prefix, "link_internet") == 2) : ((up) ? 1 : 0))
			{
				if (wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
					snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via 0.0.0.0");
				else
					snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via %s"
						, nvram_pf_get(wan_prefix, "gateway") ?: "0.0.0.0");
				snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " weight %d dev %s"
					, mtwan_prof[i].mt_weight[j]
					, nvram_pf_safe_get(wan_prefix, "gw_ifname")
					);
			}
		}
	}
	/// default route
	_dprintf("cmd: %s\n", cmd);
	system(cmd);

	eval("ip", "route", "flush", "cache");
}

void mtwan_update_main_default_route(int unit, int up)
{
	char cmd[256];
	int i;
	int wan_unit;
	char wan_prefix[16];
	int wan_proto;

	if (mtwan_prof[0].enable == 0)
	{
		snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());
		snprintf(cmd, sizeof(cmd), "ip route replace default via %s dev %s"
			, nvram_pf_get(wan_prefix, "gateway") ?: "0.0.0.0"
			, nvram_pf_safe_get(wan_prefix, "gw_ifname"));
		_dprintf("cmd: %s\n", cmd);
		system(cmd);
		eval("ip", "route", "flush", "cache");
		return;
	}

	snprintf(cmd, sizeof(cmd), "ip route replace default");
	for(i = 0; i < MAX_MULTI_WAN_NUM; i++)
	{
		if (mtwan_prof[0].mt_group[i] == mtwan_prof[0].group)
		{
			wan_unit = mtwan_prof[0].wan_units[i];
			snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
			wan_proto = get_wan_proto(wan_prefix);
			if ((unit != wan_unit) ? (nvram_pf_get_int(wan_prefix, "link_internet") == 2) : ((up) ? 1 : 0))
			{
				if (wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
					snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via 0.0.0.0");
				else
					snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via %s"
						, nvram_pf_get(wan_prefix, "gateway") ?: "0.0.0.0");
				snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " weight %d dev %s"
					, mtwan_prof[0].mt_weight[i]
					, nvram_pf_safe_get(wan_prefix, "gw_ifname")
					);
			}
		}
	}
	_dprintf("cmd: %s\n", cmd);
	system(cmd);
	eval("ip", "route", "flush", "cache");
}

void mtwan_update_lb_route(int wan_unit, int up)
{
	int i, j;
	char table[8] = {0};
	char cmd[256] = {0};
	int mtwan_idx, mtwan_group;
	char wan_prefix[16] = {0};
	int real_unit, wan_proto;
	char network[INET_ADDRSTRLEN+3] = {0};
	char *gateway, *gw_ifname, *xgateway, *ifname;

	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++)
	{
		if (mtwan_prof[i].enable == 0)
			continue;

		mtwan_idx = i + 1;
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
		mtwan_get_lb_route_table_id(mtwan_idx, mtwan_group, table, sizeof(table));
		eval("ip", "route", "flush", "table", table);

		snprintf(cmd, sizeof(cmd), "ip route replace default table %s", table);

		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] == mtwan_group)
			{
				/// wan route
				real_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
				wan_proto = get_wan_proto(wan_prefix);
				gateway = nvram_pf_get(wan_prefix, "gateway") ?: "0.0.0.0";
				gw_ifname = nvram_pf_safe_get(wan_prefix, "gw_ifname");

				switch(wan_proto)
				{
					case WAN_DHCP:
					case WAN_STATIC:
						get_network_addr_by_ip_prefix(nvram_pf_safe_get(wan_prefix, "ipaddr"), nvram_pf_safe_get(wan_prefix, "netmask"), network, sizeof(network));
						eval("ip", "route", "add", network, "via", gateway, "dev", gw_ifname, "table", table);
						eval("ip", "route", "add", gateway, "dev", gw_ifname, "table", table);
						break;
					case WAN_PPPOE:
					case WAN_PPTP:
					case WAN_L2TP:
					case WAN_V6PLUS:
					case WAN_OCNVC:
						eval("ip", "route", "add", gateway, "dev", gw_ifname, "table", table);
						if (nvram_pf_get_int(wan_prefix, "vpndhcp"))
						{
							xgateway = nvram_pf_get(wan_prefix, "xgateway") ?: "0.0.0.0";
							ifname = nvram_pf_safe_get(wan_prefix, "ifname");
							get_network_addr_by_ip_prefix(nvram_pf_safe_get(wan_prefix, "xipaddr"), nvram_pf_safe_get(wan_prefix, "xnetmask"), network, sizeof(network));
							eval("ip", "route", "add", network, "via", xgateway, "dev", ifname, "table", table);
							eval("ip", "route", "add", xgateway, "dev", ifname, "table", table);
						}
						break;
				}
				/// default route
				if ((wan_unit != real_unit) ? (nvram_pf_get_int(wan_prefix, "link_internet") == 2) : ((up) ? 1 : 0))
				{
					if (wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
						snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via 0.0.0.0");
					else
						snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " nexthop via %s"
							, nvram_pf_get(wan_prefix, "gateway") ?: "0.0.0.0");
					snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " weight %d dev %s"
						, mtwan_prof[i].mt_weight[j]
						, nvram_pf_safe_get(wan_prefix, "gw_ifname")
						);
				}
			}
		}
		/// default route
		_dprintf("cmd: %s\n", cmd);
		system(cmd);

		/// main table default route
		if (mtwan_idx == 1 && mtwan_group == mtwan_prof[i].group)	// default Multi-WAN profile using same group
		{
			char new_cmd[256] = {0};
			char *p;
			p = strstr(cmd, "table");
			if (p) {
				*p = '\0';
				strlcat(new_cmd, cmd, sizeof(new_cmd));
				p = strstr(p + 1, "nexthop");
				if (p) {
					strlcat(new_cmd, p, sizeof(new_cmd));
				}
			}
			_dprintf("cmd: %s\n", new_cmd);
			system(new_cmd);
		}

		eval("ip", "route", "flush", "cache");
	}
}

/*
 * update wanX_prob with WAN STATE
 */
void mtwan_update_lb_prob(int unit, int up)
{
	char wan_prefix[16] = {0};
	int i, j;
	int real_unit;
	int total[MAX_MULTI_WAN_NUM+1] = {0};
	char buf[128] = {0}, prob[128] = {0};

	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++)
	{
		if (mtwan_prof[i].enable == 0)
			continue;
		memset(total, 0 , sizeof(total));
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] < 1 || mtwan_prof[i].mt_group[j] > MAX_MULTI_WAN_NUM)
				continue;
			if (mtwan_prof[i].mt_weight[j] < 1 || mtwan_prof[i].mt_weight[j] > MAX_MULTI_WAN_NUM)
				continue;
			real_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
			if (real_unit < 0)
				continue;
			// total weight
			if ((unit != real_unit) ? (nvram_pf_get_int(wan_prefix, "link_internet") == 2) : ((up) ? 1 : 0))
				total[mtwan_prof[i].mt_group[j]] += mtwan_prof[i].mt_weight[j];
		}
		// prob
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] < 1 || mtwan_prof[i].mt_group[j] > MAX_MULTI_WAN_NUM)
				continue;
			if (mtwan_prof[i].mt_weight[j] < 1 || mtwan_prof[i].mt_weight[j] > MAX_MULTI_WAN_NUM)
				continue;
			real_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
			if (real_unit < 0)
				continue;
			if ((unit != real_unit) ? (nvram_pf_get_int(wan_prefix, "link_internet") == 2) : ((up) ? 1 : 0))
			{
				mtwan_prof[i].prob[j] = (double)mtwan_prof[i].mt_weight[j] / total[mtwan_prof[i].mt_group[j]];
			}
			else
			{
				mtwan_prof[i].prob[j] = 0.0;
			}
		}
		memset(prob, 0, sizeof(prob));
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			snprintf(buf, sizeof(buf), "%f", mtwan_prof[i].prob[j]);
			if (prob[0])
				strlcat(prob, " ", sizeof(prob));
			strlcat(prob, buf, sizeof(prob));
		}
		snprintf(buf, sizeof(buf), "mtwan%d_mt_prob", i+1);
		nvram_set(buf, prob);
	}
}

void mtwan_update_lb_iptables(int unit, int up, int v6)
{
	char *xtables = (v6) ? "ip6tables" : "iptables";
	char wan_prefix[16] = {0};
	int i, j, wan_unit, wan_group;
	int wan_num[MAX_MULTI_WAN_NUM+1] = {0}, add_num[MAX_MULTI_WAN_NUM+1] = {0};
	char prob[16] = {0};
	char chain[16] = {0}, smark[32] = {0}, mmark[32] = {0};
	char table[8] = {0};

	mtwan_update_lb_prob(unit, up);

	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++)
	{
		if (unit == WAN_UNIT_NONE)
			wan_group = mtwan_prof[i].group;
		else
			wan_group = mtwan_prof[i].mt_group[is_mtwan_unit(unit) ? unit - MULTI_WAN_START_IDX : unit];
		if (wan_group == 0)
			continue;
		// count number of online wan in same group
		wan_num[wan_group] = 0;
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] == wan_group)
			{
				wan_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
				if (wan_unit == -1)
					continue;
				if (v6)
					mtwan_get_route_table_id(wan_unit, table, sizeof(table));
				if ((unit != wan_unit) ?
						(v6) ?
							is_mtwan6_def_route_exist(get_wan6_ifname(wan_unit), table)
							: (nvram_pf_get_int(wan_prefix, "link_internet") == 2)
						: ((up) ? 1 : 0)
					) {
					wan_num[wan_group]++;
				}
			}
		}
		// mangle rules
		snprintf(chain, sizeof(chain), "MTWAN_LB_%d_%d", i + 1, wan_group);
		eval(xtables, "-t", "mangle", "-N", chain);
		eval(xtables, "-t", "mangle", "-F", chain);
		add_num[wan_group] = 0;
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] == wan_group
			 && (mtwan_prof[i].prob[j] != 1.0 || mtwan_prof[i].prob[j] != 0.0)
			) {
				snprintf(smark, sizeof(smark), "0x%08x/0x%08x", IPTABLES_MARK_MTWAN_SET(j,0), IPTABLES_MARK_MTWAN_MASK);
				snprintf(mmark, sizeof(mmark), "0x0/0x%08x", IPTABLES_MARK_MTWAN_MASK);
				snprintf(prob, sizeof(prob), "%f", mtwan_prof[i].prob[j]);
				if ((++add_num[wan_group]) == wan_num[wan_group])
					eval(xtables, "-t", "mangle", "-A", chain, "-j", "CONNMARK",
						"-m", "connmark", "--mark", mmark, "--set-xmark", smark);
				else
					eval(xtables, "-t", "mangle", "-A", chain, "-j", "CONNMARK",
						"-m", "statistic", "--mode", "random", "--probability", prob, "--set-xmark", smark);
			}
		}
	}
}

/*
 * get wan unit by group. for only one wan in the group.
 * return mapped unit
 */
int mtwan_get_first_wan_unit_by_group(int mtwan_idx, int mtwan_group, int check_conn)
{
	int wan_unit;
	char wan_prefix[16] = {0};
	int i = mtwan_idx - 1;
	int j;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM
	 || mtwan_group < 1 || mtwan_group > MAX_MULTI_WAN_NUM)
		return WAN_UNIT_NONE;

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].mt_group[j] == mtwan_group)
		{
			wan_unit = mtwan_get_real_wan(MULTI_WAN_START_IDX + j, wan_prefix, sizeof(wan_prefix));
			if (check_conn && !mtwanduck_get_mtwan_status(MULTI_WAN_START_IDX + j))
				continue;
			else
				return wan_unit;
		}
	}

	return WAN_UNIT_NONE;
}

/*
 * compare wan group priority in mtwan profile
 * return 1: if priority of mtwan_group_1 is higher
 * return 0: same priority
 * return -1: if priority of mtwan_group_1 is lower, or any wan_group are not used in this mtwan profile.
 */
int mtwan_group_compare(int mtwan_idx, int mtwan_group_1, int mtwan_group_2)
{
	char mtwan_prefix[16];
	char mtwan_order[32];
	char word[4] = {0}, *next = NULL;
	int i, idx_1, idx_2;

	if (mtwan_group_1 == mtwan_group_2)
		return 0;

	snprintf(mtwan_prefix, sizeof(mtwan_prefix), "mtwan%d_", mtwan_idx);
	strlcpy(mtwan_order, nvram_pf_safe_get(mtwan_prefix, "order"), sizeof(mtwan_order));
	i = 0;
	idx_1 = -1;
	idx_2 = -1;
	foreach(word, mtwan_order, next)
	{
		if (atoi(word) == mtwan_group_1)
			idx_1 = i;
		if (atoi(word) == mtwan_group_2)
			idx_2 = i;
		i++;
	}

	if (idx_1 != -1 && idx_2 != -1)
		return (idx_1 < idx_2) ? 1 : -1;
	else
		return -1;
}

static int _is_mtwan_group_connected(int mtwan_idx, int mtwan_group, int unit, int up)
{
	char nvname[32] = {0};
	int i = mtwan_idx - 1;
	int j;
	int wan_unit;

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].mt_group[j] == mtwan_group)
		{
			wan_unit = mtwan_prof[i].wan_units[j];
			snprintf(nvname, sizeof(nvname), "wan%d_link_internet", wan_unit);
			if (mtwan_prof[i].mode == MTWAN_MODE_ANY)
			{
				if (unit != -1 && unit == wan_unit)
				{
					if (!up)
						return 0;
				}
				else
				{
					if (nvram_get_int(nvname) != 2)
						return 0;
				}
			}
			else if (mtwan_prof[i].mode == MTWAN_MODE_ALL)
			{
				if (unit != -1 && unit == wan_unit)
				{
					if (up)
						return 1;
				}
				else
				{
					if (nvram_get_int(nvname) == 2)
						return 1;
				}
			}
		}
	}

	if (mtwan_prof[i].mode == MTWAN_MODE_ANY)
		return 1;
	else
		return 0;
}

int mtwan_get_wan_group(int mtwan_idx, int wan_unit)
{
	int i = mtwan_idx - 1;
	int j;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM)
		return -1;

	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].wan_units[j] == wan_unit)
			return mtwan_prof[i].mt_group[j];
	}

	return -1;
}
/*
 * get next wan group in mtwan profile
 */
int mtwan_get_fo_group(int mtwan_idx, int check_conn)
{
	int i = mtwan_idx - 1;
	int j, cur_idx;

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM)
		mtwan_idx = 1;

	cur_idx = -1;
	for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
	{
		if (mtwan_prof[i].order[j] == mtwan_prof[i].group)
			cur_idx = j;
	}
	if (cur_idx == -1)
		return mtwan_prof[i].order[0];

	for (j = cur_idx + 1; j != cur_idx; j++)
	{
		if (j >= MAX_MULTI_WAN_NUM)
		{
			j -= MAX_MULTI_WAN_NUM;
			if (j == cur_idx)
				break;
		}

		if (!mtwan_prof[i].order[j])
			continue;

		if (check_conn)
		{
			if (_is_mtwan_group_connected(mtwan_idx, mtwan_prof[i].order[j], -1, 0))
				return mtwan_prof[i].order[j];
		}
		else
			return mtwan_prof[i].order[j];
	}

	return mtwan_prof[i].group;
}

int mtwan_get_first_group(int mtwan_idx)
{
	char nvname[32];
	snprintf(nvname, sizeof(nvname), "mtwan%d_order", mtwan_idx);
	return nvram_get_int(nvname);
}

int mtwan_get_second_group(int mtwan_idx)
{
	char nvname[32], value[32], *p;
	snprintf(nvname, sizeof(nvname), "mtwan%d_order", mtwan_idx);
	nvram_safe_get_r(nvname, value, sizeof(value));
	p = strchr(value, ' ');
	return (p && (p - value) < strlen(value)) ? atoi(p+1) : atoi(value);
}

/// add dns in same group
void mtwan_append_group_main_resolvconf(int wan_unit)
{
	int i, wan_grp_idx = -1, real_unit;
	int primary_unit = wan_primary_ifunit();
	char wan_prefix[16] = {0};
	FILE *fp;
#ifdef RTCONFIG_YANDEXDNS
	int yadns_mode = nvram_get_int("yadns_enable_x") ? nvram_get_int("yadns_mode") : YADNS_DISABLED;
#endif
#ifdef RTCONFIG_DNSPRIVACY
	int dnspriv_enable = nvram_get_int("dnspriv_enable");
#endif
#ifdef RTCONFIG_MULTIWAN_PROFILE
	int primary_unit6 = wan_primary_ifunit_ipv6();
#endif

	if (g_reboot || g_upgrade)
		return;

	if (!nvram_get_int("mtwan1_enable"))
		return;
	if (!mtwan_prof[0].enable)
		mtwan_init_profile();
	for (i = 0; i < MAX_MULTI_WAN_NUM; i++)
	{
		if (mtwan_prof[0].wan_units[i] == wan_unit)
		{
			wan_grp_idx = i;
			break;
		}
	}
	if (wan_grp_idx == -1)
		return;

	_dprintf("%s: add wan group %d dns\n", __FUNCTION__, mtwan_prof[0].mt_group[wan_grp_idx]);

	for (i = 0; i < MAX_MULTI_WAN_NUM; i++)
	{
		real_unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
		if (real_unit < 0)
			continue;
		if (real_unit < WAN_UNIT_MAX && real_unit == primary_unit)
			continue;
		if (mtwan_prof[0].mt_group[i] != mtwan_prof[0].mt_group[wan_grp_idx])
			continue;
		if (!mtwanduck_get_phy_status(i + MULTI_WAN_START_IDX))
			continue;

		fp = fopen("/tmp/resolv.conf", "a+");
		if (fp)
		{
			wan_add_resolv_conf(fp, real_unit);
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_PROFILE
			if (real_unit != primary_unit6 && ipv6x_enabled(real_unit))
#endif
			wan_add_resolv_conf_ipv6(fp, real_unit);
#endif
			fclose(fp);
		}

#ifdef RTCONFIG_YANDEXDNS
		if (yadns_mode != YADNS_DISABLED)
			continue;
#endif
#ifdef RTCONFIG_DNSPRIVACY
		if (dnspriv_enable)
			continue;
#endif

		fp = fopen("/tmp/resolv.dnsmasq", "a");
		if (fp)
		{
			wan_add_resolv_dnsmasq(fp, real_unit);
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_PROFILE
			if (real_unit != primary_unit6 && ipv6x_enabled(real_unit))
#endif
			wan_add_resolv_dnsmasq_ipv6(fp, real_unit);
#endif
			fclose(fp);
		}
	}
}

static void _mtwan_set_wan_primary(int primary_unit)
{
	int i;
	char buf[32];
	int unit;
#ifdef RTCONFIG_IPV6
	int set_v6 = (get_ipv6_service_by_unit(primary_unit) != IPV6_DISABLED);
#ifdef RTCONFIG_6RELAYD
	int old_primary_unit6 = wan_primary_ifunit_ipv6();
#endif
#endif

	// pre handle nat rules before renew primary unit since current nat_ruleXXX files are based on old primary unit.
	if (nvram_get_int("nat_state") != NAT_STATE_NORMAL)
		start_nat_rules();

	for (i = MULTI_WAN_START_IDX; i < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; i++) {
		unit = mtwan_get_mapped_unit(i);
		snprintf(buf, sizeof(buf), "wan%d_primary", unit);
		nvram_set(buf, "0");
#ifdef RTCONFIG_IPV6
		if (set_v6)
			nvram_set(ipv6_nvname_by_unit("ipv6_primary", unit), "0");
#endif
	}
	snprintf(buf, sizeof(buf), "wan%d_primary", primary_unit);
	nvram_set(buf, "1");
	nvram_set_int("wan_primary", primary_unit);
#ifdef RTCONFIG_IPV6
	if (set_v6)
		nvram_set(ipv6_nvname_by_unit("ipv6_primary", primary_unit), "1");
#ifdef RTCONFIG_6RELAYD
	mtwan6_restart_6relayd(1, old_primary_unit6, primary_unit);
#endif
#endif
}

static void _mtwan_switch_primary_wan_line(int wan_unit)
{
	char service[32] = {0};

	_dprintf("\nswitch primary wan to %d\n", wan_unit);

	_mtwan_set_wan_primary(wan_unit);

#ifdef RTCONFIG_MULTISERVICE_WAN
	if (nvram_match("switch_wantag", "none"))
	{
		update_iptv_ifname(wan_unit);
	}
#endif
#ifdef RTCONFIG_TR069
	tr_switch_wan_line(wan_unit);
#endif

	if (getpid() != 1)
	{
		snprintf(service, sizeof(service), "restart_wan_line %d", wan_unit);
		notify_rc(service);
	}
	else
	{
		char wan_ifname[IFNAMSIZ] = {0};
		strlcpy(wan_ifname, get_wan_ifname(wan_unit), sizeof(wan_ifname));
		wan_up(wan_ifname);
	}

	if (nvram_get_int("link_internet") != 2)
		mtwanduck_set_primary_link_internet(wan_unit, 2, 1);
}

void mtwan_handle_group_change(int mtwan_idx, int to_group)
{
	char mtwan_prefix[16];
	char prc[16] = {0};
#ifdef RTCONFIG_MULTIWAN_PROFILE
	int old_group = 0;
#endif

	if (mtwan_idx < 1 || mtwan_idx > MAX_MTWAN_PROFILE_NUM)
		return;
	if (!is_mtwan_group_in_profile(mtwan_idx, to_group))
		return;

#ifdef RTCONFIG_MULTIWAN_PROFILE
	old_group = mtwan_prof[mtwan_idx-1].group;
#endif
	mtwan_prof[mtwan_idx-1].group = to_group;
	snprintf(mtwan_prefix, sizeof(mtwan_prefix), "mtwan%d_", mtwan_idx);
	nvram_pf_set_int(mtwan_prefix, "group", to_group);

	/// Router HOST
	if (mtwan_idx == 1)
	{
		int primary_unit = mtwan_get_first_wan_unit_by_group(mtwan_idx, to_group, MTWAN_CHECK_CONN);

		if (primary_unit != WAN_UNIT_NONE)
		{
			// switch
			_mtwan_switch_primary_wan_line(primary_unit);
		}
		else
		{
			_dprintf("MTWAN(%d) No connected WAN in group %d\n", mtwan_idx, to_group);
		}

#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_PROFILE
		mtwan6_update_main_default_route();
#endif
#endif
	}

	prctl(PR_GET_NAME, prc);
	if (!strcmp(prc, "mtwanduck"))
	{
#ifdef RTCONFIG_MULTILAN_CFG
		handle_sdn_feature(ALL_SDN, SDN_FEATURE_WAN, 1);
#else
		update_resolvconf();
		// TODO: by device/interface policy
#endif
	}

#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTILAN_CFG
	int wan_unit = mtwan6_get_active_wan_unit(mtwan_idx);
	if (wan_unit != WAN_UNIT_NONE)
	{
		restart_mtwan_dnsmasq_ipv6(wan_unit);
		add_ip6_lanaddr();
	}
#endif
#ifdef RTCONFIG_MULTIWAN_PROFILE
	if (mtwan_idx > 1) {
		mtwan6_restart_6relayd(mtwan_idx
			, mtwan_get_first_wan_unit_by_group(mtwan_idx, old_group, MTWAN_IGNORE_CONN)
			, mtwan_get_first_wan_unit_by_group(mtwan_idx, to_group, MTWAN_CHECK_CONN));
	}
#endif
#endif

}

void mtwan_handle_wan_conn(int wan_unit, int connected, int link_up)
{
	int i, j;
	int mtwan_idx;
	int mtwan_mode;
	int wan_group, mtwan_group, to_group;
	int group_priority;

	for (i = MAX_MTWAN_PROFILE_NUM - 1; i >= 0; i--)
	{
		mtwan_idx = i + 1;

		if (mtwan_prof[i].enable == 0)
			continue;

		mtwan_print_profile(mtwan_idx);
		mtwan_mode = mtwan_prof[i].mode;
		if (mtwan_mode != MTWAN_MODE_ANY && mtwan_mode != MTWAN_MODE_ALL)
			continue;

		if (!is_mtwan_unit_in_profile(mtwan_idx, wan_unit))
			continue;

		wan_group = mtwan_get_wan_group(mtwan_idx, wan_unit);
		if (wan_group < 1)
			continue;

		mtwan_group = mtwan_prof[i].group;
		to_group = mtwan_group;

		if (!is_mtwan_lb_in_profile(mtwan_idx, wan_unit) && (!connected && wan_group != mtwan_group))
			continue;

		_dprintf("MTWAN(%d) current mtwan group: %d, wan group: %d\n", mtwan_idx, mtwan_group, wan_group);
		if (connected)
		{
			if (!_is_mtwan_group_connected(mtwan_idx, mtwan_group, wan_unit, connected)
			 && mtwan_group_compare(mtwan_idx, wan_group, mtwan_group)
			) {
				_dprintf("MTWAN(%d) current group %d disconnected. Fail-over to group %d\n", mtwan_idx, mtwan_group, wan_group);
				to_group = wan_group;
			}
			else if (mtwan_prof[i].fb)
			{
				// fail-back if wan_group is high priority
				group_priority = mtwan_group_compare(mtwan_idx, wan_group, mtwan_group);
				if (group_priority > 0)
				{
					if (_is_mtwan_group_connected(mtwan_idx, wan_group, wan_unit, connected))
					{
						_dprintf("MTWAN(%d) should fail-back to %d\n", mtwan_idx, wan_group);
						to_group = wan_group;
					}
					else
					{
						_dprintf("MTWAN(%d) stay in group %d\n", mtwan_idx, mtwan_group);
					}
				}
				else if (group_priority == 0)
				{
					_dprintf("MTWAN(%d) keep same group\n", mtwan_idx);
				}
				else
				{
					_dprintf("MTWAN(%d) ignore since group %d priority is lower or not in the list.\n", mtwan_idx, wan_group);
				}
			}
			else
			{
				_dprintf("MTWAN(%d) stay in group %d\n", mtwan_idx, mtwan_group);
			}
		}
		else
		{
			if (mtwan_mode == MTWAN_MODE_ANY)
			{
				to_group = mtwan_get_fo_group(mtwan_idx, 1);
				_dprintf("MTWAN(%d) next connected group %d\n", mtwan_idx, to_group);
			}
			else if (mtwan_mode == MTWAN_MODE_ALL)
			{
				if (_is_mtwan_group_connected(mtwan_idx, wan_group, wan_unit, connected))
				{
					_dprintf("MTWAN(%d) stay in group %d since other wan is still connected.\n", mtwan_idx, mtwan_group);
				}
				else
				{
					to_group = mtwan_get_fo_group(mtwan_idx, 1);
					_dprintf("MTWAN(%d) next connected group %d\n", mtwan_idx, to_group);
				}
			}
			else
				_dprintf("MTWAN(%d) mode undefined\n", mtwan_idx);
		}

		if (to_group != mtwan_group)
		{
			_dprintf("MTWAN(%d) switch from WAN group %d to %d\n", mtwan_idx, mtwan_group, to_group);
			mtwan_handle_group_change(mtwan_idx, to_group);
		}
		else if (mtwan_idx == 1)
		{
			if (is_mtwan_group_lb(mtwan_idx, mtwan_group))
			{
				for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
				{
					if (mtwan_prof[i].mt_group[j] != mtwan_group)
						continue;
					if (mtwan_prof[i].wan_units[j] == wan_unit)
					{
						if (connected)
						{
							_mtwan_set_wan_primary(mtwan_prof[i].wan_units[j]);
#ifdef RTCONFIG_IPV6
							restart_mtwan_dnsmasq_ipv6(mtwan_prof[i].wan_units[j]);
							add_ip6_lanaddr();
#endif
							break;
						}
					}
					else
					{
						char nvname[32] = {0};
						snprintf(nvname, sizeof(nvname), "wan%d_link_internet", mtwan_prof[i].wan_units[j]);
						if (nvram_get_int(nvname) == 2)
						{
							_mtwan_set_wan_primary(mtwan_prof[i].wan_units[j]);
#ifdef RTCONFIG_IPV6
							restart_mtwan_dnsmasq_ipv6(mtwan_prof[i].wan_units[j]);
							add_ip6_lanaddr();
#endif
							break;
						}
					}
				}
			}
			else if (connected && wan_group == mtwan_group && !is_mtwan_primary(wan_unit))
			{
				_mtwan_switch_primary_wan_line(wan_unit);
			}
		}
	}

	if (is_mtwan_lb(wan_unit))
	{
		mtwan_update_lb_route(wan_unit, connected);
		mtwan_update_lb_iptables(wan_unit, connected, MTWAN_HANDLE_V4);
		if (!connected)
			remove_ip_rules(mtwan_get_mark_rule_pref(wan_unit), MTWAN_HANDLE_V4);
#ifdef RTCONFIG_IPV6
		if (!link_up && ipv6x_enabled(wan_unit))
		{
			// flush before update lb route
			char table[8];
			mtwan_get_route_table_id(wan_unit, table, sizeof(table));
			eval("ip", "-6", "route", "flush", "table", table);
			eval("ip", "-6", "route", "del", "default", "dev", get_wan6_ifname(wan_unit));
			remove_ip_rules(mtwan_get_mark_rule_pref(wan_unit), MTWAN_HANDLE_V6);
		}
		mtwan6_update_lb_route(wan_unit, connected);
		mtwan_update_lb_iptables(wan_unit, connected, MTWAN_HANDLE_V6);
#endif
	}

	if (!connected)
	{
		remove_ip_rules(mtwan_get_route_rule_pref(wan_unit), MTWAN_HANDLE_V4);
		remove_ip_rules(mtwan_get_dns_rule_pref(wan_unit), MTWAN_HANDLE_V4);
		update_resolvconf();
#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_resolvconf();
#endif
#ifdef RTCONFIG_IPV6
		if (!link_up && ipv6x_enabled(wan_unit))
		{
			remove_ip_rules(mtwan_get_route_rule_pref(wan_unit), MTWAN_HANDLE_V6);
			remove_ip_rules(mtwan_get_dns_rule_pref(wan_unit), MTWAN_HANDLE_V6);
		}
#endif
	}

}

void mtwan_init_mtwan_group()
{
	int i, mtwan_idx, mtwan_group;
	int wan_group;
	char nvname[32] = {0};

	for (i = 0; i < MAX_MULTI_WAN_NUM; i++)
	{
		mtwan_idx = i + 1;
		mtwan_group = mtwan_prof[i].group;
		if (mtwan_prof[i].enable)
		{
			if (mtwan_prof[i].mode == MTWAN_MODE_TIME)
			{
				if (check_sched_v2_on_off(mtwan_prof[i].sched))
					wan_group = mtwan_get_first_group(mtwan_idx);
				else
					wan_group = mtwan_get_second_group(mtwan_idx);
			}
			else
			{
				wan_group = mtwan_get_first_group(mtwan_idx);
				if (!_is_mtwan_group_connected(mtwan_idx, wan_group, -1, 0))
				{
					wan_group = mtwan_get_fo_group(mtwan_idx, 1);
				}
			}

			if (wan_group != mtwan_group)
				mtwan_handle_group_change(mtwan_idx, wan_group);
		}
		else if (mtwan_group != 0)
		{//enable -> disable
			if (mtwan_idx == 1)
			{
				if (mtwan_get_first_wan_unit_by_group(mtwan_idx, mtwan_group, MTWAN_CHECK_CONN) != WAN_UNIT_FIRST)
				{
					_mtwan_switch_primary_wan_line(WAN_UNIT_FIRST);
#ifdef RTCONFIG_MULTILAN_CFG
					update_sdn_resolvconf();
#endif
				}
			}
			mtwan_prof[i].group = 0;
			snprintf(nvname, sizeof(nvname), "mtwan%d_group", mtwan_idx);
			nvram_set(nvname, "");
		}
	}
}

void mtwan_update_profile()
{
	int i;
	int mtwan_idx, mtwan_group;

	/// group member/ load-balance weight changed.
	mtwan_init_nvram();
	mtwanduck_update_profile();

	/// update mtwan group
	mtwan_init_mtwan_group();

	/// iptables
	mtwan_update_lb_iptables(-1, MTWAN_CONNECTED, MTWAN_HANDLE_V4);
#ifdef RTCONFIG_IPV6
	mtwan_update_lb_iptables(-1, MTWAN_CONNECTED, MTWAN_HANDLE_V6);
#endif

	/// routing
	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++)
	{
		mtwan_idx = i + 1;

		if (mtwan_idx == 1)	//default profile
		{
			mtwan_update_main_default_route(-1, MTWAN_LINK_UP);
#ifdef RTCONFIG_IPV6
			mtwan6_update_main_default_route();
#endif
		}

		if (mtwan_prof[i].enable == 0)
			continue;

		// by group
		for (mtwan_group = 1; mtwan_group <= MAX_MULTI_WAN_NUM; mtwan_group++)
		{
			if (is_mtwan_group_lb(mtwan_idx, mtwan_group))
			{
				mtwan_update_profile_lb_route(mtwan_idx, mtwan_group, -1, MTWAN_LINK_UP);
#ifdef RTCONFIG_IPV6
				mtwan6_update_profile_lb_route(mtwan_idx, mtwan_group, -1, MTWAN_LINK_UP);
#endif
			}
		}
	}

#ifdef RTCONFIG_MULTILAN_CFG
	handle_sdn_feature(ALL_SDN, SDN_FEATURE_WAN, 1);
#else
	update_resolvconf();
	// TODO: by device/interface policy
#endif

}

void mtwan_print_profile(int mtwan_idx)
{
	int i, j;
	int start = 0, end = MAX_MTWAN_PROFILE_NUM;

	if (mtwan_idx)
	{
		start = mtwan_idx - 1;
		end = mtwan_idx;
	}

	for (i = start; i < end; i++)
	{
		_dprintf("MTWAN(%d)\n", i + 1);
		_dprintf("--------------------\n");

		_dprintf("enable: %d\n", mtwan_prof[i].enable);

		_dprintf("mt_group:");
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
			_dprintf(" %d", mtwan_prof[i].mt_group[j]);
		_dprintf("\n");

		_dprintf("mt_weight:");
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
			_dprintf(" %d", mtwan_prof[i].mt_weight[j]);
		_dprintf("\n");

		_dprintf("mode: %d\n", mtwan_prof[i].mode);

		_dprintf("fb: %d\n", mtwan_prof[i].fb);

		_dprintf("order:");
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
			if (mtwan_prof[i].order[j])
				_dprintf(" %d", mtwan_prof[i].order[j]);
		_dprintf("\n");

		_dprintf("sched: %s\n", mtwan_prof[i].sched);

		_dprintf("group: %d\n", mtwan_prof[i].group);

		_dprintf("prob:");
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
			_dprintf(" %f", mtwan_prof[i].prob[j]);
		_dprintf("\n");

		_dprintf("wan_units:");
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
			_dprintf(" %d", mtwan_prof[i].wan_units[j]);
		_dprintf("\n");

		_dprintf("lb_wan_units:");
		for (j = 0; j < MAX_MULTI_WAN_NUM; j++)
			_dprintf(" %d", mtwan_prof[i].lb_wan_units[j]);
		_dprintf("\n\n");
	}
}

void mtwan_init_profile()
{
	char mtwan_prefix[16] = {0}, wan_prefix[16] = {0};
	int i, j;
	int real_unit;
	int total[MAX_MULTI_WAN_NUM+1] = {0};
	char buf[256] = {0}, word[16] = {0}, *next = NULL;

	for (i = 0; i < MAX_MTWAN_PROFILE_NUM; i++)
	{
		snprintf(mtwan_prefix, sizeof(mtwan_prefix), "mtwan%d_", i+1);
		mtwan_prof[i].enable = nvram_pf_get_int(mtwan_prefix, "enable");
		if (mtwan_prof[i].enable == 0)
			continue;
		strlcpy(buf, nvram_pf_safe_get(mtwan_prefix, "mt_group"), sizeof(buf));
		j = 0;
		foreach(word, buf, next)
		{
			mtwan_prof[i].mt_group[j] = atoi(word);
			j++;
		}
		for (; j < MAX_MULTI_WAN_NUM; j++)
		{
			mtwan_prof[i].mt_group[j] = 0;
		}
		strlcpy(buf, nvram_pf_safe_get(mtwan_prefix, "mt_weight"), sizeof(buf));
		j = 0;
		foreach(word, buf, next)
		{
			mtwan_prof[i].mt_weight[j] = atoi(word);
			j++;
		}
		for (; j < MAX_MULTI_WAN_NUM; j++)
		{
			mtwan_prof[i].mt_weight[j] = 0;
		}
		memset(buf, 0, sizeof(buf));
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] < 1 || mtwan_prof[i].mt_group[j] > MAX_MULTI_WAN_NUM)
			{
				mtwan_prof[i].wan_units[j] = WAN_UNIT_NONE;
				continue;
			}
			if (mtwan_prof[i].mt_weight[j] < 1 || mtwan_prof[i].mt_weight[j] > MAX_MULTI_WAN_NUM)
				continue;
			real_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
			if (real_unit < 0)
				continue;
			// total weight
			if (nvram_pf_get_int(wan_prefix, "state_t") == WAN_STATE_CONNECTED)
				total[mtwan_prof[i].mt_group[j]] += mtwan_prof[i].mt_weight[j];
			// wan_units
			mtwan_prof[i].wan_units[j] = real_unit;
			if (buf[0])
				strlcat(buf, " ", sizeof(buf));
			snprintf(word, sizeof(word), "%d", real_unit);
			strlcat(buf, word, sizeof(buf));
		}
		nvram_pf_set(mtwan_prefix, "wan_units", buf);
		// prob
		memset(buf, 0, sizeof(buf));
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			if (mtwan_prof[i].mt_group[j] < 1 || mtwan_prof[i].mt_group[j] > MAX_MULTI_WAN_NUM)
				continue;
			if (mtwan_prof[i].mt_weight[j] < 1 || mtwan_prof[i].mt_weight[j] > MAX_MULTI_WAN_NUM)
				continue;
			real_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
			if (real_unit < 0)
				continue;
			if (nvram_pf_get_int(wan_prefix, "state_t") == WAN_STATE_CONNECTED)
			{
				mtwan_prof[i].prob[j] = (double)mtwan_prof[i].mt_weight[j] / total[mtwan_prof[i].mt_group[j]];
			}
			else
			{
				mtwan_prof[i].prob[j] = 0.0;
			}
		}
		for(j = 0; j < MAX_MULTI_WAN_NUM; j++)
		{
			snprintf(word, sizeof(word), "%f", mtwan_prof[i].prob[j]);
			if (buf[0])
				strlcat(buf, " ", sizeof(buf));
			strlcat(buf, word, sizeof(buf));
			if (0.0 < mtwan_prof[i].prob[j] && mtwan_prof[i].prob[j] < 1.0)
			{
				mtwan_prof[i].lb_wan_units[j] = mtwan_get_mapped_unit(j + MULTI_WAN_START_IDX);
			}
			else
			{
				mtwan_prof[i].lb_wan_units[j] = WAN_UNIT_NONE;
			}
		}
		nvram_pf_set(mtwan_prefix, "mt_prob", buf);
		mtwan_prof[i].mode = nvram_pf_get_int(mtwan_prefix, "mode");
		mtwan_prof[i].fb = nvram_pf_get_int(mtwan_prefix, "fb");
		strlcpy(buf, nvram_pf_safe_get(mtwan_prefix, "order"), sizeof(buf));
		j = 0;
		foreach(word, buf, next)
		{
			mtwan_prof[i].order[j] = atoi(word);
			j++;
		}
		for (; j < MAX_MULTI_WAN_NUM; j++)
		{
			mtwan_prof[i].order[j] = 0;
		}
		strlcpy(mtwan_prof[i].sched, nvram_pf_safe_get(mtwan_prefix, "sched"), sizeof(mtwan_prof[i].sched));
		mtwan_prof[i].group = nvram_pf_get_int(mtwan_prefix, "group");
	}
}

void mtwan_check_and_init_profile()
{
	char prc[16] = {0};

	prctl(PR_GET_NAME, prc);
	if (strcmp(prc, "mtwanduck"))
		mtwan_init_profile();
}
#endif	//RTCONFIG_MULTIWAN_PROFILE

static int _set_route_table(const int unit, const int real_unit, const char *prefix)
{
	char table[8];
	char tmp[256], *gateway, *xgateway, *ifname, *gw_ifname, *dns, *xdns, *ip , *xip, *netmask, *xnetmask;
	char word[64], *next;
	char *routes, *tmp_routes, *tmp_ipaddr, *tmp_gateway, *tmp_network;
	int metric = 0;
	char metric_str[16] = {0};
#ifndef RTCONFIG_MULTILAN_CFG
	char lan[32];
#endif

	if (!prefix)
		return -1;

#ifdef RTCONFIG_MULTISERVICE
	mtwan_get_route_table_id(get_ms_base_unit(unit), table, sizeof(table));
#else
	mtwan_get_route_table_id(unit, table, sizeof(table));
#endif

	ifname = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));
	gw_ifname = nvram_safe_get(strlcat_r(prefix, "gw_ifname", tmp, sizeof(tmp)));

	//TODO: Need to check wan interface state to do clean routing table and ip rules and decide if need to set routing table

	if (real_unit == -1)	//dhcp of dhcp + ppp case
	{
		xgateway = nvram_safe_get(strlcat_r(prefix, "xgateway", tmp, sizeof(tmp)));
		xip = nvram_safe_get(strlcat_r(prefix, "xipaddr", tmp, sizeof(tmp)));
		xnetmask = nvram_safe_get(strlcat_r(prefix, "xnetmask", tmp, sizeof(tmp)));
		xdns = nvram_safe_get(strlcat_r(prefix, "xdns", tmp, sizeof(tmp)));
		metric = 2;
#ifdef RTCONFIG_MULTISERVICE_WAN
		metric += get_ms_idx_by_wan_unit(unit);
#endif
		snprintf(metric_str, sizeof(metric_str), "%d", metric);

		// add xgateway
		if(xgateway[0] != '\0' && ifname[0] != '\0' && strcmp(xgateway, "0.0.0.0"))
		{
#ifdef RTCONFIG_MULTISERVICE_WAN
			if (!is_ms_wan_unit(unit))	//skip default route
#endif
			eval("ip", "route", "add", "default", "via", xgateway, "dev", ifname, "metric", "1", "table", table);
			eval("ip", "route", "add", xgateway, "dev", ifname, "proto", "static", "scope", "link", "table", table);
			if(is_valid_ip4(xip) && strcmp(xip, "0.0.0.0"))
			{
				if(!get_network_addr_by_ip_prefix(xip, xnetmask, tmp, sizeof(tmp)))
				{

					eval("ip", "route", "add", tmp, "dev", ifname, "proto", "static", "scope", "link", "src", xip, "table", table);
				}
			}
		}
		// add xdns
		if(xdns[0] != '\0' && strcmp(xdns, "0.0.0.0"))
		{
			foreach(word, xdns, next)
			{
				if (inet_addr(word) != inet_addr(xgateway))
					eval("ip", "route", "add",  word, "via", xgateway, "dev", ifname, "metric", metric_str, "table", table);
			}
		}

		//add dhcp xroutes, xroutes_ms, xroutes_rfc
		/// classful static routes
		routes = strdup(nvram_safe_get(strlcat_r(prefix, "xroutes", tmp, sizeof(tmp))));
		for (tmp_routes = routes; tmp_routes && *tmp_routes; ) {
			tmp_ipaddr  = strsep(&tmp_routes, "/");
			tmp_gateway = strsep(&tmp_routes, " ");
			if (tmp_gateway && inet_addr(tmp_ipaddr) != INADDR_ANY)
				eval("ip", "route", "add",  tmp_ipaddr, "via", tmp_gateway, "dev", ifname, "metric", metric_str, "table", table);
		}
		free(routes);
		/// ms claseless static routes
		routes = strdup(nvram_safe_get(strlcat_r(prefix, "xroutes_ms", tmp, sizeof(tmp))));
		for (tmp_routes = routes; tmp_routes && *tmp_routes; ) {
			tmp_network  = strsep(&tmp_routes, " ");
			tmp_gateway = strsep(&tmp_routes, " ");
			if (tmp_gateway && tmp_network)
				eval("ip", "route", "add",  tmp_network, "via", tmp_gateway, "dev", ifname, "metric", metric_str, "table", table);
		}
		free(routes);
		/// rfc3442 classless static routes
		routes = strdup(nvram_safe_get(strlcat_r(prefix, "xroutes_rfc", tmp, sizeof(tmp))));
		for (tmp_routes = routes; tmp_routes && *tmp_routes; ) {
			tmp_network  = strsep(&tmp_routes, " ");
			tmp_gateway = strsep(&tmp_routes, " ");
			if (tmp_gateway && tmp_network)
				eval("ip", "route", "add",  tmp_network, "via", tmp_gateway, "dev", ifname, "metric", metric_str, "table", table);
		}
		free(routes);
	}
	else
	{
#ifdef RTCONFIG_SOFTWIRE46
		switch(get_wan_proto((char*)prefix)) {
		case WAN_V6PLUS:
		case WAN_OCNVC:
			if (nvram_pf_get_int(prefix, "s46_hgw_case") == S46_CASE_MAP_CE_ON) {
				eval("ip", "route", "add", "default", "via", "0.0.0.0", "dev", gw_ifname, "table", table);
				return 0;
			}
			else if (nvram_pf_get_int(prefix, "s46_hgw_case") == S46_CASE_MAP_HGW_OFF) {
				eval("ip", "route", "replace", "default", "via", "0.0.0.0", "dev", gw_ifname, "table", table);
				return 0;
			}
		}
#endif

		gateway = nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp)));
		ip = nvram_safe_get(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)));
		netmask = nvram_safe_get(strlcat_r(prefix, "netmask", tmp, sizeof(tmp)));
		dns = nvram_safe_get(strlcat_r(prefix, "dns", tmp, sizeof(tmp)));
		metric = 1;
#ifdef RTCONFIG_MULTISERVICE_WAN
		metric += get_ms_idx_by_wan_unit(unit);
#endif
		snprintf(metric_str, sizeof(metric_str), "%d", metric);

		//add gateway
		if(gateway[0] != '\0' && strcmp(gateway, "0.0.0.0")  && gw_ifname[0] != '\0')
		{
#ifdef RTCONFIG_MULTISERVICE_WAN
			if (!is_ms_wan_unit(unit))	//skip default route
#endif
			eval("ip", "route", "add", "default", "via", gateway, "dev", gw_ifname, "table", table);
			eval("ip", "route", "add", gateway, "dev", gw_ifname, "proto", "static", "scope", "link", "table", table);
			if(is_valid_ip4(ip) && strcmp(ip, "0.0.0.0"))
			{
				if(!get_network_addr_by_ip_prefix(ip, netmask, tmp, sizeof(tmp)))
				{
					eval("ip", "route", "add", tmp, "dev", gw_ifname, "proto", "static", "scope", "link", "src", ip, "table", table);
				}
			}
		}

		//add dns
		if(dns[0] != '\0' && strcmp(dns, "0.0.0.0"))
		{
			foreach(word, dns, next)
			{
				if (inet_addr(word) != inet_addr(gateway))
					eval("ip", "route", "add",  word, "via", gateway, "dev", gw_ifname, "metric", metric_str, "table", table);
			}
		}

		//add dhcp routes, routes_ms, routes_rfc
		/// classful static routes
		routes = strdup(nvram_safe_get(strlcat_r(prefix, "routes", tmp, sizeof(tmp))));
		for (tmp_routes = routes; tmp_routes && *tmp_routes; ) {
			tmp_ipaddr  = strsep(&tmp_routes, "/");
			tmp_gateway = strsep(&tmp_routes, " ");
			if (tmp_gateway && inet_addr(tmp_ipaddr) != INADDR_ANY)
				eval("ip", "route", "add",  tmp_ipaddr, "via", tmp_gateway, "dev", ifname, "metric", metric_str, "table", table);
		}
		free(routes);
		/// ms claseless static routes
		routes = strdup(nvram_safe_get(strlcat_r(prefix, "routes_ms", tmp, sizeof(tmp))));
		for (tmp_routes = routes; tmp_routes && *tmp_routes; ) {
			tmp_network  = strsep(&tmp_routes, " ");
			tmp_gateway = strsep(&tmp_routes, " ");
			if (tmp_gateway && tmp_network)
				eval("ip", "route", "add",  tmp_network, "via", tmp_gateway, "dev", ifname, "metric", metric_str, "table", table);
		}
		free(routes);
		/// rfc3442 classless static routes
		routes = strdup(nvram_safe_get(strlcat_r(prefix, "routes_rfc", tmp, sizeof(tmp))));
		for (tmp_routes = routes; tmp_routes && *tmp_routes; ) {
			tmp_network  = strsep(&tmp_routes, " ");
			tmp_gateway = strsep(&tmp_routes, " ");
			if (tmp_gateway && tmp_network)
				eval("ip", "route", "add",  tmp_network, "via", tmp_gateway, "dev", ifname, "metric", metric_str, "table", table);
		}
		free(routes);
	}

	//add lan related
#ifndef RTCONFIG_MULTILAN_CFG
	get_network_addr_by_ip_prefix(nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), lan, sizeof(lan));
	eval("ip", "route", "add", lan, "dev", nvram_safe_get("lan_ifname"), "proto", "static", "scope", "link", "src", nvram_safe_get("lan_ipaddr"), "table", table);
#endif

	//TODO: add lan_route  (LAN WEB page)

	//add upnp(239.0.0.0)? need to consider the 
	return 0;
}

static int _set_route_rule(const int unit, const int real_unit, const char *prefix)
{
	char *dns, table[8], pref[16], tmp[128];
	char word[64], *next;
	int pref_num;

	if(!prefix)
		return -1;
	if(unit < 0)
		return -1;

#ifdef RTCONFIG_MULTISERVICE
	mtwan_get_route_table_id(get_ms_base_unit(unit), table, sizeof(table));
#else
	mtwan_get_route_table_id(unit, table, sizeof(table));
#endif

	/// rule for DNS
	pref_num = mtwan_get_dns_rule_pref(unit);
	snprintf(pref, sizeof(pref), "%d", pref_num);
	remove_ip_rules(pref_num, MTWAN_HANDLE_V4);

	// if(unit == real_unit ||	//by pass dual wan. It would be handled in the original wan_up()
		// (!_is_dualwan_enable() && real_unit == WAN_UNIT_FIRST))
	{		
		//set dns
		dns = nvram_safe_get(strlcat_r(prefix, "dns", tmp, sizeof(tmp)));
		foreach(word, dns, next)
		{
			eval("ip", "rule", "add", "iif", "lo", "to", word, "table", table, "pref",pref );
		}

		//set xdns
		dns = nvram_safe_get(strlcat_r(prefix, "xdns", tmp, sizeof(tmp)));
		foreach(word, dns, next)
		{
			eval("ip", "rule", "add", "iif", "lo", "to", word, "table", table, "pref",pref );
		}
	}

	//set default wan
#ifdef RTCONFIG_MULTILAN_CFG
	/// handle by SDN_FEATURE_WAN
	update_sdn_by_wan(unit);
#else
	if(mtwan_get_default_wan() == unit)
	{
		/// rule for default wan
		pref_num = IP_RULE_PREF_MTWAN_ROUTE;
		snprintf(pref, sizeof(pref), "%d", pref_num);
		remove_ip_rules(pref_num, MTWAN_HANDLE_V4);
		eval("ip", "rule", "add", "iif", nvram_safe_get("lan_ifname"), "table", table, "perf", pref);
	}
#endif

#ifdef RTCONFIG_MULTIWAN_PROFILE
	/// rule for MARK
	if (IS_MTWAN_UNIT(unit))
	{
		pref_num = mtwan_get_mark_rule_pref(unit);
		remove_ip_rules(pref_num, MTWAN_HANDLE_V4);
		doSystem("ip rule add fwmark 0x%08x/0x%08x pref %d lookup %s"
			, IPTABLES_MARK_MTWAN_SET(unit - MULTI_WAN_START_IDX, 0), IPTABLES_MARK_MTWAN_MASK
			, pref_num, table);
	}
#endif

	return 0;
}
/*
static int _update_iptables(const int unit, const int up)
{
	char prefix[] = "wanXXXXXX_", *gw_ifname, *ipaddr, tmp[512];

	if(mtwan_get_real_wan(unit, prefix, sizeof(prefix)) != -1)
	{
		gw_ifname  = nvram_safe_get(strlcat_r(prefix, "gw_ifname", tmp, sizeof(tmp)));
		ipaddr = nvram_safe_get(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)));

		if(gw_ifname[0] != '\0')
		{
			_remove_iptables(gw_ifname);
			if(up == 1 && ipaddr[0] != '\0')
			{
				snprintf(tmp, sizeof(tmp), "iptables -t nat -A POSTROUTING ! -s %s/32 -o %s -j MASQUERADE", ipaddr, gw_ifname);
				system(tmp);
			}
			return 0;
		}
	}
	return -1;
}
*/
//only handle the wan id is in the range of multi-wan
static int _set_mtwan_route(const int unit, const char *ifname, const int up)
{
	char prefix[] = "wanXXXXXXXXXX_";
	int i, real_unit;
	int wan_unit = (unit < 0) ? wanx_ifunit((char*)ifname) : unit;
#ifdef RTCONFIG_MULTISERVICE_WAN
	int base_unit = get_ms_base_unit(wan_unit);
#endif

	if (wan_unit < 0)
		return -1;

	if (unit == MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM) // re-create all route tables expect the real unit is configured by dual wan.
	{
		for (i = 0; i < MAX_MULTI_WAN_NUM; ++i)
		{
			real_unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));

			if (real_unit != -1 && real_unit >= MULTI_WAN_START_IDX)
			{
				// _remove_mtwan_routing(unit);
				//remove_ip_rules(mtwan_get_route_rule_pref(i + MULTI_WAN_START_IDX), MTWAN_HANDLE_V4);
				if(up)
				{
					_set_route_table(i + MULTI_WAN_START_IDX, real_unit, prefix);
					_set_route_rule(i + MULTI_WAN_START_IDX, real_unit, prefix);
				}
			}
		}
	}
	else
	{
		if (IS_MTWAN_UNIT(unit))
		{
			real_unit = mtwan_get_real_wan(unit, prefix, sizeof(prefix));
			if(real_unit == unit)
			{
				// _remove_mtwan_routing(unit);
				// remove_ip_rules(mtwan_get_route_rule_pref(unit), MTWAN_HANDLE_V4);
				if(up)
				{
					_set_route_table(unit, real_unit, prefix);
					_set_route_rule(unit, real_unit, prefix);
				}
			}
		}
		else if (IS_MTWAN_UNIT(wan_unit))
		{	//dhcp of dhcp + ppp case: unit = -1, wan_unit is mtwan unit.
			if(up)
			{
				snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
				_set_route_table(wan_unit, unit, prefix);
				_set_route_rule(wan_unit, unit, prefix);
			}
		}
#ifdef RTCONFIG_MULTISERVICE_WAN
		else if (is_ms_wan_unit(unit) && IS_MTWAN_UNIT(base_unit))
		{
			if(up)
			{
				snprintf(prefix, sizeof(prefix), "wan%d_", unit);
				_set_route_table(unit, unit, prefix);
				_set_route_rule(unit, unit, prefix);
			}
		}
		else if (is_ms_wan_unit(wan_unit))
		{	//dhcp of dhcp + ppp case: unit = -1, wan_unit is ms unit, base_unit is mtwan unit.
			if(up)
			{
				snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
				_set_route_table(wan_unit, unit, prefix);
				_set_route_rule(wan_unit, unit, prefix);
			}
		}
#endif
	}

	eval("ip", "route", "flush", "cache");

	return 0;
}
/*
static int _update_resolv_conf(const int unit, const int up)
{
	char path[256], prefix[] = "wanXXXXX_", tmp[256], *dns;
	char word[64], *next;
	FILE *fp;
	int i;

	if(is_mtwan_unit(unit))
	{
		snprintf(path, sizeof(path), wanX_resolv_path, unit);
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		//update it if wan is up
		if(up)
		{
			fp = fopen(path, "w");
			if(fp)
			{
				if(nvram_match(strlcat_r(prefix, "dnsenable_x", tmp, sizeof(tmp)), "1"))	//isp dns
				{
					dns = nvram_safe_get(strlcat_r(prefix, "dns", tmp, sizeof(tmp)));
					foreach (word, dns, next)
					{
						fprintf(fp, "server=%s\n", word);
					}
				}
				else	//user defined dns
				{
					for(i = 1; i <= 2; ++i)
					{
						snprintf(word, sizeof(word), "dns%d_x", i);
						dns = nvram_safe_get(strlcat_r(prefix, word, tmp, sizeof(tmp)));
						if(dns && dns[0] != '\0')
						{
							fprintf(fp, "server=%s\n", dns);
						}
					}
				}
				fclose(fp);
			}
		}
		else
		{
			unlink(path);
		}
	}
	return 0;
}
*/

#ifdef RTCONFIG_SOFTWIRE46
static void _mtwan_append_s46_resolvconf_ipv4(int wan_unit)
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
		strlcpy(wan_dns_buf, nvram_pf_safe_get(wan_prefix, "dns"), sizeof(wan_dns_buf));
		if (wan_dns_buf[0]) {
			foreach (wan_dns, wan_dns_buf, next_dns) {
				if (doSystem("cat /tmp/resolv.conf | grep %s", wan_dns)) {
					fp = fopen("/tmp/resolv.conf", "a");
					if(fp) {
						fprintf(fp, "nameserver %s\n", wan_dns);
						fclose(fp);
					}
				}
			}
		}
		else {
			strlcpy(wan_dns_buf, nvram_pf_safe_get(wan_prefix, "xdns"), sizeof(wan_dns_buf));
			if (wan_dns_buf[0]) {
				foreach (wan_dns, wan_dns_buf, next_dns) {
					if (doSystem("cat /tmp/resolv.conf | grep %s", wan_dns)) {
						fp = fopen("/tmp/resolv.conf", "a");
						if(fp) {
							fprintf(fp, "nameserver %s\n", wan_dns);
							fclose(fp);
						}
					}
				}
			}
		}
		break;
	}
}

static void _mtwan_append_s46_resolvconf_ipv6(int wan_unit)
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

void mtwan_append_s46_resolvconf()
{
	int unit;
	int real_unit;

	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++)
	{
		real_unit = mtwan_get_mapped_unit(unit);
		_mtwan_append_s46_resolvconf_ipv4(real_unit);
		_mtwan_append_s46_resolvconf_ipv6(real_unit);
	}
}
#endif

static int _update_wanX_nvram(const int unit, const char *ifname, const int up, const char *prc)
{
	char prefix[] = "wanXXXX_", tmp[256], *gateway, gw_mac[ETHER_ADDR_LEN*3];
	int real_unit, wan_proto;

	real_unit = mtwan_get_real_wan(unit, prefix, sizeof(prefix));
	wan_proto = get_wan_proto(prefix);

	if(up)
	{
		gateway = nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp)));

		//update gw_ifname
		if(ifname
#ifdef RTCONFIG_SOFTWIRE46
		 && !((wan_proto == WAN_V6PLUS || wan_proto == WAN_OCNVC)
			&& nvram_pf_get_int(prefix, "s46_hgw_case") == S46_CASE_MAP_HGW_OFF
			&& !strcmp(prc, "udhcpc_wan"))
#endif
		)
			nvram_set(strlcat_r(prefix, "gw_ifname", tmp, sizeof(tmp)), ifname);

		//update xgateway
		switch(wan_proto)
		{
			case WAN_DHCP:
			case WAN_STATIC:
#ifdef RTCONFIG_SOFTWIRE46
			case WAN_LW4O6:
			case WAN_MAPE:
			case WAN_V6PLUS:
			case WAN_OCNVC:
#endif
				if(gateway[0] != '\0')
				{
					nvram_set(strlcat_r(prefix, "xgateway", tmp, sizeof(tmp)), gateway);
				}
			break;
		}

		//update gw_mac
		if(gateway[0] != '\0' && strcmp(gateway, "0.0.0.0") && !get_gw_mac(gateway, ifname, gw_mac, sizeof(gw_mac)))
		{
			nvram_set(strcat_r(prefix, "gw_mac", tmp), gw_mac);
			_dprintf("[%s] set wan%d_gw_mac=%s\n", __FUNCTION__, real_unit, gw_mac);
		}
		else
		{
			nvram_unset(strcat_r(prefix, "gw_mac", tmp));
			_dprintf("%s: no wan%d_mac, remove\n", __func__, real_unit);
		}
	}
	return 0;
}

int is_mtwan_ifname(const char *ifname)
{
	int unit;
	char prefix[16] = {0};
	char tmp[64] = {0};

	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++)
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "pppoe_ifname", tmp), ifname))
			return 1;
		if (nvram_match(strcat_r(prefix, "ifname", tmp), ifname))
			return 1;
	}
#ifdef RTCONFIG_MULTISERVICE_WAN
	for (unit = WAN_UNIT_MTWAN0_MS_START; unit < WAN_UNIT_MTWAN_MS_MAX; unit++)
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "pppoe_ifname", tmp), ifname))
			return 1;
		if (nvram_match(strcat_r(prefix, "ifname", tmp), ifname))
			return 1;
	}
#endif
	return 0;
}

int is_mtwan_unit(const int unit)
{
	return IS_MTWAN_UNIT(unit);
}

int mtwan_get_default_wan()
{
#ifdef RTCONFIG_MULTIWAN_PROFILE
	return (nvram_get_int("wans_mt_default_wan") ?: -1);
#else
	int i;
	char  name[64], *dualwan_idx;

	for(i = MULTI_WAN_START_IDX; i < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++i)
	{
		snprintf(name, sizeof(name), "wan%d_dualwan_idx", i);
		dualwan_idx = nvram_safe_get(name);
		if(dualwan_idx && dualwan_idx[0] != '\0' && atoi(dualwan_idx) == 0)
			return i;
	}
			return -1;
#endif
}

int mtwan_get_route_table_id(const int unit, char *table, const size_t table_len)
{
	char prefix[] =  "wanXXXX_", tmp[256];
	int dualwan_idx, real_unit, dualwan_enable;
	int table_id = -1;	// -1 means "main" table
	
	if(!table)
		return -1;

	real_unit = mtwan_get_real_wan(unit, prefix, sizeof(prefix));
	dualwan_enable = _is_dualwan_enable();

	if(IS_MTWAN_UNIT(real_unit))
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", unit); 
		dualwan_idx = nvram_get_int(strlcat_r(prefix, "dualwan_idx", tmp, sizeof(tmp)));
		if(dualwan_idx == WAN_UNIT_NONE)
		{
			table_id = IP_ROUTE_TABLE_ID_MTWAN_BASE + unit;
		}
		else
		{
			if(dualwan_enable)
				table_id = IP_ROUTE_TABLE_ID_DUALWAN_BASE + dualwan_idx * 100;
		}
	}
	else if(dualwan_enable && nvram_match("wans_mode", "lb") && (unit == WAN_UNIT_FIRST || unit == WAN_UNIT_SECOND))
	{
		table_id = IP_ROUTE_TABLE_ID_DUALWAN_BASE + unit * 100;
	}
#ifdef RTCONFIG_MULTISERVICE_WAN
	else if(is_ms_wan_unit(unit))
	{
		int base_unit = get_ms_base_unit(unit);
#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (base_unit < MULTI_WAN_START_IDX)
			table_id = IP_ROUTE_TABLE_ID_MTWAN_BASE + mtwan_get_unit_by_dualwan_unit(base_unit);
		else
#endif
		table_id = IP_ROUTE_TABLE_ID_MTWAN_BASE + base_unit;

	}
#endif
#ifdef RTCONFIG_MULTIWAN_PROFILE
	else if (real_unit < MULTI_WAN_START_IDX)
	{
		table_id = IP_ROUTE_TABLE_ID_MTWAN_BASE + mtwan_get_unit_by_dualwan_unit(real_unit);
	}
	else
	{
		table_id = IP_ROUTE_TABLE_ID_MTWAN_BASE + unit;
	}
#endif

	if(table_id == -1)
		strlcpy(table, "main", table_len);
	else
		snprintf(table, table_len, "%d", table_id);
	return 0;
}

int mtwan_get_route_rule_pref(const int unit)
{
	int mtwan_unit = unit;

	if(unit < MULTI_WAN_START_IDX)
		mtwan_unit = mtwan_get_unit_by_dualwan_unit(unit);

	return IP_RULE_PREF_MTWAN_ROUTE + mtwan_unit;
}

int mtwan_get_dns_rule_pref(int unit)
{
	int mtwan_unit = unit;

	if(unit < MULTI_WAN_START_IDX)
		mtwan_unit = mtwan_get_unit_by_dualwan_unit(unit);

	return IP_RULE_PREF_MTWAN_DNS_ROUTE + mtwan_unit;
}

int mtwan_handle_ip_rule(const int unit)
{
	int mtwan_unit = unit;
	char prefix[32];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if(unit < MULTI_WAN_START_IDX)	
	{
		mtwan_unit = mtwan_get_unit_by_dualwan_unit(unit);
	}

	return _set_route_rule(mtwan_unit, unit, prefix);
}

int mtwan_handle_ip_route(int unit, int xcase)
{
	int mtwan_unit = unit;
	char prefix[32];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if (unit < MULTI_WAN_START_IDX)
	{
		mtwan_unit = mtwan_get_unit_by_dualwan_unit(unit);
	}

	if (xcase)
		_set_route_table(unit, -1, prefix);

	return _set_route_table(mtwan_unit, unit, prefix);
}

int mtwan_ifunit(const char *wan_ifname)
{
	char prefix[] = "wanXXXX_", tmp[256];
	int i, unit;

	if (!wan_ifname)
		return -1;

	for (i = 0; i < MAX_MULTI_WAN_NUM; ++i)
	{
		unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));

		if (unit != -1)
		{
			/// check pppoe ifname
			if (nvram_match(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)), wan_ifname))
			{
				return unit;
			}

			// check other protocol
			switch (get_wan_proto(prefix))
			{
			case WAN_DHCP:
			case WAN_STATIC:
#ifdef RTCONFIG_SOFTWIRE46
			case WAN_MAPE:
			case WAN_V6PLUS:
			case WAN_OCNVC:
			case WAN_DSLITE:
#endif
				if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname))
				{
					return unit;
				}
				break;
			}
		}
	}
	return -1;
}

int mtwan_start_multi_wan()
{
	char prefix[] = "wanXXXX_", tmp[256];
	int i, unit;
	for (i = 0; i < MAX_MULTI_WAN_NUM; ++i)
	{
		unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));
		if (unit != -1)
		{
			// check enable and ifname
			if (nvram_match(strlcat_r(prefix, "enable", tmp, sizeof(tmp)), "1") && !nvram_match(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ""))
			{
				_dprintf("[%s] Start wan%d\n", __FUNCTION__, unit);
				start_wan_if(unit);
			}
		}
		else
		{
			_dprintf("[%s] invalid wan_idx\n", __FUNCTION__);
		}
	}
	return 0;
}

int mtwan_stop_multi_wan6()
{
	char prefix[] = "wanXXXX_";
	char ifname[IFNAMSIZ] = {0};
	int i, unit;
	for (i = 0; i < MAX_MULTI_WAN_NUM; ++i)
	{
		unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));
		if (unit != -1)
		{
			_dprintf("[%s] Stop wan%d\n", __FUNCTION__, unit);
			mtwan_get_ifname(unit, ifname, sizeof(ifname));
			wan6_down(ifname);
		}
		else
		{
			_dprintf("[%s] invalid wan_idx\n", __FUNCTION__);
		}
	}
	return 0;
}

int mtwan_get_ifname(const int unit, char *ifname, const size_t len)
{
	char prefix[] = "wanXXXXXX_", *gw_ifname, tmp[64];

	if(!ifname || unit < 0)
		return -1;

	if(mtwan_get_real_wan(unit, prefix, sizeof(prefix)) != -1)
	{
		gw_ifname  = nvram_safe_get(strlcat_r(prefix, "gw_ifname", tmp, sizeof(tmp)));
		if(gw_ifname[0] != '\0')
		{
			strlcpy(ifname, gw_ifname, len);
			return 0;
		}
	}
	return -1;
}

#ifdef RTCONFIG_IPV6
static int _mtwan_wan6_up(int unit, const char *ifname)
{
	int wan_unit = unit;
	char wan_prefix[16] = {0};
	int wan_proto;

	if (unit < 0) {
		char wan_ifname[IFNAMSIZ];
		strlcpy(wan_ifname, ifname, sizeof(wan_ifname));
		if ((wan_unit = wanx_ifunit(wan_ifname)) < 0)
			return -1;
	}

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
	wan_proto = get_wan_proto(wan_prefix);

	switch (get_ipv6_service_by_unit(wan_unit)) {
		case IPV6_NATIVE_DHCP:
		case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
		case IPV6_PASSTHROUGH:
#endif
			if ((wan_proto == WAN_PPPOE || wan_proto == WAN_PPTP || wan_proto == WAN_L2TP)
			 && nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", wan_unit), "ppp"))
				break;
#ifdef RTCONFIG_SOFTWIRE46
			if (wan_proto == WAN_LW4O6 || wan_proto == WAN_MAPE
			 || wan_proto == WAN_V6PLUS || wan_proto == WAN_OCNVC)
				break;
#endif
			/* fall through */
		default:
			wan6_up(get_wan6_ifname(wan_unit));
			break;
	}

	return 0;
}
#endif

int mtwan_handle_if_updown(const int unit, const char *ifname, const int up)
{
	char prefix[] = "wanXXXXXXXXXX_";
	char prc[16] = {0};

	if(!ifname)
		return -1;

	prctl(PR_GET_NAME, prc);

	_dprintf("[%s]unit:%d, ifname: %s,  up:%d\n", __FUNCTION__, unit, ifname, up);
	_update_wanX_nvram(unit, ifname, up, prc);
	_set_mtwan_route(unit, ifname, up);

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

#ifdef RTCONFIG_MULTIWAN_PROFILE
	mtwan_init_profile();
#endif

	if(up)
	{
#ifdef RTCONFIG_IPV6
		_mtwan_wan6_up(unit, ifname);
#endif

		if (unit < 0
#ifdef RTCONFIG_SOFTWIRE46
		|| (nvram_pf_get_int(prefix, "s46_hgw_case") == S46_CASE_MAP_HGW_OFF && !strcmp(prc, "udhcpc_wan"))
#endif
		) {
			update_resolvconf();
			return 0;
		}

		update_wan_state(prefix, WAN_STATE_CONNECTED, 0);

		start_firewall(unit, 0);
#ifdef RTCONFIG_FILTER_CUSTOM
		reload_filter_custom_rule();
#endif

#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (is_mtwan_primary_group(unit))
			update_resolvconf();
		if (is_mtwan_lb(unit)) {
			mtwan_update_lb_route(unit, up);
			mtwan_update_lb_iptables(unit, up, MTWAN_HANDLE_V4);
		}
#endif

#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_resolvconf();
#endif

#ifdef RTCONFIG_SOFTWIRE46
		if (wan_hgw_detect(unit, ifname, prc))
			return 0;
#endif

#ifdef RTCONFIG_MULTISERVICE_WAN
		if (!is_ms_wan_unit(unit))
#endif
		{
#ifdef RTCONFIG_GETREALIP
			doSystem("realip %d 1>/dev/null &", unit);
#endif
			start_mtwan_iQos(unit);
		}
	}
	else
	{
		if (unit >=0
#ifdef RTCONFIG_MULTISERVICE_WAN
		&& !is_ms_wan_unit(unit)
#endif
		) {
			stop_mtwan_iQos(unit);
		}
		remove_ip_rules(mtwan_get_route_rule_pref(unit), MTWAN_HANDLE_V4);
		remove_ip_rules(mtwan_get_dns_rule_pref(unit), MTWAN_HANDLE_V4);
		update_wan_state(prefix, WAN_STATE_DISCONNECTED, WAN_STOPPED_REASON_NONE);

#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (is_mtwan_primary_group(unit))
			update_resolvconf();
		if (is_mtwan_lb(unit)) {
			remove_ip_rules(mtwan_get_mark_rule_pref(unit), MTWAN_HANDLE_V4);
			mtwan_update_lb_route(unit, up);
			mtwan_update_lb_iptables(unit, up, MTWAN_HANDLE_V4);
		}
#endif

#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_resolvconf();
#endif

	}

	return 0;
}

/*
int mtwan_update_nat_firewall(FILE *fp_nat)
{
	char prefix[] = "wanXXXX_", tmp[256];
	int i, unit;
	char *gw_ifname, *ipaddr;

	if(fp_nat)
	{		
		for (i = 0; i < MAX_MULTI_WAN_NUM; ++i)
		{
			unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));
			if (unit != -1 && is_mtwan_unit(unit))
			{
				//TODO: check wan state
				if (nvram_match(strlcat_r(prefix, "enable", tmp, sizeof(tmp)), "1"))
				{
					gw_ifname = nvram_safe_get(strlcat_r(prefix, "gw_ifname", tmp, sizeof(tmp)));
					ipaddr = nvram_safe_get(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)));
					if(gw_ifname[0] != '\0' && ipaddr[0] != '\0')
					{
						fprintf(fp_nat, "-A PREROUTING -d %s/32 -j VSERVER\n", ipaddr);
						fprintf(fp_nat, "-A POSTROUTING ! -s %s/32 -o %s -j MASQUERADE\n", ipaddr, gw_ifname);
					} 
				}
			}
			else
			{
				_dprintf("[%s] invalid wan_idx\n", __FUNCTION__);
			}
		}
	}
	return 0;
}
*/

char *mtwan_get_lan_ifname(int port, char *buf, size_t len)
{
#ifdef RTCONFIG_NEW_PHYMAP
	phy_port_mapping port_mapping;
	int i;
	int lport = 0;

	if (!buf)
		return NULL;

	get_phy_port_mapping(&port_mapping);
	for (i = 0; i < port_mapping.count; i++) {
		if (!(port_mapping.port[i].cap & PHY_PORT_CAP_LAN))
			continue;
		if (port_mapping.port[i].seq_no != -1)
			lport = port_mapping.port[i].seq_no;
		else
			lport = (int)strtol(port_mapping.port[i].label_name+1, NULL, 0);
		if (port == lport) {
			strlcpy(buf, port_mapping.port[i].ifname, len);
			return buf;
		}
	}
	return NULL;
#else
	char wired_ifnames[128] = {0};
	char ifname[IFNAMSIZ], *next;
	int i;

	if (!buf)
		return NULL;

	strlcpy(wired_ifnames, nvram_safe_get("ethsw_ifnames"), sizeof(wired_ifnames));
	i = 1;
	foreach(ifname, wired_ifnames, next) {
		if (i == port) {
			strlcpy(buf, ifname, len);
			break;
		}
		i++;
	}

	return buf;
#endif
}

char *mtwan_get_wan_ifname(char *buf, size_t len)
{
	if (!buf)
		return NULL;

	if (nvram_match("switch_wantag", "none"))
		strlcpy(buf, wan_if_eth(), len);
	else
		strlcpy(buf, nvram_safe_get("switch_wantag_ifname"), len);

	return buf;
}

#ifdef RTCONFIG_BONDING_WAN
// remove the lan port which will bonding with wan port.
static void _remove_mtwan_data_by_idx(const char *nvname, int idx)
{
	int i, j;
	char mtwan_prefix[16] = {0};
	char old_data[32] = {0};
	char new_data[32] = {0};
	char value[16] = {0}, *next = NULL;
	int removed_group = 0;

	if (!nvname)
		return;

	for (i = 1; i <= MAX_MTWAN_PROFILE_NUM; i++)
	{
		memset(old_data, 0, sizeof(old_data));
		snprintf(mtwan_prefix, sizeof(mtwan_prefix), "mtwan%d_", i);
		strlcpy(old_data, nvram_pf_safe_get(mtwan_prefix, nvname), sizeof(old_data));
		if (*old_data)
		{
			memset(new_data, 0, sizeof(new_data));
			j = 0;
			foreach(value, old_data, next)
			{
				if (j == idx)
				{
					if (!strcmp(nvname, "mt_group"))
						removed_group = strtol(value, NULL, 10);
					j++;
					continue;
				}
				else
				{
					if (*new_data)
						strlcat(new_data, " ", sizeof(new_data));
					strlcat(new_data, value, sizeof(new_data));
				}
				j++;
			}
			nvram_pf_set(mtwan_prefix, nvname, new_data);
		}

		if (removed_group)
		{
			memset(old_data, 0, sizeof(old_data));
			strlcpy(old_data, nvram_pf_safe_get(mtwan_prefix, "order"), sizeof(old_data));
			if (*old_data)
			{
				memset(new_data, 0, sizeof(new_data));
				foreach(value, old_data, next)
				{
					if (removed_group == (int)strtol(value, NULL, 10))
					{
						continue;
					}
					else
					{
						if (*new_data)
							strlcat(new_data, " ", sizeof(new_data));
						strlcat(new_data, value, sizeof(new_data));
					}
				}
				nvram_pf_set(mtwan_prefix, "order", new_data);
			}
		}
	}
}

static void _update_wans_mt_bonding_port()
{
	char ioports_orig[32] = {0};
	char ioports_new[32] = {0};
	char ioport[8] = {0}, *next = NULL;
	int lanport;
	char ifname[IFNAMSIZ] = {0};
	char wan_bonding_ifnames[64] = {0};
	int i;
	int removed_idx = -1;

	if (nvram_get_int("bond_wan") == 0)
		return;

	nvram_safe_get_r("wans_mt_ioport", ioports_orig, sizeof(ioports_orig));
	i = 0;
	foreach(ioport, ioports_orig, next)
	{
		lanport = 0;
		if (!strncmp(ioport, "lan", 3))
			lanport = (int)strtol(ioport + 3, NULL, 0);
#ifdef RTCONFIG_NEW_PHYMAP
		else if (!strncmp(ioport, "L", 1))
		{
			phy_port_mapping port_mapping;
			int j;

			get_phy_port_mapping(&port_mapping);
			for (j = 0; j < port_mapping.count; j++)
			{
				if (!strcmp(port_mapping.port[j].label_name, ioport))
				{
					if (port_mapping.port[j].seq_no != -1)
						lanport = port_mapping.port[j].seq_no;
					else
						lanport = (int)strtol(ioport + 1, NULL, 0);
				}
			}
		}
#endif
		if (lanport)
		{
			mtwan_get_lan_ifname(lanport, ifname, sizeof(ifname));
			get_wan_bonding_ifnames(wan_bonding_ifnames, sizeof(wan_bonding_ifnames));
			if (strstr(wan_bonding_ifnames, ifname))
			{
				removed_idx = i;
				i++;
				continue;
			}
		}

		if (*ioports_new)
			strlcat(ioports_new, " ", sizeof(ioports_new));
		strlcat(ioports_new, ioport, sizeof(ioports_new));
		i++;
	}

	if (*ioports_new)
		nvram_set("wans_mt_ioport", ioports_new);

	// update mtwan_mt_group, mtwan_mt_weight, mtwan_order
	if (removed_idx != -1)
	{
		_remove_mtwan_data_by_idx("mt_group", removed_idx);
		_remove_mtwan_data_by_idx("mt_weight", removed_idx);
	}
}
#endif

int mtwan_init_nvram()
{
	int i, default_wan = -1;
	char prefix[] = "wanXXXXXX_", *val, tmp[256];
	char wan_type[8] = {0};
	int lanport = -1;
	int modem_unit = MODEM_UNIT_FIRST;
	char modem_prefix[16] = {0};
	char ifname[IFNAMSIZ];
	int real_unit;
#ifdef RTCONFIG_MULTIWAN_PROFILE
	char prefix_orig[16] = {0};
	char buf[256] = {0}, *next = NULL;
	char wans_dualwan[16] = {0};
#endif

	for(i = MULTI_WAN_START_IDX; i < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++i)
	{
		//check default wan
		snprintf(prefix, sizeof(prefix), "wan%d_", i);
		val = nvram_get(strlcat_r(prefix, "dualwan_idx", tmp, sizeof(tmp)));
		if(val && atoi(val) == WAN_UNIT_FIRST)
		{
			default_wan = i;
			break;
		}
	}

	if(default_wan == -1)	//default state, set wan50 as default wan and enable it.
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", MULTI_WAN_START_IDX);
		nvram_set_int(strlcat_r(prefix, "dualwan_idx", tmp, sizeof(tmp)), WAN_UNIT_FIRST);
		nvram_set_int(strlcat_r(prefix, "enable", tmp, sizeof(tmp)), 1);
		return 0;
	}

#ifdef RTCONFIG_MULTIWAN_PROFILE
#ifdef RTCONFIG_BONDING_WAN
	_update_wans_mt_bonding_port();
#endif
	/// ifnames, type, lanport
	nvram_safe_get_r("wans_mt_ioport", buf, sizeof(buf));
	nvram_safe_get_r("wans_dualwan", wans_dualwan, sizeof(wans_dualwan));
	i = 0;
	set_wan_phy("");
	foreach(wan_type, buf, next)
	{
		snprintf(prefix_orig, sizeof(prefix_orig), "wan%d_", i+MULTI_WAN_START_IDX);
		if (!nvram_get(strlcat_r(prefix_orig, "dualwan_idx", tmp, sizeof(tmp))))
			nvram_set_int(strlcat_r(prefix_orig, "dualwan_idx", tmp, sizeof(tmp)), WAN_UNIT_NONE);
		real_unit = mtwan_get_real_wan(i+MULTI_WAN_START_IDX, prefix, sizeof(prefix));
		if (real_unit < 0)
			continue;
		nvram_set(strlcat_r(prefix, "enable", tmp, sizeof(tmp)), "1");
		if ((val = nvram_get(strlcat_r(prefix, "proto", tmp, sizeof(tmp)))) == NULL || !*val)
		{
			nvram_set(strlcat_r(prefix, "proto", tmp, sizeof(tmp)), "dhcp");
			nvram_set(strlcat_r(prefix, "dhcpenable_x", tmp, sizeof(tmp)), "1");
			nvram_set(strlcat_r(prefix, "dnsenable_x", tmp, sizeof(tmp)), "1");
			nvram_set(strlcat_r(prefix, "nat_x", tmp, sizeof(tmp)), "1");
		}

		if (!strncmp(wan_type, "wan", 3))
		{
			// wan type
			nvram_set(strlcat_r(prefix, "type", tmp, sizeof(tmp)), "wan");
			nvram_set(strlcat_r(prefix_orig, "type", tmp, sizeof(tmp)), "wan");
			if (i == 0 && strstr(wans_dualwan, "none"))
			{
				nvram_set("wans_dualwan", "wan none");
			}
			// wanX_ifname
#ifdef RTCONFIG_MULTISERVICE_WAN
			if (nvram_match("switch_wantag", "none")
			 && (nvram_get_int(strlcat_r(prefix, "dot1q", tmp, sizeof(tmp))) || nvram_get_int(strlcat_r(prefix, "ms_cnt", tmp, sizeof(tmp))) > 1)
			)
				nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), get_mswan_ifname(real_unit, ifname, sizeof(ifname)));
			else
#endif
#ifdef RTCONFIG_BONDING_WAN
			if (nvram_get_int("bond_wan") && if_nametoindex(get_wan_bonding_ifname(ifname, sizeof(ifname))))
				nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
			else
#endif
			nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), mtwan_get_wan_ifname(ifname, sizeof(ifname)));
			// wan_ifnames
#ifdef RTCONFIG_BONDING_WAN
			if (nvram_get_int("bond_wan") && if_nametoindex(get_wan_bonding_ifname(ifname, sizeof(ifname))))
				add_wan_phy(ifname);
			else
#endif
			add_wan_phy(mtwan_get_wan_ifname(ifname, sizeof(ifname)));
		}
		else if (!strncmp(wan_type, "lan", 3))
		{
			// wan type
			nvram_set(strlcat_r(prefix, "type", tmp, sizeof(tmp)), "lan");
			nvram_set(strlcat_r(prefix_orig, "type", tmp, sizeof(tmp)), "lan");
			if (wan_type[3])
			{
				lanport = atoi(wan_type + 3);
				nvram_set_int(strlcat_r(prefix, "lanport", tmp, sizeof(tmp)), lanport);
				if (i == 0 && strstr(wans_dualwan, "none"))
				{
					nvram_set("wans_dualwan", "lan none");
					nvram_set_int("wans_lanport", lanport);
				}
				// wan_ifnames, lan_ifnames
				if (!mtwan_get_lan_ifname(lanport, ifname, sizeof(ifname)))
					continue;
				del_lan_phy(ifname);
				add_wan_phy(ifname);
				// wanX_ifname
#ifdef RTCONFIG_MULTISERVICE_WAN
				if (nvram_get_int(strlcat_r(prefix, "dot1q", tmp, sizeof(tmp))) || nvram_get_int(strlcat_r(prefix, "ms_cnt", tmp, sizeof(tmp))) > 1)
					nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), get_mswan_ifname(real_unit, ifname, sizeof(ifname)));
				else
#endif
				nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
			}
			else
				_dprintf("%s:%d: configure error: %s\n", __FUNCTION__, __LINE__, wan_type);
		}
		else if (!strcmp(wan_type, "usb"))
		{
			// wan type
			nvram_set(strlcat_r(prefix, "type", tmp, sizeof(tmp)), "usb");
			nvram_set(strlcat_r(prefix_orig, "type", tmp, sizeof(tmp)), "usb");
			if (i == 0 && strstr(wans_dualwan, "none"))
			{
				nvram_set("wans_dualwan", "usb none");
			}
			// wanX_ifname
			usb_modem_prefix(MODEM_UNIT_FIRST, modem_prefix, sizeof(modem_prefix));
			strlcpy(ifname, nvram_safe_get(strlcat_r(modem_prefix, "act_dev", tmp, sizeof(tmp))), sizeof(ifname));
			nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
			// wan_ifnames
			add_wan_phy("usb");
		}
#ifdef RTCONFIG_NEW_PHYMAP
		else if (strlen(wan_type) == 2)
		{
			phy_port_mapping port_mapping;
			int j;

			get_phy_port_mapping(&port_mapping);
			for (j = 0; j < port_mapping.count; j++)
			{
				if (!strcmp(port_mapping.port[j].label_name, wan_type))
				{
					if (*port_mapping.port[j].label_name == 'W')
					{
						// wan type
						nvram_set(strlcat_r(prefix, "type", tmp, sizeof(tmp)), "wan");
						nvram_set(strlcat_r(prefix_orig, "type", tmp, sizeof(tmp)), "wan");
						if (i == 0 && strstr(wans_dualwan, "none"))
						{
							nvram_set("wans_dualwan", "wan none");
						}
						// wanX_ifname
#ifdef RTCONFIG_MULTISERVICE_WAN
						if (nvram_match("switch_wantag", "none")
						 && (nvram_get_int(strlcat_r(prefix, "dot1q", tmp, sizeof(tmp))) || nvram_get_int(strlcat_r(prefix, "ms_cnt", tmp, sizeof(tmp))) > 1)
						)
							nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), get_mswan_ifname(real_unit, ifname, sizeof(ifname)));
						else
#endif
#ifdef RTCONFIG_BONDING_WAN
						if (nvram_get_int("bond_wan") && if_nametoindex(get_wan_bonding_ifname(ifname, sizeof(ifname))))
							nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
						else
#endif
						if (nvram_match("switch_wantag", "none"))
							nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), port_mapping.port[j].ifname);
						else
							nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), nvram_safe_get("switch_wantag_ifname"));
						// wan_ifnames
#ifdef RTCONFIG_BONDING_WAN
						if (nvram_get_int("bond_wan") && if_nametoindex(get_wan_bonding_ifname(ifname, sizeof(ifname))))
							add_wan_phy(ifname);
						else
#endif
						if (nvram_match("switch_wantag", "none"))
							add_wan_phy(port_mapping.port[j].ifname);
						else
							add_wan_phy(nvram_safe_get("switch_wantag_ifname"));
					}
					else if (*port_mapping.port[j].label_name == 'L')
					{
						// wan type
						nvram_set(strlcat_r(prefix, "type", tmp, sizeof(tmp)), "lan");
						nvram_set(strlcat_r(prefix_orig, "type", tmp, sizeof(tmp)), "lan");
						if (port_mapping.port[j].seq_no != -1)
							lanport = port_mapping.port[j].seq_no;
						else
							lanport = (int)strtol(wan_type + 1, NULL, 10);
						nvram_set_int(strlcat_r(prefix, "lanport", tmp, sizeof(tmp)), lanport);
						if (i == 0 && strstr(wans_dualwan, "none"))
						{
							nvram_set("wans_dualwan", "lan none");
							nvram_set_int("wans_lanport", lanport);
						}
						// wanX_ifname
#ifdef RTCONFIG_MULTISERVICE_WAN
						if (nvram_get_int(strlcat_r(prefix, "dot1q", tmp, sizeof(tmp))) || nvram_get_int(strlcat_r(prefix, "ms_cnt", tmp, sizeof(tmp))) > 1)
							nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), get_mswan_ifname(real_unit, ifname, sizeof(ifname)));
						else
#endif
						nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), port_mapping.port[j].ifname);
						// wan_ifnames, lan_ifnames
						del_lan_phy(port_mapping.port[j].ifname);
						add_wan_phy(port_mapping.port[j].ifname);
					}
					else if (*port_mapping.port[j].label_name == 'U')
					{
						char *usb_wan_type;
#ifdef RTCONFIG_USB_MULTIMODEM
						modem_unit = (int)strtol(wan_type + 1, NULL, 10) - 1;
#endif
						// wan type
						usb_wan_type = (modem_unit == MODEM_UNIT_FIRST) ? "usb" : "usb2";
						nvram_set(strlcat_r(prefix, "type", tmp, sizeof(tmp)), usb_wan_type);
						nvram_set(strlcat_r(prefix_orig, "type", tmp, sizeof(tmp)), usb_wan_type);
						if (i == 0 && strstr(wans_dualwan, "none"))
						{
							char new_wans_dualwan[32];
							snprintf(new_wans_dualwan, sizeof(new_wans_dualwan), "%s none", usb_wan_type);
							nvram_set("wans_dualwan", new_wans_dualwan);
						}
						// wanX_ifname
						usb_modem_prefix(modem_unit, modem_prefix, sizeof(modem_prefix));
						strlcpy(ifname, nvram_safe_get(strlcat_r(modem_prefix, "act_dev", tmp, sizeof(tmp))), sizeof(ifname));
						nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
						// wan_ifnames
						add_wan_phy(usb_wan_type);
					}
					else
						continue;

					break;
				}
			}
		}
#endif
		i++;
	}
	for (;i < MAX_MULTI_WAN_NUM; i++)
	{
		snprintf(prefix_orig, sizeof(prefix_orig), "wan%d_", i);
		real_unit = mtwan_get_real_wan(i+MULTI_WAN_START_IDX, prefix, sizeof(prefix));
		if (real_unit < 0)
			continue;
		nvram_set(strlcat_r(prefix, "enable", tmp, sizeof(tmp)), "0");
		nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), "");
		nvram_set(strlcat_r(prefix, "type", tmp, sizeof(tmp)), "");
		if (i < WAN_UNIT_MAX)
			nvram_set(strlcat_r(prefix_orig, "type", tmp, sizeof(tmp)), "");
#ifdef RTCONFIG_IPV6
		if (nvram_get(ipv6_nvname_by_unit("ipv6_service", real_unit)))
			nvram_set(ipv6_nvname_by_unit("ipv6_service", real_unit), "disabled");
#endif
	}

	mtwan_init_profile();

#else
	// update ifnames
	for(i = MULTI_WAN_START_IDX; i < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++i)
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", i);
		nvram_safe_get_r(strlcat_r(prefix, "type", tmp, sizeof(tmp)), wan_type, sizeof(wan_type));
		if (!strcmp(wan_type, "wan"))
		{
			// set wan ifname
			real_unit = mtwan_get_real_wan(i, prefix, sizeof(prefix));
			if (real_unit < 0)
				continue;
#ifdef RTCONFIG_MULTISERVICE_WAN
			if (nvram_match("switch_wantag", "none")
			 && (nvram_get_int(strlcat_r(prefix, "dot1q", tmp, sizeof(tmp))) || nvram_get_int(strlcat_r(prefix, "ms_cnt", tmp, sizeof(tmp))) > 1)
			)
				nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), get_mswan_ifname(real_unit, ifname, sizeof(ifname)));
			else
#endif
			nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), mtwan_get_wan_ifname(ifname, sizeof(ifname)));
		}
		else if (!strcmp(wan_type, "lan"))
		{
			lanport = nvram_get_int(strlcat_r(prefix, "lanport", tmp, sizeof(tmp)));
			if (lanport < 1)
				continue;
			// update lan_ifnames
			if (!mtwan_get_lan_ifname(lanport, ifname, sizeof(ifname)))
				continue;
			del_lan_phy(ifname);
			// set wan ifname
			add_wan_phy(ifname);
			real_unit = mtwan_get_real_wan(i, prefix, sizeof(prefix));
			if (real_unit < 0)
				continue;
#ifdef RTCONFIG_MULTISERVICE_WAN
			if (nvram_match("switch_wantag", "none")
			 && (nvram_get_int(strlcat_r(prefix, "dot1q", tmp, sizeof(tmp))) || nvram_get_int(strlcat_r(prefix, "ms_cnt", tmp, sizeof(tmp))) > 1)
			)
				nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), get_mswan_ifname(real_unit, ifname, sizeof(ifname)));
			else
#endif
			nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
		}
		else
		{
			nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), "");
		}
	}
#endif

	if (nvram_get_int("success_start_service") == 0)
	{
		for(i = MULTI_WAN_START_IDX; i < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++i)
		{
			snprintf(tmp, sizeof(tmp), "wan%d_primary", i);
			val = nvram_get(tmp);
			if(val && atoi(val))
				nvram_set(tmp, "0");
		}
		nvram_set("wan_primary", "0");
#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (nvram_get_int("mtwan1_enable"))
			nvram_set_int("mtwan1_group", nvram_get_int("wan0_group"));
#endif
	}

	return 0;
}

int mtwan_get_mapped_unit (int mtwan_unit)
{
	char nvname[32];
	int dualwan_idx = -1;
	char *p;
	snprintf(nvname, sizeof(nvname), "wan%d_dualwan_idx", mtwan_unit);
	if ((p = nvram_get(nvname)))
		dualwan_idx = atoi(p);
	return ((dualwan_idx >= 0) ? dualwan_idx : mtwan_unit);
}

int mtwan_get_dns(const int mtwan_unit, char *dns1, const size_t dns1_sz, char *dns2, const size_t dns2_sz)
{
	char prefix[] = "wanXXXX_", *dns;
	char word[64], *next;
	int i = 0;
	
	if(mtwan_get_real_wan(mtwan_unit, prefix, sizeof(prefix)) != -1)
	{
		dns = nvram_pf_get(prefix, "dns");
		if(dns)
		{
			foreach(word, dns, next)
			{
				++i;
				if(i == 1)	//dns1
				{
					strlcpy(dns1, word, dns1_sz);
				}
				else
				{
					strlcpy(dns2, word, dns2_sz);
					break;
				}
			}
			return i;
		}

	}
	return -1;
}

int mtwan_get_wans_type(void)
{
	int caps = 0;
	int unit;
	char nvname[16], *type;

	for(unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++unit)
	{
		snprintf(nvname, sizeof(nvname), "wan%d_type", unit);
		type = nvram_safe_get(nvname);
		if (!strcmp(type,"lan")) caps |= WANSCAP_LAN;
		if (!strcmp(type,"2g")) caps |= WANSCAP_2G;
		if (!strcmp(type,"5g")) caps |= WANSCAP_5G;
		if (!strcmp(type,"usb")) caps |= WANSCAP_USB;
		if (!strcmp(type,"dsl")) caps |= WANSCAP_DSL;
		if (!strcmp(type,"wan")) caps |= WANSCAP_WAN;
		if (!strcmp(type,"wan2")) caps |= WANSCAP_WAN2;
		if (!strcmp(type,"sfp+")) caps |= WANSCAP_SFPP;
		if (!strcmp(type,"6g")) caps |= WANSCAP_6G;
	}

	return caps;
}
