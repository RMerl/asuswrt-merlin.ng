 /*
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. ASUS
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

/*
	gn_wbl.c is the API to add ebtables and iptables rule for guest network white and black list or Amazon WSS (WiFi Simple Setup) certificate
*/

#include "rc.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <shutils.h>

/* DEBUG DEFINE */
#define GNWBL_DEBUG             "/tmp/GNWBL_DEBUG"
#define GNWBL_DBG(fmt,args...) \
	if(f_exists(GNWBL_DEBUG) > 0) { \
		_dprintf("[GNWBL][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

static void GN_WBL_tag(int i, int j, char *buf, int len)
{
	snprintf(buf, len, "%d", 100+i*10+j);
}

static void GN_WBL_broute_rule(char *ifname, int mark)
{
	char buf[16] = {0};
	char https_port[6] = {0};
	char prefix[32] = {0}, tmp[100] = {0};
	char *wan_ipaddr = NULL;

	/* WSS only request to ban to access router index, ASUS extend the protection to whole ports except DNS / DHCP */
	/* only access DNS / DHCP port in router, others dropped */
	eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", ifname, "-p", "IPv4", "--ip-dst", nvram_safe_get("lan_ipaddr"), "--ip-proto", "udp", "--ip-dport", "53", "-j", "ACCEPT");
	eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", ifname, "-p", "IPv4", "--ip-dst", nvram_safe_get("lan_ipaddr"), "--ip-proto", "udp", "--ip-dport", "67", "-j", "ACCEPT");
	eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", ifname, "-p", "IPv4", "--ip-dst", nvram_safe_get("lan_ipaddr"), "--ip-proto", "udp", "--ip-dport", "68", "-j", "ACCEPT");
	eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", ifname, "-p", "IPv4", "--ip-dst", nvram_safe_get("lan_ipaddr"), "-j", "DROP");

	/* add mark-set to identify which interface in iptables */
	snprintf(buf, sizeof(buf), "%d", mark);
	eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", ifname, "-j", "mark", "--mark-set", buf, "--mark-target", "ACCEPT");

	GNWBL_DBG(" add rule for (%s,%d)\n", ifname, mark);
}

/* ebtables broute chain */
void add_GN_WBL_EBTbrouteRule()
{
	int i = 0;
	int j = 1;
	unsigned int max_mssid;
	char ifname[128], *next;
	char wl_if[32] = {0};
	char mssid_if[32] = {0};
	char gn_en[32] = {0};
	char gn_wbl_en[32] = {0};
	char gn_bw_en[32] = {0};
	int  g_mark = GUEST_INIT_MARKNUM - 1;

	/* flush whole broute rule */
	eval("ebtables", "-t", "broute", "-F");

	if (!is_routing_enabled()) {
		GNWBL_DBG(" no routing ability!\n");
		return;
	}

#ifdef RTCONFIG_LANTIQ
	if (nvram_get_int("wave_ready") == 0) {
		GNWBL_DBG(" wifi is not ready!\n");
		return;
	}
#endif

	/* traveling whole guest network */
	foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
#if defined(RTCONFIG_PSR_GUEST) && !defined(HND_ROUTER)
		max_mssid++;
#endif
		max_mssid = num_of_mssid_support(i);
		for (j = 1; j < max_mssid+1; j++) {
			get_wlxy_ifname(i, j, wl_if);
			snprintf(mssid_if, sizeof(mssid_if), "wl%d.%d", i, j);
			snprintf(gn_en, sizeof(gn_en), "%s_bss_enabled", mssid_if);
			snprintf(gn_wbl_en, sizeof(gn_wbl_en), "%s_gn_wbl_enable", mssid_if);
			snprintf(gn_bw_en, sizeof(gn_bw_en), "%s_bw_enabled", mssid_if);

			if (nvram_match(gn_en, "1") && nvram_match(gn_bw_en, "1")) {
				g_mark++;
			}

			if (nvram_match(gn_en, "1") && nvram_match(gn_wbl_en, "1")) {
				GN_WBL_broute_rule(wl_if, g_mark);
			}
		}
		i++;
	}
}

void add_GN_WBL_ChainRule(FILE *fp)
{
	int i = 0;
	int j = 1;
	unsigned int max_mssid;
	char ifname[128], *next;
	char mssid_if[32] = {0};
	char gn_en[32] = {0};
	char gn_wbl_en[32] = {0};
	char mark[8] = {0};
	int len = sizeof(mark);

	/* traveling whole guest network */
	foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
#if defined(RTCONFIG_PSR_GUEST) && !defined(HND_ROUTER)
		max_mssid++;
#endif
		max_mssid = num_of_mssid_support(i);
		for (j = 1; j < max_mssid+1; j++) {
			snprintf(mssid_if, sizeof(mssid_if), "wl%d.%d", i, j);
			snprintf(gn_en, sizeof(gn_en), "%s_bss_enabled", mssid_if);
			snprintf(gn_wbl_en, sizeof(gn_wbl_en), "%s_gn_wbl_enable", mssid_if);

			if (nvram_match(gn_en, "1") && nvram_match(gn_wbl_en, "1")) {
				GN_WBL_tag(i, j, mark, len);
				fprintf(fp, ":GN_WBL_%s - [0:0]\n", mark);
			}
		}
		i++;
	}
}

static void GN_WBL_applyRule(FILE *fp, char *chain, char *ifname)
{
	char gn_type[32] = {0};
	char gn_rule[1024] = {0};
	int type = 1; // default : whitelist

	char *buf = NULL, *g = NULL, *p = NULL;
	char *addr = NULL, *proto = NULL, *port = NULL;
	char *action1 = NULL;
	char *action2 = NULL;
	char rule_ip[32] = {0};
	char rule_port[32] = {0};
	char tmp[256] = {0};

	snprintf(gn_type, sizeof(gn_type), "%s_gn_wbl_type", ifname);
	snprintf(gn_rule, sizeof(gn_rule), "%s_gn_wbl_rule", ifname);

	/* check type and rule */
	if (!strcmp(nvram_safe_get(gn_type), "") || !strcmp(nvram_safe_get(gn_rule), "")) {
		GNWBL_DBG(" no gn_type and no gn_rule!\n");
		return;
	}

	/* ebtables action */
	type = nvram_get_int(gn_type);
	if (type == 0) {
		// blacklist
		action1 = "DROP";
		action2 = "ACCEPT";
	}
	else {
		// whitelist (default)
		action1 = "ACCEPT";
		action2 = "DROP";
	}

	/* ebtables dst-ip / proto / dst-port */
	g = buf = strdup(nvram_safe_get(gn_rule));
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &addr, &proto, &port)) != 3) continue;

		/* dst-ip */
		if (addr != NULL && strcmp(addr, "")) {
			snprintf(rule_ip, sizeof(rule_ip), "-d %s", addr);
		}

		/* proto + dst-port */
		if ((proto != NULL && strcmp(proto, "")) && (port != NULL && strcmp(port, "")))
		{
			if (strcmp(proto, "tcp") && strcmp(proto, "udp")) {
				GNWBL_DBG(" protocol is illegal (tcp or udp)\n");
				continue;
			}

			if (atoi(port) > 65535 || atoi(port) < 0) {
				GNWBL_DBG(" port is out of range (0~65535)\n");
				continue;
			}
			snprintf(rule_port, sizeof(rule_port), "-p %s -m multiport --dport %s", proto, port);
		}

		snprintf(tmp, sizeof(tmp), "-A %s %s %s -j %s", chain, rule_ip, rule_port, action1);
		fprintf(fp, "%s\n", tmp);
		GNWBL_DBG(" tmp=%s\n", tmp);
	}
	if (buf) free(buf);

	/* final rule : ACCPET or DROP in other cases */
	fprintf(fp, "-A %s -j %s\n", chain, action2);
}

void add_GN_WBL_ForwardRule(FILE *fp)
{
	int i = 0;
	int j = 1;
	unsigned int max_mssid;
	char ifname[128], *next;
	char mssid_if[32] = {0};
	char gn_en[32] = {0};
	char gn_wbl_en[32] = {0};
	char gn_bw_en[32] = {0};
	char chain[16] = {0};
	char mark[8] = {0};
	int  len = sizeof(mark);
	int  g_mark = GUEST_INIT_MARKNUM - 1;

	/* traveling whole guest network */
	foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
#if defined(RTCONFIG_PSR_GUEST) && !defined(HND_ROUTER)
		max_mssid++;
#endif
		max_mssid = num_of_mssid_support(i);
		for (j = 1; j < max_mssid+1; j++) {
			snprintf(mssid_if, sizeof(mssid_if), "wl%d.%d", i, j);
			snprintf(gn_en, sizeof(gn_en), "%s_bss_enabled", mssid_if);
			snprintf(gn_wbl_en, sizeof(gn_wbl_en), "%s_gn_wbl_enable", mssid_if);
			snprintf(gn_bw_en, sizeof(gn_bw_en), "%s_bw_enabled", mssid_if);

			if (nvram_match(gn_en, "1") && nvram_match(gn_bw_en, "1")) {
				g_mark++;
			}

			if (nvram_match(gn_en, "1") && nvram_match(gn_wbl_en, "1")) {
				GN_WBL_tag(i, j, mark, len);
				snprintf(chain, sizeof(chain), "GN_WBL_%s", mark);
				fprintf(fp, "-A FORWARD -m mark --mark %d -j %s\n", g_mark, chain);
				GN_WBL_applyRule(fp, chain, mssid_if);
			}
		}
		i++;
	}
}

#ifdef RTCONFIG_LANTIQ
static int state_old = 0;
void GN_WBL_restart()
{
	int state = 0;

	if (nvram_get_int("wave_ready") == 1) {
		state = 1;
	}

	if (nvram_get_int("wave_ready") == 0) {
		state = 0;
		state_old = 0;
	}

	GNWBL_DBG(" state=%d, state_old=%d\n", state, state_old);

	if (state == state_old) return;

	if (state_old == 0 && state == 1) {
		GNWBL_DBG(" apply add_GN_WBL_EBTbrouteRule()\n");
		state_old = state;

		/* restart lanaccess_wl to bring up ebtables rule */
		lanaccess_wl();
	}
}
#endif
